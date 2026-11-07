# 70 误差研究与二次粒子（libAVAS.so 逆向规格）

> 依据：`decomp/libAVAS/functions/*.c`（Ghidra 伪代码）、对 `avas/engine/libAVAS.so` 直接反汇编（`objdump`，用来恢复 Ghidra 丢掉的字符串字面量和核对个别分支）、`AVAS.dll` 中对应函数（`FUN_18001e2a0` = GenerateErrorSet，`FUN_18001e1a0` = 高斯数）作旁证。
> 偏移、字段名按 `structures.md`；伪代码中被 Ghidra 当成静态函数的成员函数（`*::GetErrField(double, Particle&, FieldMatrix const&)`）`param_2` 实为 `this`，`param_3` 为 `Particle&`。
> 文中“确定”= 有直接代码证据；“推测”= 推理所得，需要实验确认。

## 1. 概述

本子系统包括两块互相独立的功能：

**A. 误差研究（error study）**。lattice 里的 `err_*` 关键字在 `Initial::InitLattice` 中被收集进 `ErrorParameter`；`main_agent` 在 `InitLattice` 之后立即调用 `GenerateErrorSet(ErrorParameter&, Lattice&)`，一次性抽样出全部 `a×b` 组误差（`err_step a b`），放进 `ErrorParameter.dynSamples / statSamples`。若四个误差标志之和非 0，`main_agent` 进入误差循环：对第 k 组（k = 0 … a·b−1）建立 `ErrorSample`，输出目录改为 `<OutputFile>/output_<k>/`，依次 `InitParticle`（束流误差在这里施加）→ `SetSynPhase` → `MethodMP::Run`（或 `SeRun`）。`Run/SeRun` 开头调用 `errSampleOut`（写 StaticErrorData.txt / DynamicErrorData.txt），有元件误差时调用 `Lattice::SetErrInEle` → 各元件 `SetErrPara`；t-code 推进时由 `Lattice::GetErrExternalField`（多线程版在 `TcodeTrans` 内联同样逻辑）按“该元件本组是否有误差”选择 `GetErrField` 或 `GetField`。

```
main_agent
 ├─ InitLattice(Lattice, ErrorParameter, Boundary)   … 解析 err_* 关键字（§2）
 ├─ GenerateErrorSet(ErrorParameter, Lattice)         … 抽样（§3）
 └─ 若 Σflags≠0：for k in 0..a·b-1
       ErrorSample es = {flags, dyn=dynSamples[k]?, stat=statSamples[k]?, +0x48=k+1, +0x4c=0}
       outputFilePath ← <out>/output_<k>/ （mkdir，失败 "Error output folder creation failure" 后 exit(1)）
       InitParticle(Beam, es, Lattice)                 … 束流误差（§5）
       SetSynPhase；MethodMP::Run 或 SeRun（secondarybeam≠0 时）
           ├─ errSampleOut(es)                         … §7
           ├─ 若 flags[2]||flags[3]：Lattice::SetErrInEle(es) → 每个元件 SetErrPara（§4）
           └─ 推进中 GetErrExternalField（§6）
```

**B. 二次粒子输运（secondary beam）**。input.txt `secondarybeam V1≠0` 时 `InitParticle` 从 `readparticledistribution` 指定的文本文件读入每个粒子的位置、速度、电荷、质量、权重、时间和种类（§8），`main_agent` 改调 `MethodMP::SeRun`：先把 lattice 切成 t-code 段与 z-code 段，然后**逐个粒子**（先同步粒子，再 0…N−1）走完全程（§9）。每个粒子用自己的时间步 `dt = V2/(βc)`，没有空间电荷，结果写到每个输出面的 `outData_<z>.txt` 和同步粒子的 `SeSynParticle.txt`（§10）。

## 2. 误差关键字的解析（InitLattice 中与误差有关的部分）

### 2.1 ErrorParameter 布局（sizeof 0xc8，确定）

`main_agent` 构造时：`errStep = {1,1}`，`flags = {0,0,0,0}`，`nElements = 0`，其余容器为空。

| 偏移 | 类型 | 字段 | 含义 |
|---|---|---|---|
| 0x00 | vector\<int\>(2) | errStep | `err_step a b`：[0]=a，[1]=b（`stoi`） |
| 0x18 | vector\<vector\<vector\<double\>\>\> | dynSamples | a·b 组动态误差样本，每组 = 行 0 束流 + 每元件一行 |
| 0x30 | 同上 | statSamples | 静态误差样本 |
| 0x48 | vector\<double\> | beamVals | `err_beam_stat` / `err_beam_dyn` 的数值，平铺 |
| 0x60 | vector\<vector\<double\>\> | elemDefs | 每条元件误差定义（20 个 double 一行；superpose 组内为若干个 20 长度记录拼接） |
| 0x78 | vector\<int\> | beamSwitches | `err_beam_*_on` 的开关，平铺，补齐到 30 / 60 |
| 0x90 | vector\<vector\<int\>\> | elemSwitches | `err_quad/cav_*_on`，每条一行 20 个 int |
| 0xa8 | int | nElements | 读到 `end` 时的元件数 `elements.size()` |
| 0xb0 | vector\<int\>(4) | flags | [0] 束流静态、[1] 束流动态、[2] 元件静态、[3] 元件动态 |

### 2.2 内核认识的关键字（确定）

内核只比较以下 13 个误差关键字（`.rodata` 字符串，顺序为 InitLattice 中的比较顺序）：
`err_beam_stat, err_beam_dyn, err_cav_ncpl_stat, err_cav_ncpl_dyn, err_quad_ncpl_stat, err_quad_ncpl_dyn, err_step, err_beam_stat_on, err_beam_dyn_on, err_quad_stat_on, err_quad_dyn_on, err_cav_stat_on, err_cav_dyn_on`。

**`err_quad_cpl_*`、`err_cav_cpl_*`（耦合误差）在 .so 和 .dll 中都不存在**：InitLattice 对不认识的关键字直接读下一行（`LAB_0017764f`），所以耦合误差行被内核静默忽略。耦合误差完全是 Python 侧（`avas/sim/error.py` 的 `generate_error(..., 'cpl')`）的概念：Python 先按命令抽一次，再展开成每个元件一条 ncpl 命令。

`end` 关键字：`nElements ← elements.size()`；若此时 `beamVals` 为空则 `flags[0]=flags[1]=0`；之后停止读 lattice。没有 `end` 时 `nElements` 保持 0，元件误差没有任何作用对象（推测由其他子系统的输出面逻辑本来就要求 `end`）。

### 2.3 各关键字存入什么（确定）

记一行的单词为 `w[0..n-1]`，`E = elements.size()`（读到这一行时已建立的元件数，即“下一个元件”的序号）。

| 关键字 | 动作 |
|---|---|
| `err_step a b` | `errStep = {stoi(w1), stoi(w2)}` |
| `err_beam_stat …` | `beamVals.push(0.0)`，再 `push(stod(w[1..n-1]))`；然后把 **beamSwitches**（不是 beamVals）补零：长度 <30 补到 30；30 ≤ 长度 <60 补到 60；≥60 不补 |
| `err_beam_dyn …` | 同上但先 `push(1.0)` |
| `err_beam_stat_on s…` | `beamSwitches.push(0)`，再 push 每个 `stoi(w[i])`；同样补到 30/60；若任一开关非 0 则 `flags[0]=1` |
| `err_beam_dyn_on s…` | 先 `push(1)`；非 0 时 `flags[1]=1` |
| `err_quad_ncpl_stat …` | `elemDefs.push([0, 0, E, stod(w1), stod(w2), …])`，补零到 20 个 double |
| `err_quad_ncpl_dyn …` | `[0, 1, E, …]` |
| `err_cav_ncpl_stat …` | `[1, 0, E, …]` |
| `err_cav_ncpl_dyn …` | `[1, 1, E, …]` |
| `err_quad_stat_on s…` | `elemSwitches.push([0, 0, s…])` 补零到 20 个 int；任一非 0 → `flags[2]=1` |
| `err_quad_dyn_on` | `[0, 1, s…]`；非 0 → `flags[3]=1` |
| `err_cav_stat_on` | `[1, 0, s…]`；非 0 → `flags[2]=1` |
| `err_cav_dyn_on` | `[1, 1, s…]`；非 0 → `flags[3]=1` |

