# 相空间与发射度

用一个透镜可以把束流聚成很小的斑点，代价是它随后以更大的角度散开；把束流调得很平行，它又变粗了。能不能让束流同时又细又平行？为什么不能？答案是一个在束线里几乎处处守恒的量——发射度。

本章说明：为什么描述粒子必须同时给出位置和角度；线性元件怎样在“位置–角度”平面里移动粒子，为什么被占据的面积不变（刘维尔定理），加速又为什么让它缩小（绝热阻尼）；怎样用 Twiss 参数和 σ 矩阵描述一束粒子的 rms 椭圆；纵向相空间如何与相位、能量对应；最后把这些量与 `beam.txt` 的 `twissx`、`twissy`、`twissz` 逐项对应。本章用到 [相对论运动学](#kinematics) 中的 $\beta\gamma$ 和 $\delta$；不同分布的形状、边界与尾部在 [初始分布](#distributions) 一章；椭圆沿束线的演化在 [线性光学](#linear-optics) 一章。

记号说明：本章用不带下标的 $\beta$、$\gamma$ 表示相对论因子，用带平面下标的 $\beta_x$、$\alpha_x$、$\gamma_x$ 表示 Twiss 参数。

## 为什么要同时看位置和角度 {#phase-space-coordinates}

设想在束线上同一个位置给两束质子各拍一张横截面照片，两个光斑一样大。一束正在会聚，另一束正在发散，10 cm 之后它们已经完全不同了。只给位置，并不能确定束流接下来怎样走：运动方程是二阶的，必须同时知道位置和速度方向。对沿 $s$ 前进的粒子，速度方向就是斜率 $x' = \dd x/\dd s$。

所以每个粒子在每个横向平面上用一对数 $(x, x')$ 描述，这个平面叫**相平面**；三个方向合起来是六维**相空间**。坐标取相对参考粒子（沿设计轨道运动的粒子）的偏离，$s$ 为沿轨道的距离（直线段上就是纵向位置 $z$）：

| 坐标 | 含义 | 单位（AVAS 内部 / 矩阵） |
|---|---|---|
| $x$、$y$ | 横向位置偏离 | m |
| $x' = \dd x/\dd s$、$y' = \dd y/\dd s$ | 横向角度（斜率） | rad |
| $z$ | 纵向位置偏离，在参考粒子前方为正 | m |
| $\delta = \Delta p/p$ | 相对动量偏差 | 1 |

在近轴近似下 $x' \approx p_x/p$，其中 $p$ 是参考动量。传输矩阵作用在列向量 $(x, x', y, y', z, \delta)\tp$ 上，见 [线性光学](#linear-optics)。

角度有多重要，看一个数就明白：示例束流入口处 rms 尺寸 0.954 mm、rms 角度 3.026 mrad。角度为 3 mrad 的粒子走 0.1 m 横向移动 0.3 mm，是束流尺寸的三分之一。

还要分清两种“快照”：

- **同一时刻**的分布：像在某一瞬间给整个束团拍照。t-code（场图元件、漂移段）按时间推进，粒子在同一时刻位于不同的纵向位置。
- **同一纵向位置**的分布：像终点线上的冲线照片，每个粒子在通过这一平面时被记录。z-code（矩阵元件）和传输矩阵理论把所有粒子放到同一个 $s$ 处比较。

对 rms 量来说两者的差别通常很小，但在束团较长、能散较大时要注意（例如 $z$ 与 $\delta$ 的关联）。`DataSet.txt` 按手册说明记录的是 s 坐标系下的束团参数。两种描述在 AVAS 中的由来见 [t-code 与 z-code](#intro-tracking)。

## 线性元件怎样移动相空间里的点 {#phase-space-shear}

先看两种最简单的元件在相平面里做什么。

**漂移段**：粒子沿直线走 $L$，位置增加 $L x'$，角度不变：

$$
\begin{pmatrix} x \\ x' \end{pmatrix}_1 = \begin{pmatrix} 1 & L \\ 0 & 1 \end{pmatrix}\begin{pmatrix} x \\ x' \end{pmatrix}_0
$$

角度为正的点向右移，角度为负的点向左移，移动量与角度成正比——相平面被**水平剪切**。

**薄透镜**（焦距 $f$）：位置不变，角度改变 $-x/f$，离轴越远偏折越大：

$$
\begin{pmatrix} x \\ x' \end{pmatrix}_1 = \begin{pmatrix} 1 & 0 \\ -1/f & 1 \end{pmatrix}\begin{pmatrix} x \\ x' \end{pmatrix}_0
$$

这是**竖直剪切**。剪切像推一摞扑克牌：形状变了，面积不变。数学上就是两个矩阵的行列式都为 1。任何漂移段和透镜的组合都是这些矩阵的乘积，行列式仍为 1，所以一团点在相平面里占据的面积不随线性传输改变。

例：示例束流的 x 平面经过 0.1 m 漂移段，$\beta_x$ 从 0.377 增大到 0.547 mm/mrad，rms 尺寸从 0.954 mm 变为 1.149 mm，rms 椭圆面积不变。

> **想一想**：开头的问题——能不能用透镜和漂移段把束流变得又细又平行？
>
> 答：不能。在束腰（椭圆正立、位置与角度不关联）处，rms 尺寸与 rms 角度的乘积正好等于 rms 发射度：$\sigma_x\sigma_{x'} = \varepsilon$。示例束流的几何 rms 发射度是 2.412 mm·mrad，若要在束腰处把 rms 尺寸压到 0.5 mm，rms 角度至少是 4.82 mrad。透镜只能在“细”和“平行”之间做交换。

## 刘维尔定理与绝热阻尼 {#phase-space-liouville}

### 刘维尔定理

上面的面积守恒是一个更一般定理的特例。若粒子的运动可以由哈密顿量描述（外加电磁场，以及平均场近似下的空间电荷），则在正则坐标 $(x, p_x, y, p_y, z, p_z)$ 构成的六维相空间中，相空间密度沿粒子轨迹不变，任意一团粒子占据的相空间体积守恒。若三个方向的运动互不耦合，每个二维投影的面积也分别守恒。相空间里的粒子像不可压缩的流体：可以被拉长、扭曲，但不能被压缩。

刘维尔定理不保证“有效”发射度守恒。非线性力（场的非线性、非线性空间电荷、射频场的正弦形状）让不同振幅的粒子以不同频率转动，相空间中的椭圆被扭成 S 形或螺旋。被占据的面积不变，但包住它所需的椭圆面积和 rms 发射度都会增大，这称为**丝化**（filamentation）。可以想象把一勺牛奶搅进咖啡：牛奶的体积不变，却散布到整杯，任何想把牛奶圈起来的轮廓都得画得很大。这个类比在“不可压缩”这一点上是对的；它失效的地方是牛奶会靠分子扩散真正混合，而哈密顿运动中的细丝永远不会真正融合。

粒子损失、碰撞、剥离等过程不满足刘维尔定理的前提。

### 绝热阻尼与归一化发射度

角度 $x' = p_x/p$ 不是正则动量。理想的纵向加速只增大 $p$ 而不改变 $p_x$，于是 $x'$ 按 $1/p$ 缩小，$(x, x')$ 平面的面积按 $1/(\beta\gamma)$ 缩小，这称为**绝热阻尼**。正则平面 $(x, p_x/mc) = (x, \beta\gamma x')$ 的面积则守恒。因此定义

$$
\varepsilon_n = \beta\gamma\,\varepsilon
$$

$\varepsilon$ 称为几何发射度，$\varepsilon_n$ 称为归一化发射度。在线性、无耦合、无丝化的理想情况下 $\varepsilon_n$ 沿直线加速器保持不变，因此是比较不同能量处束流品质的合适指标。

例：示例束流的归一化 rms 发射度为 0.1377 π·mm·mrad，在 1.527 MeV（$\beta\gamma$ = 0.05708）处几何发射度为 2.412 mm·mrad。加速到 10 MeV，$\beta\gamma$ 增大到 0.14639，几何发射度缩小为原来的 1/2.565，即 0.940 mm·mrad，归一化发射度不变。

> **想一想**：绝热阻尼让几何发射度变小了，束流是被“冷却”了吗？
>
> 答：没有。粒子的横向动量 $p_x$ 一点没变，只是纵向动量变大，轨迹变得更“顺”。归一化发射度——正比于横向动量的散布——不变。真正的冷却（电子冷却、随机冷却）要减小 $p_x$ 的散布，需要耗散过程，不受刘维尔定理约束。

> 注意：上式使用参考粒子的 $\beta\gamma$。对能散很大的束流，逐个粒子用各自的 $\beta\gamma$ 计算的归一化发射度与此有差别，这一差别在低能电子束中可能显著。

## 发射度的定义 {#phase-space-emittance}

### rms 发射度

对 $N$ 个粒子，记中心化的二阶矩 $\langle x^2\rangle = \frac1N\sum (x - \bar x)^2$，$\langle x x'\rangle$、$\langle x'^2\rangle$ 同理。rms 发射度定义为

$$
\varepsilon_{\mathrm{rms}} = \sqrt{\langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2}
$$

它是相空间中“rms 椭圆”面积除以 $\pi$。第二项 $\langle x x'\rangle^2$ 的作用是扣除位置与角度的关联：一束只是在会聚或发散的束流，$\langle x^2\rangle\langle x'^2\rangle$ 可以很大，但关联部分是可以用透镜“拉直”的，不代表真正的弥散。rms 发射度对任何分布都有确定的值，在线性传输下（无耦合时）严格守恒，在非线性力作用下增长，所以 rms 发射度增长是衡量束流品质恶化的常用指标。为什么二阶矩是自然的度量、它对尾部有多敏感，见 [初始分布](#distributions-rms)。

### 总发射度与百分比发射度

$\varepsilon_{100\%}$（总发射度、边缘发射度）是包含全部粒子的最小椭圆面积除以 $\pi$；$\varepsilon_{90\%}$、$\varepsilon_{99\%}$ 等则包含相应比例的粒子。它们与 rms 发射度的比值取决于分布形状：KV 为 4，四维水袋为 6，抛物线为 8，高斯没有总发射度，见 [四种标准分布](#distributions-types)。

### 单位

发射度的量纲是长度 × 角度。习惯写成 π·mm·mrad，其中的“π”只是一个标记，表示数值是面积除以 π：

$$
1\ \pi\cdot\mathrm{mm\cdot mrad} \;\to\; \varepsilon = 1\ \mathrm{mm\cdot mrad} = 1\ \mu\mathrm{m} = 10^{-6}\ \mathrm{m\cdot rad}
$$

引用发射度时必须说清三件事：rms 还是总（百分比）发射度，几何还是归一化，以及数值里是否已经除去了 π。

## Twiss 参数与 Courant–Snyder 不变量 {#phase-space-twiss}

一个以原点为中心的相空间椭圆可以写成

$$
\gamma_x x^2 + 2\alpha_x x x' + \beta_x x'^2 = \varepsilon, \qquad \beta_x\gamma_x - \alpha_x^2 = 1
$$

$\alpha_x$、$\beta_x$、$\gamma_x$ 称为 Twiss 参数或 Courant–Snyder 参数，椭圆面积为 $\pi\varepsilon$。三个参数中只有两个独立：它们描述椭圆的**形状和取向**，$\varepsilon$ 描述它的**大小**。对 rms 椭圆，它们与二阶矩的关系是

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

示例束流 x 平面（$\alpha_x = -0.658$，$\beta_x = 0.377$ mm/mrad，$\gamma_x = 3.796$ mrad/mm，$\varepsilon = 2.412$ mm·mrad）：rms 椭圆的 $x_{\max}$ = 0.954 mm、$x'_{\max}$ = 3.026 mrad，与两轴的交点为 0.797 mm 和 2.528 mrad。

$\alpha_x$ 的符号表示束流的会聚 / 发散状态：

| $\alpha_x$ | $\langle x x'\rangle$ | 状态 |
|---|---|---|
| $\alpha_x > 0$ | $< 0$ | 会聚（向束腰运动） |
| $\alpha_x = 0$ | $= 0$ | 束腰（椭圆正立） |
| $\alpha_x < 0$ | $> 0$ | 发散（离开束腰） |

在漂移段中，束腰位于 $\alpha_x/\gamma_x$ 处（正值在下游），束腰处 $\beta^* = 1/\gamma_x$。示例束流 $\alpha_x/\gamma_x = -0.173$ m：束腰在入口上游 0.173 m 处，那里 $\beta^*$ = 0.263 mm/mrad，rms 尺寸 0.797 mm——正是椭圆与 $x$ 轴的交点，因为漂移段的剪切不改变 $x' = 0$ 的那些点的位置。

### Courant–Snyder 不变量

在线性聚焦系统中，Twiss 参数随 $s$ 变化（遵循 [线性光学](#linear-optics) 中的传输规律），而单个粒子的量

$$
J = \gamma_x(s)\,x^2 + 2\alpha_x(s)\,x x' + \beta_x(s)\,x'^2
$$

沿束线保持不变，称为 Courant–Snyder 不变量。也就是说，每个粒子始终位于同一族形状不断变化、面积不变的椭圆中属于自己的那一个上；$J$ 就是这个椭圆的“编号”，它衡量粒子振荡的幅度。粒子的轨迹可以写成 $x(s) = \sqrt{J\beta_x(s)}\cos\left(\psi(s) + \psi_0\right)$，其中 $\psi$ 是 betatron 相位，见 [线性光学](#linear-optics)。所以 $\sqrt{\beta_x}$ 决定了振荡幅度沿束线怎样放大缩小，$\sqrt{J}$ 决定这个粒子的“个头”。

## σ 矩阵与相椭圆 {#phase-space-sigma}

把二阶矩排成对称矩阵（束流矩阵、σ 矩阵）：

$$
\Sigma = \begin{pmatrix} \langle x^2\rangle & \langle x x'\rangle \\ \langle x x'\rangle & \langle x'^2\rangle \end{pmatrix}
= \varepsilon \begin{pmatrix} \beta_x & -\alpha_x \\ -\alpha_x & \gamma_x \end{pmatrix},
\qquad \varepsilon = \sqrt{\det\Sigma}
$$

rms 椭圆就是 $X\tp\Sigma^{-1}X = 1$，$X = (x, x')\tp$。示例束流 x 平面：$\langle x^2\rangle$ = 0.910 mm²、$\langle x x'\rangle$ = 1.587 mm·mrad、$\langle x'^2\rangle$ = 9.157 mrad²，$\sqrt{\det\Sigma}$ = 2.412 mm·mrad。

σ 矩阵的好处是它在线性传输 $X_1 = M X_0$ 下的变换极其简单：

$$
\Sigma_1 = M\,\Sigma_0\,M\tp
$$

推导只需一步：$\langle X_1 X_1\tp\rangle = M\langle X_0 X_0\tp\rangle M\tp$。这一式对任何分布都成立（不只是椭圆分布），因为它只涉及二阶矩；由 $\det\Sigma_1 = (\det M)^2\det\Sigma_0$ 和 $\det M = 1$ 也立即得到 rms 发射度守恒。AVAS 的线性预览正是用这种方式推进束流的。

### 六维 σ 矩阵

推广到六维，$\Sigma_{ij} = \langle u_i u_j\rangle$，$u = (x, x', y, y', z, \delta)$。对角块给出三个平面的投影发射度，非对角块描述平面之间的关联：

- 二极铁产生 $x$–$\delta$ 关联（色散），见 [二极铁](#dipole)；
- 螺线管使 $x$ 与 $y$ 平面耦合，此时投影发射度 $\varepsilon_x$、$\varepsilon_y$ 可以变化，但四维发射度 $\sqrt{\det\Sigma_4}$ 在线性辛变换下守恒，见 [螺线管](#solenoid)；
- 射频腔产生 $z$–$\delta$ 关联，见 [纵向运动](#longitudinal)。

## 动手：让椭圆通过透镜和漂移段 {#phase-space-lab}

实验画出 rms 椭圆在一个薄透镜（焦距 `f`，单位 m；0 表示不放透镜）和其后长度 `L` 的漂移段前后的形状，并给出出口的 Twiss 参数和 rms 尺寸。参数 `beta` 以 mm/mrad 计，`emit` 为归一化 rms 发射度（π·mm·mrad）。

先从束腰出发（$\alpha_x = 0$、$\beta_x$ = 1 m），焦距 0.5 m 的透镜后漂移 0.5 m。按传输规律，透镜后 $\alpha_x = \beta_x/f = 2$，漂移后 $\beta_x = 0.25$ m、$\alpha_x = -0.5$；新的束腰在透镜后 $\alpha_x/\gamma_x$ = 0.4 m 处，$\beta^* = 1/\gamma_x$ = 0.2 m。rms 尺寸从 1.87 mm 变为 0.94 mm，而面积（实验中的“椭圆面积 / π”）始终是 3.504 mm·mrad。

```avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
```

可以试的事：改变 `L`，找出椭圆正立（束腰）的位置；改变 `emit`，确认椭圆形状不变、只是整体放大；把 `W` 加大，几何发射度按 $1/(\beta\gamma)$ 缩小。

示例项目的 x 方向初始束流在 0.1 m 漂移段中的变化（发散束，$\alpha_x$ 从 −0.658 变为 −1.037，尺寸从 0.954 mm 增大到 1.149 mm）：

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.1
```

## 纵向相空间 {#phase-space-longitudinal}

纵向也有一对共轭量：粒子在参考粒子前方多远（$z$），以及动量偏差多少（$\delta$）。在直线加速器里人们更常用相位和能量 $(\Delta\varphi, \Delta W)$：射频腔“看”的是粒子何时到达，而不是在哪里。在参考粒子前方 $z$ 处的粒子早到 $z/(\beta c)$，看到的射频相位较小。两组坐标的换算是（见 [运动学](#kinematics-rf)）：

$$
\Delta\varphi = -\frac{2\pi}{\beta\lambda}\,z, \qquad
\Delta W = \beta^2\gamma\,mc^2\,\delta
$$

因此两种几何发射度之间相差一个常数因子

$$
\varepsilon_{\Delta\varphi\Delta W} = \frac{2\pi}{\beta\lambda}\,\beta^2\gamma\,mc^2\;\varepsilon_{z\delta}
$$

（以度为相位单位时把 $2\pi$ 换成 $360$。）$\lambda$ 是束流频率对应的波长。按示例 `beam.txt` 的 `twissz` 参数：纵向 rms 长度 1.316 mm 对应 162.5 MHz 下 4.51° 的 rms 相宽；$\delta_{\mathrm{rms}} = 3.532\times10^{-3}$ 对应 rms 能散 10.78 keV（0.71 %）；纵向几何 rms 发射度 4.644 mm·mrad 换算为 48.5 deg·keV（直接统计粒子文件 `part_rfq.dst` 则为 1.24 mm、4.26°、11.4 keV、48.7 deg·keV，两套口径的对照见[初始分布：在 AVAS 中](#distributions-in-avas)）。纵向相空间的运动——同步相位、相稳定区、纵向聚焦——在 [纵向运动](#longitudinal) 一章讨论。

## 容易误解的地方 {#phase-space-misconceptions}

- **“π·mm·mrad 里的 π 要乘进去。”** 不要乘。π 只是标记，说明数值已经是面积除以 π。不同文献在这一点上不统一，交换数据时要确认。
- **“rms 发射度和总发射度只差一个固定倍数。”** 倍数取决于分布：KV 为 4，高斯根本没有总发射度，见 [初始分布](#distributions)。
- **“发射度在加速中守恒。”** 守恒的是归一化发射度；几何发射度按 $1/(\beta\gamma)$ 缩小。比较不同能量处的束流必须用归一化发射度。
- **“刘维尔定理保证 rms 发射度不变。”** 刘维尔定理保证的是被占据的相空间体积。非线性力造成的丝化让 rms 发射度增长，而体积不变。
- **“α 为正就是发散。”** 恰好相反：$\alpha_x > 0$ 表示 $\langle x x'\rangle < 0$，束流在会聚。
- **“椭圆的倾角是物理量。”** 倾角取决于 $x$ 与 $x'$ 用什么单位（例如 mm 配 mrad 还是 mm 配 rad），换单位倾角就变；Twiss 参数换单位时也要一起换。
- **“束流变小了，发射度就变小了。”** 尺寸由 $\sqrt{\beta_x\varepsilon}$ 决定，聚焦改变的是 $\beta_x$，不是 $\varepsilon$。
- **“螺线管让 $\varepsilon_x$ 增大了，束流品质变差了。”** 螺线管把 $x$ 与 $y$ 耦合起来，投影发射度可以变化，四维发射度不变；出了螺线管耦合可以消失，见 [螺线管](#solenoid)。

## 在 AVAS 中 {#phase-space-in-avas}

### twiss 关键字

```avas-params
beam:twissx beam:twissy beam:twissz beam:distribution beam:particlenumber beam:initpos beam:initmom
```

`twissx α β ε`（`twissy`、`twissz` 同）三个数的含义：

- $\alpha$：无量纲；
- $\beta$：单位 mm/mrad，数值上等于 m/rad；
- $\varepsilon$：**归一化 rms 发射度**，单位 π·mm·mrad。

> **AVAS 约定**：rms 尺寸由 $\sigma_x = \sqrt{\beta_x\,\varepsilon_n/(\beta\gamma)}$ 给出（$\beta_x$ 以 mm/mrad、$\varepsilon_n$ 以 mm·mrad 计时 $\sigma_x$ 以 mm 计）。`twissz` 采用同样的单位与归一化：$z$ 以 mm 计，$z' = \delta = \Delta p/p$ 以 mrad 计，归一化同样乘 $\beta\gamma$（界面的线性预览按这一约定换算，并已与内核比较）。其他程序的纵向发射度可能采用 $(\Delta\varphi, \Delta W)$ 或不同的归一化因子，交换数据时请逐项核对。

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

- `distribution`：横向与纵向的分布类型（KV、GS、PB、WB），见 [初始分布：在 AVAS 中](#distributions-in-avas)。
- `particlenumber`：宏粒子数，写 1 时为单粒子模式（此时 `twiss*`、`current`、`distribution` 不生效）。
- `initpos`、`initmom`：整个束团的初始位置（mm）和角度（mrad）偏移，用来模拟入射偏差。
- `readparticledistribution`：从 `.dst`/`.edst` 文件读入粒子分布，此时除 `numofcharge` 外的关键字均不生效（界面用 `use_dst` 切换）。

输出中，`DataSet.txt` 第 16、18、20 列是 $x$、$y$、$z$ 的 rms 尺寸（m），与上面的 $\sigma$ 直接对应；第 0 列的参考粒子动能可以换算出每处的 $\beta\gamma$，把归一化发射度换成几何发射度。

## 延伸阅读 {#phase-space-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：发射度、刘维尔定理、KV 与其他分布、rms 等效束。
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：横向动力学与 Twiss 参数。
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1101：rms 发射度概念的提出。
- K. Floettmann, "Some basic features of the beam emittance", *Phys. Rev. ST Accel. Beams* 6, 034202 (2003)：归一化发射度与能散的关系。
- 继续阅读：[初始分布](#distributions)、[线性光学](#linear-optics)。
