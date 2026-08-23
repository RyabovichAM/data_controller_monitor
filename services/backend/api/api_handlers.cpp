#include "api_handlers.h"

#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <drogon/drogon.h>
#include <google/protobuf/util/json_util.h>

#include "realtime/realtime_hub.h"

namespace api {

namespace {

using clients::ConfigClient;

bool ParseJson(const std::string& text, Json::Value& value);

// The contract is the API: configs travel as the JSON of CollectorConfig, so
// there is no second schema to keep in step with the .proto file. Field names
// stay as they are written there rather than turning into camelCase.
Json::Value ProtoToJson(const google::protobuf::Message& message) {
    google::protobuf::util::JsonPrintOptions options;
    options.preserve_proto_field_names = true;
    options.always_print_primitive_fields = true;

    std::string text;
    if (!google::protobuf::util::MessageToJsonString(message, &text, options).ok()) {
        return Json::Value{Json::objectValue};
    }

    Json::Value value;
    ParseJson(text, value);

    return value;
}

bool ParseJson(const std::string& text, Json::Value& value) {
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader{builder.newCharReader()};

    std::string errors;
    return reader->parse(text.data(), text.data() + text.size(), &value, &errors);
}

drogon::HttpResponsePtr JsonError(drogon::HttpStatusCode code, const std::string& message) {
    Json::Value body;
    body["error"] = message;

    drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpJsonResponse(body);
    response->setStatusCode(code);

    return response;
}

// Every gRPC failure has an HTTP equivalent, and the interesting ones are not
// server errors: a stale version is a conflict the editor can resolve, an
// invalid config is the caller's mistake.
drogon::HttpStatusCode HttpStatusFor(grpc::StatusCode code) {
    switch (code) {
    case grpc::StatusCode::INVALID_ARGUMENT:
        return drogon::k400BadRequest;
    case grpc::StatusCode::NOT_FOUND:
        return drogon::k404NotFound;
    case grpc::StatusCode::FAILED_PRECONDITION:
        return drogon::k409Conflict;
    case grpc::StatusCode::DEADLINE_EXCEEDED:
        return drogon::k504GatewayTimeout;
    case grpc::StatusCode::UNAVAILABLE:
        return drogon::k503ServiceUnavailable;
    default:
        return drogon::k502BadGateway;
    }
}

drogon::HttpResponsePtr ErrorFor(const clients::ConfigClient::Result& result,
                                 const std::string& service) {
    return JsonError(HttpStatusFor(result.code), service + ": " + result.error);
}

void RegisterHealth() {
    // Cheap enough to answer on the loop: it touches nothing but a mutex.
    drogon::app().registerHandler(
        "/api/health",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            Json::Value body;
            body["status"] = "ok";
            body["ws_connections"] =
                static_cast<Json::UInt64>(realtime::Hub().ConnectionCount());

            callback(drogon::HttpResponse::newHttpJsonResponse(body));
        },
        {drogon::Get});
}

void RegisterCollectors(clients::StorageClient& storage,
                        trantor::EventLoopThreadPool& blocking_pool) {
    // Collectors that have data in storage — not the same list as the configured
    // ones below.
    drogon::app().registerHandler(
        "/api/collectors",
        [&storage, &blocking_pool](
            const drogon::HttpRequestPtr&,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            blocking_pool.getNextLoop()->queueInLoop(
                [&storage, callback = std::move(callback)]() mutable {
                    clients::StorageClient::CollectorList list = storage.ListCollectors();

                    if (!list.ok) {
                        callback(JsonError(drogon::k502BadGateway,
                                           "storage-service: " + list.error));
                        return;
                    }

                    Json::Value body{Json::arrayValue};
                    for (const std::string& collector_id : list.collector_ids) {
                        body.append(collector_id);
                    }

                    // Answering from a worker thread is safe: the write is
                    // handed over to the loop the connection belongs to.
                    callback(drogon::HttpResponse::newHttpJsonResponse(body));
                });
        },
        {drogon::Get});
}

// A month at one sample a second is millions of points; the ceiling keeps a
// mistyped range from turning into a response nobody can draw.
constexpr uint32_t kDefaultHistoryLimit = 5000;
constexpr uint32_t kMaxHistoryLimit = 50000;

std::optional<int64_t> ParseSeconds(const std::string& text) {
    if (text.empty()) {
        return std::nullopt;
    }

    try {
        return std::stoll(text);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

// Charts need numbers. A controller may well send them as strings — the old
// test scripts of this project do — so a string that converts whole is taken
// as a number, and anything else is not a series at all.
std::optional<double> AsNumber(const Json::Value& value) {
    if (value.isNumeric()) {
        return value.asDouble();
    }
    if (!value.isString()) {
        return std::nullopt;
    }

    const std::string text = value.asString();
    try {
        size_t consumed = 0;
        const double number = std::stod(text, &consumed);
        return consumed == text.size() ? std::optional<double>{number} : std::nullopt;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

// Columns rather than rows: a chart wants a series at a time, and the parameter
// names repeat once instead of once per point.
Json::Value ToColumns(const std::vector<clients::StorageClient::HistoryPoint>& points) {
    Json::Value times{Json::arrayValue};
    std::map<std::string, Json::Value> series;

    for (const clients::StorageClient::HistoryPoint& point : points) {
        const Json::ArrayIndex index = times.size();
        times.append(static_cast<Json::Int64>(point.seconds));

        Json::Value payload;
        if (!ParseJson(point.json, payload) || !payload.isObject()) {
            continue;
        }

        for (const std::string& name : payload.getMemberNames()) {
            std::optional<double> number = AsNumber(payload[name]);
            if (!number) {
                continue;
            }

            Json::Value& column = series[name];
            if (column.isNull()) {
                column = Json::Value{Json::arrayValue};
            }
            // A parameter that appeared later, or was missing from a sample,
            // leaves a gap rather than shifting the rest of its column.
            while (column.size() < index) {
                column.append(Json::Value{});
            }
            column.append(*number);
        }
    }

    Json::Value body;
    body["t"] = times;
    body["series"] = Json::Value{Json::objectValue};
    for (auto& [name, column] : series) {
        while (column.size() < times.size()) {
            column.append(Json::Value{});
        }
        body["series"][name] = column;
    }

    return body;
}

void RegisterHistory(clients::StorageClient& storage,
                     trantor::EventLoopThreadPool& blocking_pool) {
    drogon::app().registerHandler(
        "/api/history",
        [&storage, &blocking_pool](
            const drogon::HttpRequestPtr& request,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            const std::string collector_id = request->getParameter("collector_id");
            if (collector_id.empty()) {
                callback(JsonError(drogon::k400BadRequest, "collector_id is required"));
                return;
            }

            const std::optional<int64_t> from = ParseSeconds(request->getParameter("from"));
            const std::optional<int64_t> to = ParseSeconds(request->getParameter("to"));

            uint32_t limit = kDefaultHistoryLimit;
            if (std::optional<int64_t> requested = ParseSeconds(request->getParameter("limit"))) {
                limit = static_cast<uint32_t>(
                    std::min<int64_t>(std::max<int64_t>(*requested, 1), kMaxHistoryLimit));
            }

            blocking_pool.getNextLoop()->queueInLoop(
                [&storage, collector_id, from, to, limit,
                 callback = std::move(callback)]() mutable {
                    clients::StorageClient::History history =
                        storage.DataLoad(collector_id, from, to, limit);

                    if (!history.ok) {
                        callback(JsonError(HttpStatusFor(history.code),
                                           "storage-service: " + history.error));
                        return;
                    }

                    callback(drogon::HttpResponse::newHttpJsonResponse(
                        ToColumns(history.points)));
                });
        },
        {drogon::Get});
}

void RegisterConfigs(ConfigClient& config, trantor::EventLoopThreadPool& blocking_pool) {
    drogon::app().registerHandler(
        "/api/configs",
        [&config, &blocking_pool](
            const drogon::HttpRequestPtr&,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            blocking_pool.getNextLoop()->queueInLoop(
                [&config, callback = std::move(callback)]() mutable {
                    ConfigClient::ConfigList list = config.ListCollectors();
                    if (!list.ok) {
                        callback(ErrorFor(list, "config-service"));
                        return;
                    }

                    Json::Value body{Json::arrayValue};
                    for (const auto& collector_config : list.configs) {
                        body.append(ProtoToJson(collector_config));
                    }

                    callback(drogon::HttpResponse::newHttpJsonResponse(body));
                });
        },
        {drogon::Get});

    drogon::app().registerHandler(
        "/api/configs/{1}",
        [&config, &blocking_pool](
            const drogon::HttpRequestPtr& request,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
            std::string collector_id) {
            dcm::config::v1::CollectorConfig collector_config;

            google::protobuf::util::JsonParseOptions options;
            options.ignore_unknown_fields = true;

            const std::string body{request->getBody()};
            if (!google::protobuf::util::JsonStringToMessage(body, &collector_config, options)
                     .ok()) {
                callback(JsonError(drogon::k400BadRequest, "body is not a CollectorConfig"));
                return;
            }

            // The path names the collector; a body disagreeing with it would
            // otherwise create a second config under a different id.
            collector_config.set_collector_id(collector_id);

            // The version the editor started from travels inside the config it
            // was given, so a stale form is rejected instead of overwriting
            // what someone else saved meanwhile. A new config carries none.
            const int64_t expected_version = collector_config.version();

            blocking_pool.getNextLoop()->queueInLoop(
                [&config, collector_config, expected_version,
                 callback = std::move(callback)]() mutable {
                    ConfigClient::SaveResult result =
                        config.SaveConfig(collector_config, expected_version);

                    if (!result.ok) {
                        callback(ErrorFor(result, "config-service"));
                        return;
                    }

                    Json::Value body;
                    body["version"] = static_cast<Json::Int64>(result.version);

                    callback(drogon::HttpResponse::newHttpJsonResponse(body));
                });
        },
        {drogon::Put});

    drogon::app().registerHandler(
        "/api/configs/{1}",
        [&config, &blocking_pool](
            const drogon::HttpRequestPtr&,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
            std::string collector_id) {
            blocking_pool.getNextLoop()->queueInLoop(
                [&config, collector_id, callback = std::move(callback)]() mutable {
                    ConfigClient::Result result = config.DeleteCollector(collector_id);
                    if (!result.ok) {
                        callback(ErrorFor(result, "config-service"));
                        return;
                    }

                    drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpResponse();
                    response->setStatusCode(drogon::k204NoContent);

                    callback(response);
                });
        },
        {drogon::Delete});
}

}   //namespace

void RegisterHandlers(clients::StorageClient& storage, ConfigClient& config,
                      trantor::EventLoopThreadPool& blocking_pool) {
    RegisterHealth();
    RegisterCollectors(storage, blocking_pool);
    RegisterHistory(storage, blocking_pool);
    RegisterConfigs(config, blocking_pool);
}

}   //api
