# 40 主循环、粒子推进、损失与输出（MethodMP::Run / tcodeRun / zcodeRun 及输出函数）

依据：`libAVAS.so`（GCC 4.8.5）伪代码 `decomp/libAVAS/functions/*.c`，并直接反汇编同一个 `.so`（`objdump`）补回 Ghidra 丢掉的
字符串字面量、`write` 的字节数、浮点常数。字段名按 [structures.md](structures.md)。Windows `AVAS.dll` 只抽查了 DataSet.txt
（同样 41 列）与 synParticle.txt（同样格式）。

本文件只写本子系统的规格；单粒子推进的逐粒子细节（`TcodeTrans`）属 B，元件场（`*::GetField`）与矩阵传输（`HardEdgeTrans`）属 C，
空间电荷（`PIC::*`、`HESCFieldPre`）属 F，误差（`errSampleOut`、`SetErrInEle`）与二次粒子（`SeRun`…）属 G，lattice 解析属 A。
这里只写到"在何处、以什么参数、什么顺序调用它们"。

---

## 1. 概述

### 1.1 位置与入口

`main_agent` 构造 `MethodMP`（构造函数启动线程池）→ `Initial::InitMethodMP` 拷贝参数 → 若 `secondarybeam == 0` 调
`MethodMP::Run(lattice, beam, errSample, boundary, runFlag)`，否则 `SeRun`（G）。`runFlag` 是 `main_agent` 里的局部 `int`，
赋值 2 后不再改变（因此 tcodeRun 中 `"Programs terminated by users."` 分支在本 .so 中不可达）。

```
Run
 ├─ Mesh::Mesh / Mesh::Initial(numOfGrid, N) / PIC::PIC(mesh, beam, scPar, lpBoundary)        (F)
 ├─ errSampleOut(err)；err.flags[2]==1 或 [3]==1 时 Lattice::SetErrInEle               (G)
 ├─ 打印、截断全部输出文件、可选 BeamSet.plt 头
 ├─ 把元件序列切成交替的 t-code 段（场图等）与 z-code 段（quad/solenoid/bend/edge）
 └─ 依次：t 段 → tcodeRun(...)；z 段 → zcodeRun(...)
       tcodeRun：时间步循环（每步：空间电荷 → 线程池并行推进宏粒子 → 统计 → DataSet 行 → 推同步粒子 → 各种输出）
                 遇到 steerer 事件时转 z 表示→施加校正铁冲量→zTotcode→递归 tcodeRun
       zcodeRun：逐元件矩阵传输（HardEdgeTrans），可在元件中点做空间电荷与输出；结束时 zTotcode 转回 t 表示
 └─ 打印完成信息，追加 SimulationTime.txt
```

### 1.2 两种表示

- **t 表示**（tcodeRun）：所有宏粒子处于同一时刻 `t`，各自坐标 `Particle.{x,γβx,y,γβy,z,γβz}`（m、无量纲）。
- **z 表示**（zcodeRun 之间）：每个粒子在某个平面 `z = z_plane` 上的穿越记录 `rec[i] = {x, γβx, y, γβy, z, γβz, t_cross, flag}`
  （8 个 double，flag 1 = 已穿越）。这个数组是 Run 里分配的 `(N+1)×8` 的 `planeRec`（Run 的 `local_2028`），t 段里由 `TcodeTrans` 在粒子
  穿越"事件平面"时线性插值写入，z 段里由 `HardEdgeTrans` 直接变换。
- **z→t**：`zTotcode`（§4.3）。**t→z**：t 段末事件平面上的插值记录本身就是 z 表示。

---

## 2. 常数（内核实际使用的值）

| 符号 | 数值 | 用处 |
|---|---|---|
| c | 299792458.0 | 全部 |
| c² | 8.987551787368176e16 | 电校正铁冲量 |
| e | 1.60217653e-19 | collisionData、.edst |
| m_p | 1.67262171e-27 | .edst |
| m_p c² | 938.271875 MeV | singlePOut 能量、.edst 质量（写死，不是 beam.txt 的静止能量） |
| π | 3.141592653589793 | dstOut 相位 |
| π/2, cos(π/2) | 1.5707963267948966, 6.123233995736766e-17 | rotMode 角度 |
| 哨兵 | 1e10 / −1e10（`0x4202a05f20000000`/`0xc202…`） | zMin/zMax 初值 |
| exOutput 哨兵 | ±1e9 | 直方图范围初值 |
| 进度间隔 | 3.0 s（`MethodMP+0x288`，构造函数写入） | simulationProgress |
| 停滞因子 | 0.8 | 停滞粒子判丢 |
| 小角阈值 | 3e-4 | rotMode 角度 |

时间步 `dt = (1/f)/V1`（`InitMethodMP`：`MethodMP.dt = (1.0/frequency_Hz)/steppercycle_V1`，先除 f 再除 V1）。

---

## 3. MethodMP::Run

### 3.1 初始化与文件截断

1. `t_start = system_clock::now()`（ns）。构造 Mesh、PIC（F）。
2. 分配 `planeRec`、`spRec`：各 `N+1` 个 `vector<double>(8)`，全 0。
3. `errSampleOut(err)`；`err.flags[2]==1 || err.flags[3]==1` → `Lattice::SetErrInEle(err)`；四个 flag 全 0 → `*(int*)(err+0x48) = 1`。
4. 打印（`std::cout`，默认格式）：
   - `"The total distance simulated is " << lattice.elemZEnd.back() << " (m)"`
   - `"Simulation time step is " << dt << " (m)"`（单位标签写错，值是秒）
   - `"Start simulation, start time " << strftime("%Y/%m/%d %X  ")`
5. 用 `ofstream(outputFilePath + name)` 打开并立即关闭（=截断为空）：`DataSet.txt, synParticle.txt, test.txt, testP.txt,
   collisionData.txt, LongitudinalBeamLoss.txt, BeamSet.plt, ExData.edt, FullParticle.txt, ParticleTracking.txt`；
   `beam.recScField == 1` 时再截断 `SpaceChargeMesh.txt, SpaceChargeSet.plt`。
   **不截断**：`PCHistogram.dat`、`SingleParticle.txt`、`SimulationTime.txt`、`ErrorLog.txt`（这些只追加）。
6. `dumpPeriodicity != 0`：以 `out|binary`（截断）写 BeamSet.plt 头，失败抛 `"BeamSet.plt open failed, Check permissions"`。
   头格式（小端，无填充）：`char 0x7D, char 0x64, int32 dumpPeriodicity, int32 N, double I_mA (=beam.current×1000),
   double f_MHz (=beam.frequency/1e6), double mc²_MeV (=particles[N].restMassMeV)`。

### 3.2 元件序列分段（关键字比较用 `Element.typeName`）

z 型元件 = `"quad" | "solenoid" | "bend" | "edge"`；`"steerer"` 特殊处理；其余（drift、field、rfq…）是 t 型。
设 `i` 为当前元件序号，`flag` 表示"刚跳过一个 steerer"（跨段保留）。输出面列表 `P = MethodMP.outputPlanes`（§3.5）。

```
while i < nElem:
  if typeName(i) 是 z 型: 开 z 段（segType 2）:
     elems=[], marks=[]
     loop:
       if typeName(i) 不是 z 型: 结束本段
       elems.push(i)
       if flag: marks.push(i-1)   # steerer 的序号；flag=false；本元件不再检查输出面
       elif P 非空:
         for p in P（按顺序）:
            if p == zEnd(i): marks.push(-2); break
            if zStart(i) < p < zEnd(i): marks.push(-1); break
       if marks.size < elems.size: marks.push(-3)
       i += 1; if i >= nElem: 结束本段
       if typeName(i)=="steerer": i += 1; flag=true
  else: 开 t 段（segType 1）:
     ev=[], marks=[]
     loop:
       if typeName(i) 是 z 型: 结束本段
       if flag: ev.push(zStart(i)+0.5*L(i)); marks.push(i-1); flag=false
       for p in P: if zStart(i) < p <= zEnd(i):
            if ev 为空 或 p > ev.back(): ev.push(p); marks.push(-1)
       i += 1; if i >= nElem: 结束本段
       if typeName(i)=="steerer": i += 1; flag=true
     zE = zStart(i-1)+L(i-1)
     if ev 为空 或 zE != ev.back(): ev.push(zE); marks.push(-2)
```

