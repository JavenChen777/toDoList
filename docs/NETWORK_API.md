# 网络 API 使用指南

## 概述

待办事项应用提供了网络 API 接口，默认只允许本机通过 HTTP 请求添加任务。

## 快速开始

### 1. 启动应用

在运行应用的机器上启动 `TodoList.exe`。应用会自动在本机 `127.0.0.1` 的 **8888** 端口启动网络 API 服务器。

### 2. 从本机发送请求

## API 规范

### 端点

```
POST http://127.0.0.1:8888/api/tasks
```

### 请求头

```
Content-Type: application/json; charset=utf-8
```

### 请求体 (JSON)

#### 必填字段

- `title` (string): 任务标题

#### 可选字段

- `description` (string): 任务描述
- `category` (string): 分类（工作/个人/学习/生活/其他）
- `priority` (number): 优先级（1-5，5最高，默认3）
- `dueDate` (string): 截止日期，ISO 8601 格式（YYYY-MM-DDTHH:mm:ss）
- `tags` (array): 标签数组

### 示例请求

```json
{
  "title": "完成项目报告",
  "description": "撰写Q4季度总结报告",
  "category": "工作",
  "priority": 5,
  "dueDate": "2025-11-15T18:00:00",
  "tags": ["重要", "紧急"]
}
```

### 响应

#### 成功响应 (200 OK)

```json
{
  "success": true,
  "message": "Task added successfully",
  "task": {
    "title": "完成项目报告",
    "description": "撰写Q4季度总结报告",
    "category": "工作",
    "priority": 5,
    "dueDate": "2025-11-15T18:00:00",
    "tags": ["重要", "紧急"]
  }
}
```

#### 错误响应 (400/405/500)

```json
{
  "success": false,
  "error": "错误信息"
}
```

## 使用示例

### PowerShell (Windows)

```powershell
# 设置服务器地址
$serverIp = "127.0.0.1"
$apiUrl = "http://${serverIp}:8888/api/tasks"

# 创建任务数据
$task = @{
    title = "完成项目报告"
    description = "撰写Q4季度总结报告"
    category = "工作"
    priority = 5
    dueDate = "2025-11-15T18:00:00"
    tags = @("重要", "紧急")
} | ConvertTo-Json -Compress

# 发送请求
$response = Invoke-WebRequest -Uri $apiUrl -Method Post -Body $task `
    -ContentType "application/json; charset=utf-8"

# 显示结果
$response.Content | ConvertFrom-Json | ConvertTo-Json -Depth 3
```

运行示例脚本：
```powershell
.\examples\send_task_http.ps1
```

### Python

```python
import requests

# 设置服务器地址
SERVER_IP = "127.0.0.1"
API_URL = f"http://{SERVER_IP}:8888/api/tasks"

# 创建任务数据
task = {
    "title": "学习新技术",
    "description": "学习 Qt 6 框架",
    "category": "学习",
    "priority": 4,
    "tags": ["编程", "Qt"]
}

# 发送请求
response = requests.post(API_URL, json=task)

# 显示结果
print(response.json())
```

运行示例脚本：
```bash
python examples/send_task_http.py
```

### curl (跨平台)

```bash
curl -X POST http://127.0.0.1:8888/api/tasks \
  -H "Content-Type: application/json" \
  -d '{"title":"测试任务","description":"测试描述","category":"测试","priority":3}'
```

### JavaScript (Node.js)

```javascript
const fetch = require('node-fetch');

const SERVER_IP = '127.0.0.1';
const API_URL = `http://${SERVER_IP}:8888/api/tasks`;

const task = {
  title: '开发新功能',
  description: '实现用户管理模块',
  category: '工作',
  priority: 4,
  dueDate: '2025-11-20T12:00:00',
  tags: ['开发', '功能']
};

