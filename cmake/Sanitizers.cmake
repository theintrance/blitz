option(BLITZ_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if (BLITZ_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()
