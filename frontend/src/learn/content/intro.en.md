# How AVAS models a beam

A bunch of the example project holds about 7.07 million protons, and every pair of them interacts through the Coulomb force: $2.5\times10^{13}$ pairs per step. A linac is tens to hundreds of metres long and takes tens of thousands of steps. How does a program make this computable? And why does it advance in tiny time steps through an RF cavity but cross a whole quadrupole in a single step?

This chapter explains the model AVAS uses for a beam: one reference particle plus the deviations of all other particles from it; a few thousand "macro-particles" standing for a huge number of real ones; two ways of integrating (t-code and z-code) and which elements each suits; why the input is split into several files, what happens during a run, and what comes out. The whole picture of a linac from ion source to target, and which of its sections AVAS covers, is in [A proton's journey](#journey-overview); the phase-space concepts used here are in [Phase space and emittance](#phase-space).

## What question AVAS answers {#intro-what-avas-simulates}

AVAS (Advanced Virtual Accelerator Software) is a multi-particle beam-dynamics code for linear accelerators. The question it answers is: **given a beam at the entrance and a beamline, what does the beam look like at the exit, and how many particles were lost on the way?** The computing kernel is a binary C++ library (`AVAS.dll` on Windows); the user interface and the pre- and post-processing are done in Python and a web front end.

The fundamental difference between a linac and a ring is that each particle passes every element only once. The beam quality is therefore set by **single-pass** dynamics: the phase-space distribution at the entrance, the fields of the elements and the beam's own space-charge forces together determine the energy, emittance and losses at the exit. A ring can correct its beam through the averaged effect of thousands of turns (radiation damping in electron rings, for example); a linac has no such chance, and the entrance distribution and the error of every element stay in the result.

AVAS simulates the part of the machine where the beam is already bunched and has some energy: drifts, quadrupoles, solenoids, dipoles, steerers, RF cavities (as field maps), static electric and magnetic field maps, and superpositions of these. Ion sources, the low-energy transport and the RFQ are not AVAS elements; to start at an RFQ exit, read a particle distribution file (`.dst`) produced by another code as the initial beam (`readparticledistribution`) — the example project's `part_rfq.dst` is such a file. The example project `examples/hwr010` itself is the start of the superconducting section of a 162.5 MHz proton linac: a 1.527 MeV, 0.18 mA proton beam and one 0.21 m half-wave resonator (HWR).

In low-energy, high-intensity linacs (for example the low-energy sections of superconducting proton and heavy-ion linacs), space charge, RF defocusing and longitudinal non-linearities often act at the same time and are intertwined, so that analytic theory can hardly treat them separately — which is why multi-particle simulation codes exist.

## Reference particle and deviations {#intro-reference}

The first step in describing a beam is to choose a **reference particle** (synchronous particle): it moves on the design orbit, and its kinetic energy $W(s)$ and arrival time $t(s)$ fix the phase of every RF cavity when it arrives, and with it the "design state" of every element. At the start of a run AVAS integrates the energy and time of the reference particle along the lattice, starting from the kinetic energy given in `beam.txt`, and determines the entry phase of every RF cavity (see [Longitudinal motion](#longitudinal)).

All other particles are described only by their **deviations from the reference particle**: transverse positions and angles $(x, x', y, y')$, longitudinal position and momentum deviation $(z, \delta)$. Why split it this way? Because the deviations are small. The example beam has an rms size of about 1 mm while the element aperture is 20 mm; an rms bunch length of 1.3 mm while a particle travels $\beta\lambda$ = 105 mm in one RF period; a relative momentum deviation of $3.5\times10^{-3}$. For such small deviations the fields can be expanded into linear terms plus small corrections, which gives simple and useful tools such as transfer matrices, Twiss parameters and the linear preview (see [Phase space and emittance](#phase-space)). Beam size, emittance and energy spread are statistics of these deviations.

The split has its limits: when the deviations are no longer small — the bunch covers a large part of the RF waveform, halo particles reach the edge of the aperture — the linear approximation fails, and the particles have to be tracked one by one through the real fields. That is the kernel's job.

## Macro-particles: a few thousand points for millions of protons {#intro-macroparticles}

The bunch charge of the example is $Q = I/f$ = 0.1841 mA / 162.5 MHz = 1.133 pC, about $7.07\times10^{6}$ protons. AVAS represents them by `particlenumber` **macro-particles** — 5260 in the example, each standing for about 1344 protons.

Why does this not change the physics? The equation of motion in external fields is $\dd\boldsymbol p/\dd t = q(\boldsymbol E + \boldsymbol v\times\boldsymbol B)$: the force is proportional to the charge, the inertia to the mass, and the trajectory depends only on the charge-to-mass ratio $q/m$. A macro-particle has 1344 times the charge and the mass of a real proton, so its charge-to-mass ratio is unchanged and its trajectory in external fields is exactly that of a real proton. Only the space charge uses the total charge, each macro-particle contributing $Q/N$.

Nor is the space charge computed pair by pair. With the algorithm selected by `scmethod` in `input.txt` (FFT, PICNIC, SPICNIC) the kernel derives the beam's self-field from the charge distribution of the macro-particles and applies it to each of them (the FFT method, for example, solves Poisson's equation on a grid); the cost grows with the number of particles and the grid size, not with the square of the number of particles.

The replacement has a price, and here the analogy breaks: a charge distribution drawn by a few thousand macro-particles is far "grainier" than one made of millions of real protons; the self-field fluctuates more than in the real beam and causes spurious emittance growth (numerical noise), and every statistic carries a random error proportional to $1/\sqrt N$. How to choose the number of macro-particles and how large the errors are is discussed in [Initial distributions](#distributions-statistics) and [Multi-particle simulation essentials](#simulation).

> **Think about it**: since a macro-particle follows the same trajectory as a real particle, what can a single-particle run (`particlenumber 1`) tell us, and what can it not?
>
> Answer: it gives the trajectory, energy and phase of one particle (which `displacepos` can move off the reference orbit) in the external fields, which is a good check of the lattice and the field maps. It cannot give any statistics — rms sizes and emittances need a cloud of particles, which is why the rms columns of a single-particle run may be NaN — and it has no space charge.

## t-code and z-code {#intro-tracking}

AVAS elements come in two models that are computed differently. Think of them as two ways of photographing a bunch: the t-code is like a snapshot of the whole bunch at one instant, the z-code like a photo-finish where each particle is recorded as it crosses the same plane.

### Field-model elements (t-code)

For `field` elements (and drifts `drift`) the user supplies the three-dimensional field distribution (field-map files). The kernel takes **time $t$ as the independent variable** and integrates the equation of motion of every macro-particle step by step,

$$
\frac{\dd \boldsymbol p}{\dd t} = q\left(\boldsymbol E + \boldsymbol v \times \boldsymbol B\right) + \boldsymbol F_{\mathrm{sc}}
$$

where $\boldsymbol F_{\mathrm{sc}}$ is the space-charge force. This is called the t-code. Why must an RF cavity be computed this way? The field in a cavity oscillates in time, so the field a particle sees depends on **when** it arrives where; particles at the head and at the tail of the bunch see different phases at the same position, and a low-energy particle still changes its speed while crossing the cavity. With time as the independent variable all of this is included naturally, and so are field non-linearities and fringe fields.

The time step is set by `steppercycle` in `input.txt`: each RF period is divided into $V_1$ steps, $\Delta t = 1/(f V_1)$. With the example's `steppercycle 50`, $\Delta t$ = 0.123 ns, each step advances $\beta\lambda/50$ = 2.1 mm, and crossing the 0.21 m HWR cavity takes about 100 steps.

### Matrix-model elements (z-code)

`quad`, `solenoid`, `bend`, `edge` and `steerer` need only a few parameters (length, gradient, field …). The kernel takes the **longitudinal position $z$ as the independent variable** and transforms the particle coordinates with hard-edge transfer matrices; this is called the z-code. A static magnetic field does not change in time, so when a particle arrives does not matter: from its coordinates at the entrance plane its coordinates at the exit plane follow in one step. The z-code is fast and can be compared directly with analytic theory (see [Linear optics](#linear-optics)), but it contains no extended fringe-field distribution (`edge` only adds a thin-lens pole-face correction); whether the kernel's matrix elements include higher-order terms is not documented in the manual, so the kernel is the reference.

Both models can be mixed in one lattice; at each boundary between them the particle distribution has to be converted between the "same time" and the "same longitudinal position" descriptions (how the kernel does this is not documented in the manual; the kernel is the reference).

The beamline below puts the two models together: the solenoid field map and the HWR cavity field map use the t-code, the quadrupole doublet behind them the z-code, and both ends are drifts. The picture shows the example beam's linear preview; the exit energy is about 1.88 MeV.

```avas-beamline
drift 0.05 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010
drift 0.1 0.02 0
quad 0.08 0.02 0 8
drift 0.08 0.02 0
quad 0.08 0.02 0 -8
drift 0.15 0.02 0
```

> **Think about it**: for the same static quadrupole, should integrating in time with the t-code and crossing it in one step with the z-code give the same result?
>
> Answer: physically the trajectory is the same; the differences come from the model, not from the independent variable: the z-code uses a hard-edge matrix without fringe fields or higher-order terms, the t-code a field map with both. Besides, the two give the distribution on different "snapshots" — the same time or the same plane — which usually makes little difference for rms quantities.

## The linear preview: an estimate in seconds {#intro-preview}

One kernel run of the example project takes about 10–30 seconds; long beamlines, many particles and error studies take much longer. While tuning parameters we often only want to know "roughly what the envelope does and whether the energy is right". For this the interface provides a **linear envelope preview** ("Linear preview" in the visual editor). It does not call the kernel; it pushes the second moments of the beam with 6×6 matrices, $\Sigma \mapsto M\Sigma M\tp$, and approximates field maps by slices. The principle is described in [Linear optics](#linear-optics).

Its accuracy: reference-particle energy within about 0.01 % (compared with the engine's synchronous particle; the energy in the engine's DataSet is the mean of all macro-particles, which see a different $E_z$ off axis: in the example HWR the mean gain is about 5 % above the reference particle's), rms sizes usually within 1–2 %, up to 13 % when multi-particle effects are strong. It looks only at second moments, so it cannot see the shape of the distribution, halo or losses (see [Initial distributions](#distributions-real-beam)). The hands-on labs of this primer are computed on the back end with the same linear-optics code and the same approximations. **Final conclusions rest on the kernel's multi-particle simulation.**

## Input files and workflow {#intro-workflow}

An AVAS project has two directories, `InputFile/` and `OutputFile/`. The input is split into several files because they answer different questions: `beam.txt` says **what beam comes in**, the lattice file says **what the machine looks like**, the field-map files hold the **field data** of its elements, `input.txt` says **how to compute** (simulation type, step, space-charge algorithm …), and `ini.ini` holds project settings. Split this way, a new beam does not touch the lattice, and a new algorithm does not touch the physical input. The contents of each file are listed in [this chapter's reference part](#intro-in-avas).

A complete run goes like this:

1. Edit the inputs on the "Beam", "Lattice" and "Settings" pages of the interface (or edit the text files directly).
2. Start the run on the "Run" page. A run **never rewrites** InputFile: the input files are first copied to `<output dir>/inputs/`; the kernel uses the copies as input and the original directory as field-map directory. The output directory thus always keeps the input that belongs to its results; that is also why all input files are read-only while a full run, an error study or a segment run is in progress (including when paused).
3. At the start of the run the kernel recreates `OutputFile/DataSet.txt` and then appends one row at a time; the interface reads the new rows every second and shows the beam envelope live.
4. When the run has finished, plot on the "Results" page or replay the run on the "Run" page.

## Output: one row of bunch statistics per step {#intro-outputs}

The t-code advances in time steps, and along the way the kernel computes statistics over all macro-particles and writes them as one row of `DataSet.txt`: energy and position of the reference particle, rms sizes and maxima in every direction, the number of surviving macro-particles and more, 41 columns in all. The rows are written at the kernel's output steps; how many time steps lie between two rows is not documented in the manual (look at the spacing of the $z$ column of your own run, and compare it with the step length $\beta\lambda/50 \approx 2.1$ mm of the example). The rows of `DataSet.txt` therefore correspond to output steps in time, not to elements, and the position has to be computed from the columns (see [the reference part](#intro-in-avas)). For the particles themselves, look at the `.dst` files: `inData.dst` is the initial distribution the simulation actually used, `outData_<z>.dst` the distribution at an output plane.

> **Think about it**: run the same input twice — are the two `DataSet.txt` files identical?
>
> Answer: no. The kernel runs multithreaded and the results differ slightly from run to run, so two runs must be compared with a tolerance; conclusions drawn from differences smaller than the statistical error are not reliable.

## How to read this primer {#intro-how-to-read}

The primer has four parts:

- **A proton's journey**: six chapters starting with [From ion source to target](#journey-overview), following a linac in order: why each section exists, what it is made of, and which of them AVAS can simulate.
- **Fundamentals**: this chapter, [Relativistic kinematics](#kinematics), [Phase space and emittance](#phase-space), [Initial distributions](#distributions), [Linear optics](#linear-optics), [Longitudinal motion](#longitudinal), [Space charge](#space-charge). These chapters set up the concepts and notation used later.
- **Components**: [Drift](#drift), [Quadrupole](#quadrupole), [Solenoid](#solenoid), [Dipole](#dipole), [Steerer](#steerer), [RF cavity](#rf-cavity), [Static fields](#static-fields), [Field maps](#field-maps), [Superpose](#superpose).
- **Building an accelerator**: [Lattice design](#lattice-design), [Matching](#matching), [Errors and correction](#errors), [Multi-particle simulation essentials](#simulation), [Tutorial](#tutorial), [References](#references).

Each chapter first builds understanding; its "In AVAS" part collects keywords, parameter tables and conventions for reference. Keep the following in mind while reading:

- **Labs are linear models.** They update live as you drag the sliders and are meant to build intuition; their accuracy is given under [The linear preview](#intro-preview).
- **Field pictures** distinguish data from schematics: slices of field-map files are data; the fields of matrix elements are schematics drawn from the parameters and are labelled as such. Cross-sections are always viewed from downstream looking upstream, with the beam coming out of the screen (⊙), so that force directions can be checked directly with $\boldsymbol F = q\boldsymbol v \times \boldsymbol B$.
- **"AVAS convention"** boxes mark conventions in which AVAS differs from textbooks or the manual and which have been verified with the kernel (for example the units of the Twiss parameters, `displacepos` being read in mm, `multithreading` accepting only 1).
- Every chapter carries a review status. The current chapters are **written by AI, awaiting review**: formulas and numbers have been checked by computation, but have not yet been reviewed line by line by a physicist; please report problems to the maintainer.

## Common misconceptions {#intro-misconceptions}

- **"A macro-particle is a proton."** A macro-particle stands for many real particles (about 1344 in the example) with the same charge-to-mass ratio, so its trajectory in external fields is the same; but the graininess of the space charge and the statistical errors both come from the finite number of macro-particles.
- **"AVAS simulates the whole accelerator."** Ion sources, the low-energy transport and the RFQ are not AVAS elements; the simulation starts from their output distribution.
- **"The linear preview and the kernel compute the same thing."** The preview pushes only second moments, without distribution shape, losses or full space charge; it is an estimate, the kernel gives the result.
- **"Matrix elements and field-map elements are just two ways of writing the same thing."** Matrix elements are hard-edge models without fringe fields; field-map elements integrate the real three-dimensional field in time. The same magnet written both ways can give different results.
- **"A run rewrites InputFile."** It does not; the kernel uses the copies in `<output dir>/inputs/`.
- **"Each row of `DataSet.txt` belongs to one element."** Each row is one statistics output at an output step in time, unrelated to element boundaries; the position has to be computed from the columns.
- **"The same input always gives the same result."** Multithreading makes results differ slightly from run to run; compare with a tolerance.

## In AVAS {#intro-in-avas}

### The six things the kernel does

1. **Reference particle**: starting from the kinetic energy given in `beam.txt`, it integrates the energy and time of the reference particle along the lattice and determines the entry phase of every RF cavity (see [Longitudinal motion](#longitudinal)).
2. **Macro-particles**: it generates `particlenumber` macro-particles from the Twiss parameters and distribution type in `beam.txt` (or reads a particle distribution file, see [Initial distributions](#distributions)). Each macro-particle stands for many real particles; the total charge follows from the current `current` and the beam frequency `frequency` (bunch charge $Q = I/f$).
3. **External fields**: field-map elements and drifts are tracked with the t-code, integrating the Lorentz force in time steps; matrix elements use the transfer matrices of the z-code (see [t-code and z-code](#intro-tracking)).
4. **Space charge**: with `spacecharge 1` in `input.txt`, the beam's self-field is computed with the algorithm selected by `scmethod` (FFT, PICNIC, SPICNIC) and applied to every macro-particle (see [Space charge](#space-charge)).
5. **Errors and correction**: commands such as `err_step`, `err_beam_dyn`, `err_beam_stat`, `err_quad_*`, `err_cav_*` in the lattice add random errors to elements and to the initial beam; `adjust` and `diag_*` perform static error correction (see [Errors and correction](#errors)).
6. **Losses**: particles leaving the element aperture `R` or the longitudinal limits are counted as lost; the number of surviving macro-particles along $s$ is written to the output.

### Beamline elements

| Function | Typical elements | Written in AVAS as | See |
|---|---|---|---|
| Transverse focusing | quadrupoles, solenoids | `quad`, `solenoid`, or a static magnetic field map `field … 3 …` | [Quadrupole](#quadrupole), [Solenoid](#solenoid) |
| Acceleration and longitudinal focusing | RF cavities (e.g. half-wave resonators, HWR) | RF field map `field … 1 …` | [RF cavity](#rf-cavity) |
| Bending and orbit correction | dipoles, steerers | `bend`, `edge`, `steerer` | [Dipole](#dipole), [Steerer](#steerer) |
| Diagnostics and output | diagnostic locations, output planes | `diag_energy`, `diag_size`, `diag_position`, `outputplane` | [Errors and correction](#errors), [Multi-particle simulation essentials](#simulation) |

In addition there are drifts `drift` (field-free space, see [Drift](#drift)), static electric field maps `field … 2 …` (see [Static fields](#static-fields)), and `superpose` blocks that let several field maps overlap in space (see [Superpose](#superpose)).

> **AVAS convention**: in multi-particle simulations **do not use a matrix element as the first or last element** of the lattice (a requirement of the manual); if necessary, add a very short `drift` at either end.

### Input files

| File | Contents | See |
|---|---|---|
| `beam.txt` | particle (`particlerestmass`, `numofcharge`), kinetic energy `kneticenergy`, current `current`, beam frequency `frequency`, number of macro-particles, distribution type `distribution`, Twiss parameters `twissx`/`twissy`/`twissz` | [Kinematics](#kinematics-in-avas), [Phase space](#phase-space-in-avas), [Initial distributions](#distributions-in-avas) |
| `input.txt` | simulation type `sim_type`, step `steppercycle`, space-charge switch and solver, grid, output settings … | [Multi-particle simulation essentials](#simulation) |
| lattice file | the element sequence: the lines between `start` and `end`; `!` starts a comment | [Lattice design](#lattice-design) |
| field-map files | the 3-D field distributions referenced by field-map elements, one file per component (e.g. `hwr010.edz`, `hwr010.edx`, `sol.bsz` …) | [Field maps](#field-maps) |
| `ini.ini` | project settings; `[lattice] source` selects the lattice file used for runs | — |

The example project `examples/hwr010` uses a 1.527 MeV, 0.18 mA, 162.5 MHz proton beam; its lattice contains a single 0.21 m HWR RF cavity field map:

```text
start
field  0.21   0.02     0   1   162.5e6   -33   1.36    -1.36   efield
end
```

The example `input.txt` has `sim_type mulp`, `spacecharge 1`, `steppercycle 50`, `scmethod FFT`.

### Output files

Each row of `DataSet.txt` holds the bunch statistics at one time (or position), 41 columns in all:

| Column | Contents |
|---|---|
| 0 | kinetic energy of the reference particle (MeV) |
| 5 + 33 | longitudinal position $z$ (sum of the two columns) |
| 16, 18, 20 | rms sizes $x$, $y$, $z$ (m) |
| 22, 24, 26 | maxima $x$, $y$, $z$ (m) |
| 28 | number of surviving macro-particles |

Other outputs include:

- `inData.dst`: the initial particle distribution actually used by the simulation;
- `outData_<z>.dst`: the particle distribution at an output plane (binary); at the end the kernel always writes the distribution at the end of the lattice;
- `Phase.txt`: time, position and energy of the synchronous particle at the entry and exit of each RF cavity (two lines per cavity);
- `synData.txt`: time, $\gamma\beta$, synchronous phase $\varphi_s$ and RF phase $\varphi_{\mathrm{RF}}$ of the synchronous particle at the entry of every element;
- the results of error studies, under `OutputFile/error_output/`.

> Note: in single-particle runs (`particlenumber 1`) the reference-particle data are written as usual while the rms columns may be NaN; this is normal. With multithreading the kernel gives slightly different results from run to run, so compare two `DataSet.txt` files with a tolerance.

## Further reading {#intro-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: the standard textbook on linac physics, covering RF, longitudinal and transverse dynamics and space charge.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: a systematic treatment of beam optics, emittance and space charge.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific.
- The CERN Accelerator School (CAS) proceedings, in particular "Introduction to Accelerator Physics" and "RF for Accelerators".
- The AVAS user manual `docs/使用说明20260427.docx` (the "Parameter and file reference" under Help → User manual in the interface) and the [References](#references) chapter of this primer.
