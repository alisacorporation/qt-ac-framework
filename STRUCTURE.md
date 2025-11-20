# Project Structure

This document describes the organization of the Qt AC Framework codebase.

## Directory Layout

```
qt-ac-framework/
├── src/                          # Source files
│   ├── main.cpp                  # Application entry point
│   ├── core/                     # Core business logic
│   │   ├── SystemMonitor.cpp     # Local system monitoring
│   │   ├── DiskAnalyzer.cpp      # Disk usage analysis
│   │   └── ServerManager.cpp     # Remote server management
│   └── ui/                       # User interface
│       ├── views/                # Main application views
│       │   ├── Main.qml          # Dashboard view
│       │   ├── DiskCleanup.qml   # Disk cleanup view
│       │   └── RemoteMonitor.qml # Remote server monitoring view
│       └── components/           # Reusable UI components
│           ├── Sidebar.qml       # Navigation sidebar
│           ├── StatCircle.qml    # Circular stat gauge
│           ├── NetworkCircle.qml # Network gauge (dual)
│           └── SparkLine.qml     # Mini line chart
├── include/                      # Header files
│   ├── SystemMonitor.h
│   ├── DiskAnalyzer.h
│   └── ServerManager.h
├── build/                        # Build artifacts (generated)
├── screenshots/                  # Application screenshots
├── CMakeLists.txt               # CMake build configuration
├── README.md                    # Project documentation
├── PROJECT_STATUS.md            # Development status
└── STRUCTURE.md                 # This file

```

## Component Descriptions

### Core (`src/core/`)
Business logic and system interaction layer:
- **SystemMonitor**: Monitors local system resources (CPU, RAM, Disk, Network)
- **DiskAnalyzer**: Analyzes disk usage and provides cleanup suggestions
- **ServerManager**: Manages SSH connections to remote servers and fetches their stats

### UI Views (`src/ui/views/`)
Main application screens:
- **Main.qml**: Dashboard with system overview, health score, and info cards
- **DiskCleanup.qml**: Disk analysis and cleanup interface
- **RemoteMonitor.qml**: Remote server monitoring and management

### UI Components (`src/ui/components/`)
Reusable interface elements:
- **Sidebar.qml**: Navigation menu
- **StatCircle.qml**: Circular progress gauge for single metrics
- **NetworkCircle.qml**: Dual-ring gauge for network TX/RX
- **SparkLine.qml**: Mini line chart for historical data (30s)

## Build System

The project uses CMake with Qt 6.8. Key features:
- Automatic MOC (Meta-Object Compiler) for Qt classes
- QML module system with ahead-of-time compilation
- Organized include paths for clean imports

## Coding Conventions

- **C++ Headers**: Located in `include/`
- **C++ Implementation**: Located in `src/core/`
- **QML Views**: Located in `src/ui/views/`
- **QML Components**: Located in `src/ui/components/`
- **Naming**: PascalCase for files and classes

## Adding New Features

1. **New Core Feature**: Add `.h` to `include/`, `.cpp` to `src/core/`
2. **New View**: Add `.qml` to `src/ui/views/`
3. **New Component**: Add `.qml` to `src/ui/components/`
4. **Update CMakeLists.txt**: Add new files to appropriate sections

## Dependencies

- Qt 6.8+ (Quick module)
- C++17 or later
- CMake 3.16+
