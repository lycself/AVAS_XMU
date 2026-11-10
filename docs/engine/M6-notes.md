# 新内核 M6 实现记录：GPU

M6 分三段：**M6a CUDA 后端**（本节）、M6b PICNIC（逆向 libPIC.so，CPU 与 GPU 都实现）、M6c Apple GPU（Metal，metal-cpp）。
M6a 在 M5b（8210c90）之上实现 NVIDIA GPU 的粒子推进、空间电荷与统计量，并为 M6b、M6c 留好接口。

## M6a：CUDA 后端

### 1. 用户的决定与本段范围

2026-11-07 已定：方案 A2（一份 C++ 物理代码，`__host__ __device__` 同时编译成 CPU 与 CUDA，核函数自己写，CUDA 运行库静态链接，
机器码含 sm_61/75/80/86/89/90 并附 PTX，Linux 不依赖 PyTorch）；Pascal（1080Ti）必须支持；A100 集群（CentOS 7 / glibc 2.17、离线、Slurm）必须能直接运行；
精度分 `auto` / `fp64` / `fp32`，用户可手动指定；同一节点多卡单进程管理，跨节点用可选的 MPI；与 CPU 比对的差别在多种子起伏以内，单粒子尽量接近机器精度。

2026-11-08 新定的三项：
1. **Mac GPU 走 Metal**（M6c），不用 PyTorch。因此物理代码里不能有 CUDA 专有写法：设备相关的修饰只经 `src/core/hd.h` 的中性宏。
2. **PICNIC 要实现**（M6b）。本段把空间电荷求解器做成“主机准备 + 设备逐节点计算”的结构，PICNIC 作为第三种 `method` 接入。
3. **GPU 结果逐位可重复**：fp64 档关闭 FMA 收缩、电荷按固定方式累加。目标：无空间电荷时 GPU fp64 与 CPU 默认模式逐位相同；
   有空间电荷时每次运行、任意 GPU 数量都逐位相同。

### 2. 架构与目录

主机负责输入解析、同步粒子与相位扫描、误差抽样、空间电荷网格的设定、全部控制流程和输出文件；设备只持有宏粒子（下标 0 … n−1，同步粒子留在主机），
运行与 CPU 相同的逐粒子函数，求和类统计量按固定分组返回部分和，由主机按相同次序完成。

| 文件 | 内容 |
|---|---|
| `src/core/hd.h`（新） | 中性宏 `AVAS_HD`（CUDA 下为 `__host__ __device__`）、`AVAS_RESTRICT`；数学封装 `hm::sqrt/fabs/ceil/floor`（IEEE 正确舍入，各设备相同）、位转换 `hm::dbits/dfrom` |
| `src/core/pmath.h`（新） | 可移植的 sin/cos/sincos/exp/sinh/cosh/atan（fdlibm 的算法与系数，只用 + − × ÷ 与精确操作），CPU 与 GPU 逐位相同；单元测试与 libm 相比误差 ≤ 1 ulp（sinh ≤ 2 ulp） |
| `src/core/reduce.h`（新） | 固定次序求和（第 4 节）、`round_to_ll` |
| `src/core/kernels.h`（新） | 逐粒子物理：元件场（场图、硬边元件的 t-code 场、superpose、误差场，全部模板化为 `R` = double / float 与 z 参照）、`trans_particle`（TcodeTrans）、UpdateBeamParameters、z-code 行变换 `het_row`、steerer、HESCFieldPre、空间电荷 CIC 分配 / 插值 / 定点电荷、boundary.txt、lpboundary、longlimits |
| `src/core/secondary.h`（新） | 二次粒子 SeRun（`se_trun` / `se_zrun` / SeHardEdgeTrans），输出经 sink，CPU 写文件、GPU 记录 |
| `src/core/sc_kernels.h`（新） | 空间电荷网格求解的逐节点函数：SPICNIC 对称卷积与非对称修正、梯度；FFT 的直接变换（DST-I、z 向 DFT） |
| `src/core/latview.{h,cpp}`（新） | 晶格的平面化副本（`ElemV` / `SubV` / `SpOutV` / `GeomV`、场图视图、RF 相位槽），每步的 RF 因子表 |
| `src/core/device.{h,cpp}`（新） | 设备后端接口 `DeviceBackend`、按选项选择设备、MPI 进程号 |
| `src/core/device_stub.cpp`（新） | 不开 CUDA 时的空实现 |
| `src/core/device_emu.cpp`（新） | 测试设备 `emu`：设备后端在 CPU 上运行（自有粒子副本，可模拟多卡分片），用来在没有 GPU 的机器上检查主机与设备之间的数据往返 |
| `src/cuda/cuda_backend.cu`（新） | CUDA 后端：显存管理、核函数启动、固定次序归约、多卡分片、空间电荷求解、二次粒子、设备选择与报错 |
| `src/cuda/cuda_f32.cuh`（新） | fp32 档的粒子状态与推进（第 5 节） |
| `src/cuda/cuda_mpi.h`（新） | 可选的 MPI 层（`AVAS_WITH_MPI`） |
| `src/core/tracker.cpp` | 逐粒子循环改为调用 `kernels.h`；`dev_` 非空时把这些循环交给设备，读写粒子的输出、重启处显式 `download` / `upload` |
| `src/core/spacecharge.{h,cpp}` | 默认模式：定点电荷分配、共享角点的格林函数表、插值改用 `mesh_interp`；为设备拆出 `begin` / `mesh` / `set_rho` / `spicnic_prepare` / `fft_tables` |
| `tests/gpu/run_gpu.py`（新） | CPU 与 GPU 比对（第 7 节） |
| `tests/gpu/perf_gpu.py`（新） | 性能表（第 8 节） |
| `tools/build_cuda_linux.sh`、`tools/build_cuda_windows.bat`（新） | 构建脚本 |

接口（`avas_set_option` 与环境变量，默认 `device=cpu`，软件默认行为不变）：

| 选项 | 环境变量 | 取值 |
|---|---|---|
| `device` | `AVAS_ENGINE_DEVICE` | `cpu`（默认）、`cuda`（必须用 GPU，不能用时报错并写明原因）、`auto`（能用则用 GPU，否则在日志里写明原因后用 CPU）、`emu`（测试设备） |
| `precision` | `AVAS_ENGINE_PRECISION` | `auto`（默认）、`fp64`、`fp32` |
| `gpus` | `AVAS_ENGINE_GPUS` | 空（第一块可见的 GPU）、`all`、或列表 `0,1` |
| `gpu_atomic` | `AVAS_ENGINE_GPU_ATOMIC` | `1`：只供测量的原子加方案（结果不可重复，第 8 节），不对外 |

`compat=dll` 只在 CPU 上提供：`device=cuda` 时报错 “compat=dll is only available on the CPU”，`device=auto` 时在日志写明后用 CPU。
每次运行的第一行设备信息形如 `Device: CUDA, 1 GPU: 0 NVIDIA L40S (sm_89, 45373 MiB); precision fp64 (auto: …)`。

### 3. 默认模式（CPU）因 GPU 而作的改动

为了让 GPU fp64 与 CPU 默认模式逐位相同、并与 GPU 数量无关，默认模式改了下面几处（compat=dll 完全不变，`run_all.py` 中与改动前的输出逐字节相同）：

| 项目 | 改动前 | 改动后 | 影响 |
|---|---|---|---|
| 统计量的求和次序（TcodeTrans、UpdateBeamParameters、HESCFieldPre、z 段 tRef 平均） | 按 cores 表分块，块内顺序、块间顺序 | 固定树形次序（第 4 节），与线程数、设备、卡数无关 | 统计量末位；无空间电荷时粒子坐标不变 |
| 逐粒子的超越函数：硬边四极 / 螺线管矩阵（主束与二次粒子）、弯管 boundary、二次粒子的 RF 相位 | C 库（Windows 为 DLL 的 SSE2 sincos） | `pmath.h` 可移植函数 | 这些算例末位（≤ 1 ulp 的函数差） |
| 空间电荷电荷分配 | 双精度按粒子顺序累加 | 每次分配换成 64 位定点整数（电荷 × 2^e，e 使 n·max\|q\| ≤ 2^61）整数相加，再换回双精度 | 网格电荷相对差约 2^−61·n（1e7 粒子时单次分配 ~1e−11 相对，远小于 PIC 噪声） |
| SPICNIC 格林函数表 | 每格 8 个角点分别求原函数 | 角点 (q − ½)h 上的原函数只算一次，相邻格共用（数值末位不同，计算量少 8 倍） | 空间电荷场末位 |

每粒子都相同的量（某一步的 RF 相位、元件转角、弯铁矩阵、superposeout 出口转角）仍在主机上用 C 库算一次后传入，所以绝大多数算例的粒子坐标与改动前逐位相同。
默认模式 23 个生成算例的 inData.dst 记录值（`default_indata_sha256`）不受影响，无需重新记录。

改动中顺带保留的行为：`multithreading 0/2/3`（推进块数为 0，粒子全部“丢失”）在默认模式与 GPU 上仍与以前相同；z-code 中空间电荷冲量后的 |γβ| 恢复（第 11 节）照旧。

### 4. 确定性的做法

- **求和**：粒子按下标每 256 个一组（chunk），组内按 `s = 128, 64, …, 1: a[t] += a[t+s]` 的树相加（GPU 一个线程块在共享内存里做完全相同的树）；
  每 16 个 chunk（4096 个粒子）为一个 group，再做同样的 16 项树；各 group 的部分和在主机上按序号依次相加。缺的粒子按 +0.0 计。
  多卡与 MPI 时每张卡 / 每个进程只持有整数个 group，部分和按 group 序号合并，结果与卡数、进程数无关。最大值、最小值与整数计数与次序无关。
- **空间电荷**：电荷分配是 64 位整数原子加（整数加法满足结合律），多卡与多进程之间也是整数相加；SPICNIC 每个目标点由一个线程按与 CPU 相同的循环次序求和，
  非对称修正表在主机上按 CPU 的次序生成；插值逐粒子独立。FFT 用固定次序的直接变换，与 CPU 的 pocketfft 末位不同（第 8 节），但在任何 GPU 上都相同。
- **算术**：nvcc `--fmad=false`，CPU 端 `-ffp-contract=off` / `/fp:precise`；GPU 上双精度除法与开方本来就是 IEEE 正确舍入。
- **粒子损失**：只置标记，不压缩数组，结果与调度无关。

### 5. 精度分档

- **auto 的规则**：所选 GPU 全部是 fp64 全速的 HPC 卡（计算能力 6.0、7.0、8.0、9.0：P100、V100、A100 / A30、H100）时用 fp64，否则用 fp32；
  日志写明实际档位与原因（“requested” 或 “auto: …”）。
- **fp64**：与 CPU 默认模式的运算逐项相同（第 3、4 节）。
- **fp32**：位置与动量以同步粒子为参照（x − x_s、u − u_s，float），元件场在 float 中计算（同一份模板代码，晶格位置减去同步粒子 z 后由主机在双精度中算好、每步传入），
  场图另存一份 float。Boris 推进写成相对于同步粒子自身推进的差（半步加速 h·E − h_s·E_s，转动 (K − K_s)·u_s⁻ + K·Δu，γ 差 Np/(γ + γ_s)），
  不在 float 里出现两个大数相减。保持双精度的量：时间与 RF 相位（主机）、同步粒子、穿越记录（输出面、superposeout，按双精度重建绝对坐标后插值）、统计量的求和、
  空间电荷网格（电荷定点分配、FFT 双精度；SPICNIC 卷积在 fp32 档用 float）。z-code（硬边矩阵）段始终用 fp64。
- **误差预算**：见第 7 节的实测。

### 6. 构建、工具链与驱动

- CMake 选项 `AVAS_WITH_CUDA`（默认 OFF；OFF 时源文件只多了不含 CUDA 的 `device*.cpp`，不链接任何 CUDA 库）。需要 CMake ≥ 3.18、CUDA 12.x（CUDA 13 不支持 Pascal）。
  机器码 sm_60 / 61 / 70 / 75 / 80 / 86 / 89 / 90 并附 compute_90 的 PTX（比 2026-11-07 的计划多了 sm_60、sm_70，使 P100、V100 不必走 JIT），
  CUDA 运行库静态链接（`CMAKE_CUDA_RUNTIME_LIBRARY Static`），nvcc `--fmad=false`。
  `AVAS_WITH_MPI`（默认 OFF）加入 MPI；`AVAS_MPI_INCLUDE` + `AVAS_MPI_LIBRARY` 可绕过 FindMPI（只用 MPI 的 C 接口）。`AVAS_CUDA_FMAD=ON` 只用于测量（第 8 节）。
- **不用 cuFFT**：FFT 求解用固定次序的直接变换核函数。理由：cuFFT 的结果随 GPU 架构选用的核而变，做不到“任何 GPU、任意卡数逐位相同”；
  静态 cuFFT 约 100 MB 以上，会使发布包增大数倍；网格 ≤ 128³ 时直接变换的运算量（64³ 约 1e8 次乘加）对 GPU 不是瓶颈。
- **Linux（glibc 2.17）**：两条路线。(a) 既定的 manylinux2014 + CUDA 12.4 容器（`docker/…Dockerfile`，`tools/build_cuda_linux.sh`），用 devtoolset 的 libstdc++（系统 libstdc++ + nonshared 静态部分）；
  (b) 本段实际使用的离线路线：conda-forge 的 GCC 12（sysroot 2.17）+ NVIDIA 的 cuda-nvcc 12.4，`-static-libstdc++ -static-libgcc -Wl,--exclude-libs,ALL`。
  路线 (b) 在 A100 集群登录节点上构建：产物只依赖 GLIBC_2.14、不依赖 GLIBCXX，21 MB（全部架构）。
  **`-Wl,--exclude-libs,ALL` 是必需的**：静态 libstdc++ 的符号若导出，在已载入另一份 libstdc++ 的 Python 进程里会被其覆盖，A100 上 `pytest` 经 ctypes 调用时在 `std::ostream` 中崩溃（已复现并修正）。CMakeLists 在 Linux 上对 `avas_engine` 自动加这个选项，并用版本脚本 `engine/src/exports.map` 只导出 7 个 C 接口（`-fvisibility=hidden` 藏不住 std 模板的弱符号实例，`.cu` 另需 `CUDA_VISIBILITY_PRESET hidden`；A100 上核实 `nm -D` 只剩这 7 个，与 PyTorch 2.6 同进程加载正常），不必写进命令行；
  conda GCC 路线还要用 `-DCMAKE_CUDA_HOST_COMPILER` 指向同一个 g++，否则 nvcc 用系统 GCC 4.8，编译 `.cu` 失败。
- **Windows**：VS 2022 Build Tools + CUDA 12.x，`tools\build_cuda_windows.bat`（未能实测，见第 7 节）。
- **工具链位置**（项目结束时清理）：A100 集群 `~/avas_engine_work/toolchains/cuda124`（conda 离线环境：cuda-nvcc / cudart / cccl 12.4、gcc/g++ 12.4、sysroot 2.17、cmake 3.29、make）；
  L40S 上的 `/data/public/lycself/avas_engine_work/toolchains/` 与 docker 镜像为 2026-11-07 已有的（本段未能登录）。
- **最低驱动**：Linux 525.60.13、Windows 527.41（CUDA 12.0 起的次版本兼容；静态链接 12.4 运行库在驱动 535（CUDA 12.2）上可运行，2026-11-07 已验证）。
  没有驱动时报 “no NVIDIA driver found”，驱动过旧时报所需版本，GPU 早于 sm_60、显存不足（按粒子数估算）、`gpus` 列表无效时各有明确报错；`device=auto` 时这些原因写进日志后改用 CPU。

### 7. 比对结果

