# How AVAS simulates a beam

One bunch of the example project holds about 7.07 million protons. Protons all carry positive charge, every two of them push each other, and a single step means $2.5\times10^{13}$ pairs; a linear accelerator is tens to hundreds of metres long and takes tens of thousands of steps. Computed by brute force, that would keep a computer busy for years. AVAS needs a dozen seconds. How?

The answer is three plain ideas, and this chapter explains them one by one:

1. **Watch the group through its leader**: pick a "reference particle" and record for every other particle only how much it differs from the reference.
2. **A sample survey**: let a few thousand "macroparticles" stand for millions of real protons, just as an opinion poll asks only a few thousand people.
3. **Two ways of taking pictures**: the field in an RF cavity changes with time, so the cavity is crossed in small time steps; the field of a quadrupole does not change, so the whole element can be crossed in one step.

First, look at what a beam in a beam line looks like. In the animation below the orange dots are 60 protons drawn from the example beam, the two blue dots are particles we follow in particular, and the green blocks are quadrupoles (magnets that hold the beam together, see [Quadrupole](#quadrupole)). Press play:

```avas-motion
fodo
```

The swarm keeps swinging around a centre line. **The centre line is the path of the reference particle**; how far each dot is from it and in which direction it flies is its "deviation". This is exactly what AVAS computes: how such a swarm moves on, only with more dots (thousands), with real three-dimensional fields, and including the forces between the particles.

## What question AVAS answers {#intro-what-avas-simulates}

AVAS (Advanced Virtual Accelerator Software) is a **multi-particle beam dynamics program for linear accelerators**. First the words: a **linear accelerator** accelerates particles step by step along a straight line; a **beam** is a large swarm of particles flying in the same direction; **multi-particle** means that the program follows a large swarm rather than just one particle; **dynamics** is the study of how they move under forces.

The question AVAS answers is: **given a beam at the entrance and a beam line, what does the beam look like at the exit, and how many particles are lost on the way?** The **beam line** (also called the lattice) is a chain of **elements** placed end to end: RF cavities that accelerate the particles, magnets that hold the beam together, drifts with nothing in them, and so on. The computing part (the engine) is a binary library written in C++ (`AVAS.dll` on Windows); the user interface and the pre- and post-processing are written in Python and a web front end.

A linear accelerator has one special feature: **the particles pass every element only once**. It is like a mountain road you walk only once, while a ring accelerator is like a running track around which the particles go thousands of times. In a ring many effects average out over the turns (in electron rings, for instance, radiation damping slowly "repairs" the beam); a linear accelerator has no such chance, and the shape of the incoming beam, the errors of every element and the beam's own space-charge force (the push between the particles) all stay in the result at the exit.

AVAS simulates the beam from the point where it is "already bunched and has some energy": drifts, quadrupoles, solenoids, dipoles, steerers, RF cavities (from field maps), static electric and magnetic field maps, and their superpositions. The ion source and the low-energy transport line **are not simulated by AVAS**, and the RFQ (radio-frequency quadrupole) usually is not either (the new kernel also has an `rfq` element, not yet checked against other codes; see [RFQ](#journey-rfq-in-avas)); to start at the RFQ exit, read a particle distribution file (`.dst`) produced by another program as the initial beam (keyword `readparticledistribution`); `part_rfq.dst` in the example project is such a file. The whole of an accelerator, from the ion source to the target, is described in [A proton's journey](#journey-overview).

The example project `examples/hwr010` is the beginning of the superconducting section of a 162.5 MHz proton linac: a 1.527 MeV, 0.18 mA proton beam and one 0.21 m long half-wave resonator (HWR, a type of superconducting RF cavity).

Why do we need such a program? In low-energy, high-current linacs (for example the low-energy sections of superconducting proton or heavy-ion linacs), space charge, the defocusing of the RF cavities and the longitudinal non-linearity often act at the same time and are tangled together, hard to separate with pen-and-paper formulas. Letting a computer follow the particles one by one is the most reliable way.

## Reference particle and deviations {#intro-reference}

### Watching a cycling team through its leader

Picture a cycling team on the road. To say where everyone is, you need not give each rider's latitude and longitude; it is enough to say "30 cm to the left of the leader, 2 m behind, and a tiny bit faster". When the leader moves, all the descriptions move with him.

A beam is described the same way. First pick a **reference particle** (also called the **synchronous particle**): it flies along the very centre of the designed orbit, with exactly the design energy. For every other particle only its **deviation from the reference particle** is recorded, six numbers in all:

- how far from the centre line it is horizontally ($x$), and the angle at which it heads off horizontally ($x'$);
- the same two numbers vertically ($y$, $y'$);
- how far ahead of the reference particle it is along the direction of flight ($z$), and by what fraction its momentum exceeds the reference particle's ($\delta$; momentum is mass times velocity, see [Relativistic kinematics](#kinematics)).

The meaning and units of these six numbers are explained in detail in [Phase space and emittance](#phase-space).

The reference particle has one more important job: it **sets the beat of every RF cavity**. The electric field in an RF cavity swings back and forth more than a hundred million times a second, and a particle is accelerated only if it crosses at the moment when the field points forward. When AVAS runs, it first starts from the energy given in `beam.txt` and computes along the beam line when, and with what energy, the reference particle reaches every cavity; this fixes the entrance phase of every cavity (see [Longitudinal motion](#longitudinal)).

### Why split it this way: the deviations are small

The benefit of this split is that the deviations are small. Look at a few numbers of the example beam:

| Quantity | Typical deviation | Compared with |
|---|---|---|
| Transverse position | rms about 1 mm | pipe radius 20 mm |
| Position along the bunch | rms 1.3 mm | a particle flies 105 mm in one RF period |
| Momentum | relative deviation 0.35 % | — |

(rms means "root mean square"; think of it as the typical size, see [Phase space and emittance](#phase-space).)

Small deviations allow a "straight-line approximation": over a small enough range any curve looks like a straight line, just as you cannot feel that the Earth is round over a few steps. The effect of an element on a particle can then be written as simple multiplications and additions, and this is where transfer matrices, Twiss parameters and the linear preview come from. Why "small" allows a straight-line approximation, and how small is small enough, is shown in this chapter's deeper section [Why small deviations allow a straight-line approximation](#intro-deep-linear).

The split has its limits: once the deviations are no longer small, for example when the bunch grows to cover a large part of the RF wave or halo particles reach the edge of the aperture, the straight-line approximation fails, and every particle has to be followed step by step in the real field. That is the engine's job.

## Macroparticles: a few thousand dots for millions of protons {#intro-macroparticles}

### A sample survey

An opinion poll does not ask every person in the country; it asks a few thousand people at random, and each of them "stands for" tens of thousands of similar people. With a good sample the result is accurate; the fewer people asked, the larger the random error.

AVAS does the same with a beam. How many protons are there in an example bunch? The **beam current** is the charge flowing past per second, 0.1841 mA for the example, that is $1.841\times10^{-4}$ C per second; and $1.625\times10^{8}$ bunches fly past per second (bunch frequency 162.5 MHz). So the charge of one bunch is

$$
Q = \frac{I}{f} = \frac{1.841\times10^{-4}\ \text{C/s}}{1.625\times10^{8}\ \text{/s}} = 1.133\times10^{-12}\ \text{C} = 1.133\ \text{pC},
$$

and divided by the charge of one proton, $1.602\times10^{-19}$ C, that is about $7.07\times10^{6}$ protons. AVAS represents them by `particlenumber` **macroparticles**, 5260 in the example, so each macroparticle stands for about 1344 protons.

### Why the physics does not change

A macroparticle carries 1344 times the charge of a proton and also 1344 times its mass. In an applied electric field it feels a 1344 times larger force, but it is also 1344 times heavier; by Newton's second law $a = F/m$ its acceleration is exactly that of one proton. The same holds in a magnetic field. So **a macroparticle follows exactly the same path in the external fields as a real proton**: the path depends only on the ratio of charge to mass ($q/m$), and that ratio has not changed.

Only the push between the particles (space charge) is computed from the total charge: every macroparticle carries the charge $Q/N$ ($N$ is the number of macroparticles), and together they carry exactly the charge of the whole bunch.

Space charge is not computed pair by pair either. With the algorithm chosen by `scmethod` in `input.txt` (FFT, PICNIC, SPICNIC) the engine first works out from the macroparticle positions how the charge is distributed in space, then the electric field of this charge (the FFT method, for instance, solves Poisson's equation on a grid), and applies it to every macroparticle. It is like first counting how many people live in each square of a map and then working out how the squares affect each other: the work grows with the number of particles and of squares, not with the square of the number of particles.

### The price: graininess and statistical errors

The survey analogy also points to the price. A charge distribution drawn by a few thousand macroparticles is much "grainier" than one made of millions of real protons: the charge sits on fewer, heavier points, the computed self-field fluctuates more than that of the real beam, and the beam becomes spuriously "messier" (emittance growth from numerical noise). The statistics (rms size, emittance and so on) also carry a random error proportional to $1/\sqrt N$: four times as many macroparticles only halve the error. How to choose the number of macroparticles and how large the errors are is discussed in [Initial distributions](#distributions-statistics) and [Multi-particle simulation](#simulation); the estimates are worked out in this chapter's deeper section [Why macroparticles can be trusted and where the errors come from](#intro-deep-macroparticles).

## t-code and z-code {#intro-tracking}

AVAS elements come in two models that are computed differently. Think of them as two ways of photographing a bunch:

- the **t-code** is like a snapshot of the whole bunch at one moment, pushed forward in small time steps;
- the **z-code** is like a photo finish at the finish line: each particle is recorded as it crosses the same plane, and its position at the next plane is computed in one step.

(t stands for time, z for the position along the beam line.)

### Field-model elements (t-code)

For `field` elements (and drifts `drift`) the user supplies the three-dimensional distribution of the electromagnetic field (field map files). The engine uses **time $t$ as the independent variable** and in every small step pushes each macroparticle with the force on a charged particle:

$$
\frac{\dd \boldsymbol p}{\dd t} = q\left(\boldsymbol E + \boldsymbol v \times \boldsymbol B\right) + \boldsymbol F_{\mathrm{sc}}
$$

How to read it: the change of the momentum $\boldsymbol p$ per unit time equals the force. The force has three parts: the electric force $q\boldsymbol E$, the magnetic (Lorentz) force $q\boldsymbol v \times \boldsymbol B$ and the space-charge force $\boldsymbol F_{\mathrm{sc}}$. It is the school rule "impulse equals change of momentum", applied in every small step.

Why must an RF cavity be computed this way? The field in a cavity oscillates in time, and the field a particle sees depends on **when** it gets where: particles at the head and at the tail of the bunch see different electric fields at the same position, and a low-energy particle still changes its speed while crossing the cavity. With time as the independent variable all of this is included automatically, as are the non-linearity of the field and its fringe fields.

The time step is set by `steppercycle` in `input.txt`: every RF period is divided into $V_1$ steps, $\Delta t = 1/(f V_1)$. The example's `steppercycle 50` gives $\Delta t$ = 0.123 ns; every step advances about 2.1 mm (1/50 of the 105 mm flown in one RF period), and crossing the 0.21 m HWR cavity takes about 100 steps.

### Matrix-model elements (z-code)

`quad`, `solenoid`, `bend`, `edge` and `steerer` need only a few parameters (length, gradient, field and so on). The engine uses **the longitudinal position $z$ as the independent variable** and transforms the particle coordinates with the transfer matrix of a "hard-edge model"; this is called the z-code. **Hard edge** means that the field appears suddenly at the entrance of the element, vanishes suddenly at its exit, and is the same everywhere in between.

Why can an element be crossed in one step? A static magnetic field does not change in time, so it does not matter when a particle arrives: knowing its position and angle at the entrance plane is enough to write down its position and angle at the exit plane directly (the [Quadrupole](#quadrupole) chapter derives this "conversion table"). It is fast and can be compared directly with pen-and-paper theory (see [Linear optics](#linear-optics)), but it contains no continuously varying fringe field (`edge` only adds a thin-lens correction for the pole-face edge); whether the engine's matrix elements include higher-order terms is not stated in the manual, and the engine is the reference.

The two models can be mixed in one lattice; where they meet, the particle distribution has to be converted between the "same time" and the "same longitudinal position" descriptions (how is not stated in the manual, and the engine is the reference; the physics of the conversion and how much it matters are shown in [How the two snapshots are converted](#intro-deep-tracking)).

The beam line below puts both models together: the solenoid field map and the HWR cavity field map use the t-code, the quadrupole doublet after them uses the z-code, and both ends are drifts. The picture shows the linear preview of the example beam; the exit energy is about 1.88 MeV. Click "3D" to look from any angle, click an element to see its details.

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

## The linear preview: an estimate in seconds {#intro-preview}

One engine run of the example project takes about 10–30 seconds; long beam lines, many particles and error studies take much longer. While tuning parameters we often only want to know "roughly how wide the beam is and whether the energy is right", just as you work out a rough total in your head before shopping instead of taking out a calculator every time.

For this the user interface offers a **linear envelope preview** ("linear preview" in the visual editor). It does not call the engine and does not follow individual particles; it follows only a few averages of the whole beam (rms size, angle and the correlation between them), converting them with each element's transfer matrix (the formula is $\Sigma \mapsto M\Sigma M\tp$, see [Phase space and emittance](#phase-space-sigma)), and field maps are approximated by thin slices. The principle is explained in [Linear optics](#linear-optics).

How accurate is it?

- The energy of the reference particle is off by about 0.01 % (compared with the engine's synchronous particle). Note that the energy in the engine's `DataSet.txt` is the average over all macroparticles: off-axis particles feel a different electric field, and in the example HWR cavity the average gain is about 5 % higher than the reference particle's, so the two cannot be compared directly.
- rms sizes usually differ by 1–2 %, by up to 13 % when multi-particle effects are strong.
- It sees only averages, so it cannot show the shape of the distribution, the halo or particle losses (see [Initial distributions](#distributions-real-beam)).

The labs and particle animations of this primer are computed on the back end with the same linear model and share its approximations. **The engine's multi-particle simulation has the final word.**

## Input files and workflow {#intro-workflow}

An AVAS project contains two folders, `InputFile/` and `OutputFile/`. The input is split into several files, as a recipe keeps "ingredients", "kitchen equipment" and "cooking steps" apart:

- `beam.txt`: **what beam comes in** (the ingredients);
- the lattice file: **what the machine looks like** (which equipment stands in the kitchen);
- field map files: the **field data** of the machine's elements (the equipment's detailed specifications);
- `input.txt`: **how to compute** (simulation type, step size, space-charge algorithm and so on; the cooking steps);
- `ini.ini`: project settings.

With this split, a new beam does not touch the lattice and a new algorithm does not touch the physics input. What each file contains is listed in [this chapter's reference part](#intro-in-avas).

A full run goes like this:

1. Edit the input on the "Beam", "Lattice" and "Simulation settings" pages of the user interface (or edit the text files directly).
2. Start the run on the "Run" page. A run **does not rewrite** the InputFile: the input files are first copied to `<output folder>/inputs/`, the engine uses the copies as its input and the original folder for the field maps. The output folder therefore always keeps the input that belongs to its results; that is also why all input files are read-only while a full run, an error study or a segmented run is in progress (including paused).
3. At the start of the run the engine rebuilds `OutputFile/DataSet.txt` and then appends to it line by line; the user interface reads the new lines every second and shows the beam envelope live.
4. After the run, make plots on the "Results" page, or replay the run on the "Run" page.

## Output: one line of bunch statistics per step {#intro-outputs}

The t-code advances in time steps, and every few steps the engine computes statistics over all macroparticles and writes them as one line of `DataSet.txt`: energy and position of the reference particle, rms sizes and maxima in each direction, the number of surviving macroparticles and more, 41 columns in all. How many time steps lie between two lines is not stated in the manual (look at the spacing of the $z$ column of your own run and compare it with the example's step of about 2.1 mm). The lines of `DataSet.txt` therefore belong to output steps in time, not to elements, and the position has to be computed from the columns (see [the reference part](#intro-in-avas)).

When you need the particles themselves, look at the `.dst` files: `inData.dst` is the initial distribution the simulation actually used, `outData_<z>.dst` the distribution at an output plane.

## How to read this primer {#intro-how-to-read}

The primer has five parts:

- **A proton's journey**: [From the ion source to the target](#journey-overview) and five more chapters follow a linear accelerator in order and explain why each section exists, what it is made of, and which of them AVAS can simulate.
- **Fundamentals**: this chapter, [Relativistic kinematics](#kinematics), [Phase space and emittance](#phase-space), [Initial distributions](#distributions), [Linear optics](#linear-optics), [Longitudinal motion](#longitudinal), [Space charge](#space-charge). They build the concepts and notation used later.
- **Components**: [Drift](#drift), [Quadrupole](#quadrupole), [Solenoid](#solenoid), [Dipole](#dipole), [Steerer](#steerer), [RF cavity](#rf-cavity), [Static field elements](#static-fields), [Field maps](#field-maps), [Superposed fields](#superpose).
- **Building an accelerator**: [Lattice design](#lattice-design), [Matching](#matching), [Errors and correction](#errors), [Multi-particle simulation](#simulation), [Tutorial](#tutorial), [References](#references).
- **Accelerator control**: [Orbit correction](#orbit-correction); more chapters will follow.

Every chapter is written in the same order:

1. **First, a look**: the question, an everyday analogy and an animation to build the picture, with no formulas or only school algebra.
2. **The principle**: the result derived with school physics (impulse, Lorentz force, energy conservation, linear equations), with the numbers of the example beam.
3. **Going deeper (optional)**: sections marked "Going deeper" are folded by default; they go step by step from school knowledge to the textbook form, checking every step with the example numbers, and the closing paragraph "For readers who know …" uses more compact mathematics. The main text does not depend on them; skipping them does not stop you reading on.
4. **In AVAS**: keywords, parameter tables and conventions for reference.

While reading, note:

- **Labs and particle animations** use the linear model and update live as you drag the sliders; they are meant to build intuition. Their accuracy is discussed in [The linear preview](#intro-preview).
- **Field pictures** distinguish data from schematics: slices of field map files are data; the fields of matrix elements are schematics drawn from the parameters, and the picture says so. Cross-sections are always seen from downstream looking upstream, with the beam coming out of the screen (⊙), so that force directions can be checked directly with $\boldsymbol F = q\boldsymbol v \times \boldsymbol B$.
- **"AVAS convention"** boxes mark conventions in which AVAS differs from textbooks or the manual and which have been checked with the engine (for example the units of the Twiss parameters, `displacepos` being read in mm, `multithreading` only accepting 1); all coordinate systems, the differences from TraceWin and the list of engine pitfalls are collected in [References: In AVAS](#references-in-avas), where experienced users can look things up quickly.
- Every chapter shows its review status. The current chapters are **AI-written, awaiting review**: formulas and numbers have been checked by calculation, but not yet read line by line by a physicist; please report problems to the maintainer.

## Think about it {#intro-think}

> **Think 1**: If a macroparticle follows the same path as a real particle, what can a single-particle run (`particlenumber 1`) tell us, and what not?
>
> Answer: it gives the path, energy and phase of one particle in the external fields (`displacepos` can put it off the reference orbit), which is useful for checking that the lattice and field maps are set up correctly. It gives no statistics at all: rms sizes and emittances need a swarm of particles, so the rms columns of a single-particle run may be NaN; there is no space charge either.

> **Think 2**: Should the same static quadrupole give the same result when integrated in time with the t-code and when crossed in one step with the z-code?
>
> Answer: physically the particle path is the same; the differences come from the model, not from the independent variable: the z-code uses a hard-edge matrix without fringe fields and higher-order terms, the t-code uses a field map that contains both. Besides, the two give distributions on different "snapshots" (same time or same plane); for rms quantities the difference is usually very small.

> **Think 3**: Will `DataSet.txt` be identical when the same input is run twice?
>
> Answer: that depends on the random seed. The example project has `randomseed 0`: the seed comes from the clock, the initial particles differ and the results scatter statistically, so compare two runs with a tolerance. With a non-zero seed two runs of the new kernel are identical byte for byte, however many threads it uses (see [Multi-particle simulation](#simulation)). Do not trust conclusions drawn from differences smaller than the statistical error.

> **Think 4**: The current of the example beam is raised from 0.184 mA to 0.368 mA with the same number of macroparticles. How many protons does each macroparticle stand for? Does its path in the external fields change?
>
> Answer: the bunch charge doubles, so each macroparticle stands for about 2688 protons. The charge-to-mass ratio has not changed, so its path in the external fields stays the same; what changes is space charge: the push between the particles doubles.

## Common misconceptions {#intro-misconceptions}

- **"One macroparticle is one proton."** One macroparticle stands for many real particles (about 1344 in the example) with the same charge-to-mass ratio, so its path in the external fields is the same; but the graininess of space charge and the statistical errors both come from the limited number of macroparticles.
- **"AVAS can simulate a whole accelerator."** The ion source, the low-energy transport line and the RFQ are not among AVAS's elements; the simulation starts from the distribution at their exit.
- **"The linear preview and the engine compute the same thing."** The preview only advances averages (second moments), without the shape of the distribution, losses or full space charge; it is an estimate, the engine gives the result.
- **"Matrix elements and field-map elements are just two ways of writing the same thing."** Matrix elements are hard-edge models without fringe fields; field-map elements integrate the real three-dimensional field in time. The same magnet written both ways can give different results.
- **"A run rewrites the InputFile."** It does not; the engine uses the copies in `<output folder>/inputs/`.
- **"Each line of `DataSet.txt` belongs to one element."** Each line is one statistics output at some output time, unrelated to element boundaries; the position has to be computed from the columns.
- **"The same input always gives the same result."** Only with a non-zero random seed (then the new kernel is identical byte for byte); `randomseed 0` samples new particles every run and the results scatter statistically, so compare with a tolerance.

## Going deeper: why small deviations allow a straight-line approximation {#intro-deep-linear}

The main text said that "small deviations allow a straight-line approximation". This section turns that sentence into numbers with school mathematics: how good the straight-line approximation is, and when it fails.

### Step 1: the small-angle approximation of the pendulum

At school, for the pendulum, you used an approximation: for a small swing angle $\theta$, $\sin\theta \approx \theta$ ($\theta$ in radians). Let us check it:

| Swing angle | $\theta$ (radians) | $\sin\theta$ | Relative error |
|---|---|---|---|
| 5° | 0.08727 | 0.08716 | 0.13 % |
| 10° | 0.17453 | 0.17365 | 0.51 % |
| 20° | 0.34907 | 0.34202 | 2.1 % |

Doubling the angle makes the error about four times larger: the error grows with the **square** of the angle. This is the common rule of every "straight-line approximation": replacing a curve by a straight line near a point gives an error that grows with the square of the distance from that point.

### Step 2: the electric field a particle sees in an RF cavity

The electric field in an RF cavity oscillates as $E_0\cos\varphi$, where $\varphi$ is the phase of the field when the particle arrives. The reference particle arrives at the phase $\varphi_s$ (the example lattice writes $\varphi_s = -33°$); another particle arriving slightly earlier or later has the phase $\varphi_s + \Delta\varphi$. With the school addition formula

$$
\cos(\varphi_s + \Delta\varphi) = \cos\varphi_s\cos\Delta\varphi - \sin\varphi_s\sin\Delta\varphi ,
$$

and the approximations $\sin\Delta\varphi \approx \Delta\varphi$ and $\cos\Delta\varphi \approx 1$ from step 1, we get

$$
\cos(\varphi_s + \Delta\varphi) \approx \cos\varphi_s - \sin\varphi_s\cdot\Delta\varphi .
$$

The right-hand side is a straight line in $\Delta\varphi$: the field the particle sees changes in proportion to its phase deviation. This is "linearisation".

### Step 3: checking it with the example beam

The rms phase width of the example bunch is 4.5° (rms length 1.3 mm, 105 mm flown in one RF period, $1.3/105 \times 360° \approx 4.5°$). $\cos(-33°) = 0.8387$, $-\sin(-33°) = 0.5446$:

| Phase deviation $\Delta\varphi$ | Exact $\cos(\varphi_s+\Delta\varphi)$ | Straight line | Difference (in units of $E_0$) |
|---|---|---|---|
| 4.5° (1 rms) | 0.8788 | 0.8814 | 0.003 |
| 13.5° (3 rms) | 0.9426 | 0.9670 | 0.024 |
| 30° | 0.9986 | 1.1238 | 0.125 |

For nearly all particles (within a few rms) the straight-line approximation is off by a few thousandths to a few hundredths of the field amplitude; this is why the linear preview usually gets rms sizes within 1–2 %. Once the bunch is stretched to tens of degrees, the error becomes as large as the effect itself, and the engine has to follow the particles one by one in the real field.

### Step 4: why a whole beam can also be computed "linearly"

In the straight-line approximation every element acts on a particle by "multiplying by numbers and adding", for example $x = x_0 + L\,x_0'$ in a drift. Averaged over a whole beam, the averages obey equally simple relations (the [Drift](#drift-deep-matrix) chapter works this out with the expansion of a square). The linear preview therefore only needs to follow three averages to know the rms size of the whole beam, without following every particle.

Turned round, what the linear preview cannot see is exactly what the straight-line approximation throws away: the curvature of the RF field when the bunch is too long, the non-linear field felt by halo particles near the aperture, the non-linear space-charge force of non-uniform distributions.

### For readers who know calculus

Expanding the equations of motion in a Taylor series about the reference orbit and keeping the first-order terms gives the linear transfer map $\boldsymbol x \mapsto M\boldsymbol x$ (plus constant terms from steerers and misalignments), and the second moments then transform as $\Sigma \mapsto M\Sigma M\tp$; second- and third-order terms give the aberrations (chromatic and geometric). In Hamiltonian terms this means expanding the Hamiltonian about the reference orbit to second order, which is why the linear map is symplectic. AVAS's linear preview is such a first-order map, plus linear space charge computed from the rms sizes.

## Going deeper: why macroparticles can be trusted and where the errors come from {#intro-deep-macroparticles}

### Step 1: how slow pair by pair is

$N$ particles form $N(N-1)/2$ pairs (the first particle pairs with the other $N-1$, and dividing by 2 removes the pairs counted twice). For the example bunch, $N = 7.07\times10^{6}$:

$$
\frac{N(N-1)}{2} \approx \frac{(7.07\times10^{6})^2}{2} = 2.5\times10^{13}\ \text{pairs}.
$$

Even at $10^9$ pairs per second one step takes $2.5\times10^{4}$ seconds, about 7 hours; a linac needs tens of thousands of steps, so the run would take years. With 5260 particles there are $1.4\times10^{7}$ pairs, more than a million times fewer.

### Step 2: same charge-to-mass ratio, same path

In an electric field $E$ a particle with charge $q$ and mass $m$ feels the force $F = qE$ and has the acceleration $a = F/m = (q/m)E$. A macroparticle with charge $1344q$ and mass $1344m$ has the acceleration $(1344q)E/(1344m) = (q/m)E$, the same as one proton. The magnetic force $qvB$ is also proportional to $q$, with the same conclusion. Starting from the same position and velocity, a macroparticle and a proton therefore follow the same path in the external fields.

### Step 3: space charge from the total charge

The space-charge force comes from the charge of the whole bunch. $N$ macroparticles carry $Q/N$ each, the total is still $Q$ = 1.133 pC, so the field produced by the bunch is unchanged; the force on each macroparticle is its charge times this field, and its acceleration again depends only on the charge-to-mass ratio. The average space-charge effect is therefore not changed by switching to macroparticles. Only the "graininess" changes, see step 5.

### Step 4: why the statistical error goes as 1/√N

Toss 100 coins: on average 50 land heads up, but every toss is off by a few; the typical fluctuation is $\sqrt{100 \times \tfrac12 \times \tfrac12} = 5$ coins, a relative fluctuation of 10 %. Toss 10 000 coins and the fluctuation is 50, only 1 % relative. A hundred times as many coins give a tenth of the relative fluctuation: **the relative fluctuation is proportional to $1/\sqrt N$**.

Estimating the rms size of a beam from $N$ particles works the same way. For a Gaussian distribution the relative fluctuation of the estimated rms size is about $1/\sqrt{2N}$: about 1.0 % for the example's 5260 macroparticles, about 1.6 % for 2000. This is the fluctuation of the initial sampling; after the non-linear forces of a long beam line the differences between samples can be amplified, which is why two runs must be compared with a tolerance.

### Step 5: numerical noise from graininess

The charge in a real bunch is distributed almost continuously; millions of protons are close to each other and the close "collisions" are very weak. With a few thousand macroparticles the same charge sits on a few much heavier points, and two macroparticles that happen to come close give each other a hard shove that does not exist in the real beam. Grid algorithms spread the charge over the grid cells and smooth out most of this close interaction, but the remaining fluctuations still make the beam slowly "messier", which shows up as spurious emittance growth. The more macroparticles and the better the grid, the smaller this noise; advice is given in [Multi-particle simulation](#simulation).

### For readers who know statistics and plasma physics

The standard deviation of the sample variance is $\sigma^2\sqrt{2/(N-1)}$, so the relative standard error of the rms size is about $1/\sqrt{2(N-1)}$; higher moments (kurtosis, halo fraction) converge more slowly. The macroparticle method is a particle-in-cell (PIC) method: it replaces two-body collisions by a mean field (the Vlasov–Poisson description). The discreteness of the macroparticles amounts to an artificially enhanced effective collision rate, causing numerical relaxation and emittance growth; the grid scale cuts off the close interactions, but too coarse a grid loses real space-charge structure.

## Going deeper: how the two snapshots are converted {#intro-deep-tracking}

### Step 1: how long one t-code step is

The RF frequency is $f = 162.5$ MHz, one period is $1/f = 6.154$ ns. `steppercycle 50` divides it into 50 steps:

$$
\Delta t = \frac{1}{f V_1} = \frac{1}{1.625\times10^{8} \times 50}\ \text{s} = 0.123\ \text{ns}.
$$

The example proton flies at $v = 1.708\times10^{7}$ m/s (5.7 % of the speed of light, see [Relativistic kinematics](#kinematics)), so one step covers $v\,\Delta t = 2.1$ mm. The 0.21 m HWR cavity needs $210/2.1 = 100$ steps.

### Step 2: same time and same plane

The t-code gives the distribution at one moment: particles at the head of the bunch have already passed a given plane, those at the tail have not yet reached it. The z-code needs the coordinates of every particle as it crosses the same plane.

Suppose a particle is ahead of the reference particle by $z$ at this moment and has the transverse angle $x'$. It crossed the plane where the reference particle now is a time $z/v$ earlier; in that time it moved forward by $z$ and sideways by $x'\,z$. Its transverse position at that plane is therefore $x - x'\,z$, not $x$.

### Step 3: how large the difference is

The example beam has an rms angle of 3.03 mrad and an rms bunch length of 1.316 mm, so $x'\,z$ is typically $3.03\times10^{-3} \times 1.316\ \text{mm} \approx 0.004$ mm. Compared with the rms size of 0.954 mm, the two add in squares: $\sqrt{0.954^2 + 0.004^2} - 0.954 \approx 8\times10^{-6}$ mm. Completely negligible. This is why the main text says that "for rms quantities the difference is usually very small".

Longitudinally, the position difference $z$ at the same time becomes the time difference $z/v$ for reaching the same plane: 1.316 mm corresponds to 0.077 ns, that is $0.077/6.154 \times 360° = 4.5°$ of RF phase, the same conversion as in [Phase space and emittance](#phase-space-longitudinal). For very long bunches with a large energy spread the $z$–$\delta$ correlation at "the same time" and at "the same plane" can differ noticeably; then pay attention to which one is being compared.

### Step 4: why matrix elements can be crossed in one step

A static magnetic field does not change in time. The path of a particle from the entrance plane to the exit plane depends only on its position, angle and momentum at the entrance plane, not on when it arrives. The relation "entrance coordinates → exit coordinates" can therefore be worked out once and written as a conversion table (the transfer matrix) that every particle simply looks up. The field of an RF cavity changes in time: the same entrance coordinates reached at different moments give different results, so this is not possible there.

### For readers who know analytical mechanics

Changing the independent variable from $t$ to $s$ means $\dd x/\dd s = (\dd x/\dd t)/(\dd s/\dd t)$; in Hamiltonian form, with $s$ as the independent variable $-p_s$ plays the role of the Hamiltonian, and the time $t$ (or the phase) becomes a coordinate conjugate to the energy. Time-dependent fields can still be handled in this form, but every particle has its own $t$ and the field has to be taken at that $t$, so there is no time-independent map as for static fields. The conversion of the distribution where the t-code and the z-code meet carries a slice at one $t$ along each particle's own path to one plane $s$ (or the other way round).

## In AVAS {#intro-in-avas}

### The engine's six tasks

1. **Reference particle**: starting from the kinetic energy in `beam.txt`, integrate the energy and time of the reference particle along the lattice and determine the entrance phase of every RF cavity (see [Longitudinal motion](#longitudinal)).
2. **Macroparticles**: generate `particlenumber` macroparticles from the Twiss parameters and distribution type in `beam.txt` (or read a particle distribution file, see [Initial distributions](#distributions)). Each macroparticle stands for many real particles; the total charge follows from the current `current` and the bunch frequency `frequency` (bunch charge $Q = I/f$).
3. **External fields**: field-map elements and drifts integrate the Lorentz force in time steps with the t-code; matrix elements use the z-code's transfer matrices (see [t-code and z-code](#intro-tracking)).
4. **Space charge**: with `spacecharge 1` in `input.txt`, the beam's self-field is computed with the algorithm chosen by `scmethod` (FFT, PICNIC, SPICNIC) and applied to every macroparticle (see [Space charge](#space-charge)).
5. **Errors and correction**: commands such as `err_step`, `err_beam_dyn`, `err_beam_stat`, `err_quad_*` and `err_cav_*` in the lattice add random errors to elements and to the initial beam; `adjust` and `diag_*` perform static error correction (see [Errors and correction](#errors)).
6. **Losses**: a particle outside the element aperture `R` or the longitudinal boundary counts as lost; the number of surviving macroparticles is written to the output as a function of $s$.

### Elements of a beam line

| Function | Typical elements | In AVAS | See |
|---|---|---|---|
| Transverse focusing | quadrupoles, solenoids | `quad`, `solenoid`, or static magnetic field maps `field … 3 …` | [Quadrupole](#quadrupole), [Solenoid](#solenoid) |
| Acceleration and longitudinal focusing | RF cavities (e.g. half-wave resonator HWR) | RF field maps `field … 1 …` | [RF cavity](#rf-cavity) |
| Bending and orbit correction | dipoles, steerers | `bend`, `edge`, `steerer` | [Dipole](#dipole), [Steerer](#steerer) |
| Diagnostics and output | diagnostic positions, output planes | `diag_energy`, `diag_size`, `diag_position`, `outputplane` | [Errors and correction](#errors), [Multi-particle simulation](#simulation) |

There are also drifts `drift` (field-free space, see [Drift](#drift)), static electric field maps `field … 2 …` (see [Static field elements](#static-fields)) and `superpose` blocks that let several field maps overlap in space (see [Superposed fields](#superpose)).

> **AVAS convention**: in multi-particle simulations the **first and last elements of the lattice must not be matrix-model elements** (the manual's requirement); if needed, add a very short `drift` at both ends.

### Input files

| File | Contents | See |
|---|---|---|
| `beam.txt` | particle species (`particlerestmass`, `numofcharge`), kinetic energy `kneticenergy`, current `current`, bunch frequency `frequency`, number of macroparticles, distribution type `distribution`, Twiss parameters `twissx`/`twissy`/`twissz` | [Kinematics](#kinematics-in-avas), [Phase space](#phase-space-in-avas), [Initial distributions](#distributions-in-avas) |
| `input.txt` | simulation type `sim_type`, step size `steppercycle`, space-charge switch and algorithm, grid, output settings and more | [Multi-particle simulation](#simulation) |
| lattice file | the element sequence: the lines between `start` and `end`, `!` starts a comment | [Lattice design](#lattice-design) |
| field map files | three-dimensional field distributions used by field-map elements, one file per component (e.g. `hwr010.edz`, `hwr010.edx`, `sol.bsz`) | [Field maps](#field-maps) |
| `ini.ini` | project settings; `[lattice] source` names the lattice file used for runs | — |

The example project `examples/hwr010` has a 1.527 MeV, 0.18 mA, 162.5 MHz proton beam, and its lattice holds a single 0.21 m HWR RF cavity field map:

```text
start
field  0.21   0.02     0   1   162.5e6   -33   1.36    -1.36   efield
end
```

The example `input.txt` has `sim_type mulp`, `spacecharge 1`, `steppercycle 50`, `scmethod FFT`.

### Linear preview

The linear preview does not call the engine: it advances the beam's second moments with 6×6 transfer matrices, $\Sigma \mapsto M\Sigma M\tp$, approximates field maps by thin slices and treats space charge linearly from the rms sizes. Its accuracy is discussed in [The linear preview](#intro-preview).

### Output files

Each line of `DataSet.txt` holds the bunch statistics at one time (or position), 41 columns:

| Column | Contents |
|---|---|
| 0 | kinetic energy of the reference particle (MeV) |
| 5 + 33 | longitudinal position $z$ (sum of the two columns) |
| 16, 18, 20 | rms sizes $x$, $y$, $z$ (m) |
| 22, 24, 26 | maxima $x$, $y$, $z$ (m) |
| 28 | number of surviving macroparticles |

Other outputs include:

- `inData.dst`: the initial particle distribution the simulation actually used;
- `outData_<z>.dst`: the particle distribution at an output plane (binary); at the end the engine always writes the distribution at the end of the lattice;
- `Phase.txt`: time, position and energy of the synchronous particle at the entrance and exit of every RF cavity (two lines per cavity);
- `synData.txt`: time, $\gamma\beta$, synchronous phase $\varphi_s$ and RF phase $\varphi_{\mathrm{RF}}$ of the synchronous particle at the entrance of every element;
- error study results go to `OutputFile/error_output/`.

> Note: in a single-particle run (`particlenumber 1`) the reference-particle data are written as usual; the rms columns are NaN with the original kernel and the new kernel's compatibility mode and 0 in the new kernel's default mode, both normal. With `randomseed 0` every run samples new particles, so compare two `DataSet.txt` files with a tolerance.

## Further reading {#intro-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: the standard textbook of linac physics, covering RF, longitudinal and transverse dynamics and space charge.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: a systematic treatment of beam optics, emittance and space charge.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific.
- The lecture notes of the CERN Accelerator School (CAS), especially "Introduction to Accelerator Physics" and "RF for Accelerators".
- The AVAS manual `docs/使用说明20260427.docx` ("Parameter and file reference" under "Help → Manual" in the user interface), and this primer's [References](#references) chapter.
