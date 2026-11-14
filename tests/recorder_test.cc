#include <gtest/gtest.h>

#include <vector>

#include "qs/bench/recorder.hpp"

namespace qs::bench {
namespace {

TEST(LatencyRecorder, StartsEmpty) {
  const LatencyRecorder recorder(4);
  EXPECT_EQ(recorder.size(), 0u);
  EXPECT_EQ(recorder.capacity(), 4u);
  EXPECT_EQ(recorder.dropped(), 0u);
  EXPECT_TRUE(recorder.samples().empty());
}

TEST(LatencyRecorder, StopsAtCapacityAndCountsDropped) {
  LatencyRecorder recorder(3);
  for (ns_t value = 1; value <= 5; ++value) {
    recorder.record(value);
  }
  EXPECT_EQ(recorder.size(), 3u);
  EXPECT_EQ(recorder.dropped(), 2u);
  const auto samples = recorder.samples();
  EXPECT_EQ(std::vector<ns_t>(samples.begin(), samples.end()), (std::vector<ns_t>{1, 2, 3}));
}

}
}
