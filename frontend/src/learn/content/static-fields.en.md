# Static electric and magnetic field maps

Besides RF cavities, the `field` element can describe fields that do not change in time: field type 2 is a **static electric field** (einzel lenses, electrostatic quadrupoles, electrostatic deflectors …), field type 3 a **static magnetic field** (solenoids, quadrupoles and magnets with a bending component described by a map …). Compared with the matrix-model elements `quad`, `solenoid` and `bend`, a field-map element keeps the real longitudinal profile, the fringe fields and the higher-order components, and the particle motion is integrated directly in the field with the t-code. AVAS has no matrix-model element for electrostatic lenses; electrostatic focusing elements can only be described by type-2 field maps (or a pure deflection by the electric kind 1 of `steerer`).

The numerical examples of this chapter use the proton beam of the example project ($W = 1.527\ \mathrm{MeV}$, $\beta = 0.0570$, $\beta\gamma = 0.0571$, $p = 53.55\ \mathrm{MeV}/c$, $B\rho = 0.1786\ \mathrm{T\,m}$) and the solenoid field map `sol` (0.35 m long).

## Role and position {#static-fields-role}

- **Low-energy beam transport**: sections with energies of only tens to hundreds of keV often use electrostatic einzel lenses and quadrupoles: the electric force $qE$ does not depend on the velocity while the magnetic force $qvB$ falls with it, so at low velocity an electric field of comparable technical difficulty is more effective (see the next section), and there is no hysteresis.
- **Transverse focusing in superconducting linacs**: the superconducting solenoids between the cavities of a low-β section have high fields and long fringes that often overlap the fields of neighbouring cavities; a field map is more reliable than a hard-edge model, see [Solenoid](#solenoid) and [Superpose](#superpose).
- **Combined-function elements**: a magnet with quadrupole plus bending or steering coils, or with significant higher-order fields, is described by one map.
- **Checking matrix models**: comparing a field-map calculation with the hard-edge matrices of [Quadrupole](#quadrupole) and [Solenoid](#solenoid) shows how large the error of the hard-edge approximation is.

## Static electric fields: energy change and electric rigidity {#static-fields-electric}

### Energy follows the potential

A static electric field is conservative, $\boldsymbol{E} = -\nabla\Phi$. The total energy of a particle in an electrostatic element is conserved:

$$
W(\boldsymbol{r}) + q\,\Phi(\boldsymbol{r}) = \text{const}
$$

so the particle is accelerated or decelerated between the electrodes, and its kinetic energy is restored when it leaves the element at the original potential. Unlike in a magnetic field, the velocity, momentum and focusing strength change with position inside an electrostatic element — which is why the field map has to be integrated instead of applying a constant-energy matrix. AVAS's linear preview also includes the on-axis $E_z$ of type-2 maps in the energy of the reference particle.

### Electric rigidity

The bending power of a magnetic field is measured by the magnetic rigidity $B\rho = p/q$; for electric fields the corresponding quantity is the **electric rigidity**

$$
E\rho = \frac{p\,v}{q} = \frac{\beta^2\gamma\,mc^2}{q}\ \xrightarrow{\ \beta\ll 1\ }\ \frac{2W}{q}
$$

For the example proton $E\rho = 53.55\ \mathrm{MeV}/c\times 0.0570 \approx 3.05\ \mathrm{MV}$ (non-relativistic approximation $2W/q = 3.054\ \mathrm{MV}$). A transverse electric field $E$ bends with radius $\rho = E\rho/E$; for example $E = 1\ \mathrm{MV/m}$ gives $\rho \approx 3.05\ \mathrm{m}$.

Electric and magnetic fields are equivalent through $E \leftrightarrow vB$: at $\beta = 0.057$ a 1 T magnetic field is worth $\beta c\times 1\ \mathrm{T} \approx 17\ \mathrm{MV/m}$ of electric field. The lower the velocity, the more "valuable" a given electric field, which is why electrostatic elements are used mainly at low energy.

### Electrostatic quadrupole

An ideal electrostatic quadrupole has four hyperbolic electrodes at $\pm V$ and aperture radius $a$; the potential is

$$
\Phi = \frac{V}{a^2}\,(x^2 - y^2),\qquad E_x = -\frac{2V}{a^2}\,x,\qquad E_y = +\frac{2V}{a^2}\,y
$$

With the electric gradient $G_E = 2V/a^2$ (V/m²) the linear equations of motion are

$$
x'' + k\,x = 0,\quad y'' - k\,y = 0,\qquad k = \frac{G_E}{E\rho}
$$

For $V > 0$ a positive particle is focused in $x$ and defocused in $y$; the transfer matrices have exactly the form of a magnetic quadrupole (see [Quadrupole](#quadrupole)). For example $V = \pm 10\ \mathrm{kV}$, $a = 15\ \mathrm{mm}$: $G_E = 8.9\times 10^7\ \mathrm{V/m^2}$, which for the example proton gives $k \approx 29\ \mathrm{m^{-2}}$, the same as a magnetic quadrupole of gradient $kB\rho \approx 5.2\ \mathrm{T/m}$.

### Einzel lens

An einzel lens consists of three coaxial cylindrical electrodes, the outer two grounded and a voltage on the middle one, so the kinetic energy is the same at entrance and exit. Non-relativistic, axisymmetric and paraxial, let $\hat\Phi(z)$ be the on-axis potential referred to "zero kinetic energy" ($q\hat\Phi = W$); the trajectory obeys the paraxial ray equation

$$
r'' + \frac{\hat\Phi'}{2\hat\Phi}\,r' + \frac{\hat\Phi''}{4\hat\Phi}\,r = 0
$$

The Picht transformation $R = r\,\hat\Phi^{1/4}$ gives

$$
R'' + \frac{3}{16}\left(\frac{\hat\Phi'}{\hat\Phi}\right)^{2}R = 0
$$

The coefficient is always positive, so **an einzel lens always focuses, whether the middle electrode is at positive or negative voltage**. In the weak-lens approximation the focal length is

$$
\frac{1}{f} \approx \frac{3}{16}\int\left(\frac{\hat\Phi'}{\hat\Phi}\right)^{2}\dd z
$$

The decelerating mode (middle electrode potential slows the particles) focuses more strongly at the same voltage, but if $q\Phi$ approaches the kinetic energy the particles stop or are reflected in the lens. Relativistic corrections are small at the energies of this primer; the full form is in Reiser, chapter 3.

### Electrostatic deflection

A radial electric field between cylindrical or spherical electrodes bends the beam with radius $\rho = E\rho/E$. In the bending plane an electrostatic deflector focuses differently from a magnetic dipole (depending on the electrode shape), and the kinetic energy changes with the potential during the bend. AVAS's dipole `bend` describes magnetic bending only; electrostatic deflection can only be described by a field map, see [Problems](#static-fields-pitfalls).

## Static magnetic fields: solenoids, quadrupoles and bending components {#static-fields-magnetic}

### Solenoid maps

Near the axis an axisymmetric magnetic field is fixed by the on-axis $B_z(z)$ ($\nabla\cdot\boldsymbol{B} = 0$, $\nabla\times\boldsymbol{B} = 0$):

$$
B_r(r,z) = -\frac{r}{2}\,B_z'(z) + \frac{r^3}{16}\,B_z'''(z) - \cdots,\qquad
B_z(r,z) = B_z(z) - \frac{r^2}{4}\,B_z''(z) + \cdots
$$

At the entrance $B_z' > 0$ and $B_r$ points towards the axis; at the exit $B_z' < 0$ and $B_r$ points away from it (for $B_z > 0$). Crossing the entrance fringe the particle acquires an azimuthal velocity through $F_\theta = q v_z B_r$; in the body of the solenoid $F_r = q v_\theta B_z$ pulls it towards the axis — solenoid focusing. By Busch's theorem the two transverse planes decouple in the rotating (Larmor) frame:

$$
r'' + \left(\frac{B_z(z)}{2B\rho}\right)^{2} r = 0,\qquad
\theta_L = \int\frac{B_z}{2B\rho}\,\dd z,\qquad
\frac{1}{f} \approx \int\left(\frac{B_z}{2B\rho}\right)^{2}\dd z
$$

The focusing strength is proportional to $B_z^2$ and **independent of the field direction**; changing the sign of $K_b$ only reverses the beam rotation. Checked on the example: with drifts before and after the solenoid map, the linear preview gives identical rms sizes for $K_b = \pm 0.531$.

### The example solenoid sol

The map `sol` is 0.35 m long; the on-axis $B_z$ peaks at the centre $z \approx 0.175\ \mathrm{m}$ and falls to about 0.3 % of the peak at both ends. With $K_b = 0.531$ from the superpose example of the manual:

| Quantity | Value | Remark |
|---|---|---|
| $\int B_z\,\dd z$ | $0.387\ \mathrm{T\,m}$ | insensitive to noise |
| Larmor rotation $\theta_L$ | $62^\circ$ | $\int B_z\,\dd z/(2B\rho)$ |
| $\int B_z^2\,\dd z$ | $1.10$–$1.14\ \mathrm{T^2m}$ | noise raises the raw value by about 3 %; the first is after 9-point smoothing |
| thin-lens focal length $f$ | $\approx 0.115\ \mathrm{m}$ | comparable to the length; the thin-lens value is only an order of magnitude |
| hard-edge equivalent $B_{eff} = \int B^2/\int B$ | $\approx 2.85\ \mathrm{T}$ | |
| equivalent length $L_{eff} = (\int B)^2/\int B^2$ | $\approx 0.136\ \mathrm{m}$ | much shorter than the map |
| smoothed peak field | $\approx 4.4\ \mathrm{T}$ | the raw data jump from point to point; their maximum is not the peak |

The radial field of the map at $r = 1\ \mathrm{cm}$ (raw map values): on the entrance side at $z = 0.059\ \mathrm{m}$, $B_x(x{=}1\,\mathrm{cm}) \approx -0.053\ \mathrm{T}$, pointing to the axis; on the exit side at $z = 0.29\ \mathrm{m}$ about $+0.052\ \mathrm{T}$, pointing away; the magnitude agrees with $-\tfrac{r}{2}B_z'$.

### Quadrupole maps

Near the axis a magnetic quadrupole has $B_y = G\,x$, $B_x = G\,y$ and $k = G/(B\rho)$; for $k > 0$ a positive particle is focused in $x$. Quadrupole maps are usually exported normalized to a reference gradient (e.g. 1 T/m at the centre); then $K_b$ is the actual gradient in T/m, and **the sign of $K_b$ sets the polarity**: for $K_b < 0$ the focusing and defocusing planes swap. With fringe fields the effective length $L_{eff} = \int G(z)\,\dd z/G_0$ is used for comparison with a hard-edge quadrupole.

### Bending and steering components

If a map contains a transverse magnetic field on the axis (a dipole component), the particles are deflected. The kernel integrates the map, so the deflection is included; the linear preview does not include it and warns when the estimated angle exceeds 1 mrad. A field map is a straight box along the $z$ axis, and particles leaving the axis gradually move out of the region it covers, so maps are suited to small-angle deflection and steering; for large-angle dipoles use `bend` and `edge` (see [Dipole](#dipole)), and steering coils can also be described by a [steerer](#steerer). How the kernel defines the reference trajectory for a large deflection in a map is not documented in the manual.

## Equations of motion and transfer matrices {#static-fields-equations}

### The equation integrated by the t-code

In a field-map element the kernel integrates the Lorentz equation for every particle

$$
\frac{\dd(\gamma m\boldsymbol{v})}{\dd t} = q\,\bigl(\boldsymbol{E}(\boldsymbol{r}) + \boldsymbol{v}\times\boldsymbol{B}(\boldsymbol{r})\bigr),\qquad
\boldsymbol{E} = K_e\,\boldsymbol{E}_{map},\quad \boldsymbol{B} = K_b\,\boldsymbol{B}_{map}
$$

with the time step set by `steppercycle` in input.txt. A static magnetic field does no work and the kinetic energy is constant; a static electric field changes it by the energy conservation of the previous section.

### Linearization

Expanding the fields to first order near the axis, the transverse equation of motion is

$$
x'' + \frac{(\beta\gamma)'}{\beta\gamma}\,x' + \left(\frac{1}{B\rho}\frac{\partial B_y}{\partial x} - \frac{1}{E\rho}\frac{\partial E_x}{\partial x}\right)x = 0
$$

(The $x$–$y$ coupling of a solenoid is not written here; with an axial magnetic field use the rotating frame or 4×4 matrices, see [Solenoid](#solenoid).) The second term comes from the momentum change in electric fields. If the momentum change in the element is negligible, replacing the field by a hard-edge element of effective gradient $G_{eff}$ and length $L_{eff}$ brings us back to the matrix model: the thick-lens matrix of the [Quadrupole](#quadrupole) chapter, or for a solenoid the coupled matrix with $k = B_{eff}/(2B\rho)$, $C = \cos kL$, $S = \sin kL$.

### How good is a hard-edge equivalent

Compare two lattices on the example (no space charge, linear preview): drift 0.05 m + map `sol` ($K_b = 0.531$) + drift 0.3 m, and a hard-edge `solenoid` with $B = 2.85\ \mathrm{T}$, $L = 0.136\ \mathrm{m}$ at the same centre. Both have the waist at $z \approx 0.39\ \mathrm{m}$, with rms waists of 0.285 mm and 0.310 mm; at $z = 0.7\ \mathrm{m}$ the rms sizes are 2.69 mm and 2.43 mm, about 10 % apart. With strong focusing a hard-edge equivalent is good for a first design only; accurate work needs the map.

> **AVAS convention**: the linear preview cuts maps into 4 mm slices and treats each as "half a step of solenoid body + a thin lens from the near-axis transverse gradients + half a step"; the solenoid fringes enter through the gradient terms. The kernel integrates directly in the 3D field. Both include fringe fields; the kernel also includes the non-linear terms.

## In AVAS {#static-fields-in-avas}

```text
field  L  R  0  2  0  0  Ke  0   file      ! static electric: .esx .esy .esz
field  L  R  0  3  0  0  Ke  Kb  file      ! static magnetic: .bsx .bsy .bsz
field  0.1   0.02  0  3  0  0  1  0.677098  sol_1     ! manual example
field  0.35  0.02  0  3  0  0  1  0.531     sol       ! example solenoid
```

```avas-params
field
```

- Field type 2 (static electric) or 3 (static magnetic).
- **The frequency `f` must be 0**; `phase` is 0; `V3` has no meaning for static fields, write 0. The manual's rule is "parameters that do not exist are written 0".
- Static electric fields are in MV/m and multiplied by `Ke`; static magnetic fields are in T and multiplied by `Kb`.
- For static magnetic maps the manual example writes `Ke` = 1; without electric component files it has nothing to act on. For static electric maps write `Kb` = 0 by the manual's rule.
- `file` is the field-map name (without extension); static electric maps need `.esx/.esy/.esz`, static magnetic maps `.bsx/.bsy/.bsz`; the format is in [Field maps](#field-maps).
- `L` should equal the length in the map header; `R` is the aperture radius of the loss criterion.

> **AVAS convention**: for a static magnetic quadrupole map the sign of `Kb` is the polarity; for a solenoid it only sets the rotation direction. The element list of the GUI sorts field-map elements by field type into RF, electrostatic and magnetostatic; the specific kind of a magnetostatic map (solenoid, quadrupole …) is guessed from the file name, shown with "(guessed)" and explained in the details — a display hint, not physical data.

> **Note**: errors of static magnetic elements are described with the `err_quad_*` commands (static-field element errors) and RF cavities with `err_cav_*`; see [Errors and correction](#errors).

## Adjustable parameters and typical values {#static-fields-parameters}

| Parameter | Element | Effect | Example / typical value |
|---|---|---|---|
| `Kb` | magnetostatic solenoid | field scale, focal length $\propto 1/K_b^2$ | example 0.531 ($\int B\,\dd z = 0.387\ \mathrm{T\,m}$, $f \approx 0.12\ \mathrm{m}$) |
| `Kb` | magnetostatic quadrupole | gradient and polarity | $K_b = G$ for a map normalized to 1 T/m; room-temperature quadrupoles generally below a few tens of T/m |
| `Ke` | electrostatic quadrupole, einzel lens | field scale | for a map exported at 1 kV electrode voltage $K_e$ = voltage / kV; low-energy electrodes are often at a few to tens of kV |
| `L` | all | element length | equal to the map header length |
| `R` | all | loss aperture | equal to the real aperture, not larger than the transverse extent of the map |

Since the focusing strength is $\propto K_b^2$ (solenoid) or $\propto K_b$ (quadrupole), `Kb` can be scanned directly with the parameter scan: `avas scan --input InputFile --target NAME --param Kb --values …`.

## Dynamic effects and common problems {#static-fields-pitfalls}

- **Truncated maps**: the field should have decayed to negligible values at both ends of the map. A map cut off at its boundary gives the particles a spurious edge kick; the example `sol` has fallen to about 0.3 % of the peak at both ends.
- **Element length different from the map length**: what the kernel does then is not documented; the linear preview uses the field in $[0, \min(L, L_{map})]$ only. Always make `L` equal the map header.
- **Overlapping fields**: when the fringes of neighbouring elements overlap, do not simply put the two maps end to end (the overlapping parts would be cut off); use [Superpose](#superpose).
- **Solenoid coupling**: a solenoid couples $x$ and $y$ and rotates the beam; after a solenoid the rms $x$ and $y$ of a non-round beam are no longer independent, which matters especially after quadrupoles.
- **Electrostatic reflection**: in a decelerating electrostatic element, if $q\Phi$ exceeds the kinetic energy the particles stop or are reflected; the linear preview reports "the reference particle stops", and such particles cannot pass the element.
- **Non-zero frequency**: `f` of a static field must be 0; writing the beam frequency suggests an RF field, and by the rules it must be 0.
- **Noise**: the example maps are noisy; squared quantities such as $\int B^2$ are raised systematically by the noise (about 3 % in the example), while linear quantities such as $\int B$ are hardly affected; see [noise in field maps](#field-maps-interpolation).
- **Large-angle bends**: do not describe a large-angle dipole by a straight field-map box; see [bending components](#static-fields-magnetic).

## Hands-on {#static-fields-lab}

**Lab 1: reading the map.** Open the example solenoid in the field-map viewer: look at the longitudinal cut of `bsz`, then switch to the $x$–$y$ cross-section with arrows on and move the cut to the entrance and to the exit. Check that the transverse field points to the axis at the entrance and away from it at the exit, and that it is close to zero in the central cross-section.

```avas-field
map sol
```

**Lab 2: hard-edge equivalent.** Below is the schematic field of a hard-edge solenoid with the same $\int B\,\dd z$ and $\int B^2\,\dd z$ as the example map (computed from the parameters, hard edges, not field-map data); compare its longitudinal profile with the map above.

```avas-field
solenoid B=2.85 L=0.136 R=0.02
```

**Lab 3: focusing and rotation.** In the solenoid lab with the equivalent parameters, look at the Larmor rotation angle $kL$ (about $62^\circ$), the difference between thick- and thin-lens focal lengths, and how the waist moves with B. Then lower W to 0.05 MeV: the thin-lens focal length shrinks about 30 times as $f \propto (B\rho)^2 \propto W$, while $kL$ grows to about 6 rad and the beam is over-focused several times inside the solenoid, so the thin-lens picture fails completely — low-energy sections need much weaker solenoids.

```avas-lab
solenoid L=0.136 B=2.85 W=1.527 d1=0.05 d2=0.3
```

## Further reading {#static-fields-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, chapter 3 (paraxial ray equation, electrostatic and magnetic lenses, solenoids and Busch's theorem).
- H. Wollnik, *Optics of Charged Particles*: transfer matrices of electrostatic and magnetic elements.
- P. W. Hawkes, E. Kasper, *Principles of Electron Optics*: systematic derivation of einzel lenses and paraxial optics.
- In this primer: [Solenoid](#solenoid), [Quadrupole](#quadrupole), [Dipole](#dipole), [Field maps](#field-maps), [Superpose](#superpose), [References](#references).
