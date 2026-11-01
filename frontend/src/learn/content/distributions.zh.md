# 一团粒子怎么描述：初始分布

设想两束质子：Twiss 参数相同，rms 发射度相同，线性预览画出的包络一模一样。可是把它们送进同一条有限孔径、打开空间电荷的束线，一束几乎不丢粒子，另一束在管壁上不断丢失，发射度也长得更快。只用几个 rms 数字描述束流，究竟漏掉了什么？

本章从“一个粒子”讲到“一团粒子”：为什么二阶矩（rms 量）是描述束流最自然的语言，它又说不清什么；KV、水袋、抛物线、高斯四种标准分布各代表什么物理图像，边界和尾部差多少；有限个宏粒子能给出多精确的统计；最后说明 AVAS 怎样造出初始束流。需要的概念——相空间坐标、rms 发射度、Twiss 参数——见 [相空间与发射度](#phase-space)；分布形状在空间电荷下的后果在 [空间电荷](#space-charge) 一章展开。

## 从一个粒子到一团粒子 {#distributions-cloud}

一个粒子在任一时刻由六个数确定：横向位置和角度 $(x, x', y, y')$、纵向位置偏差和相对动量偏差 $(z, \delta)$，也就是六维相空间里的一个点。示例项目的一个束团里有约 $7.07\times10^{6}$ 个质子（束团电荷 $Q = I/f$ = 0.1841 mA / 162.5 MHz = 1.133 pC），把它们的坐标一一列出来既做不到，也没有用：我们关心的不是某个质子在哪里，而是“粒子在相空间里怎样分布”。

于是改用**相空间密度** $f(x, x', y, y', z, \delta)$：在相空间一个小体积 $\dd V$ 里大约有 $N f\,\dd V$ 个粒子，$\int f\,\dd V = 1$。这和人口密度图类似：城市里每平方公里多少人比一份住址名单更有用。这个类比在人烟稀少的地方失效——郊外一个格子里只有零星几户，“密度”只是偶然的计数。束流也一样：中心区域粒子密集，密度是光滑的函数；尾部（束晕）粒子稀少，用有限个宏粒子描述时误差最大，而丢失粒子恰恰发生在尾部。

密度函数仍然包含太多信息。最省事的描述是它的**矩**：任何量 $g$ 的平均值

$$
\langle g\rangle = \int g\,f\,\dd V \approx \frac{1}{N}\sum_{i=1}^{N} g(\boldsymbol u_i)
$$

（右边是用 $N$ 个粒子估计）。一阶矩 $\langle x\rangle$、$\langle x'\rangle$ 是束流中心——束团整体偏了多少、朝哪个方向飞；二阶矩（中心化之后）$\langle x^2\rangle$、$\langle x x'\rangle$、$\langle x'^2\rangle$ 给出尺寸、发散角和二者的关联；更高阶的矩描述形状，例如峰度 $\kappa = \langle x^4\rangle/\langle x^2\rangle^2$ 衡量尾部有多重。下面几节依次回答：为什么二阶矩最重要，它缺了什么，缺的部分怎样用“分布类型”补上。

## rms：为什么偏偏是二阶矩 {#distributions-rms}

### 线性传输只把二阶矩变成二阶矩

在线性元件中，粒子坐标经过传输矩阵 $M$ 变换：$\boldsymbol u_1 = M\boldsymbol u_0$。对所有粒子求 $\boldsymbol u_1\boldsymbol u_1\tp$ 的平均：

$$
\Sigma_1 = \langle \boldsymbol u_1 \boldsymbol u_1\tp\rangle = M\,\langle \boldsymbol u_0 \boldsymbol u_0\tp\rangle\,M\tp = M\,\Sigma_0\,M\tp
$$

这一步只用到“平均是线性运算、$M$ 对所有粒子相同”，与分布形状无关。它说明：**出口的二阶矩只取决于入口的二阶矩**。两束形状完全不同、但二阶矩相同的束流，在线性束线里的 rms 包络完全一样。这正是线性预览和 [线性光学](#linear-optics) 只需要 Twiss 参数的原因，也是本章开头两束流“包络一模一样”的原因。

又因为 $\det\Sigma_1 = (\det M)^2\det\Sigma_0$，而无耗散的线性传输满足 $\det M = 1$，rms 发射度 $\varepsilon = \sqrt{\det\Sigma}$（二维投影）对任何形状的分布都是线性运动的不变量，详见 [σ 矩阵](#phase-space-sigma)。

### 空间电荷下也基本只认 rms

更出人意料的是空间电荷。Lapostolle 提出了 rms 发射度与等效束的概念，Sacherer（1971）证明：对横截面上具有椭圆对称性的连续束，空间电荷在 rms 包络方程中出现的项 $\langle x E_x\rangle$ 只取决于 rms 尺寸，而与密度剖面的具体形状无关。于是 rms 尺寸相同的束流——不论均匀、抛物线还是高斯——受到的“平均”空间电荷散焦相同，可以用一个等效的均匀束代替，这称为 **rms 等效束**。束团化的三维束流只近似满足这一点。形状的影响并没有消失，而是转移到 rms 发射度的变化上（见 [后文](#distributions-space-charge)）。

### 代价：rms 对尾部极其敏感

rms 用 $x^2$ 加权，离中心越远的粒子权重越大。

> **想一想**：一束高斯束 rms 尺寸为 $\sigma_0$。假如有 1 % 的粒子被踢到了 $10\sigma_0$ 处形成束晕，其余不变，rms 尺寸变为多少？
>
> 答：$\langle x^2\rangle = 0.99\,\sigma_0^2 + 0.01\times(10\sigma_0)^2 = 1.99\,\sigma_0^2$，rms 尺寸增大到 $1.41\,\sigma_0$，即 41 %。1 % 的粒子决定了一半的 rms 增长；反过来，rms 增长 1 % 既可能是核心整体变大，也可能只是极少数粒子跑远了。

rms 的另一个盲区是边界：同样的 rms 尺寸，束流边缘可以在 2 倍 rms 处截止（KV），也可以无限延伸（高斯）。管道孔径挡住的是边缘和尾部的粒子，所以“会不会丢粒子”这个问题，rms 回答不了。

## Twiss 参数：rms 椭圆的形状 {#distributions-twiss}

一个平面的三个二阶矩可以改写成一个发射度和三个 Twiss 参数：

$$
\beta_x = \frac{\langle x^2\rangle}{\varepsilon}, \qquad
\alpha_x = -\frac{\langle x x'\rangle}{\varepsilon}, \qquad
\gamma_x = \frac{\langle x'^2\rangle}{\varepsilon}, \qquad
\beta_x\gamma_x - \alpha_x^2 = 1
$$

最后一个等式由 $\varepsilon^2 = \langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2$ 自动成立。它们的含义：$\beta_x$ 是“单位发射度下的尺寸平方”，$\gamma_x$ 是单位发射度下的发散角平方，$\alpha_x$ 描述位置与角度的关联。关联系数

$$
r = \frac{\langle x x'\rangle}{\sqrt{\langle x^2\rangle\langle x'^2\rangle}} = -\frac{\alpha_x}{\sqrt{1 + \alpha_x^2}}
$$

在 $-1$ 到 $1$ 之间，$r > 0$ 表示越靠外的粒子越向外飞——束流在发散。

以示例束流的 x 平面为例（$\alpha_x = -0.658$，$\beta_x = 0.377$ mm/mrad，归一化 rms 发射度 0.1377 π·mm·mrad，$\beta\gamma = 0.05708$）：几何发射度 $\varepsilon = 0.1377/0.05708 = 2.412$ mm·mrad，rms 尺寸 $\sqrt{\beta_x\varepsilon} = 0.954$ mm，rms 角度 $\sqrt{\gamma_x\varepsilon} = 3.026$ mrad，关联系数 $r = 0.550$，束流在入口处是发散的。

Twiss 参数还给出一组**归一化坐标**：

$$
X = \frac{x}{\sqrt{\beta_x}}, \qquad X' = \frac{\alpha_x x + \beta_x x'}{\sqrt{\beta_x}}, \qquad
X^2 + X'^2 = \gamma_x x^2 + 2\alpha_x x x' + \beta_x x'^2
$$

在这组坐标里 rms 椭圆变成半径 $\sqrt{\varepsilon}$ 的圆，所有 Twiss 参数不同的束流都“长得一样”。剩下能区分它们的，只有密度沿半径怎样变化——这就是分布类型。椭圆几何、$\alpha$ 的符号和 Courant–Snyder 不变量见 [Twiss 参数](#phase-space-twiss)。

## 四种标准分布 {#distributions-types}

### 用“振幅”来定义分布

在归一化坐标中，$X^2 + X'^2$ 正是粒子在 x 平面的 Courant–Snyder 不变量（振幅的平方）。把两个横向平面放在一起，定义四维“半径”

$$
r^2 = \frac{X^2 + X'^2 + Y^2 + Y'^2}{a^2}
$$

$a$ 为边界尺度。只依赖 $r^2$ 的分布，说的是“有多少粒子具有多大的总横向振幅”。四种标准分布就是四种回答：

- **KV**（Kapchinskij–Vladimirskij）：全部粒子位于同一个超椭球面 $r^2 = 1$ 上，即每个粒子的两个平面振幅之和都相同。这很不自然，但它有一个独一无二的性质：投影到实空间 $(x, y)$ 是**均匀**的椭圆。均匀带电柱体内部的电场与半径成正比（高斯定理：$E_r$ 正比于所包围的电荷除以 $r$，即 $r^2/r$），空间电荷力严格线性，包络方程严格成立，因此它是空间电荷理论的基准。实际束流不会是 KV；它在强空间电荷下还有特有的不稳定性。
- **WB**（水袋，waterbag）：在 $r^2 \le 1$ 内均匀，像一只装满水的袋子，边缘清楚。
- **PB**（抛物线，parabolic）：密度 $\propto 1 - r^2$，中心最密，向边缘平滑降到零。
- **GS**（高斯，Gaussian）：密度 $\propto \exp(-R^2/2)$（$R$ 以 rms 为单位），没有边界。许多独立小扰动叠加的结果趋于高斯（中心极限定理）；它也是唯一一种各坐标互相独立的旋转对称分布。

> **想一想**：KV 分布的粒子全在一个“球壳”上，为什么它在 $x$–$x'$ 平面里看起来是一个实心的均匀椭圆，而不是一个环？
>
> 答：四维球面向二维平面投影时，被积掉的两个方向恰好补偿了球面的弯曲，投影密度是常数——就像普通球面上均匀分布的点投影到一条直径上是均匀的一样（阿基米德的结论）。下面的一般公式给出同一结论：投影指数为 $-1 + 1 = 0$。

### 投影公式与边界／rms 比

上面的定义都在四维空间，而我们看到、测到的是一维剖面和二维相图。一个一般结果把它们联系起来：$d$ 维空间中密度 $\propto (1 - r^2)^k$ 的分布，积掉 $d - d'$ 个方向后，投影密度 $\propto (1 - r^2)^{k + (d - d')/2}$。KV 的球壳相当于 $k = -1$ 的极限，WB 为 $k = 0$，PB 为 $k = 1$。对 $d = 4$、投影到一维 $x$：

$$
\rho(x) \propto \left(1 - \frac{x^2}{a^2}\right)^{m}, \qquad m = \tfrac12,\ \tfrac32,\ \tfrac52 \quad (\mathrm{KV},\ \mathrm{WB},\ \mathrm{PB})
$$

再用 $\int_{-1}^{1} u^2 (1 - u^2)^m\,\dd u \big/ \int_{-1}^{1} (1 - u^2)^m\,\dd u = 1/(2m + 3)$，得到

$$
\langle x^2\rangle = \frac{a^2}{2m + 3} = \frac{a^2}{4},\ \frac{a^2}{6},\ \frac{a^2}{8}
\quad\Longrightarrow\quad
\frac{x_{\max}}{x_{\mathrm{rms}}} = 2,\ \sqrt6,\ \sqrt8
$$

相应地，包住全部粒子的椭圆（总发射度）与 rms 椭圆的面积比 $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}} = 4, 6, 8$。投影到 $x$–$x'$ 二维平面时指数为 $k + 1$：KV 为均匀椭圆，WB $\propto 1 - r^2$，PB $\propto (1 - r^2)^2$，于是落在 $n$ 倍 rms 椭圆内的粒子比例是 $1 - (1 - n/n_{\max})^{k+2}$（$n_{\max}$ = 4、6、8）；高斯为 $1 - \ee^{-n/2}$。

| 分布 | 四维密度 | $x$ 投影 | 边界 / rms | $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}}$ | 超出 2 倍 rms 的比例 | 峰度 $\kappa$ | 4 倍 rms 椭圆内 |
|---|---|---|---|---|---|---|---|
| KV | 球壳 $r^2 = 1$ | $(1 - x^2/a^2)^{1/2}$ | 2 | 4 | 0 | 2.00 | 100 % |
| WB | $r^2 \le 1$ 内均匀 | $(1 - x^2/a^2)^{3/2}$ | 2.449 | 6 | 2.50 % | 2.25 | 88.9 % |
| PB | $\propto 1 - r^2$ | $(1 - x^2/a^2)^{5/2}$ | 2.828 | 8 | 3.31 % | 2.40 | 87.5 % |
| GS | $\propto \exp(-R^2/2)$ | 高斯 | 无界 | 无界 | 4.55 % | 3 | 86.5 % |

