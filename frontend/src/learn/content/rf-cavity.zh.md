# 射频腔

射频腔是直线加速器里唯一给束流加能量的元件：它在谐振的电磁场中让粒子"冲浪"，同时还对束流施加纵向聚束和横向（散）焦作用。AVAS 不用传输矩阵近似射频腔，而是读入腔的三维场图（`field` 元件，场类型 1），用 t-code 在真实场里逐步积分粒子运动。本章从谐振腔的基本模式讲起，推导能量增益、渡越时间因子和射频聚焦，最后说明 AVAS 中 `field` 的参数、相位约定和扫相做法。

本章的数值例子都基于示例项目 `examples/hwr010`：质子，注入动能 $W = 1.527\ \mathrm{MeV}$，射频频率 $f = 162.5\ \mathrm{MHz}$，半波长谐振腔场图 `hwr010`（长 0.21 m，两个加速间隙）。数字由 AVAS 自己的线性包络预览（`avas.sim.linear_optics`）算出，最终以内核结果为准。

## 作用与位置 {#rf-cavity-role}

在一条直线加速器里，射频腔承担三件事：

- **加速**：同步粒子每通过一个腔获得能量 $\Delta W = qV_0T\cos\varphi_s$（下文推导）。
- **纵向聚焦**：同步相位取在电场上升沿（AVAS 的余弦约定下 $-90^\circ < \varphi_s < 0$）时，早到的粒子获得较少能量、晚到的粒子获得较多能量，束团在纵向相空间里绕同步粒子振荡而不散开，参见 [纵向运动](#longitudinal)。
- **横向散焦**：由 Maxwell 方程，纵向聚焦的相位必然伴随横向散焦（Earnshaw / Panofsky 定理的射频形式），因此腔与腔之间要放螺线管或四极铁补偿，参见 [螺线管](#solenoid)、[四极铁](#quadrupole)。

低能段（$\beta \lesssim 0.2$）的超导质子直线加速器通常是"一个或几个腔 + 一个螺线管"的周期结构，这正是 [教程](#tutorial) 搭建的结构。束流在腔里的能量相对变化大、速度变化明显，这时薄透镜和恒定速度近似都只能作粗略估计，使用真实场图做 t-code 积分是必要的。

## 谐振腔与工作模式 {#rf-cavity-modes}

### 圆柱腔（pillbox）的 TM010 模

最简单的加速腔是半径 $R$、长度 $d$ 的理想导体圆柱腔。在柱坐标中求解波动方程并要求导体壁上切向电场为零，加速用的 TM010 模只有 $E_z$ 和 $B_\theta$ 两个分量：

$$
E_z(r,t) = E_0\,J_0\!\left(\frac{2.405\,r}{R}\right)\cos\omega t,\qquad
B_\theta(r,t) = -\frac{E_0}{c}\,J_1\!\left(\frac{2.405\,r}{R}\right)\sin\omega t
$$

其中 $2.405$ 是 $J_0$ 的第一个零点（保证 $r = R$ 处 $E_z = 0$）。由此得到谐振频率

$$
f_{010} = \frac{2.405\,c}{2\pi R}
$$

它只与半径有关，与腔长无关。$B_\theta$ 的形式可以直接从 Faraday 定律 $\nabla\times\boldsymbol{E} = -\partial\boldsymbol{B}/\partial t$ 得到：$(\nabla\times\boldsymbol{E})_\theta = -\partial E_z/\partial r = E_0 k J_1(kr)\cos\omega t$（$k = 2.405/R$，用了 $J_0' = -J_1$），对时间积分后磁场随 $\sin\omega t$ 变化，与电场相差 $90^\circ$。**这正是 AVAS 对射频电场用 cos、对射频磁场用 sin 的物理来源**（见 [在 AVAS 中](#rf-cavity-in-avas)）。

代入 $f = 162.5\ \mathrm{MHz}$ 得 $R = 2.405c/(2\pi f) \approx 0.706\ \mathrm{m}$，即直径约 1.4 m 的腔——对低能质子而言既笨重又低效。因此低频、低 $\beta$ 的加速结构采用 TEM 型同轴谐振器。

### 四分之一波长与半波长谐振器

- **四分之一波长谐振器（QWR）**：一端短路、一端开路的同轴线，内导体长约 $\lambda/4$。束流横穿内导体开路端附近的漂移管，形成两个加速间隙。结构上下不对称，场中含有偶极分量，会对束流产生与相位相关的垂直偏转。
- **半波长谐振器（HWR）**：两端短路的同轴线，内导体长约 $\lambda/2$（162.5 MHz 时 $\lambda/2 \approx 0.92\ \mathrm{m}$），束流从中部穿过。HWR 关于束流轴上下对称，没有 QWR 那样的偶极偏转，但横截面内场不完全轴对称，含有四极成分。
- **多间隙结构**：spoke 腔、椭球腔等有三个及以上间隙。对 AVAS 而言它们都只是"一张场图"，物理差别全部包含在场分布里。

在两间隙腔中，同一时刻两个间隙的电场方向相反。粒子从第一个间隙中心走到第二个间隙中心若恰好用去半个射频周期，两个间隙对它都起加速作用，对应的间隙中心距离为 $\beta_g\lambda/2$，$\beta_g$ 称为腔的**几何 β**。

### 示例腔 hwr010

示例场图的轴上 $E_z$ 在 $z \approx 0.074\ \mathrm{m}$ 和 $z \approx 0.140\ \mathrm{m}$ 处各有一个峰，符号相反，过零点在 $z \approx 0.104\ \mathrm{m}$（腔的中心）：这是典型的两间隙 HWR。按恒定速度计算的渡越时间因子在 $\beta \approx 0.10$ 处最大（见 [能量增益](#rf-cavity-energy-gain)），与名称 hwr010 一致；而示例束流的 $\beta = 0.057$，远低于几何 β，这是本章例子中能量增益偏低、相位效应明显的原因。

```avas-field
map hwr010
```

## 品质因数、分路阻抗与峰值场 {#rf-cavity-figures}

这些量决定一个腔"花多少功率、能加多少电压、场强上限在哪里"，属于腔体设计范畴；AVAS 的束流动力学计算不直接用到它们，但选择 Ke 时必须心中有数。

| 量 | 定义 | 含义与典型量级 |
|---|---|---|
| 固有品质因数 $Q_0$ | $Q_0 = \omega U / P_c$ | 储能 $U$ 与每弧度损耗之比；常温铜腔约 $10^4$，超导铌腔 $10^9$–$10^{10}$ |
| 分路阻抗 $R_{sh}$ | $R_{sh} = V^2 / P_c$ | 单位壁损耗功率得到的电压；直线加速器常用此约定，电路约定差 2 倍 |
| $R/Q$ | $R/Q = V^2/(\omega U)$ | 只取决于几何形状，与材料无关；低 β 超导腔为几十到几百 Ω |
| 加速梯度 $E_{acc}$ | $E_{acc} = V_{acc}/L_{eff}$ | 超导腔常取 $L_{eff} = N_{gap}\,\beta_g\lambda/2$ |
| $E_{pk}/E_{acc}$、$B_{pk}/E_{acc}$ | 表面峰值场与梯度之比 | 低 β 腔典型约 4–6 与 5–10 mT/(MV/m)；限制场致发射与失超 |

这里 $V$ 一般指包含渡越时间因子的有效电压 $V_0T$。由定义可知壁损耗 $P_c = V^2/[(R/Q)\,Q_0]$：在 $R/Q$ 给定时，超导腔的极高 $Q_0$ 让壁损耗降到瓦量级。束流从腔中取走的功率则是 $P_b = I\,V_0T\cos\varphi_s$；示例束流 $I = 0.184\ \mathrm{mA}$、$\Delta W = 0.34\ \mathrm{MeV}$，故 $P_b \approx 63\ \mathrm{W}$。

> **AVAS 约定**：AVAS 中腔场幅度由 Ke（与 Kb）直接给定，是固定值。内核不模拟功率源、束流负载、失谐和低电平控制；场幅与相位的抖动通过误差命令 `err_cav_*` 引入，参见 [误差与校正](#errors)。

> **注意**：场图通常只覆盖束流孔径附近（示例只到 $\lvert x\rvert, \lvert y\rvert \le 2\ \mathrm{cm}$），不包含腔表面，因此无法从 AVAS 场图读出 $E_{pk}$；要用腔设计报告给出的 $E_{pk}/E_{acc}$ 检查所用 Ke 是否在可运行范围内。

## 能量增益与渡越时间因子 {#rf-cavity-energy-gain}

### 一般表达式

设参考粒子在 $t_{in}$ 时刻到达腔入口，沿轴走过 $z$ 时的时刻为 $t(z)$。按 AVAS 的约定，轴上电场为 $K_e E_z(z)\cos\varphi(z)$，其中

$$
\varphi(z) = \omega\,\bigl(t(z) - t_{in}\bigr) + \varphi_0
$$

$\varphi_0$ 是粒子到达入口时的射频相位。能量增益是电场沿实际轨迹的积分：

$$
\Delta W = q K_e\int_0^L E_z(z)\cos\varphi(z)\,\dd z
$$

定义两个积分 $C = \int E_z\cos\varphi\,\dd z$、$S = \int E_z\sin\varphi\,\dd z$，并令

$$
\varphi_s = \operatorname{atan2}(S,\,C),\qquad V_0T = K_e\sqrt{C^2+S^2}
$$

就有严格的关系 $\Delta W = qV_0T\cos\varphi_s$。这里的 $\varphi_s$ 正是 AVAS 中 V3 = 0 时所说的**同步相位**：它度量参考粒子相对于"这条轨迹上的最大加速相位"的偏离。

### 恒定速度近似与渡越时间因子

若粒子在腔内速度近似不变，$\varphi(z) = 2\pi z/(\beta\lambda) + \varphi_0$。取腔中心为原点，对对称（或反对称）的场分布可得经典形式

$$
\Delta W = qV_0T(\beta)\cos\varphi_s,\qquad
V_0 = \int\lvert E_z\rvert\,\dd z,\qquad
T(\beta) = \frac{\left\lvert\int E_z(z)\,\ee^{\,i\,2\pi z/(\beta\lambda)}\,\dd z\right\rvert}{\int\lvert E_z\rvert\,\dd z}
$$

几个常用结果：

- 长 $g$ 的均匀场单间隙：$T = \dfrac{\sin(\pi g/\beta\lambda)}{\pi g/\beta\lambda}$。
- 两个反号、中心距 $d$ 的相同间隙（QWR/HWR）：$T(\beta) = T_g(\beta)\,\left\lvert\sin\dfrac{\pi d}{\beta\lambda}\right\rvert$，其中 $T_g$ 是单间隙的因子。$d = \beta\lambda/2$ 时第二个因子为 1，这就是几何 β 的含义。

对示例场图 hwr010（$K_e = 1.36$），$V_0 = K_e\int\lvert E_z\rvert\,\dd z \approx 0.98\ \mathrm{MV}$；恒定速度的 $T(\beta)$ 为：

| β | 0.05 | 0.057（注入） | 0.07 | 0.08 | 0.10 | 0.104（最大） | 0.12 |
|---|---|---|---|---|---|---|---|
| $T$ | 0.21 | 0.40 | 0.65 | 0.75 | 0.82 | 0.82 | 0.80 |

$T(\beta)$ 在低于几何 β 一侧下降很快，所以腔的速度接受范围有限，一个腔型只能覆盖一段 β 区间。

### 低能时的偏差

示例中束流在腔内动能从 1.527 MeV 升到约 1.87 MeV（$\beta$ 从 0.057 变为 0.063），恒定速度近似明显失效。线性预览沿真实轨迹积分的结果是：

| 设定 | 入口相位 $\varphi_0$ | $\Delta W$ / MeV | 由此得到的 $V_0T$ / MV |
|---|---|---|---|
| V3 = 0，$\varphi_s = 0^\circ$ | $95.4^\circ$ | 0.475 | 0.475 |
| V3 = 0，$\varphi_s = -33^\circ$ | $52.7^\circ$ | 0.340 | 0.405 |
| V3 = 1 扫描的最大值 | $\approx 105.5^\circ$ | 0.482 | — |

可见 $V_0T$ 本身随相位变化：不同相位下粒子在第一间隙获得的能量不同，进入第二间隙的速度和时刻也就不同。最大能量增益并不在 $\varphi_s = 0$，而是在入口相位再晚约 $10^\circ$ 处。在 $\varphi_s = -33^\circ$ 时，粒子在第一间隙前半段先被减速（$z = 0.05\ \mathrm{m}$ 处动能约 1.44 MeV），之后才被加速——这是 $\beta \ll \beta_g$ 的典型表现。

## 运动方程与传输矩阵 {#rf-cavity-transverse}

### 近轴场

在轴附近把场按 $r$ 展开。由 $\nabla\cdot\boldsymbol{E} = 0$ 与 $\nabla\times\boldsymbol{B} = c^{-2}\,\partial\boldsymbol{E}/\partial t$，对轴对称的间隙有

$$
E_r \approx -\frac{r}{2}\,\frac{\partial E_z}{\partial z},\qquad
B_\theta \approx \frac{r}{2c^2}\,\frac{\partial E_z}{\partial t}
$$

粒子受到的径向力 $F_r = q(E_r - v B_\theta)$。在间隙入口 $\partial E_z/\partial z > 0$，电场把粒子推向轴（聚焦）；出口处相反（散焦）。若场随时间不变且速度不变，两者恰好抵消；实际上场在变化、粒子在加速，净效果与相位有关。

### 薄间隙模型

把一个间隙视为位于中心的薄透镜，对径向力在间隙内积分（Wangler 第 7 章），得到

$$
\Delta(\beta\gamma\,x') = -\frac{\pi\,q E_0TL\,\sin\varphi}{mc^2\,\lambda\,\beta^2\gamma^2}\;x
$$

$y$ 方向相同。式中 $qE_0TL$ 是该间隙的能量增益幅度（单位 MeV，$q$ 以 $e$ 计），$\beta\gamma$ 取间隙中心的值。在 AVAS 的余弦约定下，聚束相位 $\varphi < 0$ 使 $\sin\varphi < 0$，右边与 $x$ 同号——**横向散焦**。

同一间隙的纵向作用：设 $z > 0$ 表示粒子在参考粒子之前（先到达，相位较小），$\Delta\varphi = -2\pi z/(\beta\lambda)$，于是

$$
\Delta\delta = \frac{2\pi\,qE_0TL\,\sin\varphi}{mc^2\,\lambda\,\beta^3\gamma}\;z
$$

$\varphi < 0$ 时提前到达的粒子 $\delta$ 减小——**纵向聚焦**。把能量增益带来的动量增大也计入，在 $(x, x')$ 平面薄间隙矩阵为

$$
M_x = \begin{pmatrix} 1 & 0 \\[2pt] -\dfrac{\pi\,qE_0TL\sin\varphi}{mc^2\lambda\,\bar\beta^2\bar\gamma^2\,(\beta\gamma)_{out}} & \dfrac{(\beta\gamma)_{in}}{(\beta\gamma)_{out}} \end{pmatrix}
$$

右下角的比值就是**绝热阻尼**：$x'$ 随动量增大而减小，几何发射度按 $1/(\beta\gamma)$ 缩小，归一化发射度不变。

### 横纵耦合的约束

对周期排列、单位长度平均的情形，纵向与射频横向聚焦常数（Wangler 的记号）为

$$
k_{l0}^2 = \frac{2\pi\,qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3},\qquad
k_{t,\mathrm{rf}}^2 = -\frac{\pi\,qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3} = -\frac{k_{l0}^2}{2}
$$

纵向稳定（$k_{l0}^2 > 0$）就必然横向散焦，二者同时正比于 $\sin(-\varphi_s)$ 与 $1/(\beta\gamma)^3$。因此低能段 $\lvert\varphi_s\rvert$ 越大，聚束越强，所需的横向聚焦也越强；这也是低能段频率较低、相位通常在 $-30^\circ$ 左右取折中的原因之一，详见 [纵向运动](#longitudinal) 与 [结构设计](#lattice-design)。

### 示例的量级

对示例：每个间隙取 $qE_0TL \approx 0.20\ \mathrm{MeV}$（$\varphi_s = -33^\circ$ 时两间隙合计 0.405 MeV 的一半），$\beta\gamma = 0.0571$，$\lambda = 1.845\ \mathrm{m}$，由薄间隙公式得每个间隙 $1/f \approx 1.1\ \mathrm{m^{-1}}$，整个腔约 $2\ \mathrm{m^{-1}}$（散焦焦距约 0.5 m）。这只是估计；线性预览给出的 rms 尺寸见 [可调参数](#rf-cavity-parameters)。

> **AVAS 约定**：内核不使用上述薄间隙矩阵，而是在场图给出的 $\boldsymbol{E}(x,y,z)$、$\boldsymbol{B}(x,y,z)$ 中直接积分 Lorentz 力（t-code），非线性、横纵耦合与 HWR 的非轴对称成分都自然包含在内。线性预览则把场图切成 4 mm 的薄片，用轴附近的场梯度和线性化的射频聚束近似。

## 在 AVAS 中 {#rf-cavity-in-avas}

射频腔用场图元件 `field` 描述，场类型写 1：

```text
field  L  R  V3  1  f  phase  Ke  Kb  file
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
```

```avas-params
field
```

逐项说明：

- `L`（m）：元件长度，应与场图头部记录的长度一致（示例 0.21 m）。`R`（m）：孔径半径，粒子超出即判为损失。
- `V3`：决定 `phase` 的含义（见下）。
- `f`（Hz）：该腔的射频频率，示例写 `162.5e6`。束流（束团重复）频率另在 beam.txt 的 `frequency` 中给出，两者分开设置。
- `phase`（度）：含义由 V3 决定。
- `Ke`、`Kb`：电场与磁场的缩放系数；`file`：场图文件名（不含扩展名），射频腔读取 `.edx/.edy/.edz` 与 `.bdx/.bdy/.bdz`，格式见 [场图](#field-maps)。

> **AVAS 约定（已用内核核实）**：场图中的电场单位为 MV/m，乘以 Ke；磁场单位为 T，乘以 Kb。射频电场按 $K_e\,\boldsymbol{E}(\boldsymbol{r})\cos(\omega t+\varphi_0)$ 作用，射频磁场按 $K_b\,\boldsymbol{B}(\boldsymbol{r})\sin(\omega t+\varphi_0)$ 作用，与 [pillbox 模式](#rf-cavity-modes) 中 E 与 B 相差 $90^\circ$ 一致。

### V3：相位参考

| V3 | `phase` 的含义 | 适用场合 |
|---|---|---|
| 0 | 同步相位 $\varphi_s = \operatorname{atan2}(\int E\sin\varphi\,\dd z,\ \int E\cos\varphi\,\dd z)$，内核反解入口相位 | 设计阶段：直接指定 $\varphi_s$，上游改动后仍保持该同步相位 |
| 1 | 参考粒子到达元件入口时的射频相位 $\varphi_0$ | 扫相、复现某个入口相位 |
| 2 | $t = 0$ 时刻的绝对射频相位，入口相位 $\varphi_0 = \varphi + 360^\circ\cdot f\cdot t_{in}$ | 模拟真实机器：相位锁定在主时钟上，上游能量变化会改变到达时间和实际 $\varphi_s$ |

内核在 `synData.txt` 中记录每个元件入口处参考粒子的时间、能量以及 $\varphi_s$ 与 $\varphi_{RF}$；设定其中一种时会算出另一种。分段运行把中段单独拿出来时，V3 = 2 的相位要按 $\varphi_{RF} - 360^\circ\cdot f\cdot(t_{in} - T_{entry})$ 重新换算，AVAS 的分段运行会自动处理。

### Ke 与 Kb

- **Ke 是场幅旋钮**。线性预览中 $\Delta W$ 与 Ke 近似成正比但不严格（低能时轨迹随场幅变化）：示例在 $\varphi_s = -33^\circ$ 时 Ke = 0.5、1.0、1.36、2.0 对应 $\Delta W$ = 0.122、0.248、0.340、0.502 MeV。
- **Kb 应与 Ke 同比例变化**。腔的电场与磁场是同一个本征模的两部分，只改 Ke 不改 Kb 在物理上不自洽。示例取 $K_b = -K_e$：负号的来历手册没有说明，最可能与场图导出时磁场所取的相位/符号约定有关（pillbox 模中 $B_\theta \propto -\sin\omega t$），用来使导出的 $\boldsymbol{B}$ 与 AVAS 的 sin 约定相符。换用其他来源的场图时，应按该场图的导出约定确定 Kb 的符号，并与 Ke 保持固定比值。
- 对低 β 质子，射频磁场的作用远小于电场：力之比约为 $\beta cB/E$。示例中把 Kb 在 0 与 $\pm 1.36$ 之间切换，线性预览的出口 rms 尺寸变化小于 0.4 %。速度越高，磁场项越重要。

> **注意**：`scanphase`（input.txt）也会影响相位：0 表示按 lattice 中的相位，1 让内核扫描腔相位，2 从 `scanData.txt` 读取每个腔的入口相位和入口时间（每行一个腔，顺序与 lattice 中射频腔顺序相同）。线性预览不处理 `scanphase`。

## 可调参数与典型取值 {#rf-cavity-parameters}

| 参数 | 作用 | 示例值 / 典型范围 |
|---|---|---|
| `phase`（V3 = 0） | 同步相位：在加速、纵向接受度与横向散焦之间折中 | 示例 $-33^\circ$；低能段常取 $-40^\circ$ 到 $-20^\circ$，高能段可更靠近波峰 |
| `Ke` | 电场幅度 | 示例 1.36（$V_0 \approx 0.98\ \mathrm{MV}$）；上限由 $E_{pk}$、$B_{pk}$ 决定 |
| `Kb` | 磁场幅度 | 与 Ke 同比例，示例 $-1.36$ |
| `f` | 腔频 | 示例 162.5 MHz；低 β 超导段常见 162.5、325 MHz |
| `R` | 损失判据孔径 | 示例 0.02 m，应与实际束管 / 漂移管孔径一致 |
| input.txt `steppercycle` | 每个射频周期的积分步数 | 示例 50：$\beta\lambda/50 \approx 2.1\ \mathrm{mm}$，与场图 2 mm 网格相当 |

示例 lattice（单腔，无空间电荷，线性预览）在不同同步相位下的结果：

| $\varphi_s$ | $W_{out}$ / MeV | 出口 rms $x$ / mm | 出口 rms $z$ / mm |
|---|---|---|---|
| $-60^\circ$ | 1.694 | 1.96 | 0.62 |
| $-33^\circ$ | 1.867 | 1.81 | 0.70 |
| $0^\circ$ | 2.002 | 1.50 | 1.10 |
| $+33^\circ$ | 1.951 | 1.20 | 1.74 |
| 同长度漂移段 | 1.527 | 1.41 | 1.48 |

入口 rms 尺寸为 $x \approx 0.95\ \mathrm{mm}$、$z \approx 1.32\ \mathrm{mm}$，初始束流是发散的。趋势与 [薄间隙模型](#rf-cavity-transverse) 一致：$\varphi_s$ 越负，纵向越压缩、横向越散焦；$\varphi_s > 0$ 时横向聚焦而纵向散束。示例流强 0.184 mA 时，线性预览中空间电荷使单腔出口的 rms 尺寸变化约 0.3–0.4 %。

## 相位的选择与扫相 {#rf-cavity-phase}

### 设计时如何选相位

1. 由 [纵向运动](#longitudinal) 的分析确定各腔的 $\varphi_s$：纵向接受度（相位宽度约 $3\lvert\varphi_s\rvert$）要容纳束团，同时纵向与横向相移要避开参数共振，参见 [结构设计](#lattice-design)。
2. 设计阶段用 V3 = 0 直接写 $\varphi_s$。这样修改上游元件时内核会重新反解入口相位，各腔保持设定的同步相位。
3. 定稿后从 `synData.txt` 读出各腔的入口 RF 相位，必要时改成 V3 = 1 或 V3 = 2，用来研究相位误差或复现机器设定。

### 扫相

真实机器中腔的相位要通过扫相确定：逐步改变腔相位，用下游束流位置监测器的飞行时间测量出口能量，得到 $W_{out}(\varphi_0)$ 曲线，找到最大能量对应的相位（"波峰"），再按设计 $\varphi_s$ 设定偏移。在 AVAS 中先把腔设为 V3 = 1，然后在运行页使用参数扫描，或用命令行逐点扫描入口相位：

```text
avas scan --input InputFile --target 2 --param phase --values -180:170:36
```

`--target 2` 指 lattice 文件的第 2 行（示例中腔所在的行），也可以写元件名；`start:stop:count` 给出 36 个间隔 $10^\circ$ 的点；结果指标中的 `energy_out` 即出口能量。得到曲线后：

1. 找出最大值。示例中最大值约在 $\varphi_0 \approx 105.5^\circ$，$\Delta W_{max} \approx 0.482\ \mathrm{MeV}$；最小值约在 $\varphi_0 \approx -105^\circ$，$\Delta W \approx -0.31\ \mathrm{MeV}$，曲线并不对称。
2. 注意"相对波峰 $-33^\circ$"与 AVAS 的 $\varphi_s = -33^\circ$ 在低能时并不相同：示例中 $\varphi_s = 0$ 对应的入口相位 $95.4^\circ$ 比最大能量的入口相位早约 $10^\circ$。这是 [低能偏差](#rf-cavity-energy-gain) 的直接后果。
3. 当束流 β 接近几何 β 时两者重合：把注入能量改为 4.727 MeV（$\beta = 0.10$），扫相曲线基本对称，最大 $\Delta W \approx 0.803\ \mathrm{MeV}$ 出现在 $\varphi_s \approx 0$，$\varphi_s = -33^\circ$ 时 $\Delta W \approx 0.673\ \mathrm{MeV}$，正好等于 $0.803\cos 33^\circ$。

下面的实验就是这样一条扫相曲线：

```avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
```

## 动力学效应与常见问题 {#rf-cavity-pitfalls}

- **β 失配**：束流 β 明显低于腔的几何 β 时，$T$ 很小且第一间隙可能先减速。示例注入 $\beta = 0.057$、几何 β 约 0.10，单腔只获得 $V_0$ 的约 35 %。
- **只改 Ke 不改 Kb**：电磁场不再自洽。低 β 时影响很小，高 β 时可能产生虚假的横向作用。
- **相位参考混淆**：V3 = 0、1、2 的同一个数字含义完全不同。从别的程序（如 TraceWin）导入相位时先确认对方的相位定义和余弦/正弦约定。
- **相位符号约定**：AVAS 用 $\cos(\omega t+\varphi)$，聚束相位为负值。若参考书用 $\sin$ 约定，同步相位要相应换算。
- **纵向非线性**：束团相位宽度与 $\lvert\varphi_s\rvert$ 可比时，粒子会落到分界线外，纵向发射度增长或纵向损失，线性预览无法反映，必须用多粒子内核计算；可用 input.txt 的 `longlimits` 设定纵向损失边界。
- **场图噪声**：示例场图本身有噪声（轴上 $E_z$ 逐点起伏明显）。这是数据本身的特点，不是读取错误；参见 [场图噪声](#field-maps-interpolation)。
- **叠加场**：腔与螺线管边缘场重叠时用 `superpose`，一组叠加场中最多一个射频腔，参见 [叠加场](#superpose)。
- **第一个和最后一个元件**：多粒子计算中首末元件应避免使用矩阵模型元件；射频腔本身是场图元件，没有这一限制。

## 动手实验 {#rf-cavity-lab}

1. 在上面的扫相实验中把 `phase` 从 $-90^\circ$ 拖到 $0^\circ$，观察出口能量、rms $z$ 与 rms $x/y$ 的此消彼长，并与 [可调参数](#rf-cavity-parameters) 中的表对照。
2. 把 Ke 从 1.36 降到 0.7，观察同步点在扫相曲线上的位置和 $W(z)$ 曲线的变化。
3. 把注入能量 W 提高到约 4.7 MeV（$\beta \approx 0.10$），观察扫相曲线变得对称、最大能量增益接近 $V_0T_{max} \approx 0.80\ \mathrm{MeV}$（线性预览：0.803 MeV）。
4. 在场图查看器中分别查看 `edz`（轴上两个反号的间隙）与 `edx`、`edy` 的 $x$–$y$ 横截面：间隙入口与出口处横向电场方向相反，对应聚焦和散焦两半。

```avas-field
map hwr010
```

## 延伸阅读 {#rf-cavity-reading}

- T. P. Wangler, *RF Linear Accelerators*（第 2 版），第 2–5 章（谐振腔、渡越时间因子、分路阻抗）与第 6–7 章（纵向与横向动力学，薄间隙公式）。
- H. Padamsee, J. Knobloch, T. Hays, *RF Superconductivity for Accelerators*：超导腔的 $Q_0$、$R/Q$、几何因子与峰值场。
- CAS – CERN Accelerator School 讲义中关于射频腔与直线加速器的章节。
- 本手册：[纵向运动](#longitudinal)、[场图](#field-maps)、[叠加场](#superpose)、[误差与校正](#errors)、[教程](#tutorial)、[参考文献](#references)。
