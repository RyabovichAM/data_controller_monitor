cmake_minimum_required(VERSION 3.16)

project(data_controller_monitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialPort)

file(GLOB_RECURSE TRANSFER_SRC transfer/*.cpp transfer/*.h)
file(GLOB_RECURSE WIDGETS_SRC widgets/*.cpp widget/*.h)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TRANSFER_SRC}
        ${WIDGETS_SRC}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(data_controller_monitor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
        add_executable(data_controller_monitor
            ${PROJECT_SOURCES}
        )
endif()

target_include_directories(data_controller_monitor PRIVATE widgets/ transfer/)
target_link_libraries(data_controller_monitor PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::SerialPort)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.data_controller_monitor)
endif()
set_target_properties(data_controller_monitor PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS data_controller_monitor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

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
    ${TESTS_SRC}
  )

  target_include_directories(data_controller_monitor_tests PRIVATE transfer/ widgets/)
  target_link_libraries(data_controller_monitor_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::SerialPort
    GTest::GTest
    pthread)

endif()
