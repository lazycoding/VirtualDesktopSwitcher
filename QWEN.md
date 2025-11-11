# Virtual Desktop Switcher Development Context

## Project Overview
Virtual Desktop Switcher is a Windows utility that enables virtual desktop switching through mouse side-button gestures. It provides real-time visual feedback via Direct2D overlay and integrates with the system tray.

## Current Directory Structure
```
.
├── .clang-format                 # C/C++ formatting configuration
├── .gitignore                    # Git ignore rules
├── CMakeLists.txt                # Root CMake build configuration
├── QWEN.md                       # This file - project context for AI assistants
├── README.md                     # Project documentation
├── .cache/                       # Cached files (clangd, etc.)
├── .comate/                      # Comate configuration
├── .git/                         # Git repository metadata
├── .vscode/                      # VSCode settings
├── app/                          # Application-specific code
│   ├── app.cpp                   # Main application logic
│   ├── app.h                     # Application header
│   ├── CMakeLists.txt            # App module CMake configuration
│   ├── main.cpp                  # Application entry point
│   ├── Resource/                 # Application resources
│   ├── TrayIcon.cpp              # System tray functionality
│   └── TrayIcon.h                # System tray header
├── build/                        # Build output directory (ignored by Git)
├── core/                         # Core functionality modules
│   ├── CMakeLists.txt            # Core module CMake configuration
│   ├── include/                  # Core public headers
│   │   ├── DesktopManager.h      # Virtual desktop operations
│   │   ├── GestureAnalyzer.h     # Gesture recognition logic
│   │   ├── IRenderer.h           # Renderer interface
│   │   ├── MouseHook.h           # Mouse hook interface
│   │   ├── OverlayUI.h           # Overlay UI interface
│   │   ├── Settings.h            # Settings management
│   │   ├── utils.h               # Utility functions
│   │   └── VirtualDesktopSwitcher.h # Main Virtual Desktop Switcher interface
│   └── src/                      # Core implementation files
│       ├── D2DRenderer.cpp       # Direct2D implementation
│       ├── D2DRenderer.h         # Direct2D header
│       ├── DesktopManager.cpp    # Desktop management implementation
│       ├── GdiRenderer.cpp       # GDI implementation
│       ├── GdiRenderer.h         # GDI header
│       ├── GestureAnalyzer.cpp   # Gesture analysis implementation
│       ├── MouseHook.cpp         # Mouse hook implementation
│       ├── OverlayUI.cpp         # Overlay UI implementation
│       ├── RendererFactory.cpp   # Factory for renderer creation
│       └── Settings.cpp          # Settings implementation
├── doc/                          # Documentation files
│   ├── config_editor.html        # Configuration editor UI
│   ├── setting_ui.md             # Settings UI documentation
│   ├── virtual_desktop_switcher_architecture.md # Architecture documentation
│   └── virtual_desktop_switcher_requirements.md # Requirements documentation
└── third_party/                  # External dependencies
    └── nlohmann/                 # JSON library
        └── json.hpp              # Single header JSON library
```

## Key Features
- **Gesture-based Switching**: Use mouse side-button (XBUTTON1) with swipe gestures to switch virtual desktops
- **Real-time Visual Feedback**: Direct2D overlay displays gesture trajectory
- **System Tray Integration**: Tray icon with notification support and menu controls
- **Customizable Settings**: JSON-based configuration for sensitivity and appearance
- **Singleton Architecture**: Thread-safe mouse hook implementation

## Technologies Used
- **C++17**: Modern C++ standard for implementation
- **Windows API**: System-level functionality
- **Direct2D**: Graphics rendering for gesture visualization
- **nlohmann/json**: Header-only JSON library for configuration management
- **CMake**: Build system
- **Windows SDK**: System integration

## Build Requirements
- Windows 10/11 (64-bit)
- CMake 3.15+
- Visual Studio 2019+ or compatible C++17 compiler
- Windows SDK 10.0.19041.0+

## Build Instructions
```bash
# Create build directory
mkdir build
cd build

# Configure project
cmake ..

# Build release version
cmake --build . --config Release
```

## Configuration
The application uses `config.json` for settings, located in the same directory as the executable.

## Important Files and Modules
- **App Module** (`app/`): Contains the main application logic and entry point
- **Core Module** (`core/`): Contains all the core functionality including desktop management, gesture analysis, mouse hooks, and rendering
- **Settings** (`core/src/Settings.cpp`, `core/include/Settings.h`): Handles JSON-based configuration
- **Mouse Hook** (`core/src/MouseHook.cpp`, `core/include/MouseHook.h`): Captures mouse events
- **Overlay UI** (`core/src/OverlayUI.cpp`, `core/include/OverlayUI.h`): Manages the visual overlay for gestures
- **Gesture Analyzer** (`core/src/GestureAnalyzer.cpp`, `core/include/GestureAnalyzer.h`): Analyzes mouse movement patterns to detect gestures
- **Desktop Manager** (`core/src/DesktopManager.cpp`, `core/include/DesktopManager.h`): Manages Windows virtual desktop operations
- **Renderers** (`core/src/D2DRenderer.cpp`, `core/src/GdiRenderer.cpp`): Different rendering implementations for gesture visualization

## Development Guidelines
- Follow modern C++ best practices with C++17 standard
- Use RAII resource management
- Implement SOLID design principles
- Apply Singleton and Observer patterns where appropriate
- Include Doxygen-style documentation
- Maintain modular architecture with clear separation of concerns