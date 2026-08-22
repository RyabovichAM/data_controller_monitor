#include "api_handlers.h"

#include <memory>
#include <string>
#include <utility>

#include <drogon/drogon.h>
#include <google/protobuf/util/json_util.h>

#include "realtime/realtime_hub.h"

namespace api {

namespace {

using clients::ConfigClient;

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

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader{builder.newCharReader()};

    Json::Value value;
    std::string errors;
    reader->parse(text.data(), text.data() + text.size(), &value, &errors);

    return value;
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
    RegisterConfigs(config, blocking_pool);
}

}   //api
