#pragma once
#include <cstdint>
#include <vector>

namespace VirtualDesktop {

/**
 * @brief Analyzes mouse gestures to detect swipe directions
 */
class GestureAnalyzer {
public:
  /**
   * @brief Possible gesture directions
   */
  enum class Direction { Left, Right, Up, Down, None };

  /**
   * @brief Adds a new mouse position to the gesture analysis
   * @param x The x coordinate of mouse position
   * @param y The y coordinate of mouse position
   */
  void addPosition(int32_t x, int32_t y);

  /**
   * @brief Analyzes the collected positions to detect gesture direction
   * @return Detected gesture direction
   */
  Direction analyzeGesture() const;

  /**
   * @brief Clears all collected positions
   */
  void clearPositions();

  /**
   * @brief Checks if a gesture is currently in progress
   * @return true if positions have been recorded (gesture in progress)
   * @return false if no positions recorded (no active gesture)
   */
  bool isGestureInProgress() const;

private:
  std::vector<std::pair<int32_t, int32_t>> m_positions;
  static constexpr int32_t MIN_SWIPE_DISTANCE = 100;
};

} // namespace VirtualDesktop