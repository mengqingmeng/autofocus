cmake_minimum_required(VERSION 3.20)
project(autofocus VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动寻找 OpenCV 库
set(OpenCV_STATIC ON)  # 如果你想使用静态链接库，可以设置为 ON
find_package(OpenCV REQUIRED)

# 核心：设置全局符号隐藏（仅对 Linux/Unix 有效，Windows 默认不导出非 __declspec 的符号）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")

# 包含头文件目录
include_directories(include)

# 1. 编译动态链接库 (SHARED)
add_library(${PROJECT_NAME} SHARED src/autofocus.cpp)

# 链接 OpenCV 库到我们的动态库
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})

# 2. 【可选】编译一个可执行测试程序，演示内部如何调用
add_executable(focus_test focus_test.cpp)
target_link_libraries(focus_test PRIVATE ${PROJECT_NAME} ${OpenCV_LIBS})