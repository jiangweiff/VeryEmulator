cmake_minimum_required(VERSION 3.5)
project(VeryEmulator)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/IMGUI EXCLUDE_FROM_ALL)
add_subdirectory(render)

# Create your game executable target as usual
add_executable(VeryEmulator WIN32 core/VeryEmulator.cpp)

# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(VeryEmulator PRIVATE SDL2::SDL2main)
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(VeryEmulator PRIVATE SDL2::SDL2)
target_link_libraries(VeryEmulator PRIVATE IMGUI)
target_link_libraries(VeryEmulator PRIVATE Render)