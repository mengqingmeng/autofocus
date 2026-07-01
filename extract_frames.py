import cv2
import os

def extract_frames(video_path, output_folder, num_frames):
    if not os.path.exists(video_path):
        print(f"Video file {video_path} does not exist.")
        return

    # 读取视频
    cap = cv2.VideoCapture(video_path)
    frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    fps = cap.get(cv2.CAP_PROP_FPS)
    print(f"Total frames: {frame_count}, FPS: {fps}")
    # 计算间隔
    interval = frame_count // num_frames
    if interval == 0:
        interval = 1  # 确保至少有一个帧被抽取
    
    # 确保输出文件夹存在
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)
    
    frame_idx = 0
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        if frame_idx % interval == 0:
            filename = os.path.join(output_folder, f'{frame_idx:05}.jpg')
            cv2.imwrite(filename, frame)
        frame_idx += 1
    
    cap.release()
    print(f"Extracted {num_frames} frames from the video to {output_folder}")

# 使用示例
video_path = "C:\\Users\\MQM\\Downloads\\Video_20260701100628249.avi"
output_folder = 'output_frames'
num_frames = 80  # 你想抽取的帧数
extract_frames(video_path, output_folder, num_frames)
