# Create a library for input handling components
add_library(input
    input_handler.cpp
)

target_include_directories(input PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(input PUBLIC
    core
    threepp::threepp
)
