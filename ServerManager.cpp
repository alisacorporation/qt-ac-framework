#include "ServerManager.h"
#include <QUuid>
#include <QDebug>
#include <QSettings>
#include <QRegularExpression>

ServerManager::ServerManager(QObject *parent)
    : QObject(parent)
{
    loadServers();
    
    // Auto-refresh every 5 seconds
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &ServerManager::refreshAll);
    m_refreshTimer->start(5000);
}

ServerManager::~ServerManager()
{
    // Clean up workers and threads
    for (auto it = m_workers.begin(); it != m_workers.end(); ++it) {
        if (m_threads.contains(it.key())) {
            m_threads[it.key()]->quit();
            m_threads[it.key()]->wait();
        }
    }
}

void ServerManager::addServer(const QString &name, const QString &host, int port,
                              const QString &username, const QString &password)
{
    ServerInfo server;
    server.id = generateId();
    server.name = name;
    server.host = host;
    server.port = port;
    server.username = username;
    server.password = password;
    server.connected = false;
    server.cpuUsage = 0.0;
    server.ramUsage = 0.0;
    server.diskUsage = 0.0;
    server.networkUp = "0 KB/s";
    server.networkDown = "0 KB/s";
    
    m_serverMap[server.id] = server;
    m_servers.append(serverToVariant(server));
    
    saveServers();
    emit serversChanged();
    
    // Auto-connect
    connectToServer(server.id);
}

void ServerManager::removeServer(const QString &id)
{
    if (!m_serverMap.contains(id)) return;
    
    disconnectFromServer(id);
    m_serverMap.remove(id);
    
    // Rebuild variant list
    m_servers.clear();
    for (const ServerInfo &server : m_serverMap) {
        m_servers.append(serverToVariant(server));
    }
    
    saveServers();
    emit serversChanged();
}

void ServerManager::connectToServer(const QString &id)
{
    if (!m_serverMap.contains(id)) return;
    
    ServerInfo &server = m_serverMap[id];
    
    // Create worker thread
    QThread *thread = new QThread(this);
    RemoteWorker *worker = new RemoteWorker(id, server.host, server.port,
                                           server.username, server.password);
    worker->moveToThread(thread);
    
    // Connect signals
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &RemoteWorker::statsReady, this, &ServerManager::onRemoteStatsReady);
    connect(worker, &RemoteWorker::error, this, &ServerManager::onConnectionError);
    connect(worker, &RemoteWorker::connected, this, [this, id]() {
        if (m_serverMap.contains(id)) {
            m_serverMap[id].connected = true;
            emit serverConnected(id);
            emit serversChanged();
        }
    });
    
    m_workers[id] = worker;
    m_threads[id] = thread;
    
    thread->start();
    QMetaObject::invokeMethod(worker, "connect", Qt::QueuedConnection);
}

void ServerManager::disconnectFromServer(const QString &id)
{
    if (!m_workers.contains(id)) return;
    
    QMetaObject::invokeMethod(m_workers[id], "disconnect", Qt::QueuedConnection);
    
    if (m_threads.contains(id)) {
        m_threads[id]->quit();
        m_threads[id]->wait();
        m_threads[id]->deleteLater();
        m_threads.remove(id);
    }
    
    m_workers.remove(id);
    
    if (m_serverMap.contains(id)) {
        m_serverMap[id].connected = false;
        emit serverDisconnected(id);
        emit serversChanged();
    }
}

void ServerManager::refreshServer(const QString &id)
{
    if (!m_workers.contains(id)) return;
    QMetaObject::invokeMethod(m_workers[id], "fetchStats", Qt::QueuedConnection);
}

void ServerManager::refreshAll()
{
    for (const QString &id : m_workers.keys()) {
        refreshServer(id);
    }
}

void ServerManager::onRemoteStatsReady(const QString &id, double cpu, double ram, double disk,
                                      QString netUp, QString netDown)
{
    if (!m_serverMap.contains(id)) return;
    
    ServerInfo &server = m_serverMap[id];
    server.cpuUsage = cpu;
    server.ramUsage = ram;
    server.diskUsage = disk;
    server.networkUp = netUp;
    server.networkDown = netDown;
    server.lastError = "";
    
    // Update variant list
    m_servers.clear();
    for (const ServerInfo &srv : m_serverMap) {
        m_servers.append(serverToVariant(srv));
    }
    
    emit serverStatsUpdated(id);
    emit serversChanged();
}

void ServerManager::onConnectionError(const QString &id, const QString &error)
{
    if (!m_serverMap.contains(id)) return;
    
    m_serverMap[id].lastError = error;
    m_serverMap[id].connected = false;
    
    emit serverError(id, error);
    emit serversChanged();
}

QString ServerManager::generateId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QVariantMap ServerManager::serverToVariant(const ServerInfo &server) const
{
    QVariantMap map;
    map["id"] = server.id;
    map["name"] = server.name;
    map["host"] = server.host;
    map["port"] = server.port;
    map["username"] = server.username;
    map["connected"] = server.connected;
    map["cpuUsage"] = server.cpuUsage;
    map["ramUsage"] = server.ramUsage;
    map["diskUsage"] = server.diskUsage;
    map["networkUp"] = server.networkUp;
    map["networkDown"] = server.networkDown;
    map["lastError"] = server.lastError;
    return map;
}

