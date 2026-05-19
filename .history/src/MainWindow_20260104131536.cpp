#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "AddTaskDialog.h"
#include "SettingsDialog.h"
#include "StatsDialog.h"
#include "HelpDialog.h"
#include "TaskDelegate.h"
#include "TaskItem.h"
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QMenu>
#include <QStyle>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <algorithm>

// 简单的日志函数
void logToFile(const QString& message) {
    QFile file("C:/AI/toDoList/debug.log");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " - " << message << "\n";
        file.close();
    }
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_taskModel(new TaskModel(this))
    , m_taskStorage(new TaskStorage())
    , m_apiServer(new ApiServer(this))
    , m_networkApiServer(new NetworkApiServer(this))
    , m_autoShowTimer(new QTimer(this))
    , m_autoHideTimer(new QTimer(this))
    , m_trayIcon(new QSystemTrayIcon(this))
    , m_autoMode(false)
    , m_autoShowInterval(30)
    , m_autoHideDelay(5)
    , m_manualMode(false)
    , m_showCompleted(false)
    , m_isAutoShowing(false)
{
    ui->setupUi(this);
    setupUi();
    setupConnections();
    setupTrayIcon();
    loadSettings();
    loadTasks();
    updateTaskCount();
    
    // 启动 API 服务器
    if (!m_apiServer->start()) {
        qWarning() << "Failed to start API server";
    }
    
    // 启动网络 API 服务器
    if (!m_networkApiServer->start(8888)) {
        qWarning() << "Failed to start network API server";
    } else {
        qDebug() << "Network API server started successfully";
        qDebug() << "You can access the API from other machines at:";
        qDebug() << "http://" + m_networkApiServer->serverAddress() + ":8888/api/tasks";
    }
    
    // 安装应用级事件过滤器以捕获所有子控件的事件
    qApp->installEventFilter(this);
    
    // 检查是否应该启动时最小化
    QSettings settings("TodoList", "TodoListApp");
    bool startMinimized = settings.value("startMinimized", false).toBool();
    if (startMinimized) {
        QTimer::singleShot(100, this, [this]() {
            hide();
            m_trayIcon->show();
        });
    }
}

MainWindow::~MainWindow()
{
    saveTasks();
    saveSettings();
    delete ui;
}

void MainWindow::setupUi()
{
    // 设置窗口标题和图标
    setWindowTitle("✨ 待办便签");
    
    // 应用贴纸风格样式
    applyStickyNoteStyle();
    
    // 隐藏自定义标题栏（使用系统标题栏）
    ui->titleBar->setVisible(false);
    
    // 设置列表视图
    ui->taskListView->setModel(m_taskModel);
    ui->taskListView->setItemDelegate(new TaskDelegate(this));
    ui->taskListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->taskListView->setSelectionMode(QAbstractItemView::ExtendedSelection); // 支持多选
    
    // 设置工具栏图标大小
    ui->mainToolBar->setIconSize(QSize(24, 24));
    
    // 默认不显示已完成任务
    m_taskModel->clearFilter();
}

void MainWindow::setupConnections()
{
    // 工具栏动作
    connect(ui->actionAddTask, &QAction::triggered, this, &MainWindow::onAddTask);
    connect(ui->actionViewCompleted, &QAction::triggered, this, &MainWindow::onViewCompleted);
    connect(ui->actionStats, &QAction::triggered, this, &MainWindow::onShowStats);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::onShowSettings);
    connect(ui->actionDeleteTask, &QAction::triggered, this, &MainWindow::onDeleteTask);
    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::onShowHelp);
    
    // 列表视图
    connect(ui->taskListView, &QListView::doubleClicked, this, &MainWindow::onTaskDoubleClicked);
    
    // 搜索框
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    
    // 过滤按钮
    connect(ui->showAllButton, &QPushButton::clicked, this, &MainWindow::onFilterChanged);
    connect(ui->showTodoButton, &QPushButton::clicked, this, &MainWindow::onFilterChanged);
    connect(ui->showCompletedButton, &QPushButton::clicked, this, &MainWindow::onFilterChanged);
    
    // 定时器
    connect(m_autoShowTimer, &QTimer::timeout, this, &MainWindow::onAutoShowTimer);
    connect(m_autoHideTimer, &QTimer::timeout, this, &MainWindow::onAutoHideTimer);
    
    // 模型信号
    connect(m_taskModel, &TaskModel::taskAdded, this, &MainWindow::updateTaskCount);
    connect(m_taskModel, &TaskModel::taskRemoved, this, &MainWindow::updateTaskCount);
    connect(m_taskModel, &TaskModel::taskUpdated, this, &MainWindow::updateTaskCount);
    
    // API 服务器信号
    connect(m_apiServer, &ApiServer::taskReceived, this, 
        [this](const QString& title, const QString& desc, const QString& cat) {
            onTaskReceivedFromApi(title, desc, cat, 3, QString(), QStringList());
        });
    connect(m_networkApiServer, &NetworkApiServer::taskReceived, this, &MainWindow::onTaskReceivedFromApi);
}

