import QtQuick
import QtQuick.Shapes

// Mini sparkline chart component
Item {
    id: root
    width: 120
    height: 40
    
    property var dataPoints: []  // Array of values 0-100
    property color lineColor: "#00FF00"
    property int maxPoints: 30
    
    // Add new data point
    function addPoint(value) {
        var newData = dataPoints.slice()
        newData.push(value)
        if (newData.length > maxPoints) {
            newData.shift()
        }
        dataPoints = newData
    }
    
    Canvas {
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            
            if (root.dataPoints.length < 2) return
            
            ctx.strokeStyle = root.lineColor
            ctx.lineWidth = 1.5
            ctx.beginPath()
            
            var xStep = width / (root.maxPoints - 1)
            var yScale = height / 100.0
            
            for (var i = 0; i < root.dataPoints.length; i++) {
                var x = i * xStep
                var y = height - (root.dataPoints[i] * yScale)
                
                if (i === 0) {
                    ctx.moveTo(x, y)
                } else {
                    ctx.lineTo(x, y)
                }
            }
            
            ctx.stroke()
        }
    }
    
    // Redraw when data changes
    onDataPointsChanged: {
        for (var i = 0; i < children.length; i++) {
            if (children[i] instanceof Canvas) {
                children[i].requestPaint()
            }
        }
    }
}
