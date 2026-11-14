#include "qs/bench/report.hpp"

#include <iomanip>
#include <ostream>
#include <sstream>
#include <string_view>

namespace qs::bench {

namespace {

constexpr int kLabelWidth = 12;
constexpr int kValueWidth = 12;

void write_row(std::ostream& out, std::string_view label, ns_t value) {
  out << "  " << std::left << std::setw(kLabelWidth) << label << std::right << std::setw(kValueWidth) << value << " ns";
}

}

void print_report(const RunResult& result, std::ostream& out) {
  const Summary& latency = result.latency;
  std::ostringstream text;

  text << '[' << result.name << "]  warmup=" << result.config.warmup << "  iterations=" << result.config.iterations
       << "  dropped=" << result.recorder.dropped() << '\n';
  text << "  " << std::left << std::setw(kLabelWidth) << "count" << std::right << std::setw(kValueWidth)
       << latency.count << '\n';
  write_row(text, "min", latency.min);
  text << '\n';
  write_row(text, "p50", latency.p50);
  text << '\n';
  write_row(text, "p90", latency.p90);
  text << '\n';
  write_row(text, "p99", latency.p99);
  text << '\n';
  write_row(text, "p99.9", latency.p999);
  if (latency.count < kMinSamplesForP999) {
    text << "  (warning: fewer than " << kMinSamplesForP999 << " samples)";
  }
  text << '\n';
  write_row(text, "max", latency.max);
  text << '\n';
  text << std::fixed << std::setprecision(1);
  text << "  " << std::left << std::setw(kLabelWidth) << "mean" << std::right << std::setw(kValueWidth) << latency.mean
       << " ns\n";
  text << std::setprecision(0);
  text << "  " << std::left << std::setw(kLabelWidth) << "throughput" << std::right << std::setw(kValueWidth)
       << result.throughput_per_sec << " ops/s";
  text << std::setprecision(2);
  text << "   (wall " << static_cast<double>(result.wall_ns) / 1e9 << " s)\n";

  out << text.str();
}

void print_timer_overhead(const TimerOverhead& overhead, std::ostream& out) {
  std::ostringstream text;

  text << "[timer_overhead]  samples=" << overhead.samples << '\n';
  write_row(text, "min", overhead.min);
  text << '\n';
  write_row(text, "median", overhead.median);
  text << '\n';
  text << std::fixed << std::setprecision(1);
  text << "  " << std::left << std::setw(kLabelWidth) << "mean" << std::right << std::setw(kValueWidth) << overhead.mean
       << " ns\n";
  if (overhead.resolution) {
    write_row(text, "resolution", *overhead.resolution);
  } else {
    text << "  " << std::left << std::setw(kLabelWidth) << "resolution" << std::right << std::setw(kValueWidth)
         << "n/a";
  }
  text << '\n';

  out << text.str();
}

void write_csv_header(std::ostream& out) {
  out << "name,count,min_ns,p50_ns,p90_ns,p99_ns,p999_ns,max_ns,mean_ns,throughput_ops_per_sec\n";
}

void write_csv_row(const RunResult& result, std::ostream& out) {
  const Summary& latency = result.latency;
  std::ostringstream text;

  text << result.name << ',' << latency.count << ',' << latency.min << ',' << latency.p50 << ',' << latency.p90 << ','
       << latency.p99 << ',' << latency.p999 << ',' << latency.max << ',' << std::fixed << std::setprecision(2)
       << latency.mean << ',' << result.throughput_per_sec << '\n';

  out << text.str();
}

}
