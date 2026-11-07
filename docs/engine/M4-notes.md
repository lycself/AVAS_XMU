# 新内核 M4 实现记录：空间电荷

M4 在 M3（engine-rebuild 81f7e9d）之上实现规格 60 的空间电荷：FFT 与 SPICNIC 两种求解器、t-code 每步求解、z-code 的 HESCFieldPre 与半步冲量、
`recscfield` 记录、autoPIC、默认网格与线程分区，以及 `spacechargecomp`。M3“用户决定”（M3-notes 末尾）一并带入。

## 1. 代码

| 文件 | 内容 |
|---|---|
| `engine/src/core/spacecharge.{h,cpp}`（新） | `SpaceCharge`：静止系网格（节点 ±meshrms·rms，纵向相对中心拉伸 γ）、CIC 分配、FFT（横向 DST-I 导体壁 + 纵向周期 FFT，pocketfft）、SPICNIC（每步重做格林函数表：近场 10³ 中点积分、远场 1/r；上卦限电荷三面镜像的直接卷积；非对称修正）、场插值（E⊥/γ、Ez，网格外点电荷）、autoPIC、recordSCF；`hesc_field_pre`（z-code 坐标换成同一时刻）；`space_charge_defaults`（meshrms、numofgrid 按粒子数分档） |
| `engine/third_party/pocketfft/`（新） | pocketfft C++ 头文件（BSD-3，附 `pocketfft_LICENSE.md`）。产品不依赖 FFTW |
| `engine/src/core/tracker.cpp` | t-code 每步推进前求解（totE = extE + scE）；z-code 有空间电荷时每个元件分两半，中点 HESCFieldPre + 求解，后半步给动量冲量 `Δu = (q/m)·scE·2Δt₁/c`；`spacechargecomp` 事件；UpdateBeamParameters 给出第一次网格用的 rms |
| `engine/src/core/beam.{h,cpp}`、`lattice.cpp` | `Beam.scx/scy/scz`、`scComp`；生成 / .dst / .edst 路径调用默认参数；默认模式的线程分区不再按机器线程数截断（见第 3 节） |
| `engine/tests/unit_tests.cpp` | 均匀带电椭球的电场与解析解比较（FFT、SPICNIC，快速版） |
| `engine/tests/physics/`（新） | `sc_ellipsoid.cpp`（目标 `avas_engine_physics`）、`dc_kv_envelope.py`、`run_physics.py`：带阈值的物理检验 |
| `engine/tests/parity/` | 14 个空间电荷算例（`SC*`）与 8 个 DLL 实验算例（`X_*`，只跑参考）；`compare.py` 支持 `compat_tolerance`；`run_all.py` 新增线程无关性检查；`SpaceChargeMesh.txt` 纳入比较 |

## 2. compat=dll 比对（Windows MSVC，OMP_NUM_THREADS=12，与 XPS 参考机相同）

算例：S1 lattice（腔）× SPICNIC / FFT、GS / KV / WB、5260 与 20000 粒子、numofgrid 20 + meshrms 4、FFT 32³ + meshrms 5、`spacechargecomp 0.5`、
Z1（quad，z-code）、Z5（混合）、`recscfield 1`（SPICNIC 与 FFT）。

- **t-code（SPICNIC 与 FFT）**：10 个算例 DataSet.txt、synData、synParticle、SpaceChargeMesh.txt 逐字符一致；outData 粒子块不逐位：
  约一半数值在末位不同，跟踪 0.31 m 后最大相对差 1.2e-12（SPICNIC 5260）、1.5e-11（SPICNIC 20000）、1.7e-13～5e-13（FFT）。
  用 `recscfield 1` 直接比较第一步的网格场：SPICNIC 约 85 % 的节点有差别，最大相对差 6e-13（中位数 0）；FFT 最大 2e-15（相对场的最大值）。
  FFT 的差别来自 pocketfft 与 FFTW 的舍入；SPICNIC 的差别是卷积 / 制表的求和次序与 DLL 不完全相同（反汇编只核对了 Weight 的电荷分配与网格尺寸，
  Calculate_S_PICNIC（7000 字节）的循环次序没有逐条核对）。这些算例在 cases.json 中标 `compat_tolerance 1e-10`，严格比对时以“close”计。
  没有做“测试构建动态加载 libfftw3”的对照。
