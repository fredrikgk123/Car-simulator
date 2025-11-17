add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(input)

add_subdirectory(ui)
add_subdirectory(audio)

add_executable(bilsim main.cpp)
target_include_directories(bilsim PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(bilsim PRIVATE
        core
        graphics
        input
        ui
        audio
        threepp::threepp
)

# On Windows (MSVC), we need to explicitly link imgui even though it's already
# linked through ui, because MSVC doesn't automatically resolve symbols from
# transitive static library dependencies the same way Unix linkers do
if(MSVC)
    target_link_libraries(bilsim PRIVATE imgui)
endif()
