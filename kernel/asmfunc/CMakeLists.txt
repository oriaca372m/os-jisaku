enable_language(ASM-ATT)
add_library(asmfunc asmfunc.s)
target_include_directories(asmfunc INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
