#include <gtest/gtest.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

#include "blitz/bench/stats.hpp"

namespace blitz::bench {
namespace {

std::vector<ns_t> one_to(ns_t last) {
  std::vector<ns_t> samples(last);
  std::iota(samples.begin(), samples.end(), ns_t{1});
  return samples;
}

TEST(Summarize, OneToThousand) {
  const Summary summary = summarize(one_to(1000));
  EXPECT_EQ(summary.count, 1000u);
  EXPECT_EQ(summary.min, 1u);
  EXPECT_EQ(summary.p50, 500u);
  EXPECT_EQ(summary.p90, 900u);
  EXPECT_EQ(summary.p99, 990u);
  EXPECT_EQ(summary.p999, 999u);
  EXPECT_EQ(summary.max, 1000u);
  EXPECT_DOUBLE_EQ(summary.mean, 500.5);
}

TEST(Summarize, SingleSampleIsEveryPercentile) {
  const std::vector<ns_t> samples{42};
  const Summary summary = summarize(samples);
  EXPECT_EQ(summary.count, 1u);
  EXPECT_EQ(summary.min, 42u);
  EXPECT_EQ(summary.p50, 42u);
  EXPECT_EQ(summary.p90, 42u);
  EXPECT_EQ(summary.p99, 42u);
  EXPECT_EQ(summary.p999, 42u);
  EXPECT_EQ(summary.max, 42u);
  EXPECT_DOUBLE_EQ(summary.mean, 42.0);
}

TEST(Summarize, P999OfTenSamplesIsMax) {
  const Summary summary = summarize(one_to(10));
  EXPECT_EQ(summary.p999, 10u);
  EXPECT_EQ(summary.p999, summary.max);
}

TEST(Summarize, InputOrderDoesNotMatter) {
  const std::vector<ns_t> sorted = one_to(1000);
  std::vector<ns_t> shuffled = sorted;
  std::mt19937 engine(7);
  std::shuffle(shuffled.begin(), shuffled.end(), engine);
  ASSERT_NE(shuffled, sorted);
  EXPECT_EQ(summarize(shuffled), summarize(sorted));
}

TEST(Summarize, EmptySampleThrows) {
  EXPECT_THROW(static_cast<void>(summarize({})), std::invalid_argument);
}

TEST(Percentile, EmptySampleThrows) {
  EXPECT_THROW(static_cast<void>(percentile({}, 1, 2)), std::invalid_argument);
}

TEST(Percentile, RejectsFractionOutsideZeroToOne) {
  const std::vector<ns_t> samples = one_to(10);
  EXPECT_THROW(static_cast<void>(percentile(samples, 1, 0)), std::invalid_argument);
  EXPECT_THROW(static_cast<void>(percentile(samples, 0, 100)), std::invalid_argument);
  EXPECT_THROW(static_cast<void>(percentile(samples, 101, 100)), std::invalid_argument);
}

TEST(Percentile, NearestRankRoundsUp) {
  const std::vector<ns_t> samples = one_to(10);
  EXPECT_EQ(percentile(samples, 1, 100), 1u);
  EXPECT_EQ(percentile(samples, 11, 100), 2u);
  EXPECT_EQ(percentile(samples, 1, 1), 10u);
}

}
}
