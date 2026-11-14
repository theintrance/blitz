add_library(qs_warnings INTERFACE)
target_compile_options(qs_warnings INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
