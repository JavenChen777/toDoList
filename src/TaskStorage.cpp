#include "TaskStorage.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

TaskStorage::TaskStorage()
    : m_filePath(getDefaultFilePath())
    , m_nextId(1)
{
}

TaskStorage::TaskStorage(const QString& filePath)
    : m_filePath(filePath)
    , m_nextId(1)
{
}

void TaskStorage::setFilePath(const QString& filePath)
{
    m_filePath = filePath;
}

QString TaskStorage::getDefaultFilePath() const
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.filePath("tasks.json");
}

bool TaskStorage::saveTasks(const QList<TaskItem>& tasks)
{
    // 如果原文件存在，先尝试创建备份
    QFile existingFile(m_filePath);
    if (existingFile.exists() && existingFile.size() > 0) {
        // 关键修复：先验证原文件是否有效
        // 只有当原文件是有效的JSON时，才将其作为备份覆盖 .backup 文件
        // 这样即使当前加载失败（原文件损坏），退出时也不会用损坏的文件覆盖掉之前完好的备份
        bool isValid = false;
        if (existingFile.open(QIODevice::ReadOnly)) {
            QByteArray data = existingFile.readAll();
            existingFile.close();
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(data, &error);
            // 必须是有效的JSON且是对象结构
            if (error.error == QJsonParseError::NoError && doc.isObject()) {
                isValid = true;
            }
        }

        if (isValid) {
            if (!createBackup()) {
                // 备份失败，记录警告但继续保存
                qWarning() << "警告: 创建备份失败:" << m_lastError;
            }
        } else {
            // 如果原文件损坏，不要覆盖备份！
            // 另外保存一份损坏的文件供分析
            qWarning() << "警告: 检测到原文件已损坏，跳过备份以保护现有备份文件";
            QString corruptedPath = m_filePath + ".corrupted";
            // 如果存在旧的.corrupted文件，删除之
            QFile::remove(corruptedPath);
            QFile::copy(m_filePath, corruptedPath);
        }
    }

    QJsonArray taskArray;
    for (const TaskItem& task : tasks) {
        taskArray.append(task.toJson());
    }

    QJsonObject root;
    root["version"] = "1.0";
    root["tasks"] = taskArray;

    QJsonDocument doc(root);

    // 先写入临时文件
    QString tempFilePath = m_filePath + ".tmp";
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly)) {
        m_lastError = QString("无法打开临时文件进行写入: %1").arg(tempFile.errorString());
        return false;
    }

    qint64 bytesWritten = tempFile.write(doc.toJson(QJsonDocument::Indented));
    tempFile.close();

    if (bytesWritten == -1) {
        m_lastError = "写入临时文件失败";
        QFile::remove(tempFilePath);
        return false;
    }

    // 验证临时文件是否可以正确解析
    QFile verifyFile(tempFilePath);
    if (!verifyFile.open(QIODevice::ReadOnly)) {
        m_lastError = "无法验证临时文件";
        QFile::remove(tempFilePath);
        return false;
    }
    
    QByteArray verifyData = verifyFile.readAll();
    verifyFile.close();
    
    QJsonParseError verifyError;
    QJsonDocument verifyDoc = QJsonDocument::fromJson(verifyData, &verifyError);
    if (verifyError.error != QJsonParseError::NoError) {
        m_lastError = "临时文件验证失败";
        QFile::remove(tempFilePath);
        return false;
    }

    // 用临时文件替换原文件
    if (existingFile.exists()) {
        if (!existingFile.remove()) {
            m_lastError = QString("无法删除原文件: %1").arg(existingFile.errorString());
            QFile::remove(tempFilePath);
            return false;
        }
    }

    if (!QFile::rename(tempFilePath, m_filePath)) {
        m_lastError = "无法重命名临时文件为目标文件";
        // 尝试从备份恢复
        restoreFromBackup();
        return false;
    }

    updateNextId(tasks);
    m_lastError.clear();
    return true;
}

QList<TaskItem> TaskStorage::loadTasks()
{
    QList<TaskItem> tasks;

    QFile file(m_filePath);
    if (!file.exists()) {
        // 文件不存在，返回空列表（不是错误）
        m_lastError.clear();
        return tasks;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("无法打开文件进行读取: %1").arg(file.errorString());
        return tasks;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        m_lastError = QString("JSON 解析错误: %1").arg(parseError.errorString());
        return tasks;
    }

    if (!doc.isObject()) {
        m_lastError = "无效的 JSON 格式";
        return tasks;
    }

    QJsonObject root = doc.object();
    QJsonArray taskArray = root["tasks"].toArray();

    for (const QJsonValue& value : taskArray) {
        if (value.isObject()) {
            TaskItem task = TaskItem::fromJson(value.toObject());
            tasks.append(task);
        }
    }

    updateNextId(tasks);
    m_lastError.clear();
    return tasks;
}

