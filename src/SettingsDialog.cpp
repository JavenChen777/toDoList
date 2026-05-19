#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    
    setWindowTitle("设置");
    
    loadSettings();
    
    connect(ui->browseButton, &QPushButton::clicked, this, &SettingsDialog::onBrowseDataPath);
    connect(ui->resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetDefaults);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // 连接开机启动复选框信号
    connect(ui->autoStartCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        if (!setAutoStartRegistry(checked)) {
            // 如果设置失败，恢复复选框状态
            ui->autoStartCheckBox->blockSignals(true);
            ui->autoStartCheckBox->setChecked(!checked);
            ui->autoStartCheckBox->blockSignals(false);
            QMessageBox::warning(this, "设置失败", "无法修改开机启动设置。请确保您有足够的权限。");
        }
    });
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setAutoMode(bool enabled)
{
    ui->autoModeCheckBox->setChecked(enabled);
}

bool SettingsDialog::autoMode() const
{
    return ui->autoModeCheckBox->isChecked();
}

void SettingsDialog::setAutoShowInterval(int minutes)
{
    ui->autoShowIntervalSpinBox->setValue(minutes);
}

int SettingsDialog::autoShowInterval() const
{
    return ui->autoShowIntervalSpinBox->value();
}

void SettingsDialog::setAutoHideDelay(int seconds)
{
    ui->autoHideDelaySpinBox->setValue(seconds);
}

int SettingsDialog::autoHideDelay() const
{
    return ui->autoHideDelaySpinBox->value();
}

void SettingsDialog::setDarkMode(bool enabled)
{
    ui->darkModeCheckBox->setChecked(enabled);
}

bool SettingsDialog::darkMode() const
{
    return ui->darkModeCheckBox->isChecked();
}

void SettingsDialog::setDataPath(const QString& path)
{
    ui->dataPathLineEdit->setText(path);
}

QString SettingsDialog::dataPath() const
{
    return ui->dataPathLineEdit->text();
}

void SettingsDialog::setAutoStart(bool enabled)
{
    ui->autoStartCheckBox->setChecked(enabled);
}

bool SettingsDialog::autoStart() const
{
    return ui->autoStartCheckBox->isChecked();
}

void SettingsDialog::setStartMinimized(bool enabled)
{
    ui->startMinimizedCheckBox->setChecked(enabled);
}

bool SettingsDialog::startMinimized() const
{
    return ui->startMinimizedCheckBox->isChecked();
}

bool SettingsDialog::setAutoStartRegistry(bool enable)
{
#ifdef Q_OS_WIN
    HKEY hKey;
    LONG result;
    
    // 打开注册表键
    result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_WRITE,
        &hKey
    );
    
    if (result != ERROR_SUCCESS) {
        qWarning() << "Failed to open registry key";
        return false;
    }
    
    QString appName = "TodoList";
    QString appPath = QCoreApplication::applicationFilePath().replace('/', '\\');
    
    if (enable) {
        // 添加开机启动
        std::wstring path = appPath.toStdWString();
        result = RegSetValueExW(
            hKey,
            appName.toStdWString().c_str(),
            0,
            REG_SZ,
            (const BYTE*)path.c_str(),
            (path.length() + 1) * sizeof(wchar_t)
        );
    } else {
        // 删除开机启动
        result = RegDeleteValueW(hKey, appName.toStdWString().c_str());
        if (result == ERROR_FILE_NOT_FOUND) {
            result = ERROR_SUCCESS; // 如果项不存在，视为成功
        }
    }
    
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
#else
    Q_UNUSED(enable);
    return false;
#endif
}

bool SettingsDialog::isAutoStartEnabled()
{
#ifdef Q_OS_WIN
    HKEY hKey;
    LONG result;
    
    result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_READ,
        &hKey
    );
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    QString appName = "TodoList";
    wchar_t value[MAX_PATH];
    DWORD size = sizeof(value);
    
    result = RegQueryValueExW(
        hKey,
        appName.toStdWString().c_str(),
        NULL,
        NULL,
        (LPBYTE)value,
        &size
    );
    
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
#else
    return false;
#endif
}

void SettingsDialog::loadSettings()
{
    QSettings settings("TodoList", "TodoListApp");
    
    ui->autoModeCheckBox->setChecked(settings.value("autoMode", false).toBool());
    ui->autoShowIntervalSpinBox->setValue(settings.value("autoShowInterval", 30).toInt());
    ui->autoHideDelaySpinBox->setValue(settings.value("autoHideDelay", 5).toInt());
    ui->darkModeCheckBox->setChecked(settings.value("darkMode", false).toBool());
    ui->startMinimizedCheckBox->setChecked(settings.value("startMinimized", false).toBool());
    
    // 检查注册表中的开机启动状态
    ui->autoStartCheckBox->blockSignals(true);
    ui->autoStartCheckBox->setChecked(isAutoStartEnabled());
    ui->autoStartCheckBox->blockSignals(false);
    
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tasks.json";
    ui->dataPathLineEdit->setText(settings.value("dataPath", defaultPath).toString());
}

void SettingsDialog::saveSettings()
{
    QSettings settings("TodoList", "TodoListApp");
    
    settings.setValue("autoMode", ui->autoModeCheckBox->isChecked());
    settings.setValue("autoShowInterval", ui->autoShowIntervalSpinBox->value());
    settings.setValue("autoHideDelay", ui->autoHideDelaySpinBox->value());
    settings.setValue("darkMode", ui->darkModeCheckBox->isChecked());
    settings.setValue("dataPath", ui->dataPathLineEdit->text());
    settings.setValue("startMinimized", ui->startMinimizedCheckBox->isChecked());
    
    // 开机启动通过注册表管理，不需要保存到 QSettings
}

void SettingsDialog::onBrowseDataPath()
{
    QString currentPath = ui->dataPathLineEdit->text();
    QString dir = QFileDialog::getExistingDirectory(this, "选择数据存储目录",
                                                    currentPath,
                                                    QFileDialog::ShowDirsOnly);
    
    if (!dir.isEmpty()) {
        ui->dataPathLineEdit->setText(dir + "/tasks.json");
    }
}

void SettingsDialog::onAccept()
{
    saveSettings();
    accept();
}

void SettingsDialog::onResetDefaults()
{
    ui->autoModeCheckBox->setChecked(false);
    ui->autoShowIntervalSpinBox->setValue(30);
    ui->autoHideDelaySpinBox->setValue(5);
    ui->darkModeCheckBox->setChecked(false);
    
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tasks.json";
    ui->dataPathLineEdit->setText(defaultPath);
}
