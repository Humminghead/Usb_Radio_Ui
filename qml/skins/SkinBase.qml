import QtQuick

// Contract between the app and every skin.
//
// Internal animation state (convention, NOT part of the contract):
// a skin MAY declare a skin-local `real displayFrequency` that animates toward
// `frequency`. The shell never reads or writes it.
Item {
    // state (set by app)
    property real   frequency: minFreq
    property bool   connected: false
    property bool   playing: false
    property bool   uiActive: false
    property string skinName: ""
    property string status: ""
    property string stationName: "Station - " + frequency.toFixed(1) + " FM"
    property string stationInfo: "COMRCIAL"

    // requests (handled by app)
    signal tuneRequested(real mhz)
    signal stepRequested(int dir)
    signal playToggled()
    signal powerUpToggled()
    signal connectRequested()
    signal settingsRequested()

    readonly property real minFreq: 87.5
    readonly property real maxFreq: 108.0
}
