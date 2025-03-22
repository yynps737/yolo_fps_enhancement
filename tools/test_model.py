# YOLO模型测试工具
import cv2
import numpy as np
import argparse
import os
import time
from pathlib import Path
import onnxruntime as ort

class YOLODetector:
    def __init__(self, model_path, conf_thres=0.5, iou_thres=0.45):
        """
        初始化YOLO检测器

        参数:
            model_path: ONNX模型路径
            conf_thres: 置信度阈值
            iou_thres: IOU阈值
        """
        self.conf_threshold = conf_thres
        self.iou_threshold = iou_thres

        # 加载模型
        self.session = ort.InferenceSession(model_path, providers=['CUDAExecutionProvider', 'CPUExecutionProvider'])
        self.input_name = self.session.get_inputs()[0].name

        # 获取输入尺寸
        self.input_shape = self.session.get_inputs()[0].shape
        self.input_width = self.input_shape[2]
        self.input_height = self.input_shape[3]

        # 获取类别名称
        class_file = os.path.splitext(model_path)[0] + '.names'
        if os.path.exists(class_file):
            with open(class_file, 'r') as f:
                self.classes = [line.strip() for line in f.readlines()]
        else:
            self.classes = [f"class{i}" for i in range(1000)]  # 默认类别名称

        print(f"模型加载完成: {model_path}")
        print(f"输入尺寸: {self.input_width}x{self.input_height}")
        print(f"检测类别: {len(self.classes)}")

    def preprocess(self, image):
        """预处理图像"""
        # 调整图像大小
        img = cv2.resize(image, (self.input_width, self.input_height))

        # 转换为RGB
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

        # 归一化并转换为NCHW格式
        img = img.astype(np.float32) / 255.0
        img = img.transpose(2, 0, 1)
        img = np.expand_dims(img, axis=0)

        return img

    def detect(self, image):
        """
        检测图像中的对象

        参数:
            image: 输入图像(BGR格式)

        返回:
            boxes: 边界框坐标 [x1, y1, x2, y2]
            scores: 置信度分数
            class_ids: 类别ID
        """
        original_height, original_width = image.shape[:2]

        # 预处理图像
        input_tensor = self.preprocess(image)

        # 推理
        start_time = time.time()
        outputs = self.session.run(None, {self.input_name: input_tensor})
        inference_time = time.time() - start_time

        # 后处理
        boxes, scores, class_ids = self.postprocess(outputs, original_width, original_height)

        return boxes, scores, class_ids, inference_time

    def postprocess(self, outputs, original_width, original_height):
        """后处理YOLO输出"""
        predictions = outputs[0]

        # 提取边界框、分数和类别
        boxes = []
        scores = []
        class_ids = []

        # 检查输出格式是否符合YOLOv8 (检测到对象, 分类+框)
        if len(predictions.shape) == 3:  # YOLOv8 格式 [batch, num_detections, num_classes+4]
            for i in range(predictions.shape[1]):
                prediction = predictions[0, i, :]

                # YOLOv8 输出格式为 [x, y, w, h, class1_conf, class2_conf, ...]
                # 提取最高置信度类别
                class_scores = prediction[4:]
                class_id = np.argmax(class_scores)
                confidence = class_scores[class_id]

                if confidence > self.conf_threshold:
                    # 提取边界框
                    x, y, w, h = prediction[0:4]

                    # 还原到原始图像尺寸
                    x1 = (x - w/2) * original_width
                    y1 = (y - h/2) * original_height
                    x2 = (x + w/2) * original_width
                    y2 = (y + h/2) * original_height

                    boxes.append([x1, y1, x2, y2])
                    scores.append(float(confidence))
                    class_ids.append(class_id)

        # 应用非极大抑制
        indices = cv2.dnn.NMSBoxes(boxes, scores, self.conf_threshold, self.iou_threshold)

        result_boxes = []
        result_scores = []
        result_class_ids = []

        for i in indices:
            result_boxes.append(boxes[i])
            result_scores.append(scores[i])
            result_class_ids.append(class_ids[i])

        return result_boxes, result_scores, result_class_ids

    def draw_detections(self, image, boxes, scores, class_ids):
        """在图像上绘制检测结果"""
        colors = {
            "player": (0, 0, 255),    # 红色 - 玩家
            "enemy": (0, 0, 255),     # 红色 - 敌人
            "weapon": (255, 255, 0),  # 青色 - 武器
            "item": (0, 255, 255),    # 黄色 - 物品
            "t": (0, 0, 255),         # 红色 - T方
            "ct": (0, 255, 0),        # 绿色 - CT方
            "default": (255, 255, 255) # 白色 - 默认
        }

        for i in range(len(boxes)):
            box = boxes[i]
            class_id = class_ids[i]
            score = scores[i]

            # 获取类别名称和颜色
            class_name = self.classes[class_id] if class_id < len(self.classes) else f"class{class_id}"

            # 选择颜色
            color = colors["default"]
            for key in colors.keys():
                if key in class_name.lower():
                    color = colors[key]
                    break

            # 绘制边界框
            x1, y1, x2, y2 = map(int, box)
            cv2.rectangle(image, (x1, y1), (x2, y2), color, 2)

            # 绘制标签
            label = f"{class_name}: {score:.2f}"
            (label_width, label_height), _ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)
            cv2.rectangle(image, (x1, y1 - label_height - 10), (x1 + label_width, y1), color, -1)
            cv2.putText(image, label, (x1, y1 - 5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1)

        return image

def process_image(detector, image_path, output_path=None, show=True):
    """处理单个图像"""
    # 读取图像
    img = cv2.imread(image_path)
    if img is None:
        print(f"无法读取图像: {image_path}")
        return

    # 检测
    boxes, scores, class_ids, inference_time = detector.detect(img)

    # 绘制结果
    result_img = detector.draw_detections(img.copy(), boxes, scores, class_ids)

    # 添加信息
    cv2.putText(result_img, f"Inference: {inference_time*1000:.1f}ms", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
    cv2.putText(result_img, f"Objects: {len(boxes)}", (10, 70),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    # 保存结果
    if output_path:
        cv2.imwrite(output_path, result_img)
        print(f"结果已保存至: {output_path}")

    # 显示结果
    if show:
        cv2.imshow("Detection Result", result_img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

    return result_img

def process_video(detector, video_path, output_path=None, show=True):
    """处理视频"""
    # 打开视频
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print(f"无法打开视频: {video_path}")
        return

    # 获取视频信息
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))

    # 准备输出视频
    writer = None
    if output_path:
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        writer = cv2.VideoWriter(output_path, fourcc, fps, (width, height))

    # 处理帧
    frame_count = 0
    total_inference_time = 0

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # 检测
        boxes, scores, class_ids, inference_time = detector.detect(frame)
        total_inference_time += inference_time

        # 绘制结果
        result_frame = detector.draw_detections(frame.copy(), boxes, scores, class_ids)

        # 添加信息
        cv2.putText(result_frame, f"Frame: {frame_count}/{total_frames}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
        cv2.putText(result_frame, f"Inference: {inference_time*1000:.1f}ms", (10, 60),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
        cv2.putText(result_frame, f"FPS: {1/inference_time:.1f}", (10, 90),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

        # 保存结果
        if writer:
            writer.write(result_frame)

        # 显示结果
        if show:
            cv2.imshow("Detection Result", result_frame)
            if cv2.waitKey(1) & 0xFF == 27:  # ESC键退出
                break

        frame_count += 1

        # 进度显示
        if frame_count % 10 == 0:
            progress = frame_count / total_frames * 100
            print(f"处理进度: {progress:.1f}% ({frame_count}/{total_frames})")

    # 释放资源
    cap.release()
    if writer:
        writer.release()
    cv2.destroyAllWindows()

    # 输出统计信息
    avg_inference_time = total_inference_time / frame_count
    avg_fps = 1 / avg_inference_time
    print(f"处理完成: {frame_count}帧")
    print(f"平均推理时间: {avg_inference_time*1000:.1f}ms")
    print(f"平均FPS: {avg_fps:.1f}")

def main():
    parser = argparse.ArgumentParser(description="YOLO模型测试工具")
    parser.add_argument("--model", type=str, required=True, help="ONNX模型路径")
    parser.add_argument("--input", type=str, required=True, help="输入图像或视频路径")
    parser.add_argument("--output", type=str, default=None, help="输出结果路径")
    parser.add_argument("--conf", type=float, default=0.5, help="置信度阈值")
    parser.add_argument("--iou", type=float, default=0.45, help="IOU阈值")
    parser.add_argument("--no-show", action="store_true", help="不显示结果")
    args = parser.parse_args()

    # 创建检测器
    detector = YOLODetector(args.model, args.conf, args.iou)

    # 判断输入类型
    input_path = Path(args.input)
    if not input_path.exists():
        print(f"输入文件不存在: {args.input}")
        return

    # 处理输出路径
    output_path = args.output
    if output_path is None and args.input != "0":
        # 自动生成输出路径
        stem = input_path.stem
        output_dir = input_path.parent / "results"
        os.makedirs(output_dir, exist_ok=True)

        if input_path.suffix.lower() in ['.jpg', '.jpeg', '.png', '.bmp']:
            output_path = str(output_dir / f"{stem}_result.jpg")
        elif input_path.suffix.lower() in ['.mp4', '.avi', '.mov']:
            output_path = str(output_dir / f"{stem}_result.mp4")

    # 处理图像或视频
    if input_path.suffix.lower() in ['.jpg', '.jpeg', '.png', '.bmp']:
        process_image(detector, str(input_path), output_path, not args.no_show)
    elif input_path.suffix.lower() in ['.mp4', '.avi', '.mov'] or args.input == "0":
        process_video(detector, args.input, output_path, not args.no_show)
    else:
        print(f"不支持的文件类型: {input_path.suffix}")

if __name__ == "__main__":
    main()