file(GLOB_RECURSE blitz_bench_sources CONFIGURE_DEPENDS src/*.cpp src/*.cc)

if (blitz_bench_sources)
  add_library(blitz_bench STATIC ${blitz_bench_sources})
  target_include_directories(blitz_bench PUBLIC include)
  target_link_libraries(blitz_bench PRIVATE blitz_warnings)

  string(TOUPPER "${CMAKE_BUILD_TYPE}" blitz_build_type)
  string(STRIP "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${blitz_build_type}}" blitz_cxx_flags)
  target_compile_definitions(blitz_bench PRIVATE
    "BLITZ_COMPILER=\"${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}\""
    "BLITZ_BUILD_TYPE=\"${CMAKE_BUILD_TYPE}\""
    "BLITZ_CXX_FLAGS=\"${blitz_cxx_flags}\""
    "BLITZ_SANITIZE_ENABLED=$<BOOL:${BLITZ_SANITIZE}>"
  )
else()
  add_library(blitz_bench INTERFACE)
  target_include_directories(blitz_bench INTERFACE include)
endif()

file(GLOB_RECURSE bench_selftest_sources CONFIGURE_DEPENDS selftest/*.cpp selftest/*.cc)

if (bench_selftest_sources)
  add_executable(bench_selftest ${bench_selftest_sources})
  target_link_libraries(bench_selftest PRIVATE blitz_bench blitz_warnings)
endif()
