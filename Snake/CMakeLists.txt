cmake_minimum_required(VERSION 3.10)
project(Snake C)  # 显式声明为 C 项目

# 设置 C 标准（可选）
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(${PROJECT_NAME} 
                core/window.c 
                core/render.c 
                core/music.c 
                test/test.c 
                main.c)

# 其他设置
# 设置在外部终端中运行调试器
# Settings中搜索cmake.debugConfig，配置"console": "externalTerminal"
