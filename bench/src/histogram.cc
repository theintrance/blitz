#include "blitz/bench/histogram.hpp"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>

namespace blitz::bench {

namespace {

constexpr std::size_t kBarWidth = 40;

int digits(std::uint64_t value) {
  return static_cast<int>(std::to_string(value).size());
}

}

Histogram make_histogram(std::span<const ns_t> samples) noexcept {
  Histogram counts{};
  for (const ns_t value : samples) {
    ++counts[histogram_bucket(value)];
  }
  return counts;
}

void print_histogram(std::span<const ns_t> samples, std::ostream& out) {
  const Histogram counts = make_histogram(samples);

  std::size_t first = 0;
  while (first < counts.size() && counts[first] == 0) {
    ++first;
  }
  if (first == counts.size()) {
    return;
  }
  std::size_t last = counts.size() - 1;
  while (counts[last] == 0) {
    --last;
  }

  std::size_t peak = 0;
  for (std::size_t bucket = first; bucket <= last; ++bucket) {
    peak = std::max(peak, counts[bucket]);
  }

  const bool top_is_last = last + 1 == kHistogramBuckets;
  const ns_t widest_upper = top_is_last ? std::numeric_limits<ns_t>::max() : ns_t{1} << last;
  const int bound_width = digits(widest_upper);
  const int count_width = digits(peak);
  const double total = static_cast<double>(samples.size());

  std::ostringstream text;
  text << std::fixed << std::setprecision(2);
  for (std::size_t bucket = first; bucket <= last; ++bucket) {
    const bool top = bucket + 1 == kHistogramBuckets;
    const ns_t lower = histogram_bucket_lower(bucket);
    const ns_t upper = top ? std::numeric_limits<ns_t>::max() : ns_t{1} << bucket;
    const std::size_t count = counts[bucket];
    const double percent = 100.0 * static_cast<double>(count) / total;
    std::size_t bar = count * kBarWidth / peak;
    if (count > 0 && bar == 0) {
      bar = 1;
    }

    text << "  [" << std::setw(bound_width) << lower << ", " << std::setw(bound_width) << upper << (top ? "]" : ")")
         << " ns  " << std::setw(count_width) << count << "  " << std::setw(6) << percent << '%';
    if (bar > 0) {
      text << "  " << std::string(bar, '#');
    }
    text << '\n';
  }
  out << text.str();
}

}