读这张表的方法：四种分布的 rms 相同，从 KV 到 GS，核心逐渐变尖、尾部逐渐变长。对示例束流（$x_{\mathrm{rms}}$ = 0.954 mm），KV、WB、PB 的边界分别在 1.91、2.34、2.70 mm；元件孔径 `R` = 20 mm 约为 21 倍 rms，在没有空间电荷和非线性的漂移段里，四种分布都不会碰到管壁。

在实验里亲眼看：四朵点云用同一组 Twiss 参数（默认取示例束流的 x 平面）生成，虚线是 rms 椭圆和 4 倍 rms 椭圆；第一张图把四种分布的 $x$ 投影按 rms 归一后画在一起。

```avas-lab
distribution alpha=-0.658 beta=0.377 emit=0.138 W=1.527 mass=938.272 N=4000
```

默认 4000 个粒子时，实验给出“最大 $|x|$ / rms”分别约为 2.02、2.35、2.62、3.65，“超出 2 倍 rms 的比例”约为 0.18 %、2.45 %、3.5 %、4.6 %，与表中理论值吻合到统计涨落以内。几处值得注意：

- KV 的点云正好填满 4 倍 rms 椭圆，边缘陡峭；其余三种越过它的比例为 11 %～14 %。
- KV 出现了 0.18 % 的“超出 2 倍 rms”：这次抽样的 rms 比名义值小约 1 %（0.944 mm 对 0.955 mm），2 倍样本 rms 便落在真实边界之内。
- PB 的样本最大值 2.62 明显小于理论边界 2.83：它在边缘附近的密度 $\propto (1 - u^2)^{5/2}$ 极低，4000 个粒子中预期的最大值约为 2.66 倍 rms。有限样本总是低估平滑分布的边界。
- 把 N 拉到 20000，GS 的最大值从 3.65 升到 4.27（概率 $1/N$ 处的理论值分别约为 3.66 和 4.06），而 KV、WB 基本不变。高斯分布的“边界”只是样本数的函数。

