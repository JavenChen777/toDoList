#ifndef TASKSTORAGE_H
#define TASKSTORAGE_H

#include <QString>
#include <QList>
#include "TaskItem.h"

/**
 * @brief 任务数据持久化类
 * 
 * 负责任务数据的保存和加载，支持 JSON 格式
 */
class TaskStorage
{
public:
    TaskStorage();
    explicit TaskStorage(const QString& filePath);

    // 设置存储文件路径
    void setFilePath(const QString& filePath);
    QString filePath() const { return m_filePath; }

    // 保存和加载任务
    bool saveTasks(const QList<TaskItem>& tasks);
    QList<TaskItem> loadTasks();
    
    // 安全加载（失败时不返回空列表）
    bool loadTasksSafe(QList<TaskItem>& tasks);

    // 备份和恢复
    bool createBackup();
    bool restoreFromBackup();
    QList<TaskItem> loadFromBackup();
    bool hasBackup() const;

    // 获取下一个可用ID
    int getNextId() const;

    // 错误信息
    QString lastError() const { return m_lastError; }
    
    // 获取备份文件路径
    QString getBackupFilePath() const;

private:
    QString m_filePath;
    QString m_lastError;
    int m_nextId;

    void updateNextId(const QList<TaskItem>& tasks);
    QString getDefaultFilePath() const;
};

#endif // TASKSTORAGE_H