**CPU 不退化**（macOS arm64）：`run_all.py` 通过（compat=dll 与 AVAS.dll 最大相对差 3.44e-11，与 M5b 相同）；compat=dll 的全部输出与改动前（8210c90）逐字节相同；
默认模式全部检查通过（inData 记录值 23/23 不变、能量守恒、误差样本、边界与二次粒子、线程无关性）；单元测试（新增 pmath 与固定次序求和）、`run_physics.py`、`tools_check.py`、
`ruff check avas engine/tests engine/tools`、`tests/test_error_study.py`（`AVAS_ENGINE=new`）通过。默认模式中只有第 3 节所列的算例末位变化（无空间电荷的 154 个算例中，粒子坐标变化的只有含硬边四极 / 螺线管、二次粒子四极的算例）。
GCC 12（A100 上构建）无警告；Clang（macOS）无新警告。**Windows MSVC 的 `run_all.py --strict` 与三种方式的 `test_error_study.py` 本段未能运行**（XPS 无法连接，见第 11 节）。

**GPU fp64**（A100-PCIE-40GB，sm_80，`run_gpu.py --gpus2 0,1 --mpi "mpirun -np 2"`，Intel MPI 2021.6）：

| 检查 | 结果 |
|---|---|
| 无空间电荷的 154 个算例，GPU 与 CPU 默认模式的全部输出文件 | **154/154 逐字节相同** |
| 有空间电荷的 19 个算例：再运行一次 / 2 卡 / MPI 2 进程 | **全部逐字节相同** |
| 有空间电荷：GPU 与 CPU | SPICNIC 的 13 个算例逐字节相同；FFT 的 6 个（含 lpboundary）DataSet 文本相同、粒子坐标差 ≤ 2.4e-13（以 rms 计） |
| 经 Python 的误差研究 `tests/test_error_study.py`（`AVAS_ENGINE=new AVAS_ENGINE_DEVICE=cuda`） | 3 passed（15 次 GPU 运行） |
| 错误路径 | 无驱动、`compat=dll` + `cuda` 报错并写 ErrorLog；`auto` 写明原因后用 CPU |

“2 卡”与“MPI 2 进程”在 5260 个粒子的算例中各拿到一个 4096 粒子的组，20000 粒子的 SC9 为 2 组 / 3 组，均为真实的分片。
同一组比对在没有 GPU 的 Mac 上用测试设备 `emu`（1 片与 2 片）也全部通过（FFT 的直接变换与 GPU 结果相同）。

**GPU fp32**（A100，`--fp32 --stats 8`）：

- 单粒子误差（与 CPU fp64 比，跟踪 0.3–0.7 m）：位置 ≤ 1.2e-8 m，角度 ≤ 1.2e-7 rad，相位 ≤ 1e-6°，能量 ≤ 2.3e-7 MeV（相对 1.2e-7）；
  多粒子（5260–20000 粒子，含空间电荷、四极、混合段）单个粒子的最大差：位置 ≤ 3.9e-8 m，角度 ≤ 1e-7 rad，相位 ≤ 1.5e-6°，能量 ≤ 7e-7 MeV。
- 多种子统计（20000 粒子、两周期 sol + hwr010、8 个种子，无空间电荷 / 5 mA）：出口 rms x/y/z、三个平面的发射度、传输率，fp32 与 CPU 的均值之差 ≤ 4e-9 m（rms）、
  ≤ 1.1e-12 π·m·rad（发射度），而种子间起伏为 5e-6 m、1e-9 量级——差别比统计起伏小 3 个数量级以上；同一种子的最大差 1e-8（rms）。传输率完全相同。
- DataSet 各列与 CPU 的最大差为该列最大值的 3e-5–9e-5（主要在均值列）。
- auto 档：A100 选 fp64（“auto: full-rate fp64 GPU”），与预期一致。其他显卡（预期 fp32）未能实测。

### 8. 性能

示例 hwr010 的两周期 sol + hwr010 线（1.72 m，约 746 步，`perf_gpu.py`），GS 分布，空间电荷 SPICNIC（默认网格：1e5 为 20³，1e6 为 30³，1e7 为 44³）。
表中为推进与空间电荷的时间（SimulationTime.txt 的 Exter 项）除以步数；括号里是整个跟踪的时间（含输出文件，1e7 粒子时写 FullParticle.txt 与 outData 约需 25 s，CPU 与 GPU 相同）。

| 配置 | 1e5 无 SC | 1e6 无 SC | 1e7 无 SC | 1e5 SC | 1e6 SC | 1e7 SC |
|---|---|---|---|---|---|---|
| A100 fp64，1 卡 | 0.25 ms（1.1 s） | 1.26 ms（4.0 s） | 11.3 ms（37 s） | 2.08 ms（2.3 s） | 6.2 ms（7.7 s） | 26.6 ms（49 s） |
| A100 fp64，2 卡 | 0.18 ms | 0.79 ms | 6.0 ms（35 s） | 2.49 ms | 6.0 ms | 20.5 ms（44 s） |
| A100 fp32，1 卡 | — | 1.09 ms | 9.7 ms | — | 5.8 ms | 23.2 ms |
| A100 节点 CPU（Xeon Gold 6226R，16 线程） | 5.35 ms | 29.5 ms | — | 33.5 ms | 216 ms | — |
| L40S fp32 / fp64、CPU 32 线程、12700K、1080Ti fp32、GTX 1650 fp32 | 未能测量（机器无法连接） | | | | | |

- A100 一卡比同节点 16 线程 CPU 快 23×（1e6 无 SC）与 35×（1e6 SC）。两卡对无空间电荷 1e7 为 1.9×；有空间电荷时每步约 14 ms 不随卡数减少（主机上的 SPICNIC 格林函数表与非对称修正表、网格下载 / 上传，以及每张卡重复的网格求解），
  两卡只快 1.3×。这部分可以移到 GPU（M6b 一并考虑）。fp32 在 A100 上只快 5–15 %（A100 的 fp64 是全速的，推进受访存限制）；fp32 的收益在消费级显卡上，未能实测。
- **逐位可重复的代价**（A100，1e6 / 1e7）：只供测量的原子加方案（`AVAS_ENGINE_GPU_ATOMIC=1`：块内求和后按完成次序原子加到一个累加器、电荷按双精度原子加）
  与固定次序方案相比为 +9 % / +3 %（无 SC）与 +1 % / +2 %（SC），即没有更快；`AVAS_CUDA_FMAD=ON`（允许 FMA）为 −1.6 % / −0.3 %（无 SC）、+0.3 % / −1.7 %（SC）。
  两者都在运行间起伏（约 ±3 %）以内：**逐位可重复在 A100 上基本没有代价**，远低于预计的 10–30 %。原因：推进受访存与分支限制，FMA 节省的运算不在关键路径上；
  固定次序归约只多一个 16 项的小核函数与每步约 0.5 MB 的部分和下载。

### 9. GPU 上不能运行或有限制的项

| 项目 | GPU 上的处理 |
|---|---|
| `compat=dll` | 报错（`device=cuda`）或在日志写明后用 CPU（`device=auto`） |
| `impscfield 1` | 每次求解把粒子下载到主机读记录场，再上传（能运行，较慢） |
| `pchistogram`、`track`、`dumpperiodicity` | 需要时下载粒子由主机写文件（能运行，较慢） |
| 二次粒子（`secondarybeam 1`） | 每个 GPU 线程跟踪一个粒子走完全程；Windows 上作显示用的 GPU 有约 2 s 的看门狗，单个粒子极长的轨迹可能超时 |
| MPI | 只用于 GPU 运行；各进程都运行完整的主机程序，0 号进程写输出，其他进程在输出目录下的临时子目录工作、结束时删除 |
| fp32 | z-code 段、FFT、二次粒子用 fp64 |

### 10. 发现的问题

1. **默认模式 z-code 中空间电荷冲量的能量被丢掉**（M2 + M4 的组合）：硬边元件的行变换在变换前记下 |γβ|²、变换后按新斜率恢复（M2 的“磁元件中 |γβ| 守恒”），
   但空间电荷冲量（以及误差元件的 T_in）在“记下”之后、恢复之前施加，于是冲量改变的 |γβ| 被恢复回去，只留下方向的变化。CPU 默认模式一直如此；本段保持不变（GPU 与之逐位相同），
   建议改为在冲量之后记下 |γβ|²（见第 11 节第 4 项）。compat=dll 不受影响。**已按用户决定修正**（第 12 节）。
2. A100 集群上的静态 libstdc++ 符号冲突（第 6 节），已修正。
3. `multithreading 0/2/3` 时默认模式仍然不推进任何粒子（原内核缺陷的表现），本段为保持 CPU 结果不变未改，GPU 同样处理。

### 11. 本段未完成的验收与待用户决定

**未完成**：L40S、1080Ti 主机、XPS（Windows）在本段工作期间一直无法从 Mac 连接（ssh 超时，互联网与 A100 跳板正常），因此以下验收未做：
Windows MSVC 的 `run_all.py --strict` 与 DLL / 默认 / compat 三种方式的 `test_error_study.py`、MSVC 下的 CUDA 构建与 GTX 1650 运行；
manylinux 容器内的构建；L40S、1080Ti（Pascal、双卡）、GTX 1650 的 fp64 比对与 fp32 性能，CPU 32 线程与 12700K 的性能；这些机器上 auto 档选 fp32 的确认。
A100 上构建的 `.so`（全部架构，glibc 2.14）可直接拿到 L40S / 1080Ti 运行；脚本 `run_gpu.py`、`perf_gpu.py` 不需改动。

**待用户决定**（每项附推荐）：

1. **`gpus` 的默认值**：推荐默认只用第一块可见的 GPU，多卡须显式写 `gpus=all` 或列表（共用服务器上不会意外占满所有卡）；MPI 时默认每个进程一块卡。
2. **FFT 求解器**：推荐保持 GPU 上的直接变换（确定性、任意卡数相同、不增大发布包），接受与 CPU pocketfft 的 ~1e-13 差别；另一选择是 GPU 把电荷下载到主机用 pocketfft，与 CPU 逐位相同但每步多一次往返、64³ 时慢约 10 ms。
3. **默认模式（CPU）的数值改动**（第 3 节：固定树形求和、四极 / 螺线管等的可移植超越函数、定点电荷分配、共享角点的格林函数表）：推荐采用，它们是 GPU 与 CPU 逐位一致的前提，变化只在末位，compat=dll 不变。
4. **z-code 空间电荷冲量的 |γβ| 恢复**（第 10 节第 1 项）：推荐在默认模式改正（冲量之后再记下 |γβ|²），改动会改变含硬边元件 + 空间电荷算例的默认结果。
5. **fp32 的适用范围**：推荐 auto 规则保持“HPC 卡 fp64、其他 fp32”，z-code、FFT、二次粒子在 fp32 档也用 fp64（实测误差预算见第 7 节）。
6. **主机侧的空间电荷准备**（SPICNIC 表、修正表）：推荐在 M6b 实现 PICNIC 时一并移到 GPU（1e7 粒子时每步约 14 ms，是多卡加速的主要瓶颈）。
7. **测试设备 `emu`**：推荐保留（没有 GPU 的机器上检查设备通路），不写进用户文档。

### 12. 用户决定（2026-11-08）

七个待定点全部按推荐：

1. `gpus` 默认只用第一块可见的 GPU（`options.h` 中 `gpus` 为空即此含义，原已如此），多卡写 `gpus=all` 或列表；MPI 时每个进程一块卡。
2. GPU 上保持固定顺序的直接 FFT，接受与 CPU pocketfft 约 1e-13 的差别。
3. 默认模式（CPU）采用第 3 节的数值改动。
4. **z-code 空间电荷冲量的 |γβ| 恢复已在默认模式改正**（`kernels.h` 的 `het_row`）：施加冲量后重新记下 |γβ|²，磁元件的传输只改变方向，冲量带来的能量变化保留。
   T_in 是转动、不改变 |p|，没有冲量的行仍沿用原来记下的值，所以无空间电荷的算例逐位不变；CPU、emu 与 CUDA 共用这一函数，GPU 与 CPU 仍逐位一致。compat=dll 不变。
   对默认结果的影响（Mac，改动前后末端 DataSet）：SC5、SC6、SC11、SC14、ER16 这些含硬边元件 + 空间电荷的算例，rms z 变化 0.02 %～0.24 %，rms x、y 约 1e-5，末端平均能量 6 位有效数字不变，存活粒子数不变。
5. fp32 的适用范围保持：auto 在 HPC 卡（sm 6.0 / 7.0 / 8.0 / 9.0）用 fp64，其他用 fp32；z-code、FFT、二次粒子在 fp32 档也用 fp64。
6. 主机侧的 SPICNIC 表与修正表在 M6b 实现 PICNIC 时一并移到 GPU。
7. 测试设备 `emu` 保留，不写进用户文档。

## M6b：PICNIC

用户的决定（2026-11-08）：PICNIC 在新内核的 CPU 和 GPU 上都实现，以原 libPIC.so 的运行结果为参考；M6a 第 6 项（主机侧的 SPICNIC 表与修正表移到 GPU）在本段完成。
libPIC.so 的规格见 [65-libpic.md](65-libpic.md)。

### 1. libPIC.so 的 PICNIC（要点，详见 65 第 8 节）

网格与 AVAS.dll 的 SPICNIC 相同（束流静止系，均值 ± meshrms·σ，CIC），求解是网格电荷与系数表的直接卷积：系数表的自身单元为 0、近场用 8³ 中点平均、
半径外用点值，核函数在每个方向截断在约 0.75 N 个单元；电荷用双精度原子加（不可重复），电势中心差分求场、边界节点为 0、场存为 float；网格外的粒子用点电荷场。
S2PICNIC / S4PICNIC 是只算半个 / 四分之一截面再加非对称修正的加速版本，不对称单元超过 1/50 时退回完整 PICNIC。按这些写的模型重算的场与原库实测相差 1e-4（65 §9.2），
所以这些结论已经核实。

### 2. 实现

| 文件 | 内容 |
|---|---|
| `src/core/params.cpp` | 默认模式接受 `scmethod PICNIC`（不区分大小写，与 libPIC 相同），`scPar[0] = 2`；`compat=dll` 仍报 “SCMethod Contains unknown item”（F-01 不变） |
| `src/core/spacecharge.{h,cpp}` | method 2：网格、分配、插值与 SPICNIC 相同；`picnic()` 每步制表（与修正后的 SPICNIC 同一张精确单元平均表，核函数覆盖整个网格）后逐点调用 `sck::picnic_node`，再用 SPICNIC 的差分（边界单侧差分）；不做 autoPIC 跳步（libPIC 每步都求）；网格点数不要求偶数；不写网格时用 SPICNIC 的默认网格（`space_charge_defaults`） |
| `src/core/sc_kernels.h` | 新增 `picnic_node`（整个网格的源按 x、y、z 次序累加，跳过空节点）、`box_F` / `green_corner` / `green_cell`（格林函数表，CPU 的 `tabulate()` 也改用它们）、`corr_value`（SPICNIC 修正表的一项） |
| `src/core/pmath.h` | 新增可移植的 `p_log`（fdlibm），格林函数表的 log / atan 改用 `p_log` / `p_atan`，使主机与设备得到相同的位 |
| `src/core/device.h`、`device_emu.cpp`、`src/cuda/cuda_backend.cu` | `ScSolveIn` 不再带主机的格林函数表和修正表，改为带单元体积与阈值；设备上算表（`k_green_corner`、`k_green_cell`）与修正表（`k_corr_val` 后用 CUB `DeviceSelect::Flagged` 稳定压缩，顺序与 CPU 相同）；PICNIC 的网格点在同一进程的几块 GPU 间均分，电势经主机汇总后发给每块卡 |
| `src/core/tracker.cpp` | 设备求解不再在主机上制表 |
| `tests/parity/cases.json`、`run_all.py` | 新内核算例 PN1（默认网格，漂移 + 腔）、PN2（z-code 四极，16³，值写成小写 `picnic`）、PN3（奇数网格 15×17×21）；PN1、PN2 加入线程无关性检查 |
| `tests/unit_tests.cpp` | `p_log` 精度；PICNIC 与阈值为 0 的 SPICNIC（同一张表，八分之一求和加修正）的场相差 1.1e-14 |
| `tests/physics/` | 椭球与 DC KV 检验加 PICNIC（见第 5 节）；`run_physics.py --build DIR` |
| `tests/gpu/perf_gpu.py` | `--scmethod` |
| `tools/ExportDecomp.java` | 跳过 CUB / Thrust / cudart 的内部函数 |

