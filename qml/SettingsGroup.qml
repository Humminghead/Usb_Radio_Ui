import QtQuick

// Scrollable container for one settings group. Put Setting* rows inside.
Item {
    id: group
    property var shell                          // the SettingsPage (skins, skinIndex, playing, signals)
    default property alias rows: col.data
    Flickable {
        anchors.fill: parent
        clip: true
        contentHeight: col.height + 24
        boundsBehavior: Flickable.StopAtBounds
        Column { id: col; width: parent.width }
    }
}
