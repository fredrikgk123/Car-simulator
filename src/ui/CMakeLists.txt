# Create a library for UI components
add_library(ui
    uiManager.cpp
    uiManager.hpp
)

# Make headers available to other directories
target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core  # Add core module for vehicle.hpp
)

# Link against required libraries
target_link_libraries(ui PUBLIC
    threepp::threepp
    core  # Link against core library for Vehicle class
)
