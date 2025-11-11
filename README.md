# Virtual Desktop Switcher

A Windows utility that enables virtual desktop switching through mouse side-button gestures.

## Features
- **Configurable Gesture-based Switching**: Use any mouse button (X1, X2, Left, Right) with swipe gestures to switch virtual desktops
- **Real-time Visual Feedback**: Overlay displays gesture trajectory with customizable color and line width
- **Multi-renderer Support**: Choose between GDI+ and Direct2D rendering engines
- **System Tray Integration**: Tray icon with configurable auto-start and menu controls
- **Customizable Settings**: JSON-based configuration with multiple parameter options
- **Smooth Gesture Recognition**: Advanced algorithms for accurate gesture detection
- **Thread-safe Mouse Hook**: Singleton pattern implementation for reliable event capture

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
├── QWEN.md                     # Project context for AI assistants
├── doc/                        # Documentation
│   ├── config_editor.html      # Configuration editor UI
│   ├── setting_ui.md           # Settings UI documentation
│   ├── virtual_desktop_switcher_architecture.md # Architecture documentation
│   └── virtual_desktop_switcher_requirements.md # Requirements documentation
├── app/                        # Application-specific code
│   ├── app.cpp                 # Main application logic
│   ├── app.h                   # Application header
│   ├── CMakeLists.txt          # App module CMake configuration
│   ├── main.cpp                # Application entry point
│   ├── Resource/               # Application resources
│   ├── TrayIcon.cpp            # System tray functionality
│   └── TrayIcon.h              # System tray header
├── core/                       # Core functionality modules
│   ├── CMakeLists.txt          # Core module CMake configuration
│   ├── include/                # Core public headers
│   │   ├── DesktopManager.h    # Virtual desktop operations
│   │   ├── GestureAnalyzer.h   # Gesture recognition logic
│   │   ├── IRenderer.h         # Renderer interface
│   │   ├── MouseHook.h         # Mouse hook interface
│   │   ├── OverlayUI.h         # Overlay UI interface
│   │   ├── Settings.h          # Settings management
│   │   ├── utils.h             # Utility functions
│   │   └── VirtualDesktopSwitcher.h # Main Virtual Desktop Switcher interface
│   └── src/                    # Core implementation files
│       ├── D2DRenderer.cpp     # Direct2D implementation
│       ├── D2DRenderer.h       # Direct2D header
│       ├── DesktopManager.cpp  # Desktop management implementation
│       ├── GdiRenderer.cpp     # GDI implementation
│       ├── GdiRenderer.h       # GDI header
│       ├── GestureAnalyzer.cpp # Gesture analysis implementation
│       ├── MouseHook.cpp       # Mouse hook implementation
│       ├── OverlayUI.cpp       # Overlay UI implementation
│       ├── RendererFactory.cpp # Factory for renderer creation
│       ├── Settings.cpp        # Settings implementation
│       └── utils.cpp           # Utility functions
├── third_party/                # External dependencies
│   └── nlohmann/               # JSON library
│       └── json.hpp            # Single header JSON library
├── build/                      # Build output directory
├── .cache/                     # Cached files (clangd, etc.)
├── .comate/                    # Comate configuration
├── .vscode/                    # VSCode settings
└── .clang-format               # C/C++ formatting configuration
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
  "basic": {
    "auto_start": false,
    "tray_icon": true
  },
  "gesture": {
    "trigger_button": "X1",
    "sensitivity": 5,
    "line_width": 5,
    "color": "#6495EDAA"
  },
  "rendering": {
    "mode": "GDI+",
    "transparency": 80
  },
  "behavior": {
    "desktop_cycle": true,
    "desktop_preview": true,
    "switch_animation": true
  }
}
```

### Configuration Details
- **auto_start**: Whether the application should start automatically with Windows (default: false)
- **tray_icon**: Whether to show the application in system tray (default: true)
- **trigger_button**: Mouse button to use for triggering gestures (options: "X1", "X2", "Left", "Right", "None") (default: "X1")
- **sensitivity**: Sensitivity level for gesture recognition (1-10, default: 5)
- **line_width**: Thickness of the gesture trail visualization (1-10, default: 5)
- **color**: Color of the gesture trail visualization in #RRGGBBAA format (Red, Green, Blue, Alpha) (default: "#6495EDAA")
- **rendering.mode**: Rendering engine to use ("GDI+" or "Direct2D") (default: "GDI+")
- **transparency**: Transparency level for the overlay (0-100, default: 80)
- **desktop_cycle**: Whether to cycle from last to first desktop (default: true)
- **desktop_preview**: Whether to show desktop previews during switching (default: true)
- **switch_animation**: Whether to show animations during desktop switching (default: true)

## Usage
1. Build the application using the instructions above
2. Run the generated executable
3. The application will start minimized to system tray
4. Use the configured mouse button (default is X1, or the first mouse side button) with swipe gestures:
   - Press and hold the trigger button, then swipe left: Switch to previous virtual desktop
   - Press and hold the trigger button, then swipe right: Switch to next virtual desktop
5. The trigger button can be configured in the config.json file (options: "X1", "X2", "Left", "Right", or "None")

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
- **Windows API Components**:
  - user32.lib: User interface functionality
  - gdi32.lib: GDI graphics functionality
  - d2d1.lib: Direct2D graphics functionality
  - shell32.lib: Shell functionality
  - Shcore.lib: System core functionality
  - shlwapi.lib: Shell utility functions and path manipulation
  - advapi32.lib: Advanced Windows API functionality
  - ShellScalingApi.h: DPI awareness
  - Shlwapi.h: Path manipulation and utility functions
- **Core Windows API**: System-level functionality for mouse hooks and desktop management

## License
This project is provided for educational and development purposes.