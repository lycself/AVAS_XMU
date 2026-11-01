# Drift

A drift is the simplest element: a stretch of vacuum pipe without any applied electromagnetic field. It is also the most frequent line in a lattice — the gaps between elements, the space left for diagnostics and pumps, the transitions between cavities are all drifts. Precisely because it "does nothing" it is the reference for every other element: the transfer matrix of any element can be read as "drift plus the action of the field".

This chapter assumes the reader knows $\beta$, $\gamma$, $B\rho$ from [kinematics](#kinematics) and the Twiss parameters and rms emittance from [phase space](#phase-space).

## Role and placement {#drift-role}

What drifts do in a linac:

- **Geometric spacing**: quadrupoles, solenoids and cavities need room for flanges, bellows, vacuum valves and beam diagnostics (BPMs, current transformers, profile monitors). For the beam these spaces are drifts.
- **Phase adjustment**: between two RF cavities the drift length sets the RF phase at which the bunch reaches the next cavity (see [longitudinal motion](#longitudinal)).
- **Free evolution**: transversely a diverging beam grows linearly in a drift, a converging one first shrinks to a waist and then grows; longitudinally a bunch with momentum spread lengthens (debunching).
- **Loss detection**: the aperture radius `R` of a drift decides whether a particle "hits the wall", so drifts are one of the main places where AVAS detects beam loss.

In AVAS drifts also have a "technical" use: matrix-model elements should not be the first or last element of a multi-particle lattice, so a very short drift is often inserted at both ends (see [In AVAS](#drift-avas)).

## Physics: motion in field-free space {#drift-physics}

### Assumptions

- No external fields: $\boldsymbol{E} = 0$, $\boldsymbol{B} = 0$.
- Space charge (the beam's self-field) is ignored for now; its role in drifts is discussed in [the dynamics section](#drift-dynamics) and in the [space charge](#space-charge) chapter.
- Paraxial approximation: $x' = p_x/p \ll 1$.

Without forces each particle keeps its momentum $\boldsymbol{p}$ and moves on a straight line at constant speed. With the longitudinal position $s$ as the independent variable the transverse coordinates obey

$$
x'' = 0,\qquad y'' = 0,
$$

that is $x(s) = x_0 + x_0' s$, $y(s) = y_0 + y_0' s$.

### Longitudinal: slippage from the velocity difference

AVAS's longitudinal coordinate $z$ is the position of a particle relative to the reference particle ($z>0$ means ahead of it), and $\delta = \Delta p/p$ is the relative momentum deviation. A particle with momentum deviation $\delta$ has the velocity

$$
\frac{\Delta v}{v} = \frac{1}{\gamma^2}\,\frac{\Delta p}{p} = \frac{\delta}{\gamma^2}.
$$

Through a drift of length $L$ the reference particle needs $t = L/v$; the off-momentum particle travels $\Delta v\, t = L\,\delta/\gamma^2$ farther (or less), so

$$
z = z_0 + \frac{L}{\gamma^2}\,\delta .
$$

This is **debunching** in a drift: particles with higher momentum run ahead, those with lower momentum fall behind, and the bunch lengthens. The factor $1/\gamma^2$ shows that it matters only at low energy; for an ultra-relativistic beam, $\gamma \to \infty$, the longitudinal motion is practically frozen.

> Note: the path-length difference $\tfrac12 L\,(x'^2 + y'^2)$ from the transverse motion is neglected here; it is of second order and does not appear in the linear matrix.

## Equations of motion and transfer matrix {#drift-matrix}

In the six coordinates $(x, x', y, y', z, \delta)$ the transfer matrix of a drift of length $L$ is

$$
M_{\text{drift}}(L) =
\begin{pmatrix}
1 & L & 0 & 0 & 0 & 0\\
0 & 1 & 0 & 0 & 0 & 0\\
0 & 0 & 1 & L & 0 & 0\\
0 & 0 & 0 & 1 & 0 & 0\\
0 & 0 & 0 & 0 & 1 & L/\gamma^2\\
0 & 0 & 0 & 0 & 0 & 1
\end{pmatrix}.
$$

This is exactly the matrix used by AVAS's linear envelope preview (`drift_matrix` in `avas/sim/linear_optics.py`). Its determinant is 1 and it is symplectic, $M\tp J M = J$.

### Evolution of second moments and Twiss parameters

The beam's matrix of second moments (the $\Sigma$ matrix) is transported as $\Sigma_1 = M \Sigma_0 M\tp$. For one plane, in Twiss parameters,

$$
\begin{aligned}
\beta(s) &= \beta_0 - 2\alpha_0 s + \gamma_0 s^2,\\
\alpha(s) &= \alpha_0 - \gamma_0 s,\\
\gamma(s) &= \gamma_0,
\end{aligned}
$$

where $\gamma_0 = (1+\alpha_0^2)/\beta_0$ is the Twiss parameter (not to be confused with the Lorentz factor $\gamma$). Some direct consequences:

- $\gamma_{\text{Twiss}}$ is constant in a drift, so the rms divergence $\sigma_{x'} = \sqrt{\gamma_0 \varepsilon}$ is constant — without a force the angles cannot change.
- The waist ($\alpha = 0$) lies at $s_w = \alpha_0/\gamma_0$. $\alpha_0 > 0$ is a converging beam with its waist downstream; $\alpha_0 < 0$ a diverging beam whose "waist" is upstream (a virtual waist).
- At the waist $\beta_w = 1/\gamma_0$.

Referred to the waist, the beta function reads

$$
\beta(s) = \beta_w + \frac{(s-s_w)^2}{\beta_w}.
$$

Far enough from the waist the rms size grows linearly with distance, with the rms divergence as slope.

### Emittance conservation

A drift is a linear, symplectic map with $\det M = 1$, so the rms geometric emittance of each plane, $\varepsilon = \sqrt{\det \Sigma_{2\times 2}}$, is conserved; a drift does not accelerate, $\beta\gamma$ is constant and so is the normalized emittance. In a multi-particle simulation **without space charge** the rms emittances in DataSet.txt must stay constant through a drift (within statistical noise); if they visibly change, the cause is usually the non-linear part of space charge or particle loss (particles removed by the aperture change the statistics).

## In AVAS {#drift-avas}

A drift is written in the lattice as

```text
drift L R 0
```

```avas-params
drift
```

- `L`: length in **m** (TraceWin lattices use mm — convert when importing or comparing).
- `R`: aperture radius in m, used for loss detection: a particle whose transverse position exceeds this radius is counted as lost.
- The third parameter is reserved; write 0.

> AVAS convention: in the schema `drift` is a **field-model element**, advanced by the t-code (time as the independent variable, integrated with the step set by `steppercycle` in input.txt). Space charge is computed in drifts as usual. Quadrupoles, solenoids, dipoles, steerers and edges, by contrast, are z-code matrix-model elements (see [quadrupole](#quadrupole)).

### Very short drifts at both ends

The AVAS manual says: in a multi-particle model avoid matrix-model elements as the first and the last element; adding a very short drift solves this. For example:

```text
start
drift 0.000001 0.02 0
quad 0.1 0.02 0 6
drift 0.2 0.02 0
quad 0.1 0.02 0 -6
drift 0.2 0.02 0
end
```

Both ends are then t-code elements. The drift at the end also leaves room for output planes (an output plane too close to the end of the lattice fails, see the [simulation](#simulation) chapter).

### Aperture and separate boundary

The `R` of a drift is the circular aperture for loss detection. With `boundary 1` in input.txt the kernel uses the separate boundary of Boundary.txt instead, and the radii of all lattice elements no longer apply:

```avas-params
input:boundary
```

## Parameters and typical values {#drift-params}

| Parameter | Meaning | Typical values (low-energy proton linac) |
|---|---|---|
| `L` | drift length | gaps between elements 0.05–0.3 m; between cavities 0.1–0.5 m; several metres in transport lines |
| `R` | aperture radius | 0.01–0.03 m, usually the same as the neighbouring elements |

Choosing a drift length is a trade-off:

- **Transverse**: the longer the distance between two focusing elements, the larger the beam grows in between and the stronger the focusing must be; in a periodic structure the length directly sets the phase advance per period (see the FODO lab in the [quadrupole](#quadrupole) chapter).
- **Longitudinal**: low-energy beams debunch quickly. For the example beam (protons, $W = 1.527$ MeV, $\gamma = 1.00163$, $1/\gamma^2 = 0.99675$) we are practically in the non-relativistic limit, $L/\gamma^2 \approx L$, and the bunch spreads longitudinally as "freely" as transversely.
- **Aperture**: as a rule of thumb the aperture should be at least 5–10 times the rms size (depending on halo and error budget), otherwise losses become very sensitive.

### Worked example: growth of the example beam in a drift

The initial beam of the example project `examples/hwr010`: $\alpha_x = -0.65785$, $\beta_x = 0.37743$ mm/mrad, $\varepsilon_{n,x} = 0.13768\ \pi$·mm·mrad; $\beta\gamma = 0.05708$. The initial rms size is $\sigma_x = \sqrt{\beta_x \varepsilon_{n,x}/(\beta\gamma)} = 0.954$ mm. With $\alpha_x < 0$ the beam diverges (its virtual waist is about 0.17 m upstream of the entrance). With the matrix above:

| Drift length | rms $x$ | rms $y$ | rms $z$ | rms phase width (162.5 MHz) |
|---|---|---|---|---|
| 0 | 0.954 mm | 0.958 mm | 1.316 mm | 4.5° |
| 0.3 m | 1.64 mm | 1.64 mm | 1.65 mm | 5.7° |
| 0.5 m | 2.19 mm | 2.19 mm | 2.15 mm | 7.4° |
| 1.0 m | 3.64 mm | 3.65 mm | 3.71 mm | 12.7° |

The phase width is $\Delta\phi = 360^\circ \cdot z/(\beta\lambda)$ with $\beta\lambda = 105.1$ mm. Without focusing and without space charge, 1 m of drift almost triples the phase width of the bunch — which is why the low-energy part of a linac needs frequent transverse and longitudinal focusing.

## Dynamics and common problems {#drift-dynamics}

### Space charge makes drifts "not free"

The results above hold only at zero current. With current, the self-field of the beam keeps acting in the drift: it defocuses transversely and longitudinally, and the beam grows faster than at zero current; the non-linear part of space charge also causes rms emittance growth and halo. For low-energy, high-current beams the problem grows with the drift length — one of the main reasons for compact low-energy sections. The models are discussed in the [space charge](#space-charge) chapter.

### Debunching and phase slippage

The longitudinal coordinate changes by $L\,\delta/\gamma^2$, so between two cavities:

- the phase at which the bunch centre (the reference particle) reaches the next cavity is set by the drift length and the velocity: $\Delta\phi = 360^\circ \cdot L/(\beta\lambda)$;
- the phase width of the bunch grows with the drift length; the next cavity receives a longer bunch.

When a drift between cavities is changed, the meaning of the downstream cavity phase depends on V3 of `field`: with V3 = 0 the kernel solves the synchronous phase again, with V3 = 2 the absolute phase stays and the entry phase changes (see [RF cavity](#rf-cavity)).

### Common problems

- **Units**: AVAS lengths and apertures are in m; mm values copied from TraceWin make drifts 1000 times too long, typically seen as the whole beam being lost quickly.
- **Aperture 0 or too small**: particles are counted as lost in the first step.
- **A drift in place of a missing field map**: removes all focusing and acceleration of that element; use the result only for comparison.
- **Emittance "changes"**: without space charge the emittance in a drift must be conserved; if it changes, check losses first (surviving macro-particles, column 28 of DataSet.txt).

## Hands-on {#drift-lab}

The lab below puts the $x$-plane parameters of the example beam into a drift (`f=0` means no lens). Drag `L` and watch the rms ellipse shear: its area (the emittance) stays, its tilt changes with distance, $\alpha$ becomes more negative, the rms size grows and the rms divergence stays.

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.5
```

Then set `alpha` to a positive value (for example 1.5): the beam first converges to a waist and then diverges, and you can check the waist position $s_w = \alpha_0/\gamma_0$.

To see longitudinal debunching, build a lattice of `drift` only in the visual editor and watch the rms $z$ of the linear envelope preview; then run the same structure with the kernel and compare with and without space charge (`spacecharge 0/1` in input.txt).

## Further reading {#drift-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH (2008): chapter 7 on matrix methods in transverse dynamics, chapter 6 on longitudinal motion.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH (2008): chapter 3 on beam optics and focusing systems without space charge.
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer (2015): transfer matrices and the evolution of Twiss parameters.
- Related chapters: [phase space](#phase-space), [linear optics](#linear-optics), [longitudinal motion](#longitudinal), [space charge](#space-charge); the full bibliography is in [references](#references).
