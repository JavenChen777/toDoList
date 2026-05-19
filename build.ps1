# PowerShell 构建脚本
# 用于自动化编译 Qt TodoList 项目

param(
    [string]$QtPath = "",
    [string]$BuildType = "Release",
    [switch]$Clean,
    [switch]$Deploy
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  待办事项管理程序 - 构建脚本" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 查找 Qt 安装路径
function Find-QtPath {
    $possiblePaths = @(
        "C:\Qt\6.10.0\msvc2022_64",
        "C:\Qt\6.8.0\msvc2022_64",
        "C:\Qt\6.7.0\msvc2022_64",
        "C:\Qt\6.6.0\msvc2019_64",
        "C:\Qt\6.5.0\msvc2019_64",
        "C:\Qt\6.4.0\msvc2019_64",
        "C:\Qt6\6.10.0\msvc2022_64"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    return $null
}

# 设置 Qt 路径
if ($QtPath -eq "") {
    Write-Host "正在搜索 Qt 安装..." -ForegroundColor Yellow
    $QtPath = Find-QtPath
    
    if ($null -eq $QtPath) {
        Write-Host "错误: 未找到 Qt 安装！" -ForegroundColor Red
        Write-Host "请使用 -QtPath 参数指定 Qt 路径，例如：" -ForegroundColor Yellow
        Write-Host "  .\build.ps1 -QtPath 'C:\Qt\6.5.0\msvc2019_64'" -ForegroundColor Yellow
        exit 1
    }
    
    Write-Host "找到 Qt: $QtPath" -ForegroundColor Green
} else {
    if (-not (Test-Path $QtPath)) {
        Write-Host "错误: 指定的 Qt 路径不存在: $QtPath" -ForegroundColor Red
        exit 1
    }
}

# 设置环境变量
$env:Qt6_DIR = $QtPath
$env:PATH = "$QtPath\bin;$env:PATH"

# 获取项目根目录
$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"

Write-Host ""
Write-Host "项目目录: $ProjectRoot" -ForegroundColor Cyan
Write-Host "构建目录: $BuildDir" -ForegroundColor Cyan
Write-Host "构建类型: $BuildType" -ForegroundColor Cyan
Write-Host ""

# 清理构建目录
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "清理构建目录..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
    Write-Host "清理完成！" -ForegroundColor Green
}

# 创建构建目录
if (-not (Test-Path $BuildDir)) {
    Write-Host "创建构建目录..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# 进入构建目录
Set-Location $BuildDir

# 检查 CMake
Write-Host "检查 CMake..." -ForegroundColor Yellow
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($null -eq $cmake) {
    Write-Host "错误: 未找到 CMake！" -ForegroundColor Red
    Write-Host "请安装 CMake:" -ForegroundColor Yellow
    Write-Host "  winget install Kitware.CMake" -ForegroundColor Cyan
    exit 1
}
Write-Host "找到 CMake: $($cmake.Source)" -ForegroundColor Green

# 检查 C++ 编译器
Write-Host "检查 C++ 编译器..." -ForegroundColor Yellow
$hasCompiler = $false
$compilerType = ""

# 检查 MSVC
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $vsPath = & $vswhere -latest -property installationPath
    if ($vsPath) {
        # 检查是否真的安装了 C++ 工具
        $vcToolsPath = Join-Path $vsPath "VC\Tools\MSVC"
        if (Test-Path $vcToolsPath) {
            $hasCompiler = $true
            $compilerType = "MSVC"
            Write-Host "找到 Visual Studio: $vsPath" -ForegroundColor Green
        } else {
            Write-Host "找到 Visual Studio，但缺少 C++ 组件" -ForegroundColor Yellow
        }
    }
}

# 检查 MinGW
if (-not $hasCompiler) {
    $gcc = Get-Command gcc -ErrorAction SilentlyContinue
    if ($gcc) {
        $hasCompiler = $true
        $compilerType = "MinGW"
        Write-Host "找到 MinGW GCC: $($gcc.Source)" -ForegroundColor Green
    }
}

if (-not $hasCompiler) {
    Write-Host "错误: 未找到 C++ 编译器！" -ForegroundColor Red
    Write-Host ""
    
    # 检查是否安装了 VS 但没有 C++ 组件
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -property installationPath
        if ($vsPath) {
            Write-Host "检测到 Visual Studio 已安装，但缺少 C++ 桌面开发组件。" -ForegroundColor Yellow
            Write-Host ""
            Write-Host "解决方法：" -ForegroundColor Cyan
            Write-Host "1. 打开 Visual Studio Installer" -ForegroundColor White
            Write-Host "2. 点击 'Visual Studio 2022' 旁的 '修改' 按钮" -ForegroundColor White
            Write-Host "3. 勾选 '使用 C++ 的桌面开发'" -ForegroundColor White
            Write-Host "4. 点击 '修改' 开始安装" -ForegroundColor White
            Write-Host ""
            
            $openInstaller = Read-Host "是否打开 Visual Studio Installer？(Y/N)"
            if ($openInstaller -eq 'Y' -or $openInstaller -eq 'y') {
                $installerPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vs_installer.exe"
                if (Test-Path $installerPath) {
                    Start-Process $installerPath
                    Write-Host "✓ 已打开 Visual Studio Installer" -ForegroundColor Green
                }
            }
            Write-Host ""
            exit 1
        }
    }
    
    Write-Host "请选择以下方案之一安装编译器：" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "方案 1: 安装 Visual Studio 2022 Community + C++ 组件 (推荐)" -ForegroundColor Cyan
    Write-Host "  winget install Microsoft.VisualStudio.2022.Community --override '--add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended --passive'" -ForegroundColor Gray
    Write-Host ""
    Write-Host "方案 2: 安装 MinGW (轻量级)" -ForegroundColor Cyan
    Write-Host "  访问: https://winlibs.com/ 下载并安装" -ForegroundColor Gray
    Write-Host ""
    Write-Host "方案 3: 使用 Qt 自带的 MinGW" -ForegroundColor Cyan
    Write-Host "  安装 Qt 时选择 MinGW 编译器" -ForegroundColor Gray
    Write-Host ""
    Write-Host "提示: 运行 .\setup_guide.ps1 获取详细安装指导" -ForegroundColor Yellow
    Write-Host ""
    exit 1
}

