# 60 空间电荷（PIC：FFT / SPICNIC）

来源：`decomp/libAVAS/functions/` 中分配给本子系统的 40 个函数，另外读了调用方 `MethodMP::Run`、`MethodMP::tcodeRun`、`MethodMP::zcodeRun`、
`Initial::Read`、`Initial::Initial`、`Initial::InitParticle`、`MethodMP::UpdateBeamParameters`、辅助函数 `findClosest`、`monotonicSearch`，
用 `B_tools/libAVAS.asm` 和 `libAVAS.so` 的 `.rodata` 补回 Ghidra 丢掉的字符串、默认值数组和返回值。`AVAS.dll` 只用来查 `spacechargecomp`。
偏移量按 [structures.md](structures.md) 的命名翻译，结构表需要更正的地方集中在第 6 节。

约定：Ghidra 把 `PIC::Weight(Mesh&,Beam&,double)` 当成静态函数，伪代码里 `param_1` 是 `this`（PIC）、`param_2` 是 Mesh、`in_RDX` 是 Beam；
`tcodeRun` 里写成 `PIC::Weight((Mesh*)param_11,(Beam*)param_12,…)` 的地方，`param_11` 是 PIC，`param_12` 是 Mesh。下文已按实际含义换算。

---

## 1. 概述

空间电荷子系统根据宏粒子当前位置，在一个三维矩形网格上求出束流自身的电场，写进每个粒子的 `scE`（Particle+0x60），`scB`（+0x78）恒为 0。
推进器再把 `scE` 和外场合起来用（t-code：totE = extE + scE，见推进文档；z-code：`HardEdgeTrans` 在后半个元件按 `scE` 给动量冲量）。

求解在**束流静止系**中进行：纵向坐标相对束流中心按 γ 拉伸，求出静止系电场后，横向分量除以 γ（等效地把磁力 v×B 合进来），纵向分量不变，磁场置 0。

两种求解器（input.txt `scmethod`，**只接受 `FFT` 与 `SPICNIC`**）：

- **FFT**（scMethod = 0）：CIC 电荷分配 → 横向（x、y）二维 DST-I（导体壁、狄利克雷边界）+ 纵向一维复 FFT（周期边界）求解离散泊松方程 → 中心差分求 E → CIC 插值回粒子。
- **SPICNIC**（scMethod = 1，**内核默认值**）："对称 PICNIC"。CIC 电荷分配 → 每步重新制表格林函数（近场用 10×10×10 中点积分求单元平均 1/r）→ 假定电荷分布关于网格中心三个平面镜像对称，只用一个卦限的电荷、直接卷积求一个卦限的电势，再镜像到 8 个卦限 → 加"非对称修正"（实现有缺陷，见 2.7.5）→ 差分求 E → CIC 插值。

调用顺序：

```
MethodMP::Run / SeRun
  Mesh::Mesh, Mesh::Initial(numofgrid, N)        一次
  PIC::PIC(Mesh, Beam, scPar, lpboundary) → PIC::Initial      一次（线程分区、FFTW 计划、核表、读 SpaceChargeMesh.txt）
  tcodeRun 每个时间步（MethodMP.spaceCharge == 1 时）：
      Beam.time = 当前时间
      若 Beam.impScField == 0:
          scMethod == 0:  Weight → FFT → Allocate
          scMethod == 1:  若 autoPIC() == 1: Weight → PICNIC → Allocate；否则什么都不做
          其他值：        什么都不做
      否则: importSCF
      PIC+0x348 ++                                  （调用计数，无人读取）
  zcodeRun 每个硬边元件（前半段 HardEdgeTrans 之后，spaceCharge == 1 时）：
      HESCFieldPre（z-code 坐标 → 同一时刻的相对坐标，并重算 Beam 中心与 rms）
      其余同上（scMethod == 1 且 autoPIC 返回 0 时同样跳过）
      Allocate 内部：Beam.recScField == 1 时 recordSCF
  PIC::~PIC, Mesh::~Mesh
```

"什么都不做"时粒子保留上一次的 `scE`（不重新插值），这是可观测的行为。

---

## 2. 规格正文

### 2.1 关键字、默认值与自动设置

#### 2.1.1 input.txt 关键字（`Initial::Read`）

| 关键字 | 存放 | 读法 |
|---|---|---|
| `spacecharge V1` | Initial+0x110（int，默认 1） | 整数。为 0 时 stdout 打印 "Simulation ignores space charge effects" |
| `scmethod V1` | scPar[0] | `V1 == "FFT"` → 0.0；`V1 == "SPICNIC"` → 1.0；其他抛 std::string 异常 **"SCMethod Contains unknown item, available options include FFT / SPICNIC"**。区分大小写（推测：字符串直接比较）。**`PICNIC` 会报错**（schema.py 列出了 PICNIC，与内核不符） |
| `meshrms V1 V2 V3` | scPar[1..3] | 有几个读几个（double） |
| `integralgrid a b c` | scPar[4..6] | 读入，但 `PIC::Initial` 把积分细分数**硬写成 10、10、10**，输入无效 |
| `scutoffdistance V1` | scPar[7] | 按整数解析后存 double；.so 的 PIC 不读它（无效） |
| `negligibleratio V1` | scPar[8]（默认 0.09） | SPICNIC 忽略电荷的阈值比例 |
| `numofgrid Nx Ny Nz` | Initial+0x1c8 vector\<int\>(3) | 整数 |
| `lpboundary V1` | Initial+0x148 → MethodMP+0x1d0 → PIC+0x1e8 | 整数，1 = 纵向周期网格长度取 βγλ（见 2.4.2） |
| `multithreading V1` | Initial+0x1e8（默认 **1**） | 由 `InitParticle` 拷到 scPar[9] → PIC+0x1e4：1 时空间电荷用线程池 |
| `setnumberofcores c0 … c9` | Initial+0x88 vector\<int\>(10) | 少于 10 个参数抛 **"keyword: setnumberofcores, Insufficient parameters, need 10 parameters"**；每个截断到 `omp_get_max_threads()`，拷到 scPar[10..19] |
| `maxthreads V1` | Initial+0x1ec（默认 −1） | >0 时 `InitParticle` 再把每个 cores 截断到它 |
| `scsolverrecord V1` | Initial+0x1c4 → Beam+0x6c | 本子系统不读 |

beam.txt：`recscfield`（Beam+0xd8，=1 时每次 Allocate 后写场）、`impscfield`（Beam+0xdc，=1 时不求解，改为从文件读场）。

#### 2.1.2 默认值（`Initial::Initial`，从 `.rodata` 0xd9e80、0xd9e28、0xd9e40 读出）

```
scPar[20] = {1.0(SPICNIC), -1,-1,-1 (meshrms), -1,-1,-1 (integralgrid), -1 (scutoff), 0.09 (negligibleratio),
             1.0 (scPar[9]), 1,1,1,1,1,1,1,1,1,1 (scPar[10..19])}
numOfGrid = {-1,-1,-1}      cores = {-1,1,1,1,1,1,1,1,1,1}      spaceCharge = 1   multithreading = 1
```

#### 2.1.3 `Initial::InitParticle` 中的自动设置（N = particlenumber）

先：`current == 0` 或 `particlenumber == 1` → `current = 0`、Beam.current = 0、spaceCharge = 0，stdout 打印 "0 intensity, stop SCfield calculations"。
`Initial::Read` 读完文件后若 `secondarybeam == 1` 也把 spaceCharge 清 0。

spaceCharge == 1 时（以下 "−1" 指仍为默认值，只检查第一个分量）：

- scMethod == 0（FFT）：meshrms[0] == −1 → meshrms = (6.5, 6.5, 6.5)；Nx == −1 → numofgrid = (64, 64, 64)。
- scMethod == 1（SPICNIC）：
  - meshrms[0] == −1 → 三个都取 a(N)：N ≤ 3e4: 3.5；≤ 8e4: 3.5；≤ 3e5: 4.0；≤ 8e5: 4.0；≤ 3e6: 5.0；≤ 8e6: 5.0；≤ 3e7: 6.0；≤ 8e7: 6.0；≤ 3e8: 7.0；其余 7.0。
  - integralgrid[0] == −1 → (8, 8, 8)（之后被 PIC 覆盖为 10，无效）。
  - Nx == −1 → 三个都取 g(N)：≤ 3e4: 14；≤ 8e4: 18；≤ 3e5: 20；≤ 8e5: 26；≤ 3e6: 30；≤ 8e6: 40；≤ 3e7: 44；≤ 8e7: 56；其余 64。
  - scPar[7] == −1 → ceil(Nx/3.0)（无效）。
