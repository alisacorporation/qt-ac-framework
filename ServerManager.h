#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QObject>
#include <QProcess>
#include <QVariantList>
#include <QVariantMap>
#include <QTimer>
#include <QThread>
#include <QClipboard>
#include <QGuiApplication>

struct ServerInfo {
    QString id;
    QString name;
    QString host;
    int port;
    QString username;
    QString password; // In production, use key-based auth
    bool connected;
    double cpuUsage;
    double ramUsage;
    double diskUsage;
    QString networkUp;
    QString networkDown;
    QString lastError;
};

class RemoteWorker;

class ServerManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList servers READ servers NOTIFY serversChanged)

public:
    explicit ServerManager(QObject *parent = nullptr);
    ~ServerManager();

    QVariantList servers() const { return m_servers; }

    Q_INVOKABLE void addServer(const QString &name, const QString &host, int port,
                                const QString &username, const QString &password, bool autoConnect = false);
    Q_INVOKABLE void removeServer(const QString &id);
    Q_INVOKABLE void connectToServer(const QString &id);
    Q_INVOKABLE void disconnectFromServer(const QString &id);
    Q_INVOKABLE void refreshServer(const QString &id);
    Q_INVOKABLE void refreshAll();
    Q_INVOKABLE void copyToClipboard(const QString &text);

signals:
    void serversChanged();
    void serverConnected(const QString &id);
    void serverDisconnected(const QString &id);
    void serverError(const QString &id, const QString &error);
    void serverStatsUpdated(const QString &id);

private slots:
    void onRemoteStatsReady(const QString &id, double cpu, double ram, double disk, 
                           QString netUp, QString netDown);
    void onConnectionError(const QString &id, const QString &error);

private:
    QVariantList m_servers;
    QMap<QString, ServerInfo> m_serverMap;
    QMap<QString, RemoteWorker*> m_workers;
    QMap<QString, QThread*> m_threads;
    QTimer *m_refreshTimer;

    QString generateId() const;
    QVariantMap serverToVariant(const ServerInfo &server) const;
    void saveServers();
    void loadServers();
};

// Worker class for remote SSH operations
class RemoteWorker : public QObject
{
    Q_OBJECT

public:
    explicit RemoteWorker(const QString &id, const QString &host, int port,
                         const QString &username, const QString &password,
                         QObject *parent = nullptr);

    void setCredentials(const QString &host, int port, const QString &username, const QString &password);

public slots:
    void connect();
    void disconnect();
    void fetchStats();

signals:
    void connected();
    void disconnected();
    void statsReady(QString id, double cpu, double ram, double disk, QString netUp, QString netDown);
    void error(QString id, QString errorMsg);

private:
    QString m_id;
    QString m_host;
    int m_port;
    QString m_username;
    QString m_password;
    bool m_connected;

    QString executeRemoteCommand(const QString &command);
    double parseCpuUsage(const QString &output);
    double parseMemoryUsage(const QString &output);
    double parseDiskUsage(const QString &output);
    void parseNetworkUsage(const QString &output, QString &up, QString &down);
};

#endif // SERVERMANAGER_H
