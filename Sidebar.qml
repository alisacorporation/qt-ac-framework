import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 250
    color: "#0F0F0F"
    
    property string activePage: "DASHBOARD"

    signal itemClicked(string page)

    Column {
        anchors.fill: parent
        spacing: 0 // No spacing for border-collapse feel

        // Header / Logo Area
        Rectangle {
            width: parent.width
            height: 80
            color: "transparent"
            
            Image {
                // Placeholder for logo if we had one, using text for now
                anchors.centerIn: parent
                source: "" // TODO: Add assets
            }
            
            Text {
                anchors.centerIn: parent
                text: "ALISA CORPORATION"
                color: "#FFFFFF"
                font.weight: Font.Light
                font.family: "Segoe UI"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                font.letterSpacing: 4
            }
        }
        
        // Border below logo
        Rectangle {
            width: parent.width
            height: 1
            color: "#333333"
        }

        Rectangle { height: 20; width: 1; color: "transparent" } // Spacer

        // Nav Items
        Repeater {
            model: ["DASHBOARD", "LOCAL SYSTEM", "DISK CLEANUP", "REMOTE SERVER", "SETTINGS"]
            
            delegate: Rectangle {
                width: parent.width
                height: 42
                color: modelData === root.activePage ? "#660000" : 
                       (mouseArea.containsMouse ? "#8B0000" : "transparent")
                
                // Active indicator bar
                Rectangle {
                    width: 3
                    height: parent.height
                    color: "#FF0000"
                    visible: modelData === root.activePage
                }
                border.width: 0
                
                // Left indicator for active/hover state
                Rectangle {
                    width: 4
                    height: parent.height
                    anchors.left: parent.left
                    color: mouseArea.containsMouse ? "#FF0000" : "transparent"
                }
                
                Text {
                    text: modelData
                    color: mouseArea.containsMouse ? "white" : "#888888"
                    anchors.centerIn: parent
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    font.family: "Segoe UI"
                }
                
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.itemClicked(modelData)
                }
            }
        }
    }
    
    // Bottom Status
    Rectangle {
        width: parent.width
        height: 40
        anchors.bottom: parent.bottom
        color: "#1a1a1a"
        
        // Top Border Line
        Rectangle {
            width: parent.width
            height: 1
            color: "#333333"
            anchors.top: parent.top
        }
        
        Text {
            anchors.centerIn: parent
            text: "SECURE CONNECTION ESTABLISHED"
            color: "#00FF00"
            font.pixelSize: 10
        }
    }
}
