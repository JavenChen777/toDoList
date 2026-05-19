#include "TaskItem.h"
#include <QJsonObject>
#include <QJsonArray>

TaskItem::TaskItem()
    : m_id(0)
    , m_status(Todo)
    , m_createdAt(QDateTime::currentDateTime())
    , m_priority(3)
{
}

TaskItem::TaskItem(int id, const QString& title, const QString& description)
    : m_id(id)
    , m_title(title)
    , m_description(description)
    , m_status(Todo)
    , m_createdAt(QDateTime::currentDateTime())
    , m_priority(3)
{
}

void TaskItem::setStatus(Status status)
{
    m_status = status;
    if (status == Completed && !m_completedAt.isValid()) {
        m_completedAt = QDateTime::currentDateTime();
    } else if (status == Todo) {
        m_completedAt = QDateTime();
    }
}

void TaskItem::markAsCompleted()
{
    setStatus(Completed);
}

void TaskItem::markAsTodo()
{
    setStatus(Todo);
}

QString TaskItem::statusToString(Status status)
{
    switch (status) {
        case Todo: return "todo";
        case Completed: return "completed";
        default: return "unknown";
    }
}

TaskItem::Status TaskItem::stringToStatus(const QString& str)
{
    if (str == "completed") return Completed;
    return Todo;
}

QJsonObject TaskItem::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["title"] = m_title;
    json["description"] = m_description;
    json["status"] = statusToString(m_status);
    json["created_at"] = m_createdAt.toString(Qt::ISODate);
    json["completed_at"] = m_completedAt.isValid() ? m_completedAt.toString(Qt::ISODate) : QString();
    json["due_date"] = m_dueDate.isValid() ? m_dueDate.toString(Qt::ISODate) : QString();
    json["category"] = m_category;
    json["priority"] = m_priority;
    
    // 序列化标签数组
    QJsonArray tagsArray;
    for (const QString& tag : m_tags) {
        tagsArray.append(tag);
    }
    json["tags"] = tagsArray;
    
    return json;
}

TaskItem TaskItem::fromJson(const QJsonObject& json)
{
    TaskItem task;
    task.setId(json["id"].toInt());
    task.setTitle(json["title"].toString());
    task.setDescription(json["description"].toString());
    task.setStatus(stringToStatus(json["status"].toString()));
    task.setCreatedAt(QDateTime::fromString(json["created_at"].toString(), Qt::ISODate));
    
    QString completedStr = json["completed_at"].toString();
    if (!completedStr.isEmpty()) {
        task.setCompletedAt(QDateTime::fromString(completedStr, Qt::ISODate));
    }
    
    QString dueDateStr = json["due_date"].toString();
    if (!dueDateStr.isEmpty()) {
        task.setDueDate(QDateTime::fromString(dueDateStr, Qt::ISODate));
    }
    
    task.setCategory(json["category"].toString());
    task.setPriority(json["priority"].toInt(3));
    
    // 反序列化标签数组
    QStringList tags;
    QJsonArray tagsArray = json["tags"].toArray();
    for (const QJsonValue& tag : tagsArray) {
        tags.append(tag.toString());
    }
    task.setTags(tags);
    
    return task;
}
