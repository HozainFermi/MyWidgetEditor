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


# ����� �� ����� widgets
file(GLOB WIDGETS_SOURCES
    "src/widgets/*.cpp"
    "src/widgets/*.c"
)

file(GLOB WIDGETS_HEADERS
    "src/widgets/*.hpp"
    "src/widgets/*.h"
)

# ����� �� ����� ui
file(GLOB UI_SOURCES
    "src/ui/*.cpp"
    "src/ui/*.c"
)

file(GLOB UI_HEADERS
    "src/ui/*.hpp"
    "src/ui/*.h"
)

# ����� �� ����� styles
file(GLOB STYLES_SOURCES
    "src/styles/*.cpp"
    "src/styles/*.c"
)

file(GLOB STYLES_HEADERS
    "src/styles/*.hpp"
    "src/styles/*.h"
)

# ����� �� ����� managers
file(GLOB MANAGERS_SOURCES
    "src/managers/*.cpp"
    "src/managers/*.c"
)

file(GLOB MANAGERS_HEADERS
    "src/managers/*.hpp"
    "src/managers/*.h"
)

# ����� �� ����� include
file(GLOB INCLUDE_HEADERS
    "src/include/*.cpp"
    "src/include/*.c"
)

# ����� �� ����� Helpers
file(GLOB GLHELPERS_SOURCES
    "${GLHELPERS_DIR}/*.cpp"
)
file(GLOB GLHELPERS_HEADERS
    "${GLHELPERS_DIR}/*.hpp"
    "${GLHELPERS_DIR}/*.h"
)

# ���������� ���
set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${HELPERS_SOURCES}
    ${WIDGETS_SOURCES}
    ${UI_SOURCES}
    ${STYLES_SOURCES}
    ${MANAGERS_SOURCES}
    ${GLHELPERS_SOURCES}
    
)

set(ALL_HEADERS
    ${HELPERS_HEADERS}
    ${WIDGETS_HEADERS}
    ${UI_HEADERS}
    ${STYLES_HEADERS}
    ${MANAGERS_HEADERS}
    ${GLHELPERS_HEADERS}
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
    ${GLHELPERS_DIR}  
    ${GLFW_DIR}/include
    ${ASSIMP_DIR}/include
    ${HTTPLIB_DIR}
    ${JSON_DIR}
    ${STB_DIR}    
)

target_link_libraries(MyWidgetEditor PRIVATE 
    "${GLFW_DIR}/lib-vc2022/glfw3.lib"     
    opengl32
    assimp
)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # �������� ��� .lib, ����� �� �������� � .dll


# Copy configs directory to output
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/configs 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
     FILES_MATCHING PATTERN "*.json")

target_compile_definitions(MyWidgetEditor PRIVATE 
    PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
