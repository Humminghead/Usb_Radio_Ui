import QtQuick
import QtQuick.Controls.Basic     // Basic style: fully customisable, same look on every platform

// Drop-down setting row backed by any QAbstractItemModel / list model.
// Usage: SettingsComboBox { title: "..."; model: cppModel; textRole: "name"
//                           currentIndex: cppModel.currentIndex
//                           onActivated: (i) => cppModel.currentIndex = i }
SettingRow {
    id: row
    block: true
    required property var model
    property string textRole: "display"
    property int currentIndex: -1
    signal activated(int index)

    ComboBox {
        id: cb
        width: parent.width; height: 40
        model: row.model
        textRole: row.textRole
        enabled: count > 0
        displayText: count > 0 ? currentText : qsTr("No devices found")

        // model resets reset the ComboBox's own index: keep following the C++ side
        Binding { target: cb; property: "currentIndex"; value: row.currentIndex }
        onActivated: (i) => row.activated(i)

        background: Rectangle {
            radius: 8; color: "#2c2840"
            border.width: 1; border.color: cb.visualFocus || cb.popup.visible ? "#6a4cff" : "#3d3956"
        }
        contentItem: Text {
            leftPadding: 12; rightPadding: 36
            text: cb.displayText; font.pixelSize: 14; color: "#f1f0f7"
            verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight
        }
        indicator: Canvas {
            x: cb.width - width - 12; y: (cb.height - height) / 2; width: 12; height: 8
            rotation: cb.popup.visible ? 180 : 0
            onPaint: {
                const c = getContext("2d"); c.clearRect(0, 0, width, height)
                c.strokeStyle = "#b9b5cc"; c.lineWidth = 2; c.lineCap = "round"; c.lineJoin = "round"
                c.beginPath(); c.moveTo(1, 1.5); c.lineTo(6, 6.5); c.lineTo(11, 1.5); c.stroke()
            }
        }

        // one delegate instance per model row, created by the view from the C++ model
        delegate: ItemDelegate {
            id: del
            required property var model
            required property int index
            width: cb.width; height: 40
            highlighted: cb.highlightedIndex === index
            contentItem: Row {
                spacing: 8
                Text {
                    width: del.width - (badge.visible ? badge.width + 8 : 0) - 24
                    anchors.verticalCenter: parent.verticalCenter
                    text: del.model[cb.textRole]; elide: Text.ElideRight; font.pixelSize: 14
                    color: cb.currentIndex === del.index ? "#a99bff" : "#f1f0f7"
                }
                Text {
                    id: badge
                    visible: del.model.isDefault === true
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("default"); font.pixelSize: 11; color: "#8c88a0"
                }
            }
            background: Rectangle { color: del.highlighted ? "#3a3550" : "transparent" }
        }

        popup: Popup {
            y: cb.height + 4
            width: cb.width
            padding: 1
            implicitHeight: Math.min(contentItem.implicitHeight + 2, 240)
            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: cb.popup.visible ? cb.delegateModel : null
                currentIndex: cb.highlightedIndex
                ScrollIndicator.vertical: ScrollIndicator { }
            }
            background: Rectangle { radius: 8; color: "#2c2840"; border.width: 1; border.color: "#3d3956" }
        }
    }
}
