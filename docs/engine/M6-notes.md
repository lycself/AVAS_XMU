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