注意：
- 开关行的数值用 `stoi` 读两遍（一次判非零，一次存值），写 `1.0` 会被截成 1，写 `abc` 抛 `invalid_argument("stoi")`。
- 元件误差定义行因此为：`def[0]` 类别（0 = quad 关键字，1 = cav 关键字），`def[1]` 0 静态 / 1 动态，`def[2]` 起始元件序号 E，`def[3]` = N，`def[4]` = r（分布类型），`def[5+c]` = 第 c 列误差值（c 从 0 起）。
- `err_beam_stat` 在 schema 中第一个参数标为“保留”，**内核实际把它当作分布类型 r**（与 `err_beam_dyn` 同位置）。
- `beamVals` 不补齐（见 §3.3 的越界问题）。

### 2.4 superpose 组内的误差行（确定，细节部分推测）

在 `superpose … superposeout` 块内部（InitLattice 的子循环）出现的 `err_cav_ncpl_stat/dyn`、`err_quad_ncpl_stat/dyn` 不进普通的 `elemDefs`，而是累积到块的局部向量，每条生成一个 20 个 double 的记录：

`rec = [2.0, mode(0 静/1 动), E, w1, w2, …]`，补零到 18 个，再追加 `rec[18]` = 1.0（cav 关键字）或 0.0（quad 关键字），`rec[19]` = 该错误行之后的**子场序号**（块内子元件计数，从 0 起）。整块的记录拼成一行（长度为 20 的倍数），在块结束时 push 进 `elemDefs`。`E` 等于整个 superpose 组将得到的 Field 元件序号。

## 3. GenerateErrorSet：误差抽样

### 3.1 随机数来源（确定）

- 只用 C 库 `rand()`（glibc）和 `GenerateGaussian`。**GenerateErrorSet 在任何 `srand` 之前执行**：内核中 `srand` 只出现在 `KV2D/WB2D/PB2D/GS2D`（`randomseed≠0` 时），它们在 `InitParticle` 里才被调用。所以在新进程中误差序列等价于 `srand(1)` 的默认序列，**与 randomseed 无关**；若同一进程之前跑过模拟，则接着进程全局的 rand 状态。
- 误差抽样与初始分布共用同一个 rand 状态：GenerateErrorSet 消耗的 rand 次数会改变 `randomseed 0` 时后续初始分布的随机数（randomseed≠0 时分布生成器会重新 srand，不受影响）。
- 均匀数：`u = (double)(rand() % 10000) / 10000.0`，取值 {0, 0.0001, …, 0.9999}。
- 高斯数 `GenerateGaussian(μ, σ)`（Box–Muller，带静态缓存）：静态 `generate` 取反；为 0（偶数次调用）时返回缓存 `z1·σ + μ`；否则循环 `{u1 = rand()·4.656612875245797e-10; u2 = rand()·4.656612875245797e-10;} while (u1 ≤ 2.2250738585072014e-308)`（每轮两次 rand），`z0 = sqrt(−2 ln u1)·cos(2π u2)`，`z1 = sqrt(−2 ln u1)·sin(2π u2)`，返回 `σ·z0 + μ`（实现为 `σ * r * cos + μ`）。缓存跨调用保留。
- Windows `AVAS.dll` 用 MSVC `rand()`（RAND_MAX 32767，高斯里用 `rand()*3.051850947599719e-05`），序列不同，不能逐位对比。

### 3.2 样本模板（确定，以 .dll 结构为准、.so 分支一致）

设 `n_s = a·b`，`k = j·a + i`（`j ∈ [0,b)` 为“幅度级”，`i ∈ [0,a)` 为同一幅度下的重复）。束流行 `B26` = 26 个 0，`B1` = 1 个 0；元件行：普通元件 12 个 0；`field` 且 `stoi(w4)==4`（superpose 组）为 `(⌊nwords/15⌋·3 − 3)·4 = (⌊nwords/15⌋ − 1)·12` 个 0。`T(beam) = [beam, 元件行 0 … nElements−1]`。

| flags | dynSamples（n_s 份） | statSamples（n_s 份） |
|---|---|---|
| f2 且 f3 | T(f0∨f1 ? B26 : B1) | 同左 |
| f2，非 f3，f1 | [B26]（只有束流行） | T(B26) |
| f2，非 f3，非 f1，f0 | 空 | T(B26) |
| f2，非 f3，非 f0/f1 | 空 | T(B1) |
| 非 f2，f3，f0 | T(B26) | [B26] |
| 非 f2，f3，非 f0，f1 | T(B26) | 空 |
| 非 f2，f3，非 f0/f1 | T(B1) | 空 |
| 非 f2/f3，f0∨f1 | [B26] | [B26] |

`main_agent` 只在 `f1∨f3` 时复制 dyn、在 `f0∨f2` 时复制 stat 到 ErrorSample。

### 3.3 束流误差抽样（确定）

`sw = beamSwitches`，`v = beamVals`，`A_j = ((j+1)·x)/b`。

- **静态**（`flags[0]==1`）：开关块 `sw[0]==0` → 用块 0（`i_s = 1…29`，偏移 0），否则块 30（`i_s = 31…59`，偏移 30）。数值偏移 `vo = (v[0]==0.0) ? 0 : 30`。对每个 `sw[i_s]==1`：列号 `c = i_s − 偏移`（1…29），`r = v[vo+1]`，`x = v[vo+1+c]`；对 `j=0…b−1`、`i=0…a−1`（j 外层）：
  - r==0：值 = x（固定值）；r==1：值 = u·A_j（**[0, A_j) 的单边均匀分布**）；r==2：值 = 0（束流不支持高斯）；其他：输出 `Specify an unknown error distribution and check the type of error distribution` + 换行，`exit(1)`。
  - 写 `statSamples[k][0][c] = 值`，`statSamples[k][0][0] = 1.0`。
- **动态**（`flags[1]==1`）：块选择相反：`sw[0]==1` → 块 0，否则块 30；`vo = (v[0]==1.0) ? 0 : 30`。r==0：值 = A_j（**等步长**）；r==1、r==2 同静态；未知类型输出的字符串前面多两个字节 `0xD6 0xB8`（GBK“指”），然后 exit(1)。写入 `dynSamples[k][0]`。
- 列顺序 c=1…13：dx(mm)、dy(mm)、dφ(°)、dx′(mrad)、dy′(mrad)、dE(MeV)、dεx(%)、dεy(%)、dεz(%)、mx、my、mz、dIb(mA)（与 schema `BEAM_ERRORS` 一致）。c=14…29 的开关若置 1 会写到 26 长度行之外（越界）。
- rand 调用次序：静态列 c 升序 → j → i；然后动态同样。

**已知缺陷（由代码直接推出）**：
1. `beamVals` 不补齐到 30：若同时写了 `err_beam_stat` 和 `err_beam_dyn`，第二条的数值实际从 `1+len(第一条)` 开始，而读取用偏移 30，读到越界数据（推测为垃圾值或崩溃）。单写一条时正常。
2. 开关块按“出现先后”占位，且 `err_beam_stat/dyn` 数值行也会把开关向量补零到 30：若 `err_beam_stat` 数值行写在 `err_beam_stat_on` **之前**，块 0 是 30 个 0（`sw[0]=0`），静态读块 0 → 全部关闭，静态束流误差不起作用；动态因为判 `sw[0]==1` 否则读块 30，不受此影响。
3. 数值少于打开的列数时读越界（schema“后续空置参数默认不设置误差”只对元件行成立，元件行补零到 20）。

### 3.4 元件误差：作用对象（确定）

对每条定义 `def`，作用范围是元件序号 `[def[2], min(def[2]+N, nElements))`，**N 计入所有元件**（漂移段、校正铁等都占名额），但只有类型匹配的元件成为目标：

| 定义 | 目标元件 |
|---|---|
| quad 静态 `(0,0)` | 仅 `field` 且 `stoi(w4)==3`（静磁场图） |
| cav 静态 `(1,0)` | 仅 `field` 且 `stoi(w4)==1`（射频场图） |
| quad 动态 `(0,1)` | `field` 且 w4==3，或 `quad`、`solenoid`、`bend`、`edge` |
| cav 动态 `(1,1)` | `field` 且 w4==1 |
| superpose 记录 `(2, …)` | 只处理 `rec[1]==1`（动态）的记录：`rec[18]==0` 归 quad 动态，`==1` 归 cav 动态；目标固定为 `rec[2]` 号元件的第 `rec[19]` 个子场，**不用 N** |

因此：静态元件误差只作用于场图元件；静电场图（w4==2）、`steerer`、`drift`、RFQ 永远不会被选中；superpose 组内的静态误差行被解析但没有任何效果。

