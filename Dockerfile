# 1. 使用官方 GCC 镜像作为基础环境
FROM gcc:latest

# 2. 设置工作目录 /app
WORKDIR /app

# 3. 复制所有 C 语言源文件和头文件
# 注意：这里严格按照你截图中的文件名编写，顺序不影响，但必须拼写正确
COPY main.c ./
COPY battle.h ./
COPY pokemon.h ./
COPY pokemon_data.h ./

# 4. 编译代码
# 使用 gcc 编译 main.c，生成名为 server 的可执行文件
# -o server 表示输出文件名为 server
RUN gcc main.c -o server

# 5. 暴露端口 (可选)
# 如果你的程序监听某个端口，请取消下一行的注释并修改端口号
# EXPOSE 8080

# 6. 启动命令
# 容器启动时运行 server 程序
CMD ["./server"]