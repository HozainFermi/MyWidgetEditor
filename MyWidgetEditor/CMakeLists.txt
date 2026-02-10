cmake_minimum_required(VERSION 3.10)

project(MyWidgetEditor)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Main Source files
set(MAIN_SOURCES
    src/main.cpp
    src/glad-impl.cpp
    src/stb.cpp
)
# Файлы из папки Helpers
file(GLOB HELPERS_SOURCES
    "src/OpenGLHelpers/*.cpp"
    "src/OpenGLHelpers/*.c"
)

file(GLOB HELPERS_HEADERS
    "src/OpenGLHelpers/*.hpp"
    "src/OpenGLHelpers/*.h"
)

# Файлы из папки widgets
file(GLOB WIDGETS_SOURCES
    "src/widgets/*.cpp"
    "src/widgets/*.c"
)

file(GLOB WIDGETS_HEADERS
    "src/widgets/*.hpp"
    "src/widgets/*.h"
)

# Файлы из папки ui
file(GLOB UI_SOURCES
    "src/ui/*.cpp"
    "src/ui/*.c"
)

file(GLOB UI_HEADERS
    "src/ui/*.hpp"
    "src/ui/*.h"
)

# Файлы из папки styles
file(GLOB STYLES_SOURCES
    "src/styles/*.cpp"
    "src/styles/*.c"
)

file(GLOB STYLES_HEADERS
    "src/styles/*.hpp"
    "src/styles/*.h"
)

# Файлы из папки managers
file(GLOB MANAGERS_SOURCES
    "src/managers/*.cpp"
    "src/managers/*.c"
)

file(GLOB MANAGERS_HEADERS
    "src/managers/*.hpp"
    "src/managers/*.h"
)

# Файлы из папки include
file(GLOB INCLUDE_HEADERS
    "src/include/*.cpp"
    "src/include/*.c"
)


# Объединяем все
set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${HELPERS_SOURCES}
    ${WIDGETS_SOURCES}
    ${UI_SOURCES}
    ${STYLES_SOURCES}
    ${MANAGERS_SOURCES}
    
)

set(ALL_HEADERS
    ${HELPERS_HEADERS}
    ${WIDGETS_HEADERS}
    ${UI_HEADERS}
    ${STYLES_HEADERS}
    ${MANAGERS_HEADERS}
)

# Create executable
add_executable(MyWidgetEditor ${ALL_SOURCES} ${ALL_HEADERS} ${IMGUI_SOURCES})

# Include directories
target_include_directories(MyWidgetEditor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/managers"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OpenGLHelpers"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/styles"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets"

    ${EXTERNAL_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUI_DIR}/misc/cpp
    ${GLM_DIR}
    ${GLFW_DIR}/include
    #${GLAD_DIR}
    ${HTTPLIB_DIR}
    ${JSON_DIR}
    ${STB_DIR}    
)

target_link_libraries(MyWidgetEditor PRIVATE 
    "${GLFW_DIR}/lib-vc2022/glfw3.lib" 
    opengl32
)

# Copy configs directory to output
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/configs 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
     FILES_MATCHING PATTERN "*.json")

