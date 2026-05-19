# Python 脚本示例：通过 API 向待办事项程序添加任务
# 使用 Windows 命名管道进行通信

import json
import sys

try:
    import win32file
    import win32pipe
    import pywintypes
except ImportError:
    print("错误: 需要安装 pywin32 库")
    print("请运行: pip install pywin32")
    sys.exit(1)


def send_task(title, description="", category="", priority=3):
    """
    向待办事项程序发送新任务
    
    参数:
        title (str): 任务标题（必需）
        description (str): 任务描述（可选）
        category (str): 任务分类（可选）
        priority (int): 优先级 1-5，5最高（可选，默认3）
    """
    pipe_name = r'\\.\pipe\TodoListServer'
    
    try:
        # 连接到命名管道
        print(f"正在连接到 {pipe_name}...")
        
        handle = win32file.CreateFile(
            pipe_name,
            win32file.GENERIC_READ | win32file.GENERIC_WRITE,
            0,
            None,
            win32file.OPEN_EXISTING,
            0,
            None
        )
        
        print("已连接！")
        
        # 准备消息
        task = {
            "title": title,
            "description": description,
            "category": category,
            "priority": priority
        }
        
        message = json.dumps(task, ensure_ascii=False) + "\n"
        
        # 发送消息
        win32file.WriteFile(handle, message.encode('utf-8'))
        
        # 关闭连接
        win32file.CloseHandle(handle)
        
        print(f"✓ 任务已发送: {title}")
        return True
        
    except pywintypes.error as e:
        if e.winerror == 2:
            print("错误: 无法连接到待办事项程序")
            print("请确保程序正在运行！")
        else:
            print(f"错误: {e}")
        return False
    except Exception as e:
        print(f"发送失败: {e}")
        return False


def main():
    """主函数：演示如何使用 API"""
    
    print("=" * 50)
    print("  待办事项程序 - API 测试脚本")
    print("=" * 50)
    print()
    
    # 示例 1: 添加简单任务
    print("示例 1: 添加简单任务")
    send_task("测试任务")
    print()
    
    # 示例 2: 添加详细任务
    print("示例 2: 添加详细任务")
    send_task(
        title="备份服务器数据",
        description="每周定期备份服务器数据到云存储",
        category="运维",
        priority=5
    )
    print()
    
    # 示例 3: 批量添加任务
    print("示例 3: 批量添加任务")
    tasks = [
        {"title": "写周报", "category": "工作", "priority": 4},
        {"title": "学习 Qt 教程", "category": "学习", "priority": 3},
        {"title": "购买日用品", "category": "生活", "priority": 2},
    ]
    
    for task in tasks:
        send_task(**task)
    
    print()
    print("=" * 50)
    print("测试完成！请检查待办事项程序")
    print("=" * 50)


if __name__ == "__main__":
    # 如果提供了命令行参数，使用参数创建任务
    if len(sys.argv) > 1:
        title = sys.argv[1]
        description = sys.argv[2] if len(sys.argv) > 2 else ""
        category = sys.argv[3] if len(sys.argv) > 3 else ""
        
        send_task(title, description, category)
    else:
        # 否则运行演示
        main()
