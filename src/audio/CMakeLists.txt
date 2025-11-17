# Create a library for audio components
add_library(audio
    audio_manager.cpp
)

target_include_directories(audio PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(audio PUBLIC
    core
)

# Add miniaudio include directory
target_include_directories(audio PRIVATE
    ${miniaudio_SOURCE_DIR}
)
