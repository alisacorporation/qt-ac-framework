import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import App 1.0

Window {
    id: mainWindow
    width: 1280
    height: 720
    visible: true
    title: qsTr("Umbrella Corp. System Monitor")
    color: "#050505"

    SystemMonitor {
        id: sysMon
    }

    Row {
        anchors.fill: parent

        // Sidebar
        Sidebar {
            height: parent.height
            onItemClicked: (page) => {
                console.log("Navigating to: " + page)
                // Simple navigation logic
                if (page === "DASHBOARD") viewStack.currentIndex = 0;
                else if (page === "LOCAL SYSTEM") viewStack.currentIndex = 0;
                else if (page === "REMOTE SERVER") viewStack.currentIndex = 1; // Placeholder
                else if (page === "SETTINGS") viewStack.currentIndex = 2; // Placeholder
            }
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

                    Text {
                        text: "SYSTEM OVERVIEW // " + new Date().toLocaleTimeString()
                        color: "white"
                        font.pixelSize: 16
                        font.weight: Font.Light
                        font.family: "Segoe UI"
                        font.letterSpacing: 2
                        x: 40; y: 40
                    }

                    // Stats Grid
                    Grid {
                        x: 40; y: 100
                        columns: 3
                        spacing: 40

                        StatCircle {
                            label: "CPU LOAD"
                            value: sysMon.cpuUsage
                        }

                        StatCircle {
                            label: "MEMORY"
                            value: sysMon.ramUsage
                        }

                        StatCircle {
                            label: "DISK USAGE"
                            value: sysMon.diskUsage
                        }
                    }
                    
                    // Network Stats
                    Rectangle {
                        x: 40
                        y: 350
                        width: 680
                        height: 150
                        color: "#0F0F0F"
                        border.color: "#333333"
                        border.width: 1
                        radius: 0
                        
                        Text {
                            text: "NETWORK I/O"
                            color: "#8B0000"
                            font.bold: true
                            x: 20; y: 20
                        }
                        
                        Row {
                            anchors.centerIn: parent
                            spacing: 100
                            
                            Column {
                                Text { text: "UPLOAD"; color: "#888"; font.pixelSize: 12 }
                                Text { 
                                    text: sysMon.networkUp 
                                    color: "white"
                                    font.pixelSize: 28
                                    font.bold: true
                                }
                            }
                            
                            Column {
                                Text { text: "DOWNLOAD"; color: "#888"; font.pixelSize: 12 }
                                Text { 
                                    text: sysMon.networkDown 
                                    color: "white"
                                    font.pixelSize: 28
                                    font.bold: true
                                }
                            }
                        }
                    }
                }

                // View 1: Remote Server (Placeholder)
                Item {
                    Text {
                        anchors.centerIn: parent
                        text: "REMOTE SERVER CONNECTION REQUIRED"
                        color: "red"
                        font.pixelSize: 24
                    }
                }

                // View 2: Settings (Placeholder)
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
