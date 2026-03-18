cmake_minimum_required(VERSION 3.10)
project(ParentProject)

set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

set(MODELS_DIR "${ASSETS_DIR}/models")
set(SHADERS_DIR "${ASSETS_DIR}/shaders")
set(SHAPES_DIR "${ASSETS_DIR}/shapes")
set(TEXTURES_DIR "${ASSETS_DIR}/textures")

# Глобальные пути к либам
set(GLFW_DIR "${EXTERNAL_DIR}/glfw")
set(ASSIMP_DIR "${EXTERNAL_DIR}/assimp")
set(GLHELPERS_DIR "${EXTERNAL_DIR}/OpenGLHelpers")
set(IMGUI_DIR "${EXTERNAL_DIR}/imgui")
set(GLM_DIR "${EXTERNAL_DIR}/glm")
#set(GLAD_DIR "${EXTERNAL_DIR}/glad")
set(IMPLOT_DIR "${EXTERNAL_DIR}/implot")
set(HTTPLIB_DIR "${EXTERNAL_DIR}/httplib")
set(JSON_DIR "${EXTERNAL_DIR}/json")
set(STB_DIR "${EXTERNAL_DIR}/stb")


# Собираем список файлов ImGui один раз для всех
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp 
)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Подключаем подпроекты
add_subdirectory(${ASSIMP_DIR} assimp_build)
add_subdirectory(MyWidgetEditor)
add_subdirectory(Drawer)
