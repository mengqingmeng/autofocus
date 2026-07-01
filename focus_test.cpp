#include <iostream>
#include <string>
#include <vector>
#include <filesystem> // C++17 标准库，用于遍历目录
#include "autofocus.h"
#include <opencv2/opencv.hpp>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

// 模拟硬件控制
void MoveMotorTo(double target_z) {
    std::cout << "[硬件控制] 电机移动到 Z 轴位置: " << target_z << " mm\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟机械响应时间
}
namespace fs = std::filesystem;

int main() {
    #ifdef _WIN32
        // 设置控制台输出代码页为 UTF-8
        SetConsoleOutputCP(65001);
    #endif

    // 1. 指定你的测试图片目录（这里放你相机上下运动时拍下的一组照片）
    std::string image_dir = "E:\\workspace\\cpp\\autofocus\\output_frames"; 

    // 检查目录是否存在
    if (!fs::exists(image_dir)) {
        std::cerr << "[错误] 找不到指定的图片目录: " << image_dir << std::endl;
        std::cerr << "请先创建该目录，并在其中放入测试图片！" << std::endl;
        return -1;
    }

    // 2. 收集目录下的所有图片路径
    std::vector<std::string> image_paths;
    for (const auto& entry : fs::directory_iterator(image_dir)) {
        std::string ext = entry.path().extension().string();
        // 过滤常见的图片格式
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" || ext == ".tif") {
            image_paths.push_back(entry.path().string());
        }
    }

    // 冒泡/标准排序，确保图片按文件名顺序读取（模拟相机单向运动）
    std::sort(image_paths.begin(), image_paths.end(),std::greater<>{});

    if (image_paths.empty()) {
        std::cerr << "[错误] 目录中没有找到有效的图片文件！" << std::endl;
        return -1;
    }

    std::cout << "[初始化] 成功加载 " << image_paths.size() << " 张测试图片。" << std::endl;

    // 3. 初始化对焦状态机
    FocusState focus_manager;
    ResetFocusState(&focus_manager);

    double current_z = 0.0;
    double next_z = 0.0;
    double step_size = 0.1; // 假设每张照片之间相机移动了 0.1mm
    int search_direction = 1;

    std::cout << "\n--- 开始读取本地图片进行自动对焦测试 ---" << std::endl;

    // 4. 循环读取图片，模拟相机一边运动一边抓图
    for (size_t i = 0; i < image_paths.size(); ++i) {
        std::string current_img_path = image_paths[i];
        
        // 使用 OpenCV 读取图片 (BGR 格式)
        cv::Mat frame = cv::imread(current_img_path);
        if (frame.empty()) {
            std::cerr << "[警告] 无法读取图片: " << current_img_path << "，跳过。" << std::endl;
            continue;
        }
        auto start = std::chrono::high_resolution_clock::now();

        // 5. 调用我们暴露的动态库接口，计算清晰度得分
        // 提示：如果在实际工业场景中有固定的工件区域，可以把最后四个参数改为你的 ROI 矩形 (x, y, w, h)
        double score = GetClarityScore(frame.data, frame.cols, frame.rows, 0, 0, 0, 0);

        // 6. 将得分送入对焦状态机进行决策
        int res = UpdateFocusDecision(&focus_manager, score, current_z, &next_z, step_size, search_direction);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> elapsed = end - start;

        std::cout << "图片: " << fs::path(current_img_path).filename().string() 
            << " | 模拟Z轴: " << current_z << " mm"
            << " | 边缘清晰度: " << score
            << " | 耗时: " << elapsed.count() << " ms"
            << " | 下一步Z: " << next_z << " mm"
            << " | 对焦状态: " << (focus_manager.is_focused ? "已完成" : "进行中")
            << std::endl;

        if (res == 1) {
            std::cout << "\n[对焦成功] 算法检测到清晰度波峰（越过了最清晰点）！" << std::endl;
            std::cout << "🎯 最佳清晰度位置应在 Z = " << focus_manager.best_z_position << " mm"<< std::endl;
            std::cout << "通知电机回溯到该位置。测试闭环完成。" << std::endl;
            break;
        }

        // 更新模拟的 Z 轴位置，继续下一帧
        current_z = next_z;
    }

    if (!focus_manager.is_focused) {
        std::cout << "\n[结束] 遍历完所有图片，仍未捕获到清晰度波峰（可能运动范围不够，或图片未越过焦点）。" << std::endl;
    }

    return 0;
}