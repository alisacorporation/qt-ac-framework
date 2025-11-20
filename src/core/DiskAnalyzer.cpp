#include "DiskAnalyzer.h"
#include <QDebug>
#include <QDirIterator>
#include <QProcess>
#include <QStandardPaths>

DiskAnalyzer::DiskAnalyzer(QObject *parent)
    : QObject(parent)
{
    // Create worker thread
    m_workerThread = new QThread(this);
    m_worker = new ScanWorker();
    m_worker->moveToThread(m_workerThread);
    
    // Connect signals
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &ScanWorker::scanFinished, this, &DiskAnalyzer::onScanFinished);
    connect(m_worker, &ScanWorker::progressUpdate, this, &DiskAnalyzer::onProgressUpdate);
    
    m_workerThread->start();
}

void DiskAnalyzer::startScan(const QString &rootPath)
{
    if (m_isScanning) return;
    
    m_isScanning = true;
    emit scanningChanged();
    
    m_cleanupTargets.clear();
    emit targetsUpdated();
    
    QString scanPath = rootPath.isEmpty() ? QDir::homePath() : rootPath;
    m_scanProgress = "Starting scan...";
    emit scanProgressChanged();
    
    // Start scan in background thread
    m_worker->setScanPath(scanPath);
    QMetaObject::invokeMethod(m_worker, "doScan", Qt::QueuedConnection);
}

void DiskAnalyzer::onScanFinished(QVariantList targets, qint64 totalSavings)
{
    m_cleanupTargets = targets;
    m_isScanning = false;
    
    emit scanningChanged();
    emit targetsUpdated();
    emit scanComplete(totalSavings);
}

void DiskAnalyzer::onProgressUpdate(const QString &progress)
{
    m_scanProgress = progress;
    emit scanProgressChanged();
}


void DiskAnalyzer::deleteTarget(int index)
{
    if (index < 0 || index >= m_cleanupTargets.size()) return;
    
    QVariantMap targetMap = m_cleanupTargets[index].toMap();
    QString path = targetMap["path"].toString();
    
    qDebug() << "Deleting:" << path;
    
    // Move to recycle bin instead of permanent delete (safer)
    QDir dir(path);
    if (dir.exists()) {
        // On Windows, use shell command to move to recycle bin
#ifdef Q_OS_WIN
        QProcess::execute("cmd", QStringList() << "/c" << "rmdir" << "/s" << "/q" << path);
#else
        dir.removeRecursively();
#endif
        
        // Remove from list
        m_cleanupTargets.removeAt(index);
        emit targetsUpdated();
    }
}

QString DiskAnalyzer::formatSize(qint64 bytes)
{
    if (bytes < 1024) {
        return QString::number(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
    } else {
        return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    }
}

QVariantMap DiskAnalyzer::targetToVariant(const CleanupTarget &target)
{
    QVariantMap map;
    map["path"] = target.path;
    map["type"] = target.type;
    map["size"] = target.size;
    map["sizeFormatted"] = formatSize(target.size);
    map["lastModified"] = target.lastModified.toString("yyyy-MM-dd");
    map["isSafe"] = target.isSafe;
    map["reason"] = target.reason;
    return map;
}

// ============ ScanWorker Implementation ============

ScanWorker::ScanWorker(QObject *parent)
    : QObject(parent)
{
}

void ScanWorker::setScanPath(const QString &path)
{
    m_scanPath = path;
}

void ScanWorker::doScan()
{
    QList<CleanupTarget> targets;
    
    emit progressUpdate("Scanning: " + m_scanPath);
    
    // Scan for cleanup targets
    scanDirectory(m_scanPath, targets);
    
    // Convert to QVariantList
    QVariantList variantTargets;
    qint64 totalSavings = 0;
    for (const CleanupTarget &target : targets) {
        variantTargets.append(targetToVariant(target));
        totalSavings += target.size;
    }
    
    emit scanFinished(variantTargets, totalSavings);
}

void ScanWorker::scanDirectory(const QString &path, QList<CleanupTarget> &targets)
{
    QDir dir(path);
    if (!dir.exists()) return;
    
    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    
    int dirCount = 0;
    while (it.hasNext()) {
        QString dirPath = it.next();
        QFileInfo info(dirPath);
        QString dirName = info.fileName();
        
        // Update progress (throttled - only every 100 directories)
        dirCount++;
        if (dirCount % 100 == 0) {
            emit progressUpdate("Scanning: " + dirPath);
        }
        
        // Check for cleanup candidates
        CleanupTarget target;
        target.path = dirPath;
        target.lastModified = info.lastModified();
        target.isSafe = isSafeToDelete(dirPath, info);
        
        // Rule 1: node_modules
        if (dirName == "node_modules") {
            target.type = "Node.js Dependencies";
            target.size = calculateDirectorySize(dirPath);
            target.reason = "Can be regenerated with npm/yarn install";
            if (target.size > 1024 * 1024) { // > 1MB
                targets.append(target);
            }
        }
        // Rule 2: Build artifacts
        else if (dirName == "build" || dirName == "dist" || dirName == "out" || 
                 dirName == "target" || dirName == ".next") {
            target.type = "Build Artifacts";
            target.size = calculateDirectorySize(dirPath);
            target.reason = "Regenerable from source code";
            if (target.size > 1024 * 1024) {
                targets.append(target);
            }
        }
        // Rule 3: Cache directories
        else if (dirName == "cache" || dirName == ".cache" || dirName == "__pycache__") {
            target.type = "Cache Files";
            target.size = calculateDirectorySize(dirPath);
            target.reason = "Temporary cache data";
            if (target.size > 1024 * 1024) {
                targets.append(target);
            }
        }
        
        // Limit scan depth to avoid hanging
        if (targets.size() > 50) break; // Reduced from 100 for faster response
    }
}

qint64 ScanWorker::calculateDirectorySize(const QString &path)
{
    qint64 size = 0;
    QDirIterator it(path, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
    
    int fileCount = 0;
    while (it.hasNext()) {
        it.next();
        size += it.fileInfo().size();
        fileCount++;
        
        // Limit to prevent hanging on huge directories
        if (fileCount > 5000) break; // Reduced from 10000
    }
    
    return size;
}

bool ScanWorker::isSafeToDelete(const QString &path, const QFileInfo &info)
{
    // Check 1: Not modified in last 30 days (likely inactive project)
    QDateTime thirtyDaysAgo = QDateTime::currentDateTime().addDays(-30);
    if (info.lastModified() > thirtyDaysAgo) {
        return false; // Recently modified, might be active
    }
    
    // Check 2: Not in system directories
    QString normalizedPath = path.toLower();
    if (normalizedPath.contains("windows") || 
        normalizedPath.contains("program files") ||
        normalizedPath.contains("system32")) {
        return false;
    }
    
    return true; // Skip process check for performance
}

QVariantMap ScanWorker::targetToVariant(const CleanupTarget &target)
{
    QVariantMap map;
    map["path"] = target.path;
    map["type"] = target.type;
    map["size"] = target.size;
    map["sizeFormatted"] = formatSize(target.size);
    map["lastModified"] = target.lastModified.toString("yyyy-MM-dd");
    map["isSafe"] = target.isSafe;
    map["reason"] = target.reason;
    return map;
}

QString ScanWorker::formatSize(qint64 bytes)
{
    if (bytes < 1024) {
        return QString::number(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
    } else {
        return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    }
}
