#pragma once

#include <cstddef>
#include <iosfwd>

#include "qs/bench/runner.hpp"
#include "qs/bench/timer.hpp"

namespace qs::bench {

inline constexpr std::size_t kMinSamplesForP999 = 1000;

void print_report(const RunResult& result, std::ostream& out);
void print_timer_overhead(const TimerOverhead& overhead, std::ostream& out);
void write_csv_header(std::ostream& out);
void write_csv_row(const RunResult& result, std::ostream& out);

}
