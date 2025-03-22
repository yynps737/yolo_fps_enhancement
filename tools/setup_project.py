# 项目环境设置脚本
import os
import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path
import urllib.request
import zipfile
import platform

# ONNX模型下载链接
MODEL_URLS = {
    "yolov8n.onnx": "https://github.com/ultralytics/ultralytics/releases/download/v8.0.0/yolov8n.onnx",
    "yolov8s.onnx": "https://github.com/ultralytics/ultralytics/releases/download/v8.0.0/yolov8s.onnx"
}

# 默认配置模板
DEFAULT_CONFIG_TEMPLATE = '''
{
  "activeGame": "GAME_NAME",
  "activeModel": "yolov8s.onnx",
  "captureWidth": 1920,
  "captureHeight": 1080,
  "captureRate": 60,
  "detectionThreshold": 0.5,
  "enableTracking": true,
  "assistSettings": {
    "aimAssistEnabled": false,
    "triggerBotEnabled": false,
    "recoilControlEnabled": false,
    "espEnabled": true,
    "aimAssistStrength": 0.5,
    "smoothness": 0.3,
    "fieldOfView": 10.0
  },
  "renderSettings": {
    "showBoundingBoxes": true,
    "showLabels": true,
    "showDistance": true,
    "showHealth": true,
    "showHeadPosition": true,
    "showLines": true,
    "showCrosshair": true,
    "showFPS": true,
    "showDebugInfo": false
  },
  "inputSettings": {
    "aimKey": 2,
    "triggerKey": 160,
    "hotkeyMap": {
      "0": 112,
      "1": 113,
      "2": 114,
      "3": 115,
      "4": 38,
      "5": 40
    }
  }
}
'''.strip()

# CMAKE模板
CMAKE_TEMPLATE = '''
cmake_minimum_required(VERSION 3.15)
project(YOLO_FPS_ASSIST LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找必要的包
find_package(OpenCV REQUIRED)
find_package(CUDA QUIET)
find_package(TensorRT QUIET)
find_package(ONNX QUIET)

# 包含目录
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

# 收集源文件
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 链接库
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
)

# CUDA支持
if(CUDA_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CUDA_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${CUDA_LIBRARIES})
    add_definitions(-DWITH_CUDA)
endif()

# TensorRT支持
if(TensorRT_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${TensorRT_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${TensorRT_LIBRARIES})
    add_definitions(-DWITH_TENSORRT)
endif()

# ONNX支持
if(ONNX_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${ONNX_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${ONNX_LIBRARIES})
    add_definitions(-DWITH_ONNX)
endif()
'''.strip()

def create_directory_structure(root_dir):
    """创建项目目录结构"""
    # 主目录
    dirs = [
        "src",
        "src/core",
        "src/capture",
        "src/detection",
        "src/processing",
        "src/assist",
        "src/render",
        "src/game",
        "src/model",
        "src/utils",
        "models",
        "resources",
        "tools",
        "build"
    ]

    for d in dirs:
        os.makedirs(os.path.join(root_dir, d), exist_ok=True)

    print(f"创建目录结构完成: {len(dirs)}个目录")
    return os.path.join(root_dir, "src"), os.path.join(root_dir, "resources"), os.path.join(root_dir, "models")

def create_config_file(config_dir, game_name):
    """创建配置文件"""
    config_path = os.path.join(config_dir, "config.json")

    config_content = DEFAULT_CONFIG_TEMPLATE.replace("GAME_NAME", game_name)

    with open(config_path, 'w', encoding='utf-8') as f:
        f.write(config_content)

    print(f"创建配置文件: {config_path}")
    return config_path

def create_cmake_file(root_dir):
    """创建CMakeLists.txt文件"""
    cmake_path = os.path.join(root_dir, "CMakeLists.txt")

    with open(cmake_path, 'w', encoding='utf-8') as f:
        f.write(CMAKE_TEMPLATE)

    print(f"创建CMakeLists.txt: {cmake_path}")
    return cmake_path

def download_models(models_dir):
    """下载预训练模型"""
    for model_name, url in MODEL_URLS.items():
        model_path = os.path.join(models_dir, model_name)

        if os.path.exists(model_path):
            print(f"模型已存在: {model_path}")
            continue

        print(f"下载模型 {model_name} 从 {url} ...")
        try:
            urllib.request.urlretrieve(url, model_path)
            print(f"下载完成: {model_path}")
        except Exception as e:
            print(f"下载失败: {e}")

    print("模型下载完成")

def check_dependencies():
    """检查必要的依赖"""
    print("检查系统依赖...")

    # 检查CMake
    try:
        cmake_version = subprocess.check_output(["cmake", "--version"], text=True)
        print(f"CMake: {cmake_version.split('version')[1].strip().split('\n')[0]}")
    except:
        print("警告: 未检测到CMake，请安装后再构建项目")

    # 检查编译器
    if platform.system() == "Windows":
        try:
            msvc_version = subprocess.check_output(["cl"], stderr=subprocess.STDOUT, text=True)
            if "Microsoft" in msvc_version:
                print(f"MSVC: 已安装")
        except:
            print("警告: 未检测到Visual Studio编译器，请安装Visual Studio")
    else:
        try:
            gcc_version = subprocess.check_output(["g++", "--version"], text=True)
            print(f"GCC: {gcc_version.split('\n')[0]}")
        except:
            print("警告: 未检测到GCC，请安装GCC/G++")

    # 检查OpenCV
    try:
        result = subprocess.check_output(["pkg-config", "--modversion", "opencv4"], text=True)
        print(f"OpenCV: {result.strip()}")
    except:
        print("警告: 未检测到OpenCV，请安装OpenCV 4.x")

    # 检查Python工具
    if shutil.which("python") or shutil.which("python3"):
        print("Python: 已安装")
    else:
        print("警告: 未检测到Python，部分工具可能无法使用")

