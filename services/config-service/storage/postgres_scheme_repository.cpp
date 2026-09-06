#include "postgres_scheme_repository.h"

#include <google/protobuf/util/json_util.h>

namespace config {

namespace {

constexpr const char* kCreateTable =
    "CREATE TABLE IF NOT EXISTS scheme ("
    "    scheme_id    TEXT PRIMARY KEY,"
    "    title        TEXT        NOT NULL,"
    "    collector_id TEXT        NOT NULL,"
    "    scheme       JSONB       NOT NULL,"
    "    version      BIGINT      NOT NULL,"
    "    updated_at   TIMESTAMPTZ NOT NULL DEFAULT now())";

std::string ToJson(const Scheme& scheme) {
    google::protobuf::util::JsonPrintOptions options;
    options.preserve_proto_field_names = true;   // keeps rows readable in psql

    std::string json;
    google::protobuf::util::MessageToJsonString(scheme, &json, options);
    return json;
}

std::optional<Scheme> FromJson(const std::string& json) {
    google::protobuf::util::JsonParseOptions options;
    // A field dropped from the contract must not make old rows unreadable.
    options.ignore_unknown_fields = true;

    Scheme scheme;
    if (!google::protobuf::util::JsonStringToMessage(json, &scheme, options).ok()) {
        return std::nullopt;
    }

    return scheme;
}

}   //namespace

PostgresSchemeRepository::PostgresSchemeRepository(const std::string& connection_string)
    : connection_string_{connection_string}
    , connection_{connection_string} {
    EnsureSchema();
}

std::optional<Scheme> PostgresSchemeRepository::Get(const std::string& scheme_id) {
    std::lock_guard<std::mutex> lock{mutex_};

    return InTransaction([&scheme_id](pqxx::work& tx) -> std::optional<Scheme> {
        pqxx::result rows =
            tx.exec_params("SELECT scheme::text FROM scheme WHERE scheme_id = $1", scheme_id);

        if (rows.empty()) {
            return std::nullopt;
        }

        return FromJson(rows[0][0].as<std::string>());
    });
}

std::vector<SchemeSummary> PostgresSchemeRepository::List() {
    std::lock_guard<std::mutex> lock{mutex_};

    return InTransaction([](pqxx::work& tx) {
        pqxx::result rows = tx.exec(
            "SELECT scheme_id, title, collector_id, version FROM scheme ORDER BY title");

        std::vector<SchemeSummary> summaries;
        summaries.reserve(rows.size());
        for (const auto& row : rows) {
            SchemeSummary summary;
            summary.set_scheme_id(row[0].as<std::string>());
            summary.set_title(row[1].as<std::string>());
            summary.set_collector_id(row[2].as<std::string>());
            summary.set_version(row[3].as<int64_t>());
            summaries.push_back(std::move(summary));
        }

        return summaries;
    });
}

SchemeRepository::SaveResult PostgresSchemeRepository::Save(const Scheme& scheme,
                                                            int64_t expected_version) {
    std::lock_guard<std::mutex> lock{mutex_};

    return InTransaction([&scheme, expected_version](pqxx::work& tx) {
        // FOR UPDATE holds the row until commit, so two concurrent saves cannot
        // read the same version and bump it to the same value.
        pqxx::result rows = tx.exec_params(
            "SELECT version FROM scheme WHERE scheme_id = $1 FOR UPDATE", scheme.scheme_id());

        const int64_t current = rows.empty() ? 0 : rows[0][0].as<int64_t>();

        // Zero means overwrite unconditionally, see SaveSchemeRequest.
        if (expected_version != 0 && expected_version != current) {
            return SaveResult{true, current};
        }

        Scheme stored = scheme;
        stored.set_version(current + 1);

        tx.exec_params(
            "INSERT INTO scheme (scheme_id, title, collector_id, scheme, version, updated_at)"
            "    VALUES ($1, $2, $3, $4::jsonb, $5, now())"
            "    ON CONFLICT (scheme_id) DO UPDATE SET"
            "        title = EXCLUDED.title,"
            "        collector_id = EXCLUDED.collector_id,"
            "        scheme = EXCLUDED.scheme,"
            "        version = EXCLUDED.version,"
            "        updated_at = now()",
            stored.scheme_id(), stored.title(), stored.collector_id(), ToJson(stored),
            stored.version());

        return SaveResult{false, stored.version()};
    });
}

bool PostgresSchemeRepository::Delete(const std::string& scheme_id) {
    std::lock_guard<std::mutex> lock{mutex_};

    return InTransaction([&scheme_id](pqxx::work& tx) {
        pqxx::result rows =
            tx.exec_params("DELETE FROM scheme WHERE scheme_id = $1", scheme_id);

        return rows.affected_rows() > 0;
    });
}

void PostgresSchemeRepository::EnsureSchema() {
    pqxx::work tx{connection_};
    tx.exec(kCreateTable);
    tx.commit();
}

template <typename Operation>
auto PostgresSchemeRepository::InTransaction(Operation&& operation)
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
