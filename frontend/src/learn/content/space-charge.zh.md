# 空间电荷

束流中的带电粒子相互排斥。对低能强流束，这种**空间电荷力**会抵消一部分外部聚焦，使束流尺寸变大、相移压低，并通过非线性力引起发射度增长和束晕。本章先求均匀束的自场，引入广义导流系数 $K$，再推导 KV 包络方程和 Sacherer 的 rms 包络方程，用平滑近似给出匹配尺寸和相移压低的解析式，最后定性讨论发射度增长与束晕，并说明 AVAS 内核和线性预览各自如何处理空间电荷。横向线性光学和 Twiss 参数见 [线性光学](#linear-optics) 与 [相空间与发射度](#phase-space)，纵向运动见 [纵向动力学](#longitudinal)。

本章的数值例子用示例项目的束流：质子，$W = 1.527\ \mathrm{MeV}$（$\beta = 0.05698$，$\gamma = 1.00163$，$\beta\gamma = 0.05708$，$B\rho = 0.1786\ \mathrm{T\,m}$），平均流强 $I = 0.184\ \mathrm{mA}$，束流频率 $f = 162.5\ \mathrm{MHz}$，横向归一化 rms 发射度 $0.138\ \pi\,\mathrm{mm\,mrad}$（几何 rms 发射度 $\varepsilon = 2.41\ \mathrm{mm\,mrad}$）。

## 均匀束的自场 {#space-charge-self-fields}

### 圆截面直流束

设半径为 $a$ 的圆柱内电荷均匀分布，线电荷密度 $\lambda_q = I/(\beta c)$。由高斯定律和安培定律，束内（$r \le a$）

$$
E_r = \frac{\lambda_q}{2\pi\varepsilon_0}\frac{r}{a^2},\qquad B_\theta = \frac{\beta}{c}E_r .
$$

运动电荷的磁场力与电场力方向相反：

$$
F_r = q\left(E_r - \beta c B_\theta\right) = q E_r\left(1 - \beta^2\right) = \frac{q E_r}{\gamma^2} .
$$

这个 $1/\gamma^2$ 抵消使空间电荷在相对论能区迅速变弱。由 $x'' = F_x/(\gamma m\beta^2c^2)$，束内粒子的运动方程是

$$
x'' = \frac{K}{a^2}\,x ,
$$

$K$ 是下一节定义的广义导流系数。力与 $x$ 成正比——**均匀分布的自场是线性的**，这是 KV 理论成立的基础。

### 椭圆截面

半轴为 $a$、$b$ 的均匀椭圆截面束内

$$
E_x = \frac{\lambda_q}{\pi\varepsilon_0}\frac{x}{a(a + b)},\qquad
E_y = \frac{\lambda_q}{\pi\varepsilon_0}\frac{y}{b(a + b)},
\qquad
x'' = \frac{2K}{a(a + b)}\,x,\quad y'' = \frac{2K}{b(a + b)}\,y .
$$

$a = b$ 时回到圆截面的结果。

### 束团：均匀椭球

射频加速的束是一串束团，每个束团电荷 $Q = I/f$（$I$ 为平均流强）。把束团看成实验室系半轴 $a$、$b$、$c_z$ 的均匀椭球；在束流静止系中纵向半轴为 $\gamma c_z$，电荷密度 $\rho' = 3Q/(4\pi a b\,\gamma c_z)$。均匀椭球内的场是线性的：

$$
E'_x = \frac{\rho'}{\varepsilon_0}M_x\,x,\quad E'_y = \frac{\rho'}{\varepsilon_0}M_y\,y,\quad E'_z = \frac{\rho'}{\varepsilon_0}M_z\,\gamma z,\qquad M_x + M_y + M_z = 1,
$$

$\gamma z$ 是静止系中的纵向坐标，$M_i$ 是退极化因子（球形时都等于 $1/3$）。对旋转椭球，记静止系长径比 $p = \gamma c_z/\sqrt{ab}$，$p > 1$ 时

$$
M_z = \frac{1 - e^2}{e^3}\left(\operatorname{artanh} e - e\right),\qquad e = \sqrt{1 - 1/p^2},
$$

