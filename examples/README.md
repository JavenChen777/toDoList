# 待办事项管理程序 - API 使用示例

本目录包含了如何通过 API 与待办事项程序交互的示例脚本。

## 文件说明

### send_task.ps1
PowerShell 脚本，演示如何通过命名管道发送任务。

**使用方法：**

```powershell
# 方法 1: 运行完整演示
.\send_task.ps1

# 方法 2: 作为模块导入并使用函数
. .\send_task.ps1
Send-Task -Title "我的任务" -Description "任务描述" -Category "工作" -Priority 5
```

### send_task.py
Python 脚本，演示如何通过命名管道发送任务。

**前置要求：**
```bash
pip install pywin32
```

**使用方法：**

```bash
# 方法 1: 运行完整演示
python send_task.py

# 方法 2: 命令行参数
python send_task.py "任务标题" "任务描述" "分类"
```

### tasks_example.json
示例任务数据文件，展示 JSON 数据格式。

可以复制到应用数据目录进行测试：
```powershell
Copy-Item tasks_example.json $env:APPDATA\TodoList\tasks.json
```

## API 协议说明

### 连接方式
- **协议**: Windows 命名管道
- **管道名**: `\\.\pipe\TodoListServer`
- **通信方式**: 文本行（每条消息以换行符结束）

### 消息格式

发送 JSON 格式的消息：

```json
{
  "title": "任务标题",
  "description": "任务描述（可选）",
  "category": "分类（可选）",
  "priority": 3
}
```

**字段说明：**

- `title` (必需): 任务标题，字符串
- `description` (可选): 任务描述，字符串，默认为空
- `category` (可选): 任务分类，字符串，默认为空
- `priority` (可选): 优先级，整数 1-5，默认为 3

### 响应
当前版本不返回响应消息，发送后即可断开连接。

## 其他语言示例

### C# 示例

```csharp
using System;
using System.IO;
using System.IO.Pipes;
using System.Text;
using System.Text.Json;

class Program
{
    static void SendTask(string title, string description = "", 
                        string category = "", int priority = 3)
    {
        using (var pipe = new NamedPipeClientStream(".", "TodoListServer", 
                                                     PipeDirection.InOut))
        {
            pipe.Connect(3000);
            
            var task = new
            {
                title = title,
                description = description,
                category = category,
                priority = priority
            };
            
            string json = JsonSerializer.Serialize(task);
            byte[] data = Encoding.UTF8.GetBytes(json + "\n");
            
            pipe.Write(data, 0, data.Length);
            pipe.Flush();
        }
        
        Console.WriteLine($"✓ 任务已发送: {title}");
    }
    
    static void Main()
    {
        SendTask("测试任务", "这是通过 C# 添加的任务", "测试", 4);
    }
}
```

### Node.js 示例

```javascript
const net = require('net');

function sendTask(title, description = '', category = '', priority = 3) {
    // Windows 命名管道路径
    const pipePath = '\\\\.\\pipe\\TodoListServer';
    
    const client = net.connect(pipePath, () => {
        const task = {
            title,
            description,
            category,
            priority
        };
        
        const json = JSON.stringify(task) + '\n';
        client.write(json);
        client.end();
        
        console.log(`✓ 任务已发送: ${title}`);
    });
    
    client.on('error', (err) => {
        console.error('发送失败:', err.message);
    });
}

// 使用示例
sendTask('测试任务', '这是通过 Node.js 添加的任务', '测试', 4);
```

### VBScript 示例

```vbscript
' 注意: VBScript 访问命名管道比较复杂，建议使用 PowerShell

Set objShell = CreateObject("WScript.Shell")
Set objFSO = CreateObject("Scripting.FileSystemObject")

' 创建临时 PowerShell 脚本
strScript = "$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.', 'TodoListServer', [System.IO.Pipes.PipeDirection]::InOut);" & _
            "$pipe.Connect(1000);" & _
            "$writer = New-Object System.IO.StreamWriter($pipe);" & _
            "$writer.AutoFlush = $true;" & _
            "$task = @{title='测试任务';description='VBScript测试';category='测试';priority=3} | ConvertTo-Json -Compress;" & _
            "$writer.WriteLine($task);" & _
            "$writer.Close();" & _
            "$pipe.Close()"

' 执行 PowerShell
objShell.Run "powershell.exe -Command """ & strScript & """", 0, True
WScript.Echo "任务已发送"
```

## 集成建议

### 1. 定时任务集成

使用 Windows 任务计划程序定期添加任务：

```powershell
# 创建任务计划
$action = New-ScheduledTaskAction -Execute "powershell.exe" `
    -Argument "-File C:\path\to\send_task.ps1"

$trigger = New-ScheduledTaskTrigger -Daily -At 9AM

Register-ScheduledTask -TaskName "每日任务提醒" `
    -Action $action -Trigger $trigger
```

### 2. 监控脚本集成

在系统监控脚本中，当检测到问题时自动创建任务：

```powershell
# 监控磁盘空间
$disk = Get-PSDrive C
if ($disk.Free / $disk.Used -lt 0.1) {
    Send-Task -Title "磁盘空间不足" `
              -Description "C 盘剩余空间低于 10%" `
              -Category "系统维护" `
              -Priority 5
}
```

### 3. 邮件触发集成

从邮件中提取任务并添加到待办列表。

### 4. 语音助手集成

结合语音识别，通过语音添加任务。

## 故障排除

### 错误: 无法连接到待办事项程序

**可能原因：**
1. 程序未运行
2. API 服务器未启动

**解决方法：**
1. 确保 TodoList.exe 正在运行
2. 检查程序是否成功启动了 API 服务器（查看控制台输出）

### 错误: 访问被拒绝

**可能原因：**
权限不足

**解决方法：**
以管理员身份运行脚本或程序

### 错误: JSON 解析失败

**可能原因：**
消息格式不正确

**解决方法：**
确保 JSON 格式正确，特殊字符需要转义

## 性能说明

- 单次连接延迟: < 10ms
- 支持并发连接: 是
- 最大消息长度: 无限制（建议 < 10KB）

## 安全注意事项

1. 命名管道默认仅限本地访问
2. 不需要身份验证（适用于单用户环境）
3. 如需在多用户环境使用，建议添加身份验证机制

## 更多信息

查看主项目 README.md 了解更多功能和使用说明。
