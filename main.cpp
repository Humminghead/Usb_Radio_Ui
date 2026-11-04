#include "Audio/AudioModel.h"
#include "Settings/AppSettings.h"
#include "Tray/TrayController.h"
#include "Usb/HidDevice.h"
#include "Audio/AudioDevice.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("HidRadio");
    QCoreApplication::setApplicationName("HidRadio");
    auto settings = std::make_shared<ComboBoxSettings>();
    SettingsComboBoxModel audioOutputs(settings, AudioDeviceList{AudioDeviceRole::Output});
    SettingsComboBoxModel audioInputs(settings, AudioDeviceList{AudioDeviceRole::Input});

    Usb::Hid::Device hid(0xc0de, 0xcafe);
    TrayController tray;    

    // keep running when the window is folded into the tray
    app.setQuitOnLastWindowClosed(!tray.available());

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("hid", &hid);
    engine.rootContext()->setContextProperty("tray", &tray);
    engine.rootContext()->setContextProperty("settings", settings.get());
    engine.rootContext()->setContextProperty("audioOutputs", &audioOutputs);
    engine.rootContext()->setContextProperty("audioInputs", &audioInputs);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("Radio", "Main");
    return app.exec();
}
