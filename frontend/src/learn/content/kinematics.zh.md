# 相对论运动学

示例项目的质子动能是 1.527 MeV，只有它静止能量的 0.16 %，用牛顿公式算速度只差 0.12 %——那为什么还要讲相对论？同样 1 MeV 的电子已经以 0.94 倍光速飞行，牛顿公式却会给出 1.98 倍光速。还有一个更实际的谜题：把质子能量提高 10 倍，偏转它的磁铁只需加强约 3.2 倍，静电元件却要加强将近 10 倍。为什么？

本章从能量和动量的定义出发，说明加速器里几个最基本的运动学量——洛伦兹因子 $\gamma$、速度 $\beta$、动量 $p$、磁刚度 $B\rho$、射频周期内走过的距离 $\beta\lambda$——从哪里来、怎样相互联系，以及它们各自决定了加速器的哪一个尺度。读完你会明白：四极铁强度为什么要用 $B\rho$ 归一化，束团为什么在低能漂移段里会拉长，加速腔为什么只适用于某一段速度。一台直线加速器里 $\beta$ 跨越两个数量级的全貌见 [从离子源到靶](#journey-overview)。

## 能量、速度与动量 {#kinematics-variables}

### 从总能量出发

狭义相对论里，质量为 $m$ 的粒子以速度 $v$ 运动时总能量为 $E = \gamma mc^2$，其中 $\gamma = 1/\sqrt{1 - v^2/c^2}$。静止时 $\gamma = 1$，$E = mc^2$ 是静止能量；多出来的部分就是动能 $W = E - mc^2$。所以 $\gamma$ 有一个直观的意义：**总能量是静止能量的多少倍**。由此：

$$
\gamma = 1 + \frac{W}{mc^2}, \qquad
\beta = \frac{v}{c} = \sqrt{1 - \frac{1}{\gamma^2}}, \qquad
\beta\gamma = \sqrt{\gamma^2 - 1}
$$

动量 $p = \gamma m v$，乘以 $c$ 得 $pc = \beta\gamma\,mc^2$。把 $\gamma^2 - \beta^2\gamma^2 = 1$ 两边乘 $(mc^2)^2$，就得到能量–动量关系：

$$
E = \gamma mc^2 = W + mc^2, \qquad
pc = \beta\gamma\, mc^2 = \sqrt{W\left(W + 2mc^2\right)}, \qquad
E^2 = (pc)^2 + (mc^2)^2
$$

以上都是精确关系。示例束流（质子，$W$ = 1.527 MeV）：$\gamma = 1.001627$，$\beta = 0.05698$，$\beta\gamma = 0.05708$，$p$ = 53.55 MeV/$c$。$\gamma$ 只比 1 大 0.16 %，但 $\beta\gamma$ 这个组合会贯穿全书：它把几何发射度和归一化发射度联系起来，也出现在空间电荷力的 $(\beta\gamma)^3$ 里。

加速器中习惯用 MeV 作为能量单位、MeV/$c$ 作为动量单位，于是 $p\,[\mathrm{MeV}/c] = \beta\gamma\, mc^2\,[\mathrm{MeV}]$。常用的静止能量：

| 粒子 | $mc^2$ (MeV) |
|---|---|
| 电子 | 0.51099895 |
| 质子 | 938.272088 |
| 原子质量单位 u | 931.494102 |

> 注意：重离子的静止能量应取离子本身的质量，即原子质量减去剥离电子的质量（结合能通常可忽略）；它**不等于** $A \times 938.272$ MeV。

在实验里拖动动能 $W$，看 $\beta$、$\beta\gamma$、$B\rho$、$E\rho$、$\beta\lambda$ 怎样变化，以及 $\beta(W)$ 曲线怎样从低能时的 $\sqrt W$ 增长弯向 1：

```avas-lab
kinematics mass=938.272 q=1 W=1.527 f=162.5
```

> **想一想**：低能质子的动能翻一倍，速度增加多少？
>
> 答：非相对论时 $\beta \propto \sqrt W$，速度只增加到 $\sqrt2 = 1.414$ 倍。示例质子从 1.527 MeV 到 3.054 MeV，精确值是 1.412 倍。能量的增长换来的速度增长越来越少，这是后面一切“速度饱和”现象的根源。

## 非相对论与极端相对论极限 {#kinematics-nonrel}

记 $u = W/(mc^2)$。两个极限各有一套简单公式，知道它们在哪里失效同样重要。

### 非相对论极限（$u \ll 1$）

$$
W \approx \tfrac12 mv^2, \qquad \beta \approx \sqrt{2u}, \qquad pc \approx \sqrt{2\,mc^2\,W}
$$

精确展开为 $\beta = \sqrt{2u}\,\left(1 - \tfrac34 u + \dots\right)$，所以非相对论公式的相对误差约为 $3u/4$。对 1.527 MeV 质子，$u = 1.63\times10^{-3}$，非相对论 $\beta = 0.057052$，精确值 $0.056983$，误差 0.12 %。这就是开头问题的答案：误差虽小，但它会累积——一个 0.21 m 的腔里，0.12 % 的速度误差使到达时间差约 15 ps，相当于 0.9° 射频相位，已经与射频相位通常关心的 1° 量级相当。重离子和低能质子段可以先用非相对论公式估算，但 AVAS 的线性预览使用精确公式（内核的相对论处理手册未单独说明，按理同样是精确公式）。

### 极端相对论极限（$\gamma \gg 1$）

$$
\beta \approx 1 - \frac{1}{2\gamma^2}, \qquad pc \approx E \approx W
$$

电子在 1 MeV 时 $\gamma = 2.96$、$\beta = 0.941$；用非相对论公式会得到 $\beta \approx 1.98$，完全失效。因此电子束必须一直用相对论公式。把实验的质量改成 0.511，看 $\beta$ 在 1 MeV 以上怎样迅速趋于 1、$\beta\lambda$ 趋于 $\lambda$：

```avas-lab
kinematics mass=0.511 q=1 W=1 f=2856
```

## 小偏差怎样传递：能散、动量散与速度散 {#kinematics-differential}

束流中各粒子的能量略有不同。多推一个粒子一下，让它的动量大一点，它会快多少？对上面的关系求微分：

$$
\dd\gamma = \frac{\dd W}{mc^2}, \qquad
\frac{\dd\beta}{\beta} = \frac{\dd\gamma}{\beta^2\gamma^3} = \frac{1}{\gamma^2}\,\frac{\dd p}{p}, \qquad
\frac{\dd p}{p} = \frac{\dd\gamma}{\beta^2\gamma}
$$

中间一式的物理含义最重要：**速度偏差是动量偏差的 $1/\gamma^2$**。低能时多一分动量就多一分速度；高能时动量还在涨，速度却贴着光速几乎不动。定义相对动量偏差 $\delta = \Delta p / p$（AVAS 纵向坐标之一，见 [相空间](#phase-space-coordinates)），则

$$
\Delta W = \beta^2\gamma\, mc^2\,\delta = v\,\Delta p, \qquad
\frac{\Delta W}{W} = \frac{\gamma + 1}{\gamma}\,\delta
$$

非相对论时 $\Delta W / W \approx 2\delta$，相对论极限 $\Delta W / W \to \delta$。示例束流 $1/\gamma^2 = 0.99675$，$\Delta W/W = 1.998\,\delta$；它的 $\delta_{\mathrm{rms}} = 3.53\times10^{-3}$ 对应 rms 能散 10.78 keV。

速度散最直接的后果是**束团在漂移段里拉长**：动量大 $\delta$ 的粒子每走 $L$ 就领先 $L\delta/\gamma^2$。这正是 [线性光学](#linear-optics) 中漂移段 $z$–$\delta$ 矩阵元为 $L/\gamma^2$ 的原因。

> **想一想**：示例束团（rms 长度 1.316 mm）在没有射频腔的 1 m 漂移段之后有多长？换成 10 MeV 的电子呢？
>
> 答：按线性传输 $z_1 = z_0 + (L/\gamma^2)\,\delta$ 推进 σ 矩阵，1 m 后 rms 长度为 3.71 mm，几乎是原来的三倍——单是能散的贡献就有 $L\,\delta_{\mathrm{rms}}/\gamma^2$ = 3.52 mm。所以低能段每隔不远就要有聚束腔把束团“收回来”（见 [中能传输段](#journey-mebt)）。10 MeV 电子 $1/\gamma^2 = 2.4\times10^{-3}$，同样的 $\delta$ 造成的拉长只有质子的约 1/420，这就是电子在几 MeV 以上几乎没有纵向相位滑移、而低能质子相位滑移强烈的原因（见 [纵向运动](#longitudinal)）。

## 磁刚度与电刚度 {#kinematics-rigidity}

### 磁刚度：把粒子掰弯有多难

粒子在垂直于速度的均匀磁场 $B$ 中做圆周运动，洛伦兹力提供向心力：$qvB = \gamma m v^2/\rho = pv/\rho$。两边消去 $v$：

$$
B\rho = \frac{p}{q}
$$

$B\rho$ 称为**磁刚度**：要让粒子以半径 $\rho$ 转弯所需的磁场乘半径。它只取决于动量与电荷之比——动量越大越“硬”，电荷越多越“软”。速度 $v$ 被消掉了，是因为磁力 $qvB$ 和离心需要的力 $pv/\rho$ 都正比于 $v$。实用单位：

$$
\Brho\,[\mathrm{T\cdot m}] = \frac{p\,[\mathrm{MeV}/c]}{299.792458\,\lvert Z \rvert} = 3.3356\,\frac{p\,[\mathrm{GeV}/c]}{\lvert Z \rvert}
$$

示例质子 $B\rho$ = 0.1786 T·m：1 T 的磁场把它偏转到半径 0.179 m，0.18 T 偏转到 0.992 m。所有磁元件的强度都要用 $B\rho$ 归一化：

- 二极铁偏转半径 $\rho = B\rho / B$（见 [二极铁](#dipole)）；
- 四极铁聚焦强度 $k = G/(B\rho)$，单位 m$^{-2}$（见 [四极铁](#quadrupole)）；
- 螺线管 $k = B/(2B\rho)$，单位 m$^{-1}$（见 [螺线管](#solenoid)）。

所以同一块磁铁对不同能量、不同荷质比的束流强度不同；改变束流能量后，要按 $B\rho$ 等比例缩放磁场才能保持同样的光学。

### 电刚度

在横向电场 $E$ 中，$qE = pv/\rho$——这回 $v$ 消不掉了，因为电力不随速度变化。定义**电刚度**

$$
E\rho = \frac{pv}{q} = \beta c\,(B\rho), \qquad
E\rho\,[\mathrm{MV}] = \frac{\beta\,pc\,[\mathrm{MeV}]}{\lvert Z \rvert}
$$

非相对论时 $pv \approx 2W$，即 $E\rho \approx 2W/Z$（$W$ 以 MeV 计时 $E\rho$ 以 MV 计）。静电元件的偏转半径 $\rho = E\rho/E$，静电四极的强度 $k = (\partial E_x/\partial x)/(E\rho)$。示例质子 $E\rho$ = 3.052 MV，5 MV/m 的横向电场把它偏转到 $\rho$ = 0.610 m。

现在可以回答开头的问题：非相对论时 $B\rho \propto p \propto \sqrt W$，$E\rho \propto 2W$。能量提高 10 倍（1.527 → 15.27 MeV），$B\rho$ 增大到 3.17 倍，$E\rho$ 增大到 9.93 倍。换一种说法：同样的偏转，电场要等于 $v$ 乘磁场。对示例质子，1 T 磁场的作用相当于 17.1 MV/m 的电场；而对 $\beta \to 1$ 的粒子相当于 300 MV/m——远超实际能维持的静电场。所以静电元件只在低速时有效（$E\rho \propto \beta^2$），高能时磁元件要经济得多；低能的 RFQ 正是利用了电场聚焦不随速度减弱这一点（见 [RFQ](#journey-rfq)）。静电场元件见 [静电与静磁场元件](#static-fields)。

在实验里把 $W$ 设为 15.27 MeV，对比 $B\rho$ 和 $E\rho$ 的变化：

```avas-lab
kinematics mass=938.272 q=1 W=15.27 f=162.5
```

> 注意：刚度的符号由电荷符号决定。上面的实用公式取了 $\lvert Z \rvert$；对负粒子，同样的磁场产生相反方向的力，四极铁的聚焦平面也随之互换。

## 射频波长与 βλ {#kinematics-rf}

射频频率 $f$ 对应自由空间波长和周期

$$
\lambda = \frac{c}{f}, \qquad T_{\mathrm{RF}} = \frac{1}{f}
$$

射频场每半个周期反一次号。粒子在一个射频周期内走过的距离是 $\beta\lambda$——它是直线加速器纵向尺度的基本单位。想象一排加速间隙：粒子在第一个间隙里赶上了加速的半周期，要在下一个间隙里还被加速，间隙之间的距离就必须和粒子的速度“对拍子”：

- π 模结构相邻间隙场的方向相反，相距 $\beta\lambda/2$；0 模（如 DTL 类型）相邻间隙同向，相距 $\beta\lambda$；
- 纵向位置与相位的换算：在参考粒子前方 $\Delta z$ 处的粒子较早到达加速间隙，它看到的射频相位较小，$\Delta\varphi = -2\pi\,\Delta z/(\beta\lambda)$；
- 渡越时间因子随 $\beta$ 变化，一个腔只在一定的 $\beta$ 范围内有效加速（见 [射频腔](#rf-cavity)）。

示例：162.5 MHz 时 $\lambda = 1.8449$ m，$T_{\mathrm{RF}}$ = 6.154 ns。1.527 MeV 质子 $\beta\lambda$ = 0.10513 m，即 1° 射频相位对应 0.292 mm。0.21 m 长的 HWR 场图，参考粒子按初速度渡越约需 $0.21/(\beta c)$ = 12.29 ns，约 2.0 个射频周期。

> **想一想**：同样 162.5 MHz，为什么不能用一种腔把质子从离子源的 35 keV 一直加速到 1 GeV？
>
> 答：$\beta\lambda$ 从 35 keV 的 15.9 mm 增大到 1 GeV 的 1.614 m，变了约 100 倍（在实验里把 $W$ 拖到两端就能看到）。间隙间距固定的腔，只对一个设计速度“对拍子”；速度偏离后，粒子到达后续间隙时相位不对，有效加速随渡越时间因子下降。所以直线加速器要分段，每一段用为某一 $\beta$ 设计的腔，见 [从离子源到靶](#journey-overview)。

## 数值表：从质子到电子 {#kinematics-examples}

下面三张表都按本章的精确公式计算，用来建立数量感。读表时可以注意三件事：质子从 1.527 MeV 到 1 GeV，$\beta$ 增大 15 倍而 $B\rho$ 增大 32 倍；电子 1 MeV 以上 $\beta\lambda$ 已几乎等于 $\lambda$；电刚度 $E\rho = \beta pc$ 在低能时与 $2W$ 相近，高能时趋于 $pc$。

### 质子（$mc^2$ = 938.272 MeV，$\beta\lambda$ 按 162.5 MHz）

| $W$ (MeV) | $\gamma$ | $\beta$ | $\beta\gamma$ | $p$ (MeV/$c$) | $B\rho$ (T·m) | $E\rho$ (MV) | $\beta\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 1.527 | 1.001627 | 0.056983 | 0.057076 | 53.55 | 0.17863 | 3.052 | 0.10513 |
| 10 | 1.010658 | 0.144844 | 0.146388 | 137.35 | 0.45816 | 19.89 | 0.26722 |
| 100 | 1.106579 | 0.428195 | 0.473832 | 444.58 | 1.48297 | 190.4 | 0.78997 |
| 1000 | 2.065789 | 0.875026 | 1.807618 | 1696.04 | 5.65737 | 1484.1 | 1.61431 |

第一行就是示例项目的束流。

### 电子（$mc^2$ = 0.511 MeV，$\beta\lambda$ 按 2856 MHz，$\lambda$ = 0.10497 m）

| $W$ (MeV) | $\gamma$ | $\beta$ | $\beta\gamma$ | $p$ (MeV/$c$) | $B\rho$ (T·m) | $E\rho$ (MV) | $\beta\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 0.1 | 1.195695 | 0.548221 | 0.655505 | 0.3350 | 0.001117 | 0.1836 | 0.05755 |
| 1 | 2.956951 | 0.941079 | 2.782725 | 1.4220 | 0.004743 | 1.338 | 0.09878 |
| 10 | 20.56951 | 0.998818 | 20.54519 | 10.499 | 0.03502 | 10.49 | 0.10485 |
| 100 | 196.6951 | 0.999987 | 196.6926 | 100.51 | 0.33526 | 100.51 | 0.10497 |

### 重离子（按每核子能量给出时）

以 $^{40}$Ar$^{8+}$、每核子 0.5 MeV 为例：离子静止能量约 $39.9624 \times 931.494 - 8\times0.511$ = 37220.6 MeV，总动能 $W = 40\times0.5$ = 20 MeV，于是 $\beta$ = 0.03277、$p$ = 1220.3 MeV/$c$、$B\rho = p/(299.79\times8)$ = 0.5088 T·m。与 1.527 MeV 质子相比，它的速度只有约 58 %，磁刚度却是 2.85 倍：荷质比 $Z/A$ 越小，同样速度下的刚度越大，聚焦越困难。

## 容易误解的地方 {#kinematics-misconceptions}

- **“动能、总能量、每核子能量是一回事。”** $W$ 是动能，$E = W + mc^2$ 是总能量；重离子常按每核子动能报能量，换算到整个离子要乘核子数。AVAS 的 `kneticenergy` 是整个粒子的动能。
- **“动量的单位 MeV/$c$ 和能量的 MeV 可以互换。”** 只有极端相对论时 $pc \approx W$；示例质子 $pc$ = 53.55 MeV，而动能只有 1.527 MeV。
- **“重离子质量就是 $A$ 乘质子质量。”** 要用原子质量单位 u = 931.494 MeV 计，再减去剥离电子；$^{40}$Ar 按 $40\times938.272$ 算会大 0.8 %。
- **“非相对论公式在 MeV 量级处处可用。”** 对质子误差约 $3u/4$，可以用于估算；对电子在 100 keV 就已失效。
- **“$\beta\lambda$ 是一个固定长度。”** 它随能量变化；只有 $\beta \to 1$ 时才趋于 $\lambda$。
- **“刚度没有符号。”** 实用公式里取了 $\lvert Z \rvert$，但力的方向由电荷符号决定；负粒子的四极铁聚焦平面互换。
- **“同一块四极铁在任何能量下聚焦能力相同。”** $k = G/(B\rho)$，能量变了 $k$ 就变了。

## 在 AVAS 中 {#kinematics-in-avas}

### 关键字

`beam.txt` 中与运动学有关的关键字：

```avas-params
beam:particlerestmass beam:numofcharge beam:kneticenergy beam:frequency beam:current
```

- `particlerestmass`：粒子静止能量 $mc^2$，单位 MeV（示例项目为 938.272088159928，即质子）。
- `numofcharge`：电荷数 $Z$，单位 $e$。它同时进入刚度 $B\rho = p/(Ze)$ 和空间电荷（每个粒子的电荷）。
- `kneticenergy`：参考粒子的动能 $W$，单位 MeV（拼写就是 `kneticenergy`，手册的表格未收录，由束流页写入）。
- `frequency`：束流频率，单位 **Hz**（示例为 `162500000.0`），用来把纵向位置换算成相位并确定束团电荷 $Q = I/f$。
- `current`：流强，单位 mA。

> **AVAS 约定**：界面的线性预览按 $\gamma = 1 + W/m$ 计算，`kneticenergy` 和 `particlerestmass` 都是**整个粒子**的量（不是每核子的量）。重离子请写离子的总质量和总动能，例如上面的 $^{40}$Ar$^{8+}$ 写 `particlerestmass 37220.6`、`kneticenergy 20`、`numofcharge 8`。

> 注意：`beam.txt` 的 `frequency` 和 lattice 中 `field` 元件的频率参数 `f` 都以 Hz 为单位；本章动手实验里的 `f` 以 MHz 为单位，仅为方便拖动。负电荷粒子（如电子）按定义应写负的电荷数；手册没有专门说明内核对负电荷的处理，使用前请用简单的 lattice 核对四极铁的聚焦方向。

### 束团电荷与宏粒子

示例项目的束团电荷 $Q = 0.1841\,\mathrm{mA} / 162.5\,\mathrm{MHz}$ = 1.133 pC，约 $7.07\times10^6$ 个质子；用 5260 个宏粒子时，每个宏粒子代表约 1344 个质子。宏粒子的意义见 [初始分布](#distributions)。

### 输出中的运动学量

`DataSet.txt` 第 0 列是参考粒子动能 $W$（MeV），可以用本章公式换算出每个位置的 $\beta\gamma$，进而由归一化发射度求几何发射度（见 [相空间](#phase-space-emittance)）。

### 实验参数

实验 `kinematics` 的参数：`mass`（MeV）、`q`（电荷数）、`W`（MeV）、`f`（MHz）。建议的检查：从示例质子束开始确认上表数值；把 $W$ 增大到 10 倍（15.27 MeV），$B\rho$ 增大约 3.17 倍、$E\rho$ 增大约 9.93 倍；保持速度不变、改变电荷数，刚度与 $1/Z$ 成正比。

## 延伸阅读 {#kinematics-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：相对论关系、$\beta\lambda$ 与加速结构的尺度。
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific：磁刚度与实用单位。
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015。
- 继续阅读：[相空间与发射度](#phase-space)。
