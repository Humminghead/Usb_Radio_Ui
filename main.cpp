#include "Usb/HidDevice.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    HidDevice hid;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("hid", &hid);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("Radio", "Main");
    return app.exec();
}
