# Dipole

A dipole's job looks like the simplest of all: turn the beam's direction by an angle with a uniform magnetic field. Yet after a dipole the beam is no longer the same in the bending plane as in the other: a uniform field has no gradient at all, but it focuses in the bending plane; and how much the beam grows depends on the energy spread of the particles. Both follow from one simple fact — every particle in the field moves on an arc of a circle.

Starting from those arcs, this chapter shows how strong a field is needed to bend the beam (the magnetic rigidity $B\rho$), why a uniform field has "weak focusing", why particles of different momentum follow different orbits (dispersion), and how tilted pole faces add extra focusing at the ends. It uses the matrix methods of [linear optics](#linear-optics) and the [quadrupole](#quadrupole) chapter, and $\delta = \Delta p/p$ from [longitudinal motion](#longitudinal); computing the rigidity is covered in [kinematics](#kinematics).

A 45°, $\rho = 1$ m sector dipole (with an edge of pole-face angle 0 on each side) followed by a 0.5 m drift; the 3D view shows the beam line turning. At the dipole exit ($z \approx 0.99$ m) the rms $x$ in the bending plane is 3.2 mm, smaller than the 3.7 mm of rms $y$: that is the weak focusing of the bending plane. At the end of the drift rms $x$ is 4.4 mm, of which about 2.2 mm (subtracting in quadrature) comes from dispersion — without momentum spread the rms $x$ there would be 3.8 mm.

```avas-beamline
drift 0.2 0.03 0
edge 0 0.03 0 0 1 0 0 0 0
bend 0 0.03 0 45 1 0 0
edge 0 0.03 0 0 1 0 0 0 0
drift 0.5 0.03 0
```

## Role and placement {#dipole-role}

In a linac the dipole is not the main actor, but beam transport lines, energy analysis, beam-dump branches and injection/extraction lines cannot do without it:

- **Changing the beam direction**: sending the beam to different end stations, targets or beam dumps (see [the HEBT](#journey-hebt)).
- **Momentum (energy) analysis**: using dispersion to separate particles of different momentum in space, with slits for energy selection or to measure the energy spread.
- **Geometry**: folding the beam line into the available space.
- **Longitudinal manipulation**: the $R_{56}$ of a bend can cancel or add to the $L/\gamma^2$ of drifts, for bunch compression or stretching (for low-energy protons $L/\gamma^2$ usually dominates).

## Magnetic rigidity: how strong a field bends the beam {#dipole-field}

### Circular motion

In a uniform magnetic field $B$ perpendicular to the velocity, the Lorentz force $q v B$ keeps its magnitude and stays perpendicular to the velocity: the particle moves on a circle at constant speed. It supplies the centripetal force $\gamma m v^2/\rho$: $q v B = \gamma m v^2/\rho$, so

$$
\rho = \frac{p}{qB} = \frac{B\rho}{B},\qquad
B\rho\,[\mathrm{T\,m}] = \frac{p\,[\mathrm{MeV}/c]}{299.792458\,Z}.
$$

$B\rho = p/q$ is the **magnetic rigidity**: it measures how "stiff" the beam is, the field that bends it on a circle of 1 m radius. The bending angle $\alpha$ and the arc length $L$ are related by $L = \rho\,\alpha$ ($\alpha$ in radians). Some numbers:

| Beam | $B\rho$ | $B$ for $\rho = 1$ m |
|---|---|---|
| protons 35 keV (LEBT) | 0.0270 T·m | 0.027 T |
| protons 1.527 MeV (example) | 0.1786 T·m | 0.179 T |
| protons 100 MeV | 1.483 T·m | 1.48 T |
| protons 1 GeV | 5.66 T·m | 5.66 T |

Normal-conducting iron dipoles usually stay below 1.5–2 T, so 1 GeV protons need either a bending radius of several metres or superconducting magnets; for the example beam, 0.18 T already gives a 1 m bending radius.

### Which way it bends

In AVAS's cross-section convention (viewed from downstream, the beam coming out of the screen, $x$ to the right, $y$ up), with $B_y > 0$ (up) a positive charge feels $-q v B_y\,\hat{x}$ and is bent to the left ($-x$); the centre of curvature is on the left. Below is the field in a dipole cross-section (**schematic**): with $n = 0$ the field is uniform across the bore. The `B` here only sets the sign; the actual field follows from $B\rho/\rho$, and the picture shows the field relative to its value on the axis.

```avas-field
dipole B=0.18 N=0 rho=1 L=0.785 R=0.03
```

### Field index

In general the field of a dipole can be expanded around the reference orbit as

$$
B_y(x) = B_0\left(1 - n\,\frac{x}{\rho} + \cdots\right),\qquad
n = -\frac{\rho}{B_0}\,\frac{\partial B_y}{\partial x}\bigg\rvert_{x=0},
$$

with $x$ pointing from the reference orbit away from the centre of curvature. $n$ is the **field index**, written `N` in AVAS. $n = 0$ is a uniform field (parallel pole faces); a dipole with $n \ne 0$ also has a quadrupole component (a combined-function magnet). By Maxwell's equation $\partial B_x/\partial y = \partial B_y/\partial x$, a horizontal gradient necessarily comes with $B_x = -n B_0\,y/\rho$: for $n > 0$ the field lines bulge outwards and particles that leave the median plane are pushed back to it.

With $n = 0.5$ the field changes linearly across the bending plane (**schematic**). At the edge of a 3 cm aperture the field is only 1.5 % weaker or stronger than on the axis — the field index is a very gentle gradient. The schematic draws both the variation of $B_y$ with $x$ and the $B_x$ proportional to $y$ described above; the direction of the gradient depends only on $n$, not on the bending direction (bending the other way flips $B_0$ and the "outside" together).

```avas-field
dipole B=0.18 N=0.5 rho=1 L=0.785 R=0.03
```

## Weak focusing in a uniform field {#dipole-focusing}

### Two circles of the same radius

In a uniform field all particles of the same momentum move on circles of the same radius. Let a particle enter parallel to the reference particle, displaced outwards by $x_0$: its circle is as large as the reference circle, only its centre is also displaced by $x_0$. After turning by $\theta$ the radial distance between the two circles is $x_0\cos\theta$: at $90^\circ$ the two orbits cross, at $180^\circ$ the offset has become $-x_0$. A uniform field is itself a lens in the bending plane, with $R_{11} = \cos\theta$ and focal length $f = \rho/\sin\theta$. Physically: the outer particle travels a longer arc in the field, is turned more, and comes back towards the reference orbit.

In the same way, a particle leaving the reference orbit at a small angle moves on a circle turned by a small angle about the entry point and is back on the reference orbit after $180^\circ$ ($R_{12} = \rho\sin\theta = 0$): a $180^\circ$ uniform-field dipole images a point onto a point, the principle of early magnetic spectrometers.

This focusing is weak: a 45°, $\rho = 1$ m sector dipole has a focal length $\rho/\sin 45^\circ = 1.41$ m, while the 0.1 m, 6 T/m quadrupole of the [quadrupole](#quadrupole) chapter has 0.3 m. The focusing strength $h^2 = 1/\rho^2 = (B/B\rho)^2$ is of second order in the field: only 1 m$^{-2}$ for the example beam in 0.18 T, against $k = 33.6$ m$^{-2}$ for that quadrupole. Vertically a uniform field does nothing at all; it acts like a drift.

### Equations of motion

In the Frenet–Serret frame that follows the curved reference orbit ($x$ pointing away from the centre of curvature, $h = 1/\rho$), the linear equations of motion are

$$
x'' + (1-n)\,h^2\,x = h\,\delta,\qquad
y'' + n\,h^2\,y = 0 .
$$

- The $h^2$ in $(1-n)h^2$ is the **geometric (weak) focusing** described above.
- $-n h^2$ and $+n h^2$ come from the quadrupole component and have opposite signs in the two planes (again $k_x + k_y = 0$, with the geometric term added to the horizontal plane on top).
- For $0 < n < 1$ both planes are focused (**weak focusing**), and for $n = 1/2$ both equally; over a full turn ($2\pi$) the numbers of oscillations are $\nu_x = \sqrt{1-n}$ and $\nu_y = \sqrt{n}$.
- The $h\,\delta$ on the right is the source of dispersion: a particle of higher momentum has a larger radius and moves outwards (next section).

> **Think about it**: a solenoid and a uniform dipole both have only a uniform field, yet both focus, with a strength proportional to $B^2$. Coincidence?
> Answer: No. For a particle parallel to the reference orbit, a uniform field gives a force independent of position, so the first-order focusing is zero; the focusing appears once the field has changed the trajectory (a different path length in the dipole, an azimuthal velocity in the solenoid) and acts on it a second time — hence second order. For the same field, the dipole's horizontal $h^2 = (B/B\rho)^2$ is exactly 4 times the solenoid's $k^2 = (B/2B\rho)^2$.

## Dispersion: different momentum, different orbit {#dipole-dispersion}

### Two tangent circles

Now let two particles enter at the same point in the same direction, one with the reference momentum $p$ and one with $p(1+\delta)$. The latter's circle has a radius larger by $\rho\delta$; the two circles touch at the entrance. After turning by $\theta$ the radial distance and the angle between the two orbits are

$$
\Delta x = \rho\,(1-\cos\theta)\,\delta,\qquad \Delta x' = \sin\theta\,\delta .
$$

The coefficients $D = \rho(1-\cos\theta)$ and $D' = \sin\theta$ are the **dispersion** at the dipole exit: the offset in position and angle per unit momentum deviation. For 45° and $\rho = 1$ m, $D = 0.293$ m: a particle with 1 % more momentum leaves 2.9 mm further out, three times the rms size of the example beam at its start (about 1 mm) and close to the 3.2 mm the beam has grown to at the dipole exit in the opening picture. At $90^\circ$, $D = 1$ m; at $180^\circ$, $D = 2\rho = 2$ m with $D' = 0$ — at $180^\circ$ a spectrometer both images and has its largest dispersion.

### The dispersion function

In general the **dispersion function** $D(s)$ is the particular orbit of a particle with $\delta = 1$; it obeys $D'' + (1-n)h^2 D = h$ and is transported by the downstream elements like an orbit:

$$
\begin{pmatrix} D \\ D' \\ 1 \end{pmatrix}_1 =
\begin{pmatrix} R_{11} & R_{12} & R_{16} \\ R_{21} & R_{22} & R_{26} \\ 0 & 0 & 1 \end{pmatrix}
\begin{pmatrix} D \\ D' \\ 1 \end{pmatrix}_0 .
$$

Coming from a dispersion-free straight section, the exit has $D = R_{16}$ and $D' = R_{26}$. After that the dispersion keeps growing as $D + D' s$ in drifts even without dipoles: at the end of the opening beam line (0.5 m after the exit) $D = 0.293 + 0.5\times 0.707 = 0.646$ m. Where there is dispersion the rms beam size is

$$
\sigma_x^2 = \beta_x\,\varepsilon_x + D^2\,\sigma_\delta^2 .
$$

The example beam has an rms momentum spread $\sigma_\delta = 3.5\times 10^{-3}$, so at the end of the opening beam line dispersion contributes $D\sigma_\delta = 2.3$ mm (subtracting in quadrature in the preview gives about 2.2 mm; the difference comes from the space-charge coupling in the preview).

A layout that makes $D$ and $D'$ vanish together at the exit of a bending section is called an **achromat**, for example two identical dipoles with a quadrupole between them that makes $D' = 0$ at the midpoint: by symmetry the dispersion at the exit is then zero like at the entrance.

### Path length

Dispersion also has a longitudinal side. Along a bend the longitudinal coordinate ($z > 0$ ahead of the reference particle) changes as

$$
z' = -h\,x + \frac{\delta}{\gamma^2}.
$$

The first term is the path difference: an outer particle travels further and falls behind; the second is the velocity difference: a particle of higher momentum is faster, as in any drift. For a 45° sector dipole $R_{56} = L/\gamma^2 - \rho(\theta - \sin\theta) = 0.783 - 0.078 = 0.705$ m: for low-energy protons, the longitudinal behaviour of a bend is still mainly the debunching of a drift (see [longitudinal motion](#longitudinal)).

### Try it

The dipole lab gives the sector-magnet matrix, $D$ and $D'$ at the exit, the orbits of off-momentum particles and the $x$, $y$ envelopes (the envelopes have no momentum spread):

1. Reproduce $R_{16} = 0.293$ m with `N=0`, `edge=0`; set `angle` to 90 and 180 to check $D = \rho(1-\cos\theta)$, and see that at $180^\circ$ $R_{21} = 0$ and $D' = 0$.
2. Set `edge` to 22.5 (a rectangular magnet, see below): the horizontal focusing is cancelled and the vertical envelope is held.
3. Set `N` to 0.5 and compare the envelopes of the two planes. The orbit plot always shows the two particles with $+\delta$ and $-\delta$ (`delta` is the magnitude, 0–10 %): they leave on opposite sides of the reference orbit, and doubling `delta` doubles both offsets — dispersion is linear in $\delta$.

```avas-lab
dipole angle=45 rho=1 N=0 edge=0 W=1.527 mass=938.272 q=1 delta=1
```

## Transfer matrix of a sector magnet {#dipole-matrix}

Solve the equations of motion above over a length $L$. Let $K_x = (1-n)h^2$, $K_y = n h^2$, and $C_x, S_x$ the cosine- and sine-like solutions of $u'' + K_x u = 0$ ($C_x = \cos\sqrt{K_x}L$, $S_x = \sin(\sqrt{K_x}L)/\sqrt{K_x}$, hyperbolic functions for $K < 0$). For bending in $x$ the $6\times 6$ matrix of a sector magnet is

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

This is `bend_matrix(L, angle, n_index, plane, gamma)` of AVAS's linear preview. $R_{51}$ and $R_{52}$ are tied to $R_{16}$ and $R_{26}$ by the symplectic condition; they describe how the transverse coordinates change the path length: a particle displaced outwards at the entrance ($x > 0$) travels further and its $z$ decreases.

For a uniform-field ($n = 0$) sector magnet, $\theta = L/\rho$:

$$
R_{16} = \rho\,(1-\cos\theta),\quad
R_{26} = \sin\theta,\quad
R_{56} = \frac{L}{\gamma^2} - \rho\,(\theta - \sin\theta),
$$

and vertically it is a drift of length $L$ — exactly what the two circles of the previous sections gave.

## Pole-face rotation and edge focusing {#dipole-edge}

### Turning the pole face

The field region of a sector magnet is a wedge: entrance and exit pole faces are perpendicular to the reference orbit, and outer orbits travel further in the field, which is the geometric focusing above. Turn the pole face by an angle $\beta$, and a particle at $x$ travels $x\tan\beta$ more or less in the field, so its bending angle changes by $h\,x\tan\beta$: a thin lens proportional to $x$ at the edge. In the TRANSPORT convention, $\beta > 0$ shortens the path of outer particles in the field and bends them less: the bending plane is **defocused**.

The other plane is affected too. At a tilted edge the fringe field falls off along the normal to the pole face, and by $\nabla\times\boldsymbol{B} = 0$ there is, at a height $y$ off the median plane, a horizontal component along that normal proportional to $y$. With a tilted pole face part of this horizontal component is perpendicular to the particle's velocity and gives a vertical force: for $\beta > 0$ it pushes the particle back to the median plane, and the other plane is **focused**. With a hard edge the two planes get equal and opposite effects: an edge is a thin quadrupole lens, once more $k_x + k_y = 0$.

### The edge matrix

In the TRANSPORT / TraceWin form:

$$
M_{\text{edge}}:\qquad
R_{21} = h\tan\beta,\qquad
R_{43} = -h\tan(\beta - \psi),
$$

all other elements as in the unit matrix. The correction for the finite extent of the fringe field is

$$
\psi = K_1\,G\,h\,\frac{1 + \sin^2\beta}{\cos\beta}\,\left(1 - K_1 K_2\,G\,h\tan\beta\right),
$$

where $G$ is the total (full) gap of the magnet and $K_1$, $K_2$ are integrals describing the shape of the fringe field. $K_1 = 0$ gives $\psi = 0$, the hard-edge model. This is `edge_matrix(beta, rho, gap, k1, k2, plane)` of the linear preview. A fringe field of finite extent weakens the vertical focusing: with $\beta = 22.5^\circ$, $\rho = 1$ m, $G = 0.05$ m, $K_1 = 0.45$, $K_2 = 2.8$, $\psi = 1.56^\circ$ and the vertical focusing of one edge drops from $\tan 22.5^\circ = 0.414$ m⁻¹ to 0.383 m⁻¹, about 8 % weaker.

### The rectangular magnet

The **rectangular magnet** is an important special case: the two pole faces are parallel to each other and each makes an angle $\theta/2$ with the orbit, i.e. a sector magnet with an edge of $\beta = \theta/2$ at each end. All particles travel roughly the same length in the field, the horizontal weak focusing is cancelled completely by the edges ($R_{21} = 0$), and the vertical plane gains focusing. A 45°, $\rho = 1$ m rectangular magnet (hard edges): horizontally $R_{11} = 1$, $R_{21} = 0$; vertically $R_{43} = -0.694$ m⁻¹; the exit dispersion is unchanged ($D = 0.293$ m) and $D' = 0.828$. With the fringe correction above, the vertical $R_{43} = -0.650$ m⁻¹.

Change the two edges of the opening beam line to 22.5° and compare: at the end of the drift rms $y$ drops from 5.2 mm (sector) to 3.9 mm while rms $x$ rises from 4.4 mm to 5.5 mm — the edges have moved focusing from the bending plane to the other plane.

```avas-beamline
drift 0.2 0.03 0
edge 0 0.03 0 22.5 1 0 0 0 0
bend 0 0.03 0 45 1 0 0
edge 0 0.03 0 22.5 1 0 0 0 0
drift 0.5 0.03 0
```

## Dynamic effects {#dipole-dynamics}

### Dispersion and energy-spread coupling

A dipole converts longitudinal momentum deviations into transverse positions. Every element downstream "sees" a beam size that includes $D\sigma_\delta$; if the dispersion is not cancelled at the exit of the bending section it propagates along the line and, wherever there is acceleration or space charge, causes transverse–longitudinal coupling and effective emittance growth.

### Space charge

In a dispersive region the space-charge force depends on the total beam size including dispersion and also changes the dispersion itself (the dispersion function is defocused by space charge too); at high current the achromat condition has to be re-tuned with current. AVAS's multi-particle tracking includes these effects; the linear preview only gives estimates at zero current or with linear space charge.

### Non-linearities

For large momentum deviations dispersion and path length have second-order terms ($T_{166}$, $T_{566}$, …), and the fringe fields and saturation of the magnet add sextupole components; the matrix model is first order only. When a dipole is described by a field map, the linear preview gives a warning (it does not model the dipole content of maps); the engine result is the reference.

## Common misconceptions {#dipole-misconceptions}

- **"A uniform field has no gradient, so it does not focus"**: the bending plane has the geometric focusing $h^2$, only much weaker than a quadrupole's; it is the vertical plane that really is not focused.
- **"Dispersion is something inside the dipole"**: dispersion is created in the dipole but keeps propagating and growing through the drifts and quadrupoles downstream until an achromat cancels it.
- **"The beam grew, so its emittance grew"**: the $D\sigma_\delta$ from dispersion is a correlation between momentum and position and can be undone by an achromat; it is not the emittance itself, though left uncancelled it acts like a larger effective emittance.
- **"The pole-face angle only affects the bending plane"**: an edge acts on both planes, with opposite signs.
- **"The bending radius is fixed by the magnet"**: $\rho = B\rho/B$ depends on the beam's momentum; the same magnet bends different energies and charge-to-mass ratios differently — which is exactly what momentum analysis uses.
- **"The $z$ in the interface is a straight-line distance"**: after a dipole, the $z$ shown in the interface is the path length along the reference orbit.

## In AVAS {#dipole-in-avas}

### Keywords

In AVAS a dipole consists of two keywords: the sector-magnet body `bend` and the pole-face edge `edge`.

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
- `alpha`: bending angle (°) in the plane chosen by HV.
- `rho`: radius of curvature of the reference orbit (m).
- `N`: field index $n$.
- `HV`: bending plane, 0 = horizontal ($x$), 1 = vertical ($y$). With HV = 1 the roles of $x$ and $y$ in the matrices above are swapped: the dispersion is in $y$, $K_y = (1-n)h^2$, $K_x = nh^2$.

`edge`:

- First and third parameters are reserved, write 0; `R` is the aperture.
- `beta`: pole-face rotation angle $\beta$ (°).
- `rho`: radius of curvature (m), the same as the neighbouring `bend`.
- `G`: total gap of the magnet (m).
- `K1`, `K2`: fringe-field factors; `K1 = 0` is a hard edge.
- `HV`: the same as the neighbouring `bend`.

> **AVAS convention**: the linear preview treats `bend` as a sector-magnet body without pole-face angles; the pole-face rotation is given by an `edge` before and after it (as in TraceWin's BEND/EDGE). Whether the engine's `bend` itself includes edge effects, and whether it must be paired with `edge`, is not described in the manual; the engine is the reference. `bend` and `edge` are both z-code matrix-model elements; `edge` has zero length.

> Note: in the linear preview a negative $\alpha$ flips the sign of $h$, and $R_{16}$ etc. flip with it: the beam bends to the other side. Whether a positive $\alpha$ bends towards $+x$ or $-x$ in the engine is not stated in the manual; check with a single-particle engine run when it matters.

### Example

A 45° rectangular dipole, $\rho = 1$ m, horizontal bend, 5 cm gap:

```text
drift 0.000001 0.03 0
edge 0 0.03 0 22.5 1.0 0.05 0.45 2.8 0
bend 0 0.03 0 45 1.0 0 0
edge 0 0.03 0 22.5 1.0 0.05 0.45 2.8 0
drift 0.3 0.03 0
```

$K_1 = 0.45$ and $K_2 = 2.8$ are typical values often quoted in the TraceWin documentation; the exact values depend on the pole shape and should be fitted to measurements or 3D field calculations.

### The z coordinate in bends

Without dipoles, $z$ in DataSet.txt is column 5 + column 33. With dipoles, the AVAS interface accumulates the path step by step through the bending steps (column 35 marks the bending steps) and keeps accumulating afterwards; the $z$ in the interface is therefore the **path length** $s$ along the reference orbit, not a straight-line distance in the laboratory frame (see AGENTS.md §6 and `dataset_stream.PathLength`).

### Typical values

| Quantity | Typical range (low-energy transport line) | Note |
|---|---|---|
| Field $B$ | 0.1–1.5 T | normal-conducting iron magnets; set by $B\rho$ and $\rho$ |
| Radius $\rho$ | 0.3–2 m | |
| Bending angle $\alpha$ | 10°–90° | large angles are often split over several magnets |
| Field index $n$ | 0; analysing magnets often 0.5 | 0 < n < 1 gives weak focusing in both planes |
| Pole-face angle $\beta$ | 0 up to the order of $\alpha/2$ | balances the focusing of the two planes |
| Gap $G$ | 0.03–0.1 m | |

### Numerical example: 45° sector magnet

Example beam ($\gamma = 1.00163$), $\rho = 1$ m, $\alpha = 45^\circ$, $L = 0.7854$ m, $B = 0.1786$ T:

| | $n = 0$ | $n = 0.5$ |
|---|---|---|
| $R_{11}$, $R_{21}$ | 0.7071, −0.7071 m⁻¹ | 0.8497, −0.3728 m⁻¹ |
| $R_{33}$, $R_{43}$ | 1, 0 | 0.8497, −0.3728 m⁻¹ |
| $R_{16}$ (exit $D$) | 0.2929 m | 0.3006 m |
| $R_{26}$ (exit $D'$) | 0.7071 | 0.7456 |
| $R_{56}$ | 0.7046 m | 0.7033 m |

Some observations:

- A 1 % momentum deviation gives a 2.9 mm offset at the exit, large compared with the example beam's initial rms size (about 1 mm). The example beam's rms momentum spread is $\sigma_\delta = 3.5\times 10^{-3}$, so at the exit dispersion contributes $D\sigma_\delta \approx 1.0$ mm; in the opening beam line the betatron part there is about 3.0 mm, so in quadrature dispersion adds only about 0.15 mm at the exit — its weight grows downstream with $D + D's$ (2.3 mm at the end of the drift).
- In $R_{56}$ the drift term $L/\gamma^2 = 0.783$ m is far larger than the bending term $\rho(\theta - \sin\theta) = 0.078$ m: for low-energy protons a bend still mostly debunches.
- With two hard edges of $\beta = 22.5^\circ$ (rectangular magnet), horizontally $R_{21} = 0$, $R_{11} = 1$, vertically $R_{43} = -0.694$ m⁻¹; the exit dispersion is unchanged ($D = 0.293$ m) and $D' = 0.828$.
- With the fringe correction ($G = 0.05$ m, $K_1 = 0.45$, $K_2 = 2.8$), $\psi = 1.56^\circ$ and the vertical focusing of one edge drops from $\tan 22.5^\circ = 0.414$ m⁻¹ to 0.383 m⁻¹, about 8 % weaker.
- With $n = 0.5$ the matrices of the two planes are identical: "both planes focused equally".

### Common problems

- Pole-face angles in the design but no `edge` written: the linear preview gets a pure sector magnet with no vertical focusing, and the beam may be much larger vertically than expected (how the engine treats the edges of `bend` is up to the engine, see the convention above).
- `rho` or `HV` of an `edge` not matching the `bend`.
- `arc` of `bend` written as an angle or in the wrong unit; writing 0 is simplest.
- Comparing positions in a bend in laboratory coordinates: the $z$ in the interface is the path length.

## Further reading {#dipole-reading}

- K. L. Brown, "A First- and Second-Order Matrix Theory for the Design of Beam Transport Systems and Charged Particle Spectrometers", SLAC-75 (1967 and later revisions): matrices of sector magnets, pole-face rotation and fringe-field correction (the TRANSPORT convention).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (bending magnets, dispersion function and achromats).
- S. Y. Lee, *Accelerator Physics*, 4th ed. (weak focusing, field index, dispersion).
- Related chapters: [quadrupole](#quadrupole), [longitudinal motion](#longitudinal), [lattice design](#lattice-design), [matching](#matching), [the HEBT](#journey-hebt); the full bibliography is in [references](#references).
