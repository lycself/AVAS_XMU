# Solenoid

School physics introduces the current-carrying solenoid: wind a wire turn after turn into a long tube, pass a current, and inside the tube there is an almost uniform magnetic field along the axis. An accelerator **solenoid** is just such a large coil, and the beam passes right through its middle.

There is something odd here. The particles fly almost along the axis, and the field also points along the axis; and school physics says that when velocity and field are parallel, the Lorentz force is zero. How can a field that exerts no force gather the beam in? Stranger still, a [quadrupole](#quadrupole) always focuses one direction and defocuses the other, yet a solenoid **focuses both directions at once**, and it keeps focusing when the current in the coil is reversed. This chapter solves both puzzles.

First, the shape of the answer: a solenoid does two things. At the two ends of the coil the field is not exactly along the axis but a little "tilted", and this tilt first sets the particles turning about the axis; a turning particle moves sideways, and now the field along the axis can push it, and the push points exactly towards the centre. It is a bit like wringing a towel: as you twist it, it gets thinner.

In the animation the beam passes a short drift, then a 0.2 m long, 1.5 T solenoid, then a 0.5 m drift.

```avas-motion
solenoid
```

First look at the two side views in 2D: seen from above (horizontal direction $x$) and from the side (vertical direction $y$), the beam is gathered in **both**, it is narrowest about 0.2 m behind the solenoid ($z \approx 0.47$ m) with an rms size of about 0.99 mm, and the two pictures are almost identical. Compare this with the quadrupole's "one closes, one opens" picture. Now look at the cross-section (seen from downstream, the beam flies out of the screen towards you, and $\odot$ marks that the solenoid's field points towards you as well): the blue particle starting at $+x$ turns **clockwise** towards $-y$ as soon as it enters the solenoid, moving closer to the centre as it turns; by the exit it has turned through about 48°, after which it stops turning and heads straight for the centre, crossing it near the end of the line ($z \approx 0.72$ m). The other blue particle, starting at $+y$, also turns clockwise through the same 48°. Switch to 3D to see the cut-open copper coil (with its winding lines) inside the iron shell and end plates: the field lines run along the axis inside the coil and return through the iron.

## Why solenoids are used {#solenoid-role}

The strength of a solenoid is that **one element focuses both directions equally**. A round beam stays round after it (this is called "axially symmetric" focusing). At very low energy the particles repel each other strongly (space charge), and this symmetric focusing makes it easy for the beam to keep a stable round cross-section, without the pairs of alternating quadrupoles a quadrupole channel needs.

It has three costs: the beam rotates about the axis; the horizontal and vertical motions get mixed together ("coupling"); and as the energy rises, the field it needs soon becomes too large to build (see below). Solenoids are therefore found mainly in:

