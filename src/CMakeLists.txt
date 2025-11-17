add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(input)

add_subdirectory(ui)
add_subdirectory(audio)

add_executable(bilsim main.cpp)
target_include_directories(bilsim PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link all required libraries
target_link_libraries(bilsim PRIVATE
    core
    graphics
    input
    ui
    audio
    threepp::threepp
    imgui  # Explicitly link imgui for all platforms
)

# On Windows MSVC, use /FORCE:MULTIPLE to resolve duplicate symbols if any,
# and ensure imgui symbols are available
if(MSVC)
    # Link the imgui library with /WHOLEARCHIVE using the library name
    target_link_libraries(bilsim PRIVATE "-WHOLEARCHIVE:imgui")
endif()


