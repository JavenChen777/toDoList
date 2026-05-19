#include "ApiServer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

ApiServer::ApiServer(QObject* parent)
    : QObject(parent)
    , m_server(new QLocalServer(this))
{
    connect(m_server, &QLocalServer::newConnection, this, &ApiServer::onNewConnection);
}

ApiServer::~ApiServer()
{
    stop();
}

bool ApiServer::start(const QString& serverName)
{
    // 如果服务器已经在运行，先移除旧的
    QLocalServer::removeServer(serverName);
    
    if (!m_server->listen(serverName)) {
        qWarning() << "无法启动 API 服务器:" << m_server->errorString();
        return false;
    }
    
    qDebug() << "API 服务器已启动:" << serverName;
    return true;
}

void ApiServer::stop()
{
    // 断开所有客户端
    for (QLocalSocket* client : m_clients) {
        client->disconnectFromServer();
        client->deleteLater();
    }
    m_clients.clear();
    
    if (m_server->isListening()) {
        m_server->close();
        qDebug() << "API 服务器已停止";
    }
}

bool ApiServer::isListening() const
{
    return m_server->isListening();
}

QString ApiServer::serverName() const
{
    return m_server->fullServerName();
}

void ApiServer::onNewConnection()
{
    QLocalSocket* client = m_server->nextPendingConnection();
    
    if (!client)
        return;
    
    connect(client, &QLocalSocket::readyRead, this, &ApiServer::onReadyRead);
    connect(client, &QLocalSocket::disconnected, this, &ApiServer::onDisconnected);
    
    m_clients.append(client);
    
    qDebug() << "新客户端连接，当前连接数:" << m_clients.count();
}

void ApiServer::onReadyRead()
{
    QLocalSocket* client = qobject_cast<QLocalSocket*>(sender());
    if (!client)
        return;
    
    while (client->canReadLine()) {
        QByteArray data = client->readLine();
        QString message = QString::fromUtf8(data).trimmed();
        
        if (!message.isEmpty()) {
            processMessage(message);
        }
    }
}

void ApiServer::onDisconnected()
{
    QLocalSocket* client = qobject_cast<QLocalSocket*>(sender());
    if (!client)
        return;
    
    m_clients.removeOne(client);
    client->deleteLater();
    
    qDebug() << "客户端断开连接，当前连接数:" << m_clients.count();
}

void ApiServer::processMessage(const QString& message)
{
    qDebug() << "收到消息:" << message;
    
    // 解析 JSON 格式的消息
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误:" << parseError.errorString();
        return;
    }
    
    if (!doc.isObject()) {
        qWarning() << "无效的 JSON 格式";
        return;
    }
    
    QJsonObject obj = doc.object();
    QString title = obj["title"].toString();
    QString description = obj["description"].toString();
    QString category = obj["category"].toString();
    
    if (title.isEmpty()) {
        qWarning() << "任务标题为空";
        return;
    }
    
    emit taskReceived(title, description, category);
}
