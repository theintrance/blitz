#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

#include "blitz/bench/timer.hpp"

namespace blitz::bench {
namespace {

TEST(Timer, NeverGoesBackwards) {
  ns_t previous = now_ns();
  for (std::size_t i = 0; i < 100'000; ++i) {
    const ns_t current = now_ns();
    ASSERT_GE(current, previous);
    previous = current;
  }
}

TEST(TimerOverhead, SummarisesConsecutiveCalls) {
  const TimerOverhead overhead = measure_timer_overhead(10'000);
  EXPECT_EQ(overhead.samples, 10'000u);
  EXPECT_LE(overhead.min, overhead.median);
  EXPECT_GE(overhead.mean, static_cast<double>(overhead.min));
  if (overhead.resolution) {
    EXPECT_GT(*overhead.resolution, 0u);
  }
}

TEST(TimerOverhead, RejectsZeroSamples) {
  EXPECT_THROW(static_cast<void>(measure_timer_overhead(0)), std::invalid_argument);
}

}
}