markers 含义：`>=0` steerer 元件序号（冲量施加在其后一个元件的中点）；`-1` 输出面；`-2` 段末（z 段为元件出口处有输出面）；`-3` 无。
边界情况（照原样实现）：
- t 段里同一元件内多个输出面都保留（严格递增）；z 段里一个元件最多一个输出，且输出发生在**元件中点**而不是面的精确位置。
- steerer 之后那个元件里、位于其中点之前的输出面被丢弃（t 段要求 `p > ev.back()`）；z 段里 steerer 之后那个元件的输出面全部被忽略。
- 两个相邻 steerer：第二个被当作普通 t 元件处理，冲量只来自第一个。steerer 是第一个元件时冲量永远不施加。lattice 以 steerer 结尾时越界读取（未定义）。
- 输出面恰在 t 段末端时段末标记是 `-1` 而不是 `-2`（会触发完整输出，见 §4.2.7）。

第一段必须是 t 段，否则抛 `"Do not allow the first element to be a hard-edged model element; very short drift can be added at the beginning"`。

### 3.3 段循环

1. `MethodMP.startClock(0x168) = now()`；`MethodMP.zStartRun(0x280) = particles[N].z`。
2. 局部 `t = beam.tMin`（Beam+0x88），`dtRef = MethodMP.dt`，`zSeg = 0.0`。
3. 若 `dumpPeriodicity != 0`：对 i<N 写 `rel = (x−xs, γβx, y−ys, γβy, z−zs, γβz)`（xs… 为同步粒子坐标），然后
   `beamOut(beam, planeRec, mode=0, t=tMin, zs, step=-1)`。
4. 按段顺序：t 段 `tcodeRun(runFlag, t, dt, zSeg, marks_t[k], ev_t[k], planeRec, spRec, beam, lattice, pic, mesh, leapfrog, err, boundary)`；
   z 段 `zcodeRun(t, zSeg, marks_z[j], elems_z[j], planeRec, beam, lattice, pic, mesh, err)`。`t`、`zSeg` 由各段就地更新传给下一段。

### 3.4 结束

- `cout << "Simulation completed, completion time " << strftime(...) << 77 个空格 << endl`
- `cout << "Total number of cycles " << PIC+0x168 << ". Solving the number of space charge effects " << PIC+0x16c << endl`
  （这两个 int 只在 `PIC::PIC` 中清零，本 .so 内无其他写入 → 推测恒为 0；真正的求解计数在 PIC+0x348）。
- `MethodMP.totalTime(0x190) += (now − t_start)/1000`（ns 整除 1000 得 µs，再转 double）。
- 以 `app|out` 追加 `SimulationTime.txt` 一行，流精度 5（默认浮点格式，即 `%.5g` 风格）：
  ```
  T/1e6 " Exter:" c0 ": " E0/1e6 " " c1 ": " E1/1e6 " " c2 ": " E2/1e6 " " c3 ": " E3/1e6
  " Inter:" c4 ": " P0/1e6 " " c5 ": " P1/1e6 " " c6 ": " P2/1e6 " " c7 ": " P3/1e6 " " c5 ": " P4/1e6
  " " c8 ": " P5/1e6 " " c9 ": " P6/1e6 endl
  ```
  `ck = cores[k]`（setnumberofcores 数组，注意 **c5 被打印两次、c? 中没有专门对应 P4 的项**——原程序笔误，照抄）；
  `E0..E3 = MethodMP+0x170/0x178/0x180/0x188`（µs，分别是 t 段宏粒子推进、HardEdgeTrans、HESCFieldPre、zTotcode 的累计耗时）；
  `P0..P6 = PIC+0x178…0x1a8` 七个 double（F 的计时）。样例：`1.6092 Exter:24: 0.0522 24: 0 20: 0 24: 0 Inter:1: 0.026259 …`。

### 3.5 输出面的来源（A 解析，这里只给结论）

lattice 里 `outputplane d` 在"当前已累计长度 + d"处加一个面；`automaticoutput V1 V2 V3`：首面 = 当前累计长度 + V1，间距 V2，
直到 `≤ 首面 + V3`；读到 `end` 时：先 push lattice 总长 L；若有 automaticoutput，则**清空**已有面（包括手写 outputplane），按
automaticoutput 重建，再在最后一个面 ≠ L 时补 L。最后检查单调：任一 `P[i] > P[i+1]` 抛 `"Output plane anomaly, check automaticoutput"`。
因此 **L 永远是最后一个输出面** → 内核总会在末端写 `outData_<L>.dst`（AGENTS.md "总会写 outData_<末端>.dst" 的来源）；
写在末端之外的输出面使列表不单调而报错（AGENTS.md "超出末端会报错"）。

---

## 4. MethodMP::tcodeRun（时间推进）

### 4.1 入口检查与准备

1. 若 `events[0] <= beam.zMax`：抛 `"Tcode won't start, there are particles located behind the output plane. headParticleCoordinates: "
   + sprintf("%f", beam.zMax) + " line: " + sprintf("%f", events[0])`。注意此时 `beam.zMax` 是**上一段留下的值**（z 段不更新它，见 §8）。
2. 分块（多线程，§7）：`nCh = cores[0]`，`c = ceil(N / nCh)`，边界 `b = [0, c, 2c, …]`（`b.back()+c < N` 且已推入数 < nCh 时继续），
   最后 push N；**改写 `cores[0] = b.size()−1`**（持久修改，后续调用沿用）。`nCh < 1` 时 `b=[0]`，`cores[0]=0`。
3. 每块累加器（大小 `cores[0]`）：`maxAbs[6]`、`sum2[6]`、`cross[3]`、`zmin`(1e10)、`zmax`(−1e10)、`sumGz2`、`cnt[2]`、`spCnt`(int)、`sum1[6]`；
   `oldCoord`：N+1 个 `vector<double>(6)`（推进前坐标，供插值）；`syncRef[6]`；`prev[18]`。
4. `UpdateBeamParameters(beam, lattice)`（§5）；`synPout(beam, t)`（写一行，§6.2）。
5. `prev[0..5] = beam.centroid`，`prev[6..11] = beam.syncCoord = particles[N][x..γβz]`，`prev[12] = beam.gamma`，`prev[13] = beam.totalWeight`，`prev[14..17] = 0`。
6. `t += dt`；`TcodeTrans(..., time=t, dt, ..., γ=0.0, chunk=0, range=[N, N+1))` —— **只推同步粒子一步**；`syncRef = particles[N][0..5]`。
7. `γp = beam.gamma`，`βp = sqrt(1 − (1/γp)/γp)`；全部块累加器清零（zmin=1e10，zmax=−1e10）。
8. `iEv = 0`，停滞计数 `stall = 0`，`lastCross = 0`；若 `*runFlag == 0` 直接抛 `"Programs terminated by users.                       "`。

### 4.2 每步循环（`do { … } while (*runFlag != 0)`）

**4.2.1 空间电荷**（`spaceCharge`=MethodMP+0x1a0 为 1 时）：`beam.time = t`（在判断之前就写入）。
`beam.impScField == 0`：`scmethod == 0` → `Weight(mesh, beam, dt)`、`FFT`、`Allocate(mesh, beam)`；`scmethod == 1` 且 `autoPIC(beam)==1` →
`Weight`、`PICNIC`、`Allocate`；其他情况本步不算。`impScField == 1` → `importSCF(beam)`。无论算没算，`PIC+0x348 += 1`。
即**每个时间步最多一次**空间电荷求解，位置是推进之前。

**4.2.2 宏粒子推进**：对每块 k 向线程池提交 `TcodeTrans(lattice, beam, zSeg, err, events, planeRec, spRec, oldCoord, t, dt, syncRef,
spCnt, cnt, maxAbs, sum2, cross, zmin, zmax, sumGz2, sum1, γp, k, b[k], b[k+1])`，等待队列空且活动任务数为 0。
`MethodMP+0x170 += 耗时µs`。TcodeTrans 对每个粒子：先用推进前的 `rel` 做统计（maxAbs/sum2/cross/zmin/zmax/sumGz2，只统计 `lost != 1`
且非同步粒子），再 Boris 推进（§4.5）、算新 `rel`、在新位置以时间 t 求外场、判损失、插值事件面穿越（写 `planeRec[i]`）、
superposeout 出口穿越（写 `spRec[i]`、`spCnt++`），最后对推进后 `lost != 1` 的粒子累加 `cnt[0]+=1, cnt[1]+=w, sum1 += w·rel`。细节见 B。

**同步粒子与宏粒子的顺序**：同步粒子在上一步末尾（或 4.1.6）先以时间 t 推进，本步宏粒子再以同一个 t 推进，`syncRef` 取同步粒子推进后的坐标，
所以推进后二者处于同一时间层。

**4.2.3** `N == 1` → `singlePOut(beam, t)`。