void MainWindow::setupTrayIcon()
{
    // 使用应用程序图标
    QIcon icon = QIcon(":/icon.ico");
    if (icon.isNull()) {
        icon = windowIcon();
    }
    if (icon.isNull()) {
        // 使用系统样式的标准图标
        icon = style()->standardIcon(QStyle::SP_FileDialogListView);
    }
    m_trayIcon->setIcon(icon);
    m_trayIcon->setToolTip("待办事项管理");
    
    m_trayMenu = new QMenu(this);
    m_trayMenu->addAction("显示主窗口", this, &MainWindow::showWindow);
    m_trayMenu->addAction("新建任务", this, &MainWindow::onAddTask);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction("退出", qApp, &QApplication::quit);
    
    m_trayIcon->setContextMenu(m_trayMenu);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
    
    m_trayIcon->show();
}

void MainWindow::loadTasks()
{
    QList<TaskItem> tasks = m_taskModel->tasks(); // 保留当前数据
    
    if (!m_taskStorage->loadTasksSafe(tasks)) {
        // 加载失败，尝试从备份恢复
        QString errorMsg = m_taskStorage->lastError();
        
        if (m_taskStorage->hasBackup()) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, 
                "加载错误",
                QString("加载任务文件失败: %1\n\n检测到备份文件，是否尝试从备份恢复？").arg(errorMsg),
                QMessageBox::Yes | QMessageBox::No);
            
            if (reply == QMessageBox::Yes) {
                QList<TaskItem> backupTasks = m_taskStorage->loadFromBackup();
                if (!m_taskStorage->lastError().isEmpty()) {
                    QMessageBox::critical(this, "恢复失败",
                        QString("从备份恢复失败: %1\n\n当前数据未被修改。").arg(m_taskStorage->lastError()));
                } else {
                    // 成功从备份加载
                    tasks = backupTasks;
                    m_taskModel->setTasks(tasks);
                    
                    // 立即保存，修复损坏的文件
                    if (m_taskStorage->saveTasks(tasks)) {
                        QMessageBox::information(this, "恢复成功",
                            QString("已成功从备份恢复 %1 个任务。").arg(tasks.size()));
                    }
                    return;
                }
            }
        } else {
            QMessageBox::warning(this, "加载错误",
                QString("加载任务文件失败: %1\n\n未找到备份文件。当前数据未被修改。").arg(errorMsg));
        }
    } else {
        // 成功加载
        m_taskModel->setTasks(tasks);
    }
}

void MainWindow::saveTasks()
{
    if (!m_taskStorage->saveTasks(m_taskModel->tasks())) {
        QMessageBox::warning(this, "保存错误", 
            QString("保存任务时出错: %1").arg(m_taskStorage->lastError()));
    }
}

