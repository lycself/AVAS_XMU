# 新计算内核（重建中）

原 C++ 内核（`avas/engine/AVAS.dll`、`libAVAS.so`）和 GPU 内核（`libPIC.so`）的源码已丢失。这里是按原二进制逆向整理出规格后重写的新内核，在 `engine-rebuild` 分支开发，完成全部功能对齐前不合并到 main。

目标：同一份 C++ 物理代码（`__host__ __device__`，设备修饰只经 `src/core/hd.h` 的中性宏）编译成 CPU 版和 CUDA 版；macOS 的 GPU 走 Metal（M6c）。支持 macOS（CPU）、Windows 和 Linux（CPU、NVIDIA GPU，最低 Pascal sm_61），包括 CentOS 7 / glibc 2.17 的离线集群。精度分 `auto`／`fp64`／`fp32` 三档，`auto` 在 fp64 全速的卡上用 fp64，其余用 fp32 相对参考粒子坐标。

逆向整理的规格在 [docs/engine/](../docs/engine/README.md)：数据结构、8 个子系统、280 个对齐项、已实测核实的结论和待做的比对实验。

## 目录

- `CMakeLists.txt`、`include/`、`src/`、`tests/`：新内核本体（M1：漂移段与场图元件、同步相位、单粒子与 `.dst` 读入、全部常规输出；M2：z-code 硬边元件 quad / solenoid / bend / edge、steerer、superpose / superposeout、multipole；M3：初始分布生成与随机数；M4：空间电荷 FFT / SPICNIC，FFT 用 `third_party/pocketfft`）。构建 `cmake -S engine -B engine/build && cmake --build engine/build`；从源码运行时 `avas run --engine new`（或 `AVAS_ENGINE=new`、项目 ini.ini 的 `[engine] kernel = new`）让 `avas run` 使用它，`--compat dll`（`AVAS_ENGINE_COMPAT=dll`）复现原 AVAS.dll；见下文“接入 AVAS（M7a）”。与原内核的比对：`python3 engine/tests/parity/run_all.py`（参考输出在 `tests/parity/refs/`，由 Windows AVAS.dll 生成）。Windows MSVC 构建在 `compat=dll` 下与 DLL 逐字符一致，macOS 构建相差约 1e-14～1e-11（系统数学库末位）。macOS 上若 shell 带着 Anaconda 的 `CXXFLAGS` / `CPPFLAGS`，配置前先 `unset` 它们；Command Line Tools 残留的空 `usr/include/c++/v1` 会让 clang 找不到 `<string>`，CMakeLists 已检测并改用 SDK 中的 libc++ 头文件（根治办法是重装 Command Line Tools）。实现记录、兼容开关和比对结果见 [docs/engine/M1-notes.md](../docs/engine/M1-notes.md) 、[M2-notes.md](../docs/engine/M2-notes.md) 、[M3-notes.md](../docs/engine/M3-notes.md) 与 [M4-notes.md](../docs/engine/M4-notes.md)。

- `docker/manylinux2014-cuda12.4.Dockerfile`：Linux 构建环境（glibc 2.17、devtoolset-10、CUDA 12.4）。服务器连不上 CentOS vault 源，CUDA rpm 从 NVIDIA 的 `.cn` 镜像下载后用 `--nodeps` 安装（`cuda-nvcc` 声明依赖系统 `gcc-c++`，实际使用 devtoolset 的编译器）。
- `tools/cu_probe.cu`、`build_probe_linux.sh`、`build_probe_windows.bat`：工具链检查程序，验证多架构机器码的选择、静态 CUDA 运行库在各版本驱动上的兼容性、fp32／fp64 吞吐量，以及同一份 `__host__ __device__` 代码在 CPU 与 GPU 上的结果。
- `tools/oracle/`：原内核比对实验工具。`runner.py cases.json WORKDIR REPO` 经 `avas run --mode basic` 运行各算例（在示例 hwr010 上改 beam / input / lattice、加文件，带超时）；`direct.py` 绕过 Python 的 lattice 处理直接调用 `path()` / `main_agent()`（测试内核自己的误差处理等）。`batch1_*.json` 是第一批实验的算例，`batch1_platform_cases.json` 用到的 `win_in.dst` 取自 Windows 上 `g1_base` 算例的 inData.dst。结果见 docs/engine/90-experiments.md。
- `tools/ExportDecomp.java`：Ghidra headless 脚本，导出非库函数的伪代码和调用关系（`index.tsv`）。导出的伪代码不放进仓库。

