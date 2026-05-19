#include "TaskDelegate.h"
#include "TaskModel.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

TaskDelegate::TaskDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void TaskDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const
{
    if (!index.isValid())
        return;

    painter->save();

    // 绘制背景
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, option.palette.midlight());
    } else {
        painter->fillRect(option.rect, option.palette.base());
    }

    // 获取数据
    QString title = index.data(TaskModel::TitleRole).toString();
    QString description = index.data(TaskModel::DescriptionRole).toString();
    int status = index.data(TaskModel::StatusRole).toInt();
    int priority = index.data(TaskModel::PriorityRole).toInt();
    QString category = index.data(TaskModel::CategoryRole).toString();

    // 设置文本颜色
    QColor textColor = option.state & QStyle::State_Selected ? 
                       option.palette.highlightedText().color() : 
                       option.palette.text().color();

    // 绘制复选框
    QRect checkBoxRect(option.rect.left() + 10, option.rect.top() + 15, 20, 20);
    QStyleOptionButton checkBoxOption;
    checkBoxOption.rect = checkBoxRect;
    checkBoxOption.state = QStyle::State_Enabled;
    if (status == 1) {  // Completed
        checkBoxOption.state |= QStyle::State_On;
    } else {
        checkBoxOption.state |= QStyle::State_Off;
    }
    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);

    // 绘制标题
    QRect titleRect(option.rect.left() + 45, option.rect.top() + 10, 
                    option.rect.width() - 150, 25);
    QFont titleFont = option.font;
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    painter->setFont(titleFont);
    painter->setPen(textColor);
    
    if (status == 1) {  // 已完成，添加删除线
        titleFont.setStrikeOut(true);
        painter->setFont(titleFont);
        painter->setPen(Qt::gray);
    }
    
    painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, title);

    // 绘制描述
    if (!description.isEmpty()) {
        QRect descRect(option.rect.left() + 45, option.rect.top() + 35,
                      option.rect.width() - 150, 20);
        QFont descFont = option.font;
        descFont.setPointSize(descFont.pointSize() - 1);
        painter->setFont(descFont);
        painter->setPen(Qt::gray);
        
        QString truncated = description.left(50);
        if (description.length() > 50) {
            truncated += "...";
        }
        painter->drawText(descRect, Qt::AlignLeft | Qt::AlignVCenter, truncated);
    }

    // 绘制优先级标签
    if (priority > 0) {
        QRect priorityRect(option.rect.right() - 100, option.rect.top() + 10, 50, 20);
        painter->setPen(Qt::NoPen);
        
        // 根据优先级设置颜色
        QColor priorityColor;
        QString priorityText;
        if (priority >= 5) {
            priorityColor = QColor(220, 53, 69);  // 红色 - 最高
            priorityText = "高";
        } else if (priority >= 4) {
            priorityColor = QColor(255, 152, 0);  // 橙色 - 高
            priorityText = "高";
        } else if (priority >= 3) {
            priorityColor = QColor(255, 193, 7);  // 黄色 - 中
            priorityText = "中";
        } else {
            priorityColor = QColor(108, 117, 125); // 灰色 - 低
            priorityText = "低";
        }
        
        painter->setBrush(priorityColor);
        painter->drawRoundedRect(priorityRect, 3, 3);
        
        painter->setPen(Qt::white);
        QFont tagFont = option.font;
        tagFont.setPointSize(tagFont.pointSize() - 1);
        painter->setFont(tagFont);
        painter->drawText(priorityRect, Qt::AlignCenter, priorityText);
    }

    // 绘制分类标签
    if (!category.isEmpty()) {
        QRect categoryRect(option.rect.right() - 130, option.rect.top() + 35, 80, 20);
        painter->setPen(Qt::gray);
        QFont catFont = option.font;
        catFont.setPointSize(catFont.pointSize() - 2);
        painter->setFont(catFont);
        painter->drawText(categoryRect, Qt::AlignRight | Qt::AlignVCenter, category);
    }

    painter->restore();
}

QSize TaskDelegate::sizeHint(const QStyleOptionViewItem& option, 
                            const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(0, 65);  // 每项高度65像素
}

bool TaskDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                               const QStyleOptionViewItem& option, 
                               const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QRect checkBoxRect(option.rect.left() + 10, option.rect.top() + 15, 20, 20);
        
        if (checkBoxRect.contains(mouseEvent->pos())) {
            // 切换任务状态
            int currentStatus = index.data(TaskModel::StatusRole).toInt();
            int newStatus = (currentStatus == 0) ? 1 : 0;
            model->setData(index, newStatus, TaskModel::StatusRole);
            return true;
        }
    }
    
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
