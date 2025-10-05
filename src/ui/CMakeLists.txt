# Create a library for UI components
add_library(ui
    uiManager.cpp
    uiManager.hpp
)

# Make headers available to other directories
target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against required libraries
target_link_libraries(ui PUBLIC
    threepp::threepp
)

