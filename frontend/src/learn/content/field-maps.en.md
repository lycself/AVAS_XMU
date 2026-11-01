# Field maps

The example project's half-wave cavity hwr010 is written as 6 text files of 46 746 numbers each; the solenoid sol as 3 files of 125 625 numbers each. Why should a few hundred thousand numbers be able to stand in for a real cavity or magnet? Open the solenoid map in the viewer and the heat map shows stripes along $z$: did the program read it wrongly, or is the data like that? And the file header only says how many points there are in each direction, not in which order the numbers are stored; how does AVAS know how to read them?

This chapter explains what a field map is physically (a sampled solution of Maxwell's equations), why AVAS integrates step by step in the map with its t-code instead of using transfer matrices, what grid, storage order, interpolation and noise each do, and how to check whether a new map can be trusted. The three uses of field maps are covered in [RF cavity](#rf-cavity) (field type 1) and [Electrostatic and magnetostatic field maps](#static-fields) (types 2 and 3).

The example project `examples/hwr010/InputFile` contains two maps: the half-wave cavity `hwr010` (`.edx/.edy/.edz/.bdx/.bdy/.bdz`, 0.21 m long) and the solenoid `sol` (`.bsx/.bsy/.bsz`, 0.35 m long). `efield.*` and `bfield.*` are byte-identical copies of these two sets. Below, the two field-map elements follow each other: the magnetostatic solenoid sol, then the RF cavity hwr010, one period of the guided tutorial. In the linear preview the solenoid focuses the beam to a waist at $z \approx 0.55\ \mathrm{m}$ (rms x 0.85 mm); the cavity starts at $z = 0.50\ \mathrm{m}$ and raises the kinetic energy from 1.527 MeV to 1.884 MeV. At the end rms x is 1.65 mm; with the cavity field switched off (Ke = Kb = 0) it would be 1.19 mm: the cavity's RF defocusing is written in the map too.

```avas-beamline
drift 0.05 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010
drift 0.2 0.02 0
```

## What a field map is: a sampled solution of Maxwell's equations {#field-maps-role}

When a cavity or magnet is designed, the engineer gives its real geometry (conductor surfaces, coils, yoke) to an electromagnetic solver (such as CST, COMSOL, Opera, Superfish), which solves Maxwell's equations numerically with these boundary conditions. The result is a continuous vector field; a **field map** samples this solution on a 3D grid around the beam channel and writes it out component by component.

A field map therefore contains no "physical model" of its own: it does not assume the field to be hard-edged, axially symmetric or linear, and fringe fields, higher-order components and asymmetries computed by the solver are all in it, together with the solver's discretization error and grid noise. This explains where both the strengths and the weaknesses of maps come from: a map is as real as the solver, and as rough.

### Shape from the file, amplitude from Ke and Kb

Maxwell's equations in vacuum are linear: any multiple of a solution is again a solution. For an RF cavity the field shape of a resonant mode is set by the geometry and the amplitude by the RF power fed in; for a static magnet without iron saturation the field is proportional to the current. So the map only needs to give **the shape of the field**, and the actual amplitude comes from Ke and Kb of the `field` element: electric fields in the file are in MV/m, magnetic fields in T, and the field that acts is $K_e\boldsymbol{E}_{map}$, $K_b\boldsymbol{B}_{map}$. RF fields are further multiplied by the time factors $\cos(\omega t+\varphi_0)$ (electric) and $\sin(\omega t+\varphi_0)$ (magnetic); why the two are $90^\circ$ apart is explained in [RF cavity](#rf-cavity-modes).

> **Think about it**: can a map of an iron-yoke quadrupole computed at one current be scaled with Kb to any gradient?
>
> Answer: only while the yoke is not saturated. The permeability of iron depends on the field; once it saturates the field is no longer proportional to the current and the field shape (fringe fields and higher-order components in particular) changes too. Linear scaling is then only an approximation, and the map should be recomputed near the operating current. RF cavities do not have this problem, but one map represents one mode only.

### Coordinates and data

- **One field map = one set of component files with the same name**. The last parameter of `field` is the file name without extension; the engine reads the extensions that belong to the field type.
- **Coordinates**: $z$ runs from the element entrance ($z = 0$) to the map length $L$; $x$ and $y$ are transverse coordinates relative to the beam axis, all in m.
- **Data versus schematic**: a field map is data; the fields the interface draws for matrix-model elements (`quad`, `solenoid`, `bend`, `steerer`) are hard-edge schematics computed from the parameters. The two must not be confused; see [In AVAS](#field-maps-in-avas).

## Why integrate in the map {#field-maps-tcode}

The transfer matrices of [Linear optics](#linear-optics) rest on a few assumptions: the field is linear in the transverse coordinates, the particle energy (and hence $B\rho$) hardly changes inside the element, and the element's action can be described by a few equivalent parameters. An RF cavity breaks all of them: in the example cavity the proton's β changes by 10 %, energy gain and focusing depend on the phase, and the HWR field is not exactly axially symmetric (see [RF cavity](#rf-cavity-energy-gain)). In a strong solenoid the beam size changes inside the field, and the hard-edge equivalent is off by about 10 % (see [Electrostatic and magnetostatic field maps](#static-fields-equations)).

AVAS does not try to find equivalent matrices but integrates the equations of motion directly: inside a field-map element, with time as the independent variable (that is what "t-code" means), each step takes $K_e\boldsymbol{E}$ and $K_b\boldsymbol{B}$ at the particle's current position (RF fields times their time factors) and updates momentum and position with the Lorentz force. The time step is set by `steppercycle` in input.txt: the example uses 50, i.e. 50 steps per RF period, $\Delta t = 1/(50f) \approx 0.123\ \mathrm{ns}$; a proton moves $\beta\lambda/50 \approx 2.1\ \mathrm{mm}$ per step and needs about 100 steps for the 0.21 m cavity.

The price is computing time and a dependence on the quality of the map: every step interpolates the field between grid points, and noise and grid errors in the map turn directly into forces on the particles. The next three sections deal with exactly these issues.

> **Think about it**: what goes wrong if `steppercycle` is changed from 50 to 10?
>
> Answer: each step becomes about 10.5 mm, while one gap of hwr010 has a field-peak half-height width of only about 40 mm; a gap is sampled by only about four steps and the integration error grows clearly. The step is also much larger than the 2 mm grid, so details of the grid are skipped. Conversely, steps much smaller than the grid spacing do not gain information; they only walk more finely through the interpolated field.

## The grid: what the sampling can resolve {#field-maps-grid}

### Grid spacing

The grid spacing sets the shortest structure a map can resolve, just as the sampling rate sets the highest frequency a signal can keep. As a rule of thumb:

- The longitudinal spacing should be much smaller than the length over which the field changes (gap length, fringe length), and not larger than the order of the distance a particle covers per integration step. The 2 mm grid of hwr010 has about 52 points per $\beta\lambda = 0.105\ \mathrm{m}$ and about 20 within the half-height width of one gap (about 40 mm); comparable with the 2.1 mm step of `steppercycle 50`.
- The transverse spacing sets the accuracy of the near-axis gradients (the focusing forces). The example's 1.7–2 mm transverse spacing gives only 10–12 intervals from the axis to 2 cm: enough for linear gradients, limited for higher-order components.

### Transverse range and aperture

The transverse range of the example maps is the square $\lvert x\rvert, \lvert y\rvert \le 2\ \mathrm{cm}$, and the element aperture `R` is also 0.02 m: every point in the round aperture lies inside the map, and the corners of the square (up to $r \approx 2.83\ \mathrm{cm}$) have data too. Particles beyond the aperture radius are counted as lost; **the transverse range of the map should cover the whole aperture**, otherwise there are regions inside the aperture without field data. How the engine treats positions outside the map is not documented in the manual.

### Map length and element length

The `L` of `field` should equal the $L$ in the file header. How the engine handles a mismatch is not documented; the linear preview uses only the field within $[0, \min(L, L_{map})]$. If a field must reach into the neighbouring element (a long fringe field, for example), use [superposed fields](#superpose) rather than truncating the map.

### Map ends

At both ends of the map the field should have decayed to negligible values: at the ends of the example `hwr010` the on-axis $E_z$ is about 0.5–0.7 % of its peak, at the ends of `sol` $B_z$ is about 0.3 %. Cutting the map where the field is still large amounts to switching the field off abruptly at the element boundary, and particles receive a spurious edge kick. Solenoids are especially sensitive: $B_r = -\tfrac{r}{2}B_z'$, and an abrupt cut means an infinite $B_z'$ at the boundary.

## In which order the numbers are stored {#field-maps-order}

The header gives only the number of points in each direction, not the order of the values. The manual says nothing about it; AVAS recognizes two orders:

- **z outermost**: the whole cross-section at $z = z_0$, then the whole cross-section at $z = z_1$ …, with $x$ varying fastest within a cross-section.
- **z innermost**: for each transverse point, the full line along $z$.

Both readings give arrays of the same shape, but the values land in completely different places. How to tell which is right? Use physics: a real field is smooth along $z$ (it satisfies Maxwell's equations and has no reason to jump within one grid spacing), while the wrong reading mistakes transverse variation for longitudinal variation and looks rough along $z$. AVAS computes a "roughness" along $z$ for each reading, the ratio of the summed absolute second differences to the summed absolute first differences, and uses z innermost only when its roughness is clearly smaller (below 0.9 times that of z outermost); otherwise z outermost.

**The decision is made for a group of component files as a whole** (`group_order()` in `avas/data/fieldmap.py`), not file by file. A single component can look alike in both readings, for example one transverse component of a quadrupole map; all component files of one map necessarily share the order, so a vote over the group is more reliable. For the example:

| Map | Sum of roughness, z outermost | Sum of roughness, z innermost | Decision |
|---|---|---|---|
| hwr010 (6 components) | 9.10 | 9.64 | z outermost |
| sol (3 components) | 5.01 | 5.07 | z outermost |

The noise of the example maps makes the distinction weak: the two readings of sol differ by only about 1 %, and `sol.bsx` alone even looks slightly smoother z innermost (1.59 against 1.64). Roughness is therefore only the first filter; it must be confirmed with physical criteria (is the Bz peak at the element centre, and so on), see the [checklist](#field-maps-checks). So far all 40 engine maps of two projects that were checked are stored z outermost; **write your own maps z outermost**.

> **Think about it**: why does more noise make it harder for the roughness to tell the two orders apart?
>
> Answer: roughness measures how large the second differences along $z$ are compared with the first differences. If the data are noisy from point to point, the second differences are large even in the right reading, and the difference between right and wrong reading is drowned. The distinction comes from the smoothness of the real field, and noise destroys exactly that smoothness.

## Interpolation, Maxwell's relations and noise {#field-maps-interpolation}

### Interpolation

A particle at an integration step almost never sits exactly on a grid point, so the field has to be interpolated. How the engine interpolates between grid points is not documented in the manual; the engine is authoritative. The AVAS front and back end do the following:

- The linear preview interpolates the on-axis values linearly along $z$, and takes transverse derivatives on the axis by central differences (the difference of the two neighbouring points when the axis is not on a grid point).
- Interface slices take the nearest grid plane without interpolation; for speed at most 400×240 grid lines are shown (the first and last are always kept).

### Relations the map should satisfy

A map comes from a numerical solution and should satisfy Maxwell's equations to within the discretization accuracy. Near the axis:

$$
E_r \approx -\frac{r}{2}\frac{\partial E_z}{\partial z},\qquad
B_r \approx -\frac{r}{2}\frac{\partial B_z}{\partial z}\quad(\text{axially symmetric, paraxial})
$$

In an RF cavity E and B are also linked by Faraday's and Ampère–Maxwell's laws. With AVAS's convention the fields that act are $K_e\boldsymbol{E}_{map}(\boldsymbol{r})\cos(\omega t+\varphi)$ and $K_b\boldsymbol{B}_{map}(\boldsymbol{r})\sin(\omega t+\varphi)$; inserting them gives

$$
K_e\,\nabla\times\boldsymbol{E}_{map} = -\omega K_b\,\boldsymbol{B}_{map},\qquad
K_b\,\nabla\times\boldsymbol{B}_{map} = -\frac{\omega}{c^2}\,K_e\,\boldsymbol{E}_{map}
$$

so a map fixes not only the shapes but also the ratio $K_b/K_e$ that makes E and B one solution. When transverse component files are missing, the linear preview derives the transverse gradients from the on-axis component with the first pair of relations; for static magnetic maps the radial part (the solenoid fringe) always comes from the on-axis $B_z$, and the map's $B_x$, $B_y$ only supply the divergence-free part.

These relations are also tools for checking a map, but beware of noise. Try the first one on hwr010: the map's own $E_x(x{=}1\,\mathrm{cm})$ has a maximum of 1.89 MV/m along $z$ (raw map value); differentiating the on-axis $E_z$ after a 5-point moving average gives a maximum of $\tfrac{r}{2}\lvert\partial E_z/\partial z\rvert$ of 2.04 MV/m, about 8 % apart, so the transverse and longitudinal components are consistent. Differentiating the raw data directly gives 5.4 MV/m instead: the noise is amplified almost threefold by the derivative.

Integrals are much more robust than derivatives. Apply the second relation in integral form (Stokes): around a square of side 1.2–2 cm centred on the axis, the circulation $\oint\boldsymbol{B}_{map}\cdot\dd\boldsymbol{l}$ follows $+(\omega/c^2)\int E_{z,map}\,\dd A$ along $z$ with a correlation of 0.998, and integrated over each gap the two agree to within 2 %. The plus sign means $K_b/K_e = -1$: the example's $K_b = -K_e$ is exactly what makes the RF electric and magnetic field of hwr010 a consistent Maxwell solution under AVAS's cos / sin convention (the map evidently stores $\boldsymbol{B}$ as the amplitude of a field varying as $-\sin$, like $B_\theta$ of the [pillbox mode](#rf-cavity-modes)). The same data also show how far the HWR is from axial symmetry: near the axis $\partial B_x/\partial y \approx -0.042\ \mathrm{T/m}$ in the first gap, while $\partial B_y/\partial x \approx -0.002\ \mathrm{T/m}$; an axially symmetric gap would split the same circulation equally, $\partial B_x/\partial y = -0.020$ and $\partial B_y/\partial x = +0.020\ \mathrm{T/m}$ (map values, Ke = 1). The difference is a curl-free, quadrupole-like part, the RF quadrupole component of an HWR mentioned in [RF cavity](#rf-cavity-modes); for a proton at $\beta = 0.057$ the magnetic force is only about 1 % of the electric one, so it matters little here.

### Noise of the example maps

The example maps `hwr010` and `sol` are noticeably noisy (verified 2026-09-21): the mean second difference of the on-axis $E_z$ and $B_z$ along $z$ is about 7 % of their range, and `sol.bsz` jumps between 5.9 and 10.1 from point to point within $\pm 1.5\ \mathrm{cm}$ of its peak (the map has no flat top: smoothed peak 8.3 at $z \approx 0.177\ \mathrm{m}$, half-height width about 7.4 cm). The slice heat map shows this as stripes along $z$; **that is the data itself, not a reading or slicing error**. Its effect on calculations depends cleanly on the type of quantity:

- **Linear quantities** ($\int E_z\,\dd z$, $\int B_z\,\dd z$) are hardly affected: the noise cancels between positive and negative values.
- **Squared quantities** ($\int B_z^2\,\dd z$, which sets the solenoid focusing) come out systematically high, about 3 % in the example: the square of the noise is always positive.
- **Derivatives** (near-axis gradients, curls from finite differences) are amplified most, almost threefold in the example above. Do not use numerical derivatives of these maps to "verify" Maxwell's equations; in the example, Faraday's relation $\nabla\times\boldsymbol{E} = -\omega\boldsymbol{B}$ taken by direct differences is completely drowned by noise. Use integral forms instead, as above.
- **Maxima** are not peak fields: the raw maximum of `sol.bsz` is about 20 % above the 9-point smoothed peak.

> **Think about it**: if the noise is so obvious, why not smooth the maps in AVAS first?
>
> Answer: smoothing acts on each component separately and breaks the Maxwell relations between components (for example $\nabla\cdot\boldsymbol{B} = 0$); it also changes the shape of the fringe fields, which are the very reason for using a map rather than a hard-edge model. Map quality should be fixed at the source, by exporting with a fine enough grid and converged solver accuracy. Every number in this primer obtained after smoothing (such as the peak field of sol) says how it was smoothed.

## How to tell whether a map can be trusted {#field-maps-checks}

Before using a new map for calculations, check it item by item. Behind each item is a piece of physics from the sections above:

1. **Header and amount of data**: the number of values equals $(N_z+1)(N_x+1)(N_y+1)$; the header length equals `L` in the lattice; all components of one map share the grid.
2. **Complete components**: an RF cavity needs at least one complete set of three electric or magnetic components; static fields need all three. The Lattice page warns "Field map … not found (… needed)".
3. **Longitudinal position** (tests the storage order): the on-axis $B_z$ peak of a solenoid should be at the element centre (example `sol`: peak at $z \approx 0.177\ \mathrm{m}$, centre 0.175 m); the $E_z$ of a two-gap cavity should be antisymmetric about the centre (example `hwr010`: zero crossing at $z \approx 0.104\ \mathrm{m}$, centre 0.105 m). If the peak sits near one end or shows a periodic saw-tooth, suspect the storage order first.
4. **Transverse direction** (tests $\nabla\cdot\boldsymbol{B} = 0$ and the signs): in a cross-section at the entrance of a solenoid $(B_x, B_y)$ should point towards the axis, at the exit away from it, and nearly vanish in the central section (verified for the example, deviations at noise level); the size should match $\tfrac{r}{2}\lvert B_z'\rvert$. With noisy maps expect only rough agreement, because $B_z'$ itself is uncertain: for `sol` at $x = 1\ \mathrm{cm}$ the map gives $B_x = -0.55$ and $-0.99$ at $z = 0.121$ and $0.1435\ \mathrm{m}$, while $-\tfrac{r}{2}B_z'$ of the 9-point smoothed on-axis field gives $-0.59$ and $-1.44$ (map units, Kb = 1).
5. **Symmetry**: an HWR is up–down symmetric about the beam axis; in a quadrupole $B_y(x)$ and $B_x(y)$ have the same sign and slope; cross-sections are viewed "from downstream towards upstream".
6. **Decay at the ends**: at both ends the field should have dropped below one per cent of the peak.
7. **Physical quantities**: for an RF cavity, $\int\lvert E_z\rvert\,\dd z$ and the position of the maximum of the transit-time factor $T(\beta)$ should match the design (example: $T$ peaks at $\beta \approx 0.10$); for a solenoid, $\int B_z\,\dd z$ should match the magnet design.
8. **Trial run**: check energy gain and focal length with the linear preview or a short multi-particle run before production calculations.

## Common misconceptions {#field-maps-pitfalls}

- **"Stripes in the heat map mean it was read wrongly"**: stripes may come from a wrong storage order or from noise in the data itself (as in the example); items 3 and 4 of the [checklist](#field-maps-checks) tell them apart.
- **"The denser the map, the better"**: a grid much finer than the integration step does not improve accuracy; the accuracy limit of a map is the convergence of the solver itself. A grid that is too coarse is the real problem.
- **"The maximum in the map is the peak field"**: noise pushes the raw maximum up (about 20 % for the example sol); a cavity map does not contain the cavity surface either, so $E_{pk}$ cannot be read from it (see [RF cavity](#rf-cavity-in-avas)).
- **"Ke and Kb are just unit conversions"**: they are field amplitudes, the operating point of a cavity and the excitation of a magnet; a map can be scaled to any field strength only while everything is linear (no saturation).
- **"A transverse range smaller than the aperture does not matter"**: there are then regions inside the aperture without field data, and the manual does not say how the engine treats them.
- **"Truncate maps to avoid overlap"**: if the field is not small at the cut, a spurious edge kick results; overlapping fields belong in [superposed fields](#superpose).
- **Arrow directions**: when the two axes have different scales, the arrows in the viewer stretch with the plot; only with "Equal scales" do they show the true direction.

## In AVAS {#field-maps-in-avas}

### Text format

```text
Nz  L
Nx  xmin  xmax
Ny  ymin  ymax
norm
v1
v2
...
```

The first four lines are the header: $N_z$, $N_x$, $N_y$ are the numbers of **grid intervals** in the three directions (there are $N+1$ grid points), $L$ is the map length (m), $x_{min}$ … $y_{max}$ the transverse range (m), `norm` a normalization factor. Then follow $(N_z+1)(N_x+1)(N_y+1)$ values, one per line or several separated by white space.

The header of the example `hwr010.edz` is

```text
105 2.100000000000000E-001
20 -2.000000000000000E-002 2.000000000000000E-002
20 -2.000000000000000E-002 2.000000000000000E-002
1
```

i.e. $106\times 21\times 21 = 46\,746$ points with 2 mm spacing in all three directions; the header of `sol` is `200 0.35`, `24 ±0.02`, `24 ±0.02`, `1`, giving $201\times 25\times 25 = 125\,625$ points, 1.75 mm apart longitudinally and 1.67 mm transversely.

### Binary format

If the file does not start with plain numeric text it is read as little-endian binary: `int32 Nz`, `float64 L`, `int32 Nx`, `float64 xmin`, `float64 xmax`, `int32 Ny`, `float64 ymin`, `float64 ymax`, `float64 norm` (60 bytes, no padding), followed by $(N_z+1)(N_x+1)(N_y+1)$ `float32` values. A binary file needs only 4 bytes per value, much less than text (about 11 bytes per value in the example), and reads faster.

### Normalization factor

The AVAS interface and the linear preview multiply the values read by `norm`. All example maps have `norm` = 1; how the engine uses this line is not documented in the manual. **Write `norm` as 1 when exporting** and set the field strength with Ke and Kb only.

### Extensions

| Extension | Component | Field type | Unit |
|---|---|---|---|
| `.edx` `.edy` `.edz` | RF electric field $E_x$, $E_y$, $E_z$ | 1 | MV/m, times Ke, varies as cos |
| `.bdx` `.bdy` `.bdz` | RF magnetic field $B_x$, $B_y$, $B_z$ | 1 | T, times Kb, varies as sin |
| `.esx` `.esy` `.esz` | Electrostatic field $E_x$, $E_y$, $E_z$ | 2 | MV/m, times Ke |
| `.bsx` `.bsy` `.bsz` | Magnetostatic field $B_x$, $B_y$, $B_z$ | 3 | T, times Kb |

Mnemonic: `e`/`b` for electric/magnetic, `d`/`s` for dynamic (RF)/static, the last letter is the component. When the interface checks that a map is complete, an RF cavity needs at least one complete set of three electric or magnetic components, electrostatic and magnetostatic maps need all three of theirs.

> **AVAS convention**: E and B of an RF field come from the same eigenmode; the phase relation between them on export (and hence the sign) is set by the solver. AVAS always uses cos for E and sin for B, and sign differences are compensated by the sign of Kb. The example uses $K_b = -K_e$; the manual does not explain the minus sign, but the map itself confirms it (the circulation test in [Relations the map should satisfy](#field-maps-interpolation)). For a new map run the same test to find the sign of $K_b/K_e$; see [RF cavity](#rf-cavity-in-avas).

### Field map directory

The engine looks for maps by file name in the **field map directory**:

- by default the project's `InputFile/`;
- it can be set on the Settings page under "Field maps → Directory", stored as `[project] fieldSource` in ini.ini (empty means InputFile);
- on the command line `avas run --input DIR --output DIR --field DIR` sets it with `--field`; without it the ini.ini setting is used, and failing that the input directory.

For a run AVAS copies the text inputs to `<output>/inputs/` and the engine uses the copies, but the maps are still read from the original field map directory, so the large files are not copied. When a map is not found the Lattice page warns, and the linear preview treats the element as a drift and says so.

> **Note**: field map files are large (about 0.5–1.4 MB of text per component in the example). When several projects share maps, keep them in one common directory and point fieldSource to it rather than copying them into every project.

### Viewing maps in the interface

**Files page**: selecting any component file shows its information (meaning of the component, text/binary, length, grid, transverse range, normalization factor, other components with the same name, which lattice elements use it) and two views:

- **Along z**: the on-axis value ($x = y = 0$) and the maximum of $\lvert F\rvert$ over each cross-section as functions of $z$.
- **Slice**: a heat map of the longitudinal cuts $z$–$x$ (fixed $y$), $z$–$y$ (fixed $x$) or the cross-section $x$–$y$ (fixed $z$), with a movable cut position. Components of the same map on the same grid are cut together, so the magnitude and in-plane arrows can be shown.

**Field window of an element**: select an element on the Lattice page or in the visual editor; the element frame has a "View field map / View field distribution" button in its top right corner. For a field-map element it opens **data** (map slices), for a matrix-model element a **schematic** computed from the parameters (hard edge, zero outside the element, labelled as schematic). Both use the same views; while the window is open, selecting another element updates it, and dragging a parameter redraws it live.

**Reading conventions**:

- **Colours**: signed components use a colour scale centred on 0, symmetric for positive and negative; magnitudes use a one-sided scale.
- **Viewing direction**: all cross-sections are **viewed from downstream towards upstream, with the beam coming out of the screen**, marked by the ⊙ at the centre. Field arrows and force arrows in the same picture can then be checked against each other with $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$: $\boldsymbol{v}$ points at the reader.
- **Arrows**: arrow length is proportional to the field components in data coordinates, so with different axis scales the arrows stretch with the plot; only with "Equal scales" ticked do they show the true direction. In longitudinal cuts the element is long and becomes a thin strip at equal scale.
- **Stripes**: stripes along $z$ may come from a wrong storage order or from noise in the data itself (as in the example); the [checklist](#field-maps-checks) tells them apart.

### Try it

1. In the hwr010 viewer below, choose the $z$–$x$ longitudinal cut of `edz` and find the two gaps of opposite sign; then choose the $x$–$y$ cross-section, move it to the centre of each gap and compare how `edx` and `edy` change direction.

```avas-field
map hwr010
```

2. In the sol viewer below, choose the longitudinal cut of `bsz` and look at the stripes along $z$ (noise); switch to cross-sections near the entrance and the exit with arrows on, and carry out item 4 of the [checklist](#field-maps-checks).
3. Untick and tick "Equal scales" and compare the arrow directions, to see why arrows show the true direction only at equal scale.

```avas-field
map sol
```

## Further reading {#field-maps-reading}

- The TraceWin user manual on the 3D field-map format (FIELD_MAP): AVAS's text format is similar, but units and parameter meanings follow this chapter and [RF cavity](#rf-cavity).
- T. P. Wangler, *RF Linear Accelerators*, chapter 2: transit-time factor and paraxial fields from the on-axis field.
- M. Reiser, *Theory and Design of Charged Particle Beams*, chapter 3: paraxial expansion of axially symmetric fields.
- AVAS manual `docs/使用说明20260427.docx`: the `field` element and the field-map file format.
- In this primer: [Electrostatic and magnetostatic field maps](#static-fields), [Superposed fields](#superpose), [Simulation settings](#simulation), [References](#references).
