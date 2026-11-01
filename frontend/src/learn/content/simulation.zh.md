# 多粒子模拟要点

前面各章给出了元件的物理和线性理论；真正的设计结论要靠多粒子模拟——用大量宏粒子跟踪束流通过实际场图、包含空间电荷和非线性的束线。本章讲在 AVAS 中做一次**可信**的多粒子模拟需要注意什么：宏粒子数与统计误差、初始分布、时间步长、空间电荷网格、运行设置、输出平面与输出文件、如何判断收敛、线性预览与内核的差别、单粒子运行，以及最常见的失败原因。物理背景见 [空间电荷](#space-charge) 和 [纵向动力学](#longitudinal)，输入文件的整体结构见 [加速器与 AVAS](#intro)。

## 宏粒子与统计 {#simulation-macroparticles}

### 宏粒子代表什么

一个束团的真实粒子数是 $Q/q$（$Q = I/f$ 为束团电荷），通常是 $10^6$–$10^{10}$ 量级。内核用 `particlenumber` 个**宏粒子**代表它们：每个宏粒子的电荷为 $Q/N$，荷质比与真实粒子相同，因此外场下的运动不变，只有空间电荷按总电荷计算。示例项目 $Q = 1.133\ \mathrm{pC}$（$7.07\times 10^6$ 个质子），`particlenumber 5260`，每个宏粒子代表约 1340 个质子。

```avas-params
beam:particlenumber beam:randomseed beam:current beam:frequency
```

### 统计误差

宏粒子数有限，所有统计量都有随机误差。对高斯分布，用 $N$ 个样本估计 rms 尺寸的相对标准误差约为 $1/\sqrt{2N}$：

| $N$ | rms 尺寸的相对统计误差 |
|---|---|
| 5260（示例） | 0.97 % |
| $10^4$ | 0.71 % |
| $10^5$ | 0.22 % |
| $10^6$ | 0.07 % |

rms 发射度的统计误差与此同一量级。损失率的分辨率是 $1/N$：示例中丢失一个宏粒子就是 0.019 %，要研究 $10^{-4}$ 以下的损失或束晕，必须用远多于此的宏粒子，或者专门研究分布尾部。

另一个误差来源是**数值噪声**：宏粒子比真实粒子少得多，空间电荷场的涨落比真实束大，会造成虚假的发射度增长（数值加热）。它随每个网格单元里的宏粒子数增加而减小，见 [空间电荷网格](#simulation-sc-mesh)。

> 注意：内核多线程计算，同样的输入每次运行的结果会略有差别。比较两次运行（例如判断某个参数的影响）时要带容差，差别小于统计误差的结论不可靠。`beam.txt` 的 `randomseed` 是生成初始束流的随机数种子。

## 初始分布 {#simulation-distributions}

### 由 Twiss 参数生成

`twissx`、`twissy`、`twissz` 给出每个平面的 $\alpha$、$\beta$ 与**归一化 rms** 发射度（β 单位 mm/mrad，发射度 π·mm·mrad；$z$ 平面的角度是 $\delta = \Delta p/p$），见 [相空间与发射度](#phase-space)。`distribution V1 V2` 分别选择横向和纵向分布类型：KV、GS（高斯）、PB（抛物线）、WB（水袋）。

```avas-params
beam:distribution beam:twissx beam:twissz beam:readparticledistribution beam:initpos beam:initmom
```

同样的 rms 参数下，不同分布的边界和尾部差别很大。按通常的定义（四维相空间中的分布），某个方向投影的边界与 rms 尺寸之比为：

| 分布 | 四维相空间中的定义 | 边界 / rms |
|---|---|---|
| KV | 超椭球面上均匀 | 2 |
| WB（水袋） | 超椭球内均匀 | $\sqrt6 \approx 2.45$ |
| PB（抛物线） | 密度 $\propto 1 - r^2/r_{\max}^2$ | $2\sqrt2 \approx 2.83$ |
| GS（高斯） | 高斯 | 无界（实际按截断） |

> 注意：手册没有说明 AVAS 生成这些分布时使用的维数和高斯分布的截断位置。需要准确知道时，读取输出目录的 `inData.dst`（模拟实际使用的初始分布）统计。空间电荷强时，分布形状决定了初始阶段的发射度增长（见 [空间电荷](#space-charge-emittance-growth)），比较不同程序的结果时要保证分布一致。

### 读入粒子文件

`readparticledistribution` 指定已有的 `.dst` 或 `.edst` 文件（例如其他程序计算的 RFQ 出口分布）。使用粒子文件时，除 `numofcharge` 外 `beam.txt` 的其他关键字都不生效；`.edst` 可以描述多种粒子组成的混合束。`initpos`、`initmom` 给整个束团加位置和角度偏移，可用来研究初始偏心。

## 时间步长 {#simulation-stepping}

### t-code 的步长

场图元件和漂移段用 t-code 按时间积分运动方程（见 [加速器与 AVAS](#intro-tracking)）。`input.txt` 的 `steppercycle V1` 把一个射频周期分成 $V_1$ 步：

$$
\Delta t = \frac{1}{f\,V_1},\qquad \Delta s \approx \beta c\,\Delta t = \frac{\beta\lambda}{V_1} .
$$

示例 `steppercycle 50`、162.5 MHz、$\beta = 0.057$：$\Delta t = 0.123\ \mathrm{ns}$，每步前进约 2.1 mm；而 `hwr010` 场图沿 $z$ 的网格间距是 2 mm，两者相当。手册把公式写成 "Δt = 1/T × 1/V1"；第二个参数 V2 只用于次级粒子输运（$\Delta t = V_2/V_b$）。

```avas-params
input:steppercycle input:sim_type
```

选择步长的原则：

- 每步的距离要远小于场的变化尺度（场图网格、间隙长度、$\beta\lambda$）；
- 远小于横向振荡波长、同步振荡波长和等离子体波长（空间电荷每步计算一次，步长太大时空间电荷冲量的积分误差增大）；
- 用加倍步数的方法检验：结果不再变化（在统计误差范围内）时步长足够。

### z-code 元件

`quad`、`solenoid`、`bend`、`edge`、`steerer` 是矩阵元件，用 z-code 按纵向位置变换粒子坐标；t-code 按同步粒子的时刻记录束团，z-code 按位置记录（见手册对 `Beamset.plt` 的说明），两类元件交界处如何转换手册未说明，以内核为准。schema 对矩阵元件的说明是：多粒子 lattice 的**第一个和最后一个元件避免使用矩阵元件**，可以在两端各放一段短漂移。

## 空间电荷网格 {#simulation-sc-mesh}

`spacecharge 1` 时，内核在网格上求解束流自场。`numofgrid Nx Ny Nz` 是网格数，`meshrms V1 V2 V3` 设网格边长 $L_x = 2V_1\sigma_x$（$y$、$z$ 同理），网格单元的尺寸因此是

$$
h_x = \frac{2V_1\sigma_x}{N_x} .
$$

```avas-params
input:spacecharge input:scmethod input:numofgrid input:meshrms
```

三个相互制约的要求：

- **覆盖**：网格应盖住几乎全部粒子。$V_1$ 太小会截掉尾部；对高斯分布 $V_1$ 至少取 3–5。超出网格的粒子如何处理手册未说明。
- **分辨率**：束芯要有足够多的网格点，$h_x$ 应明显小于 $\sigma_x$；$V_1$ 越大，同样 $N_x$ 下分辨率越差。
- **每格粒子数**：平均每个单元的宏粒子数约为 $N/(N_xN_yN_z)$，束芯中更多一些。例如 $32\times 32\times 32$ 的网格配 5260 个宏粒子，平均每格只有 0.16 个，场的数值噪声很大；这样的网格需要 $10^5$–$10^6$ 个宏粒子。

`scmethod` 可选 `FFT`、`PICNIC`、`SPICNIC`，手册只列出名称。改变网格或算法后，按 [收敛性判断](#simulation-convergence) 的办法比较结果。

## 运行设置 {#simulation-settings}

```avas-params
input:multithreading input:dumpperiodicity input:boundary input:longlimits input:pchistogram input:scanphase
```

- **多线程**：写 `multithreading 1` 使用多线程，单线程就**不写这一行**。手册说 0 表示关闭，但内核遇到 `multithreading 0`（以及 2、3 等）会在起点丢失全部粒子（已用示例项目核实）。界面的设置页已按此处理。
- **损失边界**：粒子超出元件孔径 `R` 记为损失；`boundary 1` 时改用 `boundary.txt` 中的边界，lattice 中的半径不再生效，损失粒子写入 `CollisionData.txt`。`longlimits 1 相位 能量` 打开纵向损失判定，与 [纵向动力学](#longitudinal-separatrix) 中的分界线概念对应。
- **粒子数据**：`dumpperiodicity V1` 每 $V_1$ 步把全部粒子写入 `Beamset.plt`（0 表示不写）；文件可能很大。`pchistogram 1 份数` 生成 `pchistogram.dat`。
- **相位扫描**：`scanphase` 控制射频腔的相位扫描或从 `scanData.txt` 读取相位（见 [纵向动力学](#longitudinal-avas)）。
- **模型**：`sim_type mulp` 是多粒子模型，`env` 是包络模型。
- **GPU**：AVAS 的 GPU 版本只有 Linux 上预编译的库，Windows 上用多线程 CPU 计算。

## 输出平面与输出文件 {#simulation-output}

### 输出平面

```avas-params
outputplane automaticoutput
```

- `outputplane V1` 在关键字所在位置下游 $V_1$（正值）或上游（负值）输出束流分布，文件为 `outData_<位置>.dst`。
- `automaticoutput V1 V2 V3` 从相对位置 $V_1$ 开始、间距 $V_2$、最大跨度 $V_3$ 批量插入输出平面。
- 输出平面**超出 lattice 末端**会报错；**离末端 5 mm 以内**也会失败，离末端 2 cm 可以。内核结束时总会写末端的 `outData_<末端>.dst`，无需在末端另放输出平面。

### 主要输出文件

| 文件 | 内容 |
|---|---|
| `DataSet.txt` | 每个记录步一行的束团统计量（下表） |
| `inData.dst` | 模拟实际使用的初始分布 |
| `outData_<z>.dst` | 输出平面上的粒子分布（二进制） |
| `Phase.txt` | 每个射频腔入口、出口的时间、位置与同步粒子能量 |
| `synParticle.txt` | 同步粒子沿线的信息 |
| `Beamset.plt` | `dumpperiodicity` 打开时的全部粒子数据 |
| `SingleParticle.txt` | 单粒子运行的轨迹 |

### DataSet.txt

每行 41 列（从 0 开始编号），常用的列：

| 列 | 内容 |
|---|---|
| 0 | 平均动能（MeV） |
| 1、3、5 | 质心相对同步粒子的 $x$、$y$、$z$ |
| 7–9、10–12、13–15 | 三个方向的 $\alpha$、$\beta$、发射度 |
| 16、18、20 | rms $x$、$y$、$z$（m） |
| 22、24、26 | 最大 $x$、$y$、$z$ |
| 28 | 存活宏粒子数 |
| 29、31、33 | 同步粒子的 $x$、$y$、$z$ |
| 35 | 标志：0 直线段，1 曲线段，2 无效（跳过） |
| 40 | 时间 |

直线段的纵向位置是第 5 列加第 33 列；束线中有二极铁时要按曲线段累加弧长（AVAS 的读取程序已处理）。内核在运行开始时重建这个文件并逐行追加，读取正在写的文件要丢弃最后一行不完整的数据。

> AVAS 约定：运行**不改写 InputFile**。完整运行把文本输入复制到 `<输出目录>/inputs/`，内核以副本为输入、以原目录为场图目录，所以输出目录自带本次运行的输入快照。完整运行每次覆盖 OutputFile，需要保留的结果要在界面中"保留"（复制到项目的 `Runs/`）。

## 收敛性判断 {#simulation-convergence}

一个多粒子结果可信，至少要说明它对数值参数不敏感。建议的做法：

1. 先用线性预览和较少的宏粒子（$10^3$–$10^4$）调通 lattice，确认能量、相位和包络大致合理。
2. **宏粒子数**：在设计参数下把 `particlenumber` 增加 4 倍以上（例如 $10^4 \to 10^5$），比较出口 rms 尺寸、rms 发射度增长和损失率。统计误差按 $1/\sqrt N$ 下降，发射度增长若随 $N$ 增大而明显减小，说明原结果被数值噪声污染。
3. **步长**：把 `steppercycle` 加倍（例如 50 → 100），结果应在统计误差内不变。
4. **网格**：同时改变 `numofgrid` 和 `meshrms`，并保持每格宏粒子数足够；必要时比较不同的 `scmethod`。
5. **重复运行**：同一输入重复两三次，估计运行间的随机差别，作为判断"有无变化"的容差。

判断标准取决于问题：匹配和包络设计关心 rms 尺寸（1 % 量级）；发射度增长关心相对增长量；束损和束晕需要高统计量和最大尺寸、尾部分布。报告结果时同时给出所用的宏粒子数、步长和网格。

## 线性预览与内核 {#simulation-preview}

可视化编辑器的**线性包络预览**（`avas.sim.linear_optics`）用 6×6 矩阵推进束流二阶矩：参考粒子沿场图积分能量和时间，场图按 4 mm 切片计算线性聚焦与聚束，空间电荷用 rms 等效均匀椭球的线性冲量（见 [空间电荷](#space-charge-avas)）。它在拖动参数时几乎即时给出结果，适合调匹配和初步设计。

| 项目 | 线性预览 | 多粒子内核 |
|---|---|---|
| 能量 | 误差约 0.01 % | 参考 |
| rms 尺寸 | 通常差 1–2 %，多粒子效应强时可到 13 % | 参考 |
| 非线性、发射度增长、束晕 | 不包含 | 包含 |
| 损失 | 只提示 3 倍 rms 超过孔径 | 逐粒子判定 |
| 场图的二极 / 校正分量 | 不计偏转（给出提示） | 包含 |

> 注意：预览不包含低能腔中的纵向非线性、色差、`superposeout` 的偏移和 `scanphase`。设计结论以内核为准；预览与内核差别大时，通常说明非线性或空间电荷效应强，正需要多粒子模拟。

## 单粒子运行 {#simulation-single-particle}

`particlenumber 1` 时内核做单粒子模拟，输出 `SingleParticle.txt`（$x$、$y$、$z$、$\gamma\beta_{x,y,z}$、动能、时间）。此时 `twissx`、`twissy`、`twissz`、`current`、`distribution` 等多粒子关键字不生效，用 `displacepos` 和 `displacedpos` 设置这个粒子相对同步粒子的偏移：

```avas-params
beam:displacepos beam:displacedpos
```

> AVAS 约定：`displacepos dx dy dz` 内核**按 mm 读取**（手册写 m，已用内核核实）；`displacedpos dpx dpy dpz` 的单位是 %。单粒子运行的 DataSet.txt 中 rms 列为 NaN，这是正常的，要看轨迹和同步粒子的列。

单粒子运行适合检查轨道：校正铁的偏转、二极铁的色散、场图的对中，或者给单个偏离粒子算出等效的传输矩阵元。

## 常见问题 {#simulation-troubleshooting}

### 起点就丢失全部粒子

内核报 `End of simulation, all particles lost`。按以下顺序排查（第一条已用内核核实，其余是值得检查的可能原因）：

- `input.txt` 中有 `multithreading 0`（或 2、3）——删掉这一行或改为 1；
- 初始束流相对孔径 `R` 太大（3 倍 rms 已接近孔径），或 `initpos` 偏移过大；
- `longlimits` 打开且边界太窄；
- 第一个元件是矩阵元件——手册建议避免但没有说明后果，在前面加一段短漂移即可排除。

### 找不到场图

`field` 的最后一个参数是场图**基名**（不含扩展名），文件按场类型取扩展名：射频场 `.edx/.edy/.edz` 与 `.bdx/.bdy/.bdz`，静电场 `.esx/.esy/.esz`，静磁场 `.bsx/.bsy/.bsz`。场图目录默认是 InputFile，可以在 `ini.ini` 的 `[project] fieldSource` 中另设。场图的存储顺序按一组分量文件判断（见 [场图](#field-maps)）。示例场图本身有噪声，切片图沿 $z$ 的条纹是数据而不是读取错误。

### 输出平面报错

输出平面超出 lattice 末端，或离末端不到 5 mm。改到离末端至少约 2 cm 处，或者直接使用内核在末端自动写出的 `outData_<末端>.dst`。

### 其他

- **只有 `start` 到 `end` 之间被模拟**：第一个 `start` 之后、第一个 `end` 之前的行才生效，`!` 之后是注释。写在 `end` 之后的元件不会被计算，常见于调试时临时截断 lattice 后忘记恢复。
- **能量降到零或束流反向**：射频相位错误（例如同步相位符号相反、V3 理解错误）会让参考粒子减速，预览会在该处停止并给出提示。
- **rms 列为 NaN**：单粒子运行属正常；多粒子运行中出现说明存活粒子过少。
- **结果每次略有不同**：多线程计算的正常现象，按统计误差比较。

## 延伸阅读 {#simulation-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：第 9 章（多粒子模拟与空间电荷）。
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：分布函数与发射度增长。
- R. W. Hockney, J. W. Eastwood, *Computer Simulation Using Particles*, Adam Hilger, 1988：粒子–网格方法与数值噪声。
- 本入门的 [空间电荷](#space-charge)、[误差与校正](#errors)、[教程](#tutorial)；输入输出文件的完整说明见界面的使用说明（参数与文件参考）。
