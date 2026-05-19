#include "HelpDialog.h"
#include "ui_HelpDialog.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HelpDialog)
{
    ui->setupUi(this);
    setWindowTitle("使用帮助");
    setupContent();
    
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &HelpDialog::onTabChanged);
    connect(ui->btnCopyExample, &QPushButton::clicked, this, &HelpDialog::onCopyExample);
}

HelpDialog::~HelpDialog()
{
    delete ui;
}

void HelpDialog::setupContent()
{
    loadBasicHelp();
    loadApiHelp();
    loadFaqHelp();
}

void HelpDialog::loadBasicHelp()
{
    QString html = R"(
<!DOCTYPE html>
<html>
<head>
<style>
body { font-family: "Microsoft YaHei", "Segoe UI", Arial; padding: 20px; }
h2 { color: #2196F3; border-bottom: 2px solid #2196F3; padding-bottom: 5px; }
h3 { color: #666; margin-top: 20px; }
.feature { background: #f5f5f5; padding: 10px; margin: 10px 0; border-radius: 5px; }
.feature strong { color: #2196F3; }
kbd { 
    background: #eee; 
    border: 1px solid #ccc; 
    border-radius: 3px; 
    padding: 2px 5px; 
    font-family: monospace;
}
ul { line-height: 1.8; }
</style>
</head>
<body>

<h2>📋 基本功能</h2>

<h3>任务管理</h3>
<div class="feature">
<strong>添加任务：</strong>点击工具栏的 "➕ 添加任务" 按钮，或使用快捷键 <kbd>Ctrl+N</kbd><br>
<strong>编辑任务：</strong>双击任务列表中的任务<br>
<strong>删除任务：</strong>选中任务后点击 "🗑️ 删除" 按钮，或按 <kbd>Delete</kbd> 键<br>
<strong>完成任务：</strong>点击任务前的复选框，或选中后按 <kbd>Space</kbd> 键
</div>

<h3>搜索和过滤</h3>
<div class="feature">
<strong>搜索：</strong>在顶部搜索框输入关键词，可搜索标题、描述、分类和标签<br>
<strong>分类过滤：</strong>使用分类下拉框筛选特定分类的任务<br>
<strong>优先级过滤：</strong>使用优先级下拉框筛选不同优先级的任务<br>
<strong>显示已完成：</strong>勾选 "显示已完成" 查看已完成的任务
</div>

<h3>自动显示/隐藏</h3>
<div class="feature">
在 <strong>设置</strong> 中启用 "自动显示/隐藏" 功能：
<ul>
<li>程序会定时自动弹出提醒您待办事项</li>
<li>可自定义显示间隔（默认 30 分钟）</li>
<li>可自定义自动隐藏延迟（默认 5 秒）</li>
<li>点击窗口后进入手动模式，不会自动隐藏</li>
</ul>
</div>

<h3>统计图表</h3>
<div class="feature">
点击 <strong>"📊 统计"</strong> 查看：
<ul>
<li><strong>任务完成趋势：</strong>过去 7 天的任务完成情况折线图</li>
<li><strong>分类分布：</strong>不同分类的任务数量饼图</li>
<li><strong>优先级分布：</strong>不同优先级的任务数量柱状图</li>
</ul>
</div>

<h3>系统托盘</h3>
<div class="feature">
关闭窗口后程序最小化到系统托盘，右键托盘图标可：
<ul>
<li>显示/隐藏主窗口</li>
<li>快速添加任务</li>
<li>退出程序</li>
</ul>
</div>

<h2>⌨️ 快捷键</h2>
<div class="feature">
<kbd>Ctrl+N</kbd> - 添加新任务<br>
<kbd>Delete</kbd> - 删除选中的任务<br>
<kbd>Space</kbd> - 切换任务完成状态<br>
<kbd>Ctrl+F</kbd> - 聚焦到搜索框<br>
<kbd>F1</kbd> - 显示帮助（本窗口）
</div>

</body>
</html>
)";
    ui->textBasicHelp->setHtml(html);
}

void HelpDialog::loadApiHelp()
{
    QString html = R"(
<!DOCTYPE html>
<html>
<head>
<style>
body { font-family: "Microsoft YaHei", "Segoe UI", Arial; padding: 20px; }
h2 { color: #4CAF50; border-bottom: 2px solid #4CAF50; padding-bottom: 5px; }
h3 { color: #666; margin-top: 20px; }
.api-box { 
    background: #f5f5f5; 
    padding: 15px; 
    margin: 10px 0; 
    border-radius: 5px;
    border-left: 4px solid #4CAF50;
}
.network-box {
    background: #e3f2fd;
    padding: 15px;
    margin: 10px 0;
    border-radius: 5px;
    border-left: 4px solid #2196F3;
}
.code { 
    background: #2d2d2d; 
    color: #f8f8f2; 
    padding: 15px; 
    border-radius: 5px; 
    font-family: 'Consolas', 'Monaco', monospace;
    margin: 10px 0;
    overflow-x: auto;
}
.keyword { color: #ff79c6; }
.string { color: #50fa7b; }
.comment { color: #6272a4; }
.number { color: #bd93f9; }
ul { line-height: 1.8; }
strong { color: #4CAF50; }
.warning { background: #fff3cd; padding: 10px; border-left: 4px solid #ffc107; margin: 10px 0; }
</style>
</head>
<body>

<h2>🔌 API 接口说明</h2>

<h3>📡 网络 API（推荐 - 支持远程访问）</h3>

<div class="network-box">
<strong>接口地址：</strong>http://[服务器IP]:8888/api/tasks<br>
<strong>请求方法：</strong>HTTP POST<br>
<strong>数据格式：</strong>JSON<br>
<strong>编码：</strong>UTF-8<br>
<strong>适用场景：</strong>远程机器访问、局域网内其他设备、集成到 Web 应用
</div>

<div class="code">
<span class="comment"># PowerShell 示例（适用于本机或远程）</span>
<span class="keyword">$serverIp</span> = <span class="string">"192.168.1.100"</span>  <span class="comment"># 运行应用的机器IP</span>
<span class="keyword">$apiUrl</span> = <span class="string">"http://${serverIp}:8888/api/tasks"</span>

<span class="keyword">$task</span> = @{
    title = <span class="string">"完成项目报告"</span>
    description = <span class="string">"撰写Q4季度总结报告"</span>
    category = <span class="string">"工作"</span>
    priority = <span class="number">5</span>  <span class="comment"># 1-5，5最高</span>
    dueDate = <span class="string">"2025-11-15T18:00:00"</span>
    tags = @(<span class="string">"重要"</span>, <span class="string">"紧急"</span>)
} | ConvertTo-Json -Compress

Invoke-WebRequest -Uri <span class="keyword">$apiUrl</span> -Method Post -Body <span class="keyword">$task</span> `
    -ContentType <span class="string">"application/json; charset=utf-8"</span>
</div>

<div class="code">
<span class="comment"># Python 示例（需要 requests 库）</span>
<span class="keyword">import</span> requests
<span class="keyword">import</span> json

SERVER_IP = <span class="string">"192.168.1.100"</span>  <span class="comment"># 运行应用的机器IP</span>
API_URL = <span class="string">f"http://{SERVER_IP}:8888/api/tasks"</span>

task = {
    <span class="string">"title"</span>: <span class="string">"学习新技术"</span>,
    <span class="string">"description"</span>: <span class="string">"学习 Qt 6 框架"</span>,
    <span class="string">"category"</span>: <span class="string">"学习"</span>,
    <span class="string">"priority"</span>: <span class="number">4</span>,
    <span class="string">"tags"</span>: [<span class="string">"编程"</span>, <span class="string">"Qt"</span>]
}

response = requests.post(API_URL, json=task)
<span class="keyword">print</span>(response.json())
</div>

<div class="code">
<span class="comment"># curl 示例（跨平台）</span>
curl -X POST http://192.168.1.100:8888/api/tasks \
  -H <span class="string">"Content-Type: application/json"</span> \
  -d <span class="string">'{"title":"测试任务","description":"测试描述","category":"测试","priority":3}'</span>
</div>

<h3>🖥️ 本地 API（仅本机访问）</h3>

<div class="api-box">
<strong>接口地址：</strong>本地命名管道 <code>TodoListServer</code><br>
<strong>数据格式：</strong>JSON<br>
<strong>编码：</strong>UTF-8<br>
<strong>适用场景：</strong>同一台机器上的进程间通信
</div>

<div class="code">
<span class="comment"># PowerShell 本地管道示例</span>
<span class="keyword">$task</span> = @{
    action = <span class="string">"add"</span>
    title = <span class="string">"本地任务"</span>
    description = <span class="string">"通过命名管道添加"</span>
    category = <span class="string">"工作"</span>
} | ConvertTo-Json -Compress

<span class="keyword">$stream</span> = [System.IO.Pipes.NamedPipeClientStream]::new(<span class="string">"."</span>, <span class="string">"TodoListServer"</span>)
<span class="keyword">$stream</span>.Connect(<span class="number">5000</span>)
<span class="keyword">$writer</span> = [System.IO.StreamWriter]::new(<span class="keyword">$stream</span>)
<span class="keyword">$writer</span>.WriteLine(<span class="keyword">$task</span>)
<span class="keyword">$writer</span>.Flush()
<span class="keyword">$stream</span>.Close()
</div>

<h3>📋 JSON 字段说明</h3>

<div class="api-box">
<strong>必填字段：</strong>
<ul>
<li><code>title</code> (string): 任务标题</li>
</ul>

<strong>可选字段：</strong>
<ul>
<li><code>description</code> (string): 任务描述</li>
<li><code>category</code> (string): 分类（工作/个人/学习/生活/其他）</li>
<li><code>priority</code> (number): 优先级（1-5，5最高，默认3）</li>
<li><code>dueDate</code> (string): 截止日期，ISO 8601 格式：YYYY-MM-DDTHH:mm:ss</li>
<li><code>tags</code> (array): 标签数组</li>
</ul>

<strong>仅本地 API 需要：</strong>
<ul>
<li><code>action</code> (string): 操作类型，目前仅支持 "add"</li>
</ul>
</div>

<h3>📂 示例脚本位置</h3>

<div class="api-box">
项目目录下的 <code>examples/</code> 文件夹包含完整示例：
<ul>
<li><strong>send_task_http.ps1</strong> - PowerShell HTTP 网络请求示例</li>
<li><strong>send_task_http.py</strong> - Python HTTP 网络请求示例</li>
<li><strong>send_task.ps1</strong> - PowerShell 本地管道示例</li>
<li><strong>send_task.py</strong> - Python 本地管道示例</li>
<li><strong>tasks_example.json</strong> - JSON 数据示例</li>
</ul>
</div>

<h3>🔒 安全注意事项</h3>

<div class="warning">
<strong>⚠️ 重要提示：</strong>
<ul>
<li>网络 API <strong>没有身份验证</strong>，仅建议在<strong>可信的局域网</strong>内使用</li>
<li>请勿将服务暴露到公网，避免安全风险</li>
<li>如需公网访问，建议配置防火墙规则或使用 VPN</li>
<li>端口 8888 可能被防火墙阻止，需要添加防火墙规则允许访问</li>
</ul>
</div>

<h3>🌐 获取服务器 IP 地址</h3>

<div class="api-box">
在运行应用的机器上，打开 PowerShell 或命令提示符：
<ul>
<li><code>ipconfig</code> (Windows)</li>
<li>查找 "IPv4 地址" 或 "IPv4 Address"</li>
<li>例如：192.168.1.100</li>
</ul>
应用启动时会在日志中显示访问地址。
</div>

<h3>🚀 高级用法</h3>

<div class="api-box">
<strong>批量导入：</strong>通过脚本循环调用 API 批量添加任务<br>
<strong>定时任务：</strong>配合系统任务计划程序，定时添加重复性任务<br>
<strong>集成工具：</strong>集成到其他应用（邮件客户端、项目管理工具、Web服务）<br>
<strong>跨平台：</strong>可从 Linux、macOS、移动设备等任何支持 HTTP 的平台调用
</div>

</body>
</html>
)";
    ui->textApiHelp->setHtml(html);
}

void HelpDialog::loadFaqHelp()
{
    QString html = R"(
<!DOCTYPE html>
<html>
<head>
<style>
body { font-family: "Microsoft YaHei", "Segoe UI", Arial; padding: 20px; }
h2 { color: #FF9800; border-bottom: 2px solid #FF9800; padding-bottom: 5px; }
.faq-item { 
    margin: 20px 0;
    padding: 15px;
    background: #f9f9f9;
    border-radius: 5px;
    border-left: 4px solid #FF9800;
}
.question { 
    font-weight: bold; 
    color: #FF9800; 
    font-size: 16px;
    margin-bottom: 10px;
}
.answer { 
    color: #333; 
    line-height: 1.8;
}
code { 
    background: #eee; 
    padding: 2px 6px; 
    border-radius: 3px; 
    font-family: 'Consolas', monospace;
}
</style>
</head>
<body>

<h2>❓ 常见问题</h2>

<div class="faq-item">
<div class="question">Q1: 任务数据保存在哪里？</div>
<div class="answer">
任务数据以 JSON 格式保存在：<br>
<code>C:\Users\[用户名]\AppData\Local\TodoList\tasks.json</code><br>
您可以手动备份此文件，或在其他电脑上恢复数据。
</div>
</div>

<div class="faq-item">
<div class="question">Q2: 如何备份我的任务数据？</div>
<div class="answer">
方法1：直接复制 <code>tasks.json</code> 文件到安全位置<br>
方法2：使用云同步工具同步整个 <code>AppData\Local\TodoList</code> 目录<br>
方法3：定期导出任务列表（可通过 API 实现自动备份脚本）
</div>
</div>

<div class="faq-item">
<div class="question">Q3: 自动显示/隐藏功能不工作？</div>
<div class="answer">
请检查：
<ul style="margin: 5px 0;">
<li>设置中是否已启用 "启用自动显示/隐藏"</li>
<li>显示间隔是否设置过大</li>
<li>是否点击了窗口导致进入手动模式（重启程序可恢复自动模式）</li>
</ul>
</div>
</div>

<div class="faq-item">
<div class="question">Q4: 如何让程序开机自启动？</div>
<div class="answer">
方法1：将程序快捷方式放到启动文件夹<br>
<code>Win+R</code> 输入 <code>shell:startup</code> 打开启动文件夹<br><br>
方法2：在设置中勾选 "开机自启动"（如果功能可用）
</div>
</div>

<div class="faq-item">
<div class="question">Q5: API 调用失败怎么办？</div>
<div class="answer">
常见原因：
<ul style="margin: 5px 0;">
<li>确保 TodoList 程序正在运行</li>
<li>检查 JSON 格式是否正确</li>
<li>确认必填字段 <code>action</code> 和 <code>title</code> 已提供</li>
<li>检查管道连接超时时间（建议 5000ms）</li>
</ul>
</div>
</div>

<div class="faq-item">
<div class="question">Q6: 如何修改主题颜色？</div>
<div class="answer">
在 <strong>设置</strong> 中可以切换：
<ul style="margin: 5px 0;">
<li>浅色主题（默认）</li>
<li>深色主题</li>
</ul>
深色主题对夜间使用更友好。
</div>
</div>

<div class="faq-item">
<div class="question">Q7: 任务数量有限制吗？</div>
<div class="answer">
理论上没有任务数量限制，但建议：
<ul style="margin: 5px 0;">
<li>保持活动任务在 100 个以内以获得最佳性能</li>
<li>定期清理已完成的任务</li>
<li>使用分类和标签组织大量任务</li>
</ul>
</div>
</div>

<div class="faq-item">
<div class="question">Q8: 找不到系统托盘图标？</div>
<div class="answer">
Windows 11 默认隐藏部分托盘图标：
<ol style="margin: 5px 0;">
<li>点击任务栏右下角的向上箭头 ↑</li>
<li>右键 TodoList 图标，选择 "固定"</li>
<li>图标将始终显示在任务栏上</li>
</ol>
</div>
</div>

<h2>📧 获取帮助</h2>

<div class="faq-item">
<div class="answer">
如遇到其他问题，可以：
<ul style="margin: 5px 0;">
<li>查看项目目录下的 <code>README.md</code> 文档</li>
<li>查看 <code>QUICKSTART.md</code> 快速入门指南</li>
<li>检查 <code>BUILD.md</code> 了解技术细节</li>
</ul>
</div>
</div>

</body>
</html>
)";
    ui->textFaqHelp->setHtml(html);
}

void HelpDialog::onTabChanged(int index)
{
    // 根据标签页切换更新复制按钮的状态
    ui->btnCopyExample->setVisible(index == 1); // 只在 API 标签页显示
}

void HelpDialog::onCopyExample()
{
    QString example = R"({
    "action": "add",
    "title": "示例任务",
    "description": "这是一个通过 API 创建的任务",
    "category": "工作",
    "priority": "中",
    "dueDate": "2025-11-15",
    "tags": ["示例", "API"]
})";
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(example);
    
    QMessageBox::information(this, "复制成功", "JSON 示例已复制到剪贴板！");
}