- 最后 scPar[9] = multithreading。

spaceCharge == 0 时：numofgrid = (3, 3, 3)（PIC 仍会构造，但不调用求解）；meshrms 保持 −1。

cores：multithreading == 1 且 cores[0] == −1 时按 N 取表（.rodata 0xd9bc0–0xd9e28）：

| N 上限 | c0 | c1 | c2 | c3 | c4 | c5 | c6 | c7 | c8 | c9 |
|---|---|---|---|---|---|---|---|---|---|---|
| 3e4 | 64 | 54 | 20 | 24 | 1 | 48 | 96 | 12 | 48 | 12 |
| 8e4 | 72 | 68 | 24 | 56 | 22 | 48 | 96 | 20 | 48 | 24 |
| 3e5 | 96 | 96 | 32 | 96 | 26 | 48 | 96 | 24 | 48 | 36 |
| 8e5 | 96 | 96 | 52 | 96 | 74 | 48 | 96 | 48 | 48 | 96 |
| 3e6 | 96 | 96 | 62 | 96 | 86 | 48 | 96 | 48 | 48 | 96 |
| 8e6 | 96 | 96 | 82 | 96 | 92 | 48 | 96 | 48 | 48 | 96 |
| 3e7 | 96 | 96 | 92 | 96 | 96 | 48 | 96 | 48 | 48 | 96 |
| ≥ 8e7 | 96 | 96 | 96 | 96 | 96 | 48 | 96 | 48 | 48 | 96 |

（N ≤ 8e7、≤ 3e8、> 3e8 三档相同。）然后每个 c_k ← min(c_k, omp_get_max_threads())，maxthreads > 0 时再 ← min(c_k, maxthreads)，存入 scPar[10+k]。
本子系统用到的：c2 = HESCFieldPre 的 OpenMP 线程数；c4 = Weight 的任务数；c5 = Weight / PICNIC 归约的 OpenMP 线程数；c6 = 格林函数制表任务数；
c7 = SPICNIC 卷积任务数；c8 = FFT / PICNIC 求梯度的 OpenMP 线程数；c9 = Allocate 的任务数。c0、c1、c3 属于推进部分。

#### 2.1.4 `spacechargelong`、`spacechargetype`、`spacechargecomp`

- `spacechargelong`、`spacechargetype`：libAVAS.so 与 AVAS.dll 的字符串表里都**没有**，内核不认识（schema.py 注明"只对包络模型起效"，包络模型不在内核里）。
  写进 input.txt 时 `Initial::Read` 走 "input.txt Contains unknown keywords, check input.txt" 分支（属于输入子系统）。
- `spacechargecomp V1`（lattice）：**只有 AVAS.dll 有**，.so 没有。DLL 的 lattice 读取（FUN_180030ba0）遇到它时压入一条记录
  `{当前累计 z（为空时 0）, 1.0, V1}` 到一个 vector\<vector\<double\>\>，再拷进 MethodMP（DLL 偏移 +0x2c0）；DLL 的 tcodeRun 在某个 z 处查找
  `|rec[0] − z| < 1e-9 且 |rec[1] − 1| < 1e-9` 的记录，把 rec[2] 写进 Beam 的一个新成员（DLL Beam+0xe8）；找不到时下标为 −1（越界读，DLL 的缺陷）。
  DLL 的 `PIC::Weight`（FUN_1800835c0）在电荷分配之后：若该值 **< 0.999999999**，把电荷网格每个格点乘 (1 − V1)。
  所以 V1 是补偿度（0 = 不补偿）；V1 ≥ 0.999999999（含 1.0 = 完全补偿）反而**不缩放**（推测为实现缺陷或"未设置"哨兵）。哪个 z 触发查找、Beam 成员初值，需要细读 DLL tcodeRun（见第 5 节）。
  **触发位置（M11，2026-11-17）**：只有 t 段（场图、漂移段）在 `zS ≤ z < zE` 的元件入口生成事件（40 §4.2.12 的 −4）；写在 quad、solenoid、bend、edge 之前的记录不生效（z 段的空间电荷半步冲量照样用当时的系数），
  写在第一个元件之前时事件位于束团头部之后，报 “Tcode won't start”。新内核默认模式在 z 段元件开始时应用记录，lattice 起点的记录在运行开始时应用；兼容模式与 AVAS.dll 相同。

### 2.2 对象与一次性初始化

#### 2.2.1 Mesh（0x140）

`Mesh::Mesh` 全部清零。`Mesh::Initial(numOfGrid, N)`：`n = numOfGrid`，`nParticles = (int)N`，`cellOf` = N 个 vector\<int\>(3)，`xs/ys/zs` 长 Nx/Ny/Nz，
8 个 `grid[g]` 都是 Nx×Ny×Nz（下标 [i][j][k]）。用途：grid[0] 节点电荷（C，**不是密度**），grid[1] 电势 φ（V），grid[2..4] 静止系 Ex、Ey、Ez（V/m）；
grid[5..7] 在本子系统的函数里没有读写（推测为预留的 B 场）。`Mesh::MeshGenerator(Beam&)` 只 `return 0`。

#### 2.2.2 PIC 构造

`PIC::PIC(Mesh&, Beam&, vector<double>& scPar, int lpBoundary)`：线程池 `progschj::ThreadPool(max(2, hardware_concurrency()))`；
+0x170 = 1.0（选新 PICNIC 路径，之后**从不改为 0**）；+0x1e4 = 1；+0x1f0 = vector\<int\>(10)；+0x208 = vector\<double\>(3) 全 0；
+0x2b0 = vector\<int\>(3)；+0x530..0x53c = 7,7,7,7（两组 FFTW r2r kind = `FFTW_RODFT00`，即 DST-I）；+0x1e8 = lpBoundary；然后调 `PIC::Initial`。
无参构造 `PIC::PIC()` 做同样的初始化但不调 `Initial`，代码里没有调用者。

#### 2.2.3 `PIC::Initial(Mesh&, Beam&, scPar)`

1. 若 lpBoundary == 1：scMethod(+0x258) = 0。**随后第 5 步又被 scPar[0] 覆盖**，所以这个"lpboundary 强制 FFT"实际无效（伪代码顺序明确）。
2. Nx,Ny,Nz ← mesh.n；Mx = My = Mz = (int)round(Nx·3·0.25)（**三个都由 Nx 算**）存 +0x2cc..0x2d4；+0x2d8..0x2e0 = (int)(round(4N/7) − 2)（无人使用）；
   +0x1c4.. = N−1；+0x1d0.. = N/2（整数除法）。
3. meshrms → +0x240..0x250；scMethod = scPar[0]；+0x1e4 = (int)scPar[9]；积分细分 +0x2b0 先写 (int)scPar[4..6]，**随即改为 (10,10,10)**。
4. 近场半径平方 R2 = (int)ceil((Nx·0.5 + 1)²)（+0x2c8）。cores[k] = (int)scPar[10+k]；若 +0x1e4 == 0，cores 全部置 1。
5. 粒子数组 per-particle 权重表 +0x288 = N 个 vector\<double\>(8)（零）。negligibleratio = scPar[8] → +0x228；+0x238（TextVol 用）= 0。
   stdout 打印一行：`Nx Ny Nz negligibleratio meshrms_x meshrms_y meshrms_z`（分隔符推测为单个空格）。
6. 线程分区（只影响多线程路径，见 2.10）：
   - Weight 粒子分区 +0x350：c = (int)ceil(N/c4)，边界 0, c, 2c, …（< N），最后 N；c4 ← 块数。
   - Allocate 粒子分区 +0x368：同上用 c9；c9 ← 块数。
   - SPICNIC 目标点二维分区 +0x380/+0x398（起点 i、终点 i）、+0x3b0/+0x3c8（起点 j、终点 j）：A = Nx/2，B = Ny/2，c = (int)ceil(A·B / c7)，
     q = c / B，r = c % B；起点从 (A, B) 开始，每次 (i, j) ← (i + q, j + r)，j ≥ Ny 时折回 (i+1, j − Ny + B)，j 恰好等于 Ny 时下一段从 (i+1, B) 开始；
     i 到达 Nx−1 时收尾，最后一段终点为 (Nx−1, Ny)，r == 0 时多插一个分点 (Nx−1, B)。c7 ← 段数。
   - 制表二维分区 +0x3e0/+0x3f8/+0x410/+0x428：同样算法，用 (Mx, My)、起点 (0, 0)、终点 (Mx−1, My)、任务数 c6。c6 ← 段数。
   - stdout 打印 `mulitthread:c0  c1 c2 c3 c4 c5 c6 c7 c8  c9`（拼写原文如此；c4、c6、c7、c9 为改写后的段数）。
