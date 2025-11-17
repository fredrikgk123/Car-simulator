add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(input)

add_subdirectory(ui)
add_subdirectory(audio)

add_executable(bilsim main.cpp)
target_include_directories(bilsim PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link all required libraries
# On Windows, we need to explicitly link imgui because MSVC/MinGW don't
# automatically resolve symbols from transitive static library dependencies
target_link_libraries(bilsim PRIVATE
    core
    graphics
    input
    ui
    audio
    threepp::threepp
    imgui  # Explicitly link imgui (ui also links it, but we need it here for Windows)
)

# On MSVC, use /WHOLEARCHIVE to ensure ALL symbols from imgui are included
# This forces the linker to pull in all object files from imgui.lib, not just
# the ones directly referenced by bilsim.exe
if(MSVC)
    target_link_options(bilsim PRIVATE
        "/WHOLEARCHIVE:$<TARGET_FILE:imgui>"
    )
endif()

