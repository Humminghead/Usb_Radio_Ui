import QtQuick
import QtQuick.Window

Window {
    id: root
    width: 360; height: 600; visible: true
    title: "HID Radio"
    color: "#202020"

    // ---- Skin registry: add a new skin = add a file + one line here ----
    readonly property var skins: [
        { name: "Nokia",   source: "skins/NokiaSkin.qml" },
        { name: "Minimal", source: "skins/MinimalSkin.qml" }
    ]
    property int skinIndex: 0
    property bool playing: true

    Loader {
        id: loader
        anchors.fill: parent
    }

    // The skin is created with the current state as initial properties, so a freshly
    // loaded skin starts at the confirmed frequency and never replays an animation.
    function loadSkin() {
        loader.setSource(Qt.resolvedUrl(skins[skinIndex].source), {
            "frequency": hid.frequency,
            "connected": hid.connected,
            "playing":   root.playing,
            "skinName":  skins[skinIndex].name,
            "status":    hid.lastError
        })
    }
    onSkinIndexChanged: loadSkin()

    // Everything a skin gets/sends is defined by SkinBase.qml
    Binding { target: loader.item; property: "frequency"; value: hid.frequency; when: loader.item }
    Binding { target: loader.item; property: "connected";  value: hid.connected; when: loader.item }
    Binding { target: loader.item; property: "playing";    value: root.playing;  when: loader.item }
    Binding { target: loader.item; property: "skinName";   value: root.skins[root.skinIndex].name; when: loader.item }
    Binding { target: loader.item; property: "status";     value: hid.lastError; when: loader.item }

    Connections {
        target: loader.item
        function onTuneRequested(mhz) { hid.setFrequency(mhz) }
        function onStepRequested(dir) { hid.seek(dir) }
        function onPlayToggled()      { root.playing = !root.playing }   // TODO: send mute/play command if your firmware has one
        function onConnectRequested() { hid.connected ? hid.close() : hid.open() }
        function onNextSkinRequested() { root.skinIndex = (root.skinIndex + 1) % root.skins.length }
    }

    Connections {
        target: hid
        function onReportReceived(data) {
            // TODO: decode the 7-byte input report (Report ID 1) here
            console.log("RX:", Array.from(new Uint8Array(data)).map(b => b.toString(16).padStart(2, "0")).join(" "))
        }
    }

    Component.onCompleted: {
        if (hid.open())
            hid.setFrequency(99.2)
        loadSkin()
    }
}
