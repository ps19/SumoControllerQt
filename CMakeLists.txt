cmake_minimum_required(VERSION 3.16)
project(SumoControllerQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.2 COMPONENTS Core Gui Widgets Bluetooth REQUIRED)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/blemanager.cpp
)

set(HEADERS
    src/mainwindow.h
    src/blemanager.h
)

qt_add_executable(SumoControllerQt
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(SumoControllerQt PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Bluetooth
)

if(ANDROID)
    set_target_properties(SumoControllerQt PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )
endif()

install(TARGETS SumoControllerQt
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
