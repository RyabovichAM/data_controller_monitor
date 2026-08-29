cmake_minimum_required(VERSION 3.16)

# if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
#     set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel" CACHE STRING "" FORCE)
# endif()

project(data_controller_monitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets SerialPort Charts Network REQUIRED)

# The desktop application keeps its own copies of these layers: the ones under
# services/ are being rewritten without Qt, and the widgets here are built
# around QString, QDateTime, QJsonDocument and the Qt transports.
file(GLOB_RECURSE TRANSFER_SRC transfer/*.cpp transfer/*.h)
file(GLOB_RECURSE APP_SRC     app/*.cpp     app/*.h)
file(GLOB_RECURSE STORAGE_SRC data_storage/*.cpp data_storage/*.h)
file(GLOB_RECURSE WIDGETS_SRC widgets/*.cpp widgets/*.h)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TRANSFER_SRC}
        ${WIDGETS_SRC}
        ${APP_SRC}
        ${STORAGE_SRC}
)

qt_add_executable(data_controller_monitor
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_include_directories(data_controller_monitor PRIVATE
    app/
    transfer/
    data_storage/
    widgets/
    widgets/charts
    widgets/view_widget
    widgets/view_widget/component_object_widgets)
target_link_libraries(data_controller_monitor PRIVATE Qt6::Widgets Qt6::SerialPort Qt6::Charts Qt6::Network)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(data_controller_monitor)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS})

  file(GLOB_RECURSE TESTS_SRC tests/*.cpp tests/*.h)

  qt_add_executable(data_controller_monitor_tests
    ${TRANSFER_SRC}
    ${WIDGETS_SRC}
    ${APP_SRC}
    ${STORAGE_SRC}
    ${TESTS_SRC}
  )

  target_include_directories(data_controller_monitor_tests PRIVATE
      app/
      transfer/
      data_storage/
      widgets/
      widgets/charts
      widgets/view_widget
      widgets/view_widget/component_object_widgets)
  target_link_libraries(data_controller_monitor_tests PRIVATE
    Qt6::Widgets Qt6::SerialPort Qt6::Charts Qt6::Network
    GTest::GTest
    pthread)

endif()
