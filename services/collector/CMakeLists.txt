cmake_minimum_required(VERSION 3.16)

project(collector VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)

# Standalone asio, not the Boost one: the same library without the rest of
# Boost in the dependencies. Header-only, so nothing of it reaches the image.
add_compile_definitions(ASIO_STANDALONE ASIO_NO_DEPRECATED)

# Shared with the other services, built from source in every image.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../proto ${CMAKE_BINARY_DIR}/proto)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/kafka ${CMAKE_BINARY_DIR}/kafka)

file(GLOB_RECURSE COLLECTOR_SRC app/*.cpp app/*.h transfer/*.cpp transfer/*.h
                                config/*.cpp config/*.h)

add_executable(collector main.cpp ${COLLECTOR_SRC})

# No Qt: the transports are sockets and termios, the loop is poll(), and the
# desktop application keeps its own Qt copies of these layers at the repository
# root.
target_include_directories(collector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    app/
    transfer/
    config/)

target_link_libraries(collector PRIVATE
    nlohmann_json::nlohmann_json
    Threads::Threads
    dcm_kafka
    dcm_proto)

# The mapping between the contract and the transfer layer is what breaks
# silently when either side renames a value, and the framer is where a stream of
# bytes becomes messages. Debug only, like the tests of the monolith.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_package(GTest REQUIRED)

    file(GLOB_RECURSE COLLECTOR_TESTS_SRC tests/*.cpp)

    add_executable(collector_tests
        ${COLLECTOR_TESTS_SRC}
        config/config_mapping.cpp
        transfer/json_framer.cpp
        transfer/transfer_domain.cpp
        transfer/serial_transfer.cpp
        transfer/serial_transfer_domain.cpp
        transfer/tcpip_transfer_domain.cpp)

    target_include_directories(collector_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        app/
        transfer/
        config/)

    target_link_libraries(collector_tests PRIVATE
        dcm_proto
        Threads::Threads
        GTest::gtest
        GTest::gtest_main
        util)   # openpty, for the serial test
endif()