int TaskStorage::getNextId() const
{
    return m_nextId;
}

void TaskStorage::updateNextId(const QList<TaskItem>& tasks)
{
    m_nextId = 1;
    for (const TaskItem& task : tasks) {
        if (task.id() >= m_nextId) {
            m_nextId = task.id() + 1;
        }
    }
}

bool TaskStorage::loadTasksSafe(QList<TaskItem>& tasks)
{
    QFile file(m_filePath);
    if (!file.exists()) {
        // 文件不存在，返回true但tasks保持不变
        m_lastError.clear();
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("无法打开文件进行读取: %1").arg(file.errorString());
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    // 检查文件是否为空
    if (data.isEmpty()) {
        m_lastError = "文件为空";
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        m_lastError = QString("JSON 解析错误: %1 (位置: %2)").arg(parseError.errorString()).arg(parseError.offset);
        return false;
    }

    if (!doc.isObject()) {
        m_lastError = "无效的 JSON 格式";
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray taskArray = root["tasks"].toArray();

    QList<TaskItem> loadedTasks;
    for (const QJsonValue& value : taskArray) {
        if (value.isObject()) {
            TaskItem task = TaskItem::fromJson(value.toObject());
            loadedTasks.append(task);
        }
    }

    // 只有成功加载后才更新传入的tasks
    tasks = loadedTasks;
    updateNextId(tasks);
    m_lastError.clear();
    return true;
}

QString TaskStorage::getBackupFilePath() const
{
    return m_filePath + ".backup";
}

bool TaskStorage::createBackup()
{
    QFile sourceFile(m_filePath);
    if (!sourceFile.exists()) {
        return true; // 源文件不存在，无需备份
    }

    QString backupPath = getBackupFilePath();
    
    // 如果备份文件已存在，先删除
    QFile oldBackup(backupPath);
    if (oldBackup.exists()) {
        if (!oldBackup.remove()) {
            m_lastError = QString("无法删除旧备份文件: %1").arg(oldBackup.errorString());
            return false;
        }
    }

    // 复制文件
    if (!sourceFile.copy(backupPath)) {
        m_lastError = QString("无法创建备份: %1").arg(sourceFile.errorString());
        return false;
    }

    m_lastError.clear();
    return true;
}

bool TaskStorage::restoreFromBackup()
{
    QString backupPath = getBackupFilePath();
    QFile backupFile(backupPath);
    
    if (!backupFile.exists()) {
        m_lastError = "备份文件不存在";
        return false;
    }

    // 删除损坏的原文件
    QFile originalFile(m_filePath);
    if (originalFile.exists()) {
        if (!originalFile.remove()) {
            m_lastError = QString("无法删除损坏的文件: %1").arg(originalFile.errorString());
            return false;
        }
    }

    // 从备份恢复
    if (!backupFile.copy(m_filePath)) {
        m_lastError = QString("无法从备份恢复: %1").arg(backupFile.errorString());
        return false;
    }

    m_lastError.clear();
    return true;
}

QList<TaskItem> TaskStorage::loadFromBackup()
{
    QString backupPath = getBackupFilePath();
    QFile file(backupPath);
    
    QList<TaskItem> tasks;
    
    if (!file.exists()) {
        m_lastError = "备份文件不存在";
        return tasks;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("无法打开备份文件: %1").arg(file.errorString());
        return tasks;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        m_lastError = QString("备份文件 JSON 解析错误: %1").arg(parseError.errorString());
        return tasks;
    }

    if (!doc.isObject()) {
        m_lastError = "备份文件无效的 JSON 格式";
        return tasks;
    }

    QJsonObject root = doc.object();
    QJsonArray taskArray = root["tasks"].toArray();

    for (const QJsonValue& value : taskArray) {
        if (value.isObject()) {
            TaskItem task = TaskItem::fromJson(value.toObject());
            tasks.append(task);
        }
    }

    updateNextId(tasks);
    m_lastError.clear();
    return tasks;
}

bool TaskStorage::hasBackup() const
{
    QFile backupFile(getBackupFilePath());
    return backupFile.exists() && backupFile.size() > 0;
}
