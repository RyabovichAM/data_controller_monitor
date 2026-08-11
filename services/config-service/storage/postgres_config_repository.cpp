#include "postgres_config_repository.h"

#include <google/protobuf/util/json_util.h>

namespace config {

namespace {

constexpr const char* kCreateTable =
    "CREATE TABLE IF NOT EXISTS collector_config ("
    "    collector_id TEXT PRIMARY KEY,"
    "    config       JSONB       NOT NULL,"
    "    version      BIGINT      NOT NULL,"
    "    updated_at   TIMESTAMPTZ NOT NULL DEFAULT now())";

std::string ToJson(const CollectorConfig& config) {
    google::protobuf::util::JsonPrintOptions options;
    options.preserve_proto_field_names = true;   // keeps rows readable in psql

    std::string json;
    google::protobuf::util::MessageToJsonString(config, &json, options);
    return json;
}

std::optional<CollectorConfig> FromJson(const std::string& json) {
    google::protobuf::util::JsonParseOptions options;
    // A field dropped from the contract must not make old rows unreadable.
    options.ignore_unknown_fields = true;

    CollectorConfig config;
    if (!google::protobuf::util::JsonStringToMessage(json, &config, options).ok()) {
        return std::nullopt;
    }

    return config;
}

}   //namespace

PostgresConfigRepository::PostgresConfigRepository(const std::string& connection_string)
    : connection_string_{connection_string}
    , connection_{connection_string} {
    EnsureSchema();
}

std::optional<CollectorConfig> PostgresConfigRepository::Get(const std::string& collector_id) {
    std::lock_guard<std::mutex> lock{mutex_};

    return InTransaction([&collector_id](pqxx::work& tx) -> std::optional<CollectorConfig> {
        pqxx::result rows = tx.exec_params(
            "SELECT config::text FROM collector_config WHERE collector_id = $1",
            collector_id);

        if (rows.empty()) {
            return std::nullopt;
        }

        return FromJson(rows[0][0].as<std::string>());
    });
}

std::vector<CollectorConfig> PostgresConfigRepository::List() {
    std::lock_guard<std::mutex> lock{mutex_};

    return InTransaction([](pqxx::work& tx) {
        pqxx::result rows = tx.exec(
            "SELECT config::text FROM collector_config ORDER BY collector_id");

        std::vector<CollectorConfig> configs;
        configs.reserve(rows.size());
        for (const auto& row : rows) {
            std::optional<CollectorConfig> config = FromJson(row[0].as<std::string>());
            if (config) {
                configs.push_back(std::move(*config));
            }
        }

        return configs;
    });
}

ConfigRepository::SaveResult PostgresConfigRepository::Save(const CollectorConfig& config,
                                                            int64_t expected_version) {
    std::lock_guard<std::mutex> lock{mutex_};

    return InTransaction([&config, expected_version](pqxx::work& tx) {
        // FOR UPDATE holds the row until commit, so two concurrent saves cannot
        // read the same version and bump it to the same value.
        pqxx::result rows = tx.exec_params(
            "SELECT version FROM collector_config WHERE collector_id = $1 FOR UPDATE",
            config.collector_id());

        const int64_t current = rows.empty() ? 0 : rows[0][0].as<int64_t>();

        // Zero means overwrite unconditionally, see SaveConfigRequest in the contract.
        if (expected_version != 0 && expected_version != current) {
            return SaveResult{true, current};
        }

        CollectorConfig stored = config;
        stored.set_version(current + 1);

        tx.exec_params(
            "INSERT INTO collector_config (collector_id, config, version, updated_at)"
            "    VALUES ($1, $2::jsonb, $3, now())"
            "    ON CONFLICT (collector_id) DO UPDATE SET"
            "        config = EXCLUDED.config,"
            "        version = EXCLUDED.version,"
            "        updated_at = now()",
            stored.collector_id(), ToJson(stored), stored.version());

        return SaveResult{false, stored.version()};
    });
}

bool PostgresConfigRepository::Delete(const std::string& collector_id) {
    std::lock_guard<std::mutex> lock{mutex_};

    return InTransaction([&collector_id](pqxx::work& tx) {
        pqxx::result rows = tx.exec_params(
            "DELETE FROM collector_config WHERE collector_id = $1", collector_id);

        return rows.affected_rows() > 0;
    });
}

void PostgresConfigRepository::EnsureSchema() {
    pqxx::work tx{connection_};
    tx.exec(kCreateTable);
    tx.commit();
}

template <typename Operation>
auto PostgresConfigRepository::InTransaction(Operation&& operation)
    -> decltype(operation(std::declval<pqxx::work&>())) {
    try {
        pqxx::work tx{connection_};
        auto result = operation(tx);
        tx.commit();
        return result;
    } catch (const pqxx::broken_connection&) {
        connection_ = pqxx::connection{connection_string_};

        pqxx::work tx{connection_};
        auto result = operation(tx);
        tx.commit();
        return result;
    }
}

}   //config
