#include "MainWindow.h"
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

    return app.exec();
}
