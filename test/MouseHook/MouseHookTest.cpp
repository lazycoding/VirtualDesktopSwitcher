#include "MouseHook/MouseHook.h"
#include <gtest/gtest.h>

class MouseHookTest : public ::testing::Test {
protected:
  void SetUp() override { hook = &VirtualDesktop::MouseHook::GetInstance(); }

  void TearDown() override { hook->Shutdown(); }

  VirtualDesktop::MouseHook *hook;
};

TEST_F(MouseHookTest, Initialization) {
  EXPECT_TRUE(hook->Initialize());
  EXPECT_TRUE(hook->Initialize()); // Test double initialization
}

TEST_F(MouseHookTest, Shutdown) {
  hook->Initialize();
  hook->Shutdown();
  EXPECT_NO_THROW(hook->Shutdown()); // Test double shutdown
}

TEST_F(MouseHookTest, Singleton) {
  auto &hook2 = VirtualDesktop::MouseHook::GetInstance();
  EXPECT_EQ(hook, &hook2);
}

TEST_F(MouseHookTest, CallbackRegistration) {
  bool callbackCalled = false;
  auto callback = [&](int, WPARAM, LPARAM) { callbackCalled = true; };

  hook->Initialize();
  hook->addCallback(callback);

  // Simulate mouse event
  MSLLHOOKSTRUCT mockEvent = {};
  hook->HookCallback(HC_ACTION, WM_MOUSEMOVE,
                     reinterpret_cast<LPARAM>(&mockEvent));

  EXPECT_TRUE(callbackCalled);
}

TEST_F(MouseHookTest, RemoveCallbacks) {
  bool callbackCalled = false;
  auto callback = [&](int, WPARAM, LPARAM) { callbackCalled = true; };

  hook->Initialize();
  hook->addCallback(callback);
  hook->removeCallbacks();

  // Simulate mouse event
  MSLLHOOKSTRUCT mockEvent = {};
  hook->HookCallback(HC_ACTION, WM_MOUSEMOVE,
                     reinterpret_cast<LPARAM>(&mockEvent));

  EXPECT_FALSE(callbackCalled);
}