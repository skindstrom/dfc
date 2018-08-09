#ifndef DFC_MATCH_PATH_HPP
#define DFC_MATCH_PATH_HPP

#include <utility>

#include "byte.hpp"
#include "ct.hpp"
#include "df.hpp"
#include "is-ct.hpp"
#include "is-df.hpp"
#include "is-pattern-range.hpp"
#include "on-matcher.hpp"

namespace dfc {

template <typename PatternRange, typename DF, typename CT> class MatchPath {
  static_assert(is_pattern_range<PatternRange>::value,
                "First template parameter must be a pattern range");
  static_assert(is_direct_filter<DF>::value,
                "Second template parameter must be a direct filter");
  static_assert(is_compact_table<CT>::value,
                "Last template parameter must be a compact table");
  static_assert(PatternRange::startInclusive ==
                    sizeof(typename CT::segment_type),
                "The segment type of the CT must be equal in size to the "
                "smallest pattern length");

private:
  PatternRange const patternRange_{};
  DF df_;
  CT ct_;

public:
  void addPattern(ImmutablePattern const &pattern) {
    if (patternRange_.includes(pattern)) {
      df_.addPattern(pattern);
      ct_.addPattern(pattern);
    }
  }

  inline void match(InputView const &input, OnMatcher const &onMatcher) const {
    if (doesInputFitInDf(input) && df_.contains(input.data())) {
      if (doesInputFitInCt(input)) {
        ct_.findAllMatches(input, onMatcher);
      }
    }
  }

  inline bool doesInputFitInDf(InputView const &input) const noexcept {
    return input.size() >= static_cast<decltype(input.size())>(
                               sizeof(typename DF::segment_type));
  }

  inline bool doesInputFitInCt(InputView const &input) const noexcept {
    return input.size() >= static_cast<decltype(input.size())>(
                               sizeof(typename CT::segment_type));
  }

  /**
   * as this should only ever happen as the size of the input is 1, keep it out
   * of the hot path
   */
  inline void matchWithExtension(InputView const &input,
                                 OnMatcher const &onMatcher) const {
    if (shouldExtendInput(input)) {
      auto segments = extendInput(input);

      for (auto const &segment : segments) {
        if (df_.contains(segment.data())) {
          ct_.findAllMatches(InputView(segment.data(), segment.size()),
                             onMatcher);
        }
      }
    }
  }

private:
  inline bool shouldExtendInput(InputView const &input) const noexcept {
    const auto size = input.size();
    return size == 1 && size < PatternRange::startInclusive &&
           static_cast<int>(sizeof(typename DF::segment_type)) == 2;
  }

  inline auto extendInput(InputView const &input) const {
    SegmentExtender<typename DF::segment_type> extender;

    return extender.extend(input.data(), input.size());
  }
};
} // namespace dfc

#endif
