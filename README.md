# YOLO-FPS-ASSIST

<div align="center">

![Logo](https://via.placeholder.com/200x200)

[![版本](https://img.shields.io/badge/版本-1.0.0-blue.svg)](https://github.com/yourusername/yolo-fps-assist)
[![许可证](https://img.shields.io/badge/许可证-MIT-green.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.x-green.svg)](https://opencv.org/)
[![ONNX](https://img.shields.io/badge/ONNX-Runtime-orange.svg)](https://onnxruntime.ai/)

</div>

## 项目简介

YOLO-FPS-ASSIST是一个高性能的游戏视觉分析系统，利用最新的YOLOv8目标检测技术对FPS游戏画面进行实时分析，提供增强的游戏体验。该系统完全基于计算机视觉，通过分析屏幕像素来识别游戏中的实体，无需接入游戏内存或修改游戏文件。

> 声明：本项目仅供教育和研究目的使用。请负责任地使用，并尊重各游戏的服务条款。

## 核心功能

- **实时物体检测**：毫秒级识别游戏中的玩家、武器和物品
- **敌友识别**：智能区分敌方和友方单位
- **视觉增强**：通过ESP（Extra Sensory Perception）显示边界框、距离和生命值等信息
- **辅助瞄准**：可配置的目标跟踪和辅助瞄准系统
- **游戏适配**：为多种主流FPS游戏提供专用适配器
- **自适应学习**：能够通过持续使用提高检测精度

## 系统架构

![系统架构](https://via.placeholder.com/800x400)

- **检测引擎**：基于YOLOv8的高效神经网络，支持CUDA/DirectML/TensorRT加速
- **屏幕捕获**：优化的低延迟屏幕捕获模块，支持DirectX和GDI+
- **实体跟踪**：使用增强型跟踪算法实现高精度目标跟踪
- **距离估算**：基于实体大小的精确距离计算模型
- **威胁评估**：综合考虑距离、武器类型和位置的实体威胁度分析
- **渲染系统**：高性能OpenCV渲染模块，支持自定义UI

## 技术规格

- **编程语言**：C++20
- **图像处理**：OpenCV 4.x
- **深度学习**：ONNX Runtime
- **GPU加速**：CUDA/DirectML/TensorRT（可选）
- **模型格式**：ONNX (YOLOv8)
- **帧率表现**：60+ FPS (取决于硬件配置)
- **检测延迟**：< 10ms (使用GPU加速)

## 支持的游戏

- Counter-Strike 2
- Valorant
- Battlefield 系列
- Call of Duty 系列
- Left 4 Dead 2
- 其他FPS游戏（需通过通用适配器）

## 安装指南

### 系统要求

- **操作系统**：Windows 10/11 (64位)
- **处理器**：Intel Core i5-8400 或 AMD Ryzen 5 2600 及以上
- **内存**：8GB RAM 及以上
- **显卡**：NVIDIA GTX 1060 6GB / AMD RX 580 8GB 及以上（推荐支持CUDA/DirectML）
- **存储**：500MB 可用空间
- **依赖**：
    - Visual C++ Redistributable 2019+
    - DirectX 11+

### 构建步骤

1. 克隆仓库：
```bash
git clone https://github.com/yourusername/yolo-fps-assist.git
cd yolo-fps-assist
```

2. 配置CMake构建：
```bash
mkdir build
cd build
cmake ..
```

3. 编译项目：
```bash
cmake --build . --config Release
```

4. 运行应用程序：
```bash
cd bin
YOLO_FPS_ASSIST.exe
```

**注意**：首次运行前，请确保`models`目录中包含所需的ONNX模型。您可以使用`tools/setup_project.py`脚本自动下载预训练模型。

## 使用指南

### 基本操作

1. 启动YOLO-FPS-ASSIST
2. 选择游戏配置文件
3. 启动目标游戏
4. 按下快捷键激活/停用各项功能

### 热键配置

默认热键如下：
- **F10**：显示/隐藏设置窗口
- **F11**：显示/隐藏辅助叠加层
- **F12**：退出程序
- **F1**：开启/关闭辅助瞄准
- **F2**：开启/关闭扳机机器人
- **F3**：开启/关闭ESP显示
- **F4**：开启/关闭后坐力控制
- **↑**：增加辅助强度
- **↓**：减少辅助强度

热键可在`resources/config.json`中自定义。

### 模型配置

通过修改`resources/config.json`文件，可以更改以下配置：
- 活动游戏和检测模型
- 捕获分辨率和帧率
- 检测阈值和跟踪设置
- 辅助系统参数
- 渲染设置
- 输入设置

```json
{
  "activeGame": "Counter-Strike 2",
  "activeModel": "cs2_model.onnx",
  "captureWidth": 1920,
  "captureHeight": 1080,
  "captureRate": 60,
  "detectionThreshold": 0.5,
  "enableTracking": true,
  ...
}
```

## 训练自定义模型

可以使用提供的工具训练针对特定游戏优化的自定义模型。

1. 收集训练数据：
```bash
python tools/data_collector.py --game "Game Name" --output "./data"
```

2. 标注数据：
   使用支持的标注工具（如CVAT、LabelImg）标注收集的图像，然后转换为YOLO格式：
```bash
python tools/convert_annotations.py --input "./annotations" --output "./yolo_labels" --format "coco"
```

3. 训练模型：
```bash
python models/train_model.py --game "Game Name" --data "./data" --classes "./classes.txt" --epochs 100
```

4. 测试模型：
```bash
python tools/test_model.py --model "./models/game_name_model.onnx" --input "./test_images"
```

## 性能优化

为获得最佳性能，请考虑以下建议：

- 启用GPU加速（CUDA/DirectML/TensorRT）
- 降低捕获分辨率以提高帧率
- 对特定游戏使用专用模型
- 关闭不必要的视觉效果
- 使用更快的检测模型变体（如YOLOv8n而非YOLOv8s）

## 故障排除

常见问题及解决方案：

- **程序无法启动**：检查是否安装了所有必需的依赖项
- **检测不准确**：尝试使用特定游戏的模型并调整检测阈值
- **帧率低**：降低捕获分辨率或使用更轻量级的模型
- **无法捕获屏幕**：确保以管理员身份运行或更新显卡驱动程序
- **GPU加速无效**：确保安装了最新的GPU驱动程序并启用了CUDA/DirectML支持

## 代码结构

```
yolo-fps-assist/
├── src/                  # 源代码
│   ├── core/             # 核心应用逻辑
│   ├── capture/          # 屏幕捕获模块
│   ├── detection/        # YOLO检测器
│   ├── processing/       # 检测后处理
│   ├── assist/           # 辅助逻辑
│   ├── render/           # 渲染系统
│   ├── game/             # 游戏适配器
│   ├── model/            # 模型管理
│   └── utils/            # 实用工具
├── models/               # ONNX模型
├── resources/            # 配置文件和资源
├── tools/                # 训练和数据工具
├── build/                # 构建目录
├── CMakeLists.txt        # CMake配置
└── README.md             # 本文档
```

## 贡献指南

我们欢迎并鼓励社区贡献。如要贡献，请遵循以下步骤：

1. Fork仓库
2. 创建特性分支：`git checkout -b feature/your-feature`
3. 提交更改：`git commit -m 'Add your feature'`
4. 推送到分支：`git push origin feature/your-feature`
5. 提交Pull Request

请确保您的代码遵循我们的编码规范并包含相应的测试。

## 许可证

本项目采用MIT许可证。详见[LICENSE](LICENSE)文件。

## 致谢

- [Ultralytics YOLOv8](https://github.com/ultralytics/ultralytics)
- [OpenCV](https://opencv.org/)
- [ONNX Runtime](https://onnxruntime.ai/)
- [RapidJSON](https://rapidjson.org/)
- 以及所有贡献者

## 联系方式

- 项目维护者：Your Name
- 电子邮件：your.email@example.com
- GitHub: [https://github.com/yourusername/yolo-fps-assist](https://github.com/yourusername/yolo-fps-assist)

---

<p align="center">
<strong>本项目仅供教育和研究目的使用</strong><br>
请负责任地使用，并尊重各游戏的服务条款
</p>