# 纵向动力学

162.5 MHz 的射频场每 6.15 ns 就在正负之间振荡一个周期。束团里的粒子到达腔的时刻各不相同，早到的和晚到的看到的场不一样，按理说能量会越来越分散、束团会越拉越长。可是在直线加速器里，束团能稳定地走过几十上百个腔。是什么把早到和晚到的粒子拉回到一起？为什么 AVAS 示例里的同步相位取 $-33°$，而不是能量增益最大的波峰？为什么让束团纵向聚集的同一个腔，又会让它横向散开？

本章从单个加速间隙的能量增益出发，引入渡越时间因子，然后用"能量决定速度、速度决定到达时刻"这条链解释相稳定原理，再讨论同步振荡、纵向接受度（分界线）和射频散焦。需要的准备：[相对论运动学](#kinematics) 中的 $\beta$、$\gamma$、$\beta\lambda$，[相空间与发射度](#phase-space) 中的发射度，以及 [漂移段](#drift) 中"动量高的粒子跑到前面"的 $L/\gamma^2$。射频腔元件本身的写法见 [射频腔](#rf-cavity)；在故事线里，本章对应 [超导加速段](#journey-sc) 选择同步相位的理由和 [中能传输段](#journey-mebt) 的聚束腔。

本章的数值例子都用示例项目 `examples/hwr010`：质子，入口动能 $W = 1.527\ \mathrm{MeV}$（$\beta = 0.05698$，$\gamma = 1.00163$），射频频率 $f = 162.5\ \mathrm{MHz}$（$\lambda = c/f = 1.8449\ \mathrm{m}$，$\beta\lambda = 0.1051\ \mathrm{m}$），半波长谐振腔（HWR）场图 `hwr010`，长 0.21 m，`Ke = 1.36`。

## 射频间隙中的能量增益 {#longitudinal-gap}

### 场与相位

腔里的电场是驻波：空间分布固定，大小随时间按余弦振荡。粒子穿过腔要花一段时间，它在每个位置看到的是"它到达那一刻"的场。所以同一个腔给粒子多少能量，取决于粒子什么时候到——用射频相位来记这个时刻最方便。

设腔内轴上纵向电场为

$$
E_z(z, t) = E(z)\cos\left(\omega t + \varphi_0\right),
$$

$E(z)$ 是场的空间分布，$\omega = 2\pi f$。粒子到达 $z$ 的时刻为 $t(z)$，它在 $z$ 处"看到"的相位是

$$
\varphi(z) = \omega t(z) + \varphi_0 .
$$

在这个约定下，**相位越大表示粒子越晚到达**，$\varphi = 0$ 是场的波峰。本章和 AVAS 一致采用这个余弦约定（与 Wangler《RF Linear Accelerators》相同，见 [在 AVAS 中](#longitudinal-in-avas)）。粒子穿过间隙得到的能量增益是

$$
\Delta W = q\int E(z)\cos\varphi(z)\,\dd z .
\tag{1}
$$

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

$E_0 = V_0/L$ 是间隙长度 $L$ 上的平均轴上场，$V_0 = \int E\,\dd z$ 是轴上电压。**渡越时间因子** $T \le 1$ 描述的是：粒子穿过间隙时场还在变化，它不能一直赶上场的最大值。对长 $g$ 的均匀场间隙，

$$
T = \frac{\sin(\pi g/\beta\lambda)}{\pi g/\beta\lambda},
$$

间隙越短于 $\beta\lambda$，$T$ 越接近 1。示例束流 $\beta\lambda = 105\ \mathrm{mm}$：间隙长 $\beta\lambda/4 = 26\ \mathrm{mm}$ 时 $T = 0.90$，长 $\beta\lambda/2$ 时只剩 $2/\pi = 0.64$。实际腔的场在间隙外有延伸，$T$ 要用场分布积分。

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

半波长谐振腔（HWR）、四分之一波长腔（QWR）的加速部分是两个间隙，场方向相反（$\pi$ 模）。要两个间隙都加速，粒子从第一个间隙飞到第二个间隙必须恰好用半个射频周期，场正好反了号。把单个间隙的场记为 $e(z)$，两个间隙中心相距 $d$：$E(z) = e(z - d/2) - e(z + d/2)$。其傅里叶变换给出

$$
T(\beta) = T_g(\beta)\,\left\lvert\sin\frac{k d}{2}\right\rvert = T_g(\beta)\,\left\lvert\sin\frac{\pi\beta_g}{2\beta}\right\rvert,
\qquad d = \frac{\beta_g\lambda}{2},
$$

$T_g$ 是单个间隙的渡越时间因子。对正负交替的多间隙场 $\int E\,\dd z \approx 0$，此时式 (2) 的分母改用 $V_0 = \int\lvert E\rvert\,\dd z$（下表即如此）。粒子速度恰好为 $\beta = \beta_g$ 时，两个间隙同相加速；速度偏离越多，第二个间隙越"不合拍"。$N$ 个间隙的腔 $T(\beta)$ 更尖锐，能量接受范围更窄；低 $\beta$ 超导直线加速器因此用间隙少的腔，并分成几种几何 $\beta$（见 [超导加速段](#journey-sc)）。

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

$T(\beta)$ 最大值处的 $\beta \approx 0.10$ 与名字中的 "010"（通常表示设计 $\beta = 0.10$）相符；它比由间距估计的 0.088 稍大，是因为单间隙的 $T_g(\beta)$ 随 $\beta$ 增大。示例束流的 $\beta = 0.057$ 远低于最佳 $\beta$，所以 $T$ 只有 0.40；粒子在腔内被加速后 $T$ 逐渐增大，这也是低能腔里匀速近似失效的原因之一。下图是这张场图的切片，两个间隙的场方向相反：

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

$\varphi_s = -90°$ 时能量增益为零，符合 (3)。但 $\Delta W/\cos\varphi_s$ 在 −33° 时为 0.405 MeV，在 0° 时为 0.475 MeV，相差约 15 %，而且用入口相位扫描时最大能量增益（出口能量 2.0087 MeV）出现在 $\varphi_s \approx +8°$ 而不是 0°。这是真实的物理，还是预览求解相位的方式造成的？有一个简单的检验可以把两者分开：把同一场图再积分一次，但让速度冻结在入口值不变。这时在所有相位上 $\Delta W = qV_0T\cos\varphi_s$ 都在 1 % 以内成立，$V_0T = 0.389\ \mathrm{MV}$（−33° 时 0.325 MeV，+33° 时 0.328 MeV，最大值在 $\varphi_s \approx 0$）。可见不对称完全来自粒子在腔内加速：$\Delta W/W \approx 30\ \%$，$\beta$ 从 0.057 增大到约 0.063 至 0.065。

为什么加速会偏向一侧？把两个间隙分开看。对这个腔来说粒子太慢了：从一个间隙中心走到另一个需要约 0.77 个射频周期，而不是半个周期，所以两个间隙不可能都靠近波峰。$\varphi_s = -33°$ 时，第一个间隙处在约 −81°，只给 0.043 MeV，第二个间隙接近波峰，给 0.297 MeV；$\varphi_s$ 是两者的加权平均。$\varphi_s = +33°$ 时正好反过来：第一个间隙接近波峰，给 0.292 MeV，粒子随后明显变快、更早到达第二个间隙，使那里的相位从波峰后约 79° 提前到约 66°，增益从 0.054 MeV 增加到 0.132 MeV。能量在腔里的哪个位置获得，决定了腔后半部分的时间关系移动多少——这就是这里"有效电压随相位变化"的含义。对这种腔，Panofsky 公式只能作估算，设计要以沿轨迹积分的结果为准。内核同样按 (3) 沿参考轨迹定义 $\varphi_s$（见 [在 AVAS 中](#longitudinal-in-avas)），所以这是物理和这一定义本身的性质，不是预览的问题；以上数值来自预览对参考粒子的轴上积分。

## 同步粒子与同步相位 {#longitudinal-synchronous}

束团里有成千上万个粒子，要描述"束团在哪里、什么相位"，需要一个参照。**同步粒子**（参考粒子）就是设计上沿轴运动、在每个腔都处在设计相位的那个假想粒子；其他粒子的纵向坐标都是相对它定义的。同步粒子在某个腔里"看到"的相位就是该腔的**同步相位** $\varphi_s$，按式 (3) 定义。

同步粒子到达第 $n$ 个腔时的相位由两件事共同决定：腔的射频相位，以及它在前面各段的飞行时间。腔与腔之间的漂移会让相位滑移 $360°\cdot L/(\beta\lambda)$：示例束流飞过 0.1 m 漂移，相位就前进 $342.4°$。低能时 $\beta\lambda$ 只有十几厘米，几厘米的长度误差或一点能量偏差都会明显改变下游腔的相位，这就是低能段相位设置敏感的根本原因。

AVAS 中 `field` 的参数 V3 决定相位参数是"同步相位""入口相位"还是"绝对相位"，三种写法和例子见 [在 AVAS 中](#longitudinal-in-avas)。

## 相稳定原理 {#longitudinal-stability}

### 思想实验：三个粒子

设想三个粒子前后跟着进入一列腔：同步粒子在相位 $\varphi_s$，一个早到一点（相位更小），一个晚到一点（相位更大）。能量增益 $\propto\cos\varphi$，波峰在 $\varphi = 0$。

- 若 $\varphi_s$ 在**波峰之前**（$-90° < \varphi_s < 0$，场正在上升）：晚到的粒子更靠近波峰，得到**更多**能量；早到的离波峰更远，得到更少。非相对论粒子能量高就跑得快，于是在接下来的漂移里，晚到的追上来，早到的落回去。偏离被纠正——这是一个恢复力。
- 若 $\varphi_s$ 在**波峰之后**（$0 < \varphi_s < 90°$）：晚到的得到更少能量、更慢，越落越后；早到的越跑越前。偏离被放大——不稳定。
- 若 $\varphi_s$ 正好在**波峰**：早到和晚到的都比同步粒子少一点能量，差别是偏离的二阶小量，没有一阶的恢复力，束团只会在漂移里继续拉长。

所以纵向聚焦由两个环节组成：腔让能量随相位变化（"透镜"），漂移让速度差变成位置差（[漂移段](#drift-longitudinal) 的 $R_{56} = L/\gamma^2$）。这和横向的"薄透镜 + 漂移"完全同构，纵向稳定也可以用同样的矩阵迹判据来判断。在直线段里 $R_{56} = L/\gamma^2$ 总是正的，所以稳定相位总是在场上升的一侧。

### 运动方程

把上面的图像写成方程。考虑一列腔组成的周期结构，把每个腔的作用平均到单位长度上：平均加速梯度 $E_0 T$（腔内 $E_0 T$ 乘以腔长占周期长度的比例）。定义相对同步粒子的偏离

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

式 (4) 是"速度差变成相位差"：能量高的粒子（$w > 0$）跑得快，相位减小（提前到达）。系数 $A \propto 1/(\beta^3\gamma^3)$ 在示例能量下是 $1.83\times 10^4\ \mathrm{m^{-1}}$，低能质子的相位对能量极其敏感。式 (5) 是"相位差变成能量差"。

### 稳定条件

对 (4)、(5) 线性化，$\cos\varphi - \cos\varphi_s \approx -\sin\varphi_s\,\Delta\varphi$：

$$
\frac{\dd^2\Delta\varphi}{\dd s^2} = A B \sin\varphi_s\,\Delta\varphi .
$$

$A, B > 0$，所以**只有 $\sin\varphi_s < 0$ 时运动稳定**——右边成为恢复力。结合 $\cos\varphi_s > 0$（要加速），稳定加速区间是

$$
-90° < \varphi_s < 0° .
$$

这正是思想实验的结论：同步相位为负，意味着同步粒子在波峰之前到达。

> 注意：符号约定因文献而异。在 AVAS 和 Wangler 的余弦约定（相位越大越晚）下稳定同步相位为负，示例项目用 $-33°$。若采用正弦约定或把相位定义为 $-\omega t$，稳定区会变成正相位。从其他程序或文献移植相位时务必核对约定。

### 用 AVAS 的线性预览核对符号

同样用示例束流（初始 rms $z$ = 1.316 mm，rms $x$ = 0.954 mm）穿过一个 hwr010 腔，出口处的 rms 尺寸（线性预览，不计空间电荷）：

| 情况 | rms $z$ / mm | rms $x$ / mm |
|---|---|---|
| 0.21 m 漂移（无腔） | 1.482 | 1.407 |
| $\varphi_s = -33°$ | 0.702 | 1.811 |
| $\varphi_s = +33°$ | 1.738 | 1.196 |

负同步相位使束团纵向压缩（聚束），正同步相位使束团拉长（散束），与上面的稳定条件一致；横向的情况正好相反，这就是 [射频散焦](#longitudinal-rf-defocusing)。顺便注意：$+33°$ 时能量增益是 0.424 MeV，比 $-33°$ 的 0.340 MeV 还多——低能腔里有效电压随相位变化，正负相位并不对称（原因是粒子在第一个间隙里加速，见 [上文](#longitudinal-multigap)）。在下面的实验里拖动相位：看负相位时 rms $z$ 变小、rms $x$ 变大，正相位时反过来；相位扫描图上 $\varphi_s$ 在波峰哪一侧，一目了然。

```avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
```

> **想一想**：10 MeV 的电子 $\gamma \approx 20.6$。电子直线加速器里还需要相稳定吗？
>
> 答：几乎不需要。式 (4) 的 $A \propto 1/(\beta^3\gamma^3)$，10 MeV 电子的 $A$ 只有示例质子的 $2\times 10^{-8}$ 倍：能量差几乎不改变速度，粒子的相位被"冻结"了。电子直线加速器因此可以把束团放在波峰附近（$\varphi_s \approx 0$）获得最大能量增益，纵向不会因为能量差而散开。低能质子则相反，必须靠相稳定把束团拉住。

## 小振幅同步振荡与绝热阻尼 {#longitudinal-synchrotron}

### 纵向振荡波数

稳定时 $\Delta\varphi$ 做简谐振荡 $\Delta\varphi'' + k_{l0}^2\,\Delta\varphi = 0$，粒子在同步粒子前后来回摆动，能量也随之高低交替，这就是**同步振荡**。**零流强纵向波数**为

$$
k_{l0}^2 = -A B\sin\varphi_s = \frac{2\pi\, q E_0 T\sin(-\varphi_s)}{mc^2\,\beta_s^3\gamma_s^3\,\lambda} .
\tag{6}
$$

同步振荡的波长 $\lambda_l = 2\pi/k_{l0}$，每个聚焦周期 $L_p$ 的纵向相移 $\sigma_{0l} = k_{l0}L_p$。由 (4)，能量振幅与相位振幅之比 $w_{\max}/\Delta\varphi_{\max} = k_{l0}/A$。式 (6) 中的 $\beta^3\gamma^3$ 说明：纵向聚焦随能量迅速变弱，低能段的同步振荡最快。

数值例：若一列腔的平均梯度等于示例腔在入口处的 $V_0 T / L = 0.39\ \mathrm{MV}/0.21\ \mathrm{m} = 1.86\ \mathrm{MV/m}$，取 $\varphi_s = -33°$，则 $k_{l0} = 4.44\ \mathrm{m}^{-1}$，$\lambda_l = 1.41\ \mathrm{m}$。实际结构中腔之间还有螺线管和漂移，平均梯度更低，$k_{l0}$ 相应减小（$\propto\sqrt{E_0 T}$）。

也可以把一个腔看成纵向的薄透镜。按 (5) 线性化并换成 $(z, \delta)$ 坐标（见下），一个腔的纵向焦距满足 $1/f_z = 2\pi qV_0T\sin(-\varphi_s)/(mc^2\beta^3\gamma^3\lambda)$。示例腔、$\varphi_s = -33°$ 时 $1/f_z \approx 4.1\ \mathrm{m^{-1}}$，$f_z \approx 0.24\ \mathrm m$——比上一章 FODO 实验里的四极铁（$f = 0.45\ \mathrm m$）还强。这只是按平均公式的粗估（腔内 $\beta$ 变化约 14 %），但它解释了上表里 rms $z$ 为什么一个腔就从 1.48 mm 压到 0.70 mm。

### 与 AVAS 纵向坐标的换算

AVAS 的纵向相空间用 $(z, \delta)$，$\delta = \Delta p/p$：

$$
z = -\frac{\beta\lambda}{2\pi}\,\Delta\varphi,\qquad
\delta = \frac{\Delta W}{\beta^2\gamma\,mc^2} = \frac{w}{\beta^2\gamma} .
$$

因此 $z$ 与 $\Delta\varphi$ 满足同样的振荡方程。示例束流的 `twissz 0.04275 0.37292 0.26506` 对应初始 rms $z = 1.316\ \mathrm{mm}$（rms 相位宽度 4.5°）、rms $\delta = 3.53\ \mathrm{mrad}$（rms 能散 10.8 keV）；Twiss 参数的含义见 [相空间与发射度](#phase-space)，`twissz` 的写法见 [在 AVAS 中](#longitudinal-in-avas)。

### 绝热阻尼

当加速使 $\beta\gamma$、$E_0 T$ 缓慢变化时，$(\Delta\varphi, w)$ 是一对正则变量（哈密顿量见 [下一节](#longitudinal-separatrix)），作用量 $\oint w\,\dd\Delta\varphi \propto \Delta\varphi_{\max} w_{\max}$ 是绝热不变量——就像摆长缓慢变化的单摆，振幅会变，但相空间面积不变。结合 $w_{\max}/\Delta\varphi_{\max} = k_{l0}/A$ 得到

$$
\Delta\varphi_{\max} \propto \left[E_0 T\sin(-\varphi_s)\,\lambda\,\beta_s^3\gamma_s^3\right]^{-1/4},\qquad
\Delta W_{\max} \propto \left[E_0 T\sin(-\varphi_s)\,\lambda\,\beta_s^3\gamma_s^3\right]^{1/4}.
$$

梯度、相位和频率不变时 $\Delta\varphi \propto (\beta\gamma)^{-3/4}$，$\Delta W \propto (\beta\gamma)^{3/4}$：相位宽度缩小，能散增大，乘积不变。例如从 1.527 MeV 加速到 10 MeV，相位宽度缩小到 0.49 倍，绝对能散增大到 2.0 倍，而相对能散 $\Delta W/W$ 下降到约 0.31 倍。束团在相位上越来越短，这也是下游可以把频率加倍的原因（见 [从离子源到靶](#journey-overview)）。在 $(z, \delta)$ 中，$\beta\gamma\cdot z\,\delta \propto \lambda\,\Delta\varphi\,\Delta W$ 不变，即**归一化纵向发射度守恒**，横向的情况相同（几何发射度 $\propto 1/\beta\gamma$），见 [相空间与发射度](#phase-space)。

> 注意：上面的守恒只对线性、无耦合、无空间电荷的运动成立。低能腔中的纵向非线性（相位宽度达到几十度时 $\cos\varphi$ 的弯曲）会使纵向 rms 发射度增长，线性预览不包含这一效应，要用多粒子内核计算。

## 纵向接受度与分界线 {#longitudinal-separatrix}

### 物理图像：倾斜搓衣板上的小球

小振幅时纵向运动像单摆，大振幅时呢？把运动方程写成哈密顿形式就能看清全貌。方程 (4)、(5) 可以由哈密顿量（以 $s$ 为自变量，$\varphi$ 与 $w$ 共轭）

$$
H(\varphi, w) = -\frac{A w^2}{2} - B\left(\sin\varphi - \varphi\cos\varphi_s\right)
$$

导出：$\varphi' = \partial H/\partial w$，$w' = -\partial H/\partial\varphi$。$H$ 守恒，相轨迹就是 $H$ 的等值线。把 $-H$ 看成"动能 + 势能"，势能就是 $U(\varphi) = B(\sin\varphi - \varphi\cos\varphi_s)$：一条正弦形的起伏，再叠加一个线性倾斜 $-B\varphi\cos\varphi_s$。这就像一块倾斜的搓衣板，粒子是板上的小球。正弦起伏来自射频场，倾斜来自同步粒子必须获得的那份加速。

- 小球在一个坑里来回滚动，就是有界的同步振荡；
- 坑的下坡一侧有一道"坎"，能量超过坎的小球会越过它，沿着倾斜的板一路滚下去——相位不断滑移，再也回不来，这就是纵向损失。

这个类比在数学上是严格的，边界在于：$A$、$B$ 随能量缓慢变化（坑在慢慢变形），而且真实的腔是离散的，"平均到连续"只在每个周期的纵向相移不大时成立。

### 不动点与分界线

$w = 0$ 且 $\cos\varphi = \cos\varphi_s$ 给出两个不动点：$\varphi = \varphi_s$（稳定，坑底，椭圆点）和 $\varphi_2 = -\varphi_s$（不稳定，坎顶，双曲点）。经过 $\varphi_2$ 的等值线是**分界线**（separatrix）：

$$
\frac{A w^2}{2} + B\left(\sin\varphi - \varphi\cos\varphi_s\right) = -B\left(\sin\varphi_s - \varphi_s\cos\varphi_s\right).
$$

分界线内的粒子做有界的同步振荡，构成纵向**接受度**（"鱼形"或"高尔夫球杆形"的稳定区，常称为 bucket）；分界线外的粒子相位不断滑移，最终离开加速相位，成为纵向损失。

### 稳定区的宽度和高度

- 相位范围从 $\varphi_1$ 到 $\varphi_2 = -\varphi_s$，$\varphi_1$ 由 $\sin\varphi_1 - \varphi_1\cos\varphi_s = -\sin\varphi_s + \varphi_s\cos\varphi_s$ 求出。小 $\lvert\varphi_s\rvert$ 时总宽度 $\psi \approx 3\lvert\varphi_s\rvert$。
- 能量半高度在 $\varphi = \varphi_s$ 处：

$$
\Delta W_{\max} = 2\left[\frac{q E_0 T\,\beta_s^3\gamma_s^3\,\lambda\, mc^2\left(\varphi_s\cos\varphi_s - \sin\varphi_s\right)}{2\pi}\right]^{1/2}.
$$

坑的深度正比于 $\varphi_s\cos\varphi_s - \sin\varphi_s$：$\varphi_s = 0$ 时为零（没有坑），$\lvert\varphi_s\rvert$ 越大坑越深。用上一节的平均梯度 1.86 MV/m、示例束流能量计算：

| $\varphi_s$ | $\varphi_1$ | 相位宽度 | $\Delta W_{\max}$ / MeV | $\cos\varphi_s$ |
|---|---|---|---|---|
| −20° | −40.3° | 60° | ±0.073 | 0.940 |
| −30° | −60.9° | 91° | ±0.133 | 0.866 |
| −33° | −67.2° | 100° | ±0.153 | 0.839 |

这就是开头问题的答案：$\lvert\varphi_s\rvert$ 越大，接受度越大，但每个腔的加速 $\propto\cos\varphi_s$ 越小。示例的 $-33°$ 付出了约 16 % 的能量增益，换来 100° 宽、±0.15 MeV 高的稳定区，而束团本身的 rms 相位宽度只有 4.5°、rms 能散 10.8 keV，有充足的余量。低能段通常取较大的 $\lvert\varphi_s\rvert$（−30° 到 −40° 左右）保证接受度，高能段束团在相位上变短（绝热阻尼），再逐渐减小 $\lvert\varphi_s\rvert$。束流的纵向尺寸应只占分界线面积的一部分，以免非线性区引起发射度增长和损失。

> **想一想**：为什么不能把同步相位放在波峰 $\varphi_s = 0$，再靠别的办法聚束？
>
> 答：$\varphi_s = 0$ 时 $U(\varphi) = B(\sin\varphi - \varphi)$ 单调下降，没有坑，分界线面积为零：任何相位偏差都没有恢复力。低能质子的 $A$ 很大，一点能量差很快就变成大的相位差，束团会在几个腔内散开。要么在腔之间另加聚束腔，要么让加速腔本身离开波峰——后者就是选择 $\varphi_s < 0$。

> 注意：分界线是对"平均到连续"的周期结构推出的。在只有一两个腔的束线、或每个腔能量增益很大的低能段，它只能作定性估计，真实的纵向接受度要用多粒子模拟（扫描入口相位和能量）确定。

## 射频散焦 {#longitudinal-rf-defocusing}

### 物理图像

为什么让束团纵向聚集的腔，会让它横向散开？有两种互相印证的看法。

**静止系的看法（Earnshaw 定理）**：在跟着束团走的参考系里，粒子相对运动很慢，腔的场近似是静电场。静电势在无源区满足拉普拉斯方程，三个方向的二阶导数之和为零。纵向要稳定，势在 $z$ 方向必须是一个坑（二阶导数为正），那么至少有一个横向方向的二阶导数为负——对轴对称的腔，两个横向方向都是。纵向聚焦和横向散焦来自同一个势，不可能只要一个。

**间隙的看法**：间隙入口处电场线向轴内收拢（$\partial E_z/\partial z > 0$，$E_r$ 指向轴，聚焦），出口处向外张开（散焦）。如果场不随时间变，两半大致抵消；但在稳定相位 $\varphi_s < 0$ 时，粒子穿过间隙的过程中场正在**增强**（还没到波峰），出口那半的散焦比入口那半的聚焦更强，净效果是散焦。

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

对整个间隙积分时，全导数项在场为零的两端消失（假设 $r$、$\beta$ 在间隙内近似不变），剩下的就是"场随时间变化"那一项——正是间隙看法里的物理。结果是

$$
\Delta p_r = -\frac{\pi q E_0 T L\, r\sin\varphi}{c\,\beta^2\gamma^2\lambda},\qquad
\Delta r' = \frac{\Delta p_r}{p} = -\frac{\pi q E_0 T L\sin\varphi}{mc^2\,\beta^3\gamma^3\lambda}\, r .
\tag{7}
$$

$\sin\varphi < 0$（纵向稳定）时 $\Delta r'$ 与 $r$ 同号，**间隙是散焦透镜**，焦距 $1/f = \pi q E_0 T L\sin(-\varphi)/(mc^2\beta^3\gamma^3\lambda)$。纵向聚焦和横向散焦来自同一个 $\sin\varphi$，二者不能在同一相位同时得到。

数值：示例腔（$V_0T = E_0TL = 0.39\ \mathrm{MV}$）、$\varphi_s = -33°$，按 (7) 粗估 $1/f \approx 2.07\ \mathrm{m^{-1}}$，$f \approx 0.48\ \mathrm m$——一个腔的横向散焦和 FODO 实验里一个四极铁的聚焦（$f = 0.45\ \mathrm m$）差不多强，而纵向焦距 0.24 m 正好是它的一半。上一节表中 $-33°$ 时出口 rms $x$ 从漂移的 1.41 mm 增大到 1.81 mm，就是这个效应。线性预览沿实际轨迹积分场图的梯度，由它的传输矩阵得到 $x$ 方向 $-1/M_{21} \approx 0.37\ \mathrm m$、$y$ 方向 $0.45\ \mathrm m$，纵向 $0.24\ \mathrm m$：平均化的公式 (7) 给对了量级和两个方向之间的 2 倍关系，但细节不准。HWR 并非轴对称，$x$ 与 $y$ 不必相同（内核是否给出同样的 $x$/$y$ 差别尚未核对）。

### 与纵向聚焦的关系

把 (7) 平均到单位长度，和 (6) 比较：

$$
k_{r}^2 = -\frac{\pi q E_0 T\sin(-\varphi_s)}{mc^2\,\beta^3\gamma^3\lambda} = -\frac{k_{l0}^2}{2} .
$$

这就是 Earnshaw 定理的定量形式：在无源区域，三个方向的聚焦强度之和为零，纵向聚焦必然伴随两个横向方向各一半强度的散焦。上一节的数值例中 $k_r^2 = -9.9\ \mathrm{m^{-2}}$，必须由螺线管或四极铁提供更强的横向聚焦（见 [螺线管](#solenoid)、[四极铁](#quadrupole)）；这是低能超导段让螺线管与腔交替排列的重要原因之一。更一般的 Panofsky–Wenzel 定理把横向冲量与纵向冲量的横向梯度联系起来，同样说明纵向能量增益随横向位置的变化必然伴随横向偏转。

磁场项 $vB_\theta$ 与电场项之比约为 $\beta^2$，低能时很小：在示例中把 `Kb` 从 −1.36 改为 0，线性预览的出口 rms $x$ 只改变约 0.01 %，rms $y$ 约 0.2 %。场图的 $B$ 分量仍应与 $E$ 按同一个模式一致地给出。

## 容易误解的地方 {#longitudinal-misconceptions}

- **同步相位的符号**：AVAS 用余弦约定、相位越大越晚，稳定区是 $-90° < \varphi_s < 0$。正弦约定或 $-\omega t$ 约定下稳定区是正相位。移植相位时符号错了，束团会纵向散开、横向反而聚焦。
- **同步相位不是入口相位**：示例腔 $\varphi_s = -33°$ 对应的入口相位是 52.7°，两者相差约 86°，取决于场图的零点和场分布，没有通用换算。
- **$\varphi_s = 0$ 能量增益最大？** 只在匀速近似下成立。示例的低 $\beta$ 腔最大增益在 $\varphi_s \approx +8°$，而且 $\pm 33°$ 的增益不对称。这不是数值误差：把速度冻结后不对称就消失了，它来自腔内的加速。
- **Panofsky 公式是精确的？** 它假设腔内速度不变；$\Delta W/W$ 达到百分之几十时，只能作估算。
- **能散越来越大说明束流变差？** 不是。绝热阻尼使绝对能散 $\propto(\beta\gamma)^{3/4}$ 增大，但相位宽度同时缩小，归一化纵向发射度不变，相对能散还在减小。
- **束团可以占满分界线？** 分界线边缘附近运动高度非线性，粒子在那里转得慢、容易丝化，发射度增长和损失都从那里开始。束团应只占分界线的一小部分。
- **射频散焦是腔设计的缺陷？** 不是。它是拉普拉斯方程的结果，任何纵向稳定的加速相位都伴随横向散焦，只能用外部聚焦补偿。
- **腔的频率就是束流频率？** 不一定。`field` 的 `f` 是腔的频率，`beam.txt` 的 `frequency` 是束团重复频率，倍频段两者不同。
- **电子和质子一样？** 高能电子的相位几乎冻结（$A \propto 1/\beta^3\gamma^3$），相稳定的需求和低能质子完全不同。

## 在 AVAS 中 {#longitudinal-in-avas}

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
- `longlimits` 设置纵向损失判定的相位和能量边界，对应上文分界线的概念：越过边界的粒子记为纵向损失。
- 输出文件 `Phase.txt` 每两行给出一个射频腔入口和出口的时间、位置和同步粒子能量，可用来核对每个腔的能量增益。
- `beam.txt` 的 `twissz` 是纵向 Twiss 参数：$\alpha_z$、$\beta_z$（mm/mrad，$z$ 单位 mm、$\delta = \Delta p/p$ 单位 mrad）、归一化 rms 纵向发射度（π·mm·mrad），与上文的 $(z, \delta)$ 换算一致。

### 射频场与相位的约定

> **AVAS 约定**：射频电场按 $K_e\,E(\boldsymbol r)\cos(\omega t + \varphi_0)$ 作用，射频磁场按 $K_b\,B(\boldsymbol r)\sin(\omega t + \varphi_0)$ 作用（场图中 $E$ 单位 MV/m，$B$ 单位 T）。这与 Wangler《RF Linear Accelerators》的余弦约定一致。纵向坐标 $z$ 与相位的关系为 $\Delta\varphi = -2\pi z/(\beta\lambda)$，即 $z > 0$ 的粒子在参考粒子前方、相位较小。

### 相位的三种含义（V3）

`field L R V3 type f phase Ke Kb file` 的第 3 个参数 V3 决定第 6 个参数 `phase` 的含义：

| V3 | `phase` 的含义 | 内核如何确定腔的入口相位 $\varphi_0$ |
|---|---|---|
| 0 | 同步相位 $\varphi_s$ | 求解 $\varphi_0$，使沿参考粒子轨迹的 $\operatorname{atan2}(S, C) = \varphi_s$，见式 (3) |
| 1 | 参考粒子到达元件入口时的射频相位 | $\varphi_0$ = `phase` |
| 2 | $t = 0$ 时刻的射频相位（绝对相位） | $\varphi_0 = $ `phase` $+ 360°\cdot f\cdot t_{\mathrm{in}}$，$t_{\mathrm{in}}$ 是参考粒子到达入口的时刻 |

V3 = 0 最方便设计：物理上有意义的是同步相位，入口相位由程序反求。V3 = 1 适合做相位扫描（入口相位与腔的几何直接对应）。V3 = 2 对应真实机器上"每个腔的射频相对公共参考时钟的相位"，上游元件改变时同步相位会跟着变。

例：示例腔前加一段 0.1 m 漂移，参考粒子到达腔入口的时刻 $t_{\mathrm{in}} = 0.1/(\beta c) = 5.854\ \mathrm{ns}$，$360°\,f\,t_{\mathrm{in}} = 342.45°$。V3 = 2、`phase = 0` 时入口相位为 $342.45° \equiv -17.55°$，对应同步相位约 −93.7°——这个腔几乎不加速。而 V3 = 0、`phase = -33` 时，无论前面漂移多长，入口相位都是 52.7°，同步相位保持 −33°。

> **AVAS 约定**：示例中同步相位 $-33°$ 对应的入口相位是 $52.7°$，两者之差（约 86°）取决于场图的零点位置和场分布，没有通用换算；对 V3 = 0 的腔，线性预览会给出求得的入口相位与同步相位。分段运行把一段 lattice 单独拿出来时，AVAS（`avas/data/segment.py` 的 `rephase`）从参考运行的同步粒子数据取每个腔的射频相位 $\varphi_{\mathrm{RF}}$，按 $\varphi_{\mathrm{RF}} - 360°\cdot f\cdot(t_{\mathrm{in}} - T_{\mathrm{entry}})$ 换算成以分段入口时刻为零点的 V3 = 2 相位，而不是沿用原来的相位或改成 V3 = 0。

### 线性预览中的纵向模型

线性预览按 1 mm 步长沿轴积分参考粒子的能量和时间，对 V3 = 0 的腔迭代求出入口相位，再以 $(z, \delta)$ 的线性聚束项和动量增加带来的阻尼推进二阶矩。它不包含纵向非线性，因而不能给出低能腔中纵向发射度的增长；能量的误差约 0.01 %，rms 尺寸通常差 1–2 %。见 [多粒子模拟要点](#simulation)。

### 常见问题

- **相位符号**：从其他程序移植时同步相位符号相反，束团会散开、横向反而聚焦；对照上面 ±33° 的表格判断。
- **腔的 $\beta$ 与束流不匹配**：$T(\beta)$ 很小时腔几乎不加速，且相位对能量非常敏感；先看能量增益是否合理。
- **V3 = 2 的腔在上游改动后失相**：漂移长度或上游能量改变会改变 $t_{\mathrm{in}}$，同步相位随之改变；设计阶段宜用 V3 = 0。
- **大相位宽度**：束团相位宽度接近分界线宽度时会出现纵向尾巴和损失，需要缩短入口束团或加大 $\lvert\varphi_s\rvert$。

## 延伸阅读 {#longitudinal-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：渡越时间因子、纵向动力学（分界线、绝热阻尼）与射频散焦。
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：直线加速器中的纵向运动。
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific：同步振荡与相稳定。
- CERN Accelerator School 直线加速器课程讲义（[CAS](https://cas.web.cern.ch/)）。
- 本入门的 [射频腔](#rf-cavity)、[场图](#field-maps)、[空间电荷](#space-charge) 和 [教程](#tutorial)。
