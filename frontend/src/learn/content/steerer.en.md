# Steerer

In a real accelerator every quadrupole, solenoid and cavity is installed with errors of a few tenths of a millimetre, and the beam centre drifts away from the design axis bit by bit. A steerer (also called a dipole corrector or steering magnet) is nothing but a weak dipole field that gives the beam centre a small deflection of a few tenths of a milliradian. It does not change the size of the beam, yet it decides whether the beam scrapes the pipe. So: how strong a field brings back a beam that is 1 mm off? Where is the same deflection most effective? And why are steerers always used in pairs?

This chapter first works out how large a deflection is, then follows it downstream as it propagates with the betatron oscillation, and finally shows how the steerers along the line work with beam position monitors (BPMs) to "pull" the beam back close to the axis. It uses the deflections and transfer matrices of the [quadrupole](#quadrupole) and [dipole](#dipole) chapters and the beta function and phase advance of [linear optics](#linear-optics); the sources of errors and AVAS's error and correction commands are treated systematically in [errors and correction](#errors).

A steerer has zero length and acts over the element after it (here the 0.2 m drift): $B_y = 1$ mT over 0.2 m is an integrated field of $2\times 10^{-4}$ T·m, which deflects the example beam by 1.12 mrad; at the end of the line (0.5 m after the centre of the kick) the beam centre is about 0.56 mm off, about a fifth of the rms size there (2.8 mm). The envelope plot shows only the rms size, which is exactly the same as without the steerer, so this offset cannot be seen in it.

```avas-beamline
drift 0.1 0.02 0
steerer 0 0.02 0 0 0.001 0 0.002
drift 0.2 0.02 0
drift 0.4 0.02 0
```

## Role and placement {#steerer-role}

- **Orbit correction**: compensating the orbit distortion caused by misaligned elements, field errors and initial beam offsets, reducing the losses and effective emittance growth that an off-centre beam brings.
- **Beam centring**: steering the beam through the electrical centres of quadrupoles, solenoids and cavities to avoid the extra deflection (quadrupoles), coupling (solenoids) or transverse RF kicks (cavities) of an off-centre passage.
- **Local bumps**: using 3–4 steerers to make a controlled local orbit excursion, for example to go around an obstacle or to scan the aperture.
- **Placement**: usually right next to a quadrupole or built into the coils of quadrupoles or solenoids (superconducting solenoids often carry dipole correction coils), alternating with BPMs, at least one per plane per focusing period.

## A small-angle deflection {#steerer-field}

### Magnetic steerer

The field inside a steerer is nearly uniform, with transverse components $B_x$ and $B_y$. For a particle moving along $+z$, $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$ gives

$$
F_x = -q v B_y,\qquad F_y = q v B_x .
$$

$B_y$ deflects the particle in $x$, $B_x$ deflects it in $y$: field and deflection are perpendicular. In AVAS's cross-section pictures (viewed from downstream, the beam coming out of the screen, $x$ to the right, $y$ up), $B_y > 0$ deflects a positive particle to the left ($-x$). The two pictures below are cross-section **schematics** computed from the parameters: a uniform transverse field across the bore. First $B_y > 0$: the field points up, and the right-hand rule gives a force to the left on a positive charge.

```avas-field
steerer Bx=0 By=0.01 R=0.02
```

Now $B_x > 0$: the field points to the right and the force points up ($+y$).

```avas-field
steerer Bx=0.01 By=0 R=0.02
```

### The deflection angle

It is the same kind of field as a [dipole](#dipole), only much weaker: the arc the particle travels in the field is far shorter than the bending radius, and the trajectory turns only by a small angle. With an effective length $L$ of the field, and the particle's position hardly changing inside, the steerer is a thin "kick" that changes the angle but not the position:

$$
\Delta x' = -\frac{q B_y L}{p} = -\frac{B_y L}{B\rho},\qquad
\Delta y' = \frac{q B_x L}{p} = \frac{B_x L}{B\rho}.
$$

The deflection is the integrated field $BL$ divided by the rigidity — the dipole's $L/\rho$ with a very large $\rho = B\rho/B$. The example beam ($B\rho = 0.17863$ T·m) is deflected by 0.56 mrad in a steerer with $B_y = 1$ mT and $L = 0.1$ m; a 1 mrad deflection needs $BL = 1.79\times 10^{-4}$ T·m. The deflection scales as $1/B\rho$: the same $10^{-4}$ T·m deflects 35 keV protons by 3.7 mrad and 100 MeV protons by only 0.067 mrad.

### Electric steerer

A pair of parallel plates produces a transverse electric field $E_x$ (or $E_y$); the force $F_x = q E_x$ is independent of velocity and points along the field. The particle spends a time $L/v$ in the field, its transverse momentum changes by $qE_x L/v$, so

$$
\Delta x' = \frac{q E_x L}{p v} = \frac{E_x L}{E\rho},\qquad
\Delta y' = \frac{E_y L}{E\rho},\qquad
E\rho = \frac{p v}{q} = B\rho \cdot v .
$$

The electric rigidity $E\rho$ is in V; in MeV, $pv = \beta^2\gamma\,mc^2$, about $2W$ non-relativistically. The example beam has $pv = 3.05$ MeV, $E\rho = 3.05$ MV: $E_x = 10^5$ V/m over 0.1 m deflects it by 3.28 mrad, and a 1 mrad deflection needs $EL = 3.05\times 10^3$ V.

The electric deflection goes as $1/(pv)$, the magnetic one as $1/p$: their ratio is just the velocity $v$. For the example beam ($\beta = 0.057$, $v = 1.71\times 10^7$ m/s), an electric field of $10^5$ V/m is equivalent to a magnetic field of 5.9 mT ($E = vB$); the same integrated voltage of $10^4$ V deflects 35 keV protons by 143 mrad and 100 MeV protons by only 0.053 mrad. The lower the velocity, the more effective an electric steerer is, which is why low-energy beam lines often use electrostatic deflector plates.

## A deflection only moves the centroid {#steerer-matrix}

### A translation

A uniform field changes the angle of every particle in the beam by the same amount, whatever its position. A steerer is therefore not a linear map but a translation:

$$
\begin{pmatrix} x \\ x' \\ y \\ y' \end{pmatrix}_1 =
\begin{pmatrix} x \\ x' \\ y \\ y' \end{pmatrix}_0 +
\begin{pmatrix} 0 \\ \Delta x' \\ 0 \\ \Delta y' \end{pmatrix}.
$$

It moves the beam centre and leaves the second central moments (rms sizes, emittances, Twiss parameters) unchanged — the ellipse in phase space is shifted along $x'$ as a whole, its shape unchanged. In the $6\times 6$ linear matrix a steerer is the unit matrix; the deflection needs a $7\times 7$ augmented matrix or a separate tracking of the centre orbit.

If it does not change the rms size, why does it matter? Because losses depend on how far the edge of the beam is from the wall: for a beam 1 mm off centre with an rms size of 1.5 mm, the 3-rms edge is at 5.5 mm instead of 4.5 mm. An off-centre passage through a quadrupole adds a further deflection, through a cavity a transverse RF kick, and both make the orbit deviation grow further.

> **AVAS convention**: the linear envelope preview treats steerers as unit matrices; it computes only the rms envelope, not the centre orbit. Look at the effect of steerers on the orbit with an engine run: in DataSet.txt the $x$ centre = column 29 (synchronous particle $x$) + column 1 (centroid offset), the $y$ centre = column 31 + column 3, in m.

## How a deflection propagates downstream {#steerer-orbit}

### In a drift

In a drift the deflected centre leaves the axis on a straight line: $\Delta x = \theta\,s$. The opening beam line is just that: 1.12 mrad times 0.5 m gives 0.56 mm.

### In a focusing channel

With focusing, the deflected centre no longer runs straight; like a single particle it performs a betatron oscillation about the axis. An angle $\theta_j$ applied at position $j$ gives, at a downstream position $i$ (without acceleration), the offset

$$
\Delta x_i = \sqrt{\beta_i\,\beta_j}\,\sin(\psi_i - \psi_j)\,\theta_j ,
$$

where $\beta$ and $\psi$ are the beta function and phase of that plane (see [linear optics](#linear-optics)). It is the transfer-matrix element $R_{12}$ written with Twiss parameters. With acceleration it is multiplied by the adiabatic damping factor $\sqrt{(\beta\gamma)_j/(\beta\gamma)_i}$. The formula says:

- A steerer is more "powerful" where $\beta$ is large: the larger $\sqrt{\beta_j}$, the larger the downstream offset for the same angle. In a FODO the steerer for a plane usually sits next to the focusing quadrupole of that plane, where $\beta$ is largest.
- The offset varies as the sine of the phase: largest $90^\circ$ downstream, zero at $180^\circ$. One steerer can correct a position $90^\circ$ downstream or an angle $0^\circ$ or $180^\circ$ downstream; to correct position and angle together you need two steerers about $90^\circ$ apart in phase.

A number: in the $G = 6$ T/m FODO of the [quadrupole](#quadrupole) chapter ($L_q = 0.1$ m, $L_d = 0.2$ m, 52.7° per period, $\beta_x$ 1.06 m at QF and 0.43 m at QD), a kick of 0.56 mrad at a QF starts an orbit oscillation whose amplitude is $\sqrt{1.06 \times 1.06}\times 0.56 = 0.59$ mm at each QF and $\sqrt{0.43\times 1.06}\times 0.56 = 0.38$ mm at each QD; at the next QF (52.7° later) the actual offset is 0.47 mm.

### Where orbit errors come from

Seen the other way round, a misaligned quadrupole is an unwanted steerer: a quadrupole offset by $d$ kicks the beam centre by $\Delta x' = kL\,d$. A $G = 6$ T/m, $L = 0.1$ m quadrupole offset by 0.1 mm kicks the example beam by 0.34 mrad, the same as a steerer of integrated field $6\times 10^{-5}$ T·m. Many such random kicks along the line add up to the orbit distortion that has to be corrected (see [errors and correction](#errors)).

### Local bumps

Three steerers can change the orbit locally only: the first kicks the beam out, the second bends it back, the third removes the remaining angle, so that the orbit outside the bump is not affected. The three kick angles follow from the propagation formula above, by making position and angle after the third steerer return to zero; without acceleration this gives

$$
\theta_2 = -\theta_1\sqrt{\frac{\beta_1}{\beta_2}}\,\frac{\sin\psi_{13}}{\sin\psi_{23}},\qquad
\theta_3 = \theta_1\sqrt{\frac{\beta_1}{\beta_3}}\,\frac{\sin\psi_{12}}{\sin\psi_{23}},
$$

with $\psi_{ij}$ the phase advance from steerer $i$ to steerer $j$. The solution fails only when $\sin\psi_{23} = 0$ and the kicks grow without bound as $\psi_{23}$ approaches a multiple of $180^\circ$; in practice one keeps the whole bump within less than about $180^\circ$ of phase so that the kicks stay moderate and the bump stays short. With a fourth steerer, position and angle in the middle of the bump can be set independently.

> **Think about it**: with BPMs and steerers all along the line and every BPM reading set to zero, does the beam necessarily pass through the magnetic centre of every quadrupole?
> Answer: Not necessarily. A BPM reads the beam position relative to its own electrical centre, and the BPM itself is offset from the quadrupole's magnetic centre (BPM offset); moreover, between two BPMs the orbit can be an oscillation that happens to be zero at the BPMs. A centre at zero in the BPMs after correction does not mean zero in the quadrupoles or cavities.

## Orbit correction and side effects {#steerer-dynamics}

### The response matrix

Write the response of all BPMs to all steerers as a response matrix $\mathsf{R}$, $R_{ij} = \partial x_i/\partial\theta_j$ (in a linac non-zero only when $j$ is upstream of $i$; the values come from the formula above or from simulations changing one steerer at a time). With a measured orbit $\boldsymbol{x}$, the correction is

$$
\boldsymbol{\theta} = -\,\mathsf{R}^{+}\,\boldsymbol{x},
$$

where $\mathsf{R}^{+}$ is the pseudo-inverse (the least-squares solution, usually by singular value decomposition, truncating small singular values where needed to avoid excessive corrections). Common simplifications are "one-to-one" correction (each steerer aims at the nearest downstream BPM) and local bumps. AVAS's `adjust` varies parameters iteratively by gradient descent until the diagnostic targets are met (see [errors and correction](#errors)).

### Steerers have side effects too

- **Dispersion**: a steerer is in essence a weak dipole and creates a little dispersion, $D \sim \theta\cdot(\text{distance})$; for a beam with a large energy spread, corrections with large deflections increase the effective size.
- **Off-centre passage through elements**: a centre at zero in the BPMs after correction is not a centre at zero in the quadrupoles or cavities; the offset between BPMs and the magnetic centres (BPM offset) has to be considered as well.
- **Field quality**: the field of small steerers is not uniform near the edge of the aperture, so large-amplitude particles are deflected differently; the matrix model does not include this.
- **Saturation and limits**: if `max` is the corrector limit (see the keyword below), corrections close to `max` mean that the errors exceed the correction capability; check the error settings or add steerers.

## Common misconceptions {#steerer-misconceptions}

- **"A steerer changes the beam size"**: an ideal uniform field only moves the centroid; the rms size and emittance stay the same, which is exactly why the linear preview shows no effect.
- **"The larger the deflection the better, and placement does not matter"**: the same angle does more where $\beta$ is large, and the offset varies as the sine of the phase, so a steerer $180^\circ$ in phase before the target does almost nothing there.
- **"BPM readings of zero mean the beam is centred"**: see the "think about it" above.
- **"Electric and magnetic steerers are equally effective at all energies"**: their ratio is the velocity; at low energy electric steerers are far more efficient.
- **"A steerer's length is its active length"**: in AVAS a steerer is written with length 0 and acts over the length of the next element (see below).

## In AVAS {#steerer-in-avas}

### Keyword

```text
steerer 0 R 0 Bx By kind max
```

```avas-params
steerer
```

- The first parameter (length) **must be 0**; the third is reserved, write 0.
- `R`: aperture radius (m).
- `Bx`, `By`: magnetic field (T) for `kind = 0`, electric field `Ex`, `Ey` (V/m) for `kind = 1`.
- `kind`: 0 = magnetic steerer, 1 = electric steerer.
- `max`: maximum value. The manual only says "maximum", usually understood as the upper limit allowed during correction; its exact meaning and unit are not explained further in the manual; the engine is the reference.

> **AVAS convention**: a steerer is written with length 0 but **acts over the next element**: its active length defaults to the length of the next element and it sits in the middle of that element (the manual's words). The integrated field is therefore $B\,L_{\text{next}}$, and the deflection follows from the formula above with the length of the next element. The lattice check warns about a `steerer` with non-zero length.

Example: a steerer with a vertical field $B_y = 1$ mT on a quadrupole:

```text
steerer 0 0.02 0 0 0.001 0 0.002
quad 0.1 0.02 0 6
```

For the example beam ($B\rho = 0.17863$ T·m), $B_y L = 1\times 10^{-4}$ T·m, a deflection of $\lvert\Delta x'\rvert = 0.56$ mrad. The sign follows from $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$ in a right-handed frame; the sign convention actually used by the engine is not written in the manual; check with a single-particle run when it matters.

### Correction commands

AVAS's static error correction names the parameter to vary with `adjust` and the target with a diagnostic command:

```avas-params
adjust diag_position
```

`adjust 0 v n min max first_step` is written before the element to be varied; `v` is the index of that element's parameter. The manual's only example is `adjust 1 7 5 0 3 0` before a `field`, varying its 7th parameter `Ke`, i.e. counting from the first number after the keyword; counting the same way, the 4th parameter of `steerer` is `Bx` and the 5th is `By`. The manual gives no steerer example, so this correspondence is up to the engine; before relying on it, confirm with a short error study that the steerer field really is changed. `diag_position 0 x y 0` gives the target centre position (mm). A schematic fragment:

```text
adjust 0 5 1 -0.002 0.002 0
steerer 0 0.02 0 0 0 0 0.002
quad 0.1 0.02 0 6
drift 0.3 0.02 0
diag_position 0 0 0 0
```

Correction takes place within an error study (it needs `err_step` and the other error commands); the algorithm, the meaning of the parameters and a complete case are in [errors and correction](#errors).

### Seeing the orbit in the engine

The linear preview does not track the centre orbit. To see what a steerer does, add a `steerer` (followed by a drift or quadrupole) to a copy of the example project, run the engine with a single particle or a few particles, follow the beam centre (column 29 + column 1) along $z$ in DataSet.txt, and check the downstream offset and its direction against $\Delta x' = B_y L/B\rho$.

### Typical values

| Quantity | Typical range | Note |
|---|---|---|
| Integrated field $BL$ | $10^{-5}$–$10^{-3}$ T·m | small normal-conducting correction coils |
| Electric field $E$ | $10^4$–$10^6$ V/m | low-energy electrostatic deflector plates |
| Deflection | 0.1 to a few mrad | set by the orbit error budget |
| Number | one per plane every 1–2 focusing periods | paired with BPMs |

### Numerical examples

Example beam: 1.527 MeV protons, $B\rho = 0.17863$ T·m, $E\rho = 3.05$ MV ($pv = 3.05$ MeV).

| Steerer | Field | Active length | Deflection |
|---|---|---|---|
| magnetic | $B_y = 1$ mT | 0.1 m | 0.56 mrad |
| magnetic | $B_y = 10$ mT | 0.1 m | 5.6 mrad |
| electric | $E_x = 10^5$ V/m | 0.1 m | 3.28 mrad |

A 1 mrad deflection needs $BL = 1.79\times 10^{-4}$ T·m or $EL = 3.05\times 10^{3}$ V. At such a low velocity ($\beta = 0.057$), $10^5$ V/m is equivalent to 5.9 mT ($E = vB$, $v = 1.71\times 10^7$ m/s).

### Common problems

- A `steerer` with non-zero length: the manual requires length 0 and the lattice check warns.
- A `steerer` as the last element of the lattice: there is no "next element" for it to act on; and as a matrix-model element it should not be first or last anyway.
- `kind` not matching the units: with `kind = 1` the `Bx`, `By` positions hold V/m, usually several orders of magnitude larger than a magnetic field in T.
- No visible effect in the linear preview: the preview does not track the centre orbit; use an engine run.

## Further reading {#steerer-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed. (transverse beam dynamics in linacs).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (orbit distortion and correction, response matrices).
- CERN Accelerator School lectures on beam diagnostics and orbit correction (CAS proceedings).
- Related chapters: [errors and correction](#errors), [quadrupole](#quadrupole), [dipole](#dipole), [simulation](#simulation); the full bibliography is in [references](#references).