开关行：每种 `(类别,模式)` 只用 **最后一条** 开关行（循环覆盖下标）。开关列 `s = 2…len−1`（列号 `c = s−2`），值取 `def[5+c]`，写入目标元件行的 `[c+1]`，并置该行 `[0]=1.0`。

### 3.5 元件误差：分布（确定）

`A_j = ((j+1)·x)/b`，`x = def[5+c]`，`r = def[4]`：

| r | 静态 | 动态 |
|---|---|---|
| 0 | x（固定） | A_j（等步长） |
| 1 | u·A_j，范围 [0, A_j) | `d = (u − 0.5)·A_j; 值 = d + d`，范围 [−A_j, A_j) |
| 2 | 输出 `Unknown error distribution type, check the error distribution type` + 换行，exit(1) | `GenerateGaussian(0, A_j)`（σ = A_j，**不截断**） |
| 3 | 同上报错退出 | 同上报错退出 |
| 其他（如 −1） | 不赋值，沿用上一次的值（首次为未初始化栈值，未定义行为） | 同左 |

schema 写的 “−1 = 等步长（TraceWin 的 0 类型）”在内核中**没有对应**；内核的“等步长”是动态 r=0。“截断”在内核中不存在。

### 3.6 元件误差：写入位置与 rand 次序（确定）

- 普通定义：`samples[k][e+1][c+1] = 值`，`samples[k][e+1][0] = 1.0`（静态写 statSamples，动态写 dynSamples）。
- superpose 记录：`dynSamples[k][rec[2]+1][rec[19]·12 + c + 1] = 值`，`[rec[19]·12] = 1.0`。superpose 的 `x` 取 `rec[5+c]`、`r` 取 `rec[4]`（相对于记录起点）。
- 次序：先静态（quad 开关行，再 cav 开关行），后动态（quad 开关行，再 cav 开关行）。每个开关行内：列 c 升序 → 定义（`elemDefs` 中出现顺序）→ 目标元件（序号升序；superpose 为记录序）→ j（0…b−1）→ i（0…a−1）。动态的每一列先处理全部普通定义，再处理 superpose 记录。
- 同一元件被多条定义覆盖时，后处理的覆盖先处理的（不是相加）。

### 3.7 a 与 b 的含义（确定，与 schema/Python 相反）

内核中幅度按 `(j+1)/b` 递增，`j` 遍历 **b**（第二个参数），`i` 遍历 a（第一个参数）。所以对内核而言 **b 是幅度级数、a 是每级的重复次数**；schema 写的 “a 分组数、b 每组运行次数”和 Python `Error.get_group_time`（`all_group = a`，幅度 ∝ group/all_group）都与内核相反。GUI/Python 的误差研究总是给内核写 `err_step 1 1` 并把已经抽好的值写成动态 r=0，所以实际运行不受影响（见 §11）。

## 4. 误差作用到元件：SetErrPara

`Lattice::SetErrInEle(es)` 依次对 `elements[0…n−1]` 调用虚函数 `SetErrPara(es)`（vtable +0x20）。元件 i 读取 `es.dyn[i+1]` / `es.stat[i+1]`。行的列定义（下标从 0）：

- 静磁 / quad 型：`[0]` 标记（1.0 = 本组该元件有误差）、`[1]` dx(mm)、`[2]` dy(mm)、`[3]` θx(°)、`[4]` θy(°)、`[5]` θz(°)、`[6]` dG(%)、`[7]` dz(mm)。
- 射频腔型：`[1]` dx、`[2]` dy、`[3]` θx、`[4]` θy、`[5]` dK(%)、`[6]` φ(°)、`[7]` dz。

### 4.1 Quadrupole / Solenoid / Bend / Edge（确定）

只在 `flags[3]==1`（元件动态）时执行，**全部是在原值上累加 / 累乘**：
```
dx += row[1]·0.001;  dy += row[2]·0.001;  dz += row[7]·0.001
rotX += row[3]·π/180; rotY += row[4]·π/180; rotZ += row[5]·π/180   (π = 3.141592653589793，先乘 π 再除 180)
strength(+0x58) = (row[6]·0.01 + 1)·strength;  fieldErr = 同值
```
字段偏移：Quadrupole dx/dy/dz 0xc8/0xd0/0xd8，rot 0xe0/0xe8/0xf0，gradErr 0xc0；Solenoid 0x90…0xb8，Berr 0x88；Bend 0xd8…0x100，fieldErr 0xd0；Edge 0xc8…0xf0，fieldErr 0xc0。

后果：
1. **跨样本累积**：`main_agent` 的误差循环只调用一次 InitLattice，所有样本共用同一个 Lattice，第 k 组的四极梯度是前 k 组 (1+dG) 的连乘、错位是累加（Field 元件除外，见 §4.2）。内核单次运行 `err_step a b`（a·b>1）时的这些元件结果因此不等于“各组独立误差”。Python 每组单独调用一次内核，不受影响。
2. Bend、Edge 的 `strength`（+0x58）从未被 SetParameter 赋值（恒为 0），dG 对它们无效。
3. 错位/转角写进了元件，但 `Quadrupole::GetErrField` 不使用它们（§6.2）；二次粒子的 z-code（§9.4）也不使用。主束 z-code `HardEdgeTrans` 带 ErrorSample 参数，是否用到这些错位属其他子系统，本文未核对。

### 4.2 Field（场图元件，确定）

每次调用先把 `errDx…errRotZ, keErr, kbErr, phaseErr`（0xb0–0xf0）全部清零（所以 Field **不**跨样本累积）。`es+0x4c`（“errorUsed”模式）：0 = 静态+动态；1 = 只用静态；其他值输出 `errorUsed` 并 exit(1)。`main_agent` 总是写 0，模式 1 在 .so 正常流程中不可达。以下为模式 0；`S = es.stat[i+1]`，`D = es.dyn[i+1]`：

- **fieldType 1（射频）**：
  - 若 f2：`dx=S1·0.001, dy=S2·0.001, dz=S7·0.001, rotX=S3°, rotY=S4°`（rad），`keErr=(S5·0.01+1)·Ke`，`kbErr=(S5·0.01+1)·Kb`，`phaseErr=S6`。
  - 若 f3：上述错位再加 `D` 的同列；`keErr = (D5·0.01 + 1 + S5·0.01)·Ke`（f2 时）或 `(D5·0.01+1)·Ke`（无 f2），kbErr 同理；`phaseErr += D6`（度，在 GetErrField 中加到腔相位上）。射频腔没有 rotZ。
- **fieldType 2（静电）**：错位同上，另有 `rotZ = S5°`（+D5）；`keErr = (S6·0.01+1)·Ke`，有 f2 且 f3 时 `(D6·0.01 + 1 + S6·0.01)·Ke`；kbErr 保持 0。
- **fieldType 3（静磁）**：同 2，但写 `kbErr`（用 Kb），keErr 保持 0。
- **fieldType 4（superpose 组）**：`subErr` 调整为子场个数 × 12，全部清零，然后按子场类型预置：类型 1 → `subErr[s][7] = subKb[s]`（0x180）；类型 2 → `[7]=subKb, [8]=subKe`（0x168）；类型 3 → `[8]=subKe`。再对每个子场 s 读行中第 s 个 12 槽（`p = row + 12s`），`p[0]==1.0` 时：`subErr[s][0]=1`，`[1]+=p1·0.001`(dx)，`[2]+=p2·0.001`(dy)，`[3]+=p7·0.001`(dz)，`[4]+=p3°`，`[5]+=p4°`；类型 1/3：`[6]+=p5°`(θz)，`[7]` 或 `[8] = (p6·0.01+1)·subKb/subKe`；类型 2：`[7]=(p5·0.01+1)·subKb, [8]=(p5·0.01+1)·subKe`（f2 且 f3 时加上静态行的 p5），`[9] += p6`（相位）。GenerateErrorSet 从不写 superpose 的静态槽，所以实际只有动态生效。
  由 GetField 中类型 3 的子场电场乘 1e6 判断：子场类型 1 = 静磁（用 Kb），2 = 射频，3 = 静电（用 Ke）——与顶层 fieldType（2 静电、3 静磁）编号不同（推测，见 §13）。

### 4.3 其余元件（确定）

`Element::SetErrPara`、`Drift::SetErrPara` 为空；RFQ、Steerer 用基类空实现：不支持误差。

## 5. 束流误差作用到初始束流（InitParticle 中与误差有关的部分）

条件均为 `flags[0] + flags[1] != 0`，**只读 `es.dyn[0]`（动态行）**（确定）：

