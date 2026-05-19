#include "NetworkApiServer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkInterface>
#include <QDateTime>

NetworkApiServer::NetworkApiServer(QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &NetworkApiServer::onNewConnection);
}

NetworkApiServer::~NetworkApiServer()
{
    stop();
}

bool NetworkApiServer::start(quint16 port)
{
    if (m_server->isListening()) {
        return true;
    }
    
    if (!m_server->listen(QHostAddress::Any, port)) {
        qWarning() << "Failed to start network API server on port" << port;
        return false;
    }
    
    qDebug() << "Network API server started on port" << port;
    qDebug() << "API accessible at: http://" + serverAddress() + ":" + QString::number(port) + "/api/tasks";
    return true;
}

void NetworkApiServer::stop()
{
    if (m_server->isListening()) {
        m_server->close();
    }
    
    for (QTcpSocket* client : m_clients) {
        client->disconnectFromHost();
        client->deleteLater();
    }
    m_clients.clear();
}

bool NetworkApiServer::isListening() const
{
    return m_server->isListening();
}

quint16 NetworkApiServer::serverPort() const
{
    return m_server->serverPort();
}

QString NetworkApiServer::serverAddress() const
{
    // 获取本机 IP 地址
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    for (const QHostAddress& address : addresses) {
        if (address != QHostAddress::LocalHost && 
            address.toIPv4Address() && 
            !address.isLoopback()) {
            return address.toString();
        }
    }
    return "localhost";
}

void NetworkApiServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* client = m_server->nextPendingConnection();
        m_clients.append(client);
        
        connect(client, &QTcpSocket::readyRead, this, &NetworkApiServer::onReadyRead);
        connect(client, &QTcpSocket::disconnected, this, &NetworkApiServer::onDisconnected);
    }
}

void NetworkApiServer::onReadyRead()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client)
        return;
    
    QByteArray data = client->readAll();
    processHttpRequest(client, data);
}

void NetworkApiServer::onDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        m_clients.removeOne(client);
        client->deleteLater();
    }
}

void NetworkApiServer::processHttpRequest(QTcpSocket* socket, const QByteArray& data)
{
    QString request = QString::fromUtf8(data);
    QStringList lines = request.split("\r\n");
    
    if (lines.isEmpty()) {
        sendHttpResponse(socket, 400, "Bad Request");
        return;
    }
    
    // 解析请求行
    QStringList requestLine = lines[0].split(" ");
    if (requestLine.size() < 3) {
        sendHttpResponse(socket, 400, "Bad Request");
        return;
    }
    
    QString method = requestLine[0];
    QString path = requestLine[1];
    
    // 只接受 POST 请求到 /api/tasks
    if (method != "POST" || !path.startsWith("/api/tasks")) {
        sendHttpResponse(socket, 405, "Method Not Allowed");
        return;
    }
    
    // 查找 JSON 数据（在空行之后）
    int bodyStart = request.indexOf("\r\n\r\n");
    if (bodyStart == -1) {
        sendHttpResponse(socket, 400, "No JSON data");
        return;
    }
    
    QString jsonData = request.mid(bodyStart + 4);
    
    // 解析 JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        sendHttpResponse(socket, 400, "Invalid JSON: " + parseError.errorString());
        return;
    }
    
    if (!doc.isObject()) {
        sendHttpResponse(socket, 400, "JSON must be an object");
        return;
    }
    
    QJsonObject obj = doc.object();
    
    // 提取任务信息
    QString title = obj.value("title").toString();
    if (title.isEmpty()) {
        sendHttpResponse(socket, 400, "Missing required field: title");
        return;
    }
    
    QString description = obj.value("description").toString();
    QString category = obj.value("category").toString();
    int priority = obj.value("priority").toInt(3);
    QString dueDate = obj.value("dueDate").toString();
    
    QStringList tags;
    QJsonArray tagsArray = obj.value("tags").toArray();
    for (const QJsonValue& tag : tagsArray) {
        tags.append(tag.toString());
    }
    
    // 发送信号
    emit taskReceived(title, description, category, priority, dueDate, tags);
    
    // 返回成功响应
    QJsonObject response;
    response["success"] = true;
    response["message"] = "Task added successfully";
    response["task"] = obj;
    
    QByteArray responseBody = QJsonDocument(response).toJson(QJsonDocument::Compact);
    QByteArray httpResponse = buildHttpResponse(200, "application/json", responseBody);
    
    socket->write(httpResponse);
    socket->flush();
    socket->disconnectFromHost();
}

void NetworkApiServer::sendHttpResponse(QTcpSocket* socket, int statusCode, const QString& message)
{
    QJsonObject response;
    response["success"] = false;
    response["error"] = message;
    
    QByteArray body = QJsonDocument(response).toJson(QJsonDocument::Compact);
    QByteArray httpResponse = buildHttpResponse(statusCode, "application/json", body);
    
    socket->write(httpResponse);
    socket->flush();
    socket->disconnectFromHost();
}

QByteArray NetworkApiServer::buildHttpResponse(int statusCode, const QString& contentType, const QByteArray& body)
{
    QString statusText;
    switch (statusCode) {
        case 200: statusText = "OK"; break;
        case 400: statusText = "Bad Request"; break;
        case 405: statusText = "Method Not Allowed"; break;
        case 500: statusText = "Internal Server Error"; break;
        default: statusText = "Unknown"; break;
    }
    
    QByteArray response;
    response += "HTTP/1.1 " + QByteArray::number(statusCode) + " " + statusText.toUtf8() + "\r\n";
    response += "Content-Type: " + contentType.toUtf8() + "; charset=utf-8\r\n";
    response += "Content-Length: " + QByteArray::number(body.length()) + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += body;
    
    return response;
}
