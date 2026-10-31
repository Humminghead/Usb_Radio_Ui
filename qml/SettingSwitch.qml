import QtQuick

SettingRow {
    id: sw
    Rectangle {
        width: 46; height: 26; radius: 13
        color: sw.value ? "#6a4cff" : "#3a3550"
        Behavior on color { ColorAnimation { duration: 120 } }
        Rectangle {
            width: 20; height: 20; radius: 10; color: "white"; y: 3
            x: sw.value ? 23 : 3
            Behavior on x { NumberAnimation { duration: 120 } }
        }
        MouseArea { anchors.fill: parent; anchors.margins: -8; onClicked: sw.edit(!sw.value) }
    }
}
