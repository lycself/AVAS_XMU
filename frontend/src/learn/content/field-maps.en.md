# Field maps

The half-wave cavity hwr010 of the example project is written as 6 text files of 46 746 numbers each; the solenoid sol is 3 files of 125 625 numbers each. How can a few hundred thousand numbers stand in for a real cavity or magnet? Open the solenoid map in the viewer and the heat map shows vertical stripes along $z$: did the program read it wrongly, or is the data really like that? The file header only says how many points there are in each direction, not in which order the numbers are written; how does AVAS know how to read them?

This chapter answers these questions: what a field map is physically, why AVAS pushes the particles through a map in small steps instead of using a transfer matrix, what problems the grid, the storage order, interpolation and noise each bring, and how to check whether a new map can be trusted. The three uses of field maps are covered in [RF cavity](#rf-cavity) (field type 1) and [Electrostatic and magnetostatic field maps](#static-fields) (types 2 and 3).

The example project `examples/hwr010/InputFile` comes with two field maps: the half-wave cavity `hwr010` (`.edx/.edy/.edz/.bdx/.bdy/.bdz`, 0.21 m long) and the solenoid `sol` (`.bsx/.bsy/.bsz`, 0.35 m long). `efield.*` and `bfield.*` are byte-identical copies of these two sets. The two field-map elements below follow each other: the magnetostatic solenoid sol, then the RF cavity hwr010, one period of the guided tutorial. In the linear preview the solenoid focuses the beam to a waist at $z \approx 0.55\ \mathrm{m}$ (rms x 0.85 mm); the cavity starts at $z = 0.50\ \mathrm{m}$ and raises the kinetic energy from 1.527 MeV to 1.884 MeV; at the end rms x is 1.65 mm, and with the cavity field switched off (Ke = Kb = 0) it would be 1.19 mm. The cavity's transverse defocusing of the beam is in the map too.

```avas-beamline
drift 0.05 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010
drift 0.2 0.02 0
```

The numerical examples of this chapter use the example project's proton beam: kinetic energy 1.527 MeV, $\beta = 0.0570$, RF frequency 162.5 MHz (wavelength $\lambda = c/f = 1.845\ \mathrm{m}$).

## What a field map is: a sampled solution of Maxwell's equations {#field-maps-role}

### A field written down on a grid

Imagine describing the temperature at every point of a room. There are infinitely many points, so you cannot write them all down; instead you place a thermometer every 2 mm and list the readings in a table. For the temperature between two thermometers you estimate a value from the readings on either side. A field map is such a table, only it records the electric or magnetic field.

Where do the numbers come from? When a cavity or magnet is designed, the engineers give its real geometry (conductor surfaces, coils, yoke) to an electromagnetic field program (such as CST, COMSOL, Opera, Superfish). Electromagnetic fields must obey **Maxwell's equations**, the complete form of Coulomb's law, Ampère's law and the law of induction from school physics together. The program solves Maxwell's equations numerically with these boundary conditions; the resulting field is continuous, and a **field map** samples this solution on a 3D grid around the beam channel, writing one table per component ($x$, $y$ and $z$ direction).

So a field map has no "physical model" of its own: it assumes no hard edge, no axial symmetry, no linearity. Everything the field program computed is in it: fringe fields, higher-order components, asymmetries, together with the program's discretization errors and grid noise. Both the strength and the weakness of a field map come from here: it is as real as the field program, and as rough.

First, see what a field map looks like. The viewer below shows the **data** of the hwr010 map: choose the $z$–$x$ longitudinal cut of `edz` (the longitudinal electric field) and you see two regions of opposite colour, the two accelerating gaps of the cavity, whose fields point in opposite directions at the same moment; then choose the $x$–$y$ cross-section, move it to the centre of each gap and compare how `edx` and `edy` change direction.

```avas-field
map hwr010
```

### Shape from the file, amplitude from Ke and Kb

Maxwell's equations in vacuum are linear: a solution multiplied by any constant is still a solution. In school you learned that the field inside a solenoid, $B = \mu_0 nI$, is proportional to the current $I$: double the current and the field doubles everywhere, keeping its shape. For an RF cavity the shape of a resonant mode is set by the geometry and its amplitude by the power fed in; for a static magnet without iron saturation the field is proportional to the current.

A field map therefore only has to give **the shape of the field**; the actual amplitude is set by Ke and Kb of the `field` element: the file holds electric fields in MV/m and magnetic fields in T, and the field acting is $K_e\boldsymbol{E}_{map}$, $K_b\boldsymbol{B}_{map}$. RF fields are also multiplied by an oscillating time factor: $\cos(\omega t+\varphi_0)$ for the electric field and $\sin(\omega t+\varphi_0)$ for the magnetic field, with $\omega = 2\pi f$. Why the two differ by $90^\circ$ is explained in [RF cavity](#rf-cavity-modes).

> **Think about it**: can a map of an iron-yoke quadrupole computed at one current be scaled with Kb to any gradient?
>
> Answer: only while the yoke is not saturated. How well iron carries a magnetic field depends on the field strength; once it saturates, the field is no longer proportional to the current and the shape (especially the fringe field and higher-order components) changes too. Linear scaling is then only an approximation, and the map should be recomputed at a current near the working point. RF cavities do not have this problem, but one map represents one mode only.

### Coordinates and data

- **One field map = one set of component files with the same name.** The last parameter of `field` is the file name without extension; the engine reads the extensions belonging to the field type.
- **Coordinates**: $z$ runs from the element entrance ($z = 0$) to the map length $L$; $x$ and $y$ are transverse coordinates relative to the beam axis, all in m.
- **Data versus schematic**: a field map is data; the fields the interface draws for matrix-model elements (`quad`, `solenoid`, `bend`, `steerer`) are hard-edge schematics computed from the parameters and are labelled "schematic". The two must not be confused, see [In AVAS](#field-maps-in-avas).

## Why integrate in the map {#field-maps-tcode}

### The assumptions of the transfer matrix break down

For the transfer matrices of [linear optics](#linear-optics) to hold, a few things must be true: the field is linear in the transverse coordinates, the particle energy (and so the magnetic rigidity $B\rho$) hardly changes inside the element, and the element's action can be described by a few effective parameters. An RF cavity breaks all of them: in the example cavity the proton speed changes by 10 %, energy gain and focusing both depend on the RF phase, and the HWR field is not perfectly axially symmetric (see [RF cavity](#rf-cavity-energy-gain)). In a strong solenoid the beam size already changes inside the field, and the hard-edge equivalent is off by about 10 % (see [Electrostatic and magnetostatic field maps](#static-fields-equations)).

### Pushing in small steps

AVAS does not look for an effective matrix; it applies Newton's law step by step, much as in school you draw the path of a projectile by working out its velocity and position again after every short interval. Inside a field-map element, at every small step:

1. at the particle's current position, take $K_e\boldsymbol{E}$ and $K_b\boldsymbol{B}$ from the map (RF fields times their time factor);
2. compute the Lorentz force $\boldsymbol{F} = q(\boldsymbol{E} + \boldsymbol{v}\times\boldsymbol{B})$;
3. update the momentum with "impulse = change of momentum", then move the particle a small step forward with the new velocity.

Each step is a fixed time interval, so this approach is called **t-code** (time as the independent variable). The time step is set by `steppercycle` in input.txt: the example uses 50, i.e. 50 steps per RF period, $\Delta t = 1/(50f) \approx 0.123\ \mathrm{ns}$; a proton moves $\beta\lambda/50 \approx 2.1\ \mathrm{mm}$ per step, about 100 steps through the 0.21 m cavity.

### Animation: particles through the example cavity

Below we borrow the animation of the [RF cavity](#rf-cavity) chapter. It uses the example HWR field map `hwr010` itself (Ke = 1.36, Kb = −1.36, synchronous phase $-30^\circ$, the example beam at 1.527 MeV and 0.184 mA).

```avas-motion
cavity
```

**What to look at**: the 3D view shows a see-through vertical tank; the inner conductor crosses the beam in the middle, carrying a drift tube, with an accelerating gap on either side. Each gap holds field lines with small cones: the electric fields of the two gaps point in opposite directions, and as the bunch moves on and the RF phase advances, the cones flip over and grow and shrink. This is "shape from the file, amplitude from Ke, time from the cosine": the field drawn is $K_e E_z(z)\cos\varphi$, where $E_z(z)$ is the map's on-axis longitudinal electric field. The bunch arrives at both gaps just when the field pushes forward; the panel shows an energy gain of 0.357 MeV and an RF phase at the entrance of $\varphi_0 = 56.6^\circ$; in the 2D phase space "z" the bunch length (rms z) shrinks from 1.32 mm to 1.17 mm.

**What to drag**: drag Ke from 1.36 to 0.68 and the energy gain drops from 0.357 MeV to 0.174 MeV, almost exactly half: the shape of the field has not changed, it has only been scaled down (not exactly half, because the particle speed changes less and the entrance phase has to be readjusted to $58.6^\circ$ to keep the synchronous phase at $-30^\circ$). Drag Ke to 0 and the energy no longer changes; the bunch just flies on with the energy spread it has, and rms z grows from 1.32 mm to 2.06 mm.

**What it does not show**: the particles in the animation are pushed by AVAS's linear model computed from the map: the energy gain uses the map's on-axis $E_z$, the focusing the field gradients near the axis. It is not the engine's step-by-step tracking described above. Off-axis particles feel a different $E_z$ in the 3D map than on the axis, and the engine's mean energy gain can differ from the linear model by a few per cent (see [RF cavity](#rf-cavity)). The tank, the inner conductor and the drift tube are a schematic drawing; the cones sit on five straight lines and their size follows the on-axis $E_z$, not the map's off-axis data; the cavity's RF magnetic field is not drawn.

### The cost

The price of step-by-step integration is computing time and dependence on the quality of the map: at every step the field between grid points has to be estimated (interpolated), and noise and grid errors in the map turn directly into forces on the particles. The next three sections are about these problems.

## The grid: what the sampling can resolve {#field-maps-grid}

### Grid spacing

The pixels of a digital photo decide how small a detail you can see: a hair thinner than a pixel does not show. The grid spacing of a field map likewise decides the shortest structure the map can resolve. As a rule of thumb:

- The longitudinal spacing should be much smaller than the length over which the field changes (gap length, fringe length) and not larger than the order of the distance a particle travels per integration step. The 2 mm grid of hwr010 has about 52 points per $\beta\lambda = 0.105\ \mathrm{m}$ and about 20 within the half-width (about 40 mm) of one gap's field peak, comparable to the 2.1 mm step of `steppercycle 50`.
- The transverse spacing sets the accuracy of the gradients near the axis (the focusing forces). The 1.7–2 mm transverse spacing of the example has only 10–12 intervals from the axis to 2 cm, enough for linear gradients but of limited resolution for higher-order components.

### Transverse range and aperture

The example maps cover a square $\lvert x\rvert, \lvert y\rvert \le 2\ \mathrm{cm}$, and the element aperture `R` is also 0.02 m: all points of the circular aperture lie inside the map, and the corners of the square (up to $r \approx 2.83$ cm) have data too. A particle beyond the aperture radius counts as lost; **the transverse range of the map should cover the whole aperture**, otherwise there are regions inside the aperture without field data, and the manual does not say what the engine uses outside the map.

### Map length and element length

The `L` of `field` should equal the $L$ in the file header. How the engine handles a mismatch is not documented; the linear preview uses only the field in $[0, \min(L, L_{map})]$. If the field has to reach into a neighbouring element (a long fringe field, say), use [superposed fields](#superpose) rather than truncating the map.

### Map ends

The field should have decayed to negligible values at both ends of the map: the on-axis $E_z$ of the example `hwr010` is about 0.5–0.7 % of the peak at the ends, the $B_z$ of `sol` about 0.3 %. Cutting the map where the field is still large amounts to switching the field off suddenly at the element boundary, and the particles get a spurious edge kick. Solenoids show it most: the radial field $B_r = -\tfrac{r}{2}\,\dd B_z/\dd z$ is proportional to the rate of change of $B_z$ along $z$ (see [Electrostatic and magnetostatic field maps](#static-fields-magnetic)), and a sudden cut means an infinite rate of change at the boundary.

## In which order the numbers are stored {#field-maps-order}

### One row of numbers, two ways to read it

A table of 3 rows and 4 columns can be written as a single row of 12 numbers either row by row or column by column. Given only the 12 numbers, without knowing which way they were written, you cannot put them back. Field-map files are in exactly this situation: the header gives only the number of points in each direction, not the order of the values. The manual says nothing about it; AVAS recognises two orders:

- **z outermost**: the whole cross-section at $z = z_0$, then the whole cross-section at $z = z_1$, …; within a cross-section $x$ varies fastest.
- **z innermost**: for each transverse point, the whole line along $z$.

Both readings give arrays of the same shape, but the values land in completely different places.

### Letting physics decide

How to tell which is right? Use physics: the real field is smooth along $z$ (it satisfies Maxwell's equations and has no reason to jump within one grid spacing), whereas the wrong reading mistakes transverse variation for longitudinal variation and looks rough along $z$. For each reading AVAS computes a "roughness" along $z$, the sum of the absolute second differences of three neighbouring points divided by the sum of the absolute first differences of two neighbouring points (formula in [Going deeper: sampling, interpolation and derivatives](#field-maps-deep-sampling)), and adopts z innermost only if its roughness is clearly smaller (less than 0.9 times that of z outermost); otherwise z outermost.

**The decision is made for a whole set of component files**, not file by file. A single component may look about the same in both readings, for example a transverse component of a quadrupole map; all component files of one map necessarily share the same order, so a vote of the whole set is more reliable. For the examples:

| Map | Roughness sum, z outermost | Roughness sum, z innermost | Decision |
|---|---|---|---|
| hwr010 (6 components) | 9.10 | 9.64 | z outermost |
| sol (3 components) | 5.01 | 5.07 | z outermost |

The noise of the example maps makes the distinction weak: the two readings of sol differ by only about 1 %, and `sol.bsx` on its own is even slightly smoother as z innermost (1.59 against 1.64). Roughness is therefore only a first filter and has to be confirmed with physical criteria (whether the $B_z$ peak falls at the element centre, and the like), see the [checklist](#field-maps-checks). All 40 engine maps of the two projects checked so far are stored z outermost; **write z outermost when you export maps yourself**.

## Interpolation, Maxwell's relations and noise {#field-maps-interpolation}

### Interpolation

Particles at an integration step almost never sit exactly on grid points, so the field has to be interpolated: like reading a table and estimating a value between two neighbouring entries in proportion. How the engine interpolates between grid points is not described in the manual; the engine is the reference. What AVAS's front and back end do:

- The linear preview interpolates the on-axis values linearly along $z$ and takes transverse derivatives on the axis by central differences (the difference of the two neighbouring points when the axis is not on a grid point).
- Interface slices take the nearest grid plane without interpolation; for speed the display keeps at most 400×240 grid lines (always including the first and last).

### Noise of the example maps

The example maps `hwr010` and `sol` are noticeably noisy themselves (verified 2026-09-21): the mean second difference of the on-axis $E_z$ and $B_z$ along $z$ is about 7 % of the range, and `sol.bsz` jumps from point to point between 5.9 and 10.1 within $\pm 1.5\ \mathrm{cm}$ of the peak (this map has no flat top: the smoothed peak is 8.3, at $z \approx 0.177\ \mathrm{m}$, with a half-width of about 7.4 cm). Choose the longitudinal cut of `bsz` in the sol viewer below and you see the vertical stripes along $z$. **That is the data itself, not a reading or slicing error.**

```avas-field
map sol
```

How the noise affects a calculation depends clearly on the kind of quantity:

- **Linear quantities** ($\int E_z\,\dd z$, $\int B_z\,\dd z$, i.e. areas under the curve) are hardly affected: the noise is sometimes positive and sometimes negative and cancels in the sum.
- **Squared quantities** ($\int B_z^2\,\dd z$, which sets solenoid focusing) are systematically raised, by about 3 % in the example: the square of the noise is always positive and does not cancel.
- **Derivatives** (gradients near the axis, curls from differences) are amplified most: the difference of two neighbouring points divided by a small spacing amplifies the noise. In the example below the amplification is nearly 3 times. Do not "verify" Maxwell's equations from these maps by subtracting neighbouring points; use the summed-up (integral) form instead.
- **The maximum** is not the peak field: the raw maximum of `sol.bsz` is about 20 % above the 9-point smoothed peak.

### Relations the map should satisfy

A field map comes from a numerical solution and should satisfy Maxwell's equations within the accuracy of the computation; this is also a tool for checking maps. The simplest relation is the same as the flux argument in [Electrostatic and magnetostatic field maps](#static-fields-magnetic): near the axis the transverse field is set by how the longitudinal field changes along $z$,

$$
E_r \approx -\frac{r}{2}\frac{\partial E_z}{\partial z},\qquad
B_r \approx -\frac{r}{2}\frac{\partial B_z}{\partial z}\quad(\text{axially symmetric, near the axis})
$$

In the sol viewer, switch to the $x$–$y$ cross-section, turn on the arrows and place the slice near the entrance and the exit: the transverse field points towards the axis everywhere at the entrance, away from it everywhere at the exit, and vanishes in the central cross-section (item 4 of the [checklist](#field-maps-checks)). In hwr010, check the first relation: the map's own $E_x(x{=}1\,\mathrm{cm})$ has a maximum along $z$ of 1.89 MV/m (raw map value), while the smoothed on-axis $E_z$ gives 2.04 MV/m, about 8 % apart: the transverse and longitudinal components are consistent. Subtracting neighbouring points of the raw data directly gives 5.4 MV/m: that is the amplification of noise by derivatives mentioned above.

In an RF cavity the electric and magnetic fields are also tied together by the law of induction. Checked in summed-up form ([Going deeper: testing a map with Maxwell's equations](#field-maps-deep-maxwell)), the circulation of hwr010's magnetic field around the axis and the flux of its electric field agree within 2 %, and the sign shows that $K_b/K_e = -1$: the example's choice $K_b = -K_e$ is exactly what makes hwr010's RF electric and magnetic fields one consistent solution under AVAS's cos / sin convention. The same data also show that the HWR field is not perfectly axially symmetric and has a small quadrupole component.

When transverse component files are missing, the linear preview uses these relations to derive transverse gradients from the on-axis component; the radial part of a magnetostatic field (the solenoid fringe) is always derived from the on-axis $B_z$, and the map's $B_x$, $B_y$ provide only the divergence-free part (quadrupole and skew-quadrupole components).

## How to tell whether a map can be trusted {#field-maps-checks}

Before using a new map for calculations, check it item by item. Behind each item is a piece of physics from the sections above:

1. **Header and number of values**: the number of values equals $(N_z+1)(N_x+1)(N_y+1)$; the header length equals `L` in the lattice; all components of one map share the same grid.
2. **Complete components**: an RF cavity needs at least one complete set of three electric or magnetic components; static fields need all three. The Lattice page warns "field map not found … (needs …)".
3. **Longitudinal position** (tests the storage order): the on-axis $B_z$ peak of a solenoid should be at the element centre (example `sol`: peak at $z \approx 0.177\ \mathrm{m}$, centre 0.175 m); the $E_z$ of a two-gap cavity should be antisymmetric about the centre (example `hwr010`: zero crossing at $z \approx 0.104\ \mathrm{m}$, centre 0.105 m). If the peak is pushed to one end or looks like a periodic sawtooth, suspect the storage order first.
4. **Transverse direction** (tests closed field lines and the sign): in a cross-section at a solenoid's entrance $(B_x, B_y)$ should point towards the axis, at the exit away from it, and the transverse field should be close to zero in the central cross-section (verified for the example, deviations at the noise level); the magnitude should agree with $\tfrac{r}{2}\lvert B_z'\rvert$. With a noisy map only rough agreement can be expected, because $B_z'$ itself is inaccurate: for `sol` at $x = 1\ \mathrm{cm}$ and $z = 0.121$ and $0.1435\ \mathrm{m}$ the map gives $B_x = -0.55$ and $-0.99$, while $-\tfrac{r}{2}B_z'$ from the 9-point smoothed on-axis field gives $-0.59$ and $-1.44$ (map units, Kb = 1).
5. **Symmetry**: an HWR is symmetric top-to-bottom about the beam axis; for a quadrupole $B_y(x)$ and $B_x(y)$ have the same sign and slope; the viewing direction is "from downstream towards upstream".
6. **Decay at the ends**: at both ends the field should have dropped below one per cent of the peak.
7. **Physical quantities**: for an RF cavity $\int\lvert E_z\rvert\,\dd z$ and the position of the maximum of the transit-time factor $T(\beta)$ should match the design (example: $T$ is largest at $\beta \approx 0.10$); for a solenoid $\int B_z\,\dd z$ should match the magnet design.
8. **Trial run**: check with the linear preview or a short multi-particle run that the energy gain and focal length are reasonable before production runs.

## Think about it {#field-maps-think}

> **Think about it 1**: what goes wrong if `steppercycle` is changed from 50 to 10?
>
> Answer: each step becomes about 10.5 mm, while the half-width of one gap's field peak in hwr010 is only about 40 mm; one gap is sampled by only about four steps and the integration error grows noticeably. The step is also much larger than the 2 mm grid, so details of the grid are skipped. Conversely, steps much shorter than the grid spacing buy no extra information; they only walk more finely through the interpolated field.

> **Think about it 2**: why does more noise make it harder for the roughness to tell the two storage orders apart?
>
> Answer: roughness measures how large the second differences along $z$ are compared with the first differences. If the data are noisy from point to point, the second differences are large even in the correct reading, and the difference between correct and wrong readings is drowned. The distinction comes from the smoothness of the real field, and noise destroys exactly that.

> **Think about it 3**: with noise this visible, why does AVAS not smooth the maps first?
>
> Answer: smoothing is done on each component separately and destroys the Maxwell relations between components (closed field lines, for instance); it also changes the shape of the fringe field, which is the very reason for using a map instead of a hard-edge model. Map quality should be fixed at the source: export with a fine enough grid and converged accuracy of the computation. Every value in this primer obtained after smoothing (such as the peak field of sol) states how it was smoothed.

> **Think about it 4**: in the animation, halving Ke nearly halves the energy gain. What happens to the focusing if the Kb of a quadrupole map is doubled?
>
> Answer: without saturation the shape stays and the field doubles everywhere; the gradient doubles and the focal length halves (for a thin lens). The map fixes the shape, Kb the amplitude; but if the real yoke saturates at twice the current, this scaling is no longer accurate.

## Common misconceptions {#field-maps-misconceptions}

- **"Stripes in the heat map mean it was read wrongly"**: stripes may come from a wrong storage order or from noise in the data itself (as in the example); items 3 and 4 of the [checklist](#field-maps-checks) tell them apart.
- **"The finer the map, the better"**: a grid much finer than the integration step does not improve accuracy; the accuracy is limited by how well the field program itself converged. Too coarse a grid is the problem.
- **"The maximum in the map is the peak field"**: noise raises the raw maximum (about 20 % for the example sol); a cavity map does not contain the cavity surface either, so the surface peak field $E_{pk}$ cannot be read from it (see [RF cavity](#rf-cavity-in-avas)).
- **"Ke and Kb are just unit conversions"**: they are field amplitudes, the working point for a cavity, the excitation for a magnet; only in the linear (unsaturated) case can one map be scaled to any field level.
- **"A map narrower than the aperture does not matter"**: there are regions inside the aperture without field data, and the manual does not say what the engine uses outside the map.
- **"Truncate maps to avoid overlap"**: if the field at the cut is not small, the cut creates a spurious edge kick; overlaps call for [superposed fields](#superpose).
- **"The animation is the engine's result"**: the primer's animations push the particles with the linear model, while the engine tracks them step by step through the map; their energy gains can differ by a few per cent.
- **Arrow directions**: with different axis scales the arrows in the viewer stretch with the plot; only at equal scale do they show the true direction.

## Going deeper: sampling, interpolation and derivatives {#field-maps-deep-sampling}

The main text says that the grid decides what can be resolved, that derivatives amplify noise and that roughness can tell the storage order. This section turns those three statements into numbers, using only linear functions, sums of squares and the addition of variances; the last paragraph is for readers who know numerical analysis.

### Step 1: linear interpolation

In one dimension, grid points $z_i$ and $z_{i+1}$ are $h$ apart, with field values $f_i$ and $f_{i+1}$. The estimate at a point $z$ in between is the straight line through the two points:

$$
f(z) \approx f_i + \bigl(f_{i+1} - f_i\bigr)\,\frac{z - z_i}{h}.
$$

In three dimensions one interpolates along $x$, then $y$, then $z$, using the 8 grid points around the particle (trilinear interpolation). Replacing the curve with a straight line makes an error of at most about $h^2\lvert f''\rvert/8$, where $f''$ measures how strongly the curve bends. Estimate it for hwr010: one gap's field peak has a half-width of about 40 mm, like a bell curve of width parameter $\sigma \approx 17\ \mathrm{mm}$, with $\lvert f''\rvert \approx f/\sigma^2$ at the top; for $h = 2\ \mathrm{mm}$ the error is about $4/(8\times 17^2) \approx 0.2\,\%$. For a smooth field a 2 mm grid is enough.

### Step 2: the shortest structure a grid can resolve

To draw a wiggle you need at least one point on an "up" and one on a "down", so the shortest period a grid can resolve is about $2h$. Shorter structures are not just lost; they are mistaken for longer wiggles (like a wheel turning backwards in a video). Field-map noise is exactly this kind of point-to-point wiggle: it lives on the $2h$ scale, and the grid cannot separate it from the real field.

### Step 3: why derivatives amplify noise

Estimate the slope by a central difference: $f'(z_i) \approx (f_{i+1} - f_{i-1})/(2h)$. Suppose each point carries an independent random error of size about $\varepsilon$. Subtracting two independent errors adds their variances, so the difference is about $\sqrt2\,\varepsilon$, and the slope error is about

$$
\delta f' \approx \frac{\sqrt2\,\varepsilon}{2h} = \frac{\varepsilon}{\sqrt2\,h}.
$$

The smaller $h$, the larger the error: the finer the grid, the more the noise is amplified. The real slope is about $f/\sigma$, so the relative error is about $(\varepsilon/f)(\sigma/h)$, and $\sigma/h$ is about 10 in the example. Averaging over $n$ points first reduces the noise to about $\varepsilon/\sqrt n$; that is why the main text averages over 5 points before differentiating: $\tfrac r2\lvert\partial E_z/\partial z\rvert$ for hwr010 is 5.4 MV/m from the raw data and 2.04 MV/m after averaging, in agreement with the map's own $E_x$ (1.89 MV/m).

Summing (integrating) works the other way round: the sum of $n$ independent errors grows only to $\sqrt n\,\varepsilon$, while the sum of the real signal grows $n$-fold, so the relative error gets smaller and smaller. That is why maps are tested in integral form.

### Step 4: the definition of roughness

For a column of data along $z$, $f_1, f_2, \ldots$, AVAS's roughness is

$$
\mathcal{R} = \frac{\sum_i \lvert f_{i+1} - 2f_i + f_{i-1}\rvert}{\sum_i \lvert f_{i+1} - f_i\rvert},
$$

summed together over the columns of at most about 16×16 points of the cross-section. The numerator is "change of the slope", the denominator "change" itself; the ratio does not depend on the size of the field. Two extremes:

- **A smooth field**: neighbouring first differences are nearly equal, the second differences are about $h/\sigma$ times the first, and $\mathcal{R}$ is far below 1.
- **Pure random noise**: a second difference combines three independent errors with variance $1 + 4 + 1 = 6$ times; a first difference has variance 2 times; the ratio is about $\sqrt{6/2} = \sqrt3 \approx 1.73$.

For the examples: the z-outermost roughness sum of the three sol components is 5.01, an average of 1.67 each; the six hwr010 components sum to 9.10, an average of 1.52 each. Both are close to the 1.73 of pure noise: the wiggles between neighbouring points along $z$ are mostly noise. That is why the two readings differ by only 1–6 %, and why roughness has to be followed by physical criteria.

### For readers who know numerical analysis

Linear interpolation has a truncation error $O(h^2)$; a central difference has truncation error $O(h^2)$ and rounding (noise) error $O(\varepsilon/h)$, whose balance gives the optimal step $h \sim (\varepsilon/\lvert f'''\rvert)^{1/3}$. Point-to-point noise has its power near the Nyquist frequency $1/(2h)$; smoothing is low-pass filtering, and filters of the Savitzky–Golay kind suppress noise while keeping low-order moments, but cannot guarantee that the filtered components still satisfy $\nabla\cdot\boldsymbol{B} = 0$.

## Going deeper: testing a map with Maxwell's equations {#field-maps-deep-maxwell}

The main text gave the conclusions of two tests: the transverse and longitudinal fields are consistent, and the ratio of electric to magnetic field is $K_b/K_e = -1$. This section derives them step by step, using magnetic flux, the law of induction and the derivatives of trigonometric functions from school mathematics; the last paragraph is for readers who know vector analysis.

### Step 1: the transverse field comes from the change of the longitudinal field

As in [Electrostatic and magnetostatic field maps](#static-fields-magnetic), take a small cylindrical box of radius $r$ and length $\Delta z$ near the axis. Magnetic field lines are closed, so what goes in comes out; with no charge in the vacuum the same holds for electric field lines. The extra flux of the longitudinal field through the end faces is made up by the radial field on the side:

$$
B_r = -\frac{r}{2}\,\frac{\Delta B_z}{\Delta z},\qquad E_r = -\frac{r}{2}\,\frac{\Delta E_z}{\Delta z}.
$$

In an RF cavity both fields oscillate in time, but the relation holds at every moment, so it holds for the shape stored in the map as well.

### Step 2: checking it in hwr010

Take $r = 1\ \mathrm{cm}$. The map's own $E_x$ has a maximum along $z$ of 1.89 MV/m; averaging the on-axis $E_z$ over 5 points first and then applying step 1, the maximum of $\tfrac r2\lvert\Delta E_z/\Delta z\rvert$ is 2.04 MV/m, about 8 % apart (raw map values, Ke = 1). The raw data directly give 5.4 MV/m: the noise is amplified nearly 3 times (the reason is in step 3 of the previous section).

### Step 3: checking it in sol

For `sol` at $x = 1\ \mathrm{cm}$ and $z = 0.121$ and $0.1435\ \mathrm{m}$, the map gives $B_x = -0.55$ and $-0.99$; step 1 with the 9-point smoothed on-axis field gives $-0.59$ and $-1.44$ (map units, Kb = 1). Sign and order of magnitude agree; the point-by-point differences come from the noise amplified by the derivative. Sum up instead: the paraxial gradient integrated from the entrance to the peak is $-3.98\ \mathrm{T}$, and step 1 requires it to equal $-\tfrac12B_{z,\text{peak}} = -4.14\ \mathrm{T}$, 4 % apart (see [Electrostatic and magnetostatic field maps](#static-fields-deep-magnetic)).

### Step 4: induction ties E and B together

School physics teaches Faraday's law of induction: when the magnetic flux through a loop changes, an EMF appears around the loop, $\oint\boldsymbol{E}\cdot\dd\boldsymbol{l} = -\dd\Phi_B/\dd t$. Maxwell added its "other half": when the electric flux through a loop changes, a circulation of the magnetic field appears around it, in vacuum $\oint\boldsymbol{B}\cdot\dd\boldsymbol{l} = \tfrac{1}{c^2}\,\dd\Phi_E/\dd t$.

Insert AVAS's convention into the second law. The flux of the electric field $K_e\boldsymbol{E}_{map}\cos(\omega t+\varphi)$ through the loop is $K_e\cos(\omega t+\varphi)\int E_{z,map}\,\dd A$, and its rate of change needs the derivative of $\cos$, which is $-\omega\sin$; the circulation of the magnetic field $K_b\boldsymbol{B}_{map}\sin(\omega t+\varphi)$ is $K_b\sin(\omega t+\varphi)\oint\boldsymbol{B}_{map}\cdot\dd\boldsymbol{l}$. The $\sin(\omega t+\varphi)$ on both sides cancels:

$$
K_b\oint\boldsymbol{B}_{map}\cdot\dd\boldsymbol{l} = -\frac{\omega}{c^2}\,K_e\int E_{z,map}\,\dd A.
$$

In the same way the first law gives $K_e\oint\boldsymbol{E}_{map}\cdot\dd\boldsymbol{l} = -\omega K_b\int B_{z,map}\,\dd A$. So an RF field map fixes not only the field shape but also the ratio $K_b/K_e$ that makes E and B one and the same solution.

### Step 5: reading Kb/Ke from the map

Use as loop a square centred on the axis with sides of 1.2–2 cm, cross-section by cross-section along $z$. In the map, $\oint\boldsymbol{B}_{map}\cdot\dd\boldsymbol{l}$ and $+(\omega/c^2)\int E_{z,map}\,\dd A$ have a correlation coefficient of 0.998 along $z$, and integrated over each gap they agree within 2 %. Compared with step 4 the sign is positive, which means $K_b/K_e = -1$: the example's choice $K_b = -K_e$ is exactly what makes hwr010's RF electric and magnetic fields one consistent solution under AVAS's cos / sin convention (the map evidently stores $\boldsymbol{B}$ as the amplitude of a field varying as $-\sin$, like $B_\theta$ in the [pillbox mode](#rf-cavity-modes)). Circulation and flux are summed quantities, so they are immune to the noise; testing the same relation by differencing neighbouring points of the map is completely drowned in noise.

### Step 6: an HWR is not perfectly axially symmetric

The same data show how far the HWR is from axial symmetry. In the first gap, near the axis, $\partial B_x/\partial y \approx -0.042\ \mathrm{T/m}$ while $\partial B_y/\partial x \approx -0.002\ \mathrm{T/m}$; an axially symmetric gap would share the same circulation equally, $\partial B_x/\partial y = -0.020$ and $\partial B_y/\partial x = +0.020\ \mathrm{T/m}$ (raw map values, Ke = 1). The difference is a part that produces no circulation and looks like a quadrupole magnet: the HWR quadrupole component mentioned in [RF cavity](#rf-cavity-modes). For protons at $\beta = 0.057$ the magnetic force $qvB$ is only about 1 % of the electric force, so its effect is small here.

### For readers who know vector analysis

Step 1 is the first-order paraxial expansion of $\nabla\cdot\boldsymbol{B} = 0$ and $\nabla\cdot\boldsymbol{E} = 0$. Inserting $\boldsymbol{E} = K_e\boldsymbol{E}_{map}(\boldsymbol{r})\cos(\omega t+\varphi)$ and $\boldsymbol{B} = K_b\boldsymbol{B}_{map}(\boldsymbol{r})\sin(\omega t+\varphi)$, Faraday's law and the Ampère–Maxwell law give

$$
K_e\,\nabla\times\boldsymbol{E}_{map} = -\omega K_b\,\boldsymbol{B}_{map},\qquad
K_b\,\nabla\times\boldsymbol{B}_{map} = -\frac{\omega}{c^2}\,K_e\,\boldsymbol{E}_{map}.
$$

Step 5 is the Stokes integral form of the second equation. $(\nabla\times\boldsymbol{B})_z = \partial B_y/\partial x - \partial B_x/\partial y$ constrains only the difference of the two partial derivatives; their sum $\partial B_y/\partial x + \partial B_x/\partial y$ is the curl-free normal quadrupole component, which is what step 6 measures.

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

The first four lines are the header: $N_z$, $N_x$, $N_y$ are the **numbers of grid intervals** in the three directions (the numbers of grid points are $N+1$), $L$ is the map length (m), $x_{min}$ … $y_{max}$ the transverse range (m), and `norm` the normalization factor. Then follow $(N_z+1)(N_x+1)(N_y+1)$ values, one per line or several separated by whitespace.

The header of the example `hwr010.edz` is

```text
105 2.100000000000000E-001
20 -2.000000000000000E-002 2.000000000000000E-002
20 -2.000000000000000E-002 2.000000000000000E-002
1
```

i.e. $106\times 21\times 21 = 46\,746$ points with 2 mm spacing in all three directions; the header of `sol` is `200 0.35`, `24 ±0.02`, `24 ±0.02`, `1`, for $201\times 25\times 25 = 125\,625$ points with 1.75 mm longitudinal and 1.67 mm transverse spacing.

### Binary format

A file that does not start with plain numeric text is read as little-endian binary: `int32 Nz`, `float64 L`, `int32 Nx`, `float64 xmin`, `float64 xmax`, `int32 Ny`, `float64 ymin`, `float64 ymax`, `float64 norm` (60 bytes, no padding), followed by $(N_z+1)(N_x+1)(N_y+1)$ `float32` values. A binary file takes only 4 bytes per value, much less than text (about 11 bytes per number in the example), and is faster to read.

### Storage order

The order of the values is recognised as either "z outermost" or "z innermost", decided for the whole set of component files with the same name from the sum of their roughness (z innermost only if its sum is less than 0.9 times that of z outermost), see [In which order the numbers are stored](#field-maps-order). Export maps z outermost.

### Normalization factor

The AVAS interface and the linear preview multiply the values read by `norm`. All example maps have `norm` 1; how the engine uses this line is not described in the manual. **Write `norm` as 1 when exporting** and set field levels with Ke and Kb.

### Extensions

| Extension | Component | Field type | Unit |
|---|---|---|---|
| `.edx` `.edy` `.edz` | RF electric field $E_x$, $E_y$, $E_z$ | 1 | MV/m, times Ke, varies as cos |
| `.bdx` `.bdy` `.bdz` | RF magnetic field $B_x$, $B_y$, $B_z$ | 1 | T, times Kb, varies as sin |
| `.esx` `.esy` `.esz` | Electrostatic field $E_x$, $E_y$, $E_z$ | 2 | MV/m, times Ke |
| `.bsx` `.bsy` `.bsz` | Magnetostatic field $B_x$, $B_y$, $B_z$ | 3 | T, times Kb |

Mnemonic: `e`/`b` for electric/magnetic, `d`/`s` for dynamic (RF)/static, the last letter the component. When the interface checks whether a map is complete, an RF cavity needs at least one complete set of three electric or magnetic components, and electrostatic and magnetostatic maps need all three of their components.

> **AVAS convention**: the E and B of an RF field come from the same eigenmode, and the phase relation between them on export (and the sign it implies) is up to the field program; AVAS always uses cos for E and sin for B and compensates sign differences with the sign of Kb. The example uses $K_b = -K_e$; the manual does not explain the minus sign, but the map itself confirms it (see the circulation test in [Going deeper: testing a map with Maxwell's equations](#field-maps-deep-maxwell)). When switching to a new map, determine the sign of $K_b/K_e$ with the same test, see [RF cavity](#rf-cavity-in-avas).

### Field map directory

The engine looks for field maps by file name in the **field map directory**:

- by default the project's `InputFile/`;
- it can be set on the Simulation settings page under "Field maps → Directory", stored in ini.ini as `[project] fieldSource` (empty means InputFile);
- on the command line `avas run --input DIR --output DIR --field DIR` sets it with `--field`; without it, the ini.ini setting is used, and failing that the input directory.

During a run AVAS copies the text inputs to `<output>/inputs/` and the engine uses the copy, but field maps are still read from the original field map directory, so the large files are not copied. If a map is missing, the Lattice page warns, and the linear preview treats the element as a drift and says so.

> **Note**: field-map files are large (about 0.5–1.4 MB of text per component in the example). When several projects share maps, put them in a common directory and point fieldSource at it rather than copying them into every project.

### Viewing maps in the interface

**Files page**: selecting any component file shows its information (meaning of the component, text/binary, length, grid, transverse range, normalization factor, other components with the same name, which lattice elements use it) and two views:

- **Along z**: the on-axis value ($x = y = 0$) and the maximum of $\lvert F\rvert$ over each cross-section as functions of $z$.
- **Slice**: a heat map of the longitudinal cuts $z$–$x$ (fixed $y$), $z$–$y$ (fixed $x$) or the cross-section $x$–$y$ (fixed $z$), with a movable cut position. Components of the same map on the same grid are cut together, so the magnitude and in-plane arrows can be shown.

**Field window of an element**: select an element on the Lattice page or in the visual editor; the element frame has a "View field map / View field distribution" button in its top right corner. For a field-map element it opens **data** (map slices), for a matrix-model element a **schematic** computed from the parameters (hard edge, zero outside the element, labelled as schematic). Both use the same views; while the window is open, selecting another element updates it, and dragging a parameter redraws it live. The "Field map files" table at the bottom of the window lists, for each component file, the actual path, grid, range, normalization and storage format.

**Reading conventions**:

- **Colours**: signed components use a colour scale centred on 0, symmetric for positive and negative; magnitudes use a one-sided scale.
- **Viewing direction**: all cross-sections are **viewed from downstream towards upstream, with the beam coming out of the screen**, marked by the ⊙ at the centre. Field arrows and force arrows in the same picture can then be checked against each other with $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$: $\boldsymbol{v}$ points at the reader.
- **Arrows**: arrow length is proportional to the field components in data coordinates, so with different axis scales the arrows stretch with the plot; only with "Equal scales" ticked do they show the true direction. Untick and tick "Equal scales" in the viewers above to see the arrow directions change. In longitudinal cuts the element is long and becomes a thin strip at equal scale.
- **Stripes**: stripes along $z$ may come from a wrong storage order or from noise in the data itself (as in the example); the [checklist](#field-maps-checks) tells them apart.

## Further reading {#field-maps-reading}

- The TraceWin user manual on the 3D field map (FIELD_MAP) format: AVAS's text format is similar, but units and parameter meanings follow this chapter and [RF cavity](#rf-cavity).
- T. P. Wangler, *RF Linear Accelerators*, chapter 2: transit-time factor and paraxial fields from the on-axis field.
- M. Reiser, *Theory and Design of Charged Particle Beams*, chapter 3: paraxial expansion of axially symmetric fields.
- AVAS manual `docs/使用说明20260427.docx`: the `field` element and the field-map file format.
- In this primer: [Electrostatic and magnetostatic field maps](#static-fields), [Superposed fields](#superpose), [Simulation settings](#simulation), [References](#references).
