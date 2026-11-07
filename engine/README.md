# 新计算内核（重建中）

原 C++ 内核（`avas/engine/AVAS.dll`、`libAVAS.so`）和 GPU 内核（`libPIC.so`）的源码已丢失。这里是按原二进制逆向整理出规格后重写的新内核，在 `engine-rebuild` 分支开发，完成全部功能对齐前不合并到 main。

目标：同一份 C++ 物理代码（`__host__ __device__`）编译成 CPU 版（OpenMP）和 CUDA 版；macOS 的 MPS 用 PyTorch。支持 macOS（CPU、MPS）、Windows 和 Linux（CPU、NVIDIA GPU，最低 Pascal sm_61），包括 CentOS 7 / glibc 2.17 的离线集群。精度分 `auto`／`fp64`／`fp32` 三档，`auto` 在 fp64 全速的卡上用 fp64，其余用 fp32 相对参考粒子坐标。

## 目录

- `docker/manylinux2014-cuda12.4.Dockerfile`：Linux 构建环境（glibc 2.17、devtoolset-10、CUDA 12.4）。服务器连不上 CentOS vault 源，CUDA rpm 从 NVIDIA 的 `.cn` 镜像下载后用 `--nodeps` 安装（`cuda-nvcc` 声明依赖系统 `gcc-c++`，实际使用 devtoolset 的编译器）。
- `tools/cu_probe.cu`、`build_probe_linux.sh`、`build_probe_windows.bat`：工具链检查程序，验证多架构机器码的选择、静态 CUDA 运行库在各版本驱动上的兼容性、fp32／fp64 吞吐量，以及同一份 `__host__ __device__` 代码在 CPU 与 GPU 上的结果。
- `tools/ExportDecomp.java`：Ghidra headless 脚本，导出非库函数的伪代码和调用关系（`index.tsv`）。导出的伪代码不放进仓库。

## 工具链检查结果（2026-11-07）

用 CUDA 12.4 编译 sm_61/75/80/86/89/90 的机器码并附 compute_90 的 PTX，CUDA 运行库静态链接；Linux 版只依赖 GLIBC_2.14 和系统 libgomp。

| 机器 | GPU | 驱动（最高 CUDA） | 选中的机器码 | fp32 / fp64（GFLOPS） |
|---|---|---|---|---|
| Ubuntu 24.04 | L40S | 580（13.0） | sm_89 | 73 639 / 1 197 |
| Ubuntu 22.04 | GTX 1080 Ti | 535（12.2） | sm_61 | 9 175 / 331 |
| CentOS 7 集群 | A100-PCIE-40GB | 550（12.4） | sm_80 | 12 379 / 6 173 |
| Windows 11（VS 2022 17.12） | GTX 1650 | 566（12.7） | sm_75 | 2 807 / 74 |

驱动 535 只支持到 CUDA 12.2，静态链接的 12.4 运行库仍可运行（CUDA 12 次版本兼容）。没有 GPU 的节点上 `cudaGetDeviceCount` 返回错误，程序能给出提示并改用 CPU。同一份推进代码 GPU 与 CPU 的 fp64 结果相差 1.6e-11（GPU 使用 FMA 合并），若要逐位一致需要 `-fmad=false`。
