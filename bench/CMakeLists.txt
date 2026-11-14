file(GLOB_RECURSE qs_bench_sources CONFIGURE_DEPENDS src/*.cpp src/*.cc)

if (qs_bench_sources)
  add_library(qs_bench STATIC ${qs_bench_sources})
  target_include_directories(qs_bench PUBLIC include)
  target_link_libraries(qs_bench PRIVATE qs_warnings)
else()
  add_library(qs_bench INTERFACE)
  target_include_directories(qs_bench INTERFACE include)
endif()

file(GLOB_RECURSE bench_selftest_sources CONFIGURE_DEPENDS selftest/*.cpp selftest/*.cc)

if (bench_selftest_sources)
  add_executable(bench_selftest ${bench_selftest_sources})
  target_link_libraries(bench_selftest PRIVATE qs_bench qs_warnings)
endif()
