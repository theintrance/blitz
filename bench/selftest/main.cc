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

#include "blitz/bench/histogram.hpp"
#include "blitz/bench/report.hpp"
#include "blitz/bench/runner.hpp"
#include "blitz/bench/timer.hpp"

namespace {

constexpr std::size_t kTimerSamples = 1'000'000;
constexpr std::size_t kSortSize = 1024;
constexpr std::uint32_t kSortSeed = 42;
constexpr blitz::bench::ns_t kSleepNs = 100'000;
constexpr std::string_view kDefaultCsvPath = "bench_selftest.csv";

std::vector<std::uint32_t> make_sort_input() {
  std::mt19937 engine(kSortSeed);
  std::vector<std::uint32_t> input(kSortSize);
  for (std::uint32_t& value : input) {
    value = static_cast<std::uint32_t>(engine());
  }
  return input;
}

void report(const blitz::bench::RunResult& result, std::ostream& csv) {
  blitz::bench::print_report(result, std::cout);
  std::cout << '\n';
  blitz::bench::print_histogram(result.recorder.samples(), std::cout);
  std::cout << '\n';
  blitz::bench::write_csv_row(result, csv);
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
    blitz::bench::write_csv_header(csv);
  }

  blitz::bench::print_build_info(std::cout);
  std::cout << '\n';

  blitz::bench::print_timer_overhead(blitz::bench::measure_timer_overhead(kTimerSamples), std::cout);
  std::cout << '\n';

  report(blitz::bench::run("empty", blitz::bench::RunConfig{}, [] {}), csv);

  const blitz::bench::RunResult sleep =
      blitz::bench::run("sleep_100us", blitz::bench::RunConfig{.warmup = 10, .iterations = 1000},
                        [] { std::this_thread::sleep_for(std::chrono::nanoseconds(kSleepNs)); });
  report(sleep, csv);

  const std::vector<std::uint32_t> input = make_sort_input();
  std::vector<std::uint32_t> work(input.size());
  report(blitz::bench::run("sort_1024", blitz::bench::RunConfig{},
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
