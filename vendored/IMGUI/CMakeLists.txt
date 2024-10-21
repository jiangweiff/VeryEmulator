project(IMGUI)

include_directories(../SDL/include)

file(GLOB SOURCE_FILES
  ${IMGUI_SOURCE_DIR}/*.cpp
  ${IMGUI_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
  ${IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(IMGUI ${SOURCE_FILES})
target_include_directories(IMGUI PUBLIC .)
target_include_directories(IMGUI PUBLIC ./backends)

