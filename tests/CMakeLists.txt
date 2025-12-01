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

# For multi-config generators, copy assets to the test executable directory
add_custom_command(TARGET run_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:run_tests>/assets
    COMMENT "Copying assets to test output directory"
    VERBATIM
)

# NOTE: enable_testing() is called in the main CMakeLists.txt AFTER this subdirectory
# This ensures the test executable is fully defined before the RUN_TESTS utility target is created

# Add the test - use TARGET_FILE to get the full path including config subdirectory
add_test(NAME AllTests COMMAND $<TARGET_FILE:run_tests>)

# For Visual Studio: Set working directory so test can find assets
if(MSVC OR CMAKE_GENERATOR MATCHES "Visual Studio")
    set_tests_properties(AllTests PROPERTIES
        WORKING_DIRECTORY $<TARGET_FILE_DIR:run_tests>
    )
endif()
