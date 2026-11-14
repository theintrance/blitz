file(GLOB_RECURSE qs_tests_sources CONFIGURE_DEPENDS *.cpp *.cc)

if (NOT qs_tests_sources)
  return()
endif()

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  URL_HASH SHA256=1f357c27ca988c3f7c6b4bf68a9395005ac6761f034046e9dde0896e3aba00e4
  SYSTEM
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_executable(qs_tests ${qs_tests_sources})
target_link_libraries(qs_tests PRIVATE qs_bench qs_warnings GTest::gtest_main)
gtest_discover_tests(qs_tests)
