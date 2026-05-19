#include "MainWindow.h"
#include "ApiServer.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    QCoreApplication::setOrganizationName("TodoList");
    QCoreApplication::setApplicationName("TodoListApp");
    QCoreApplication::setApplicationVersion("1.0.0");

    // 加载翻译
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "TodoList_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    // 创建主窗口
    MainWindow mainWindow;
    mainWindow.show();

    // 启动 API 服务器
    ApiServer* apiServer = new ApiServer(&app);
    if (apiServer->start("TodoListServer")) {
        // 连接 API 服务器信号到主窗口
        QObject::connect(apiServer, &ApiServer::taskReceived, 
            [&mainWindow](const QString& title, const QString& description, const QString& category) {
                // 这里需要在 MainWindow 中添加一个公共方法来添加任务
                // 或者通过信号槽机制
                qDebug() << "通过 API 接收到新任务:" << title;
                // TODO: 添加任务到主窗口
            });
    }

    return app.exec();
}
