@echo off
echo 正在检查OpenCV安装...

rem 尝试设置环境变量
if exist "C:\opencv\build" (
    set OpenCV_DIR=C:\opencv\build
    echo 找到OpenCV: %OpenCV_DIR%
    goto :SetupEnv
) else if exist "C:\Program Files\opencv\build" (
    set OpenCV_DIR=C:\Program Files\opencv\build
    echo 找到OpenCV: %OpenCV_DIR%
    goto :SetupEnv
) else if exist "C:\opencv-mingw\x64\mingw\lib" (
    set OpenCV_DIR=C:\opencv-mingw\x64\mingw\lib
    echo 找到OpenCV: %OpenCV_DIR%
    goto :SetupEnv
) else (
    echo [错误] 未找到OpenCV！
    echo 请下载并安装OpenCV，然后重试
    echo 推荐路径: C:\opencv\build
    goto :EOF
)

:SetupEnv
echo.
echo 设置环境变量: OpenCV_DIR=%OpenCV_DIR%
setx OpenCV_DIR "%OpenCV_DIR%"

rem 设置PATH环境变量
if exist "%OpenCV_DIR%\x64\vc16\bin" (
    echo 添加OpenCV bin目录到PATH: %OpenCV_DIR%\x64\vc16\bin
    setx PATH "%PATH%;%OpenCV_DIR%\x64\vc16\bin"
) else if exist "%OpenCV_DIR%\x64\vc15\bin" (
    echo 添加OpenCV bin目录到PATH: %OpenCV_DIR%\x64\vc15\bin
    setx PATH "%PATH%;%OpenCV_DIR%\x64\vc15\bin"
) else if exist "%OpenCV_DIR%\bin" (
    echo 添加OpenCV bin目录到PATH: %OpenCV_DIR%\bin
    setx PATH "%PATH%;%OpenCV_DIR%\bin"
)

echo.
echo OpenCV环境已配置完成!
echo 请重新启动您的命令提示符或IDE以使环境变量生效
echo.
pause