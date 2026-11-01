# RF cavity

A proton passes through a grounded metal box with a hole at each end. If the box held only an electrostatic field, the proton would give back on the way out exactly what it gained on the way in: the line integral of an electrostatic field around a closed path is zero, and entrance and exit sit at the same potential. Yet an RF cavity in a linac is precisely such a closed metal box, and it hands the proton a few hundred keV per pass. Where does the energy come from? Why does the same cavity treat fast and slow particles so differently? And why does the very phase that holds the bunch together push the beam apart transversely?

This chapter starts from why the field oscillates at all, then works through the transit-time factor, the energy gain $qV_0T\cos\varphi$, the choice of phase and RF defocusing, and ends with the `field` element and its phase conventions in AVAS. Read [Kinematics](#kinematics) first (the meaning of β, γ, βλ); longitudinal oscillations are treated in [Longitudinal motion](#longitudinal), and why a whole linac needs several kinds of cavity is told in [From source to target](#journey-overview) and [Superconducting section](#journey-sc).

Start with the example project's 162.5 MHz half-wave resonator hwr010 (synchronous phase $-30^\circ$, Ke = 1.36). Turn on the energy curve in the 2D view: the linear preview takes the kinetic energy from 1.527 MeV to 1.884 MeV. Then look at rms x: after the drift behind the cavity it is 3.01 mm at the end, while the same length of plain drift gives only 2.09 mm. The cavity accelerates and at the same time pushes the beam apart transversely; this chapter explains why the two always come together.

```avas-beamline
drift 0.05 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010
drift 0.2 0.02 0
```

All numbers in this chapter use the example project `examples/hwr010`: protons, injection energy $W = 1.527\ \mathrm{MeV}$ ($\beta = 0.05698$, $\beta\gamma = 0.05708$), $f = 162.5\ \mathrm{MHz}$, $\lambda = 1.845\ \mathrm{m}$, $\beta\lambda = 0.1051\ \mathrm{m}$, current 0.184 mA. Unless stated otherwise they come from AVAS's own linear envelope preview (`avas.sim.linear_optics`); the engine result is what counts in the end.

## How a closed box gives energy {#rf-cavity-role}

The answer is in Faraday's law, $\nabla\times\boldsymbol{E} = -\partial\boldsymbol{B}/\partial t$. As soon as the field changes in time, $\oint\boldsymbol{E}\cdot\dd\boldsymbol{l}$ need not vanish, and the argument "what you gain going in you return going out" no longer holds. Change alone is not enough, though: a particle that stayed in the box for many RF periods would be pushed and pulled in turn and gain nothing on average. RF acceleration is a matter of **timing**. The particle crosses an accelerating gap only while the field points forward; while the field is reversed the particle has not arrived yet, has already left, or is hidden inside a drift tube (there is no field inside a conductor).

What an RF cavity does to the beam therefore comes down to three things, each explained in the sections below:

- **Acceleration**: the synchronous particle gains $\Delta W = qV_0T\cos\varphi_s$ per cavity, where $T$ measures how well the timing works out ([transit time](#rf-cavity-energy-gain)).
- **Longitudinal focusing**: put the synchronous particle on the side where the field rises in time; early particles gain less, late ones more, and the bunch stays together ([phase](#rf-cavity-phase)).
- **Transverse defocusing**: a longitudinally focusing phase necessarily defocuses transversely. Maxwell's equations demand it; it is not a flaw of the cavity design ([transverse defocusing](#rf-cavity-transverse)).

The low-energy part ($\beta \lesssim 0.2$) of a superconducting proton linac is usually a periodic structure of "one or a few cavities + one solenoid", the solenoid making up for the third point; the [tutorial](#tutorial) builds exactly this structure.

## Resonant cavities: why the field oscillates {#rf-cavity-modes}

### The picture of an LC circuit

Think of an LC circuit first. With the capacitor fully charged, all the energy is in the electric field and no current flows; the capacitor discharges through the inductor and the energy moves into the magnetic field; a quarter period later the electric field is zero and the current is largest; then the inductor charges the capacitor the other way round … The energy sloshes between electric and magnetic field at $f = 1/(2\pi\sqrt{LC})$.

A resonant cavity is the "distributed" version of the same thing. The conductor faces on either side of the accelerating gap act as the capacitor, and the electric field is concentrated there; RF currents flow on the surrounding walls, act as the inductor, and the magnetic field circles those currents. One important consequence follows at once: **when the electric field is largest the magnetic field is zero; the two are $90^\circ$ apart in time**. Where does the picture break? In a real cavity capacitance and inductance are spread over the whole volume, and the same cavity has infinitely many resonant modes, each with its own field pattern and frequency; the LC circuit describes only one of them. The actual field shape comes from solving Maxwell's equations.

### The TM010 mode of a pillbox

The simplest accelerating cavity that can be solved exactly is a perfectly conducting cylinder (pillbox) of radius $R$ and length $d$. Solving the wave equation in cylindrical coordinates with zero tangential electric field on the walls, the accelerating TM010 mode has only $E_z$ and $B_\theta$:

$$
E_z(r,t) = E_0\,J_0\!\left(\frac{2.405\,r}{R}\right)\cos\omega t,\qquad
B_\theta(r,t) = -\frac{E_0}{c}\,J_1\!\left(\frac{2.405\,r}{R}\right)\sin\omega t
$$

2.405 is the first zero of $J_0$, so that $E_z = 0$ at $r = R$. The magnetic field follows directly from Faraday's law: $(\nabla\times\boldsymbol{E})_\theta = -\partial E_z/\partial r = E_0 k J_1(kr)\cos\omega t$ (with $k = 2.405/R$ and $J_0' = -J_1$); integrating over time, the magnetic field varies as $\sin\omega t$, the $90^\circ$ of the LC picture. **This is the physical origin of AVAS using cos for the RF electric field and sin for the RF magnetic field** (see [In AVAS](#rf-cavity-in-avas)).

The resonance condition gives the frequency

$$
f_{010} = \frac{2.405\,c}{2\pi R}
$$

It depends on the radius only, not on the length: the frequency is set by the transverse size. For $f = 162.5\ \mathrm{MHz}$, $R \approx 0.706\ \mathrm{m}$, a cavity about 1.4 m across. For a low-energy proton beam that needs an aperture of a few centimetres this is bulky and wasteful: most of the stored energy sits far from the beam.

### Coaxial resonators: QWR and HWR

Low-frequency, low-β structures therefore use TEM coaxial-line resonators, which fold the inductance into an inner conductor:

- **Quarter-wave resonator (QWR)**: a coaxial line shorted at one end and open at the other, inner conductor about $\lambda/4$ long. The beam crosses the drift tube near the open end of the inner conductor, forming two accelerating gaps. The structure is not up–down symmetric; the field contains a dipole component that deflects the beam vertically depending on phase.
- **Half-wave resonator (HWR)**: a coaxial line shorted at both ends, inner conductor about $\lambda/2$ long (about 0.92 m at 162.5 MHz), with the beam crossing the middle. The middle is where the voltage is largest and the current smallest, so near the beam the electric field is strong and the magnetic field weak. An HWR is up–down symmetric around the beam axis and has no QWR-like dipole kick, but the transverse field is not exactly axially symmetric and contains a quadrupole component.
- **Multi-gap structures**: spoke cavities, elliptical cavities and others have three or more gaps. For AVAS they are all "one field map"; every physical difference is contained in the field distribution.

In a two-gap cavity the fields of the two gaps point in opposite directions at any instant. If the particle needs exactly half an RF period to travel from the centre of the first gap to the centre of the second, the field has just turned round and both gaps accelerate it. This requires a gap-centre distance $d = \beta_g\lambda/2$; $\beta_g$ is the cavity's **geometric β**.

### The example cavity hwr010

The on-axis $E_z$ of the example map has a peak at $z \approx 0.074\ \mathrm{m}$ and one of opposite sign at $z \approx 0.140\ \mathrm{m}$, with the zero crossing at $z \approx 0.104\ \mathrm{m}$ (cavity centre 0.105 m): a typical two-gap HWR. The field-weighted centres of the two lobes are $d = 0.081\ \mathrm{m}$ apart, so $d = \beta_g\lambda/2$ gives $\beta_g \approx 0.088$. In the viewer below, the longitudinal cut of `edz` shows the two gaps of opposite sign directly. Then look at `bdx` and `bdy`: near the beam hole the RF magnetic field is only of order millitesla (raw map values: at most about 3.4 mT over the whole map, while the electric field reaches about 17 MV/m, 9.6 MV/m on axis). This is "largest voltage, smallest current in the middle of an HWR" at work. Note also that `bdx` is much stronger than `bdy` (largest values 3.4 and 0.65 mT): the RF magnetic field of an HWR is far from axially symmetric, as [Field maps](#field-maps-interpolation) shows quantitatively.

```avas-field
map hwr010
```

> **Think about it**: acting on a proton with $\beta = 0.057$, which matters more, 1 mT of RF magnetic field or 2 MV/m of transverse electric field from the map?
>
> Answer: the ratio of magnetic to electric force is about $\beta c B/E = 0.057\times 3\times 10^8\times 10^{-3}/(2\times 10^6) \approx 1\,\%$, and the two are $90^\circ$ apart in time. A low-β proton feels almost only the electric field; switching Kb between 0 and $\pm 1.36$ changes the exit rms sizes of the preview by less than 0.4 %. The faster the particle, the more the magnetic term matters.

## Transit time: why a cavity picks its velocity {#rf-cavity-energy-gain}

### The general energy gain

Let the reference particle reach the cavity entrance at $t_{in}$ and position $z$ at $t(z)$. In AVAS's convention the on-axis field is $K_eE_z(z)\cos\varphi(z)$ with

$$
\varphi(z) = \omega\,\bigl(t(z) - t_{in}\bigr) + \varphi_0
$$

where $\varphi_0$ is the RF phase when the particle arrives at the entrance. The energy gain is the work of the electric force along the actual trajectory:

$$
\Delta W = qK_e\int_0^L E_z(z)\cos\varphi(z)\,\dd z
$$

This is exact (for the reference particle on axis), but it mixes "field" and "timing". To separate them, define the integrals $C = \int E_z\cos\varphi\,\dd z$ and $S = \int E_z\sin\varphi\,\dd z$ and set

$$
\varphi_s = \operatorname{atan2}(S,\,C),\qquad V_0T = K_e\sqrt{C^2+S^2}
$$

A trigonometric identity then gives $\Delta W = qV_0T\cos\varphi_s$. $\varphi_s$ measures how far the reference particle's phase is from "the most accelerating phase on this trajectory"; it is what AVAS calls the **synchronous phase** for V3 = 0. $V_0T$ is the voltage the particle would get with the best timing.

### Constant velocity: the transit-time factor

To see what timing does, approximate: the particle's velocity is constant in the cavity, so $\varphi(z) = 2\pi z/(\beta\lambda) + \varphi_0$. Each time the particle travels $\beta\lambda$, the RF turns through a full cycle. With the origin at the cavity centre this gives the classical form

$$
\Delta W = qV_0T(\beta)\cos\varphi_s,\qquad
V_0 = \int\lvert E_z\rvert\,\dd z,\qquad
T(\beta) = \frac{\left\lvert\int E_z(z)\,\ee^{\,i\,2\pi z/(\beta\lambda)}\,\dd z\right\rvert}{\int\lvert E_z\rvert\,\dd z}
$$

$V_0$ is the voltage the particle would collect if the field were frozen at its maximum and the particle crossed instantly; $T \le 1$ is the transit-time factor, the Fourier component of the field distribution at spatial frequency $2\pi/(\beta\lambda)$. The physics is simple: crossing a gap takes time, and the field changes meanwhile; when the crossing time is comparable to half an RF period, the two halves of the crossing cancel. Two standard results:

- A single gap of length $g$ with uniform field: $T_g = \dfrac{\sin(\pi g/\beta\lambda)}{\pi g/\beta\lambda}$. The shorter the gap and the faster the particle, the closer $T_g$ is to 1.
- Two identical gaps of opposite sign, centre distance $d$ (QWR / HWR): $T(\beta) = T_g(\beta)\,\left\lvert\sin\dfrac{\pi d}{\beta\lambda}\right\rvert$. For $d = \beta\lambda/2$ the second factor is 1; that is what geometric β means.

For the example map hwr010 ($K_e = 1.36$), $V_0 = K_e\int\lvert E_z\rvert\,\dd z \approx 0.978\ \mathrm{MV}$, and integrating the real on-axis field gives:

| β | 0.05 | 0.057 (injection) | 0.07 | 0.08 | 0.10 | 0.1035 (max) | 0.12 | 0.15 | 0.20 |
|---|---|---|---|---|---|---|---|---|---|
| $T$ | 0.21 | 0.40 | 0.65 | 0.75 | 0.82 | 0.82 | 0.80 | 0.73 | 0.61 |

$T$ falls steeply for $\beta < \beta_g$ and more slowly above: it stays above 90 % of its maximum for $0.079 \le \beta \le 0.147$ (proton kinetic energy about 2.9–10.4 MeV). **One cavity type serves only a band of β efficiently**, which is why a linac changes cavity type several times ([From source to target](#journey-overview)).

Treating each gap roughly as a uniform field 40 mm long (the half-height width of one on-axis lobe), the two-gap formula above gives $T \approx 0.52$ and $0.91$ at $\beta = 0.057$ and $0.10$, while the real field gives 0.40 and 0.82: the crude model has the right trend, but the numbers need the real field.

> **Think about it**: the gap spacing gives a geometric β of 0.088. Why is the maximum of $T(\beta)$ at $\beta \approx 0.104$?
>
> Answer: $T = T_g\cdot\lvert\sin(\pi d/\beta\lambda)\rvert$ is a product of two factors. The second peaks at $\beta = 0.088$, but the single-gap factor $T_g$ rises monotonically with β (the faster the particle, the less the field changes while it crosses one gap), which pushes the maximum of the product to higher β. "Geometric β" and "β of maximum $T$" are therefore different numbers; the β in a cavity's name usually means the latter or a design value, so say which one you quote.

### At low energy constant velocity fails

In the example the beam goes from 1.527 MeV to about 1.87 MeV inside the cavity, and $\beta$ from 0.057 to 0.063, a 10 % change. The constant-velocity estimate of the largest gain is $V_0T(0.057) = 0.978\times 0.40 \approx 0.39\ \mathrm{MeV}$, while integrating along the real trajectory (scanning the entrance phase with V3 = 1) gives 0.482 MeV: the particle speeds up in the first gap, arrives at the second with a β closer to $\beta_g$, and the timing there works out better. Integrating along the real trajectory:

| Setting | Entrance phase $\varphi_0$ | $\Delta W$ / MeV | Resulting $V_0T$ / MV |
|---|---|---|---|
| V3 = 0, $\varphi_s = 0^\circ$ | $95.4^\circ$ | 0.475 | 0.475 |
| V3 = 0, $\varphi_s = -30^\circ$ | $56.6^\circ$ | 0.357 | 0.412 |
| V3 = 0, $\varphi_s = -33^\circ$ | $52.7^\circ$ | 0.340 | 0.405 |
| Maximum of a V3 = 1 scan | $\approx 105.5^\circ$ | 0.482 | — |

So $V_0T$ itself depends on the phase: a different phase gives a different energy gain in the first gap, hence a different speed and arrival time at the second. The largest gain is not at $\varphi_s = 0$ but about $10^\circ$ later in entrance phase (where $\varphi_s \approx +8^\circ$). At $\varphi_s = -33^\circ$ the particle is first decelerated in the front half of the first gap (about 1.44 MeV at $z = 0.05\ \mathrm{m}$, lowest about 1.40 MeV) and only then accelerated, typical for $\beta \ll \beta_g$.

Conversely, when the beam's β is close to $\beta_g$ the approximation is good: with an injection energy of 4.727 MeV ($\beta = 0.100$) the largest gain of the phase scan, 0.803 MeV, is almost the same as $V_0T(0.10) = 0.978\times 0.82 = 0.80\ \mathrm{MeV}$. This is also why AVAS does not replace the cavity by a "$V_0T\cos\varphi$ thin lens" but integrates step by step in the 3D field map with its t-code (see [Field maps](#field-maps)): at low energy, gain, phase and focusing are all entangled with the trajectory itself.

The lab below sets the injection energy to 4.727 MeV; the phase scan is nearly symmetric and $\Delta W/\Delta W_{max}$ should be close to $\cos\varphi_s$. Drag W back to 1.527 MeV and watch the curve become lopsided.

```avas-lab
rf-cavity phase=-30 Ke=1.36 W=4.727
```

## Phase: acceleration against bunching {#rf-cavity-phase}

### Why the synchronous phase is negative

The particles of a bunch do not all arrive at once. Suppose the synchronous particle sits on the side where the electric field **rises** in time: with AVAS's cosine convention that is $-90^\circ < \varphi_s < 0$. A particle arriving earlier has a smaller phase, a smaller $\cos\varphi$, and gains less energy; a later one gains more. At low energy a particle with more energy runs faster and catches up by the next cavity, one with less energy falls behind. Early particles are pulled back, late ones pushed forward, and the particles oscillate around the synchronous particle: the bunch holds together. This is the **principle of phase stability**; details in [Longitudinal motion](#longitudinal). On crest ($\varphi_s = 0$) all particles gain the same energy to first order and there is no restoring force; on the falling side ($\varphi_s > 0$) early particles gain more and the bunch is stretched.

The price is less acceleration: the synchronous particle gets only $\cos\varphi_s$ of the peak, while the longitudinal "capture range" (the phase width of the stable region is about $3\lvert\varphi_s\rvert$) requires $\lvert\varphi_s\rvert$ not to be too small. The low-energy part usually compromises between $-40^\circ$ and $-20^\circ$; the example uses $-33^\circ$.

### The trade-off in the example

One cavity placed directly at the start of the beam line (no 5 cm drift in front, no space charge, linear preview) at different synchronous phases, at the cavity exit:

| $\varphi_s$ | $W_{out}$ / MeV | exit rms $x$ / mm | exit rms $z$ / mm |
|---|---|---|---|
| $-60^\circ$ | 1.694 | 1.96 | 0.62 |
| $-33^\circ$ | 1.867 | 1.81 | 0.70 |
| $0^\circ$ | 2.002 | 1.50 | 1.10 |
| $+33^\circ$ | 1.951 | 1.20 | 1.74 |
| Drift of the same length | 1.527 | 1.41 | 1.48 |

At the entrance rms $x \approx 0.95\ \mathrm{mm}$ and $z \approx 1.32\ \mathrm{mm}$; the initial beam is diverging. The pattern is clear: the more negative $\varphi_s$, the tighter the longitudinal squeeze and the stronger the transverse push; for $\varphi_s > 0$ the beam is focused transversely and debunched longitudinally; on crest the energy is highest but the longitudinal plane is hardly controlled. The next section explains why longitudinal and transverse always go opposite ways. At the example current of 0.184 mA, space charge changes the exit rms sizes of the single cavity in the linear preview by less than 0.5 %.

### Where is the crest: phase scans

In a real machine nobody knows which reading of the RF source corresponds to "$\varphi_s = -30^\circ$"; it is found with a **phase scan**. The cavity phase is stepped, the exit energy is measured by time of flight between downstream beam position monitors, giving a curve $W_{out}(\varphi_0)$; the phase of largest energy (the "crest") is found and the design $\varphi_s$ is set as an offset from it. The lab below is such a phase-scan curve, with the current setting marked. Drag `phase` from $-90^\circ$ to $0^\circ$ and watch exit energy, rms $z$ and rms $x/y$ trade against each other; compare with the table above. Then lower Ke from 1.36 to 0.7 and see where the synchronous point moves on the curve.

```avas-lab
rf-cavity phase=-30 Ke=1.36 W=1.527
```

At $W = 1.527\ \mathrm{MeV}$ and $\varphi_s = -30^\circ$ the lab gives $\Delta W = 0.357\ \mathrm{MeV}$, only 74 % of the scan maximum of 0.482 MeV, while $\cos 30^\circ = 0.866$; the scan is also asymmetric (minimum near $\varphi_0 \approx -106^\circ$ with $\Delta W \approx -0.31\ \mathrm{MeV}$). With W set to 4.727 MeV the ratio returns to 0.866. Take care with very low W: far below the geometric β the two gaps nearly cancel, the synchronous phase as defined covers only part of the circle, and some $\varphi_s$ have no solution at all. For example at Ke = 1.36 and $\varphi_s = -30^\circ$: scanning the entrance phase at 0.3 MeV gives $\varphi_s$ only between about $65^\circ$ and $107^\circ$, at 1.0 MeV between about $14^\circ$ and $144^\circ$; by the linear preview, an accelerating $-30^\circ$ solution exists only at 0.4–0.65 MeV and above 1.1 MeV. At these velocities the two gaps nearly cancel, the net accelerating vector $\int E\,e^{i\omega t}\,\dd z$ is close to zero, and the energy change inside the cavity is enough to keep it from going round the origin. Without an accelerating solution the engine takes the one $180^\circ$ away: at 0.7–0.75 MeV and 1.05 MeV $\varphi_s = 150^\circ$ can be reached and the cavity decelerates (checked on 2026-11-02: at 0.7 MeV the engine's synchronous particle loses 0.113 MeV, while synData.txt still writes $-30^\circ$); the linear preview does the same and warns that the cavity decelerates. Where neither exists (0.15–0.35 MeV, 0.8–1.0 MeV) the engine refuses to run with an initialisation error saying the "scan phase" finds no RF phase (checked at 0.3 and 1.0 MeV), and the linear preview warns, gives the reachable range and continues with the closest synchronous phase. The lab's "synchronous phase" always shows the value actually reached.

> **Think about it**: an operator says "this cavity is set $-30^\circ$ from crest". Is that the same setting as V3 = 0 with `phase` = −30 in AVAS?
>
> Answer: only when $\beta \approx \beta_g$. At low energy AVAS's $\varphi_s = 0$ is not the phase of largest energy: in the example the entrance phase $95.4^\circ$ for $\varphi_s = 0$ is about $10^\circ$ earlier than the entrance phase of maximum energy. "$-30^\circ$ from crest" corresponds in the example to an entrance phase of $75.5^\circ$, not to AVAS's $\varphi_s = -30^\circ$ (entrance phase $56.6^\circ$). To reproduce a machine setting, writing the entrance phase with V3 = 1 or V3 = 2 is more direct.

## Transverse defocusing: the price of bunching {#rf-cavity-transverse}

### Why it cannot be avoided: the bunch rest frame

Ride along with the synchronous particle and look at the field around the bunch. The RF wavelength, $\lambda = 1.85\ \mathrm{m}$, is much larger than the bunch (rms length about 1 mm); in this frame the field changes slowly and can be treated as quasi-electrostatic, so the potential satisfies Laplace's equation

$$
\frac{\partial^2\Phi}{\partial x^2} + \frac{\partial^2\Phi}{\partial y^2} + \frac{\partial^2\Phi}{\partial z^2} = 0
$$

The strength of a linear restoring force is proportional to the second derivative of the potential, so the three "spring constants" add up to zero: a potential well along $z$ (longitudinal focusing) must come with a potential hill in the two transverse directions together (defocusing). For an axially symmetric gap each transverse direction gets half the longitudinal strength with the opposite sign. This is often called the RF form of Earnshaw's theorem: **a longitudinally stable phase is necessarily transversely defocusing**, and vice versa. It relies only on "no charges near the bunch, field nearly static" and not on the shape of the cavity; space charge and magnetic fields are not part of the argument.

### In the laboratory frame: the two halves of a gap

In the laboratory frame the same thing happens like this. Expanding the field near the axis in $r$, $\nabla\cdot\boldsymbol{E} = 0$ and $\nabla\times\boldsymbol{B} = c^{-2}\,\partial\boldsymbol{E}/\partial t$ give for an axially symmetric gap

$$
E_r \approx -\frac{r}{2}\,\frac{\partial E_z}{\partial z},\qquad
B_\theta \approx \frac{r}{2c^2}\,\frac{\partial E_z}{\partial t}
$$

The radial force on the particle is $F_r = q(E_r - vB_\theta)$. At the gap entrance $\partial E_z/\partial z > 0$, field lines converge towards the axis and push the particle in (focusing); at the exit $\partial E_z/\partial z < 0$ and the particle is pushed out (defocusing). If the field were constant in time and the velocity constant, the two halves would cancel exactly, the transverse version of "an electrostatic field does no net work" from the opening. With $\varphi_s < 0$ the field grows while the particle crosses, the second (defocusing) half is stronger than the first (focusing) half, and the net effect is defocusing. In the [hwr010 viewer above](#rf-cavity-modes), look at the $x$–$y$ cross-sections of `edx` and `edy` and move the cut from the entrance of a gap to its exit: the transverse electric field reverses, the focusing and the defocusing half.

### Thin-gap model

Treating one gap as a thin lens at its centre and integrating the radial force over the gap (Wangler, chapter 7) gives

$$
\Delta(\beta\gamma\,x') = -\frac{\pi\,qE_0TL\,\sin\varphi}{mc^2\,\lambda\,\beta^2\gamma^2}\;x
$$

and the same for $y$. $qE_0TL$ is the energy-gain amplitude of the gap (MeV, $q$ in units of $e$), and $\beta\gamma$ is taken at the gap centre. For $\varphi < 0$ the right-hand side has the sign of $x$: transverse defocusing. Its strength grows as $1/(\beta\gamma)^2$ ($1/(\beta\gamma)^3$ after dividing by the momentum), so it is worst at low energy.

The longitudinal action of the same gap: let $z > 0$ mean a particle ahead of the reference (arriving earlier, smaller phase), $\Delta\varphi = -2\pi z/(\beta\lambda)$, so

$$
\Delta\delta = \frac{2\pi\,qE_0TL\,\sin\varphi}{mc^2\,\lambda\,\beta^3\gamma}\;z
$$

For $\varphi < 0$ an early particle's $\delta$ decreases: longitudinal focusing. Including the momentum increase from the energy gain, the thin-gap matrix in $(x, x')$ is

$$
M_x = \begin{pmatrix} 1 & 0 \\[2pt] -\dfrac{\pi\,qE_0TL\sin\varphi}{mc^2\lambda\,\bar\beta^2\bar\gamma^2\,(\beta\gamma)_{out}} & \dfrac{(\beta\gamma)_{in}}{(\beta\gamma)_{out}} \end{pmatrix}
$$

The ratio in the lower right is **adiabatic damping**: the transverse momentum is unchanged while the longitudinal momentum grows, so $x'$ shrinks. In the example cavity $(\beta\gamma)_{in}/(\beta\gamma)_{out} = 0.05708/0.06311 \approx 0.90$: the geometric emittance shrinks by about 10 %, the normalized emittance stays the same.

### How strong, longitudinally and transversely

For a periodic arrangement, averaged per unit length, the longitudinal and RF transverse focusing constants (Wangler's notation) are

$$
k_{l0}^2 = \frac{2\pi\,qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3},\qquad
k_{t,\mathrm{rf}}^2 = -\frac{\pi\,qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3} = -\frac{k_{l0}^2}{2}
$$

exactly the "each transverse direction gets half, with the opposite sign" of the rest-frame argument. Both are proportional to $\sin(-\varphi_s)$ and $1/(\beta\gamma)^3$: at low energy a larger $\lvert\varphi_s\rvert$ bunches harder and needs stronger transverse focusing. This is one reason why low-energy sections use lower frequencies and a compromise phase around $-30^\circ$; see [Longitudinal motion](#longitudinal) and [Lattice design](#lattice-design).

Orders of magnitude in the example, first the quick way: at $\varphi_s = -33^\circ$, give each gap half of the 0.405 MeV and the synchronous phase $-33^\circ$; with the injection $\beta\gamma$ the thin-gap formula gives $1/f \approx 1.08\ \mathrm{m^{-1}}$ per gap, about $2.15\ \mathrm{m^{-1}}$ for the cavity. Inserting the cavity's average field $E_0T \approx 0.405\ \mathrm{MV}/0.21\ \mathrm{m} = 1.93\ \mathrm{MV/m}$ into the smooth formula gives $k_{t,\mathrm{rf}}^2 \approx -10.3\ \mathrm{m^{-2}}$, which times 0.21 m is the same $2.15\ \mathrm{m^{-1}}$ (it is the same formula averaged over the length). A thin defocusing lens of this strength in the middle of the cavity, plus adiabatic damping, takes the example beam to an exit rms $x$ of about 1.67 mm; the preview gives 1.81 mm.

The difference is instructive. Far below $\beta_g$ the two gaps do not see the same phase. Integrating each gap separately along the real trajectory at $\varphi_s = -33^\circ$: the first gap sits at about $-81^\circ$ of its own crest (amplitude $qE_0TL \approx 0.27\ \mathrm{MeV}$, but it gives only $+0.04\ \mathrm{MeV}$), the second at about $+9^\circ$ (amplitude 0.30 MeV, gain $+0.30\ \mathrm{MeV}$). The defocusing goes with $\sin\varphi$, not with the gain: the first gap, which hardly accelerates, defocuses strongly ($1/f \approx +3.0\ \mathrm{m^{-1}}$), the second gap even focuses slightly ($1/f \approx -0.37\ \mathrm{m^{-1}}$). Net about $2.6\ \mathrm{m^{-1}}$, a focal length of about 0.38 m; two thin lenses with these strengths at the two gap centres reproduce the preview's 1.81 mm (against 1.41 mm for a drift of the same length). The single number $\varphi_s$ of a cavity hides how the phase is shared between its gaps, and at low β that sharing decides the transverse effect.

> **Think about it**: since $\varphi_s > 0$ focuses transversely, could some cavities run at a positive phase instead of using solenoids?
>
> Answer: a cavity at $\varphi_s > 0$ debunches longitudinally. Laplace's equation only lets you move focusing from one direction to another, never focus all three at once. Alternating positive and negative phases can indeed give net 3D stability, like a FODO lattice (alternating-phase focusing, APF). It costs acceleration and acceptance, so it has been used only in a few compact low-energy structures; proton linacs normally use magnets for transverse focusing.

> **AVAS convention**: the engine does not use the thin-gap matrix above. It integrates the Lorentz force directly in the field map's $\boldsymbol{E}(x,y,z)$ and $\boldsymbol{B}(x,y,z)$ (t-code), so nonlinearity, transverse–longitudinal coupling and the non-axisymmetric parts of the HWR are included naturally. The linear preview follows the reference particle in 1 mm steps and builds the transverse optics from 4 mm slices, using the near-axis field gradients and a linearized RF bunching.

## Power, quality factor and superconductivity {#rf-cavity-figures}

Back to the LC picture: a real inductor has resistance, and every period a little energy turns into heat in the walls. The quantities below decide how much power a cavity costs, how much voltage it gives and where the field limit is. They belong to cavity design; AVAS's beam dynamics does not use them directly, but keep them in mind when choosing Ke.

| Quantity | Definition | Meaning and typical size |
|---|---|---|
| Unloaded quality factor $Q_0$ | $Q_0 = \omega U / P_c$ | Stored energy $U$ over wall loss per radian; room-temperature copper about $10^4$, superconducting niobium $10^9$–$10^{10}$ |
| Shunt impedance $R_{sh}$ | $R_{sh} = V^2 / P_c$ | Voltage per wall-loss power; linac convention, the circuit convention differs by 2 |
| $R/Q$ | $R/Q = V^2/(\omega U)$ | Depends on geometry only, not on the material; tens to hundreds of Ω for low-β superconducting cavities |
| Accelerating gradient $E_{acc}$ | $E_{acc} = V_{acc}/L_{eff}$ | For superconducting cavities often $L_{eff} = N_{gap}\,\beta_g\lambda/2$ |
| $E_{pk}/E_{acc}$, $B_{pk}/E_{acc}$ | Surface peak fields over gradient | Typically about 4–6 and 5–10 mT/(MV/m) for low-β cavities; they limit field emission and quench |

Here $V$ usually means the effective voltage $V_0T$ including the transit-time factor. From the definitions the wall loss is $P_c = V^2/[(R/Q)\,Q_0]$: $R/Q$ is fixed by the shape and the loss depends on $Q_0$ only. For an order of magnitude (assuming $R/Q = 100\ \Omega$, typical of low-β cavities, not the design value of hwr010): at $V = 0.4\ \mathrm{MV}$ a superconducting cavity ($Q_0 = 10^9$) dissipates about 1.6 W, a room-temperature copper cavity of the same shape ($Q_0 \sim 10^4$) about 160 kW. The superconducting cavity's 1.6 W must be removed at liquid-helium temperature, and each watt of heat load costs several hundred watts of refrigerator power (typical value), yet this is still far below the copper cavity: the main reason why continuous-wave (CW) proton linacs choose superconductivity.

The power the beam takes from the cavity is $P_b = I\,V_0T\cos\varphi_s$; in the example $I = 0.184\ \mathrm{mA}$ and $\Delta W = 0.34\ \mathrm{MeV}$, so $P_b \approx 63\ \mathrm{W}$.

## Common misconceptions {#rf-cavity-pitfalls}

- **"$\varphi_s = -30^\circ$ means $30^\circ$ from crest"**: true only for $\beta \approx \beta_g$. At the example's injection $\beta = 0.057$, AVAS's $\varphi_s = -30^\circ$ gets only 74 % of the largest gain, and the crest is at $\varphi_s \approx +8^\circ$ ([phase scan](#rf-cavity-phase)).
- **"$V_0T$ is a constant of the cavity"**: $T$ depends on β, and at low energy $V_0T$ also depends on the phase (0.405–0.475 MV, [low-energy deviation](#rf-cavity-energy-gain)). A cavity type suits only a band of β.
- **"Geometric β is the β of maximum $T$"**: in the example the gap spacing gives 0.088, while $T$ peaks at 0.104.
- **Mixing phase conventions**: AVAS uses $\cos(\omega t+\varphi)$ and bunching phases are negative; some books or codes use a sine convention or the zero crossing as reference, so the sign and value of the stable phase differ. When importing phases from another code (e.g. TraceWin), check its phase definition first.
- **"A cavity only acts longitudinally"**: the RF defocusing focal length of the example cavity is about 0.4 m, comparable to a weak solenoid, and must be part of the transverse design.
- **"Acceleration reduces the emittance"**: adiabatic damping reduces the geometric emittance (about 10 % for the example cavity); the normalized emittance stays the same.
- **"The energy gain is strictly proportional to Ke"**: at low energy the trajectory changes with the field amplitude; $\Delta W/K_e$ is 0.245, 0.248, 0.250 and 0.251 MeV at Ke = 0.5, 1.0, 1.36 and 2.0: nearly but not strictly proportional.
- **"The magnetic field is small, so any Kb will do"**: at low β the effect of Kb is indeed small, but electric and magnetic field are two parts of the same eigenmode, and Kb should scale with Ke; at high β an inconsistent Kb produces a spurious transverse effect.

## In AVAS {#rf-cavity-in-avas}

### Syntax and parameters

An RF cavity is described by the field-map element `field` with field type 1:

```text
field  L  R  V3  1  f  phase  Ke  Kb  file
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
```

```avas-params
field
```

- `L` (m): element length; it should equal the length in the field map header (0.21 m in the example). `R` (m): aperture radius; particles beyond it are counted as lost.
- `V3`: sets the meaning of `phase` (see below).
- `f` (Hz): RF frequency of this cavity, `162.5e6` in the example. The beam (bunch repetition) frequency is given separately by `frequency` in beam.txt.
- `phase` (degrees): meaning set by V3.
- `Ke`, `Kb`: scale factors of electric and magnetic field; `file`: field map name (without extension). An RF cavity reads `.edx/.edy/.edz` and `.bdx/.bdy/.bdz`; format in [Field maps](#field-maps).

> **AVAS convention (verified with the engine)**: electric fields in the map are in MV/m and multiplied by Ke; magnetic fields are in T and multiplied by Kb. The RF electric field acts as $K_e\,\boldsymbol{E}(\boldsymbol{r})\cos(\omega t+\varphi_0)$ and the RF magnetic field as $K_b\,\boldsymbol{B}(\boldsymbol{r})\sin(\omega t+\varphi_0)$, consistent with the $90^\circ$ between E and B in the [pillbox mode](#rf-cavity-modes).

### V3: phase reference

| V3 | Meaning of `phase` | Use |
|---|---|---|
| 0 | Synchronous phase $\varphi_s = \operatorname{atan2}(\int E\sin\varphi\,\dd z,\ \int E\cos\varphi\,\dd z)$; the engine solves for the entrance phase | Design: set $\varphi_s$ directly; it is kept when upstream elements change |
| 1 | RF phase $\varphi_0$ when the reference particle reaches the element entrance | Phase scans, reproducing an entrance phase |
| 2 | Absolute RF phase at $t = 0$; entrance phase $\varphi_0 = \varphi + 360^\circ\cdot f\cdot t_{in}$ | Modelling a real machine: phases locked to the master clock; upstream energy changes alter the arrival time and the actual $\varphi_s$ |

The engine writes the reference particle's time and energy at each element entrance, together with $\varphi_s$ and $\varphi_{RF}$, to `synData.txt`; setting one yields the other (`φRF = phase_t0 + 360·f·t_in`). When a segment run takes a middle part on its own, V3 = 2 phases must be converted with $\varphi_{RF} - 360^\circ\cdot f\cdot(t_{in} - T_{entry})$, not kept as they are or changed to V3 = 0; AVAS's segment runs do this automatically (`segment.rephase`).

### Ke and Kb

- **Ke is the amplitude knob**. At $\varphi_s = -33^\circ$ the example gives $\Delta W$ = 0.122, 0.248, 0.340 and 0.502 MeV for Ke = 0.5, 1.0, 1.36 and 2.0. With Ke = 1.36, $V_0 \approx 0.978\ \mathrm{MV}$.
- **Kb should change in proportion to Ke**. Electric and magnetic field of a cavity are two parts of the same eigenmode; changing Ke without Kb is physically inconsistent. The example uses $K_b = -K_e$. The manual does not explain the minus sign, but the map itself does: the circulation of the map's $\boldsymbol{B}$ around the axis equals $+(\omega/c^2)$ times the flux of the map's $E_z$ to within 2 % (see [Field maps](#field-maps-interpolation)), i.e. the map stores $\boldsymbol{B}$ as the amplitude of a field varying as $-\sin$, as $B_\theta$ of the pillbox mode above. With AVAS's $+\sin$ convention, only $K_b = -K_e$ makes the electric and magnetic field one Maxwell solution. For maps from other sources, determine the sign of $K_b/K_e$ the same way (or from the export convention) and keep the ratio fixed.
- For low-β protons the RF magnetic field matters much less than the electric field (see [Think about it](#rf-cavity-modes)): switching Kb between 0 and $\pm 1.36$ in the example changes the exit rms sizes of the linear preview by less than 0.4 %.

> **AVAS convention**: the cavity field amplitude in AVAS is set directly by Ke (and Kb) and is fixed. The engine does not model the RF source, beam loading, detuning or low-level control; amplitude and phase jitter are introduced with the error commands `err_cav_*`, see [Errors and correction](#errors).

> **Note**: `scanphase` (input.txt) also affects phases: 0 uses the phases in the lattice, 1 lets the engine scan the cavity phases, 2 reads each cavity's entrance phase and entrance time from `scanData.txt` (one line per cavity, in the order of the RF cavities in the lattice; the example's `scanData.txt` holds 52.66°, exactly the entrance phase for $\varphi_s = -33^\circ$). The linear preview ignores `scanphase`.

```avas-params
input:scanphase input:steppercycle input:longlimits
```

### Typical values

| Parameter | Role | Example / typical range |
|---|---|---|
| `phase` (V3 = 0) | Synchronous phase: compromise between acceleration, longitudinal acceptance and transverse defocusing | Example $-33^\circ$; low-energy sections often $-40^\circ$ to $-20^\circ$, higher energies closer to crest |
| `Ke` | Electric field amplitude | Example 1.36 ($V_0 \approx 0.98\ \mathrm{MV}$); upper limit set by $E_{pk}$, $B_{pk}$ |
| `Kb` | Magnetic field amplitude | In proportion to Ke, example $-1.36$ |
| `f` | Cavity frequency | Example 162.5 MHz; low-β superconducting sections commonly 162.5 or 325 MHz |
| `R` | Loss aperture | Example 0.02 m; should match the real beam pipe / drift tube aperture |
| input.txt `steppercycle` | Integration steps per RF period | Example 50: $\beta\lambda/50 \approx 2.1\ \mathrm{mm}$, comparable to the 2 mm map grid |

### Choosing phases in a design

1. Choose each cavity's $\varphi_s$ from the analysis in [Longitudinal motion](#longitudinal): the longitudinal acceptance (phase width about $3\lvert\varphi_s\rvert$) must hold the bunch, and longitudinal and transverse phase advances must avoid parametric resonances; see [Lattice design](#lattice-design).
2. In the design phase write $\varphi_s$ directly with V3 = 0. When upstream elements change, the engine solves for the entrance phase again and every cavity keeps its synchronous phase.
3. Once the design is fixed, read each cavity's entrance RF phase from `synData.txt` and switch to V3 = 1 or V3 = 2 where needed, to study phase errors or reproduce machine settings.

### Phase scans in AVAS

Set the cavity to V3 = 1, then use a parameter scan on the Run page or scan the entrance phase point by point from the command line:

```text
avas scan --input InputFile --target 2 --param phase --values -180:170:36
```

`--target 2` is line 2 of the lattice file (the cavity's line in the example); an element name works too. `start:stop:count` gives 36 points $10^\circ$ apart; the metric `energy_out` is the exit energy. With the curve in hand:

1. Find the maximum. In the example it is near $\varphi_0 \approx 105.5^\circ$ with $\Delta W_{max} \approx 0.482\ \mathrm{MeV}$; the minimum is near $\varphi_0 \approx -106^\circ$ with $\Delta W \approx -0.31\ \mathrm{MeV}$: the curve is not symmetric.
2. Note that "$-33^\circ$ from crest" and AVAS's $\varphi_s = -33^\circ$ differ at low energy (see [Phase](#rf-cavity-phase)).
3. When the beam's β approaches the geometric β they coincide: at 4.727 MeV injection ($\beta = 0.10$) the scan is nearly symmetric, the largest $\Delta W \approx 0.803\ \mathrm{MeV}$ occurs at $\varphi_s \approx 0$, and at $\varphi_s = -33^\circ$, $\Delta W \approx 0.673\ \mathrm{MeV}$, exactly $0.803\cos 33^\circ$.

### Pitfalls

- **The map does not contain the cavity surface**: maps usually cover only the region near the beam aperture (the example only $\lvert x\rvert, \lvert y\rvert \le 2\ \mathrm{cm}$), so $E_{pk}$ cannot be read from an AVAS map; use the $E_{pk}/E_{acc}$ of the cavity design report to check that the chosen Ke is within operating limits.
- **β mismatch**: when the beam's β is well below the cavity's geometric β, $T$ is small and the first gap may decelerate first. With the example's injection $\beta = 0.057$ the cavity gives only about 35 % of $V_0$ at $\varphi_s = -33^\circ$.
- **Longitudinal nonlinearity**: when the bunch phase width is comparable to $\lvert\varphi_s\rvert$, particles fall outside the separatrix, with longitudinal emittance growth or losses that the linear preview cannot show; the multi-particle engine is needed. The longitudinal loss limits are set with `longlimits` in input.txt.
- **Field map noise**: the example maps are noisy (the on-axis $E_z$ jumps visibly from point to point). That is a property of the data, not a reading error; see [map noise](#field-maps-interpolation).
- **Superposed fields**: when a cavity overlaps the fringe field of a solenoid, use `superpose`; one superposed group may contain at most one RF cavity; see [Superposed fields](#superpose).
- **First and last element**: in multi-particle runs the first and last element should not be matrix-model elements; an RF cavity is a field-map element and is not affected.

## Further reading {#rf-cavity-reading}

- T. P. Wangler, *RF Linear Accelerators* (2nd ed.), chapters 2–5 (resonant cavities, transit-time factor, shunt impedance) and 6–7 (longitudinal and transverse dynamics, thin-gap formulas).
- H. Padamsee, J. Knobloch, T. Hays, *RF Superconductivity for Accelerators*: $Q_0$, $R/Q$, geometry factor and peak fields of superconducting cavities.
- CAS – CERN Accelerator School proceedings, lectures on RF cavities and linacs.
- AVAS manual `docs/使用说明20260427.docx`: the `field` element and its phase parameters.
- In this primer: [Longitudinal motion](#longitudinal), [Field maps](#field-maps), [Superposed fields](#superpose), [Errors and correction](#errors), [Tutorial](#tutorial), [References](#references).
