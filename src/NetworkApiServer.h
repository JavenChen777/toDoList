#ifndef NETWORKAPISERVER_H
#define NETWORKAPISERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

/**
 * @brief 网络 API 服务器
 * 
 * 提供 TCP 网络接口，允许网络上的其他机器通过 HTTP POST 添加任务
 */
class NetworkApiServer : public QObject
{
    Q_OBJECT

public:
    explicit NetworkApiServer(QObject* parent = nullptr);
    ~NetworkApiServer();

    bool start(quint16 port = 8888);
    void stop();
    
    bool isListening() const;
    quint16 serverPort() const;
    QString serverAddress() const;

signals:
    void taskReceived(const QString& title, const QString& description, 
                     const QString& category, int priority, const QString& dueDate, 
                     const QStringList& tags);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpServer* m_server;
    QList<QTcpSocket*> m_clients;
    
    void processHttpRequest(QTcpSocket* socket, const QByteArray& data);
    void sendHttpResponse(QTcpSocket* socket, int statusCode, const QString& message);
    QByteArray buildHttpResponse(int statusCode, const QString& contentType, const QByteArray& body);
};

#endif // NETWORKAPISERVER_H
