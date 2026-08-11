#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "service/config_service_impl.h"
#include "service/watch_registry.h"
#include "storage/postgres_config_repository.h"

namespace {

std::atomic_bool stop_requested{false};

void RequestStop(int /*signal*/) {
    stop_requested = true;
}

std::string Env(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return (value && *value) ? std::string{value} : fallback;
}

// The database may still be starting: the compose healthcheck covers the usual
// case, but a service that dies on a slow postgres would restart-loop for
// nothing.
std::unique_ptr<config::PostgresConfigRepository> Connect(const std::string& dsn,
                                                          int attempts) {
    for (int attempt = 1;; ++attempt) {
        try {
            return std::make_unique<config::PostgresConfigRepository>(dsn);
        } catch (const std::exception& error) {
            if (attempt >= attempts) {
                throw;
            }
            std::cerr << "[config-service] postgres: " << error.what()
                      << " (attempt " << attempt << "/" << attempts << ")" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds{2});
        }
    }
}

}   //namespace

int main() {
    const std::string address = Env("CONFIG_SERVICE_ADDRESS", "0.0.0.0:50051");
    const std::string dsn =
        Env("POSTGRES_DSN", "postgresql://dcm:dcm@postgres:5432/dcm_config");

    std::signal(SIGINT, RequestStop);
    std::signal(SIGTERM, RequestStop);

    std::unique_ptr<config::PostgresConfigRepository> repository;
    try {
        repository = Connect(dsn, 30);
    } catch (const std::exception& error) {
        std::cerr << "[config-service] cannot connect to postgres: " << error.what()
                  << std::endl;
        return 1;
    }

    config::WatchRegistry watchers;
    config::ConfigServiceImpl service{*repository, watchers};

    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};
    if (!server) {
        std::cerr << "[config-service] cannot listen on " << address << std::endl;
        return 1;
    }

    std::cout << "[config-service] listening on " << address << std::endl;

    // Shutdown is called from a plain thread: a signal handler may only touch
    // the flag. The deadline cancels WatchConfig streams, which would otherwise
    // hold the shutdown for as long as their clients stay connected.
    std::thread stopper{[&server]() {
        while (!stop_requested) {
            std::this_thread::sleep_for(std::chrono::milliseconds{200});
        }
        server->Shutdown(std::chrono::system_clock::now() + std::chrono::seconds{2});
    }};

    server->Wait();

    stop_requested = true;
    stopper.join();

    return 0;
}