7. FFTW：8 个 `fftw_malloc` 缓冲（+0x4d0、0x4d8、0x4e0、0x4e8、0x4f0 各 Nx·Ny·Nz 个 double；+0x4f8、0x500、0x508 各 Nx·Ny·Nz 个 complex；
   0x4d0/0x4d8/0x4e0 从未使用）。四个计划，flags 都是 0（`FFTW_MEASURE`），数据布局 idx = (i·Ny + j)·Nz + k：
   - +0x510 `fftw_plan_many_r2r(2, {Nx,Ny}, howmany=Nz, in=0x4e8, inembed={Nx,Ny}, istride=Nz, idist=1, out=0x4f0, 同上, kind={RODFT00,RODFT00})`
   - +0x520 `fftw_plan_many_dft(1, {Nz}, howmany=Nx·Ny, in=0x4f8, istride=1, idist=Nz, out=0x500, …, FFTW_FORWARD)`
   - +0x528 同上，0x500 → 0x508，`FFTW_BACKWARD`
   - +0x518 r2r，0x4f0 → 0x4e8，DST-I
8. 核表：+0x2e8、+0x300、+0x318 三个 (2Mx+1)×(2My+1)×(2Mz+1)（旧路径的 E 核，新路径不用），+0x330 一个 (2Nx+1)×(2Ny+1)×(2Nz+1)（φ 核）。
9. 若 Beam.impScField == 1：读 `inputFilePath + "SpaceChargeMesh.txt"`（见 2.9.2），打不开抛 **"SpaceChargeMesh.txt file reading error"**。
10. 返回 1。

引用的 libfftw3 函数一共 6 个：`fftw_malloc`、`fftw_plan_many_r2r`、`fftw_plan_many_dft`、`fftw_execute`、`fftw_destroy_plan`、`fftw_free`。
没有用 FFTW 的多线程接口。

### 2.3 z-code 的准备：`MethodMP::HESCFieldPre(Beam&, coords, Lattice&, double& tRef)`

z-code 中粒子状态放在 `coords[i]`（每粒子一个 vector\<double\>），本函数用到 [0] x、[1] γβx、[2] y、[3] γβy、[5] γβz、[6] 到达参考面的时间（推测）、
[7] 存活标志（== 1.0 为存活）。目的：把"同一 z、不同时刻"的粒子换成"同一时刻 tRef"的相对坐标，填 Particle.rel，供 Weight 使用。

第一个并行区（OpenMP，c2 个线程，粒子按静态块分给线程）：对每个存活粒子 i：

```
Δt  = coords[6] − tRef
γ   = sqrt(γβx² + 1 + γβy² + γβz²)                 （求和顺序：γβx² + 1.0 + γβy² + γβz²）
β   = sqrt(1 − (1/γ)/γ)
x'  = γβx/γβz,  y' = γβy/γβz
vz  = sqrt((β·c·β·c) / (x'² + y'² + 1))           （伪代码：(β·299792458·β·299792458)/(…)）
rel[4] = −Δt·vz
rel[0] = x − ((γβx·Δt)/γ)·c,   rel[2] = y − ((Δt·γβy)/γ)·c
rel[1] = γβx,  rel[3] = γβy,  rel[5] = γβz
```

每线程累加 n（个数，**不加权**）和 Σrel[0..5]，`GOMP_atomic` 区里加到共享和（线程合并顺序不定 → 多线程时求和结果有舍入级的非确定性）。
然后 Beam.centroid[k] = Σrel[k]/n，Beam.rms 清零；γc = 1/sqrt(1 − (c5/sqrt(c1² + 1 + c3² + c5²))²)（c 为 centroid）。
第二个并行区累加 (rel[0]−c0)²、(rel[2]−c2)²、((rel[4]−c4)·γc)²（每次直接加到共享数组，**没有原子保护**，多线程时存在数据竞争；DLL 的 FUN_180090940 同样是普通的读-加-写。
**已用 DLL 实测证实**（M4-notes 第 2 节）：12 线程时 rms 约小一半、每次运行不同，网格随之缩小、场变强；单线程时与无竞争的实现逐字符相同）；
最后 rms[k] = sqrt(Σ/n)。

注意：本函数**不更新 Beam.gamma（+0x78）**，而 Weight 用 Beam.gamma 做纵向拉伸，所以 z-code 中拉伸用的是上一次 `UpdateBeamParameters`/`zTotcode` 算的 γ。

### 2.4 坐标系与网格（`PIC::Weight` 前半）

输入全部取自 Beam：centroid（+0x00）、rms（+0x18）、gamma（+0x78）、betaGammaLambda（+0x70）、粒子 rel（Particle+0xc0..0xe8）。
在 t-code 中 rel = 相对同步粒子的位置（动量为绝对 γβ），centroid、rms 由 `UpdateBeamParameters` 给出：centroid = 存活粒子（lost ≠ 1）按 weight 加权的 rel 平均；
rms[2] 已乘 γ（静止系纵向 rms）；gamma = sqrt(1 + ⟨γβx⟩² + ⟨γβy⟩² + ⟨γβz⟩²)。

#### 2.4.1 一般情况（lpBoundary ≠ 1）

```
dx = ((rms[0] + rms[0]) · meshrms_x) / (Nx − 1)
dy = ((rms[1] + rms[1]) · meshrms_y) / (Ny − 1)
dz = ((rms[2] + rms[2]) · meshrms_z) / (Nz − 1)
xs[i] = ((i − Nx·0.5) + 0.5)·dx + centroid[0]        i = 0..Nx−1（Nx·0.5 为 double）
ys[j] = ((j − Ny·0.5) + 0.5)·dy + centroid[2]
zs[k] = ((k − Nz·0.5) + 0.5)·dz + centroid[4]
```

即网格**节点**覆盖中心 ± meshrms·rms，网格以束流中心（不是同步粒子）为中心，**每次调用都按当前 rms 重建**。
纵向坐标在静止系：粒子的纵向坐标取 `zr = (rel[4] − centroid[4])·γ + centroid[4]`（相对中心拉伸 γ）。横向不变。

#### 2.4.2 lpBoundary == 1

```
dx, dy 同上；dz = betaGammaLambda / (Nz − 1)
xs[i] = ((i − Nx·0.5) + 0.5)·dx       （不加中心，以同步粒子为原点）
ys[j], zs[k] 同理，不加中心
```

纵向周期长度 = Nz·dz = βγλ·Nz/(Nz−1)，**比 βγλ 长一个格距**（推测为实现缺陷）。粒子纵坐标的拉伸见 2.5（串行与多线程两条路径公式不同）。
如 2.2.3 第 1 步所述，lpboundary 并不强制 FFT。

> **默认模式（M10 D7）**：`lpboundary V1 1`（`Params::lpPeriodic`，此时 lpBoundary = 0，粒子不折叠）：强制 FFT；纵向网格是束流静止系中的一个射频周期 L = λ·(γβz)_s，dz = L/Nz，节点 (k − Nz/2 + 0.5)·dz，拉伸 γ 用同步粒子的完整 γ_s、以同步粒子为原点（lpRel）；横向网格与中心同一般情况。沉积与插值时 zr 折回 [zs0, zs0 + L)，最后一格连接节点 Nz−1 与节点 0（`MeshV::lpWrap/lpLen`，mesh_kernels.h，CPU、CUDA、Metal 共用），没有粒子落在纵向网格外。FFT 的 z 方向本来就是周期为 Nz·dz 的 DFT。beam.gamma 与 DataSet 不受影响（07-22 版改写 beam.gamma 并用 βλ 作周期，短 γ 倍，不跟）。检验：1.3 GHz 一个周期长（13 mm）的 DC KV 束、20 mA、1 m 漂移，全部粒子的 rms 增长与 KV 包络之比 0.9949／0.9957（开放盒 0.891），见 physics/dc_kv_envelope.py --periodic-check。

#### 2.4.3 其他量

PIC+0x2a0 = γ，+0x2a8 = 1/γ，+0x278 = V = dx·dy·dz，+0x280 = 0（随后累加网格内粒子的 weight）。

### 2.5 电荷分配（CIC，`PIC::Weight` 后半 / `Calculate_Weight`）

先把 grid[0] 每个元素置 0。对粒子 n = 0..N−1（**不含同步粒子**），跳过 `lost == 1`（`lost == 2` 的粒子照样分配）：