- **spacechargecomp**：逐字符一致。DLL 在 −4 事件处与 steerer 一样调用 zTotcode 并递归重启 t-code（新发现），兼容模式照做。
- **z-code（SC5 Z1、SC6 Z5、SC11 FFT Z1）：原来的“DLL 冲量强约 20 %”是 DLL 的数据竞争，已查明**（2026-11-07）。
  - 原因：DLL 的 HESCFieldPre 第二个并行区（FUN_180090940，`cores[2]` 个 OpenMP 线程、静态分块）把 (rel−c)² 直接加到共享的 Beam.rms 数组，
    反汇编是普通的 `addsd (%rdx),%xmm0; movsd %xmm0,(%rdx)`，没有 lock、原子区或归约。12 线程同时读-加-写同一地址，大量加法丢失，
    rms 偏小且每次运行不同；网格随 rms 缩小（节点仍覆盖 ±meshrms·rms），场变强，表现为冲量“强约 20 %”。`.so` 的同一并行区同样无保护（规格 60 2.3）。
  - 证据：`recscfield 1` 的单 quad 算例记录了 z 段中点那次求解的网格。DLL 在 OMP_NUM_THREADS=12 下两次运行 dx 为 0.00019973、0.000173566 m
    （dy、dz 同样约为一半，网格内电荷 Q 也少 0.3～1.5 %），新内核为 0.00042194；DLL 在 OMP_NUM_THREADS=1 下 dx、dy、dz、Q、γ 与新内核逐字符相同，
    整个 SpaceChargeMesh.txt 一致（中心坐标见下一条），SpaceChargeSet.plt 的 z 段场最大相对差 1.7e-15（FFTW 与 pocketfft，与 t-code 相同）。
    原 SC5／SC6／SC11 在 DLL 上连跑三次，DataSet 末端 rms 每次不同（第 4～5 位），三次都与旧参考不同，所以旧参考本身不可复现。
    Beam.gamma 在 z 段确实沿用上一次 t-code 的值（规格 60 未解决问题 5），DLL 与新内核相同（1.00163）。
  - 修正：竞争的结果无法复现，compat 按无竞争的结果实现（等于 DLL 取 `cores[2] = 1`），默认模式本来就是串行求和，不受影响。
    三个算例的 input.txt 加 `setnumberofcores 64 54 1 24 1 48 96 12 48 12`（只把 cores[2] 改为 1，其余为 N ≤ 3e4 的默认表），
    参考输出用 DLL（OMP_NUM_THREADS=12）重新生成，三次运行除路径外逐字节相同；去掉 `compat_expected_diff`，改标 `compat_tolerance 1e-10`。
    结果：Windows（MSVC，--strict）与 macOS 上这三个算例的 DataSet、synData、synParticle、LongitudinalBeamLoss、FullParticle 逐字符一致，
    outData 矩相差 ≤ 3.5e-15（Windows）／≤ 6.8e-15（macOS），与 t-code 空间电荷算例同一类（FFT／SPICNIC 舍入次序）。
  - 顺带修正：z 段 recordSCF 的中心坐标。DLL 的 SpaceChargeMesh.txt 第 2～4 列用的“同步粒子坐标”在 z 段仍是最后一个 t-code 步保存的旧值
    （例：0.0164674，新内核原来写 6.3478e-07），compat 照做，默认模式用 z 段同步行（z = 0，即参考面）。新增算例 SC14_fft_Z1_record（z 段 + `recscfield 1`）锁定。
  - `setnumberofcores` 的截断：DLL 在读入时把每项截到 omp_get_max_threads()；新内核默认模式原来也在这里截断，使显式给出 setnumberofcores 的运行随机器线程数变化
    （与第 3 节“分区不随机器截断”矛盾，SC5 的线程无关检查因此失败）。现在只在 compat 截断，与默认表的处理一致。
- M1–M3 的 75 个算例仍逐字符一致（已知不可复现 4 个）。

## 3. 默认模式

- **线程无关**：电荷分配按粒子顺序单遍累加；SPICNIC 每个目标点独立计算，非对称修正按固定顺序累加；插值逐粒子独立；FFT 单线程 pocketfft。
  另外把 M1 起的推进分块（cores 表）改为不按 `omp_get_max_threads()` / maxthreads 截断——原内核的统计量求和次序因此随机器线程数变化。
  `AVAS_ENGINE_THREADS` / `OMP_NUM_THREADS` 取 1、4、12 时 SC1、SC2、SC5、SC9 与 D_GS_GS 的 DataSet.txt 和 outData 逐位相同（`run_all.py` 每次检查 1 线程与多线程）。
