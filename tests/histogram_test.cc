#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "blitz/bench/histogram.hpp"

namespace blitz::bench {
namespace {

TEST(Histogram, PowerOfTwoBuckets) {
  EXPECT_EQ(histogram_bucket(0), 0u);
  EXPECT_EQ(histogram_bucket(1), 1u);
  EXPECT_EQ(histogram_bucket(2), 2u);
  EXPECT_EQ(histogram_bucket(3), 2u);
  EXPECT_EQ(histogram_bucket(4), 3u);
  EXPECT_EQ(histogram_bucket(1023), 10u);
  EXPECT_EQ(histogram_bucket(1024), 11u);
  EXPECT_EQ(histogram_bucket(std::numeric_limits<ns_t>::max()), kHistogramBuckets - 1);
}

TEST(Histogram, BucketHoldsValuesFromLowerToTwiceLower) {
  for (const ns_t value : {1u, 2u, 3u, 4u, 1023u, 1024u}) {
    const ns_t lower = histogram_bucket_lower(histogram_bucket(value));
    EXPECT_LE(lower, value) << value;
    EXPECT_LT(value, 2 * lower) << value;
  }
  EXPECT_EQ(histogram_bucket_lower(histogram_bucket(1023)), 512u);
  EXPECT_EQ(histogram_bucket_lower(histogram_bucket(1024)), 1024u);
}

TEST(Histogram, ZeroHasItsOwnBucket) {
  EXPECT_EQ(histogram_bucket_lower(histogram_bucket(0)), 0u);
  EXPECT_NE(histogram_bucket(0), histogram_bucket(1));
}

TEST(Histogram, CountsSamplesPerBucket) {
  const std::vector<ns_t> samples{0, 1, 2, 3, 4, 1023, 1024};
  const Histogram counts = make_histogram(samples);
  EXPECT_EQ(counts[0], 1u);
  EXPECT_EQ(counts[1], 1u);
  EXPECT_EQ(counts[2], 2u);
  EXPECT_EQ(counts[3], 1u);
  EXPECT_EQ(counts[10], 1u);
  EXPECT_EQ(counts[11], 1u);
  std::size_t total = 0;
  for (const std::size_t count : counts) {
    total += count;
  }
  EXPECT_EQ(total, samples.size());
}

TEST(Histogram, PrintsFromFirstToLastNonEmptyBucket) {
  const std::vector<ns_t> samples{4, 1024};
  std::ostringstream out;
  print_histogram(samples, out);
  const std::string text = out.str();
  EXPECT_EQ(std::count(text.begin(), text.end(), '\n'), 9);
  EXPECT_EQ(text.find("[   4,    8) ns"), text.find('['));
  EXPECT_NE(text.find("[1024, 2048) ns"), std::string::npos);
}

TEST(Histogram, PrintsNothingForEmptySample) {
  std::ostringstream out;
  print_histogram({}, out);
  EXPECT_TRUE(out.str().empty());
}

}
}
