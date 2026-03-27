cmake_minimum_required(VERSION 3.10)
project(Drawer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MAIN_SOURCES
    main.cpp
    glad-impl.cpp
    stb.cpp
)

# Implot
file(GLOB IMPLOT_FILES
    "${IMPLOT_DIR}/implot.cpp"
    "${IMPLOT_DIR}/implot.h"
    "${IMPLOT_DIR}/implot_items.cpp"
    "${IMPLOT_DIR}/implot_internal.h"
)
file(GLOB_RECURSE RUNTIME_SOURCES "runtime_widgets/*.*")

file(GLOB GLHELPERS_SOURCES
    "${GLHELPERS_DIR}/*.cpp"
)

file(GLOB GLHELPERS_HEADERS
    "${GLHELPERS_DIR}/*.hpp"
    "${GLHELPERS_DIR}/*.h"
)

set(MANAGERS_FILES managers/RuntimeWidgetManager.cpp managers/RuntimeWidgetManager.h managers/RuntimeWidgetFactory.h)
set(UI_FILES ui/MainWindowLayout.cpp ui/MainWindowLayout.h ui/RuntimeWindowProperties.h)
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
set(BBCODE_FILE "${BB_DIR}/BBCodeParser.h" "${BB_DIR}/IconText.h")

find_package(OpenSSL REQUIRED)


add_executable(Drawer 
    ${MAIN_SOURCES}
    ${MANAGERS_FILES} 
    ${UI_FILES} 
    ${RUNTIME_SOURCES}
    ${IMGUI_SOURCES}
    ${IMPLOT_FILES}
    ${BBCODE_FILE}
    ${GLHELPERS_SOURCES}
    ${GLHELPERS_HEADERS}
)

target_include_directories(Drawer PRIVATE 
    
    "${CMAKE_CURRENT_SOURCE_DIR}/managers"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    
    ${EXTERNAL_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUI_DIR}/misc/cpp
    ${GLM_DIR}
    ${GLHELPERS_DIR}
    ${GLFW_DIR}/include
    ${GLAD_DIR}
    ${HTTPLIB_DIR}
    ${JSON_DIR}
    ${STB_DIR}  
    ${BB_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

target_compile_definitions(Drawer PRIVATE 
    PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

# 
target_link_libraries(Drawer PRIVATE 
     "${GLFW_DIR}/lib-vc2022/glfw3.lib" 
    opengl32
    OpenSSL::SSL 
    OpenSSL::Crypto
    assimp
)
