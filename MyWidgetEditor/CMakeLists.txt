cmake_minimum_required(VERSION 3.10)

project(MyWidgetEditor)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Main Source files
set(MAIN_SOURCES
    src/main.cpp
)


# ui
file(GLOB UI_SOURCES
    "src/ui/*.cpp"
    "src/ui/*.c"
)

file(GLOB UI_HEADERS
    "src/ui/*.hpp"
    "src/ui/*.h"
)

#  styles
file(GLOB STYLES_SOURCES
    "src/styles/*.cpp"
    "src/styles/*.c"
)

file(GLOB STYLES_HEADERS
    "src/styles/*.hpp"
    "src/styles/*.h"
)

#  managers
file(GLOB MANAGERS_SOURCES
    "src/managers/*.cpp"
    "src/managers/*.c"
)

file(GLOB MANAGERS_HEADERS
    "src/managers/*.hpp"
    "src/managers/*.h"
)

# include
file(GLOB INCLUDE_HEADERS
    "src/include/*.cpp"
    "src/include/*.c"
)

# 
set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${UI_SOURCES}
    ${STYLES_SOURCES}
    ${MANAGERS_SOURCES}
)

set(ALL_HEADERS
    ${UI_HEADERS}
    ${STYLES_HEADERS}
    ${MANAGERS_HEADERS}
)

# Create executable
add_executable(MyWidgetEditor ${ALL_SOURCES} ${ALL_HEADERS})

# Include directories
target_include_directories(MyWidgetEditor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/managers"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/styles"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui"

)

target_compile_options(MyWidgetEditor PRIVATE ${PROJECT_UTF8_FLAG})

# Виджеты, ImGui, ImPlot, OpenGL-хелперы и сторонние либы приходят из WidgetCore
target_link_libraries(MyWidgetEditor PRIVATE WidgetCore)


# Copy configs directory to output
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/configs 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
     FILES_MATCHING PATTERN "*.json")

     

add_definitions(-DASSETS_PATH="${ASSETS_PATH}/")

get_directory_property(PARENT_DIR PARENT_DIRECTORY)

set(ICONS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/icons")

target_compile_definitions(MyWidgetEditor PRIVATE 
    PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    ICONS_DIR="${ICONS_PATH}"

)
