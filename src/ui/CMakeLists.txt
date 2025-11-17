# Create a library for UI components
add_library(ui STATIC
    imgui_layer.cpp
    imgui_context.cpp
)

target_include_directories(ui PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# If FetchContent provided imgui, add its source dir to includes so <imgui.h> resolves
if(DEFINED imgui_SOURCE_DIR)
    target_include_directories(ui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)

    # Compile ImGui sources directly into ui library
    target_sources(ui PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
endif()

# Link against required libraries (ImGui implementation is compiled into ui directly)
target_link_libraries(ui PUBLIC
    threepp::threepp
    core  # Link against core library for Vehicle class
)

# If ImGui was compiled into ui, link OpenGL (glfw comes from threepp)
if(DEFINED imgui_SOURCE_DIR)
    find_package(OpenGL REQUIRED)
    target_link_libraries(ui PUBLIC OpenGL::GL)
endif()

# Ensure consistent MSVC runtime library
if(MSVC)
    set_target_properties(ui PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

