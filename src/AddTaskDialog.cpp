#include "AddTaskDialog.h"
#include "ui_AddTaskDialog.h"
#include <QMessageBox>

AddTaskDialog::AddTaskDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AddTaskDialog)
{
    ui->setupUi(this);
    
    setWindowTitle("新建任务");
    
    // 设置优先级默认值
    ui->prioritySpinBox->setValue(3);
    ui->prioritySpinBox->setMinimum(1);
    ui->prioritySpinBox->setMaximum(5);
    
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AddTaskDialog::onAccept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

AddTaskDialog::~AddTaskDialog()
{
    delete ui;
}

void AddTaskDialog::setTask(const TaskItem& task)
{
    m_task = task;
    
    ui->titleLineEdit->setText(task.title());
    ui->descriptionTextEdit->setPlainText(task.description());
    ui->categoryLineEdit->setText(task.category());
    ui->prioritySpinBox->setValue(task.priority());
}

TaskItem AddTaskDialog::getTask() const
{
    return m_task;
}

void AddTaskDialog::onAccept()
{
    QString title = ui->titleLineEdit->text().trimmed();
    
    if (title.isEmpty()) {
        QMessageBox::warning(const_cast<AddTaskDialog*>(this), "输入错误", "任务标题不能为空！");
        return;
    }
    
    m_task.setTitle(title);
    m_task.setDescription(ui->descriptionTextEdit->toPlainText());
    m_task.setCategory(ui->categoryLineEdit->text());
    m_task.setPriority(ui->prioritySpinBox->value());
    
    if (!m_task.createdAt().isValid()) {
        m_task.setCreatedAt(QDateTime::currentDateTime());
    }
    
    // 新建任务时在描述右下方自动追加时间戳
    if (m_task.id() == 0) {
        QDateTime now = QDateTime::currentDateTime();
        QString timestamp = QString("创建于：%1").arg(now.toString("yyyy-MM-dd HH:mm"));
        QString timestampLine = timestamp.rightJustified(50);
        QString desc = m_task.description();
        if (!desc.isEmpty()) {
            desc += "\n\n" + timestampLine;
        } else {
            desc = timestampLine;
        }
        m_task.setDescription(desc);
    }
    
    accept();
}
