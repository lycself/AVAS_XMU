# 相对论运动学

直线加速器里所有的"强度"——四极铁的聚焦强度、二极铁的偏转半径、射频腔的相位滑移——都通过几个运动学量与束流能量联系在一起：洛伦兹因子 $\gamma$、速度 $\beta$、动量 $p$、磁刚度 $B\rho$ 和 $\beta\lambda$。本章给出这些量的定义、相互关系和数值示例，并说明它们与 `beam.txt` 关键字的对应。

## 基本量 {#kinematics-variables}

设粒子静止能量为 $mc^2$，电荷为 $q = Ze$（$Z$ 为电荷数，可为负），动能为 $W$。以下关系是精确的狭义相对论关系：

$$
\gamma = 1 + \frac{W}{mc^2}, \qquad
\beta = \frac{v}{c} = \sqrt{1 - \frac{1}{\gamma^2}}, \qquad
\beta\gamma = \sqrt{\gamma^2 - 1}
$$

总能量与动量为

$$
E = \gamma mc^2 = W + mc^2, \qquad
pc = \beta\gamma\, mc^2 = \sqrt{W\left(W + 2mc^2\right)}, \qquad
E^2 = (pc)^2 + (mc^2)^2
$$

在加速器中习惯用 MeV 作为能量单位、MeV/$c$ 作为动量单位，于是 $p\,[\mathrm{MeV}/c] = \beta\gamma\, mc^2\,[\mathrm{MeV}]$。常用的静止能量：

| 粒子 | $mc^2$ (MeV) |
|---|---|
| 电子 | 0.51099895 |
| 质子 | 938.272088 |
| 原子质量单位 u | 931.494102 |

> 注意：重离子的静止能量应取离子本身的质量，即原子质量减去剥离电子的质量（结合能通常可忽略）；它**不等于** $A \times 938.272$ MeV。

## 微分关系与能散 {#kinematics-differential}

束流中各粒子的能量略有不同，需要把能量偏差换算成速度、动量偏差。对上面的关系求微分：

$$
\dd\gamma = \frac{\dd W}{mc^2}, \qquad
\frac{\dd\beta}{\beta} = \frac{\dd\gamma}{\beta^2\gamma^3} = \frac{1}{\gamma^2}\,\frac{\dd p}{p}, \qquad
\frac{\dd p}{p} = \frac{\dd\gamma}{\beta^2\gamma}
$$

