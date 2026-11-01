# Longitudinal dynamics

A 162.5 MHz RF field goes through one full positive–negative cycle every 6.15 ns. The particles of a bunch reach a cavity at slightly different times, early and late ones see different fields, and one would expect the energies to spread and the bunch to grow longer and longer. Yet in a linac a bunch passes tens or hundreds of cavities without falling apart. What pulls the early and the late particles back together? Why does the AVAS example use a synchronous phase of $-33°$ rather than the crest, where the energy gain is largest? And why does the same cavity that holds the bunch together longitudinally push it apart transversely?

The chapter starts from the energy gain of a single accelerating gap and the transit-time factor, then explains phase stability through the chain "energy sets velocity, velocity sets arrival time", and goes on to synchrotron oscillations, the longitudinal acceptance (separatrix) and RF defocusing. You need the $\beta$, $\gamma$ and $\beta\lambda$ of [Relativistic kinematics](#kinematics), the emittance of [Phase space and emittance](#phase-space), and the $L/\gamma^2$ of [Drift](#drift) ("particles with more momentum run ahead"). How to write an RF cavity element is in [RF cavity](#rf-cavity); in the story line this chapter supplies the reasons behind the choice of synchronous phase in [the superconducting section](#journey-sc) and the bunchers of [the medium-energy transport](#journey-mebt).

All numerical examples use the example project `examples/hwr010`: protons with entrance kinetic energy $W = 1.527\ \mathrm{MeV}$ ($\beta = 0.05698$, $\gamma = 1.00163$), RF frequency $f = 162.5\ \mathrm{MHz}$ ($\lambda = c/f = 1.8449\ \mathrm{m}$, $\beta\lambda = 0.1051\ \mathrm{m}$), the half-wave resonator (HWR) field map `hwr010`, 0.21 m long, `Ke = 1.36`.

## Energy gain in an RF gap {#longitudinal-gap}

### Field and phase

The field in a cavity is a standing wave: its spatial pattern is fixed and its strength oscillates as a cosine in time. A particle needs some time to cross the cavity, and at each position it sees the field of the moment it arrives there. So the energy a cavity gives a particle depends on when the particle arrives — and the RF phase is the most convenient way to record that moment.

Let the longitudinal on-axis field be

$$
E_z(z, t) = E(z)\cos\left(\omega t + \varphi_0\right),
$$

with $E(z)$ the spatial distribution and $\omega = 2\pi f$. A particle reaches $z$ at the time $t(z)$, and the phase it "sees" there is

$$
\varphi(z) = \omega t(z) + \varphi_0 .
$$

In this convention **a larger phase means a later arrival**, and $\varphi = 0$ is the crest of the field. This chapter uses the same cosine convention as AVAS (and Wangler, *RF Linear Accelerators*; see [In AVAS](#longitudinal-in-avas)). The energy gained across the gap is

$$
\Delta W = q\int E(z)\cos\varphi(z)\,\dd z .
\tag{1}
$$

### Panofsky's formula and the transit-time factor

If the velocity changes little in the gap ($\Delta W \ll W$), $t(z)$ can be approximated by uniform motion: $\omega t(z) = \omega t_c + k (z - z_c)$, $k = 2\pi/(\beta\lambda)$, with $z_c$ a reference point of the gap. Inserting into (1):

$$
\Delta W = q\cos\varphi\int E(z)\cos k(z - z_c)\,\dd z - q\sin\varphi\int E(z)\sin k(z - z_c)\,\dd z ,
$$

where $\varphi = \omega t_c + \varphi_0$ is the phase at $z_c$. Choosing $z_c$ at the **electrical centre**, where the second integral vanishes (the geometric centre of a symmetric gap), gives Panofsky's formula

$$
\Delta W = q E_0 T L\cos\varphi = q V_0 T\cos\varphi ,
\qquad
T = \frac{\int E(z)\cos k(z - z_c)\,\dd z}{\int E(z)\,\dd z},
\tag{2}
$$

with $E_0 = V_0/L$ the average on-axis field over the gap length $L$ and $V_0 = \int E\,\dd z$ the on-axis voltage. The **transit-time factor** $T \le 1$ expresses that the field keeps changing while the particle crosses the gap, so the particle cannot stay at the field maximum all the way. For a uniform field over a gap of length $g$,

$$
T = \frac{\sin(\pi g/\beta\lambda)}{\pi g/\beta\lambda},
$$

and the shorter the gap compared with $\beta\lambda$, the closer $T$ is to 1. For the example beam $\beta\lambda = 105\ \mathrm{mm}$: a gap of $\beta\lambda/4 = 26\ \mathrm{mm}$ has $T = 0.90$, a gap of $\beta\lambda/2$ only $2/\pi = 0.64$. In a real cavity the field extends beyond the gap and $T$ must be integrated over the field distribution.

### Exact definition: the phase along the actual trajectory

When the energy gain is comparable to the entrance energy (low-energy protons, heavy ions), uniform motion is no longer a good approximation. Then write (1) as

$$
C = \int E(z)\cos\varphi(z)\,\dd z,\quad S = \int E(z)\sin\varphi(z)\,\dd z,\quad
\Delta W = q\sqrt{C^2 + S^2}\,\cos\varphi_s,\quad \varphi_s = \operatorname{atan2}(S, C),
\tag{3}
$$

with the integrals taken along the reference particle's **actual** trajectory $t(z)$. Eq. (3) is exact and $\varphi_s$ is what AVAS calls the synchronous phase (see [below](#longitudinal-synchronous)), but the "effective voltage" $\sqrt{C^2 + S^2}$ depends on the trajectory and hence on $\varphi_s$, so the energy gain no longer varies strictly as $\cos\varphi_s$.

## Transit-time factor and multi-gap cavities {#longitudinal-multigap}

### Two-gap cavities

The accelerating part of a half-wave (HWR) or quarter-wave (QWR) resonator consists of two gaps with opposite fields ($\pi$ mode). For both gaps to accelerate, the particle must take exactly half an RF period from the first gap to the second, so that the field has just changed sign. With the field of one gap $e(z)$ and the gap centres a distance $d$ apart, $E(z) = e(z - d/2) - e(z + d/2)$, and its Fourier transform gives

$$
T(\beta) = T_g(\beta)\,\left\lvert\sin\frac{k d}{2}\right\rvert = T_g(\beta)\,\left\lvert\sin\frac{\pi\beta_g}{2\beta}\right\rvert,
\qquad d = \frac{\beta_g\lambda}{2},
$$

with $T_g$ the transit-time factor of one gap. For multi-gap fields of alternating sign $\int E\,\dd z \approx 0$, so the denominator of (2) is replaced by $V_0 = \int\lvert E\rvert\,\dd z$ (as in the table below). At $\beta = \beta_g$ both gaps accelerate in phase; the further the velocity is from it, the more the second gap is "out of step". A cavity with $N$ gaps has a sharper $T(\beta)$ and a narrower energy acceptance; low-$\beta$ superconducting linacs therefore use cavities with few gaps in several geometric-β families (see [the superconducting section](#journey-sc)).

### The example cavity hwr010

Integrating the on-axis $E_z$ of the `hwr010` map (times `Ke = 1.36`):

| Quantity | Value |
|---|---|
| On-axis field peaks | $+13.03\ \mathrm{MV/m}$ and $-12.74\ \mathrm{MV/m}$ |
| Field sign change | $z \approx 0.104\ \mathrm{m}$ (two gaps) |
| Centroids of the two gap fields | $0.064\ \mathrm{m}$, $0.145\ \mathrm{m}$, spacing $d = 0.081\ \mathrm{m}$ |
| $\beta_g = 2d/\lambda$ from the spacing | 0.088 |
| $V_0 = \int\lvert E_z\rvert\,\dd z$ | 0.978 MV |
| Maximum of $T(\beta)$ | 0.82 at $\beta \approx 0.104$ |
| $T$ at the entrance $\beta = 0.0570$ | 0.40, $V_0 T = 0.39\ \mathrm{MV}$ |

The maximum of $T(\beta)$ at $\beta \approx 0.10$ matches the "010" in the name (usually a design $\beta$ of 0.10); it is a little above the 0.088 estimated from the spacing because the single-gap $T_g(\beta)$ rises with $\beta$. The example beam's $\beta = 0.057$ is far below the optimum, so $T$ is only 0.40; as the particle gains energy inside the cavity $T$ increases, one more reason why uniform motion fails in low-energy cavities. The slice of this map below shows the opposite fields of the two gaps:

```avas-field
map hwr010
```

### Energy gain and synchronous phase: example

With AVAS's linear preview (`avas.sim.linear_optics`, lattice `field 0.21 0.02 0 1 162.5e6 φs 1.36 -1.36 hwr010`):

| $\varphi_s$ (V3 = 0) | Entrance phase | Exit energy / MeV | $\Delta W$ / MeV |
|---|---|---|---|
| −90° | −14.0° | 1.5270 | 0.000 |
| −60° | 18.6° | 1.6943 | 0.167 |
| −33° | 52.7° | 1.8665 | 0.340 |
| −20° | 69.7° | 1.9374 | 0.410 |
| 0° | 95.4° | 2.0023 | 0.475 |
| +20° | 119.7° | 1.9954 | 0.468 |

At $\varphi_s = -90°$ the gain is zero, as (3) says. But $\Delta W/\cos\varphi_s$ is 0.405 MeV at −33° and 0.475 MeV at 0°, about 15 % apart, and a scan of the entrance phase puts the largest gain (exit energy 2.0087 MeV) at $\varphi_s \approx +8°$, not 0°. Is this real, or an artefact of how the preview solves for the phase? A simple test separates the two: integrate the same map once more with the velocity frozen at its entrance value. Then $\Delta W = qV_0T\cos\varphi_s$ holds to within 1 % at every phase, with $V_0T = 0.389\ \mathrm{MV}$ (0.325 MeV at −33°, 0.328 MeV at +33°, the maximum at $\varphi_s \approx 0$). So the asymmetry comes entirely from the particle speeding up inside the cavity: $\Delta W/W \approx 30\ \%$, and $\beta$ grows from 0.057 to about 0.063–0.065.

Why does the speed-up favour one side? Look at the two gaps separately. The particle is far too slow for this cavity: it needs about 0.77 of an RF period to get from one gap centre to the other instead of half a period, so the two gaps cannot both be near their crests. At $\varphi_s = -33°$ the first gap sits at about −81° and gives only 0.043 MeV, the second sits near its crest and gives 0.297 MeV; $\varphi_s$ is the weighted average of the two. At $\varphi_s = +33°$ it is the other way round: the first gap is near its crest and gives 0.292 MeV. The particle then reaches the second gap noticeably faster and therefore earlier, which moves that gap from about 79° to about 66° past its crest and raises its gain from 0.054 to 0.132 MeV. Where in the cavity the energy is gained decides how much the timing of the rest of the cavity shifts — this is what "the effective voltage depends on the phase" means here. For such cavities Panofsky's formula is only an estimate and design must rely on the integral along the trajectory. The engine also defines $\varphi_s$ by (3) along the reference trajectory (see [In AVAS](#longitudinal-in-avas)), so this is a property of the physics and of that definition, not of the preview; the numbers above are from the preview's on-axis reference-particle integration.

## Synchronous particle and synchronous phase {#longitudinal-synchronous}

A bunch contains thousands or millions of particles; to say "where the bunch is and at what phase" we need a reference. The **synchronous particle** (reference particle) is the imaginary particle that by design moves on axis and arrives at every cavity at the design phase; the longitudinal coordinates of all other particles are defined relative to it. The phase it "sees" in a cavity is that cavity's **synchronous phase** $\varphi_s$, defined by (3).

The phase of the synchronous particle at the $n$-th cavity is set by two things together: the RF phase of the cavity and the particle's flight time through everything upstream. A drift between cavities slips the phase by $360°\cdot L/(\beta\lambda)$: crossing a 0.1 m drift, the example beam advances by $342.4°$. At low energy $\beta\lambda$ is only some ten centimetres, so a few centimetres of length error or a small energy deviation noticeably change the phase in downstream cavities — the basic reason why phase settings are so sensitive at low energy.

In AVAS, the parameter V3 of `field` decides whether the phase parameter is a "synchronous", "entrance" or "absolute" phase; the three forms and an example are in [In AVAS](#longitudinal-in-avas).

## Phase stability {#longitudinal-stability}

### Thought experiment: three particles

Imagine three particles entering a series of cavities one behind the other: the synchronous particle at $\varphi_s$, one a little early (smaller phase), one a little late (larger phase). The energy gain is $\propto\cos\varphi$, with the crest at $\varphi = 0$.

- If $\varphi_s$ lies **before the crest** ($-90° < \varphi_s < 0$, the field is rising): the late particle is closer to the crest and gains **more** energy; the early one is further from it and gains less. A non-relativistic particle with more energy is faster, so in the following drift the late one catches up and the early one drops back. The deviation is corrected — a restoring force.
- If $\varphi_s$ lies **after the crest** ($0 < \varphi_s < 90°$): the late particle gains less, is slower and falls further behind; the early one runs further ahead. The deviation grows — unstable.
- If $\varphi_s$ is **exactly on the crest**: early and late particles both gain slightly less than the synchronous one, the difference is of second order in the deviation, there is no first-order restoring force, and the bunch just keeps lengthening in the drifts.

Longitudinal focusing thus consists of two links: the cavity makes the energy depend on the phase (the "lens"), and the drift turns velocity differences into position differences (the $R_{56} = L/\gamma^2$ of [Drift](#drift-longitudinal)). This is exactly the counterpart of the transverse "thin lens + drift", and longitudinal stability can be judged with the same matrix-trace criterion. In straight sections $R_{56} = L/\gamma^2$ is always positive, so the stable phase always lies on the rising side of the field.

### Equations of motion

Now the picture as equations. Consider a periodic series of cavities and average each cavity's action over the length: an average accelerating gradient $E_0 T$ (the $E_0 T$ inside a cavity times the fraction of the period the cavity occupies). Define the deviations from the synchronous particle

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

Eq. (4) is "velocity difference becomes phase difference": a particle with more energy ($w > 0$) is faster and its phase decreases (it arrives earlier). The coefficient $A \propto 1/(\beta^3\gamma^3)$ is $1.83\times 10^4\ \mathrm{m^{-1}}$ at the example energy: the phase of a low-energy proton is extremely sensitive to its energy. Eq. (5) is "phase difference becomes energy difference".

### Stability condition

Linearizing (4) and (5) with $\cos\varphi - \cos\varphi_s \approx -\sin\varphi_s\,\Delta\varphi$:

$$
\frac{\dd^2\Delta\varphi}{\dd s^2} = A B \sin\varphi_s\,\Delta\varphi .
$$

Since $A, B > 0$, **the motion is stable only for $\sin\varphi_s < 0$** — then the right-hand side is a restoring force. Together with $\cos\varphi_s > 0$ (acceleration), the stable accelerating range is

$$
-90° < \varphi_s < 0° .
$$

This is the conclusion of the thought experiment: a negative synchronous phase means the synchronous particle arrives before the crest.

> Note: sign conventions differ between texts. In the cosine convention of AVAS and Wangler (larger phase = later) the stable synchronous phase is negative; the example uses $-33°$. With a sine convention or a phase defined as $-\omega t$ the stable range becomes positive. Always check the convention when you take phases from another code or paper.

### Checking the sign with AVAS's linear preview

The example beam (initial rms $z$ = 1.316 mm, rms $x$ = 0.954 mm) through one hwr010 cavity, rms sizes at the exit (linear preview, no space charge):

| Case | rms $z$ / mm | rms $x$ / mm |
|---|---|---|
| 0.21 m drift (no cavity) | 1.482 | 1.407 |
| $\varphi_s = -33°$ | 0.702 | 1.811 |
| $\varphi_s = +33°$ | 1.738 | 1.196 |

A negative synchronous phase compresses the bunch longitudinally (bunching), a positive one stretches it (debunching), as the stability condition says; transversely it is the other way round, which is [RF defocusing](#longitudinal-rf-defocusing). Notice also that at $+33°$ the energy gain is 0.424 MeV, more than the 0.340 MeV at $-33°$ — in a low-energy cavity the effective voltage depends on the phase and positive and negative phases are not symmetric (the reason, the particle speeding up in the first gap, is explained [above](#longitudinal-multigap)). Drag the phase in the lab below: with negative phases rms $z$ shrinks and rms $x$ grows, with positive phases the opposite; the phase-scan plot shows at a glance on which side of the crest $\varphi_s$ lies.

```avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
```

> **Think about it**: a 10 MeV electron has $\gamma \approx 20.6$. Does an electron linac still need phase stability?
>
> Answer: hardly. The $A$ of (4) goes as $1/(\beta^3\gamma^3)$; for a 10 MeV electron it is only $2\times 10^{-8}$ times that of the example proton: energy differences barely change the velocity and the phase is "frozen". Electron linacs can therefore run the bunch near the crest ($\varphi_s \approx 0$) for maximum energy gain without the bunch spreading because of energy differences. Low-energy protons are the opposite case and must be held together by phase stability.

## Small-amplitude synchrotron oscillations and adiabatic damping {#longitudinal-synchrotron}

### Longitudinal wave number

In the stable case $\Delta\varphi$ oscillates harmonically, $\Delta\varphi'' + k_{l0}^2\,\Delta\varphi = 0$: the particle swings back and forth around the synchronous particle and its energy alternately rises and falls — the **synchrotron oscillation**. The **zero-current longitudinal wave number** is

$$
k_{l0}^2 = -A B\sin\varphi_s = \frac{2\pi\, q E_0 T\sin(-\varphi_s)}{mc^2\,\beta_s^3\gamma_s^3\,\lambda} .
\tag{6}
$$

The synchrotron wavelength is $\lambda_l = 2\pi/k_{l0}$ and the longitudinal phase advance per focusing period $L_p$ is $\sigma_{0l} = k_{l0}L_p$. From (4), the ratio of energy to phase amplitude is $w_{\max}/\Delta\varphi_{\max} = k_{l0}/A$. The $\beta^3\gamma^3$ in (6) says that longitudinal focusing weakens rapidly with energy; synchrotron oscillations are fastest at low energy.

A number: if a series of cavities had an average gradient equal to the example cavity's $V_0 T / L = 0.39\ \mathrm{MV}/0.21\ \mathrm{m} = 1.86\ \mathrm{MV/m}$ at the entrance, with $\varphi_s = -33°$, then $k_{l0} = 4.44\ \mathrm{m}^{-1}$ and $\lambda_l = 1.41\ \mathrm{m}$. In a real structure solenoids and drifts sit between the cavities, the average gradient is lower and $k_{l0}$ smaller ($\propto\sqrt{E_0 T}$).

One cavity can also be seen as a longitudinal thin lens. Linearizing (5) and changing to $(z, \delta)$ coordinates (below), its longitudinal focal length is $1/f_z = 2\pi qV_0T\sin(-\varphi_s)/(mc^2\beta^3\gamma^3\lambda)$. For the example cavity at $\varphi_s = -33°$, $1/f_z \approx 4.1\ \mathrm{m^{-1}}$ and $f_z \approx 0.24\ \mathrm m$ — stronger than the quadrupoles of the FODO lab in the previous chapter ($f = 0.45\ \mathrm m$). This is only a rough estimate from the averaged formula ($\beta$ changes by about 14 % inside the cavity), but it explains why a single cavity squeezes the rms $z$ in the table above from 1.48 mm to 0.70 mm.

### Conversion to AVAS's longitudinal coordinates

AVAS's longitudinal phase space uses $(z, \delta)$ with $\delta = \Delta p/p$:

$$
z = -\frac{\beta\lambda}{2\pi}\,\Delta\varphi,\qquad
\delta = \frac{\Delta W}{\beta^2\gamma\,mc^2} = \frac{w}{\beta^2\gamma} .
$$

So $z$ obeys the same oscillation equation as $\Delta\varphi$. The example beam's `twissz 0.04275 0.37292 0.26506` gives an initial rms $z = 1.316\ \mathrm{mm}$ (rms phase width 4.5°) and rms $\delta = 3.53\ \mathrm{mrad}$ (rms energy spread 10.8 keV); for the meaning of the Twiss parameters see [Phase space and emittance](#phase-space), for the syntax of `twissz` see [In AVAS](#longitudinal-in-avas).

### Adiabatic damping

When acceleration changes $\beta\gamma$ and $E_0 T$ slowly, $(\Delta\varphi, w)$ remain a canonical pair (Hamiltonian in the [next section](#longitudinal-separatrix)) and the action $\oint w\,\dd\Delta\varphi \propto \Delta\varphi_{\max} w_{\max}$ is an adiabatic invariant — like a pendulum whose length changes slowly: the amplitude changes, the phase-space area does not. With $w_{\max}/\Delta\varphi_{\max} = k_{l0}/A$ this gives

$$
\Delta\varphi_{\max} \propto \left[E_0 T\sin(-\varphi_s)\,\lambda\,\beta_s^3\gamma_s^3\right]^{-1/4},\qquad
\Delta W_{\max} \propto \left[E_0 T\sin(-\varphi_s)\,\lambda\,\beta_s^3\gamma_s^3\right]^{1/4}.
$$

For constant gradient, phase and frequency, $\Delta\varphi \propto (\beta\gamma)^{-3/4}$ and $\Delta W \propto (\beta\gamma)^{3/4}$: the phase width shrinks, the energy spread grows, and their product stays. From 1.527 MeV to 10 MeV, for example, the phase width shrinks to 0.49 times, the absolute energy spread grows to 2.0 times, and the relative spread $\Delta W/W$ falls to about 0.31 times. Bunches become ever shorter in phase, which is why the frequency can be doubled further downstream (see [From the source to the target](#journey-overview)). In $(z, \delta)$, $\beta\gamma\cdot z\,\delta \propto \lambda\,\Delta\varphi\,\Delta W$ is constant: the **normalized longitudinal emittance is conserved**, just as transversely (geometric emittance $\propto 1/\beta\gamma$), see [Phase space and emittance](#phase-space).

> Note: this conservation holds only for linear, uncoupled motion without space charge. Longitudinal non-linearity in low-energy cavities (the curvature of $\cos\varphi$ once the phase width reaches tens of degrees) makes the longitudinal rms emittance grow; the linear preview does not include this, use the multi-particle engine.

## Longitudinal acceptance and separatrix {#longitudinal-separatrix}

### Physical picture: a ball on a tilted washboard

At small amplitude the longitudinal motion is a pendulum — and at large amplitude? The Hamiltonian form shows the whole picture. Eqs. (4) and (5) follow from the Hamiltonian ($s$ as independent variable, $\varphi$ and $w$ conjugate)

$$
H(\varphi, w) = -\frac{A w^2}{2} - B\left(\sin\varphi - \varphi\cos\varphi_s\right)
$$

via $\varphi' = \partial H/\partial w$, $w' = -\partial H/\partial\varphi$. $H$ is conserved and the phase-space trajectories are its contour lines. Read $-H$ as "kinetic + potential energy": the potential is $U(\varphi) = B(\sin\varphi - \varphi\cos\varphi_s)$, a sinusoidal ripple plus a linear tilt $-B\varphi\cos\varphi_s$ — a tilted washboard with the particle as a ball on it. The ripple comes from the RF field, the tilt from the acceleration the synchronous particle must receive.

- A ball rolling back and forth in one hollow is a bounded synchrotron oscillation;
- on the downhill side of the hollow there is a "lip"; a ball with more energy than the lip rolls over it and all the way down the tilted board — its phase slips without end and never comes back: a longitudinal loss.

The analogy is mathematically exact; its limits are that $A$ and $B$ change slowly with energy (the hollow slowly deforms) and that real cavities are discrete, so "averaging to a continuum" holds only when the longitudinal phase advance per period is not large.

### Fixed points and separatrix

$w = 0$ and $\cos\varphi = \cos\varphi_s$ give two fixed points: $\varphi = \varphi_s$ (stable, the bottom of the hollow, elliptic) and $\varphi_2 = -\varphi_s$ (unstable, the lip, hyperbolic). The contour through $\varphi_2$ is the **separatrix**:

$$
\frac{A w^2}{2} + B\left(\sin\varphi - \varphi\cos\varphi_s\right) = -B\left(\sin\varphi_s - \varphi_s\cos\varphi_s\right).
$$

Particles inside the separatrix perform bounded synchrotron oscillations; together they form the longitudinal **acceptance** (the "fish" or "golf-club" shaped stable region, often called the bucket). Particles outside it slip in phase continuously, eventually leave the accelerating phase and are lost longitudinally.

### Width and height of the stable region

- The phase range extends from $\varphi_1$ to $\varphi_2 = -\varphi_s$, with $\varphi_1$ from $\sin\varphi_1 - \varphi_1\cos\varphi_s = -\sin\varphi_s + \varphi_s\cos\varphi_s$. For small $\lvert\varphi_s\rvert$ the total width is $\psi \approx 3\lvert\varphi_s\rvert$.
- The energy half-height, at $\varphi = \varphi_s$:

$$
\Delta W_{\max} = 2\left[\frac{q E_0 T\,\beta_s^3\gamma_s^3\,\lambda\, mc^2\left(\varphi_s\cos\varphi_s - \sin\varphi_s\right)}{2\pi}\right]^{1/2}.
$$

The depth of the hollow is proportional to $\varphi_s\cos\varphi_s - \sin\varphi_s$: zero at $\varphi_s = 0$ (no hollow at all), deeper for larger $\lvert\varphi_s\rvert$. With the average gradient of 1.86 MV/m from the previous section and the example beam energy:

| $\varphi_s$ | $\varphi_1$ | Phase width | $\Delta W_{\max}$ / MeV | $\cos\varphi_s$ |
|---|---|---|---|---|
| −20° | −40.3° | 60° | ±0.073 | 0.940 |
| −30° | −60.9° | 91° | ±0.133 | 0.866 |
| −33° | −67.2° | 100° | ±0.153 | 0.839 |

This answers the opening question: a larger $\lvert\varphi_s\rvert$ gives a larger acceptance but less acceleration per cavity, $\propto\cos\varphi_s$. The example's $-33°$ gives up about 16 % of the energy gain in exchange for a stable region 100° wide and ±0.15 MeV high, while the bunch itself has an rms phase width of only 4.5° and an rms energy spread of 10.8 keV — ample margin. Low-energy sections usually choose a larger $\lvert\varphi_s\rvert$ (about −30° to −40°) for acceptance; further downstream the bunches become shorter in phase (adiabatic damping) and $\lvert\varphi_s\rvert$ is reduced gradually. The longitudinal size of the beam should fill only part of the separatrix area, so that the non-linear region does not cause emittance growth and losses.

> **Think about it**: why not put the synchronous phase on the crest, $\varphi_s = 0$, and bunch the beam some other way?
>
> Answer: at $\varphi_s = 0$, $U(\varphi) = B(\sin\varphi - \varphi)$ decreases monotonically: there is no hollow and the separatrix has zero area, so no phase deviation is restored. For low-energy protons $A$ is large, a small energy difference quickly becomes a large phase difference, and the bunch spreads within a few cavities. Either separate bunching cavities are added between the accelerating ones, or the accelerating cavities themselves run off the crest — which is the choice $\varphi_s < 0$.

> Note: the separatrix is derived for a periodic structure "averaged to a continuum". In a line with only one or two cavities, or at low energy with a large energy gain per cavity, it is only a qualitative estimate; the real longitudinal acceptance must be found by multi-particle simulation (scanning entrance phase and energy).

## RF defocusing {#longitudinal-rf-defocusing}

### Physical picture

Why does a cavity that gathers the bunch longitudinally spread it transversely? There are two views that support each other.

**The rest-frame view (Earnshaw's theorem)**: in the frame moving with the bunch the particles move slowly relative to each other and the cavity field is approximately electrostatic. An electrostatic potential in a source-free region obeys Laplace's equation: the second derivatives in the three directions add up to zero. For longitudinal stability the potential must be a hollow in $z$ (positive second derivative), so at least one transverse second derivative is negative — for an axially symmetric cavity, both. Longitudinal focusing and transverse defocusing come from the same potential; one cannot have only one of them.

**The gap view**: at the gap entrance the field lines bend towards the axis ($\partial E_z/\partial z > 0$, $E_r$ points inwards: focusing), at the exit they open outwards (defocusing). With a static field the two halves would roughly cancel; but at a stable phase $\varphi_s < 0$ the field is still **rising** while the particle crosses (the crest has not arrived yet), so the defocusing exit half is stronger than the focusing entrance half and the net effect is defocusing.

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

Integrated over the gap, the total-derivative term vanishes at the field-free ends (assuming $r$ and $\beta$ roughly constant in the gap); what is left is the "field changing in time" term — exactly the physics of the gap view. The result is

$$
\Delta p_r = -\frac{\pi q E_0 T L\, r\sin\varphi}{c\,\beta^2\gamma^2\lambda},\qquad
\Delta r' = \frac{\Delta p_r}{p} = -\frac{\pi q E_0 T L\sin\varphi}{mc^2\,\beta^3\gamma^3\lambda}\, r .
\tag{7}
$$

For $\sin\varphi < 0$ (longitudinally stable) $\Delta r'$ has the sign of $r$: **the gap is a defocusing lens** with $1/f = \pi q E_0 T L\sin(-\varphi)/(mc^2\beta^3\gamma^3\lambda)$. Longitudinal focusing and transverse defocusing come from the same $\sin\varphi$ and cannot be separated at a given phase.

A number: for the example cavity ($V_0T = E_0TL = 0.39\ \mathrm{MV}$) at $\varphi_s = -33°$, (7) gives roughly $1/f \approx 2.07\ \mathrm{m^{-1}}$, $f \approx 0.48\ \mathrm m$ — the transverse defocusing of one cavity is about as strong as the focusing of one quadrupole of the FODO lab ($f = 0.45\ \mathrm m$), and the longitudinal focal length of 0.24 m is exactly half of it. In the earlier table, the exit rms $x$ grows from 1.41 mm (drift) to 1.81 mm at $-33°$: this is the effect. The transfer matrix of the linear preview, which integrates the gradients of the map along the actual trajectory, gives $-1/M_{21} \approx 0.37\ \mathrm m$ in $x$ and $0.45\ \mathrm m$ in $y$, and $0.24\ \mathrm m$ longitudinally: the averaged formula (7) gets the order of magnitude and the factor 2 between the planes right, but not the details. An HWR is not axially symmetric, so $x$ and $y$ need not agree (whether the engine shows the same $x$/$y$ difference has not been checked).

### Relation to longitudinal focusing

Averaging (7) over the length and comparing with (6):

$$
k_{r}^2 = -\frac{\pi q E_0 T\sin(-\varphi_s)}{mc^2\,\beta^3\gamma^3\lambda} = -\frac{k_{l0}^2}{2} .
$$

This is the quantitative form of Earnshaw's theorem: in a source-free region the focusing strengths of the three directions add up to zero, and longitudinal focusing necessarily comes with defocusing of half the strength in each transverse direction. In the numerical example above $k_r^2 = -9.9\ \mathrm{m^{-2}}$, which solenoids or quadrupoles must overcome with stronger transverse focusing (see [Solenoid](#solenoid), [Quadrupole](#quadrupole)); this is one of the main reasons why low-energy superconducting sections alternate solenoids and cavities. The more general Panofsky–Wenzel theorem relates the transverse kick to the transverse gradient of the longitudinal kick and says the same: an energy gain that depends on the transverse position always comes with a transverse deflection.

The magnetic term $vB_\theta$ is about $\beta^2$ times the electric term and small at low energy: setting `Kb` from −1.36 to 0 in the example changes the exit rms $x$ of the linear preview by only about 0.01 % and rms $y$ by about 0.2 %. The $B$ components of a field map should nevertheless belong consistently to the same mode as $E$.

## Common misconceptions {#longitudinal-misconceptions}

- **Sign of the synchronous phase**: AVAS uses the cosine convention with larger phase = later; the stable range is $-90° < \varphi_s < 0$. With a sine or $-\omega t$ convention it is positive. With the wrong sign after porting, the bunch spreads longitudinally and is focused transversely instead.
- **The synchronous phase is not the entrance phase**: in the example cavity $\varphi_s = -33°$ corresponds to an entrance phase of 52.7°, about 86° apart; the difference depends on the zero of the field map and the field distribution, and there is no general conversion.
- **Is the energy gain largest at $\varphi_s = 0$?** Only with uniform motion. In the example's low-β cavity the largest gain is at $\varphi_s \approx +8°$, and the gains at $\pm 33°$ differ. This is not a numerical artefact: with the velocity frozen the asymmetry disappears; it comes from the acceleration inside the cavity.
- **Is Panofsky's formula exact?** It assumes a constant velocity in the cavity; with $\Delta W/W$ of tens of per cent it is only an estimate.
- **Does a growing energy spread mean a worse beam?** No. Adiabatic damping makes the absolute spread grow as $(\beta\gamma)^{3/4}$, but the phase width shrinks at the same time; the normalized longitudinal emittance stays and the relative spread decreases.
- **May the bunch fill the separatrix?** Near the separatrix the motion is strongly non-linear, particles turn slowly and filament easily; emittance growth and losses start there. The bunch should occupy only a small part of it.
- **Is RF defocusing a flaw of the cavity design?** No. It follows from Laplace's equation; every longitudinally stable accelerating phase comes with transverse defocusing, which only external focusing can compensate.
- **Is the cavity frequency the beam frequency?** Not necessarily. The `f` of `field` is the cavity frequency, `frequency` in `beam.txt` the bunch repetition frequency; they differ after a frequency jump.
- **Do electrons behave like protons?** The phase of high-energy electrons is almost frozen ($A \propto 1/\beta^3\gamma^3$); their need for phase stability is entirely different from that of low-energy protons.

## In AVAS {#longitudinal-in-avas}

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

- `f` is the cavity frequency; `frequency` in `beam.txt` is the beam frequency (bunch repetition rate, which sets the bunch charge $Q = I/f$). They may differ (e.g. after a frequency doubling).
- `Ke` scales the electric field (map in MV/m) and `Kb` the magnetic field; scale both together when changing the cavity gradient.
- `scanphase` in `input.txt` controls the phase scan: 0 no scan, 1 scan the phase, 2 read the entrance phases and times of the cavities from `scanData.txt`.
- `longlimits` sets the phase and energy limits of the longitudinal loss criterion, the counterpart of the separatrix above: particles outside the limits count as lost longitudinally.
- The output file `Phase.txt` gives, two lines per RF cavity, the entrance and exit time, position and synchronous energy; use it to check the gain of each cavity.
- `twissz` in `beam.txt` holds the longitudinal Twiss parameters: $\alpha_z$, $\beta_z$ (mm/mrad, with $z$ in mm and $\delta = \Delta p/p$ in mrad) and the normalized rms longitudinal emittance (π·mm·mrad), consistent with the $(z, \delta)$ conversion above.

### Convention for RF fields and phases

> **AVAS convention**: the RF electric field acts as $K_e\,E(\boldsymbol r)\cos(\omega t + \varphi_0)$ and the RF magnetic field as $K_b\,B(\boldsymbol r)\sin(\omega t + \varphi_0)$ ($E$ in MV/m, $B$ in T in the maps). This matches the cosine convention of Wangler's *RF Linear Accelerators*. The longitudinal coordinate relates to the phase by $\Delta\varphi = -2\pi z/(\beta\lambda)$: a particle with $z > 0$ is ahead of the reference particle and has a smaller phase.

### Three meanings of the phase (V3)

In `field L R V3 type f phase Ke Kb file` the third parameter V3 sets the meaning of the sixth parameter `phase`:

| V3 | Meaning of `phase` | How the engine sets the entrance phase $\varphi_0$ |
|---|---|---|
| 0 | synchronous phase $\varphi_s$ | solves for $\varphi_0$ so that $\operatorname{atan2}(S, C) = \varphi_s$ along the reference trajectory, eq. (3) |
| 1 | RF phase when the reference particle reaches the element entrance | $\varphi_0$ = `phase` |
| 2 | RF phase at $t = 0$ (absolute phase) | $\varphi_0 = $ `phase` $+ 360°\cdot f\cdot t_{\mathrm{in}}$, with $t_{\mathrm{in}}$ the arrival time of the reference particle at the entrance |

V3 = 0 is the most convenient for design: the synchronous phase is what matters physically, and the code finds the entrance phase. V3 = 1 suits phase scans (the entrance phase is tied directly to the cavity geometry). V3 = 2 corresponds to a real machine, where each cavity's RF is phased against a common reference clock; changes upstream then change the synchronous phase.

Example: put a 0.1 m drift before the example cavity. The reference particle reaches the cavity entrance at $t_{\mathrm{in}} = 0.1/(\beta c) = 5.854\ \mathrm{ns}$, $360°\,f\,t_{\mathrm{in}} = 342.45°$. With V3 = 2 and `phase = 0` the entrance phase is $342.45° \equiv -17.55°$, a synchronous phase of about −93.7°: the cavity hardly accelerates. With V3 = 0 and `phase = -33`, the entrance phase is 52.7° and the synchronous phase stays −33° whatever the length of the drift.

> **AVAS convention**: in the example the synchronous phase $-33°$ corresponds to an entrance phase of $52.7°$; the difference (about 86°) depends on the zero of the field map and the field distribution, and there is no general conversion. For V3 = 0 cavities the linear preview shows the entrance phase it found together with the synchronous phase. When a segment run takes a piece of the lattice on its own, AVAS (`rephase` in `avas/data/segment.py`) takes each cavity's RF phase $\varphi_{\mathrm{RF}}$ from the synchronous-particle data of the reference run and converts it to a V3 = 2 phase with the segment entrance as time zero, $\varphi_{\mathrm{RF}} - 360°\cdot f\cdot(t_{\mathrm{in}} - T_{\mathrm{entry}})$, instead of keeping the original phase or switching to V3 = 0.

### Longitudinal model of the linear preview

The linear preview integrates the reference particle's energy and time along the axis in 1 mm steps, solves iteratively for the entrance phase of V3 = 0 cavities (when the iteration does not converge it scans the entrance phase and bisects; without an accelerating solution it takes, like the engine, the decelerating one 180° away, and when neither exists it warns and takes the closest one), and pushes the second moments with the linear bunching in $(z, \delta)$ and the damping from the momentum gain. It contains no longitudinal non-linearity and so cannot show the growth of the longitudinal emittance in low-energy cavities; the reference energy is accurate to about 0.01 % (compared with the engine's synchronous particle; the energy in the engine's DataSet is the mean of all macro-particles, which see a different $E_z$ off axis: in the example HWR the mean gain is about 5 % above the reference particle's), rms sizes typically to 1–2 %. See [Multi-particle simulation](#simulation).

### Common problems

- **Phase sign**: a phase ported from another code with the opposite sign makes the bunch spread while being focused transversely; compare with the ±33° table above.
- **Cavity β not matched to the beam**: when $T(\beta)$ is small the cavity hardly accelerates and the phase is very sensitive to the energy; check first that the energy gain is reasonable.
- **V3 = 2 cavities dephased after upstream changes**: changing a drift length or the upstream energy changes $t_{\mathrm{in}}$ and with it the synchronous phase; prefer V3 = 0 during design.
- **Large phase width**: when the bunch's phase width approaches the separatrix width, longitudinal tails and losses appear; shorten the incoming bunch or increase $\lvert\varphi_s\rvert$.

## Further reading {#longitudinal-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: transit-time factor, longitudinal dynamics (separatrix, adiabatic damping) and RF defocusing.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: longitudinal motion in linacs.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific: synchrotron oscillations and phase stability.
- CERN Accelerator School linac course notes ([CAS](https://cas.web.cern.ch/)).
- In this primer: [RF cavity](#rf-cavity), [Field maps](#field-maps), [Space charge](#space-charge) and [Tutorial](#tutorial).
