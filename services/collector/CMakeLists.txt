cmake_minimum_required(VERSION 3.16)

project(collector VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Network SerialPort REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(RDKAFKA REQUIRED IMPORTED_TARGET rdkafka++)

file(GLOB_RECURSE COLLECTOR_SRC app/*.cpp app/*.h transfer/*.cpp transfer/*.h kafka/*.cpp kafka/*.h)

add_executable(collector main.cpp ${COLLECTOR_SRC})

target_include_directories(collector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    app/
    transfer/
    kafka/)

target_link_libraries(collector PRIVATE
    Qt6::Core Qt6::Network Qt6::SerialPort
    PkgConfig::RDKAFKA)
