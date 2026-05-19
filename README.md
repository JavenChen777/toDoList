# 待办事项管理程序

一个功能完整的 Windows 11 待办事项管理程序，使用 C++ 和 Qt6 框架开发。

## 功能特性

### 核心功能
- ✅ 任务的新增、编辑、删除、批量删除
- ✅ 标记任务为已完成/待办
- ✅ 任务搜索和过滤（按状态、关键词）
- ✅ 任务分类、优先级、截止日期、标签管理
- ✅ 数据持久化（JSON 格式）
- ✅ **数据安全保护** - 自动备份、损坏恢复、原子写入
- ✅ 开机自动启动
- ✅ 启动时最小化到系统托盘

### 自动弹出/隐藏
- ✅ 可配置的自动显示间隔（分钟）
- ✅ 自动隐藏延迟（秒）
- ✅ 点击窗口取消自动隐藏（进入手动模式）
- ✅ 系统托盘支持

### 统计与分析
- ✅ 任务完成趋势图（最近7天）
- ✅ 分类分布饼图
- ✅ 优先级分布柱状图
- ✅ 完成率统计

### 外部 API 接口
- ✅ **网络 API** (推荐) - HTTP REST API，支持远程访问
  - 端口 8888
  - 支持局域网内任何设备访问
  - 完整的任务字段支持（标题、描述、分类、优先级、截止日期、标签）
- ✅ 本地 API - IPC 命名管道（仅本机访问）
- ✅ JSON 格式消息
- ✅ PowerShell、Python、curl 等多种调用方式
- ✅ 详细 API 文档和示例

### 界面设计
- ✅ 贴纸风格界面（黄色渐变、圆角）
- ✅ 浅色/深色主题切换
- ✅ 自定义任务列表委托（美观显示）
- ✅ 优先级颜色编码（红色=最高，灰色=最低）
- ✅ 系统托盘图标和菜单
- ✅ 响应式布局

## 系统要求

- Windows 11 (或 Windows 10)
- Qt 6.2 或更高版本
- CMake 3.16 或更高版本
- MSVC 2019 或更高版本（或 MinGW）

## 项目结构

```
toDoList/
├── CMakeLists.txt          # CMake 构建配置
├── README.md               # 本文件
├── BUILD.md                # 详细编译说明
├── src/                    # 源代码
│   ├── main.cpp           # 程序入口
│   ├── MainWindow.h/cpp   # 主窗口
│   ├── TaskItem.h/cpp     # 任务数据结构
│   ├── TaskModel.h/cpp    # 任务列表模型
│   ├── TaskStorage.h/cpp  # 数据存储
│   ├── TaskDelegate.h/cpp # 列表项委托
│   ├── AddTaskDialog.h/cpp    # 添加/编辑任务对话框
│   ├── SettingsDialog.h/cpp   # 设置对话框
│   ├── StatsDialog.h/cpp      # 统计对话框
│   └── ApiServer.h/cpp        # API 服务器
├── ui/                     # Qt Designer UI 文件
│   ├── MainWindow.ui
│   ├── AddTaskDialog.ui
│   ├── SettingsDialog.ui
│   └── StatsDialog.ui
└── build/                  # 构建目录（自动生成）
```

## 快速开始

### 1. 安装 Qt 6

