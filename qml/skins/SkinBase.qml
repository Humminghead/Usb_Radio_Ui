import QtQuick

// Contract between the app and every skin.
Item {
    // state (set by app)
    property real   frequency: 99.2
    property bool   connected: false
    property bool   playing: true
    property string skinName: ""
    property string status: ""
    property string stationName: "Station - " + frequency.toFixed(1) + " FM"
    property string stationInfo: "COMRCIAL"

    // requests (handled by app)
    signal tuneRequested(real mhz)
    signal stepRequested(int dir)
    signal playToggled()
    signal connectRequested()
    signal nextSkinRequested()

    readonly property real minFreq: 87.5
    readonly property real maxFreq: 108.0
}
