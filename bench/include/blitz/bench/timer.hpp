#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

namespace blitz::bench {

using ns_t = std::uint64_t;

ns_t now_ns() noexcept;

struct TimerOverhead {
  std::size_t samples;
  ns_t min;
  ns_t median;
  double mean;
  std::optional<ns_t> resolution;
};

TimerOverhead measure_timer_overhead(std::size_t samples);

}
