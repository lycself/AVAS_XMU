# 四极铁

四极铁（quadrupole）是加速器中最主要的横向聚焦元件。它的磁场在束流轴上为零、随离轴距离线性增长，因此对束流相当于一块"磁透镜"。与光学透镜不同，四极铁在一个横向平面聚焦的同时必然在另一个平面散焦；把聚焦和散焦四极铁交替排列（强聚焦，交变梯度），才能在两个平面同时得到净聚焦。

阅读本章前建议先看[漂移段](#drift)（传输矩阵的基本形式）和[线性光学](#linear-optics)（Hill 方程、相移、稳定性）。

## 作用与位置 {#quadrupole-role}

- **横向聚焦**：在直线加速器的中能、高能段，四极铁组成 FODO、双合（doublet）或三合（triplet）周期，把束流约束在孔径内。
- **匹配**：在两段结构的交界处（例如 RFQ 之后的中能传输段 MEBT、不同周期的过渡处），用若干个独立可调的四极铁把束流的 Twiss 参数匹配到下游周期解（见[匹配](#matching)）。
- **束流整形**：在靶前或束流收集器前调节束斑的大小和形状。

四极铁对低能束流同样有效，但它对 $x$、$y$ 两个平面的作用不对称，而且强聚焦需要交替排列，所需空间较大；在极低能段或超导腔串之间，常用[螺线管](#solenoid)代替。

## 物理原理与场 {#quadrupole-field}

### 极头几何与理想场

理想四极铁有四个磁极，按 N–S–N–S 交替排列在 $\pm 45^\circ$、$\pm 135^\circ$ 方向上。在无电流、无铁的孔径内部，磁场可以写成磁标势 $\Phi_m$ 的梯度 $\boldsymbol{B} = -\nabla \Phi_m$，$\Phi_m$ 满足拉普拉斯方程。最低阶的四极解为

$$
\Phi_m = -G\,x\,y,
\qquad
B_x = G\,y,\qquad B_y = G\,x,
$$

其中 $G = \partial B_y/\partial x = \partial B_x/\partial y$ 是**磁场梯度**，单位 T/m。磁极表面是等势面 $x y = \pm R^2/2$——一组双曲线，$R$ 是内切圆（孔径）半径。等价地，用矢势 $A_z = \tfrac{G}{2}(y^2 - x^2)$ 也可得到同一场。

极头处的场 $B_{\text{pole}} = G\,R$。铁磁极在 1–1.5 T 附近开始饱和，所以对常温四极铁，$G R \lesssim 1$ T 是一个实际的上限：孔径越小，可达到的梯度越高。

### Maxwell 方程的约束

孔径内 $\nabla\cdot\boldsymbol{B} = 0$、$\nabla\times\boldsymbol{B} = 0$。后者的 $z$ 分量给出

$$
\frac{\partial B_y}{\partial x} = \frac{\partial B_x}{\partial y}.
$$

这正是四极铁"一个平面聚焦、另一个平面散焦"的根源：$x$ 方向的恢复力来自 $\partial B_y/\partial x$，$y$ 方向的力来自 $\partial B_x/\partial y$，两者相等而作用符号相反（见下节）。在无源区域内不存在对两个平面同时聚焦的静磁场（Earnshaw 定理的一种表现）。

### 洛伦兹力

对沿 $+z$ 运动、电荷为 $q$、速度近似为 $v\hat{z}$ 的粒子，$\boldsymbol{F} = q\,\boldsymbol{v}\times\boldsymbol{B}$ 给出

$$
F_x = -q v B_y = -q v G\,x,\qquad F_y = q v B_x = q v G\,y .
$$

当 $qG > 0$ 时，$x$ 方向的力指向轴心（聚焦），$y$ 方向的力背离轴心（散焦）；$qG < 0$ 时相反。力与离轴距离成正比，所以是线性透镜，透镜的强度与粒子的横向位置无关。

### 边缘场与有效长度

实际四极铁的梯度在两端不会突然降为零，而是在大约一个孔径半径的距离内平滑衰减（边缘场）。线性光学上通常用**有效长度**

$$
L_{\text{eff}} = \frac{1}{G_0}\int G(s)\,\dd s
$$

和中心梯度 $G_0$ 的"硬边"模型代替。硬边模型保持了积分聚焦强度 $\int G\,\dd s$，但忽略了边缘场引起的高阶（非线性）效应，例如与位置三次方相关的端部作用。高阶多极场（十二极、二十极等，由极头形状与加工误差引起）同样不在矩阵模型中。

## 运动方程与传输矩阵 {#quadrupole-matrix}

### 聚焦强度

将力除以 $p v$，得到以 $s$ 为自变量的线性运动方程（Hill 方程，常系数情形）

$$
x'' + k\,x = 0,\qquad y'' - k\,y = 0,\qquad
k = \frac{q\,G}{p} = \frac{G}{B\rho}.
$$

$B\rho = p/q$ 是磁刚度，常用公式 $B\rho\,[\mathrm{T\,m}] = p\,[\mathrm{MeV}/c]/(299.792458\,Z)$。$k$ 的单位是 m$^{-2}$。

### 厚透镜矩阵

记 $\varphi = \sqrt{\lvert k\rvert}\,L$。对 $k>0$，$x$ 平面聚焦、$y$ 平面散焦：

$$
M_x =
\begin{pmatrix}
\cos\varphi & \dfrac{\sin\varphi}{\sqrt{k}}\\[4pt]
-\sqrt{k}\,\sin\varphi & \cos\varphi
\end{pmatrix},
\qquad
M_y =
\begin{pmatrix}
\cosh\varphi & \dfrac{\sinh\varphi}{\sqrt{k}}\\[4pt]
\sqrt{k}\,\sinh\varphi & \cosh\varphi
\end{pmatrix}.
$$

$k<0$ 时两个平面互换。纵向部分与同长度的漂移段相同（$R_{56} = L/\gamma^2$）。完整的 $6\times 6$ 矩阵是三块对角块：

$$
M_{\text{quad}} = \operatorname{diag}\!\left(M_x,\; M_y,\; \begin{pmatrix} 1 & L/\gamma^2 \\ 0 & 1 \end{pmatrix}\right).
$$

这与 AVAS 线性包络预览的 `quad_matrix(L, k, gamma)` 一致（其中 $k = G/B\rho$，正值在 $x$ 平面聚焦）。

### 薄透镜近似

当 $\varphi \ll 1$ 时，把四极铁看成位于中心的薄透镜加两段 $L/2$ 漂移，透镜矩阵为

$$
M_{\text{thin}} =
\begin{pmatrix}
1 & 0\\
-1/f & 1
\end{pmatrix},
\qquad
\frac{1}{f} = k\,L = \frac{G L}{B\rho}.
$$

$x$ 平面焦距为 $+f$，$y$ 平面为 $-f$。厚透镜的实际焦距（$-1/M_{21}$）与之的差别为 $\mathcal{O}(\varphi^2)$：聚焦平面 $f_x = 1/(\sqrt{k}\sin\varphi)$ 略长，散焦平面 $\lvert f_y \rvert = 1/(\sqrt{k}\sinh\varphi)$ 略短。

### 双合与三合透镜

两个焦距为 $+f$ 和 $-f$ 的薄透镜相距 $d$，组合焦距

$$
\frac{1}{F} = \frac{1}{f_1} + \frac{1}{f_2} - \frac{d}{f_1 f_2} = \frac{d}{f^2}
$$

在两个平面都为正——这就是**交变梯度聚焦**的核心：束流在聚焦透镜处较大、在散焦透镜处较小，净效应是聚焦。双合透镜在两个平面的主平面位置不同，出射束一般不是圆的；对称的三合透镜（$-f/2$、$+f$、$-f/2$ 或反之）可以在两个平面给出相近的成像，常用于需要圆束斑的地方。

### FODO 周期

由聚焦四极铁 QF、漂移、散焦四极铁 QD、漂移组成的周期称为 FODO。对薄透镜，若透镜中心间距为 $L_h$，每周期的相移 $\sigma_0$ 满足

$$
\sin\frac{\sigma_0}{2} = \frac{L_h}{2 f}.
$$

周期运动稳定的条件是单周期矩阵满足 $\lvert \Tr M \rvert/2 < 1$，对薄透镜即 $f > L_h/2$。厚四极铁要用厚透镜矩阵相乘得到 $M$，再由 $\cos\sigma_0 = \Tr M/2$ 求相移。

## 在 AVAS 中 {#quadrupole-avas}

### 矩阵模型四极铁

```text
quad L R 0 G
```

```avas-params
quad
```

- `L`：长度（m），硬边模型的有效长度。
- `R`：孔径半径（m），用于损失判断。
- 第三个参数保留，写 0。
- `G`：磁场梯度（T/m）。

> AVAS 约定：$k = G/(B\rho)$，**$G > 0$ 对正电荷粒子在 $x$ 平面聚焦、$y$ 平面散焦**（已用线性预览的 `quad_matrix` 核对：$G = 10$ T/m、$L = 0.1$ m 时 $M_{21} = -5.09\ \mathrm{m^{-1}}$，即 $x$ 聚焦）。负电荷粒子（例如 H⁻）在同一 $G$ 下聚焦平面互换。设计 FODO 时 QF 用正 $G$、QD 用负 $G$。

> AVAS 约定：`quad` 是 **z-code 矩阵模型元件**，按传输矩阵整体作用，场分布不进入粒子跟踪；手册要求多粒子模拟中第一个和最后一个元件不要是矩阵模型元件，可在首尾加一段很短的 drift（见[漂移段](#drift-avas)）。矩阵模型元件内部如何处理空间电荷，手册没有说明，以内核结果为准。

例：一个 FODO 周期（示例束流，见下文的数值）：

```text
quad  0.1 0.02 0  6
drift 0.2 0.02 0
quad  0.1 0.02 0 -6
drift 0.2 0.02 0
```

### 场图四极铁

需要边缘场或实测场时，用静磁场场图元件（`field`，类型 3）代替：

```text
field 0.1 0.02 0 3 0 0 0 1.0 qmap
```

场图文件为 `qmap.bsx`、`qmap.bsy`（以及可选的 `qmap.bsz`），磁场单位为 T，内核按 `Kb` 缩放：实际场 = 场图 × `Kb`。**`Kb` 的符号决定极性**：把 `Kb` 变号即交换聚焦和散焦平面。静磁场不使用的参数（频率、相位、`Ke`）按手册写 0。场图元件由 t-code 跟踪，自然包含边缘场和场图中的高阶分量。场图的存储顺序和读法见[场图](#field-maps)一章。

### 误差

四极铁的位置、转角和梯度误差用"静磁元件误差"命令给出（作用于命令下方 N 个元件）：

```avas-params
err_quad_ncpl_stat
```

`dx`、`dy` 为横向偏移（mm），`dφx`、`dφy`、`dφz` 为绕三个轴的转角（°），`dG` 为梯度相对误差（%），`dz` 为纵向偏移（mm）。完整的误差研究流程见[误差与校正](#errors)。

## 可调参数与典型取值 {#quadrupole-params}

| 量 | 典型范围（低、中能质子直线加速器） | 说明 |
|---|---|---|
| 梯度 $G$ | 数 T/m 至数十 T/m | 常温电磁铁；永磁四极铁可以更高 |
| 有效长度 $L$ | 0.05–0.3 m | 低能段短而强，高能段长 |
| 孔径半径 $R$ | 0.01–0.04 m | 由束流尺寸、误差和损失预算决定 |
| 极头场 $G R$ | $\lesssim 1$ T（铁芯） | 超过后铁芯饱和、场质量下降 |
| 每周期相移 $\sigma_0$ | 40°–90°（零流强） | 避免 90° 以上的参数共振，见[空间电荷](#space-charge) |

### 数值例子

示例束流：质子，$W = 1.527$ MeV，$p = 53.55$ MeV/$c$，$B\rho = 0.17863$ T·m。

| $G$ | $L$ | $k$ | $\varphi$ | 薄透镜 $f$ | 厚透镜 $f_x$ | 厚透镜 $f_y$ |
|---|---|---|---|---|---|---|
| 4 T/m | 0.1 m | 22.39 m⁻² | 0.473 | 0.447 m | 0.464 m | −0.430 m |
| 6 T/m | 0.1 m | 33.59 m⁻² | 0.580 | 0.298 m | 0.315 m | −0.282 m |
| 10 T/m | 0.1 m | 55.98 m⁻² | 0.748 | 0.179 m | 0.196 m | −0.163 m |
| 20 T/m | 0.05 m | 111.96 m⁻² | 0.529 | 0.179 m | 0.187 m | −0.171 m |

对低能质子，几 T/m 的梯度就已经是很强的透镜；$\varphi$ 接近 1 时薄透镜近似的误差达到 10 % 量级。

FODO 周期 $L_q = 0.1$ m、$L_d = 0.2$ m（透镜中心间距 $L_h = 0.3$ m）的结果：

| $G$ | 厚透镜相移 $\sigma_0$ | 薄透镜估计 | $\Tr M/2$ |
|---|---|---|---|
| 6 T/m | 52.7° | 60.5° | 0.606 |
| 10 T/m | 95.4° | 114.2° | −0.094 |
| 10 T/m，$L_d = 0.3$ m | 不稳定 | 不稳定 | −1.083 |

$G = 6$ T/m 时周期解 $\beta_x$ 在 0.43–1.06 m 之间变化，对示例发射度（$\varepsilon_{n,x} = 0.1377\ \pi$·mm·mrad）对应 rms 尺寸 1.0–1.6 mm。注意薄透镜估计在 $\varphi \approx 0.6$ 时已经偏差 8°，强聚焦时偏差更大，设计时应使用厚透镜矩阵。

## 动力学效应与常见问题 {#quadrupole-dynamics}

### 包络的"呼吸"

在 FODO 周期中，$x$ 方向的 $\beta$ 在 QF 处最大、QD 处最小，$y$ 方向相反；两者的相位相差半个周期。束流截面因此在两个平面交替地变扁，这是四极铁聚焦的特征。圆形束只出现在两者交叉的位置。

### 色差

聚焦强度 $k \propto 1/p$，动量偏差 $\delta$ 的粒子感受到的强度为 $k(1-\delta)$（一阶）。在色散为零的直线段中，色差对 rms 尺寸的影响通常较小，但在能散大、聚焦强的地方会造成有效发射度增长。AVAS 的线性预览不包含色差（见 `linear_optics.py` 的说明），以内核结果为准。

### 错位与转角

- 四极铁横向偏移 $d_x$：束流中心相对磁场中心偏 $-d_x$，受到 $\Delta x' = k L\,d_x$（薄透镜近似）的偏转，即一个偶极"踢"；这是轨道畸变的主要来源，需用[校正铁](#steerer)校正。
- 绕 $z$ 轴转角 $\theta$：产生斜四极分量（强度约 $\sin 2\theta$ 倍），耦合 $x$ 与 $y$，使投影发射度增长。
- 梯度误差 $\Delta G/G$：改变相移，造成失配和包络拍频（beta beating）。

### 空间电荷

强流时空间电荷在两个平面产生散焦，使每周期相移从 $\sigma_0$ 降到 $\sigma$（"相移压低"）。四极铁通道中的束流在两个平面交替变扁，空间电荷力随之变化；为避免包络不稳定和参数共振，零流强相移一般取 $\sigma_0 < 90^\circ$。可用[空间电荷](#space-charge)一章的实验观察这一效应。

### 常见问题

- 梯度符号写反：本来的 FODO 变成"FFOO"，束流在一个平面迅速发散而损失。
- 把 TraceWin 的 mm 单位直接抄入：TraceWin `QUAD` 的长度和孔径是 mm，AVAS 是 m。
- 将矩阵模型四极铁放在 lattice 首尾（见上文 AVAS 约定）。
- 用矩阵模型和场图模型比较结果时，注意场图包含边缘场，有效长度不一定等于场图长度：应使 $\int G\,\dd s$ 相同再比较。

## 动手实验 {#quadrupole-lab}

先看看四极铁横截面上的场和受力（按参数算出的**示意图**，不是场图数据）。AVAS 的横截面一律从下游往上游看，束流垂直屏幕向外（⊙），因此可以直接用 $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$ 核对：$G > 0$ 时，$x$ 轴上的受力指向圆心，$y$ 轴上的受力背离圆心。

```avas-field
quadrupole G=6 L=0.1 R=0.02
```

下面的 FODO 实验使用厚透镜矩阵。先用 $G = 6$ T/m、$L_d = 0.2$ m 复现上表的 52.7°；然后：

1. 逐步增大 `G`，观察相移和 $\lvert\Tr M\rvert/2$，找到稳定区的边界（约在 $\sigma_0 = 180^\circ$）。
2. 把 `Ld` 增大到 0.3 m，在 $G = 10$ T/m 下周期不再稳定，包络发散。
3. 把 `mismatch` 改为 1.3，观察失配束流的包络拍频；再加上流强 `I`，比较有无空间电荷时的包络。

```avas-lab
fodo Lq=0.1 G=6 Ld=0.2 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
```

## 延伸阅读 {#quadrupole-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.（第 7 章：四极铁聚焦、FODO 与周期解）。
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed.（多极场展开、四极铁设计与边缘场）。
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific (2019)（Hill 方程、强聚焦与稳定性）。
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1–48（交变梯度聚焦的经典论文）。
- 相关章节：[线性光学](#linear-optics)、[螺线管](#solenoid)、[匹配](#matching)、[lattice 设计](#lattice-design)、[误差与校正](#errors)；完整书目见[参考文献](#references)。