```
zr = (rel[4] − centroid[4])·γ + centroid[4]
i = (int)floor((rel[0] − xs[0]) / dx);  j = (int)floor((rel[2] − ys[0]) / dy);  k = (int)floor((zr − zs[0]) / dz)
cellOf[n] = {i, j, k}
若 0 ≤ i < Nx−1 且 0 ≤ j < Ny−1 且 0 ≤ k < Nz−1：
    Particle+0xf8 = 0；PIC+0x280 += weight
    w[m] = |x − xs[i + ((m&1)^1)]| · |y − ys[j + ((m&3)==1 || (m&3)==0)]| · |zr − zs[k + (m<4)]| / V    m = 0..7
           （乘法顺序：(|Δx|·|Δy|)·|Δz|，再除以 V）
    依次 grid[0][i][j][k] += w0·q;  [i+1][j][k] += w1·q;  [i][j+1][k] += w2·q;  [i+1][j+1][k] += w3·q;
         [i][j][k+1] += w4·q;  [i+1][j][k+1] += w5·q;  [i][j+1][k+1] += w6·q;  [i+1][j+1][k+1] += w7·q
    （q = Particle.charge，+0x118，宏粒子电荷 C；伪代码写成 w·q + old）
否则：Particle+0xf8 = 1，w[0..7] = 0
w[] 存入 PIC+0x288[n]，供 Allocate 复用。
```

这是标准的三线性（CIC）权重：w0 对应节点 (i,j,k)，权重为到对角节点的距离积。代码里另有 "k == Nz−1 时 k+1 折回 0" 的分支，但由于条件 k < Nz−1 已排除，**是死代码**。

多线程路径（+0x1e4 == 1 且 c4 ≠ 1）：c4 个线程池任务各处理 +0x350 的一个粒子段，写各自的局部网格 L[t]（同尺寸），
网格内粒子的 weight 累加到 `int counts[t]`（`counts[t] = (int)((double)counts[t] + weight)`，**逐次截断为整数**），再用 c5 个 OpenMP 线程按 i 平面归约：
`grid[0][i][j][k] = ((0 + L[0]) + L[1]) + … + L[c4−1]`；PIC+0x280 += Σ counts。
任务函数 `Calculate_Weight` 多一个整型参数 f：f ≠ 1 时 `zr = (rel[4] − centroid[4])·γ + centroid[4]`，f == 1 时 `zr = rel[4]·γ + centroid[4]`。
线程池入队的参数被 Ghidra 截断，f 的来源看不到（推测为 PIC+0x1e8 = lpBoundary）。串行路径没有这个分支。

### 2.6 FFT 泊松求解（`PIC::FFT` + `._omp_fn.8`）

求 ∇²φ = −ρ/ε₀ 的离散解（7 点差分拉普拉斯），横向狄利克雷、纵向周期：

1. `buf[0x4e8][(i·Ny+j)·Nz+k] = grid[0][i][j][k] · s`，s = ((1/dx)/dy)/dz（电荷 → 电荷密度）。
2. 执行 +0x510（x、y 两维 DST-I）。
3. 0x4f0 的实数拷进复数缓冲 0x4f8（虚部 0），执行 +0x520（z 向正 FFT）。
4. 本征值：
   ```
   λx[i] = ((4.0/dx)/dx) · s·s,   s = sin((i+1)·(1.5707963267948966/(Nx+1)))     i = 0..Nx−1
   λy[j] 同理（Ny）
   λz[k] = ((4.0/dz)/dz) · s·s,   s = sin(k·(3.141592653589793/Nz))             k = 0..Nz−1
   ```
   对每个 (i,j,k)：f = 112940906683.72818 / ((λx[i] + λy[j]) + λz[k])（= 1/ε₀ ÷ λ，ε₀ = 8.854187817e-12），复数的实部、虚部都乘 f。
   因为 λx > 0，分母不会为 0。
5. 执行 +0x528（z 向逆 FFT）→ 0x508；实部拷回 0x4f0；执行 +0x518（DST-I）→ 0x4e8。
6. `grid[1][i][j][k] = buf[0x4e8][…] · (1.0 / (double)(int)((4·Nx + 4)·(Ny + 1)·Nz))`，即除以 2(Nx+1)·2(Ny+1)·Nz（FFTW 不归一化）。
   注意分母先按 **int** 相乘，网格很大时会溢出（2^31 ≈ 2.1e9；例如 1024³ 量级）。
7. 求场（OpenMP，c8 个线程，按 i 静态分块），hx = 1/(dx+dx) 等：
   ```
   Ex[i] = −(φ[i+1] − φ[i−1])·hx，  φ[−1] = φ[Nx] = 0      （i = 0：−φ[1]·hx；i = Nx−1：−(0 − φ[Nx−2])·hx）
   Ey 同理（狄利克雷）
   Ez[k] = −(φ[k+1] − φ[k−1])·hz，k 周期：k=0 用 φ[Nz−1]，k=Nz−1 用 φ[0]
   ```
   结果写 grid[2]、grid[3]、grid[4]。

几何含义：导体壁在最外层节点之外一个格距处（x = xs[0] − dx 与 xs[Nx−1] + dx），即 ±(meshrms·rms + dx)；
纵向周期 Nz·dz = 2·meshrms·rms_z·Nz/(Nz−1)，束团在纵向有周期镜像。

### 2.7 SPICNIC（`PIC::PICNIC` 新路径）

#### 2.7.1 总流程（PIC+0x170 == 1.0，恒成立）

```
1. 制表：c6 个任务 Calculate_Tabulation，填 φ 核 G（+0x330）
2. thr = negligibleratio · grid[0][Nx/2][Ny/2][Nz/2]          （PIC+0x230，中心格点电荷）
3. 每任务一个修正数组 C[t]（Nx×Ny×Nz，零初始化，t = 0..c7−1）
   c7 个任务 Calculate_S_PICNIC：直接卷积写 grid[1]，并写 C[t]
4. 归约（OpenMP c5 线程，按 i 平面）：grid[1][i][j][k] = ((φ + C[0]) + C[1]) + …
5. 求场（OpenMP c8 线程）：
   内点 E = −(φ[i+1] − φ[i−1])·hx；边界用单侧差分 E[0] = 2·(−(φ[1] − φ[0])·hx)，E[N−1] = 2·(−(φ[N−1] − φ[N−2])·hx)
   x、y、z 三个方向相同（都不是周期）
```

注意：串行/并行由线程池决定，但 SPICNIC **始终走线程池**（不看 +0x1e4）；+0x1e4 == 0 时各 c_k = 1，等价于单任务。
grid[1] 在调用之间不清零，靠第 3 步覆盖每个点（见 2.7.4 的例外）。

#### 2.7.2 格林函数制表（`Calculate_Tabulation`，每步重做）

参数：dx、dy、dz、V、核半宽 Mx、My、Mz、中心 (Nx, Ny, Nz)、近场半径平方 R2、细分 n = (10,10,10)。对任务分到的 (a, b)（0 ≤ a < Mx、0 ≤ b < My）和全部 0 ≤ c < Mz：

```
s2 = a² + b² + c²（整数）
s2 == 0          → g = 0
s2 >  R2         → g = 1 / pow((a·dx)² + (b·dy)² + (c·dz)², 0.5)
否则              → g = Integrate_3D_Phi([a·dx ± dx/2] × [b·dy ± dy/2] × [c·dz ± dz/2], n) / V
写入 G[Nx ± a][Ny ± b][Nz ± c]（8 个镜像位置，同一个值）
```

`Integrate_3D_Phi`：每维等分 n 份，中点 x_m = x0 + (m + 0.5)·h；按 x 外层、y、z 内层的顺序累加 `((hx·hy)·hz)·(1/pow(x² + y² + z², 0.5))`。
即近场取单元平均的 1/r（中点规则），远场取点值 1/r。|偏移| ≥ M 的表项保持 0（核截断在 0.75·Nx 个格距）。
R2 = ceil((Nx/2 + 1)²)，大于 Mx² 时（Nx ≥ 8 左右）所有表项都走积分。

#### 2.7.3 对称卷积（`Calculate_S_PICNIC`）

记 ρ = grid[0]（节点电荷），k_e = 8987551787.368176（1/4πε₀）。任务负责的目标点：i ∈ [i_s, i_e]（闭区间，i ≥ Nx/2），
j 的范围：首行 [j_s, Ny)，中间行 [⌊Nz·0.5⌋, Ny)，末行 [⌊Nz·0.5⌋, j_e)，只有一行时 [j_s, j_e)；k ∈ [⌊Nz·0.5⌋, Nz)。
（中间行、末行的 j 起点用的是 **Nz/2 而不是 Ny/2**，推测为源代码笔误；Ny == Nz 时无影响；Ny ≠ Nz 时可能漏算或多算若干 j 行。）

