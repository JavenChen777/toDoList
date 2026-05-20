#include "TaskModel.h"

TaskModel::TaskModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_filtered(false)
    , m_statusFilterEnabled(false)
    , m_filterStatus(TaskItem::Todo)
{
}

int TaskModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    
    return m_filtered ? m_filteredTasks.count() : m_tasks.count();
}

QVariant TaskModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return QVariant();

    const TaskItem& task = m_filtered ? m_filteredTasks.at(index.row()) : m_tasks.at(index.row());

    switch (role) {
        case Qt::DisplayRole:
        case TitleRole:
            return task.title();
        case IdRole:
            return task.id();
        case DescriptionRole:
            return task.description();
        case StatusRole:
            return task.status();
        case CreatedAtRole:
            return task.createdAt();
        case CompletedAtRole:
            return task.completedAt();
        case CategoryRole:
            return task.category();
        case PriorityRole:
            return task.priority();
        case Qt::CheckStateRole:
            return task.status() == TaskItem::Completed ? Qt::Checked : Qt::Unchecked;
        default:
            return QVariant();
    }
}

Qt::ItemFlags TaskModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

bool TaskModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= rowCount())
        return false;

    // 获取原始任务ID
    int taskId = getTask(index.row()).id();
    
    // 在原始列表中查找并更新任务
    bool updated = false;
    for (int i = 0; i < m_tasks.count(); ++i) {
        if (m_tasks[i].id() == taskId) {
            TaskItem& task = m_tasks[i];
            
            switch (role) {
                case TitleRole:
                    task.setTitle(value.toString());
                    break;
                case DescriptionRole:
                    task.setDescription(value.toString());
                    break;
                case StatusRole:
                    task.setStatus(static_cast<TaskItem::Status>(value.toInt()));
                    break;
                case Qt::CheckStateRole:
                    if (value.toInt() == Qt::Checked)
                        task.markAsCompleted();
                    else
                        task.markAsTodo();
                    break;
                default:
                    return false;
            }
            
            updated = true;
            emit taskUpdated(task);
            break;
        }
    }
    
    if (!updated)
        return false;
    
    // 如果有过滤，重新应用过滤
    if (m_filtered) {
        beginResetModel();
        applyFilter();
        endResetModel();
    } else {
        emit dataChanged(index, index, {role});
    }
    
    return true;
}

void TaskModel::addTask(const TaskItem& task)
{
    if (!m_filtered || matchesFilter(task)) {
        int row = m_filtered ? m_filteredTasks.count() : m_tasks.count();
        beginInsertRows(QModelIndex(), row, row);
        
        if (!m_filtered || matchesFilter(task)) {
            if (m_filtered)
                m_filteredTasks.append(task);
        }
        
        m_tasks.append(task);
        endInsertRows();
        
        emit taskAdded(task);
    } else {
        m_tasks.append(task);
        emit taskAdded(task);
    }
}

void TaskModel::removeTask(int row)
{
    if (row < 0 || row >= rowCount())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    
    TaskItem task = m_filtered ? m_filteredTasks.at(row) : m_tasks.at(row);
    int taskId = task.id();
    
    if (m_filtered) {
        m_filteredTasks.removeAt(row);
        m_tasks.removeIf([taskId](const TaskItem& t) { return t.id() == taskId; });
    } else {
        m_tasks.removeAt(row);
    }
    
    endRemoveRows();
    
    emit taskRemoved(taskId);
}

void TaskModel::updateTask(int row, const TaskItem& task)
{
    if (row < 0 || row >= rowCount())
        return;

    // 获取任务ID
    int taskId = getTask(row).id();
    
    // 更新原始列表中的任务
    for (int i = 0; i < m_tasks.count(); ++i) {
        if (m_tasks[i].id() == taskId) {
            m_tasks[i] = task;
            break;
        }
    }
    
    // 如果有过滤，重新应用过滤
    if (m_filtered) {
        beginResetModel();
        applyFilter();
        endResetModel();
    } else {
        // 直接更新
        m_tasks[row] = task;
        QModelIndex idx = index(row);
        emit dataChanged(idx, idx);
    }

    emit taskUpdated(task);
}

TaskItem TaskModel::getTask(int row) const
{
    if (row < 0 || row >= rowCount())
        return TaskItem();

    return m_filtered ? m_filteredTasks.at(row) : m_tasks.at(row);
}

void TaskModel::setTasks(const QList<TaskItem>& tasks)
{
    beginResetModel();
    m_tasks = tasks;
    if (m_filtered) {
        applyFilter();
    }
    endResetModel();
}

void TaskModel::setFilterStatus(TaskItem::Status status)
{
    m_filterStatus = status;
    m_statusFilterEnabled = true;
    m_filtered = true;
    beginResetModel();
    applyFilter();
    endResetModel();
}

void TaskModel::clearFilter()
{
    if (!m_filtered)
        return;
        
    beginResetModel();
    m_filtered = false;
    m_statusFilterEnabled = false;
    m_filteredTasks.clear();
    m_searchText.clear();
    endResetModel();
}

void TaskModel::setSearchText(const QString& text)
{
    m_searchText = text;
    beginResetModel();
    m_filtered = m_statusFilterEnabled || !m_searchText.isEmpty();
    if (m_filtered) {
        applyFilter();
    } else {
        m_filteredTasks.clear();
    }
    endResetModel();
}

void TaskModel::applyFilter()
{
    m_filteredTasks.clear();
    for (const TaskItem& task : m_tasks) {
        if (matchesFilter(task)) {
            m_filteredTasks.append(task);
        }
    }
}

bool TaskModel::matchesFilter(const TaskItem& task) const
{
    // 状态过滤
    if (m_statusFilterEnabled && task.status() != m_filterStatus) {
        return false;
    }

    // 文本搜索
    if (!m_searchText.isEmpty()) {
        QString search = m_searchText.toLower();
        return task.title().toLower().contains(search) ||
               task.description().toLower().contains(search) ||
               task.category().toLower().contains(search);
    }

    return true;
}

int TaskModel::completedCount() const
{
    return std::count_if(m_tasks.begin(), m_tasks.end(), 
        [](const TaskItem& task) { return task.status() == TaskItem::Completed; });
}

int TaskModel::todoCount() const
{
    return std::count_if(m_tasks.begin(), m_tasks.end(), 
        [](const TaskItem& task) { return task.status() == TaskItem::Todo; });
}
