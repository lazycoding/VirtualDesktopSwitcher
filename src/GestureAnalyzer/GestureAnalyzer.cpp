#define _USE_MATH_DEFINES
#include "GestureAnalyzer/GestureAnalyzer.h"
#include <cmath>

namespace VirtualDesktop {

void GestureAnalyzer::addPosition(int32_t x, int32_t y) {
    // Filter out duplicate positions to reduce noise
    if (!m_rawPositions.empty() && m_rawPositions.back().first == x && m_rawPositions.back().second == y) {
        return;
    }
    m_rawPositions.emplace_back(x, y);
}

GestureAnalyzer::Direction GestureAnalyzer::analyzeGesture() const {
    if (m_rawPositions.size() < 3) { // Need at least 3 points for recognition
        return Direction::None;
    }

    // Total displacement from first to last recorded position
    int32_t totalDx = m_rawPositions.back().first - m_rawPositions.front().first;
    int32_t totalDy = m_rawPositions.back().second - m_rawPositions.front().second;

    // Check if movement is significant enough
    const int32_t MIN_SWIPE_DISTANCE = 50; 
    if (std::abs(totalDx) < MIN_SWIPE_DISTANCE && std::abs(totalDy) < MIN_SWIPE_DISTANCE) {
        return Direction::None;
    }

    // Determine primary direction based on dominant axis
    if (std::abs(totalDx) >= std::abs(totalDy)) {
        // Horizontal movement dominates
        if (totalDx > 0) {
            return Direction::Right;
        } else {
            return Direction::Left;
        }
    } else {
        // Vertical movement dominates
        if (totalDy > 0) {
            return Direction::Down;
        } else {
            return Direction::Up;
        }
    }
}

void GestureAnalyzer::clearPositions() {
    m_rawPositions.clear();
}

bool GestureAnalyzer::isGestureInProgress() const {
    return !m_rawPositions.empty();
}

}  // namespace VirtualDesktop