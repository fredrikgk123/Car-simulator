add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(input)

add_subdirectory(ui)
add_subdirectory(audio)

add_executable(bilsim main.cpp)
target_include_directories(bilsim PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Ensure imgui is built before bilsim
if(TARGET imgui)
    add_dependencies(bilsim imgui)
endif()

# Link all required libraries
target_link_libraries(bilsim PRIVATE
    imgui  # Link imgui FIRST
    core
    graphics
    input
    ui
    audio
    threepp::threepp
)

# On MSVC, use linker flags to force inclusion of all ImGui symbols
if(MSVC)
    # Get the output file of imgui library
    target_link_options(bilsim PRIVATE "/WHOLEARCHIVE:imgui.lib")
endif()

