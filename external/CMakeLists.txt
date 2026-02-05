# External libraries configuration

# ImGui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Drawer/lib/imgui" CACHE PATH "ImGui directory")
if(NOT EXISTS "${IMGUI_DIR}")
    set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../MyWidgetEditor/lib/imgui" CACHE PATH "ImGui directory")
endif()

# GLFW
set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Drawer/lib/glfw" CACHE PATH "GLFW directory")
if(NOT EXISTS "${GLFW_DIR}")
    set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../MyWidgetEditor/lib/glfw" CACHE PATH "GLFW directory")
endif()

# JSON (nlohmann)
set(JSON_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Drawer/lib" CACHE PATH "JSON include directory")
if(NOT EXISTS "${JSON_INCLUDE_DIR}/json.hpp")
    set(JSON_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../MyWidgetEditor/lib" CACHE PATH "JSON include directory")
endif()

# httplib
set(HTTPLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Drawer/lib" CACHE PATH "httplib include directory")
if(NOT EXISTS "${HTTPLIB_INCLUDE_DIR}/httplib.h")
    set(HTTPLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../MyWidgetEditor/lib" CACHE PATH "httplib include directory")
endif()

# Create ImGui library target
add_library(imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
    "${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp"
)

target_include_directories(imgui PUBLIC
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
    "${GLFW_DIR}/include"
)

# GLFW library
if(WIN32)
    set(GLFW_LIB_DIR "${GLFW_DIR}/lib-vc2022")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64-bit
        set(GLFW_LIB "${GLFW_LIB_DIR}/glfw3_mt.lib")
    else()
        # 32-bit
        set(GLFW_LIB "${GLFW_LIB_DIR}/glfw3_mt.lib")
    endif()
    
    add_library(glfw STATIC IMPORTED)
    set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION "${GLFW_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLFW_DIR}/include"
    )
    
    # Export GLFW directory for DLL copying in child projects
    set(GLFW_LIB_DIR "${GLFW_LIB_DIR}" PARENT_SCOPE)
    
    # Note: DLL will be copied by each executable that links to glfw
    # This is handled in the project-specific CMakeLists.txt files
else()
    # For non-Windows, you might need to find GLFW via find_package
    find_package(glfw3 REQUIRED)
endif()

# JSON header-only library
add_library(json INTERFACE)
target_include_directories(json INTERFACE "${JSON_INCLUDE_DIR}")

# httplib header-only library
add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE "${HTTPLIB_INCLUDE_DIR}")

# Export targets
set(EXTERNAL_LIBS imgui glfw json httplib PARENT_SCOPE)
