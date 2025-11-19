import QtQuick
import QtQuick.Controls
import App 1.0

Item {
    id: root

    DiskAnalyzer {
        id: diskAnalyzer
        onScanComplete: (totalSavings) => {
            statusText.text = "Found " + diskAnalyzer.cleanupTargets.length + " items. Potential savings: " + diskAnalyzer.formatSize(totalSavings)
        }
    }

    property string sortBy: "size" // size, date, type
    property bool sortAscending: false
    property var sortedTargets: {
        var items = diskAnalyzer.cleanupTargets.slice()
        if (sortBy === "size") {
            items.sort((a, b) => sortAscending ? a.size - b.size : b.size - a.size)
        } else if (sortBy === "date") {
            items.sort((a, b) => {
                var dateA = new Date(a.lastModified)
                var dateB = new Date(b.lastModified)
                return sortAscending ? dateA - dateB : dateB - dateA
            })
        } else if (sortBy === "type") {
            items.sort((a, b) => sortAscending ? a.type.localeCompare(b.type) : b.type.localeCompare(a.type))
        }
        return items
    }

    Column {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 20

        // Header
        Row {
            width: parent.width
            spacing: 20

            Text {
                text: "DISK CLEANUP ANALYZER"
                color: "white"
                font.pixelSize: 16
                font.weight: Font.Light
                font.family: "Segoe UI"
                font.letterSpacing: 2
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { 
                width: parent.width - 600
                height: 1
            }

            // Sort controls
            Row {
                spacing: 10
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    text: "SORT BY:"
                    color: "#666"
                    font.pixelSize: 10
                    font.weight: Font.Light
                    font.family: "Segoe UI"
                    anchors.verticalCenter: parent.verticalCenter
                }

                Repeater {
                    model: [
                        {label: "SIZE", value: "size"},
                        {label: "DATE", value: "date"},
                        {label: "TYPE", value: "type"}
                    ]

                    Rectangle {
                        width: 50
                        height: 24
                        color: sortBy === modelData.value ? "#8B0000" : "transparent"
                        border.color: sortBy === modelData.value ? "#FF0000" : "#444"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: modelData.label
                            color: sortBy === modelData.value ? "white" : "#888"
                            font.pixelSize: 9
                            font.weight: Font.Light
                            font.family: "Segoe UI"
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: sortBy = modelData.value
                        }
                    }
                }

                // Ascending/Descending toggle
                Rectangle {
                    width: 24
                    height: 24
                    color: "transparent"
                    border.color: "#444"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: sortAscending ? "↑" : "↓"
                        color: "#888"
                        font.pixelSize: 14
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: sortAscending = !sortAscending
                    }
                }
            }
        }

        // Control Panel
        Rectangle {
            width: parent.width
            height: 80
            color: "#0F0F0F"
            border.color: "#333333"
            border.width: 1
            radius: 0

            Row {
                anchors.centerIn: parent
                spacing: 20

                Button {
                    text: diskAnalyzer.isScanning ? "Scanning..." : "Start Scan"
                    enabled: !diskAnalyzer.isScanning
                    background: Rectangle {
                        color: parent.hovered ? "#8B0000" : "#660000"
                        border.color: "#FF0000"
                        border.width: 1
                        radius: 0
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.family: "Segoe UI"
                        font.weight: Font.Light
                    }
                    onClicked: diskAnalyzer.startScan()
                }

                Text {
                    id: statusText
                    text: diskAnalyzer.scanProgress
                    color: "#888"
                    font.pixelSize: 12
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Results List
        Rectangle {
            width: parent.width
            height: parent.height - 180
            color: "transparent"
            border.color: "#333333"
            border.width: 1
            radius: 0

            ListView {
                id: listView
                anchors.fill: parent
                clip: true
                model: sortedTargets

                delegate: Item {
                    width: listView.width
                    height: 48

                    Rectangle {
                        anchors.fill: parent
                        color: mouseArea.containsMouse ? "#1a1a1a" : "transparent"
                        
                        // Bottom border
                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 1
                            color: "#222222"
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 20
                            anchors.rightMargin: 20
                            spacing: 15

                            // Type indicator
                            Rectangle {
                                width: 3
                                height: 32
                                anchors.verticalCenter: parent.verticalCenter
                                color: modelData.isSafe ? "#8B0000" : "#FF0000"
                            }

                            // Content
                            Column {
                                width: parent.width - 220
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 3

                                Row {
                                    spacing: 8
                                    Text {
                                        text: modelData.type
                                        color: "#8B0000"
                                        font.pixelSize: 10
                                        font.weight: Font.DemiBold
                                        font.family: "Segoe UI"
                                    }
                                    Text {
                                        text: "• " + modelData.lastModified
                                        color: "#555"
                                        font.pixelSize: 9
                                        font.family: "Segoe UI"
                                    }
                                    Text {
                                        text: "• " + modelData.reason
                                        color: "#555"
                                        font.pixelSize: 9
                                        font.family: "Segoe UI"
                                        font.italic: true
                                    }
                                }

                                Text {
                                    text: modelData.path
                                    color: "#999"
                                    font.pixelSize: 9
                                    font.family: "Consolas"
                                    elide: Text.ElideMiddle
                                    width: parent.width
                                }
                            }

                            // Size and action
                            Row {
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 12

                                Text {
                                    text: modelData.sizeFormatted
                                    color: "#00FF00"
                                    font.pixelSize: 11
                                    font.weight: Font.Light
                                    font.family: "Consolas"
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Button {
                                    text: "DELETE"
                                    width: 70
                                    height: 26
                                    anchors.verticalCenter: parent.verticalCenter
                                    background: Rectangle {
                                        color: parent.hovered ? "#CC0000" : "#660000"
                                        border.color: parent.hovered ? "#FF0000" : "#8B0000"
                                        border.width: 1
                                        radius: 0
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize: 9
                                        font.weight: Font.Light
                                        font.letterSpacing: 1
                                        font.family: "Segoe UI"
                                    }
                                    onClicked: {
                                        // Find original index in unsorted array
                                        for (var i = 0; i < diskAnalyzer.cleanupTargets.length; i++) {
                                            if (diskAnalyzer.cleanupTargets[i].path === modelData.path) {
                                                diskAnalyzer.deleteTarget(i)
                                                break
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.NoButton
                        }
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: diskAnalyzer.cleanupTargets.length === 0 ? "Click 'Start Scan' to analyze disk usage" : ""
                color: "#666"
                font.pixelSize: 14
                visible: diskAnalyzer.cleanupTargets.length === 0
            }
        }
    }
}
