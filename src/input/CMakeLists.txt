# Create a library for input handling components
add_library(input
    inputHandler.cpp
    inputHandler.hpp
)

# Make headers available to other directories
target_include_directories(input PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against the core library for Vehicle class access
target_link_libraries(input PUBLIC
    core
    threepp::threepp
)

