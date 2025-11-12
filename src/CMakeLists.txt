add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(input)

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.89.8
)
FetchContent_MakeAvailable(imgui)

# Build a small imgui static library from the fetched ImGui sources
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
# If your build provides a glfw target, link it here (uncomment if available)
# target_link_libraries(imgui PUBLIC glfw)

add_subdirectory(ui)
add_subdirectory(audio)

add_executable(bilsim main.cpp)
target_link_libraries(bilsim PRIVATE
        core
        graphics
        input
        ui
        audio
        imgui
        threepp::threepp
)