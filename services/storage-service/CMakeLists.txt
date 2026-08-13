cmake_minimum_required(VERSION 3.16)

project(storage-service VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RDKAFKA REQUIRED IMPORTED_TARGET rdkafka++)

# gRPC contracts shared by all services, generated at build time.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../proto ${CMAKE_BINARY_DIR}/proto)

file(GLOB_RECURSE STORAGE_SRC data_storage/*.cpp data_storage/*.h kafka/*.cpp kafka/*.h)

add_executable(storage-service main.cpp ${STORAGE_SRC})

# No Qt: nothing here is a widget and libQt6Core would drag libicu into the
# image. The desktop application keeps its own copy of data_storage at the
# repository root.
target_include_directories(storage-service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    data_storage/
    kafka/)

target_link_libraries(storage-service PRIVATE
    PkgConfig::RDKAFKA
    dcm_proto)

# DataLoad is the part with dates, time zones and file walking in it, and
# nothing calls it yet — until the gRPC server does, these tests are what keeps
# it honest. Debug only, like the tests of the monolith.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_package(GTest REQUIRED)

    file(GLOB_RECURSE STORAGE_TESTS_SRC tests/*.cpp)
    file(GLOB DATA_STORAGE_SRC data_storage/*.cpp)

    add_executable(storage_service_tests ${STORAGE_TESTS_SRC} ${DATA_STORAGE_SRC})

    target_include_directories(storage_service_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        data_storage/)

    target_link_libraries(storage_service_tests PRIVATE
        GTest::gtest
        GTest::gtest_main)
endif()
