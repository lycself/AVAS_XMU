# Multi-particle simulation essentials

One bunch of the example project contains about 7 million protons; AVAS represents them by 5260 macro-particles and advances them in steps of 0.12 ns. Can such a result be trusted? Why does the same input give slightly different results on two runs, and how large a difference is a real one? Starting from these questions, this chapter explains where the errors of a multi-particle simulation come from — statistical errors and numerical noise from the finite number of macro-particles, the time step, the space-charge mesh — how convergence tests tell you whether a result can be trusted, and what the linear preview and the engine can each tell you. It ends with AVAS's run settings, output files, single-particle runs and the most common causes of failure.

The physics background is in [space charge](#space-charge) and [longitudinal dynamics](#longitudinal), initial distributions in [initial distributions](#distributions), the overall structure of the input files in [accelerators and AVAS](#intro).

## Macro-particles and statistics {#simulation-macroparticles}

### What a macro-particle stands for

The real number of particles in a bunch is $Q/q$ ($Q = I/f$ the bunch charge), typically $10^6$–$10^{10}$, far too many to compute all their mutual forces. The engine represents them by `particlenumber` **macro-particles**: each carries the charge $Q/N$ and a mass scaled by the same factor, so its charge-to-mass ratio equals that of a real particle. Motion in external fields depends only on the charge-to-mass ratio, so every macro-particle follows exactly the trajectory of one real particle; only space charge is computed from the total charge. Think of an opinion poll: a few thousand people stand for millions; if the sample is representative the averages are right, but every statistic carries a sampling error.

The example project has $Q = 1.133\ \mathrm{pC}$ ($7.07\times 10^6$ protons) and `particlenumber 5260`, so each macro-particle stands for about 1340 protons.

### How large the statistical error is

Estimating the variance $\sigma^2$ of a Gaussian from $N$ samples, the estimate itself has the variance $2\sigma^4/N$. The rms size is the square root of the variance, which halves the relative error, so

$$
\frac{\delta\sigma}{\sigma} \approx \frac{1}{2}\sqrt{\frac{2}{N}} = \frac{1}{\sqrt{2N}}
$$

It falls as $1/\sqrt N$: to reduce the error tenfold, the number of macro-particles must grow a hundredfold. The factor 2 belongs to the Gaussian, which is what the example uses (`distribution GS GS`). For a general distribution the variance of the variance estimate is $(\mu_4 - \sigma^4)/N$, so $\delta\sigma/\sigma \approx \sqrt{(\kappa - 1)/(4N)}$ with the kurtosis $\kappa = \mu_4/\sigma^4$: distributions with sharp edges are estimated more precisely (a uniform projection, $\kappa = 1.8$, gives $0.45/\sqrt N$ instead of $0.71/\sqrt N$), heavy tails less precisely.

| $N$ | Relative statistical error of the rms size |
|---|---|
| 5260 (example) | 0.97 % |
| $10^4$ | 0.71 % |
| $10^5$ | 0.22 % |
| $10^6$ | 0.07 % |

The statistical error of the rms emittance is of the same order, about $1/\sqrt N$ for a Gaussian (1.4 % for the example). The resolution of the loss rate is $1/N$: in the example, losing one macro-particle is 0.019 %; studying losses below $10^{-4}$ or halo requires far more macro-particles, or a dedicated study of the distribution tails.

> **Think about it**: with 5260 macro-particles the exit rms size changes by 1 % when you change a parameter. Can you conclude that the parameter matters?
>
> Answer: not for certain. If the initial distributions of the two runs are independent samples, each result has a statistical error of about 0.97 %, and their difference has a standard deviation of about $\sqrt2 \times 0.97\ \% = 1.4\ \%$; a 1 % difference lies within it. If both runs start from the same sample (same `randomseed`; check that the two `inData.dst` files agree, since the manual does not say how the seed is used), the sampling error is common to both and largely cancels in the difference — what remains is the run-to-run scatter of the engine itself. Either way, increase the number of macro-particles, or repeat the runs to measure the run-to-run spread, before drawing a conclusion.

### Numerical noise

A second source of error is **numerical noise**: with far fewer macro-particles than real particles, each carrying a much larger charge, the space-charge field fluctuates more than in the real beam; particles receive small random kicks, which causes spurious emittance growth (numerical heating). It decreases as the number of macro-particles per mesh cell increases, see [space-charge mesh](#simulation-sc-mesh).

> **Note**: the engine computes with several threads, and the same input gives slightly different results on every run. When comparing two runs (to judge the effect of a parameter, say), allow a tolerance; conclusions drawn from differences smaller than the statistical error are unreliable.

## Initial distributions {#simulation-distributions}

The engine can generate four distributions from Twiss parameters with `twissx`, `twissy`, `twissz` and `distribution` — KV, GS (Gaussian), PB (parabolic), WB (waterbag) — or read an existing particle file (for example an RFQ exit distribution) with `readparticledistribution`. For the same rms parameters the edges and tails of these distributions differ greatly, and with strong space charge they decide the emittance growth of the first stretch; make sure the distributions agree when comparing codes. How these distributions are defined, how they differ and how to check which distribution a simulation actually used is explained in the [initial distributions](#distributions) chapter; the keywords are listed in [In AVAS](#simulation-in-avas).

## Time step {#simulation-stepping}

### Why the step must be small

The engine cannot solve the equations of motion continuously; it advances step by step: in each step it computes the force from the present fields and moves the particles a little. It is like tracing a curve with straight segments: the shorter the segments, the more accurate the tracing; the more the field changes within one step, the larger the error.

Field-map elements and drifts integrate the equations of motion in time with the t-code (see [accelerators and AVAS](#intro-tracking)). `steppercycle V1` in `input.txt` divides one RF period into $V_1$ steps:

$$
\Delta t = \frac{1}{f\,V_1},\qquad \Delta s \approx \beta c\,\Delta t = \frac{\beta\lambda}{V_1} .
$$

For the example, `steppercycle 50`, 162.5 MHz, $\beta = 0.057$: $\Delta t = 0.123\ \mathrm{ns}$ and each step advances about 2.1 mm, while the `hwr010` map has a grid spacing of 2 mm along $z$, about the same. Crossing the 0.21 m HWR takes about 100 steps, one 0.76 m period of the [hands-on tutorial](#tutorial) about 360. Note that $\Delta s$ is proportional to β: with the same `steppercycle`, faster particles travel farther per step.

Rules for choosing the step:

- the distance per step must be much smaller than the scale on which the fields change (map grid, gap length, $\beta\lambda$);
- much smaller than the transverse, synchrotron and plasma oscillation wavelengths (the space-charge kick is integrated with the same step; how often the engine recomputes the space-charge field is not described in the manual);
- check by doubling the number of steps: the step is small enough when the results no longer change (within the statistical error).

### z-code elements

`quad`, `solenoid`, `bend`, `edge` and `steerer` are matrix elements: the z-code transforms the particle coordinates by longitudinal position instead of advancing in time steps. The t-code records the bunch at the time of the synchronous particle, the z-code at a position (see the manual's description of `Beamset.plt`); how the engine converts at the boundary between the two kinds of elements is not described in the manual — the engine's behaviour is the reference. The schema's advice for matrix elements: **avoid matrix elements as the first and last element** of a multi-particle lattice; put a short drift at each end.

## Space-charge mesh {#simulation-sc-mesh}

### What the mesh does

Computing the Coulomb force between every pair of $N$ macro-particles takes $N^2$ operations. The particle–mesh method does it differently: first distribute the charge of all macro-particles onto a 3D mesh, giving the charge density on the mesh; solve Poisson's equation on the mesh for the field; then interpolate the field back to every particle. It is a bit like estimating the density of a crowd: divide the square into cells and count the people in each, instead of measuring the distance between every two people. Cells too large, and you cannot see the shape of the crowd; cells too small, and each holds only a stray person or two, so the counted density is all fluctuation.

With `spacecharge 1` the engine solves the beam's self-field on a mesh. `numofgrid Nx Ny Nz` is the number of mesh cells, `meshrms V1 V2 V3` sets the mesh size $L_x = 2V_1\sigma_x$ (likewise for $y$, $z$), so the cell size is

$$
h_x = \frac{2V_1\sigma_x}{N_x} .
$$

For example, $V_1 = 4$ and $N_x = 32$ cover $\pm 4\sigma_x$ with cells of $h_x = 0.25\,\sigma_x$.

### Three requirements that pull against each other

- **Coverage**: the mesh should contain almost all particles. Too small a $V_1$ cuts off the tails; for a Gaussian take $V_1$ at least 3–5. What happens to particles outside the mesh is not described in the manual.
- **Resolution**: the core needs enough mesh points, $h_x$ clearly smaller than $\sigma_x$; the larger $V_1$, the poorer the resolution at the same $N_x$.
- **Particles per cell**: the mean number of macro-particles per cell is about $N/(N_xN_yN_z)$, more in the core. A $32\times 32\times 32$ mesh with 5260 macro-particles has only 0.16 per cell on average, and the field is very noisy; such a mesh needs $10^5$–$10^6$ macro-particles.

The conflict: better resolution needs more cells, and more cells need more macro-particles. The number of macro-particles and the mesh must therefore be chosen together. After changing the mesh or the algorithm, compare results as described under [convergence](#simulation-convergence).

## Judging convergence {#simulation-convergence}

A multi-particle result is credible only if it is shown to be insensitive to the numerical parameters: making them finer no longer changes the result beyond the statistical error. A suggested procedure:

1. First get the lattice right with the linear preview and few macro-particles ($10^3$–$10^4$): energy, phases and envelope should be roughly reasonable.
2. **Number of macro-particles**: at the design parameters raise `particlenumber` by a factor of 4 or more (e.g. $10^4 \to 10^5$) and compare the exit rms sizes, the rms emittance growth and the loss rate. The statistical error falls as $1/\sqrt N$; if the emittance growth drops clearly as $N$ grows, the original result was contaminated by numerical noise.
3. **Step**: double `steppercycle` (e.g. 50 → 100); the results should not change beyond the statistical error.
4. **Mesh**: change `numofgrid` and `meshrms` together while keeping enough macro-particles per cell; if needed, compare different `scmethod`s.
5. **Repeat runs**: repeat the same input two or three times to estimate the random run-to-run difference, the tolerance for deciding whether something changed.

The criterion depends on the question: matching and envelope design care about rms sizes (at the 1 % level); emittance growth about the relative growth; losses and halo need high statistics, maximum sizes and the tails. When reporting results, state the number of macro-particles, the step and the mesh used.

## Linear preview and engine {#simulation-preview}

The visual editor's **linear envelope preview** (`avas.sim.linear_optics`) does not track particles; it advances the beam's second moments (the sigma matrix) with 6×6 matrices: the reference particle integrates energy and time along the field maps, field maps are sliced into 4 mm pieces for linear focusing and bunching, and space charge is a linear kick of the rms-equivalent uniform ellipsoid (see [space charge](#space-charge)). One computation advances a single 6×6 matrix, so results appear almost instantly while you drag parameters, which suits matching and preliminary design. The price is that it knows only second moments and cannot see changes in the shape of the distribution.

| Item | Linear preview | Multi-particle engine |
|---|---|---|
| Energy | reference particle within about 0.01 % (against the engine's synchronous particle); the engine's DataSet gives the macro-particle mean, which feels the off-axis field: about 5 % of the gain in the example cavity | reference |
| rms sizes | usually within 1–2 %, up to 13 % where multi-particle effects are strong | reference |
| Non-linearities, emittance growth, halo | not included | included |
| Losses | only a warning when 3 × rms exceeds the aperture | decided particle by particle |
| Dipole / steering components of field maps | no deflection (a warning is given) | included |

> **Note**: the preview does not include the longitudinal non-linearity of low-energy cavities, chromatic effects, the offsets of `superposeout` or `scanphase`. Design conclusions rest on the engine; a large difference between preview and engine usually means strong non-linear or space-charge effects — exactly when a multi-particle simulation is needed.

## Common misconceptions {#simulation-misconceptions}

- **More macro-particles are better, but only together with the mesh**: adding macro-particles without changing the mesh does not improve the resolution; adding cells without macro-particles makes the noise worse.
- **Differences below the statistical error mean nothing**: with 5260 macro-particles the rms size is uncertain by about 1 %; smaller changes are no basis for a conclusion.
- **No loss is not zero loss**: none of 5260 macro-particles lost only says that the loss rate is below the order of $1/5260$.
- **`steppercycle` is steps per RF period**, not a fixed spatial step: after acceleration each step covers more distance.
- **`multithreading 0` does not switch multithreading off**: it makes the engine lose all particles at the start; to run single-threaded, delete the line (see below).
- **NaN in single-particle runs**: with `particlenumber 1` the rms columns of DataSet are NaN, which is normal.

## In AVAS {#simulation-in-avas}

### Beam and macro-particles

```avas-params
beam:particlenumber beam:randomseed beam:current beam:frequency
```

`randomseed` seeds the random numbers that generate the initial beam. The manual lists it among the `beam.txt` keywords, while the example project writes `randomseed 0` in `input.txt` (the schema accepts it in both files); which one the engine uses when both are present, and whether 0 has a special meaning, is not documented — the engine is the reference.

### Keywords of the initial distribution

```avas-params
beam:distribution beam:twissx beam:twissz beam:readparticledistribution beam:initpos beam:initmom
```

- `twissx`, `twissy`, `twissz` give $\alpha$, $\beta$ and the **normalized rms** emittance of each plane (β in mm/mrad, emittance in π·mm·mrad; the angle of the $z$ plane is $\delta = \Delta p/p$), see [phase space and emittance](#phase-space). `distribution V1 V2` chooses the transverse and longitudinal distribution types: KV, GS, PB, WB.
- `readparticledistribution` names an existing `.dst` or `.edst` file. With a particle file, all `beam.txt` keywords except `numofcharge` are ignored; an `.edst` file can describe a mixed beam of several particle species.
- `initpos` and `initmom` add a position and angle offset to the whole bunch, for studying initial off-centring.

> **Note**: the manual does not state the dimensionality AVAS uses to generate these distributions or where the Gaussian is truncated. When you need to know exactly, take statistics of `inData.dst` in the output folder (the initial distribution the simulation actually used).

### Step and model

```avas-params
input:steppercycle input:sim_type
```

The manual writes the step formula as "Δt = 1/T × 1/V1"; the second parameter V2 is used only for secondary-particle transport ($\Delta t = V_2/V_b$). `sim_type mulp` is the multi-particle model, `env` the envelope model.

### Space charge

```avas-params
input:spacecharge input:scmethod input:numofgrid input:meshrms
```

`scmethod` can be `FFT`, `PICNIC` or `SPICNIC`; the manual only lists the names. The example project's `input.txt` uses `spacecharge 1` and `scmethod FFT`.

### Run settings

```avas-params
input:multithreading input:dumpperiodicity input:boundary input:longlimits input:pchistogram input:scanphase
```

- **Multithreading**: write `multithreading 1` to use several threads; for a single thread **leave the line out**. The manual says 0 means off, but the engine loses all particles at the start with `multithreading 0` (and 2, 3 …) (verified with the example project). The Settings page handles this.
- **Loss boundary**: a particle beyond the element aperture `R` counts as lost; with `boundary 1` the boundary in `boundary.txt` is used instead, the radii in the lattice no longer apply, and lost particles are written to `CollisionData.txt`. `longlimits 1 phase energy` enables longitudinal loss detection, corresponding to the separatrix of [longitudinal dynamics](#longitudinal).
- **Particle data**: `dumpperiodicity V1` writes all particles to `Beamset.plt` every $V_1$ steps (0 = never); the file can become very large. `pchistogram 1 bins` produces `pchistogram.dat`.
- **Phase scan**: `scanphase` controls a phase scan of the RF cavities or reads phases from `scanData.txt` (see [longitudinal dynamics](#longitudinal)).
- **GPU**: the GPU version of AVAS exists only as a precompiled library for Linux; on Windows the CPU computes with multiple threads.

### Output planes

```avas-params
outputplane automaticoutput
```

- `outputplane V1` writes the beam distribution $V_1$ downstream (positive) or upstream (negative) of the keyword's position, to `outData_<position>.dst`.
- `automaticoutput V1 V2 V3` inserts output planes starting at relative position $V_1$, spaced $V_2$, over at most $V_3$.
- An output plane **beyond the end of the lattice** is an error; **within 5 mm of the end** also fails, 2 cm from the end works. The engine always writes `outData_<end>.dst` at the end when it finishes, so no output plane is needed there.

### Main output files

| File | Content |
|---|---|
| `DataSet.txt` | bunch statistics, one line per recorded step (table below) |
| `inData.dst` | the initial distribution actually used |
| `outData_<z>.dst` | particle distribution at an output plane (binary) |
| `Phase.txt` | time, position and synchronous energy at the entrance and exit of each RF cavity |
| `synParticle.txt` | the synchronous particle along the line |
| `Beamset.plt` | all particles when `dumpperiodicity` is on |
| `SingleParticle.txt` | trajectory of a single-particle run |

### DataSet.txt

Each line has 41 columns (numbered from 0); the commonly used ones:

| Column | Content |
|---|---|
| 0 | mean kinetic energy (MeV) |
| 1, 3, 5 | centroid $x$, $y$, $z$ relative to the synchronous particle |
| 7–9, 10–12, 13–15 | $\alpha$, $\beta$, emittance of the three planes |
| 16, 18, 20 | rms $x$, $y$, $z$ (m) |
| 22, 24, 26 | maximum $x$, $y$, $z$ |
| 28 | surviving macro-particles |
| 29, 31, 33 | $x$, $y$, $z$ of the synchronous particle |
| 35 | flag: 0 straight, 1 curved, 2 invalid (skipped) |
| 40 | time |

On straight sections the longitudinal position is column 5 plus column 33; with dipoles in the line the arc length of curved sections must be accumulated (AVAS's reader does this). The engine recreates the file when a run starts and appends line by line; when reading a file that is still being written, discard the incomplete last line.

> **AVAS convention**: runs **do not rewrite InputFile**. A full run copies the text inputs to `<output folder>/inputs/`, and the engine uses the copies as input and the original folder for field maps, so every output folder carries a snapshot of its inputs. A full run overwrites OutputFile each time; results to keep must be "kept" in the interface (copied to the project's `Runs/`).

### Single-particle runs

With `particlenumber 1` the engine simulates a single particle and writes `SingleParticle.txt` ($x$, $y$, $z$, $\gamma\beta_{x,y,z}$, kinetic energy, time). The multi-particle keywords `twissx`, `twissy`, `twissz`, `current`, `distribution` and so on are ignored; `displacepos` and `displacedpos` set the particle's offset from the synchronous particle:

```avas-params
beam:displacepos beam:displacedpos
```

> **AVAS convention**: the engine **reads `displacepos dx dy dz` in mm** (the manual says m; verified with the engine); `displacedpos dpx dpy dpz` is in %. In a single-particle run the rms columns of DataSet.txt are NaN, which is normal; look at the trajectory and the synchronous-particle columns.

Single-particle runs are good for checking orbits: steerer kicks, dipole dispersion, alignment of field maps, or computing effective transfer-matrix elements from one offset particle.

### Common problems

**All particles lost at the start**: the engine reports `End of simulation, all particles lost`. Check in this order (the first item is verified with the engine, the others are plausible causes worth checking):

- `input.txt` contains `multithreading 0` (or 2, 3) — delete the line or set it to 1;
- the initial beam is too large for the aperture `R` (3 × rms already near the aperture), or the `initpos` offset is too large;
- `longlimits` is on with too narrow limits;
- the first element is a matrix element — the manual advises against it without saying what happens; a short drift in front rules it out.

**Field map not found**: the last parameter of `field` is the map's **base name** (without extension); the extensions follow the field type: RF fields `.edx/.edy/.edz` and `.bdx/.bdy/.bdz`, electrostatic `.esx/.esy/.esz`, magnetostatic `.bsx/.bsy/.bsz`. The field-map folder is InputFile by default and can be set with `[project] fieldSource` in `ini.ini`. The storage order of a map is decided from the whole group of component files (see [field maps](#field-maps)). The example maps are themselves noisy; stripes along $z$ in the slices are data, not a reading error.

**Output plane error**: the output plane lies beyond the end of the lattice or less than 5 mm from it. Move it to at least about 2 cm from the end, or simply use the `outData_<end>.dst` the engine writes at the end.

**Other**:

- **Only lines between `start` and `end` are simulated**: only lines after the first `start` and before the first `end` take effect; everything after `!` is a comment. Elements after `end` are not computed — a common slip after temporarily truncating a lattice while debugging.
- **Energy drops to zero or the beam turns back**: a wrong RF phase (the sign of the synchronous phase reversed, V3 misunderstood) decelerates the reference particle; the preview stops there with a warning.
- **NaN in rms columns**: normal in single-particle runs; in multi-particle runs it means too few surviving particles.
- **Results differ slightly every time**: normal with multithreaded computation; compare within the statistical error.

## Further reading {#simulation-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: chapter 9 (multiparticle simulation and space charge).
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: distribution functions and emittance growth.
- R. W. Hockney, J. W. Eastwood, *Computer Simulation Using Particles*, Adam Hilger, 1988: particle–mesh methods and numerical noise.
- C. K. Birdsall, A. B. Langdon, *Plasma Physics via Computer Simulation*: numerical heating and the choice of mesh in particle simulations.
- This primer's [space charge](#space-charge), [errors and correction](#errors) and [tutorial](#tutorial); the complete description of input and output files is in the interface's user manual (parameter and file reference).
