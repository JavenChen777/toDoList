#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>

namespace Ui {
class HelpDialog;
}

/**
 * @brief 帮助对话框
 * 
 * 显示程序使用帮助和API文档
 */
class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = nullptr);
    ~HelpDialog();

private slots:
    void onTabChanged(int index);
    void onCopyExample();

private:
    Ui::HelpDialog *ui;
    
    void setupContent();
    void loadBasicHelp();
    void loadApiHelp();
    void loadFaqHelp();
};

#endif // HELPDIALOG_H