- **物理检验**（`python3 engine/tests/physics/run_physics.py`，带阈值，Mac / Windows 一条命令；程序 `avas_engine_physics` 与 `dc_kv_envelope.py`）：
  - **均匀带电椭球**（半轴 1、1.5、3 mm，在各轴 0.8 倍半轴以内拟合线性场系数）：SPICNIC 20³/25 万粒子 1.005 / 1.004 / 0.984，30³/50 万 1.004 / 0.999 / 0.998，
    40³/100 万 1.003 / 1.000 / 0.998（阈值：40³ 最大偏差 ≤ 1.5 % 且加密不变差；实测 0.34 %）。剩余偏差来自 CIC 分配 / 插值在椭球表面附近把边界抹平约一个格距，
    以及粒子统计（20³ 时 z 向 1.6 % 主要是格距 0.36 rms 的抹平）。
    FFT 固定格距、加大盒子：meshrms 6.5（64³）1.035 / 0.973 / 0.929，9.75（96³）1.013 / 0.988 / 0.976，13（128³）1.006 / 0.992 / 0.990
    （阈值：13 时 ≤ 1.5 % 且单调）。偏差按镜像距离的三次方减小（7.2 % → 2.4 % → 1.0 %），说明 FFT 的差别完全来自导体壁与纵向周期镜像。
  - **DC KV 包络**（beamtype dc、KV GS、20 mA，32.5 MHz 使 dc 束团长 0.53 m≈65 倍束半径，取入口相位中间一半的粒子并按序号跟踪；KV 起点和零流强参考都用这些粒子自己的
    入口矩，meshrms 2.5 2.5 2.0 使网格紧贴 KV 边缘）：空间电荷引起的出口 rms 增长与 kv.py 之比，SPICNIC 20³/2 万 0.977 / 0.975，30³/8 万 0.990 / 0.989，
    40³/16 万 0.993 / 0.994（阈值 2 %）。
  - **由此发现并修正的默认模式问题**：SPICNIC 格林函数表。原实现（DLL 同）近场用 10³ 中点积分、近 / 远场按“格数”划分、本格取 0、核在 0.75 Nx 格处截断；
    对拉长的格子（长束团、dc 束，格子长宽比可达 40）中点积分严重欠采样、远场点值也不对、截断切掉了束团远端，修正前 DC KV 的增长只有 KV 的 38～81 %，且随网格剧烈变化。
    默认模式现在每个表项都用长方体上 1/r 的解析积分（含本格），核覆盖整个网格；compat 不变。修正后椭球结果基本不变（立方格子时原近似已够），DC KV 收敛到 1 % 以内。
    耗时：核范围扩大后默认模式的 SPICNIC 仍与原内核相当（macOS：SC1 0.18 s、SC9 0.30 s，compat 0.14 s、0.27 s）。
- **修正的缺陷**（均先用 DLL 实验确认，X_* 算例）：
  - SPICNIC 非对称修正由“赋值（每个任务只留最后一个镜像点）”改为累加；实验证实 DLL 的结果依赖任务划分（setnumberofcores 第 8 个数 1 与 12 结果不同）。
  - 奇数网格：DLL 实测挂起（numofgrid 15 16 16，90 s 超时）；新内核两种模式都报错“SPICNIC needs an even numofgrid”。
  - Ny ≠ Nz 时 j 起点误用 Nz/2：默认模式所有目标点都计算，与分区无关（DLL 实验只确认结果随网格改变，未单独隔离该缺陷）。
  - 格林函数表：解析的格子平均 1/r、核覆盖整个网格（见上；DLL 三个方向都按 Nx 截断在 0.75 Nx 格，本格取 0）。
  - integralgrid：DLL 实测无效（输入 4 4 4 与默认结果逐位相同）；新内核两种模式都沿用固定的 10³（不改变输入含义，见待决定 2）。
  - lpboundary：DLL 实测不会强制 FFT（SPICNIC 与 FFT 结果不同）；新内核默认模式在 lpboundary 1 时使用 FFT、纵向周期取 βγλ（DLL 多一个格距）——但 lpboundary 的 t-code 折回尚未实现，
    整个 `lpboundary 1` 目前在两种模式都报 not implemented。
  - 第一次网格的 rms：DLL 用绝对坐标减相对质心（且计入丢失粒子）；默认模式用相对坐标、只计存活粒子。
  - spacechargecomp：默认模式系数 1 − V 截断到 [0, 1]，不在 −4 事件处重启 t-code。
  - z-code 的 HESCFieldPre 默认模式用完整 γ 与 vz = βc/√(1+x′²+y′²)。
  - HESCFieldPre 的 rms：DLL 多线程无保护累加（数据竞争，实测 12 线程时 rms 约小一半、每次不同，见第 2 节）；默认模式串行求和（compat 同样，即 DLL 单线程的结果）。
  - 默认模式与 compat 的 DataSet 差别（SC1：最大 0.7 %，均值列除外）来自上述修正以及 M1–M3 的读回、γ 定义等修正。
- 未修正：PIC 调用计数（“Total number of cycles 0 …”）两种模式仍打印 0；`impscfield 1`（importSCF）与 `lpboundary 1` 报 not implemented。

## 4. 性能（DLL 与新内核的 Simulated total time，同一算例，0.31 m）

