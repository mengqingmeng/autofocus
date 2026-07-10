## 工业相机自动聚焦
使用opencv实现工业相机的自动聚焦功能

### 依赖
- opencv:4.9.0

### 目录说明
- include 头文件目录
- src 源文件目录
- extract_frame.py 从视频中提取图片
- focus_test.cpp 使用图片目录模拟相机实时采集，自动对焦测试程序

### 如何使用
#### 1. 编译项目
```bash
cd build 
cmake -DOpenCV_DIR="E:/SDK/opencv/4.9.0/build/x64/vc16/lib" ..
cmake --build . --config Release
```
- OpenCV_DIR 换成你的opencv路径
#### 2. 添加库
将静态库拷贝至项目中
##### CMake
```CMake
add_executable(focus_test focus_test.cpp)
target_link_libraries(focus_test PRIVATE ${PROJECT_NAME} ${OpenCV_LIBS})
```
#### 3. 调用
1. 初始化对焦状态机
```cpp
FocusState focus_manager;
ResetFocusState(&focus_manager);
```
2. 计算图像的清晰度
```cpp
// 图像
cv::Mat frame = YOUR_CV_MAT;

// 计算清晰度得分，最后四个参数为ROI区域，全部设置0的为检测整幅图像
double score = GetClarityScore(frame.data, frame.cols, frame.rows, 0, 0, 0, 0);

// 将得分送入对焦状态机进行决策。
// 状态代码：
//  0 继续寻找
//  1 成功找到波峰并已计算出最佳位置
//  -1 异常
int res = UpdateFocusDecision(&focus_manager, score, current_z, &next_z, step_size, search_direction);
```
- 在我的电脑上测试，每帧图像计算耗时50ms左右。