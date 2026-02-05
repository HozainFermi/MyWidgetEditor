cmake_minimum_required(VERSION 3.10)

project(MyWidgetEditor)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.cpp
    src/managers/WidgetManager.cpp
    src/widgets/Widget.cpp
    src/widgets/TextWidget.cpp
    src/widgets/InputTextWidget.cpp
    src/widgets/TableWidget.cpp
    src/ui/EditorMainWindowLayout.cpp
)

# Header files
set(HEADERS
    src/include/independent_launcher.h
    src/managers/WidgetFactory.h
    src/managers/WidgetManager.h
    src/widgets/Widget.h
    src/widgets/TextWidget.h
    src/widgets/InputTextWidget.h
    src/widgets/TableWidget.h
    src/ui/EditorMainWindowLayout.h
    src/ui/RuntimeWindowProperties.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

# Link external libraries (from external/CMakeLists.txt)
target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui
    glfw
    json
    httplib
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        GL_SILENCE_DEPRECATION
    )
    
    # Link Windows libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        opengl32
    )
    
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
            /W4
            /permissive-
        )
    endif()
endif()

# Copy configs directory to output
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/configs 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
     FILES_MATCHING PATTERN "*.json")

# Copy GLFW DLL to output directory
# Use the GLFW_LIB_DIR variable set in external/CMakeLists.txt
if(WIN32 AND DEFINED GLFW_LIB_DIR)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLFW_LIB_DIR}/glfw3.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
