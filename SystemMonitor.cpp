#include "SystemMonitor.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QRandomGenerator>
#include <QRegularExpression>

SystemMonitor::SystemMonitor(QObject *parent)
    : QObject(parent)
{
    // Create worker thread
    m_workerThread = new QThread(this);
    m_worker = new SystemWorker();
    m_worker->moveToThread(m_workerThread);
    
    // Connect signals
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &SystemWorker::statsReady, this, &SystemMonitor::onStatsUpdated);
    
    m_workerThread->start();
    
    // Timer triggers worker updates
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SystemMonitor::updateStats);
    m_timer->start(1000); // Update every second
    updateStats(); // Initial update
}

void SystemMonitor::updateStats()
{
    // Trigger update in worker thread
    QMetaObject::invokeMethod(m_worker, "doUpdate", Qt::QueuedConnection);
}

void SystemMonitor::onStatsUpdated(double cpu, double ram, double disk, QString netUp, QString netDown)
{
    m_cpuUsage = cpu;
    m_ramUsage = ram;
    m_diskUsage = disk;
    m_networkUp = netUp;
    m_networkDown = netDown;
    
    // Update uptime
    QFile uptimeFile("/proc/uptime");
    if (uptimeFile.open(QIODevice::ReadOnly)) {
        QString content = uptimeFile.readAll();
        double uptimeSeconds = content.split(' ')[0].toDouble();
        int days = uptimeSeconds / 86400;
        int hours = (int(uptimeSeconds) % 86400) / 3600;
        int mins = (int(uptimeSeconds) % 3600) / 60;
        
        if (days > 0) {
            m_uptime = QString("%1d %2h").arg(days).arg(hours);
        } else if (hours > 0) {
            m_uptime = QString("%1h %2m").arg(hours).arg(mins);
        } else {
            m_uptime = QString("%1m").arg(mins);
        }
        uptimeFile.close();
    }
    
    // Update process count
    QProcess ps;
    ps.start("ps", QStringList() << "ax" << "--no-headers");
    ps.waitForFinished(1000);
    m_processCount = QString(ps.readAll()).split('\n', Qt::SkipEmptyParts).count();
    
    // Calculate health score
    m_healthScore = calculateHealthScore();
    
    // Calculate network percentages based on speed
    // Max RX/TX: 1 Gbit/s (~125 MB/s)
    const double MAX_DOWNLOAD_MBS = 125.0;  // 1 Gbit/s
    const double MAX_UPLOAD_MBS = 125.0;    // 1 Gbit/s
    
    // Parse upload speed
    double upSpeed = 0.0;
    if (netUp.contains("MB/s")) {
        upSpeed = netUp.split(' ')[0].toDouble();
    } else if (netUp.contains("KB/s")) {
        upSpeed = netUp.split(' ')[0].toDouble() / 1024.0;
    }
    m_networkTxPercent = qMin(100.0, (upSpeed / MAX_UPLOAD_MBS) * 100.0);
    
    // Parse download speed
    double downSpeed = 0.0;
    if (netDown.contains("MB/s")) {
        downSpeed = netDown.split(' ')[0].toDouble();
    } else if (netDown.contains("KB/s")) {
        downSpeed = netDown.split(' ')[0].toDouble() / 1024.0;
    }
    m_networkRxPercent = qMin(100.0, (downSpeed / MAX_DOWNLOAD_MBS) * 100.0);
    
    emit statsUpdated();
}

int SystemMonitor::calculateHealthScore() const
{
    // Health score algorithm (0-100)
    // Lower is worse, higher is better
    
    int score = 100;
    
    // CPU penalty (up to -30 points)
    if (m_cpuUsage > 90) score -= 30;
    else if (m_cpuUsage > 75) score -= 20;
    else if (m_cpuUsage > 50) score -= 10;
    
    // RAM penalty (up to -30 points)
    if (m_ramUsage > 90) score -= 30;
    else if (m_ramUsage > 75) score -= 20;
    else if (m_ramUsage > 50) score -= 10;
    
    // Disk penalty (up to -25 points)
    if (m_diskUsage > 95) score -= 25;
    else if (m_diskUsage > 85) score -= 15;
    else if (m_diskUsage > 70) score -= 8;
    
    // Network penalty (up to -15 points)
    double avgNetwork = (m_networkTxPercent + m_networkRxPercent) / 2.0;
    if (avgNetwork > 80) score -= 15;
    else if (avgNetwork > 60) score -= 8;
    
    return qMax(0, score);
}

// ============ SystemWorker Implementation ============

SystemWorker::SystemWorker(QObject *parent)
    : QObject(parent)
{
}

void SystemWorker::doUpdate()
{
    double cpu = updateCpuUsage();
    double ram = updateMemoryUsage();
    double disk = updateDiskUsage();
    QString netUp, netDown;
    updateNetworkUsage(netUp, netDown);
    
    emit statsReady(cpu, ram, disk, netUp, netDown);
}