## 统计：N 个粒子能说明多少 {#distributions-statistics}

用 $N$ 个粒子估计 $\langle x^2\rangle$，其方差约为 $(\langle x^4\rangle - \langle x^2\rangle^2)/N$。rms 是 $\langle x^2\rangle$ 的平方根，相对误差减半，于是

$$
\frac{\Delta x_{\mathrm{rms}}}{x_{\mathrm{rms}}} \approx \frac{1}{2}\sqrt{\frac{\kappa - 1}{N}}
$$

尾部越重（峰度 $\kappa$ 越大），rms 的统计误差越大：高斯为 $1/\sqrt{2N}$，KV 为 $1/(2\sqrt N)$。示例的 5260 个宏粒子，高斯束 rms 尺寸的统计误差约 0.97 %，KV 约 0.69 %；rms 发射度的统计误差与此同一量级。

尾部的分辨率更差：一个宏粒子就是 $1/N$ 的份额，示例中丢失一个宏粒子即 0.019 %。高斯样本的最大值约在 $P(|x| > x_{\max}) \approx 1/N$ 处，随 $N$ 只按 $\sqrt{2\ln N}$ 缓慢增长。

> **想一想**：要可靠地看到 $10^{-4}$ 量级的损失，大约需要多少宏粒子？
>
> 答：$10^{-4}$ 乘以 $N$ 才是丢失的宏粒子数。想要它有 10 % 左右的统计精度（约 100 个丢失粒子，泊松误差 $1/\sqrt{100}$），$N$ 需要 $10^6$ 量级；5260 个宏粒子时期望值只有 0.5 个，根本无法分辨。宏粒子数与统计误差的更多讨论见 [多粒子模拟要点](#simulation)。

## 真实束流：示例里的 RFQ 出口分布 {#distributions-real-beam}

示例项目的 `InputFile/part_rfq.dst` 是一份 RFQ 出口的粒子分布：5260 个粒子、1.527 MeV、0.1841 mA、162.5 MHz，正是 `beam.txt` 里的数值；它的 x、y 平面 rms Twiss 参数与 `twissx`、`twissy` 五位有效数字相同。也就是说，在横向平面上，示例的 `beam.txt` 是这份分布的 **rms 等效束**，再按 `distribution GS GS` 重新生成；纵向的 `twissz` 与文件并不完全一致（见 [示例](#distributions-in-avas)）。

逐项统计这份文件（5260 个粒子）：

| 量 | RFQ 束 | KV | WB | PB | GS |
|---|---|---|---|---|---|
| $x$ 投影的峰度 $\kappa$ | 2.07 | 2.00 | 2.25 | 2.40 | 3 |
| $x'$ 投影的峰度 $\kappa$ | 2.45 | 2.00 | 2.25 | 2.40 | 3 |
| $x$ 超出 2 倍 rms 的比例 | 0.89 % | 0 | 2.50 % | 3.31 % | 4.55 % |
| $x$–$x'$：4 倍 rms 椭圆内 | 89.3 % | 100 % | 88.9 % | 87.5 % | 86.5 % |
| $x$–$x'$：6 倍 rms 椭圆内 | 98.8 % | 100 % | 100 % | 98.4 % | 95.0 % |
| $x$–$x'$：最外粒子所在椭圆 / rms | 8.1 | 4 | 6 | 8 | 无界 |

真实束流不属于四种中的任何一种：位置投影接近 KV，角度投影接近 PB，相平面上的包含比例落在 WB 与 PB 之间。这是 RFQ 中非线性场和空间电荷共同塑造的结果。重要的是与 GS 的差别：GS 认为有 5.0 % 的粒子在 6 倍 rms 椭圆之外，真实束只有 1.2 %。用 GS 代替它，线性光学和 rms 包络完全一样，尾部却重得多——研究损失和束晕时，这一差别就是结论本身。

> **想一想**：把示例的 `distribution GS GS` 改成 `WB WB`，结构页的线性预览会变吗？
>
> 答：不会。线性预览只推进二阶矩，不读分布类型。只有内核的多粒子模拟——在空间电荷、射频腔的正弦场和孔径作用下——才看得到形状的差别。

## 分布形状为什么在空间电荷下要紧 {#distributions-space-charge}

没有空间电荷、元件又是线性的时候，分布形状对 rms 演化毫无影响，只决定有多少粒子落在孔径外。空间电荷改变了这一点，原因可以用高斯定理一步看清。

长直圆束的径向电场 $E_r$ 正比于半径 $r$ 内包围的电荷除以 $r$。均匀束内部包围电荷 $\propto r^2$，所以 $E_r \propto r$，是线性力，与线性聚焦一起只改变包络，不改变发射度。高斯束（rms $\sigma$）包围的电荷为 $\lambda\,(1 - \ee^{-r^2/2\sigma^2})$：

$$
E_r = \frac{\lambda}{2\pi\varepsilon_0}\,\frac{1 - \ee^{-r^2/2\sigma^2}}{r}
\;\approx\; \frac{\lambda}{2\pi\varepsilon_0}\,\frac{r}{2\sigma^2}\quad (r \ll \sigma)
$$

中心附近线性，外面按 $1/r$ 下降。与之 rms 相同的均匀束半径 $a = 2\sigma$，内部 $E_r = \lambda r/(2\pi\varepsilon_0 a^2) = \lambda r/(2\pi\varepsilon_0\cdot 4\sigma^2)$。也就是说，**高斯束核心受到的空间电荷散焦梯度是 rms 等效均匀束的两倍**，而尾部受到的力弱得多。

非线性力让不同振幅的粒子以不同频率振荡，相空间里的椭圆被扭成 S 形并逐渐丝化，rms 发射度随之增长。从能量角度看，rms 尺寸相同时非均匀分布的电场能量比均匀分布高；束流趋向均匀时，多出的“自由能”转化为横向运动的能量，表现为发射度增长。这种电荷重新分布很快，大约在四分之一个等离子体振荡周期内完成，流强越强、初始分布越不均匀，增长越大（Wangler、Reiser）。匹配的 KV 束在实空间本来就是均匀的，自场严格线性，没有可以释放的自由能，因此这一机制不会让它的发射度增长。

外场的非线性也会区分分布：射频腔的电场随相位按正弦变化，束团在纵向上越长的尾部，感受到的非线性越强。纵向分布（`distribution` 的第二个参数）的选择对此敏感。

以示例束流为例：平均流强只有 0.18 mA，但电荷集中在束团里。纵向 rms 长度 1.316 mm 对应 $\sigma_t = 77.0$ ps，高斯束团的峰值流强约为 $Q/(\sqrt{2\pi}\,\sigma_t)$ = 5.9 mA，比平均值大 32 倍。空间电荷是否重要要看峰值流强，并与外聚焦比较，见 [空间电荷](#space-charge)。

## 容易误解的地方 {#distributions-misconceptions}

- **“Twiss 参数相同就是同一束流。”** Twiss 参数和 rms 发射度只确定二阶矩。线性光学下两者等价；涉及孔径、损失、空间电荷和非线性场时，分布形状同样重要。
- **“总发射度就是 4 倍 rms 发射度。”** 只有 KV 如此（在二维相平面中均匀的椭圆也是 4 倍）。四维 WB 为 6 倍、PB 为 8 倍、高斯没有总发射度。比值还取决于分布在几维空间中定义：同叫“水袋”，二维定义（$x$–$x'$ 平面内均匀）的比值是 4，四维定义是 6。
- **“样本的最大值就是分布的边界。”** 平滑分布的边缘密度极低，有限样本会低估边界（PB 在 4000 个粒子时只到 2.62 倍 rms，而不是 2.83）；高斯样本的最大值只反映样本数。
- **“高斯分布的 σ 就是 rms。”** 未截断时是。截断后 rms 变小：在每个坐标 $\pm 3\sigma$ 截断的一维高斯，rms 为 $0.987\sigma$；$\pm 2\sigma$ 截断时为 $0.880\sigma$。
- **“rms 发射度主要反映束流核心。”** 恰好相反：它按 $x^2$ 加权，被远处少数粒子主导（1 % 的粒子在 10 倍 rms 处即可让 rms 尺寸增大 41 %）。研究束晕要看百分比发射度或尾部比例。
- **“KV 是最理想、最真实的分布。”** KV 是理论上最方便的分布，不是真实束流的写照；它在强空间电荷下还有特有的不稳定性。
- **“归一化和几何发射度只差一个记号。”** `beam.txt` 里写的是归一化 rms 发射度；本章实验画的点云和给出的发射度是几何量，两者相差 $\beta\gamma$（示例中约 17.5 倍）。

## 在 AVAS 中 {#distributions-in-avas}

### 两种初始束流

AVAS 的初始束流有两种来源，由束流页的“使用粒子文件”开关（`use_dst`）选择：

1. **按参数生成**：由 `twissx`、`twissy`、`twissz` 给出三个平面的 rms 参数，`distribution V1 V2` 选择分布类型，`particlenumber` 给出宏粒子数，内核用随机数 `randomseed` 抽样。
2. **读入粒子文件**：`readparticledistribution` 指定已有的 `.dst` 或 `.edst` 文件（例如其他程序计算的 RFQ 出口分布）。此时除 `numofcharge` 外，`beam.txt` 的其他关键字都不生效；`.edst` 可以描述多种粒子组成的混合束。

```avas-params
beam:distribution beam:twissx beam:twissy beam:twissz beam:particlenumber beam:randomseed beam:readparticledistribution beam:use_dst beam:initpos beam:initmom beam:numofcharge beam:beamtype
```

### 关键字

- `distribution V1 V2`：V1 为横向（x、y 两个平面共用）分布，V2 为纵向分布，可选 `KV`、`GS`、`PB`、`WB`。
- `twissx α β ε`（`twissy`、`twissz` 同）：$\alpha$ 无量纲，$\beta$ 单位 mm/mrad，$\varepsilon$ 为**归一化 rms** 发射度（π·mm·mrad）；$z$ 平面的“角度”是 $\delta = \Delta p/p$（以 mrad 计），同样乘 $\beta\gamma$ 归一化。单位与换算见 [相空间：在 AVAS 中](#phase-space-in-avas)。
- `particlenumber`：宏粒子数。写 1 时为单粒子模式，此时 `twiss*`、`current`、`distribution` 不生效。每个宏粒子的电荷为 $Q/N$，荷质比与真实粒子相同，所以外场中的运动不变，只有空间电荷按总电荷计算。示例 5260 个宏粒子，每个代表约 1344 个质子。
- `randomseed`：生成初始束流的随机数种子。
- `initpos x y z`（mm）、`initmom x' y' z'`（mrad）：给整个束团加位置和角度偏移（改变一阶矩），用来研究初始偏心。
- `numofcharge`：电荷数；读入粒子文件时它仍然有效。
- `beamtype`：写 `dc` 时生成直流束，`notdc` 为束团。
- `use_dst`：界面使用的开关（手册未收录），1 表示读取 `readparticledistribution` 指定的文件，0 表示按参数生成。

> **AVAS 约定**：手册没有说明 AVAS 生成 KV、WB、PB 时在几维空间中定义、高斯分布在哪里截断，以内核为准。本章实验按教科书在四维横向相空间中的定义抽样。需要准确知道时，读取运行输出目录中的 `inData.dst`——模拟实际使用的初始分布——直接统计。

> **一次运行的观察（手册未说明）**：下面的统计来自本仓库之外、一个较早的 AVAS 示例目录中的一份 `inData.dst`（5260 个粒子）；它的 `beam.txt` 与示例相同（`distribution GS GS`，只少了 `beamtype` 一行），生成它的内核版本没有记录。这些数字说明的是那一次运行的结果，不是文档规定的行为；需要时请统计自己运行的 `inData.dst`。该文件中各坐标投影的峰度为 2.88～3.05，超出 2 倍 rms 的比例为 4.3 %～4.6 %，最远粒子在 3.5～4.0 倍 rms，二维、四维、六维振幅的分布与不截断的高斯一致，三个平面之间没有可见的关联。在这一样本量下看不出截断。那次生成的结果**没有**重新缩放到精确的输入值：x 平面 rms 发射度为 0.1361（输入 0.13768，小 1.1 %），$\beta_x$ 为 0.3796（大 0.6 %），$\alpha_x$ 为 −0.672（大 2.1 %），都在 5260 个粒子的统计涨落之内。纵向按 $(\varphi, W)$ 换算回 $(z, \delta)$ 后得到 $\alpha_z = 0.053$、$\beta_z = 0.378$、$\varepsilon_z = 0.262$，与 `twissz` 的约定一致。

### 示例：beam.txt 与 part_rfq.dst

```text
readparticledistribution unknown
use_dst 0
particlenumber 5260
twissx -0.65785 0.37743 0.13768
twissy -0.6341 0.37292 0.14059
twissz 0.04275 0.37292 0.26506
distribution GS GS
```

x、y 平面的 Twiss 参数就是 `part_rfq.dst` 的 rms 值。纵向不同：文件统计得到 $\beta_z$ = 0.3326 mm/mrad、$\varepsilon_z$ = 0.2659 π·mm·mrad，而 `twissz` 写的 $\beta_z$ = 0.37292，恰与 `twissy` 的 $\beta$ 相同；两者对应的 rms 束长分别为 1.245 mm 和 1.316 mm。这只是对示例文件的观察，`twissz` 为何这样填写没有记录。要让模拟从真实的 RFQ 分布出发，把 `use_dst` 设为 1、`readparticledistribution` 设为 `part_rfq.dst` 即可。束流页可以从粒子文件计算 rms 参数并填入 Twiss 栏。

### 需要注意

- 线性预览只用 rms 参数：按参数生成时读 `twiss*`，读入 `.dst` 时用文件的 rms Twiss 参数；`.edst` 文件预览不读取，改用 `twiss*` 并给出提示。分布类型不影响预览。
- 比较不同程序的结果时，要保证初始分布一致——不只是 Twiss 参数，还有分布类型、维数定义和截断。最稳妥的做法是交换粒子文件。
- 宏粒子数有限，初始束流本身就带着 [统计误差](#distributions-statistics)；判断某个参数的影响时，差别要大于这一误差才可靠。

## 延伸阅读 {#distributions-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：KV 与其他平稳分布、rms 发射度与 rms 等效束、自由能与发射度增长。
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：空间电荷一章中的电荷重新分布与发射度增长。
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1101。
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1105。
- I. M. Kapchinskij, V. V. Vladimirskij, *Proc. Int. Conf. on High Energy Accelerators and Instrumentation*, CERN, 1959：KV 分布的原始论文。
- AVAS 使用说明 `docs/使用说明20260427.docx`：`beam.txt` 关键字。
