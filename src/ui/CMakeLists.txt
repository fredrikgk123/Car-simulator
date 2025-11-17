# Create a library for UI components
add_library(ui STATIC
    imgui_layer.cpp
    imgui_context.cpp
)

target_include_directories(ui PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link against required libraries
# The imgui target (created in root CMakeLists.txt) provides ImGui implementation and GLFW headers
target_link_libraries(ui PUBLIC
    imgui         # Provides ImGui and GLFW headers
    threepp::threepp
    core
)


# Ensure consistent MSVC runtime library
if(MSVC)
    set_target_properties(ui PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