# 配置 CMake
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  配置 CMake" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 根据编译器类型选择生成器
if ($compilerType -eq "MSVC") {
    # 使用 Visual Studio 生成器（不需要初始化环境）
    $generator = "Visual Studio 17 2022"
    $cmakeArgs = @(
        "..",
        "-G", $generator,
        "-A", "x64"
    )
    Write-Host "使用生成器: $generator" -ForegroundColor Cyan
} else {
    # MinGW 使用 Ninja
    $generator = "Ninja"
    $cmakeArgs = @(
        "..",
        "-G", $generator
    )
}

if ($QtPath -ne "") {
    $cmakeArgs += "-DCMAKE_PREFIX_PATH=$QtPath"
}

Write-Host "执行: cmake $($cmakeArgs -join ' ')" -ForegroundColor Gray
& cmake @cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "CMake 配置失败！" -ForegroundColor Red
    Set-Location $ProjectRoot
    exit 1
}

# 编译
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  编译项目" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "执行: cmake --build . --config $BuildType" -ForegroundColor Gray
& cmake --build . --config $BuildType

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "编译失败！" -ForegroundColor Red
    Set-Location $ProjectRoot
    exit 1
}

Write-Host ""
Write-Host "编译成功！" -ForegroundColor Green

# 部署 Qt 依赖
if ($Deploy) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "  部署 Qt 依赖" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    
    $exePath = Join-Path $BuildDir "bin\$BuildType\TodoList.exe"
    
    if (Test-Path $exePath) {
        $windeployqt = Join-Path $QtPath "bin\windeployqt.exe"
        
        if (Test-Path $windeployqt) {
            Set-Location (Split-Path $exePath)
            
            Write-Host "执行: windeployqt TodoList.exe --release --no-translations" -ForegroundColor Gray
            & $windeployqt "TodoList.exe" --release --no-translations
            
            if ($LASTEXITCODE -eq 0) {
                Write-Host ""
                Write-Host "部署成功！" -ForegroundColor Green
            } else {
                Write-Host ""
                Write-Host "警告: 部署过程出现错误" -ForegroundColor Yellow
            }
        } else {
            Write-Host "警告: 未找到 windeployqt.exe" -ForegroundColor Yellow
        }
    } else {
        Write-Host "警告: 未找到可执行文件" -ForegroundColor Yellow
    }
}

# 返回项目根目录
Set-Location $ProjectRoot

# 显示结果
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  构建完成！" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$exePath = Join-Path $BuildDir "bin\$BuildType\TodoList.exe"
if (Test-Path $exePath) {
    Write-Host "可执行文件: $exePath" -ForegroundColor Green
    Write-Host ""
    Write-Host "运行程序:" -ForegroundColor Cyan
    Write-Host "  & '$exePath'" -ForegroundColor Yellow
    Write-Host ""
    
    # 询问是否运行
    $run = Read-Host "是否立即运行程序？(Y/N)"
    if ($run -eq "Y" -or $run -eq "y") {
        Write-Host ""
        Write-Host "启动程序..." -ForegroundColor Green
        Start-Process $exePath
    }
} else {
    Write-Host "警告: 未找到可执行文件" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "构建脚本执行完毕！" -ForegroundColor Green
