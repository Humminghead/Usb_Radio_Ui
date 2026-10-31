import QtQuick

SettingsGroup {
    id: page

    SettingSwitch {
        title: "Mute"
        description: "Silence output and pause the visualizer"
        value: !page.shell.playing                    // no key: state lives in Main.qml
        onUserChanged: page.shell.muteToggled()
    }
    SettingSlider {
        title: "Volume"
        description: "Saved; sent to the device once a SetVolume call exists (see Main.qml)"
        key: "audio/volume"; defaultValue: 80
        from: 0; to: 100; suffix: " %"
    }
    // add more Audio settings here
}
