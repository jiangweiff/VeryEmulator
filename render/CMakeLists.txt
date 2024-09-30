project(Render)

add_library(Render Glad/src/glad.c)
target_include_directories(Render PUBLIC Glad/include)