**4.2.4 boundary.txt 几何损失**（`boundary`=MethodMP+0x1cc 为 1）：对 i<N：
- 若 `z < 0` 或 `lost != 0` 或 `z >= Boundary.segEnd.back()`（Boundary+0x18 的最后一个）或 `z < Particle.spOutZ0`(0x108)：不查几何；其中若还满足
  `z > L` 且 `lattice.spOutCur == lattice.spOut.size()` 且 `lost == 0` → `lost = 2`。
- 否则 `row = Boundary.rows[Particle.spOutCount]`（Boundary+0x60），`zz = row[0] + z`，`r1=round(row[1])`，`r2=round(row[2])`；
  仅当 `segStart[r1] <= zz <= segEnd[r2]`：`gi = Particle.geomIndex`；**若 `r2 < gi < r1` 则 `gi = r2`**（原文如此，通常永不成立）；
  向下搜 `while zz < segStart[gi]: gi--`，向上搜 `while segEnd[gi] <= zz: gi++`，存回 geomIndex；调用
  `Geometry::Boundary(geoms[gi], t, dt, particle, oldCoord[i](拷贝), outputFilePath)`（§6.12）。

**4.2.5 块归约**（顺序固定：块 0,1,2,… 依次累加到从 0.0 开始的和，与线程调度无关）：
`beam.zMin = min_k zmin_k`，`beam.zMax = max_k zmax_k`，`beam.nAlive = Σ round(cnt_k[0])`，`beam.totalWeight = Σ cnt_k[1]`，
`lattice.spCrossCount(0x22c) += Σ spCnt_k`（跨步累计），`SG = Σ sumGz2_k`，`S1[j] = Σ sum1_k[j]`，`S2[j] = Σ sum2_k[j]`，
`SX[j] = Σ cross_k[j]`，`MX[j] = max_k maxAbs_k[j]`。
若 `cores[0] < 1`，或 `nAlive == 0`，或（`S2[0]==0 && S2[2]==0 && S2[4]==0 && N != 1`）：抛
`"End of simulation, all particles lost.                     "`。

**4.2.6 段末检查（写 DataSet 之前）**：`zev = events[iEv]`。若 `zev < beam.zMin` 且 `zev <= spOut[spOutCur][6]` 且 `events.size() <= iEv+1`
（最后一个事件，全部活粒子已越过）：若 `marks[iEv] == -1` 则 `FullParticleOutput(beam)`、`dstOut(zev, beam, planeRec)`、
`beamOut(beam, planeRec, 0, t, particles[N].z, step)`；然后**返回**（不写本步 DataSet 行）。

**4.2.7 束流统计与 DataSet 行**（W' = `prev[13]`，即上一步的总权重）：
```
mx = SX[0]/W', my = SX[1]/W', mz = SX[2]/W'            # <Δx·Δx'>, <Δy·Δy'>, <Δz·δ>
qx = S2[0]/W', qxp = S2[1]/W', qy = S2[2]/W', qyp = S2[3]/W', qz = S2[4]/W', qd = S2[5]/W'
εx = sqrt(qxp·qx − mx²), εy = sqrt(qyp·qy − my²), εz = sqrt(qd·qz − mz²)
εnx = εx·βp·γp,  εny = εy·βp·γp,  εnz = γp·εz·βp·γp·γp            # 按这个乘法顺序
centroid[j] = S1[j] / beam.totalWeight   (j=0..5)                 # 新的（推进后）质心
γ̄ = sqrt(γβx̄² + 1 + γβȳ² + γβz̄²)  （顺序：c1²+1+c3²+c5²）
beam.gamma = γ̄；beam.betaGammaLambda = (c/f)·centroid[5]
beam.rms = { sqrt(qx), sqrt(qy), sqrt(SG/W') }
```
其中 TcodeTrans 统计用的偏差（相对**统计时刻的** `beam.centroid`，即上一步质心）：`Δx = rel_x − c0`，`Δx' = (γβx − c1)/γβz_i`，`Δy`、`Δy'` 同理，
`Δz = rel_z − c4`，`δ = (γβz_i − c5)/γβz_i`，`SG` 累加 `w·(γp·Δz)²`（γp 是上一步的 γ̄）；权重 `w = Particle.weight`。
rms 不再减均值的平方（偏差本身已对上一步质心取）。

`lpBoundary`(MethodMP+0x1d0)==1：`beam.gamma = sqrt(1+|u_s|²)`（同步粒子），`beam.betaGammaLambda = (c/f)·γβz_s`，
`h = 0.5·(c/f)·γβz_s / γ̄`；对 `lost == 0` 的 i<N 循环 `while z−zs > h: z −= 2h`，`while z−zs < −h: z += 2h`。
`track` 非空 → `SingleTrack(beam, step)`。

然后以 `app|out` 打开 `DataSet.txt`，用流默认格式（`%g` 风格，6 位有效数字；int 按十进制）写一行 41 列，空格分隔，`'\n'` + flush 后关闭：

| 列 | 内容 | 单位 |
|---|---|---|
| 0 | `(prev[12] − 1)·particles[N].restMassMeV` = 由**平均动量**算的动能 (γ̄−1)mc²（不是能量的平均） | MeV |
| 1–6 | `prev[0..5]`：⟨x−xs⟩, ⟨γβx⟩, ⟨y−ys⟩, ⟨γβy⟩, ⟨z−zs⟩, ⟨γβz⟩（权重平均，上一步） | m / 1 |
| 7, 8, 9 | αx = −mx/εx, αy = −my/εy, αz = −mz/εz | 1 |
| 10, 11, 12 | βx = qx/εx, βy = qy/εy, βz = qz/εz | m |
| 13, 14, 15 | εnx, εny, εnz（rms，归一化） | m·rad |
| 16, 17 | sqrt(qx)=rms x, sqrt(qxp)=rms x' | m, rad |
| 18, 19 | rms y, rms y' | |
| 20, 21 | rms z（实验室系 Δz，不乘 γ）, rms δ | m, 1 |
| 22–27 | max|Δx|, max|Δx'|, max|Δy|, max|Δy'|, max|Δz|, max|δ| | |
| 28 | 存活宏粒子数（上一步的 `beam.nAlive`，`lost != 1`，**含 lost==2**） | int |
| 29–34 | `prev[6..11]` 同步粒子 x, γβx, y, γβy, z, γβz | |
| 35 | `prev[14]` 弯转标志 0/1/2 | |
| 36 | `prev[15]` rotMode（double） | |
| 37 | `prev[16]` 本步同步粒子横向位移 | m |
| 38 | `prev[17]` 本步同步粒子纵向位移 | m |
| 39 | `step − 1`，step = MethodMP+0x1c8 全局步计数（尚未自增） | int |
| 40 | 本步推进所用时间 t | s |

**时间对齐**：第 k 次迭代写出的行描述"完成 k−1 次宏粒子推进后"的状态（第一行是初始分布，第 39 列为 −1），而第 40 列是第 k 次推进所用的 t
（第一段第一行 = tMin + dt）。α/β/ε 用上一步 γp、βp 归一化，与状态一致。z 段（zcodeRun）期间**不写 DataSet 行**，但 step 照样增加，所以第 39 列会跳号。
样例（hwr010）第一行：`1.52714 -1.33776e-05 … 5260 0 0 0 0 -0.00459569 0.0570787 0 0 0 0 -1 -1.4593e-10`，第二行第 38 列 = 0.00210264 = 同步粒子每步 Δz。

**4.2.8** 块累加器清零；`synPout(beam, t)`；`prev[0..5] = centroid`、`prev[6..11] = 同步粒子当前坐标`、`prev[12] = beam.gamma`、
`local nAlive = beam.nAlive`、`prev[13] = beam.totalWeight`。

**4.2.9 弯转标志 prev[14..17]**（old = `oldCoord[N]`，即同步粒子最近一次推进前的坐标；rot = beam.rotMode）：
- rot==0：若 `prev[14]==2`：同步粒子 `z < zSeg` → `{2,0,0,0}`，否则 `{0,0,0, zs − zSeg}`；否则 `{0,0,0, zs − old.z}`。
- rot≠0：若 `prev[14]==2` → `{2,0,0,0}`；否则若 `spRec[N][7]==1`（同步粒子本步越过 superposeout 出口）→
  `f = 1 − (t − spRec[N][6])/dt`，`{2, rot, (rot==1 ? ys−old.y : xs−old.x)·f, (zs−old.z)·f}`；否则 `{1, rot, rot==1 ? ys−old.y : xs−old.x, zs−old.z}`。
