#include "api_handlers.h"

#include <utility>

#include <drogon/drogon.h>

#include "realtime/realtime_hub.h"

namespace api {

namespace {

drogon::HttpResponsePtr JsonError(drogon::HttpStatusCode code, const std::string& message) {
    Json::Value body;
    body["error"] = message;

    drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpJsonResponse(body);
    response->setStatusCode(code);

    return response;
}

}   //namespace

void RegisterHandlers(clients::StorageClient& storage,
                      trantor::EventLoopThreadPool& blocking_pool) {
    // Cheap enough to answer on the loop: it touches nothing but a mutex.
    drogon::app().registerHandler(
        "/api/health",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            Json::Value body;
            body["status"] = "ok";
            body["ws_connections"] = static_cast<Json::UInt64>(
                realtime::Hub().ConnectionCount());

            callback(drogon::HttpResponse::newHttpJsonResponse(body));
        },
        {drogon::Get});

    // Collectors that have data in storage — not the same list as the one
    // config-service keeps, which holds the configured ones.
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

}   //api
