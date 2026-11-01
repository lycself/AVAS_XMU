# 静电与静磁场图元件

除了射频腔，`field` 元件还可以描述不随时间变化的场：场类型 2 为**静电场**（单透镜、静电四极、静电偏转板等），场类型 3 为**静磁场**（用场图描述的螺线管、四极铁、带偏转成分的磁铁等）。与矩阵模型元件 `quad`、`solenoid`、`bend` 相比，场图元件保留了真实的纵向分布、边缘场和高阶成分，并用 t-code 在场中直接积分粒子运动。AVAS 没有静电透镜的矩阵模型元件，静电聚焦元件只能用类型 2 的场图描述（或用电场校正铁 `steerer` 的类型 1 描述纯偏转）。

本章的数值例子使用示例项目的质子束（$W = 1.527\ \mathrm{MeV}$，$\beta = 0.0570$，$\beta\gamma = 0.0571$，$p = 53.55\ \mathrm{MeV}/c$，$B\rho = 0.1786\ \mathrm{T\,m}$）和螺线管场图 `sol`（长 0.35 m）。

## 作用与位置 {#static-fields-role}

- **低能束流输运**：能量只有几十 keV 到几百 keV 的区段常用静电单透镜和静电四极：电场力 $qE$ 与速度无关，而磁场力 $qvB$ 随速度降低而减小，低速时同样技术难度的电场更有效（见下节），而且没有磁滞。
- **超导直线加速器的横向聚焦**：低 β 段腔与腔之间的超导螺线管场强高、边缘场长，常和相邻腔的场相互重叠，用场图比硬边模型可靠，参见 [螺线管](#solenoid)、[叠加场](#superpose)。
- **组合功能元件**：同一个磁体中既有四极又有偏转或校正线圈，或磁铁有明显的高阶场，用场图可以一并描述。
- **核对矩阵模型**：用场图计算与硬边矩阵 [四极铁](#quadrupole)、[螺线管](#solenoid) 的结果对比，可以判断硬边近似的误差有多大。

## 静电场：能量变化与电刚度 {#static-fields-electric}

### 能量随电势变化

静电场是保守场，$\boldsymbol{E} = -\nabla\Phi$。粒子在静电元件中的总能量守恒：

$$
W(\boldsymbol{r}) + q\,\Phi(\boldsymbol{r}) = \text{常数}
$$

所以粒子在电极之间会被加速或减速，离开元件回到原电势时动能恢复。与磁场不同，静电元件内部的速度、动量和聚焦强度都随位置变化——这正是必须用场图积分、而不能简单套用恒定能量矩阵的原因。AVAS 的线性预览也把类型 2 场图的轴上 $E_z$ 计入参考粒子的能量。

### 电刚度

磁场中的偏转能力用磁刚度 $B\rho = p/q$ 衡量；静电场中对应的是**电刚度**

$$
E\rho = \frac{p\,v}{q} = \frac{\beta^2\gamma\,mc^2}{q}\ \xrightarrow{\ \beta\ll 1\ }\ \frac{2W}{q}
$$

对示例质子 $E\rho = 53.55\ \mathrm{MeV}/c\times 0.0570 \approx 3.05\ \mathrm{MV}$（非相对论近似 $2W/q = 3.054\ \mathrm{MV}$）。横向电场 $E$ 产生的曲率半径为 $\rho = E\rho/E$，例如 $E = 1\ \mathrm{MV/m}$ 时 $\rho \approx 3.05\ \mathrm{m}$。

电场与磁场的等效关系是 $E \leftrightarrow vB$：在 $\beta = 0.057$ 时，1 T 的磁场相当于 $\beta c\times 1\ \mathrm{T} \approx 17\ \mathrm{MV/m}$ 的电场。速度越低，同样的电场越"值钱"，所以静电元件主要用在低能段。

### 静电四极

理想静电四极的四个双曲面电极分别加 $\pm V$，口径半径为 $a$，电势为

$$
\Phi = \frac{V}{a^2}\,(x^2 - y^2),\qquad E_x = -\frac{2V}{a^2}\,x,\qquad E_y = +\frac{2V}{a^2}\,y
$$

定义电场梯度 $G_E = 2V/a^2$（V/m²），则线性运动方程为

$$
x'' + k\,x = 0,\quad y'' - k\,y = 0,\qquad k = \frac{G_E}{E\rho}
$$

$V > 0$ 时正粒子在 $x$ 方向聚焦、$y$ 方向散焦，与磁四极的传输矩阵形式完全相同（见 [四极铁](#quadrupole)）。例如 $V = \pm 10\ \mathrm{kV}$、$a = 15\ \mathrm{mm}$：$G_E = 8.9\times 10^7\ \mathrm{V/m^2}$，对示例质子 $k \approx 29\ \mathrm{m^{-2}}$，相当于梯度 $kB\rho \approx 5.2\ \mathrm{T/m}$ 的磁四极。

### 单透镜（Einzel lens）

单透镜由三个同轴圆筒电极组成，两端电极接地、中间电极加电压，入口与出口动能相同。在非相对论、轴对称、近轴条件下，令 $\hat\Phi(z)$ 为以"动能为零"为参考的轴上电势（$q\hat\Phi = W$），粒子轨迹满足近轴射线方程

$$
r'' + \frac{\hat\Phi'}{2\hat\Phi}\,r' + \frac{\hat\Phi''}{4\hat\Phi}\,r = 0
$$

作 Picht 变换 $R = r\,\hat\Phi^{1/4}$ 得到

$$
R'' + \frac{3}{16}\left(\frac{\hat\Phi'}{\hat\Phi}\right)^{2}R = 0
$$

右边系数恒为正，所以**无论中间电极加正电压还是负电压，单透镜总是聚焦的**。弱透镜近似下焦距为

$$
\frac{1}{f} \approx \frac{3}{16}\int\left(\frac{\hat\Phi'}{\hat\Phi}\right)^{2}\dd z
$$

减速模式（中间电极电势使粒子减速）在相同电压下聚焦更强，但若 $q\Phi$ 接近动能，粒子会在透镜中停下或被反射。相对论修正在本手册涉及的能量下很小，完整形式见 Reiser 第 3 章。

### 静电偏转

圆柱或球面电极之间的径向电场可以让束流转弯，半径 $\rho = E\rho/E$。静电偏转在偏转平面内的聚焦性质与磁偏转不同（与电极形状有关），而且偏转中粒子动能会随电势变化。AVAS 的二极铁 `bend` 只描述磁偏转；静电偏转只能用场图描述，见 [注意事项](#static-fields-pitfalls)。

## 静磁场：螺线管、四极与偏转成分 {#static-fields-magnetic}

### 螺线管场图

轴对称磁场在轴附近由轴上 $B_z(z)$ 唯一确定（$\nabla\cdot\boldsymbol{B} = 0$、$\nabla\times\boldsymbol{B} = 0$）：

$$
B_r(r,z) = -\frac{r}{2}\,B_z'(z) + \frac{r^3}{16}\,B_z'''(z) - \cdots,\qquad
B_z(r,z) = B_z(z) - \frac{r^2}{4}\,B_z''(z) + \cdots
$$

在入口端 $B_z' > 0$，$B_r$ 指向轴心；出口端 $B_z' < 0$，$B_r$ 背离轴心（对 $B_z > 0$）。粒子穿过入口边缘时由 $F_\theta = q v_z B_r$ 获得角向速度，进入螺线管主体后 $F_r = q v_\theta B_z$ 把它拉向轴心，这就是螺线管聚焦。由 Busch 定理，在旋转坐标系（Larmor 坐标系）中两个横向平面解耦，满足

$$
r'' + \left(\frac{B_z(z)}{2B\rho}\right)^{2} r = 0,\qquad
\theta_L = \int\frac{B_z}{2B\rho}\,\dd z,\qquad
\frac{1}{f} \approx \int\left(\frac{B_z}{2B\rho}\right)^{2}\dd z
$$

聚焦强度正比于 $B_z^2$，**与场的方向无关**；改变 $K_b$ 的符号只改变束流旋转方向。用示例 lattice 验证：螺线管场图前后各接漂移段，$K_b = \pm 0.531$ 时线性预览得到的 rms 尺寸完全相同。

### 示例螺线管 sol

场图 `sol` 长 0.35 m，轴上 $B_z$ 在中心 $z \approx 0.175\ \mathrm{m}$ 处最大，两端降到峰值的约 0.3 %。取手册叠加场示例中的 $K_b = 0.531$：

| 量 | 数值 | 说明 |
|---|---|---|
| $\int B_z\,\dd z$ | $0.387\ \mathrm{T\,m}$ | 对噪声不敏感 |
| Larmor 旋转角 $\theta_L$ | $62^\circ$ | $\int B_z\,\dd z/(2B\rho)$ |
| $\int B_z^2\,\dd z$ | $1.10$–$1.14\ \mathrm{T^2m}$ | 噪声使原始数据偏大约 3 %，前者为 9 点平滑后的值 |
| 薄透镜焦距 $f$ | $\approx 0.115\ \mathrm{m}$ | 与长度相当，薄透镜近似只是量级估计 |
| 等效硬边 $B_{eff} = \int B^2/\int B$ | $\approx 2.85\ \mathrm{T}$ | |
| 等效长度 $L_{eff} = (\int B)^2/\int B^2$ | $\approx 0.136\ \mathrm{m}$ | 远短于场图长度 |
| 平滑后峰值场 | $\approx 4.4\ \mathrm{T}$ | 原始数据逐点跳动，最大值不可直接当作峰值 |

场图横截面在 $r = 1\ \mathrm{cm}$ 处的径向场（场图原值）：入口侧 $z = 0.059\ \mathrm{m}$ 处 $B_x(x{=}1\,\mathrm{cm}) \approx -0.053\ \mathrm{T}$，指向轴心；出口侧 $z = 0.29\ \mathrm{m}$ 处约 $+0.052\ \mathrm{T}$，背离轴心；量级与 $-\tfrac{r}{2}B_z'$ 相符。

### 四极场图

磁四极在轴附近 $B_y = G\,x$、$B_x = G\,y$，$k = G/(B\rho)$；正粒子在 $k > 0$ 时 $x$ 方向聚焦。场图四极一般按某个参考梯度（例如中心 1 T/m）归一化导出，此时 $K_b$ 就是实际梯度（T/m），**$K_b$ 的符号决定极性**：$K_b < 0$ 时聚焦和散焦平面互换。有边缘场时常用等效长度 $L_{eff} = \int G(z)\,\dd z/G_0$ 与硬边四极比较。

### 偏转与校正成分

场图中若含有轴上横向磁场（偶极成分），粒子会被偏转。内核按场图积分，偏转自然计入；线性预览不计偏转，当估计偏角超过 1 mrad 时给出警告。场图在空间中是一个沿 $z$ 轴的直长方体，粒子偏离轴线后会逐渐走出场图覆盖的区域，因此场图适合描述小角度的偏转和校正；大角度二极铁请使用 `bend` 与 `edge`（见 [二极铁](#dipole)），校正线圈也可以用 [校正铁](#steerer) 描述。内核在场图中如何定义大偏转时的参考轨道，手册没有说明。

## 运动方程与传输矩阵 {#static-fields-equations}

### t-code 积分的方程

内核在场图元件中对每个粒子积分 Lorentz 方程

$$
\frac{\dd(\gamma m\boldsymbol{v})}{\dd t} = q\,\bigl(\boldsymbol{E}(\boldsymbol{r}) + \boldsymbol{v}\times\boldsymbol{B}(\boldsymbol{r})\bigr),\qquad
\boldsymbol{E} = K_e\,\boldsymbol{E}_{map},\quad \boldsymbol{B} = K_b\,\boldsymbol{B}_{map}
$$

时间步长由 input.txt 的 `steppercycle` 决定。静磁场不做功，粒子动能不变；静电场按上一节的能量守恒改变动能。

### 线性化

把场在轴附近展开到一阶，横向运动方程可以写成

$$
x'' + \frac{(\beta\gamma)'}{\beta\gamma}\,x' + \left(\frac{1}{B\rho}\frac{\partial B_y}{\partial x} - \frac{1}{E\rho}\frac{\partial E_x}{\partial x}\right)x = 0
$$

（这里没有写出螺线管的 $x$–$y$ 耦合项；有轴向磁场时用旋转坐标系或 4×4 矩阵，见 [螺线管](#solenoid)。）第二项来自静电场中动量的变化。若元件内动量变化可以忽略，把场按等效梯度 $G_{eff}$、等效长度 $L_{eff}$ 换成硬边元件，就回到矩阵模型：四极用 [四极铁](#quadrupole) 一章的厚透镜矩阵，螺线管用 $k = B_{eff}/(2B\rho)$、$C = \cos kL$、$S = \sin kL$ 的耦合矩阵。

### 硬边等效有多准

在示例中比较两个 lattice（无空间电荷，线性预览）：漂移 0.05 m + 场图 `sol`（$K_b = 0.531$）+ 漂移 0.3 m，与在相同中心位置放置 $B = 2.85\ \mathrm{T}$、$L = 0.136\ \mathrm{m}$ 的硬边 `solenoid`。两者的束腰都在 $z \approx 0.39\ \mathrm{m}$，rms 束腰分别为 0.285 mm 和 0.310 mm；到 $z = 0.7\ \mathrm{m}$ 时 rms 为 2.69 mm 和 2.43 mm，相差约 10 %。强聚焦时硬边等效只能作初步设计，精确计算应使用场图。

> **AVAS 约定**：线性预览把场图切成 4 mm 的薄片，每片做"半步螺线管主体 + 由轴附近横向梯度给出的薄透镜 + 半步"的处理，螺线管的边缘作用由梯度项给出；内核则直接在三维场中积分，二者都包含边缘场，后者还包含非线性项。

## 在 AVAS 中 {#static-fields-in-avas}

```text
field  L  R  0  2  0  0  Ke  0   file      ! 静电场：.esx .esy .esz
field  L  R  0  3  0  0  Ke  Kb  file      ! 静磁场：.bsx .bsy .bsz
field  0.1   0.02  0  3  0  0  1  0.677098  sol_1     ! 手册示例
field  0.35  0.02  0  3  0  0  1  0.531     sol       ! 示例螺线管
```

```avas-params
field
```

- 场类型写 2（静电）或 3（静磁）。
- **频率 `f` 必须写 0**；`phase` 写 0；`V3` 对静态场无意义，写 0。手册的规则是"不存在的参数写 0"。
- 静电场的电场单位为 MV/m，乘以 `Ke`；静磁场的磁场单位为 T，乘以 `Kb`。
- 对静磁场，手册示例中 `Ke` 写的是 1；场图没有电场分量文件时它没有作用对象。对静电场，按手册规则 `Kb` 写 0。
- `file` 为场图文件名（不含扩展名），静电场需要 `.esx/.esy/.esz`，静磁场需要 `.bsx/.bsy/.bsz`，格式见 [场图](#field-maps)。
- `L` 应等于场图头部的长度；`R` 是损失判据的孔径半径。

> **AVAS 约定**：静磁场图四极的 `Kb` 符号就是极性；螺线管的 `Kb` 符号只改变旋转方向。界面的元件列表根据场类型把场图元件分为射频、静电、静磁三类；静磁场图的具体类型（螺线管、四极……）由场图文件名推测，显示时带"（推测）"，识别依据写在详情说明中——这只是显示用的推测，不是物理数据。

> **注意**：静磁元件的误差用 `err_quad_*`（静磁元件误差）命令描述，射频腔用 `err_cav_*`，参见 [误差与校正](#errors)。

## 可调参数与典型取值 {#static-fields-parameters}

| 参数 | 元件 | 作用 | 示例 / 典型值 |
|---|---|---|---|
| `Kb` | 静磁螺线管 | 场强缩放，焦距 $\propto 1/K_b^2$ | 示例 0.531（$\int B\,\dd z = 0.387\ \mathrm{T\,m}$，$f \approx 0.12\ \mathrm{m}$） |
| `Kb` | 静磁四极 | 梯度与极性 | 场图按 1 T/m 归一化时 $K_b = G$；常温四极一般在几十 T/m 以内 |
| `Ke` | 静电四极、单透镜 | 电场缩放 | 场图按 1 kV 电极电压导出时 $K_e$ = 电压 / kV；低能段电极电压常为几到几十 kV |
| `L` | 全部 | 元件长度 | 等于场图头部长度 |
| `R` | 全部 | 损失孔径 | 与真实孔径一致，不大于场图横向范围 |

由于聚焦强度 $\propto K_b^2$（螺线管）或 $\propto K_b$（四极），扫描 `Kb` 可以直接用参数扫描：`avas scan --input InputFile --target 元件名 --param Kb --values …`。

## 动力学效应与常见问题 {#static-fields-pitfalls}

- **场图截断**：场图两端的场应已衰减到可以忽略。若场图在边界处截断，粒子会受到一个虚假的边缘冲击；示例 `sol` 两端已降到峰值的约 0.3 %。
- **元件长度与场图长度不一致**：内核对两者不一致时的处理，手册没有说明；线性预览只使用 $[0, \min(L, L_{map})]$ 范围内的场。务必让 `L` 与场图头部一致。
- **重叠的场**：相邻元件的边缘场重叠时，不能简单地把两个场图首尾相接（重叠部分会被截掉），要用 [叠加场](#superpose)。
- **螺线管耦合**：螺线管使 $x$、$y$ 耦合并旋转束流；非圆束经过螺线管后 rms $x$、$y$ 不再独立，四极之后接螺线管时尤其要注意。
- **静电反射**：减速型静电元件中若 $q\Phi$ 超过动能，粒子会停下或被反射，线性预览会提示"参考粒子停止"；这类粒子无法通过元件。
- **频率写成非零值**：静态场的 `f` 必须为 0；写成束流频率会让人误以为它是射频场，规则上应写 0。
- **噪声**：示例场图本身有噪声，$\int B^2$ 等平方量会被噪声系统性地抬高（示例约 3 %），线性量如 $\int B$ 基本不受影响，参见 [场图噪声](#field-maps-interpolation)。
- **大角度偏转**：不要用直的场图盒子描述大角度二极铁，见 [偏转成分](#static-fields-magnetic)。

## 动手实验 {#static-fields-lab}

**实验一：读场图。** 在场图查看器中打开示例螺线管：先看 `bsz` 的纵切面，再切到 $x$–$y$ 横截面并打开箭头，把截面位置分别移到入口端和出口端，确认横向场在入口指向轴心、在出口背离轴心，中心截面的横向场接近零。

```avas-field
map sol
```

**实验二：硬边等效。** 下图是与示例场图有相同 $\int B\,\dd z$ 和 $\int B^2\,\dd z$ 的硬边螺线管的示意场（由参数计算，硬边，不是场图数据），与上面的场图对比纵向分布的差别。

```avas-field
solenoid B=2.85 L=0.136 R=0.02
```

**实验三：聚焦与旋转。** 在螺线管实验中使用等效参数，观察 Larmor 旋转角 $kL$（约 $62^\circ$）、厚透镜与薄透镜焦距的差别，以及束腰位置随 B 的变化。再把 W 降到 0.05 MeV：薄透镜焦距按 $f \propto (B\rho)^2 \propto W$ 缩短约 30 倍，而 $kL$ 增大到约 6 rad，束流在螺线管内多次过焦，薄透镜近似完全失效——低能段要用弱得多的螺线管。

```avas-lab
solenoid L=0.136 B=2.85 W=1.527 d1=0.05 d2=0.3
```

## 延伸阅读 {#static-fields-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*，第 3 章（近轴射线方程、静电与磁透镜、螺线管与 Busch 定理）。
- H. Wollnik, *Optics of Charged Particles*：静电与磁元件的传输矩阵。
- P. W. Hawkes, E. Kasper, *Principles of Electron Optics*：单透镜与近轴光学的系统推导。
- 本手册：[螺线管](#solenoid)、[四极铁](#quadrupole)、[二极铁](#dipole)、[场图](#field-maps)、[叠加场](#superpose)、[参考文献](#references)。
