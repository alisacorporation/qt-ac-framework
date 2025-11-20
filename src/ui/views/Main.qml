import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import App 1.0

Window {
    id: mainWindow
    width: 1280
    height: 720
    visible: true
    title: qsTr("Alisa Corporation :: System Monitor")
    color: "#050505"

    SystemMonitor {
        id: sysMon
    }

    Row {
        anchors.fill: parent

        // Sidebar
        Sidebar {
            id: sidebar
            height: parent.height
            onItemClicked: (page) => {
                console.log("Navigating to: " + page)
                sidebar.activePage = page
                // Simple navigation logic
                if (page === "DASHBOARD") viewStack.currentIndex = 0;
                else if (page === "LOCAL SYSTEM") viewStack.currentIndex = 0;
                else if (page === "DISK CLEANUP") viewStack.currentIndex = 1;
                else if (page === "REMOTE SERVER") viewStack.currentIndex = 2;
                else if (page === "SETTINGS") viewStack.currentIndex = 3;
            }
        }

        // Border separator
        Rectangle {
            width: 1
            height: parent.height
            color: "#333333"
        }

        // Main Content
        Rectangle {
            width: parent.width - 250
            height: parent.height
            color: "transparent"
            clip: true

            StackLayout {
                id: viewStack
                anchors.fill: parent
                currentIndex: 0
                
                // View 0: Dashboard / Local System
                Item {
                    id: dashboard
                    property string currentTime: new Date().toLocaleTimeString()
                    
                    // Sparkline data storage
                    property var cpuHistory: []
                    property var ramHistory: []
                    property var diskHistory: []
                    property var networkHistory: []
                    
                    Timer {
                        interval: 1000
                        running: true
                        repeat: true
                        onTriggered: {
                            parent.currentTime = Qt.formatTime(new Date(), "HH:mm:ss")
                            
                            // Update sparklines every second
                            cpuSparkline.addPoint(sysMon.cpuUsage)
                            ramSparkline.addPoint(sysMon.ramUsage)
                            diskSparkline.addPoint(sysMon.diskUsage)
                            networkSparkline.addPoint((sysMon.networkTxPercent + sysMon.networkRxPercent) / 2)
                        }
                    }
                    
                    // Background Grid Decoration
                    Grid {
                        rows: 20; columns: 20
                        spacing: 40
                        anchors.fill: parent
                        opacity: 0.1
                        Repeater {
                            model: 400
                            Rectangle { width: 2; height: 2; color: "#8B0000" }
                        }
                    }

                    // Quick Stats Bar
                    Rectangle {
                        id: quickStatsBar
                        width: parent.width
                        height: 50
                        color: "#0a0a0a"
                        border.color: "#222"
                        border.width: 1
                        
                        Row {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 40
                            
                            // Health Score
                            Row {
                                spacing: 10
                                anchors.verticalCenter: parent.verticalCenter
                                
                                Rectangle {
                                    width: 30
                                    height: 30
                                    radius: 15
                                    anchors.verticalCenter: parent.verticalCenter
                                    color: sysMon.healthScore >= 90 ? "#00FF00" : 
                                           sysMon.healthScore >= 70 ? "#FFA500" : "#FF0000"
                                    
                                    Text {
                                        anchors.centerIn: parent
                                        text: sysMon.healthScore
                                        color: "black"
                                        font.bold: true
                                        font.pixelSize: 12
                                    }
                                    
                                    // Pulse animation when critical
                                    SequentialAnimation on opacity {
                                        running: sysMon.healthScore < 70
                                        loops: Animation.Infinite
                                        NumberAnimation { to: 0.3; duration: 500 }
                                        NumberAnimation { to: 1.0; duration: 500 }
                                    }
                                }
                                
                                Column {
                                    anchors.verticalCenter: parent.verticalCenter
                                    Text {
                                        text: "HEALTH"
                                        color: "#666"
                                        font.pixelSize: 9
                                        font.letterSpacing: 1
                                    }
                                    Text {
                                        text: sysMon.healthScore >= 90 ? "OPTIMAL" : 
                                              sysMon.healthScore >= 70 ? "MODERATE" : "CRITICAL"
                                        color: sysMon.healthScore >= 90 ? "#00FF00" : 
                                               sysMon.healthScore >= 70 ? "#FFA500" : "#FF0000"
                                        font.pixelSize: 11
                                        font.bold: true
                                    }
                                }
                            }
                            
                            Rectangle { width: 1; height: 30; color: "#333"; anchors.verticalCenter: parent.verticalCenter }
                            
                            // Uptime
                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    text: "UPTIME"
                                    color: "#666"
                                    font.pixelSize: 9
                                    font.letterSpacing: 1
                                }
                                Text {
                                    text: sysMon.uptime
                                    color: "white"
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                            }
                            
                            Rectangle { width: 1; height: 30; color: "#333"; anchors.verticalCenter: parent.verticalCenter }
                            
                            // Processes
                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    text: "PROCESSES"
                                    color: "#666"
                                    font.pixelSize: 9
                                    font.letterSpacing: 1
                                }
                                Text {
                                    text: sysMon.processCount + " running"
                                    color: "white"
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                            }
                            
                            Rectangle { width: 1; height: 30; color: "#333"; anchors.verticalCenter: parent.verticalCenter }
                            
                            // Time
                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    text: "TIME"
                                    color: "#666"
                                    font.pixelSize: 9
                                    font.letterSpacing: 1
                                }
                                Text {
                                    text: dashboard.currentTime
                                    color: "white"
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                            }
                        }
                    }

                    Text {
                        text: "SYSTEM OVERVIEW"
                        color: "white"
                        font.pixelSize: 16
                        font.weight: Font.Light
                        font.family: "Segoe UI"
                        font.letterSpacing: 2
                        x: 40
                        y: quickStatsBar.height + 20
                    }

                    // Stats Grid with Sparklines
                    Grid {
                        x: 40
                        y: quickStatsBar.height + 50
                        columns: 4
                        spacing: 40

                        // CPU Column
                        Column {
                            spacing: 10
                            StatCircle {
                                label: "CPU LOAD"
                                value: sysMon.cpuUsage
                            }
                            SparkLine {
                                id: cpuSparkline
                                width: 150
                                height: 40
                                lineColor: "#00FF00"
                            }
                            Text {
                                text: "30s history"
                                color: "#666"
                                font.pixelSize: 9
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        // RAM Column
                        Column {
                            spacing: 10
                            StatCircle {
                                label: "MEMORY"
                                value: sysMon.ramUsage
                            }
                            SparkLine {
                                id: ramSparkline
                                width: 150
                                height: 40
                                lineColor: "#00BFFF"
                            }
                            Text {
                                text: "30s history"
                                color: "#666"
                                font.pixelSize: 9
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        // Disk Column
                        Column {
                            spacing: 10
                            StatCircle {
                                label: "DISK USAGE"
                                value: sysMon.diskUsage
                            }
                            SparkLine {
                                id: diskSparkline
                                width: 150
                                height: 40
                                lineColor: "#FFA500"
                            }
                            Text {
                                text: "30s history"
                                color: "#666"
                                font.pixelSize: 9
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        // Network Column
                        Column {
                            spacing: 10
                            NetworkCircle {
                                label: "NETWORK"
                                rxValue: sysMon.networkRxPercent
                                txValue: sysMon.networkTxPercent
                            }
                            SparkLine {
                                id: networkSparkline
                                width: 150
                                height: 40
                                lineColor: "#FF00FF"
                            }
                            Text {
                                text: "30s history"
                                color: "#666"
                                font.pixelSize: 9
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                    
                    // Network I/O - Compact
                    Rectangle {
                        x: 40
                        y: quickStatsBar.height + 380
                        width: 300
                        height: 120
                        color: "#0F0F0F"
                        border.color: "#333333"
                        border.width: 1
                        
                        Column {
                            anchors.fill: parent
                            anchors.margins: 15
                            spacing: 8
                            
                            Text {
                                text: "NETWORK I/O"
                                color: "#8B0000"
                                font.bold: true
                                font.pixelSize: 11
                                font.letterSpacing: 1
                            }
                            
                            Rectangle { width: parent.width; height: 1; color: "#222" }
                            
                            // Upload
                            Row {
                                spacing: 6
                                width: parent.width
                                
                                Text {
                                    text: "↑"
                                    color: "#FF0000"
                                    font.pixelSize: 12
                                    font.bold: true
                                    width: 16
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                
                                Column {
                                    spacing: 0
                                    Text {
                                        text: "TX"
                                        color: "#666"
                                        font.pixelSize: 9
                                    }
                                    Text {
                                        text: sysMon.networkUp
                                        color: "#FF0000"
                                        font.pixelSize: 12
                                        font.weight: Font.Light
                                    }
                                }
                            }
                            
                            Item { height: 2 }
                            
                            // Download
                            Row {
                                spacing: 6
                                width: parent.width
                                
                                Text {
                                    text: "↓"
                                    color: "#00FF00"
                                    font.pixelSize: 12
                                    font.bold: true
                                    width: 16
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                
                                Column {
                                    spacing: 0
                                    Text {
                                        text: "RX"
                                        color: "#666"
                                        font.pixelSize: 9
                                    }
                                    Text {
                                        text: sysMon.networkDown
                                        color: "#00FF00"
                                        font.pixelSize: 12
                                        font.weight: Font.Light
                                    }
                                }
                            }
                        }
                    }
                    
                    // System Info Cards
                    Grid {
                        x: 40
                        y: quickStatsBar.height + 520
                        columns: 3
                        spacing: 20
                        
                        // Hardware Card
                        Rectangle {
                            width: 300
                            height: 120
                            color: "#0F0F0F"
                            border.color: "#333333"
                            border.width: 1
                            
                            Column {
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 8
                                
                                Text {
                                    text: "HARDWARE"
                                    color: "#8B0000"
                                    font.bold: true
                                    font.pixelSize: 11
                                    font.letterSpacing: 1
                                }
                                
                                Rectangle { width: parent.width; height: 1; color: "#222" }
                                
                                Row {
                                    spacing: 10
                                    Text { text: "CPU:"; color: "#666"; font.pixelSize: 10; width: 60 }
                                    Text { 
                                        text: sysMon.cpuModel
                                        color: "white"
                                        font.pixelSize: 10
                                        width: 210
                                        elide: Text.ElideRight
                                    }
                                }
                                
                                Row {
                                    spacing: 10
                                    Text { text: "Cores:"; color: "#666"; font.pixelSize: 10; width: 60 }
                                    Text { text: sysMon.cpuCores; color: "white"; font.pixelSize: 10 }
                                }
                                
                                Row {
                                    spacing: 10
                                    Text { text: "RAM:"; color: "#666"; font.pixelSize: 10; width: 60 }
                                    Text { text: sysMon.totalRam; color: "white"; font.pixelSize: 10 }
                                }
                            }
                        }
                        
                        // OS Card
                        Rectangle {
                            width: 300
                            height: 120
                            color: "#0F0F0F"
                            border.color: "#333333"
                            border.width: 1
                            
                            Column {
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 8
                                
                                Text {
                                    text: "OPERATING SYSTEM"
                                    color: "#8B0000"
                                    font.bold: true
                                    font.pixelSize: 11
                                    font.letterSpacing: 1
                                }
                                
                                Rectangle { width: parent.width; height: 1; color: "#222" }
                                
                                Row {
                                    spacing: 10
                                    Text { text: "OS:"; color: "#666"; font.pixelSize: 10; width: 60 }
                                    Text { 
                                        text: sysMon.osInfo
                                        color: "white"
                                        font.pixelSize: 10
                                        width: 210
                                        elide: Text.ElideRight
                                    }
                                }
                                
                                Row {
                                    spacing: 10
                                    Text { text: "Kernel:"; color: "#666"; font.pixelSize: 10; width: 60 }
                                    Text { text: sysMon.kernelVersion; color: "white"; font.pixelSize: 10 }
                                }
                                
                                Row {
                                    spacing: 10
                                    Text { text: "Hostname:"; color: "#666"; font.pixelSize: 10; width: 60 }
                                    Text { text: sysMon.hostname; color: "white"; font.pixelSize: 10 }
                                }
                            }
                        }
                        
                        // Storage Card
                        Rectangle {
                            width: 300
                            height: 120
                            color: "#0F0F0F"
                            border.color: "#333333"
                            border.width: 1
                            
                            Column {
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 8
                                
                                Text {
                                    text: "STORAGE"
                                    color: "#8B0000"
                                    font.bold: true
                                    font.pixelSize: 11
                                    font.letterSpacing: 1
                                }
                                
                                Rectangle { width: parent.width; height: 1; color: "#222" }
                                
                                Row {
                                    spacing: 10
                                    Text { text: "Usage:"; color: "#666"; font.pixelSize: 10; width: 60 }
                                    Text { 
                                        text: sysMon.diskUsage.toFixed(1) + "%"
                                        color: sysMon.diskUsage > 85 ? "#FF0000" : "white"
                                        font.pixelSize: 10
                                    }
                                }
                                
                                Row {
                                    spacing: 10
                                    Text { text: "Status:"; color: "#666"; font.pixelSize: 10; width: 60 }
                                    Text { 
                                        text: sysMon.diskUsage > 85 ? "CRITICAL" : "HEALTHY"
                                        color: sysMon.diskUsage > 85 ? "#FF0000" : "#00FF00"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                }
                                
                                Text {
                                    text: "Root filesystem (/)"
                                    color: "#444"
                                    font.pixelSize: 9
                                    font.italic: true
                                }
                            }
                        }
                    }
                }

                // View 1: Disk Cleanup
                DiskCleanup {
                }

                // View 2: Remote Server
                RemoteMonitor {
                }

                // View 3: Settings (Placeholder)
                 Item {
                    Text {
                        anchors.centerIn: parent
                        text: "SYSTEM CONFIGURATION LOCKED"
                        color: "red"
                        font.pixelSize: 24
                    }
                }
            }
        }
    }
}
