import QtQuick

SettingRow {
    id: btn
    property string buttonText
    signal clicked()
    Rectangle {
        width: label.width + 28; height: 32; radius: 16; color: "#6a4cff"
        Text { id: label; anchors.centerIn: parent; text: btn.buttonText
               color: "white"; font.pixelSize: 13; font.bold: true }
        MouseArea { anchors.fill: parent; onClicked: btn.clicked() }
    }
}
