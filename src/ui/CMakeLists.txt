# Create a library for UI components
add_library(ui STATIC
    imgui_layer.cpp
    imgui_context.cpp
)

target_include_directories(ui PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link against required libraries
# imgui library contains all ImGui implementation and provides GLFW/OpenGL
target_link_libraries(ui PUBLIC
    threepp::threepp
    core
    imgui  # ImGui library built in root CMakeLists.txt
)

# Ensure consistent MSVC runtime library
if(MSVC)
    set_target_properties(ui PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