- **按 twiss 生成束流时**（"Starting to generate initial bunch" 分支），若 `dyn[0][0]==1.0`：
  - `I(A) = I(mA)/1000 + dyn[0][13]/1000`（dIb，mA）。
  - twiss 向量 `[εx,βx,αx, εy,βy,αy, εz,βz,αz]`：`εx *= 1+R7/100`，`εy *= 1+R8/100`，`εz *= 1+R9/100`；`βx, αx *= (1+R10/100)²`；`βy, αy *= (1+R11/100)²`；`βz, αz *= (1+R12/100)²`。**mx/my/mz（手册未说明）就是失配因子：β 与 α 同乘 (1+m%)²**。
  - 生成粒子后（.dst 单位：x cm、x′ rad、φ rad、W MeV），对每个粒子 `x += R1·0.1`（mm→cm）、`x′ += R4·0.001`、`y += R2·0.1`、`y′ += R5·0.001`、`φ += R3·π/180`、`W += R6`。这一步**不检查** `dyn[0][0]`。
- **读 .dst 初始分布时**：流强（mA）`+= R13`，坐标偏移同上（×0.1、×0.001、π/180、+dE）。
- **secondarybeam 下读 .dst 时**：坐标偏移位置用 `×0.001`（其余相同）——与另两个分支单位不一致（推测为缺陷，§13）。
- 其他读入分支（非 .dst 文本分布）未见误差处理。

后果（推测，由 §3.2 模板推出）：静态束流误差（err_beam_stat）**永远不会作用到束流**——静态值只写在 statSamples 而 InitParticle 只读 dyn 行；当 `f0` 开、`f1/f3` 都关而 `f2` 开时，ErrorSample 的 dyn 为空向量，InitParticle 解引用空指针，预计段错误。

## 6. t-code 推进中取误差场

### 6.1 Lattice::GetErrExternalField(t, Beam&, ErrorSample&)（确定）

对 `i = 0 … N−1`：
1. `z = p.z`。若 `z < 0`，或 `z > spOut[spOutCur][6]`（当前 superposeout 出口面），或 `p.lost ≠ 0`：若 `z > 总长 elemZEnd.back()` 且 `lost==0` 则 `lost = 2`；`extE = extB = 0`；下一个粒子。
2. 更新 `p.elemIndex`：`z < elemZStart[idx]` 时向后退（退到 <0 时打印 `Warning! beam.particle[i].indexElem<0`、置 0），`elemZEnd[idx] ≤ z` 时向前进。
3. 行选择：`flags[2]==1` → `es.stat[idx+1]`，否则 `es.dyn[idx+1]`。`row[0] == 1.0` → 调 `GetErrField(t, p, fm)`（vtable +0x08），否则（含 NaN）调 `GetField`（+0x10）。

同步粒子 N：只判 `z<0`、`z>总长`、`lost≠0`（不判 superposeout），然后更新序号并**总是调用 `GetField`**（同步粒子不受元件误差影响）。

推论：静态与动态同时打开时，用的是 **静态行的标记**。某场图元件只有动态误差时，它的 SetErrPara 已经写入动态错位/幅度，但 t-code 仍调用不带误差的 `GetField`，动态误差被忽略（Quadrupole 例外：它直接改 strength，GetField 也会看到）。

t-code 何时调用本函数：`tcodeRun` 在 `flags[2]+flags[3] ≠ 0` 时使用误差版本（单线程调本函数，多线程 `TcodeTrans` 内联相同逻辑，已核对 `TcodeTrans` 中 `*(lVar14+0xc)+iVar8 == 0` 分支）。

### 6.2 各元件 GetErrField（确定）

| 元件 | 行为 |
|---|---|
| Quadrupole | `extE=0`；`Bx = G·y`，`By = G·x`，`Bz = 0`，G 为（已乘 1+dG 的）strength；**不使用错位/转角** |
| Solenoid | `Bz = strength`（已乘 1+dG），其余 0 |
| Bend、Edge、Steerer | 六个分量全 0 |
| Drift | `sqrt(x²+y²) ≥ R` 且 `boundaryMode==0` 时 `lost=1`；场全 0 |
| Element（基类，RFQ 用） | 空函数（不写场） |
| Field | 见 §6.3 |

### 6.3 Field::GetErrField（确定，fieldType 4 部分推测）

**坐标变换（类型 1–3）**：先平移再依次绕 z、y、x 旋转（`s,c` 为 `sincos` 结果）：
```
X0 = x − errDx;  Y0 = y − errDy;  Z0 = (z − errDz) − zStart
X1 = cz·X0 + sz·Y0;          Y1 = cz·Y0 − sz·X0              (θz = errRotZ)
xl = cy·X1 − sy·Z0;          T  = sy·X1 + cy·Z0              (θy = errRotY)
yl = sx·T + cx·Y1;           zl = cx·T − sx·Y1               (θx = errRotX)
r  = sqrt(xl² + yl²)
```
- 若 `0 < zl < L` 且 `r ≥ R`：`p.lost = 1`，返回（**不看 `boundaryMode`、不看 superposeout 条件**，与 `Field::GetField` 不同：后者要求 `boundaryMode==0`、`zStart+1e-6 < spOutZ1`、`z > spOutZ0`）。
- 否则若 `xmin < xl < xmax`、`ymin < yl < ymax`、`0 < zl < zlen`：三线性插值（下式）；不在网格内则 `extE = extB = zeroField = (0,0,0)`。

**插值**：`u = (xl−xmin)/dx`，`v = (yl−ymin)/dy`，`w = zl/dz`，对 `k = 0…7`：`ix = ceil(u) − (k&1 ? 1 : 0)`，`iy = ceil(v) − ((k&3) ≥ 2 ? 1 : 0)`，`iz = ceil(w) − (k ≥ 4 ? 1 : 0)`，节点 `n = (iy + iz·(ny+1))·(nx+1) + ix`（取整为 `(int)`），权 `wk = (1−|u−ix|)·(1−|v−iy|)·(1−|w−iz|)`。分量按 k 升序从 0.0 累加。

- **类型 1（射频）**：`E += ((wk·map[n][0..2])·1e6)·keErr`，`B += (wk·map[n][3..5])·kbErr`；`φ = (f·2π)·(t − cavTime[cav]) + ((cavPhase[cav] + phaseErr)/180)·π`；`E *= cos φ`，`B *= cos(φ − 1.5707963267948966)`。然后把局部场转回实验室系（严格的逆变换，先 θx、再 θy、再 θz）：
  ```
  Ey' = cx·Ey − sx·Ez;  Ez' = sx·Ey + cx·Ez
  Ex''= cy·Ex + sy·Ez'; Ez''= cy·Ez' − sy·Ex
  Ex_lab = cz·Ex'' − sz·Ey';  Ey_lab = sz·Ex'' + cz·Ey';  Ez_lab = Ez''
  ```
  B 同样变换。时间 t 为调用参数（主束为全局时间，二次粒子为粒子自身时间）。
- **类型 3（静磁）**：`B = Σ (wk·map[n][0..2])·kbErr`（静态场图三个分量存在 0..2 号槽，与 GetField 一致），`E=0`，B 按上式转回。
- **类型 2（静电）**：伪代码与反汇编（`0x29a12 xorpd; movsd %xmm7,0x50(%rsp)`、`0x2c311` 起的转回段）一致地显示：E 的累加**没有 ×1e6**，并且转回时三个分量都取一个值为 0 的栈变量，最终 `E = B = 0`（±0.0）。由于 §3.4 静电场图从不成为误差目标，这段在正常输入下不可达。
- **类型 4（superpose）**：若 `|x| ≥ R` 或 `|y| ≥ R` 或 `z ≤ zStart` 或 `z ≥ zEnd`：场全 0 返回（不判丢失）。否则场清零，对每个子场 s：`subErr[s][0]==1` 时用 `(dx,dy,dz)=subErr[1..3]`、`(θx,θy,θz)=subErr[4..6]`，以 `subZStart[s]` 为起点做同样的变换，否则 `zl = z − subZStart[s]`、`xl=x, yl=y`；若 `0 < zl < subLength[s]` 且 `r ≥ subRadius[s]` 则 `lost=1` 返回；网格内则按子场类型插值累加（射频子场：`E` 用 `subErr[8]·1e6`，`B` 用 `subErr[7]`，相位加 `subErr[9]`）。各子类型的分量与转回细节未逐项核对（§13）。

## 7. 误差样本输出：MethodMP::errSampleOut（确定）

