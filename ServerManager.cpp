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
                              const QString &username, const QString &password, bool autoConnect)
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
    server.lastError = "";

    m_serverMap[server.id] = server;
    m_servers.append(serverToVariant(server));

    saveServers();
    emit serversChanged();

    // Auto-connect only if requested
    if (autoConnect) {
        connectToServer(server.id);
    }
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
            m_serverMap[id].lastError = ""; // Clear any previous errors

            // Update variant list
            m_servers.clear();
            for (const ServerInfo &srv : m_serverMap) {
                m_servers.append(serverToVariant(srv));
            }

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

        // Update variant list
        m_servers.clear();
        for (const ServerInfo &srv : m_serverMap) {
            m_servers.append(serverToVariant(srv));
        }

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

void ServerManager::copyToClipboard(const QString &text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setText(text);
        qDebug() << "Copied to clipboard:" << text;
    } else {
        qWarning() << "Failed to access clipboard";
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

    qDebug() << "Connection error for server" << id << ":" << error;
    m_serverMap[id].lastError = error;
    m_serverMap[id].connected = false;

    // Update variant list
    m_servers.clear();
    for (const ServerInfo &srv : m_serverMap) {
        m_servers.append(serverToVariant(srv));
    }

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
        server.lastError = "";
        
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
    , m_prevBytesSent(0)
    , m_prevBytesReceived(0)
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
    // Use full paths and LC_ALL=C to ensure consistent output format
    QString commands = "LC_ALL=C /usr/bin/top -bn1 | /usr/bin/grep 'Cpu(s)' ; "
                      "LC_ALL=C /usr/bin/free -m ; "
                      "LC_ALL=C /usr/bin/df -h / ; "
                      "/usr/bin/cat /proc/net/dev";
    
    QString output = executeRemoteCommand(commands);
    
    qDebug() << "Remote stats output for" << m_id << ":" << output.left(200);
    
    if (output.isEmpty() || output.startsWith("Error:")) {
        qDebug() << "Failed to fetch stats for" << m_id;
        return;
    }
    
    double cpu = parseCpuUsage(output);
    double ram = parseMemoryUsage(output);
    double disk = parseDiskUsage(output);
    QString netUp, netDown;
    parseNetworkUsage(output, netUp, netDown);
    
    qDebug() << "Parsed stats for" << m_id << "- CPU:" << cpu << "RAM:" << ram << "DISK:" << disk;
    
    emit statsReady(m_id, cpu, ram, disk, netUp, netDown);
}

QString RemoteWorker::executeRemoteCommand(const QString &command)
{
    QProcess process;
    
    QStringList args;
    QString program;
    
    // Use sshpass for password authentication if password is provided
    if (!m_password.isEmpty()) {
        program = "sshpass";
        args << "-p" << m_password
             << "ssh"
             << "-p" << QString::number(m_port)
             << "-o" << "StrictHostKeyChecking=no"
             << "-o" << "ConnectTimeout=5"
             << "-o" << "PreferredAuthentications=password"
             << "-o" << "PubkeyAuthentication=no"
             << m_username + "@" + m_host
             << command;
    } else {
        // Use SSH key-based authentication
        program = "ssh";
        args << "-p" << QString::number(m_port)
             << "-o" << "StrictHostKeyChecking=no"
             << "-o" << "ConnectTimeout=5"
             << m_username + "@" + m_host
             << command;
    }
    
    process.start(program, args);
    process.waitForFinished(10000); // 10 second timeout
    
    if (process.exitCode() != 0) {
        QString errorMsg = process.readAllStandardError();
        if (program == "sshpass" && errorMsg.isEmpty()) {
            errorMsg = "Authentication failed. Please check your credentials.";
        } else if (program == "sshpass" && process.error() == QProcess::FailedToStart) {
            errorMsg = "sshpass not found. Please install it: sudo apt install sshpass";
        }
        return "Error: " + errorMsg;
    }
    
    return process.readAllStandardOutput();
}

double RemoteWorker::parseCpuUsage(const QString &output)
{
    // Parse: %Cpu(s): 12.5 us, 3.2 sy, 0.0 ni, 84.3 id (or with comma: 12,5)
    QRegularExpression re("([\\d,\\.]+)\\s+id");
    QRegularExpressionMatch match = re.match(output);
    if (match.hasMatch()) {
        QString idleStr = match.captured(1);
        idleStr.replace(',', '.'); // Handle locale with comma decimal separator
        double idle = idleStr.toDouble();
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
    quint64 bytesSent = 0;
    quint64 bytesReceived = 0;
    
    // Parse /proc/net/dev output
    QStringList lines = output.split('\n');
    bool foundHeader = false;
    
    for (const QString &line : lines) {
        // Skip until we find the header
        if (line.contains("Inter-|") || line.contains("face |")) {
            foundHeader = true;
            continue;
        }
        
        if (!foundHeader) continue;
        
        // Parse interface lines
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
    
    // Calculate speed (bytes per second)
    // Note: Stats are fetched every 5 seconds, so we need to divide by 5
    if (m_prevBytesReceived > 0 && m_prevBytesSent > 0) {
        qint64 uploadSpeed = (bytesSent - m_prevBytesSent) / 5;  // Divide by 5 seconds
        qint64 downloadSpeed = (bytesReceived - m_prevBytesReceived) / 5;  // Divide by 5 seconds
        
        // Prevent negative values from counter resets
        if (uploadSpeed < 0) uploadSpeed = 0;
        if (downloadSpeed < 0) downloadSpeed = 0;
        
        // Convert to KB/s or MB/s
        double upKB = uploadSpeed / 1024.0;
        double downKB = downloadSpeed / 1024.0;
        
        if (upKB > 1024) {
            up = QString::number(upKB / 1024.0, 'f', 2) + " MB/s";
        } else {
            up = QString::number(upKB, 'f', 1) + " KB/s";
        }
        
        if (downKB > 1024) {
            down = QString::number(downKB / 1024.0, 'f', 2) + " MB/s";
        } else {
            down = QString::number(downKB, 'f', 1) + " KB/s";
        }
    } else {
        up = "0 KB/s";
        down = "0 KB/s";
    }
    
    m_prevBytesSent = bytesSent;
    m_prevBytesReceived = bytesReceived;
}