void MainWindow::loadSettings()
{
    QSettings settings("TodoList", "TodoListApp");
    
    m_autoMode = settings.value("autoMode", false).toBool();
    m_autoShowInterval = settings.value("autoShowInterval", 30).toInt();
    m_autoHideDelay = settings.value("autoHideDelay", 5).toInt();
    
    logToFile(QString("Settings loaded - autoMode: %1, interval: %2 min, delay: %3 sec")
        .arg(m_autoMode).arg(m_autoShowInterval).arg(m_autoHideDelay));
    bool darkMode = settings.value("darkMode", false).toBool();
    
    QString dataPath = settings.value("dataPath", m_taskStorage->filePath()).toString();
    m_taskStorage->setFilePath(dataPath);
    
    applyTheme(darkMode);
    
    if (m_autoMode) {
        startAutoMode();
    }
    
    // 恢复窗口位置和大小
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::saveSettings()
{
    QSettings settings("TodoList", "TodoListApp");
    
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::onAddTask()
{
    AddTaskDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        TaskItem task = dialog.getTask();
        task.setId(m_taskStorage->getNextId());
        m_taskModel->addTask(task);
        saveTasks();
    }
}

void MainWindow::onViewCompleted()
{
    m_showCompleted = !m_showCompleted;
    
    if (m_showCompleted) {
        m_taskModel->setFilterStatus(TaskItem::Completed);
        ui->actionViewCompleted->setText("查看待办");
    } else {
        m_taskModel->clearFilter();
        ui->actionViewCompleted->setText("查看已完成");
    }
}

void MainWindow::onShowStats()
{
    StatsDialog dialog(m_taskModel->tasks(), this);
    dialog.exec();
}

void MainWindow::onShowSettings()
{
    SettingsDialog dialog(this);
    
    // 传递当前设置
    dialog.setAutoMode(m_autoMode);
    dialog.setAutoShowInterval(m_autoShowInterval);
    dialog.setAutoHideDelay(m_autoHideDelay);
    
    if (dialog.exec() == QDialog::Accepted) {
        onSettingsChanged();
    }
}

void MainWindow::onTaskDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    
    // 用户点击后进入手动模式，取消自动隐藏
    if (m_autoHideTimer->isActive()) {
        m_autoHideTimer->stop();
        m_manualMode = true;
    }
    
    TaskItem task = m_taskModel->getTask(index.row());
    
    AddTaskDialog dialog(this);
    dialog.setTask(task);
    dialog.setWindowTitle("编辑任务");
    
    if (dialog.exec() == QDialog::Accepted) {
        TaskItem updatedTask = dialog.getTask();
        updatedTask.setId(task.id());
        updatedTask.setCreatedAt(task.createdAt());
        
        m_taskModel->updateTask(index.row(), updatedTask);
        saveTasks();
    }
}

void MainWindow::onDeleteTask()
{
    QModelIndexList selectedIndexes = ui->taskListView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
        return;
    
    // 获取选中的任务数量
    int count = selectedIndexes.count();
    
    QString message;
    if (count == 1) {
        TaskItem task = m_taskModel->getTask(selectedIndexes.first().row());
        message = QString("确定要删除任务 \"%1\" 吗？").arg(task.title());
    } else {
        message = QString("确定要删除选中的 %1 个任务吗？").arg(count);
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认删除",
        message, QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // 从后往前删除，避免索引变化问题
        QList<int> rows;
        for (const QModelIndex& index : selectedIndexes) {
            rows.append(index.row());
        }
        
        // 排序并去重
        std::sort(rows.begin(), rows.end(), std::greater<int>());
        rows.erase(std::unique(rows.begin(), rows.end()), rows.end());
        
        // 删除任务
        for (int row : rows) {
            m_taskModel->removeTask(row);
        }
        
        saveTasks();
    }
}

void MainWindow::onToggleTaskStatus()
{
    QModelIndex index = ui->taskListView->currentIndex();
    if (!index.isValid())
        return;
    
    TaskItem task = m_taskModel->getTask(index.row());
    
    if (task.status() == TaskItem::Todo) {
        task.markAsCompleted();
    } else {
        task.markAsTodo();
    }
    
    m_taskModel->updateTask(index.row(), task);
    saveTasks();
}

void MainWindow::onSearchTextChanged(const QString& text)
{
    m_taskModel->setSearchText(text);
}

void MainWindow::onFilterChanged()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button)
        return;
    
    if (button == ui->showAllButton) {
        m_taskModel->clearFilter();
    } else if (button == ui->showTodoButton) {
        m_taskModel->setFilterStatus(TaskItem::Todo);
    } else if (button == ui->showCompletedButton) {
        m_taskModel->setFilterStatus(TaskItem::Completed);
    }
}

void MainWindow::onAutoShowTimer()
{
    logToFile(QString("onAutoShowTimer called - m_manualMode: %1, isVisible: %2")
        .arg(m_manualMode).arg(isVisible()));
    
    if (!m_manualMode && !isVisible()) {
        // 只有在窗口隐藏时才自动显示
        logToFile("Auto showing window");
        
        // 标记正在自动显示
        m_isAutoShowing = true;
        
        // 自动显示时置顶
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        showWindow();
        qDebug() << "Starting auto hide timer for" << m_autoHideDelay << "seconds";
        m_autoHideTimer->start(m_autoHideDelay * 1000);
        
        // 延迟一小段时间后清除自动显示标志，给窗口显示和焦点事件时间完成
        QTimer::singleShot(200, this, [this]() {
            m_isAutoShowing = false;
            qDebug() << "Auto showing flag cleared";
        });
    } else {
        qDebug() << "Auto show timer triggered but skipped - m_manualMode:" << m_manualMode << "isVisible:" << isVisible();
    }
}

