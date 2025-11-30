# Test executable
add_executable(run_tests
    test_vehicle.cpp
    test_gameObject.cpp
    test_collision.cpp
    test_validation.cpp
)

# Add include directories
target_include_directories(run_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link against core library, audio library, Catch2, and threading library
target_link_libraries(run_tests PRIVATE
    core
    audio
    Catch2::Catch2WithMain
    Threads::Threads
)

# Ensure assets are copied before running tests
add_dependencies(run_tests copy_assets)

# Enable testing
enable_testing()

# Add test with proper generator expression for multi-config builds
# The working directory should be where the executable is located
add_test(NAME AllTests COMMAND run_tests)

# Set working directory to the location of the executable
# This works for both single-config (Makefile/Ninja) and multi-config (Visual Studio/Xcode) generators
set_tests_properties(AllTests PROPERTIES
    WORKING_DIRECTORY $<TARGET_FILE_DIR:run_tests>
)