定义相对动量偏差 $\delta = \Delta p / p$（AVAS 纵向坐标之一，见 [相空间](#phase-space-coordinates)），则

$$
\Delta W = \beta^2\gamma\, mc^2\,\delta = v\,\Delta p, \qquad
\frac{\Delta W}{W} = \frac{\gamma + 1}{\gamma}\,\delta
$$

几点推论：

- 非相对论时 $\Delta W / W \approx 2\delta$，相对论极限 $\Delta W / W \to \delta$。
- 速度偏差 $\Delta\beta/\beta = \delta/\gamma^2$：低能时速度对动量偏差很敏感，高能时速度几乎不变。这正是 [线性光学](#linear-optics-6d) 中漂移段 $z$–$\delta$ 矩阵元为 $L/\gamma^2$ 的原因，也是电子在几 MeV 以上几乎没有纵向相位滑移、而低能质子有强烈相位滑移的原因（见 [纵向运动](#longitudinal)）。
- 以示例束流（质子，$W$ = 1.527 MeV）为例：$1/\gamma^2 = 0.9968$，$\Delta W/W = 1.998\,\delta$。

## 非相对论与极端相对论极限 {#kinematics-nonrel}

记 $u = W/(mc^2)$。

### 非相对论极限（$u \ll 1$）

$$
W \approx \tfrac12 mv^2, \qquad \beta \approx \sqrt{2u}, \qquad pc \approx \sqrt{2\,mc^2\,W}
$$

精确展开为 $\beta = \sqrt{2u}\,\left(1 - \tfrac34 u + \dots\right)$，所以非相对论公式的相对误差约为 $3u/4$。对 1.527 MeV 质子，$u = 1.63\times10^{-3}$，非相对论 $\beta = 0.057052$，精确值 $0.056983$，误差 0.12 %。重离子和低能质子段通常可以先用非相对论公式估算，但 AVAS 内核和线性预览都使用精确公式。

### 极端相对论极限（$\gamma \gg 1$）

$$
\beta \approx 1 - \frac{1}{2\gamma^2}, \qquad pc \approx E \approx W
$$

电子在 1 MeV 时 $\gamma = 2.96$、$\beta = 0.941$；用非相对论公式会得到 $\beta \approx 1.98$，完全失效。因此电子束必须一直用相对论公式。

## 磁刚度与电刚度 {#kinematics-rigidity}

### 磁刚度

粒子在垂直于速度的均匀磁场 $B$ 中做圆周运动，洛伦兹力提供向心力：$qvB = \gamma m v^2/\rho = pv/\rho$，于是

$$
B\rho = \frac{p}{q}
$$

$B\rho$ 称为**磁刚度**，只取决于粒子的动量与电荷之比。实用单位：

$$
\Brho\,[\mathrm{T\cdot m}] = \frac{p\,[\mathrm{MeV}/c]}{299.792458\,\lvert Z \rvert} = 3.3356\,\frac{p\,[\mathrm{GeV}/c]}{\lvert Z \rvert}
$$

所有磁元件的强度都要用 $B\rho$ 归一化：

- 二极铁偏转半径 $\rho = B\rho / B$（见 [二极铁](#dipole)）；
- 四极铁聚焦强度 $k = G/(B\rho)$，单位 m$^{-2}$（见 [四极铁](#quadrupole)）；
- 螺线管 $k = B/(2B\rho)$，单位 m$^{-1}$（见 [螺线管](#solenoid)）。

所以同一块磁铁对不同能量、不同荷质比的束流强度不同；改变束流能量后，要按 $B\rho$ 等比例缩放磁场才能保持同样的光学。

### 电刚度

在横向电场 $E$ 中，$qE = pv/\rho$，定义**电刚度**

$$
E\rho = \frac{pv}{q} = \beta c\,(B\rho), \qquad
E\rho\,[\mathrm{MV}] = \frac{\beta\,pc\,[\mathrm{MeV}]}{\lvert Z \rvert}
$$

非相对论时 $pv \approx 2W$，即 $E\rho \approx 2W/Z$（$W$ 以 MeV 计时 $E\rho$ 以 MV 计）。静电元件的偏转半径 $\rho = E\rho/E$，静电四极的强度 $k = (\partial E_x/\partial x)/(E\rho)$。可见静电元件对低速粒子有效（$E\rho \propto \beta^2$），而高能时磁元件远比电元件经济。静电场元件见 [静电与静磁场元件](#static-fields)。

> 注意：刚度的符号由电荷符号决定。上面的实用公式取了 $\lvert Z \rvert$；对负粒子，同样的磁场产生相反方向的力，四极铁的聚焦平面也随之互换。

## 射频波长与 βλ {#kinematics-rf}

射频频率 $f$ 对应自由空间波长和周期

$$
\lambda = \frac{c}{f}, \qquad T_{\mathrm{RF}} = \frac{1}{f}
$$

粒子在一个射频周期内走过的距离是 $\beta\lambda$。它是直线加速器纵向尺度的基本单位：

- 加速间隙的间距：π 模结构相邻间隙相距 $\beta\lambda/2$，0 模（如 DTL 类型）相距 $\beta\lambda$；
- 纵向位置与相位的换算：在参考粒子前方 $\Delta z$ 处的粒子较早到达加速间隙，它看到的射频相位较小，$\Delta\varphi = -2\pi\,\Delta z/(\beta\lambda)$；
- 渡越时间因子随 $\beta$ 变化，一个腔只在一定的 $\beta$ 范围内有效加速（见 [射频腔](#rf-cavity)）。

示例：162.5 MHz 时 $\lambda = 1.8449$ m，$T_{\mathrm{RF}}$ = 6.154 ns。1.527 MeV 质子 $\beta\lambda$ = 0.10513 m，即 1° 射频相位对应 0.292 mm。0.21 m 长的 HWR 场图，参考粒子按初速度渡越约需 $0.21/(\beta c)$ = 12.29 ns，约 2.0 个射频周期。

## 数值示例 {#kinematics-examples}

### 质子（$mc^2$ = 938.272 MeV，$\beta\lambda$ 按 162.5 MHz）

| $W$ (MeV) | $\gamma$ | $\beta$ | $\beta\gamma$ | $p$ (MeV/$c$) | $B\rho$ (T·m) | $E\rho$ (MV) | $\beta\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 1.527 | 1.001627 | 0.056983 | 0.057076 | 53.55 | 0.17863 | 3.052 | 0.10513 |
| 10 | 1.010658 | 0.144844 | 0.146388 | 137.35 | 0.45816 | 19.89 | 0.26722 |
| 100 | 1.106579 | 0.428195 | 0.473832 | 444.58 | 1.48297 | 190.4 | 0.78997 |
| 1000 | 2.065789 | 0.875026 | 1.807618 | 1696.04 | 5.65737 | 1484.1 | 1.61431 |

第一行就是示例项目的束流：0.18 T 的二极场把它偏转到 $\rho$ = 0.992 m；5 MV/m 的横向电场对应 $\rho$ = 0.610 m。

### 电子（$mc^2$ = 0.511 MeV，$\beta\lambda$ 按 2856 MHz，$\lambda$ = 0.10497 m）

| $W$ (MeV) | $\gamma$ | $\beta$ | $\beta\gamma$ | $p$ (MeV/$c$) | $B\rho$ (T·m) | $E\rho$ (MV) | $\beta\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 0.1 | 1.195695 | 0.548221 | 0.655505 | 0.3350 | 0.001117 | 0.1836 | 0.05755 |
| 1 | 2.956951 | 0.941079 | 2.782725 | 1.4220 | 0.004743 | 1.338 | 0.09878 |
| 10 | 20.56951 | 0.998818 | 20.54519 | 10.499 | 0.03502 | 10.49 | 0.10485 |
| 100 | 196.6951 | 0.999987 | 196.6926 | 100.51 | 0.33526 | 100.51 | 0.10497 |

### 重离子（按每核子能量给出时）

以 $^{40}$Ar$^{8+}$、每核子 0.5 MeV 为例：离子静止能量约 $39.9624 \times 931.494 - 8\times0.511$ = 37220.6 MeV，总动能 $W = 40\times0.5$ = 20 MeV，于是 $\beta$ = 0.03277、$p$ = 1220.3 MeV/$c$、$B\rho = p/(299.79\times8)$ = 0.5088 T·m。荷质比 $Z/A$ 越小，同样速度下的刚度越大，聚焦越困难。

## 在 AVAS 中 {#kinematics-in-avas}

`beam.txt` 中与运动学有关的关键字：

```avas-params
beam:particlerestmass beam:numofcharge beam:kneticenergy beam:frequency beam:current
```

- `particlerestmass`：粒子静止能量 $mc^2$，单位 MeV（示例项目为 938.272088159928，即质子）。
- `numofcharge`：电荷数 $Z$，单位 $e$。它同时进入刚度 $B\rho = p/(Ze)$ 和空间电荷（每个粒子的电荷）。
- `kneticenergy`：参考粒子的动能 $W$，单位 MeV（拼写就是 `kneticenergy`，手册的表格未收录，由束流页写入）。
- `frequency`：束流频率，单位 **Hz**（示例为 `162500000.0`），用来把纵向位置换算成相位并确定束团电荷 $Q = I/f$。
- `current`：流强，单位 mA。

> AVAS 约定：界面的线性预览按 $\gamma = 1 + W/m$ 计算，`kneticenergy` 和 `particlerestmass` 都是**整个粒子**的量（不是每核子的量）。重离子请写离子的总质量和总动能，例如上面的 $^{40}$Ar$^{8+}$ 写 `particlerestmass 37220.6`、`kneticenergy 20`、`numofcharge 8`。

> 注意：`beam.txt` 的 `frequency` 和 lattice 中 `field` 元件的频率参数 `f` 都以 Hz 为单位；本章动手实验里的 `f` 以 MHz 为单位，仅为方便拖动。负电荷粒子（如电子）按定义应写负的电荷数；手册没有专门说明内核对负电荷的处理，使用前请用简单的 lattice 核对四极铁的聚焦方向。

示例项目的束团电荷 $Q = 0.1841\,\mathrm{mA} / 162.5\,\mathrm{MHz}$ = 1.133 pC，约 $7.07\times10^6$ 个质子；用 5260 个宏粒子时，每个宏粒子代表约 1340 个质子。

运动学量在输出中的体现：`DataSet.txt` 第 0 列是参考粒子动能 $W$（MeV），可以用本章公式换算出每个位置的 $\beta\gamma$，进而由归一化发射度求几何发射度（见 [相空间](#phase-space-emittance)）。

## 动手实验 {#kinematics-lab}

拖动动能 $W$，观察 $\beta$、$\beta\gamma$、$B\rho$、$E\rho$、$\beta\lambda$ 的变化以及 $\beta(W)$ 曲线。建议：

1. 从示例质子束开始，确认表中的数值；把 $W$ 增大到 10 倍（15.27 MeV），$B\rho$ 只增大到约 3.17 倍（接近 $\sqrt{10}$），而 $E\rho$ 增大到约 9.93 倍（接近 10）。
2. 把质量改成 0.511（电子），观察 $\beta$ 在 1 MeV 以上迅速趋于 1、$\beta\lambda$ 趋于 $\lambda$。
3. 保持速度不变、改变电荷数，观察刚度与 $1/Z$ 成正比。

```avas-lab
kinematics mass=938.272 q=1 W=1.527 f=162.5
```

电子示例（2856 MHz）：

```avas-lab
kinematics mass=0.511 q=1 W=1 f=2856
```

## 延伸阅读 {#kinematics-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.：相对论关系、$\beta\lambda$ 与加速结构的尺度。
- S. Y. Lee, *Accelerator Physics*, 4th ed.：磁刚度与实用单位。
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015。
- 继续阅读：[相空间与发射度](#phase-space)。
