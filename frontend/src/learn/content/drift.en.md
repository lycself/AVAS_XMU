# Drift

A drift is a piece of vacuum pipe without applied electromagnetic fields. The particles inside feel no external force and each moves on a straight line. Yet when the beam leaves a drift it is wider transversely and longer longitudinally; and over the same metre of drift a bunch of 1.5 MeV protons stretches to almost three times its length while a bunch of 10 MeV electrons hardly changes. With no force, where does the change come from?

This chapter shows that a drift changes the momentum of no particle at all; it only turns the **angle differences** and **velocity differences** already present between the particles into position differences. The transverse half is the divergence familiar from optics; the longitudinal half — $R_{56} = L/\gamma^2$ — is why low-energy linacs have to rebunch so often, and also why a buncher cavity works at all. Drifts are the most frequent element of a lattice and the reference for understanding all others: the transfer matrix of any element can be seen as "drift + the action of the field".

You need the $\beta$ and $\gamma$ of [Kinematics](#kinematics), the Twiss parameters and rms emittance of [Phase space](#phase-space), and the transfer matrices of [Linear optics](#linear-optics). Below is a 1 m drift; the example beam starts at its entrance and its rms size grows monotonically:

```avas-beamline
drift 1.0 0.02 0
```

## Role and placement {#drift-role}

What drifts do in a linac:

- **Geometric spacing**: quadrupoles, solenoids and cavities need room between them for flanges, bellows, vacuum valves and beam diagnostics (BPMs, current transformers, profile monitors). For the beam these spaces are drifts.
- **Phase adjustment**: between two RF cavities the drift length sets the RF phase at which the bunch reaches the next cavity (see [Longitudinal dynamics](#longitudinal)).
- **Free evolution**: transversely a diverging beam grows linearly in a drift, a converging beam first shrinks (waist) and then grows; longitudinally a bunch with momentum spread lengthens (debunching).
- **Loss detection**: the aperture radius `R` of a drift decides whether a particle "hits the wall", so drifts are among the main places where AVAS detects beam loss.

In AVAS drifts also have a "technical" use: matrix-model elements should not be the first or last element of a multi-particle lattice, so a very short drift is often inserted at both ends (see [In AVAS](#drift-in-avas)).

## Transversely: no force, yet the ellipse changes {#drift-physics}

### Physical picture

Think of the light from a lamp: every ray is straight, yet the beam of light gets wider and wider because the rays point in different directions. A beam in a drift behaves exactly the same way. Each particle keeps its angle $x'$ and its position grows by $x' \cdot s$: particles with large angles travel far, those with small angles little.

In the $(x, x')$ phase plane this is a "shear": each point moves horizontally by $x' s$ and not at all vertically. The ellipse is tilted and stretched, but its area (the emittance) stays, and so does its vertical width (the rms divergence).

### Assumptions and equations

- No external fields: $\boldsymbol{E} = 0$, $\boldsymbol{B} = 0$.
- Space charge (the beam's self-field) is ignored for now; its effect in drifts is discussed in [Space charge and phase slip](#drift-dynamics) and in the [Space charge](#space-charge) chapter.
- Paraxial approximation: $x' = p_x/p \ll 1$.

Without forces each particle's momentum $\boldsymbol{p}$ is conserved and it moves uniformly in a straight line. With the longitudinal position $s$ as the independent variable, the transverse coordinates satisfy

$$
x'' = 0,\qquad y'' = 0,
$$

i.e. $x(s) = x_0 + x_0' s$, $y(s) = y_0 + y_0' s$.

A number: the example beam has an rms divergence $\sigma_{x'} = 3.03\ \mathrm{mrad}$, so far from a waist its rms size grows by about 3 mm per metre. It is already diverging at the entrance ($\alpha_x = -0.658 < 0$); the corresponding "virtual waist" lies 0.173 m upstream of the entrance with an rms size of 0.80 mm.

> **Think about it**: in a drift no particle changes its angle and the rms divergence stays constant. Why then does the Twiss parameter $\alpha$ change?
>
> Answer: $\alpha \propto -\langle x x'\rangle$ describes the **correlation** between position and angle. After some distance the particles with positive angles have all moved to positive $x$ and those with negative angles to negative $x$; $x$ and $x'$ increasingly have the same sign and the correlation grows. What changes is not any single particle but the way the particles are arranged.

## Longitudinally: velocity differences become position differences {#drift-longitudinal}

### Physical picture

The longitudinal story is the same as the transverse one with "different angles" replaced by "different speeds". Picture runners starting together at slightly different speeds: the further they run, the more the fast ones pull away from the slow ones. The particles of a bunch have different momenta; those with more momentum are faster and gradually move ahead in a drift, those with less fall behind, and the bunch lengthens — this is **debunching**.

### How sensitive is velocity to momentum: $1/\gamma^2$

Does a given momentum difference mean the same velocity difference? No. From $E^2 = p^2c^2 + m^2c^4$ we get $\dd E/E = \beta^2\,\dd p/p$; the velocity is $v = pc^2/E$, so

$$
\frac{\Delta v}{v} = \frac{\Delta p}{p} - \frac{\Delta E}{E} = \left(1 - \beta^2\right)\frac{\Delta p}{p} = \frac{\delta}{\gamma^2}.
$$

with $\delta = \Delta p/p$. At low energy $\gamma \approx 1$: 1 % more momentum means 1 % more speed; at high energy the speed is "capped" by the speed of light and hardly changes however large the momentum.

AVAS's longitudinal coordinate $z$ is the particle's longitudinal position relative to the reference particle ($z>0$ means ahead of it). Through a drift of length $L$ the reference particle needs $t = L/v$, and the off-momentum particle travels $\Delta v\, t = L\,\delta/\gamma^2$ more (or less), hence

$$
z = z_0 + \frac{L}{\gamma^2}\,\delta, \qquad R_{56} = \frac{L}{\gamma^2}.
$$

$R_{56}$ is the element of the 6×6 transfer matrix that maps $\delta$ to $z$, the exact counterpart of the transverse $M_{12} = L$: the longitudinal "angle" is $\delta/\gamma^2$. $R_{56}$ per metre of drift for different particles:

| Particle and kinetic energy | $\gamma$ | $\beta$ | $R_{56}/L = 1/\gamma^2$ |
|---|---|---|---|
| proton 1.527 MeV (example) | 1.0016 | 0.0570 | 0.997 |
| proton 10 MeV | 1.0107 | 0.145 | 0.979 |
| proton 100 MeV | 1.107 | 0.428 | 0.817 |
| proton 1 GeV | 2.066 | 0.875 | 0.234 |
| electron 1 MeV | 2.957 | 0.941 | 0.114 |
| electron 10 MeV | 20.57 | 0.9988 | 0.0024 |

The example protons are almost in the non-relativistic limit, $L/\gamma^2 \approx L$: longitudinally they spread "freely" just as transversely. The $R_{56}$ of 10 MeV electrons is only 1/420 of that; over one metre an electron with $\delta = 10^{-3}$ moves only 2.4 μm out of place, and the longitudinal motion is almost frozen.

### In terms of RF phase

For the RF what matters is not millimetres but phase: $\Delta\varphi = -2\pi z/(\beta\lambda)$. For the example beam $\beta\lambda = 105.1\ \mathrm{mm}$, so 1 mm corresponds to 3.42°. The example's rms momentum spread $\delta = 3.53\ \mathrm{mrad}$ (rms energy spread 10.8 keV) widens the rms phase width from 4.5° to 12.7° in one metre of drift.

> **Think about it**: $z$ is only the relative position of the particles. Why does it matter if the bunch gets a little longer?
>
> Answer: the next cavity notices. The longer the bunch, the more spread the phases at which the particles arrive and the more different the energies they gain; at phase widths of tens of degrees the curvature of $\cos\varphi$ also makes the longitudinal emittance grow; near the width of the longitudinal acceptance (about 100° for the example cavity at $-33°$, see [Longitudinal dynamics](#longitudinal-separatrix)) particles are lost. Low-energy sections must rebunch before the bunch gets too long, which is why the drifts between cavities cannot be long.

### Debunching as a tool: ballistic bunching

$R_{56}$ is not only a nuisance. If the bunch is first given **less energy at its head and more at its tail** (a "chirp"), a following drift lets the tail catch up with the head and the bunch becomes shorter. An RF cavity at $\varphi_s = -90°$ does not accelerate but produces exactly this energy–position correlation, so it is a **buncher**; the drift turns the energy differences into position differences. With the example cavity (linear preview): hwr010 at $\varphi_s = -90°$ leaves the energy unchanged; placed directly at the start of the example beam, the rms $z$ is 0.72 mm at the cavity exit, reaches its minimum of about 0.63 mm some 5 cm downstream, and then grows again because of the energy spread the bunch now carries. Try it in the lab below (5 cm of drift, the cavity, 0.2 m of drift): look at the dashed rms $z$ curve in the envelope plot — with the 5 cm of drift in front the bunch arrives slightly longer, and the numbers become 0.77 mm at the exit and a minimum of 0.64 mm about 6 cm downstream. Move `phase` away from −90° and the cavity starts to accelerate as well as bunch. The bunchers of the medium-energy transport work this way, see [Medium-energy transport](#journey-mebt).

```avas-lab
rf-cavity phase=-90 Ke=1.36 W=1.527
```

> Note: the path-length difference $\tfrac12 L\,(x'^2 + y'^2)$ from transverse motion is neglected here; it is of second order and does not appear in the linear matrix.

## Equation of motion and transfer matrix {#drift-matrix}

In the six-dimensional coordinates $(x, x', y, y', z, \delta)$ the transfer matrix of a drift of length $L$ is

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

Each of the three planes is a shear. This is exactly the matrix used by AVAS's linear envelope preview (`drift_matrix` in `avas/sim/linear_optics.py`). Its determinant is 1 and it satisfies the symplectic condition $M\tp J M = J$.

### Evolution of second moments and Twiss parameters

The second-moment matrix ($\Sigma$ matrix) transforms as $\Sigma_1 = M \Sigma_0 M\tp$. For a single plane, in terms of Twiss parameters:

$$
\begin{aligned}
\beta(s) &= \beta_0 - 2\alpha_0 s + \gamma_0 s^2,\\
\alpha(s) &= \alpha_0 - \gamma_0 s,\\
\gamma(s) &= \gamma_0,
\end{aligned}
$$

where $\gamma_0 = (1+\alpha_0^2)/\beta_0$ is the Twiss parameter (not to be confused with the relativistic factor $\gamma$). Some direct consequences:

- $\gamma_{\text{Twiss}}$ is constant in a drift, i.e. the rms divergence $\sigma_{x'} = \sqrt{\gamma_0 \varepsilon}$ is constant — without forces, angles do not change.
- The waist ($\alpha = 0$) is at $s_w = \alpha_0/\gamma_0$. $\alpha_0 > 0$ means a converging beam with the waist downstream; $\alpha_0 < 0$ means a diverging beam with the "waist" upstream (virtual waist).
- At the waist $\beta_w = 1/\gamma_0$.

Relative to the waist the $\beta$ function reads

$$
\beta(s) = \beta_w + \frac{(s-s_w)^2}{\beta_w}.
$$

This formula is very useful: the thinner the waist (the smaller $\beta_w$), the faster the beam grows away from it. Squeezing a beam to a very small size at one place costs a rapid growth on both sides — this is what a finite emittance means. Far enough from the waist the rms size grows linearly with distance, with the rms divergence as slope.

### Emittance conservation

A drift is a linear, symplectic map with $\det M = 1$, so the rms geometric emittance $\varepsilon = \sqrt{\det \Sigma_{2\times 2}}$ of each plane is conserved; a drift does not accelerate, $\beta\gamma$ is constant, and the normalized emittance is conserved too. In an actual multi-particle simulation **without space charge**, the rms emittances in DataSet.txt should stay constant through a drift (within statistical noise); a clear change usually comes from the non-linear part of space charge or from particle losses (particles removed by the aperture change the statistics).

## The example beam in a drift {#drift-example}

The initial beam of the example project `examples/hwr010`: $\alpha_x = -0.65785$, $\beta_x = 0.37743$ mm/mrad, $\varepsilon_{n,x} = 0.13768\ \pi$·mm·mrad; $\beta\gamma = 0.05708$. The initial rms size is $\sigma_x = \sqrt{\beta_x \varepsilon_{n,x}/(\beta\gamma)} = 0.954$ mm. Since $\alpha_x < 0$ the beam is diverging (virtual waist about 0.17 m upstream of the entrance). With the matrix above (zero current):

| Drift length | rms $x$ | rms $y$ | rms $z$ | rms phase width (162.5 MHz) |
|---|---|---|---|---|
| 0 | 0.954 mm | 0.958 mm | 1.316 mm | 4.5° |
| 0.3 m | 1.64 mm | 1.64 mm | 1.65 mm | 5.7° |
| 0.5 m | 2.19 mm | 2.19 mm | 2.15 mm | 7.4° |
| 1.0 m | 3.64 mm | 3.65 mm | 3.71 mm | 12.7° |

The phase width is $\Delta\phi = 360^\circ \cdot z/(\beta\lambda)$ with $\beta\lambda = 105.1$ mm. Without focusing and space charge, 1 m of drift nearly triples the phase width of the bunch — which shows why low-energy sections need frequent transverse and longitudinal focusing. Incidentally the beam grows about equally fast in both directions: the example's rms divergence of 3.03 mrad is close to its rms $\delta/\gamma^2$ of 3.52 mrad.

The lab below puts the example beam's $x$-plane parameters into a drift (`f=0` means no lens). Drag `L` and watch the rms ellipse being "sheared": its area (emittance) stays, its tilt increases with distance, $\alpha$ becomes more negative, and the rms size grows while the rms divergence stays.

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.5
```

Then set `alpha` to a positive value (e.g. 1.5): the beam converges to a waist first and then diverges, and you can check the waist position $s_w = \alpha_0/\gamma_0$.

## Space charge and phase slip {#drift-dynamics}

### Space charge makes the drift "no longer free"

The above holds only at zero current. With current, the beam's self-field acts all along the drift: both transversely and longitudinally there is a defocusing force and the beam grows faster than at zero current; the non-linear part of space charge also causes rms emittance growth and halo. The linear preview for the example beam after 1 m of drift:

| Average current | rms $x$ | rms $z$ |
|---|---|---|
| 0 | 3.64 mm | 3.71 mm |
| 0.184 mA (example) | 3.69 mm | 3.75 mm |
| 1 mA | 3.93 mm | 3.92 mm |
| 10 mA | 6.07 mm | 5.62 mm |

At the example current space charge adds only 1.5 % to the growth; the same bunch at 10 mA is 67 % wider transversely after 1 m. For low-energy, high-current beams the problem worsens with the drift length, one of the main reasons why low-energy sections must be compact. The detailed model is in the [Space charge](#space-charge) chapter.

### Debunching and phase slip

The longitudinal coordinate changes by $L\,\delta/\gamma^2$, so between two cavities:

- the phase at which the bunch centre (reference particle) reaches the next cavity is set by the drift length and the speed: $\Delta\phi = 360^\circ \cdot L/(\beta\lambda)$; the example beam advances 342.4° per 0.1 m;
- the phase width of the bunch grows with the drift length, and the next cavity receives a longer bunch.

When a drift between cavities is changed, the meaning of the downstream cavity phases depends on the V3 setting of `field`: with V3 = 0 the engine solves for the synchronous phase again, with V3 = 2 the absolute phase stays and the entrance phase changes (see [RF cavity](#rf-cavity) and [Longitudinal dynamics](#longitudinal-in-avas)).

## Common misconceptions {#drift-misconceptions}

- **Does the beam always grow in a drift?** Not necessarily. A converging beam ($\alpha > 0$) first shrinks to a waist and then grows.
- **Is the rms divergence always constant?** Only at zero current. Space charge keeps pushing the particles out in a drift and the divergence grows.
- **Is the longitudinal motion "frozen" in a drift?** Only approximately for high-energy electrons. Low-energy protons have $1/\gamma^2 \approx 1$ and spread longitudinally just as transversely.
- **Mixing $R_{56}$ with the phase-slip coefficient**: $R_{56} = L/\gamma^2$ is the coefficient in $(z, \delta)$; in $(\Delta\varphi, \Delta W)$ it becomes the $2\pi/(\beta^3\gamma^3\lambda)$ of [Longitudinal dynamics](#longitudinal-stability) (with $\Delta W$ in units of $mc^2$): the conversion $z \to \Delta\varphi$ contributes $2\pi/(\beta\lambda)$ and $\delta \to \Delta W$ contributes $1/(\beta^2\gamma)$, which together with $1/\gamma^2$ give $2\pi/(\beta^3\gamma^3\lambda)$.
- **Two γs**: the Twiss parameter $\gamma_x = (1+\alpha^2)/\beta_x$ and the relativistic factor $\gamma$ are different quantities; both appear in the drift formulas.
- **The emittance changed in a drift**: without space charge it must be conserved; when it changes, first check particle losses (surviving macro-particles, column 28 of DataSet.txt), then space charge.
- **A drift "computes nothing"**: in AVAS a drift is a t-code element; space charge is computed and losses at the aperture are detected as usual.

## In AVAS {#drift-in-avas}

### Syntax

A drift is written in the lattice as

```text
drift L R 0
```

```avas-params
drift
```

- `L`: length in **m** (TraceWin lattices use mm; convert when importing or comparing).
- `R`: aperture radius in m, used for loss detection: a particle whose transverse position exceeds this radius counts as lost.
- The third parameter is reserved; write 0.

> **AVAS convention**: in the schema `drift` is a **field-model element**, advanced by the t-code (time as independent variable, integrated with the step set by `steppercycle` in input.txt). Space charge is computed in drifts as usual. By contrast, quadrupoles, solenoids, dipoles, steerers and edges are z-code matrix-model elements (see [Quadrupole](#quadrupole)).

### Very short drifts at both ends

The AVAS manual advises against matrix-model elements as the first or the last element of a multi-particle model and suggests a very short drift to avoid this. For example:

```text
start
drift 0.000001 0.02 0
quad 0.1 0.02 0 6
drift 0.2 0.02 0
quad 0.1 0.02 0 -6
drift 0.2 0.02 0
end
```

Both ends are then t-code elements. The final drift also leaves a margin for output planes (an output plane too close to the end of the lattice fails, see [Multi-particle simulation](#simulation)).

### Aperture and independent boundary

The drift's `R` is a circular aperture for loss detection. With `boundary 1` in input.txt the engine uses the independent boundary in Boundary.txt instead, and the radii of all lattice elements no longer apply:

```avas-params
input:boundary
```

### Typical values

| Parameter | Meaning | Typical values (low-energy proton linac) |
|---|---|---|
| `L` | drift length | 0.05–0.3 m between elements; 0.1–0.5 m between cavities; up to several metres in transfer lines |
| `R` | aperture radius | 0.01–0.03 m, often the same as neighbouring elements |

Choosing a drift length is a trade-off:

- **Transverse**: the longer the distance between two focusing elements, the more the beam grows in between and the stronger the focusing must be; in a periodic structure the length directly sets the phase advance per period (see the FODO lab in [Linear optics](#linear-optics-periodic)).
- **Longitudinal**: low-energy beams debunch quickly. The example beam has $1/\gamma^2 = 0.99675$, $L/\gamma^2 \approx L$; the longer the drift between cavities, the longer the bunch that reaches the next one.
- **Aperture**: as a rule of thumb the aperture should be at least 5–10 times the rms size (depending on halo and error budget), otherwise the losses become very sensitive.

### Common problems

- **Units**: AVAS lengths and apertures are in m; mm values copied from TraceWin make a drift 1000 times longer, and typically the beam is lost almost immediately.
- **Aperture 0 or too small**: particles are counted as lost at the first step.
- **Using a drift in place of a missing field map**: this removes all focusing and acceleration of the element; use the result only for comparison.
- **Emittance "changes"**: without space charge the emittance should be conserved in a drift; if it changes, check losses first (surviving macro-particles, column 28 of DataSet.txt).

### Watching debunching in the editor

To see longitudinal debunching, build a lattice with only `drift` in the visual editor and look at rms $z$ in the linear envelope preview; then run the same structure with the engine and compare with and without space charge (`spacecharge 0/1` in input.txt).

## Further reading {#drift-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH (2008): chapter 7 on transverse dynamics (matrix methods), chapter 6 on longitudinal motion.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH (2008): chapter 3 (beam optics and focusing systems without space charge).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer (2015): transfer matrices and the evolution of Twiss parameters.
- Related chapters: [Phase space](#phase-space), [Linear optics](#linear-optics), [Longitudinal dynamics](#longitudinal), [Space charge](#space-charge); full bibliography in [References](#references).