对每个目标 (i, j, k) 计算

```
S = Σ_{i'} Σ_{j'} Σ_{k'}  ρ̃(i',j',k') · G[Nx − i + i'][Ny − j + j'][Nz − k + k']，只累加 ρ̃ > thr 的项（严格大于）
```

源点范围：i' ∈ [max(0, ⌊i − Mx⌋), min(Nx, i + Mx))，j'、k' 同理。每一维分成两段：
下半段 i' < Nx·0.5 使用**镜像电荷** ρ[Nx−1−i']，上半段 i' ≥ ⌊Nx·0.5⌋ 使用 ρ[i']；三维共 8 种组合，各自独立循环（循环嵌套顺序 i' 段 → j' 段 → k' 段，段内 i' 外层、k' 内层，按升序累加）。
即：**只用上卦限（i ≥ Nx/2、j ≥ Ny/2、k ≥ Nz/2）的电荷，并假定电荷分布关于网格中心三个平面镜像对称**。

然后 φ = S·k_e 写到目标点及其镜像：`for I in {Nx−1−i, …, i}（步长 2i − (Nx−1)） for J … for K …: grid[1][I][J][K] = S·k_e`
（偶数 N 时就是 I ∈ {Nx−1−i, i}）。

#### 2.7.4 奇数网格

- N 为奇数时 Nx/2 = (Nx−1)/2，首个目标 i = (Nx−1)/2 的镜像步长 2i − (Nx−1) = 0，`do … while (I <= i)` **死循环**（推测：程序挂起；三个方向任一为奇数都会如此）。
- 即使不挂起，源点分段在奇数 N 时中心平面 i' = (Nx−1)/2 会被两段各算一次。
- 默认网格（2.1.3）全是偶数。

#### 2.7.5 "非对称修正"及其缺陷

在写镜像点的循环里，对每个镜像点 P = (I, J, K) 计算 Δ = ρ(P) − ρ(i, j, k)；若 |Δ| ≥ thr（`thr <= Δ || Δ <= −thr`）：

```
for a, b, c 遍历全网格:  C[t][a][b][c] = G[Nx + I − a][Ny + J − b][Nz + K − c] · Δ · k_e      ← 赋值，不是累加
```

本意是把"实际电荷 − 对称化电荷"的电势叠加上去，但写成了**赋值**：每个任务的 C[t] 最后只保留该任务中**最后一个**满足条件的镜像点的贡献
（遍历顺序：i 升序 → j 升序 → k 升序 → I、J、K 各自从 N−1−i 到 i）。所以最终 φ 依赖任务划分（c7 和 2.2.3 的分区算法），
不同 cores 设置的结果会不同。新内核若要逐位复现必须照抄这个行为（并照抄分区算法）；若要修正，应改为累加并告知用户结果会变化。
每个满足条件的镜像点都要遍历全网格，最坏 O(N⁶)。

#### 2.7.6 `autoPIC`（是否重算）

```
prev = PIC+0x208（3 个 double，初值 0）
若 prev[0] != 0:
    d = |(prev[2] − rms[2]) / prev[2]|           ← 只有 z 分量（x、y 两项先后赋给同一变量又被覆盖，推测本意为三项平方和）
    若 d < 1e-8: PIC+0x168 ++，返回 0（本步不重算，粒子保留旧 scE）
prev = rms[0..2]；PIC+0x168 ++；PIC+0x16c ++；返回 1
```

只在 SPICNIC 时调用。FFT 每步都重算。

### 2.8 场插值与实验室系变换（`PIC::Allocate` / `Calculate_Allocate`）

1. Q = Σ grid[0]（按 k 外层、j、i 内层的顺序累加，写 PIC+0x1b0）。
2. 对每个粒子 n = 0..N−1：
   - lost == 1：scE = scB = 0。
   - Particle+0xf8 == 0（在网格内）：用 Weight 存下的 w[8] 和 cellOf，
     ```
     Ez_r = Ez[i][j][k]·w0 + Ez[i+1][j][k]·w1 + Ez[i][j+1][k]·w2 + Ez[i+1][j+1][k]·w3
          + Ez[i][j][k+1]·w4 + Ez[i+1][j][k+1]·w5 + Ez[i][j+1][k+1]·w6 + Ez[i+1][j+1][k+1]·w7     （按此顺序左结合）
     Ey、Ex 同理
     scE = (Ex_r/γ, Ey_r/γ, Ez_r)     （乘 PIC+0x2a8 = 1/γ），scB = 0
     ```
     Beam.rotMode（+0xa8）≠ 0 时旋转（θ = Beam.rotAngle，s = sinθ、c = cosθ）：
     rotMode == 1：Ey' = Ey·c − Ez·s，Ez' = Ey·s + Ez·c；其他非零值：Ex' = Ex·c + Ez·s，Ez' = Ez·c − Ex·s。
   - Particle+0xf8 == 1（网格外）：按总电荷 Q 位于中心的点电荷：
     ```
     d = (rel[0] − c0, rel[2] − c2, rel[4] − c4)        ← 纵向用实验室系距离（未乘 γ）
     r = sqrt(dx² + dy² + dz²)
     f = (((Q·8987551787.368176)/r)/r)/r · (1/γ)
     scE = (dx·f, dy·f, f·dz·γ)，scB = 0；不做 rotMode 旋转
     ```
     Q 不含网格外粒子的电荷。`Calculate_Allocate` 多一个参数 f2：f2 == 1 时 d 直接取 rel（不减中心），来源同样被截断（推测为 lpBoundary）。
3. 物理含义：静止系电场 E_r 换到实验室系后横向 E⊥ = γE_r⊥、B = β×E/c，对以 β 运动的粒子横向力 q(E − v×B) = qE_r⊥/γ；
   内核直接把 E_r⊥/γ 当作电场、磁场置 0，二者对推进等价（只要粒子速度≈束流速度）。
4. 多线程（+0x1e4 == 1）：c9 个任务各处理 +0x368 的一段，每个粒子独立计算，**结果与串行逐位相同**（除 f2 分支外）。
5. 最后若 Beam.recScField == 1：`recordSCF`。

### 2.9 空间电荷场的保存与读入

#### 2.9.1 `recordSCF`（每次 Allocate 后调用）

1. `outputFilePath + "SpaceChargeSet.plt"`，ofstream 模式 0x15 = out|binary|**app**（追加，旧运行留下的记录不会被清掉）。打不开时 stdout 打印
   "SpaceChargeSet.plt cannot be output             "（含尾随空格，共 0x30 字符），不中止，仍尝试写。
   写入：k 外层、j、i 内层，每个节点依次 `write` 8 字节原生 double：Ex_r、Ey_r、Ez_r（**静止系**，未除 γ）。一条记录 Nx·Ny·Nz·3·8 字节。
2. `outputFilePath + "SpaceChargeMesh.txt"`，fstream 模式 0x11 = out|app（文本追加），写一行（ostream 默认格式，6 位有效数字，字段间单个空格，`endl` 结尾）：
   ```
   Beam.time  syncCoord[4]+centroid[4]  syncCoord[0]+centroid[0]  syncCoord[2]+centroid[2]  Nx  Ny  Nz  dx  dy  dz  Q  γ
   ```
   （中心的绝对坐标 = 同步粒子坐标 + 相对中心；dz 为静止系格距。）
   z 段中 syncCoord 不更新，仍是最后一个 t-code 步保存的同步粒子坐标（DLL 实测），而相对中心来自 HESCFieldPre。
   精度只有 6 位，读回时会损失精度。

#### 2.9.2 读入索引（`PIC::Initial`，impScField == 1）

逐行读 `inputFilePath + "SpaceChargeMesh.txt"`（用 `StringSplit` 拆分、`stod`）：列 0 → times（+0x440），列 1 → zc（+0x458），列 7、8、9 → {dx,dy,dz}（+0x470），
列 10 → Q（+0x488），列 11 → γ（+0x4b8），列 2、3 → {xc,yc}（+0x4a0），列 4、5、6 → 覆盖 PIC 的 Nx、Ny、Nz（以最后一行为准，假定各行相同）。
FFT 缓冲和核表已按 input.txt 的 numofgrid 分配，导入模式不再使用它们。

#### 2.9.3 `importSCF(Beam&)`（代替 Weight/求解/Allocate）

