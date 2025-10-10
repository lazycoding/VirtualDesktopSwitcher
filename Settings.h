#pragma once
#include <nlohmann/json.hpp>
#include <string>


class Settings {
public:
  struct Config {
    int sensitivity = 50;
    int threshold = 5;
    bool enabled = true;
    bool startWithWindows = false;
    D2D1::ColorF trailColor = D2D1::ColorF(D2D1::ColorF::Blue);
    float trailWidth = 2.0f;
    float fadeSpeed = 0.1f;
  };

  bool Load();
  bool Save();

  const Config &GetConfig() const { return config; }
  void UpdateConfig(const Config &newConfig);

private:
  std::wstring GetConfigPath() const;

  Config config;
  nlohmann::json jsonData;
};