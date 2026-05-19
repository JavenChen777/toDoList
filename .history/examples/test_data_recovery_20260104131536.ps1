# 数据丢失修复测试脚本
# 用于验证JSON文件读取错误的修复方案

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  数据丢失修复 - 测试脚本" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# 获取数据目录
$appDataPath = [Environment]::GetFolderPath('ApplicationData')
$todoDataPath = Join-Path $appDataPath "TodoList"
$tasksFile = Join-Path $todoDataPath "tasks.json"
$backupFile = Join-Path $todoDataPath "tasks.json.backup"
$testDir = Join-Path $PSScriptRoot "test_data_recovery"

Write-Host "数据目录: $todoDataPath" -ForegroundColor Yellow
Write-Host "任务文件: $tasksFile" -ForegroundColor Yellow
Write-Host "备份文件: $backupFile`n" -ForegroundColor Yellow

# 创建测试目录
if (!(Test-Path $testDir)) {
    New-Item -ItemType Directory -Path $testDir | Out-Null
}

function Test-Scenario {
    param(
        [string]$Name,
        [scriptblock]$Setup,
        [scriptblock]$Verify,
        [string]$Description
    )
    
    Write-Host "----------------------------------------" -ForegroundColor Green
    Write-Host "测试: $Name" -ForegroundColor Green
    Write-Host "描述: $Description" -ForegroundColor Gray
    Write-Host "----------------------------------------" -ForegroundColor Green
    
    try {
        # 执行设置
        & $Setup
        
        Write-Host "等待用户操作程序..." -ForegroundColor Yellow
        Write-Host "请启动程序并观察行为，完成后按任意键继续..." -ForegroundColor Yellow
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        
        # 验证结果
        & $Verify
        
        Write-Host "✓ 测试完成`n" -ForegroundColor Green
    }
    catch {
        Write-Host "✗ 测试失败: $_`n" -ForegroundColor Red
    }
}

# 备份当前数据
Write-Host "正在备份当前数据..." -ForegroundColor Cyan
if (Test-Path $tasksFile) {
    Copy-Item $tasksFile (Join-Path $testDir "tasks.json.original") -Force
    Write-Host "✓ 已备份: tasks.json.original`n" -ForegroundColor Green
}
if (Test-Path $backupFile) {
    Copy-Item $backupFile (Join-Path $testDir "tasks.json.backup.original") -Force
    Write-Host "✓ 已备份: tasks.json.backup.original`n" -ForegroundColor Green
}

# 测试1: 正常备份机制
Test-Scenario -Name "备份创建测试" -Description "验证保存时是否自动创建备份" -Setup {
    # 创建一个有效的测试文件
    $testData = @{
        version = "1.0"
        tasks = @(
            @{
                id = 1
                title = "测试任务1"
                description = "这是一个测试任务"
                priority = 1
                status = 0
                createdTime = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ss")
                dueDate = ""
                tags = @()
            }
        )
    } | ConvertTo-Json -Depth 10
    
    if (!(Test-Path $todoDataPath)) {
        New-Item -ItemType Directory -Path $todoDataPath | Out-Null
    }
    
    $testData | Out-File -FilePath $tasksFile -Encoding UTF8 -Force
    
    # 删除旧备份
    if (Test-Path $backupFile) {
        Remove-Item $backupFile -Force
    }
    
    Write-Host "已创建测试文件，现在启动程序并添加一个新任务..." -ForegroundColor Yellow
} -Verify {
    if (Test-Path $backupFile) {
        Write-Host "✓ 备份文件已创建" -ForegroundColor Green
        $backupContent = Get-Content $backupFile -Raw | ConvertFrom-Json
        Write-Host "  备份中的任务数: $($backupContent.tasks.Count)" -ForegroundColor Gray
    } else {
        Write-Host "✗ 备份文件未创建" -ForegroundColor Red
    }
}

