# 新内核 M3 实现记录：初始分布与随机数

M3 在 M2（engine-rebuild 8e1e6c6）之上实现规格 50 的初始分布生成和随机数，并落实 M2“用户决定”中的第 4、5 项。
原则不变：默认修正缺陷；`compat=dll` 以 Windows AVAS.dll 为逐位基准。

## 1. 代码

| 文件 | 内容 |
|---|---|
| `engine/src/core/rng.h`（新） | `MsvcRand`（MSVC CRT `srand/rand`，LCG，RAND_MAX 32767，各平台逐位相同）；`Philox4x32`（Philox4x32-10，计数器 = 粒子号、流号、抽取号）；可移植的 `p_log`、`p_sincos_turns`、`p_gauss2`（Box–Muller）、`p_parabolic_inv`（3s²−2s³ 的反函数：二分 + Newton）。只用 + − × ÷ 与 sqrt，写成 `AVAS_HD` 模板，编译禁止 FMA 合并 |
| `engine/src/core/distribution.{h,cpp}`（新） | `dll_pretreatment`（PreTreatment，含 Eigen 3.3 `SelfAdjointEigenSolver` 的 2×2 特征值：缩放、隐式对称 QR、Wilkinson 位移、升序排序）；`dll_generate`（KV2D / WB2D / PB2D / GS2D、GenerateCoordinates、GS 的 8 数互异重抽、PB 的 MSVC `<complex>`：`sqrt`、`log(abs)`（双双精度 + log1p）、`polar`（`_Exp`）、Smith 除法）；`default_generate`（新随机数的四种分布） |
| `engine/src/core/beam.{h,cpp}` | 生成路径（Twiss 换算、横纵向生成、DC 束、probe 网格、写 inData.dst 并读回、同步粒子取平均能量）；`.edst` 读入；读回公式抽成 `readback_particles` 供 .dst / 生成 / .edst 共用；`Beam.fromDstFile`、`useWeightFile` |
| `engine/src/core/tracker.cpp` | dstOut 写 `.edst`（初始分布来自 .edst）与 `.dstb`（权重）；默认模式下越过 t 段末端的粒子无场漂移（M2 决定 5） |
| `engine/src/core/synchronous.cpp` | SetUpSynParticle 的 `.edst` 分支（读 beam.txt 的 W、mc²、f，γβz 用入口给定值） |
| `engine/src/core/lattice.cpp` | 默认模式 `spacechargecomp V` 截断到 [0, 1] 并警告（M2 决定 4，空间电荷本身在 M4） |
| `engine/tests/parity/` | 25 个 M3 算例（`D_*`）、`data/small300.edst`、`data/hwr010_seed12345.dstb`；`run_all.py` 新增四项默认模式检查（见第 3 节）；`run_engine.py` 摘要 `.edst` / `.dstb` |
| `engine/tests/unit_tests.cpp` | MSVC rand 已知值（srand(1) → 41, 18467, 6334）、Philox4x32-10 的 Random123 已知答案、可移植 log / sin / cos 与 libm 的误差、PB 反函数 |

## 2. compat=dll（原版随机数与抽样）

- 播种次序与 DLL 相同：`srand(time)` 后横向生成器 `if (seed) srand(seed)`，纵向生成器**再次** `srand(seed)`（固定种子时横纵完全相关，见下）；
  randomseed 先读 input.txt（stoi）后被 beam.txt（stod）覆盖，使用时 `(int)` 截断；DC 束能量展宽在相位赋值后每粒子再抽一个数。
- 按反汇编确认、与 Ghidra 伪代码不同的运算次序：GenerateCoordinates 第二平面用 `(T5 − r²)/T4`（不是 r·r）、角度一次乘 2π/32767、cos 与 sin 分别调用；
  WB 的 `√U·(6ε)`（6ε 先算好）；PreTreatment 的 `T1 = sin2θ·((a/b − b/a)·(0.5/T0))`；PB 的 `s = (c.re − √3 c.im)·0.25 + (d + 0.5)`；
  MSVC 双双精度加法 `((A + B) + (a₁ + b₁)) + s`；ε_z 换算 `(ε·1e-6)/(βγ·γ·γ)`。
