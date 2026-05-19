# 开机启动设置说明

## 功能介绍

待办事项应用支持两种启动相关的设置：

1. **开机自动启动** - Windows 系统启动时自动运行应用
2. **启动时最小化到系统托盘** - 启动后不显示主窗口，直接隐藏到托盘

## 使用方法

### 设置开机启动

1. 启动应用程序
2. 点击工具栏的 **"设置"** 按钮（齿轮图标）
3. 在设置对话框中找到 **"系统"** 分组
4. 勾选 **"开机自动启动"** 复选框
5. 点击 **"确定"** 保存设置

✅ 设置会立即生效，无需重启应用或系统

### 设置启动时最小化

在同一个设置对话框中：
1. 勾选 **"启动时最小化到系统托盘"** 复选框
2. 点击 **"确定"** 保存

这样应用在启动时不会显示主窗口，只会在系统托盘显示图标。

## 技术实现

### Windows 注册表

开机启动通过修改 Windows 注册表实现：

**注册表位置**：
```
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
```

**键名**：`TodoList`

**键值**：应用程序的完整路径

### 手动管理（可选）

如果需要手动管理开机启动，可以使用 PowerShell：

#### 添加开机启动
```powershell
$path = "C:\AI\toDoList\build\bin\Release\TodoList.exe"
Set-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" -Name "TodoList" -Value $path
```

#### 删除开机启动
```powershell
Remove-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" -Name "TodoList"
```

#### 检查当前状态
```powershell
Get-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" -Name "TodoList" -ErrorAction SilentlyContinue
```

## 验证设置

### 方法 1：使用应用界面
1. 打开设置对话框
2. 查看"开机自动启动"复选框状态
3. 如果已勾选，说明已启用

### 方法 2：检查注册表
运行测试脚本：
```powershell
.\test_autostart.ps1
```

或手动检查：
```powershell
Get-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" -Name "TodoList"
```

### 方法 3：重启电脑
重启 Windows 系统，观察应用是否自动启动

## 权限说明

- ✅ **不需要管理员权限** - 修改 HKCU 注册表项不需要提升权限
- ✅ **仅影响当前用户** - 设置只对当前登录的用户有效
- ✅ **安全可靠** - 使用 Windows 标准机制，不会影响系统稳定性

## 注意事项

1. **路径变更**：如果移动了应用程序，需要重新设置开机启动
2. **杀毒软件**：某些安全软件可能会提示注册表修改，请允许
3. **多用户系统**：每个用户需要独立设置，不会影响其他用户
4. **启动延迟**：开机启动可能有几秒延迟，这是正常现象

## 故障排除

### 设置失败

**症状**：勾选后提示"无法修改开机启动设置"

**解决方案**：
1. 确保应用程序路径有效
2. 检查是否被安全软件阻止
3. 尝试以管理员身份运行应用
4. 手动使用 PowerShell 添加注册表项

### 开机没有自动启动

**检查清单**：
1. ✓ 确认注册表中有对应项
2. ✓ 确认应用程序路径正确
3. ✓ 检查是否被启动管理工具禁用
4. ✓ 查看 Windows 任务管理器 → 启动 选项卡

### 启动后不显示窗口

这可能是因为启用了"启动时最小化到系统托盘"：
1. 查看系统托盘区域
2. 找到应用图标并点击
3. 或在设置中取消"启动时最小化到系统托盘"

## 相关功能

配合开机启动，您还可以使用：
- **自动弹出/隐藏模式** - 定期提醒待办事项
- **系统托盘** - 最小化到托盘，不占用任务栏空间
- **网络 API** - 从其他设备添加任务

## 卸载说明

如需完全卸载应用并移除开机启动：

```powershell
# 1. 删除开机启动项
Remove-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" -Name "TodoList" -ErrorAction SilentlyContinue

# 2. 删除配置文件
Remove-Item -Path "$env:APPDATA\TodoList" -Recurse -Force -ErrorAction SilentlyContinue

# 3. 删除应用程序文件
# 手动删除应用程序目录
```

---

**提示**：如有任何问题，请查看主 README.md 或提交 Issue。
