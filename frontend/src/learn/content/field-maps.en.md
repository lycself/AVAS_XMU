# Field maps

A field map is a table of electric or magnetic field components on a three-dimensional grid, usually computed and exported by an electromagnetic solver (CST, COMSOL, Opera, Superfish …). AVAS's `field` element reads a field map and integrates the particle motion with the t-code in the field interpolated from the grid: RF cavities (type 1), electrostatic elements (type 2) and magnetostatic elements (type 3) are all described this way, see [RF cavities](#rf-cavity) and [Static field maps](#static-fields). This chapter explains the file format and units, grid and aperture, the storage order of the component files, interpolation and noise, and how to view and check a map in the GUI.

The example project `examples/hwr010/InputFile` contains two maps: the half-wave cavity `hwr010` (`.edx/.edy/.edz/.bdx/.bdy/.bdz`, 0.21 m long) and the solenoid `sol` (`.bsx/.bsy/.bsz`, 0.35 m long). `efield.*` and `bfield.*` are byte-identical copies of these two groups.

## What a field map describes {#field-maps-role}

- **One field map = one group of component files with the same name.** The last parameter of `field` is the file name without extension; the kernel reads the extensions belonging to the field type.
- **The shape of the field comes from the files, the amplitude from Ke and Kb.** Electric fields in the files are in MV/m and magnetic fields in T; the field acting on the beam is $K_e\boldsymbol{E}_{map}$, $K_b\boldsymbol{B}_{map}$. RF fields are also multiplied by the time factor: $\cos(\omega t+\varphi_0)$ for the electric and $\sin(\omega t+\varphi_0)$ for the magnetic field.
- **Coordinates**: $z$ runs from the element entrance ($z = 0$) to the map length $L$; $x$ and $y$ are transverse coordinates relative to the beam axis, all in m.
- **Data versus schematic**: a field map is data and contains every detail the solver produced (fringe fields, higher-order components, grid noise); the fields the GUI draws for matrix-model elements (`quad`, `solenoid`, `bend`, `steerer`) are hard-edge schematics computed from the parameters. The two must not be confused, see [Field maps in the GUI](#field-maps-gui).

## File format and units {#field-maps-format}

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

The first four lines are the header: $N_z$, $N_x$, $N_y$ are the **numbers of grid intervals** in the three directions (the number of grid points is $N+1$), $L$ is the map length (m), $x_{min}$ … $y_{max}$ the transverse range (m), and `norm` a normalization factor. Then follow $(N_z+1)(N_x+1)(N_y+1)$ values, one per line or several separated by white space.

The header of the example `hwr010.edz` is

```text
105 2.100000000000000E-001
20 -2.000000000000000E-002 2.000000000000000E-002
20 -2.000000000000000E-002 2.000000000000000E-002
1
```

i.e. $106\times 21\times 21 = 46\,746$ points with a grid spacing of 2 mm in all three directions; the header of `sol` is `200 0.35`, `24 ±0.02`, `24 ±0.02`, `1`: $201\times 25\times 25 = 125\,625$ points, 1.75 mm longitudinally and 1.67 mm transversely.

### Binary format

A file that does not start with plain numeric text is read as little-endian binary: `int32 Nz`, `float64 L`, `int32 Nx`, `float64 xmin`, `float64 xmax`, `int32 Ny`, `float64 ymin`, `float64 ymax`, `float64 norm` (60 bytes, no padding), followed by $(N_z+1)(N_x+1)(N_y+1)$ `float32` values. Each value takes only 4 bytes, much less than in text (about 11 bytes per value in the example), and reading is faster.

### The normalization factor

The AVAS GUI and the linear preview multiply the values read by `norm`. All example maps have `norm` = 1; how the kernel uses this line is not documented in the manual. **Export maps with `norm` = 1** and set the field strength with Ke and Kb only.

### Extensions

| Extension | Component | Field type | Units |
|---|---|---|---|
| `.edx` `.edy` `.edz` | RF electric field $E_x$, $E_y$, $E_z$ | 1 | MV/m, times Ke, varies as cos |
| `.bdx` `.bdy` `.bdz` | RF magnetic field $B_x$, $B_y$, $B_z$ | 1 | T, times Kb, varies as sin |
| `.esx` `.esy` `.esz` | static electric field $E_x$, $E_y$, $E_z$ | 2 | MV/m, times Ke |
| `.bsx` `.bsy` `.bsz` | static magnetic field $B_x$, $B_y$, $B_z$ | 3 | T, times Kb |

Mnemonic: `e`/`b` electric/magnetic, `d`/`s` dynamic (RF)/static, the last letter the component. When the GUI checks whether a map is complete, an RF cavity needs at least one complete triplet (electric or magnetic), static electric and static magnetic maps their three components.

> **AVAS convention**: the E and B of an RF field come from the same eigenmode, and the phase relation between them in the export (and the sign that follows from it) is set by the solver; AVAS always uses cos for E and sin for B, and a sign difference is compensated by the sign of Kb. The example uses $K_b = -K_e$, see [RF cavities](#rf-cavity-in-avas).

## Grid, extent and aperture {#field-maps-grid}

### Grid spacing

The grid spacing sets the shortest structure a map can resolve. As a rule of thumb:

- The longitudinal spacing should be much smaller than the length over which the field changes (gap length, fringe length), and not larger than the order of the distance a particle travels per integration step. In the example, with `steppercycle 50` and $\beta = 0.057$ a step is about $\beta\lambda/50 \approx 2.1\ \mathrm{mm}$, comparable to the 2 mm grid of hwr010.
- The transverse spacing sets the accuracy of the near-axis gradients (the focusing forces). The 1.7–2 mm of the example give only about 10 intervals within $\pm 2\ \mathrm{cm}$: enough for linear gradients, limited for higher-order components.

### Transverse extent and aperture

The example maps cover the square $\lvert x\rvert, \lvert y\rvert \le 2\ \mathrm{cm}$ and the element aperture `R` is also 0.02 m: every point inside the round aperture is inside the map, and the corners of the square (up to $r \approx 2.83\ \mathrm{cm}$) also have data. A particle beyond the aperture radius counts as lost; **the transverse extent of the map should cover the whole aperture**, otherwise there are regions inside the aperture without field data, and what the kernel does outside the map is not documented.

### Map length and element length

The `L` of `field` should equal the $L$ in the header. What the kernel does when they differ is not documented; the linear preview uses the field in $[0, \min(L, L_{map})]$ only. To let a field extend into a neighbouring element (e.g. a long fringe), use [Superpose](#superpose) instead of truncating the map.

### The ends of a map

The field should have decayed to negligible values at both ends: the on-axis $E_z$ of `hwr010` is about 0.5–0.7 % of the peak at the ends, the $B_z$ of `sol` about 0.3 %. Cutting a map where the field is still large is like switching the field off abruptly at the element boundary, and the particles get a spurious edge kick (especially for solenoids, since $B_r \propto -\tfrac{r}{2}B_z'$).

## Storage order of the component files {#field-maps-order}

The header gives the number of points in each direction but not the order of the values. The manual is silent about it; AVAS recognizes two orders:

- **z outermost**: the whole cross-section at $z = z_0$, then the whole cross-section at $z = z_1$, …, with $x$ varying fastest inside a cross-section.
- **z innermost**: for every transverse point, the whole line along $z$.

Both readings give arrays of the same shape, but the values land in completely different places. AVAS decides by the "roughness along $z$": for each reading it computes the ratio of the summed absolute second differences to the summed absolute first differences along $z$. The real field is smooth along $z$; a wrong reading turns transverse variations into longitudinal ones and looks rough. z innermost is used only when it is clearly smoother (less than 0.9 times the z-outermost roughness); otherwise z outermost.

**The decision is made for a whole group of component files** (`group_order()` in `avas/data/fieldmap.py`), not file by file. A single component can look alike in both readings, for example one transverse component of a quadrupole map; all component files of one map necessarily share the order, so a vote over the group is more reliable. For the examples:

| Map | sum of roughness, z outermost | sum of roughness, z innermost | decision |
|---|---|---|---|
| hwr010 (6 components) | 9.10 | 9.64 | z outermost |
| sol (3 components) | 5.01 | 5.07 | z outermost |

The noise of the example maps makes the distinction weak (only about 1 % for sol, and `sol.bsx` alone is even slightly smoother z innermost), so physical criteria are needed as a cross-check, see the [checklist](#field-maps-checks). All 40 kernel maps of two projects checked so far are stored z outermost; **write your own maps z outermost**.

## Interpolation, Maxwell's equations and noise {#field-maps-interpolation}

### Interpolation

How the kernel interpolates between grid points is not documented in the manual; the kernel is the reference. In AVAS's front and back end:

- The linear preview interpolates the on-axis values linearly along $z$ and takes the transverse derivatives on the axis by central differences (by the difference of the two neighbouring points when the axis is not a grid point).
- The GUI slices take the nearest grid plane without interpolation; for speed at most 400×240 grid lines are kept for display (the first and last always).

### Relations a map should satisfy

A map comes from a numerical solution and should satisfy Maxwell's equations to the accuracy of the discretization. Near the axis:

$$
E_r \approx -\frac{r}{2}\frac{\partial E_z}{\partial z},\qquad
B_r \approx -\frac{r}{2}\frac{\partial B_z}{\partial z}\quad(\text{axisymmetric, paraxial})
$$

In an RF cavity E and B are also linked by Faraday's law: with AVAS's convention $\boldsymbol{E}(\boldsymbol{r})\cos(\omega t+\varphi)$, $\boldsymbol{B}(\boldsymbol{r})\sin(\omega t+\varphi)$ one has $\nabla\times\boldsymbol{E}(\boldsymbol{r}) = -\omega\,\boldsymbol{B}(\boldsymbol{r})$. When the transverse component files are missing, the linear preview derives the transverse gradients from the on-axis component with the first relations.

### Noise in the example maps

The example maps `hwr010` and `sol` are noticeably noisy themselves (checked 2026-09-21): the mean second difference of the on-axis $E_z$ and $B_z$ along $z$ is about 7 % of the range, and `sol.bsz` jumps from point to point in its central region. Slice heat maps show this as stripes along $z$ — **that is the data, not an error of reading or slicing**. Effects on calculations:

- linear quantities ($\int E_z\,\dd z$, $\int B_z\,\dd z$) are hardly affected;
- squared quantities ($\int B_z^2\,\dd z$, which sets the solenoid focusing) are raised systematically, by about 3 % in the example;
- derivatives (near-axis gradients, curls computed by differences) amplify the noise most; do not try to "verify" Maxwell's equations on these maps by numerical differentiation;
- the maximum of the raw data is not the peak field: the raw maximum of `sol.bsz` is about 20 % above the peak after 9-point smoothing.

> **Note**: for production calculations export maps with a fine enough grid and a converged solver accuracy; do not smooth maps outside AVAS on your own — smoothing breaks the Maxwell relations and changes the fringe fields.

## Field maps in the GUI {#field-maps-gui}

### Files page

Selecting any component file on the Files page shows the file information (meaning of the component, text/binary, length, grid, transverse range, normalization, the other components with the same name, the lattice elements that use it) and two views:

- **Along z**: the on-axis value ($x = y = 0$) and the largest $\lvert F\rvert$ of each cross-section versus $z$.
- **Slices**: a heat map, either a longitudinal cut $z$–$x$ (fixed $y$), $z$–$y$ (fixed $x$) or a cross-section $x$–$y$ (fixed $z$), with a movable position. Components of the same map with the same grid are cut together, so the magnitude and in-plane arrows can be shown.

### Field window of an element

When an element is selected on the Lattice page or in the visual editor, its frame has a "View field map / View field distribution" button in the top right corner: a field-map element opens **data** (slices of the map), a matrix-model element a **schematic** computed from its parameters (hard edges, zero outside the element, labelled as schematic). Both use the same views; while the window is open, selecting another element updates it and dragging a parameter redraws it live.

### Conventions for reading the pictures

- **Colours**: signed components use a colour scale centred on 0, symmetric for both signs; magnitudes a one-sided scale.
- **Viewing direction**: all cross-sections are drawn **looking from downstream to upstream, with the beam coming out of the screen**; the ⊙ mark at the centre shows this direction. Field arrows and force arrows in the same picture can then be checked directly with $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$: $\boldsymbol{v}$ points at the reader.
- **Arrows**: arrow lengths are proportional to the field components in data coordinates, so with different axis scales the arrows are stretched with the picture; only with "Equal scales" ticked do they show the true direction. A long element then becomes a thin strip in a longitudinal cut.
- **Stripes**: stripes along $z$ can come from a wrong storage order or from noise in the data itself (as in the examples); the [checklist](#field-maps-checks) tells them apart.

```avas-field
map hwr010
```

## Checklist {#field-maps-checks}

Before using a new field map for calculations, check item by item:

1. **Header and amount of data**: the number of values equals $(N_z+1)(N_x+1)(N_y+1)$; the header length equals the `L` in the lattice; all components of a map have the same grid.
2. **Complete components**: an RF cavity has at least one complete electric or magnetic triplet; static maps have all three components. The Lattice page warns "Field map … not found (… needed)".
3. **Longitudinal position**: the on-axis $B_z$ of a solenoid should peak at the element centre (example `sol`: peak at $z \approx 0.177\ \mathrm{m}$, centre 0.175 m); the $E_z$ of a two-gap cavity should be antisymmetric about the centre (example `hwr010`: zero crossing at $z \approx 0.104\ \mathrm{m}$, centre 0.105 m). If the peak sits near one end or the profile looks like a periodic saw-tooth, suspect the storage order first.
4. **Transverse direction**: in the cross-sections of a solenoid $(B_x, B_y)$ should point to the axis at the entrance and away from it at the exit, with almost no transverse field in the central cross-section (checked on the example, deviations at the noise level); the magnitude should agree with $\tfrac{r}{2}\lvert B_z'\rvert$.
5. **Symmetry**: an HWR is up–down symmetric about the beam axis; in a quadrupole map $B_y(x)$ and $B_x(y)$ have the same sign and slope; remember the "downstream looking upstream" view.
6. **Decay at the ends**: at both ends the field should be below about one per cent of the peak.
7. **Physical quantities**: for an RF cavity, $\int\lvert E_z\rvert\,\dd z$ and the β at which the transit-time factor $T(\beta)$ peaks should match the design geometric beta (about 0.10 in the example); for a solenoid $\int B_z\,\dd z$ should match the magnet design.
8. **Trial run**: check energy gain and focal length with the linear preview or a short multi-particle run before production runs.

## Field-map directory {#field-maps-directory}

The kernel looks for field maps by name in the **field-map directory**:

- by default the project's `InputFile/`;
- it can be set on the Settings page under "Field maps → Directory", stored as `[project] fieldSource` in ini.ini (empty means InputFile);
- on the command line `avas run --input DIR --output DIR --field DIR` sets it with `--field`; without it the ini.ini setting is used, and without that the input directory.

When running, AVAS copies the text inputs to `<output>/inputs/` and the kernel uses the copies, but the field maps are still read from the original field-map directory, so the large files are not copied. If a map is missing, the Lattice page warns and the linear preview treats the element as a drift with a warning.

> **Note**: field-map files are large (a single component of the example is about 0.5–1.4 MB of text). When several projects share maps, put them in one common directory and point fieldSource to it rather than copying them into every project.

## Hands-on {#field-maps-lab}

1. In the hwr010 viewer above choose the $z$–$x$ cut of `edz` and find the two gaps of opposite sign; then choose the $x$–$y$ cross-section, move it to the two gap centres and compare how `edx` and `edy` change direction.
2. In the sol viewer below choose the longitudinal cut of `bsz` and look at the stripes along $z$ (noise); switch to cross-sections near the entrance and the exit with arrows on, and carry out item 4 of the [checklist](#field-maps-checks).
3. Untick and tick "Equal scales" and compare the arrow directions, to see why only equal scales show the true direction.

```avas-field
map sol
```

## Further reading {#field-maps-reading}

- The description of 3D field maps (FIELD_MAP) in the TraceWin user manual: AVAS's text format is similar, but units and parameter meanings are those of this chapter and of [RF cavities](#rf-cavity).
- T. P. Wangler, *RF Linear Accelerators*, chapter 2: transit-time factor and paraxial fields from the on-axis field.
- M. Reiser, *Theory and Design of Charged Particle Beams*, chapter 3: paraxial expansion of axisymmetric fields.
- In this primer: [Static field maps](#static-fields), [Superpose](#superpose), [Simulation settings](#simulation), [References](#references).