- **Windows MSVC：25 个 M3 算例中 24 个全部文件逐字符一致**（inData.dst 与 outData 粒子块 SHA-256 相同、DataSet、synData、synParticle 等），
  覆盖 KV / WB / PB / GS 的横纵组合、seed 12345 / 1 / 99 / 777777、particlenumber 5260 / 200、beamtype dc（含能量展宽）、不写 twissz、probe、
  α = 0、β² = 1 + α²、种子只写在 input.txt、小数种子、`.edst` 输入、`indweight 1` 的 `.dst` + `.dstb`。
  另一个 `D_probe_n100` 是 DLL 的越界读（probe 需要 125 行而只生成 100 行），不可复现，新内核两种模式都至少生成 125 行。
  M1、M2 的全部算例仍逐字符一致（总计 75 个一致 + 4 个已知不可复现）。
- **macOS / Linux 上的 compat=dll 不能逐位**：inData.dst 中约 70–85 % 的粒子有 1 ulp 量级差别（最大相对差约 1e-12，出现在接近 0 的 x′、y′），
  原因是 PreTreatment 的静态 SSE2 sincos、CRT `sin/cos/atan/log/atan2/pow` 在各平台末位不同；PB 中另有 1/5260 个粒子相对差 0.32
  （U ≈ 1 的边界，`atan2`/`pow` 分支，规格 50 Q5）。经过 0.31 m 跟踪后 DataSet 等相对差不超过 1.5e-11（`run_all.py` 非严格判据 1e-10 通过；D_PB_PB、D_PB_KV 与 D_probe 标为 `libm_sensitive`，
只在 Windows 上比对——probe 网格的质心是 ±σ 相消后的 1e-20 量级，末位差别就使其完全不同）。要逐位需内置 UCRT 的 libm（闭源），未做。
- 实测：DLL 的 corr(x, φ) = −1.0000（GS GS 与 KV KV，seed 12345），corr(x′, W) ≈ 0.81；`D_twiss_round`（β² = 1 + α²）中 DLL 的逆矩阵对角元并不逐位相等，
  规格 50 所说的 θ = 0 缺陷没有触发（生成的 α、β 与输入在统计误差内一致），兼容模式照原样保留该判断。
- `.edst` 路径（规格 50 标为未与 DLL 核对）已逐位核实，包括 outData_*.edst（记录含同步粒子、头写 n−1）。

## 3. 默认模式（新随机数）

- **随机数**：Philox4x32-10，密钥 = 种子（64 位），计数器 = （粒子号，流号，抽取号，0）；流 1 横向、流 2 纵向、流 3 DC 能量展宽，互相独立，
  与线程数、调用次序无关。randomseed 0 时取当前时间；实际种子打印为 `Random seed: N` 并写入 `OutputFile/inData.seed.txt`（`randomseed N`），可复现。
- **分布**（各平面 rms 发射度、α、β 等于输入）：KV 为 4 维超椭球面，WB 体内均匀（R² CDF = s²），PB 抛物线（CDF = 3s² − 2s³），GS 不截断的 4 维高斯；
  纵向沿用原内核的定义——同一 4 维分布在一个平面上的投影（KV → 均匀椭圆，WB → ∝ 1 − r²，PB → ∝ (1 − r²)²，GS → 2 维高斯），只是改用独立的流。
  PreTreatment 用闭式 `√β`、`−α/√β`（无 σ11 = σ22 问题），ε ≤ 0 或 β ≤ 0 报错。
- **换算为 .dst**：δ = Δp/p 作用于总动量，γβz = |γβ|/√(1 + x′² + y′²)，W 用总动量；φ = −2πf·z/vz；生成的是同一时刻的快照，
  写入时沿斜率移到 z = 0 的穿越点（x − x′z），读回后恰好回到同一时刻。读回用 libAVAS.so 的公式（|p| = βγ）。
