#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include "TaskModel.h"
#include "TaskStorage.h"
#include "ApiServer.h"
#include "NetworkApiServer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief 主窗口类
 * 
 * 应用程序的主界面，管理任务列表的显示和操作
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    // 工具栏动作
    void onAddTask();
    void onViewCompleted();
    void onShowStats();
    void onShowSettings();
    
    // 任务操作
    void onTaskDoubleClicked(const QModelIndex& index);
    void onDeleteTask();
    void onToggleTaskStatus();
    
    // 搜索和过滤
    void onSearchTextChanged(const QString& text);
    void onFilterChanged();
    
    // 自动显示/隐藏
    void onAutoShowTimer();
    void onAutoHideTimer();
    
    // 系统托盘
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    
    // 设置更新
    void onSettingsChanged();
    
    // 帮助
    void onShowHelp();
    
    // API
    void onTaskReceivedFromApi(const QString& title, const QString& description, const QString& category, int priority = 3, const QString& dueDate = QString(), const QStringList& tags = QStringList());

private:
    Ui::MainWindow* ui;
    TaskModel* m_taskModel;
    TaskStorage* m_taskStorage;
    ApiServer* m_apiServer;
    NetworkApiServer* m_networkApiServer;
    
    // 定时器
    QTimer* m_autoShowTimer;     // 自动显示定时器
    QTimer* m_autoHideTimer;     // 自动隐藏定时器
    
    // 系统托盘
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    
    // 状态
    bool m_autoMode;             // 是否启用自动模式
    int m_autoShowInterval;      // 自动显示间隔（分钟）
    int m_autoHideDelay;         // 自动隐藏延迟（秒）
    bool m_manualMode;           // 用户点击后进入手动模式
    bool m_showCompleted;        // 是否显示已完成任务
    bool m_isAutoShowing;        // 标记是否正在自动显示过程中
    
    void setupUi();
    void setupConnections();
    void setupTrayIcon();
    void loadTasks();
    void saveTasks();
    void loadSettings();
    void saveSettings();
    
    void startAutoMode();
    void stopAutoMode();
    void showWindow();
    void hideWindow();
    
    void updateTaskCount();
    void applyTheme(bool darkMode);
    void applyStickyNoteStyle();
};

#endif // MAINWINDOW_H
