import QtQuick

SettingsGroup {
    id: page

    SettingInfo { title: "Device"; description: "USB vendor : product ID"; infoText: "c0de:cafe" }
    SettingInfo { title: "Status"; infoText: hid.Connected ? "Connected" : "Disconnected" }
    SettingInfo { title: "Frequency"; infoText: hid.Frequency.toFixed(1) + " MHz" }
    SettingInfo { title: "Last error"; infoText: hid.LastError || "—" }
    SettingButton {
        title: "Connection"
        buttonText: hid.Connected ? "Disconnect" : "Connect"
        onClicked: hid.Connected ? hid.close() : hid.open()
    }
    SettingSwitch {
        title: "Auto-connect"
        description: "Keep trying to open the device on start (applies after restart)"
        key: "hardware/autoConnect"; defaultValue: true
    }
    // add more Hardware settings here
}