| 算例 | Windows AVAS.dll | Windows 新内核 compat | Windows 新内核默认 | macOS 新内核 compat | macOS 新内核默认 |
|---|---|---|---|---|---|
| SC1 SPICNIC 14³，5260 粒子 | 0.96 s | 0.72 s | 0.83 s | 0.16 s | 0.21 s |
| SC2 FFT 64³，5260 粒子 | 4.80 s | 4.87 s | 4.89 s | 1.58 s | 1.52 s |
| SC9 SPICNIC 20³，20000 粒子 | 1.93 s | 0.95 s | 1.24 s | 0.30 s | 0.34 s |
| SC5 SPICNIC Z1（z-code） | 0.60 s | 0.51 s | 0.72 s | 0.11 s | 0.12 s |

Windows 为 XPS（12 线程），macOS 为 Apple Silicon。FFT 64³ 的耗时主要在单线程 pocketfft，与 DLL 的 FFTW_MEASURE 持平。

## 5. 待用户决定（最终）

1. **integralgrid**：默认模式不再使用（格林函数改为解析积分，无需细分数），两种模式都忽略该输入、与原内核一致。请确认（另一选择：报“已不使用”的提示）。
2. **lpboundary**：放到 M5 实现（t-code 纵向折回 + 默认模式周期正好 βγλ、使用 FFT）；在此之前两种模式都报 not implemented。请确认。
3. **SPICNIC / FFT 的 compat 逐位**：t-code 与 z-code 的 DataSet、synData、SpaceChargeMesh 已逐字符一致，粒子坐标末位差 ≤ 1.5e-11（cases.json `compat_tolerance 1e-10`）。
   建议不再追求逐位（FFT 需要 FFTW，SPICNIC 需逐条核对 7000 字节的卷积循环）。请确认。
4. **默认模式分区不随机器线程数截断**：默认模式在任何机器、任何线程数下逐位相同；`maxthreads` 与 `setnumberofcores` 在默认模式只决定分区，实际线程数由 `AVAS_ENGINE_THREADS` 决定。请确认。
5. **默认 SPICNIC 的格林函数修正**（解析积分、全网格核）：改变了默认模式的数值（compat 不变），原内核对长束团 / dc 束的空间电荷明显偏弱。请确认采用。

## 6. 兼容开关累积清单（M1 + M2 + M3 + M4）

M1–M3 各项见 [M3-notes.md](M3-notes.md) 第 5 节（不变）。M4 新增：

| 项目 | compat=dll（AVAS.dll） | 默认（修正） |
|---|---|---|
| 推进、统计、空间电荷的分区 | cores 表截断到 omp 线程数 / maxthreads（结果随机器变） | 固定分区，线程数只影响速度 |
| 电荷分配 | cores[4] 段私有网格按段序归约（multithreading 1 且段数 > 1） | 按粒子顺序单遍累加 |
| SPICNIC 非对称修正 | 每个任务只保留最后一个镜像点（赋值），依赖 cores[7] 分区 | 全部镜像点累加 |
| SPICNIC 目标行 j 起点 | 中间行与末行用 Nz/2 | 所有上卦限目标点 |
| SPICNIC 格林函数表 | 近场 10³ 中点积分（按格数划分近 / 远场）、远场点值 1/r、本格 0、三个方向都截断在 0.75 Nx 格 | 每项为长方体上 1/r 的解析积分（含本格），核覆盖整个网格 |
| 奇数网格 SPICNIC | DLL 挂起（新内核报错） | 报错 |
| lpboundary 1 | 不强制 FFT；纵向周期 βγλ·Nz/(Nz−1) | 使用 FFT、周期 βγλ（整体仍 not implemented） |
| 第一次网格的 rms | 绝对坐标减相对质心、含丢失粒子 | 相对坐标、只计存活粒子 |
| HESCFieldPre | γ 只用 γβz、v = βc、cores[2] 静态分块求和 | 完整 γ、vz = βc/√(1+x′²+y′²) |
| HESCFieldPre 的 rms 求和 | cores[2] > 1 时多线程无保护写共享和（数据竞争，丢失更新，rms 约小一半且每次不同）；compat 无法复现，按无竞争结果（= cores[2] 为 1） | 串行求和 |
| z 段 recordSCF 的中心 | 同步粒子坐标沿用最后一个 t-code 步的旧值 | z 段同步行（参考面） |
| setnumberofcores 读入 | 每项截到 omp_get_max_threads() | 不截断（分区固定，线程数只影响速度） |
| spacechargecomp | 原值（V ≥ 0.999999999 不缩放、负值放大）；−4 事件处重启 t-code | 系数 1 − V 截断到 [0, 1]；不重启 |
| 网格尺寸 | dx、dy 用除法，dz 乘 1/(Nz−1)（按反汇编） | 除法 |