**确定性**：电荷分配沿用 M6a 的定点整数累加；每个网格点的电势是同一个函数按固定次序求和，与在哪个线程、哪块 GPU 上算无关；GPU 编译 `--fmad=false`。
因此 PICNIC 的结果与线程数、GPU 数、MPI 进程数无关，**CPU 与 GPU fp64 逐位相同**（第 4 节）。fp32 档在 GPU 上按 float 求和（同 SPICNIC）。

**修正的 libPIC 缺陷**（不提供复现开关，见第 6 节第 4 项）：精确的单元平均系数表（含自身单元）、不截断、边界节点单侧差分、双精度场、可重复的分配与全粒子统计、网格点数按输入。

**默认模式 SPICNIC 的变化**：CPU 的格林函数表改用 `p_log` / `p_atan`（原为 C 库 log / atan），SPICNIC 的默认结果只在末位变化（`run_all.py` 的默认模式检查全部通过）；
好处是各平台的 C 库不再影响这张表。compat=dll 不变（compat 用 AVAS.dll 的 10³ 中点表，不经过这里）。

### 3. 参考运行（libPIC.so，A100）

方法与全部数字见 65 §9：ctypes 调原库导出的 `main`，单进程 singleton MPI（用户态 OpenMPI 4.1.6，cudart 11 与 cufft 10 来自 `module load cuda/11.4`），
同一份 dst（质子 3 MeV、30 mA、1e5 粒子、5 个种子），1 m 漂移，32³、meshrms 4。结论：

- **同一束流的网格场**：新内核 PICNIC 比原库横向强 3.3 %、纵向强 1.2 %；逐项分解后差别完全来自三处修正（系数表 3.6 %、截断 0.2 %、边界节点 0.9 %，三者合起来与新内核相差 3e-8）。
- **统计**：出口 rms x 新内核小 0.97 % ± 0.14 %、ε x 大 0.54 %，超出种子起伏，原因已查明：① libPIC 从第一步起就对还在入口上游的束团计空间电荷，新内核（沿用 AVAS.dll）
  在束团进入 lattice 的约 20 步内 rms x′ 不变；② 束团完全进入后 rms x′ 的增长新内核大 3.3 %、rms z′ 大 0.7 %，与场强之比一致。传输率都是 100 %。
- libPIC 的 s4picnic 与 picnic 在种子起伏以内；libPIC 的 fft 是三维周期、单精度，与开放边界相差 5–18 %。
- 物理检验见第 5 节：新内核 PICNIC 收敛到解析解；原库的 PICNIC 没有单独做椭球检验（它的场与新内核的差别已逐项量出）。

### 4. 检查结果

**Mac（arm64，Clang）**：构建无新警告；单元测试、`run_all.py`（compat=dll 与 AVAS.dll 最大相对差 3.44e-11 不变，默认模式检查全部通过，inData 记录值 23/23 不变，
PN1 / PN2 线程无关）、`run_physics.py`、`tools_check.py`、`uvx ruff@0.6.9 check avas engine/tests engine/tools` 通过。
**A100 构建（GCC 12 + nvcc 12.4）**：无新警告；`nm -D` 只有 7 个 C 接口，只依赖 GLIBC_2.14。

**GPU（A100，`run_gpu.py --gpus2 0,1 --mpi "mpirun -np 2"`，M6b 构建）**：

构建包含 dcc9132 的全部改动（`het_row` 的 |γβ| 恢复），所以这也是 dcc9132 在 GPU 上的重跑。176 个默认模式算例（22 个有空间电荷）：

| 检查 | 结果 |
|---|---|
| 无空间电荷的 154 个算例，GPU 与 CPU 的全部输出文件 | **154/154 逐字节相同** |
| 有空间电荷的 22 个算例：第二次 GPU 运行、GPU 0,1 双卡、Intel MPI 2 进程 | **全部与第一次 GPU 运行逐字节相同** |
| PICNIC（PN1、PN2、PN3、EP_F01）GPU 与 CPU | **逐字节相同** |
| SPICNIC（SC1、3–7、9、10、12、ER16，格林函数表与修正表改在设备上算）GPU 与 CPU | **逐字节相同**（与 M6a 相同） |
| 含 hard-edge + SC 的算例（SC5、SC6、ER16，dcc9132 改动所涉） | GPU 与 CPU 逐字节相同 |
| FFT（SC2、8、11、13、14、LP2、LP3）GPU 与 CPU | dst 相差 7e-16～1.9e-11（rms），DataSet 相同，与 M6a 相同（直接变换对 pocketfft） |

Mac 上用 emu 设备（`run_gpu.py --device emu`）同样得到 PICNIC、SPICNIC 与 CPU 逐位相同。
fp32（`--fp32`，PN1–PN3、SC1、SC9）：DataSet 与 CPU 相差 3e-5～9e-5（列最大值的比例），dst 的粒子坐标 PN1、PN2 相差 1.5e-5、5e-6 rms，PN3（奇数小网格、meshrms 3.5）
最大 5e-3 rms——是单个粒子在网格边缘按 float 落到网格内外的不同一侧（网格场与点电荷场的切换），统计量不受影响。

### 5. 物理检验（Mac，`run_physics.py`）

| 检验 | 结果 | 阈值 |
|---|---|---|
| 均匀带电椭球（1 / 1.5 / 3 mm），PICNIC 16³ / 24³ / 32³ | 最大偏差 2.0 % / 0.54 % / 0.19 %，随网格单调减小 | 32³ 时 ≤ 1 % |
| DC KV 束 1 m 漂移，PICNIC 24³、8 万粒子 | 空间电荷增长与 KV 包络之比 x 0.984、y 0.984 | 与 KV 相差 ≤ 2 % |

SPICNIC、FFT 的检验结果不变（SPICNIC 40³ 0.34 %，FFT meshrms 13 为 0.99 %，DC KV SPICNIC 40³ 为 0.993）。

### 6. 性能（A100 fp64，hwr010 两周期线、`perf_gpu.py`，推进 + 空间电荷每步时间）

| 配置 | 1e6（30³） | 1e7（44³） |
|---|---|---|
| SPICNIC，dcc9132（主机制表），1 卡 | 6.18 ms | 27.7 ms |
| SPICNIC，M6b（设备制表），1 卡 | **3.68 ms** | **22.5 ms** |
| SPICNIC，dcc9132，2 卡 | 5.76 ms | 21.3 ms |
| SPICNIC，M6b，2 卡 | **2.93 ms** | **14.9 ms** |
| PICNIC，M6b，1 卡 | 3.93 ms | 24.3 ms |
| PICNIC，M6b，2 卡 | 3.30 ms | 14.7 ms |

- SPICNIC 准备移到 GPU 后，1e7 粒子每步快 5.2 ms（1 卡）/ 6.5 ms（2 卡），双卡加速比从 1.30× 提高到 1.51×；1e6 每步快 2.5 / 2.8 ms。
- PICNIC 在 GPU 上只比 SPICNIC 慢约 1.7 ms/步（1e7，44³），双卡时网格点在两卡间均分，与 SPICNIC 相当。CPU 上 PICNIC 的计算量 ∝ 非零源 × 网格点，32³ 以上明显慢于 SPICNIC，
  大网格建议在 GPU 上用。
- 参考：原 libPIC.so 在 1e5 粒子、32³ 时 PICNIC 约 1.2 ms/步（截断的核函数、原子加）。

### 7. 工具链与临时数据

- **工具链（保留）**：A100 `~/avas_engine_work/toolchains/cuda124`（M6a）；`~/avas_engine_work/toolchains/openmpi4`（本段新增：conda-forge 的 OpenMPI 4.1.6，含 `libmpi.so.40`、
  libstdc++ 14，以及 `cuobjdump` / `nvdisasm`；运行原 libPIC.so 时 `LD_LIBRARY_PATH` 依次放 `/public/apps/cuda/cuda-11.4/lib64` 与该目录的 `lib`，设 `OPAL_PREFIX` 为该目录）。
- 临时数据（本段结束时已删除）：A100 的 `~/avas_engine_work/tmp_m6b`（libPIC.so 副本、SASS 导出、源码包、两套构建、参考运行与性能 / 检查的运行目录，约 2.4 GB）
  以及本段 conda 安装留在 `~/.conda/pkgs` 中的 16 个包（只删本段的包名）；Mac 上的 Ghidra / JDK 下载、伪代码导出、构建与运行目录。L40S、lyc-pc、XPS 本段未能登录，没有产生临时数据。

### 8. 本段未完成的验收

- L40S、1080Ti 主机、XPS（Windows）在本段开始、中途和结束时都无法连接（ssh 超时），M6a 遗留的验收仍未做：Windows MSVC 的 `run_all.py --strict`、
  `test_error_study.py` 的 DLL / 默认 / compat 三种方式、MSVC 下的 CUDA 构建与 GTX 1650、manylinux 容器构建、L40S / 1080Ti（双卡）/ GTX 1650 的比对与性能；MSVC 是否有新警告也未知。
- A100 上 dcc9132 的 GPU 全套用 M6b 的构建重跑（包含 dcc9132 的全部改动，第 4 节），没有再单独跑 dcc9132 的构建。
- 原 libPIC.so 只跑了漂移段的空间电荷参考；RF 场图、硬边元件、bend、输出面、多 rank 的行为未实测（65 §9.5）。
- PICNIC 在多个 MPI 进程间不分摊网格点（第 9 节第 6 项）。

### 9. 待用户决定（每项附推荐）

1. **`numofgrid` 的含义**：libPIC 向上取到 8 的倍数，新内核按输入。推荐按输入（用户写的网格就是实际网格）。
2. **PICNIC 不写网格时的默认值**：libPIC 没有默认值（未初始化），新内核用 SPICNIC 的默认网格（按粒子数 14³～64³、meshrms 3.5～7）。推荐保持。
3. **`s2picnic` / `s4picnic`**：推荐不实现，继续报 “unknown item”，提示改用 PICNIC（它们是 PICNIC 的近似，场差 0.5–0.7 %，新内核 PICNIC 已足够快）；`spicnic` 继续表示 AVAS.dll 的 SPICNIC（libPIC 里它是 s4picnic 的别名，两者含义不同，在 65 中已注明）。
4. **是否提供复现 libPIC 缺陷的兼容开关**（8³ 中点表、自身单元 0、截断、边界 0、float）：推荐不提供（没有依赖原库逐位结果的用户；差别已逐项量出并写进规格）。
5. **束团进入 lattice 时的空间电荷**：libPIC 从第一步起对入口上游的束团计空间电荷，新内核沿用 AVAS.dll。推荐保持 AVAS.dll 的约定（与 CPU 内核和全部既有结果一致）。
6. **多 MPI 进程的 PICNIC**：现在每个进程都求整个网格（同一进程的多卡已分摊网格点）。推荐暂不改；多节点需要时再按 MPI 进程分摊网格点并用 Allgatherv 汇总电势。
7. **默认模式下 `scmethod` 的值是否不区分大小写**：PICNIC 已不区分；推荐 FFT / SPICNIC 也不区分（无害），compat=dll 仍区分。
8. **libPIC.so 其余的独有功能**（65 §11.3 的表）：推荐只实现 `statoutputinterval`（每 N 步写一行 DataSet）与 BeamSet 大小预估警告，其余不实现。

### 10. 用户决定（2026-11-08）

第 9 节的 8 个待定点全部按推荐。落实情况（compat=dll 都不受影响）：

1. **`numofgrid` 按输入**，不取到 8 的倍数（原已如此）。
2. **PICNIC 不写网格时用 SPICNIC 的默认网格**（原已如此）。
3. **`s2picnic` / `s4picnic` 不实现**：默认模式报错 “SCMethod s2picnic (S2PICNIC / S4PICNIC of the GPU kernel libPIC.so) is not supported, use PICNIC”；
   compat=dll 仍是 AVAS.dll 的 “SCMethod Contains unknown item, available options include FFT / SPICNIC”。默认模式其他未知值的报错列出 FFT / SPICNIC / PICNIC。
4. **不提供复现 libPIC 缺陷的开关**。
5. **束团进入 lattice 时沿用 AVAS.dll 的约定**（不改动）。
6. **多 MPI 进程的 PICNIC 暂不分摊网格点**（不改动）。
7. **默认模式下 `scmethod` 不区分大小写**（`fft`、`Spicnic`、`picnic` 都可以）；compat=dll 仍只认 `FFT`、`SPICNIC`，F-01 等算例不变。
8. **libPIC 的其余功能**（65 §11.3 已改为已定）：
   - `statoutputinterval N`（`params.cpp` / `tracker.cpp`，只在默认模式识别）：只写步号（DataSet 列 39）是 N 倍数的行，0 不写，第一行总写、不补最后一行，与 libPIC 相同；
     新内核每行描述推进前的状态、z-code 硬边元件段没有行，与 libPIC 的差别写在 65 §5.1。统计照常每步计算，所以 CPU、emu、CUDA 自动一致；默认 1 时输出逐字节不变。
     compat=dll 当作未知关键字（警告 “input.txt Contains unknown keywords”，无作用），与 AVAS.dll 相同。
   - **BeamSet.plt 大小预估警告**（`tracker.cpp` 的 `beamset_estimate`，只警告、不拒绝运行）：大小 = 34 + (⌊步数 / dumpperiodicity⌋ + 1)·(25 + 56 N) 字节，
     步数 = lattice 长度 / (β c dt) + 束团时长 / dt（β 取同步粒子入口值，加速线上偏多）；超过 50 GB（50·2^30 字节）时在日志写
     “Warning: BeamSet.plt is estimated at … GB …”。阈值可用选项 `beamset_warn_gb`（环境变量 `AVAS_ENGINE_BEAMSET_WARN_GB`）改，测试用 0。parity 算例上估计 / 实际 = 0.94。
   - **libPIC 调优关键字**（`usingaggregation`、`arrangesequenceinterval`、`blockgroupsize`、`numsamplingcoef`、`numbufferpool`、`bufferpoolsize`、`fieldpath`、`outputpath`、`beampath`、`scananglestep`）：
     新内核原本对任何未知关键字都警告 “input.txt Contains unknown keywords” 并忽略，已与表中“警告并忽略”一致；默认模式现改为点名的警告
     “input.txt: keyword X of the GPU kernel libPIC.so is not used by this engine, ignored”，compat=dll 不变。
   - 其余各项不实现。

**测试**：
- parity 新增 PX1–PX8（新内核算例）：PX1 写 `s2picnic` 时报错；PX2 `fft`、PX3 `Spicnic` 与 SC2 / SC1 逐字节相同；PX4 `statoutputinterval 1` 加 `usingaggregation` 时与 SC1 逐字节相同，并有点名警告；
  PX5（N = 7）、PX6（N = 0）、PX7（N = 3，z-code 四极）的 DataSet 行恰为参考算例中步号是 N 倍数的行，outData 逐字节相同；PX5 在 compat=dll 下与 SC1 相同并有未知关键字警告；
  PX8 的 BeamSet 警告与估计 / 实际之比。这些都由 `run_all.py` 的 `libpic_keyword_check` 检查。
- `run_engine.py` 支持算例的 `env`（每个算例的引擎选项）。
- `run_all.py` 不给 `--work` 时，临时目录（约 0.5 GB）在结束时自动删除（`--keep` 保留）；`run_gpu.py` 同样；`dc_kv_envelope.py` 在内核失败时也删除临时目录。
  此前 `$TMPDIR` 里残留的 `avas_parity_*` / `avas_dckv_*` 来自这两处。

