# Umbrella Corp. System Monitor (Qt SLR)

A futuristic system monitoring dashboard inspired by the Resident Evil series "Umbrella Corporation" aesthetic.

## Features
- **Real-time Monitoring**: Displays CPU, RAM, Disk, and Network usage.
- **Umbrella Aesthetic**: Dark theme, deep red accents, glowing effects.
- **Modular Design**: Built with Qt 6 C++ and QML for performance and extensibility.
- **Demo Mode**: Currently runs with simulated data for UI testing.

## Requirements
- Qt 6.8 or higher
- CMake 3.16+
- C++20 compatible compiler

## Build Instructions
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Project Structure
- **SystemMonitor.cpp/h**: C++ Backend for data metrics.
- **Main.qml**: Main application entry and layout.
- **Sidebar.qml**: Navigation component.
- **StatCircle.qml**: Custom reusable gauge component.
