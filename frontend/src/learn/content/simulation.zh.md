# 多粒子模拟要点

示例项目的一个束团里约有 700 万个质子，AVAS 只用 5260 个宏粒子代表它们，每 0.12 ns 推进一步。这样算出来的结果可信吗？同一个输入跑两次，结果为什么不完全一样？差多少才算真的不同？本章从这些问题出发，讲清楚多粒子模拟的误差从哪里来：宏粒子数有限带来的统计误差和数值噪声、时间步长、空间电荷网格；怎样用收敛性检验判断结果可信；线性预览与内核各自能告诉你什么。最后是 AVAS 的运行设置、输出文件、单粒子运行和常见失败原因。

物理背景见 [空间电荷](#space-charge) 和 [纵向动力学](#longitudinal)，初始分布见 [初始分布](#distributions)，输入文件的整体结构见 [加速器与 AVAS](#intro)。

## 宏粒子与统计 {#simulation-macroparticles}

### 宏粒子代表什么

一个束团的真实粒子数是 $Q/q$（$Q = I/f$ 为束团电荷），通常是 $10^6$–$10^{10}$ 量级，逐个计算它们之间的作用力不现实。内核用 `particlenumber` 个**宏粒子**代表它们：每个宏粒子的电荷为 $Q/N$，质量也按同样比例放大，荷质比与真实粒子相同。外场中的运动只取决于荷质比，所以每个宏粒子走的轨迹和一个真实粒子完全一样；只有空间电荷按总电荷计算。可以把它想成民意调查：用几千个样本代表几百万人，只要样本有代表性，平均值就是对的，但每个统计量都带着抽样误差。

示例项目 $Q = 1.133\ \mathrm{pC}$（$7.07\times 10^6$ 个质子），`particlenumber 5260`，每个宏粒子代表约 1340 个质子。

### 统计误差有多大

用 $N$ 个样本估计高斯分布的方差 $\sigma^2$，估计值本身的方差是 $2\sigma^4/N$。rms 尺寸是方差的平方根，相对误差减半，于是

$$
\frac{\delta\sigma}{\sigma} \approx \frac{1}{2}\sqrt{\frac{2}{N}} = \frac{1}{\sqrt{2N}}
$$

它随 $1/\sqrt N$ 下降：要把误差缩小 10 倍，宏粒子数要增加 100 倍。式中的因子 2 属于高斯分布，示例用的正是高斯分布（`distribution GS GS`）。对一般的分布，方差估计值的方差为 $(\mu_4 - \sigma^4)/N$，于是 $\delta\sigma/\sigma \approx \sqrt{(\kappa - 1)/(4N)}$，$\kappa = \mu_4/\sigma^4$ 是峰度：边缘陡的分布估计得更准（均匀投影 $\kappa = 1.8$，误差为 $0.45/\sqrt N$，而不是 $0.71/\sqrt N$），尾部重的分布估计得更差。

| $N$ | rms 尺寸的相对统计误差 |
|---|---|
| 5260（示例） | 0.97 % |
| $10^4$ | 0.71 % |
| $10^5$ | 0.22 % |
| $10^6$ | 0.07 % |

rms 发射度的统计误差与此同一量级，高斯分布约为 $1/\sqrt N$（示例为 1.4 %）。损失率的分辨率是 $1/N$：示例中丢失一个宏粒子就是 0.019 %，要研究 $10^{-4}$ 以下的损失或束晕，必须用远多于此的宏粒子，或者专门研究分布尾部。

> **想一想**：用 5260 个宏粒子，改一个参数前后出口 rms 尺寸差了 1 %。能据此说这个参数有影响吗？
>
> 答：不能确定。若两次运行的初始分布是独立抽样，每个结果各有约 0.97 % 的统计误差，两者之差的标准差约为 $\sqrt2 \times 0.97\ \% = 1.4\ \%$，1 % 的差别落在其中。若两次运行从同一个样本出发（相同的 `randomseed`；手册没有说明种子怎样使用，可以比较两次的 `inData.dst` 是否相同），抽样误差是两者共有的，在差值中大部分抵消，剩下的是内核自身每次运行的随机差别。无论哪种情况，都要么增加宏粒子数，要么重复运行测出运行间的差别，再下结论。

### 数值噪声

另一个误差来源是**数值噪声**：宏粒子比真实粒子少得多，每个宏粒子的电荷大得多，空间电荷场的涨落因此比真实束大，粒子受到随机的小冲量，造成虚假的发射度增长（数值加热）。它随每个网格单元里的宏粒子数增加而减小，见 [空间电荷网格](#simulation-sc-mesh)。

> **注意**：内核多线程计算，同样的输入每次运行的结果会略有差别。比较两次运行（例如判断某个参数的影响）时要带容差，差别小于统计误差的结论不可靠。

## 初始分布 {#simulation-distributions}

内核可以按 `twissx`、`twissy`、`twissz` 和 `distribution` 由 Twiss 参数生成 KV、GS（高斯）、PB（抛物线）、WB（水袋）四种分布，也可以用 `readparticledistribution` 读入已有的粒子文件（例如 RFQ 出口分布）。同样的 rms 参数下，不同分布的边界和尾部差别很大，空间电荷强时还决定初始阶段的发射度增长；比较不同程序的结果时要保证分布一致。这些分布怎样定义、彼此有什么不同、怎样检查模拟实际用了什么分布，见 [初始分布](#distributions) 一章；相关关键字列在 [在 AVAS 中](#simulation-in-avas)。

## 时间步长 {#simulation-stepping}

### 为什么步长要小

内核不能连续地求解运动方程，只能一步一步推进：每一步用当前的场算出受力，再把粒子推进一小段。这就像用一段段直线去描一条曲线，直线越短，描得越准；场在一步之内变化越大，误差越大。

场图元件和漂移段用 t-code 按时间积分运动方程（见 [加速器与 AVAS](#intro-tracking)）。`input.txt` 的 `steppercycle V1` 把一个射频周期分成 $V_1$ 步：

$$
\Delta t = \frac{1}{f\,V_1},\qquad \Delta s \approx \beta c\,\Delta t = \frac{\beta\lambda}{V_1} .
$$

示例 `steppercycle 50`、162.5 MHz、$\beta = 0.057$：$\Delta t = 0.123\ \mathrm{ns}$，每步前进约 2.1 mm；而 `hwr010` 场图沿 $z$ 的网格间距是 2 mm，两者相当。穿过 0.21 m 长的 HWR 约需 100 步，[动手教程](#tutorial) 中 0.76 m 的一个周期约 360 步。注意 $\Delta s$ 正比于 β：同样的 `steppercycle`，粒子越快，每步走得越远。

选择步长的原则：

- 每步的距离要远小于场的变化尺度（场图网格、间隙长度、$\beta\lambda$）；
- 远小于横向振荡波长、同步振荡波长和等离子体波长（空间电荷冲量用同一个步长积分；内核多久重新计算一次空间电荷场，手册没有说明）；
- 用加倍步数的方法检验：结果不再变化（在统计误差范围内）时步长足够。

### z-code 元件

`quad`、`solenoid`、`bend`、`edge`、`steerer` 是矩阵元件，用 z-code 按纵向位置变换粒子坐标，不按时间步推进。t-code 按同步粒子的时刻记录束团，z-code 按位置记录（见手册对 `Beamset.plt` 的说明），两类元件交界处如何转换手册未说明，以内核为准。schema 对矩阵元件的说明是：多粒子 lattice 的**第一个和最后一个元件避免使用矩阵元件**，可以在两端各放一段短漂移。

## 空间电荷网格 {#simulation-sc-mesh}

### 网格做什么

直接计算 $N$ 个宏粒子两两之间的库仑力要 $N^2$ 次运算。粒子–网格方法换一种做法：先把所有宏粒子的电荷分配到一个三维网格上，得到网格上的电荷密度；在网格上解 Poisson 方程得到电场；再把电场插值回每个粒子。这有点像估计人群密度：把广场划成方格，数每格的人数，而不是测量每两个人之间的距离。方格太大，看不出人群的形状；方格太小，每格只有零星几个人，数出来的密度全是涨落。

`spacecharge 1` 时，内核在网格上求解束流自场。`numofgrid Nx Ny Nz` 是网格数，`meshrms V1 V2 V3` 设网格边长 $L_x = 2V_1\sigma_x$（$y$、$z$ 同理），网格单元的尺寸因此是

$$
h_x = \frac{2V_1\sigma_x}{N_x} .
$$

例如 $V_1 = 4$、$N_x = 32$ 时，网格盖住 $\pm 4\sigma_x$，单元尺寸 $h_x = 0.25\,\sigma_x$。

### 三个相互制约的要求

- **覆盖**：网格应盖住几乎全部粒子。$V_1$ 太小会截掉尾部；对高斯分布 $V_1$ 至少取 3–5。超出网格的粒子如何处理手册未说明。
- **分辨率**：束芯要有足够多的网格点，$h_x$ 应明显小于 $\sigma_x$；$V_1$ 越大，同样 $N_x$ 下分辨率越差。
- **每格粒子数**：平均每个单元的宏粒子数约为 $N/(N_xN_yN_z)$，束芯中更多一些。例如 $32\times 32\times 32$ 的网格配 5260 个宏粒子，平均每格只有 0.16 个，场的数值噪声很大；这样的网格需要 $10^5$–$10^6$ 个宏粒子。

三者的矛盾在于：提高分辨率要更多网格，更多网格又要求更多宏粒子。所以宏粒子数和网格必须一起选。改变网格或算法后，按 [收敛性判断](#simulation-convergence) 的办法比较结果。

## 收敛性判断 {#simulation-convergence}

一个多粒子结果可信，至少要说明它对数值参数不敏感：把数值参数变得更精细，结果在统计误差内不再变化。建议的做法：

1. 先用线性预览和较少的宏粒子（$10^3$–$10^4$）调通 lattice，确认能量、相位和包络大致合理。
2. **宏粒子数**：在设计参数下把 `particlenumber` 增加 4 倍以上（例如 $10^4 \to 10^5$），比较出口 rms 尺寸、rms 发射度增长和损失率。统计误差按 $1/\sqrt N$ 下降，发射度增长若随 $N$ 增大而明显减小，说明原结果被数值噪声污染。
3. **步长**：把 `steppercycle` 加倍（例如 50 → 100），结果应在统计误差内不变。
4. **网格**：同时改变 `numofgrid` 和 `meshrms`，并保持每格宏粒子数足够；必要时比较不同的 `scmethod`。
5. **重复运行**：同一输入重复两三次，估计运行间的随机差别，作为判断"有无变化"的容差。

判断标准取决于问题：匹配和包络设计关心 rms 尺寸（1 % 量级）；发射度增长关心相对增长量；束损和束晕需要高统计量和最大尺寸、尾部分布。报告结果时同时给出所用的宏粒子数、步长和网格。

## 线性预览与内核 {#simulation-preview}

可视化编辑器的**线性包络预览**（`avas.sim.linear_optics`）不跟踪粒子，而是用 6×6 矩阵推进束流的二阶矩（σ 矩阵）：参考粒子沿场图积分能量和时间，场图按 4 mm 切片计算线性聚焦与聚束，空间电荷用 rms 等效均匀椭球的线性冲量（见 [空间电荷](#space-charge)）。一次计算只推进一个 6×6 矩阵，所以拖动参数时几乎即时给出结果，适合调匹配和初步设计。代价是它只知道二阶矩，看不到分布形状的变化。

| 项目 | 线性预览 | 多粒子内核 |
|---|---|---|
| 能量 | 参考粒子误差约 0.01 %（与内核同步粒子比）；内核 DataSet 是宏粒子平均能量，受离轴场影响，示例腔增益差约 5 % | 参考 |
| rms 尺寸 | 通常差 1–2 %，多粒子效应强时可到 13 % | 参考 |
| 非线性、发射度增长、束晕 | 不包含 | 包含 |
| 损失 | 只提示 3 倍 rms 超过孔径 | 逐粒子判定 |
| 场图的二极 / 校正分量 | 不计偏转（给出提示） | 包含 |

> **注意**：预览不包含低能腔中的纵向非线性、色差、`superposeout` 的偏移和 `scanphase`。设计结论以内核为准；预览与内核差别大时，通常说明非线性或空间电荷效应强，正需要多粒子模拟。

## 容易误解的地方 {#simulation-misconceptions}

- **宏粒子越多越好，但要配合网格**：只加宏粒子而网格不变，分辨率不提高；只加网格而宏粒子不变，噪声更大。
- **小于统计误差的差别没有意义**：5260 个宏粒子的 rms 尺寸误差约 1 %，比这小的变化不能作为结论。
- **零损失不等于无损失**：5260 个宏粒子中没有一个丢失，只说明损失率低于约 $1/5260$ 的量级。
- **`steppercycle` 是每个射频周期的步数**，不是固定的空间步长：粒子加速后每步走得更远。
- **`multithreading 0` 不是关闭多线程**：它会让内核在起点丢失全部粒子，关闭多线程要删掉这一行（见下文）。
- **单粒子运行的 NaN**：`particlenumber 1` 时 DataSet 的 rms 列为 NaN，是正常的。

## 在 AVAS 中 {#simulation-in-avas}

### 束流与宏粒子

```avas-params
beam:particlenumber beam:randomseed beam:current beam:frequency
```

`randomseed` 是生成初始束流的随机数种子。手册把它列在 `beam.txt` 的关键字中，示例项目却写在 `input.txt` 里（`randomseed 0`，schema 两处都接受）；两处都写时内核用哪一个、0 是否有特殊含义，手册没有说明，以内核为准。

### 初始分布的关键字

```avas-params
beam:distribution beam:twissx beam:twissz beam:readparticledistribution beam:initpos beam:initmom
```

- `twissx`、`twissy`、`twissz` 给出每个平面的 $\alpha$、$\beta$ 与**归一化 rms** 发射度（β 单位 mm/mrad，发射度 π·mm·mrad；$z$ 平面的角度是 $\delta = \Delta p/p$），见 [相空间与发射度](#phase-space)。`distribution V1 V2` 分别选择横向和纵向分布类型：KV、GS、PB、WB。
- `readparticledistribution` 指定已有的 `.dst` 或 `.edst` 文件。使用粒子文件时，除 `numofcharge` 外 `beam.txt` 的其他关键字都不生效；`.edst` 可以描述多种粒子组成的混合束。
- `initpos`、`initmom` 给整个束团加位置和角度偏移，可用来研究初始偏心。

> **注意**：手册没有说明 AVAS 生成这些分布时使用的维数和高斯分布的截断位置。需要准确知道时，读取输出目录的 `inData.dst`（模拟实际使用的初始分布）统计。

### 步长与模型

```avas-params
input:steppercycle input:sim_type
```

手册把步长公式写成 "Δt = 1/T × 1/V1"；第二个参数 V2 只用于次级粒子输运（$\Delta t = V_2/V_b$）。`sim_type mulp` 是多粒子模型，`env` 是包络模型。

### 空间电荷

```avas-params
input:spacecharge input:scmethod input:numofgrid input:meshrms
```

`scmethod` 可选 `FFT`、`PICNIC`、`SPICNIC`，手册只列出名称。示例项目的 `input.txt` 用 `spacecharge 1`、`scmethod FFT`。

### 运行设置

```avas-params
input:multithreading input:dumpperiodicity input:boundary input:longlimits input:pchistogram input:scanphase
```

- **多线程**：写 `multithreading 1` 使用多线程，单线程就**不写这一行**。手册说 0 表示关闭，但内核遇到 `multithreading 0`（以及 2、3 等）会在起点丢失全部粒子（已用示例项目核实）。界面的模拟设置页已按此处理。
- **损失边界**：粒子超出元件孔径 `R` 记为损失；`boundary 1` 时改用 `boundary.txt` 中的边界，lattice 中的半径不再生效，损失粒子写入 `CollisionData.txt`。`longlimits 1 相位 能量` 打开纵向损失判定，与 [纵向动力学](#longitudinal) 中的分界线概念对应。
- **粒子数据**：`dumpperiodicity V1` 每 $V_1$ 步把全部粒子写入 `Beamset.plt`（0 表示不写）；文件可能很大。`pchistogram 1 份数` 生成 `pchistogram.dat`。
- **相位扫描**：`scanphase` 控制射频腔的相位扫描或从 `scanData.txt` 读取相位（见 [纵向动力学](#longitudinal)）。
- **GPU**：AVAS 的 GPU 版本只有 Linux 上预编译的库，Windows 上用多线程 CPU 计算。

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

> **AVAS 约定**：运行**不改写 InputFile**。完整运行把文本输入复制到 `<输出目录>/inputs/`，内核以副本为输入、以原目录为场图目录，所以输出目录自带本次运行的输入快照。完整运行每次覆盖 OutputFile，需要保留的结果要在界面中"保留"（复制到项目的 `Runs/`）。

### 单粒子运行

`particlenumber 1` 时内核做单粒子模拟，输出 `SingleParticle.txt`（$x$、$y$、$z$、$\gamma\beta_{x,y,z}$、动能、时间）。此时 `twissx`、`twissy`、`twissz`、`current`、`distribution` 等多粒子关键字不生效，用 `displacepos` 和 `displacedpos` 设置这个粒子相对同步粒子的偏移：

```avas-params
beam:displacepos beam:displacedpos
```

> **AVAS 约定**：`displacepos dx dy dz` 内核**按 mm 读取**（手册写 m，已用内核核实）；`displacedpos dpx dpy dpz` 的单位是 %。单粒子运行的 DataSet.txt 中 rms 列为 NaN，这是正常的，要看轨迹和同步粒子的列。

单粒子运行适合检查轨道：校正铁的偏转、二极铁的色散、场图的对中，或者给单个偏离粒子算出等效的传输矩阵元。

### 常见问题

**起点就丢失全部粒子**：内核报 `End of simulation, all particles lost`。按以下顺序排查（第一条已用内核核实，其余是值得检查的可能原因）：

- `input.txt` 中有 `multithreading 0`（或 2、3）——删掉这一行或改为 1；
- 初始束流相对孔径 `R` 太大（3 倍 rms 已接近孔径），或 `initpos` 偏移过大；
- `longlimits` 打开且边界太窄；
- 第一个元件是矩阵元件——手册建议避免但没有说明后果，在前面加一段短漂移即可排除。

**找不到场图**：`field` 的最后一个参数是场图**基名**（不含扩展名），文件按场类型取扩展名：射频场 `.edx/.edy/.edz` 与 `.bdx/.bdy/.bdz`，静电场 `.esx/.esy/.esz`，静磁场 `.bsx/.bsy/.bsz`。场图目录默认是 InputFile，可以在 `ini.ini` 的 `[project] fieldSource` 中另设。场图的存储顺序按一组分量文件判断（见 [场图](#field-maps)）。示例场图本身有噪声，切片图沿 $z$ 的条纹是数据而不是读取错误。

**输出平面报错**：输出平面超出 lattice 末端，或离末端不到 5 mm。改到离末端至少约 2 cm 处，或者直接使用内核在末端自动写出的 `outData_<末端>.dst`。

**其他**：

- **只有 `start` 到 `end` 之间被模拟**：第一个 `start` 之后、第一个 `end` 之前的行才生效，`!` 之后是注释。写在 `end` 之后的元件不会被计算，常见于调试时临时截断 lattice 后忘记恢复。
- **能量降到零或束流反向**：射频相位错误（例如同步相位符号相反、V3 理解错误）会让参考粒子减速，预览会在该处停止并给出提示。
- **rms 列为 NaN**：单粒子运行属正常；多粒子运行中出现说明存活粒子过少。
- **结果每次略有不同**：多线程计算的正常现象，按统计误差比较。

## 延伸阅读 {#simulation-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：第 9 章（多粒子模拟与空间电荷）。
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：分布函数与发射度增长。
- R. W. Hockney, J. W. Eastwood, *Computer Simulation Using Particles*, Adam Hilger, 1988：粒子–网格方法与数值噪声。
- C. K. Birdsall, A. B. Langdon, *Plasma Physics via Computer Simulation*：粒子模拟的数值加热与网格选择。
- 本入门的 [空间电荷](#space-charge)、[误差与校正](#errors)、[教程](#tutorial)；输入输出文件的完整说明见界面的使用说明（参数与文件参考）。
