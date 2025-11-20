#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include "SystemMonitor.h"
#include "DiskAnalyzer.h"
#include "ServerManager.h"

// Global log file
QFile *logFile = nullptr;

// Custom message handler for logging
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString formattedMessage;
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    
    switch (type) {
    case QtDebugMsg:
        formattedMessage = QString("[%1] DEBUG: %2").arg(timestamp, msg);
        break;
    case QtInfoMsg:
        formattedMessage = QString("[%1] INFO: %2").arg(timestamp, msg);
        break;
    case QtWarningMsg:
        formattedMessage = QString("[%1] WARNING: %2").arg(timestamp, msg);
        break;
    case QtCriticalMsg:
        formattedMessage = QString("[%1] CRITICAL: %2").arg(timestamp, msg);
        break;
    case QtFatalMsg:
        formattedMessage = QString("[%1] FATAL: %2").arg(timestamp, msg);
        break;
    }
    
    // Write to console
    fprintf(stderr, "%s\n", formattedMessage.toLocal8Bit().constData());
    
    // Write to file if available
    if (logFile && logFile->isOpen()) {
        QTextStream stream(logFile);
        stream << formattedMessage << "\n";
        stream.flush();
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // Set up logging to file
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(logDir);
    QString logPath = logDir + "/debug.log";
    
    logFile = new QFile(logPath);
    if (logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qInstallMessageHandler(messageHandler);
        qDebug() << "=== Application started ===" << QDateTime::currentDateTime().toString();
        qDebug() << "Log file:" << logPath;
    } else {
        qWarning() << "Failed to open log file:" << logPath;
    }

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

    int result = app.exec();
    
    // Clean up logging
    qDebug() << "=== Application exiting ===" << QDateTime::currentDateTime().toString();
    if (logFile) {
        logFile->close();
        delete logFile;
    }
    
    return result;
}
