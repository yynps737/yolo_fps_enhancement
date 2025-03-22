# YOLOv8模型训练脚本
from ultralytics import YOLO
import os
import argparse
import shutil
import yaml
import cv2
import numpy as np
from datetime import datetime

def setup_dataset_structure(game_name, data_dir):
    """设置训练数据集文件夹结构"""
    # 创建必要的目录
    train_dir = os.path.join(data_dir, 'train')
    val_dir = os.path.join(data_dir, 'val')

    for d in [train_dir, val_dir]:
        img_dir = os.path.join(d, 'images')
        label_dir = os.path.join(d, 'labels')
        os.makedirs(img_dir, exist_ok=True)
        os.makedirs(label_dir, exist_ok=True)

    # 创建data.yaml文件
    data_yaml = {
        'train': os.path.join('train', 'images'),
        'val': os.path.join('val', 'images'),
        'nc': 0,  # 类别数量，后续更新
        'names': []  # 类别名称，后续更新
    }

    return data_yaml

def process_images(game_name, images_dir, data_dir, split_ratio=0.8):
    """处理图像文件并分配到训练和验证集"""
    if not os.path.exists(images_dir):
        print(f"错误: 图像目录 {images_dir} 不存在")
        return False

    image_files = [f for f in os.listdir(images_dir) if f.lower().endswith(('.jpg', '.jpeg', '.png'))]
    if not image_files:
        print(f"错误: 在 {images_dir} 中没有找到图像文件")
        return False

    # 随机分配到训练和验证集
    np.random.shuffle(image_files)
    split_idx = int(len(image_files) * split_ratio)
    train_files = image_files[:split_idx]
    val_files = image_files[split_idx:]

    # 复制文件到相应目录
    for file_list, subset in [(train_files, 'train'), (val_files, 'val')]:
        for f in file_list:
            src_img = os.path.join(images_dir, f)
            dst_img = os.path.join(data_dir, subset, 'images', f)
            src_label = os.path.join(images_dir, f.rsplit('.', 1)[0] + '.txt')
            dst_label = os.path.join(data_dir, subset, 'labels', f.rsplit('.', 1)[0] + '.txt')

            shutil.copy2(src_img, dst_img)
            if os.path.exists(src_label):
                shutil.copy2(src_label, dst_label)

    print(f"成功分配 {len(train_files)} 个文件到训练集，{len(val_files)} 个文件到验证集")
    return True

def update_class_info(data_dir, class_file):
    """更新类别信息到data.yaml"""
    if not os.path.exists(class_file):
        print(f"错误: 类别文件 {class_file} 不存在")
        return False

    with open(class_file, 'r', encoding='utf-8') as f:
        class_names = [line.strip() for line in f if line.strip()]

    yaml_path = os.path.join(data_dir, 'data.yaml')
    data_yaml = {}

    if os.path.exists(yaml_path):
        with open(yaml_path, 'r', encoding='utf-8') as f:
            data_yaml = yaml.safe_load(f)

    data_yaml['nc'] = len(class_names)
    data_yaml['names'] = class_names

    with open(yaml_path, 'w', encoding='utf-8') as f:
        yaml.dump(data_yaml, f, default_flow_style=False)

    print(f"更新了 {len(class_names)} 个类别到 data.yaml")
    return True

def train_model(game_name, data_dir, epochs=100, batch_size=16, image_size=640, weights='yolov8s.pt'):
    """训练YOLOv8模型"""
    yaml_path = os.path.join(data_dir, 'data.yaml')
    if not os.path.exists(yaml_path):
        print(f"错误: data.yaml 文件 {yaml_path} 不存在")
        return False

    print(f"开始训练 {game_name} 模型...")
    model = YOLO(weights)

    results = model.train(
        data=yaml_path,
        epochs=epochs,
        batch=batch_size,
        imgsz=image_size,
        patience=20,
        name=f"{game_name}_model"
    )

    # 导出模型
    model.export(format='onnx', opset=12)

    # 复制最佳模型到模型目录
    best_model_path = os.path.join(model.trainer.save_dir, 'weights', 'best.onnx')
    final_model_path = os.path.join('models', f"{game_name}_model.onnx")
    if os.path.exists(best_model_path):
        shutil.copy2(best_model_path, final_model_path)
        print(f"模型导出到 {final_model_path}")

    return True

def main():
    parser = argparse.ArgumentParser(description='训练YOLOv8游戏辅助模型')
    parser.add_argument('--game', type=str, required=True, help='游戏名称')
    parser.add_argument('--data', type=str, required=True, help='数据目录')
    parser.add_argument('--classes', type=str, required=True, help='类别文件路径')
    parser.add_argument('--epochs', type=int, default=100, help='训练轮数')
    parser.add_argument('--batch', type=int, default=16, help='批量大小')
    parser.add_argument('--img-size', type=int, default=640, help='图像大小')
    parser.add_argument('--weights', type=str, default='yolov8s.pt', help='起始权重')
    args = parser.parse_args()

    # 替换游戏名称中的空格
    game_name = args.game.replace(' ', '_').lower()

    # 设置数据集路径
    dataset_dir = os.path.join('datasets', game_name)
    os.makedirs(dataset_dir, exist_ok=True)

    # 设置数据集结构
    data_yaml = setup_dataset_structure(game_name, dataset_dir)

    # 处理图像和标签
    if not process_images(game_name, args.data, dataset_dir):
        return

    # 更新类别信息
    if not update_class_info(dataset_dir, args.classes):
        return

    # 训练模型
    train_model(game_name, dataset_dir, args.epochs, args.batch, args.img_size, args.weights)

    print(f"{game_name} 模型训练完成!")

if __name__ == "__main__":
    main()