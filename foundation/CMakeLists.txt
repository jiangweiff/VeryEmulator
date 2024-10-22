project(Foundation)

add_library(Foundation src/dummy.cpp src/Util/CommandLine.cpp)

target_include_directories(Foundation PUBLIC inc)
# target_compile_definitions(Foundation INTERFACE "-D_CRT_SECURE_NO_DEPRECATE")
target_compile_features(Foundation INTERFACE cxx_std_17)



