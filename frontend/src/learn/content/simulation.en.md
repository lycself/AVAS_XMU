# Multi-particle simulation in practice

The previous chapters gave the physics of the elements and the linear theory; actual design decisions rest on multi-particle simulations, which track many macro-particles through the real field maps of a beamline, including space charge and non-linearities. This chapter covers what a **trustworthy** multi-particle simulation in AVAS requires: the number of macro-particles and statistical errors, initial distributions, time steps, the space-charge mesh, run settings, output planes and output files, how to judge convergence, the difference between the linear preview and the kernel, single-particle runs, and the most common reasons for failure. For the physics background see [Space charge](#space-charge) and [Longitudinal dynamics](#longitudinal); the overall structure of the input files is in [Accelerators and AVAS](#intro).

## Macro-particles and statistics {#simulation-macroparticles}

### What a macro-particle stands for

A bunch contains $Q/q$ real particles ($Q = I/f$ the bunch charge), typically $10^6$–$10^{10}$. The kernel represents them by `particlenumber` **macro-particles**: each carries the charge $Q/N$ with the charge-to-mass ratio of the real particle, so the motion in external fields is unchanged and only space charge is computed from the total charge. The example project has $Q = 1.133\ \mathrm{pC}$ ($7.07\times 10^6$ protons) and `particlenumber 5260`, so each macro-particle stands for about 1340 protons.

```avas-params
beam:particlenumber beam:randomseed beam:current beam:frequency
```

### Statistical errors

With a finite number of macro-particles every statistical quantity has a random error. For a Gaussian distribution the relative standard error of an rms size estimated from $N$ samples is about $1/\sqrt{2N}$:

| $N$ | Relative statistical error of the rms size |
|---|---|
| 5260 (example) | 0.97 % |
| $10^4$ | 0.71 % |
| $10^5$ | 0.22 % |
| $10^6$ | 0.07 % |

The statistical error of the rms emittance is of the same order. The resolution of the loss fraction is $1/N$: in the example, one lost macro-particle is 0.019 %; studying losses below $10^{-4}$ or halo needs far more macro-particles, or a dedicated study of the distribution tails.

Another error source is **numerical noise**: with far fewer macro-particles than real particles, the space-charge field fluctuates more than in the real beam and causes spurious emittance growth (numerical heating). It decreases as the number of macro-particles per mesh cell increases; see [Space-charge mesh](#simulation-sc-mesh).

> Note: the kernel computes with several threads, and identical inputs give slightly different results from run to run. When comparing two runs (for example to judge the effect of a parameter) use a tolerance; conclusions based on differences smaller than the statistical error are not reliable. `randomseed` in `beam.txt` is the seed used to generate the initial beam.

## Initial distributions {#simulation-distributions}

### Generated from Twiss parameters

`twissx`, `twissy` and `twissz` give $\alpha$, $\beta$ and the **normalised rms** emittance of each plane (β in mm/mrad, emittance in π·mm·mrad; the angle of the $z$ plane is $\delta = \Delta p/p$), see [Phase space and emittance](#phase-space). `distribution V1 V2` selects the transverse and longitudinal distribution type: KV, GS (Gaussian), PB (parabolic), WB (waterbag).

```avas-params
beam:distribution beam:twissx beam:twissz beam:readparticledistribution beam:initpos beam:initmom
```

For the same rms parameters the edges and tails of these distributions differ a lot. With the usual definitions (distributions in four-dimensional phase space), the ratio of the projected edge to the rms size is:

| Distribution | Definition in 4D phase space | Edge / rms |
|---|---|---|
| KV | uniform on the hyper-ellipsoid surface | 2 |
| WB (waterbag) | uniform inside the hyper-ellipsoid | $\sqrt6 \approx 2.45$ |
| PB (parabolic) | density $\propto 1 - r^2/r_{\max}^2$ | $2\sqrt2 \approx 2.83$ |
| GS (Gaussian) | Gaussian | unbounded (truncated in practice) |

> Note: the manual does not state the dimensionality AVAS uses when generating these distributions, nor where the Gaussian is truncated. When you need to know exactly, analyse `inData.dst` in the output directory (the initial distribution actually used). With strong space charge the distribution shape determines the initial emittance growth (see [Space charge](#space-charge-emittance-growth)); when comparing codes, make sure the distributions agree.

### Reading a particle file

`readparticledistribution` names an existing `.dst` or `.edst` file (for example an RFQ output distribution computed with another code). When a particle file is used, every `beam.txt` keyword except `numofcharge` is ignored; `.edst` can describe mixed beams of several particle species. `initpos` and `initmom` add a position and angle offset to the whole bunch, useful for studying initial off-centring.

## Time steps {#simulation-stepping}

### Steps of the t-code

Field-map elements and drifts are integrated in time with the t-code (see [Accelerators and AVAS](#intro-tracking)). `steppercycle V1` in `input.txt` divides one RF period into $V_1$ steps:

$$
\Delta t = \frac{1}{f\,V_1},\qquad \Delta s \approx \beta c\,\Delta t = \frac{\beta\lambda}{V_1} .
$$

In the example, `steppercycle 50` at 162.5 MHz and $\beta = 0.057$ gives $\Delta t = 0.123\ \mathrm{ns}$ and about 2.1 mm per step, comparable with the 2 mm grid spacing of the `hwr010` map along $z$. The manual writes the formula as "Δt = 1/T × 1/V1"; the second parameter V2 is used only for secondary-particle transport ($\Delta t = V_2/V_b$).

```avas-params
input:steppercycle input:sim_type
```

Rules for choosing the step:

- the distance per step must be much smaller than the scale on which the field varies (field-map grid, gap length, $\beta\lambda$);
- much smaller than the transverse, synchrotron and plasma wavelengths (space charge is computed once per step, and too large a step increases the integration error of the space-charge kick);
- check by doubling the number of steps: when the results no longer change (within the statistical error), the step is small enough.

### z-code elements

`quad`, `solenoid`, `bend`, `edge` and `steerer` are matrix elements that transform the particle coordinates in longitudinal position with the z-code; the t-code records the bunch at the synchronous particle's time and the z-code at a position (see the manual's description of `Beamset.plt`); how the kernel converts between them at the boundary is not documented. The schema notes for matrix elements: **avoid matrix elements as the first or last element of a multi-particle lattice**; put a short drift at each end.

## Space-charge mesh {#simulation-sc-mesh}

With `spacecharge 1` the kernel solves for the self field of the beam on a mesh. `numofgrid Nx Ny Nz` sets the number of mesh points and `meshrms V1 V2 V3` the mesh length $L_x = 2V_1\sigma_x$ (likewise $y$, $z$), so the cell size is

$$
h_x = \frac{2V_1\sigma_x}{N_x} .
$$

```avas-params
input:spacecharge input:scmethod input:numofgrid input:meshrms
```

Three competing requirements:

- **Coverage**: the mesh should contain nearly all particles. Too small a $V_1$ cuts the tails; for Gaussian beams take $V_1$ of at least 3–5. How particles outside the mesh are treated is not documented in the manual.
- **Resolution**: the core needs enough mesh points, $h_x$ clearly smaller than $\sigma_x$; the larger $V_1$, the poorer the resolution for the same $N_x$.
- **Particles per cell**: the average number of macro-particles per cell is about $N/(N_xN_yN_z)$, more in the core. For example, a $32\times 32\times 32$ mesh with 5260 macro-particles holds only 0.16 particles per cell on average, and the field is very noisy; such a mesh needs $10^5$–$10^6$ macro-particles.

`scmethod` can be `FFT`, `PICNIC` or `SPICNIC`; the manual only lists the names. After changing the mesh or the solver, compare results as described in [Judging convergence](#simulation-convergence).

## Run settings {#simulation-settings}

```avas-params
input:multithreading input:dumpperiodicity input:boundary input:longlimits input:pchistogram input:scanphase
```

- **Multithreading**: write `multithreading 1` to use several threads; for a single thread **leave the line out**. The manual says 0 means off, but with `multithreading 0` (or 2, 3, ...) the kernel loses every particle at the start (verified with the example project). The settings page of the GUI already handles this.
- **Loss boundaries**: a particle outside the element aperture `R` counts as lost; with `boundary 1` the boundaries in `boundary.txt` are used instead, the lattice radii no longer apply, and lost particles are written to `CollisionData.txt`. `longlimits 1 phase energy` enables longitudinal loss limits, corresponding to the separatrix of [Longitudinal dynamics](#longitudinal-separatrix).
- **Particle data**: `dumpperiodicity V1` writes all particles to `Beamset.plt` every $V_1$ steps (0 = never); the file can be large. `pchistogram 1 bins` writes `pchistogram.dat`.
- **Phase scan**: `scanphase` controls the phase scan of the RF cavities or reads the phases from `scanData.txt` (see [Longitudinal dynamics](#longitudinal-avas)).
- **Model**: `sim_type mulp` is the multi-particle model, `env` the envelope model.
- **GPU**: the GPU version of AVAS exists only as a pre-compiled Linux library; on Windows, use multithreaded CPU runs.

## Output planes and output files {#simulation-output}

### Output planes

```avas-params
outputplane automaticoutput
```

- `outputplane V1` dumps the beam distribution $V_1$ downstream (positive) or upstream (negative) of the keyword's position, as `outData_<position>.dst`.
- `automaticoutput V1 V2 V3` inserts output planes starting at relative position $V_1$, spaced by $V_2$, over a maximum span $V_3$.
- An output plane **beyond the end of the lattice** is an error; one **within 5 mm of the end** also fails, while 2 cm before the end works. The kernel always writes `outData_<end>.dst` at the end of the lattice, so no output plane is needed there.

### Main output files

| File | Content |
|---|---|
| `DataSet.txt` | bunch statistics, one row per recorded step (table below) |
| `inData.dst` | the initial distribution actually used |
| `outData_<z>.dst` | particle distribution at an output plane (binary) |
| `Phase.txt` | time, position and synchronous-particle energy at entry and exit of every RF cavity |
| `synParticle.txt` | synchronous-particle data along the line |
| `Beamset.plt` | all particle data when `dumpperiodicity` is on |
| `SingleParticle.txt` | trajectory of a single-particle run |

### DataSet.txt

41 columns per row (numbered from 0); the most used columns:

| Column | Content |
|---|---|
| 0 | mean kinetic energy (MeV) |
| 1, 3, 5 | centroid $x$, $y$, $z$ relative to the synchronous particle |
| 7–9, 10–12, 13–15 | $\alpha$, $\beta$, emittance of the three planes |
| 16, 18, 20 | rms $x$, $y$, $z$ (m) |
| 22, 24, 26 | maximum $x$, $y$, $z$ |
| 28 | surviving macro-particles |
| 29, 31, 33 | synchronous particle $x$, $y$, $z$ |
| 35 | flag: 0 straight section, 1 curved section, 2 invalid (skip) |
| 40 | time |

In straight sections the longitudinal position is column 5 plus column 33; with dipoles in the line the arc length of curved sections has to be accumulated (AVAS's readers do this). The kernel recreates the file when a run starts and appends rows one by one; when reading a file that is still being written, drop an incomplete last line.

> AVAS convention: runs **never write InputFile**. A full run copies the text inputs to `<output directory>/inputs/`, and the kernel uses the copies as input with the original directory as field-map directory, so every output directory carries a snapshot of its inputs. A full run overwrites OutputFile each time; results worth keeping must be "kept" in the GUI (copied to the project's `Runs/`).

## Judging convergence {#simulation-convergence}

A multi-particle result is credible only if it is shown to be insensitive to the numerical parameters. A recommended procedure:

1. First get the lattice working with the linear preview and few macro-particles ($10^3$–$10^4$); check that energies, phases and envelopes are roughly right.
2. **Macro-particles**: at the design parameters increase `particlenumber` by a factor of 4 or more (e.g. $10^4 \to 10^5$) and compare exit rms sizes, rms emittance growth and losses. Statistical errors fall as $1/\sqrt N$; if the emittance growth drops markedly as $N$ grows, the original result was polluted by numerical noise.
3. **Step**: double `steppercycle` (e.g. 50 → 100); results should stay the same within the statistical error.
4. **Mesh**: vary `numofgrid` and `meshrms` together while keeping enough macro-particles per cell; if needed, compare different `scmethod` choices.
5. **Repeat runs**: run the same input two or three times to estimate the run-to-run scatter, and use it as the tolerance for "changed or not".

The criterion depends on the question: matching and envelope design care about rms sizes (at the 1 % level); emittance growth about the relative growth; losses and halo need high statistics, the maximum sizes and the tail distribution. When reporting results, state the number of macro-particles, the step and the mesh used.

## Linear preview and kernel {#simulation-preview}

The **linear envelope preview** of the visual editor (`avas.sim.linear_optics`) pushes the second moments of the beam with 6×6 matrices: the reference particle's energy and time are integrated through the field maps, the maps are cut into 4 mm slices for the linear focusing and bunching, and space charge is a linear kick of the rms-equivalent uniform ellipsoid (see [Space charge](#space-charge-avas)). It responds almost instantly while parameters are dragged and suits matching and early design.

| Aspect | Linear preview | Multi-particle kernel |
|---|---|---|
| Energy | error about 0.01 % | reference |
| rms sizes | usually within 1–2 %, up to 13 % with strong multi-particle effects | reference |
| Non-linearity, emittance growth, halo | not included | included |
| Losses | only a warning when 3 × rms exceeds the aperture | particle by particle |
| Dipole / steering content of field maps | deflection not modelled (warning) | included |

> Note: the preview contains neither the longitudinal non-linearity of low-energy cavities, nor chromatic effects, nor `superposeout` offsets, nor `scanphase`. Design conclusions rest on the kernel; a large difference between preview and kernel usually indicates strong non-linear or space-charge effects, which is exactly when multi-particle simulation is needed.

## Single-particle runs {#simulation-single-particle}

With `particlenumber 1` the kernel tracks a single particle and writes `SingleParticle.txt` ($x$, $y$, $z$, $\gamma\beta_{x,y,z}$, kinetic energy, time). The multi-particle keywords `twissx`, `twissy`, `twissz`, `current`, `distribution` and so on are ignored; `displacepos` and `displacedpos` set the offset of this particle from the synchronous particle:

```avas-params
beam:displacepos beam:displacedpos
```

> AVAS convention: the kernel **reads `displacepos dx dy dz` in mm** (the manual says m; verified with the kernel); `displacedpos dpx dpy dpz` is in %. In single-particle runs the rms columns of DataSet.txt are NaN; this is normal, look at the trajectory and the synchronous-particle columns instead.

Single-particle runs are useful for orbit checks: the kick of a steerer, the dispersion of a dipole, the alignment of a field map, or effective transfer-matrix elements from individual offset particles.

## Common problems {#simulation-troubleshooting}

### All particles lost at the start

The kernel reports `End of simulation, all particles lost`. Check in this order (the first item is verified with the kernel, the others are possible causes worth checking):

- `input.txt` contains `multithreading 0` (or 2, 3): remove the line or set it to 1;
- the initial beam is too large for the aperture `R` (3 × rms close to the aperture), or `initpos` offsets it too much;
- `longlimits` is on with too narrow limits;
- the first element is a matrix element: the manual advises against it without saying what happens; put a short drift in front to rule it out.

### Field map not found

The last parameter of `field` is the field map's **base name** (without extension); the extensions follow the field type: RF `.edx/.edy/.edz` and `.bdx/.bdy/.bdz`, static electric `.esx/.esy/.esz`, static magnetic `.bsx/.bsy/.bsz`. The field-map directory is InputFile by default and can be set with `[project] fieldSource` in `ini.ini`. The storage order of a map is decided for the whole group of component files (see [Field maps](#field-maps)). The example maps are noisy themselves; stripes along $z$ in slice pictures are data, not a reading error.

### Output plane errors

The output plane is beyond the end of the lattice or less than 5 mm from it. Move it at least about 2 cm before the end, or use the `outData_<end>.dst` the kernel writes at the end anyway.

### Other

- **Only lines between `start` and `end` are simulated**: only lines after the first `start` and before the first `end` count, and `!` starts a comment. Elements written after `end` are not computed, which typically happens when a lattice was cut short while debugging and never restored.
- **Energy drops to zero or the beam reverses**: a wrong RF phase (opposite sign of the synchronous phase, a misread V3) decelerates the reference particle; the preview stops there with a warning.
- **NaN rms columns**: normal in single-particle runs; in multi-particle runs it means too few surviving particles.
- **Results differ slightly every time**: normal with multithreading; compare within the statistical error.

## Further reading {#simulation-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: chapter 9 (multiparticle simulation and space charge).
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: distribution functions and emittance growth.
- R. W. Hockney, J. W. Eastwood, *Computer Simulation Using Particles*, Adam Hilger, 1988: particle–mesh methods and numerical noise.
- In this primer: [Space charge](#space-charge), [Errors and correction](#errors), the [tutorial](#tutorial); the full description of input and output files is in the GUI's user manual (parameter and file reference).
