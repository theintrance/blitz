#include "blitz/bench/timer.hpp"

#include <algorithm>
#include <chrono>
#include <numeric>
#include <stdexcept>
#include <vector>

#include "blitz/bench/stats.hpp"

namespace blitz::bench {

ns_t now_ns() noexcept {
  const auto since_epoch = std::chrono::steady_clock::now().time_since_epoch();
  return static_cast<ns_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(since_epoch).count());
}

TimerOverhead measure_timer_overhead(std::size_t samples) {
  if (samples == 0) {
    throw std::invalid_argument("measure_timer_overhead: samples must be positive");
  }

  std::vector<ns_t> deltas(samples);
  for (ns_t& delta : deltas) {
    const ns_t first = now_ns();
    const ns_t second = now_ns();
    delta = second - first;
  }
  std::sort(deltas.begin(), deltas.end());
  const ns_t total = std::accumulate(deltas.begin(), deltas.end(), ns_t{0});

  TimerOverhead overhead{
      .samples = samples,
      .min = deltas.front(),
      .median = percentile(deltas, 1, 2),
      .mean = static_cast<double>(total) / static_cast<double>(samples),
      .resolution = std::nullopt,
  };
  const auto nonzero = std::upper_bound(deltas.begin(), deltas.end(), ns_t{0});
  if (nonzero != deltas.end()) {
    overhead.resolution = *nonzero;
  }
  return overhead;
}

}
