#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QStorageInfo>

class SystemMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY statsUpdated)
    Q_PROPERTY(double ramUsage READ ramUsage NOTIFY statsUpdated)
    Q_PROPERTY(double diskUsage READ diskUsage NOTIFY statsUpdated)
    Q_PROPERTY(QString networkUp READ networkUp NOTIFY statsUpdated)
    Q_PROPERTY(QString networkDown READ networkDown NOTIFY statsUpdated)

public:
    explicit SystemMonitor(QObject *parent = nullptr);

    double cpuUsage() const { return m_cpuUsage; }
    double ramUsage() const { return m_ramUsage; }
    double diskUsage() const { return m_diskUsage; }
    QString networkUp() const { return m_networkUp; }
    QString networkDown() const { return m_networkDown; }

public slots:
    void updateStats();

signals:
    void statsUpdated();

private:
    QTimer *m_timer;
    double m_cpuUsage = 0;
    double m_ramUsage = 0;
    double m_diskUsage = 0;
    QString m_networkUp = "0 KB/s";
    QString m_networkDown = "0 KB/s";

    // Previous values for calculating deltas
    quint64 m_prevTotalTime = 0;
    quint64 m_prevIdleTime = 0;

    void updateCpuUsage();
    void updateMemoryUsage();
    void updateDiskUsage();
    void updateNetworkUsage();
};

#endif // SYSTEMMONITOR_H
