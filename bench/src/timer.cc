#include "blitz/bench/timer.hpp"

#include <algorithm>
#include <chrono>
#include <numeric>
#include <stdexcept>
#include <utility>
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
  return summarize_timer_deltas(std::move(deltas));
}

TimerOverhead summarize_timer_deltas(std::vector<ns_t> deltas) {
  if (deltas.empty()) {
    throw std::invalid_argument("summarize_timer_deltas: deltas must not be empty");
  }

  std::sort(deltas.begin(), deltas.end());
  const ns_t total = std::accumulate(deltas.begin(), deltas.end(), ns_t{0});

  TimerOverhead overhead{
      .samples = deltas.size(),
      .min = deltas.front(),
      .median = percentile(deltas, 1, 2),
      .mean = static_cast<double>(total) / static_cast<double>(deltas.size()),
      .resolution = std::nullopt,
  };
  if (overhead.min == 0) {
    const auto nonzero = std::upper_bound(deltas.begin(), deltas.end(), ns_t{0});
    if (nonzero != deltas.end()) {
      overhead.resolution = *nonzero;
    }
  }
  return overhead;
}

}
