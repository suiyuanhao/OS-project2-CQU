# OS-project2-CQU
基于 Rust for openeuler的内核模块开发与动态加载机制深度剖析

# openEuler 内核模块开发与动态加载机制解析（C vs Rust）
本项目是操作系统内核实验的完整工程实现。我们在 openEuler 6.6.0 (Rustlab) 环境下，分别使用传统的 **C 语言** 和现代系统级语言 **Rust**，完成了从基础模块加载、`/proc` 虚拟文件系统交互到字符设备驱动的全套内核模块开发，并深度对比了两种语言在底层寻址与编译期安全上的本质差异。
## 👥 小组成员
*   张创举
*   丁厚凯
*   隋元皓
*   赵虹尊
*   李志杰
*   刘晓航
## 📂 仓库目录结构
为了保证工程的纯净度与可运行性，仓库同时保留了源码与编译产物：
*   **`01-c-hello` ~ `04-c-char-device`**：C 语言版本的内核模块源码（含 `Makefile`）。
*   **`05-rust-hello` ~ `08-rust-char-device`**：Rust 语言版本的内核模块源码（含 `Makefile`、`*.rs` 以及底层的 `proc_shim.c` 桥接文件）。
*   **`compiled-ko/`**（可选）：在 openEuler 环境下编译通过的 `.ko` 产物备份，方便直接加载测试。（*注：跨内核版本或编译器可能触发 `Invalid module format` 报错。*）
## 🛠️ 编译与运行指南
以 Rust 版本的 `05-rust-hello` 为例，所有实验的加载流程一致：
```bash
# 1. 进入实验目录
cd 05-rust-hello
# 2. 清理并编译（确保使用与内核一致的 Clang 编译器）
make clean
make CC=clang
# 3. 加载内核模块
sudo insmod rust_hello.ko
# 4. 查看内核日志输出
sudo dmesg | tail -n 5
# 5. 卸载模块
sudo rmmod rust_hello
