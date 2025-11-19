#include "SystemMonitor.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QRandomGenerator>
#include <QRegularExpression>

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
    updateCpuUsage();
    updateMemoryUsage();
    updateDiskUsage();
    updateNetworkUsage();
    
    emit statsUpdated();
}

void SystemMonitor::updateCpuUsage()
{
#ifdef Q_OS_WIN
    // Windows: Use WMIC command
    QProcess process;
    process.start("wmic", QStringList() << "cpu" << "get" << "loadpercentage");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    if (lines.size() > 1) {
        bool ok;
        double cpu = lines[1].trimmed().toDouble(&ok);
        if (ok) {
            m_cpuUsage = cpu;
            return;
        }
    }
#elif defined(Q_OS_LINUX)
    // Linux: Read /proc/stat
    QFile file("/proc/stat");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        QString line = in.readLine();
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        
        if (parts.size() >= 5 && parts[0] == "cpu") {
            quint64 user = parts[1].toULongLong();
            quint64 nice = parts[2].toULongLong();
            quint64 system = parts[3].toULongLong();
            quint64 idle = parts[4].toULongLong();
            
            quint64 totalTime = user + nice + system + idle;
            quint64 idleTime = idle;
            
            if (m_prevTotalTime > 0) {
                quint64 totalDelta = totalTime - m_prevTotalTime;
                quint64 idleDelta = idleTime - m_prevIdleTime;
                
                if (totalDelta > 0) {
                    m_cpuUsage = 100.0 * (1.0 - (double)idleDelta / totalDelta);
                }
            }
            
            m_prevTotalTime = totalTime;
            m_prevIdleTime = idleTime;
        }
        file.close();
        return;
    }
#elif defined(Q_OS_MACOS)
    // macOS: Use top command
    QProcess process;
    process.start("top", QStringList() << "-l" << "1" << "-n" << "0");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    
    // Parse CPU usage from top output
    QStringList lines = output.split('\n');
    for (const QString &line : lines) {
        if (line.contains("CPU usage")) {
            // Extract percentage
            QRegularExpression re("(\\d+\\.\\d+)%\\s+user");
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                m_cpuUsage = match.captured(1).toDouble();
                return;
            }
        }
    }
#endif
    
    // Fallback: smooth simulation
    double target = QRandomGenerator::global()->bounded(10, 90);
    m_cpuUsage = m_cpuUsage * 0.7 + target * 0.3;
}

void SystemMonitor::updateMemoryUsage()
{
#ifdef Q_OS_WIN
    // Windows: Use WMIC
    QProcess process;
    process.start("wmic", QStringList() << "OS" << "get" << "FreePhysicalMemory,TotalVisibleMemorySize");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    if (lines.size() > 1) {
        QStringList values = lines[1].simplified().split(' ');
        if (values.size() >= 2) {
            quint64 free = values[0].toULongLong();
            quint64 total = values[1].toULongLong();
            if (total > 0) {
                m_ramUsage = 100.0 * (1.0 - (double)free / total);
                return;
            }
        }
    }
#elif defined(Q_OS_LINUX)
    // Linux: Read /proc/meminfo
    QFile file("/proc/meminfo");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        quint64 memTotal = 0, memAvailable = 0;
        
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("MemTotal:")) {
                memTotal = line.split(QRegularExpression("\\s+"))[1].toULongLong();
            } else if (line.startsWith("MemAvailable:")) {
                memAvailable = line.split(QRegularExpression("\\s+"))[1].toULongLong();
            }
        }
        
        if (memTotal > 0) {
            m_ramUsage = 100.0 * (1.0 - (double)memAvailable / memTotal);
            file.close();
            return;
        }
        file.close();
    }
#endif
    
    // Fallback
    double target = QRandomGenerator::global()->bounded(30, 80);
    m_ramUsage = m_ramUsage * 0.9 + target * 0.1;
}

void SystemMonitor::updateDiskUsage()
{
    QStorageInfo storage(QStorageInfo::root());
    if (storage.isValid() && storage.isReady()) {
        double total = storage.bytesTotal();
        double free = storage.bytesAvailable();
        double used = total - free;
        m_diskUsage = (used / total) * 100.0;
    }
}

void SystemMonitor::updateNetworkUsage()
{
    // Network monitoring is complex and OS-specific
    // For now, keep it simulated
    int up = QRandomGenerator::global()->bounded(0, 5000);
    int down = QRandomGenerator::global()->bounded(0, 50000);
    
    m_networkUp = (up > 1000) ? QString::number(up / 1000.0, 'f', 1) + " MB/s" : QString::number(up) + " KB/s";
    m_networkDown = (down > 1000) ? QString::number(down / 1000.0, 'f', 1) + " MB/s" : QString::number(down) + " KB/s";
}
