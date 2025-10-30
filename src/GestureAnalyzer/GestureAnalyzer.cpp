#define _USE_MATH_DEFINES
#include "GestureAnalyzer/GestureAnalyzer.h"
#include <cmath>

namespace VirtualDesktop {

void GestureAnalyzer::addPosition(int32_t x, int32_t y) {
    // Filter out duplicate positions to reduce noise
    if (!m_positions.empty() && m_positions.back().first == x && m_positions.back().second == y) {
        return;
    }
    m_positions.emplace_back(x, y);
}

GestureAnalyzer::Direction GestureAnalyzer::analyzeGesture() const {
    // Simplified: only determine horizontal direction (Left / Right / None)
    constexpr size_t MIN_POSITIONS = 3;
    if (m_positions.size() < MIN_POSITIONS) {
        return Direction::None;
    }

    // Total displacement from first to last recorded position
    int32_t totalDx = m_positions.back().first - m_positions.front().first;
    int32_t totalDy = m_positions.back().second - m_positions.front().second;

    // SPDLOG_DEBUG("Gesture analysis - totalDx: {}, totalDy: {}", totalDx, totalDy);

    // Only consider significant horizontal movement as a swipe
    if (std::abs(totalDx) < MIN_SWIPE_DISTANCE) {
        return Direction::None;
    }

    if (std::abs(totalDy) > std::abs(totalDx)) {
        // Vertical movement dominates, ignore for horizontal-only detection
        return Direction::None;
    }

    if (totalDx > 0) {
        return Direction::Right;
    } else {
        return Direction::Left;
    }
}

void GestureAnalyzer::clearPositions() {
    m_positions.clear();
}

bool GestureAnalyzer::isGestureInProgress() const {
    return !m_positions.empty();
}

}  // namespace VirtualDesktop