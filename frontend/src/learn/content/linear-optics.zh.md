# 线性光学

当外力与偏离量成正比时，粒子的运动方程是线性的，任意一段束线都可以用一个传输矩阵描述，束流的二阶矩和 Twiss 参数按简单的规则传输。线性光学是束线设计、匹配和稳定性分析的骨架，也是 AVAS 线性包络预览和本入门动手实验所用的模型。本章从 Hill 方程出发，给出漂移段、薄透镜和厚四极铁的传输矩阵，说明辛性、Twiss 参数的传输、betatron 相移、周期系统的稳定条件和匹配束，最后介绍 AVAS 线性预览所用的 6×6 形式。

记号同 [相空间](#phase-space)：$\beta$、$\gamma$ 为相对论因子，$\beta_x$、$\alpha_x$、$\gamma_x$ 为 Twiss 参数。

## 运动方程与 Hill 方程 {#linear-optics-hill}

### 近轴运动方程

设粒子沿 $s$ 运动，横向动量 $p_x \approx p\,x'$。由 $\dd p_x/\dd t = F_x$ 和 $\dd/\dd t = v\,\dd/\dd s$ 得

$$
x'' + \frac{p'}{p}\,x' = \frac{F_x}{p\,v}
$$

其中 $p'/p = (\beta\gamma)'/(\beta\gamma)$ 只在加速段不为零，它就是 [绝热阻尼](#phase-space-liouville) 项。对磁场 $F_x = -q v B_y$，对电场 $F_x = qE_x$，于是右边分别为 $-B_y/(B\rho)$ 和 $E_x/(E\rho)$（[刚度](#kinematics-rigidity)）。

### Hill 方程

若横向场对偏离量是线性的，例如四极铁 $B_y = G\,x$、$B_x = G\,y$，在无加速的段落中运动方程化为

$$
x'' + K_x(s)\,x = 0, \qquad y'' + K_y(s)\,y = 0
$$

这就是 Hill 方程。各元件的聚焦函数：

| 元件 | $K_x$ | $K_y$ | 详见 |
|---|---|---|---|
| 漂移段 | 0 | 0 | [漂移段](#drift) |
| 四极铁（梯度 $G$） | $k = G/(B\rho)$ | $-k$ | [四极铁](#quadrupole) |
| 扇形二极铁（场指数 $N$，偏转在 $x$ 平面） | $(1-N)/\rho^2$ | $N/\rho^2$ | [二极铁](#dipole) |
| 螺线管（在拉莫尔旋转坐标系中） | $k_s^2$，$k_s = B/(2B\rho)$ | $k_s^2$ | [螺线管](#solenoid) |

对正电荷粒子，$G > 0$ 的四极铁在 $x$ 方向聚焦、$y$ 方向散焦：在 $x > 0$ 处 $B_y > 0$，$\boldsymbol v \times \boldsymbol B$ 指向 $-x$。四极铁不可能同时在两个平面聚焦（无源区 $\nabla\times\boldsymbol B = 0$ 要求 $\partial B_y/\partial x = \partial B_x/\partial y$，于是 $K_y = -K_x$），所以需要交替排列，见 [周期系统](#linear-optics-periodic)。

## 传输矩阵 {#linear-optics-matrices}

线性方程的解可以写成初值的线性组合 $X(s) = M(s \mid s_0)\,X(s_0)$，$X = (x, x')\tp$。设 $C(s)$、$S(s)$ 是满足 $C(s_0) = 1, C'(s_0) = 0$ 与 $S(s_0) = 0, S'(s_0) = 1$ 的两个基本解（余弦型与正弦型），则

$$
M = \begin{pmatrix} C & S \\ C' & S' \end{pmatrix}
$$

一段束线的总矩阵是各元件矩阵按顺序相乘，**后经过的元件写在左边**：$M = M_n \cdots M_2 M_1$。

### 漂移段（长度 $L$）

$$
M_{\mathrm{drift}} = \begin{pmatrix} 1 & L \\ 0 & 1 \end{pmatrix}
$$

### 薄透镜（焦距 $f$）

$$
M_{\mathrm{thin}} = \begin{pmatrix} 1 & 0 \\ -1/f & 1 \end{pmatrix}
$$

$f > 0$ 为聚焦，$f < 0$ 为散焦。

### 厚四极铁（长度 $L$，$k > 0$，$\phi = \sqrt{k}\,L$）

聚焦平面与散焦平面分别为

$$
M_{\mathrm F} = \begin{pmatrix} \cos\phi & \dfrac{\sin\phi}{\sqrt{k}} \\ -\sqrt{k}\sin\phi & \cos\phi \end{pmatrix}, \qquad
M_{\mathrm D} = \begin{pmatrix} \cosh\phi & \dfrac{\sinh\phi}{\sqrt{k}} \\ \sqrt{k}\sinh\phi & \cosh\phi \end{pmatrix}
$$

厚透镜的等效焦距由 $1/f = -M_{21}$ 给出，即聚焦平面 $1/f = \sqrt{k}\sin\phi$；当 $\phi \ll 1$ 时退化为薄透镜 $1/f \approx kL$，称为薄透镜近似。

示例：1.527 MeV 质子（$B\rho$ = 0.17863 T·m），$G$ = 20 T/m、$L$ = 0.05 m 的四极铁，$k$ = 111.96 m$^{-2}$，$\phi$ = 0.529：

$$
M_{\mathrm F} = \begin{pmatrix} 0.86328 & 0.04770\ \mathrm m \\ -5.34063\ \mathrm m^{-1} & 0.86328 \end{pmatrix}, \qquad
M_{\mathrm D} = \begin{pmatrix} 1.14325 & 0.05237\ \mathrm m \\ 5.86300\ \mathrm m^{-1} & 1.14325 \end{pmatrix}
$$

聚焦平面等效焦距 0.1872 m，散焦平面 $-0.1706$ m，薄透镜近似给出 $\pm 0.1786$ m。$\phi \approx 0.5$ 时薄透镜近似的误差已有约 5 %，这在低能强聚焦段很常见，所以 AVAS 的矩阵元件和线性预览都使用厚透镜矩阵。

## 辛性与行列式 {#linear-optics-symplectic}

哈密顿系统的线性传输矩阵是**辛矩阵**：

$$
M\tp J\,M = J, \qquad J = \begin{pmatrix} 0 & 1 \\ -1 & 0 \end{pmatrix}
$$

（多维时 $J$ 是各平面 2×2 块组成的分块对角矩阵。）对 2×2 矩阵，辛条件等价于 $\det M = CS' - SC' = 1$。上面的所有矩阵都满足这一点，可以用来检查手算或程序的结果。辛性保证相空间面积守恒，也就是 [刘维尔定理](#phase-space-liouville) 在线性情况下的体现；它还给出逆矩阵的简单形式

$$
M^{-1} = \begin{pmatrix} S' & -S \\ -C' & C \end{pmatrix}
$$

**有加速时**，在 $(x, x')$ 坐标下 $\det M = (\beta\gamma)_0/(\beta\gamma)_1 < 1$，这就是绝热阻尼；改用 $(x, \beta\gamma\,x')$ 坐标，矩阵重新成为辛矩阵。

> 注意：线性预览中的 drift、quad、solenoid、bend、edge 的 6×6 矩阵都是辛矩阵（已在 $(x, x')$、$(y, y')$、$(z, \delta)$ 三对共轭坐标下数值核对）；行列式不为 1 的只有场图中带加速的切片。

## σ 矩阵与 Twiss 参数的传输 {#linear-optics-twiss-transport}

### σ 矩阵

由 $X_1 = MX_0$ 立即得到二阶矩的传输规律（[σ 矩阵](#phase-space-sigma)）：

$$
\Sigma_1 = M\,\Sigma_0\,M\tp
$$

由于 $\det\Sigma_1 = (\det M)^2\det\Sigma_0$，辛传输下 rms 发射度不变。

### Twiss 参数

把 $\Sigma = \varepsilon\begin{pmatrix} \beta_x & -\alpha_x \\ -\alpha_x & \gamma_x \end{pmatrix}$ 代入，得到

$$
\begin{pmatrix} \beta_x \\ \alpha_x \\ \gamma_x \end{pmatrix}_1 =
\begin{pmatrix} C^2 & -2CS & S^2 \\ -CC' & CS' + SC' & -SS' \\ C'^2 & -2C'S' & S'^2 \end{pmatrix}
\begin{pmatrix} \beta_x \\ \alpha_x \\ \gamma_x \end{pmatrix}_0
$$

两个常用特例：

- **漂移段**：$\beta_x(s) = \beta_0 - 2\alpha_0 s + \gamma_0 s^2$，$\alpha_x(s) = \alpha_0 - \gamma_0 s$，$\gamma_x$ 不变。$\beta_x$ 是 $s$ 的抛物线，束腰位于 $s^* = \alpha_0/\gamma_0$，束腰处 $\beta^* = 1/\gamma_0$。离开束腰距离 $s$ 后 $\beta_x = \beta^* + s^2/\beta^*$。
- **薄透镜**：$\beta_x$ 不变，$\alpha_1 = \alpha_0 + \beta_0/f$。聚焦透镜使 $\alpha_x$ 增大，即使束流向会聚方向转。

例如束腰 $\beta_0$ = 1 m 处放 $f$ = 0.5 m 的透镜：透镜后 $\alpha_x = 2$、$\gamma_x = 5$ m$^{-1}$，新束腰在其后 0.4 m 处，$\beta^*$ = 0.2 m，可以在 [相空间实验](#phase-space-lab) 中验证。

### 微分形式

在聚焦函数为 $K(s)$ 的系统中，Twiss 参数满足

$$
\beta_x' = -2\alpha_x, \qquad \alpha_x' = K\beta_x - \gamma_x
$$

令 $w = \sqrt{\beta_x}$，则 $w'' + K w - 1/w^3 = 0$；乘以 $\sqrt{\varepsilon}$ 得到 rms 包络 $a = \sqrt{\varepsilon\beta_x}$ 的方程 $a'' + Ka - \varepsilon^2/a^3 = 0$。最后一项是发射度对包络的"压力"，加入空间电荷后就是 [空间电荷](#space-charge) 一章的包络方程。

## β 函数与 betatron 相移 {#linear-optics-phase-advance}

Hill 方程的一般解可以写成（Floquet 形式）

$$
x(s) = \sqrt{J\beta_x(s)}\,\cos\left(\psi(s) + \psi_0\right), \qquad
\psi(s) = \int_{s_0}^{s} \frac{\dd\sigma}{\beta_x(\sigma)}
$$

$J$ 是 [Courant–Snyder 不变量](#phase-space-twiss)，$\psi$ 是 betatron 相位。由此 $\beta_x$ 有两个意义：$\sqrt{J\beta_x}$ 是单个粒子振荡的包络，$\sqrt{\varepsilon\beta_x}$ 是束流的 rms 尺寸；同时 $\beta_x$ 越小，相位推进越快。

用两端的 Twiss 参数和相移 $\Delta\psi$，任意传输矩阵可以写成

$$
M = \begin{pmatrix}
\sqrt{\dfrac{\beta_1}{\beta_0}}\left(\cos\Delta\psi + \alpha_0\sin\Delta\psi\right) & \sqrt{\beta_0\beta_1}\,\sin\Delta\psi \\
\dfrac{(\alpha_0 - \alpha_1)\cos\Delta\psi - (1 + \alpha_0\alpha_1)\sin\Delta\psi}{\sqrt{\beta_0\beta_1}} & \sqrt{\dfrac{\beta_0}{\beta_1}}\left(\cos\Delta\psi - \alpha_1\sin\Delta\psi\right)
\end{pmatrix}
$$

反过来，已知 $M$ 和入口 Twiss 参数时，相移由

$$
\tan\Delta\psi = \frac{M_{12}}{\beta_0 M_{11} - \alpha_0 M_{12}}
$$

给出（按 $M_{12}$ 的符号选取象限）。相移只取决于入口 Twiss 参数和矩阵，与发射度无关。

## 周期系统与稳定性 {#linear-optics-periodic}

直线加速器的聚焦通常由重复的周期单元组成（如 FODO：聚焦四极—漂移—散焦四极—漂移，或"螺线管 + 腔"的周期）。设一个周期的矩阵为 $M$。

### 稳定条件

粒子经过 $n$ 个周期后的坐标为 $M^n X_0$。$M$ 的特征值 $\lambda$ 满足 $\lambda^2 - (\Tr M)\lambda + 1 = 0$（因 $\det M = 1$）。当 $\lvert \Tr M \rvert < 2$ 时两个特征值为 $\ee^{\pm i\mu}$，运动有界；当 $\lvert \Tr M \rvert > 2$ 时有一个特征值模大于 1，振幅指数增长。因此稳定条件是

$$
\left\lvert \tfrac12 \Tr M \right\rvert < 1, \qquad \cos\mu = \tfrac12 \Tr M = \tfrac12\left(M_{11} + M_{22}\right)
$$

$\mu$ 是每个周期的相移（零流强时记为 $\sigma_0$）。

### 周期 Twiss 参数

稳定时 $M$ 可以写成

$$
M = \begin{pmatrix} \cos\mu + \alpha_x\sin\mu & \beta_x\sin\mu \\ -\gamma_x\sin\mu & \cos\mu - \alpha_x\sin\mu \end{pmatrix}
$$

由此得到周期解：$\beta_x = M_{12}/\sin\mu$，$\alpha_x = (M_{11} - M_{22})/(2\sin\mu)$，$\gamma_x = -M_{21}/\sin\mu$，其中 $\sin\mu$ 的符号取得使 $\beta_x > 0$。它们只由周期结构决定，与束流无关。

### 薄透镜 FODO

两个焦距 $\pm f$ 的薄透镜相距 $L$（周期长度 $2L$），可以得到

$$
\sin\frac{\mu}{2} = \frac{L}{2f}, \qquad
\beta_{\max,\min} = \frac{2L\left(1 \pm \sin(\mu/2)\right)}{\sin\mu}
$$

$\beta_{\max}$ 在聚焦透镜中心，$\beta_{\min}$ 在散焦透镜中心。稳定要求 $f > L/2$。平均 β 可以用平滑近似 $\bar\beta \approx 2L/\mu$ 估计。

### 示例

1.527 MeV 质子，周期为 QF（0.05 m）— 漂移 0.10 m — QD（0.05 m）— 漂移 0.10 m，$G$ = 20 T/m：

- 厚透镜矩阵：$\tfrac12\Tr M = 0.726$，$\mu$ = 43.45°（两个平面相同），QF 入口处 $\beta_x$ = 0.546 m、$\beta_y$ = 0.303 m；
- 薄透镜估计（$f$ = 0.1786 m，透镜间距 0.15 m）：$\sin(\mu/2) = 0.420$，$\mu$ = 49.65°，偏大约 6°；
- 把梯度增大到约 38.2 T/m 时 $\mu$ = 90°；约 54.1 T/m 时 $\tfrac12\lvert\Tr M\rvert$ 达到 1，束流失稳（薄透镜近似给出 47.6 T/m）。

FODO 单元的设计要点（相移的选择、与腔的配置、平滑近似）在 [束线设计](#lattice-design) 中详述。

> 注意：零流强稳定只是必要条件。有空间电荷时每周期相移会被压低（$\sigma < \sigma_0$），而 $\sigma_0 > 90°$ 时强流束容易出现包络不稳定和参数共振，所以强流直线加速器通常取 $\sigma_0 < 90°$，见 [空间电荷](#space-charge)。

## 匹配束与失配 {#linear-optics-matched}

如果入口束流的 Twiss 参数恰好等于周期解，那么经过每个周期后束流椭圆回到原来的形状，rms 尺寸沿束线按周期重复，这称为**匹配束**。否则椭圆在每个周期后转过 $\mu$ 角并改变形状，包络出现"拍"（beating）：它的相位每个周期推进 $2\mu$（包络振荡频率是 betatron 振荡的两倍），最大尺寸增大。

失配程度常用失配因子衡量。对两组 Twiss 参数（束流与周期解）定义

$$
R = \beta_1\gamma_2 + \beta_2\gamma_1 - 2\alpha_1\alpha_2, \qquad
M_{\mathrm{mis}} = \sqrt{\frac{R + \sqrt{R^2 - 4}}{2}} - 1
$$

完全匹配时 $R = 2$、$M_{\mathrm{mis}} = 0$。若只是 $\beta$ 不同（$\alpha = 0$，$\beta_1 = \lambda\beta_2$），$M_{\mathrm{mis}} = \sqrt{\lambda} - 1$，即最大 rms 尺寸增大的比例。在线性系统中失配束的 rms 发射度不变，只是包络振荡；而在有非线性力的真实束线中，失配椭圆会丝化，完全丝化后的 rms 发射度增大到原来的 $R/2$ 倍。如何把束流匹配到周期结构见 [匹配](#matching)。

## 6×6 形式与 AVAS 线性预览 {#linear-optics-6d}

AVAS 的线性预览把横向和纵向放在一起，用作用于 $(x, x', y, y', z, \delta)$ 的 6×6 矩阵推进 6×6 σ 矩阵 $\Sigma_1 = M\Sigma_0 M\tp$。无耦合元件的矩阵是分块对角的：

$$
M = \begin{pmatrix} M_x & 0 & 0 \\ 0 & M_y & 0 \\ 0 & 0 & M_z \end{pmatrix}, \qquad
M_z^{\mathrm{drift}} = \begin{pmatrix} 1 & L/\gamma^2 \\ 0 & 1 \end{pmatrix}
$$

纵向块的 $L/\gamma^2$ 来自 $\Delta\beta/\beta = \delta/\gamma^2$（[运动学](#kinematics-differential)）：动量较大的粒子跑得较快，经过 $L$ 后在参考粒子前方 $L\delta/\gamma^2$ 处。各元件的处理：

- **drift、quad**：横向如上，纵向块同漂移段；
- **solenoid**：$x$–$y$ 耦合的 4×4 块（硬边，$C = \cos k_sL$、$S = \sin k_sL$），见 [螺线管](#solenoid)；
- **bend、edge**：扇形二极铁含场指数 $N$，出现 $x$–$\delta$ 与 $z$–$x$ 的关联项（色散），边缘矩阵为薄极面旋转，见 [二极铁](#dipole)；
- **steerer** 与束诊在预览中是单位矩阵（只改变中心轨道，不改变二阶矩）；
- **场图**：参考粒子以 1 mm 步长积分能量与时间；光学按 4 mm 切片，每片是"局部 $B_z$ 中的半步—由近轴场梯度给出的薄透镜冲量（含射频聚束）—再半步"，并计入动量增加带来的阻尼；
- **空间电荷**（可选）：每隔几个步长加一次 rms 等效均匀椭球（束团）或椭圆柱（直流束）的线性冲量。

> AVAS 约定：线性预览只用于快速评估。参考粒子能量误差约 0.01 %，rms 尺寸通常相差 1–2 %，多粒子效应强时可达 13 %；它不包含场图中的二极 / 校正分量、`superposeout` 偏移、色差和非线性效应（如低能腔中纵向发射度的增长）、粒子损失以及 `scanphase`。最终以内核结果为准。

## 动手实验 {#linear-optics-lab}

### 薄透镜与漂移

用相空间实验验证 [Twiss 传输](#linear-optics-twiss-transport) 中的例子：改变 `f` 和 `L`，观察 $\alpha_x$ 的符号变化与束腰位置 $s^* = \alpha_x/\gamma_x$。

```avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
```

### FODO 周期的稳定性

实验给出每周期相移（厚透镜矩阵和薄透镜估计）、稳定判据 $\tfrac12\lvert\Tr M\rvert$、一个周期内的周期 $\beta_x$、$\beta_y$ 以及多个周期上的 rms 包络。建议：

1. 从 $G$ = 20 T/m 开始，确认 $\mu \approx 43°$；逐步增大 $G$，看 $\mu$ 经过 90° 后在约 54 T/m 处失稳，包络发散。
2. 比较厚透镜与薄透镜的相移，体会 $\sqrt{k}L$ 不小时薄透镜近似的误差。
3. 把 `mismatch` 设为 1.3（初始尺寸放大 30 %），观察包络的拍；设回 1 时包络严格周期。

```avas-lab
fodo Lq=0.05 G=20 Ld=0.1 cells=10 W=1.527 mass=938.272 q=1 emit=0.14 I=0 mismatch=1
```

## 延伸阅读 {#linear-optics-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.：横向动力学、FODO 与周期系统。
- S. Y. Lee, *Accelerator Physics*, 4th ed.：Hill 方程、Floquet 变换与 Courant–Snyder 参数。
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1。
- K. L. Brown, "A first- and second-order matrix theory for the design of beam transport systems and charged particle spectrometers", SLAC-75：TRANSPORT 矩阵约定。
- 继续阅读：[纵向运动](#longitudinal)、[四极铁](#quadrupole)、[束线设计](#lattice-design)。
