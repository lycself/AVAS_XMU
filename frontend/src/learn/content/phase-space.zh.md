# 相空间与发射度

束流由大量粒子组成，描述它的自然语言是相空间：每个粒子是相空间中的一个点，束流是一团点云。本章定义 AVAS 使用的六维坐标，介绍刘维尔定理、发射度、Twiss（Courant–Snyder）参数和 σ 矩阵，说明常用束流分布的边缘发射度与 rms 发射度的关系，最后把这些量与 `beam.txt` 的 `twissx`、`twissy`、`twissz` 逐项对应（尤其是单位）。

记号说明：本章用不带下标的 $\beta$、$\gamma$ 表示相对论因子（见 [相对论运动学](#kinematics)），用带平面下标的 $\beta_x$、$\alpha_x$、$\gamma_x$ 表示 Twiss 参数。

## 坐标与参考粒子 {#phase-space-coordinates}

取参考粒子的设计轨道为基准，$s$ 为沿轨道的距离（直线段上就是纵向位置 $z$）。任一粒子相对参考粒子的偏离用六个量描述：

| 坐标 | 含义 | 单位（AVAS 内部 / 矩阵） |
|---|---|---|
| $x$、$y$ | 横向位置偏离 | m |
| $x' = \dd x/\dd s$、$y' = \dd y/\dd s$ | 横向角度（斜率） | rad |
| $z$ | 纵向位置偏离，在参考粒子前方为正 | m |
| $\delta = \Delta p/p$ | 相对动量偏差 | 1 |

在近轴近似下 $x' \approx p_x/p$，其中 $p$ 是参考动量。传输矩阵作用在列向量 $(x, x', y, y', z, \delta)\tp$ 上，见 [线性光学](#linear-optics)。

两种描述方式要区分清楚：

- **同一时刻**的分布：t-code（场图元件、漂移段）按时间推进，粒子在同一时刻位于不同的纵向位置；
- **同一纵向位置**的分布：z-code（矩阵元件）和传输矩阵理论把所有粒子放到同一个 $s$ 处比较。

对 rms 量来说两者的差别通常很小，但在束团较长、能散较大时要注意（例如 $z$ 与 $\delta$ 的关联）。`DataSet.txt` 按手册说明记录的是 s 坐标系下的束团参数。

## 刘维尔定理与绝热阻尼 {#phase-space-liouville}

### 刘维尔定理

若粒子的运动可以由哈密顿量描述（外加电磁场，以及平均场近似下的空间电荷），则在正则坐标 $(x, p_x, y, p_y, z, p_z)$ 构成的六维相空间中，相空间密度沿粒子轨迹不变，任意一团粒子占据的相空间体积守恒。若三个方向的运动互不耦合，每个二维投影的面积也分别守恒。

刘维尔定理不保证"有效"发射度守恒：非线性力（场的非线性、非线性空间电荷、射频场的正弦形状）会把相空间中的椭圆扭曲成 S 形或螺旋，被占据的面积不变，但包住它所需的椭圆面积和 rms 发射度都会增大，这称为**丝化**（filamentation）。粒子损失、碰撞、剥离等过程不满足刘维尔定理的前提。

### 绝热阻尼与归一化发射度

角度 $x' = p_x/p$ 不是正则动量。纵向加速只增大 $p$ 而不改变 $p_x$，所以 $x'$ 按 $1/p$ 缩小，$(x, x')$ 平面的面积按 $1/(\beta\gamma)$ 缩小，这称为**绝热阻尼**。$(x, p_x/mc) = (x, \beta\gamma x')$ 平面的面积则守恒。因此定义

$$
\varepsilon_n = \beta\gamma\,\varepsilon
$$

$\varepsilon$ 称为几何发射度，$\varepsilon_n$ 称为归一化发射度。在线性、无耦合、无丝化的理想情况下 $\varepsilon_n$ 沿直线加速器保持不变，因此是比较不同能量处束流品质的合适指标。示例：质子从 1.527 MeV 加速到 10 MeV，$\beta\gamma$ 从 0.05708 增大到 0.14639，几何发射度缩小为原来的 1/2.565。

> 注意：上式使用参考粒子的 $\beta\gamma$。对能散很大的束流，逐个粒子用各自的 $\beta\gamma$ 计算的归一化发射度与此有差别，这一差别在低能电子束中可能显著。

## 发射度的定义 {#phase-space-emittance}

### rms 发射度

对 $N$ 个粒子，记中心化的二阶矩 $\langle x^2\rangle = \frac1N\sum (x - \bar x)^2$，$\langle x x'\rangle$、$\langle x'^2\rangle$ 同理。rms 发射度定义为

$$
\varepsilon_{\mathrm{rms}} = \sqrt{\langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2}
$$

它是相空间中"rms 椭圆"面积除以 $\pi$。rms 发射度的优点是对任何分布都有确定的值，而且在线性传输下（无耦合时）严格守恒；在非线性力作用下它会增长，因此 rms 发射度增长是衡量束流品质恶化的常用指标。

### 总发射度与百分比发射度

$\varepsilon_{100\%}$（总发射度、边缘发射度）是包含全部粒子的最小椭圆面积除以 $\pi$；$\varepsilon_{90\%}$、$\varepsilon_{99\%}$ 等则包含相应比例的粒子。对理想分布，它们与 rms 发射度有固定比值，见 [束流分布](#phase-space-distributions)。

### 单位

发射度的量纲是长度 × 角度。习惯写成 π·mm·mrad，其中的"π"只是一个标记，表示数值是面积除以 π：

$$
1\ \pi\cdot\mathrm{mm\cdot mrad} \;\to\; \varepsilon = 1\ \mathrm{mm\cdot mrad} = 1\ \mu\mathrm{m} = 10^{-6}\ \mathrm{m\cdot rad}
$$

引用发射度时必须说清三件事：rms 还是总（百分比）发射度，几何还是归一化，以及数值里是否已经除去了 π。

## Twiss 参数与 Courant–Snyder 不变量 {#phase-space-twiss}

一个以原点为中心的相空间椭圆可以写成

$$
\gamma_x x^2 + 2\alpha_x x x' + \beta_x x'^2 = \varepsilon, \qquad \beta_x\gamma_x - \alpha_x^2 = 1
$$

$\alpha_x$、$\beta_x$、$\gamma_x$ 称为 Twiss 参数或 Courant–Snyder 参数，椭圆面积为 $\pi\varepsilon$。三个参数中只有两个独立。对 rms 椭圆，它们与二阶矩的关系是

$$
\langle x^2\rangle = \beta_x\varepsilon, \qquad
\langle x x'\rangle = -\alpha_x\varepsilon, \qquad
\langle x'^2\rangle = \gamma_x\varepsilon
$$

反过来由粒子分布求 Twiss 参数：$\beta_x = \langle x^2\rangle/\varepsilon_{\mathrm{rms}}$、$\alpha_x = -\langle x x'\rangle/\varepsilon_{\mathrm{rms}}$。

### 椭圆的几何

- 最大位置 $x_{\max} = \sqrt{\varepsilon\beta_x}$，最大角度 $x'_{\max} = \sqrt{\varepsilon\gamma_x}$；
- 椭圆与 $x$ 轴的交点 $x = \sqrt{\varepsilon/\gamma_x}$，与 $x'$ 轴的交点 $x' = \sqrt{\varepsilon/\beta_x}$；
- 椭圆主轴相对 $x$ 轴的倾角 $\theta$ 满足 $\tan 2\theta = 2\alpha_x/(\gamma_x - \beta_x)$。

$\alpha_x$ 的符号表示束流的会聚 / 发散状态：

| $\alpha_x$ | $\langle x x'\rangle$ | 状态 |
|---|---|---|
| $\alpha_x > 0$ | $< 0$ | 会聚（向束腰运动） |
| $\alpha_x = 0$ | $= 0$ | 束腰（椭圆正立） |
| $\alpha_x < 0$ | $> 0$ | 发散（离开束腰） |

### Courant–Snyder 不变量

在线性聚焦系统中，Twiss 参数随 $s$ 变化（遵循 [线性光学](#linear-optics-twiss-transport) 中的传输规律），而单个粒子的量

$$
J = \gamma_x(s)\,x^2 + 2\alpha_x(s)\,x x' + \beta_x(s)\,x'^2
$$

沿束线保持不变，称为 Courant–Snyder 不变量。也就是说，粒子始终位于同一族形状不断变化、面积不变的椭圆上。粒子的轨迹可以写成 $x(s) = \sqrt{J\beta_x(s)}\cos\left(\psi(s) + \psi_0\right)$，其中 $\psi$ 是 betatron 相位，见 [线性光学](#linear-optics-phase-advance)。

## σ 矩阵与相椭圆 {#phase-space-sigma}

把二阶矩排成对称矩阵（束流矩阵、σ 矩阵）：

$$
\Sigma = \begin{pmatrix} \langle x^2\rangle & \langle x x'\rangle \\ \langle x x'\rangle & \langle x'^2\rangle \end{pmatrix}
= \varepsilon \begin{pmatrix} \beta_x & -\alpha_x \\ -\alpha_x & \gamma_x \end{pmatrix},
\qquad \varepsilon = \sqrt{\det\Sigma}
$$

rms 椭圆就是 $X\tp\Sigma^{-1}X = 1$，$X = (x, x')\tp$。σ 矩阵的好处是它在线性传输 $X_1 = M X_0$ 下的变换极其简单：

$$
\Sigma_1 = M\,\Sigma_0\,M\tp
$$

这一式对任何分布都成立（不只是椭圆分布），因为它只涉及二阶矩。AVAS 的线性预览正是用这种方式推进束流的。

### 六维 σ 矩阵

推广到六维，$\Sigma_{ij} = \langle u_i u_j\rangle$，$u = (x, x', y, y', z, \delta)$。对角块给出三个平面的投影发射度，非对角块描述平面之间的关联：

- 二极铁产生 $x$–$\delta$ 关联（色散），见 [二极铁](#dipole)；
- 螺线管使 $x$ 与 $y$ 平面耦合，此时投影发射度 $\varepsilon_x$、$\varepsilon_y$ 可以变化，但四维发射度 $\sqrt{\det\Sigma_4}$ 在线性辛变换下守恒，见 [螺线管](#solenoid)；
- 射频腔产生 $z$–$\delta$ 关联，见 [纵向运动](#longitudinal)。

## 束流分布与边缘发射度 {#phase-space-distributions}

`beam.txt` 的 `distribution V1 V2` 分别选择横向和纵向分布，可选 KV、GS、PB、WB：

| 代号 | 分布 | 四维相空间密度（$r^2$ 为归一化"半径"平方） | $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}}$（四维分布投影到 $x$–$x'$） |
|---|---|---|---|
| KV | Kapchinskij–Vladimirskij | 集中在四维椭球面 $r^2 = 1$ 上 | 4 |
| WB | 水袋（waterbag） | 在 $r^2 \le 1$ 内均匀 | 6 |
| PB | 抛物线（parabolic） | $\propto 1 - r^2$，$r^2 \le 1$ | 8 |
| GS | 高斯（Gaussian） | $\propto \exp(-r^2/2)$ | 无界（需截断） |

这些比值来自一个一般结果：若 $d$ 维椭球内的均匀分布投影到某一坐标，有 $\langle x^2\rangle = a^2/(d+2)$，$a$ 为半轴。几点说明：

- **KV 分布**在实空间 $(x, y)$ 的投影是均匀椭圆，其空间电荷力严格线性，因此是包络方程和空间电荷理论的基准分布（见 [空间电荷](#space-charge)）；但它的相空间结构很特殊，实际束流中并不存在。
- **WB**、**PB** 更接近实际束流，边缘清楚；**GS** 尾部最长。
- 对二维高斯分布，发射度为 $n\,\varepsilon_{\mathrm{rms}}$ 的 rms 相似椭圆内所含粒子比例为 $1 - \ee^{-n/2}$：$n$ = 1、4、6、8 时分别为 39.3 %、86.5 %、95.0 %、98.2 %。

> 注意：上表是理想四维分布的标准结果（Lapostolle、Reiser）。AVAS 如何生成纵向分布、高斯分布在哪里截断，手册没有说明；需要时可以读取运行输出的 `inData.dst`（模拟实际使用的初始分布）直接统计。

## 纵向相空间 {#phase-space-longitudinal}

纵向坐标 $(z, \delta)$ 与直线加速器中常用的 $(\Delta\varphi, \Delta W)$ 之间的换算是（见 [运动学](#kinematics-rf)）：

$$
\Delta\varphi = -\frac{2\pi}{\beta\lambda}\,z, \qquad
\Delta W = \beta^2\gamma\,mc^2\,\delta
$$

因此两种几何发射度之间相差一个常数因子

$$
\varepsilon_{\Delta\varphi\Delta W} = \frac{2\pi}{\beta\lambda}\,\beta^2\gamma\,mc^2\;\varepsilon_{z\delta}
$$

（以度为相位单位时把 $2\pi$ 换成 $360$。）$\lambda$ 是束流频率对应的波长。纵向相空间的运动——同步相位、相稳定区、纵向聚焦——在 [纵向运动](#longitudinal) 一章讨论。

## 在 AVAS 中 {#phase-space-in-avas}

```avas-params
beam:twissx beam:twissy beam:twissz beam:distribution beam:particlenumber beam:initpos beam:initmom
```

`twissx α β ε`（`twissy`、`twissz` 同）三个数的含义：

- $\alpha$：无量纲；
- $\beta$：单位 mm/mrad，数值上等于 m/rad；
- $\varepsilon$：**归一化 rms 发射度**，单位 π·mm·mrad。

> AVAS 约定：rms 尺寸由 $\sigma_x = \sqrt{\beta_x\,\varepsilon_n/(\beta\gamma)}$ 给出（$\beta_x$ 以 mm/mrad、$\varepsilon_n$ 以 mm·mrad 计时 $\sigma_x$ 以 mm 计）。`twissz` 采用同样的单位与归一化：$z$ 以 mm 计，$z' = \delta = \Delta p/p$ 以 mrad 计，归一化同样乘 $\beta\gamma$（界面的线性预览按这一约定换算，并已与内核比较）。其他程序的纵向发射度可能采用 $(\Delta\varphi, \Delta W)$ 或不同的归一化因子，交换数据时请逐项核对。

### 示例项目的初始束流

示例 `beam.txt`（质子 1.527 MeV，$\beta\gamma$ = 0.057076）：

```text
twissx -0.65785 0.37743 0.13768
twissy -0.6341 0.37292 0.14059
twissz 0.04275 0.37292 0.26506
distribution GS GS
```

由上面的约定换算：

| 平面 | 几何 rms 发射度 (mm·mrad) | rms 尺寸 | rms 角度 / 动量偏差 |
|---|---|---|---|
| x | 2.412 | 0.954 mm | 3.026 mrad |
| y | 2.463 | 0.958 mm | 3.043 mrad |
| z | 4.644 | 1.316 mm | $\delta_{\mathrm{rms}}$ = 3.532 × 10$^{-3}$ |

横向 $\alpha < 0$，说明束流在入口处是发散的。纵向 rms 长度 1.316 mm 对应 162.5 MHz 下 4.51° 的 rms 相宽；$\delta_{\mathrm{rms}}$ 对应 rms 能散 10.78 keV（0.71 %）；纵向几何 rms 发射度换算为 48.5 deg·keV。

### 其他关键字

- `particlenumber`：宏粒子数，写 1 时为单粒子模式（此时 `twiss*`、`current`、`distribution` 不生效）。
- `initpos`、`initmom`：整个束团的初始位置（mm）和角度（mrad）偏移，用来模拟入射偏差。
- `readparticledistribution`：从 `.dst`/`.edst` 文件读入粒子分布，此时除 `numofcharge` 外的关键字均不生效（界面用 `use_dst` 切换）。

输出中，`DataSet.txt` 第 16、18、20 列是 $x$、$y$、$z$ 的 rms 尺寸（m），与上面的 $\sigma$ 直接对应。

## 动手实验 {#phase-space-lab}

实验画出 rms 椭圆在一个薄透镜（焦距 `f`，单位 m；0 表示不放透镜）和其后长度 `L` 的漂移段前后的形状，并给出出口的 Twiss 参数和 rms 尺寸。参数 `beta` 以 mm/mrad 计，`emit` 为归一化 rms 发射度（π·mm·mrad）。

1. 从束腰出发（$\alpha_x = 0$、$\beta_x$ = 1 m），焦距 0.5 m 的透镜后漂移 0.5 m。按 [线性光学](#linear-optics-twiss-transport) 的公式，透镜后 $\alpha_x = \beta_x/f = 2$，漂移后 $\beta_x = 0.25$ m、$\alpha_x = -0.5$；新的束腰在透镜后 $\alpha_x/\gamma_x$ = 0.4 m 处，$\beta^* = 1/\gamma_x$ = 0.2 m。rms 尺寸从 1.87 mm 变为 0.94 mm。
2. 改变 `L`，找出椭圆正立（束腰）的位置；改变 `emit`，确认椭圆形状不变、只是整体放大。
3. 把 `W` 加大，几何发射度按 $1/(\beta\gamma)$ 缩小。

```avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
```

示例项目的 x 方向初始束流在 0.1 m 漂移段中的变化（发散束，尺寸增大）：

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.1
```

## 延伸阅读 {#phase-space-further}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.：发射度、刘维尔定理、KV 与其他分布、rms 等效束。
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.：横向动力学与 Twiss 参数。
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1101：rms 发射度概念的提出。
- K. Floettmann, "Some basic features of the beam emittance", *Phys. Rev. ST Accel. Beams* 6, 034202 (2003)：归一化发射度与能散的关系。
- 继续阅读：[线性光学](#linear-optics)。
