#include "Settings.h"
#include <fstream>
#include <stdexcept>

namespace VirtualDesktop {

namespace {
constexpr const char *DEFAULT_CONFIG = R"({
        "gesture_sensitivity": 5,
        "overlay_color": "#6495EDAA"
    })";
}

Settings &Settings::GetInstance() {
  static Settings instance;
  return instance;
}

bool Settings::load(const std::string &filePath) {
  try {
    std::ifstream file(filePath);
    if (!file.is_open()) {
      m_config = nlohmann::json::parse(DEFAULT_CONFIG);
      return false;
    }
    m_config = nlohmann::json::parse(file);
    return true;
  } catch (const std::exception &) {
    m_config = nlohmann::json::parse(DEFAULT_CONFIG);
    return false;
  }
}

bool Settings::save(const std::string &filePath) const {
  try {
    std::ofstream file(filePath);
    if (!file.is_open()) {
      return false;
    }
    file << m_config.dump(4);
    return true;
  } catch (const std::exception &) {
    return false;
  }
}

int Settings::getGestureSensitivity() const {
  return m_config.value("gesture_sensitivity", 5);
}

void Settings::setGestureSensitivity(int value) {
  m_config["gesture_sensitivity"] = std::clamp(value, 1, 10);
}

std::string Settings::getOverlayColor() const {
  return m_config.value("overlay_color", "#6495EDAA");
}

void Settings::setOverlayColor(const std::string &color) {
  if (color.size() == 9 && color[0] == '#') {
    m_config["overlay_color"] = color;
  }
}

} // namespace VirtualDesktop