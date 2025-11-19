#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "SystemMonitor.h"
#include "DiskAnalyzer.h"
#include "ServerManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<SystemMonitor>("App", 1, 0, "SystemMonitor");
    qmlRegisterType<DiskAnalyzer>("App", 1, 0, "DiskAnalyzer");
    qmlRegisterType<ServerManager>("App", 1, 0, "ServerManager");

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