在 `Run`/`SeRun` 开头调用，文件在当前 `outputFilePath`（误差循环中为 `output_<k>/`）：
- `flags[0]||flags[2]` → `StaticErrorData.txt`，写 `es.stat`；`flags[1]||flags[3]` → `DynamicErrorData.txt`，写 `es.dyn`。打开方式 `ios::out`（截断）。
- 每行对应样本的一行（第 1 行束流，之后每个元件一行），每个数用默认 `ostream<<double`（精度 6，`%g` 风格）后跟一个空格，行末换行（行尾带空格）。束流行 26 个数（无束流误差时 1 个 `0`），普通元件 12 个数，superpose 组 12×子场数。
- 文件中第 0 列为“已施加”标记 1/0；其余列含义见 §4 的列表（束流行见 §3.3）。

`ErrorLog.txt`（`errorOut`、`Initial::errorOut`、`MethodMP::errorOut`）不是误差研究专用，是通用错误日志：`outputFilePath + "ErrorLog.txt"` 以追加方式写 `strftime("%Y/%m/%d %X  ")` + `"   simulation stage error: "` / `"   Initialisation stage error: "` + 消息；细节归其他子系统。

## 8. 二次粒子的读入（InitParticle 的 secondarybeam 分支，确定）

- 条件：Initial+0（input.txt `secondarybeam`）非 0。文件名取 **beam.txt 的 `readparticledistribution`**（Initial+0x08，路径前加 `inputFilePath`）。取最后一个 `.` 之后的扩展名：没有 `.`（含默认值 `unknown`）→ 抛出 `The initial bunch distribution file cannot be opened, check particleDistributionPath`；扩展名为 `dst` → 按 .dst 读初始分布（"Start reading initial beam distribution..."，此时仍走 SeRun）；其他扩展名 → 按二次粒子文本读（"Start reading secondary particle distribution..."）。
- **内核二进制中没有 `SeParticle.txt` 这个字符串**（.so 与 .dll 都检索过）：GUI（`avas/gui/services/beam.py`）假定“内核读 SeParticle.txt”，只有在 beam.txt 写了 `readparticledistribution SeParticle.txt` 时才成立。
- 文本格式：逐行 `getline` → `StringSplit`，**每行必须至少 11 个单词**：第 0–9 个用 `stod`，第 10 个为字符串（种类）。列：`x(m) y(m) z(m) vx vy vz(m/s) charge(e) mass(MeV) weight time(s) kind`（与 schema `SEPARTICLE_COLUMNS` 一致）。行数即粒子数 N；空行或单词不足会越界读（未定义行为）；非数字抛 `invalid_argument("stod")`。
- 每个粒子 i：
  ```
  |v| = sqrt(vz² + vy² + vx²)    （实现顺序：v4²+v3²+v5²，即 vy²+vx²+vz²）
  β = |v|/c,  γ = 1/sqrt(1−β²)
  x,y,z = 列 0,1,2；γβx = vx·γ/c，γβy = vy·γ/c，γβz = γ·vz/c   (c = 299792458.0)
  t = 列 9；dt = (seStep/β)/c     （seStep = steppercycle V2，默认 0.01）
  weight = 列 8；charge = weight·q·1.60217653e-19；mass = (weight·m_MeV·1.67262171e-27)/938.271875
  qOverM = charge/mass；restMassMeV = 列 7；kind = 列 10；id = i
  elemIndex = geomIndex = lost = nextPlane = spOutIndex = 0；boundaryMode = input `boundary`
  E/B/scE/scB/totE/totB 清零
  ```
  `|v|=0` 时 dt 为无穷大。
- 同步粒子（下标 N）：由 beam.txt `kneticenergy`（W）与 `particlerestmass`（m）：`γ = W/m+1`，`β = sqrt(1−1/γ²)`，`γβz = γβ`，坐标与时间 0，`dt = seStep/(β·c)`，`charge = q·e`，`mass = m·1.67262171e-27/938.271875`，`weight = 1`，`kind = kindofparticle`。
- Beam：`n = N`，`frequency = 0`，`current = 0`，`bunchTime = tMin = 0`，`seStep` 拷入；`numofgrid` 强制 `{3,3,3}`；空间电荷由 `Initial::Read` 在二次束时清零。

## 9. MethodMP::SeRun 流程

### 9.1 初始化（确定）

1. 构造 Mesh、PIC（不参与后续推进：二次粒子**没有空间电荷**）。
2. `errSampleOut`；有元件误差时 `SetErrInEle`；四个标志全 0 时 `es+0x48 = 1`。
3. 打印 `The total distance simulated is <总长> (m)` 与开始时间。
4. 以 `out|trunc` 打开 `DataSet.txt`、`synParticle.txt`、`SeSynParticle.txt`、`testP.txt`、`collisionData.txt`（全部在 outputFilePath），SeRun 结束时关闭。**DataSet.txt 在二次粒子模式下保持为空**；`SeSynParticle.txt` 由 §10.2 追加写入。
5. 若 `dumpperiodicity ≠ 0`：写二进制 `BeamSet.plt` 头：`char 125, char 100, int dumpperiodicity, int N, double current·1000, double frequency/1e6, double 同步粒子 restMassMeV`（二次模式下 current、frequency 都是 0），打不开时抛 `BeamSet.plt open failed, Check permissions`。此后不再写帧。

### 9.2 分段（确定）

按元件顺序切成交替的段：z-code 段（类型 2）由连续的 `quad / solenoid / bend / edge` 组成，t-code 段（类型 1）由其余元件组成。遍历中若下一个元件是 `steerer`，则跳过它并置“前面有校正铁”标志：
- z-code 段：段内元件序号表；每个元件一个标记：若前面紧跟校正铁则标记为校正铁序号；否则若有输出面 `zp` 满足 `zStart < zp < zEnd` 记 `−1`，若有输出面恰等于 `zEnd` 记 `−2`，都没有不记；标记数少于元件数时补 `−3`。
- t-code 段：输出面表（`zStart < zp ≤ zEnd` 且大于表中最后一个的输出面，标记 `−1`）；若前面紧跟校正铁，先插入“首元件中点 `zStart+L/2`”，标记为校正铁序号；段末：若段终点不等于表中最后一个面，追加段终点（标记 `−2`），相等则不追加。
- 第一段必须是 t-code，否则抛 `Do not allow the first element to be a hard-edged model element; very short drift can be added at the beginning`。

推论（推测）：校正铁对二次粒子**不起作用**——它不在任何段里，标记在 SeTRun/SeZRun 中都没有被当作校正铁处理。

### 9.3 逐粒子推进（确定）

- 先用同步粒子（`isSync = 1`）依次走所有段（SeTRun / SeZRun），打印 `Synchronized particles have completed simulation.`，然后 `maxSecondaryCount (MethodMP+0x298) *= 10`。
- 再对 i = 0…N−1：新建 8 个 double 的状态向量（全 0），依次走所有段（`isSync = 0`），每个粒子结束打印 `printf("Simulating the %d particle.  total number of particles %d.                          \r", i, N)`。
- 最后统计 `lost ≠ 1` 的粒子数（lost==2，越过末端，算作未丢失），打印 `Simulation completed, completion time …   Number of particles not lost: <n>` + 77 个空格。

### 9.4 SeTRun：t-code 段（确定）

参数：段标记表、段输出面表 `P`、状态向量 `S[8]`（`S[0..5]` = x, γβx, y, γβy, z, γβz，`S[6]` = 时间，`S[7]` = 有效标志）、粒子、isSync。`lost≠0` 直接返回。每一步 `iter++`：

1. 二次粒子：`iter > maxSecondaryCount` → `lost = 1`，返回（即二次粒子在某段内的步数超过同步粒子“单段最大步数”的 10 倍即判丢失）。同步粒子：每步先调用 `SeSynPout`（写的是上一步的场）。
2. 取场：若（`z ≥ 0` 且 `lost==0` 且 `z ≤ 总长`）或（当前元件 `inSuperposeOut ≠ 0` 且 `lost==0`）：
   - `z ≥ 0` 时更新元件序号；无元件误差（`flags[2]+flags[3]==0`）→ `GetField(p.t, …)`；有误差 → 与 §6.1 相同的行选择（`flags[2]` 选静态行，标记 1.0 用 GetErrField）。**同步粒子在这里也会受误差影响**（与主束不同）。时间参数用**粒子自身时间** `p.t`。`totE = extE`，`totB = extB`。
   - `z < 0`（仅 superposeout 情形）：`totE = totB = 0`。
   否则：`z > 总长` → `lost = 2`；`z < 0` → `lost = 1`；`totE = totB = 0`。随后把当前 6 个坐标存为“旧坐标”；若 `lost≠0` 返回。
