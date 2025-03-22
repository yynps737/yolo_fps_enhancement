# 游戏屏幕数据收集工具
import cv2
import numpy as np
import os
import time
import argparse
import json
import mss
import mss.tools
import keyboard
from datetime import datetime

class GameDataCollector:
    def __init__(self, game_name, output_dir="./data", interval=1.0, hotkey='f9', quit_key='esc'):
        """
        初始化游戏数据收集器

        参数:
            game_name: 游戏名称
            output_dir: 输出目录
            interval: 截图间隔（秒）
            hotkey: 开始/暂停截图的热键
            quit_key: 退出程序的热键
        """
        self.game_name = game_name
        self.output_dir = os.path.join(output_dir, game_name)
        self.interval = interval
        self.hotkey = hotkey
        self.quit_key = quit_key
        self.running = False
        self.collecting = False
        self.frame_count = 0

        # 确保输出目录存在
        if not os.path.exists(self.output_dir):
            os.makedirs(self.output_dir)

        # 创建用于标注的目录
        self.images_dir = os.path.join(self.output_dir, "images")
        if not os.path.exists(self.images_dir):
            os.makedirs(self.images_dir)

        self.session_id = datetime.now().strftime("%Y%m%d_%H%M%S")

        # 配置屏幕捕获
        self.sct = mss.mss()
        self.monitor = self.sct.monitors[1]  # 主显示器

        # 创建会话信息文件
        self.session_info = {
            "game_name": game_name,
            "session_id": self.session_id,
            "start_time": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            "resolution": f"{self.monitor['width']}x{self.monitor['height']}",
            "frame_count": 0
        }
        self.save_session_info()

        print(f"数据收集器初始化完成 - 游戏: {game_name}")
        print(f"按 {self.hotkey} 开始/暂停截图，按 {self.quit_key} 退出程序")

    def save_session_info(self):
        """保存会话信息到JSON文件"""
        info_file = os.path.join(self.output_dir, f"session_{self.session_id}_info.json")
        with open(info_file, 'w', encoding='utf-8') as f:
            json.dump(self.session_info, f, indent=4, ensure_ascii=False)

    def capture_screen(self):
        """捕获屏幕截图"""
        sct_img = self.sct.grab(self.monitor)
        img = np.array(sct_img)
        img = cv2.cvtColor(img, cv2.COLOR_BGRA2BGR)
        return img

    def save_frame(self, frame):
        """保存帧到文件"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        filename = f"{self.game_name}_{self.session_id}_{self.frame_count:06d}.jpg"
        filepath = os.path.join(self.images_dir, filename)
        cv2.imwrite(filepath, frame)
        self.frame_count += 1
        self.session_info["frame_count"] = self.frame_count
        return filename

    def start(self):
        """启动数据收集器"""
        self.running = True

        # 注册热键回调
        keyboard.on_press_key(self.hotkey, lambda e: self.toggle_collection())
        keyboard.on_press_key(self.quit_key, lambda e: self.stop())

        last_capture_time = 0

        try:
            print("数据收集器已启动，等待按下热键开始收集...")
            while self.running:
                if self.collecting:
                    current_time = time.time()
                    if current_time - last_capture_time >= self.interval:
                        frame = self.capture_screen()
                        filename = self.save_frame(frame)
                        last_capture_time = current_time

                        # 显示预览窗口
                        preview = cv2.resize(frame, (960, 540))
                        cv2.putText(preview, f"Frame: {self.frame_count}", (10, 30),
                                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                        cv2.putText(preview, f"Press {self.hotkey} to pause", (10, 70),
                                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                        cv2.imshow("Data Collector Preview", preview)

                    # 显示状态信息
                    status_img = np.zeros((100, 400, 3), dtype=np.uint8)
                    cv2.putText(status_img, f"Game: {self.game_name}", (10, 30),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                    cv2.putText(status_img, f"Collecting: {self.collecting}", (10, 60),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                    cv2.putText(status_img, f"Frames: {self.frame_count}", (10, 90),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                    cv2.imshow("Status", status_img)

                else:
                    # 显示暂停状态
                    status_img = np.zeros((100, 400, 3), dtype=np.uint8)
                    cv2.putText(status_img, f"Game: {self.game_name}", (10, 30),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 0, 0), 2)
                    cv2.putText(status_img, "PAUSED - Press F9 to resume", (10, 60),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 0, 0), 2)
                    cv2.imshow("Status", status_img)

                # 检查关闭窗口
                if cv2.waitKey(1) & 0xFF == 27:  # ESC键
                    break

                time.sleep(0.01)  # 减少CPU使用率

        finally:
            self.save_session_info()
            cv2.destroyAllWindows()
            print(f"数据收集会话结束 - 共收集了 {self.frame_count} 帧")

    def toggle_collection(self):
        """切换收集状态"""
        self.collecting = not self.collecting
        status = "开始" if self.collecting else "暂停"
        print(f"{status}数据收集...")

    def stop(self):
        """停止数据收集器"""
        self.running = False
        self.collecting = False
        self.session_info["end_time"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print("正在停止数据收集器...")

def main():
    parser = argparse.ArgumentParser(description="游戏屏幕数据收集工具")
    parser.add_argument("--game", type=str, required=True, help="游戏名称")
    parser.add_argument("--output", type=str, default="./data", help="输出目录")
    parser.add_argument("--interval", type=float, default=1.0, help="截图间隔（秒）")
    parser.add_argument("--hotkey", type=str, default="f9", help="开始/暂停热键")
    parser.add_argument("--quit-key", type=str, default="esc", help="退出热键")
    args = parser.parse_args()

    collector = GameDataCollector(
        game_name=args.game,
        output_dir=args.output,
        interval=args.interval,
        hotkey=args.hotkey,
        quit_key=args.quit_key
    )

    collector.start()

if __name__ == "__main__":
    main()