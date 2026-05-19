# 快速测试数据恢复功能

$appDataPath = [Environment]::GetFolderPath('ApplicationData')
$todoDataPath = Join-Path $appDataPath "TodoList"
$tasksFile = Join-Path $todoDataPath "tasks.json"
$backupFile = Join-Path $todoDataPath "tasks.json.backup"

Write-Host "=== 快速测试：损坏JSON恢复 ===" -ForegroundColor Cyan
Write-Host ""

# 备份当前文件
if (Test-Path $tasksFile) {
    Copy-Item $tasksFile "$tasksFile.test_backup" -Force
    Write-Host "✓ 已备份当前文件" -ForegroundColor Green
}

# 创建一个有效的备份
$validData = @{
    version = "1.0"
    tasks = @(
        @{
            id = 1
            title = "测试任务 - 从备份恢复"
            description = "如果你看到这个任务，说明恢复成功"
            priority = 3
            status = 0
            createdTime = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ss")
            dueDate = ""
            tags = @("测试", "恢复")
        }
    )
} | ConvertTo-Json -Depth 10

if (!(Test-Path $todoDataPath)) {
    New-Item -ItemType Directory -Path $todoDataPath | Out-Null
}

$validData | Out-File -FilePath $backupFile -Encoding UTF8 -Force
Write-Host "✓ 已创建有效备份文件" -ForegroundColor Green

# 创建损坏的主文件
"{ 这是一个损坏的JSON文件 [ }" | Out-File -FilePath $tasksFile -Encoding UTF8 -Force
Write-Host "✓ 已创建损坏的主文件" -ForegroundColor Green

Write-Host ""
Write-Host "现在启动程序测试..." -ForegroundColor Yellow
Write-Host "预期行为：" -ForegroundColor Yellow
Write-Host "  1. 程序检测到主文件损坏" -ForegroundColor Gray
Write-Host "  2. 提示是否从备份恢复" -ForegroundColor Gray
Write-Host "  3. 选择'是'后，应该看到'测试任务 - 从备份恢复'" -ForegroundColor Gray
Write-Host ""

Read-Host "按Enter键启动程序"

& "c:\AI\toDoList\build\bin\Release\TodoList.exe"

Write-Host ""
Write-Host "测试完成后，是否恢复原始数据？(Y/N)" -ForegroundColor Yellow
$restore = Read-Host

if ($restore -eq 'Y' -or $restore -eq 'y') {
    if (Test-Path "$tasksFile.test_backup") {
        Copy-Item "$tasksFile.test_backup" $tasksFile -Force
        Remove-Item "$tasksFile.test_backup" -Force
        Write-Host "✓ 已恢复原始数据" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "测试脚本执行完毕！" -ForegroundColor Green