3. 推进（Boris，`dt = p.dt`，`qm = p.qOverM`）：
   - `qm == 0`：`γ = sqrt(γβx²+1+γβy²+γβz²)`，不加速。
   - 否则：`h = (qm/c)·dt·0.5`；`γβ += h·E`（半步加速）；`γ = sqrt(γβx²+1+γβy²+γβz²)`；`τ = (qm·dt/γ)·B`（a,b,c 三分量）；`D = 1/((a²+b²+c²)·0.25 + 1)`；
     ```
     γβx' = [((a²−b²−c²)/4+1)·γβx + (ab/2 + c)·γβy + (ac/2 − b)·γβz]·D + Ex·h
     γβy' = [(ab/2 − c)·γβx + ((b²−c²−a²)/4+1)·γβy + (bc/2 + a)·γβz]·D + Ey·h
     γβz' = [(b + ac/2)·γβx + (bc/2 − a)·γβy + ((c²−b²−a²)/4+1)·γβz]·D + Ez·h
     ```
     （各项按上式从左到右求和。）再算 `γ`。
   - 位置：`f = (c/γ)·dt`；`t += dt`；`x += γβx·f`；`y += γβy·f`；`z += γβz·f`。
4. 输出面：`k = p.nextPlane`；若 `k ≤ |P|−1`、`P[k] ≤ z_new` 且当前元件 `inSuperposeOut == 0`：`s = (P[k] − z_old)/(z_new − z_old)`，`S[m] = old[m] + s·(new[m] − old[m])`（m=0…5），`S[6] = (s−1)·dt + t`，`S[7] = 1.0`，`nextPlane++`。**仅当 `nextPlane ≥ |P|`（越过表中最后一个面）时**：若该面的标记为 `−1` 调 `SePlaneOut(p, S, P[k], isSync)`；然后（同步粒子更新 `maxSecondaryCount = max(·, iter)`）返回。中间的输出面只插值、不输出（推测为缺陷，§13）。
5. superposeout：若 `p.spOutIndex < Lattice.spOutCount` 且当前元件 `inSuperposeOut ≠ 0`：取块 `B = spOut[spOutIndex] = {z0,x0,y0,θa,θb,θc,zExit}`，把新、旧位置分别变换到块坐标（平移 `(x0, y0, z0+元件zStart)`，依次用 `B[3]`、`B[5]`、`B[4]` 旋转，动量同样旋转），若新位置的局部纵坐标 `> 0`：按局部纵坐标线性插值到 0，粒子坐标与动量取插值后的局部值，`z = zExit`，`t += (s−1)·dt`；若 `zExit == 总长` 则把这些值写进 S（`S[6]=t, S[7]=1`）并 `SePlaneOut(总长)` 后返回；否则 `spOutZ1 = spOut[idx+1][6]`、`elemIndex++`、`spOutIndex++`、`spOutZ0 = zExit`。（与主束 tcodeRun 的 superposeout 处理对应，坐标约定以那边的规格为准。）
6. 若 input `boundary == 1`：`z < 0` 或 `z > P.back()` 或 `lost≠0` 时，若 `z > 总长`、`spOutCur == spOut.size()` 且 `lost==0` 则 `lost = 2`；否则按 `Boundary` 的段起止 z 更新 `geomIndex`，调用 `Geometry::Boundary(t, dt, geoms[geomIndex], p, 旧坐标, outputFilePath)`（碰撞判定与 collisionData.txt 属边界子系统）。

### 9.5 SeZRun：z-code 段（确定）

`lost≠0` 或 `S[7] ≠ 1.0` 时直接返回。对段内每个元件 e：若类型为 quad/solenoid/bend/edge → `SeHardEdgeTrans(e, zStart, zStart+L, S, p, lattice, isSync)`；然后 `r = sqrt(S[0]²+S[2]²)`，`r ≥ 元件 R` → `lost = 1` 返回（只在元件出口判一次，不看 `boundary` 设置）；标记为 `−2` → `SePlaneOut(p, S, zStart+L, isSync)`。结束后 `nextPlane = 0`；`S[7] ≠ 1` 则 `lost = 1`；粒子取 `t = S[6]`，`x,γβx,y,γβy,γβz = S[0,1,2,3,5]`，`z = 最后元件 zStart+L`。

### 9.6 SeHardEdgeTrans：硬边传输（确定）

记 `γ = sqrt(1+γβx²+γβy²+γβz²)`（来自 S），`β = sqrt(1−1/γ²)`，`x′ = S1/S5`，`y′ = S3/S5`，`v_z = sqrt(β²c²/(x′²+y′²+1))`。变换后 `S1 = x′·S5`，`S3 = y′·S5`（γβz、能量不变）。

- **quad**：`L = zEnd − zStart`，`k = sqrt(|G/((γ·c·β)/qm)|)`（G = strength，已含 dG），`S[4] = L/v_z`，`S[6] += L/v_z`。`G·qm ≤ 0`：x 用 `cosh/sinh`（散焦），y 用 `cos/sin`；否则 x 聚焦、y 散焦。矩阵 `[cos, sin/k; −k sin, cos]` 与 `[cosh, sinh/k; k sinh, cosh]`，`φ = k·L`。
- **solenoid**：`K = (B·qm)/(β·599584916.0·γ)`（599584916 = 2c），`θ = K·L`，`C=cos θ, S=sin θ`：
  ```
  x  = x·C² + x′·SC/K + y·SC + y′·S²/K
  x′ = −x·K·SC + x′·C² − y·K·S² + y′·SC
  y  = −x′·S²/K − x·SC + y·C² + y′·SC/K
  y′ = x·K·S² − x′·SC − y·K·SC + y′·C²
  ```
  时间同 quad。
- **edge**：`Edge::GetParameter` 给出 `[hv, tanβ/|ρ|, −tan(β−ψ)/|ρ|, …]`；hv=0：`x′ += p1·x`，`y′ += p2·y`；hv=1：`x′ += p2·x`，`y′ += p1·y`。零长度，不改时间。
- **bend**：同步粒子调用时先把自己的 γβz 放进参数向量，`Bend::GetParameter` 把它存进 Bend+0xc8（参考 γβz）；二次粒子得到 `[hv, arc=ρ|α|, h, kx, ky, refγβz]`。若 `zEnd−zStart < 0.85·arc`，arc 取一半（推测为对“分成两半的弯铁”的处理）。`δ = (S5 − refγβz)/refγβz`，`θx = arc·kx`、`θy = ky·arc`；hv=0（水平）：
  ```
  x  = x·cosθx + x′·sinθx/kx + h·δ·(1−cosθx)/kx²
  x′ = −x·kx·sinθx + x′·cosθx + h·δ·sinθx/kx
  y, y′：ky==0 时为长 arc 的漂移；否则 [cos, sin/ky; −ky sin, cos]
  ```
  hv=1 把 x、y 互换。另算一个纵向量写进粒子 z（随后被 SeZRun 覆盖，无效）。时间 `S[6] += arc/v_z`。

二次粒子的 z-code **不使用元件错位/转角**（SeHardEdgeTrans 没有 ErrorSample 参数），只有通过 SetErrPara 改变的 strength（四极、螺线管的 dG）起作用。

## 10. 二次粒子输出

### 10.1 MethodMP::SePlaneOut(p, S, z, isSync)（确定）

文件名 `outputFilePath + "outData_" + sprintf("%f", z) + ".txt"`（`%f` 即 6 位小数，如 `outData_1.234000.txt`）。
- isSync==1：以 `out|trunc` 打开后立即关闭——同步粒子先跑，所以每个输出面的文件在二次粒子写入前被清空。
- isSync==0：以 `out|app` 打开，写一行：`id`（int，默认格式）+ `" "`，然后设为 `fixed`、精度 15，依次 `S[0]  S[1]  S[2]  S[3]  S[6]  S[5]`（两个空格分隔），换行。即列为 **id, x(m), γβx, y(m), γβy, t(s), γβz**（不输出 z）。

### 10.2 MethodMP::SeSynPout(p)（确定）

`outputFilePath + "SeSynParticle.txt"`，`out|app`，`fixed` 精度 12：`x  γβx  y  γβy  z  γβz  Bx  By  Bz  `（每个后跟两个空格，B 为 `extB`），再写 `lost`（int），换行。同步粒子在 t-code 段每一步调用一次（z-code 段不写）。

### 10.3 其他

