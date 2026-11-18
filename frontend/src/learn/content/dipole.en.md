# Dipole

So far the beam has always flown in a straight line. But a beam has to be delivered somewhere: to different experimental stations, into a beam dump, or around a corner of a hall that is too small. To make a bunch of very fast protons turn, one uses a **dipole magnet**: two poles facing each other above and below, the vacuum pipe between them, and a uniform magnetic field inside the pipe.

A dipole's job sounds like the simplest of all: just bend the beam's direction through an angle. But after a dipole two unexpected things happen:

1. The field is equally strong everywhere, with no gradient of the "the farther out, the harder the push" kind, and yet the beam is **focused** in the bending direction.
2. How much the beam grows depends on how spread out the particles' **energies** are.

Both come from one simple fact: **every particle in the field moves on an arc of a circle**. This chapter starts from that arc.

First, see what one dipole does. In the animation the beam passes a 0.15 m drift, then a dipole that bends by 30° with a bending radius of 0.5 m (arc length 0.26 m), and finally a 0.5 m drift.

```avas-motion
dipole
```

Click "3D" first: the magnet and the vacuum pipe both follow the arc, and the beam turns through 30° with them. The magnet is an "H magnet" (the cross-section of its iron yoke looks like the letter H, with two poles above and below the pipe); the S pole is on top and the N pole below, so the field points up. The pink F arrows are the force on the particles, pointing towards the centre of the bend.

Now the 2D pictures. Here $x$ is not measured from a straight line but from the **curved reference orbit** (the arc the ideal particle follows), positive towards the outside of the bend. So the whole beam turns, but in the picture it stays in the middle. Notice three things:

- **The two dashed lines**: they are the centres of two beams whose momentum is 1 % above and 1 % below the nominal one. They separate inside the dipole and are 3.2 mm off on either side at the end. The higher momentum goes on the outside ($+x$), the lower on the inside. This is called **dispersion**.
- **The beam is narrower horizontally than vertically**: at the end rms $x$ is 2.7 mm and rms $y$ is 3.4 mm, yet entering the dipole both were almost the same (about 1.3 mm). The uniform field helped in the bending direction; this is the "weak focusing" this chapter explains.
- **The red arrows in the cross-section**: of the two blue particles one is off in $x$ and the other in $y$, but the forces on them point the same way, both towards $-x$ (the centre of the bend). In a uniform field the force is the same everywhere. So where does the focusing come from? We shall see that the answer is geometry.

Parameters you can drag: the bend angle α, the bend radius ρ, the field index N, the pole-face angle β and the kinetic energy W. Each one tests one statement in the text; we shall try them when we get there.

## What dipoles are used for {#dipole-role}

In a linac the dipole is not the main actor, but beam transport lines, energy analysis, beam-dump branches and injection / extraction lines cannot do without it:

