# Longitudinal dynamics

RF cavities give the particles energy and at the same time decide whether the bunch stays together longitudinally (in time / phase). This chapter starts from the energy gain in one accelerating gap, introduces the transit-time factor and multi-gap cavities, and then treats the synchronous particle, phase stability, small-amplitude synchrotron oscillations, the longitudinal acceptance, adiabatic damping and the RF defocusing that necessarily accompanies longitudinal focusing. It ends with the three definitions of the RF phase in AVAS (parameter V3 of `field`) and the related keywords. Kinematic quantities ($\beta$, $\gamma$, $\beta\lambda$) are defined in [Relativistic kinematics](#kinematics), phase space and emittance in [Phase space and emittance](#phase-space), and the cavity element itself in [RF cavities](#rf-cavity).

All numerical examples use the example project `examples/hwr010`: protons, entry kinetic energy $W = 1.527\ \mathrm{MeV}$ ($\beta = 0.05698$, $\gamma = 1.00163$), RF frequency $f = 162.5\ \mathrm{MHz}$ ($\lambda = c/f = 1.8449\ \mathrm{m}$, $\beta\lambda = 0.1051\ \mathrm{m}$), half-wave resonator (HWR) field map `hwr010`, 0.21 m long, `Ke = 1.36`.

## Energy gain in an RF gap {#longitudinal-gap}

### Field and phase

Let the longitudinal on-axis field in the cavity be a standing wave

$$
E_z(z, t) = E(z)\cos\left(\omega t + \varphi_0\right),
$$

where $E(z)$ is the spatial profile and $\omega = 2\pi f$. A particle moving along the axis reaches $z$ at time $t(z)$ and "sees" the phase

$$
\varphi(z) = \omega t(z) + \varphi_0 .
$$

In this convention **a larger phase means a later arrival**. The energy gained in the gap is

$$
\Delta W = q\int E(z)\cos\varphi(z)\,\dd z .
\tag{1}
$$

> AVAS convention: the RF electric field acts as $K_e\,E(\boldsymbol r)\cos(\omega t + \varphi_0)$ and the RF magnetic field as $K_b\,B(\boldsymbol r)\sin(\omega t + \varphi_0)$ ($E$ in MV/m, $B$ in T in the map). This is the cosine convention of Wangler's *RF Linear Accelerators*. The longitudinal coordinate is related to the phase by $\Delta\varphi = -2\pi z/(\beta\lambda)$: a particle with $z > 0$ is ahead of the reference particle and has a smaller phase.

### Panofsky formula and transit-time factor

If the velocity hardly changes in the gap ($\Delta W \ll W$), $t(z)$ can be taken as uniform motion: $\omega t(z) = \omega t_c + k (z - z_c)$ with $k = 2\pi/(\beta\lambda)$ and $z_c$ a reference point of the gap. Inserting into (1):

$$
\Delta W = q\cos\varphi\int E(z)\cos k(z - z_c)\,\dd z - q\sin\varphi\int E(z)\sin k(z - z_c)\,\dd z ,
$$

where $\varphi = \omega t_c + \varphi_0$ is the phase at $z_c$. Choosing $z_c$ at the **electrical centre**, where the second integral vanishes (the geometric centre of a symmetric gap), gives the Panofsky formula

$$
\Delta W = q E_0 T L\cos\varphi = q V_0 T\cos\varphi ,
\qquad
T = \frac{\int E(z)\cos k(z - z_c)\,\dd z}{\int E(z)\,\dd z},
\tag{2}
$$

with $E_0 = V_0/L$ the average on-axis field over the gap length $L$ and $V_0 = \int E\,\dd z$ the on-axis voltage. The **transit-time factor** $T \le 1$ accounts for the field changing while the particle crosses the gap. For a uniform field over a gap of length $g$,

$$
T = \frac{\sin(\pi g/\beta\lambda)}{\pi g/\beta\lambda},
$$

which approaches 1 the shorter the gap is compared with $\beta\lambda$. Real cavity fields extend beyond the gap, so $T$ has to be computed from the field profile.

### Exact definition: phase along the actual trajectory

When the energy gain is comparable with the entry energy (low-energy protons, heavy ions) uniform motion is no longer a good approximation. Equation (1) is then written as

$$
C = \int E(z)\cos\varphi(z)\,\dd z,\quad S = \int E(z)\sin\varphi(z)\,\dd z,\quad
\Delta W = q\sqrt{C^2 + S^2}\,\cos\varphi_s,\quad \varphi_s = \operatorname{atan2}(S, C),
\tag{3}
$$

with the integrals taken along the **actual** trajectory $t(z)$ of the reference particle. Equation (3) is exact and $\varphi_s$ is what AVAS calls the synchronous phase (see [below](#longitudinal-synchronous)); but the "effective voltage" $\sqrt{C^2 + S^2}$ itself depends on the trajectory, hence on $\varphi_s$, so the energy gain no longer follows $\cos\varphi_s$ exactly.

## Transit-time factor and multi-gap cavities {#longitudinal-multigap}

### Two-gap cavities

The accelerating part of a half-wave resonator (HWR) or quarter-wave resonator (QWR) consists of two gaps with opposite field directions ($\pi$ mode). With $e(z)$ the field of one gap and $d$ the distance of the gap centres, $E(z) = e(z - d/2) - e(z + d/2)$. Its Fourier transform gives

$$
T(\beta) = T_g(\beta)\,\left\lvert\sin\frac{k d}{2}\right\rvert = T_g(\beta)\,\left\lvert\sin\frac{\pi\beta_g}{2\beta}\right\rvert,
\qquad d = \frac{\beta_g\lambda}{2},
$$

$T_g$ being the transit-time factor of a single gap. For a multi-gap field of alternating sign $\int E\,\dd z \approx 0$, so the denominator of (2) is replaced by $V_0 = \int\lvert E\rvert\,\dd z$ (as in the table below). When the particle needs exactly half an RF period from one gap to the next ($\beta = \beta_g$), both gaps accelerate in phase. A cavity with $N$ gaps has a sharper $T(\beta)$ and a narrower energy acceptance; low-$\beta$ superconducting linacs therefore use cavities with few gaps, in several geometric $\beta$ families.

### The example cavity hwr010

Integrating the on-axis $E_z$ of the `hwr010` map (times `Ke = 1.36`) gives:

| Quantity | Value |
|---|---|
| Peak on-axis field | $+13.03\ \mathrm{MV/m}$ and $-12.74\ \mathrm{MV/m}$ |
| Sign change of the field | $z \approx 0.104\ \mathrm{m}$ (two gaps) |
| Field centroids of the two gaps | $0.064\ \mathrm{m}$, $0.145\ \mathrm{m}$, spacing $d = 0.081\ \mathrm{m}$ |
| $\beta_g = 2d/\lambda$ from the spacing | 0.088 |
| $V_0 = \int\lvert E_z\rvert\,\dd z$ | 0.978 MV |
| Maximum of $T(\beta)$ | 0.82 at $\beta \approx 0.104$ |
| $T$ at the entry $\beta = 0.0570$ | 0.40, $V_0 T = 0.39\ \mathrm{MV}$ |

The maximum of $T(\beta)$ at $\beta \approx 0.10$ matches the "010" in the name (usually meaning a design $\beta = 0.10$); it is a little above the 0.088 estimated from the spacing because the single-gap factor $T_g(\beta)$ grows with $\beta$. The example beam has $\beta = 0.057$, far below the optimum, so $T$ is only 0.40; as the particle is accelerated inside the cavity $T$ grows, which is one reason why the uniform-motion approximation fails in low-energy cavities.

```avas-field
map hwr010
```

### Energy gain versus synchronous phase: example

Exit energies from the AVAS linear preview (`avas.sim.linear_optics`, lattice `field 0.21 0.02 0 1 162.5e6 φs 1.36 -1.36 hwr010`):

| $\varphi_s$ (V3 = 0) | Entry phase | Exit energy / MeV | $\Delta W$ / MeV |
|---|---|---|---|
| −90° | −14.0° | 1.5270 | 0.000 |
| −60° | 18.6° | 1.6943 | 0.167 |
| −33° | 52.7° | 1.8665 | 0.340 |
| −20° | 69.7° | 1.9374 | 0.410 |
| 0° | 95.4° | 2.0023 | 0.475 |
| +20° | 119.7° | 1.9954 | 0.468 |

At $\varphi_s = -90°$ the gain vanishes, as (3) requires. But $\Delta W/\cos\varphi_s$ is 0.405 MeV at −33° and 0.475 MeV at 0°, about 15 % apart, and in a scan of the entry phase the largest gain (2.0087 MeV) occurs at $\varphi_s \approx +8°$, not at 0°. The reason is the one given above: $\Delta W/W \approx 30\ \%$, and $\beta$ grows from 0.057 to about 0.065 inside the cavity, so the effective voltage changes with the phase. For such cavities the Panofsky formula is only an estimate; designs must rely on the integration along the trajectory.

## Synchronous particle and synchronous phase {#longitudinal-synchronous}

The **synchronous particle** (reference particle) is the particle that by design moves along the axis and meets every cavity at its design phase; the longitudinal coordinates of all other particles are defined relative to it. In a linac made of many cavities, the phase $\varphi_{s,n}$ of the synchronous particle in cavity $n$ is set by the RF phase of that cavity together with the flight time through everything upstream.

### The three meanings of the phase in AVAS (V3)

The third parameter V3 of `field L R V3 type f phase Ke Kb file` sets the meaning of the sixth parameter `phase`:

| V3 | Meaning of `phase` | How the kernel sets the entry phase $\varphi_0$ |
|---|---|---|
| 0 | synchronous phase $\varphi_s$ | solves for $\varphi_0$ such that $\operatorname{atan2}(S, C) = \varphi_s$ along the reference trajectory, eq. (3) |
| 1 | RF phase when the reference particle reaches the element entry | $\varphi_0$ = `phase` |
| 2 | RF phase at $t = 0$ (absolute phase) | $\varphi_0 = $ `phase` $+ 360°\cdot f\cdot t_{\mathrm{in}}$, $t_{\mathrm{in}}$ the arrival time of the reference particle at the entry |

V3 = 0 is the most convenient for design: the physically meaningful quantity is the synchronous phase, and the program solves for the entry phase. V3 = 1 suits phase scans (the entry phase is tied directly to the cavity geometry). V3 = 2 corresponds to a real machine, where each cavity's RF is phased against a common reference clock; changing anything upstream changes the synchronous phase.

Example: put a 0.1 m drift before the example cavity. The reference particle reaches the cavity at $t_{\mathrm{in}} = 0.1/(\beta c) = 5.854\ \mathrm{ns}$, so $360°\,f\,t_{\mathrm{in}} = 342.45°$. With V3 = 2 and `phase = 0` the entry phase is $342.45° \equiv -17.55°$, which corresponds to a synchronous phase of about −93.7°: the cavity hardly accelerates. With V3 = 0 and `phase = -33`, the entry phase is 52.7° whatever the length of the drift, and the synchronous phase stays at −33°.

> AVAS convention: in the example, the synchronous phase $-33°$ corresponds to the entry phase $52.7°$; the difference (about 86°) depends on where the field map starts and on its field profile, and there is no general conversion. For V3 = 0 cavities the linear preview reports both the solved entry phase and the synchronous phase. When a segment of the lattice is run on its own, AVAS (`rephase` in `avas/data/segment.py`) takes the RF phase $\varphi_{\mathrm{RF}}$ of every cavity from the synchronous-particle data of the reference run and converts it with $\varphi_{\mathrm{RF}} - 360°\cdot f\cdot(t_{\mathrm{in}} - T_{\mathrm{entry}})$ into a V3 = 2 phase whose clock starts at the segment entry, instead of keeping the original phases or switching to V3 = 0.

## Phase stability {#longitudinal-stability}

### Equations of motion

Consider a periodic array of cavities and average their action over unit length: average accelerating gradient $E_0 T$ (the in-cavity $E_0 T$ times the fraction of the period occupied by the cavity). Define the deviations from the synchronous particle

$$
\Delta\varphi = \varphi - \varphi_s,\qquad w = \frac{W - W_s}{mc^2} .
$$

From $\dd t/\dd s = 1/(\beta c)$ and $\Delta\beta = \Delta\gamma/(\beta\gamma^3)$:

$$
\frac{\dd\,\Delta\varphi}{\dd s} = \omega\left(\frac{1}{\beta c} - \frac{1}{\beta_s c}\right) \approx -\frac{2\pi}{\beta_s^3\gamma_s^3\lambda}\,w \equiv -A\,w ,
\tag{4}
$$

and the energy equation

$$
\frac{\dd w}{\dd s} = \frac{q E_0 T}{mc^2}\left(\cos\varphi - \cos\varphi_s\right) \equiv B\left(\cos\varphi - \cos\varphi_s\right).
\tag{5}
$$

Equation (4) says that a particle with more energy ($w > 0$) is faster and its phase decreases (it arrives earlier).

### Stability condition

Linearising (4) and (5) with $\cos\varphi - \cos\varphi_s \approx -\sin\varphi_s\,\Delta\varphi$:

$$
\frac{\dd^2\Delta\varphi}{\dd s^2} = A B \sin\varphi_s\,\Delta\varphi .
$$

Since $A, B > 0$, **the motion is stable only for $\sin\varphi_s < 0$**. Together with $\cos\varphi_s > 0$ (acceleration), the stable accelerating range is

$$
-90° < \varphi_s < 0° .
$$

Physical picture: a late particle ($\Delta\varphi > 0$) sits on the rising side of the field, gains more energy, becomes faster and catches up; an early particle gains less and falls back.

> Note: sign conventions differ between texts. In the cosine convention of AVAS and Wangler (larger phase = later) the stable synchronous phase is negative; the example project uses $-33°$. With a sine convention, or with the phase defined as $-\omega t$, the stable range becomes positive. Always check the convention when taking phases from other codes or papers.

### Checking the sign with the AVAS linear preview

The example beam (initial rms $z$ = 1.316 mm, rms $x$ = 0.954 mm) through one hwr010 cavity, rms sizes at the exit (linear preview, space charge off):

| Case | rms $z$ / mm | rms $x$ / mm |
|---|---|---|
| 0.21 m drift (no cavity) | 1.482 | 1.407 |
| $\varphi_s = -33°$ | 0.702 | 1.811 |
| $\varphi_s = +33°$ | 1.738 | 1.196 |

A negative synchronous phase compresses the bunch longitudinally (bunching), a positive one lengthens it (debunching), in agreement with the stability condition; transversely it is the other way round, which is the [RF defocusing](#longitudinal-rf-defocusing). Drag the phase in the lab below to see it yourself.

```avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
```

## Small-amplitude synchrotron oscillations and adiabatic damping {#longitudinal-synchrotron}

### Longitudinal wave number

In the stable case $\Delta\varphi$ oscillates harmonically, $\Delta\varphi'' + k_{l0}^2\,\Delta\varphi = 0$, with the **zero-current longitudinal wave number**

$$
k_{l0}^2 = -A B\sin\varphi_s = \frac{2\pi\, q E_0 T\sin(-\varphi_s)}{mc^2\,\beta_s^3\gamma_s^3\,\lambda} .
\tag{6}
$$

The synchrotron wavelength is $\lambda_l = 2\pi/k_{l0}$ and the longitudinal phase advance per focusing period $L_p$ is $\sigma_{0l} = k_{l0}L_p$. From (4), the ratio of the energy and phase amplitudes is $w_{\max}/\Delta\varphi_{\max} = k_{l0}/A$. The factor $\beta^3\gamma^3$ in (6) shows that longitudinal focusing weakens quickly with energy; synchrotron oscillations are fastest at low energy.

Numerical example: if an array of cavities had an average gradient equal to the example cavity's $V_0 T / L = 0.39\ \mathrm{MV}/0.21\ \mathrm{m} = 1.86\ \mathrm{MV/m}$ at the entry, with $\varphi_s = -33°$, then $k_{l0} = 4.44\ \mathrm{m}^{-1}$ and $\lambda_l = 1.41\ \mathrm{m}$. In a real structure solenoids and drifts sit between the cavities, the average gradient is lower and $k_{l0}$ smaller ($\propto\sqrt{E_0 T}$).

### Conversion to the AVAS longitudinal coordinates

AVAS uses the longitudinal phase space $(z, \delta)$ with $\delta = \Delta p/p$:

$$
z = -\frac{\beta\lambda}{2\pi}\,\Delta\varphi,\qquad
\delta = \frac{\Delta W}{\beta^2\gamma\,mc^2} = \frac{w}{\beta^2\gamma} .
$$

So $z$ obeys the same oscillation equation as $\Delta\varphi$. The `twissz` keyword of `beam.txt` uses $z$ (mm) and $\delta$ (mrad), and its emittance is again the normalised rms value. The example beam has `twissz 0.04275 0.37292 0.26506`, i.e. initial rms $z = 1.316\ \mathrm{mm}$ and rms $\delta = 3.53\ \mathrm{mrad}$; for the meaning of the Twiss parameters see [Phase space and emittance](#phase-space).

### Adiabatic damping

When acceleration changes $\beta\gamma$ and $E_0 T$ slowly, $(\Delta\varphi, w)$ remain a canonical pair (Hamiltonian in the [next section](#longitudinal-separatrix)) and the action $\oint w\,\dd\Delta\varphi \propto \Delta\varphi_{\max} w_{\max}$ is an adiabatic invariant. With $w_{\max}/\Delta\varphi_{\max} = k_{l0}/A$ this gives

$$
\Delta\varphi_{\max} \propto \left[E_0 T\sin(-\varphi_s)\,\lambda\,\beta_s^3\gamma_s^3\right]^{-1/4},\qquad
\Delta W_{\max} \propto \left[E_0 T\sin(-\varphi_s)\,\lambda\,\beta_s^3\gamma_s^3\right]^{1/4}.
$$

For constant gradient, phase and frequency, $\Delta\varphi \propto (\beta\gamma)^{-3/4}$ and $\Delta W \propto (\beta\gamma)^{3/4}$: the phase width shrinks, the energy spread grows, and their product is constant. In $(z, \delta)$, $\beta\gamma\cdot z\,\delta \propto \lambda\,\Delta\varphi\,\Delta W$ is constant, i.e. **the normalised longitudinal emittance is conserved**, as in the transverse planes (geometric emittance $\propto 1/\beta\gamma$), see [Phase space and emittance](#phase-space).

> Note: this conservation holds only for linear, uncoupled motion without space charge. The longitudinal non-linearity in low-energy cavities (the curvature of $\cos\varphi$ once the phase width reaches tens of degrees) makes the longitudinal rms emittance grow; the linear preview does not contain this effect, it has to be computed with the multi-particle kernel.

## Longitudinal acceptance and separatrix {#longitudinal-separatrix}

### Hamiltonian

Equations (4) and (5) follow from the Hamiltonian (independent variable $s$, $\varphi$ and $w$ conjugate)

$$
H(\varphi, w) = -\frac{A w^2}{2} - B\left(\sin\varphi - \varphi\cos\varphi_s\right)
$$

via $\varphi' = \partial H/\partial w$ and $w' = -\partial H/\partial\varphi$. $H$ is conserved, so the phase-space trajectories are its contour lines.

### Fixed points and separatrix

$w = 0$ and $\cos\varphi = \cos\varphi_s$ give two fixed points: $\varphi = \varphi_s$ (stable, elliptic) and $\varphi_2 = -\varphi_s$ (unstable, hyperbolic). The contour through $\varphi_2$ is the **separatrix**:

$$
\frac{A w^2}{2} + B\left(\sin\varphi - \varphi\cos\varphi_s\right) = -B\left(\sin\varphi_s - \varphi_s\cos\varphi_s\right).
$$

Particles inside the separatrix perform bounded synchrotron oscillations and form the longitudinal **acceptance** (the "fish" or "golf-club" shaped stable region); particles outside slip continuously in phase, leave the accelerating phase and become longitudinal losses (in AVAS, `longlimits` sets phase and energy limits for counting them).

### Width and height of the stable region

- The phase range extends from $\varphi_1$ to $\varphi_2 = -\varphi_s$, with $\varphi_1$ from $\sin\varphi_1 - \varphi_1\cos\varphi_s = -\sin\varphi_s + \varphi_s\cos\varphi_s$. For small $\lvert\varphi_s\rvert$ the total width is $\psi \approx 3\lvert\varphi_s\rvert$.
- The energy half-height, at $\varphi = \varphi_s$, is

$$
\Delta W_{\max} = 2\left[\frac{q E_0 T\,\beta_s^3\gamma_s^3\,\lambda\, mc^2\left(\varphi_s\cos\varphi_s - \sin\varphi_s\right)}{2\pi}\right]^{1/2}.
$$

With the average gradient 1.86 MV/m of the previous section and the example beam energy:

| $\varphi_s$ | $\varphi_1$ | Phase width | $\Delta W_{\max}$ / MeV |
|---|---|---|---|
| −20° | −40.3° | 60° | ±0.073 |
| −30° | −60.9° | 91° | ±0.133 |
| −33° | −67.2° | 100° | ±0.153 |

The larger $\lvert\varphi_s\rvert$, the larger the acceptance, but the acceleration per cavity $\propto\cos\varphi_s$ drops; low-energy sections usually use a larger $\lvert\varphi_s\rvert$ (around −30° to −40°) to secure the acceptance and reduce it gradually at higher energy. The longitudinal beam size should fill only part of the separatrix, so that the non-linear region does not cause emittance growth and losses.

> Note: the separatrix is derived for a periodic structure smoothed into a continuum. For a beamline with only one or two cavities, or a low-energy section with large energy gain per cavity, it is only a qualitative estimate; the real longitudinal acceptance must be found with multi-particle simulations (scans of entry phase and energy).

## RF defocusing {#longitudinal-rf-defocusing}

### Derivation

Near the axis of an axially symmetric TM mode, $\nabla\cdot\boldsymbol E = 0$ and the Ampère–Maxwell law give

$$
E_r = -\frac{r}{2}\frac{\partial E_z}{\partial z},\qquad
B_\theta = \frac{r}{2c^2}\frac{\partial E_z}{\partial t} .
$$

The radial force is $F_r = q(E_r - v B_\theta)$. Using the total derivative along the trajectory, $\dd E_z/\dd z = \partial_z E_z + \partial_t E_z/(\beta c)$:

$$
F_r = -\frac{q r}{2}\left[\frac{\dd E_z}{\dd z} - \frac{1}{\beta c\gamma^2}\frac{\partial E_z}{\partial t}\right].
$$

Integrated over the gap, the total-derivative term vanishes at the field-free ends (assuming $r$ and $\beta$ roughly constant in the gap), and the remaining term gives

$$
\Delta p_r = -\frac{\pi q E_0 T L\, r\sin\varphi}{c\,\beta^2\gamma^2\lambda},\qquad
\Delta r' = \frac{\Delta p_r}{p} = -\frac{\pi q E_0 T L\sin\varphi}{mc^2\,\beta^3\gamma^3\lambda}\, r .
\tag{7}
$$

For $\sin\varphi < 0$ (longitudinally stable) $\Delta r'$ has the sign of $r$: **the gap is a defocusing lens** with $1/f = \pi q E_0 T L\sin(-\varphi)/(mc^2\beta^3\gamma^3\lambda)$. Longitudinal focusing and transverse defocusing come from the same $\sin\varphi$; one gap cannot provide both at the same phase.

### Relation to longitudinal focusing

Averaging (7) over unit length and comparing with (6):

$$
k_{r}^2 = -\frac{\pi q E_0 T\sin(-\varphi_s)}{mc^2\,\beta^3\gamma^3\lambda} = -\frac{k_{l0}^2}{2} .
$$

This is Earnshaw's theorem (Laplace's equation) in the beam rest frame: in a source-free region the focusing strengths of the three directions add up to zero, so longitudinal focusing is necessarily accompanied by defocusing in both transverse planes. In the numerical example above $k_r^2 = -9.9\ \mathrm{m^{-2}}$, which solenoids or quadrupoles must overcome (see [Solenoids](#solenoid), [Quadrupoles](#quadrupole)). The more general Panofsky–Wenzel theorem relates the transverse kick to the transverse gradient of the longitudinal kick and says the same: an energy gain that varies with transverse position comes with a transverse deflection.

The magnetic term $vB_\theta$ is about $\beta^2$ times the electric term and small at low energy: in the example, changing `Kb` from −1.36 to 0 changes the exit rms $x$ of the linear preview by only about 0.01 % and rms $y$ by about 0.2 %. The $B$ components of a map should still be given consistently with $E$, from the same mode.

## In AVAS {#longitudinal-avas}

### Keywords

An RF cavity is a `field` element with `type = 1`:

```avas-params
field input:scanphase input:longlimits beam:twissz beam:frequency
```

```text
start
! L     R     V3 type f        phase Ke   Kb    file
field 0.21  0.02  0  1    162.5e6  -33   1.36 -1.36 hwr010
end
```

- `f` is the cavity frequency; `frequency` in `beam.txt` is the beam (bunch repetition) frequency, which sets the bunch charge $Q = I/f$. The two may differ (for example after a frequency jump).
- `Ke` scales the electric field (map in MV/m) and `Kb` the magnetic field; when changing the gradient of a cavity scale both by the same factor.
- `scanphase` in `input.txt` controls the phase scan: 0 no scan, 1 scan the phases, 2 read the entry phase and entry time of every cavity from `scanData.txt`.
- The output file `Phase.txt` gives, in two lines per RF cavity, time, position and synchronous-particle energy at its entry and exit, which lets you check the energy gain of every cavity.

### Longitudinal model of the linear preview

The linear preview integrates the reference particle's energy and time along the axis in 1 mm steps, iterates the entry phase of V3 = 0 cavities, and pushes the second moments with the linear bunching term in $(z, \delta)$ and the damping from the momentum gain. It contains no longitudinal non-linearity and therefore cannot show the growth of the longitudinal emittance in low-energy cavities; energies agree to about 0.01 %, rms sizes usually to 1–2 %. See [Multi-particle simulation in practice](#simulation).

### Common problems

- **Phase sign**: when phases are taken from another code with the opposite sign, the bunch debunches and is transversely focused instead; compare with the table above.
- **Cavity $\beta$ does not match the beam**: when $T(\beta)$ is small the cavity hardly accelerates and the energy is very sensitive to the phase; check first that the energy gain is reasonable.
- **V3 = 2 cavities lose their phase after upstream changes**: changing a drift length or the upstream energy changes $t_{\mathrm{in}}$ and with it the synchronous phase; use V3 = 0 during design.
- **Large phase width**: a bunch whose phase width approaches the separatrix width develops longitudinal tails and losses; shorten the incoming bunch or increase $\lvert\varphi_s\rvert$.

## Further reading {#longitudinal-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: transit-time factor, longitudinal dynamics (separatrix, adiabatic damping) and RF defocusing.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: longitudinal motion in linear accelerators.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific: synchrotron motion and phase stability.
- CERN Accelerator School lecture notes on linear accelerators ([CAS](https://cas.web.cern.ch/)).
- In this primer: [RF cavities](#rf-cavity), [Field maps](#field-maps), [Space charge](#space-charge) and the [tutorial](#tutorial).
