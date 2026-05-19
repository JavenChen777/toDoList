#include "StatsDialog.h"
#include "ui_StatsDialog.h"
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QMap>

StatsDialog::StatsDialog(const QList<TaskItem>& tasks, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::StatsDialog)
    , m_tasks(tasks)
{
    ui->setupUi(this);
    
    setWindowTitle("任务统计");
    resize(800, 600);
    
    updateStatistics();
    setupCharts();
}

StatsDialog::~StatsDialog()
{
    delete ui;
}

void StatsDialog::setupCharts()
{
    createCompletionChart();
    createCategoryChart();
    createPriorityChart();
}

void StatsDialog::createCompletionChart()
{
    // 创建完成趋势折线图
    auto completions = getCompletionsByDate();
    
    QLineSeries* series = new QLineSeries();
    series->setName("每日完成数");
    
    QStringList categories;
    int maxValue = 0;
    
    for (auto it = completions.begin(); it != completions.end(); ++it) {
        QString dateStr = it.key().toString("MM/dd");
        categories << dateStr;
        series->append(categories.count() - 1, it.value());
        maxValue = qMax(maxValue, it.value());
    }
    
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("任务完成趋势（最近7天）");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, maxValue + 1);
    axisY->setLabelFormat("%d");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->completionTab->layout());
    if (!layout) {
        layout = new QVBoxLayout(ui->completionTab);
        ui->completionTab->setLayout(layout);
    }
    layout->addWidget(chartView);
}

void StatsDialog::createCategoryChart()
{
    // 创建分类饼图
    auto categories = getTasksByCategory();
    
    if (categories.isEmpty()) {
        QLabel* label = new QLabel("暂无分类数据", ui->categoryTab);
        label->setAlignment(Qt::AlignCenter);
        QVBoxLayout* layout = new QVBoxLayout(ui->categoryTab);
        layout->addWidget(label);
        return;
    }
    
    QPieSeries* series = new QPieSeries();
    
    for (auto it = categories.begin(); it != categories.end(); ++it) {
        QString category = it.key().isEmpty() ? "未分类" : it.key();
        series->append(category, it.value());
    }
    
    // 设置切片样式
    for (QPieSlice* slice : series->slices()) {
        slice->setLabel(QString("%1: %2 (%3%)")
                       .arg(slice->label())
                       .arg(slice->value())
                       .arg(100 * slice->percentage(), 0, 'f', 1));
        slice->setLabelVisible(true);
    }
    
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("任务分类分布");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);
    
    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->categoryTab->layout());
    if (!layout) {
        layout = new QVBoxLayout(ui->categoryTab);
        ui->categoryTab->setLayout(layout);
    }
    layout->addWidget(chartView);
}

void StatsDialog::createPriorityChart()
{
    // 创建优先级柱状图
    auto priorities = getTasksByPriority();
    
    QBarSet* todoSet = new QBarSet("待办");
    QBarSet* completedSet = new QBarSet("已完成");
    
    for (int p = 1; p <= 5; ++p) {
        int todoCount = 0;
        int completedCount = 0;
        
        for (const TaskItem& task : m_tasks) {
            if (task.priority() == p) {
                if (task.status() == TaskItem::Todo)
                    todoCount++;
                else
                    completedCount++;
            }
        }
        
        *todoSet << todoCount;
        *completedSet << completedCount;
    }
    
    QBarSeries* series = new QBarSeries();
    series->append(todoSet);
    series->append(completedSet);
    
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("优先级分布");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    QStringList categories;
    categories << "P1" << "P2" << "P3" << "P4" << "P5";
    
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->priorityTab->layout());
    if (!layout) {
        layout = new QVBoxLayout(ui->priorityTab);
        ui->priorityTab->setLayout(layout);
    }
    layout->addWidget(chartView);
}

void StatsDialog::updateStatistics()
{
    int total = m_tasks.count();
    int completed = 0;
    int todo = 0;
    
    for (const TaskItem& task : m_tasks) {
        if (task.status() == TaskItem::Completed)
            completed++;
        else
            todo++;
    }
    
    double completionRate = total > 0 ? (100.0 * completed / total) : 0.0;
    
    QString stats = QString(
        "总任务数：%1\n"
        "已完成：%2\n"
        "待办：%3\n"
        "完成率：%4%"
    ).arg(total).arg(completed).arg(todo).arg(completionRate, 0, 'f', 1);
    
    ui->statsLabel->setText(stats);
}

QMap<QString, int> StatsDialog::getTasksByCategory() const
{
    QMap<QString, int> result;
    
    for (const TaskItem& task : m_tasks) {
        QString category = task.category();
        result[category]++;
    }
    
    return result;
}

QMap<int, int> StatsDialog::getTasksByPriority() const
{
    QMap<int, int> result;
    
    for (const TaskItem& task : m_tasks) {
        result[task.priority()]++;
    }
    
    return result;
}

QMap<QDate, int> StatsDialog::getCompletionsByDate() const
{
    QMap<QDate, int> result;
    QDate today = QDate::currentDate();
    
    // 初始化最近7天
    for (int i = 6; i >= 0; --i) {
        result[today.addDays(-i)] = 0;
    }
    
    // 统计已完成任务
    for (const TaskItem& task : m_tasks) {
        if (task.status() == TaskItem::Completed && task.completedAt().isValid()) {
            QDate date = task.completedAt().date();
            if (date >= today.addDays(-6) && date <= today) {
                result[date]++;
            }
        }
    }
    
    return result;
}
