# YOLO FPS 辅助系统

![版本](https://img.shields.io/badge/版本-1.0-blue)
![C++](https://img.shields.io/badge/C%2B%2B-20-orange)
![OpenCV](https://img.shields.io/badge/OpenCV-4.5%2B-green)

基于YOLOv8实时对象检测的FPS游戏视觉辅助系统。通过计算机视觉技术识别游戏画面中的元素，提供实时目标检测和视觉增强功能。

## 主要特点

- **非侵入式设计**: 仅分析游戏画面，不读取或修改游戏内存
- **多游戏支持**: 可用于多种FPS游戏，包括CS2、Valorant、战地系列等
- **实时性能**: 使用优化的YOLO检测算法和多线程架构实现低延迟检测
- **自适应系统**: 支持增量训练以适应新游戏或游戏更新
- **直观的视觉增强**: 标记敌方玩家、武器和物品，显示距离和威胁评估

## 系统架构

该系统采用模块化设计，主要包含以下核心组件：

```
┌─────────────────────────────────────────────────────────────┐
│                    游戏视觉辅助应用程序                      │
│                                                             │
│  ┌───────────────┐   ┌───────────────┐   ┌───────────────┐  │
│  │  屏幕捕获模块  │──▶│  YOLO检测引擎  │──▶│  渲染覆盖层   │  │
│  └───────────────┘   └───────────────┘   └───────────────┘  │
│         │                    │                   │          │
│         │                    │                   │          │
│  ┌───────────────┐   ┌───────────────┐   ┌───────────────┐  │
│  │  游戏适配器    │◀──│  数据处理管道  │──▶│  辅助控制器   │  │
│  └───────────────┘   └───────────────┘   └───────────────┘  │
│                              │                              │
│  ┌───────────────────────────────────────────────────────┐  │
│  │                     模型管理系统                       │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## 系统要求

### 硬件要求
- **CPU**: Intel Core i5-8400或AMD Ryzen 5 2600或更高
- **内存**: 8GB+
- **显卡**: 支持CUDA的NVIDIA GTX 1060+或同等性能的AMD显卡
- **存储**: 1GB可用空间

### 软件要求
- **操作系统**: Windows 10/11 64位
- **依赖库**:
    - C++20兼容的编译器 (MSVC 19.28+, GCC 10+)
    - CMake 3.15+
    - OpenCV 4.5+
    - vcpkg (推荐用于依赖管理)

## 安装与构建

### 修复RapidJSON编译问题

项目使用C++20标准，而RapidJSON的测试组件使用了已废弃的`std::tr1`命名空间，可能导致编译错误。已在`CMakeLists.txt`中禁用RapidJSON的测试和示例来解决此问题。

### Windows构建步骤

1. 安装依赖:
   ```
   vcpkg install opencv:x64-windows
   ```

2. 克隆仓库:
   ```
   git clone https://github.com/yourusername/yolo-fps-assist.git
   cd yolo-fps-assist
   ```

3. 配置与构建:
   ```
   mkdir build
   cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build . --config Release
   ```

4. 运行程序:
   ```
   cd bin\Release
   YOLO_FPS_ASSIST.exe
   ```

### 手动复制模型文件

确保将YOLO模型文件(`.onnx`格式)复制到`models`目录:
```
copy yolov8s.onnx models\
```

## 使用指南

### 快速开始

1. 运行程序后，会自动开始屏幕捕获和分析
2. 默认情况下，程序会显示一个覆盖窗口，标记检测到的游戏元素
3. 通过快捷键可以控制各项功能

### 控制快捷键

- **F1**: 开启/关闭辅助瞄准
- **F2**: 开启/关闭触发辅助
- **F3**: 开启/关闭ESP视觉增强
- **F4**: 开启/关闭后坐力控制
- **F10**: 显示/隐藏设置窗口
- **F11**: 显示/隐藏覆盖层
- **F12**: 退出程序

### 游戏支持

当前版本支持以下游戏:

- Counter-Strike 2
- Valorant
- Battlefield系列
- Call of Duty系列
- Left 4 Dead 2

## 自定义模型训练

项目包含了模型训练工具，位于`tools`目录：

1. 收集训练数据:
   ```
   python tools/data_collector.py --game "游戏名称" --output "./data"
   ```

2. 转换标注格式:
   ```
   python tools/convert_annotations.py --input "./annotations" --output "./yolo_labels" --format "labelme"
   ```

3. 训练模型:
   ```
   python tools/train_model.py --game "游戏名称" --data "./data" --classes "./classes.txt" --epochs 100
   ```

## 故障排除

### 编译错误

如果遇到与RapidJSON相关的编译错误(如`tr1::"左侧的符号必须是一种类型`):

1. 确保使用更新后的CMakeLists.txt文件，已禁用RapidJSON的测试和示例
2. 清理并重新构建项目:
   ```
   cd build
   cmake --build . --clean-first
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build . --config Release
   ```

### 运行时错误

1. **模型加载失败**:
    - 确保模型文件(.onnx)已正确放置在models目录中
    - 检查模型文件名是否与配置文件中的匹配

2. **屏幕捕获问题**:
    - 如运行在全屏游戏模式下，尝试使用无边框窗口模式
    - 确保有足够的权限访问屏幕

3. **性能问题**:
    - 降低捕获分辨率(在config.json中设置)
    - 使用更小的模型变体(如YOLOv8n)

## 免责声明

本项目仅用于教育和研究目的。使用者需要遵守游戏服务条款和当地法律法规。作者不对因使用本软件而导致的任何后果负责。

## 贡献

欢迎通过以下方式贡献代码：
1. Fork仓库
2. 创建特性分支(`git checkout -b feature/amazing-feature`)
3. 提交更改(`git commit -m 'Add some amazing feature'`)
4. 推送到分支(`git push origin feature/amazing-feature`)
5. 创建Pull Request

## 许可证

本项目采用MIT许可证。详情请查看LICENSE文件。