**结果**：
- Mac：构建无新警告；单元测试、`run_all.py`（libPIC 关键字检查 10 项全部通过；compat=dll 与 AVAS.dll 最大相对差 3.44e-11 不变，inData 记录值 23/23 不变）、
  `run_physics.py`、`tools_check.py`、`uvx ruff@0.6.9 check avas engine/tests engine/tools` 通过；emu 设备上 PX 算例与 CPU 逐位相同。
- A100（按正常 Slurm 申请 2 块空闲 GPU，未动用户的 7888_* 作业）：`run_gpu.py --gpus2 0,1 --mpi "mpirun -np 2"` 全部通过，184 个算例中无空间电荷的 154 个与 CPU 逐字节相同，
  30 个空间电荷算例（含 PX1–PX8）重跑、双卡、MPI 2 进程逐字节相同，PICNIC / SPICNIC 与 CPU 逐字节相同；GCC 无新警告，`nm -D` 7 个接口、GLIBC_2.14。
- L40S、lyc-pc、XPS 仍无法连接，M6a 的欠账（第 8 节）未能补做。
- 临时数据：A100 的 `~/avas_engine_work/tmp_m6c` 与 Mac 的构建 / 运行目录已删除，工具链保留。

## M6c：Metal

已定（用户）：Mac GPU 用 Metal；物理代码与 CUDA 共用；Apple GPU 没有 fp64，Metal 固定为 fp32 档；同一台机器重跑逐位相同，不能有依赖调度次序的浮点累加；
CUDA 能跑的功能 Metal 都要能跑，不能跑或受限的项明确列出。本机：Apple M2（10 核 GPU，16 GB 统一内存，Metal 3），只有 Command Line Tools。

### 1. 本机核实（2026-11-08，探针程序，不进仓库）

| 问题 | 结果 | 对设计的影响 |
|---|---|---|
| 没有离线 `metal` 编译器时能否运行时编译 | 能：`newLibraryWithSource` 用的是系统自带的编译器，只装 CLT 即可；示例源 0.1–0.7 s，本段的全部内核约 0.3 s | 运行时编译，MSL 源在构建时嵌入库中；不需要 Xcode、不需要 metal-cpp（用 Objective-C++ 直接调 Metal.framework） |
| MSL 与共享头文件 | 不支持 `double`、`long long`、lambda、程序作用域的非 `constant` 变量；**每个指针和引用都必须写地址空间**（`device` / `thread` / `constant`） | 共享代码要加地址空间宏（`AVAS_G` / `AVAS_T`，CPU 与 CUDA 上为空）、`avas_i64`、`AVAS_PCONST`；MSL 源开头 `#define double float`；lambda 改为函数 |
| fast math | `mathMode = Safe`（macOS 15，旧系统 `fastMathEnabled = NO`）| 必须关闭 |
| 浮点收缩 | 关闭 fast math 后 **仍会把 a·b + c 收缩成 FMA**（32 % 的样本与分开计算不同）；源中 `#pragma METAL fp contract(off)` 后 0 个不同 | 必须加这条 pragma；加了之后 GPU 的 float 加减乘除与 CPU 的 float（`-ffp-contract=off`）逐位相同 |
| 内建 sin 等 | 与 macOS libm 的 sinf 在 27 % 的点上差 1 ulp | 需要与 CPU 逐位一致的地方用可移植实现（float 版 pmath，见第 2 节） |
| 64 位原子 | `atomic_ulong` 只有 min / max，**没有原子加**（M2，Apple8） | 定点电荷累加拆成低 32 位与高 32 位两个 32 位原子加，低位溢出时向高位进 1：整数和与次序无关，逐位可重复 |
| 统一内存 | `hasUnifiedMemory = 1`，单个缓冲区最大 8.6 GB；共享存储的缓冲区 CPU 与 GPU 直接读写，不需要拷贝 | 主机直接读写缓冲区；结构体里的设备指针用 `gpuAddress`（Metal 3） |

### 2. 设计

**共享代码编译成 MSL**：
- `hd.h` 增加 Metal 分支：`AVAS_HD` 为空，`AVAS_G = device`、`AVAS_T = thread`、`AVAS_PCONST = constant`，`avas_i64 = long`，`hm::sqrt` 等用 `metal::precise::`；
  CPU / CUDA 分支中这些宏为空、`avas_i64 = long long`，**生成的代码不变**（只是类型别名与空宏）。
- MSL 源 = `#define double float` + `hd.h` + `mesh_kernels.h` + `sc_kernels.h` + `src/metal/kernels.metal`（CMake 在配置时拼接，去掉 `#include "core/…"`，
  在 `hd.h` 之后加 `#pragma METAL fp contract(off)`），嵌入库中；运行时编译，关闭 fast math。
- 需要改写的头文件：编入 MSL 的函数要给指针 / 引用加宏、`long long` 换成 `avas_i64`、lambda 换成函数。本段做了空间电荷网格部分：
  `kern::MeshV` 与 `mesh_cell_t` / `mesh_trilinear` / `mesh_interp_t`（从 `kernels.h` 移到新的 `mesh_kernels.h`，算术不变），`sck::SpGrid`、`picnic_node`、
  `sym_target`、`corr_node`、`grad_spicnic_node`；`sc_kernels.h` 的格林函数表、修正表与 FFT 部分只在主机 / CUDA 编译（`#ifndef AVAS_METAL`）。
- 粒子推进（`kernels.h` 的 `trans_particle` 及场图、Boris、损失、事件面、二次粒子、z-code 行变换）要编入 MSL，同样要加地址空间宏、去掉 lambda，并且整个 fp32 档
  要不用 double：CUDA 的 fp32 档仍在设备上用 double 算时间、RF 相位、同步粒子差分的重建与统计和，z-code、FFT、二次粒子整段用 fp64。
  Metal 上这些要么在主机做（统一内存），要么用双 float（float-float）模拟。这是 M6c 的主要工作量，**本段没有完成**（第 5 节）。

**fp32 的数学函数**：Metal 内建的 sin / cos / exp / log / atan（precise）与 macOS libm 不逐位相同。推荐为 fp32 档写 float 版的可移植实现
（fdlibm 的 float 版 `__kernel_sinf` 等，只用加减乘除与位运算），CPU 的 fp32 参照与 Metal 共用，从而 Metal 与 emu 的 fp32 能逐位相同；
内建函数作为备选（误差 ≤ 1–2 ulp，但不能与 CPU 逐位一致）。本段的空间电荷网格部分只用加减乘除、sqrt、floor、rint、ldexp（都是正确舍入或精确的）。

**定点电荷累加**：每个分配量 v = round(w·q·2^e)（float 乘以 2 的整数次幂是精确的，`rint` 舍入到偶数），转成 64 位整数后拆成高低两个 32 位原子加：
`old = atomic_add(lo, l)`，`carry = (old + l < old)`，`atomic_add(hi, h + carry)`。整数加法可交换，最终的 64 位和与线程次序无关。代价：每次分配多一次 32 位原子加，
1e6 粒子、30³ 网格时分配核函数约 1 ms（第 4 节）。也考虑过按单元排序后分段归约（排序本身要可重复，开销更大），未采用。

**归约、压缩、FFT、网格制表**：
- 统计量的固定次序和：粒子在 GPU 上推进后，推荐在 GPU 上算每组（4096 个粒子）的部分和——float 积用 Dekker 分拆成两个 float、按定点整数累加，与次序无关——或者在主机上
  直接读统一内存按 `reduce.h` 的固定分组求和（双精度）。本段统计在主机上算（粒子在主机）。
- SPICNIC 修正表的压缩：本段在主机上建表（与 CPU、CUDA 同一顺序），GPU 只做求和。
- FFT：不能用 MPS；沿用 CUDA 后端的直接变换需要 double 才能保持 1e-13 量级，float 的直接 DST / DFT 误差约 1e-6 量级。本段 FFT 在主机上解（双精度）。
- PICNIC / SPICNIC 的格林函数表：在主机上用双精度算（与 CPU 相同的值），转成 float 写进统一内存；GPU 上 float 求和、float 差分。

**设备选择与选项**：`device=metal`（环境变量 `AVAS_ENGINE_DEVICE=metal`）；没有 Metal、Metal 3 或编译失败时报错并说明原因；`auto` 暂不选 Metal（待定点）。
`precision` 对 Metal 无意义（固定 fp32 档）；`gpus` 不适用（Apple 芯片只有一块 GPU）。compat=dll 只在 CPU。

### 3. 实现（第一阶段：空间电荷网格在 GPU 上）

| 文件 | 内容 |
|---|---|
| `src/core/hd.h` | Metal 分支与 `AVAS_G` / `AVAS_T` / `AVAS_PCONST` / `avas_i64` |
| `src/core/mesh_kernels.h`（新） | `MeshV` 与逐粒子的网格函数（从 `kernels.h` 移出，lambda 改为 `mesh_trilinear`），带地址空间宏 |
| `src/core/sc_kernels.h` | 求和与差分函数带地址空间宏；主机专用部分 `#ifndef AVAS_METAL` |
| `src/core/device.h`、`device_emu.cpp` | `ScAccel` 接口：分配、PICNIC / SPICNIC 求解、插值可交给加速器；emu 的推进与归约改为按归约组多线程（每组写自己的输出，与线程数无关） |
| `src/metal/kernels.metal`、`metal_backend.mm`（新） | 内核：定点分配（两个 32 位原子带进位）、插值、PICNIC、SPICNIC 八分之一求和与修正、差分；主机侧把 `MeshV` 换成 Metal 布局（启动时核对 sizeof） |
| `src/core/metal_stub.cpp`、`device.cpp`、`options.*` | `device=metal`；不开 Metal 的构建给出明确报错 |
| `CMakeLists.txt` | `AVAS_WITH_METAL`（只在 macOS，默认 OFF）：拼接 MSL 源、编译 `.mm`、链接 Metal / Foundation；C++ 编译器必须是 Apple clang（Objective-C++ 用同一个编译器、同一套 libc++），否则配置时报错——conda 激活的环境会设置 `CC`／`CXX`，conda 的 clang 编译不了 SDK 的 Foundation 头文件（复核时在本机默认 shell 里遇到），只改 `CMAKE_OBJCXX_COMPILER` 也不行（两种编译器的 libc++ 头文件不同） |
| `tests/physics/sc_ellipsoid.cpp` | `--metal`：SPICNIC / PICNIC 的分配、求解、插值在 Metal 上重做 |

第一阶段的 Metal 后端 = emu 设备（粒子在 CPU 上，fp64，多线程）+ Metal 上的空间电荷网格工作（fp32）。所以：没有空间电荷的算例与 CPU 逐字节相同；
有空间电荷时只有网格部分是 fp32，与 CPU 的差别来自 fp32 的分配权重、求和与插值。

### 4. 检查结果（Apple M2）

- **`run_gpu.py --device metal`**（184 个默认模式算例）：无空间电荷的 154 个与 CPU **逐字节相同**；有空间电荷的 30 个重跑**逐字节相同**；
  与 CPU 的差别（fp32 的网格部分）：DataSet 最大为列最大值的 1e-8～7e-6（SPICNIC 约 1e-6，PICNIC 1e-7～1e-6，FFT 只有分配与插值在 GPU 上，0～7e-6），
  dst 粒子坐标 3e-10～2e-7 rms。误差研究（ER*）、硬边元件段、二次粒子、边界、lpboundary、导入空间电荷场等算例都在 Metal 设备上跑通。
- **与 emu fp32 能否逐位相同**：本阶段粒子在 CPU 上（fp64），emu 没有 fp32 档，所以没有可比的对象；网格部分只用加减乘除、sqrt、floor、rint、ldexp，
  关闭收缩后与 CPU 上同样的 float 运算逐位相同（探针核实），将来 fp32 粒子推进用 float 版可移植数学函数即可与 CPU 的 fp32 参照逐位一致。
- **物理检验**：`avas_engine_physics --metal`（SPICNIC、PICNIC 在 Metal 上）与 CPU 的四位有效数字相同，阈值全部通过；DC KV（`AVAS_ENGINE_DEVICE=metal`）
  SPICNIC 40³ 0.993、PICNIC 24³ 0.984，通过。
- emu 的推进与归约改为多线程后，emu（2 个分片）与 CPU 仍逐字节相同。
- **CPU 与 CUDA 不受影响**：Mac 不开 Metal 的构建全套通过（见第 6 节）；A100 上 CUDA 构建（GCC 12 + nvcc 12.4，无新警告，`nm -D` 7 个接口、GLIBC_2.14）
  `run_gpu.py --gpus2 0,1 --mpi` 全部通过，154/154 逐字节相同，空间电荷算例重跑、双卡、MPI 逐字节相同（按正常 Slurm 申请 2 块空闲 GPU，未动用户的 7888_* 作业）。

### 5. 性能（M2，hwr010 两周期线，`perf_gpu.py`，推进 + 空间电荷每步时间）

| 配置 | 1e5 无 SC | 1e6 无 SC | 1e5 SPICNIC | 1e6 SPICNIC | 1e5 PICNIC | 1e6 PICNIC |
|---|---|---|---|---|---|---|
| CPU（8 线程） | 3.42 ms | 34.4 ms | 18.0 ms | 192 ms | 33.9 ms | 311 ms |
| Metal 第一阶段 | 4.94 ms | 49.8 ms | 9.57 ms | 73.8 ms | 8.66 ms | 78.5 ms |

- 有空间电荷时 Metal 快 1.9×（1e5）到 2.6×（1e6，SPICNIC）、4×（1e6，PICNIC）：网格求和与分配 / 插值在 GPU 上。
- 没有空间电荷时慢约 1.45×：粒子在 emu 设备里推进（多线程，但每步多一份拷贝与组归约），GPU 不参与。粒子推进移到 GPU 之前，Metal 只在有空间电荷时有用。
- 1e7 粒子没有测：emu 设备与主机各持一份 fp64 粒子数组（约 6.4 GB），16 GB 机器上接近内存上限。

### 6. Mac 全套（不开 Metal 的构建）

单元测试、`run_all.py`（compat=dll 最大差 3.44e-11 不变，inData 23/23 不变，libPIC 关键字检查通过）、`run_physics.py`、`tools_check.py`、`ruff` 通过；Clang 无新警告。

### 7. 不能运行或受限的项（第一阶段）

| 项 | 现状 |
|---|---|
| 粒子推进（t-code 场图 / Boris / 损失 / 事件面）、z-code、二次粒子、边界 | 在 CPU 上（emu 设备，fp64），不在 GPU 上 |
| FFT 求解 | 在 CPU 上（emu 设备的直接变换，双精度）；只有分配与插值在 GPU 上 |
| SPICNIC 修正表、格林函数表 | 在 CPU 上建（与 CPU 引擎同一张表），GPU 上 float 求和 |
| 统计量 | 在 CPU 上（双精度、固定次序） |
| `precision`、`gpus` 选项 | 对 Metal 无意义（固定 fp32 网格、只有一块 GPU） |
| `device=auto` | 不选 Metal |
| compat=dll | 只在 CPU |

### 8. 未完成

- **粒子推进移到 Metal**（fp32 档，相对同步粒子坐标）：要把 `kernels.h`、`physics.h`、`secondary.h` 中设备代码的全部指针与引用加地址空间宏、去掉 lambda、
  把 CUDA fp32 档仍用 double 的部分（时间、RF 相位、记录重建、统计和、z-code、FFT、二次粒子）改为主机计算或 float-float，并写 float 版可移植数学函数。
  本段只完成了空间电荷网格部分与基础设施（运行时编译、地址空间宏、统一内存、64 位定点原子），推进的移植没有做。
- 1e7 粒子的性能、L40S / lyc-pc / XPS 上的 M6a 欠账（三台机器开始与结束时都无法连接）。

### 9. 待用户决定（每项附推荐）