## GPU（M6a，CUDA）

构建（默认 `AVAS_WITH_CUDA=OFF`，CPU 构建不受影响）：

- Linux（glibc 2.17 及以后）：在 manylinux2014 + CUDA 12.4 容器里运行 `tools/build_cuda_linux.sh`
  （`docker run --rm -v 源码:/src -v 输出:/out 镜像 bash /src/engine/tools/build_cuda_linux.sh`）；也可以用 conda-forge 的 GCC（sysroot 2.17）加 NVIDIA 的 cuda-nvcc 12.4，
  `cmake -S engine -B build -DAVAS_WITH_CUDA=ON -DCMAKE_CUDA_COMPILER=…/nvcc -DCMAKE_CXX_COMPILER=…/g++ -DCMAKE_CUDA_HOST_COMPILER=…/g++ -DCMAKE_SHARED_LINKER_FLAGS="-static-libstdc++ -static-libgcc"`
  （`CMAKE_CUDA_HOST_COMPILER` 必须指向同一个 conda GCC，否则 nvcc 用系统的 GCC 4.8，在 CentOS 7 上编译 `.cu` 失败）。
- Windows：VS 2022 Build Tools + CUDA 12.x，`tools\build_cuda_windows.bat engine源码 构建目录 [CUDA目录]`。
- 机器码 sm_60/61/70/75/80/86/89/90 并附 compute_90 的 PTX，CUDA 运行库静态链接，nvcc `--fmad=false`。CUDA 13 不支持 Pascal，必须用 12.x。
- `-DAVAS_WITH_MPI=ON` 加入跨进程（跨节点）的 MPI；`-DAVAS_CUDA_FMAD=ON` 只用于测量逐位一致的代价。

运行（`avas_set_option` 或环境变量；默认 `device=cpu`）：`AVAS_ENGINE_DEVICE=cuda|auto|cpu`、`AVAS_ENGINE_PRECISION=auto|fp64|fp32`、
`AVAS_ENGINE_GPUS=0|0,1|all`。`compat=dll` 只在 CPU 上提供。MPI：`mpirun -np 2 avas_engine_run IN OUT FIELD`（每个进程一块 GPU）。

检查：`python3 engine/tests/gpu/run_gpu.py --lib 库 [--gpus2 0,1] [--mpi "mpirun -np 2"] [--fp32 --stats 8]`（CPU 与 GPU 比对；没有 GPU 时可用 `--device emu` 检查设备通路），
性能 `engine/tests/gpu/perf_gpu.py`（`--scmethod PICNIC` 测 PICNIC）。实现记录、结果和限制见 [docs/engine/M6-notes.md](../docs/engine/M6-notes.md)。

## PICNIC（M6b）

默认模式新增 `scmethod PICNIC`（旧 GPU 内核 libPIC.so 的直接求和空间电荷，规格见 [docs/engine/65-libpic.md](../docs/engine/65-libpic.md)），CPU、emu 与 CUDA 都能用：
网格、电荷分配与 SPICNIC 相同，求解是整个网格电荷与格林函数表的直接求和（`src/core/sc_kernels.h` 的 `picnic_node`，CPU 与 GPU 共用，fp64 逐位相同）；
不写 `numofgrid` / `meshrms` 时用 SPICNIC 的默认网格。libPIC.so 的系数表、截断、边界节点与 float 精度的缺陷已修正（不提供复现开关）；`compat=dll` 下仍按 AVAS.dll 报错（F-01）。
GPU 上 SPICNIC / PICNIC 的格林函数表和 SPICNIC 的非对称修正表在设备上计算（与 CPU 逐位相同，可移植的 `p_log` / `p_atan`）。

