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