1. **是否继续第二阶段（粒子推进移到 Metal）**：推荐继续，但先确定范围——只做 t-code 推进与统计（z-code、FFT、二次粒子留在 CPU），预计改动 `kernels.h` 的设备函数签名（只加空宏，CPU / CUDA 结果不变）。
2. **数学函数**：推荐 fp32 档用 float 版的可移植实现（与 CPU 参照逐位一致）；另一选择是 Metal 内建 precise 函数（快，但只能保证 1–2 ulp）。
3. **运行时编译**：推荐保持运行时编译（用户机器多半只有 CLT；每次启动约 0.3 s，可加磁盘缓存）；另一选择是要求安装完整 Xcode 离线编译 `.metallib`。
4. **metal-cpp**：推荐不引入（Objective-C++ 直接调用 Metal.framework 已经足够，不增加第三方文件与许可证；metal-cpp 是 Apple 以 Apache-2.0 发布的头文件包，本段未下载）。
5. **`AVAS_WITH_METAL` 的默认值**：推荐在 macOS 上默认打开（只依赖系统框架，不开时没有任何代价），但 `device` 的默认仍是 cpu。
6. **`device=auto` 在 Mac 上**：推荐第一阶段仍选 CPU（无空间电荷时 Metal 更慢）；第二阶段完成后改为有空间电荷时选 Metal。
7. **统计量放在哪里**（第二阶段）：推荐 GPU 上用定点整数累加每组的和（与次序无关、可重复），主机只合并组；另一选择是主机直接读统一内存求和（简单，但限制加速）。

### 10. 临时数据

Mac 上的探针程序、两套构建、run_gpu / 性能 / parity 运行目录已删除；A100 的 `~/avas_engine_work/tmp_m6d` 已删除，工具链保留；L40S、lyc-pc、XPS 未能登录。

### 11. 用户决定（2026-11-08）

第 9 节的 7 个待定点全部按推荐：

1. **继续第二阶段**，先做 t-code 推进与统计（z-code、FFT 求解、二次粒子留在 CPU）——已完成，见第 12 节。
2. **fp32 数学函数用可移植实现**：第二阶段的 t-code 推进只用到加减乘除、`sqrt`（正确舍入）、`ceil`、`fabs`、`floor`、`rint`、`ldexp`；RF 因子是主机按步算好的表，
   所以推进里没有超越函数，也就不需要 float 版 sin / cos。代码只用 IEEE 754 正确舍入的运算并关闭收缩，结果只取决于运算次序（第 12 节说明与 CPU 参照的关系）。
3. **保持运行时编译**：整个 MSL 源第一次编译约 1.8 s，之后由系统的着色器缓存命中（约 0.05 s），不需要自己加磁盘缓存。
4. **不引入 metal-cpp**。
5. **`AVAS_WITH_METAL` 在 macOS 上默认打开**：APPLE 且 C++ 编译器为 AppleClang 时默认 ON；否则默认 OFF，并在配置时打一行 STATUS 说明原因与改法
   （例如 conda 环境设置了 CC / CXX 时：用 `CC=/usr/bin/clang CXX=/usr/bin/clang++` 新建构建目录）；显式 `-DAVAS_WITH_METAL=ON` 而编译器不对时仍 FATAL_ERROR。
   `device` 的默认仍是 cpu。
6. **`device=auto` 在 Mac 上**：没有 CUDA、编译了 Metal 时，有空间电荷的运行选 Metal，否则选 CPU（日志写明原因；Metal 不可用时退回 CPU 并写明原因）。
7. **统计量在 GPU 上用定点整数按组累加**，主机只合并组——已实现（第 12 节）。

### 12. 第二阶段：t-code 推进与统计在 Metal 上

**做法**（与 CUDA 的 fp32 档同一设计）：粒子状态在 Metal 上以 float 存相对同步粒子的坐标 d = x − x_s、u − u_s；同步粒子的推进、晶格位置减去同步粒子 z、
事件面位置、RF 因子由主机每步用双精度算好再转 float 传入；推进是 `cuda_f32.cuh` 的 `F32Step` 的移植（`src/metal/kernels.metal` 的 `k_step`），元件场调用共享的
`kern::external_field_t<…, float, …>`。混合运行用“当前态”标记（CUDA 的 `auth32_`）：Metal 推进一步后 float 态是当前态；设备后端（emu）的任何其他操作
（z-code 行变换、FFT、二次粒子、边界、`lpboundary`、纵向损失、下载等）之前先把 float 态写回 fp64 数组（`to_host`，x = x_s + d，用上一步的同步粒子），
上传之后作废 float 态，下一个 t-code 步再从 fp64 数组转成 float（`from64`）。缓冲区都是统一内存的共享存储，主机直接读写，没有拷贝；转换只在切换时做（一次运行通常各一次）。
空间电荷在 float 态下直接用 float 坐标分配、插值写回 float 场。

**为此改的共享代码**（CPU、CUDA 的生成代码不变）：
- `physics.h`：`k::` 常量加 `AVAS_PCONST`；`MapView::data` 加地址空间；`trilinear` 拆成 `tri_pos` / `tri_corner`（同一次序的同一算术，`trilinear` 改由它们实现）；推进器与 z-code 行变换只在主机 / CUDA 编译。
- `kernels.h`：`LatV` 的指针、`RfSrc::table`、场函数的引用与数组参数加地址空间宏；`element_field_t` 中三处 lambda 改为 `tri_pos` / `tri_corner` 循环（逐项同序）；
  superpose 块、误差场、`PartV` 与其后的 fp64 包装只在主机 / CUDA 编译（Metal 上 `element_field_t` 遇到 superpose 块返回 0，后端不会对这类晶格用 Metal 推进）。
- MSL 源增加 `physics.h`、`kernels.h`。

**统计量**：`k_step` 把每个粒子的 21 个求和项和 8 个最大值写进列优先的缓冲区；`k_group` 每个线程组处理一个归约组（4096 个粒子）：每列先求组内最大绝对值，
取 2 的幂 e 使 |v|·2^e < 2^50，再把各项换成 64 位整数相加（与次序无关），主机把 (和, e) 换回双精度并按组的次序合并（同 `reduce.h`）。最大值在 float 中精确。
z 的最小 / 最大值存 d_z，由主机加同步粒子的 z（双精度，与 CUDA 相同）。

**事件面与 old**：粒子越过下一个输出面时，`k_step` 记下它推进前后的 float 状态（12 个数）并把序号原子地追加到列表；主机按列表在双精度中完成插值
（与 CUDA fp32 档逐式相同：`interp_momentum_norm`、`rec[6] = (r − 1) dt + t_end`），写 rec 并推进 `nextPlane`。`writeOld` 时记下推进前的 float 状态，主机换成双精度的 old。
列表的次序不确定，但每个粒子的处理互相独立，所以结果确定。

**不在 Metal 上推进的情形**（自动留在 CPU，日志写一行原因）：晶格含 superpose 块（场类型 4）、误差研究的误差场（`errOn`）、superposeout 段。
z-code、FFT 求解、二次粒子、`boundary.txt` 几何损失、`lpboundary` 折叠、纵向损失在 CPU 上做（每次切换一次转换）。

**与 CPU 参照的关系**：t-code 推进只用正确舍入的运算、关闭收缩，结果只取决于运算次序，重跑逐位相同。但 MSL 源以 `double = float` 编译，`ElemV` 等结构体中的
双精度字段在 Metal 上也是 float（例如 `R(e.ke * 1e6)` 在 Metal 上是 float 乘积，在 CUDA fp32 档是 double 乘积再转 float），所以 **Metal 与 CUDA fp32 档、与 CPU
不能逐位相同**；现在也没有 CPU 上的 fp32 参照实现（emu 只有 fp64）。要逐位一致，需要把这些结构体按标量类型模板化，并在 CPU 上编译同一套 float 代码（第 14 节待定点 1）。

**检查**（Apple M2，Mac 默认构建：AppleClang，`AVAS_WITH_METAL` 默认 ON）：
- `run_gpu.py --device metal`（184 个默认模式算例，Metal 下按 fp32 档比较）：**每个算例重跑逐字节相同**；与 CPU（fp64）的差别：DataSet 一般为列最大值的 3e-5～1e-4
  （与 CUDA fp32 档同量级：SC1 为 3.24e-5 / dst 1.53e-5，CUDA fp32 为 3.24e-5 / 1.52e-5），最大 3.4e-3 出现在单粒子校正铁算例（该列接近 0）；dst 粒子坐标 ≤ 4e-5 rms，
  PN3 的单个网格边缘粒子 5e-3 rms（与 CUDA fp32 相同）；单粒子算例 ≤ 1.2e-6 rms。
- 物理检验：DC KV（`AVAS_ENGINE_DEVICE=metal`，推进与空间电荷都在 Metal 上）SPICNIC 40³ 增长比 0.993、PICNIC 24³ 0.984，与 CPU 的四位有效数字相同，通过；椭球（`avas_engine_physics --metal`）SPICNIC 40³ 最大偏差 0.34 %、PICNIC 32³ 0.19 %，通过。
- A100 CUDA（fp64 + `--fp32`，按正常 Slurm 申请 2 块空闲 GPU）：fp64 154/154 逐字节相同、空间电荷算例重跑 / 双卡 / MPI 逐字节相同；fp32 档的数字与 M6b 时完全相同
  （SC1 3.24e-5 / 1.52e-5、PN3 3.29e-5 / 5.12e-3 等），即 `trilinear` 的改写没有改变 CUDA fp32；`nm -D` 7 个接口、GLIBC_2.14，无新警告。
- Mac CPU 全套：单元测试、`run_all.py`（compat=dll 最大差 3.44e-11 不变，inData 23/23 不变）、`run_physics.py`、`tools_check.py`、`ruff` 通过；默认构建（含 Metal）无新警告。conda 的 clang 配置时默认关闭 Metal 并给出提示，显式打开则报错（已核实）。

**性能**（M2，hwr010 两周期线，推进 + 空间电荷每步时间；CPU 为 8 线程）：

| 粒子数 | CPU 无 SC | Metal 无 SC | CPU SPICNIC | Metal SPICNIC | CPU PICNIC | Metal PICNIC |
|---|---|---|---|---|---|---|
| 1e5 | 3.21 ms | 1.93 ms | 17.9 ms | 6.27 ms | 33.9 ms | 7.70 ms |
| 1e6 | 31.4 ms | 6.86 ms | 169 ms | 23.4 ms | 311–374 ms | 27.9 ms |
| 1e7 | 592 ms | 98.4 ms | — | 186 ms | — | — |

- 无空间电荷 4.6×（1e6）到 6.0×（1e7），SPICNIC 7.2×、PICNIC 11–13×（1e6）。1e7 粒子在 16 GB 机器上能跑（设备后端仍持有一份 fp64 数组，作为切换时的副本）。
- 最初统计缓冲区按粒子行优先存放，组归约每步 49 ms；改为列优先（合并访问）后 2 ms。
- 每步主机开销约 0.07 ms（偏移表、RF 表、组合并）。

### 13. 第二阶段未完成 / 受限

- Metal 与 CPU / CUDA fp32 的逐位一致（见上；需要结构体模板化与 CPU 上的 float 参照）。
- superpose 块、误差研究的误差场、superposeout 段的晶格，推进留在 CPU（emu，fp64，多线程）。
- z-code、FFT 求解、二次粒子、边界几何仍在 CPU；带 `boundary.txt` 的运行每步要来回转换，Metal 反而可能更慢。
- 设备后端（emu）仍保留一份完整的 fp64 粒子数组（内存约为纯 GPU 设计的两倍）。
- L40S、lyc-pc、XPS 本段开始与结束时都无法连接。

### 14. 新的待定点（每项附推荐）

1. **CPU 上的 fp32 参照**（让 Metal 与 CPU / emu 逐位一致）：需要把 `ElemV` / `SubV` / `LatV` 等按标量类型模板化（`ElemVT<double>` 为现有的 `ElemV`），emu 增加 fp32 档，
   并让 CUDA fp32 档也用同一套 float 字段（CUDA fp32 的结果会变，量级约 1e-7 相对）。推荐做，但放在 superpose / 误差场上 Metal 之后。
2. **superpose 块、误差场、superposeout 也在 Metal 上推进**：要把 `superpose_field_t`、`element_err_field_t`、superposeout 记录改写为 MSL 可编译（去 lambda、加地址空间）。推荐下一步做（误差研究常用）。
3. **去掉 emu 的 fp64 副本**（Metal 为当前态时不保留 fp64 数组，切换时才分配）：推荐暂不做，16 GB 机器上 1e7 已能运行。
4. **`boundary.txt` 几何损失放到 Metal**：推荐与第 2 项一起做。

### 15. 用户决定（2026-11-09）与第三阶段

第 14 节 4 个待定点全部按推荐：第 2 项（superpose 块、误差场、superposeout 在 Metal 上推进）与第 4 项（`boundary.txt` 几何损失放到 Metal）先做；
第 1 项（CPU / emu 的 fp32 参照，Metal 与之逐位一致；CUDA fp32 用同一套 float 字段）随后；第 3 项（去掉 emu 的 fp64 副本）暂不做。

**superpose、误差场、superposeout**：`superpose_field_t`、`element_err_field_t` 的 lambda 改为 `tri_pos` / `tri_corner` 循环与函数 `err_back`（逐项同序），
指针与引用加地址空间宏，编入 MSL（只有 compat=dll 用的 `superpose_err_field_dll_t` 仍只在主机）。Metal 上 `SubV`、校正后的 superpose 块（`eff`）、误差开关随晶格上传，
superposeout 期间元件表变化时（`set_elems`）重新上传。superposeout 出口：`k_step` 把可能穿过出口面的粒子（同 CUDA 的条件，浮点比较带余量）追加到候选列表，
主机按 CUDA fp32 档的公式在双精度中判断并记录 spRec（`sp_local`、`interp_momentum_norm`），计数进 `StepGroup::sp`。

**boundary.txt**：`k_bound` 在 GPU 上对每个粒子做几何段查找与壁面判定（float，所有比较带 1e-5 相对余量，弯管的 atan / sincos 用 Metal 内建 precise 函数），
只把可能碰壁、靠近段边界或越过 lattice 末端的粒子列为候选；主机对候选用共享的 `kern::boundary_particle` 在双精度中给出与 CPU 相同的判定、碰壁坐标与时刻
（推进前的坐标按需从 float 快照换成双精度，不再每步整体换算）。其他粒子在 GPU 上更新几何段序号（离段边界足够远，与双精度的结果相同）。
LB1 / LB2 的碰壁记录数、存活粒子数与 CPU 相同（LB2：684 行 collisionData、4597 个存活）。

**CPU 上的 fp32 参照（第 1 项的 CPU 部分）**：`k_step`、`k_bound`、分配、插值的逐粒子函数移到共享头文件 `core/f32_kernels.h`；`core/f32_cpu.cpp` 把同一套头文件在
C++ 中按 Metal 的方式编译——先包含标准库，再 `#define avas avas_f32`（与引擎其余部分隔开，不违反 ODR）、`#define double float`，`hd.h` 的 `AVAS_F32_CPU` 分支只给 float 版函数；
结构体中的双精度字段因此与 Metal 一样是 float，而不必把结构体模板化。emu 设备的 fp32 档（`AVAS_ENGINE_DEVICE=emu AVAS_ENGINE_PRECISION=fp32`）用 Metal 后端的同一套主机逻辑，
内核换成这些 CPU 函数（目前只在含 Metal 的构建中提供；Linux 上 `f32_cpu.cpp` 也能编译，A100 上 GCC 12 通过）。
**结果：`run_gpu.py --device metal` 的 184 个算例，Metal 与 emu fp32 全部逐字节相同（184/184）**——包括空间电荷（FFT、SPICNIC、PICNIC）、superpose、误差研究、superposeout、boundary。
这说明 Metal 关闭收缩后的 float 加减乘除、sqrt、rint、ldexp 与 CPU 逐位相同；弯管预选用的 atan / sin / cos 只影响候选集合，不影响结果。

