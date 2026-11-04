cmake_minimum_required(VERSION 3.10)
project(Drawer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Drawer
    main.cpp
    managers/RuntimeWidgetManager.cpp
    managers/RuntimeWidgetManager.h
    ui/MainWindowLayout.cpp
    ui/MainWindowLayout.h
    ui/WindowSettings.h
)

target_include_directories(Drawer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/managers"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)

target_compile_definitions(Drawer PRIVATE
    PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_options(Drawer PRIVATE ${PROJECT_UTF8_FLAG})

# Виджеты, ImGui, ImPlot, OpenGL-хелперы и сторонние либы приходят из WidgetCore
target_link_libraries(Drawer PRIVATE WidgetCore)
