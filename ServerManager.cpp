#include "ServerManager.h"
#include <QUuid>
#include <QDebug>
#include <QSettings>
#include <QRegularExpression>

ServerManager::ServerManager(QObject *parent)
    : QObject(parent)
{
    loadServers();
    
    // Note: Each RemoteWorker has its own timer for stats updates
    // No need for a global refresh timer
}

ServerManager::~ServerManager()
{
    qDebug() << "ServerManager destructor - cleaning up" << m_workers.size() << "workers";
    
    // Disconnect all servers properly
    QStringList serverIds = m_workers.keys();
    for (const QString &id : serverIds) {
        qDebug() << "Disconnecting server:" << id;
        disconnectFromServer(id);
    }
    
    qDebug() << "ServerManager cleanup complete";
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

void ServerManager::updateServer(const QString &id, const QString &name, const QString &host, int port,
                                 const QString &username, const QString &password)
{
    if (!m_serverMap.contains(id)) return;
    
    // Disconnect if currently connected
    bool wasConnected = m_serverMap[id].connected;
    if (wasConnected) {
        disconnectFromServer(id);
    }
    
    // Update server info
    m_serverMap[id].name = name;
    m_serverMap[id].host = host;
    m_serverMap[id].port = port;
    m_serverMap[id].username = username;
    m_serverMap[id].password = password;
    
    // Rebuild variant list
    m_servers.clear();
    for (const ServerInfo &server : m_serverMap) {
        m_servers.append(serverToVariant(server));
    }
    
    saveServers();
    emit serversChanged();
    
    // Reconnect if it was connected before
    if (wasConnected) {
        connectToServer(id);
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
    map["password"] = server.password;  // Include password for edit functionality
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
    , m_statsTimer(nullptr)
{
    // Create timer for periodic stats updates
    m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(5000); // 5 seconds
    QObject::connect(m_statsTimer, &QTimer::timeout, this, &RemoteWorker::fetchStats);
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
    qDebug() << "RemoteWorker::connect() called for" << m_id << "(" << m_host << ":" << m_port << ")";
    
    // Check if this is localhost - if so, skip remote monitoring
    if (isLocalhost()) {
        qDebug() << "Skipping remote monitoring for" << m_id << "- detected as localhost";
        emit error(m_id, "Cannot monitor localhost remotely. Use Dashboard instead.");
        return;
    }
    
    qDebug() << "Testing SSH connection to" << m_host;
    // Test connection with simple command
    QString output = executeRemoteCommand("echo 'connected'");
    qDebug() << "Connection test output:" << output.left(100);
    
    if (output.contains("connected")) {
        qDebug() << "Connection successful for" << m_id;
        m_connected = true;
        emit connected();
        fetchStats(); // Initial fetch
        m_statsTimer->start(); // Start periodic updates
        qDebug() << "Stats timer started for" << m_id;
    } else {
        qDebug() << "Connection failed for" << m_id;
        emit error(m_id, "Connection failed: " + output);
    }
}

void RemoteWorker::disconnect()
{
    qDebug() << "RemoteWorker::disconnect() called for" << m_id;
    m_connected = false;
    if (m_statsTimer) {
        m_statsTimer->stop();
        qDebug() << "Stats timer stopped for" << m_id;
    }
    
    // Close SSH control socket
    QString controlPath = QString("/tmp/ssh-control-%1-%2-%3")
        .arg(m_host)
        .arg(m_port)
        .arg(m_username);
    
    QProcess closeProcess;
    QStringList args;
    args << "-p" << QString::number(m_port)
         << "-O" << "exit"
         << "-o" << "ControlPath=" + controlPath
         << m_username + "@" + m_host;
    
    closeProcess.start("ssh", args);
    closeProcess.waitForFinished(2000);
    qDebug() << "SSH control socket closed for" << m_id;
    
    emit disconnected();
    qDebug() << "Disconnect complete for" << m_id;
}

void RemoteWorker::fetchStats()
{
    if (!m_connected) {
        qDebug() << "fetchStats called but not connected for" << m_id;
        return;
    }
    
    qDebug() << "fetchStats() called for" << m_id << "- fetching...";
    
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
    qDebug() << "Timer active for" << m_id << ":" << (m_statsTimer ? m_statsTimer->isActive() : false);
    
    emit statsReady(m_id, cpu, ram, disk, netUp, netDown);
    qDebug() << "Stats emitted for" << m_id;
}

QString RemoteWorker::executeRemoteCommand(const QString &command)
{
    QProcess process;
    
    QStringList args;
    QString program;
    
    // Create control path for SSH connection multiplexing
    QString controlPath = QString("/tmp/ssh-control-%1-%2-%3")
        .arg(m_host)
        .arg(m_port)
        .arg(m_username);
    
    // Use sshpass for password authentication if password is provided
    if (!m_password.isEmpty()) {
        program = "sshpass";
        args << "-p" << m_password
             << "ssh"
             << "-p" << QString::number(m_port)
             << "-o" << "StrictHostKeyChecking=no"
             << "-o" << "ConnectTimeout=10"
             << "-o" << "ServerAliveInterval=10"
             << "-o" << "ServerAliveCountMax=3"
             << "-o" << "ControlMaster=auto"
             << "-o" << "ControlPath=" + controlPath
             << "-o" << "ControlPersist=60"
             << "-o" << "PreferredAuthentications=password"
             << "-o" << "PubkeyAuthentication=no"
             << m_username + "@" + m_host
             << command;
    } else {
        // Use SSH key-based authentication with connection multiplexing
        program = "ssh";
        args << "-p" << QString::number(m_port)
             << "-o" << "StrictHostKeyChecking=no"
             << "-o" << "ConnectTimeout=10"
             << "-o" << "ServerAliveInterval=10"
             << "-o" << "ServerAliveCountMax=3"
             << "-o" << "ControlMaster=auto"
             << "-o" << "ControlPath=" + controlPath
             << "-o" << "ControlPersist=60"
             << m_username + "@" + m_host
             << command;
    }
    
    qDebug() << "Executing SSH command for" << m_id << ":" << program << args.join(" ").replace(m_password, "***");
    
    process.start(program, args);
    bool finished = process.waitForFinished(15000); // 15 second timeout
    
    if (!finished) {
        process.kill();
        qDebug() << "SSH command timed out for" << m_id;
        return "Error: Connection timed out after 15 seconds";
    }
    
    if (process.exitCode() != 0) {
        QString errorMsg = process.readAllStandardError();
        qDebug() << "SSH error for" << m_id << "- Exit code:" << process.exitCode() << "Error:" << errorMsg;
        
        if (program == "sshpass" && errorMsg.isEmpty()) {
            errorMsg = "Authentication failed. Please check your credentials.";
        } else if (program == "sshpass" && process.error() == QProcess::FailedToStart) {
            errorMsg = "sshpass not found. Please install it: sudo apt install sshpass";
        } else if (errorMsg.contains("Connection timed out")) {
            errorMsg = "Connection timed out. Check firewall/network or increase timeout.";
        }
        return "Error: " + errorMsg;
    }
    
    QString output = process.readAllStandardOutput();
    qDebug() << "SSH command successful for" << m_id << "- Output length:" << output.length();
    return output;
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
    if (m_prevBytesReceived > 0 || m_prevBytesSent > 0) {
        // We have previous values, calculate speed
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
        
        qDebug() << "Network for" << m_id << "- Up:" << up << "Down:" << down 
                 << "(bytes:" << bytesSent << bytesReceived << "prev:" << m_prevBytesSent << m_prevBytesReceived << ")";
    } else {
        // First fetch, just initialize
        up = "0 KB/s";
        down = "0 KB/s";
        qDebug() << "Network for" << m_id << "- First fetch, initializing counters";
    }
    
    m_prevBytesSent = bytesSent;
    m_prevBytesReceived = bytesReceived;
}

bool RemoteWorker::isLocalhost() const
{
    // Check common localhost identifiers
    if (m_host == "localhost" || 
        m_host == "127.0.0.1" || 
        m_host == "::1" ||
        m_host == "0.0.0.0") {
        return true;
    }
    
    // Check if it's the local machine's hostname
    QProcess process;
    process.start("hostname", QStringList());
    process.waitForFinished(1000);
    QString localHostname = process.readAllStandardOutput().trimmed();
    
    if (m_host == localHostname) {
        return true;
    }
    
    // Check local IP addresses
    process.start("hostname", QStringList() << "-I");
    process.waitForFinished(1000);
    QString localIPs = process.readAllStandardOutput().trimmed();
    QStringList ipList = localIPs.split(' ', Qt::SkipEmptyParts);
    
    for (const QString &ip : ipList) {
        if (m_host == ip.trimmed()) {
            return true;
        }
    }
    
    return false;
}
