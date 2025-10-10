#include "Settings.h"
#include <ShlObj.h>
#include <codecvt>
#include <d2d1.h>
#include <fstream>


using json = nlohmann::json;

std::wstring Settings::GetConfigPath() const {
  PWSTR appDataPath = nullptr;
  std::wstring configPath;

  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr,
                                     &appDataPath))) {
    configPath = appDataPath;
    configPath += L"\\VirtualDesktopSwitcher\\config.json";
    CoTaskMemFree(appDataPath);
  }

  return configPath;
}

bool Settings::Load() {
  std::wstring path = GetConfigPath();
  if (path.empty())
    return false;

  try {
    std::ifstream file(path);
    if (file.is_open()) {
      jsonData = json::parse(file);

      // 解析配置
      config.sensitivity = jsonData.value("sensitivity", 50);
      config.threshold = jsonData.value("threshold", 5);
      config.enabled = jsonData.value("enabled", true);
      config.startWithWindows = jsonData.value("startWithWindows", false);

      if (jsonData.contains("trailColor")) {
        auto &color = jsonData["trailColor"];
        config.trailColor =
            D2D1::ColorF(color.value("r", 0.0f), color.value("g", 0.0f),
                         color.value("b", 1.0f), color.value("a", 1.0f));
      }

      config.trailWidth = jsonData.value("trailWidth", 2.0f);
      config.fadeSpeed = jsonData.value("fadeSpeed", 0.1f);

      return true;
    }
  } catch (...) {
    // 解析失败使用默认配置
  }

  return false;
}

bool Settings::Save() {
  std::wstring path = GetConfigPath();
  if (path.empty())
    return false;

  // 创建配置目录
  size_t pos = path.find_last_of(L'\\');
  if (pos != std::wstring::npos) {
    std::wstring dir = path.substr(0, pos);
    CreateDirectory(dir.c_str(), nullptr);
  }

  try {
    // 更新JSON数据
    jsonData["sensitivity"] = config.sensitivity;
    jsonData["threshold"] = config.threshold;
    jsonData["enabled"] = config.enabled;
    jsonData["startWithWindows"] = config.startWithWindows;

    jsonData["trailColor"] = {{"r", config.trailColor.r},
                              {"g", config.trailColor.g},
                              {"b", config.trailColor.b},
                              {"a", config.trailColor.a}};

    jsonData["trailWidth"] = config.trailWidth;
    jsonData["fadeSpeed"] = config.fadeSpeed;

    // 写入文件
    std::ofstream file(path);
    if (file.is_open()) {
      file << jsonData.dump(4);
      return true;
    }
  } catch (...) {
    // 保存失败
  }

  return false;
}

void Settings::UpdateConfig(const Config &newConfig) {
  config = newConfig;
  Save();
}