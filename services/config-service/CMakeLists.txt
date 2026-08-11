cmake_minimum_required(VERSION 3.16)

project(config-service VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

# gRPC contracts shared by all services, generated at build time.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../proto ${CMAKE_BINARY_DIR}/proto)

file(GLOB_RECURSE CONFIG_SRC service/*.cpp service/*.h storage/*.cpp storage/*.h)

add_executable(config-service main.cpp ${CONFIG_SRC})

# Qt is deliberately absent: nothing here reuses the Qt-based layers of the
# monolith, and the service handles no event loop of its own.
target_include_directories(config-service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(config-service PRIVATE
    PkgConfig::PQXX
    Threads::Threads
    dcm_proto)
