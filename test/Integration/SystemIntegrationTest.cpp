#include "DesktopManager/DesktopManager.h"
#include "GestureAnalyzer/GestureAnalyzer.h"
#include "MouseHook/MouseHook.h"
#include "OverlayUI/OverlayUI.h"
#include "Settings/Settings.h"
#include <gtest/gtest.h>


class SystemIntegrationTest : public ::testing::Test {
protected:
  void SetUp() override {
    settings.load("config.json");

    if (!overlay.initialize(nullptr)) {
      GTEST_SKIP() << "Failed to initialize overlay UI";
    }

    if (!mouseHook.Initialize()) {
      GTEST_SKIP() << "Failed to initialize mouse hook";
    }

    mouseHook.addCallback([this](int, WPARAM wParam, LPARAM lParam) {
      if (wParam == WM_MOUSEMOVE) {
        auto *mouseData = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
        gestureAnalyzer.addPosition(mouseData->pt.x, mouseData->pt.y);
      }
    });
  }

  void TearDown() override { mouseHook.Shutdown(); }

  VirtualDesktop::Settings settings;
  VirtualDesktop::MouseHook &mouseHook =
      VirtualDesktop::MouseHook::GetInstance();
  VirtualDesktop::GestureAnalyzer gestureAnalyzer;
  VirtualDesktop::DesktopManager desktopManager;
  VirtualDesktop::OverlayUI overlay;
};

TEST_F(SystemIntegrationTest, FullWorkflow) {
  // Simulate right swipe gesture
  gestureAnalyzer.addPosition(0, 50);
  gestureAnalyzer.addPosition(100, 50);

  auto direction = gestureAnalyzer.analyzeGesture();
  ASSERT_EQ(direction, VirtualDesktop::GestureAnalyzer::Direction::Right);

  EXPECT_TRUE(desktopManager.switchDesktop(true));

  // Verify overlay rendering
  std::vector<POINT> points = {{0, 50}, {100, 50}};
  EXPECT_NO_THROW(overlay.render(points));
}

TEST_F(SystemIntegrationTest, ConfigurationChange) {
  settings.setGestureSensitivity(8);
  ASSERT_EQ(settings.getGestureSensitivity(), 8);

  settings.setOverlayColor("#FF0000AA");
  ASSERT_EQ(settings.getOverlayColor(), "#FF0000AA");
}