import QtQuick

SettingsGroup {
    id: page

    SettingChoice {
        title: "Skin"
        description: "Look of the main window"
        options: page.shell ? page.shell.skins.map(s => s.name) : []
        value: page.shell ? page.shell.skinIndex : 0
        onUserChanged: (v) => page.shell.skinSelected(v)
    }
    SettingSwitch {
        title: "Close button folds to tray"
        description: tray.available ? "Otherwise the close button quits the app" : "System tray is not available"
        key: "general/closeToTray"; defaultValue: true
        enabled: tray.available
    }
    SettingSwitch {
        title: "Start in tray"
        key: "general/startInTray"; defaultValue: false
        enabled: tray.available
    }
    SettingButton {
        title: "Reset settings"
        description: "Restore every setting to its default"
        buttonText: "Reset"
        onClicked: settings.reset()
    }
    // add more General settings here
}
