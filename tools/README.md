# YOLO-FPS-辅助 工具集

本目录包含用于支持主项目开发和训练的实用工具。

## 数据收集工具 (data_collector.py)

此工具用于从游戏中收集训练数据。它会定期从屏幕上捕获截图，并将其保存到指定目录以便于后续标注和训练。

### 使用方法

```bash
python data_collector.py --game "游戏名称" --output "./data" --interval 0.5
```

参数说明：
- `--game`: 游戏名称，用于组织数据文件
- `--output`: 输出目录（默认为`./data`）
- `--interval`: 截图间隔，单位为秒（默认为1.0秒）
- `--hotkey`: 开始/暂停收集的热键（默认为F9）
- `--quit-key`: 退出程序的热键（默认为ESC）

### 操作说明

1. 运行程序后，它将打开一个状态窗口
2. 按下指定的热键（默认F9）开始/暂停收集数据
3. 在收集过程中，会实时显示预览窗口
4. 按下ESC键退出程序

## 标注转换工具 (convert_annotations.py)

此工具用于将各种常见标注格式转换为YOLO格式。

### 使用方法

```bash
python convert_annotations.py --input "./annotations" --output "./yolo_labels" --format "coco"
```

参数说明：
- `--input`: 输入标注文件或目录
- `--output`: 输出YOLO标注目录
- `--format`: 输入标注格式（支持"coco", "voc", "labelme"）

## 模型测试工具 (test_model.py)

此工具用于测试已训练好的YOLO模型在静态图像或视频上的性能。

### 使用方法

```bash
python test_model.py --model "../models/cs2_model.onnx" --input "test_image.jpg" --conf 0.5
```

参数说明：
- `--model`: ONNX模型路径
- `--input`: 输入图像或视频路径
- `--conf`: 置信度阈值（默认为0.5）
- `--output`: 输出结果的保存路径（可选）

## 项目构建脚本 (setup_project.py)

此脚本用于快速设置项目环境，包括创建必要的目录结构和初始配置文件。

### 使用方法

```bash
python setup_project.py --root-dir "../" --game "Counter-Strike 2"
```

参数说明：
- `--root-dir`: 项目根目录路径
- `--game`: 默认游戏配置
- `--init-models`: 是否初始化基本模型（下载YOLOv8预训练模型）