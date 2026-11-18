# 一团粒子怎么描述：初始分布

设想两束质子：它们的"典型尺寸"一样，"典型角度"一样，线性预览画出的包络也一模一样。可是把它们送进同一条管道有限粗细、粒子之间互相推挤的束线，一束几乎不丢粒子，另一束在管壁上不断丢失，而且越走越"乱"。只用几个平均数字描述束流，究竟漏掉了什么？

打个比方。两位射手各打 1000 发，弹着点离靶心的"平均距离"完全一样。第一位的弹孔全都落在第 8 环以内，边缘干干净净；第二位大部分打得更集中，可偶尔有几发飞到靶纸外面。如果靶纸的边缘就是加速器的管壁，那么决定"打不打得到墙"的不是平均距离，而是那几发飞得最远的。束流也一样：**平均数字说的是"大多数粒子"，丢失却发生在"极少数粒子"身上**。

这一章从"一个粒子"讲到"一团粒子"：用什么数字描述一团粒子，这些数字为什么够用、什么时候不够用；四种标准分布 KV、水袋、抛物线、高斯各是什么样子，边缘和尾巴差多少；有限个宏粒子能给出多准的统计；最后说明 AVAS 怎样造出初始束流。用到的相空间、发射度、Twiss 参数在[相空间与发射度](#phase-space)一章有更多介绍；分布形状在空间电荷下的后果在[空间电荷](#space-charge)一章展开。

## 从一个粒子到一团粒子 {#distributions-cloud}

一个粒子在任一时刻由六个数确定：横向两个方向的位置和角度 $(x, x', y, y')$，以及前进方向上相对参考粒子的位置偏差 $z$ 和动量的相对偏差 $\delta = \Delta p/p$。把这六个数当成坐标，一个粒子就是一个六维空间（**相空间**）里的一个点。平时我们一次只画两个数，例如横坐标 $x$、纵坐标 $x'$，每个粒子就是图上的一个点。

示例项目的一个束团里有约 $7.07\times10^{6}$ 个质子：束团电荷 $Q = I/f$ = 0.1841 mA / 162.5 MHz = 1.133 pC（平均流强除以每秒的束团数），再除以每个质子的电荷 $1.602\times10^{-19}$ C。把七百万个质子的坐标一一列出来既做不到，也没有用：我们关心的不是某个质子在哪里，而是"粒子在相空间里怎样分布"。

这就像人口：一份全国人口的住址名单没法读，一张**人口密度图**（每平方公里多少人）却一目了然。描述束流用**相空间密度** $f(x, x', y, y', z, \delta)$：在相空间一个小体积 $\dd V$ 里大约有 $N f\,\dd V$ 个粒子，全部加起来 $\int f\,\dd V = 1$。这个类比在人烟稀少的地方会失效——郊外一个格子里只有零星几户，"密度"只是偶然的计数。束流也一样：中心区域粒子密集，密度是光滑的函数；尾部（**束晕**）粒子稀少，用有限个宏粒子描述时误差最大，而丢失粒子恰恰发生在尾部。

密度函数仍然包含太多信息，最省事的描述是几个**平均值**（也叫**矩**）。任何量 $g$ 的平均值是

$$
\langle g\rangle = \int g\,f\,\dd V \approx \frac{1}{N}\sum_{i=1}^{N} g(\boldsymbol u_i),
$$

右边是用 $N$ 个粒子估计：把每个粒子的 $g$ 加起来除以 $N$，就是中学的算术平均。$\boldsymbol u_i$ 是第 $i$ 个粒子的六个坐标。

- **一阶矩** $\langle x\rangle$、$\langle x'\rangle$：束流中心，即整个束团偏了多少、朝哪个方向飞。
- **二阶矩**（先减去中心）$\langle x^2\rangle$、$\langle x x'\rangle$、$\langle x'^2\rangle$：尺寸、角度的分散和二者的关联。
- **更高阶的矩**描述形状，例如**峰度** $\kappa = \langle x^4\rangle/\langle x^2\rangle^2$ 衡量尾巴有多重：四次方让远处的粒子分量更重，$\kappa$ 越大，尾巴越长。

下面几节依次回答：为什么二阶矩最重要，它缺了什么，缺的部分怎样用"分布类型"补上。

## rms：为什么偏偏是二阶矩 {#distributions-rms}

### 什么是 rms

**rms**（root mean square，均方根）是"先平方、再平均、再开方"。例：五个粒子的位置是 $-2, -1, 0, 1, 2$ mm，平方是 4、1、0、1、4，平均是 2，开方得 rms 尺寸 1.41 mm。它是"粒子离中心的典型距离"。同样有 rms 角度（$x'$ 的 rms）。前面的 $\langle x^2\rangle$ 开方就是 rms 尺寸。

### 线性传输只认二阶矩

在线性元件里（力和离轴距离成正比，见[线性光学](#linear-optics)），每个粒子的出口坐标是入口坐标乘上系数再相加，例如漂移段 $x = x_0 + L\,x_0'$。把它平方再对所有粒子取平均：

$$
\langle x^2\rangle = \langle x_0^2\rangle + 2L\,\langle x_0x_0'\rangle + L^2\,\langle x_0'^2\rangle .
$$

出口的 $\langle x^2\rangle$ 只用到了入口的三个二阶矩，和粒子具体怎样分布无关。这是一个惊人的结论：**两束形状完全不同、但二阶矩相同的束流，在线性束线里的 rms 包络完全一样**。

用四个粒子就能亲手验证（位置单位 mm，角度单位 mrad）：

- **菱形束**：四个粒子在 $(\pm1.414, 0)$、$(0, \pm1.414)$，即相空间图上一个菱形的四个顶点。
- **方形束**：四个粒子在 $(1, 1)$、$(-1, -1)$、$(1, -1)$、$(-1, 1)$，即一个正方形的四个角。

两束的二阶矩一样：$\langle x^2\rangle = 1$、$\langle x'^2\rangle = 1$、$\langle x x'\rangle = 0$。走 1 m 漂移（$x \to x + x'$）：菱形束的位置变成 $1.414, -1.414, 1.414, -1.414$，方形束变成 $2, -2, 0, 0$。两束的 $\langle x^2\rangle$ 都是 2，rms 尺寸都是 1.41 mm，和上式算的一样。可是最远的粒子不一样：菱形束最远 1.41 mm，方形束最远 2 mm。**rms 完全相同，边缘却差了 40 %**。

这正是线性预览和[线性光学](#linear-optics)只需要 Twiss 参数的原因，也是本章开头两束流"包络一模一样"的原因。同样只用二阶矩算出的 **rms 发射度** $\varepsilon = \sqrt{\langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2}$（"椭圆面积"），在线性传输中对任何形状的分布都不变（上面两束走 1 m 前后都是 1 mm·mrad），详见[深入部分](#distributions-deep-moments)和[σ 矩阵](#phase-space-sigma)。

### 空间电荷下也基本只认 rms

更出人意料的是空间电荷（粒子之间的电排斥）。Lapostolle 提出了 rms 发射度与等效束的概念，Sacherer（1971）证明：对横截面上具有椭圆对称性的连续束，空间电荷在 rms 尺寸的方程里出现的那一项只取决于 rms 尺寸，而与密度剖面的具体形状无关。于是 rms 尺寸相同的束流——不论均匀、抛物线还是高斯——受到的"平均"空间电荷散焦相同，可以用一个等效的均匀束代替，这称为 **rms 等效束**。成团的三维束流只近似满足这一点。形状的影响并没有消失，而是转移到 rms 发射度的变化上（见[后文](#distributions-space-charge)）。

### 代价：rms 对尾部极其敏感

rms 是先平方再平均，离中心越远的粒子权重越大。举个例子：一束高斯束 rms 尺寸为 $\sigma_0$，假如有 1 % 的粒子被踢到了 $10\sigma_0$ 处形成束晕，其余不变，那么

$$
\langle x^2\rangle = 0.99\,\sigma_0^2 + 0.01\times(10\sigma_0)^2 = 1.99\,\sigma_0^2 ,
$$

rms 尺寸增大到 $1.41\,\sigma_0$，即 41 %。1 % 的粒子决定了一半的 rms 增长；反过来，rms 增长 1 % 既可能是核心整体变大，也可能只是极少数粒子跑远了。

rms 的另一个盲区是边缘：同样的 rms 尺寸，束流边缘可以在 2 倍 rms 处截止（KV 分布），也可以无限延伸（高斯分布）。管道挡住的是边缘和尾部的粒子，所以"会不会丢粒子"这个问题，rms 回答不了。

## Twiss 参数：rms 椭圆的形状 {#distributions-twiss}

把一团粒子画在 $x$–$x'$ 图上，它大致占据一个椭圆，叫 **rms 椭圆**。一个平面的三个二阶矩可以改写成一个发射度（椭圆大小）和三个 **Twiss 参数**（椭圆形状）：

$$
\beta_x = \frac{\langle x^2\rangle}{\varepsilon}, \qquad
\alpha_x = -\frac{\langle x x'\rangle}{\varepsilon}, \qquad
\gamma_x = \frac{\langle x'^2\rangle}{\varepsilon}, \qquad
\beta_x\gamma_x - \alpha_x^2 = 1 .
$$

最后一个等式由 $\varepsilon^2 = \langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2$ 自动成立（代进去就能验证）。它们的含义：$\beta_x$ 是"单位发射度下的尺寸平方"，$\gamma_x$ 是单位发射度下的角度平方，$\alpha_x$ 描述位置与角度的关联，也就是椭圆的倾斜。关联的强弱用**关联系数**

$$
r = \frac{\langle x x'\rangle}{\sqrt{\langle x^2\rangle\langle x'^2\rangle}} = -\frac{\alpha_x}{\sqrt{1 + \alpha_x^2}}
$$

衡量，它在 $-1$ 到 $1$ 之间，$r > 0$ 表示越靠外的粒子越向外飞——束流在发散。

以示例束流的 $x$ 平面为例（$\alpha_x = -0.658$，$\beta_x = 0.377$ mm/mrad，归一化 rms 发射度 0.1377 π·mm·mrad，相对论量 $\beta\gamma = 0.05708$）：几何发射度 $\varepsilon = 0.1377/0.05708 = 2.412$ mm·mrad，rms 尺寸 $\sqrt{\beta_x\varepsilon} = 0.954$ mm，rms 角度 $\sqrt{\gamma_x\varepsilon} = 3.026$ mrad，关联系数 $r = 0.550$，束流在入口处是发散的。（归一化与几何发射度的区别见[相空间](#phase-space)。）

还可以把任何一个 rms 椭圆"拉伸再转正"，变成半径 $\sqrt{\varepsilon}$ 的圆（**归一化坐标**，具体公式见[深入部分](#distributions-deep-shapes)）。在归一化坐标里，所有 Twiss 参数不同的束流都"长得一样"，都是一个圆。剩下能区分它们的，只有**密度从圆心向外怎样变化**：中心挤、边缘稀，还是处处一样密，有没有清楚的边界。这就是**分布类型**。椭圆几何、$\alpha$ 的符号和 Courant–Snyder 不变量见[Twiss 参数](#phase-space-twiss)。

## 四种标准分布 {#distributions-types}

### 四种回答

在归一化坐标里，粒子离圆心的距离代表它摆动的**振幅**（摆得多远）。把两个横向平面放在一起，每个粒子有一个"总横向振幅"。四种标准分布就是对"多少粒子具有多大的总振幅"的四种回答：

- **KV**（Kapchinskij–Vladimirskij）：所有粒子的总振幅都一样，全部位于四维空间里一个"球壳"上。这很不自然，但它有一个独一无二的性质：投影到实际空间 $(x, y)$ 上是**均匀**的椭圆，像一根均匀带电的圆柱。均匀带电圆柱内部的电场与半径成正比，所以空间电荷力严格线性，包络方程严格成立，KV 因此是空间电荷理论的基准。实际束流不会是 KV；它在强空间电荷下还有特有的不稳定性。
- **WB**（水袋，waterbag）：在边界以内处处一样密，像一只装满水的袋子，边缘清楚。
- **PB**（抛物线，parabolic）：中心最密，向边缘平滑地降到零，密度沿半径像一条倒扣的抛物线。
- **GS**（高斯，Gaussian）：中心最密，向外按钟形曲线下降，**没有边界**，再远也还有一点点粒子。许多独立的小扰动叠加起来，结果趋于高斯（中心极限定理）；它也是唯一一种各坐标互相独立的旋转对称分布。

从 KV 到 GS，四种分布在 rms 相同的前提下，核心越来越尖，尾巴越来越长。

### 边界、尾巴和峰度

下表把四种分布放在一起比较（rms 相同）。几个说明：

- "边界 / rms"：最远的粒子离中心是 rms 的几倍。
- "$n$ 倍 rms 椭圆"：面积是 rms 椭圆 $n$ 倍的椭圆（尺寸是 $\sqrt{n}$ 倍）。"4 倍 rms 椭圆内"就是落在尺寸 2 倍于 rms 椭圆的那个椭圆里的粒子比例。
- $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}}$：包住全部粒子的椭圆（**总发射度**）与 rms 椭圆的面积比。

| 分布 | 四维密度 | $x$ 投影 | 边界 / rms | $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}}$ | 超出 2 倍 rms 的比例 | 峰度 $\kappa$ | 4 倍 rms 椭圆内 |
|---|---|---|---|---|---|---|---|
| KV | 球壳 $r^2 = 1$ | $(1 - x^2/a^2)^{1/2}$ | 2 | 4 | 0 | 2.00 | 100 % |
| WB | $r^2 \le 1$ 内均匀 | $(1 - x^2/a^2)^{3/2}$ | 2.449 | 6 | 2.50 % | 2.25 | 88.9 % |
| PB | $\propto 1 - r^2$ | $(1 - x^2/a^2)^{5/2}$ | 2.828 | 8 | 3.31 % | 2.40 | 87.5 % |
| GS | $\propto \exp(-R^2/2)$ | 高斯 | 无界 | 无界 | 4.55 % | 3 | 86.5 % |

（"四维密度"和"$x$ 投影"两列的符号和推导见[深入部分](#distributions-deep-shapes)；$a$ 是边界的大小，$r$ 是以 $a$ 为单位的总振幅，$R$ 以 rms 为单位。）

对示例束流（$x_{\mathrm{rms}}$ = 0.954 mm），KV、WB、PB 的边界分别在 1.91、2.34、2.70 mm；元件孔径 `R` = 20 mm 约为 21 倍 rms，在没有空间电荷和非线性的漂移段里，四种分布都不会碰到管壁。

### 亲眼看一看

在实验里，四朵点云用同一组 Twiss 参数（默认取示例束流的 $x$ 平面）生成，虚线是 rms 椭圆和 4 倍 rms 椭圆；第一张图把四种分布的 $x$ 投影按 rms 归一后画在一起。先看四朵云的大小：它们的 rms 完全一样；再看边缘：KV 齐刷刷地截止，GS 的点稀稀拉拉地伸得很远。

```avas-lab
distribution alpha=-0.658 beta=0.377 emit=0.138 W=1.527 mass=938.272 N=4000
```

默认 4000 个粒子时，实验给出"最大 $|x|$ / rms"分别约为 2.02、2.35、2.62、3.65，"超出 2 倍 rms 的比例"约为 0.18 %、2.45 %、3.5 %、4.6 %，与表中理论值吻合到统计涨落以内。几处值得注意：

- KV 的点云正好填满 4 倍 rms 椭圆，边缘陡峭；其余三种越过它的比例为 11 %～14 %。
- KV 出现了 0.18 % 的"超出 2 倍 rms"：这次抽样的 rms 比名义值小约 1 %（0.944 mm 对 0.955 mm），2 倍样本 rms 便落在真实边界之内。
- PB 的样本最大值 2.62 明显小于理论边界 2.83：它在边缘附近的密度 $\propto (1 - u^2)^{5/2}$ 极低，4000 个粒子中预期的最大值约为 2.66 倍 rms。有限样本总是低估平滑分布的边界。
- 把 N 拉到 20000，GS 的最大值从 3.65 升到 4.27（概率 $1/N$ 处的理论值分别约为 3.66 和 4.06），而 KV、WB 基本不变。高斯分布的"边界"只是样本数的函数。

## 统计：N 个粒子能说明多少 {#distributions-statistics}

模拟用几千个宏粒子代表几百万个真实粒子，就像民意调查用一千人代表全国。调查一千人，结果有几个百分点的误差；模拟几千个粒子，rms 也带着误差。

用 $N$ 个粒子估计 rms 尺寸，它的相对误差约为

$$
\frac{\Delta x_{\mathrm{rms}}}{x_{\mathrm{rms}}} \approx \frac{1}{2}\sqrt{\frac{\kappa - 1}{N}} ,
$$

（推导见[深入部分](#distributions-deep-statistics)）。两点直观：粒子数多 4 倍，误差减半；尾巴越重（峰度 $\kappa$ 越大），误差越大，因为 rms 被远处少数粒子左右。高斯为 $1/\sqrt{2N}$，KV 为 $1/(2\sqrt N)$。示例的 5260 个宏粒子，高斯束 rms 尺寸的统计误差约 0.97 %，KV 约 0.69 %；rms 发射度的统计误差与此同一量级。

尾部的分辨率更差：一个宏粒子就是 $1/N$ 的份额，示例中丢失一个宏粒子即 0.019 %。高斯样本的最大值约在"超出它的概率 ≈ $1/N$"处，随 $N$ 增长得很慢，大致按 $\sqrt{2\ln N}$ 的规律。要可靠地看到很小的损失，需要的粒子数多得惊人（见[想一想](#distributions-think)）；宏粒子数与统计误差的更多讨论见[多粒子模拟要点](#simulation)。

## 真实束流：示例里的 RFQ 出口分布 {#distributions-real-beam}

示例项目的 `InputFile/part_rfq.dst` 是一份 **RFQ**（射频四极加速器，把离子源出来的低能束流聚成束团并加速的那一段，示例中出口能量为 1.527 MeV）出口的粒子分布。RFQ 本身通常不由 AVAS 模拟：其他程序算好 RFQ 出口的粒子坐标，以这样的粒子文件（或由它算出的 rms 参数）交给 AVAS，作为模拟的起点（新内核另有一个尚未与其他程序核对的 `rfq` 元件，见[RFQ](#journey-rfq-in-avas)）。这份文件有 5260 个粒子、1.527 MeV、0.1841 mA、162.5 MHz，正是 `beam.txt` 里的数值；它的 $x$、$y$ 平面 rms Twiss 参数与 `twissx`、`twissy` 五位有效数字相同。也就是说，在横向平面上，示例的 `beam.txt` 是这份分布的 **rms 等效束**，再按 `distribution GS GS` 重新生成；纵向的 `twissz` 与文件并不完全一致（见[示例](#distributions-in-avas)）。

逐项统计这份文件（5260 个粒子）：

| 量 | RFQ 束 | KV | WB | PB | GS |
|---|---|---|---|---|---|
| $x$ 投影的峰度 $\kappa$ | 2.07 | 2.00 | 2.25 | 2.40 | 3 |
| $x'$ 投影的峰度 $\kappa$ | 2.45 | 2.00 | 2.25 | 2.40 | 3 |
| $x$ 超出 2 倍 rms 的比例 | 0.89 % | 0 | 2.50 % | 3.31 % | 4.55 % |
| $x$–$x'$：4 倍 rms 椭圆内 | 89.3 % | 100 % | 88.9 % | 87.5 % | 86.5 % |
| $x$–$x'$：6 倍 rms 椭圆内 | 98.8 % | 100 % | 100 % | 98.4 % | 95.0 % |
| $x$–$x'$：最外粒子所在椭圆 / rms | 8.1 | 4 | 6 | 8 | 无界 |

真实束流不属于四种中的任何一种：位置投影接近 KV，角度投影接近 PB，相平面上的包含比例落在 WB 与 PB 之间。这是 RFQ 中非线性场和空间电荷共同塑造的结果。重要的是与 GS 的差别：GS 认为有 5.0 % 的粒子在 6 倍 rms 椭圆之外，真实束只有 1.2 %。用 GS 代替它，线性光学和 rms 包络完全一样，尾巴却重得多——研究损失和束晕时，这一差别就是结论本身。

## 分布形状为什么在空间电荷下要紧 {#distributions-space-charge}

没有空间电荷、元件又是线性的时候，分布形状对 rms 演化毫无影响，只决定有多少粒子落在孔径外。空间电荷改变了这一点。

先看一根长长的、均匀带电的圆柱形束流。圆柱里离轴 $r$ 处的粒子被往外推，推力只来自它"里面"的电荷（外面那一圈圈电荷的作用互相抵消），而且和"里面的电荷 ÷ $r$"成正比。均匀束里面的电荷正比于截面积 $r^2$，所以推力正比于 $r^2/r = r$：**和离轴距离成正比，是线性力**，就像一块（散焦的）透镜，只改变包络，不改变发射度。

高斯束不一样：中心粒子最密，往外越来越稀。靠近轴的地方，推力也近似和 $r$ 成正比，但比 rms 相同的均匀束**强一倍**；远离轴的地方，"里面的电荷"几乎不再增加，推力按 $1/r$ 下降（计算见[深入部分](#distributions-deep-space-charge)）。所以高斯束受到的是**非线性**的推力：核心被推得猛，尾巴被推得弱。

非线性力让不同振幅的粒子以不同快慢摆动，相空间里的椭圆被扭成 S 形并逐渐**丝化**（像搅拌的奶油被拉成细丝），rms 发射度随之增长。从能量角度看，rms 尺寸相同时非均匀分布的电场能量比均匀分布高；束流趋向均匀时，多出的"自由能"转化为横向运动的能量，表现为发射度增长。这种电荷重新分布很快，大约在四分之一个等离子体振荡周期内完成，流强越强、初始分布越不均匀，增长越大（Wangler、Reiser）。匹配的 KV 束在实际空间本来就是均匀的，自场严格线性，没有可以释放的自由能，因此这一机制不会让它的发射度增长。

外场的非线性也会区分分布：射频腔的电场随相位按正弦变化，束团在纵向上越长的尾巴，感受到的非线性越强。纵向分布（`distribution` 的第二个参数）的选择对此敏感。

以示例束流为例：平均流强只有 0.18 mA，但电荷集中在一个个束团里。纵向 rms 长度 1.316 mm 对应时间宽度 $\sigma_t = 77.0$ ps，高斯束团的峰值流强约为 $Q/(\sqrt{2\pi}\,\sigma_t)$ = 5.9 mA，比平均值大 32 倍。空间电荷是否重要要看峰值流强，并与外聚焦比较，见[空间电荷](#space-charge)。

## 想一想 {#distributions-think}

> **想一想 1**：KV 分布的粒子全在一个四维"球壳"上，为什么它在 $x$–$x'$ 平面里看起来是一个实心的均匀椭圆，而不是一个环？
>
> 答：四维球面向二维平面投影时，被"压扁"掉的两个方向恰好补偿了球面的弯曲，投影密度是常数——就像普通球面上均匀分布的点投影到一条直径上是均匀的一样（阿基米德的结论）。[深入部分](#distributions-deep-shapes)的一般公式给出同一结论：投影指数为 $-1 + 1 = 0$。

> **想一想 2**：要可靠地看到 $10^{-4}$ 量级的损失，大约需要多少宏粒子？
>
> 答：$10^{-4}$ 乘以 $N$ 才是丢失的宏粒子数。想要它有 10 % 左右的统计精度（约 100 个丢失粒子，计数误差 $1/\sqrt{100}$），$N$ 需要 $10^6$ 量级；5260 个宏粒子时期望值只有 0.5 个，根本无法分辨。

> **想一想 3**：把示例的 `distribution GS GS` 改成 `WB WB`，结构页的线性预览会变吗？
>
> 答：不会。线性预览只推进二阶矩，不读分布类型。只有内核的多粒子模拟——在空间电荷、射频腔的正弦场和孔径作用下——才看得到形状的差别。

> **想一想 4**：前面的菱形束和方形束，在一段线性束线里走多远，rms 尺寸都保持相同吗？最远粒子的距离呢？
>
> 答：rms 尺寸处处相同，因为它只由二阶矩决定，而两束的二阶矩一开始就相同。最远粒子的距离则各走各的：1 m 漂移后菱形束 1.41 mm、方形束 2 mm。

## 容易误解的地方 {#distributions-misconceptions}

- **"Twiss 参数相同就是同一束流。"** Twiss 参数和 rms 发射度只确定二阶矩。线性光学下两者等价；涉及孔径、损失、空间电荷和非线性场时，分布形状同样重要。
- **"总发射度就是 4 倍 rms 发射度。"** 只有 KV 如此（在二维相平面中均匀的椭圆也是 4 倍）。四维 WB 为 6 倍、PB 为 8 倍、高斯没有总发射度。比值还取决于分布在几维空间中定义：同叫"水袋"，二维定义（$x$–$x'$ 平面内均匀）的比值是 4，四维定义是 6。
- **"样本的最大值就是分布的边界。"** 平滑分布的边缘密度极低，有限样本会低估边界（PB 在 4000 个粒子时只到 2.62 倍 rms，而不是 2.83）；高斯样本的最大值只反映样本数。
- **"高斯分布的 σ 就是 rms。"** 未截断时是。截断后 rms 变小：在每个坐标 $\pm 3\sigma$ 截断的一维高斯，rms 为 $0.987\sigma$；$\pm 2\sigma$ 截断时为 $0.880\sigma$。
- **"rms 发射度主要反映束流核心。"** 恰好相反：它按 $x^2$ 加权，被远处少数粒子主导（1 % 的粒子在 10 倍 rms 处即可让 rms 尺寸增大 41 %）。研究束晕要看百分比发射度或尾部比例。
- **"KV 是最理想、最真实的分布。"** KV 是理论上最方便的分布，不是真实束流的写照；它在强空间电荷下还有特有的不稳定性。
- **"归一化和几何发射度只差一个记号。"** `beam.txt` 里写的是归一化 rms 发射度；本章实验画的点云和给出的发射度是几何量，两者相差 $\beta\gamma$（示例中约 17.5 倍）。

## 深入：线性传输为什么只认二阶矩 {#distributions-deep-moments}

正文用漂移段和四个粒子说明了"线性传输只认二阶矩"。这一节把它推广到任意线性元件，并证明 rms 发射度不变。前四步只用中学代数，最后一段给学过线性代数的读者。

### 第一步：平均是"线性"的

取平均有两条简单的规则：和的平均等于平均的和，常数可以提到平均外面：

$$
\langle a\,u + b\,v\rangle = a\,\langle u\rangle + b\,\langle v\rangle .
$$

例：两个粒子 $u = 1, 3$，$v = 2, 0$，取 $a = 2$、$b = 5$：$2u + 5v$ 是 12 和 6，平均 9；$2\langle u\rangle + 5\langle v\rangle = 2 \times 2 + 5 \times 1 = 9$。

### 第二步：任意线性元件

任何线性元件都可以写成 $x = C x_0 + S x_0'$、$x' = C' x_0 + S' x_0'$（$C$、$S$、$C'$、$S'$ 是这个元件的四个系数，对所有粒子都一样，见[线性光学](#linear-optics-matrices)）。漂移段是 $C = 1$、$S = L$、$C' = 0$、$S' = 1$。

### 第三步：平方再取平均

把 $x$ 平方，用完全平方公式，再用第一步把系数提出来：

$$
\begin{aligned}
\langle x^2\rangle &= C^2\langle x_0^2\rangle + 2CS\,\langle x_0x_0'\rangle + S^2\langle x_0'^2\rangle,\\
\langle x x'\rangle &= CC'\langle x_0^2\rangle + (CS' + SC')\,\langle x_0x_0'\rangle + SS'\langle x_0'^2\rangle,\\
\langle x'^2\rangle &= C'^2\langle x_0^2\rangle + 2C'S'\,\langle x_0x_0'\rangle + S'^2\langle x_0'^2\rangle .
\end{aligned}
$$

右边只出现入口的三个二阶矩。分布的形状（更高阶的矩，例如 $\langle x_0^4\rangle$）根本没有机会进来。

### 第四步：rms 发射度不变

记入口 $A = \langle x_0^2\rangle$、$B = \langle x_0x_0'\rangle$、$D = \langle x_0'^2\rangle$。把第三步代入 $\varepsilon^2 = \langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2$，展开整理（耐心乘开即可），所有交叉项相消，剩下

$$
\varepsilon_1^2 = (CS' - SC')^2\,(AD - B^2) = (CS' - SC')^2\,\varepsilon_0^2 .
$$

$CS' - SC'$ 是元件的行列式，没有加速的线性元件里它等于 1（见[线性光学：辛性](#linear-optics-symplectic)），所以 $\varepsilon_1 = \varepsilon_0$。验算正文的菱形束：入口 $A = D = 1$、$B = 0$，$\varepsilon = 1$；1 m 漂移后 $\langle x^2\rangle = 2$、$\langle x'^2\rangle = 1$、$\langle xx'\rangle = (0 + 0 + 2 + 2)/4 = 1$，$\varepsilon^2 = 2 \times 1 - 1 = 1$，仍是 1。方形束同样得 1。

### 第五步：空间电荷里的 rms

Sacherer 的结论说的是：在 rms 尺寸的方程里，空间电荷以 $\langle x E_x\rangle$（位置乘电场的平均）的形式出现；对横截面椭圆对称的连续束，这个平均只取决于 rms 尺寸，与密度剖面无关。所以 rms 包络方程对任何这样的分布都一样，可以用 rms 等效的均匀束来算。它不保证 rms 发射度不变：非线性的那部分力仍会让发射度增长，见正文。

### 给学过线性代数的读者

把二阶矩排成 σ 矩阵 $\Sigma = \langle \boldsymbol u \boldsymbol u\tp\rangle$，线性传输 $\boldsymbol u_1 = M\boldsymbol u_0$ 给出

$$
\Sigma_1 = \langle \boldsymbol u_1 \boldsymbol u_1\tp\rangle = M\,\langle \boldsymbol u_0 \boldsymbol u_0\tp\rangle\,M\tp = M\,\Sigma_0\,M\tp ,
$$

这一步只用到"平均是线性运算、$M$ 对所有粒子相同"，与分布形状无关。又因为 $\det\Sigma_1 = (\det M)^2\det\Sigma_0$，而无耗散的线性传输满足 $\det M = 1$，rms 发射度 $\varepsilon = \sqrt{\det\Sigma}$（二维投影）对任何形状的分布都是线性运动的不变量，详见[σ 矩阵](#phase-space-sigma)。

## 深入：四种分布的边界和尾部 {#distributions-deep-shapes}

正文的表给出了四种分布的边界、尾部比例和峰度。这一节从归一化坐标出发，一步步算出表里的数。前五步只用中学代数和幂函数的定积分，最后一段给学过微积分的读者。

### 第一步：把椭圆变成圆

Twiss 参数给出一组**归一化坐标**：

$$
X = \frac{x}{\sqrt{\beta_x}}, \qquad X' = \frac{\alpha_x x + \beta_x x'}{\sqrt{\beta_x}} .
$$

把它们平方相加，用 $\gamma_x = (1 + \alpha_x^2)/\beta_x$：

$$
X^2 + X'^2 = \frac{x^2 + \alpha_x^2x^2 + 2\alpha_x\beta_x xx' + \beta_x^2x'^2}{\beta_x} = \gamma_x x^2 + 2\alpha_x x x' + \beta_x x'^2 .
$$

右边正是粒子在 $x$ 平面的 Courant–Snyder 不变量（振幅的平方）。在 $(X, X')$ 坐标里 rms 椭圆变成半径 $\sqrt{\varepsilon}$ 的圆，并且 $\langle X^2\rangle = \langle X'^2\rangle = \varepsilon$。

### 第二步：用"总振幅"定义分布

把两个横向平面放在一起，定义四维"半径"

$$
r^2 = \frac{X^2 + X'^2 + Y^2 + Y'^2}{a^2},
$$

$a$ 为边界尺度。只依赖 $r^2$ 的分布，说的就是"有多少粒子具有多大的总横向振幅"。KV 是全部粒子在球壳 $r^2 = 1$ 上；WB 是 $r^2 \le 1$ 内均匀；PB 是密度 $\propto 1 - r^2$；GS 是密度 $\propto \exp(-R^2/2)$（$R$ 以 rms 为单位）。

### 第三步：边界是 rms 的几倍

在四维里，半径 $\le r$ 的"球"体积正比于 $r^4$（二维圆面积 $\propto r^2$，三维球体积 $\propto r^3$，依此类推），所以半径在 $r$ 附近一层薄壳的体积正比于 $r^3$。由此算 $\langle r^2\rangle$，再按对称性平分给四个坐标：$\langle X^2\rangle = a^2\langle r^2\rangle/4$。

- **KV**：所有粒子 $r^2 = 1$，$\langle r^2\rangle = 1$，$\langle X^2\rangle = a^2/4$，边界 / rms $= a/(a/2) = 2$。
- **WB**：密度均匀，按薄壳加权，$\langle r^2\rangle = \dfrac{\int_0^1 r^2\cdot r^3\,\dd r}{\int_0^1 r^3\,\dd r} = \dfrac{1/6}{1/4} = \dfrac23$，$\langle X^2\rangle = a^2/6$，边界 / rms $= \sqrt6 = 2.449$。
- **PB**：再乘密度 $1 - r^2$，$\langle r^2\rangle = \dfrac{\int_0^1 r^2(1 - r^2)\,r^3\,\dd r}{\int_0^1 (1 - r^2)\,r^3\,\dd r} = \dfrac{1/6 - 1/8}{1/4 - 1/6} = \dfrac12$，$\langle X^2\rangle = a^2/8$，边界 / rms $= \sqrt8 = 2.828$。

这里只用到幂函数的定积分 $\int_0^1 r^n\,\dd r = 1/(n+1)$。包住全部粒子的椭圆在归一化坐标里半径是 $a$，"面积"$a^2$；rms 椭圆是 $\langle X^2\rangle$。两者之比 $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}} = 4, 6, 8$。

### 第四步：投影——阿基米德的帽盒

我们看到、测到的是一维剖面和二维相图，要把四维分布"压扁"到低维。阿基米德发现：球面上均匀分布的点，投影到一条直径上是**均匀**的（夹在两个平行平面之间的球带面积只取决于两平面的间距）。一般结果是：$d$ 维空间中密度 $\propto (1 - r^2)^k$ 的分布，压掉 $d - d'$ 个方向后，投影密度 $\propto (1 - r^2)^{k + (d - d')/2}$，每压掉一个方向，指数加 $\tfrac12$。KV 的球壳相当于 $k = -1$ 的极限，WB 为 $k = 0$，PB 为 $k = 1$。

- 投影到一维 $x$（压掉 3 个方向）：

$$
\rho(x) \propto \left(1 - \frac{x^2}{a^2}\right)^{m}, \qquad m = \tfrac12,\ \tfrac32,\ \tfrac52 \quad (\mathrm{KV},\ \mathrm{WB},\ \mathrm{PB}) .
$$

- 投影到二维 $x$–$x'$（压掉 2 个方向），指数为 $k + 1$：KV 是均匀椭圆（指数 $-1 + 1 = 0$，正是[想一想 1](#distributions-think)），WB $\propto 1 - r^2$，PB $\propto (1 - r^2)^2$。

### 第五步：椭圆里装了多少粒子

在二维投影里，用 $t = $ 椭圆面积 ÷ 总发射度（$t = n/n_{\max}$，$n_{\max}$ = 4、6、8），落在 $n$ 倍 rms 椭圆内的比例是对密度 $(1 - s)^{k+1}$ 从 0 积到 $t$（$s$ 是以 $a^2$ 为单位的半径平方，二维里按 $s$ 均匀加权）：

$$
\frac{\int_0^t (1 - s)^{k+1}\,\dd s}{\int_0^1 (1 - s)^{k+1}\,\dd s} = 1 - \left(1 - \frac{n}{n_{\max}}\right)^{k+2} .
$$

验算 $n = 4$：KV $1 - (1 - 1)^1 = 100\ \%$；WB $1 - (1 - 4/6)^2 = 88.9\ \%$；PB $1 - (1 - 4/8)^3 = 87.5\ \%$。$n = 6$ 时 PB 为 $1 - (1/4)^3 = 98.4\ \%$。高斯没有边界，比例是 $1 - \ee^{-n/2}$：$n = 4$ 时 86.5 %，$n = 6$ 时 95.0 %。一维投影里"超出 2 倍 rms"的比例（WB 2.50 %、PB 3.31 %、GS 4.55 %）要对 $(1 - u^2)^m$ 积分，结果直接列在表里。

### 给学过微积分的读者

一般地 $\int_{-1}^{1} u^2 (1 - u^2)^m\,\dd u \big/ \int_{-1}^{1} (1 - u^2)^m\,\dd u = 1/(2m + 3)$，所以

$$
\langle x^2\rangle = \frac{a^2}{2m + 3} = \frac{a^2}{4},\ \frac{a^2}{6},\ \frac{a^2}{8}
\quad\Longrightarrow\quad
\frac{x_{\max}}{x_{\mathrm{rms}}} = 2,\ \sqrt6,\ \sqrt8 ,
$$

同理 $\langle u^4\rangle/\langle u^2\rangle^2 = 3(2m + 3)/(2m + 5)$，给出峰度 2、2.25、2.4；$m \to \infty$ 时趋于高斯的 3。投影指数的规律来自对 $d - d'$ 维球的体积积分 $\int (1 - r^2 - \rho^2)^k\,\dd^{d-d'}\rho \propto (1 - r^2)^{k + (d - d')/2}$。高斯的比例：二维高斯在归一化坐标里 $J = X^2 + X'^2$ 服从指数分布，$P(J \le n\varepsilon) = 1 - \ee^{-n/2}$。

## 深入：统计误差从哪里来 {#distributions-deep-statistics}

正文给出了 rms 的统计误差 $\tfrac12\sqrt{(\kappa - 1)/N}$。这一节从中学统计的"平均值的误差"出发推出它。

### 第一步：平均值的误差按 √N 缩小

掷一枚硬币，正面记 1、反面记 0，一次的结果分散程度（标准差）是 0.5。掷 100 次求平均（正面比例），平均值的分散程度缩小为 $0.5/\sqrt{100} = 0.05$：大约在 45 %～55 % 之间。一般地，$N$ 个独立样本的平均值，标准差是单个样本标准差的 $1/\sqrt N$（中学学过：方差按 $1/N$ 缩小）。

### 第二步：估计 ⟨x²⟩ 就是对 x² 取平均

估计 $\langle x^2\rangle$，就是把每个粒子的 $x^2$ 加起来除以 $N$。单个粒子的 $x^2$ 分散程度是 $\sqrt{\langle x^4\rangle - \langle x^2\rangle^2}$（"平方的平均减平均的平方"就是方差）。用峰度 $\kappa = \langle x^4\rangle/\langle x^2\rangle^2$ 写出来，它等于 $\langle x^2\rangle\sqrt{\kappa - 1}$。按第一步，$N$ 个粒子求平均后：

$$
\frac{\Delta\langle x^2\rangle}{\langle x^2\rangle} \approx \sqrt{\frac{\kappa - 1}{N}} .
$$

### 第三步：开方让相对误差减半

rms 是 $\langle x^2\rangle$ 的平方根。一个数大了 2 %，它的平方根只大约 1 %：$\sqrt{1.02} = 1.00995$。一般地 $\sqrt{1 + \epsilon} \approx 1 + \epsilon/2$，所以 rms 的相对误差是上式的一半，就是正文的公式。

### 第四步：代入数字

高斯 $\kappa = 3$：$\tfrac12\sqrt{2/N} = 1/\sqrt{2N}$，$N = 5260$ 时 $1/\sqrt{10520} = 0.97\ \%$。KV $\kappa = 2$：$\tfrac12\sqrt{1/N} = 1/(2\sqrt N) = 0.69\ \%$。所以判断某个参数对 rms 的影响时，差别要明显大于约 1 % 才可靠（不同随机种子之间的差别也是这个量级）。

### 第五步：数尾部的粒子

丢失粒子要一个一个地数。期望丢失 $Np$ 个（$p$ 是丢失比例）时，实际数到的个数上下起伏约 $\sqrt{Np}$（计数的泊松起伏）。所以相对误差是 $1/\sqrt{Np}$：要 10 % 的精度，需要 $Np \approx 100$。$p = 10^{-4}$ 时 $N \approx 10^6$。高斯样本的最大值约在 $P(\lvert x\rvert > x_{\max}) \approx 1/N$ 处，这个概率随 $x_{\max}$ 下降得极快，所以 $x_{\max}$ 随 $N$ 增长得很慢，大致按 $\sqrt{2\ln N}$ 的规律。

### 给学过概率论的读者

$\langle x^2\rangle$ 的样本估计 $\frac1N\sum x_i^2$ 的方差为 $(\mu_4 - \mu_2^2)/N$；由 delta 方法，$\sqrt{\cdot}$ 的方差乘以 $(1/(2\sqrt{\mu_2}))^2$，相对标准差即 $\tfrac12\sqrt{(\kappa - 1)/N}$。这里假设粒子独立抽样、中心已知；用样本均值中心化只带来 $O(1/N)$ 的修正。rms 发射度涉及三个二阶矩，误差同量级但与 $x$–$x'$ 关联有关。

## 深入：高斯束的空间电荷场 {#distributions-deep-space-charge}

正文说"高斯束核心受到的空间电荷散焦是 rms 等效均匀束的两倍"。这一节一步步算出这个"两倍"，以及示例束团的峰值流强。

### 第一步：长直带电束的电场

一根很长的带电圆柱，单位长度的电荷记作 $\lambda$。离轴 $r$ 处的径向电场只由半径 $r$ 以内的电荷 $\lambda_{\text{内}}$ 决定：可以想象电场线从里面的电荷均匀地向四周发出，穿过周长为 $2\pi r$ 的圆圈，所以

$$
E_r = \frac{\lambda_{\text{内}}}{2\pi\varepsilon_0\,r},
$$

$\varepsilon_0$ 是真空介电常数。（外面的电荷对里面的粒子作用相互抵消，这一点和"均匀球壳内部没有引力"同一个道理；严格的论证叫高斯定理。）

### 第二步：均匀束

半径 $a$ 的均匀束，$r$ 以内的电荷占 $r^2/a^2$，所以 $E_r = \lambda r/(2\pi\varepsilon_0 a^2)$，**正比于 $r$**：线性力。均匀圆束的 rms 尺寸是 $a/2$：截面上 $\langle r^2\rangle = \int_0^a r^2\cdot r\,\dd r \big/ \int_0^a r\,\dd r = a^2/2$，平分给 $x$、$y$ 两个方向得 $\langle x^2\rangle = a^2/4$。所以 rms 为 $\sigma$ 的均匀束 $a = 2\sigma$，内部

$$
E_r = \frac{\lambda\,r}{2\pi\varepsilon_0\cdot 4\sigma^2} .
$$

### 第三步：高斯束

每个方向 rms 为 $\sigma$ 的圆高斯束，$r$ 以内的电荷是 $\lambda\,(1 - \ee^{-r^2/2\sigma^2})$：

$$
E_r = \frac{\lambda}{2\pi\varepsilon_0}\,\frac{1 - \ee^{-r^2/2\sigma^2}}{r}
\;\approx\; \frac{\lambda}{2\pi\varepsilon_0}\,\frac{r}{2\sigma^2}\quad (r \ll \sigma) .
$$

近似用的是 $\ee^{-u} \approx 1 - u$（$u$ 很小时；例如 $\ee^{-0.01} = 0.99005$）。中心附近线性，远处 $1 - \ee^{-r^2/2\sigma^2} \to 1$，按 $1/r$ 下降。

### 第四步：比一比

中心附近，高斯束的 $E_r/r = \lambda/(2\pi\varepsilon_0\cdot 2\sigma^2)$，均匀束是 $\lambda/(2\pi\varepsilon_0\cdot 4\sigma^2)$：**高斯束核心受到的空间电荷散焦梯度是 rms 等效均匀束的两倍**，而尾巴受到的力弱得多。正是这种"核心强、尾巴弱"的非线性让 rms 发射度增长。

### 第五步：示例束团的峰值流强

示例束流的纵向 rms 长度 $\sigma_z = 1.316$ mm，速度 $v = \beta c = 0.0570 \times 2.998\times10^{8} = 1.708\times10^{7}$ m/s，换成时间 $\sigma_t = \sigma_z/v = 77.0$ ps。高斯束团的电荷 $Q = 1.133$ pC 集中在约 $\sqrt{2\pi}\,\sigma_t$ 的时间里，峰值流强

$$
I_{\text{peak}} \approx \frac{Q}{\sqrt{2\pi}\,\sigma_t} = \frac{1.133\ \mathrm{pC}}{2.507 \times 77.0\ \mathrm{ps}} = 5.9\ \mathrm{mA},
$$

是平均流强 0.184 mA 的 32 倍。

### 给学过电磁学的读者

第一步就是二维高斯定理 $\oint \boldsymbol E\cdot\dd\boldsymbol l_\perp = \lambda_{\text{内}}/\varepsilon_0$。Sacherer 的结论来自 $\langle x E_x\rangle = \lambda/(4\pi\varepsilon_0)\cdot\sigma_x/(\sigma_x + \sigma_y)$ 对任何椭圆对称分布成立；自由能论证见延伸阅读中 Reiser 的书（非均匀分布与均匀分布的场能差在约四分之一等离子体周期内转化为横向热能）。

## 在 AVAS 中 {#distributions-in-avas}

### 两种初始束流

AVAS 的初始束流有两种来源，由束流页的"使用粒子文件"开关（`use_dst`）选择：

1. **按参数生成**：由 `twissx`、`twissy`、`twissz` 给出三个平面的 rms 参数，`distribution V1 V2` 选择分布类型，`particlenumber` 给出宏粒子数，内核用随机数 `randomseed` 抽样。
2. **读入粒子文件**：`readparticledistribution` 指定已有的 `.dst` 或 `.edst` 文件（例如其他程序计算的 RFQ 出口分布）。此时除 `numofcharge` 外，`beam.txt` 的其他关键字都不生效；`.edst` 可以描述多种粒子组成的混合束。

```avas-params
beam:distribution beam:twissx beam:twissy beam:twissz beam:particlenumber beam:randomseed beam:readparticledistribution beam:use_dst beam:initpos beam:initmom beam:numofcharge beam:beamtype
```

### 关键字

- `distribution V1 V2`：V1 为横向（$x$、$y$ 两个平面共用）分布，V2 为纵向分布，可选 `KV`、`GS`、`PB`、`WB`。
- `twissx α β ε`（`twissy`、`twissz` 同）：$\alpha$ 无量纲，$\beta$ 单位 mm/mrad，$\varepsilon$ 为**归一化 rms** 发射度（π·mm·mrad）；$z$ 平面的"角度"是 $\delta = \Delta p/p$（以 mrad 计），同样乘 $\beta\gamma$ 归一化。单位与换算见[相空间：在 AVAS 中](#phase-space-in-avas)。
- `particlenumber`：宏粒子数。写 1 时为单粒子模式，此时 `twiss*`、`current`、`distribution` 不生效。每个宏粒子的电荷为 $Q/N$，荷质比与真实粒子相同，所以外场中的运动不变，只有空间电荷按总电荷计算。示例 5260 个宏粒子，每个代表约 1344 个质子。
- `randomseed`：生成初始束流的随机数种子。原内核和新内核的兼容模式（compat = dll）在 `randomseed` 不为 0 时有缺陷：生成的分布中 $x$ 与相位完全相关（相关系数 −1），可以从 `inData.dst` 中看出来；模拟设置页会在随机种子旁警告。新内核的默认模式没有这个缺陷。
- `initpos x y z`（mm）、`initmom x' y' z'`（mrad）：给整个束团加位置和角度偏移（改变一阶矩），用来研究初始偏心。
- `numofcharge`：电荷数；读入粒子文件时它仍然有效。
- `beamtype`：写 `dc` 时生成直流束，`notdc` 为束团。
- `use_dst`：界面使用的开关（手册未收录），1 表示读取 `readparticledistribution` 指定的文件，0 表示按参数生成。

> **AVAS 约定**：手册没有说明 AVAS 生成 KV、WB、PB 时在几维空间中定义、高斯分布在哪里截断，以内核为准。本章实验按教科书在四维横向相空间中的定义抽样。需要准确知道时，读取运行输出目录中的 `inData.dst`——模拟实际使用的初始分布——直接统计。

> **一次运行的观察（手册未说明）**：下面的统计来自本仓库之外、一个较早的 AVAS 示例目录中的一份 `inData.dst`（5260 个粒子）；它的 `beam.txt` 与示例相同（`distribution GS GS`，只少了 `beamtype` 一行），生成它的内核版本没有记录。这些数字说明的是那一次运行的结果，不是文档规定的行为；需要时请统计自己运行的 `inData.dst`。该文件中各坐标投影的峰度为 2.88～3.05，超出 2 倍 rms 的比例为 4.3 %～4.6 %，最远粒子在 3.5～4.0 倍 rms，二维、四维、六维振幅的分布与不截断的高斯一致，三个平面之间没有可见的关联。在这一样本量下看不出截断。那次生成的结果**没有**重新缩放到精确的输入值：$x$ 平面 rms 发射度为 0.1361（输入 0.13768，小 1.1 %），$\beta_x$ 为 0.3796（大 0.6 %），$\alpha_x$ 为 −0.672（大 2.1 %），都在 5260 个粒子的统计涨落之内。纵向按 $(\varphi, W)$ 换算回 $(z, \delta)$ 后得到 $\alpha_z = 0.053$、$\beta_z = 0.378$、$\varepsilon_z = 0.262$，与 `twissz` 的约定一致。

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

$x$、$y$ 平面的 Twiss 参数就是 `part_rfq.dst` 的 rms 值。纵向不同：文件统计得到 $\beta_z$ = 0.3326 mm/mrad、$\varepsilon_z$ = 0.2659 π·mm·mrad，而 `twissz` 写的 $\beta_z$ = 0.37292，恰与 `twissy` 的 $\beta$ 相同；两者对应的 rms 束长分别为 1.245 mm 和 1.316 mm。这只是对示例文件的观察，`twissz` 为何这样填写没有记录。要让模拟从真实的 RFQ 分布出发，把 `use_dst` 设为 1、`readparticledistribution` 设为 `part_rfq.dst` 即可。束流页可以从粒子文件计算 rms 参数并填入 Twiss 栏。

### 需要注意

- 线性预览只用 rms 参数：按参数生成时读 `twiss*`，读入 `.dst` 时用文件的 rms Twiss 参数；`.edst` 文件预览不读取，改用 `twiss*` 并给出提示。分布类型不影响预览。
- 比较不同程序的结果时，要保证初始分布一致——不只是 Twiss 参数，还有分布类型、维数定义和截断。最稳妥的做法是交换粒子文件。
- 宏粒子数有限，初始束流本身就带着[统计误差](#distributions-statistics)；判断某个参数的影响时，差别要大于这一误差才可靠。

## 延伸阅读 {#distributions-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：KV 与其他平稳分布、rms 发射度与 rms 等效束、自由能与发射度增长。
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：空间电荷一章中的电荷重新分布与发射度增长。
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1101。
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1105。
- I. M. Kapchinskij, V. V. Vladimirskij, *Proc. Int. Conf. on High Energy Accelerators and Instrumentation*, CERN, 1959：KV 分布的原始论文。
- AVAS 使用说明 `docs/使用说明20260427.docx`：`beam.txt` 关键字。
