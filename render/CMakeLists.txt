project(Render)

include_directories(inc/Render)
include_directories(../foundation/inc)

file(GLOB SOURCE_FILES
  ${Render_SOURCE_DIR}/Glad/src/glad.c
  ${Render_SOURCE_DIR}/src/*.cpp
)

add_library(Render ${SOURCE_FILES})
target_include_directories(Render PUBLIC Glad/include)
target_include_directories(Render PUBLIC inc)
target_compile_features(Render PUBLIC cxx_std_17)

