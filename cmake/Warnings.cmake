add_library(blitz_warnings INTERFACE)
target_compile_options(blitz_warnings INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