常用近似 $M_z \approx 1/(3p)$（$p$ 在 1 附近到几倍之间），横向 $M_x \approx (1 - M_z)\,b/(a + b)$。变换回实验室系（横向力再乘 $1/\gamma$），得到

$$
x'' = \frac{K_3 M_x}{a b c_z}\,x,\qquad
z'' = \frac{K_3 M_z}{a b c_z}\,z,\qquad
K_3 = \frac{3 q Q}{4\pi\varepsilon_0\, mc^2\,\beta^2\gamma^3},
$$

这里 $z' = \dd z/\dd s = \delta/\gamma^2$。

## 广义导流系数 {#space-charge-perveance}

### 直流束

$$
K = \frac{q I}{2\pi\varepsilon_0\, m c^3\beta^3\gamma^3} = \frac{2I}{I_0\,\beta^3\gamma^3},\qquad
I_0 = \frac{4\pi\varepsilon_0\, m c^3}{q} .
$$

$I_0$ 是特征电流：质子 $I_0 = 3.13\times 10^7\ \mathrm{A}$，质量数 $A$、电荷态 $Z$ 的离子为 $3.13\times 10^7\,A/Z\ \mathrm{A}$。$K$ 无量纲，衡量空间电荷相对于束流"刚度"的强弱；$\beta^3\gamma^3$ 使它在低能时很大。示例能量下

| 直流流强 | $K$ |
|---|---|
| 0.184 mA | $6.32\times 10^{-8}$ |
| 1 mA | $3.44\times 10^{-7}$ |
| 10 mA | $3.44\times 10^{-6}$ |

### 束团的等效导流系数

把均匀椭球的横向方程 $x'' = K_3 M_x x/(a b c_z)$ 与直流圆束的 $x'' = K x/a^2$ 比较（取 $a = b$，$M_x = (1 - M_z)/2$），束团中心截面相当于一个直流束，其**等效流强**和导流系数为

$$
I_{\mathrm{eq}} = \frac{3}{4}\,\frac{Q\,\beta c}{c_z}\,(1 - M_z),\qquad
K_{\mathrm{eq}} = \frac{2 I_{\mathrm{eq}}}{I_0\beta^3\gamma^3} .
$$