（Python `PathLength` 按这三列累加弧长，与此一致。）

**4.2.10** 若 `nAlive == 0` 抛 "End of simulation, all particles lost."。

**4.2.11 superposeout 出口重启**：若 `nAlive <= lattice.spCrossCount` 且 `spOutCur < spOutCount`：`zSeg = spOut[spOutCur][6]`，`spCrossCount = 0`；
- 若 `zSeg == L`：`dstOut(zSeg, beam, spRec)` 并**返回**。
- 否则：`tmin = min{ spRec[i][6] : i<N, spRec[i][7]==1 }`（初值 1e10）；对 i = 0..N（**含同步粒子**）：
  `spRec[i][7] != 1` → `lost = 1`；否则 γ、β 由记录动量算，`x' = γβx/γβz`，`vz = sqrt((βc)²/(x'²+y'²+1))`，`Δt = spRec[i][6] − tmin`，
  `x = rec.x − (rec.γβx·Δt/γ)·c`，`y` 同理，`z = −Δt·vz + zSeg`，动量取记录值，`spOutZ1(0x110) = spOut[spOutCur+1][6]`，`spOutCount(0xfc)++`，`spOutZ0(0x108) = zSeg`。
  然后 `beam.rotMode = 0`、`beam.rotAngle = 0`、`t = tmin − dt`、`spOutCur++`、`elements[sync.elemIndex].rotMode(0x6c) = 0`，spRec 全清 0。

**4.2.12 事件处理**：`zev = events[iEv]`；若 `zev < beam.zMin` 且 `zev <= spOut[spOutCur][6]`（全部活粒子已越过）：
- 若 `iEv+2 <= events.size()` 且 `events[iEv+1] <= beam.zMax`：抛 `"The output plane interval is less than the bunch length. Increase the output plane interval and restart the program"`
  （AGENTS.md "离末端 5 mm 失败、2 cm 可以" 的原因：最后两个面间距小于束长）。
- 若是最后一个事件：同 4.2.6（`-1` 时三项输出）后返回。
- 否则 `iEv++`，`m = marks[iEv−1]`：
  - `m > 0`（steerer）：`par = elements[m].GetParameter()`（vtable+0x18），`L' = elements[m+1].length`，`k = L'·particles[0].qOverM`；
    `par[0]==0`（磁）：`dX = par[2]·k/c`，`dY = par[1]·k/c`，对 i<N 且 `planeRec[i][7]==1`：`rec.z = 0`，`rec.γβx −= dX`，`rec.γβy += dY`；
    否则（电）：`dX = par[1]·k/c²`，`dY = par[2]·k/c²`，`rec.z = 0`，`rec.γβx += dX/β_i`，`rec.γβy += dY/β_i`（β_i 由该记录动量算）。
    即磁 By>0 使 γβx 减小，Bx>0 使 γβy 增大；电 Ex>0 使 γβx 增大（与 AGENTS.md 一致）。同步粒子记录不加冲量。
    然后 OpenMP 并行求 `tmin = min{planeRec[i][6] : i<N, flag==1}`（`._omp_fn.0`，**无原子保护的竞争写**，§7）；
    `t = zTotcode(zev, beam, planeRec, tmin)`（返回 tmin）；`zSeg = zev`；以剩余的 `marks[iEv…]`、`events[iEv…]` **递归调用 tcodeRun** 后返回。
  - `m <= 0`（输出面）：`dstOut(zev, beam, planeRec 拷贝)`，然后 planeRec 全清 0，继续。

**4.2.13 进度**：距上次（初值为本次 tcodeRun 开始）超过 3.0 s → 重置计时并 `simulationProgress(beam, err)`。

**4.2.14 推同步粒子**：`t += dt`；`TcodeTrans(..., time=t, ..., γ=γ̄, chunk 0, [N, N+1))`；块累加器清零。

**4.2.15 rotMode**：`e = elements[particles[N].elemIndex]`，`rm = e+0x68`：
- 0：`syncRef = 同步粒子坐标`，`beam.rotMode = 0`。
- 1：`gz = γβz_s, gy = γβy_s`；`|gz| <= 3e-4` → `θ = ±π/2`（gy>0 取 +，否则 −），`cosθ` 取 6.123233995736766e-17，`sinθ = ±1`；否则 `θ = atan(gy/|gz|)`，`(sinθ,cosθ)=sincos(θ)`；
  `beam.rotAngle = θ`，`beam.rotMode = 1`，`syncRef = {x, γβx, y, gy·cos+gz·sin, z, cos·gz − gy·sin}`，`e+0x6c = 1`。
- 其他：同理用 `gx = γβx_s`：`θ = atan(gx/|gz|)`（或 ±π/2），`rotMode = 2`，`syncRef = {x, gx·cos − gz·sin, y, γβy, z, sin·gx + gz·cos}`，`e+0x6c = 2`。
  （注意 1 与 2 的旋转方向写法不同，照抄；推测见 §9。）

**4.2.16** `dumpPeriodicity != 0` 且 `step % dumpPeriodicity == 0` → `beamOut(beam, planeRec, 0, t − dt, particles[N].z, step)`。
`pcHistogram[0] == 1.0` → `exOutput(beam, step)`。然后 `step++`。

**4.2.17 停滞判丢**：`nc = #{i<N : planeRec[i][7] == 1}`；若 `nc == lastCross && nc != 0` 则 `stall++`，否则 `stall = 0`；`lastCross = nc`。
若 `stall > 0.8·step`（step 已自增，全局计数，含 z 段步数）：对 i<N 且 `planeRec[i][7] == 0.0`：`lost = 1`，并向 `LongitudinalBeamLoss.txt`
追加 `fixed, precision 12`：`id(0x170)` 紧接 `x  γβx  y  γβy  z  γβz`（**id 与 x 之间没有分隔符**，其余两个空格）+ `'\n'`。

**4.2.18** `longLimits[0] != 0` → `LongitudinalBunchLoss(beam)`。

### 4.3 zTotcode（z 表示 → t 表示）

`zTotcode(zPlane, beam, rec, tRef)`：`omp_set_num_threads(cores[3])`（对调用线程持久生效）；并行（静态分块：`N/nt`，余数给前几个线程）对 i<N：
`nextPlane(0x104) = 0`；若 `rec[i][7]==1`：`γ = sqrt(γβx²+1+γβy²+γβz²)`，`β = sqrt(1−(1/γ)/γ)`，`x'=γβx/γβz`，`y'=γβy/γβz`，
`vz = sqrt((β·c·β·c)/(x'²+y'²+1))`，`Δt = rec[6] − tRef`，`x = rec.x − ((rec.γβx·Δt)/γ)·c`，`y` 同理，`z = −Δt·vz + zPlane`，动量 = 记录值。
`rec[i]` 无论 flag 都清 0。未穿越的粒子坐标不变。同步粒子：`z = zPlane − vz·(rec[N][6] − tRef)`，`x = γβx = y = γβy = 0`（强制在轴上），
`γβz = rec[N][5]`，`nextPlane = 0`，`rec[N]` 清 0。返回 `tRef`。

### 4.4 zcodeRun（矩阵段）

参数 `(t&, zSeg&, marks, elems, rec, beam, lattice, pic, mesh, err)`；`tRef = 0.0`（局部，交给 HardEdgeTrans 维护）。对每个元件 j：
1. `split = (spaceCharge == 1) || marks[j] > −1.5 （即 −1 或 steerer 序号）`，且元件不是 `edge`（edge 总是整段）。
2. 不分割：`HardEdgeTrans(el, zStart, zEnd, rec, beam, lattice, &mode=0, &tRef, err)`（C），`MethodMP+0x178 += µs`。
3. 分割：`mode=2`，`HardEdgeTrans(el, zStart, zMid, …)`，`step++`，计时；若 `spaceCharge==1`：`HESCFieldPre(beam, rec, lattice, &tRef)`（`+0x180` 计时）后
   按 4.2.1 同样规则调用 PIC（此处 Allocate 放在各分支内部，`importSCF` 分支不 Allocate），`mode = 1`，`PIC+0x348++`。
   然后：`marks[j]==−1` → `dstOut(zMid, beam, rec 拷贝)`；`marks[j] >= 0` → 同 4.2.12 的校正铁冲量（L' = 当前元件长度，即 steerer 后一元件）。
   再 `HardEdgeTrans(el, zMid, zEnd, …)`。
4. 两种情况都再调 `HardEdgeTrans(el, rec, beam, lattice)`（5 参数重载，C）。
5. `dumpPeriodicity != 0` → `beamOut(beam, rec, mode=1, tRef, zEnd, step)`（**每个元件都写**，不取模）。`step++`。
6. `marks[j] == −2` → `dstOut(zEnd, beam, rec 拷贝)`。

