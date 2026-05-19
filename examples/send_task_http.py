#!/usr/bin/env python3
"""
网络 API 测试脚本
通过 HTTP POST 添加任务到待办事项应用
"""

import requests
import json
from datetime import datetime, timedelta

# 设置服务器地址（可以是本机或局域网内的其他机器）
SERVER_IP = "localhost"  # 或者使用实际IP地址，如 "192.168.1.100"
SERVER_PORT = 8888
API_URL = f"http://{SERVER_IP}:{SERVER_PORT}/api/tasks"

# 创建任务数据
task = {
    "title": "Python 网络测试任务",
    "description": "通过 Python 网络 API 添加的任务",
    "category": "开发",
    "priority": 5,
    "dueDate": (datetime.now() + timedelta(days=7)).strftime("%Y-%m-%dT%H:%M:%S"),
    "tags": ["Python", "网络API", "测试"]
}

print(f"发送任务到: {API_URL}")
print(f"任务数据: {json.dumps(task, ensure_ascii=False, indent=2)}")
print()

try:
    # 发送 HTTP POST 请求
    response = requests.post(
        API_URL,
        json=task,
        headers={"Content-Type": "application/json; charset=utf-8"},
        timeout=5
    )
    
    print(f"成功! 状态码: {response.status_code}")
    print("响应内容:")
    print(json.dumps(response.json(), ensure_ascii=False, indent=2))
    
except requests.exceptions.ConnectionError:
    print("❌ 连接失败: 无法连接到服务器")
    print(f"   请确保应用程序正在运行，并且可以访问 {SERVER_IP}:{SERVER_PORT}")
except requests.exceptions.Timeout:
    print("❌ 请求超时")
except requests.exceptions.RequestException as e:
    print(f"❌ 请求错误: {e}")
except Exception as e:
    print(f"❌ 错误: {e}")

print()
print("💡 提示: 如果要从其他机器访问，请将 SERVER_IP 改为运行应用的机器的 IP 地址")
