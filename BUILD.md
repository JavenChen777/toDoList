# 编译说明

本文档提供详细的编译步骤和环境配置说明。

## 环境准备

### 1. 安装 Visual Studio

下载并安装 [Visual Studio 2019](https://visualstudio.microsoft.com/zh-hans/vs/older-downloads/) 或更高版本。

安装时选择以下组件：
- Desktop development with C++
- C++ CMake tools for Windows

### 2. 安装 Qt 6

#### 方法一：在线安装器（推荐）

1. 访问 [Qt 下载页面](https://www.qt.io/download-qt-installer)
2. 下载 Qt Online Installer
3. 运行安装器并登录（可以创建免费账号）
4. 选择以下组件：
   - Qt 6.5.0 (或最新稳定版)
     - MSVC 2019 64-bit
     - Qt Charts
     - Qt 5 Compatibility Module
   - Developer and Designer Tools
     - CMake
     - Ninja

5. 安装路径建议：`C:\Qt`

#### 方法二：离线安装

1. 从 [Qt 归档页面](https://download.qt.io/archive/qt/) 下载对应版本
2. 解压到 `C:\Qt\6.5.0`

### 3. 安装 CMake（如果没有）

如果 Qt 安装器没有包含 CMake，可以：

1. 访问 [CMake 官网](https://cmake.org/download/)
2. 下载 Windows x64 Installer
3. 安装时选择"Add CMake to system PATH"

## 编译步骤

### 方法一：使用 PowerShell 脚本（最简单）

1. 打开 PowerShell
2. 进入项目目录：
   ```powershell
   cd c:\AI\toDoList
   ```

3. 运行构建脚本：
   ```powershell
   .\build.ps1
   ```

脚本会自动检测 Qt 安装路径并完成编译。

### 方法二：使用命令行手动编译

#### Step 1: 打开开发者命令提示符

从开始菜单搜索并打开：
```
x64 Native Tools Command Prompt for VS 2019
```

#### Step 2: 设置 Qt 环境变量

```cmd
set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64
set PATH=%Qt6_DIR%\bin;%PATH%
```

#### Step 3: 配置 CMake

```cmd
cd c:\AI\toDoList
mkdir build
cd build

cmake .. -G "Visual Studio 16 2019" -A x64 ^
    -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/msvc2019_64
```

如果使用 Ninja 生成器（更快）：

```cmd
cmake .. -G "Ninja" ^
    -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/msvc2019_64 ^
    -DCMAKE_BUILD_TYPE=Release
```

#### Step 4: 编译

使用 Visual Studio 生成器：
```cmd
cmake --build . --config Release
```

使用 Ninja：
```cmd
ninja
```

#### Step 5: 运行

```cmd
.\bin\Release\TodoList.exe
```

### 方法三：使用 Qt Creator

Qt Creator 提供了最友好的开发环境。

#### Step 1: 打开项目

1. 启动 Qt Creator
2. File → Open File or Project
3. 选择 `c:\AI\toDoList\CMakeLists.txt`

#### Step 2: 配置 Kit

1. 在 Configure Project 页面
2. 选择 "Desktop Qt 6.5.0 MSVC2019 64bit"
3. 点击 "Configure Project"

#### Step 3: 构建和运行

1. 选择 Release 模式（左下角）
2. 点击绿色三角形运行按钮（或按 Ctrl+R）

## 发布程序

编译成功后，需要部署 Qt 依赖库。

### 使用 windeployqt 工具

```cmd
cd c:\AI\toDoList\build\bin\Release

C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe TodoList.exe --release
```

这会自动复制所需的 Qt DLL 文件到 exe 所在目录。

### 创建发布包

1. 创建发布文件夹：
   ```cmd
   mkdir c:\AI\toDoList\release
   ```

2. 复制文件：
   ```cmd
   xcopy /E /I c:\AI\toDoList\build\bin\Release c:\AI\toDoList\release
   ```

3. 运行 windeployqt：
   ```cmd
   cd c:\AI\toDoList\release
   C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe TodoList.exe --release
   ```

4. 打包：
   将 `release` 文件夹打包为 ZIP 即可分发。

## 常见问题

### 问题 1: CMake 找不到 Qt

**错误信息：**
```
Could not find a package configuration file provided by "Qt6"
```

**解决方案：**
确保在 CMake 命令中指定了 `CMAKE_PREFIX_PATH`：
```cmd
cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/msvc2019_64
```

### 问题 2: 找不到 Qt6Charts

**错误信息：**
```
Could not find a package configuration file provided by "Qt6Charts"
```

**解决方案：**
使用 Qt Maintenance Tool 安装 Qt Charts 组件：
1. 运行 `C:\Qt\MaintenanceTool.exe`
2. 选择 "Add or remove components"
3. 展开 Qt → Qt 6.5.0
4. 勾选 "Qt Charts"

### 问题 3: 编译时出现 LNK2019 错误

**解决方案：**
1. 确保使用了正确的编译器（MSVC 2019）
2. 清理构建目录重新编译：
   ```cmd
   cd build
   del /S /Q *
   cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/msvc2019_64
   cmake --build . --config Release
   ```

### 问题 4: 运行时缺少 DLL

**错误信息：**
```
无法启动此程序，因为计算机中丢失 Qt6Core.dll
```

**解决方案：**

方法一：将 Qt bin 目录添加到 PATH
```cmd
set PATH=C:\Qt\6.5.0\msvc2019_64\bin;%PATH%
```

方法二：使用 windeployqt（推荐）
```cmd
cd build\bin\Release
C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe TodoList.exe
```

### 问题 5: UI 文件无法加载

**错误信息：**
```
Could not find ui_MainWindow.h
```

**解决方案：**
确保 CMakeLists.txt 中启用了 AUTOUIC：
```cmake
set(CMAKE_AUTOUIC ON)
```

重新配置 CMake：
```cmd
cmake ..
```

## 不同编译器的配置

### 使用 MinGW

```cmd
set Qt6_DIR=C:\Qt\6.5.0\mingw_64

cmake .. -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/mingw_64 ^
    -DCMAKE_BUILD_TYPE=Release

mingw32-make
```

### 使用 Clang

```cmd
cmake .. -G "Ninja" ^
    -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/msvc2019_64 ^
    -DCMAKE_C_COMPILER=clang ^
    -DCMAKE_CXX_COMPILER=clang++ ^
    -DCMAKE_BUILD_TYPE=Release

ninja
```

## 性能优化编译选项

### Release 模式优化

```cmd
cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/msvc2019_64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_CXX_FLAGS_RELEASE="/O2 /Ob2 /DNDEBUG"
```

### 启用链接时优化（LTO）

在 CMakeLists.txt 中添加：
```cmake
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
```

## 调试编译

### Debug 模式

```cmd
cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/msvc2019_64 ^
    -DCMAKE_BUILD_TYPE=Debug

cmake --build . --config Debug
```

### 使用 Visual Studio 调试器

1. 在 Qt Creator 中打开项目
2. 切换到 Debug 模式
3. 设置断点
4. 按 F5 开始调试

或使用 Visual Studio：
```cmd
start build\TodoList.sln
```

## 自动化构建

### 使用批处理脚本

创建 `build.bat`：

```batch
@echo off
set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64
set PATH=%Qt6_DIR%\bin;%PATH%

if not exist build mkdir build
cd build

cmake .. -G "Ninja" -DCMAKE_PREFIX_PATH=%Qt6_DIR% -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1

ninja
if errorlevel 1 exit /b 1

cd bin\Release
%Qt6_DIR%\bin\windeployqt.exe TodoList.exe --release

echo Build completed successfully!
pause
```

## 验证安装

编译完成后，运行以下命令验证：

```cmd
cd build\bin\Release
TodoList.exe --version
```

应该看到程序正常启动并显示主窗口。

## 获取帮助

如果遇到其他问题：

1. 检查 Qt 和编译器版本兼容性
2. 查看 CMake 输出日志
3. 确认所有依赖库都已安装
4. 尝试清理构建目录重新编译

祝编译顺利！
