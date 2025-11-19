import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import App 1.0

Item {
    id: root

    ServerManager {
        id: serverManager
    }

    property bool showAddDialog: false

    Column {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 20

        // Header
        Row {
            width: parent.width
            spacing: 20

            Text {
                text: "REMOTE SERVER MONITORING"
                color: "white"
                font.pixelSize: 16
                font.weight: Font.Light
                font.family: "Segoe UI"
                font.letterSpacing: 2
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { width: parent.width - 600; height: 1 }

            Button {
                text: "+ ADD SERVER"
                width: 120
                height: 32
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
                    font.pixelSize: 10
                    font.weight: Font.Light
                    font.letterSpacing: 1
                    font.family: "Segoe UI"
                }
                onClicked: showAddDialog = true
            }
        }

        // Server List
        Rectangle {
            width: parent.width
            height: parent.height - 100
            color: "transparent"
            border.color: "#333333"
            border.width: 1
            radius: 0

            ListView {
                id: serverList
                anchors.fill: parent
                clip: true
                model: serverManager.servers
                spacing: 0

                delegate: Item {
                    width: serverList.width
                    height: 120

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
                            anchors.margins: 20
                            spacing: 20

                            // Status indicator
                            Rectangle {
                                width: 8
                                height: 8
                                radius: 4
                                anchors.verticalCenter: parent.verticalCenter
                                color: modelData.connected ? "#00FF00" : "#FF0000"
                            }

                            // Server info
                            Column {
                                width: 200
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 5

                                Text {
                                    text: modelData.name
                                    color: "white"
                                    font.pixelSize: 14
                                    font.weight: Font.DemiBold
                                    font.family: "Segoe UI"
                                }

                                Text {
                                    text: modelData.username + "@" + modelData.host + ":" + modelData.port
                                    color: "#888"
                                    font.pixelSize: 10
                                    font.family: "Consolas"
                                }

                                Text {
                                    text: modelData.connected ? "Connected" : (modelData.lastError || "Disconnected")
                                    color: modelData.connected ? "#00FF00" : "#FF0000"
                                    font.pixelSize: 9
                                    font.family: "Segoe UI"
                                }
                            }

                            // Stats gauges
                            Row {
                                spacing: 30
                                anchors.verticalCenter: parent.verticalCenter

                                // CPU
                                Column {
                                    spacing: 5
                                    Rectangle {
                                        width: 60
                                        height: 60
                                        color: "transparent"
                                        border.color: "#8B0000"
                                        border.width: 2
                                        radius: 30

                                        Text {
                                            anchors.centerIn: parent
                                            text: Math.round(modelData.cpuUsage) + "%"
                                            color: "white"
                                            font.pixelSize: 12
                                            font.bold: true
                                        }
                                    }
                                    Text {
                                        text: "CPU"
                                        color: "#666"
                                        font.pixelSize: 9
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }
                                }

                                // RAM
                                Column {
                                    spacing: 5
                                    Rectangle {
                                        width: 60
                                        height: 60
                                        color: "transparent"
                                        border.color: "#8B0000"
                                        border.width: 2
                                        radius: 30

                                        Text {
                                            anchors.centerIn: parent
                                            text: Math.round(modelData.ramUsage) + "%"
                                            color: "white"
                                            font.pixelSize: 12
                                            font.bold: true
                                        }
                                    }
                                    Text {
                                        text: "RAM"
                                        color: "#666"
                                        font.pixelSize: 9
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }
                                }

                                // Disk
                                Column {
                                    spacing: 5
                                    Rectangle {
                                        width: 60
                                        height: 60
                                        color: "transparent"
                                        border.color: "#8B0000"
                                        border.width: 2
                                        radius: 30

                                        Text {
                                            anchors.centerIn: parent
                                            text: Math.round(modelData.diskUsage) + "%"
                                            color: "white"
                                            font.pixelSize: 12
                                            font.bold: true
                                        }
                                    }
                                    Text {
                                        text: "DISK"
                                        color: "#666"
                                        font.pixelSize: 9
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }
                                }

                                // Network
                                Column {
                                    spacing: 5
                                    anchors.verticalCenter: parent.verticalCenter
                                    Text {
                                        text: "↑ " + modelData.networkUp
                                        color: "#00FF00"
                                        font.pixelSize: 10
                                        font.family: "Consolas"
                                    }
                                    Text {
                                        text: "↓ " + modelData.networkDown
                                        color: "#00FF00"
                                        font.pixelSize: 10
                                        font.family: "Consolas"
                                    }
                                }
                            }

                            Item { width: 1; height: 1 }

                            // Actions
                            Column {
                                spacing: 5
                                anchors.verticalCenter: parent.verticalCenter

                                Button {
                                    text: "REMOVE"
                                    width: 80
                                    height: 26
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
                                    onClicked: serverManager.removeServer(modelData.id)
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
                text: serverManager.servers.length === 0 ? "No servers added. Click '+ ADD SERVER' to get started." : ""
                color: "#666"
                font.pixelSize: 14
                visible: serverManager.servers.length === 0
            }
        }
    }

    // Add Server Dialog
    Rectangle {
        id: addDialog
        anchors.fill: parent
        color: "#000000CC"
        visible: showAddDialog

        MouseArea {
            anchors.fill: parent
            onClicked: showAddDialog = false
        }

        Rectangle {
            anchors.centerIn: parent
            width: 400
            height: 350
            color: "#0F0F0F"
            border.color: "#8B0000"
            border.width: 2

            Column {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 15

                Text {
                    text: "ADD REMOTE SERVER"
                    color: "white"
                    font.pixelSize: 14
                    font.weight: Font.Light
                    font.letterSpacing: 2
                }

                TextField {
                    id: nameField
                    width: parent.width
                    placeholderText: "Server Name"
                    placeholderTextColor: "#666"
                    color: "white"
                    background: Rectangle {
                        color: "#1a1a1a"
                        border.color: "#333"
                        border.width: 1
                    }
                }

                TextField {
                    id: hostField
                    width: parent.width
                    placeholderText: "Host (IP or domain)"
                    placeholderTextColor: "#666"
                    color: "white"
                    background: Rectangle {
                        color: "#1a1a1a"
                        border.color: "#333"
                        border.width: 1
                    }
                }

                TextField {
                    id: portField
                    width: parent.width
                    placeholderText: "Port (default: 22)"
                    placeholderTextColor: "#666"
                    text: "22"
                    color: "white"
                    background: Rectangle {
                        color: "#1a1a1a"
                        border.color: "#333"
                        border.width: 1
                    }
                }

                TextField {
                    id: usernameField
                    width: parent.width
                    placeholderText: "Username"
                    placeholderTextColor: "#666"
                    color: "white"
                    background: Rectangle {
                        color: "#1a1a1a"
                        border.color: "#333"
                        border.width: 1
                    }
                }

                TextField {
                    id: passwordField
                    width: parent.width
                    placeholderText: "Password (or use SSH keys)"
                    placeholderTextColor: "#666"
                    echoMode: TextInput.Password
                    color: "white"
                    background: Rectangle {
                        color: "#1a1a1a"
                        border.color: "#333"
                        border.width: 1
                    }
                }

                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        text: "ADD"
                        width: 100
                        height: 32
                        background: Rectangle {
                            color: parent.hovered ? "#8B0000" : "#660000"
                            border.color: "#FF0000"
                            border.width: 1
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 10
                            font.weight: Font.Light
                        }
                        onClicked: {
                            serverManager.addServer(
                                nameField.text,
                                hostField.text,
                                parseInt(portField.text) || 22,
                                usernameField.text,
                                passwordField.text
                            )
                            showAddDialog = false
                            nameField.text = ""
                            hostField.text = ""
                            portField.text = "22"
                            usernameField.text = ""
                            passwordField.text = ""
                        }
                    }

                    Button {
                        text: "CANCEL"
                        width: 100
                        height: 32
                        background: Rectangle {
                            color: parent.hovered ? "#333" : "#1a1a1a"
                            border.color: "#666"
                            border.width: 1
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 10
                            font.weight: Font.Light
                        }
                        onClicked: showAddDialog = false
                    }
                }
            }
        }
    }
}
