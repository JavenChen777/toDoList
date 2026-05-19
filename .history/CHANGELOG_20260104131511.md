# 更新日志

## [未发布] - 2026-01-04

### 🔒 安全修复
- **[重要] 修复数据丢失bug** - 解决开机启动时JSON文件读取错误导致数据清空的问题
  - 添加自动备份机制（每次保存前创建 `.backup` 文件）
  - 实现原子写入（通过临时文件确保数据完整性）
  - 添加安全加载方法（失败时不清空现有数据）
  - 实现智能恢复（自动提示从备份恢复）
  - 详见：[docs/DATA_LOSS_FIX.md](docs/DATA_LOSS_FIX.md)

### ✨ 新增功能
- 添加数据恢复测试脚本
  - `examples/quick_test_recovery.ps1` - 快速测试
  - `examples/test_data_recovery.ps1` - 完整测试套件

### 📝 文档
- 新增数据丢失修复技术文档 ([docs/DATA_LOSS_FIX.md](docs/DATA_LOSS_FIX.md))
- 新增数据安全使用指南 ([docs/DATA_PROTECTION_GUIDE.md](docs/DATA_PROTECTION_GUIDE.md))
- 新增修复总结文档 ([BUGFIX_SUMMARY.md](BUGFIX_SUMMARY.md))
- 更新 README.md，添加数据保护功能说明

### 🔧 API 变更
- **TaskStorage** 新增方法：
  - `loadTasksSafe(QList<TaskItem>& tasks)` - 安全加载
  - `createBackup()` - 创建备份
  - `restoreFromBackup()` - 从备份恢复
  - `loadFromBackup()` - 加载备份文件
  - `hasBackup()` - 检查备份是否存在
  - `getBackupFilePath()` - 获取备份文件路径

### 🐛 Bug修复
- 修复JSON解析失败时返回空列表导致数据覆盖的问题
- 修复文件写入中断可能导致数据损坏的问题
- 修复开机启动时云同步冲突可能导致数据丢失的问题

### ⚡ 性能
- 保存操作增加约 5-10ms（用于备份和验证）
- 加载失败时的恢复流程对正常使用无影响

---

## [之前版本]

### 主要功能
- ✅ 任务的新增、编辑、删除、批量删除
- ✅ 标记任务为已完成/待办
- ✅ 任务搜索和过滤（按状态、关键词）
- ✅ 任务分类、优先级、截止日期、标签管理
- ✅ 数据持久化（JSON 格式）
- ✅ 开机自动启动
- ✅ 启动时最小化到系统托盘
- ✅ 自动弹出/隐藏
- ✅ 统计与分析
- ✅ 外部 API 接口（网络 API 和本地 API）
- ✅ 贴纸风格界面
- ✅ 浅色/深色主题切换
