import QtQuick

Item {
    id: page
    // provided by Main.qml
    property var skins: []
    property int skinIndex: 0
    property bool playing: true
    signal closeRequested()
    signal skinSelected(int index)
    signal muteToggled()

    // ---- add a group = add a file + one line here ----
    readonly property var groups: [
        { title: "General",  subtitle: "Skin, tray",         source: "GeneralPage.qml" },
        { title: "Audio",    subtitle: "Mute, volume",       source: "AudioPage.qml" },
        { title: "Hardware", subtitle: "Device, connection", source: "HardwarePage.qml" }
    ]
    property int group: -1                         // -1 = list of groups

    function openGroup(i) {
        groupLoader.setSource(Qt.resolvedUrl(groups[i].source), { "shell": page })
        group = i
    }
    function back() { if (group >= 0) group = -1; else closeRequested() }

    focus: visible
    onVisibleChanged: if (visible) forceActiveFocus()
    Keys.onEscapePressed: back()

    component Chevron: Canvas {
        property int dir: 1                         // 1 = right, -1 = left
        property color tint: "#b9b5cc"
        width: 20; height: 20
        onTintChanged: requestPaint()
        onPaint: {
            const c = getContext("2d"); c.clearRect(0, 0, width, height)
            c.strokeStyle = tint; c.lineWidth = 2; c.lineCap = "round"; c.lineJoin = "round"
            c.beginPath(); c.moveTo(10 - 3 * dir, 4); c.lineTo(10 + 3 * dir, 10); c.lineTo(10 - 3 * dir, 16); c.stroke()
        }
    }

    Rectangle { anchors.fill: parent; color: "#1f1c2e" }
    MouseArea { anchors.fill: parent }              // swallow clicks meant for the skin below

    // ---- header ----
    Item {
        id: header
        width: parent.width; height: 56
        Item {
            x: 6; width: 48; height: parent.height
            Chevron { anchors.centerIn: parent; dir: -1; tint: "#f1f0f7" }
            MouseArea { anchors.fill: parent; onClicked: page.back() }
        }
        Text { x: 56; anchors.verticalCenter: parent.verticalCenter; color: "#f1f0f7"; font.pixelSize: 20
               text: page.group < 0 ? "Settings" : page.groups[page.group].title }
        Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                    height: 1; color: "#2f2b44" }
    }

    // ---- content: group list / group page slide horizontally ----
    Item {
        anchors { left: parent.left; right: parent.right; top: header.bottom; bottom: parent.bottom }
        clip: true

        Column {
            id: list
            width: parent.width
            x: page.group < 0 ? 0 : -width * 0.3
            opacity: page.group < 0 ? 1 : 0
            Behavior on x { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
            Behavior on opacity { NumberAnimation { duration: 200 } }
            Repeater {
                model: page.groups
                delegate: Item {
                    width: list.width; height: 68
                    Column {
                        x: 16; anchors.verticalCenter: parent.verticalCenter; spacing: 3
                        Text { text: modelData.title; color: "#f1f0f7"; font.pixelSize: 17 }
                        Text { text: modelData.subtitle; color: "#8c88a0"; font.pixelSize: 12 }
                    }
                    Chevron { anchors { right: parent.right; rightMargin: 14; verticalCenter: parent.verticalCenter } }
                    Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom; leftMargin: 16 }
                                height: 1; color: "#2f2b44" }
                    MouseArea { anchors.fill: parent; enabled: page.group < 0; onClicked: page.openGroup(index) }
                }
            }
        }

        Loader {
            id: groupLoader
            width: parent.width; height: parent.height
            x: page.group >= 0 ? 0 : width
            Behavior on x { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
        }
    }
}
