#include "GestureAnalyzer/GestureAnalyzer.h"
#include <gtest/gtest.h>

class GestureAnalyzerTest : public ::testing::Test {
protected:
  void SetUp() override { analyzer = VirtualDesktop::GestureAnalyzer(); }

  VirtualDesktop::GestureAnalyzer analyzer;
};

TEST_F(GestureAnalyzerTest, EmptyGesture) {
  EXPECT_EQ(analyzer.analyzeGesture(),
            VirtualDesktop::GestureAnalyzer::Direction::None);
}

TEST_F(GestureAnalyzerTest, LeftSwipe) {
  analyzer.addPosition(100, 50);
  analyzer.addPosition(50, 50);
  analyzer.addPosition(0, 50);
  EXPECT_EQ(analyzer.analyzeGesture(),
            VirtualDesktop::GestureAnalyzer::Direction::Left);
}

TEST_F(GestureAnalyzerTest, RightSwipe) {
  analyzer.addPosition(0, 50);
  analyzer.addPosition(50, 50);
  analyzer.addPosition(100, 50);
  EXPECT_EQ(analyzer.analyzeGesture(),
            VirtualDesktop::GestureAnalyzer::Direction::Right);
}

TEST_F(GestureAnalyzerTest, InsufficientDistance) {
  analyzer.addPosition(0, 50);
  analyzer.addPosition(20, 50);
  EXPECT_EQ(analyzer.analyzeGesture(),
            VirtualDesktop::GestureAnalyzer::Direction::None);
}

TEST_F(GestureAnalyzerTest, ClearPositions) {
  analyzer.addPosition(0, 50);
  analyzer.addPosition(100, 50);
  analyzer.clearPositions();
  EXPECT_EQ(analyzer.analyzeGesture(),
            VirtualDesktop::GestureAnalyzer::Direction::None);
}