# Steerer

A steerer (dipole corrector, steering magnet) is a weak dipole field that gives the beam centre a small angular deflection in order to correct the orbit. In a real accelerator quadrupoles, solenoids and cavities inevitably have alignment errors, and the beam centre gradually drifts away from the design axis; steerers distributed along the line, together with beam position monitors (BPMs), "pull" the beam back close to the axis.

This chapter builds on the deflection and transfer-matrix concepts of the [quadrupole](#quadrupole) and [dipole](#dipole) chapters; the sources of errors and AVAS's error and correction commands are covered systematically in [errors and correction](#errors).

## Role and placement {#steerer-role}

- **Orbit correction**: compensating the orbit distortion caused by misaligned elements, field errors and initial beam offsets, and so reducing the losses and effective emittance growth that come from an off-centre beam.
- **Beam centring**: making the beam pass through the electrical centre of quadrupoles, solenoids and cavities, avoiding the extra deflection (quadrupoles), coupling (solenoids) or transverse RF kicks (cavities) of an off-centre passage.
- **Local bumps**: 3–4 steerers create a controlled local orbit offset, for example to avoid an obstacle or to scan the aperture.
- **Placement**: usually next to a quadrupole or built into quadrupole/solenoid coils, interleaved with BPMs, at least one per plane per focusing period.

## Physics and field {#steerer-field}

### Magnetic steerer

The field inside a steerer is nearly uniform, with transverse components $B_x$, $B_y$. For a particle moving along $+z$, $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$ gives

$$
F_x = -q v B_y,\qquad F_y = q v B_x ,
$$

so $B_y$ deflects in $x$ and $B_x$ deflects in $y$. In AVAS's cross-section pictures (looking from downstream to upstream, beam coming out of the screen, $x$ to the right, $y$ up), $B_y > 0$ deflects a positive particle to the left ($-x$).

### Electric steerer

A pair of parallel plates produces a transverse electric field $E_x$ (or $E_y$); the force $F_x = q E_x$ does not depend on the velocity and points along the field.

## Equations of motion and transfer matrix {#steerer-matrix}

### Thin kick

For an effective field length $L$ and a small deflection, a steerer is a thin "kick" that changes only the angle, not the position:

$$
\Delta x' = -\frac{q B_y L}{p} = -\frac{B_y L}{B\rho},\qquad
\Delta y' = \frac{q B_x L}{p} = \frac{B_x L}{B\rho}.
$$

For an electric steerer,

$$
\Delta x' = \frac{q E_x L}{p v} = \frac{E_x L}{E\rho},\qquad
\Delta y' = \frac{E_y L}{E\rho},\qquad
E\rho = \frac{p v}{q} = B\rho \cdot v .
$$

The electric rigidity $E\rho$ is measured in V; in MeV, $pv = \beta^2\gamma\,mc^2$, about $2W$ in the non-relativistic limit. Electric deflection scales as $1/(pv)$, magnetic deflection as $1/p$: the lower the velocity, the more effective an electric steerer — which is why low-energy transport lines often use electrostatic deflector plates.

### Matrix form

A steerer is not a linear map but a translation:

$$
\begin{pmatrix} x \\ x' \\ y \\ y' \end{pmatrix}_1 =
\begin{pmatrix} x \\ x' \\ y \\ y' \end{pmatrix}_0 +
\begin{pmatrix} 0 \\ \Delta x' \\ 0 \\ \Delta y' \end{pmatrix}.
$$

It moves the beam centre and leaves the central second moments (rms sizes, emittances, Twiss parameters) unchanged. In the $6\times 6$ linear matrix a steerer is the unit matrix; the deflection needs a $7\times 7$ augmented matrix or a separate tracking of the central orbit.

> AVAS convention: the linear envelope preview treats steerers as unit matrices (`linear_optics.py`: steerers and diagnostics are identities); it computes only the rms envelope, not the central orbit. The effect of a steerer on the orbit must be checked with a kernel run: in DataSet.txt the $x$ centre = column 29 (synchronous particle $x$) + column 1 (centroid offset), the $y$ centre = column 31 + column 3, in m.

### Propagation of a kick downstream

An angle $\theta_j$ applied at position $j$ moves the beam at a downstream position $i$ (without acceleration) by

$$
\Delta x_i = \sqrt{\beta_i\,\beta_j}\,\sin(\psi_i - \psi_j)\,\theta_j ,
$$

where $\beta$ and $\psi$ are the beta function and phase of that plane. With acceleration, multiply by the adiabatic-damping factor $\sqrt{(\beta\gamma)_j/(\beta\gamma)_i}$. Consequences:

- a steerer is "stronger" where $\beta$ is large (the same angle gives a larger offset);
- the offset is largest $90^\circ$ downstream in phase and zero at $180^\circ$ — the phase advance between two neighbouring steerers decides whether position and angle can be corrected at the same time.

## In AVAS {#steerer-avas}

```text
steerer 0 R 0 Bx By kind max
```

```avas-params
steerer
```

- The first parameter (length) **must be 0**; the third parameter is reserved, write 0.
- `R`: aperture radius (m).
- `Bx`, `By`: magnetic field (T) for `kind = 0`, electric field `Ex`, `Ey` (V/m) for `kind = 1`.
- `kind`: 0 = magnetic steerer, 1 = electric steerer.
- `max`: the maximum. The manual only says "maximum", usually read as the limit allowed during correction; neither its meaning nor its unit is explained further in the manual — the kernel is the reference.

> AVAS convention: a steerer is written with length 0 but **acts over the next element**: its effective length is by default the length of the next element, and it sits at the middle of the next element (manual). The integrated field is therefore $B\,L_{\text{next}}$, and the deflection follows from the formulas above with the length of the next element. The lattice check warns about a `steerer` with non-zero length.

Example: a steerer with a vertical field $B_y = 1$ mT on top of a quadrupole:

```text
steerer 0 0.02 0 0 0.001 0 0.002
quad 0.1 0.02 0 6
```

For the example beam ($B\rho = 0.17863$ T·m), $B_y L = 1\times 10^{-4}$ T·m, and the deflection is $\lvert\Delta x'\rvert = 0.56$ mrad. The sign of the deflection follows from $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$ in a right-handed frame; the sign convention actually used by the kernel is not written in the manual — confirm it with a single-particle run when it matters.

### Correction commands

AVAS's static error correction names the parameter to adjust with `adjust` and gives the target with diagnostic commands:

```avas-params
adjust diag_position
```

`adjust 0 v n min max first_step` is written before the element to adjust; `v` is the number of the element's parameter. The manual's only example is `adjust 1 7 5 0 3 0` before a `field`, varying its 7th parameter `Ke`, i.e. counting from the first number after the keyword; counting the same way, the 4th parameter of `steerer` is `Bx` and the 5th is `By`. The manual gives no steerer example, so this correspondence is defined by the kernel — before relying on it, confirm with a short error study that the steerer field is actually varied. `diag_position 0 x y 0` gives the target centre position (mm). A sketch:

```text
adjust 0 5 1 -0.002 0.002 0
steerer 0 0.02 0 0 0 0 0.002
quad 0.1 0.02 0 6
drift 0.3 0.02 0
diag_position 0 0 0 0
```

Correction takes place within an error study (it needs `err_step` and the other error commands); algorithm, parameter meanings and a complete case are in [errors and correction](#errors).

## Parameters and typical values {#steerer-params}

| Quantity | Typical range | Remark |
|---|---|---|
| integrated field $BL$ | $10^{-5}$–$10^{-3}$ T·m | small room-temperature corrector coils |
| electric field $E$ | $10^4$–$10^6$ V/m | low-energy electrostatic deflectors |
| deflection angle | 0.1 to a few mrad | set by the orbit error budget |
| number | one per plane every 1–2 focusing periods | paired with BPMs |

### Worked example

Example beam: protons at 1.527 MeV, $B\rho = 0.17863$ T·m, $E\rho = 3.05$ MV ($pv = 3.05$ MeV).

| Steerer | Field | Effective length | Deflection |
|---|---|---|---|
| magnetic | $B_y = 1$ mT | 0.1 m | 0.56 mrad |
| magnetic | $B_y = 10$ mT | 0.1 m | 5.6 mrad |
| electric | $E_x = 10^5$ V/m | 0.1 m | 3.28 mrad |

A 1 mrad deflection needs $BL = 1.79\times 10^{-4}$ T·m or $EL = 3.05\times 10^{3}$ V. At such a low velocity ($\beta = 0.057$), $10^5$ V/m is equivalent to a magnetic field of 5.9 mT ($E = vB$, $v = 1.71\times 10^7$ m/s).

## Dynamics and common problems {#steerer-dynamics}

### The basic idea of orbit correction

Write the response of all BPMs to all steerers as a response matrix $\mathsf{R}$, $R_{ij} = \partial x_i/\partial\theta_j$ (in a linac non-zero only for $j$ upstream of $i$; the values come from the formula above or from simulations that change one steerer at a time). From a measured orbit $\boldsymbol{x}$ the correction is

$$
\boldsymbol{\theta} = -\,\mathsf{R}^{+}\,\boldsymbol{x},
$$

where $\mathsf{R}^{+}$ is the pseudo-inverse (the least-squares solution, usually by singular value decomposition, truncating small singular values when needed to avoid excessive corrections). Common simplifications are "one-to-one" correction (each steerer zeroes the nearest downstream BPM) and local bumps. AVAS's `adjust` iterates the parameters by gradient descent until the diagnostic targets are met (see [errors and correction](#errors)).

### Steerers have side effects

- **Dispersion**: a steerer is in essence a weak dipole; after a deflection there is a small dispersion $D \sim \theta\cdot(\text{distance})$; for beams with a large energy spread, large corrections increase the effective size.
- **Off-centre passage through elements**: a centre of zero at the BPM does not mean zero at the quadrupole or cavity centre; offsets between BPM and magnetic centre (BPM offsets) also have to be considered.
- **Field quality**: the field of a small steerer is non-uniform near the edge of the aperture, so large-amplitude particles are deflected differently; the matrix model does not include this.
- **Saturation and limits**: corrections close to `max` mean the errors exceed the correction capability; check the error settings or add steerers.

### Common problems

- Writing a non-zero length for `steerer`: the manual requires 0, and the lattice check warns.
- Putting a `steerer` last in the lattice: there is no "next element" for it to act over; and as a matrix-model element it should not be at either end anyway.
- `kind` not matching the units: with `kind = 1` the `Bx`, `By` positions hold V/m, values usually several orders of magnitude larger than magnetic fields in T.
- No visible effect in the linear preview: the linear preview does not track the central orbit; use a kernel run.

## Hands-on {#steerer-lab}

The field on the steerer cross-section (**schematic**, computed from the parameters): a uniform transverse field in the bore. By AVAS's convention the cross-section is seen from downstream to upstream with the beam coming out of the screen (⊙), so for $B_y > 0$ you can read off directly that a positive particle is pushed to the left.

```avas-field
steerer Bx=0 By=0.01 R=0.02
```

With $B_x > 0$ instead: the field points right and the force points up ($+y$).

```avas-field
steerer Bx=0.01 By=0 R=0.02
```

To see the effect on the orbit, add a `steerer` (followed by a drift or a quadrupole) to a copy of the example project, run the kernel with a single particle or a few particles, watch the beam centre (column 29 + column 1) along $z$ in DataSet.txt, and check the downstream offset and its direction against $\Delta x' = B_y L/B\rho$.

## Further reading {#steerer-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed. (errors, alignment and orbit correction).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (orbit distortion and correction, response matrices).
- CERN Accelerator School lectures on beam diagnostics and orbit correction (CAS proceedings).
- Related chapters: [errors and correction](#errors), [quadrupole](#quadrupole), [dipole](#dipole), [simulation](#simulation); the full bibliography is in [references](#references).
