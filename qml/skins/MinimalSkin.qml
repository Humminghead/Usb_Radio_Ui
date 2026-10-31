import QtQuick

SkinBase {
    id: skin

    property real displayFrequency: frequency
    property bool dragging: false

    Behavior on displayFrequency {
        enabled: !skin.dragging
        NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
    }

    onFrequencyChanged: {
        if (!skin.dragging)
            skin.displayFrequency = skin.frequency
    }

    Rectangle { anchors.fill: parent; color: "#f4f1ea" }

    Column {
        anchors.centerIn: parent; spacing: 18

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: skin.displayFrequency.toFixed(1) + " MHz"
            font.pixelSize: 56; color: "#222"
        }

        // slider (inlined: inline components cannot see the file's ids)
        Rectangle {
            id: track
            anchors.horizontalCenter: parent.horizontalCenter
            width: 300; height: 6; radius: 3; color: "#ccc"

            Rectangle {
                id: handle
                width: 18; height: 18; radius: 9; color: "#c0392b"
                anchors.verticalCenter: parent.verticalCenter
                x: (skin.displayFrequency - skin.minFreq) / (skin.maxFreq - skin.minFreq)
                   * (track.width - width)
            }

            MouseArea {
                anchors.fill: parent; anchors.margins: -12
                // map pointer to frequency in the track's coordinates (handle centred on pointer)
                function apply(m) {
                    const px = mapToItem(track, m.x, m.y).x
                    const r = Math.max(0, Math.min(1, (px - handle.width / 2) / (track.width - handle.width)))
                    const f = Math.round((skin.minFreq + r * (skin.maxFreq - skin.minFreq)) * 10) / 10
                    const c = Math.max(skin.minFreq, Math.min(skin.maxFreq, f))
                    skin.displayFrequency = c
                    if (Math.abs(c - skin.frequency) > 0.01)
                        skin.tuneRequested(c)
                }
                onPressed: (m) => { skin.dragging = true; apply(m) }
                onPositionChanged: (m) => apply(m)
                onReleased: { skin.dragging = false; skin.displayFrequency = skin.frequency }
                onCanceled: { skin.dragging = false; skin.displayFrequency = skin.frequency }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter; spacing: 12
            Repeater {
                model: ["−0.1", "Connect", "+0.1", "Settings"]
                delegate: Rectangle {
                    width: 72; height: 40; radius: 6; color: "#222"
                    Text { anchors.centerIn: parent; text: modelData; color: "white" }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (index === 0) skin.stepRequested(-1)
                            else if (index === 1) skin.connectRequested()
                            else if (index === 2) skin.stepRequested(1)
                            else skin.settingsRequested()
                        }
                    }
                }
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: skin.connected ? "connected" : (skin.status || "disconnected")
            color: "#777"
        }
    }
}