```
t = Beam.time
若 t < times[0]：所有粒子 scE = scB = 0，返回
T = 1/Beam.frequency
while (t − times[0] > T): t −= T，并把 Beam.time 改写为 t       ← 副作用：模拟时间被折回第一个周期
查询时刻 q_m = t + m·T（m = 0,1,…，只取 < times.back() 的；在 double 上逐次加 T）
monotonicSearch(times, q, res)：对每个 q 从上次位置向后找 times[a] < q ≤ times[a+1]，得 {a, a+1, times[a], times[a+1]}；找不到为 {−1,−1,0,0}
zq_m = zc[a] + (zc[a+1] − zc[a])·((q − t_a)/(t_b − t_a))
r_m  = (q − t_a ≤ t_b − q) ? a : a+1                         （取较近的记录）
打开 inputFilePath + "SpaceChargeSet.plt"（BinaryFileReader）
对每个粒子（跳过 lost == 1）：
    (m, zq) = findClosest(zq[], Particle.z)     （二分 lower_bound，再比较相邻两项距离，距离相等取前一项；数组为空抛 runtime_error "Vector is empty!"）
    r = r_m；x0 = xc[r] − (Nx·0.5 − 0.5)·dx[r]；y0 同理；z0 = zq − (Nz·0.5 − 0.5)·dz[r]
    zr = (Particle.z − zq)·γ[r] + zq
    i = floor((Particle.x − x0)/dx[r]) …（用**绝对**坐标 x、y、z，而不是 rel）
    网格内：CIC 权重同 2.5（V = dz·dx·dy），8 个节点各 seekg 到
            ((Nx·j + k·Nx·Ny + i)·3 + Nx·Ny·Nz·3·r)·8 读 24 字节 (Ex,Ey,Ez)；
            seekg 失败 stderr "Localization failed: Location <偏移>"，read 失败 stderr "Read failed: Location <偏移>"，继续
            scE = (ΣwEx/γ[r], ΣwEy/γ[r], ΣwEz)，scB = 0
    网格外：d = (x − xc[r], y − yc[r], (zr − zq))（**纵向是拉伸后的**，与 Allocate 不同）
            f = ((((Q[r]·k_e)/r)/r)/r)/γ[r]；scE = (dx·f, dy·f, f·dz·γ[r])
```

边界情况：t 恰等于 times[0] 时 monotonicSearch 找不到区间（条件是严格 `<`），下标 −1 越界读；t ≥ times.back() 时查询列表为空，findClosest 抛异常。
用途（推测）：把一次运行记录的单束团场按 RF 周期 T 平移，给后续束团（或同一束团在后续周期）施加，模拟多束团的场。

### 2.10 多线程实现与求和顺序

| 步骤 | 并行方式 | 对结果的影响 |
|---|---|---|
| Weight 串行（+0x1e4 ≠ 1 或 c4 == 1） | 无 | 按粒子顺序累加，确定 |
| Weight 并行 | 线程池 c4 个任务（每任务固定粒子段）+ OpenMP c5 归约（每元素按 t 升序相加） | 与串行有舍入级差异；同一 c4 与分区下确定。weight 计数被截断为 int（只影响 +0x280，无人使用） |
| FFT | 单线程 FFTW（FFTW_MEASURE 规划） | MEASURE 可能每次选不同算法 → 1e-16 量级的运行间差异；求梯度并行但逐点独立 |
| SPICNIC 制表、卷积 | 线程池；每个点独立计算，求和顺序固定 | 确定；**但非对称修正的"最后一个赢"依赖任务划分**（2.7.5） |
| SPICNIC 归约 | OpenMP，按 t 升序 | 确定 |
| Allocate | 线程池，逐粒子独立 | 与串行逐位相同 |
| HESCFieldPre | OpenMP，原子区合并、rms 区有数据竞争 | 多线程时非确定 |
| Q（总电荷） | 串行 | 确定 |

`Calculate_*` 都是线程池任务函数，各带 [起点, 终点) 和任务号参数；`PIC::*._omp_fn.N` 是 OpenMP 并行区（静态均分：每线程 ⌊n/T⌋ 个，前 n mod T 个线程多 1 个）。

### 2.11 未被调用的函数（旧 PICNIC 路径与调试代码）

`.so` 中没有调用者，或只在 PIC+0x170 == 0 时调用（构造函数写 1.0 且没有任何地方改写，所以永远不执行）：

- PIC+0x170 == 0 时 `PIC::Initial` 会改 Mx = Nx−1 等、积分细分 15、R2 = ceil(0.343·Nx·Ny·Nz)；`PIC::PICNIC` 走 `Tabulation` + `._omp_fn.5`：
  `Tabulation`（omp_fn.2，Integrate_3D_Vec）制 E 核表 Gx、Gy、Gz（+0x2e8/0x300/0x318）：远场 r⃗/|r|³（pow(r²,1.5)），近场单元平均（细分中点和 / V），
  按奇偶性镜像写入（x 分量关于 x 反号等）；`._omp_fn.5` 对每个节点调 `Lattices2Node` 直接求和 E = k_e·Σρ·G⃗ 写 grid[2..4]。这是"非对称"的原始 PICNIC（直接对网格单元求库仑和）。
- `Lattices2NodeNew`：对称版本（镜像电荷 ρ[N−1−i] 等、阈值 +0x230）的单节点 E 求和。
- `Lattices2NodePhi` / `Lattices2NodeNewPhi`：同上求 φ（用 +0x330 表；前者用 |偏移| 取表，后者对称镜像）。
- `CLattices2Node`：z 方向周期折回的 `Lattices2Node`，每次内循环向 cout 打印 k（调试残留）。
- `TabulationPhi`（及 omp_fn.3）：同时制 E 核和 φ 核。
- `CalculateVar`：沿三轴（过中心）求 ⟨|ρ[c+m] − ρ[c−1−m]| / ρ[c]⟩（m < N/3，取整），是对称性诊断量，返回 vector(3)。
- `TextVol`：把 E 核表项乘 0.0 累加 +0x238 次（+0x238 = 0），恒返回 (0,0,0)，测试桩。
- `Mesh::MeshGenerator`：空函数。

新内核不需要实现这些函数（`CalculateVar` 若想做诊断可以保留）。

### 2.12 标准输出

`PIC::Initial` 两行（2.2.3 第 5、6 步）；`Initial::InitParticle` 的 "0 intensity, stop SCfield calculations"；`Initial::Read` 的 "Simulation ignores space charge effects"；
`recordSCF` 失败提示；`importSCF` 的 stderr 提示。Python 端只解析进度行，这些行不影响结果文件。

---

## 3. 对齐清单

