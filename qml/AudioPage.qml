import QtQuick

SettingsGroup {
    id: page

    SettingsComboBox {
        title: "Audio output"
        description: "Sound device used for playback"
        model: audioOutputs                      // C++ QAbstractListModel
        textRole: "name"
        currentIndex: audioOutputs.currentIndex
        onActivated: (i) => audioOutputs.currentIndex = i
    }

    SettingsComboBox {
        title: "Audio input"
        description: "Sound device used for input"
        model: audioInputs                      // C++ QAbstractListModel
        textRole: "name"
        currentIndex: audioInputs.currentIndex
        onActivated: (i) => audioInputs.currentIndex = i
    }

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
