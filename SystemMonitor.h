#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QStorageInfo>
#include <QThread>

class SystemWorker;

class SystemMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY statsUpdated)
    Q_PROPERTY(double ramUsage READ ramUsage NOTIFY statsUpdated)
    Q_PROPERTY(double diskUsage READ diskUsage NOTIFY statsUpdated)
    Q_PROPERTY(QString networkUp READ networkUp NOTIFY statsUpdated)
    Q_PROPERTY(QString networkDown READ networkDown NOTIFY statsUpdated)
    Q_PROPERTY(double networkTxPercent READ networkTxPercent NOTIFY statsUpdated)
    Q_PROPERTY(double networkRxPercent READ networkRxPercent NOTIFY statsUpdated)
    Q_PROPERTY(int healthScore READ healthScore NOTIFY statsUpdated)
    Q_PROPERTY(QString uptime READ uptime NOTIFY statsUpdated)
    Q_PROPERTY(int processCount READ processCount NOTIFY statsUpdated)
    Q_PROPERTY(QString cpuModel READ cpuModel NOTIFY systemInfoUpdated)
    Q_PROPERTY(int cpuCores READ cpuCores NOTIFY systemInfoUpdated)
    Q_PROPERTY(QString totalRam READ totalRam NOTIFY systemInfoUpdated)
    Q_PROPERTY(QString osInfo READ osInfo NOTIFY systemInfoUpdated)
    Q_PROPERTY(QString kernelVersion READ kernelVersion NOTIFY systemInfoUpdated)
    Q_PROPERTY(QString hostname READ hostname NOTIFY systemInfoUpdated)

public:
    explicit SystemMonitor(QObject *parent = nullptr);

    double cpuUsage() const { return m_cpuUsage; }
    double ramUsage() const { return m_ramUsage; }
    double diskUsage() const { return m_diskUsage; }
    QString networkUp() const { return m_networkUp; }
    QString networkDown() const { return m_networkDown; }
    double networkTxPercent() const { return m_networkTxPercent; }
    double networkRxPercent() const { return m_networkRxPercent; }
    int healthScore() const { return m_healthScore; }
    QString uptime() const { return m_uptime; }
    int processCount() const { return m_processCount; }
    QString cpuModel() const { return m_cpuModel; }
    int cpuCores() const { return m_cpuCores; }
    QString totalRam() const { return m_totalRam; }
    QString osInfo() const { return m_osInfo; }
    QString kernelVersion() const { return m_kernelVersion; }
    QString hostname() const { return m_hostname; }

public slots:
    void updateStats();

signals:
    void statsUpdated();
    void systemInfoUpdated();

private slots:
    void onStatsUpdated(double cpu, double ram, double disk, QString netUp, QString netDown);

private:
    QTimer *m_timer;
    double m_cpuUsage = 0.0;
    double m_ramUsage = 0.0;
    double m_diskUsage = 0.0;
    QString m_networkUp = "0 KB/s";
    QString m_networkDown = "0 KB/s";
    double m_networkTxPercent = 0.0;
    double m_networkRxPercent = 0.0;
    int m_healthScore = 100;
    QString m_uptime = "0s";
    int m_processCount = 0;
    QString m_cpuModel = "Unknown";
    int m_cpuCores = 0;
    QString m_totalRam = "0 GB";
    QString m_osInfo = "Unknown";
    QString m_kernelVersion = "Unknown";
    QString m_hostname = "localhost";

    QThread *m_workerThread = nullptr;
    SystemWorker *m_worker = nullptr;
    
    int calculateHealthScore() const;
    void loadSystemInfo();
};

// Worker class for background monitoring
class SystemWorker : public QObject
{
    Q_OBJECT

public:
    explicit SystemWorker(QObject *parent = nullptr);

public slots:
    void doUpdate();

signals:
    void statsReady(double cpu, double ram, double disk, QString netUp, QString netDown);

private:
    // For CPU calculation
    quint64 m_prevTotalTime = 0;
    quint64 m_prevIdleTime = 0;

    // For network calculation
    quint64 m_prevBytesSent = 0;
    quint64 m_prevBytesReceived = 0;

    double updateCpuUsage();
    double updateMemoryUsage();
    double updateDiskUsage();
    void updateNetworkUsage(QString &netUp, QString &netDown);
};

#endif // SYSTEMMONITOR_H
