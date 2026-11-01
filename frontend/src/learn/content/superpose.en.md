# Superposed fields

In an AVAS lattice the elements follow one another end to end, each one owning its own stretch of $z$. Real accelerators are not laid out like that: the fringe field of a superconducting solenoid reaches into the neighbouring cavity, steering coils are wound inside the solenoid, magnets with long fringe fields sit close together. How can two elements occupy the same space? Is it enough to add the numbers of two field maps? And when is adding them wrong?

This chapter answers with one fact: Maxwell's equations in vacuum are linear, so fields computed separately can be added, provided the two elements do not change each other's field. On that basis it explains how AVAS's `superpose` command fixes the longitudinal position of each element, what superposition actually changes in the example, and the difference between `superposeend` and `superposeout`. Read [Field maps](#field-maps) first; the physics of field-map elements is in [RF cavity](#rf-cavity) and [Electrostatic and magnetostatic field maps](#static-fields).

Below is the superposed group of the manual's example: the solenoid sol and the cavity hwr010 overlap by 5 mm. The 2D view draws overlapping elements in stacked layers (the layers are only a drawing aid, not a transverse offset). In the linear preview the block starts at $z = 0.085\ \mathrm{m}$ and ends at 0.64 m; the beam reaches a waist downstream of the solenoid at $z \approx 0.42\ \mathrm{m}$ (rms x 0.27 mm), the cavity raises the kinetic energy from 1.527 MeV to 1.866 MeV, and at the end (0.94 m) rms x is 5.25 mm.

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

