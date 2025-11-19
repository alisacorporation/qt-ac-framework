# Umbrella Corp. System Monitor - Project Documentation

## 1. Project Overview
**Goal**: Create a Qt-based desktop application featuring a futuristic UI/UX inspired by the Umbrella Corporation's aesthetic from the Resident Evil series.
**Vibe**: Dark red/black palette, biohazard motifs, sleek gradients, glowing accents, high-tech laboratory atmosphere.

## 2. Core Functionality
- **Central Management Dashboard**: Real-time system monitoring.
- **Local PC Statistics**: CPU usage, RAM utilization, disk space, network I/O.
- **Remote Server Support**: Monitor remote servers with similar metrics (Planned).
- **Architecture**: Modular, extensible for future features (SSH, remote admin).

## 3. UI/UX Requirements
- **Theme**: Dark (#050505) with deep crimson (#8B0000) accents.
- **Style**: Futuristic, "beautiful control panel", professional polish.
- **Elements**:
  - Clean, hierarchical navigation (Sidebar).
  - Dashboard-style interface with glowing charts/gauges.
  - Responsive layout.
  - Status indicators (Green/Yellow/Red).
- **Navigation**: Full-width, seamless menu items with hover effects.

## 4. Technical Specifications
- **Framework**: Qt 6.x (C++ and QML).
- **Platform**: Windows, Linux, macOS (Cross-platform).
- **Backend**: C++ for high-performance data gathering (SystemMonitor class).
- **Frontend**: QML for hardware-accelerated, fluid UI.
- **Data Source**: 
  - Currently: Simulated data (Demo Mode).
  - Planned: `QSystemTrayIcon`, Windows Performance Counters, Linux `/proc`.

## 5. Architecture
### Frontend (QML)
- **`Main.qml`**: Application entry point, layout management (`StackLayout`).
- **`Sidebar.qml`**: Collapsible/Fixed navigation bar.
- **`StatCircle.qml`**: Reusable circular gauge component for percentages.
- **`SystemMonitor` (QML Type)**: Interface to C++ backend.

### Backend (C++)
- **`SystemMonitor` Class**:
  - Inherits `QObject`.
  - Properties: `cpuUsage`, `ramUsage`, `diskUsage`, `networkUp`, `networkDown`.
  - Uses `QTimer` for periodic updates (1s interval).
  - Emits `statsUpdated()` signal.

## 6. Current Status
- [x] **Project Setup**: CMake configuration with Qt 6.
- [x] **Basic UI**: Main dashboard layout, Sidebar, Circular Gauges.
- [x] **UI Polish**: Borderless "seamless" design for cards and sidebar.
- [x] **Data Binding**: C++ backend connected to QML.
- [x] **Prototype**: Running with simulated data.
- [x] **Real Data Integration**: 
  - [x] CPU & RAM (Windows PDH)
  - [x] Disk (QStorageInfo)
  - [ ] Network (Simulated)
- [ ] **Remote Monitoring**: Implementation of SSH/Network data fetching.
- [ ] **Settings**: Configuration for refresh rates and alerts.

## 7. Roadmap
1.  **Real Data Integration**: Implement real network traffic monitoring (Pending).
2.  **Remote Module**: Add server list and SSH connection logic.
3.  **Alert System**: Visual feedback when thresholds are crossed (Red glowing warnings).
