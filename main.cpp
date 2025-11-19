#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "SystemMonitor.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<SystemMonitor>("App", 1, 0, "SystemMonitor");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("qt_slr", "Main");

    return app.exec();
}
