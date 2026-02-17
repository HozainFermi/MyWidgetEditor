cmake_minimum_required(VERSION 3.10)
project(ParentProject)

# Глобальные пути к либам
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(GLFW_DIR "${EXTERNAL_DIR}/glfw")
set(IMGUI_DIR "${EXTERNAL_DIR}/imgui")
set(GLM_DIR "${EXTERNAL_DIR}/glm")
#set(GLAD_DIR "${EXTERNAL_DIR}/glad")
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

# Подключаем подпроекты
add_subdirectory(MyWidgetEditor)
add_subdirectory(Drawer)
