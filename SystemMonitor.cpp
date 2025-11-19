#include "SystemMonitor.h"
#include <QDebug>

SystemMonitor::SystemMonitor(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SystemMonitor::updateStats);
    m_timer->start(1000); // Update every second
    updateStats(); // Initial update
}

void SystemMonitor::updateStats()
{
    // Simulated Data for Demo Mode
    // In a real application, you would read /proc/stat (Linux) or PDH (Windows)
    
    // Simulate fluctuating CPU between 10% and 90%
    double currentCpu = m_cpuUsage;
    double targetCpu = QRandomGenerator::global()->bounded(10, 90);
    // Smooth transition
    m_cpuUsage = currentCpu * 0.7 + targetCpu * 0.3;

    // Simulate RAM usage slowly increasing/decreasing
    double currentRam = m_ramUsage;
    if (currentRam == 0) currentRam = 40;
    double change = QRandomGenerator::global()->bounded(-2, 3);
    m_ramUsage = qBound(20.0, currentRam + change, 95.0);

    // Simulate Disk usage (static-ish)
    m_diskUsage = 65.4;

    // Simulate Network
    int up = QRandomGenerator::global()->bounded(0, 5000); // KB/s
    int down = QRandomGenerator::global()->bounded(0, 50000); // KB/s
    
    m_networkUp = (up > 1000) ? QString::number(up / 1000.0, 'f', 1) + " MB/s" : QString::number(up) + " KB/s";
    m_networkDown = (down > 1000) ? QString::number(down / 1000.0, 'f', 1) + " MB/s" : QString::number(down) + " KB/s";

    emit statsUpdated();
}
