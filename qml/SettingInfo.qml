import QtQuick

// read-only line
SettingRow {
    id: inf
    property string infoText
    Text {
        text: inf.infoText; color: "#b9b5cc"; font.pixelSize: 14
        width: Math.min(implicitWidth, 170); elide: Text.ElideRight
        horizontalAlignment: Text.AlignRight
    }
}
