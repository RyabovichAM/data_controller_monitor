cmake_minimum_required(VERSION 3.16)

project(storage-service VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(RDKAFKA REQUIRED IMPORTED_TARGET rdkafka++)

file(GLOB_RECURSE STORAGE_SRC data_storage/*.cpp data_storage/*.h kafka/*.cpp kafka/*.h)

add_executable(storage-service main.cpp ${STORAGE_SRC})

target_include_directories(storage-service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    data_storage/
    kafka/)

target_link_libraries(storage-service PRIVATE
    Qt6::Core
    PkgConfig::RDKAFKA)