从 [Qt 官网](https://www.qt.io/download) 下载并安装 Qt 6.2 或更高版本。

推荐安装组件：
- Qt 6.x for MSVC 2019 64-bit
- Qt Charts
- CMake
- Qt Creator (可选)

### 2. 配置环境变量

确保将 Qt 的 bin 目录添加到系统 PATH，例如：
```
C:\Qt\6.5.0\msvc2019_64\bin
```

### 3. 编译项目

#### 使用命令行（推荐）

打开 "x64 Native Tools Command Prompt for VS 2019" 或 PowerShell，执行：

```powershell
# 进入项目目录
cd c:\AI\toDoList

# 创建构建目录
mkdir build
cd build

# 配置 CMake（指定 Qt 路径）
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.5.0/msvc2019_64"

# 编译
cmake --build . --config Release

# 运行
.\bin\Release\TodoList.exe
```

#### 使用 Qt Creator

1. 打开 Qt Creator
2. File -> Open File or Project
3. 选择 `CMakeLists.txt`
4. 选择 Qt Kit (MSVC 2019 64-bit)
5. 点击 "Configure Project"
6. 按 Ctrl+R 运行

### 4. 使用 PowerShell 脚本编译

我们提供了便捷的 PowerShell 脚本：

```powershell
.\build.ps1
```

## 使用说明

### 主界面操作

1. **新建任务**：点击工具栏"新建任务"按钮或按 Ctrl+N
2. **编辑任务**：双击列表中的任务
3. **删除任务**：选中任务后点击"删除任务"按钮
4. **完成任务**：点击任务左侧的复选框
5. **搜索任务**：在顶部搜索框输入关键词
6. **过滤显示**：点击"全部"、"待办"、"已完成"按钮

### 设置功能

点击"设置"按钮打开设置对话框：

- **自动模式**：启用后程序会定期自动弹出显示任务
  - 自动显示间隔：设置多少分钟后自动弹出（1-1440 分钟）
  - 自动隐藏延迟：设置弹出后多少秒自动隐藏（1-60 秒）
  - 提示：点击窗口可取消自动隐藏

- **外观**：切换深色/浅色主题

- **数据存储**：选择任务数据文件的保存路径

### 统计功能

点击"统计"按钮查看：

- **完成趋势**：最近7天的任务完成数量折线图
- **分类分布**：各分类任务的饼图
- **优先级**：不同优先级任务的柱状图

### API 接口使用

程序启动后会自动开启本地 IPC 服务器，其他程序可以通过以下方式添加任务：

#### PowerShell 示例

```powershell
# 发送新任务到待办事项程序
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream(".", "TodoListServer", [System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(1000)
$writer = New-Object System.IO.StreamWriter($pipe)
$writer.AutoFlush = $true

$task = @{
    title = "备份服务器"
    description = "每周定期备份服务器数据"
    category = "运维"
} | ConvertTo-Json -Compress

$writer.WriteLine($task)
$writer.Close()
$pipe.Close()
```

#### Python 示例

创建 `send_task.py`：

```python
import json
import socket

def send_task(title, description="", category=""):
    # Windows 命名管道路径
    pipe_name = r'\\.\pipe\TodoListServer'
    
    try:
        # 连接到命名管道
        client = socket.socket(socket.AF_UNIX if hasattr(socket, 'AF_UNIX') else socket.AF_INET)
        # 在 Windows 上需要使用特殊方式
        import win32pipe, win32file
        
        handle = win32file.CreateFile(
            pipe_name,
            win32file.GENERIC_READ | win32file.GENERIC_WRITE,
            0, None,
            win32file.OPEN_EXISTING,
            0, None
        )
        
        # 准备消息
        message = json.dumps({
            "title": title,
            "description": description,
            "category": category
        }) + "\n"
        
        # 发送消息
        win32file.WriteFile(handle, message.encode('utf-8'))
        win32file.CloseHandle(handle)
        
        print(f"任务已发送: {title}")
        
    except Exception as e:
        print(f"发送失败: {e}")

if __name__ == "__main__":
    send_task("测试任务", "这是通过 Python 添加的任务", "测试")
```

## 数据格式

任务数据保存在 JSON 文件中（默认路径：`%APPDATA%/TodoList/tasks.json`）：

```json
{
  "version": "1.0",
  "tasks": [
    {
      "id": 1,
      "title": "更新文档",
      "description": "检查并修订项目文档",
      "status": "todo",
      "created_at": "2025-11-11T10:30:00",
      "completed_at": "",
      "category": "工作",
      "priority": 3
    }
  ]
}
```

## 快捷键

- **Ctrl+N**：新建任务
- **Delete**：删除选中任务
- **Esc**：关闭对话框
- **Enter**：确认对话框

## 设置选项

通过工具栏的"设置"按钮可以配置以下选项：

### 自动模式
- **启用自动弹出/隐藏模式**：定期自动显示窗口
- **自动显示间隔**：多久自动弹出一次（1-1440分钟）
- **自动隐藏延迟**：显示后多久自动隐藏（1-60秒）

### 系统
- **开机自动启动**：Windows 启动时自动运行应用
  - 通过注册表实现（HKCU\Software\Microsoft\Windows\CurrentVersion\Run）
  - 勾选后立即生效，无需重启
- **启动时最小化到系统托盘**：启动后不显示主窗口，直接隐藏到托盘

### 外观
- **深色主题**：切换界面颜色主题

### 数据存储
- **数据文件路径**：自定义任务数据保存位置

## 故障排除

### 编译错误

1. **找不到 Qt**：确保在 CMake 命令中指定了正确的 Qt 路径
2. **Charts 模块错误**：确保安装了 Qt Charts 组件
3. **编译器错误**：使用 MSVC 2019 或更高版本

### 运行错误

1. **DLL 缺失**：将 Qt 的 bin 目录添加到 PATH
2. **配置文件错误**：删除 `%APPDATA%/TodoList` 目录重置设置

## 技术特点

- **模块化设计**：清晰的 MVC 架构
- **信号槽机制**：松耦合的组件通信
- **自定义委托**：美观的列表项显示
- **数据持久化**：可靠的 JSON 存储
- **数据保护机制**：
  - 自动备份：每次保存前创建 `.backup` 文件
  - 原子写入：通过临时文件确保数据完整性
  - 智能恢复：加载失败时自动提示从备份恢复
  - 详见 [docs/DATA_LOSS_FIX.md](docs/DATA_LOSS_FIX.md)
- **跨进程通信**：灵活的 API 接口
- **图表可视化**：Qt Charts 数据展示

## 待改进功能

- [ ] 任务提醒（弹窗/声音）
- [ ] 任务拖拽排序
- [ ] 导出任务（PDF/Excel）
- [ ] 任务标签系统
- [ ] 多用户支持
- [ ] 云同步功能

## 许可证

本项目仅供学习和参考使用。

## 作者

xxxxx

## 更新日志

### v1.0.0 (2025-11-11)
- 初始版本发布
- 实现所有核心功能
- 支持自动弹出/隐藏
- 统计图表功能
- API 接口支持