void MainWindow::onAutoHideTimer()
{
    qDebug() << "Auto hide timer triggered, m_manualMode =" << m_manualMode;
    if (!m_manualMode) {
        qDebug() << "Hiding window";
        hideWindow();
    } else {
        qDebug() << "Not hiding - manual mode";
    }
    // 无论是否手动模式，都重置状态，为下一次自动显示做准备
    m_manualMode = false;
    m_autoHideTimer->stop();
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        showWindow();
    }
}

void MainWindow::onSettingsChanged()
{
    QSettings settings("TodoList", "TodoListApp");
    
    m_autoMode = settings.value("autoMode", false).toBool();
    m_autoShowInterval = settings.value("autoShowInterval", 30).toInt();
    m_autoHideDelay = settings.value("autoHideDelay", 5).toInt();
    bool darkMode = settings.value("darkMode", false).toBool();
    
    QString dataPath = settings.value("dataPath").toString();
    if (!dataPath.isEmpty() && dataPath != m_taskStorage->filePath()) {
        m_taskStorage->setFilePath(dataPath);
        loadTasks();
    }
    
    applyTheme(darkMode);
    
    if (m_autoMode) {
        startAutoMode();
    } else {
        stopAutoMode();
    }
}

void MainWindow::startAutoMode()
{
    m_autoShowTimer->start(m_autoShowInterval * 60 * 1000);
    m_manualMode = false;
    logToFile(QString("Auto mode started, timer interval: %1 ms").arg(m_autoShowInterval * 60 * 1000));
}

void MainWindow::stopAutoMode()
{
    m_autoShowTimer->stop();
    m_autoHideTimer->stop();
}

void MainWindow::showWindow()
{
    show();
    raise();
    activateWindow();
    // 如果是最小化状态，恢复窗口
    if (isMinimized()) {
        showNormal();
    }
    // 确保窗口在屏幕上可见
    setFocus();
}

void MainWindow::hideWindow()
{
    hide();
}

void MainWindow::updateTaskCount()
{
    int total = m_taskModel->tasks().count();
    int completed = m_taskModel->completedCount();
    int todo = m_taskModel->todoCount();
    
    QString status = QString("总计: %1 | 待办: %2 | 已完成: %3").arg(total).arg(todo).arg(completed);
    ui->statusBar->showMessage(status);
}

void MainWindow::applyTheme(bool darkMode)
{
    // 贴纸风格不使用深色主题，保持明亮贴纸效果
    Q_UNUSED(darkMode);
}