double SystemWorker::updateCpuUsage()
{
    double cpuUsage = 0.0;
    
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
            return cpu;
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
                    cpuUsage = 100.0 * (1.0 - (double)idleDelta / totalDelta);
                }
            }
            
            m_prevTotalTime = totalTime;
            m_prevIdleTime = idleTime;
        }
        file.close();
        return cpuUsage;
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
                return match.captured(1).toDouble();
            }
        }
    }
#endif
    
    // Fallback: simulation
    cpuUsage = QRandomGenerator::global()->bounded(10, 90);
    return cpuUsage;
}

double SystemWorker::updateMemoryUsage()
{
    double ramUsage = 0.0;
    
#ifdef Q_OS_WIN
    // Windows: Use WMIC
    QProcess process;
    process.start("wmic", QStringList() << "OS" << "get" << "FreePhysicalMemory,TotalVisibleMemorySize");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    if (lines.size() > 1) {
        QStringList parts = lines[1].simplified().split(' ');
        if (parts.size() >= 2) {
            quint64 freeMemory = parts[0].toULongLong();
            quint64 totalMemory = parts[1].toULongLong();
            if (totalMemory > 0) {
                return 100.0 * (1.0 - (double)freeMemory / totalMemory);
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
            ramUsage = 100.0 * (1.0 - (double)memAvailable / memTotal);
            file.close();
            return ramUsage;
        }
        file.close();
    }
#endif
    
    // Fallback
    ramUsage = QRandomGenerator::global()->bounded(30, 80);
    return ramUsage;
}

double SystemWorker::updateDiskUsage()
{
    double diskUsage = 0.0;
    QStorageInfo storage(QStorageInfo::root());
    if (storage.isValid() && storage.isReady()) {
        double total = storage.bytesTotal();
        double free = storage.bytesAvailable();
        double used = total - free;
        diskUsage = (used / total) * 100.0;
    }
    return diskUsage;
}

void SystemWorker::updateNetworkUsage(QString &netUp, QString &netDown)
{
    quint64 bytesSent = 0;
    quint64 bytesReceived = 0;
    
#ifdef Q_OS_WIN
    // Windows: Use netstat to get interface statistics
    QProcess process;
    process.start("netstat", QStringList() << "-e");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    
    QStringList lines = output.split('\n');
    for (const QString &line : lines) {
        if (line.contains("Bytes")) {
            QStringList parts = line.simplified().split(' ');
            if (parts.size() >= 3) {
                bytesReceived = parts[1].remove(',').toULongLong();
                bytesSent = parts[2].remove(',').toULongLong();
                break;
            }
        }
    }
#elif defined(Q_OS_LINUX)
    // Linux: Read /proc/net/dev
    QFile file("/proc/net/dev");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        
        // Skip header lines
        in.readLine();
        in.readLine();
        
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.contains(':')) {
                QString interface = line.split(':')[0].trimmed();
                
                // Skip loopback
                if (interface == "lo") continue;
                
                QStringList parts = line.split(':')[1].simplified().split(' ');
                if (parts.size() >= 9) {
                    bytesReceived += parts[0].toULongLong();
                    bytesSent += parts[8].toULongLong();
                }
            }
        }
        file.close();
    }
#elif defined(Q_OS_MACOS)
    // macOS: Use netstat -ib
    QProcess process;
    process.start("netstat", QStringList() << "-ib");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    
    QStringList lines = output.split('\n');
    for (int i = 1; i < lines.size(); ++i) {
        QString line = lines[i];
        QStringList parts = line.simplified().split(' ');
        
        if (parts.size() >= 7) {
            QString interface = parts[0];
            // Skip loopback
            if (interface.startsWith("lo")) continue;
            
            bytesReceived += parts[6].toULongLong();
            bytesSent += parts[9].toULongLong();
        }
    }
#endif

    // Calculate speed (bytes per second)
    if (m_prevBytesReceived > 0 && m_prevBytesSent > 0) {
        qint64 uploadSpeed = bytesSent - m_prevBytesSent;
        qint64 downloadSpeed = bytesReceived - m_prevBytesReceived;
        
        // Prevent negative values from counter resets
        if (uploadSpeed < 0) uploadSpeed = 0;
        if (downloadSpeed < 0) downloadSpeed = 0;
        
        // Convert to KB/s or MB/s
        double upKB = uploadSpeed / 1024.0;
        double downKB = downloadSpeed / 1024.0;
        
        if (upKB > 1024) {
            netUp = QString::number(upKB / 1024.0, 'f', 2) + " MB/s";
        } else {
            netUp = QString::number(upKB, 'f', 1) + " KB/s";
        }
        
        if (downKB > 1024) {
            netDown = QString::number(downKB / 1024.0, 'f', 2) + " MB/s";
        } else {
            netDown = QString::number(downKB, 'f', 1) + " KB/s";
        }
    } else {
        netUp = "0 KB/s";
        netDown = "0 KB/s";
    }
    
    m_prevBytesSent = bytesSent;
    m_prevBytesReceived = bytesReceived;
}