**CUDA fp32 用同一套 float 字段（第 1 项的 CUDA 部分）：本轮没有做**。要让 CUDA fp32 与 Metal / emu fp32 逐位相同，CUDA fp32 档要整体换成第二阶段的设计
（`f32_kernels.h` 的逐粒子函数在 nvcc 下按 float 世界编译、统计改为定点整数组和、事件面 / superposeout / boundary 改为候选列表由主机完成），相当于把 Metal 后端的主机逻辑移到 CUDA 后端；
现在的 CUDA fp32 结果不变（见下）。

**检查**（Apple M2）：
- `run_gpu.py --device metal`（184 个算例，`AVAS_ENGINE_PROFILE=1` 计数）：重跑全部逐字节相同；**没有一个算例退回 CPU 推进**（日志中 0 个“stays on the CPU”），
  149 个有 t-code 步的算例都在 Metal 上推进（其余 35 个只有 z-code 段、单步或在初始化时报错）。例如 ER5（误差研究，147 步）、SE4（superpose 误差，223 步）、
  SP4（superposeout，230 步，两次切换）、LB2（boundary，169 步，169 次 `k_bound`，只有 1 次往返转换）都在 GPU 上。与 CPU fp64 的差别与第二阶段相同（DataSet ≤ 1e-4，
  单粒子校正铁算例 3.4e-3；dst ≤ 4e-5 rms，PN3 的单个边缘粒子 5e-3）。**与 emu fp32 逐字节相同 184/184。**
- 物理检验在 Metal 上通过：DC KV SPICNIC 40³ 0.993、PICNIC 24³ 0.984；椭球 SPICNIC 40³ 0.34 %、PICNIC 32³ 0.19 %。
- Mac CPU 全套（默认构建，含 Metal）：单元测试、`run_all.py`（inData 23/23，compat=dll 最大差 3.44e-11）、`run_physics.py`、`tools_check.py`、`ruff` 通过，无新警告；
  conda 的 clang 构建（默认不含 Metal，配置时提示）编译通过、无警告。
- A100 CUDA（GCC 12 + nvcc 12.4，`run_gpu.py --gpus2 0,1 --mpi "mpirun -np 2" --fp32`，按正常 Slurm 申请 2 块空闲 GPU，未动用户作业）：fp64 154/154 逐字节相同，
  30 个空间电荷算例重跑 / 双卡 / MPI 逐字节相同；fp32 的数字与之前完全相同（SC1 3.24e-5 / 1.52e-5、SE4 3.27e-5 / 7.04e-6、PN3 3.29e-5 / 5.12e-3）；`nm -D` 7 个接口、GLIBC_2.14，无新警告。

**性能**（M2，推进 + 空间电荷每步时间；晶格同 LB2 / ER5，束流改为生成的 1e5 / 1e6 粒子，无空间电荷）：

| 运行 | CPU（8 线程） | Metal |
|---|---|---|
| boundary.txt，1e5 | 3.07 ms | 1.82 ms |
| boundary.txt，1e6 | 30.3 ms | 7.19 ms（每步约 990 个候选由主机判定） |
| 误差研究（2 组），1e5 | 3.48 ms | 1.34 ms |
| 误差研究（2 组），1e6 | 40.6 ms | 6.94 ms |

**受限 / 未完成**：
- CUDA fp32 与 Metal / emu fp32 的逐位一致（见上，CUDA fp32 档需要换成第二阶段的设计）。
- emu fp32 档目前只在含 Metal 的构建中可用（它复用 Metal 后端的主机逻辑与共享内存缓冲区）；`f32_cpu.cpp` 本身可在任何平台编译。
- z-code、FFT 求解、二次粒子仍在 CPU；emu 的 fp64 副本仍在（第 14 节第 3 项暂不做）。
- L40S、lyc-pc、XPS 开始与结束时都无法连接。

**新的待定点（附推荐）**：
1. **CUDA fp32 档换成与 Metal 相同的设计**（共享 `f32_kernels.h`、定点组和、候选列表），使三者 fp32 逐位一致：推荐做，同时把 Metal 后端的主机逻辑抽成与设备无关的 C++ 类
   （Metal、CUDA、CPU 共用），emu fp32 也就能在 Linux 上用。CUDA fp32 结果会改变（预计与现在相差约 1e-6 相对，与 Metal 相同）。
2. **z-code 行变换上 Metal**（误差研究、四极段常用，现在每次进出 z-code 各转换一次）：推荐在第 1 项之后做。

### 16. 用户决定（2026-11-09）与第四阶段

第 15 节两个待定点都做，按推荐顺序：先把 CUDA fp32 档换成与 Metal 相同的设计（主机逻辑抽成与设备无关的类），再把 z-code 行变换放到 GPU 上。

**设备无关的 fp32 主机逻辑**：Metal 后端的主机逻辑移到 `core/f32_host.cpp` 的 `F32Accel`（设备后端 emu 的加速器，接口 `ScAccel`）；设备只实现 `core/f32_host.h` 的
`F32Device`——分配主机能直接读写的缓冲区（`F32Buf`：主机指针 + 内核用的地址），启动 `core/f32_kernels.h` 的内核，每次调用返回时已完成。三个实现：
- Metal：`src/metal/metal_backend.mm`（共享内存缓冲区、运行时编译的 MSL）；
- CUDA：`src/cuda/cuda_f32_device.cu`（托管内存 `cudaMallocManaged`）与 `cuda_f32_kernels.cu`（内核由 nvcc 在 “double = float” 世界编译：`hd.h` 新增 `AVAS_F32_CUDA`
  分支，`AVAS_HD` 为 `__host__ __device__`；与 CPU / Metal 一样关闭收缩，除法与开方为 IEEE 正确舍入）；
- CPU：`core/f32_cpu_device.cpp` 与 `f32_cpu.cpp`（emu fp32 档）。这两个文件和 `f32_host.cpp` 在每个构建中都编译，emu fp32 因此在 Linux 和 conda clang 的 Mac 构建中也可用。
结构体布局在启动时按设备报告的 `sizeof` 核对（新增 `ZArgs`）；C / Mx 列的步长单独传给内核。
**主机只碰紧凑的数据**：平面穿越与 superposeout 候选在内核里连同快照（12 个 float）、下一平面号、元件号写进按候选槽排列的紧凑表（取代每粒子的快照列），
主机在双精度中补全后，把结果（记录标志、下一平面号加一、float 行；superposeout 标志）写进一张结果表，由内核 `kZApply` 写回粒子；清除记录（`zero_rec`）、
空间电荷场写回粒子（`kZSc`）也在设备上。正常的一步里主机不读写任何粒子列——统一内存的 Metal 上这只是少了几次散写，在 CUDA 的托管内存上则是关键（见下）。

**CUDA fp32 档**（`AVAS_ENGINE_PRECISION=fp32`，或 auto 在非 HPC 卡上）现在是“emu 设备后端 + `F32Accel` + CUDA 设备”，与 Metal 完全同一套主机逻辑与内核；
旧的 CUDA fp32 实现（`cuda_f32.cuh`、`CudaBackend` 中的 fp32 分支、float 网格求和模板实例）删除，`CudaBackend` 只剩 fp64 档，运算不变。
- 每张卡（单卡也一样）的候选计数、列表与紧凑表、每步的偏移表与 RF 表、组和结果都在该卡自己的显存里，用显式复制进出（只复制实际的候选数）；
  `PartF` 中这几个指针按卡替换，主机再按卡拼接（主机逻辑与候选次序无关）。粒子状态、晶格表与场图在托管内存中，正常的一步里只有 GPU 访问。
- 多卡：每张卡分到整数个归约组；分配每张卡有自己的 64 位整数网格（结果在主机按模 2^64 相加，与次序无关），统计按组分卡，网格求解（PICNIC / SPICNIC 求和、梯度）
  在第一张卡；托管内存中的表、场图与网格数组设为 read-mostly（每张卡一份副本），粒子列按卡的粒子段设首选位置。
- 第一版直接让主机读写托管内存（候选快照在粒子列里、组和与偏移表也在托管内存中），A100 上每步都有大量页在主机与 GPU 之间来回：1e6 粒子无空间电荷每步推进比旧
  CUDA fp32 慢 15 倍、双卡再慢一倍，中间版本的空间电荷每步也慢 17 倍（场写回粒子在主机上）。改成上面的紧凑表与显式复制后见下面的性能表。
- MPI：fp32 档下每个进程计算全部粒子（各自用自己的卡），结果与进程数无关；MPI 对 fp32 档不加速（见待定点）。
- 显存估计按 fp32 状态（每粒子约 94 个 4 字节字）计算。

**z-code 行在 GPU 上**：
- 粒子状态新增 8 列 float 行 {x, ux, y, uy, dt, uz, t − T0, flag} 与 q/m 列。t-code 中主机按双精度完成平面穿越记录（`P.rec`）时同时写 float 行，时间原点 T0
  取第一次记录那一步的结束时刻（与设备、候选次序无关）。
- `rows`、`rows_tref`、`rows_kick`、HESC 两遍、`ztot`、`update_beam_parameters` 两遍（`ubp1` / `ubp2`）都由一个内核 `zrow_particle`（`f32_kernels.h`）完成，
  求和走与 t-code 统计相同的定点组和（组和内核推广为任意列数与最大值列数）。行变换用共享的 `kern::het_row`、`kick_row` 与 `physics.h` 的 `zrow_advance`、
  `solenoid_row`、`bend_row`、`edge_row`、`steerer_kick`、`ztot_row`：这部分移出 `AVAS_FLOAT_ONLY` 保护、加地址空间宏，字面量写成 `double(1)`（在 float 世界里是 float），
  fp64 代码的运算一个不变。float 世界的 sin / cos / sinh / cosh 用只含 + − × ÷、floor、ldexp 的 Cody–Waite 约化加 Cephes 单精度多项式（`kernels.h` 的 `fm`），
  三种设备逐位相同。
- 每个元件之后（`rows_tref`）行时间移到参考时间：下一个行内核先减去同一个 float 值、T0 在主机双精度中加上它（行时间与它相差不到一倍，减法精确）。
- `ztot` 在 GPU 上直接把行换回相对新同步粒子 {0, 0, 0, 0, z_s, uz_s} 的 float 状态（`DeviceBackend::ztot` 增加同步粒子参数，tracker 先算同步行），
  随后的 `update_beam_parameters` 也在 GPU 上，所以 **进出 z-code 都不再整体转换粒子数据**。需要主机记录时（dst / dump 输出）只把行换回 `P.rec`，不动粒子状态；
  加速器的 `to_host` 不再放弃“当前态”，只有主机改动粒子数组（`host_mod`：回退路径、lpboundary、longlimits、upload）时才失效，改动记录时 float 行才重建。
- float 精度的三处处理：(1) β² 不用 1 − 1/γ²（在 β = 0.05 时丢约 9 位，而且整束团舍入方向相同，飞行时间整体偏 1e-5 相对），float 世界改用 p²/γ²
  （`zrow_c2b2` 等，fp64 仍是原式）；(2) c² 与元件长度舍入成 float 对每个粒子相同（约 4e-8 相对，每个元件约 1e-16 s，低于行时间的 float 分辨率），改由 T0 在双精度中扣除；
  (3) `ztot` 中 zPlane − z_s 与 uz_s 用 float 加余数传入。
- `AVAS_ENGINE_F32_ZCODE=0` 让这些操作回到主机（与之前的走法相同，仅供测量）；`AVAS_ENGINE_PROFILE=1` 的 `[fp32 profile]` 增加 z-code 在 GPU 上的次数与行上传 / 记录下载 / 同步重定基的次数。

**检查**（Apple M2，Mac 默认构建：AppleClang、含 Metal）：
- `run_gpu.py --device metal`（184 个算例）：重跑全部逐字节相同；**Metal 与 emu fp32 逐字节相同 184/184**（含 45 个 z-code 在 GPU 上的算例）。
- z-code 在 GPU 上的证据（`AVAS_ENGINE_PROFILE=1`，184 个算例的计数）：45 个含 z-code 的算例共 224 次行操作、74 次 zTotcode、74 次 `update_beam_parameters`
  全部在 GPU 上完成；这 45 个算例每个只有开始时 1 次 fp64→fp32 转换，多粒子算例只有结束输出时 1 次 fp32→fp64（单粒子算例每步的单粒子输出只读取，不再转回 float），
  行上传（fp64 记录→float 行）0 次、同步粒子重定基 0 次。第四阶段之前每个 z-code 段进、出各一次整体转换，行变换在主机上逐粒子串行。
  仍逐步转换的只有 lpboundary（LP1–3）、longlimits（LL1）与 importSCF（LS1），它们的这部分工作在主机上。
- 与 CPU（fp64）的差别：t-code 算例与第三阶段相同（DataSet 一般 3e-5～1e-4、最大 2.2e-4；dst ≤ 7e-5 rms，PN3 的单个网格边缘粒子 5e-3）。
  z-code 多粒子算例（23 个）：dst 中位 6.1e-5、最大 1.2e-4 rms（z-code 在主机时 ≤ 1.2e-5）；DataSet 中位 5.1e-5，最大 7.5e-4 出现在质心 z 偏移列
  （接近 0 的列，ER16 的绝对差 1.5e-8 m），弯铁 Z3 / Z4 的横向质心列 3.9e-4（绝对 2.6e-10 m）。z-code 单粒子算例（22 个）：dst ≤ 4.4e-6 rms；DataSet 的
  z 偏移列相对差最大 0.77（Z3，绝对 7e-8 m，162.5 MHz 下 2e-4 度）。来源是 float 行用绝对的 βγ_z：βγ_z ≈ 0.057 时舍入 3.7e-9，对应能量 2e-7 MeV、
  每个元件的飞行时间约 1e-15 s（见待定点 1）。修正前（β² 用 1 − 1/γ²、c² 与长度的舍入未扣除）Z1 的质心 z 列差 1.1e-2，修正后 4.6e-5。
- emu fp32 在不含 Metal 的构建中可用：conda clang 的 Mac 构建（默认关闭 Metal）编译通过，emu fp32 运行 Z1 / SC5 / LB2，z-code 同样走 float 行；
  A100（Linux，GCC 12）上 emu fp32 跑全部 184 个算例（见下）。
- 物理检验在 Metal 上通过（z-code 不涉及，数字与第三阶段相同）：DC KV SPICNIC 40³ 增长比 0.993、PICNIC 24³ 0.984；椭球 SPICNIC 40³ 0.34 %、PICNIC 32³ 0.19 %。
- Mac CPU 全套（默认构建）：单元测试、`run_all.py`（inData 23/23 不变，compat=dll 与 AVAS.dll 最大差 3.44e-11 不变）、`run_physics.py`、`tools_check.py`、
  `ruff`（`engine/tests`、`engine/tools`、`avas`）通过；两种 Mac 构建（AppleClang 含 Metal、conda clang 不含 Metal）都无新警告（conda 构建的 `-pie` 链接提示来自 conda 的 `LDFLAGS`）。
  CPU 默认模式只改了不改变运算的地方（`physics.h` / `kernels.h` 的地址空间宏与 `double(…)` 字面量、tracker 先算同步行、`RowOp::uzs` 对所有元件填写）。

**A100**（GCC 12 + nvcc 12.4，`--fmad=false`，无新警告；`nm -D` 只有 7 个 C 接口，只依赖 GLIBC_2.14；按正常 Slurm 申请 2 块空闲 GPU，未动用户的 7888_* 作业）：
- 第一部分完成时（z-code 仍在主机）先跑一遍全套：fp64 154/154 与 CPU 逐字节相同、30 个空间电荷算例重跑 / 双卡 / MPI 逐字节相同（删除旧 fp32 代码不影响 fp64）；
  **CUDA fp32 与 emu fp32 逐字节相同 184/184**，fp32 重跑、双卡（`AVAS_ENGINE_GPUS=0,1`）、MPI（`mpirun -np 2`）对全部 184 个算例逐字节相同（552/552）；
  CUDA fp32 与旧 CUDA fp32 的差别：DataSet 最大 3.2e-5（列最大值的比例）、dst 最大 1.4e-5 rms，62 个算例不变；与 CPU 的最大差不变（3.4e-3，单粒子校正铁算例）。
