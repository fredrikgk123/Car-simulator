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

# Enable testing
enable_testing()

# Add test - but don't let Visual Studio run it automatically during build
add_test(NAME AllTests COMMAND run_tests)

# Disable automatic test running on Visual Studio builds
# This prevents the circular dependency where VS tries to run tests before building the exe
if(MSVC OR CMAKE_GENERATOR MATCHES "Visual Studio")
    set_tests_properties(AllTests PROPERTIES
        DISABLED OFF
    )
    # Ensure run_tests is a normal executable target, not a test runner
    set_target_properties(run_tests PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:run_tests>"
    )
endif()

# Instead of using generator expressions in WORKING_DIRECTORY, we'll rely on:
# 1. The test executable running from wherever CTest invokes it
# 2. Assets being copied to the test output directory via POST_BUILD command above
# 3. The executable being in the same directory as the assets folder
