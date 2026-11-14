#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "qs/bench/histogram.hpp"
#include "qs/bench/report.hpp"
#include "qs/bench/runner.hpp"
#include "qs/bench/timer.hpp"

namespace {

constexpr std::size_t kTimerSamples = 1'000'000;
constexpr std::size_t kSortSize = 1024;
constexpr std::uint32_t kSortSeed = 42;
constexpr qs::bench::ns_t kSleepNs = 100'000;
constexpr std::string_view kDefaultCsvPath = "bench_selftest.csv";

std::vector<std::uint32_t> make_sort_input() {
  std::mt19937 engine(kSortSeed);
  std::vector<std::uint32_t> input(kSortSize);
  for (std::uint32_t& value : input) {
    value = static_cast<std::uint32_t>(engine());
  }
  return input;
}

void report(const qs::bench::RunResult& result, std::ostream& csv) {
  qs::bench::print_report(result, std::cout);
  std::cout << '\n';
  qs::bench::print_histogram(result.recorder.samples(), std::cout);
  std::cout << '\n';
  qs::bench::write_csv_row(result, csv);
}

bool is_missing_or_empty(const std::string& path) {
  std::ifstream existing(path);
  return !existing || existing.peek() == std::ifstream::traits_type::eof();
}

}

int main(int argc, char** argv) {
  std::string csv_path(kDefaultCsvPath);
  if (argc == 3 && std::string_view(argv[1]) == "--csv") {
    csv_path = argv[2];
  } else if (argc != 1) {
    std::cerr << "usage: " << argv[0] << " [--csv PATH]\n";
    return EXIT_FAILURE;
  }

  const bool new_csv = is_missing_or_empty(csv_path);
  std::ofstream csv(csv_path, std::ios::app);
  if (!csv) {
    std::cerr << "cannot open " << csv_path << '\n';
    return EXIT_FAILURE;
  }
  if (new_csv) {
    qs::bench::write_csv_header(csv);
  }

  qs::bench::print_timer_overhead(qs::bench::measure_timer_overhead(kTimerSamples), std::cout);
  std::cout << '\n';

  report(qs::bench::run("empty", qs::bench::RunConfig{}, [] {}), csv);

  const qs::bench::RunResult sleep =
      qs::bench::run("sleep_100us", qs::bench::RunConfig{.warmup = 10, .iterations = 1000},
                     [] { std::this_thread::sleep_for(std::chrono::nanoseconds(kSleepNs)); });
  report(sleep, csv);

  const std::vector<std::uint32_t> input = make_sort_input();
  std::vector<std::uint32_t> work(input.size());
  report(qs::bench::run("sort_1024", qs::bench::RunConfig{},
                        [&input, &work] {
                          std::copy(input.begin(), input.end(), work.begin());
                          std::sort(work.begin(), work.end());
                          return work.front();
                        }),
         csv);

  std::cout << "csv: " << csv_path << '\n';

  if (sleep.latency.min < kSleepNs) {
    std::cerr << "selftest failed: sleep_100us min " << sleep.latency.min << " ns < " << kSleepNs << " ns\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
