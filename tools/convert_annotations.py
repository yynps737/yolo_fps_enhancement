# 标注格式转换工具
import os
import json
import argparse
import glob
import xml.etree.ElementTree as ET
import cv2
from pathlib import Path
from tqdm import tqdm

def parse_coco_annotations(json_file, output_dir, classes_file=None):
    """
    将COCO格式标注转换为YOLO格式

    参数:
        json_file: COCO标注JSON文件路径
        output_dir: 输出YOLO标注目录
        classes_file: 类别文件路径，如果提供则将保存类别名称
    """
    with open(json_file, 'r', encoding='utf-8') as f:
        coco_data = json.load(f)

    # 构建类别ID到类别名称的映射
    categories = {cat['id']: cat['name'] for cat in coco_data['categories']}

    # 构建图像ID到文件名的映射
    images = {img['id']: {'file_name': img['file_name'], 'width': img['width'], 'height': img['height']}
              for img in coco_data['images']}

    # 按图像ID分组注释
    annotations_by_image = {}
    for ann in coco_data['annotations']:
        img_id = ann['image_id']
        if img_id not in annotations_by_image:
            annotations_by_image[img_id] = []
        annotations_by_image[img_id].append(ann)

    # 创建输出目录
    os.makedirs(output_dir, exist_ok=True)

    # 保存类别名称到文件
    if classes_file:
        with open(classes_file, 'w', encoding='utf-8') as f:
            for _, name in sorted(categories.items(), key=lambda x: x[0]):
                f.write(f"{name}\n")

    # 转换标注
    for img_id, anns in tqdm(annotations_by_image.items(), desc="Converting COCO annotations"):
        img_info = images[img_id]
        img_width, img_height = img_info['width'], img_info['height']

        # 获取不带扩展名的文件名
        base_name = os.path.splitext(img_info['file_name'])[0]
        out_file = os.path.join(output_dir, base_name + ".txt")

        with open(out_file, 'w', encoding='utf-8') as f:
            for ann in anns:
                # 获取边界框坐标
                bbox = ann['bbox']  # [x, y, width, height]

                # 转换为YOLO格式 [class_id, x_center, y_center, width, height] 归一化到0-1
                x_center = (bbox[0] + bbox[2] / 2) / img_width
                y_center = (bbox[1] + bbox[3] / 2) / img_height
                width = bbox[2] / img_width
                height = bbox[3] / img_height

                # 获取类别ID
                class_id = ann['category_id']

                # 写入YOLO格式
                f.write(f"{class_id} {x_center:.6f} {y_center:.6f} {width:.6f} {height:.6f}\n")

    print(f"转换完成: {len(annotations_by_image)}个图像的标注已转换为YOLO格式")

