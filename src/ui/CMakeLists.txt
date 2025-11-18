# ImGui sources to compile into the ui library
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Create a library for UI components with ImGui sources included
add_library(ui STATIC
    imgui_layer.cpp
    imgui_context.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(ui PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Ensure OpenGL is available and link against required libraries
find_package(OpenGL REQUIRED)

# Link against required libraries (threepp provides GLFW/OpenGL via its targets)
target_link_libraries(ui PUBLIC
    threepp::threepp
    core
    OpenGL::GL
)

# If a glfw target exists, link it as well (threepp may provide it depending on configuration)
if(TARGET glfw)
    target_link_libraries(ui PUBLIC glfw)
    message(STATUS "ui: Linked to glfw target")
endif()

# Ensure consistent MSVC runtime library
if(MSVC)
    set_target_properties(ui PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()