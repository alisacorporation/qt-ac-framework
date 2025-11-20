import QtQuick
import QtQuick.Effects

Item {
    id: root
    property real rxValue: 0 // 0 to 100 (download/receive)
    property real txValue: 0 // 0 to 100 (upload/transmit)
    property string label: "NETWORK"

    width: 200
    height: 200

    // Background Circle
    Rectangle {
        anchors.centerIn: parent
        width: parent.width * 0.9
        height: parent.width * 0.9
        radius: width / 2
        color: "#111111"
        border.color: "#333333"
        border.width: 1
    }

    // RX Progress Arc (Green - Download/Receive)
    Canvas {
        id: rxCanvas
        anchors.fill: parent
        property real angleOffset: -90
        property real arcValue: root.rxValue

        onArcValueChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d");
            var x = width / 2;
            var y = height / 2;
            var radius = width * 0.4;
            var startAngle = (Math.PI / 180) * angleOffset;
            var endAngle = (Math.PI / 180) * (angleOffset + (180 * arcValue / 100));

            ctx.reset();
            ctx.lineCap = "round";
            
            ctx.beginPath();
            ctx.arc(x, y, radius, startAngle, endAngle, false);
            ctx.lineWidth = 3;
            ctx.strokeStyle = "#008B00"; // Dark green for RX
            ctx.stroke();
        }
    }

    // TX Progress Arc (Red - Upload/Transmit)
    Canvas {
        id: txCanvas
        anchors.fill: parent
        property real angleOffset: 90
        property real arcValue: root.txValue

        onArcValueChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d");
            var x = width / 2;
            var y = height / 2;
            var radius = width * 0.4;
            var startAngle = (Math.PI / 180) * angleOffset;
            var endAngle = (Math.PI / 180) * (angleOffset + (180 * arcValue / 100));

            ctx.reset();
            ctx.lineCap = "round";
            
            ctx.beginPath();
            ctx.arc(x, y, radius, startAngle, endAngle, false);
            ctx.lineWidth = 3;
            ctx.strokeStyle = "#8B0000"; // Dark red for TX
            ctx.stroke();
        }
    }

    // Center Text
    Column {
        anchors.centerIn: parent
        spacing: 5
        
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            
            Column {
                Text {
                    text: "RX"
                    color: "#008B00"
                    font.pixelSize: 12
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: Math.round(root.rxValue) + "%"
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
            
            Column {
                Text {
                    text: "TX"
                    color: "#8B0000"
                    font.pixelSize: 12
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: Math.round(root.txValue) + "%"
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
        
        Text {
            text: root.label
            color: "#AAAAAA"
            font.pixelSize: 14
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
    
    // Decorative tech marks
    Repeater {
        model: 8
        Rectangle {
            width: 4
            height: 10
            color: index < 4 ? "#008B00" : "#8B0000"
            x: parent.width/2 - width/2
            y: 5
            transform: Rotation {
                origin.x: width/2
                origin.y: parent.height/2 - 5
                angle: index * 45
            }
        }
    }
}
