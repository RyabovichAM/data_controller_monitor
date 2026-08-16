cmake_minimum_required(VERSION 3.16)

project(backend VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Shared with the other services, built from source in every image. The
# contracts come first on purpose: Drogon's config leaves CMAKE_MODULE_PATH
# pointing at its own Find modules, and gRPC's config then fails to resolve
# c-ares through it.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../proto ${CMAKE_BINARY_DIR}/proto)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/kafka ${CMAKE_BINARY_DIR}/kafka)

find_package(Drogon CONFIG REQUIRED)

file(GLOB_RECURSE BACKEND_SRC api/*.cpp api/*.h clients/*.cpp clients/*.h
                              realtime/*.cpp realtime/*.h)

add_executable(backend main.cpp ${BACKEND_SRC})

target_include_directories(backend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Drogon brings trantor and jsoncpp along with it.
target_link_libraries(backend PRIVATE
    Drogon::Drogon
    dcm_kafka
    dcm_proto)