- **Changing the beam direction**: sending the beam to different end stations, target stations or beam dumps (see [High-energy transport](#journey-hebt)).
- **Momentum (energy) analysis**: dispersion separates particles of different momenta in space; with a narrow opening (a slit) one can pick out particles of one energy, or measure how large the energy spread is.
- **Geometry**: folding a beam line into a limited space.
- **Longitudinal manipulation**: in a bend the outer particles travel a longer path and the inner ones a shorter one, which can be used to change the bunch length (compress or stretch the bunch). For low-energy protons, though, the lengthening caused by the velocity difference is usually larger; see [going deeper](#dipole-deep-matrix).

## A magnetic field makes particles move on arcs {#dipole-field}

### The Lorentz force as the centripetal force

From school physics: a charged particle moving in a magnetic field feels the **Lorentz force** $F = qvB$ (when the velocity is perpendicular to the field), perpendicular both to the velocity and to the field. Since the force is always perpendicular to the velocity, it changes only the direction, not the speed. A force of constant size that is always perpendicular to the velocity is exactly the **centripetal force** of uniform circular motion:

$$
q\,v\,B = \frac{m\,v^2}{\rho}
\quad\Longrightarrow\quad
\rho = \frac{m v}{q B} = \frac{p}{q B}.
$$

$\rho$ is the radius of the circle (the **bending radius**) and $p = mv$ the momentum. Writing $p/q$ on its own as $B\rho$ gives the **magnetic rigidity**:

$$
B\rho = \frac{p}{q}.
$$

It means "how hard it is to bend this particle with a magnetic field": how many tesla are needed to bend it on a circle of 1 m radius. For a magnet with field $B$, the bending radius is $\rho = B\rho / B$.

For the example protons: $m = 1.673\times 10^{-27}$ kg, $v = 1.708\times 10^{7}$ m/s (about 5.7 % of the speed of light), $q = 1.602\times 10^{-19}$ C, giving $B\rho = 0.178$ T·m. (The more accurate value is 0.1786 T·m: for faster particles the momentum is slightly larger than $mv$, here by only 0.16 %; see [going deeper](#dipole-deep-geometry).)

The animation's default asks for $\rho = 0.5$ m, so the field is $B = 0.1786/0.5 = 0.357$ T. Drag ρ to 1 m in the animation and the field needed is only half (0.179 T); drag the kinetic energy to 10 MeV and bending on the same 0.5 m radius needs 0.916 T: the "stiffer" the particle, the harder it is to bend.

The bend angle $\alpha$ and the arc length $L$ the particle travels in the field are related by $L = \rho\,\alpha$ ($\alpha$ in radians). In the animation $0.5\ \mathrm{m} \times 30^\circ \times \pi/180^\circ = 0.26$ m.

Some numbers:

| Beam | $B\rho$ | $B$ for $\rho = 1$ m |
|---|---|---|
| protons 35 keV (LEBT) | 0.0270 T·m | 0.027 T |
| protons 1.527 MeV (example) | 0.1786 T·m | 0.179 T |
| protons 100 MeV | 1.483 T·m | 1.48 T |
| protons 1 GeV | 5.66 T·m | 5.66 T |

Room-temperature iron-dominated dipoles are generally limited to below 1.5–2 T (beyond that the iron saturates), so 1 GeV protons need either several metres of bending radius or superconducting magnets; for the example beam 0.18 T is enough for a 1 m radius.

### Which way it bends

Use the right-hand rule: point the fingers of your right hand along the velocity of a positive charge, curl them towards the field, and the thumb gives the force. With the AVAS cross-section convention (**seen from downstream looking upstream**, the beam coming out of the screen towards you, $x$ to the right, $y$ up), a field pointing up ($B_y > 0$) pushes a positive charge to the left ($-x$), so it bends towards $-x$ and the centre of curvature is on the left. Below is the field in a dipole's cross-section (a **schematic** computed from the parameters, not measured data): the field is the same everywhere in the aperture. The `B` here only sets the sign; the actual field is $B\rho/\rho$, and the picture shows magnitudes relative to the on-axis value.

```avas-field
dipole B=0.18 N=0 rho=1 L=0.785 R=0.03
```

Set α to −30° in the animation: the beam turns the other way and the two dispersion lines change sides too.

## A uniform field also focuses: two circles of the same size {#dipole-focusing}

### Two circles of the same radius

Now the first puzzle. Two particles with the same momentum enter the uniform field side by side: one on the reference orbit, the other a little to the outside, at distance $x_0$.

Same momentum and same field means the same bending radius: the two circles are equally large, but the centre of the second one is also shifted outwards by $x_0$. Two equal circles whose centres are slightly apart are not the same distance apart everywhere:

- at the entrance the two orbits are $x_0$ apart;
- after $90^\circ$ the two circles **intersect**: the distance is zero;
- after $180^\circ$ the distance is $-x_0$: the outer particle has crossed to the inside.

In general, after an angle $\theta$ the distance is $x_0\cos\theta$ (derived in [going deeper](#dipole-deep-geometry), using only the law of cosines). The distance keeps shrinking, so the outer particle is closing in on the reference orbit: **a uniform field is by itself a lens in the bending direction**. The physical picture: the outer particle goes round a circle shifted outwards, and it turns towards the reference orbit before it leaves the magnet, as if pulled back by an invisible force. There is no such force: each particle just follows its own circle. That is also why the red arrows in the animation's cross-section point the same way and have the same length: the force really is the same everywhere, and the "focusing" comes entirely from geometry.

### How strong this lens is

In optics, the focal length $f$ is the $f$ in "deflection angle = distance from the axis ÷ focal length". For a stretch of uniform field that turns through $\theta$ this gives

$$
f = \frac{\rho}{\sin\theta}.
$$

For the animation, $\rho = 0.5$ m and $\theta = 30^\circ$ give $f = 1.0$ m, whereas the 0.1 m long, 10 T/m quadrupole of the [Quadrupole](#quadrupole) chapter has a focal length of only 0.18 m. A dipole's focusing is weak; hence the name **weak focusing**.

And vertically? The field is vertical, so the vertical part of a particle's velocity is parallel to the field and feels no force at all (the Lorentz force comes only from the velocity perpendicular to the field). So **nothing happens vertically**: it is the same as a drift of the same length.

This is what the animation shows: entering the dipole both rms sizes are almost equal (1.26 mm); at the dipole exit ($z \approx 0.41$ m) rms $x$ is 1.76 mm and rms $y$ 1.94 mm, and at the end they are 2.7 mm and 3.4 mm. Increase α to 60° and at the end rms $y$ becomes 4.2 mm while rms $x$ is only 3.0 mm: the more it turns, the more focusing in the bending direction.

### Making the field slightly non-uniform: the field index

Can the vertical direction be focused as well? The trick is to make the two poles not exactly parallel but opening slightly, so that the field is a little weaker on the outside and a little stronger on the inside. The number describing this is the **field index** $n$ (written `N` in AVAS): $n = 0$ is a uniform field, $n > 0$ means the field gets weaker outwards.

Once the field is non-uniform, the field lines are no longer straight vertical lines but bulge outwards a little. A particle away from the middle plane meets these slanted field lines and feels a force back towards the middle plane: **the vertical direction is focused**. The price is weaker horizontal focusing: the outer particle sees a weaker field and turns a little less. For $0 < n < 1$ both directions are focused, and for $n = 0.5$ both equally.

Below is the field for $n = 0.5$ (**schematic**). At the 3 cm edge of the aperture the field is only 1.5 % weaker or stronger than on axis; the field index is a very gentle gradient. The schematic shows both how $B_y$ changes with $x$ and the horizontal component $B_x$ that grows with $y$ (the part that makes the field lines bulge); the direction of the gradient depends only on $n$, not on the bending direction (for the opposite bend $B_0$ and "outside" both change sign).

```avas-field
dipole B=0.18 N=0.5 rho=1 L=0.785 R=0.03
```

Drag N to 0.5 in the animation: at the end rms $x$ becomes 3.1 mm and rms $y$ 2.9 mm; the beam is narrower vertically than before and wider horizontally. (Without the dispersion of the next section both directions are 2.9 mm: "both equally strong".) The quantitative derivation is in [going deeper](#dipole-deep-geometry).

## Dispersion: different momentum, different orbit {#dipole-dispersion}

### Two touching circles

Now the second puzzle. Two particles enter at the same point in the same direction, one with the nominal momentum $p$, the other with a little more, $p(1+\delta)$. $\delta$ is the relative momentum deviation; $\delta = 0.01$ means "1 % more".

The particle with more momentum is harder to bend, and its circle is larger by $\rho\delta$. The two circles touch at the entrance (same point, same direction) and then separate further and further. After an angle $\theta$ the distance and the angle between the two orbits are

$$
\Delta x = \rho\,(1-\cos\theta)\,\delta,\qquad \Delta x' = \sin\theta\,\delta .
$$

(Again derived in [going deeper](#dipole-deep-geometry).) $\Delta x'$ is the angle between the two orbits, in radians. The coefficients $D = \rho(1-\cos\theta)$ and $D' = \sin\theta$ are the **dispersion** at the dipole exit: the offset in position and angle per unit (that is, per 100 %) of momentum deviation.

For the animation: $\rho = 0.5$ m and $\theta = 30^\circ$ give $D = 0.5\times(1 - 0.866) = 0.067$ m and $D' = 0.5$. A particle with 1 % more momentum is 0.67 mm further out at the dipole exit, and it keeps moving outwards at an angle of 5 mrad. Over the 0.5 m drift it moves out another $0.5\ \mathrm{m}\times 5\ \mathrm{mrad} = 2.5$ mm, 3.2 mm in total at the end: exactly the offset of the dashed line at the end of the animation. Drag α to 60° and the offset at the end becomes 6.8 mm; at 90° it is 10.0 mm.

This shows something important: **dispersion is created in the dipole but keeps growing in the drifts downstream**, until a dedicated structure removes it.

### Dispersion makes the beam larger

The particles in a beam do not all have the same momentum. The rms value of the example beam's relative momentum deviation (the **momentum spread**) is $\sigma_\delta = 3.5\times 10^{-3}$, i.e. 0.35 %. So besides its own offset, each particle moves by another $D\,\delta$ according to its $\delta$. The two offsets are unrelated, and in statistics the rms values of unrelated quantities add in squares:

$$
\sigma_x^2 = \sigma_{x,\beta}^2 + D^2\,\sigma_\delta^2 .
$$

$\sigma_{x,\beta}$ is the size without dispersion (called the **betatron part** in accelerators: the particles' transverse oscillation about the reference orbit). At the end of the animation $D = 0.317$ m, so $D\sigma_\delta = 1.1$ mm; the betatron part is 2.44 mm; together $\sqrt{2.44^2 + 1.1^2} = 2.7$ mm, exactly the rms $x$ the animation gives at the end.

The beam line below has a 45°, $\rho = 1$ m sector dipole instead (with an edge of pole-face angle 0 before and after it, see the [next section](#dipole-edge)), followed by a 0.5 m drift; the 3D view shows the line turning. This linear preview includes the space charge of the example beam's 0.184 mA. At the dipole exit ($z \approx 0.99$ m) rms $x$ in the bending plane is 3.2 mm, smaller than the 3.7 mm of rms $y$: weak focusing. At the end of the drift rms $x$ is 4.4 mm, of which about 2.2 mm (subtracting in squares) comes from dispersion: without a momentum spread rms $x$ would be 3.8 mm here.

```avas-beamline
drift 0.2 0.03 0
edge 0 0.03 0 0 1 0 0 0 0
bend 0 0.03 0 45 1 0 0
edge 0 0.03 0 0 1 0 0 0 0
drift 0.5 0.03 0
```

At 45° and $\rho = 1$ m the exit dispersion is $D = 0.293$ m: a particle with 1 % more momentum is 2.9 mm further out, three times the example beam's initial rms size (about 1 mm). At the end of the drift $D = 0.293 + 0.5\times 0.707 = 0.646$ m and $D\sigma_\delta = 2.3$ mm (the preview gives about 2.2 mm by subtracting in squares; the difference comes from the space-charge coupling). At $90^\circ$, $D = \rho = 1$ m; at $180^\circ$, $D = 2\rho = 2$ m and $D' = 0$: at $180^\circ$ a magnetic spectrometer both images and has its largest dispersion.

### Achromats

If the dispersion is not removed, every element downstream sees a beam with this extra width. A layout in which both $D$ and $D'$ vanish at the exit of the bending section is an **achromat**. One example: two identical dipoles with a quadrupole between them that makes $D' = 0$ at the midpoint (the dispersion orbit runs parallel to the reference orbit there); by left–right symmetry the exit dispersion is then zero, just like at the entrance.

### Try it

The dipole lab gives the sector-magnet transfer matrix (see [going deeper](#dipole-deep-matrix)), $D$ and $D'$ at the exit, the orbits of off-momentum particles and the $x$, $y$ envelopes (the envelopes exclude the momentum spread). $R_{16}$ in the table is the exit $D$:

1. Reproduce $R_{16} = 0.293$ m with `N=0`, `edge=0`; change `angle` to 90 and 180 to verify $D = \rho(1-\cos\theta)$, and see $R_{21} = 0$, $D' = 0$ at $180^\circ$.
2. Change `edge` to 22.5 (a rectangular magnet, see the next section): the horizontal focusing is cancelled and the vertical envelope is contained.
3. Change `N` to 0.5 and compare the two planes' envelopes. The orbit plot always shows both a $+\delta$ and a $-\delta$ particle (`delta` is the magnitude, 0–10 %): they leave on either side of the reference orbit; double `delta` and both offsets double: dispersion is linear in $\delta$.

```avas-lab
dipole angle=45 rho=1 N=0 edge=0 W=1.527 mass=938.272 q=1 delta=1
```

## Turning the pole faces: moving focusing to the other plane {#dipole-edge}

### Changing a particle's path in the field

The dipole above is a **sector magnet**: the field region is shaped like a wedge (a sector), and the end faces of the poles at entrance and exit (the **pole faces**) are perpendicular to the reference orbit. An outer particle travels further in the field and turns more, so it comes back towards the centre: another way of describing weak focusing.

If the pole faces are rotated by an angle $\beta$ (the **pole-face angle**), an outer particle's path in the field becomes shorter or longer, and its deflection smaller or larger. The farther from the axis, the larger the change, exactly in proportion to the distance: this amounts to a thin lens at the entrance and at the exit of the magnet. By the usual convention, $\beta > 0$ shortens the outer particles' path in the field and reduces their deflection: the bending direction is **defocused**, cancelling part of the weak focusing.

The other direction is affected too. At the end of the magnet the field falls from its full value to zero (this region is the **fringe field**), and the field lines curve outwards there; a particle away from the middle plane meets these curved field lines. When the pole face is tilted, part of this curved field is perpendicular to the particle's velocity and gives a vertical force: for $\beta > 0$ it pushes the particle back to the middle plane, so the vertical direction is **focused**.

The two effects are equally strong with opposite signs: an edge is like a thin quadrupole. This is again the rule from the quadrupole chapter: "whatever is focused in one direction is defocused in the other".

### The rectangular magnet

An important special case: both pole faces parallel to each other, each at $\theta/2$ to the incoming orbit, is a **rectangular magnet** (the field region is a rectangle). It is equivalent to a sector magnet with $\beta = \theta/2$ at both ends. Particles everywhere travel about the same length in the field, the horizontal weak focusing is cancelled completely, and the vertical direction is focused instead.

Drag the pole-face angle to 15° (half of the 30° magnet) in the animation: at the end rms $x$ goes from 2.7 mm to 3.5 mm and rms $y$ from 3.4 mm to 2.5 mm; the focusing has moved from horizontal to vertical.

Changing the two edges of the 45° line above to 22.5° shows the same thing: at the end of the drift rms $y$ drops from 5.2 mm (sector magnet) to 3.9 mm, while rms $x$ grows from 4.4 mm to 5.5 mm.

```avas-beamline
drift 0.2 0.03 0
edge 0 0.03 0 22.5 1 0 0 0 0
bend 0 0.03 0 45 1 0 0
edge 0 0.03 0 22.5 1 0 0 0 0
drift 0.5 0.03 0
```

The pole-face angle does not change the size of the dispersion (the 45° rectangular magnet still has $D = 0.293$ m at the exit), but it changes its angle $D'$. The matrices, the correction for the width of the fringe field and the numbers are in [going deeper](#dipole-deep-edge).

## Think about it {#dipole-think}

> **Think 1**: A dipole bends 1.527 MeV protons on a 0.5 m radius. What happens if H⁻ ions (one negative charge, almost the same mass) come along instead?
>
> Answer: $B\rho$ is almost the same in size, so the bending radius is still about 0.5 m, but the force points the other way and they bend to the other side. One magnet can therefore separate different particles by charge and momentum.

> **Think 2**: In the animation the forces on the two blue particles are identical. Why is the beam still focused in $x$?
>
> Answer: because each particle follows its own circle, and two equal circles with slightly shifted centres approach each other. The focusing comes from the geometry of the arcs, not from a difference in force.

> **Think 3**: A solenoid and a uniform dipole both have only uniform fields, yet both focus, with a strength proportional to $B^2$. A coincidence?
>
> Answer: no. A uniform field gives a particle parallel to the central orbit a force that does not depend on its position, so the first-order focusing is zero; the focusing appears after the field has changed the particle's path (a different path length in a dipole, a velocity around the axis first in a solenoid) and the field acts again, so it is second order. For the same field, the dipole's horizontal $h^2 = (B/B\rho)^2$ is exactly 4 times the solenoid's $k^2 = (B/2B\rho)^2$ (see [Solenoid](#solenoid)).

## Common misconceptions {#dipole-misconceptions}

- **"A uniform field has no gradient, so it does not focus"**: the bending plane has geometric focusing $h^2 = 1/\rho^2$, only much weaker than a quadrupole's; it is the vertical plane that is really not focused.
- **"Dispersion is a dipole affair"**: dispersion is created in dipoles, but it propagates and grows in the drifts and quadrupoles downstream until an achromatic structure removes it.
- **"A growing beam means emittance growth"**: the $D\sigma_\delta$ from dispersion is a correlation between momentum and position and is undone once the dispersion is removed; it is not emittance itself, though if left in place it shows up as a larger effective emittance.
- **"The pole-face angle affects only the bending plane"**: an edge acts on both planes, with opposite signs.
- **"The bending radius is fixed by the magnet"**: $\rho = B\rho/B$ depends on the beam's momentum; the same magnet bends particles of different energy or charge-to-mass ratio differently, which is exactly what makes momentum analysis possible.
- **"$z$ in the interface is a straight-line distance"**: after a dipole, $z$ in the interface is the path length along the central orbit.

## Going deeper: weak focusing and dispersion from the geometry of circles {#dipole-deep-geometry}

The main text gave three results: the distance in the bending plane varies as $x_0\cos\theta$, the focal length is $f = \rho/\sin\theta$, and the dispersion is $D = \rho(1-\cos\theta)$. This section derives them using only school circles and the law of cosines; the last paragraph is for readers who know calculus.

### Step 1: the exact momentum and rigidity

The main text used $p = mv$ and found $B\rho = 0.178$ T·m. For fast particles the momentum is $p = \gamma m v$, where $\gamma = 1/\sqrt{1 - v^2/c^2}$ is the relativistic factor (see [Kinematics](#kinematics)). For the example protons $\gamma = 1.00163$, so $p$ is 0.16 % larger than $mv$. The formula used in accelerators is

$$
B\rho\,[\mathrm{T\,m}] = \frac{p\,[\mathrm{MeV}/c]}{299.792458\,Z},
$$

with $Z$ the charge number. The example beam has $p = 53.55$ MeV/$c$, so $B\rho = 53.55/299.79 = 0.1786$ T·m. The derivation of $\rho = p/(qB)$ only used "the force is perpendicular to the velocity and constant in size", which still holds in relativity.

### Step 2: a useful approximation

Let the reference particle move on a circle of radius $\rho$ about the centre $O$. Another particle moves on a circle of radius $R$ whose centre $C$ is very close to $O$, $OC = c$. Draw a ray from $O$ at an angle $\phi$ to $OC$; it meets the second circle at distance $r$ from $O$. The law of cosines in the triangle $O$, $C$, intersection gives

$$
R^2 = r^2 + c^2 - 2\,r\,c\cos\phi .
$$

When $c$ is small, $c^2$ is negligible and $r$ is close to $R$, so $r^2 - R^2 = (r - R)(r + R) \approx 2R\,(r - R)$, and

$$
r \approx R + c\cos\phi .
$$

In words: **when the centre of a circle moves a little, the circle bulges out in each direction by the projection of that shift onto the direction**. The three results below are all this one sentence.

### Step 3: a particle shifted sideways

At the entrance the reference particle is at $P$, a distance $\rho$ from $O$. The second particle enters parallel to it, $x_0$ outside $P$, with the same momentum, so $R = \rho$ and its centre is also shifted outwards by $x_0$, the shift pointing along "outside" at the entrance. When the reference particle has turned through $\theta$, the direction seen from $O$ makes exactly the angle $\theta$ with the shift. By step 2, the radial distance between the two orbits is

$$
x = x_0\cos\theta .
$$

Check: zero at $\theta = 90^\circ$ (the circles intersect), $-x_0$ at $\theta = 180^\circ$.

### Step 4: a particle with a small angle

The second particle starts at $P$ but tilted outwards by a small angle $x_0'$ (in radians). Its circle is as large as the reference one but turned about $P$ by $x_0'$, so its centre moves by $\rho\,x_0'$ (radius times angle is the arc) along the reference orbit's forward direction. This shift is perpendicular to "outside" at the entrance, so after $\theta$ its projection is $\rho\,x_0'\sin\theta$:

$$
x = \rho\sin\theta\;x_0' .
$$

It is zero at $\theta = 180^\circ$: particles leaving one point at various small angles come together at one point again after half a circle. This is why a $180^\circ$ uniform dipole "images", the principle of early magnetic spectrometers.

### Step 5: a particle with more momentum

The second particle enters at $P$ along the reference direction with momentum $p(1+\delta)$ and radius $R = \rho(1+\delta)$. The two circles touch at $P$, so its centre lies on the other side of $O$ (inwards), at a distance $\rho\delta$ from $O$. By step 2 (the shift points opposite to "outside" at the entrance, so the projection gets a minus sign):

$$
r \approx \rho(1+\delta) - \rho\,\delta\cos\theta ,\qquad
x = r - \rho = \rho\,(1-\cos\theta)\,\delta .
$$

This is the dispersion $D = \rho(1-\cos\theta)$ of the main text. Numbers: $\rho = 0.5$ m, $\theta = 30^\circ$ give $D = 0.067$ m; $\rho = 1$ m, $\theta = 45^\circ$ give $D = 0.293$ m; at $\theta = 90^\circ$, $D = \rho$; at $180^\circ$, $D = 2\rho$.

### Step 6: from position to angle, and the focal length

A particle's angle $x'$ is the rate of change of its position $x$ with path length $s$. The reference particle has travelled $s = \rho\theta$. With school derivatives (the derivative of $\cos\theta$ is $-\sin\theta$, that of $\sin\theta$ is $\cos\theta$), the rate of change of each term with $s$ is its derivative with respect to $\theta$ divided by $\rho$:

- step 3: $x' = -\dfrac{\sin\theta}{\rho}\,x_0$;
- step 4: $x' = \cos\theta\;x_0'$;
- step 5: $x' = \sin\theta\;\delta$, which is the main text's $D' = \sin\theta$.

The first line says: a particle entering parallel, $x_0$ off, leaves deflected towards the axis by $x_0\sin\theta/\rho$. With the optics rule "deflection = distance from the axis ÷ focal length", the focal length is $f = \rho/\sin\theta$: 1.0 m for the animation ($\rho = 0.5$ m, $30^\circ$), 1.41 m for 45° and $\rho = 1$ m.

Compare with a quadrupole: its focusing strength is $k = G/B\rho$ (in m⁻²); the bending plane's "strength" written the same way is $h^2 = 1/\rho^2 = (B/B\rho)^2$, where $h = 1/\rho$ is the **curvature**. The example beam in 0.18 T has only 1 m$^{-2}$, in the animation's 0.357 T it has 4 m$^{-2}$, while a 6 T/m quadrupole has $k = 33.6$ m$^{-2}$. $h^2$ is the square of the field; this is what "second order" means.

### Step 7: the vertical direction and the field index

With a vertical field, the vertical velocity is parallel to the field and gives no force; vertically a dipole is a drift of length $L$.

For a non-uniform field, write the field near the axis as

$$
B_y(x) = B_0\left(1 - n\,\frac{x}{\rho}\right),\qquad
n = -\frac{\rho}{B_0}\,\frac{\Delta B_y}{\Delta x},
$$

with $x$ pointing from the central orbit away from the centre of curvature; $n$ is the field index of the main text. For $n > 0$ the field is weaker outside and the particle turns less: $n h^2$ must be subtracted from the "spring" $h^2$ of step 6. On the other hand, the [Quadrupole](#quadrupole-deep-field) chapter showed that where there is no current the field "does not swirl", so a horizontal gradient $\Delta B_y/\Delta x$ necessarily comes with an equal vertical gradient $\Delta B_x/\Delta y$, i.e. $B_x = -n B_0\,y/\rho$: for $n > 0$ the field lines bulge outwards and a particle away from the middle plane is pushed back, giving vertical focusing of strength $n h^2$.

Both directions behave like a spring oscillator (the simple harmonic motion of the [Quadrupole](#quadrupole-deep-matrix) chapter, with time replaced by path length), with "spring constants" $(1-n)h^2$ and $n h^2$:

- for $0 < n < 1$ both are positive and both directions are focused (**weak focusing**, on which early cyclotrons and synchrotrons relied);
- for $n = 1/2$ both directions are equally strong;
- the number of oscillations per full turn ($2\pi$) is $\nu_x = \sqrt{1-n}$, $\nu_y = \sqrt{n}$, both below 1, hence "weak".

For $n = 0$, $\nu_x = 1$: exactly one oscillation per turn, in agreement with the $\cos\theta$ of step 3.

### For readers who know calculus

In the Frenet–Serret frame that follows the curved central orbit ($x$ pointing away from the centre of curvature, $h = 1/\rho$), the linear equations of motion are

$$
x'' + (1-n)\,h^2\,x = h\,\delta,\qquad
y'' + n\,h^2\,y = 0 .
$$

- The $h^2$ in $(1-n)h^2$ is the geometric (weak) focusing.
- $-n h^2$ and $+n h^2$ come from the quadrupole component and have opposite signs in the two planes (again $k_x + k_y = 0$, with the geometric term added only horizontally).
- The $h\,\delta$ on the right is the dispersion source term: a particle with more momentum has a larger radius and moves outwards. Its particular solution is the dispersion function $D(s)$, with $D'' + (1-n)h^2 D = h$.

From Maxwell's equation $\partial B_x/\partial y = \partial B_y/\partial x$, a horizontal gradient necessarily comes with $B_x = -n B_0\,y/\rho$. The geometric results of steps 3–5 are exactly the homogeneous and particular solutions of this equation for $n = 0$.

## Going deeper: the sector-magnet transfer matrix and path length {#dipole-deep-matrix}

The previous section gave a particle's position and angle after a dipole. This section collects them into a "conversion table" (transfer matrix) like that of the [Drift](#drift-deep-matrix), and shows how dispersion propagates downstream and why outer particles fall behind.

### Step 1: three cases in one table

A general particle has an offset $x_0$, an angle $x_0'$ and a momentum deviation $\delta$ at once. The motion is linear (for small offsets the parts do not disturb each other), so steps 3–6 of the previous section simply add:

$$
\begin{pmatrix} x \\ x' \\ \delta \end{pmatrix} =
\begin{pmatrix}
\cos\theta & \rho\sin\theta & \rho(1-\cos\theta)\\
-\sin\theta/\rho & \cos\theta & \sin\theta\\
0 & 0 & 1
\end{pmatrix}
\begin{pmatrix} x_0 \\ x_0' \\ \delta \end{pmatrix}.
$$

The last row says $\delta$ does not change (a magnetic field does no work, so the size of the momentum stays the same). The entries are numbered by row and column: $R_{11} = \cos\theta$, $R_{12} = \rho\sin\theta$, $R_{21} = -\sin\theta/\rho$; in the full 6 × 6 table the third column is called $R_{16}$, $R_{26}$ (the sixth coordinate is $\delta$). For the animation's 30°, 0.5 m magnet: $R_{11} = 0.866$, $R_{12} = 0.25$ m, $R_{21} = -1.0$ m⁻¹, $R_{16} = 0.067$ m, $R_{26} = 0.5$.

### Step 2: dispersion is carried along like an orbit

The **dispersion function** $D(s)$ is "the offset from the reference orbit of a particle with $\delta = 1$". The elements downstream convert it like any particle:

$$
\begin{pmatrix} D \\ D' \\ 1 \end{pmatrix}_1 =
\begin{pmatrix} R_{11} & R_{12} & R_{16} \\ R_{21} & R_{22} & R_{26} \\ 0 & 0 & 1 \end{pmatrix}
\begin{pmatrix} D \\ D' \\ 1 \end{pmatrix}_0 .
$$

Coming from a dispersion-free straight section ($D_0 = D'_0 = 0$), the exit values are $D = R_{16}$, $D' = R_{26}$. A drift of length $\ell$ has the table $\begin{pmatrix} 1 & \ell & 0 \\ 0 & 1 & 0 \\ 0 & 0 & 1\end{pmatrix}$; one matrix multiplication gives $D_1 = D + \ell D'$. Animation: $0.067 + 0.5\times 0.5 = 0.317$ m, i.e. 3.17 mm per 1 % of momentum; the 45° line of the main text: $0.293 + 0.5\times 0.707 = 0.646$ m.

### Step 3: why sizes add in squares

Each particle's offset is $x = x_\beta + D\delta$, with $x_\beta$ the part without dispersion. Square it and average over all particles:

$$
\langle x^2\rangle = \langle x_\beta^2\rangle + 2D\,\langle x_\beta\,\delta\rangle + D^2\langle\delta^2\rangle .
$$

When the beam enters the bending section its transverse position is unrelated to its momentum, so the middle term averages to zero and $\sigma_x^2 = \sigma_{x,\beta}^2 + D^2\sigma_\delta^2$. In Twiss parameters (see [Drift](#drift-deep-matrix)) $\sigma_{x,\beta}^2 = \beta_x\varepsilon_x$:

$$
\sigma_x^2 = \beta_x\,\varepsilon_x + D^2\,\sigma_\delta^2 .
$$

### Step 4: path length: outer particles fall behind

Dispersion also has a longitudinal side. Let $z$ be a particle's longitudinal position relative to the reference particle ($z > 0$ ahead). A particle at $x$ on the outside goes round a circle larger by $x$; for the same angle it travels $x\,\Delta\theta = x\,\Delta s/\rho$ further, so for every $\Delta s$ it falls behind by $h\,x\,\Delta s$. On the other hand, a particle with more momentum is faster and gains $\delta\,\Delta s/\gamma^2$ per $\Delta s$ (this term is present in a drift too, see [Drift](#drift-deep-longitudinal)). Together the rate of change of the longitudinal position is

$$
z' = -h\,x + \frac{\delta}{\gamma^2}.
$$

For a particle with $\delta$, $x = D\delta$. Adding up $-hD$ along the dipole ($D = \rho(1-\cos\theta)$ grows from 0 to its exit value) gives for the 45° sector magnet

$$
R_{56} = \frac{L}{\gamma^2} - \rho\,(\theta - \sin\theta) = 0.783 - 0.078 = 0.705\ \mathrm{m}.
$$

The first term (velocity difference) is much larger than the second (path difference): for low-energy protons a bend still behaves longitudinally mostly like a drift that debunches (see [Longitudinal motion](#longitudinal)). $R_{51}$, $R_{52}$ describe how the transverse coordinates at the entrance affect the path: a particle displaced outwards at the entrance ($x > 0$) travels further and its $z$ decreases.

### Step 5: the general form with a field index

Let $K_x = (1-n)h^2$, $K_y = n h^2$, and let $C_x, S_x$ be the cosine- and sine-like solutions of $u'' + K_x u = 0$ ($C_x = \cos\sqrt{K_x}L$, $S_x = \sin(\sqrt{K_x}L)/\sqrt{K_x}$, replaced by hyperbolic functions for $K < 0$). For $n = 0$, $\sqrt{K_x}L = hL = \theta$ and we are back at step 1. With $x$ as the bending plane, the $6\times 6$ sector-magnet matrix is

$$
M_{\text{bend}} =
\begin{pmatrix}
C_x & S_x & 0 & 0 & 0 & R_{16}\\
-K_x S_x & C_x & 0 & 0 & 0 & R_{26}\\
0 & 0 & C_y & S_y & 0 & 0\\
0 & 0 & -K_y S_y & C_y & 0 & 0\\
R_{51} & R_{52} & 0 & 0 & 1 & R_{56}\\
0 & 0 & 0 & 0 & 0 & 1
\end{pmatrix},
$$

with

$$
\begin{aligned}
R_{16} &= \frac{h\,(1-C_x)}{K_x}, &
R_{26} &= h\,S_x,\\
R_{51} &= -h\,S_x, &
R_{52} &= -\frac{h\,(1-C_x)}{K_x},\\
R_{56} &= \frac{L}{\gamma^2} - \frac{h^2\,(L - S_x)}{K_x}.
\end{aligned}
$$

This is the sector-magnet matrix used by the AVAS linear envelope preview. For a uniform sector magnet ($n = 0$), $\theta = L/\rho$:

$$
R_{16} = \rho\,(1-\cos\theta),\quad
R_{26} = \sin\theta,\quad
R_{56} = \frac{L}{\gamma^2} - \rho\,(\theta - \sin\theta),
$$

and the vertical plane is a drift of length $L$: exactly what the circles of the previous section gave. Numbers are in the table of [In AVAS](#dipole-in-avas).

### For readers who know linear algebra

$R_{51}$, $R_{52}$ are tied to $R_{16}$, $R_{26}$ by the symplectic condition $M\tp J M = J$ and are not independent: $R_{51} = R_{21}R_{16} - R_{11}R_{26}$, $R_{52} = R_{22}R_{16} - R_{12}R_{26}$. The transverse 2 × 2 blocks have determinant 1, so a dipole preserves emittance; dispersion merely correlates $\delta$ with $x$, and an achromatic structure can undo that correlation.

## Going deeper: the thin lens of a rotated pole face {#dipole-deep-edge}

The main text said that rotating the pole faces by $\beta$ is like adding a thin quadrupole lens at each end. This section puts numbers on it and works out the rectangular magnet.

### Step 1: the horizontal direction

With the pole face rotated by $\beta$, a particle at distance $x$ from the reference orbit meets the pole face shifted along its path by $x\tan\beta$ (right triangle: opposite = adjacent × tangent). For $\beta > 0$ an outer particle travels $x\tan\beta$ less in the field and turns $x\tan\beta/\rho = h\,x\tan\beta$ less. Turning less towards the centre means being deflected outwards by that much more:

$$
\Delta x' = +h\tan\beta\;x .
$$

The deflection is proportional to $x$: a thin lens of focal length $-1/(h\tan\beta)$ (the minus sign meaning defocusing), i.e. $R_{21} = h\tan\beta$.

### Step 2: the vertical direction

At a tilted edge the fringe field decays along the normal of the pole face. Because the field "does not swirl" where there is no current ($\nabla\times\boldsymbol{B} = 0$), off the median plane at height $y$ there is a horizontal component along the pole-face normal, proportional to $y$. With the pole face tilted, part of this horizontal component is perpendicular to the particle's velocity and gives a vertical force: for $\beta > 0$ it pushes the particle back to the median plane. For a hard edge (fringe-field width zero) it is as large as the horizontal effect with the opposite sign, $R_{43} = -h\tan\beta$: once more $k_x + k_y = 0$.

### Step 3: correction for a fringe field of finite width

In the TRANSPORT / TraceWin form:

$$
M_{\text{edge}}:\qquad
R_{21} = h\tan\beta,\qquad
R_{43} = -h\tan(\beta - \psi),
$$

with all other entries as in the unit matrix. The correction angle due to the finite width of the fringe field is

$$
\psi = K_1\,G\,h\,\frac{1 + \sin^2\beta}{\cos\beta}\,\left(1 - K_1 K_2\,G\,h\tan\beta\right),
$$

where $G$ is the full magnet gap and $K_1$, $K_2$ are integral factors describing the fringe-field shape. $K_1 = 0$ gives $\psi = 0$, the hard-edge model. This is the pole-face edge matrix used by the AVAS linear envelope preview. A fringe field of finite width weakens the vertical focusing: for $\beta = 22.5^\circ$, $\rho = 1$ m, $G = 0.05$ m, $K_1 = 0.45$, $K_2 = 2.8$, $\psi = 1.56^\circ$ and the vertical focusing of one edge drops from $\tan 22.5^\circ = 0.414$ m⁻¹ to 0.383 m⁻¹, about 8 % weaker.

### Step 4: the rectangular magnet

A 45°, $\rho = 1$ m sector magnet with hard edges of $\beta = 22.5^\circ$ at both ends:

- Horizontally: two defocusing thin lenses $h\tan 22.5^\circ = 0.414$ m⁻¹ plus the sector magnet's own $R_{21} = -0.707$ m⁻¹; multiplying the three tables gives $R_{11} = 1$, $R_{21} = 0$: the weak focusing is cancelled completely.
- Vertically: two focusing thin lenses with a 0.785 m drift between them, $R_{43} = -0.414 - 0.414 + 0.414\times 0.785\times 0.414 = -0.694$ m⁻¹ (the two-lens ray tracing of the [Quadrupole](#quadrupole-deep-fodo) chapter).
- The exit dispersion is unchanged ($D = 0.293$ m), $D' = 0.828$.

With the fringe-field correction above, vertically $R_{43} = -0.650$ m⁻¹.

### For readers who know vector calculus

In the TRANSPORT convention, $\beta > 0$ corresponds to a pole face that shortens the outer particles' path in the field. In the edge region, $\nabla\times\boldsymbol{B} = 0$ requires a component along the pole-face normal off the median plane, $B_n \approx y\,\partial B_y/\partial n$; its projection onto the horizontal direction perpendicular to the velocity is a fraction $\sin\beta$, and integrating across the edge gives $\Delta y' = -h\tan\beta\;y$. The correction angle $\psi$ comes from an integral expansion over the shape of the finite-width fringe field ($K_1$, $K_2$ are two of these integrals; see Brown, SLAC-75).

## Going deeper: dispersion, space charge and nonlinearity {#dipole-deep-dynamics}

### Dispersion and energy-spread coupling

A dipole converts a longitudinal momentum deviation into a transverse position. Every downstream element "sees" a beam size that includes $D\sigma_\delta$; if the dispersion is not cancelled at the exit of the bending section, it propagates along the transport line and, wherever there is acceleration or space charge, causes transverse–longitudinal coupling and effective emittance growth.

### Space charge

In a dispersive region the space-charge force depends on the total beam size including dispersion, and it also changes the dispersion itself (the dispersion function is defocused by space charge too); at high current the achromat condition has to be retuned with the current present. The multi-particle tracking of the AVAS engine includes these effects; the linear preview only gives estimates at zero current or with linear space charge. The animation in this chapter has no space charge.

### Nonlinearity

For large momentum deviations both dispersion and path length have second-order terms ($T_{166}$, $T_{566}$, …), and fringe fields and saturation add sextupole components; the matrix model is first order only. When a dipole is described by a field map, the linear preview gives a warning (the dipole component of field maps is not modelled); the engine is the reference.

## In AVAS {#dipole-in-avas}

### Keywords

In AVAS a dipole consists of two keywords: the sector-magnet body `bend` and the pole-face edge `edge`.

```text
edge 0 R 0 beta rho G K1 K2 HV
bend arc R 0 alpha rho N HV
edge 0 R 0 beta rho G K1 K2 HV
```

```avas-params
bend edge
```

`bend`:

- `arc`: arc length $\lvert\alpha\rho\rvert$ (m); writing 0 is enough, the program computes it from $\alpha$ and $\rho$.
- `R`: aperture radius (m); the third parameter is reserved, write 0.
- `alpha`: bend angle (°), in the plane chosen by HV.
- `rho`: radius of curvature of the central orbit (m).
- `N`: field index $n$.
- `HV`: bending plane, 0 = horizontal ($x$), 1 = vertical ($y$). With HV = 1 the roles of $x$ and $y$ in the [transfer matrix](#dipole-deep-matrix) are swapped: the dispersion appears in the $y$ plane, $K_y = (1-n)h^2$, $K_x = nh^2$.

`edge`:

- The first and third parameters are reserved, write 0; `R` is the aperture.
- `beta`: pole-face rotation angle $\beta$ (°).
- `rho`: radius of curvature (m), should equal that of the adjacent `bend`.
- `G`: full magnet gap (m).
- `K1`, `K2`: fringe-field factors; `K1 = 0` is a hard edge.
- `HV`: same as the adjacent `bend`.

> **AVAS convention**: the linear preview treats `bend` as a sector-magnet body without pole-face angles; pole-face rotation is given by one `edge` before and one after (the same habit as TraceWin's BEND/EDGE). Whether the engine's `bend` itself includes an edge effect, or must be paired with `edge`, the manual does not say; the engine is the reference. Both `bend` and `edge` are z-code matrix-model elements; `edge` has zero length.

> **Bending direction**: in the linear preview (and in this chapter's animation and field schematics), $\alpha > 0$ bends a positive particle towards $-x$, with $x$ pointing to the outside of the bend and $R_{16} > 0$; a negative $\alpha$ flips the sign of $h$, and $R_{16}$ and the others change sign with it, i.e. the beam bends the other way. The engine does the same (not stated in the manual; checked with the engine on 2026-11-15, the new kernel's default and compatibility modes agreeing): a positive $\alpha$ puts the higher-momentum particles at $x > 0$ after the dipole, so $+x$ is the outside and the beam turns towards $-x$ ($-y$ for a vertical bend); the 3D view draws it this way.

### Example

A 45° rectangular dipole, $\rho = 1$ m, horizontal bend, 5 cm gap:

```text
drift 0.000001 0.03 0
edge 0 0.03 0 22.5 1.0 0.05 0.45 2.8 0
bend 0 0.03 0 45 1.0 0 0
edge 0 0.03 0 22.5 1.0 0.05 0.45 2.8 0
drift 0.3 0.03 0
```

$K_1 = 0.45$, $K_2 = 2.8$ are typical values often quoted in the TraceWin documentation; the exact values depend on the pole shape and should be fitted from measurements or 3D field calculations.

### The z coordinate in bends

Without dipoles, $z$ in DataSet.txt = column 5 + column 33. With dipoles, the AVAS interface accumulates the path step by step in bending steps (column 35 marks them) and keeps accumulating afterwards; $z$ in the interface is therefore the **path length** $s$ along the central orbit, not the straight-line distance in the laboratory frame.

### Typical values

| Quantity | Typical range (low-energy transport line) | Note |
|---|---|---|
| Field $B$ | 0.1–1.5 T | room-temperature iron magnets; set by $B\rho$ and $\rho$ |
| Radius of curvature $\rho$ | 0.3–2 m | |
| Bend angle $\alpha$ | 10°–90° | large angles are often split into several magnets |
| Field index $n$ | 0; 0.5 common for analysing magnets | 0 < n < 1: weak focusing in both planes |
| Pole-face angle $\beta$ | 0 to about $\alpha/2$ | used to balance the focusing of the two planes |
| Gap $G$ | 0.03–0.1 m | |

### Numerical example: 45° sector magnet

Example beam ($\gamma = 1.00163$), $\rho = 1$ m, $\alpha = 45^\circ$, $L = 0.7854$ m, $B = 0.1786$ T:

| | $n = 0$ | $n = 0.5$ |
|---|---|---|
| $R_{11}$, $R_{21}$ | 0.7071, −0.7071 m⁻¹ | 0.8497, −0.3728 m⁻¹ |
| $R_{33}$, $R_{43}$ | 1, 0 | 0.8497, −0.3728 m⁻¹ |
| $R_{16}$ (exit $D$) | 0.2929 m | 0.3006 m |
| $R_{26}$ (exit $D'$) | 0.7071 | 0.7456 |
| $R_{56}$ | 0.7046 m | 0.7033 m |

Some observations:

- A 1 % momentum deviation gives a 2.9 mm position offset at the exit, large compared with the example beam's initial rms size (about 1 mm). The example beam's rms momentum spread is $\sigma_\delta = 3.5\times 10^{-3}$, so the dispersive contribution at the exit is $D\sigma_\delta \approx 1.0$ mm; the betatron part of the opening beam line at the exit is about 3.0 mm, and adding in squares, the dispersion increases the rms size at the exit by only about 0.15 mm: its share grows downstream as $D + D's$ (2.3 mm at the end of the drift).
- In $R_{56}$ the drift term $L/\gamma^2 = 0.783$ m is much larger than the bending term $\rho(\theta - \sin\theta) = 0.078$ m: for low-energy protons a bend still behaves longitudinally mostly like debunching.
- With two hard edges of $\beta = 22.5^\circ$ (rectangular magnet), horizontally $R_{21} = 0$, $R_{11} = 1$, vertically $R_{43} = -0.694$ m⁻¹; the exit dispersion is unchanged ($D = 0.293$ m), $D' = 0.828$.
- With the fringe-field correction ($G = 0.05$ m, $K_1 = 0.45$, $K_2 = 2.8$), $\psi = 1.56^\circ$, and the vertical focusing of one edge drops from $\tan 22.5^\circ = 0.414$ m⁻¹ to 0.383 m⁻¹, about 8 % weaker.
- For $n = 0.5$ the matrices of the two planes are identical: "both planes focus equally".

Defaults of this chapter's animation (30°, $\rho = 0.5$ m, a 0.15 m drift before and a 0.5 m drift after, no space charge): $B = 0.357$ T, arc length 0.262 m, offset per 1 % of momentum at the end 3.17 mm, rms $x$ at the end 2.69 mm (betatron part 2.44 mm), rms $y$ 3.39 mm.

### Common problems

- The design has pole-face angles but no `edge` lines: the linear preview gives a pure sector magnet with no vertical focusing, and the beam may be much larger vertically than expected (how the engine handles `bend` edges is up to the engine, see the convention above).
- The `rho` or `HV` of an `edge` does not match the `bend`.
- The `arc` of `bend` written as an angle or in the wrong unit; writing 0 is simplest.
- Comparing positions in a bend with laboratory coordinates: $z$ in the interface is path length.

## Further reading {#dipole-reading}

- K. L. Brown, "A First- and Second-Order Matrix Theory for the Design of Beam Transport Systems and Charged Particle Spectrometers", SLAC-75 (1967 and later revisions): matrices of sector magnets, pole-face rotation and fringe-field corrections (TRANSPORT convention).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (bending magnets, dispersion function and achromats).
- S. Y. Lee, *Accelerator Physics*, 4th ed. (weak focusing, field index, dispersion).
- Related chapters: [Quadrupole](#quadrupole), [Longitudinal motion](#longitudinal), [Lattice design](#lattice-design), [Matching](#matching), [High-energy transport](#journey-hebt); full bibliography in [References](#references).
