#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "TaskItem.h"

/**
 * @brief 任务列表模型
 * 
 * 管理任务列表数据，为视图提供数据接口
 */
class TaskModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // 自定义角色
    enum TaskRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole,
        StatusRole,
        CreatedAtRole,
        CompletedAtRole,
        CategoryRole,
        PriorityRole
    };

    explicit TaskModel(QObject* parent = nullptr);

    // QAbstractListModel 必须实现的方法
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    // 自定义方法
    void addTask(const TaskItem& task);
    void removeTask(int row);
    void updateTask(int row, const TaskItem& task);
    TaskItem getTask(int row) const;
    
    // 批量操作
    void setTasks(const QList<TaskItem>& tasks);
    QList<TaskItem> tasks() const { return m_tasks; }
    
    // 过滤和搜索
    void setFilterStatus(TaskItem::Status status);
    void clearFilter();
    void setSearchText(const QString& text);

    // 统计方法
    int completedCount() const;
    int todoCount() const;

signals:
    void taskAdded(const TaskItem& task);
    void taskRemoved(int id);
    void taskUpdated(const TaskItem& task);

private:
    QList<TaskItem> m_tasks;
    QList<TaskItem> m_filteredTasks;
    bool m_filtered;
    bool m_statusFilterEnabled;
    TaskItem::Status m_filterStatus;
    QString m_searchText;

    void applyFilter();
    bool matchesFilter(const TaskItem& task) const;
};

#endif // TASKMODEL_H