- **Low-energy transport and matching**: in the low-energy beam transport (LEBT) after the ion source, 1–3 solenoids converge the diverging beam and deliver it into the RFQ in the shape the RFQ needs (see [Ion source and LEBT](#journey-source)).
- **The low-energy part of superconducting linacs**: in the low-velocity section built from half-wave (HWR), quarter-wave (QWR) and spoke cavities, periods alternating "cavity, solenoid, cavity, solenoid, …" are common. A superconducting solenoid fits into the same low-temperature vessel (cryostat) as the cavities, which keeps the structure compact; it focuses both directions at once, so one period needs only one focusing element (see [The superconducting section](#journey-sc); the AVAS example project is the start of such a period and includes the solenoid field map `sol`).
- **Steering**: superconducting solenoids often carry built-in dipole steering coils (see [Steerer](#steerer)).

## The field inside the coil {#solenoid-field}

### In the middle: a uniform field along the axis

In the middle of the coil the field points along the axis and is almost the same everywhere. Its direction follows the right-hand grip rule: curl the fingers of your right hand along the current in the turns, and the thumb points along the field inside the coil. In the middle of a long solenoid the field is

$$
B = \mu_0\, n\, I ,
$$

where $n$ is the number of turns per metre, $I$ the current and $\mu_0 = 4\pi\times 10^{-7}$ T·m/A. The animation's 1.5 T needs $nI \approx 1.2\times 10^6$ A/m: with 1000 turns per metre, each turn carries about 1200 A. Such currents produce a lot of heat in ordinary copper wire, which is why strong solenoids often use superconducting wire. The coil is usually wrapped in iron (a shell with end plates at both ends) so that the field lines return through the iron and there is almost no field outside the solenoid.

In AVAS a positive solenoid field points along the beam direction ($+z$). Every cross-section is **seen from downstream**: the beam flies out of the screen towards you, $x$ to the right, $y$ up; the field of a solenoid with $B > 0$ also points towards you and is drawn as $\odot$.

### At the ends: the field lines have to bend

Field lines have one property: they do not appear from nothing and do not vanish (there is no isolated N or S pole). Think of the field lines through the solenoid as a bundle of threads: far away the field is weak and the threads are sparse; inside the coil the field is stronger and the threads must be packed more densely. Since no new threads can appear, the only way to become denser is to **converge towards the axis**. So at the entrance, where the field grows, it leans slightly towards the axis and has a component pointing towards the axis (the **radial component** $B_r$; "radial" means along the line from the axis outwards). At the exit, where the field weakens, the lines spread out again and the radial component points outwards.

How large is this radial component? The "going deeper" part proves with magnetic flux that

$$
B_r \approx -\frac{r}{2}\times(\text{how much the field on the axis changes per metre}),
$$

where $r$ is the distance from the axis. It is proportional to the distance from the axis; it is zero in the middle, where the field does not change, and exists only at the ends. The minus sign means that it points towards the axis where the field grows. In a real solenoid the field changes gradually over about one coil radius at each end, and at the coil end it is about half the central value.

### The hard-edge model and the real field map

Linear calculations often use a simplified **hard-edge model**: the field is the same everywhere inside the solenoid and drops to zero abruptly at the ends. The radial field is then squeezed into a thin layer at the ends, and inside the solenoid **the transverse field is zero everywhere**. Below is the cross-section of a hard-edge solenoid (a **schematic** computed from the parameters, not measured data): the $x$–$y$ cross-section shows only a uniform $B_z$ pointing towards you, with no arrows; the longitudinal cut shows the field cut off sharply at both ends.

```avas-field
solenoid B=1 L=0.2 R=0.02
```

> **AVAS convention**: the cross-section schematic of a matrix-model solenoid has zero transverse field and draws no grid arrows — this is a property of the hard-edge model itself, not missing data. A field-map solenoid (such as the example `sol`) has a real fringe field: from $B_r = -\tfrac{r}{2}B'$, for $B_z>0$ the $(B_x, B_y)$ arrows point towards the axis everywhere at the entrance, where the field rises, outwards at the exit, where it falls, and the transverse field is close to zero at the centre (checked with the example map: at $x = +1$ cm, $B_x$ is about $-0.55$ at $z = 0.12$ m and about $+0.35$ at $z = 0.23$ m, in map units, multiplied by `Kb` to give T). The direction always agrees with the formula; the magnitude only roughly, see the "going deeper" part and [In AVAS](#solenoid-in-avas).

Now a slice of the example field map `sol`, which is **data**. Its on-axis field has no flat top: the smoothed peak of about 8.3 (map units) lies at $z \approx 0.177$ m, the full width at half maximum is only about 7.4 cm, and most of the 0.35 m map length is a slowly decaying fringe. The field rises most steeply at $z \approx 0.144$ m and falls most steeply at $z \approx 0.200$ m. Move the cut to the entrance side (e.g. $z = 0.12$ m) and the $(B_x, B_y)$ arrows all point towards the axis; move it to the exit side (e.g. $z = 0.23$ m) and they point outwards; at the centre the transverse field is close to zero. The vertical stripes along $z$ in the heat map come from the grid noise of the map itself; they are data, not a display error (point values at the end of this chapter, [In AVAS](#solenoid-in-avas)).

```avas-field
map sol
```

## Focusing in two steps: set turning, then pulled back {#solenoid-busch}

Take a positively charged particle, a solenoid with $B > 0$, and let the particle enter parallel to the axis at a distance $r$ from it. We use three school tools: the Lorentz force, impulse, and the circular motion of a charged particle in a uniform magnetic field.

### Step 1: the tilted field at the entrance sets the particle turning

At the entrance the field has a radial component $B_r$ pointing towards the axis. The particle flies along $z$, perpendicular to $B_r$, so it feels a Lorentz force $F = q v B_r$. The force is perpendicular both to the velocity and to $B_r$, that is, along the tangent of a circle about the axis (the **azimuthal direction**, the direction of "turning about the axis"). In the AVAS view (from downstream), for $B > 0$ a particle at $+x$ is pushed towards $-y$: the particle starts turning **clockwise** about the axis.

How big is the push? Use impulse. Crossing a short stretch $\Delta z$ of the entrance takes $\Delta t = \Delta z / v$, and the transverse momentum gained is

$$
F\,\Delta t = q\,v\,B_r\,\frac{\Delta z}{v} = q\,B_r\,\Delta z .
$$

The velocity $v$ cancels. Now add up the whole entrance region piece by piece: $B_r$ equals $\tfrac{r}{2}$ times "how much the field changes per metre", so times $\Delta z$ it is $\tfrac{r}{2}$ times the increase of the field over that short stretch; all the increases add up to the total change of the field from 0 to $B$. So the entrance gives the particle a transverse momentum $q\,r B/2$ in all, and dividing the transverse momentum by the forward momentum $p$ gives the angle of the "turning about the axis" motion:

$$
\frac{q\,r\,B}{2\,p} = \frac{B}{2B\rho}\;r = k\,r ,\qquad k = \frac{B}{2B\rho}.
$$

$B\rho = p/q$ is the **magnetic rigidity** introduced in the [quadrupole](#quadrupole-lorentz) chapter (how hard the particle is to deflect with a magnetic field, see [Kinematics](#kinematics)); for the example proton $B\rho = 0.179$ T·m. The animation's default $B = 1.5$ T gives $k = 4.20$ m⁻¹: a particle 1 mm off axis gains a transverse angle of 4.2 mrad, more than the typical 3 mrad angle of the example beam.

Note that this angle is **proportional to $r$**: twice as far from the axis means twice the transverse speed, and all particles turn about the axis equally fast. The whole beam turns as one, like a record.

### Step 2: a particle moving sideways draws a circle in the field

Inside the solenoid the particle now has a transverse velocity, and this transverse velocity is perpendicular to the field $B$ along the axis. School physics says that a charged particle moving perpendicular to a uniform magnetic field moves on a circle of radius

$$
R = \frac{m v_\perp}{q B} = \frac{p_\perp}{q B},
$$

where $p_\perp$ is the transverse momentum. (The particle also keeps flying forward along $z$, so its real path is a helix; seen from downstream, its projection onto the cross-section is a circle.) Insert $p_\perp = q\,r B/2$ from step 1:

$$
R = \frac{q\,r\,B/2}{q\,B} = \frac{r}{2}.
$$

The radius of the circle is exactly half the distance from the axis; in other words **the diameter of the circle equals $r$**. Now where is its centre? When the particle has just entered, its transverse velocity is along the tangent, perpendicular to the line from the axis to the particle, so the centre lies on that line, $r/2$ from the particle: exactly halfway between the particle and the axis. So the far side of this small circle **passes through the axis**! In the cross-section the particle runs along this small circle, round towards the axis.

Two theorems of school geometry tell us more. When the particle has travelled through a central angle $\phi$ on the small circle:

- **The inscribed angle is half the central angle**: seen from the axis, the particle has turned through $\phi/2$.
- **The angle in a semicircle is a right angle**: the particle, the axis and the particle's starting point form a right triangle, so the particle's distance from the axis is $r\cos(\phi/2)$.

How fast does the particle go round the small circle? One revolution in a uniform field (the cyclotron period) takes $T = 2\pi m/(qB)$. At 1.5 T, $T = 43.8$ ns, while a proton at $1.71\times 10^7$ m/s crosses the 0.2 m solenoid in only 11.7 ns, covering $\phi = 96^\circ$ of the small circle. So, seen from the axis, the particle turns through $48^\circ$ and its distance from the axis shrinks to $\cos 48^\circ = 0.67$ of the starting value. These are the 48° the blue particle turns through in the animation.

### Step 3: the turning stops at the exit and the particle heads for the axis

At the exit the field weakens, its radial component points outwards, and the particle gets a push opposite to the one at the entrance. Its size is again $k$ times the particle's present distance from the axis, and it exactly cancels the "turning about the axis" part of the velocity (this holds exactly and is called Busch's theorem, see "going deeper").

So after the solenoid the particle stops turning and keeps only its velocity towards the axis: seen from downstream it heads straight for the axis. With the default parameters a particle entering parallel 1 mm off axis is 0.67 mm off axis at the exit, heads for the axis at an angle of 3.1 mrad, and crosses the axis about 0.21 m further on. All particles that enter parallel head for the axis in the same proportion: this is exactly a lens, plus a rotation of the whole beam.

### Why it always focuses, and in both directions

- **Direction**: the small circle of step 2 passes through the axis whether $B$ is positive or negative. Reverse $B$ and the particle turns the other way (anticlockwise), but it still runs round towards the axis. So a solenoid **always focuses**; the sign only decides the sense of rotation. The same holds for negatively charged particles.
- **Both directions**: the whole process is symmetric about the axis and favours neither $x$ nor $y$, so horizontal and vertical are focused equally.
- **Strength**: the push at the entrance is proportional to $B$, and the pull back once the particle is turning is again proportional to $B$; together they are proportional to $B^2$. Double the field and the focusing is four times stronger.

## Focal length and rotation angle: try it in the animation {#solenoid-focal}

Written as formulas (derived in the "going deeper" part), the angle through which the beam turns is called the **Larmor angle**:

$$
\theta_L = k\,L = \frac{B\,L}{2B\rho},
$$

and the focal length is

$$
\frac{1}{f} = k\sin(kL) \approx k^2 L = \left(\frac{B}{2B\rho}\right)^2 L ,
$$

where the approximation holds when $kL$ is small ("thin lens"). With the animation's defaults ($B = 1.5$ T, $L = 0.2$ m): $k = 4.20$ m⁻¹, $\theta_L = 48.1^\circ$, $f = 0.320$ m (thin-lens estimate 0.284 m). The table of values under the animation lists these quantities.

Drag the parameters to check the claims of the text:

- **What happens without the solenoid**: set $B$ to 0 and the beam grows to rms 2.94 mm at the exit; at the default 1.5 T it is only 1.21 mm, almost the same in $x$ and $y$.
- **Reversed, it still focuses**: set $B$ to $-1.5$ T; the particles now turn anticlockwise, the angle becomes $-48.1^\circ$, and the focal length and exit size do not change at all.
- **Focusing goes as $B^2$**: halve $B$ to 0.75 T and the focal length becomes 1.17 m, 3.7 times the original (the thin-lens estimate gives exactly 4).
- **Too strong "overshoots"**: raise $B$ to 3 T; the rotation is 96° and the focal length only 0.12 m; the beam is squeezed to 0.28 mm near the solenoid exit and then spreads violently, reaching 4.4 mm at the end, wider than with no solenoid at all.
- **The "stiffer" the particle, the harder to focus**: set the kinetic energy to 10 MeV; the rigidity becomes 0.458 T·m, the rotation only 18.8° and the focal length 1.90 m, about 6 times weaker.
- **Space charge**: raise the current from 0.184 mA to 10 mA; the particles push each other apart, there is no longer a waist behind the solenoid, and the size at the end is 2.43 mm.

The formulas tell one more thing: the rotation depends on $BL$, the focal length on $B^2 L$. Double the field and halve the length: the rotation stays the same, but the focal length halves. The focal length is much more sensitive to the field than to the length.

Below is a 0.2 m, 2 T hard-edge solenoid as it appears in the lattice editor: it focuses $x$ and $y$ at once, and the beam forms a waist of rms 0.50 mm at $z \approx 0.46$ m; the rms $x$ and rms $y$ curves almost coincide (the initial parameters of the example beam differ slightly between the two directions). Compare it with the picture at the start of the [quadrupole](#quadrupole) chapter, where one direction closes and the other opens.

```avas-beamline
drift 0.1 0.02 0
solenoid 0.2 0.02 0 2
drift 0.5 0.02 0
```

## Why solenoids are mostly used at low energy {#solenoid-energy}

Compare the strength of the two lenses: a single quadrupole has $1/f = GL/B\rho$, a solenoid $1/f = (B/2B\rho)^2 L$. Halve the particle's rigidity and the quadrupole gets twice as strong, but the solenoid four times. So the lower the energy, the better value a solenoid is.

The same thing seen another way: for a given focal length, the field each needs is proportional to $B\rho$. The table gives the solenoid field and quadrupole gradient a proton needs for a focal length $f = 0.3$ m with length $L = 0.2$ m (thin-lens estimates; the quadrupole focuses one direction only):

| Proton kinetic energy | $B\rho$ | Solenoid $B$ | Quadrupole $G$ | Quadrupole pole-tip field ($R = 2$ cm) |
|---|---|---|---|---|
| 35 keV (LEBT) | 0.0270 T·m | 0.22 T | 0.45 T/m | 0.009 T |
| 1.527 MeV (example) | 0.1786 T·m | 1.46 T | 2.98 T/m | 0.06 T |
| 10 MeV | 0.458 T·m | 3.7 T | 7.6 T/m | 0.15 T |
| 100 MeV | 1.483 T·m | 12.1 T | 24.7 T/m | 0.49 T |

In every row the solenoid needs about 25 times the quadrupole's pole-tip field (the field at the pole surface). Divide the two formulas and $B\rho$ drops out: $B_{\text{sol}}/(GR) = 2\sqrt{fL}/R$, here $2\sqrt{0.3 \times 0.2}/0.02 = 24.5$. This factor does not depend on the energy, but it does depend on the geometry: it grows with the focal length and shrinks with the aperture. The reason is that a solenoid can only push particles through the weak two-step "turn, then pull" mechanism, whereas a quadrupole pushes them directly with a transverse field that grows linearly from the axis to the pole. Comparing with a single quadrupole even favours the quadrupole, because it focuses only one direction; for a doublet of two quadrupoles (see "Think about it" below) the factor in that example is 17, and for the 0.8 m period compared in [Lattice design](#lattice-design) about 9.

Why use solenoids at low energy at all, then? Because from tens of keV to a few MeV the required 0.2–2 T can be built — with room-temperature solenoids at the low end and superconducting ones at the high end — and in exchange both directions are focused symmetrically; at 100 MeV a dozen tesla is no longer practical, while a quadrupole's pole-tip field is still below 0.5 T.

> **Think about it**: quadrupoles need to be arranged as a FODO to focus both directions, and the net focusing of a FODO is also "second order" ($1/F = d/f^2$). Is the comparison fair?
> Answer: yes, and the conclusion stands. Two thin lenses $\pm f$ a distance $d$ apart give a net focusing $1/F = (G L_q/B\rho)^2 d$, like a solenoid $\propto 1/(B\rho)^2$; equal focusing requires $B = 2G\sqrt{L_q d}$ (taking the solenoid as long as one quadrupole, $L_q$). A doublet with $G = 6$ T/m, $L_q = 0.1$ m, $d = 0.3$ m (pole-tip field 0.12 T at $R = 2$ cm) is equivalent to a 0.1 m, 2.1 T solenoid, 17 times the pole-tip field. The quadrupoles gain enormous "leverage" from the transverse gradient, at the price of envelopes that alternate between the two directions and a beam that is not round.

## Think about it {#solenoid-think}

> **Think about it 1**: the same $B > 0$ solenoid, but negatively charged H⁻ ions come through. What happens?
>
> Answer: both directions are still focused; the ions just turn anticlockwise. The focusing goes as $B^2$ and does not depend on the sign of the charge or of the field; the sense of rotation follows the sign of $qB$.

> **Think about it 2**: two identical solenoids one after the other, the second with reversed current. What happens to the beam?
>
> Answer: the two rotations, one positive and one negative, cancel, and the beam leaves without rotation; the two focusing effects both point inwards and add. Such "opposite pairs" are commonly used to cancel the total rotation.

> **Think about it 3**: why does the whole beam turn equally fast, like a record, rather than the outer part slower and the inner part faster?
>
> Answer: the transverse speed given at the entrance is proportional to the distance from the axis; further out, the transverse speed is larger, but the way round is longer too, and the two cancel. Or from step 2: how fast each particle goes round its small circle depends only on $B$, not on $r$.

## Common misconceptions {#solenoid-misconceptions}

- **"The sign of `B` decides between focusing and defocusing"**: a solenoid always focuses; reversing `B` only reverses the rotation.
- **"The longitudinal field exerts no force, so the focusing comes from the fringe field itself"**: the fringe field only gives a push about the axis; it does not push the particle towards the axis directly. The force towards the axis comes from the longitudinal field inside the solenoid acting on the turning particle. In the hard-edge model there is no transverse field inside, and the focusing is still there.
- **"The exit fringe field cancels the focusing"**: what the exit cancels is the rotation (the velocity about the axis), not the velocity towards the axis gained inside.
- **"The emittance bump inside a solenoid means the beam got worse"**: for a round beam it is a projection effect of the rotation and disappears after the solenoid (see "going deeper").
- **"Rotation and focal length are set by the same quantity"**: the rotation goes with $\int B\,\dd s$, the focal length with $\int B^2\,\dd s$.
- **"Solenoids save field at low energy"**: for the same focal length and aperture, a solenoid needs several to tens of times the quadrupole's pole-tip field (the factor $2\sqrt{fL}/R$ is set by the geometry, independent of energy); solenoids are used at low energy because the field required there can be built, and they buy symmetric focusing in both directions.

## Going deeper: where the fringe field comes from {#solenoid-deep-field}

The main text said that "at the entrance the field lines converge towards the axis, with a radial field $B_r = -\tfrac{r}{2}\times$ (how much the field changes per metre)". This section derives it step by step with the school notion of magnetic flux, and then shows the on-axis field of a real solenoid and how the hard-edge model replaces it. The last paragraph is for readers who know vector analysis.

### Step 1: the magnetic flux through a small cylindrical box

That field lines neither appear nor vanish can be said with the school notion of **magnetic flux** (field times area, $\Phi = BS$): for any closed box, the flux going in equals the flux coming out.

Put a very short small cylinder on the axis, radius $r$, length $\Delta z$, with its two end faces perpendicular to the axis.

- Front face (upstream): the field $B(z)$ comes in; flux $\pi r^2 B(z)$.
- Back face (downstream): the field $B(z + \Delta z)$ goes out; flux $\pi r^2 B(z + \Delta z)$.
- Side: area $2\pi r\,\Delta z$; the radial component $B_r$ goes out here ($B_r > 0$ means outwards); flux $2\pi r\,\Delta z\,B_r$.

Out equals in:

$$
\pi r^2 B(z+\Delta z) + 2\pi r\,\Delta z\,B_r = \pi r^2 B(z)
\quad\Longrightarrow\quad
B_r = -\frac{r}{2}\,\frac{B(z+\Delta z) - B(z)}{\Delta z}.
$$

The fraction on the right is "how much the on-axis field changes per metre", written $B'$ ("$B$ prime"). So $B_r = -\tfrac{r}{2}B'$: where the field grows along $z$ ($B' > 0$), $B_r < 0$ points towards the axis; where it weakens, outwards; where the field does not change, $B_r = 0$. (We assumed the cylinder is thin, so the field on its end faces can be taken equal to the on-axis value everywhere.)

### Step 2: a check with the example field map

At $z = 0.12$ m in the example map `sol` (entrance side), inserting the smoothed slope of the on-axis field into the formula gives $-\tfrac{r}{2}B' \approx -0.59$ (map units) at $x = 1$ cm; the map's actual $B_x$ is $-0.55$, same direction and close in size; on the exit side, at $z = 0.23$ m, the sign is reversed. Where the entrance is steepest, the map's transverse field is noticeably smaller than the formula ($-0.99$ against $-1.44$ at $z = 0.1435$ m). This map is noisy, and its transverse components do not agree point by point with $B_z$; see [In AVAS](#solenoid-in-avas).

### Step 3: the on-axis field of a real solenoid

For a solenoid of radius $a$, length $\ell$, $n$ turns per metre and current $I$, adding up the on-axis fields of all the turns (Biot–Savart law) gives the on-axis field ($z$ measured from the coil centre):

$$
B(z) = \frac{\mu_0 n I}{2}\left[\frac{z + \ell/2}{\sqrt{(z+\ell/2)^2 + a^2}} - \frac{z - \ell/2}{\sqrt{(z-\ell/2)^2 + a^2}}\right].
$$

Check two places. At the centre $z = 0$ of a long coil ($\ell \gg a$), the two terms in the bracket are about 1 and $-1$, and $B \to \mu_0 n I$, the formula of the main text. At the coil end $z = \ell/2$, the second term is 0 and the first about 1, so $B \approx \tfrac12\mu_0 n I$: about half the central value. The formula also shows that the field falls from its central value over a distance of about one coil radius $a$: that is the width of the fringe region.

### Step 4: the hard-edge model

Replace $B(z)$ by a rectangle of length $L$ and height $B$ (hard edge). The on-axis field now jumps only at the ends, where $B'$ is "infinite" over zero width, but step 1 shows that adding up $B_r$ along $z$ gives a result that depends only on the total change of the field:

$$
\int B_r\,\dd z = -\frac{r}{2}\,B \quad(\text{entrance}),
$$

with the opposite sign at the exit. ($\int \ldots\,\dd z$ reads "integral along $z$" and simply means multiplying the value of each short piece by its length and adding them all up.) So the hard-edge model keeps the "push" at the entrance and at the exit; inside the solenoid there is no transverse field anywhere, and the focusing comes entirely from the combination "radial field at the ends × longitudinal field inside".

How is a real solenoid replaced by a hard-edge model? The main text said the rotation follows $\int B\,\dd z$ and the focal length $\int B^2\,\dd z$. Make both integrals of the hard-edge model equal to those of the real field:

$$
B_{\text{hard edge}} = \frac{\int B^2\,\dd z}{\int B\,\dd z},\qquad
L_{\text{hard edge}} = \frac{\left(\int B\,\dd z\right)^2}{\int B^2\,\dd z}.
$$

(Check: $B_{\text{hard edge}} L_{\text{hard edge}} = \int B\,\dd z$ and $B_{\text{hard edge}}^2 L_{\text{hard edge}} = \int B^2\,\dd z$.) For the example map `sol` with `Kb = 0.35` (smoothed peak about 2.9 T, the default of the [tutorial](#tutorial)), $\int B\,\dd z \approx 0.729 \times 0.35 = 0.255$ T·m and $\int B^2\,\dd z \approx 4.04 \times 0.35^2 = 0.495$ T²·m, so $B = 1.95$ T and $L = 0.13$ m.

The two beam lines below compare the map `sol` with this hard-edge solenoid **of equal integrals** (centred on the map's peak). Both form a waist at $z \approx 0.6$ m with rms about 0.8 mm (map: $x$ waist 0.81 mm, $y$ waist 0.75 mm; hard edge: 0.79 mm in both directions), and the Larmor angle is about 41° in both: once the two integrals agree, the hard-edge model represents this solenoid well (for the small $x$/$y$ difference of the map and the smoothed integrals see [In AVAS](#solenoid-in-avas)).

```avas-beamline
drift 0.05 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.6 0.02 0
```

```avas-beamline
drift 0.162 0.02 0
solenoid 0.13 0.02 0 1.95
drift 0.708 0.02 0
```

### Step 5: off axis the longitudinal field changes a little too

Where there is no current, a magnetic field "does not swirl" (adding up the field component along any small closed loop gives zero; see step 1 of the [quadrupole](#quadrupole-deep-field) chapter). Applying this rule to a small rectangular loop in a plane containing the axis shows that at a distance $r$ from the axis the longitudinal field is slightly smaller than on the axis, $B_z \approx B - \tfrac{r^2}{4}B''$, where $B''$ is the rate of change of the rate of change of the on-axis field (how strongly the field curve bends). This term grows as $r^2$; it is a nonlinear effect and one source of the spherical aberration mentioned in "Going deeper: rotation, coupling and projected emittance".

### For readers who know vector analysis

For an axially symmetric field in cylindrical coordinates $\nabla\cdot\boldsymbol{B} = \frac{1}{r}\frac{\partial (r B_r)}{\partial r} + \frac{\partial B_z}{\partial z} = 0$. Near the axis $B_z \approx B(z)$ (the on-axis field), and integrating once in $r$ gives

$$
B_r(r, z) \approx -\frac{r}{2}\,B'(z),\qquad
B_z(r, z) \approx B(z) - \frac{r^2}{4}\,B''(z),
$$

the second from the azimuthal component of $\nabla\times\boldsymbol{B} = 0$, $\partial B_r/\partial z = \partial B_z/\partial r$ (paraxial expansion). In the hard-edge model $B'$ is a $\delta$ function at the ends.

## Going deeper: from the Larmor rotation to the transfer matrix {#solenoid-deep-matrix}

The main text used geometry (a small circle through the axis) to explain how a solenoid focuses. This section turns it, step by step, into the 4 × 4 transfer matrix used in accelerator design; the tools are the "conversion table" (matrix) of the [drift](#drift-deep-matrix) chapter and the simple harmonic motion of the [quadrupole](#quadrupole-deep-matrix) chapter; the general treatment of transfer matrices is in [Linear optics](#linear-optics).

### Step 1: the push at the entrance, written in x and y

Step 1 of the main text said that the entrance gives a particle at distance $r$ an angle $k r$ about the axis, clockwise seen from downstream for $B > 0$. Split into $x$ and $y$: a particle at $+x$ is pushed towards $-y$, a particle at $+y$ towards $+x$. So the entrance amounts to

$$
\Delta x' = +k\,y,\qquad \Delta y' = -k\,x .
$$

$x'$, $y'$ are the angles of the particle's path to the axis ([drift](#drift-deep-matrix) chapter). The change of the $x$ angle depends on $y$: the two directions are mixed, and this is "coupling". The exit gives the push with the opposite sign (using $x$, $y$ at the exit).

### Step 2: turn along with the beam

Step 2 of the main text found that, seen from the axis, the particle turns about the axis at a steady rate, $k$ radians per metre ($k\,s$ is the angle after a distance $s$), while its distance from the axis varies as $r\cos(ks)$.

Imagine a coordinate system that turns with the beam (the **Larmor frame**), also by $k$ radians per metre. In this turning frame the particle no longer turns about the axis; it just moves back and forth along a straight line towards and away from the axis, its position varying as $\cos(ks)$. This is exactly the focusing direction of the [quadrupole](#quadrupole-deep-matrix) chapter: a simple harmonic motion $w'' = -k^2 w$ ($w$ is the position in the turning frame) with "spring constant" $k^2$. $k^2$ is always positive, so both directions are focused.

So in the Larmor frame the conversion table of each direction is the thick-lens matrix of the quadrupole's focusing direction with $\sqrt{k_{\text{quad}}}$ replaced by $k$:

$$
F_2 = \begin{pmatrix} C & S/k \\ -kS & C \end{pmatrix},\qquad C = \cos kL,\quad S = \sin kL .
$$

Check: a particle entering parallel at $r_0$ at $s = 0$ leaves at position $C\,r_0 = r_0\cos kL$, as the right triangle of the main text says, and at angle $-kS\,r_0$, the "heads for the axis at $k\sin kL$ times" of the main text.

### Step 3: turn back

At the exit the Larmor frame has turned through $kL$ relative to the laboratory. Converting $(x, y)$ of the turning frame back to the laboratory takes a "rotation table" (the angles turn too):

$$
R(\theta) = \begin{pmatrix} \cos\theta\, I & \sin\theta\, I \\ -\sin\theta\, I & \cos\theta\, I \end{pmatrix},
$$

where $I$ is the 2 × 2 identity matrix and each entry stands for a 2 × 2 block acting on $(x, x', y, y')$. $R(\theta)$ takes the point $(x, y) = (x_0, 0)$ to $(x_0\cos\theta, -x_0\sin\theta)$: for $qB > 0$, seen from downstream the beam turns **clockwise** through $\theta$, as in the main text.

The whole solenoid is "focus in the turning frame, then turn back":

$$
M_{\text{sol}} = R(kL)\,F(kL),\qquad
F = \begin{pmatrix} F_2 & 0 \\ 0 & F_2 \end{pmatrix}.
$$

$F$ is the same in both directions, so $R$ and $F$ commute: the rotation can be placed before or after the focusing. Multiplying out, the transfer matrix in laboratory coordinates $(x, x', y, y')$ is

$$
M_{\text{sol}} =
\begin{pmatrix}
C^2 & \dfrac{SC}{k} & SC & \dfrac{S^2}{k}\\[4pt]
-kSC & C^2 & -kS^2 & SC\\[4pt]
-SC & -\dfrac{S^2}{k} & C^2 & \dfrac{SC}{k}\\[4pt]
kS^2 & -SC & -kSC & C^2
\end{pmatrix}.
$$

This is exactly the hard-edge solenoid matrix used by the AVAS linear envelope preview ($k = B_z/(2B\rho)$, entrance and exit fringe effects included). The longitudinal block is that of a drift of the same length, $R_{56} = L/\gamma^2$.

Check: for the example beam with $B = 1$ T and $L = 0.2$ m, $k = 2.80$ m⁻¹, $kL = 32.1^\circ$, $C = 0.847$, $S = 0.531$, so $C^2 = 0.718$, $SC = 0.450$, $SC/k = 0.161$, $S^2/k = 0.101$, $kSC = 1.260$, $kS^2 = 0.789$, entry by entry the numerical example in [In AVAS](#solenoid-in-avas).

### Step 4: focal length and rotation angle

- Larmor angle: $\theta_L = kL$; for a field varying along $s$, $\theta_L = \int k\,\dd s = \dfrac{1}{2B\rho}\int B\,\dd s$.
- Thick-lens focal length (in the Larmor frame, the same in both directions):

$$
\frac{1}{f} = k\sin kL .
$$

- Thin-lens approximation ($kL \ll 1$, $\sin kL \approx kL$):

$$
\frac{1}{f} \approx k^2 L = \left(\frac{B}{2B\rho}\right)^2 L,\qquad \frac{1}{f} \approx \frac{1}{4(B\rho)^2}\int B^2\,\dd s\ \ (\text{general } B(s)).
$$

The rotation is proportional to $\int B\,\dd s$ and the focal length is set by $\int B^2\,\dd s$: these two integrals fully describe a thin solenoid. For the example beam with $B = 1$ T and $L = 0.2$ m, the thick-lens $f = 0.673$ m and the thin-lens $f = 0.638$ m. Doubling the field and halving the length keeps the rotation and halves the focal length (0.336 m).

### Step 5: check it in the lab

Check these results with the solenoid lab (hard-edge matrix model; the initial beam is a round waist with $\beta = 0.4$ mm/mrad). The lab also draws the trajectory of a particle starting parallel at $x = 1$ mm and its projection onto the $(x, y)$ plane (seen from downstream):

1. Raise `B` step by step from 0.5 T to 2 T and compare the thick- and thin-lens focal lengths to verify $1/f \propto B^2$; reverse the sign of `B`: the focal length stays the same and the projected trajectory turns the other way.
2. Keep $BL$ fixed (e.g. `B=2 L=0.1`) and see the rotation unchanged while the focal length halves.
3. Look at the projection: inside the solenoid the particle moves along an arc (a piece of the small circle through the origin of the main text), and after it heads straight for the origin. Adjust `d2` to find the waist downstream of the solenoid.

```avas-lab
solenoid L=0.2 B=1 W=1.527 mass=938.272 q=1 emit=0.1377 d1=0.2 d2=0.6
```

### For readers who know differential equations and analytical mechanics

Keeping linear terms in Cartesian coordinates, $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$ with $B_x = -\tfrac{x}{2}B'$, $B_y = -\tfrac{y}{2}B'$ gives

$$
x'' = \frac{q}{p}\left(B\,y' + \tfrac12 B'\,y\right),\qquad
y'' = -\frac{q}{p}\left(B\,x' + \tfrac12 B'\,x\right).
$$

With $k(s) = qB(s)/(2p) = B(s)/(2B\rho)$, the complex coordinate $u = x + \mathrm{i}\,y$ combines both into $u'' + 2\mathrm{i}\,k\,u' + \mathrm{i}\,k'\,u = 0$. Substituting $u = w\,\ee^{-\mathrm{i}\theta(s)}$ with $\theta' = k$ cancels the first-derivative terms:

$$
w'' + k^2(s)\,w = 0 ,
$$

i.e. in the Larmor frame the two directions decouple and each is focused with strength $k^2$. That the rotation stops at the exit is exact: in an axially symmetric magnetic field the **canonical angular momentum**

$$
P_\theta = \gamma m r^2 \dot\theta + q\,r A_\theta = \gamma m r^2\dot\theta + \frac{q}{2\pi}\,\Psi(r, z)
$$

is conserved, where $\Psi$ is the magnetic flux inside radius $r$; paraxially $A_\theta = rB/2$ and $\Psi = \pi r^2 B$ (Busch's theorem). A particle entering from a field-free region has $P_\theta = 0$, and after leaving, $B = 0$, so $\dot\theta = 0$ — however much closer to the axis it has been pulled inside. At the entrance, $\dd(\gamma m r^2\dot\theta)/\dd t = r F_\theta$ with $F_\theta = q v_z B_r$ gives $\dot\theta = -qB/(2\gamma m)$: the particle turns about the axis at the **Larmor frequency** (half the cyclotron frequency), independent of $r$. Inside, the radial force $F_r = q v_\theta B = -q^2B^2 r/(2\gamma m)$; half of it maintains the circular motion (the centrifugal term $\gamma m r\dot\theta^2 = q^2B^2 r/(4\gamma m)$), and the other half gives $r'' = -k^2 r$.

> **Think about it**: what if the particles are "born" inside the field (for example ionised in the magnetic field of an ECR ion source) — what happens after they leave it?
> Answer: then $P_\theta = \tfrac{q}{2\pi}\Psi \neq 0$, and after leaving the field $\dot\theta = P_\theta/(\gamma m r^2) \neq 0$: the beam leaves rotating. No axially symmetric element can remove this, since $P_\theta$ is conserved in all of them (only non-symmetric optics such as a skew-quadrupole channel can convert it, at the price of a flat beam); in the $x$–$x'$ and $y$–$y'$ planes it shows up as an increased projected emittance. This is one source of the emittance of beams extracted from ion sources (see [Ion source and LEBT](#journey-source)).

## Going deeper: rotation, coupling and projected emittance {#solenoid-deep-dynamics}

So far the solenoids were ideal and the beams round. This section looks at four things that happen in real situations; each is described first and then given a number.

### Round beams and projected emittance

Emittance measures the area the beam occupies in the "position–angle" picture (see the [drift](#drift-size) chapter). At the solenoid exit a round beam has the same emittance as at the entrance: rotating as a whole does not change the distribution a round beam shows in $x$. But **inside** the solenoid the particles are turning about the axis: just after the entrance, $x'$ has an extra term $+k\,y$ ($y'$ an extra $-k\,x$). This term has nothing to do with the particle's $x$; it just spreads the angles, so looking at $x$ alone, the area in the "position–angle" picture grows. Assuming $x$ and $y$ uncorrelated, the definition of emittance $\varepsilon^2 = \langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2$ (angle brackets are averages over all particles) with $\langle x'^2\rangle$ increased by $k^2\langle y^2\rangle$ gives

$$
\varepsilon_x^2 = \varepsilon_0^2 + k^2 \langle x^2\rangle\langle y^2\rangle
$$

($\varepsilon_0$ is the entrance emittance). It grows temporarily inside the solenoid and recovers after it. For the example beam at the entrance of a 1 T solenoid (rms size 0.95 mm), the projected emittance temporarily becomes about 1.46 times its entrance value. With a field-map solenoid, the "bump" in the rms emittance in the middle of the solenoid in DataSet.txt has this origin; it does not mean the beam quality got worse.

### Flat beams and coupling

The solenoid matrix has non-zero off-diagonal $x$–$y$ blocks. For a round beam this does not matter; for a flat beam (for example after quadrupoles) the solenoid "rotates" emittance from one direction into the other, causing real projected-emittance growth. In lattices mixing solenoids and quadrupoles, either the solenoids are paired with opposite excitation to cancel the total rotation (see [Think about it 2](#solenoid-think)), or the beam is made round before the quadrupole section.

### Chromatic and spherical aberration

The focusing strength is $\propto 1/p^2$, so particles of different energy have different focal lengths; an energy spread gives a focal-length variation $\Delta f/f \approx 2\delta$ ($\delta$ is the relative momentum deviation). Further off axis, higher-order field terms (the $r^2 B''$ term of $B_z$ and the third-order radial field at the ends) cause **spherical aberration**: large-amplitude particles are over-focused, producing an "S"-shaped distortion in phase space, a common cause of emittance growth in LEBTs. The matrix model contains none of this; field-map elements are tracked by the engine and include the nonlinearities in the map.

### Space charge and neutralisation

The space charge of low-energy, high-current beams is strong; the axially symmetric focusing of a solenoid matches the space-charge force of a round beam, which makes matched round beams easy to form. In a real LEBT, ionisation of the residual gas partly neutralises the space charge; AVAS's lattice keyword `spacechargecomp V` (space-charge compensation) multiplies the space charge by $(1 - V)$ from that line on (how the kernels differ: see [Space charge](#space-charge-in-avas)); it is only a constant factor without modelling electron trapping, so treat the degree of neutralisation itself as a separately assessed uncertainty.

## In AVAS {#solenoid-in-avas}

### Matrix-model solenoid

```text
solenoid L R 0 B
```

```avas-params
solenoid
```

- `L`: length (m), the hard-edge length.
- `R`: aperture radius (m).
- The third parameter is reserved; write 0.
- `B`: axial field (T); its sign sets the sense of rotation and does not affect focusing (focusing goes as $B^2$).

> **AVAS convention**: $k = B/(2B\rho)$, with the hard-edge matrix of the "going deeper" part. `solenoid` is a z-code matrix-model element and likewise should not be the first or last element of a multi-particle lattice.

### Field-map solenoid

A real solenoid's fringe field is wide (comparable to the coil radius) and in compact cavity–solenoid periods often overlaps neighbouring elements, so it is usually described with a static magnetic field map:

```text
field 0.35 0.02 0 3 0 0 0 Kb sol
```

The example project `examples/hwr010` includes the solenoid field map `sol` (`sol.bsx`, `sol.bsy`, `sol.bsz`), 0.35 m long, with a transverse grid of $\pm 0.02$ m. Actual field = map × `Kb` (T). The raw on-axis maximum of $B_z$ in this map is about 10.1 (map units, T); after 9-point smoothing the peak is about 8.3, at $z \approx 0.177$ m; so `Kb = 0.1` corresponds to a peak field of about 0.83 T (raw maximum about 1.0 T).

> Note: the example map itself is noisy: `sol.bsz` jumps point by point between 5.9 and 10.1 in the "flat-top" region, and the vertical stripes along $z$ in the slice heat map are the data itself, not a reading error. The integrals ($\int B\,\dd z$, $\int B^2\,\dd z$) are more reliable than the peak for estimating rotation and focal length.

Integrating the map gives $\int B\,\dd z \approx 0.729\,\mathrm{Kb}$ T·m and $\int B^2\,\dd z \approx 4.04\,\mathrm{Kb}^2$ T²·m (the noise makes the latter slightly too large; the 9-point smoothed on-axis field integrates to about 3.91, giving an equivalent hard-edge solenoid of $B = 1.88$ T, $L = 0.136$ m and a waist of 0.83 mm, a few per cent from the $B = 1.95$ T, $L = 0.13$ m and 0.79 mm waist of the raw integrals). What really sets the focusing are integrals: from the start of the map to the peak, the near-axis gradient integrals are $\int (\partial B_x/\partial x)\,\dd z \approx -3.98$ and $\int (\partial B_y/\partial y)\,\dd z \approx -4.15$, in good agreement with the $-\tfrac12 B_{\text{peak}} = -4.14$ that $\nabla\cdot\boldsymbol{B} = 0$ requires for the smoothed peak. For the example beam ($B\rho = 0.17863$ T·m):

| `Kb` | Peak field | Larmor angle | Thin-lens focal length |
|---|---|---|---|
| 0.1 | ≈ 0.83 T | 11.7° | 3.16 m |
| 0.2 | ≈ 1.7 T | 23.4° | 0.79 m |
| 0.35 | ≈ 2.9 T | 40.9° | 0.26 m |

Field-map elements are tracked by the t-code and include the fringe field. The transverse components of the example `sol` do not agree point by point with $-\tfrac{r}{2}B_z'$: at $z = 0.12$ m ($r = 1$ cm), the smoothed on-axis slope gives $-\tfrac{r}{2}B' \approx -0.59$, close to the map's $B_x = -0.55$ (but at $y = 1$ cm $B_y = -0.42$; the $x$ and $y$ gradients differ point by point by 10–25 %, which is why the map gives waists of 0.81 / 0.75 mm even with identical initial Twiss parameters in both directions); at the steepest point, $z = 0.1435$ m, the formula gives $-1.44$ but the map only $B_x \approx -0.99$ ($+0.65$ against $+0.35$ at $z = 0.23$ m). Using the map's noisy transverse components directly for linear slices therefore makes the period transfer matrix deviate from a symplectic matrix; the AVAS linear preview instead derives the radial fringe field from the on-axis $B_z$ via $\nabla\cdot\boldsymbol{B} = 0$, and takes only the divergence-free quadrupole and skew-quadrupole parts from the map's $B_x$, $B_y$. Map format, storage order and normalisation are in [Field maps](#field-maps); overlapping a solenoid with an RF cavity is in [Superposed fields](#superpose).

### Typical values

| Quantity | Typical range | Note |
|---|---|---|
| Field $B$ | room temperature 0.1–0.6 T; superconducting up to several T | strong fields are needed for short focal lengths at low energy |
| Length $L$ | 0.1–0.4 m | solenoids inside superconducting cryostats are usually short |
| Aperture radius $R$ | 0.01–0.05 m | larger in LEBTs |
| Rotation $\theta_L$ | a few to tens of degrees | only affects the beam orientation, not the rms size of a round beam |

### Numerical examples

The example beam (protons, 1.527 MeV, $B\rho = 0.17863$ T·m) through a hard-edge solenoid:

| $B$ | $L$ | $k$ | $kL$ | Thick-lens $f$ | Thin-lens $f$ |
|---|---|---|---|---|---|
| 0.5 T | 0.2 m | 1.400 m⁻¹ | 16.0° | 2.59 m | 2.55 m |
| 1 T | 0.2 m | 2.799 m⁻¹ | 32.1° | 0.673 m | 0.638 m |
| 1.5 T | 0.2 m | 4.199 m⁻¹ | 48.1° | 0.320 m | 0.284 m |
| 2 T | 0.1 m | 5.598 m⁻¹ | 32.1° | 0.336 m | 0.319 m |
| 2 T | 0.2 m | 5.598 m⁻¹ | 64.2° | 0.198 m | 0.160 m |

For $B = 1$ T and $L = 0.2$ m the 4×4 matrix is

$$
M =
\begin{pmatrix}
0.718 & 0.161 & 0.450 & 0.101\\
-1.260 & 0.718 & -0.789 & 0.450\\
-0.450 & -0.101 & 0.718 & 0.161\\
0.789 & -0.450 & -1.260 & 0.718
\end{pmatrix},
\qquad \det M = 1 .
$$

For comparison, a typical LEBT solenoid: 35 keV protons ($B\rho = 0.0270$ T·m) through a 0.3 T, 0.2 m solenoid have $kL = 64^\circ$ and a thick-lens focal length of 0.20 m (the thin-lens estimate, 0.16 m, is clearly off).

### Common problems

- **`Kb` of the field map too large**: the peak of the example `sol` is about 8.3 T × `Kb` (raw maximum about 10 T × `Kb`), so `Kb = 1` means 8–10 T; for 1.5 MeV protons the thin-lens focal length is then only about 3 cm, far shorter than the solenoid itself, and the beam is over-focused inside the solenoid and diverges rapidly.
- **Comparing matrix and field-map results**: make the $\int B^2\,\dd z$ of the two equal (equal focal length), not the peak or the length; the rotation $\int B\,\dd z$ will then generally differ slightly. The two beam lines in the "going deeper" part match both integrals.
- **Taking a reversed `B` for defocusing**: see "Common misconceptions".

## Further reading {#solenoid-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed. (axially symmetric fields, Busch's theorem, solenoid lenses).
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed. (transverse focusing in linacs, including solenoid focusing).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (solenoid transfer matrix and coupling).
- Related chapters: [Quadrupole](#quadrupole), [Field maps](#field-maps), [Superposed fields](#superpose), [RF cavity](#rf-cavity), [Space charge](#space-charge); to build a cavity–solenoid structure yourself see the [tutorial](#tutorial); full bibliography in [References](#references).
