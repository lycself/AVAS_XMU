import{i as e,n as t,r as n,t as r}from"./chapters-_iX45hSl.js";var i=`# Dipole

A dipole (bending magnet) bends the central orbit of the beam with a uniform transverse magnetic field. In a linac it is not the main actor, but it is indispensable in transport lines, energy analysis, branch lines to beam dumps and injection/extraction lines. A dipole brings two effects absent in straight sections: **weak focusing** (focusing from the curvature of the orbit itself) and **dispersion** (particles of different momentum follow different orbits); tilted pole faces add focusing or defocusing at both ends.

In AVAS a dipole consists of two keywords: the sector-magnet body \`bend\` and the pole-face edge \`edge\`. This chapter uses the matrix methods of [linear optics](#linear-optics) and the [quadrupole](#quadrupole) chapter, and $\\delta = \\Delta p/p$ from [longitudinal motion](#longitudinal).

## Role and placement {#dipole-role}

- **Changing the beam direction**: sending the beam to different end stations, targets or beam dumps.
- **Momentum (energy) analysis**: dispersion separates particles of different momentum in space; together with slits this selects energy or measures the energy spread.
- **Geometric layout**: folding the beam line into the available space.
- **Longitudinal manipulation**: the $R_{56}$ of a bending section can add to or cancel the $L/\\gamma^2$ of drifts, for bunch compression or stretching (for low-energy protons $L/\\gamma^2$ usually dominates).

## Physics and field {#dipole-field}

### Bending radius

In a uniform field $B$ a particle of momentum $p$ and charge $q$ moves on a circle in the plane perpendicular to the field, with radius

$$
\\rho = \\frac{p}{qB} = \\frac{B\\rho}{B},\\qquad
B\\rho\\,[\\mathrm{T\\,m}] = \\frac{p\\,[\\mathrm{MeV}/c]}{299.792458\\,Z}.
$$

Bending angle $\\alpha$ and arc length $L$ are related by $L = \\rho\\,\\alpha$ ($\\alpha$ in radians). For the example beam (protons at 1.527 MeV) $B\\rho = 0.17863$ T·m, so $\\rho = 1$ m needs $B = 0.179$ T.

### Field index

A general dipole field can be expanded around the central orbit as

$$
B_y(x) = B_0\\left(1 - n\\,\\frac{x}{\\rho} + \\cdots\\right),\\qquad
n = -\\frac{\\rho}{B_0}\\,\\frac{\\partial B_y}{\\partial x}\\bigg\\rvert_{x=0},
$$

where $n$ is the **field index**, called \`N\` in AVAS. $n = 0$ is a uniform field (parallel pole faces); a dipole with $n \\ne 0$ also carries a quadrupole component (a combined-function magnet), and by Maxwell's equations the horizontal gradient comes with an equal $\\partial B_x/\\partial y$.

### Equations of motion

In the Frenet–Serret frame that follows the curved central orbit ($x$ pointing away from the centre of curvature, $h = 1/\\rho$), the linear equations of motion are

$$
x'' + (1-n)\\,h^2\\,x = h\\,\\delta,\\qquad
y'' + n\\,h^2\\,y = 0 .
$$

- The $h^2$ in $(1-n)h^2$ is **geometric (weak) focusing**: particles outside the central orbit travel a longer arc, those inside a shorter one, and both return towards the central orbit.
- $-n h^2$ and $+n h^2$ come from the quadrupole component, with opposite signs in the two planes.
- For $0 < n < 1$ both planes focus (**weak focusing**); for $n = 1/2$ the two planes focus equally. Over one full turn ($2\\pi$) the numbers of oscillations are $\\nu_x = \\sqrt{1-n}$ and $\\nu_y = \\sqrt{n}$.
- The term $h\\,\\delta$ on the right is the source of dispersion: particles with higher momentum have a larger radius and move outwards.

The longitudinal coordinate ($z > 0$ ahead of the reference particle) changes at the rate

$$
z' = -h\\,x + \\frac{\\delta}{\\gamma^2};
$$

the first term is the path-length difference: an outer particle travels a longer path and falls behind.

## Equations of motion and transfer matrix {#dipole-matrix}

### Sector-magnet matrix

Let $K_x = (1-n)h^2$, $K_y = n h^2$, and $C_x, S_x$ the cosine- and sine-like solutions of $u'' + K_x u = 0$ ($C_x = \\cos\\sqrt{K_x}L$, $S_x = \\sin(\\sqrt{K_x}L)/\\sqrt{K_x}$, hyperbolic functions for $K < 0$). For bending in $x$, the $6\\times 6$ matrix of a sector magnet is

$$
M_{\\text{bend}} =
\\begin{pmatrix}
C_x & S_x & 0 & 0 & 0 & R_{16}\\\\
-K_x S_x & C_x & 0 & 0 & 0 & R_{26}\\\\
0 & 0 & C_y & S_y & 0 & 0\\\\
0 & 0 & -K_y S_y & C_y & 0 & 0\\\\
R_{51} & R_{52} & 0 & 0 & 1 & R_{56}\\\\
0 & 0 & 0 & 0 & 0 & 1
\\end{pmatrix},
$$

with

$$
\\begin{aligned}
R_{16} &= \\frac{h\\,(1-C_x)}{K_x}, &
R_{26} &= h\\,S_x,\\\\
R_{51} &= -h\\,S_x, &
R_{52} &= -\\frac{h\\,(1-C_x)}{K_x},\\\\
R_{56} &= \\frac{L}{\\gamma^2} - \\frac{h^2\\,(L - S_x)}{K_x}.
\\end{aligned}
$$

This agrees with \`bend_matrix(L, angle, n_index, plane, gamma)\` of AVAS's linear preview. $R_{51}$ and $R_{52}$ are tied to $R_{16}$ and $R_{26}$ by symplecticity; they describe how the transverse coordinates change the path length.

For a uniform-field sector magnet ($n = 0$), with $\\theta = L/\\rho$:

$$
R_{16} = \\rho\\,(1-\\cos\\theta),\\quad
R_{26} = \\sin\\theta,\\quad
R_{56} = \\frac{L}{\\gamma^2} - \\rho\\,(\\theta - \\sin\\theta),
$$

and the vertical plane is a drift of length $L$.

### Dispersion function

The dispersion function $D(s)$ is the (particular-solution) orbit of a particle with momentum deviation $\\delta = 1$; it obeys $D'' + K_x D = h$. Through a beam line section,

$$
\\begin{pmatrix} D \\\\ D' \\\\ 1 \\end{pmatrix}_1 =
\\begin{pmatrix} R_{11} & R_{12} & R_{16} \\\\ R_{21} & R_{22} & R_{26} \\\\ 0 & 0 & 1 \\end{pmatrix}
\\begin{pmatrix} D \\\\ D' \\\\ 1 \\end{pmatrix}_0 .
$$

Coming from a dispersion-free straight section, the exit values are $D = R_{16}$, $D' = R_{26}$. Where there is dispersion the rms beam size is

$$
\\sigma_x^2 = \\beta_x\\,\\varepsilon_x + D^2\\,\\sigma_\\delta^2 .
$$

A layout in which $D$ and $D'$ both vanish at the exit of the bending section is an **achromat**, for example two identical dipoles with a quadrupole between them that makes $D' = 0$ at the midpoint.

### Edges (pole-face rotation)

When a pole face is not perpendicular to the central orbit but rotated by an angle $\\beta$, the length of field a particle crosses at the entrance or exit varies linearly with $x$, which acts as a thin lens at the edge. In the TRANSPORT / TraceWin form:

$$
M_{\\text{edge}}:\\qquad
R_{21} = h\\tan\\beta,\\qquad
R_{43} = -h\\tan(\\beta - \\psi),
$$

all other elements as in the unit matrix. $\\beta > 0$ **defocuses** in the bending plane and **focuses** in the other. The correction for the finite extent of the fringe field is

$$
\\psi = K_1\\,G\\,h\\,\\frac{1 + \\sin^2\\beta}{\\cos\\beta}\\,\\left(1 - K_1 K_2\\,G\\,h\\tan\\beta\\right),
$$

where $G$ is the total (full) gap of the magnet and $K_1$, $K_2$ are integral factors describing the shape of the fringe field. $K_1 = 0$ gives $\\psi = 0$, the hard-edge model. This agrees with \`edge_matrix(beta, rho, gap, k1, k2, plane)\` of the linear preview.

The **rectangular magnet** is an important special case: both pole faces are perpendicular to the incoming orbit, which amounts to edges of $\\beta = \\theta/2$ at both ends of a sector magnet. The horizontal weak focusing is exactly cancelled by the edges ($R_{21} = 0$) and the vertical plane gains focusing.

## In AVAS {#dipole-avas}

### Keywords

\`\`\`text
edge 0 R 0 beta rho G K1 K2 HV
bend arc R 0 alpha rho N HV
edge 0 R 0 beta rho G K1 K2 HV
\`\`\`

\`\`\`avas-params
bend edge
\`\`\`

\`bend\`:

- \`arc\`: arc length $\\lvert\\alpha\\rho\\rvert$ (m); write 0 and the program computes it from $\\alpha$ and $\\rho$.
- \`R\`: aperture radius (m); the third parameter is reserved, write 0.
- \`alpha\`: bending angle (°) in the plane selected by HV.
- \`rho\`: radius of curvature of the central orbit (m).
- \`N\`: field index $n$.
- \`HV\`: bending plane, 0 = horizontal ($x$), 1 = vertical ($y$). With HV = 1 the roles of $x$ and $y$ in the matrix above are swapped: dispersion appears in the $y$ plane, $K_y = (1-n)h^2$, $K_x = nh^2$.

\`edge\`:

- The first and third parameters are reserved, write 0; \`R\` is the aperture.
- \`beta\`: pole-face rotation angle $\\beta$ (°).
- \`rho\`: radius of curvature (m), the same as that of the adjacent \`bend\`.
- \`G\`: total gap of the magnet (m).
- \`K1\`, \`K2\`: fringe-field factors; \`K1 = 0\` is a hard edge.
- \`HV\`: the same as the adjacent \`bend\`.

> AVAS convention: the linear preview treats \`bend\` as a sector-magnet body without pole-face angles, the pole-face rotations being given by one \`edge\` before and one after it (the same habit as TraceWin's BEND/EDGE). Whether the kernel's \`bend\` has edge effects of its own, or needs \`edge\` lines, is not stated in the manual; the kernel is the reference. \`bend\` and \`edge\` are z-code matrix-model elements; \`edge\` has zero length.

> Note: in the linear preview a negative $\\alpha$ changes the sign of $h$, and of $R_{16}$ etc. with it, i.e. it bends to the other side. Whether a positive $\\alpha$ bends towards $+x$ or $-x$ in the kernel is not stated in the manual; when it matters, confirm it with a single-particle kernel run.

### Example

A 45° rectangular dipole, $\\rho = 1$ m, horizontal bending, 5 cm gap:

\`\`\`text
drift 0.000001 0.03 0
edge 0 0.03 0 22.5 1.0 0.05 0.45 2.8 0
bend 0 0.03 0 45 1.0 0 0
edge 0 0.03 0 22.5 1.0 0.05 0.45 2.8 0
drift 0.3 0.03 0
\`\`\`

$K_1 = 0.45$ and $K_2 = 2.8$ are typical values often quoted in the TraceWin documentation; the exact values depend on the pole shape and should be fitted to measurements or 3D field calculations.

### The z coordinate in bends

Without dipoles, $z$ in DataSet.txt is column 5 + column 33. With dipoles, the AVAS interface adds up the step lengths in bending steps (flagged in column 35) and continues adding afterwards; the $z$ shown is therefore the **path length** $s$ along the central orbit, not a straight-line distance in the laboratory frame (see AGENTS.md §6 and \`dataset_stream.PathLength\`).

## Parameters and typical values {#dipole-params}

| Quantity | Typical range (low-energy transport lines) | Remark |
|---|---|---|
| field $B$ | 0.1–1.5 T | room-temperature iron magnets; set by $B\\rho$ and $\\rho$ |
| radius $\\rho$ | 0.3–2 m | |
| bending angle $\\alpha$ | 10°–90° | large angles are often split into several magnets |
| field index $n$ | 0; 0.5 common for analysing magnets | 0 < n < 1 gives weak focusing in both planes |
| pole-face angle $\\beta$ | 0 up to the order of $\\alpha/2$ | used to balance the focusing of the two planes |
| gap $G$ | 0.03–0.1 m | |

### Worked example: a 45° sector magnet

Example beam ($\\gamma = 1.00163$), $\\rho = 1$ m, $\\alpha = 45^\\circ$, $L = 0.7854$ m, $B = 0.1786$ T:

| | $n = 0$ | $n = 0.5$ |
|---|---|---|
| $R_{11}$, $R_{21}$ | 0.7071, −0.7071 m⁻¹ | 0.8497, −0.3728 m⁻¹ |
| $R_{33}$, $R_{43}$ | 1, 0 | 0.8497, −0.3728 m⁻¹ |
| $R_{16}$ (exit $D$) | 0.2929 m | 0.3006 m |
| $R_{26}$ (exit $D'$) | 0.7071 | 0.7456 |
| $R_{56}$ | 0.7046 m | 0.7033 m |

Some observations:

- A 1 % momentum deviation gives a 2.9 mm offset at the exit, large compared with the rms beam size (about 1 mm); the example beam has an rms momentum spread $\\sigma_\\delta = 3.5\\times 10^{-3}$, so dispersion adds $D\\sigma_\\delta \\approx 1.0$ mm to the rms size, as much as the emittance does.
- In $R_{56}$ the drift term $L/\\gamma^2 = 0.783$ m is much larger than the bending term $\\rho(\\theta - \\sin\\theta) = 0.078$ m: for low-energy protons the longitudinal behaviour of a bend is still mainly debunching.
- With two hard edges of $\\beta = 22.5^\\circ$ (a rectangular magnet), horizontally $R_{21} = 0$, $R_{11} = 1$, vertically $R_{43} = -0.694$ m⁻¹; the exit dispersion is unchanged ($D = 0.293$ m) and $D' = 0.828$.
- With the fringe-field correction ($G = 0.05$ m, $K_1 = 0.45$, $K_2 = 2.8$), $\\psi = 1.56^\\circ$, and the vertical focusing of one edge drops from $\\tan 22.5^\\circ = 0.414$ m⁻¹ to 0.383 m⁻¹, about 8 % weaker.

## Dynamics and common problems {#dipole-dynamics}

### Dispersion and energy-spread coupling

A dipole converts the longitudinal momentum deviation into a transverse position. Every downstream element "sees" a beam size that includes $D\\sigma_\\delta$; if the dispersion is not cancelled at the exit of the bending section it propagates along the line and, where there is acceleration or space charge, causes transverse–longitudinal coupling and effective emittance growth.

### Space charge

In dispersive regions the space-charge force depends on the total beam size including dispersion, and it also changes the dispersion itself (the dispersion function is defocused by space charge too); at high current the achromat conditions must be re-tuned with current. The kernel's multi-particle tracking includes these effects; the linear preview only gives estimates at zero current or with linear space charge.

### Non-linearities

At large momentum deviations dispersion and path length have second-order terms ($T_{166}$, $T_{566}$, …), and fringe fields and saturation add sextupole components; the matrix model is first order only. When a dipole is described by a field map, the linear preview gives a warning (dipole content of maps is not modelled); the kernel is the reference.

### Common problems

- Pole-face angles in the design but no \`edge\` lines: the linear preview gives a pure sector magnet without vertical focusing, and the beam may become much larger vertically than expected (how the kernel treats the edges of \`bend\` is defined by the kernel, see the convention above).
- \`rho\` or \`HV\` of the \`edge\` not matching the \`bend\`.
- \`arc\` of \`bend\` written as an angle or in wrong units; writing 0 is simplest.
- Comparing positions in bends with laboratory coordinates: the $z$ in the interface is the path length.

## Hands-on {#dipole-lab}

The field on the dipole cross-section (**schematic**): for $n = 0$ the field in the bore is uniform; with $n = 0.5$ it varies linearly across the bending plane, i.e. it carries a quadrupole component. \`B\` here is only a relative size; the actual field is $B\\rho/\\rho$.

\`\`\`avas-field
dipole B=0.18 N=0 rho=1 L=0.785 R=0.03
\`\`\`

\`\`\`avas-field
dipole B=0.18 N=0.5 rho=1 L=0.785 R=0.03
\`\`\`

The dipole lab gives the sector-magnet matrix, $D$ and $D'$ at the exit, the off-momentum trajectory and the $x$, $y$ envelopes:

1. Reproduce $R_{16} = 0.293$ m of the table with \`N=0\`, \`edge=0\`.
2. Set \`edge\` to 22.5 (a rectangular magnet): the horizontal focusing is cancelled and the vertical envelope is held.
3. Set \`N\` to 0.5 and compare the envelopes of the two planes; change \`delta\` from 1 to −1 and watch the off-momentum orbit move to the other side.

\`\`\`avas-lab
dipole angle=45 rho=1 N=0 edge=0 W=1.527 mass=938.272 q=1 delta=1
\`\`\`

## Further reading {#dipole-reading}

- K. L. Brown, "A First- and Second-Order Matrix Theory for the Design of Beam Transport Systems and Charged Particle Spectrometers", SLAC-75 (1967, later revised): matrices of sector magnets, pole-face rotation and fringe-field correction (the TRANSPORT conventions).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (bending magnets, dispersion function and achromats).
- S. Y. Lee, *Accelerator Physics*, 4th ed. (weak focusing, field index, dispersion).
- Related chapters: [quadrupole](#quadrupole), [longitudinal motion](#longitudinal), [lattice design](#lattice-design), [matching](#matching); the full bibliography is in [references](#references).
`,a=`# 二极铁

二极铁（dipole，bending magnet）用横向的均匀磁场使束流的中心轨道弯曲。在直线加速器里它不是主角，但在束流输运线、能量分析、束流收集器支线和注入/引出线中必不可少。二极铁带来直线段中没有的两种效应：**弱聚焦**（轨道弯曲本身产生的聚焦）和**色散**（不同动量的粒子走不同的轨道）；极面的倾斜还会在两端产生额外的聚焦或散焦。

AVAS 中二极铁由两个关键字组成：扇形磁铁本体 \`bend\` 和极面边缘 \`edge\`。本章用到[线性光学](#linear-optics)和[四极铁](#quadrupole)一章的矩阵方法以及[纵向运动](#longitudinal)中的 $\\delta = \\Delta p/p$。

## 作用与位置 {#dipole-role}

- **改变束流方向**：把束流引到不同的终端、靶站或束流收集器。
- **动量（能量）分析**：利用色散，把不同动量的粒子在空间上分开，配合狭缝做能量选择或测量能散。
- **几何布局**：在有限的场地中折转束线。
- **纵向操作**：弯转段的 $R_{56}$ 可以与漂移段的 $L/\\gamma^2$ 相抵或相加，用于束团压缩或拉伸（低能质子束中 $L/\\gamma^2$ 通常占主导）。

## 物理原理与场 {#dipole-field}

### 偏转半径

在均匀磁场 $B$ 中，动量为 $p$、电荷为 $q$ 的粒子在垂直于场的平面内做圆周运动，半径

$$
\\rho = \\frac{p}{qB} = \\frac{B\\rho}{B},\\qquad
B\\rho\\,[\\mathrm{T\\,m}] = \\frac{p\\,[\\mathrm{MeV}/c]}{299.792458\\,Z}.
$$

偏转角 $\\alpha$ 与弧长 $L$ 的关系为 $L = \\rho\\,\\alpha$（$\\alpha$ 用弧度）。示例束流（质子 1.527 MeV）$B\\rho = 0.17863$ T·m，$\\rho = 1$ m 需要 $B = 0.179$ T。

### 场梯度指数

一般的二极铁场可以在中心轨道附近展开为

$$
B_y(x) = B_0\\left(1 - n\\,\\frac{x}{\\rho} + \\cdots\\right),\\qquad
n = -\\frac{\\rho}{B_0}\\,\\frac{\\partial B_y}{\\partial x}\\bigg\\rvert_{x=0},
$$

$n$ 称为**场梯度指数**（field index），AVAS 中记为 \`N\`。$n = 0$ 是均匀场（极面平行）；$n \\ne 0$ 的二极铁同时带有四极分量（组合作用磁铁），由 Maxwell 方程，水平方向的梯度伴随着同样大小的 $\\partial B_x/\\partial y$。

### 运动方程

在随中心轨道弯曲的 Frenet–Serret 坐标系中（$x$ 指向曲率中心外侧，$h = 1/\\rho$），线性运动方程为

$$
x'' + (1-n)\\,h^2\\,x = h\\,\\delta,\\qquad
y'' + n\\,h^2\\,y = 0 .
$$

- $(1-n)h^2$ 中的 $h^2$ 是**几何（弱）聚焦**：轨道外侧的粒子走更长的圆弧，内侧的走更短的圆弧，结果向中心轨道回归。
- $-n h^2$ 与 $+n h^2$ 来自四极分量，在两个平面符号相反。
- $0 < n < 1$ 时两个平面都聚焦（**弱聚焦**），$n = 1/2$ 时两个平面聚焦强度相同；一个完整回转（$2\\pi$）对应的振荡数为 $\\nu_x = \\sqrt{1-n}$、$\\nu_y = \\sqrt{n}$。
- 右边的 $h\\,\\delta$ 是色散源项：动量较高的粒子半径较大，偏向外侧。

纵向坐标（$z > 0$ 在参考粒子前方）的变化率为

$$
z' = -h\\,x + \\frac{\\delta}{\\gamma^2},
$$

第一项是路程差：外侧的粒子走更长的路，落在后面。

## 运动方程与传输矩阵 {#dipole-matrix}

### 扇形磁铁矩阵

令 $K_x = (1-n)h^2$、$K_y = n h^2$，$C_x, S_x$ 为 $u'' + K_x u = 0$ 的余弦、正弦解（$C_x = \\cos\\sqrt{K_x}L$，$S_x = \\sin(\\sqrt{K_x}L)/\\sqrt{K_x}$，$K < 0$ 时换成双曲函数）。弯转平面为 $x$ 时，扇形磁铁的 $6\\times 6$ 矩阵为

$$
M_{\\text{bend}} =
\\begin{pmatrix}
C_x & S_x & 0 & 0 & 0 & R_{16}\\\\
-K_x S_x & C_x & 0 & 0 & 0 & R_{26}\\\\
0 & 0 & C_y & S_y & 0 & 0\\\\
0 & 0 & -K_y S_y & C_y & 0 & 0\\\\
R_{51} & R_{52} & 0 & 0 & 1 & R_{56}\\\\
0 & 0 & 0 & 0 & 0 & 1
\\end{pmatrix},
$$

其中

$$
\\begin{aligned}
R_{16} &= \\frac{h\\,(1-C_x)}{K_x}, &
R_{26} &= h\\,S_x,\\\\
R_{51} &= -h\\,S_x, &
R_{52} &= -\\frac{h\\,(1-C_x)}{K_x},\\\\
R_{56} &= \\frac{L}{\\gamma^2} - \\frac{h^2\\,(L - S_x)}{K_x}.
\\end{aligned}
$$

这与 AVAS 线性预览的 \`bend_matrix(L, angle, n_index, plane, gamma)\` 一致。$R_{51}$、$R_{52}$ 由辛条件与 $R_{16}$、$R_{26}$ 联系在一起，它们描述横向坐标对路程长度的影响。

对 $n = 0$ 的均匀场扇形磁铁，$\\theta = L/\\rho$：

$$
R_{16} = \\rho\\,(1-\\cos\\theta),\\quad
R_{26} = \\sin\\theta,\\quad
R_{56} = \\frac{L}{\\gamma^2} - \\rho\\,(\\theta - \\sin\\theta),
$$

垂直方向是长度为 $L$ 的漂移。

### 色散函数

色散函数 $D(s)$ 是动量偏差 $\\delta = 1$ 的粒子的闭合（特解）轨道，满足 $D'' + K_x D = h$。经过一段传输，

$$
\\begin{pmatrix} D \\\\ D' \\\\ 1 \\end{pmatrix}_1 =
\\begin{pmatrix} R_{11} & R_{12} & R_{16} \\\\ R_{21} & R_{22} & R_{26} \\\\ 0 & 0 & 1 \\end{pmatrix}
\\begin{pmatrix} D \\\\ D' \\\\ 1 \\end{pmatrix}_0 .
$$

从无色散的直线段进入时，出口处 $D = R_{16}$、$D' = R_{26}$。有色散处的 rms 束流尺寸为

$$
\\sigma_x^2 = \\beta_x\\,\\varepsilon_x + D^2\\,\\sigma_\\delta^2 .
$$

让 $D$ 和 $D'$ 在弯转段出口同时为零的布局称为**消色散**（achromat），例如两块相同的二极铁之间放一个四极铁，使中点处 $D' = 0$。

### 边缘（极面旋转）

当极面不垂直于中心轨道、而是转过角度 $\\beta$ 时，进出口处的粒子在磁场中走过的长度随 $x$ 线性变化，相当于在边缘处加了一个薄透镜。采用 TRANSPORT / TraceWin 的形式：

$$
M_{\\text{edge}}:\\qquad
R_{21} = h\\tan\\beta,\\qquad
R_{43} = -h\\tan(\\beta - \\psi),
$$

其他元素与单位矩阵相同。$\\beta > 0$ 使弯转平面**散焦**、另一平面**聚焦**。边缘场有限宽度带来的修正角为

$$
\\psi = K_1\\,G\\,h\\,\\frac{1 + \\sin^2\\beta}{\\cos\\beta}\\,\\left(1 - K_1 K_2\\,G\\,h\\tan\\beta\\right),
$$

其中 $G$ 是磁体总间隙（全间隙），$K_1$、$K_2$ 是描述边缘场形状的积分因子。$K_1 = 0$ 时 $\\psi = 0$，即硬边模型。这与线性预览的 \`edge_matrix(beta, rho, gap, k1, k2, plane)\` 一致。

**矩形磁铁**是一个重要的特例：两端极面都与入射轨道垂直，相当于扇形磁铁两端各加 $\\beta = \\theta/2$ 的边缘。水平方向的弱聚焦被边缘完全抵消（$R_{21} = 0$），垂直方向获得聚焦。

## 在 AVAS 中 {#dipole-avas}

### 关键字

\`\`\`text
edge 0 R 0 beta rho G K1 K2 HV
bend arc R 0 alpha rho N HV
edge 0 R 0 beta rho G K1 K2 HV
\`\`\`

\`\`\`avas-params
bend edge
\`\`\`

\`bend\`：

- \`arc\`：弧长 $\\lvert\\alpha\\rho\\rvert$（m），写 0 即可，程序按 $\\alpha$、$\\rho$ 自动计算。
- \`R\`：孔径半径（m）；第三个参数保留，写 0。
- \`alpha\`：偏转角（°），在 HV 所选平面内。
- \`rho\`：中心轨道曲率半径（m）。
- \`N\`：场梯度指数 $n$。
- \`HV\`：弯转平面，0 = 水平（$x$），1 = 垂直（$y$）。HV = 1 时上述矩阵中 $x$ 与 $y$ 的角色互换：色散出现在 $y$ 平面，$K_y = (1-n)h^2$，$K_x = nh^2$。

\`edge\`：

- 第一和第三个参数保留，写 0；\`R\` 为孔径。
- \`beta\`：极面旋转角 $\\beta$（°）。
- \`rho\`：曲率半径（m），应与相邻 \`bend\` 的 $\\rho$ 相同。
- \`G\`：磁体总间隙（m）。
- \`K1\`、\`K2\`：边缘场因子；\`K1 = 0\` 为硬边。
- \`HV\`：与相邻 \`bend\` 相同。

> AVAS 约定：线性预览把 \`bend\` 当作不含极面角的扇形磁铁本体，极面旋转由前后各一条 \`edge\` 给出（与 TraceWin 的 BEND/EDGE 习惯相同）。内核中 \`bend\` 本身是否带有边缘作用、是否必须配 \`edge\`，手册没有说明，以内核为准。\`bend\` 与 \`edge\` 都是 z-code 矩阵模型元件；\`edge\` 长度为 0。

> 注意：线性预览中 $\\alpha$ 取负值时 $h$ 变号，$R_{16}$ 等随之变号，即向另一侧偏转。内核中正的 $\\alpha$ 究竟朝 $+x$ 还是 $-x$ 偏转，手册没有说明；需要时用单粒子内核运行确认。

### 示例

45° 矩形二极铁，$\\rho = 1$ m，水平偏转，间隙 5 cm：

\`\`\`text
drift 0.000001 0.03 0
edge 0 0.03 0 22.5 1.0 0.05 0.45 2.8 0
bend 0 0.03 0 45 1.0 0 0
edge 0 0.03 0 22.5 1.0 0.05 0.45 2.8 0
drift 0.3 0.03 0
\`\`\`

$K_1 = 0.45$、$K_2 = 2.8$ 是 TraceWin 文档中常见的典型值；准确值取决于磁极形状，应由实测或三维场计算拟合得到。

### 弯转段中的 z 坐标

DataSet.txt 在没有二极铁时 $z$ = 第 5 列 + 第 33 列。有二极铁时，AVAS 界面在弯转步中按步长累加路程（第 35 列标出弯转步），之后继续累加；界面上的 $z$ 因此是沿中心轨道的**路程长度** $s$，不是实验室坐标系中的直线距离（见 AGENTS.md §6 与 \`dataset_stream.PathLength\`）。

## 可调参数与典型取值 {#dipole-params}

| 量 | 典型范围（低能输运线） | 说明 |
|---|---|---|
| 场强 $B$ | 0.1–1.5 T | 铁芯常温磁铁；由 $B\\rho$ 和 $\\rho$ 决定 |
| 曲率半径 $\\rho$ | 0.3–2 m | |
| 偏转角 $\\alpha$ | 10°–90° | 大角度常分成几块 |
| 场梯度指数 $n$ | 0；分析磁铁常用 0.5 | 0 < n < 1 两平面弱聚焦 |
| 极面角 $\\beta$ | 0 至 $\\alpha/2$ 量级 | 用于平衡两个平面的聚焦 |
| 间隙 $G$ | 0.03–0.1 m | |

### 数值例子：45° 扇形磁铁

示例束流（$\\gamma = 1.00163$），$\\rho = 1$ m，$\\alpha = 45^\\circ$，$L = 0.7854$ m，$B = 0.1786$ T：

| | $n = 0$ | $n = 0.5$ |
|---|---|---|
| $R_{11}$，$R_{21}$ | 0.7071，−0.7071 m⁻¹ | 0.8497，−0.3728 m⁻¹ |
| $R_{33}$，$R_{43}$ | 1，0 | 0.8497，−0.3728 m⁻¹ |
| $R_{16}$（出口 $D$） | 0.2929 m | 0.3006 m |
| $R_{26}$（出口 $D'$） | 0.7071 | 0.7456 |
| $R_{56}$ | 0.7046 m | 0.7033 m |

几点观察：

- 1 % 的动量偏差在出口产生 2.9 mm 的位置偏移，与束流 rms 尺寸（约 1 mm）相比很大；示例束流的 rms 动量散度 $\\sigma_\\delta = 3.5\\times 10^{-3}$，色散对 rms 尺寸的贡献 $D\\sigma_\\delta \\approx 1.0$ mm，与发射度的贡献相当。
- $R_{56}$ 中漂移项 $L/\\gamma^2 = 0.783$ m 远大于弯转项 $\\rho(\\theta - \\sin\\theta) = 0.078$ m：对低能质子，弯转段的纵向行为主要还是去聚束。
- 加上 $\\beta = 22.5^\\circ$ 的两个硬边边缘（矩形磁铁）后，水平 $R_{21} = 0$、$R_{11} = 1$，垂直 $R_{43} = -0.694$ m⁻¹；出口色散不变（$D = 0.293$ m），$D' = 0.828$。
- 带边缘场修正（$G = 0.05$ m，$K_1 = 0.45$，$K_2 = 2.8$）时 $\\psi = 1.56^\\circ$，单个边缘的垂直聚焦从 $\\tan 22.5^\\circ = 0.414$ m⁻¹ 降为 0.383 m⁻¹，约弱 8 %。

## 动力学效应与常见问题 {#dipole-dynamics}

### 色散与能散耦合

二极铁把纵向的动量偏差转换为横向位置。下游所有元件"看到"的束流尺寸都包含 $D\\sigma_\\delta$；如果色散不在弯转段出口消掉，它会沿输运线传播，并在有加速或空间电荷的地方造成横向–纵向耦合和有效发射度增长。

### 空间电荷

在色散区，空间电荷力依赖于包含色散的总束流尺寸，还会改变色散本身（色散函数也受空间电荷散焦），强流时消色散条件需要在有流强的条件下重新调。AVAS 内核的多粒子跟踪包含这些效应，线性预览只给出零流强或线性空间电荷下的估计。

### 非线性

大动量偏差下，色散和路程长度都有二阶项（$T_{166}$、$T_{566}$ 等），磁铁的边缘场和饱和还会带来六极分量；矩阵模型只有一阶。用场图描述二极铁时，线性预览会给出警告（不模拟场图的偶极分量），以内核结果为准。

### 常见问题

- 设计中有极面角却没写 \`edge\`：线性预览得到的是纯扇形磁铁，垂直方向没有聚焦，束流在垂直面上可能比预期大很多（内核对 \`bend\` 边缘的处理以内核为准，见上文约定）。
- \`edge\` 的 \`rho\` 或 \`HV\` 与 \`bend\` 不一致。
- \`bend\` 的 \`arc\` 写成角度或写错单位；写 0 最省事。
- 用实验室坐标比较弯转段的位置：界面中的 $z$ 是路程长度。

## 动手实验 {#dipole-lab}

二极铁横截面的场（**示意**）：$n = 0$ 时孔径内场均匀；改成 $n = 0.5$ 时场沿弯转平面线性变化，即带有四极分量。这里的 \`B\` 只是相对大小，实际场由 $B\\rho/\\rho$ 决定。

\`\`\`avas-field
dipole B=0.18 N=0 rho=1 L=0.785 R=0.03
\`\`\`

\`\`\`avas-field
dipole B=0.18 N=0.5 rho=1 L=0.785 R=0.03
\`\`\`

二极铁实验给出扇形磁铁矩阵、出口处的 $D$、$D'$、偏离动量粒子的轨道和 $x$、$y$ 包络：

1. 用 \`N=0\`、\`edge=0\` 复现上表 $R_{16} = 0.293$ m。
2. 把 \`edge\` 改为 22.5（矩形磁铁），观察水平聚焦被抵消、垂直包络被约束。
3. 把 \`N\` 改为 0.5，比较两个平面的包络；把 \`delta\` 从 1 改为 −1，观察偏离动量轨道翻到另一侧。

\`\`\`avas-lab
dipole angle=45 rho=1 N=0 edge=0 W=1.527 mass=938.272 q=1 delta=1
\`\`\`

## 延伸阅读 {#dipole-reading}

- K. L. Brown, "A First- and Second-Order Matrix Theory for the Design of Beam Transport Systems and Charged Particle Spectrometers", SLAC-75 (1967 及后续修订)：扇形磁铁、极面旋转和边缘场修正的矩阵（TRANSPORT 约定）。
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed.（弯转磁铁、色散函数与消色散系统）。
- S. Y. Lee, *Accelerator Physics*, 4th ed.（弱聚焦、场梯度指数、色散）。
- 相关章节：[四极铁](#quadrupole)、[纵向运动](#longitudinal)、[lattice 设计](#lattice-design)、[匹配](#matching)；完整书目见[参考文献](#references)。
`,o=`# Drift

A drift is the simplest element: a stretch of vacuum pipe without any applied electromagnetic field. It is also the most frequent line in a lattice — the gaps between elements, the space left for diagnostics and pumps, the transitions between cavities are all drifts. Precisely because it "does nothing" it is the reference for every other element: the transfer matrix of any element can be read as "drift plus the action of the field".

This chapter assumes the reader knows $\\beta$, $\\gamma$, $B\\rho$ from [kinematics](#kinematics) and the Twiss parameters and rms emittance from [phase space](#phase-space).

## Role and placement {#drift-role}

What drifts do in a linac:

- **Geometric spacing**: quadrupoles, solenoids and cavities need room for flanges, bellows, vacuum valves and beam diagnostics (BPMs, current transformers, profile monitors). For the beam these spaces are drifts.
- **Phase adjustment**: between two RF cavities the drift length sets the RF phase at which the bunch reaches the next cavity (see [longitudinal motion](#longitudinal)).
- **Free evolution**: transversely a diverging beam grows linearly in a drift, a converging one first shrinks to a waist and then grows; longitudinally a bunch with momentum spread lengthens (debunching).
- **Loss detection**: the aperture radius \`R\` of a drift decides whether a particle "hits the wall", so drifts are one of the main places where AVAS detects beam loss.

In AVAS drifts also have a "technical" use: matrix-model elements should not be the first or last element of a multi-particle lattice, so a very short drift is often inserted at both ends (see [In AVAS](#drift-avas)).

## Physics: motion in field-free space {#drift-physics}

### Assumptions

- No external fields: $\\boldsymbol{E} = 0$, $\\boldsymbol{B} = 0$.
- Space charge (the beam's self-field) is ignored for now; its role in drifts is discussed in [the dynamics section](#drift-dynamics) and in the [space charge](#space-charge) chapter.
- Paraxial approximation: $x' = p_x/p \\ll 1$.

Without forces each particle keeps its momentum $\\boldsymbol{p}$ and moves on a straight line at constant speed. With the longitudinal position $s$ as the independent variable the transverse coordinates obey

$$
x'' = 0,\\qquad y'' = 0,
$$

that is $x(s) = x_0 + x_0' s$, $y(s) = y_0 + y_0' s$.

### Longitudinal: slippage from the velocity difference

AVAS's longitudinal coordinate $z$ is the position of a particle relative to the reference particle ($z>0$ means ahead of it), and $\\delta = \\Delta p/p$ is the relative momentum deviation. A particle with momentum deviation $\\delta$ has the velocity

$$
\\frac{\\Delta v}{v} = \\frac{1}{\\gamma^2}\\,\\frac{\\Delta p}{p} = \\frac{\\delta}{\\gamma^2}.
$$

Through a drift of length $L$ the reference particle needs $t = L/v$; the off-momentum particle travels $\\Delta v\\, t = L\\,\\delta/\\gamma^2$ farther (or less), so

$$
z = z_0 + \\frac{L}{\\gamma^2}\\,\\delta .
$$

This is **debunching** in a drift: particles with higher momentum run ahead, those with lower momentum fall behind, and the bunch lengthens. The factor $1/\\gamma^2$ shows that it matters only at low energy; for an ultra-relativistic beam, $\\gamma \\to \\infty$, the longitudinal motion is practically frozen.

> Note: the path-length difference $\\tfrac12 L\\,(x'^2 + y'^2)$ from the transverse motion is neglected here; it is of second order and does not appear in the linear matrix.

## Equations of motion and transfer matrix {#drift-matrix}

In the six coordinates $(x, x', y, y', z, \\delta)$ the transfer matrix of a drift of length $L$ is

$$
M_{\\text{drift}}(L) =
\\begin{pmatrix}
1 & L & 0 & 0 & 0 & 0\\\\
0 & 1 & 0 & 0 & 0 & 0\\\\
0 & 0 & 1 & L & 0 & 0\\\\
0 & 0 & 0 & 1 & 0 & 0\\\\
0 & 0 & 0 & 0 & 1 & L/\\gamma^2\\\\
0 & 0 & 0 & 0 & 0 & 1
\\end{pmatrix}.
$$

This is exactly the matrix used by AVAS's linear envelope preview (\`drift_matrix\` in \`avas/sim/linear_optics.py\`). Its determinant is 1 and it is symplectic, $M\\tp J M = J$.

### Evolution of second moments and Twiss parameters

The beam's matrix of second moments (the $\\Sigma$ matrix) is transported as $\\Sigma_1 = M \\Sigma_0 M\\tp$. For one plane, in Twiss parameters,

$$
\\begin{aligned}
\\beta(s) &= \\beta_0 - 2\\alpha_0 s + \\gamma_0 s^2,\\\\
\\alpha(s) &= \\alpha_0 - \\gamma_0 s,\\\\
\\gamma(s) &= \\gamma_0,
\\end{aligned}
$$

where $\\gamma_0 = (1+\\alpha_0^2)/\\beta_0$ is the Twiss parameter (not to be confused with the Lorentz factor $\\gamma$). Some direct consequences:

- $\\gamma_{\\text{Twiss}}$ is constant in a drift, so the rms divergence $\\sigma_{x'} = \\sqrt{\\gamma_0 \\varepsilon}$ is constant — without a force the angles cannot change.
- The waist ($\\alpha = 0$) lies at $s_w = \\alpha_0/\\gamma_0$. $\\alpha_0 > 0$ is a converging beam with its waist downstream; $\\alpha_0 < 0$ a diverging beam whose "waist" is upstream (a virtual waist).
- At the waist $\\beta_w = 1/\\gamma_0$.

Referred to the waist, the beta function reads

$$
\\beta(s) = \\beta_w + \\frac{(s-s_w)^2}{\\beta_w}.
$$

Far enough from the waist the rms size grows linearly with distance, with the rms divergence as slope.

### Emittance conservation

A drift is a linear, symplectic map with $\\det M = 1$, so the rms geometric emittance of each plane, $\\varepsilon = \\sqrt{\\det \\Sigma_{2\\times 2}}$, is conserved; a drift does not accelerate, $\\beta\\gamma$ is constant and so is the normalized emittance. In a multi-particle simulation **without space charge** the rms emittances in DataSet.txt must stay constant through a drift (within statistical noise); if they visibly change, the cause is usually the non-linear part of space charge or particle loss (particles removed by the aperture change the statistics).

## In AVAS {#drift-avas}

A drift is written in the lattice as

\`\`\`text
drift L R 0
\`\`\`

\`\`\`avas-params
drift
\`\`\`

- \`L\`: length in **m** (TraceWin lattices use mm — convert when importing or comparing).
- \`R\`: aperture radius in m, used for loss detection: a particle whose transverse position exceeds this radius is counted as lost.
- The third parameter is reserved; write 0.

> AVAS convention: in the schema \`drift\` is a **field-model element**, advanced by the t-code (time as the independent variable, integrated with the step set by \`steppercycle\` in input.txt). Space charge is computed in drifts as usual. Quadrupoles, solenoids, dipoles, steerers and edges, by contrast, are z-code matrix-model elements (see [quadrupole](#quadrupole)).

### Very short drifts at both ends

The AVAS manual says: in a multi-particle model avoid matrix-model elements as the first and the last element; adding a very short drift solves this. For example:

\`\`\`text
start
drift 0.000001 0.02 0
quad 0.1 0.02 0 6
drift 0.2 0.02 0
quad 0.1 0.02 0 -6
drift 0.2 0.02 0
end
\`\`\`

Both ends are then t-code elements. The drift at the end also leaves room for output planes (an output plane too close to the end of the lattice fails, see the [simulation](#simulation) chapter).

### Aperture and separate boundary

The \`R\` of a drift is the circular aperture for loss detection. With \`boundary 1\` in input.txt the kernel uses the separate boundary of Boundary.txt instead, and the radii of all lattice elements no longer apply:

\`\`\`avas-params
input:boundary
\`\`\`

## Parameters and typical values {#drift-params}

| Parameter | Meaning | Typical values (low-energy proton linac) |
|---|---|---|
| \`L\` | drift length | gaps between elements 0.05–0.3 m; between cavities 0.1–0.5 m; several metres in transport lines |
| \`R\` | aperture radius | 0.01–0.03 m, usually the same as the neighbouring elements |

Choosing a drift length is a trade-off:

- **Transverse**: the longer the distance between two focusing elements, the larger the beam grows in between and the stronger the focusing must be; in a periodic structure the length directly sets the phase advance per period (see the FODO lab in the [quadrupole](#quadrupole) chapter).
- **Longitudinal**: low-energy beams debunch quickly. For the example beam (protons, $W = 1.527$ MeV, $\\gamma = 1.00163$, $1/\\gamma^2 = 0.99675$) we are practically in the non-relativistic limit, $L/\\gamma^2 \\approx L$, and the bunch spreads longitudinally as "freely" as transversely.
- **Aperture**: as a rule of thumb the aperture should be at least 5–10 times the rms size (depending on halo and error budget), otherwise losses become very sensitive.

### Worked example: growth of the example beam in a drift

The initial beam of the example project \`examples/hwr010\`: $\\alpha_x = -0.65785$, $\\beta_x = 0.37743$ mm/mrad, $\\varepsilon_{n,x} = 0.13768\\ \\pi$·mm·mrad; $\\beta\\gamma = 0.05708$. The initial rms size is $\\sigma_x = \\sqrt{\\beta_x \\varepsilon_{n,x}/(\\beta\\gamma)} = 0.954$ mm. With $\\alpha_x < 0$ the beam diverges (its virtual waist is about 0.17 m upstream of the entrance). With the matrix above:

| Drift length | rms $x$ | rms $y$ | rms $z$ | rms phase width (162.5 MHz) |
|---|---|---|---|---|
| 0 | 0.954 mm | 0.958 mm | 1.316 mm | 4.5° |
| 0.3 m | 1.64 mm | 1.64 mm | 1.65 mm | 5.7° |
| 0.5 m | 2.19 mm | 2.19 mm | 2.15 mm | 7.4° |
| 1.0 m | 3.64 mm | 3.65 mm | 3.71 mm | 12.7° |

The phase width is $\\Delta\\phi = 360^\\circ \\cdot z/(\\beta\\lambda)$ with $\\beta\\lambda = 105.1$ mm. Without focusing and without space charge, 1 m of drift almost triples the phase width of the bunch — which is why the low-energy part of a linac needs frequent transverse and longitudinal focusing.

## Dynamics and common problems {#drift-dynamics}

### Space charge makes drifts "not free"

The results above hold only at zero current. With current, the self-field of the beam keeps acting in the drift: it defocuses transversely and longitudinally, and the beam grows faster than at zero current; the non-linear part of space charge also causes rms emittance growth and halo. For low-energy, high-current beams the problem grows with the drift length — one of the main reasons for compact low-energy sections. The models are discussed in the [space charge](#space-charge) chapter.

### Debunching and phase slippage

The longitudinal coordinate changes by $L\\,\\delta/\\gamma^2$, so between two cavities:

- the phase at which the bunch centre (the reference particle) reaches the next cavity is set by the drift length and the velocity: $\\Delta\\phi = 360^\\circ \\cdot L/(\\beta\\lambda)$;
- the phase width of the bunch grows with the drift length; the next cavity receives a longer bunch.

When a drift between cavities is changed, the meaning of the downstream cavity phase depends on V3 of \`field\`: with V3 = 0 the kernel solves the synchronous phase again, with V3 = 2 the absolute phase stays and the entry phase changes (see [RF cavity](#rf-cavity)).

### Common problems

- **Units**: AVAS lengths and apertures are in m; mm values copied from TraceWin make drifts 1000 times too long, typically seen as the whole beam being lost quickly.
- **Aperture 0 or too small**: particles are counted as lost in the first step.
- **A drift in place of a missing field map**: removes all focusing and acceleration of that element; use the result only for comparison.
- **Emittance "changes"**: without space charge the emittance in a drift must be conserved; if it changes, check losses first (surviving macro-particles, column 28 of DataSet.txt).

## Hands-on {#drift-lab}

The lab below puts the $x$-plane parameters of the example beam into a drift (\`f=0\` means no lens). Drag \`L\` and watch the rms ellipse shear: its area (the emittance) stays, its tilt changes with distance, $\\alpha$ becomes more negative, the rms size grows and the rms divergence stays.

\`\`\`avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.5
\`\`\`

Then set \`alpha\` to a positive value (for example 1.5): the beam first converges to a waist and then diverges, and you can check the waist position $s_w = \\alpha_0/\\gamma_0$.

To see longitudinal debunching, build a lattice of \`drift\` only in the visual editor and watch the rms $z$ of the linear envelope preview; then run the same structure with the kernel and compare with and without space charge (\`spacecharge 0/1\` in input.txt).

## Further reading {#drift-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH (2008): chapter 7 on matrix methods in transverse dynamics, chapter 6 on longitudinal motion.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH (2008): chapter 3 on beam optics and focusing systems without space charge.
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer (2015): transfer matrices and the evolution of Twiss parameters.
- Related chapters: [phase space](#phase-space), [linear optics](#linear-optics), [longitudinal motion](#longitudinal), [space charge](#space-charge); the full bibliography is in [references](#references).
`,s=`# 漂移段

漂移段（drift）是最简单的元件：一段没有外加电磁场的真空管道。它在 lattice 里出现得最多——元件之间的间隔、诊断和泵站留出的空间、腔体之间的过渡段都是漂移段。正因为"什么也不做"，它是理解其他元件的参照：任何元件的传输矩阵都可以看作"漂移 + 场的作用"。

本章假设读者已经熟悉[运动学](#kinematics)中的 $\\beta$、$\\gamma$、$B\\rho$ 和[相空间](#phase-space)中的 Twiss 参数与 rms 发射度。

## 作用与位置 {#drift-role}

漂移段在直线加速器中承担的角色：

- **几何间隔**：四极铁、螺线管、腔体之间需要留出法兰、波纹管、真空阀、束流诊断（BPM、流强互感器、剖面探测器）的空间。这些空间对束流而言就是漂移段。
- **相位调整**：在两个射频腔之间，漂移长度决定了束团到达下一个腔时的 RF 相位（见[纵向运动](#longitudinal)）。
- **自由演化**：横向上，发散的束流在漂移段中线性变大，会聚的束流先变小（束腰）再变大；纵向上，有动量散度的束团会拉长（去聚束）。
- **损失判定**：漂移段的孔径半径 \`R\` 决定粒子是否"撞墙"，因此漂移段也是 AVAS 判断束流损失的主要位置之一。

在 AVAS 中漂移段还有一个"工程"用途：矩阵模型元件不宜作为多粒子 lattice 的第一个或最后一个元件，常在首尾插入一段极短的 drift（见[在 AVAS 中](#drift-avas)）。

## 物理原理：无场空间中的运动 {#drift-physics}

### 假设

- 没有外场：$\\boldsymbol{E} = 0$，$\\boldsymbol{B} = 0$。
- 暂不考虑空间电荷（束流自场）；空间电荷在漂移段中的作用见[本章动力学部分](#drift-dynamics)和[空间电荷](#space-charge)一章。
- 采用傍轴近似：$x' = p_x/p \\ll 1$。

没有力作用时，每个粒子的动量 $\\boldsymbol{p}$ 守恒，粒子做匀速直线运动。以纵向位置 $s$ 为自变量，横向坐标满足

$$
x'' = 0,\\qquad y'' = 0,
$$

即 $x(s) = x_0 + x_0' s$，$y(s) = y_0 + y_0' s$。

### 纵向：速度差带来的相对滑移

AVAS 的纵向坐标 $z$ 是粒子相对参考粒子的纵向位置（$z>0$ 表示在参考粒子前方），$\\delta = \\Delta p/p$ 是相对动量偏差。动量偏差为 $\\delta$ 的粒子速度为

$$
\\frac{\\Delta v}{v} = \\frac{1}{\\gamma^2}\\,\\frac{\\Delta p}{p} = \\frac{\\delta}{\\gamma^2}.
$$

经过长度 $L$ 的漂移段，参考粒子用时 $t = L/v$，偏差粒子多走（或少走）$\\Delta v\\, t = L\\,\\delta/\\gamma^2$，于是

$$
z = z_0 + \\frac{L}{\\gamma^2}\\,\\delta .
$$

这就是漂移段中的**去聚束（debunching）**：动量较高的粒子跑到前面，较低的落在后面，束团在纵向上拉长。$1/\\gamma^2$ 因子说明它只在低能时重要；对极端相对论束流 $\\gamma \\to \\infty$，纵向几乎"冻结"。

> 注意：这里忽略了横向运动带来的路程差 $\\tfrac12 L\\,(x'^2 + y'^2)$，它是二阶小量，线性矩阵中不出现。

## 运动方程与传输矩阵 {#drift-matrix}

在六维坐标 $(x, x', y, y', z, \\delta)$ 下，长度为 $L$ 的漂移段的传输矩阵为

$$
M_{\\text{drift}}(L) =
\\begin{pmatrix}
1 & L & 0 & 0 & 0 & 0\\\\
0 & 1 & 0 & 0 & 0 & 0\\\\
0 & 0 & 1 & L & 0 & 0\\\\
0 & 0 & 0 & 1 & 0 & 0\\\\
0 & 0 & 0 & 0 & 1 & L/\\gamma^2\\\\
0 & 0 & 0 & 0 & 0 & 1
\\end{pmatrix}.
$$

这与 AVAS 线性包络预览（\`avas/sim/linear_optics.py\` 的 \`drift_matrix\`）使用的矩阵完全一致。矩阵行列式为 1，满足辛条件 $M\\tp J M = J$。

### 二阶矩与 Twiss 参数的演化

束流的二阶矩矩阵（$\\Sigma$ 矩阵）按 $\\Sigma_1 = M \\Sigma_0 M\\tp$ 传输。对单个平面，用 Twiss 参数写成

$$
\\begin{aligned}
\\beta(s) &= \\beta_0 - 2\\alpha_0 s + \\gamma_0 s^2,\\\\
\\alpha(s) &= \\alpha_0 - \\gamma_0 s,\\\\
\\gamma(s) &= \\gamma_0,
\\end{aligned}
$$

其中 $\\gamma_0 = (1+\\alpha_0^2)/\\beta_0$ 是 Twiss 参数（不要与相对论因子 $\\gamma$ 混淆）。由此可得几个直接结论：

- $\\gamma_{\\text{Twiss}}$ 在漂移段中不变，即 rms 发散角 $\\sigma_{x'} = \\sqrt{\\gamma_0 \\varepsilon}$ 不变——没有力，角度不会改变。
- 束腰（$\\alpha = 0$）位于 $s_w = \\alpha_0/\\gamma_0$。$\\alpha_0 > 0$ 表示会聚束，腰在下游；$\\alpha_0 < 0$ 表示发散束，"腰"在上游（虚腰）。
- 束腰处 $\\beta_w = 1/\\gamma_0$。

以束腰为参照，$\\beta$ 函数可以写成

$$
\\beta(s) = \\beta_w + \\frac{(s-s_w)^2}{\\beta_w}.
$$

离开束腰足够远时 rms 尺寸随距离线性增长，斜率就是 rms 发散角。

### 发射度守恒

漂移段是线性、辛的变换，$\\det M = 1$，因此每个平面的 rms 几何发射度 $\\varepsilon = \\sqrt{\\det \\Sigma_{2\\times 2}}$ 保持不变；漂移段不加速，$\\beta\\gamma$ 不变，归一化发射度也不变。在实际的多粒子模拟中，**无空间电荷**时漂移段内 DataSet.txt 中的 rms 发射度应保持常数（在统计噪声范围内）；若看到明显变化，原因通常是空间电荷的非线性部分或粒子损失（被孔径截掉的粒子会改变统计量）。

## 在 AVAS 中 {#drift-avas}

lattice 中漂移段写作

\`\`\`text
drift L R 0
\`\`\`

\`\`\`avas-params
drift
\`\`\`

- \`L\`：长度，单位 **m**（TraceWin 的 lattice 用 mm，导入或对照时要换算）。
- \`R\`：孔径半径，单位 m，用于判断束流损失：粒子横向位置超出该半径即记为损失。
- 第三个参数保留，写 0。

> AVAS 约定：\`drift\` 在 schema 中归为**场模型元件**，由 t-code 推进（以时间为自变量，按 input.txt 中 \`steppercycle\` 设定的步长积分）。空间电荷在漂移段中照常计算。与之相对，四极铁、螺线管、二极铁、校正铁、边缘是 z-code 矩阵模型元件（见[四极铁](#quadrupole)）。

### 首尾的极短漂移段

AVAS 手册要求：多粒子模型的第一个和最后一个元件避免使用矩阵模型元件，可以加一段很短的 drift 解决。例如：

\`\`\`text
start
drift 0.000001 0.02 0
quad 0.1 0.02 0 6
drift 0.2 0.02 0
quad 0.1 0.02 0 -6
drift 0.2 0.02 0
end
\`\`\`

这样首尾都是 t-code 元件。末端那段 drift 同时为输出面留出余量（输出面离 lattice 末端太近会失败，见[模拟](#simulation)一章）。

### 孔径与独立边界

漂移段的 \`R\` 是损失判定用的圆形孔径。如果在 input.txt 中开启 \`boundary 1\`，内核改用 Boundary.txt 中的独立边界判断损失，lattice 里所有元件的半径都不再生效：

\`\`\`avas-params
input:boundary
\`\`\`

## 可调参数与典型取值 {#drift-params}

| 参数 | 含义 | 典型取值（低能质子直线加速器） |
|---|---|---|
| \`L\` | 漂移长度 | 元件间隔 0.05–0.3 m；腔间 0.1–0.5 m；输运线中可达数米 |
| \`R\` | 孔径半径 | 0.01–0.03 m，常取与相邻元件孔径相同 |

选择漂移长度时需要权衡：

- **横向**：两个聚焦元件之间的距离越长，束流在其中长得越大，需要的聚焦越强；对周期结构，长度直接决定每周期的相移（见[四极铁](#quadrupole)一章的 FODO 实验）。
- **纵向**：低能束流的去聚束很快。以示例束流为例（质子 $W = 1.527$ MeV，$\\gamma = 1.00163$，$1/\\gamma^2 = 0.99675$），几乎处于非相对论极限，$L/\\gamma^2 \\approx L$，纵向和横向一样"自由地"散开。
- **孔径**：经验上要求孔径不小于 rms 尺寸的 5–10 倍（取决于束晕和误差预算），否则损失会很敏感。

### 数值例子：示例束流在漂移段中的增长

示例项目 \`examples/hwr010\` 的初始束流：$\\alpha_x = -0.65785$，$\\beta_x = 0.37743$ mm/mrad，$\\varepsilon_{n,x} = 0.13768\\ \\pi$·mm·mrad；$\\beta\\gamma = 0.05708$。初始 rms 尺寸 $\\sigma_x = \\sqrt{\\beta_x \\varepsilon_{n,x}/(\\beta\\gamma)} = 0.954$ mm。$\\alpha_x < 0$，束流在发散（虚腰位于入口上游约 0.17 m 处）。用上面的矩阵计算：

| 漂移长度 | rms $x$ | rms $y$ | rms $z$ | rms 相位宽度（162.5 MHz） |
|---|---|---|---|---|
| 0 | 0.954 mm | 0.958 mm | 1.316 mm | 4.5° |
| 0.3 m | 1.64 mm | 1.64 mm | 1.65 mm | 5.7° |
| 0.5 m | 2.19 mm | 2.19 mm | 2.15 mm | 7.4° |
| 1.0 m | 3.64 mm | 3.65 mm | 3.71 mm | 12.7° |

相位宽度按 $\\Delta\\phi = 360^\\circ \\cdot z/(\\beta\\lambda)$ 换算，$\\beta\\lambda = 105.1$ mm。无聚焦、无空间电荷时 1 m 漂移即可使束团的相位宽度增大到将近 3 倍——这说明低能段必须频繁地横向和纵向聚焦。

## 动力学效应与常见问题 {#drift-dynamics}

### 空间电荷让漂移段"不再自由"

上面的结论只在零流强下成立。有流强时束流自场在漂移段里持续作用：横向和纵向都受到散焦力，束流增长得比零流强时快；空间电荷的非线性部分还会造成 rms 发射度增长和束晕。对低能、强流束流，漂移段越长问题越严重，这是低能段要求结构紧凑的主要原因之一。详细的模型见[空间电荷](#space-charge)一章。

### 去聚束与相位滑移

纵向坐标随 $L\\,\\delta/\\gamma^2$ 变化，因此在两个腔之间：

- 束团中心（参考粒子）到达下一个腔的相位由漂移长度和速度决定：$\\Delta\\phi = 360^\\circ \\cdot L/(\\beta\\lambda)$；
- 束团的相位宽度随漂移长度增长，下一个腔接收到的是更长的束团。

改动腔间漂移长度时，下游腔的相位含义取决于 \`field\` 的 V3 设置：V3 = 0 时内核重新求解同步相位，V3 = 2 时绝对相位不变而入口相位改变（见[射频腔](#rf-cavity)）。

### 常见问题

- **单位**：AVAS 的长度和孔径用 m；从 TraceWin 抄来的 mm 数值会让漂移段长 1000 倍，常见现象是束流迅速全部损失。
- **孔径写 0 或太小**：粒子在第一步就会被判为损失。
- **用 drift 代替缺失的场图**：会去掉该元件的全部聚焦和加速，结果仅用于对比。
- **发射度"变化"**：无空间电荷时漂移段中发射度应守恒；如果改变了，先检查损失（DataSet.txt 第 28 列存活宏粒子数）。

## 动手实验 {#drift-lab}

下面的实验把示例束流的 $x$ 平面参数放进一段漂移（\`f=0\` 表示不加透镜）。拖动 \`L\` 观察 rms 椭圆如何"剪切"：椭圆面积（发射度）不变，倾斜方向随距离改变，$\\alpha$ 由负变得更负，rms 尺寸增长而 rms 发散角不变。

\`\`\`avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.5
\`\`\`

再试一试把 \`alpha\` 改为正值（例如 1.5）：束流先会聚到束腰再发散，可以验证腰的位置 $s_w = \\alpha_0/\\gamma_0$。

要看纵向去聚束，可以在可视化编辑器中建一段只有 \`drift\` 的 lattice，打开线性包络预览观察 rms $z$；再用内核运行同一结构，比较有无空间电荷（input.txt 中 \`spacecharge 0/1\`）时的差别。

## 延伸阅读 {#drift-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH (2008)：第 7 章横向动力学中的矩阵方法，第 6 章纵向运动。
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH (2008)：第 3 章（无空间电荷的束流光学与聚焦系统）。
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer (2015)：传输矩阵与 Twiss 参数的演化。
- 本手册相关章节：[相空间](#phase-space)、[线性光学](#linear-optics)、[纵向运动](#longitudinal)、[空间电荷](#space-charge)；完整书目见[参考文献](#references)。
`,c=`# Errors and correction

The elements of a real accelerator are never exactly at their design positions, and field strengths and RF amplitudes and phases are off as well. A lattice design must answer two questions: how far do these errors push the beam off, and how much beam do they lose? How many correctors and diagnostics are needed, and how far can they reduce the effect? This chapter first gives linear estimates of the effects of each kind of error, then the method of statistical error studies, and finally goes through AVAS's error commands, the sampling rules, static error correction (\`adjust\` and \`diag_*\`), and running and output.

## Sources and kinds of errors {#errors-types}

| Kind | Examples | AVAS columns |
|---|---|---|
| Element position | transverse offsets $dx$, $dy$, longitudinal offset $dz$ | \`dx\`, \`dy\` (mm), \`dz\` (mm) |
| Element attitude | rotations about the x, y, z axes | \`dφx\`, \`dφy\`, \`dφz\` (°) |
| Static field strength | calibration errors of quadrupole gradients and solenoid fields | \`dG\` (%) |
| RF amplitude and phase | set-point errors and jitter of cavity voltage and phase | \`ke/kb\` (%), \`φs\` (°) |
| Initial beam | entrance position, angle, energy, phase, emittance, current offsets | \`dx dy dφ dx' dy' dE dεx dεy dεz … dIb\` |

By their time behaviour there are two classes:

- **Static errors** stay constant during a run (over a period of time), for example installation offsets and magnet calibration errors. They can be measured with diagnostics and compensated with steerers, cavity re-phasing and similar means.
- **Dynamic errors** vary in time, for example RF amplitude and phase jitter, mechanical vibration and power-supply ripple. They change faster than the correction loops or cannot be measured, are usually not correctable, and can only be controlled by tolerances.

An error study samples the statistical distribution of the errors many times (many "seeds"), tracks the beam each time, and collects the distribution of figures of merit such as losses, emittance growth and centroid offsets.

## Linear effects of errors {#errors-effects}

The estimates below use linear optics and neglect space charge; they give orders of magnitude and a check of simulation results.

### Transverse quadrupole offsets: centroid offsets

With a quadrupole offset by $d$ the particles feel the field centred on $d$: $x'' = -k\\,(x - d)$. For a thin lens this is an ideal quadrupole plus a deflection

$$
\\Delta x' = k L_q\\, d = \\frac{d}{f}
$$

i.e. in its focusing plane the beam is pulled towards the actual quadrupole centre. Deflections $\\theta_i$ produce at a downstream position $s$ the centroid offset

$$
x_c(s) = \\sum_i \\sqrt{\\beta(s)\\,\\beta_i}\\,\\sin\\!\\big(\\psi(s) - \\psi_i\\big)\\,\\theta_i
$$

For independent zero-mean offsets the ensemble average gives $\\langle\\sin^2\\rangle = 1/2$ and

$$
\\langle x_c^2(s)\\rangle^{1/2} = \\sqrt{\\frac{\\beta(s)}{2}\\sum_i \\beta_i\\,\\langle\\theta_i^2\\rangle}
$$

Example: the FODO of [focusing periods](#lattice-design-periods) ($G = 5\\ \\mathrm{T/m}$, $kL_q = 2.80\\ \\mathrm{m^{-1}}$), 10 periods with 20 quadrupoles, each with a Gaussian x offset of rms $0.1\\ \\mathrm{mm}$. With $\\sigma_\\theta = 0.28\\ \\mathrm{mrad}$, $\\beta = 1.35$ and $0.45\\ \\mathrm{m}$ at the F and D centres and $\\beta = 1.26\\ \\mathrm{m}$ at the end (F entrance), the formula gives an rms centroid offset of about $0.94\\ \\mathrm{mm}$ at the end; a Monte Carlo of 4000 samples with transfer matrices gives $0.97\\ \\mathrm{mm}$. The rms beam size there is about $1.74\\ \\mathrm{mm}$ — an installation error of only 0.1 mm moves the centroid by more than half the rms size, which is why long lattices need orbit correction. The centroid offset grows as $\\sqrt{N}$ and is damped adiabatically as $1/\\sqrt{\\beta\\gamma}$ by acceleration.

### Rotation errors: coupling

A quadrupole rotated by $\\varphi_z$ about z acquires a skew-quadrupole component (about $2\\varphi_z$ times the normal component) that couples x and y and increases the projected emittances. Rotations about x or y displace the element ends transversely by about $\\pm L\\varphi/2$, an effect similar to but smaller than offsets. A solenoid couples the planes anyway; its offsets and rotations also deflect the beam.

### Field errors: mismatch

A relative gradient error $\\Delta G/G$ is a thin-lens error $\\Delta(1/f) = kL_q\\,\\Delta G/G$. From the section on the [mismatch factor](#matching-mismatch-factor), a matched beam passing it gets

$$
M \\approx \\frac{\\beta\\, kL_q}{2}\\,\\frac{\\Delta G}{G}
$$

Example: a 1 % gradient error in one F quadrupole of the FODO above gives $M \\approx 1.35 \\times 2.80 \\times 0.01/2 = 0.019$. The mismatch from many independent small errors adds roughly in quadrature. The phase advance changes as well, by about $\\Delta\\sigma \\approx \\tfrac12\\sum\\beta_i\\,\\Delta(1/f)_i$ (radians) per period.

### RF amplitude and phase errors

For one cavity, in the thin-gap approximation,

$$
\\Delta W = qV_0T\\cos\\varphi_s\\,\\frac{\\Delta V}{V},\\qquad
\\Delta W \\approx -qV_0T\\sin\\varphi_s\\,\\Delta\\varphi
$$

Example: for the example HWR at $\\varphi_s = -33^\\circ$ with $V_0T \\approx 0.405\\ \\mathrm{MV}$, a 1 % amplitude error gives $\\Delta W \\approx 3.4\\ \\mathrm{keV}$ and a $1^\\circ$ phase error $\\Delta W \\approx 3.8\\ \\mathrm{keV}$, small compared with the energy half-width of the longitudinal acceptance (about $\\pm 0.13\\ \\mathrm{MeV}$, see [lattice design](#lattice-design-longitudinal)). With longitudinal focusing, energy and phase errors show up as an oscillation of the bunch centre around the synchronous particle; if they are too large, particles leave the separatrix, are lost longitudinally first and then transversely because of their energy offset. At low energy the velocity is sensitive to the energy, so an energy error shifts the arrival phase at the downstream cavities; chains of independently phased cavities are particularly sensitive to this.

### Initial beam errors

Entrance position and angle offsets cause transverse centroid oscillations (with the same propagation formula as element offsets), energy and phase offsets cause longitudinal centroid oscillations, and emittance and current errors show up mainly as mismatch.

## Static and dynamic errors in AVAS {#errors-static-dynamic}

For every error run (one "run" of one "group") AVAS's Python code re-reads the lattice from the input snapshot, draws all errors for the group, writes the drawn values as kernel commands and runs the kernel.

> AVAS convention: the kernel only knows the dynamic error keywords (\`err_*_ncpl_dyn\`, \`err_beam_dyn\` and the \`*_dyn_on\` switches). A static error study rewrites the drawn static errors as dynamic error commands with fixed values. The difference between "static" and "dynamic" in AVAS is therefore not in the kernel but in the workflow:

- \`error_type = stat\`: only static errors (\`*_stat\` commands) are drawn, dynamic error commands are ignored; \`adjust\` correction can run.
- \`error_type = dyn\`: only dynamic errors are drawn, static error commands are ignored; no correction.
- \`error_type = stat_dyn\`: both are drawn. A correction (if any) "sees" only the static errors — the lattice written for the kernel during the optimisation contains only static errors; once the correction is fixed, the static and dynamic errors of each element are added column by column into one dynamic error command, the switches are the union of both, and the kernel runs. This corresponds to the physical picture "tune the correctors on the static errors, then add the uncorrectable jitter".

Whether the error values stay constant throughout the tracking of one run is up to the kernel and not described in the manual; from the commands Python writes, each run has one fixed set of values.

## Statistical error studies {#errors-statistics}

### Why many seeds

Errors are random and a single run is one sample. Figures of merit such as losses and emittance growth often have skewed, long-tailed distributions, and looking only at the mean underestimates the risk. The usual procedure:

1. Study each kind of error alone: scan its amplitude from small to large and find the amplitude at which the figure of merit (loss rate, emittance growth, centroid offset) reaches the acceptable value — the individual tolerance.
2. Combine all individual tolerances in a global error study, confirm that the total effect stays within budget, and tighten individual tolerances where needed.
3. Repeat with correction to obtain "corrected tolerances".

### Sample size

- The statistical error of a mean is about $\\sigma/\\sqrt{N}$; estimating the tail of a distribution (for example the 99 % quantile) needs far more samples than estimating the mean.
- Loss rate: if none of $N$ runs loses particles, the probability that a run has losses is below about $3/N$ at 95 % confidence (the "rule of three").
- A uniform distribution on $[-A, A]$ has rms $A/\\sqrt3$, a Gaussian has rms σ. Compare results of different distributions at equal rms.

### Scanning the amplitude with groups

AVAS's \`err_step a b\` divides the study into a groups of b runs each. Group g uses $g/a$ times the given amplitudes (except the fixed-value type, see the next section), so one study yields a "figure of merit versus error amplitude" curve: the last group has the full given amplitude, and the b runs of each group give the statistics at that amplitude.

## AVAS error commands {#errors-commands}

Error commands are written in the lattice file (between \`start\` and \`end\`, usually at the beginning) and act on the elements **below** the command.

### Groups and initial beam errors

\`\`\`avas-params
err_step err_beam_dyn err_beam_stat
\`\`\`

- \`err_step a b\`: a groups of b runs; required for an error study.
- \`err_beam_dyn r dx dy dφ dx' dy' dE dεx dεy dεz mx my mz dIb\`: dynamic errors of the initial beam, \`r\` is the distribution type; the parameters need not all be written, trailing empty columns carry no error.
- \`err_beam_stat 0 …\`: the static version; the manual says to write 0 as the first parameter.

> Note: the manual does not describe \`mx\`, \`my\`, \`mz\`. The current code interprets the first parameter of \`err_beam_stat\` like \`r\` of \`err_beam_dyn\`; writing 0 gives fixed-value errors, consistent with the manual.

### Static-field element and RF cavity errors

\`\`\`avas-params
err_quad_ncpl_stat err_quad_cpl_stat err_cav_ncpl_stat err_cav_cpl_stat
\`\`\`

The dynamic versions \`err_quad_ncpl_dyn\`, \`err_quad_cpl_dyn\`, \`err_cav_ncpl_dyn\`, \`err_cav_cpl_dyn\` have the same columns:

\`\`\`text
err_quad_ncpl_stat  N  r  dx  dy  dφx  dφy  dφz  dG  dz  Nb
err_cav_ncpl_stat   N  r  dx  dy  dφx  dφy  ke/kb  φs  dz  Nb
\`\`\`

- \`N\`: acts on the N **elements** below the command. All elements are counted (drifts, field maps, matrix elements, diagnostics), but the errors go only to elements of the right type; write a large value to cover all elements.
- Static-element errors (\`err_quad_*\`) go to \`quad\` elements and static magnetic field maps (\`field\` type 3, e.g. solenoid maps); RF cavity errors (\`err_cav_*\`) go to RF field maps (\`field\` type 1). \`solenoid\` matrix elements and static electric maps (type 2) take neither.
- \`Nb\` is not described in the manual and is not sampled.

> Note: the current code leaves the **last column** of an element error command unsampled, as written. With the command written up to \`Nb\` that column is \`Nb\`; if you stop at \`dz\` and omit \`Nb\`, \`dz\` becomes the last column and is neither sampled nor scaled with the group. When you need \`dz\` errors, write \`Nb\` too (for example 0).

### Switches

\`\`\`avas-params
err_quad_stat_on err_cav_stat_on err_beam_stat_on
\`\`\`

\`err_quad_dyn_on\`, \`err_cav_dyn_on\` and \`err_beam_dyn_on\` work the same way. Each column is 0 or 1, in the order of the error columns of the corresponding command; columns switched off carry no error. An error study needs at least one switch column set to 1 for its mode, otherwise it fails (Missing … error on command).

### An example

\`\`\`text
start
err_step 3 20
err_quad_stat_on 1 1 0 0 0 0 0
err_quad_ncpl_stat 1000 1 0.1 0.1
err_cav_dyn_on 0 0 0 0 1 1 0
err_cav_ncpl_dyn 1000 2 0 0 0 0 1.0 1.0
drift 0.05 0.02 0
...
end
\`\`\`

Meaning: 3 groups of 20 runs. All quadrupoles / static magnetic maps get uniformly distributed x and y offsets, $[-0.1, 0.1]\\ \\mathrm{mm}$ in group 3 and $[-0.033, 0.033]\\ \\mathrm{mm}$ in group 1; all RF cavities get Gaussian dynamic amplitude and phase errors with standard deviations 1 % and $1^\\circ$ in group 3. Run in \`stat_dyn\` mode both act; \`stat\` gives only the offsets, \`dyn\` only the RF errors.

## Sampling rules {#errors-sampling}

### Distribution type r

| r | Meaning | Column with given value A in group g (of a) |
|---|---|---|
| 0 | fixed value | always A, not scaled with the group |
| 1 | uniform | uniform in $[-A g/a,\\ A g/a]$ |
| 2 | Gaussian | mean 0, standard deviation $A g/a$ (not truncated) |
| -1 | equal steps | $A g/a$ (TraceWin's type 0) |

\`\`\`avas-params
err_quad_ncpl_dyn
\`\`\`

> Note: for initial beam errors the current code computes equal steps (r = −1) as $A(g+1)/a$, unlike element errors; check the generated error records before relying on it. Drawn values are rounded to 5 decimals.

### Coupled and uncoupled

- \`ncpl\` (uncoupled): every affected element is sampled **independently**.
- \`cpl\` (coupled): each command is sampled once and the same values go to all elements it covers, modelling common supports, common power supplies and other collective errors.
- When an element is covered by several commands of the same family (static or dynamic, quadrupole or cavity), the current code keeps only the one closest to the element (the lowest); they do not add.

### Random seeds

- \`[error] seed\` in \`ini.ini\` ("Seed" under "Error study" on the Settings page, \`--seed\` on the command line) drives the error sampling (and the random start of the correction). The same seed and lattice give the same error sequence.
- \`randomseed\` (listed under \`beam.txt\` in the manual, written in \`input.txt\` in the example project) is the random seed used to generate the initial particle distribution, a setting independent of the error seed. Fix both when comparing two error studies.

## Static error correction {#errors-correction}

### Commands

\`\`\`avas-params
adjust diag_energy diag_size diag_position
\`\`\`

- \`adjust 0 v n min max first_step\`: placed directly in front of the element to tune; makes its v-th parameter a knob (counted from the first parameter after the keyword: the phase of \`field\` is the 6th, \`Ke\` the 7th, \`Kb\` the 8th; the gradient of \`quad\` the 4th; \`Bx\`/\`By\` of \`steerer\` the 4th/5th), limited to \`[min, max]\`; knobs with the same non-zero \`n\` are constrained to one value; \`first_step = 1\` starts from the element's current value, 0 from a random value inside the range.
- \`diag_position 0 x y 0\`: target beam centre at its position (mm).
- \`diag_size 0 sx sy 0\`: target rms sizes at its position (mm).
- \`diag_energy 0 W 0\`: target energy at its position (MeV).
- \`diag_*\` are zero-length elements marking where the targets are; they are not passed to the kernel, Python reads the beam parameters at their positions from DataSet.

### Algorithm

The following describes the current Python code (\`avas/sim/err_adjust.py\`); the manual does not document these details and they may change:

1. A correction runs only when an \`adjust\` and a \`diag_*\` have the same first parameter (described in the manual as "meaningless, write 0"; writing 0 everywhere makes one group). Different numbers are corrected one after another in increasing order, each starting from the lattice corrected by the previous one.
2. Each group is optimised with SLSQP: every evaluation writes the knobs into the lattice, runs the kernel up to just after the last diagnostic and computes the objective — the mean squared deviation over the group's diagnostics: position $(x_c - x)^2 + (y_c - y)^2$ (mm²), size $(x_{\\mathrm{rms}} - s_x)^2 + (y_{\\mathrm{rms}} - s_y)^2$ (mm²), energy $(W - W_{\\mathrm{target}})^2$ (MeV²).
3. It stops as soon as the objective is below 0.05; at most 100 iterations, finite-difference step 0.1 (in the knob's own unit), tolerance $10^{-4}$.
4. The correction is repeated for every error run; the knob values go to \`Adjust_Datas_<group>_<run>.txt\` and the beam parameters at the diagnostics to \`par_diag_datas_<group>_<run>.txt\`. The kernel runs of the optimisation are kept under \`error_adjust/\`. The values are not written back to the lattice file.

> Note: the finite-difference step is fixed at 0.1, so for knobs whose useful range is much smaller than 0.1 (for example steerer fields in T, ±0.01 T) the gradient estimate is unreliable; the threshold 0.05 has the units of the target type (mm² or MeV²) and may be loose for small beams. The energy-correction example of the manual reproduced in the case tutorials (target 5 MeV) does not reach its target (the recorded final energy stays at about $2.3\\ \\mathrm{MeV}$) — a finished run is not a successful correction; always check the diagnostic data after correction.

### Orbit correction

The classic "one-to-one" orbit correction puts a steerer and a position diagnostic near each quadrupole and uses the steerer to bring the centroid at the downstream diagnostic back to zero. By the estimate above, offsets of order 0.1 mm already call for correction. In AVAS it looks like this (a sketch not validated with the kernel, showing only how the commands go together):

\`\`\`text
adjust 0 4 0 -0.005 0.005 1
adjust 0 5 0 -0.005 0.005 1
steerer 0 0.02 0 0 0 0 0.005
field 0.35 0.02 0 3 0 0 1 0.38 sol
drift 0.05 0.02 0
diag_position 0 0 0 0
\`\`\`

The two \`adjust\` lines make the steerer's \`Bx\` and \`By\` knobs; the steerer acts over the next element (the solenoid map), and \`diag_position\` asks for the downstream centre at (0, 0). Mind the field unit T and the finite-difference step above; check the convergence on a small study first.

## Running error studies and their output {#errors-running}

### Choosing the mode

The \`[error]\` section of \`ini.ini\`:

| Key | Meaning |
|---|---|
| \`error_type\` | empty = normal run; \`stat\`, \`dyn\`, \`stat_dyn\` = the corresponding error study |
| \`seed\` | random seed of the error sampling |
| \`if_normal\` | 1 = run an error-free reference first |

In the GUI: under "Error study" on the Settings page choose None / Static / Dynamic / Static + dynamic and fill in "Seed" (GUI runs always do the error-free reference first); then write \`err_step\`, the error commands and the switches in the lattice, save and run. On the command line use \`avas run --input … --output … --mode stat|dyn|stat_dyn --seed 7\`, with \`--no-normal\` to skip the reference run; \`--mode auto\` (the default) reads \`ini.ini\`. The AI assistant's \`set_error_study\` tool changes the same two settings (with the user's approval).

Like a full run, an error study runs on the \`inputs/\` snapshot in the output folder and does not rewrite the project's InputFile; the input files are read-only while it runs (including pauses).

### Output files

| Path (relative to the output folder) | Contents |
|---|---|
| \`error_output/output_0_0/\` | the error-free reference run |
| \`error_output/output_<group>_<run>/\` | complete output of each error run (DataSet.txt etc., and the lattice.txt used) |
| \`error_middle/output_0/\` | the run in progress, moved to \`error_output\` when finished |
| \`errors_par_tot.txt\` | one row per run: loss rate, emittance growth $\\varepsilon_{\\mathrm{out}}/\\varepsilon_{\\mathrm{in}} - 1$ in three planes, final centroid and angle, rms sizes and divergences, energy difference to the reference run, final Twiss parameters |
| \`errors_par.txt\` | one row per group: means over the group's runs of the quantities from loss ratio to energy difference (not the Twiss parameters), and root-mean-square spreads of centre, angle, size, divergence and energy difference |
| \`Error_Datas_<group>_<run>.txt\` | the actual error values applied to each cavity / quadrupole in that run |
| \`Adjust_Datas_<group>_<run>.txt\`, \`par_diag_datas_<group>_<run>.txt\` | correction values, beam parameters at the diagnostics |
| \`density_par_*.dat\`, \`density_tot_par*.dat\` | particle density statistics (need \`pchistogram 1 …\` in \`input.txt\`) |

A run whose beam does not reach the end of the lattice (last DataSet position more than 5 cm short of the total length) is recorded with loss rate 1 and zeros in the other columns.

### Viewing in the GUI

- The Run page shows the envelope of the current seed live and overlays the finished seeds as an envelope band ("Finished error seeds"), which shows the spread caused by the errors at a glance.
- The "Error study" group of the Results page has "Emittance growth and loss" (from \`errors_par*.txt\`) and "Output parameters" (with the mean or root-mean-square statistic) plots.
- Detailed results of single runs can be viewed by choosing the corresponding \`error_output/output_<group>_<run>\` folder on the Results page.

## Hands-on {#errors-lab}

The linear effects of errors can be estimated with the earlier labs. In the FODO lab change \`G\` from 5 to 5.05 (a 1 % gradient error on all quadrupoles) and compare the phase advance per period and the periodic β:

\`\`\`avas-lab
fodo Lq=0.1 G=5.05 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
\`\`\`

In the RF cavity lab change the synchronous phase from $-33^\\circ$ to $-32^\\circ$, or \`Ke\` by 1 %, compare the exit energies and check them against the $\\Delta W$ estimates of this chapter:

\`\`\`avas-lab
rf-cavity phase=-32 Ke=1.36 W=1.527
\`\`\`

Real statistical studies need the kernel: copy the example project to a practice folder, add \`err_step\` and error commands as in the example above, choose the error mode on the Settings page and run.

## Further reading {#errors-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: errors and tolerances, estimates of beam centroid offsets.
- S. Y. Lee, *Accelerator Physics*; H. Wiedemann, *Particle Accelerator Physics*: the linear theory of orbit distortion and correction.
- The chapters on error analysis and static error correction of the AVAS user manual (\`docs/使用说明20260427.docx\`), and the error commands in the in-app "Parameter and file reference".
- The full bibliography is in [references](#references).
`,l='# 误差与校正\n\n真实加速器的元件不会精确地放在设计位置，场强和射频幅值、相位也有偏差。设计 lattice 时必须回答两个问题：这些误差会让束流偏离多少、损失多少？需要多少校正元件和束诊、能把误差影响压到多低？本章先给出各类误差的线性效应估计，再介绍统计误差研究的方法，最后逐条说明 AVAS 的误差命令、抽样规则、静态误差校正（`adjust` 与 `diag_*`）以及运行和输出。\n\n## 误差的来源与分类 {#errors-types}\n\n| 类别 | 例子 | AVAS 中的列 |\n|---|---|---|\n| 元件位置 | 横向位移 $dx$、$dy$，纵向位移 $dz$ | `dx`、`dy`（mm），`dz`（mm） |\n| 元件姿态 | 绕 x、y、z 轴的转角 | `dφx`、`dφy`、`dφz`（°） |\n| 静态场强 | 四极铁梯度、螺线管场的标定误差 | `dG`（%） |\n| 射频幅值与相位 | 腔压、腔相位的设定误差与抖动 | `ke/kb`（%）、`φs`（°） |\n| 初始束流 | 入口位置、角度、能量、相位、发射度、流强偏差 | `dx dy dφ dx\' dy\' dE dεx dεy dεz … dIb` |\n\n按时间特性分为两类：\n\n- **静态误差**：在一次运行（一段时间）内不变，例如安装位置误差、磁铁标定误差。它们可以用束诊测量，并用校正铁、腔体调相等方法补偿。\n- **动态误差**：随时间变化，例如射频幅值和相位抖动、机械振动、电源纹波。它们变化比校正回路快或不可测，通常无法补偿，只能靠公差控制。\n\n一次误差研究就是对误差的统计分布做许多次随机抽样（多个"种子"），每次跟踪束流，统计损失、发射度增长、中心偏移等指标的分布。\n\n## 误差的线性效应 {#errors-effects}\n\n下面的估计都基于线性光学，忽略空间电荷，用来判断量级和检验模拟结果是否合理。\n\n### 四极铁横向位移：中心轨道偏移\n\n四极铁横向偏移 $d$ 时，粒子感受到以 $d$ 为中心的场：$x\'\' = -k\\,(x - d)$。对薄透镜，这相当于在理想四极铁上叠加一个偏转\n\n$$\n\\Delta x\' = k L_q\\, d = \\frac{d}{f}\n$$\n\n即束流在聚焦平面被拉向四极铁的实际中心。偏转 $\\theta_i$ 在下游位置 $s$ 产生的中心偏移为\n\n$$\nx_c(s) = \\sum_i \\sqrt{\\beta(s)\\,\\beta_i}\\,\\sin\\!\\big(\\psi(s) - \\psi_i\\big)\\,\\theta_i\n$$\n\n各四极铁位移独立、均值为零时，对系综平均 $\\langle\\sin^2\\rangle = 1/2$，得到\n\n$$\n\\langle x_c^2(s)\\rangle^{1/2} = \\sqrt{\\frac{\\beta(s)}{2}\\sum_i \\beta_i\\,\\langle\\theta_i^2\\rangle}\n$$\n\n例：[聚焦周期](#lattice-design-periods) 中的 FODO（$G = 5\\ \\mathrm{T/m}$，$kL_q = 2.80\\ \\mathrm{m^{-1}}$），10 个周期 20 块四极铁，每块 x 位移为 rms $0.1\\ \\mathrm{mm}$ 的高斯分布。$\\sigma_\\theta = 0.28\\ \\mathrm{mrad}$，F、D 铁中心处 $\\beta = 1.35$、$0.45\\ \\mathrm{m}$，末端（F 铁入口）$\\beta = 1.26\\ \\mathrm{m}$，公式给出末端中心偏移 rms 约 $0.94\\ \\mathrm{mm}$；用传输矩阵做 4000 次蒙特卡罗抽样得到 $0.97\\ \\mathrm{mm}$。同一位置束流 rms 尺寸约 $1.74\\ \\mathrm{mm}$——仅 0.1 mm 的安装误差就让中心偏移达到 rms 尺寸的一半以上，这就是为什么长 lattice 需要轨道校正。中心偏移随 $\\sqrt{N}$ 增长，加速时按 $1/\\sqrt{\\beta\\gamma}$ 绝热衰减。\n\n### 转角误差：耦合\n\n四极铁绕 z 轴转角 $\\varphi_z$ 后产生斜四极分量（强度约为 $2\\varphi_z$ 乘以正常分量），把 x、y 两个平面耦合起来，投影发射度增大。绕 x、y 轴的转角使元件两端横向位移约 $\\pm L\\varphi/2$，效果与位移误差类似但较小。螺线管本身就耦合两个平面，其位移和转角误差还会产生偏转。\n\n### 场强误差：失配\n\n四极铁梯度相对误差 $\\Delta G/G$ 等价于薄透镜误差 $\\Delta(1/f) = kL_q\\,\\Delta G/G$。由 [失配因子](#matching-mismatch-factor) 一节，匹配束经过它后\n\n$$\nM \\approx \\frac{\\beta\\, kL_q}{2}\\,\\frac{\\Delta G}{G}\n$$\n\n例：上面的 FODO 中一块 F 铁有 1 % 梯度误差，$M \\approx 1.35 \\times 2.80 \\times 0.01/2 = 0.019$。许多独立的小误差引起的失配大致按平方和累积。相移本身也改变：每周期相移的变化约为 $\\Delta\\sigma \\approx \\tfrac12\\sum\\beta_i\\,\\Delta(1/f)_i$（弧度）。\n\n### 射频幅值与相位误差\n\n对一个腔，薄间隙近似下\n\n$$\n\\Delta W = qV_0T\\cos\\varphi_s\\,\\frac{\\Delta V}{V},\\qquad\n\\Delta W \\approx -qV_0T\\sin\\varphi_s\\,\\Delta\\varphi\n$$\n\n例：示例 HWR 在 $\\varphi_s = -33^\\circ$、$V_0T \\approx 0.405\\ \\mathrm{MV}$ 时，1 % 幅值误差给出 $\\Delta W \\approx 3.4\\ \\mathrm{keV}$，$1^\\circ$ 相位误差给出 $\\Delta W \\approx 3.8\\ \\mathrm{keV}$，与纵向接受度的能量半宽（约 $\\pm 0.13\\ \\mathrm{MeV}$，见 [lattice 设计](#lattice-design-longitudinal)）相比很小。有纵向聚焦时，能量和相位误差表现为束团中心绕同步粒子的纵向振荡；误差太大时粒子越出分界线，先在纵向丢失，随后因能量偏差在横向丢失。低能段速度对能量敏感，能量误差会使粒子到达下游腔的相位偏移，独立相控的腔链对此尤其敏感。\n\n### 初始束流误差\n\n入口位置和角度偏差引起横向中心振荡（与位移误差同样的传播公式），能量和相位偏差引起纵向中心振荡，发射度和流强偏差则主要表现为失配。\n\n## 静态与动态误差在 AVAS 中的实现 {#errors-static-dynamic}\n\nAVAS 的每一次误差运行（一个"组"中的一"次"）都由 Python 重新从输入快照读取 lattice，按本次的组号抽取所有误差，把抽到的数值写成内核命令后运行内核。\n\n> AVAS 约定：内核只认识动态误差关键字（`err_*_ncpl_dyn`、`err_beam_dyn` 与 `*_dyn_on` 开关）。静态误差研究把抽到的静态误差改写成固定数值的动态误差命令交给内核。因此"静态"与"动态"在 AVAS 中的区别不在内核里，而在于流程：\n\n- `error_type = stat`：只抽取静态误差（`*_stat` 命令），动态误差命令被忽略；可以执行 `adjust` 校正。\n- `error_type = dyn`：只抽取动态误差，静态误差命令被忽略；不执行校正。\n- `error_type = stat_dyn`：两者都抽取。校正（若有）只"看到"静态误差——优化时写给内核的 lattice 只含静态误差；校正结果确定后，同一元件的静态和动态误差逐列相加写成一条动态误差命令，开关取两者的并集，再运行内核。这正对应物理上"先根据静态误差调好校正元件，再叠加无法补偿的抖动"。\n\n每次运行中误差数值在整条束流跟踪过程中是否保持不变由内核决定，手册未说明；按 Python 写给内核的命令，每次运行只有一组固定数值。\n\n## 统计误差研究 {#errors-statistics}\n\n### 为什么要多个种子\n\n误差是随机的，单次运行只是一个样本。损失、发射度增长这类指标的分布往往偏斜、有长尾，只看平均值会低估风险。一般做法：\n\n1. 对每类误差单独研究：从小到大扫描幅度，找出使指标（如损失率、发射度增长、中心偏移）达到容许值的幅度，作为单项公差。\n2. 把各项公差合在一起做综合误差研究，确认总效果仍在预算内，必要时收紧个别公差。\n3. 引入校正后重复以上步骤，得到"校正后公差"。\n\n### 样本量\n\n- 均值的统计误差约为 $\\sigma/\\sqrt{N}$；要估计分布尾部（例如 99 % 分位）需要远多于估计均值的样本。\n- 损失率：$N$ 次运行都无损失时，单次运行有损失的概率在 95 % 置信度下上限约为 $3/N$（"三法则"）。\n- 均匀分布在 $[-A, A]$ 的 rms 为 $A/\\sqrt3$，高斯分布的 rms 为 σ。比较不同分布的结果时应按 rms 换算。\n\n### 分组扫描幅度\n\nAVAS 的 `err_step a b` 把研究分成 a 组、每组 b 次。第 g 组的误差幅度为给定幅度的 $g/a$ 倍（固定值类型除外，见下节），所以一次研究就能得到"指标—误差幅度"曲线：最后一组是给定的满幅度，每组 b 次给出该幅度下的统计。\n\n## AVAS 误差命令 {#errors-commands}\n\n误差命令写在 lattice 文件中（`start` 与 `end` 之间，通常放在开头），作用于命令**下面**的元件。\n\n### 分组与初始束流误差\n\n```avas-params\nerr_step err_beam_dyn err_beam_stat\n```\n\n- `err_step a b`：a 组，每组 b 次；误差研究必须有它。\n- `err_beam_dyn r dx dy dφ dx\' dy\' dE dεx dεy dεz mx my mz dIb`：初始束流的动态误差，`r` 为分布类型；参数可以不写满，后面空着的列没有误差。\n- `err_beam_stat 0 …`：静态版本，手册规定第一个参数写 0。\n\n> 注意：`mx`、`my`、`mz` 手册未说明含义。当前代码对 `err_beam_stat` 的第一个参数按与 `err_beam_dyn` 的 `r` 相同的方式解释，写 0 即为固定值误差，与手册一致。\n\n### 静磁元件与射频腔误差\n\n```avas-params\nerr_quad_ncpl_stat err_quad_cpl_stat err_cav_ncpl_stat err_cav_cpl_stat\n```\n\n动态版本 `err_quad_ncpl_dyn`、`err_quad_cpl_dyn`、`err_cav_ncpl_dyn`、`err_cav_cpl_dyn` 的列相同：\n\n```text\nerr_quad_ncpl_stat  N  r  dx  dy  dφx  dφy  dφz  dG  dz  Nb\nerr_cav_ncpl_stat   N  r  dx  dy  dφx  dφy  ke/kb  φs  dz  Nb\n```\n\n- `N`：作用于命令下面的 N 个**元件**。计数包括所有元件（漂移段、场图、矩阵元件、束诊），但误差只加到合适类型的元件上；想作用于全部元件可写一个很大的值。\n- 静磁元件误差（`err_quad_*`）加到 `quad` 和静磁场图（`field` 类型 3，如螺线管场图）上；射频腔误差（`err_cav_*`）加到射频场图（`field` 类型 1）上。`solenoid` 矩阵元件和静电场图（类型 2）不接受这两类误差。\n- `Nb` 手册未说明，抽样时不处理。\n\n> 注意：当前代码对元件误差命令的**最后一列**不做抽样、原样保留。写满到 `Nb` 时这一列就是 `Nb`；若写到 `dz` 为止而省略 `Nb`，`dz` 会成为最后一列，不随分布抽样、也不随组号缩放。需要 `dz` 误差时请把 `Nb` 也写上（例如 0）。\n\n### 开关\n\n```avas-params\nerr_quad_stat_on err_cav_stat_on err_beam_stat_on\n```\n\n`err_quad_dyn_on`、`err_cav_dyn_on`、`err_beam_dyn_on` 同理。每列只能写 0 或 1，顺序与对应误差命令的误差列一致；开关为 0 的列不加误差。误差研究要求该模式下至少有一个开关列为 1，否则报错（Missing … error on command）。\n\n### 一个例子\n\n```text\nstart\nerr_step 3 20\nerr_quad_stat_on 1 1 0 0 0 0 0\nerr_quad_ncpl_stat 1000 1 0.1 0.1\nerr_cav_dyn_on 0 0 0 0 1 1 0\nerr_cav_ncpl_dyn 1000 2 0 0 0 0 1.0 1.0\ndrift 0.05 0.02 0\n...\nend\n```\n\n含义：3 组、每组 20 次。所有四极铁 / 静磁场图在 x、y 有均匀分布位移，第 3 组为 $[-0.1, 0.1]\\ \\mathrm{mm}$，第 1 组为 $[-0.033, 0.033]\\ \\mathrm{mm}$；所有射频腔有高斯分布的幅值和相位动态误差，第 3 组标准差为 1 % 与 $1^\\circ$。用 `stat_dyn` 模式运行时两者同时生效；用 `stat` 只有位移误差，用 `dyn` 只有射频误差。\n\n## 抽样规则 {#errors-sampling}\n\n### 分布类型 r\n\n| r | 含义 | 第 g 组（共 a 组）给定值 A 的列 |\n|---|---|---|\n| 0 | 固定值 | 每次都取 A，不随组号缩放 |\n| 1 | 均匀分布 | 在 $[-A g/a,\\ A g/a]$ 内均匀抽样 |\n| 2 | 高斯分布 | 均值 0、标准差 $A g/a$（不截断） |\n| -1 | 等步长 | 取 $A g/a$（等价于 TraceWin 的 0 类型） |\n\n```avas-params\nerr_quad_ncpl_dyn\n```\n\n> 注意：初始束流误差的等步长（r = −1）在当前代码中按 $A(g+1)/a$ 计算，与元件误差不同；使用前请核对生成的误差记录。抽到的数值保留 5 位小数。\n\n### 耦合与非耦合\n\n- `ncpl`（非耦合）：每个受影响的元件**各自独立**抽样。\n- `cpl`（耦合）：每条命令只抽一次，同样的数值加到它覆盖的全部元件上，用来模拟共同的支架、共同的电源等整体误差。\n- 同一元件被多条同类（静态或动态）四极铁 / 腔误差命令覆盖时，当前代码只保留离元件最近（最靠下）的一条，不叠加。\n\n### 随机种子\n\n- `ini.ini` 的 `[error] seed`（设置页"误差分析"中的"随机种子"，命令行 `--seed`）决定误差抽样（以及校正的随机初值）。同一种子、同一 lattice 给出相同的误差序列。\n- `randomseed`（手册列在 `beam.txt` 中，示例项目写在 `input.txt` 中）是生成初始粒子分布的随机数种子，与误差种子是两个独立的设置。比较两次误差研究时，两者都要固定。\n\n## 静态误差校正 {#errors-correction}\n\n### 命令\n\n```avas-params\nadjust diag_energy diag_size diag_position\n```\n\n- `adjust 0 v n min max first_step`：紧放在要调的元件前，把该元件的第 v 个参数设为旋钮（从关键字后第 1 个参数数起：`field` 的相位为第 6 个、`Ke` 第 7 个、`Kb` 第 8 个；`quad` 的梯度第 4 个；`steerer` 的 `Bx`/`By` 第 4/5 个），取值限制在 `[min, max]`；`n` 相同（且非 0）的旋钮被约束为同一值；`first_step = 1` 从元件当前值开始，0 则在范围内随机取初值。\n- `diag_position 0 x y 0`：该位置束流中心的目标（mm）。\n- `diag_size 0 sx sy 0`：该位置 rms 尺寸的目标（mm）。\n- `diag_energy 0 W 0`：该位置能量的目标（MeV）。\n- `diag_*` 是长度为零的元件，标出目标所在位置；它们不传给内核，Python 从 DataSet 中读取该位置的束流参数。\n\n### 算法\n\n以下是当前 Python 代码（`avas/sim/err_adjust.py`）的做法，手册没有记载这些细节，以后可能变化：\n\n1. 只有当某个 `adjust` 与某个 `diag_*` 的第一个参数相同时才执行校正（手册把它写作"无意义，写 0"；全部写 0 即为一组）。不同编号按从小到大依次校正，后一组从前一组校正后的 lattice 开始。\n2. 每组用 SLSQP 优化：每次函数求值把旋钮写入 lattice，运行内核到最后一个束诊之后，计算目标函数——本组各束诊的平均平方偏差：位置 $(x_c - x)^2 + (y_c - y)^2$（mm²），尺寸 $(x_{\\mathrm{rms}} - s_x)^2 + (y_{\\mathrm{rms}} - s_y)^2$（mm²），能量 $(W - W_{\\mathrm{target}})^2$（MeV²）。\n3. 目标函数低于 0.05 即停止；最多 100 次迭代，数值差分步长 0.1（旋钮自身单位），收敛容差 $10^{-4}$。\n4. 每次误差运行都重新校正，校正值写入 `Adjust_Datas_<组>_<次>.txt`，各束诊处的束流参数写入 `par_diag_datas_<组>_<次>.txt`。校正过程的内核运行放在 `error_adjust/` 下。校正值不会写回 lattice 文件。\n\n> 注意：差分步长固定为 0.1，旋钮的有效范围远小于 0.1 时（例如以 T 为单位的校正铁磁场 ±0.01 T）梯度估计不可靠；目标函数阈值 0.05 的量纲依目标类型而定（mm² 或 MeV²），对小束流尺寸可能过松。案例教程中按手册写的能量校正例子（目标 5 MeV）并未达到目标（记录的末能量仍约 $2.3\\ \\mathrm{MeV}$）——运行完成不等于校正成功，务必检查校正后的束诊数据。\n\n### 轨道校正的思路\n\n经典的"一对一"轨道校正：每个四极铁附近放一个校正铁和一个位置束诊，用校正铁把下游束诊处的中心拉回零。按上一节的估计，0.1 mm 量级的安装误差就需要校正。在 AVAS 中的写法示意（未经内核验证，仅说明命令搭配）：\n\n```text\nadjust 0 4 0 -0.005 0.005 1\nadjust 0 5 0 -0.005 0.005 1\nsteerer 0 0.02 0 0 0 0 0.005\nfield 0.35 0.02 0 3 0 0 1 0.38 sol\ndrift 0.05 0.02 0\ndiag_position 0 0 0 0\n```\n\n两条 `adjust` 分别把校正铁的 `Bx`、`By` 设为旋钮，校正铁作用于下一个元件（螺线管场图）；`diag_position` 要求下游中心回到 (0, 0)。注意磁场单位 T 与上面的差分步长问题；实际使用前应先用小规模研究检查收敛情况。\n\n## 运行误差研究与输出 {#errors-running}\n\n### 选择模式\n\n`ini.ini` 的 `[error]` 段：\n\n| 键 | 含义 |\n|---|---|\n| `error_type` | 空 = 普通运行；`stat`、`dyn`、`stat_dyn` = 相应误差研究 |\n| `seed` | 误差抽样的随机种子 |\n| `if_normal` | 1 = 先运行一次无误差的参考 |\n\n在界面中：设置页的"误差分析"选择 无 / 静态 / 动态 / 静态 + 动态，并填写"随机种子"（界面运行总会先跑无误差参考）；然后在结构中写好 `err_step`、误差命令与开关，保存并运行。命令行用 `avas run --input … --output … --mode stat|dyn|stat_dyn --seed 7`，`--no-normal` 跳过参考运行；`--mode auto`（默认）读取 `ini.ini`。AI 助手的 `set_error_study` 工具同样修改这两项（需用户批准）。\n\n误差研究和完整运行一样在输出目录的 `inputs/` 快照上进行，不改写项目的 InputFile；运行期间（包括暂停）输入文件只读。\n\n### 输出文件\n\n| 路径（相对输出目录） | 内容 |\n|---|---|\n| `error_output/output_0_0/` | 无误差参考运行 |\n| `error_output/output_<组>_<次>/` | 每次误差运行的完整输出（DataSet.txt 等，及所用 lattice.txt） |\n| `error_middle/output_0/` | 正在进行的那一次运行，完成后移到 `error_output` |\n| `errors_par_tot.txt` | 每次运行一行：损失率、三个平面的发射度增长 $\\varepsilon_{\\mathrm{out}}/\\varepsilon_{\\mathrm{in}} - 1$、末端中心与角度、rms 尺寸与角散、相对参考运行的能量差、末端 Twiss |\n| `errors_par.txt` | 每组一行：从损失率到能量差（不含 Twiss）各量在本组各次运行上的平均值，以及中心、角度、尺寸、角散与能量差的均方根 |\n| `Error_Datas_<组>_<次>.txt` | 本次加到每个腔 / 四极铁上的具体误差值 |\n| `Adjust_Datas_<组>_<次>.txt`、`par_diag_datas_<组>_<次>.txt` | 校正值、各束诊处的束流参数 |\n| `density_par_*.dat`、`density_tot_par*.dat` | 粒子密度统计（需要 `input.txt` 中 `pchistogram 1 …`） |\n\n束流没有到达 lattice 末端（DataSet 的最后位置比总长短 5 cm 以上）的运行记为损失率 1，其余列为 0。\n\n### 在界面中查看\n\n- 运行页实时显示当前种子的包络，已完成的种子以包络带叠加（"已完成的误差种子"），可以直观看到误差引起的包络散布。\n- 结果页"误差分析"组有"发射度增长与损失"（读 `errors_par*.txt`）和"输出参数"（可选平均值或均方根统计量）两类图。\n- 单次运行的详细结果可以在结果页选择对应的 `error_output/output_<组>_<次>` 目录查看。\n\n## 动手实验 {#errors-lab}\n\n误差的线性效应可以用前面的实验估计。在 FODO 实验中把 `G` 从 5 改为 5.05（1 % 梯度误差，作用于全部四极铁），比较每周期相移和周期 β 的变化：\n\n```avas-lab\nfodo Lq=0.1 G=5.05 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1\n```\n\n在射频腔实验中把同步相位从 $-33^\\circ$ 改为 $-32^\\circ$ 或把 `Ke` 改变 1 %，比较出口能量的变化，并与本章的 $\\Delta W$ 估计对照：\n\n```avas-lab\nrf-cavity phase=-32 Ke=1.36 W=1.527\n```\n\n真正的统计研究要用内核：复制示例项目到练习目录，按上面的例子加入 `err_step` 与误差命令，在设置页选择误差模式后运行。\n\n## 延伸阅读 {#errors-reading}\n\n- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.：误差与公差、束流中心偏移的估计。\n- S. Y. Lee, *Accelerator Physics*；H. Wiedemann, *Particle Accelerator Physics*：轨道畸变与校正的线性理论。\n- AVAS 使用说明（`docs/使用说明20260427.docx`）中误差分析与静态误差校正的章节，以及应用内"参数与文件参考"中的误差命令说明。\n- 完整书目见 [参考文献](#references)。\n',u=`# Field maps

A field map is a table of electric or magnetic field components on a three-dimensional grid, usually computed and exported by an electromagnetic solver (CST, COMSOL, Opera, Superfish …). AVAS's \`field\` element reads a field map and integrates the particle motion with the t-code in the field interpolated from the grid: RF cavities (type 1), electrostatic elements (type 2) and magnetostatic elements (type 3) are all described this way, see [RF cavities](#rf-cavity) and [Static field maps](#static-fields). This chapter explains the file format and units, grid and aperture, the storage order of the component files, interpolation and noise, and how to view and check a map in the GUI.

The example project \`examples/hwr010/InputFile\` contains two maps: the half-wave cavity \`hwr010\` (\`.edx/.edy/.edz/.bdx/.bdy/.bdz\`, 0.21 m long) and the solenoid \`sol\` (\`.bsx/.bsy/.bsz\`, 0.35 m long). \`efield.*\` and \`bfield.*\` are byte-identical copies of these two groups.

## What a field map describes {#field-maps-role}

- **One field map = one group of component files with the same name.** The last parameter of \`field\` is the file name without extension; the kernel reads the extensions belonging to the field type.
- **The shape of the field comes from the files, the amplitude from Ke and Kb.** Electric fields in the files are in MV/m and magnetic fields in T; the field acting on the beam is $K_e\\boldsymbol{E}_{map}$, $K_b\\boldsymbol{B}_{map}$. RF fields are also multiplied by the time factor: $\\cos(\\omega t+\\varphi_0)$ for the electric and $\\sin(\\omega t+\\varphi_0)$ for the magnetic field.
- **Coordinates**: $z$ runs from the element entrance ($z = 0$) to the map length $L$; $x$ and $y$ are transverse coordinates relative to the beam axis, all in m.
- **Data versus schematic**: a field map is data and contains every detail the solver produced (fringe fields, higher-order components, grid noise); the fields the GUI draws for matrix-model elements (\`quad\`, \`solenoid\`, \`bend\`, \`steerer\`) are hard-edge schematics computed from the parameters. The two must not be confused, see [Field maps in the GUI](#field-maps-gui).

## File format and units {#field-maps-format}

### Text format

\`\`\`text
Nz  L
Nx  xmin  xmax
Ny  ymin  ymax
norm
v1
v2
...
\`\`\`

The first four lines are the header: $N_z$, $N_x$, $N_y$ are the **numbers of grid intervals** in the three directions (the number of grid points is $N+1$), $L$ is the map length (m), $x_{min}$ … $y_{max}$ the transverse range (m), and \`norm\` a normalization factor. Then follow $(N_z+1)(N_x+1)(N_y+1)$ values, one per line or several separated by white space.

The header of the example \`hwr010.edz\` is

\`\`\`text
105 2.100000000000000E-001
20 -2.000000000000000E-002 2.000000000000000E-002
20 -2.000000000000000E-002 2.000000000000000E-002
1
\`\`\`

i.e. $106\\times 21\\times 21 = 46\\,746$ points with a grid spacing of 2 mm in all three directions; the header of \`sol\` is \`200 0.35\`, \`24 ±0.02\`, \`24 ±0.02\`, \`1\`: $201\\times 25\\times 25 = 125\\,625$ points, 1.75 mm longitudinally and 1.67 mm transversely.

### Binary format

A file that does not start with plain numeric text is read as little-endian binary: \`int32 Nz\`, \`float64 L\`, \`int32 Nx\`, \`float64 xmin\`, \`float64 xmax\`, \`int32 Ny\`, \`float64 ymin\`, \`float64 ymax\`, \`float64 norm\` (60 bytes, no padding), followed by $(N_z+1)(N_x+1)(N_y+1)$ \`float32\` values. Each value takes only 4 bytes, much less than in text (about 11 bytes per value in the example), and reading is faster.

### The normalization factor

The AVAS GUI and the linear preview multiply the values read by \`norm\`. All example maps have \`norm\` = 1; how the kernel uses this line is not documented in the manual. **Export maps with \`norm\` = 1** and set the field strength with Ke and Kb only.

### Extensions

| Extension | Component | Field type | Units |
|---|---|---|---|
| \`.edx\` \`.edy\` \`.edz\` | RF electric field $E_x$, $E_y$, $E_z$ | 1 | MV/m, times Ke, varies as cos |
| \`.bdx\` \`.bdy\` \`.bdz\` | RF magnetic field $B_x$, $B_y$, $B_z$ | 1 | T, times Kb, varies as sin |
| \`.esx\` \`.esy\` \`.esz\` | static electric field $E_x$, $E_y$, $E_z$ | 2 | MV/m, times Ke |
| \`.bsx\` \`.bsy\` \`.bsz\` | static magnetic field $B_x$, $B_y$, $B_z$ | 3 | T, times Kb |

Mnemonic: \`e\`/\`b\` electric/magnetic, \`d\`/\`s\` dynamic (RF)/static, the last letter the component. When the GUI checks whether a map is complete, an RF cavity needs at least one complete triplet (electric or magnetic), static electric and static magnetic maps their three components.

> **AVAS convention**: the E and B of an RF field come from the same eigenmode, and the phase relation between them in the export (and the sign that follows from it) is set by the solver; AVAS always uses cos for E and sin for B, and a sign difference is compensated by the sign of Kb. The example uses $K_b = -K_e$, see [RF cavities](#rf-cavity-in-avas).

## Grid, extent and aperture {#field-maps-grid}

### Grid spacing

The grid spacing sets the shortest structure a map can resolve. As a rule of thumb:

- The longitudinal spacing should be much smaller than the length over which the field changes (gap length, fringe length), and not larger than the order of the distance a particle travels per integration step. In the example, with \`steppercycle 50\` and $\\beta = 0.057$ a step is about $\\beta\\lambda/50 \\approx 2.1\\ \\mathrm{mm}$, comparable to the 2 mm grid of hwr010.
- The transverse spacing sets the accuracy of the near-axis gradients (the focusing forces). The 1.7–2 mm of the example give only about 10 intervals within $\\pm 2\\ \\mathrm{cm}$: enough for linear gradients, limited for higher-order components.

### Transverse extent and aperture

The example maps cover the square $\\lvert x\\rvert, \\lvert y\\rvert \\le 2\\ \\mathrm{cm}$ and the element aperture \`R\` is also 0.02 m: every point inside the round aperture is inside the map, and the corners of the square (up to $r \\approx 2.83\\ \\mathrm{cm}$) also have data. A particle beyond the aperture radius counts as lost; **the transverse extent of the map should cover the whole aperture**, otherwise there are regions inside the aperture without field data, and what the kernel does outside the map is not documented.

### Map length and element length

The \`L\` of \`field\` should equal the $L$ in the header. What the kernel does when they differ is not documented; the linear preview uses the field in $[0, \\min(L, L_{map})]$ only. To let a field extend into a neighbouring element (e.g. a long fringe), use [Superpose](#superpose) instead of truncating the map.

### The ends of a map

The field should have decayed to negligible values at both ends: the on-axis $E_z$ of \`hwr010\` is about 0.5–0.7 % of the peak at the ends, the $B_z$ of \`sol\` about 0.3 %. Cutting a map where the field is still large is like switching the field off abruptly at the element boundary, and the particles get a spurious edge kick (especially for solenoids, since $B_r \\propto -\\tfrac{r}{2}B_z'$).

## Storage order of the component files {#field-maps-order}

The header gives the number of points in each direction but not the order of the values. The manual is silent about it; AVAS recognizes two orders:

- **z outermost**: the whole cross-section at $z = z_0$, then the whole cross-section at $z = z_1$, …, with $x$ varying fastest inside a cross-section.
- **z innermost**: for every transverse point, the whole line along $z$.

Both readings give arrays of the same shape, but the values land in completely different places. AVAS decides by the "roughness along $z$": for each reading it computes the ratio of the summed absolute second differences to the summed absolute first differences along $z$. The real field is smooth along $z$; a wrong reading turns transverse variations into longitudinal ones and looks rough. z innermost is used only when it is clearly smoother (less than 0.9 times the z-outermost roughness); otherwise z outermost.

**The decision is made for a whole group of component files** (\`group_order()\` in \`avas/data/fieldmap.py\`), not file by file. A single component can look alike in both readings, for example one transverse component of a quadrupole map; all component files of one map necessarily share the order, so a vote over the group is more reliable. For the examples:

| Map | sum of roughness, z outermost | sum of roughness, z innermost | decision |
|---|---|---|---|
| hwr010 (6 components) | 9.10 | 9.64 | z outermost |
| sol (3 components) | 5.01 | 5.07 | z outermost |

The noise of the example maps makes the distinction weak (only about 1 % for sol, and \`sol.bsx\` alone is even slightly smoother z innermost), so physical criteria are needed as a cross-check, see the [checklist](#field-maps-checks). All 40 kernel maps of two projects checked so far are stored z outermost; **write your own maps z outermost**.

## Interpolation, Maxwell's equations and noise {#field-maps-interpolation}

### Interpolation

How the kernel interpolates between grid points is not documented in the manual; the kernel is the reference. In AVAS's front and back end:

- The linear preview interpolates the on-axis values linearly along $z$ and takes the transverse derivatives on the axis by central differences (by the difference of the two neighbouring points when the axis is not a grid point).
- The GUI slices take the nearest grid plane without interpolation; for speed at most 400×240 grid lines are kept for display (the first and last always).

### Relations a map should satisfy

A map comes from a numerical solution and should satisfy Maxwell's equations to the accuracy of the discretization. Near the axis:

$$
E_r \\approx -\\frac{r}{2}\\frac{\\partial E_z}{\\partial z},\\qquad
B_r \\approx -\\frac{r}{2}\\frac{\\partial B_z}{\\partial z}\\quad(\\text{axisymmetric, paraxial})
$$

In an RF cavity E and B are also linked by Faraday's law: with AVAS's convention $\\boldsymbol{E}(\\boldsymbol{r})\\cos(\\omega t+\\varphi)$, $\\boldsymbol{B}(\\boldsymbol{r})\\sin(\\omega t+\\varphi)$ one has $\\nabla\\times\\boldsymbol{E}(\\boldsymbol{r}) = -\\omega\\,\\boldsymbol{B}(\\boldsymbol{r})$. When the transverse component files are missing, the linear preview derives the transverse gradients from the on-axis component with the first relations.

### Noise in the example maps

The example maps \`hwr010\` and \`sol\` are noticeably noisy themselves (checked 2026-09-21): the mean second difference of the on-axis $E_z$ and $B_z$ along $z$ is about 7 % of the range, and \`sol.bsz\` jumps from point to point in its central region. Slice heat maps show this as stripes along $z$ — **that is the data, not an error of reading or slicing**. Effects on calculations:

- linear quantities ($\\int E_z\\,\\dd z$, $\\int B_z\\,\\dd z$) are hardly affected;
- squared quantities ($\\int B_z^2\\,\\dd z$, which sets the solenoid focusing) are raised systematically, by about 3 % in the example;
- derivatives (near-axis gradients, curls computed by differences) amplify the noise most; do not try to "verify" Maxwell's equations on these maps by numerical differentiation;
- the maximum of the raw data is not the peak field: the raw maximum of \`sol.bsz\` is about 20 % above the peak after 9-point smoothing.

> **Note**: for production calculations export maps with a fine enough grid and a converged solver accuracy; do not smooth maps outside AVAS on your own — smoothing breaks the Maxwell relations and changes the fringe fields.

## Field maps in the GUI {#field-maps-gui}

### Files page

Selecting any component file on the Files page shows the file information (meaning of the component, text/binary, length, grid, transverse range, normalization, the other components with the same name, the lattice elements that use it) and two views:

- **Along z**: the on-axis value ($x = y = 0$) and the largest $\\lvert F\\rvert$ of each cross-section versus $z$.
- **Slices**: a heat map, either a longitudinal cut $z$–$x$ (fixed $y$), $z$–$y$ (fixed $x$) or a cross-section $x$–$y$ (fixed $z$), with a movable position. Components of the same map with the same grid are cut together, so the magnitude and in-plane arrows can be shown.

### Field window of an element

When an element is selected on the Lattice page or in the visual editor, its frame has a "View field map / View field distribution" button in the top right corner: a field-map element opens **data** (slices of the map), a matrix-model element a **schematic** computed from its parameters (hard edges, zero outside the element, labelled as schematic). Both use the same views; while the window is open, selecting another element updates it and dragging a parameter redraws it live.

### Conventions for reading the pictures

- **Colours**: signed components use a colour scale centred on 0, symmetric for both signs; magnitudes a one-sided scale.
- **Viewing direction**: all cross-sections are drawn **looking from downstream to upstream, with the beam coming out of the screen**; the ⊙ mark at the centre shows this direction. Field arrows and force arrows in the same picture can then be checked directly with $\\boldsymbol{F} = q\\boldsymbol{v}\\times\\boldsymbol{B}$: $\\boldsymbol{v}$ points at the reader.
- **Arrows**: arrow lengths are proportional to the field components in data coordinates, so with different axis scales the arrows are stretched with the picture; only with "Equal scales" ticked do they show the true direction. A long element then becomes a thin strip in a longitudinal cut.
- **Stripes**: stripes along $z$ can come from a wrong storage order or from noise in the data itself (as in the examples); the [checklist](#field-maps-checks) tells them apart.

\`\`\`avas-field
map hwr010
\`\`\`

## Checklist {#field-maps-checks}

Before using a new field map for calculations, check item by item:

1. **Header and amount of data**: the number of values equals $(N_z+1)(N_x+1)(N_y+1)$; the header length equals the \`L\` in the lattice; all components of a map have the same grid.
2. **Complete components**: an RF cavity has at least one complete electric or magnetic triplet; static maps have all three components. The Lattice page warns "Field map … not found (… needed)".
3. **Longitudinal position**: the on-axis $B_z$ of a solenoid should peak at the element centre (example \`sol\`: peak at $z \\approx 0.177\\ \\mathrm{m}$, centre 0.175 m); the $E_z$ of a two-gap cavity should be antisymmetric about the centre (example \`hwr010\`: zero crossing at $z \\approx 0.104\\ \\mathrm{m}$, centre 0.105 m). If the peak sits near one end or the profile looks like a periodic saw-tooth, suspect the storage order first.
4. **Transverse direction**: in the cross-sections of a solenoid $(B_x, B_y)$ should point to the axis at the entrance and away from it at the exit, with almost no transverse field in the central cross-section (checked on the example, deviations at the noise level); the magnitude should agree with $\\tfrac{r}{2}\\lvert B_z'\\rvert$.
5. **Symmetry**: an HWR is up–down symmetric about the beam axis; in a quadrupole map $B_y(x)$ and $B_x(y)$ have the same sign and slope; remember the "downstream looking upstream" view.
6. **Decay at the ends**: at both ends the field should be below about one per cent of the peak.
7. **Physical quantities**: for an RF cavity, $\\int\\lvert E_z\\rvert\\,\\dd z$ and the β at which the transit-time factor $T(\\beta)$ peaks should match the design geometric beta (about 0.10 in the example); for a solenoid $\\int B_z\\,\\dd z$ should match the magnet design.
8. **Trial run**: check energy gain and focal length with the linear preview or a short multi-particle run before production runs.

## Field-map directory {#field-maps-directory}

The kernel looks for field maps by name in the **field-map directory**:

- by default the project's \`InputFile/\`;
- it can be set on the Settings page under "Field maps → Directory", stored as \`[project] fieldSource\` in ini.ini (empty means InputFile);
- on the command line \`avas run --input DIR --output DIR --field DIR\` sets it with \`--field\`; without it the ini.ini setting is used, and without that the input directory.

When running, AVAS copies the text inputs to \`<output>/inputs/\` and the kernel uses the copies, but the field maps are still read from the original field-map directory, so the large files are not copied. If a map is missing, the Lattice page warns and the linear preview treats the element as a drift with a warning.

> **Note**: field-map files are large (a single component of the example is about 0.5–1.4 MB of text). When several projects share maps, put them in one common directory and point fieldSource to it rather than copying them into every project.

## Hands-on {#field-maps-lab}

1. In the hwr010 viewer above choose the $z$–$x$ cut of \`edz\` and find the two gaps of opposite sign; then choose the $x$–$y$ cross-section, move it to the two gap centres and compare how \`edx\` and \`edy\` change direction.
2. In the sol viewer below choose the longitudinal cut of \`bsz\` and look at the stripes along $z$ (noise); switch to cross-sections near the entrance and the exit with arrows on, and carry out item 4 of the [checklist](#field-maps-checks).
3. Untick and tick "Equal scales" and compare the arrow directions, to see why only equal scales show the true direction.

\`\`\`avas-field
map sol
\`\`\`

## Further reading {#field-maps-reading}

- The description of 3D field maps (FIELD_MAP) in the TraceWin user manual: AVAS's text format is similar, but units and parameter meanings are those of this chapter and of [RF cavities](#rf-cavity).
- T. P. Wangler, *RF Linear Accelerators*, chapter 2: transit-time factor and paraxial fields from the on-axis field.
- M. Reiser, *Theory and Design of Charged Particle Beams*, chapter 3: paraxial expansion of axisymmetric fields.
- In this primer: [Static field maps](#static-fields), [Superpose](#superpose), [Simulation settings](#simulation), [References](#references).
`,d=`# 场图

场图（field map）是在三维网格上给出的电场或磁场分量表，通常由电磁场求解程序（如 CST、COMSOL、Opera、Superfish）计算后导出。AVAS 的 \`field\` 元件读取场图，用 t-code 在网格插值得到的场中积分粒子运动：射频腔（类型 1）、静电元件（类型 2）、静磁元件（类型 3）都靠它描述，见 [射频腔](#rf-cavity) 与 [静电与静磁场图](#static-fields)。本章说明场图文件的格式与单位、网格与孔径、分量文件的存储顺序、插值与噪声，以及在界面中如何查看和检查一张场图。

示例项目 \`examples/hwr010/InputFile\` 带有两张场图：半波长腔 \`hwr010\`（\`.edx/.edy/.edz/.bdx/.bdy/.bdz\`，长 0.21 m）和螺线管 \`sol\`（\`.bsx/.bsy/.bsz\`，长 0.35 m）。\`efield.*\` 与 \`bfield.*\` 分别是这两组文件逐字节相同的副本。

## 场图描述什么 {#field-maps-role}

- **一张场图 = 一组同名分量文件**。\`field\` 的最后一个参数是不含扩展名的文件名，内核按场类型读取对应的扩展名。
- **场的形状由文件给出，幅度由 Ke、Kb 给出**。文件中电场单位为 MV/m，磁场单位为 T；实际作用的场是 $K_e\\boldsymbol{E}_{map}$、$K_b\\boldsymbol{B}_{map}$。射频场还要乘以时间因子：电场 $\\cos(\\omega t+\\varphi_0)$，磁场 $\\sin(\\omega t+\\varphi_0)$。
- **坐标**：$z$ 从元件入口（$z = 0$）到场图长度 $L$；$x$、$y$ 是相对于束流轴的横向坐标，单位都是 m。
- **数据与示意的区别**：场图是数据，包含求解器给出的全部细节（边缘场、高阶成分、网格噪声）；矩阵模型元件（\`quad\`、\`solenoid\`、\`bend\`、\`steerer\`）在界面里画出的场是按参数算出的硬边示意，二者不能混为一谈，见 [界面中的场图](#field-maps-gui)。

## 文件格式与单位 {#field-maps-format}

### 文本格式

\`\`\`text
Nz  L
Nx  xmin  xmax
Ny  ymin  ymax
norm
v1
v2
...
\`\`\`

前四行是文件头：$N_z$、$N_x$、$N_y$ 是三个方向的**网格间隔数**（网格点数为 $N+1$），$L$ 是场图长度（m），$x_{min}$…$y_{max}$ 是横向范围（m），\`norm\` 是归一化系数。之后是 $(N_z+1)(N_x+1)(N_y+1)$ 个数值，可以每行一个，也可以多个用空白分开。

示例 \`hwr010.edz\` 的文件头为

\`\`\`text
105 2.100000000000000E-001
20 -2.000000000000000E-002 2.000000000000000E-002
20 -2.000000000000000E-002 2.000000000000000E-002
1
\`\`\`

即 $106\\times 21\\times 21 = 46\\,746$ 个点，网格间距在三个方向都是 2 mm；\`sol\` 的文件头为 \`200 0.35\`、\`24 ±0.02\`、\`24 ±0.02\`、\`1\`，共 $201\\times 25\\times 25 = 125\\,625$ 个点，纵向间距 1.75 mm、横向 1.67 mm。

### 二进制格式

文件开头不是纯数字文本时按小端二进制读取：依次为 \`int32 Nz\`、\`float64 L\`、\`int32 Nx\`、\`float64 xmin\`、\`float64 xmax\`、\`int32 Ny\`、\`float64 ymin\`、\`float64 ymax\`、\`float64 norm\`（共 60 字节，无填充），之后是 $(N_z+1)(N_x+1)(N_y+1)$ 个 \`float32\` 数值。二进制文件每个数值只占 4 字节，比文本（示例约 11 字节一个数）小得多，读取也更快。

### 归一化系数

AVAS 界面和线性预览把读入的数值乘以 \`norm\`。示例场图的 \`norm\` 都是 1；内核如何使用这一行，手册没有说明。**建议导出时把 \`norm\` 写成 1**，场强一律用 Ke、Kb 调节。

### 扩展名

| 扩展名 | 分量 | 场类型 | 单位 |
|---|---|---|---|
| \`.edx\` \`.edy\` \`.edz\` | 射频电场 $E_x$、$E_y$、$E_z$ | 1 | MV/m，乘 Ke，随 cos 变化 |
| \`.bdx\` \`.bdy\` \`.bdz\` | 射频磁场 $B_x$、$B_y$、$B_z$ | 1 | T，乘 Kb，随 sin 变化 |
| \`.esx\` \`.esy\` \`.esz\` | 静电场 $E_x$、$E_y$、$E_z$ | 2 | MV/m，乘 Ke |
| \`.bsx\` \`.bsy\` \`.bsz\` | 静磁场 $B_x$、$B_y$、$B_z$ | 3 | T，乘 Kb |

记法：\`e\`/\`b\` 表示电场/磁场，\`d\`/\`s\` 表示动态（射频）/静态，最后一个字母是分量。界面检查场图是否齐全时，射频腔要求电场或磁场三个分量中至少有一组完整，静电和静磁场图要求各自的三个分量都在。

> **AVAS 约定**：射频场的 E 与 B 来自同一个本征模，导出时两者之间的相位关系（以及由此决定的符号）由求解程序决定；AVAS 统一按 E 用 cos、B 用 sin 处理，符号差异通过 Kb 的正负补偿。示例取 $K_b = -K_e$，见 [射频腔](#rf-cavity-in-avas)。

## 网格、范围与孔径 {#field-maps-grid}

### 网格间距

网格间距决定了场图能分辨的最短结构。经验上：

- 纵向间距应远小于场的变化长度（间隙长度、边缘场长度），也应不大于粒子每个积分步走过的距离的量级。示例中 \`steppercycle 50\`、$\\beta = 0.057$ 时每步约 $\\beta\\lambda/50 \\approx 2.1\\ \\mathrm{mm}$，与 hwr010 的 2 mm 网格相当。
- 横向间距决定了近轴梯度（聚焦力）的精度。示例的 1.7–2 mm 横向间距在 $\\pm 2\\ \\mathrm{cm}$ 范围内只有约 10 个间隔，对线性梯度够用，对高阶成分分辨有限。

### 横向范围与孔径

示例场图横向范围是 $\\lvert x\\rvert, \\lvert y\\rvert \\le 2\\ \\mathrm{cm}$ 的正方形，元件孔径 \`R\` 也是 0.02 m：圆孔内的点都在场图内，正方形的四角（$r$ 最大约 2.83 cm）也有数据。粒子到达孔径半径之外即判为损失；**场图横向范围应覆盖整个孔径**，否则孔径内会出现没有场数据的区域，内核在场图范围外如何取值，手册没有说明。

### 场图长度与元件长度

\`field\` 的 \`L\` 应等于文件头中的 $L$。两者不一致时内核的处理方式未见文档；线性预览只使用 $[0, \\min(L, L_{map})]$ 区间内的场。若需要让场延伸到相邻元件里（例如边缘场很长），应使用 [叠加场](#superpose)，而不是截短场图。

### 场图两端

场图两端的场应该已经衰减到可以忽略：示例 \`hwr010\` 两端轴上 $E_z$ 约为峰值的 0.5–0.7 %，\`sol\` 两端 $B_z$ 约为峰值的 0.3 %。若在场还很大的地方截断，等于在元件边界处突然关掉场，粒子会受到虚假的边缘冲击（螺线管尤其明显，因为 $B_r \\propto -\\tfrac{r}{2}B_z'$）。

## 分量文件的存储顺序 {#field-maps-order}

文件头只给出三个方向的点数，没有写数值按什么顺序排列。手册对此没有说明，AVAS 识别两种顺序：

- **z 在最外层**：依次写出 $z = z_0$ 的整个横截面、$z = z_1$ 的整个横截面……横截面内 $x$ 变化最快。
- **z 在最内层**：对每个横向点写出整条沿 $z$ 的数据。

两种读法得到的数组形状相同，但数值落在完全不同的位置。AVAS 用"沿 $z$ 的粗糙度"判断：对每种读法计算二阶差分与一阶差分绝对值之和的比值，真实场沿 $z$ 应该平滑，错误的读法会把横向变化错当成纵向变化，显得粗糙。只有当 z 最内层的粗糙度明显更小（小于 z 最外层的 0.9 倍）时才采用 z 最内层，否则按 z 最外层。

**判断按一组分量文件整体进行**（\`avas/data/fieldmap.py\` 的 \`group_order()\`），而不是逐个文件。原因是单个分量可能两种读法看起来差不多，例如四极场图的一个横向分量；同一张场图的所有分量文件必然用同一种顺序，整组投票更可靠。示例的结果：

| 场图 | z 最外层粗糙度之和 | z 最内层粗糙度之和 | 判定 |
|---|---|---|---|
| hwr010（6 个分量） | 9.10 | 9.64 | z 最外层 |
| sol（3 个分量） | 5.01 | 5.07 | z 最外层 |

可以看出示例场图本身的噪声让区分度很弱（sol 两种读法只差约 1 %，单看 \`sol.bsx\` 甚至是 z 最内层略平滑）。因此还要用物理判据复核，见 [检查清单](#field-maps-checks)。到目前为止核对过的两个项目共 40 张内核场图都按 z 最外层存储，**自己导出场图时请按 z 最外层写**。

## 插值、Maxwell 方程与噪声 {#field-maps-interpolation}

### 插值

内核在网格点之间如何插值，手册没有说明，以内核为准。AVAS 前后端的做法是：

- 线性预览沿 $z$ 对轴上值线性插值，横向导数在轴上用中心差分（轴不在网格点上时取相邻两点的差分）。
- 界面切片取最近的网格平面，不做插值；显示时为了速度最多保留 400×240 条网格线（首尾两条总保留）。

### 场图应满足的关系

场图来自数值求解，应当在离散精度内满足 Maxwell 方程。在轴附近：

$$
E_r \\approx -\\frac{r}{2}\\frac{\\partial E_z}{\\partial z},\\qquad
B_r \\approx -\\frac{r}{2}\\frac{\\partial B_z}{\\partial z}\\quad(\\text{轴对称、近轴})
$$

射频腔中 E 与 B 还由 Faraday 定律联系：在 AVAS 的约定 $\\boldsymbol{E}(\\boldsymbol{r})\\cos(\\omega t+\\varphi)$、$\\boldsymbol{B}(\\boldsymbol{r})\\sin(\\omega t+\\varphi)$ 下，$\\nabla\\times\\boldsymbol{E}(\\boldsymbol{r}) = -\\omega\\,\\boldsymbol{B}(\\boldsymbol{r})$。线性预览在缺少横向分量文件时就是用第一组关系由轴上分量推出横向梯度的。

### 示例场图的噪声

示例的 \`hwr010\` 与 \`sol\` 场图本身有明显噪声（2026-09-21 核实）：轴上 $E_z$ 与 $B_z$ 沿 $z$ 的二阶差分平均值约为量程的 7 %，\`sol.bsz\` 在中心区逐点跳动。切片热图会把它显示成沿 $z$ 的竖条纹——**那是数据本身，不是读取或切片的错误**。对计算的影响：

- 线性量（$\\int E_z\\,\\dd z$、$\\int B_z\\,\\dd z$）几乎不受影响；
- 平方量（$\\int B_z^2\\,\\dd z$，决定螺线管聚焦）会被系统性地抬高，示例约 3 %；
- 导数量（近轴梯度、由差分求的旋度）被噪声放大最严重，不要从这些场图里用数值微分去"验证"Maxwell 方程；
- 原始数据的最大值不是峰值场：\`sol.bsz\` 的原始最大值比 9 点平滑后的峰值高约 20 %。

> **注意**：用于正式计算的场图，最好在导出时就用足够细的网格和收敛的求解精度；不要在 AVAS 之外随意平滑场图——平滑会破坏 Maxwell 关系，改变边缘场。

## 在界面中查看场图 {#field-maps-gui}

### 文件页

在文件页选中任一分量文件，会显示文件信息（分量含义、文本/二进制、长度、网格、横向范围、归一化系数、同名的其他分量、被 lattice 中哪些元件使用），以及两个视图：

- **沿 z**：轴上（$x = y = 0$）的值和每个横截面上 $\\lvert F\\rvert$ 的最大值随 $z$ 的变化。
- **切片**：热图，可选纵切面 $z$–$x$（固定 $y$）、$z$–$y$（固定 $x$）和横截面 $x$–$y$（固定 $z$），并可移动切面位置。同一场图中网格相同的分量一起切，可以显示模和面内箭头。

### 元件的场分布窗口

在结构页或可视化编辑器中选中元件，元件图框右上角有"查看场图／查看场分布"按钮：场图元件打开的是**数据**（场图切片），矩阵模型元件打开的是按参数计算的**示意**（硬边，元件外为零，界面标注为示意）。两者用同一套视图，窗口开着时切换元件会更新内容，拖动参数会实时重画。

### 读图约定

- **颜色**：有符号分量的色标以 0 为中点，正负对称；模用单侧色标。
- **观察方向**：所有横截面都**从下游往上游看，束流垂直屏幕向外**，圆心的 ⊙ 记号标出这一方向。这样同一张图里的场箭头与受力箭头可以直接用 $\\boldsymbol{F} = q\\boldsymbol{v}\\times\\boldsymbol{B}$ 核对：$\\boldsymbol{v}$ 指向读者。
- **箭头**：箭头长度在数据坐标下正比于场分量，因此两个坐标轴比例不同时箭头随图拉伸，只有勾选"等比例"时才是真实方向。纵切面上元件很长，等比例时会变成细条。
- **条纹**：沿 $z$ 的条纹可能来自存储顺序判错，也可能是数据本身的噪声（示例即属后者），用 [检查清单](#field-maps-checks) 区分。

\`\`\`avas-field
map hwr010
\`\`\`

## 检查清单 {#field-maps-checks}

拿到一张新场图，在用于计算之前建议逐项检查：

1. **文件头与数据量**：数值个数等于 $(N_z+1)(N_x+1)(N_y+1)$；文件头长度等于 lattice 中的 \`L\`；同一张场图的各分量网格相同。
2. **分量齐全**：射频腔至少有一组完整的电场或磁场三分量；静态场三分量齐全。结构页会提示"找不到场图……（需要 ……）"。
3. **纵向位置**：螺线管轴上 $B_z$ 的峰值应在元件中心（示例 \`sol\`：峰在 $z \\approx 0.177\\ \\mathrm{m}$，中心 0.175 m）；两间隙腔的 $E_z$ 应关于中心反对称（示例 \`hwr010\`：过零点 $z \\approx 0.104\\ \\mathrm{m}$，中心 0.105 m）。若峰值偏到一端或呈周期性锯齿，先怀疑存储顺序。
4. **横向方向**：螺线管入口端横截面上 $(B_x, B_y)$ 应指向轴心，出口端背离轴心，中心截面横向场接近零（示例已核实，偏差在噪声量级）；量级应与 $\\tfrac{r}{2}\\lvert B_z'\\rvert$ 相符。
5. **对称性**：HWR 关于束流轴上下对称；四极场的 $B_y(x)$ 与 $B_x(y)$ 同号同斜率；观察方向按"从下游往上游看"。
6. **两端衰减**：两端的场应已降到峰值的百分之一以下。
7. **物理量核对**：射频腔的 $\\int\\lvert E_z\\rvert\\,\\dd z$、渡越时间因子 $T(\\beta)$ 的最大位置应与设计的几何 β 一致（示例约 0.10）；螺线管的 $\\int B_z\\,\\dd z$ 应与磁铁设计相符。
8. **试算**：用线性预览或一次短的多粒子运行看能量增益、焦距是否合理，再进行正式计算。

## 场图目录 {#field-maps-directory}

内核在**场图目录**中按文件名查找场图：

- 默认是项目的 \`InputFile/\`；
- 可以在设置页"场图 → 目录"中指定，保存在 ini.ini 的 \`[project] fieldSource\`（空表示 InputFile）；
- 命令行 \`avas run --input DIR --output DIR --field DIR\` 用 \`--field\` 指定，缺省时读 ini.ini 的设置，再缺省为输入目录。

运行时 AVAS 把文本输入复制到 \`<输出>/inputs/\`，内核以副本为输入，但场图仍从原来的场图目录读取，所以大文件不会被复制。找不到场图时，结构页给出警告，线性预览把该元件当作漂移段并提示。

> **注意**：场图文件很大（示例单个分量约 0.5–1.4 MB 的文本），多个项目共用场图时，把它们放在一个公共目录并用 fieldSource 指向它，比在每个项目中复制一份更好。

## 动手实验 {#field-maps-lab}

1. 在上面的 hwr010 查看器中选 \`edz\` 的 $z$–$x$ 纵切面，找到两个反号的间隙；再选横截面 $x$–$y$，把位置移到两个间隙中心，比较 \`edx\`、\`edy\` 的方向变化。
2. 在下面的 sol 查看器中选 \`bsz\` 的纵切面，观察沿 $z$ 的竖条纹（噪声）；切到入口与出口附近的横截面，打开箭头，完成 [检查清单](#field-maps-checks) 第 4 项。
3. 取消"等比例"再勾选，比较箭头方向的变化，理解为什么只有等比例时箭头才表示真实方向。

\`\`\`avas-field
map sol
\`\`\`

## 延伸阅读 {#field-maps-reading}

- TraceWin 用户手册中关于三维场图（FIELD_MAP）格式的说明：AVAS 的文本格式与之相似，但单位与参数含义以本章和 [射频腔](#rf-cavity) 为准。
- T. P. Wangler, *RF Linear Accelerators*，第 2 章：由轴上场计算渡越时间因子和近轴场。
- M. Reiser, *Theory and Design of Charged Particle Beams*，第 3 章：轴对称场的近轴展开。
- 本手册：[静电与静磁场图](#static-fields)、[叠加场](#superpose)、[模拟设置](#simulation)、[参考文献](#references)。
`,f='# Accelerators and AVAS\n\nThis primer is written for accelerator physicists and graduate students who use AVAS in their research. It starts from the basic ideas of a linear accelerator, goes through relativistic kinematics, phase space and linear optics, introduces every element AVAS can simulate, and finally shows how to assemble them into a beamline and carry out matching, error studies and multi-particle simulations. This chapter gives the overall picture: what a linac is made of, what AVAS simulates, which files a simulation goes through from input to output, and how to read this primer.\n\n## What a linac does {#intro-linac}\n\nA linear accelerator (linac) guides charged particles along an approximately straight reference trajectory through a sequence of accelerating gaps and focusing elements, so that the particles gain energy while the beam stays confined to a finite transverse size and a finite longitudinal (phase) extent. Unlike in a ring, each particle passes every element only once, so the beam quality is set by **single-pass** dynamics: the phase-space distribution at the entrance, the fields of the elements and the beam\'s own space-charge forces together determine the energy, emittance and losses at the exit.\n\nTwo kinds of quantities describe a beam:\n\n- the motion of the **reference particle** (synchronous particle): it moves on the design orbit, and its kinetic energy $W(s)$ and arrival time $t(s)$ fix the phases of the RF cavities and the "design state" of every element;\n- the **deviations of all other particles from the reference particle**: transverse positions and angles $(x, x\', y, y\')$, longitudinal position and momentum deviation $(z, \\delta)$. Beam size, emittance and energy spread are statistics of these deviations, see [Phase space and emittance](#phase-space).\n\nIn low-energy, high-intensity linacs (for example the low-energy sections of superconducting proton and heavy-ion linacs), space charge, RF defocusing and longitudinal non-linearities often act at the same time — which is why multi-particle simulation codes exist.\n\n## Building blocks of a beamline {#intro-components}\n\nA linac beamline can be divided into four functional classes of elements. The table lists how each is written in AVAS:\n\n| Function | Typical elements | Written in AVAS as | See |\n|---|---|---|---|\n| Transverse focusing | quadrupoles, solenoids | `quad`, `solenoid`, or a static magnetic field map `field … 3 …` | [Quadrupole](#quadrupole), [Solenoid](#solenoid) |\n| Acceleration and longitudinal focusing | RF cavities (e.g. half-wave resonators, HWR) | RF field map `field … 1 …` | [RF cavity](#rf-cavity) |\n| Bending and orbit correction | dipoles, steerers | `bend`, `edge`, `steerer` | [Dipole](#dipole), [Steerer](#steerer) |\n| Diagnostics and output | diagnostic locations, output planes | `diag_energy`, `diag_size`, `diag_position`, `outputplane` | [Errors and correction](#errors), [Simulation settings](#simulation) |\n\nIn addition there are drifts `drift` (field-free space, see [Drift](#drift)), static electric field maps `field … 2 …` (see [Static fields](#static-fields)), and `superpose` blocks that let several field maps overlap in space (see [Superpose](#superpose)).\n\n> Note: this primer only covers what AVAS can simulate. Dedicated models of RFQ or DTL structures, ion sources and the like are not AVAS elements and are not presented as AVAS features here. To start from the beam at an RFQ exit, read a particle distribution file (`.dst`) produced by another code as the initial beam (`readparticledistribution`).\n\n## What AVAS simulates {#intro-what-avas-simulates}\n\nAVAS (Advanced Virtual Accelerator Software) is a multi-particle beam-dynamics code for linear accelerators. The computing kernel is a binary C++ library (`AVAS.dll` on Windows); the user interface and the pre- and post-processing are done in Python and a web front end. What the kernel does can be summarised as follows:\n\n1. **Reference particle**: starting from the kinetic energy given in `beam.txt`, it integrates the energy and time of the reference particle along the lattice and determines the entry phase of every RF cavity (see [Longitudinal motion](#longitudinal)).\n2. **Macro-particles**: it generates `particlenumber` macro-particles from the Twiss parameters and distribution type in `beam.txt` (or reads a particle distribution file). Each macro-particle stands for many real particles; the total charge follows from the current `current` and the beam frequency `frequency` (bunch charge $Q = I/f$).\n3. **External fields**: field-map elements and drifts are tracked with the t-code, integrating the Lorentz force in time steps; matrix elements use the transfer matrices of the z-code (see [the next section](#intro-tracking)).\n4. **Space charge**: with `spacecharge 1` in `input.txt`, the beam\'s self-field is computed with the algorithm selected by `scmethod` (FFT, PICNIC, SPICNIC) and applied to every macro-particle (see [Space charge](#space-charge)).\n5. **Errors and correction**: commands such as `err_step`, `err_beam_dyn`, `err_beam_stat`, `err_quad_*`, `err_cav_*` in the lattice add random errors to elements and to the initial beam; `adjust` and `diag_*` perform static error correction (see [Errors and correction](#errors)).\n6. **Losses**: particles leaving the element aperture `R` or the longitudinal limits are counted as lost; the number of surviving macro-particles along $s$ is written to the output.\n\n## t-code and z-code {#intro-tracking}\n\nAVAS elements come in two models that are computed differently.\n\n### Field-model elements (t-code)\n\nFor `field` elements (and drifts `drift`) the user supplies the three-dimensional field distribution (field-map files). The kernel takes **time $t$ as the independent variable** and integrates the equation of motion of every macro-particle step by step,\n\n$$\n\\frac{\\dd \\boldsymbol p}{\\dd t} = q\\left(\\boldsymbol E + \\boldsymbol v \\times \\boldsymbol B\\right) + \\boldsymbol F_{\\mathrm{sc}}\n$$\n\nwhere $\\boldsymbol F_{\\mathrm{sc}}$ is the space-charge force. This is called the t-code. The time step is set by `steppercycle` in `input.txt` (the number of steps per RF period). The t-code naturally includes field non-linearities, fringe fields and the change of particle velocity inside a cavity, which is essential for low-energy RF cavities.\n\n### Matrix-model elements (z-code)\n\n`quad`, `solenoid`, `bend`, `edge` and `steerer` need only a few parameters (length, gradient, field …). The kernel takes the **longitudinal position $z$ as the independent variable** and transforms the particle coordinates with hard-edge transfer matrices; this is called the z-code. It is fast and can be compared directly with analytic theory (see [Linear optics](#linear-optics)), but contains no extended fringe-field distribution (`edge` only adds a thin-lens pole-face correction); whether the kernel\'s matrix elements include higher-order terms is not documented in the manual, so the kernel is the reference.\n\nBoth models can be mixed in one lattice; at each boundary between them the particle distribution has to be converted between the "same time" and the "same longitudinal position" descriptions (how the kernel does this is not documented in the manual).\n\n> AVAS convention: in multi-particle simulations **do not use a matrix element as the first or last element** of the lattice (a requirement of the manual); if necessary, add a very short `drift` at either end.\n\nThe AVAS interface also provides a **linear envelope preview** ("Linear preview" in the visual editor). It does not call the kernel; it pushes the second moments of the beam with 6×6 matrices and approximates field maps by slices. Its principle is described in [Linear optics](#linear-optics-6d); its accuracy and range of validity are given in [the last section of this chapter](#intro-how-to-read).\n\n## Input files and workflow {#intro-workflow}\n\nAn AVAS project has two directories, `InputFile/` and `OutputFile/`. The input consists of:\n\n| File | Contents | See |\n|---|---|---|\n| `beam.txt` | particle (`particlerestmass`, `numofcharge`), kinetic energy `kneticenergy`, current `current`, beam frequency `frequency`, number of macro-particles, distribution type `distribution`, Twiss parameters `twissx`/`twissy`/`twissz` | [Kinematics](#kinematics-in-avas), [Phase space](#phase-space-in-avas) |\n| `input.txt` | simulation type `sim_type`, step `steppercycle`, space-charge switch and solver, grid, output settings … | [Simulation settings](#simulation) |\n| lattice file | the element sequence: the lines between `start` and `end`; `!` starts a comment | [Lattice design](#lattice-design) |\n| field-map files | the 3-D field distributions referenced by field-map elements, one file per component (e.g. `hwr010.edz`, `hwr010.edx`, `sol.bsz` …) | [Field maps](#field-maps) |\n| `ini.ini` | project settings; `[lattice] source` selects the lattice file used for runs | — |\n\nThe example project `examples/hwr010` uses a 1.527 MeV, 0.18 mA, 162.5 MHz proton beam; its lattice contains a single 0.21 m HWR RF cavity field map:\n\n```text\nstart\nfield  0.21   0.02     0   1   162.5e6   -33   1.36    -1.36   efield\nend\n```\n\nA complete run goes like this:\n\n1. Edit the inputs on the "Beam", "Lattice" and "Settings" pages of the interface (or edit the text files directly).\n2. Start the run on the "Run" page. A run **never rewrites** InputFile: the input files are first copied to `<output dir>/inputs/`; the kernel uses the copies as input and the original directory as field-map directory.\n3. At the start of the run the kernel recreates `OutputFile/DataSet.txt` and then appends one row at a time; the interface reads the new rows every second and shows the beam envelope live.\n4. When the run has finished, plot on the "Results" page or replay the run on the "Run" page.\n\n## Outputs and results {#intro-outputs}\n\nThe most used output is `DataSet.txt`: every row holds the bunch statistics at one time (or position), 41 columns in all. Column 0 is the kinetic energy of the reference particle (MeV), the longitudinal position $z$ is the sum of columns 5 and 33, the rms sizes $x$, $y$, $z$ are in columns 16, 18, 20 (in m), the maxima in columns 22, 24, 26, and the number of surviving macro-particles in column 28. Other outputs include:\n\n- `inData.dst`: the initial particle distribution actually used by the simulation;\n- `outData_<z>.dst`: the particle distribution at an output plane (binary); at the end the kernel always writes the distribution at the end of the lattice;\n- `Phase.txt`: time, position and energy of the synchronous particle at the entry and exit of each RF cavity (two lines per cavity);\n- `synData.txt`: time, $\\gamma\\beta$, synchronous phase $\\varphi_s$ and RF phase $\\varphi_{\\mathrm{RF}}$ of the synchronous particle at the entry of every element;\n- the results of error studies, under `OutputFile/error_output/`.\n\n> Note: in single-particle runs (`particlenumber 1`) the reference-particle data are written as usual while the rms columns may be NaN; this is normal. With multithreading the kernel gives slightly different results from run to run, so compare two `DataSet.txt` files with a tolerance.\n\n## How to read this primer {#intro-how-to-read}\n\nThe primer has three parts:\n\n- **Fundamentals**: [Relativistic kinematics](#kinematics), [Phase space and emittance](#phase-space), [Linear optics](#linear-optics), [Longitudinal motion](#longitudinal), [Space charge](#space-charge). These chapters set up the concepts and notation used later.\n- **Components**: [Drift](#drift), [Quadrupole](#quadrupole), [Solenoid](#solenoid), [Dipole](#dipole), [Steerer](#steerer), [RF cavity](#rf-cavity), [Static fields](#static-fields), [Field maps](#field-maps), [Superpose](#superpose). Each chapter covers the role of the element, its physics, its transfer matrix, the AVAS keywords, adjustable parameters, common problems and hands-on labs.\n- **Building an accelerator**: [Lattice design](#lattice-design), [Matching](#matching), [Errors and correction](#errors), [Simulation settings](#simulation), [Tutorial](#tutorial), [References](#references).\n\nKeep the following in mind while reading:\n\n- **Labs are linear models.** The lab blocks are computed on the back end with AVAS\'s own linear-optics code and update live as you drag the sliders; they are meant to build intuition. They use the same approximations as the linear envelope preview: reference-particle energy within about 0.01 %, rms sizes usually within 1–2 %, up to 13 % when multi-particle effects are strong. **Final conclusions rest on the kernel\'s multi-particle simulation.**\n- **Field pictures** distinguish data from schematics: slices of field-map files are data; the fields of matrix elements are schematics drawn from the parameters and are labelled as such. Cross-sections are always viewed from downstream looking upstream, with the beam coming out of the screen (⊙), so that force directions can be checked directly with $\\boldsymbol F = q\\boldsymbol v \\times \\boldsymbol B$.\n- **"AVAS convention"** boxes mark conventions in which AVAS differs from textbooks or the manual and which have been verified with the kernel (for example the units of the Twiss parameters, `displacepos` being read in mm, `multithreading` accepting only 1).\n- Every chapter carries a review status. The current chapters are **written by AI, awaiting review**: formulas and numbers have been checked by computation, but have not yet been reviewed line by line by a physicist; please report problems to the maintainer.\n\n## Further reading {#intro-further}\n\n- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: the standard textbook on linac physics, covering RF, longitudinal and transverse dynamics and space charge.\n- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: a systematic treatment of beam optics, emittance and space charge.\n- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific.\n- The CERN Accelerator School (CAS) proceedings, in particular "Introduction to Accelerator Physics" and "RF for Accelerators".\n- The "Parameter and file reference" in the AVAS user manual (Help → User manual) and the [References](#references) chapter of this primer.\n',p='# 加速器与 AVAS\n\n本入门面向用 AVAS 做研究的加速器物理工作者和研究生：从直线加速器的基本概念出发，经过相对论运动学、相空间与线性光学，逐个介绍 AVAS 能模拟的元件，最后讲如何把它们组装成一条束线并完成匹配、误差研究和多粒子模拟。本章先给出全局图景：直线加速器由什么组成、AVAS 模拟的是什么、一次模拟从输入到输出经过哪些文件，以及本入门的阅读方式。\n\n## 直线加速器做什么 {#intro-linac}\n\n直线加速器（linac）让带电粒子沿一条近似直线的参考轨道通过一系列加速间隙和聚焦元件，使粒子获得能量，同时把束流约束在有限的横向尺寸和纵向（相位）范围内。与环形加速器不同，粒子只通过每个元件一次，因此束流品质由**一次通过**的动力学决定：入口束流的相空间分布、各元件的场以及束流自身的空间电荷力共同决定出口的能量、发射度和损失。\n\n描述束流需要两类量：\n\n- **参考粒子**（同步粒子）的运动：它沿设计轨道运动，其动能 $W(s)$ 和到达各处的时刻 $t(s)$ 决定了射频腔的相位和每个元件的"设计状态"；\n- **其他粒子相对参考粒子的偏离**：横向位置和角度 $(x, x\', y, y\')$、纵向位置和动量偏差 $(z, \\delta)$。束流的尺寸、发射度、能散都是这些偏离量的统计量，见 [相空间与发射度](#phase-space)。\n\n对低能强流直线加速器（例如质子、重离子超导直线加速器的低能段），空间电荷、射频散焦和纵向非线性往往同时起作用，这正是多粒子模拟程序存在的原因。\n\n## 束线的基本组成 {#intro-components}\n\n一条直线加速器束线可以按功能分成四类元件，下表列出它们在 AVAS 中的对应写法：\n\n| 功能 | 典型元件 | AVAS 中的写法 | 详见 |\n|---|---|---|---|\n| 横向聚焦 | 四极铁、螺线管 | `quad`、`solenoid`，或静磁场图 `field … 3 …` | [四极铁](#quadrupole)、[螺线管](#solenoid) |\n| 加速与纵向聚焦 | 射频腔（如半波长谐振腔 HWR） | 射频场图 `field … 1 …` | [射频腔](#rf-cavity) |\n| 偏转与轨道校正 | 二极铁、校正铁 | `bend`、`edge`、`steerer` | [二极铁](#dipole)、[校正铁](#steerer) |\n| 束流诊断与输出 | 束诊位置、输出平面 | `diag_energy`、`diag_size`、`diag_position`、`outputplane` | [误差与校正](#errors)、[模拟设置](#simulation) |\n\n此外还有漂移段 `drift`（无场空间，见 [漂移段](#drift)）、静电场图 `field … 2 …`（见 [静电与静磁场元件](#static-fields)），以及让多个场图在空间上重叠的 `superpose` 块（见 [叠加场](#superpose)）。\n\n> 注意：本入门只讨论 AVAS 能够模拟的内容。RFQ、DTL 加速结构的专门模型、离子源等不在 AVAS 的元件之内，书中不作为 AVAS 的功能介绍；如果需要处理 RFQ 出口的束流，可以把其他程序生成的粒子分布文件（`.dst`）作为 AVAS 的初始束流读入（`readparticledistribution`）。\n\n## AVAS 模拟什么 {#intro-what-avas-simulates}\n\nAVAS（Advanced Virtual Accelerator Software）是一个直线加速器多粒子束流动力学程序。计算内核是 C++ 编写的二进制库（Windows 下为 `AVAS.dll`），界面与前后处理由 Python 和网页前端完成。内核做的事情可以概括为：\n\n1. **参考粒子**：从 `beam.txt` 给定的动能出发，沿 lattice 积分参考粒子的能量和时间，确定每个射频腔的入口相位（见 [纵向运动](#longitudinal)）。\n2. **宏粒子**：按 `beam.txt` 的 Twiss 参数和分布类型生成 `particlenumber` 个宏粒子（或读入粒子分布文件）。每个宏粒子代表许多真实粒子，总电荷由流强 `current` 和束流频率 `frequency` 决定（每个束团电荷 $Q = I/f$）。\n3. **外场作用**：场图元件和漂移段用 t-code 按时间步积分洛伦兹力；矩阵元件用 z-code 的传输矩阵（见 [下一节](#intro-tracking)）。\n4. **空间电荷**：`input.txt` 中 `spacecharge 1` 打开后，按 `scmethod` 选择的算法（FFT、PICNIC、SPICNIC）计算束流自场并作用到每个宏粒子上（见 [空间电荷](#space-charge)）。\n5. **误差与校正**：在 lattice 中用 `err_step`、`err_beam_dyn`、`err_beam_stat`、`err_quad_*`、`err_cav_*` 等命令给元件和初始束流加上随机误差，用 `adjust` 和 `diag_*` 做静态误差校正（见 [误差与校正](#errors)）。\n6. **损失**：粒子超出元件孔径 `R` 或纵向边界时记为损失，存活宏粒子数随 $s$ 写入输出。\n\n## t-code 与 z-code {#intro-tracking}\n\nAVAS 的元件分为两种模型，它们的计算方式不同。\n\n### 场模型元件（t-code）\n\n`field` 元件（以及漂移段 `drift`）由用户提供电磁场的三维分布（场图文件）。内核以**时间 $t$ 为自变量**，逐步积分每个宏粒子的运动方程\n\n$$\n\\frac{\\dd \\boldsymbol p}{\\dd t} = q\\left(\\boldsymbol E + \\boldsymbol v \\times \\boldsymbol B\\right) + \\boldsymbol F_{\\mathrm{sc}}\n$$\n\n其中 $\\boldsymbol F_{\\mathrm{sc}}$ 是空间电荷力。这称为 t-code。时间步长由 `input.txt` 的 `steppercycle` 决定（每个射频周期分为若干步）。t-code 自然包含场的非线性、边缘场以及粒子在腔内速度变化等效应，是低能射频腔模拟所必需的。\n\n### 矩阵模型元件（z-code）\n\n`quad`、`solenoid`、`bend`、`edge`、`steerer` 只需几个参数（长度、梯度、磁场等）。内核以**纵向位置 $z$ 为自变量**，用硬边模型的传输矩阵变换粒子坐标，这称为 z-code。它计算快，可以直接与解析理论对照（见 [线性光学](#linear-optics)），但不含连续的边缘场分布（`edge` 只给出极面边缘的薄透镜修正）；内核矩阵元件是否包含高阶项，手册未说明，以内核为准。\n\n两种模型在 lattice 中可以混用；在两者的交界处，粒子分布要在"同一时刻"与"同一纵向位置"两种描述之间转换（具体做法手册未说明，以内核为准）。\n\n> AVAS 约定：多粒子模拟时，lattice 的**第一个和最后一个元件不要用矩阵模型元件**（手册的要求）；需要时在两端加一段很短的 `drift`。\n\n此外，AVAS 的图形界面提供一个**线性包络预览**（可视化编辑器中的"线性预览"）：它不调用内核，而是用 6×6 矩阵推进束流的二阶矩，并对场图做切片近似。它的原理见 [线性光学](#linear-optics-6d)，精度和适用范围见 [本章最后一节](#intro-how-to-read)。\n\n## 输入文件与工作流程 {#intro-workflow}\n\n一个 AVAS 项目包含 `InputFile/` 和 `OutputFile/` 两个目录。输入由以下文件组成：\n\n| 文件 | 内容 | 详见 |\n|---|---|---|\n| `beam.txt` | 粒子种类（`particlerestmass`、`numofcharge`）、动能 `kneticenergy`、流强 `current`、束流频率 `frequency`、宏粒子数、分布类型 `distribution`、Twiss 参数 `twissx`/`twissy`/`twissz` | [运动学](#kinematics-in-avas)、[相空间](#phase-space-in-avas) |\n| `input.txt` | 模拟类型 `sim_type`、步长 `steppercycle`、空间电荷开关与算法、网格、输出设置等 | [模拟设置](#simulation) |\n| lattice 文件 | 元件序列：`start` 与 `end` 之间的各行，`!` 后为注释 | [束线设计](#lattice-design) |\n| 场图文件 | 场图元件引用的三维场分布，按分量分文件（如 `hwr010.edz`、`hwr010.edx`、`sol.bsz` 等） | [场图](#field-maps) |\n| `ini.ini` | 项目设置；`[lattice] source` 指定运行所用的 lattice 文件 | — |\n\n示例项目 `examples/hwr010` 的束流是 1.527 MeV、0.18 mA、162.5 MHz 的质子束，lattice 只有一个 0.21 m 的 HWR 射频腔场图：\n\n```text\nstart\nfield  0.21   0.02     0   1   162.5e6   -33   1.36    -1.36   efield\nend\n```\n\n一次完整运行的流程是：\n\n1. 在界面的"束流""结构""设置"页面编辑输入（或直接编辑文本文件）。\n2. 在"运行"页面开始运行。运行**不会改写** InputFile：输入文件先被复制到 `<输出目录>/inputs/`，内核以副本为输入，以原目录为场图目录。\n3. 内核在运行开始时重建 `OutputFile/DataSet.txt`，之后逐行追加；界面每秒读取新增的行，实时显示束流包络。\n4. 运行结束后在"结果"页面画图，或在"运行"页面回放本次运行。\n\n## 输出与结果 {#intro-outputs}\n\n最常用的输出是 `DataSet.txt`：每一行是某个时刻（或位置）的束团统计量，共 41 列。其中第 0 列是参考粒子动能（MeV），纵向位置 $z$ 为第 5 列与第 33 列之和，rms 尺寸 $x$、$y$、$z$ 在第 16、18、20 列（单位 m），最大值在第 22、24、26 列，存活宏粒子数在第 28 列。其他输出包括：\n\n- `inData.dst`：模拟实际使用的初始粒子分布；\n- `outData_<z>.dst`：输出平面上的粒子分布（二进制），内核结束时总会写出 lattice 末端的分布；\n- `Phase.txt`：每个射频腔入口与出口处同步粒子的时间、位置和能量（每腔两行）；\n- `synData.txt`：同步粒子到达每个元件入口时的时间、$\\gamma\\beta$、同步相位 $\\varphi_s$ 与射频相位 $\\varphi_{\\mathrm{RF}}$；\n- 误差研究的结果放在 `OutputFile/error_output/` 下。\n\n> 注意：单粒子运行（`particlenumber 1`）时参考粒子数据照常输出，而 rms 列可能为 NaN，这是正常的。内核多线程运行时，每次结果略有差别，比较两次 `DataSet.txt` 要带容差。\n\n## 如何阅读本入门 {#intro-how-to-read}\n\n本入门分为三部分：\n\n- **基础**：[相对论运动学](#kinematics)、[相空间与发射度](#phase-space)、[线性光学](#linear-optics)、[纵向运动](#longitudinal)、[空间电荷](#space-charge)。这些章节建立后文需要的概念和记号。\n- **元件**：[漂移段](#drift)、[四极铁](#quadrupole)、[螺线管](#solenoid)、[二极铁](#dipole)、[校正铁](#steerer)、[射频腔](#rf-cavity)、[静电与静磁场元件](#static-fields)、[场图](#field-maps)、[叠加场](#superpose)。每章依次讲作用、物理原理、传输矩阵、AVAS 中的关键字、可调参数、常见问题和动手实验。\n- **组装**：[束线设计](#lattice-design)、[匹配](#matching)、[误差与校正](#errors)、[模拟设置](#simulation)、[教程](#tutorial)、[参考文献](#references)。\n\n阅读时请注意以下几点：\n\n- **动手实验**是线性模型。实验区块在后端用 AVAS 自己的线性光学代码计算，拖动滑块时结果实时更新，适合建立直觉；它们与线性包络预览采用同一套近似：参考粒子能量误差约 0.01 %，rms 尺寸通常相差 1–2 %，多粒子效应强时可达 13 %。**最终结论以内核的多粒子模拟为准。**\n- **场的图形**区分数据与示意：场图文件的切片是数据；矩阵元件的场是按参数画出的示意图，图中会标明。横截面一律从下游往上游看，束流垂直屏幕向外（⊙），这样可以直接用 $\\boldsymbol F = q\\boldsymbol v \\times \\boldsymbol B$ 核对受力方向。\n- **"AVAS 约定"** 框标出 AVAS 与教科书或手册不同、并已用内核核实的约定（例如 Twiss 的单位、`displacepos` 按 mm 读取、`multithreading` 只能写 1）。\n- 各章标有审核状态。当前章节为 **AI 撰写，待审核**：公式和数值经过计算核对，但尚未经物理学家逐条审阅；发现问题请反馈给维护者。\n\n## 延伸阅读 {#intro-further}\n\n- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：直线加速器物理的标准教材，覆盖射频、纵向与横向动力学和空间电荷。\n- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：束流光学、发射度与空间电荷的系统论述。\n- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific。\n- CERN Accelerator School（CAS）各期讲义，尤其是 "Introduction to Accelerator Physics" 与 "RF for Accelerators"。\n- AVAS 使用说明（界面"帮助 → 使用说明"）中的"参数与文件参考"，以及本入门的 [参考文献](#references) 一章。\n',m=`# Relativistic kinematics

Every "strength" in a linac — the focusing strength of a quadrupole, the bending radius of a dipole, the phase slip in an RF cavity — is tied to the beam energy through a few kinematic quantities: the Lorentz factor $\\gamma$, the velocity $\\beta$, the momentum $p$, the magnetic rigidity $B\\rho$ and $\\beta\\lambda$. This chapter gives their definitions, relations and numerical examples, and shows how they correspond to the keywords of \`beam.txt\`.

## Basic quantities {#kinematics-variables}

Let the particle have rest energy $mc^2$, charge $q = Ze$ ($Z$ is the charge number, possibly negative) and kinetic energy $W$. The following relations of special relativity are exact:

$$
\\gamma = 1 + \\frac{W}{mc^2}, \\qquad
\\beta = \\frac{v}{c} = \\sqrt{1 - \\frac{1}{\\gamma^2}}, \\qquad
\\beta\\gamma = \\sqrt{\\gamma^2 - 1}
$$

Total energy and momentum are

$$
E = \\gamma mc^2 = W + mc^2, \\qquad
pc = \\beta\\gamma\\, mc^2 = \\sqrt{W\\left(W + 2mc^2\\right)}, \\qquad
E^2 = (pc)^2 + (mc^2)^2
$$

In accelerators energies are usually given in MeV and momenta in MeV/$c$, so $p\\,[\\mathrm{MeV}/c] = \\beta\\gamma\\, mc^2\\,[\\mathrm{MeV}]$. Common rest energies:

| Particle | $mc^2$ (MeV) |
|---|---|
| electron | 0.51099895 |
| proton | 938.272088 |
| atomic mass unit u | 931.494102 |

> Note: for heavy ions use the rest energy of the ion itself, i.e. the atomic mass minus the mass of the stripped electrons (binding energies can usually be neglected); it is **not** $A \\times 938.272$ MeV.

## Differential relations and energy spread {#kinematics-differential}

The particles of a beam have slightly different energies, and energy deviations must be converted into velocity and momentum deviations. Differentiating the relations above:

$$
\\dd\\gamma = \\frac{\\dd W}{mc^2}, \\qquad
\\frac{\\dd\\beta}{\\beta} = \\frac{\\dd\\gamma}{\\beta^2\\gamma^3} = \\frac{1}{\\gamma^2}\\,\\frac{\\dd p}{p}, \\qquad
\\frac{\\dd p}{p} = \\frac{\\dd\\gamma}{\\beta^2\\gamma}
$$

With the relative momentum deviation $\\delta = \\Delta p / p$ (one of AVAS's longitudinal coordinates, see [Phase space](#phase-space-coordinates)):

$$
\\Delta W = \\beta^2\\gamma\\, mc^2\\,\\delta = v\\,\\Delta p, \\qquad
\\frac{\\Delta W}{W} = \\frac{\\gamma + 1}{\\gamma}\\,\\delta
$$

Some consequences:

- Non-relativistically $\\Delta W / W \\approx 2\\delta$; in the relativistic limit $\\Delta W / W \\to \\delta$.
- The velocity deviation is $\\Delta\\beta/\\beta = \\delta/\\gamma^2$: at low energy the velocity is very sensitive to momentum deviations, at high energy it hardly changes. This is why the $z$–$\\delta$ element of a drift in [linear optics](#linear-optics-6d) is $L/\\gamma^2$, and why electrons above a few MeV show almost no longitudinal phase slip while low-energy protons slip strongly (see [Longitudinal motion](#longitudinal)).
- For the example beam (protons, $W$ = 1.527 MeV): $1/\\gamma^2 = 0.9968$ and $\\Delta W/W = 1.998\\,\\delta$.

## Non-relativistic and ultra-relativistic limits {#kinematics-nonrel}

Write $u = W/(mc^2)$.

### Non-relativistic limit ($u \\ll 1$)

$$
W \\approx \\tfrac12 mv^2, \\qquad \\beta \\approx \\sqrt{2u}, \\qquad pc \\approx \\sqrt{2\\,mc^2\\,W}
$$

The exact expansion is $\\beta = \\sqrt{2u}\\,\\left(1 - \\tfrac34 u + \\dots\\right)$, so the relative error of the non-relativistic formula is about $3u/4$. For 1.527 MeV protons $u = 1.63\\times10^{-3}$: the non-relativistic $\\beta = 0.057052$, the exact value $0.056983$, an error of 0.12 %. Heavy ions and low-energy proton sections can be estimated with the non-relativistic formulas, but the AVAS kernel and the linear preview always use the exact ones.

### Ultra-relativistic limit ($\\gamma \\gg 1$)

$$
\\beta \\approx 1 - \\frac{1}{2\\gamma^2}, \\qquad pc \\approx E \\approx W
$$

At 1 MeV an electron has $\\gamma = 2.96$ and $\\beta = 0.941$; the non-relativistic formula would give $\\beta \\approx 1.98$ and fails completely. Electron beams must always be treated relativistically.

## Magnetic and electric rigidity {#kinematics-rigidity}

### Magnetic rigidity

A particle in a uniform magnetic field $B$ perpendicular to its velocity moves on a circle; the Lorentz force provides the centripetal force, $qvB = \\gamma m v^2/\\rho = pv/\\rho$, hence

$$
B\\rho = \\frac{p}{q}
$$

$B\\rho$ is the **magnetic rigidity**; it depends only on the ratio of momentum to charge. In practical units:

$$
\\Brho\\,[\\mathrm{T\\cdot m}] = \\frac{p\\,[\\mathrm{MeV}/c]}{299.792458\\,\\lvert Z \\rvert} = 3.3356\\,\\frac{p\\,[\\mathrm{GeV}/c]}{\\lvert Z \\rvert}
$$

The strengths of all magnetic elements are normalised by $B\\rho$:

- dipole bending radius $\\rho = B\\rho / B$ (see [Dipole](#dipole));
- quadrupole focusing strength $k = G/(B\\rho)$ in m$^{-2}$ (see [Quadrupole](#quadrupole));
- solenoid $k = B/(2B\\rho)$ in m$^{-1}$ (see [Solenoid](#solenoid)).

The same magnet therefore acts differently on beams of different energy or charge-to-mass ratio; after changing the beam energy the fields must be scaled in proportion to $B\\rho$ to keep the same optics.

### Electric rigidity

In a transverse electric field $E$, $qE = pv/\\rho$, which defines the **electric rigidity**

$$
E\\rho = \\frac{pv}{q} = \\beta c\\,(B\\rho), \\qquad
E\\rho\\,[\\mathrm{MV}] = \\frac{\\beta\\,pc\\,[\\mathrm{MeV}]}{\\lvert Z \\rvert}
$$

Non-relativistically $pv \\approx 2W$, i.e. $E\\rho \\approx 2W/Z$ ($E\\rho$ in MV for $W$ in MeV). An electrostatic element bends with radius $\\rho = E\\rho/E$, and an electrostatic quadrupole has strength $k = (\\partial E_x/\\partial x)/(E\\rho)$. Electrostatic elements are thus effective for slow particles ($E\\rho \\propto \\beta^2$), while at high energy magnetic elements are far more economical. Electrostatic field elements are covered in [Static fields](#static-fields).

> Note: the sign of the rigidity follows the sign of the charge. The practical formulas above use $\\lvert Z \\rvert$; for a negative particle the same magnetic field produces the opposite force, and the focusing planes of a quadrupole are swapped.

## RF wavelength and βλ {#kinematics-rf}

An RF frequency $f$ corresponds to the free-space wavelength and period

$$
\\lambda = \\frac{c}{f}, \\qquad T_{\\mathrm{RF}} = \\frac{1}{f}
$$

In one RF period the particle travels a distance $\\beta\\lambda$. This is the basic longitudinal length scale of a linac:

- gap spacing: in a π-mode structure neighbouring gaps are $\\beta\\lambda/2$ apart, in a 0-mode structure (DTL type) $\\beta\\lambda$;
- conversion between longitudinal position and phase: a particle a distance $\\Delta z$ ahead of the reference particle reaches an accelerating gap earlier and sees a smaller RF phase, $\\Delta\\varphi = -2\\pi\\,\\Delta z/(\\beta\\lambda)$;
- the transit-time factor depends on $\\beta$, so a cavity accelerates efficiently only within a certain range of $\\beta$ (see [RF cavity](#rf-cavity)).

Example: at 162.5 MHz, $\\lambda = 1.8449$ m and $T_{\\mathrm{RF}}$ = 6.154 ns. For 1.527 MeV protons $\\beta\\lambda$ = 0.10513 m, i.e. 1° of RF phase corresponds to 0.292 mm. Crossing the 0.21 m HWR field map at the initial velocity takes the reference particle about $0.21/(\\beta c)$ = 12.29 ns, about 2.0 RF periods.

## Numerical examples {#kinematics-examples}

### Protons ($mc^2$ = 938.272 MeV, $\\beta\\lambda$ at 162.5 MHz)

| $W$ (MeV) | $\\gamma$ | $\\beta$ | $\\beta\\gamma$ | $p$ (MeV/$c$) | $B\\rho$ (T·m) | $E\\rho$ (MV) | $\\beta\\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 1.527 | 1.001627 | 0.056983 | 0.057076 | 53.55 | 0.17863 | 3.052 | 0.10513 |
| 10 | 1.010658 | 0.144844 | 0.146388 | 137.35 | 0.45816 | 19.89 | 0.26722 |
| 100 | 1.106579 | 0.428195 | 0.473832 | 444.58 | 1.48297 | 190.4 | 0.78997 |
| 1000 | 2.065789 | 0.875026 | 1.807618 | 1696.04 | 5.65737 | 1484.1 | 1.61431 |

The first row is the beam of the example project: a 0.18 T dipole field bends it with $\\rho$ = 0.992 m; a 5 MV/m transverse electric field corresponds to $\\rho$ = 0.610 m.

### Electrons ($mc^2$ = 0.511 MeV, $\\beta\\lambda$ at 2856 MHz, $\\lambda$ = 0.10497 m)

| $W$ (MeV) | $\\gamma$ | $\\beta$ | $\\beta\\gamma$ | $p$ (MeV/$c$) | $B\\rho$ (T·m) | $E\\rho$ (MV) | $\\beta\\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 0.1 | 1.195695 | 0.548221 | 0.655505 | 0.3350 | 0.001117 | 0.1836 | 0.05755 |
| 1 | 2.956951 | 0.941079 | 2.782725 | 1.4220 | 0.004743 | 1.338 | 0.09878 |
| 10 | 20.56951 | 0.998818 | 20.54519 | 10.499 | 0.03502 | 10.49 | 0.10485 |
| 100 | 196.6951 | 0.999987 | 196.6926 | 100.51 | 0.33526 | 100.51 | 0.10497 |

### Heavy ions (energy given per nucleon)

Take $^{40}$Ar$^{8+}$ at 0.5 MeV per nucleon: the ion rest energy is about $39.9624 \\times 931.494 - 8\\times0.511$ = 37220.6 MeV and the total kinetic energy $W = 40\\times0.5$ = 20 MeV, so $\\beta$ = 0.03277, $p$ = 1220.3 MeV/$c$ and $B\\rho = p/(299.79\\times8)$ = 0.5088 T·m. The smaller the charge-to-mass ratio $Z/A$, the larger the rigidity at a given velocity and the harder the beam is to focus.

## In AVAS {#kinematics-in-avas}

The kinematic keywords of \`beam.txt\`:

\`\`\`avas-params
beam:particlerestmass beam:numofcharge beam:kneticenergy beam:frequency beam:current
\`\`\`

- \`particlerestmass\`: rest energy $mc^2$ in MeV (938.272088159928 in the example project, i.e. a proton).
- \`numofcharge\`: charge number $Z$ in units of $e$. It enters both the rigidity $B\\rho = p/(Ze)$ and space charge (the charge of each particle).
- \`kneticenergy\`: kinetic energy $W$ of the reference particle in MeV (spelled exactly \`kneticenergy\`; not in the manual's table, written by the Beam page).
- \`frequency\`: beam frequency in **Hz** (\`162500000.0\` in the example), used to convert longitudinal position into phase and to fix the bunch charge $Q = I/f$.
- \`current\`: beam current in mA.

> AVAS convention: the linear preview of the interface computes $\\gamma = 1 + W/m$, so \`kneticenergy\` and \`particlerestmass\` refer to the **whole particle** (not per nucleon). For heavy ions write the total mass and total kinetic energy of the ion, e.g. for the $^{40}$Ar$^{8+}$ above \`particlerestmass 37220.6\`, \`kneticenergy 20\`, \`numofcharge 8\`.

> Note: both \`frequency\` in \`beam.txt\` and the frequency parameter \`f\` of the \`field\` element in the lattice are in Hz; the \`f\` of this chapter's lab is in MHz only for convenient dragging. A negatively charged particle (e.g. an electron) should by definition be given a negative charge number; the manual does not describe how the kernel handles negative charges, so check the focusing direction of a quadrupole with a simple lattice before relying on it.

The bunch charge of the example project is $Q = 0.1841\\,\\mathrm{mA} / 162.5\\,\\mathrm{MHz}$ = 1.133 pC, about $7.07\\times10^6$ protons; with 5260 macro-particles each macro-particle stands for about 1340 protons.

Kinematics in the output: column 0 of \`DataSet.txt\` is the kinetic energy $W$ of the reference particle (MeV); with this chapter's formulas you can compute $\\beta\\gamma$ at every position and from it the geometric emittance from the normalised one (see [Phase space](#phase-space-emittance)).

## Hands-on lab {#kinematics-lab}

Drag the kinetic energy $W$ and watch $\\beta$, $\\beta\\gamma$, $B\\rho$, $E\\rho$, $\\beta\\lambda$ and the $\\beta(W)$ curve. Suggestions:

1. Start from the example proton beam and confirm the numbers in the table; raise $W$ to 10 times its value (15.27 MeV): $B\\rho$ grows only to about 3.17 times (close to $\\sqrt{10}$), while $E\\rho$ grows to about 9.93 times (close to 10).
2. Change the mass to 0.511 (electrons) and watch $\\beta$ approach 1 quickly above 1 MeV and $\\beta\\lambda$ approach $\\lambda$.
3. Keep the velocity fixed and change the charge number: the rigidity scales as $1/Z$.

\`\`\`avas-lab
kinematics mass=938.272 q=1 W=1.527 f=162.5
\`\`\`

Electron example (2856 MHz):

\`\`\`avas-lab
kinematics mass=0.511 q=1 W=1 f=2856
\`\`\`

## Further reading {#kinematics-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: relativistic relations, $\\beta\\lambda$ and the length scales of accelerating structures.
- S. Y. Lee, *Accelerator Physics*, 4th ed.: magnetic rigidity and practical units.
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015.
- Continue with [Phase space and emittance](#phase-space).
`,ee=`# 相对论运动学

直线加速器里所有的"强度"——四极铁的聚焦强度、二极铁的偏转半径、射频腔的相位滑移——都通过几个运动学量与束流能量联系在一起：洛伦兹因子 $\\gamma$、速度 $\\beta$、动量 $p$、磁刚度 $B\\rho$ 和 $\\beta\\lambda$。本章给出这些量的定义、相互关系和数值示例，并说明它们与 \`beam.txt\` 关键字的对应。

## 基本量 {#kinematics-variables}

设粒子静止能量为 $mc^2$，电荷为 $q = Ze$（$Z$ 为电荷数，可为负），动能为 $W$。以下关系是精确的狭义相对论关系：

$$
\\gamma = 1 + \\frac{W}{mc^2}, \\qquad
\\beta = \\frac{v}{c} = \\sqrt{1 - \\frac{1}{\\gamma^2}}, \\qquad
\\beta\\gamma = \\sqrt{\\gamma^2 - 1}
$$

总能量与动量为

$$
E = \\gamma mc^2 = W + mc^2, \\qquad
pc = \\beta\\gamma\\, mc^2 = \\sqrt{W\\left(W + 2mc^2\\right)}, \\qquad
E^2 = (pc)^2 + (mc^2)^2
$$

在加速器中习惯用 MeV 作为能量单位、MeV/$c$ 作为动量单位，于是 $p\\,[\\mathrm{MeV}/c] = \\beta\\gamma\\, mc^2\\,[\\mathrm{MeV}]$。常用的静止能量：

| 粒子 | $mc^2$ (MeV) |
|---|---|
| 电子 | 0.51099895 |
| 质子 | 938.272088 |
| 原子质量单位 u | 931.494102 |

> 注意：重离子的静止能量应取离子本身的质量，即原子质量减去剥离电子的质量（结合能通常可忽略）；它**不等于** $A \\times 938.272$ MeV。

## 微分关系与能散 {#kinematics-differential}

束流中各粒子的能量略有不同，需要把能量偏差换算成速度、动量偏差。对上面的关系求微分：

$$
\\dd\\gamma = \\frac{\\dd W}{mc^2}, \\qquad
\\frac{\\dd\\beta}{\\beta} = \\frac{\\dd\\gamma}{\\beta^2\\gamma^3} = \\frac{1}{\\gamma^2}\\,\\frac{\\dd p}{p}, \\qquad
\\frac{\\dd p}{p} = \\frac{\\dd\\gamma}{\\beta^2\\gamma}
$$

定义相对动量偏差 $\\delta = \\Delta p / p$（AVAS 纵向坐标之一，见 [相空间](#phase-space-coordinates)），则

$$
\\Delta W = \\beta^2\\gamma\\, mc^2\\,\\delta = v\\,\\Delta p, \\qquad
\\frac{\\Delta W}{W} = \\frac{\\gamma + 1}{\\gamma}\\,\\delta
$$

几点推论：

- 非相对论时 $\\Delta W / W \\approx 2\\delta$，相对论极限 $\\Delta W / W \\to \\delta$。
- 速度偏差 $\\Delta\\beta/\\beta = \\delta/\\gamma^2$：低能时速度对动量偏差很敏感，高能时速度几乎不变。这正是 [线性光学](#linear-optics-6d) 中漂移段 $z$–$\\delta$ 矩阵元为 $L/\\gamma^2$ 的原因，也是电子在几 MeV 以上几乎没有纵向相位滑移、而低能质子有强烈相位滑移的原因（见 [纵向运动](#longitudinal)）。
- 以示例束流（质子，$W$ = 1.527 MeV）为例：$1/\\gamma^2 = 0.9968$，$\\Delta W/W = 1.998\\,\\delta$。

## 非相对论与极端相对论极限 {#kinematics-nonrel}

记 $u = W/(mc^2)$。

### 非相对论极限（$u \\ll 1$）

$$
W \\approx \\tfrac12 mv^2, \\qquad \\beta \\approx \\sqrt{2u}, \\qquad pc \\approx \\sqrt{2\\,mc^2\\,W}
$$

精确展开为 $\\beta = \\sqrt{2u}\\,\\left(1 - \\tfrac34 u + \\dots\\right)$，所以非相对论公式的相对误差约为 $3u/4$。对 1.527 MeV 质子，$u = 1.63\\times10^{-3}$，非相对论 $\\beta = 0.057052$，精确值 $0.056983$，误差 0.12 %。重离子和低能质子段通常可以先用非相对论公式估算，但 AVAS 内核和线性预览都使用精确公式。

### 极端相对论极限（$\\gamma \\gg 1$）

$$
\\beta \\approx 1 - \\frac{1}{2\\gamma^2}, \\qquad pc \\approx E \\approx W
$$

电子在 1 MeV 时 $\\gamma = 2.96$、$\\beta = 0.941$；用非相对论公式会得到 $\\beta \\approx 1.98$，完全失效。因此电子束必须一直用相对论公式。

## 磁刚度与电刚度 {#kinematics-rigidity}

### 磁刚度

粒子在垂直于速度的均匀磁场 $B$ 中做圆周运动，洛伦兹力提供向心力：$qvB = \\gamma m v^2/\\rho = pv/\\rho$，于是

$$
B\\rho = \\frac{p}{q}
$$

$B\\rho$ 称为**磁刚度**，只取决于粒子的动量与电荷之比。实用单位：

$$
\\Brho\\,[\\mathrm{T\\cdot m}] = \\frac{p\\,[\\mathrm{MeV}/c]}{299.792458\\,\\lvert Z \\rvert} = 3.3356\\,\\frac{p\\,[\\mathrm{GeV}/c]}{\\lvert Z \\rvert}
$$

所有磁元件的强度都要用 $B\\rho$ 归一化：

- 二极铁偏转半径 $\\rho = B\\rho / B$（见 [二极铁](#dipole)）；
- 四极铁聚焦强度 $k = G/(B\\rho)$，单位 m$^{-2}$（见 [四极铁](#quadrupole)）；
- 螺线管 $k = B/(2B\\rho)$，单位 m$^{-1}$（见 [螺线管](#solenoid)）。

所以同一块磁铁对不同能量、不同荷质比的束流强度不同；改变束流能量后，要按 $B\\rho$ 等比例缩放磁场才能保持同样的光学。

### 电刚度

在横向电场 $E$ 中，$qE = pv/\\rho$，定义**电刚度**

$$
E\\rho = \\frac{pv}{q} = \\beta c\\,(B\\rho), \\qquad
E\\rho\\,[\\mathrm{MV}] = \\frac{\\beta\\,pc\\,[\\mathrm{MeV}]}{\\lvert Z \\rvert}
$$

非相对论时 $pv \\approx 2W$，即 $E\\rho \\approx 2W/Z$（$W$ 以 MeV 计时 $E\\rho$ 以 MV 计）。静电元件的偏转半径 $\\rho = E\\rho/E$，静电四极的强度 $k = (\\partial E_x/\\partial x)/(E\\rho)$。可见静电元件对低速粒子有效（$E\\rho \\propto \\beta^2$），而高能时磁元件远比电元件经济。静电场元件见 [静电与静磁场元件](#static-fields)。

> 注意：刚度的符号由电荷符号决定。上面的实用公式取了 $\\lvert Z \\rvert$；对负粒子，同样的磁场产生相反方向的力，四极铁的聚焦平面也随之互换。

## 射频波长与 βλ {#kinematics-rf}

射频频率 $f$ 对应自由空间波长和周期

$$
\\lambda = \\frac{c}{f}, \\qquad T_{\\mathrm{RF}} = \\frac{1}{f}
$$

粒子在一个射频周期内走过的距离是 $\\beta\\lambda$。它是直线加速器纵向尺度的基本单位：

- 加速间隙的间距：π 模结构相邻间隙相距 $\\beta\\lambda/2$，0 模（如 DTL 类型）相距 $\\beta\\lambda$；
- 纵向位置与相位的换算：在参考粒子前方 $\\Delta z$ 处的粒子较早到达加速间隙，它看到的射频相位较小，$\\Delta\\varphi = -2\\pi\\,\\Delta z/(\\beta\\lambda)$；
- 渡越时间因子随 $\\beta$ 变化，一个腔只在一定的 $\\beta$ 范围内有效加速（见 [射频腔](#rf-cavity)）。

示例：162.5 MHz 时 $\\lambda = 1.8449$ m，$T_{\\mathrm{RF}}$ = 6.154 ns。1.527 MeV 质子 $\\beta\\lambda$ = 0.10513 m，即 1° 射频相位对应 0.292 mm。0.21 m 长的 HWR 场图，参考粒子按初速度渡越约需 $0.21/(\\beta c)$ = 12.29 ns，约 2.0 个射频周期。

## 数值示例 {#kinematics-examples}

### 质子（$mc^2$ = 938.272 MeV，$\\beta\\lambda$ 按 162.5 MHz）

| $W$ (MeV) | $\\gamma$ | $\\beta$ | $\\beta\\gamma$ | $p$ (MeV/$c$) | $B\\rho$ (T·m) | $E\\rho$ (MV) | $\\beta\\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 1.527 | 1.001627 | 0.056983 | 0.057076 | 53.55 | 0.17863 | 3.052 | 0.10513 |
| 10 | 1.010658 | 0.144844 | 0.146388 | 137.35 | 0.45816 | 19.89 | 0.26722 |
| 100 | 1.106579 | 0.428195 | 0.473832 | 444.58 | 1.48297 | 190.4 | 0.78997 |
| 1000 | 2.065789 | 0.875026 | 1.807618 | 1696.04 | 5.65737 | 1484.1 | 1.61431 |

第一行就是示例项目的束流：0.18 T 的二极场把它偏转到 $\\rho$ = 0.992 m；5 MV/m 的横向电场对应 $\\rho$ = 0.610 m。

### 电子（$mc^2$ = 0.511 MeV，$\\beta\\lambda$ 按 2856 MHz，$\\lambda$ = 0.10497 m）

| $W$ (MeV) | $\\gamma$ | $\\beta$ | $\\beta\\gamma$ | $p$ (MeV/$c$) | $B\\rho$ (T·m) | $E\\rho$ (MV) | $\\beta\\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 0.1 | 1.195695 | 0.548221 | 0.655505 | 0.3350 | 0.001117 | 0.1836 | 0.05755 |
| 1 | 2.956951 | 0.941079 | 2.782725 | 1.4220 | 0.004743 | 1.338 | 0.09878 |
| 10 | 20.56951 | 0.998818 | 20.54519 | 10.499 | 0.03502 | 10.49 | 0.10485 |
| 100 | 196.6951 | 0.999987 | 196.6926 | 100.51 | 0.33526 | 100.51 | 0.10497 |

### 重离子（按每核子能量给出时）

以 $^{40}$Ar$^{8+}$、每核子 0.5 MeV 为例：离子静止能量约 $39.9624 \\times 931.494 - 8\\times0.511$ = 37220.6 MeV，总动能 $W = 40\\times0.5$ = 20 MeV，于是 $\\beta$ = 0.03277、$p$ = 1220.3 MeV/$c$、$B\\rho = p/(299.79\\times8)$ = 0.5088 T·m。荷质比 $Z/A$ 越小，同样速度下的刚度越大，聚焦越困难。

## 在 AVAS 中 {#kinematics-in-avas}

\`beam.txt\` 中与运动学有关的关键字：

\`\`\`avas-params
beam:particlerestmass beam:numofcharge beam:kneticenergy beam:frequency beam:current
\`\`\`

- \`particlerestmass\`：粒子静止能量 $mc^2$，单位 MeV（示例项目为 938.272088159928，即质子）。
- \`numofcharge\`：电荷数 $Z$，单位 $e$。它同时进入刚度 $B\\rho = p/(Ze)$ 和空间电荷（每个粒子的电荷）。
- \`kneticenergy\`：参考粒子的动能 $W$，单位 MeV（拼写就是 \`kneticenergy\`，手册的表格未收录，由束流页写入）。
- \`frequency\`：束流频率，单位 **Hz**（示例为 \`162500000.0\`），用来把纵向位置换算成相位并确定束团电荷 $Q = I/f$。
- \`current\`：流强，单位 mA。

> AVAS 约定：界面的线性预览按 $\\gamma = 1 + W/m$ 计算，\`kneticenergy\` 和 \`particlerestmass\` 都是**整个粒子**的量（不是每核子的量）。重离子请写离子的总质量和总动能，例如上面的 $^{40}$Ar$^{8+}$ 写 \`particlerestmass 37220.6\`、\`kneticenergy 20\`、\`numofcharge 8\`。

> 注意：\`beam.txt\` 的 \`frequency\` 和 lattice 中 \`field\` 元件的频率参数 \`f\` 都以 Hz 为单位；本章动手实验里的 \`f\` 以 MHz 为单位，仅为方便拖动。负电荷粒子（如电子）按定义应写负的电荷数；手册没有专门说明内核对负电荷的处理，使用前请用简单的 lattice 核对四极铁的聚焦方向。

示例项目的束团电荷 $Q = 0.1841\\,\\mathrm{mA} / 162.5\\,\\mathrm{MHz}$ = 1.133 pC，约 $7.07\\times10^6$ 个质子；用 5260 个宏粒子时，每个宏粒子代表约 1340 个质子。

运动学量在输出中的体现：\`DataSet.txt\` 第 0 列是参考粒子动能 $W$（MeV），可以用本章公式换算出每个位置的 $\\beta\\gamma$，进而由归一化发射度求几何发射度（见 [相空间](#phase-space-emittance)）。

## 动手实验 {#kinematics-lab}

拖动动能 $W$，观察 $\\beta$、$\\beta\\gamma$、$B\\rho$、$E\\rho$、$\\beta\\lambda$ 的变化以及 $\\beta(W)$ 曲线。建议：

1. 从示例质子束开始，确认表中的数值；把 $W$ 增大到 10 倍（15.27 MeV），$B\\rho$ 只增大到约 3.17 倍（接近 $\\sqrt{10}$），而 $E\\rho$ 增大到约 9.93 倍（接近 10）。
2. 把质量改成 0.511（电子），观察 $\\beta$ 在 1 MeV 以上迅速趋于 1、$\\beta\\lambda$ 趋于 $\\lambda$。
3. 保持速度不变、改变电荷数，观察刚度与 $1/Z$ 成正比。

\`\`\`avas-lab
kinematics mass=938.272 q=1 W=1.527 f=162.5
\`\`\`

电子示例（2856 MHz）：

\`\`\`avas-lab
kinematics mass=0.511 q=1 W=1 f=2856
\`\`\`

## 延伸阅读 {#kinematics-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.：相对论关系、$\\beta\\lambda$ 与加速结构的尺度。
- S. Y. Lee, *Accelerator Physics*, 4th ed.：磁刚度与实用单位。
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015。
- 继续阅读：[相空间与发射度](#phase-space)。
`,te=`# Putting an accelerator together

The previous chapters dealt with single components. This chapter is about combining drifts, quadrupoles, solenoids and RF cavities into a linac lattice that transports and accelerates the beam stably: choose the focusing period and the phase advance per period, arrange the cavities and the synchronous-phase law, handle the transitions between sections, check the aperture margin, and finally write it all as an AVAS lattice file. The chapter gives design rules and their physical origin; the [hands-on tutorial](#tutorial) is the practical version, building a short superconducting linac step by step from the example HWR cavity and solenoid field maps.

The notation of the [linear optics](#linear-optics), [longitudinal dynamics](#longitudinal) and [space charge](#space-charge) chapters is assumed: transverse coordinates $(x, x', y, y')$, longitudinal $(z, \\delta = \\Delta p/p)$, period length $L_p$, phase advance per period $\\sigma$ (in degrees), zero-current phase advance $\\sigma_0$.

## Functional sections of a linac {#lattice-design-overview}

A high-intensity proton or heavy-ion linac usually consists of the following parts (in beam direction):

| Section | Purpose | Typical elements | Simulated by AVAS |
|---|---|---|---|
| Ion source | produces a DC beam | extraction electrodes | no |
| LEBT, low-energy transport | matches the DC beam into the RFQ | solenoids, chopper | no (usually) |
| RFQ | bunches, focuses, accelerates to a few MeV | four-vane / four-rod electrodes | no |
| MEBT, medium-energy transport | transverse and longitudinal matching, chopping, diagnostics | quadrupoles, buncher cavities | yes |
| Accelerating sections | main energy gain | superconducting HWR / spoke / elliptical cavities + solenoids or quadrupoles | yes (cavities as field maps) |
| HEBT, high-energy transport | transport to the target or downstream | quadrupoles, dipoles | yes |

AVAS tracks from the initial beam given in \`beam.txt\` (or a \`.dst\` particle file such as the example's \`part_rfq.dst\`), so it describes the lattice **downstream of a given entrance beam**. Ion sources, RFQs and the accelerating structures of a DTL are not elements AVAS models directly; if a three-dimensional field map of such a structure exists it can in principle be read as a \`field\` element, but this chapter does not discuss it.

The example project \`examples/hwr010\` corresponds to the start of a superconducting section: protons, $W = 1.527\\ \\mathrm{MeV}$, $f = 162.5\\ \\mathrm{MHz}$, $I = 0.18\\ \\mathrm{mA}$. For this beam:

$$
\\gamma = 1.00163,\\quad \\beta = 0.05698,\\quad \\beta\\gamma = 0.05708,\\quad B\\rho = 0.1786\\ \\mathrm{T\\,m},\\quad \\beta\\lambda = 0.1051\\ \\mathrm{m}
$$

$\\beta\\lambda$ is the distance a particle travels in one RF period; it sets the scale of cavity gaps and period lengths. $B\\rho$ sets how strongly a given quadrupole gradient or solenoid field focuses (see [kinematics](#kinematics)).

## Focusing periods {#lattice-design-periods}

Transverse focusing in a linac is almost always periodic: the same "period" (cell) is repeated many times. The transfer matrix $M$ of one period is stable when

$$
\\left\\lvert \\tfrac{1}{2}\\Tr M \\right\\rvert < 1,\\qquad \\cos\\sigma = \\tfrac{1}{2}\\Tr M
$$

Then there is a unique periodic set of Twiss parameters $(\\alpha, \\beta)$ (the matched beam, see [matching](#matching)) and the envelope oscillates in step with the period. Common periods follow.

### FODO

Focusing (F) and defocusing (D) quadrupoles alternate, with drifts or cavities between them. For thin lenses of focal length $\\pm f$ spaced $L_p/2$ apart:

$$
\\sin\\frac{\\sigma}{2} = \\frac{L_p}{4f},\\qquad
\\beta_{\\max,\\min} = \\frac{L_p\\,(1 \\pm \\sin\\frac{\\sigma}{2})}{\\sin\\sigma}
$$

Stability requires $f > L_p/4$. FODO is symmetric in the two planes and needs few elements per period; on the other hand $\\beta_{\\max}/\\beta_{\\min}$ grows with the phase advance, and the beam is largest in the F quadrupoles.

Example: the example beam, quadrupoles $L_q = 0.1\\ \\mathrm{m}$, $G = 5\\ \\mathrm{T/m}$, drifts $L_d = 0.3\\ \\mathrm{m}$ (period $L_p = 0.8\\ \\mathrm{m}$). $k = G/B\\rho = 27.99\\ \\mathrm{m^{-2}}$ and the thin-lens focal length is $f = 1/(kL_q) = 0.357\\ \\mathrm{m}$. The thin-lens formula gives $\\sigma = 68.1^\\circ$; the exact thick-lens matrices give $\\sigma = 61.4^\\circ$, with periodic $\\beta_x = 1.35\\ \\mathrm{m}$ at the centre of the F quadrupole and $\\beta_x = 0.45\\ \\mathrm{m}$ at the centre of the D quadrupole. When $kL_q^2$ is not small (here $kL_q^2 = 0.28$) the thin-lens approximation overestimates the phase advance; design with the thick matrices.

### Doublets and triplets

Placing the F and D quadrupoles close together (FD + long drift) leaves a long free space for cavities. Doublet periods use the long drift efficiently but the $\\beta$ functions differ between the planes; triplets (FDF or DFD) restore the symmetry. Their stability regions differ from FODO and must be checked with the actual $\\Tr M$.

### Solenoid periods

Low-energy superconducting sections often use solenoid + cavity periods (S–C, S–C–C, …). A solenoid focuses both planes; in the rotating Larmor frame its thin-lens focal length is

$$
\\frac{1}{f} = \\int k^2\\,\\dd s,\\qquad k = \\frac{B_z}{2B\\rho}
$$

(see [solenoid](#solenoid)). A period of one thin solenoid and a drift $L_p$ has $\\cos\\sigma = 1 - L_p/(2f)$. Solenoid focusing scales as $1/(B\\rho)^2$, so at low energy (small $B\\rho$) it is "cheaper" than quadrupole focusing, while at higher energy quadrupoles are more effective — one reason why low-energy superconducting sections use solenoids and higher-energy sections switch to quadrupoles.

> Note: a solenoid rotates the beam by $kL$ about the axis. A round beam is unchanged by the rotation, but a non-round beam or a beam with transverse errors becomes x–y coupled; the AVAS 2D/3D views and DataSet show laboratory-frame quantities.

## Choosing the phase advance per period {#lattice-design-phase-advance}

### Transverse zero-current phase advance σ0t < 90°

The most important design rule is: **the zero-current transverse phase advance per focusing period, $\\sigma_{0t}$, stays below $90^\\circ$**. The reason is a parametric resonance of the envelope. Small envelope oscillations of a round beam in a uniform focusing channel have two eigenmodes (see [matching](#matching-envelope-modes)):

$$
k_{\\mathrm{breathing}} = \\sqrt{2k_0^2 + 2k^2},\\qquad k_{\\mathrm{quadrupole}} = \\sqrt{k_0^2 + 3k^2}
$$

where $k_0$ and $k$ are the zero-current and space-charge-depressed transverse wave numbers. At zero current both equal $2k_0$: the envelope oscillates with twice the particle phase advance. Periodic focusing modulates the envelope once per $360^\\circ$ of the period, and when the envelope oscillation advances $180^\\circ$ per period (half-integer parametric resonance) the **envelope instability** occurs, i.e. at $\\sigma_0 = 90^\\circ$. Space charge lowers the envelope-mode frequencies, so the unstable band lies at $\\sigma_0 > 90^\\circ$ with high enough current (depressed $\\sigma$); for $\\sigma_0 < 90^\\circ$ the envelope modes advance less than $180^\\circ$ per period and do not enter this band. To leave margin for errors and current, high-intensity designs usually take $\\sigma_{0t} \\approx 50^\\circ$–$80^\\circ$. In addition, when $\\sigma_0 > 90^\\circ$ and space charge depresses the phase advance to about $90^\\circ$, a fourth-order resonance ($4\\sigma \\approx 360^\\circ$) is excited and causes emittance growth.

### Too small a phase advance is not good either

A small phase advance means weak focusing, large $\\beta$ functions and large beams, relatively stronger space charge (smaller tune depression $\\sigma/\\sigma_0$), and larger apertures. For the example FODO ($\\sigma_0 = 61.4^\\circ$, $L_p = 0.8\\ \\mathrm{m}$), estimated with the smooth approximation for a DC beam:

| DC current | depressed σ | σ/σ0 |
|---|---|---|
| 0.18 mA | 61.3° | 0.998 |
| 5 mA | 57.5° | 0.936 |
| 10 mA | 53.8° | 0.876 |
| 20 mA | 47.2° | 0.769 |

(Generalised perveance $K = 2I/(I_0\\beta^3\\gamma^3)$, $I_0 = 4\\pi\\varepsilon_0 mc^3/q \\approx 31.3\\ \\mathrm{MA}$, KV-equivalent edge emittance $4\\varepsilon_{\\mathrm{rms}}$.) The lab of the [space charge](#space-charge) chapter lets you check this by dragging the parameters. The example's 0.18 mA beam has very weak space charge, but above about 10 mA the depression is no longer negligible.

### Longitudinal phase advance and RF defocusing

In an accelerating section a synchronous phase $\\varphi_s < 0$ provides longitudinal focusing. In the smooth approximation the zero-current longitudinal wave number is

$$
k_{l0}^2 = \\frac{2\\pi\\, q E_0 T \\sin(-\\varphi_s)}{m c^2\\, \\lambda\\, \\beta^3\\gamma^3}
$$

where $E_0T$ is the period-averaged effective accelerating gradient (for one cavity per period, $V_0T/L_p$). By Laplace's equation the RF field defocuses transversely by half the longitudinal focusing:

$$
k_{t0}^2 = k_{\\mathrm{foc}}^2 - \\frac{k_{l0}^2}{2}
$$

with $k_{\\mathrm{foc}}$ the focusing of the quadrupoles or solenoids. The transverse elements must therefore be stronger than without RF to reach the same $\\sigma_{0t}$.

Example: with the example HWR at $\\varphi_s = -33^\\circ$, $K_e = 1.36$ the linear preview gives an energy gain of $0.339\\ \\mathrm{MeV}$ per cavity, i.e. $V_0T \\approx \\Delta W/(q\\cos\\varphi_s) = 0.405\\ \\mathrm{MV}$. With one cavity per $L_p = 0.7\\ \\mathrm{m}$ and a mean $\\beta = 0.060$ the smooth approximation gives $\\sigma_{0l} \\approx 92^\\circ$; the RF defocusing is equivalent to a "negative" phase advance of $\\sigma_{0l}/\\sqrt{2} \\approx 65^\\circ$, so for $\\sigma_{0t} = 60^\\circ$ the solenoids must provide about $\\sqrt{60^2 + 65^2} \\approx 89^\\circ$. Lowering the cavity to $V_0T = 0.2\\ \\mathrm{MV}$ gives $\\sigma_{0l} \\approx 65^\\circ$. At the low-energy end ($\\beta \\approx 0.06$) the cavities therefore usually cannot run at full gradient, or the longitudinal phase advance exceeds $90^\\circ$.

> Note: these $\\sigma_{0l}$ values use the smooth approximation, which is already inaccurate near $90^\\circ$; treat them as order-of-magnitude estimates. In a real periodic structure check with multi-particle simulations or the "Phase advance" plot of the Results page.

### Ratios, equipartitioning and resonances

Besides each being below $90^\\circ$, the relation between longitudinal and transverse phase advances matters:

- Avoid $\\sigma_l \\approx \\sigma_t$ (with space charge, use the depressed values): when the two planes oscillate at the same frequency the space-charge coupling is strongest and a non-equipartitioned beam exchanges emittance between the planes.
- Avoid $\\sigma_l \\approx 2\\sigma_t$: the longitudinal (synchrotron) oscillation of a particle modulates the RF defocusing, a parametric drive of the transverse motion that resonates at a frequency ratio of 2.
- The **equipartitioning** condition $\\varepsilon_{nl}\\,k_l = \\varepsilon_{nt}\\,k_t$ (rms normalised emittances with a consistent normalisation, depressed wave numbers): the two planes then have the same "temperature" and there is no free energy driving emittance exchange.
- The Hofmann stability charts use the tune ratio $k_z/k_x$ and the transverse tune depression $k_x/k_{x0}$ as coordinates and show, for a given emittance ratio $\\varepsilon_z/\\varepsilon_x$, the unstable bands of the space-charge coupling resonances. A non-equipartitioned design is equally possible as long as the working point stays in a stable region or crosses narrow resonance bands quickly.

These rules matter most at high current; for the example's 0.18 mA beam space charge is weak and the main constraints are the single-particle $\\sigma_0 < 90^\\circ$ and the longitudinal acceptance.

## Cavity periods, energy gain and the synchronous-phase law {#lattice-design-longitudinal}

### Periods with cavities and focusing

Each period usually contains one or a few cavities and a set of transverse focusing elements: S–C (solenoid + cavity), S–C–C, FD–C and so on. Longer periods give less energy gain per metre but more space between cavities (for diagnostics, steerers, vacuum valves). Low-β superconducting sections often put only one cavity per period to keep the longitudinal phase advance per period down.

In the thin-gap approximation the energy gain of one cavity is

$$
\\Delta W = q V_0 T(\\beta) \\cos\\varphi_s
$$

with $T(\\beta)$ the transit-time factor. AVAS does not use this formula for field-map elements: it integrates $q\\,K_e E_z(z)\\cos(\\omega t + \\varphi_0)$ along the map; with \`V3 = 0\` the synchronous phase is defined as $\\varphi_s = \\operatorname{atan2}(\\int E\\sin\\varphi\\,\\dd z, \\int E\\cos\\varphi\\,\\dd z)$ and the kernel solves for the entrance phase (see [RF cavity](#rf-cavity)). At low β, when one cavity adds more than 20 % of the entrance energy, the particle accelerates noticeably inside the cavity and $\\Delta W/\\cos\\varphi_s$ is no longer constant: for the example cavity from $\\varphi_s = -20^\\circ$ to $-40^\\circ$ it falls from $0.437$ to $0.386\\ \\mathrm{MeV}$ (linear preview).

### The energy-gain profile

For three identical S–C periods (the lattice example below) the linear preview gives cavity exit energies of $1.867$, $2.332$ and $2.895\\ \\mathrm{MeV}$, i.e. gains of $0.340$, $0.465$ and $0.563\\ \\mathrm{MeV}$. The gain grows from cavity to cavity because the particle velocity approaches the optimum β of the HWR and the transit-time factor $T(\\beta)$ increases. In general:

- one cavity type covers a β range; $T(\\beta)$ peaks near the optimum β and the gain drops towards both ends of the range;
- plot $W(z)$ and the gain of each cavity during design to check the range in which each cavity type is used;
- independently phased superconducting cavities can each be phased for the local velocity, which makes them more flexible than multi-gap room-temperature structures.

### The synchronous-phase law

The longitudinal acceptance (the phase space inside the separatrix) grows with $\\lvert\\varphi_s\\rvert$: its phase width is about $3\\lvert\\varphi_s\\rvert$ (exactly $60.3^\\circ$ at $-20^\\circ$, $100.2^\\circ$ at $-33^\\circ$, $122.2^\\circ$ at $-40^\\circ$), and its energy half-width is

$$
\\Delta W_{\\max} = 2\\left[\\frac{q\\, m c^2 \\beta^3\\gamma^3 E_0 T \\lambda\\,(\\varphi_s\\cos\\varphi_s - \\sin\\varphi_s)}{\\pi}\\right]^{1/2}
$$

(about $\\pm 0.13\\ \\mathrm{MeV}$ for the example with $E_0T = 0.405\\ \\mathrm{MV}/0.7\\ \\mathrm{m}$, $\\beta = 0.06$, $\\varphi_s = -33^\\circ$). But a larger $\\lvert\\varphi_s\\rvert$ means a smaller $\\cos\\varphi_s$, i.e. less efficient acceleration, and a larger $\\sin(-\\varphi_s)$, i.e. a larger longitudinal phase advance. The usual practice is:

1. at the low-energy end use a larger $\\lvert\\varphi_s\\rvert$ (for example $-40^\\circ$ to $-30^\\circ$) for capture and acceptance;
2. reduce $\\lvert\\varphi_s\\rvert$ gradually as the energy rises (for example to about $-20^\\circ$) for more efficient acceleration;
3. adjust $E_0T$ and $\\varphi_s$ together so that the longitudinal phase advance **per metre**, $k_{l0}$, varies smoothly along the linac and no abrupt change mismatches the beam longitudinally.

Since $k_{l0}^2 \\propto E_0T\\sin(-\\varphi_s)/(\\beta^3\\gamma^3)$, the longitudinal focusing weakens naturally as the velocity rises, so the gradient can be increased step by step with β.

## Transitions between sections {#lattice-design-transitions}

Sections with different cavity types, period structures or frequencies need transitions. The basic principles:

- **Continuous phase advance per metre**: keep the transverse and longitudinal $k_0 = \\sigma_0/L_p$ as continuous as possible across the transition. $\\sigma_0$ may change with the period length, but the phase advance per metre should not jump; then the matched Twiss parameters of the two sections are similar and the transition needs only small corrections.
- **Changing period length**: a longer period means larger $\\beta$ functions; keeping the beam size requires a larger phase advance per metre, without exceeding $90^\\circ$ per period.
- **Frequency jumps**: doubling the frequency doubles the RF phase width of a given bunch length, so the longitudinal acceptance becomes relatively smaller; compress the bunch before the transition or start the new section with a larger $\\lvert\\varphi_s\\rvert$.
- **Matching sections**: when the periodic Twiss parameters of the two sections differ a lot, insert a dedicated matching section (quadrupoles, solenoids or buncher cavities), see [matching](#matching-sections).

Mismatch at transitions is one of the main sources of emittance growth and halo. After a design, plot the rms envelope and the "Phase advance" over the whole line on the Results page; in the new section the envelope should quickly become periodic again.

## Aperture and beam size {#lattice-design-aperture}

The ratio of the aperture radius $R$ to the rms beam size directly measures the loss margin. There is no universal standard; the following values are common rules of thumb in low-loss high-intensity linac design, for orientation only:

- design (error-free) beam: $R/x_{\\mathrm{rms}} \\gtrsim 6$–$10$;
- worst case with errors: the maximum beam size (including halo) must not touch the aperture;
- high-power machines require extremely low losses per metre and more conservative ratios.

In AVAS the second parameter \`R\` (m) of every element is the aperture radius the kernel uses to detect particle losses (with \`boundary 1\` in \`input.txt\` it uses \`boundary.txt\` instead). The linear preview warns "expect losses" when $3\\times$ the rms size exceeds the aperture.

> Note: the aperture ratio is based on the rms size, whereas losses are decided by the tails of the distribution. A truncated Gaussian, space-charge halo and error-induced centroid offsets all make the real envelope much larger than the rms estimate; confirm with multi-particle simulations (maximum sizes in DataSet columns 22/24, surviving macro-particles in column 28) and error studies (see [errors and correction](#errors)).

## Writing it as an AVAS lattice file {#lattice-design-avas}

### File skeleton

- Only the lines between the first \`start\` and the first \`end\` are simulated; \`!\` starts a comment.
- Use comments as section headings: comments such as \`! ---- MEBT ----\` or \`!;;; SC1 ;;;\` appear as group headings in the structure tree.
- \`section NAME {\` … \`}\` only folds lines in the GUI and is not simulated.
- Elements can be named: \`Q1 : quad 0.1 0.02 0 5\`; the name is used by the Lattice page, parameter scans (\`avas scan --target Q1\`) and the AI assistant to find elements. For a run AVAS writes the kernel's \`lattice.txt\` in the output's \`inputs/\` folder with only the commands the kernel knows; the project's input files are not rewritten.
- The lattice that runs is the file named in \`[lattice] source\` of \`ini.ini\` (the "Set as run lattice" button of the Lattice page).

### Period markers

\`lattice n1 n2\` and \`lattice_end\` enclose a periodic structure; \`n1\` is the number of elements per basic period and \`n2\` is written as 1:

\`\`\`avas-params
lattice lattice_end
\`\`\`

The number of elements between the two commands (drifts, field maps and matrix elements count, diagnostics do not) must be an integer multiple of \`n1\`, otherwise an error is reported. The "Phase advance" plot of the Results page uses these periods and the beam Twiss β from DataSet to compute $\\int \\dd z/\\beta$ per period (x, y and z), shown per period or per metre. Because it uses the Twiss parameters of the actual beam, it reflects the phase advance with space charge and RF effects, and it equals the phase advance of the periodic structure itself only for a matched beam.

### A three-period example

Three S–C periods with the example field maps (4 elements per period: drift, solenoid map, drift, HWR map), period length $0.66\\ \\mathrm{m}$:

\`\`\`text
start
! ---- SC section: solenoid + HWR ----
lattice 4 1
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
lattice_end
drift  0.1  0.02  0
outputplane -0.05
end
\`\`\`

With the example beam the linear preview gives a final energy of about $2.89\\ \\mathrm{MeV}$ and a maximum rms size of about $1.6\\ \\mathrm{mm}$ (aperture 20 mm, ratio about 12). The envelope is very sensitive to the solenoid factor \`Kb\`: with \`Kb = 0.45\` the maximum rms is about $2.7\\ \\mathrm{mm}$, and with \`Kb = 0.531\` the beam is already overfocused and the preview reports that 3 × rms exceeds the aperture. The entrance beam is not the matched beam of this period, so the envelope is not strictly periodic — exactly the problem the [matching](#matching) chapter solves.

### Further points

- **Superpose**: when a solenoid and a cavity are so close that their fringe fields overlap, put them in one \`superpose\` block so both fields act in the overlap region; a block may contain at most one RF cavity (see [superpose](#superpose)).
- **Steerers and diagnostics**: reserve places in the period for \`steerer\` (length 0, acting over the next element) and \`diag_position\` / \`diag_size\` / \`diag_energy\`, used by the error correction (see [errors and correction](#errors)).
- **No matrix elements at the ends**: in multi-particle runs avoid matrix elements such as \`quad\` or \`solenoid\` as the first or last element; add a short \`drift\`.
- **Output planes**: \`outputplane V1\` dumps the particle distribution V1 metres downstream or upstream of its position; it must not lie past the lattice end, 5 mm before the end fails, 2 cm or more works (see [simulation settings and output](#simulation)).
- **Linear preview first**: the visual editor shows the linear envelope live while you drag parameters (energy to about 0.01 %, rms usually within 1–2 %); once it looks reasonable, run the kernel for the multi-particle simulation.

## Hands-on {#lattice-design-lab}

In the FODO lab change \`G\` and watch the phase advance per period and the stability. With the thick matrices the phase advance passes $90^\\circ$ at \`G\` ≈ $6.9\\ \\mathrm{T/m}$; at about $9.8\\ \\mathrm{T/m}$, $\\lvert\\Tr M\\rvert/2$ reaches 1 (phase advance $180^\\circ$), the periodic β functions diverge, and beyond that no periodic solution exists. Compare the thick-matrix phase advance with the thin-lens estimate as well.

\`\`\`avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
\`\`\`

In the space-charge lab increase the DC current \`I\` and watch the tune depression $\\sigma/\\sigma_0$ and the envelope; compare the same current with \`G = 5\` ($\\sigma_0 \\approx 61^\\circ$) and \`G = 6.6\` ($\\sigma_0 \\approx 85^\\circ$).

\`\`\`avas-lab
space-charge I=10 W=1.527 mass=938.272 q=1 emit=0.1377 G=5 Lq=0.1 Ld=0.3 cells=10
\`\`\`

In the RF cavity lab change the synchronous phase and watch the energy gain and the longitudinal rms size:

\`\`\`avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
\`\`\`

The complete step-by-step construction is in the [hands-on tutorial](#tutorial).

## Further reading {#lattice-design-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: the chapters on longitudinal and transverse dynamics, space charge and linac design; this chapter follows its notation.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.: periodic focusing channels, envelope equations, equipartitioning.
- I. Hofmann's papers on the stability of anisotropic beams: ratios and resonance charts.
- The full bibliography is in [references](#references).
`,h=`# 如何组合成一台加速器

前面各章分别讲了单个元件。本章讨论如何把漂移段、四极铁、螺线管、射频腔等元件组合成一条可以稳定传输并加速束流的直线加速器 lattice：先选聚焦周期和每周期相移，再安排腔体与同步相位规律，处理段与段之间的过渡，检查孔径裕量，最后写成 AVAS 的 lattice 文件。本章给出设计规则及其物理来源；[动手教程](#tutorial)是对应的实操版本，用示例 HWR 腔和螺线管场图一步步搭出一小段超导直线加速器。

本章假设读者已熟悉 [线性光学](#linear-optics)、[纵向动力学](#longitudinal) 与 [空间电荷](#space-charge) 三章的记号：横向坐标 $(x, x', y, y')$，纵向 $(z, \\delta = \\Delta p/p)$，周期长度 $L_p$，每周期相移 $\\sigma$（单位度），零流相移 $\\sigma_0$。

## 直线加速器的功能分段 {#lattice-design-overview}

一台强流质子 / 重离子直线加速器通常由以下几部分组成（按束流方向）：

| 段 | 作用 | 典型元件 | AVAS 是否模拟 |
|---|---|---|---|
| 离子源 | 产生直流束 | 引出电极 | 否 |
| LEBT 低能传输段 | 把直流束匹配进 RFQ | 螺线管、斩波器 | 否（通常） |
| RFQ | 聚束、聚焦、加速到 MeV 量级 | 四翼 / 四杆电极 | 否 |
| MEBT 中能传输段 | 横向与纵向匹配、斩束、诊断 | 四极铁、聚束腔 | 是 |
| 加速段 | 主要能量增益 | 超导 HWR / spoke / 椭球腔 + 螺线管或四极铁 | 是（腔体用场图） |
| HEBT 高能传输段 | 输运到靶或下游 | 四极铁、二极铁 | 是 |

AVAS 从 \`beam.txt\` 给出的初始束（或 \`.dst\` 粒子文件，例如示例中的 \`part_rfq.dst\`）开始跟踪，因此它描述的是**给定入口束流之后**的 lattice。离子源、RFQ 和 DTL 的加速结构不是 AVAS 可以直接建模的元件；如果有这些结构的三维场图，原则上可以作为 \`field\` 元件读入，但本章不讨论。

示例项目 \`examples/hwr010\` 对应超导段的起点：质子，$W = 1.527\\ \\mathrm{MeV}$，$f = 162.5\\ \\mathrm{MHz}$，$I = 0.18\\ \\mathrm{mA}$。对这一束流：

$$
\\gamma = 1.00163,\\quad \\beta = 0.05698,\\quad \\beta\\gamma = 0.05708,\\quad B\\rho = 0.1786\\ \\mathrm{T\\,m},\\quad \\beta\\lambda = 0.1051\\ \\mathrm{m}
$$

$\\beta\\lambda$ 是一个射频周期内粒子走过的距离，它决定了腔体间隙和周期长度的尺度；$B\\rho$ 决定同样梯度下四极铁和螺线管的聚焦强度（见 [运动学](#kinematics)）。

## 聚焦周期 {#lattice-design-periods}

直线加速器的横向聚焦几乎总是周期性的：同一个"周期（period / cell）"重复许多次。周期内的传输矩阵 $M$ 满足稳定条件

$$
\\left\\lvert \\tfrac{1}{2}\\Tr M \\right\\rvert < 1,\\qquad \\cos\\sigma = \\tfrac{1}{2}\\Tr M
$$

此时存在唯一的周期 Twiss 参数 $(\\alpha, \\beta)$（匹配束，见 [匹配](#matching)），束流包络与周期同步振荡。常见周期如下。

### FODO

聚焦（F）与散焦（D）四极铁交替，中间为漂移或腔体。以焦距 $\\pm f$ 的薄透镜、透镜间距 $L_p/2$ 计：

$$
\\sin\\frac{\\sigma}{2} = \\frac{L_p}{4f},\\qquad
\\beta_{\\max,\\min} = \\frac{L_p\\,(1 \\pm \\sin\\frac{\\sigma}{2})}{\\sin\\sigma}
$$

稳定要求 $f > L_p/4$。FODO 的优点是两个平面对称、每周期元件少；缺点是 $\\beta_{\\max}/\\beta_{\\min}$ 随相移增大而增大，束流在 F 铁处最大。

例：示例束流，四极铁 $L_q = 0.1\\ \\mathrm{m}$、$G = 5\\ \\mathrm{T/m}$，漂移 $L_d = 0.3\\ \\mathrm{m}$（周期 $L_p = 0.8\\ \\mathrm{m}$）。$k = G/B\\rho = 27.99\\ \\mathrm{m^{-2}}$，薄透镜焦距 $f = 1/(kL_q) = 0.357\\ \\mathrm{m}$。薄透镜公式给出 $\\sigma = 68.1^\\circ$；用厚透镜矩阵算出的精确值为 $\\sigma = 61.4^\\circ$，F 铁中心处周期 $\\beta_x = 1.35\\ \\mathrm{m}$，D 铁中心处 $\\beta_x = 0.45\\ \\mathrm{m}$。薄透镜近似在 $kL_q^2$ 不小时（这里 $kL_q^2 = 0.28$）会高估相移，设计时应以厚矩阵为准。

### 双透镜（doublet）与三透镜（triplet）

把 F、D 两块四极铁靠在一起（FD + 长漂移），中间留出放腔体的长空间。双透镜周期的长漂移段利用率高，但两个平面的 $\\beta$ 函数不对称；三透镜（FDF 或 DFD）可以恢复对称。它们的稳定区与 FODO 不同，必须用实际矩阵计算 $\\Tr M$。

### 螺线管周期

低能超导段常用螺线管 + 腔体的周期（S–C、S–C–C 等）。螺线管同时聚焦两个平面，在 Larmor 旋转坐标系中薄透镜焦距为

$$
\\frac{1}{f} = \\int k^2\\,\\dd s,\\qquad k = \\frac{B_z}{2B\\rho}
$$

（见 [螺线管](#solenoid)）。单个薄螺线管 + 漂移 $L_p$ 的周期满足 $\\cos\\sigma = 1 - L_p/(2f)$。螺线管的聚焦强度与 $1/(B\\rho)^2$ 成正比，所以在低能（$B\\rho$ 小）时比四极铁更"划算"，到高能时四极铁更有效；这也是低能超导段多用螺线管、高能段改用四极铁的原因之一。

> 注意：螺线管使束流绕轴旋转 $kL$。圆对称束旋转后不变，但非圆束或有横向误差的束流会出现 x–y 耦合；AVAS 的 2D/3D 视图和 DataSet 给出的是实验室坐标系中的量。

## 每周期相移的选择 {#lattice-design-phase-advance}

### 横向零流相移 σ0t < 90°

最重要的一条设计规则是：**每个聚焦周期的零流横向相移 $\\sigma_{0t}$ 小于 $90^\\circ$**。原因来自包络的参数共振。圆束在均匀聚焦通道中的包络小振荡有两个本征模（见 [匹配](#matching-envelope-modes)）：

$$
k_{\\mathrm{breathing}} = \\sqrt{2k_0^2 + 2k^2},\\qquad k_{\\mathrm{quadrupole}} = \\sqrt{k_0^2 + 3k^2}
$$

其中 $k_0$、$k$ 分别是零流和有空间电荷时的横向波数。零流时两者都等于 $2k_0$，即包络振荡相移是粒子相移的两倍。周期聚焦以每周期 $360^\\circ$ 的频率调制包络，当包络振荡每周期 $180^\\circ$（半整数参数共振）时发生**包络不稳定性**，对应 $\\sigma_0 = 90^\\circ$。空间电荷使包络模频率降低，所以不稳定带出现在 $\\sigma_0 > 90^\\circ$、且流强足够大（$\\sigma$ 被压低）的一侧；$\\sigma_0 < 90^\\circ$ 时包络模每周期相移低于 $180^\\circ$，不会进入这一不稳定带。为了给误差和流强留出裕量，强流设计一般取 $\\sigma_{0t} \\approx 50^\\circ$–$80^\\circ$。此外，$\\sigma_0 > 90^\\circ$ 而空间电荷把相移压到 $90^\\circ$ 附近时，还会激发四阶共振（$4\\sigma \\approx 360^\\circ$），引起发射度增长。

### 相移太小同样不好

相移过小意味着聚焦弱、$\\beta$ 函数大、束流尺寸大，空间电荷相对更强（流强降低因子 $\\sigma/\\sigma_0$ 更小），需要更大孔径。以示例的 FODO（$\\sigma_0 = 61.4^\\circ$、$L_p = 0.8\\ \\mathrm{m}$）为例，按平滑近似和直流束估算：

| 直流流强 | 有空间电荷的 σ | σ/σ0 |
|---|---|---|
| 0.18 mA | 61.3° | 0.998 |
| 5 mA | 57.5° | 0.936 |
| 10 mA | 53.8° | 0.876 |
| 20 mA | 47.2° | 0.769 |

（广义导流系数 $K = 2I/(I_0\\beta^3\\gamma^3)$，$I_0 = 4\\pi\\varepsilon_0 mc^3/q \\approx 31.3\\ \\mathrm{MA}$，KV 等效边缘发射度 $4\\varepsilon_{\\mathrm{rms}}$。）可以在 [空间电荷](#space-charge) 一章的实验里拖动参数验证。示例的 0.18 mA 束流空间电荷很弱，但 10 mA 以上时相移降低已不可忽略。

### 纵向相移与射频散焦

在加速段，同步相位 $\\varphi_s < 0$ 提供纵向聚焦。平滑近似下纵向零流波数为

$$
k_{l0}^2 = \\frac{2\\pi\\, q E_0 T \\sin(-\\varphi_s)}{m c^2\\, \\lambda\\, \\beta^3\\gamma^3}
$$

其中 $E_0T$ 是周期平均的有效加速梯度（单腔周期可取 $V_0T/L_p$）。由 Laplace 方程，射频场在横向产生大小为纵向一半的散焦：

$$
k_{t0}^2 = k_{\\mathrm{foc}}^2 - \\frac{k_{l0}^2}{2}
$$

$k_{\\mathrm{foc}}$ 是四极铁或螺线管提供的横向聚焦。所以横向元件必须比无射频时更强，才能得到同样的 $\\sigma_{0t}$。

例：示例 HWR 腔在 $\\varphi_s = -33^\\circ$、$K_e = 1.36$ 时，线性预览给出单腔能量增益 $0.339\\ \\mathrm{MeV}$，相应 $V_0T \\approx \\Delta W/(q\\cos\\varphi_s) = 0.405\\ \\mathrm{MV}$。若每 $L_p = 0.7\\ \\mathrm{m}$ 一个腔、取平均 $\\beta = 0.060$，平滑近似给出 $\\sigma_{0l} \\approx 92^\\circ$；射频散焦相当于 $\\sigma_{0l}/\\sqrt{2} \\approx 65^\\circ$ 的"负相移"，要得到 $\\sigma_{0t} = 60^\\circ$，螺线管需提供约 $\\sqrt{60^2 + 65^2} \\approx 89^\\circ$。把腔压降到 $V_0T = 0.2\\ \\mathrm{MV}$ 时 $\\sigma_{0l} \\approx 65^\\circ$。这说明在 $\\beta \\approx 0.06$ 的低能端，腔体通常不能满梯度运行，否则纵向相移超过 $90^\\circ$。

> 注意：上述 $\\sigma_{0l}$ 用的是平滑近似，相移接近 $90^\\circ$ 时误差已经较大，只能作量级估计；周期结构中的精确值要用多粒子模拟或 AVAS 结果页的"相移"图检查。

### 纵横比、能量均分与共振

除了各自小于 $90^\\circ$，纵向和横向相移之间的关系也很重要：

- 避免 $\\sigma_l \\approx \\sigma_t$ 附近（有空间电荷时用压低后的相移）：两平面振荡频率相同时空间电荷耦合最强，非均分束会在两平面间交换发射度。
- 避免 $\\sigma_l \\approx 2\\sigma_t$：粒子的纵向（同步）振荡调制射频散焦，构成横向的参数激励，频率比为 2 时共振。
- **能量均分**（equipartitioning）条件 $\\varepsilon_{nl}\\,k_l = \\varepsilon_{nt}\\,k_t$（rms 归一化发射度与压低后的波数，发射度用一致的归一化定义）：满足时两平面"温度"相同，不存在驱动发射度交换的自由能。
- Hofmann 稳定性图以纵横波数比 $k_z/k_x$ 和横向降低因子 $k_x/k_{x0}$ 为坐标，对给定的发射度比 $\\varepsilon_z/\\varepsilon_x$ 画出空间电荷耦合共振的不稳定带。非均分设计只要工作点在稳定区、或快速穿过窄的共振带，同样可行。

这些规则在高流强时最重要；对示例的 0.18 mA 束，空间电荷弱，主要约束是单粒子的 $\\sigma_0 < 90^\\circ$ 和纵向接受度。

## 腔体周期、能量增益与同步相位规律 {#lattice-design-longitudinal}

### 腔体加聚焦的周期

每个周期通常包含一个或几个腔体与一组横向聚焦元件，例如 S–C（螺线管 + 腔）、S–C–C、FD–C 等。周期越长，每米能量增益越低，但腔间空间越多（可放诊断、校正铁、真空阀）。超导低 β 段为降低每周期纵向相移，常常每周期只放一个腔。

单腔能量增益在薄间隙近似下为

$$
\\Delta W = q V_0 T(\\beta) \\cos\\varphi_s
$$

$T(\\beta)$ 是渡越时间因子。对场图元件，AVAS 不用这一公式，而是沿场图积分 $q\\,K_e E_z(z)\\cos(\\omega t + \\varphi_0)$；\`V3 = 0\` 时同步相位定义为 $\\varphi_s = \\operatorname{atan2}(\\int E\\sin\\varphi\\,\\dd z, \\int E\\cos\\varphi\\,\\dd z)$，内核反解入口相位（见 [射频腔](#rf-cavity)）。在低 β、单腔增能占入口能量 20 % 以上时，粒子在腔内显著加速，$\\Delta W/\\cos\\varphi_s$ 不再是常数：示例腔从 $\\varphi_s = -20^\\circ$ 到 $-40^\\circ$，$\\Delta W/\\cos\\varphi_s$ 从 $0.437$ 降到 $0.386\\ \\mathrm{MeV}$（线性预览）。

### 能量增益曲线

在三个相同的 S–C 周期中（下文的 lattice 例子），线性预览给出各腔出口能量 $1.867$、$2.332$、$2.895\\ \\mathrm{MeV}$，单腔增益依次为 $0.340$、$0.465$、$0.563\\ \\mathrm{MeV}$。增益逐腔增大，是因为粒子速度逐渐接近 HWR 的最佳 β，渡越时间因子 $T(\\beta)$ 增大。一般来说：

- 同一种腔覆盖一个 β 区间，$T(\\beta)$ 在最佳 β 附近最大，区间两端增益下降；
- 设计时要画出 $W(z)$ 和每腔 $\\Delta W$，检查每种腔的使用区间；
- 独立相控的超导腔可以各自调相位，适应不同速度，这是它们比多间隙常温结构灵活的地方。

### 同步相位规律

纵向接受度（分界线内的相空间）随 $\\lvert\\varphi_s\\rvert$ 增大：相位宽度约为 $3\\lvert\\varphi_s\\rvert$（精确值：$-20^\\circ$ 时 $60.3^\\circ$，$-33^\\circ$ 时 $100.2^\\circ$，$-40^\\circ$ 时 $122.2^\\circ$），能量半宽为

$$
\\Delta W_{\\max} = 2\\left[\\frac{q\\, m c^2 \\beta^3\\gamma^3 E_0 T \\lambda\\,(\\varphi_s\\cos\\varphi_s - \\sin\\varphi_s)}{\\pi}\\right]^{1/2}
$$

（对示例，$E_0T = 0.405\\ \\mathrm{MV}/0.7\\ \\mathrm{m}$、$\\beta = 0.06$、$\\varphi_s = -33^\\circ$ 时约 $\\pm 0.13\\ \\mathrm{MeV}$）。但 $\\lvert\\varphi_s\\rvert$ 越大，$\\cos\\varphi_s$ 越小，加速效率越低，且 $\\sin(-\\varphi_s)$ 增大使纵向相移增大。常见做法是：

1. 低能端用较大的 $\\lvert\\varphi_s\\rvert$（如 $-40^\\circ$ 到 $-30^\\circ$），保证俘获和接受度；
2. 随能量升高逐渐减小 $\\lvert\\varphi_s\\rvert$（如到 $-20^\\circ$ 左右），提高加速效率；
3. 同时调节 $E_0T$ 与 $\\varphi_s$，使**每米**纵向相移 $k_{l0}$ 沿加速器平滑变化，避免突变造成纵向失配。

$k_{l0}^2 \\propto E_0T\\sin(-\\varphi_s)/(\\beta^3\\gamma^3)$：速度升高时纵向聚焦自然变弱，所以可以随 β 增大逐步提高梯度。

## 段与段之间的过渡 {#lattice-design-transitions}

不同腔型、不同周期结构或不同频率的段之间需要过渡。基本原则：

- **每米相移连续**：横向与纵向的 $k_0 = \\sigma_0/L_p$ 在过渡处尽量连续。$\\sigma_0$ 可以随周期长度变化，但每米相移不要跳变，这样两段的匹配 Twiss 参数相近，过渡段只需小修正。
- **周期长度变化**：周期变长时 $\\beta$ 函数随之变大；若要保持束流尺寸，需同时提高每米相移，但不能突破每周期 $90^\\circ$。
- **频率跳变**：频率加倍时，同一束团长度对应的射频相位宽度加倍，纵向接受度相对变小，过渡处需要预先压缩束团或在新段开始时使用较大 $\\lvert\\varphi_s\\rvert$。
- **匹配段**：两段周期 Twiss 参数差别较大时，在中间插入专门的匹配段（四极铁、螺线管或聚束腔），见 [匹配](#matching-sections)。

过渡处的失配是发射度增长和晕的主要来源之一。设计后应在结果页对整条线画 rms 包络和"相移"图，包络在新段中应迅速重新呈周期形状。

## 孔径与束流尺寸 {#lattice-design-aperture}

孔径半径 $R$ 与 rms 束流尺寸之比是"损失裕量"的直接量度。没有统一标准，下列数值是低损失强流直线加速器设计中常见的经验取值，仅供参考：

- 设计（无误差）束流：$R/x_{\\mathrm{rms}} \\gtrsim 6$–$10$；
- 考虑误差后的最坏情况：保证束流最大尺寸（含晕）不碰到孔径；
- 高功率机器要求每米损失极低，孔径比更保守。

AVAS 中每个元件的第二个参数 \`R\`（m）就是孔径半径，内核据此判断粒子损失（\`input.txt\` 设置 \`boundary 1\` 时改用 \`boundary.txt\`）。线性预览在 $3\\times$ rms 尺寸超过孔径时给出警告"可能有束损"。

> 注意：孔径比只基于 rms 尺寸，而损失由分布尾部决定。高斯分布截断、空间电荷引起的晕和误差引起的中心偏移都会让实际包络比 rms 估计大得多；最终必须用多粒子模拟（看 DataSet 第 22/24 列的最大尺寸和第 28 列存活粒子数）和误差研究（见 [误差与校正](#errors)）来确认。

## 写成 AVAS lattice 文件 {#lattice-design-avas}

### 文件骨架

- 只有第一个 \`start\` 到第一个 \`end\` 之间的行会被模拟；\`!\` 开始注释。
- 用注释做段标题：形如 \`! ---- MEBT ----\`、\`!;;; SC1 ;;;\` 的注释在结构树中显示为分组标题。
- \`section NAME {\` … \`}\` 只用于界面折叠，不进入计算。
- 元件可以命名：\`Q1 : quad 0.1 0.02 0 5\`，名字用于结构页、参数扫描（\`avas scan --target Q1\`）和 AI 助手定位元件。运行时 AVAS 在输出目录的 \`inputs/\` 中生成内核读的 \`lattice.txt\`，只写内核认识的命令，不改写项目的输入文件。
- 运行使用的 lattice 是 \`ini.ini\` 中 \`[lattice] source\` 指定的文件（结构页"设为运行结构"按钮）。

### 周期标记

\`lattice n1 n2\` 与 \`lattice_end\` 之间是周期结构，\`n1\` 是每个基础周期的元件数，\`n2\` 写 1：

\`\`\`avas-params
lattice lattice_end
\`\`\`

两条命令之间的元件数（漂移段、场图元件和矩阵元件都计数，束诊命令不计）必须是 \`n1\` 的整数倍，否则报错。结果页的"相移"图按这些周期，用 DataSet 中束流的 Twiss β 计算每周期 $\\int \\dd z/\\beta$（x、y、z 三个平面），可选按周期或按米显示。由于用的是实际束流的 Twiss 参数，它反映的是有空间电荷和射频效应后的相移，而且只有在束流匹配时才等于周期结构本身的相移。

### 一个三周期的例子

下面用示例场图写三个 S–C 周期（每周期 4 个元件：漂移、螺线管场图、漂移、HWR 场图），周期长度 $0.66\\ \\mathrm{m}$：

\`\`\`text
start
! ---- SC section: solenoid + HWR ----
lattice 4 1
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
lattice_end
drift  0.1  0.02  0
outputplane -0.05
end
\`\`\`

用示例束流做线性预览：末端能量约 $2.89\\ \\mathrm{MeV}$，rms 尺寸最大约 $1.6\\ \\mathrm{mm}$（孔径 20 mm，孔径比约 12）。螺线管系数 \`Kb\` 对包络非常敏感：\`Kb = 0.45\` 时最大 rms 约 $2.7\\ \\mathrm{mm}$，\`Kb = 0.531\` 时已经过聚焦，预览报告 3 倍 rms 超过孔径。入口束流并不是这个周期的匹配束，所以包络并不严格周期——这正是 [匹配](#matching) 一章要解决的问题。

### 其他要点

- **叠加场**：螺线管与腔体靠得很近、边缘场重叠时，用 \`superpose\` 把它们放进同一个叠加块，使重叠区两个场同时作用；一个叠加块最多一个射频腔（见 [叠加场](#superpose)）。
- **校正铁与束诊**：在周期中预留 \`steerer\`（长度 0，作用于下一个元件）和 \`diag_position\` / \`diag_size\` / \`diag_energy\` 的位置，误差校正时使用（见 [误差与校正](#errors)）。
- **矩阵元件不要放在首尾**：多粒子模拟中第一个和最后一个元件避免用 \`quad\`、\`solenoid\` 等矩阵元件，可以加一小段 \`drift\`。
- **输出平面**：\`outputplane V1\` 在当前位置上下游 V1 米处输出粒子分布；不能超出 lattice 末端，离末端 5 mm 会失败，留 2 cm 以上即可（见 [模拟设置与输出](#simulation)）。
- **设置先用线性预览**：可视化编辑器中拖动参数时实时显示线性包络（能量约 0.01 %、rms 通常 1–2 % 的精度），确认大致合理后再用内核做多粒子模拟。

## 动手实验 {#lattice-design-lab}

在 FODO 实验中改变 \`G\` 观察每周期相移和稳定性。按厚矩阵计算，\`G\` 约 $6.9\\ \\mathrm{T/m}$ 时相移越过 $90^\\circ$；约 $9.8\\ \\mathrm{T/m}$ 时 $\\lvert\\Tr M\\rvert/2$ 达到 1（相移 $180^\\circ$），周期 β 函数发散，再增大则周期解不存在。同时对比厚矩阵相移与薄透镜估计的差别。

\`\`\`avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
\`\`\`

在空间电荷实验中增大直流流强 \`I\`，观察相移降低因子 $\\sigma/\\sigma_0$ 和包络的变化；对比 \`G = 5\`（$\\sigma_0 \\approx 61^\\circ$）与 \`G = 6.6\`（$\\sigma_0 \\approx 85^\\circ$）两种聚焦下同一流强的效果。

\`\`\`avas-lab
space-charge I=10 W=1.527 mass=938.272 q=1 emit=0.1377 G=5 Lq=0.1 Ld=0.3 cells=10
\`\`\`

在射频腔实验中改变同步相位，观察能量增益和纵向 rms 尺寸随相位的变化：

\`\`\`avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
\`\`\`

完整的分步搭建过程见 [动手教程](#tutorial)。

## 延伸阅读 {#lattice-design-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.：纵向与横向动力学、空间电荷和直线加速器设计各章，本章的公式记号与之一致。
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.：周期聚焦通道、包络方程、能量均分。
- I. Hofmann 关于非各向同性束稳定性的论文：纵横比与共振图。
- 完整书目见 [参考文献](#references)。
`,g=`# Linear optics

When the external forces are proportional to the deviations, the equations of motion are linear: any section of beamline can be described by a transfer matrix, and the second moments and Twiss parameters of the beam are transported by simple rules. Linear optics is the backbone of beamline design, matching and stability analysis, and it is the model behind AVAS's linear envelope preview and the labs of this primer. Starting from Hill's equation, this chapter gives the transfer matrices of a drift, a thin lens and a thick quadrupole, explains symplecticity, the transport of Twiss parameters, the betatron phase advance, the stability condition of periodic systems and the matched beam, and finally presents the 6×6 form used by AVAS's linear preview.

Notation as in [Phase space](#phase-space): $\\beta$, $\\gamma$ are relativistic factors, $\\beta_x$, $\\alpha_x$, $\\gamma_x$ are Twiss parameters.

## Equation of motion and Hill's equation {#linear-optics-hill}

### Paraxial equation of motion

Let the particle move along $s$ with transverse momentum $p_x \\approx p\\,x'$. From $\\dd p_x/\\dd t = F_x$ and $\\dd/\\dd t = v\\,\\dd/\\dd s$,

$$
x'' + \\frac{p'}{p}\\,x' = \\frac{F_x}{p\\,v}
$$

where $p'/p = (\\beta\\gamma)'/(\\beta\\gamma)$ is non-zero only in accelerating sections; it is the [adiabatic damping](#phase-space-liouville) term. For a magnetic field $F_x = -q v B_y$ and for an electric field $F_x = qE_x$, so the right-hand side is $-B_y/(B\\rho)$ and $E_x/(E\\rho)$ respectively ([rigidity](#kinematics-rigidity)).

### Hill's equation

If the transverse fields are linear in the deviations, for example in a quadrupole $B_y = G\\,x$, $B_x = G\\,y$, the equations of motion in a non-accelerating section become

$$
x'' + K_x(s)\\,x = 0, \\qquad y'' + K_y(s)\\,y = 0
$$

This is Hill's equation. The focusing functions of the elements:

| Element | $K_x$ | $K_y$ | See |
|---|---|---|---|
| drift | 0 | 0 | [Drift](#drift) |
| quadrupole (gradient $G$) | $k = G/(B\\rho)$ | $-k$ | [Quadrupole](#quadrupole) |
| sector dipole (field index $N$, bending in $x$) | $(1-N)/\\rho^2$ | $N/\\rho^2$ | [Dipole](#dipole) |
| solenoid (in the Larmor rotating frame) | $k_s^2$, $k_s = B/(2B\\rho)$ | $k_s^2$ | [Solenoid](#solenoid) |

For a positive particle a quadrupole with $G > 0$ focuses in $x$ and defocuses in $y$: at $x > 0$, $B_y > 0$ and $\\boldsymbol v \\times \\boldsymbol B$ points towards $-x$. A quadrupole cannot focus in both planes at once (in a source-free region $\\nabla\\times\\boldsymbol B = 0$ requires $\\partial B_y/\\partial x = \\partial B_x/\\partial y$, hence $K_y = -K_x$), so quadrupoles of alternating sign are needed, see [Periodic systems](#linear-optics-periodic).

## Transfer matrices {#linear-optics-matrices}

The solution of a linear equation is a linear combination of the initial values, $X(s) = M(s \\mid s_0)\\,X(s_0)$ with $X = (x, x')\\tp$. If $C(s)$ and $S(s)$ are the two fundamental (cosine-like and sine-like) solutions with $C(s_0) = 1, C'(s_0) = 0$ and $S(s_0) = 0, S'(s_0) = 1$, then

$$
M = \\begin{pmatrix} C & S \\\\ C' & S' \\end{pmatrix}
$$

The matrix of a beamline section is the product of the element matrices in order, **with the element passed last on the left**: $M = M_n \\cdots M_2 M_1$.

### Drift (length $L$)

$$
M_{\\mathrm{drift}} = \\begin{pmatrix} 1 & L \\\\ 0 & 1 \\end{pmatrix}
$$

### Thin lens (focal length $f$)

$$
M_{\\mathrm{thin}} = \\begin{pmatrix} 1 & 0 \\\\ -1/f & 1 \\end{pmatrix}
$$

$f > 0$ focuses, $f < 0$ defocuses.

### Thick quadrupole (length $L$, $k > 0$, $\\phi = \\sqrt{k}\\,L$)

The focusing and the defocusing plane are

$$
M_{\\mathrm F} = \\begin{pmatrix} \\cos\\phi & \\dfrac{\\sin\\phi}{\\sqrt{k}} \\\\ -\\sqrt{k}\\sin\\phi & \\cos\\phi \\end{pmatrix}, \\qquad
M_{\\mathrm D} = \\begin{pmatrix} \\cosh\\phi & \\dfrac{\\sinh\\phi}{\\sqrt{k}} \\\\ \\sqrt{k}\\sinh\\phi & \\cosh\\phi \\end{pmatrix}
$$

The equivalent focal length of a thick lens is given by $1/f = -M_{21}$, i.e. $1/f = \\sqrt{k}\\sin\\phi$ in the focusing plane; for $\\phi \\ll 1$ it reduces to the thin lens $1/f \\approx kL$, the thin-lens approximation.

Example: 1.527 MeV protons ($B\\rho$ = 0.17863 T·m), a quadrupole with $G$ = 20 T/m and $L$ = 0.05 m, $k$ = 111.96 m$^{-2}$, $\\phi$ = 0.529:

$$
M_{\\mathrm F} = \\begin{pmatrix} 0.86328 & 0.04770\\ \\mathrm m \\\\ -5.34063\\ \\mathrm m^{-1} & 0.86328 \\end{pmatrix}, \\qquad
M_{\\mathrm D} = \\begin{pmatrix} 1.14325 & 0.05237\\ \\mathrm m \\\\ 5.86300\\ \\mathrm m^{-1} & 1.14325 \\end{pmatrix}
$$

The equivalent focal length is 0.1872 m in the focusing plane and $-0.1706$ m in the defocusing plane; the thin-lens approximation gives $\\pm 0.1786$ m. At $\\phi \\approx 0.5$ the thin-lens error is already about 5 %, which is common in strongly focusing low-energy sections, so AVAS's matrix elements and the linear preview use thick-lens matrices.

## Symplecticity and the determinant {#linear-optics-symplectic}

The linear transfer matrix of a Hamiltonian system is **symplectic**:

$$
M\\tp J\\,M = J, \\qquad J = \\begin{pmatrix} 0 & 1 \\\\ -1 & 0 \\end{pmatrix}
$$

(in several dimensions $J$ is block-diagonal with one 2×2 block per plane). For a 2×2 matrix the symplectic condition is equivalent to $\\det M = CS' - SC' = 1$. All matrices above satisfy it, which is a handy check of hand calculations or program output. Symplecticity guarantees conservation of phase-space area — [Liouville's theorem](#phase-space-liouville) in the linear case — and gives the inverse matrix a simple form:

$$
M^{-1} = \\begin{pmatrix} S' & -S \\\\ -C' & C \\end{pmatrix}
$$

**With acceleration**, $\\det M = (\\beta\\gamma)_0/(\\beta\\gamma)_1 < 1$ in $(x, x')$ coordinates — this is adiabatic damping; in $(x, \\beta\\gamma\\,x')$ coordinates the matrix is symplectic again.

> Note: the 6×6 matrices of drift, quad, solenoid, bend and edge in the linear preview are all symplectic (checked numerically with the three conjugate pairs $(x, x')$, $(y, y')$, $(z, \\delta)$); only the accelerating slices of field maps have a determinant different from 1.

## Transport of the σ-matrix and Twiss parameters {#linear-optics-twiss-transport}

### σ-matrix

$X_1 = MX_0$ immediately gives the transport rule of the second moments ([σ-matrix](#phase-space-sigma)):

$$
\\Sigma_1 = M\\,\\Sigma_0\\,M\\tp
$$

Since $\\det\\Sigma_1 = (\\det M)^2\\det\\Sigma_0$, symplectic transport leaves the rms emittance unchanged.

### Twiss parameters

Substituting $\\Sigma = \\varepsilon\\begin{pmatrix} \\beta_x & -\\alpha_x \\\\ -\\alpha_x & \\gamma_x \\end{pmatrix}$ gives

$$
\\begin{pmatrix} \\beta_x \\\\ \\alpha_x \\\\ \\gamma_x \\end{pmatrix}_1 =
\\begin{pmatrix} C^2 & -2CS & S^2 \\\\ -CC' & CS' + SC' & -SS' \\\\ C'^2 & -2C'S' & S'^2 \\end{pmatrix}
\\begin{pmatrix} \\beta_x \\\\ \\alpha_x \\\\ \\gamma_x \\end{pmatrix}_0
$$

Two frequently used special cases:

- **Drift**: $\\beta_x(s) = \\beta_0 - 2\\alpha_0 s + \\gamma_0 s^2$, $\\alpha_x(s) = \\alpha_0 - \\gamma_0 s$, $\\gamma_x$ unchanged. $\\beta_x$ is a parabola in $s$ with the waist at $s^* = \\alpha_0/\\gamma_0$, where $\\beta^* = 1/\\gamma_0$. A distance $s$ from the waist, $\\beta_x = \\beta^* + s^2/\\beta^*$.
- **Thin lens**: $\\beta_x$ unchanged, $\\alpha_1 = \\alpha_0 + \\beta_0/f$. A focusing lens increases $\\alpha_x$, i.e. turns the beam towards convergence.

For example, a lens with $f$ = 0.5 m at a waist with $\\beta_0$ = 1 m: after the lens $\\alpha_x = 2$ and $\\gamma_x = 5$ m$^{-1}$; the new waist lies 0.4 m downstream with $\\beta^*$ = 0.2 m, which can be checked in the [phase-space lab](#phase-space-lab).

### Differential form

In a system with focusing function $K(s)$ the Twiss parameters satisfy

$$
\\beta_x' = -2\\alpha_x, \\qquad \\alpha_x' = K\\beta_x - \\gamma_x
$$

With $w = \\sqrt{\\beta_x}$ this becomes $w'' + K w - 1/w^3 = 0$; multiplying by $\\sqrt{\\varepsilon}$ gives the equation of the rms envelope $a = \\sqrt{\\varepsilon\\beta_x}$, $a'' + Ka - \\varepsilon^2/a^3 = 0$. The last term is the "pressure" of the emittance on the envelope; adding space charge yields the envelope equation of the [Space charge](#space-charge) chapter.

## β-function and betatron phase advance {#linear-optics-phase-advance}

The general solution of Hill's equation can be written (Floquet form) as

$$
x(s) = \\sqrt{J\\beta_x(s)}\\,\\cos\\left(\\psi(s) + \\psi_0\\right), \\qquad
\\psi(s) = \\int_{s_0}^{s} \\frac{\\dd\\sigma}{\\beta_x(\\sigma)}
$$

$J$ is the [Courant–Snyder invariant](#phase-space-twiss) and $\\psi$ the betatron phase. Hence $\\beta_x$ has two meanings: $\\sqrt{J\\beta_x}$ is the envelope of a single particle's oscillation and $\\sqrt{\\varepsilon\\beta_x}$ is the rms size of the beam; and the smaller $\\beta_x$, the faster the phase advances.

In terms of the Twiss parameters at both ends and the phase advance $\\Delta\\psi$, any transfer matrix can be written as

$$
M = \\begin{pmatrix}
\\sqrt{\\dfrac{\\beta_1}{\\beta_0}}\\left(\\cos\\Delta\\psi + \\alpha_0\\sin\\Delta\\psi\\right) & \\sqrt{\\beta_0\\beta_1}\\,\\sin\\Delta\\psi \\\\
\\dfrac{(\\alpha_0 - \\alpha_1)\\cos\\Delta\\psi - (1 + \\alpha_0\\alpha_1)\\sin\\Delta\\psi}{\\sqrt{\\beta_0\\beta_1}} & \\sqrt{\\dfrac{\\beta_0}{\\beta_1}}\\left(\\cos\\Delta\\psi - \\alpha_1\\sin\\Delta\\psi\\right)
\\end{pmatrix}
$$

Conversely, given $M$ and the entrance Twiss parameters, the phase advance follows from

$$
\\tan\\Delta\\psi = \\frac{M_{12}}{\\beta_0 M_{11} - \\alpha_0 M_{12}}
$$

(choosing the quadrant by the sign of $M_{12}$). The phase advance depends only on the entrance Twiss parameters and the matrix, not on the emittance.

## Periodic systems and stability {#linear-optics-periodic}

Linac focusing usually consists of repeated periodic cells (e.g. FODO: focusing quadrupole – drift – defocusing quadrupole – drift, or a "solenoid + cavity" period). Let $M$ be the matrix of one period.

### Stability condition

After $n$ periods the coordinates are $M^n X_0$. The eigenvalues $\\lambda$ of $M$ satisfy $\\lambda^2 - (\\Tr M)\\lambda + 1 = 0$ (because $\\det M = 1$). For $\\lvert \\Tr M \\rvert < 2$ the two eigenvalues are $\\ee^{\\pm i\\mu}$ and the motion is bounded; for $\\lvert \\Tr M \\rvert > 2$ one eigenvalue has modulus greater than 1 and the amplitude grows exponentially. The stability condition is therefore

$$
\\left\\lvert \\tfrac12 \\Tr M \\right\\rvert < 1, \\qquad \\cos\\mu = \\tfrac12 \\Tr M = \\tfrac12\\left(M_{11} + M_{22}\\right)
$$

$\\mu$ is the phase advance per period (written $\\sigma_0$ at zero current).

### Periodic Twiss parameters

In the stable case $M$ can be written as

$$
M = \\begin{pmatrix} \\cos\\mu + \\alpha_x\\sin\\mu & \\beta_x\\sin\\mu \\\\ -\\gamma_x\\sin\\mu & \\cos\\mu - \\alpha_x\\sin\\mu \\end{pmatrix}
$$

which gives the periodic solution $\\beta_x = M_{12}/\\sin\\mu$, $\\alpha_x = (M_{11} - M_{22})/(2\\sin\\mu)$, $\\gamma_x = -M_{21}/\\sin\\mu$, with the sign of $\\sin\\mu$ chosen so that $\\beta_x > 0$. These are fixed by the periodic structure alone, independent of the beam.

### Thin-lens FODO

For two thin lenses of focal lengths $\\pm f$ a distance $L$ apart (period length $2L$),

$$
\\sin\\frac{\\mu}{2} = \\frac{L}{2f}, \\qquad
\\beta_{\\max,\\min} = \\frac{2L\\left(1 \\pm \\sin(\\mu/2)\\right)}{\\sin\\mu}
$$

$\\beta_{\\max}$ is at the centre of the focusing lens and $\\beta_{\\min}$ at the centre of the defocusing lens. Stability requires $f > L/2$. The average β can be estimated with the smooth approximation $\\bar\\beta \\approx 2L/\\mu$.

### Example

1.527 MeV protons, a period QF (0.05 m) – drift 0.10 m – QD (0.05 m) – drift 0.10 m, $G$ = 20 T/m:

- thick-lens matrices: $\\tfrac12\\Tr M = 0.726$, $\\mu$ = 43.45° (the same in both planes), at the QF entrance $\\beta_x$ = 0.546 m and $\\beta_y$ = 0.303 m;
- thin-lens estimate ($f$ = 0.1786 m, lens spacing 0.15 m): $\\sin(\\mu/2) = 0.420$, $\\mu$ = 49.65°, about 6° too large;
- raising the gradient to about 38.2 T/m gives $\\mu$ = 90°; at about 54.1 T/m $\\tfrac12\\lvert\\Tr M\\rvert$ reaches 1 and the beam becomes unstable (the thin-lens approximation gives 47.6 T/m).

Design rules for FODO cells (choice of phase advance, placement of cavities, smooth approximation) are discussed in [Lattice design](#lattice-design).

> Note: stability at zero current is only a necessary condition. Space charge depresses the phase advance per period ($\\sigma < \\sigma_0$), and for $\\sigma_0 > 90°$ intense beams are prone to envelope instabilities and parametric resonances, so high-intensity linacs usually choose $\\sigma_0 < 90°$, see [Space charge](#space-charge).

## Matched beam and mismatch {#linear-optics-matched}

If the Twiss parameters of the incoming beam equal the periodic solution, the beam ellipse returns to its original shape after every period and the rms size repeats periodically along the beamline: the beam is **matched**. Otherwise the ellipse rotates by $\\mu$ and changes shape from period to period, and the envelope shows a "beating": its phase advances by $2\\mu$ per period (the envelope oscillates at twice the betatron frequency) and the maximum size grows.

The degree of mismatch is commonly measured by the mismatch factor. For two sets of Twiss parameters (beam and periodic solution) define

$$
R = \\beta_1\\gamma_2 + \\beta_2\\gamma_1 - 2\\alpha_1\\alpha_2, \\qquad
M_{\\mathrm{mis}} = \\sqrt{\\frac{R + \\sqrt{R^2 - 4}}{2}} - 1
$$

For a perfect match $R = 2$ and $M_{\\mathrm{mis}} = 0$. If only $\\beta$ differs ($\\alpha = 0$, $\\beta_1 = \\lambda\\beta_2$), $M_{\\mathrm{mis}} = \\sqrt{\\lambda} - 1$, i.e. the relative increase of the maximum rms size. In a linear system the rms emittance of a mismatched beam is unchanged and only the envelope oscillates; in a real beamline with non-linear forces the mismatched ellipse filaments, and after complete filamentation the rms emittance has grown by the factor $R/2$. How to match a beam to a periodic structure is the subject of [Matching](#matching).

## The 6×6 form and AVAS's linear preview {#linear-optics-6d}

AVAS's linear preview treats the transverse and longitudinal planes together: 6×6 matrices acting on $(x, x', y, y', z, \\delta)$ push the 6×6 σ-matrix, $\\Sigma_1 = M\\Sigma_0 M\\tp$. For uncoupled elements the matrix is block-diagonal:

$$
M = \\begin{pmatrix} M_x & 0 & 0 \\\\ 0 & M_y & 0 \\\\ 0 & 0 & M_z \\end{pmatrix}, \\qquad
M_z^{\\mathrm{drift}} = \\begin{pmatrix} 1 & L/\\gamma^2 \\\\ 0 & 1 \\end{pmatrix}
$$

The $L/\\gamma^2$ in the longitudinal block comes from $\\Delta\\beta/\\beta = \\delta/\\gamma^2$ ([kinematics](#kinematics-differential)): a particle with larger momentum is faster and after a length $L$ is $L\\delta/\\gamma^2$ ahead of the reference particle. The elements are treated as follows:

- **drift, quad**: transverse as above, longitudinal block as for a drift;
- **solenoid**: an $x$–$y$ coupled 4×4 block (hard edge, $C = \\cos k_sL$, $S = \\sin k_sL$), see [Solenoid](#solenoid);
- **bend, edge**: the sector dipole includes the field index $N$ and has $x$–$\\delta$ and $z$–$x$ correlation terms (dispersion); the edge matrix is a thin pole-face rotation, see [Dipole](#dipole);
- **steerer** and diagnostics are identity matrices in the preview (they change the centroid orbit, not the second moments);
- **field maps**: the reference particle's energy and time are integrated in 1 mm steps; the optics uses 4 mm slices, each "a half step in the local $B_z$ – a thin-lens kick from the near-axis field gradients (including RF bunching) – another half step", with the damping from the momentum gain;
- **space charge** (optional): every few steps a linear kick of the rms-equivalent uniform ellipsoid (bunched beam) or elliptical cylinder (DC beam).

> AVAS convention: the linear preview is for quick estimates only. The reference-particle energy is within about 0.01 %, rms sizes usually within 1–2 %, up to 13 % when multi-particle effects are strong; it does not include dipole / steering content of field maps, \`superposeout\` offsets, chromatic and non-linear effects (such as the growth of the longitudinal emittance in low-energy cavities), particle losses or \`scanphase\`. The kernel results are the reference.

## Hands-on lab {#linear-optics-lab}

### Thin lens and drift

Use the phase-space lab to verify the example of [Twiss transport](#linear-optics-twiss-transport): change \`f\` and \`L\` and watch the sign of $\\alpha_x$ change and the waist position $s^* = \\alpha_x/\\gamma_x$.

\`\`\`avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
\`\`\`

### Stability of a FODO period

The lab gives the phase advance per cell (thick-lens matrix and thin-lens estimate), the stability criterion $\\tfrac12\\lvert\\Tr M\\rvert$, the periodic $\\beta_x$, $\\beta_y$ along one cell and the rms envelope over several cells. Suggestions:

1. Start at $G$ = 20 T/m and confirm $\\mu \\approx 43°$; increase $G$ step by step and see $\\mu$ pass 90° and the beam become unstable at about 54 T/m, with a diverging envelope.
2. Compare the thick-lens and thin-lens phase advances to see the error of the thin-lens approximation when $\\sqrt{k}L$ is not small.
3. Set \`mismatch\` to 1.3 (initial size enlarged by 30 %) and watch the envelope beat; back at 1 the envelope is strictly periodic.

\`\`\`avas-lab
fodo Lq=0.05 G=20 Ld=0.1 cells=10 W=1.527 mass=938.272 q=1 emit=0.14 I=0 mismatch=1
\`\`\`

## Further reading {#linear-optics-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: transverse dynamics, FODO and periodic systems.
- S. Y. Lee, *Accelerator Physics*, 4th ed.: Hill's equation, the Floquet transformation and Courant–Snyder parameters.
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1.
- K. L. Brown, "A first- and second-order matrix theory for the design of beam transport systems and charged particle spectrometers", SLAC-75: the TRANSPORT matrix conventions.
- Continue with [Longitudinal motion](#longitudinal), [Quadrupole](#quadrupole), [Lattice design](#lattice-design).
`,_=`# 线性光学

当外力与偏离量成正比时，粒子的运动方程是线性的，任意一段束线都可以用一个传输矩阵描述，束流的二阶矩和 Twiss 参数按简单的规则传输。线性光学是束线设计、匹配和稳定性分析的骨架，也是 AVAS 线性包络预览和本入门动手实验所用的模型。本章从 Hill 方程出发，给出漂移段、薄透镜和厚四极铁的传输矩阵，说明辛性、Twiss 参数的传输、betatron 相移、周期系统的稳定条件和匹配束，最后介绍 AVAS 线性预览所用的 6×6 形式。

记号同 [相空间](#phase-space)：$\\beta$、$\\gamma$ 为相对论因子，$\\beta_x$、$\\alpha_x$、$\\gamma_x$ 为 Twiss 参数。

## 运动方程与 Hill 方程 {#linear-optics-hill}

### 近轴运动方程

设粒子沿 $s$ 运动，横向动量 $p_x \\approx p\\,x'$。由 $\\dd p_x/\\dd t = F_x$ 和 $\\dd/\\dd t = v\\,\\dd/\\dd s$ 得

$$
x'' + \\frac{p'}{p}\\,x' = \\frac{F_x}{p\\,v}
$$

其中 $p'/p = (\\beta\\gamma)'/(\\beta\\gamma)$ 只在加速段不为零，它就是 [绝热阻尼](#phase-space-liouville) 项。对磁场 $F_x = -q v B_y$，对电场 $F_x = qE_x$，于是右边分别为 $-B_y/(B\\rho)$ 和 $E_x/(E\\rho)$（[刚度](#kinematics-rigidity)）。

### Hill 方程

若横向场对偏离量是线性的，例如四极铁 $B_y = G\\,x$、$B_x = G\\,y$，在无加速的段落中运动方程化为

$$
x'' + K_x(s)\\,x = 0, \\qquad y'' + K_y(s)\\,y = 0
$$

这就是 Hill 方程。各元件的聚焦函数：

| 元件 | $K_x$ | $K_y$ | 详见 |
|---|---|---|---|
| 漂移段 | 0 | 0 | [漂移段](#drift) |
| 四极铁（梯度 $G$） | $k = G/(B\\rho)$ | $-k$ | [四极铁](#quadrupole) |
| 扇形二极铁（场指数 $N$，偏转在 $x$ 平面） | $(1-N)/\\rho^2$ | $N/\\rho^2$ | [二极铁](#dipole) |
| 螺线管（在拉莫尔旋转坐标系中） | $k_s^2$，$k_s = B/(2B\\rho)$ | $k_s^2$ | [螺线管](#solenoid) |

对正电荷粒子，$G > 0$ 的四极铁在 $x$ 方向聚焦、$y$ 方向散焦：在 $x > 0$ 处 $B_y > 0$，$\\boldsymbol v \\times \\boldsymbol B$ 指向 $-x$。四极铁不可能同时在两个平面聚焦（无源区 $\\nabla\\times\\boldsymbol B = 0$ 要求 $\\partial B_y/\\partial x = \\partial B_x/\\partial y$，于是 $K_y = -K_x$），所以需要交替排列，见 [周期系统](#linear-optics-periodic)。

## 传输矩阵 {#linear-optics-matrices}

线性方程的解可以写成初值的线性组合 $X(s) = M(s \\mid s_0)\\,X(s_0)$，$X = (x, x')\\tp$。设 $C(s)$、$S(s)$ 是满足 $C(s_0) = 1, C'(s_0) = 0$ 与 $S(s_0) = 0, S'(s_0) = 1$ 的两个基本解（余弦型与正弦型），则

$$
M = \\begin{pmatrix} C & S \\\\ C' & S' \\end{pmatrix}
$$

一段束线的总矩阵是各元件矩阵按顺序相乘，**后经过的元件写在左边**：$M = M_n \\cdots M_2 M_1$。

### 漂移段（长度 $L$）

$$
M_{\\mathrm{drift}} = \\begin{pmatrix} 1 & L \\\\ 0 & 1 \\end{pmatrix}
$$

### 薄透镜（焦距 $f$）

$$
M_{\\mathrm{thin}} = \\begin{pmatrix} 1 & 0 \\\\ -1/f & 1 \\end{pmatrix}
$$

$f > 0$ 为聚焦，$f < 0$ 为散焦。

### 厚四极铁（长度 $L$，$k > 0$，$\\phi = \\sqrt{k}\\,L$）

聚焦平面与散焦平面分别为

$$
M_{\\mathrm F} = \\begin{pmatrix} \\cos\\phi & \\dfrac{\\sin\\phi}{\\sqrt{k}} \\\\ -\\sqrt{k}\\sin\\phi & \\cos\\phi \\end{pmatrix}, \\qquad
M_{\\mathrm D} = \\begin{pmatrix} \\cosh\\phi & \\dfrac{\\sinh\\phi}{\\sqrt{k}} \\\\ \\sqrt{k}\\sinh\\phi & \\cosh\\phi \\end{pmatrix}
$$

厚透镜的等效焦距由 $1/f = -M_{21}$ 给出，即聚焦平面 $1/f = \\sqrt{k}\\sin\\phi$；当 $\\phi \\ll 1$ 时退化为薄透镜 $1/f \\approx kL$，称为薄透镜近似。

示例：1.527 MeV 质子（$B\\rho$ = 0.17863 T·m），$G$ = 20 T/m、$L$ = 0.05 m 的四极铁，$k$ = 111.96 m$^{-2}$，$\\phi$ = 0.529：

$$
M_{\\mathrm F} = \\begin{pmatrix} 0.86328 & 0.04770\\ \\mathrm m \\\\ -5.34063\\ \\mathrm m^{-1} & 0.86328 \\end{pmatrix}, \\qquad
M_{\\mathrm D} = \\begin{pmatrix} 1.14325 & 0.05237\\ \\mathrm m \\\\ 5.86300\\ \\mathrm m^{-1} & 1.14325 \\end{pmatrix}
$$

聚焦平面等效焦距 0.1872 m，散焦平面 $-0.1706$ m，薄透镜近似给出 $\\pm 0.1786$ m。$\\phi \\approx 0.5$ 时薄透镜近似的误差已有约 5 %，这在低能强聚焦段很常见，所以 AVAS 的矩阵元件和线性预览都使用厚透镜矩阵。

## 辛性与行列式 {#linear-optics-symplectic}

哈密顿系统的线性传输矩阵是**辛矩阵**：

$$
M\\tp J\\,M = J, \\qquad J = \\begin{pmatrix} 0 & 1 \\\\ -1 & 0 \\end{pmatrix}
$$

（多维时 $J$ 是各平面 2×2 块组成的分块对角矩阵。）对 2×2 矩阵，辛条件等价于 $\\det M = CS' - SC' = 1$。上面的所有矩阵都满足这一点，可以用来检查手算或程序的结果。辛性保证相空间面积守恒，也就是 [刘维尔定理](#phase-space-liouville) 在线性情况下的体现；它还给出逆矩阵的简单形式

$$
M^{-1} = \\begin{pmatrix} S' & -S \\\\ -C' & C \\end{pmatrix}
$$

**有加速时**，在 $(x, x')$ 坐标下 $\\det M = (\\beta\\gamma)_0/(\\beta\\gamma)_1 < 1$，这就是绝热阻尼；改用 $(x, \\beta\\gamma\\,x')$ 坐标，矩阵重新成为辛矩阵。

> 注意：线性预览中的 drift、quad、solenoid、bend、edge 的 6×6 矩阵都是辛矩阵（已在 $(x, x')$、$(y, y')$、$(z, \\delta)$ 三对共轭坐标下数值核对）；行列式不为 1 的只有场图中带加速的切片。

## σ 矩阵与 Twiss 参数的传输 {#linear-optics-twiss-transport}

### σ 矩阵

由 $X_1 = MX_0$ 立即得到二阶矩的传输规律（[σ 矩阵](#phase-space-sigma)）：

$$
\\Sigma_1 = M\\,\\Sigma_0\\,M\\tp
$$

由于 $\\det\\Sigma_1 = (\\det M)^2\\det\\Sigma_0$，辛传输下 rms 发射度不变。

### Twiss 参数

把 $\\Sigma = \\varepsilon\\begin{pmatrix} \\beta_x & -\\alpha_x \\\\ -\\alpha_x & \\gamma_x \\end{pmatrix}$ 代入，得到

$$
\\begin{pmatrix} \\beta_x \\\\ \\alpha_x \\\\ \\gamma_x \\end{pmatrix}_1 =
\\begin{pmatrix} C^2 & -2CS & S^2 \\\\ -CC' & CS' + SC' & -SS' \\\\ C'^2 & -2C'S' & S'^2 \\end{pmatrix}
\\begin{pmatrix} \\beta_x \\\\ \\alpha_x \\\\ \\gamma_x \\end{pmatrix}_0
$$

两个常用特例：

- **漂移段**：$\\beta_x(s) = \\beta_0 - 2\\alpha_0 s + \\gamma_0 s^2$，$\\alpha_x(s) = \\alpha_0 - \\gamma_0 s$，$\\gamma_x$ 不变。$\\beta_x$ 是 $s$ 的抛物线，束腰位于 $s^* = \\alpha_0/\\gamma_0$，束腰处 $\\beta^* = 1/\\gamma_0$。离开束腰距离 $s$ 后 $\\beta_x = \\beta^* + s^2/\\beta^*$。
- **薄透镜**：$\\beta_x$ 不变，$\\alpha_1 = \\alpha_0 + \\beta_0/f$。聚焦透镜使 $\\alpha_x$ 增大，即使束流向会聚方向转。

例如束腰 $\\beta_0$ = 1 m 处放 $f$ = 0.5 m 的透镜：透镜后 $\\alpha_x = 2$、$\\gamma_x = 5$ m$^{-1}$，新束腰在其后 0.4 m 处，$\\beta^*$ = 0.2 m，可以在 [相空间实验](#phase-space-lab) 中验证。

### 微分形式

在聚焦函数为 $K(s)$ 的系统中，Twiss 参数满足

$$
\\beta_x' = -2\\alpha_x, \\qquad \\alpha_x' = K\\beta_x - \\gamma_x
$$

令 $w = \\sqrt{\\beta_x}$，则 $w'' + K w - 1/w^3 = 0$；乘以 $\\sqrt{\\varepsilon}$ 得到 rms 包络 $a = \\sqrt{\\varepsilon\\beta_x}$ 的方程 $a'' + Ka - \\varepsilon^2/a^3 = 0$。最后一项是发射度对包络的"压力"，加入空间电荷后就是 [空间电荷](#space-charge) 一章的包络方程。

## β 函数与 betatron 相移 {#linear-optics-phase-advance}

Hill 方程的一般解可以写成（Floquet 形式）

$$
x(s) = \\sqrt{J\\beta_x(s)}\\,\\cos\\left(\\psi(s) + \\psi_0\\right), \\qquad
\\psi(s) = \\int_{s_0}^{s} \\frac{\\dd\\sigma}{\\beta_x(\\sigma)}
$$

$J$ 是 [Courant–Snyder 不变量](#phase-space-twiss)，$\\psi$ 是 betatron 相位。由此 $\\beta_x$ 有两个意义：$\\sqrt{J\\beta_x}$ 是单个粒子振荡的包络，$\\sqrt{\\varepsilon\\beta_x}$ 是束流的 rms 尺寸；同时 $\\beta_x$ 越小，相位推进越快。

用两端的 Twiss 参数和相移 $\\Delta\\psi$，任意传输矩阵可以写成

$$
M = \\begin{pmatrix}
\\sqrt{\\dfrac{\\beta_1}{\\beta_0}}\\left(\\cos\\Delta\\psi + \\alpha_0\\sin\\Delta\\psi\\right) & \\sqrt{\\beta_0\\beta_1}\\,\\sin\\Delta\\psi \\\\
\\dfrac{(\\alpha_0 - \\alpha_1)\\cos\\Delta\\psi - (1 + \\alpha_0\\alpha_1)\\sin\\Delta\\psi}{\\sqrt{\\beta_0\\beta_1}} & \\sqrt{\\dfrac{\\beta_0}{\\beta_1}}\\left(\\cos\\Delta\\psi - \\alpha_1\\sin\\Delta\\psi\\right)
\\end{pmatrix}
$$

反过来，已知 $M$ 和入口 Twiss 参数时，相移由

$$
\\tan\\Delta\\psi = \\frac{M_{12}}{\\beta_0 M_{11} - \\alpha_0 M_{12}}
$$

给出（按 $M_{12}$ 的符号选取象限）。相移只取决于入口 Twiss 参数和矩阵，与发射度无关。

## 周期系统与稳定性 {#linear-optics-periodic}

直线加速器的聚焦通常由重复的周期单元组成（如 FODO：聚焦四极—漂移—散焦四极—漂移，或"螺线管 + 腔"的周期）。设一个周期的矩阵为 $M$。

### 稳定条件

粒子经过 $n$ 个周期后的坐标为 $M^n X_0$。$M$ 的特征值 $\\lambda$ 满足 $\\lambda^2 - (\\Tr M)\\lambda + 1 = 0$（因 $\\det M = 1$）。当 $\\lvert \\Tr M \\rvert < 2$ 时两个特征值为 $\\ee^{\\pm i\\mu}$，运动有界；当 $\\lvert \\Tr M \\rvert > 2$ 时有一个特征值模大于 1，振幅指数增长。因此稳定条件是

$$
\\left\\lvert \\tfrac12 \\Tr M \\right\\rvert < 1, \\qquad \\cos\\mu = \\tfrac12 \\Tr M = \\tfrac12\\left(M_{11} + M_{22}\\right)
$$

$\\mu$ 是每个周期的相移（零流强时记为 $\\sigma_0$）。

### 周期 Twiss 参数

稳定时 $M$ 可以写成

$$
M = \\begin{pmatrix} \\cos\\mu + \\alpha_x\\sin\\mu & \\beta_x\\sin\\mu \\\\ -\\gamma_x\\sin\\mu & \\cos\\mu - \\alpha_x\\sin\\mu \\end{pmatrix}
$$

由此得到周期解：$\\beta_x = M_{12}/\\sin\\mu$，$\\alpha_x = (M_{11} - M_{22})/(2\\sin\\mu)$，$\\gamma_x = -M_{21}/\\sin\\mu$，其中 $\\sin\\mu$ 的符号取得使 $\\beta_x > 0$。它们只由周期结构决定，与束流无关。

### 薄透镜 FODO

两个焦距 $\\pm f$ 的薄透镜相距 $L$（周期长度 $2L$），可以得到

$$
\\sin\\frac{\\mu}{2} = \\frac{L}{2f}, \\qquad
\\beta_{\\max,\\min} = \\frac{2L\\left(1 \\pm \\sin(\\mu/2)\\right)}{\\sin\\mu}
$$

$\\beta_{\\max}$ 在聚焦透镜中心，$\\beta_{\\min}$ 在散焦透镜中心。稳定要求 $f > L/2$。平均 β 可以用平滑近似 $\\bar\\beta \\approx 2L/\\mu$ 估计。

### 示例

1.527 MeV 质子，周期为 QF（0.05 m）— 漂移 0.10 m — QD（0.05 m）— 漂移 0.10 m，$G$ = 20 T/m：

- 厚透镜矩阵：$\\tfrac12\\Tr M = 0.726$，$\\mu$ = 43.45°（两个平面相同），QF 入口处 $\\beta_x$ = 0.546 m、$\\beta_y$ = 0.303 m；
- 薄透镜估计（$f$ = 0.1786 m，透镜间距 0.15 m）：$\\sin(\\mu/2) = 0.420$，$\\mu$ = 49.65°，偏大约 6°；
- 把梯度增大到约 38.2 T/m 时 $\\mu$ = 90°；约 54.1 T/m 时 $\\tfrac12\\lvert\\Tr M\\rvert$ 达到 1，束流失稳（薄透镜近似给出 47.6 T/m）。

FODO 单元的设计要点（相移的选择、与腔的配置、平滑近似）在 [束线设计](#lattice-design) 中详述。

> 注意：零流强稳定只是必要条件。有空间电荷时每周期相移会被压低（$\\sigma < \\sigma_0$），而 $\\sigma_0 > 90°$ 时强流束容易出现包络不稳定和参数共振，所以强流直线加速器通常取 $\\sigma_0 < 90°$，见 [空间电荷](#space-charge)。

## 匹配束与失配 {#linear-optics-matched}

如果入口束流的 Twiss 参数恰好等于周期解，那么经过每个周期后束流椭圆回到原来的形状，rms 尺寸沿束线按周期重复，这称为**匹配束**。否则椭圆在每个周期后转过 $\\mu$ 角并改变形状，包络出现"拍"（beating）：它的相位每个周期推进 $2\\mu$（包络振荡频率是 betatron 振荡的两倍），最大尺寸增大。

失配程度常用失配因子衡量。对两组 Twiss 参数（束流与周期解）定义

$$
R = \\beta_1\\gamma_2 + \\beta_2\\gamma_1 - 2\\alpha_1\\alpha_2, \\qquad
M_{\\mathrm{mis}} = \\sqrt{\\frac{R + \\sqrt{R^2 - 4}}{2}} - 1
$$

完全匹配时 $R = 2$、$M_{\\mathrm{mis}} = 0$。若只是 $\\beta$ 不同（$\\alpha = 0$，$\\beta_1 = \\lambda\\beta_2$），$M_{\\mathrm{mis}} = \\sqrt{\\lambda} - 1$，即最大 rms 尺寸增大的比例。在线性系统中失配束的 rms 发射度不变，只是包络振荡；而在有非线性力的真实束线中，失配椭圆会丝化，完全丝化后的 rms 发射度增大到原来的 $R/2$ 倍。如何把束流匹配到周期结构见 [匹配](#matching)。

## 6×6 形式与 AVAS 线性预览 {#linear-optics-6d}

AVAS 的线性预览把横向和纵向放在一起，用作用于 $(x, x', y, y', z, \\delta)$ 的 6×6 矩阵推进 6×6 σ 矩阵 $\\Sigma_1 = M\\Sigma_0 M\\tp$。无耦合元件的矩阵是分块对角的：

$$
M = \\begin{pmatrix} M_x & 0 & 0 \\\\ 0 & M_y & 0 \\\\ 0 & 0 & M_z \\end{pmatrix}, \\qquad
M_z^{\\mathrm{drift}} = \\begin{pmatrix} 1 & L/\\gamma^2 \\\\ 0 & 1 \\end{pmatrix}
$$

纵向块的 $L/\\gamma^2$ 来自 $\\Delta\\beta/\\beta = \\delta/\\gamma^2$（[运动学](#kinematics-differential)）：动量较大的粒子跑得较快，经过 $L$ 后在参考粒子前方 $L\\delta/\\gamma^2$ 处。各元件的处理：

- **drift、quad**：横向如上，纵向块同漂移段；
- **solenoid**：$x$–$y$ 耦合的 4×4 块（硬边，$C = \\cos k_sL$、$S = \\sin k_sL$），见 [螺线管](#solenoid)；
- **bend、edge**：扇形二极铁含场指数 $N$，出现 $x$–$\\delta$ 与 $z$–$x$ 的关联项（色散），边缘矩阵为薄极面旋转，见 [二极铁](#dipole)；
- **steerer** 与束诊在预览中是单位矩阵（只改变中心轨道，不改变二阶矩）；
- **场图**：参考粒子以 1 mm 步长积分能量与时间；光学按 4 mm 切片，每片是"局部 $B_z$ 中的半步—由近轴场梯度给出的薄透镜冲量（含射频聚束）—再半步"，并计入动量增加带来的阻尼；
- **空间电荷**（可选）：每隔几个步长加一次 rms 等效均匀椭球（束团）或椭圆柱（直流束）的线性冲量。

> AVAS 约定：线性预览只用于快速评估。参考粒子能量误差约 0.01 %，rms 尺寸通常相差 1–2 %，多粒子效应强时可达 13 %；它不包含场图中的二极 / 校正分量、\`superposeout\` 偏移、色差和非线性效应（如低能腔中纵向发射度的增长）、粒子损失以及 \`scanphase\`。最终以内核结果为准。

## 动手实验 {#linear-optics-lab}

### 薄透镜与漂移

用相空间实验验证 [Twiss 传输](#linear-optics-twiss-transport) 中的例子：改变 \`f\` 和 \`L\`，观察 $\\alpha_x$ 的符号变化与束腰位置 $s^* = \\alpha_x/\\gamma_x$。

\`\`\`avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
\`\`\`

### FODO 周期的稳定性

实验给出每周期相移（厚透镜矩阵和薄透镜估计）、稳定判据 $\\tfrac12\\lvert\\Tr M\\rvert$、一个周期内的周期 $\\beta_x$、$\\beta_y$ 以及多个周期上的 rms 包络。建议：

1. 从 $G$ = 20 T/m 开始，确认 $\\mu \\approx 43°$；逐步增大 $G$，看 $\\mu$ 经过 90° 后在约 54 T/m 处失稳，包络发散。
2. 比较厚透镜与薄透镜的相移，体会 $\\sqrt{k}L$ 不小时薄透镜近似的误差。
3. 把 \`mismatch\` 设为 1.3（初始尺寸放大 30 %），观察包络的拍；设回 1 时包络严格周期。

\`\`\`avas-lab
fodo Lq=0.05 G=20 Ld=0.1 cells=10 W=1.527 mass=938.272 q=1 emit=0.14 I=0 mismatch=1
\`\`\`

## 延伸阅读 {#linear-optics-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.：横向动力学、FODO 与周期系统。
- S. Y. Lee, *Accelerator Physics*, 4th ed.：Hill 方程、Floquet 变换与 Courant–Snyder 参数。
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1。
- K. L. Brown, "A first- and second-order matrix theory for the design of beam transport systems and charged particle spectrometers", SLAC-75：TRANSPORT 矩阵约定。
- 继续阅读：[纵向运动](#longitudinal)、[四极铁](#quadrupole)、[束线设计](#lattice-design)。
`,v=`# Longitudinal dynamics

RF cavities give the particles energy and at the same time decide whether the bunch stays together longitudinally (in time / phase). This chapter starts from the energy gain in one accelerating gap, introduces the transit-time factor and multi-gap cavities, and then treats the synchronous particle, phase stability, small-amplitude synchrotron oscillations, the longitudinal acceptance, adiabatic damping and the RF defocusing that necessarily accompanies longitudinal focusing. It ends with the three definitions of the RF phase in AVAS (parameter V3 of \`field\`) and the related keywords. Kinematic quantities ($\\beta$, $\\gamma$, $\\beta\\lambda$) are defined in [Relativistic kinematics](#kinematics), phase space and emittance in [Phase space and emittance](#phase-space), and the cavity element itself in [RF cavities](#rf-cavity).

All numerical examples use the example project \`examples/hwr010\`: protons, entry kinetic energy $W = 1.527\\ \\mathrm{MeV}$ ($\\beta = 0.05698$, $\\gamma = 1.00163$), RF frequency $f = 162.5\\ \\mathrm{MHz}$ ($\\lambda = c/f = 1.8449\\ \\mathrm{m}$, $\\beta\\lambda = 0.1051\\ \\mathrm{m}$), half-wave resonator (HWR) field map \`hwr010\`, 0.21 m long, \`Ke = 1.36\`.

## Energy gain in an RF gap {#longitudinal-gap}

### Field and phase

Let the longitudinal on-axis field in the cavity be a standing wave

$$
E_z(z, t) = E(z)\\cos\\left(\\omega t + \\varphi_0\\right),
$$

where $E(z)$ is the spatial profile and $\\omega = 2\\pi f$. A particle moving along the axis reaches $z$ at time $t(z)$ and "sees" the phase

$$
\\varphi(z) = \\omega t(z) + \\varphi_0 .
$$

In this convention **a larger phase means a later arrival**. The energy gained in the gap is

$$
\\Delta W = q\\int E(z)\\cos\\varphi(z)\\,\\dd z .
\\tag{1}
$$

> AVAS convention: the RF electric field acts as $K_e\\,E(\\boldsymbol r)\\cos(\\omega t + \\varphi_0)$ and the RF magnetic field as $K_b\\,B(\\boldsymbol r)\\sin(\\omega t + \\varphi_0)$ ($E$ in MV/m, $B$ in T in the map). This is the cosine convention of Wangler's *RF Linear Accelerators*. The longitudinal coordinate is related to the phase by $\\Delta\\varphi = -2\\pi z/(\\beta\\lambda)$: a particle with $z > 0$ is ahead of the reference particle and has a smaller phase.

### Panofsky formula and transit-time factor

If the velocity hardly changes in the gap ($\\Delta W \\ll W$), $t(z)$ can be taken as uniform motion: $\\omega t(z) = \\omega t_c + k (z - z_c)$ with $k = 2\\pi/(\\beta\\lambda)$ and $z_c$ a reference point of the gap. Inserting into (1):

$$
\\Delta W = q\\cos\\varphi\\int E(z)\\cos k(z - z_c)\\,\\dd z - q\\sin\\varphi\\int E(z)\\sin k(z - z_c)\\,\\dd z ,
$$

where $\\varphi = \\omega t_c + \\varphi_0$ is the phase at $z_c$. Choosing $z_c$ at the **electrical centre**, where the second integral vanishes (the geometric centre of a symmetric gap), gives the Panofsky formula

$$
\\Delta W = q E_0 T L\\cos\\varphi = q V_0 T\\cos\\varphi ,
\\qquad
T = \\frac{\\int E(z)\\cos k(z - z_c)\\,\\dd z}{\\int E(z)\\,\\dd z},
\\tag{2}
$$

with $E_0 = V_0/L$ the average on-axis field over the gap length $L$ and $V_0 = \\int E\\,\\dd z$ the on-axis voltage. The **transit-time factor** $T \\le 1$ accounts for the field changing while the particle crosses the gap. For a uniform field over a gap of length $g$,

$$
T = \\frac{\\sin(\\pi g/\\beta\\lambda)}{\\pi g/\\beta\\lambda},
$$

which approaches 1 the shorter the gap is compared with $\\beta\\lambda$. Real cavity fields extend beyond the gap, so $T$ has to be computed from the field profile.

### Exact definition: phase along the actual trajectory

When the energy gain is comparable with the entry energy (low-energy protons, heavy ions) uniform motion is no longer a good approximation. Equation (1) is then written as

$$
C = \\int E(z)\\cos\\varphi(z)\\,\\dd z,\\quad S = \\int E(z)\\sin\\varphi(z)\\,\\dd z,\\quad
\\Delta W = q\\sqrt{C^2 + S^2}\\,\\cos\\varphi_s,\\quad \\varphi_s = \\operatorname{atan2}(S, C),
\\tag{3}
$$

with the integrals taken along the **actual** trajectory $t(z)$ of the reference particle. Equation (3) is exact and $\\varphi_s$ is what AVAS calls the synchronous phase (see [below](#longitudinal-synchronous)); but the "effective voltage" $\\sqrt{C^2 + S^2}$ itself depends on the trajectory, hence on $\\varphi_s$, so the energy gain no longer follows $\\cos\\varphi_s$ exactly.

## Transit-time factor and multi-gap cavities {#longitudinal-multigap}

### Two-gap cavities

The accelerating part of a half-wave resonator (HWR) or quarter-wave resonator (QWR) consists of two gaps with opposite field directions ($\\pi$ mode). With $e(z)$ the field of one gap and $d$ the distance of the gap centres, $E(z) = e(z - d/2) - e(z + d/2)$. Its Fourier transform gives

$$
T(\\beta) = T_g(\\beta)\\,\\left\\lvert\\sin\\frac{k d}{2}\\right\\rvert = T_g(\\beta)\\,\\left\\lvert\\sin\\frac{\\pi\\beta_g}{2\\beta}\\right\\rvert,
\\qquad d = \\frac{\\beta_g\\lambda}{2},
$$

$T_g$ being the transit-time factor of a single gap. For a multi-gap field of alternating sign $\\int E\\,\\dd z \\approx 0$, so the denominator of (2) is replaced by $V_0 = \\int\\lvert E\\rvert\\,\\dd z$ (as in the table below). When the particle needs exactly half an RF period from one gap to the next ($\\beta = \\beta_g$), both gaps accelerate in phase. A cavity with $N$ gaps has a sharper $T(\\beta)$ and a narrower energy acceptance; low-$\\beta$ superconducting linacs therefore use cavities with few gaps, in several geometric $\\beta$ families.

### The example cavity hwr010

Integrating the on-axis $E_z$ of the \`hwr010\` map (times \`Ke = 1.36\`) gives:

| Quantity | Value |
|---|---|
| Peak on-axis field | $+13.03\\ \\mathrm{MV/m}$ and $-12.74\\ \\mathrm{MV/m}$ |
| Sign change of the field | $z \\approx 0.104\\ \\mathrm{m}$ (two gaps) |
| Field centroids of the two gaps | $0.064\\ \\mathrm{m}$, $0.145\\ \\mathrm{m}$, spacing $d = 0.081\\ \\mathrm{m}$ |
| $\\beta_g = 2d/\\lambda$ from the spacing | 0.088 |
| $V_0 = \\int\\lvert E_z\\rvert\\,\\dd z$ | 0.978 MV |
| Maximum of $T(\\beta)$ | 0.82 at $\\beta \\approx 0.104$ |
| $T$ at the entry $\\beta = 0.0570$ | 0.40, $V_0 T = 0.39\\ \\mathrm{MV}$ |

The maximum of $T(\\beta)$ at $\\beta \\approx 0.10$ matches the "010" in the name (usually meaning a design $\\beta = 0.10$); it is a little above the 0.088 estimated from the spacing because the single-gap factor $T_g(\\beta)$ grows with $\\beta$. The example beam has $\\beta = 0.057$, far below the optimum, so $T$ is only 0.40; as the particle is accelerated inside the cavity $T$ grows, which is one reason why the uniform-motion approximation fails in low-energy cavities.

\`\`\`avas-field
map hwr010
\`\`\`

### Energy gain versus synchronous phase: example

Exit energies from the AVAS linear preview (\`avas.sim.linear_optics\`, lattice \`field 0.21 0.02 0 1 162.5e6 φs 1.36 -1.36 hwr010\`):

| $\\varphi_s$ (V3 = 0) | Entry phase | Exit energy / MeV | $\\Delta W$ / MeV |
|---|---|---|---|
| −90° | −14.0° | 1.5270 | 0.000 |
| −60° | 18.6° | 1.6943 | 0.167 |
| −33° | 52.7° | 1.8665 | 0.340 |
| −20° | 69.7° | 1.9374 | 0.410 |
| 0° | 95.4° | 2.0023 | 0.475 |
| +20° | 119.7° | 1.9954 | 0.468 |

At $\\varphi_s = -90°$ the gain vanishes, as (3) requires. But $\\Delta W/\\cos\\varphi_s$ is 0.405 MeV at −33° and 0.475 MeV at 0°, about 15 % apart, and in a scan of the entry phase the largest gain (2.0087 MeV) occurs at $\\varphi_s \\approx +8°$, not at 0°. The reason is the one given above: $\\Delta W/W \\approx 30\\ \\%$, and $\\beta$ grows from 0.057 to about 0.065 inside the cavity, so the effective voltage changes with the phase. For such cavities the Panofsky formula is only an estimate; designs must rely on the integration along the trajectory.

## Synchronous particle and synchronous phase {#longitudinal-synchronous}

The **synchronous particle** (reference particle) is the particle that by design moves along the axis and meets every cavity at its design phase; the longitudinal coordinates of all other particles are defined relative to it. In a linac made of many cavities, the phase $\\varphi_{s,n}$ of the synchronous particle in cavity $n$ is set by the RF phase of that cavity together with the flight time through everything upstream.

### The three meanings of the phase in AVAS (V3)

The third parameter V3 of \`field L R V3 type f phase Ke Kb file\` sets the meaning of the sixth parameter \`phase\`:

| V3 | Meaning of \`phase\` | How the kernel sets the entry phase $\\varphi_0$ |
|---|---|---|
| 0 | synchronous phase $\\varphi_s$ | solves for $\\varphi_0$ such that $\\operatorname{atan2}(S, C) = \\varphi_s$ along the reference trajectory, eq. (3) |
| 1 | RF phase when the reference particle reaches the element entry | $\\varphi_0$ = \`phase\` |
| 2 | RF phase at $t = 0$ (absolute phase) | $\\varphi_0 = $ \`phase\` $+ 360°\\cdot f\\cdot t_{\\mathrm{in}}$, $t_{\\mathrm{in}}$ the arrival time of the reference particle at the entry |

V3 = 0 is the most convenient for design: the physically meaningful quantity is the synchronous phase, and the program solves for the entry phase. V3 = 1 suits phase scans (the entry phase is tied directly to the cavity geometry). V3 = 2 corresponds to a real machine, where each cavity's RF is phased against a common reference clock; changing anything upstream changes the synchronous phase.

Example: put a 0.1 m drift before the example cavity. The reference particle reaches the cavity at $t_{\\mathrm{in}} = 0.1/(\\beta c) = 5.854\\ \\mathrm{ns}$, so $360°\\,f\\,t_{\\mathrm{in}} = 342.45°$. With V3 = 2 and \`phase = 0\` the entry phase is $342.45° \\equiv -17.55°$, which corresponds to a synchronous phase of about −93.7°: the cavity hardly accelerates. With V3 = 0 and \`phase = -33\`, the entry phase is 52.7° whatever the length of the drift, and the synchronous phase stays at −33°.

> AVAS convention: in the example, the synchronous phase $-33°$ corresponds to the entry phase $52.7°$; the difference (about 86°) depends on where the field map starts and on its field profile, and there is no general conversion. For V3 = 0 cavities the linear preview reports both the solved entry phase and the synchronous phase. When a segment of the lattice is run on its own, AVAS (\`rephase\` in \`avas/data/segment.py\`) takes the RF phase $\\varphi_{\\mathrm{RF}}$ of every cavity from the synchronous-particle data of the reference run and converts it with $\\varphi_{\\mathrm{RF}} - 360°\\cdot f\\cdot(t_{\\mathrm{in}} - T_{\\mathrm{entry}})$ into a V3 = 2 phase whose clock starts at the segment entry, instead of keeping the original phases or switching to V3 = 0.

## Phase stability {#longitudinal-stability}

### Equations of motion

Consider a periodic array of cavities and average their action over unit length: average accelerating gradient $E_0 T$ (the in-cavity $E_0 T$ times the fraction of the period occupied by the cavity). Define the deviations from the synchronous particle

$$
\\Delta\\varphi = \\varphi - \\varphi_s,\\qquad w = \\frac{W - W_s}{mc^2} .
$$

From $\\dd t/\\dd s = 1/(\\beta c)$ and $\\Delta\\beta = \\Delta\\gamma/(\\beta\\gamma^3)$:

$$
\\frac{\\dd\\,\\Delta\\varphi}{\\dd s} = \\omega\\left(\\frac{1}{\\beta c} - \\frac{1}{\\beta_s c}\\right) \\approx -\\frac{2\\pi}{\\beta_s^3\\gamma_s^3\\lambda}\\,w \\equiv -A\\,w ,
\\tag{4}
$$

and the energy equation

$$
\\frac{\\dd w}{\\dd s} = \\frac{q E_0 T}{mc^2}\\left(\\cos\\varphi - \\cos\\varphi_s\\right) \\equiv B\\left(\\cos\\varphi - \\cos\\varphi_s\\right).
\\tag{5}
$$

Equation (4) says that a particle with more energy ($w > 0$) is faster and its phase decreases (it arrives earlier).

### Stability condition

Linearising (4) and (5) with $\\cos\\varphi - \\cos\\varphi_s \\approx -\\sin\\varphi_s\\,\\Delta\\varphi$:

$$
\\frac{\\dd^2\\Delta\\varphi}{\\dd s^2} = A B \\sin\\varphi_s\\,\\Delta\\varphi .
$$

Since $A, B > 0$, **the motion is stable only for $\\sin\\varphi_s < 0$**. Together with $\\cos\\varphi_s > 0$ (acceleration), the stable accelerating range is

$$
-90° < \\varphi_s < 0° .
$$

Physical picture: a late particle ($\\Delta\\varphi > 0$) sits on the rising side of the field, gains more energy, becomes faster and catches up; an early particle gains less and falls back.

> Note: sign conventions differ between texts. In the cosine convention of AVAS and Wangler (larger phase = later) the stable synchronous phase is negative; the example project uses $-33°$. With a sine convention, or with the phase defined as $-\\omega t$, the stable range becomes positive. Always check the convention when taking phases from other codes or papers.

### Checking the sign with the AVAS linear preview

The example beam (initial rms $z$ = 1.316 mm, rms $x$ = 0.954 mm) through one hwr010 cavity, rms sizes at the exit (linear preview, space charge off):

| Case | rms $z$ / mm | rms $x$ / mm |
|---|---|---|
| 0.21 m drift (no cavity) | 1.482 | 1.407 |
| $\\varphi_s = -33°$ | 0.702 | 1.811 |
| $\\varphi_s = +33°$ | 1.738 | 1.196 |

A negative synchronous phase compresses the bunch longitudinally (bunching), a positive one lengthens it (debunching), in agreement with the stability condition; transversely it is the other way round, which is the [RF defocusing](#longitudinal-rf-defocusing). Drag the phase in the lab below to see it yourself.

\`\`\`avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
\`\`\`

## Small-amplitude synchrotron oscillations and adiabatic damping {#longitudinal-synchrotron}

### Longitudinal wave number

In the stable case $\\Delta\\varphi$ oscillates harmonically, $\\Delta\\varphi'' + k_{l0}^2\\,\\Delta\\varphi = 0$, with the **zero-current longitudinal wave number**

$$
k_{l0}^2 = -A B\\sin\\varphi_s = \\frac{2\\pi\\, q E_0 T\\sin(-\\varphi_s)}{mc^2\\,\\beta_s^3\\gamma_s^3\\,\\lambda} .
\\tag{6}
$$

The synchrotron wavelength is $\\lambda_l = 2\\pi/k_{l0}$ and the longitudinal phase advance per focusing period $L_p$ is $\\sigma_{0l} = k_{l0}L_p$. From (4), the ratio of the energy and phase amplitudes is $w_{\\max}/\\Delta\\varphi_{\\max} = k_{l0}/A$. The factor $\\beta^3\\gamma^3$ in (6) shows that longitudinal focusing weakens quickly with energy; synchrotron oscillations are fastest at low energy.

Numerical example: if an array of cavities had an average gradient equal to the example cavity's $V_0 T / L = 0.39\\ \\mathrm{MV}/0.21\\ \\mathrm{m} = 1.86\\ \\mathrm{MV/m}$ at the entry, with $\\varphi_s = -33°$, then $k_{l0} = 4.44\\ \\mathrm{m}^{-1}$ and $\\lambda_l = 1.41\\ \\mathrm{m}$. In a real structure solenoids and drifts sit between the cavities, the average gradient is lower and $k_{l0}$ smaller ($\\propto\\sqrt{E_0 T}$).

### Conversion to the AVAS longitudinal coordinates

AVAS uses the longitudinal phase space $(z, \\delta)$ with $\\delta = \\Delta p/p$:

$$
z = -\\frac{\\beta\\lambda}{2\\pi}\\,\\Delta\\varphi,\\qquad
\\delta = \\frac{\\Delta W}{\\beta^2\\gamma\\,mc^2} = \\frac{w}{\\beta^2\\gamma} .
$$

So $z$ obeys the same oscillation equation as $\\Delta\\varphi$. The \`twissz\` keyword of \`beam.txt\` uses $z$ (mm) and $\\delta$ (mrad), and its emittance is again the normalised rms value. The example beam has \`twissz 0.04275 0.37292 0.26506\`, i.e. initial rms $z = 1.316\\ \\mathrm{mm}$ and rms $\\delta = 3.53\\ \\mathrm{mrad}$; for the meaning of the Twiss parameters see [Phase space and emittance](#phase-space).

### Adiabatic damping

When acceleration changes $\\beta\\gamma$ and $E_0 T$ slowly, $(\\Delta\\varphi, w)$ remain a canonical pair (Hamiltonian in the [next section](#longitudinal-separatrix)) and the action $\\oint w\\,\\dd\\Delta\\varphi \\propto \\Delta\\varphi_{\\max} w_{\\max}$ is an adiabatic invariant. With $w_{\\max}/\\Delta\\varphi_{\\max} = k_{l0}/A$ this gives

$$
\\Delta\\varphi_{\\max} \\propto \\left[E_0 T\\sin(-\\varphi_s)\\,\\lambda\\,\\beta_s^3\\gamma_s^3\\right]^{-1/4},\\qquad
\\Delta W_{\\max} \\propto \\left[E_0 T\\sin(-\\varphi_s)\\,\\lambda\\,\\beta_s^3\\gamma_s^3\\right]^{1/4}.
$$

For constant gradient, phase and frequency, $\\Delta\\varphi \\propto (\\beta\\gamma)^{-3/4}$ and $\\Delta W \\propto (\\beta\\gamma)^{3/4}$: the phase width shrinks, the energy spread grows, and their product is constant. In $(z, \\delta)$, $\\beta\\gamma\\cdot z\\,\\delta \\propto \\lambda\\,\\Delta\\varphi\\,\\Delta W$ is constant, i.e. **the normalised longitudinal emittance is conserved**, as in the transverse planes (geometric emittance $\\propto 1/\\beta\\gamma$), see [Phase space and emittance](#phase-space).

> Note: this conservation holds only for linear, uncoupled motion without space charge. The longitudinal non-linearity in low-energy cavities (the curvature of $\\cos\\varphi$ once the phase width reaches tens of degrees) makes the longitudinal rms emittance grow; the linear preview does not contain this effect, it has to be computed with the multi-particle kernel.

## Longitudinal acceptance and separatrix {#longitudinal-separatrix}

### Hamiltonian

Equations (4) and (5) follow from the Hamiltonian (independent variable $s$, $\\varphi$ and $w$ conjugate)

$$
H(\\varphi, w) = -\\frac{A w^2}{2} - B\\left(\\sin\\varphi - \\varphi\\cos\\varphi_s\\right)
$$

via $\\varphi' = \\partial H/\\partial w$ and $w' = -\\partial H/\\partial\\varphi$. $H$ is conserved, so the phase-space trajectories are its contour lines.

### Fixed points and separatrix

$w = 0$ and $\\cos\\varphi = \\cos\\varphi_s$ give two fixed points: $\\varphi = \\varphi_s$ (stable, elliptic) and $\\varphi_2 = -\\varphi_s$ (unstable, hyperbolic). The contour through $\\varphi_2$ is the **separatrix**:

$$
\\frac{A w^2}{2} + B\\left(\\sin\\varphi - \\varphi\\cos\\varphi_s\\right) = -B\\left(\\sin\\varphi_s - \\varphi_s\\cos\\varphi_s\\right).
$$

Particles inside the separatrix perform bounded synchrotron oscillations and form the longitudinal **acceptance** (the "fish" or "golf-club" shaped stable region); particles outside slip continuously in phase, leave the accelerating phase and become longitudinal losses (in AVAS, \`longlimits\` sets phase and energy limits for counting them).

### Width and height of the stable region

- The phase range extends from $\\varphi_1$ to $\\varphi_2 = -\\varphi_s$, with $\\varphi_1$ from $\\sin\\varphi_1 - \\varphi_1\\cos\\varphi_s = -\\sin\\varphi_s + \\varphi_s\\cos\\varphi_s$. For small $\\lvert\\varphi_s\\rvert$ the total width is $\\psi \\approx 3\\lvert\\varphi_s\\rvert$.
- The energy half-height, at $\\varphi = \\varphi_s$, is

$$
\\Delta W_{\\max} = 2\\left[\\frac{q E_0 T\\,\\beta_s^3\\gamma_s^3\\,\\lambda\\, mc^2\\left(\\varphi_s\\cos\\varphi_s - \\sin\\varphi_s\\right)}{2\\pi}\\right]^{1/2}.
$$

With the average gradient 1.86 MV/m of the previous section and the example beam energy:

| $\\varphi_s$ | $\\varphi_1$ | Phase width | $\\Delta W_{\\max}$ / MeV |
|---|---|---|---|
| −20° | −40.3° | 60° | ±0.073 |
| −30° | −60.9° | 91° | ±0.133 |
| −33° | −67.2° | 100° | ±0.153 |

The larger $\\lvert\\varphi_s\\rvert$, the larger the acceptance, but the acceleration per cavity $\\propto\\cos\\varphi_s$ drops; low-energy sections usually use a larger $\\lvert\\varphi_s\\rvert$ (around −30° to −40°) to secure the acceptance and reduce it gradually at higher energy. The longitudinal beam size should fill only part of the separatrix, so that the non-linear region does not cause emittance growth and losses.

> Note: the separatrix is derived for a periodic structure smoothed into a continuum. For a beamline with only one or two cavities, or a low-energy section with large energy gain per cavity, it is only a qualitative estimate; the real longitudinal acceptance must be found with multi-particle simulations (scans of entry phase and energy).

## RF defocusing {#longitudinal-rf-defocusing}

### Derivation

Near the axis of an axially symmetric TM mode, $\\nabla\\cdot\\boldsymbol E = 0$ and the Ampère–Maxwell law give

$$
E_r = -\\frac{r}{2}\\frac{\\partial E_z}{\\partial z},\\qquad
B_\\theta = \\frac{r}{2c^2}\\frac{\\partial E_z}{\\partial t} .
$$

The radial force is $F_r = q(E_r - v B_\\theta)$. Using the total derivative along the trajectory, $\\dd E_z/\\dd z = \\partial_z E_z + \\partial_t E_z/(\\beta c)$:

$$
F_r = -\\frac{q r}{2}\\left[\\frac{\\dd E_z}{\\dd z} - \\frac{1}{\\beta c\\gamma^2}\\frac{\\partial E_z}{\\partial t}\\right].
$$

Integrated over the gap, the total-derivative term vanishes at the field-free ends (assuming $r$ and $\\beta$ roughly constant in the gap), and the remaining term gives

$$
\\Delta p_r = -\\frac{\\pi q E_0 T L\\, r\\sin\\varphi}{c\\,\\beta^2\\gamma^2\\lambda},\\qquad
\\Delta r' = \\frac{\\Delta p_r}{p} = -\\frac{\\pi q E_0 T L\\sin\\varphi}{mc^2\\,\\beta^3\\gamma^3\\lambda}\\, r .
\\tag{7}
$$

For $\\sin\\varphi < 0$ (longitudinally stable) $\\Delta r'$ has the sign of $r$: **the gap is a defocusing lens** with $1/f = \\pi q E_0 T L\\sin(-\\varphi)/(mc^2\\beta^3\\gamma^3\\lambda)$. Longitudinal focusing and transverse defocusing come from the same $\\sin\\varphi$; one gap cannot provide both at the same phase.

### Relation to longitudinal focusing

Averaging (7) over unit length and comparing with (6):

$$
k_{r}^2 = -\\frac{\\pi q E_0 T\\sin(-\\varphi_s)}{mc^2\\,\\beta^3\\gamma^3\\lambda} = -\\frac{k_{l0}^2}{2} .
$$

This is Earnshaw's theorem (Laplace's equation) in the beam rest frame: in a source-free region the focusing strengths of the three directions add up to zero, so longitudinal focusing is necessarily accompanied by defocusing in both transverse planes. In the numerical example above $k_r^2 = -9.9\\ \\mathrm{m^{-2}}$, which solenoids or quadrupoles must overcome (see [Solenoids](#solenoid), [Quadrupoles](#quadrupole)). The more general Panofsky–Wenzel theorem relates the transverse kick to the transverse gradient of the longitudinal kick and says the same: an energy gain that varies with transverse position comes with a transverse deflection.

The magnetic term $vB_\\theta$ is about $\\beta^2$ times the electric term and small at low energy: in the example, changing \`Kb\` from −1.36 to 0 changes the exit rms $x$ of the linear preview by only about 0.01 % and rms $y$ by about 0.2 %. The $B$ components of a map should still be given consistently with $E$, from the same mode.

## In AVAS {#longitudinal-avas}

### Keywords

An RF cavity is a \`field\` element with \`type = 1\`:

\`\`\`avas-params
field input:scanphase input:longlimits beam:twissz beam:frequency
\`\`\`

\`\`\`text
start
! L     R     V3 type f        phase Ke   Kb    file
field 0.21  0.02  0  1    162.5e6  -33   1.36 -1.36 hwr010
end
\`\`\`

- \`f\` is the cavity frequency; \`frequency\` in \`beam.txt\` is the beam (bunch repetition) frequency, which sets the bunch charge $Q = I/f$. The two may differ (for example after a frequency jump).
- \`Ke\` scales the electric field (map in MV/m) and \`Kb\` the magnetic field; when changing the gradient of a cavity scale both by the same factor.
- \`scanphase\` in \`input.txt\` controls the phase scan: 0 no scan, 1 scan the phases, 2 read the entry phase and entry time of every cavity from \`scanData.txt\`.
- The output file \`Phase.txt\` gives, in two lines per RF cavity, time, position and synchronous-particle energy at its entry and exit, which lets you check the energy gain of every cavity.

### Longitudinal model of the linear preview

The linear preview integrates the reference particle's energy and time along the axis in 1 mm steps, iterates the entry phase of V3 = 0 cavities, and pushes the second moments with the linear bunching term in $(z, \\delta)$ and the damping from the momentum gain. It contains no longitudinal non-linearity and therefore cannot show the growth of the longitudinal emittance in low-energy cavities; energies agree to about 0.01 %, rms sizes usually to 1–2 %. See [Multi-particle simulation in practice](#simulation).

### Common problems

- **Phase sign**: when phases are taken from another code with the opposite sign, the bunch debunches and is transversely focused instead; compare with the table above.
- **Cavity $\\beta$ does not match the beam**: when $T(\\beta)$ is small the cavity hardly accelerates and the energy is very sensitive to the phase; check first that the energy gain is reasonable.
- **V3 = 2 cavities lose their phase after upstream changes**: changing a drift length or the upstream energy changes $t_{\\mathrm{in}}$ and with it the synchronous phase; use V3 = 0 during design.
- **Large phase width**: a bunch whose phase width approaches the separatrix width develops longitudinal tails and losses; shorten the incoming bunch or increase $\\lvert\\varphi_s\\rvert$.

## Further reading {#longitudinal-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: transit-time factor, longitudinal dynamics (separatrix, adiabatic damping) and RF defocusing.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: longitudinal motion in linear accelerators.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific: synchrotron motion and phase stability.
- CERN Accelerator School lecture notes on linear accelerators ([CAS](https://cas.web.cern.ch/)).
- In this primer: [RF cavities](#rf-cavity), [Field maps](#field-maps), [Space charge](#space-charge) and the [tutorial](#tutorial).
`,y=`# 纵向动力学

射频腔给粒子能量，同时决定束团在纵向（时间 / 相位）上能否保持聚集。本章从单个加速间隙的能量增益出发，引入渡越时间因子和多间隙腔，然后讨论同步粒子、相稳定原理、小振幅同步振荡、纵向接受度、绝热阻尼以及与纵向聚焦必然相伴的射频散焦。最后说明 AVAS 中射频相位的三种定义（\`field\` 的 V3 参数）和相关关键字。运动学量（$\\beta$、$\\gamma$、$\\beta\\lambda$）的定义见 [相对论运动学](#kinematics)，相空间与发射度见 [相空间与发射度](#phase-space)，射频腔元件本身的写法见 [射频腔](#rf-cavity)。

本章的数值例子都用示例项目 \`examples/hwr010\`：质子，入口动能 $W = 1.527\\ \\mathrm{MeV}$（$\\beta = 0.05698$，$\\gamma = 1.00163$），射频频率 $f = 162.5\\ \\mathrm{MHz}$（$\\lambda = c/f = 1.8449\\ \\mathrm{m}$，$\\beta\\lambda = 0.1051\\ \\mathrm{m}$），半波长谐振腔（HWR）场图 \`hwr010\`，长 0.21 m，\`Ke = 1.36\`。

## 射频间隙中的能量增益 {#longitudinal-gap}

### 场与相位

设腔内轴上纵向电场为驻波

$$
E_z(z, t) = E(z)\\cos\\left(\\omega t + \\varphi_0\\right),
$$

$E(z)$ 是场的空间分布，$\\omega = 2\\pi f$。粒子沿轴运动，到达 $z$ 的时刻为 $t(z)$，它在 $z$ 处"看到"的相位是

$$
\\varphi(z) = \\omega t(z) + \\varphi_0 .
$$

在这个约定下，**相位越大表示粒子越晚到达**。粒子穿过间隙得到的能量增益是

$$
\\Delta W = q\\int E(z)\\cos\\varphi(z)\\,\\dd z .
\\tag{1}
$$

> AVAS 约定：射频电场按 $K_e\\,E(\\boldsymbol r)\\cos(\\omega t + \\varphi_0)$ 作用，射频磁场按 $K_b\\,B(\\boldsymbol r)\\sin(\\omega t + \\varphi_0)$ 作用（场图中 $E$ 单位 MV/m，$B$ 单位 T）。这与 Wangler《RF Linear Accelerators》的余弦约定一致。纵向坐标 $z$ 与相位的关系为 $\\Delta\\varphi = -2\\pi z/(\\beta\\lambda)$，即 $z > 0$ 的粒子在参考粒子前方、相位较小。

### Panofsky 公式与渡越时间因子

假设粒子在间隙内速度变化很小（$\\Delta W \\ll W$），可以把 $t(z)$ 近似为匀速：$\\omega t(z) = \\omega t_c + k (z - z_c)$，$k = 2\\pi/(\\beta\\lambda)$，$z_c$ 是间隙的某个参考点。代入 (1)：

$$
\\Delta W = q\\cos\\varphi\\int E(z)\\cos k(z - z_c)\\,\\dd z - q\\sin\\varphi\\int E(z)\\sin k(z - z_c)\\,\\dd z ,
$$

其中 $\\varphi = \\omega t_c + \\varphi_0$ 是粒子到达 $z_c$ 时的相位。把 $z_c$ 选在**电中心**，使第二个积分为零（对称间隙即几何中心），就得到 Panofsky 公式

$$
\\Delta W = q E_0 T L\\cos\\varphi = q V_0 T\\cos\\varphi ,
\\qquad
T = \\frac{\\int E(z)\\cos k(z - z_c)\\,\\dd z}{\\int E(z)\\,\\dd z},
\\tag{2}
$$

$E_0 = V_0/L$ 是间隙长度 $L$ 上的平均轴上场，$V_0 = \\int E\\,\\dd z$ 是轴上电压。**渡越时间因子** $T \\le 1$ 描述粒子在场变化的时间内穿过间隙造成的损失。对长 $g$ 的均匀场间隙，

$$
T = \\frac{\\sin(\\pi g/\\beta\\lambda)}{\\pi g/\\beta\\lambda},
$$

间隙越短于 $\\beta\\lambda$，$T$ 越接近 1。实际腔的场在间隙外有延伸，$T$ 要用场分布积分。

### 准确定义：沿实际轨迹的相位

当能量增益与入口能量可比时（低能质子、重离子），匀速近似不再成立。此时把 (1) 写成

$$
C = \\int E(z)\\cos\\varphi(z)\\,\\dd z,\\quad S = \\int E(z)\\sin\\varphi(z)\\,\\dd z,\\quad
\\Delta W = q\\sqrt{C^2 + S^2}\\,\\cos\\varphi_s,\\quad \\varphi_s = \\operatorname{atan2}(S, C),
\\tag{3}
$$

积分沿参考粒子的**实际**轨迹 $t(z)$ 进行。式 (3) 是精确的，$\\varphi_s$ 就是 AVAS 所说的同步相位（见 [下文](#longitudinal-synchronous)），但"有效电压" $\\sqrt{C^2 + S^2}$ 本身依赖于轨迹，因而依赖于 $\\varphi_s$，能量增益不再严格按 $\\cos\\varphi_s$ 变化。

## 渡越时间因子与多间隙腔 {#longitudinal-multigap}

### 两间隙腔

半波长谐振腔（HWR）、四分之一波长腔（QWR）的加速部分是两个间隙，场方向相反（$\\pi$ 模）。把单个间隙的场记为 $e(z)$，两个间隙中心相距 $d$：$E(z) = e(z - d/2) - e(z + d/2)$。其傅里叶变换给出

$$
T(\\beta) = T_g(\\beta)\\,\\left\\lvert\\sin\\frac{k d}{2}\\right\\rvert = T_g(\\beta)\\,\\left\\lvert\\sin\\frac{\\pi\\beta_g}{2\\beta}\\right\\rvert,
\\qquad d = \\frac{\\beta_g\\lambda}{2},
$$

$T_g$ 是单个间隙的渡越时间因子。对正负交替的多间隙场 $\\int E\\,\\dd z \\approx 0$，此时式 (2) 的分母改用 $V_0 = \\int\\lvert E\\rvert\\,\\dd z$（下表即如此）。粒子从一个间隙飞到另一个间隙恰好用半个射频周期时（$\\beta = \\beta_g$），两个间隙同相加速。$N$ 个间隙的腔 $T(\\beta)$ 更尖锐，能量接受范围更窄；低 $\\beta$ 超导直线加速器因此用间隙少的腔，并分成几种几何 $\\beta$。

### 示例腔 hwr010

对 \`hwr010\` 场图（乘以 \`Ke = 1.36\`）的轴上 $E_z$ 积分得到：

| 量 | 数值 |
|---|---|
| 轴上场峰值 | $+13.03\\ \\mathrm{MV/m}$ 与 $-12.74\\ \\mathrm{MV/m}$ |
| 场反号位置 | $z \\approx 0.104\\ \\mathrm{m}$（两个间隙） |
| 两间隙场的重心 | $0.064\\ \\mathrm{m}$、$0.145\\ \\mathrm{m}$，间距 $d = 0.081\\ \\mathrm{m}$ |
| 由间距得到的 $\\beta_g = 2d/\\lambda$ | 0.088 |
| $V_0 = \\int\\lvert E_z\\rvert\\,\\dd z$ | 0.978 MV |
| $T(\\beta)$ 的最大值 | 0.82，在 $\\beta \\approx 0.104$ |
| 入口 $\\beta = 0.0570$ 处的 $T$ | 0.40，$V_0 T = 0.39\\ \\mathrm{MV}$ |

$T(\\beta)$ 最大值处的 $\\beta \\approx 0.10$ 与名字中的 "010"（通常表示设计 $\\beta = 0.10$）相符；它比由间距估计的 0.088 稍大，是因为单间隙的 $T_g(\\beta)$ 随 $\\beta$ 增大。示例束流的 $\\beta = 0.057$ 远低于最佳 $\\beta$，所以 $T$ 只有 0.40；粒子在腔内被加速后 $T$ 逐渐增大，这也是低能腔里匀速近似失效的原因之一。

\`\`\`avas-field
map hwr010
\`\`\`

### 能量增益与同步相位：示例

用 AVAS 的线性预览（\`avas.sim.linear_optics\`，lattice 为 \`field 0.21 0.02 0 1 162.5e6 φs 1.36 -1.36 hwr010\`）计算出口能量：

| $\\varphi_s$（V3 = 0） | 入口相位 | 出口能量 / MeV | $\\Delta W$ / MeV |
|---|---|---|---|
| −90° | −14.0° | 1.5270 | 0.000 |
| −60° | 18.6° | 1.6943 | 0.167 |
| −33° | 52.7° | 1.8665 | 0.340 |
| −20° | 69.7° | 1.9374 | 0.410 |
| 0° | 95.4° | 2.0023 | 0.475 |
| +20° | 119.7° | 1.9954 | 0.468 |

$\\varphi_s = -90°$ 时能量增益为零，符合 (3)。但 $\\Delta W/\\cos\\varphi_s$ 在 −33° 时为 0.405 MeV，在 0° 时为 0.475 MeV，相差约 15 %，而且用入口相位扫描时最大能量增益（2.0087 MeV）出现在 $\\varphi_s \\approx +8°$ 而不是 0°。原因就是本节开头说的：$\\Delta W/W \\approx 30\\ \\%$，$\\beta$ 在腔内从 0.057 增大到约 0.065，有效电压随相位改变。对这种腔，Panofsky 公式只能作估算，设计要以沿轨迹积分的结果为准。

## 同步粒子与同步相位 {#longitudinal-synchronous}

**同步粒子**（参考粒子）是设计上沿轴运动、在每个腔都处在设计相位的粒子；其他粒子的纵向坐标都是相对它定义的。在一系列腔组成的直线加速器中，同步粒子到达第 $n$ 个腔时的相位 $\\varphi_{s,n}$ 由腔的射频相位和它在前面各段的飞行时间共同决定。

### AVAS 中相位的三种含义（V3）

\`field L R V3 type f phase Ke Kb file\` 的第 3 个参数 V3 决定第 6 个参数 \`phase\` 的含义：

| V3 | \`phase\` 的含义 | 内核如何确定腔的入口相位 $\\varphi_0$ |
|---|---|---|
| 0 | 同步相位 $\\varphi_s$ | 求解 $\\varphi_0$，使沿参考粒子轨迹的 $\\operatorname{atan2}(S, C) = \\varphi_s$，见式 (3) |
| 1 | 参考粒子到达元件入口时的射频相位 | $\\varphi_0$ = \`phase\` |
| 2 | $t = 0$ 时刻的射频相位（绝对相位） | $\\varphi_0 = $ \`phase\` $+ 360°\\cdot f\\cdot t_{\\mathrm{in}}$，$t_{\\mathrm{in}}$ 是参考粒子到达入口的时刻 |

V3 = 0 最方便设计：物理上有意义的是同步相位，入口相位由程序反求。V3 = 1 适合做相位扫描（入口相位与腔的几何直接对应）。V3 = 2 对应真实机器上"每个腔的射频相对公共参考时钟的相位"，上游元件改变时同步相位会跟着变。

例：示例腔前加一段 0.1 m 漂移，参考粒子到达腔入口的时刻 $t_{\\mathrm{in}} = 0.1/(\\beta c) = 5.854\\ \\mathrm{ns}$，$360°\\,f\\,t_{\\mathrm{in}} = 342.45°$。V3 = 2、\`phase = 0\` 时入口相位为 $342.45° \\equiv -17.55°$，对应同步相位约 −93.7°——这个腔几乎不加速。而 V3 = 0、\`phase = -33\` 时，无论前面漂移多长，入口相位都是 52.7°，同步相位保持 −33°。

> AVAS 约定：示例中同步相位 $-33°$ 对应的入口相位是 $52.7°$，两者之差（约 86°）取决于场图的零点位置和场分布，没有通用换算；对 V3 = 0 的腔，线性预览会给出求得的入口相位与同步相位。分段运行把一段 lattice 单独拿出来时，AVAS（\`avas/data/segment.py\` 的 \`rephase\`）从参考运行的同步粒子数据取每个腔的射频相位 $\\varphi_{\\mathrm{RF}}$，按 $\\varphi_{\\mathrm{RF}} - 360°\\cdot f\\cdot(t_{\\mathrm{in}} - T_{\\mathrm{entry}})$ 换算成以分段入口时刻为零点的 V3 = 2 相位，而不是沿用原来的相位或改成 V3 = 0。

## 相稳定原理 {#longitudinal-stability}

### 运动方程

考虑一列腔组成的周期结构，把每个腔的作用平均到单位长度上：平均加速梯度 $E_0 T$（腔内 $E_0 T$ 乘以腔长占周期长度的比例）。定义相对同步粒子的偏离

$$
\\Delta\\varphi = \\varphi - \\varphi_s,\\qquad w = \\frac{W - W_s}{mc^2} .
$$

由 $\\dd t/\\dd s = 1/(\\beta c)$ 和 $\\Delta\\beta = \\Delta\\gamma/(\\beta\\gamma^3)$：

$$
\\frac{\\dd\\,\\Delta\\varphi}{\\dd s} = \\omega\\left(\\frac{1}{\\beta c} - \\frac{1}{\\beta_s c}\\right) \\approx -\\frac{2\\pi}{\\beta_s^3\\gamma_s^3\\lambda}\\,w \\equiv -A\\,w ,
\\tag{4}
$$

能量方程

$$
\\frac{\\dd w}{\\dd s} = \\frac{q E_0 T}{mc^2}\\left(\\cos\\varphi - \\cos\\varphi_s\\right) \\equiv B\\left(\\cos\\varphi - \\cos\\varphi_s\\right).
\\tag{5}
$$

式 (4) 说明能量高的粒子（$w > 0$）跑得快，相位减小（提前到达）。

### 稳定条件

对 (4)、(5) 线性化，$\\cos\\varphi - \\cos\\varphi_s \\approx -\\sin\\varphi_s\\,\\Delta\\varphi$：

$$
\\frac{\\dd^2\\Delta\\varphi}{\\dd s^2} = A B \\sin\\varphi_s\\,\\Delta\\varphi .
$$

$A, B > 0$，所以**只有 $\\sin\\varphi_s < 0$ 时运动稳定**。结合 $\\cos\\varphi_s > 0$（要加速），稳定加速区间是

$$
-90° < \\varphi_s < 0° .
$$

物理图像：晚到的粒子（$\\Delta\\varphi > 0$）处在场正在上升的一侧，得到更多能量，速度变大而追上；早到的粒子得到较少能量而落后。

> 注意：符号约定因文献而异。在 AVAS 和 Wangler 的余弦约定（相位越大越晚）下稳定同步相位为负，示例项目用 $-33°$。若采用正弦约定或把相位定义为 $-\\omega t$，稳定区会变成正相位。从其他程序或文献移植相位时务必核对约定。

### 用 AVAS 的线性预览核对符号

同样用示例束流（初始 rms $z$ = 1.316 mm，rms $x$ = 0.954 mm）穿过一个 hwr010 腔，出口处的 rms 尺寸（线性预览，不计空间电荷）：

| 情况 | rms $z$ / mm | rms $x$ / mm |
|---|---|---|
| 0.21 m 漂移（无腔） | 1.482 | 1.407 |
| $\\varphi_s = -33°$ | 0.702 | 1.811 |
| $\\varphi_s = +33°$ | 1.738 | 1.196 |

负同步相位使束团纵向压缩（聚束），正同步相位使束团拉长（散束），与上面的稳定条件一致；横向的情况正好相反，这就是 [射频散焦](#longitudinal-rf-defocusing)。可以在下面的实验里拖动相位亲自验证。

\`\`\`avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
\`\`\`

## 小振幅同步振荡与绝热阻尼 {#longitudinal-synchrotron}

### 纵向振荡波数

稳定时 $\\Delta\\varphi$ 做简谐振荡 $\\Delta\\varphi'' + k_{l0}^2\\,\\Delta\\varphi = 0$，**零流强纵向波数**为

$$
k_{l0}^2 = -A B\\sin\\varphi_s = \\frac{2\\pi\\, q E_0 T\\sin(-\\varphi_s)}{mc^2\\,\\beta_s^3\\gamma_s^3\\,\\lambda} .
\\tag{6}
$$

同步振荡的波长 $\\lambda_l = 2\\pi/k_{l0}$，每个聚焦周期 $L_p$ 的纵向相移 $\\sigma_{0l} = k_{l0}L_p$。由 (4)，能量振幅与相位振幅之比 $w_{\\max}/\\Delta\\varphi_{\\max} = k_{l0}/A$。式 (6) 中的 $\\beta^3\\gamma^3$ 说明：纵向聚焦随能量迅速变弱，低能段的同步振荡最快。

数值例：若一列腔的平均梯度等于示例腔在入口处的 $V_0 T / L = 0.39\\ \\mathrm{MV}/0.21\\ \\mathrm{m} = 1.86\\ \\mathrm{MV/m}$，取 $\\varphi_s = -33°$，则 $k_{l0} = 4.44\\ \\mathrm{m}^{-1}$，$\\lambda_l = 1.41\\ \\mathrm{m}$。实际结构中腔之间还有螺线管和漂移，平均梯度更低，$k_{l0}$ 相应减小（$\\propto\\sqrt{E_0 T}$）。

### 与 AVAS 纵向坐标的换算

AVAS 的纵向相空间用 $(z, \\delta)$，$\\delta = \\Delta p/p$：

$$
z = -\\frac{\\beta\\lambda}{2\\pi}\\,\\Delta\\varphi,\\qquad
\\delta = \\frac{\\Delta W}{\\beta^2\\gamma\\,mc^2} = \\frac{w}{\\beta^2\\gamma} .
$$

因此 $z$ 与 $\\Delta\\varphi$ 满足同样的振荡方程。\`beam.txt\` 的 \`twissz\` 用 $z$（mm）和 $\\delta$（mrad），发射度同样是归一化 rms 值。示例束流为 \`twissz 0.04275 0.37292 0.26506\`，对应初始 rms $z = 1.316\\ \\mathrm{mm}$、rms $\\delta = 3.53\\ \\mathrm{mrad}$；Twiss 参数的含义见 [相空间与发射度](#phase-space)。

### 绝热阻尼

当加速使 $\\beta\\gamma$、$E_0 T$ 缓慢变化时，$(\\Delta\\varphi, w)$ 是一对正则变量（哈密顿量见 [下一节](#longitudinal-separatrix)），作用量 $\\oint w\\,\\dd\\Delta\\varphi \\propto \\Delta\\varphi_{\\max} w_{\\max}$ 是绝热不变量。结合 $w_{\\max}/\\Delta\\varphi_{\\max} = k_{l0}/A$ 得到

$$
\\Delta\\varphi_{\\max} \\propto \\left[E_0 T\\sin(-\\varphi_s)\\,\\lambda\\,\\beta_s^3\\gamma_s^3\\right]^{-1/4},\\qquad
\\Delta W_{\\max} \\propto \\left[E_0 T\\sin(-\\varphi_s)\\,\\lambda\\,\\beta_s^3\\gamma_s^3\\right]^{1/4}.
$$

梯度、相位和频率不变时 $\\Delta\\varphi \\propto (\\beta\\gamma)^{-3/4}$，$\\Delta W \\propto (\\beta\\gamma)^{3/4}$：相位宽度缩小，能散增大，乘积不变。在 $(z, \\delta)$ 中，$\\beta\\gamma\\cdot z\\,\\delta \\propto \\lambda\\,\\Delta\\varphi\\,\\Delta W$ 不变，即**归一化纵向发射度守恒**，横向的情况相同（几何发射度 $\\propto 1/\\beta\\gamma$），见 [相空间与发射度](#phase-space)。

> 注意：上面的守恒只对线性、无耦合、无空间电荷的运动成立。低能腔中的纵向非线性（相位宽度达到几十度时 $\\cos\\varphi$ 的弯曲）会使纵向 rms 发射度增长，线性预览不包含这一效应，要用多粒子内核计算。

## 纵向接受度与分界线 {#longitudinal-separatrix}

### 哈密顿量

方程 (4)、(5) 可以由哈密顿量（以 $s$ 为自变量，$\\varphi$ 与 $w$ 共轭）

$$
H(\\varphi, w) = -\\frac{A w^2}{2} - B\\left(\\sin\\varphi - \\varphi\\cos\\varphi_s\\right)
$$

导出：$\\varphi' = \\partial H/\\partial w$，$w' = -\\partial H/\\partial\\varphi$。$H$ 守恒，相轨迹就是 $H$ 的等值线。

### 不动点与分界线

$w = 0$ 且 $\\cos\\varphi = \\cos\\varphi_s$ 给出两个不动点：$\\varphi = \\varphi_s$（稳定，椭圆点）和 $\\varphi_2 = -\\varphi_s$（不稳定，双曲点）。经过 $\\varphi_2$ 的等值线是**分界线**（separatrix）：

$$
\\frac{A w^2}{2} + B\\left(\\sin\\varphi - \\varphi\\cos\\varphi_s\\right) = -B\\left(\\sin\\varphi_s - \\varphi_s\\cos\\varphi_s\\right).
$$

分界线内的粒子做有界的同步振荡，构成纵向**接受度**（"鱼形"或"高尔夫球杆形"的稳定区）；分界线外的粒子相位不断滑移，最终离开加速相位，成为纵向损失（在 AVAS 中可以用 \`longlimits\` 设置相位和能量边界来判定）。

### 稳定区的宽度和高度

- 相位范围从 $\\varphi_1$ 到 $\\varphi_2 = -\\varphi_s$，$\\varphi_1$ 由 $\\sin\\varphi_1 - \\varphi_1\\cos\\varphi_s = -\\sin\\varphi_s + \\varphi_s\\cos\\varphi_s$ 求出。小 $\\lvert\\varphi_s\\rvert$ 时总宽度 $\\psi \\approx 3\\lvert\\varphi_s\\rvert$。
- 能量半高度在 $\\varphi = \\varphi_s$ 处：

$$
\\Delta W_{\\max} = 2\\left[\\frac{q E_0 T\\,\\beta_s^3\\gamma_s^3\\,\\lambda\\, mc^2\\left(\\varphi_s\\cos\\varphi_s - \\sin\\varphi_s\\right)}{2\\pi}\\right]^{1/2}.
$$

用上一节的平均梯度 1.86 MV/m、示例束流能量计算：

| $\\varphi_s$ | $\\varphi_1$ | 相位宽度 | $\\Delta W_{\\max}$ / MeV |
|---|---|---|---|
| −20° | −40.3° | 60° | ±0.073 |
| −30° | −60.9° | 91° | ±0.133 |
| −33° | −67.2° | 100° | ±0.153 |

$\\lvert\\varphi_s\\rvert$ 越大，接受度越大，但每个腔的加速 $\\propto\\cos\\varphi_s$ 越小；低能段通常取较大的 $\\lvert\\varphi_s\\rvert$（−30° 到 −40° 左右）保证接受度，高能段再逐渐减小。束流的纵向尺寸应只占分界线面积的一部分，以免非线性区引起发射度增长和损失。

> 注意：分界线是对"平均到连续"的周期结构推出的。在只有一两个腔的束线、或每个腔能量增益很大的低能段，它只能作定性估计，真实的纵向接受度要用多粒子模拟（扫描入口相位和能量）确定。

## 射频散焦 {#longitudinal-rf-defocusing}

### 推导

轴附近的轴对称 TM 模式中，由 $\\nabla\\cdot\\boldsymbol E = 0$ 和安培–麦克斯韦定律：

$$
E_r = -\\frac{r}{2}\\frac{\\partial E_z}{\\partial z},\\qquad
B_\\theta = \\frac{r}{2c^2}\\frac{\\partial E_z}{\\partial t} .
$$

径向力 $F_r = q(E_r - v B_\\theta)$。利用沿粒子轨迹的全导数 $\\dd E_z/\\dd z = \\partial_z E_z + \\partial_t E_z/(\\beta c)$ 整理：

$$
F_r = -\\frac{q r}{2}\\left[\\frac{\\dd E_z}{\\dd z} - \\frac{1}{\\beta c\\gamma^2}\\frac{\\partial E_z}{\\partial t}\\right].
$$

对整个间隙积分时，全导数项在场为零的两端消失（假设 $r$、$\\beta$ 在间隙内近似不变），剩下的项给出

$$
\\Delta p_r = -\\frac{\\pi q E_0 T L\\, r\\sin\\varphi}{c\\,\\beta^2\\gamma^2\\lambda},\\qquad
\\Delta r' = \\frac{\\Delta p_r}{p} = -\\frac{\\pi q E_0 T L\\sin\\varphi}{mc^2\\,\\beta^3\\gamma^3\\lambda}\\, r .
\\tag{7}
$$

$\\sin\\varphi < 0$（纵向稳定）时 $\\Delta r'$ 与 $r$ 同号，**间隙是散焦透镜**，焦距 $1/f = \\pi q E_0 T L\\sin(-\\varphi)/(mc^2\\beta^3\\gamma^3\\lambda)$。纵向聚焦和横向散焦来自同一个 $\\sin\\varphi$，二者不能在同一相位同时得到。

### 与纵向聚焦的关系

把 (7) 平均到单位长度，和 (6) 比较：

$$
k_{r}^2 = -\\frac{\\pi q E_0 T\\sin(-\\varphi_s)}{mc^2\\,\\beta^3\\gamma^3\\lambda} = -\\frac{k_{l0}^2}{2} .
$$

这是静电学 Earnshaw 定理（拉普拉斯方程）在束流静止系中的体现：在无源区域，三个方向的聚焦强度之和为零，纵向聚焦必然伴随两个横向方向的散焦。上一节的数值例中 $k_r^2 = -9.9\\ \\mathrm{m^{-2}}$，必须由螺线管或四极铁提供更强的横向聚焦（见 [螺线管](#solenoid)、[四极铁](#quadrupole)）。更一般的 Panofsky–Wenzel 定理把横向冲量与纵向冲量的横向梯度联系起来，同样说明纵向能量增益随横向位置的变化必然伴随横向偏转。

磁场项 $vB_\\theta$ 与电场项之比约为 $\\beta^2$，低能时很小：在示例中把 \`Kb\` 从 −1.36 改为 0，线性预览的出口 rms $x$ 只改变约 0.01 %，rms $y$ 约 0.2 %。场图的 $B$ 分量仍应与 $E$ 按同一个模式一致地给出。

## 在 AVAS 中 {#longitudinal-avas}

### 关键字

射频腔是 \`field\` 元件，\`type = 1\`：

\`\`\`avas-params
field input:scanphase input:longlimits beam:twissz beam:frequency
\`\`\`

\`\`\`text
start
! L     R     V3 type f        phase Ke   Kb    file
field 0.21  0.02  0  1    162.5e6  -33   1.36 -1.36 hwr010
end
\`\`\`

- \`f\` 是腔的频率；\`beam.txt\` 的 \`frequency\` 是束流频率（束团重复频率，决定每个束团的电荷 $Q = I/f$），二者可以不同（例如倍频段）。
- \`Ke\` 缩放电场（场图单位 MV/m），\`Kb\` 缩放磁场；改变腔的加速梯度时两者按比例一起改。
- \`input.txt\` 的 \`scanphase\` 控制相位扫描：0 不扫描，1 扫描相位，2 从 \`scanData.txt\` 读取各腔的入口相位和入口时间。
- 输出文件 \`Phase.txt\` 每两行给出一个射频腔入口和出口的时间、位置和同步粒子能量，可用来核对每个腔的能量增益。

### 线性预览中的纵向模型

线性预览按 1 mm 步长沿轴积分参考粒子的能量和时间，对 V3 = 0 的腔迭代求出入口相位，再以 $(z, \\delta)$ 的线性聚束项和动量增加带来的阻尼推进二阶矩。它不包含纵向非线性，因而不能给出低能腔中纵向发射度的增长；能量的误差约 0.01 %，rms 尺寸通常差 1–2 %。见 [多粒子模拟要点](#simulation)。

### 常见问题

- **相位符号**：从其他程序移植时同步相位符号相反，束团会散开、横向反而聚焦；对照上面的表格判断。
- **腔的 $\\beta$ 与束流不匹配**：$T(\\beta)$ 很小时腔几乎不加速，且相位对能量非常敏感；先看能量增益是否合理。
- **V3 = 2 的腔在上游改动后失相**：漂移长度或上游能量改变会改变 $t_{\\mathrm{in}}$，同步相位随之改变；设计阶段宜用 V3 = 0。
- **大相位宽度**：束团相位宽度接近分界线宽度时会出现纵向尾巴和损失，需要缩短入口束团或加大 $\\lvert\\varphi_s\\rvert$。

## 延伸阅读 {#longitudinal-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：渡越时间因子、纵向动力学（分界线、绝热阻尼）与射频散焦。
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：直线加速器中的纵向运动。
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific：同步振荡与相稳定。
- CERN Accelerator School 直线加速器课程讲义（[CAS](https://cas.web.cern.ch/)）。
- 本入门的 [射频腔](#rf-cavity)、[场图](#field-maps)、[空间电荷](#space-charge) 和 [教程](#tutorial)。
`,b=`# Beam matching

A periodic structure has its own "eigen" phase ellipse — the periodic Twiss parameters. When the phase ellipse of the incoming beam agrees with it, the beam envelope repeats every period; when it does not, the envelope oscillates around the periodic envelope (mismatch), and nonlinear forces and space charge turn this into emittance growth and halo. This chapter gives the definition of matching, the mismatch factor, the envelope oscillation modes, the number of knobs a match needs, and how to match in AVAS.

The notation follows [phase space](#phase-space) and [linear optics](#linear-optics): Twiss parameters $(\\alpha, \\beta, \\gamma)$ with $\\beta\\gamma - \\alpha^2 = 1$, rms geometric emittance $\\varepsilon$, beam sigma matrix

$$
\\Sigma = \\varepsilon \\begin{pmatrix} \\beta & -\\alpha \\\\ -\\alpha & \\gamma \\end{pmatrix},\\qquad x_{\\mathrm{rms}} = \\sqrt{\\beta\\varepsilon}
$$

## Definition of a matched beam {#matching-definition}

### The zero-current periodic solution

Let $M$ be the 2×2 transfer matrix of one period with $\\cos\\sigma = \\tfrac12\\Tr M$ and $\\lvert\\cos\\sigma\\rvert < 1$. Writing $M$ in Twiss form

$$
M = \\begin{pmatrix} \\cos\\sigma + \\alpha\\sin\\sigma & \\beta\\sin\\sigma \\\\ -\\gamma\\sin\\sigma & \\cos\\sigma - \\alpha\\sin\\sigma \\end{pmatrix}
$$

gives the periodic solution:

$$
\\beta = \\frac{M_{12}}{\\sin\\sigma},\\qquad \\alpha = \\frac{M_{11} - M_{22}}{2\\sin\\sigma},\\qquad \\gamma = -\\frac{M_{21}}{\\sin\\sigma}
$$

The sign of $\\sin\\sigma$ is taken equal to that of $M_{12}$ so that $\\beta > 0$. These $(\\alpha, \\beta)$ satisfy $M\\Sigma M\\tp = \\Sigma$: a beam with these Twiss parameters has the same phase ellipse after one period — the **matched beam**. The periodic solution depends on where the period is taken to start; at the centre of a FODO F quadrupole $\\alpha = 0$ and $\\beta$ is maximal, at the centre of the D quadrupole $\\alpha = 0$ and $\\beta$ is minimal.

### With space charge

The space-charge force depends on the beam size itself, so the matched solution must be self-consistent. For a continuous beam (KV model, edge radii $a$, $b$, $a = 2x_{\\mathrm{rms}}$) the envelope equations are

$$
a'' + k_x(s)\\,a - \\frac{2K}{a+b} - \\frac{\\varepsilon_x^2}{a^3} = 0,\\qquad
b'' + k_y(s)\\,b - \\frac{2K}{a+b} - \\frac{\\varepsilon_y^2}{b^3} = 0
$$

with $\\varepsilon$ the edge emittance ($4\\varepsilon_{\\mathrm{rms}}$) and $K$ the generalised perveance (see [space charge](#space-charge)). The matched beam is the solution of these equations with period $L_p$; it changes with the current, and zero-current matched parameters applied to an intense beam produce a mismatch. Bunched beams are similar in three dimensions, and the longitudinal envelope must be matched as well.

### With acceleration

Acceleration increases $\\beta\\gamma$, the geometric emittance shrinks adiabatically as $1/(\\beta\\gamma)$, and the periodic solution also changes slowly with energy. "Matched" then means that the envelope follows the slowly varying periodic solution rather than repeating exactly from period to period. The Twiss parameters in AVAS's \`beam.txt\` are the values **at the entrance** (β in mm/mrad, emittance normalised rms in π·mm·mrad); the entrance rms size is $\\sqrt{\\beta\\varepsilon_n/(\\beta\\gamma)}$.

## The mismatch factor {#matching-mismatch-factor}

To compare a beam $(\\alpha, \\beta, \\gamma)$ with the matched ellipse $(\\alpha_0, \\beta_0, \\gamma_0)$ (same place, same emittance) define

$$
R = \\beta\\gamma_0 + \\beta_0\\gamma - 2\\alpha\\alpha_0,\\qquad
M = \\left[\\frac{1}{2}\\left(R + \\sqrt{R^2 - 4}\\right)\\right]^{1/2} - 1
$$

This is the definition used in Wangler's book and common envelope codes. Properties:

- $R \\ge 2$, with $R = 2$ and $M = 0$ if and only if the two ellipses coincide.
- $R = \\Tr(S S_0^{-1})$ with $S = \\begin{pmatrix}\\beta & -\\alpha\\\\ -\\alpha & \\gamma\\end{pmatrix}$. In linear transport $S \\to MSM\\tp$ and the trace is invariant, so **in linear transport without space charge the mismatch factor is conserved along the line** and can be evaluated anywhere.
- Geometric meaning: in the normalised coordinates of the matched ellipse the matched ellipse is a circle of radius $\\sqrt{\\varepsilon}$ and the mismatched ellipse has semi-axes $\\sqrt{\\lambda_\\pm\\varepsilon}$, $\\lambda_\\pm = \\tfrac12(R \\pm \\sqrt{R^2-4})$, $\\lambda_+\\lambda_- = 1$ (equal areas). So $1 + M = \\sqrt{\\lambda_+}$ is the factor by which the mismatched ellipse exceeds the matched one in the worst direction; enclosing the mismatched beam in an ellipse of the matched shape needs an emittance $(1+M)^2\\varepsilon$.
- Without space charge the envelope of a mismatched beam beats between $1/(1+M)$ and $1+M$ times the matched envelope.

Examples (at a place with $\\alpha_0 = \\alpha = 0$): $\\beta = 1.2\\beta_0$ gives $R = 1.2 + 1/1.2$ and $M = \\sqrt{1.2} - 1 = 0.095$; $\\beta = 1.44\\beta_0$ (beam 20 % too large) gives $M = 0.200$. With $\\beta = \\beta_0$ but $\\alpha$ off by 0.5, $M = 0.281$ — errors in α matter just as much.

A useful small-error result: after a thin-lens error $\\Delta(1/f)$ (for example a quadrupole gradient error) a matched beam keeps $\\beta$ while $\\alpha \\to \\alpha + \\beta\\,\\Delta(1/f)$; substituting gives $R = 2 + \\beta^2\\Delta(1/f)^2$ and therefore

$$
M \\approx \\frac{\\beta\\,\\Delta(1/f)}{2}
$$

Empirically $M \\lesssim 0.05$–$0.1$ counts as a good match, but the acceptable value depends on the current and the aperture margin (see [errors and correction](#errors)).

## Envelope oscillation modes {#matching-envelope-modes}

Linearising the envelope equations around the matched solution gives the eigenmodes of the mismatch oscillation. For uniform focusing $k_0^2$ and a round beam $a = b$, $\\varepsilon_x = \\varepsilon_y$, the matching condition is $k_0^2 a = K/a + \\varepsilon^2/a^3$; write $k = \\varepsilon/a^2$ for the depressed particle wave number:

- **breathing mode** ($\\delta a = \\delta b$, planes in phase): $k_{\\mathrm{b}}^2 = 2k_0^2 + 2k^2$;
- **quadrupole mode** ($\\delta a = -\\delta b$, planes in antiphase): $k_{\\mathrm{q}}^2 = k_0^2 + 3k^2$.

Outline of the derivation: for an antisymmetric perturbation $2K/(a+b)$ is unchanged to first order, so the quadrupole mode feels only $k_0^2$ and the emittance term $3\\varepsilon^2/a^4 = 3k^2$; for a symmetric perturbation the space-charge term contributes $K/a^2 = k_0^2 - k^2$. At zero current $k = k_0$ and both modes are at $2k_0$; in the extreme space-charge limit $k \\to 0$, $k_{\\mathrm{b}} \\to \\sqrt2\\,k_0$ and $k_{\\mathrm{q}} \\to k_0$. Axisymmetric bunched beams have three modes in three dimensions: the transverse quadrupole mode, and a high-frequency and a low-frequency mode in which the transverse breathing and the longitudinal oscillation are coupled.

Physical consequences of a mismatch:

1. The envelope frequency differs from the particle frequency, so particles are driven periodically by the oscillating core; nonlinear space-charge forces filament the phase space and the rms emittance eventually grows. Reiser's "free energy" picture estimates the emittance growth when the mismatch energy is fully converted; larger mismatch and stronger space charge give more growth.
2. In periodic focusing, the envelope instability occurs when an envelope mode advances by about $180^\\circ$ per period — the origin of the $\\sigma_0 < 90^\\circ$ rule of [lattice design](#lattice-design-phase-advance).
3. The parametric resonance between particles and the oscillating core drives some particles to large amplitudes and forms a halo (see [rms matching and halo](#matching-halo)).

## Matching sections: knobs and constraints {#matching-knobs}

### Number of constraints

A full 6D match must satisfy at the target point

| Plane | Constraints | Number |
|---|---|---|
| x | $\\alpha_x$, $\\beta_x$ | 2 |
| y | $\\alpha_y$, $\\beta_y$ | 2 |
| z | $\\alpha_z$, $\\beta_z$ | 2 |

The emittances belong to the beam and are not matching variables. At least as many **independent** knobs as constraints are therefore needed:

- Transversely with quadrupoles: at least 4 (four gradients) to match $(\\alpha_x, \\beta_x, \\alpha_y, \\beta_y)$; in practice 4–6 are common, the extra freedom being used to limit the maximum β, the gradients and the sensitivity to errors.
- A solenoid acts identically on both planes: one solenoid changes x and y together, a round beam has only the two constraints $(\\alpha, \\beta)$, so at least 2 solenoids are needed, and they cannot match x and y to different values.
- Longitudinally with bunchers: a buncher at $\\varphi_s = -90^\\circ$ gives no energy gain, only an energy offset proportional to the phase offset — a longitudinal thin lens; at least 2 bunchers (amplitudes as knobs) match $(\\alpha_z, \\beta_z)$. The first cavities of the downstream accelerating section can be used too.

### Practical rules

- Avoid large β inside the matching section: the beam is large there and more sensitive to quadrupole displacements and gradient errors (mismatch $M \\approx \\beta\\,\\Delta(1/f)/2$).
- With space charge the matched parameters depend on the current; iterate at the actual current, using the zero-current result only as a start.
- The target is the periodic Twiss parameters of the downstream period at its entrance, not an arbitrary "small spot".
- The nonlinear problem can have several solutions; check that the envelope through the whole matching section is reasonable, not only the end parameters.

## Matching between sections {#matching-sections}

The general procedure for matching between sections:

1. Find the periodic matched parameters at the entrance of the downstream section (with the formulas above at zero current; with self-consistent envelopes or multi-particle simulations with space charge).
2. Determine the Twiss parameters of the upstream beam at the entrance of the matching section (from upstream simulations or measurements).
3. Use the knobs of the matching section to transform 2 into 1 within the gradient and aperture limits.
4. Check with a multi-particle simulation that the envelope is periodic over several downstream periods and that the emittance does not grow.

The typical case is the MEBT between the RFQ exit and the first accelerating section: the beam leaving an RFQ usually converges or diverges strongly and differently in the two planes, and bunchers are needed to control the bunch length. If the [lattice design](#lattice-design-transitions) keeps the phase advance per metre continuous, the periodic parameters of the two sections are close and the matching section can be short or even omitted.

> Note: in an accelerating section the "periodic solution" of the first downstream period itself changes with energy, so strictly periodic matched parameters do not exist. One usually matches to the local periodic solution of the first period at its entrance energy and checks with simulations.

## rms matching and halo {#matching-halo}

rms matching only makes the second moments (rms sizes, Twiss parameters) right. The tails of the real distribution are also affected by:

- **Particle–core resonance**: a mismatched core oscillates at the envelope breathing frequency and a particle passing through it feels a periodic nonlinear force; particles whose oscillation frequency is about half the core frequency are resonantly driven to amplitudes well beyond the core. The particle–core model (Gluckstern; Wangler et al.) gives a maximum halo amplitude that grows with the mismatch, approximately logarithmically; any mismatch lets an intense beam form a halo.
- **Nonlinear external fields**: higher-order components in field maps and the fringe fields of solenoids and cavities distort the phase ellipse.
- **Initial distribution**: Gaussian tails already extend several rms sizes out, a KV distribution has no tails; the choice of KV / GS / PB / WB in AVAS's \`distribution\` affects loss estimates.

In AVAS the direct halo indicators are the ratio of maximum to rms size (DataSet columns 22/24 against 16/18), the emittance growth along the line and the surviving macro-particles (column 28). A good rms match with a steadily growing max/rms ratio usually points to nonlinear or resonance problems.

## Matching in AVAS {#matching-avas}

### Setting the entrance beam

The Beam page sets \`twissx\`, \`twissy\`, \`twissz\` (α, β, ε); units:

\`\`\`avas-params
beam:twissx beam:twissy beam:twissz beam:distribution
\`\`\`

β is in mm/mrad (numerically m/rad), ε is the normalised rms emittance (π·mm·mrad), and twissz uses z (mm) and $z' = \\Delta p/p$ (mrad). The example beam \`twissx -0.65785 0.37743 0.13768\` has an entrance $x_{\\mathrm{rms}} = \\sqrt{0.37743 \\times 0.13768/0.05708} = 0.954\\ \\mathrm{mm}$.

### Interactively: the linear preview

When you drag quadrupole gradients, solenoid \`Kb\` and other parameters in the visual editor, the linear envelope preview updates live (tick "Space charge in the preview" to include linear space charge). The visual criterion for a match: in a periodic section the envelope repeats every period without a period-to-period beat. The preview is an approximate model (rms usually within 1–2 %, up to 13 % with strong multi-particle effects) without losses or nonlinearities; confirm with the kernel.

### Scans and optimisation

- **The Parameter scan page** (or the command line \`avas scan --input DIR --target ELEMENT --param PARAM --values …\`): runs the kernel for a list of values of one parameter in copies under \`Scans/\` and compares the results.
- **The AI assistant's \`optimize\` tool**: varies parameters of several elements within bounds to minimise or maximise an objective. Metrics available with the preview engine are \`rms_x_max\`, \`rms_y_max\`, \`rms_x_out\`, \`rms_y_out\`, \`energy_out\` and \`rms_mismatch_out\`; the objective may be an expression such as \`abs(rms_x_out-1.5)+abs(rms_y_out-1.5)\`. Note that \`rms_mismatch_out\` is the end value of $\\lvert x_{\\mathrm{rms}} - y_{\\mathrm{rms}}\\rvert$ (roundness), not the Twiss mismatch factor of this chapter. The simulation engine adds metrics such as transmission and emittance growth. The result is proposed as an edit and is written to the lattice only after the user approves it.

### Size targets with adjust and diag_size

The static-error correction can also be used to tune parameters for beam sizes (details in [errors and correction](#errors-correction)):

\`\`\`avas-params
adjust diag_size
\`\`\`

- \`adjust 0 v n min max first_step\` goes in front of the element to tune; \`v\` is the v-th parameter of that element (counted from 1 after the keyword: the gradient of \`quad\` is the 4th, \`Kb\` of \`field\` the 8th and \`Ke\` the 7th), knobs with the same \`n\` share one value, and \`first_step = 1\` starts the optimisation from the current value.
- \`diag_size 0 sx sy 0\` gives the target **rms** sizes (mm) at its position. The current code pairs \`adjust\` and \`diag_*\` by their **first parameter** (equal numbers form one correction, done in order of the number); the manual describes this parameter as "meaningless, write 0", and writing 0 everywhere makes one group.
- The optimiser is SLSQP (every function evaluation is one kernel run); the objective is the mean of $(x_{\\mathrm{rms}} - s_x)^2 + (y_{\\mathrm{rms}} - s_y)^2$ (mm²) over the group's diagnostics; it stops below 0.05, after at most 100 iterations, with a finite-difference step of 0.1 (in the parameter's own unit).
- The mechanism runs only in a **static error study** (\`error_type\` \`stat\` or \`stat_dyn\` in \`ini.ini\`) and needs \`err_step\` and at least one error switch turned on (a static one in \`stat\` mode). The results are written to \`Adjust_Datas_<group>_<run>.txt\` and are **not written back to the lattice**.

> Note: a single \`diag_size\` constrains only sizes, not α, and cannot fix a match; size targets at two places about $90^\\circ$ of phase advance apart constrain $(\\alpha, \\beta)$ indirectly. Using zero-amplitude static errors to borrow this mechanism for an error-free match is inferred from the current code and has not been validated systematically with the kernel; check the result of every run.

### Checking

- Results page: rms envelope, Twiss α/β along the line, emittances; the "Phase advance" plot (needs \`lattice … lattice_end\` period markers) gives the phase advance of the actual beam as $\\int\\dd z/\\beta$ per period.
- The Run page shows the rms envelope live; "compare with the previous run" overlays the previous run to compare before and after matching.

> AVAS convention: the old matching commands described in the manual (\`MATCHING\`, \`SETTWISS\`, \`CIRCLE_MATCH\`, …) belong to a legacy workflow that has not been verified in the current GUI; do not use them as a current feature.

## Hands-on {#matching-lab}

In the FODO lab set \`mismatch\` to 1 (matched), 1.2 and 1.44 and watch the envelope beat around the periodic envelope. \`mismatch\` is the initial size scale factor $s$: the lab multiplies β at the cell entrance by $s^2$ and keeps α. At a place with $\\alpha = 0$ this would give $M = s - 1$; at the cell entrance $\\alpha \\approx -1.76$, so the same scaling is a larger mismatch: $M \\approx 0.44$ for $s = 1.2$ and $M \\approx 1.0$ for $s = 1.44$.

\`\`\`avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1.2
\`\`\`

Then raise the DC current \`I\` to 10–20 mA: the zero-current periodic Twiss parameters are no longer matched and the envelope begins to beat, with periods corresponding to the breathing and quadrupole modes of [envelope oscillation modes](#matching-envelope-modes).

\`\`\`avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=15 mismatch=1
\`\`\`

In the phase-space lab send an ellipse through a thin lens and a drift to see how α and β together determine the downstream size:

\`\`\`avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0.5 L=0.5
\`\`\`

## Further reading {#matching-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: mismatch factor, envelope modes, the particle–core model of halo.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.: envelope equations, free energy of mismatch and emittance growth.
- F. J. Sacherer (1971): rms envelope equations, which make rms matching valid for any elliptically symmetric distribution.
- J. Struckmeier, M. Reiser (1984): envelope oscillations and instabilities of mismatched beams in periodic channels.
- The full bibliography is in [references](#references).
`,x=`# 束流匹配

周期结构有自己的"本征"相椭圆——周期 Twiss 参数。入口束流的相椭圆与它一致时，束流包络在每个周期重复；不一致时包络在周期包络附近振荡（失配），在非线性力和空间电荷作用下转化为发射度增长和束晕。本章给出匹配的定义、失配因子、包络振荡模式、匹配所需的旋钮数，以及在 AVAS 中做匹配的方法。

记号沿用 [相空间](#phase-space) 与 [线性光学](#linear-optics)：Twiss 参数 $(\\alpha, \\beta, \\gamma)$ 满足 $\\beta\\gamma - \\alpha^2 = 1$，rms 几何发射度 $\\varepsilon$，束流 σ 矩阵

$$
\\Sigma = \\varepsilon \\begin{pmatrix} \\beta & -\\alpha \\\\ -\\alpha & \\gamma \\end{pmatrix},\\qquad x_{\\mathrm{rms}} = \\sqrt{\\beta\\varepsilon}
$$

## 匹配束的定义 {#matching-definition}

### 零流周期解

设一个周期的 2×2 传输矩阵为 $M$，$\\cos\\sigma = \\tfrac12\\Tr M$ 且 $\\lvert\\cos\\sigma\\rvert < 1$。把 $M$ 写成 Twiss 形式

$$
M = \\begin{pmatrix} \\cos\\sigma + \\alpha\\sin\\sigma & \\beta\\sin\\sigma \\\\ -\\gamma\\sin\\sigma & \\cos\\sigma - \\alpha\\sin\\sigma \\end{pmatrix}
$$

即可读出周期解：

$$
\\beta = \\frac{M_{12}}{\\sin\\sigma},\\qquad \\alpha = \\frac{M_{11} - M_{22}}{2\\sin\\sigma},\\qquad \\gamma = -\\frac{M_{21}}{\\sin\\sigma}
$$

$\\sin\\sigma$ 的符号取与 $M_{12}$ 相同，使 $\\beta > 0$。这组 $(\\alpha, \\beta)$ 满足 $M\\Sigma M\\tp = \\Sigma$：以它为 Twiss 参数的束流经过一个周期后相椭圆不变，这就是**匹配束**。周期解依赖于取周期起点的位置；在 FODO 的 F 铁中心 $\\alpha = 0$、$\\beta$ 最大，D 铁中心 $\\alpha = 0$、$\\beta$ 最小。

### 有空间电荷时

空间电荷力依赖于束流尺寸本身，匹配解必须自洽。对连续束（KV 模型，$a$、$b$ 为边缘半径，$a = 2x_{\\mathrm{rms}}$）包络方程为

$$
a'' + k_x(s)\\,a - \\frac{2K}{a+b} - \\frac{\\varepsilon_x^2}{a^3} = 0,\\qquad
b'' + k_y(s)\\,b - \\frac{2K}{a+b} - \\frac{\\varepsilon_y^2}{b^3} = 0
$$

其中 $\\varepsilon$ 为边缘发射度（$4\\varepsilon_{\\mathrm{rms}}$），$K$ 为广义导流系数（见 [空间电荷](#space-charge)）。匹配束是这对方程具有周期 $L_p$ 的解，随流强改变；零流匹配参数用在强流束上会产生失配。束团束的三维情况类似，纵向包络也要匹配。

### 加速时

加速使 $\\beta\\gamma$ 增大，几何发射度按 $1/(\\beta\\gamma)$ 绝热减小，周期解也随能量缓慢变化。此时"匹配"指包络跟随缓变的周期解，而不是严格逐周期重复。AVAS 的 \`beam.txt\` 中 Twiss 参数是**入口处**的值（β 单位 mm/mrad，发射度为归一化 rms，π·mm·mrad），入口 rms 尺寸为 $\\sqrt{\\beta\\varepsilon_n/(\\beta\\gamma)}$。

## 失配因子 {#matching-mismatch-factor}

比较束流 $(\\alpha, \\beta, \\gamma)$ 与匹配椭圆 $(\\alpha_0, \\beta_0, \\gamma_0)$（同一位置、相同发射度），定义

$$
R = \\beta\\gamma_0 + \\beta_0\\gamma - 2\\alpha\\alpha_0,\\qquad
M = \\left[\\frac{1}{2}\\left(R + \\sqrt{R^2 - 4}\\right)\\right]^{1/2} - 1
$$

这是 Wangler 书中及常用包络程序采用的定义。性质：

- $R \\ge 2$，当且仅当两椭圆重合时 $R = 2$、$M = 0$。
- $R = \\Tr(S S_0^{-1})$，其中 $S = \\begin{pmatrix}\\beta & -\\alpha\\\\ -\\alpha & \\gamma\\end{pmatrix}$。线性输运中 $S \\to MSM\\tp$，迹不变，所以**无空间电荷的线性输运中失配因子沿束线守恒**，可以在任意位置计算。
- 几何意义：在匹配椭圆的归一化坐标中，匹配椭圆是半径 $\\sqrt{\\varepsilon}$ 的圆，失配椭圆的两个半轴为 $\\sqrt{\\lambda_\\pm\\varepsilon}$，$\\lambda_\\pm = \\tfrac12(R \\pm \\sqrt{R^2-4})$，$\\lambda_+\\lambda_- = 1$（面积相同）。所以 $1 + M = \\sqrt{\\lambda_+}$ 是失配椭圆在最不利方向上超出匹配椭圆的倍数；要用匹配形状的椭圆完全包住失配束，需要 $(1+M)^2\\varepsilon$ 的发射度。
- 无空间电荷时，失配束的包络在匹配包络的 $1/(1+M)$ 到 $1+M$ 倍之间拍振。

例（$\\alpha_0 = \\alpha = 0$ 处）：$\\beta = 1.2\\beta_0$ 时 $R = 1.2 + 1/1.2$，$M = \\sqrt{1.2} - 1 = 0.095$；$\\beta = 1.44\\beta_0$（尺寸大 20 %）时 $M = 0.200$。若 $\\beta = \\beta_0$ 而 $\\alpha$ 偏 0.5，$M = 0.281$——α 的误差同样重要。

一个常用的小量结果：匹配束经过一个薄透镜误差 $\\Delta(1/f)$（例如四极铁梯度误差）后，$\\beta$ 不变、$\\alpha \\to \\alpha + \\beta\\,\\Delta(1/f)$，代入得 $R = 2 + \\beta^2\\Delta(1/f)^2$，于是

$$
M \\approx \\frac{\\beta\\,\\Delta(1/f)}{2}
$$

经验上 $M \\lesssim 0.05$–$0.1$ 被视为良好匹配，但允许值取决于流强和孔径裕量（见 [误差与校正](#errors)）。

## 包络振荡模式 {#matching-envelope-modes}

把包络方程在匹配解附近线性化，可得到失配振荡的本征模。以均匀聚焦 $k_0^2$、圆束 $a = b$、$\\varepsilon_x = \\varepsilon_y$ 为例，匹配条件 $k_0^2 a = K/a + \\varepsilon^2/a^3$，记有空间电荷时粒子的波数 $k = \\varepsilon/a^2$：

- **呼吸模**（$\\delta a = \\delta b$，两平面同相）：$k_{\\mathrm{b}}^2 = 2k_0^2 + 2k^2$；
- **四极模**（$\\delta a = -\\delta b$，两平面反相）：$k_{\\mathrm{q}}^2 = k_0^2 + 3k^2$。

推导要点：反相扰动下 $2K/(a+b)$ 一阶不变，所以四极模只受 $k_0^2$ 和发射度项 $3\\varepsilon^2/a^4 = 3k^2$ 作用；同相扰动下空间电荷项贡献 $K/a^2 = k_0^2 - k^2$。零流时 $k = k_0$，两模都为 $2k_0$；空间电荷极强时 $k \\to 0$，$k_{\\mathrm{b}} \\to \\sqrt2\\,k_0$，$k_{\\mathrm{q}} \\to k_0$。轴对称束团束的三维情况有三个模：横向四极模，以及由横向呼吸与纵向振荡耦合而成的高频模和低频模。

失配的物理后果：

1. 包络振荡频率与粒子频率不同，粒子在振荡的核周围受到周期驱动；非线性空间电荷力使相空间丝化，最终 rms 发射度增长。Reiser 用"自由能"的观点估计失配能量全部转化时的发射度增长，失配越大、空间电荷越强，增长越大。
2. 在周期聚焦中，包络模式每周期相移接近 $180^\\circ$ 时发生包络不稳定性——这就是 [聚焦周期设计](#lattice-design-phase-advance) 中 $\\sigma_0 < 90^\\circ$ 规则的来源。
3. 粒子与振荡核的参数共振把部分粒子推到大振幅，形成束晕（见 [rms 匹配与束晕](#matching-halo)）。

## 匹配段：旋钮与约束 {#matching-knobs}

### 约束数

完整的 6D 匹配要求在目标位置满足

| 平面 | 约束 | 个数 |
|---|---|---|
| x | $\\alpha_x$, $\\beta_x$ | 2 |
| y | $\\alpha_y$, $\\beta_y$ | 2 |
| z | $\\alpha_z$, $\\beta_z$ | 2 |

发射度由束流本身决定，不是匹配变量。因此至少需要与约束数相同的**独立**旋钮：

- 横向用四极铁：至少 4 块（4 个梯度）匹配 $(\\alpha_x, \\beta_x, \\alpha_y, \\beta_y)$；实际常用 4–6 块，多出的自由度用来限制最大 β、梯度上限和对误差的敏感度。
- 螺线管对两个平面作用相同：一块螺线管只能同时改变 x、y 两个平面，对圆束只有 $(\\alpha, \\beta)$ 两个约束，至少需要 2 块螺线管；它不能把 x、y 匹配到不同的值。
- 纵向用聚束腔：在 $\\varphi_s = -90^\\circ$ 运行的聚束腔不增能，只给出与相位偏差成正比的能量偏差，相当于纵向薄透镜；至少 2 个聚束腔（幅值为旋钮）匹配 $(\\alpha_z, \\beta_z)$。也可以用下游加速段的头几个腔。

### 实用规则

- 匹配段内 β 不宜过大：大 β 处束流大，对四极铁位移和梯度误差也更敏感（失配 $M \\approx \\beta\\,\\Delta(1/f)/2$）。
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
- **初始分布**：高斯分布的尾部本身就延伸到多个 rms 之外，KV 分布没有尾部；AVAS 的 \`distribution\` 选择 KV / GS / PB / WB 会影响损失估计。

在 AVAS 中，晕的直观指标是最大尺寸与 rms 尺寸之比（DataSet 第 22/24 列与第 16/18 列），以及沿线的发射度增长和存活粒子数（第 28 列）。rms 匹配良好但最大尺寸比持续增大，通常说明有非线性或共振问题。

## 在 AVAS 中做匹配 {#matching-avas}

### 设置入口束流

束流页设置 \`twissx\`、\`twissy\`、\`twissz\`（α、β、ε），单位见下表：

\`\`\`avas-params
beam:twissx beam:twissy beam:twissz beam:distribution
\`\`\`

β 单位 mm/mrad（数值上等于 m/rad），ε 为归一化 rms 发射度（π·mm·mrad），twissz 用 z（mm）与 $z' = \\Delta p/p$（mrad）。示例束流 \`twissx -0.65785 0.37743 0.13768\` 对应入口 $x_{\\mathrm{rms}} = \\sqrt{0.37743 \\times 0.13768/0.05708} = 0.954\\ \\mathrm{mm}$。

### 交互：线性预览

可视化编辑器中拖动四极铁梯度、螺线管 \`Kb\` 等参数时，线性包络预览实时更新（勾选"预览中计入空间电荷"可加入线性空间电荷）。判断匹配的直观标准：周期段内的包络每个周期重复，没有逐周期的拍振。预览是近似模型（rms 通常 1–2 %，多粒子效应强时可达 13 %），不考虑损失和非线性，最终要用内核确认。

### 扫描与优化

- **参数扫描页**（或命令行 \`avas scan --input DIR --target 元件 --param 参数 --values …\`）：对一个参数取一组值，在 \`Scans/\` 下的副本里逐个运行内核，比较各取值的结果。
- **AI 助手的 \`optimize\` 工具**：在给定范围内调多个元件参数，最小化或最大化目标。预览引擎可用的指标有 \`rms_x_max\`、\`rms_y_max\`、\`rms_x_out\`、\`rms_y_out\`、\`energy_out\`、\`rms_mismatch_out\`；目标可以是表达式，例如 \`abs(rms_x_out-1.5)+abs(rms_y_out-1.5)\`。注意 \`rms_mismatch_out\` 是末端 $\\lvert x_{\\mathrm{rms}} - y_{\\mathrm{rms}}\\rvert$（圆度），不是本章的 Twiss 失配因子。用模拟引擎时还有传输效率、发射度增长等指标。优化结果作为修改提案，用户批准后才写入 lattice。

### 用 adjust 和 diag_size 做尺寸约束

静态误差校正机制也可以用来按束流尺寸调参数（详见 [误差与校正](#errors-correction)）：

\`\`\`avas-params
adjust diag_size
\`\`\`

- \`adjust 0 v n min max first_step\` 放在要调的元件前，\`v\` 是该元件的第 v 个参数（关键字后从 1 数起：\`quad\` 的梯度是第 4 个，\`field\` 的 \`Kb\` 是第 8 个、\`Ke\` 是第 7 个），\`n\` 相同的旋钮取同一值，\`first_step = 1\` 表示从当前值开始优化。
- \`diag_size 0 sx sy 0\` 给出该位置的目标 **rms** 尺寸（mm）。当前代码按 \`adjust\` 与 \`diag_*\` 的**第一个参数**配对（相同编号组成一次校正，按编号顺序依次进行），手册把该参数写作"无意义，写 0"，全部写 0 即为一组。
- 优化器为 SLSQP（每次函数求值是一次内核运行），目标为该组所有束诊的平均 $(x_{\\mathrm{rms}} - s_x)^2 + (y_{\\mathrm{rms}} - s_y)^2$（mm²），低于 0.05 即停止，最多 100 次迭代，差分步长 0.1（参数自身单位）。
- 该机制只在**静态误差研究**（\`ini.ini\` 的 \`error_type\` 为 \`stat\` 或 \`stat_dyn\`）中执行，并要求有 \`err_step\` 和至少一个打开的误差开关（\`stat\` 模式下须为静态误差开关）。结果写在 \`Adjust_Datas_<组>_<次>.txt\`，**不会写回 lattice**。

> 注意：只约束尺寸、不约束 α 时，单个 \`diag_size\` 不能确定匹配；在相隔约 $90^\\circ$ 相移的两处放置尺寸目标，才能间接约束 $(\\alpha, \\beta)$。用零幅度的静态误差借用这一机制做无误差匹配，是根据当前代码推断的用法，尚未用内核系统验证，使用时要核对每次运行的结果。

### 检查

- 结果页：rms 包络、Twiss α/β 沿线曲线、发射度；"相移"图（需要 \`lattice … lattice_end\` 周期标记）按每周期 $\\int\\dd z/\\beta$ 给出实际束流的相移。
- 运行页实时显示 rms 包络；"与前一次运行对比"可以叠加上一次运行，比较匹配前后的差别。

> AVAS 约定：手册中记载的旧版 \`MATCHING\`、\`SETTWISS\`、\`CIRCLE_MATCH\` 等匹配命令属于旧流程，当前界面未验证，不应作为现行功能使用。

## 动手实验 {#matching-lab}

在 FODO 实验中把 \`mismatch\` 设为 1（匹配）、1.2、1.44，观察包络在周期包络附近的拍振。\`mismatch\` 是初始尺寸的放大倍数 $s$：实验把周期入口处的 β 乘以 $s^2$、α 保持不变。若在 $\\alpha = 0$ 的位置这样放大，$M = s - 1$；但周期入口处 $\\alpha \\approx -1.76$，同样的放大失配更大：$s = 1.2$ 时 $M \\approx 0.44$，$s = 1.44$ 时 $M \\approx 1.0$。

\`\`\`avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1.2
\`\`\`

再把直流流强 \`I\` 增加到 10–20 mA：零流的周期 Twiss 参数不再匹配，包络开始拍振；拍振周期与 [包络振荡模式](#matching-envelope-modes) 中的呼吸模和四极模相对应。

\`\`\`avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=15 mismatch=1
\`\`\`

在相空间实验中把一个椭圆通过薄透镜和漂移，体会 α 与 β 如何同时决定下游尺寸：

\`\`\`avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0.5 L=0.5
\`\`\`

## 延伸阅读 {#matching-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.：失配因子、包络模式、束晕的粒子–核模型。
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.：包络方程、失配的自由能与发射度增长。
- F. J. Sacherer (1971)：rms 包络方程，使 rms 匹配对椭圆对称分布普遍成立。
- J. Struckmeier, M. Reiser (1984)：周期通道中失配束的包络振荡与不稳定性。
- 完整书目见 [参考文献](#references)。
`,S=`# Phase space and emittance

A beam consists of a large number of particles, and the natural language to describe it is phase space: every particle is a point in phase space and the beam is a cloud of points. This chapter defines the six-dimensional coordinates used by AVAS, introduces Liouville's theorem, emittance, the Twiss (Courant–Snyder) parameters and the σ-matrix, gives the relation between the edge and rms emittances of common beam distributions, and finally maps these quantities term by term onto \`twissx\`, \`twissy\`, \`twissz\` of \`beam.txt\` (in particular their units).

Notation: in this chapter $\\beta$ and $\\gamma$ without subscript are the relativistic factors (see [Relativistic kinematics](#kinematics)); $\\beta_x$, $\\alpha_x$, $\\gamma_x$ with a plane subscript are Twiss parameters.

## Coordinates and the reference particle {#phase-space-coordinates}

The design orbit of the reference particle is the reference, and $s$ is the distance along it (on a straight section simply the longitudinal position $z$). The deviation of any particle from the reference particle is described by six quantities:

| Coordinate | Meaning | Unit (AVAS internal / matrices) |
|---|---|---|
| $x$, $y$ | transverse position deviation | m |
| $x' = \\dd x/\\dd s$, $y' = \\dd y/\\dd s$ | transverse angle (slope) | rad |
| $z$ | longitudinal position deviation, positive ahead of the reference particle | m |
| $\\delta = \\Delta p/p$ | relative momentum deviation | 1 |

In the paraxial approximation $x' \\approx p_x/p$, where $p$ is the reference momentum. Transfer matrices act on the column vector $(x, x', y, y', z, \\delta)\\tp$, see [Linear optics](#linear-optics).

Two descriptions must be kept apart:

- the distribution **at the same time**: the t-code (field-map elements, drifts) advances in time, and at a given time the particles sit at different longitudinal positions;
- the distribution **at the same longitudinal position**: the z-code (matrix elements) and transfer-matrix theory compare all particles at the same $s$.

For rms quantities the difference is usually small, but it matters for long bunches with a large energy spread (for example through the $z$–$\\delta$ correlation). According to the manual, \`DataSet.txt\` records the bunch parameters in the s frame.

## Liouville's theorem and adiabatic damping {#phase-space-liouville}

### Liouville's theorem

If the motion of the particles can be described by a Hamiltonian (external electromagnetic fields, and space charge in the mean-field approximation), the phase-space density in the six-dimensional phase space of canonical coordinates $(x, p_x, y, p_y, z, p_z)$ is constant along particle trajectories, and the phase-space volume occupied by any group of particles is conserved. If the three directions are uncoupled, the area of each two-dimensional projection is conserved separately.

Liouville's theorem does not guarantee that the "effective" emittance is conserved: non-linear forces (field non-linearities, non-linear space charge, the sinusoidal shape of the RF field) distort an ellipse in phase space into an S shape or a spiral. The occupied area stays the same, but the area of the ellipse needed to enclose it — and the rms emittance — grow. This is called **filamentation**. Particle losses, collisions, stripping and similar processes violate the premises of Liouville's theorem.

### Adiabatic damping and normalised emittance

The angle $x' = p_x/p$ is not a canonical momentum. Longitudinal acceleration increases $p$ without changing $p_x$, so $x'$ shrinks as $1/p$ and the area in the $(x, x')$ plane shrinks as $1/(\\beta\\gamma)$; this is **adiabatic damping**. The area in the $(x, p_x/mc) = (x, \\beta\\gamma x')$ plane is conserved. One therefore defines

$$
\\varepsilon_n = \\beta\\gamma\\,\\varepsilon
$$

$\\varepsilon$ is called the geometric emittance and $\\varepsilon_n$ the normalised emittance. In the ideal linear, uncoupled case without filamentation $\\varepsilon_n$ stays constant along a linac, which makes it the right figure of merit for comparing beams at different energies. Example: accelerating protons from 1.527 MeV to 10 MeV raises $\\beta\\gamma$ from 0.05708 to 0.14639, and the geometric emittance shrinks by a factor of 2.565.

> Note: the formula uses the $\\beta\\gamma$ of the reference particle. For beams with a very large energy spread, the normalised emittance computed with each particle's own $\\beta\\gamma$ differs from this, and the difference can be significant for low-energy electron beams.

## Definitions of emittance {#phase-space-emittance}

### rms emittance

For $N$ particles write the centred second moments $\\langle x^2\\rangle = \\frac1N\\sum (x - \\bar x)^2$, and likewise $\\langle x x'\\rangle$, $\\langle x'^2\\rangle$. The rms emittance is

$$
\\varepsilon_{\\mathrm{rms}} = \\sqrt{\\langle x^2\\rangle\\langle x'^2\\rangle - \\langle x x'\\rangle^2}
$$

It is the area of the "rms ellipse" in phase space divided by $\\pi$. The rms emittance has a well-defined value for any distribution and is strictly conserved under linear transport (without coupling); it grows under non-linear forces, so rms-emittance growth is the usual measure of beam-quality degradation.

### Total and percentage emittances

$\\varepsilon_{100\\%}$ (total or edge emittance) is the area of the smallest ellipse containing all particles divided by $\\pi$; $\\varepsilon_{90\\%}$, $\\varepsilon_{99\\%}$ etc. contain the corresponding fraction of particles. For ideal distributions they have fixed ratios to the rms emittance, see [Beam distributions](#phase-space-distributions).

### Units

Emittance has the dimension length × angle. It is customarily written in π·mm·mrad, where the "π" is only a label indicating that the number is the area divided by π:

$$
1\\ \\pi\\cdot\\mathrm{mm\\cdot mrad} \\;\\to\\; \\varepsilon = 1\\ \\mathrm{mm\\cdot mrad} = 1\\ \\mu\\mathrm{m} = 10^{-6}\\ \\mathrm{m\\cdot rad}
$$

When quoting an emittance always state three things: rms or total (percentage), geometric or normalised, and whether π has already been divided out of the number.

## Twiss parameters and the Courant–Snyder invariant {#phase-space-twiss}

A phase-space ellipse centred on the origin can be written as

$$
\\gamma_x x^2 + 2\\alpha_x x x' + \\beta_x x'^2 = \\varepsilon, \\qquad \\beta_x\\gamma_x - \\alpha_x^2 = 1
$$

$\\alpha_x$, $\\beta_x$, $\\gamma_x$ are the Twiss or Courant–Snyder parameters; the ellipse area is $\\pi\\varepsilon$. Only two of the three parameters are independent. For the rms ellipse they are related to the second moments by

$$
\\langle x^2\\rangle = \\beta_x\\varepsilon, \\qquad
\\langle x x'\\rangle = -\\alpha_x\\varepsilon, \\qquad
\\langle x'^2\\rangle = \\gamma_x\\varepsilon
$$

Conversely, the Twiss parameters of a particle distribution are $\\beta_x = \\langle x^2\\rangle/\\varepsilon_{\\mathrm{rms}}$ and $\\alpha_x = -\\langle x x'\\rangle/\\varepsilon_{\\mathrm{rms}}$.

### Geometry of the ellipse

- maximum position $x_{\\max} = \\sqrt{\\varepsilon\\beta_x}$, maximum angle $x'_{\\max} = \\sqrt{\\varepsilon\\gamma_x}$;
- intercept with the $x$ axis $x = \\sqrt{\\varepsilon/\\gamma_x}$, with the $x'$ axis $x' = \\sqrt{\\varepsilon/\\beta_x}$;
- the tilt $\\theta$ of the major axis against the $x$ axis satisfies $\\tan 2\\theta = 2\\alpha_x/(\\gamma_x - \\beta_x)$.

The sign of $\\alpha_x$ tells whether the beam converges or diverges:

| $\\alpha_x$ | $\\langle x x'\\rangle$ | State |
|---|---|---|
| $\\alpha_x > 0$ | $< 0$ | converging (moving towards a waist) |
| $\\alpha_x = 0$ | $= 0$ | waist (upright ellipse) |
| $\\alpha_x < 0$ | $> 0$ | diverging (moving away from a waist) |

### Courant–Snyder invariant

In a linear focusing system the Twiss parameters vary with $s$ (following the transport rules in [Linear optics](#linear-optics-twiss-transport)), while the single-particle quantity

$$
J = \\gamma_x(s)\\,x^2 + 2\\alpha_x(s)\\,x x' + \\beta_x(s)\\,x'^2
$$

stays constant along the beamline; it is called the Courant–Snyder invariant. A particle thus always lies on one member of a family of ellipses whose shape changes but whose area does not. The trajectory can be written $x(s) = \\sqrt{J\\beta_x(s)}\\cos\\left(\\psi(s) + \\psi_0\\right)$, where $\\psi$ is the betatron phase, see [Linear optics](#linear-optics-phase-advance).

## The σ-matrix and the phase ellipse {#phase-space-sigma}

Arrange the second moments in a symmetric matrix (the beam matrix or σ-matrix):

$$
\\Sigma = \\begin{pmatrix} \\langle x^2\\rangle & \\langle x x'\\rangle \\\\ \\langle x x'\\rangle & \\langle x'^2\\rangle \\end{pmatrix}
= \\varepsilon \\begin{pmatrix} \\beta_x & -\\alpha_x \\\\ -\\alpha_x & \\gamma_x \\end{pmatrix},
\\qquad \\varepsilon = \\sqrt{\\det\\Sigma}
$$

The rms ellipse is $X\\tp\\Sigma^{-1}X = 1$ with $X = (x, x')\\tp$. The advantage of the σ-matrix is its very simple transformation under linear transport $X_1 = M X_0$:

$$
\\Sigma_1 = M\\,\\Sigma_0\\,M\\tp
$$

This holds for any distribution (not only elliptical ones), since it involves only second moments. AVAS's linear preview pushes the beam in exactly this way.

### The six-dimensional σ-matrix

In six dimensions $\\Sigma_{ij} = \\langle u_i u_j\\rangle$ with $u = (x, x', y, y', z, \\delta)$. The diagonal blocks give the projected emittances of the three planes, the off-diagonal blocks describe correlations between planes:

- dipoles create $x$–$\\delta$ correlations (dispersion), see [Dipole](#dipole);
- solenoids couple the $x$ and $y$ planes; the projected emittances $\\varepsilon_x$, $\\varepsilon_y$ can then change, but the four-dimensional emittance $\\sqrt{\\det\\Sigma_4}$ is conserved under linear symplectic maps, see [Solenoid](#solenoid);
- RF cavities create $z$–$\\delta$ correlations, see [Longitudinal motion](#longitudinal).

## Beam distributions and edge emittance {#phase-space-distributions}

\`distribution V1 V2\` in \`beam.txt\` selects the transverse and the longitudinal distribution respectively, each one of KV, GS, PB, WB:

| Code | Distribution | Density in 4-D phase space ($r^2$ is the normalised "radius" squared) | $\\varepsilon_{100\\%}/\\varepsilon_{\\mathrm{rms}}$ (4-D distribution projected onto $x$–$x'$) |
|---|---|---|---|
| KV | Kapchinskij–Vladimirskij | concentrated on the 4-D ellipsoid surface $r^2 = 1$ | 4 |
| WB | waterbag | uniform inside $r^2 \\le 1$ | 6 |
| PB | parabolic | $\\propto 1 - r^2$, $r^2 \\le 1$ | 8 |
| GS | Gaussian | $\\propto \\exp(-r^2/2)$ | unbounded (needs truncation) |

The ratios follow from a general result: projecting a uniform distribution inside a $d$-dimensional ellipsoid onto one coordinate gives $\\langle x^2\\rangle = a^2/(d+2)$, with $a$ the semi-axis. Some remarks:

- The **KV distribution** projects onto a uniform ellipse in real space $(x, y)$; its space-charge force is exactly linear, which makes it the reference distribution of envelope equations and space-charge theory (see [Space charge](#space-charge)). Its phase-space structure is very special, however, and does not occur in real beams.
- **WB** and **PB** are closer to real beams and have a sharp edge; **GS** has the longest tails.
- For a two-dimensional Gaussian, the fraction of particles inside the rms-similar ellipse of emittance $n\\,\\varepsilon_{\\mathrm{rms}}$ is $1 - \\ee^{-n/2}$: 39.3 %, 86.5 %, 95.0 %, 98.2 % for $n$ = 1, 4, 6, 8.

> Note: the table gives the standard results for ideal four-dimensional distributions (Lapostolle, Reiser). How AVAS generates the longitudinal distribution and where it truncates the Gaussian is not described in the manual; if needed, read the \`inData.dst\` of a run (the initial distribution actually used) and compute the statistics directly.

## Longitudinal phase space {#phase-space-longitudinal}

The longitudinal coordinates $(z, \\delta)$ convert to the $(\\Delta\\varphi, \\Delta W)$ commonly used in linacs as (see [Kinematics](#kinematics-rf)):

$$
\\Delta\\varphi = -\\frac{2\\pi}{\\beta\\lambda}\\,z, \\qquad
\\Delta W = \\beta^2\\gamma\\,mc^2\\,\\delta
$$

so the two geometric emittances differ by a constant factor,

$$
\\varepsilon_{\\Delta\\varphi\\Delta W} = \\frac{2\\pi}{\\beta\\lambda}\\,\\beta^2\\gamma\\,mc^2\\;\\varepsilon_{z\\delta}
$$

(replace $2\\pi$ by $360$ for the phase in degrees). $\\lambda$ is the wavelength of the beam frequency. Motion in longitudinal phase space — synchronous phase, the stable region, longitudinal focusing — is discussed in [Longitudinal motion](#longitudinal).

## In AVAS {#phase-space-in-avas}

\`\`\`avas-params
beam:twissx beam:twissy beam:twissz beam:distribution beam:particlenumber beam:initpos beam:initmom
\`\`\`

The three numbers of \`twissx α β ε\` (likewise \`twissy\`, \`twissz\`) mean:

- $\\alpha$: dimensionless;
- $\\beta$: in mm/mrad, numerically equal to m/rad;
- $\\varepsilon$: the **normalised rms emittance** in π·mm·mrad.

> AVAS convention: the rms size is $\\sigma_x = \\sqrt{\\beta_x\\,\\varepsilon_n/(\\beta\\gamma)}$ ($\\sigma_x$ in mm for $\\beta_x$ in mm/mrad and $\\varepsilon_n$ in mm·mrad). \`twissz\` uses the same units and normalisation: $z$ in mm, $z' = \\delta = \\Delta p/p$ in mrad, normalised with the same factor $\\beta\\gamma$ (the interface's linear preview converts with this convention, which has been compared with the kernel). Other codes may give the longitudinal emittance in $(\\Delta\\varphi, \\Delta W)$ or with a different normalisation factor; check every item when exchanging data.

### Initial beam of the example project

The example \`beam.txt\` (protons at 1.527 MeV, $\\beta\\gamma$ = 0.057076):

\`\`\`text
twissx -0.65785 0.37743 0.13768
twissy -0.6341 0.37292 0.14059
twissz 0.04275 0.37292 0.26506
distribution GS GS
\`\`\`

Converted with the convention above:

| Plane | Geometric rms emittance (mm·mrad) | rms size | rms angle / momentum deviation |
|---|---|---|---|
| x | 2.412 | 0.954 mm | 3.026 mrad |
| y | 2.463 | 0.958 mm | 3.043 mrad |
| z | 4.644 | 1.316 mm | $\\delta_{\\mathrm{rms}}$ = 3.532 × 10$^{-3}$ |

The transverse $\\alpha < 0$ means the beam is diverging at the entrance. The longitudinal rms length of 1.316 mm corresponds to an rms phase width of 4.51° at 162.5 MHz; $\\delta_{\\mathrm{rms}}$ corresponds to an rms energy spread of 10.78 keV (0.71 %); the longitudinal geometric rms emittance converts to 48.5 deg·keV.

### Other keywords

- \`particlenumber\`: number of macro-particles; 1 selects single-particle mode (then \`twiss*\`, \`current\`, \`distribution\` have no effect).
- \`initpos\`, \`initmom\`: initial position (mm) and angle (mrad) offsets of the whole bunch, used to simulate injection errors.
- \`readparticledistribution\`: read the particle distribution from a \`.dst\`/\`.edst\` file; every keyword except \`numofcharge\` is then ignored (the interface switches with \`use_dst\`).

In the output, columns 16, 18, 20 of \`DataSet.txt\` are the rms sizes of $x$, $y$, $z$ (in m), corresponding directly to the $\\sigma$ above.

## Hands-on lab {#phase-space-lab}

The lab draws the rms ellipse before and after a thin lens (focal length \`f\` in m; 0 means no lens) followed by a drift of length \`L\`, and gives the Twiss parameters and rms size at the exit. \`beta\` is in mm/mrad and \`emit\` is the normalised rms emittance (π·mm·mrad).

1. Start at a waist ($\\alpha_x = 0$, $\\beta_x$ = 1 m), pass a lens of focal length 0.5 m and drift 0.5 m. By the formulas of [Linear optics](#linear-optics-twiss-transport), after the lens $\\alpha_x = \\beta_x/f = 2$, after the drift $\\beta_x = 0.25$ m and $\\alpha_x = -0.5$; the new waist lies $\\alpha_x/\\gamma_x$ = 0.4 m behind the lens with $\\beta^* = 1/\\gamma_x$ = 0.2 m. The rms size goes from 1.87 mm to 0.94 mm.
2. Change \`L\` to find where the ellipse is upright (the waist); change \`emit\` and confirm that the ellipse keeps its shape and is only scaled.
3. Increase \`W\`: the geometric emittance shrinks as $1/(\\beta\\gamma)$.

\`\`\`avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
\`\`\`

The initial x beam of the example project in a 0.1 m drift (a diverging beam, the size grows):

\`\`\`avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.1
\`\`\`

## Further reading {#phase-space-further}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.: emittance, Liouville's theorem, the KV and other distributions, rms-equivalent beams.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: transverse dynamics and Twiss parameters.
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1101: the introduction of the rms emittance concept.
- K. Floettmann, "Some basic features of the beam emittance", *Phys. Rev. ST Accel. Beams* 6, 034202 (2003): normalised emittance and energy spread.
- Continue with [Linear optics](#linear-optics).
`,C=`# 相空间与发射度

束流由大量粒子组成，描述它的自然语言是相空间：每个粒子是相空间中的一个点，束流是一团点云。本章定义 AVAS 使用的六维坐标，介绍刘维尔定理、发射度、Twiss（Courant–Snyder）参数和 σ 矩阵，说明常用束流分布的边缘发射度与 rms 发射度的关系，最后把这些量与 \`beam.txt\` 的 \`twissx\`、\`twissy\`、\`twissz\` 逐项对应（尤其是单位）。

记号说明：本章用不带下标的 $\\beta$、$\\gamma$ 表示相对论因子（见 [相对论运动学](#kinematics)），用带平面下标的 $\\beta_x$、$\\alpha_x$、$\\gamma_x$ 表示 Twiss 参数。

## 坐标与参考粒子 {#phase-space-coordinates}

取参考粒子的设计轨道为基准，$s$ 为沿轨道的距离（直线段上就是纵向位置 $z$）。任一粒子相对参考粒子的偏离用六个量描述：

| 坐标 | 含义 | 单位（AVAS 内部 / 矩阵） |
|---|---|---|
| $x$、$y$ | 横向位置偏离 | m |
| $x' = \\dd x/\\dd s$、$y' = \\dd y/\\dd s$ | 横向角度（斜率） | rad |
| $z$ | 纵向位置偏离，在参考粒子前方为正 | m |
| $\\delta = \\Delta p/p$ | 相对动量偏差 | 1 |

在近轴近似下 $x' \\approx p_x/p$，其中 $p$ 是参考动量。传输矩阵作用在列向量 $(x, x', y, y', z, \\delta)\\tp$ 上，见 [线性光学](#linear-optics)。

两种描述方式要区分清楚：

- **同一时刻**的分布：t-code（场图元件、漂移段）按时间推进，粒子在同一时刻位于不同的纵向位置；
- **同一纵向位置**的分布：z-code（矩阵元件）和传输矩阵理论把所有粒子放到同一个 $s$ 处比较。

对 rms 量来说两者的差别通常很小，但在束团较长、能散较大时要注意（例如 $z$ 与 $\\delta$ 的关联）。\`DataSet.txt\` 按手册说明记录的是 s 坐标系下的束团参数。

## 刘维尔定理与绝热阻尼 {#phase-space-liouville}

### 刘维尔定理

若粒子的运动可以由哈密顿量描述（外加电磁场，以及平均场近似下的空间电荷），则在正则坐标 $(x, p_x, y, p_y, z, p_z)$ 构成的六维相空间中，相空间密度沿粒子轨迹不变，任意一团粒子占据的相空间体积守恒。若三个方向的运动互不耦合，每个二维投影的面积也分别守恒。

刘维尔定理不保证"有效"发射度守恒：非线性力（场的非线性、非线性空间电荷、射频场的正弦形状）会把相空间中的椭圆扭曲成 S 形或螺旋，被占据的面积不变，但包住它所需的椭圆面积和 rms 发射度都会增大，这称为**丝化**（filamentation）。粒子损失、碰撞、剥离等过程不满足刘维尔定理的前提。

### 绝热阻尼与归一化发射度

角度 $x' = p_x/p$ 不是正则动量。纵向加速只增大 $p$ 而不改变 $p_x$，所以 $x'$ 按 $1/p$ 缩小，$(x, x')$ 平面的面积按 $1/(\\beta\\gamma)$ 缩小，这称为**绝热阻尼**。$(x, p_x/mc) = (x, \\beta\\gamma x')$ 平面的面积则守恒。因此定义

$$
\\varepsilon_n = \\beta\\gamma\\,\\varepsilon
$$

$\\varepsilon$ 称为几何发射度，$\\varepsilon_n$ 称为归一化发射度。在线性、无耦合、无丝化的理想情况下 $\\varepsilon_n$ 沿直线加速器保持不变，因此是比较不同能量处束流品质的合适指标。示例：质子从 1.527 MeV 加速到 10 MeV，$\\beta\\gamma$ 从 0.05708 增大到 0.14639，几何发射度缩小为原来的 1/2.565。

> 注意：上式使用参考粒子的 $\\beta\\gamma$。对能散很大的束流，逐个粒子用各自的 $\\beta\\gamma$ 计算的归一化发射度与此有差别，这一差别在低能电子束中可能显著。

## 发射度的定义 {#phase-space-emittance}

### rms 发射度

对 $N$ 个粒子，记中心化的二阶矩 $\\langle x^2\\rangle = \\frac1N\\sum (x - \\bar x)^2$，$\\langle x x'\\rangle$、$\\langle x'^2\\rangle$ 同理。rms 发射度定义为

$$
\\varepsilon_{\\mathrm{rms}} = \\sqrt{\\langle x^2\\rangle\\langle x'^2\\rangle - \\langle x x'\\rangle^2}
$$

它是相空间中"rms 椭圆"面积除以 $\\pi$。rms 发射度的优点是对任何分布都有确定的值，而且在线性传输下（无耦合时）严格守恒；在非线性力作用下它会增长，因此 rms 发射度增长是衡量束流品质恶化的常用指标。

### 总发射度与百分比发射度

$\\varepsilon_{100\\%}$（总发射度、边缘发射度）是包含全部粒子的最小椭圆面积除以 $\\pi$；$\\varepsilon_{90\\%}$、$\\varepsilon_{99\\%}$ 等则包含相应比例的粒子。对理想分布，它们与 rms 发射度有固定比值，见 [束流分布](#phase-space-distributions)。

### 单位

发射度的量纲是长度 × 角度。习惯写成 π·mm·mrad，其中的"π"只是一个标记，表示数值是面积除以 π：

$$
1\\ \\pi\\cdot\\mathrm{mm\\cdot mrad} \\;\\to\\; \\varepsilon = 1\\ \\mathrm{mm\\cdot mrad} = 1\\ \\mu\\mathrm{m} = 10^{-6}\\ \\mathrm{m\\cdot rad}
$$

引用发射度时必须说清三件事：rms 还是总（百分比）发射度，几何还是归一化，以及数值里是否已经除去了 π。

## Twiss 参数与 Courant–Snyder 不变量 {#phase-space-twiss}

一个以原点为中心的相空间椭圆可以写成

$$
\\gamma_x x^2 + 2\\alpha_x x x' + \\beta_x x'^2 = \\varepsilon, \\qquad \\beta_x\\gamma_x - \\alpha_x^2 = 1
$$

$\\alpha_x$、$\\beta_x$、$\\gamma_x$ 称为 Twiss 参数或 Courant–Snyder 参数，椭圆面积为 $\\pi\\varepsilon$。三个参数中只有两个独立。对 rms 椭圆，它们与二阶矩的关系是

$$
\\langle x^2\\rangle = \\beta_x\\varepsilon, \\qquad
\\langle x x'\\rangle = -\\alpha_x\\varepsilon, \\qquad
\\langle x'^2\\rangle = \\gamma_x\\varepsilon
$$

反过来由粒子分布求 Twiss 参数：$\\beta_x = \\langle x^2\\rangle/\\varepsilon_{\\mathrm{rms}}$、$\\alpha_x = -\\langle x x'\\rangle/\\varepsilon_{\\mathrm{rms}}$。

### 椭圆的几何

- 最大位置 $x_{\\max} = \\sqrt{\\varepsilon\\beta_x}$，最大角度 $x'_{\\max} = \\sqrt{\\varepsilon\\gamma_x}$；
- 椭圆与 $x$ 轴的交点 $x = \\sqrt{\\varepsilon/\\gamma_x}$，与 $x'$ 轴的交点 $x' = \\sqrt{\\varepsilon/\\beta_x}$；
- 椭圆主轴相对 $x$ 轴的倾角 $\\theta$ 满足 $\\tan 2\\theta = 2\\alpha_x/(\\gamma_x - \\beta_x)$。

$\\alpha_x$ 的符号表示束流的会聚 / 发散状态：

| $\\alpha_x$ | $\\langle x x'\\rangle$ | 状态 |
|---|---|---|
| $\\alpha_x > 0$ | $< 0$ | 会聚（向束腰运动） |
| $\\alpha_x = 0$ | $= 0$ | 束腰（椭圆正立） |
| $\\alpha_x < 0$ | $> 0$ | 发散（离开束腰） |

### Courant–Snyder 不变量

在线性聚焦系统中，Twiss 参数随 $s$ 变化（遵循 [线性光学](#linear-optics-twiss-transport) 中的传输规律），而单个粒子的量

$$
J = \\gamma_x(s)\\,x^2 + 2\\alpha_x(s)\\,x x' + \\beta_x(s)\\,x'^2
$$

沿束线保持不变，称为 Courant–Snyder 不变量。也就是说，粒子始终位于同一族形状不断变化、面积不变的椭圆上。粒子的轨迹可以写成 $x(s) = \\sqrt{J\\beta_x(s)}\\cos\\left(\\psi(s) + \\psi_0\\right)$，其中 $\\psi$ 是 betatron 相位，见 [线性光学](#linear-optics-phase-advance)。

## σ 矩阵与相椭圆 {#phase-space-sigma}

把二阶矩排成对称矩阵（束流矩阵、σ 矩阵）：

$$
\\Sigma = \\begin{pmatrix} \\langle x^2\\rangle & \\langle x x'\\rangle \\\\ \\langle x x'\\rangle & \\langle x'^2\\rangle \\end{pmatrix}
= \\varepsilon \\begin{pmatrix} \\beta_x & -\\alpha_x \\\\ -\\alpha_x & \\gamma_x \\end{pmatrix},
\\qquad \\varepsilon = \\sqrt{\\det\\Sigma}
$$

rms 椭圆就是 $X\\tp\\Sigma^{-1}X = 1$，$X = (x, x')\\tp$。σ 矩阵的好处是它在线性传输 $X_1 = M X_0$ 下的变换极其简单：

$$
\\Sigma_1 = M\\,\\Sigma_0\\,M\\tp
$$

这一式对任何分布都成立（不只是椭圆分布），因为它只涉及二阶矩。AVAS 的线性预览正是用这种方式推进束流的。

### 六维 σ 矩阵

推广到六维，$\\Sigma_{ij} = \\langle u_i u_j\\rangle$，$u = (x, x', y, y', z, \\delta)$。对角块给出三个平面的投影发射度，非对角块描述平面之间的关联：

- 二极铁产生 $x$–$\\delta$ 关联（色散），见 [二极铁](#dipole)；
- 螺线管使 $x$ 与 $y$ 平面耦合，此时投影发射度 $\\varepsilon_x$、$\\varepsilon_y$ 可以变化，但四维发射度 $\\sqrt{\\det\\Sigma_4}$ 在线性辛变换下守恒，见 [螺线管](#solenoid)；
- 射频腔产生 $z$–$\\delta$ 关联，见 [纵向运动](#longitudinal)。

## 束流分布与边缘发射度 {#phase-space-distributions}

\`beam.txt\` 的 \`distribution V1 V2\` 分别选择横向和纵向分布，可选 KV、GS、PB、WB：

| 代号 | 分布 | 四维相空间密度（$r^2$ 为归一化"半径"平方） | $\\varepsilon_{100\\%}/\\varepsilon_{\\mathrm{rms}}$（四维分布投影到 $x$–$x'$） |
|---|---|---|---|
| KV | Kapchinskij–Vladimirskij | 集中在四维椭球面 $r^2 = 1$ 上 | 4 |
| WB | 水袋（waterbag） | 在 $r^2 \\le 1$ 内均匀 | 6 |
| PB | 抛物线（parabolic） | $\\propto 1 - r^2$，$r^2 \\le 1$ | 8 |
| GS | 高斯（Gaussian） | $\\propto \\exp(-r^2/2)$ | 无界（需截断） |

这些比值来自一个一般结果：若 $d$ 维椭球内的均匀分布投影到某一坐标，有 $\\langle x^2\\rangle = a^2/(d+2)$，$a$ 为半轴。几点说明：

- **KV 分布**在实空间 $(x, y)$ 的投影是均匀椭圆，其空间电荷力严格线性，因此是包络方程和空间电荷理论的基准分布（见 [空间电荷](#space-charge)）；但它的相空间结构很特殊，实际束流中并不存在。
- **WB**、**PB** 更接近实际束流，边缘清楚；**GS** 尾部最长。
- 对二维高斯分布，发射度为 $n\\,\\varepsilon_{\\mathrm{rms}}$ 的 rms 相似椭圆内所含粒子比例为 $1 - \\ee^{-n/2}$：$n$ = 1、4、6、8 时分别为 39.3 %、86.5 %、95.0 %、98.2 %。

> 注意：上表是理想四维分布的标准结果（Lapostolle、Reiser）。AVAS 如何生成纵向分布、高斯分布在哪里截断，手册没有说明；需要时可以读取运行输出的 \`inData.dst\`（模拟实际使用的初始分布）直接统计。

## 纵向相空间 {#phase-space-longitudinal}

纵向坐标 $(z, \\delta)$ 与直线加速器中常用的 $(\\Delta\\varphi, \\Delta W)$ 之间的换算是（见 [运动学](#kinematics-rf)）：

$$
\\Delta\\varphi = -\\frac{2\\pi}{\\beta\\lambda}\\,z, \\qquad
\\Delta W = \\beta^2\\gamma\\,mc^2\\,\\delta
$$

因此两种几何发射度之间相差一个常数因子

$$
\\varepsilon_{\\Delta\\varphi\\Delta W} = \\frac{2\\pi}{\\beta\\lambda}\\,\\beta^2\\gamma\\,mc^2\\;\\varepsilon_{z\\delta}
$$

（以度为相位单位时把 $2\\pi$ 换成 $360$。）$\\lambda$ 是束流频率对应的波长。纵向相空间的运动——同步相位、相稳定区、纵向聚焦——在 [纵向运动](#longitudinal) 一章讨论。

## 在 AVAS 中 {#phase-space-in-avas}

\`\`\`avas-params
beam:twissx beam:twissy beam:twissz beam:distribution beam:particlenumber beam:initpos beam:initmom
\`\`\`

\`twissx α β ε\`（\`twissy\`、\`twissz\` 同）三个数的含义：

- $\\alpha$：无量纲；
- $\\beta$：单位 mm/mrad，数值上等于 m/rad；
- $\\varepsilon$：**归一化 rms 发射度**，单位 π·mm·mrad。

> AVAS 约定：rms 尺寸由 $\\sigma_x = \\sqrt{\\beta_x\\,\\varepsilon_n/(\\beta\\gamma)}$ 给出（$\\beta_x$ 以 mm/mrad、$\\varepsilon_n$ 以 mm·mrad 计时 $\\sigma_x$ 以 mm 计）。\`twissz\` 采用同样的单位与归一化：$z$ 以 mm 计，$z' = \\delta = \\Delta p/p$ 以 mrad 计，归一化同样乘 $\\beta\\gamma$（界面的线性预览按这一约定换算，并已与内核比较）。其他程序的纵向发射度可能采用 $(\\Delta\\varphi, \\Delta W)$ 或不同的归一化因子，交换数据时请逐项核对。

### 示例项目的初始束流

示例 \`beam.txt\`（质子 1.527 MeV，$\\beta\\gamma$ = 0.057076）：

\`\`\`text
twissx -0.65785 0.37743 0.13768
twissy -0.6341 0.37292 0.14059
twissz 0.04275 0.37292 0.26506
distribution GS GS
\`\`\`

由上面的约定换算：

| 平面 | 几何 rms 发射度 (mm·mrad) | rms 尺寸 | rms 角度 / 动量偏差 |
|---|---|---|---|
| x | 2.412 | 0.954 mm | 3.026 mrad |
| y | 2.463 | 0.958 mm | 3.043 mrad |
| z | 4.644 | 1.316 mm | $\\delta_{\\mathrm{rms}}$ = 3.532 × 10$^{-3}$ |

横向 $\\alpha < 0$，说明束流在入口处是发散的。纵向 rms 长度 1.316 mm 对应 162.5 MHz 下 4.51° 的 rms 相宽；$\\delta_{\\mathrm{rms}}$ 对应 rms 能散 10.78 keV（0.71 %）；纵向几何 rms 发射度换算为 48.5 deg·keV。

### 其他关键字

- \`particlenumber\`：宏粒子数，写 1 时为单粒子模式（此时 \`twiss*\`、\`current\`、\`distribution\` 不生效）。
- \`initpos\`、\`initmom\`：整个束团的初始位置（mm）和角度（mrad）偏移，用来模拟入射偏差。
- \`readparticledistribution\`：从 \`.dst\`/\`.edst\` 文件读入粒子分布，此时除 \`numofcharge\` 外的关键字均不生效（界面用 \`use_dst\` 切换）。

输出中，\`DataSet.txt\` 第 16、18、20 列是 $x$、$y$、$z$ 的 rms 尺寸（m），与上面的 $\\sigma$ 直接对应。

## 动手实验 {#phase-space-lab}

实验画出 rms 椭圆在一个薄透镜（焦距 \`f\`，单位 m；0 表示不放透镜）和其后长度 \`L\` 的漂移段前后的形状，并给出出口的 Twiss 参数和 rms 尺寸。参数 \`beta\` 以 mm/mrad 计，\`emit\` 为归一化 rms 发射度（π·mm·mrad）。

1. 从束腰出发（$\\alpha_x = 0$、$\\beta_x$ = 1 m），焦距 0.5 m 的透镜后漂移 0.5 m。按 [线性光学](#linear-optics-twiss-transport) 的公式，透镜后 $\\alpha_x = \\beta_x/f = 2$，漂移后 $\\beta_x = 0.25$ m、$\\alpha_x = -0.5$；新的束腰在透镜后 $\\alpha_x/\\gamma_x$ = 0.4 m 处，$\\beta^* = 1/\\gamma_x$ = 0.2 m。rms 尺寸从 1.87 mm 变为 0.94 mm。
2. 改变 \`L\`，找出椭圆正立（束腰）的位置；改变 \`emit\`，确认椭圆形状不变、只是整体放大。
3. 把 \`W\` 加大，几何发射度按 $1/(\\beta\\gamma)$ 缩小。

\`\`\`avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
\`\`\`

示例项目的 x 方向初始束流在 0.1 m 漂移段中的变化（发散束，尺寸增大）：

\`\`\`avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.1
\`\`\`

## 延伸阅读 {#phase-space-further}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.：发射度、刘维尔定理、KV 与其他分布、rms 等效束。
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.：横向动力学与 Twiss 参数。
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1101：rms 发射度概念的提出。
- K. Floettmann, "Some basic features of the beam emittance", *Phys. Rev. ST Accel. Beams* 6, 034202 (2003)：归一化发射度与能散的关系。
- 继续阅读：[线性光学](#linear-optics)。
`,w=`# Quadrupole

The quadrupole is the main transverse focusing element of an accelerator. Its magnetic field vanishes on the beam axis and grows linearly with the distance from it, so for the beam it acts as a "magnetic lens". Unlike an optical lens, a quadrupole that focuses in one transverse plane necessarily defocuses in the other; only by alternating focusing and defocusing quadrupoles (strong, alternating-gradient focusing) is net focusing obtained in both planes.

Before this chapter it helps to read [drift](#drift) (the basic form of transfer matrices) and [linear optics](#linear-optics) (Hill's equation, phase advance, stability).

## Role and placement {#quadrupole-role}

- **Transverse focusing**: in the medium- and high-energy parts of a linac, quadrupoles form FODO, doublet or triplet periods that keep the beam inside the aperture.
- **Matching**: at the junction of two structures (for example the medium-energy transport line, MEBT, after an RFQ, or transitions between different periods) several independently powered quadrupoles match the beam's Twiss parameters to the downstream periodic solution (see [matching](#matching)).
- **Beam shaping**: adjusting the size and shape of the beam spot in front of a target or beam dump.

Quadrupoles also work for low-energy beams, but they act asymmetrically on $x$ and $y$, and strong focusing needs alternation, which takes room; at very low energy or between superconducting cavities [solenoids](#solenoid) are often used instead.

## Physics and field {#quadrupole-field}

### Pole geometry and the ideal field

An ideal quadrupole has four poles, N–S–N–S alternating, placed at $\\pm 45^\\circ$ and $\\pm 135^\\circ$. In the bore, free of currents and iron, the field is the gradient of a magnetic scalar potential, $\\boldsymbol{B} = -\\nabla \\Phi_m$, and $\\Phi_m$ obeys Laplace's equation. The lowest quadrupole solution is

$$
\\Phi_m = -G\\,x\\,y,
\\qquad
B_x = G\\,y,\\qquad B_y = G\\,x,
$$

where $G = \\partial B_y/\\partial x = \\partial B_x/\\partial y$ is the **field gradient** in T/m. The pole surfaces are the equipotentials $x y = \\pm R^2/2$ — hyperbolas, with $R$ the radius of the inscribed circle (the aperture). Equivalently the vector potential $A_z = \\tfrac{G}{2}(y^2 - x^2)$ gives the same field.

The field at the pole tip is $B_{\\text{pole}} = G\\,R$. Iron poles begin to saturate around 1–1.5 T, so for room-temperature quadrupoles $G R \\lesssim 1$ T is a practical limit: the smaller the aperture, the higher the reachable gradient.

### Constraint from Maxwell's equations

In the bore $\\nabla\\cdot\\boldsymbol{B} = 0$ and $\\nabla\\times\\boldsymbol{B} = 0$. The $z$ component of the latter gives

$$
\\frac{\\partial B_y}{\\partial x} = \\frac{\\partial B_x}{\\partial y}.
$$

This is the origin of "focusing in one plane, defocusing in the other": the restoring force in $x$ comes from $\\partial B_y/\\partial x$, the force in $y$ from $\\partial B_x/\\partial y$; the two are equal but act with opposite signs (next subsection). In a source-free region no static magnetic field focuses both planes at once (a form of Earnshaw's theorem).

### Lorentz force

For a particle of charge $q$ moving along $+z$ with velocity approximately $v\\hat{z}$, $\\boldsymbol{F} = q\\,\\boldsymbol{v}\\times\\boldsymbol{B}$ gives

$$
F_x = -q v B_y = -q v G\\,x,\\qquad F_y = q v B_x = q v G\\,y .
$$

For $qG > 0$ the force in $x$ points to the axis (focusing) and the force in $y$ away from it (defocusing); for $qG < 0$ the other way round. The force is proportional to the distance from the axis, so the lens is linear: its strength does not depend on the particle's transverse position.

### Fringe fields and effective length

In a real quadrupole the gradient does not drop to zero abruptly at the ends; it decays smoothly over a distance of about one aperture radius (the fringe field). In linear optics it is usually replaced by a "hard-edge" model with the central gradient $G_0$ and the **effective length**

$$
L_{\\text{eff}} = \\frac{1}{G_0}\\int G(s)\\,\\dd s .
$$

The hard-edge model keeps the integrated focusing strength $\\int G\\,\\dd s$ but neglects the higher-order (non-linear) effects of the fringe field, such as end effects cubic in the position. Higher multipoles (dodecapole, 20-pole …, from the pole shape and manufacturing errors) are not part of the matrix model either.

## Equations of motion and transfer matrix {#quadrupole-matrix}

### Focusing strength

Dividing the force by $p v$ gives the linear equations of motion with $s$ as the independent variable (Hill's equation with constant coefficient)

$$
x'' + k\\,x = 0,\\qquad y'' - k\\,y = 0,\\qquad
k = \\frac{q\\,G}{p} = \\frac{G}{B\\rho}.
$$

$B\\rho = p/q$ is the magnetic rigidity, in practice $B\\rho\\,[\\mathrm{T\\,m}] = p\\,[\\mathrm{MeV}/c]/(299.792458\\,Z)$. The unit of $k$ is m$^{-2}$.

### Thick-lens matrix

Let $\\varphi = \\sqrt{\\lvert k\\rvert}\\,L$. For $k>0$ the $x$ plane focuses and the $y$ plane defocuses:

$$
M_x =
\\begin{pmatrix}
\\cos\\varphi & \\dfrac{\\sin\\varphi}{\\sqrt{k}}\\\\[4pt]
-\\sqrt{k}\\,\\sin\\varphi & \\cos\\varphi
\\end{pmatrix},
\\qquad
M_y =
\\begin{pmatrix}
\\cosh\\varphi & \\dfrac{\\sinh\\varphi}{\\sqrt{k}}\\\\[4pt]
\\sqrt{k}\\,\\sinh\\varphi & \\cosh\\varphi
\\end{pmatrix}.
$$

For $k<0$ the planes are swapped. The longitudinal part is that of a drift of the same length ($R_{56} = L/\\gamma^2$). The full $6\\times 6$ matrix consists of three diagonal blocks:

$$
M_{\\text{quad}} = \\operatorname{diag}\\!\\left(M_x,\\; M_y,\\; \\begin{pmatrix} 1 & L/\\gamma^2 \\\\ 0 & 1 \\end{pmatrix}\\right).
$$

This agrees with \`quad_matrix(L, k, gamma)\` of AVAS's linear envelope preview (with $k = G/B\\rho$, positive values focusing in $x$).

### Thin-lens approximation

For $\\varphi \\ll 1$ the quadrupole is treated as a thin lens at its centre between two drifts of $L/2$; the lens matrix is

$$
M_{\\text{thin}} =
\\begin{pmatrix}
1 & 0\\\\
-1/f & 1
\\end{pmatrix},
\\qquad
\\frac{1}{f} = k\\,L = \\frac{G L}{B\\rho}.
$$

The focal length is $+f$ in $x$ and $-f$ in $y$. The real thick-lens focal length ($-1/M_{21}$) differs by $\\mathcal{O}(\\varphi^2)$: in the focusing plane $f_x = 1/(\\sqrt{k}\\sin\\varphi)$ is slightly longer, in the defocusing plane $\\lvert f_y \\rvert = 1/(\\sqrt{k}\\sinh\\varphi)$ slightly shorter.

### Doublets and triplets

Two thin lenses of focal lengths $+f$ and $-f$ a distance $d$ apart have the combined focal length

$$
\\frac{1}{F} = \\frac{1}{f_1} + \\frac{1}{f_2} - \\frac{d}{f_1 f_2} = \\frac{d}{f^2},
$$

positive in both planes — the heart of **alternating-gradient focusing**: the beam is larger in the focusing lens and smaller in the defocusing lens, and the net effect is focusing. The principal planes of a doublet differ between the two planes, so the beam leaving it is in general not round; a symmetric triplet ($-f/2$, $+f$, $-f/2$ or the reverse) gives similar imaging in both planes and is used where a round spot is needed.

### FODO period

A period of a focusing quadrupole QF, a drift, a defocusing quadrupole QD and a drift is called FODO. For thin lenses whose centres are $L_h$ apart, the phase advance per period $\\sigma_0$ satisfies

$$
\\sin\\frac{\\sigma_0}{2} = \\frac{L_h}{2 f}.
$$

Periodic motion is stable when the one-period matrix satisfies $\\lvert \\Tr M \\rvert/2 < 1$, for thin lenses $f > L_h/2$. For thick quadrupoles multiply the thick-lens matrices to get $M$ and take the phase advance from $\\cos\\sigma_0 = \\Tr M/2$.

## In AVAS {#quadrupole-avas}

### Matrix-model quadrupole

\`\`\`text
quad L R 0 G
\`\`\`

\`\`\`avas-params
quad
\`\`\`

- \`L\`: length (m), the effective length of the hard-edge model.
- \`R\`: aperture radius (m), used for loss detection.
- The third parameter is reserved; write 0.
- \`G\`: field gradient (T/m).

> AVAS convention: $k = G/(B\\rho)$; **$G > 0$ focuses a positively charged particle in $x$ and defocuses it in $y$** (checked with \`quad_matrix\` of the linear preview: for $G = 10$ T/m and $L = 0.1$ m, $M_{21} = -5.09\\ \\mathrm{m^{-1}}$, i.e. focusing in $x$). For negatively charged particles (for example H⁻) the same $G$ swaps the focusing plane. In a FODO, QF takes a positive and QD a negative $G$.

> AVAS convention: \`quad\` is a **z-code matrix-model element**; it acts as a whole through its transfer matrix and no field distribution enters the particle tracking. The manual asks that the first and the last element of a multi-particle simulation not be matrix-model elements; put a very short drift at both ends (see [drift](#drift-avas)). How space charge is handled inside matrix-model elements is not described in the manual; the kernel is the reference.

Example: one FODO period (example beam, numbers below):

\`\`\`text
quad  0.1 0.02 0  6
drift 0.2 0.02 0
quad  0.1 0.02 0 -6
drift 0.2 0.02 0
\`\`\`

### Field-map quadrupole

When fringe fields or a measured field matter, use a static magnetic field-map element (\`field\`, type 3) instead:

\`\`\`text
field 0.1 0.02 0 3 0 0 0 1.0 qmap
\`\`\`

The field-map files are \`qmap.bsx\`, \`qmap.bsy\` (and optionally \`qmap.bsz\`), the field is in T, and the kernel scales it by \`Kb\`: actual field = map × \`Kb\`. **The sign of \`Kb\` sets the polarity**: changing the sign of \`Kb\` swaps the focusing and defocusing planes. Parameters a static magnetic field does not use (frequency, phase, \`Ke\`) are written as 0, as the manual says. Field-map elements are tracked with the t-code and naturally include the fringe field and the higher components contained in the map. Storage order and reading of maps are covered in the [field maps](#field-maps) chapter.

### Errors

Position, rotation and gradient errors of quadrupoles are given with the "static-field element error" commands (acting on the N elements below the command):

\`\`\`avas-params
err_quad_ncpl_stat
\`\`\`

\`dx\`, \`dy\` are transverse offsets (mm), \`dφx\`, \`dφy\`, \`dφz\` rotations about the three axes (°), \`dG\` the relative gradient error (%), \`dz\` the longitudinal offset (mm). The full error-study workflow is in [errors and correction](#errors).

## Parameters and typical values {#quadrupole-params}

| Quantity | Typical range (low/medium-energy proton linac) | Remark |
|---|---|---|
| gradient $G$ | a few T/m to a few tens of T/m | room-temperature electromagnets; permanent-magnet quadrupoles can go higher |
| effective length $L$ | 0.05–0.3 m | short and strong at low energy, longer at high energy |
| aperture radius $R$ | 0.01–0.04 m | set by beam size, errors and loss budget |
| pole-tip field $G R$ | $\\lesssim 1$ T (iron) | above it the iron saturates and the field quality degrades |
| phase advance per period $\\sigma_0$ | 40°–90° (zero current) | avoid parametric resonances above 90°, see [space charge](#space-charge) |

### Worked example

Example beam: protons, $W = 1.527$ MeV, $p = 53.55$ MeV/$c$, $B\\rho = 0.17863$ T·m.

| $G$ | $L$ | $k$ | $\\varphi$ | thin-lens $f$ | thick $f_x$ | thick $f_y$ |
|---|---|---|---|---|---|---|
| 4 T/m | 0.1 m | 22.39 m⁻² | 0.473 | 0.447 m | 0.464 m | −0.430 m |
| 6 T/m | 0.1 m | 33.59 m⁻² | 0.580 | 0.298 m | 0.315 m | −0.282 m |
| 10 T/m | 0.1 m | 55.98 m⁻² | 0.748 | 0.179 m | 0.196 m | −0.163 m |
| 20 T/m | 0.05 m | 111.96 m⁻² | 0.529 | 0.179 m | 0.187 m | −0.171 m |

For low-energy protons a few T/m already make a strong lens; when $\\varphi$ approaches 1 the thin-lens approximation is off by about 10 %.

FODO period with $L_q = 0.1$ m, $L_d = 0.2$ m (lens centres $L_h = 0.3$ m apart):

| $G$ | thick-lens phase advance $\\sigma_0$ | thin-lens estimate | $\\Tr M/2$ |
|---|---|---|---|
| 6 T/m | 52.7° | 60.5° | 0.606 |
| 10 T/m | 95.4° | 114.2° | −0.094 |
| 10 T/m, $L_d = 0.3$ m | unstable | unstable | −1.083 |

For $G = 6$ T/m the periodic $\\beta_x$ varies between 0.43 and 1.06 m, which for the example emittance ($\\varepsilon_{n,x} = 0.1377\\ \\pi$·mm·mrad) means rms sizes of 1.0–1.6 mm. Note that at $\\varphi \\approx 0.6$ the thin-lens estimate is already 8° off, and more for stronger focusing; use thick-lens matrices for design.

## Dynamics and common problems {#quadrupole-dynamics}

### The "breathing" envelope

In a FODO period $\\beta_x$ is largest at QF and smallest at QD, $\\beta_y$ the other way round; the two are half a period out of phase. The beam cross-section therefore flattens alternately in the two planes — the signature of quadrupole focusing. The beam is round only where the two cross.

### Chromatic effects

The focusing strength is $k \\propto 1/p$; a particle with momentum deviation $\\delta$ sees $k(1-\\delta)$ (to first order). In dispersion-free straight sections the effect on the rms size is usually small, but with a large energy spread and strong focusing it causes effective emittance growth. AVAS's linear preview does not include chromatic effects (see the notes in \`linear_optics.py\`); the kernel is the reference.

### Misalignment and rotation

- A transverse offset $d_x$ of the quadrupole puts the beam centre at $-d_x$ relative to the magnetic centre; it receives a deflection $\\Delta x' = k L\\,d_x$ (thin lens), a dipole "kick". This is the main source of orbit distortion and is corrected with [steerers](#steerer).
- A rotation $\\theta$ about $z$ creates a skew-quadrupole component (of relative strength about $\\sin 2\\theta$) that couples $x$ and $y$ and makes the projected emittances grow.
- A gradient error $\\Delta G/G$ changes the phase advance and causes mismatch and beta beating.

### Space charge

At high current, space charge defocuses in both planes and lowers the phase advance per period from $\\sigma_0$ to $\\sigma$ ("tune depression"). In a quadrupole channel the beam flattens alternately in the two planes, and the space-charge force varies with it; to avoid envelope instabilities and parametric resonances the zero-current phase advance is usually kept below $90^\\circ$. The lab in the [space charge](#space-charge) chapter shows this effect.

### Common problems

- Wrong gradient sign: the intended FODO becomes "FFOO", and the beam diverges quickly in one plane and is lost.
- Copying TraceWin's mm units: length and aperture of TraceWin's \`QUAD\` are in mm, AVAS uses m.
- Putting a matrix-model quadrupole first or last in the lattice (see the AVAS convention above).
- When comparing matrix-model and field-map results, remember that the map includes the fringe field and its effective length is not necessarily the map length: make $\\int G\\,\\dd s$ equal before comparing.

## Hands-on {#quadrupole-lab}

First look at the field and the force on the quadrupole cross-section (a **schematic** computed from the parameters, not field-map data). AVAS always draws cross-sections looking from downstream to upstream with the beam coming out of the screen (⊙), so $\\boldsymbol{F} = q\\boldsymbol{v}\\times\\boldsymbol{B}$ can be checked directly: for $G > 0$ the force on the $x$ axis points to the centre, the force on the $y$ axis away from it.

\`\`\`avas-field
quadrupole G=6 L=0.1 R=0.02
\`\`\`

The FODO lab below uses thick-lens matrices. First reproduce the 52.7° of the table with $G = 6$ T/m and $L_d = 0.2$ m; then:

1. Increase \`G\` step by step, watch the phase advance and $\\lvert\\Tr M\\rvert/2$, and find the edge of the stable region (near $\\sigma_0 = 180^\\circ$).
2. Increase \`Ld\` to 0.3 m: at $G = 10$ T/m the period is no longer stable and the envelope blows up.
3. Set \`mismatch\` to 1.3 and watch the beta beating of the mismatched beam; then add a current \`I\` and compare the envelope with and without space charge.

\`\`\`avas-lab
fodo Lq=0.1 G=6 Ld=0.2 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
\`\`\`

## Further reading {#quadrupole-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed. (chapter 7: quadrupole focusing, FODO and periodic solutions).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (multipole expansion, quadrupole design and fringe fields).
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific (2019) (Hill's equation, strong focusing and stability).
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1–48 (the classic paper on alternating-gradient focusing).
- Related chapters: [linear optics](#linear-optics), [solenoid](#solenoid), [matching](#matching), [lattice design](#lattice-design), [errors and correction](#errors); the full bibliography is in [references](#references).
`,T=`# 四极铁

四极铁（quadrupole）是加速器中最主要的横向聚焦元件。它的磁场在束流轴上为零、随离轴距离线性增长，因此对束流相当于一块"磁透镜"。与光学透镜不同，四极铁在一个横向平面聚焦的同时必然在另一个平面散焦；把聚焦和散焦四极铁交替排列（强聚焦，交变梯度），才能在两个平面同时得到净聚焦。

阅读本章前建议先看[漂移段](#drift)（传输矩阵的基本形式）和[线性光学](#linear-optics)（Hill 方程、相移、稳定性）。

## 作用与位置 {#quadrupole-role}

- **横向聚焦**：在直线加速器的中能、高能段，四极铁组成 FODO、双合（doublet）或三合（triplet）周期，把束流约束在孔径内。
- **匹配**：在两段结构的交界处（例如 RFQ 之后的中能传输段 MEBT、不同周期的过渡处），用若干个独立可调的四极铁把束流的 Twiss 参数匹配到下游周期解（见[匹配](#matching)）。
- **束流整形**：在靶前或束流收集器前调节束斑的大小和形状。

四极铁对低能束流同样有效，但它对 $x$、$y$ 两个平面的作用不对称，而且强聚焦需要交替排列，所需空间较大；在极低能段或超导腔串之间，常用[螺线管](#solenoid)代替。

## 物理原理与场 {#quadrupole-field}

### 极头几何与理想场

理想四极铁有四个磁极，按 N–S–N–S 交替排列在 $\\pm 45^\\circ$、$\\pm 135^\\circ$ 方向上。在无电流、无铁的孔径内部，磁场可以写成磁标势 $\\Phi_m$ 的梯度 $\\boldsymbol{B} = -\\nabla \\Phi_m$，$\\Phi_m$ 满足拉普拉斯方程。最低阶的四极解为

$$
\\Phi_m = -G\\,x\\,y,
\\qquad
B_x = G\\,y,\\qquad B_y = G\\,x,
$$

其中 $G = \\partial B_y/\\partial x = \\partial B_x/\\partial y$ 是**磁场梯度**，单位 T/m。磁极表面是等势面 $x y = \\pm R^2/2$——一组双曲线，$R$ 是内切圆（孔径）半径。等价地，用矢势 $A_z = \\tfrac{G}{2}(y^2 - x^2)$ 也可得到同一场。

极头处的场 $B_{\\text{pole}} = G\\,R$。铁磁极在 1–1.5 T 附近开始饱和，所以对常温四极铁，$G R \\lesssim 1$ T 是一个实际的上限：孔径越小，可达到的梯度越高。

### Maxwell 方程的约束

孔径内 $\\nabla\\cdot\\boldsymbol{B} = 0$、$\\nabla\\times\\boldsymbol{B} = 0$。后者的 $z$ 分量给出

$$
\\frac{\\partial B_y}{\\partial x} = \\frac{\\partial B_x}{\\partial y}.
$$

这正是四极铁"一个平面聚焦、另一个平面散焦"的根源：$x$ 方向的恢复力来自 $\\partial B_y/\\partial x$，$y$ 方向的力来自 $\\partial B_x/\\partial y$，两者相等而作用符号相反（见下节）。在无源区域内不存在对两个平面同时聚焦的静磁场（Earnshaw 定理的一种表现）。

### 洛伦兹力

对沿 $+z$ 运动、电荷为 $q$、速度近似为 $v\\hat{z}$ 的粒子，$\\boldsymbol{F} = q\\,\\boldsymbol{v}\\times\\boldsymbol{B}$ 给出

$$
F_x = -q v B_y = -q v G\\,x,\\qquad F_y = q v B_x = q v G\\,y .
$$

当 $qG > 0$ 时，$x$ 方向的力指向轴心（聚焦），$y$ 方向的力背离轴心（散焦）；$qG < 0$ 时相反。力与离轴距离成正比，所以是线性透镜，透镜的强度与粒子的横向位置无关。

### 边缘场与有效长度

实际四极铁的梯度在两端不会突然降为零，而是在大约一个孔径半径的距离内平滑衰减（边缘场）。线性光学上通常用**有效长度**

$$
L_{\\text{eff}} = \\frac{1}{G_0}\\int G(s)\\,\\dd s
$$

和中心梯度 $G_0$ 的"硬边"模型代替。硬边模型保持了积分聚焦强度 $\\int G\\,\\dd s$，但忽略了边缘场引起的高阶（非线性）效应，例如与位置三次方相关的端部作用。高阶多极场（十二极、二十极等，由极头形状与加工误差引起）同样不在矩阵模型中。

## 运动方程与传输矩阵 {#quadrupole-matrix}

### 聚焦强度

将力除以 $p v$，得到以 $s$ 为自变量的线性运动方程（Hill 方程，常系数情形）

$$
x'' + k\\,x = 0,\\qquad y'' - k\\,y = 0,\\qquad
k = \\frac{q\\,G}{p} = \\frac{G}{B\\rho}.
$$

$B\\rho = p/q$ 是磁刚度，常用公式 $B\\rho\\,[\\mathrm{T\\,m}] = p\\,[\\mathrm{MeV}/c]/(299.792458\\,Z)$。$k$ 的单位是 m$^{-2}$。

### 厚透镜矩阵

记 $\\varphi = \\sqrt{\\lvert k\\rvert}\\,L$。对 $k>0$，$x$ 平面聚焦、$y$ 平面散焦：

$$
M_x =
\\begin{pmatrix}
\\cos\\varphi & \\dfrac{\\sin\\varphi}{\\sqrt{k}}\\\\[4pt]
-\\sqrt{k}\\,\\sin\\varphi & \\cos\\varphi
\\end{pmatrix},
\\qquad
M_y =
\\begin{pmatrix}
\\cosh\\varphi & \\dfrac{\\sinh\\varphi}{\\sqrt{k}}\\\\[4pt]
\\sqrt{k}\\,\\sinh\\varphi & \\cosh\\varphi
\\end{pmatrix}.
$$

$k<0$ 时两个平面互换。纵向部分与同长度的漂移段相同（$R_{56} = L/\\gamma^2$）。完整的 $6\\times 6$ 矩阵是三块对角块：

$$
M_{\\text{quad}} = \\operatorname{diag}\\!\\left(M_x,\\; M_y,\\; \\begin{pmatrix} 1 & L/\\gamma^2 \\\\ 0 & 1 \\end{pmatrix}\\right).
$$

这与 AVAS 线性包络预览的 \`quad_matrix(L, k, gamma)\` 一致（其中 $k = G/B\\rho$，正值在 $x$ 平面聚焦）。

### 薄透镜近似

当 $\\varphi \\ll 1$ 时，把四极铁看成位于中心的薄透镜加两段 $L/2$ 漂移，透镜矩阵为

$$
M_{\\text{thin}} =
\\begin{pmatrix}
1 & 0\\\\
-1/f & 1
\\end{pmatrix},
\\qquad
\\frac{1}{f} = k\\,L = \\frac{G L}{B\\rho}.
$$

$x$ 平面焦距为 $+f$，$y$ 平面为 $-f$。厚透镜的实际焦距（$-1/M_{21}$）与之的差别为 $\\mathcal{O}(\\varphi^2)$：聚焦平面 $f_x = 1/(\\sqrt{k}\\sin\\varphi)$ 略长，散焦平面 $\\lvert f_y \\rvert = 1/(\\sqrt{k}\\sinh\\varphi)$ 略短。

### 双合与三合透镜

两个焦距为 $+f$ 和 $-f$ 的薄透镜相距 $d$，组合焦距

$$
\\frac{1}{F} = \\frac{1}{f_1} + \\frac{1}{f_2} - \\frac{d}{f_1 f_2} = \\frac{d}{f^2}
$$

在两个平面都为正——这就是**交变梯度聚焦**的核心：束流在聚焦透镜处较大、在散焦透镜处较小，净效应是聚焦。双合透镜在两个平面的主平面位置不同，出射束一般不是圆的；对称的三合透镜（$-f/2$、$+f$、$-f/2$ 或反之）可以在两个平面给出相近的成像，常用于需要圆束斑的地方。

### FODO 周期

由聚焦四极铁 QF、漂移、散焦四极铁 QD、漂移组成的周期称为 FODO。对薄透镜，若透镜中心间距为 $L_h$，每周期的相移 $\\sigma_0$ 满足

$$
\\sin\\frac{\\sigma_0}{2} = \\frac{L_h}{2 f}.
$$

周期运动稳定的条件是单周期矩阵满足 $\\lvert \\Tr M \\rvert/2 < 1$，对薄透镜即 $f > L_h/2$。厚四极铁要用厚透镜矩阵相乘得到 $M$，再由 $\\cos\\sigma_0 = \\Tr M/2$ 求相移。

## 在 AVAS 中 {#quadrupole-avas}

### 矩阵模型四极铁

\`\`\`text
quad L R 0 G
\`\`\`

\`\`\`avas-params
quad
\`\`\`

- \`L\`：长度（m），硬边模型的有效长度。
- \`R\`：孔径半径（m），用于损失判断。
- 第三个参数保留，写 0。
- \`G\`：磁场梯度（T/m）。

> AVAS 约定：$k = G/(B\\rho)$，**$G > 0$ 对正电荷粒子在 $x$ 平面聚焦、$y$ 平面散焦**（已用线性预览的 \`quad_matrix\` 核对：$G = 10$ T/m、$L = 0.1$ m 时 $M_{21} = -5.09\\ \\mathrm{m^{-1}}$，即 $x$ 聚焦）。负电荷粒子（例如 H⁻）在同一 $G$ 下聚焦平面互换。设计 FODO 时 QF 用正 $G$、QD 用负 $G$。

> AVAS 约定：\`quad\` 是 **z-code 矩阵模型元件**，按传输矩阵整体作用，场分布不进入粒子跟踪；手册要求多粒子模拟中第一个和最后一个元件不要是矩阵模型元件，可在首尾加一段很短的 drift（见[漂移段](#drift-avas)）。矩阵模型元件内部如何处理空间电荷，手册没有说明，以内核结果为准。

例：一个 FODO 周期（示例束流，见下文的数值）：

\`\`\`text
quad  0.1 0.02 0  6
drift 0.2 0.02 0
quad  0.1 0.02 0 -6
drift 0.2 0.02 0
\`\`\`

### 场图四极铁

需要边缘场或实测场时，用静磁场场图元件（\`field\`，类型 3）代替：

\`\`\`text
field 0.1 0.02 0 3 0 0 0 1.0 qmap
\`\`\`

场图文件为 \`qmap.bsx\`、\`qmap.bsy\`（以及可选的 \`qmap.bsz\`），磁场单位为 T，内核按 \`Kb\` 缩放：实际场 = 场图 × \`Kb\`。**\`Kb\` 的符号决定极性**：把 \`Kb\` 变号即交换聚焦和散焦平面。静磁场不使用的参数（频率、相位、\`Ke\`）按手册写 0。场图元件由 t-code 跟踪，自然包含边缘场和场图中的高阶分量。场图的存储顺序和读法见[场图](#field-maps)一章。

### 误差

四极铁的位置、转角和梯度误差用"静磁元件误差"命令给出（作用于命令下方 N 个元件）：

\`\`\`avas-params
err_quad_ncpl_stat
\`\`\`

\`dx\`、\`dy\` 为横向偏移（mm），\`dφx\`、\`dφy\`、\`dφz\` 为绕三个轴的转角（°），\`dG\` 为梯度相对误差（%），\`dz\` 为纵向偏移（mm）。完整的误差研究流程见[误差与校正](#errors)。

## 可调参数与典型取值 {#quadrupole-params}

| 量 | 典型范围（低、中能质子直线加速器） | 说明 |
|---|---|---|
| 梯度 $G$ | 数 T/m 至数十 T/m | 常温电磁铁；永磁四极铁可以更高 |
| 有效长度 $L$ | 0.05–0.3 m | 低能段短而强，高能段长 |
| 孔径半径 $R$ | 0.01–0.04 m | 由束流尺寸、误差和损失预算决定 |
| 极头场 $G R$ | $\\lesssim 1$ T（铁芯） | 超过后铁芯饱和、场质量下降 |
| 每周期相移 $\\sigma_0$ | 40°–90°（零流强） | 避免 90° 以上的参数共振，见[空间电荷](#space-charge) |

### 数值例子

示例束流：质子，$W = 1.527$ MeV，$p = 53.55$ MeV/$c$，$B\\rho = 0.17863$ T·m。

| $G$ | $L$ | $k$ | $\\varphi$ | 薄透镜 $f$ | 厚透镜 $f_x$ | 厚透镜 $f_y$ |
|---|---|---|---|---|---|---|
| 4 T/m | 0.1 m | 22.39 m⁻² | 0.473 | 0.447 m | 0.464 m | −0.430 m |
| 6 T/m | 0.1 m | 33.59 m⁻² | 0.580 | 0.298 m | 0.315 m | −0.282 m |
| 10 T/m | 0.1 m | 55.98 m⁻² | 0.748 | 0.179 m | 0.196 m | −0.163 m |
| 20 T/m | 0.05 m | 111.96 m⁻² | 0.529 | 0.179 m | 0.187 m | −0.171 m |

对低能质子，几 T/m 的梯度就已经是很强的透镜；$\\varphi$ 接近 1 时薄透镜近似的误差达到 10 % 量级。

FODO 周期 $L_q = 0.1$ m、$L_d = 0.2$ m（透镜中心间距 $L_h = 0.3$ m）的结果：

| $G$ | 厚透镜相移 $\\sigma_0$ | 薄透镜估计 | $\\Tr M/2$ |
|---|---|---|---|
| 6 T/m | 52.7° | 60.5° | 0.606 |
| 10 T/m | 95.4° | 114.2° | −0.094 |
| 10 T/m，$L_d = 0.3$ m | 不稳定 | 不稳定 | −1.083 |

$G = 6$ T/m 时周期解 $\\beta_x$ 在 0.43–1.06 m 之间变化，对示例发射度（$\\varepsilon_{n,x} = 0.1377\\ \\pi$·mm·mrad）对应 rms 尺寸 1.0–1.6 mm。注意薄透镜估计在 $\\varphi \\approx 0.6$ 时已经偏差 8°，强聚焦时偏差更大，设计时应使用厚透镜矩阵。

## 动力学效应与常见问题 {#quadrupole-dynamics}

### 包络的"呼吸"

在 FODO 周期中，$x$ 方向的 $\\beta$ 在 QF 处最大、QD 处最小，$y$ 方向相反；两者的相位相差半个周期。束流截面因此在两个平面交替地变扁，这是四极铁聚焦的特征。圆形束只出现在两者交叉的位置。

### 色差

聚焦强度 $k \\propto 1/p$，动量偏差 $\\delta$ 的粒子感受到的强度为 $k(1-\\delta)$（一阶）。在色散为零的直线段中，色差对 rms 尺寸的影响通常较小，但在能散大、聚焦强的地方会造成有效发射度增长。AVAS 的线性预览不包含色差（见 \`linear_optics.py\` 的说明），以内核结果为准。

### 错位与转角

- 四极铁横向偏移 $d_x$：束流中心相对磁场中心偏 $-d_x$，受到 $\\Delta x' = k L\\,d_x$（薄透镜近似）的偏转，即一个偶极"踢"；这是轨道畸变的主要来源，需用[校正铁](#steerer)校正。
- 绕 $z$ 轴转角 $\\theta$：产生斜四极分量（强度约 $\\sin 2\\theta$ 倍），耦合 $x$ 与 $y$，使投影发射度增长。
- 梯度误差 $\\Delta G/G$：改变相移，造成失配和包络拍频（beta beating）。

### 空间电荷

强流时空间电荷在两个平面产生散焦，使每周期相移从 $\\sigma_0$ 降到 $\\sigma$（"相移压低"）。四极铁通道中的束流在两个平面交替变扁，空间电荷力随之变化；为避免包络不稳定和参数共振，零流强相移一般取 $\\sigma_0 < 90^\\circ$。可用[空间电荷](#space-charge)一章的实验观察这一效应。

### 常见问题

- 梯度符号写反：本来的 FODO 变成"FFOO"，束流在一个平面迅速发散而损失。
- 把 TraceWin 的 mm 单位直接抄入：TraceWin \`QUAD\` 的长度和孔径是 mm，AVAS 是 m。
- 将矩阵模型四极铁放在 lattice 首尾（见上文 AVAS 约定）。
- 用矩阵模型和场图模型比较结果时，注意场图包含边缘场，有效长度不一定等于场图长度：应使 $\\int G\\,\\dd s$ 相同再比较。

## 动手实验 {#quadrupole-lab}

先看看四极铁横截面上的场和受力（按参数算出的**示意图**，不是场图数据）。AVAS 的横截面一律从下游往上游看，束流垂直屏幕向外（⊙），因此可以直接用 $\\boldsymbol{F} = q\\boldsymbol{v}\\times\\boldsymbol{B}$ 核对：$G > 0$ 时，$x$ 轴上的受力指向圆心，$y$ 轴上的受力背离圆心。

\`\`\`avas-field
quadrupole G=6 L=0.1 R=0.02
\`\`\`

下面的 FODO 实验使用厚透镜矩阵。先用 $G = 6$ T/m、$L_d = 0.2$ m 复现上表的 52.7°；然后：

1. 逐步增大 \`G\`，观察相移和 $\\lvert\\Tr M\\rvert/2$，找到稳定区的边界（约在 $\\sigma_0 = 180^\\circ$）。
2. 把 \`Ld\` 增大到 0.3 m，在 $G = 10$ T/m 下周期不再稳定，包络发散。
3. 把 \`mismatch\` 改为 1.3，观察失配束流的包络拍频；再加上流强 \`I\`，比较有无空间电荷时的包络。

\`\`\`avas-lab
fodo Lq=0.1 G=6 Ld=0.2 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
\`\`\`

## 延伸阅读 {#quadrupole-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.（第 7 章：四极铁聚焦、FODO 与周期解）。
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed.（多极场展开、四极铁设计与边缘场）。
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific (2019)（Hill 方程、强聚焦与稳定性）。
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1–48（交变梯度聚焦的经典论文）。
- 相关章节：[线性光学](#linear-optics)、[螺线管](#solenoid)、[匹配](#matching)、[lattice 设计](#lattice-design)、[误差与校正](#errors)；完整书目见[参考文献](#references)。
`,E=`# References

This page lists, by topic, the textbooks and papers cited in the primer chapters, each with a one-line note on what to read it for. Bibliographic entries give authors, title, publisher or journal, volume, page and year; DOIs are not listed — search the publisher's site by title when needed. The chapters follow AVAS's actual behaviour; where it differs from textbook conventions, the "AVAS convention" callouts of each chapter say so.

## Textbooks {#references-textbooks}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, Weinheim, 2008. — The standard text on RF linacs: cavities and transit-time factors, longitudinal and transverse dynamics, space charge, the mismatch factor, halo, superconducting linacs; the primer follows its notation.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, Weinheim, 2008. — A systematic treatment of charged-particle beams: periodic focusing, KV and rms envelope equations, space-charge-dominated beams, equipartitioning, free energy of mismatch and emittance growth.
- S. Y. Lee, *Accelerator Physics*, World Scientific (several editions). — A comprehensive textbook with clear treatment of transverse linear optics, Courant–Snyder theory, orbit distortion and correction, and many exercises.
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015. — A comprehensive reference (the 4th edition is open access) with systematic derivations of beam optics, phase space, magnets and errors.
- A. W. Chao, K. H. Mess, M. Tigner, F. Zimmermann (eds.), *Handbook of Accelerator Physics and Engineering*, 2nd ed., World Scientific, 2013. — Handbook of formulas and data, for looking up common formulas and typical parameters.
- S. Humphries, *Charged Particle Beams*, Wiley, 1990. — An introduction to beam transport, focusing elements and space charge, well suited to self-study by graduate students.
- S. Humphries, *Principles of Charged Particle Acceleration*, Wiley, 1986. — An introduction to the principles of accelerators, including electrostatic and RF acceleration and magnetic focusing.

## Linacs, RF and superconducting cavities {#references-linac}

- P. M. Lapostolle, A. L. Septier (eds.), *Linear Accelerators*, North-Holland, Amsterdam, 1970. — The classic collection on linacs, an original source of early design methods and beam dynamics.
- I. M. Kapchinsky, *Theory of Resonance Linear Accelerators*, Harwood Academic, 1985. — A systematic theory of longitudinal and transverse motion in resonance linacs.
- H. Padamsee, J. Knobloch, T. Hays, *RF Superconductivity for Accelerators*, Wiley, 1998 (2nd ed. Wiley-VCH, 2008). — Physics and technology of superconducting cavities: surface resistance, cavity shapes, gradient limits; read it to understand the design constraints of HWR, spoke and elliptical cavities.

## Classic papers on linear optics and phase space {#references-classics}

- H. Busch, "Berechnung der Bahn von Kathodenstrahlen im axialsymmetrischen elektromagnetischen Felde", *Ann. Phys.* 386, 974 (1926). — Busch's theorem: conservation of canonical angular momentum in axisymmetric magnetic fields, the starting point of solenoid focusing and Larmor rotation.
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Ann. Phys.* 3, 1 (1958). — The founding paper of strong focusing: stability of periodic focusing, β functions and Twiss parameters, phase advance.

## Space charge, envelopes, matching and halo {#references-space-charge}

- I. M. Kapchinskij, V. V. Vladimirskij, "Limitations of proton beam current in a strong focusing linear accelerator associated with the beam space charge", in *Proc. Int. Conf. on High Energy Accelerators and Instrumentation*, CERN, Geneva, 1959, p. 274. — The original paper of the KV distribution and the KV envelope equations.
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18, 1105 (1971). — Shows that the rms envelope equations hold for any elliptically symmetric distribution; the theoretical basis of rms matching and rms-equivalent beams.
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18, 1101 (1971). — Definition of the rms emittance and emittance growth through space-charge filamentation.
- I. Hofmann, L. J. Laslett, L. Smith, I. Haber, "Stability of the Kapchinskij-Vladimirskij (K-V) distribution in long periodic transport systems", *Part. Accel.* 13, 145 (1983). — Instabilities of KV beams in periodic channels, one theoretical source of the phase-advance rules.
- J. Struckmeier, M. Reiser, "Theoretical studies of envelope oscillations and instabilities of mismatched intense charged-particle beams in periodic focusing channels", *Part. Accel.* 14, 227 (1984). — Envelope oscillation modes and the envelope instability, the origin of the $\\sigma_0 < 90^\\circ$ rule.
- I. Hofmann, "Stability of anisotropic beams with space charge", *Phys. Rev. E* 57, 4713 (1998). — Space-charge coupling resonances of anisotropic beams and the stability charts (Hofmann charts).
- I. Hofmann, J. Qiang, R. D. Ryne, "Collective resonance model of energy exchange in 3D nonequipartitioned beams", *Phys. Rev. Lett.* 86, 2313 (2001). — Emittance exchange in 3D non-equipartitioned beams and its resonance model; read it to understand when the equipartitioning rule is needed.
- R. L. Gluckstern, "Analytic model for halo formation in high current ion linacs", *Phys. Rev. Lett.* 73, 1247 (1994). — The analytic model explaining halo by the particle–core resonance.
- T. P. Wangler, K. R. Crandall, R. Ryne, T. S. Wang, "Particle-core model for transverse dynamics of beam halo", *Phys. Rev. ST Accel. Beams* 1, 084201 (1998). — A systematic study of the particle–core model and the relation between mismatch and maximum halo amplitude.
- K. R. Crandall, D. P. Rusthoi, *TRACE 3-D Documentation*, 3rd ed., Los Alamos report LA-UR-97-886, 1997. — Manual of the classic 3D linear envelope code, with its conventions for the mismatch factor, matching algorithms and element matrices; useful for comparison with AVAS's linear preview.

## Errors and correction {#references-errors}

- T. O. Raubenheimer, R. D. Ruth, "A dispersion-free trajectory correction technique for linear colliders", *Nucl. Instrum. Methods A* 302, 191 (1991). — A representative paper on trajectory correction in linacs (for electron linear colliders); read it for the basic ideas of correction algorithms.
- Linear error propagation, tolerances and one-to-one orbit correction: see the corresponding chapters of Wangler and Lee, and the estimates in the [errors and correction](#errors) chapter.

## AVAS {#references-avas}

- C. Jin, Z.-J. Wang, X. Qi, Y. He, K. Li, et al., Phys. Rev. Accel. Beams 28, 044602 (2025) — the AVAS reference (listed in the "About AVAS" dialog); please cite it when publishing results obtained with AVAS.
- AVAS user manual \`docs/使用说明20260427.docx\` — the physical original reference for keywords, parameters, units and file formats; parameter meanings follow it and are collected in the in-app "Parameters and files" reference and in \`avas/data/schema.py\`.
- The in-app user manual (Help menu; "Operation guide / Case tutorials / Parameters and files") — GUI operation, verified cases and their run conditions.
- The [first chapter](#intro) of this primer introduces the parts of AVAS and the reading order of the primer.

## Lecture notes and online resources {#references-online}

- CERN Accelerator School (CAS) proceedings, [https://cas.web.cern.ch](https://cas.web.cern.ch) — lecture notes on introductory and intermediate accelerator physics, RF, intense beams and other topics, freely available.
- US Particle Accelerator School (USPAS) course materials, [https://uspas.fnal.gov](https://uspas.fnal.gov) — lecture notes and exercises from courses on linacs, beam physics and more.
- JACoW conference proceedings, [https://www.jacow.org](https://www.jacow.org) — full texts of LINAC, IPAC, HB and other conference papers, for design and error-study examples of specific accelerators.
`,D=`# 参考文献

本页按主题列出入门各章引用的教材与论文，每条附一句说明"读它来做什么"。书目信息只列出作者、题名、出版者 / 期刊、卷、页和年份；没有列出 DOI，需要时请在出版者网站按题名检索。章节内容以 AVAS 实际行为为准，与教科书约定不同之处已在各章的"AVAS 约定"中说明。

## 教材 {#references-textbooks}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, Weinheim, 2008.——射频直线加速器的标准教材：腔体与渡越时间因子、纵向与横向动力学、空间电荷、失配因子、束晕、超导直线加速器；入门各章的记号与之一致。
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, Weinheim, 2008.——带电粒子束理论的系统论述：周期聚焦、KV 与 rms 包络方程、空间电荷主导束、能量均分、失配的自由能与发射度增长。
- S. Y. Lee, *Accelerator Physics*, World Scientific（有多个版本）.——加速器物理的综合教材，横向线性光学、Courant–Snyder 理论、轨道畸变与校正讲得清楚，配有大量习题。
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015.——内容全面的参考书（第 4 版开放获取），束流光学、相空间、磁铁与误差的系统推导。
- A. W. Chao, K. H. Mess, M. Tigner, F. Zimmermann (eds.), *Handbook of Accelerator Physics and Engineering*, 2nd ed., World Scientific, 2013.——公式与数据手册，查常用公式和典型参数。
- S. Humphries, *Charged Particle Beams*, Wiley, 1990.——束流传输、聚焦元件和空间电荷的入门讲解，适合研究生自学。
- S. Humphries, *Principles of Charged Particle Acceleration*, Wiley, 1986.——各类加速器原理的入门，含静电与射频加速、磁场聚焦。

## 直线加速器、射频与超导腔 {#references-linac}

- P. M. Lapostolle, A. L. Septier (eds.), *Linear Accelerators*, North-Holland, Amsterdam, 1970.——直线加速器的经典文集，早期设计方法与束流动力学的原始来源。
- I. M. Kapchinsky, *Theory of Resonance Linear Accelerators*, Harwood Academic, 1985.——共振直线加速器中纵向与横向运动的系统理论。
- H. Padamsee, J. Knobloch, T. Hays, *RF Superconductivity for Accelerators*, Wiley, 1998（第 2 版 Wiley-VCH, 2008）.——超导腔的物理与技术：表面电阻、腔型、梯度限制；读来理解 HWR、spoke、椭球腔的设计约束。

## 线性光学与相空间的经典论文 {#references-classics}

- H. Busch, "Berechnung der Bahn von Kathodenstrahlen im axialsymmetrischen elektromagnetischen Felde", *Ann. Phys.* 386, 974 (1926).——Busch 定理：轴对称磁场中正则角动量守恒，螺线管聚焦与 Larmor 旋转的出发点。
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Ann. Phys.* 3, 1 (1958).——强聚焦理论的奠基论文：周期聚焦的稳定条件、β 函数与 Twiss 参数、相移。

## 空间电荷、包络、匹配与束晕 {#references-space-charge}

- I. M. Kapchinskij, V. V. Vladimirskij, "Limitations of proton beam current in a strong focusing linear accelerator associated with the beam space charge", in *Proc. Int. Conf. on High Energy Accelerators and Instrumentation*, CERN, Geneva, 1959, p. 274.——KV 分布与 KV 包络方程的原始论文。
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18, 1105 (1971).——证明 rms 包络方程对任意椭圆对称分布成立，rms 匹配和 rms 等效束的理论基础。
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18, 1101 (1971).——rms 发射度的定义与空间电荷丝化引起的发射度增长。
- I. Hofmann, L. J. Laslett, L. Smith, I. Haber, "Stability of the Kapchinskij-Vladimirskij (K-V) distribution in long periodic transport systems", *Part. Accel.* 13, 145 (1983).——周期通道中 KV 束的不稳定性，相移选择规则的理论来源之一。
- J. Struckmeier, M. Reiser, "Theoretical studies of envelope oscillations and instabilities of mismatched intense charged-particle beams in periodic focusing channels", *Part. Accel.* 14, 227 (1984).——包络振荡模式与包络不稳定性，$\\sigma_0 < 90^\\circ$ 规则的来源。
- I. Hofmann, "Stability of anisotropic beams with space charge", *Phys. Rev. E* 57, 4713 (1998).——非各向同性束的空间电荷耦合共振与稳定性图（Hofmann 图）。
- I. Hofmann, J. Qiang, R. D. Ryne, "Collective resonance model of energy exchange in 3D nonequipartitioned beams", *Phys. Rev. Lett.* 86, 2313 (2001).——三维非均分束的发射度交换与共振模型，读来理解能量均分规则何时必要。
- R. L. Gluckstern, "Analytic model for halo formation in high current ion linacs", *Phys. Rev. Lett.* 73, 1247 (1994).——粒子–核共振解释束晕的解析模型。
- T. P. Wangler, K. R. Crandall, R. Ryne, T. S. Wang, "Particle-core model for transverse dynamics of beam halo", *Phys. Rev. ST Accel. Beams* 1, 084201 (1998).——粒子–核模型的系统研究，失配与最大晕振幅的关系。
- K. R. Crandall, D. P. Rusthoi, *TRACE 3-D Documentation*, 3rd ed., Los Alamos report LA-UR-97-886, 1997.——经典三维线性包络程序的说明书，含失配因子、匹配算法和元件矩阵的约定，可与 AVAS 线性预览对照。

## 误差与校正 {#references-errors}

- T. O. Raubenheimer, R. D. Ruth, "A dispersion-free trajectory correction technique for linear colliders", *Nucl. Instrum. Methods A* 302, 191 (1991).——直线加速器轨道校正方法的代表性论文（针对电子直线对撞机），读来了解校正算法的基本思想。
- 误差的线性传播、公差与一对一轨道校正：见 Wangler 与 Lee 的教材相应章节，以及 [误差与校正](#errors) 一章的估计。

## AVAS {#references-avas}

- C. Jin, Z.-J. Wang, X. Qi, Y. He, K. Li, et al., Phys. Rev. Accel. Beams 28, 044602 (2025)——AVAS 的参考文献（"关于 AVAS"对话框中列出的文献），使用 AVAS 发表结果时请引用。
- AVAS 使用说明 \`docs/使用说明20260427.docx\`——关键字、参数、单位与文件格式的物理原始参考；参数含义以此为准，并已整理到应用内"参数与文件参考"和 \`avas/data/schema.py\`。
- 应用内使用说明（帮助菜单，"操作指南 / 案例教程 / 参数与文件参考"三栏）——界面操作、经过验证的案例及其运行条件。
- 本入门的 [第一章](#intro) 介绍 AVAS 的组成与本入门的阅读顺序。

## 讲义与在线资源 {#references-online}

- CERN Accelerator School（CAS）历届讲义，[https://cas.web.cern.ch](https://cas.web.cern.ch)——入门与中级加速器物理、射频、强流束等专题的讲义，免费获取。
- US Particle Accelerator School（USPAS）课程资料，[https://uspas.fnal.gov](https://uspas.fnal.gov)——直线加速器、束流物理等课程的讲义与习题。
- JACoW 会议论文库，[https://www.jacow.org](https://www.jacow.org)——LINAC、IPAC、HB 等会议论文全文，查具体加速器的设计与误差研究实例。
`,O=`# RF cavities

RF cavities are the only elements of a linac that give the beam energy: the particles "surf" on a resonant electromagnetic field, and the same field also bunches the beam longitudinally and focuses or defocuses it transversely. AVAS does not approximate a cavity by a transfer matrix; it reads the cavity's three-dimensional field map (the \`field\` element, field type 1) and integrates the particle motion step by step in the real field (t-code). This chapter starts from the basic cavity modes, derives the energy gain, the transit-time factor and RF focusing, and ends with the parameters of \`field\`, the phase conventions and phase scans in AVAS.

All numerical examples use the example project \`examples/hwr010\`: protons with injection kinetic energy $W = 1.527\\ \\mathrm{MeV}$, RF frequency $f = 162.5\\ \\mathrm{MHz}$, and the half-wave resonator field map \`hwr010\` (0.21 m long, two accelerating gaps). The numbers were computed with AVAS's own linear envelope preview (\`avas.sim.linear_optics\`); the kernel result is the reference.

## Role and position {#rf-cavity-role}

In a linac the RF cavities do three things:

- **Acceleration**: the synchronous particle gains $\\Delta W = qV_0T\\cos\\varphi_s$ per cavity (derived below).
- **Longitudinal focusing**: with the synchronous phase on the rising slope of the field (in AVAS's cosine convention $-90^\\circ < \\varphi_s < 0$), early particles gain less energy and late particles more, so the bunch oscillates around the synchronous particle in longitudinal phase space instead of spreading out; see [Longitudinal motion](#longitudinal).
- **Transverse defocusing**: by Maxwell's equations, a longitudinally focusing phase necessarily defocuses transversely (the RF form of the Earnshaw / Panofsky theorem), so solenoids or quadrupoles between the cavities have to compensate; see [Solenoid](#solenoid) and [Quadrupole](#quadrupole).

The low-energy part ($\\beta \\lesssim 0.2$) of a superconducting proton linac is usually a periodic structure of "one or a few cavities + one solenoid", exactly what the [tutorial](#tutorial) builds. The relative energy change in a cavity is large and the velocity changes noticeably, so thin-lens and constant-velocity approximations are only rough estimates there; t-code integration through real field maps is needed.

## Resonant cavities and modes {#rf-cavity-modes}

### The TM010 mode of a pillbox

The simplest accelerating cavity is a perfectly conducting cylinder of radius $R$ and length $d$. Solving the wave equation in cylindrical coordinates with zero tangential electric field on the walls, the accelerating TM010 mode has only $E_z$ and $B_\\theta$:

$$
E_z(r,t) = E_0\\,J_0\\!\\left(\\frac{2.405\\,r}{R}\\right)\\cos\\omega t,\\qquad
B_\\theta(r,t) = -\\frac{E_0}{c}\\,J_1\\!\\left(\\frac{2.405\\,r}{R}\\right)\\sin\\omega t
$$

where $2.405$ is the first zero of $J_0$ (so that $E_z = 0$ at $r = R$). The resonant frequency is

$$
f_{010} = \\frac{2.405\\,c}{2\\pi R}
$$

which depends on the radius only, not on the length. The form of $B_\\theta$ follows directly from Faraday's law $\\nabla\\times\\boldsymbol{E} = -\\partial\\boldsymbol{B}/\\partial t$: $(\\nabla\\times\\boldsymbol{E})_\\theta = -\\partial E_z/\\partial r = E_0 k J_1(kr)\\cos\\omega t$ (with $k = 2.405/R$ and $J_0' = -J_1$); integrating over time, the magnetic field varies as $\\sin\\omega t$, $90^\\circ$ out of phase with the electric field. **This is the physical origin of AVAS's convention of cos for the RF electric field and sin for the RF magnetic field** (see [In AVAS](#rf-cavity-in-avas)).

For $f = 162.5\\ \\mathrm{MHz}$ one gets $R = 2.405c/(2\\pi f) \\approx 0.706\\ \\mathrm{m}$, a cavity about 1.4 m across, which is bulky and inefficient for low-energy protons. Low-frequency, low-$\\beta$ structures therefore use TEM-type coaxial resonators.

### Quarter-wave and half-wave resonators

- **Quarter-wave resonator (QWR)**: a coaxial line shorted at one end and open at the other, inner conductor about $\\lambda/4$ long. The beam crosses a drift tube near the open end of the inner conductor, which forms two accelerating gaps. The structure is not up–down symmetric; the field contains a dipole component that deflects the beam vertically depending on the phase.
- **Half-wave resonator (HWR)**: a coaxial line shorted at both ends, inner conductor about $\\lambda/2$ long ($\\lambda/2 \\approx 0.92\\ \\mathrm{m}$ at 162.5 MHz), with the beam crossing in the middle. An HWR is up–down symmetric about the beam axis and has no QWR-like dipole steering, but the field in the cross-section is not fully axisymmetric and contains a quadrupole component.
- **Multi-gap structures**: spoke cavities, elliptical cavities and others have three or more gaps. For AVAS each is "just a field map"; all the physical differences are in the field distribution.

In a two-gap cavity the fields in the two gaps point in opposite directions at any instant. If a particle needs exactly half an RF period to go from the centre of the first gap to the centre of the second, both gaps accelerate it; the corresponding gap-centre distance is $\\beta_g\\lambda/2$, where $\\beta_g$ is the cavity's **geometric beta**.

### The example cavity hwr010

The on-axis $E_z$ of the example map has one peak at $z \\approx 0.074\\ \\mathrm{m}$ and one at $z \\approx 0.140\\ \\mathrm{m}$, of opposite sign, with the zero crossing at $z \\approx 0.104\\ \\mathrm{m}$ (the centre of the cavity): a typical two-gap HWR. The constant-velocity transit-time factor is largest at $\\beta \\approx 0.10$ (see [Energy gain](#rf-cavity-energy-gain)), consistent with the name hwr010; the example beam has $\\beta = 0.057$, far below the geometric beta, which is why the energy gain in this chapter's examples is low and the phase effects are pronounced.

\`\`\`avas-field
map hwr010
\`\`\`

## Quality factor, shunt impedance and peak fields {#rf-cavity-figures}

These quantities decide how much power a cavity needs, how much voltage it gives and where the field limit is. They belong to cavity design; AVAS's beam dynamics does not use them directly, but you need them in mind when choosing Ke.

| Quantity | Definition | Meaning and typical magnitude |
|---|---|---|
| Unloaded quality factor $Q_0$ | $Q_0 = \\omega U / P_c$ | stored energy $U$ over loss per radian; room-temperature copper about $10^4$, superconducting niobium $10^9$–$10^{10}$ |
| Shunt impedance $R_{sh}$ | $R_{sh} = V^2 / P_c$ | voltage per unit wall loss; the linac convention, the circuit convention differs by a factor 2 |
| $R/Q$ | $R/Q = V^2/(\\omega U)$ | depends on the geometry only, not on the material; tens to hundreds of Ω for low-β superconducting cavities |
| Accelerating gradient $E_{acc}$ | $E_{acc} = V_{acc}/L_{eff}$ | for superconducting cavities usually $L_{eff} = N_{gap}\\,\\beta_g\\lambda/2$ |
| $E_{pk}/E_{acc}$, $B_{pk}/E_{acc}$ | surface peak field over gradient | for low-β cavities typically about 4–6 and 5–10 mT/(MV/m); they limit field emission and quench |

Here $V$ usually means the effective voltage $V_0T$ including the transit-time factor. From the definitions the wall loss is $P_c = V^2/[(R/Q)\\,Q_0]$: for a given $R/Q$ the very high $Q_0$ of superconducting cavities brings the wall loss down to the watt level. The power the beam takes from the cavity is $P_b = I\\,V_0T\\cos\\varphi_s$; for the example beam, $I = 0.184\\ \\mathrm{mA}$ and $\\Delta W = 0.34\\ \\mathrm{MeV}$, so $P_b \\approx 63\\ \\mathrm{W}$.

> **AVAS convention**: in AVAS the cavity field amplitude is given directly by Ke (and Kb) and is fixed. The kernel does not model the power source, beam loading, detuning or low-level RF control; field amplitude and phase jitter enter through the error commands \`err_cav_*\`, see [Errors and correction](#errors).

> **Note**: field maps usually cover only the region near the beam aperture (the example only up to $\\lvert x\\rvert, \\lvert y\\rvert \\le 2\\ \\mathrm{cm}$) and not the cavity surface, so $E_{pk}$ cannot be read from an AVAS field map; check that the Ke you use is within the operating range with the $E_{pk}/E_{acc}$ of the cavity design report.

## Energy gain and transit-time factor {#rf-cavity-energy-gain}

### General expression

Let the reference particle reach the cavity entrance at $t_{in}$ and be at $z$ on the axis at time $t(z)$. In AVAS's convention the on-axis field is $K_e E_z(z)\\cos\\varphi(z)$ with

$$
\\varphi(z) = \\omega\\,\\bigl(t(z) - t_{in}\\bigr) + \\varphi_0
$$

where $\\varphi_0$ is the RF phase when the particle reaches the entrance. The energy gain is the integral of the field along the actual trajectory:

$$
\\Delta W = q K_e\\int_0^L E_z(z)\\cos\\varphi(z)\\,\\dd z
$$

Define the two integrals $C = \\int E_z\\cos\\varphi\\,\\dd z$ and $S = \\int E_z\\sin\\varphi\\,\\dd z$, and let

$$
\\varphi_s = \\operatorname{atan2}(S,\\,C),\\qquad V_0T = K_e\\sqrt{C^2+S^2}
$$

Then $\\Delta W = qV_0T\\cos\\varphi_s$ holds exactly. This $\\varphi_s$ is the **synchronous phase** AVAS means for V3 = 0: it measures how far the reference particle is from "the phase of maximum acceleration on this trajectory".

### Constant-velocity approximation and transit-time factor

If the velocity hardly changes in the cavity, $\\varphi(z) = 2\\pi z/(\\beta\\lambda) + \\varphi_0$. With the origin at the cavity centre and a symmetric (or antisymmetric) field one obtains the classical form

$$
\\Delta W = qV_0T(\\beta)\\cos\\varphi_s,\\qquad
V_0 = \\int\\lvert E_z\\rvert\\,\\dd z,\\qquad
T(\\beta) = \\frac{\\left\\lvert\\int E_z(z)\\,\\ee^{\\,i\\,2\\pi z/(\\beta\\lambda)}\\,\\dd z\\right\\rvert}{\\int\\lvert E_z\\rvert\\,\\dd z}
$$

Some common results:

- A single gap of length $g$ with uniform field: $T = \\dfrac{\\sin(\\pi g/\\beta\\lambda)}{\\pi g/\\beta\\lambda}$.
- Two identical gaps of opposite sign with centre distance $d$ (QWR/HWR): $T(\\beta) = T_g(\\beta)\\,\\left\\lvert\\sin\\dfrac{\\pi d}{\\beta\\lambda}\\right\\rvert$, where $T_g$ is the single-gap factor. For $d = \\beta\\lambda/2$ the second factor is 1; this is the meaning of the geometric beta.

For the example map hwr010 ($K_e = 1.36$), $V_0 = K_e\\int\\lvert E_z\\rvert\\,\\dd z \\approx 0.98\\ \\mathrm{MV}$; the constant-velocity $T(\\beta)$ is:

| β | 0.05 | 0.057 (injection) | 0.07 | 0.08 | 0.10 | 0.104 (max) | 0.12 |
|---|---|---|---|---|---|---|---|
| $T$ | 0.21 | 0.40 | 0.65 | 0.75 | 0.82 | 0.82 | 0.80 |

$T(\\beta)$ falls quickly below the geometric beta, so a cavity's velocity acceptance is limited and one cavity type covers only a range of β.

### Deviations at low energy

In the example the kinetic energy rises from 1.527 MeV to about 1.87 MeV in the cavity ($\\beta$ from 0.057 to 0.063), and the constant-velocity approximation clearly fails. Integrating along the real trajectory, the linear preview gives:

| Setting | Entry phase $\\varphi_0$ | $\\Delta W$ / MeV | Resulting $V_0T$ / MV |
|---|---|---|---|
| V3 = 0, $\\varphi_s = 0^\\circ$ | $95.4^\\circ$ | 0.475 | 0.475 |
| V3 = 0, $\\varphi_s = -33^\\circ$ | $52.7^\\circ$ | 0.340 | 0.405 |
| maximum of a V3 = 1 scan | $\\approx 105.5^\\circ$ | 0.482 | — |

So $V_0T$ itself depends on the phase: at different phases the particle gains different energy in the first gap and enters the second gap with a different velocity and at a different time. The maximum energy gain is not at $\\varphi_s = 0$ but at an entry phase about $10^\\circ$ later. At $\\varphi_s = -33^\\circ$ the particle is first decelerated in the front half of the first gap (kinetic energy about 1.44 MeV at $z = 0.05\\ \\mathrm{m}$) and accelerated only afterwards — typical for $\\beta \\ll \\beta_g$.

## Equations of motion and transfer matrix {#rf-cavity-transverse}

### Paraxial fields

Expand the fields near the axis in $r$. From $\\nabla\\cdot\\boldsymbol{E} = 0$ and $\\nabla\\times\\boldsymbol{B} = c^{-2}\\,\\partial\\boldsymbol{E}/\\partial t$, an axisymmetric gap has

$$
E_r \\approx -\\frac{r}{2}\\,\\frac{\\partial E_z}{\\partial z},\\qquad
B_\\theta \\approx \\frac{r}{2c^2}\\,\\frac{\\partial E_z}{\\partial t}
$$

The radial force on the particle is $F_r = q(E_r - v B_\\theta)$. At the gap entrance $\\partial E_z/\\partial z > 0$ and the electric field pushes the particle towards the axis (focusing); at the exit the opposite (defocusing). For a static field and constant velocity the two would cancel exactly; in reality the field changes and the particle accelerates, and the net effect depends on the phase.

### Thin-gap model

Treat a gap as a thin lens at its centre and integrate the radial force over the gap (Wangler, chapter 7):

$$
\\Delta(\\beta\\gamma\\,x') = -\\frac{\\pi\\,q E_0TL\\,\\sin\\varphi}{mc^2\\,\\lambda\\,\\beta^2\\gamma^2}\\;x
$$

and the same for $y$. Here $qE_0TL$ is the energy-gain amplitude of the gap (in MeV, with $q$ in units of $e$) and $\\beta\\gamma$ is taken at the gap centre. In AVAS's cosine convention the bunching phase $\\varphi < 0$ gives $\\sin\\varphi < 0$, so the right-hand side has the sign of $x$: **transverse defocusing**.

The longitudinal action of the same gap: let $z > 0$ mean a particle ahead of the reference particle (arriving earlier, at a smaller phase), $\\Delta\\varphi = -2\\pi z/(\\beta\\lambda)$, so that

$$
\\Delta\\delta = \\frac{2\\pi\\,qE_0TL\\,\\sin\\varphi}{mc^2\\,\\lambda\\,\\beta^3\\gamma}\\;z
$$

For $\\varphi < 0$ an early particle has its $\\delta$ reduced: **longitudinal focusing**. Including the momentum increase from the energy gain, the thin-gap matrix in the $(x, x')$ plane is

$$
M_x = \\begin{pmatrix} 1 & 0 \\\\[2pt] -\\dfrac{\\pi\\,qE_0TL\\sin\\varphi}{mc^2\\lambda\\,\\bar\\beta^2\\bar\\gamma^2\\,(\\beta\\gamma)_{out}} & \\dfrac{(\\beta\\gamma)_{in}}{(\\beta\\gamma)_{out}} \\end{pmatrix}
$$

The ratio in the lower right corner is **adiabatic damping**: $x'$ shrinks as the momentum grows, the geometric emittance decreases as $1/(\\beta\\gamma)$ and the normalized emittance stays constant.

### The constraint coupling the planes

For a periodic arrangement averaged per unit length, the longitudinal and RF transverse focusing constants (Wangler's notation) are

$$
k_{l0}^2 = \\frac{2\\pi\\,qE_0T\\sin(-\\varphi_s)}{mc^2\\,\\lambda\\,\\beta^3\\gamma^3},\\qquad
k_{t,\\mathrm{rf}}^2 = -\\frac{\\pi\\,qE_0T\\sin(-\\varphi_s)}{mc^2\\,\\lambda\\,\\beta^3\\gamma^3} = -\\frac{k_{l0}^2}{2}
$$

Longitudinal stability ($k_{l0}^2 > 0$) therefore implies transverse defocusing, both proportional to $\\sin(-\\varphi_s)$ and $1/(\\beta\\gamma)^3$. At low energy, a larger $\\lvert\\varphi_s\\rvert$ means stronger bunching and requires stronger transverse focusing; this is one of the reasons why low-energy sections use lower frequencies and phases around $-30^\\circ$ as a compromise; see [Longitudinal motion](#longitudinal) and [Lattice design](#lattice-design).

### Orders of magnitude for the example

For the example take $qE_0TL \\approx 0.20\\ \\mathrm{MeV}$ per gap (half of the 0.405 MeV of both gaps at $\\varphi_s = -33^\\circ$), $\\beta\\gamma = 0.0571$, $\\lambda = 1.845\\ \\mathrm{m}$. The thin-gap formula gives $1/f \\approx 1.1\\ \\mathrm{m^{-1}}$ per gap and about $2\\ \\mathrm{m^{-1}}$ for the whole cavity (a defocusing focal length of about 0.5 m). This is only an estimate; the rms sizes of the linear preview are in [Parameters](#rf-cavity-parameters).

> **AVAS convention**: the kernel does not use the thin-gap matrix above; it integrates the Lorentz force directly in the field map's $\\boldsymbol{E}(x,y,z)$ and $\\boldsymbol{B}(x,y,z)$ (t-code), so non-linearities, transverse–longitudinal coupling and the non-axisymmetric parts of an HWR are all included. The linear preview cuts the map into 4 mm slices and uses the near-axis field gradients and a linearized RF bunching.

## In AVAS {#rf-cavity-in-avas}

An RF cavity is a field-map element \`field\` with field type 1:

\`\`\`text
field  L  R  V3  1  f  phase  Ke  Kb  file
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
\`\`\`

\`\`\`avas-params
field
\`\`\`

Item by item:

- \`L\` (m): element length; it should equal the length in the field-map header (0.21 m in the example). \`R\` (m): aperture radius; a particle beyond it counts as lost.
- \`V3\`: sets the meaning of \`phase\` (see below).
- \`f\` (Hz): the RF frequency of this cavity, \`162.5e6\` in the example. The beam (bunch repetition) frequency is given separately by \`frequency\` in beam.txt; the two are set independently.
- \`phase\` (degrees): meaning set by V3.
- \`Ke\`, \`Kb\`: scale factors of the electric and magnetic field; \`file\`: field-map name (without extension). An RF cavity reads \`.edx/.edy/.edz\` and \`.bdx/.bdy/.bdz\`; the format is in [Field maps](#field-maps).

> **AVAS convention (verified with the kernel)**: electric fields in a map are in MV/m and multiplied by Ke; magnetic fields are in T and multiplied by Kb. The RF electric field acts as $K_e\\,\\boldsymbol{E}(\\boldsymbol{r})\\cos(\\omega t+\\varphi_0)$ and the RF magnetic field as $K_b\\,\\boldsymbol{B}(\\boldsymbol{r})\\sin(\\omega t+\\varphi_0)$, consistent with the $90^\\circ$ between E and B in the [pillbox mode](#rf-cavity-modes).

### V3: phase reference

| V3 | Meaning of \`phase\` | Use |
|---|---|---|
| 0 | synchronous phase $\\varphi_s = \\operatorname{atan2}(\\int E\\sin\\varphi\\,\\dd z,\\ \\int E\\cos\\varphi\\,\\dd z)$; the kernel solves for the entry phase | design: give $\\varphi_s$ directly; it is kept when upstream elements change |
| 1 | RF phase $\\varphi_0$ when the reference particle reaches the element entrance | phase scans, reproducing a given entry phase |
| 2 | absolute RF phase at $t = 0$; entry phase $\\varphi_0 = \\varphi + 360^\\circ\\cdot f\\cdot t_{in}$ | modelling a real machine: phases locked to the master clock, so an upstream energy change shifts the arrival time and the actual $\\varphi_s$ |

The kernel writes the time and energy of the reference particle at every element entrance, together with $\\varphi_s$ and $\\varphi_{RF}$, to \`synData.txt\`; whichever one you set, it computes the other. When a segment run takes a middle section on its own, V3 = 2 phases must be converted with $\\varphi_{RF} - 360^\\circ\\cdot f\\cdot(t_{in} - T_{entry})$; AVAS's segment runs do this automatically.

### Ke and Kb

- **Ke is the amplitude knob.** In the linear preview $\\Delta W$ is roughly but not strictly proportional to Ke (at low energy the trajectory changes with the amplitude): at $\\varphi_s = -33^\\circ$, Ke = 0.5, 1.0, 1.36, 2.0 give $\\Delta W$ = 0.122, 0.248, 0.340, 0.502 MeV.
- **Kb should scale together with Ke.** The electric and magnetic fields of a cavity are two parts of the same eigenmode; changing Ke without Kb is physically inconsistent. The example uses $K_b = -K_e$: the manual does not explain the minus sign; most likely it is related to the phase/sign convention with which the magnetic field was exported (in the pillbox mode $B_\\theta \\propto -\\sin\\omega t$), so that the exported $\\boldsymbol{B}$ matches AVAS's sin convention. For a map from another source, set the sign of Kb according to that map's export convention and keep a fixed ratio to Ke.
- For low-β protons the RF magnetic field matters much less than the electric field: the force ratio is about $\\beta cB/E$. In the example, switching Kb between 0 and $\\pm 1.36$ changes the exit rms size of the linear preview by less than 0.4 %. The magnetic term grows with velocity.

> **Note**: \`scanphase\` (input.txt) also affects phases: 0 uses the phases in the lattice, 1 lets the kernel scan the cavity phases, 2 reads the entry phase and entry time of every cavity from \`scanData.txt\` (one line per cavity, in the order of the RF cavities in the lattice). The linear preview ignores \`scanphase\`.

## Adjustable parameters and typical values {#rf-cavity-parameters}

| Parameter | Effect | Example / typical range |
|---|---|---|
| \`phase\` (V3 = 0) | synchronous phase: trade-off between acceleration, longitudinal acceptance and transverse defocusing | example $-33^\\circ$; low-energy sections often $-40^\\circ$ to $-20^\\circ$, higher energies closer to the crest |
| \`Ke\` | electric field amplitude | example 1.36 ($V_0 \\approx 0.98\\ \\mathrm{MV}$); the limit is set by $E_{pk}$, $B_{pk}$ |
| \`Kb\` | magnetic field amplitude | proportional to Ke, example $-1.36$ |
| \`f\` | cavity frequency | example 162.5 MHz; 162.5 and 325 MHz are common in low-β superconducting sections |
| \`R\` | aperture for the loss criterion | example 0.02 m; should match the real beam pipe / drift-tube aperture |
| input.txt \`steppercycle\` | integration steps per RF period | example 50: $\\beta\\lambda/50 \\approx 2.1\\ \\mathrm{mm}$, comparable to the 2 mm grid of the map |

The example lattice (one cavity, no space charge, linear preview) at different synchronous phases:

| $\\varphi_s$ | $W_{out}$ / MeV | exit rms $x$ / mm | exit rms $z$ / mm |
|---|---|---|---|
| $-60^\\circ$ | 1.694 | 1.96 | 0.62 |
| $-33^\\circ$ | 1.867 | 1.81 | 0.70 |
| $0^\\circ$ | 2.002 | 1.50 | 1.10 |
| $+33^\\circ$ | 1.951 | 1.20 | 1.74 |
| drift of the same length | 1.527 | 1.41 | 1.48 |

The entrance rms sizes are $x \\approx 0.95\\ \\mathrm{mm}$ and $z \\approx 1.32\\ \\mathrm{mm}$, and the initial beam is diverging. The trends agree with the [thin-gap model](#rf-cavity-transverse): the more negative $\\varphi_s$, the stronger the longitudinal compression and the transverse defocusing; for $\\varphi_s > 0$ the beam is focused transversely and debunched longitudinally. At the example current of 0.184 mA, space charge changes the rms sizes at the cavity exit by about 0.3–0.4 % in the linear preview.

## Choosing the phase and phase scans {#rf-cavity-phase}

### Choosing phases in the design

1. Fix $\\varphi_s$ of every cavity from the analysis in [Longitudinal motion](#longitudinal): the longitudinal acceptance (phase width about $3\\lvert\\varphi_s\\rvert$) must hold the bunch, and the longitudinal and transverse phase advances must avoid parametric resonances; see [Lattice design](#lattice-design).
2. In the design phase write $\\varphi_s$ directly with V3 = 0. When upstream elements change, the kernel solves the entry phase again and every cavity keeps its synchronous phase.
3. Once the design is fixed, read the entry RF phases from \`synData.txt\` and switch to V3 = 1 or V3 = 2 where needed, to study phase errors or reproduce machine settings.

### Phase scans

In a real machine the cavity phase is found by a phase scan: the cavity phase is stepped, the exit energy is measured by time of flight between downstream beam position monitors, and from the curve $W_{out}(\\varphi_0)$ the phase of maximum energy (the "crest") is found; the design $\\varphi_s$ is then set as an offset. In AVAS, first set the cavity to V3 = 1, then use the parameter scan on the Run page, or scan the entry phase point by point from the command line:

\`\`\`text
avas scan --input InputFile --target 2 --param phase --values -180:170:36
\`\`\`

\`--target 2\` means line 2 of the lattice file (the cavity's line in the example); an element name works too. \`start:stop:count\` gives 36 points $10^\\circ$ apart; the metric \`energy_out\` is the exit energy. With the curve:

1. Find the maximum. In the example it is near $\\varphi_0 \\approx 105.5^\\circ$ with $\\Delta W_{max} \\approx 0.482\\ \\mathrm{MeV}$; the minimum is near $\\varphi_0 \\approx -105^\\circ$ with $\\Delta W \\approx -0.31\\ \\mathrm{MeV}$; the curve is not symmetric.
2. Note that "$-33^\\circ$ from the crest" and AVAS's $\\varphi_s = -33^\\circ$ are not the same at low energy: in the example the entry phase $95.4^\\circ$ of $\\varphi_s = 0$ is about $10^\\circ$ earlier than the entry phase of maximum energy. This follows directly from the [low-energy deviations](#rf-cavity-energy-gain).
3. When the beam β is close to the geometric beta the two coincide: with an injection energy of 4.727 MeV ($\\beta = 0.10$) the scan curve is nearly symmetric, the maximum $\\Delta W \\approx 0.803\\ \\mathrm{MeV}$ occurs at $\\varphi_s \\approx 0$, and at $\\varphi_s = -33^\\circ$ $\\Delta W \\approx 0.673\\ \\mathrm{MeV}$, exactly $0.803\\cos 33^\\circ$.

The lab below is such a phase-scan curve:

\`\`\`avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
\`\`\`

## Dynamic effects and common problems {#rf-cavity-pitfalls}

- **β mismatch**: when the beam β is clearly below the geometric beta, $T$ is small and the first gap may decelerate first. The example injects $\\beta = 0.057$ into a cavity of geometric beta about 0.10; the cavity gives only about 35 % of $V_0$.
- **Changing Ke but not Kb**: the fields are no longer consistent. The effect is small at low β but can produce spurious transverse forces at high β.
- **Mixing up phase references**: the same number means completely different things for V3 = 0, 1, 2. When importing phases from another code (e.g. TraceWin), check its phase definition and cosine/sine convention first.
- **Phase sign convention**: AVAS uses $\\cos(\\omega t+\\varphi)$ and bunching phases are negative. If a textbook uses a $\\sin$ convention, convert the synchronous phase accordingly.
- **Longitudinal non-linearity**: when the bunch phase width is comparable to $\\lvert\\varphi_s\\rvert$, particles fall outside the separatrix and the longitudinal emittance grows or particles are lost longitudinally. The linear preview cannot show this; use the multi-particle kernel. Longitudinal loss limits can be set with \`longlimits\` in input.txt.
- **Field-map noise**: the example maps are noisy themselves (the on-axis $E_z$ jumps visibly from point to point). This is a property of the data, not a reading error; see [noise in field maps](#field-maps-interpolation).
- **Superposition**: when a cavity overlaps the fringe field of a solenoid use \`superpose\`; at most one RF cavity per block, see [Superpose](#superpose).
- **First and last element**: in multi-particle runs the first and last elements should not be matrix-model elements; an RF cavity is a field-map element and is not affected.

## Hands-on {#rf-cavity-lab}

1. In the phase-scan lab above, drag \`phase\` from $-90^\\circ$ to $0^\\circ$ and watch exit energy, rms $z$ and rms $x/y$ trade off; compare with the table in [Parameters](#rf-cavity-parameters).
2. Lower Ke from 1.36 to 0.7 and watch the synchronous point on the scan curve and the $W(z)$ curve change.
3. Raise the injection energy W to about 4.7 MeV ($\\beta \\approx 0.10$): the scan curve becomes symmetric and the maximum energy gain approaches $V_0T_{max} \\approx 0.80\\ \\mathrm{MeV}$ (linear preview: 0.803 MeV).
4. In the field-map viewer look at \`edz\` (two gaps of opposite sign on the axis) and at the $x$–$y$ cross-sections of \`edx\` and \`edy\`: the transverse electric field points in opposite directions at the gap entrance and exit, the focusing and defocusing halves.

\`\`\`avas-field
map hwr010
\`\`\`

## Further reading {#rf-cavity-reading}

- T. P. Wangler, *RF Linear Accelerators* (2nd ed.), chapters 2–5 (cavities, transit-time factor, shunt impedance) and 6–7 (longitudinal and transverse dynamics, thin-gap formulas).
- H. Padamsee, J. Knobloch, T. Hays, *RF Superconductivity for Accelerators*: $Q_0$, $R/Q$, geometry factor and peak fields of superconducting cavities.
- CAS – CERN Accelerator School lectures on RF cavities and linacs.
- In this primer: [Longitudinal motion](#longitudinal), [Field maps](#field-maps), [Superpose](#superpose), [Errors and correction](#errors), [Tutorial](#tutorial), [References](#references).
`,k=`# 射频腔

射频腔是直线加速器里唯一给束流加能量的元件：它在谐振的电磁场中让粒子"冲浪"，同时还对束流施加纵向聚束和横向（散）焦作用。AVAS 不用传输矩阵近似射频腔，而是读入腔的三维场图（\`field\` 元件，场类型 1），用 t-code 在真实场里逐步积分粒子运动。本章从谐振腔的基本模式讲起，推导能量增益、渡越时间因子和射频聚焦，最后说明 AVAS 中 \`field\` 的参数、相位约定和扫相做法。

本章的数值例子都基于示例项目 \`examples/hwr010\`：质子，注入动能 $W = 1.527\\ \\mathrm{MeV}$，射频频率 $f = 162.5\\ \\mathrm{MHz}$，半波长谐振腔场图 \`hwr010\`（长 0.21 m，两个加速间隙）。数字由 AVAS 自己的线性包络预览（\`avas.sim.linear_optics\`）算出，最终以内核结果为准。

## 作用与位置 {#rf-cavity-role}

在一条直线加速器里，射频腔承担三件事：

- **加速**：同步粒子每通过一个腔获得能量 $\\Delta W = qV_0T\\cos\\varphi_s$（下文推导）。
- **纵向聚焦**：同步相位取在电场上升沿（AVAS 的余弦约定下 $-90^\\circ < \\varphi_s < 0$）时，早到的粒子获得较少能量、晚到的粒子获得较多能量，束团在纵向相空间里绕同步粒子振荡而不散开，参见 [纵向运动](#longitudinal)。
- **横向散焦**：由 Maxwell 方程，纵向聚焦的相位必然伴随横向散焦（Earnshaw / Panofsky 定理的射频形式），因此腔与腔之间要放螺线管或四极铁补偿，参见 [螺线管](#solenoid)、[四极铁](#quadrupole)。

低能段（$\\beta \\lesssim 0.2$）的超导质子直线加速器通常是"一个或几个腔 + 一个螺线管"的周期结构，这正是 [教程](#tutorial) 搭建的结构。束流在腔里的能量相对变化大、速度变化明显，这时薄透镜和恒定速度近似都只能作粗略估计，使用真实场图做 t-code 积分是必要的。

## 谐振腔与工作模式 {#rf-cavity-modes}

### 圆柱腔（pillbox）的 TM010 模

最简单的加速腔是半径 $R$、长度 $d$ 的理想导体圆柱腔。在柱坐标中求解波动方程并要求导体壁上切向电场为零，加速用的 TM010 模只有 $E_z$ 和 $B_\\theta$ 两个分量：

$$
E_z(r,t) = E_0\\,J_0\\!\\left(\\frac{2.405\\,r}{R}\\right)\\cos\\omega t,\\qquad
B_\\theta(r,t) = -\\frac{E_0}{c}\\,J_1\\!\\left(\\frac{2.405\\,r}{R}\\right)\\sin\\omega t
$$

其中 $2.405$ 是 $J_0$ 的第一个零点（保证 $r = R$ 处 $E_z = 0$）。由此得到谐振频率

$$
f_{010} = \\frac{2.405\\,c}{2\\pi R}
$$

它只与半径有关，与腔长无关。$B_\\theta$ 的形式可以直接从 Faraday 定律 $\\nabla\\times\\boldsymbol{E} = -\\partial\\boldsymbol{B}/\\partial t$ 得到：$(\\nabla\\times\\boldsymbol{E})_\\theta = -\\partial E_z/\\partial r = E_0 k J_1(kr)\\cos\\omega t$（$k = 2.405/R$，用了 $J_0' = -J_1$），对时间积分后磁场随 $\\sin\\omega t$ 变化，与电场相差 $90^\\circ$。**这正是 AVAS 对射频电场用 cos、对射频磁场用 sin 的物理来源**（见 [在 AVAS 中](#rf-cavity-in-avas)）。

代入 $f = 162.5\\ \\mathrm{MHz}$ 得 $R = 2.405c/(2\\pi f) \\approx 0.706\\ \\mathrm{m}$，即直径约 1.4 m 的腔——对低能质子而言既笨重又低效。因此低频、低 $\\beta$ 的加速结构采用 TEM 型同轴谐振器。

### 四分之一波长与半波长谐振器

- **四分之一波长谐振器（QWR）**：一端短路、一端开路的同轴线，内导体长约 $\\lambda/4$。束流横穿内导体开路端附近的漂移管，形成两个加速间隙。结构上下不对称，场中含有偶极分量，会对束流产生与相位相关的垂直偏转。
- **半波长谐振器（HWR）**：两端短路的同轴线，内导体长约 $\\lambda/2$（162.5 MHz 时 $\\lambda/2 \\approx 0.92\\ \\mathrm{m}$），束流从中部穿过。HWR 关于束流轴上下对称，没有 QWR 那样的偶极偏转，但横截面内场不完全轴对称，含有四极成分。
- **多间隙结构**：spoke 腔、椭球腔等有三个及以上间隙。对 AVAS 而言它们都只是"一张场图"，物理差别全部包含在场分布里。

在两间隙腔中，同一时刻两个间隙的电场方向相反。粒子从第一个间隙中心走到第二个间隙中心若恰好用去半个射频周期，两个间隙对它都起加速作用，对应的间隙中心距离为 $\\beta_g\\lambda/2$，$\\beta_g$ 称为腔的**几何 β**。

### 示例腔 hwr010

示例场图的轴上 $E_z$ 在 $z \\approx 0.074\\ \\mathrm{m}$ 和 $z \\approx 0.140\\ \\mathrm{m}$ 处各有一个峰，符号相反，过零点在 $z \\approx 0.104\\ \\mathrm{m}$（腔的中心）：这是典型的两间隙 HWR。按恒定速度计算的渡越时间因子在 $\\beta \\approx 0.10$ 处最大（见 [能量增益](#rf-cavity-energy-gain)），与名称 hwr010 一致；而示例束流的 $\\beta = 0.057$，远低于几何 β，这是本章例子中能量增益偏低、相位效应明显的原因。

\`\`\`avas-field
map hwr010
\`\`\`

## 品质因数、分路阻抗与峰值场 {#rf-cavity-figures}

这些量决定一个腔"花多少功率、能加多少电压、场强上限在哪里"，属于腔体设计范畴；AVAS 的束流动力学计算不直接用到它们，但选择 Ke 时必须心中有数。

| 量 | 定义 | 含义与典型量级 |
|---|---|---|
| 固有品质因数 $Q_0$ | $Q_0 = \\omega U / P_c$ | 储能 $U$ 与每弧度损耗之比；常温铜腔约 $10^4$，超导铌腔 $10^9$–$10^{10}$ |
| 分路阻抗 $R_{sh}$ | $R_{sh} = V^2 / P_c$ | 单位壁损耗功率得到的电压；直线加速器常用此约定，电路约定差 2 倍 |
| $R/Q$ | $R/Q = V^2/(\\omega U)$ | 只取决于几何形状，与材料无关；低 β 超导腔为几十到几百 Ω |
| 加速梯度 $E_{acc}$ | $E_{acc} = V_{acc}/L_{eff}$ | 超导腔常取 $L_{eff} = N_{gap}\\,\\beta_g\\lambda/2$ |
| $E_{pk}/E_{acc}$、$B_{pk}/E_{acc}$ | 表面峰值场与梯度之比 | 低 β 腔典型约 4–6 与 5–10 mT/(MV/m)；限制场致发射与失超 |

这里 $V$ 一般指包含渡越时间因子的有效电压 $V_0T$。由定义可知壁损耗 $P_c = V^2/[(R/Q)\\,Q_0]$：在 $R/Q$ 给定时，超导腔的极高 $Q_0$ 让壁损耗降到瓦量级。束流从腔中取走的功率则是 $P_b = I\\,V_0T\\cos\\varphi_s$；示例束流 $I = 0.184\\ \\mathrm{mA}$、$\\Delta W = 0.34\\ \\mathrm{MeV}$，故 $P_b \\approx 63\\ \\mathrm{W}$。

> **AVAS 约定**：AVAS 中腔场幅度由 Ke（与 Kb）直接给定，是固定值。内核不模拟功率源、束流负载、失谐和低电平控制；场幅与相位的抖动通过误差命令 \`err_cav_*\` 引入，参见 [误差与校正](#errors)。

> **注意**：场图通常只覆盖束流孔径附近（示例只到 $\\lvert x\\rvert, \\lvert y\\rvert \\le 2\\ \\mathrm{cm}$），不包含腔表面，因此无法从 AVAS 场图读出 $E_{pk}$；要用腔设计报告给出的 $E_{pk}/E_{acc}$ 检查所用 Ke 是否在可运行范围内。

## 能量增益与渡越时间因子 {#rf-cavity-energy-gain}

### 一般表达式

设参考粒子在 $t_{in}$ 时刻到达腔入口，沿轴走过 $z$ 时的时刻为 $t(z)$。按 AVAS 的约定，轴上电场为 $K_e E_z(z)\\cos\\varphi(z)$，其中

$$
\\varphi(z) = \\omega\\,\\bigl(t(z) - t_{in}\\bigr) + \\varphi_0
$$

$\\varphi_0$ 是粒子到达入口时的射频相位。能量增益是电场沿实际轨迹的积分：

$$
\\Delta W = q K_e\\int_0^L E_z(z)\\cos\\varphi(z)\\,\\dd z
$$

定义两个积分 $C = \\int E_z\\cos\\varphi\\,\\dd z$、$S = \\int E_z\\sin\\varphi\\,\\dd z$，并令

$$
\\varphi_s = \\operatorname{atan2}(S,\\,C),\\qquad V_0T = K_e\\sqrt{C^2+S^2}
$$

就有严格的关系 $\\Delta W = qV_0T\\cos\\varphi_s$。这里的 $\\varphi_s$ 正是 AVAS 中 V3 = 0 时所说的**同步相位**：它度量参考粒子相对于"这条轨迹上的最大加速相位"的偏离。

### 恒定速度近似与渡越时间因子

若粒子在腔内速度近似不变，$\\varphi(z) = 2\\pi z/(\\beta\\lambda) + \\varphi_0$。取腔中心为原点，对对称（或反对称）的场分布可得经典形式

$$
\\Delta W = qV_0T(\\beta)\\cos\\varphi_s,\\qquad
V_0 = \\int\\lvert E_z\\rvert\\,\\dd z,\\qquad
T(\\beta) = \\frac{\\left\\lvert\\int E_z(z)\\,\\ee^{\\,i\\,2\\pi z/(\\beta\\lambda)}\\,\\dd z\\right\\rvert}{\\int\\lvert E_z\\rvert\\,\\dd z}
$$

几个常用结果：

- 长 $g$ 的均匀场单间隙：$T = \\dfrac{\\sin(\\pi g/\\beta\\lambda)}{\\pi g/\\beta\\lambda}$。
- 两个反号、中心距 $d$ 的相同间隙（QWR/HWR）：$T(\\beta) = T_g(\\beta)\\,\\left\\lvert\\sin\\dfrac{\\pi d}{\\beta\\lambda}\\right\\rvert$，其中 $T_g$ 是单间隙的因子。$d = \\beta\\lambda/2$ 时第二个因子为 1，这就是几何 β 的含义。

对示例场图 hwr010（$K_e = 1.36$），$V_0 = K_e\\int\\lvert E_z\\rvert\\,\\dd z \\approx 0.98\\ \\mathrm{MV}$；恒定速度的 $T(\\beta)$ 为：

| β | 0.05 | 0.057（注入） | 0.07 | 0.08 | 0.10 | 0.104（最大） | 0.12 |
|---|---|---|---|---|---|---|---|
| $T$ | 0.21 | 0.40 | 0.65 | 0.75 | 0.82 | 0.82 | 0.80 |

$T(\\beta)$ 在低于几何 β 一侧下降很快，所以腔的速度接受范围有限，一个腔型只能覆盖一段 β 区间。

### 低能时的偏差

示例中束流在腔内动能从 1.527 MeV 升到约 1.87 MeV（$\\beta$ 从 0.057 变为 0.063），恒定速度近似明显失效。线性预览沿真实轨迹积分的结果是：

| 设定 | 入口相位 $\\varphi_0$ | $\\Delta W$ / MeV | 由此得到的 $V_0T$ / MV |
|---|---|---|---|
| V3 = 0，$\\varphi_s = 0^\\circ$ | $95.4^\\circ$ | 0.475 | 0.475 |
| V3 = 0，$\\varphi_s = -33^\\circ$ | $52.7^\\circ$ | 0.340 | 0.405 |
| V3 = 1 扫描的最大值 | $\\approx 105.5^\\circ$ | 0.482 | — |

可见 $V_0T$ 本身随相位变化：不同相位下粒子在第一间隙获得的能量不同，进入第二间隙的速度和时刻也就不同。最大能量增益并不在 $\\varphi_s = 0$，而是在入口相位再晚约 $10^\\circ$ 处。在 $\\varphi_s = -33^\\circ$ 时，粒子在第一间隙前半段先被减速（$z = 0.05\\ \\mathrm{m}$ 处动能约 1.44 MeV），之后才被加速——这是 $\\beta \\ll \\beta_g$ 的典型表现。

## 运动方程与传输矩阵 {#rf-cavity-transverse}

### 近轴场

在轴附近把场按 $r$ 展开。由 $\\nabla\\cdot\\boldsymbol{E} = 0$ 与 $\\nabla\\times\\boldsymbol{B} = c^{-2}\\,\\partial\\boldsymbol{E}/\\partial t$，对轴对称的间隙有

$$
E_r \\approx -\\frac{r}{2}\\,\\frac{\\partial E_z}{\\partial z},\\qquad
B_\\theta \\approx \\frac{r}{2c^2}\\,\\frac{\\partial E_z}{\\partial t}
$$

粒子受到的径向力 $F_r = q(E_r - v B_\\theta)$。在间隙入口 $\\partial E_z/\\partial z > 0$，电场把粒子推向轴（聚焦）；出口处相反（散焦）。若场随时间不变且速度不变，两者恰好抵消；实际上场在变化、粒子在加速，净效果与相位有关。

### 薄间隙模型

把一个间隙视为位于中心的薄透镜，对径向力在间隙内积分（Wangler 第 7 章），得到

$$
\\Delta(\\beta\\gamma\\,x') = -\\frac{\\pi\\,q E_0TL\\,\\sin\\varphi}{mc^2\\,\\lambda\\,\\beta^2\\gamma^2}\\;x
$$

$y$ 方向相同。式中 $qE_0TL$ 是该间隙的能量增益幅度（单位 MeV，$q$ 以 $e$ 计），$\\beta\\gamma$ 取间隙中心的值。在 AVAS 的余弦约定下，聚束相位 $\\varphi < 0$ 使 $\\sin\\varphi < 0$，右边与 $x$ 同号——**横向散焦**。

同一间隙的纵向作用：设 $z > 0$ 表示粒子在参考粒子之前（先到达，相位较小），$\\Delta\\varphi = -2\\pi z/(\\beta\\lambda)$，于是

$$
\\Delta\\delta = \\frac{2\\pi\\,qE_0TL\\,\\sin\\varphi}{mc^2\\,\\lambda\\,\\beta^3\\gamma}\\;z
$$

$\\varphi < 0$ 时提前到达的粒子 $\\delta$ 减小——**纵向聚焦**。把能量增益带来的动量增大也计入，在 $(x, x')$ 平面薄间隙矩阵为

$$
M_x = \\begin{pmatrix} 1 & 0 \\\\[2pt] -\\dfrac{\\pi\\,qE_0TL\\sin\\varphi}{mc^2\\lambda\\,\\bar\\beta^2\\bar\\gamma^2\\,(\\beta\\gamma)_{out}} & \\dfrac{(\\beta\\gamma)_{in}}{(\\beta\\gamma)_{out}} \\end{pmatrix}
$$

右下角的比值就是**绝热阻尼**：$x'$ 随动量增大而减小，几何发射度按 $1/(\\beta\\gamma)$ 缩小，归一化发射度不变。

### 横纵耦合的约束

对周期排列、单位长度平均的情形，纵向与射频横向聚焦常数（Wangler 的记号）为

$$
k_{l0}^2 = \\frac{2\\pi\\,qE_0T\\sin(-\\varphi_s)}{mc^2\\,\\lambda\\,\\beta^3\\gamma^3},\\qquad
k_{t,\\mathrm{rf}}^2 = -\\frac{\\pi\\,qE_0T\\sin(-\\varphi_s)}{mc^2\\,\\lambda\\,\\beta^3\\gamma^3} = -\\frac{k_{l0}^2}{2}
$$

纵向稳定（$k_{l0}^2 > 0$）就必然横向散焦，二者同时正比于 $\\sin(-\\varphi_s)$ 与 $1/(\\beta\\gamma)^3$。因此低能段 $\\lvert\\varphi_s\\rvert$ 越大，聚束越强，所需的横向聚焦也越强；这也是低能段频率较低、相位通常在 $-30^\\circ$ 左右取折中的原因之一，详见 [纵向运动](#longitudinal) 与 [结构设计](#lattice-design)。

### 示例的量级

对示例：每个间隙取 $qE_0TL \\approx 0.20\\ \\mathrm{MeV}$（$\\varphi_s = -33^\\circ$ 时两间隙合计 0.405 MeV 的一半），$\\beta\\gamma = 0.0571$，$\\lambda = 1.845\\ \\mathrm{m}$，由薄间隙公式得每个间隙 $1/f \\approx 1.1\\ \\mathrm{m^{-1}}$，整个腔约 $2\\ \\mathrm{m^{-1}}$（散焦焦距约 0.5 m）。这只是估计；线性预览给出的 rms 尺寸见 [可调参数](#rf-cavity-parameters)。

> **AVAS 约定**：内核不使用上述薄间隙矩阵，而是在场图给出的 $\\boldsymbol{E}(x,y,z)$、$\\boldsymbol{B}(x,y,z)$ 中直接积分 Lorentz 力（t-code），非线性、横纵耦合与 HWR 的非轴对称成分都自然包含在内。线性预览则把场图切成 4 mm 的薄片，用轴附近的场梯度和线性化的射频聚束近似。

## 在 AVAS 中 {#rf-cavity-in-avas}

射频腔用场图元件 \`field\` 描述，场类型写 1：

\`\`\`text
field  L  R  V3  1  f  phase  Ke  Kb  file
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
\`\`\`

\`\`\`avas-params
field
\`\`\`

逐项说明：

- \`L\`（m）：元件长度，应与场图头部记录的长度一致（示例 0.21 m）。\`R\`（m）：孔径半径，粒子超出即判为损失。
- \`V3\`：决定 \`phase\` 的含义（见下）。
- \`f\`（Hz）：该腔的射频频率，示例写 \`162.5e6\`。束流（束团重复）频率另在 beam.txt 的 \`frequency\` 中给出，两者分开设置。
- \`phase\`（度）：含义由 V3 决定。
- \`Ke\`、\`Kb\`：电场与磁场的缩放系数；\`file\`：场图文件名（不含扩展名），射频腔读取 \`.edx/.edy/.edz\` 与 \`.bdx/.bdy/.bdz\`，格式见 [场图](#field-maps)。

> **AVAS 约定（已用内核核实）**：场图中的电场单位为 MV/m，乘以 Ke；磁场单位为 T，乘以 Kb。射频电场按 $K_e\\,\\boldsymbol{E}(\\boldsymbol{r})\\cos(\\omega t+\\varphi_0)$ 作用，射频磁场按 $K_b\\,\\boldsymbol{B}(\\boldsymbol{r})\\sin(\\omega t+\\varphi_0)$ 作用，与 [pillbox 模式](#rf-cavity-modes) 中 E 与 B 相差 $90^\\circ$ 一致。

### V3：相位参考

| V3 | \`phase\` 的含义 | 适用场合 |
|---|---|---|
| 0 | 同步相位 $\\varphi_s = \\operatorname{atan2}(\\int E\\sin\\varphi\\,\\dd z,\\ \\int E\\cos\\varphi\\,\\dd z)$，内核反解入口相位 | 设计阶段：直接指定 $\\varphi_s$，上游改动后仍保持该同步相位 |
| 1 | 参考粒子到达元件入口时的射频相位 $\\varphi_0$ | 扫相、复现某个入口相位 |
| 2 | $t = 0$ 时刻的绝对射频相位，入口相位 $\\varphi_0 = \\varphi + 360^\\circ\\cdot f\\cdot t_{in}$ | 模拟真实机器：相位锁定在主时钟上，上游能量变化会改变到达时间和实际 $\\varphi_s$ |

内核在 \`synData.txt\` 中记录每个元件入口处参考粒子的时间、能量以及 $\\varphi_s$ 与 $\\varphi_{RF}$；设定其中一种时会算出另一种。分段运行把中段单独拿出来时，V3 = 2 的相位要按 $\\varphi_{RF} - 360^\\circ\\cdot f\\cdot(t_{in} - T_{entry})$ 重新换算，AVAS 的分段运行会自动处理。

### Ke 与 Kb

- **Ke 是场幅旋钮**。线性预览中 $\\Delta W$ 与 Ke 近似成正比但不严格（低能时轨迹随场幅变化）：示例在 $\\varphi_s = -33^\\circ$ 时 Ke = 0.5、1.0、1.36、2.0 对应 $\\Delta W$ = 0.122、0.248、0.340、0.502 MeV。
- **Kb 应与 Ke 同比例变化**。腔的电场与磁场是同一个本征模的两部分，只改 Ke 不改 Kb 在物理上不自洽。示例取 $K_b = -K_e$：负号的来历手册没有说明，最可能与场图导出时磁场所取的相位/符号约定有关（pillbox 模中 $B_\\theta \\propto -\\sin\\omega t$），用来使导出的 $\\boldsymbol{B}$ 与 AVAS 的 sin 约定相符。换用其他来源的场图时，应按该场图的导出约定确定 Kb 的符号，并与 Ke 保持固定比值。
- 对低 β 质子，射频磁场的作用远小于电场：力之比约为 $\\beta cB/E$。示例中把 Kb 在 0 与 $\\pm 1.36$ 之间切换，线性预览的出口 rms 尺寸变化小于 0.4 %。速度越高，磁场项越重要。

> **注意**：\`scanphase\`（input.txt）也会影响相位：0 表示按 lattice 中的相位，1 让内核扫描腔相位，2 从 \`scanData.txt\` 读取每个腔的入口相位和入口时间（每行一个腔，顺序与 lattice 中射频腔顺序相同）。线性预览不处理 \`scanphase\`。

## 可调参数与典型取值 {#rf-cavity-parameters}

| 参数 | 作用 | 示例值 / 典型范围 |
|---|---|---|
| \`phase\`（V3 = 0） | 同步相位：在加速、纵向接受度与横向散焦之间折中 | 示例 $-33^\\circ$；低能段常取 $-40^\\circ$ 到 $-20^\\circ$，高能段可更靠近波峰 |
| \`Ke\` | 电场幅度 | 示例 1.36（$V_0 \\approx 0.98\\ \\mathrm{MV}$）；上限由 $E_{pk}$、$B_{pk}$ 决定 |
| \`Kb\` | 磁场幅度 | 与 Ke 同比例，示例 $-1.36$ |
| \`f\` | 腔频 | 示例 162.5 MHz；低 β 超导段常见 162.5、325 MHz |
| \`R\` | 损失判据孔径 | 示例 0.02 m，应与实际束管 / 漂移管孔径一致 |
| input.txt \`steppercycle\` | 每个射频周期的积分步数 | 示例 50：$\\beta\\lambda/50 \\approx 2.1\\ \\mathrm{mm}$，与场图 2 mm 网格相当 |

示例 lattice（单腔，无空间电荷，线性预览）在不同同步相位下的结果：

| $\\varphi_s$ | $W_{out}$ / MeV | 出口 rms $x$ / mm | 出口 rms $z$ / mm |
|---|---|---|---|
| $-60^\\circ$ | 1.694 | 1.96 | 0.62 |
| $-33^\\circ$ | 1.867 | 1.81 | 0.70 |
| $0^\\circ$ | 2.002 | 1.50 | 1.10 |
| $+33^\\circ$ | 1.951 | 1.20 | 1.74 |
| 同长度漂移段 | 1.527 | 1.41 | 1.48 |

入口 rms 尺寸为 $x \\approx 0.95\\ \\mathrm{mm}$、$z \\approx 1.32\\ \\mathrm{mm}$，初始束流是发散的。趋势与 [薄间隙模型](#rf-cavity-transverse) 一致：$\\varphi_s$ 越负，纵向越压缩、横向越散焦；$\\varphi_s > 0$ 时横向聚焦而纵向散束。示例流强 0.184 mA 时，线性预览中空间电荷使单腔出口的 rms 尺寸变化约 0.3–0.4 %。

## 相位的选择与扫相 {#rf-cavity-phase}

### 设计时如何选相位

1. 由 [纵向运动](#longitudinal) 的分析确定各腔的 $\\varphi_s$：纵向接受度（相位宽度约 $3\\lvert\\varphi_s\\rvert$）要容纳束团，同时纵向与横向相移要避开参数共振，参见 [结构设计](#lattice-design)。
2. 设计阶段用 V3 = 0 直接写 $\\varphi_s$。这样修改上游元件时内核会重新反解入口相位，各腔保持设定的同步相位。
3. 定稿后从 \`synData.txt\` 读出各腔的入口 RF 相位，必要时改成 V3 = 1 或 V3 = 2，用来研究相位误差或复现机器设定。

### 扫相

真实机器中腔的相位要通过扫相确定：逐步改变腔相位，用下游束流位置监测器的飞行时间测量出口能量，得到 $W_{out}(\\varphi_0)$ 曲线，找到最大能量对应的相位（"波峰"），再按设计 $\\varphi_s$ 设定偏移。在 AVAS 中先把腔设为 V3 = 1，然后在运行页使用参数扫描，或用命令行逐点扫描入口相位：

\`\`\`text
avas scan --input InputFile --target 2 --param phase --values -180:170:36
\`\`\`

\`--target 2\` 指 lattice 文件的第 2 行（示例中腔所在的行），也可以写元件名；\`start:stop:count\` 给出 36 个间隔 $10^\\circ$ 的点；结果指标中的 \`energy_out\` 即出口能量。得到曲线后：

1. 找出最大值。示例中最大值约在 $\\varphi_0 \\approx 105.5^\\circ$，$\\Delta W_{max} \\approx 0.482\\ \\mathrm{MeV}$；最小值约在 $\\varphi_0 \\approx -105^\\circ$，$\\Delta W \\approx -0.31\\ \\mathrm{MeV}$，曲线并不对称。
2. 注意"相对波峰 $-33^\\circ$"与 AVAS 的 $\\varphi_s = -33^\\circ$ 在低能时并不相同：示例中 $\\varphi_s = 0$ 对应的入口相位 $95.4^\\circ$ 比最大能量的入口相位早约 $10^\\circ$。这是 [低能偏差](#rf-cavity-energy-gain) 的直接后果。
3. 当束流 β 接近几何 β 时两者重合：把注入能量改为 4.727 MeV（$\\beta = 0.10$），扫相曲线基本对称，最大 $\\Delta W \\approx 0.803\\ \\mathrm{MeV}$ 出现在 $\\varphi_s \\approx 0$，$\\varphi_s = -33^\\circ$ 时 $\\Delta W \\approx 0.673\\ \\mathrm{MeV}$，正好等于 $0.803\\cos 33^\\circ$。

下面的实验就是这样一条扫相曲线：

\`\`\`avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
\`\`\`

## 动力学效应与常见问题 {#rf-cavity-pitfalls}

- **β 失配**：束流 β 明显低于腔的几何 β 时，$T$ 很小且第一间隙可能先减速。示例注入 $\\beta = 0.057$、几何 β 约 0.10，单腔只获得 $V_0$ 的约 35 %。
- **只改 Ke 不改 Kb**：电磁场不再自洽。低 β 时影响很小，高 β 时可能产生虚假的横向作用。
- **相位参考混淆**：V3 = 0、1、2 的同一个数字含义完全不同。从别的程序（如 TraceWin）导入相位时先确认对方的相位定义和余弦/正弦约定。
- **相位符号约定**：AVAS 用 $\\cos(\\omega t+\\varphi)$，聚束相位为负值。若参考书用 $\\sin$ 约定，同步相位要相应换算。
- **纵向非线性**：束团相位宽度与 $\\lvert\\varphi_s\\rvert$ 可比时，粒子会落到分界线外，纵向发射度增长或纵向损失，线性预览无法反映，必须用多粒子内核计算；可用 input.txt 的 \`longlimits\` 设定纵向损失边界。
- **场图噪声**：示例场图本身有噪声（轴上 $E_z$ 逐点起伏明显）。这是数据本身的特点，不是读取错误；参见 [场图噪声](#field-maps-interpolation)。
- **叠加场**：腔与螺线管边缘场重叠时用 \`superpose\`，一组叠加场中最多一个射频腔，参见 [叠加场](#superpose)。
- **第一个和最后一个元件**：多粒子计算中首末元件应避免使用矩阵模型元件；射频腔本身是场图元件，没有这一限制。

## 动手实验 {#rf-cavity-lab}

1. 在上面的扫相实验中把 \`phase\` 从 $-90^\\circ$ 拖到 $0^\\circ$，观察出口能量、rms $z$ 与 rms $x/y$ 的此消彼长，并与 [可调参数](#rf-cavity-parameters) 中的表对照。
2. 把 Ke 从 1.36 降到 0.7，观察同步点在扫相曲线上的位置和 $W(z)$ 曲线的变化。
3. 把注入能量 W 提高到约 4.7 MeV（$\\beta \\approx 0.10$），观察扫相曲线变得对称、最大能量增益接近 $V_0T_{max} \\approx 0.80\\ \\mathrm{MeV}$（线性预览：0.803 MeV）。
4. 在场图查看器中分别查看 \`edz\`（轴上两个反号的间隙）与 \`edx\`、\`edy\` 的 $x$–$y$ 横截面：间隙入口与出口处横向电场方向相反，对应聚焦和散焦两半。

\`\`\`avas-field
map hwr010
\`\`\`

## 延伸阅读 {#rf-cavity-reading}

- T. P. Wangler, *RF Linear Accelerators*（第 2 版），第 2–5 章（谐振腔、渡越时间因子、分路阻抗）与第 6–7 章（纵向与横向动力学，薄间隙公式）。
- H. Padamsee, J. Knobloch, T. Hays, *RF Superconductivity for Accelerators*：超导腔的 $Q_0$、$R/Q$、几何因子与峰值场。
- CAS – CERN Accelerator School 讲义中关于射频腔与直线加速器的章节。
- 本手册：[纵向运动](#longitudinal)、[场图](#field-maps)、[叠加场](#superpose)、[误差与校正](#errors)、[教程](#tutorial)、[参考文献](#references)。
`,A="# Multi-particle simulation in practice\n\nThe previous chapters gave the physics of the elements and the linear theory; actual design decisions rest on multi-particle simulations, which track many macro-particles through the real field maps of a beamline, including space charge and non-linearities. This chapter covers what a **trustworthy** multi-particle simulation in AVAS requires: the number of macro-particles and statistical errors, initial distributions, time steps, the space-charge mesh, run settings, output planes and output files, how to judge convergence, the difference between the linear preview and the kernel, single-particle runs, and the most common reasons for failure. For the physics background see [Space charge](#space-charge) and [Longitudinal dynamics](#longitudinal); the overall structure of the input files is in [Accelerators and AVAS](#intro).\n\n## Macro-particles and statistics {#simulation-macroparticles}\n\n### What a macro-particle stands for\n\nA bunch contains $Q/q$ real particles ($Q = I/f$ the bunch charge), typically $10^6$–$10^{10}$. The kernel represents them by `particlenumber` **macro-particles**: each carries the charge $Q/N$ with the charge-to-mass ratio of the real particle, so the motion in external fields is unchanged and only space charge is computed from the total charge. The example project has $Q = 1.133\\ \\mathrm{pC}$ ($7.07\\times 10^6$ protons) and `particlenumber 5260`, so each macro-particle stands for about 1340 protons.\n\n```avas-params\nbeam:particlenumber beam:randomseed beam:current beam:frequency\n```\n\n### Statistical errors\n\nWith a finite number of macro-particles every statistical quantity has a random error. For a Gaussian distribution the relative standard error of an rms size estimated from $N$ samples is about $1/\\sqrt{2N}$:\n\n| $N$ | Relative statistical error of the rms size |\n|---|---|\n| 5260 (example) | 0.97 % |\n| $10^4$ | 0.71 % |\n| $10^5$ | 0.22 % |\n| $10^6$ | 0.07 % |\n\nThe statistical error of the rms emittance is of the same order. The resolution of the loss fraction is $1/N$: in the example, one lost macro-particle is 0.019 %; studying losses below $10^{-4}$ or halo needs far more macro-particles, or a dedicated study of the distribution tails.\n\nAnother error source is **numerical noise**: with far fewer macro-particles than real particles, the space-charge field fluctuates more than in the real beam and causes spurious emittance growth (numerical heating). It decreases as the number of macro-particles per mesh cell increases; see [Space-charge mesh](#simulation-sc-mesh).\n\n> Note: the kernel computes with several threads, and identical inputs give slightly different results from run to run. When comparing two runs (for example to judge the effect of a parameter) use a tolerance; conclusions based on differences smaller than the statistical error are not reliable. `randomseed` in `beam.txt` is the seed used to generate the initial beam.\n\n## Initial distributions {#simulation-distributions}\n\n### Generated from Twiss parameters\n\n`twissx`, `twissy` and `twissz` give $\\alpha$, $\\beta$ and the **normalised rms** emittance of each plane (β in mm/mrad, emittance in π·mm·mrad; the angle of the $z$ plane is $\\delta = \\Delta p/p$), see [Phase space and emittance](#phase-space). `distribution V1 V2` selects the transverse and longitudinal distribution type: KV, GS (Gaussian), PB (parabolic), WB (waterbag).\n\n```avas-params\nbeam:distribution beam:twissx beam:twissz beam:readparticledistribution beam:initpos beam:initmom\n```\n\nFor the same rms parameters the edges and tails of these distributions differ a lot. With the usual definitions (distributions in four-dimensional phase space), the ratio of the projected edge to the rms size is:\n\n| Distribution | Definition in 4D phase space | Edge / rms |\n|---|---|---|\n| KV | uniform on the hyper-ellipsoid surface | 2 |\n| WB (waterbag) | uniform inside the hyper-ellipsoid | $\\sqrt6 \\approx 2.45$ |\n| PB (parabolic) | density $\\propto 1 - r^2/r_{\\max}^2$ | $2\\sqrt2 \\approx 2.83$ |\n| GS (Gaussian) | Gaussian | unbounded (truncated in practice) |\n\n> Note: the manual does not state the dimensionality AVAS uses when generating these distributions, nor where the Gaussian is truncated. When you need to know exactly, analyse `inData.dst` in the output directory (the initial distribution actually used). With strong space charge the distribution shape determines the initial emittance growth (see [Space charge](#space-charge-emittance-growth)); when comparing codes, make sure the distributions agree.\n\n### Reading a particle file\n\n`readparticledistribution` names an existing `.dst` or `.edst` file (for example an RFQ output distribution computed with another code). When a particle file is used, every `beam.txt` keyword except `numofcharge` is ignored; `.edst` can describe mixed beams of several particle species. `initpos` and `initmom` add a position and angle offset to the whole bunch, useful for studying initial off-centring.\n\n## Time steps {#simulation-stepping}\n\n### Steps of the t-code\n\nField-map elements and drifts are integrated in time with the t-code (see [Accelerators and AVAS](#intro-tracking)). `steppercycle V1` in `input.txt` divides one RF period into $V_1$ steps:\n\n$$\n\\Delta t = \\frac{1}{f\\,V_1},\\qquad \\Delta s \\approx \\beta c\\,\\Delta t = \\frac{\\beta\\lambda}{V_1} .\n$$\n\nIn the example, `steppercycle 50` at 162.5 MHz and $\\beta = 0.057$ gives $\\Delta t = 0.123\\ \\mathrm{ns}$ and about 2.1 mm per step, comparable with the 2 mm grid spacing of the `hwr010` map along $z$. The manual writes the formula as \"Δt = 1/T × 1/V1\"; the second parameter V2 is used only for secondary-particle transport ($\\Delta t = V_2/V_b$).\n\n```avas-params\ninput:steppercycle input:sim_type\n```\n\nRules for choosing the step:\n\n- the distance per step must be much smaller than the scale on which the field varies (field-map grid, gap length, $\\beta\\lambda$);\n- much smaller than the transverse, synchrotron and plasma wavelengths (space charge is computed once per step, and too large a step increases the integration error of the space-charge kick);\n- check by doubling the number of steps: when the results no longer change (within the statistical error), the step is small enough.\n\n### z-code elements\n\n`quad`, `solenoid`, `bend`, `edge` and `steerer` are matrix elements that transform the particle coordinates in longitudinal position with the z-code; the t-code records the bunch at the synchronous particle's time and the z-code at a position (see the manual's description of `Beamset.plt`); how the kernel converts between them at the boundary is not documented. The schema notes for matrix elements: **avoid matrix elements as the first or last element of a multi-particle lattice**; put a short drift at each end.\n\n## Space-charge mesh {#simulation-sc-mesh}\n\nWith `spacecharge 1` the kernel solves for the self field of the beam on a mesh. `numofgrid Nx Ny Nz` sets the number of mesh points and `meshrms V1 V2 V3` the mesh length $L_x = 2V_1\\sigma_x$ (likewise $y$, $z$), so the cell size is\n\n$$\nh_x = \\frac{2V_1\\sigma_x}{N_x} .\n$$\n\n```avas-params\ninput:spacecharge input:scmethod input:numofgrid input:meshrms\n```\n\nThree competing requirements:\n\n- **Coverage**: the mesh should contain nearly all particles. Too small a $V_1$ cuts the tails; for Gaussian beams take $V_1$ of at least 3–5. How particles outside the mesh are treated is not documented in the manual.\n- **Resolution**: the core needs enough mesh points, $h_x$ clearly smaller than $\\sigma_x$; the larger $V_1$, the poorer the resolution for the same $N_x$.\n- **Particles per cell**: the average number of macro-particles per cell is about $N/(N_xN_yN_z)$, more in the core. For example, a $32\\times 32\\times 32$ mesh with 5260 macro-particles holds only 0.16 particles per cell on average, and the field is very noisy; such a mesh needs $10^5$–$10^6$ macro-particles.\n\n`scmethod` can be `FFT`, `PICNIC` or `SPICNIC`; the manual only lists the names. After changing the mesh or the solver, compare results as described in [Judging convergence](#simulation-convergence).\n\n## Run settings {#simulation-settings}\n\n```avas-params\ninput:multithreading input:dumpperiodicity input:boundary input:longlimits input:pchistogram input:scanphase\n```\n\n- **Multithreading**: write `multithreading 1` to use several threads; for a single thread **leave the line out**. The manual says 0 means off, but with `multithreading 0` (or 2, 3, ...) the kernel loses every particle at the start (verified with the example project). The settings page of the GUI already handles this.\n- **Loss boundaries**: a particle outside the element aperture `R` counts as lost; with `boundary 1` the boundaries in `boundary.txt` are used instead, the lattice radii no longer apply, and lost particles are written to `CollisionData.txt`. `longlimits 1 phase energy` enables longitudinal loss limits, corresponding to the separatrix of [Longitudinal dynamics](#longitudinal-separatrix).\n- **Particle data**: `dumpperiodicity V1` writes all particles to `Beamset.plt` every $V_1$ steps (0 = never); the file can be large. `pchistogram 1 bins` writes `pchistogram.dat`.\n- **Phase scan**: `scanphase` controls the phase scan of the RF cavities or reads the phases from `scanData.txt` (see [Longitudinal dynamics](#longitudinal-avas)).\n- **Model**: `sim_type mulp` is the multi-particle model, `env` the envelope model.\n- **GPU**: the GPU version of AVAS exists only as a pre-compiled Linux library; on Windows, use multithreaded CPU runs.\n\n## Output planes and output files {#simulation-output}\n\n### Output planes\n\n```avas-params\noutputplane automaticoutput\n```\n\n- `outputplane V1` dumps the beam distribution $V_1$ downstream (positive) or upstream (negative) of the keyword's position, as `outData_<position>.dst`.\n- `automaticoutput V1 V2 V3` inserts output planes starting at relative position $V_1$, spaced by $V_2$, over a maximum span $V_3$.\n- An output plane **beyond the end of the lattice** is an error; one **within 5 mm of the end** also fails, while 2 cm before the end works. The kernel always writes `outData_<end>.dst` at the end of the lattice, so no output plane is needed there.\n\n### Main output files\n\n| File | Content |\n|---|---|\n| `DataSet.txt` | bunch statistics, one row per recorded step (table below) |\n| `inData.dst` | the initial distribution actually used |\n| `outData_<z>.dst` | particle distribution at an output plane (binary) |\n| `Phase.txt` | time, position and synchronous-particle energy at entry and exit of every RF cavity |\n| `synParticle.txt` | synchronous-particle data along the line |\n| `Beamset.plt` | all particle data when `dumpperiodicity` is on |\n| `SingleParticle.txt` | trajectory of a single-particle run |\n\n### DataSet.txt\n\n41 columns per row (numbered from 0); the most used columns:\n\n| Column | Content |\n|---|---|\n| 0 | mean kinetic energy (MeV) |\n| 1, 3, 5 | centroid $x$, $y$, $z$ relative to the synchronous particle |\n| 7–9, 10–12, 13–15 | $\\alpha$, $\\beta$, emittance of the three planes |\n| 16, 18, 20 | rms $x$, $y$, $z$ (m) |\n| 22, 24, 26 | maximum $x$, $y$, $z$ |\n| 28 | surviving macro-particles |\n| 29, 31, 33 | synchronous particle $x$, $y$, $z$ |\n| 35 | flag: 0 straight section, 1 curved section, 2 invalid (skip) |\n| 40 | time |\n\nIn straight sections the longitudinal position is column 5 plus column 33; with dipoles in the line the arc length of curved sections has to be accumulated (AVAS's readers do this). The kernel recreates the file when a run starts and appends rows one by one; when reading a file that is still being written, drop an incomplete last line.\n\n> AVAS convention: runs **never write InputFile**. A full run copies the text inputs to `<output directory>/inputs/`, and the kernel uses the copies as input with the original directory as field-map directory, so every output directory carries a snapshot of its inputs. A full run overwrites OutputFile each time; results worth keeping must be \"kept\" in the GUI (copied to the project's `Runs/`).\n\n## Judging convergence {#simulation-convergence}\n\nA multi-particle result is credible only if it is shown to be insensitive to the numerical parameters. A recommended procedure:\n\n1. First get the lattice working with the linear preview and few macro-particles ($10^3$–$10^4$); check that energies, phases and envelopes are roughly right.\n2. **Macro-particles**: at the design parameters increase `particlenumber` by a factor of 4 or more (e.g. $10^4 \\to 10^5$) and compare exit rms sizes, rms emittance growth and losses. Statistical errors fall as $1/\\sqrt N$; if the emittance growth drops markedly as $N$ grows, the original result was polluted by numerical noise.\n3. **Step**: double `steppercycle` (e.g. 50 → 100); results should stay the same within the statistical error.\n4. **Mesh**: vary `numofgrid` and `meshrms` together while keeping enough macro-particles per cell; if needed, compare different `scmethod` choices.\n5. **Repeat runs**: run the same input two or three times to estimate the run-to-run scatter, and use it as the tolerance for \"changed or not\".\n\nThe criterion depends on the question: matching and envelope design care about rms sizes (at the 1 % level); emittance growth about the relative growth; losses and halo need high statistics, the maximum sizes and the tail distribution. When reporting results, state the number of macro-particles, the step and the mesh used.\n\n## Linear preview and kernel {#simulation-preview}\n\nThe **linear envelope preview** of the visual editor (`avas.sim.linear_optics`) pushes the second moments of the beam with 6×6 matrices: the reference particle's energy and time are integrated through the field maps, the maps are cut into 4 mm slices for the linear focusing and bunching, and space charge is a linear kick of the rms-equivalent uniform ellipsoid (see [Space charge](#space-charge-avas)). It responds almost instantly while parameters are dragged and suits matching and early design.\n\n| Aspect | Linear preview | Multi-particle kernel |\n|---|---|---|\n| Energy | error about 0.01 % | reference |\n| rms sizes | usually within 1–2 %, up to 13 % with strong multi-particle effects | reference |\n| Non-linearity, emittance growth, halo | not included | included |\n| Losses | only a warning when 3 × rms exceeds the aperture | particle by particle |\n| Dipole / steering content of field maps | deflection not modelled (warning) | included |\n\n> Note: the preview contains neither the longitudinal non-linearity of low-energy cavities, nor chromatic effects, nor `superposeout` offsets, nor `scanphase`. Design conclusions rest on the kernel; a large difference between preview and kernel usually indicates strong non-linear or space-charge effects, which is exactly when multi-particle simulation is needed.\n\n## Single-particle runs {#simulation-single-particle}\n\nWith `particlenumber 1` the kernel tracks a single particle and writes `SingleParticle.txt` ($x$, $y$, $z$, $\\gamma\\beta_{x,y,z}$, kinetic energy, time). The multi-particle keywords `twissx`, `twissy`, `twissz`, `current`, `distribution` and so on are ignored; `displacepos` and `displacedpos` set the offset of this particle from the synchronous particle:\n\n```avas-params\nbeam:displacepos beam:displacedpos\n```\n\n> AVAS convention: the kernel **reads `displacepos dx dy dz` in mm** (the manual says m; verified with the kernel); `displacedpos dpx dpy dpz` is in %. In single-particle runs the rms columns of DataSet.txt are NaN; this is normal, look at the trajectory and the synchronous-particle columns instead.\n\nSingle-particle runs are useful for orbit checks: the kick of a steerer, the dispersion of a dipole, the alignment of a field map, or effective transfer-matrix elements from individual offset particles.\n\n## Common problems {#simulation-troubleshooting}\n\n### All particles lost at the start\n\nThe kernel reports `End of simulation, all particles lost`. Check in this order (the first item is verified with the kernel, the others are possible causes worth checking):\n\n- `input.txt` contains `multithreading 0` (or 2, 3): remove the line or set it to 1;\n- the initial beam is too large for the aperture `R` (3 × rms close to the aperture), or `initpos` offsets it too much;\n- `longlimits` is on with too narrow limits;\n- the first element is a matrix element: the manual advises against it without saying what happens; put a short drift in front to rule it out.\n\n### Field map not found\n\nThe last parameter of `field` is the field map's **base name** (without extension); the extensions follow the field type: RF `.edx/.edy/.edz` and `.bdx/.bdy/.bdz`, static electric `.esx/.esy/.esz`, static magnetic `.bsx/.bsy/.bsz`. The field-map directory is InputFile by default and can be set with `[project] fieldSource` in `ini.ini`. The storage order of a map is decided for the whole group of component files (see [Field maps](#field-maps)). The example maps are noisy themselves; stripes along $z$ in slice pictures are data, not a reading error.\n\n### Output plane errors\n\nThe output plane is beyond the end of the lattice or less than 5 mm from it. Move it at least about 2 cm before the end, or use the `outData_<end>.dst` the kernel writes at the end anyway.\n\n### Other\n\n- **Only lines between `start` and `end` are simulated**: only lines after the first `start` and before the first `end` count, and `!` starts a comment. Elements written after `end` are not computed, which typically happens when a lattice was cut short while debugging and never restored.\n- **Energy drops to zero or the beam reverses**: a wrong RF phase (opposite sign of the synchronous phase, a misread V3) decelerates the reference particle; the preview stops there with a warning.\n- **NaN rms columns**: normal in single-particle runs; in multi-particle runs it means too few surviving particles.\n- **Results differ slightly every time**: normal with multithreading; compare within the statistical error.\n\n## Further reading {#simulation-further}\n\n- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: chapter 9 (multiparticle simulation and space charge).\n- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: distribution functions and emittance growth.\n- R. W. Hockney, J. W. Eastwood, *Computer Simulation Using Particles*, Adam Hilger, 1988: particle–mesh methods and numerical noise.\n- In this primer: [Space charge](#space-charge), [Errors and correction](#errors), the [tutorial](#tutorial); the full description of input and output files is in the GUI's user manual (parameter and file reference).\n",j='# 多粒子模拟要点\n\n前面各章给出了元件的物理和线性理论；真正的设计结论要靠多粒子模拟——用大量宏粒子跟踪束流通过实际场图、包含空间电荷和非线性的束线。本章讲在 AVAS 中做一次**可信**的多粒子模拟需要注意什么：宏粒子数与统计误差、初始分布、时间步长、空间电荷网格、运行设置、输出平面与输出文件、如何判断收敛、线性预览与内核的差别、单粒子运行，以及最常见的失败原因。物理背景见 [空间电荷](#space-charge) 和 [纵向动力学](#longitudinal)，输入文件的整体结构见 [加速器与 AVAS](#intro)。\n\n## 宏粒子与统计 {#simulation-macroparticles}\n\n### 宏粒子代表什么\n\n一个束团的真实粒子数是 $Q/q$（$Q = I/f$ 为束团电荷），通常是 $10^6$–$10^{10}$ 量级。内核用 `particlenumber` 个**宏粒子**代表它们：每个宏粒子的电荷为 $Q/N$，荷质比与真实粒子相同，因此外场下的运动不变，只有空间电荷按总电荷计算。示例项目 $Q = 1.133\\ \\mathrm{pC}$（$7.07\\times 10^6$ 个质子），`particlenumber 5260`，每个宏粒子代表约 1340 个质子。\n\n```avas-params\nbeam:particlenumber beam:randomseed beam:current beam:frequency\n```\n\n### 统计误差\n\n宏粒子数有限，所有统计量都有随机误差。对高斯分布，用 $N$ 个样本估计 rms 尺寸的相对标准误差约为 $1/\\sqrt{2N}$：\n\n| $N$ | rms 尺寸的相对统计误差 |\n|---|---|\n| 5260（示例） | 0.97 % |\n| $10^4$ | 0.71 % |\n| $10^5$ | 0.22 % |\n| $10^6$ | 0.07 % |\n\nrms 发射度的统计误差与此同一量级。损失率的分辨率是 $1/N$：示例中丢失一个宏粒子就是 0.019 %，要研究 $10^{-4}$ 以下的损失或束晕，必须用远多于此的宏粒子，或者专门研究分布尾部。\n\n另一个误差来源是**数值噪声**：宏粒子比真实粒子少得多，空间电荷场的涨落比真实束大，会造成虚假的发射度增长（数值加热）。它随每个网格单元里的宏粒子数增加而减小，见 [空间电荷网格](#simulation-sc-mesh)。\n\n> 注意：内核多线程计算，同样的输入每次运行的结果会略有差别。比较两次运行（例如判断某个参数的影响）时要带容差，差别小于统计误差的结论不可靠。`beam.txt` 的 `randomseed` 是生成初始束流的随机数种子。\n\n## 初始分布 {#simulation-distributions}\n\n### 由 Twiss 参数生成\n\n`twissx`、`twissy`、`twissz` 给出每个平面的 $\\alpha$、$\\beta$ 与**归一化 rms** 发射度（β 单位 mm/mrad，发射度 π·mm·mrad；$z$ 平面的角度是 $\\delta = \\Delta p/p$），见 [相空间与发射度](#phase-space)。`distribution V1 V2` 分别选择横向和纵向分布类型：KV、GS（高斯）、PB（抛物线）、WB（水袋）。\n\n```avas-params\nbeam:distribution beam:twissx beam:twissz beam:readparticledistribution beam:initpos beam:initmom\n```\n\n同样的 rms 参数下，不同分布的边界和尾部差别很大。按通常的定义（四维相空间中的分布），某个方向投影的边界与 rms 尺寸之比为：\n\n| 分布 | 四维相空间中的定义 | 边界 / rms |\n|---|---|---|\n| KV | 超椭球面上均匀 | 2 |\n| WB（水袋） | 超椭球内均匀 | $\\sqrt6 \\approx 2.45$ |\n| PB（抛物线） | 密度 $\\propto 1 - r^2/r_{\\max}^2$ | $2\\sqrt2 \\approx 2.83$ |\n| GS（高斯） | 高斯 | 无界（实际按截断） |\n\n> 注意：手册没有说明 AVAS 生成这些分布时使用的维数和高斯分布的截断位置。需要准确知道时，读取输出目录的 `inData.dst`（模拟实际使用的初始分布）统计。空间电荷强时，分布形状决定了初始阶段的发射度增长（见 [空间电荷](#space-charge-emittance-growth)），比较不同程序的结果时要保证分布一致。\n\n### 读入粒子文件\n\n`readparticledistribution` 指定已有的 `.dst` 或 `.edst` 文件（例如其他程序计算的 RFQ 出口分布）。使用粒子文件时，除 `numofcharge` 外 `beam.txt` 的其他关键字都不生效；`.edst` 可以描述多种粒子组成的混合束。`initpos`、`initmom` 给整个束团加位置和角度偏移，可用来研究初始偏心。\n\n## 时间步长 {#simulation-stepping}\n\n### t-code 的步长\n\n场图元件和漂移段用 t-code 按时间积分运动方程（见 [加速器与 AVAS](#intro-tracking)）。`input.txt` 的 `steppercycle V1` 把一个射频周期分成 $V_1$ 步：\n\n$$\n\\Delta t = \\frac{1}{f\\,V_1},\\qquad \\Delta s \\approx \\beta c\\,\\Delta t = \\frac{\\beta\\lambda}{V_1} .\n$$\n\n示例 `steppercycle 50`、162.5 MHz、$\\beta = 0.057$：$\\Delta t = 0.123\\ \\mathrm{ns}$，每步前进约 2.1 mm；而 `hwr010` 场图沿 $z$ 的网格间距是 2 mm，两者相当。手册把公式写成 "Δt = 1/T × 1/V1"；第二个参数 V2 只用于次级粒子输运（$\\Delta t = V_2/V_b$）。\n\n```avas-params\ninput:steppercycle input:sim_type\n```\n\n选择步长的原则：\n\n- 每步的距离要远小于场的变化尺度（场图网格、间隙长度、$\\beta\\lambda$）；\n- 远小于横向振荡波长、同步振荡波长和等离子体波长（空间电荷每步计算一次，步长太大时空间电荷冲量的积分误差增大）；\n- 用加倍步数的方法检验：结果不再变化（在统计误差范围内）时步长足够。\n\n### z-code 元件\n\n`quad`、`solenoid`、`bend`、`edge`、`steerer` 是矩阵元件，用 z-code 按纵向位置变换粒子坐标；t-code 按同步粒子的时刻记录束团，z-code 按位置记录（见手册对 `Beamset.plt` 的说明），两类元件交界处如何转换手册未说明，以内核为准。schema 对矩阵元件的说明是：多粒子 lattice 的**第一个和最后一个元件避免使用矩阵元件**，可以在两端各放一段短漂移。\n\n## 空间电荷网格 {#simulation-sc-mesh}\n\n`spacecharge 1` 时，内核在网格上求解束流自场。`numofgrid Nx Ny Nz` 是网格数，`meshrms V1 V2 V3` 设网格边长 $L_x = 2V_1\\sigma_x$（$y$、$z$ 同理），网格单元的尺寸因此是\n\n$$\nh_x = \\frac{2V_1\\sigma_x}{N_x} .\n$$\n\n```avas-params\ninput:spacecharge input:scmethod input:numofgrid input:meshrms\n```\n\n三个相互制约的要求：\n\n- **覆盖**：网格应盖住几乎全部粒子。$V_1$ 太小会截掉尾部；对高斯分布 $V_1$ 至少取 3–5。超出网格的粒子如何处理手册未说明。\n- **分辨率**：束芯要有足够多的网格点，$h_x$ 应明显小于 $\\sigma_x$；$V_1$ 越大，同样 $N_x$ 下分辨率越差。\n- **每格粒子数**：平均每个单元的宏粒子数约为 $N/(N_xN_yN_z)$，束芯中更多一些。例如 $32\\times 32\\times 32$ 的网格配 5260 个宏粒子，平均每格只有 0.16 个，场的数值噪声很大；这样的网格需要 $10^5$–$10^6$ 个宏粒子。\n\n`scmethod` 可选 `FFT`、`PICNIC`、`SPICNIC`，手册只列出名称。改变网格或算法后，按 [收敛性判断](#simulation-convergence) 的办法比较结果。\n\n## 运行设置 {#simulation-settings}\n\n```avas-params\ninput:multithreading input:dumpperiodicity input:boundary input:longlimits input:pchistogram input:scanphase\n```\n\n- **多线程**：写 `multithreading 1` 使用多线程，单线程就**不写这一行**。手册说 0 表示关闭，但内核遇到 `multithreading 0`（以及 2、3 等）会在起点丢失全部粒子（已用示例项目核实）。界面的设置页已按此处理。\n- **损失边界**：粒子超出元件孔径 `R` 记为损失；`boundary 1` 时改用 `boundary.txt` 中的边界，lattice 中的半径不再生效，损失粒子写入 `CollisionData.txt`。`longlimits 1 相位 能量` 打开纵向损失判定，与 [纵向动力学](#longitudinal-separatrix) 中的分界线概念对应。\n- **粒子数据**：`dumpperiodicity V1` 每 $V_1$ 步把全部粒子写入 `Beamset.plt`（0 表示不写）；文件可能很大。`pchistogram 1 份数` 生成 `pchistogram.dat`。\n- **相位扫描**：`scanphase` 控制射频腔的相位扫描或从 `scanData.txt` 读取相位（见 [纵向动力学](#longitudinal-avas)）。\n- **模型**：`sim_type mulp` 是多粒子模型，`env` 是包络模型。\n- **GPU**：AVAS 的 GPU 版本只有 Linux 上预编译的库，Windows 上用多线程 CPU 计算。\n\n## 输出平面与输出文件 {#simulation-output}\n\n### 输出平面\n\n```avas-params\noutputplane automaticoutput\n```\n\n- `outputplane V1` 在关键字所在位置下游 $V_1$（正值）或上游（负值）输出束流分布，文件为 `outData_<位置>.dst`。\n- `automaticoutput V1 V2 V3` 从相对位置 $V_1$ 开始、间距 $V_2$、最大跨度 $V_3$ 批量插入输出平面。\n- 输出平面**超出 lattice 末端**会报错；**离末端 5 mm 以内**也会失败，离末端 2 cm 可以。内核结束时总会写末端的 `outData_<末端>.dst`，无需在末端另放输出平面。\n\n### 主要输出文件\n\n| 文件 | 内容 |\n|---|---|\n| `DataSet.txt` | 每个记录步一行的束团统计量（下表） |\n| `inData.dst` | 模拟实际使用的初始分布 |\n| `outData_<z>.dst` | 输出平面上的粒子分布（二进制） |\n| `Phase.txt` | 每个射频腔入口、出口的时间、位置与同步粒子能量 |\n| `synParticle.txt` | 同步粒子沿线的信息 |\n| `Beamset.plt` | `dumpperiodicity` 打开时的全部粒子数据 |\n| `SingleParticle.txt` | 单粒子运行的轨迹 |\n\n### DataSet.txt\n\n每行 41 列（从 0 开始编号），常用的列：\n\n| 列 | 内容 |\n|---|---|\n| 0 | 平均动能（MeV） |\n| 1、3、5 | 质心相对同步粒子的 $x$、$y$、$z$ |\n| 7–9、10–12、13–15 | 三个方向的 $\\alpha$、$\\beta$、发射度 |\n| 16、18、20 | rms $x$、$y$、$z$（m） |\n| 22、24、26 | 最大 $x$、$y$、$z$ |\n| 28 | 存活宏粒子数 |\n| 29、31、33 | 同步粒子的 $x$、$y$、$z$ |\n| 35 | 标志：0 直线段，1 曲线段，2 无效（跳过） |\n| 40 | 时间 |\n\n直线段的纵向位置是第 5 列加第 33 列；束线中有二极铁时要按曲线段累加弧长（AVAS 的读取程序已处理）。内核在运行开始时重建这个文件并逐行追加，读取正在写的文件要丢弃最后一行不完整的数据。\n\n> AVAS 约定：运行**不改写 InputFile**。完整运行把文本输入复制到 `<输出目录>/inputs/`，内核以副本为输入、以原目录为场图目录，所以输出目录自带本次运行的输入快照。完整运行每次覆盖 OutputFile，需要保留的结果要在界面中"保留"（复制到项目的 `Runs/`）。\n\n## 收敛性判断 {#simulation-convergence}\n\n一个多粒子结果可信，至少要说明它对数值参数不敏感。建议的做法：\n\n1. 先用线性预览和较少的宏粒子（$10^3$–$10^4$）调通 lattice，确认能量、相位和包络大致合理。\n2. **宏粒子数**：在设计参数下把 `particlenumber` 增加 4 倍以上（例如 $10^4 \\to 10^5$），比较出口 rms 尺寸、rms 发射度增长和损失率。统计误差按 $1/\\sqrt N$ 下降，发射度增长若随 $N$ 增大而明显减小，说明原结果被数值噪声污染。\n3. **步长**：把 `steppercycle` 加倍（例如 50 → 100），结果应在统计误差内不变。\n4. **网格**：同时改变 `numofgrid` 和 `meshrms`，并保持每格宏粒子数足够；必要时比较不同的 `scmethod`。\n5. **重复运行**：同一输入重复两三次，估计运行间的随机差别，作为判断"有无变化"的容差。\n\n判断标准取决于问题：匹配和包络设计关心 rms 尺寸（1 % 量级）；发射度增长关心相对增长量；束损和束晕需要高统计量和最大尺寸、尾部分布。报告结果时同时给出所用的宏粒子数、步长和网格。\n\n## 线性预览与内核 {#simulation-preview}\n\n可视化编辑器的**线性包络预览**（`avas.sim.linear_optics`）用 6×6 矩阵推进束流二阶矩：参考粒子沿场图积分能量和时间，场图按 4 mm 切片计算线性聚焦与聚束，空间电荷用 rms 等效均匀椭球的线性冲量（见 [空间电荷](#space-charge-avas)）。它在拖动参数时几乎即时给出结果，适合调匹配和初步设计。\n\n| 项目 | 线性预览 | 多粒子内核 |\n|---|---|---|\n| 能量 | 误差约 0.01 % | 参考 |\n| rms 尺寸 | 通常差 1–2 %，多粒子效应强时可到 13 % | 参考 |\n| 非线性、发射度增长、束晕 | 不包含 | 包含 |\n| 损失 | 只提示 3 倍 rms 超过孔径 | 逐粒子判定 |\n| 场图的二极 / 校正分量 | 不计偏转（给出提示） | 包含 |\n\n> 注意：预览不包含低能腔中的纵向非线性、色差、`superposeout` 的偏移和 `scanphase`。设计结论以内核为准；预览与内核差别大时，通常说明非线性或空间电荷效应强，正需要多粒子模拟。\n\n## 单粒子运行 {#simulation-single-particle}\n\n`particlenumber 1` 时内核做单粒子模拟，输出 `SingleParticle.txt`（$x$、$y$、$z$、$\\gamma\\beta_{x,y,z}$、动能、时间）。此时 `twissx`、`twissy`、`twissz`、`current`、`distribution` 等多粒子关键字不生效，用 `displacepos` 和 `displacedpos` 设置这个粒子相对同步粒子的偏移：\n\n```avas-params\nbeam:displacepos beam:displacedpos\n```\n\n> AVAS 约定：`displacepos dx dy dz` 内核**按 mm 读取**（手册写 m，已用内核核实）；`displacedpos dpx dpy dpz` 的单位是 %。单粒子运行的 DataSet.txt 中 rms 列为 NaN，这是正常的，要看轨迹和同步粒子的列。\n\n单粒子运行适合检查轨道：校正铁的偏转、二极铁的色散、场图的对中，或者给单个偏离粒子算出等效的传输矩阵元。\n\n## 常见问题 {#simulation-troubleshooting}\n\n### 起点就丢失全部粒子\n\n内核报 `End of simulation, all particles lost`。按以下顺序排查（第一条已用内核核实，其余是值得检查的可能原因）：\n\n- `input.txt` 中有 `multithreading 0`（或 2、3）——删掉这一行或改为 1；\n- 初始束流相对孔径 `R` 太大（3 倍 rms 已接近孔径），或 `initpos` 偏移过大；\n- `longlimits` 打开且边界太窄；\n- 第一个元件是矩阵元件——手册建议避免但没有说明后果，在前面加一段短漂移即可排除。\n\n### 找不到场图\n\n`field` 的最后一个参数是场图**基名**（不含扩展名），文件按场类型取扩展名：射频场 `.edx/.edy/.edz` 与 `.bdx/.bdy/.bdz`，静电场 `.esx/.esy/.esz`，静磁场 `.bsx/.bsy/.bsz`。场图目录默认是 InputFile，可以在 `ini.ini` 的 `[project] fieldSource` 中另设。场图的存储顺序按一组分量文件判断（见 [场图](#field-maps)）。示例场图本身有噪声，切片图沿 $z$ 的条纹是数据而不是读取错误。\n\n### 输出平面报错\n\n输出平面超出 lattice 末端，或离末端不到 5 mm。改到离末端至少约 2 cm 处，或者直接使用内核在末端自动写出的 `outData_<末端>.dst`。\n\n### 其他\n\n- **只有 `start` 到 `end` 之间被模拟**：第一个 `start` 之后、第一个 `end` 之前的行才生效，`!` 之后是注释。写在 `end` 之后的元件不会被计算，常见于调试时临时截断 lattice 后忘记恢复。\n- **能量降到零或束流反向**：射频相位错误（例如同步相位符号相反、V3 理解错误）会让参考粒子减速，预览会在该处停止并给出提示。\n- **rms 列为 NaN**：单粒子运行属正常；多粒子运行中出现说明存活粒子过少。\n- **结果每次略有不同**：多线程计算的正常现象，按统计误差比较。\n\n## 延伸阅读 {#simulation-further}\n\n- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：第 9 章（多粒子模拟与空间电荷）。\n- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：分布函数与发射度增长。\n- R. W. Hockney, J. W. Eastwood, *Computer Simulation Using Particles*, Adam Hilger, 1988：粒子–网格方法与数值噪声。\n- 本入门的 [空间电荷](#space-charge)、[误差与校正](#errors)、[教程](#tutorial)；输入输出文件的完整说明见界面的使用说明（参数与文件参考）。\n',M=`# Solenoid

A solenoid produces a longitudinal magnetic field $B_z$ along the beam. It focuses in both transverse planes **at once**, and its action on a round beam is axially symmetric; the price is that the beam rotates about the axis and the $x$ and $y$ planes are coupled. The focusing strength of a solenoid is proportional to $(B/B\\rho)^2$, which makes it especially suited for low-energy, low-rigidity beams: focusing in low-energy beam transport lines (LEBT) and in low-$\\beta$ superconducting cavity sections is mostly done with solenoids. AVAS's example project comes with a solenoid field map, \`sol\`.

This chapter uses the transfer matrices of [linear optics](#linear-optics) and the focal-length concept of the [quadrupole](#quadrupole) chapter.

## Role and placement {#solenoid-role}

- **Low-energy transport and matching**: in the LEBT after the ion source, 1–3 solenoids converge the diverging beam and match it into the downstream structure.
- **Low-energy section of superconducting linacs**: in low-$\\beta$ sections of half-wave (HWR), quarter-wave (QWR) or spoke cavities, "cavity–solenoid" periods are common. Superconducting solenoids fit into the same cryostat, which keeps the structure compact; they focus both planes at once, so a period needs only one focusing element.
- **Correction**: superconducting solenoids often carry dipole corrector windings (see [steerer](#steerer)).

## Physics and field {#solenoid-field}

### Field of a finite solenoid

In the bore, close to the axis, an axially symmetric static field can be expanded from the on-axis field $B(z) \\equiv B_z(0, z)$ (paraxial expansion, from $\\nabla\\cdot\\boldsymbol{B}=0$ and $\\nabla\\times\\boldsymbol{B}=0$):

$$
B_z(r, z) \\approx B(z) - \\frac{r^2}{4}\\,B''(z),\\qquad
B_r(r, z) \\approx -\\frac{r}{2}\\,B'(z).
$$

In the middle of the solenoid $B(z)$ is nearly constant and $B_r \\approx 0$; at the ends $B(z)$ falls off and produces a **radial fringe field** proportional to the radius. For an ideal solenoid of radius $a$, length $\\ell$, $n$ turns per unit length and current $I$, the on-axis field is

$$
B(z) = \\frac{\\mu_0 n I}{2}\\left[\\frac{z + \\ell/2}{\\sqrt{(z+\\ell/2)^2 + a^2}} - \\frac{z - \\ell/2}{\\sqrt{(z-\\ell/2)^2 + a^2}}\\right];
$$

at the centre it tends to $\\mu_0 n I$ for $\\ell \\gg a$, at the ends it is about half the central value, and the fringe region is of the order of the coil radius.

### Hard-edge model

Replacing $B(z)$ by a rectangular profile of length $L$ and height $B$ (hard edge) turns the radial field into a $\\delta$ function at the ends: at the entrance the particle crosses a thin layer with $\\int B_r\\,\\dd z = -\\tfrac{r}{2}B$, at the exit the sign is reversed. Inside the hard-edge model the **transverse field is zero everywhere**; the focusing comes entirely from the combination "radial edge field × rotation inside the body" (next section).

> AVAS convention: the cross-section schematic of a matrix-model solenoid shows no transverse field and draws no grid arrows — this is a property of the hard-edge model, not missing data. A field-map solenoid (such as the example \`sol\`) has a real fringe field: from $B_r = -\\tfrac{r}{2}B'$, for $B_z>0$ the transverse field $(B_x, B_y)$ points to the axis everywhere at the entrance end, where the field rises, and outwards at the exit end, where it falls; in the central cross-section the transverse field vanishes (checked with the example map: at $x = +1$ cm, $B_x$ is about $-0.55$ at $z = 0.12$ m and about $+0.35$ at $z = 0.23$ m, in map units, times \`Kb\` for T).

### Busch's theorem and canonical angular momentum

In an axially symmetric magnetic field the canonical angular momentum

$$
P_\\theta = \\gamma m r^2 \\dot\\theta + q\\,r A_\\theta = \\gamma m r^2\\dot\\theta + \\frac{q}{2\\pi}\\,\\Psi(r, z)
$$

is conserved, where $\\Psi$ is the magnetic flux inside radius $r$; paraxially $A_\\theta = rB/2$ and $\\Psi = \\pi r^2 B$. If the particle has no angular momentum in the field-free region ($P_\\theta = 0$), then inside the field

$$
\\dot\\theta = -\\frac{q B}{2\\gamma m},\\qquad
\\frac{\\dd\\theta}{\\dd s} = -\\frac{q B}{2p} = -k,
$$

that is, the particle rotates about the axis at the **Larmor frequency** (half the cyclotron frequency). This is Busch's theorem. The rotation rate does not depend on $r$, so the whole beam turns like a rigid body; after the solenoid the rotation stops ($\\dot\\theta=0$ where $B=0$) and only a net rotation angle remains.

## Equations of motion and transfer matrix {#solenoid-matrix}

### Equations of motion

From $\\boldsymbol{F} = q\\boldsymbol{v}\\times\\boldsymbol{B}$, kept to linear terms:

$$
x'' = \\frac{q}{p}\\left(B\\,y' + \\tfrac12 B'\\,y\\right),\\qquad
y'' = -\\frac{q}{p}\\left(B\\,x' + \\tfrac12 B'\\,x\\right).
$$

Define

$$
k(s) = \\frac{q B(s)}{2p} = \\frac{B(s)}{2B\\rho};
$$

with the complex coordinate $u = x + \\mathrm{i}\\,y$ the equations read $u'' + 2\\mathrm{i}\\,k\\,u' + \\mathrm{i}\\,k'\\,u = 0$. Substituting $u = w\\,\\ee^{-\\mathrm{i}\\theta(s)}$ with $\\theta' = k$ cancels the first-derivative terms:

$$
w'' + k^2(s)\\,w = 0 .
$$

In the frame rotating at the Larmor frequency (the **Larmor frame**) the two planes decouple and each is focused with strength $k^2$ — whatever the sign of $k$, $k^2 > 0$, so a solenoid **always focuses both planes**.

### 4×4 matrix of a hard-edge solenoid

For a hard-edge solenoid of length $L$ and field $B$, let $C = \\cos kL$, $S = \\sin kL$. The transfer matrix in laboratory coordinates $(x, x', y, y')$ is

$$
M_{\\text{sol}} =
\\begin{pmatrix}
C^2 & \\dfrac{SC}{k} & SC & \\dfrac{S^2}{k}\\\\[4pt]
-kSC & C^2 & -kS^2 & SC\\\\[4pt]
-SC & -\\dfrac{S^2}{k} & C^2 & \\dfrac{SC}{k}\\\\[4pt]
kS^2 & -SC & -kSC & C^2
\\end{pmatrix}.
$$

This is identical to \`_solenoid4(k, L)\` of AVAS's linear preview ($k = B_z/(2B\\rho)$, entrance and exit edge effects included). The longitudinal block is that of a drift, $R_{56} = L/\\gamma^2$.

It factorizes into "focusing in the Larmor frame" times "rotation":

$$
M_{\\text{sol}} = R(kL)\\,F(kL),\\quad
F = \\begin{pmatrix} F_2 & 0 \\\\ 0 & F_2 \\end{pmatrix},\\quad
F_2 = \\begin{pmatrix} C & S/k \\\\ -kS & C \\end{pmatrix},\\quad
R(\\theta) = \\begin{pmatrix} \\cos\\theta\\, I & \\sin\\theta\\, I \\\\ -\\sin\\theta\\, I & \\cos\\theta\\, I \\end{pmatrix},
$$

with $I$ the $2\\times 2$ unit matrix. $R(\\theta)$ takes the point $(x, y) = (x_0, 0)$ to $(x_0\\cos\\theta, -x_0\\sin\\theta)$: for $qB > 0$, seen from downstream (beam coming towards the viewer), the beam turns **clockwise** by $\\theta = kL$.

### Focal length and rotation angle

- Larmor rotation angle: $\\theta_L = kL$, in general $\\theta_L = \\int k\\,\\dd s = \\dfrac{1}{2B\\rho}\\int B\\,\\dd s$.
- Thick-lens focal length (in the Larmor frame, the same in both planes):

$$
\\frac{1}{f} = k\\sin kL .
$$

- Thin-lens approximation ($kL \\ll 1$):

$$
\\frac{1}{f} \\approx k^2 L = \\left(\\frac{B}{2B\\rho}\\right)^2 L,\\qquad\\text{in general}\\quad \\frac{1}{f} \\approx \\frac{1}{4(B\\rho)^2}\\int B^2\\,\\dd s .
$$

Compare with a quadrupole: there $1/f = G L/B\\rho \\propto 1/B\\rho$, for a solenoid $1/f \\propto 1/(B\\rho)^2$. The lower the rigidity, the more "economical" the solenoid; at high rigidity the field needed for the same focal length grows quickly — which is why solenoids are used mainly at low energy.

### Round beams and projected emittance

For an axially symmetric (round) beam without initial canonical angular momentum, the rotation $R$ does not change the second moments, and the projected rms emittance at the solenoid exit equals that at the entrance. **Inside** the solenoid, however, the mechanical angular momentum is non-zero, just past the entrance $x'$ contains an extra term $+k\\,y$ ($y'$ an extra $-k\\,x$), and the projected emittance

$$
\\varepsilon_x^2 = \\varepsilon_0^2 + k^2 \\langle x^2\\rangle\\langle y^2\\rangle
$$

($\\varepsilon_0$ the entrance emittance, $x$ and $y$ assumed uncorrelated) grows temporarily and recovers after the exit. With a field-map solenoid, the "bump" of the rms emittance in the middle of the solenoid in DataSet.txt has this origin; it is not a loss of beam quality. For non-round beams, or beams with initial coupling, a solenoid causes real projected emittance growth.

## In AVAS {#solenoid-avas}

### Matrix-model solenoid

\`\`\`text
solenoid L R 0 B
\`\`\`

\`\`\`avas-params
solenoid
\`\`\`

- \`L\`: length (m), the hard-edge length.
- \`R\`: aperture radius (m).
- The third parameter is reserved; write 0.
- \`B\`: axial field (T); its sign sets the direction of rotation and does not affect the focusing (which is proportional to $B^2$).

> AVAS convention: $k = B/(2B\\rho)$, with the hard-edge matrix above (\`linear_optics._solenoid4\`). \`solenoid\` is a z-code matrix-model element and, like the others, should not be the first or last element of a multi-particle lattice.

### Field-map solenoid

The fringe field of a real solenoid is wide (comparable to the coil radius) and in compact cavity–solenoid periods often overlaps neighbouring elements, so solenoids are usually described by a static magnetic field map:

\`\`\`text
field 0.35 0.02 0 3 0 0 0 Kb sol
\`\`\`

The example project \`examples/hwr010\` comes with the solenoid map \`sol\` (\`sol.bsx\`, \`sol.bsy\`, \`sol.bsz\`), 0.35 m long, transverse grid $\\pm 0.02$ m. Actual field = map × \`Kb\` (T). The raw maximum of the on-axis $B_z$ of this map is about 10.1 (map units, T) and the 9-point smoothed peak about 8.3, near $z \\approx 0.177$ m, so \`Kb = 0.1\` corresponds to a peak field of about 0.83 T (raw maximum about 1.0 T).

> Note: the example map itself is noisy; in its "flat top" \`sol.bsz\` jumps point by point between 5.9 and 10.1 (AGENTS.md §6). The vertical stripes along $z$ in the slice heat map are the data, not a reading error. Estimate rotation and focal length from integrals ($\\int B\\,\\dd z$, $\\int B^2\\,\\dd z$) rather than from the peak.

Integrating the map gives $\\int B\\,\\dd z \\approx 0.729\\,\\mathrm{Kb}$ T·m and $\\int B^2\\,\\dd z \\approx 4.04\\,\\mathrm{Kb}^2$ T²·m (the noise makes the latter slightly too large). For the example beam ($B\\rho = 0.17863$ T·m):

| \`Kb\` | peak field | Larmor angle | thin-lens focal length |
|---|---|---|---|
| 0.1 | ≈ 0.83 T | 11.7° | 3.16 m |
| 0.2 | ≈ 1.7 T | 23.4° | 0.79 m |

Field-map elements are tracked with the t-code and include the fringe field; the linear preview computes the edge effects from the near-axis gradients of the map, radial components included (see the notes in \`linear_optics.py\`). Map format, storage order and normalisation are covered in [field maps](#field-maps), placing a solenoid on top of a cavity in [superpose](#superpose).

## Parameters and typical values {#solenoid-params}

| Quantity | Typical range | Remark |
|---|---|---|
| field $B$ | room temperature 0.1–0.6 T; superconducting up to several T | short focal lengths at low energy need strong fields |
| length $L$ | 0.1–0.4 m | solenoids inside superconducting cryostats are usually short |
| aperture radius $R$ | 0.01–0.05 m | larger apertures in LEBTs |
| rotation $\\theta_L$ | a few to a few tens of degrees | changes only the orientation of the beam, not the rms size of a round beam |

### Worked example

The example beam (protons at 1.527 MeV, $B\\rho = 0.17863$ T·m) through a hard-edge solenoid:

| $B$ | $L$ | $k$ | $kL$ | thick-lens $f$ | thin-lens $f$ |
|---|---|---|---|---|---|
| 0.5 T | 0.2 m | 1.400 m⁻¹ | 16.0° | 2.59 m | 2.55 m |
| 1 T | 0.2 m | 2.799 m⁻¹ | 32.1° | 0.673 m | 0.638 m |
| 2 T | 0.1 m | 5.598 m⁻¹ | 32.1° | 0.336 m | 0.319 m |

For $B = 1$ T and $L = 0.2$ m the 4×4 matrix is

$$
M =
\\begin{pmatrix}
0.718 & 0.161 & 0.450 & 0.101\\\\
-1.260 & 0.718 & -0.789 & 0.450\\\\
-0.450 & -0.101 & 0.718 & 0.161\\\\
0.789 & -0.450 & -1.260 & 0.718
\\end{pmatrix},
\\qquad \\det M = 1 .
$$

Doubling the field and halving the length keeps the rotation and halves the focal length — the focal length is more sensitive to the field than to the length.

## Dynamics and common problems {#solenoid-dynamics}

### Coupling and rotation

The solenoid matrix has non-zero off-diagonal blocks between $x$ and $y$. For a round beam this does not matter; for a flat beam (for example after quadrupoles) a solenoid "turns" emittance from one plane into the other, and the projected emittances grow. In lattices mixing solenoids and quadrupoles, either pair the solenoids with opposite polarity to cancel the net rotation or make the beam round before the quadrupole section.

### Chromatic and spherical aberrations

The focusing strength is $\\propto 1/p^2$; the energy spread changes the focal length by $\\Delta f/f \\approx 2\\delta$. Far from the axis the higher-order terms of the field (the $r^2 B''$ term of $B_z$, the third-order radial field at the ends) cause **spherical aberration**: large-amplitude particles are over-focused and the phase space takes an "S" shape, a common source of emittance growth in LEBTs. The matrix model contains none of these effects; field-map elements are tracked by the kernel and include the non-linearities present in the map.

### Space charge and neutralisation

Space charge of low-energy, high-current beams is strong; the axially symmetric focusing of a solenoid matches the space-charge force of a round beam and makes a matched round beam easy to obtain. In a real LEBT, ionisation of the residual gas partly neutralises the space charge; AVAS's lattice has a \`spacechargecomp\` (space-charge compensation) keyword, but the manual does not describe its model, so check it against kernel results before use, and treat the degree of neutralisation as an uncertainty to be assessed separately (see [space charge](#space-charge)).

### Common problems

- **Thinking the sign of \`B\` selects focusing/defocusing**: a solenoid always focuses; changing the sign of \`B\` only reverses the rotation.
- **Too large a map \`Kb\`**: the example \`sol\` peaks at about 8.3 T × \`Kb\` (raw maximum about 10 T × \`Kb\`), so \`Kb = 1\` means 8–10 T; for 1.5 MeV protons the thin-lens estimate of the focal length is only about 3 cm, far shorter than the solenoid itself, and the beam is over-focused inside the solenoid and diverges quickly.
- **Comparing matrix and field-map results**: make $\\int B^2\\,\\dd z$ equal (same focal length), not the peak or the length; the rotation $\\int B\\,\\dd z$ then generally differs slightly.

## Hands-on {#solenoid-lab}

First the cross-section of a matrix-model solenoid: inside the hard edge the transverse field is zero and only the direction of the longitudinal field is marked (**schematic**).

\`\`\`avas-field
solenoid B=1 L=0.2 R=0.02
\`\`\`

Then slices of the example field map \`sol\` (**data**): in a longitudinal cut look at the flat top of $B_z$ and its fall-off at both ends; in a cross-section at the entrance end $(B_x, B_y)$ points to the axis, at the exit end outwards, and in the central cross-section the transverse field is close to zero. The vertical stripes along $z$ come from the noise of the map itself.

\`\`\`avas-field
map sol
\`\`\`

Finally the solenoid lab shows focal length, rotation and the 4×4 matrix, with the example beam and a drift before and after the solenoid:

1. Increase \`B\` from 0.5 T to 2 T, compare thick- and thin-lens focal lengths and check $1/f \\propto B^2$.
2. Keep $BL$ fixed (for example \`B=2 L=0.1\`): the rotation stays and the focal length halves.
3. Adjust \`d2\` to find the waist downstream of the solenoid.

\`\`\`avas-lab
solenoid L=0.2 B=1 W=1.527 mass=938.272 q=1 emit=0.1377 d1=0.2 d2=0.6
\`\`\`

## Further reading {#solenoid-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed. (chapter 3: axially symmetric magnetic fields, Busch's theorem, solenoid lenses).
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed. (chapter 7: solenoid focusing; focusing structures of low-$\\beta$ superconducting linacs).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (solenoid transfer matrix and coupling).
- Related chapters: [quadrupole](#quadrupole), [field maps](#field-maps), [superpose](#superpose), [RF cavity](#rf-cavity), [space charge](#space-charge); building a cavity–solenoid structure step by step in the [tutorial](#tutorial); the full bibliography is in [references](#references).
`,N=`# 螺线管

螺线管（solenoid）产生沿束流方向的纵向磁场 $B_z$。它在两个横向平面**同时**聚焦，对圆束的作用是轴对称的，代价是使束流绕轴旋转并耦合 $x$、$y$ 两个平面。螺线管的聚焦强度正比于 $(B/B\\rho)^2$，因此特别适合低能、低刚度的束流：低能束流传输线（LEBT）和低 $\\beta$ 超导腔段的聚焦大多由螺线管承担。AVAS 的示例项目就带有一个螺线管场图 \`sol\`。

本章用到[线性光学](#linear-optics)的传输矩阵和[四极铁](#quadrupole)一章的焦距概念。

## 作用与位置 {#solenoid-role}

- **低能束流传输与匹配**：离子源之后的 LEBT 中，1–3 个螺线管把发散的束流会聚并匹配进下游结构。
- **超导直线加速器的低能段**：在半波长（HWR）、四分之一波长（QWR）、轮辐（spoke）腔组成的低 $\\beta$ 段，常见"腔–螺线管"交替的周期。超导螺线管可以放进同一个低温恒温器，结构紧凑；它对两个平面同时聚焦，一个周期只需一个聚焦元件。
- **校正**：超导螺线管内常附带偶极校正线圈（见[校正铁](#steerer)）。

## 物理原理与场 {#solenoid-field}

### 有限长螺线管的场

在孔径内、靠近轴线处，轴对称静磁场可以由轴上场 $B(z) \\equiv B_z(0, z)$ 展开（傍轴展开，由 $\\nabla\\cdot\\boldsymbol{B}=0$ 与 $\\nabla\\times\\boldsymbol{B}=0$ 推出）：

$$
B_z(r, z) \\approx B(z) - \\frac{r^2}{4}\\,B''(z),\\qquad
B_r(r, z) \\approx -\\frac{r}{2}\\,B'(z).
$$

螺线管中部 $B(z)$ 近似为常数，$B_r \\approx 0$；在两端 $B(z)$ 下降，产生与半径成正比的**径向边缘场**。一个半径为 $a$、长度为 $\\ell$、单位长度匝数为 $n$、电流为 $I$ 的理想螺线管，轴上场为

$$
B(z) = \\frac{\\mu_0 n I}{2}\\left[\\frac{z + \\ell/2}{\\sqrt{(z+\\ell/2)^2 + a^2}} - \\frac{z - \\ell/2}{\\sqrt{(z-\\ell/2)^2 + a^2}}\\right],
$$

中心处在 $\\ell \\gg a$ 时趋于 $\\mu_0 n I$，在端口处约为中心值的一半，边缘区的宽度约为线圈半径量级。

### 硬边模型

把 $B(z)$ 换成长度 $L$、幅值 $B$ 的矩形分布（硬边），径向场就变成端口处的 $\\delta$ 函数：在入口处粒子穿过一个积分为 $\\int B_r\\,\\dd z = -\\tfrac{r}{2}B$ 的薄层，出口处符号相反。硬边模型内部**横向场处处为零**；聚焦完全来自"端口径向场 × 体内旋转速度"的组合（下节）。

> AVAS 约定：矩阵模型螺线管的横截面示意图里横向场为零，不画网格箭头——这是硬边模型本身的性质，不是缺数据。场图螺线管（如示例 \`sol\`）则有真实的边缘场：由 $B_r = -\\tfrac{r}{2}B'$，$B_z>0$ 时在场强上升的入口端 $(B_x, B_y)$ 处处指向轴心，在场强下降的出口端指向外侧，中心截面上横向场为零（已用示例场图核实：$x = +1$ cm 处 $B_x$ 在 $z = 0.12$ m 约为 $-0.55$，在 $z = 0.23$ m 约为 $+0.35$，场图单位，乘 \`Kb\` 为 T）。

### Busch 定理与正则角动量

在轴对称磁场中，正则角动量

$$
P_\\theta = \\gamma m r^2 \\dot\\theta + q\\,r A_\\theta = \\gamma m r^2\\dot\\theta + \\frac{q}{2\\pi}\\,\\Psi(r, z)
$$

守恒，其中 $\\Psi$ 是半径 $r$ 内的磁通，傍轴时 $A_\\theta = rB/2$、$\\Psi = \\pi r^2 B$。若粒子在无场区没有角动量（$P_\\theta = 0$），进入场后

$$
\\dot\\theta = -\\frac{q B}{2\\gamma m},\\qquad
\\frac{\\dd\\theta}{\\dd s} = -\\frac{q B}{2p} = -k,
$$

即粒子以**拉莫尔频率**（回旋频率的一半）绕轴旋转。这就是 Busch 定理。旋转速度与 $r$ 无关，因此整个束流像刚体一样转动；离开螺线管后旋转停止（$B=0$ 时 $\\dot\\theta=0$），只留下一个总转角。

## 运动方程与传输矩阵 {#solenoid-matrix}

### 运动方程

由 $\\boldsymbol{F} = q\\boldsymbol{v}\\times\\boldsymbol{B}$ 并保留到线性项：

$$
x'' = \\frac{q}{p}\\left(B\\,y' + \\tfrac12 B'\\,y\\right),\\qquad
y'' = -\\frac{q}{p}\\left(B\\,x' + \\tfrac12 B'\\,x\\right).
$$

定义

$$
k(s) = \\frac{q B(s)}{2p} = \\frac{B(s)}{2B\\rho},
$$

用复坐标 $u = x + \\mathrm{i}\\,y$ 写成 $u'' + 2\\mathrm{i}\\,k\\,u' + \\mathrm{i}\\,k'\\,u = 0$。代入 $u = w\\,\\ee^{-\\mathrm{i}\\theta(s)}$、$\\theta' = k$ 后，一阶导数项相消：

$$
w'' + k^2(s)\\,w = 0 .
$$

在随拉莫尔频率旋转的坐标系（**拉莫尔坐标系**）里，两个平面互不耦合，各自受到强度为 $k^2$ 的聚焦——无论 $k$ 的正负，$k^2 > 0$，所以螺线管**总是在两个平面同时聚焦**。

### 硬边螺线管的 4×4 矩阵

对长度 $L$、场强 $B$ 的硬边螺线管，令 $C = \\cos kL$，$S = \\sin kL$。实验室坐标 $(x, x', y, y')$ 下的传输矩阵为

$$
M_{\\text{sol}} =
\\begin{pmatrix}
C^2 & \\dfrac{SC}{k} & SC & \\dfrac{S^2}{k}\\\\[4pt]
-kSC & C^2 & -kS^2 & SC\\\\[4pt]
-SC & -\\dfrac{S^2}{k} & C^2 & \\dfrac{SC}{k}\\\\[4pt]
kS^2 & -SC & -kSC & C^2
\\end{pmatrix}.
$$

这与 AVAS 线性预览的 \`_solenoid4(k, L)\` 完全相同（$k = B_z/(2B\\rho)$，含入口和出口的边缘作用）。纵向块与漂移段相同，$R_{56} = L/\\gamma^2$。

它可以分解为"拉莫尔坐标系中的聚焦"与"转动"的乘积：

$$
M_{\\text{sol}} = R(kL)\\,F(kL),\\quad
F = \\begin{pmatrix} F_2 & 0 \\\\ 0 & F_2 \\end{pmatrix},\\quad
F_2 = \\begin{pmatrix} C & S/k \\\\ -kS & C \\end{pmatrix},\\quad
R(\\theta) = \\begin{pmatrix} \\cos\\theta\\, I & \\sin\\theta\\, I \\\\ -\\sin\\theta\\, I & \\cos\\theta\\, I \\end{pmatrix},
$$

$I$ 为 $2\\times 2$ 单位矩阵。$R(\\theta)$ 把点 $(x, y) = (x_0, 0)$ 变到 $(x_0\\cos\\theta, -x_0\\sin\\theta)$：对 $qB > 0$，从下游看（束流朝向观察者）束流按**顺时针**转过 $\\theta = kL$。

### 焦距与转角

- 拉莫尔转角：$\\theta_L = kL$，一般地 $\\theta_L = \\int k\\,\\dd s = \\dfrac{1}{2B\\rho}\\int B\\,\\dd s$。
- 厚透镜焦距（拉莫尔坐标系中，两个平面相同）：

$$
\\frac{1}{f} = k\\sin kL .
$$

- 薄透镜近似（$kL \\ll 1$）：

$$
\\frac{1}{f} \\approx k^2 L = \\left(\\frac{B}{2B\\rho}\\right)^2 L,\\qquad\\text{一般地}\\quad \\frac{1}{f} \\approx \\frac{1}{4(B\\rho)^2}\\int B^2\\,\\dd s .
$$

可以和四极铁比较：四极铁 $1/f = G L/B\\rho \\propto 1/B\\rho$，螺线管 $1/f \\propto 1/(B\\rho)^2$。刚度越低，螺线管越"划算"；刚度高时要得到同样的焦距，螺线管需要的场强迅速增大——这就是螺线管主要用于低能段的原因。

### 圆束与投影发射度

对没有初始正则角动量的轴对称（圆）束，转动矩阵 $R$ 不改变束流的二阶矩，螺线管出口处的投影 rms 发射度与入口相同。但在螺线管**内部**，机械角动量非零，刚过入口时 $x'$ 中多出一项 $+k\\,y$（$y'$ 中多出 $-k\\,x$），投影发射度

$$
\\varepsilon_x^2 = \\varepsilon_0^2 + k^2 \\langle x^2\\rangle\\langle y^2\\rangle
$$

（$\\varepsilon_0$ 为入口发射度，假设 $x$、$y$ 不相关）会暂时增大，出场后恢复。用场图螺线管时，DataSet.txt 里螺线管中间的 rms 发射度"鼓包"就是这个原因，不是束流品质变差。对非圆束或带初始耦合的束流，螺线管会造成真正的投影发射度增长。

## 在 AVAS 中 {#solenoid-avas}

### 矩阵模型螺线管

\`\`\`text
solenoid L R 0 B
\`\`\`

\`\`\`avas-params
solenoid
\`\`\`

- \`L\`：长度（m），硬边长度。
- \`R\`：孔径半径（m）。
- 第三个参数保留，写 0。
- \`B\`：轴向磁场（T），符号决定旋转方向，不影响聚焦（聚焦正比于 $B^2$）。

> AVAS 约定：$k = B/(2B\\rho)$，矩阵为上面的硬边形式（\`linear_optics._solenoid4\`）。\`solenoid\` 是 z-code 矩阵模型元件，同样不宜放在多粒子 lattice 的首尾。

### 场图螺线管

实际螺线管的边缘场很宽（与线圈半径相当），在紧凑的腔–螺线管周期里常与相邻元件重叠，因此通常用静磁场图描述：

\`\`\`text
field 0.35 0.02 0 3 0 0 0 Kb sol
\`\`\`

示例项目 \`examples/hwr010\` 带有螺线管场图 \`sol\`（\`sol.bsx\`、\`sol.bsy\`、\`sol.bsz\`），长 0.35 m，横向网格 $\\pm 0.02$ m。实际场 = 场图 × \`Kb\`（T）。该场图轴上 $B_z$ 的原始最大值约为 10.1（场图单位，T），9 点平滑后的峰值约为 8.3，位于 $z \\approx 0.177$ m；因此 \`Kb = 0.1\` 对应约 0.83 T 的峰值场（原始最大值约 1.0 T）。

> 注意：示例场图本身有噪声，\`sol.bsz\` 在"平顶"区逐点在 5.9–10.1 之间跳动（AGENTS.md §6），切片热图上的沿 $z$ 竖条纹是数据本身，不是读取错误。用积分量（$\\int B\\,\\dd z$、$\\int B^2\\,\\dd z$）估算转角和焦距比看峰值更可靠。

由场图积分得到 $\\int B\\,\\dd z \\approx 0.729\\,\\mathrm{Kb}$ T·m，$\\int B^2\\,\\dd z \\approx 4.04\\,\\mathrm{Kb}^2$ T²·m（噪声会使后者略偏大）。对示例束流（$B\\rho = 0.17863$ T·m）：

| \`Kb\` | 峰值场 | 拉莫尔转角 | 薄透镜焦距 |
|---|---|---|---|
| 0.1 | ≈ 0.83 T | 11.7° | 3.16 m |
| 0.2 | ≈ 1.7 T | 23.4° | 0.79 m |

场图元件由 t-code 跟踪，包含边缘场；线性预览用场图的近轴梯度（含径向分量）计算边缘作用（见 \`linear_optics.py\` 说明）。场图的格式、存储顺序和归一化见[场图](#field-maps)，与射频腔叠加放置的方法见[叠加场](#superpose)。

## 可调参数与典型取值 {#solenoid-params}

| 量 | 典型范围 | 说明 |
|---|---|---|
| 场强 $B$ | 常温 0.1–0.6 T；超导可达数 T | 低能段为得到短焦距需要较强的场 |
| 长度 $L$ | 0.1–0.4 m | 超导低温恒温器内的螺线管通常较短 |
| 孔径半径 $R$ | 0.01–0.05 m | LEBT 中孔径较大 |
| 转角 $\\theta_L$ | 数度至数十度 | 只影响束流取向，不影响圆束的 rms 尺寸 |

### 数值例子

示例束流（质子 1.527 MeV，$B\\rho = 0.17863$ T·m）通过硬边螺线管：

| $B$ | $L$ | $k$ | $kL$ | 厚透镜 $f$ | 薄透镜 $f$ |
|---|---|---|---|---|---|
| 0.5 T | 0.2 m | 1.400 m⁻¹ | 16.0° | 2.59 m | 2.55 m |
| 1 T | 0.2 m | 2.799 m⁻¹ | 32.1° | 0.673 m | 0.638 m |
| 2 T | 0.1 m | 5.598 m⁻¹ | 32.1° | 0.336 m | 0.319 m |

$B = 1$ T、$L = 0.2$ m 时的 4×4 矩阵为

$$
M =
\\begin{pmatrix}
0.718 & 0.161 & 0.450 & 0.101\\\\
-1.260 & 0.718 & -0.789 & 0.450\\\\
-0.450 & -0.101 & 0.718 & 0.161\\\\
0.789 & -0.450 & -1.260 & 0.718
\\end{pmatrix},
\\qquad \\det M = 1 .
$$

把场强加倍、长度减半，转角不变，焦距减半——焦距对场强比对长度敏感。

## 动力学效应与常见问题 {#solenoid-dynamics}

### 耦合与旋转

螺线管的矩阵在 $x$–$y$ 之间有非零的非对角块。对圆束这无关紧要；对扁束（例如经过四极铁之后的束流），螺线管会把一个平面的发射度"转"到另一个平面，造成投影发射度增长。在螺线管和四极铁混合的 lattice 中，要么让螺线管成对、反向励磁以抵消总转角，要么在四极铁段之前把束流调圆。

### 色差与球差

聚焦强度 $\\propto 1/p^2$，能散带来的焦距变化 $\\Delta f/f \\approx 2\\delta$。离轴较远处，场的高阶项（$B_z$ 的 $r^2 B''$ 项、端部的三阶径向场）造成**球差**，使大振幅粒子聚焦过强，在相空间中形成"S"形畸变，是 LEBT 中发射度增长的常见原因。矩阵模型不包含这些效应；场图元件由内核跟踪，包含场图里的非线性。

### 空间电荷与中和

低能强流束流的空间电荷很强，螺线管的轴对称聚焦与圆束的空间电荷力相匹配，便于形成匹配的圆束。在真实的 LEBT 中残余气体电离会部分中和空间电荷；AVAS 的 lattice 中有 \`spacechargecomp\`（空间电荷补偿）关键字，但手册没有说明其模型，使用前应以内核结果核对，中和度本身也应作为单独评估的不确定因素（见[空间电荷](#space-charge)）。

### 常见问题

- **误以为 \`B\` 的符号决定聚焦/散焦**：螺线管总是聚焦，\`B\` 变号只改变旋转方向。
- **场图 \`Kb\` 过大**：示例 \`sol\` 的峰值约为 8.3 T × \`Kb\`（原始最大值约 10 T × \`Kb\`），\`Kb = 1\` 意味着 8–10 T；对 1.5 MeV 质子，薄透镜估计的焦距只有约 3 cm，远小于螺线管本身的长度，束流会在螺线管中过聚焦并迅速发散。
- **比较矩阵与场图结果**：应令两者的 $\\int B^2\\,\\dd z$ 相同（焦距相同），而不是峰值或长度相同；此时转角 $\\int B\\,\\dd z$ 一般略有不同。

## 动手实验 {#solenoid-lab}

先看矩阵模型螺线管的横截面：硬边内部横向场为零，图上只标注纵向场方向（**示意**）。

\`\`\`avas-field
solenoid B=1 L=0.2 R=0.02
\`\`\`

再看示例场图 \`sol\` 的切片（**数据**）：在纵切面上观察 $B_z$ 的平顶与两端的衰减；在入口端横截面上 $(B_x, B_y)$ 指向轴心，出口端指向外侧，中心横截面上横向场接近零。沿 $z$ 的竖条纹来自场图本身的噪声。

\`\`\`avas-field
map sol
\`\`\`

最后用螺线管实验观察焦距、转角和 4×4 矩阵。束流参数取示例束流，螺线管前后各有一段漂移：

1. 把 \`B\` 从 0.5 T 逐步增加到 2 T，对比厚透镜和薄透镜焦距，验证 $1/f \\propto B^2$。
2. 保持 $BL$ 不变（例如 \`B=2 L=0.1\`），观察转角不变而焦距减半。
3. 调整 \`d2\`，找到螺线管下游的束腰位置。

\`\`\`avas-lab
solenoid L=0.2 B=1 W=1.527 mass=938.272 q=1 emit=0.1377 d1=0.2 d2=0.6
\`\`\`

## 延伸阅读 {#solenoid-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.（第 3 章：轴对称磁场、Busch 定理、螺线管透镜）。
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.（第 7 章：螺线管聚焦；低 $\\beta$ 超导直线加速器的聚焦结构）。
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed.（螺线管传输矩阵与耦合）。
- 相关章节：[四极铁](#quadrupole)、[场图](#field-maps)、[叠加场](#superpose)、[射频腔](#rf-cavity)、[空间电荷](#space-charge)；动手搭建腔–螺线管结构见[教程](#tutorial)；完整书目见[参考文献](#references)。
`,P=`# Space charge

The charged particles of a beam repel each other. In low-energy, high-intensity beams this **space-charge force** cancels part of the external focusing, enlarges the beam, depresses the phase advance and, through its non-linear part, causes emittance growth and halo. This chapter derives the self fields of uniform beams, introduces the generalised perveance $K$, derives the KV envelope equations and Sacherer's rms envelope equations, gives analytic expressions for the matched size and the tune depression in the smooth approximation, discusses emittance growth and halo qualitatively, and explains how the AVAS kernel and the linear preview handle space charge. Transverse linear optics and Twiss parameters are in [Linear optics](#linear-optics) and [Phase space and emittance](#phase-space), longitudinal motion in [Longitudinal dynamics](#longitudinal).

The numerical examples use the beam of the example project: protons, $W = 1.527\\ \\mathrm{MeV}$ ($\\beta = 0.05698$, $\\gamma = 1.00163$, $\\beta\\gamma = 0.05708$, $B\\rho = 0.1786\\ \\mathrm{T\\,m}$), average current $I = 0.184\\ \\mathrm{mA}$, beam frequency $f = 162.5\\ \\mathrm{MHz}$, transverse normalised rms emittance $0.138\\ \\pi\\,\\mathrm{mm\\,mrad}$ (geometric rms emittance $\\varepsilon = 2.41\\ \\mathrm{mm\\,mrad}$).

## Self fields of a uniform beam {#space-charge-self-fields}

### Round DC beam

Take charge distributed uniformly in a cylinder of radius $a$ with line density $\\lambda_q = I/(\\beta c)$. Gauss's and Ampère's laws give inside the beam ($r \\le a$)

$$
E_r = \\frac{\\lambda_q}{2\\pi\\varepsilon_0}\\frac{r}{a^2},\\qquad B_\\theta = \\frac{\\beta}{c}E_r .
$$

The magnetic force of the moving charges opposes the electric force:

$$
F_r = q\\left(E_r - \\beta c B_\\theta\\right) = q E_r\\left(1 - \\beta^2\\right) = \\frac{q E_r}{\\gamma^2} .
$$

This $1/\\gamma^2$ cancellation makes space charge fade quickly at relativistic energies. With $x'' = F_x/(\\gamma m\\beta^2c^2)$ the equation of motion of a particle inside the beam is

$$
x'' = \\frac{K}{a^2}\\,x ,
$$

with $K$ the generalised perveance defined in the next section. The force is proportional to $x$: **the self field of a uniform distribution is linear**, which is the basis of the KV theory.

### Elliptical cross-section

Inside a uniform beam of elliptical cross-section with semi-axes $a$, $b$

$$
E_x = \\frac{\\lambda_q}{\\pi\\varepsilon_0}\\frac{x}{a(a + b)},\\qquad
E_y = \\frac{\\lambda_q}{\\pi\\varepsilon_0}\\frac{y}{b(a + b)},
\\qquad
x'' = \\frac{2K}{a(a + b)}\\,x,\\quad y'' = \\frac{2K}{b(a + b)}\\,y .
$$

For $a = b$ this reduces to the round beam.

### Bunches: the uniform ellipsoid

An RF-accelerated beam is a train of bunches, each with charge $Q = I/f$ ($I$ the average current). Model a bunch as a uniform ellipsoid with lab-frame semi-axes $a$, $b$, $c_z$; in the beam rest frame the longitudinal semi-axis is $\\gamma c_z$ and the charge density $\\rho' = 3Q/(4\\pi a b\\,\\gamma c_z)$. The field inside a uniform ellipsoid is linear:

$$
E'_x = \\frac{\\rho'}{\\varepsilon_0}M_x\\,x,\\quad E'_y = \\frac{\\rho'}{\\varepsilon_0}M_y\\,y,\\quad E'_z = \\frac{\\rho'}{\\varepsilon_0}M_z\\,\\gamma z,\\qquad M_x + M_y + M_z = 1,
$$

with $\\gamma z$ the rest-frame longitudinal coordinate and $M_i$ the depolarisation factors (all equal to $1/3$ for a sphere). For a spheroid with rest-frame aspect ratio $p = \\gamma c_z/\\sqrt{ab} > 1$,

$$
M_z = \\frac{1 - e^2}{e^3}\\left(\\operatorname{artanh} e - e\\right),\\qquad e = \\sqrt{1 - 1/p^2},
$$

with the common approximation $M_z \\approx 1/(3p)$ (for $p$ from about 1 to a few) and transversely $M_x \\approx (1 - M_z)\\,b/(a + b)$. Transforming back to the lab (the transverse force gets another $1/\\gamma$) gives

$$
x'' = \\frac{K_3 M_x}{a b c_z}\\,x,\\qquad
z'' = \\frac{K_3 M_z}{a b c_z}\\,z,\\qquad
K_3 = \\frac{3 q Q}{4\\pi\\varepsilon_0\\, mc^2\\,\\beta^2\\gamma^3},
$$

where $z' = \\dd z/\\dd s = \\delta/\\gamma^2$.

## Generalised perveance {#space-charge-perveance}

### DC beams

$$
K = \\frac{q I}{2\\pi\\varepsilon_0\\, m c^3\\beta^3\\gamma^3} = \\frac{2I}{I_0\\,\\beta^3\\gamma^3},\\qquad
I_0 = \\frac{4\\pi\\varepsilon_0\\, m c^3}{q} .
$$

$I_0$ is the characteristic current: $I_0 = 3.13\\times 10^7\\ \\mathrm{A}$ for protons and $3.13\\times 10^7\\,A/Z\\ \\mathrm{A}$ for ions of mass number $A$ and charge state $Z$. $K$ is dimensionless and measures space charge against the beam "stiffness"; the $\\beta^3\\gamma^3$ makes it large at low energy. At the example energy:

| DC current | $K$ |
|---|---|
| 0.184 mA | $6.32\\times 10^{-8}$ |
| 1 mA | $3.44\\times 10^{-7}$ |
| 10 mA | $3.44\\times 10^{-6}$ |

### Equivalent perveance of a bunch

Comparing the transverse equation of the uniform ellipsoid, $x'' = K_3 M_x x/(a b c_z)$, with $x'' = K x/a^2$ of a round DC beam (taking $a = b$, $M_x = (1 - M_z)/2$), the centre of a bunch acts like a DC beam with the **equivalent current** and perveance

$$
I_{\\mathrm{eq}} = \\frac{3}{4}\\,\\frac{Q\\,\\beta c}{c_z}\\,(1 - M_z),\\qquad
K_{\\mathrm{eq}} = \\frac{2 I_{\\mathrm{eq}}}{I_0\\beta^3\\gamma^3} .
$$

Example bunch: $Q = I/f = 1.133\\ \\mathrm{pC}$ ($7.07\\times 10^6$ protons), rms sizes $\\sigma_x = 0.954$, $\\sigma_y = 0.958$, $\\sigma_z = 1.316\\ \\mathrm{mm}$. The semi-axes of the rms-equivalent uniform ellipsoid are $\\sqrt5\\,\\sigma$ (see [next section](#space-charge-envelope)), the rest-frame aspect ratio is $p = 1.379$ and $M_z = 0.252$, so $I_{\\mathrm{eq}} = 3.69\\ \\mathrm{mA}$ and $K_{\\mathrm{eq}} = 1.27\\times 10^{-6}$.

> Note: the space-charge strength of a bunch is set by the **peak line density inside the bunch**, not by the average current. In the example the equivalent current is 20 times the average current, and $I_{\\mathrm{eq}}$ grows further when the bunch is compressed longitudinally. Estimating $K$ from the average current badly underestimates space charge.

## KV and rms envelope equations {#space-charge-envelope}

### KV envelope equations

The KV (Kapchinsky–Vladimirsky) distribution is uniform on a hyper-ellipsoidal shell in four-dimensional phase space; its projection on the $x$–$y$ plane is a uniform ellipse, its self field is exactly linear, and it stays KV in linear external fields. The beam-edge semi-axes $a(s)$, $b(s)$ obey

$$
a'' + k_x(s)\\,a - \\frac{2K}{a + b} - \\frac{\\varepsilon_x^2}{a^3} = 0,\\qquad
b'' + k_y(s)\\,b - \\frac{2K}{a + b} - \\frac{\\varepsilon_y^2}{b^3} = 0 ,
$$

with $k_{x,y}(s)$ the external focusing (quadrupoles $k = \\pm G/B\\rho$, see [Quadrupoles](#quadrupole)) and $\\varepsilon$ the **edge** (total) geometric emittance. The three terms are external focusing, space-charge defocusing and the emittance ("thermal pressure") term.

### Sacherer's rms envelope equations

Sacherer (1971) showed that for any distribution with elliptical symmetry the rms size $\\sigma = \\sqrt{\\langle x^2\\rangle}$ obeys an equation of the same form, for continuous (2D) beams the space-charge term does not depend on the profile at all, and for 3D ellipsoidally symmetric bunches it depends only weakly on it:

$$
\\sigma_x'' + k_x\\,\\sigma_x - \\frac{K}{2(\\sigma_x + \\sigma_y)} - \\frac{\\varepsilon_{x}^2}{\\sigma_x^3} = 0 ,
$$

with $\\varepsilon_x$ the rms geometric emittance. Substituting $a = 2\\sigma_x$, $\\varepsilon_{\\mathrm{edge}} = 4\\varepsilon_{\\mathrm{rms}}$ in the KV equation gives exactly this. Any distribution can therefore be described by its "rms-equivalent KV beam": **edge size = 2 × rms size, edge emittance = 4 × rms emittance**.

### Three-dimensional rms envelope equations of a bunch

For a uniform ellipsoid $\\langle x^2\\rangle = a^2/5$, i.e. the rms-equivalent semi-axis is $\\sqrt5\\,\\sigma$. With the linear forces of the previous section:

$$
\\sigma_x'' + k_x\\sigma_x - \\frac{\\varepsilon_x^2}{\\sigma_x^3} - \\frac{K_3 M_x}{5\\sqrt5\\,\\sigma_y\\sigma_z} = 0,\\qquad
\\sigma_z'' + k_z\\sigma_z - \\frac{\\varepsilon_z^2}{\\sigma_z^3} - \\frac{K_3 M_z}{5\\sqrt5\\,\\sigma_x\\sigma_y} = 0 ,
$$

$y$ being analogous to $x$, $k_z$ the longitudinal RF focusing ($k_{l0}^2$ of [Longitudinal dynamics](#longitudinal)) and $\\varepsilon_z$ the geometric rms emittance in the $(z, \\dd z/\\dd s)$ plane. The three planes are coupled through the $M_i$ and the sizes.

> AVAS convention: the longitudinal angle of \`twissz\` is $\\delta = \\Delta p/p$, while the equation above uses $\\dd z/\\dd s = \\delta/\\gamma^2$; they differ by $\\gamma^2$. At the example energy $\\gamma^2 = 1.003$ and the difference is negligible, but convert carefully at higher energy.

### Emittance-dominated or space-charge-dominated

Compare the two defocusing terms $K/(4\\sigma)$ (round beam) and $\\varepsilon^2/\\sigma^3$: for $K\\sigma^2/(4\\varepsilon^2) \\ll 1$ the beam is **emittance dominated**, for $\\gg 1$ **space-charge dominated**. With the example emittance, a 10 mA DC beam at $\\sigma = 1.33\\ \\mathrm{mm}$ has a ratio of about 0.26: still emittance dominated, but space charge already changes the matched size noticeably (next section).

## Tune depression and the smooth approximation {#space-charge-tune-depression}

### Smooth approximation

If the zero-current phase advance per focusing period $L_p$, $\\sigma_0$, is not too large (roughly $\\sigma_0 \\lesssim 60°$–$80°$), the periodic focusing can be averaged into continuous focusing $k_0 = \\sigma_0/L_p$. The rms envelope equation of a round DC beam becomes

$$
\\sigma'' + k_0^2\\,\\sigma - \\frac{K}{4\\sigma} - \\frac{\\varepsilon^2}{\\sigma^3} = 0 .
$$

### Matched size

For a matched beam $\\sigma'' = 0$:

$$
k_0^2\\sigma^4 - \\frac{K}{4}\\sigma^2 - \\varepsilon^2 = 0
\\quad\\Longrightarrow\\quad
\\sigma^2 = \\frac{K/4 + \\sqrt{K^2/16 + 4k_0^2\\varepsilon^2}}{2k_0^2} .
$$

At zero current $\\sigma_{0,\\mathrm{rms}} = \\sqrt{\\varepsilon/k_0}$.

### Depressed phase advance

A particle inside the beam feels the external focusing and the linear space-charge defocusing, so its wave number is $k^2 = k_0^2 - K/(4\\sigma^2)$. The matching condition gives $k = \\varepsilon/\\sigma^2$; inserting and rearranging:

$$
\\frac{\\sigma}{\\sigma_0} = \\frac{k}{k_0} = \\sqrt{1 + u^2} - u,\\qquad
u = \\frac{K}{8 k_0\\,\\varepsilon_{\\mathrm{rms}}} = \\frac{K}{2 k_0\\,\\varepsilon_{\\mathrm{edge}}} ,
$$

where all emittances are **geometric** (unnormalised) and $\\varepsilon_{\\mathrm{edge}} = 4\\varepsilon_{\\mathrm{rms}}$. Here $\\sigma$, $\\sigma_0$ denote phase advances per period (not rms sizes); the ratio $\\sigma/\\sigma_0$ is the **tune depression**: 1 means no space charge, values approaching 0 mean extreme space-charge domination. Designs commonly keep $\\sigma/\\sigma_0 \\gtrsim 0.4$–0.5.

### Numerical example

FODO period: quadrupoles $L_q = 0.1\\ \\mathrm{m}$, $G = 6\\ \\mathrm{T/m}$, drifts $L_d = 0.2\\ \\mathrm{m}$, period $L_p = 0.6\\ \\mathrm{m}$. AVAS's thick-lens matrices give $\\sigma_0 = 52.7°$, $k_0 = 1.534\\ \\mathrm{m}^{-1}$ and a zero-current matched rms size of 1.254 mm.

| Beam | $K$ | $u$ | $\\sigma/\\sigma_0$ | $\\sigma$ | Matched rms size |
|---|---|---|---|---|---|
| Example bunch ($I_{\\mathrm{eq}} = 3.69\\ \\mathrm{mA}$) | $1.27\\times 10^{-6}$ | 0.043 | 0.958 | 50.5° | 1.281 mm |
| 10 mA DC | $3.44\\times 10^{-6}$ | 0.116 | 0.891 | 47.0° | 1.329 mm |
| 30 mA DC | $1.03\\times 10^{-5}$ | 0.348 | 0.711 | 37.5° | 1.488 mm |

For the same period at $G = 4\\ \\mathrm{T/m}$ ($\\sigma_0 = 34.5°$) the 30 mA DC beam is depressed to 0.60: the weaker the focusing, the larger the relative effect of space charge.

\`\`\`avas-lab
space-charge I=10 W=1.527 mass=938.272 q=1 emit=0.13768 G=6 Lq=0.1 Ld=0.2 cells=10
\`\`\`

> Note: the smooth approximation ignores the envelope flutter within a period, and its error grows with $\\sigma_0$; exact matching requires solving the periodic envelope equations (see [Matching](#matching)). The equivalent $K$ of a bunch also changes with the bunch length; the first row of the table is only an estimate at the entry.

## Emittance growth and halo {#space-charge-emittance-growth}

In linear theory the rms emittance is conserved. Real beams are not KV, their self fields have non-linear parts, and the emittance grows. The main mechanisms (qualitatively):

- **Charge redistribution**: an rms-matched but non-uniform beam (Gaussian, or with a peaked core) relaxes towards a more uniform distribution within about a quarter of a plasma period, and the released "free energy" becomes emittance growth. For a round uniform beam the plasma wave number is $k_p^2 = 2K/a^2 = K/(2\\sigma^2)$; in the 10 mA example $k_p = 0.99\\ \\mathrm{m}^{-1}$, a plasma wavelength of about 6.4 m. The stronger the space charge, the faster this happens and the larger the growth.
- **Mismatch and halo**: the envelope oscillations of a mismatched beam (for a round beam in the smooth approximation, breathing mode $k_{\\mathrm{b}} = \\sqrt{2k_0^2 + 2k^2}$, quadrupole mode $k_{\\mathrm{q}} = \\sqrt{k_0^2 + 3k^2}$) drive single particles through a 2:1 parametric resonance onto orbits far from the core, forming a **halo**. Halo particles are few and barely change the rms size, but they decide the losses.
- **Envelope instabilities and resonances**: with a zero-current phase advance $\\sigma_0 > 90°$, space charge can drive envelope instabilities (hence the common design rule $\\sigma_0 < 90°$); other parametric resonances of the periodic structure also cause growth.
- **Transverse–longitudinal coupling**: when the transverse and longitudinal "temperatures" (emittance over focusing) differ, space charge exchanges energy between the planes (the equipartition problem); Hofmann's stability charts show the dangerous parameter regions.

In AVAS output, compare the rms sizes in DataSet.txt (columns 16/18/20) with the maximum sizes (columns 22/24/26): a steadily growing ratio of maximum to rms usually signals developing halo. Emittance growth and halo require the multi-particle kernel; the linear preview contains neither.

## In AVAS {#space-charge-avas}

### Keywords

Space charge is controlled in \`input.txt\`, the beam charge comes from \`beam.txt\`:

\`\`\`avas-params
input:spacecharge input:scmethod input:numofgrid input:meshrms input:steppercycle beam:current beam:frequency beam:beamtype beam:particlenumber spacechargecomp
\`\`\`

- \`spacecharge 1\` turns space charge on, \`0\` off.
- \`current\` (mA, average current) and \`frequency\` (Hz, beam frequency) give the bunch charge $Q = I/f$; each macro-particle carries $Q$ divided by the number of macro-particles \`particlenumber\`. \`beamtype dc\` generates a DC beam.
- \`scmethod\` selects the solver: \`FFT\`, \`PICNIC\` or \`SPICNIC\`. The manual only lists the names and does not describe the algorithms; the example project uses \`FFT\`. When switching to another solver, compare with \`FFT\` on the same problem.
- \`numofgrid Nx Ny Nz\` sets the number of mesh points per direction; \`meshrms V1 V2 V3\` sets the mesh length $L_x = 2 V_1\\sigma_x$ (likewise $y$, $z$), i.e. the mesh covers $\\pm V_1$ rms sizes. The mesh must cover the whole bunch (tails included) while each cell still holds enough macro-particles; see [Multi-particle simulation in practice](#simulation).
- Space charge is computed every time step; the step is set by \`steppercycle\` (steps per RF period).
- The manual also states that with a longitudinal periodic boundary the space-charge solver switches to FFT automatically and the longitudinal mesh length is set to the bunch period, so more longitudinal mesh points are needed.
- The lattice keyword \`spacechargecomp\` (space-charge compensation) is not described in the manual; neither its parameters nor its effect are documented, so do not rely on it.
- \`spacechargelong\` and \`spacechargetype\` act only in the envelope model (\`sim_type env\`).

> Note: \`boundary\` in \`input.txt\` refers to the beam **loss** boundary (\`boundary.txt\`), not to a boundary condition of the space-charge solver. The boundary conditions, coordinate frame and interpolation of the kernel's space-charge solvers are not documented in the manual; the kernel is the reference.

### Space-charge model of the linear preview

The linear preview of the visual editor adds linear space charge when the beam current is positive:

- Bunched beams: the rms-equivalent uniform ellipsoid (semi-axes $\\sqrt5\\,\\sigma_{x,y,z}$), $M_z$ from the spheroid formula with the rest-frame aspect ratio $p = \\gamma c_z/\\sqrt{ab}$, transversely $M_x = (1 - M_z)\\,b/(a + b)$, i.e. the linear $K_3$ force above. It needs \`frequency\` and \`twissz\`; without them space charge is left out with a warning.
- DC beams (\`beamtype dc\`): the rms-equivalent uniform elliptical cross-section (semi-axes $2\\sigma$), i.e. the KV force $2K/(a(a+b))$.
- A thin kick with the strength integrated over 4 steps is applied in the middle of each group of steps (second-order splitting), with the force updated from the current rms sizes.

It describes only the linear part of the rms envelope, without emittance growth, halo or losses. With strong space charge its rms sizes can differ from the kernel's by up to 13 %.

## Further reading {#space-charge-further}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: self fields, KV distribution, envelope equations, emittance growth.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: multiparticle dynamics with space charge and halo.
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18, 1105 (1971).
- I. M. Kapchinsky, V. V. Vladimirsky, Proc. Int. Conf. on High Energy Accelerators, CERN, 1959 (KV distribution).
- I. Hofmann, "Stability of anisotropic beams with space charge", *Phys. Rev. E* 57, 4713 (1998).
- In this primer: [Linear optics](#linear-optics), [Matching](#matching), [Multi-particle simulation in practice](#simulation).
`,ne=`# 空间电荷

束流中的带电粒子相互排斥。对低能强流束，这种**空间电荷力**会抵消一部分外部聚焦，使束流尺寸变大、相移压低，并通过非线性力引起发射度增长和束晕。本章先求均匀束的自场，引入广义导流系数 $K$，再推导 KV 包络方程和 Sacherer 的 rms 包络方程，用平滑近似给出匹配尺寸和相移压低的解析式，最后定性讨论发射度增长与束晕，并说明 AVAS 内核和线性预览各自如何处理空间电荷。横向线性光学和 Twiss 参数见 [线性光学](#linear-optics) 与 [相空间与发射度](#phase-space)，纵向运动见 [纵向动力学](#longitudinal)。

本章的数值例子用示例项目的束流：质子，$W = 1.527\\ \\mathrm{MeV}$（$\\beta = 0.05698$，$\\gamma = 1.00163$，$\\beta\\gamma = 0.05708$，$B\\rho = 0.1786\\ \\mathrm{T\\,m}$），平均流强 $I = 0.184\\ \\mathrm{mA}$，束流频率 $f = 162.5\\ \\mathrm{MHz}$，横向归一化 rms 发射度 $0.138\\ \\pi\\,\\mathrm{mm\\,mrad}$（几何 rms 发射度 $\\varepsilon = 2.41\\ \\mathrm{mm\\,mrad}$）。

## 均匀束的自场 {#space-charge-self-fields}

### 圆截面直流束

设半径为 $a$ 的圆柱内电荷均匀分布，线电荷密度 $\\lambda_q = I/(\\beta c)$。由高斯定律和安培定律，束内（$r \\le a$）

$$
E_r = \\frac{\\lambda_q}{2\\pi\\varepsilon_0}\\frac{r}{a^2},\\qquad B_\\theta = \\frac{\\beta}{c}E_r .
$$

运动电荷的磁场力与电场力方向相反：

$$
F_r = q\\left(E_r - \\beta c B_\\theta\\right) = q E_r\\left(1 - \\beta^2\\right) = \\frac{q E_r}{\\gamma^2} .
$$

这个 $1/\\gamma^2$ 抵消使空间电荷在相对论能区迅速变弱。由 $x'' = F_x/(\\gamma m\\beta^2c^2)$，束内粒子的运动方程是

$$
x'' = \\frac{K}{a^2}\\,x ,
$$

$K$ 是下一节定义的广义导流系数。力与 $x$ 成正比——**均匀分布的自场是线性的**，这是 KV 理论成立的基础。

### 椭圆截面

半轴为 $a$、$b$ 的均匀椭圆截面束内

$$
E_x = \\frac{\\lambda_q}{\\pi\\varepsilon_0}\\frac{x}{a(a + b)},\\qquad
E_y = \\frac{\\lambda_q}{\\pi\\varepsilon_0}\\frac{y}{b(a + b)},
\\qquad
x'' = \\frac{2K}{a(a + b)}\\,x,\\quad y'' = \\frac{2K}{b(a + b)}\\,y .
$$

$a = b$ 时回到圆截面的结果。

### 束团：均匀椭球

射频加速的束是一串束团，每个束团电荷 $Q = I/f$（$I$ 为平均流强）。把束团看成实验室系半轴 $a$、$b$、$c_z$ 的均匀椭球；在束流静止系中纵向半轴为 $\\gamma c_z$，电荷密度 $\\rho' = 3Q/(4\\pi a b\\,\\gamma c_z)$。均匀椭球内的场是线性的：

$$
E'_x = \\frac{\\rho'}{\\varepsilon_0}M_x\\,x,\\quad E'_y = \\frac{\\rho'}{\\varepsilon_0}M_y\\,y,\\quad E'_z = \\frac{\\rho'}{\\varepsilon_0}M_z\\,\\gamma z,\\qquad M_x + M_y + M_z = 1,
$$

$\\gamma z$ 是静止系中的纵向坐标，$M_i$ 是退极化因子（球形时都等于 $1/3$）。对旋转椭球，记静止系长径比 $p = \\gamma c_z/\\sqrt{ab}$，$p > 1$ 时

$$
M_z = \\frac{1 - e^2}{e^3}\\left(\\operatorname{artanh} e - e\\right),\\qquad e = \\sqrt{1 - 1/p^2},
$$

常用近似 $M_z \\approx 1/(3p)$（$p$ 在 1 附近到几倍之间），横向 $M_x \\approx (1 - M_z)\\,b/(a + b)$。变换回实验室系（横向力再乘 $1/\\gamma$），得到

$$
x'' = \\frac{K_3 M_x}{a b c_z}\\,x,\\qquad
z'' = \\frac{K_3 M_z}{a b c_z}\\,z,\\qquad
K_3 = \\frac{3 q Q}{4\\pi\\varepsilon_0\\, mc^2\\,\\beta^2\\gamma^3},
$$

这里 $z' = \\dd z/\\dd s = \\delta/\\gamma^2$。

## 广义导流系数 {#space-charge-perveance}

### 直流束

$$
K = \\frac{q I}{2\\pi\\varepsilon_0\\, m c^3\\beta^3\\gamma^3} = \\frac{2I}{I_0\\,\\beta^3\\gamma^3},\\qquad
I_0 = \\frac{4\\pi\\varepsilon_0\\, m c^3}{q} .
$$

$I_0$ 是特征电流：质子 $I_0 = 3.13\\times 10^7\\ \\mathrm{A}$，质量数 $A$、电荷态 $Z$ 的离子为 $3.13\\times 10^7\\,A/Z\\ \\mathrm{A}$。$K$ 无量纲，衡量空间电荷相对于束流"刚度"的强弱；$\\beta^3\\gamma^3$ 使它在低能时很大。示例能量下

| 直流流强 | $K$ |
|---|---|
| 0.184 mA | $6.32\\times 10^{-8}$ |
| 1 mA | $3.44\\times 10^{-7}$ |
| 10 mA | $3.44\\times 10^{-6}$ |

### 束团的等效导流系数

把均匀椭球的横向方程 $x'' = K_3 M_x x/(a b c_z)$ 与直流圆束的 $x'' = K x/a^2$ 比较（取 $a = b$，$M_x = (1 - M_z)/2$），束团中心截面相当于一个直流束，其**等效流强**和导流系数为

$$
I_{\\mathrm{eq}} = \\frac{3}{4}\\,\\frac{Q\\,\\beta c}{c_z}\\,(1 - M_z),\\qquad
K_{\\mathrm{eq}} = \\frac{2 I_{\\mathrm{eq}}}{I_0\\beta^3\\gamma^3} .
$$

示例束团：$Q = I/f = 1.133\\ \\mathrm{pC}$（$7.07\\times 10^6$ 个质子），rms 尺寸 $\\sigma_x = 0.954$、$\\sigma_y = 0.958$、$\\sigma_z = 1.316\\ \\mathrm{mm}$。rms 等效均匀椭球的半轴是 $\\sqrt5\\,\\sigma$（见 [下一节](#space-charge-envelope)），静止系长径比 $p = 1.379$，$M_z = 0.252$，于是 $I_{\\mathrm{eq}} = 3.69\\ \\mathrm{mA}$，$K_{\\mathrm{eq}} = 1.27\\times 10^{-6}$。

> 注意：束团的空间电荷强度由**束团内的峰值线密度**决定，而不是平均流强。示例中等效流强是平均流强的 20 倍；束团被纵向压缩时 $I_{\\mathrm{eq}}$ 还会增大。直接用平均流强估计 $K$ 会严重低估空间电荷。

## KV 与 rms 包络方程 {#space-charge-envelope}

### KV 包络方程

KV（Kapchinsky–Vladimirsky）分布是四维相空间中一个超椭球面上的均匀分布，它在 $x$–$y$ 平面的投影是均匀椭圆，自场严格线性，并且在线性外场中保持 KV 分布。束流边界半轴 $a(s)$、$b(s)$ 满足

$$
a'' + k_x(s)\\,a - \\frac{2K}{a + b} - \\frac{\\varepsilon_x^2}{a^3} = 0,\\qquad
b'' + k_y(s)\\,b - \\frac{2K}{a + b} - \\frac{\\varepsilon_y^2}{b^3} = 0 ,
$$

$k_{x,y}(s)$ 是外部聚焦强度（四极铁 $k = \\pm G/B\\rho$，见 [四极铁](#quadrupole)），$\\varepsilon$ 是**边界**（总）几何发射度。三项分别代表外部聚焦、空间电荷散焦和发射度（"热压力"）项。

### Sacherer 的 rms 包络方程

Sacherer（1971）证明，对任何具有椭圆对称性的分布，rms 尺寸 $\\sigma = \\sqrt{\\langle x^2\\rangle}$ 满足形式相同的方程；对二维连续束，空间电荷项与分布形状无关，对三维椭球对称的束团只弱依赖于分布形状：

$$
\\sigma_x'' + k_x\\,\\sigma_x - \\frac{K}{2(\\sigma_x + \\sigma_y)} - \\frac{\\varepsilon_{x}^2}{\\sigma_x^3} = 0 ,
$$

$\\varepsilon_x$ 是 rms 几何发射度。把 $a = 2\\sigma_x$、$\\varepsilon_{\\mathrm{edge}} = 4\\varepsilon_{\\mathrm{rms}}$ 代入 KV 方程即得此式。于是任意分布都可以用"rms 等效 KV 束"描述：**边界量 = 2 × rms 尺寸，边界发射度 = 4 × rms 发射度**。

### 束团的三维 rms 包络方程

对均匀椭球，$\\langle x^2\\rangle = a^2/5$，即 rms 等效半轴是 $\\sqrt5\\,\\sigma$。代入上节的线性力：

$$
\\sigma_x'' + k_x\\sigma_x - \\frac{\\varepsilon_x^2}{\\sigma_x^3} - \\frac{K_3 M_x}{5\\sqrt5\\,\\sigma_y\\sigma_z} = 0,\\qquad
\\sigma_z'' + k_z\\sigma_z - \\frac{\\varepsilon_z^2}{\\sigma_z^3} - \\frac{K_3 M_z}{5\\sqrt5\\,\\sigma_x\\sigma_y} = 0 ,
$$

$y$ 方向与 $x$ 类似，$k_z$ 是纵向射频聚焦（[纵向动力学](#longitudinal) 中的 $k_{l0}^2$），$\\varepsilon_z$ 是 $(z, \\dd z/\\dd s)$ 平面的几何 rms 发射度。三个方向通过 $M_i$ 和尺寸耦合在一起。

> AVAS 约定：\`twissz\` 的纵向角度用 $\\delta = \\Delta p/p$，而上式用 $\\dd z/\\dd s = \\delta/\\gamma^2$，二者差 $\\gamma^2$；示例能量下 $\\gamma^2 = 1.003$，差别可以忽略，高能时要注意换算。

### 发射度主导还是空间电荷主导

比较两个散焦项 $K/(4\\sigma)$（圆束）与 $\\varepsilon^2/\\sigma^3$：比值 $K\\sigma^2/(4\\varepsilon^2) \\ll 1$ 时束流**发射度主导**，$\\gg 1$ 时**空间电荷主导**。示例发射度、10 mA 直流束在 $\\sigma = 1.33\\ \\mathrm{mm}$ 时比值约 0.26，仍属发射度主导，但空间电荷已明显改变匹配尺寸（见下节）。

## 相移压低与平滑近似 {#space-charge-tune-depression}

### 平滑近似

当每个聚焦周期 $L_p$ 的零流强相移 $\\sigma_0$ 不太大（约 $\\sigma_0 \\lesssim 60°$–$80°$）时，可以把周期聚焦平均成连续聚焦 $k_0 = \\sigma_0/L_p$。圆截面直流束的 rms 包络方程变为

$$
\\sigma'' + k_0^2\\,\\sigma - \\frac{K}{4\\sigma} - \\frac{\\varepsilon^2}{\\sigma^3} = 0 .
$$

### 匹配尺寸

匹配束 $\\sigma'' = 0$：

$$
k_0^2\\sigma^4 - \\frac{K}{4}\\sigma^2 - \\varepsilon^2 = 0
\\quad\\Longrightarrow\\quad
\\sigma^2 = \\frac{K/4 + \\sqrt{K^2/16 + 4k_0^2\\varepsilon^2}}{2k_0^2} .
$$

零流强时 $\\sigma_{0,\\mathrm{rms}} = \\sqrt{\\varepsilon/k_0}$。

### 压低后的相移

束内粒子受外部聚焦和线性空间电荷散焦，其波数 $k^2 = k_0^2 - K/(4\\sigma^2)$。由匹配条件得 $k = \\varepsilon/\\sigma^2$，代入并整理：

$$
\\frac{\\sigma}{\\sigma_0} = \\frac{k}{k_0} = \\sqrt{1 + u^2} - u,\\qquad
u = \\frac{K}{8 k_0\\,\\varepsilon_{\\mathrm{rms}}} = \\frac{K}{2 k_0\\,\\varepsilon_{\\mathrm{edge}}} ,
$$

其中 $\\varepsilon$ 都是**几何**（非归一化）发射度，$\\varepsilon_{\\mathrm{edge}} = 4\\varepsilon_{\\mathrm{rms}}$。这里 $\\sigma$、$\\sigma_0$ 指每周期的相移（不是 rms 尺寸），$\\sigma/\\sigma_0$ 称为**相移压低**（tune depression）：1 表示无空间电荷，趋于 0 表示极端空间电荷主导。设计中常见 $\\sigma/\\sigma_0 \\gtrsim 0.4$–0.5。

### 数值例

FODO 周期：四极铁 $L_q = 0.1\\ \\mathrm{m}$、$G = 6\\ \\mathrm{T/m}$，漂移 $L_d = 0.2\\ \\mathrm{m}$，周期 $L_p = 0.6\\ \\mathrm{m}$。用 AVAS 的厚透镜矩阵算得 $\\sigma_0 = 52.7°$，$k_0 = 1.534\\ \\mathrm{m}^{-1}$，零流强匹配 rms 尺寸 1.254 mm。

| 束流 | $K$ | $u$ | $\\sigma/\\sigma_0$ | $\\sigma$ | 匹配 rms 尺寸 |
|---|---|---|---|---|---|
| 示例束团（$I_{\\mathrm{eq}} = 3.69\\ \\mathrm{mA}$） | $1.27\\times 10^{-6}$ | 0.043 | 0.958 | 50.5° | 1.281 mm |
| 10 mA 直流 | $3.44\\times 10^{-6}$ | 0.116 | 0.891 | 47.0° | 1.329 mm |
| 30 mA 直流 | $1.03\\times 10^{-5}$ | 0.348 | 0.711 | 37.5° | 1.488 mm |

同一个周期在 $G = 4\\ \\mathrm{T/m}$（$\\sigma_0 = 34.5°$）时，30 mA 直流束的相移压低到 0.60：聚焦越弱，空间电荷的相对影响越大。

\`\`\`avas-lab
space-charge I=10 W=1.527 mass=938.272 q=1 emit=0.13768 G=6 Lq=0.1 Ld=0.2 cells=10
\`\`\`

> 注意：平滑近似忽略了包络在周期内的起伏，$\\sigma_0$ 较大时误差增大；严格的匹配要解周期性的包络方程（见 [匹配](#matching)）。束团的等效 $K$ 还随束团长度变化，表中第一行只是入口处的估计。

## 发射度增长与束晕 {#space-charge-emittance-growth}

线性理论中 rms 发射度守恒。真实束流的分布不是 KV，自场含非线性成分，发射度会增长。主要机制（定性）：

- **电荷重新分布**：rms 匹配但分布不均匀（如高斯、带尖峰的分布）的束，会在约四分之一个等离子体振荡周期内向更均匀的分布弛豫，释放的"自由能"转化为发射度增长。圆截面均匀束的等离子体波数 $k_p^2 = 2K/a^2 = K/(2\\sigma^2)$；10 mA 例子中 $k_p = 0.99\\ \\mathrm{m}^{-1}$，等离子体波长约 6.4 m。空间电荷越强，这一过程越快、增长越大。
- **失配与束晕**：失配束的包络振荡（圆束平滑近似下，呼吸模波数 $k_{\\mathrm{b}} = \\sqrt{2k_0^2 + 2k^2}$，四极模 $k_{\\mathrm{q}} = \\sqrt{k_0^2 + 3k^2}$）会通过 2:1 参数共振把单个粒子推到远离束芯的轨道，形成**束晕**（halo）。束晕粒子很少，几乎不改变 rms 尺寸，却决定了束损。
- **包络不稳定性与共振**：零流强相移 $\\sigma_0 > 90°$ 时，空间电荷可以激发包络不稳定（因此常用 $\\sigma_0 < 90°$ 的设计规则）；周期结构的其他参数共振也会引起增长。
- **横纵耦合**：横向和纵向的"温度"（发射度与聚焦之比）不相等时，空间电荷会在两者之间交换能量（能量均分问题），Hofmann 稳定图给出了危险的参数区域。

在 AVAS 输出中，可以比较 DataSet.txt 的 rms 尺寸（第 16/18/20 列）与最大尺寸（第 22/24/26 列）：最大值与 rms 之比持续增大通常是束晕发展的信号。发射度增长和束晕都要用多粒子内核计算，线性预览不包含这些效应。

## 在 AVAS 中 {#space-charge-avas}

### 关键字

空间电荷由 \`input.txt\` 控制，束流电荷由 \`beam.txt\` 给出：

\`\`\`avas-params
input:spacecharge input:scmethod input:numofgrid input:meshrms input:steppercycle beam:current beam:frequency beam:beamtype beam:particlenumber spacechargecomp
\`\`\`

- \`spacecharge 1\` 打开空间电荷，\`0\` 关闭。
- \`current\`（mA，平均流强）与 \`frequency\`（Hz，束流频率）决定束团电荷 $Q = I/f$；每个宏粒子的电荷为 $Q$ 除以宏粒子数 \`particlenumber\`。\`beamtype dc\` 生成直流束。
- \`scmethod\` 选择算法：\`FFT\`、\`PICNIC\` 或 \`SPICNIC\`。手册只列出名称，没有说明各算法的实现细节；示例项目用 \`FFT\`。改用其他算法时，应在同一问题上与 \`FFT\` 的结果比较。
- \`numofgrid Nx Ny Nz\` 是三个方向的网格数；\`meshrms V1 V2 V3\` 设网格边长 $L_x = 2 V_1\\sigma_x$（$y$、$z$ 同理），即网格覆盖 $\\pm V_1$ 倍 rms 尺寸。网格要盖住整个束团（包括尾部），同时每个网格单元里要有足够的宏粒子，见 [多粒子模拟要点](#simulation)。
- 空间电荷在每个时间步计算，步长由 \`steppercycle\` 决定（每个射频周期的步数）。
- 手册还说明：启用纵向周期性边界时，空间电荷算法自动切换为 FFT，纵向网格长度设为束团周期长度，因此需要更多的纵向网格点。
- lattice 关键字 \`spacechargecomp\`（空间电荷补偿）在手册中没有说明，参数和作用都未记录，不要依赖它。
- \`spacechargelong\`、\`spacechargetype\` 只对包络模型（\`sim_type env\`）起作用。

> 注意：\`input.txt\` 的 \`boundary\` 指束流**损失**边界（\`boundary.txt\`），不是空间电荷求解的边界条件。内核空间电荷求解器的边界条件、坐标系和插值方式手册未说明，以内核为准。

### 线性预览的空间电荷模型

可视化编辑器的线性预览在束流流强大于零时加入线性空间电荷：

- 束团：用 rms 等效的均匀椭球（半轴 $\\sqrt5\\,\\sigma_{x,y,z}$），$M_z$ 按旋转椭球公式和静止系长径比 $p = \\gamma c_z/\\sqrt{ab}$ 计算，横向取 $M_x = (1 - M_z)\\,b/(a + b)$，即上文的 $K_3$ 线性力；需要 \`frequency\` 和 \`twissz\`，缺少时不计空间电荷并给出提示。
- 直流束（\`beamtype dc\`）：用 rms 等效的均匀椭圆截面（半轴 $2\\sigma$），即 KV 力 $2K/(a(a+b))$。
- 每 4 个步长施加一次积分强度的薄透镜冲量（放在这组步长的中间，二阶分裂），力随当前 rms 尺寸更新。

它只描述 rms 包络的线性部分，不含发射度增长、束晕和损失。强空间电荷时它与内核的 rms 尺寸可相差到 13 %。

## 延伸阅读 {#space-charge-further}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008：自场、KV 分布、包络方程、发射度增长。
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008：空间电荷多粒子动力学与束晕。
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18, 1105 (1971).
- I. M. Kapchinsky, V. V. Vladimirsky, Proc. Int. Conf. on High Energy Accelerators, CERN, 1959（KV 分布）。
- I. Hofmann, "Stability of anisotropic beams with space charge", *Phys. Rev. E* 57, 4713 (1998).
- 本入门的 [线性光学](#linear-optics)、[匹配](#matching)、[多粒子模拟要点](#simulation)。
`,F=`# Static electric and magnetic field maps

Besides RF cavities, the \`field\` element can describe fields that do not change in time: field type 2 is a **static electric field** (einzel lenses, electrostatic quadrupoles, electrostatic deflectors …), field type 3 a **static magnetic field** (solenoids, quadrupoles and magnets with a bending component described by a map …). Compared with the matrix-model elements \`quad\`, \`solenoid\` and \`bend\`, a field-map element keeps the real longitudinal profile, the fringe fields and the higher-order components, and the particle motion is integrated directly in the field with the t-code. AVAS has no matrix-model element for electrostatic lenses; electrostatic focusing elements can only be described by type-2 field maps (or a pure deflection by the electric kind 1 of \`steerer\`).

The numerical examples of this chapter use the proton beam of the example project ($W = 1.527\\ \\mathrm{MeV}$, $\\beta = 0.0570$, $\\beta\\gamma = 0.0571$, $p = 53.55\\ \\mathrm{MeV}/c$, $B\\rho = 0.1786\\ \\mathrm{T\\,m}$) and the solenoid field map \`sol\` (0.35 m long).

## Role and position {#static-fields-role}

- **Low-energy beam transport**: sections with energies of only tens to hundreds of keV often use electrostatic einzel lenses and quadrupoles: the electric force $qE$ does not depend on the velocity while the magnetic force $qvB$ falls with it, so at low velocity an electric field of comparable technical difficulty is more effective (see the next section), and there is no hysteresis.
- **Transverse focusing in superconducting linacs**: the superconducting solenoids between the cavities of a low-β section have high fields and long fringes that often overlap the fields of neighbouring cavities; a field map is more reliable than a hard-edge model, see [Solenoid](#solenoid) and [Superpose](#superpose).
- **Combined-function elements**: a magnet with quadrupole plus bending or steering coils, or with significant higher-order fields, is described by one map.
- **Checking matrix models**: comparing a field-map calculation with the hard-edge matrices of [Quadrupole](#quadrupole) and [Solenoid](#solenoid) shows how large the error of the hard-edge approximation is.

## Static electric fields: energy change and electric rigidity {#static-fields-electric}

### Energy follows the potential

A static electric field is conservative, $\\boldsymbol{E} = -\\nabla\\Phi$. The total energy of a particle in an electrostatic element is conserved:

$$
W(\\boldsymbol{r}) + q\\,\\Phi(\\boldsymbol{r}) = \\text{const}
$$

so the particle is accelerated or decelerated between the electrodes, and its kinetic energy is restored when it leaves the element at the original potential. Unlike in a magnetic field, the velocity, momentum and focusing strength change with position inside an electrostatic element — which is why the field map has to be integrated instead of applying a constant-energy matrix. AVAS's linear preview also includes the on-axis $E_z$ of type-2 maps in the energy of the reference particle.

### Electric rigidity

The bending power of a magnetic field is measured by the magnetic rigidity $B\\rho = p/q$; for electric fields the corresponding quantity is the **electric rigidity**

$$
E\\rho = \\frac{p\\,v}{q} = \\frac{\\beta^2\\gamma\\,mc^2}{q}\\ \\xrightarrow{\\ \\beta\\ll 1\\ }\\ \\frac{2W}{q}
$$

For the example proton $E\\rho = 53.55\\ \\mathrm{MeV}/c\\times 0.0570 \\approx 3.05\\ \\mathrm{MV}$ (non-relativistic approximation $2W/q = 3.054\\ \\mathrm{MV}$). A transverse electric field $E$ bends with radius $\\rho = E\\rho/E$; for example $E = 1\\ \\mathrm{MV/m}$ gives $\\rho \\approx 3.05\\ \\mathrm{m}$.

Electric and magnetic fields are equivalent through $E \\leftrightarrow vB$: at $\\beta = 0.057$ a 1 T magnetic field is worth $\\beta c\\times 1\\ \\mathrm{T} \\approx 17\\ \\mathrm{MV/m}$ of electric field. The lower the velocity, the more "valuable" a given electric field, which is why electrostatic elements are used mainly at low energy.

### Electrostatic quadrupole

An ideal electrostatic quadrupole has four hyperbolic electrodes at $\\pm V$ and aperture radius $a$; the potential is

$$
\\Phi = \\frac{V}{a^2}\\,(x^2 - y^2),\\qquad E_x = -\\frac{2V}{a^2}\\,x,\\qquad E_y = +\\frac{2V}{a^2}\\,y
$$

With the electric gradient $G_E = 2V/a^2$ (V/m²) the linear equations of motion are

$$
x'' + k\\,x = 0,\\quad y'' - k\\,y = 0,\\qquad k = \\frac{G_E}{E\\rho}
$$

For $V > 0$ a positive particle is focused in $x$ and defocused in $y$; the transfer matrices have exactly the form of a magnetic quadrupole (see [Quadrupole](#quadrupole)). For example $V = \\pm 10\\ \\mathrm{kV}$, $a = 15\\ \\mathrm{mm}$: $G_E = 8.9\\times 10^7\\ \\mathrm{V/m^2}$, which for the example proton gives $k \\approx 29\\ \\mathrm{m^{-2}}$, the same as a magnetic quadrupole of gradient $kB\\rho \\approx 5.2\\ \\mathrm{T/m}$.

### Einzel lens

An einzel lens consists of three coaxial cylindrical electrodes, the outer two grounded and a voltage on the middle one, so the kinetic energy is the same at entrance and exit. Non-relativistic, axisymmetric and paraxial, let $\\hat\\Phi(z)$ be the on-axis potential referred to "zero kinetic energy" ($q\\hat\\Phi = W$); the trajectory obeys the paraxial ray equation

$$
r'' + \\frac{\\hat\\Phi'}{2\\hat\\Phi}\\,r' + \\frac{\\hat\\Phi''}{4\\hat\\Phi}\\,r = 0
$$

The Picht transformation $R = r\\,\\hat\\Phi^{1/4}$ gives

$$
R'' + \\frac{3}{16}\\left(\\frac{\\hat\\Phi'}{\\hat\\Phi}\\right)^{2}R = 0
$$

The coefficient is always positive, so **an einzel lens always focuses, whether the middle electrode is at positive or negative voltage**. In the weak-lens approximation the focal length is

$$
\\frac{1}{f} \\approx \\frac{3}{16}\\int\\left(\\frac{\\hat\\Phi'}{\\hat\\Phi}\\right)^{2}\\dd z
$$

The decelerating mode (middle electrode potential slows the particles) focuses more strongly at the same voltage, but if $q\\Phi$ approaches the kinetic energy the particles stop or are reflected in the lens. Relativistic corrections are small at the energies of this primer; the full form is in Reiser, chapter 3.

### Electrostatic deflection

A radial electric field between cylindrical or spherical electrodes bends the beam with radius $\\rho = E\\rho/E$. In the bending plane an electrostatic deflector focuses differently from a magnetic dipole (depending on the electrode shape), and the kinetic energy changes with the potential during the bend. AVAS's dipole \`bend\` describes magnetic bending only; electrostatic deflection can only be described by a field map, see [Problems](#static-fields-pitfalls).

## Static magnetic fields: solenoids, quadrupoles and bending components {#static-fields-magnetic}

### Solenoid maps

Near the axis an axisymmetric magnetic field is fixed by the on-axis $B_z(z)$ ($\\nabla\\cdot\\boldsymbol{B} = 0$, $\\nabla\\times\\boldsymbol{B} = 0$):

$$
B_r(r,z) = -\\frac{r}{2}\\,B_z'(z) + \\frac{r^3}{16}\\,B_z'''(z) - \\cdots,\\qquad
B_z(r,z) = B_z(z) - \\frac{r^2}{4}\\,B_z''(z) + \\cdots
$$

At the entrance $B_z' > 0$ and $B_r$ points towards the axis; at the exit $B_z' < 0$ and $B_r$ points away from it (for $B_z > 0$). Crossing the entrance fringe the particle acquires an azimuthal velocity through $F_\\theta = q v_z B_r$; in the body of the solenoid $F_r = q v_\\theta B_z$ pulls it towards the axis — solenoid focusing. By Busch's theorem the two transverse planes decouple in the rotating (Larmor) frame:

$$
r'' + \\left(\\frac{B_z(z)}{2B\\rho}\\right)^{2} r = 0,\\qquad
\\theta_L = \\int\\frac{B_z}{2B\\rho}\\,\\dd z,\\qquad
\\frac{1}{f} \\approx \\int\\left(\\frac{B_z}{2B\\rho}\\right)^{2}\\dd z
$$

The focusing strength is proportional to $B_z^2$ and **independent of the field direction**; changing the sign of $K_b$ only reverses the beam rotation. Checked on the example: with drifts before and after the solenoid map, the linear preview gives identical rms sizes for $K_b = \\pm 0.531$.

### The example solenoid sol

The map \`sol\` is 0.35 m long; the on-axis $B_z$ peaks at the centre $z \\approx 0.175\\ \\mathrm{m}$ and falls to about 0.3 % of the peak at both ends. With $K_b = 0.531$ from the superpose example of the manual:

| Quantity | Value | Remark |
|---|---|---|
| $\\int B_z\\,\\dd z$ | $0.387\\ \\mathrm{T\\,m}$ | insensitive to noise |
| Larmor rotation $\\theta_L$ | $62^\\circ$ | $\\int B_z\\,\\dd z/(2B\\rho)$ |
| $\\int B_z^2\\,\\dd z$ | $1.10$–$1.14\\ \\mathrm{T^2m}$ | noise raises the raw value by about 3 %; the first is after 9-point smoothing |
| thin-lens focal length $f$ | $\\approx 0.115\\ \\mathrm{m}$ | comparable to the length; the thin-lens value is only an order of magnitude |
| hard-edge equivalent $B_{eff} = \\int B^2/\\int B$ | $\\approx 2.85\\ \\mathrm{T}$ | |
| equivalent length $L_{eff} = (\\int B)^2/\\int B^2$ | $\\approx 0.136\\ \\mathrm{m}$ | much shorter than the map |
| smoothed peak field | $\\approx 4.4\\ \\mathrm{T}$ | the raw data jump from point to point; their maximum is not the peak |

The radial field of the map at $r = 1\\ \\mathrm{cm}$ (raw map values): on the entrance side at $z = 0.059\\ \\mathrm{m}$, $B_x(x{=}1\\,\\mathrm{cm}) \\approx -0.053\\ \\mathrm{T}$, pointing to the axis; on the exit side at $z = 0.29\\ \\mathrm{m}$ about $+0.052\\ \\mathrm{T}$, pointing away; the magnitude agrees with $-\\tfrac{r}{2}B_z'$.

### Quadrupole maps

Near the axis a magnetic quadrupole has $B_y = G\\,x$, $B_x = G\\,y$ and $k = G/(B\\rho)$; for $k > 0$ a positive particle is focused in $x$. Quadrupole maps are usually exported normalized to a reference gradient (e.g. 1 T/m at the centre); then $K_b$ is the actual gradient in T/m, and **the sign of $K_b$ sets the polarity**: for $K_b < 0$ the focusing and defocusing planes swap. With fringe fields the effective length $L_{eff} = \\int G(z)\\,\\dd z/G_0$ is used for comparison with a hard-edge quadrupole.

### Bending and steering components

If a map contains a transverse magnetic field on the axis (a dipole component), the particles are deflected. The kernel integrates the map, so the deflection is included; the linear preview does not include it and warns when the estimated angle exceeds 1 mrad. A field map is a straight box along the $z$ axis, and particles leaving the axis gradually move out of the region it covers, so maps are suited to small-angle deflection and steering; for large-angle dipoles use \`bend\` and \`edge\` (see [Dipole](#dipole)), and steering coils can also be described by a [steerer](#steerer). How the kernel defines the reference trajectory for a large deflection in a map is not documented in the manual.

## Equations of motion and transfer matrices {#static-fields-equations}

### The equation integrated by the t-code

In a field-map element the kernel integrates the Lorentz equation for every particle

$$
\\frac{\\dd(\\gamma m\\boldsymbol{v})}{\\dd t} = q\\,\\bigl(\\boldsymbol{E}(\\boldsymbol{r}) + \\boldsymbol{v}\\times\\boldsymbol{B}(\\boldsymbol{r})\\bigr),\\qquad
\\boldsymbol{E} = K_e\\,\\boldsymbol{E}_{map},\\quad \\boldsymbol{B} = K_b\\,\\boldsymbol{B}_{map}
$$

with the time step set by \`steppercycle\` in input.txt. A static magnetic field does no work and the kinetic energy is constant; a static electric field changes it by the energy conservation of the previous section.

### Linearization

Expanding the fields to first order near the axis, the transverse equation of motion is

$$
x'' + \\frac{(\\beta\\gamma)'}{\\beta\\gamma}\\,x' + \\left(\\frac{1}{B\\rho}\\frac{\\partial B_y}{\\partial x} - \\frac{1}{E\\rho}\\frac{\\partial E_x}{\\partial x}\\right)x = 0
$$

(The $x$–$y$ coupling of a solenoid is not written here; with an axial magnetic field use the rotating frame or 4×4 matrices, see [Solenoid](#solenoid).) The second term comes from the momentum change in electric fields. If the momentum change in the element is negligible, replacing the field by a hard-edge element of effective gradient $G_{eff}$ and length $L_{eff}$ brings us back to the matrix model: the thick-lens matrix of the [Quadrupole](#quadrupole) chapter, or for a solenoid the coupled matrix with $k = B_{eff}/(2B\\rho)$, $C = \\cos kL$, $S = \\sin kL$.

### How good is a hard-edge equivalent

Compare two lattices on the example (no space charge, linear preview): drift 0.05 m + map \`sol\` ($K_b = 0.531$) + drift 0.3 m, and a hard-edge \`solenoid\` with $B = 2.85\\ \\mathrm{T}$, $L = 0.136\\ \\mathrm{m}$ at the same centre. Both have the waist at $z \\approx 0.39\\ \\mathrm{m}$, with rms waists of 0.285 mm and 0.310 mm; at $z = 0.7\\ \\mathrm{m}$ the rms sizes are 2.69 mm and 2.43 mm, about 10 % apart. With strong focusing a hard-edge equivalent is good for a first design only; accurate work needs the map.

> **AVAS convention**: the linear preview cuts maps into 4 mm slices and treats each as "half a step of solenoid body + a thin lens from the near-axis transverse gradients + half a step"; the solenoid fringes enter through the gradient terms. The kernel integrates directly in the 3D field. Both include fringe fields; the kernel also includes the non-linear terms.

## In AVAS {#static-fields-in-avas}

\`\`\`text
field  L  R  0  2  0  0  Ke  0   file      ! static electric: .esx .esy .esz
field  L  R  0  3  0  0  Ke  Kb  file      ! static magnetic: .bsx .bsy .bsz
field  0.1   0.02  0  3  0  0  1  0.677098  sol_1     ! manual example
field  0.35  0.02  0  3  0  0  1  0.531     sol       ! example solenoid
\`\`\`

\`\`\`avas-params
field
\`\`\`

- Field type 2 (static electric) or 3 (static magnetic).
- **The frequency \`f\` must be 0**; \`phase\` is 0; \`V3\` has no meaning for static fields, write 0. The manual's rule is "parameters that do not exist are written 0".
- Static electric fields are in MV/m and multiplied by \`Ke\`; static magnetic fields are in T and multiplied by \`Kb\`.
- For static magnetic maps the manual example writes \`Ke\` = 1; without electric component files it has nothing to act on. For static electric maps write \`Kb\` = 0 by the manual's rule.
- \`file\` is the field-map name (without extension); static electric maps need \`.esx/.esy/.esz\`, static magnetic maps \`.bsx/.bsy/.bsz\`; the format is in [Field maps](#field-maps).
- \`L\` should equal the length in the map header; \`R\` is the aperture radius of the loss criterion.

> **AVAS convention**: for a static magnetic quadrupole map the sign of \`Kb\` is the polarity; for a solenoid it only sets the rotation direction. The element list of the GUI sorts field-map elements by field type into RF, electrostatic and magnetostatic; the specific kind of a magnetostatic map (solenoid, quadrupole …) is guessed from the file name, shown with "(guessed)" and explained in the details — a display hint, not physical data.

> **Note**: errors of static magnetic elements are described with the \`err_quad_*\` commands (static-field element errors) and RF cavities with \`err_cav_*\`; see [Errors and correction](#errors).

## Adjustable parameters and typical values {#static-fields-parameters}

| Parameter | Element | Effect | Example / typical value |
|---|---|---|---|
| \`Kb\` | magnetostatic solenoid | field scale, focal length $\\propto 1/K_b^2$ | example 0.531 ($\\int B\\,\\dd z = 0.387\\ \\mathrm{T\\,m}$, $f \\approx 0.12\\ \\mathrm{m}$) |
| \`Kb\` | magnetostatic quadrupole | gradient and polarity | $K_b = G$ for a map normalized to 1 T/m; room-temperature quadrupoles generally below a few tens of T/m |
| \`Ke\` | electrostatic quadrupole, einzel lens | field scale | for a map exported at 1 kV electrode voltage $K_e$ = voltage / kV; low-energy electrodes are often at a few to tens of kV |
| \`L\` | all | element length | equal to the map header length |
| \`R\` | all | loss aperture | equal to the real aperture, not larger than the transverse extent of the map |

Since the focusing strength is $\\propto K_b^2$ (solenoid) or $\\propto K_b$ (quadrupole), \`Kb\` can be scanned directly with the parameter scan: \`avas scan --input InputFile --target NAME --param Kb --values …\`.

## Dynamic effects and common problems {#static-fields-pitfalls}

- **Truncated maps**: the field should have decayed to negligible values at both ends of the map. A map cut off at its boundary gives the particles a spurious edge kick; the example \`sol\` has fallen to about 0.3 % of the peak at both ends.
- **Element length different from the map length**: what the kernel does then is not documented; the linear preview uses the field in $[0, \\min(L, L_{map})]$ only. Always make \`L\` equal the map header.
- **Overlapping fields**: when the fringes of neighbouring elements overlap, do not simply put the two maps end to end (the overlapping parts would be cut off); use [Superpose](#superpose).
- **Solenoid coupling**: a solenoid couples $x$ and $y$ and rotates the beam; after a solenoid the rms $x$ and $y$ of a non-round beam are no longer independent, which matters especially after quadrupoles.
- **Electrostatic reflection**: in a decelerating electrostatic element, if $q\\Phi$ exceeds the kinetic energy the particles stop or are reflected; the linear preview reports "the reference particle stops", and such particles cannot pass the element.
- **Non-zero frequency**: \`f\` of a static field must be 0; writing the beam frequency suggests an RF field, and by the rules it must be 0.
- **Noise**: the example maps are noisy; squared quantities such as $\\int B^2$ are raised systematically by the noise (about 3 % in the example), while linear quantities such as $\\int B$ are hardly affected; see [noise in field maps](#field-maps-interpolation).
- **Large-angle bends**: do not describe a large-angle dipole by a straight field-map box; see [bending components](#static-fields-magnetic).

## Hands-on {#static-fields-lab}

**Lab 1: reading the map.** Open the example solenoid in the field-map viewer: look at the longitudinal cut of \`bsz\`, then switch to the $x$–$y$ cross-section with arrows on and move the cut to the entrance and to the exit. Check that the transverse field points to the axis at the entrance and away from it at the exit, and that it is close to zero in the central cross-section.

\`\`\`avas-field
map sol
\`\`\`

**Lab 2: hard-edge equivalent.** Below is the schematic field of a hard-edge solenoid with the same $\\int B\\,\\dd z$ and $\\int B^2\\,\\dd z$ as the example map (computed from the parameters, hard edges, not field-map data); compare its longitudinal profile with the map above.

\`\`\`avas-field
solenoid B=2.85 L=0.136 R=0.02
\`\`\`

**Lab 3: focusing and rotation.** In the solenoid lab with the equivalent parameters, look at the Larmor rotation angle $kL$ (about $62^\\circ$), the difference between thick- and thin-lens focal lengths, and how the waist moves with B. Then lower W to 0.05 MeV: the thin-lens focal length shrinks about 30 times as $f \\propto (B\\rho)^2 \\propto W$, while $kL$ grows to about 6 rad and the beam is over-focused several times inside the solenoid, so the thin-lens picture fails completely — low-energy sections need much weaker solenoids.

\`\`\`avas-lab
solenoid L=0.136 B=2.85 W=1.527 d1=0.05 d2=0.3
\`\`\`

## Further reading {#static-fields-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, chapter 3 (paraxial ray equation, electrostatic and magnetic lenses, solenoids and Busch's theorem).
- H. Wollnik, *Optics of Charged Particles*: transfer matrices of electrostatic and magnetic elements.
- P. W. Hawkes, E. Kasper, *Principles of Electron Optics*: systematic derivation of einzel lenses and paraxial optics.
- In this primer: [Solenoid](#solenoid), [Quadrupole](#quadrupole), [Dipole](#dipole), [Field maps](#field-maps), [Superpose](#superpose), [References](#references).
`,I=`# 静电与静磁场图元件

除了射频腔，\`field\` 元件还可以描述不随时间变化的场：场类型 2 为**静电场**（单透镜、静电四极、静电偏转板等），场类型 3 为**静磁场**（用场图描述的螺线管、四极铁、带偏转成分的磁铁等）。与矩阵模型元件 \`quad\`、\`solenoid\`、\`bend\` 相比，场图元件保留了真实的纵向分布、边缘场和高阶成分，并用 t-code 在场中直接积分粒子运动。AVAS 没有静电透镜的矩阵模型元件，静电聚焦元件只能用类型 2 的场图描述（或用电场校正铁 \`steerer\` 的类型 1 描述纯偏转）。

本章的数值例子使用示例项目的质子束（$W = 1.527\\ \\mathrm{MeV}$，$\\beta = 0.0570$，$\\beta\\gamma = 0.0571$，$p = 53.55\\ \\mathrm{MeV}/c$，$B\\rho = 0.1786\\ \\mathrm{T\\,m}$）和螺线管场图 \`sol\`（长 0.35 m）。

## 作用与位置 {#static-fields-role}

- **低能束流输运**：能量只有几十 keV 到几百 keV 的区段常用静电单透镜和静电四极：电场力 $qE$ 与速度无关，而磁场力 $qvB$ 随速度降低而减小，低速时同样技术难度的电场更有效（见下节），而且没有磁滞。
- **超导直线加速器的横向聚焦**：低 β 段腔与腔之间的超导螺线管场强高、边缘场长，常和相邻腔的场相互重叠，用场图比硬边模型可靠，参见 [螺线管](#solenoid)、[叠加场](#superpose)。
- **组合功能元件**：同一个磁体中既有四极又有偏转或校正线圈，或磁铁有明显的高阶场，用场图可以一并描述。
- **核对矩阵模型**：用场图计算与硬边矩阵 [四极铁](#quadrupole)、[螺线管](#solenoid) 的结果对比，可以判断硬边近似的误差有多大。

## 静电场：能量变化与电刚度 {#static-fields-electric}

### 能量随电势变化

静电场是保守场，$\\boldsymbol{E} = -\\nabla\\Phi$。粒子在静电元件中的总能量守恒：

$$
W(\\boldsymbol{r}) + q\\,\\Phi(\\boldsymbol{r}) = \\text{常数}
$$

所以粒子在电极之间会被加速或减速，离开元件回到原电势时动能恢复。与磁场不同，静电元件内部的速度、动量和聚焦强度都随位置变化——这正是必须用场图积分、而不能简单套用恒定能量矩阵的原因。AVAS 的线性预览也把类型 2 场图的轴上 $E_z$ 计入参考粒子的能量。

### 电刚度

磁场中的偏转能力用磁刚度 $B\\rho = p/q$ 衡量；静电场中对应的是**电刚度**

$$
E\\rho = \\frac{p\\,v}{q} = \\frac{\\beta^2\\gamma\\,mc^2}{q}\\ \\xrightarrow{\\ \\beta\\ll 1\\ }\\ \\frac{2W}{q}
$$

对示例质子 $E\\rho = 53.55\\ \\mathrm{MeV}/c\\times 0.0570 \\approx 3.05\\ \\mathrm{MV}$（非相对论近似 $2W/q = 3.054\\ \\mathrm{MV}$）。横向电场 $E$ 产生的曲率半径为 $\\rho = E\\rho/E$，例如 $E = 1\\ \\mathrm{MV/m}$ 时 $\\rho \\approx 3.05\\ \\mathrm{m}$。

电场与磁场的等效关系是 $E \\leftrightarrow vB$：在 $\\beta = 0.057$ 时，1 T 的磁场相当于 $\\beta c\\times 1\\ \\mathrm{T} \\approx 17\\ \\mathrm{MV/m}$ 的电场。速度越低，同样的电场越"值钱"，所以静电元件主要用在低能段。

### 静电四极

理想静电四极的四个双曲面电极分别加 $\\pm V$，口径半径为 $a$，电势为

$$
\\Phi = \\frac{V}{a^2}\\,(x^2 - y^2),\\qquad E_x = -\\frac{2V}{a^2}\\,x,\\qquad E_y = +\\frac{2V}{a^2}\\,y
$$

定义电场梯度 $G_E = 2V/a^2$（V/m²），则线性运动方程为

$$
x'' + k\\,x = 0,\\quad y'' - k\\,y = 0,\\qquad k = \\frac{G_E}{E\\rho}
$$

$V > 0$ 时正粒子在 $x$ 方向聚焦、$y$ 方向散焦，与磁四极的传输矩阵形式完全相同（见 [四极铁](#quadrupole)）。例如 $V = \\pm 10\\ \\mathrm{kV}$、$a = 15\\ \\mathrm{mm}$：$G_E = 8.9\\times 10^7\\ \\mathrm{V/m^2}$，对示例质子 $k \\approx 29\\ \\mathrm{m^{-2}}$，相当于梯度 $kB\\rho \\approx 5.2\\ \\mathrm{T/m}$ 的磁四极。

### 单透镜（Einzel lens）

单透镜由三个同轴圆筒电极组成，两端电极接地、中间电极加电压，入口与出口动能相同。在非相对论、轴对称、近轴条件下，令 $\\hat\\Phi(z)$ 为以"动能为零"为参考的轴上电势（$q\\hat\\Phi = W$），粒子轨迹满足近轴射线方程

$$
r'' + \\frac{\\hat\\Phi'}{2\\hat\\Phi}\\,r' + \\frac{\\hat\\Phi''}{4\\hat\\Phi}\\,r = 0
$$

作 Picht 变换 $R = r\\,\\hat\\Phi^{1/4}$ 得到

$$
R'' + \\frac{3}{16}\\left(\\frac{\\hat\\Phi'}{\\hat\\Phi}\\right)^{2}R = 0
$$

右边系数恒为正，所以**无论中间电极加正电压还是负电压，单透镜总是聚焦的**。弱透镜近似下焦距为

$$
\\frac{1}{f} \\approx \\frac{3}{16}\\int\\left(\\frac{\\hat\\Phi'}{\\hat\\Phi}\\right)^{2}\\dd z
$$

减速模式（中间电极电势使粒子减速）在相同电压下聚焦更强，但若 $q\\Phi$ 接近动能，粒子会在透镜中停下或被反射。相对论修正在本手册涉及的能量下很小，完整形式见 Reiser 第 3 章。

### 静电偏转

圆柱或球面电极之间的径向电场可以让束流转弯，半径 $\\rho = E\\rho/E$。静电偏转在偏转平面内的聚焦性质与磁偏转不同（与电极形状有关），而且偏转中粒子动能会随电势变化。AVAS 的二极铁 \`bend\` 只描述磁偏转；静电偏转只能用场图描述，见 [注意事项](#static-fields-pitfalls)。

## 静磁场：螺线管、四极与偏转成分 {#static-fields-magnetic}

### 螺线管场图

轴对称磁场在轴附近由轴上 $B_z(z)$ 唯一确定（$\\nabla\\cdot\\boldsymbol{B} = 0$、$\\nabla\\times\\boldsymbol{B} = 0$）：

$$
B_r(r,z) = -\\frac{r}{2}\\,B_z'(z) + \\frac{r^3}{16}\\,B_z'''(z) - \\cdots,\\qquad
B_z(r,z) = B_z(z) - \\frac{r^2}{4}\\,B_z''(z) + \\cdots
$$

在入口端 $B_z' > 0$，$B_r$ 指向轴心；出口端 $B_z' < 0$，$B_r$ 背离轴心（对 $B_z > 0$）。粒子穿过入口边缘时由 $F_\\theta = q v_z B_r$ 获得角向速度，进入螺线管主体后 $F_r = q v_\\theta B_z$ 把它拉向轴心，这就是螺线管聚焦。由 Busch 定理，在旋转坐标系（Larmor 坐标系）中两个横向平面解耦，满足

$$
r'' + \\left(\\frac{B_z(z)}{2B\\rho}\\right)^{2} r = 0,\\qquad
\\theta_L = \\int\\frac{B_z}{2B\\rho}\\,\\dd z,\\qquad
\\frac{1}{f} \\approx \\int\\left(\\frac{B_z}{2B\\rho}\\right)^{2}\\dd z
$$

聚焦强度正比于 $B_z^2$，**与场的方向无关**；改变 $K_b$ 的符号只改变束流旋转方向。用示例 lattice 验证：螺线管场图前后各接漂移段，$K_b = \\pm 0.531$ 时线性预览得到的 rms 尺寸完全相同。

### 示例螺线管 sol

场图 \`sol\` 长 0.35 m，轴上 $B_z$ 在中心 $z \\approx 0.175\\ \\mathrm{m}$ 处最大，两端降到峰值的约 0.3 %。取手册叠加场示例中的 $K_b = 0.531$：

| 量 | 数值 | 说明 |
|---|---|---|
| $\\int B_z\\,\\dd z$ | $0.387\\ \\mathrm{T\\,m}$ | 对噪声不敏感 |
| Larmor 旋转角 $\\theta_L$ | $62^\\circ$ | $\\int B_z\\,\\dd z/(2B\\rho)$ |
| $\\int B_z^2\\,\\dd z$ | $1.10$–$1.14\\ \\mathrm{T^2m}$ | 噪声使原始数据偏大约 3 %，前者为 9 点平滑后的值 |
| 薄透镜焦距 $f$ | $\\approx 0.115\\ \\mathrm{m}$ | 与长度相当，薄透镜近似只是量级估计 |
| 等效硬边 $B_{eff} = \\int B^2/\\int B$ | $\\approx 2.85\\ \\mathrm{T}$ | |
| 等效长度 $L_{eff} = (\\int B)^2/\\int B^2$ | $\\approx 0.136\\ \\mathrm{m}$ | 远短于场图长度 |
| 平滑后峰值场 | $\\approx 4.4\\ \\mathrm{T}$ | 原始数据逐点跳动，最大值不可直接当作峰值 |

场图横截面在 $r = 1\\ \\mathrm{cm}$ 处的径向场（场图原值）：入口侧 $z = 0.059\\ \\mathrm{m}$ 处 $B_x(x{=}1\\,\\mathrm{cm}) \\approx -0.053\\ \\mathrm{T}$，指向轴心；出口侧 $z = 0.29\\ \\mathrm{m}$ 处约 $+0.052\\ \\mathrm{T}$，背离轴心；量级与 $-\\tfrac{r}{2}B_z'$ 相符。

### 四极场图

磁四极在轴附近 $B_y = G\\,x$、$B_x = G\\,y$，$k = G/(B\\rho)$；正粒子在 $k > 0$ 时 $x$ 方向聚焦。场图四极一般按某个参考梯度（例如中心 1 T/m）归一化导出，此时 $K_b$ 就是实际梯度（T/m），**$K_b$ 的符号决定极性**：$K_b < 0$ 时聚焦和散焦平面互换。有边缘场时常用等效长度 $L_{eff} = \\int G(z)\\,\\dd z/G_0$ 与硬边四极比较。

### 偏转与校正成分

场图中若含有轴上横向磁场（偶极成分），粒子会被偏转。内核按场图积分，偏转自然计入；线性预览不计偏转，当估计偏角超过 1 mrad 时给出警告。场图在空间中是一个沿 $z$ 轴的直长方体，粒子偏离轴线后会逐渐走出场图覆盖的区域，因此场图适合描述小角度的偏转和校正；大角度二极铁请使用 \`bend\` 与 \`edge\`（见 [二极铁](#dipole)），校正线圈也可以用 [校正铁](#steerer) 描述。内核在场图中如何定义大偏转时的参考轨道，手册没有说明。

## 运动方程与传输矩阵 {#static-fields-equations}

### t-code 积分的方程

内核在场图元件中对每个粒子积分 Lorentz 方程

$$
\\frac{\\dd(\\gamma m\\boldsymbol{v})}{\\dd t} = q\\,\\bigl(\\boldsymbol{E}(\\boldsymbol{r}) + \\boldsymbol{v}\\times\\boldsymbol{B}(\\boldsymbol{r})\\bigr),\\qquad
\\boldsymbol{E} = K_e\\,\\boldsymbol{E}_{map},\\quad \\boldsymbol{B} = K_b\\,\\boldsymbol{B}_{map}
$$

时间步长由 input.txt 的 \`steppercycle\` 决定。静磁场不做功，粒子动能不变；静电场按上一节的能量守恒改变动能。

### 线性化

把场在轴附近展开到一阶，横向运动方程可以写成

$$
x'' + \\frac{(\\beta\\gamma)'}{\\beta\\gamma}\\,x' + \\left(\\frac{1}{B\\rho}\\frac{\\partial B_y}{\\partial x} - \\frac{1}{E\\rho}\\frac{\\partial E_x}{\\partial x}\\right)x = 0
$$

（这里没有写出螺线管的 $x$–$y$ 耦合项；有轴向磁场时用旋转坐标系或 4×4 矩阵，见 [螺线管](#solenoid)。）第二项来自静电场中动量的变化。若元件内动量变化可以忽略，把场按等效梯度 $G_{eff}$、等效长度 $L_{eff}$ 换成硬边元件，就回到矩阵模型：四极用 [四极铁](#quadrupole) 一章的厚透镜矩阵，螺线管用 $k = B_{eff}/(2B\\rho)$、$C = \\cos kL$、$S = \\sin kL$ 的耦合矩阵。

### 硬边等效有多准

在示例中比较两个 lattice（无空间电荷，线性预览）：漂移 0.05 m + 场图 \`sol\`（$K_b = 0.531$）+ 漂移 0.3 m，与在相同中心位置放置 $B = 2.85\\ \\mathrm{T}$、$L = 0.136\\ \\mathrm{m}$ 的硬边 \`solenoid\`。两者的束腰都在 $z \\approx 0.39\\ \\mathrm{m}$，rms 束腰分别为 0.285 mm 和 0.310 mm；到 $z = 0.7\\ \\mathrm{m}$ 时 rms 为 2.69 mm 和 2.43 mm，相差约 10 %。强聚焦时硬边等效只能作初步设计，精确计算应使用场图。

> **AVAS 约定**：线性预览把场图切成 4 mm 的薄片，每片做"半步螺线管主体 + 由轴附近横向梯度给出的薄透镜 + 半步"的处理，螺线管的边缘作用由梯度项给出；内核则直接在三维场中积分，二者都包含边缘场，后者还包含非线性项。

## 在 AVAS 中 {#static-fields-in-avas}

\`\`\`text
field  L  R  0  2  0  0  Ke  0   file      ! 静电场：.esx .esy .esz
field  L  R  0  3  0  0  Ke  Kb  file      ! 静磁场：.bsx .bsy .bsz
field  0.1   0.02  0  3  0  0  1  0.677098  sol_1     ! 手册示例
field  0.35  0.02  0  3  0  0  1  0.531     sol       ! 示例螺线管
\`\`\`

\`\`\`avas-params
field
\`\`\`

- 场类型写 2（静电）或 3（静磁）。
- **频率 \`f\` 必须写 0**；\`phase\` 写 0；\`V3\` 对静态场无意义，写 0。手册的规则是"不存在的参数写 0"。
- 静电场的电场单位为 MV/m，乘以 \`Ke\`；静磁场的磁场单位为 T，乘以 \`Kb\`。
- 对静磁场，手册示例中 \`Ke\` 写的是 1；场图没有电场分量文件时它没有作用对象。对静电场，按手册规则 \`Kb\` 写 0。
- \`file\` 为场图文件名（不含扩展名），静电场需要 \`.esx/.esy/.esz\`，静磁场需要 \`.bsx/.bsy/.bsz\`，格式见 [场图](#field-maps)。
- \`L\` 应等于场图头部的长度；\`R\` 是损失判据的孔径半径。

> **AVAS 约定**：静磁场图四极的 \`Kb\` 符号就是极性；螺线管的 \`Kb\` 符号只改变旋转方向。界面的元件列表根据场类型把场图元件分为射频、静电、静磁三类；静磁场图的具体类型（螺线管、四极……）由场图文件名推测，显示时带"（推测）"，识别依据写在详情说明中——这只是显示用的推测，不是物理数据。

> **注意**：静磁元件的误差用 \`err_quad_*\`（静磁元件误差）命令描述，射频腔用 \`err_cav_*\`，参见 [误差与校正](#errors)。

## 可调参数与典型取值 {#static-fields-parameters}

| 参数 | 元件 | 作用 | 示例 / 典型值 |
|---|---|---|---|
| \`Kb\` | 静磁螺线管 | 场强缩放，焦距 $\\propto 1/K_b^2$ | 示例 0.531（$\\int B\\,\\dd z = 0.387\\ \\mathrm{T\\,m}$，$f \\approx 0.12\\ \\mathrm{m}$） |
| \`Kb\` | 静磁四极 | 梯度与极性 | 场图按 1 T/m 归一化时 $K_b = G$；常温四极一般在几十 T/m 以内 |
| \`Ke\` | 静电四极、单透镜 | 电场缩放 | 场图按 1 kV 电极电压导出时 $K_e$ = 电压 / kV；低能段电极电压常为几到几十 kV |
| \`L\` | 全部 | 元件长度 | 等于场图头部长度 |
| \`R\` | 全部 | 损失孔径 | 与真实孔径一致，不大于场图横向范围 |

由于聚焦强度 $\\propto K_b^2$（螺线管）或 $\\propto K_b$（四极），扫描 \`Kb\` 可以直接用参数扫描：\`avas scan --input InputFile --target 元件名 --param Kb --values …\`。

## 动力学效应与常见问题 {#static-fields-pitfalls}

- **场图截断**：场图两端的场应已衰减到可以忽略。若场图在边界处截断，粒子会受到一个虚假的边缘冲击；示例 \`sol\` 两端已降到峰值的约 0.3 %。
- **元件长度与场图长度不一致**：内核对两者不一致时的处理，手册没有说明；线性预览只使用 $[0, \\min(L, L_{map})]$ 范围内的场。务必让 \`L\` 与场图头部一致。
- **重叠的场**：相邻元件的边缘场重叠时，不能简单地把两个场图首尾相接（重叠部分会被截掉），要用 [叠加场](#superpose)。
- **螺线管耦合**：螺线管使 $x$、$y$ 耦合并旋转束流；非圆束经过螺线管后 rms $x$、$y$ 不再独立，四极之后接螺线管时尤其要注意。
- **静电反射**：减速型静电元件中若 $q\\Phi$ 超过动能，粒子会停下或被反射，线性预览会提示"参考粒子停止"；这类粒子无法通过元件。
- **频率写成非零值**：静态场的 \`f\` 必须为 0；写成束流频率会让人误以为它是射频场，规则上应写 0。
- **噪声**：示例场图本身有噪声，$\\int B^2$ 等平方量会被噪声系统性地抬高（示例约 3 %），线性量如 $\\int B$ 基本不受影响，参见 [场图噪声](#field-maps-interpolation)。
- **大角度偏转**：不要用直的场图盒子描述大角度二极铁，见 [偏转成分](#static-fields-magnetic)。

## 动手实验 {#static-fields-lab}

**实验一：读场图。** 在场图查看器中打开示例螺线管：先看 \`bsz\` 的纵切面，再切到 $x$–$y$ 横截面并打开箭头，把截面位置分别移到入口端和出口端，确认横向场在入口指向轴心、在出口背离轴心，中心截面的横向场接近零。

\`\`\`avas-field
map sol
\`\`\`

**实验二：硬边等效。** 下图是与示例场图有相同 $\\int B\\,\\dd z$ 和 $\\int B^2\\,\\dd z$ 的硬边螺线管的示意场（由参数计算，硬边，不是场图数据），与上面的场图对比纵向分布的差别。

\`\`\`avas-field
solenoid B=2.85 L=0.136 R=0.02
\`\`\`

**实验三：聚焦与旋转。** 在螺线管实验中使用等效参数，观察 Larmor 旋转角 $kL$（约 $62^\\circ$）、厚透镜与薄透镜焦距的差别，以及束腰位置随 B 的变化。再把 W 降到 0.05 MeV：薄透镜焦距按 $f \\propto (B\\rho)^2 \\propto W$ 缩短约 30 倍，而 $kL$ 增大到约 6 rad，束流在螺线管内多次过焦，薄透镜近似完全失效——低能段要用弱得多的螺线管。

\`\`\`avas-lab
solenoid L=0.136 B=2.85 W=1.527 d1=0.05 d2=0.3
\`\`\`

## 延伸阅读 {#static-fields-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*，第 3 章（近轴射线方程、静电与磁透镜、螺线管与 Busch 定理）。
- H. Wollnik, *Optics of Charged Particles*：静电与磁元件的传输矩阵。
- P. W. Hawkes, E. Kasper, *Principles of Electron Optics*：单透镜与近轴光学的系统推导。
- 本手册：[螺线管](#solenoid)、[四极铁](#quadrupole)、[二极铁](#dipole)、[场图](#field-maps)、[叠加场](#superpose)、[参考文献](#references)。
`,L=`# Steerer

A steerer (dipole corrector, steering magnet) is a weak dipole field that gives the beam centre a small angular deflection in order to correct the orbit. In a real accelerator quadrupoles, solenoids and cavities inevitably have alignment errors, and the beam centre gradually drifts away from the design axis; steerers distributed along the line, together with beam position monitors (BPMs), "pull" the beam back close to the axis.

This chapter builds on the deflection and transfer-matrix concepts of the [quadrupole](#quadrupole) and [dipole](#dipole) chapters; the sources of errors and AVAS's error and correction commands are covered systematically in [errors and correction](#errors).

## Role and placement {#steerer-role}

- **Orbit correction**: compensating the orbit distortion caused by misaligned elements, field errors and initial beam offsets, and so reducing the losses and effective emittance growth that come from an off-centre beam.
- **Beam centring**: making the beam pass through the electrical centre of quadrupoles, solenoids and cavities, avoiding the extra deflection (quadrupoles), coupling (solenoids) or transverse RF kicks (cavities) of an off-centre passage.
- **Local bumps**: 3–4 steerers create a controlled local orbit offset, for example to avoid an obstacle or to scan the aperture.
- **Placement**: usually next to a quadrupole or built into quadrupole/solenoid coils, interleaved with BPMs, at least one per plane per focusing period.

## Physics and field {#steerer-field}

### Magnetic steerer

The field inside a steerer is nearly uniform, with transverse components $B_x$, $B_y$. For a particle moving along $+z$, $\\boldsymbol{F} = q\\boldsymbol{v}\\times\\boldsymbol{B}$ gives

$$
F_x = -q v B_y,\\qquad F_y = q v B_x ,
$$

so $B_y$ deflects in $x$ and $B_x$ deflects in $y$. In AVAS's cross-section pictures (looking from downstream to upstream, beam coming out of the screen, $x$ to the right, $y$ up), $B_y > 0$ deflects a positive particle to the left ($-x$).

### Electric steerer

A pair of parallel plates produces a transverse electric field $E_x$ (or $E_y$); the force $F_x = q E_x$ does not depend on the velocity and points along the field.

## Equations of motion and transfer matrix {#steerer-matrix}

### Thin kick

For an effective field length $L$ and a small deflection, a steerer is a thin "kick" that changes only the angle, not the position:

$$
\\Delta x' = -\\frac{q B_y L}{p} = -\\frac{B_y L}{B\\rho},\\qquad
\\Delta y' = \\frac{q B_x L}{p} = \\frac{B_x L}{B\\rho}.
$$

For an electric steerer,

$$
\\Delta x' = \\frac{q E_x L}{p v} = \\frac{E_x L}{E\\rho},\\qquad
\\Delta y' = \\frac{E_y L}{E\\rho},\\qquad
E\\rho = \\frac{p v}{q} = B\\rho \\cdot v .
$$

The electric rigidity $E\\rho$ is measured in V; in MeV, $pv = \\beta^2\\gamma\\,mc^2$, about $2W$ in the non-relativistic limit. Electric deflection scales as $1/(pv)$, magnetic deflection as $1/p$: the lower the velocity, the more effective an electric steerer — which is why low-energy transport lines often use electrostatic deflector plates.

### Matrix form

A steerer is not a linear map but a translation:

$$
\\begin{pmatrix} x \\\\ x' \\\\ y \\\\ y' \\end{pmatrix}_1 =
\\begin{pmatrix} x \\\\ x' \\\\ y \\\\ y' \\end{pmatrix}_0 +
\\begin{pmatrix} 0 \\\\ \\Delta x' \\\\ 0 \\\\ \\Delta y' \\end{pmatrix}.
$$

It moves the beam centre and leaves the central second moments (rms sizes, emittances, Twiss parameters) unchanged. In the $6\\times 6$ linear matrix a steerer is the unit matrix; the deflection needs a $7\\times 7$ augmented matrix or a separate tracking of the central orbit.

> AVAS convention: the linear envelope preview treats steerers as unit matrices (\`linear_optics.py\`: steerers and diagnostics are identities); it computes only the rms envelope, not the central orbit. The effect of a steerer on the orbit must be checked with a kernel run: in DataSet.txt the $x$ centre = column 29 (synchronous particle $x$) + column 1 (centroid offset), the $y$ centre = column 31 + column 3, in m.

### Propagation of a kick downstream

An angle $\\theta_j$ applied at position $j$ moves the beam at a downstream position $i$ (without acceleration) by

$$
\\Delta x_i = \\sqrt{\\beta_i\\,\\beta_j}\\,\\sin(\\psi_i - \\psi_j)\\,\\theta_j ,
$$

where $\\beta$ and $\\psi$ are the beta function and phase of that plane. With acceleration, multiply by the adiabatic-damping factor $\\sqrt{(\\beta\\gamma)_j/(\\beta\\gamma)_i}$. Consequences:

- a steerer is "stronger" where $\\beta$ is large (the same angle gives a larger offset);
- the offset is largest $90^\\circ$ downstream in phase and zero at $180^\\circ$ — the phase advance between two neighbouring steerers decides whether position and angle can be corrected at the same time.

## In AVAS {#steerer-avas}

\`\`\`text
steerer 0 R 0 Bx By kind max
\`\`\`

\`\`\`avas-params
steerer
\`\`\`

- The first parameter (length) **must be 0**; the third parameter is reserved, write 0.
- \`R\`: aperture radius (m).
- \`Bx\`, \`By\`: magnetic field (T) for \`kind = 0\`, electric field \`Ex\`, \`Ey\` (V/m) for \`kind = 1\`.
- \`kind\`: 0 = magnetic steerer, 1 = electric steerer.
- \`max\`: the maximum. The manual only says "maximum", usually read as the limit allowed during correction; neither its meaning nor its unit is explained further in the manual — the kernel is the reference.

> AVAS convention: a steerer is written with length 0 but **acts over the next element**: its effective length is by default the length of the next element, and it sits at the middle of the next element (manual). The integrated field is therefore $B\\,L_{\\text{next}}$, and the deflection follows from the formulas above with the length of the next element. The lattice check warns about a \`steerer\` with non-zero length.

Example: a steerer with a vertical field $B_y = 1$ mT on top of a quadrupole:

\`\`\`text
steerer 0 0.02 0 0 0.001 0 0.002
quad 0.1 0.02 0 6
\`\`\`

For the example beam ($B\\rho = 0.17863$ T·m), $B_y L = 1\\times 10^{-4}$ T·m, and the deflection is $\\lvert\\Delta x'\\rvert = 0.56$ mrad. The sign of the deflection follows from $\\boldsymbol{F} = q\\boldsymbol{v}\\times\\boldsymbol{B}$ in a right-handed frame; the sign convention actually used by the kernel is not written in the manual — confirm it with a single-particle run when it matters.

### Correction commands

AVAS's static error correction names the parameter to adjust with \`adjust\` and gives the target with diagnostic commands:

\`\`\`avas-params
adjust diag_position
\`\`\`

\`adjust 0 v n min max first_step\` is written before the element to adjust; \`v\` is the number of the element's parameter. The manual's only example is \`adjust 1 7 5 0 3 0\` before a \`field\`, varying its 7th parameter \`Ke\`, i.e. counting from the first number after the keyword; counting the same way, the 4th parameter of \`steerer\` is \`Bx\` and the 5th is \`By\`. The manual gives no steerer example, so this correspondence is defined by the kernel — before relying on it, confirm with a short error study that the steerer field is actually varied. \`diag_position 0 x y 0\` gives the target centre position (mm). A sketch:

\`\`\`text
adjust 0 5 1 -0.002 0.002 0
steerer 0 0.02 0 0 0 0 0.002
quad 0.1 0.02 0 6
drift 0.3 0.02 0
diag_position 0 0 0 0
\`\`\`

Correction takes place within an error study (it needs \`err_step\` and the other error commands); algorithm, parameter meanings and a complete case are in [errors and correction](#errors).

## Parameters and typical values {#steerer-params}

| Quantity | Typical range | Remark |
|---|---|---|
| integrated field $BL$ | $10^{-5}$–$10^{-3}$ T·m | small room-temperature corrector coils |
| electric field $E$ | $10^4$–$10^6$ V/m | low-energy electrostatic deflectors |
| deflection angle | 0.1 to a few mrad | set by the orbit error budget |
| number | one per plane every 1–2 focusing periods | paired with BPMs |

### Worked example

Example beam: protons at 1.527 MeV, $B\\rho = 0.17863$ T·m, $E\\rho = 3.05$ MV ($pv = 3.05$ MeV).

| Steerer | Field | Effective length | Deflection |
|---|---|---|---|
| magnetic | $B_y = 1$ mT | 0.1 m | 0.56 mrad |
| magnetic | $B_y = 10$ mT | 0.1 m | 5.6 mrad |
| electric | $E_x = 10^5$ V/m | 0.1 m | 3.28 mrad |

A 1 mrad deflection needs $BL = 1.79\\times 10^{-4}$ T·m or $EL = 3.05\\times 10^{3}$ V. At such a low velocity ($\\beta = 0.057$), $10^5$ V/m is equivalent to a magnetic field of 5.9 mT ($E = vB$, $v = 1.71\\times 10^7$ m/s).

## Dynamics and common problems {#steerer-dynamics}

### The basic idea of orbit correction

Write the response of all BPMs to all steerers as a response matrix $\\mathsf{R}$, $R_{ij} = \\partial x_i/\\partial\\theta_j$ (in a linac non-zero only for $j$ upstream of $i$; the values come from the formula above or from simulations that change one steerer at a time). From a measured orbit $\\boldsymbol{x}$ the correction is

$$
\\boldsymbol{\\theta} = -\\,\\mathsf{R}^{+}\\,\\boldsymbol{x},
$$

where $\\mathsf{R}^{+}$ is the pseudo-inverse (the least-squares solution, usually by singular value decomposition, truncating small singular values when needed to avoid excessive corrections). Common simplifications are "one-to-one" correction (each steerer zeroes the nearest downstream BPM) and local bumps. AVAS's \`adjust\` iterates the parameters by gradient descent until the diagnostic targets are met (see [errors and correction](#errors)).

### Steerers have side effects

- **Dispersion**: a steerer is in essence a weak dipole; after a deflection there is a small dispersion $D \\sim \\theta\\cdot(\\text{distance})$; for beams with a large energy spread, large corrections increase the effective size.
- **Off-centre passage through elements**: a centre of zero at the BPM does not mean zero at the quadrupole or cavity centre; offsets between BPM and magnetic centre (BPM offsets) also have to be considered.
- **Field quality**: the field of a small steerer is non-uniform near the edge of the aperture, so large-amplitude particles are deflected differently; the matrix model does not include this.
- **Saturation and limits**: corrections close to \`max\` mean the errors exceed the correction capability; check the error settings or add steerers.

### Common problems

- Writing a non-zero length for \`steerer\`: the manual requires 0, and the lattice check warns.
- Putting a \`steerer\` last in the lattice: there is no "next element" for it to act over; and as a matrix-model element it should not be at either end anyway.
- \`kind\` not matching the units: with \`kind = 1\` the \`Bx\`, \`By\` positions hold V/m, values usually several orders of magnitude larger than magnetic fields in T.
- No visible effect in the linear preview: the linear preview does not track the central orbit; use a kernel run.

## Hands-on {#steerer-lab}

The field on the steerer cross-section (**schematic**, computed from the parameters): a uniform transverse field in the bore. By AVAS's convention the cross-section is seen from downstream to upstream with the beam coming out of the screen (⊙), so for $B_y > 0$ you can read off directly that a positive particle is pushed to the left.

\`\`\`avas-field
steerer Bx=0 By=0.01 R=0.02
\`\`\`

With $B_x > 0$ instead: the field points right and the force points up ($+y$).

\`\`\`avas-field
steerer Bx=0.01 By=0 R=0.02
\`\`\`

To see the effect on the orbit, add a \`steerer\` (followed by a drift or a quadrupole) to a copy of the example project, run the kernel with a single particle or a few particles, watch the beam centre (column 29 + column 1) along $z$ in DataSet.txt, and check the downstream offset and its direction against $\\Delta x' = B_y L/B\\rho$.

## Further reading {#steerer-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed. (errors, alignment and orbit correction).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (orbit distortion and correction, response matrices).
- CERN Accelerator School lectures on beam diagnostics and orbit correction (CAS proceedings).
- Related chapters: [errors and correction](#errors), [quadrupole](#quadrupole), [dipole](#dipole), [simulation](#simulation); the full bibliography is in [references](#references).
`,R=`# 校正铁

校正铁（steerer，又称偶极校正器、导向磁铁）是一个弱的二极场，用来给束流中心一个小角度的偏转，从而修正轨道。实际加速器中的四极铁、螺线管、腔体都不可避免地存在安装误差，束流中心会逐渐偏离设计轴线；沿线分布的校正铁与束流位置探测器（BPM）配合，把束流"拉回"轴线附近。

本章依赖[四极铁](#quadrupole)和[二极铁](#dipole)中的偏转与传输矩阵概念；误差的来源和 AVAS 的误差、校正命令在[误差与校正](#errors)一章中系统介绍。

## 作用与位置 {#steerer-role}

- **轨道校正**：抵消元件错位、磁场误差和初始束流偏移造成的轨道畸变，降低因中心偏离而带来的束损和有效发射度增长。
- **束流对中**：让束流穿过四极铁、螺线管和腔体的电中心，避免偏心通过带来的额外偏转（四极铁）、耦合（螺线管）或横向 RF 踢（腔体）。
- **局部凸轨（bump）**：用 3–4 个校正铁在局部制造一段受控的轨道偏移，例如绕开障碍或扫描孔径。
- **位置**：通常紧邻四极铁或嵌入四极铁 / 螺线管线圈内，与 BPM 交错分布，每个平面每个聚焦周期至少一个。

## 物理原理与场 {#steerer-field}

### 磁场校正铁

校正铁内的场近似均匀，横向分量为 $B_x$、$B_y$。对沿 $+z$ 运动的粒子，$\\boldsymbol{F} = q\\boldsymbol{v}\\times\\boldsymbol{B}$ 给出

$$
F_x = -q v B_y,\\qquad F_y = q v B_x .
$$

即 $B_y$ 使粒子在 $x$ 方向偏转，$B_x$ 使粒子在 $y$ 方向偏转。在 AVAS 的横截面图中（从下游往上游看，束流垂直屏幕向外，$x$ 向右、$y$ 向上），$B_y > 0$ 使正电荷粒子向左（$-x$）偏转。

### 电场校正铁

一对平行极板产生横向电场 $E_x$（或 $E_y$），力 $F_x = q E_x$ 与速度无关，方向与电场相同。

## 运动方程与传输矩阵 {#steerer-matrix}

### 薄透镜踢

设场作用的有效长度为 $L$，且偏转角很小，校正铁相当于一个只改变角度、不改变位置的薄"踢"：

$$
\\Delta x' = -\\frac{q B_y L}{p} = -\\frac{B_y L}{B\\rho},\\qquad
\\Delta y' = \\frac{q B_x L}{p} = \\frac{B_x L}{B\\rho}.
$$

对电场校正铁，

$$
\\Delta x' = \\frac{q E_x L}{p v} = \\frac{E_x L}{E\\rho},\\qquad
\\Delta y' = \\frac{E_y L}{E\\rho},\\qquad
E\\rho = \\frac{p v}{q} = B\\rho \\cdot v .
$$

电刚度 $E\\rho$ 的单位是 V；以 MeV 计时 $pv = \\beta^2\\gamma\\,mc^2$，非相对论下约为 $2W$。可以看出，电场偏转 $\\propto 1/(pv)$，磁场偏转 $\\propto 1/p$：速度越低，电场校正铁越有效，这是低能束流传输线常用静电偏转板的原因。

### 矩阵形式

校正铁的作用不是线性映射，而是平移：

$$
\\begin{pmatrix} x \\\\ x' \\\\ y \\\\ y' \\end{pmatrix}_1 =
\\begin{pmatrix} x \\\\ x' \\\\ y \\\\ y' \\end{pmatrix}_0 +
\\begin{pmatrix} 0 \\\\ \\Delta x' \\\\ 0 \\\\ \\Delta y' \\end{pmatrix}.
$$

它移动束流中心，不改变束流的二阶中心矩（rms 尺寸、发射度、Twiss 参数）。在 $6\\times 6$ 线性矩阵中校正铁等于单位矩阵，偏转需要用 $7\\times 7$ 的增广矩阵或单独跟踪中心轨道来表示。

> AVAS 约定：线性包络预览把校正铁当作单位矩阵（\`linear_optics.py\` 说明：steerers and diagnostics are identities），只计算 rms 包络，不计算中心轨道。校正铁对轨道的效果要用内核运行查看：DataSet.txt 中 $x$ 中心 = 第 29 列（同步粒子 $x$）+ 第 1 列（质心相对偏移），$y$ 中心 = 第 31 列 + 第 3 列，单位 m。

### 偏转在下游的传播

在第 $j$ 个位置施加角度 $\\theta_j$，下游第 $i$ 个位置（无加速）的位置偏移为

$$
\\Delta x_i = \\sqrt{\\beta_i\\,\\beta_j}\\,\\sin(\\psi_i - \\psi_j)\\,\\theta_j ,
$$

其中 $\\beta$、$\\psi$ 是该平面的 beta 函数与相位。有加速时还要乘以绝热阻尼因子 $\\sqrt{(\\beta\\gamma)_j/(\\beta\\gamma)_i}$。这说明：

- 校正铁在 $\\beta$ 大的地方更"有力"（同样的角度产生更大的偏移）；
- 相位差 $90^\\circ$ 处偏移最大，$180^\\circ$ 处为零——相邻两个校正铁之间的相移决定能否同时校正位置和角度。

## 在 AVAS 中 {#steerer-avas}

\`\`\`text
steerer 0 R 0 Bx By kind max
\`\`\`

\`\`\`avas-params
steerer
\`\`\`

- 第一个参数（长度）**必须为 0**；第三个参数保留，写 0。
- \`R\`：孔径半径（m）。
- \`Bx\`、\`By\`：\`kind = 0\` 时为磁场（T），\`kind = 1\` 时为电场 \`Ex\`、\`Ey\`（V/m）。
- \`kind\`：0 = 磁场校正铁，1 = 电场校正铁。
- \`max\`：最大值。手册只写“最大值”，通常理解为校正时允许的上限；其含义和单位手册都没有进一步说明，以内核为准。

> AVAS 约定：校正铁长度写 0，但它**作用于下一个元件**：作用长度默认等于下一个元件的长度，并位于下一个元件的中间（手册原文）。因此积分场为 $B\\,L_{\\text{next}}$，偏转角按上面的公式用下一个元件的长度计算。lattice 检查会对长度不为 0 的 \`steerer\` 给出警告。

例：在四极铁上叠加一个垂直磁场 $B_y = 1$ mT 的校正铁：

\`\`\`text
steerer 0 0.02 0 0 0.001 0 0.002
quad 0.1 0.02 0 6
\`\`\`

对示例束流（$B\\rho = 0.17863$ T·m），$B_y L = 1\\times 10^{-4}$ T·m，偏转 $\\lvert\\Delta x'\\rvert = 0.56$ mrad。偏转的正负方向按右手系由 $\\boldsymbol{F} = q\\boldsymbol{v}\\times\\boldsymbol{B}$ 推出；内核实际采用的符号约定手册没有写明，需要时用单粒子运行确认。

### 校正命令

AVAS 的静态误差校正由 \`adjust\` 指定要调的参数，由束诊命令给出目标：

\`\`\`avas-params
adjust diag_position
\`\`\`

\`adjust 0 v n min max first_step\` 写在要调的元件前，\`v\` 是该元件的第几个参数。手册唯一的例子是 \`adjust 1 7 5 0 3 0\` 写在 \`field\` 前、修改第 7 个参数 \`Ke\`，即从关键字后第一个数起计；按同样的数法类推，\`steerer\` 的第 4 个是 \`Bx\`，第 5 个是 \`By\`。手册没有给出校正铁的例子，这一对应以内核为准，使用前先用一次短的误差研究确认校正铁的场确实被修改。\`diag_position 0 x y 0\` 给出目标中心位置（mm）。一个示意片段：

\`\`\`text
adjust 0 5 1 -0.002 0.002 0
steerer 0 0.02 0 0 0 0 0.002
quad 0.1 0.02 0 6
drift 0.3 0.02 0
diag_position 0 0 0 0
\`\`\`

校正在误差研究中进行（需要 \`err_step\` 等命令），算法、参数含义与完整的案例见[误差与校正](#errors)。

## 可调参数与典型取值 {#steerer-params}

| 量 | 典型范围 | 说明 |
|---|---|---|
| 积分场 $BL$ | $10^{-5}$–$10^{-3}$ T·m | 常温小型校正线圈 |
| 电场 $E$ | $10^4$–$10^6$ V/m | 低能静电偏转板 |
| 偏转角 | 0.1–数 mrad | 由轨道误差预算决定 |
| 数量 | 每个平面每 1–2 个聚焦周期一个 | 与 BPM 配对 |

### 数值例子

示例束流：质子 1.527 MeV，$B\\rho = 0.17863$ T·m，$E\\rho = 3.05$ MV（$pv = 3.05$ MeV）。

| 校正铁 | 场 | 作用长度 | 偏转角 |
|---|---|---|---|
| 磁场 | $B_y = 1$ mT | 0.1 m | 0.56 mrad |
| 磁场 | $B_y = 10$ mT | 0.1 m | 5.6 mrad |
| 电场 | $E_x = 10^5$ V/m | 0.1 m | 3.28 mrad |

产生 1 mrad 偏转需要 $BL = 1.79\\times 10^{-4}$ T·m 或 $EL = 3.05\\times 10^{3}$ V。对这样低的速度（$\\beta = 0.057$），$10^5$ V/m 的电场与 5.9 mT 的磁场等效（$E = vB$，$v = 1.71\\times 10^7$ m/s）。

## 动力学效应与常见问题 {#steerer-dynamics}

### 轨道校正的基本思路

把所有校正铁到所有 BPM 的响应写成响应矩阵 $\\mathsf{R}$，$R_{ij} = \\partial x_i/\\partial\\theta_j$（直线加速器中只有 $j$ 在 $i$ 上游时非零，数值由上节公式或逐个改动校正铁的模拟得到）。测得轨道 $\\boldsymbol{x}$ 后，求校正量

$$
\\boldsymbol{\\theta} = -\\,\\mathsf{R}^{+}\\,\\boldsymbol{x},
$$

$\\mathsf{R}^{+}$ 是伪逆（最小二乘解，常用奇异值分解，必要时截断小奇异值以避免校正量过大）。常见的简化做法有"一对一"校正（每个校正铁对准下游最近的 BPM）和局部凸轨。AVAS 的 \`adjust\` 以梯度下降迭代调节参数，使束诊目标满足（见[误差与校正](#errors)）。

### 校正铁也有副作用

- **色散**：校正铁本质上是弱二极铁，偏转后会产生小量色散 $D \\sim \\theta\\cdot(\\text{距离})$；对能散大的束流，大偏转的校正会使有效尺寸变大。
- **偏心通过元件**：校正后中心在 BPM 处为零，不代表在四极铁或腔中心为零；BPM 与元件磁中心的偏差（BPM 偏置）同样需要考虑。
- **场质量**：小型校正铁的场在孔径边缘不均匀，大振幅粒子受到的偏转不同，矩阵模型不包含这一效应。
- **饱和与上限**：校正量用到 \`max\` 附近说明误差超出了校正能力，应检查误差设置或增加校正铁。

### 常见问题

- 给 \`steerer\` 写了非零长度：手册要求长度为 0，lattice 会给出警告。
- 把 \`steerer\` 放在 lattice 最后：它没有"下一个元件"可作用；而且它是矩阵模型元件，本来也不宜放在首尾。
- \`kind\` 与单位不符：\`kind = 1\` 时 \`Bx\`、\`By\` 位置写的是 V/m，数值通常比磁场（T）大好几个数量级。
- 在线性预览里看不到校正铁的效果：线性预览不跟踪中心轨道，要用内核运行。

## 动手实验 {#steerer-lab}

校正铁横截面的场（**示意**，按参数算出）：孔径内是均匀的横向场。按 AVAS 的约定，横截面从下游往上游看、束流垂直屏幕向外（⊙），对 $B_y > 0$ 可以直接读出正电荷粒子受力向左。

\`\`\`avas-field
steerer Bx=0 By=0.01 R=0.02
\`\`\`

换成 $B_x > 0$：场水平向右，受力向上（$+y$）。

\`\`\`avas-field
steerer Bx=0.01 By=0 R=0.02
\`\`\`

要看轨道效果，可以在示例项目的副本中加入一个 \`steerer\`（后接一段 drift 或四极铁），以单粒子或少量粒子运行内核，观察 DataSet.txt 中束流中心（第 29 列 + 第 1 列）沿 $z$ 的变化，并用 $\\Delta x' = B_y L/B\\rho$ 核对下游的偏移量和方向。

## 延伸阅读 {#steerer-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.（误差、对中与轨道校正的讨论）。
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed.（轨道畸变与校正、响应矩阵）。
- CERN Accelerator School 关于束流诊断与轨道校正的讲义（CAS proceedings）。
- 相关章节：[误差与校正](#errors)、[四极铁](#quadrupole)、[二极铁](#dipole)、[模拟](#simulation)；完整书目见[参考文献](#references)。
`,z=`# Superposed fields

In a real accelerator the fields of neighbouring elements often overlap in space: the fringe of a superconducting solenoid reaches into the next cavity, a solenoid carries steering coils, magnets with long fringes sit close together. Elements in a lattice are placed end to end by default, each taking its own stretch of $z$; to let the fields of several elements act together in the same region you need the **superpose** command \`superpose\`. This chapter explains the physical premise of superposition, AVAS's syntax and rules, how longitudinal positions are computed, the difference between \`superposeend\` and \`superposeout\`, and superpose groups in the GUI.

## Role and position {#superpose-role}

Typical uses:

- **Solenoid and RF cavity**: in low-β superconducting sections the solenoid sits right next to the cavity and the fringe of the solenoid map overlaps the cavity map. The example of the manual and of the case tutorial superposes exactly the solenoid map \`sol\` and the cavity map \`hwr010\`.
- **Magnets with steering coils**: a solenoid or quadrupole with embedded dipole steering coils; the map of the coils can be superposed on the map of the main magnet.
- **Maps with long fringes**: putting two maps end to end forces you to cut each fringe at the boundary; superposition keeps the full fringes of both maps and adds the fields where they overlap.

When superposition is not needed: a steerer inside a matrix-model quadrupole is written simply as \`steerer\` before \`quad\` — a steerer has length 0 and by default acts over the whole length of the next element, centred on it; see [Steerer](#steerer).

## Physics: linear superposition of fields {#superpose-physics}

Maxwell's equations in vacuum are linear, so the total field of several sources is the vector sum of their fields. Inside a superpose block the force on a particle is

$$
\\boldsymbol{F} = q\\Bigl[\\sum_i \\boldsymbol{E}_i(\\boldsymbol{r}, t) + \\boldsymbol{v}\\times\\sum_i \\boldsymbol{B}_i(\\boldsymbol{r}, t)\\Bigr]
$$

where each field is shifted to the coordinates of its element: the map coordinate of element $i$ is $z_i' = z - z_b - z_{0,i}$ ($z_b$ being the start of the block, see [the next section](#superpose-z)), and it is multiplied by its own Ke, Kb; RF fields also carry their time factor (cos for the electric, sin for the magnetic field, see [RF cavities](#rf-cavity-in-avas)). The kernel integrates in the total field with the t-code.

Premises and limits of superposition:

- **Only linear media can be superposed.** Adding separately computed maps assumes that the elements do not influence each other. When magnets with iron yokes are close together, saturation and mutual magnetization change their fields; superconducting cavity walls expel an applied static magnetic field (Meissner effect), so the real stray field of a solenoid inside a cavity differs from the separately computed solenoid map. For accurate work, compute the combined field in one electromagnetic calculation and use it as a single map.
- **Time structure of RF fields.** Each RF cavity has its own frequency and phase; AVAS allows at most one RF cavity per superpose block, with no such limit for static fields. The manual does not give the reason for this rule; cases where the fields of two cavities overlap significantly cannot be described with AVAS's superposition.
- **When a static field overlaps an RF field**, the static magnetic field does no work and does not change the energy gain directly; what it changes is the transverse motion inside the cavity.

## Longitudinal positions {#superpose-z}

### The zero of the block

A block starts at its first \`superpose\`. If the lattice has accumulated $z_b$ at that point (the total length of all elements before the first superpose), element $i$ of the block occupies

$$
z \\in \\bigl[\\,z_b + z_{0,i},\\ \\ z_b + z_{0,i} + L_i\\,\\bigr]
$$

where $z_{0,i}$ is the first parameter of the \`superpose\` in front of that element, "the entrance of this element relative to the entrance of the element after the first superpose". The first superpose must be all zeros; it defines the zero.

### Position after the block

The AVAS GUI (Lattice page, 2D layout, linear preview) takes the length of the block as the furthest exit of its elements:

$$
z_{after} = z_b + \\max_i\\bigl(z_{0,i} + L_i\\bigr)
$$

Elements after \`superposeend\` continue from $z_{after}$. The manual does not state the kernel's rule for the position after a block explicitly; before production runs check the total length with the positions in DataSet.txt (column 5 plus column 33) or the beamline picture.

### Example

The lattice of the manual and the case tutorial:

\`\`\`text
start
drift      0.085  0.02   0
superpose  0 0 0 0 0 0
field      0.35   0.02     0   3   0   0   1    0.531  sol
superpose  0.345 0 0 0 0 0
field      0.21   0.02     0   1   162.5e6   -33   1.36    -1.36   hwr010
superposeend
end
\`\`\`

| Element | $z_0$ / m | Occupies / m |
|---|---|---|
| drift | — | 0 – 0.085 |
| sol (magnetostatic) | 0 | 0.085 – 0.435 |
| hwr010 (RF) | 0.345 | 0.430 – 0.640 |
| after the block | — | from 0.640 |

The last 5 mm of the solenoid map overlap the first 5 mm of the cavity map, and both fringes are kept in full. With $z_0$ = 0.35 (the solenoid length) the two elements would just touch, equivalent to writing two \`field\` lines one after the other without superposition; the linear preview gives identical results for both.

> **Note**: in the manual's original text the second superpose of this example has 7 numbers (\`superpose 0.345 0 0 0 0 0 0\`), one 0 too many; superpose has 6 parameters, see the table below.

## In AVAS {#superpose-in-avas}

\`\`\`text
superpose     z0  x0  y0  θz0  θx0  θy0
superposeend
superposeout  z0  x0  y0  θz0  θx0  θy0
\`\`\`

\`\`\`avas-params
superpose superposeend superposeout
\`\`\`

Rules (from the manual; the Lattice page checks them):

1. The parameters of the first \`superpose\` must all be 0; it is the zero of the other superposes.
2. The 6 parameters of \`superpose\` (and \`superposeout\`) give the position ($z_0$, $x_0$, $y_0$, in m) and angles ($\\theta_{z0}$, $\\theta_{x0}$, $\\theta_{y0}$, in degrees) of the entrance plane (transformation plane) of the following element relative to the entrance plane of the element after the first superpose.
3. A block must end with \`superposeend\` or \`superposeout\`; the parameters other than $z_0$ take effect only when it ends with \`superposeout\`. With \`superposeend\` the elements are superposed only longitudinally.
4. At most one RF cavity per block.
5. Every element in a block has exactly one \`superpose\` in front of it.

> **AVAS convention**: the manual only gives examples of superposed field-map elements. What happens with matrix-model elements (\`quad\`, \`solenoid\`, \`bend\` …) inside a block is not documented; the linear preview ignores elements other than \`field\`, \`drift\`, \`steerer\` and diagnostics inside a block, with a warning. Put only field-map elements in a block.

## superposeend and superposeout {#superpose-out}

| End | Parameters that take effect | Use |
|---|---|---|
| \`superposeend\` | only the $z_0$ of each superpose | longitudinal overlap of coaxial elements (the usual case) |
| \`superposeout\` | $z_0$ and $x_0$, $y_0$, $\\theta_{z0}$, $\\theta_{x0}$, $\\theta_{y0}$ | elements offset or rotated relative to the zero of the block, e.g. a solenoid displaced or tilted relative to the cavity, or a map rotated about $z$ |

The manual says that the 6 parameters after \`superpose\` (\`superposeout\`) give the position and angles of the entrance plane of the element following the command relative to the zero of the block. Read literally, the parameters of \`superposeout\` itself refer to the element after the block; whether the kernel uses them to shift or turn the beamline after the block is not explained further in the manual — the kernel is the reference.

> **Note**: the exact conventions of offsets and rotations (order of the three rotations, positive directions) are not documented in the manual; the kernel is the reference. Before relying on them, check with a single-particle run (\`particlenumber 1\`) that a simple offset shows up in the trajectory as expected. The GUI's beamline layout does not read these parameters, and the linear preview ignores the offsets and rotations of superposeout (with a warning). For static alignment errors the usual way is the error commands, see [Errors and correction](#errors).

## Superpose groups in the GUI {#superpose-gui}

- **Structure tree**: a block is shown as a group \`superpose (n)\`, $n$ being the number of elements in it; the superpose lines, the elements and the end line all sit inside the group.
- **Checks**: the first superpose not all zeros (warning); a block not closed by superposeend / superposeout (error); an element without a superpose in front of it (error); two superpose lines in a row (error); more than one RF cavity in a block (warning).
- **2D layout and the small beamline picture of the Lattice page**: overlapping elements are drawn in separate layers according to their real $z$ ranges, with unchanged horizontal positions and lengths; the layers are only schematic and do not mean transverse offsets. Each layer can be selected by clicking, hovering lists all superposed elements at that position, and a bracket marks the superpose group; there is no limit on the number of layers, and many layers scroll.
- **Drag and drop in the visual editor**: dropping an element on the picture of a non-drift element superposes it starting at that element's entrance, and several elements with the same start can be stacked in a row; dropping inside an ordinary non-drift element creates a superpose group automatically, endpoints are still inserted in order and drifts are split as usual. Positions inside a group are relative to its first element, which stays at 0; changing the length of a group moves the downstream elements. Buttons acting on the whole group say "whole group". The context menu of the 2D and 3D views offers superpose, copy, delete and more; write operations are disabled in browse mode or under the run lock.
- **Linear preview**: the fields of all maps of a block are added by position and computed together; reference-particle energy, transverse gradients and solenoid fields are all superposed.

## Common problems {#superpose-pitfalls}

- **First line not zero**: a non-zero $z_0$ in the first superpose shifts the whole block wrongly; it must be all zeros.
- **Missing superpose**: every element of a block needs one, and two in a row are not allowed either.
- **Forgetting the end**: without superposeend / superposeout all following elements count as part of the block.
- **Two RF cavities**: cavities with overlapping fields cannot go into the same block. If only the tails of the fringes overlap slightly, cut the maps where the field has decayed to negligible values and put them end to end (the field at the cut must be small enough, see [the ends of a map](#field-maps-grid)).
- **Wrong block length**: the elements after a block start at the furthest exit inside the block, not at the exit of the last element; write the elements of a block in the order of $z$.
- **Using superposition as a "patch"**: superposition only holds for fields computed separately in linear media; interactions between ferromagnetic or superconducting boundaries cannot be approximated by it, see [Physics](#superpose-physics).
- **Offsets ignored**: with superposeend, $x_0$, $y_0$ and the angles have no effect; use superposeout when offsets are needed.

## Hands-on {#superpose-lab}

Starting from the [example lattice](#superpose-z), add a 0.3 m drift after the block and change only the $z_0$ of the superpose in front of the cavity (linear preview, no space charge):

| $z_0$ / m | block length / m | overlap with the solenoid | exit energy / MeV |
|---|---|---|---|
| 0.35 | 0.560 | none (equivalent to placing them in sequence) | 1.8665 |
| 0.345 | 0.555 | 5 mm | 1.8665 |
| 0.30 | 0.510 | 50 mm | 1.8665 |
| 0.25 | 0.460 | 100 mm | 1.8665 |

The exit energy hardly changes (by less than 0.1 keV): the static magnetic field does no work, and the cavity is given its synchronous phase with V3 = 0, so the kernel (and the preview) solves the entry phase again for the new position. What changes is the transverse motion: as the cavity moves further into the downstream fringe of the solenoid, the transverse beam size inside the cavity and the difference between $x$ and $y$ change.

Suggested exercises:

1. Copy the example project, create a new lattice on the Lattice page with the example above, and check that the 2D layout draws the two elements in two layers and that hovering over the overlap shows both.
2. Change $z_0$ to 0.35, then rewrite it as two consecutive \`field\` lines, and check that the linear-preview curves of the two versions coincide.
3. End the block with superposeout and give the cavity's superpose $x_0 = 0.001$; look at the warning of the linear preview, then use a single-particle run to see how the trajectory changes in the kernel.
4. In the field-map viewer look at how $B_z$ decays at the exit of the solenoid map, and judge how much field is left in the 5 mm overlap:

\`\`\`avas-field
map sol
\`\`\`

## Further reading {#superpose-reading}

- The "superpose" section of the original manual and the case "Superpose, multi-particle introduction" (parameter and file reference and case tutorials in the help window).
- The SUPERPOSE_MAP command in the TraceWin user manual: the same idea, but lengths in mm and not exactly the same parameters.
- In this primer: [RF cavities](#rf-cavity), [Static field maps](#static-fields), [Field maps](#field-maps), [Lattice design](#lattice-design), [References](#references).
`,B=`# 叠加场

真实加速器中，相邻元件的场常常在空间上重叠：超导螺线管的边缘场伸进相邻的腔，螺线管里绕有校正线圈，长边缘场的磁铁靠得很近。lattice 中的元件默认首尾相接、各占一段 $z$，要让几个元件的场在同一段空间里同时作用，就要用**叠加场**命令 \`superpose\`。本章说明叠加的物理前提、AVAS 的语法和规则、纵向位置如何计算、\`superposeend\` 与 \`superposeout\` 的区别，以及界面中的叠加组。

## 作用与位置 {#superpose-role}

典型用途：

- **螺线管与射频腔**：低 β 超导段中螺线管紧挨着腔，螺线管场图的边缘部分与腔场图重叠。手册与案例教程的示例就是螺线管场图 \`sol\` 与腔场图 \`hwr010\` 的叠加。
- **带校正线圈的磁铁**：螺线管或四极中嵌有偶极校正线圈，可以把校正线圈的场图与主磁铁场图叠加。
- **边缘场很长的场图**：两个场图首尾相接时，必须把各自的边缘场截断在交界处；叠加允许两个场图都保留完整的边缘场，在重叠区内场相加。

不需要用叠加的情况：矩阵模型的四极里放一个校正铁，直接在 \`quad\` 前写 \`steerer\` 即可——校正铁长度为 0，默认作用于下一个元件的全长并位于其中间，见 [校正铁](#steerer)。

## 物理原理：场的线性叠加 {#superpose-physics}

真空中的 Maxwell 方程是线性的，几个源产生的总场等于各自场的矢量和。叠加块内粒子受到的力为

$$
\\boldsymbol{F} = q\\Bigl[\\sum_i \\boldsymbol{E}_i(\\boldsymbol{r}, t) + \\boldsymbol{v}\\times\\sum_i \\boldsymbol{B}_i(\\boldsymbol{r}, t)\\Bigr]
$$

其中每个场按其所在元件的坐标平移：第 $i$ 个元件的场图坐标 $z_i' = z - z_b - z_{0,i}$（$z_b$ 为块的起点，见 [下一节](#superpose-z)），并乘以各自的 Ke、Kb；射频场再乘以各自的时间因子（电场 cos、磁场 sin，见 [射频腔](#rf-cavity-in-avas)）。内核用 t-code 在总场中积分。

叠加的前提和局限：

- **只有线性介质才能叠加**。分别计算的场图相加，默认各元件互不影响。含铁磁轭的磁铁靠得很近时，铁的饱和与相互磁化会改变各自的场；超导腔壁会排斥外加的静磁场（迈斯纳效应），螺线管在腔内的真实杂散场与单独计算的螺线管场图并不相同。要求精确时应在同一个电磁场计算中求出组合场，再作为一张场图使用。
- **射频场的时间结构**。不同射频腔各自有频率与相位，AVAS 规定一组叠加场中最多一个射频腔；静态场的个数不受此限制。手册没有说明这条规则的原因；两个腔的场明显重叠的情形不能用 AVAS 的叠加描述。
- **静态场与射频场叠加时**，静磁场不做功，不直接改变能量增益；它改变的是腔内的横向运动。

## 纵向位置的计算 {#superpose-z}

### 块的零点

叠加块从第一条 \`superpose\` 开始。设此时 lattice 已累计到 $z_b$（第一条 superpose 之前所有元件的总长），则块内第 $i$ 个元件占据

$$
z \\in \\bigl[\\,z_b + z_{0,i},\\ \\ z_b + z_{0,i} + L_i\\,\\bigr]
$$

其中 $z_{0,i}$ 是该元件前面那条 \`superpose\` 的第一个参数，即"该元件入口相对于第一条 superpose 后元件入口的纵向位置"。第一条 superpose 的参数必须全为 0，它定义了零点。

### 块之后的位置

AVAS 界面（结构页、二维布局、线性预览）把块的长度取为块内元件出口的最远位置：

$$
z_{after} = z_b + \\max_i\\bigl(z_{0,i} + L_i\\bigr)
$$

\`superposeend\` 之后的元件从 $z_{after}$ 接着排。手册没有明确写出内核对块后位置的规则；正式计算前可以用运行结果 DataSet.txt 的位置（第 5 列加第 33 列）或束线图核对总长度。

### 示例

手册与案例教程中的 lattice：

\`\`\`text
start
drift      0.085  0.02   0
superpose  0 0 0 0 0 0
field      0.35   0.02     0   3   0   0   1    0.531  sol
superpose  0.345 0 0 0 0 0
field      0.21   0.02     0   1   162.5e6   -33   1.36    -1.36   hwr010
superposeend
end
\`\`\`

| 元件 | $z_0$ / m | 占据范围 / m |
|---|---|---|
| drift | — | 0 – 0.085 |
| sol（静磁） | 0 | 0.085 – 0.435 |
| hwr010（射频） | 0.345 | 0.430 – 0.640 |
| 块之后 | — | 从 0.640 开始 |

螺线管场图的最后 5 mm 与腔场图的最初 5 mm 重叠，两张场图的边缘都完整保留。若把 $z_0$ 写成 0.35（= 螺线管长度），两个元件恰好首尾相接，与不用叠加、依次写两个 \`field\` 等价；线性预览中两种写法的结果完全相同。

> **注意**：手册原文的这个例子中第二条 superpose 写了 7 个数（\`superpose 0.345 0 0 0 0 0 0\`），多了一个 0；superpose 只有 6 个参数，见下表。

## 在 AVAS 中 {#superpose-in-avas}

\`\`\`text
superpose     z0  x0  y0  θz0  θx0  θy0
superposeend
superposeout  z0  x0  y0  θz0  θx0  θy0
\`\`\`

\`\`\`avas-params
superpose superposeend superposeout
\`\`\`

规则（来自手册，结构页会检查）：

1. 第一条 \`superpose\` 的参数必须全为 0，作为其他 superpose 的零点。
2. \`superpose\`（以及 \`superposeout\`）的 6 个参数给出其后一个元件的入口平面（转换平面）相对于第一条 superpose 后元件入口平面的位置（$z_0$、$x_0$、$y_0$，m）和角度（$\\theta_{z0}$、$\\theta_{x0}$、$\\theta_{y0}$，度）。
3. 叠加块必须以 \`superposeend\` 或 \`superposeout\` 结束；只有以 \`superposeout\` 结束时，$z_0$ 以外的参数才生效。以 \`superposeend\` 结束时元件只在纵向上叠加。
4. 一组叠加场中最多只能有一个射频腔。
5. 叠加块中每个元件前有且只有一条 \`superpose\`。

> **AVAS 约定**：手册只给出了场图元件的叠加示例。块内放矩阵模型元件（\`quad\`、\`solenoid\`、\`bend\` 等）的行为没有文档；线性预览会忽略块内除 \`field\`、\`drift\`、\`steerer\` 和束诊以外的元件并给出警告。请只在块内放场图元件。

## superposeend 与 superposeout {#superpose-out}

| 结束方式 | 生效的参数 | 用途 |
|---|---|---|
| \`superposeend\` | 只有各 superpose 的 $z_0$ | 同轴元件的纵向重叠（最常见） |
| \`superposeout\` | $z_0$ 以及 $x_0$、$y_0$、$\\theta_{z0}$、$\\theta_{x0}$、$\\theta_{y0}$ | 块内元件相对于零点有横向偏移或转角，例如研究螺线管相对腔的错位、倾斜，或绕 $z$ 轴转动场图 |

手册的原文是 \`superpose\`（\`superposeout\`）后面的 6 个参数给出该命令之后一个元件的入口平面相对于块零点的位置和角度。按字面理解，\`superposeout\` 自身的参数对应块之后的元件；内核是否据此改变块后束线的位置和方向，手册没有进一步说明，以内核为准。

> **注意**：偏移和转角的具体约定（三个转角的作用顺序、坐标系的正方向）手册没有说明，以内核为准；使用前建议用单粒子运行（\`particlenumber 1\`）验证一个简单的偏移是否按预期出现在轨迹中。AVAS 界面的束线布局不读取这些参数，线性预览也忽略 superposeout 的偏移和转角（会给出警告）。对静态的安装误差，更常规的做法是使用误差命令，见 [误差与校正](#errors)。

## 界面中的叠加组 {#superpose-gui}

- **结构树**：一个叠加块显示为一个组 \`superpose (n)\`，$n$ 是块内元件数；块内的 superpose 行、元件和结束行都在组内。
- **检查提示**：第一条 superpose 不全为 0（警告）；块没有以 superposeend / superposeout 结束（错误）；元件前缺少 superpose（错误）；连续两条 superpose（错误）；块内多于一个射频腔（警告）。
- **二维布局与结构页小束线图**：按真实 $z$ 区间把重叠的元件上下分层显示，横向位置和长度不变；分层只是示意，不表示横向偏移。每一层可以单独点击选中，鼠标悬停时列出该位置的全部叠加元件，叠加组用括线标出；层数不限，层多时可以滚动。
- **可视化编辑器的拖放**：把元件拖到某个非漂移元件的图形上，会以该元件的入口为起点建立叠加，可以连续叠加多个同起点元件；拖到普通非漂移元件内部会自动创建叠加组，端点仍按顺序插入，漂移段照常拆分。组内位置相对首元件计算，首元件固定为 0；组长度变化会移动下游元件。作用于整组的按钮会明确写出"整组"。二维、三维视图中的右键菜单提供叠加元件、复制、删除等操作；浏览状态或运行锁下写操作不可用。
- **线性预览**：同一块内所有场图的场按位置相加后统一计算，参考粒子能量、横向梯度与螺线管场都一并叠加。

## 常见问题 {#superpose-pitfalls}

- **第一条不为零**：第一条 superpose 写了非零 $z_0$，整个块会被错误地平移；它必须全为 0。
- **漏写 superpose**：块内每个元件前都要有一条，连续两条也不行。
- **忘记结束**：没有 superposeend / superposeout 时，后面的元件都会被当作块内元件。
- **两个射频腔**：两个腔的场重叠时不能放进同一块。若只是边缘场的尾部轻微重叠，可以在场已衰减到可忽略处截断场图、首尾相接（截断处的场要足够小，见 [场图两端](#field-maps-grid)）。
- **把块长度算错**：块后元件从块内最远的出口开始，而不是从最后一个元件的出口开始；块内元件建议按 $z$ 的顺序书写。
- **把叠加当成"补丁"**：叠加只适用于线性介质中分别计算的场，铁磁或超导边界之间的相互作用不能用叠加近似，见 [物理原理](#superpose-physics)。
- **偏移参数被忽略**：以 superposeend 结束时，$x_0$、$y_0$ 和角度都不起作用；需要偏移时要用 superposeout。

## 动手实验 {#superpose-lab}

以 [示例 lattice](#superpose-z) 为基础，在块后再接一段 0.3 m 的漂移段，只改变腔前那条 superpose 的 $z_0$（线性预览，无空间电荷）：

| $z_0$ / m | 块长度 / m | 与螺线管的重叠 | 出口能量 / MeV |
|---|---|---|---|
| 0.35 | 0.560 | 无（等价于依次排列） | 1.8665 |
| 0.345 | 0.555 | 5 mm | 1.8665 |
| 0.30 | 0.510 | 50 mm | 1.8665 |
| 0.25 | 0.460 | 100 mm | 1.8665 |

出口能量几乎不变（差别小于 0.1 keV）：静磁场不做功，而腔用 V3 = 0 指定同步相位，内核（以及预览）会按新的位置重新反解入口相位。变化的是横向：随着腔更多地进入螺线管的下游边缘场，束流在腔内的横向尺寸和 $x$、$y$ 的差别都会改变。

建议的练习：

1. 复制示例项目，在结构页新建一个 lattice，写入上面的示例，确认二维布局中两个元件分两层显示，悬停在重叠处能看到两个元件。
2. 把 $z_0$ 改成 0.35，再改成依次写两个 \`field\` 的形式，比较两种写法的线性预览曲线是否重合。
3. 把块的结束改成 superposeout，并给腔的 superpose 一个 $x_0 = 0.001$，观察线性预览给出的警告；再用单粒子运行看内核中轨迹的变化。
4. 在场图查看器中看螺线管场图出口端的 $B_z$ 衰减，判断 5 mm 的重叠里还有多少场：

\`\`\`avas-field
map sol
\`\`\`

## 延伸阅读 {#superpose-reading}

- 原手册"叠加场"一节与案例教程"叠加场多粒子入门"（帮助窗口的参数与文件参考、案例教程）。
- TraceWin 用户手册中 SUPERPOSE_MAP 命令的说明：思路相同，但长度单位为 mm，参数不完全一致。
- 本手册：[射频腔](#rf-cavity)、[静电与静磁场图](#static-fields)、[场图](#field-maps)、[结构设计](#lattice-design)、[参考文献](#references)。
`,V=`# Hands-on: build a small superconducting linac

This chapter ties the previous ones together. Using only two field maps of the example project \`examples/hwr010\` — the 162.5 MHz half-wave resonator \`hwr010\` (0.21 m) and the solenoid \`sol\` (0.35 m) — you build a proton superconducting accelerating section step by step. At every step you can drag the parameters and see the linear envelope preview at once, or press "Run with the engine" to let the AVAS multi-particle engine simulate it and overlay both results.

> **Note**: engine runs happen in a sandbox copy in the AVAS data folder (\`%LOCALAPPDATA%\\AVAS\\learn\\runs\\\`); only the last few runs are kept, no project file is changed and no project needs to be open. While it runs, the Run page, tool bar and status bar show it and can pause or stop it; no other simulation can start meanwhile. When you are happy with a step, "Save as a new project" turns it into an ordinary AVAS project.

## The tutorial workbench {#tutorial-workbench}

The tool below contains all five steps. When you move on to the next step, the parameters you chose (solenoid factor, phase …) are carried over.

\`\`\`avas-tutorial
build-linac
\`\`\`

Every step starts from the example project's \`beam.txt\`: protons, kinetic energy $W_0 = 1.527\\ \\mathrm{MeV}$ ($\\beta \\approx 0.057$), frequency 162.5 MHz, Twiss parameters $\\alpha_x = -0.658$, $\\beta_x = 0.377\\ \\mathrm{mm/mrad}$, normalized rms emittance $\\varepsilon_{n,x} = 0.138\\ \\pi\\,\\mathrm{mm\\,mrad}$ (similar in $y$), current 0.184 mA (adjustable in step 5). $\\alpha_x < 0$ means the beam is diverging at the entrance.

> **AVAS convention**: dashed curves (or the only curve) are the linear envelope preview, a quick approximation (rms usually within 1–2 %, up to some ten percent with strong multi-particle effects); the solid curves that appear after an engine run are the multi-particle engine, which is the reference. See [linear preview and engine](#simulation).

## Step 1: a beam in a drift {#tutorial-step1}

A single drift \`drift L 0.02 0\`. The beam enters diverging ($\\alpha<0$) and nothing focuses it, so the rms size grows as in the [drift](#drift) chapter:

$$
\\sigma_x^2(s) = \\sigma_{x,0}^2 - 2\\sigma_{xx',0}\\, s + \\sigma_{x'x',0}\\, s^2 ,
$$

i.e. $\\sigma_x(s) = \\sqrt{\\varepsilon\\,(\\beta_0 - 2\\alpha_0 s + \\gamma_0 s^2)}$. For $L = 0.8\\ \\mathrm{m}$ the preview gives rms $x$ growing from 0.95 mm to about 3.1 mm. The longitudinal plane behaves alike: with no focusing force the bunch lengthens through its momentum spread ($R_{56} = L/\\gamma^2$), rms $z$ from about 1.3 mm to about 3.1 mm.

**Try**: increase $L$ to 2 m and see how quickly the envelope approaches the aperture (be careful beyond about a third of the 20 mm bore); then compare with "Run with the engine" — in a plain drift the preview and the engine should agree closely.

## Step 2: focusing with a solenoid {#tutorial-step2}

A field-map solenoid follows the entrance:

\`\`\`text
field 0.35 0.02 0 3 0 0 0 Kb sol
\`\`\`

\`type = 3\` is a static magnetic field, frequency and phase are 0 and so is \`Ke\`; the smoothed on-axis peak of the map \`sol\` is about 8.3 T (the map is noisy; single points reach 10 T), so $B_{z,\\max} \\approx 8.3\\,K_b\\ \\mathrm{T}$. As in the [solenoid](#solenoid) chapter the focusing strength is $k = B/(2B\\rho)$ and the thin-lens focal length is about

$$
\\frac{1}{f} \\approx \\int k^2\\, \\dd z = \\int \\left(\\frac{B_z}{2B\\rho}\\right)^2 \\dd z .
$$

The example protons have $B\\rho \\approx 0.179\\ \\mathrm{T\\,m}$. With the default $K_b = 0.35$ (peak about 2.9 T) the beam forms a waist at $z \\approx 0.6\\ \\mathrm{m}$ (about 0.2 m after the solenoid exit), smallest rms $x$ about 0.82 mm; $K_b = 0.25$ is too weak (rms $x$ only comes down to about 1.5 mm); $K_b = 0.45$ is too strong: the waist comes earlier and is smaller (about 0.43 mm), after which the beam diverges faster and reaches 3.1 mm at the exit. The focal length scales with $B^2$, so a 30 % change of $K_b$ shows clearly.

**Note**: the solenoid also rotates the transverse phase space about the axis (Larmor angle $\\theta = \\int k\\,\\dd z$) and couples $x$ and $y$, but for a round beam rms $x$ and rms $y$ stay equal. This is why low-energy sections like solenoids: they focus both planes at once.

## Step 3: accelerating in a cavity {#tutorial-step3}

An HWR cavity is added:

\`\`\`text
field 0.21 0.02 0 1 162.5e6 phis Ke -Ke hwr010
\`\`\`

\`type = 1\` is an RF field; \`V3 = 0\` makes the sixth parameter the synchronous phase $\\varphi_s$ (the engine solves for the entry phase). The electric field acts as $K_e E(\\boldsymbol r)\\cos(\\omega t + \\varphi_0)$, the magnetic field as $K_b B(\\boldsymbol r)\\sin(\\omega t + \\varphi_0)$; as in the example, $K_b = -K_e$. The reference particle gains approximately

$$
\\Delta W = q V_0 T \\cos\\varphi_s ,
$$

see [longitudinal dynamics](#longitudinal). The preview for $K_e = 1.36$:

| $\\varphi_s$ | exit energy (MeV) | exit rms $z$ (mm) |
|---|---|---|
| −60° | 1.694 | 2.15 |
| −30° | 1.884 | 1.12 |
| 0° | 2.002 | 2.43 |
| +20° | 1.995 | 5.05 |

The gain is largest near $\\varphi_s = 0$ (on crest), but the bunch lengthens; for $\\varphi_s < 0$ the gain is slightly lower and the bunch is compressed — **phase stability**: in AVAS's $\\cos(\\omega t+\\varphi)$ convention a negative synchronous phase gives late particles more energy, so they catch up with the reference particle. On the positive side ($+20^\\circ$) there is no longitudinal focusing and the bunch lengthens quickly. The price is [RF defocusing](#longitudinal): the stronger the longitudinal focusing, the stronger the transverse defocusing, which is why the transverse envelope is largest at $\\varphi_s = -60^\\circ$.

**Try**: fine-tune the phase around $-30^\\circ$ and watch the trade-off between exit energy and rms $z$; then run the engine and compare the energy (preview and engine usually differ by about 0.01 %) and rms $z$ (larger differences: longitudinal non-linearity is strong in a low-energy cavity and the linear preview does not model longitudinal emittance growth).

## Step 4: a periodic accelerating section {#tutorial-step4}

Repeating "solenoid + drift + cavity + drift" $N$ times gives the most common structure of a superconducting linac: one focusing element and one cavity per period.

\`\`\`text
! period k
field 0.35 0.02 0 3 0 0 0 Kb sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 phis Ke -Ke hwr010
drift 0.1 0.02 0
\`\`\`

A period is about 0.76 m long. The preview ($K_b = 0.35$, $\\varphi_s = -30^\\circ$, $K_e = 1.36$) gives about 1.88 MeV after 1 period, 2.96 MeV after 3 and 4.99 MeV after 6. The first period gains about 0.36 MeV, later ones gradually more (on average about 0.58 MeV over 6 periods): as the particles speed up they come closer to the velocity this cavity suits and its transit-time factor grows. The largest transverse rms is about 1.8 mm and rms $z$ oscillates below about 3 mm instead of growing without bound — both planes are held by the periodic focusing.

Points to keep in mind (see [building an accelerator](#lattice-design)):

- The transverse and longitudinal phase advances per period, $\\sigma_{0t}$ and $\\sigma_{0l}$, should both stay below 90°, and low-order resonances between the two planes (for example both nearly equal, or in a simple integer ratio; see the Hofmann stability chart) should be avoided.
- As the energy rises, $\\beta\\lambda$ grows and $B\\rho$ increases, so the same $K_b$ focuses less and the cavity's transit-time factor changes. Real designs adjust $K_b$, $\\varphi_s$ and even the cavity type section by section; this tutorial keeps one set of values for simplicity.
- The entrance beam is not matched to this periodic structure: the envelope wobble over the first periods is the sign of the mismatch, see [matching](#matching).

**Try**: increase the number of periods to 8 and check whether the energy keeps growing; set $K_b$ to 0.25 or 0.45 and watch the envelope get out of hand.

## Step 5: adding beam current {#tutorial-step5}

The last step lets you change \`current\` in \`beam.txt\`. The higher the current, the stronger the defocusing by [space charge](#space-charge), which effectively weakens the focusing of every period:

$$
K = \\frac{q I}{2\\pi \\varepsilon_0 m c^3 (\\beta\\gamma)^3}
$$

(generalized perveance of a continuous beam; for a bunched beam use the average charge as an approximation). The preview's largest rms $x$ over 3 periods: about 1.8 mm at 0.18 mA, 2.8 mm at 5 mA, 4.6 mm at 15 mA and 6.7 mm at 30 mA; rms $z$ grows too. At low energy ($\\beta\\gamma \\approx 0.057$) $(\\beta\\gamma)^3$ is small, so a few milliamperes already matter.

This step is the one to check with the engine: it solves the self-fields with macro-particles on a mesh and shows what the linear model cannot — emittance growth, halo and losses ("Transmission (engine)" below 100 % means particles hit the aperture). Raise "Macro-particles" above 5000 to reduce the statistical noise, at the cost of a slower run, see [multi-particle simulation](#simulation).

## Engine runs and saving as a project {#tutorial-engine}

"Run with the engine" writes the step's lattice (the text shown under "AVAS lattice of this step") and the modified \`beam.txt\` into the sandbox, runs the engine with the example project's \`input.txt\` (space charge on, FFT solver), reads rms sizes, energy and surviving macro-particles from \`DataSet.txt\` and draws them over the preview. The run time grows with the number of periods and of macro-particles; for reference, the example project (5260 macro-particles, one cavity) takes about 10–30 s on an ordinary computer.

When comparing:

- The energy curves should almost coincide; a clear difference usually means a wrong phase or field factor.
- At low current the rms sizes should be close (within a few percent); at high current the difference grows and the engine is the reference.
- If you changed parameters after the run, the tool tells you that the engine result belongs to the earlier parameters.

"Save as a new project" creates \`InputFile/\` (\`beam.txt\`, \`input.txt\`, \`ini.ini\`, \`lattice_mulp.txt\` and the field maps used) and \`OutputFile/\` in a new folder of your choice and opens it. From there you can keep editing on the Lattice page (output planes, errors, diagnostics …), run it from the Run page and analyse it on the Results page.

## Where to go next {#tutorial-next}

- To design your own focusing period: read [linear optics](#linear-optics) and [building an accelerator](#lattice-design), and estimate the phase advance with the FODO lab.
- To study tolerances: save the tutorial as a project and add cavity phase and amplitude errors and solenoid misalignments as in [errors and correction](#errors), then run an error study.
- If something puzzles you, press "Ask the tutor" at the top of the page: the AI assistant explains it with this primer and the lab numbers.
`,H=`# 动手：搭一台小型超导直线加速器

本章把前面各章的内容串起来：只用示例项目 \`examples/hwr010\` 里的两张场图——162.5 MHz 半波长谐振腔 \`hwr010\`（0.21 m）和螺线管 \`sol\`（0.35 m）——一步一步搭出一段质子超导加速段。每一步你都可以拖动参数，立即看到线性包络预览；也可以点「用内核运行」，让 AVAS 多粒子内核真正跑一遍，再把两种结果叠在一起比较。

> **说明**：内核运行发生在 AVAS 数据目录（\`%LOCALAPPDATA%\\AVAS\\learn\\runs\\\`）中的沙盒副本里，只保留最近几次运行，不会修改任何项目文件，也不需要先打开项目。运行期间运行页、工具栏和状态栏会显示这次运行，可以暂停或停止；此时不能开始其他模拟。满意之后可以点「另存为新项目」，把当前这一步保存成普通 AVAS 项目继续深入。

## 教程工具 {#tutorial-workbench}

下面的工具包含全部五个步骤。切换步骤时，你在前一步选好的参数（螺线管系数、相位等）会带到下一步。

\`\`\`avas-tutorial
build-linac
\`\`\`

所有步骤的初始束流都来自示例项目的 \`beam.txt\`：质子，动能 $W_0 = 1.527\\ \\mathrm{MeV}$（$\\beta \\approx 0.057$），频率 162.5 MHz，Twiss 参数 $\\alpha_x = -0.658$、$\\beta_x = 0.377\\ \\mathrm{mm/mrad}$、归一化 rms 发射度 $\\varepsilon_{n,x} = 0.138\\ \\pi\\,\\mathrm{mm\\,mrad}$（$y$ 方向相近），流强 0.184 mA（第 5 步可以调节）。$\\alpha_x < 0$ 说明束流在入口处正在发散。

> **AVAS 约定**：曲线中虚线（或只有一条曲线时）是线性包络预览，只是快速近似（rms 通常差 1–2 %，多粒子效应强时可到十几个百分点）；内核运行后出现的实线才是多粒子内核的结果，以它为准。见 [线性预览与内核](#simulation)。

## 第 1 步：漂移段中的束流 {#tutorial-step1}

只有一段漂移段 \`drift L 0.02 0\`。束流在入口处发散（$\\alpha<0$），没有任何聚焦，rms 尺寸按 [漂移段](#drift) 中的规律增大：

$$
\\sigma_x^2(s) = \\sigma_{x,0}^2 - 2\\sigma_{xx',0}\\, s + \\sigma_{x'x',0}\\, s^2 ,
$$

即 $\\sigma_x(s) = \\sqrt{\\varepsilon\\,(\\beta_0 - 2\\alpha_0 s + \\gamma_0 s^2)}$。$L = 0.8\\ \\mathrm{m}$ 时，预览给出 rms $x$ 从 0.95 mm 增大到约 3.1 mm。纵向也一样：漂移段中没有聚焦力，束团因动量分散而拉长（$R_{56} = L/\\gamma^2$），rms $z$ 从约 1.3 mm 增大到约 3.1 mm。

**试一试**：把 $L$ 加到 2 m，看包络有多快超过孔径（20 mm 的 1/3 左右就要警惕）；再用「用内核运行」比较——单纯漂移段里线性预览和内核应当很接近。

## 第 2 步：用螺线管聚焦 {#tutorial-step2}

在入口后放一个场图螺线管：

\`\`\`text
field 0.35 0.02 0 3 0 0 0 Kb sol
\`\`\`

\`type = 3\` 表示静磁场，频率与相位写 0，\`Ke\` 写 0；场图 \`sol\` 轴上峰值平滑后约 8.3 T（场图有噪声，个别点达 10 T），所以 $B_{z,\\max} \\approx 8.3\\,K_b\\ \\mathrm{T}$。按 [螺线管](#solenoid) 一章，聚焦强度 $k = B/(2B\\rho)$，薄透镜焦距约

$$
\\frac{1}{f} \\approx \\int k^2\\, \\dd z = \\int \\left(\\frac{B_z}{2B\\rho}\\right)^2 \\dd z .
$$

示例质子 $B\\rho \\approx 0.179\\ \\mathrm{T\\,m}$。默认 $K_b = 0.35$（峰值约 2.9 T）时，束流在 $z \\approx 0.6\\ \\mathrm{m}$（螺线管出口后约 0.2 m）处形成腰，rms $x$ 最小约 0.82 mm；$K_b = 0.25$ 太弱，rms $x$ 最小只降到约 1.5 mm；$K_b = 0.45$ 太强，腰更早、更细（约 0.43 mm），之后发散得更快，到出口反而有 3.1 mm。焦距随 $B^2$ 变化，所以 $K_b$ 改动 30 % 效果就很明显。

**注意**：螺线管同时让横向相空间绕轴旋转（拉莫尔角 $\\theta = \\int k\\,\\dd z$），$x$ 与 $y$ 耦合，但对圆束来说 rms $x$ 与 rms $y$ 仍然一样。这也是低能段常用螺线管的原因：它同时聚焦两个方向。

## 第 3 步：在腔中加速 {#tutorial-step3}

加入一个 HWR 腔：

\`\`\`text
field 0.21 0.02 0 1 162.5e6 phis Ke -Ke hwr010
\`\`\`

\`type = 1\` 表示射频场，\`V3 = 0\` 表示第六个参数是同步相位 $\\varphi_s$（内核自己求入口相位），电场按 $K_e E(\\boldsymbol r)\\cos(\\omega t + \\varphi_0)$、磁场按 $K_b B(\\boldsymbol r)\\sin(\\omega t + \\varphi_0)$ 作用；这里与示例一样取 $K_b = -K_e$。参考粒子的能量增益近似为

$$
\\Delta W = q V_0 T \\cos\\varphi_s ,
$$

见 [纵向动力学](#longitudinal)。预览中 $K_e = 1.36$ 的腔：

| $\\varphi_s$ | 出口能量 (MeV) | 出口 rms $z$ (mm) |
|---|---|---|
| −60° | 1.694 | 2.15 |
| −30° | 1.884 | 1.12 |
| 0° | 2.002 | 2.43 |
| +20° | 1.995 | 5.05 |

$\\varphi_s = 0$ 附近增益最大（波峰），但束团被拉长；$\\varphi_s < 0$ 时增益略少，束团却被压缩——这就是 **相稳定性**：在 AVAS 的 $\\cos(\\omega t+\\varphi)$ 约定下，负的同步相位使晚到的粒子获得更多能量，从而追上参考粒子。正相位（$+20^\\circ$）一侧没有纵向聚焦，束团迅速变长。代价是 [射频散焦](#longitudinal)：纵向聚焦越强，横向散焦也越强，所以 $\\varphi_s = -60^\\circ$ 时横向包络最大。

**试一试**：在 $-30^\\circ$ 附近微调相位，观察出口能量和 rms $z$ 的取舍；然后用内核运行，比较能量（预览与内核通常差 0.01 % 量级）和 rms $z$（差别更大，因为低能腔中纵向非线性明显，线性预览不考虑纵向发射度增长）。

## 第 4 步：周期性加速段 {#tutorial-step4}

把「螺线管 + 漂移 + 腔 + 漂移」重复 $N$ 次，就是超导直线加速器最常见的结构：每个周期一个聚焦元件和一个加速腔。

\`\`\`text
! period k
field 0.35 0.02 0 3 0 0 0 Kb sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 phis Ke -Ke hwr010
drift 0.1 0.02 0
\`\`\`

周期长度约 0.76 m。预览（$K_b = 0.35$、$\\varphi_s = -30^\\circ$、$K_e = 1.36$）：1 个周期出口约 1.88 MeV，3 个周期约 2.96 MeV，6 个周期约 4.99 MeV。第一个周期约增益 0.36 MeV，之后每个周期增益逐渐变大（6 个周期平均约 0.58 MeV）：粒子速度升高后更接近这个腔适合的速度，渡越时间因子随之增大；横向最大 rms 约 1.8 mm，纵向 rms $z$ 在 3 mm 以内振荡，不再无限增长——横向和纵向都被周期性地约束住了。

几个要点（详见 [如何组成加速器](#lattice-design)）：

- 每个周期的横向相移 $\\sigma_{0t}$ 与纵向相移 $\\sigma_{0l}$ 都应小于 90°，并避开纵横运动之间的低阶共振（例如二者接近相等或成简单整数比，见 Hofmann 稳定性图）。
- 随着能量升高，$\\beta\\lambda$ 变长、$B\\rho$ 变大，同样的 $K_b$ 聚焦变弱、同样的腔渡越时间因子也在变化。真实设计会逐段调整 $K_b$、$\\varphi_s$ 甚至腔型；本教程为简单起见各周期用同一组参数。
- 入口束流并不匹配到这个周期结构，前一两个周期的包络起伏就是失配的表现，见 [匹配与失配](#matching)。

**试一试**：把周期数加到 8，看能量是否线性增长；把 $K_b$ 调到 0.25 或 0.45，看包络如何失控。

## 第 5 步：加入束流流强 {#tutorial-step5}

最后一步可以调节 \`beam.txt\` 中的 \`current\`。流强越大，[空间电荷](#space-charge) 的散焦越强，相当于减弱了每个周期的聚焦：

$$
K = \\frac{q I}{2\\pi \\varepsilon_0 m c^3 (\\beta\\gamma)^3}
$$

（连续束的广义导流系数；束团束用平均电荷近似。）预览中 3 个周期的最大 rms $x$：0.18 mA 时约 1.8 mm，5 mA 时约 2.8 mm，15 mA 时约 4.6 mm，30 mA 时约 6.7 mm；rms $z$ 也随之增大。低能（$\\beta\\gamma \\approx 0.057$）时 $(\\beta\\gamma)^3$ 很小，所以几个毫安就已经很明显。

这一步最适合用内核验证：内核用宏粒子和网格求解自场，能看到线性模型看不到的东西——发射度增长、晕和束流损失（「传输效率（内核）」小于 100 % 即有粒子打到孔径）。可以把「宏粒子数」加到 5000 以上减小统计噪声，代价是运行更慢，见 [多粒子模拟要点](#simulation)。

## 用内核运行与另存为项目 {#tutorial-engine}

「用内核运行」把当前这一步的 lattice（工具里「本步骤的 AVAS lattice」显示的文字）和修改过的 \`beam.txt\` 写进沙盒，用示例项目的 \`input.txt\`（空间电荷开启，FFT 求解）运行内核，读取 \`DataSet.txt\` 中的 rms 尺寸、能量和存活宏粒子数，与预览画在同一张图上。运行时间随周期数和宏粒子数增加；作为参考，示例项目（5260 个宏粒子、一个腔）在一般电脑上约需 10–30 秒。

比较时请注意：

- 能量曲线应几乎重合；若明显不同，多半是相位或场系数设置有误。
- rms 尺寸在低流强时应接近（百分之几以内）；流强大时差别变大，以内核为准。
- 如果参数改过而没有重新运行，工具会提示内核结果属于之前的参数。

「另存为新项目」在你选择的新文件夹里创建 \`InputFile/\`（\`beam.txt\`、\`input.txt\`、\`ini.ini\`、\`lattice_mulp.txt\` 以及用到的场图）和 \`OutputFile/\`，并直接打开它。之后就可以在结构页里继续编辑（加输出平面、误差、诊断……），用运行页运行、结果页分析。

## 下一步 {#tutorial-next}

- 想设计自己的聚焦周期：先读 [线性光学](#linear-optics) 和 [如何组成加速器](#lattice-design)，用 FODO 实验估计相移。
- 想研究容差：把教程项目另存后，按 [误差与校正](#errors) 加入腔的相位、幅度误差和螺线管错位，运行误差研究。
- 遇到不明白的现象，点页面上方的「问讲解员」，AI 助手会结合本入门内容和实验数值解释。
`,U={"avas-lab":`lab`,"avas-params":`params`,"avas-field":`field`,"avas-tutorial":`tutorial`};function W(e){return e in U}function G(e,t){let n=U[e];if(!n)return null;let r=t.trim().split(/\s+/).filter(Boolean),i={},a=[];for(let e of r.slice(1)){let t=e.indexOf(`=`);t>0?i[e.slice(0,t)]=e.slice(t+1):a.push(e)}return{kind:n,id:r[0]??``,args:i,items:a}}function K(e){return[e.id,...e.items].filter(Boolean).map(e=>{let[t,n]=e.includes(`:`)?e.split(`:`,2):[`lattice`,e];return{group:t,key:n}})}var q=/^(#{1,2}) (.+?)(?: \{#([\w-]+)\})?\s*$/;function J(e,t){let n={...e,title:e.id,intro:``,sections:[]},r=``,i=null;for(let a of t.replace(/^﻿/,``).split(/\r?\n/)){let t=a.match(/^\s*(`{3,}|~{3,})/);t&&(r?t[1][0]===r[0]&&t[1].length>=r.length&&(r=``):r=t[1]);let o=!r&&a.match(q);if(o&&o[1]===`#`&&n.title===e.id&&!i){n.title=o[2];continue}if(o&&o[1]===`##`){i={id:o[3]||`${e.id}-${n.sections.length+1}`,title:o[2],text:``},n.sections.push(i);continue}i?i.text+=a+`
`:n.intro+=a+`
`}return n}var Y=Object.assign({"./content/dipole.en.md":i,"./content/dipole.zh.md":a,"./content/drift.en.md":o,"./content/drift.zh.md":s,"./content/errors.en.md":c,"./content/errors.zh.md":l,"./content/field-maps.en.md":u,"./content/field-maps.zh.md":d,"./content/intro.en.md":f,"./content/intro.zh.md":p,"./content/kinematics.en.md":m,"./content/kinematics.zh.md":ee,"./content/lattice-design.en.md":te,"./content/lattice-design.zh.md":h,"./content/linear-optics.en.md":g,"./content/linear-optics.zh.md":_,"./content/longitudinal.en.md":v,"./content/longitudinal.zh.md":y,"./content/matching.en.md":b,"./content/matching.zh.md":x,"./content/phase-space.en.md":S,"./content/phase-space.zh.md":C,"./content/quadrupole.en.md":w,"./content/quadrupole.zh.md":T,"./content/references.en.md":E,"./content/references.zh.md":D,"./content/rf-cavity.en.md":O,"./content/rf-cavity.zh.md":k,"./content/simulation.en.md":A,"./content/simulation.zh.md":j,"./content/solenoid.en.md":M,"./content/solenoid.zh.md":N,"./content/space-charge.en.md":P,"./content/space-charge.zh.md":ne,"./content/static-fields.en.md":F,"./content/static-fields.zh.md":I,"./content/steerer.en.md":L,"./content/steerer.zh.md":R,"./content/superpose.en.md":z,"./content/superpose.zh.md":B,"./content/tutorial.en.md":V,"./content/tutorial.zh.md":H});function X(e,t){return Y[`./content/${e}.${t}.md`]}function Z(){return Object.keys(Y).map(e=>e.replace(`./content/`,``))}var Q=new Map;function re(e){let t=Q.get(e);return t||(t=r.filter(t=>X(t.id,e)??X(t.id,e===`zh`?`en`:`zh`)).map(t=>J(t,X(t.id,e)??X(t.id,e===`zh`?`en`:`zh`))),Q.set(e,t)),t}function ie(e,t){for(let n of e){if(n.id===t)return{chapter:n};let e=n.sections.find(e=>e.id===t);if(e)return{chapter:n,section:e}}return null}function ae(e,t,n=40){let r=t.trim().toLocaleLowerCase().split(/\s+/).filter(Boolean);if(!r.length)return[];let i=[];for(let t of e){let e=[{title:t.title,text:t.intro},...t.sections.map(e=>({section:e,title:e.title,text:e.text}))];for(let n of e){let e=n.title.toLocaleLowerCase(),a=n.text.toLocaleLowerCase();if(!r.every(n=>e.includes(n)||a.includes(n)||t.title.toLocaleLowerCase().includes(n)))continue;let o=r.reduce((t,n)=>t+(e.includes(n)?5:0)+(a.split(n).length-1),0);if(!o)continue;let s=Math.max(0,a.indexOf(r[0])),c=$(n.text.slice(Math.max(0,s-40),s+120));i.push({chapter:t,section:n.section,snippet:c,score:o})}}return i.sort((e,t)=>t.score-e.score).slice(0,n)}function $(e){return e.replace(/```[\s\S]*?```/g,` `).replace(/\$\$?([^$]*)\$\$?/g,`$1`).replace(/[*_`#>|]/g,``).replace(/\[([^\]]+)\]\([^)]*\)/g,`$1`).replace(/\s+/g,` `).trim()}function oe(e){let t=[],n=e.replace(/```[\s\S]*?```/g,``).replace(/`[^`\n]*`/g,``).replace(/\$\$([\s\S]+?)\$\$/g,(e,n)=>(t.push({tex:n.trim(),display:!0}),` `));for(let e of n.matchAll(/\$([^$\n]+)\$/g))t.push({tex:e[1],display:!1});return t}function se(e){return[...e.replace(/```[\s\S]*?```/g,``).matchAll(/\]\(#([\w-]+)\)/g)].map(e=>e[1])}function ce(e){let t=[];for(let n of e.matchAll(/^\s*```([\w-]+)[ \t]*\n([\s\S]*?)^\s*```\s*$/gm))if(W(n[1])){let e=G(n[1],n[2]);e&&t.push(e)}return t}export{r as CHAPTERS,t as PARTS,ce as chapterDirectives,n as chapterForKeyword,e as chapterOf,X as chapterSource,Z as contentFiles,se as linkTargets,re as loadChapters,ie as locate,oe as mathSnippets,G as n,J as parseChapter,$ as plain,ae as searchChapters,K as t};