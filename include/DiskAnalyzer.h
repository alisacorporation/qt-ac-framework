#ifndef DISKANALYZER_H
#define DISKANALYZER_H

#include <QObject>
#include <QDir>
#include <QFileInfo>
#include <QStorageInfo>
#include <QVariantList>
#include <QVariantMap>
#include <QDateTime>
#include <QThread>
#include <QMutex>

class ScanWorker;

class DiskAnalyzer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList cleanupTargets READ cleanupTargets NOTIFY targetsUpdated)
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(QString scanProgress READ scanProgress NOTIFY scanProgressChanged)

public:
    explicit DiskAnalyzer(QObject *parent = nullptr);

    QVariantList cleanupTargets() const { return m_cleanupTargets; }
    bool isScanning() const { return m_isScanning; }
    QString scanProgress() const { return m_scanProgress; }

    Q_INVOKABLE void startScan(const QString &rootPath = "");
    Q_INVOKABLE void deleteTarget(int index);
    Q_INVOKABLE QString formatSize(qint64 bytes);

signals:
    void targetsUpdated();
    void scanningChanged();
    void scanProgressChanged();
    void scanComplete(qint64 totalSavings);

private slots:
    void onScanFinished(QVariantList targets, qint64 totalSavings);
    void onProgressUpdate(const QString &progress);

private:
    struct CleanupTarget {
        QString path;
        QString type;
        qint64 size;
        QDateTime lastModified;
        bool isSafe;
        QString reason;
    };

    QVariantList m_cleanupTargets;
    bool m_isScanning = false;
    QString m_scanProgress;
    QThread *m_workerThread = nullptr;
    ScanWorker *m_worker = nullptr;

    QVariantMap targetToVariant(const CleanupTarget &target);
};

// Worker class for background scanning
class ScanWorker : public QObject
{
    Q_OBJECT

public:
    explicit ScanWorker(QObject *parent = nullptr);
    void setScanPath(const QString &path);

public slots:
    void doScan();

signals:
    void scanFinished(QVariantList targets, qint64 totalSavings);
    void progressUpdate(const QString &progress);

private:
    QString m_scanPath;

    struct CleanupTarget {
        QString path;
        QString type;
        qint64 size;
        QDateTime lastModified;
        bool isSafe;
        QString reason;
    };

    void scanDirectory(const QString &path, QList<CleanupTarget> &targets);
    qint64 calculateDirectorySize(const QString &path);
    bool isSafeToDelete(const QString &path, const QFileInfo &info);
    QVariantMap targetToVariant(const CleanupTarget &target);
    QString formatSize(qint64 bytes);
};

#endif // DISKANALYZER_H
