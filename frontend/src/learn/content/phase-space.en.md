# Phase space and emittance

A lens can focus a beam to a very small spot — at the price of the beam spreading out at a larger angle afterwards; make the beam very parallel and it becomes wide. Can a beam be both narrow and parallel? Why not? The answer is a quantity that is conserved almost everywhere along a beamline: the emittance.

This chapter explains why a particle has to be described by its position and its angle together; how linear elements move particles in the position–angle plane, why the occupied area does not change (Liouville's theorem) and why acceleration still shrinks it (adiabatic damping); how the rms ellipse of a beam is described by Twiss parameters and the σ matrix; how longitudinal phase space relates to phase and energy; and finally how these quantities map onto `twissx`, `twissy`, `twissz` in `beam.txt`. The chapter uses $\beta\gamma$ and $\delta$ from [Relativistic kinematics](#kinematics); shapes, edges and tails of different distributions are in the chapter [Initial distributions](#distributions); the evolution of the ellipse along a beamline is in [Linear optics](#linear-optics).

Notation: in this chapter $\beta$ and $\gamma$ without subscript are the relativistic factors; $\beta_x$, $\alpha_x$, $\gamma_x$ with a plane subscript are Twiss parameters.

## Why position and angle together {#phase-space-coordinates}

Imagine taking a cross-section photograph of two proton beams at the same place on a beamline: both spots are equally large. One beam is converging, the other diverging, and 10 cm later they are completely different. Position alone does not determine where the beam goes next: the equation of motion is second order, so we need both the position and the direction of the velocity. For a particle advancing along $s$ the direction is the slope $x' = \dd x/\dd s$.

So in each transverse plane a particle is described by a pair $(x, x')$; this plane is the **phase plane**, and the three directions together form the six-dimensional **phase space**. The coordinates are deviations from the reference particle (the particle moving on the design orbit), and $s$ is the distance along the orbit (on a straight section simply the longitudinal position $z$):

| Coordinate | Meaning | Unit (AVAS internal / matrices) |
|---|---|---|
| $x$, $y$ | transverse position deviation | m |
| $x' = \dd x/\dd s$, $y' = \dd y/\dd s$ | transverse angle (slope) | rad |
| $z$ | longitudinal position deviation, positive ahead of the reference particle | m |
| $\delta = \Delta p/p$ | relative momentum deviation | 1 |

In the paraxial approximation $x' \approx p_x/p$, with $p$ the reference momentum. Transfer matrices act on the column vector $(x, x', y, y', z, \delta)\tp$; see [Linear optics](#linear-optics).

One number shows how much the angle matters: at the entrance the example beam has an rms size of 0.954 mm and an rms angle of 3.026 mrad. A particle with a 3 mrad angle moves 0.3 mm sideways in 0.1 m — a third of the beam size.

Two kinds of "snapshot" have to be kept apart:

- the distribution **at one time**: like photographing the whole bunch at one instant. The t-code (field-map elements and drifts) advances in time, and at a given time the particles are at different longitudinal positions;
- the distribution **at one longitudinal position**: like a photo-finish at a finish line, each particle recorded as it crosses the plane. The z-code (matrix elements) and transfer-matrix theory compare all particles at the same $s$.

For rms quantities the difference is usually small, but care is needed for long bunches with a large energy spread (for example the $z$–$\delta$ correlation). According to the manual, `DataSet.txt` records the bunch parameters in the s frame. Where the two descriptions come from in AVAS is explained in [t-code and z-code](#intro-tracking).

## How linear elements move points in phase space {#phase-space-shear}

Look first at what the two simplest elements do in the phase plane.

**Drift**: the particle travels a straight distance $L$; its position grows by $L x'$ and its angle is unchanged:

$$
\begin{pmatrix} x \\ x' \end{pmatrix}_1 = \begin{pmatrix} 1 & L \\ 0 & 1 \end{pmatrix}\begin{pmatrix} x \\ x' \end{pmatrix}_0
$$

Points with a positive angle move to the right, points with a negative angle to the left, by an amount proportional to the angle — the phase plane is **sheared horizontally**.

**Thin lens** (focal length $f$): the position is unchanged and the angle changes by $-x/f$; the farther from the axis, the stronger the kick:

$$
\begin{pmatrix} x \\ x' \end{pmatrix}_1 = \begin{pmatrix} 1 & 0 \\ -1/f & 1 \end{pmatrix}\begin{pmatrix} x \\ x' \end{pmatrix}_0
$$

This is a **vertical shear**. A shear is like pushing a deck of cards: the shape changes, the area does not. Mathematically, both matrices have determinant 1. Any combination of drifts and lenses is a product of such matrices and still has determinant 1, so the area a cloud of points occupies in the phase plane does not change in linear transport.

Example: in a 0.1 m drift the x plane of the example beam goes from $\beta_x$ = 0.377 to 0.547 mm/mrad, and the rms size from 0.954 mm to 1.149 mm; the area of the rms ellipse is unchanged.

> **Think about it**: the opening question — can lenses and drifts make a beam both narrow and parallel?
>
> Answer: no. At a waist (upright ellipse, no correlation between position and angle) the product of rms size and rms angle is exactly the rms emittance: $\sigma_x\sigma_{x'} = \varepsilon$. The example beam has a geometric rms emittance of 2.412 mm·mrad; squeezing the rms size to 0.5 mm at a waist means an rms angle of at least 4.82 mrad. Lenses can only trade "narrow" against "parallel".

## Liouville's theorem and adiabatic damping {#phase-space-liouville}

### Liouville's theorem

The area conservation above is a special case of a more general theorem. If the motion of the particles can be described by a Hamiltonian (external electromagnetic fields, and space charge in the mean-field approximation), the phase-space density is constant along particle trajectories in the six-dimensional phase space of canonical coordinates $(x, p_x, y, p_y, z, p_z)$, and the phase-space volume occupied by any group of particles is conserved. If the three directions are uncoupled, the area of each two-dimensional projection is conserved separately. Particles in phase space behave like an incompressible fluid: they can be stretched and twisted, but not compressed.

Liouville's theorem does not guarantee that the "effective" emittance is conserved. Non-linear forces (field non-linearities, non-linear space charge, the sinusoidal shape of RF fields) make particles of different amplitude rotate at different frequencies, and the phase-space ellipse is twisted into an S shape or a spiral. The occupied area is unchanged, but the ellipse needed to enclose it and the rms emittance both grow; this is called **filamentation**. Think of stirring a spoonful of milk into coffee: the volume of the milk stays the same, but it spreads through the whole cup, and any outline drawn around the milk has to be large. The analogy is right about incompressibility; it fails in that milk really mixes by molecular diffusion, whereas the filaments of Hamiltonian motion never truly merge.

Particle losses, collisions, stripping and similar processes do not satisfy the premises of Liouville's theorem.

### Adiabatic damping and normalized emittance

The angle $x' = p_x/p$ is not a canonical momentum. Ideal longitudinal acceleration increases $p$ without changing $p_x$, so $x'$ shrinks like $1/p$ and the area of the $(x, x')$ plane shrinks like $1/(\beta\gamma)$; this is called **adiabatic damping**. The area of the canonical plane $(x, p_x/mc) = (x, \beta\gamma x')$ is conserved. Hence the definition

$$
\varepsilon_n = \beta\gamma\,\varepsilon
$$

where $\varepsilon$ is the geometric and $\varepsilon_n$ the normalized emittance. In the ideal linear, uncoupled, filamentation-free case $\varepsilon_n$ stays constant along a linac, which makes it the right measure for comparing beam quality at different energies.

Example: the example beam has a normalized rms emittance of 0.1377 π·mm·mrad, which at 1.527 MeV ($\beta\gamma$ = 0.05708) is a geometric emittance of 2.412 mm·mrad. Accelerated to 10 MeV, $\beta\gamma$ grows to 0.14639 and the geometric emittance shrinks by a factor 2.565 to 0.940 mm·mrad, while the normalized emittance stays the same.

> **Think about it**: adiabatic damping makes the geometric emittance smaller. Has the beam been "cooled"?
>
> Answer: no. The transverse momentum $p_x$ of each particle has not changed at all; only the longitudinal momentum has grown, so the trajectories have become more "forward". The normalized emittance — proportional to the spread of the transverse momentum — is unchanged. Real cooling (electron cooling, stochastic cooling) reduces the spread of $p_x$ and needs a dissipative process, which is not bound by Liouville's theorem.

> Note: the formula uses the $\beta\gamma$ of the reference particle. For beams with a very large energy spread, a normalized emittance computed with each particle's own $\beta\gamma$ differs from it; the difference can be significant in low-energy electron beams.

## Definitions of emittance {#phase-space-emittance}

### rms emittance

For $N$ particles write the centred second moments $\langle x^2\rangle = \frac1N\sum (x - \bar x)^2$, and likewise $\langle x x'\rangle$, $\langle x'^2\rangle$. The rms emittance is

$$
\varepsilon_{\mathrm{rms}} = \sqrt{\langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2}
$$

It is the area of the "rms ellipse" in phase space divided by $\pi$. The second term $\langle x x'\rangle^2$ removes the correlation between position and angle: a beam that is merely converging or diverging can have a large $\langle x^2\rangle\langle x'^2\rangle$, but the correlated part can be "straightened out" by a lens and is not a true spread. The rms emittance has a definite value for any distribution, is exactly conserved in linear transport (without coupling) and grows under non-linear forces, which makes rms emittance growth the usual measure of beam-quality degradation. Why second moments are the natural measure and how sensitive they are to tails is discussed in [Initial distributions](#distributions-rms).

### Total and fractional emittance

$\varepsilon_{100\%}$ (total or edge emittance) is the area of the smallest ellipse containing all particles, divided by $\pi$; $\varepsilon_{90\%}$, $\varepsilon_{99\%}$ and so on contain the corresponding fraction of particles. Their ratio to the rms emittance depends on the shape of the distribution: 4 for KV, 6 for a 4-D waterbag, 8 for parabolic, and a Gaussian has no total emittance; see [The four standard distributions](#distributions-types).

### Units

Emittance has the dimension length × angle. It is customarily written in π·mm·mrad, where the "π" is only a label meaning that the number is the area divided by π:

$$
1\ \pi\cdot\mathrm{mm\cdot mrad} \;\to\; \varepsilon = 1\ \mathrm{mm\cdot mrad} = 1\ \mu\mathrm{m} = 10^{-6}\ \mathrm{m\cdot rad}
$$

When quoting an emittance, always state three things: rms or total (fractional), geometric or normalized, and whether π has already been divided out.

## Twiss parameters and the Courant–Snyder invariant {#phase-space-twiss}

A phase-space ellipse centred at the origin can be written as

$$
\gamma_x x^2 + 2\alpha_x x x' + \beta_x x'^2 = \varepsilon, \qquad \beta_x\gamma_x - \alpha_x^2 = 1
$$

$\alpha_x$, $\beta_x$, $\gamma_x$ are the Twiss or Courant–Snyder parameters; the ellipse has area $\pi\varepsilon$. Only two of the three parameters are independent: they describe the **shape and orientation** of the ellipse, while $\varepsilon$ describes its **size**. For the rms ellipse they are related to the second moments by

$$
\langle x^2\rangle = \beta_x\varepsilon, \qquad
\langle x x'\rangle = -\alpha_x\varepsilon, \qquad
\langle x'^2\rangle = \gamma_x\varepsilon
$$

Conversely, from a particle distribution: $\beta_x = \langle x^2\rangle/\varepsilon_{\mathrm{rms}}$, $\alpha_x = -\langle x x'\rangle/\varepsilon_{\mathrm{rms}}$.

### Geometry of the ellipse

- maximum position $x_{\max} = \sqrt{\varepsilon\beta_x}$, maximum angle $x'_{\max} = \sqrt{\varepsilon\gamma_x}$;
- intersection with the $x$ axis at $x = \sqrt{\varepsilon/\gamma_x}$, with the $x'$ axis at $x' = \sqrt{\varepsilon/\beta_x}$;
- the tilt $\theta$ of the principal axes relative to the $x$ axis satisfies $\tan 2\theta = 2\alpha_x/(\gamma_x - \beta_x)$.

The x plane of the example beam ($\alpha_x = -0.658$, $\beta_x = 0.377$ mm/mrad, $\gamma_x = 3.796$ mrad/mm, $\varepsilon = 2.412$ mm·mrad): the rms ellipse has $x_{\max}$ = 0.954 mm and $x'_{\max}$ = 3.026 mrad, and crosses the axes at 0.797 mm and 2.528 mrad.

The sign of $\alpha_x$ tells whether the beam is converging or diverging:

| $\alpha_x$ | $\langle x x'\rangle$ | State |
|---|---|---|
| $\alpha_x > 0$ | $< 0$ | converging (moving towards a waist) |
| $\alpha_x = 0$ | $= 0$ | waist (upright ellipse) |
| $\alpha_x < 0$ | $> 0$ | diverging (moving away from a waist) |

In a drift the waist lies at $\alpha_x/\gamma_x$ (positive means downstream), and at the waist $\beta^* = 1/\gamma_x$. For the example beam $\alpha_x/\gamma_x = -0.173$ m: the waist is 0.173 m upstream of the entrance, with $\beta^*$ = 0.263 mm/mrad and an rms size of 0.797 mm — exactly the intersection of the ellipse with the $x$ axis, because the shear of a drift does not move the points with $x' = 0$.

### The Courant–Snyder invariant

In a linear focusing system the Twiss parameters vary with $s$ (following the transport rules of [Linear optics](#linear-optics)), while the single-particle quantity

$$
J = \gamma_x(s)\,x^2 + 2\alpha_x(s)\,x x' + \beta_x(s)\,x'^2
$$

stays constant along the beamline; it is called the Courant–Snyder invariant. In other words, every particle stays on its own member of a family of ellipses that keep changing shape but not area; $J$ is the "label" of that ellipse and measures the amplitude of the particle's oscillation. The trajectory can be written $x(s) = \sqrt{J\beta_x(s)}\cos\left(\psi(s) + \psi_0\right)$, where $\psi$ is the betatron phase; see [Linear optics](#linear-optics). So $\sqrt{\beta_x}$ decides how the oscillation amplitude swells and shrinks along the beamline, and $\sqrt{J}$ decides the "size" of this particular particle.

## The σ matrix and the phase ellipse {#phase-space-sigma}

Arrange the second moments in a symmetric matrix (beam matrix, σ matrix):

$$
\Sigma = \begin{pmatrix} \langle x^2\rangle & \langle x x'\rangle \\ \langle x x'\rangle & \langle x'^2\rangle \end{pmatrix}
= \varepsilon \begin{pmatrix} \beta_x & -\alpha_x \\ -\alpha_x & \gamma_x \end{pmatrix},
\qquad \varepsilon = \sqrt{\det\Sigma}
$$

The rms ellipse is $X\tp\Sigma^{-1}X = 1$ with $X = (x, x')\tp$. For the x plane of the example beam: $\langle x^2\rangle$ = 0.910 mm², $\langle x x'\rangle$ = 1.587 mm·mrad, $\langle x'^2\rangle$ = 9.157 mrad², and $\sqrt{\det\Sigma}$ = 2.412 mm·mrad.

The advantage of the σ matrix is how simply it transforms under linear transport $X_1 = M X_0$:

$$
\Sigma_1 = M\,\Sigma_0\,M\tp
$$

The derivation takes one step: $\langle X_1 X_1\tp\rangle = M\langle X_0 X_0\tp\rangle M\tp$. It holds for any distribution (not only elliptical ones), because only second moments are involved; and $\det\Sigma_1 = (\det M)^2\det\Sigma_0$ with $\det M = 1$ immediately gives the conservation of the rms emittance. The AVAS linear preview advances the beam in exactly this way.

### The six-dimensional σ matrix

In six dimensions $\Sigma_{ij} = \langle u_i u_j\rangle$ with $u = (x, x', y, y', z, \delta)$. The diagonal blocks give the projected emittances of the three planes; the off-diagonal blocks describe correlations between planes:

- dipoles create an $x$–$\delta$ correlation (dispersion), see [Dipole](#dipole);
- solenoids couple the $x$ and $y$ planes; the projected emittances $\varepsilon_x$, $\varepsilon_y$ can then change, but the four-dimensional emittance $\sqrt{\det\Sigma_4}$ is conserved under linear symplectic transport, see [Solenoid](#solenoid);
- RF cavities create a $z$–$\delta$ correlation, see [Longitudinal motion](#longitudinal).

## Hands-on: an ellipse through a lens and a drift {#phase-space-lab}

The lab draws the rms ellipse before and after a thin lens (focal length `f` in m; 0 means no lens) followed by a drift of length `L`, and gives the Twiss parameters and rms size at the exit. `beta` is in mm/mrad and `emit` is the normalized rms emittance (π·mm·mrad).

Start at a waist ($\alpha_x = 0$, $\beta_x$ = 1 m) with a lens of 0.5 m focal length followed by a 0.5 m drift. By the transport rules, after the lens $\alpha_x = \beta_x/f = 2$; after the drift $\beta_x = 0.25$ m and $\alpha_x = -0.5$; the new waist is $\alpha_x/\gamma_x$ = 0.4 m behind the lens with $\beta^* = 1/\gamma_x$ = 0.2 m. The rms size changes from 1.87 mm to 0.94 mm, while the area ("ellipse area / π" in the lab) stays 3.504 mm·mrad throughout.

```avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
```

Things to try: vary `L` to find where the ellipse is upright (the waist); vary `emit` and check that the shape stays the same and only the size scales; increase `W` and watch the geometric emittance shrink like $1/(\beta\gamma)$.

The x-plane initial beam of the example project in a 0.1 m drift (a diverging beam: $\alpha_x$ goes from −0.658 to −1.037 and the size grows from 0.954 mm to 1.149 mm):

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.1
```

## Longitudinal phase space {#phase-space-longitudinal}

The longitudinal direction also has a conjugate pair: how far ahead of the reference particle a particle is ($z$), and how large its momentum deviation is ($\delta$). In linacs phase and energy $(\Delta\varphi, \Delta W)$ are more common: an RF cavity "sees" when a particle arrives, not where it is. A particle a distance $z$ ahead of the reference particle arrives $z/(\beta c)$ earlier and sees a smaller RF phase. The conversion between the two sets is (see [Kinematics](#kinematics-rf)):

$$
\Delta\varphi = -\frac{2\pi}{\beta\lambda}\,z, \qquad
\Delta W = \beta^2\gamma\,mc^2\,\delta
$$

so the two geometric emittances differ by a constant factor

$$
\varepsilon_{\Delta\varphi\Delta W} = \frac{2\pi}{\beta\lambda}\,\beta^2\gamma\,mc^2\;\varepsilon_{z\delta}
$$

(with the phase in degrees, replace $2\pi$ by $360$). $\lambda$ is the wavelength of the beam frequency. From the `twissz` parameters of the example `beam.txt`: the longitudinal rms length 1.316 mm corresponds to an rms phase width of 4.51° at 162.5 MHz; $\delta_{\mathrm{rms}} = 3.532\times10^{-3}$ to an rms energy spread of 10.78 keV (0.71 %); the longitudinal geometric rms emittance 4.644 mm·mrad converts to 48.5 deg·keV (whereas direct statistics of the particle file `part_rfq.dst` give 1.24 mm, 4.26°, 11.4 keV and 48.7 deg·keV; the two sets are compared in [Initial distributions: In AVAS](#distributions-in-avas)). Motion in longitudinal phase space — synchronous phase, stable region, longitudinal focusing — is discussed in [Longitudinal motion](#longitudinal).

## Common misconceptions {#phase-space-misconceptions}

- **"The π in π·mm·mrad has to be multiplied in."** It does not. The π is a label saying the number is already the area divided by π. The literature is not consistent on this point; check when exchanging data.
- **"rms and total emittance differ by a fixed factor."** The factor depends on the distribution: 4 for KV, and a Gaussian has no total emittance at all; see [Initial distributions](#distributions).
- **"Emittance is conserved during acceleration."** The normalized emittance is; the geometric emittance shrinks like $1/(\beta\gamma)$. Beams at different energies must be compared by their normalized emittance.
- **"Liouville's theorem keeps the rms emittance constant."** Liouville's theorem conserves the occupied phase-space volume. Filamentation by non-linear forces makes the rms emittance grow while the volume stays the same.
- **"Positive α means diverging."** The opposite: $\alpha_x > 0$ means $\langle x x'\rangle < 0$, a converging beam.
- **"The tilt of the ellipse is a physical quantity."** The tilt depends on the units of $x$ and $x'$ (for example mm with mrad or mm with rad) and changes with them; the Twiss parameters must be converted together with the units.
- **"A smaller beam means a smaller emittance."** The size is $\sqrt{\beta_x\varepsilon}$; focusing changes $\beta_x$, not $\varepsilon$.
- **"A solenoid increased $\varepsilon_x$, so it spoiled the beam."** A solenoid couples $x$ and $y$; the projected emittances can change while the four-dimensional emittance does not, and the coupling can vanish again after the solenoid; see [Solenoid](#solenoid).

## In AVAS {#phase-space-in-avas}

### The twiss keywords

```avas-params
beam:twissx beam:twissy beam:twissz beam:distribution beam:particlenumber beam:initpos beam:initmom
```

The three numbers of `twissx α β ε` (likewise `twissy`, `twissz`) are:

- $\alpha$: dimensionless;
- $\beta$: in mm/mrad, numerically equal to m/rad;
- $\varepsilon$: the **normalized rms emittance**, in π·mm·mrad.

> **AVAS convention**: the rms size is $\sigma_x = \sqrt{\beta_x\,\varepsilon_n/(\beta\gamma)}$ (with $\beta_x$ in mm/mrad and $\varepsilon_n$ in mm·mrad, $\sigma_x$ comes out in mm). `twissz` uses the same units and normalization: $z$ in mm, $z' = \delta = \Delta p/p$ in mrad, normalized by multiplying with $\beta\gamma$ as well (the linear preview of the interface converts this way and has been compared with the kernel). Other codes may give the longitudinal emittance in $(\Delta\varphi, \Delta W)$ or with a different normalization factor; check item by item when exchanging data.

### Initial beam of the example project

The example `beam.txt` (protons at 1.527 MeV, $\beta\gamma$ = 0.057076):

```text
twissx -0.65785 0.37743 0.13768
twissy -0.6341 0.37292 0.14059
twissz 0.04275 0.37292 0.26506
distribution GS GS
```

Converted with the convention above:

| Plane | geometric rms emittance (mm·mrad) | rms size | rms angle / momentum deviation |
|---|---|---|---|
| x | 2.412 | 0.954 mm | 3.026 mrad |
| y | 2.463 | 0.958 mm | 3.043 mrad |
| z | 4.644 | 1.316 mm | $\delta_{\mathrm{rms}}$ = 3.532 × 10$^{-3}$ |

The transverse $\alpha < 0$ means the beam is diverging at the entrance. The longitudinal rms length of 1.316 mm corresponds to an rms phase width of 4.51° at 162.5 MHz; $\delta_{\mathrm{rms}}$ corresponds to an rms energy spread of 10.78 keV (0.71 %); the longitudinal geometric rms emittance converts to 48.5 deg·keV.

### Other keywords

- `distribution`: transverse and longitudinal distribution types (KV, GS, PB, WB); see [Initial distributions: In AVAS](#distributions-in-avas).
- `particlenumber`: number of macro-particles; 1 selects single-particle mode (`twiss*`, `current` and `distribution` then have no effect).
- `initpos`, `initmom`: initial position (mm) and angle (mrad) offset of the whole bunch, used to model injection errors.
- `readparticledistribution`: read the particle distribution from a `.dst`/`.edst` file; every keyword except `numofcharge` is then ignored (the interface switches with `use_dst`).

In the output, columns 16, 18, 20 of `DataSet.txt` are the rms sizes of $x$, $y$, $z$ (in m), directly corresponding to the $\sigma$ above; the reference-particle kinetic energy in column 0 gives $\beta\gamma$ at every position, to turn normalized into geometric emittances.

## Further reading {#phase-space-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: emittance, Liouville's theorem, KV and other distributions, rms-equivalent beams.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: transverse dynamics and Twiss parameters.
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1101: the introduction of the rms emittance concept.
- K. Floettmann, "Some basic features of the beam emittance", *Phys. Rev. ST Accel. Beams* 6, 034202 (2003): normalized emittance and energy spread.
- Continue with: [Initial distributions](#distributions), [Linear optics](#linear-optics).
