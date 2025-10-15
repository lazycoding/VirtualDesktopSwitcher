#include "GestureAnalyzer.h"

namespace VirtualDesktop {

void GestureAnalyzer::addPosition(int32_t x, int32_t y) {
  m_positions.emplace_back(x, y);
}

GestureAnalyzer::Direction GestureAnalyzer::analyzeGesture() const {
  if (m_positions.size() < 2) {
    return Direction::None;
  }

  const auto &first = m_positions.front();
  const auto &last = m_positions.back();
  const int32_t deltaX = last.first - first.first;

  if (std::abs(deltaX) < MIN_SWIPE_DISTANCE) {
    return Direction::None;
  }

  return deltaX > 0 ? Direction::Right : Direction::Left;
}

void GestureAnalyzer::clearPositions() { m_positions.clear(); }

} // namespace VirtualDesktop