def parse_voc_annotations(xml_dir, output_dir, classes_file=None):
    """
    将Pascal VOC格式标注转换为YOLO格式

    参数:
        xml_dir: VOC标注XML文件目录
        output_dir: 输出YOLO标注目录
        classes_file: 类别文件路径，如果提供则将保存类别名称
    """
    # 创建输出目录
    os.makedirs(output_dir, exist_ok=True)

    # 查找所有XML文件
    xml_files = glob.glob(os.path.join(xml_dir, "*.xml"))

    # 收集所有类别
    class_names = set()
    for xml_file in xml_files:
        tree = ET.parse(xml_file)
        root = tree.getroot()
        for obj in root.findall("object"):
            class_name = obj.find("name").text
            class_names.add(class_name)

    # 创建类别到ID的映射
    class_map = {name: i for i, name in enumerate(sorted(class_names))}

    # 保存类别名称到文件
    if classes_file:
        with open(classes_file, 'w', encoding='utf-8') as f:
            for name in sorted(class_names):
                f.write(f"{name}\n")

    # 转换标注
    for xml_file in tqdm(xml_files, desc="Converting VOC annotations"):
        tree = ET.parse(xml_file)
        root = tree.getroot()

        # 获取图像尺寸
        size = root.find("size")
        img_width = int(size.find("width").text)
        img_height = int(size.find("height").text)

        # 获取不带扩展名的文件名
        base_name = os.path.splitext(os.path.basename(xml_file))[0]
        out_file = os.path.join(output_dir, base_name + ".txt")

        with open(out_file, 'w', encoding='utf-8') as f:
            for obj in root.findall("object"):
                # 获取类别
                class_name = obj.find("name").text
                class_id = class_map[class_name]

                # 获取边界框
                bbox = obj.find("bndbox")
                xmin = float(bbox.find("xmin").text)
                ymin = float(bbox.find("ymin").text)
                xmax = float(bbox.find("xmax").text)
                ymax = float(bbox.find("ymax").text)

                # 转换为YOLO格式 [class_id, x_center, y_center, width, height] 归一化到0-1
                x_center = (xmin + xmax) / 2 / img_width
                y_center = (ymin + ymax) / 2 / img_height
                width = (xmax - xmin) / img_width
                height = (ymax - ymin) / img_height

                # 写入YOLO格式
                f.write(f"{class_id} {x_center:.6f} {y_center:.6f} {width:.6f} {height:.6f}\n")

    print(f"转换完成: {len(xml_files)}个XML文件已转换为YOLO格式")

