# 加速器与 AVAS

本入门面向用 AVAS 做研究的加速器物理工作者和研究生：从直线加速器的基本概念出发，经过相对论运动学、相空间与线性光学，逐个介绍 AVAS 能模拟的元件，最后讲如何把它们组装成一条束线并完成匹配、误差研究和多粒子模拟。本章先给出全局图景：直线加速器由什么组成、AVAS 模拟的是什么、一次模拟从输入到输出经过哪些文件，以及本入门的阅读方式。

## 直线加速器做什么 {#intro-linac}

直线加速器（linac）让带电粒子沿一条近似直线的参考轨道通过一系列加速间隙和聚焦元件，使粒子获得能量，同时把束流约束在有限的横向尺寸和纵向（相位）范围内。与环形加速器不同，粒子只通过每个元件一次，因此束流品质由**一次通过**的动力学决定：入口束流的相空间分布、各元件的场以及束流自身的空间电荷力共同决定出口的能量、发射度和损失。

描述束流需要两类量：

- **参考粒子**（同步粒子）的运动：它沿设计轨道运动，其动能 $W(s)$ 和到达各处的时刻 $t(s)$ 决定了射频腔的相位和每个元件的"设计状态"；
- **其他粒子相对参考粒子的偏离**：横向位置和角度 $(x, x', y, y')$、纵向位置和动量偏差 $(z, \delta)$。束流的尺寸、发射度、能散都是这些偏离量的统计量，见 [相空间与发射度](#phase-space)。

对低能强流直线加速器（例如质子、重离子超导直线加速器的低能段），空间电荷、射频散焦和纵向非线性往往同时起作用，这正是多粒子模拟程序存在的原因。

## 束线的基本组成 {#intro-components}

一条直线加速器束线可以按功能分成四类元件，下表列出它们在 AVAS 中的对应写法：

| 功能 | 典型元件 | AVAS 中的写法 | 详见 |
|---|---|---|---|
| 横向聚焦 | 四极铁、螺线管 | `quad`、`solenoid`，或静磁场图 `field … 3 …` | [四极铁](#quadrupole)、[螺线管](#solenoid) |
| 加速与纵向聚焦 | 射频腔（如半波长谐振腔 HWR） | 射频场图 `field … 1 …` | [射频腔](#rf-cavity) |
| 偏转与轨道校正 | 二极铁、校正铁 | `bend`、`edge`、`steerer` | [二极铁](#dipole)、[校正铁](#steerer) |
| 束流诊断与输出 | 束诊位置、输出平面 | `diag_energy`、`diag_size`、`diag_position`、`outputplane` | [误差与校正](#errors)、[模拟设置](#simulation) |

此外还有漂移段 `drift`（无场空间，见 [漂移段](#drift)）、静电场图 `field … 2 …`（见 [静电与静磁场元件](#static-fields)），以及让多个场图在空间上重叠的 `superpose` 块（见 [叠加场](#superpose)）。

> 注意：本入门只讨论 AVAS 能够模拟的内容。RFQ、DTL 加速结构的专门模型、离子源等不在 AVAS 的元件之内，书中不作为 AVAS 的功能介绍；如果需要处理 RFQ 出口的束流，可以把其他程序生成的粒子分布文件（`.dst`）作为 AVAS 的初始束流读入（`readparticledistribution`）。

## AVAS 模拟什么 {#intro-what-avas-simulates}

AVAS（Advanced Virtual Accelerator Software）是一个直线加速器多粒子束流动力学程序。计算内核是 C++ 编写的二进制库（Windows 下为 `AVAS.dll`），界面与前后处理由 Python 和网页前端完成。内核做的事情可以概括为：

1. **参考粒子**：从 `beam.txt` 给定的动能出发，沿 lattice 积分参考粒子的能量和时间，确定每个射频腔的入口相位（见 [纵向运动](#longitudinal)）。
2. **宏粒子**：按 `beam.txt` 的 Twiss 参数和分布类型生成 `particlenumber` 个宏粒子（或读入粒子分布文件）。每个宏粒子代表许多真实粒子，总电荷由流强 `current` 和束流频率 `frequency` 决定（每个束团电荷 $Q = I/f$）。
3. **外场作用**：场图元件和漂移段用 t-code 按时间步积分洛伦兹力；矩阵元件用 z-code 的传输矩阵（见 [下一节](#intro-tracking)）。
4. **空间电荷**：`input.txt` 中 `spacecharge 1` 打开后，按 `scmethod` 选择的算法（FFT、PICNIC、SPICNIC）计算束流自场并作用到每个宏粒子上（见 [空间电荷](#space-charge)）。
5. **误差与校正**：在 lattice 中用 `err_step`、`err_beam_dyn`、`err_beam_stat`、`err_quad_*`、`err_cav_*` 等命令给元件和初始束流加上随机误差，用 `adjust` 和 `diag_*` 做静态误差校正（见 [误差与校正](#errors)）。
6. **损失**：粒子超出元件孔径 `R` 或纵向边界时记为损失，存活宏粒子数随 $s$ 写入输出。

## t-code 与 z-code {#intro-tracking}

AVAS 的元件分为两种模型，它们的计算方式不同。

### 场模型元件（t-code）

`field` 元件（以及漂移段 `drift`）由用户提供电磁场的三维分布（场图文件）。内核以**时间 $t$ 为自变量**，逐步积分每个宏粒子的运动方程

$$
\frac{\dd \boldsymbol p}{\dd t} = q\left(\boldsymbol E + \boldsymbol v \times \boldsymbol B\right) + \boldsymbol F_{\mathrm{sc}}
$$

其中 $\boldsymbol F_{\mathrm{sc}}$ 是空间电荷力。这称为 t-code。时间步长由 `input.txt` 的 `steppercycle` 决定（每个射频周期分为若干步）。t-code 自然包含场的非线性、边缘场以及粒子在腔内速度变化等效应，是低能射频腔模拟所必需的。

### 矩阵模型元件（z-code）

`quad`、`solenoid`、`bend`、`edge`、`steerer` 只需几个参数（长度、梯度、磁场等）。内核以**纵向位置 $z$ 为自变量**，用硬边模型的传输矩阵变换粒子坐标，这称为 z-code。它计算快，可以直接与解析理论对照（见 [线性光学](#linear-optics)），但不含连续的边缘场分布（`edge` 只给出极面边缘的薄透镜修正）；内核矩阵元件是否包含高阶项，手册未说明，以内核为准。

两种模型在 lattice 中可以混用；在两者的交界处，粒子分布要在"同一时刻"与"同一纵向位置"两种描述之间转换（具体做法手册未说明，以内核为准）。

> AVAS 约定：多粒子模拟时，lattice 的**第一个和最后一个元件不要用矩阵模型元件**（手册的要求）；需要时在两端加一段很短的 `drift`。

此外，AVAS 的图形界面提供一个**线性包络预览**（可视化编辑器中的"线性预览"）：它不调用内核，而是用 6×6 矩阵推进束流的二阶矩，并对场图做切片近似。它的原理见 [线性光学](#linear-optics-6d)，精度和适用范围见 [本章最后一节](#intro-how-to-read)。

## 输入文件与工作流程 {#intro-workflow}

一个 AVAS 项目包含 `InputFile/` 和 `OutputFile/` 两个目录。输入由以下文件组成：

| 文件 | 内容 | 详见 |
|---|---|---|
| `beam.txt` | 粒子种类（`particlerestmass`、`numofcharge`）、动能 `kneticenergy`、流强 `current`、束流频率 `frequency`、宏粒子数、分布类型 `distribution`、Twiss 参数 `twissx`/`twissy`/`twissz` | [运动学](#kinematics-in-avas)、[相空间](#phase-space-in-avas) |
| `input.txt` | 模拟类型 `sim_type`、步长 `steppercycle`、空间电荷开关与算法、网格、输出设置等 | [模拟设置](#simulation) |
| lattice 文件 | 元件序列：`start` 与 `end` 之间的各行，`!` 后为注释 | [束线设计](#lattice-design) |
| 场图文件 | 场图元件引用的三维场分布，按分量分文件（如 `hwr010.edz`、`hwr010.edx`、`sol.bsz` 等） | [场图](#field-maps) |
| `ini.ini` | 项目设置；`[lattice] source` 指定运行所用的 lattice 文件 | — |

示例项目 `examples/hwr010` 的束流是 1.527 MeV、0.18 mA、162.5 MHz 的质子束，lattice 只有一个 0.21 m 的 HWR 射频腔场图：

```text
start
field  0.21   0.02     0   1   162.5e6   -33   1.36    -1.36   efield
end
```

一次完整运行的流程是：

1. 在界面的"束流""结构""设置"页面编辑输入（或直接编辑文本文件）。
2. 在"运行"页面开始运行。运行**不会改写** InputFile：输入文件先被复制到 `<输出目录>/inputs/`，内核以副本为输入，以原目录为场图目录。
3. 内核在运行开始时重建 `OutputFile/DataSet.txt`，之后逐行追加；界面每秒读取新增的行，实时显示束流包络。
4. 运行结束后在"结果"页面画图，或在"运行"页面回放本次运行。

## 输出与结果 {#intro-outputs}

最常用的输出是 `DataSet.txt`：每一行是某个时刻（或位置）的束团统计量，共 41 列。其中第 0 列是参考粒子动能（MeV），纵向位置 $z$ 为第 5 列与第 33 列之和，rms 尺寸 $x$、$y$、$z$ 在第 16、18、20 列（单位 m），最大值在第 22、24、26 列，存活宏粒子数在第 28 列。其他输出包括：

- `inData.dst`：模拟实际使用的初始粒子分布；
- `outData_<z>.dst`：输出平面上的粒子分布（二进制），内核结束时总会写出 lattice 末端的分布；
- `Phase.txt`：每个射频腔入口与出口处同步粒子的时间、位置和能量（每腔两行）；
- `synData.txt`：同步粒子到达每个元件入口时的时间、$\gamma\beta$、同步相位 $\varphi_s$ 与射频相位 $\varphi_{\mathrm{RF}}$；
- 误差研究的结果放在 `OutputFile/error_output/` 下。

> 注意：单粒子运行（`particlenumber 1`）时参考粒子数据照常输出，而 rms 列可能为 NaN，这是正常的。内核多线程运行时，每次结果略有差别，比较两次 `DataSet.txt` 要带容差。

## 如何阅读本入门 {#intro-how-to-read}

本入门分为三部分：

- **基础**：[相对论运动学](#kinematics)、[相空间与发射度](#phase-space)、[线性光学](#linear-optics)、[纵向运动](#longitudinal)、[空间电荷](#space-charge)。这些章节建立后文需要的概念和记号。
- **元件**：[漂移段](#drift)、[四极铁](#quadrupole)、[螺线管](#solenoid)、[二极铁](#dipole)、[校正铁](#steerer)、[射频腔](#rf-cavity)、[静电与静磁场元件](#static-fields)、[场图](#field-maps)、[叠加场](#superpose)。每章依次讲作用、物理原理、传输矩阵、AVAS 中的关键字、可调参数、常见问题和动手实验。
- **组装**：[束线设计](#lattice-design)、[匹配](#matching)、[误差与校正](#errors)、[模拟设置](#simulation)、[教程](#tutorial)、[参考文献](#references)。

阅读时请注意以下几点：

- **动手实验**是线性模型。实验区块在后端用 AVAS 自己的线性光学代码计算，拖动滑块时结果实时更新，适合建立直觉；它们与线性包络预览采用同一套近似：参考粒子能量误差约 0.01 %，rms 尺寸通常相差 1–2 %，多粒子效应强时可达 13 %。**最终结论以内核的多粒子模拟为准。**
- **场的图形**区分数据与示意：场图文件的切片是数据；矩阵元件的场是按参数画出的示意图，图中会标明。横截面一律从下游往上游看，束流垂直屏幕向外（⊙），这样可以直接用 $\boldsymbol F = q\boldsymbol v \times \boldsymbol B$ 核对受力方向。
- **"AVAS 约定"** 框标出 AVAS 与教科书或手册不同、并已用内核核实的约定（例如 Twiss 的单位、`displacepos` 按 mm 读取、`multithreading` 只能写 1）。
- 各章标有审核状态。当前章节为 **AI 撰写，待审核**：公式和数值经过计算核对，但尚未经物理学家逐条审阅；发现问题请反馈给维护者。

## 延伸阅读 {#intro-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：直线加速器物理的标准教材，覆盖射频、纵向与横向动力学和空间电荷。
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：束流光学、发射度与空间电荷的系统论述。
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific。
- CERN Accelerator School（CAS）各期讲义，尤其是 "Introduction to Accelerator Physics" 与 "RF for Accelerators"。
- AVAS 使用说明（界面"帮助 → 使用说明"）中的"参数与文件参考"，以及本入门的 [参考文献](#references) 一章。
