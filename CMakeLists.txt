cmake_minimum_required(VERSION 3.16)

project(DrawingHelper VERSION 0.0.6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/imagecanvas.cpp
    src/screenselector.cpp
)

set(PROJECT_HEADERS
    src/mainwindow.h
    src/imagecanvas.h
    src/screenselector.h
    src/version.h
)

add_executable(DrawingHelper
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

target_link_libraries(DrawingHelper PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

if(WIN32)
    set_target_properties(DrawingHelper PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
