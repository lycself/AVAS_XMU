# Quadrupole

The quadrupole is the main transverse focusing element of an accelerator. Its magnetic field vanishes on the beam axis and grows linearly with the distance from it, so for the beam it acts as a "magnetic lens". Unlike an optical lens, a quadrupole that focuses in one transverse plane necessarily defocuses in the other; only by alternating focusing and defocusing quadrupoles (strong, alternating-gradient focusing) is net focusing obtained in both planes.

Before this chapter it helps to read [drift](#drift) (the basic form of transfer matrices) and [linear optics](#linear-optics) (Hill's equation, phase advance, stability).

## Role and placement {#quadrupole-role}

- **Transverse focusing**: in the medium- and high-energy parts of a linac, quadrupoles form FODO, doublet or triplet periods that keep the beam inside the aperture.
- **Matching**: at the junction of two structures (for example the medium-energy transport line, MEBT, after an RFQ, or transitions between different periods) several independently powered quadrupoles match the beam's Twiss parameters to the downstream periodic solution (see [matching](#matching)).
- **Beam shaping**: adjusting the size and shape of the beam spot in front of a target or beam dump.

Quadrupoles also work for low-energy beams, but they act asymmetrically on $x$ and $y$, and strong focusing needs alternation, which takes room; at very low energy or between superconducting cavities [solenoids](#solenoid) are often used instead.

## Physics and field {#quadrupole-field}

### Pole geometry and the ideal field

An ideal quadrupole has four poles, N–S–N–S alternating, placed at $\pm 45^\circ$ and $\pm 135^\circ$. In the bore, free of currents and iron, the field is the gradient of a magnetic scalar potential, $\boldsymbol{B} = -\nabla \Phi_m$, and $\Phi_m$ obeys Laplace's equation. The lowest quadrupole solution is

$$
\Phi_m = -G\,x\,y,
\qquad
B_x = G\,y,\qquad B_y = G\,x,
$$

where $G = \partial B_y/\partial x = \partial B_x/\partial y$ is the **field gradient** in T/m. The pole surfaces are the equipotentials $x y = \pm R^2/2$ — hyperbolas, with $R$ the radius of the inscribed circle (the aperture). Equivalently the vector potential $A_z = \tfrac{G}{2}(y^2 - x^2)$ gives the same field.

The field at the pole tip is $B_{\text{pole}} = G\,R$. Iron poles begin to saturate around 1–1.5 T, so for room-temperature quadrupoles $G R \lesssim 1$ T is a practical limit: the smaller the aperture, the higher the reachable gradient.

### Constraint from Maxwell's equations

In the bore $\nabla\cdot\boldsymbol{B} = 0$ and $\nabla\times\boldsymbol{B} = 0$. The $z$ component of the latter gives

$$
\frac{\partial B_y}{\partial x} = \frac{\partial B_x}{\partial y}.
$$

This is the origin of "focusing in one plane, defocusing in the other": the restoring force in $x$ comes from $\partial B_y/\partial x$, the force in $y$ from $\partial B_x/\partial y$; the two are equal but act with opposite signs (next subsection). In a source-free region no static magnetic field focuses both planes at once (a form of Earnshaw's theorem).

### Lorentz force

For a particle of charge $q$ moving along $+z$ with velocity approximately $v\hat{z}$, $\boldsymbol{F} = q\,\boldsymbol{v}\times\boldsymbol{B}$ gives

$$
F_x = -q v B_y = -q v G\,x,\qquad F_y = q v B_x = q v G\,y .
$$

For $qG > 0$ the force in $x$ points to the axis (focusing) and the force in $y$ away from it (defocusing); for $qG < 0$ the other way round. The force is proportional to the distance from the axis, so the lens is linear: its strength does not depend on the particle's transverse position.

### Fringe fields and effective length

In a real quadrupole the gradient does not drop to zero abruptly at the ends; it decays smoothly over a distance of about one aperture radius (the fringe field). In linear optics it is usually replaced by a "hard-edge" model with the central gradient $G_0$ and the **effective length**

$$
L_{\text{eff}} = \frac{1}{G_0}\int G(s)\,\dd s .
$$

The hard-edge model keeps the integrated focusing strength $\int G\,\dd s$ but neglects the higher-order (non-linear) effects of the fringe field, such as end effects cubic in the position. Higher multipoles (dodecapole, 20-pole …, from the pole shape and manufacturing errors) are not part of the matrix model either.

## Equations of motion and transfer matrix {#quadrupole-matrix}

### Focusing strength

Dividing the force by $p v$ gives the linear equations of motion with $s$ as the independent variable (Hill's equation with constant coefficient)

$$
x'' + k\,x = 0,\qquad y'' - k\,y = 0,\qquad
k = \frac{q\,G}{p} = \frac{G}{B\rho}.
$$

$B\rho = p/q$ is the magnetic rigidity, in practice $B\rho\,[\mathrm{T\,m}] = p\,[\mathrm{MeV}/c]/(299.792458\,Z)$. The unit of $k$ is m$^{-2}$.

### Thick-lens matrix

Let $\varphi = \sqrt{\lvert k\rvert}\,L$. For $k>0$ the $x$ plane focuses and the $y$ plane defocuses:

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

For $k<0$ the planes are swapped. The longitudinal part is that of a drift of the same length ($R_{56} = L/\gamma^2$). The full $6\times 6$ matrix consists of three diagonal blocks:

$$
M_{\text{quad}} = \operatorname{diag}\!\left(M_x,\; M_y,\; \begin{pmatrix} 1 & L/\gamma^2 \\ 0 & 1 \end{pmatrix}\right).
$$

This agrees with `quad_matrix(L, k, gamma)` of AVAS's linear envelope preview (with $k = G/B\rho$, positive values focusing in $x$).

### Thin-lens approximation

For $\varphi \ll 1$ the quadrupole is treated as a thin lens at its centre between two drifts of $L/2$; the lens matrix is

$$
M_{\text{thin}} =
\begin{pmatrix}
1 & 0\\
-1/f & 1
\end{pmatrix},
\qquad
\frac{1}{f} = k\,L = \frac{G L}{B\rho}.
$$

The focal length is $+f$ in $x$ and $-f$ in $y$. The real thick-lens focal length ($-1/M_{21}$) differs by $\mathcal{O}(\varphi^2)$: in the focusing plane $f_x = 1/(\sqrt{k}\sin\varphi)$ is slightly longer, in the defocusing plane $\lvert f_y \rvert = 1/(\sqrt{k}\sinh\varphi)$ slightly shorter.

### Doublets and triplets

Two thin lenses of focal lengths $+f$ and $-f$ a distance $d$ apart have the combined focal length

$$
\frac{1}{F} = \frac{1}{f_1} + \frac{1}{f_2} - \frac{d}{f_1 f_2} = \frac{d}{f^2},
$$

positive in both planes — the heart of **alternating-gradient focusing**: the beam is larger in the focusing lens and smaller in the defocusing lens, and the net effect is focusing. The principal planes of a doublet differ between the two planes, so the beam leaving it is in general not round; a symmetric triplet ($-f/2$, $+f$, $-f/2$ or the reverse) gives similar imaging in both planes and is used where a round spot is needed.

### FODO period

A period of a focusing quadrupole QF, a drift, a defocusing quadrupole QD and a drift is called FODO. For thin lenses whose centres are $L_h$ apart, the phase advance per period $\sigma_0$ satisfies

$$
\sin\frac{\sigma_0}{2} = \frac{L_h}{2 f}.
$$

Periodic motion is stable when the one-period matrix satisfies $\lvert \Tr M \rvert/2 < 1$, for thin lenses $f > L_h/2$. For thick quadrupoles multiply the thick-lens matrices to get $M$ and take the phase advance from $\cos\sigma_0 = \Tr M/2$.

## In AVAS {#quadrupole-avas}

### Matrix-model quadrupole

```text
quad L R 0 G
```

```avas-params
quad
```

- `L`: length (m), the effective length of the hard-edge model.
- `R`: aperture radius (m), used for loss detection.
- The third parameter is reserved; write 0.
- `G`: field gradient (T/m).

> AVAS convention: $k = G/(B\rho)$; **$G > 0$ focuses a positively charged particle in $x$ and defocuses it in $y$** (checked with `quad_matrix` of the linear preview: for $G = 10$ T/m and $L = 0.1$ m, $M_{21} = -5.09\ \mathrm{m^{-1}}$, i.e. focusing in $x$). For negatively charged particles (for example H⁻) the same $G$ swaps the focusing plane. In a FODO, QF takes a positive and QD a negative $G$.

> AVAS convention: `quad` is a **z-code matrix-model element**; it acts as a whole through its transfer matrix and no field distribution enters the particle tracking. The manual asks that the first and the last element of a multi-particle simulation not be matrix-model elements; put a very short drift at both ends (see [drift](#drift-avas)). How space charge is handled inside matrix-model elements is not described in the manual; the kernel is the reference.

Example: one FODO period (example beam, numbers below):

```text
quad  0.1 0.02 0  6
drift 0.2 0.02 0
quad  0.1 0.02 0 -6
drift 0.2 0.02 0
```

### Field-map quadrupole

When fringe fields or a measured field matter, use a static magnetic field-map element (`field`, type 3) instead:

```text
field 0.1 0.02 0 3 0 0 0 1.0 qmap
```

The field-map files are `qmap.bsx`, `qmap.bsy` (and optionally `qmap.bsz`), the field is in T, and the kernel scales it by `Kb`: actual field = map × `Kb`. **The sign of `Kb` sets the polarity**: changing the sign of `Kb` swaps the focusing and defocusing planes. Parameters a static magnetic field does not use (frequency, phase, `Ke`) are written as 0, as the manual says. Field-map elements are tracked with the t-code and naturally include the fringe field and the higher components contained in the map. Storage order and reading of maps are covered in the [field maps](#field-maps) chapter.

### Errors

Position, rotation and gradient errors of quadrupoles are given with the "static-field element error" commands (acting on the N elements below the command):

```avas-params
err_quad_ncpl_stat
```

`dx`, `dy` are transverse offsets (mm), `dφx`, `dφy`, `dφz` rotations about the three axes (°), `dG` the relative gradient error (%), `dz` the longitudinal offset (mm). The full error-study workflow is in [errors and correction](#errors).

## Parameters and typical values {#quadrupole-params}

| Quantity | Typical range (low/medium-energy proton linac) | Remark |
|---|---|---|
| gradient $G$ | a few T/m to a few tens of T/m | room-temperature electromagnets; permanent-magnet quadrupoles can go higher |
| effective length $L$ | 0.05–0.3 m | short and strong at low energy, longer at high energy |
| aperture radius $R$ | 0.01–0.04 m | set by beam size, errors and loss budget |
| pole-tip field $G R$ | $\lesssim 1$ T (iron) | above it the iron saturates and the field quality degrades |
| phase advance per period $\sigma_0$ | 40°–90° (zero current) | avoid parametric resonances above 90°, see [space charge](#space-charge) |

### Worked example

Example beam: protons, $W = 1.527$ MeV, $p = 53.55$ MeV/$c$, $B\rho = 0.17863$ T·m.

| $G$ | $L$ | $k$ | $\varphi$ | thin-lens $f$ | thick $f_x$ | thick $f_y$ |
|---|---|---|---|---|---|---|
| 4 T/m | 0.1 m | 22.39 m⁻² | 0.473 | 0.447 m | 0.464 m | −0.430 m |
| 6 T/m | 0.1 m | 33.59 m⁻² | 0.580 | 0.298 m | 0.315 m | −0.282 m |
| 10 T/m | 0.1 m | 55.98 m⁻² | 0.748 | 0.179 m | 0.196 m | −0.163 m |
| 20 T/m | 0.05 m | 111.96 m⁻² | 0.529 | 0.179 m | 0.187 m | −0.171 m |

For low-energy protons a few T/m already make a strong lens; when $\varphi$ approaches 1 the thin-lens approximation is off by about 10 %.

FODO period with $L_q = 0.1$ m, $L_d = 0.2$ m (lens centres $L_h = 0.3$ m apart):

| $G$ | thick-lens phase advance $\sigma_0$ | thin-lens estimate | $\Tr M/2$ |
|---|---|---|---|
| 6 T/m | 52.7° | 60.5° | 0.606 |
| 10 T/m | 95.4° | 114.2° | −0.094 |
| 10 T/m, $L_d = 0.3$ m | unstable | unstable | −1.083 |

For $G = 6$ T/m the periodic $\beta_x$ varies between 0.43 and 1.06 m, which for the example emittance ($\varepsilon_{n,x} = 0.1377\ \pi$·mm·mrad) means rms sizes of 1.0–1.6 mm. Note that at $\varphi \approx 0.6$ the thin-lens estimate is already 8° off, and more for stronger focusing; use thick-lens matrices for design.

## Dynamics and common problems {#quadrupole-dynamics}

### The "breathing" envelope

In a FODO period $\beta_x$ is largest at QF and smallest at QD, $\beta_y$ the other way round; the two are half a period out of phase. The beam cross-section therefore flattens alternately in the two planes — the signature of quadrupole focusing. The beam is round only where the two cross.

### Chromatic effects

The focusing strength is $k \propto 1/p$; a particle with momentum deviation $\delta$ sees $k(1-\delta)$ (to first order). In dispersion-free straight sections the effect on the rms size is usually small, but with a large energy spread and strong focusing it causes effective emittance growth. AVAS's linear preview does not include chromatic effects (see the notes in `linear_optics.py`); the kernel is the reference.

### Misalignment and rotation

- A transverse offset $d_x$ of the quadrupole puts the beam centre at $-d_x$ relative to the magnetic centre; it receives a deflection $\Delta x' = k L\,d_x$ (thin lens), a dipole "kick". This is the main source of orbit distortion and is corrected with [steerers](#steerer).
- A rotation $\theta$ about $z$ creates a skew-quadrupole component (of relative strength about $\sin 2\theta$) that couples $x$ and $y$ and makes the projected emittances grow.
- A gradient error $\Delta G/G$ changes the phase advance and causes mismatch and beta beating.

### Space charge

At high current, space charge defocuses in both planes and lowers the phase advance per period from $\sigma_0$ to $\sigma$ ("tune depression"). In a quadrupole channel the beam flattens alternately in the two planes, and the space-charge force varies with it; to avoid envelope instabilities and parametric resonances the zero-current phase advance is usually kept below $90^\circ$. The lab in the [space charge](#space-charge) chapter shows this effect.

### Common problems

- Wrong gradient sign: the intended FODO becomes "FFOO", and the beam diverges quickly in one plane and is lost.
- Copying TraceWin's mm units: length and aperture of TraceWin's `QUAD` are in mm, AVAS uses m.
- Putting a matrix-model quadrupole first or last in the lattice (see the AVAS convention above).
- When comparing matrix-model and field-map results, remember that the map includes the fringe field and its effective length is not necessarily the map length: make $\int G\,\dd s$ equal before comparing.

## Hands-on {#quadrupole-lab}

First look at the field and the force on the quadrupole cross-section (a **schematic** computed from the parameters, not field-map data). AVAS always draws cross-sections looking from downstream to upstream with the beam coming out of the screen (⊙), so $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$ can be checked directly: for $G > 0$ the force on the $x$ axis points to the centre, the force on the $y$ axis away from it.

```avas-field
quadrupole G=6 L=0.1 R=0.02
```

The FODO lab below uses thick-lens matrices. First reproduce the 52.7° of the table with $G = 6$ T/m and $L_d = 0.2$ m; then:

1. Increase `G` step by step, watch the phase advance and $\lvert\Tr M\rvert/2$, and find the edge of the stable region (near $\sigma_0 = 180^\circ$).
2. Increase `Ld` to 0.3 m: at $G = 10$ T/m the period is no longer stable and the envelope blows up.
3. Set `mismatch` to 1.3 and watch the beta beating of the mismatched beam; then add a current `I` and compare the envelope with and without space charge.

```avas-lab
fodo Lq=0.1 G=6 Ld=0.2 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
```

## Further reading {#quadrupole-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed. (chapter 7: quadrupole focusing, FODO and periodic solutions).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (multipole expansion, quadrupole design and fringe fields).
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific (2019) (Hill's equation, strong focusing and stability).
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1–48 (the classic paper on alternating-gradient focusing).
- Related chapters: [linear optics](#linear-optics), [solenoid](#solenoid), [matching](#matching), [lattice design](#lattice-design), [errors and correction](#errors); the full bibliography is in [references](#references).
