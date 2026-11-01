# RF cavities

RF cavities are the only elements of a linac that give the beam energy: the particles "surf" on a resonant electromagnetic field, and the same field also bunches the beam longitudinally and focuses or defocuses it transversely. AVAS does not approximate a cavity by a transfer matrix; it reads the cavity's three-dimensional field map (the `field` element, field type 1) and integrates the particle motion step by step in the real field (t-code). This chapter starts from the basic cavity modes, derives the energy gain, the transit-time factor and RF focusing, and ends with the parameters of `field`, the phase conventions and phase scans in AVAS.

All numerical examples use the example project `examples/hwr010`: protons with injection kinetic energy $W = 1.527\ \mathrm{MeV}$, RF frequency $f = 162.5\ \mathrm{MHz}$, and the half-wave resonator field map `hwr010` (0.21 m long, two accelerating gaps). The numbers were computed with AVAS's own linear envelope preview (`avas.sim.linear_optics`); the kernel result is the reference.

## Role and position {#rf-cavity-role}

In a linac the RF cavities do three things:

- **Acceleration**: the synchronous particle gains $\Delta W = qV_0T\cos\varphi_s$ per cavity (derived below).
- **Longitudinal focusing**: with the synchronous phase on the rising slope of the field (in AVAS's cosine convention $-90^\circ < \varphi_s < 0$), early particles gain less energy and late particles more, so the bunch oscillates around the synchronous particle in longitudinal phase space instead of spreading out; see [Longitudinal motion](#longitudinal).
- **Transverse defocusing**: by Maxwell's equations, a longitudinally focusing phase necessarily defocuses transversely (the RF form of the Earnshaw / Panofsky theorem), so solenoids or quadrupoles between the cavities have to compensate; see [Solenoid](#solenoid) and [Quadrupole](#quadrupole).

The low-energy part ($\beta \lesssim 0.2$) of a superconducting proton linac is usually a periodic structure of "one or a few cavities + one solenoid", exactly what the [tutorial](#tutorial) builds. The relative energy change in a cavity is large and the velocity changes noticeably, so thin-lens and constant-velocity approximations are only rough estimates there; t-code integration through real field maps is needed.

## Resonant cavities and modes {#rf-cavity-modes}

### The TM010 mode of a pillbox

The simplest accelerating cavity is a perfectly conducting cylinder of radius $R$ and length $d$. Solving the wave equation in cylindrical coordinates with zero tangential electric field on the walls, the accelerating TM010 mode has only $E_z$ and $B_\theta$:

$$
E_z(r,t) = E_0\,J_0\!\left(\frac{2.405\,r}{R}\right)\cos\omega t,\qquad
B_\theta(r,t) = -\frac{E_0}{c}\,J_1\!\left(\frac{2.405\,r}{R}\right)\sin\omega t
$$

where $2.405$ is the first zero of $J_0$ (so that $E_z = 0$ at $r = R$). The resonant frequency is

$$
f_{010} = \frac{2.405\,c}{2\pi R}
$$

which depends on the radius only, not on the length. The form of $B_\theta$ follows directly from Faraday's law $\nabla\times\boldsymbol{E} = -\partial\boldsymbol{B}/\partial t$: $(\nabla\times\boldsymbol{E})_\theta = -\partial E_z/\partial r = E_0 k J_1(kr)\cos\omega t$ (with $k = 2.405/R$ and $J_0' = -J_1$); integrating over time, the magnetic field varies as $\sin\omega t$, $90^\circ$ out of phase with the electric field. **This is the physical origin of AVAS's convention of cos for the RF electric field and sin for the RF magnetic field** (see [In AVAS](#rf-cavity-in-avas)).

For $f = 162.5\ \mathrm{MHz}$ one gets $R = 2.405c/(2\pi f) \approx 0.706\ \mathrm{m}$, a cavity about 1.4 m across, which is bulky and inefficient for low-energy protons. Low-frequency, low-$\beta$ structures therefore use TEM-type coaxial resonators.

### Quarter-wave and half-wave resonators

- **Quarter-wave resonator (QWR)**: a coaxial line shorted at one end and open at the other, inner conductor about $\lambda/4$ long. The beam crosses a drift tube near the open end of the inner conductor, which forms two accelerating gaps. The structure is not up–down symmetric; the field contains a dipole component that deflects the beam vertically depending on the phase.
- **Half-wave resonator (HWR)**: a coaxial line shorted at both ends, inner conductor about $\lambda/2$ long ($\lambda/2 \approx 0.92\ \mathrm{m}$ at 162.5 MHz), with the beam crossing in the middle. An HWR is up–down symmetric about the beam axis and has no QWR-like dipole steering, but the field in the cross-section is not fully axisymmetric and contains a quadrupole component.
- **Multi-gap structures**: spoke cavities, elliptical cavities and others have three or more gaps. For AVAS each is "just a field map"; all the physical differences are in the field distribution.

In a two-gap cavity the fields in the two gaps point in opposite directions at any instant. If a particle needs exactly half an RF period to go from the centre of the first gap to the centre of the second, both gaps accelerate it; the corresponding gap-centre distance is $\beta_g\lambda/2$, where $\beta_g$ is the cavity's **geometric beta**.

### The example cavity hwr010

The on-axis $E_z$ of the example map has one peak at $z \approx 0.074\ \mathrm{m}$ and one at $z \approx 0.140\ \mathrm{m}$, of opposite sign, with the zero crossing at $z \approx 0.104\ \mathrm{m}$ (the centre of the cavity): a typical two-gap HWR. The constant-velocity transit-time factor is largest at $\beta \approx 0.10$ (see [Energy gain](#rf-cavity-energy-gain)), consistent with the name hwr010; the example beam has $\beta = 0.057$, far below the geometric beta, which is why the energy gain in this chapter's examples is low and the phase effects are pronounced.

```avas-field
map hwr010
```

## Quality factor, shunt impedance and peak fields {#rf-cavity-figures}

These quantities decide how much power a cavity needs, how much voltage it gives and where the field limit is. They belong to cavity design; AVAS's beam dynamics does not use them directly, but you need them in mind when choosing Ke.

| Quantity | Definition | Meaning and typical magnitude |
|---|---|---|
| Unloaded quality factor $Q_0$ | $Q_0 = \omega U / P_c$ | stored energy $U$ over loss per radian; room-temperature copper about $10^4$, superconducting niobium $10^9$–$10^{10}$ |
| Shunt impedance $R_{sh}$ | $R_{sh} = V^2 / P_c$ | voltage per unit wall loss; the linac convention, the circuit convention differs by a factor 2 |
| $R/Q$ | $R/Q = V^2/(\omega U)$ | depends on the geometry only, not on the material; tens to hundreds of Ω for low-β superconducting cavities |
| Accelerating gradient $E_{acc}$ | $E_{acc} = V_{acc}/L_{eff}$ | for superconducting cavities usually $L_{eff} = N_{gap}\,\beta_g\lambda/2$ |
| $E_{pk}/E_{acc}$, $B_{pk}/E_{acc}$ | surface peak field over gradient | for low-β cavities typically about 4–6 and 5–10 mT/(MV/m); they limit field emission and quench |

Here $V$ usually means the effective voltage $V_0T$ including the transit-time factor. From the definitions the wall loss is $P_c = V^2/[(R/Q)\,Q_0]$: for a given $R/Q$ the very high $Q_0$ of superconducting cavities brings the wall loss down to the watt level. The power the beam takes from the cavity is $P_b = I\,V_0T\cos\varphi_s$; for the example beam, $I = 0.184\ \mathrm{mA}$ and $\Delta W = 0.34\ \mathrm{MeV}$, so $P_b \approx 63\ \mathrm{W}$.

> **AVAS convention**: in AVAS the cavity field amplitude is given directly by Ke (and Kb) and is fixed. The kernel does not model the power source, beam loading, detuning or low-level RF control; field amplitude and phase jitter enter through the error commands `err_cav_*`, see [Errors and correction](#errors).

> **Note**: field maps usually cover only the region near the beam aperture (the example only up to $\lvert x\rvert, \lvert y\rvert \le 2\ \mathrm{cm}$) and not the cavity surface, so $E_{pk}$ cannot be read from an AVAS field map; check that the Ke you use is within the operating range with the $E_{pk}/E_{acc}$ of the cavity design report.

## Energy gain and transit-time factor {#rf-cavity-energy-gain}

### General expression

Let the reference particle reach the cavity entrance at $t_{in}$ and be at $z$ on the axis at time $t(z)$. In AVAS's convention the on-axis field is $K_e E_z(z)\cos\varphi(z)$ with

$$
\varphi(z) = \omega\,\bigl(t(z) - t_{in}\bigr) + \varphi_0
$$

where $\varphi_0$ is the RF phase when the particle reaches the entrance. The energy gain is the integral of the field along the actual trajectory:

$$
\Delta W = q K_e\int_0^L E_z(z)\cos\varphi(z)\,\dd z
$$

Define the two integrals $C = \int E_z\cos\varphi\,\dd z$ and $S = \int E_z\sin\varphi\,\dd z$, and let

$$
\varphi_s = \operatorname{atan2}(S,\,C),\qquad V_0T = K_e\sqrt{C^2+S^2}
$$

Then $\Delta W = qV_0T\cos\varphi_s$ holds exactly. This $\varphi_s$ is the **synchronous phase** AVAS means for V3 = 0: it measures how far the reference particle is from "the phase of maximum acceleration on this trajectory".

### Constant-velocity approximation and transit-time factor

If the velocity hardly changes in the cavity, $\varphi(z) = 2\pi z/(\beta\lambda) + \varphi_0$. With the origin at the cavity centre and a symmetric (or antisymmetric) field one obtains the classical form

$$
\Delta W = qV_0T(\beta)\cos\varphi_s,\qquad
V_0 = \int\lvert E_z\rvert\,\dd z,\qquad
T(\beta) = \frac{\left\lvert\int E_z(z)\,\ee^{\,i\,2\pi z/(\beta\lambda)}\,\dd z\right\rvert}{\int\lvert E_z\rvert\,\dd z}
$$

Some common results:

- A single gap of length $g$ with uniform field: $T = \dfrac{\sin(\pi g/\beta\lambda)}{\pi g/\beta\lambda}$.
- Two identical gaps of opposite sign with centre distance $d$ (QWR/HWR): $T(\beta) = T_g(\beta)\,\left\lvert\sin\dfrac{\pi d}{\beta\lambda}\right\rvert$, where $T_g$ is the single-gap factor. For $d = \beta\lambda/2$ the second factor is 1; this is the meaning of the geometric beta.

For the example map hwr010 ($K_e = 1.36$), $V_0 = K_e\int\lvert E_z\rvert\,\dd z \approx 0.98\ \mathrm{MV}$; the constant-velocity $T(\beta)$ is:

| β | 0.05 | 0.057 (injection) | 0.07 | 0.08 | 0.10 | 0.104 (max) | 0.12 |
|---|---|---|---|---|---|---|---|
| $T$ | 0.21 | 0.40 | 0.65 | 0.75 | 0.82 | 0.82 | 0.80 |

$T(\beta)$ falls quickly below the geometric beta, so a cavity's velocity acceptance is limited and one cavity type covers only a range of β.

### Deviations at low energy

In the example the kinetic energy rises from 1.527 MeV to about 1.87 MeV in the cavity ($\beta$ from 0.057 to 0.063), and the constant-velocity approximation clearly fails. Integrating along the real trajectory, the linear preview gives:

| Setting | Entry phase $\varphi_0$ | $\Delta W$ / MeV | Resulting $V_0T$ / MV |
|---|---|---|---|
| V3 = 0, $\varphi_s = 0^\circ$ | $95.4^\circ$ | 0.475 | 0.475 |
| V3 = 0, $\varphi_s = -33^\circ$ | $52.7^\circ$ | 0.340 | 0.405 |
| maximum of a V3 = 1 scan | $\approx 105.5^\circ$ | 0.482 | — |

So $V_0T$ itself depends on the phase: at different phases the particle gains different energy in the first gap and enters the second gap with a different velocity and at a different time. The maximum energy gain is not at $\varphi_s = 0$ but at an entry phase about $10^\circ$ later. At $\varphi_s = -33^\circ$ the particle is first decelerated in the front half of the first gap (kinetic energy about 1.44 MeV at $z = 0.05\ \mathrm{m}$) and accelerated only afterwards — typical for $\beta \ll \beta_g$.

## Equations of motion and transfer matrix {#rf-cavity-transverse}

### Paraxial fields

Expand the fields near the axis in $r$. From $\nabla\cdot\boldsymbol{E} = 0$ and $\nabla\times\boldsymbol{B} = c^{-2}\,\partial\boldsymbol{E}/\partial t$, an axisymmetric gap has

$$
E_r \approx -\frac{r}{2}\,\frac{\partial E_z}{\partial z},\qquad
B_\theta \approx \frac{r}{2c^2}\,\frac{\partial E_z}{\partial t}
$$

The radial force on the particle is $F_r = q(E_r - v B_\theta)$. At the gap entrance $\partial E_z/\partial z > 0$ and the electric field pushes the particle towards the axis (focusing); at the exit the opposite (defocusing). For a static field and constant velocity the two would cancel exactly; in reality the field changes and the particle accelerates, and the net effect depends on the phase.

### Thin-gap model

Treat a gap as a thin lens at its centre and integrate the radial force over the gap (Wangler, chapter 7):

$$
\Delta(\beta\gamma\,x') = -\frac{\pi\,q E_0TL\,\sin\varphi}{mc^2\,\lambda\,\beta^2\gamma^2}\;x
$$

and the same for $y$. Here $qE_0TL$ is the energy-gain amplitude of the gap (in MeV, with $q$ in units of $e$) and $\beta\gamma$ is taken at the gap centre. In AVAS's cosine convention the bunching phase $\varphi < 0$ gives $\sin\varphi < 0$, so the right-hand side has the sign of $x$: **transverse defocusing**.

The longitudinal action of the same gap: let $z > 0$ mean a particle ahead of the reference particle (arriving earlier, at a smaller phase), $\Delta\varphi = -2\pi z/(\beta\lambda)$, so that

$$
\Delta\delta = \frac{2\pi\,qE_0TL\,\sin\varphi}{mc^2\,\lambda\,\beta^3\gamma}\;z
$$

For $\varphi < 0$ an early particle has its $\delta$ reduced: **longitudinal focusing**. Including the momentum increase from the energy gain, the thin-gap matrix in the $(x, x')$ plane is

$$
M_x = \begin{pmatrix} 1 & 0 \\[2pt] -\dfrac{\pi\,qE_0TL\sin\varphi}{mc^2\lambda\,\bar\beta^2\bar\gamma^2\,(\beta\gamma)_{out}} & \dfrac{(\beta\gamma)_{in}}{(\beta\gamma)_{out}} \end{pmatrix}
$$

The ratio in the lower right corner is **adiabatic damping**: $x'$ shrinks as the momentum grows, the geometric emittance decreases as $1/(\beta\gamma)$ and the normalized emittance stays constant.

### The constraint coupling the planes

For a periodic arrangement averaged per unit length, the longitudinal and RF transverse focusing constants (Wangler's notation) are

$$
k_{l0}^2 = \frac{2\pi\,qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3},\qquad
k_{t,\mathrm{rf}}^2 = -\frac{\pi\,qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3} = -\frac{k_{l0}^2}{2}
$$

Longitudinal stability ($k_{l0}^2 > 0$) therefore implies transverse defocusing, both proportional to $\sin(-\varphi_s)$ and $1/(\beta\gamma)^3$. At low energy, a larger $\lvert\varphi_s\rvert$ means stronger bunching and requires stronger transverse focusing; this is one of the reasons why low-energy sections use lower frequencies and phases around $-30^\circ$ as a compromise; see [Longitudinal motion](#longitudinal) and [Lattice design](#lattice-design).

### Orders of magnitude for the example

For the example take $qE_0TL \approx 0.20\ \mathrm{MeV}$ per gap (half of the 0.405 MeV of both gaps at $\varphi_s = -33^\circ$), $\beta\gamma = 0.0571$, $\lambda = 1.845\ \mathrm{m}$. The thin-gap formula gives $1/f \approx 1.1\ \mathrm{m^{-1}}$ per gap and about $2\ \mathrm{m^{-1}}$ for the whole cavity (a defocusing focal length of about 0.5 m). This is only an estimate; the rms sizes of the linear preview are in [Parameters](#rf-cavity-parameters).

> **AVAS convention**: the kernel does not use the thin-gap matrix above; it integrates the Lorentz force directly in the field map's $\boldsymbol{E}(x,y,z)$ and $\boldsymbol{B}(x,y,z)$ (t-code), so non-linearities, transverse–longitudinal coupling and the non-axisymmetric parts of an HWR are all included. The linear preview cuts the map into 4 mm slices and uses the near-axis field gradients and a linearized RF bunching.

## In AVAS {#rf-cavity-in-avas}

An RF cavity is a field-map element `field` with field type 1:

```text
field  L  R  V3  1  f  phase  Ke  Kb  file
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
```

```avas-params
field
```

Item by item:

- `L` (m): element length; it should equal the length in the field-map header (0.21 m in the example). `R` (m): aperture radius; a particle beyond it counts as lost.
- `V3`: sets the meaning of `phase` (see below).
- `f` (Hz): the RF frequency of this cavity, `162.5e6` in the example. The beam (bunch repetition) frequency is given separately by `frequency` in beam.txt; the two are set independently.
- `phase` (degrees): meaning set by V3.
- `Ke`, `Kb`: scale factors of the electric and magnetic field; `file`: field-map name (without extension). An RF cavity reads `.edx/.edy/.edz` and `.bdx/.bdy/.bdz`; the format is in [Field maps](#field-maps).

> **AVAS convention (verified with the kernel)**: electric fields in a map are in MV/m and multiplied by Ke; magnetic fields are in T and multiplied by Kb. The RF electric field acts as $K_e\,\boldsymbol{E}(\boldsymbol{r})\cos(\omega t+\varphi_0)$ and the RF magnetic field as $K_b\,\boldsymbol{B}(\boldsymbol{r})\sin(\omega t+\varphi_0)$, consistent with the $90^\circ$ between E and B in the [pillbox mode](#rf-cavity-modes).

### V3: phase reference

| V3 | Meaning of `phase` | Use |
|---|---|---|
| 0 | synchronous phase $\varphi_s = \operatorname{atan2}(\int E\sin\varphi\,\dd z,\ \int E\cos\varphi\,\dd z)$; the kernel solves for the entry phase | design: give $\varphi_s$ directly; it is kept when upstream elements change |
| 1 | RF phase $\varphi_0$ when the reference particle reaches the element entrance | phase scans, reproducing a given entry phase |
| 2 | absolute RF phase at $t = 0$; entry phase $\varphi_0 = \varphi + 360^\circ\cdot f\cdot t_{in}$ | modelling a real machine: phases locked to the master clock, so an upstream energy change shifts the arrival time and the actual $\varphi_s$ |

The kernel writes the time and energy of the reference particle at every element entrance, together with $\varphi_s$ and $\varphi_{RF}$, to `synData.txt`; whichever one you set, it computes the other. When a segment run takes a middle section on its own, V3 = 2 phases must be converted with $\varphi_{RF} - 360^\circ\cdot f\cdot(t_{in} - T_{entry})$; AVAS's segment runs do this automatically.

### Ke and Kb

- **Ke is the amplitude knob.** In the linear preview $\Delta W$ is roughly but not strictly proportional to Ke (at low energy the trajectory changes with the amplitude): at $\varphi_s = -33^\circ$, Ke = 0.5, 1.0, 1.36, 2.0 give $\Delta W$ = 0.122, 0.248, 0.340, 0.502 MeV.
- **Kb should scale together with Ke.** The electric and magnetic fields of a cavity are two parts of the same eigenmode; changing Ke without Kb is physically inconsistent. The example uses $K_b = -K_e$: the manual does not explain the minus sign; most likely it is related to the phase/sign convention with which the magnetic field was exported (in the pillbox mode $B_\theta \propto -\sin\omega t$), so that the exported $\boldsymbol{B}$ matches AVAS's sin convention. For a map from another source, set the sign of Kb according to that map's export convention and keep a fixed ratio to Ke.
- For low-β protons the RF magnetic field matters much less than the electric field: the force ratio is about $\beta cB/E$. In the example, switching Kb between 0 and $\pm 1.36$ changes the exit rms size of the linear preview by less than 0.4 %. The magnetic term grows with velocity.

> **Note**: `scanphase` (input.txt) also affects phases: 0 uses the phases in the lattice, 1 lets the kernel scan the cavity phases, 2 reads the entry phase and entry time of every cavity from `scanData.txt` (one line per cavity, in the order of the RF cavities in the lattice). The linear preview ignores `scanphase`.

## Adjustable parameters and typical values {#rf-cavity-parameters}

| Parameter | Effect | Example / typical range |
|---|---|---|
| `phase` (V3 = 0) | synchronous phase: trade-off between acceleration, longitudinal acceptance and transverse defocusing | example $-33^\circ$; low-energy sections often $-40^\circ$ to $-20^\circ$, higher energies closer to the crest |
| `Ke` | electric field amplitude | example 1.36 ($V_0 \approx 0.98\ \mathrm{MV}$); the limit is set by $E_{pk}$, $B_{pk}$ |
| `Kb` | magnetic field amplitude | proportional to Ke, example $-1.36$ |
| `f` | cavity frequency | example 162.5 MHz; 162.5 and 325 MHz are common in low-β superconducting sections |
| `R` | aperture for the loss criterion | example 0.02 m; should match the real beam pipe / drift-tube aperture |
| input.txt `steppercycle` | integration steps per RF period | example 50: $\beta\lambda/50 \approx 2.1\ \mathrm{mm}$, comparable to the 2 mm grid of the map |

The example lattice (one cavity, no space charge, linear preview) at different synchronous phases:

| $\varphi_s$ | $W_{out}$ / MeV | exit rms $x$ / mm | exit rms $z$ / mm |
|---|---|---|---|
| $-60^\circ$ | 1.694 | 1.96 | 0.62 |
| $-33^\circ$ | 1.867 | 1.81 | 0.70 |
| $0^\circ$ | 2.002 | 1.50 | 1.10 |
| $+33^\circ$ | 1.951 | 1.20 | 1.74 |
| drift of the same length | 1.527 | 1.41 | 1.48 |

The entrance rms sizes are $x \approx 0.95\ \mathrm{mm}$ and $z \approx 1.32\ \mathrm{mm}$, and the initial beam is diverging. The trends agree with the [thin-gap model](#rf-cavity-transverse): the more negative $\varphi_s$, the stronger the longitudinal compression and the transverse defocusing; for $\varphi_s > 0$ the beam is focused transversely and debunched longitudinally. At the example current of 0.184 mA, space charge changes the rms sizes at the cavity exit by about 0.3–0.4 % in the linear preview.

## Choosing the phase and phase scans {#rf-cavity-phase}

### Choosing phases in the design

1. Fix $\varphi_s$ of every cavity from the analysis in [Longitudinal motion](#longitudinal): the longitudinal acceptance (phase width about $3\lvert\varphi_s\rvert$) must hold the bunch, and the longitudinal and transverse phase advances must avoid parametric resonances; see [Lattice design](#lattice-design).
2. In the design phase write $\varphi_s$ directly with V3 = 0. When upstream elements change, the kernel solves the entry phase again and every cavity keeps its synchronous phase.
3. Once the design is fixed, read the entry RF phases from `synData.txt` and switch to V3 = 1 or V3 = 2 where needed, to study phase errors or reproduce machine settings.

### Phase scans

In a real machine the cavity phase is found by a phase scan: the cavity phase is stepped, the exit energy is measured by time of flight between downstream beam position monitors, and from the curve $W_{out}(\varphi_0)$ the phase of maximum energy (the "crest") is found; the design $\varphi_s$ is then set as an offset. In AVAS, first set the cavity to V3 = 1, then use the parameter scan on the Run page, or scan the entry phase point by point from the command line:

```text
avas scan --input InputFile --target 2 --param phase --values -180:170:36
```

`--target 2` means line 2 of the lattice file (the cavity's line in the example); an element name works too. `start:stop:count` gives 36 points $10^\circ$ apart; the metric `energy_out` is the exit energy. With the curve:

1. Find the maximum. In the example it is near $\varphi_0 \approx 105.5^\circ$ with $\Delta W_{max} \approx 0.482\ \mathrm{MeV}$; the minimum is near $\varphi_0 \approx -105^\circ$ with $\Delta W \approx -0.31\ \mathrm{MeV}$; the curve is not symmetric.
2. Note that "$-33^\circ$ from the crest" and AVAS's $\varphi_s = -33^\circ$ are not the same at low energy: in the example the entry phase $95.4^\circ$ of $\varphi_s = 0$ is about $10^\circ$ earlier than the entry phase of maximum energy. This follows directly from the [low-energy deviations](#rf-cavity-energy-gain).
3. When the beam β is close to the geometric beta the two coincide: with an injection energy of 4.727 MeV ($\beta = 0.10$) the scan curve is nearly symmetric, the maximum $\Delta W \approx 0.803\ \mathrm{MeV}$ occurs at $\varphi_s \approx 0$, and at $\varphi_s = -33^\circ$ $\Delta W \approx 0.673\ \mathrm{MeV}$, exactly $0.803\cos 33^\circ$.

The lab below is such a phase-scan curve:

```avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
```

## Dynamic effects and common problems {#rf-cavity-pitfalls}

- **β mismatch**: when the beam β is clearly below the geometric beta, $T$ is small and the first gap may decelerate first. The example injects $\beta = 0.057$ into a cavity of geometric beta about 0.10; the cavity gives only about 35 % of $V_0$.
- **Changing Ke but not Kb**: the fields are no longer consistent. The effect is small at low β but can produce spurious transverse forces at high β.
- **Mixing up phase references**: the same number means completely different things for V3 = 0, 1, 2. When importing phases from another code (e.g. TraceWin), check its phase definition and cosine/sine convention first.
- **Phase sign convention**: AVAS uses $\cos(\omega t+\varphi)$ and bunching phases are negative. If a textbook uses a $\sin$ convention, convert the synchronous phase accordingly.
- **Longitudinal non-linearity**: when the bunch phase width is comparable to $\lvert\varphi_s\rvert$, particles fall outside the separatrix and the longitudinal emittance grows or particles are lost longitudinally. The linear preview cannot show this; use the multi-particle kernel. Longitudinal loss limits can be set with `longlimits` in input.txt.
- **Field-map noise**: the example maps are noisy themselves (the on-axis $E_z$ jumps visibly from point to point). This is a property of the data, not a reading error; see [noise in field maps](#field-maps-interpolation).
- **Superposition**: when a cavity overlaps the fringe field of a solenoid use `superpose`; at most one RF cavity per block, see [Superpose](#superpose).
- **First and last element**: in multi-particle runs the first and last elements should not be matrix-model elements; an RF cavity is a field-map element and is not affected.

## Hands-on {#rf-cavity-lab}

1. In the phase-scan lab above, drag `phase` from $-90^\circ$ to $0^\circ$ and watch exit energy, rms $z$ and rms $x/y$ trade off; compare with the table in [Parameters](#rf-cavity-parameters).
2. Lower Ke from 1.36 to 0.7 and watch the synchronous point on the scan curve and the $W(z)$ curve change.
3. Raise the injection energy W to about 4.7 MeV ($\beta \approx 0.10$): the scan curve becomes symmetric and the maximum energy gain approaches $V_0T_{max} \approx 0.80\ \mathrm{MeV}$ (linear preview: 0.803 MeV).
4. In the field-map viewer look at `edz` (two gaps of opposite sign on the axis) and at the $x$–$y$ cross-sections of `edx` and `edy`: the transverse electric field points in opposite directions at the gap entrance and exit, the focusing and defocusing halves.

```avas-field
map hwr010
```

## Further reading {#rf-cavity-reading}

- T. P. Wangler, *RF Linear Accelerators* (2nd ed.), chapters 2–5 (cavities, transit-time factor, shunt impedance) and 6–7 (longitudinal and transverse dynamics, thin-gap formulas).
- H. Padamsee, J. Knobloch, T. Hays, *RF Superconductivity for Accelerators*: $Q_0$, $R/Q$, geometry factor and peak fields of superconducting cavities.
- CAS – CERN Accelerator School lectures on RF cavities and linacs.
- In this primer: [Longitudinal motion](#longitudinal), [Field maps](#field-maps), [Superpose](#superpose), [Errors and correction](#errors), [Tutorial](#tutorial), [References](#references).