| ID | 行为 | 涉及关键字/文件 | 建议的比对测试 |
|---|---|---|---|
| F-01 | scmethod 只接受 FFT / SPICNIC，PICNIC 等抛错 | input.txt scmethod | 分别写 FFT、SPICNIC、PICNIC、fft：前两者正常，后两者报 "SCMethod Contains unknown item…"（若小写也报错则确认区分大小写） |
| F-02 | 不写 scmethod 时默认 SPICNIC | input.txt | 删去 scmethod 与写 SPICNIC 各跑一次（固定 randomseed、相同 cores），DataSet.txt 逐位相同 |
| F-03 | FFT 默认 meshrms 6.5、网格 64³ | numofgrid、meshrms | FFT 下不写与显式写 `numofgrid 64 64 64`、`meshrms 6.5 6.5 6.5` 比较，逐位相同；stdout 第一行打印 64 64 64 … 6.5 6.5 6.5 |
| F-04 | SPICNIC 默认 meshrms、网格按粒子数分档 | particlenumber | N = 1e4、5e4、2e5 各跑一次，读 stdout 第一行，应为 14/3.5、18/3.5、20/4.0 |
| F-05 | 默认 cores 按粒子数取表并截断到 omp 最大线程数 | setnumberofcores、maxthreads | 读 stdout "mulitthread:" 行，与 2.1.3 表比较（注意 c4/c6/c7/c9 是段数） |
| F-06 | 流强 0 或单粒子关闭空间电荷 | current、particlenumber | current 0：stdout 有 "0 intensity…"；与 spacecharge 0 的 DataSet.txt 逐位相同 |
| F-07 | 网格以束流中心为中心、节点覆盖 ±meshrms·rms，静止系 z 拉伸 γ | meshrms | 单元测试：给定 rel、centroid、rms、γ，比较 xs/ys/zs、dx/dy/dz（容差 1e-15 相对） |
| F-08 | CIC 权重与分配顺序 | — | 单元测试：随机 1e4 个粒子，比较 grid[0]（与原内核导出无法直接取得时，用 recordSCF 的 E 场间接比较） |
| F-09 | 网格外粒子用点电荷场，Particle+0xf8 标志 | meshrms | meshrms 1.0、FFT、1e4 粒子，recscfield 1；新旧内核比较一步后外围粒子的 x' 变化，容差 1e-10 相对 |
| F-10 | FFT 求解：DST-I 横向 + 周期 FFT 纵向，归一化 1/(2(Nx+1)·2(Ny+1)·Nz) | scmethod FFT | recscfield 1 跑 1 步，比较 SpaceChargeSet.plt 第一条记录（相对误差 < 1e-12，FFTW 算法差异） |
| F-11 | 中心差分求 E，横向 φ 边界外为 0、纵向周期 | — | 同 F-10，检查边界节点的场 |
| F-12 | 静止系 → 实验室：E⊥/γ、Ez 不变、B = 0 | — | 单元测试 Allocate；整机：对比 FFT 运行的 DataSet.txt rms（容差 1e-6 相对，固定种子） |
| F-13 | rotMode 1/2 时旋转 SC 场 | lattice bend | 含 bend 的 lattice，FFT，比较 DataSet.txt（容差 1e-6） |
| F-14 | SPICNIC 格林函数制表（近场 10³ 中点积分 / V、远场 1/r、截断 0.75Nx） | integralgrid（无效） | 单元测试制表；改 integralgrid 值验证结果不变 |
| F-15 | SPICNIC 对称化：只用上卦限电荷，结果镜像 | scmethod SPICNIC | 构造关于中心不对称的初始分布（displacepos 使一半偏移），recscfield 1，比较 SpaceChargeSet.plt 的 φ 导出场是否镜像对称 |
| F-16 | SPICNIC 非对称修正"最后一个赢"，结果依赖 c7 | setnumberofcores | 同一输入只改 setnumberofcores 的第 8 个数（c7 = 1、4、12），比较 SpaceChargeSet.plt：若不同则证实；新内核用相同 c7 应逐位复现 |
| F-17 | negligibleratio 阈值（中心格点电荷 × 比例，严格大于） | negligibleratio | 0、0.09、0.5 三次，比较 SpaceChargeSet.plt |
| F-18 | 奇数网格 SPICNIC 死循环 | numofgrid | `numofgrid 15 16 16`、SPICNIC，设超时 60 s，观察是否挂起；FFT 下同网格正常 |
| F-19 | Ny ≠ Nz 时 SPICNIC 的 j 起点用 Nz/2 | numofgrid | `numofgrid 20 20 30` 与 `20 30 20`，检查 SpaceChargeSet.plt 中是否有未更新（保持上一步）的节点 |
| F-20 | autoPIC 只比较 rms_z 的相对变化（< 1e-8 跳过） | SPICNIC | 单元测试；整机难以触发（可用零长度漂移/冻结束流验证 scE 不更新） |
| F-21 | 跳过求解时粒子保留旧 scE | scmethod | 单元测试（scMethod 非 0/1 无法从输入得到，只能内部测） |
| F-22 | lpboundary 1：dz = βγλ/(Nz−1)、网格以同步粒子为原点、并**不**强制 FFT | lpboundary | lpboundary 1 + 默认 scmethod 与 + FFT 各跑一次，结果应不同（证实未强制）；recscfield 1 检查 SpaceChargeMesh.txt 的 dz 列 = βγλ/(Nz−1) |
| F-23 | recordSCF 文件格式：二进制追加、k-j-i 顺序、静止系场；文本行 12 列 6 位有效数字 | recscfield | 跑两次不清 OutputFile，检查两个文件都被追加；按格式解析 |
| F-24 | importSCF 时间折叠（改写 Beam.time）、按最近中心 z 选记录、点电荷用拉伸 z | impscfield | 用 F-23 的输出放入 InputFile 再运行 impscfield 1，与新内核比较 DataSet.txt（容差 1e-9） |
| F-25 | t < times[0] 时 SC 场为 0 | impscfield | 手改 SpaceChargeMesh.txt 第一行时间为很大值，结果应与 spacecharge 0 相同 |
| F-26 | z-code 的 HESCFieldPre：同一时刻换算、不加权中心与 rms、不更新 γ | 矩阵元件（quad 等） | 只含四极与漂移的 lattice、FFT、单线程（cores 全 1），比较 DataSet.txt（容差 1e-9） |
| F-27 | 多线程 Weight 与串行的差异 | setnumberofcores c4 | c4 = 1 与 c4 = 26 比较 DataSet.txt：差异应为 1e-12 量级；同一 c4 两次运行逐位相同（FFT 除 FFTW 差异外） |
| F-28 | spacechargecomp（仅 DLL）把电荷乘 (1 − V1)，V1 ≥ 0.999999999 不缩放 | lattice spacechargecomp | Windows DLL：V1 = 0、0.5、0.999、1.0 比较 rms 增长；.so 应报未知关键字 |
| F-29 | spacechargelong / spacechargetype 内核不认识 | input.txt | 写入后观察内核报错或忽略（与输入子系统文档一致） |
| F-30 | FFT 归一化分母按 int 相乘可能溢出 | numofgrid | 只做单元测试（大网格内存不现实）；新内核注明并改用 64 位 |

---

## 4. 函数覆盖表

| 函数 | 作用 | 正文 |
|---|---|---|
| Mesh::Mesh | 全部成员清零 | 2.2.1 |
| Mesh::~Mesh | 释放 8 个三维数组和坐标、cellOf（编译器生成式的析构） | 2.2.1 |
| Mesh::MeshGenerator | 空函数，返回 0 | 2.2.1、2.11 |
| Mesh::Initial | 按 numofgrid 与粒子数分配网格数组 | 2.2.1 |
| MethodMP::HESCFieldPre（00193a50，omp_fn.16） | z-code：累加相对坐标二阶矩（无原子保护） | 2.3 |
| MethodMP::HESCFieldPre（00193b80，omp_fn.15） | z-code：换算到同一时刻的 rel 坐标并累加一阶矩 | 2.3 |
| MethodMP::HESCFieldPre（001a20a0） | z-code：调度上面两个并行区，写 Beam 中心与 rms | 2.3 |
| Calculate_S_PICNIC | SPICNIC 对称直接卷积求 φ（含有缺陷的非对称修正） | 2.7.3–2.7.5 |
| Calculate_Weight | Weight 的线程池任务：一段粒子的 CIC 分配到局部网格 | 2.5 |
| Calculate_Allocate | Allocate 的线程池任务：一段粒子的场插值 | 2.8 |
| PIC::FFT（001be410，omp_fn.8） | 由 φ 求 E（横向狄利克雷、纵向周期中心差分） | 2.6 |
| PIC::Weight（001be8d0，omp_fn.0） | 归约各任务局部电荷网格 | 2.5 |
| PIC::PICNIC（001bea90，omp_fn.7） | SPICNIC 由 φ 求 E（边界单侧差分） | 2.7.1 |
| PIC::PICNIC（001bef60，omp_fn.6） | 把各任务修正数组加到 φ | 2.7.1 |
| PIC::autoPIC | 判断 rms_z 是否变化，决定 SPICNIC 是否重算 | 2.7.6 |
| PIC::Lattices2NodeNew | 旧路径：对称镜像的单节点 E 直接求和（未调用） | 2.11 |
| PIC::Lattices2NodeNewPhi | 旧路径：对称镜像的单节点 φ 求和（未调用） | 2.11 |
| PIC::Lattices2Node | 旧路径：单节点 E 直接求和（只被死路径调用） | 2.11 |
| PIC::PICNIC（001c0710，omp_fn.5） | 旧路径：逐节点调 Lattices2Node（死路径） | 2.11 |
| PIC::Lattices2NodePhi | 旧路径：单节点 φ 求和（未调用） | 2.11 |
| PIC::CLattices2Node | 旧路径：z 周期折回的 E 求和，带调试打印（未调用） | 2.11 |
| PIC::CalculateVar | 电荷分布对称性诊断（未调用） | 2.11 |
| PIC::TextVol | 恒返回 0 的测试桩（未调用） | 2.11 |
| Integrate_3D_Phi（isra.156） | 长方体上 1/r 的中点规则积分 | 2.7.2 |
| Calculate_Tabulation | SPICNIC 每步制 φ 核表（线程池任务） | 2.7.2 |
| PIC::FFT（001c1c00） | FFT 泊松求解主函数 | 2.6 |
| Integrate_3D_Vec（isra.227） | 长方体上 r⃗/r³ 的中点规则积分（旧路径） | 2.11 |
| PIC::Tabulation（001c2d00，omp_fn.2） | 旧路径：制 E 核表 | 2.11 |
| PIC::Tabulation（001c3190） | 旧路径：按 i 循环调 omp_fn.2（死路径） | 2.11 |
| PIC::TabulationPhi（001c31f0，omp_fn.3） | 旧路径：同时制 E、φ 核表（未调用） | 2.11 |
| PIC::TabulationPhi（001c3720） | 旧路径：调度 omp_fn.3（未调用） | 2.11 |
| PIC::recordSCF | 追加写 SpaceChargeSet.plt（二进制）与 SpaceChargeMesh.txt | 2.9.1 |
| PIC::importSCF | 从记录文件插值空间电荷场 | 2.9.3 |
| PIC::~PIC | 释放 FFTW 缓冲与计划、各容器、线程池 | 2.2.2 |
| PIC::PIC()（001c5a70） | 无参构造，只初始化成员（无调用者） | 2.2.2 |
| PIC::Initial | 线程分区、FFTW 计划、核表分配、读 SpaceChargeMesh.txt | 2.2.3、2.9.2 |
| PIC::PIC(Mesh&,Beam&,vector&,int)（001c7980） | 构造并调用 Initial | 2.2.2 |
| PIC::Weight（001c8020） | 建网格、CIC 电荷分配（串行或线程池） | 2.4、2.5 |
| PIC::PICNIC（001c9750） | SPICNIC 主函数 | 2.7.1 |
| PIC::Allocate | 场插值回粒子、静止系→实验室系、网格外点电荷、调 recordSCF | 2.8 |

