import QtQuick

SettingRow {
    id: sl
    block: true
    property real from: 0
    property real to: 100
    property real stepSize: 1
    property string suffix: ""
    valueText: Math.round(Number(value)) + suffix

    Item {
        width: parent.width; height: 28
        Rectangle {
            id: track
            y: 12; width: parent.width; height: 4; radius: 2; color: "#3a3550"
            Rectangle { width: handle.x + handle.width / 2; height: parent.height; radius: 2; color: "#6a4cff" }
        }
        Rectangle {
            id: handle
            width: 22; height: 22; radius: 11; color: "white"; y: 3
            x: (Number(sl.value) - sl.from) / (sl.to - sl.from) * (track.width - width)
        }
        MouseArea {
            anchors.fill: parent
            function apply(mx) {
                const r = Math.max(0, Math.min(1, (mx - handle.width / 2) / (track.width - handle.width)))
                const v = Math.round((sl.from + r * (sl.to - sl.from)) / sl.stepSize) * sl.stepSize
                if (v !== Number(sl.value)) sl.edit(v)
            }
            onPressed: (m) => apply(m.x)
            onPositionChanged: (m) => apply(m.x)
        }
    }
}
