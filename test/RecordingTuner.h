#pragma once

#include "FakeTuner.h"
#include <sstream>
#include <string>
#include <vector>

// Golden Master용: FakeTuner 동작 + Tuner API 호출 순서 기록
class RecordingTuner : public Tuner {
  FakeTuner inner_;
  std::ostringstream trace_;

  void append(const std::string &line) { trace_ << line << '\n'; }

public:
  explicit RecordingTuner(std::vector<int> available = {1, 4, 12, 56})
      : inner_(std::move(available)) {}

  std::string seekCH() override {
    const std::string result = inner_.seekCH();
    append("seekCH -> " + result);
    return result;
  }

  void setCH(const std::string &ch) override {
    append("setCH(" + ch + ")");
    inner_.setCH(ch);
  }

  std::string getCurrentCH() override {
    const std::string result = inner_.getCurrentCH();
    append("getCurrentCH -> " + result);
    return result;
  }

  std::string trace() const { return trace_.str(); }

  void clearTrace() { trace_.str(""); trace_.clear(); }
};
