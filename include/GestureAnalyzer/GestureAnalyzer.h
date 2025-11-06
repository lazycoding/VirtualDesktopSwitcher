#pragma once
#define _USE_MATH_DEFINES
#include <cstdint>
#include <vector>
#include <utility>
#include <cmath>

namespace VirtualDesktop {

// Structure to represent a 2D point for gesture recognition
struct Point {
    double x;
    double y;
    Point(double x = 0.0, double y = 0.0) : x(x), y(y) {}
    
    // Operators needed for vector math
    Point operator+(const Point& other) const {
        return Point(x + other.x, y + other.y);
    }
    
    Point operator-(const Point& other) const {
        return Point(x - other.x, y - other.y);
    }
    
    Point operator*(double scalar) const {
        return Point(x * scalar, y * scalar);
    }
    
    Point operator/(double scalar) const {
        return Point(x / scalar, y / scalar);
    }
    
    Point& operator+=(const Point& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    Point& operator-=(const Point& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    
    Point& operator*=(double scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    
    Point& operator/=(double scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }
};

/**
 * @brief Analyzes mouse gestures to detect swipe directions using $1 Unistroke Recognizer
 */
class GestureAnalyzer {
public:
    /**
     * @brief Possible gesture directions
     */
    enum class Direction { Left, Right, Up, Down, None };

    /**
     * @brief Constructor that initializes the gesture templates
     */
    GestureAnalyzer();

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
    // $1 Unistroke Recognizer constants
    static constexpr int NUM_POINTS = 64;      // Number of points to resample each gesture to
    static constexpr double ANGLE_RANGE = 45.0; // Angle range for rotation in degrees
    static constexpr double ANGLE_PRECISION = 2.0; // Angle precision for search in degrees
    static constexpr double DIAGONAL = 250.0;   // Square root of 250^2 + 250^2 (bounding box size)
    static constexpr double HALF_DIAGONAL = 125.0; // Half of the diagonal
    static constexpr double PHI = 0.618033988; // Golden ratio - 1 (0.5 * (-1.0 + std::sqrt(5.0)) calculated)

    std::vector<std::pair<int32_t, int32_t>> m_rawPositions;
    std::vector<Point> m_processedGesture;
    static std::vector<std::vector<Point>> s_templates; // Predefined gesture templates

    // $1 Unistroke Recognizer methods
    std::vector<Point> resample(const std::vector<Point>& points, int n) const;
    double indicativeAngle(const std::vector<Point>& points) const;
    std::vector<Point> rotateBy(const std::vector<Point>& points, double radians) const;
    std::vector<Point> scaleTo(const std::vector<Point>& points, double size) const;
    std::vector<Point> translateTo(const std::vector<Point>& points, Point origin) const;
    double distanceAtAngle(const std::vector<Point>& points, const std::vector<Point>& templatePoints, double radians) const;
    double distanceAtBestAngle(const std::vector<Point>& points, const std::vector<Point>& templatePoints) const;
    double pathDistance(const std::vector<Point>& pts1, const std::vector<Point>& pts2) const;
    Point centroid(const std::vector<Point>& points) const;
    double pathLength(const std::vector<Point>& points) const;
    double distance(const Point& p1, const Point& p2) const;

    // Initialize predefined gesture templates
    void initializeTemplates();
};

}  // namespace VirtualDesktop