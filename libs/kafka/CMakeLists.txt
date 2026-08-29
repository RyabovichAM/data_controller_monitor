cmake_minimum_required(VERSION 3.16)

project(dcm_kafka VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RDKAFKA REQUIRED IMPORTED_TARGET rdkafka++)

# Shared by every service that touches the topic: collector publishes,
# storage-service and backend consume. An identical copy per service is the
# kind that silently drifts apart.
add_library(dcm_kafka STATIC kafka_consumer.cpp kafka_producer.cpp)

target_include_directories(dcm_kafka PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(dcm_kafka PUBLIC PkgConfig::RDKAFKA)
