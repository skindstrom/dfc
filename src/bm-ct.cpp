#include <cstdint>

#include "benchmark/benchmark.h"
#include "ct-initializer.hpp"

namespace {
int i;
struct TestOnMatcher : public dfc::OnMatcher<TestOnMatcher> {
  void onMatch(dfc::Pattern const& pattern) const {
    (void)(pattern);
    ++i;
  }
};

dfc::RawPattern pattern(char const* val) {
  return dfc::RawPattern(reinterpret_cast<byte const*>(val), std::strlen(val));
}

void CT_OneByte_ExactMatching(benchmark::State& state) {
  auto patterns = std::make_shared<std::vector<dfc::Pattern>>();

  dfc::CompactTableInitializer<uint8_t, 1, 0x100> initializer;
  std::string patternValue("x");
  benchmark::DoNotOptimize(&patternValue);

  patterns->emplace_back(0, pattern(patternValue.data()));
  int const patternIndex = 0;
  initializer.addPattern(patternIndex, patterns->at(patternIndex));

  auto const ct = initializer.ct<TestOnMatcher>(patterns);

  for (auto _ : state) {
    ct.exactMatching(patternValue.data(), 1);
  }
}

BENCHMARK(CT_OneByte_ExactMatching);

void CT_FourByte_ExactMatching(benchmark::State& state) {
  auto patterns = std::make_shared<std::vector<dfc::Pattern>>();

  dfc::CompactTableInitializer<uint32_t, 49157, 0x20000> initializer;
  std::string patternValue("abcd");
  benchmark::DoNotOptimize(&patternValue);

  patterns->emplace_back(0, pattern(patternValue.data()));
  int const patternIndex = 0;
  initializer.addPattern(patternIndex, patterns->at(patternIndex));

  auto const ct = initializer.ct<TestOnMatcher>(patterns);

  for (auto _ : state) {
    ct.exactMatching(patternValue.data(), 4);
  }
}

BENCHMARK(CT_FourByte_ExactMatching);
}  // namespace
