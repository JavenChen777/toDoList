#ifndef TASKDELEGATE_H
#define TASKDELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief 任务列表项委托
 * 
 * 自定义任务在列表中的显示样式
 */
class TaskDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TaskDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
               const QModelIndex& index) const override;
    
    QSize sizeHint(const QStyleOptionViewItem& option, 
                   const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option, 
                     const QModelIndex& index) override;
};

#endif // TASKDELEGATE_H
