add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(input)

add_subdirectory(ui)
add_subdirectory(audio)

add_executable(bilsim main.cpp)
target_include_directories(bilsim PRIVATE ${CMAKE_SOURCE_DIR}/include)

# On Windows, we need to explicitly link imgui even though it's already
# linked through ui, because Windows linkers don't automatically resolve symbols from
# transitive static library dependencies the same way Unix linkers do
if(WIN32)
    target_link_libraries(bilsim PRIVATE
            core
            graphics
            input
            ui
            audio
            threepp::threepp
            imgui              # <-- ensure imgui is explicitly linked on Windows
    )

    # Optional: for MSVC, force whole-archive on the built imgui .lib if required
    if(MSVC)
        target_link_options(bilsim PRIVATE "/WHOLEARCHIVE:$<TARGET_FILE:imgui>")
    endif()
else()
    target_link_libraries(bilsim PRIVATE
            core
            graphics
            input
            ui
            audio
            threepp::threepp
            imgui
    )
endif()
