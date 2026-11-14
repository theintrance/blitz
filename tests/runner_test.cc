#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

#include "qs/bench/runner.hpp"

namespace qs::bench {
namespace {

TEST(Run, CallsWarmupPlusIterationsAndRecordsOnlyIterations) {
  std::size_t calls = 0;
  const RunResult result = run("count", RunConfig{.warmup = 5, .iterations = 100}, [&calls] { ++calls; });
  EXPECT_EQ(calls, 105u);
  EXPECT_EQ(result.name, "count");
  EXPECT_EQ(result.latency.count, 100u);
  EXPECT_EQ(result.recorder.size(), 100u);
  EXPECT_EQ(result.recorder.dropped(), 0u);
  EXPECT_GE(result.wall_ns, result.latency.max);
  EXPECT_GT(result.throughput_per_sec, 0.0);
}

TEST(Run, AcceptsFunctionsReturningValues) {
  std::size_t calls = 0;
  const RunResult result = run("value", RunConfig{.warmup = 0, .iterations = 10}, [&calls] { return ++calls; });
  EXPECT_EQ(calls, 10u);
  EXPECT_EQ(result.latency.count, 10u);
}

TEST(Run, RejectsZeroIterations) {
  EXPECT_THROW(static_cast<void>(run("none", RunConfig{.warmup = 0, .iterations = 0}, [] {})), std::invalid_argument);
}

}
}
