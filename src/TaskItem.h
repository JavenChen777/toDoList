#ifndef TASKITEM_H
#define TASKITEM_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QStringList>

/**
 * @brief 任务项数据结构
 * 
 * 表示单个待办事项，包含标题、描述、状态和时间信息
 */
class TaskItem
{
public:
    // 任务状态枚举
    enum Status {
        Todo,       // 待完成
        Completed   // 已完成
    };

    TaskItem();
    TaskItem(int id, const QString& title, const QString& description = QString());

    // Getter 方法
    int id() const { return m_id; }
    QString title() const { return m_title; }
    QString description() const { return m_description; }
    Status status() const { return m_status; }
    QDateTime createdAt() const { return m_createdAt; }
    QDateTime completedAt() const { return m_completedAt; }
    QDateTime dueDate() const { return m_dueDate; }
    QString category() const { return m_category; }
    int priority() const { return m_priority; }
    QStringList tags() const { return m_tags; }

    // Setter 方法
    void setId(int id) { m_id = id; }
    void setTitle(const QString& title) { m_title = title; }
    void setDescription(const QString& description) { m_description = description; }
    void setStatus(Status status);
    void setCreatedAt(const QDateTime& dateTime) { m_createdAt = dateTime; }
    void setCompletedAt(const QDateTime& dateTime) { m_completedAt = dateTime; }
    void setDueDate(const QDateTime& dateTime) { m_dueDate = dateTime; }
    void setCategory(const QString& category) { m_category = category; }
    void setPriority(int priority) { m_priority = priority; }
    void setTags(const QStringList& tags) { m_tags = tags; }

    // 序列化方法
    QJsonObject toJson() const;
    static TaskItem fromJson(const QJsonObject& json);

    // 状态转换
    void markAsCompleted();
    void markAsTodo();

    // 状态字符串转换
    static QString statusToString(Status status);
    static Status stringToStatus(const QString& str);

private:
    int m_id;                   // 任务ID
    QString m_title;            // 任务标题
    QString m_description;      // 任务描述
    Status m_status;            // 任务状态
    QDateTime m_createdAt;      // 创建时间
    QDateTime m_completedAt;    // 完成时间
    QDateTime m_dueDate;        // 截止日期
    QString m_category;         // 分类
    int m_priority;             // 优先级 (1-5, 5最高)
    QStringList m_tags;         // 标签列表
};

#endif // TASKITEM_H