void MainWindow::applyStickyNoteStyle()
{
    // 贴纸便签风格 CSS - 保留系统边框
    QString stickyStyle = R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FFF9E6, stop:1 #FFF3CC);
        }
        
        QWidget#centralwidget {
            background: transparent;
        }
        
        QMenuBar {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FFEB99, stop:1 #FFE680);
            color: #5D4E37;
            font-weight: bold;
            border-bottom: 1px solid #FFD54F;
            padding: 2px;
        }
        
        QMenuBar::item {
            background: transparent;
            padding: 4px 12px;
            border-radius: 4px;
        }
        
        QMenuBar::item:selected {
            background: rgba(255, 235, 153, 180);
        }
        
        QMenuBar::item:pressed {
            background: #FFE082;
        }
        
        QMenu {
            background: #FFF9E6;
            border: 2px solid #FFD54F;
            border-radius: 8px;
            padding: 5px;
        }
        
        QMenu::item {
            padding: 8px 30px;
            border-radius: 4px;
            color: #5D4E37;
        }
        
        QMenu::item:selected {
            background: #FFE082;
        }
        
        QToolBar {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FFEB99, stop:1 #FFE680);
            border: none;
            border-bottom: 2px solid #FFD54F;
            padding: 8px;
            spacing: 8px;
        }
        
        QToolButton {
            background: rgba(255, 255, 255, 120);
            border: 2px solid #FFD54F;
            border-radius: 8px;
            padding: 8px 15px;
            color: #5D4E37;
            font-weight: bold;
            font-size: 13px;
        }
        
        QToolButton:hover {
            background: rgba(255, 255, 255, 200);
            border: 2px solid #FFC107;
        }
        
        QToolButton:pressed {
            background: #FFE082;
        }
        
        QLineEdit {
            background: white;
            border: 2px solid #FFD54F;
            border-radius: 10px;
            padding: 8px 15px;
            font-size: 13px;
            color: #333;
        }
        
        QLineEdit:focus {
            border: 2px solid #FFC107;
            background: #FFFEF7;
        }
        
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FFE082, stop:1 #FFD54F);
            border: 2px solid #FFC107;
            border-radius: 10px;
            padding: 8px 18px;
            color: #5D4E37;
            font-weight: bold;
            font-size: 12px;
        }
        
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FFEB99, stop:1 #FFE082);
            border: 2px solid #FF9800;
        }
        
        QPushButton:pressed {
            background: #FFD54F;
        }
        
        QListView {
            background: rgba(255, 255, 255, 220);
            border: 2px solid #FFE082;
            border-radius: 12px;
            padding: 8px;
            outline: none;
        }
        
        QListView::item {
            border-radius: 10px;
            padding: 5px;
            margin: 3px;
        }
        
        QListView::item:selected {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FFE082, stop:1 #FFD54F);
            border: 2px solid #FFC107;
        }
        
        QListView::item:hover {
            background: rgba(255, 235, 153, 120);
        }
        
        QStatusBar {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FFEB99, stop:1 #FFE680);
            border: none;
            border-top: 2px solid #FFD54F;
            color: #5D4E37;
            font-weight: bold;
            padding: 6px;
        }
    )";
    
    setStyleSheet(stickyStyle);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // 最小化到托盘而不是退出
    if (m_trayIcon->isVisible()) {
        hide();
        event->ignore();
        
        // 重置手动模式标志，允许下次自动弹出
        m_manualMode = false;
        m_autoHideTimer->stop();
        logToFile("Window closed to tray, manual mode reset");
    } else {
        event->accept();
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized() && m_trayIcon->isVisible()) {
            hide();
            event->ignore();
            
            // 重置手动模式标志，允许下次自动弹出
            m_manualMode = false;
            m_autoHideTimer->stop();
            logToFile("Window minimized to tray, manual mode reset");
            return;
        }
    }
    QMainWindow::changeEvent(event);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    // 只处理属于这个窗口或其子控件的事件
    QWidget* widget = qobject_cast<QWidget*>(obj);
    if (!widget || !widget->window() || widget->window() != this) {
        return QObject::eventFilter(obj, event);
    }
    
    // 检测用户交互事件（鼠标点击、键盘输入等）
    if (m_autoMode && !m_manualMode && !m_isAutoShowing && isVisible()) {
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::KeyPress ||
            event->type() == QEvent::Wheel) {
            
            qDebug() << "User interaction detected on" << obj->metaObject()->className() << ", entering manual mode";
            // 用户开始操作，进入手动模式
            m_manualMode = true;
            m_autoHideTimer->stop();
            
            // 取消置顶
            setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
            show(); // 重新显示以应用窗口标志变化
        }
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::onShowHelp()
{
    HelpDialog dialog(this);
    dialog.exec();
}

void MainWindow::onTaskReceivedFromApi(const QString& title, const QString& description, const QString& category, int priority, const QString& dueDate, const QStringList& tags)
{
    // 创建新任务
    TaskItem task;
    task.setId(m_taskStorage->getNextId());
    task.setTitle(title);
    task.setDescription(description);
    task.setCategory(category);
    task.setStatus(TaskItem::Todo);
    task.setPriority(priority > 0 && priority <= 5 ? priority : 3); // 验证优先级范围
    task.setCreatedAt(QDateTime::currentDateTime());
    
    // 设置截止日期（如果提供）
    if (!dueDate.isEmpty()) {
        QDateTime dt = QDateTime::fromString(dueDate, Qt::ISODate);
        if (dt.isValid()) {
            task.setDueDate(dt);
        }
    }
    
    // 设置标签（如果提供）
    if (!tags.isEmpty()) {
        task.setTags(tags);
    }
    
    // 添加到模型
    m_taskModel->addTask(task);
    
    // 保存
    saveTasks();
    
    // 显示通知
    if (m_trayIcon->isVisible()) {
        m_trayIcon->showMessage(
            "新任务", 
            QString("已添加: %1").arg(title),
            QSystemTrayIcon::Information,
            3000
        );
    }
}
