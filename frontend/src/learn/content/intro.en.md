# Accelerators and AVAS

This primer is written for accelerator physicists and graduate students who use AVAS in their research. It starts from the basic ideas of a linear accelerator, goes through relativistic kinematics, phase space and linear optics, introduces every element AVAS can simulate, and finally shows how to assemble them into a beamline and carry out matching, error studies and multi-particle simulations. This chapter gives the overall picture: what a linac is made of, what AVAS simulates, which files a simulation goes through from input to output, and how to read this primer.

## What a linac does {#intro-linac}

A linear accelerator (linac) guides charged particles along an approximately straight reference trajectory through a sequence of accelerating gaps and focusing elements, so that the particles gain energy while the beam stays confined to a finite transverse size and a finite longitudinal (phase) extent. Unlike in a ring, each particle passes every element only once, so the beam quality is set by **single-pass** dynamics: the phase-space distribution at the entrance, the fields of the elements and the beam's own space-charge forces together determine the energy, emittance and losses at the exit.

Two kinds of quantities describe a beam:

- the motion of the **reference particle** (synchronous particle): it moves on the design orbit, and its kinetic energy $W(s)$ and arrival time $t(s)$ fix the phases of the RF cavities and the "design state" of every element;
- the **deviations of all other particles from the reference particle**: transverse positions and angles $(x, x', y, y')$, longitudinal position and momentum deviation $(z, \delta)$. Beam size, emittance and energy spread are statistics of these deviations, see [Phase space and emittance](#phase-space).

In low-energy, high-intensity linacs (for example the low-energy sections of superconducting proton and heavy-ion linacs), space charge, RF defocusing and longitudinal non-linearities often act at the same time — which is why multi-particle simulation codes exist.

## Building blocks of a beamline {#intro-components}

A linac beamline can be divided into four functional classes of elements. The table lists how each is written in AVAS:

| Function | Typical elements | Written in AVAS as | See |
|---|---|---|---|
| Transverse focusing | quadrupoles, solenoids | `quad`, `solenoid`, or a static magnetic field map `field … 3 …` | [Quadrupole](#quadrupole), [Solenoid](#solenoid) |
| Acceleration and longitudinal focusing | RF cavities (e.g. half-wave resonators, HWR) | RF field map `field … 1 …` | [RF cavity](#rf-cavity) |
| Bending and orbit correction | dipoles, steerers | `bend`, `edge`, `steerer` | [Dipole](#dipole), [Steerer](#steerer) |
| Diagnostics and output | diagnostic locations, output planes | `diag_energy`, `diag_size`, `diag_position`, `outputplane` | [Errors and correction](#errors), [Simulation settings](#simulation) |

In addition there are drifts `drift` (field-free space, see [Drift](#drift)), static electric field maps `field … 2 …` (see [Static fields](#static-fields)), and `superpose` blocks that let several field maps overlap in space (see [Superpose](#superpose)).

> Note: this primer only covers what AVAS can simulate. Dedicated models of RFQ or DTL structures, ion sources and the like are not AVAS elements and are not presented as AVAS features here. To start from the beam at an RFQ exit, read a particle distribution file (`.dst`) produced by another code as the initial beam (`readparticledistribution`).

## What AVAS simulates {#intro-what-avas-simulates}

AVAS (Advanced Virtual Accelerator Software) is a multi-particle beam-dynamics code for linear accelerators. The computing kernel is a binary C++ library (`AVAS.dll` on Windows); the user interface and the pre- and post-processing are done in Python and a web front end. What the kernel does can be summarised as follows:

1. **Reference particle**: starting from the kinetic energy given in `beam.txt`, it integrates the energy and time of the reference particle along the lattice and determines the entry phase of every RF cavity (see [Longitudinal motion](#longitudinal)).
2. **Macro-particles**: it generates `particlenumber` macro-particles from the Twiss parameters and distribution type in `beam.txt` (or reads a particle distribution file). Each macro-particle stands for many real particles; the total charge follows from the current `current` and the beam frequency `frequency` (bunch charge $Q = I/f$).
3. **External fields**: field-map elements and drifts are tracked with the t-code, integrating the Lorentz force in time steps; matrix elements use the transfer matrices of the z-code (see [the next section](#intro-tracking)).
4. **Space charge**: with `spacecharge 1` in `input.txt`, the beam's self-field is computed with the algorithm selected by `scmethod` (FFT, PICNIC, SPICNIC) and applied to every macro-particle (see [Space charge](#space-charge)).
5. **Errors and correction**: commands such as `err_step`, `err_beam_dyn`, `err_beam_stat`, `err_quad_*`, `err_cav_*` in the lattice add random errors to elements and to the initial beam; `adjust` and `diag_*` perform static error correction (see [Errors and correction](#errors)).
6. **Losses**: particles leaving the element aperture `R` or the longitudinal limits are counted as lost; the number of surviving macro-particles along $s$ is written to the output.

## t-code and z-code {#intro-tracking}

AVAS elements come in two models that are computed differently.

### Field-model elements (t-code)

For `field` elements (and drifts `drift`) the user supplies the three-dimensional field distribution (field-map files). The kernel takes **time $t$ as the independent variable** and integrates the equation of motion of every macro-particle step by step,

$$
\frac{\dd \boldsymbol p}{\dd t} = q\left(\boldsymbol E + \boldsymbol v \times \boldsymbol B\right) + \boldsymbol F_{\mathrm{sc}}
$$

where $\boldsymbol F_{\mathrm{sc}}$ is the space-charge force. This is called the t-code. The time step is set by `steppercycle` in `input.txt` (the number of steps per RF period). The t-code naturally includes field non-linearities, fringe fields and the change of particle velocity inside a cavity, which is essential for low-energy RF cavities.

### Matrix-model elements (z-code)

`quad`, `solenoid`, `bend`, `edge` and `steerer` need only a few parameters (length, gradient, field …). The kernel takes the **longitudinal position $z$ as the independent variable** and transforms the particle coordinates with hard-edge transfer matrices; this is called the z-code. It is fast and can be compared directly with analytic theory (see [Linear optics](#linear-optics)), but contains no extended fringe-field distribution (`edge` only adds a thin-lens pole-face correction); whether the kernel's matrix elements include higher-order terms is not documented in the manual, so the kernel is the reference.

Both models can be mixed in one lattice; at each boundary between them the particle distribution has to be converted between the "same time" and the "same longitudinal position" descriptions (how the kernel does this is not documented in the manual).

> AVAS convention: in multi-particle simulations **do not use a matrix element as the first or last element** of the lattice (a requirement of the manual); if necessary, add a very short `drift` at either end.

The AVAS interface also provides a **linear envelope preview** ("Linear preview" in the visual editor). It does not call the kernel; it pushes the second moments of the beam with 6×6 matrices and approximates field maps by slices. Its principle is described in [Linear optics](#linear-optics-6d); its accuracy and range of validity are given in [the last section of this chapter](#intro-how-to-read).

## Input files and workflow {#intro-workflow}

An AVAS project has two directories, `InputFile/` and `OutputFile/`. The input consists of:

| File | Contents | See |
|---|---|---|
| `beam.txt` | particle (`particlerestmass`, `numofcharge`), kinetic energy `kneticenergy`, current `current`, beam frequency `frequency`, number of macro-particles, distribution type `distribution`, Twiss parameters `twissx`/`twissy`/`twissz` | [Kinematics](#kinematics-in-avas), [Phase space](#phase-space-in-avas) |
| `input.txt` | simulation type `sim_type`, step `steppercycle`, space-charge switch and solver, grid, output settings … | [Simulation settings](#simulation) |
| lattice file | the element sequence: the lines between `start` and `end`; `!` starts a comment | [Lattice design](#lattice-design) |
| field-map files | the 3-D field distributions referenced by field-map elements, one file per component (e.g. `hwr010.edz`, `hwr010.edx`, `sol.bsz` …) | [Field maps](#field-maps) |
| `ini.ini` | project settings; `[lattice] source` selects the lattice file used for runs | — |

The example project `examples/hwr010` uses a 1.527 MeV, 0.18 mA, 162.5 MHz proton beam; its lattice contains a single 0.21 m HWR RF cavity field map:

```text
start
field  0.21   0.02     0   1   162.5e6   -33   1.36    -1.36   efield
end
```

A complete run goes like this:

1. Edit the inputs on the "Beam", "Lattice" and "Settings" pages of the interface (or edit the text files directly).
2. Start the run on the "Run" page. A run **never rewrites** InputFile: the input files are first copied to `<output dir>/inputs/`; the kernel uses the copies as input and the original directory as field-map directory.
3. At the start of the run the kernel recreates `OutputFile/DataSet.txt` and then appends one row at a time; the interface reads the new rows every second and shows the beam envelope live.
4. When the run has finished, plot on the "Results" page or replay the run on the "Run" page.

## Outputs and results {#intro-outputs}

The most used output is `DataSet.txt`: every row holds the bunch statistics at one time (or position), 41 columns in all. Column 0 is the kinetic energy of the reference particle (MeV), the longitudinal position $z$ is the sum of columns 5 and 33, the rms sizes $x$, $y$, $z$ are in columns 16, 18, 20 (in m), the maxima in columns 22, 24, 26, and the number of surviving macro-particles in column 28. Other outputs include:

- `inData.dst`: the initial particle distribution actually used by the simulation;
- `outData_<z>.dst`: the particle distribution at an output plane (binary); at the end the kernel always writes the distribution at the end of the lattice;
- `Phase.txt`: time, position and energy of the synchronous particle at the entry and exit of each RF cavity (two lines per cavity);
- `synData.txt`: time, $\gamma\beta$, synchronous phase $\varphi_s$ and RF phase $\varphi_{\mathrm{RF}}$ of the synchronous particle at the entry of every element;
- the results of error studies, under `OutputFile/error_output/`.

> Note: in single-particle runs (`particlenumber 1`) the reference-particle data are written as usual while the rms columns may be NaN; this is normal. With multithreading the kernel gives slightly different results from run to run, so compare two `DataSet.txt` files with a tolerance.

## How to read this primer {#intro-how-to-read}

The primer has three parts:

- **Fundamentals**: [Relativistic kinematics](#kinematics), [Phase space and emittance](#phase-space), [Linear optics](#linear-optics), [Longitudinal motion](#longitudinal), [Space charge](#space-charge). These chapters set up the concepts and notation used later.
- **Components**: [Drift](#drift), [Quadrupole](#quadrupole), [Solenoid](#solenoid), [Dipole](#dipole), [Steerer](#steerer), [RF cavity](#rf-cavity), [Static fields](#static-fields), [Field maps](#field-maps), [Superpose](#superpose). Each chapter covers the role of the element, its physics, its transfer matrix, the AVAS keywords, adjustable parameters, common problems and hands-on labs.
- **Building an accelerator**: [Lattice design](#lattice-design), [Matching](#matching), [Errors and correction](#errors), [Simulation settings](#simulation), [Tutorial](#tutorial), [References](#references).

Keep the following in mind while reading:

- **Labs are linear models.** The lab blocks are computed on the back end with AVAS's own linear-optics code and update live as you drag the sliders; they are meant to build intuition. They use the same approximations as the linear envelope preview: reference-particle energy within about 0.01 %, rms sizes usually within 1–2 %, up to 13 % when multi-particle effects are strong. **Final conclusions rest on the kernel's multi-particle simulation.**
- **Field pictures** distinguish data from schematics: slices of field-map files are data; the fields of matrix elements are schematics drawn from the parameters and are labelled as such. Cross-sections are always viewed from downstream looking upstream, with the beam coming out of the screen (⊙), so that force directions can be checked directly with $\boldsymbol F = q\boldsymbol v \times \boldsymbol B$.
- **"AVAS convention"** boxes mark conventions in which AVAS differs from textbooks or the manual and which have been verified with the kernel (for example the units of the Twiss parameters, `displacepos` being read in mm, `multithreading` accepting only 1).
- Every chapter carries a review status. The current chapters are **written by AI, awaiting review**: formulas and numbers have been checked by computation, but have not yet been reviewed line by line by a physicist; please report problems to the maintainer.

## Further reading {#intro-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: the standard textbook on linac physics, covering RF, longitudinal and transverse dynamics and space charge.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: a systematic treatment of beam optics, emittance and space charge.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific.
- The CERN Accelerator School (CAS) proceedings, in particular "Introduction to Accelerator Physics" and "RF for Accelerators".
- The "Parameter and file reference" in the AVAS user manual (Help → User manual) and the [References](#references) chapter of this primer.