# 测试2: 损坏的JSON文件恢复
Test-Scenario -Name "损坏文件恢复测试" -Description "验证从备份恢复损坏的JSON文件" -Setup {
    # 创建一个有效的备份
    $validData = @{
        version = "1.0"
        tasks = @(
            @{
                id = 1
                title = "备份任务1"
                description = "这个任务在备份中"
                priority = 1
                status = 0
                createdTime = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ss")
                dueDate = ""
                tags = @()
            },
            @{
                id = 2
                title = "备份任务2"
                description = "这个任务也在备份中"
                priority = 2
                status = 0
                createdTime = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ss")
                dueDate = ""
                tags = @()
            }
        )
    } | ConvertTo-Json -Depth 10
    
    $validData | Out-File -FilePath $backupFile -Encoding UTF8 -Force
    
    # 创建一个损坏的主文件
    "{ 这是损坏的JSON内容 [ }" | Out-File -FilePath $tasksFile -Encoding UTF8 -Force
    
    Write-Host "已创建损坏的tasks.json和有效的备份文件" -ForegroundColor Yellow
    Write-Host "启动程序时应该提示从备份恢复..." -ForegroundColor Yellow
} -Verify {
    if (Test-Path $tasksFile) {
        try {
            $content = Get-Content $tasksFile -Raw | ConvertFrom-Json
            Write-Host "✓ 主文件已恢复并可以正常解析" -ForegroundColor Green
            Write-Host "  恢复的任务数: $($content.tasks.Count)" -ForegroundColor Gray
        } catch {
            Write-Host "✗ 主文件仍然损坏" -ForegroundColor Red
        }
    }
}

# 测试3: 空文件处理
Test-Scenario -Name "空文件处理测试" -Description "验证空文件不会覆盖现有数据" -Setup {
    # 创建一个有效的备份
    $validData = @{
        version = "1.0"
        tasks = @(
            @{
                id = 1
                title = "重要任务"
                description = "这个任务不应该被清空"
                priority = 3
                status = 0
                createdTime = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ss")
                dueDate = ""
                tags = @("重要")
            }
        )
    } | ConvertTo-Json -Depth 10
    
    $validData | Out-File -FilePath $backupFile -Encoding UTF8 -Force
    
    # 创建一个空文件
    "" | Out-File -FilePath $tasksFile -Encoding UTF8 -Force
    
    Write-Host "已创建空的tasks.json文件" -ForegroundColor Yellow
    Write-Host "启动程序时应该检测到错误并提示恢复..." -ForegroundColor Yellow
} -Verify {
    if (Test-Path $tasksFile) {
        $content = Get-Content $tasksFile -Raw
        if ($content.Trim() -ne "") {
            try {
                $json = $content | ConvertFrom-Json
                Write-Host "✓ 文件已从备份恢复" -ForegroundColor Green
                Write-Host "  任务数: $($json.tasks.Count)" -ForegroundColor Gray
            } catch {
                Write-Host "✗ 文件未正确恢复" -ForegroundColor Red
            }
        } else {
            Write-Host "✗ 文件仍然为空" -ForegroundColor Red
        }
    }
}

# 测试4: 不完整的JSON（模拟写入中断）
Test-Scenario -Name "不完整JSON处理测试" -Description "验证不完整的JSON不会导致数据丢失" -Setup {
    # 创建一个有效的备份
    $validData = @{
        version = "1.0"
        tasks = @(
            @{
                id = 1
                title = "安全任务"
                description = "这个任务应该被保护"
                priority = 2
                status = 0
                createdTime = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ss")
                dueDate = ""
                tags = @()
            }
        )
    } | ConvertTo-Json -Depth 10
    
    $validData | Out-File -FilePath $backupFile -Encoding UTF8 -Force
    
    # 创建一个不完整的JSON（模拟写入中断）
    $incompleteJson = @"
{
    "version": "1.0",
    "tasks": [
        {
            "id": 1,
            "title": "不完整的任务",
            "description": "
"@
    $incompleteJson | Out-File -FilePath $tasksFile -Encoding UTF8 -Force
    
    Write-Host "已创建不完整的JSON文件（模拟写入中断）" -ForegroundColor Yellow
} -Verify {
    if (Test-Path $tasksFile) {
        try {
            $content = Get-Content $tasksFile -Raw | ConvertFrom-Json
            Write-Host "✓ 文件已从备份恢复" -ForegroundColor Green
            Write-Host "  任务数: $($content.tasks.Count)" -ForegroundColor Gray
        } catch {
            Write-Host "✗ 文件仍然无法解析" -ForegroundColor Red
        }
    }
}

# 恢复原始数据
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  测试完成，恢复原始数据" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$originalFile = Join-Path $testDir "tasks.json.original"
if (Test-Path $originalFile) {
    Copy-Item $originalFile $tasksFile -Force
    Write-Host "✓ 已恢复原始 tasks.json" -ForegroundColor Green
}

$originalBackup = Join-Path $testDir "tasks.json.backup.original"
if (Test-Path $originalBackup) {
    Copy-Item $originalBackup $backupFile -Force
    Write-Host "✓ 已恢复原始备份文件" -ForegroundColor Green
}

Write-Host "`n所有测试完成！" -ForegroundColor Green
Write-Host "测试数据保存在: $testDir" -ForegroundColor Yellow