def create_build_scripts(root_dir):
    """创建构建脚本"""
    if platform.system() == "Windows":
        # Windows批处理脚本
        build_script = os.path.join(root_dir, "build.bat")
        with open(build_script, 'w') as f:
            f.write('''
@echo off
mkdir build 2>nul
cd build
cmake .. -A x64
cmake --build . --config Debug
pause
'''.strip())
        print(f"创建Windows构建脚本: {build_script}")

        run_script = os.path.join(root_dir, "run.bat")
        with open(run_script, 'w') as f:
            f.write('''
@echo off
cd build\\Debug
YOLO_FPS_ASSIST.exe
pause
'''.strip())
        print(f"创建Windows运行脚本: {run_script}")
    else:
        # Linux/macOS Shell脚本
        build_script = os.path.join(root_dir, "build.sh")
        with open(build_script, 'w') as f:
            f.write('''
#!/bin/bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
'''.strip())
        os.chmod(build_script, 0o755)
        print(f"创建Linux/macOS构建脚本: {build_script}")

        run_script = os.path.join(root_dir, "run.sh")
        with open(run_script, 'w') as f:
            f.write('''
#!/bin/bash
cd build
./YOLO_FPS_ASSIST
'''.strip())
        os.chmod(run_script, 0o755)
        print(f"创建Linux/macOS运行脚本: {run_script}")

def create_readme(root_dir, game_name):
    """创建README文件"""
    readme_path = os.path.join(root_dir, "README.md")

    with open(readme_path, 'w', encoding='utf-8') as f:
        f.write(f'''
# 基于YOLOv8的FPS游戏视觉辅助系统

## 简介

这是一个基于计算机视觉的FPS游戏辅助系统，使用YOLOv8实时对象检测技术识别屏幕上的游戏元素。与传统的内存读取方法不同，该系统通过分析游戏画面实现玩家、武器和物品的识别，并提供视觉增强和辅助瞄准功能。

默认配置为: {game_name}

## 功能特点

- 实时识别敌方和友方玩家
- 标记关键目标区域（如头部、躯干）
- 距离估算和威胁评估
- 视觉辅助（类似ESP功能）
- 辅助瞄准（基于检测框的中心点）
- 自适应学习系统

## 构建说明

### 依赖项

- C++20兼容的编译器
- CMake 3.15+
- OpenCV 4.x
- ONNX Runtime (可选)
- CUDA/TensorRT (可选，用于加速)

### 构建步骤

Windows:
```
build.bat
```

Linux/macOS:
```
./build.sh
```

## 注意事项

本项目仅用于教育和学习目的，请遵守各游戏的使用条款和服务协议。
'''.strip())

    print(f"创建README: {readme_path}")

def copy_tools(root_dir):
    """复制工具脚本到工具目录"""
    tools_dir = os.path.join(root_dir, "tools")

    # 检查scripts目录是否存在
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # 复制所有.py文件
    for file in os.listdir(script_dir):
        if file.endswith(".py") and file != "setup_project.py":
            shutil.copy2(os.path.join(script_dir, file), os.path.join(tools_dir, file))

    print(f"工具脚本已复制到: {tools_dir}")

def main():
    parser = argparse.ArgumentParser(description="设置YOLO-FPS-辅助项目环境")
    parser.add_argument("--root-dir", type=str, default="./", help="项目根目录")
    parser.add_argument("--game", type=str, default="Counter-Strike 2", help="默认游戏配置")
    parser.add_argument("--init-models", action="store_true", help="是否下载初始模型")
    args = parser.parse_args()

    # 创建完整路径
    root_dir = os.path.abspath(args.root_dir)

    print(f"设置项目目录: {root_dir}")
    print(f"默认游戏: {args.game}")

    # 检查系统依赖
    check_dependencies()

    # 创建目录结构
    src_dir, config_dir, models_dir = create_directory_structure(root_dir)

    # 创建配置文件
    create_config_file(config_dir, args.game)

    # 创建CMakeLists.txt
    create_cmake_file(root_dir)

    # 下载模型
    if args.init_models:
        download_models(models_dir)

    # 创建构建脚本
    create_build_scripts(root_dir)

    # 创建README
    create_readme(root_dir, args.game)

    # 复制工具脚本
    copy_tools(root_dir)

    print("\n项目环境设置完成!")
    print(f"请使用以下命令构建项目:")
    if platform.system() == "Windows":
        print("build.bat")
    else:
        print("./build.sh")

if __name__ == "__main__":
    main()