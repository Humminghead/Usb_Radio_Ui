#include "Audio/AudioDeviceList.h"
#include "Audio/AudioFormatList.h"
#include "Audio/SettingsComboBoxModel.h"
#include "Settings/AppSettings.h"
#include "Tray/TrayController.h"
#include "Usb/HidDevice.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QAudioOutput>

auto UpdateDevice =
    []<typename AudioCh>(AudioCh &ch, const SettingsComboBoxModel &model, std::shared_ptr<AudioDeviceList> list) {
        if (auto idx = model.currentIndex(); idx >= 0) {
            auto &dev = list->At(idx);
            ch.setDevice(dev);
        }
    };

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

    // Audio capture
    QMediaCaptureSession audioSessionIn;
    QAudioInput audioIn;
    QAudioOutput audioOut;
    audioSessionIn.setAudioInput(&audioIn);
    audioSessionIn.setAudioOutput(&audioOut);

    // Re-sync the input devives and their parameters
    auto ReloadInputComboBoxes = [&] {
        if (const auto currentIdx = audioInputs.currentIndex(); currentIdx < audioInList->Size()) {
            const auto id = audioInList->GetElementId(currentIdx);
            audioFormatList->SetCurrentDeviceId(id);
            audioInputFormats.Reset();
        }
    };

    // Do the connections
    // In
    QObject::connect(&audioInputs, &SettingsComboBoxModel::countChanged, [&]() {
        audioSessionIn.setAudioInput(nullptr);
        audioInputFormats.Reset();
        audioSessionIn.setAudioInput(&audioIn);
    });
    QObject::connect(&audioInputs, &SettingsComboBoxModel::currentIndexChanged, [&] {
        ReloadInputComboBoxes();
        UpdateDevice(audioIn, audioInputs, audioInList);
    });
    QObject::connect(&audioInputs, &SettingsComboBoxModel::modelReset, [&](auto) { ReloadInputComboBoxes(); });
    QObject::connect(&audioInputFormats, &SettingsComboBoxModel::currentIndexChanged,[](){
        ///\todo
        // qDebug()<<"AUDIO INPUT FORMAT CHANGED";
    });
    // Out
    QObject::connect(&audioOutputs, &SettingsComboBoxModel::countChanged, [&]() {
        audioSessionIn.setAudioOutput(nullptr);
        audioSessionIn.setAudioOutput(&audioOut);
    });
    QObject::connect(&audioOutputs, &SettingsComboBoxModel::currentIndexChanged, [&]{
        UpdateDevice(audioOut,audioOutputs,audioOutList);
    });

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
