#include "Audio/AudioDeviceList.h"
#include "Audio/AudioFormatList.h"
#include "Audio/SettingsComboBoxModel.h"
#include "Settings/AppSettings.h"
#include "Tray/TrayController.h"
#include "Usb/HidDevice.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("HidRadio");
    QCoreApplication::setApplicationName("HidRadio");
    auto settings = std::make_shared<ComboBoxSettings>();
    auto audioOutList = std::make_shared<AudioDeviceList>(AudioDeviceRole::Output);
    auto audioInList = std::make_shared<AudioDeviceList>(AudioDeviceRole::Input);
    auto audioFormatList = std::make_shared<AudioFormatList>(audioInList);

    SettingsComboBoxModel audioOutputs(settings, audioOutList);
    SettingsComboBoxModel audioInputs(settings, audioInList);
    SettingsComboBoxModel audioInputFormats(settings, audioFormatList);

    // Re-sync the input devives and their parameters
    auto ReloadInputComboBoxes = [&] {
        if (const auto currentIdx = audioInputs.currentIndex(); currentIdx < audioInList->Size()) {
            const auto id = audioInList->GetElementId(currentIdx);
            audioFormatList->SetCurrentDeviceId(id);
            audioInputFormats.Reset();
        }
    };

    // Do the connections
    QObject::connect(&audioInputs, &SettingsComboBoxModel::countChanged, [&]() { audioInputFormats.Reset(); });
    QObject::connect(&audioInputs, &SettingsComboBoxModel::currentIndexChanged, [&] { ReloadInputComboBoxes(); });
    QObject::connect(&audioInputs, &SettingsComboBoxModel::modelReset, [&](auto) { ReloadInputComboBoxes(); });

    // Create HID
    Usb::Hid::Device hid(0xc0de, 0xcafe);
    TrayController tray;

    // Keep running when the window is folded into the tray
    app.setQuitOnLastWindowClosed(!tray.available());

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("hid", &hid);
    engine.rootContext()->setContextProperty("tray", &tray);
    engine.rootContext()->setContextProperty("settings", settings.get());
    engine.rootContext()->setContextProperty("audioOutputs", &audioOutputs);
    engine.rootContext()->setContextProperty("audioInputs", &audioInputs);
    engine.rootContext()->setContextProperty("audioInputFormats", &audioInputFormats);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("Radio", "Main");

    // Reset all models after QML loading
    audioOutputs.Reset();
    audioInputs.Reset();

    return app.exec();
}