fetch(API_URL, {
  method: 'POST',
  headers: { 'Content-Type': 'application/json; charset=utf-8' },
  body: JSON.stringify(task)
})
.then(res => res.json())
.then(data => console.log(data))
.catch(err => console.error(err));
```

## 安全注意事项

⚠️ **重要提示**

- **无身份验证**：网络 API 没有身份验证机制
- **默认仅本机**：服务默认监听 `127.0.0.1`，避免局域网内其他设备直接写入
- **不要暴露到公网**：如果未来改为监听局域网地址，应先增加认证或访问控制

### 远程访问

当前版本默认不开放局域网访问。如需恢复远程访问，建议先增加 token 或其他认证机制，再改为监听指定网卡地址。

## 集成场景

### 1. 邮件客户端集成

在邮件客户端（如 Outlook）中设置规则，自动将特定邮件转换为任务：

```python
# 示例：将带有特定标签的邮件转为任务
import imaplib
import email
import requests

# 连接邮箱，读取邮件...
# 发送到待办 API
task = {
    "title": email_subject,
    "description": email_body,
    "category": "工作",
    "priority": 4
}
requests.post("http://127.0.0.1:8888/api/tasks", json=task)
```

### 2. CI/CD 集成

在 CI/CD 流程中添加待办提醒：

```bash
# 构建失败时创建任务
if [ $BUILD_STATUS == "failed" ]; then
  curl -X POST http://127.0.0.1:8888/api/tasks \
    -H "Content-Type: application/json" \
    -d "{\"title\":\"构建失败\",\"description\":\"项目 $PROJECT_NAME 构建失败\",\"priority\":5}"
fi
```

### 3. 定时任务

使用 Windows 任务计划程序定期创建重复性任务：

```powershell
# scheduled_task.ps1
$task = @{
    title = "每周总结"
    description = "撰写本周工作总结"
    category = "工作"
    priority = 3
} | ConvertTo-Json -Compress

Invoke-WebRequest -Uri "http://127.0.0.1:8888/api/tasks" `
    -Method Post -Body $task -ContentType "application/json"
```

### 4. IoT 设备集成

从智能家居设备或传感器创建任务：

```python
# 示例：温度传感器异常时创建任务
if temperature > threshold:
    task = {
        "title": "检查服务器温度",
        "description": f"服务器温度过高: {temperature}°C",
        "category": "维护",
        "priority": 5
    }
    requests.post("http://127.0.0.1:8888/api/tasks", json=task)
```

## 故障排除

### 无法连接

1. **检查应用是否运行**：确保 TodoList.exe 正在运行
2. **检查地址**：确认使用 `http://127.0.0.1:8888/api/tasks`
3. **检查端口占用**：确认 8888 端口未被其他程序占用
4. **确认请求来自本机**：当前版本默认不接受其他机器连接
5. **测试连接**：
   ```powershell
   Test-NetConnection -ComputerName 127.0.0.1 -Port 8888
   ```

### 请求失败

1. **检查 JSON 格式**：确保 JSON 格式正确
2. **检查编码**：使用 UTF-8 编码
3. **查看错误信息**：检查响应中的 error 字段

### 中文乱码

确保请求使用 UTF-8 编码：

```powershell
# PowerShell
-ContentType "application/json; charset=utf-8"
```

```python
# Python
headers = {'Content-Type': 'application/json; charset=utf-8'}
```

## 本地 API vs 网络 API

| 特性 | 本地 API (命名管道) | 网络 API (HTTP) |
|------|-------------------|----------------|
| 访问范围 | 仅本机 | 默认仅本机 |
| 协议 | Windows 命名管道 | HTTP/TCP |
| 端口 | 无 | 8888 |
| 使用难度 | 较复杂 | 简单 |
| 跨平台 | 仅 Windows | 所有平台 |
| 推荐场景 | 本机进程间通信 | 本机 HTTP 集成、脚本调用 |

**推荐使用网络 API**，因为它更简单、更通用。

## 更多信息

- 查看 `examples/` 目录获取更多示例脚本
- 在应用中点击 "帮助" 查看完整 API 文档
- API 服务器启动信息会显示在应用日志中

---

**注意**：网络 API 默认仅绑定 `127.0.0.1`；请勿在缺少认证的情况下暴露到不安全网络。