- 含 z-code 上 GPU 的构建（CUDA 内存改造之前）：`run_gpu.py --gpus2 0,1 --mpi "mpirun -np 2" --fp32 --stats 8` 全部通过——fp64 154/154、空间电荷 30 个算例重跑 / 双卡 / MPI 逐字节相同；
  **CUDA fp32 与 emu fp32 逐字节相同 184/184**（数字与 Metal 完全相同）；fp32 重跑 184、双卡 184、MPI 61 个算例（空间电荷与多粒子的 z-code、误差、superpose(out)、
  boundary 算例；fp32 下 MPI 每个进程算全部粒子，每个算例一次 `mpirun` 太慢，全部 184 个已在第一部分跑过）逐字节相同 429/429；8 个种子的统计检验通过。
  z-code 计数与 Metal 相同（45 个算例、224 次行操作、74 次 zTotcode，行上传 0）。与旧 CUDA fp32 相比：t-code 算例同上（≤ 3.2e-5），z-code 算例的变化即上面
  float 行的差别（多粒子 DataSet ≤ 7.5e-4、dst ≤ 1.2e-4 rms；单粒子 z 偏移列最大 0.77，绝对 7e-8 m）。
- CUDA 内存改造（紧凑候选表、显式复制，见上）之后的构建再跑全套：fp64 154/154、空间电荷 30 个算例重跑 / 双卡 / MPI 逐字节相同，fp32 重跑 / 双卡 / MPI 429/429，
  与 emu fp32 逐字节相同 184/184；最后加上的托管内存提示（read-mostly、首选位置、批量预取，只影响页的位置）的构建跑 18 个代表算例（空间电荷、z-code、误差、
  superposeout、boundary、单粒子、dump）：fp64 11/11，fp32 重跑 / 双卡 / MPI 50/50，与 emu fp32 18/18；`nm -D` 7 个接口、GLIBC_2.14，无新警告。
- Linux 上 emu fp32 可用：A100 计算节点上（不含 Metal 的构建）emu fp32 跑全部 184 个算例，即上面逐字节比较的参照。
**CUDA fp32 的性能（A100，`perf_gpu.py`，推进 + 空间电荷时间 / 跟踪总时间，秒；“旧”为 c534785 的 CUDA fp32，新构建与同节点的其他作业并行时测得）**：

| 运行 | 旧 CUDA fp32 | CUDA fp64 | 新 CUDA fp32，1 卡 | 新 CUDA fp32，2 卡 |
|---|---|---|---|---|
| hwr010 两周期，1e6，无 SC | 0.82 / 4.25 | 0.97 / 4.30 | 1.00 / 4.79 | 1.32–3.63 / 5.4–8.6 |
| hwr010 两周期，1e6，SPICNIC | 2.38 / 5.83 | 2.79 / 6.13 | 55 / 59 | 31–36 / 36–40 |
| 四极段，1e5，无 SC | 0.30 / 1.21 | 0.36 / 1.34 | 0.96 / 1.87 | 1.64 / 2.91 |
| 四极段，1e6，无 SC | 1.70 / 5.36 | 2.04 / 5.49 | 6.1–40 / 10–44 | 7.1 / 11.3 |
| 四极段，1e6，SPICNIC | 5.76 / 9.39 | 6.28 / 9.82 | 125–128 / 130–133 | 80 / 85 |

- 无空间电荷的 t-code 与旧 fp32 持平；z-code 束线 1e5 慢约 1.5 倍，1e6 时单卡在不同运行间相差可达 6 倍（托管内存的页迁移不稳定，原因未查清），双卡稳定在约 2 倍。
- **有空间电荷时比旧 CUDA fp32 慢 15–25 倍**：网格的格林函数表、SPICNIC 修正表、电荷与场的 float / double 转换都在主机上（emu 的做法；Metal 的统一内存上这些开销小，
  第二阶段测得 Metal 比 CPU 快 7–13 倍），而旧 CUDA fp32 在 GPU 上算格林函数表与修正表。这是本阶段最大的遗留问题（见待定点 4）。
- 排查中发现并修正的问题：托管内存由主机 `memset` 清零时逐页缺页（1e6 粒子开始时耗时几十秒，改为 GPU 清零、主机批量读写前后整体预取）；单卡上粒子页
  没有首选位置时每步都被迁走（推进内核一步 23 ms，设首选位置后 0.2 ms）；表与列表设为 read-mostly。

**性能**（M2；束线为 20 个四极铁的 FODO（`quad 0.1` 与 `drift 0.1` 交替，4.05 m，20 个 z-code 段）与同一束线加四极误差研究（2 组，ER16 的误差设置）；
示例 hwr010 束流生成 1e5 / 1e6 粒子；`perf_gpu.py --lattice`；“之前”为第三阶段提交 c534785 的构建，与现在的构建交替运行，同一配置测两轮）。
这台机器本段测时的起伏很大（同一构建同一配置两轮相差可到 30 %，第一次测量时甚至慢 5 倍，原因未查明），总时间的差别大多在起伏之内；
能稳定分辨的是“跟踪时间 − 推进与空间电荷时间”，即 z-code 段、进出转换与输出的时间：

| 四极段（每次运行，两轮） | Metal 之前 | Metal 现在 | CPU（8 线程）总跟踪时间 |
|---|---|---|---|
| 1e5，无 SC：z-code + 转换 | 0.51 / 0.47 s | 0.25 / 0.26 s | 6.5 s（Metal 现在总计 2.2 s） |
| 1e6，无 SC：z-code + 转换 | 4.10 / 3.83 s | 1.52 / 1.54 s | 64 s（Metal 现在总计 12.7–13.0 s） |
| 1e6，SPICNIC：z-code + 转换 | 5.16 / 5.15 s | 1.96 / 3.52 s | — |

- z-code 部分快 2–2.7 倍。之前 Metal 在这条束线上 1e5 无空间电荷时比 CPU 只快 2.4 倍，现在快 3 倍；1e6 时 5 倍。
- 误差研究（2 组，进程总时间，两轮）：1e5 无 SC 之前 6.7 / 5.5 s、现在 6.2 / 6.6 s，1e6 无 SC 之前 30.5 / 42.0 s、现在 25.8 / 40.5 s，1e6 SPICNIC 之前 73.7 / 90.8 s、
  现在 66.0 / 91.0 s；CPU 1e5 无 SC 17.3 s、1e6 无 SC 401 s、1e5 SPICNIC 248 s。总时间由 t-code 推进主导，改进在起伏之内。
- t-code（hwr010 两周期线）与之前持平（1e6 无 SC 推进 4.5–5.2 s 对 4.9–5.4 s）。中间试过把粒子列长度补齐到整组（多卡时按页对齐），
  结果一个粒子的约 80 列都相隔 16 KiB、落在同一组缓存上，推进内核慢 25 %，已撤回。
- 1e6 时每步主机约 2–4.6 ms：t-code 中每个粒子穿过 20 个四极入口和输出面，穿越点由主机在双精度中补全（同时写 float 行），这是剩下的主要主机开销。

**受限 / 未完成**：
- z-code 的 float 行是绝对坐标（见上），z-code 算例的 fp32 差别比 t-code 大几倍（仍远小于物理需要）。
- MPI 下 fp32 档不加速（每个进程计算全部粒子）。
- CUDA fp32 用托管内存：Linux 上可并发访问（已在 A100 上验证）；Windows（WDDM）上托管内存不能与内核并发访问，本设计每次调用后都同步，结果正确，性能未实测（XPS 不能连接）。
- 在主机上的部分：FFT 求解、二次粒子、lpboundary、longlimits、importSCF、dump / 单粒子输出的读取；emu 的 fp64 副本仍在（第 14 节第 3 项暂不做）。
- CUDA fp32 的性能：有空间电荷时比旧 CUDA fp32 慢 15–25 倍，z-code 束线单卡耗时不稳定（见性能表与待定点 4）。
- L40S、lyc-pc、XPS 在本段开始与结束时都无法连接（ssh 超时）。
- 临时数据：A100 的 `~/avas_engine_work/tmp_m6g`（源码包、各次构建、全套与性能运行目录）与 Mac 的构建 / 运行目录已删除，工具链保留。

**新的待定点（附推荐）**：
1. **z-code 行改为相对同步行的增量形式**（时间与 βγ_z 存相对量、行变换按增量更新），把 z-code 算例的 fp32 差别降到 t-code 水平：要为 float 世界另写行变换的时间与动量更新
   （共享的 `het_row` 是绝对坐标，fp64 路径必须不变），工作量中等。推荐：暂不做，现在的差别（能量 2e-7 MeV、z 1e-7 m）远小于物理需要；若需要，可用
   `AVAS_ENGINE_F32_ZCODE=0` 让 z-code 回到主机 fp64（变慢）。
2. **MPI 下 fp32 档按进程分粒子**：要让主机逻辑只持有本进程的粒子，并在进程间合并统计、候选与网格。推荐：暂不做——fp32 档面向单机消费级显卡，单进程多卡已支持；
   A100 等 HPC 卡 auto 用 fp64 档，MPI 照常加速。
3. **FFT 求解与 lpboundary / longlimits 也放到 fp32 设备上**（现在在主机，后两者每步转换）：推荐在第 4 项之后做 FFT（与 PICNIC / SPICNIC 同样编进 float 世界），
   lpboundary / longlimits 随后。
4. **CUDA fp32 的性能**（本阶段最大的遗留问题）：(a) 网格的格林函数表与 SPICNIC 修正表在设备上计算（可移植的 `p_log` / `p_atan`，fp64 档已有，CPU / Metal 用同一份 fp64 代码，
   结果仍逐字节相同），电荷与场的转换放到设备；(b) CUDA 设备改用显式显存加批量复制（`F32Device` 增加读写区段的接口），不再依赖托管内存的页迁移。推荐：下一步先做 (a)，
   再做 (b)。在此之前 **auto 精度在非 HPC 卡上仍选 fp32**，有空间电荷的运行会比旧版本慢；可以选择暂时让 CUDA 的 auto 改选 fp64（消费级卡 fp64 慢，但不受上述主机开销影响）——
   需用户决定，推荐先保持 fp32、尽快做 (a)。

### 17. 用户决定（2026-11-10）与第五阶段

第 16 节四个待定点全部按推荐：
1. z-code 行改为增量形式：暂不做（需要时用 `AVAS_ENGINE_F32_ZCODE=0`）。
2. MPI 下 fp32 档按进程分粒子：暂不做。
3. FFT 求解、lpboundary / longlimits 放到 fp32 设备：在第 4 项之后做，先 FFT，再 lpboundary / longlimits。
4. CUDA fp32 性能：先 (a) 网格格林函数表与 SPICNIC 修正表在设备上计算、电荷与场的 float / double 转换放到设备，再 (b) CUDA 设备改用显式显存加批量复制，不再依赖托管内存的页迁移。
   auto 精度在非 HPC 卡上保持 fp32。

实施顺序 4(a) → 4(b) → FFT → lpboundary / longlimits；importSCF 没有顺手做（见受限项）。

**设计**

- **内存模型**（`core/f32_host.h`）：`F32Device::ensure()` 是设备缓冲区加一份主机副本，二者只经 `upload()` / `download()`（按字节区段）交换——Metal 与 CPU 上主机与设备共用内存，
  这两个调用是空操作；`ensure_shared()` 是主机与内核都直接读写的内存（主机的候选表、结果表、组和、电荷网格的两个字），CUDA 上是映射到设备的锁页主机内存
  （`cudaHostAlloc` Mapped | Portable）。另有两个只影响放置的提示：`tables()`（只读的表：每张卡一份）与 `particles()`（按粒子的列：每段放在该段的卡上，可指定每个粒子的字节数）。
  `F32Accel` 只在这些调用处交换数据：开始时整体上传粒子状态与表，每步上传偏移表与 RF 表，候选与结果走共享内存，结束 / 需要主机记录时下载。
- **CUDA 设备**（`src/cuda/cuda_f32_device.cu` 重写）：单卡时 `ensure()` 是 `cudaMalloc` 加主机副本，**完全不用托管内存**；多卡时每个缓冲区仍是一块托管内存，但只当作各卡共用的地址
  （首选位置第一张卡、各卡 AccessedBy；表设 read-mostly，粒子列按卡的段设首选位置并预取），主机从不直接读写它，数据只经显式复制移动。
- **4(a) 网格表与转换在设备上**：
  - SPICNIC / PICNIC 的格林函数表（`sck::green_corner` / `green_cell`）与 SPICNIC 修正表（`sck::corr_value`，cub `DeviceSelect::Flagged` 稳定压缩，保持 CPU 的次序）
    在 CUDA 上用 fp64 内核计算，再在设备上转成 float——与 fp64 档同一份可移植代码（`p_log` / `p_atan`，`--fmad=false`），与主机算出的数逐位相同。
  - Metal 与 CPU 设备在主机上算同一张表（`host_green` 按条目并行，修正表按 GPU 压缩的次序），然后上传。**所以 fp32 结果没有因为表改在设备上算而变化**
    （下面“与第四阶段的差别”中 SPICNIC / PICNIC 算例全部逐字节不变）。
  - 电荷：CUDA 的分配改用原生 64 位原子加（`k_deposit64`，模 2^64 的和与 Metal / CPU 的两个 32 位字加进位相同），各卡的网格下载后在主机按模 2^64 相加，
    电荷以 double 上传、在设备上转 float；场留在设备上（`bfield_`），插值直接用它，只有主机需要时（`recscfield` 记录）才由 `fields()` 下载。
    多卡时插值前把场复制一份到其余各卡（`cudaMemcpyAsync`，每步 3·n³ 个 float），不再跨卡读托管内存。
- **4(b)**：上面的显式缓冲区之外，主机侧的工作也改成只碰紧凑数据：`boundary.txt` 的候选改成与平面穿越相同的紧凑表（`bEnt`）加结果表（`kZApply` 第 2 类）；
  主机补全平面穿越在候选多（≥ 16384）时并行（第一遍按粒子在 double 中记录、设 T0，第二遍按槽写结果行，与次序无关）。
- **FFT 在设备上**：`f32::fft_node` 的 10 遍（缩放、x / y 的 DST、z 的 DFT、乘格林因子、反 DFT、反 DST、归一化、梯度），用 `sc_kernels.h` 中原有的直接变换
  （`dst_node`、`dft_z_node`、`green_factor`、`grad_fft_node` 改为 float 世界也能编译，字面量写成 `double(…)`）；变换表来自主机的 `sck::fft_tables`（按网格缓存），
  lx / ly / lz 每次上传。CUDA、Metal、CPU 三种设备逐字节相同；与 CPU fp64 的 pocketfft 差别见下。
- **lpboundary / longlimits 在设备上**：`kZLp`（纵向周期折回，按同步粒子的相对位置）与 `kZLong`（纵向能量 / 位置损失）两个 `zrow` 内核；粒子状态新增静止质量列。
  这些算例因此只有开始 1 次 fp64→fp32、结束 1 次 fp32→fp64 转换。
- **Metal 合并提交**：`F32Device` 新增 `batch_begin()` / `batch_end()`，其间的内核只排队（Metal：同一个命令缓冲区，每次分派后加缓冲区屏障），`batch_end()` 返回时都已完成；
  用于 FFT 的 10 遍、SPICNIC / PICNIC 的求和 + 修正 + 梯度、插值 + 写回粒子。CUDA 与 CPU 上为空操作。
