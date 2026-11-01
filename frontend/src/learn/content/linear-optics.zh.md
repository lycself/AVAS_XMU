# 线性光学

一个四极铁只能在一个平面里聚焦，在另一个平面里必然散焦。可是把聚焦和散焦的四极铁交替排起来，束流在两个平面里都被约束住了，而不是一边收拢、一边散掉。这是怎么回事？本章用一个假设来回答——**力与偏离量成正比**。有了这个假设，任何一段束线都可以写成一个矩阵，束流的形状按矩阵乘法一段段传下去，系统稳不稳定只看一个数：周期矩阵的迹。读完本章，你应当能解释交变梯度聚焦为什么净效果是聚焦、聚焦太强为什么反而失稳、"匹配束"是什么意思，以及 AVAS 的线性预览在算什么。

需要的准备：[相对论运动学](#kinematics) 中的 $\beta$、$\gamma$、$B\rho$，以及 [相空间与发射度](#phase-space) 中的 Twiss 参数 $\beta_x$、$\alpha_x$、$\gamma_x$ 和 rms 发射度。本章的内容在 [从离子源到靶](#journey-overview) 的故事里处处出现：中能段的四极铁、超导段"螺线管 + 腔"的周期，都是这里讲的周期系统。数值例子都用示例束流：质子，$W = 1.527\ \mathrm{MeV}$，$\beta\gamma = 0.05708$，$B\rho = 0.1786\ \mathrm{T\,m}$。

## 像弹簧一样的力：Hill 方程 {#linear-optics-hill}

### 物理图像

设想一个粒子在束流轴旁边 $x$ 处飞行。如果它受到的横向力与 $x$ 成正比、并且指向轴，它就像挂在弹簧上的小球，绕轴来回振荡；只是这根"弹簧"的强度沿束线变化——在四极铁里很硬，在漂移段里完全没有。线性光学研究的就是这样一个强度随位置 $s$ 变化的弹簧。

线性为什么这么重要？因为线性方程的解可以叠加：两个粒子的初始坐标相加，轨迹也相加。于是只要算出两条特殊的轨迹，所有粒子的运动就都知道了，这就是下一节传输矩阵的来历。

### 近轴运动方程

设粒子沿 $s$ 运动，横向动量 $p_x \approx p\,x'$（近轴近似，$x' \ll 1$）。由 $\dd p_x/\dd t = F_x$ 和 $\dd/\dd t = v\,\dd/\dd s$ 得

$$
x'' + \frac{p'}{p}\,x' = \frac{F_x}{p\,v}
$$

右边的分母 $p\,v$ 值得体会：动量越大，同样的力越难改变方向；速度越大，粒子在单位长度上受力的时间越短。左边的 $p'/p = (\beta\gamma)'/(\beta\gamma)$ 只在加速段不为零，它就是 [绝热阻尼](#phase-space-liouville) 项。对磁场 $F_x = -q v B_y$，对电场 $F_x = qE_x$，于是右边分别为 $-B_y/(B\rho)$ 和 $E_x/(E\rho)$（[刚度](#kinematics-rigidity)）。

### Hill 方程

若横向场对偏离量是线性的，例如四极铁 $B_y = G\,x$、$B_x = G\,y$，在无加速的段落中运动方程化为

$$
x'' + K_x(s)\,x = 0, \qquad y'' + K_y(s)\,y = 0
$$

这就是 Hill 方程，$K(s)$ 是"弹簧强度"，单位 m$^{-2}$。$K > 0$ 处聚焦，$K < 0$ 处散焦。各元件的聚焦函数：

| 元件 | $K_x$ | $K_y$ | 详见 |
|---|---|---|---|
| 漂移段 | 0 | 0 | [漂移段](#drift) |
| 四极铁（梯度 $G$） | $k = G/(B\rho)$ | $-k$ | [四极铁](#quadrupole) |
| 扇形二极铁（场指数 $N$，偏转在 $x$ 平面） | $(1-N)/\rho^2$ | $N/\rho^2$ | [二极铁](#dipole) |
| 螺线管（在拉莫尔旋转坐标系中） | $k_s^2$，$k_s = B/(2B\rho)$ | $k_s^2$ | [螺线管](#solenoid) |

数值：$G = 4\ \mathrm{T/m}$ 的四极铁对示例质子 $k = 4/0.1786 = 22.4\ \mathrm{m^{-2}}$。一个偏离轴 1 mm 的粒子穿过 0.1 m 长的这种四极铁，角度改变约 $kLx = 2.24\ \mathrm{mrad}$，与示例束流的 rms 发散角 3.0 mrad 相当——这正是"适度"聚焦的量级。

对正电荷粒子，$G > 0$ 的四极铁在 $x$ 方向聚焦、$y$ 方向散焦：在 $x > 0$ 处 $B_y > 0$，$\boldsymbol v \times \boldsymbol B$ 指向 $-x$。四极铁不可能同时在两个平面聚焦：无源区 $\nabla\times\boldsymbol B = 0$ 要求 $\partial B_y/\partial x = \partial B_x/\partial y$，于是 $K_y = -K_x$。这是麦克斯韦方程的限制，不是设计不够巧妙，所以要靠交替排列，见 [交变梯度](#linear-optics-alternating)。

> **想一想**：聚焦强度里出现的是 $G/(B\rho)$，而不是梯度 $G$ 本身。同一个四极铁，对 1.5 MeV 质子和 1 GeV 质子的作用差多少？
>
> 答：$B\rho$ 正比于动量。1 GeV 质子的 $B\rho = 5.66\ \mathrm{T\,m}$，是 1.527 MeV 时的 32 倍，同一个四极铁的 $k$ 只剩 1/32。高能段的四极铁要么更长、要么梯度更高，就是这个原因。

Hill 方程用到的近似要记住：近轴（$x' \ll 1$）、场对偏离量线性、动量偏差 $\delta$ 足够小（不同动量的粒子 $K$ 不同，即色差，这里忽略）、段内不加速。任何一条不成立，线性光学都只是近似。

## 传输矩阵：两条轨迹决定一切 {#linear-optics-matrices}

线性方程的解可以写成初值的线性组合 $X(s) = M(s \mid s_0)\,X(s_0)$，$X = (x, x')\tp$。设 $C(s)$、$S(s)$ 是满足 $C(s_0) = 1, C'(s_0) = 0$ 与 $S(s_0) = 0, S'(s_0) = 1$ 的两个基本解：$C$ 是"从轴外平行出发"的轨迹（余弦型），$S$ 是"从轴上斜着出发"的轨迹（正弦型）。任何粒子都是这两条轨迹按初值 $x_0$、$x_0'$ 的叠加，所以

$$
M = \begin{pmatrix} C & S \\ C' & S' \end{pmatrix}
$$

一段束线的总矩阵是各元件矩阵按顺序相乘，**后经过的元件写在左边**：$M = M_n \cdots M_2 M_1$。

> **想一想**：为什么后经过的元件写在左边？
>
> 答：矩阵作用在右边的列向量上。粒子先经过 $M_1$ 得到 $M_1X_0$，再经过 $M_2$ 得到 $M_2(M_1X_0)$。最先作用的矩阵离向量最近，所以在最右边。

### 漂移段（长度 $L$）

$$
M_{\mathrm{drift}} = \begin{pmatrix} 1 & L \\ 0 & 1 \end{pmatrix}
$$

角度不变，位置按角度线性增加，详见 [漂移段](#drift)。

### 薄透镜（焦距 $f$）

$$
M_{\mathrm{thin}} = \begin{pmatrix} 1 & 0 \\ -1/f & 1 \end{pmatrix}
$$

位置不变，角度改变 $-x/f$；$f > 0$ 为聚焦，$f < 0$ 为散焦。

### 厚四极铁（长度 $L$，$k > 0$，$\phi = \sqrt{k}\,L$）

在四极铁内 $K$ 是常数，聚焦平面的解是余弦和正弦，散焦平面是双曲函数：

$$
M_{\mathrm F} = \begin{pmatrix} \cos\phi & \dfrac{\sin\phi}{\sqrt{k}} \\ -\sqrt{k}\sin\phi & \cos\phi \end{pmatrix}, \qquad
M_{\mathrm D} = \begin{pmatrix} \cosh\phi & \dfrac{\sinh\phi}{\sqrt{k}} \\ \sqrt{k}\sinh\phi & \cosh\phi \end{pmatrix}
$$

厚透镜的等效焦距由 $1/f = -M_{21}$ 给出，即聚焦平面 $1/f = \sqrt{k}\sin\phi$；当 $\phi \ll 1$ 时退化为薄透镜 $1/f \approx kL$，称为薄透镜近似。$\phi$ 是粒子在四极铁里走过的"振荡相位"：$\phi$ 小，说明粒子在透镜内几乎来不及改变位置，可以把整个透镜压成一个冲量。

示例：1.527 MeV 质子（$B\rho$ = 0.17863 T·m），$G$ = 20 T/m、$L$ = 0.05 m 的四极铁，$k$ = 111.96 m$^{-2}$，$\phi$ = 0.529：

$$
M_{\mathrm F} = \begin{pmatrix} 0.86328 & 0.04770\ \mathrm m \\ -5.34063\ \mathrm m^{-1} & 0.86328 \end{pmatrix}, \qquad
M_{\mathrm D} = \begin{pmatrix} 1.14325 & 0.05237\ \mathrm m \\ 5.86300\ \mathrm m^{-1} & 1.14325 \end{pmatrix}
$$

聚焦平面等效焦距 0.1872 m，散焦平面 $-0.1706$ m，薄透镜近似给出 $\pm 0.1786$ m。厚透镜在聚焦平面比薄透镜弱、在散焦平面比薄透镜强：粒子在聚焦透镜里一边走一边靠近轴，后半段受力变小；在散焦透镜里一边走一边远离轴，受力变大。$\phi \approx 0.5$ 时薄透镜近似的误差已有约 5 %，这在低能强聚焦段很常见，所以 AVAS 的线性预览对 `quad` 使用厚透镜矩阵（内核矩阵元件内部的具体形式手册未说明，以内核为准）。

## 辛性：相空间面积为什么不变 {#linear-optics-symplectic}

把 $(x, x')$ 平面上一小块平行四边形送进矩阵 $M$，出来的面积是原来的 $\lvert\det M\rvert$ 倍。漂移段把这块面积沿 $x$ 方向"剪切"（角度大的点走得远），薄透镜沿 $x'$ 方向剪切，两种剪切都不改变面积。[刘维尔定理](#phase-space-liouville) 说哈密顿系统的相空间体积守恒，在线性情况下它的表述就是：传输矩阵是**辛矩阵**，

$$
M\tp J\,M = J, \qquad J = \begin{pmatrix} 0 & 1 \\ -1 & 0 \end{pmatrix}
$$

（多维时 $J$ 是各平面 2×2 块组成的分块对角矩阵。）对 2×2 矩阵，辛条件等价于 $\det M = CS' - SC' = 1$。上面的所有矩阵都满足这一点，可以用来检查手算或程序的结果。辛性还给出逆矩阵的简单形式

$$
M^{-1} = \begin{pmatrix} S' & -S \\ -C' & C \end{pmatrix}
$$

**有加速时**，在 $(x, x')$ 坐标下 $\det M = (\beta\gamma)_0/(\beta\gamma)_1 < 1$：纵向动量增大，同样的横向动量对应的角度变小，椭圆面积缩小，这就是绝热阻尼。它不违反刘维尔定理，因为 $(x, x')$ 不是正则坐标；改用 $(x, \beta\gamma\,x')$ 坐标，矩阵重新成为辛矩阵，归一化发射度守恒。

## σ 矩阵与 Twiss 参数的传输 {#linear-optics-twiss-transport}

### σ 矩阵

单个粒子按 $X_1 = MX_0$ 传输，束流的二阶矩（[σ 矩阵](#phase-space-sigma)）就按

$$
\Sigma_1 = M\,\Sigma_0\,M\tp
$$

传输。σ 矩阵描述的是 rms 椭圆，即"束流的形状"；这条规则说明，在线性系统里束流的形状和单个粒子用同一个矩阵传输，不需要跟踪每个粒子。由于 $\det\Sigma_1 = (\det M)^2\det\Sigma_0$，辛传输下 rms 发射度不变。

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

例如束腰 $\beta_0$ = 1 m 处放 $f$ = 0.5 m 的透镜：透镜后 $\alpha_x = 2$、$\gamma_x = 5$ m$^{-1}$，新束腰在其后 0.4 m 处，$\beta^*$ = 0.2 m。注意新束腰不在焦点（0.5 m）处：束流有发射度，不是一束平行光，束腰比几何焦点更靠近透镜。下面的实验就是这个例子：拖动 `f` 和 `L`，看 $\alpha_x$ 在束腰处变号，椭圆面积始终不变。

```avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
```

### 微分形式与包络方程

在聚焦函数为 $K(s)$ 的系统中，Twiss 参数满足

$$
\beta_x' = -2\alpha_x, \qquad \alpha_x' = K\beta_x - \gamma_x
$$

令 $w = \sqrt{\beta_x}$，则 $w'' + K w - 1/w^3 = 0$；乘以 $\sqrt{\varepsilon}$ 得到 rms 包络 $a = \sqrt{\varepsilon\beta_x}$ 的方程

$$
a'' + K a - \frac{\varepsilon^2}{a^3} = 0
$$

它和单粒子的 Hill 方程只差最后一项。这一项总是向外推，像气体的压力：发射度反映了粒子横向的杂乱运动（束流的"热运动"），即使外部聚焦再强，有发射度的束流也不能被压成一个点——尺寸越小，这一项涨得越快（$\propto 1/a^3$）。加入空间电荷后，这个方程再多一项向外的力，就是 [空间电荷](#space-charge) 一章的包络方程。

## β 函数与相移 {#linear-optics-phase-advance}

Hill 方程的一般解可以写成（Floquet 形式）

$$
x(s) = \sqrt{J\beta_x(s)}\,\cos\left(\psi(s) + \psi_0\right), \qquad
\psi(s) = \int_{s_0}^{s} \frac{\dd\sigma}{\beta_x(\sigma)}
$$

$J$ 是 [Courant–Snyder 不变量](#phase-space-twiss)，$\psi$ 是 betatron 相位。这个解看起来像一个振幅和频率都在变的余弦，$\beta_x$ 同时扮演两个角色：

- **振幅**：$\sqrt{J\beta_x}$ 是单个粒子振荡的包络，$\sqrt{\varepsilon\beta_x}$ 是束流的 rms 尺寸；
- **局部波长**：$\dd\psi/\dd s = 1/\beta_x$，$\beta_x$ 越小，相位推进越快。

最容易体会的是常数聚焦 $K$：此时 $\beta_x = 1/\sqrt{K}$，$\psi = \sqrt{K}\,s$，$\beta_x$ 正是振荡波长除以 $2\pi$。聚焦越强，波长越短，束流也越细（$\sqrt{\varepsilon/\sqrt K}$）。

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

给出（按 $M_{12}$ 的符号选取象限）。相移只取决于入口 Twiss 参数和矩阵，与发射度无关：它描述的是单个粒子转了多少圈，而不是束流有多大。

## 交变梯度：为什么"聚焦 + 散焦"净效果是聚焦 {#linear-optics-alternating}

现在回到开头的问题。取两个薄透镜，焦距 $f_1$ 和 $f_2$，相距 $d$。总矩阵 $M = M_{\mathrm{thin}}(f_2)\,M_{\mathrm{drift}}(d)\,M_{\mathrm{thin}}(f_1)$ 的 $-M_{21}$ 就是组合的光焦度：

$$
\frac{1}{F} = \frac{1}{f_1} + \frac{1}{f_2} - \frac{d}{f_1 f_2}
$$

令 $f_1 = f$、$f_2 = -f$（先聚焦后散焦），前两项相消，剩下

$$
\frac{1}{F} = \frac{d}{f^2} > 0
$$

反过来 $f_1 = -f$、$f_2 = f$（先散焦后聚焦，即另一个平面看到的顺序）结果完全一样。**两个平面都得到净聚焦。**

为什么？跟着一个平行入射、离轴 $x_0$ 的粒子走一遍。若先经过聚焦透镜，它被折向轴，走过 $d$ 以后离轴只剩 $x_0(1 - d/f)$，到了散焦透镜受到的向外冲量就比刚才向内的小。若先经过散焦透镜，它被推离轴，到聚焦透镜时离轴 $x_0(1 + d/f)$，受到的向内冲量就比刚才向外的大。关键在于力正比于 $x$：**粒子在聚焦透镜里总是离轴更远，在散焦透镜里总是离轴更近**，所以聚焦总占上风。净聚焦 $1/F = d/f^2$ 是透镜强度 $1/f$ 的二阶量：单个透镜越强，交替排列得到的净聚焦增长得越快。交变梯度同步加速器用的正是梯度很大的磁铁（相对于此前弱聚焦机器里很小的场指数），这一方案因此被称为"强聚焦"（Christofilos 在 1950 年提出但未正式发表，Courant、Livingston、Snyder 于 1952 年独立提出并发表）。

数值：示例束流、$G = 4\ \mathrm{T/m}$、$L_q = 0.1\ \mathrm m$ 的四极铁，薄透镜焦距 $f = 1/(kL_q) = 0.447\ \mathrm m$；两个四极铁中心相距 $d = 0.4\ \mathrm m$ 时，组合焦距 $F = f^2/d = 0.50\ \mathrm m$，两个平面都一样。

一个物理上正确的类比是 Kapitza 摆：支点上下快速振动的倒立摆可以稳定地立着。支点的振动让摆角带上一个小的快速抖动，而这个抖动恰好与振动的力相关——力把摆往外推时摆角偏小，往回拉时摆角偏大，平均下来是一个指向竖直的恢复力矩。这和"粒子在聚焦透镜里离轴更远"是同一个机制。类比的边界在于：Kapitza 摆需要振动远快于摆的自然频率，而 FODO 中每个周期的相移可以大到几十度，只能用下一节的矩阵稳定性判据精确判断；另外光学里的凸透镜在两个方向同时聚焦，四极铁则是一个平面看到"聚焦—散焦"，另一个平面看到"散焦—聚焦"。

> **想一想**：如果把 FODO 中所有四极铁都换成同一极性，会怎样？
>
> 答：$x$ 方向处处聚焦，没问题；$y$ 方向处处散焦，轨迹按 $\cosh$ 指数增长，束流在 $y$ 方向很快打到管壁。交替排列不是为了"更好"，而是唯一能让两个平面同时稳定的办法（螺线管能同时聚焦两个平面，但它的强度按 $(B/B\rho)^2$ 变化，只在低能有优势，见 [螺线管](#solenoid)）。

## 周期系统与稳定性 {#linear-optics-periodic}

直线加速器的聚焦通常由重复的周期单元组成（如 FODO：聚焦四极—漂移—散焦四极—漂移，或"螺线管 + 腔"的周期）。设一个周期的矩阵为 $M$。

### 稳定条件

粒子经过 $n$ 个周期后的坐标为 $M^n X_0$。$M$ 的特征值 $\lambda$ 满足 $\lambda^2 - (\Tr M)\lambda + 1 = 0$（因 $\det M = 1$）。当 $\lvert \Tr M \rvert < 2$ 时两个特征值为 $\ee^{\pm i\mu}$，$M^n$ 只是不断"转圈"，运动有界；当 $\lvert \Tr M \rvert > 2$ 时有一个特征值模大于 1，振幅指数增长。因此稳定条件是

$$
\left\lvert \tfrac12 \Tr M \right\rvert < 1, \qquad \cos\mu = \tfrac12 \Tr M = \tfrac12\left(M_{11} + M_{22}\right)
$$

$\mu$ 是每个周期的相移（零流强时记为 $\sigma_0$）：粒子每经过一个周期，在相空间里绕一圈的 $\mu/360°$。一个 2×2 矩阵有四个数，但稳定与否只由迹这一个组合决定，这是辛性（$\det M = 1$）带来的简化。

### 周期 Twiss 参数

稳定时 $M$ 可以写成

$$
M = \begin{pmatrix} \cos\mu + \alpha_x\sin\mu & \beta_x\sin\mu \\ -\gamma_x\sin\mu & \cos\mu - \alpha_x\sin\mu \end{pmatrix}
$$

由此得到周期解：$\beta_x = M_{12}/\sin\mu$，$\alpha_x = (M_{11} - M_{22})/(2\sin\mu)$，$\gamma_x = -M_{21}/\sin\mu$，其中 $\sin\mu$ 的符号取得使 $\beta_x > 0$。它们只由周期结构决定，与束流无关；它们定义的椭圆经过一个周期后回到自己，是下一节匹配束的基础。

### 薄透镜 FODO

两个焦距 $\pm f$ 的薄透镜相距 $L$（周期长度 $2L$），可以得到

$$
\sin\frac{\mu}{2} = \frac{L}{2f}, \qquad
\beta_{\max,\min} = \frac{2L\left(1 \pm \sin(\mu/2)\right)}{\sin\mu}
$$

$\beta_{\max}$ 在聚焦透镜中心，$\beta_{\min}$ 在散焦透镜中心——与上一节的解释一致：束流在聚焦四极铁里最粗、在散焦四极铁里最细。稳定要求 $f > L/2$。平均 β 可以用平滑近似 $\bar\beta \approx 2L/\mu$ 估计。

### 示例与实验

实验的默认周期是：四极铁 $L_q = 0.1\ \mathrm m$、$G = \pm 4\ \mathrm{T/m}$，漂移 $L_d = 0.3\ \mathrm m$，周期长度 0.8 m，示例质子。用 AVAS 的厚透镜矩阵：

- $\tfrac12\Tr M = 0.666$，$\mu = 48.3°$（两个平面相同）；周期 $\beta_x$ 在 1.485 m 与 0.645 m 之间变化；平滑近似 $\bar\beta \approx 0.8/0.842 = 0.95\ \mathrm m$。
- 薄透镜估计（$f = 0.447\ \mathrm m$，透镜间距 0.4 m）：$\sin(\mu/2) = 0.448$，$\mu = 53.2°$，$\beta$ 在 1.446 m 与 0.552 m 之间。四极铁的 $\phi = \sqrt k L_q = 0.47$，薄透镜近似把相移高估了约 5°。
- 把梯度增大到 6.92 T/m 时 $\mu = 90°$；到 9.80 T/m 时 $\tfrac12\lvert\Tr M\rvert$ 达到 1，束流失稳（薄透镜近似给出 8.93 T/m）。

另一个例子是短而强的四极铁：QF（0.05 m）— 漂移 0.10 m — QD（0.05 m）— 漂移 0.10 m，$G$ = 20 T/m。厚透镜矩阵给出 $\tfrac12\Tr M = 0.726$，$\mu$ = 43.45°，QF 入口处 $\beta_x$ = 0.546 m、$\beta_y$ = 0.303 m；薄透镜估计 $\sin(\mu/2) = 0.420$，$\mu$ = 49.65°，偏大约 6°；梯度约 38.2 T/m 时 $\mu$ = 90°，约 54.1 T/m 时失稳（薄透镜近似给出 47.6 T/m）。

在下面的实验里亲自验证：

1. 从 $G = 4$ 开始，确认 $\mu \approx 48°$，周期 $\beta_x$、$\beta_y$ 在聚焦四极铁处最大；
2. 逐步增大 $G$，看 $\mu$ 经过 90° 后在约 9.8 T/m 处失稳，包络发散；
3. 比较厚透镜和薄透镜的相移，体会 $\sqrt{k}L$ 不小时薄透镜近似的误差。

```avas-lab
fodo Lq=0.1 G=4 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.14 I=0 mismatch=1
```

> **想一想**：聚焦是好事，为什么聚焦太强反而失稳？
>
> 答：每个透镜都把粒子往回拉，拉得太猛就会"矫枉过正"：粒子越过轴跑到另一侧，而且离轴更远。$\mu = 180°$ 时粒子每个周期正好翻到轴的另一侧，再强一点，每次翻过去振幅都放大，$\lvert\Tr M\rvert > 2$。用薄透镜的话说，焦距短于透镜间距的一半（$f < L/2$），束流在到达下一个透镜之前就已经穿过轴并散开。

FODO 单元的设计要点（相移的选择、与腔的配置、平滑近似）在 [束线设计](#lattice-design) 中详述。

> 注意：零流强稳定只是必要条件。有空间电荷时每周期相移会被压低（$\sigma < \sigma_0$），而 $\sigma_0 > 90°$ 时强流束容易出现包络不稳定和参数共振，所以强流直线加速器通常取 $\sigma_0 < 90°$，见 [空间电荷](#space-charge)。

## 匹配束与失配 {#linear-optics-matched}

如果入口束流的 Twiss 参数恰好等于周期解，那么经过每个周期后束流椭圆回到原来的形状，rms 尺寸沿束线按周期重复，这称为**匹配束**。可以把周期解的椭圆想成相空间里的一条"等高线"：每个粒子沿着它转，转了多少都不改变整体的形状。

如果入口椭圆的形状不对，各个粒子仍然沿着各自的等高线转，但整体的椭圆每个周期转过 $\mu$ 角并改变形状，包络出现"拍"（beating）。椭圆转半圈就与原来重合，所以包络振荡的相位每个周期推进 $2\mu$，频率是 betatron 振荡的两倍；最大尺寸比匹配时大。

失配程度常用失配因子衡量。对两组 Twiss 参数（束流与周期解）定义

$$
R = \beta_1\gamma_2 + \beta_2\gamma_1 - 2\alpha_1\alpha_2, \qquad
M_{\mathrm{mis}} = \sqrt{\frac{R + \sqrt{R^2 - 4}}{2}} - 1
$$

完全匹配时 $R = 2$、$M_{\mathrm{mis}} = 0$。若只是 $\beta$ 不同（$\alpha = 0$，$\beta_1 = \lambda\beta_2$），$M_{\mathrm{mis}} = \sqrt{\lambda} - 1$，即最大 rms 尺寸增大的比例。在线性系统中失配束的 rms 发射度不变，只是包络振荡；而在有非线性力的真实束线中，各个粒子转动的快慢略有不同，失配椭圆会慢慢"丝化"成一个更大的椭圆，完全丝化后的 rms 发射度增大到原来的 $R/2$ 倍。如何把束流匹配到周期结构见 [匹配](#matching)。

在实验里把 `mismatch` 设为 1.3：实验把入口 $\beta$ 乘以 $1.3^2 = 1.69$、$\alpha$ 保持匹配值 $-1.57$。由于 $\alpha \ne 0$，失配因子不是 0.3，而是 0.61（$R = 2.97$）；包络最大值从匹配时的 1.91 mm 增大到 3.05 mm，大 60 %，接近上限 $1 + M_{\mathrm{mis}} = 1.61$（只有包络振荡的相位恰好与 β 最大处重合时才达到上限，10 个周期里差一点）；若完全丝化，发射度将增大到 1.49 倍。设回 1 时包络严格周期。

```avas-lab
fodo Lq=0.1 G=4 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.14 I=0 mismatch=1.3
```

## 从 2×2 到 6×6 {#linear-optics-6d}

真实的束流有三个平面。把横向和纵向放在一起，用作用于 $(x, x', y, y', z, \delta)$ 的 6×6 矩阵推进 6×6 σ 矩阵 $\Sigma_1 = M\Sigma_0 M\tp$，这也是 AVAS 线性预览的做法。无耦合元件的矩阵是分块对角的：

$$
M = \begin{pmatrix} M_x & 0 & 0 \\ 0 & M_y & 0 \\ 0 & 0 & M_z \end{pmatrix}, \qquad
M_z^{\mathrm{drift}} = \begin{pmatrix} 1 & L/\gamma^2 \\ 0 & 1 \end{pmatrix}
$$

纵向块的 $L/\gamma^2$ 来自 $\Delta\beta/\beta = \delta/\gamma^2$（[运动学](#kinematics-differential)）：动量较大的粒子跑得较快，经过 $L$ 后在参考粒子前方 $L\delta/\gamma^2$ 处。它和横向漂移的 $L$ 完全同构——纵向的"角度"是动量偏差，"位置"是纵向偏离；高能时 $1/\gamma^2 \to 0$，纵向运动几乎冻结，详见 [漂移段](#drift-longitudinal)。

非对角块描述平面之间的耦合：

- **螺线管** 把 $x$ 与 $y$ 耦合在一起（粒子绕轴旋转），在拉莫尔旋转坐标系中才分解成两个独立平面，见 [螺线管](#solenoid)；
- **二极铁** 让 $x$ 与 $\delta$ 关联（色散：不同动量偏转不同），并通过路程差让 $z$ 与 $x$ 关联，见 [二极铁](#dipole)；
- **射频腔** 的聚束是纵向块里的"透镜"（$\delta$ 随 $z$ 改变），横向则同时出现散焦，见 [纵向动力学](#longitudinal)。

AVAS 线性预览对各元件的处理：

- **drift、quad**：横向如上，纵向块同漂移段；
- **solenoid**：$x$–$y$ 耦合的 4×4 块（硬边，$C = \cos k_sL$、$S = \sin k_sL$）；
- **bend、edge**：扇形二极铁含场指数 $N$，出现 $x$–$\delta$ 与 $z$–$x$ 的关联项（色散），边缘矩阵为薄极面旋转；
- **steerer** 与束诊在预览中是单位矩阵（只改变中心轨道，不改变二阶矩）；
- **场图**：参考粒子以 1 mm 步长积分能量与时间；光学按 4 mm 切片，每片是"局部 $B_z$ 中的半步—由近轴场梯度给出的薄透镜冲量（含射频聚束）—再半步"，并计入动量增加带来的阻尼；
- **空间电荷**（可选）：每隔几个步长加一次 rms 等效均匀椭球（束团）或椭圆柱（直流束）的线性冲量。

预览的精度和不包含的效应见 [在 AVAS 中](#linear-optics-in-avas)。

## 容易误解的地方 {#linear-optics-misconceptions}

- **两个 β**：Twiss 参数 $\beta_x$ 是长度（m，即 mm/mrad），相对论因子 $\beta = v/c$ 无量纲。本章公式里两者同时出现时，带下标的是 Twiss 参数。
- **束流在聚焦四极铁里最细？** 恰好相反。FODO 中 $\beta_{\max}$ 在聚焦四极铁，$\beta_{\min}$ 在散焦四极铁；聚焦透镜的作用是让束流离开它之后变细。
- **束腰在焦点上？** 只有发射度为零的"平行光"才如此。有发射度的束流，束腰位置 $s^* = \alpha/\gamma_x$ 比几何焦点更靠近透镜（上面的例子：焦距 0.5 m，束腰在 0.4 m）。
- **薄透镜近似总是够用？** $\phi = \sqrt{k}L$ 到 0.5 左右时，相移误差已有 5°–6°，失稳梯度的估计也偏低约 10 %。
- **失配因子只看 $\beta$？** 不是。$\alpha$ 不对同样是失配，上面实验里 $\beta$ 只放大 1.69 倍，因为 $\alpha \ne 0$，失配因子却达到 0.61。
- **匹配束就是最小的束？** 匹配说的是"每个周期形状不变"，不是尺寸最小。周期 β 由结构决定，要更细的束要改结构（更强的聚焦、更短的周期）。
- **$\det M = 1$ 永远成立？** 只在正则坐标中成立。在 $(x, x')$ 中有加速时 $\det M < 1$（绝热阻尼），几何发射度减小，归一化发射度不变。
- **零流强稳定就够了？** 那只是必要条件。空间电荷会压低相移、激发共振，强流时要按 $\sigma_0 < 90°$ 设计。
- **相移和流强无关？** 零流强时相移只由结构决定，与发射度无关；有空间电荷时单粒子相移随流强下降，见 [空间电荷](#space-charge-tune-depression)。

## 在 AVAS 中 {#linear-optics-in-avas}

### 矩阵元件与束流参数

lattice 中按传输矩阵整体作用的元件（z-code 矩阵模型元件）是 `quad`、`solenoid`、`bend`、`edge`、`steerer`；`drift` 是 t-code 场模型元件（见 [漂移段](#drift-in-avas)）。它们的参数和符号约定在各元件章节中说明：

```avas-params
drift quad solenoid bend edge steerer
```

束流的入口 Twiss 参数写在 `beam.txt`：

```avas-params
beam:twissx beam:twissy
```

> **AVAS 约定**：`twissx`、`twissy` 依次为 $\alpha$、$\beta$（mm/mrad）、**归一化** rms 发射度（π·mm·mrad）；rms 尺寸为 $\sqrt{\beta\varepsilon_n/(\beta\gamma)}$。示例束流 `twissx -0.65785 0.37743 0.13768` 对应 rms $x$ = 0.954 mm。

### 线性预览

可视化编辑器的线性预览按 [从 2×2 到 6×6](#linear-optics-6d) 的方法推进 σ 矩阵，拖动参数时几乎即时给出 rms 包络。

> **AVAS 约定**：线性预览只用于快速评估。参考粒子能量误差约 0.01 %，rms 尺寸通常相差 1–2 %，多粒子效应强时可达 13 %；它不包含场图中的二极 / 校正分量、`superposeout` 偏移、色差和非线性效应（如低能腔中纵向发射度的增长）、粒子损失以及 `scanphase`。最终以内核结果为准。

> 注意：线性预览中的 drift、quad、solenoid、bend、edge 的 6×6 矩阵都是辛矩阵（已在 $(x, x')$、$(y, y')$、$(z, \delta)$ 三对共轭坐标下数值核对）；行列式不为 1 的只有场图中带加速的切片。

本入门的实验（相空间、FODO、螺线管、二极铁、射频腔、空间电荷）都直接调用线性预览的矩阵和代码，所以实验结果与可视化编辑器中的预览一致，同样以内核为准。

## 延伸阅读 {#linear-optics-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：横向动力学、FODO 与周期系统。
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific：Hill 方程、Floquet 变换与 Courant–Snyder 参数。
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015：传输矩阵与 Twiss 参数的演化。
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1。
- K. L. Brown, "A first- and second-order matrix theory for the design of beam transport systems and charged particle spectrometers", SLAC-75：TRANSPORT 矩阵约定。
- 继续阅读：[纵向动力学](#longitudinal)、[空间电荷](#space-charge)、[四极铁](#quadrupole)、[束线设计](#lattice-design)、[匹配](#matching)。
