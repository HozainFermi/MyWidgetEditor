cmake_minimum_required(VERSION 3.10)
project(WidgetCore)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Общий код редактора и Drawer: виджеты, конфиг, связи, а также ImGui/ImPlot/OpenGL-хелперы,
# которые раньше компилировались в каждом проекте отдельно.
file(GLOB CORE_FILES CONFIGURE_DEPENDS
    "core/*.cpp" "core/*.h"
    "widgets/*.cpp" "widgets/*.h"
    "nodes/*.cpp" "nodes/*.h"
)

file(GLOB GLHELPERS_FILES CONFIGURE_DEPENDS
    "${GLHELPERS_DIR}/*.cpp"
    "${GLHELPERS_DIR}/*.hpp"
    "${GLHELPERS_DIR}/*.h"
)

set(IMPLOT_FILES
    "${IMPLOT_DIR}/implot.cpp"
    "${IMPLOT_DIR}/implot_items.cpp"
)

set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
find_package(OpenSSL REQUIRED)

add_library(WidgetCore STATIC
    ${CORE_FILES}
    ${GLHELPERS_FILES}
    ${IMGUI_SOURCES}
    ${IMPLOT_FILES}
    "${BB_DIR}/BBCodeParser.h"
    "${BB_DIR}/IconText.h"
)

target_include_directories(WidgetCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
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
    ${BB_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

get_directory_property(PARENT_DIR PARENT_DIRECTORY)
target_compile_definitions(WidgetCore PUBLIC
    ASSETS_SOURCE_DIR="${PARENT_DIR}/assets"
)

target_compile_options(WidgetCore PRIVATE ${PROJECT_UTF8_FLAG})

target_link_libraries(WidgetCore PUBLIC
    "${GLFW_DIR}/lib-vc2022/glfw3.lib"
    opengl32
    assimp
    OpenSSL::SSL
    OpenSSL::Crypto
)
