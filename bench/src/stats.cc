#include "qs/bench/stats.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace qs::bench {

ns_t percentile(std::span<const ns_t> sorted, std::uint64_t num, std::uint64_t den) {
  if (sorted.empty()) {
    throw std::invalid_argument("percentile: empty sample");
  }
  if (den == 0 || num == 0 || num > den) {
    throw std::invalid_argument("percentile: fraction must be in (0, 1]");
  }
  const std::uint64_t n = sorted.size();
  const std::uint64_t rank = (n * num + den - 1) / den;
  return sorted[static_cast<std::size_t>(rank - 1)];
}

Summary summarize(std::span<const ns_t> samples) {
  if (samples.empty()) {
    throw std::invalid_argument("summarize: empty sample");
  }

  std::vector<ns_t> sorted(samples.begin(), samples.end());
  std::sort(sorted.begin(), sorted.end());
  const ns_t total = std::accumulate(sorted.begin(), sorted.end(), ns_t{0});

  return Summary{
      .count = sorted.size(),
      .min = sorted.front(),
      .p50 = percentile(sorted, 1, 2),
      .p90 = percentile(sorted, 9, 10),
      .p99 = percentile(sorted, 99, 100),
      .p999 = percentile(sorted, 999, 1000),
      .max = sorted.back(),
      .mean = static_cast<double>(total) / static_cast<double>(sorted.size()),
  };
}

}
