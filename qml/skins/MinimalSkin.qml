import QtQuick

// Tiny second skin to demonstrate switching.
SkinBase {
    id: skin
    Rectangle { anchors.fill: parent; color: "#f4f1ea" }
    Column {
        anchors.centerIn: parent; spacing: 18
        Text { anchors.horizontalCenter: parent.horizontalCenter
               text: skin.frequency.toFixed(1) + " MHz"; font.pixelSize: 56; color: "#222" }
        Slider2 { }
        Row {
            anchors.horizontalCenter: parent.horizontalCenter; spacing: 12
            Repeater {
                model: [ "−0.1", "Connect", "+0.1", "Skin" ]
                delegate: Rectangle {
                    width: 72; height: 40; radius: 6; color: "#222"
                    Text { anchors.centerIn: parent; text: modelData; color: "white" }
                    MouseArea { anchors.fill: parent; onClicked: {
                        if (index === 0) skin.stepRequested(-1)
                        else if (index === 1) skin.connectRequested()
                        else if (index === 2) skin.stepRequested(1)
                        else skin.nextSkinRequested() } }
                }
            }
        }
        Text { anchors.horizontalCenter: parent.horizontalCenter
               text: skin.connected ? "connected" : (skin.status || "disconnected"); color: "#777" }
    }
    component Slider2 : Rectangle {
        width: 300; height: 6; radius: 3; color: "#ccc"
        anchors.horizontalCenter: parent.horizontalCenter
        Rectangle { width: 18; height: 18; radius: 9; color: "#c0392b"; anchors.verticalCenter: parent.verticalCenter
            x: (skin.frequency - skin.minFreq) / (skin.maxFreq - skin.minFreq) * (parent.width - width) }
        MouseArea { anchors.fill: parent; anchors.margins: -12
            onPositionChanged: (m) => skin.tuneRequested(skin.minFreq + Math.max(0, Math.min(1, m.x / width)) * (skin.maxFreq - skin.minFreq))
            onPressed: (m) => positionChanged(m) }
    }
}
