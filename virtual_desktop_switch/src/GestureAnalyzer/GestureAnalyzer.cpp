#define _USE_MATH_DEFINES
#include "GestureAnalyzer.h"

namespace VirtualDesktop {

std::vector<std::vector<Point>> GestureAnalyzer::s_templates;

GestureAnalyzer::GestureAnalyzer() : m_rawPositions(), m_processedGesture(), m_useUnistroke(false) {
    initializeTemplates();
}

void GestureAnalyzer::addPosition(int32_t x, int32_t y) {
    // Filter out duplicate positions to reduce noise
    if (!m_rawPositions.empty() && m_rawPositions.back().first == x && m_rawPositions.back().second == y) {
        return;
    }
    m_rawPositions.emplace_back(x, y);
}

GestureAnalyzer::Direction GestureAnalyzer::analyzeGesture() const {
    if (m_useUnistroke) {
        return analyzeGestureUnistroke();
    } else {
        return analyzeGestureSimple();
    }
}

GestureAnalyzer::Direction GestureAnalyzer::analyzeGestureSimple() const {
    if (m_rawPositions.size() < 3) {  // Need at least 3 points for recognition
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

GestureAnalyzer::Direction GestureAnalyzer::analyzeGestureUnistroke() const {
    if (m_rawPositions.size() < 3) {  // Need at least 3 points for recognition
        return Direction::None;
    }

    // Convert positions to Point structure
    std::vector<Point> points;
    for (const auto& pos : m_rawPositions) {
        points.emplace_back(static_cast<double>(pos.first), static_cast<double>(pos.second));
    }

    // Process the gesture using $1 Unistroke Recognizer
    std::vector<Point> processedGesture = resample(points, NUM_POINTS);
    processedGesture = rotateBy(processedGesture, -indicativeAngle(processedGesture));
    processedGesture = scaleTo(processedGesture, DIAGONAL);
    processedGesture = translateTo(processedGesture, Point(0, 0));

    // Find the best matching template
    double bestDistance = std::numeric_limits<double>::max();
    int bestTemplateIndex = -1;

    for (size_t i = 0; i < s_templates.size(); ++i) {
        double distance = distanceAtBestAngle(processedGesture, s_templates[i]);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestTemplateIndex = static_cast<int>(i);
        }
    }

    // Threshold for recognition confidence
    const double threshold = 150.0;  // Adjusted to a more reasonable value

    if (bestTemplateIndex != -1 && bestDistance < threshold) {
        // Map template index to direction: 0=Right, 1=Left, 2=Down, 3=Up
        switch (bestTemplateIndex) {
            case 0:
                return Direction::Right;
            case 1:
                return Direction::Left;
            case 2:
                return Direction::Down;
            case 3:
                return Direction::Up;
            default:
                return Direction::None;
        }
    } else {
        return Direction::None;
    }
}

void GestureAnalyzer::clearPositions() {
    m_rawPositions.clear();
    m_processedGesture.clear();
}

bool GestureAnalyzer::isGestureInProgress() const {
    return !m_rawPositions.empty();
}

void GestureAnalyzer::setAlgorithm(bool useUnistroke) {
    m_useUnistroke = useUnistroke;
}

// $1 Unistroke Recognizer helper methods
std::vector<Point> GestureAnalyzer::resample(const std::vector<Point>& points, int n) const {
    if (points.empty())
        return points;

    std::vector<Point> newPoints;
    if (points.size() == 1) {
        newPoints.assign(n, points[0]);  // If only one point, duplicate it
        return newPoints;
    }

    double interval = pathLength(points) / (n - 1);
    newPoints.push_back(points[0]);  // Start with the first point

    std::vector<double> D;  // Distances between consecutive points
    for (size_t i = 1; i < points.size(); i++) {
        D.push_back(distance(points[i - 1], points[i]));
    }

    double accumulatedDistance = 0.0;  // Accumulated distance
    size_t currentPointIndex = 1;

    while (newPoints.size() < static_cast<size_t>(n)) {
        if (currentPointIndex >= points.size()) {
            // If we've run out of original points, add the last point
            newPoints.push_back(points.back());
            continue;
        }

        if ((accumulatedDistance + D[currentPointIndex - 1]) >= interval) {
            double ratio = (interval - accumulatedDistance) / D[currentPointIndex - 1];
            double qx = points[currentPointIndex - 1].x +
                    ratio * (points[currentPointIndex].x - points[currentPointIndex - 1].x);
            double qy = points[currentPointIndex - 1].y +
                    ratio * (points[currentPointIndex].y - points[currentPointIndex - 1].y);
            newPoints.emplace_back(qx, qy);
            accumulatedDistance = 0.0;  // Reset distance accumulator
        } else {
            accumulatedDistance += D[currentPointIndex - 1];
            currentPointIndex++;
        }
    }

    // Sometimes we don't get quite enough points, so add the last point
    while (newPoints.size() < static_cast<size_t>(n)) {
        newPoints.push_back(points.back());
    }

    // Trim to exactly n points if we have more
    if (newPoints.size() > static_cast<size_t>(n)) {
        newPoints.resize(n);
    }

    return newPoints;
}

double GestureAnalyzer::indicativeAngle(const std::vector<Point>& points) const {
    if (points.empty())
        return 0.0;

    Point c = centroid(points);
    return std::atan2(c.y - points[0].y, c.x - points[0].x);
}

std::vector<Point> GestureAnalyzer::rotateBy(const std::vector<Point>& points, double radians) const {
    double cos = std::cos(radians);
    double sin = std::sin(radians);
    Point c = centroid(points);

    std::vector<Point> newPoints;
    for (const auto& p : points) {
        double qx = (p.x - c.x) * cos - (p.y - c.y) * sin + c.x;
        double qy = (p.x - c.x) * sin + (p.y - c.y) * cos + c.y;
        newPoints.emplace_back(qx, qy);
    }

    return newPoints;
}

std::vector<Point> GestureAnalyzer::scaleTo(const std::vector<Point>& points, double size) const {
    // Calculate the bounding box
    if (points.empty())
        return points;

    double minX = points[0].x, maxX = points[0].x;
    double minY = points[0].y, maxY = points[0].y;

    for (const auto& p : points) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    double w = maxX - minX;
    double h = maxY - minY;

    // Maintain aspect ratio by using the maximum dimension
    double scale = (w > h) ? size / w : size / h;

    std::vector<Point> newPoints;
    for (const auto& p : points) {
        double qx = p.x * scale;
        double qy = p.y * scale;
        newPoints.emplace_back(qx, qy);
    }

    return newPoints;
}

std::vector<Point> GestureAnalyzer::translateTo(const std::vector<Point>& points, Point origin) const {
    Point c = centroid(points);
    std::vector<Point> newPoints;
    for (const auto& p : points) {
        double qx = p.x + origin.x - c.x;
        double qy = p.y + origin.y - c.y;
        newPoints.emplace_back(qx, qy);
    }
    return newPoints;
}

double GestureAnalyzer::distanceAtAngle(
        const std::vector<Point>& points,
        const std::vector<Point>& templatePoints,
        double radians) const {
    std::vector<Point> rotatedPoints = rotateBy(points, radians);
    return pathDistance(rotatedPoints, templatePoints);
}

double GestureAnalyzer::distanceAtBestAngle(const std::vector<Point>& points, const std::vector<Point>& templatePoints)
        const {
    // Convert angles from degrees to radians
    double degToRad = M_PI / 180.0;
    double angle1 = -ANGLE_RANGE * degToRad;
    double angle2 = ANGLE_RANGE * degToRad;
    double x1 = PHI * angle1 + (1.0 - PHI) * angle2;
    double f1 = distanceAtAngle(points, templatePoints, x1);
    double x2 = (1.0 - PHI) * angle1 + PHI * angle2;
    double f2 = distanceAtAngle(points, templatePoints, x2);

    double precisionRad = ANGLE_PRECISION * degToRad;  // Convert precision to radians too

    while (std::abs(angle2 - angle1) > precisionRad) {
        if (f1 < f2) {
            angle2 = x2;
            x2 = x1;
            f2 = f1;
            x1 = PHI * angle1 + (1.0 - PHI) * angle2;
            f1 = distanceAtAngle(points, templatePoints, x1);
        } else {
            angle1 = x1;
            x1 = x2;
            f1 = f2;
            x2 = (1.0 - PHI) * angle1 + PHI * angle2;
            f2 = distanceAtAngle(points, templatePoints, x2);
        }
    }

    return std::min(f1, f2);
}

double GestureAnalyzer::pathDistance(const std::vector<Point>& pts1, const std::vector<Point>& pts2) const {
    if (pts1.size() != pts2.size())
        return std::numeric_limits<double>::max();

    double sum = 0.0;
    for (size_t i = 0; i < pts1.size(); i++) {
        sum += distance(pts1[i], pts2[i]);
    }

    return sum / pts1.size();
}

Point GestureAnalyzer::centroid(const std::vector<Point>& points) const {
    if (points.empty())
        return Point(0, 0);

    double x = 0.0, y = 0.0;
    for (const auto& p : points) {
        x += p.x;
        y += p.y;
    }
    return Point(x / points.size(), y / points.size());
}

double GestureAnalyzer::pathLength(const std::vector<Point>& points) const {
    double length = 0.0;
    for (size_t i = 1; i < points.size(); i++) {
        length += distance(points[i - 1], points[i]);
    }
    return length;
}

double GestureAnalyzer::distance(const Point& p1, const Point& p2) const {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    return std::sqrt(dx * dx + dy * dy);
}

void GestureAnalyzer::initializeTemplates() {
    if (!s_templates.empty())
        return;  // Already initialized

    // Define basic gesture templates for Left, Right, Up, Down directions
    // These will be processed with the same transformations as input gestures

    // Right swipe template (from left to right)
    std::vector<Point> rawRightSwipe = {
            Point(0, 0),
            Point(50, 0),
            Point(100, 0),
            Point(150, 0),
            Point(200, 0),
            Point(250, 0),
            Point(300, 0),
            Point(350, 0),
            Point(400, 0),
            Point(450, 0)};
    // Process the template with the same transformations as input gestures
    std::vector<Point> rightSwipe = resample(rawRightSwipe, NUM_POINTS);
    rightSwipe = rotateBy(rightSwipe, -indicativeAngle(rightSwipe));
    rightSwipe = scaleTo(rightSwipe, DIAGONAL);
    rightSwipe = translateTo(rightSwipe, Point(0, 0));
    s_templates.push_back(rightSwipe);

    // Left swipe template (from right to left)
    std::vector<Point> rawLeftSwipe = {
            Point(450, 0),
            Point(400, 0),
            Point(350, 0),
            Point(300, 0),
            Point(250, 0),
            Point(200, 0),
            Point(150, 0),
            Point(100, 0),
            Point(50, 0),
            Point(0, 0)};
    std::vector<Point> leftSwipe = resample(rawLeftSwipe, NUM_POINTS);
    leftSwipe = rotateBy(leftSwipe, -indicativeAngle(leftSwipe));
    leftSwipe = scaleTo(leftSwipe, DIAGONAL);
    leftSwipe = translateTo(leftSwipe, Point(0, 0));
    s_templates.push_back(leftSwipe);

    // Down swipe template (from top to bottom)
    std::vector<Point> rawDownSwipe = {
            Point(0, 0),
            Point(0, 50),
            Point(0, 100),
            Point(0, 150),
            Point(0, 200),
            Point(0, 250),
            Point(0, 300),
            Point(0, 350),
            Point(0, 400),
            Point(0, 450)};
    std::vector<Point> downSwipe = resample(rawDownSwipe, NUM_POINTS);
    downSwipe = rotateBy(downSwipe, -indicativeAngle(downSwipe));
    downSwipe = scaleTo(downSwipe, DIAGONAL);
    downSwipe = translateTo(downSwipe, Point(0, 0));
    s_templates.push_back(downSwipe);

    // Up swipe template (from bottom to top)
    std::vector<Point> rawUpSwipe = {
            Point(0, 450),
            Point(0, 400),
            Point(0, 350),
            Point(0, 300),
            Point(0, 250),
            Point(0, 200),
            Point(0, 150),
            Point(0, 100),
            Point(0, 50),
            Point(0, 0)};
    std::vector<Point> upSwipe = resample(rawUpSwipe, NUM_POINTS);
    upSwipe = rotateBy(upSwipe, -indicativeAngle(upSwipe));
    upSwipe = scaleTo(upSwipe, DIAGONAL);
    upSwipe = translateTo(upSwipe, Point(0, 0));
    s_templates.push_back(upSwipe);
}

}  // namespace VirtualDesktop
