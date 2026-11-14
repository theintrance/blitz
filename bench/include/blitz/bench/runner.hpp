#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "blitz/bench/recorder.hpp"
#include "blitz/bench/stats.hpp"
#include "blitz/bench/timer.hpp"

namespace blitz::bench {

template <typename T>
inline void do_not_optimize(const T& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

struct RunConfig {
  std::size_t warmup = 10'000;
  std::size_t iterations = 1'000'000;
};

struct RunResult {
  std::string name;
  RunConfig config;
  Summary latency;
  LatencyRecorder recorder;
  ns_t wall_ns;
  double throughput_per_sec;
};

namespace detail {

template <typename F>
inline void invoke_once(F& fn) {
  if constexpr (std::is_void_v<std::invoke_result_t<F&>>) {
    std::invoke(fn);
  } else {
    do_not_optimize(std::invoke(fn));
  }
}

}

template <typename F>
RunResult run(std::string_view name, const RunConfig& config, F&& fn) {
  if (config.iterations == 0) {
    throw std::invalid_argument("run: iterations must be positive");
  }

  LatencyRecorder recorder(config.iterations);

  for (std::size_t i = 0; i < config.warmup; ++i) {
    detail::invoke_once(fn);
  }

  const ns_t wall_start = now_ns();
  for (std::size_t i = 0; i < config.iterations; ++i) {
    const ns_t start = now_ns();
    detail::invoke_once(fn);
    const ns_t end = now_ns();
    recorder.record(end - start);
  }
  const ns_t wall_ns = now_ns() - wall_start;

  const Summary latency = summarize(recorder.samples());
  const double seconds = static_cast<double>(wall_ns) / 1e9;
  const double throughput = seconds > 0 ? static_cast<double>(config.iterations) / seconds : 0.0;

  return RunResult{
      .name = std::string(name),
      .config = config,
      .latency = latency,
      .recorder = std::move(recorder),
      .wall_ns = wall_ns,
      .throughput_per_sec = throughput,
  };
}

}
