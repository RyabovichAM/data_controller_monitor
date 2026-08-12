cmake_minimum_required(VERSION 3.16)

project(collector VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Network SerialPort REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(RDKAFKA REQUIRED IMPORTED_TARGET rdkafka++)

# gRPC contracts shared by all services, generated at build time.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../proto ${CMAKE_BINARY_DIR}/proto)

file(GLOB_RECURSE COLLECTOR_SRC app/*.cpp app/*.h transfer/*.cpp transfer/*.h
                                kafka/*.cpp kafka/*.h config/*.cpp config/*.h)

add_executable(collector main.cpp ${COLLECTOR_SRC})

target_include_directories(collector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    app/
    transfer/
    kafka/
    config/)

target_link_libraries(collector PRIVATE
    Qt6::Core Qt6::Network Qt6::SerialPort
    PkgConfig::RDKAFKA
    dcm_proto)

# The mapping between the contract and the transfer layer is what breaks
# silently when either side renames a value, and it is pure — so it is the part
# worth a test target of its own. Debug only, like the tests of the monolith.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_package(GTest REQUIRED)

    file(GLOB_RECURSE COLLECTOR_TESTS_SRC tests/*.cpp)

    add_executable(collector_tests
        ${COLLECTOR_TESTS_SRC}
        config/config_mapping.cpp
        transfer/serial_transfer_domain.cpp
        transfer/tcpip_transfer_domain.cpp)

    target_include_directories(collector_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        app/
        transfer/
        config/)

    target_link_libraries(collector_tests PRIVATE
        Qt6::Core Qt6::Network Qt6::SerialPort
        dcm_proto
        GTest::gtest
        GTest::gtest_main)
endif()