Putting a field map into a lattice says "from here to here the particle feels the field of this map". If the fields of two elements overlap in space and we still place them end to end, there are only two choices: cut both maps at the boundary and throw away the part of each fringe field that reaches into the other (if the field is not small at the cut, particles receive a spurious edge kick, see [map ends](#field-maps-grid)); or accept the overlap and let both maps act together in the overlap region. `superpose` is the second choice.

Typical uses:

- **Solenoid and RF cavity**: in a low-β superconducting section the solenoid sits right next to the cavity, and the fringe part of the solenoid map overlaps the cavity map. The example of the manual and the case tutorial is exactly the superposition of the solenoid map `sol` and the cavity map `hwr010`.
- **Magnets with steering coils**: dipole steering coils embedded in a solenoid or quadrupole can be described by superposing the coil map on the main magnet map. This is the most complete overlap: the two fields occupy the same space.
- **Maps with long fringe fields**: placed end to end, two maps must have their fringe fields cut at the boundary; superposition keeps both fringe fields complete and adds the fields in the overlap region.

When superposition is not needed: a steerer inside a matrix-model quadrupole is simply written as `steerer` before the `quad`. A steerer has zero length and by default acts over the full length of the next element, centred in it; see [Steerer](#steerer).

## The physics: Maxwell's equations are linear {#superpose-physics}

Maxwell's equations in vacuum are linear: if $(\boldsymbol{E}_1, \boldsymbol{B}_1)$ and $(\boldsymbol{E}_2, \boldsymbol{B}_2)$ are solutions, so is their sum. The total field of several sources is the vector sum of their fields, and the force on a particle is

$$
\boldsymbol{F} = q\Bigl[\sum_i \boldsymbol{E}_i(\boldsymbol{r}, t) + \boldsymbol{v}\times\sum_i \boldsymbol{B}_i(\boldsymbol{r}, t)\Bigr]
$$

where each field is shifted to its own element's coordinates: the map coordinate of element $i$ is $z_i' = z - z_b - z_{0,i}$ ($z_b$ is the start of the block, see [the next section](#superpose-z)), multiplied by its own Ke and Kb; RF fields are further multiplied by their time factors (cos for the electric, sin for the magnetic field, see [RF cavity](#rf-cavity-in-avas)). The engine integrates in the total field with its t-code.

> **Think about it**: two maps each satisfy $\nabla\cdot\boldsymbol{B} = 0$. Does their sum? Is there a hidden condition?
>
> Answer: divergence is linear, so the sum has zero divergence. The hidden condition is that both maps are solutions in **the same vacuum region** where they overlap. If the conductors, yoke or coils of one element sit inside the field of the other, that region is not vacuum, and the separately computed map is not the right solution there.

This hidden condition is where superposition reaches its limits:

- **Only linear media can be superposed**. Adding separately computed maps assumes the elements do not influence each other. When magnets with iron yokes are close together, saturation and mutual magnetization change their fields; superconducting cavity walls screen a static magnetic field applied after cool-down (Meissner effect; flux present during cool-down can instead be trapped in the niobium), so the real stray field of a solenoid inside a cavity differs from the separately computed solenoid map. For accurate work compute the combined field in one electromagnetic calculation and use it as a single map.
- **Time structure of RF fields**. Different RF cavities have their own frequencies and phases; AVAS allows at most one RF cavity per superposed group, while the number of static fields is not limited. The manual does not give the reason for this rule; cases where the fields of two cavities overlap significantly cannot be described with AVAS's superposition.
- **When static and RF fields are superposed**, the magnetostatic field does no work and does not change the energy gain directly; it changes the transverse motion inside the cavity.

## How the longitudinal positions work {#superpose-z}

### The zero of a block

A superposed block starts at the first `superpose`. Let the lattice have reached $z_b$ at that point (the total length of all elements before the first superpose); element $i$ of the block then occupies

$$
z \in \bigl[\,z_b + z_{0,i},\ \ z_b + z_{0,i} + L_i\,\bigr]
$$

where $z_{0,i}$ is the first parameter of the `superpose` in front of that element, i.e. "the longitudinal position of this element's entrance relative to the entrance of the element after the first superpose". All parameters of the first superpose must be 0; it defines the zero. In other words, positions inside a block no longer follow on from the previous element; every element is placed independently relative to the same zero.

### Positions after the block

The AVAS interface (Lattice page, 2D layout, linear preview) takes the block length as the farthest exit of the elements in the block:

$$
z_{after} = z_b + \max_i\bigl(z_{0,i} + L_i\bigr)
$$

Elements after `superposeend` continue from $z_{after}$. The manual does not state the engine's rule for the position after a block explicitly; before production runs, check the total length with the positions in the run's DataSet.txt (column 5 plus column 33) or with the beam-line plot.

### Example

The lattice of the manual and the case tutorial:

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
| sol (magnetostatic) | 0 | 0.085 – 0.435 |
| hwr010 (RF) | 0.345 | 0.430 – 0.640 |
| After the block | — | from 0.640 |

The last 5 mm of the solenoid map overlap the first 5 mm of the cavity map, and the fringes of both maps are kept complete. With $z_0 = 0.35$ (= the solenoid length) the two elements would just touch end to end, equivalent to writing two `field` lines one after the other without superposition; in the linear preview both forms give identical results.

> **Note**: in the manual's text the second superpose of this example has 7 numbers (`superpose 0.345 0 0 0 0 0 0`), one 0 too many; superpose has only 6 parameters, see [In AVAS](#superpose-in-avas).

## What superposition changes {#superpose-effect}

First estimate how much field there is in the 5 mm overlap of the manual's example. In the last 5 mm of the `sol` map the on-axis $B_z$ is only about 0.25 % of the peak; that stretch holds about 0.01 % of $\int B_z\,\dd z$ and $4\times 10^{-7}$ of $\int B_z^2\,\dd z$. The first 5 mm of `hwr010` hold only 0.04 % of the cavity's $\int\lvert E_z\rvert\,\dd z$. Both maps are practically zero in the overlap, which is just what "the field should have decayed at the map ends" demands. So what does superposition change in this example?

A comparison (linear preview, no space charge): change only the $z_0$ of the superpose before the cavity, and adjust the drift after the block so that the beam line is always 0.945 m long and the end is compared at the same place. Then compute each case once more with the cavity field off (Ke = Kb = 0):

| $z_0$ / m | Overlap with the solenoid | Cavity occupies / m | Exit energy / MeV | rms x at the end / mm | Cavity field off / mm |
|---|---|---|---|---|---|
| 0.35 | none (equivalent to end to end) | 0.435 – 0.645 | 1.8665 | 5.36 | 4.80 |
| 0.345 | 5 mm | 0.430 – 0.640 | 1.8665 | 5.32 | 4.80 |
| 0.30 | 50 mm | 0.385 – 0.595 | 1.8665 | 4.82 | 4.81 |
| 0.25 | 100 mm | 0.335 – 0.545 | 1.8665 | 4.13 | 4.79 |

Three things stand out:

- **The exit energy hardly changes** (by less than 0.1 keV): the magnetostatic field does no work, and the cavity's synchronous phase is set with V3 = 0, so the engine (and the preview) solves for the entrance phase again at the new position ($52.67^\circ$ in all four cases).
- **The solenoid tail itself does almost nothing**: the last column (solenoid only) stays between 4.79 and 4.81 mm; even with 100 mm of overlap the tail holds only 0.4 % of $\int B_z^2$.
- **The change comes from the cavity's position relative to the waist**: the solenoid focuses the beam to a waist at $z \approx 0.42\ \mathrm{m}$ (rms x 0.27 mm). The smaller $z_0$, the closer the cavity is to the waist: for $z_0 = 0.35$ rms x at the cavity centre is about 1.2 mm, for $z_0 = 0.25$ only about 0.3 mm. Most of the cavity's defocusing comes from its first gap (at this low β the first gap sits far from its crest, see [RF cavity](#rf-cavity-transverse)), about 6.5 cm behind the cavity entrance. For $z_0 = 0.35$ that gap is about 8 cm downstream of the waist, where the beam already diverges: the outward kick adds to the angles and the end size grows (5.36 mm against 4.80 mm). For $z_0 = 0.25$ the gap sits about 2 cm *upstream* of the waist, where the beam still converges: the same outward kick now cancels part of the converging angles, so the angular spread after the cavity is smaller. Together with adiabatic damping from the acceleration (angles shrink by about 10 %) the end size becomes smaller than with the cavity off (4.13 mm against 4.79 mm). A two-thin-lens estimate of the cavity reproduces this trend (5.14, 5.10, 4.65, 4.07 mm for the four rows).

In the manual's example, then, `superpose` matters mainly because it **puts the cavity where it really is** while keeping the complete fringes of both maps, not because of the field in the overlap. The field in the overlap really counts when steering coils sit inside the main magnet, or when fringe fields are long.

> **Think about it**: why does a lens placed at a beam waist hardly change the beam size downstream?
>
> Answer: the angular kick a lens gives each particle is proportional to its distance $x$ from the axis. At a waist all particles have small $x$, so the kicks are small, while the size downstream is set by the angles at the waist, which the lens hardly touches. In matrix language: a thin lens with $\Delta x' = x/f$ ($f > 0$ defocusing here) changes $\sigma_{22}$ by $2\sigma_{12}/f + \sigma_{11}/f^2$. At the waist $\sigma_{12} = 0$ and $\sigma_{11}$ is smallest, so only the small second term remains. Away from the waist the first term dominates and its sign matters: after the waist ($\sigma_{12} > 0$) a defocusing lens increases the angular spread, before it ($\sigma_{12} < 0$) it decreases it, which is what happens for $z_0 = 0.25$ in the table above.

## Common misconceptions {#superpose-pitfalls}

- **"Superposition just adds the map values and is always right"**: it only applies to fields computed separately in linear media; iron saturation, mutual magnetization and the Meissner effect of superconducting walls cannot be approximated by superposition, see [the physics](#superpose-physics).
- **"The more overlap, the more superposition matters"**: in the example the 5 mm overlap contains almost no field; superposition changes the result mainly because the element positions change, see [what superposition changes](#superpose-effect).
- **First superpose not zero**: a non-zero $z_0$ in the first superpose shifts the whole block wrongly; it must be all zeros.
- **Missing superpose**: every element in the block needs one in front of it, and two in a row are not allowed either.
- **Forgetting the end**: without superposeend / superposeout, all following elements are treated as part of the block.
- **Two RF cavities**: two cavities whose fields overlap cannot go into the same block. If only the tails of the fringe fields overlap slightly, the maps can be cut where the field has decayed to negligible values and placed end to end (the field at the cut must be small enough, see [map ends](#field-maps-grid)).
- **Wrong block length**: elements after the block start at the farthest exit in the block, not at the exit of the last element; write the elements of a block in order of $z$.
- **Offset parameters ignored**: when the block ends with superposeend, $x_0$, $y_0$ and the angles have no effect; offsets need superposeout.

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

Rules (from the manual; the Lattice page checks them):

1. All parameters of the first `superpose` must be 0; it is the zero for all other superposes.
2. The 6 parameters of `superpose` (and `superposeout`) give the position ($z_0$, $x_0$, $y_0$, m) and angles ($\theta_{z0}$, $\theta_{x0}$, $\theta_{y0}$, degrees) of the entrance plane (transformation plane) of the following element relative to the entrance plane of the element after the first superpose.
3. A superposed block must end with `superposeend` or `superposeout`; parameters other than $z_0$ take effect only when it ends with `superposeout`. With `superposeend` the elements are superposed longitudinally only.
4. A superposed group may contain at most one RF cavity.
5. Every element in a superposed block has exactly one `superpose` in front of it.

> **AVAS convention**: the manual only gives examples of superposed field-map elements. The behaviour of matrix-model elements (`quad`, `solenoid`, `bend` …) inside a block is not documented; the linear preview ignores elements other than `field`, `drift`, `steerer` and diagnostics inside a block and warns. Put only field-map elements in a block.

### superposeend and superposeout

| End | Parameters in effect | Use |
|---|---|---|
| `superposeend` | Only the $z_0$ of each superpose | Longitudinal overlap of coaxial elements (the most common case) |
| `superposeout` | $z_0$ plus $x_0$, $y_0$, $\theta_{z0}$, $\theta_{x0}$, $\theta_{y0}$ | Elements of the block offset or rotated relative to the zero, e.g. to study a solenoid misaligned or tilted against the cavity, or a map rotated about $z$ |

The manual says that the 6 parameters after `superpose` (`superposeout`) give the position and angles of the entrance plane of the element following that command relative to the zero of the block. Read literally, the parameters of `superposeout` itself refer to the element after the block; whether the engine changes the position and direction of the beam line after the block accordingly is not explained further in the manual; the engine is authoritative.

> **Note**: the exact conventions of offsets and rotations (order in which the three angles act, positive directions) are not documented in the manual; the engine is authoritative. Before using them, check with a single-particle run (`particlenumber 1`) that a simple offset shows up in the trajectory as expected. The beam-line layout of the interface does not read these parameters, and the linear preview ignores the offsets and angles of superposeout (with a warning). For static installation errors the usual way is the error commands, see [Errors and correction](#errors).

### Superposed groups in the interface

- **Structure tree**: a superposed block is shown as a group `superpose (n)`, $n$ being the number of elements in it; the superpose lines, elements and end line of the block are all inside the group.
- **Checks**: first superpose not all zero (warning); block not ended with superposeend / superposeout (error); element without a superpose in front (error); two superposes in a row (error); more than one RF cavity in a block (warning).
- **2D layout and the small beam-line plot of the Lattice page**: overlapping elements are drawn in stacked layers according to their real $z$ ranges, with unchanged transverse position and length; the layers are only a drawing aid, not a transverse offset. Each layer can be clicked separately; hovering lists all superposed elements at that position, and a superposed group is marked with a bracket. There is no limit on the number of layers; with many layers the plot scrolls.
- **Drag and drop in the visual editor**: dropping an element onto the drawing of a non-drift element creates a superposition starting at that element's entrance, and several elements with the same start can be superposed in a row; dropping inside an ordinary non-drift element creates a superposed group automatically, endpoints are still inserted in order and drifts are split as usual. Positions in a group are relative to the first element, which is fixed at 0; when the group length changes, downstream elements move. Buttons that act on the whole group say "whole group" explicitly. The context menu of the 2D and 3D views offers superposing, copying, deleting and so on; write operations are unavailable in browse mode or under the run lock.
- **Linear preview**: the fields of all maps in a block are added by position and computed together; reference energy, transverse gradients and solenoid fields are all superposed.

### Exercises

1. Copy the example project, create a new lattice on the Lattice page with the [example](#superpose-z), and check that the 2D layout shows the two elements in two layers and that hovering over the overlap lists both elements.
2. Change $z_0$ to 0.35, then rewrite it as two consecutive `field` lines, and check whether the linear preview curves of the two forms coincide.
3. End the block with superposeout and give the cavity's superpose $x_0 = 0.001$; look at the warning of the linear preview, then look at the change of the trajectory in the engine with a single-particle run.
4. In the field-map viewer, look at how $B_z$ decays at the exit end of the solenoid map and judge how much field is left in the 5 mm overlap:

```avas-field
map sol
```

## Further reading {#superpose-reading}

- The "superposed fields" section of the original manual and the case tutorial "multi-particle introduction to superposed fields" (parameter and file reference and case tutorials in the help window); AVAS manual `docs/使用说明20260427.docx`.
- The TraceWin user manual on the SUPERPOSE_MAP command: the same idea, but lengths in mm and parameters not entirely the same.
- J. D. Jackson, *Classical Electrodynamics*: linearity of Maxwell's equations and the superposition principle, boundary conditions at magnetic media.
- In this primer: [RF cavity](#rf-cavity), [Electrostatic and magnetostatic field maps](#static-fields), [Field maps](#field-maps), [Lattice design](#lattice-design), [References](#references).
