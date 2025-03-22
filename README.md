# 基于YOLOv8的FPS游戏视觉辅助系统

![版本](https://img.shields.io/badge/版本-1.0-blue)
![许可证](https://img.shields.io/badge/许可证-MIT-green)
![C++](https://img.shields.io/badge/C%2B%2B-20-orange)

基于计算机视觉的FPS游戏辅助系统，使用YOLOv8实时对象检测技术识别屏幕上的游戏元素。与传统的内存读取方法不同，该系统通过分析游戏画面实现玩家、武器和物品的识别，并提供视觉增强和辅助瞄准功能。

## 核心特性

- **非侵入式设计**: 不读取或修改游戏内存，降低被反作弊系统检测的风险
- **多游戏支持**: 一套系统支持多种视觉相似的FPS游戏
- **实时性能**: 利用YOLO的高效算法和多线程架构实现低延迟检测
- **自适应系统**: 支持增量训练以适应新游戏或游戏更新
- **可配置界面**: 用户友好的设置界面，可定制辅助功能

## 支持游戏

| 游戏 | 检测精度 | 性能(FPS) | 特殊功能 |
|------|---------|---------|---------|
| Counter-Strike 2 | 高(85-95%) | 120+ | T/CT识别, 武器分类 |
| Valorant | 高(85-90%) | 144+ | 角色识别, 能力检测 |
| Apex Legends | 中高(80-90%) | 90+ | 角色识别, 护甲等级 |
| Call of Duty | 中(75-85%) | 80+ | 武器识别, 载具检测 |
| Rainbow Six Siege | 高(85-95%) | 120+ | 干员识别, 道具检测 |
| Battlefield | 中(75-85%) | 80+ | 载具检测, 兵种识别 |
| Left 4 Dead 2 | 高(85-90%) | 120+ | 特感识别, 物品检测 |

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
│  │  ┌───────────────┐  ┌───────────────┐ ┌────────────┐  │  │
│  │  │ 预训练模型库  │  │  在线学习模块  │ │ 模型切换器 │  │  │
│  │  └───────────────┘  └───────────────┘ └────────────┘  │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

- **屏幕捕获模块**: 高效低延迟截取游戏画面
- **YOLO检测引擎**: 运行YOLOv8模型进行对象检测
- **数据处理管道**: 处理检测结果并生成可用数据
- **渲染覆盖层**: 将检测结果可视化叠加在游戏上
- **辅助控制器**: 实现辅助瞄准和其他交互功能
- **游戏适配器**: 处理不同游戏的特定配置和优化
- **模型管理系统**: 管理和切换针对不同游戏的模型

## 系统要求

### 硬件要求
- **CPU**: 至少Intel Core i5-8400或AMD Ryzen 5 2600
- **内存**: 8GB+
- **显卡**: 支持CUDA的NVIDIA GTX 1060+或同等性能的AMD显卡
- **存储**: 1GB可用空间

### 软件要求
- **操作系统**: Windows 10/11 64位 (主要支持)，Linux和macOS支持有限
- **依赖库**:
   - C++20兼容的编译器 (MSVC 19.28+, GCC 10+, Clang 10+)
   - CMake 3.15+
   - OpenCV 4.5+
   - ONNX Runtime 1.8+ (推荐)
   - CUDA 11.0+ (可选，用于GPU加速)
   - DirectX SDK (Windows平台)

## 安装与构建

### Windows

1. 安装依赖:
   ```
   vcpkg install opencv:x64-windows
   vcpkg install onnxruntime:x64-windows
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
   cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg目录]/scripts/buildsystems/vcpkg.cmake
   cmake --build . --config Release
   ```

4. 复制模型和资源:
   ```
   mkdir -p Release/models
   cp ../models/*.onnx Release/models/
   cp -r ../resources Release/
   ```

### Linux

1. 安装依赖:
   ```
   sudo apt update
   sudo apt install build-essential cmake libopencv-dev
   ```

2. 构建步骤与Windows类似:
   ```
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ```

## 快速入门

1. 启动程序:
   ```
   cd build/Release
   ./YOLO_FPS_ASSIST
   ```

2. 配置游戏适配器:
   - 打开程序后，按F10键显示设置窗口
   - 从下拉菜单中选择你的游戏
   - 调整视觉增强选项和辅助功能参数

3. 控制快捷键:
   - **F1**: 开启/关闭辅助瞄准
   - **F2**: 开启/关闭触发辅助
   - **F3**: 开启/关闭ESP视觉增强
   - **F4**: 开启/关闭后坐力控制
   - **F10**: 显示/隐藏设置窗口
   - **F11**: 显示/隐藏覆盖层
   - **F12**: 退出程序

## 自定义模型训练

如需为特定游戏训练自定义模型，可使用提供的训练工具:

1. 收集训练数据:
   ```
   python tools/data_collector.py --game "游戏名称" --output "./data"
   ```

2. 标注数据（推荐使用LabelImg等工具）

3. 转换标注格式:
   ```
   python tools/convert_annotations.py --input "./annotations" --output "./yolo_labels" --format "labelme"
   ```

4. 训练模型:
   ```
   python tools/train_model.py --game "游戏名称" --data "./data" --classes "./classes.txt" --epochs 100
   ```

5. 测试模型:
   ```
   python tools/test_model.py --model "./models/game_model.onnx" --input "./test_images" --conf 0.5
   ```

## 性能优化

- **模型选择**: 根据硬件配置选择合适的YOLOv8变种:
   - 低性能设备: YOLOv8n (Nano)
   - 中等性能: YOLOv8s (Small)
   - 高性能: YOLOv8m/l (Medium/Large)

- **分辨率设置**: 降低捕获分辨率可提高FPS，推荐值为1280x720

- **帧率控制**: 在`config.json`中调整`captureRate`参数

- **GPU加速**: 确保启用CUDA加速和TensorRT优化

## 开发指南

### 添加新游戏支持

1. 创建新的游戏适配器类:
   ```cpp
   // src/game/new_game_adapter.h
   class NewGameAdapter : public GameAdapter {
   public:
       NewGameAdapter();
       void processGameSpecific(ProcessedGameData& gameData) override;
       bool detectGameMode(const cv::Mat& frame) override;
       bool analyzeMapContext(const cv::Mat& frame) override;
   private:
       // 游戏特定方法...
   };
   ```

2. 在`App::switchGame`方法中注册新游戏:
   ```cpp
   bool App::switchGame(const std::string& gameName) {
       if (gameName == "New Game") {
           m_gameAdapter = std::make_unique<NewGameAdapter>();
           Logger::info("加载New Game游戏适配器");
           return true;
       }
       // ...
   }
   ```

3. 为新游戏训练YOLO模型

### 添加新功能

系统设计允许轻松添加新的功能组件:

1. 在相应模块中定义新功能接口
2. 实现功能逻辑
3. 在主程序流程中集成新功能
4. 在配置系统中添加相关设置

## 故障排除

### 常见问题

1. **程序无法启动**
   - 检查依赖库是否正确安装
   - 确认模型文件位于正确位置

2. **检测不准确**
   - 调整`config.json`中的`detectionThreshold`
   - 尝试使用为特定游戏训练的模型

3. **性能问题**
   - 降低捕获分辨率和帧率
   - 选择较小的模型变体
   - 关闭不必要的视觉效果

4. **游戏兼容性**
   - 确保使用兼容游戏的适配器
   - 对于新游戏，可能需要训练自定义模型

### 日志文件

系统日志位于程序目录下的`yolo_fps_assist.log`，报告问题时请附上该文件。

## 与内存方案对比

| 方面 | 计算机视觉方案 | 内存读取方案 |
|------|--------------|------------|
| 检测准确率 | 70-95% | 95-100% |
| 被检测风险 | 低 | 中-高 |
| 计算资源 | 高 | 低 |
| 延迟 | 5-20ms | 1-5ms |
| 通用性 | 高 | 低 |
| 维护复杂度 | 中 | 高 |
| 适用游戏范围 | 广泛 | 特定 |
| 开发复杂度 | 中(ML经验) | 高(逆向工程) |
| 法律风险 | 低 | 中-高 |

## 项目路线图

- [ ] 增强跨平台支持
- [ ] 添加GUI配置界面
- [ ] 实现自适应学习系统
- [ ] 支持更多游戏
- [ ] 优化低端设备性能
- [ ] 添加高级追踪算法

## 贡献指南

欢迎贡献代码、模型和游戏适配器:

1. Fork项目仓库
2. 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建Pull Request

## 免责声明

本项目仅用于教育和研究目的。作者不对因使用本软件而导致的任何后果负责。请遵守游戏服务条款和当地法律法规。

## 许可证

本项目采用MIT许可证。详情请查看LICENSE文件。

## 致谢

- [YOLOv8](https://github.com/ultralytics/ultralytics) - 对象检测框架
- [OpenCV](https://opencv.org/) - 计算机视觉库
- [ONNX Runtime](https://onnxruntime.ai/) - 推理引擎