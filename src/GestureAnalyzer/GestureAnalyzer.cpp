#define _USE_MATH_DEFINES
#include "GestureAnalyzer.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <spdlog/spdlog.h>

namespace VirtualDesktop {

void GestureAnalyzer::addPosition(int32_t x, int32_t y) {
  // Filter out duplicate positions to reduce noise
  if (!m_positions.empty() && m_positions.back().first == x &&
      m_positions.back().second == y) {
    return;
  }
  m_positions.emplace_back(x, y);
}

GestureAnalyzer::Direction GestureAnalyzer::analyzeGesture() const {
  constexpr size_t MIN_POSITIONS = 3;
  if (m_positions.size() < MIN_POSITIONS) {
    SPDLOG_DEBUG("Not enough positions to analyze gesture ({} < {})",
                 m_positions.size(), MIN_POSITIONS);
    return Direction::None;
  }

  // Calculate movement vectors and velocities
  std::vector<std::pair<double, double>> velocities;
  for (size_t i = 1; i < m_positions.size(); ++i) {
    double dx = m_positions[i].first - m_positions[i - 1].first;
    double dy = m_positions[i].second - m_positions[i - 1].second;
    velocities.emplace_back(dx, dy);
  }

  // Calculate average velocity and total displacement
  double avgVx = 0.0, avgVy = 0.0;
  double totalDx = m_positions.back().first - m_positions.front().first;
  double totalDy = m_positions.back().second - m_positions.front().second;

  for (const auto &v : velocities) {
    avgVx += v.first;
    avgVy += v.second;
  }
  avgVx /= velocities.size();
  avgVy /= velocities.size();

  // Calculate velocity magnitude and direction angle
  double velocityMagnitude = std::sqrt(avgVx * avgVx + avgVy * avgVy);
  double angle = std::atan2(avgVy, avgVx) * 180.0 / M_PI;

  SPDLOG_DEBUG("Gesture analysis - totalDx: {:.2f}, totalDy: {:.2f}, velocity: "
               "{:.2f}, angle: {:.2f}°",
               totalDx, totalDy, velocityMagnitude, angle);

  // Check if movement is significant enough
  if (velocityMagnitude < MIN_SWIPE_DISTANCE) {
    SPDLOG_DEBUG("Movement too small to be considered a gesture");
    return Direction::None;
  }

  // Determine direction based on angle ranges (like use-gesture library)
  if (angle >= -45 && angle < 45) {
    SPDLOG_DEBUG("Detected Right gesture");
    return Direction::Right;
  } else if (angle >= 45 && angle < 135) {
    SPDLOG_DEBUG("Detected Down gesture");
    return Direction::Down;
  } else if (angle >= -135 && angle < -45) {
    SPDLOG_DEBUG("Detected Up gesture");
    return Direction::Up;
  } else {
    SPDLOG_DEBUG("Detected Left gesture");
    return Direction::Left;
  }
}

void GestureAnalyzer::clearPositions() { m_positions.clear(); }

bool GestureAnalyzer::isGestureInProgress() const {
  return !m_positions.empty();
}

} // namespace VirtualDesktop