- **probe**：σx、σy 用各自的均值（DLL 除两次 n、y 用 x 的均值），网格以同步粒子为中心（DLL 网格中心与读回坐标系差 v·tMin）；粒子少于 125 时补足。
- **跨平台逐位**：23 个生成算例的默认模式 inData.dst 在 Windows（MSVC 17.12）、macOS（Apple clang）、Linux（WSL Ubuntu 24.04，gcc 13.3）上 SHA-256 完全相同，
  记录在 cases.json 的 `default_indata_sha256`，`run_all.py` 每次检查。
- **统计检验**（`run_all.py`，默认模式，由 inData.dst 反算）：各平面 rms 发射度、β、α 与 beam.txt 在约 4σ 统计误差内一致（5260 粒子时偏差 ≤ 2 %）；
  KV 的 Jx/εx + Jy/εy = 4 到 1e-15；WB、PB 横向径向分布与纵向投影分布的 KS 距离在 1.5 倍临界值内；**corr(x, φ)、corr(x′, W) 的绝对值 < 4/√N**
  （GS GS seed 12345：0.004 / −0.009，原内核 −1.0000 / 0.814）。
- **M2 决定 5**：默认模式下越过 t 段末端（下一段是硬边元件）的粒子无场漂移、仍计入统计。检查：Z1、Z2、ST1 的 dst 算例中第一段（漂移段）的
  DataSet rms x′、y′ 全程不变（原内核在粒子进入 quad / solenoid 区域后变化）。默认模式 Z1_dst 的 DataSet 均值列 ⟨z − zs⟩ 因此再变（对 DLL 的最大相对差 1.15，均值接近 0）。
- **M2 决定 4**：默认模式 `spacechargecomp V` 截断到 [0, 1]（V = 1 即完全补偿）并打印警告；compat=dll 保存原值（V ≥ 1 静默忽略、负值放大），M4 实现空间电荷时使用。
- 能量守恒检查（M2）与全部 M1/M2 默认模式检查继续通过。

## 4. 待用户决定

1. **默认模式的纵向分布定义**：目前沿用原内核“4 维分布投影到纵向平面”（纵向 KV 是均匀椭圆，不是二维 KV 的椭圆边界）。是否改为二维教科书定义？
2. **DC 束**：相位仍按原内核从 π 等间隔递减（不随机），能量展宽 `kneticenergy W ΔW`（DLL 扩展，.so 没有）在默认模式也支持，用独立的随机流。是否保留？
3. **生成坐标写成穿越点**（x − x′z）：物理上与读回一致，但使 inData.dst 的 x 与原内核含义不同（原内核直接写同一时刻的 x，读回时被当作穿越点而多漂移一次）。确认采用。
4. **compat=dll 在 macOS / Linux 不逐位**：是否需要内置与 UCRT 逐位一致的 sin / cos / log / atan2 / pow（需要逆向 UCRT libm，工作量大）？目前只报告差异。
5. **randomseed 0**：默认模式写 `OutputFile/inData.seed.txt` 记录实际种子（新文件）。文件名与位置是否合适？

## 5. 兼容开关累积清单（M1 + M2 + M3）