M6b 决定（默认模式，compat=dll 不变）：`scmethod` 不区分大小写；`s2picnic` / `s4picnic` 报错并提示改用 PICNIC；`statoutputinterval N` 每 N 步写一行 DataSet（0 不写，默认 1）；
`dumpperiodicity` 非 0 时估计 BeamSet.plt 大小，超过 `beamset_warn_gb`（`AVAS_ENGINE_BEAMSET_WARN_GB`，默认 50 GB）只警告；libPIC.so 的调优关键字点名警告后忽略。

测试脚本的临时目录：`run_all.py`、`run_gpu.py` 不给 `--work` 时在 `$TMPDIR` 建临时目录并在结束时删除（`run_all.py --keep` 保留），给了 `--work` 的目录由使用者删除。

## Metal（M6c，macOS GPU）

构建：在 macOS 上用 Apple 的 clang 时 `AVAS_WITH_METAL` 默认打开，`CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -S engine -B build && cmake --build build` 即可。
conda 等环境设置的 `CC`／`CXX` 编译不了 SDK 的 Foundation 头文件：这时默认关闭 Metal 并在配置时提示改法，显式 `-DAVAS_WITH_METAL=ON` 则报错。
只需 Command Line Tools（Metal 着色器在运行时编译，源代码在构建时嵌入库中）；需要支持 Metal 3 的 GPU（Apple 芯片等）与 macOS 13 以上。
运行：`AVAS_ENGINE_DEVICE=metal`（或 `avas_set_option("device", "metal")`）；`device=auto` 在 Mac 上有空间电荷时选 Metal、否则 CPU；默认仍是 cpu。
Metal 上 t-code 推进（含 superpose 块、误差场、superposeout）、z-code 行变换、统计量、`boundary.txt` 几何损失、`lpboundary` / `longlimits` 和空间电荷网格工作
（分配、PICNIC / SPICNIC / FFT 求解、插值）在 GPU 上（fp32 档，相对同步粒子坐标），二次粒子与 `impscfield` 在 CPU 上；同一台机器重跑逐位相同。

fp32 档在所有设备上是同一套设计（M6c 第四阶段）：主机逻辑在 `core/f32_host.cpp`（`F32Accel`），内核在 `core/f32_kernels.h`，设备只负责缓冲区和启动内核——
Metal（`src/metal/`）、CUDA（`src/cuda/cuda_f32_*.cu`，`AVAS_ENGINE_PRECISION=fp32` 或 auto 在非 HPC 卡上）、CPU（`AVAS_ENGINE_DEVICE=emu AVAS_ENGINE_PRECISION=fp32`，
任何构建都可用，作参照）三者逐字节相同；CUDA 多卡与之相同，MPI 下 fp32 档每个进程计算全部粒子（结果不变，但不加速）。
CUDA fp32 档（M6c 第五阶段）用显式显存与批量复制，单卡不用托管内存（Windows WDDM 对托管内存的限制不影响单卡）；SPICNIC / PICNIC 的格林函数表与修正表在 GPU 上以 fp64 计算（与 CPU 逐位相同的数）。
检查：`run_gpu.py --device metal`（按 fp32 档与 CPU 比较，并与 emu fp32 逐字节比较）、`run_gpu.py --fp32`（CUDA：另与 emu fp32、重跑、双卡、MPI 逐字节比较）、
`avas_engine_physics --metal`；`AVAS_ENGINE_PROFILE=1` 打印每步各部分耗时、z-code 在 GPU 上的次数与转换次数（`AVAS_ENGINE_F32_ZCODE=0` 让 z-code 回到主机，仅供测量）。
设计、结果与限制见 [docs/engine/M6-notes.md](../docs/engine/M6-notes.md) 的 M6c 一节。

## 接入 AVAS（M7a）

新内核是**可选**内核，默认仍用原内核。Python 侧的选择层是 `avas/engines.py`（设计与检查结果见 [docs/engine/M7-notes.md](../docs/engine/M7-notes.md)）：

- 每个设置的优先级：`avas run` 的命令行（`--engine original|new --device auto|cpu|cuda|metal --precision auto|fp64|fp32 --gpus all|0,1 --threads N --compat none|dll`）
  > 环境变量（`AVAS_ENGINE`、`AVAS_ENGINE_DEVICE`、`AVAS_ENGINE_PRECISION`、`AVAS_ENGINE_GPUS`、`AVAS_ENGINE_THREADS`、`AVAS_ENGINE_COMPAT`）
  > 项目 ini.ini 的 `[engine]`（`kernel device precision gpus threads compat`）> 默认（原内核；新内核 auto / auto / all / 0 / none）。
  Python 在 `main_agent` 前用 `avas_set_option` 设置全部选项，所以它解析过的值优先于库自己读的环境变量。库本身的默认仍是 `device=cpu`、`gpus=""`（第一块卡）。
