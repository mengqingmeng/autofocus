#include "autofocus.h"
#include <opencv2/opencv.hpp>
#include <cmath>

FOCUS_API double GetClarityScore(unsigned char* buffer, int width, int height, int roi_x, int roi_y, int roi_w, int roi_h) 
{
    if (!buffer || width <= 0 || height <= 0) return 0.0;

    // 1. 将裸数据封装为 cv::Mat (不复制内存，高效率)
    cv::Mat img(height, width, CV_8UC3, buffer);
    
    // 2. 转为灰度图
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    // 3. 处理 ROI 区域
    cv::Mat roi = gray;
    if (roi_w > 0 && roi_h > 0 && (roi_x + roi_w <= width) && (roi_y + roi_h <= height)) {
        roi = gray(cv::Rect(roi_x, roi_y, roi_w, roi_h));
    }

    // 4. 轻微高斯模糊，消除高频工业噪声/相机噪点
    cv::GaussianBlur(roi, roi, cv::Size(3, 3), 0);

    // 5. 拉普拉斯变换计算方差
    cv::Mat laplacian_img;
    cv::Laplacian(roi, laplacian_img, CV_64F);

    cv::Scalar mean, stddev;
    cv::meanStdDev(laplacian_img, mean, stddev);

    // 方差即标准差的平方
    return stddev[0] * stddev[0];
}

FOCUS_API void ResetFocusState(FocusState* state) {
    if (state) {
        state->max_score = 0.0;
        state->best_z_position = 0.0;
        state->decrease_count = 0;
        state->is_focused = 0;
    }
}

FOCUS_API int UpdateFocusDecision(FocusState* state, double current_score, double current_z, double* out_next_z, double step_size, int direction,float drop_threshold,int max_decrease_count) 
{
    if (!state) return -1;

    if (state->is_focused) {
        if(out_next_z)
            *out_next_z = state->best_z_position;
        return 1;
    }

    max_decrease_count = std::max(1, max_decrease_count); // 确保至少为 1

    // 1. 发现更高的清晰度，更新最高点
    if (current_score > state->max_score) {
        state->max_score = current_score;
        state->best_z_position = current_z;
        state->decrease_count = 0; // 重置计数
    } 
    // 2. 得分比历史最高点低
    else if (current_score < state->max_score) {
        state->decrease_count++;
    }

    // 3. 【核心改动】双重判定：不仅要连续下降3次，而且当前得分必须明显低于最高点
    // 设定一个相对门限（例如：比最高点低了 8%）
    double drop_ratio = (state->max_score - current_score) / state->max_score;

    if (state->decrease_count >= max_decrease_count && drop_ratio > drop_threshold) {
        state->is_focused = 1;
        if(out_next_z)
            *out_next_z = state->best_z_position; // 确认为波峰，回溯
        return 1; 
    }

    if(out_next_z)// 4. 未满足判据，继续前进
        *out_next_z = current_z + (direction * step_size);
    return 0;
}

FOCUS_API int UpdateFocusDecision(FocusState *state, double current_score, double current_z,int direction, float drop_threshold, int max_decrease_count)
{
    return UpdateFocusDecision(state,current_score,current_z,nullptr,0,direction,drop_threshold,max_decrease_count);
}
