cmake_minimum_required(VERSION 3.10)
project(snakes C)  # 显式声明为 C 项目

# 设置 C 标准（可选）
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 开启UNICODE
add_definitions(-DUNICODE -D_UNICODE)

# 设置编译器选项
if(MSVC)
    # MSVC编译器选项
    add_compile_options(/W4) # 设置警告级别为4，提示未使用的变量等
else()
    # GCC编译器选项
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 添加可执行文件
add_executable(${PROJECT_NAME} 
                ../core/util.c 
                ../core/console.c 
                ../core/render.c 
                ../core/music.c 
                map.c
                food.c
                snake.c
                main.c)

# 其他设置
# 设置在外部终端中运行调试器
# Settings中搜索cmake.debugConfig，配置"console": "externalTerminal"
