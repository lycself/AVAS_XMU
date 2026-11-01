# Superposed fields

In a real accelerator the fields of neighbouring elements often overlap in space: the fringe of a superconducting solenoid reaches into the next cavity, a solenoid carries steering coils, magnets with long fringes sit close together. Elements in a lattice are placed end to end by default, each taking its own stretch of $z$; to let the fields of several elements act together in the same region you need the **superpose** command `superpose`. This chapter explains the physical premise of superposition, AVAS's syntax and rules, how longitudinal positions are computed, the difference between `superposeend` and `superposeout`, and superpose groups in the GUI.

## Role and position {#superpose-role}

Typical uses:

- **Solenoid and RF cavity**: in low-β superconducting sections the solenoid sits right next to the cavity and the fringe of the solenoid map overlaps the cavity map. The example of the manual and of the case tutorial superposes exactly the solenoid map `sol` and the cavity map `hwr010`.
- **Magnets with steering coils**: a solenoid or quadrupole with embedded dipole steering coils; the map of the coils can be superposed on the map of the main magnet.
- **Maps with long fringes**: putting two maps end to end forces you to cut each fringe at the boundary; superposition keeps the full fringes of both maps and adds the fields where they overlap.

When superposition is not needed: a steerer inside a matrix-model quadrupole is written simply as `steerer` before `quad` — a steerer has length 0 and by default acts over the whole length of the next element, centred on it; see [Steerer](#steerer).

## Physics: linear superposition of fields {#superpose-physics}

Maxwell's equations in vacuum are linear, so the total field of several sources is the vector sum of their fields. Inside a superpose block the force on a particle is

$$
\boldsymbol{F} = q\Bigl[\sum_i \boldsymbol{E}_i(\boldsymbol{r}, t) + \boldsymbol{v}\times\sum_i \boldsymbol{B}_i(\boldsymbol{r}, t)\Bigr]
$$

where each field is shifted to the coordinates of its element: the map coordinate of element $i$ is $z_i' = z - z_b - z_{0,i}$ ($z_b$ being the start of the block, see [the next section](#superpose-z)), and it is multiplied by its own Ke, Kb; RF fields also carry their time factor (cos for the electric, sin for the magnetic field, see [RF cavities](#rf-cavity-in-avas)). The kernel integrates in the total field with the t-code.

Premises and limits of superposition:

- **Only linear media can be superposed.** Adding separately computed maps assumes that the elements do not influence each other. When magnets with iron yokes are close together, saturation and mutual magnetization change their fields; superconducting cavity walls expel an applied static magnetic field (Meissner effect), so the real stray field of a solenoid inside a cavity differs from the separately computed solenoid map. For accurate work, compute the combined field in one electromagnetic calculation and use it as a single map.
- **Time structure of RF fields.** Each RF cavity has its own frequency and phase; AVAS allows at most one RF cavity per superpose block, with no such limit for static fields. The manual does not give the reason for this rule; cases where the fields of two cavities overlap significantly cannot be described with AVAS's superposition.
- **When a static field overlaps an RF field**, the static magnetic field does no work and does not change the energy gain directly; what it changes is the transverse motion inside the cavity.

## Longitudinal positions {#superpose-z}

### The zero of the block

A block starts at its first `superpose`. If the lattice has accumulated $z_b$ at that point (the total length of all elements before the first superpose), element $i$ of the block occupies

$$
z \in \bigl[\,z_b + z_{0,i},\ \ z_b + z_{0,i} + L_i\,\bigr]
$$

where $z_{0,i}$ is the first parameter of the `superpose` in front of that element, "the entrance of this element relative to the entrance of the element after the first superpose". The first superpose must be all zeros; it defines the zero.

### Position after the block

The AVAS GUI (Lattice page, 2D layout, linear preview) takes the length of the block as the furthest exit of its elements:

$$
z_{after} = z_b + \max_i\bigl(z_{0,i} + L_i\bigr)
$$

Elements after `superposeend` continue from $z_{after}$. The manual does not state the kernel's rule for the position after a block explicitly; before production runs check the total length with the positions in DataSet.txt (column 5 plus column 33) or the beamline picture.

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
| after the block | — | from 0.640 |

The last 5 mm of the solenoid map overlap the first 5 mm of the cavity map, and both fringes are kept in full. With $z_0$ = 0.35 (the solenoid length) the two elements would just touch, equivalent to writing two `field` lines one after the other without superposition; the linear preview gives identical results for both.

> **Note**: in the manual's original text the second superpose of this example has 7 numbers (`superpose 0.345 0 0 0 0 0 0`), one 0 too many; superpose has 6 parameters, see the table below.

## In AVAS {#superpose-in-avas}

```text
superpose     z0  x0  y0  θz0  θx0  θy0
superposeend
superposeout  z0  x0  y0  θz0  θx0  θy0
```

```avas-params
superpose superposeend superposeout
```

Rules (from the manual; the Lattice page checks them):

1. The parameters of the first `superpose` must all be 0; it is the zero of the other superposes.
2. The 6 parameters of `superpose` (and `superposeout`) give the position ($z_0$, $x_0$, $y_0$, in m) and angles ($\theta_{z0}$, $\theta_{x0}$, $\theta_{y0}$, in degrees) of the entrance plane (transformation plane) of the following element relative to the entrance plane of the element after the first superpose.
3. A block must end with `superposeend` or `superposeout`; the parameters other than $z_0$ take effect only when it ends with `superposeout`. With `superposeend` the elements are superposed only longitudinally.
4. At most one RF cavity per block.
5. Every element in a block has exactly one `superpose` in front of it.

> **AVAS convention**: the manual only gives examples of superposed field-map elements. What happens with matrix-model elements (`quad`, `solenoid`, `bend` …) inside a block is not documented; the linear preview ignores elements other than `field`, `drift`, `steerer` and diagnostics inside a block, with a warning. Put only field-map elements in a block.

## superposeend and superposeout {#superpose-out}

| End | Parameters that take effect | Use |
|---|---|---|
| `superposeend` | only the $z_0$ of each superpose | longitudinal overlap of coaxial elements (the usual case) |
| `superposeout` | $z_0$ and $x_0$, $y_0$, $\theta_{z0}$, $\theta_{x0}$, $\theta_{y0}$ | elements offset or rotated relative to the zero of the block, e.g. a solenoid displaced or tilted relative to the cavity, or a map rotated about $z$ |

The manual says that the 6 parameters after `superpose` (`superposeout`) give the position and angles of the entrance plane of the element following the command relative to the zero of the block. Read literally, the parameters of `superposeout` itself refer to the element after the block; whether the kernel uses them to shift or turn the beamline after the block is not explained further in the manual — the kernel is the reference.

> **Note**: the exact conventions of offsets and rotations (order of the three rotations, positive directions) are not documented in the manual; the kernel is the reference. Before relying on them, check with a single-particle run (`particlenumber 1`) that a simple offset shows up in the trajectory as expected. The GUI's beamline layout does not read these parameters, and the linear preview ignores the offsets and rotations of superposeout (with a warning). For static alignment errors the usual way is the error commands, see [Errors and correction](#errors).

## Superpose groups in the GUI {#superpose-gui}

- **Structure tree**: a block is shown as a group `superpose (n)`, $n$ being the number of elements in it; the superpose lines, the elements and the end line all sit inside the group.
- **Checks**: the first superpose not all zeros (warning); a block not closed by superposeend / superposeout (error); an element without a superpose in front of it (error); two superpose lines in a row (error); more than one RF cavity in a block (warning).
- **2D layout and the small beamline picture of the Lattice page**: overlapping elements are drawn in separate layers according to their real $z$ ranges, with unchanged horizontal positions and lengths; the layers are only schematic and do not mean transverse offsets. Each layer can be selected by clicking, hovering lists all superposed elements at that position, and a bracket marks the superpose group; there is no limit on the number of layers, and many layers scroll.
- **Drag and drop in the visual editor**: dropping an element on the picture of a non-drift element superposes it starting at that element's entrance, and several elements with the same start can be stacked in a row; dropping inside an ordinary non-drift element creates a superpose group automatically, endpoints are still inserted in order and drifts are split as usual. Positions inside a group are relative to its first element, which stays at 0; changing the length of a group moves the downstream elements. Buttons acting on the whole group say "whole group". The context menu of the 2D and 3D views offers superpose, copy, delete and more; write operations are disabled in browse mode or under the run lock.
- **Linear preview**: the fields of all maps of a block are added by position and computed together; reference-particle energy, transverse gradients and solenoid fields are all superposed.

## Common problems {#superpose-pitfalls}

- **First line not zero**: a non-zero $z_0$ in the first superpose shifts the whole block wrongly; it must be all zeros.
- **Missing superpose**: every element of a block needs one, and two in a row are not allowed either.
- **Forgetting the end**: without superposeend / superposeout all following elements count as part of the block.
- **Two RF cavities**: cavities with overlapping fields cannot go into the same block. If only the tails of the fringes overlap slightly, cut the maps where the field has decayed to negligible values and put them end to end (the field at the cut must be small enough, see [the ends of a map](#field-maps-grid)).
- **Wrong block length**: the elements after a block start at the furthest exit inside the block, not at the exit of the last element; write the elements of a block in the order of $z$.
- **Using superposition as a "patch"**: superposition only holds for fields computed separately in linear media; interactions between ferromagnetic or superconducting boundaries cannot be approximated by it, see [Physics](#superpose-physics).
- **Offsets ignored**: with superposeend, $x_0$, $y_0$ and the angles have no effect; use superposeout when offsets are needed.

## Hands-on {#superpose-lab}

Starting from the [example lattice](#superpose-z), add a 0.3 m drift after the block and change only the $z_0$ of the superpose in front of the cavity (linear preview, no space charge):

| $z_0$ / m | block length / m | overlap with the solenoid | exit energy / MeV |
|---|---|---|---|
| 0.35 | 0.560 | none (equivalent to placing them in sequence) | 1.8665 |
| 0.345 | 0.555 | 5 mm | 1.8665 |
| 0.30 | 0.510 | 50 mm | 1.8665 |
| 0.25 | 0.460 | 100 mm | 1.8665 |

The exit energy hardly changes (by less than 0.1 keV): the static magnetic field does no work, and the cavity is given its synchronous phase with V3 = 0, so the kernel (and the preview) solves the entry phase again for the new position. What changes is the transverse motion: as the cavity moves further into the downstream fringe of the solenoid, the transverse beam size inside the cavity and the difference between $x$ and $y$ change.

Suggested exercises:

1. Copy the example project, create a new lattice on the Lattice page with the example above, and check that the 2D layout draws the two elements in two layers and that hovering over the overlap shows both.
2. Change $z_0$ to 0.35, then rewrite it as two consecutive `field` lines, and check that the linear-preview curves of the two versions coincide.
3. End the block with superposeout and give the cavity's superpose $x_0 = 0.001$; look at the warning of the linear preview, then use a single-particle run to see how the trajectory changes in the kernel.
4. In the field-map viewer look at how $B_z$ decays at the exit of the solenoid map, and judge how much field is left in the 5 mm overlap:

```avas-field
map sol
```

## Further reading {#superpose-reading}

- The "superpose" section of the original manual and the case "Superpose, multi-particle introduction" (parameter and file reference and case tutorials in the help window).
- The SUPERPOSE_MAP command in the TraceWin user manual: the same idea, but lengths in mm and not exactly the same parameters.
- In this primer: [RF cavities](#rf-cavity), [Static field maps](#static-fields), [Field maps](#field-maps), [Lattice design](#lattice-design), [References](#references).
