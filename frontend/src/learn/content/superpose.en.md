# Superposed fields

In an AVAS lattice, elements follow one another like the carriages of a train: each starts at the exit of the previous one and has its own stretch of $z$ to itself. A real accelerator is not laid out like that. A magnet's field does not stop abruptly at the end face of the iron; it reaches out some distance and fades away, and this part is called the **fringe field**. The fringe field of a superconducting solenoid reaches into the RF cavity right next to it; some solenoids even have a small set of correction coils wound inside them, so that two fields occupy the very same space.

That raises three questions:

1. How can two elements occupy the same stretch of space?
2. Can the values of two field maps (data tables of the field at points in space, see [Field maps](#field-maps)) simply be added?
3. When is adding them wrong?

This chapter answers them with a fact from school: **when several fields are present at once, the force on a particle is the sum of the forces each field exerts on its own**, provided the elements do not change each other's fields. On that basis it explains how AVAS's `superpose` command positions each element, what superposition actually changes in the example, and the difference between `superposeend` and `superposeout`. The physics of the field-map elements themselves is in [RF cavity](#rf-cavity) and [Static field maps](#static-fields).

Below is the manual's example superposition group: the solenoid sol (a static field map, 0.35 m) and the RF cavity hwr010 (0.21 m) overlapping by 5 mm. Open the 2D view: the two overlapping elements are drawn in two layers, one above the other (the layers are only schematic and do not mean a transverse offset). Now look at the curves: the block starts at $z = 0.085\ \mathrm{m}$ and ends at 0.64 m; the solenoid focuses the beam, which is narrowest at $z \approx 0.42\ \mathrm{m}$ downstream of the solenoid (this is called the **waist**, rms x 0.27 mm); the cavity raises the kinetic energy from 1.527 MeV to 1.866 MeV; at the end (0.94 m) rms x is 5.35 mm.

```avas-beamline
drift 0.085 0.02 0
superpose 0 0 0 0 0 0
field 0.35 0.02 0 3 0 0 1 0.531 sol
superpose 0.345 0 0 0 0 0
field 0.21 0.02 0 1 162.5e6 -33 1.36 -1.36 hwr010
superposeend
drift 0.3 0.02 0
```

## Why let fields overlap {#superpose-role}

Putting a field map into the lattice means saying "from here to here, the particle feels the field in this map". If the fields of two elements overlap in space but we still place them end to end, only two choices remain:

- Cut both field maps at the boundary, throwing away the part of each fringe field that reaches into the other. If the field is still sizeable at the cut, the particle jumps there from "field" to "no field" and receives a kick that does not exist in reality (see [the ends of a field map](#field-maps-grid)).
- Accept that they overlap and let both maps act at once in the overlap region.

`superpose` is the second choice. Typical uses:

- **Solenoids and RF cavities**: in the superconducting section at low speed (particles at a few to some twenty per cent of the speed of light, called the low-β section), solenoids sit right next to cavities, and the fringe of the solenoid map overlaps the cavity map. The example in the manual and the case tutorial is exactly the superposition of the solenoid map `sol` and the cavity map `hwr010`.
- **Magnets with correction coils**: a solenoid or quadrupole with a dipole correction coil inside it (a small coil that tilts the whole beam by a little, see [Steerer](#steerer)); the coil's field map can be superposed on the main magnet's. This is the most complete overlap: both fields occupy the same space.
- **Field maps with long fringes**: placing two maps end to end forces each fringe to be cut at the boundary; superposition lets both maps keep their full fringe fields, which add in the overlap region.

When superposition is not needed: to put a steerer in a matrix-model quadrupole, simply write `steerer` before the `quad`. A steerer has zero length and by default acts over the full length of the next element, centred on it; see [Steerer](#steerer).

## The physics: forces add {#superpose-physics}

### The superposition principle from school

School physics has two familiar rules:

- **Adding forces**: several forces acting on a body together have the same effect as their vector sum (the parallelogram rule).
- **Superposition of fields**: the electric field of two point charges at a point is the vector sum of the fields each produces alone; the same holds for the magnetic field around two current-carrying wires.

Put together: a particle inside both the solenoid's magnetic field and the cavity's electric field feels "the solenoid's force on it alone" plus "the cavity's force on it alone". So to get the total effect we only need to add the two maps' values at the same position.

Each map is first put in its own place (the next section shows how the positions are worked out) and multiplied by its own scale factor: electric fields by Ke, magnetic fields by Kb; RF fields are also multiplied by their own oscillating time factor (cos for the electric field, sin for the magnetic field, see [RF cavity](#rf-cavity-in-avas)). The AVAS engine tracks particles through the total field with its t-code (tracking that integrates the particle motion step by step through the field). The full formula is in [going deeper](#superpose-deep-linearity).

### When fields cannot be added

The superposition principle has a precondition that is easy to miss: **each field must be computable on its own, and the fields must not affect each other when put together**. The two maps were computed (or measured) separately: when the solenoid's field was computed there was no cavity next to it, and when the cavity's field was computed there was no solenoid. If, once put together, they really change each other, adding is wrong:

- **Iron gets magnetised.** Many magnets guide their field lines with an iron yoke, and how strongly iron is magnetised depends on the total field it sits in. When two iron magnets are very close, the field of one changes the magnetisation of the other's iron, and iron also saturates (beyond a certain field it cannot help any more). Then the sum of two separately computed maps is not the real total field.
- **Superconducting cavity walls keep magnetic fields out.** A superconductor, once cooled, expels a magnetic field applied afterwards (the Meissner effect; flux present during cool-down may instead be "frozen" into the niobium wall). So the real stray field of a solenoid inside a superconducting cavity differs from the separately computed solenoid map.

When accuracy matters, the combined field of several elements should be computed together in one electromagnetic calculation and used as a single field map.

There are also two rules and one fact:

- **At most one RF cavity per superposition group.** Different RF cavities have their own frequency and phase; AVAS allows at most one RF cavity in a superposition group, while the number of static fields is not limited. The manual does not give the reason for this rule; cavities whose fields clearly overlap cannot be described with AVAS superposition.
- **A static magnetic field does not change the energy.** The magnetic force is always perpendicular to the velocity and does no work (the school statement "the Lorentz force does no work"). So when a static magnetic field is superposed on an RF field, it does not directly change the energy gain; it changes the transverse motion inside the cavity.

## Longitudinal positions {#superpose-z}

### The block's zero

A superposition block starts at the first `superpose`. If the lattice has reached $z_b$ at that point (the total length of all elements before the first superpose), the $i$-th element of the block occupies

$$
z \in \bigl[\,z_b + z_{0,i},\ \ z_b + z_{0,i} + L_i\,\bigr]
$$

where $L_i$ is the element's length and $z_{0,i}$ is the first parameter of the `superpose` just before it, i.e. "the longitudinal position of this element's entrance relative to the entrance of the element after the first superpose". The first superpose must have all parameters zero; it defines the zero. In other words, positions inside a block no longer "follow on from the previous element"; every element is placed relative to the same zero, like marking each one's starting point on the same ruler.

### Position after the block

The AVAS interface (lattice page, 2D layout, linear preview) takes the block's length as the farthest exit of the elements in it:

$$
z_{\text{after}} = z_b + \max_i\bigl(z_{0,i} + L_i\bigr)
$$

Elements after `superposeend` continue from $z_{\text{after}}$. The manual does not state the engine's rule for positions after a block explicitly; before a production run, check the total length against the positions in DataSet.txt (column 5 plus column 33) or the beam-line plot.

### Example

The lattice from the manual and the case tutorial:

```text
start
drift      0.085  0.02   0
superpose  0 0 0 0 0 0
field      0.35   0.02     0   3   0   0   1    0.531  sol
superpose  0.345 0 0 0 0 0
field      0.21   0.02     0   1   162.5e6   -33   1.36    -1.36   hwr010
superposeend
end
```

| Element | $z_0$ / m | Occupies / m |
|---|---|---|
| drift | — | 0 – 0.085 |
| sol (static magnetic) | 0 | 0.085 – 0.435 |
| hwr010 (RF) | 0.345 | 0.430 – 0.640 |
| after the block | — | from 0.640 |

Check: the cavity starts at $0.085 + 0.345 = 0.430$ m and ends at $0.430 + 0.21 = 0.640$ m; the solenoid ends at $0.085 + 0.35 = 0.435$ m. So the last 5 mm of the solenoid map overlap the first 5 mm of the cavity map, and both maps keep their fringes intact. Writing $z_0 = 0.35$ (= the solenoid length) puts the two elements exactly end to end, equivalent to writing two `field` lines one after the other without superposition; in the linear preview the two forms give identical results.

> **Note**: in the manual's original example the second superpose has 7 numbers (`superpose 0.345 0 0 0 0 0 0`), one 0 too many; superpose has only 6 parameters, see [In AVAS](#superpose-in-avas).

## What superposition changes {#superpose-effect}

### What the cavity does on its own

To understand what superposition changes, we first need to know what the cavity does to the beam on its own. The animation below is exactly the example's cavity map hwr010, placed by itself after a 5 cm drift and followed by a 0.2 m drift; the beam is the example beam (synchronous phase −30°, Ke = 1.36, 0.184 mA).

```avas-motion
cavity
```

Click "3D" and follow the bunch through: the cavity has two accelerating gaps, centred about 7 cm and 14 cm after the cavity entrance (the animation draws its field lines there); the fields in the two gaps point in opposite directions, and the field lines with small cones flip and grow and shrink with the RF phase. After the cavity the kinetic energy is 0.357 MeV higher.

Now the upper 2D picture (horizontal direction $x$): the beam grows in the cavity and after it, and at the end rms x is 3.0 mm. Drag Ke to 0 and the cavity becomes an empty pipe: only 2.1 mm at the end. So while accelerating, this cavity also pushes the beam apart transversely like a **defocusing lens**. Note that the example beam is already growing (diverging) as it enters the cavity, so the cavity's push adds to angles that already point outwards. We shall see below that this is the key to what changes in the superposition example.

What this animation shows is the cavity's two effects: **acceleration** and **transverse defocusing**. It does not show superposition itself: the animation contains only the cavity, no solenoid, and no adding of two maps; the beam has not been focused into a waist by a solenoid before the cavity; the synchronous phase is −30°, not the example's −33°, so the energy gain (0.357 MeV) differs from the example's 0.339 MeV. Also, the animation draws field lines only at the two gaps and does not show how weak the field is at the ends of the map; for that, see the numbers below.

### How much field is in the overlap

Count how much field there is in the manual example's 5 mm overlap: on axis, $B_z$ in the last 5 mm of the `sol` map is only about 0.25 % of its peak; this stretch holds only about 0.01 % of the total $\int B_z\,\dd z$ (the sum of $B_z$ along the axis) and $4\times 10^{-7}$ of $\int B_z^2\,\dd z$ (the quantity that sets a solenoid's focusing strength); the first 5 mm of `hwr010` hold only 0.04 % of the cavity's $\int\lvert E_z\rvert\,\dd z$. Both maps are practically zero in the overlap, exactly as required of field-map ends ("decayed to negligible"). So what does superposition change in this example?

### A controlled comparison

A comparison (linear preview, no space charge): change only the $z_0$ of the superpose before the cavity, and adjust the length of the drift after the block so that the total length is always 0.945 m and the end is compared at the same place. Then compute each case again with the cavity field switched off (Ke = Kb = 0):

| $z_0$ / m | Overlap with the solenoid | Cavity occupies / m | Exit energy / MeV | rms x at the end / mm | Cavity off / mm |
|---|---|---|---|---|---|
| 0.35 | none (same as end to end) | 0.435 – 0.645 | 1.8665 | 5.46 | 4.87 |
| 0.345 | 5 mm | 0.430 – 0.640 | 1.8665 | 5.42 | 4.88 |
| 0.30 | 50 mm | 0.385 – 0.595 | 1.8665 | 4.94 | 4.90 |
| 0.25 | 100 mm | 0.335 – 0.545 | 1.8665 | 4.22 | 4.86 |

Three things stand out:

- **The exit energy hardly changes** (by less than 0.1 keV): a static magnetic field does no work; the cavity specifies its synchronous phase with V3 = 0, and the engine (and the preview) solves the entrance phase again for the new position (52.67° in all four cases), so the bunch is always accelerated at the same phase.
- **The solenoid's tail itself does almost nothing**: the last column (solenoid only) stays between 4.86 and 4.90 mm; even with 100 mm of overlap the tail holds only 0.4 % of $\int B_z^2$.
- **The change comes from where the cavity sits relative to the waist**: the smaller $z_0$, the closer the cavity is to the waist made by the solenoid ($z \approx 0.42$ m).

### Where a lens sits matters

As the animation showed, an RF cavity besides accelerating pushes the beam apart transversely like a defocusing lens (the reason is in [RF cavity](#rf-cavity-transverse)). At such low speed most of the defocusing comes from the first gap (about 6.5–7 cm after the cavity entrance).

What a lens does depends strongly on how wide the beam is where it sits. A lens deflects each particle in proportion to its distance from the axis; at a waist all particles are close to the axis, so the deflections are small. Therefore:

- With $z_0 = 0.35$, rms x at the cavity centre is about 1.2 mm and the first gap is about 8 cm **downstream** of the waist. The beam is already growing (diverging), as in the animation; the outward push adds to angles that already point outwards, and the beam is wider at the end: 5.46 mm, against 4.87 mm without the cavity.
- With $z_0 = 0.25$, rms x at the cavity centre is only about 0.3 mm and the first gap is about 2 cm **upstream** of the waist. The beam is still shrinking (converging), and the same outward push cancels part of the converging angles, so the particles' angles after the cavity are more orderly. Add the roughly 10 % reduction of the angles by acceleration (**adiabatic damping**: the transverse momentum stays the same while the longitudinal momentum grows, so the angles shrink), and the beam at the end is even narrower than without the cavity: 4.22 mm against 4.86 mm.

An estimate treating the cavity as two thin lenses reproduces this trend (5.14, 5.10, 4.65, 4.07 mm for the four rows). The step-by-step derivation of "why a lens at the waist does almost nothing, and why the sign flips before and after the waist" is in [going deeper](#superpose-deep-waist).

So in the manual's example the point of `superpose` is mainly to **put the cavity in its true position** while keeping both maps' fringes complete, not the field in the overlap itself. The overlap field really matters when a correction coil sits inside a main magnet, or when fringe fields are long.

## Think about it {#superpose-think}

> **Think 1**: Each of two field maps obeys "field lines are never created or destroyed". Does their sum obey it too? Is there a hidden condition?
>
> Answer: yes. The number of field lines entering a small box equals the number leaving it for each map, and so also for the sum. The hidden condition is that each map describes the field in **the same vacuum** in the overlap region. If the conductors, yoke or coils of one element lie inside the other element's field, that region is not vacuum, and the separately computed map is not the correct field there.

> **Think 2**: Why does a lens placed at a waist hardly change the beam size downstream?
>
> Answer: a lens deflects each particle in proportion to its distance from the axis. At a waist all particles are close to the axis, so the deflections are small; the size downstream is set by the particles' angles at the waist, which the lens hardly touches.

> **Think 3**: Changing the cavity's $z_0$ in the example from 0.345 to 0.35 changes almost nothing; changing it to 0.25 makes the beam 1.2 mm narrower at the end. Is that a contradiction?
>
> Answer: no. The first only changes a 5 mm overlap with almost no field in it; the second moves the cavity 10 cm closer to the waist, changing how wide the beam is where the cavity sits and whether it is converging or diverging there.

## Pitfalls {#superpose-pitfalls}

- **"Superposition means adding the map values, and that is always right"**: only for fields computed separately in linear media; iron saturation, mutual magnetisation and the Meissner effect of superconducting walls cannot be approximated by superposition, see [the physics](#superpose-physics).
- **"The more overlap, the more superposition matters"**: in the example the 5 mm overlap contains almost no field; superposition changes the result mainly because the element's position changes, see [what superposition changes](#superpose-effect).
- **A non-zero first line**: if the first superpose has a non-zero $z_0$, the whole block is shifted wrongly; it must be all zeros.
- **A missing superpose**: every element in the block needs one in front of it, and two in a row are not allowed either.
- **Forgetting to close**: without superposeend / superposeout all following elements are treated as part of the block.
- **Two RF cavities**: cavities whose fields overlap cannot go into the same block. If only the tails of the fringes overlap slightly, the maps can be cut where the field has decayed to negligible and placed end to end (the field at the cut must be small enough, see [the ends of a field map](#field-maps-grid)).
- **Getting the block length wrong**: the elements after a block start at the farthest exit inside the block, not at the exit of the last element; it is advisable to write the elements in a block in order of $z$.
- **Offsets that are ignored**: when a block ends with superposeend, $x_0$, $y_0$ and the angles have no effect; offsets need superposeout.

## Going deeper: why fields can be added, and when not {#superpose-deep-linearity}

The main text used "adding forces" and "superposition of fields" to argue that field maps can be added. This section goes step by step through why that holds and which step fails in iron and in superconductors; the last paragraph is for readers who know electromagnetism.

### Step 1: forces add

Newton's second law only cares about the net force on a body. A particle feels the electric force $q\boldsymbol{E}$ and the magnetic (Lorentz) force at once, and the net force is their vector sum. With two elements giving forces $\boldsymbol{F}_1$ and $\boldsymbol{F}_2$ on their own, the net force is $\boldsymbol{F}_1 + \boldsymbol{F}_2$ as long as these forces do not change when the elements are put together. So the question becomes: does each element's field change when the two are put together?

### Step 2: a coil's field is proportional to its current

From school, the field inside a long solenoid is $B = \mu_0 n I$ ($n$ the turns per unit length, $I$ the current), and the field around a straight wire is proportional to its current too. Double the current and the field doubles; run current through two coils at once and the field is the sum of their fields. This relation, "double the cause, double the effect; add the causes, add the effects", is called **linearity**. Electric and magnetic fields in vacuum are linear, so separately computed fields can be added.

### Step 3: the sum is still a "legal" field

Magnetic fields obey an iron law: field lines are never created or destroyed, and the number entering any small box equals the number leaving it (see [Quadrupole](#quadrupole-deep-field)). Each map obeys it; adding the numbers entering and the numbers leaving separately, the equality still holds. So the sum also obeys the law and is not a physically impossible field.

### Step 4: why iron breaks linearity

Iron works by becoming magnetised and producing a field of its own, helping the coil to guide the field lines where they are needed. But iron's magnetisation is not proportional to the applied field: for small applied fields it grows quickly, and around 1–1.5 T the iron begins to **saturate**: a stronger applied field gets little extra help from it. So the field of an iron-yoke magnet is not strictly proportional to its current. When two iron-yoke magnets are close, the field of one changes how strongly the other's iron is magnetised, and "compute separately, then add" is no longer "compute together".

### Step 5: why superconductors break superposition

Superconducting cavities are made of niobium; once cooled into the superconducting state it expels a magnetic field applied after cool-down (the Meissner effect), while flux present during cool-down may be "frozen" into the niobium wall. When the solenoid map was computed there was no such field-expelling wall next to it, so the solenoid's real stray field inside the cavity differs from the separately computed map.

### Step 6: RF fields together with static magnetic fields

RF fields oscillate in time, and each RF map is multiplied by its own time factor ($\cos(\omega t + \varphi_0)$ for the electric field, sin for the magnetic field). Static magnetic fields do not vary in time. The Lorentz force is always perpendicular to the velocity, so its component along the velocity is zero and it does no work: a static magnetic field does not directly change the energy, only the transverse motion. This is one reason the exit energy is the same for all four overlaps in the comparison of the main text (the other is that with V3 = 0 the cavity's entrance phase is solved again).

### For readers who know electromagnetism

Maxwell's equations in vacuum are linear: if $(\boldsymbol{E}_1, \boldsymbol{B}_1)$ and $(\boldsymbol{E}_2, \boldsymbol{B}_2)$ are solutions, so is their sum. The total field of several sources is the vector sum of their fields, and the force on a particle is

$$
\boldsymbol{F} = q\Bigl[\sum_i \boldsymbol{E}_i(\boldsymbol{r}, t) + \boldsymbol{v}\times\sum_i \boldsymbol{B}_i(\boldsymbol{r}, t)\Bigr]
$$

where each field is shifted to its element's coordinates: the map coordinate of element $i$ is $z_i' = z - z_b - z_{0,i}$ ($z_b$ the start of the block, see [Longitudinal positions](#superpose-z)), multiplied by its own Ke, Kb; RF fields are further multiplied by their own time factors (cos for the electric field, sin for the magnetic field, see [RF cavity](#rf-cavity-in-avas)). The engine integrates in the total field with its t-code. Divergence is a linear operation, so the sum of two maps that each satisfy $\nabla\cdot\boldsymbol{B} = 0$ still does; the hidden condition is that both maps are solutions in the same vacuum region in the overlap. A ferromagnetic medium has the nonlinear constitutive relation $\boldsymbol{B} = \mu(\boldsymbol{H})\,\boldsymbol{H}$, and a superconductor imposes the boundary condition $\boldsymbol{B} = 0$; both make "solve separately, then add" fail, and the combined field has to be found from one boundary-value problem.

## Going deeper: why a lens at the waist does almost nothing {#superpose-deep-waist}

In the comparison of the main text, the closer the cavity is to the waist, the narrower the beam at the end, and with the cavity upstream of the waist it is even narrower than without the cavity. This section explains why, step by step, with school algebra (expanding a square, taking an average).

### Step 1: what a thin lens does

Approximate the cavity's transverse effect by a thin defocusing lens: a particle's position $x$ does not change and its angle $x'$ increases by $x/f$ ($f > 0$ meaning defocusing: the farther from the axis, the harder the push outwards):

$$
x'_{\text{new}} = x' + \frac{x}{f}.
$$

### Step 2: how the spread of angles changes

The beam size downstream is set mainly by the spread of the particles' angles. Square the equation above and average over all particles (angle brackets $\langle\ \rangle$ mean the average):

$$
\langle x'^2_{\text{new}}\rangle = \langle x'^2\rangle + \frac{2}{f}\,\langle x\,x'\rangle + \frac{1}{f^2}\,\langle x^2\rangle .
$$

This is the school expansion $(a + b)^2 = a^2 + 2ab + b^2$, averaged term by term.

### Step 3: what the middle term means

$\langle x\,x'\rangle$ describes how position and angle are related:

- when the beam is **growing** (diverging), particles on the outside fly outwards ($x' > 0$ when $x > 0$, $x' < 0$ when $x < 0$), so $\langle x\,x'\rangle > 0$;
- when the beam is **shrinking** (converging) it is the other way round, $\langle x\,x'\rangle < 0$;
- at the **waist** the beam is neither growing nor shrinking, $\langle x\,x'\rangle = 0$.

### Step 4: three positions

- **At the waist**: the middle term is zero and only $\langle x^2\rangle/f^2$ remains. $\langle x^2\rangle$ is smallest at the waist, so this term is small and the lens hardly changes the spread of angles.
- **Downstream of the waist** (diverging): the middle term is positive and the defocusing lens increases the spread of angles: wider downstream. With $z_0 = 0.35$ the cavity's first gap is about 8 cm downstream of the waist: 5.46 mm at the end (4.87 mm without the cavity).
- **Upstream of the waist** (converging): the middle term is negative, and as long as $\langle x^2\rangle/f^2$ is not too large the defocusing lens actually reduces the spread of angles. With $z_0 = 0.25$ the first gap is about 2 cm upstream of the waist: 4.22 mm at the end (4.86 mm without the cavity).

The sizes: $\langle x^2\rangle$ is the square of the rms size. At the cavity centre rms x is about 1.2 mm for $z_0 = 0.35$ and about 0.3 mm for $z_0 = 0.25$; the squares differ by more than a factor of ten. With the last term that much smaller, the sign of the middle term decides the outcome.

### Step 5: acceleration makes the angles smaller

An angle is the ratio of transverse to longitudinal momentum, $x' = p_x/p$. When the cavity accelerates, the longitudinal momentum $p$ grows while the transverse momentum stays essentially the same, so the angles shrink by $p_{\text{in}}/p_{\text{out}}$ (adiabatic damping). The example beam's $p$ grows from 53.55 MeV/$c$ (1.527 MeV) to 59.21 MeV/$c$ (1.866 MeV), so the angles shrink to 0.904 times their value, about 10 %.

### For readers who know linear algebra

With the beam's second-moment matrix $\sigma$ ($\sigma_{11} = \langle x^2\rangle$, $\sigma_{12} = \langle x x'\rangle$, $\sigma_{22} = \langle x'^2\rangle$), a thin lens $M = \begin{pmatrix} 1 & 0 \\ 1/f & 1\end{pmatrix}$ maps $\sigma \to M\sigma M\tp$, so $\sigma_{22}$ changes by $2\sigma_{12}/f + \sigma_{11}/f^2$. At a waist $\sigma_{12} = 0$ and $\sigma_{11}$ is smallest, leaving only the small second term. Away from the waist the first term dominates, and its sign matters: after the waist ($\sigma_{12} > 0$) a defocusing lens increases the spread of angles, before it ($\sigma_{12} < 0$) it decreases it, which is exactly the $z_0 = 0.25$ case in the table of the main text.

## In AVAS {#superpose-in-avas}

### Syntax and rules

```text
superpose     z0  x0  y0  θz0  θx0  θy0
superposeend
superposeout  z0  x0  y0  θz0  θx0  θy0
```

```avas-params
superpose superposeend superposeout
```

Rules (from the manual; the lattice page checks them):

1. The first `superpose` must have all parameters zero; it is the zero for the other superposes.
2. The 6 parameters of `superpose` (and `superposeout`) give the position ($z_0$, $x_0$, $y_0$, m) and angles ($\theta_{z0}$, $\theta_{x0}$, $\theta_{y0}$, degrees) of the entrance plane (transformation plane) of the following element relative to the entrance plane of the element after the first superpose.
3. A superposition block must end with `superposeend` or `superposeout`; only when it ends with `superposeout` do the parameters other than $z_0$ take effect. With `superposeend` the elements are superposed only longitudinally.
4. A superposition group may contain at most one RF cavity.
5. Every element in a block is preceded by exactly one `superpose`.

> **AVAS convention**: the manual only gives superposition examples with field-map elements. The behaviour of matrix-model elements (`quad`, `solenoid`, `bend`, …) inside a block is undocumented; the linear preview ignores elements in a block other than `field`, `drift`, `steerer` and diagnostics, with a warning. Put only field-map elements in a block.

### superposeend and superposeout

| Ending | Parameters in effect | Use |
|---|---|---|
| `superposeend` | only the $z_0$ of each superpose | longitudinal overlap of coaxial elements (the most common case) |
| `superposeout` | $z_0$ and $x_0$, $y_0$, $\theta_{z0}$, $\theta_{x0}$, $\theta_{y0}$ | elements in the block offset or rotated relative to the zero, e.g. studying a solenoid misaligned or tilted relative to a cavity, or rotating a field map about $z$ |

The manual says that the 6 parameters after `superpose` (`superposeout`) give the position and angles of the entrance plane of the element following that command relative to the block's zero. Taken literally, the parameters of `superposeout` itself refer to the element after the block; whether the engine accordingly changes the position and direction of the beam line after the block, the manual does not say further; the engine is the reference.

> **Note**: the exact conventions for offsets and angles (the order in which the three rotations act, the positive directions of the frame) are not stated in the manual; the engine is the reference. Before using them, it is advisable to verify with a single-particle run (`particlenumber 1`) that a simple offset shows up in the trajectory as expected. The beam-line layout of the AVAS interface does not read these parameters, and the linear preview also ignores the offsets and angles of superposeout (with a warning). For static installation errors, the more usual way is the error commands, see [Errors and correction](#errors).

### Superposition groups in the interface

- **Structure tree**: a block is shown as a group `superpose (n)`, $n$ the number of elements in it; the superpose lines, the elements and the closing line are all inside the group.
- **Checks**: first superpose not all zeros (warning); block not closed with superposeend / superposeout (error); element without a superpose before it (error); two superposes in a row (error); more than one RF cavity in a block (warning).
- **2D layout and the small beam-line plot on the lattice page**: overlapping elements are drawn in layers by their real $z$ ranges, with transverse position and length unchanged; the layers are only schematic and do not mean a transverse offset. Each layer can be clicked on its own, hovering lists all superposed elements at that position, and superposition groups are marked with a bracket; there is no limit on the number of layers, and many layers can be scrolled.
- **Drag and drop in the visual editor**: dropping an element onto the shape of a non-drift element creates a superposition starting at that element's entrance, and several elements can be superposed at the same start one after another; dropping inside an ordinary non-drift element creates a superposition group automatically, with the end points still inserted in order and drifts split as usual. Positions within the group are relative to its first element, which is fixed at 0; a change in the group's length moves the elements downstream. Buttons that act on the whole group say "whole group" explicitly. The context menu in the 2D and 3D views offers superpose, copy, delete and so on; write operations are unavailable while browsing or under the run lock.
- **Linear preview**: the fields of all maps in a block are added by position and computed together; the reference-particle energy, the transverse gradients and the solenoid field are all superposed.

### Exercises

1. Copy the example project, create a new lattice on the lattice page, enter the [example](#superpose-z), and check that the 2D layout shows the two elements in two layers and that hovering over the overlap shows both.
2. Change $z_0$ to 0.35, then to the form with two `field` lines one after the other, and check that the linear-preview curves of the two forms coincide.
3. Change the end of the block to superposeout and give the cavity's superpose $x_0 = 0.001$; look at the warning from the linear preview, then use a single-particle run to see how the trajectory changes in the engine.
4. In the field-map viewer, look at how $B_z$ decays at the exit end of the solenoid map and judge how much field is left in the 5 mm overlap:

```avas-field
map sol
```

## Further reading {#superpose-reading}

- The "Superposed fields" section of the original manual and the case tutorial "Getting started with superposed fields, multi-particle" (the parameter and file reference and the case tutorials in the help window); the AVAS user guide `docs/使用说明20260427.docx`.
- The description of the SUPERPOSE_MAP command in the TraceWin user manual: the same idea, but lengths in mm and not quite the same parameters.
- J. D. Jackson, *Classical Electrodynamics*: linearity of Maxwell's equations and the superposition principle, boundary conditions in magnetic media.
- In this primer: [RF cavity](#rf-cavity), [Static field maps](#static-fields), [Field maps](#field-maps), [Lattice design](#lattice-design), [References](#references).
