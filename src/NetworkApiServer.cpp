#include "NetworkApiServer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkInterface>
#include <QDateTime>
#include <QPointer>
#include <QTimer>

namespace {
constexpr int MaxHeaderSize = 16 * 1024;
constexpr int MaxBodySize = 1024 * 1024;
constexpr int RequestTimeoutMs = 30 * 1000;

int contentLengthFromHeaders(const QByteArray& headers)
{
    const QList<QByteArray> lines = headers.split('\n');
    for (QByteArray line : lines) {
        line = line.trimmed();
        int colon = line.indexOf(':');
        if (colon <= 0) {
            continue;
        }

        QByteArray name = line.left(colon).trimmed();
        if (name.compare("Content-Length", Qt::CaseInsensitive) == 0) {
            bool ok = false;
            int length = line.mid(colon + 1).trimmed().toInt(&ok);
            return ok ? length : -1;
        }
    }

    return 0;
}
}

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
    
    if (!m_server->listen(QHostAddress::LocalHost, port)) {
        qWarning() << "Failed to start network API server on port" << port;
        return false;
    }
    
    qDebug() << "Network API server started on 127.0.0.1 port" << port;
    qDebug() << "API accessible at: http://" + serverAddress() + ":" + QString::number(port) + "/api/tasks";
    return true;
}

void NetworkApiServer::stop()
{
    if (m_server->isListening()) {
        m_server->close();
    }
    
    for (QTcpSocket* client : m_clients) {
        m_pendingRequests.remove(client);
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
    return "127.0.0.1";
}

void NetworkApiServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* client = m_server->nextPendingConnection();
        m_clients.append(client);
        
        connect(client, &QTcpSocket::readyRead, this, &NetworkApiServer::onReadyRead);
        connect(client, &QTcpSocket::disconnected, this, &NetworkApiServer::onDisconnected);

        QPointer<QTcpSocket> guardedClient(client);
        QTimer::singleShot(RequestTimeoutMs, this, [this, guardedClient]() {
            if (!guardedClient || !m_clients.contains(guardedClient.data())) {
                return;
            }

            sendHttpResponse(guardedClient.data(), 408, "Request timeout");
            m_pendingRequests.remove(guardedClient.data());
        });
    }
}

void NetworkApiServer::onReadyRead()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client)
        return;
    
    QByteArray& buffer = m_pendingRequests[client];
    buffer.append(client->readAll());

    int headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd == -1) {
        if (buffer.size() > MaxHeaderSize) {
            sendHttpResponse(client, 413, "Request header too large");
            m_pendingRequests.remove(client);
        }
        return;
    }

    if (headerEnd > MaxHeaderSize) {
        sendHttpResponse(client, 413, "Request header too large");
        m_pendingRequests.remove(client);
        return;
    }

    QByteArray headers = buffer.left(headerEnd);
    int contentLength = contentLengthFromHeaders(headers);
    if (contentLength < 0) {
        sendHttpResponse(client, 400, "Invalid Content-Length");
        m_pendingRequests.remove(client);
        return;
    }

    if (contentLength > MaxBodySize) {
        sendHttpResponse(client, 413, "Request body too large");
        m_pendingRequests.remove(client);
        return;
    }

    int requestSize = headerEnd + 4 + contentLength;
    if (buffer.size() < requestSize) {
        return;
    }

    QByteArray request = buffer.left(requestSize);
    m_pendingRequests.remove(client);
    processHttpRequest(client, request);
}

void NetworkApiServer::onDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        m_clients.removeOne(client);
        m_pendingRequests.remove(client);
        client->deleteLater();
    }
}

void NetworkApiServer::processHttpRequest(QTcpSocket* socket, const QByteArray& data)
{
    int bodyStart = data.indexOf("\r\n\r\n");
    if (bodyStart == -1) {
        sendHttpResponse(socket, 400, "No JSON data");
        return;
    }

    QString headerText = QString::fromLatin1(data.left(bodyStart));
    QStringList lines = headerText.split("\r\n");
    
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
    if (method != "POST" || path != "/api/tasks") {
        sendHttpResponse(socket, 405, "Method Not Allowed");
        return;
    }
    
    QByteArray jsonData = data.mid(bodyStart + 4);
    
    // 解析 JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    
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
        case 408: statusText = "Request Timeout"; break;
        case 405: statusText = "Method Not Allowed"; break;
        case 413: statusText = "Payload Too Large"; break;
        case 500: statusText = "Internal Server Error"; break;
        default: statusText = "Unknown"; break;
    }
    
    QByteArray response;
    response += "HTTP/1.1 " + QByteArray::number(statusCode) + " " + statusText.toUtf8() + "\r\n";
    response += "Content-Type: " + contentType.toUtf8() + "; charset=utf-8\r\n";
    response += "Content-Length: " + QByteArray::number(body.length()) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += body;
    
    return response;
}
