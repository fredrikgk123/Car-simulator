add_library(core vehicle.cpp vehicle.hpp)
target_include_directories(core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
