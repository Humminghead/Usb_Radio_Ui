import QtQuick

// Base for every setting. Set `key` (+ `defaultValue`) to persist automatically,
// or leave `key` empty and bind `value` yourself (then listen to userChanged).
Item {
    id: row
    property string title
    property string description
    property string key
    property var defaultValue
    property var value
    property bool block: false          // true: control goes under the text, full width
    property string valueText: ""       // optional text at the right of the title (block rows)
    default property alias control: holder.data
    signal userChanged(var v)

    function edit(v) {
        if (key !== "") { value = v; settings.setValue(key, v) }
        userChanged(v)
    }
    function reload() { if (key !== "") value = settings.value(key, defaultValue) }
    Component.onCompleted: reload()
    Connections {
        target: settings
        function onValueChanged(k) { if (k === "" || k === row.key) row.reload() }
    }

    width: parent ? parent.width : 0
    height: block ? textCol.height + holder.height + 34 : Math.max(60, textCol.height + 24)
    opacity: enabled ? 1 : 0.4

    Column {
        id: textCol
        x: 16; y: 12; spacing: 3
        width: row.block ? row.width - 32 : row.width - 32 - holder.width - 16
        Item {
            width: parent.width; height: titleText.height
            Text { id: titleText; text: row.title; color: "#f1f0f7"; font.pixelSize: 16
                   elide: Text.ElideRight; width: Math.min(implicitWidth, parent.width - rightText.width - 8) }
            Text { id: rightText; anchors.right: parent.right; text: row.valueText
                   color: "#b9b5cc"; font.pixelSize: 14 }
        }
        Text { visible: row.description !== ""; text: row.description; width: parent.width
               color: "#8c88a0"; font.pixelSize: 12; wrapMode: Text.WordWrap }
    }

    Item {
        id: holder
        width: row.block ? row.width - 32 : childrenRect.width
        height: childrenRect.height
        x: row.block ? 16 : row.width - 16 - width
        y: row.block ? textCol.y + textCol.height + 10 : (row.height - height) / 2
    }

    Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom; leftMargin: 16 }
                height: 1; color: "#2f2b44" }
}
