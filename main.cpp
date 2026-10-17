#include "Tray/TrayController.h"
#include "Usb/HidDevice.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    HidDevice hid;
    TrayController tray;
    // keep running when the window is folded into the tray
    app.setQuitOnLastWindowClosed(!tray.available());

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("hid", &hid);
    engine.rootContext()->setContextProperty("tray", &tray);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("Radio", "Main");
    return app.exec();
}
