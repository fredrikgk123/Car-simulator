add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(input)

add_subdirectory(ui)
add_subdirectory(audio)

add_executable(bilsim main.cpp)
target_include_directories(bilsim PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link all required libraries
if(WIN32)
    target_link_libraries(bilsim PRIVATE
            core
            graphics
            input
            ui
            audio
            threepp::threepp
    )

    # On MSVC, explicitly link imgui to resolve symbols from ui.lib
    # The ui library links to imgui, but MSVC doesn't automatically pull in
    # transitive static library dependencies
    if(MSVC)
        target_link_libraries(bilsim PRIVATE imgui)
    endif()
else()
    target_link_libraries(bilsim PRIVATE
            core
            graphics
            input
            ui
            audio
            threepp::threepp
    )
endif()
