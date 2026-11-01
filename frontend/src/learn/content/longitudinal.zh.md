# 纵向动力学

射频腔给粒子能量，同时决定束团在纵向（时间 / 相位）上能否保持聚集。本章从单个加速间隙的能量增益出发，引入渡越时间因子和多间隙腔，然后讨论同步粒子、相稳定原理、小振幅同步振荡、纵向接受度、绝热阻尼以及与纵向聚焦必然相伴的射频散焦。最后说明 AVAS 中射频相位的三种定义（`field` 的 V3 参数）和相关关键字。运动学量（$\beta$、$\gamma$、$\beta\lambda$）的定义见 [相对论运动学](#kinematics)，相空间与发射度见 [相空间与发射度](#phase-space)，射频腔元件本身的写法见 [射频腔](#rf-cavity)。

本章的数值例子都用示例项目 `examples/hwr010`：质子，入口动能 $W = 1.527\ \mathrm{MeV}$（$\beta = 0.05698$，$\gamma = 1.00163$），射频频率 $f = 162.5\ \mathrm{MHz}$（$\lambda = c/f = 1.8449\ \mathrm{m}$，$\beta\lambda = 0.1051\ \mathrm{m}$），半波长谐振腔（HWR）场图 `hwr010`，长 0.21 m，`Ke = 1.36`。

## 射频间隙中的能量增益 {#longitudinal-gap}

### 场与相位

设腔内轴上纵向电场为驻波

$$
E_z(z, t) = E(z)\cos\left(\omega t + \varphi_0\right),
$$

$E(z)$ 是场的空间分布，$\omega = 2\pi f$。粒子沿轴运动，到达 $z$ 的时刻为 $t(z)$，它在 $z$ 处"看到"的相位是

$$
\varphi(z) = \omega t(z) + \varphi_0 .
$$

在这个约定下，**相位越大表示粒子越晚到达**。粒子穿过间隙得到的能量增益是

$$
\Delta W = q\int E(z)\cos\varphi(z)\,\dd z .
\tag{1}
$$

> AVAS 约定：射频电场按 $K_e\,E(\boldsymbol r)\cos(\omega t + \varphi_0)$ 作用，射频磁场按 $K_b\,B(\boldsymbol r)\sin(\omega t + \varphi_0)$ 作用（场图中 $E$ 单位 MV/m，$B$ 单位 T）。这与 Wangler《RF Linear Accelerators》的余弦约定一致。纵向坐标 $z$ 与相位的关系为 $\Delta\varphi = -2\pi z/(\beta\lambda)$，即 $z > 0$ 的粒子在参考粒子前方、相位较小。

### Panofsky 公式与渡越时间因子

假设粒子在间隙内速度变化很小（$\Delta W \ll W$），可以把 $t(z)$ 近似为匀速：$\omega t(z) = \omega t_c + k (z - z_c)$，$k = 2\pi/(\beta\lambda)$，$z_c$ 是间隙的某个参考点。代入 (1)：

$$
\Delta W = q\cos\varphi\int E(z)\cos k(z - z_c)\,\dd z - q\sin\varphi\int E(z)\sin k(z - z_c)\,\dd z ,
$$

其中 $\varphi = \omega t_c + \varphi_0$ 是粒子到达 $z_c$ 时的相位。把 $z_c$ 选在**电中心**，使第二个积分为零（对称间隙即几何中心），就得到 Panofsky 公式

$$
\Delta W = q E_0 T L\cos\varphi = q V_0 T\cos\varphi ,
\qquad
T = \frac{\int E(z)\cos k(z - z_c)\,\dd z}{\int E(z)\,\dd z},
\tag{2}
$$

$E_0 = V_0/L$ 是间隙长度 $L$ 上的平均轴上场，$V_0 = \int E\,\dd z$ 是轴上电压。**渡越时间因子** $T \le 1$ 描述粒子在场变化的时间内穿过间隙造成的损失。对长 $g$ 的均匀场间隙，

$$
T = \frac{\sin(\pi g/\beta\lambda)}{\pi g/\beta\lambda},
$$

间隙越短于 $\beta\lambda$，$T$ 越接近 1。实际腔的场在间隙外有延伸，$T$ 要用场分布积分。

### 准确定义：沿实际轨迹的相位

当能量增益与入口能量可比时（低能质子、重离子），匀速近似不再成立。此时把 (1) 写成

$$
C = \int E(z)\cos\varphi(z)\,\dd z,\quad S = \int E(z)\sin\varphi(z)\,\dd z,\quad
\Delta W = q\sqrt{C^2 + S^2}\,\cos\varphi_s,\quad \varphi_s = \operatorname{atan2}(S, C),
\tag{3}
$$

积分沿参考粒子的**实际**轨迹 $t(z)$ 进行。式 (3) 是精确的，$\varphi_s$ 就是 AVAS 所说的同步相位（见 [下文](#longitudinal-synchronous)），但"有效电压" $\sqrt{C^2 + S^2}$ 本身依赖于轨迹，因而依赖于 $\varphi_s$，能量增益不再严格按 $\cos\varphi_s$ 变化。

## 渡越时间因子与多间隙腔 {#longitudinal-multigap}

### 两间隙腔

半波长谐振腔（HWR）、四分之一波长腔（QWR）的加速部分是两个间隙，场方向相反（$\pi$ 模）。把单个间隙的场记为 $e(z)$，两个间隙中心相距 $d$：$E(z) = e(z - d/2) - e(z + d/2)$。其傅里叶变换给出

$$
T(\beta) = T_g(\beta)\,\left\lvert\sin\frac{k d}{2}\right\rvert = T_g(\beta)\,\left\lvert\sin\frac{\pi\beta_g}{2\beta}\right\rvert,
\qquad d = \frac{\beta_g\lambda}{2},
$$

$T_g$ 是单个间隙的渡越时间因子。对正负交替的多间隙场 $\int E\,\dd z \approx 0$，此时式 (2) 的分母改用 $V_0 = \int\lvert E\rvert\,\dd z$（下表即如此）。粒子从一个间隙飞到另一个间隙恰好用半个射频周期时（$\beta = \beta_g$），两个间隙同相加速。$N$ 个间隙的腔 $T(\beta)$ 更尖锐，能量接受范围更窄；低 $\beta$ 超导直线加速器因此用间隙少的腔，并分成几种几何 $\beta$。

### 示例腔 hwr010

对 `hwr010` 场图（乘以 `Ke = 1.36`）的轴上 $E_z$ 积分得到：

| 量 | 数值 |
|---|---|
| 轴上场峰值 | $+13.03\ \mathrm{MV/m}$ 与 $-12.74\ \mathrm{MV/m}$ |
| 场反号位置 | $z \approx 0.104\ \mathrm{m}$（两个间隙） |
| 两间隙场的重心 | $0.064\ \mathrm{m}$、$0.145\ \mathrm{m}$，间距 $d = 0.081\ \mathrm{m}$ |
| 由间距得到的 $\beta_g = 2d/\lambda$ | 0.088 |
| $V_0 = \int\lvert E_z\rvert\,\dd z$ | 0.978 MV |
| $T(\beta)$ 的最大值 | 0.82，在 $\beta \approx 0.104$ |
| 入口 $\beta = 0.0570$ 处的 $T$ | 0.40，$V_0 T = 0.39\ \mathrm{MV}$ |

$T(\beta)$ 最大值处的 $\beta \approx 0.10$ 与名字中的 "010"（通常表示设计 $\beta = 0.10$）相符；它比由间距估计的 0.088 稍大，是因为单间隙的 $T_g(\beta)$ 随 $\beta$ 增大。示例束流的 $\beta = 0.057$ 远低于最佳 $\beta$，所以 $T$ 只有 0.40；粒子在腔内被加速后 $T$ 逐渐增大，这也是低能腔里匀速近似失效的原因之一。

```avas-field
map hwr010
```

### 能量增益与同步相位：示例

用 AVAS 的线性预览（`avas.sim.linear_optics`，lattice 为 `field 0.21 0.02 0 1 162.5e6 φs 1.36 -1.36 hwr010`）计算出口能量：

| $\varphi_s$（V3 = 0） | 入口相位 | 出口能量 / MeV | $\Delta W$ / MeV |
|---|---|---|---|
| −90° | −14.0° | 1.5270 | 0.000 |
| −60° | 18.6° | 1.6943 | 0.167 |
| −33° | 52.7° | 1.8665 | 0.340 |
| −20° | 69.7° | 1.9374 | 0.410 |
| 0° | 95.4° | 2.0023 | 0.475 |
| +20° | 119.7° | 1.9954 | 0.468 |

$\varphi_s = -90°$ 时能量增益为零，符合 (3)。但 $\Delta W/\cos\varphi_s$ 在 −33° 时为 0.405 MeV，在 0° 时为 0.475 MeV，相差约 15 %，而且用入口相位扫描时最大能量增益（2.0087 MeV）出现在 $\varphi_s \approx +8°$ 而不是 0°。原因就是本节开头说的：$\Delta W/W \approx 30\ \%$，$\beta$ 在腔内从 0.057 增大到约 0.065，有效电压随相位改变。对这种腔，Panofsky 公式只能作估算，设计要以沿轨迹积分的结果为准。

## 同步粒子与同步相位 {#longitudinal-synchronous}

**同步粒子**（参考粒子）是设计上沿轴运动、在每个腔都处在设计相位的粒子；其他粒子的纵向坐标都是相对它定义的。在一系列腔组成的直线加速器中，同步粒子到达第 $n$ 个腔时的相位 $\varphi_{s,n}$ 由腔的射频相位和它在前面各段的飞行时间共同决定。

### AVAS 中相位的三种含义（V3）

`field L R V3 type f phase Ke Kb file` 的第 3 个参数 V3 决定第 6 个参数 `phase` 的含义：

| V3 | `phase` 的含义 | 内核如何确定腔的入口相位 $\varphi_0$ |
|---|---|---|
| 0 | 同步相位 $\varphi_s$ | 求解 $\varphi_0$，使沿参考粒子轨迹的 $\operatorname{atan2}(S, C) = \varphi_s$，见式 (3) |
| 1 | 参考粒子到达元件入口时的射频相位 | $\varphi_0$ = `phase` |
| 2 | $t = 0$ 时刻的射频相位（绝对相位） | $\varphi_0 = $ `phase` $+ 360°\cdot f\cdot t_{\mathrm{in}}$，$t_{\mathrm{in}}$ 是参考粒子到达入口的时刻 |

V3 = 0 最方便设计：物理上有意义的是同步相位，入口相位由程序反求。V3 = 1 适合做相位扫描（入口相位与腔的几何直接对应）。V3 = 2 对应真实机器上"每个腔的射频相对公共参考时钟的相位"，上游元件改变时同步相位会跟着变。

例：示例腔前加一段 0.1 m 漂移，参考粒子到达腔入口的时刻 $t_{\mathrm{in}} = 0.1/(\beta c) = 5.854\ \mathrm{ns}$，$360°\,f\,t_{\mathrm{in}} = 342.45°$。V3 = 2、`phase = 0` 时入口相位为 $342.45° \equiv -17.55°$，对应同步相位约 −93.7°——这个腔几乎不加速。而 V3 = 0、`phase = -33` 时，无论前面漂移多长，入口相位都是 52.7°，同步相位保持 −33°。

> AVAS 约定：示例中同步相位 $-33°$ 对应的入口相位是 $52.7°$，两者之差（约 86°）取决于场图的零点位置和场分布，没有通用换算；对 V3 = 0 的腔，线性预览会给出求得的入口相位与同步相位。分段运行把一段 lattice 单独拿出来时，AVAS（`avas/data/segment.py` 的 `rephase`）从参考运行的同步粒子数据取每个腔的射频相位 $\varphi_{\mathrm{RF}}$，按 $\varphi_{\mathrm{RF}} - 360°\cdot f\cdot(t_{\mathrm{in}} - T_{\mathrm{entry}})$ 换算成以分段入口时刻为零点的 V3 = 2 相位，而不是沿用原来的相位或改成 V3 = 0。

## 相稳定原理 {#longitudinal-stability}

### 运动方程

考虑一列腔组成的周期结构，把每个腔的作用平均到单位长度上：平均加速梯度 $E_0 T$（腔内 $E_0 T$ 乘以腔长占周期长度的比例）。定义相对同步粒子的偏离

$$
\Delta\varphi = \varphi - \varphi_s,\qquad w = \frac{W - W_s}{mc^2} .
$$

由 $\dd t/\dd s = 1/(\beta c)$ 和 $\Delta\beta = \Delta\gamma/(\beta\gamma^3)$：

$$
\frac{\dd\,\Delta\varphi}{\dd s} = \omega\left(\frac{1}{\beta c} - \frac{1}{\beta_s c}\right) \approx -\frac{2\pi}{\beta_s^3\gamma_s^3\lambda}\,w \equiv -A\,w ,
\tag{4}
$$

能量方程

$$
\frac{\dd w}{\dd s} = \frac{q E_0 T}{mc^2}\left(\cos\varphi - \cos\varphi_s\right) \equiv B\left(\cos\varphi - \cos\varphi_s\right).
\tag{5}
$$

式 (4) 说明能量高的粒子（$w > 0$）跑得快，相位减小（提前到达）。

### 稳定条件

对 (4)、(5) 线性化，$\cos\varphi - \cos\varphi_s \approx -\sin\varphi_s\,\Delta\varphi$：

$$
\frac{\dd^2\Delta\varphi}{\dd s^2} = A B \sin\varphi_s\,\Delta\varphi .
$$

$A, B > 0$，所以**只有 $\sin\varphi_s < 0$ 时运动稳定**。结合 $\cos\varphi_s > 0$（要加速），稳定加速区间是

$$
-90° < \varphi_s < 0° .
$$

物理图像：晚到的粒子（$\Delta\varphi > 0$）处在场正在上升的一侧，得到更多能量，速度变大而追上；早到的粒子得到较少能量而落后。

> 注意：符号约定因文献而异。在 AVAS 和 Wangler 的余弦约定（相位越大越晚）下稳定同步相位为负，示例项目用 $-33°$。若采用正弦约定或把相位定义为 $-\omega t$，稳定区会变成正相位。从其他程序或文献移植相位时务必核对约定。

### 用 AVAS 的线性预览核对符号

同样用示例束流（初始 rms $z$ = 1.316 mm，rms $x$ = 0.954 mm）穿过一个 hwr010 腔，出口处的 rms 尺寸（线性预览，不计空间电荷）：

| 情况 | rms $z$ / mm | rms $x$ / mm |
|---|---|---|
| 0.21 m 漂移（无腔） | 1.482 | 1.407 |
| $\varphi_s = -33°$ | 0.702 | 1.811 |
| $\varphi_s = +33°$ | 1.738 | 1.196 |

负同步相位使束团纵向压缩（聚束），正同步相位使束团拉长（散束），与上面的稳定条件一致；横向的情况正好相反，这就是 [射频散焦](#longitudinal-rf-defocusing)。可以在下面的实验里拖动相位亲自验证。

```avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
```

## 小振幅同步振荡与绝热阻尼 {#longitudinal-synchrotron}

### 纵向振荡波数

稳定时 $\Delta\varphi$ 做简谐振荡 $\Delta\varphi'' + k_{l0}^2\,\Delta\varphi = 0$，**零流强纵向波数**为

$$
k_{l0}^2 = -A B\sin\varphi_s = \frac{2\pi\, q E_0 T\sin(-\varphi_s)}{mc^2\,\beta_s^3\gamma_s^3\,\lambda} .
\tag{6}
$$

同步振荡的波长 $\lambda_l = 2\pi/k_{l0}$，每个聚焦周期 $L_p$ 的纵向相移 $\sigma_{0l} = k_{l0}L_p$。由 (4)，能量振幅与相位振幅之比 $w_{\max}/\Delta\varphi_{\max} = k_{l0}/A$。式 (6) 中的 $\beta^3\gamma^3$ 说明：纵向聚焦随能量迅速变弱，低能段的同步振荡最快。

数值例：若一列腔的平均梯度等于示例腔在入口处的 $V_0 T / L = 0.39\ \mathrm{MV}/0.21\ \mathrm{m} = 1.86\ \mathrm{MV/m}$，取 $\varphi_s = -33°$，则 $k_{l0} = 4.44\ \mathrm{m}^{-1}$，$\lambda_l = 1.41\ \mathrm{m}$。实际结构中腔之间还有螺线管和漂移，平均梯度更低，$k_{l0}$ 相应减小（$\propto\sqrt{E_0 T}$）。

### 与 AVAS 纵向坐标的换算

AVAS 的纵向相空间用 $(z, \delta)$，$\delta = \Delta p/p$：

$$
z = -\frac{\beta\lambda}{2\pi}\,\Delta\varphi,\qquad
\delta = \frac{\Delta W}{\beta^2\gamma\,mc^2} = \frac{w}{\beta^2\gamma} .
$$

因此 $z$ 与 $\Delta\varphi$ 满足同样的振荡方程。`beam.txt` 的 `twissz` 用 $z$（mm）和 $\delta$（mrad），发射度同样是归一化 rms 值。示例束流为 `twissz 0.04275 0.37292 0.26506`，对应初始 rms $z = 1.316\ \mathrm{mm}$、rms $\delta = 3.53\ \mathrm{mrad}$；Twiss 参数的含义见 [相空间与发射度](#phase-space)。

### 绝热阻尼

当加速使 $\beta\gamma$、$E_0 T$ 缓慢变化时，$(\Delta\varphi, w)$ 是一对正则变量（哈密顿量见 [下一节](#longitudinal-separatrix)），作用量 $\oint w\,\dd\Delta\varphi \propto \Delta\varphi_{\max} w_{\max}$ 是绝热不变量。结合 $w_{\max}/\Delta\varphi_{\max} = k_{l0}/A$ 得到

$$
\Delta\varphi_{\max} \propto \left[E_0 T\sin(-\varphi_s)\,\lambda\,\beta_s^3\gamma_s^3\right]^{-1/4},\qquad
\Delta W_{\max} \propto \left[E_0 T\sin(-\varphi_s)\,\lambda\,\beta_s^3\gamma_s^3\right]^{1/4}.
$$

梯度、相位和频率不变时 $\Delta\varphi \propto (\beta\gamma)^{-3/4}$，$\Delta W \propto (\beta\gamma)^{3/4}$：相位宽度缩小，能散增大，乘积不变。在 $(z, \delta)$ 中，$\beta\gamma\cdot z\,\delta \propto \lambda\,\Delta\varphi\,\Delta W$ 不变，即**归一化纵向发射度守恒**，横向的情况相同（几何发射度 $\propto 1/\beta\gamma$），见 [相空间与发射度](#phase-space)。

> 注意：上面的守恒只对线性、无耦合、无空间电荷的运动成立。低能腔中的纵向非线性（相位宽度达到几十度时 $\cos\varphi$ 的弯曲）会使纵向 rms 发射度增长，线性预览不包含这一效应，要用多粒子内核计算。

## 纵向接受度与分界线 {#longitudinal-separatrix}

### 哈密顿量

方程 (4)、(5) 可以由哈密顿量（以 $s$ 为自变量，$\varphi$ 与 $w$ 共轭）

$$
H(\varphi, w) = -\frac{A w^2}{2} - B\left(\sin\varphi - \varphi\cos\varphi_s\right)
$$

导出：$\varphi' = \partial H/\partial w$，$w' = -\partial H/\partial\varphi$。$H$ 守恒，相轨迹就是 $H$ 的等值线。

### 不动点与分界线

$w = 0$ 且 $\cos\varphi = \cos\varphi_s$ 给出两个不动点：$\varphi = \varphi_s$（稳定，椭圆点）和 $\varphi_2 = -\varphi_s$（不稳定，双曲点）。经过 $\varphi_2$ 的等值线是**分界线**（separatrix）：

$$
\frac{A w^2}{2} + B\left(\sin\varphi - \varphi\cos\varphi_s\right) = -B\left(\sin\varphi_s - \varphi_s\cos\varphi_s\right).
$$

分界线内的粒子做有界的同步振荡，构成纵向**接受度**（"鱼形"或"高尔夫球杆形"的稳定区）；分界线外的粒子相位不断滑移，最终离开加速相位，成为纵向损失（在 AVAS 中可以用 `longlimits` 设置相位和能量边界来判定）。

### 稳定区的宽度和高度

- 相位范围从 $\varphi_1$ 到 $\varphi_2 = -\varphi_s$，$\varphi_1$ 由 $\sin\varphi_1 - \varphi_1\cos\varphi_s = -\sin\varphi_s + \varphi_s\cos\varphi_s$ 求出。小 $\lvert\varphi_s\rvert$ 时总宽度 $\psi \approx 3\lvert\varphi_s\rvert$。
- 能量半高度在 $\varphi = \varphi_s$ 处：

$$
\Delta W_{\max} = 2\left[\frac{q E_0 T\,\beta_s^3\gamma_s^3\,\lambda\, mc^2\left(\varphi_s\cos\varphi_s - \sin\varphi_s\right)}{2\pi}\right]^{1/2}.
$$

用上一节的平均梯度 1.86 MV/m、示例束流能量计算：

| $\varphi_s$ | $\varphi_1$ | 相位宽度 | $\Delta W_{\max}$ / MeV |
|---|---|---|---|
| −20° | −40.3° | 60° | ±0.073 |
| −30° | −60.9° | 91° | ±0.133 |
| −33° | −67.2° | 100° | ±0.153 |

$\lvert\varphi_s\rvert$ 越大，接受度越大，但每个腔的加速 $\propto\cos\varphi_s$ 越小；低能段通常取较大的 $\lvert\varphi_s\rvert$（−30° 到 −40° 左右）保证接受度，高能段再逐渐减小。束流的纵向尺寸应只占分界线面积的一部分，以免非线性区引起发射度增长和损失。

> 注意：分界线是对"平均到连续"的周期结构推出的。在只有一两个腔的束线、或每个腔能量增益很大的低能段，它只能作定性估计，真实的纵向接受度要用多粒子模拟（扫描入口相位和能量）确定。

## 射频散焦 {#longitudinal-rf-defocusing}

### 推导

轴附近的轴对称 TM 模式中，由 $\nabla\cdot\boldsymbol E = 0$ 和安培–麦克斯韦定律：

$$
E_r = -\frac{r}{2}\frac{\partial E_z}{\partial z},\qquad
B_\theta = \frac{r}{2c^2}\frac{\partial E_z}{\partial t} .
$$

径向力 $F_r = q(E_r - v B_\theta)$。利用沿粒子轨迹的全导数 $\dd E_z/\dd z = \partial_z E_z + \partial_t E_z/(\beta c)$ 整理：

$$
F_r = -\frac{q r}{2}\left[\frac{\dd E_z}{\dd z} - \frac{1}{\beta c\gamma^2}\frac{\partial E_z}{\partial t}\right].
$$

对整个间隙积分时，全导数项在场为零的两端消失（假设 $r$、$\beta$ 在间隙内近似不变），剩下的项给出

$$
\Delta p_r = -\frac{\pi q E_0 T L\, r\sin\varphi}{c\,\beta^2\gamma^2\lambda},\qquad
\Delta r' = \frac{\Delta p_r}{p} = -\frac{\pi q E_0 T L\sin\varphi}{mc^2\,\beta^3\gamma^3\lambda}\, r .
\tag{7}
$$

$\sin\varphi < 0$（纵向稳定）时 $\Delta r'$ 与 $r$ 同号，**间隙是散焦透镜**，焦距 $1/f = \pi q E_0 T L\sin(-\varphi)/(mc^2\beta^3\gamma^3\lambda)$。纵向聚焦和横向散焦来自同一个 $\sin\varphi$，二者不能在同一相位同时得到。

### 与纵向聚焦的关系

把 (7) 平均到单位长度，和 (6) 比较：

$$
k_{r}^2 = -\frac{\pi q E_0 T\sin(-\varphi_s)}{mc^2\,\beta^3\gamma^3\lambda} = -\frac{k_{l0}^2}{2} .
$$

这是静电学 Earnshaw 定理（拉普拉斯方程）在束流静止系中的体现：在无源区域，三个方向的聚焦强度之和为零，纵向聚焦必然伴随两个横向方向的散焦。上一节的数值例中 $k_r^2 = -9.9\ \mathrm{m^{-2}}$，必须由螺线管或四极铁提供更强的横向聚焦（见 [螺线管](#solenoid)、[四极铁](#quadrupole)）。更一般的 Panofsky–Wenzel 定理把横向冲量与纵向冲量的横向梯度联系起来，同样说明纵向能量增益随横向位置的变化必然伴随横向偏转。

磁场项 $vB_\theta$ 与电场项之比约为 $\beta^2$，低能时很小：在示例中把 `Kb` 从 −1.36 改为 0，线性预览的出口 rms $x$ 只改变约 0.01 %，rms $y$ 约 0.2 %。场图的 $B$ 分量仍应与 $E$ 按同一个模式一致地给出。

## 在 AVAS 中 {#longitudinal-avas}

### 关键字

射频腔是 `field` 元件，`type = 1`：

```avas-params
field input:scanphase input:longlimits beam:twissz beam:frequency
```

```text
start
! L     R     V3 type f        phase Ke   Kb    file
field 0.21  0.02  0  1    162.5e6  -33   1.36 -1.36 hwr010
end
```

- `f` 是腔的频率；`beam.txt` 的 `frequency` 是束流频率（束团重复频率，决定每个束团的电荷 $Q = I/f$），二者可以不同（例如倍频段）。
- `Ke` 缩放电场（场图单位 MV/m），`Kb` 缩放磁场；改变腔的加速梯度时两者按比例一起改。
- `input.txt` 的 `scanphase` 控制相位扫描：0 不扫描，1 扫描相位，2 从 `scanData.txt` 读取各腔的入口相位和入口时间。
- 输出文件 `Phase.txt` 每两行给出一个射频腔入口和出口的时间、位置和同步粒子能量，可用来核对每个腔的能量增益。

### 线性预览中的纵向模型

线性预览按 1 mm 步长沿轴积分参考粒子的能量和时间，对 V3 = 0 的腔迭代求出入口相位，再以 $(z, \delta)$ 的线性聚束项和动量增加带来的阻尼推进二阶矩。它不包含纵向非线性，因而不能给出低能腔中纵向发射度的增长；能量的误差约 0.01 %，rms 尺寸通常差 1–2 %。见 [多粒子模拟要点](#simulation)。

### 常见问题

- **相位符号**：从其他程序移植时同步相位符号相反，束团会散开、横向反而聚焦；对照上面的表格判断。
- **腔的 $\beta$ 与束流不匹配**：$T(\beta)$ 很小时腔几乎不加速，且相位对能量非常敏感；先看能量增益是否合理。
- **V3 = 2 的腔在上游改动后失相**：漂移长度或上游能量改变会改变 $t_{\mathrm{in}}$，同步相位随之改变；设计阶段宜用 V3 = 0。
- **大相位宽度**：束团相位宽度接近分界线宽度时会出现纵向尾巴和损失，需要缩短入口束团或加大 $\lvert\varphi_s\rvert$。

## 延伸阅读 {#longitudinal-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：渡越时间因子、纵向动力学（分界线、绝热阻尼）与射频散焦。
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：直线加速器中的纵向运动。
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific：同步振荡与相稳定。
- CERN Accelerator School 直线加速器课程讲义（[CAS](https://cas.web.cern.ch/)）。
- 本入门的 [射频腔](#rf-cavity)、[场图](#field-maps)、[空间电荷](#space-charge) 和 [教程](#tutorial)。
