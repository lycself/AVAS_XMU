# Phase space and emittance

A beam consists of a large number of particles, and the natural language to describe it is phase space: every particle is a point in phase space and the beam is a cloud of points. This chapter defines the six-dimensional coordinates used by AVAS, introduces Liouville's theorem, emittance, the Twiss (Courant–Snyder) parameters and the σ-matrix, gives the relation between the edge and rms emittances of common beam distributions, and finally maps these quantities term by term onto `twissx`, `twissy`, `twissz` of `beam.txt` (in particular their units).

Notation: in this chapter $\beta$ and $\gamma$ without subscript are the relativistic factors (see [Relativistic kinematics](#kinematics)); $\beta_x$, $\alpha_x$, $\gamma_x$ with a plane subscript are Twiss parameters.

## Coordinates and the reference particle {#phase-space-coordinates}

The design orbit of the reference particle is the reference, and $s$ is the distance along it (on a straight section simply the longitudinal position $z$). The deviation of any particle from the reference particle is described by six quantities:

| Coordinate | Meaning | Unit (AVAS internal / matrices) |
|---|---|---|
| $x$, $y$ | transverse position deviation | m |
| $x' = \dd x/\dd s$, $y' = \dd y/\dd s$ | transverse angle (slope) | rad |
| $z$ | longitudinal position deviation, positive ahead of the reference particle | m |
| $\delta = \Delta p/p$ | relative momentum deviation | 1 |

In the paraxial approximation $x' \approx p_x/p$, where $p$ is the reference momentum. Transfer matrices act on the column vector $(x, x', y, y', z, \delta)\tp$, see [Linear optics](#linear-optics).

Two descriptions must be kept apart:

- the distribution **at the same time**: the t-code (field-map elements, drifts) advances in time, and at a given time the particles sit at different longitudinal positions;
- the distribution **at the same longitudinal position**: the z-code (matrix elements) and transfer-matrix theory compare all particles at the same $s$.

For rms quantities the difference is usually small, but it matters for long bunches with a large energy spread (for example through the $z$–$\delta$ correlation). According to the manual, `DataSet.txt` records the bunch parameters in the s frame.

## Liouville's theorem and adiabatic damping {#phase-space-liouville}

### Liouville's theorem

If the motion of the particles can be described by a Hamiltonian (external electromagnetic fields, and space charge in the mean-field approximation), the phase-space density in the six-dimensional phase space of canonical coordinates $(x, p_x, y, p_y, z, p_z)$ is constant along particle trajectories, and the phase-space volume occupied by any group of particles is conserved. If the three directions are uncoupled, the area of each two-dimensional projection is conserved separately.

Liouville's theorem does not guarantee that the "effective" emittance is conserved: non-linear forces (field non-linearities, non-linear space charge, the sinusoidal shape of the RF field) distort an ellipse in phase space into an S shape or a spiral. The occupied area stays the same, but the area of the ellipse needed to enclose it — and the rms emittance — grow. This is called **filamentation**. Particle losses, collisions, stripping and similar processes violate the premises of Liouville's theorem.

### Adiabatic damping and normalised emittance

The angle $x' = p_x/p$ is not a canonical momentum. Longitudinal acceleration increases $p$ without changing $p_x$, so $x'$ shrinks as $1/p$ and the area in the $(x, x')$ plane shrinks as $1/(\beta\gamma)$; this is **adiabatic damping**. The area in the $(x, p_x/mc) = (x, \beta\gamma x')$ plane is conserved. One therefore defines

$$
\varepsilon_n = \beta\gamma\,\varepsilon
$$

$\varepsilon$ is called the geometric emittance and $\varepsilon_n$ the normalised emittance. In the ideal linear, uncoupled case without filamentation $\varepsilon_n$ stays constant along a linac, which makes it the right figure of merit for comparing beams at different energies. Example: accelerating protons from 1.527 MeV to 10 MeV raises $\beta\gamma$ from 0.05708 to 0.14639, and the geometric emittance shrinks by a factor of 2.565.

> Note: the formula uses the $\beta\gamma$ of the reference particle. For beams with a very large energy spread, the normalised emittance computed with each particle's own $\beta\gamma$ differs from this, and the difference can be significant for low-energy electron beams.

## Definitions of emittance {#phase-space-emittance}

### rms emittance

For $N$ particles write the centred second moments $\langle x^2\rangle = \frac1N\sum (x - \bar x)^2$, and likewise $\langle x x'\rangle$, $\langle x'^2\rangle$. The rms emittance is

$$
\varepsilon_{\mathrm{rms}} = \sqrt{\langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2}
$$

It is the area of the "rms ellipse" in phase space divided by $\pi$. The rms emittance has a well-defined value for any distribution and is strictly conserved under linear transport (without coupling); it grows under non-linear forces, so rms-emittance growth is the usual measure of beam-quality degradation.

### Total and percentage emittances

$\varepsilon_{100\%}$ (total or edge emittance) is the area of the smallest ellipse containing all particles divided by $\pi$; $\varepsilon_{90\%}$, $\varepsilon_{99\%}$ etc. contain the corresponding fraction of particles. For ideal distributions they have fixed ratios to the rms emittance, see [Beam distributions](#phase-space-distributions).

### Units

Emittance has the dimension length × angle. It is customarily written in π·mm·mrad, where the "π" is only a label indicating that the number is the area divided by π:

$$
1\ \pi\cdot\mathrm{mm\cdot mrad} \;\to\; \varepsilon = 1\ \mathrm{mm\cdot mrad} = 1\ \mu\mathrm{m} = 10^{-6}\ \mathrm{m\cdot rad}
$$

When quoting an emittance always state three things: rms or total (percentage), geometric or normalised, and whether π has already been divided out of the number.

## Twiss parameters and the Courant–Snyder invariant {#phase-space-twiss}

A phase-space ellipse centred on the origin can be written as

$$
\gamma_x x^2 + 2\alpha_x x x' + \beta_x x'^2 = \varepsilon, \qquad \beta_x\gamma_x - \alpha_x^2 = 1
$$

$\alpha_x$, $\beta_x$, $\gamma_x$ are the Twiss or Courant–Snyder parameters; the ellipse area is $\pi\varepsilon$. Only two of the three parameters are independent. For the rms ellipse they are related to the second moments by

$$
\langle x^2\rangle = \beta_x\varepsilon, \qquad
\langle x x'\rangle = -\alpha_x\varepsilon, \qquad
\langle x'^2\rangle = \gamma_x\varepsilon
$$

Conversely, the Twiss parameters of a particle distribution are $\beta_x = \langle x^2\rangle/\varepsilon_{\mathrm{rms}}$ and $\alpha_x = -\langle x x'\rangle/\varepsilon_{\mathrm{rms}}$.

### Geometry of the ellipse

- maximum position $x_{\max} = \sqrt{\varepsilon\beta_x}$, maximum angle $x'_{\max} = \sqrt{\varepsilon\gamma_x}$;
- intercept with the $x$ axis $x = \sqrt{\varepsilon/\gamma_x}$, with the $x'$ axis $x' = \sqrt{\varepsilon/\beta_x}$;
- the tilt $\theta$ of the major axis against the $x$ axis satisfies $\tan 2\theta = 2\alpha_x/(\gamma_x - \beta_x)$.

The sign of $\alpha_x$ tells whether the beam converges or diverges:

| $\alpha_x$ | $\langle x x'\rangle$ | State |
|---|---|---|
| $\alpha_x > 0$ | $< 0$ | converging (moving towards a waist) |
| $\alpha_x = 0$ | $= 0$ | waist (upright ellipse) |
| $\alpha_x < 0$ | $> 0$ | diverging (moving away from a waist) |

### Courant–Snyder invariant

In a linear focusing system the Twiss parameters vary with $s$ (following the transport rules in [Linear optics](#linear-optics-twiss-transport)), while the single-particle quantity

$$
J = \gamma_x(s)\,x^2 + 2\alpha_x(s)\,x x' + \beta_x(s)\,x'^2
$$

stays constant along the beamline; it is called the Courant–Snyder invariant. A particle thus always lies on one member of a family of ellipses whose shape changes but whose area does not. The trajectory can be written $x(s) = \sqrt{J\beta_x(s)}\cos\left(\psi(s) + \psi_0\right)$, where $\psi$ is the betatron phase, see [Linear optics](#linear-optics-phase-advance).

## The σ-matrix and the phase ellipse {#phase-space-sigma}

Arrange the second moments in a symmetric matrix (the beam matrix or σ-matrix):

$$
\Sigma = \begin{pmatrix} \langle x^2\rangle & \langle x x'\rangle \\ \langle x x'\rangle & \langle x'^2\rangle \end{pmatrix}
= \varepsilon \begin{pmatrix} \beta_x & -\alpha_x \\ -\alpha_x & \gamma_x \end{pmatrix},
\qquad \varepsilon = \sqrt{\det\Sigma}
$$

The rms ellipse is $X\tp\Sigma^{-1}X = 1$ with $X = (x, x')\tp$. The advantage of the σ-matrix is its very simple transformation under linear transport $X_1 = M X_0$:

$$
\Sigma_1 = M\,\Sigma_0\,M\tp
$$

This holds for any distribution (not only elliptical ones), since it involves only second moments. AVAS's linear preview pushes the beam in exactly this way.

### The six-dimensional σ-matrix

In six dimensions $\Sigma_{ij} = \langle u_i u_j\rangle$ with $u = (x, x', y, y', z, \delta)$. The diagonal blocks give the projected emittances of the three planes, the off-diagonal blocks describe correlations between planes:

- dipoles create $x$–$\delta$ correlations (dispersion), see [Dipole](#dipole);
- solenoids couple the $x$ and $y$ planes; the projected emittances $\varepsilon_x$, $\varepsilon_y$ can then change, but the four-dimensional emittance $\sqrt{\det\Sigma_4}$ is conserved under linear symplectic maps, see [Solenoid](#solenoid);
- RF cavities create $z$–$\delta$ correlations, see [Longitudinal motion](#longitudinal).

## Beam distributions and edge emittance {#phase-space-distributions}

`distribution V1 V2` in `beam.txt` selects the transverse and the longitudinal distribution respectively, each one of KV, GS, PB, WB:

| Code | Distribution | Density in 4-D phase space ($r^2$ is the normalised "radius" squared) | $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}}$ (4-D distribution projected onto $x$–$x'$) |
|---|---|---|---|
| KV | Kapchinskij–Vladimirskij | concentrated on the 4-D ellipsoid surface $r^2 = 1$ | 4 |
| WB | waterbag | uniform inside $r^2 \le 1$ | 6 |
| PB | parabolic | $\propto 1 - r^2$, $r^2 \le 1$ | 8 |
| GS | Gaussian | $\propto \exp(-r^2/2)$ | unbounded (needs truncation) |

The ratios follow from a general result: projecting a uniform distribution inside a $d$-dimensional ellipsoid onto one coordinate gives $\langle x^2\rangle = a^2/(d+2)$, with $a$ the semi-axis. Some remarks:

- The **KV distribution** projects onto a uniform ellipse in real space $(x, y)$; its space-charge force is exactly linear, which makes it the reference distribution of envelope equations and space-charge theory (see [Space charge](#space-charge)). Its phase-space structure is very special, however, and does not occur in real beams.
- **WB** and **PB** are closer to real beams and have a sharp edge; **GS** has the longest tails.
- For a two-dimensional Gaussian, the fraction of particles inside the rms-similar ellipse of emittance $n\,\varepsilon_{\mathrm{rms}}$ is $1 - \ee^{-n/2}$: 39.3 %, 86.5 %, 95.0 %, 98.2 % for $n$ = 1, 4, 6, 8.

> Note: the table gives the standard results for ideal four-dimensional distributions (Lapostolle, Reiser). How AVAS generates the longitudinal distribution and where it truncates the Gaussian is not described in the manual; if needed, read the `inData.dst` of a run (the initial distribution actually used) and compute the statistics directly.

## Longitudinal phase space {#phase-space-longitudinal}

The longitudinal coordinates $(z, \delta)$ convert to the $(\Delta\varphi, \Delta W)$ commonly used in linacs as (see [Kinematics](#kinematics-rf)):

$$
\Delta\varphi = -\frac{2\pi}{\beta\lambda}\,z, \qquad
\Delta W = \beta^2\gamma\,mc^2\,\delta
$$

so the two geometric emittances differ by a constant factor,

$$
\varepsilon_{\Delta\varphi\Delta W} = \frac{2\pi}{\beta\lambda}\,\beta^2\gamma\,mc^2\;\varepsilon_{z\delta}
$$

(replace $2\pi$ by $360$ for the phase in degrees). $\lambda$ is the wavelength of the beam frequency. Motion in longitudinal phase space — synchronous phase, the stable region, longitudinal focusing — is discussed in [Longitudinal motion](#longitudinal).

## In AVAS {#phase-space-in-avas}

```avas-params
beam:twissx beam:twissy beam:twissz beam:distribution beam:particlenumber beam:initpos beam:initmom
```

The three numbers of `twissx α β ε` (likewise `twissy`, `twissz`) mean:

- $\alpha$: dimensionless;
- $\beta$: in mm/mrad, numerically equal to m/rad;
- $\varepsilon$: the **normalised rms emittance** in π·mm·mrad.

> AVAS convention: the rms size is $\sigma_x = \sqrt{\beta_x\,\varepsilon_n/(\beta\gamma)}$ ($\sigma_x$ in mm for $\beta_x$ in mm/mrad and $\varepsilon_n$ in mm·mrad). `twissz` uses the same units and normalisation: $z$ in mm, $z' = \delta = \Delta p/p$ in mrad, normalised with the same factor $\beta\gamma$ (the interface's linear preview converts with this convention, which has been compared with the kernel). Other codes may give the longitudinal emittance in $(\Delta\varphi, \Delta W)$ or with a different normalisation factor; check every item when exchanging data.

### Initial beam of the example project

The example `beam.txt` (protons at 1.527 MeV, $\beta\gamma$ = 0.057076):

```text
twissx -0.65785 0.37743 0.13768
twissy -0.6341 0.37292 0.14059
twissz 0.04275 0.37292 0.26506
distribution GS GS
```

Converted with the convention above:

| Plane | Geometric rms emittance (mm·mrad) | rms size | rms angle / momentum deviation |
|---|---|---|---|
| x | 2.412 | 0.954 mm | 3.026 mrad |
| y | 2.463 | 0.958 mm | 3.043 mrad |
| z | 4.644 | 1.316 mm | $\delta_{\mathrm{rms}}$ = 3.532 × 10$^{-3}$ |

The transverse $\alpha < 0$ means the beam is diverging at the entrance. The longitudinal rms length of 1.316 mm corresponds to an rms phase width of 4.51° at 162.5 MHz; $\delta_{\mathrm{rms}}$ corresponds to an rms energy spread of 10.78 keV (0.71 %); the longitudinal geometric rms emittance converts to 48.5 deg·keV.

### Other keywords

- `particlenumber`: number of macro-particles; 1 selects single-particle mode (then `twiss*`, `current`, `distribution` have no effect).
- `initpos`, `initmom`: initial position (mm) and angle (mrad) offsets of the whole bunch, used to simulate injection errors.
- `readparticledistribution`: read the particle distribution from a `.dst`/`.edst` file; every keyword except `numofcharge` is then ignored (the interface switches with `use_dst`).

In the output, columns 16, 18, 20 of `DataSet.txt` are the rms sizes of $x$, $y$, $z$ (in m), corresponding directly to the $\sigma$ above.

## Hands-on lab {#phase-space-lab}

The lab draws the rms ellipse before and after a thin lens (focal length `f` in m; 0 means no lens) followed by a drift of length `L`, and gives the Twiss parameters and rms size at the exit. `beta` is in mm/mrad and `emit` is the normalised rms emittance (π·mm·mrad).

1. Start at a waist ($\alpha_x = 0$, $\beta_x$ = 1 m), pass a lens of focal length 0.5 m and drift 0.5 m. By the formulas of [Linear optics](#linear-optics-twiss-transport), after the lens $\alpha_x = \beta_x/f = 2$, after the drift $\beta_x = 0.25$ m and $\alpha_x = -0.5$; the new waist lies $\alpha_x/\gamma_x$ = 0.4 m behind the lens with $\beta^* = 1/\gamma_x$ = 0.2 m. The rms size goes from 1.87 mm to 0.94 mm.
2. Change `L` to find where the ellipse is upright (the waist); change `emit` and confirm that the ellipse keeps its shape and is only scaled.
3. Increase `W`: the geometric emittance shrinks as $1/(\beta\gamma)$.

```avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
```

The initial x beam of the example project in a 0.1 m drift (a diverging beam, the size grows):

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.1
```

## Further reading {#phase-space-further}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.: emittance, Liouville's theorem, the KV and other distributions, rms-equivalent beams.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: transverse dynamics and Twiss parameters.
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1101: the introduction of the rms emittance concept.
- K. Floettmann, "Some basic features of the beam emittance", *Phys. Rev. ST Accel. Beams* 6, 034202 (2003): normalised emittance and energy spread.
- Continue with [Linear optics](#linear-optics).
