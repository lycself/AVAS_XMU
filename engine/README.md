# 新计算内核（重建中）

原 C++ 内核（`avas/engine/AVAS.dll`、`libAVAS.so`）和 GPU 内核（`libPIC.so`）的源码已丢失。这里是按原二进制逆向整理出规格后重写的新内核，在 `engine-rebuild` 分支开发，完成全部功能对齐前不合并到 main。

目标：同一份 C++ 物理代码（`__host__ __device__`）编译成 CPU 版（OpenMP）和 CUDA 版；macOS 的 MPS 用 PyTorch。支持 macOS（CPU、MPS）、Windows 和 Linux（CPU、NVIDIA GPU，最低 Pascal sm_61），包括 CentOS 7 / glibc 2.17 的离线集群。精度分 `auto`／`fp64`／`fp32` 三档，`auto` 在 fp64 全速的卡上用 fp64，其余用 fp32 相对参考粒子坐标。

逆向整理的规格在 [docs/engine/](../docs/engine/README.md)：数据结构、8 个子系统、280 个对齐项、已实测核实的结论和待做的比对实验。

## 目录

- `CMakeLists.txt`、`include/`、`src/`、`tests/`：新内核本体（M1：漂移段与场图元件、同步相位、单粒子与 `.dst` 读入、全部常规输出；M2：z-code 硬边元件 quad / solenoid / bend / edge、steerer、superpose / superposeout、multipole）。构建 `cmake -S engine -B engine/build && cmake --build engine/build`；从源码运行时 `AVAS_ENGINE=new` 让 `avas run` 使用它，`AVAS_ENGINE_COMPAT=dll` 复现原 AVAS.dll。与原内核的比对：`python3 engine/tests/parity/run_all.py`（参考输出在 `tests/parity/refs/`，由 Windows AVAS.dll 生成）。Windows MSVC 构建在 `compat=dll` 下与 DLL 逐字符一致，macOS 构建相差约 1e-14～1e-11（系统数学库末位）。macOS 上若 shell 带着 Anaconda 的 `CXXFLAGS` / `CPPFLAGS`，配置前先 `unset` 它们；Command Line Tools 残留的空 `usr/include/c++/v1` 会让 clang 找不到 `<string>`，CMakeLists 已检测并改用 SDK 中的 libc++ 头文件（根治办法是重装 Command Line Tools）。实现记录、兼容开关和比对结果见 [docs/engine/M1-notes.md](../docs/engine/M1-notes.md) 与 [M2-notes.md](../docs/engine/M2-notes.md)。

- `docker/manylinux2014-cuda12.4.Dockerfile`：Linux 构建环境（glibc 2.17、devtoolset-10、CUDA 12.4）。服务器连不上 CentOS vault 源，CUDA rpm 从 NVIDIA 的 `.cn` 镜像下载后用 `--nodeps` 安装（`cuda-nvcc` 声明依赖系统 `gcc-c++`，实际使用 devtoolset 的编译器）。
- `tools/cu_probe.cu`、`build_probe_linux.sh`、`build_probe_windows.bat`：工具链检查程序，验证多架构机器码的选择、静态 CUDA 运行库在各版本驱动上的兼容性、fp32／fp64 吞吐量，以及同一份 `__host__ __device__` 代码在 CPU 与 GPU 上的结果。
- `tools/oracle/`：原内核比对实验工具。`runner.py cases.json WORKDIR REPO` 经 `avas run --mode basic` 运行各算例（在示例 hwr010 上改 beam / input / lattice、加文件，带超时）；`direct.py` 绕过 Python 的 lattice 处理直接调用 `path()` / `main_agent()`（测试内核自己的误差处理等）。`batch1_*.json` 是第一批实验的算例，`batch1_platform_cases.json` 用到的 `win_in.dst` 取自 Windows 上 `g1_base` 算例的 inData.dst。结果见 docs/engine/90-experiments.md。
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
