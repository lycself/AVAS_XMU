# Electrostatic and magnetostatic field maps

The quadrupoles and solenoids of the earlier chapters are "ideal" elements: the field appears suddenly at the entrance, vanishes suddenly at the exit and is equally strong everywhere in between. This picture is called the **hard-edge model**; along the beam the field looks like a box. Real magnets are not like that: there is field outside the coil too, and at both ends it rises and falls gradually. That part is called the **fringe field**.

AVAS already has hard-edge elements such as `quad` and `solenoid`. Why describe a magnet that does not even change in time with a table of several hundred thousand numbers? And a second question: at the very front of an accelerator, where the particles are still slow, focusing is often done with electrodes (electrostatic lenses) rather than magnets, while further on only magnets are used. Why?

This chapter answers both with two ideas:

1. **The electric force does not depend on velocity; the magnetic force is proportional to it.** The slower the particle, the better value an electric field is.
2. **Wherever the field changes along the beam, there must also be a transverse field.** The fringe field pushes the beam; the hard-edge model squeezes that part into two "kicks" at the ends.

AVAS reads field maps (tables of field values on a grid, format in [Field maps](#field-maps)) with the `field` element. Field type 2 is an **electrostatic field** (einzel lenses, electrostatic quadrupoles, deflector plates …), type 3 a **magnetostatic field** (solenoids, quadrupoles, magnets with bending components … described by maps). "Static" means the field does not change in time, unlike an RF cavity (type 1, see [RF cavity](#rf-cavity)). Read [Solenoid](#solenoid) and [Quadrupole](#quadrupole) first.

Start with the example project's magnetostatic solenoid map `sol`. The beam line below is a 0.05 m drift, the 0.35 m long `sol` (field factor Kb = 0.35, smoothed on-axis peak about 2.9 T) and a 0.4 m drift. Click the element to see the on-axis distribution of the map: there is no flat top, just a "hill" falling off gently on both sides. In the linear preview the beam is widest near the solenoid centre (rms x about 1.40 mm), is then focused to a waist of 0.81 mm at $z \approx 0.62\ \mathrm{m}$, and is 0.97 mm at the end; with Kb set to 0 it would be 3.09 mm at the end.

```avas-beamline
drift 0.05 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.4 0.02 0
```

The numerical examples of this chapter use the example project's proton beam: kinetic energy $W = 1.527\ \mathrm{MeV}$, speed $v = \beta c$ with $\beta = 0.0570$ ($v = 1.71\times 10^7\ \mathrm{m/s}$), $\beta\gamma = 0.0571$, momentum $p = 53.55\ \mathrm{MeV}/c$, magnetic rigidity $B\rho = 0.1786\ \mathrm{T\,m}$ (see [Quadrupole](#quadrupole-lorentz)), current 0.184 mA.

## Electric or magnetic: velocity decides {#static-fields-role}

### Two forces

In school physics a charge $q$ in an electric field $E$ feels the force $F = qE$, and moving with speed $v$ perpendicular to a magnetic field $B$ it feels $F = qvB$ (the Lorentz force). The difference is that $v$:

- **The electric force is like a hand**: it pushes equally hard however fast the particle goes.
- **The magnetic force pushes only moving charges**: the faster they go, the harder it pushes.

The two are equal when $qE = qvB$, i.e. $E = vB = \beta cB$. For two typical energies:

| Proton kinetic energy | β | 1 T of magnetic field is worth |
|---|---|---|
| 30 keV (typical at an ion source exit) | 0.0080 | 2.4 MV/m of electric field |
| 1.527 MeV (example) | 0.0570 | 17 MV/m of electric field |

The other half of the answer is engineering. The field between two electrodes is limited by breakdown (sparking), and in practice a DC electric field reaches a few to a dozen or so MV/m (typical values), whereas 1 T is easy for a room-temperature magnet and superconducting solenoids reach several tesla. Taking 10 MV/m against 1 T as the dividing line, the two break even at $\beta = 10^7/(3\times 10^8) \approx 0.033$; with $W = \tfrac12 mv^2$ that is a proton kinetic energy of about 0.52 MeV. Below that scale an electric field of the same technical difficulty pushes harder; above it the magnetic field wins, and wins more the faster the beam.

### Deflection angle: impulse over momentum

What counts in the end is the deflection angle. A particle with speed $v$ crosses an element of length $L$ in the time $t = L/v$; the transverse force $F$ gives it the transverse momentum $Ft$ (the impulse), and the deflection angle is that transverse momentum divided by the forward momentum $p$:

- **Magnetic field**: $\theta = qvB\cdot(L/v)/p = BL/(B\rho)$. The $v$ cancels; the deflection depends only on the magnetic rigidity $B\rho = p/q$.
- **Electric field**: $\theta = qE\cdot(L/v)/p = EL/(E\rho)$, where $E\rho = pv/q$ is called the **electric rigidity** (derived in the next section). A slow particle has little momentum and also stays in the field longer; both effects add up.

As particles slow down, the electric rigidity drops faster than the magnetic rigidity, so at low energy the electric field is more effective. That is why electrostatic elements appear mainly at low energy. They also have no hysteresis and their voltage can be changed quickly; the price is that the electric field sweeps away the low-energy electrons that compensate space charge, so low-energy transport has to weigh electrostatic against magnetic focusing; see [Ion source and LEBT](#journey-source).

Typical uses of these elements in a beam line:

- **Low-energy transport**: sections from tens to hundreds of keV often use einzel lenses and electrostatic quadrupoles.
- **Transverse focusing in superconducting linacs**: between low-β cavities the superconducting solenoids have strong fields and long fringes that often overlap the neighbouring cavity fields; a map is more reliable than a hard-edge model there; see [Solenoid](#solenoid) and [Superposed fields](#superpose).
- **Combined-function elements**: a quadrupole with bending or steering coils in the same magnet, or a magnet with significant higher-order fields, can be described in one map.
- **Checking matrix models**: comparing a map calculation with hard-edge [quadrupole](#quadrupole) or [solenoid](#solenoid) matrices shows how much the hard-edge approximation is off.

## Electrostatic elements: lenses that change the speed {#static-fields-electric}

### Like a ball rolling over a hill

A ball rolling up a hill slows down, rolling down it speeds up again, and back at the original height it has its original speed: potential energy and kinetic energy turn into each other while their sum stays fixed. Electric fields work the same way. The electric potential $\Phi$ (in volts) is the "electric height"; a charge $q$ at potential $\Phi$ has the potential energy $q\Phi$, so

$$
W + q\,\Phi = \text{const}.
$$

The particle is accelerated or decelerated between the electrodes and recovers its kinetic energy when it leaves the element at the original potential. This is the most basic difference from magnetic elements: **the magnetic force is always perpendicular to the velocity and does no work**, so the particle's speed is constant in a magnet, while in an electrostatic element speed, momentum and focusing strength all change with position. A constant-energy transfer matrix will therefore not do; the motion has to be computed step by step in the field. AVAS's linear preview also includes the on-axis longitudinal electric field $E_z$ of a type-2 map in the reference particle's energy.

### Electric rigidity

The magnetic rigidity $B\rho$ answers "how strong a magnetic field bends the particle onto a circle of radius $\rho$". The same question can be asked of an electric field: the transverse electric field provides the centripetal force of the circular motion,

$$
qE = \frac{mv^2}{\rho}\quad\Longrightarrow\quad E\rho = \frac{mv^2}{q} = \frac{2W}{q}.
$$

(This is the form for slow particles; with relativity $E\rho = pv/q$, which returns to $2W/q$ for $\beta \ll 1$.) For the example proton $E\rho = 2\times 1.527\ \mathrm{MV} \approx 3.05\ \mathrm{MV}$; the relativistic formula gives $53.55\ \mathrm{MeV}/c\times 0.0570 \approx 3.05\ \mathrm{MV}$, the same. A transverse field $E = 1\ \mathrm{MV/m}$ bends it onto a circle of $\rho = E\rho/E \approx 3.05\ \mathrm{m}$.

$E\rho$ is proportional to the kinetic energy, $B\rho$ to the momentum ($\propto\sqrt{W}$ for slow particles). When the energy is halved, an electrostatic element becomes twice as strong, a magnetic one only about $\sqrt2$ times.

### Electrostatic quadrupole

The electric version of a quadrupole: four hyperbolic electrodes, two opposite ones at $+V$ and the other two at $-V$, aperture radius $a$. With the electrodes on the $x$ axis at $+V$, a positive charge is pushed back towards the centre in $x$ and pulled towards the $-V$ electrodes in $y$: focusing in $x$, defocusing in $y$, the same "one hand helps, the other hinders" as a magnetic quadrupole (the reason is in the deep part of [Quadrupole](#quadrupole-deep-field)). Near the centre the field is proportional to the distance from it:

$$
E_x = -G_E\,x,\qquad E_y = +G_E\,y,\qquad G_E = \frac{2V}{a^2}.
$$

$G_E$ is the electric gradient, in V/m². Compare the force of the two kinds of quadrupole on a particle at distance $x$: $qG_Ex$ for the electric one, $qvGx$ for the magnetic one. So an electrostatic quadrupole is worth a magnetic quadrupole of gradient

$$
G_{\text{equiv}} = \frac{G_E}{v},
$$

and the slower the particle, the larger the equivalent gradient.

Count it: with $V = \pm 10\ \mathrm{kV}$ and $a = 15\ \mathrm{mm}$, $G_E = 2\times 10^4/(0.015)^2 = 8.9\times 10^7\ \mathrm{V/m^2}$. For the example proton ($v = 1.71\times 10^7\ \mathrm{m/s}$) that is worth a 5.2 T/m magnetic quadrupole; for a 50 keV proton ($v = 3.1\times 10^6\ \mathrm{m/s}$) it is worth 29 T/m, which a room-temperature magnetic quadrupole can hardly reach in such a small aperture. The approximation requires the kinetic energy to be much larger than $qV$; otherwise the change of energy inside the quadrupole cannot be neglected.

### Einzel lens: focusing with either sign of voltage

An **einzel lens** consists of three coaxial tubes: the two outer tubes are grounded and the middle one carries a voltage. A particle passing through is first decelerated, then accelerated (or the other way round), and leaves with the same kinetic energy it came in with.

It has a surprising property: **it focuses whether the middle electrode is at positive or negative voltage**. The reason is that the lens has focusing regions and defocusing regions, and the particle spends different times in them:

- Middle electrode positive (a positive charge is decelerated in the middle): the middle is the focusing region, and the particle is slowest there, feeling the force for the longest time.
- Middle electrode negative (a positive charge is accelerated in the middle): the middle is the defocusing region, but the particle is fastest there, feeling the force for the shortest time; the focusing regions are at the ends.

Either way the focusing regions get the better deal, and the net effect is focusing. This is a second-order effect, so an einzel lens is much weaker than an electrostatic quadrupole at the same voltage.

A typical example: with a bell-shaped (Gaussian, half-width 20 mm) on-axis potential that halves the kinetic energy at the centre, the focal length is about 0.17 m; with the voltage reversed, raising the kinetic energy to 1.5 times at the centre, it is about 0.54 m. The decelerating mode is about 3 times stronger because the particle is slower where the field is strongest; but if $q\Phi$ approaches the kinetic energy, the particle stops in the lens or is turned back. The focal length depends only on the electrode shape and the ratio of voltage to kinetic energy $qV/W$: doubling the energy requires doubling the voltage for the same focal length. The derivation is in [Going deeper: the paraxial equation of electrostatic lenses](#static-fields-deep-electric).

### Electrostatic deflection

The radial field between cylindrical or spherical electrodes can bend a beam with radius $\rho = E\rho/E$. In the bending plane an electrostatic deflector focuses differently from a magnetic one (depending on the electrode shape), and the kinetic energy changes with the potential during the deflection. AVAS's dipole `bend` describes magnetic bending only; electrostatic deflection can only be described by a field map, see [In AVAS](#static-fields-in-avas).

## Magnetostatic fields: the ends must have a radial field {#static-fields-magnetic}

### Field lines do not just disappear

School physics teaches that magnetic field lines are closed curves with no beginning and no end. Put differently: **the magnetic flux entering a closed box always equals the flux leaving it** (magnetic flux = field strength × perpendicular area).

Near the entrance of a solenoid, take a small cylindrical box of radius $r$ and length $\Delta z$ centred on the beam axis. The axial field $B_z$ is growing along $z$:

- the flux entering through the front face is $\pi r^2 B_z(z)$;
- the flux leaving through the back face is $\pi r^2 B_z(z+\Delta z)$, larger by $\pi r^2\,\Delta B_z$;
- the extra can only come in through the side. The side area is $2\pi r\,\Delta z$, so there must be a radial field $B_r$ on the side with $2\pi r\,\Delta z\cdot B_r = -\pi r^2\,\Delta B_z$.

Hence

$$
B_r = -\frac{r}{2}\,\frac{\Delta B_z}{\Delta z}.
$$

This means: **wherever the on-axis field changes along $z$, there is a radial field**, proportional to the distance $r$ from the axis. For $B_z > 0$, at the entrance (field growing) $B_r < 0$ and the transverse field points towards the axis everywhere; at the exit (field falling) $B_r > 0$ and it points away from the axis everywhere; where the field is constant in the middle there is no radial field. The hard-edge model draws $B_z$ as a square wave, which squeezes these radial fields into two infinitely thin "kicks" at the ends of the element.

### Solenoid: first set spinning, then pulled back

A solenoid contains mainly a field $B_z$ along the axis, and the particles also fly mainly along the axis. When the velocity is parallel to the field the Lorentz force vanishes, so how does a solenoid focus? The answer is a relay between the fringe field and the main field:

1. **Entrance: the fringe field sets the particles spinning.** The radial field $B_r$ at the entrance is perpendicular to the forward velocity and produces a force around the axis. For $B_z > 0$, looking from downstream towards upstream (the beam flying towards you, $x$ to the right, $y$ up), a particle at $+x$ is pushed towards $-y$; the whole beam begins to turn clockwise about the axis.
2. **Body: the spinning particles are pulled towards the axis.** Once a particle moves around the axis, that velocity is perpendicular to $B_z$, and the Lorentz force points towards the axis.
3. **Exit: the reversed fringe field "brakes" the spinning.** The particles leave without turning about the axis any more, but they have been pulled towards it, and the beam as a whole has turned through some angle.

Working it out (see [Going deeper: where solenoid focusing comes from](#static-fields-deep-magnetic)) shows that the focusing strength is proportional to $B_z^2$, **independent of the field direction**; changing the sign of Kb only changes the sense of rotation. Check it with the beam line at the top: change Kb from 0.35 to −0.35 and the rms curves of the linear preview do not change at all (both have their waist at $z = 0.62\ \mathrm{m}$, 0.81 mm).

### Animation: an idealised hard-edge solenoid

The elements of this chapter have no animation of their own, so below we borrow the animation of the [Solenoid](#solenoid) chapter, with the parameters of the **equivalent hard-edge solenoid** of the `sol` map at Kb = 0.35: $B = 1.88\ \mathrm{T}$, length $0.136\ \mathrm{m}$ (where these two numbers come from is in the next section, [Field map or hard edge](#static-fields-equations)). The beam passes a 0.05 m drift, the solenoid and a 0.5 m drift, with the example beam's 0.184 mA.

```avas-motion
solenoid B=1.88 Ls=0.136
```

**What to look at**: the 3D view shows a cut-open copper coil inside an iron shell; the field lines run along the axis inside the coil and return through the iron. Drag the play bar to the moment the particles enter the coil and look at the cross-section (seen from downstream; ⊙ means the field points along the beam, out of the screen): the red force arrows first make the particles turn clockwise about the axis and then point towards the axis. The beam is focused in both planes at once; the exit rms x and y are both about 1.19 mm; drag B to 0 and the exit value is 2.75 mm. The panel's "rotation of the beam" is 41.0°, the same as the Larmor angle of 41° of the `sol` map in the table below, because both have the same $\int B\,\dd z$.

**What to drag**: set B to −1.88 and the particles turn anticlockwise instead, but the exit rms and the focal length (0.290 m) stay the same: focusing does not depend on the field direction. Double B to 3.76 T: the focal length shrinks from 0.290 m to 0.096 m, about a third (a quarter for a thin lens, see the deep part); the beam crosses over in the drift and spreads out again, so at the exit it is actually wider, 5.0 mm.

**What it does not show**: this is a hard-edge idealisation. In the animation the radial field acts only at the two end faces of the coil (a thin "kick"), and the particles start turning abruptly there; in the real `sol` map the fringe field is spread over several centimetres and the particles start turning gradually. The noise of the map and the non-linear fields far from the axis are not in the animation. The particle motion comes from AVAS's linear model (with linear space charge), not from the engine's step-by-step tracking; the coil, the iron shell and the field lines are a schematic drawing, not field-map data.

### The example solenoid sol

The map `sol` is 0.35 m long; the on-axis $B_z$ is largest near the centre (after 9-point smoothing the peak is at $z \approx 0.177\ \mathrm{m}$, the centre at 0.175 m), its full width at half maximum is about 74 mm, and at both ends it has dropped to about 0.3 % of the peak. The smoothed peak is about $8.3K_b$ T. For the $K_b = 0.35$ of this chapter's beam line and the $K_b = 0.531$ of the manual's superposition example:

| Quantity | $K_b = 0.35$ | $K_b = 0.531$ | Note |
|---|---|---|---|
| Smoothed peak field | 2.90 T | 4.40 T | The raw data jump from point to point; the maximum is about 20 % above the smoothed peak |
| $\int B_z\,\dd z$ | 0.255 T·m | 0.387 T·m | Area under the curve, insensitive to noise |
| Larmor rotation angle $\theta_L$ | $41^\circ$ | $62^\circ$ | $\int B_z\,\dd z/(2B\rho)$, the angle the beam turns through as a whole |
| $\int B_z^2\,\dd z$ (smoothed) | 0.479 T²·m | 1.10 T²·m | About 3 % larger from the raw data |
| Thin-lens focal length $f$ | 0.27 m | 0.116 m | The latter is comparable to the length; the thin-lens value is only an order of magnitude |
| Equivalent hard-edge $B_{eff} = \int B^2/\int B$ | 1.88 T | 2.85 T | |
| Effective length $L_{eff} = (\int B)^2/\int B^2$ | 0.136 m | 0.136 m | Depends only on the field shape; much shorter than the 0.35 m map length |

The viewer below shows the map **data** itself. First look at the longitudinal slice of `bsz`: you will see vertical stripes along $z$. That is the map's own noise, not a reading error (see [Field maps](#field-maps-interpolation)). Then switch to the $x$–$y$ cross-section, turn on the arrows and move the slice to the entrance and to the exit: the transverse field points towards the axis everywhere at the entrance and away from it everywhere at the exit, and it is close to zero in the central cross-section, exactly as the flux argument above says.

```avas-field
map sol
```

How well do the numbers agree? The map's raw values at $x = 1\ \mathrm{cm}$, $y = 0$ (in map units, i.e. tesla for $K_b = 1$): on the entrance side at $z = 0.12\ \mathrm{m}$, $B_x \approx -0.55$, while $-\tfrac{r}{2}\,\Delta B_z/\Delta z$ from the smoothed on-axis field gives $-0.59$; on the exit side at $z = 0.23\ \mathrm{m}$, $B_x \approx +0.35$ against $+0.65$. Point by point only the sign and the order of magnitude agree, because a rate of change computed from noisy data is itself inaccurate; summed up they agree well (within 4 %, see the deep part).

### Quadrupole maps

Near the axis a magnetic quadrupole has $B_y = G\,x$, $B_x = G\,y$, $k = G/(B\rho)$; for $k > 0$ a positive particle is focused in $x$. Quadrupole maps are usually exported normalized to a reference gradient (e.g. 1 T/m at the centre), so that $K_b$ is the actual gradient (T/m). Unlike a solenoid, the quadrupole force is linear in $B$, so **the sign of $K_b$ sets the polarity**: for $K_b < 0$ the focusing and defocusing planes swap. With fringe fields, the effective length $L_{eff} = \int G(z)\,\dd z/G_0$ is commonly used for comparison with a hard-edge quadrupole (see [Quadrupole](#quadrupole-deep-field)).

### Bending and steering components

If a map contains an on-axis transverse magnetic field (a dipole component), the particles are deflected. The engine integrates through the map, so the deflection is included naturally; the linear preview does not include the deflection and warns when the estimated angle exceeds 1 mrad. A field map is a straight box along the $z$ axis, and a particle that leaves the axis gradually walks out of the region it covers, so maps are suited to small-angle bending and steering; for large-angle dipoles use `bend` and `edge` (see [Dipole](#dipole)), and steering coils can also be described with a [steerer](#steerer). How the engine defines the reference orbit for large deflections inside a map is not described in the manual.

## Field map or hard edge {#static-fields-equations}

### Replacing two areas with one box

To replace a "hill" like `sol` with a hard-edge box, there are two numbers to choose: the height $B_{eff}$ and the length $L_{eff}$. Which two things should be kept?

- **The rotation angle**: the angle the beam turns through depends on $\int B_z\,\dd z$, the area under the $B_z$ curve. For the box it is $B_{eff}L_{eff}$.
- **The focusing**: the thin-lens focal length depends on $\int B_z^2\,\dd z$, the area under the $B_z^2$ curve. For the box it is $B_{eff}^2L_{eff}$.

The two conditions give $B_{eff} = \int B^2/\int B$ and $L_{eff} = (\int B)^2/\int B^2$, the last two rows of the table above. The effective length of `sol` is only 0.136 m, much shorter than the map length of 0.35 m: the flanks of the hill are wide but weak, and after squaring they contribute even less.

Both areas match; why is there still a difference? Because they fully determine the beam only in the thin-lens limit. When the focal length is comparable to the element length, the beam size already changes inside the field, and how the field is distributed along $z$ starts to matter.

### How accurate is the hard-edge equivalent?

Compare two lattices in the example (example current 0.184 mA, linear preview with linear space charge, rms $x$): a 0.05 m drift + the map `sol` + a 0.3 m drift, against the equivalent hard-edge `solenoid` ($L = 0.136\ \mathrm{m}$) at the same centre position:

| | Map rms waist | Hard-edge rms waist | At $z = 0.7\ \mathrm{m}$, map / hard edge |
|---|---|---|---|
| $K_b = 0.35$ ($B_{eff} = 1.88\ \mathrm{T}$, $f/L_{eff} \approx 2$) | 0.811 mm @ 0.62 m | 0.828 mm @ 0.62 m | 0.845 / 0.857 mm |
| $K_b = 0.531$ ($B_{eff} = 2.85\ \mathrm{T}$, $f/L_{eff} \approx 0.85$) | 0.285 mm @ 0.388 m | 0.311 mm @ 0.402 m | 2.73 / 2.42 mm |

With weak focusing the two differ by about 2 %; with strong focusing they already differ by about 13 % at $z = 0.7\ \mathrm{m}$. Rule of thumb: when the focal length is several times the element length, the hard-edge equivalent is good enough for a first design; when they are comparable, accurate calculations should use the map.

The picture below is the **schematic** field of the equivalent hard-edge solenoid for $K_b = 0.531$ (computed from parameters, hard edge, zero outside the element, not map data); compare its longitudinal distribution with the map above:

```avas-field
solenoid B=2.85 L=0.136 R=0.02
```

In the solenoid lab with these equivalent parameters, look at the Larmor angle $kL$ (about $62^\circ$), the difference between the thick-lens (0.142 m) and thin-lens (0.116 m) focal lengths, and how the waist position moves with B. Then lower W to 0.05 MeV: the thin-lens focal length shrinks with $f \propto (B\rho)^2 \propto W$ by a factor of about 30 (0.116 m → 3.8 mm), $kL$ grows to about 6 rad and the beam crosses over several times inside the solenoid; the thin-lens approximation fails completely, and a low-energy section needs a much weaker solenoid.

```avas-lab
solenoid L=0.136 B=2.85 W=1.527 d1=0.05 d2=0.3
```

## Think about it {#static-fields-think}

> **Think about it 1**: an RFQ works between tens of keV and a few MeV. Why does it focus with an electric quadrupole field rather than magnetic quadrupoles?
>
> Answer: for the same reason. The electric force does not weaken as the velocity drops; at $\beta \sim 0.01$ the same force needs a magnetic field $1/\beta \sim 100$ times the electric one (comparing $E$ with $cB$ in SI units). See [RFQ](#journey-rfq).

> **Think about it 2**: the example map is noisy. Why is $\int B_z\,\dd z$ hardly affected while $\int B_z^2\,\dd z$ is systematically about 3 % too large?
>
> Answer: write the measured field as $B + n$, with $n$ a noise of zero mean. In the linear integral the positive and negative $n$ cancel; the squared integral gains a term $\int n^2\,\dd z$, which is always positive and does not cancel. So a solenoid focal length computed from a noisy map is too short.

> **Think about it 3**: an einzel lens has both focusing and defocusing regions. Why is the net effect always focusing?
>
> Answer: look at where the particle is slow. In the decelerating mode the middle is the focusing region and the particle is slowest there, feeling the force longest; in the accelerating mode the middle is the defocusing region but the particle is fastest there, feeling the force for the shortest time. The focusing regions always get the better deal. It is a second-order effect, so the focal length goes as $(\hat\Phi'/\hat\Phi)^2$ (see the deep part), whatever the sign of the voltage.

> **Think about it 4**: the hard-edge solenoid in the animation and the `sol` map both turn the beam through 41° and focus almost equally strongly. What is the point of using the map, then?
>
> Answer: the two are equivalent only in the thin-lens sense. When the beam changes noticeably inside the field (strong focusing, low energy), the shape of the field along $z$ starts to matter; in the table above they already differ by 13 % for $K_b = 0.531$. The map also contains non-linear fringe fields and manufacturing errors that the hard-edge model lacks.

## Common misconceptions {#static-fields-misconceptions}

- **The sign of Kb**: for a solenoid the sign of Kb only changes the sense of rotation, not the focal length; for a quadrupole it sets the polarity, and reversing it swaps the focusing and defocusing planes.
- **Map length is not effective length**: the example `sol` map is 0.35 m long, its equivalent hard-edge length only 0.136 m. A hard-edge model built from the map length and the peak field badly overestimates the focusing.
- **The raw maximum is not the peak field**: the raw maximum of `sol.bsz` is about 20 % above the 9-point smoothed peak; squared quantities ($\int B^2$) are raised by about 3 % by the noise, linear ones ($\int B$) are essentially unaffected; see [field-map noise](#field-maps-interpolation).
- **"Electrostatic elements do not change the energy"**: the kinetic energy is recovered only after leaving the element at the original potential; inside, kinetic energy, speed and focusing strength all change, and in a decelerating element a particle stops or is reflected if $q\Phi$ exceeds its kinetic energy.
- **Mixing electric and magnetic rigidity**: $E\rho \approx 2W/q$ is proportional to the kinetic energy, $B\rho = p/q$ to the momentum. When the energy is halved, the same electrostatic element becomes twice as strong, a magnetic one only about $\sqrt 2$ times.
- **"There is no radial field in a magnet"**: wherever the on-axis field changes along $z$ there is $B_r = -\tfrac{r}{2}B_z'$; the hard-edge model compresses it into kicks at the ends, which does not mean it is absent.
- **"The animation shows the result of the map"**: this chapter's solenoid animation is the equivalent hard-edge model, not the `sol` map; for the effect of the map, see the beam line at the top and the viewer.
- **Large-angle bending**: do not describe a large-angle dipole with a straight field-map box, see [bending components](#static-fields-magnetic).

## Going deeper: the paraxial equation of electrostatic lenses {#static-fields-deep-electric}

The main text says an einzel lens "focuses with either sign of voltage" and gives numbers for the focal length. This section derives it step by step, using only energy conservation, Newton's second law and the derivative (rate of change) of school mathematics; the last paragraph is for readers who know vector analysis. The whole section is non-relativistic, which is a very small error at the energies of this primer.

### Step 1: kinetic energy as the "height"

Let $\hat\Phi(z)$ be the on-axis potential measured from "zero kinetic energy", i.e. such that $q\hat\Phi = W$ everywhere. Then the kinetic energy at any point can be read straight off $\hat\Phi$. For a slow particle $W = \tfrac12 mv^2$, so

$$
v^2 = \frac{2q\hat\Phi}{m}.
$$

Primes denote the rate of change along $z$: $\hat\Phi' = \dd\hat\Phi/\dd z$, and $\hat\Phi''$ is the rate of change of the rate of change. $\hat\Phi$ differs from the ordinary potential $\Phi$ only by a minus sign and a constant, so the longitudinal field is $E_z = -\Phi' = +\hat\Phi'$: where $\hat\Phi$ grows, the field pushes forward and the particle speeds up.

### Step 2: the radial electric field near the axis

Exactly as in the flux argument of the main text: there is no charge in the vacuum, so the electric flux entering a small cylindrical box equals the flux leaving it. The change of $E_z$ across the end faces is made up by a radial field $E_r$ on the side:

$$
E_r = -\frac{r}{2}\,E_z' = -\frac{r}{2}\,\hat\Phi''.
$$

Where $\hat\Phi'' > 0$ (the kinetic-energy curve bends upwards, like a valley floor) $E_r$ points towards the axis: focusing. Where $\hat\Phi'' < 0$ (it bends downwards, like a hilltop) it points away: defocusing. An einzel lens has both: in the decelerating mode the kinetic-energy curve has a valley in the middle, in the accelerating mode a hilltop.

### Step 3: the force changes the "transverse momentum"

A particle at distance $r$ from the axis, with path slope $r' = \dd r/\dd z$, has transverse momentum $mv\,r'$. Crossing $\Delta z$ takes the time $\Delta z/v$, and the impulse $qE_r\,\Delta z/v$ changes the transverse momentum:

$$
\frac{\dd(mv\,r')}{\dd z} = \frac{qE_r}{v} = -\frac{q\,r\,\hat\Phi''}{2v}.
$$

The left side needs the product rule: $(v\,r')' = v'r' + v\,r''$. Multiply both sides by $v/m$ and use $v^2 = 2q\hat\Phi/m$ from step 1 together with $v v' = \tfrac12(v^2)' = q\hat\Phi'/m$, which follows from it:

$$
\frac{q}{m}\bigl(\hat\Phi'\,r' + 2\hat\Phi\,r''\bigr) = -\frac{q}{m}\cdot\frac{r\,\hat\Phi''}{2}.
$$

Dividing both sides by $2q\hat\Phi/m$ and rearranging gives the **paraxial ray equation**:

$$
r'' + \frac{\hat\Phi'}{2\hat\Phi}\,r' + \frac{\hat\Phi''}{4\hat\Phi}\,r = 0.
$$

The second term comes from the change of speed: when the particle speeds up, the same transverse momentum means a smaller slope, like a "damping". The third term is the action of the radial electric field.

### Step 4: a clever substitution

The second term makes the equation awkward. Set $R = r\,\hat\Phi^{1/4}$ (the Picht transformation), put $r = R\,\hat\Phi^{-1/4}$ back in and apply the product rule twice: the terms with $R'$ cancel exactly, leaving

$$
R'' + \frac{3}{16}\left(\frac{\hat\Phi'}{\hat\Phi}\right)^{2}R = 0.
$$

(The substitution takes half a page of algebra; it is not hard, and readers can check it themselves.) The coefficient is a square times a positive number, **always positive**. $R'' = -(\text{positive})\cdot R$ is a "spring": an einzel lens focuses whether the middle electrode is positive or negative. What is left after the focusing and defocusing regions of step 2 cancel is this term; it is the square of $\hat\Phi'$, a second-order quantity.

### Step 5: the focal length of a weak lens

If the lens is "thin", $R$ hardly changes while the particle passes, and the slope changes by $\Delta R' = -\tfrac{3}{16}R\int(\hat\Phi'/\hat\Phi)^2\,\dd z$. Before and after the lens $\hat\Phi$ is the same (equal kinetic energy at entrance and exit), so there $R$ and $r$ differ by the same constant factor, and with "deflection angle = distance from axis / focal length":

$$
\frac{1}{f} \approx \frac{3}{16}\int\left(\frac{\hat\Phi'}{\hat\Phi}\right)^{2}\dd z.
$$

Only $\hat\Phi'/\hat\Phi$ appears; it is unchanged when voltage and kinetic energy are doubled together, so the focal length depends only on the electrode shape and $qV/W$. Check the example of the main text: $\hat\Phi = W_0\bigl(1 - a\,\mathrm{e}^{-z^2/w^2}\bigr)$ with $w = 20\ \mathrm{mm}$; numerical integration gives $f = 0.166\ \mathrm{m}$ for $a = 0.5$ (kinetic energy halved at the centre) and $f = 0.536\ \mathrm{m}$ for $a = -0.5$ (raised to 1.5 times). When the focal length is comparable to the lens length, the weak-lens value is only an estimate.

### Step 6: the equation of motion in an electrostatic quadrupole

In an electrostatic quadrupole the particle energy is nearly constant, and $E_x = -G_Ex$ gives $x'' = qE_x/(mv^2) = -\bigl(G_E/(E\rho)\bigr)x$, i.e.

$$
x'' + k\,x = 0,\quad y'' - k\,y = 0,\qquad k = \frac{G_E}{E\rho}.
$$

It has exactly the form of the magnetic quadrupole equation, with the same transfer matrices (see [Quadrupole](#quadrupole-deep-matrix)). Example proton, $G_E = 8.9\times 10^7\ \mathrm{V/m^2}$: $k = 8.9\times 10^7/3.05\times 10^6 \approx 29\ \mathrm{m^{-2}}$; times $B\rho$ that is an equivalent magnetic gradient of 5.2 T/m, as in the main text; for a 50 keV proton $E\rho = 0.1\ \mathrm{MV}$ and $k \approx 890\ \mathrm{m^{-2}}$.

### For readers who know vector analysis

The electrostatic field is $\boldsymbol{E} = -\nabla\Phi$ with $\nabla^2\Phi = 0$ in vacuum. With axial symmetry the potential near the axis expands as $\Phi(r,z) = \Phi(z) - \tfrac{r^2}{4}\Phi''(z) + \tfrac{r^4}{64}\Phi''''(z) - \cdots$; the first order gives the $E_r$ of step 2. Relativistically the electric rigidity is $E\rho = pv/q = \beta^2\gamma\,mc^2/q$, and the coefficients of the paraxial ray equation acquire $\gamma$ corrections; the complete form is in Reiser, chapter 3. The potential of an ideal electrostatic quadrupole is $\Phi = \tfrac{V}{a^2}(x^2 - y^2)$.

## Going deeper: where solenoid focusing comes from {#static-fields-deep-magnetic}

The main text described solenoid focusing as "first set spinning, then pulled back". This section turns that into numbers, derives the Larmor angle and the focal length, and explains why the hard-edge equivalent is chosen as it is. It uses the impulse theorem, circular motion and simple harmonic motion; the last paragraph is for readers who know vector analysis.

### Step 1: how much angular velocity the entrance gives

The main text found the radial field at the entrance, $B_r = -\tfrac r2\,\Delta B_z/\Delta z$. A particle crossing a short piece $\Delta z$ with speed $v_z$ feels an angular force (around the axis) $qv_zB_r$ for a time $\Delta z/v_z$, an impulse

$$
\Delta p_\theta = qv_zB_r\cdot\frac{\Delta z}{v_z} = qB_r\,\Delta z = -\frac{qr}{2}\,\Delta B_z.
$$

The $v_z$ cancels. Adding up the pieces over the whole entrance fringe (with $r$ nearly constant while the particle crosses it), $\sum\Delta B_z$ is the rise of the field from 0 to $B$, so on leaving the entrance $p_\theta = -qrB/2$, whether the fringe field is long or short. Written as a slope, the angular velocity is

$$
\frac{v_\theta}{v_z} = -\frac{rB}{2B\rho} = -k\,r,\qquad k = \frac{B}{2B\rho}.
$$

$k$ is the strength parameter of the solenoid (in 1/m). Count it: for $B = 1.88\ \mathrm{T}$, $k = 1.88/(2\times 0.1786) = 5.26\ \mathrm{m^{-1}}$; a particle 1 mm off axis gets an angular slope of 5.3 mrad, of the same order as the rms angle of the example beam (about 3 mrad). The minus sign is the "pushed towards $-y$ at $+x$" of the main text.

### Step 2: the angle of rotation

Each particle turns about the axis with slope $k\,r$; the farther out, the faster it moves, so all particles turn through the same angle $k$ (in radians) per metre. The angle turned through in the whole solenoid is

$$
\theta_L = \int k\,\dd z = \frac{1}{2B\rho}\int B_z\,\dd z,
$$

called the **Larmor angle**. For `sol` at $K_b = 0.35$, $\int B_z\,\dd z = 0.255\ \mathrm{T\,m}$ and $\theta_L = 0.255/0.357 = 0.714\ \mathrm{rad} = 41^\circ$; for the equivalent hard-edge solenoid of the animation, $kL = 5.26\times 0.136 = 0.716\ \mathrm{rad}$, also 41°.

### Step 3: pulled towards the axis

The particle now has the angular velocity $v_\theta = k\,r\,v_z$, perpendicular to $B_z$, and the Lorentz force $qv_\theta B_z$ points towards the axis. But the particle is also moving on a circle around the axis, and keeping up that circular motion by itself requires a centripetal force $mv_\theta^2/r$ (school circular motion). Only the excess pulls the particle closer to the axis:

$$
F_{\text{net}} = qv_\theta B - \frac{mv_\theta^2}{r}.
$$

Insert $qB = 2k\,p$, $v_\theta = k\,r\,v$ and $p = mv$: the first term is $2k^2r\,pv$, the second $k^2r\,pv$, and the net force $k^2r\,pv$ points towards the axis. Converting the impulse into the bending of the path, $r'' = -F_{\text{net}}/(pv)$:

$$
r'' = -k^2\,r = -\left(\frac{B}{2B\rho}\right)^{2} r.
$$

This is the simple-harmonic equation again (see [Quadrupole](#quadrupole-deep-matrix)), and it is the same for $x$ and $y$: both planes are focused at once. $B$ appears squared in $k^2$: **reverse the field and the focusing stays the same**.

### Step 4: the focal length

For a thin lens $r$ hardly changes inside the solenoid, the slope changes by $\Delta r' = -r\int k^2\,\dd z$, and

$$
\frac{1}{f} \approx \int\left(\frac{B_z}{2B\rho}\right)^{2}\dd z.
$$

For `sol` at $K_b = 0.35$, $\int B_z^2\,\dd z = 0.479\ \mathrm{T^2\,m}$ and $f = (2\times 0.1786)^2/0.479 = 0.27\ \mathrm{m}$. Doubling B makes the thin-lens focal length a quarter.

A hard-edge solenoid can be solved exactly (turn through $kL$ of simple harmonic motion, then compute the exit slope), and its focal length is

$$
f = \frac{1}{k\sin kL}.
$$

For the animation's defaults $B = 1.88\ \mathrm{T}$, $L = 0.136\ \mathrm{m}$: $kL = 0.716$, $\sin kL = 0.656$, $f = 1/(5.26\times 0.656) = 0.290\ \mathrm{m}$, a little longer than the thin-lens 0.27 m. Doubling B to 3.76 T gives $kL = 1.43$ and $f = 0.096\ \mathrm{m}$, only a third, because $\sin kL$ is no longer close to $kL$.

### Step 5: checking the radial field with the sol map

Turn the piece-by-piece argument of step 1 round and use it to test the map. For `sol` at $x = 1\ \mathrm{cm}$, at the steepest point $z = 0.1435\ \mathrm{m}$, the map gives $B_x = -0.99$, while $-\tfrac r2\,\Delta B_z/\Delta z$ from the 9-point smoothed on-axis field gives $-1.44$ (map units, $K_b = 1$). Point by point the difference is large, because a rate of change computed from noisy data amplifies the noise. Sum up instead: the paraxial gradient $\partial B_x/\partial x$ (i.e. $B_x/x$) integrated from the entrance to the peak is $-3.98\ \mathrm{T}$, while step 1 says it should equal $-\tfrac12 B_{z,\text{peak}} = -4.14\ \mathrm{T}$: 4 % apart. The sum is exactly the sum of step 1 for which "the length of the fringe does not matter", and in a sum the noise cancels.

### Step 6: why the equivalent hard edge is chosen that way

Step 2 says the rotation angle depends on $\int B$, step 4 that the thin-lens focal length depends on $\int B^2$. A hard-edge box (height $B_{eff}$, length $L_{eff}$) must satisfy both $B_{eff}L_{eff} = \int B$ and $B_{eff}^2L_{eff} = \int B^2$; dividing gives $B_{eff} = \int B^2/\int B$, and substituting back $L_{eff} = (\int B)^2/\int B^2$. For `sol`: $B_{eff} = 0.479/0.255 = 1.88\ \mathrm{T}$, $L_{eff} = 0.255^2/0.479 = 0.136\ \mathrm{m}$. Step 4 already showed that the same $\int B^2$ gives different thick-lens focal lengths under strong focusing; that is where the 13 % difference in the table of the main text comes from.

### For readers who know vector analysis

In vacuum $\nabla\cdot\boldsymbol{B} = 0$ and $\nabla\times\boldsymbol{B} = 0$, so $\boldsymbol{B} = -\nabla\psi$ with $\nabla^2\psi = 0$. An axially symmetric field near the axis is completely determined by the on-axis $B_z(z)$:

$$
B_r(r,z) = -\frac{r}{2}\,B_z'(z) + \frac{r^3}{16}\,B_z'''(z) - \cdots,\qquad
B_z(r,z) = B_z(z) - \frac{r^2}{4}\,B_z''(z) + \cdots
$$

Step 1 is the first-order term of $\nabla\cdot\boldsymbol{B} = 0$; step 3 holds for particles in a meridional plane, and in general Busch's theorem (conservation of the canonical angular momentum $\gamma mr^2\dot\theta + \tfrac{q}{2}r^2B_z$) shows that in a frame rotating at the Larmor frequency the two transverse planes decouple and obey $r'' + k(z)^2r = 0$. The 4×4 matrix of a hard-edge solenoid is built from $C = \cos kL$, $S = \sin kL$ and is the product of a rotation and a focusing matrix, see [Solenoid](#solenoid). Linearizing the field near the axis, the transverse equation with both electrostatic and magnetostatic fields is

$$
x'' + \frac{(\beta\gamma)'}{\beta\gamma}\,x' + \left(\frac{1}{B\rho}\frac{\partial B_y}{\partial x} - \frac{1}{E\rho}\frac{\partial E_x}{\partial x}\right)x = 0
$$

(the $x$–$y$ coupling of a solenoid is not written out; with an axial magnetic field use the rotating frame or 4×4 matrices). The second term comes from the change of momentum in an electrostatic field. If the momentum change inside the element is negligible, replacing the field by a hard-edge element with effective gradient $G_{eff}$ and effective length $L_{eff}$ brings back the matrix model: a quadrupole uses the thick-lens matrix of the [Quadrupole](#quadrupole) chapter, a solenoid the coupled matrix with $k = B_{eff}/(2B\rho)$, $C = \cos kL$, $S = \sin kL$.

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

### How the engine and the linear preview compute

In a field-map element the engine integrates the Lorentz equation for every particle in time steps (t-code):

$$
\frac{\dd(\gamma m\boldsymbol{v})}{\dd t} = q\,\bigl(\boldsymbol{E}(\boldsymbol{r}) + \boldsymbol{v}\times\boldsymbol{B}(\boldsymbol{r})\bigr),\qquad
\boldsymbol{E} = K_e\,\boldsymbol{E}_{map},\quad \boldsymbol{B} = K_b\,\boldsymbol{B}_{map}
$$

The time step is set by `steppercycle` in input.txt. A magnetostatic field does no work and the kinetic energy stays constant; an electrostatic field changes it according to energy conservation.

> **AVAS convention**: the linear preview cuts a map into 4 mm slices and treats each as "edge + half-step solenoid body + thin lens from the transverse gradients near the axis + half-step body + edge". The radial fringe field of a solenoid is derived from the superposed on-axis $B_z$ by Maxwell's equations ($\partial B_x/\partial x + \partial B_y/\partial y = -\partial B_z/\partial z$) and placed at both ends of each slice, which keeps the transfer matrix exactly symplectic; the map's $B_x$ and $B_y$ provide only the divergence-free part (quadrupole and skew-quadrupole components), because in a noisy map these two components are not consistent with $B_z$. The engine integrates directly in the 3D field; both include fringe fields, and the engine also includes non-linear terms.

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
