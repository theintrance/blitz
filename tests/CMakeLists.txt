file(GLOB_RECURSE blitz_tests_sources CONFIGURE_DEPENDS *.cpp *.cc)

if (NOT blitz_tests_sources)
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

add_executable(blitz_tests ${blitz_tests_sources})
target_link_libraries(blitz_tests PRIVATE blitz_bench blitz_warnings GTest::gtest_main)
gtest_discover_tests(blitz_tests)