def parse_labelme_annotations(json_dir, output_dir, classes_file=None):
    """
    将LabelMe格式标注转换为YOLO格式

    参数:
        json_dir: LabelMe标注JSON文件目录
        output_dir: 输出YOLO标注目录
        classes_file: 类别文件路径，如果提供则将保存类别名称
    """
    # 创建输出目录
    os.makedirs(output_dir, exist_ok=True)

    # 查找所有JSON文件
    json_files = glob.glob(os.path.join(json_dir, "*.json"))

    # 收集所有类别
    class_names = set()
    for json_file in json_files:
        with open(json_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
        for shape in data['shapes']:
            class_names.add(shape['label'])

    # 创建类别到ID的映射
    class_map = {name: i for i, name in enumerate(sorted(class_names))}

    # 保存类别名称到文件
    if classes_file:
        with open(classes_file, 'w', encoding='utf-8') as f:
            for name in sorted(class_names):
                f.write(f"{name}\n")

    # 转换标注
    for json_file in tqdm(json_files, desc="Converting LabelMe annotations"):
        with open(json_file, 'r', encoding='utf-8') as f:
            data = json.load(f)

        # 获取图像尺寸
        img_width = data['imageWidth']
        img_height = data['imageHeight']

        # 获取不带扩展名的文件名
        base_name = os.path.splitext(os.path.basename(json_file))[0]
        out_file = os.path.join(output_dir, base_name + ".txt")

        with open(out_file, 'w', encoding='utf-8') as f:
            for shape in data['shapes']:
                # 仅处理矩形标注
                if shape['shape_type'] != 'rectangle':
                    continue

                # 获取类别
                class_name = shape['label']
                class_id = class_map[class_name]

                # 获取边界框坐标
                points = shape['points']
                xmin = min(points[0][0], points[1][0])
                ymin = min(points[0][1], points[1][1])
                xmax = max(points[0][0], points[1][0])
                ymax = max(points[0][1], points[1][1])

                # 转换为YOLO格式 [class_id, x_center, y_center, width, height] 归一化到0-1
                x_center = (xmin + xmax) / 2 / img_width
                y_center = (ymin + ymax) / 2 / img_height
                width = (xmax - xmin) / img_width
                height = (ymax - ymin) / img_height

                # 写入YOLO格式
                f.write(f"{class_id} {x_center:.6f} {y_center:.6f} {width:.6f} {height:.6f}\n")

    print(f"转换完成: {len(json_files)}个JSON文件已转换为YOLO格式")

def verify_annotations(yolo_dir, images_dir, classes_file=None):
    """
    验证YOLO格式标注，并生成可视化结果

    参数:
        yolo_dir: YOLO标注目录
        images_dir: 图像目录
        classes_file: 类别文件路径
    """
    # 加载类别名称
    classes = []
    if classes_file and os.path.exists(classes_file):
        with open(classes_file, 'r', encoding='utf-8') as f:
            classes = [line.strip() for line in f.readlines()]

    # 创建输出目录
    output_dir = os.path.join(os.path.dirname(yolo_dir), "verification")
    os.makedirs(output_dir, exist_ok=True)

    # 获取所有标注文件
    annotation_files = glob.glob(os.path.join(yolo_dir, "*.txt"))

    # 随机颜色
    import random
    colors = []
    for i in range(len(classes) if classes else 100):
        colors.append((random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)))

    # 验证标注
    valid_count = 0
    total_boxes = 0

    for ann_file in tqdm(annotation_files, desc="Verifying annotations"):
        # 查找对应图像文件
        base_name = os.path.splitext(os.path.basename(ann_file))[0]
        img_files = glob.glob(os.path.join(images_dir, base_name + ".*"))

        if not img_files:
            print(f"警告: 未找到对应图像: {base_name}")
            continue

        img_file = img_files[0]
        img = cv2.imread(img_file)
        if img is None:
            print(f"警告: 无法读取图像: {img_file}")
            continue

        # 获取图像尺寸
        img_height, img_width = img.shape[:2]

        # 读取标注
        boxes = []
        with open(ann_file, 'r', encoding='utf-8') as f:
            for line in f:
                parts = line.strip().split()
                if len(parts) != 5:
                    print(f"警告: 标注格式错误: {line}")
                    continue

                class_id = int(parts[0])
                x_center = float(parts[1]) * img_width
                y_center = float(parts[2]) * img_height
                width = float(parts[3]) * img_width
                height = float(parts[4]) * img_height

                # 计算边界框坐标
                xmin = int(x_center - width / 2)
                ymin = int(y_center - height / 2)
                xmax = int(x_center + width / 2)
                ymax = int(y_center + height / 2)

                boxes.append((class_id, xmin, ymin, xmax, ymax))

        # 绘制边界框
        for box in boxes:
            class_id, xmin, ymin, xmax, ymax = box
            color = colors[class_id % len(colors)]
            cv2.rectangle(img, (xmin, ymin), (xmax, ymax), color, 2)

            # 添加标签
            label = f"{classes[class_id] if class_id < len(classes) else class_id}"
            cv2.putText(img, label, (xmin, ymin - 5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

        # 保存结果
        output_file = os.path.join(output_dir, os.path.basename(img_file))
        cv2.imwrite(output_file, img)

        valid_count += 1
        total_boxes += len(boxes)

    print(f"验证完成: {valid_count}/{len(annotation_files)}个标注有效，共{total_boxes}个边界框")
    print(f"可视化结果已保存到: {output_dir}")

def main():
    parser = argparse.ArgumentParser(description="标注格式转换工具")
    parser.add_argument("--input", type=str, required=True, help="输入标注文件或目录")
    parser.add_argument("--output", type=str, required=True, help="输出YOLO标注目录")
    parser.add_argument("--format", type=str, required=True, choices=["coco", "voc", "labelme"], help="输入标注格式")
    parser.add_argument("--classes", type=str, default=None, help="输出类别文件路径")
    parser.add_argument("--verify", action="store_true", help="验证生成的标注")
    parser.add_argument("--images", type=str, default=None, help="图像目录，用于验证")
    args = parser.parse_args()

    # 创建输出目录
    os.makedirs(args.output, exist_ok=True)

    # 转换标注
    if args.format == "coco":
        parse_coco_annotations(args.input, args.output, args.classes)
    elif args.format == "voc":
        parse_voc_annotations(args.input, args.output, args.classes)
    elif args.format == "labelme":
        parse_labelme_annotations(args.input, args.output, args.classes)

    # 验证标注
    if args.verify and args.images:
        verify_annotations(args.output, args.images, args.classes)

if __name__ == "__main__":
    main()