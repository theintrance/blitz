#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <iosfwd>
#include <limits>
#include <span>

#include "blitz/bench/timer.hpp"

namespace blitz::bench {

inline constexpr std::size_t kHistogramBuckets = std::numeric_limits<ns_t>::digits + 1;

using Histogram = std::array<std::size_t, kHistogramBuckets>;

constexpr std::size_t histogram_bucket(ns_t value) noexcept {
  return static_cast<std::size_t>(std::bit_width(value));
}

constexpr ns_t histogram_bucket_lower(std::size_t bucket) noexcept {
  return bucket == 0 ? ns_t{0} : ns_t{1} << (bucket - 1);
}

Histogram make_histogram(std::span<const ns_t> samples) noexcept;
void print_histogram(std::span<const ns_t> samples, std::ostream& out);

}
