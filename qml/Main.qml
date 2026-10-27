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
    property bool active: false

    Item {
        Timer {
            id: timer
            interval: 500; running: false; repeat: true
            onTriggered: {
                if(!active){
                    active = hid.open();
                }

                timer.running = !active;
            }
        }

        Text { id: time }
    }

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
        function onPowerUpToggled()   { hid.powerUp() }
        function onConnectRequested() { hid.connected ? hid.close() : hid.open() }
        function onNextSkinRequested() { root.skinIndex = (root.skinIndex + 1) % root.skins.length }
    }

    // ---- tray: fold / unfold ----
    function toggleWindow() {
        if (root.visible && root.visibility !== Window.Minimized) {
            root.hide()
        } else {
            root.showNormal(); root.raise(); root.requestActivate()
        }
    }
    onVisibleChanged: tray.windowVisible = visible
    // close button folds into the tray (only when a tray exists)
    onClosing: (close) => {
        if (tray.available) { close.accepted = false; root.hide() }
    }
    Binding { target: tray; property: "connected"; value: hid.connected }
    Connections {
        target: tray
        function onToggleRequested()        { root.toggleWindow() }
        function onConnectToggleRequested() { hid.connected ? hid.close() : hid.open() }
        function onQuitRequested()          { Qt.quit() }
    }

    Connections {
        target: hid
        function onReportReceived(data) {
            // TODO: decode the 7-byte input report (Report ID 1) here
            console.log("RX:", Array.from(new Uint8Array(data)).map(b => b.toString(16).padStart(2, "0")).join(" "))
        }
    }

    Component.onCompleted: {        
        loadSkin()
        timer.running = true;
    }
}
