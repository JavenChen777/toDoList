# PowerShell 脚本示例：通过 API 向待办事项程序添加任务

function Send-Task {
    <#
    .SYNOPSIS
        向待办事项程序发送新任务
    
    .DESCRIPTION
        通过命名管道与待办事项程序通信，添加新的待办任务
    
    .PARAMETER Title
        任务标题（必需）
    
    .PARAMETER Description
        任务描述（可选）
    
    .PARAMETER Category
        任务分类（可选）
    
    .PARAMETER Priority
        优先级 1-5，5最高（可选，默认3）
    
    .EXAMPLE
        Send-Task -Title "备份数据"
    
    .EXAMPLE
        Send-Task -Title "更新文档" -Description "检查并更新项目文档" -Category "工作" -Priority 4
    #>
    
    param(
        [Parameter(Mandatory=$true)]
        [string]$Title,
        
        [Parameter(Mandatory=$false)]
        [string]$Description = "",
        
        [Parameter(Mandatory=$false)]
        [string]$Category = "",
        
        [Parameter(Mandatory=$false)]
        [ValidateRange(1, 5)]
        [int]$Priority = 3
    )
    
    $pipeName = "TodoListServer"
    
    try {
        Write-Host "正在连接到待办事项程序..." -ForegroundColor Yellow
        
        # 创建命名管道客户端
        $pipe = New-Object System.IO.Pipes.NamedPipeClientStream(
            ".", 
            $pipeName, 
            [System.IO.Pipes.PipeDirection]::InOut
        )
        
        # 尝试连接（超时 3 秒）
        $pipe.Connect(3000)
        
        if (-not $pipe.IsConnected) {
            Write-Host "错误: 无法连接到待办事项程序" -ForegroundColor Red
            Write-Host "请确保程序正在运行！" -ForegroundColor Yellow
            return $false
        }
        
        Write-Host "已连接！" -ForegroundColor Green
        
        # 创建 StreamWriter
        $writer = New-Object System.IO.StreamWriter($pipe)
        $writer.AutoFlush = $true
        
        # 准备 JSON 消息
        $task = @{
            title = $Title
            description = $Description
            category = $Category
            priority = $Priority
        }
        
        $json = $task | ConvertTo-Json -Compress
        
        # 发送消息
        $writer.WriteLine($json)
        
        # 清理
        $writer.Close()
        $pipe.Close()
        
        Write-Host "✓ 任务已发送: $Title" -ForegroundColor Green
        return $true
        
    } catch {
        Write-Host "发送失败: $_" -ForegroundColor Red
        return $false
    }
}

# 主程序
function Main {
    Write-Host ""
    Write-Host "=" * 60 -ForegroundColor Cyan
    Write-Host "  待办事项程序 - API 测试脚本 (PowerShell)" -ForegroundColor Cyan
    Write-Host "=" * 60 -ForegroundColor Cyan
    Write-Host ""
    
    # 示例 1: 添加简单任务
    Write-Host "示例 1: 添加简单任务" -ForegroundColor Yellow
    Send-Task -Title "测试任务"
    Write-Host ""
    Start-Sleep -Seconds 1
    
    # 示例 2: 添加详细任务
    Write-Host "示例 2: 添加详细任务" -ForegroundColor Yellow
    Send-Task -Title "备份服务器数据" `
              -Description "每周定期备份服务器数据到云存储" `
              -Category "运维" `
              -Priority 5
    Write-Host ""
    Start-Sleep -Seconds 1
    
    # 示例 3: 批量添加任务
    Write-Host "示例 3: 批量添加任务" -ForegroundColor Yellow
    
    $tasks = @(
        @{ Title = "写周报"; Category = "工作"; Priority = 4 },
        @{ Title = "学习 Qt 教程"; Category = "学习"; Priority = 3 },
        @{ Title = "购买日用品"; Category = "生活"; Priority = 2 }
    )
    
    foreach ($task in $tasks) {
        Send-Task @task
        Start-Sleep -Milliseconds 500
    }
    
    Write-Host ""
    Write-Host "=" * 60 -ForegroundColor Cyan
    Write-Host "测试完成！请检查待办事项程序" -ForegroundColor Green
    Write-Host "=" * 60 -ForegroundColor Cyan
    Write-Host ""
}

# 如果直接运行脚本，执行主程序
if ($MyInvocation.InvocationName -ne '.') {
    Main
}

# 导出函数供其他脚本使用
Export-ModuleMember -Function Send-Task
