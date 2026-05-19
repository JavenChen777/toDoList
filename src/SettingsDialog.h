#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

/**
 * @brief 设置对话框
 * 
 * 管理应用程序设置，包括自动模式、主题等
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

    // 设置值的 getter 和 setter
    void setAutoMode(bool enabled);
    bool autoMode() const;

    void setAutoShowInterval(int minutes);
    int autoShowInterval() const;

    void setAutoHideDelay(int seconds);
    int autoHideDelay() const;

    void setDarkMode(bool enabled);
    bool darkMode() const;

    void setDataPath(const QString& path);
    QString dataPath() const;

    void setAutoStart(bool enabled);
    bool autoStart() const;

    void setStartMinimized(bool enabled);
    bool startMinimized() const;

private slots:
    void onBrowseDataPath();
    void onAccept();
    void onResetDefaults();

private:
    Ui::SettingsDialog* ui;
    
    void loadSettings();
    void saveSettings();
    bool setAutoStartRegistry(bool enable);
    bool isAutoStartEnabled();
};

#endif // SETTINGSDIALOG_H
