# Dipole

A dipole (bending magnet) bends the central orbit of the beam with a uniform transverse magnetic field. In a linac it is not the main actor, but it is indispensable in transport lines, energy analysis, branch lines to beam dumps and injection/extraction lines. A dipole brings two effects absent in straight sections: **weak focusing** (focusing from the curvature of the orbit itself) and **dispersion** (particles of different momentum follow different orbits); tilted pole faces add focusing or defocusing at both ends.

In AVAS a dipole consists of two keywords: the sector-magnet body `bend` and the pole-face edge `edge`. This chapter uses the matrix methods of [linear optics](#linear-optics) and the [quadrupole](#quadrupole) chapter, and $\delta = \Delta p/p$ from [longitudinal motion](#longitudinal).

## Role and placement {#dipole-role}

- **Changing the beam direction**: sending the beam to different end stations, targets or beam dumps.
- **Momentum (energy) analysis**: dispersion separates particles of different momentum in space; together with slits this selects energy or measures the energy spread.
- **Geometric layout**: folding the beam line into the available space.
- **Longitudinal manipulation**: the $R_{56}$ of a bending section can add to or cancel the $L/\gamma^2$ of drifts, for bunch compression or stretching (for low-energy protons $L/\gamma^2$ usually dominates).

## Physics and field {#dipole-field}

### Bending radius

In a uniform field $B$ a particle of momentum $p$ and charge $q$ moves on a circle in the plane perpendicular to the field, with radius

$$
\rho = \frac{p}{qB} = \frac{B\rho}{B},\qquad
B\rho\,[\mathrm{T\,m}] = \frac{p\,[\mathrm{MeV}/c]}{299.792458\,Z}.
$$

Bending angle $\alpha$ and arc length $L$ are related by $L = \rho\,\alpha$ ($\alpha$ in radians). For the example beam (protons at 1.527 MeV) $B\rho = 0.17863$ T·m, so $\rho = 1$ m needs $B = 0.179$ T.

### Field index

A general dipole field can be expanded around the central orbit as

$$
B_y(x) = B_0\left(1 - n\,\frac{x}{\rho} + \cdots\right),\qquad
n = -\frac{\rho}{B_0}\,\frac{\partial B_y}{\partial x}\bigg\rvert_{x=0},
$$

where $n$ is the **field index**, called `N` in AVAS. $n = 0$ is a uniform field (parallel pole faces); a dipole with $n \ne 0$ also carries a quadrupole component (a combined-function magnet), and by Maxwell's equations the horizontal gradient comes with an equal $\partial B_x/\partial y$.

### Equations of motion

In the Frenet–Serret frame that follows the curved central orbit ($x$ pointing away from the centre of curvature, $h = 1/\rho$), the linear equations of motion are

$$
x'' + (1-n)\,h^2\,x = h\,\delta,\qquad
y'' + n\,h^2\,y = 0 .
$$

- The $h^2$ in $(1-n)h^2$ is **geometric (weak) focusing**: particles outside the central orbit travel a longer arc, those inside a shorter one, and both return towards the central orbit.
- $-n h^2$ and $+n h^2$ come from the quadrupole component, with opposite signs in the two planes.
- For $0 < n < 1$ both planes focus (**weak focusing**); for $n = 1/2$ the two planes focus equally. Over one full turn ($2\pi$) the numbers of oscillations are $\nu_x = \sqrt{1-n}$ and $\nu_y = \sqrt{n}$.
- The term $h\,\delta$ on the right is the source of dispersion: particles with higher momentum have a larger radius and move outwards.

The longitudinal coordinate ($z > 0$ ahead of the reference particle) changes at the rate

$$
z' = -h\,x + \frac{\delta}{\gamma^2};
$$

the first term is the path-length difference: an outer particle travels a longer path and falls behind.

## Equations of motion and transfer matrix {#dipole-matrix}

### Sector-magnet matrix

Let $K_x = (1-n)h^2$, $K_y = n h^2$, and $C_x, S_x$ the cosine- and sine-like solutions of $u'' + K_x u = 0$ ($C_x = \cos\sqrt{K_x}L$, $S_x = \sin(\sqrt{K_x}L)/\sqrt{K_x}$, hyperbolic functions for $K < 0$). For bending in $x$, the $6\times 6$ matrix of a sector magnet is

$$
M_{\text{bend}} =
\begin{pmatrix}
C_x & S_x & 0 & 0 & 0 & R_{16}\\
-K_x S_x & C_x & 0 & 0 & 0 & R_{26}\\
0 & 0 & C_y & S_y & 0 & 0\\
0 & 0 & -K_y S_y & C_y & 0 & 0\\
R_{51} & R_{52} & 0 & 0 & 1 & R_{56}\\
0 & 0 & 0 & 0 & 0 & 1
\end{pmatrix},
$$

with

$$
\begin{aligned}
R_{16} &= \frac{h\,(1-C_x)}{K_x}, &
R_{26} &= h\,S_x,\\
R_{51} &= -h\,S_x, &
R_{52} &= -\frac{h\,(1-C_x)}{K_x},\\
R_{56} &= \frac{L}{\gamma^2} - \frac{h^2\,(L - S_x)}{K_x}.
\end{aligned}
$$

This agrees with `bend_matrix(L, angle, n_index, plane, gamma)` of AVAS's linear preview. $R_{51}$ and $R_{52}$ are tied to $R_{16}$ and $R_{26}$ by symplecticity; they describe how the transverse coordinates change the path length.

For a uniform-field sector magnet ($n = 0$), with $\theta = L/\rho$:

$$
R_{16} = \rho\,(1-\cos\theta),\quad
R_{26} = \sin\theta,\quad
R_{56} = \frac{L}{\gamma^2} - \rho\,(\theta - \sin\theta),
$$

and the vertical plane is a drift of length $L$.

### Dispersion function

The dispersion function $D(s)$ is the (particular-solution) orbit of a particle with momentum deviation $\delta = 1$; it obeys $D'' + K_x D = h$. Through a beam line section,

$$
\begin{pmatrix} D \\ D' \\ 1 \end{pmatrix}_1 =
\begin{pmatrix} R_{11} & R_{12} & R_{16} \\ R_{21} & R_{22} & R_{26} \\ 0 & 0 & 1 \end{pmatrix}
\begin{pmatrix} D \\ D' \\ 1 \end{pmatrix}_0 .
$$

Coming from a dispersion-free straight section, the exit values are $D = R_{16}$, $D' = R_{26}$. Where there is dispersion the rms beam size is

$$
\sigma_x^2 = \beta_x\,\varepsilon_x + D^2\,\sigma_\delta^2 .
$$

A layout in which $D$ and $D'$ both vanish at the exit of the bending section is an **achromat**, for example two identical dipoles with a quadrupole between them that makes $D' = 0$ at the midpoint.

### Edges (pole-face rotation)

When a pole face is not perpendicular to the central orbit but rotated by an angle $\beta$, the length of field a particle crosses at the entrance or exit varies linearly with $x$, which acts as a thin lens at the edge. In the TRANSPORT / TraceWin form:

$$
M_{\text{edge}}:\qquad
R_{21} = h\tan\beta,\qquad
R_{43} = -h\tan(\beta - \psi),
$$

all other elements as in the unit matrix. $\beta > 0$ **defocuses** in the bending plane and **focuses** in the other. The correction for the finite extent of the fringe field is

$$
\psi = K_1\,G\,h\,\frac{1 + \sin^2\beta}{\cos\beta}\,\left(1 - K_1 K_2\,G\,h\tan\beta\right),
$$

where $G$ is the total (full) gap of the magnet and $K_1$, $K_2$ are integral factors describing the shape of the fringe field. $K_1 = 0$ gives $\psi = 0$, the hard-edge model. This agrees with `edge_matrix(beta, rho, gap, k1, k2, plane)` of the linear preview.

The **rectangular magnet** is an important special case: both pole faces are perpendicular to the incoming orbit, which amounts to edges of $\beta = \theta/2$ at both ends of a sector magnet. The horizontal weak focusing is exactly cancelled by the edges ($R_{21} = 0$) and the vertical plane gains focusing.

## In AVAS {#dipole-avas}

### Keywords

```text
edge 0 R 0 beta rho G K1 K2 HV
bend arc R 0 alpha rho N HV
edge 0 R 0 beta rho G K1 K2 HV
```

```avas-params
bend edge
```

`bend`:

- `arc`: arc length $\lvert\alpha\rho\rvert$ (m); write 0 and the program computes it from $\alpha$ and $\rho$.
- `R`: aperture radius (m); the third parameter is reserved, write 0.
- `alpha`: bending angle (°) in the plane selected by HV.
- `rho`: radius of curvature of the central orbit (m).
- `N`: field index $n$.
- `HV`: bending plane, 0 = horizontal ($x$), 1 = vertical ($y$). With HV = 1 the roles of $x$ and $y$ in the matrix above are swapped: dispersion appears in the $y$ plane, $K_y = (1-n)h^2$, $K_x = nh^2$.

`edge`:

- The first and third parameters are reserved, write 0; `R` is the aperture.
- `beta`: pole-face rotation angle $\beta$ (°).
- `rho`: radius of curvature (m), the same as that of the adjacent `bend`.
- `G`: total gap of the magnet (m).
- `K1`, `K2`: fringe-field factors; `K1 = 0` is a hard edge.
- `HV`: the same as the adjacent `bend`.

> AVAS convention: the linear preview treats `bend` as a sector-magnet body without pole-face angles, the pole-face rotations being given by one `edge` before and one after it (the same habit as TraceWin's BEND/EDGE). Whether the kernel's `bend` has edge effects of its own, or needs `edge` lines, is not stated in the manual; the kernel is the reference. `bend` and `edge` are z-code matrix-model elements; `edge` has zero length.

> Note: in the linear preview a negative $\alpha$ changes the sign of $h$, and of $R_{16}$ etc. with it, i.e. it bends to the other side. Whether a positive $\alpha$ bends towards $+x$ or $-x$ in the kernel is not stated in the manual; when it matters, confirm it with a single-particle kernel run.

### Example

A 45° rectangular dipole, $\rho = 1$ m, horizontal bending, 5 cm gap:

```text
drift 0.000001 0.03 0
edge 0 0.03 0 22.5 1.0 0.05 0.45 2.8 0
bend 0 0.03 0 45 1.0 0 0
edge 0 0.03 0 22.5 1.0 0.05 0.45 2.8 0
drift 0.3 0.03 0
```

$K_1 = 0.45$ and $K_2 = 2.8$ are typical values often quoted in the TraceWin documentation; the exact values depend on the pole shape and should be fitted to measurements or 3D field calculations.

### The z coordinate in bends

Without dipoles, $z$ in DataSet.txt is column 5 + column 33. With dipoles, the AVAS interface adds up the step lengths in bending steps (flagged in column 35) and continues adding afterwards; the $z$ shown is therefore the **path length** $s$ along the central orbit, not a straight-line distance in the laboratory frame (see AGENTS.md §6 and `dataset_stream.PathLength`).

## Parameters and typical values {#dipole-params}

| Quantity | Typical range (low-energy transport lines) | Remark |
|---|---|---|
| field $B$ | 0.1–1.5 T | room-temperature iron magnets; set by $B\rho$ and $\rho$ |
| radius $\rho$ | 0.3–2 m | |
| bending angle $\alpha$ | 10°–90° | large angles are often split into several magnets |
| field index $n$ | 0; 0.5 common for analysing magnets | 0 < n < 1 gives weak focusing in both planes |
| pole-face angle $\beta$ | 0 up to the order of $\alpha/2$ | used to balance the focusing of the two planes |
| gap $G$ | 0.03–0.1 m | |

### Worked example: a 45° sector magnet

Example beam ($\gamma = 1.00163$), $\rho = 1$ m, $\alpha = 45^\circ$, $L = 0.7854$ m, $B = 0.1786$ T:

| | $n = 0$ | $n = 0.5$ |
|---|---|---|
| $R_{11}$, $R_{21}$ | 0.7071, −0.7071 m⁻¹ | 0.8497, −0.3728 m⁻¹ |
| $R_{33}$, $R_{43}$ | 1, 0 | 0.8497, −0.3728 m⁻¹ |
| $R_{16}$ (exit $D$) | 0.2929 m | 0.3006 m |
| $R_{26}$ (exit $D'$) | 0.7071 | 0.7456 |
| $R_{56}$ | 0.7046 m | 0.7033 m |

Some observations:

- A 1 % momentum deviation gives a 2.9 mm offset at the exit, large compared with the rms beam size (about 1 mm); the example beam has an rms momentum spread $\sigma_\delta = 3.5\times 10^{-3}$, so dispersion adds $D\sigma_\delta \approx 1.0$ mm to the rms size, as much as the emittance does.
- In $R_{56}$ the drift term $L/\gamma^2 = 0.783$ m is much larger than the bending term $\rho(\theta - \sin\theta) = 0.078$ m: for low-energy protons the longitudinal behaviour of a bend is still mainly debunching.
- With two hard edges of $\beta = 22.5^\circ$ (a rectangular magnet), horizontally $R_{21} = 0$, $R_{11} = 1$, vertically $R_{43} = -0.694$ m⁻¹; the exit dispersion is unchanged ($D = 0.293$ m) and $D' = 0.828$.
- With the fringe-field correction ($G = 0.05$ m, $K_1 = 0.45$, $K_2 = 2.8$), $\psi = 1.56^\circ$, and the vertical focusing of one edge drops from $\tan 22.5^\circ = 0.414$ m⁻¹ to 0.383 m⁻¹, about 8 % weaker.

## Dynamics and common problems {#dipole-dynamics}

### Dispersion and energy-spread coupling

A dipole converts the longitudinal momentum deviation into a transverse position. Every downstream element "sees" a beam size that includes $D\sigma_\delta$; if the dispersion is not cancelled at the exit of the bending section it propagates along the line and, where there is acceleration or space charge, causes transverse–longitudinal coupling and effective emittance growth.

### Space charge

In dispersive regions the space-charge force depends on the total beam size including dispersion, and it also changes the dispersion itself (the dispersion function is defocused by space charge too); at high current the achromat conditions must be re-tuned with current. The kernel's multi-particle tracking includes these effects; the linear preview only gives estimates at zero current or with linear space charge.

### Non-linearities

At large momentum deviations dispersion and path length have second-order terms ($T_{166}$, $T_{566}$, …), and fringe fields and saturation add sextupole components; the matrix model is first order only. When a dipole is described by a field map, the linear preview gives a warning (dipole content of maps is not modelled); the kernel is the reference.

### Common problems

- Pole-face angles in the design but no `edge` lines: the linear preview gives a pure sector magnet without vertical focusing, and the beam may become much larger vertically than expected (how the kernel treats the edges of `bend` is defined by the kernel, see the convention above).
- `rho` or `HV` of the `edge` not matching the `bend`.
- `arc` of `bend` written as an angle or in wrong units; writing 0 is simplest.
- Comparing positions in bends with laboratory coordinates: the $z$ in the interface is the path length.

## Hands-on {#dipole-lab}

The field on the dipole cross-section (**schematic**): for $n = 0$ the field in the bore is uniform; with $n = 0.5$ it varies linearly across the bending plane, i.e. it carries a quadrupole component. `B` here is only a relative size; the actual field is $B\rho/\rho$.

```avas-field
dipole B=0.18 N=0 rho=1 L=0.785 R=0.03
```

```avas-field
dipole B=0.18 N=0.5 rho=1 L=0.785 R=0.03
```

The dipole lab gives the sector-magnet matrix, $D$ and $D'$ at the exit, the off-momentum trajectory and the $x$, $y$ envelopes:

1. Reproduce $R_{16} = 0.293$ m of the table with `N=0`, `edge=0`.
2. Set `edge` to 22.5 (a rectangular magnet): the horizontal focusing is cancelled and the vertical envelope is held.
3. Set `N` to 0.5 and compare the envelopes of the two planes; change `delta` from 1 to −1 and watch the off-momentum orbit move to the other side.

```avas-lab
dipole angle=45 rho=1 N=0 edge=0 W=1.527 mass=938.272 q=1 delta=1
```

## Further reading {#dipole-reading}

- K. L. Brown, "A First- and Second-Order Matrix Theory for the Design of Beam Transport Systems and Charged Particle Spectrometers", SLAC-75 (1967, later revised): matrices of sector magnets, pole-face rotation and fringe-field correction (the TRANSPORT conventions).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (bending magnets, dispersion function and achromats).
- S. Y. Lee, *Accelerator Physics*, 4th ed. (weak focusing, field index, dispersion).
- Related chapters: [quadrupole](#quadrupole), [longitudinal motion](#longitudinal), [lattice design](#lattice-design), [matching](#matching); the full bibliography is in [references](#references).
