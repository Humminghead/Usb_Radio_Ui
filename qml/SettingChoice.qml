import QtQuick

// value = index of the selected option
SettingRow {
    id: ch
    block: true
    property var options: []
    Flow {
        width: parent.width; spacing: 8
        Repeater {
            model: ch.options
            delegate: Rectangle {
                height: 32; width: label.width + 26; radius: 16
                color: index === ch.value ? "#6a4cff" : "#2c2840"
                Text { id: label; anchors.centerIn: parent; text: modelData; font.pixelSize: 13
                       color: index === ch.value ? "white" : "#b9b5cc" }
                MouseArea { anchors.fill: parent; onClicked: ch.edit(index) }
            }
        }
    }
}
