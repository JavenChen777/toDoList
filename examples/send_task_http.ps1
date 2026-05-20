# 网络 API 测试脚本
# 通过 HTTP POST 添加任务到待办事项应用

# 设置服务器地址（当前版本默认仅支持本机）
$serverIp = "127.0.0.1"
$serverPort = 8888
$apiUrl = "http://${serverIp}:${serverPort}/api/tasks"

# 创建任务数据
$task = @{
    title = "网络测试任务"
    description = "通过网络 API 添加的任务"
    category = "测试"
    priority = 4
    dueDate = (Get-Date).AddDays(7).ToString("yyyy-MM-ddTHH:mm:ss")
    tags = @("测试", "网络API")
} | ConvertTo-Json -Compress

Write-Host "发送任务到: $apiUrl"
Write-Host "任务数据: $task"
Write-Host ""

try {
    # 发送 HTTP POST 请求
    $response = Invoke-WebRequest -Uri $apiUrl -Method Post -Body $task -ContentType "application/json; charset=utf-8" -UseBasicParsing
    
    Write-Host "成功! 状态码: $($response.StatusCode)" -ForegroundColor Green
    Write-Host "响应内容:"
    $response.Content | ConvertFrom-Json | ConvertTo-Json -Depth 3
}
catch {
    Write-Host "错误: $_" -ForegroundColor Red
    if ($_.Exception.Response) {
        $reader = [System.IO.StreamReader]::new($_.Exception.Response.GetResponseStream())
        $responseBody = $reader.ReadToEnd()
        Write-Host "服务器响应: $responseBody"
    }
}

Write-Host ""
Write-Host "提示: 当前版本默认仅监听 127.0.0.1，请在运行应用的同一台机器上调用" -ForegroundColor Yellow
