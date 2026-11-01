# 束流匹配

周期结构有自己的"本征"相椭圆——周期 Twiss 参数。入口束流的相椭圆与它一致时，束流包络在每个周期重复；不一致时包络在周期包络附近振荡（失配），在非线性力和空间电荷作用下转化为发射度增长和束晕。本章给出匹配的定义、失配因子、包络振荡模式、匹配所需的旋钮数，以及在 AVAS 中做匹配的方法。

记号沿用 [相空间](#phase-space) 与 [线性光学](#linear-optics)：Twiss 参数 $(\alpha, \beta, \gamma)$ 满足 $\beta\gamma - \alpha^2 = 1$，rms 几何发射度 $\varepsilon$，束流 σ 矩阵

$$
\Sigma = \varepsilon \begin{pmatrix} \beta & -\alpha \\ -\alpha & \gamma \end{pmatrix},\qquad x_{\mathrm{rms}} = \sqrt{\beta\varepsilon}
$$

## 匹配束的定义 {#matching-definition}

### 零流周期解

设一个周期的 2×2 传输矩阵为 $M$，$\cos\sigma = \tfrac12\Tr M$ 且 $\lvert\cos\sigma\rvert < 1$。把 $M$ 写成 Twiss 形式

$$
M = \begin{pmatrix} \cos\sigma + \alpha\sin\sigma & \beta\sin\sigma \\ -\gamma\sin\sigma & \cos\sigma - \alpha\sin\sigma \end{pmatrix}
$$

即可读出周期解：

$$
\beta = \frac{M_{12}}{\sin\sigma},\qquad \alpha = \frac{M_{11} - M_{22}}{2\sin\sigma},\qquad \gamma = -\frac{M_{21}}{\sin\sigma}
$$

$\sin\sigma$ 的符号取与 $M_{12}$ 相同，使 $\beta > 0$。这组 $(\alpha, \beta)$ 满足 $M\Sigma M\tp = \Sigma$：以它为 Twiss 参数的束流经过一个周期后相椭圆不变，这就是**匹配束**。周期解依赖于取周期起点的位置；在 FODO 的 F 铁中心 $\alpha = 0$、$\beta$ 最大，D 铁中心 $\alpha = 0$、$\beta$ 最小。

### 有空间电荷时

空间电荷力依赖于束流尺寸本身，匹配解必须自洽。对连续束（KV 模型，$a$、$b$ 为边缘半径，$a = 2x_{\mathrm{rms}}$）包络方程为

$$
a'' + k_x(s)\,a - \frac{2K}{a+b} - \frac{\varepsilon_x^2}{a^3} = 0,\qquad
b'' + k_y(s)\,b - \frac{2K}{a+b} - \frac{\varepsilon_y^2}{b^3} = 0
$$

其中 $\varepsilon$ 为边缘发射度（$4\varepsilon_{\mathrm{rms}}$），$K$ 为广义导流系数（见 [空间电荷](#space-charge)）。匹配束是这对方程具有周期 $L_p$ 的解，随流强改变；零流匹配参数用在强流束上会产生失配。束团束的三维情况类似，纵向包络也要匹配。

### 加速时

加速使 $\beta\gamma$ 增大，几何发射度按 $1/(\beta\gamma)$ 绝热减小，周期解也随能量缓慢变化。此时"匹配"指包络跟随缓变的周期解，而不是严格逐周期重复。AVAS 的 `beam.txt` 中 Twiss 参数是**入口处**的值（β 单位 mm/mrad，发射度为归一化 rms，π·mm·mrad），入口 rms 尺寸为 $\sqrt{\beta\varepsilon_n/(\beta\gamma)}$。

## 失配因子 {#matching-mismatch-factor}

比较束流 $(\alpha, \beta, \gamma)$ 与匹配椭圆 $(\alpha_0, \beta_0, \gamma_0)$（同一位置、相同发射度），定义

$$
R = \beta\gamma_0 + \beta_0\gamma - 2\alpha\alpha_0,\qquad
M = \left[\frac{1}{2}\left(R + \sqrt{R^2 - 4}\right)\right]^{1/2} - 1
$$

这是 Wangler 书中及常用包络程序采用的定义。性质：

- $R \ge 2$，当且仅当两椭圆重合时 $R = 2$、$M = 0$。
- $R = \Tr(S S_0^{-1})$，其中 $S = \begin{pmatrix}\beta & -\alpha\\ -\alpha & \gamma\end{pmatrix}$。线性输运中 $S \to MSM\tp$，迹不变，所以**无空间电荷的线性输运中失配因子沿束线守恒**，可以在任意位置计算。
- 几何意义：在匹配椭圆的归一化坐标中，匹配椭圆是半径 $\sqrt{\varepsilon}$ 的圆，失配椭圆的两个半轴为 $\sqrt{\lambda_\pm\varepsilon}$，$\lambda_\pm = \tfrac12(R \pm \sqrt{R^2-4})$，$\lambda_+\lambda_- = 1$（面积相同）。所以 $1 + M = \sqrt{\lambda_+}$ 是失配椭圆在最不利方向上超出匹配椭圆的倍数；要用匹配形状的椭圆完全包住失配束，需要 $(1+M)^2\varepsilon$ 的发射度。
- 无空间电荷时，失配束的包络在匹配包络的 $1/(1+M)$ 到 $1+M$ 倍之间拍振。

例（$\alpha_0 = \alpha = 0$ 处）：$\beta = 1.2\beta_0$ 时 $R = 1.2 + 1/1.2$，$M = \sqrt{1.2} - 1 = 0.095$；$\beta = 1.44\beta_0$（尺寸大 20 %）时 $M = 0.200$。若 $\beta = \beta_0$ 而 $\alpha$ 偏 0.5，$M = 0.281$——α 的误差同样重要。

一个常用的小量结果：匹配束经过一个薄透镜误差 $\Delta(1/f)$（例如四极铁梯度误差）后，$\beta$ 不变、$\alpha \to \alpha + \beta\,\Delta(1/f)$，代入得 $R = 2 + \beta^2\Delta(1/f)^2$，于是

$$
M \approx \frac{\beta\,\Delta(1/f)}{2}
$$

经验上 $M \lesssim 0.05$–$0.1$ 被视为良好匹配，但允许值取决于流强和孔径裕量（见 [误差与校正](#errors)）。

## 包络振荡模式 {#matching-envelope-modes}

把包络方程在匹配解附近线性化，可得到失配振荡的本征模。以均匀聚焦 $k_0^2$、圆束 $a = b$、$\varepsilon_x = \varepsilon_y$ 为例，匹配条件 $k_0^2 a = K/a + \varepsilon^2/a^3$，记有空间电荷时粒子的波数 $k = \varepsilon/a^2$：

- **呼吸模**（$\delta a = \delta b$，两平面同相）：$k_{\mathrm{b}}^2 = 2k_0^2 + 2k^2$；
- **四极模**（$\delta a = -\delta b$，两平面反相）：$k_{\mathrm{q}}^2 = k_0^2 + 3k^2$。

推导要点：反相扰动下 $2K/(a+b)$ 一阶不变，所以四极模只受 $k_0^2$ 和发射度项 $3\varepsilon^2/a^4 = 3k^2$ 作用；同相扰动下空间电荷项贡献 $K/a^2 = k_0^2 - k^2$。零流时 $k = k_0$，两模都为 $2k_0$；空间电荷极强时 $k \to 0$，$k_{\mathrm{b}} \to \sqrt2\,k_0$，$k_{\mathrm{q}} \to k_0$。轴对称束团束的三维情况有三个模：横向四极模，以及由横向呼吸与纵向振荡耦合而成的高频模和低频模。

失配的物理后果：

1. 包络振荡频率与粒子频率不同，粒子在振荡的核周围受到周期驱动；非线性空间电荷力使相空间丝化，最终 rms 发射度增长。Reiser 用"自由能"的观点估计失配能量全部转化时的发射度增长，失配越大、空间电荷越强，增长越大。
2. 在周期聚焦中，包络模式每周期相移接近 $180^\circ$ 时发生包络不稳定性——这就是 [聚焦周期设计](#lattice-design-phase-advance) 中 $\sigma_0 < 90^\circ$ 规则的来源。
3. 粒子与振荡核的参数共振把部分粒子推到大振幅，形成束晕（见 [rms 匹配与束晕](#matching-halo)）。

## 匹配段：旋钮与约束 {#matching-knobs}

### 约束数

完整的 6D 匹配要求在目标位置满足

| 平面 | 约束 | 个数 |
|---|---|---|
| x | $\alpha_x$, $\beta_x$ | 2 |
| y | $\alpha_y$, $\beta_y$ | 2 |
| z | $\alpha_z$, $\beta_z$ | 2 |

发射度由束流本身决定，不是匹配变量。因此至少需要与约束数相同的**独立**旋钮：

- 横向用四极铁：至少 4 块（4 个梯度）匹配 $(\alpha_x, \beta_x, \alpha_y, \beta_y)$；实际常用 4–6 块，多出的自由度用来限制最大 β、梯度上限和对误差的敏感度。
- 螺线管对两个平面作用相同：一块螺线管只能同时改变 x、y 两个平面，对圆束只有 $(\alpha, \beta)$ 两个约束，至少需要 2 块螺线管；它不能把 x、y 匹配到不同的值。
- 纵向用聚束腔：在 $\varphi_s = -90^\circ$ 运行的聚束腔不增能，只给出与相位偏差成正比的能量偏差，相当于纵向薄透镜；至少 2 个聚束腔（幅值为旋钮）匹配 $(\alpha_z, \beta_z)$。也可以用下游加速段的头几个腔。

### 实用规则

- 匹配段内 β 不宜过大：大 β 处束流大，对四极铁位移和梯度误差也更敏感（失配 $M \approx \beta\,\Delta(1/f)/2$）。
- 有空间电荷时匹配参数依赖流强，要用实际流强迭代；零流结果只是初值。
- 匹配的目标是下游周期在其入口的周期 Twiss 参数，而不是任意的"小束斑"。
- 非线性问题可能有多个解，应检查整个匹配段的包络是否合理，而不仅是终点参数。

## 段与段之间的匹配 {#matching-sections}

段间匹配的一般流程：

1. 求下游段入口处的周期匹配参数（零流时用上一节公式；有空间电荷时用自洽包络或多粒子模拟）。
2. 确定上游束流在匹配段入口的 Twiss 参数（来自上游模拟或测量）。
3. 用匹配段的旋钮把 2 变换到 1，同时满足梯度、孔径限制。
4. 用多粒子模拟检查下游若干周期内包络是否呈周期形状、发射度是否增长。

典型场景是 RFQ 出口到第一个加速段之间的 MEBT：RFQ 出口束流在横向通常强烈汇聚或发散、两平面不对称，纵向需要聚束腔控制束团长度。若在 [lattice 设计](#lattice-design-transitions) 时让两段的每米相移连续，段间周期参数接近，匹配段可以很短甚至省略。

> 注意：在有加速的段中，下游第一个周期的"周期解"本身随能量变化，严格的周期匹配参数不存在。通常对第一个周期按其入口能量下的局部周期解匹配，再用模拟检查。

## rms 匹配与束晕 {#matching-halo}

rms 匹配只保证二阶矩（rms 尺寸、Twiss 参数）正确。实际分布的尾部还受以下因素影响：

- **粒子–核共振**：失配核以包络呼吸频率振荡，单个粒子穿过核时受周期性的非线性力；振荡频率约为核频率一半的粒子被共振激励到远大于核的振幅。粒子–核模型（Gluckstern；Wangler 等）给出的最大晕振幅随失配增大而增大，近似按失配的对数增长；只要存在失配，强流束就会形成晕。
- **非线性外场**：场图中的高阶分量、螺线管和腔的边缘场使相椭圆畸变。
- **初始分布**：高斯分布的尾部本身就延伸到多个 rms 之外，KV 分布没有尾部；AVAS 的 `distribution` 选择 KV / GS / PB / WB 会影响损失估计。

在 AVAS 中，晕的直观指标是最大尺寸与 rms 尺寸之比（DataSet 第 22/24 列与第 16/18 列），以及沿线的发射度增长和存活粒子数（第 28 列）。rms 匹配良好但最大尺寸比持续增大，通常说明有非线性或共振问题。

## 在 AVAS 中做匹配 {#matching-avas}

### 设置入口束流

束流页设置 `twissx`、`twissy`、`twissz`（α、β、ε），单位见下表：

```avas-params
beam:twissx beam:twissy beam:twissz beam:distribution
```

β 单位 mm/mrad（数值上等于 m/rad），ε 为归一化 rms 发射度（π·mm·mrad），twissz 用 z（mm）与 $z' = \Delta p/p$（mrad）。示例束流 `twissx -0.65785 0.37743 0.13768` 对应入口 $x_{\mathrm{rms}} = \sqrt{0.37743 \times 0.13768/0.05708} = 0.954\ \mathrm{mm}$。

### 交互：线性预览

可视化编辑器中拖动四极铁梯度、螺线管 `Kb` 等参数时，线性包络预览实时更新（勾选"预览中计入空间电荷"可加入线性空间电荷）。判断匹配的直观标准：周期段内的包络每个周期重复，没有逐周期的拍振。预览是近似模型（rms 通常 1–2 %，多粒子效应强时可达 13 %），不考虑损失和非线性，最终要用内核确认。

### 扫描与优化

- **参数扫描页**（或命令行 `avas scan --input DIR --target 元件 --param 参数 --values …`）：对一个参数取一组值，在 `Scans/` 下的副本里逐个运行内核，比较各取值的结果。
- **AI 助手的 `optimize` 工具**：在给定范围内调多个元件参数，最小化或最大化目标。预览引擎可用的指标有 `rms_x_max`、`rms_y_max`、`rms_x_out`、`rms_y_out`、`energy_out`、`rms_mismatch_out`；目标可以是表达式，例如 `abs(rms_x_out-1.5)+abs(rms_y_out-1.5)`。注意 `rms_mismatch_out` 是末端 $\lvert x_{\mathrm{rms}} - y_{\mathrm{rms}}\rvert$（圆度），不是本章的 Twiss 失配因子。用模拟引擎时还有传输效率、发射度增长等指标。优化结果作为修改提案，用户批准后才写入 lattice。

### 用 adjust 和 diag_size 做尺寸约束

静态误差校正机制也可以用来按束流尺寸调参数（详见 [误差与校正](#errors-correction)）：

```avas-params
adjust diag_size
```

- `adjust 0 v n min max first_step` 放在要调的元件前，`v` 是该元件的第 v 个参数（关键字后从 1 数起：`quad` 的梯度是第 4 个，`field` 的 `Kb` 是第 8 个、`Ke` 是第 7 个），`n` 相同的旋钮取同一值，`first_step = 1` 表示从当前值开始优化。
- `diag_size 0 sx sy 0` 给出该位置的目标 **rms** 尺寸（mm）。当前代码按 `adjust` 与 `diag_*` 的**第一个参数**配对（相同编号组成一次校正，按编号顺序依次进行），手册把该参数写作"无意义，写 0"，全部写 0 即为一组。
- 优化器为 SLSQP（每次函数求值是一次内核运行），目标为该组所有束诊的平均 $(x_{\mathrm{rms}} - s_x)^2 + (y_{\mathrm{rms}} - s_y)^2$（mm²），低于 0.05 即停止，最多 100 次迭代，差分步长 0.1（参数自身单位）。
- 该机制只在**静态误差研究**（`ini.ini` 的 `error_type` 为 `stat` 或 `stat_dyn`）中执行，并要求有 `err_step` 和至少一个打开的误差开关（`stat` 模式下须为静态误差开关）。结果写在 `Adjust_Datas_<组>_<次>.txt`，**不会写回 lattice**。

> 注意：只约束尺寸、不约束 α 时，单个 `diag_size` 不能确定匹配；在相隔约 $90^\circ$ 相移的两处放置尺寸目标，才能间接约束 $(\alpha, \beta)$。用零幅度的静态误差借用这一机制做无误差匹配，是根据当前代码推断的用法，尚未用内核系统验证，使用时要核对每次运行的结果。

### 检查

- 结果页：rms 包络、Twiss α/β 沿线曲线、发射度；"相移"图（需要 `lattice … lattice_end` 周期标记）按每周期 $\int\dd z/\beta$ 给出实际束流的相移。
- 运行页实时显示 rms 包络；"与前一次运行对比"可以叠加上一次运行，比较匹配前后的差别。

> AVAS 约定：手册中记载的旧版 `MATCHING`、`SETTWISS`、`CIRCLE_MATCH` 等匹配命令属于旧流程，当前界面未验证，不应作为现行功能使用。

## 动手实验 {#matching-lab}

在 FODO 实验中把 `mismatch` 设为 1（匹配）、1.2、1.44，观察包络在周期包络附近的拍振。`mismatch` 是初始尺寸的放大倍数 $s$：实验把周期入口处的 β 乘以 $s^2$、α 保持不变。若在 $\alpha = 0$ 的位置这样放大，$M = s - 1$；但周期入口处 $\alpha \approx -1.76$，同样的放大失配更大：$s = 1.2$ 时 $M \approx 0.44$，$s = 1.44$ 时 $M \approx 1.0$。

```avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1.2
```

再把直流流强 `I` 增加到 10–20 mA：零流的周期 Twiss 参数不再匹配，包络开始拍振；拍振周期与 [包络振荡模式](#matching-envelope-modes) 中的呼吸模和四极模相对应。

```avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=15 mismatch=1
```

在相空间实验中把一个椭圆通过薄透镜和漂移，体会 α 与 β 如何同时决定下游尺寸：

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0.5 L=0.5
```

## 延伸阅读 {#matching-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.：失配因子、包络模式、束晕的粒子–核模型。
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.：包络方程、失配的自由能与发射度增长。
- F. J. Sacherer (1971)：rms 包络方程，使 rms 匹配对椭圆对称分布普遍成立。
- J. Struckmeier, M. Reiser (1984)：周期通道中失配束的包络振荡与不稳定性。
- 完整书目见 [参考文献](#references)。