全部元件后：`t = zTotcode(zEnd(最后元件), beam, rec, tRef)`，`MethodMP+0x188 += µs`，`zSeg = zEnd`。

### 4.5 LeapFrog::Update（Boris 推进；本 .so 中无调用者，TcodeTrans 内联了同一算法）

单位：u = γβ（无量纲），E V/m，B T，q/m C/kg，dt s。对一个粒子（`Update(dt, Particle&)` 仅当 `lost != 1`）：
```
h  = (qOverM / c) · dt · 0.5
u  += h·E_tot                      # 半步电场冲量（E_tot = Particle+0x90）
γ  = sqrt(uz² + uy² + ux² + 1)     # LeapFrog 的求和顺序；TcodeTrans 是 ((ux²+1)+uy²)+uz²
b  = (qOverM·dt/γ) · B_tot         # 全角，Boris 的 t = b/2
D  = 1/((bx²+by²+bz²)·0.25 + 1)
ux' = ((1+(bx²−by²−bz²)/4)·ux + (bx·by/2 + bz)·uy + (bx·bz/2 − by)·uz)·D   # 每项各乘 D 再相加
uy' = ((bx·by/2 − bz)·ux + (1+(by²−bz²−bx²)/4)·uy + (by·bz/2 + bx)·uz)·D
uz' = ((by + bx·bz/2)·ux + (by·bz/2 − bx)·uy + (1+(bz²−by²−bx²)/4)·uz)·D
u  = u' + h·E_tot                  # 第二个半步
γ  = sqrt(|u|²+1);  x += u·(c/γ)·dt   # 写作 x += ux·(dt·(c/γ))
```
每一项的写法：`term = coef·u_k·D`，三项与 `h·E` 按"项1+项2+项3+hE"左结合相加（Beam 版本把第二个半步并进同一表达式，Particle 版本分开写，
数值相同）。`Update(dt, Beam&)`：对 i<N 中 `lost != 1`（lost==2 照样推）的粒子逐个，然后无条件推第 N 个（同步粒子）。
Particle 版本开头多一次无用的 sqrt。这是标准 Boris（旋转为 Cayley 变换，`u+ − u− = (u+ + u−)×t`）。

---

## 5. MethodMP::UpdateBeamParameters(beam, lattice)（只在 tcodeRun 入口调用；lattice 未使用）

1. `nAlive=0, zMin=1e10, zMax=−1e10, rms={0,0,0}, W=0`。
2. 对 i<N 且 `lost != 1`（**lost==2 也算**）：`nAlive++`，`W += w`，写 `rel = (x−xs, γβx, y−ys, γβy, z−zs, γβz)`，累加 `Σ w·rel_j`。
3. `centroid_j = Σ/W`（第 0 项用局部累加的 W，其余用写回的 Beam.totalWeight，数值相同）；`γ̄ = sqrt(c1²+1+c3²+c5²)` → `beam.gamma`；
   `βz = c5/γ̄`，`beam.betaGammaLambda = (c/f)·βz·(1/sqrt(1−βz²))`（**与 tcodeRun 里 `(c/f)·c5` 的公式不同**）。
4. 对**全部** i<N（不看 lost）：更新 zMin/zMax（实验室 z）；`Δx = x − c0`、`Δy = y − c2`、`Δz = z − c4` ——**用绝对坐标减相对质心**（原程序缺陷：
   同步粒子不在原点时 Δz≈zs），`Δz_γ = Δz/sqrt(1−βz²)`；`rms = {sqrt(Σw Δx²/W), sqrt(Σw Δy²/W), sqrt(Σw Δz_γ²/W)}`。x'、y'、δ 的和算了但丢弃。
5. 这些 Beam 字段在 tcodeRun 第一步归约后即被覆盖；只影响第一次空间电荷求解前的 `beam.rms`（若 PIC 用它，见 §9）。

---

## 6. 输出函数（文件均为 `outputFilePath + 名字`）

### 6.1 格式约定
"默认格式" = `std::ostream` 默认（precision 6、通用格式，glibc 下 NaN 打印为 `nan`/`-nan`）；"fixed12" = `setprecision(12)` + `fixed`。
文本文件追加模式 `ios::app|ios::out`（0x11），每次打开-写-关闭；二进制 `app|binary|out`（0x15）或 `out|binary`（0x14，截断）。

### 6.2 synPout(beam, t) → synParticle.txt（追加，fixed12）
`strftime("%Y/%m/%d %X  ")` 后依次：`z, (γs−1)·mc²_s, x, y, γβx, γβy, γβz`（以 `"  "` 分隔），`"  " rotMode(int) "  " rotAngle`，
`" "`（一个空格）`extE_x "  " extE_y "  " extE_z "  " extB_x "  " extB_y "  " extB_z "  " lost(int)` + `'\n'`。全部是同步粒子（particles[N]）。
参数 t 未使用。每个 tcodeRun 入口一行 + 每写一行 DataSet 一行。样例文件每行 17 个字段（没有末尾的 lost），与本 .so 的 18 个不同（版本差异，§9）。

### 6.3 beamOut(beam, rec, mode, t, zs, step) → BeamSet.plt（`app|binary|out`）
打开失败只打印 `"BeamSet.plt cannot be output"`。记录：`char 0x7D, int32 mode, int32 step, double t, double zs`，然后对 i<N：
- mode 0：`6×double rel[0..5]`，`int32 (lost==1 ? 0 : 1)`，`int32 i`；
- mode≠0：`rec[i][0..3]`（4 double），`rec[i][6]`，`rec[i][5]`，`int32 round(rec[i][7])`，`int32 i`。
无文件头时（dumpPeriodicity=0）文件只有末端那一条记录（样例 `7d 00000000 65000000 …`）。

### 6.4 dstOut(z, beam, rec) → outData_<z>.dst / .dstb / .edst
文件名 `outData_` + `sprintf("%f", z)` + 扩展名（如 `outData_0.210000.dst`），均以 `out|binary`（截断）打开，失败打印 `"output error"`。
`t0 = rec[N][6]`（同步粒子穿越时间），`mc²_s = particles[N].restMassMeV`。对选中的粒子：
`X = x·100 (cm)`，`X' = γβx/γβz`，`Y = y·100`，`Y' = γβy/γβz`，`φ = (d + d)·f`，其中 `d = (rec[6] − t0)·π`，`W = (γ−1)·mc²`（γ 由记录动量算）。
- **beam.fromDstFile == 0**（常见）：选 i<N 且 `rec[i][7]==1`，`W` 用同步粒子的 mc²；
  `I = |Σw_sel · realPerMacro · particles[N].charge · f · 1000|`（mA，仅 spaceCharge==1，否则 0）。
  `.dst`：`char 0x7D, char 0x64, int32 n, double I, double f/1e6, char 0x7D, n×{X,X',Y,Y',φ,W}, double mc²_s` —— TraceWin dst 格式。
  `beam.useWeightFile == 1` 时另写 `.dstb`：`int32 n, n×double weight`。
- **fromDstFile ≠ 0**：选 i = 0..N（**含同步粒子**）且 flag==1，`W` 用各自的 mc²；`I = beam.current·1000`；写 `.edst`：
  `char 0x7D, char 0x64, int32 (n−1), double I, double f/1e6, char 0x7D`，每个选中粒子 `{X,X',Y,Y',φ,W, charge/w/e, (mass/w/m_p)·938.271875, w}`（9 double），末尾 `double mc²_s`。
  （头里写 n−1 而记录有 n 条，见 §9。）

### 6.5 FullParticleOutput(beam) → FullParticle.txt（追加，默认格式）
对 i<N 一行：`lost charge x γβx y γβy z γβz`（单空格分隔，charge 为宏粒子电荷 C）。只在末事件为输出面时调用。

### 6.6 singlePOut(beam, t) → SingleParticle.txt（追加，fixed12，不被 Run 截断）
particles[0]：`x  y  z  γβx  γβy  γβz  (γ−1)·938.271875` + `" "` + `t` + `'\n'`（能量用写死的质子质量）。

### 6.7 SingleTrack(beam, step) → ParticleTracking.txt（追加，默认格式）
对 `track` 列表中每个序号 id（不检查越界）：`step id lost charge mc² x γβx y γβy z γβz scE_x scE_y scE_z extE_x extE_y extE_z extB_x extB_y extB_z`。

