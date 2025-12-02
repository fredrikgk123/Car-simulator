add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(input)

add_subdirectory(ui)
add_subdirectory(audio)

add_executable(carsimulator main.cpp)
target_include_directories(carsimulator PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link all required libraries
# ImGui is now compiled directly into the ui library
target_link_libraries(carsimulator PRIVATE
    core
    graphics
    input
    ui
    audio
    threepp::threepp
)
