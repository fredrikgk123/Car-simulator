# Create a library for graphics components
add_library(graphics
    game_object_renderer.cpp
    vehicle_renderer.cpp
    powerup_renderer.cpp
    obstacle_renderer.cpp
    scene_manager.cpp
)

target_include_directories(graphics PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(graphics PUBLIC
    core
    threepp::threepp
)