### 6.8 exOutput(beam, step) → PCHistogram.dat（`app|binary|out`，不被截断）
只用 `lost != 1` 的 i<N。范围：xmin/xmax、ymin/ymax、zmin/zmax 初值 ±1e9，更新用 **else-if**（某坐标刷新了 min 就不再比较 max）；
`r = sqrt(x²+y²)`，rmin 固定 0，rmax 取最大。`n = round(pcHistogram[1])`，各方向 bin = `floor((v − vmin)/((vmax−vmin)/n))`，夹到 `[0, n−1]`。
写：`char 0x7D, int32 step, int32 n`，然后 hx、hy、hr、hz 各 `int32×n'`（循环条件是 `i < pcHistogram[1]` 的 double 比较，非整数 V2 时多写/越界），
再 `double xmin,xmax,x̄, ymin,ymax,ȳ, 0, rmax, r̄, zmin,zmax,z̄`（均值 = 和/计数）。打开失败打印 `"PCHistogram.dat cannot be output"`。

### 6.9 LongitudinalBunchLoss(beam)（不写文件）
`γs`、`βs` 由同步粒子算；对全部 i<N（不看 lost）：`|(γi−1)·mc²_i − (γs−1)·mc²_s| > longLimits[2]` → `lost=1`；
`|z_i − z_s| > (longLimits[1]/360)/f · βs · c` → `lost=1`（相位窗口按同步粒子速度换成长度）。

### 6.10 simulationProgress(beam, err) → stdout
`p = (zs − zStartRun) / ((P.back() − zStartRun) + zs − beam.zMin) · 100`；`T = (now − startClock)/1000/1e6` s；`R = (T·100/p − T)·0.5`。
存入 MethodMP+0x268/0x270/0x278（换算后的值）。`printf` 格式 `"Simulate progress %.2f%%. Estimated remaining time %…. Run time %….  %d  %.2fm"` + 30 个空格 + `"\r"`，`fflush(stdout)`。
T ≤ 60 用 `%.0fs`，60<T≤3600 用 `%.2fmin`（T/60），T>3600 用 `%.2fh`；R > 3600 用 `%.2fh`，60 < R ≤ 3600 用 `%.2fmin`，否则 `%.0fs`。
末尾两个数为 `beam.nAlive` 与同步粒子 z（与 AGENTS.md 一致）。

### 6.11 SimulationTime.txt、DataSet.txt、LongitudinalBeamLoss.txt 见 §3.4、§4.2.7、§4.2.17。

### 6.12 Geometry::Boundary(t, dt, particle, old[6], path)（boundary.txt 几何）
`g = this`，`p = g+0x18`（6 个：[0] z 偏移、[1] x0、[2] y0、[3..5] 三个角，rad），`zs0 = g+0x30`，形状参数 `s = g+0x40`，类型 `g+0x58`。
局部坐标（对任意点 (x,y,z)）：`dx=x−p1, dy=y−p2, dz=(z−zs0)−p0`；`u=c3·dx+s3·dy, v=c3·dy−s3·dx`；`w=u·s5+dz·c5, u'=u·c5−dz·s5`；
`zl=w·c4−v·s4, v'=v·c4+w·s4`（ck/sk = cos/sin p[k]）。
- 类型 0：不处理。类型 >2：打印 `"Unknown geometric shapes have been set"` 并 `exit(1)`。
- 类型 1（直管）：`zl <= 0` 或 `zl >= s[0]` → 返回；`r = sqrt(u'²+v'²)`，`r < s[1]` → 返回。否则对 old 同样变换得 `r0, zl0`，
  `f = (s[1] − r0)/(r − r0)`；`zh = (zl−zl0)·f + zl0`，若 `zh < 0` 则 `f = (0 − zl0)/(zl − zl0)`；碰撞点 `P = old + (new − old)·f`（6 分量）；时间 `t − (1−f)·dt`。
- 类型 2（弯管）：`zl` 范围检查同上；`ρ = s[2]`；`s[4]==0`（x–z 弯）：`s[5] <= 0` → `φ = atan(zl/(u'+ρ))`，中心 `(cx, cz) = ((cosφ−1)ρ, ρ sinφ)`；
  否则 `φ = atan(zl/(ρ−u'))`，`cx = (1−cosφ)ρ`；`s[4]==1`（y–z 弯）同理用 v'、得 cy；其他：φ=0、中心 0。`φ > |s[3]|·π/180` → 返回；
  `d = sqrt((v'−cy)² + (u'−cx)² + (zl−cz)²)`，`d < s[1]` → 返回。否则碰撞点取**当前**坐标，时间 t。
- 碰撞：向 `collisionData.txt` 追加（默认格式）：`X " " Y "  " Z "  " vx "  " vy "  " vz "  " charge/e "  " mc² "  " w "  " t_hit "  " kind` + `'\n'`
  （`v = γβ·c/γ`），然后 `lost = 1`。
- `Geometry::Boundary(double,double,SynParticle&,vector)` 重载（0x147e20）只做类型检查（>2 报同样错误并退出），无调用者。

### 6.13 无调用者的输出函数
`MethodMP::testOut(beam)`：向 test.txt / testP.txt 追加（每个 `lost==0` 粒子 12 个量：坐标与 rel；以及一行质心、rms、rotAngle 等）；
`MethodMP::errorOut(msg)`：`cout << msg`，并向 `ErrorLog.txt` 追加 `strftime + "   simulation stage error: " + msg`。二者在本 .so 中都不被调用
（`main_agent` 用的是同名全局函数 `errorOut`）。新内核可不实现，但 Run 仍要截断 test.txt、testP.txt。

---

## 7. 多线程

- **线程池**：`MethodMP` 内嵌 `progschj::ThreadPool`，工作线程数 `max(2, hardware_concurrency())`，任务队列上限在 MethodMP+0x70。
  t 段每步把宏粒子按 §4.1.2 的块边界提交 `cores[0]` 个任务；每块写自己的累加器，归约按块序号顺序进行 → **在块划分固定时结果与调度无关**
  （AGENTS.md "固定种子后两次运行完全相同" 的原因）；但块数取决于 `omp_get_max_threads()`（见下），不同核数的机器上求和顺序不同，结果在舍入级别不同。
- **OpenMP**：zTotcode 用 `cores[3]` 个线程；tcodeRun 的 `tmin` 并行求最小值（`._omp_fn.0`）对共享变量无锁读-比较-写，存在数据竞争，结果可能不确定（只在 steerer 事件发生）。
- **cores 数组**（setnumberofcores，10 个 int；默认 `{-1,1,1,1,1,1,1,1,1,1}`，`Initial::Initial`）：在 `InitParticle` 中，
  若 `multithreading == 1` 且 `cores[0] == -1`，按粒子数 N 取表（N≤3e4：`{64,54,20,24,1,48,96,12,48,12}`；≤8e4：`{72,68,24,56,22,48,96,20,48,24}`；
  ≤3e5：`{96,96,32,96,26,48,96,24,48,36}`；≤8e5：`{96,96,52,96,74,48,96,48,48,96}`；≤3e6：`{96,96,62,96,86,…}`；≤8e6：`{96,96,82,96,92,…}`；
  ≤3e7：`{96,96,92,96,96,…}`；更大：`{96,96,96,96,96,48,96,48,48,96}`）；然后每项截到 `omp_get_max_threads()`，`maxthreads > 0` 时再截到它；拷入 `scPar[10..]`。
  `scPar[9] = multithreading`。`MethodMP+0x1c0`（multithreading 副本）从不被读取。
- **为什么 `multithreading 0`（或 2、3…）丢光粒子**：此时不查表，`cores[0]` 保持默认 −1（clip 不改变负数）；tcodeRun 分块时 `cores[0] < 1`
  → 边界只有 `[0]`，`cores[0]` 被改成 0 → 不提交任何推进任务，归约检测到块数 < 1，立即抛 `"End of simulation, all particles lost."`。
  推论（待验证）：`multithreading 0` 同时显式给 `setnumberofcores`（第一个数 ≥1）应能正常运行。

---

## 8. 损失与粒子状态

`lost`：0 存活；1 丢失；2 越过末端。各来源：
| 来源 | 条件 | 置为 |
|---|---|---|
| 元件孔径（C，`Drift/Field::GetField`） | `boundaryMode == 0` 且越出孔径 | 1 |
| TcodeTrans（B） | `γβz < 0` 且 `spOutCount <= spOutCur` 且 `z < 0` | 1 |
| TcodeTrans / tcodeRun 4.2.4 | `z > L` 且 `spOutCur == spOut.size()` 且 `lost == 0` | 2（无 superposeout 时 spOut 只有一行哑元，spOutCur=0≠1，**实际上不会发生**；越过 L 后外场被清零、粒子自由漂移） |
| Geometry::Boundary | 碰壁 | 1，并写 collisionData.txt |
| LongitudinalBunchLoss | ΔW 或 Δz 超限 | 1（会把 2 改成 1） |
| 停滞判丢 4.2.17 | 长期未越过事件面 | 1，并写 LongitudinalBeamLoss.txt |
| superposeout 重启 4.2.11 | 未越过出口面 | 1 |

