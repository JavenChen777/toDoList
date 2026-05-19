#ifndef APISERVER_H
#define APISERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>

/**
 * @brief API 服务器
 * 
 * 提供本地 IPC 接口，允许其他程序写入任务
 */
class ApiServer : public QObject
{
    Q_OBJECT

public:
    explicit ApiServer(QObject* parent = nullptr);
    ~ApiServer();

    bool start(const QString& serverName = "TodoListServer");
    void stop();
    
    bool isListening() const;
    QString serverName() const;

signals:
    void taskReceived(const QString& title, const QString& description, const QString& category);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QLocalServer* m_server;
    QList<QLocalSocket*> m_clients;
    
    void processMessage(const QString& message);
};

#endif // APISERVER_H
