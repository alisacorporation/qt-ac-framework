import QtQuick
import QtQuick.Effects

Item {
    id: root
    property real value: 0 // 0 to 100
    property string label: "CPU"
    property color primaryColor: "#8B0000"
    property color glowColor: "#FF3333"

    width: 200
    height: 200

    // Background Circle
    Rectangle {
        anchors.centerIn: parent
        width: parent.width * 0.9
        height: parent.width * 0.9
        radius: width / 2
        color: "#111111" // Filled dark track
        border.color: "#333333"
        border.width: 1
    }

    // Progress Arc
    Canvas {
        id: canvas
        anchors.fill: parent
        property real angleOffset: -90
        property real arcValue: root.value

        onArcValueChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d");
            var x = width / 2;
            var y = height / 2;
            var radius = width * 0.4;
            var startAngle = (Math.PI / 180) * angleOffset;
            var endAngle = (Math.PI / 180) * (angleOffset + (360 * arcValue / 100));

            ctx.reset();

            // Glow effect simulation using multiple strokes with decreasing opacity
            ctx.lineCap = "round";
            
            // Main Path
            ctx.beginPath();
            ctx.arc(x, y, radius, startAngle, endAngle, false);
            ctx.lineWidth = 3;
            ctx.strokeStyle = root.primaryColor;
            ctx.stroke();
        }
    }

    // Center Text
    Column {
        anchors.centerIn: parent
        spacing: -2
        Text {
            text: Math.round(root.value) + "%"
            color: "white"
            font.pixelSize: Math.max(12, root.width * 0.16)
            font.bold: true
            font.family: "Arial" // Or a tech font if available
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: root.label
            color: "#AAAAAA"
            font.pixelSize: Math.max(8, root.width * 0.07)
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
    
    // Decorative tech marks
    Repeater {
        model: 8
        Rectangle {
            width: Math.max(2, root.width * 0.02)
            height: Math.max(6, root.width * 0.05)
            color: root.primaryColor
            x: parent.width/2 - width/2
            y: Math.max(3, root.width * 0.025)
            transform: Rotation {
                origin.x: width/2
                origin.y: parent.height/2 - y
                angle: index * 45
            }
        }
    }
}