统计（DataSet、UpdateBeamParameters、nAlive）一律以 `lost != 1` 为存活，lost==2 仍计入。运行结束靠"最后一个事件面（= L）被全部活粒子越过"，不是靠 lost=2。

---

## 9. 未解决的问题与建议实验

1. **PIC+0x168/0x16c**：Run 末尾 "Total number of cycles … Solving the number of space charge effects …" 打印的两个计数在 .so 中无写入。实验：开空间电荷跑示例，看 stdout 是否恒为 0。
2. **rotMode 旋转方向**：4.2.15 中 rotMode 1 的写法使同步粒子横向动量变成约 2 倍而非 0（rotMode 2 的写法相反）。需要含 superpose 类型 1/2 的 lattice 跑内核，比较 synParticle.txt 第 10–11 列与 DataSet 第 35–38 列。
3. **短 z 段**：z 段不更新 `beam.zMax`，若 z 段短于束长，下一 t 段入口检查会用旧 zMax 报 "Tcode won't start…"。实验：drift + 很短 quad + drift（短于束长）。（若 HardEdgeTrans 实际更新了 zMax，以 C 的结论为准。）
4. **第一步外场**：宏粒子第一次推进用的是推进前已存的 `extE/extB`（InitParticle 阶段的值，推测为 0），即第一步是漂移。由 B 结合 TcodeTrans 核实；实验：起点就在腔内的单粒子运行，比较第一步能量。
5. **.edst 头计数**：fromDstFile 分支头写 n−1，记录含同步粒子共 n 条。实验：用 .dst 初始分布运行，检查 outData_*.edst 字节数 = 1+1+4+8+8+1 + 72·n + 8。
6. **synParticle.txt 列数**：本 .so 写 18 字段，样例（`ADANES/AVAS/AVAS/example/OutputFile`，来源版本不明）17 字段。实验：用当前 libAVAS.so/AVAS.dll 各跑一次。
7. **不被截断的文件**：PCHistogram.dat、SingleParticle.txt 在同一输出目录连续运行两次会累积。实验：连续运行两次比较文件大小。
8. **块数与结果**：设置不同 `OMP_NUM_THREADS`（改变 omp_get_max_threads → cores[0]）跑同一输入（固定种子），比较 DataSet.txt 是否只在末几位不同。
9. **multithreading 0 + setnumberofcores**：验证 §7 的推论。
10. **steerer 的 tmin 竞争**：含 steerer 的 t 段多线程重复运行，看结果是否偶尔不同。
11. **几何段号夹取条件**（4.2.4 `r2 < gi < r1`）疑为笔误；需要多段 boundary.txt + superposeout 偏移的算例确认实际行为。
12. **停滞判丢**：阈值依赖全局 step（含 z 段），较晚的事件面要求更长停滞。实验：制造反射粒子（强减速场）看 LongitudinalBeamLoss.txt 何时出现。
13. **UpdateBeamParameters 的 rms 缺陷**是否影响第一次 PIC 网格（若 F 确认 PIC 读取 `beam.rms`）：比较第一步与第二步的 SpaceChargeMesh 输出。

---

## 10. 对齐清单

| ID | 行为 | 涉及关键字/文件 | 建议的比对测试 |
|---|---|---|---|
| D-01 | dt = (1/f)/V1；Run 打印的 "Simulation time step is" 为该值 | steppercycle、frequency | 解析 stdout，相对差 < 1e-12 |
| D-02 | 元件分段与 markers（§3.2），第一元件为 z 型时报错原文 | lattice | 构造 quad 开头 lattice，比对异常字符串；单元测试分段函数输出 |
| D-03 | Run 截断的文件列表；PCHistogram.dat、SingleParticle.txt 不截断 | 输出目录 | 预置旧文件后运行，比较哪些被清空 |
| D-04 | BeamSet.plt 头（dumpperiodicity≠0）与 beamOut 记录二进制格式 | dumpperiodicity | 逐字节解析，对 header/rel 坐标容差 1e-12 相对 |
| D-05 | tcodeRun 每步顺序：SC → 宏粒子推进 → … → 同步粒子推进 | spacecharge | 单粒子 + 同步粒子算例，比对 synParticle.txt 与 DataSet 第 29–34 列一一对应（DataSet 行 k = synParticle 行 k） |
| D-06 | DataSet.txt 41 列定义、时间对齐（首行第 39 列 −1、第 40 列 tMin+dt） | DataSet.txt | 示例 hwr010 固定种子，逐列比较，容差 1e-5 相对（6 位有效数字） |
| D-07 | DataSet 第 0 列 = (γ̄−1)mc²（平均动量），不是平均能量 | DataSet.txt | 从 outData_*.dst 自算两种定义，确认与第 0 列吻合的是前者 |
| D-08 | α/β/ε 定义（x'=Δγβx/γβz_i，εn = ε·β·γ，εnz = γ³βεz） | DataSet.txt | 用同一状态的粒子分布（dumpperiodicity 输出）自算，容差 1e-6 |
| D-09 | 第 28 列存活数含 lost==2；整数格式 | DataSet.txt | 粒子越过末端的算例 |
| D-10 | 第 35–38 列弯转/位移标志 | superpose 1/2 | 含 superposeout 的算例比对 |
| D-11 | DataSet 数值格式（%g 6 位，单空格，行尾 '\n'） | DataSet.txt | 字符串级比较格式（非数值） |
| D-12 | synParticle.txt 格式（fixed12，分隔符，18 字段） | synParticle.txt | 字符串级比较（时间戳除外） |
| D-13 | 进度行格式、3 s 间隔、R 含 0.5 因子 | stdout | 长算例捕获 stdout，正则匹配 |
| D-14 | SimulationTime.txt 一行格式（precision 5、c5 重复） | SimulationTime.txt | 正则匹配字段数与 cores 值 |
| D-15 | outData_<z>.dst TraceWin 格式、文件名 `%f`、φ=2πf(t−t_s)、W 用同步粒子 mc² | outputplane | 解析二进制，坐标容差 1e-9 相对，粒子数相等 |
| D-16 | .dstb（indweight 1）与 .edst（初始分布来自 .dst）格式 | indweight、readparticledistribution | 二进制长度与字段比对 |
| D-17 | 末端总写 outData_<L>.dst；输出面越界报 "Output plane anomaly…" | outputplane、automaticoutput | 正常算例检查文件存在；越界面算例比对异常 |
| D-18 | 面间距 < 束长报 "The output plane interval is less than the bunch length…" | outputplane | L−5 mm 处加面 |
| D-19 | 输出面在 z 段元件内部 → 元件中点输出；在出口 → 出口输出 | outputplane + quad | 比较 outData 文件名中的 z |
| D-20 | 校正铁冲量方向与大小（磁 −By·L·q/(mc)，电 Ex·L·q/(mc²β)），施加于其后元件中点 | steerer | 单粒子算例，比对下游质心，容差 1e-6 |
| D-21 | zTotcode 重建（x−γβx·Δt/γ·c，z=zPlane−vzΔt，同步粒子强制在轴） | z 段后接 t 段 | 比对 z 段后第一行 DataSet，容差 1e-9 |
| D-22 | zcodeRun 空间电荷在元件中点、每元件一次；无 SC 且无输出时整段传输 | spacecharge、quad | 比较开/关 SC 时 step 计数（第 39 列跳号） |
| D-23 | Boris 推进公式（§4.5） | — | 匀强 E、B 中单粒子与解析解/原内核比较，逐位或 1e-14 |
| D-24 | 空间电荷每 t 步一次、位置在推进前 | spacecharge | PIC+0x348 计数 = t 步数 + 分割元件数（若可读出） |
| D-25 | LongitudinalBunchLoss 判据（相位按 βs 换成长度，能量差绝对值） | longlimits | 构造越界粒子，比较存活数 |
| D-26 | boundary.txt 直管/弯管碰撞与 collisionData.txt 一行格式 | boundary 1 | 单粒子打在管壁，比对碰撞点（容差 1e-9）与行格式 |
| D-27 | lpboundary 周期折返 ±βλ/2（h 用同步粒子 γβz 与平均 γ̄） | lpboundary | 比对折返后 z 分布 |
| D-28 | 停滞判丢及 LongitudinalBeamLoss.txt 格式（id 与 x 无分隔） | — | 反射粒子算例 |
| D-29 | pchistogram → PCHistogram.dat 二进制格式与 else-if 范围更新 | pchistogram | 解析二进制，直方图逐 bin 相等 |
| D-30 | track → ParticleTracking.txt 20 列 | track | 字段数与数值比较 |
| D-31 | N==1 → SingleParticle.txt（能量用 938.271875） | particlenumber 1 | 数值比较 |
| D-32 | multithreading 0/2/3 → "End of simulation, all particles lost."；1 正常 | multithreading、setnumberofcores | 运行比对异常文本；加 setnumberofcores 验证推论 |
| D-33 | cores 表按 N 选取并截到 omp_get_max_threads/maxthreads；cores[0] 被改写为块数 | setnumberofcores、maxthreads | SimulationTime.txt 中打印的 c0…c9 |
| D-34 | 块内/块间求和顺序固定（同核数下结果可重复） | — | 固定种子两次运行 DataSet 逐字节相同 |
| D-35 | superposeout 出口重启（t = tmin − dt，未越过者 lost=1） | superposeout | 含偏移块算例，比对出口后质心 |
| D-36 | "Tcode won't start…" 报错条件与消息（含两个 %f） | — | 构造束团头部已过首个面的初始分布 |
| D-37 | UpdateBeamParameters 的 betaGammaLambda 与 rms 定义（含 x−⟨x−xs⟩ 缺陷） | — | 单元测试（内部量，无文件可比；仅当 PIC 用到时通过 SC 结果间接验证） |

