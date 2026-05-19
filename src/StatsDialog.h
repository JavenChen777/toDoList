#ifndef STATSDIALOG_H
#define STATSDIALOG_H

#include <QDialog>
#include <QList>
#include "TaskItem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class StatsDialog; }
QT_END_NAMESPACE

/**
 * @brief 统计对话框
 * 
 * 显示任务完成统计和图表
 */
class StatsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatsDialog(const QList<TaskItem>& tasks, QWidget* parent = nullptr);
    ~StatsDialog();

private:
    Ui::StatsDialog* ui;
    QList<TaskItem> m_tasks;
    
    void setupCharts();
    void createCompletionChart();
    void createCategoryChart();
    void createPriorityChart();
    void updateStatistics();
    
    // 统计辅助方法
    QMap<QString, int> getTasksByCategory() const;
    QMap<int, int> getTasksByPriority() const;
    QMap<QDate, int> getCompletionsByDate() const;
};

#endif // STATSDIALOG_H