| 项目 | compat=dll（AVAS.dll） | 默认（修正） | 里程碑 |
|---|---|---|---|
| B-3 V3 = 0 腔 φs 偏差 | 扫描步长 `scantstep/f/N`、相位取步首；主阶段取步末 → φs 偏 +0.07° | 扫描与主阶段同步长、都取步首 | M1 |
| B-1 扫描阶段 V3 = 2 | `φ − 360·f·t` | `φ + 360·f·t` | M1 |
| 扫描中场图外的 EzAmp | 保留上次值 | 置 0 | M1 |
| main_agent(2) | 再运行一次 | 请求停止 | M1 |
| Windows 宽字符路径 | 逐字节去 0 | UTF-16 → UTF-8 | M1 |
| stoi / stod 失败、`vector::at` 越界 | 进程 abort | ErrorLog + 返回 1/2 | M1 |
| field / drift 参数不足 | `invalid` 场图、`exit(1)` 或默认值入表 | ErrorLog 报错 | M1 |
| 同步粒子超出孔径、读不到 beam.txt / 分布文件 | `exit(1)` | ErrorLog + 返回 1 | M1 |
| `.dst` 读回 | vz = βc，pz = βγ，`x·0.01` | vz = βc/√(1+x′²+y′²)，pz = γvz/c | M1 |
| 单宏粒子权重、spOutZ0/Z1 | 0、不设 | 1、按 lattice 设 | M1 |
| DataSet 第 0 列与 γp、SingleParticle / outData 能量 | 只用 γβz（不加权；SingleParticle 写死质子质量） | 完整 \|γβ\|、加权、实际 mc² | M1 |
| lattice 未知关键字 | 静默 | 控制台警告 | M1 |
| solenoid / steerer / bend / edge 参数不足 | 打印后以默认值（名 drift）入表 | ErrorLog 报错 | M2 |
| superpose 含 RF 腔、drift 子块、漏写 superposeend、%f 舍入 | 场行错位、drift 不识别、吞掉下一行、1e-6 m | 正确的场行、识别、警告并处理、%.17g | M2 |
| multipole Kb | G（场 ∝ G²） | 1 | M2 |
| bend 半步判断；弧长 ≠ L 的 bend 的同步时间 | 长度 < 0.85 弧长即半弧；按 L 漂移 | 显式分割标志；补 (弧长 − L)/v | M2 |
| zTotcode、superposeout 重启 | γ 只用 γβz | 完整 γ、vz | M2 |
| z 段中点 `.dst` 参考时间；steerer 冲量 q/m | 同步行在入口；粒子 0 | 推进半个元件；各粒子 | M2 |
| 硬边矩阵 / steerer 后的 \|γβ\|；步内插值的动量大小 | γβz 不变；矢量线性插值 | \|γβ\| 守恒；大小单独插值 | M2 |
| 越过 t 段末端的粒子 | 硬边元件的简化 t-code 场 | 无场漂移 | M3（M2 决定 5） |
| spacechargecomp V | 原值（V ≥ 1 忽略、负值放大） | 截断到 [0, 1] 并警告 | M3（M2 决定 4） |
| 随机数 | MSVC `rand()`；横纵向各自从同一种子重新播种（corr(x, φ) = −1） | Philox4x32-10，横向 / 纵向 / DC 能量独立的流 | M3 |
| randomseed 0 | `srand(time)`，不记录 | 时间种子，打印并写 inData.seed.txt | M3 |
| 抽样中的超越函数 | CRT / 静态 sincos（平台相关） | 可移植的 log、sin、cos（各平台逐位） | M3 |
| PreTreatment | Eigen 特征分解，σ11 = σ22 时 θ = 0 | 闭式 √β、−α/√β | M3 |
| 生成后换算 | δ 加在 γβz 上，γ 不含横向动量，φ 用总速度；x 直接写 | δ 作用于总动量，W 含横向动量，φ 用 vz；x 写穿越点 x − x′z | M3 |
| probe | σ 均值除两次 n、y 用 x 的均值；粒子 < 125 越界；网格中心不在同步粒子 | 各自均值；补足 125；网格以同步粒子为中心 | M3 |
| DC 束能量展宽 | `(rand/32767 − 0.5)·ΔW` | 同式，独立随机流 | M3 |
| `.edst` 的 atomicMass | 写成 mc²（MeV） | mc²/938.271875 | M3 |

## 6. 未实现 / 遗留

- 束流误差（err_beam_*）、二次粒子（secondarybeam 1 的 .dst 与文本）留给 M5，遇到时报 not implemented；空间电荷留给 M4（算例都用 spacecharge 0，
  `spacecharge 1` 的生成路径报 not implemented，空间电荷网格默认值表在 M4 落实）。
- GenerateGaussian（误差研究用的 Box–Muller 缓存）随误差研究在 M5 实现。
