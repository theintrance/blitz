#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "blitz/bench/timer.hpp"

namespace blitz::bench {

struct Summary {
  std::size_t count;
  ns_t min;
  ns_t p50;
  ns_t p90;
  ns_t p99;
  ns_t p999;
  ns_t max;
  double mean;

  friend bool operator==(const Summary&, const Summary&) = default;
};

ns_t percentile(std::span<const ns_t> sorted, std::uint64_t num, std::uint64_t den);
Summary summarize(std::span<const ns_t> samples);

}
