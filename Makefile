# 定义编译器
CC = gcc
# 定义编译参数 (-o 后面是输出的文件名)
CFLAGS = -o server

# 默认目标：当只输入 make 时，执行 build
all: build

# 编译目标：将 main.c 编译成 server 可执行文件
build:
	$(CC) main.c $(CFLAGS)

# 测试目标：先执行 build，然后运行 server init 1
test: build
	./server init 1

# 清理目标：删除生成的可执行文件，保持目录整洁
clean:
	rm -f server