- `outData_*.dst`、`DataSet.txt` 内容、`beamOut`/`dstOut` 在二次模式下都不产生（SeRun 不调用它们）。
- `SeTrans`（160 字节）只分配并释放一个 8 个 0 的向量，`SeTcode`、`SeZcode` 是空函数：都是遗留的空壳，新内核不需要实现。

## 11. 与 Python 侧的关系（avas/sim/error.py、avas/api/basic.py）

- `_error_study`：把文本输入复制到 `<输出>/inputs/`，在那里重写 `lattice.txt`，场图仍从原目录读。
- `Error.run`：读 `err_step a b`（`all_group=a, all_time=b`）与开关；可选先跑无误差参考（`output_0_0`）；对 group=1…a、time=1…b 逐次：**每次都从项目 lattice 重新读取**并用 Python 自己的 `random.Random(seed)` 抽样（均匀 `uniform(−A, A)`、高斯 `gauss(0, A)`、−1 等步长，`A = 值·group/all_group`，保留 5 位小数），把所有误差写成每元件一条 N=1、r=0 的**动态** ncpl 命令（静态模式经 `stat_to_dyn_lines` 改名为动态，静+动模式把两条相加），`err_step` 改为 `1 1`，然后单独调用一次内核（输出到 `error_middle/output_0`，再移到 `error_output/output_<g>_<t>`）。
- 因此 GUI 的误差研究绕开了内核自己的抽样、静态/动态分支、a/b 含义差异和跨样本累积问题：内核只做“动态 r=0、b=1”的固定值施加（值 = `(0+1)·x/1 = x`）。这也意味着 GUI 下“静态”误差实际上会作用于四极等矩阵元件（因为被改成了动态）。
- Python 侧 `set_error_to_lattice` 对 quad 误差只挂 `field` 类型 3 和 `quad`，不挂 solenoid/bend/edge；内核动态 quad 误差会作用到这四类（Python 写的是 N=1 的逐元件命令，所以不会扩散到别的元件）。

## 12. 对齐清单

| ID | 行为 | 涉及关键字/文件 | 建议的比对测试 |
|---|---|---|---|
| G-01 | 内核忽略 `err_*_cpl_*` | lattice | 同一 lattice 加/不加 `err_quad_cpl_dyn 10 0 1 …`，DataSet.txt 逐位相同（固定 randomseed） |
| G-02 | 动态 r=0、`err_step 1 1` 时施加值 = 写入值 | err_quad_ncpl_dyn + err_quad_dyn_on | 单个 quad 写 dG=10%：DynamicErrorData.txt 该元件行 `1 0 0 0 0 0 10 0 …`；与把 G 手工乘 1.1 的无误差运行比较 DataSet（容差 1e-12 相对） |
| G-03 | 输出目录 `output_<k>`，k=j·a+i | err_step | `err_step 2 3`：检查生成 output_0…output_5；DynamicErrorData.txt 中 r=0 的值为 x/3、2x/3、x（每个各 2 个目录，k=j·2+i） |
| G-04 | b 为幅度级数 | err_step | 同上，确认幅度随 j=⌊k/a⌋ 变化而非随 i |
| G-05 | 误差序列与 randomseed 无关 | randomseed、r=1 | 两次运行 randomseed 1 与 2，r=1 动态误差：DynamicErrorData.txt 完全相同；与 glibc `srand(1)` 下按 §3 次序重算的值逐位相同 |
| G-06 | 静态 r=1 为 [0,A)，动态 r=1 为 [−A,A) | r=1 | 大 a·b（如 200）统计 DynamicErrorData/StaticErrorData 的最小最大值与 u 的 0.0001 量化 |
| G-07 | 动态 r=2 为 σ=A 的不截断高斯 | r=2 | 与按 GenerateGaussian（含缓存）重算的值逐位比较 |
| G-08 | 静态 r=2/3、动态 r=3 报错退出 | r | 检查 stdout 文本 `Unknown error distribution type, check the error distribution type` 与退出码 1 |
| G-09 | 束流 r=2 → 0；r 非 0/1/2 → 报错（动态带 0xD6B8 前缀） | err_beam_* | 检查 DynamicErrorData 第 1 行与 stdout 字节 |
| G-10 | 静态元件误差只作用于场图类型 3/1 | err_quad_ncpl_stat | 范围内含 quad 与 field(3)：StaticErrorData 中 quad 行全 0、field 行有值 |
| G-11 | 动态 quad 误差作用于 field3/quad/solenoid/bend/edge，N 计入所有元件 | err_quad_ncpl_dyn N | N=3 覆盖 drift,quad,drift：只有 quad 行有值；N=2 时 quad 仍被覆盖 |
| G-12 | 每种开关只用最后一条 | err_quad_dyn_on ×2 | 两条不同开关，结果只按第二条 |
| G-13 | 跨样本累积（四极/螺线管） | err_step 1 2 | 动态 r=0、dG=10%：output_1 的四极等效梯度为 G·1.05·1.1（可用单粒子运行的质心响应或 GetParameter 输出核对） |
| G-14 | Field 不累积；RF 的 ke/kb 与相位误差 | err_cav_ncpl_dyn | dK=5%、φ=2°：与手工把 Ke、Kb ×1.05、腔相位+2° 的无误差运行比较（同步相位按无误差求，见 §6.1） |
| G-15 | 场图错位/转角变换与孔径丢失 | err_quad_ncpl_dyn dx | 静磁场图 dx=1 mm：单粒子 x=1 mm 入射与无误差 x=0 入射轨迹一致（容差 1e-9 m）；大 dx 时在局部坐标判丢失，且 `boundary 1` 下仍丢失 |
| G-16 | 静、动同时开时 t-code 用静态行标记 | stat+dyn | field 只有动态误差、另一元件有静态误差：该 field 的动态误差不生效（DataSet 与去掉该动态误差相同） |
| G-17 | 同步粒子主束不受元件误差影响 | 任意元件误差 | synData.txt / 同步粒子行在有无误差时相同 |
| G-18 | 束流动态误差作用 | err_beam_dyn | dx=1 mm：inData.dst 中 x 平移 0.1 cm；dIb=1 mA；mx=10%：生成束的 βx、αx 乘 1.21（比较 inData.dst 的 rms） |
| G-19 | 静态束流误差不生效 / 可能崩溃 | err_beam_stat | ① 仅 err_beam_stat+on：inData.dst 与无误差相同；② 再加元件静态误差（无动态）：预计段错误 |
| G-20 | 束流开关顺序缺陷 | err_beam_stat 在 on 之前 | 交换两行顺序，比较 StaticErrorData 第 1 行 |
| G-21 | errSampleOut 文件格式 | StaticErrorData.txt | 逐字节比较（%g 6 位、每数后空格、行尾换行） |
| G-22 | 二次粒子文件名来自 readparticledistribution | beam.txt、secondarybeam | 只放 SeParticle.txt、`readparticledistribution unknown`：内核抛出 “The initial bunch distribution file cannot be opened…”；改为 `SeParticle.txt` 后正常 |
| G-23 | 二次粒子初始化 | SeParticle.txt | 单个粒子，检查第一个输出面 outData_*.txt 的 γβ 与由 v 计算的 γv/c（容差 1e-12） |
| G-24 | 二次粒子 t-code 推进（漂移） | 只有 drift | 粒子 v、t0 给定：输出面的 x、t 与解析 `x0+vx·Δz/vz`、`t0+Δz/vz` 比较（误差 ≤ dt 量级插值误差） |
| G-25 | 只在段末面输出 | outputplane | t-code 段中间插一个输出面：预计该面 outData 文件只有同步粒子清空后的空文件 |
| G-26 | outData_<z>.txt 格式 | 输出面 | 文件名 `%f`；列 id,x,γβx,y,γβy,t,γβz，fixed 15 位，两空格分隔 |
| G-27 | SeSynParticle.txt 格式 | 同步粒子 | fixed 12 位，9 个数各跟两空格，再 lost，换行；每个 t-code 步一行 |
| G-28 | 步数上限 = 同步粒子单段最大步数 ×10 | 慢粒子 | 速度为同步粒子 1/20 的二次粒子在第一段被判 lost=1 |
| G-29 | z-code 段四极/螺线管/弯铁/边缘的传输 | quad 等 | 单粒子小偏移：出口 x、x′ 与 §9.6 矩阵比较（1e-12）；时间 `L/v_z` |
| G-30 | z-code 出口孔径判断 | quad R | 出口 r ≥ R 判丢失，入口/中间不判 |
| G-31 | 校正铁对二次粒子无效 | steerer | 有无 steerer（非零场）时二次粒子输出相同 |
| G-32 | 二次模式下 DataSet.txt 为空、BeamSet.plt 只有头 | dumpperiodicity | 检查文件大小 0 / 头 2+4+4+8·3 字节 |
| G-33 | 第一段为硬边元件时报错 | lattice | 第一个元件为 quad：异常信息逐字比较 |

