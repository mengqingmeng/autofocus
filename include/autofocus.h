#ifndef AUTOFOCUS_H
#define AUTOFOCUS_H

// 定义导出宏
#if defined(_WIN32)
    #define FOCUS_API __declspec(dllexport)
#else
    #define FOCUS_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 计算图像指定区域(ROI)的清晰度得分（拉普拉斯方差法）
 * * @param buffer 图像原始数据指针 (BGR 格式)
 * @param width 图像宽度
 * @param height 图像高度
 * @param roi_x ROI 起始 X 坐标 (若全图计算传 0)
 * @param roi_y ROI 起始 Y 坐标 (若全图计算传 0)
 * @param roi_w ROI 宽度 (若全图计算传 0)
 * @param roi_h ROI 高度 (若全图计算传 0)
 * @return double 清晰度得分，分数越高越清晰
 */
FOCUS_API double GetClarityScore(unsigned char* buffer, int width, int height, int roi_x, int roi_y, int roi_w, int roi_h);

/**
 * @brief 对焦状态机结构体
 */
typedef struct {
    double max_score;        // 历史最高得分
    double best_z_position;  // 最佳 Z 轴位置
    int decrease_count;      // 得分连续下降次数
    int is_focused;          // 是否完成对焦 (1: 完成, 0: 进行中)
} FocusState;

/**
 * @brief 初始化/重置对焦状态
 */
FOCUS_API void ResetFocusState(FocusState* state);

/**
 * @brief 爬山法对焦步进决策
 * * @param state 当前状态机指针
 * @param current_score 当前帧的清晰度得分
 * @param current_z 当前 Z 轴实际位置
 * @param out_next_z [输出] 决策后的下一个 Z 轴目标位置
 * @param step_size 步长
 * @param direction 当前运动方向 (1 向上，-1 向下)
 * @return int 状态代码：0-继续寻找，1-成功找到波峰并已计算出最佳位置，-1-异常
 */
FOCUS_API int UpdateFocusDecision(FocusState* state, double current_score, double current_z, double* out_next_z, double step_size, int direction);

#ifdef __cplusplus
}
#endif

#endif // AUTOFOCUS_H