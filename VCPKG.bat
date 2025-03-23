@echo off
echo ===================================================
echo   vcpkg和OpenCV安装脚本
echo ===================================================
echo.

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo 错误: 此脚本需要管理员权限!
    echo 请右键点击此脚本，选择"以管理员身份运行"。
    pause
    exit /b 1
)

REM 确保有git
where git >nul 2>&1
if %errorLevel% neq 0 (
    echo 错误: 未安装Git或Git不在PATH中。
    echo 请安装Git，然后重新运行此脚本。
    echo 下载地址: https://git-scm.com/downloads
    pause
    exit /b 1
)

REM 检查是否安装了Visual Studio和C++工作负载
echo 检查Visual Studio...
if not exist "C:\Program Files\Microsoft Visual Studio" (
    if not exist "C:\Program Files (x86)\Microsoft Visual Studio" (
        echo 警告: 未检测到Visual Studio。
        echo vcpkg需要Visual Studio 2015 Update 3或更高版本以及C++桌面开发工作负载。
        echo 请确认已安装Visual Studio及C++桌面开发组件。
        choice /C YN /M "是否继续安装?"
        if errorlevel 2 goto :EOF
    )
)
echo.

REM 检查C:\vcpkg是否已存在
if exist C:\vcpkg (
    echo 检测到C:\vcpkg目录已存在。
    choice /C YN /M "是否删除并重新安装vcpkg?"
    if errorlevel 2 (
        goto VCPKG_EXISTS
    ) else (
        echo 删除现有vcpkg...
        rmdir /s /q C:\vcpkg
    )
)

echo ===================================================
echo 步骤1: 克隆vcpkg到C盘根目录
echo ===================================================
echo 正在克隆vcpkg到C:\vcpkg...
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
if %errorLevel% neq 0 (
    echo 克隆vcpkg失败！
    pause
    exit /b 1
)
echo vcpkg克隆成功！
echo.

:BOOTSTRAP
echo ===================================================
echo 步骤2: 引导安装vcpkg
echo ===================================================
echo 正在引导安装vcpkg...
cd /d C:\vcpkg
call bootstrap-vcpkg.bat
if %errorLevel% neq 0 (
    echo vcpkg引导安装失败！
    pause
    exit /b 1
)
echo vcpkg引导安装成功！
echo.

:VCPKG_EXISTS
echo ===================================================
echo 步骤3: 安装OpenCV
echo ===================================================
echo 正在安装OpenCV(x64-windows)...
echo 这可能需要一些时间，请耐心等待...
cd /d C:\vcpkg
call vcpkg install opencv4:x64-windows
if %errorLevel% neq 0 (
    echo 安装OpenCV失败！
    pause
    exit /b 1
)
echo OpenCV安装成功！
echo.

echo ===================================================
echo 步骤4: 集成vcpkg
echo ===================================================
echo 正在将vcpkg集成到系统...
call vcpkg integrate install
if %errorLevel% neq 0 (
    echo vcpkg集成失败！
    pause
    exit /b 1
)
echo vcpkg集成成功！
echo.

echo ===================================================
echo 步骤5: 设置环境变量
echo ===================================================
echo 正在设置环境变量...
setx VCPKG_ROOT C:\vcpkg
setx OpenCV_DIR C:\vcpkg\installed\x64-windows\share\opencv4
echo 环境变量设置完成！
echo.

echo ===================================================
echo 安装完成！
echo ===================================================
echo vcpkg已安装在: C:\vcpkg
echo OpenCV已安装在: C:\vcpkg\installed\x64-windows
echo.
echo 为了在CMake项目中使用OpenCV，请使用以下工具链文件:
echo -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
echo.
echo 按任意键退出...
pause