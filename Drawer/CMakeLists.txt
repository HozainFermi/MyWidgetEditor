cmake_minimum_required(VERSION 3.10)

project(Drawer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    main.cpp
    managers/RuntimeWidgetManager.cpp
    runtime_widgets/Widget.cpp
    runtime_widgets/TextWidget.cpp
    runtime_widgets/InputTextWidget.cpp
    runtime_widgets/TableWidget.cpp
    ui/MainWindowLayout.cpp
)

# Header files
set(HEADERS
    managers/RuntimeWidgetFactory.h
    managers/RuntimeWidgetManager.h
    runtime_widgets/Widget.h
    runtime_widgets/TextWidget.h
    runtime_widgets/InputTextWidget.h
    runtime_widgets/TableWidget.h
    runtime_widgets/httplib_config.h
    ui/MainWindowLayout.h
    ui/RuntimeWindowProperties.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/managers
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime_widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
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
        CPPHTTPLIB_OPENSSL_SUPPORT
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
    
    # Copy GLFW DLL to output directory
    # Use the GLFW_LIB_DIR variable set in external/CMakeLists.txt
    if(DEFINED GLFW_LIB_DIR)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLFW_LIB_DIR}/glfw3.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    endif()
endif()
