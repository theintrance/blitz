#include <gtest/gtest.h>

#include <algorithm>
#include <sstream>
#include <string>

#include "qs/bench/report.hpp"
#include "qs/bench/runner.hpp"

namespace qs::bench {
namespace {

RunResult small_run(std::size_t iterations) {
  return run("small", RunConfig{.warmup = 0, .iterations = iterations}, [] {});
}

TEST(Report, WarnsWhenTooFewSamplesForP999) {
  std::ostringstream few;
  print_report(small_run(kMinSamplesForP999 - 1), few);
  EXPECT_NE(few.str().find("warning"), std::string::npos);

  std::ostringstream enough;
  print_report(small_run(kMinSamplesForP999), enough);
  EXPECT_EQ(enough.str().find("warning"), std::string::npos);
}

TEST(Report, CsvRowMatchesHeaderColumns) {
  std::ostringstream header;
  write_csv_header(header);
  std::ostringstream row;
  write_csv_row(small_run(10), row);

  const std::string header_text = header.str();
  const std::string row_text = row.str();
  EXPECT_EQ(std::count(header_text.begin(), header_text.end(), ','), 9);
  EXPECT_EQ(std::count(row_text.begin(), row_text.end(), ','), 9);
  EXPECT_EQ(row_text.rfind("small,10,", 0), 0u);
}

}
}
