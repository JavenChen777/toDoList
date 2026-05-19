#ifndef ADDTASKDIALOG_H
#define ADDTASKDIALOG_H

#include <QDialog>
#include "TaskItem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AddTaskDialog; }
QT_END_NAMESPACE

/**
 * @brief 添加/编辑任务对话框
 */
class AddTaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTaskDialog(QWidget* parent = nullptr);
    ~AddTaskDialog();

    // 设置和获取任务
    void setTask(const TaskItem& task);
    TaskItem getTask() const;

private slots:
    void onAccept();

private:
    Ui::AddTaskDialog* ui;
    TaskItem m_task;
};

#endif // ADDTASKDIALOG_H