共 40 个，全部列出。

---

## 5. 未解决的问题

1. **`Calculate_Weight` 的标志 f 与 `Calculate_Allocate` 的 f2 来源**：线程池 `enqueue` 的实参被 Ghidra 截断，推测为 PIC+0x1e8（lpBoundary）。
   实验：lpboundary 1、FFT、c4 = 1（串行）与 c4 = 26（并行）各跑一次，若结果差异远大于舍入级，说明并行路径用了 `zr = rel_z·γ + c_z`。也可在 `libAVAS.asm` 里读 `enqueue` 模板实例的参数打包代码确认。
2. **奇数网格死循环**（2.7.4）是从循环步长推出来的，需要实际运行确认（F-18）。
3. **非对称修正的赋值缺陷**（2.7.5）：F-16 改 c7 比较 SpaceChargeSet.plt。若结果与 c7 无关，说明我们对参数映射的理解有误。
4. **j 起点 Nz/2**（2.7.3）：Ghidra 的双精度参数顺序可能有误，用 F-19 确认。
5. ~~Beam.gamma 在 z-code 中的取值~~：已核实（M4-notes 第 2 节）：z 段沿用上一次 t-code 的 γ，DLL 的 SpaceChargeMesh.txt 记录与此一致。
6. **HESCFieldPre 的 coords[6]、tRef 的确切含义**（推测为到达时间与参考时间），以及 z-code 冲量中 coords[4] 的意义，归推进/z-code 文档核实。
7. **spacechargecomp 的触发位置与 DLL Beam 成员初值**：需细读 DLL tcodeRun（FUN_180070b90）中 `uVar30 + lVar24*8` 所指的 z 列表和 DLL `Beam` 构造。
   实验：Windows 上 V1 = 0.5 放在束线中段，比较前后两段 rms 增长速率。
8. **importSCF 的物理用途**（多束团？）与 `monotonicSearch` 在 t == times[0] 时的越界：需要用户或原作者确认，实验 F-24/F-25。
9. **FFTW_MEASURE 的运行间差异**：同一输入跑 5 次 FFT，比较 DataSet.txt 是否逐位一致；若不一致，比对容差需放到 1e-12 相对。
10. **PIC::Initial stdout 第一行的分隔符**（推测单个空格），直接运行看输出即可。
11. **UpdateBeamParameters 的 rms 循环**看起来用绝对坐标减相对中心（伪代码 `pdVar15[0..5]`），若属实 rms 会错得离谱，显然不是；需由循环/输出文档核实 Beam.rms 的确切算法，因为网格尺寸完全依赖它。

---

## 6. 对结构表的更正

| 位置 | 原表 | 更正 | 证据 |
|---|---|---|---|
| Particle+0xf8 | unk_0f8，"未见任何访问" | int **outOfMesh**：最近一次 Weight 中粒子落在网格外为 1，网格内为 0 | `PIC::Weight`、`Calculate_Weight` 写；`PIC::Allocate`、`Calculate_Allocate` 读 |
| PIC sizeof | ≥ 0x530 | ≥ 0x540 | 构造函数写 +0x530..0x53c（FFTW kind 数组） |
| PIC+0x1e4 | "= SC 参数[9]" | = scPar[9] = input.txt `multithreading`（由 `InitParticle` 拷入）；1 时 Weight/Allocate 走线程池 | `Initial::InitParticle`：`pdVar21[9] = (double)*(int*)(this+0x1e8)` |
| PIC+0x1e8 | 未列 | int lpBoundary（构造参数，= MethodMP+0x1d0） | `MethodMP::Run`：`PIC::PIC(…, *(int*)(this+0x1d0))` |
| PIC+0x258 | "非 0 = SPICNIC" | 0 = FFT、1 = SPICNIC，其他值不求解；默认 1 | `tcodeRun`、`zcodeRun` 的 `== 0.0` / `== 1.0` 判断；scPar 默认值 |
| PIC+0x2b0 | "vector\<int\>(3) 网格数" | 积分细分数，恒为 (10,10,10) | `PIC::Initial` 覆盖写 10；`Calculate_Tabulation` 传给 `Integrate_3D_Phi` |
| PIC 其他成员 | 未列 | 0x168/0x16c autoPIC 调用/重算计数；0x170 double 新旧 PICNIC 选择（恒 1.0）；0x178–0x1a8 各阶段耗时累计（µs）；0x1b0 网格总电荷 Q；0x1f0 vector\<int\>(10) cores；0x208 vector\<double\>(3) 上次 rms；0x220 autoPIC 相对变化；0x228 negligibleratio；0x230 SPICNIC 电荷阈值；0x238 int（TextVol 次数，0）；0x260/0x268/0x270 dx/dy/dz；0x278 单元体积；0x280 网格内 weight 和；0x288 vector\<vector\<double\>\> 每粒子 8 个 CIC 权重；0x2a0 γ；0x2a8 1/γ；0x2c8 近场半径平方；0x2cc–0x2d4 核半宽 Mx/My/Mz；0x2d8–0x2e0 未用；0x2e8/0x300/0x318 E 核表；0x330 φ 核表；0x348 求解调用计数；0x350/0x368 粒子分区；0x380–0x3c8 SPICNIC 分区；0x3e0–0x428 制表分区；0x440 times、0x458 zc、0x470 {dx,dy,dz}、0x488 Q、0x4a0 {xc,yc}、0x4b8 γ（导入用）；0x510 r2r 正、0x518 r2r 逆、0x520 dft 正、0x528 dft 逆；0x530、0x538 r2r kind {7,7} | 见 2.2、2.7、2.9 |
| Mesh 8 个三维数组 | 含义未确认 | grid[0] 节点电荷（C）、grid[1] φ、grid[2..4] 静止系 Ex/Ey/Ez；grid[5..7] 本子系统未用 | `PIC::Weight` 写 +0x80；`PIC::FFT` 写 +0x98、+0xb0/+0xc8/+0xe0；`Allocate` 读 |
| Initial+0x118 scPar 默认值 | 只列了含义 | 默认 {1(SPICNIC), −1×7, 0.09, 1, 1×10}；[4..6] integralgrid 与 [7] scutoffdistance 在 .so 中无效 | `.rodata` 0xd9e80；`PIC::Initial` |
| Initial+0x1c8 numOfGrid、+0x88 cores 默认值 | 未列 | numOfGrid 默认 {−1,−1,−1}；cores 默认 {−1,1,…,1}，cores[0] == −1 且 multithreading == 1 时按粒子数取表 | `.rodata` 0xd9e28、0xd9e40；`InitParticle` |
| Initial+0x1e8 multithreading | "默认 1" | 正确；补充：它同时控制空间电荷的线程池（经 scPar[9]） | 同上 |
| Beam centroid/rms 的来源 | `UpdateBeamParameters` | z-code 中由 `HESCFieldPre` 重写（不加权、按个数平均） | `MethodMP::HESCFieldPre` |
