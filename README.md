# Virtual Desktop Switcher

A Windows utility that enables virtual desktop switching through mouse side-button gestures.

## Features
- **Gesture-based Switching**: Use mouse side-button (XBUTTON1) with swipe gestures to switch virtual desktops
- **Real-time Visual Feedback**: Direct2D overlay displays gesture trajectory
- **System Tray Integration**: Tray icon with notification support and menu controls
- **Customizable Settings**: JSON-based configuration for sensitivity and appearance
- **Singleton Architecture**: Thread-safe mouse hook implementation

## System Requirements
- Windows 10/11 (64-bit)
- CMake 3.15+
- Visual Studio 2019+ or compatible C++17 compiler
- Windows SDK 10.0.19041.0+

## Project Structure
```
.
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # Project documentation
├── .gitignore                  # Git ignore rules
├── doc/                        # Documentation
│   ├── virtual_desktop_switcher_architecture.md
│   └── virtual_desktop_switcher_requirements.md
├── src/                        # Source code (modular structure)
│   ├── main.cpp                # Application entry point
│   ├── DesktopManager/         # Virtual desktop operations
│   ├── GestureAnalyzer/        # Mouse gesture recognition
│   ├── MouseHook/              # Low-level mouse event capture
│   ├── OverlayUI/              # Direct2D gesture visualization
│   ├── Settings/               # JSON configuration management
│   ├── TrayIcon/               # System tray integration
│   └── res/                    # Application resources
│       └── favicon.ico         # Application icon
├── third_party/                # External dependencies
│   └── nlohmann/               # JSON library
│       └── json.hpp
└── build/                      # Build output directory
```

## Build Instructions

### Prerequisites
- Install Visual Studio 2019 or later with C++ development tools
- Ensure Windows SDK is installed
- Install CMake 3.15 or later

### Build Steps
```bash
# Create build directory
mkdir build
cd build

# Configure project
cmake ..

# Build release version
cmake --build . --config Release

# Optional: Build debug version
cmake --build . --config Debug
```

## Configuration
The application uses `config.json` for settings. Default location: Same directory as executable (config.json)

### Available Settings
```json
{
  "gesture_line_width": 5,
  "overlay_color": "#FF0000FF"
}
```

### Configuration Details
- **gesture_line_width**: Controls the thickness of the gesture trail visualization
- **overlay_color**: Sets the color of the gesture trail visualization in #RRGGBBAA format (Red, Green, Blue, Alpha). 
  - The MouseTrailRenderer uses this color for the gesture trajectory visualization.
  - The alpha channel controls the transparency (00 = fully transparent, FF = fully opaque).
  - The line width of the gesture trail is also dynamically adjusted based on the sensitivity setting.

## Usage
1. Build the application using the instructions above
2. Run the generated executable
3. The application will start minimized to system tray
4. Use mouse side-button (usually back button) with swipe gestures:
   - Swipe left: Switch to previous virtual desktop
   - Swipe right: Switch to next virtual desktop

## Development Guidelines
This project follows modern C++ best practices:
- C++17 standard compliance
- SOLID design principles
- RAII resource management
- Singleton and Observer patterns
- Doxygen-style documentation
- Modular architecture with clear separation of concerns

## Dependencies
- **nlohmann/json**: Header-only JSON library for configuration management
- **Windows API Extensions**:
  - ShellScalingApi.h (DPI awareness)
  - Shlwapi.h (path manipulation)
  - Libraries: shlwapi.lib
- **Core Windows API**: System-level functionality

## License
This project is provided for educational and development purposes.