void ServerManager::saveServers()
{
    QSettings settings("AlisaCorp", "SystemMonitor");
    settings.beginWriteArray("servers");
    int index = 0;
    for (const ServerInfo &server : m_serverMap) {
        settings.setArrayIndex(index++);
        settings.setValue("id", server.id);
        settings.setValue("name", server.name);
        settings.setValue("host", server.host);
        settings.setValue("port", server.port);
        settings.setValue("username", server.username);
        settings.setValue("password", server.password); // Encrypt in production!
    }
    settings.endArray();
}

void ServerManager::loadServers()
{
    QSettings settings("AlisaCorp", "SystemMonitor");
    int size = settings.beginReadArray("servers");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        ServerInfo server;
        server.id = settings.value("id").toString();
        server.name = settings.value("name").toString();
        server.host = settings.value("host").toString();
        server.port = settings.value("port").toInt();
        server.username = settings.value("username").toString();
        server.password = settings.value("password").toString();
        server.connected = false;
        server.cpuUsage = 0.0;
        server.ramUsage = 0.0;
        server.diskUsage = 0.0;
        server.networkUp = "0 KB/s";
        server.networkDown = "0 KB/s";
        
        m_serverMap[server.id] = server;
        m_servers.append(serverToVariant(server));
    }
    settings.endArray();
}

// ============ RemoteWorker Implementation ============

RemoteWorker::RemoteWorker(const QString &id, const QString &host, int port,
                          const QString &username, const QString &password,
                          QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_host(host)
    , m_port(port)
    , m_username(username)
    , m_password(password)
    , m_connected(false)
{
}

void RemoteWorker::setCredentials(const QString &host, int port, const QString &username, const QString &password)
{
    m_host = host;
    m_port = port;
    m_username = username;
    m_password = password;
}

void RemoteWorker::connect()
{
    // Test connection with simple command
    QString output = executeRemoteCommand("echo 'connected'");
    if (output.contains("connected")) {
        m_connected = true;
        emit connected();
        fetchStats(); // Initial fetch
    } else {
        emit error(m_id, "Connection failed: " + output);
    }
}

void RemoteWorker::disconnect()
{
    m_connected = false;
    emit disconnected();
}

void RemoteWorker::fetchStats()
{
    if (!m_connected) return;
    
    // Fetch all stats in one SSH session for efficiency
    QString commands = "top -bn1 | grep 'Cpu(s)' ; "
                      "free -m ; "
                      "df -h / ; "
                      "cat /proc/net/dev";
    
    QString output = executeRemoteCommand(commands);
    
    if (output.isEmpty()) {
        emit error(m_id, "Failed to fetch stats");
        return;
    }
    
    double cpu = parseCpuUsage(output);
    double ram = parseMemoryUsage(output);
    double disk = parseDiskUsage(output);
    QString netUp, netDown;
    parseNetworkUsage(output, netUp, netDown);
    
    emit statsReady(m_id, cpu, ram, disk, netUp, netDown);
}

QString RemoteWorker::executeRemoteCommand(const QString &command)
{
    QProcess process;
    
    // Use ssh command (assumes ssh is in PATH and key-based auth is set up)
    // For password auth, use sshpass (Linux) or plink (Windows)
    QStringList args;
    args << "-p" << QString::number(m_port)
         << "-o" << "StrictHostKeyChecking=no"
         << "-o" << "ConnectTimeout=5"
         << m_username + "@" + m_host
         << command;
    
    process.start("ssh", args);
    process.waitForFinished(10000); // 10 second timeout
    
    if (process.exitCode() != 0) {
        return "Error: " + process.readAllStandardError();
    }
    
    return process.readAllStandardOutput();
}

double RemoteWorker::parseCpuUsage(const QString &output)
{
    // Parse: %Cpu(s): 12.5 us, 3.2 sy, 0.0 ni, 84.3 id
    QRegularExpression re("(\\d+\\.\\d+)\\s+id");
    QRegularExpressionMatch match = re.match(output);
    if (match.hasMatch()) {
        double idle = match.captured(1).toDouble();
        return 100.0 - idle;
    }
    return 0.0;
}

double RemoteWorker::parseMemoryUsage(const QString &output)
{
    // Parse free -m output
    QStringList lines = output.split('\n');
    for (const QString &line : lines) {
        if (line.startsWith("Mem:")) {
            QStringList parts = line.simplified().split(' ');
            if (parts.size() >= 3) {
                double total = parts[1].toDouble();
                double used = parts[2].toDouble();
                if (total > 0) {
                    return (used / total) * 100.0;
                }
            }
        }
    }
    return 0.0;
}

double RemoteWorker::parseDiskUsage(const QString &output)
{
    // Parse df -h output
    QStringList lines = output.split('\n');
    for (const QString &line : lines) {
        if (line.contains("/ ") || line.endsWith("/")) {
            QStringList parts = line.simplified().split(' ');
            if (parts.size() >= 5) {
                QString percent = parts[4];
                percent.remove('%');
                return percent.toDouble();
            }
        }
    }
    return 0.0;
}

void RemoteWorker::parseNetworkUsage(const QString &output, QString &up, QString &down)
{
    // Simple placeholder - would need previous values for speed calculation
    up = "0 KB/s";
    down = "0 KB/s";
    
    // Parse /proc/net/dev for byte counts (implementation similar to local monitoring)
    // For now, just return placeholder values
}
