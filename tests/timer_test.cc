#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>
#include <vector>

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
    EXPECT_EQ(overhead.min, 0u);
    EXPECT_GT(*overhead.resolution, 0u);
  }
}

TEST(TimerOverhead, ResolutionIsTickWhenCallsShareATick) {
  const TimerOverhead overhead = summarize_timer_deltas({42, 0, 41, 0, 0, 83});
  EXPECT_EQ(overhead.samples, 6u);
  EXPECT_EQ(overhead.min, 0u);
  ASSERT_TRUE(overhead.resolution.has_value());
  EXPECT_EQ(*overhead.resolution, 41u);
}

TEST(TimerOverhead, NoResolutionWhenEveryDeltaIsNonzero) {
  const TimerOverhead overhead = summarize_timer_deltas({50, 21, 30, 50, 51});
  EXPECT_EQ(overhead.min, 21u);
  EXPECT_EQ(overhead.median, 50u);
  EXPECT_FALSE(overhead.resolution.has_value());
}

TEST(TimerOverhead, NoResolutionWhenEveryDeltaIsZero) {
  EXPECT_FALSE(summarize_timer_deltas({0, 0, 0}).resolution.has_value());
}

TEST(TimerOverhead, RejectsZeroSamples) {
  EXPECT_THROW(static_cast<void>(measure_timer_overhead(0)), std::invalid_argument);
  EXPECT_THROW(static_cast<void>(summarize_timer_deltas(std::vector<ns_t>{})), std::invalid_argument);
}

}
}