## 13. 函数覆盖表

| 函数 | 作用 | 正文 |
|---|---|---|
| ErrorParameter::~ErrorParameter | 释放 0x00–0xb0 各容器，给出布局 | §2.1 |
| ErrorSample::~ErrorSample | 释放 flags、dyn、stat | §4、§14 |
| Bend::GetErrField | 场全 0 | §6.2 |
| Bend::SetErrPara | 动态时累加错位、strength×(1+dG)（strength 恒 0） | §4.1 |
| Element::GetErrField | 空函数（RFQ 用） | §6.2 |
| Element::SetErrPara | 空函数（RFQ、Steerer 用） | §4.3 |
| Drift::GetErrField | 孔径判丢失（boundaryMode==0），场 0 | §6.2 |
| Drift::SetErrPara | 空函数 | §4.3 |
| Edge::GetErrField | 场全 0 | §6.2 |
| Edge::SetErrPara | 同 Bend（strength 恒 0） | §4.1 |
| Field::GetErrField | 带错位/转角的场图场与局部孔径判断 | §6.3 |
| Field::SetErrPara | 场图元件错位、幅度、相位误差（每次清零，不累积） | §4.2 |
| GenerateErrorSet | 生成全部误差样本 | §3 |
| Lattice::GetErrExternalField | t-code 中按行标记选 GetErrField/GetField | §6.1 |
| Lattice::SetErrInEle | 对每个元件调用 SetErrPara | §4 |
| SeTrans | 空壳（分配后释放 8 个 0） | §10.3 |
| SeTcode | 空函数 | §10.3 |
| SeZcode | 空函数 | §10.3 |
| MethodMP::errSampleOut | 写 StaticErrorData.txt / DynamicErrorData.txt | §7 |
| MethodMP::SePlaneOut | 写/清空 outData_<z>.txt | §10.1 |
| MethodMP::SeSynPout | 写 SeSynParticle.txt | §10.2 |
| MethodMP::SeTRun | 二次粒子 t-code 段逐步推进 | §9.4 |
| MethodMP::SeHardEdgeTrans | 二次粒子硬边元件传输 | §9.6 |
| MethodMP::SeZRun | 二次粒子 z-code 段 | §9.5 |
| MethodMP::SeRun | 二次粒子总流程、分段、逐粒子循环 | §9.1–9.3 |
| Quadrupole::GetErrField | Bx=G·y、By=G·x（不用错位） | §6.2 |
| Quadrupole::SetErrPara | 动态时累加错位、G×(1+dG) | §4.1 |
| Solenoid::GetErrField | Bz=strength | §6.2 |
| Solenoid::SetErrPara | 同 Quadrupole | §4.1 |
| Steerer::GetErrField | 场全 0 | §6.2 |

另为理解调用关系参考了（不属本文负责）：`Initial::InitLattice`（误差关键字与 superpose 记录）、`Initial::InitParticle`（束流误差、二次粒子读入）、`main_agent`、`MethodMP::Run`、`TcodeTrans`、`GenerateGaussian`、`KV2D/GS2D…`（srand）、`Bend/Edge::GetParameter`、`Field::GetField`、`errorOut`。

## 14. 未解决的问题与建议实验

1. **跨样本累积**（§4.1）：用 `err_step 1 3`、单个 quad、动态 r=0、dG=10%，单粒子离轴运行比较三个 output 的出口质心；预期等效 G 为 1.0333、1.0333·1.0667、…（r=0 时值为 x/3、2x/3、x 的连乘）。
2. **静态束流误差**（§5）：确认 ① 无效、② 只有 f0+f2 时崩溃。
3. **束流数值偏移 30 的越界**（§3.3 缺陷 1）：同时写 err_beam_stat 与 err_beam_dyn，看 Static/DynamicErrorData 第 1 行。
4. **SeTRun 只在段末面输出**（§9.4 第 4 步）：t-code 段中间放输出面，检查该面文件是否为空；若实际有内容说明我们漏读了分支。
5. **校正铁对二次粒子无效**（§9.2）：非零校正铁前后二次粒子 outData 是否相同。
6. **secondarybeam + .dst 时束流误差单位 ×0.001**（§5）：写 dx=1 mm，比较二次模式下 inData.dst 平移量（0.1 cm 还是 0.001）。
7. **superpose 子场类型编号与 subKe/subKb 的对应**（§4.2、§6.3）：在 superpose 组中分别放静磁、射频、静电子场并给动态 dG，检查哪一个子场的场幅被改变。
8. **Field::GetErrField 类型 2 返回 0**：正常输入不可达；若要验证需人为让静电场图成为目标（目前无法通过关键字做到），可略。
9. **未知 r（如 −1）用未初始化值**：用 r=−1 运行，看元件行值是否为前一次抽样值（同一开关列内）或随机垃圾。
10. **GenerateGaussian 截断与 σ**：用 r=2、x=1、a·b=1000 统计标准差与最大值。
11. **弯铁 `L < 0.85·arc` 取半长**（§9.6）：含义未确认，需结合 bend 在 lattice 中被拆成两半的写法（其他子系统）。
12. **ErrorSample+0x48** 在 Run 中“无误差时置 1”，main_agent 写 k+1；读取者未找到（可能被 dstOut/beamOut 用作样本号），需在主循环子系统中确认。

## 15. 对结构表的更正

1. **ErrorSample** sizeof 至少 0x50：`+0x48 int sampleNo`（main_agent 写 k+1；Run/SeRun 在四个标志全 0 时写 1），`+0x4c int errorUsedMode`（0 = 静态+动态，1 = 只用静态，其他值 `errorUsed` 后 exit(1)；见 Field::SetErrPara）。元件误差行的 `[0]` 列是“本组该元件已施加误差”标记（1.0/0.0），束流行 `[0]` 同理；不是分布类型。
2. **ErrorParameter** 完整布局见 §2.1（sizeof 0xc8）：0x00 errStep{a,b}，0x18 dynSamples，0x30 statSamples，0x48 beamVals，0x60 elemDefs，0x78 beamSwitches，0x90 elemSwitches（vector\<vector\<int\>\>，structures.md 写成 vector\<vector\<double\>\>），0xa8 nElements，0xb0 flags（vector\<int\>(4)，structures.md 写成 vector，未说明）。
3. **Bend +0xc8**（structures.md 记为 unk_c8）：参考 γβz。`Bend::GetParameter(v)` 在 v 非空时把 `v[0]` 存进 +0xc8 并以 hv 覆盖 `v[0]`，最后把 +0xc8 追加为第 6 个元素；SeHardEdgeTrans 中同步粒子借此登记 bend 入口的 γβz。
4. **Edge::GetParameter** 输出 `[hv, tanβ/|ρ|, −tan(β−ψ)/|ρ|, +0xc8(dx), …]`，+0xb8 ψ 被使用。
5. **Field 0x1e0 subErr** 每子场 12 个 double：`[0]` 标记，`[1..3]` dx,dy,dz(m)，`[4..6]` θx,θy,θz(rad)，`[7]` 由 0x180 缩放的幅度，`[8]` 由 0x168 缩放的幅度，`[9]` 相位误差(°)；射频子场 E 用 `[8]`、B 用 `[7]`，故 0x168 = subKe、0x180 = subKb 与结构表一致。superpose 子场类型编号推测为 1 静磁、2 射频、3 静电（与顶层 fieldType 不同）。
6. **Field 0x0f0 phaseErr** 单位为度（GetErrField 中 `(cavPhase + phaseErr)/180·π`），可去掉“推测”。
7. **Element 0x58 strength** 对 Bend/Edge 确实从未赋值（恒 0），SetErrPara 的 dG 对它们无效。
8. **MethodMP 0x298**：二次粒子单段步数上限，= 同步粒子各 t-code 段最大步数 ×10（SeRun 在同步粒子跑完后乘 10）。
9. **Particle 0x104 nextPlane、0x174 spOutIndex、0x108/0x110 spOutZ0/Z1**：在 SeTRun 中的用法与结构表推测一致，可改为确定（二次粒子路径）。
10. 误差关键字中 `err_quad_cpl_*` / `err_cav_cpl_*` 不存在于内核（结构表与 schema 未说明）。
