#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include "qs/bench/timer.hpp"

namespace qs::bench {

class LatencyRecorder {
 public:
  explicit LatencyRecorder(std::size_t capacity) : samples_(capacity) {}

  void record(ns_t value) noexcept {
    if (size_ < samples_.size()) {
      samples_[size_++] = value;
    } else {
      ++dropped_;
    }
  }

  std::size_t size() const noexcept { return size_; }
  std::size_t capacity() const noexcept { return samples_.size(); }
  std::size_t dropped() const noexcept { return dropped_; }
  std::span<const ns_t> samples() const noexcept { return {samples_.data(), size_}; }

 private:
  std::vector<ns_t> samples_;
  std::size_t size_ = 0;
  std::size_t dropped_ = 0;
};

}