- **SPICNIC 求和的访存**：`sck::sym_target` 的最内层循环先取 4 个节点的电荷与格林函数值，再按原次序累加（每个目标点一个线程，目标点只有 n³/8 个，
  大 GPU 上每个 SM 只有一两个 warp，原来每次迭代都等一次访存）。运算与次序不变，fp64 / fp32、CPU / GPU 的结果逐位不变（CPU 默认模式 12 个 SPICNIC 算例逐字节核对）。
- `AVAS_ENGINE_PROFILE=1` 的 `[fp32 profile]` 增加网格求解与 lpboundary / longlimits 在 GPU 上的次数，以及每次电荷分配、网格求解（电荷与修正表、格林函数表、
  势清零、求和）、插值的耗时。

**与第四阶段的差别（emu fp32，Mac，184 个算例）**：176 个逐字节不变（全部 SPICNIC / PICNIC、t-code、z-code、boundary、LP1、LL1）；变化的 8 个都是 FFT 求解
（包括 LP2、LP3：lpboundary 下按周期边界一律走 FFT），原因是 FFT 从主机 fp64 的 pocketfft（结果再转 float）改为设备上 float 的直接变换：
DataSet 最大 7.0e-6（列最大值的比例，SC11），dst 最大 3.65e-5 rms（SC11），其余 DataSet ≤ 5e-6、dst ≤ 4.7e-6 rms。与 CPU fp64 的差别：FFT 算例 DataSet 3e-5～8e-5、
dst ≤ 8e-5 rms（SC2 3.3e-5 / 1.4e-5，SC11 6.8e-5 / 8.0e-5，LP1 3.1e-5 / 1.6e-5，LL1 2.8e-5 / 7.0e-5），与 t-code 算例的 fp32 差别同一量级。

**检查**
- Mac（Apple M2，AppleClang 含 Metal）：`run_gpu.py --device metal` 184 个算例（最终构建）：重跑全部逐字节相同，**Metal 与 emu fp32 逐字节相同 184/184**；emu fp32 与 CPU 的输出和本阶段中途的构建逐字节相同（只差时间戳）；单元测试、`run_all.py`（inData 23/23，compat=dll 与 AVAS.dll 最大差 3.44e-11，均不变）、
  `run_physics.py`、`tools_check.py`、`ruff`（`avas`、`engine/tests`、`engine/tools`）通过。Metal 上的物理检验：椭球 SPICNIC 40³ 最大偏差 0.34 %、PICNIC 32³ 0.19 %、
  FFT 128³ 0.99 %（FFT 现在也在 Metal 上）；DC KV SPICNIC 40³ 增长比 0.993 / 0.994、PICNIC 24³ 0.984。conda clang 的构建（不含 Metal）编译通过、无新警告，
  emu fp32 的 SC2 / LP1 / LL1 / SC5 与 AppleClang 构建逐字节相同。
- A100（GCC 12 + nvcc 12.4，`--fmad=false`；每个构建都无新警告，`nm -D` 7 个接口，只依赖 GLIBC_2.14；按正常 Slurm 申请，未动用户的 7933_* 作业）：
  - 含 4(a)(b) 的构建（b1）与再加上 FFT / lpboundary / longlimits 的构建（b2）各跑 17 个代表算例（空间电荷、FFT、PICNIC、z-code、误差、superposeout、boundary、单粒子、
    LP1、LL1）：fp64 9/9 与 CPU 逐字节相同，fp32 重跑 / 双卡 / MPI 47/47 逐字节相同，与 emu fp32 17/17 逐字节相同。
  - **最终构建的全套（fp64 154 + 空间电荷 30 的重跑 / 双卡 / MPI，fp32 184 与 emu、重跑、双卡、MPI）与性能表的作业（7963）已提交，交回时仍在排队**：
    gn1 的 8 块 GPU 被用户的 4 个 7933_* 作业和另一用户的数组作业（每个 1～4 小时，优先级更高）占满，等了两个多小时没有轮到。
    最终构建与 b2 的差别只有 CUDA 的 64 位原子分配、多卡插值前复制场、两块缓冲区的放置提示、SPICNIC 求和的取数顺序与 Metal 的合并提交；
    这些改动已在 L40S（sm_89）与 1080Ti（sm_61，双卡）上用最终构建跑完全套（见下），Metal 与 emu 也已核对。
  - 中途构建的性能（1 卡，`perf_gpu.py`，推进 + 空间电荷 / 跟踪总时间，秒；“旧”为 c534785 的 CUDA fp32，同一作业中交替测得）：

| 运行 | 旧 CUDA fp32 | CUDA fp64（第 16 节） | 第四阶段 CUDA fp32，1 卡 | 本阶段中途，1 卡 | 本阶段中途，2 卡 |
|---|---|---|---|---|---|
| hwr010 两周期，1e6，无 SC | 0.82–0.85 / 4.32–4.39 | 0.97 / 4.30 | 1.00 / 4.79 | 0.74–0.78 / 4.91–4.97 | — |
| hwr010 两周期，1e6，SPICNIC | 2.38–2.44 / 5.74–5.98 | 2.79 / 6.13 | 55 / 59 | 2.64 / 6.97 | — |
| 四极段，1e6，无 SC | 1.70–1.79 / 5.36–5.51 | 2.04 / 5.49 | 6.1–40 / 10–44 | 2.08–2.18 / 6.51–6.75 | 2.54 / 7.45 |
| 四极段，1e6，SPICNIC | 5.76–5.83 / 9.27–9.52 | 6.28 / 9.82 | 125–128 / 130–133 | 6.46 / 11.02 | 11.3 / 16.1 |

    “本阶段中途”是 64 位原子分配之后、SPICNIC 取数改动与多卡插值 / 放置改动之前的构建（b5；2 卡一列为 b3）。有空间电荷时从第四阶段的慢 15～25 倍回到旧 CUDA fp32 的
    1.1 倍左右；每步分解（`AVAS_ENGINE_PROFILE`，hwr010 1e6 SPICNIC 30³）：电荷分配 0.73 ms（两字加进位时 1.03 ms）、网格求解 1.58 ms（其中电荷与修正表 0.06 ms、
    格林函数表 0.03 ms，其余是 SPICNIC 求和）、插值 0.09 ms。四极段无 SC 单卡在多次运行中稳定在 2.08–2.18 s（第四阶段 6–40 s），比旧版仍慢约 0.3 s，
    是主机补全平面穿越（每步约 0.5 ms）。2 卡有空间电荷时插值曾因跨卡读托管内存中的场要 2.6 ms，最终构建改为先复制场。
- L40S（5 号卡，与他人共用：测试期间该卡另有进程占用 70～80 %）与 lyc-pc（2 × GTX 1080 Ti），用 A100 上构建的全架构 `.so`（sm_60/61/70/75/80/86/89 + compute_90，无 MPI）
  分别跑了本阶段中途的构建和最终构建，两次结果相同：
  - L40S：fp64 154/154 与 CPU 逐字节相同，30 个空间电荷算例重跑逐字节相同；fp32 重跑 184/184、与 emu fp32 184/184 逐字节相同。
  - 1080Ti：fp64 154/154，30 个空间电荷算例重跑 / 双卡（`0,1`）逐字节相同；fp32 重跑 + 双卡 368/368、与 emu fp32 184/184 逐字节相同。
  - 两台机器上 `AVAS_ENGINE_PRECISION=auto` 都选 fp32（设备行：“precision fp32 (auto: fp64 is slow on this GPU …)”），A100 上 auto 选 fp64（同前）。
- manylinux2014 容器构建（L40S 上已有的 `avas-engine-build:cuda12.4-manylinux2014` 镜像，`tools/build_cuda_linux.sh`，GCC 10.2 + CUDA 12.4，全部默认架构）：
  编译无警告，`libavas_engine.so` 只依赖 GLIBC_2.14 / GLIBCXX_3.4.19，`nm -D` 7 个接口；用它在 L40S 上跑 10 个代表算例：fp64 5/5 与 CPU 逐字节相同、空间电荷算例重跑相同，
  fp32 重跑与 emu fp32 10/10 逐字节相同。（不同平台之间 CPU fp64 不要求逐位相同：Linux 与 Mac 的同步粒子数据因系统数学库末位不同，emu fp32 的输出也随之不同。）

**Metal 性能（M2；“之前”为第四阶段 d08ade4 的构建，与现在的构建交替运行）**：
- 测试算例（5260 个粒子，每步毫秒，三轮）：FFT 算例 SC2（64³）88–96 → 16–17，PX2（64³）89–90 → 16–18，SC8 / SC11（32³）7.8–8.3 → 3.7–5.8，SC13 / SC14（32³，含记录）
  11–13 → 8.6–10.6；LP2（lpboundary + FFT 32³）7.9–8.4 → 3.8–6.8；LP1、LP3（14³）、LL1 在起伏之内（2.0 / 2.9 / 2.0 左右，粒子少、每步只有几个小内核）。
  `AVAS_ENGINE_PROFILE` 证实这些算例在第四阶段每步各转换一次（LP1–3 147 / 147 次、LL1 252 / 252 次），现在只有开始、结束各 1 次（147 次网格求解、146 次 lpboundary 在 GPU 上）；
  设备描述行由 “… on the GPU in fp32 (FFT solve, secondary particles on the CPU)” 变为 “… lpboundary / longlimits and the space-charge mesh work (PICNIC, SPICNIC, FFT)
  on the GPU in fp32 (secondary particles on the CPU)”。现在仍每步转换的只有 importSCF（LS1）。
- hwr010 两周期线，FFT 64³（`perf_gpu.py --scmethod FFT`，推进 + 空间电荷每步时间）：1e5 粒子 88.3 → 17.1 ms，1e6 粒子 103.9 → 26.2 ms（第二轮测量时本机有其他程序占满 CPU，负载约 100，结果作废）。
  现在每步网格求解（10 遍直接变换）约 13.7 ms，是 FFT 运行的主要部分。

**L40S 与 GTX 1080 Ti 的性能（`perf_gpu.py`，推进 + 空间电荷 / 跟踪总时间，秒；全架构 `.so` 为本阶段中途的构建，不含最后的 SPICNIC 访存与多卡提示两项改动；
L40S 的 5 号卡测试时另有进程占用 70～80 %，数字只作参考；最终构建复测 hwr010 1e6 SPICNIC：L40S 4.41 / 7.05、1080Ti 5.70 / 7.23，四极段 1e6 SPICNIC 10.05 / 13.78、
12.45 / 14.35，在起伏之内）**：

| 运行 | L40S fp64 | L40S fp32（auto） | 1080Ti fp64 | 1080Ti fp32（auto） |
|---|---|---|---|---|
| hwr010，1e5，无 SC | 0.56 / 0.99 | 0.70 / 1.18 | 0.69 / 0.88 | 0.25 / 0.45 |
| hwr010，1e6，无 SC | 1.60 / 3.73 | 1.34 / 4.07 | 5.81 / 7.10 | 1.86 / 3.42 |
| hwr010，1e5，SPICNIC | 2.09 / 2.53 | 2.92 / 3.41 | 1.90 / 2.08 | 1.17 / 1.38 |
| hwr010，1e6，SPICNIC | 5.37 / 7.47 | 4.16 / 6.83 | 10.48 / 11.75 | 5.41 / 6.96 |
| 四极段，1e6，无 SC | 3.19 / 5.63 | 3.27 / 6.27 | 12.03 / 13.49 | 3.98 / 5.81 |
| 四极段，1e6，SPICNIC | 12.11 / 14.61 | 9.85 / 13.52 | 23.05 / 24.66 | 11.81 / 13.72 |

- 1080Ti 双卡（hwr010 1e6）：fp64 无 SC 3.12 / 4.69、SPICNIC 7.56 / 9.14；fp32 无 SC 1.85 / 3.85，SPICNIC 起初 93.9 / 95.8——电荷列与插值的临时数组没有按卡放置，
  而 1080Ti 之间的托管内存只能整页迁移，每步来回搬运。给这两块缓冲区加上按粒子段的放置提示（`particles()` 的新参数）后为 6.48 / 8.45（最终构建复测 6.46 / 8.44；
  网格求解只在第一张卡上，双卡有空间电荷时仍不如单卡）。
- CPU：L40S（限 32 线程）hwr010 1e5 无 SC 1.09 / 1.48、1e5 SPICNIC 20.6 / 21.2、1e6 无 SC 8.68 / 15.0；i7-12700K（20 线程）1e5 无 SC 1.06 / 1.54、1e5 SPICNIC 10.8 / 11.3、
  1e6 无 SC 16.5 / 21.7。
- 消费级卡上 fp32 档普遍快于 fp64（1080Ti 1e6 无 SC 3.1 倍、四极段 3 倍）；L40S 的 fp64 吞吐量虽只有 fp32 的 1/60，hwr010 的推进主要受访存限制，两档相近，
  L40S 1e5 SPICNIC 时 fp32 反而慢（每步网格往返的固定开销，fp64 档整个在 GPU 上），1e6 时 fp32 快 23 %。

**受限 / 未完成**：
- importSCF（`impscfield 1`，LS1）仍在主机上，按记录的场逐粒子插值，每步整体转换一次（使用很少，没有顺手做）；二次粒子仍在主机上；dump / 单粒子输出仍读取主机记录。
- MPI 下 fp32 档每个进程计算全部粒子（按第 16 节决定不改）。
- 多卡 fp32 仍用托管内存作为各卡共用的地址（数据只经显式复制和放置提示移动，A100 与 1080Ti 上已验证）；Windows（WDDM）上单卡完全不用托管内存，多卡未实测。
- 网格求解（PICNIC / SPICNIC / FFT）只在第一张卡上，所以有空间电荷时双卡不如单卡；SPICNIC 每个目标点一个线程（30³ 网格只有 3375 个线程），大 GPU 上占用率低，
  改成多线程分段求和会改变 float 的累加次序（见待定点 2）。
- FFT 用设备上的直接变换（与 fp64 档同一做法），64³ 时 Metal 上每次求解约 13.7 ms，比 SPICNIC 慢。
- XPS（Windows、GTX 1650）在本段开始与结束时都无法连接（ssh：Network is unreachable），Windows 上的 CUDA fp32 与 MSVC 构建未测。

**新的待定点（附推荐）**：
1. **多卡 fp32 完全去掉托管内存**（每张卡自己的粒子段缓冲区，指针按卡替换）：推荐暂不做——单卡已不用托管内存，多卡的放置提示在 A100 与 1080Ti 上有效；
   等需要 Windows 多卡时再做。
2. **SPICNIC 求和在 fp32 档按固定分段并行**（每个目标点由多个线程各求一段、再按固定次序相加；所有 fp32 设备同时改，结果仍逐字节相同，但 fp32 的 SPICNIC 结果会在末位变化，
   fp64 档不变）：大网格 / 大 GPU 上可把求解时间降到几分之一。推荐：先不做，现在 SPICNIC 每步约 1.4 ms（A100，30³），与推进相当；用户的典型网格变大时再做。
3. **importSCF 放到设备**：推荐不做（功能很少用，逐粒子按记录时间插值，搬到设备的工作量与收益不成比例）。

**临时数据**：Mac 的构建 / 运行目录（scratchpad，含第四阶段对照构建）已删除，`$TMPDIR` 下没有 `avas_*` 目录；L40S 的 `/data/public/lycself/avas_m7tmp`
（含 manylinux 构建输出）与 lyc-pc 的 `~/avas_m7tmp` 已删除，L40S 上原有的 `avas-engine-build:cuda12.4-manylinux2014` 镜像保留（工具链）。
A100 的 `~/avas_engine_work/tmp_m7` 因作业 7963 仍在排队而保留（只剩源码、最终构建 b8、旧 fp32 对照构建、四极段 lattice 与作业脚本，95 MB）；
作业完成后结果在其中的 `final_f8.log`，读完后删除整个目录即可。另有两个已失效的排队作业 7950、7957（所用构建目录已删，运行时立即失败退出，不占 GPU 时间）。
工具链 `~/avas_engine_work/toolchains` 保留。
