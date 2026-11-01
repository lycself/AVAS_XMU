# Electrostatic and magnetostatic field maps

A 30 keV proton leaving an ion source feels the same force in a 1 T magnetic field as in a 2.4 MV/m electric field; at 1.527 MeV the same 1 T is worth 17 MV/m. Why are electrostatic lenses common in low-energy beam lines while high-energy sections use only magnets? And another question: AVAS already has matrix elements such as `quad` and `solenoid`, so why describe a magnet that does not even change in time with a field map of several hundred thousand numbers?

This chapter answers both with two ideas: the electric force does not depend on velocity while the magnetic force is proportional to it; and a static field in vacuum is completely determined by its distribution on the axis, whose longitudinal shape (the fringe field) acts on the beam in a way the hard-edge model leaves out. Field type 2 of the `field` element is an **electrostatic field** (einzel lenses, electrostatic quadrupoles, deflector plates …), type 3 a **magnetostatic field** (solenoids, quadrupoles, magnets with bending components … described by maps). Read [Solenoid](#solenoid) and [Quadrupole](#quadrupole) first; the field-map files themselves are covered in [Field maps](#field-maps).

Start with the example project's magnetostatic solenoid map sol (Kb = 0.35, smoothed on-axis peak about 2.9 T). Click the element to see the on-axis distribution of the map. In the linear preview, rms x is largest near the solenoid centre (about 1.40 mm), the beam is then focused to a waist of 0.81 mm at $z \approx 0.62\ \mathrm{m}$, and it is 0.97 mm at the end; with Kb set to 0 it would be 3.09 mm at the end.

```avas-beamline
drift 0.05 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.4 0.02 0
```

The numerical examples of this chapter use the example project's proton beam: $W = 1.527\ \mathrm{MeV}$, $\beta = 0.0570$, $\beta\gamma = 0.0571$, $p = 53.55\ \mathrm{MeV}/c$, $B\rho = 0.1786\ \mathrm{T\,m}$.

## Electric or magnetic: velocity decides {#static-fields-role}

The Lorentz force $\boldsymbol{F} = q(\boldsymbol{E} + \boldsymbol{v}\times\boldsymbol{B})$ already gives half of the answer: the force of a transverse electric field does not depend on velocity, that of a transverse magnetic field is proportional to it. The two are equivalent when $E = vB = \beta cB$:

| Proton kinetic energy | β | 1 T is worth |
|---|---|---|
| 30 keV (typical at an ion source exit) | 0.0080 | 2.4 MV/m |
| 1.527 MeV (example) | 0.0570 | 17 MV/m |

The other half is engineering: the field between DC electrodes is limited by breakdown and in practice reaches a few to a dozen or so MV/m (typical values), whereas 1 T is easy for a room-temperature magnet and superconducting solenoids reach several tesla. Taking 10 MV/m against 1 T as the dividing line, the two break even at $\beta = 10^7/(3\times 10^8) \approx 0.033$, a proton kinetic energy of about 0.52 MeV. Below that scale an electric field of the same technical difficulty is "worth more"; above it the magnetic field wins, and wins more the faster the beam.

That is why electrostatic elements appear mainly at low energy. They also have no hysteresis and their voltage can be changed quickly; the price is that the electric field sweeps away the low-energy electrons that compensate space charge, so low-energy transport has to weigh electrostatic against magnetic focusing; see [Ion source and LEBT](#journey-source).

> **Think about it**: an RFQ works between tens of keV and a few MeV. Why does it focus with an electric quadrupole field rather than magnetic quadrupoles?
>
> Answer: for the same reason. The electric force does not weaken as the velocity drops; at $\beta \sim 0.01$ the same force needs a magnetic field $1/\beta \sim 100$ times the electric one (comparing $E$ with $cB$ in SI units). See [RFQ](#journey-rfq).

Typical uses of these elements in a beam line:

- **Low-energy transport**: sections from tens to hundreds of keV often use einzel lenses and electrostatic quadrupoles.
- **Transverse focusing in superconducting linacs**: between low-β cavities the superconducting solenoids have strong fields and long fringes that often overlap the neighbouring cavity fields; a map is more reliable than a hard-edge model there; see [Solenoid](#solenoid) and [Superposed fields](#superpose).
- **Combined-function elements**: a quadrupole with bending or steering coils in the same magnet, or a magnet with significant higher-order fields, can be described in one map.
- **Checking matrix models**: comparing a map calculation with hard-edge [quadrupole](#quadrupole) or [solenoid](#solenoid) matrices shows how large the hard-edge error is.

## Electrostatic fields: lenses that change the energy {#static-fields-electric}

### Energy follows the potential

An electrostatic field is conservative, $\boldsymbol{E} = -\nabla\Phi$, and the total energy of a particle in an electrostatic element is conserved:

$$
W(\boldsymbol{r}) + q\,\Phi(\boldsymbol{r}) = \text{const}
$$

So the particle is accelerated or decelerated between the electrodes and recovers its kinetic energy when it leaves the element at the original potential. This is the most basic difference from magnetic elements: a magnetic field does no work and the particle's speed is constant in a magnet, while in an electrostatic element speed, momentum and focusing strength all change with position. That is exactly why a constant-energy matrix will not do and the motion must be integrated in the field. AVAS's linear preview also includes the on-axis $E_z$ of a type-2 map in the reference particle's energy.

### Electric rigidity

The bending power of a magnetic field is measured by the magnetic rigidity $B\rho = p/q$. The same derivation for an electrostatic field, with the transverse electric field providing the centripetal force $qE = \gamma mv^2/\rho = pv/\rho$, gives the **electric rigidity**

$$
E\rho = \frac{p\,v}{q} = \frac{\beta^2\gamma\,mc^2}{q}\ \xrightarrow{\ \beta\ll 1\ }\ \frac{2W}{q}
$$

For the example proton $E\rho = 53.55\ \mathrm{MeV}/c\times 0.0570 \approx 3.05\ \mathrm{MV}$ (non-relativistic $2W/q = 3.054\ \mathrm{MV}$). A transverse field $E$ gives a radius of curvature $\rho = E\rho/E$, e.g. $\rho \approx 3.05\ \mathrm{m}$ for $E = 1\ \mathrm{MV/m}$. $E\rho$ is proportional to the kinetic energy while $B\rho$ is proportional to the momentum ($\propto\sqrt{W}$ non-relativistically), so the electric rigidity falls faster as the energy drops: once more, electric fields are more effective at low energy.

### Electrostatic quadrupole

An ideal electrostatic quadrupole has four hyperbolic electrodes at $\pm V$ and aperture radius $a$; the potential is

$$
\Phi = \frac{V}{a^2}\,(x^2 - y^2),\qquad E_x = -\frac{2V}{a^2}\,x,\qquad E_y = +\frac{2V}{a^2}\,y
$$

With the electric gradient $G_E = 2V/a^2$ (V/m²) the linear equations of motion are

$$
x'' + k\,x = 0,\quad y'' - k\,y = 0,\qquad k = \frac{G_E}{E\rho}
$$

For $V > 0$ a positive particle is focused in $x$ and defocused in $y$; the transfer matrices have exactly the form of a magnetic quadrupole (see [Quadrupole](#quadrupole)). Here the electrode on the $x$ axis is at $+V$, and the approximation assumes a kinetic energy much larger than $qV$, otherwise the energy change inside the quadrupole cannot be neglected.

Put in numbers: $V = \pm 10\ \mathrm{kV}$ and $a = 15\ \mathrm{mm}$ give $G_E = 8.9\times 10^7\ \mathrm{V/m^2}$. For the example proton $k \approx 29\ \mathrm{m^{-2}}$, equivalent to a magnetic quadrupole of gradient $kB\rho \approx 5.2\ \mathrm{T/m}$; for a 50 keV proton the same electrostatic quadrupole gives $k \approx 890\ \mathrm{m^{-2}}$, equivalent to 29 T/m, hard to reach with a room-temperature magnetic quadrupole of such a small aperture.

### Einzel lens

An einzel lens consists of three coaxial cylindrical electrodes, the outer two grounded and the middle one at a voltage; entrance and exit kinetic energies are equal. In the non-relativistic, axially symmetric, paraxial limit, let $\hat\Phi(z)$ be the on-axis potential referenced to zero kinetic energy ($q\hat\Phi = W$). The trajectory satisfies the paraxial ray equation

$$
r'' + \frac{\hat\Phi'}{2\hat\Phi}\,r' + \frac{\hat\Phi''}{4\hat\Phi}\,r = 0
$$

The second term is the "damping" from the changing momentum, the third the action of the radial field. The Picht transformation $R = r\,\hat\Phi^{1/4}$ removes the first-derivative term:

$$
R'' + \frac{3}{16}\left(\frac{\hat\Phi'}{\hat\Phi}\right)^{2}R = 0
$$

The coefficient is always positive, so **an einzel lens always focuses, whether the middle electrode is at positive or negative voltage**. In the weak-lens approximation the focal length is

$$
\frac{1}{f} \approx \frac{3}{16}\int\left(\frac{\hat\Phi'}{\hat\Phi}\right)^{2}\dd z
$$

Only $\hat\Phi'/\hat\Phi$ appears, so the focal length depends on the electrode geometry and on the ratio of voltage to kinetic energy $qV/W$, not on the beam energy itself: double the energy and you must double the voltage to keep the focal length. A typical example: with a Gaussian on-axis potential of $1/\mathrm{e}$ half-width 20 mm that halves the kinetic energy at the centre (decelerating mode), $f \approx 0.17\ \mathrm{m}$; with the same voltage of opposite sign, raising the kinetic energy to 1.5 times at the centre (accelerating mode), $f \approx 0.54\ \mathrm{m}$. The decelerating mode is about 3 times stronger because the denominator $\hat\Phi$ is smaller where the field is strongest; but if $q\Phi$ approaches the kinetic energy the particle stops in the lens or is reflected. When the focal length is comparable to the lens length the weak-lens approximation is only an estimate. Relativistic corrections are small at the energies of this primer; the full form is in Reiser, chapter 3.

> **Think about it**: in an einzel lens the radial field $E_r = \tfrac{r}{2}\Phi''$ defocuses where the potential curves upward and focuses where it curves downward, and both regions are present. Why is the net effect always focusing?
>
> Answer: look at where the particle is slow. In the decelerating mode the potential has a hump in the middle; the middle is the focusing region, and the particle is slowest there and feels the force longest. In the accelerating mode the middle is the defocusing region, but the particle is fastest there and feels it shortest; the focusing regions are at the ends. Either way the focusing region "wins". It is a second-order effect, hence the focal length goes as $(\hat\Phi'/\hat\Phi)^2$, independent of the sign of the voltage.

### Electrostatic deflection

A radial electric field between cylindrical or spherical electrodes bends the beam with radius $\rho = E\rho/E$. The focusing of an electrostatic bend in the bending plane differs from that of a magnetic bend (it depends on the electrode shape), and the kinetic energy changes with the potential during the bend. AVAS's dipole `bend` describes magnetic bending only; electrostatic deflection can only be described by a field map, see [In AVAS](#static-fields-in-avas).

## Magnetostatic fields: the axis tells everything {#static-fields-magnetic}

### Why the on-axis field is enough

In a vacuum region without currents or magnetic material, a static magnetic field satisfies $\nabla\cdot\boldsymbol{B} = 0$ and $\nabla\times\boldsymbol{B} = 0$; it is the gradient of a scalar potential that obeys Laplace's equation. Solutions of Laplace's equation are fixed by their boundary values, and for an axially symmetric field the expansion near the axis follows from the on-axis $B_z(z)$ and its derivatives alone:

$$
B_r(r,z) = -\frac{r}{2}\,B_z'(z) + \frac{r^3}{16}\,B_z'''(z) - \cdots,\qquad
B_z(r,z) = B_z(z) - \frac{r^2}{4}\,B_z''(z) + \cdots
$$

The first term is easy to see: take a small cylinder of radius $r$ and length $\dd z$; $\nabla\cdot\boldsymbol{B} = 0$ requires the change of flux through the end faces, $\pi r^2 B_z'\,\dd z$, to be made up by the flux $2\pi r\,B_r\,\dd z$ through the side, so $B_r = -\tfrac{r}{2}B_z'$. In other words, **wherever the on-axis field changes along $z$, there is a radial field**. A hard-edge model draws $B_z$ as a square pulse and thereby squeezes these radial fields into two infinitely thin "kicks" at the ends.

### Solenoid: first spin, then pull in

At the entrance $B_z' > 0$ and $B_r$ points towards the axis; at the exit $B_z' < 0$ and $B_r$ points away from it (for $B_z > 0$). Crossing the entrance fringe, the particle gets an azimuthal velocity from $F_\theta = qv_zB_r$; inside the solenoid body $F_r = qv_\theta B_z$ pulls it towards the axis. Solenoid focusing is a relay between the fringe and the main field. By Busch's theorem, in a frame rotating at the Larmor frequency the two transverse planes decouple and satisfy

$$
r'' + \left(\frac{B_z(z)}{2B\rho}\right)^{2} r = 0,\qquad
\theta_L = \int\frac{B_z}{2B\rho}\,\dd z,\qquad
\frac{1}{f} \approx \int\left(\frac{B_z}{2B\rho}\right)^{2}\dd z
$$

The focusing strength is proportional to $B_z^2$ and **independent of the field direction**; changing the sign of $K_b$ only changes the sense of rotation of the beam. Check with the beam line above: changing Kb from 0.35 to −0.35 leaves the rms curves of the linear preview exactly the same (waist at $z = 0.62\ \mathrm{m}$, 0.81 mm, in both cases).

### The example solenoid sol

The map `sol` is 0.35 m long; the on-axis $B_z$ is largest near the centre (after 9-point smoothing the peak is at $z \approx 0.177\ \mathrm{m}$, centre 0.175 m), the full width at half maximum is about 74 mm, and at both ends it has dropped to about 0.3 % of the peak. The smoothed peak is about $8.3K_b$ T. For the $K_b = 0.35$ of this chapter's beam line and the $K_b = 0.531$ of the manual's superposition example:

| Quantity | $K_b = 0.35$ | $K_b = 0.531$ | Note |
|---|---|---|---|
| Smoothed peak field | 2.90 T | 4.40 T | Raw data jump from point to point; the raw maximum is about 20 % above the smoothed peak |
| $\int B_z\,\dd z$ | 0.255 T·m | 0.387 T·m | Insensitive to noise |
| Larmor angle $\theta_L$ | $41^\circ$ | $62^\circ$ | $\int B_z\,\dd z/(2B\rho)$ |
| $\int B_z^2\,\dd z$ (smoothed) | 0.479 T²·m | 1.10 T²·m | Raw data about 3 % higher |
| Thin-lens focal length $f$ | 0.27 m | 0.116 m | The latter is comparable to the length; the thin lens is only an order-of-magnitude estimate |
| Equivalent hard edge $B_{eff} = \int B^2/\int B$ | 1.88 T | 2.85 T | |
| Equivalent length $L_{eff} = (\int B)^2/\int B^2$ | 0.136 m | 0.136 m | Depends on the field shape only; much shorter than the 0.35 m map |

In the viewer below, look at the longitudinal cut of `bsz` first, then switch to an $x$–$y$ cross-section with arrows on and move it to the entrance and to the exit: the transverse field points towards the axis at the entrance and away from it at the exit, and is close to zero in the central section. Raw map values at $x = 1\ \mathrm{cm}$, $y = 0$ (map units, i.e. T for $K_b = 1$): on the entrance flank at $z = 0.12\ \mathrm{m}$, $B_x \approx -0.55$, while $-\tfrac{r}{2}B_z'$ from the smoothed on-axis field gives $-0.59$; on the exit flank at $z = 0.23\ \mathrm{m}$, $B_x \approx +0.35$ against $+0.65$. Point by point the agreement is only in sign and order of magnitude, because the derivative of the noisy on-axis field is itself uncertain (at the steepest point, $z = 0.1435\ \mathrm{m}$: $-0.99$ against $-1.44$). Integrated, the relation holds well: the near-axis gradient $\partial B_x/\partial x$ of the map, integrated from the entrance to the peak, gives $-3.98\ \mathrm{T}$, against $-\tfrac12 B_{z,\text{peak}} = -4.14\ \mathrm{T}$ (4 % apart).

```avas-field
map sol
```

### Quadrupole maps

Near the axis of a magnetic quadrupole $B_y = G\,x$ and $B_x = G\,y$, with $k = G/(B\rho)$; a positive particle is focused in $x$ for $k > 0$. Quadrupole maps are usually exported normalized to a reference gradient (e.g. 1 T/m at the centre), so that $K_b$ is the actual gradient in T/m. Unlike the solenoid, the quadrupole force is linear in $B$, and **the sign of $K_b$ is the polarity**: for $K_b < 0$ the focusing and defocusing planes swap. With fringe fields the equivalent length $L_{eff} = \int G(z)\,\dd z/G_0$ is used for comparison with a hard-edge quadrupole.

### Bending and steering components

If the map contains a transverse magnetic field on the axis (a dipole component), particles are deflected. The engine integrates in the map, so the deflection is included naturally; the linear preview ignores deflection and warns when the estimated angle exceeds 1 mrad. In space a field map is a straight box along the $z$ axis, and a particle moving off the axis gradually leaves the region the map covers; maps therefore suit small-angle bending and steering. For large-angle dipoles use `bend` with `edge` (see [Dipole](#dipole)); steering coils can also be described with a [steerer](#steerer). How the engine defines the reference trajectory for large deflections in a map is not documented in the manual.

## Field map or hard edge {#static-fields-equations}

### The equations of the t-code

In a field-map element the engine integrates the Lorentz equation for every particle

$$
\frac{\dd(\gamma m\boldsymbol{v})}{\dd t} = q\,\bigl(\boldsymbol{E}(\boldsymbol{r}) + \boldsymbol{v}\times\boldsymbol{B}(\boldsymbol{r})\bigr),\qquad
\boldsymbol{E} = K_e\,\boldsymbol{E}_{map},\quad \boldsymbol{B} = K_b\,\boldsymbol{B}_{map}
$$

with the time step set by `steppercycle` in input.txt. A magnetostatic field does no work and the kinetic energy is constant; an electrostatic field changes it according to energy conservation, as in the previous section.

### Linearization and hard-edge equivalents

Expanding the field to first order near the axis, the transverse equation of motion can be written

$$
x'' + \frac{(\beta\gamma)'}{\beta\gamma}\,x' + \left(\frac{1}{B\rho}\frac{\partial B_y}{\partial x} - \frac{1}{E\rho}\frac{\partial E_x}{\partial x}\right)x = 0
$$

(the $x$–$y$ coupling of a solenoid is not written here; with an axial magnetic field use the rotating frame or 4×4 matrices, see [Solenoid](#solenoid)). The second term comes from the momentum change in electrostatic fields. If that change is negligible inside the element, replacing the field by a hard-edge element with equivalent gradient $G_{eff}$ and length $L_{eff}$ leads back to the matrix model: the thick-lens matrix of the [Quadrupole](#quadrupole) chapter for a quadrupole, the coupled matrix with $k = B_{eff}/(2B\rho)$, $C = \cos kL$, $S = \sin kL$ for a solenoid.

The $B_{eff}$ and $L_{eff}$ of the table above are chosen so that the hard-edge solenoid has the same $\int B\,\dd z$ as the map (same rotation) and the same $\int B^2\,\dd z$ (same thin-lens focal length). If both integrals match, why is there still a difference? Because they are thin-lens quantities. When the focal length is comparable to the element length, the beam size already changes inside the field and the way the field is distributed along $z$ starts to matter.

### How good is the hard-edge equivalent

Compare two lattices in the example (example current 0.184 mA with the preview's linear space charge, rms $x$): drift 0.05 m + map `sol` + drift 0.3 m, against the equivalent hard-edge `solenoid` ($L = 0.136\ \mathrm{m}$) at the same centre position:

| | Map rms waist | Hard-edge rms waist | At $z = 0.7\ \mathrm{m}$, map / hard edge |
|---|---|---|---|
| $K_b = 0.35$ ($B_{eff} = 1.88\ \mathrm{T}$, $f/L_{eff} \approx 2$) | 0.811 mm @ 0.62 m | 0.828 mm @ 0.62 m | 0.845 / 0.857 mm |
| $K_b = 0.531$ ($B_{eff} = 2.85\ \mathrm{T}$, $f/L_{eff} \approx 0.85$) | 0.285 mm @ 0.388 m | 0.311 mm @ 0.402 m | 2.73 / 2.42 mm |

With weak focusing the two differ by about 2 %; with strong focusing they are about 13 % apart at $z = 0.7\ \mathrm{m}$. Rule of thumb: when $f$ is several times the element length the hard-edge equivalent is good enough for a first design; when $f$ is comparable to the length, accurate work should use the map.

Below is the schematic field of the $K_b = 0.531$ equivalent hard-edge solenoid (computed from the parameters, hard edge, not map data); compare its longitudinal distribution with the map above:

```avas-field
solenoid B=2.85 L=0.136 R=0.02
```

In the solenoid lab, use these equivalent parameters and look at the Larmor angle $kL$ (about $62^\circ$), at the difference between the thick-lens (0.142 m) and thin-lens (0.116 m) focal lengths, and at how the waist moves with B. Then lower W to 0.05 MeV: the thin-lens focal length shrinks with $f \propto (B\rho)^2 \propto W$ by about 30 times (0.116 m → 3.8 mm), $kL$ grows to about 6 rad, the beam goes through several foci inside the solenoid and the thin-lens approximation fails completely. Low-energy sections need much weaker solenoids.

```avas-lab
solenoid L=0.136 B=2.85 W=1.527 d1=0.05 d2=0.3
```

> **Think about it**: the example map is noisy. Why is $\int B_z\,\dd z$ hardly affected while $\int B_z^2\,\dd z$ comes out systematically about 3 % too high?
>
> Answer: write the measured field as $B + n$, with $n$ noise of zero mean. In the linear integral $n$ cancels between positive and negative values; the squared integral gains an extra $\int n^2\,\dd z$, which is always positive and never cancels. A solenoid focal length computed from a noisy map is therefore too short.

> **AVAS convention**: the linear preview slices a map into 4 mm pieces and treats each as "edge + half a step of solenoid body + a thin lens from the near-axis transverse gradients + half a step of body + edge". The solenoid's radial fringe field comes from the summed on-axis $B_z$ by Maxwell's equations ($\partial B_x/\partial x + \partial B_y/\partial y = -\partial B_z/\partial z$) and sits at both ends of each slice, which keeps the transfer matrices exactly symplectic; the map's $B_x$, $B_y$ only supply the divergence-free part (normal and skew quadrupole content), because in a noisy map these two components disagree with $B_z$. The engine integrates directly in the 3D field. Both include the fringe fields; the engine also includes the nonlinear terms.

## Common misconceptions {#static-fields-pitfalls}

- **The sign of Kb**: for a solenoid the sign of Kb only changes the sense of rotation, the focal length stays the same; for a quadrupole the sign of Kb is the polarity, and reversing it swaps the focusing and defocusing planes.
- **The map length is not the equivalent length**: the example map `sol` is 0.35 m long, its equivalent hard-edge length only 0.136 m. A hard-edge model built from the map length and the peak field overestimates the focusing badly.
- **The raw maximum is not the peak field**: the raw maximum of `sol.bsz` is about 20 % above the 9-point smoothed peak; squared quantities ($\int B^2$) are raised about 3 % by the noise, linear ones ($\int B$) hardly at all; see [map noise](#field-maps-interpolation).
- **"An electrostatic element does not change the energy"**: the kinetic energy is restored only after leaving the element at the original potential; inside it, kinetic energy, speed and focusing strength all change, and in a decelerating element a particle stops or is reflected if $q\Phi$ exceeds its kinetic energy.
- **Mixing up electric and magnetic rigidity**: $E\rho \approx 2W/q$ is proportional to the kinetic energy, $B\rho = p/q$ to the momentum. Halving the energy doubles the strength of an electrostatic element but only raises that of a magnetic one by about $\sqrt 2$.
- **"There is no radial field in a magnet"**: wherever the on-axis field changes along $z$ there is $B_r = -\tfrac{r}{2}B_z'$; the hard-edge model compresses it into kicks at the ends, which does not mean it is absent.
- **Large-angle bends**: do not describe a large-angle dipole with a straight field-map box, see [bending components](#static-fields-magnetic).

## In AVAS {#static-fields-in-avas}

### Syntax and parameters

```text
field  L  R  0  2  0  0  Ke  0   file      ! electrostatic: .esx .esy .esz
field  L  R  0  3  0  0  Ke  Kb  file      ! magnetostatic: .bsx .bsy .bsz
field  0.1   0.02  0  3  0  0  1  0.677098  sol_1     ! manual example
field  0.35  0.02  0  3  0  0  1  0.531     sol       ! example solenoid
```

```avas-params
field
```

- Field type 2 (electrostatic) or 3 (magnetostatic). AVAS has no matrix-model element for electrostatic lenses: electrostatic focusing elements can only be described by type-2 maps; a pure deflection can also be described by an electric steerer (`steerer` of kind 1), see [Steerer](#steerer).
- **The frequency `f` must be 0**; `phase` is 0; `V3` has no meaning for static fields and is 0. The manual's rule is "write 0 for parameters that do not apply". Writing the beam frequency suggests an RF field.
- Electrostatic fields are in MV/m and multiplied by `Ke`; magnetostatic fields are in T and multiplied by `Kb`.
- For magnetostatic fields the manual example writes `Ke` as 1; without electric component files it has nothing to act on (this chapter's beam line writes 0, with the same effect). For electrostatic fields, by the manual's rule, `Kb` is 0.
- `file` is the field map name (without extension); electrostatic maps need `.esx/.esy/.esz`, magnetostatic maps `.bsx/.bsy/.bsz`; format in [Field maps](#field-maps).
- `L` should equal the length in the map header; `R` is the loss aperture radius.

> **AVAS convention**: for a magnetostatic quadrupole map the sign of `Kb` is the polarity; for a solenoid it only changes the sense of rotation. The interface's element list sorts field-map elements by field type into RF, electrostatic and magnetostatic; the specific kind of a magnetostatic map (solenoid, quadrupole …) is guessed from the map's file name, shown with "(inferred)", and the reason is given in the details. This guess is for display only, not physical data.

> **Note**: errors of magnetostatic elements are described with the `err_quad_*` commands (static magnet errors), RF cavities with `err_cav_*`; see [Errors and correction](#errors).

### Typical values

| Parameter | Element | Role | Example / typical value |
|---|---|---|---|
| `Kb` | Magnetostatic solenoid | Field scale, focal length $\propto 1/K_b^2$ | Example 0.531 ($\int B\,\dd z = 0.387\ \mathrm{T\,m}$, $f \approx 0.12\ \mathrm{m}$); this chapter's beam line 0.35 ($f \approx 0.27\ \mathrm{m}$) |
| `Kb` | Magnetostatic quadrupole | Gradient and polarity | $K_b = G$ for a map normalized to 1 T/m; room-temperature quadrupoles generally stay within a few tens of T/m |
| `Ke` | Electrostatic quadrupole, einzel lens | Electric field scale | For a map exported at 1 kV electrode voltage, $K_e$ = voltage / kV; low-energy electrode voltages are typically a few to a few tens of kV |
| `L` | All | Element length | Equal to the map header length |
| `R` | All | Loss aperture | Equal to the real aperture, not larger than the map's transverse range |

Since the focusing strength is $\propto K_b^2$ (solenoid) or $\propto K_b$ (quadrupole), `Kb` can be scanned directly with a parameter scan: `avas scan --input InputFile --target <element> --param Kb --values …`.

### Pitfalls

- **Truncated maps**: the field should have decayed to negligible values at both ends of the map. A map cut off at its boundary gives the particle a spurious edge kick; the example `sol` has dropped to about 0.3 % of the peak at both ends.
- **Element length different from map length**: the manual does not say how the engine handles a mismatch; the linear preview uses only the field within $[0, \min(L, L_{map})]$. Make `L` equal to the map header.
- **Overlapping fields**: when the fringe fields of neighbouring elements overlap, two maps cannot simply be placed end to end (the overlap would be cut away); use [superposed fields](#superpose).
- **Solenoid coupling**: a solenoid couples $x$ and $y$ and rotates the beam; after a solenoid the rms $x$ and $y$ of a non-round beam are no longer independent, which matters in particular when a solenoid follows quadrupoles.
- **Electrostatic reflection**: in a decelerating electrostatic element a particle stops or is reflected if $q\Phi$ exceeds its kinetic energy; the linear preview reports where the reference particle stops and ends there. Such particles cannot pass the element.
- **Large-angle bends**: neither electrostatic deflection nor large-angle magnetic bending is suited to a straight field-map box, see [bending components](#static-fields-magnetic).

## Further reading {#static-fields-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, chapter 3 (paraxial ray equation, electrostatic and magnetic lenses, solenoids and Busch's theorem).
- H. Wollnik, *Optics of Charged Particles*: transfer matrices of electrostatic and magnetic elements.
- P. W. Hawkes, E. Kasper, *Principles of Electron Optics*: systematic derivation of einzel lenses and paraxial optics.
- AVAS manual `docs/使用说明20260427.docx`: field types and parameters of the `field` element.
- In this primer: [Solenoid](#solenoid), [Quadrupole](#quadrupole), [Dipole](#dipole), [Field maps](#field-maps), [Superposed fields](#superpose), [References](#references).