---

## 11. 函数覆盖表

| 函数 | 作用一句话 | 在正文哪节 |
|---|---|---|
| Geometry::Boundary (0x147e20, SynParticle 重载) | 只检查形状类型 >2 报错退出，无调用者 | §6.12 |
| Geometry::Boundary (0x147e50, Particle 重载) | boundary.txt 直管/弯管碰撞判定、写 collisionData.txt、置 lost | §6.12、§4.2.4 |
| LeapFrog::Update(double, Beam&) | 全束 Boris 推进（.so 内无调用者，TcodeTrans 内联同一算法） | §4.5 |
| LeapFrog::Update(double, Particle&) | 单粒子 Boris 推进（无调用者） | §4.5 |
| MethodMP::tcodeRun (0x19d7f0, `._omp_fn.0`) | 并行求已穿越粒子的最小穿越时间（无锁竞争） | §4.2.12、§7 |
| MethodMP::zTotcode (0x19df10, `._omp_fn.17`) | zTotcode 的并行体：逐粒子由穿越记录重建同一时刻坐标 | §4.3 |
| MethodMP::UpdateBeamParameters | tcodeRun 入口的质心、γ、rel、rms 初始化 | §5 |
| MethodMP::synPout | 同步粒子一行写 synParticle.txt | §6.2 |
| MethodMP::testOut | test.txt/testP.txt 调试输出，无调用者 | §6.13 |
| MethodMP::simulationProgress | 打印进度行 | §6.10 |
| MethodMP::beamOut | BeamSet.plt 二进制记录 | §6.3 |
| MethodMP::errorOut | 写 ErrorLog.txt，无调用者 | §6.13 |
| MethodMP::exOutput | PCHistogram.dat 直方图 | §6.8 |
| MethodMP::LongitudinalBunchLoss | longlimits 纵向判丢 | §6.9 |
| MethodMP::singlePOut | 单粒子 SingleParticle.txt | §6.6 |
| MethodMP::FullParticleOutput | FullParticle.txt 全粒子文本 | §6.5 |
| MethodMP::SingleTrack | track 粒子写 ParticleTracking.txt | §6.7 |
| MethodMP::dstOut | outData_<z>.dst/.dstb/.edst | §6.4 |
| MethodMP::zTotcode (0x1abb50) | z→t 表示转换（设 OMP 线程数、处理同步粒子） | §4.3 |
| MethodMP::zcodeRun | 矩阵元件段循环（中点 SC、输出、校正铁） | §4.4 |
| MethodMP::~MethodMP | 释放成员 vector、停止并 join 线程池 | §7（无行为规格） |
| MethodMP::MethodMP | 启动 max(2, hardware_concurrency) 个工作线程，成员清零，0x288=3.0 | §7、§2 |
| MethodMP::tcodeRun (0x1afb20) | 时间步主循环 | §4 |
| MethodMP::Run | 运行总流程：分段、截断文件、调度各段、收尾 | §3 |

---

## 12. 对结构表的更正 / 补充

| 位置 | 原表 | 更正 / 补充 | 证据 |
|---|---|---|---|
| MethodMP+0x170/0x178/0x180/0x188 | 未列 | 累计耗时（µs）：t 段推进 / HardEdgeTrans / HESCFieldPre / zTotcode；写入 SimulationTime.txt | tcodeRun、zcodeRun、Run |
| MethodMP+0x190 | 未列 | 总运行时间（µs） | Run |
| MethodMP+0x1c8 | 未列 | 全局步计数 int（t 步、z 元件及其半段都加 1），DataSet 第 39 列 = 它 −1 | tcodeRun、zcodeRun |
| MethodMP+0x1c0 | multithreading | 拷入后从不读取 | 全部伪代码 |
| MethodMP+0x220 | outputPlanes（推测） | 确定：InitMethodMP 从 Initial+0x1a8 拷入；Run 分段与进度分母使用 | Run、simulationProgress |
| MethodMP+0x288 | 未列 | 进度打印间隔 3.0 s；0x290 = 0（未见使用） | 构造函数、tcodeRun |
| MethodMP+0x1d0 | lpBoundary | 确定：tcodeRun 周期折返；也作为 PIC::PIC 最后一个参数 | tcodeRun、Run |
| PIC+0x348 | 未列 | 空间电荷求解计数（PIC::Initial 清零，tcodeRun/zcodeRun 每次 ++） | tcodeRun、zcodeRun |
| PIC+0x168/0x16c | 未列 | Run 末尾打印的两个 int，只在构造时清零 | Run、PIC::PIC |
| Element+0x68 | inSuperposeOut（推测） | InitLattice 对 superpose 组按类型写 1 或 2（并 push 一行 spOut）；tcodeRun 用同步粒子所在元件的该值决定 Beam.rotMode（1→y–z，2→x–z）；TcodeTrans 以非 0 判 superposeout | InitLattice、tcodeRun、TcodeTrans |
| Element+0x6c | unk_6c | 存当前 rotMode（tcodeRun 写 1/2，superposeout 出口重启时清 0） | tcodeRun |
| Lattice+0x22c | unk_22c | 已越过当前 superposeout 出口面的粒子累计数，推进后由块计数累加，换段时清 0 | tcodeRun |
| Lattice+0x228 | spOutCount | 无 superposeout 时保持 0（哑元行不计入）；最后一个真实块出口 ≥ L 时等于 spOut.size() | InitLattice |
| Particle+0x104 | nextPlane（推测，仅二次粒子） | 主束同样使用：TcodeTrans 用作 events 下标，zTotcode 清 0 | TcodeTrans、zTotcode |
| Particle+0x108 / 0x110 | spOutZ0/spOutZ1（推测） | 主束 superposeout 重启时写入：0x108 = 刚越过的出口 z，0x110 = 下一出口 z；几何检查要求 z ≥ 0x108 | tcodeRun |
| Beam+0x70 | (c/f)·βγ | 两种公式：UpdateBeamParameters 为 λ·βz/sqrt(1−βz²)，tcodeRun 为 λ·⟨γβz⟩，lpBoundary 时为 λ·γβz_s | UpdateBeamParameters、tcodeRun |
| Beam+0x78 | 平均 γ | 一般为 sqrt(1+|⟨γβ⟩|²)；lpBoundary==1 时被改为同步粒子 γ | tcodeRun |
| Beam+0x18 rms[2] | rms(z)·γ | tcodeRun 中 = sqrt(Σw(γp·Δz)²/W)，γp 为上一步平均 γ | tcodeRun |
| Beam+0x90/0x98 zMin/zMax | 存活粒子最小/最大 z | tcodeRun 中取推进**前**的 z（`lost != 1`）；UpdateBeamParameters 中包括丢失粒子 | TcodeTrans、UpdateBeamParameters |
| Initial+0x088 cores 默认 | 未列默认值 | `{-1,1,1,1,1,1,1,1,1,1}`；按 N 的取值表见 §7 | Initial::Initial、InitParticle |