示例束团：$Q = I/f = 1.133\ \mathrm{pC}$（$7.07\times 10^6$ 个质子），rms 尺寸 $\sigma_x = 0.954$、$\sigma_y = 0.958$、$\sigma_z = 1.316\ \mathrm{mm}$。rms 等效均匀椭球的半轴是 $\sqrt5\,\sigma$（见 [下一节](#space-charge-envelope)），静止系长径比 $p = 1.379$，$M_z = 0.252$，于是 $I_{\mathrm{eq}} = 3.69\ \mathrm{mA}$，$K_{\mathrm{eq}} = 1.27\times 10^{-6}$。

> 注意：束团的空间电荷强度由**束团内的峰值线密度**决定，而不是平均流强。示例中等效流强是平均流强的 20 倍；束团被纵向压缩时 $I_{\mathrm{eq}}$ 还会增大。直接用平均流强估计 $K$ 会严重低估空间电荷。

## KV 与 rms 包络方程 {#space-charge-envelope}

### KV 包络方程

KV（Kapchinsky–Vladimirsky）分布是四维相空间中一个超椭球面上的均匀分布，它在 $x$–$y$ 平面的投影是均匀椭圆，自场严格线性，并且在线性外场中保持 KV 分布。束流边界半轴 $a(s)$、$b(s)$ 满足

$$
a'' + k_x(s)\,a - \frac{2K}{a + b} - \frac{\varepsilon_x^2}{a^3} = 0,\qquad
b'' + k_y(s)\,b - \frac{2K}{a + b} - \frac{\varepsilon_y^2}{b^3} = 0 ,
$$

$k_{x,y}(s)$ 是外部聚焦强度（四极铁 $k = \pm G/B\rho$，见 [四极铁](#quadrupole)），$\varepsilon$ 是**边界**（总）几何发射度。三项分别代表外部聚焦、空间电荷散焦和发射度（"热压力"）项。

### Sacherer 的 rms 包络方程

Sacherer（1971）证明，对任何具有椭圆对称性的分布，rms 尺寸 $\sigma = \sqrt{\langle x^2\rangle}$ 满足形式相同的方程；对二维连续束，空间电荷项与分布形状无关，对三维椭球对称的束团只弱依赖于分布形状：

$$
\sigma_x'' + k_x\,\sigma_x - \frac{K}{2(\sigma_x + \sigma_y)} - \frac{\varepsilon_{x}^2}{\sigma_x^3} = 0 ,
$$

$\varepsilon_x$ 是 rms 几何发射度。把 $a = 2\sigma_x$、$\varepsilon_{\mathrm{edge}} = 4\varepsilon_{\mathrm{rms}}$ 代入 KV 方程即得此式。于是任意分布都可以用"rms 等效 KV 束"描述：**边界量 = 2 × rms 尺寸，边界发射度 = 4 × rms 发射度**。

### 束团的三维 rms 包络方程

对均匀椭球，$\langle x^2\rangle = a^2/5$，即 rms 等效半轴是 $\sqrt5\,\sigma$。代入上节的线性力：

$$
\sigma_x'' + k_x\sigma_x - \frac{\varepsilon_x^2}{\sigma_x^3} - \frac{K_3 M_x}{5\sqrt5\,\sigma_y\sigma_z} = 0,\qquad
\sigma_z'' + k_z\sigma_z - \frac{\varepsilon_z^2}{\sigma_z^3} - \frac{K_3 M_z}{5\sqrt5\,\sigma_x\sigma_y} = 0 ,
$$

$y$ 方向与 $x$ 类似，$k_z$ 是纵向射频聚焦（[纵向动力学](#longitudinal) 中的 $k_{l0}^2$），$\varepsilon_z$ 是 $(z, \dd z/\dd s)$ 平面的几何 rms 发射度。三个方向通过 $M_i$ 和尺寸耦合在一起。

> AVAS 约定：`twissz` 的纵向角度用 $\delta = \Delta p/p$，而上式用 $\dd z/\dd s = \delta/\gamma^2$，二者差 $\gamma^2$；示例能量下 $\gamma^2 = 1.003$，差别可以忽略，高能时要注意换算。

### 发射度主导还是空间电荷主导

比较两个散焦项 $K/(4\sigma)$（圆束）与 $\varepsilon^2/\sigma^3$：比值 $K\sigma^2/(4\varepsilon^2) \ll 1$ 时束流**发射度主导**，$\gg 1$ 时**空间电荷主导**。示例发射度、10 mA 直流束在 $\sigma = 1.33\ \mathrm{mm}$ 时比值约 0.26，仍属发射度主导，但空间电荷已明显改变匹配尺寸（见下节）。

## 相移压低与平滑近似 {#space-charge-tune-depression}

### 平滑近似

当每个聚焦周期 $L_p$ 的零流强相移 $\sigma_0$ 不太大（约 $\sigma_0 \lesssim 60°$–$80°$）时，可以把周期聚焦平均成连续聚焦 $k_0 = \sigma_0/L_p$。圆截面直流束的 rms 包络方程变为

$$
\sigma'' + k_0^2\,\sigma - \frac{K}{4\sigma} - \frac{\varepsilon^2}{\sigma^3} = 0 .
$$

### 匹配尺寸

匹配束 $\sigma'' = 0$：

$$
k_0^2\sigma^4 - \frac{K}{4}\sigma^2 - \varepsilon^2 = 0
\quad\Longrightarrow\quad
\sigma^2 = \frac{K/4 + \sqrt{K^2/16 + 4k_0^2\varepsilon^2}}{2k_0^2} .
$$

零流强时 $\sigma_{0,\mathrm{rms}} = \sqrt{\varepsilon/k_0}$。

### 压低后的相移

束内粒子受外部聚焦和线性空间电荷散焦，其波数 $k^2 = k_0^2 - K/(4\sigma^2)$。由匹配条件得 $k = \varepsilon/\sigma^2$，代入并整理：

$$
\frac{\sigma}{\sigma_0} = \frac{k}{k_0} = \sqrt{1 + u^2} - u,\qquad
u = \frac{K}{8 k_0\,\varepsilon_{\mathrm{rms}}} = \frac{K}{2 k_0\,\varepsilon_{\mathrm{edge}}} ,
$$

其中 $\varepsilon$ 都是**几何**（非归一化）发射度，$\varepsilon_{\mathrm{edge}} = 4\varepsilon_{\mathrm{rms}}$。这里 $\sigma$、$\sigma_0$ 指每周期的相移（不是 rms 尺寸），$\sigma/\sigma_0$ 称为**相移压低**（tune depression）：1 表示无空间电荷，趋于 0 表示极端空间电荷主导。设计中常见 $\sigma/\sigma_0 \gtrsim 0.4$–0.5。

### 数值例

FODO 周期：四极铁 $L_q = 0.1\ \mathrm{m}$、$G = 6\ \mathrm{T/m}$，漂移 $L_d = 0.2\ \mathrm{m}$，周期 $L_p = 0.6\ \mathrm{m}$。用 AVAS 的厚透镜矩阵算得 $\sigma_0 = 52.7°$，$k_0 = 1.534\ \mathrm{m}^{-1}$，零流强匹配 rms 尺寸 1.254 mm。

| 束流 | $K$ | $u$ | $\sigma/\sigma_0$ | $\sigma$ | 匹配 rms 尺寸 |
|---|---|---|---|---|---|
| 示例束团（$I_{\mathrm{eq}} = 3.69\ \mathrm{mA}$） | $1.27\times 10^{-6}$ | 0.043 | 0.958 | 50.5° | 1.281 mm |
| 10 mA 直流 | $3.44\times 10^{-6}$ | 0.116 | 0.891 | 47.0° | 1.329 mm |
| 30 mA 直流 | $1.03\times 10^{-5}$ | 0.348 | 0.711 | 37.5° | 1.488 mm |

同一个周期在 $G = 4\ \mathrm{T/m}$（$\sigma_0 = 34.5°$）时，30 mA 直流束的相移压低到 0.60：聚焦越弱，空间电荷的相对影响越大。

```avas-lab
space-charge I=10 W=1.527 mass=938.272 q=1 emit=0.13768 G=6 Lq=0.1 Ld=0.2 cells=10
```

> 注意：平滑近似忽略了包络在周期内的起伏，$\sigma_0$ 较大时误差增大；严格的匹配要解周期性的包络方程（见 [匹配](#matching)）。束团的等效 $K$ 还随束团长度变化，表中第一行只是入口处的估计。

## 发射度增长与束晕 {#space-charge-emittance-growth}

线性理论中 rms 发射度守恒。真实束流的分布不是 KV，自场含非线性成分，发射度会增长。主要机制（定性）：

- **电荷重新分布**：rms 匹配但分布不均匀（如高斯、带尖峰的分布）的束，会在约四分之一个等离子体振荡周期内向更均匀的分布弛豫，释放的"自由能"转化为发射度增长。圆截面均匀束的等离子体波数 $k_p^2 = 2K/a^2 = K/(2\sigma^2)$；10 mA 例子中 $k_p = 0.99\ \mathrm{m}^{-1}$，等离子体波长约 6.4 m。空间电荷越强，这一过程越快、增长越大。
- **失配与束晕**：失配束的包络振荡（圆束平滑近似下，呼吸模波数 $k_{\mathrm{b}} = \sqrt{2k_0^2 + 2k^2}$，四极模 $k_{\mathrm{q}} = \sqrt{k_0^2 + 3k^2}$）会通过 2:1 参数共振把单个粒子推到远离束芯的轨道，形成**束晕**（halo）。束晕粒子很少，几乎不改变 rms 尺寸，却决定了束损。
- **包络不稳定性与共振**：零流强相移 $\sigma_0 > 90°$ 时，空间电荷可以激发包络不稳定（因此常用 $\sigma_0 < 90°$ 的设计规则）；周期结构的其他参数共振也会引起增长。
- **横纵耦合**：横向和纵向的"温度"（发射度与聚焦之比）不相等时，空间电荷会在两者之间交换能量（能量均分问题），Hofmann 稳定图给出了危险的参数区域。

在 AVAS 输出中，可以比较 DataSet.txt 的 rms 尺寸（第 16/18/20 列）与最大尺寸（第 22/24/26 列）：最大值与 rms 之比持续增大通常是束晕发展的信号。发射度增长和束晕都要用多粒子内核计算，线性预览不包含这些效应。

## 在 AVAS 中 {#space-charge-avas}

### 关键字

空间电荷由 `input.txt` 控制，束流电荷由 `beam.txt` 给出：

```avas-params
input:spacecharge input:scmethod input:numofgrid input:meshrms input:steppercycle beam:current beam:frequency beam:beamtype beam:particlenumber spacechargecomp
```

- `spacecharge 1` 打开空间电荷，`0` 关闭。
- `current`（mA，平均流强）与 `frequency`（Hz，束流频率）决定束团电荷 $Q = I/f$；每个宏粒子的电荷为 $Q$ 除以宏粒子数 `particlenumber`。`beamtype dc` 生成直流束。
- `scmethod` 选择算法：`FFT`、`PICNIC` 或 `SPICNIC`。手册只列出名称，没有说明各算法的实现细节；示例项目用 `FFT`。改用其他算法时，应在同一问题上与 `FFT` 的结果比较。
- `numofgrid Nx Ny Nz` 是三个方向的网格数；`meshrms V1 V2 V3` 设网格边长 $L_x = 2 V_1\sigma_x$（$y$、$z$ 同理），即网格覆盖 $\pm V_1$ 倍 rms 尺寸。网格要盖住整个束团（包括尾部），同时每个网格单元里要有足够的宏粒子，见 [多粒子模拟要点](#simulation)。
- 空间电荷在每个时间步计算，步长由 `steppercycle` 决定（每个射频周期的步数）。
- 手册还说明：启用纵向周期性边界时，空间电荷算法自动切换为 FFT，纵向网格长度设为束团周期长度，因此需要更多的纵向网格点。
- lattice 关键字 `spacechargecomp`（空间电荷补偿）在手册中没有说明，参数和作用都未记录，不要依赖它。
- `spacechargelong`、`spacechargetype` 只对包络模型（`sim_type env`）起作用。

> 注意：`input.txt` 的 `boundary` 指束流**损失**边界（`boundary.txt`），不是空间电荷求解的边界条件。内核空间电荷求解器的边界条件、坐标系和插值方式手册未说明，以内核为准。

### 线性预览的空间电荷模型

可视化编辑器的线性预览在束流流强大于零时加入线性空间电荷：

- 束团：用 rms 等效的均匀椭球（半轴 $\sqrt5\,\sigma_{x,y,z}$），$M_z$ 按旋转椭球公式和静止系长径比 $p = \gamma c_z/\sqrt{ab}$ 计算，横向取 $M_x = (1 - M_z)\,b/(a + b)$，即上文的 $K_3$ 线性力；需要 `frequency` 和 `twissz`，缺少时不计空间电荷并给出提示。
- 直流束（`beamtype dc`）：用 rms 等效的均匀椭圆截面（半轴 $2\sigma$），即 KV 力 $2K/(a(a+b))$。
- 每 4 个步长施加一次积分强度的薄透镜冲量（放在这组步长的中间，二阶分裂），力随当前 rms 尺寸更新。

它只描述 rms 包络的线性部分，不含发射度增长、束晕和损失。强空间电荷时它与内核的 rms 尺寸可相差到 13 %。

## 延伸阅读 {#space-charge-further}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：自场、KV 分布、包络方程、发射度增长。
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：空间电荷多粒子动力学与束晕。
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18, 1105 (1971).
- I. M. Kapchinsky, V. V. Vladimirsky, Proc. Int. Conf. on High Energy Accelerators, CERN, 1959（KV 分布）。
- I. Hofmann, "Stability of anisotropic beams with space charge", *Phys. Rev. E* 57, 4713 (1998).
- 本入门的 [线性光学](#linear-optics)、[匹配](#matching)、[多粒子模拟要点](#simulation)。