- 库的查找：`AVAS_ENGINE_LIB` > `avas/engine/new/`（M7c 的预编译库）> `engine/build`、`engine/build/Release`、`engine/build/RelWithDebInfo`。
- 选了 `cuda` / `metal` 而本机或这个构建没有时，`avas run` 在运行前报错并提示改用 auto 或 cpu，不会悄悄改用 CPU（`auto` 才会在没有 GPU 时用 CPU）。
- `avas info`（项目里或 `--input DIR`）列出两个内核、新内核的构建选项、本机设备和 auto 的解析结果；`avas info --engine-json` 给出同样内容的 JSON（研究层用它在子进程里预判 auto）。
  `avas_run.json` 的 `engine` 字段记录这次实际用的内核、版本、设备、精度、GPU、兼容模式、线程数与结果档。

C 接口新增三个查询（JSON，见 `include/avas_engine.h`）：`avas_engine_info()`（源码哈希、CUDA / Metal / MPI、CUDA 机器码、编译器、平台）、
`avas_engine_devices()`（CUDA 卡、Metal 设备、CPU 线程数，以及 `auto` 和当前选项在有 / 无空间电荷时会选什么——与 `main_agent` 共用 `src/core/device.cpp` 的 `plan_device`）、
`avas_engine_last_run()`（上一次 `main_agent` 实际用的设备、精度、GPU、compat、线程数、是否有空间电荷、耗时与返回值）。`avas_engine_version()` 返回引擎源码哈希的前 16 位。

**引擎源码哈希**（`cmake/source_hash.cmake`，Python 的 `avas.engines.engine_source_hash` 是同一算法）：输入为 `CMakeLists.txt`、`include/` 与 `src/` 下的全部文件、
`third_party/` 下的源文件（.h .hpp .c .cc .cpp .cu .cuh .mm .metal .inc），跳过以 “.” 开头的名字；按相对路径（`/` 分隔）排序，每个文件先把 CRLF 换成 LF，
哈希 = SHA-256(Σ "相对路径\n" + 文件 SHA-256 + "\n")。构建时由自定义命令重新计算（任何被哈希的文件改动后，下一次构建更新 `generated/avas_source_hash.h`，
哈希不变时不重写，`capi.cpp` 也就不重编），新增文件经 `CONFIGURE_DEPENDS` 重新扫描。`tests/test_engines.py` 检查 `engine/build` 里的库报告的哈希与 Python 算的一致（库是旧的时会失败，提示重新构建）。

## 工具链检查结果（2026-11-07）

用 CUDA 12.4 编译 sm_61/75/80/86/89/90 的机器码并附 compute_90 的 PTX，CUDA 运行库静态链接；Linux 版只依赖 GLIBC_2.14 和系统 libgomp。

| 机器 | GPU | 驱动（最高 CUDA） | 选中的机器码 | fp32 / fp64（GFLOPS） |
|---|---|---|---|---|
| Ubuntu 24.04 | L40S | 580（13.0） | sm_89 | 73 639 / 1 197 |
| Ubuntu 22.04 | GTX 1080 Ti | 535（12.2） | sm_61 | 9 175 / 331 |
| CentOS 7 集群 | A100-PCIE-40GB | 550（12.4） | sm_80 | 12 379 / 6 173 |
| Windows 11（VS 2022 17.12） | GTX 1650 | 566（12.7） | sm_75 | 2 807 / 74 |

驱动 535 只支持到 CUDA 12.2，静态链接的 12.4 运行库仍可运行（CUDA 12 次版本兼容）。没有 GPU 的节点上 `cudaGetDeviceCount` 返回错误，程序能给出提示并改用 CPU。同一份推进代码 GPU 与 CPU 的 fp64 结果相差 1.6e-11（GPU 使用 FMA 合并），若要逐位一致需要 `-fmad=false`。
