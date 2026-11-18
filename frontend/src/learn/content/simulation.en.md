# Multi-particle simulation essentials

A bunch of the example project contains about 7 million protons; AVAS represents them by only 5260 "macro-particles" and advances them in steps of 0.12 ns. Can results computed this way be trusted? Why do two runs of the same input not give exactly the same result? How large must a difference be to be real? This chapter starts from these questions and explains how the AVAS engine "computes" a beam, where the errors come from and how to judge whether a result can be trusted.

First, what the engine does, in a nutshell. It splits the beam into a few thousand representatives and then does the same thing over and over:

1. **Count**: see where all representatives are at this moment and work out the charge distribution;
2. **Work out the forces**: the force on each representative = the elements' electromagnetic fields + the field of the charges of all other particles (space charge);
3. **Take a small step**: push each representative forward by a small interval of time according to its force;
4. **Write it down**: collect this step's mean energy, beam size, emittance and number of surviving particles in a record file; at chosen positions, take a "group photo" of the coordinates of all representatives.

Going on like this to the end of the beam line is one multi-particle simulation. Each section below covers one part of it: the number of representatives (macro-particles), how long each step is (the time step), how the push between particles is computed (the space-charge mesh), what is recorded (output), how to judge whether results can be trusted (convergence), and what the linear preview and the engine can each tell you. At the end come AVAS's run settings, output files, single-particle runs and common reasons for failure.

For the physics see [space charge](#space-charge) and [longitudinal dynamics](#longitudinal), for initial distributions [initial distributions](#distributions), and for the overall structure of the input files [accelerators and AVAS](#intro).

## Macro-particles and statistics {#simulation-macroparticles}

### What a macro-particle represents

The real number of particles in a bunch is the bunch charge divided by the charge of one particle. The bunch charge is $Q = I/f$: the average current $I$ divided by the number of bunches per second $f$ (the RF frequency). The example project has $I = 0.184\ \mathrm{mA}$ and $f = 162.5\ \mathrm{MHz}$, so $Q = 1.13\ \mathrm{pC}$, which is $7.07\times 10^6$ protons. Real machines usually have $10^6$ to $10^{10}$; computing the forces between every pair of them is more than a computer can manage.

The engine instead represents them by `particlenumber` **macro-particles**. Each macro-particle carries the charge $Q/N$, and its mass is scaled up in the same proportion, so its charge-to-mass ratio is that of a real particle. In applied electromagnetic fields, how a particle moves depends only on the charge-to-mass ratio (the force is proportional to the charge, acceleration = force ÷ mass), so every macro-particle follows exactly the trajectory of one real particle; only the push between particles (space charge) is computed from the total charge. The example project uses 5260 macro-particles, each standing for about 1340 protons.

Think of an **opinion poll**: a few thousand people stand for millions; as long as the sample is representative, the average is right, but every statistic carries a "sampling error".

### How large the statistical error is

The sampling error follows a simple rule: **as the number of samples $N$ grows, the error shrinks as $1/\sqrt N$**. To make the error 10 times smaller, the number of macro-particles must grow 100 times. For the Gaussian distribution used by the example, the relative statistical error of the rms size is $1/\sqrt{2N}$:

| Macro-particles $N$ | Relative statistical error of the rms size |
|---|---|
| 5260 (example) | 0.97 % |
| $10^4$ | 0.71 % |
| $10^5$ | 0.22 % |
| $10^6$ | 0.07 % |

The statistical error of the rms emittance is of the same order, about $1/\sqrt N$ for a Gaussian (1.4 % in the example). The smallest step of the **loss rate** is $1/N$: in the example, losing one macro-particle is 0.019 %. Studying losses below $10^{-4}$, or the sparse "halo" around the beam, requires far more macro-particles or a dedicated study of the distribution tails. Where the formula comes from is in "Going deeper: why the statistical error is 1/√N".

See the sampling error for yourself in the lab below. It draws N particles with the parameters of the example beam (the same Twiss parameters) and computes the rms size from those particles. The theoretical value is $\sqrt{\beta\varepsilon} = 0.954\ \mathrm{mm}$. The number of particles is set to 300, and the lab's "rms x" is 0.914 mm, 4 % below the theory; drag N to 5260 and it becomes 0.949 mm (0.5 % off); at 20000 it is 0.956 mm. Now look at "Gaussian (GS): share beyond 2 rms": the theory says 4.55 %; with 300 particles the count gives only 3.3 %, with 20000 it is 4.4 %. The tails of a distribution hold very few particles and are counted even less accurately.

```avas-lab
distribution N=300
```

### Numerical noise

Few macro-particles have another consequence. With far fewer macro-particles than real particles, each macro-particle has a far larger charge, so the push between them is "grainier" than in a real beam, and particles receive small random pushes that cause spurious emittance growth. This is called **numerical heating** (also numerical noise). It decreases as the number of macro-particles per mesh cell increases; see [the space-charge mesh](#simulation-sc-mesh).

> **Note**: whether the same input gives the same result twice depends on the random seed, not on multithreading. The example project has `randomseed 0`: the seed is taken from the current time (in seconds), the initial particles differ every run and the results scatter statistically (two runs of the example end with rms x 1.800 and 1.828 mm, about 1.6 % apart). With a non-zero seed the new kernel's output is identical byte for byte, whatever the number of threads and the space-charge method. To compare two runs (for example to judge the effect of a parameter), either fix the seed or use the statistical error as the tolerance; conclusions from differences below the statistical error are unreliable.

## Initial distributions {#simulation-distributions}

Before the simulation starts, the engine must first "set up" its few thousand representatives: the position, angle and energy deviation of each macro-particle. It can generate four distributions from Twiss parameters with `twissx`, `twissy`, `twissz` and `distribution` — KV, GS (Gaussian), PB (parabolic) and WB (waterbag) — or read an existing particle file with `readparticledistribution` (for example the distribution at the RFQ exit). For the same rms size, different distributions have very different edges and tails, and with strong space charge they also decide the emittance growth at the start; when comparing results of different codes, make sure the distributions agree. How these distributions are defined, how they differ, and how to check which distribution a simulation actually used is covered in the [initial distributions](#distributions) chapter; the keywords are listed in [In AVAS](#simulation-in-avas).

## Time step {#simulation-stepping}

### Why the step must be small

The engine cannot solve the motion continuously; it advances step by step: each step computes the force from the current fields, assumes the force stays constant for that short time, and pushes the particle a little further. It is like tracing a curve with short straight segments: the shorter the segments, the more accurate the tracing; the more sharply the curve bends (the more the field changes within one step), the larger the error. It is also like recording a runner with burst photos: the denser the shots, the better you see the motion.

Field-map elements and drifts are advanced in time by the **t-code** (see [accelerators and AVAS](#intro-tracking)). `steppercycle V1` in `input.txt` divides one RF period into $V_1$ steps, so the time of each step and the distance a particle travels per step are

$$
\Delta t = \frac{1}{f\,V_1},\qquad \Delta s \approx \beta c\,\Delta t = \frac{\beta\lambda}{V_1} .
$$

Here $\lambda = c/f$ is the RF wavelength (1.845 m at 162.5 MHz) and $\beta = v/c$ the particle's speed relative to the speed of light; $\beta\lambda$ is the distance the particle travels in one RF period. Example: `steppercycle 50`, 162.5 MHz, $\beta = 0.057$: $\Delta t = 1/(162.5\times 10^6 \times 50) = 0.123\ \mathrm{ns}$, and each step advances $0.057 \times 1.845\ \mathrm{m}/50 \approx 2.1\ \mathrm{mm}$; the `hwr010` field map has a grid spacing along $z$ of 2 mm, comparable. Crossing the 0.21 m long HWR cavity takes about 100 steps, one 0.76 m period of the [hands-on tutorial](#tutorial) about 360.

Note that $\Delta s$ is proportional to β: with the same `steppercycle`, faster particles travel farther per step. When the beam of step 4 of the tutorial reaches 4.1 MeV, $\beta \approx 0.094$ and each step covers 3.5 mm.

Rules for choosing the step:

- the distance per step must be much smaller than the scale on which the fields change (field-map grid, gap length, $\beta\lambda$);
- much smaller than the distance of one transverse oscillation, one longitudinal (synchrotron) oscillation and the oscillation caused by space charge (the space-charge push is integrated with the same step; how often the engine recomputes the space-charge field is not described in the manual);
- check by **doubling the number of steps**: when the result no longer changes (within the statistical error), the step is small enough.

### z-code elements

`quad`, `solenoid`, `bend`, `edge` and `steerer` are matrix elements: the **z-code** transforms the particle coordinates by longitudinal position in one go instead of advancing in time steps. The t-code records the bunch at the time of the synchronous particle, the z-code at a position (see the manual's description of `Beamset.plt`); how the transition between the two kinds of element works is not described in the manual, the engine is authoritative. The schema says of matrix elements: in a multi-particle lattice, **avoid matrix elements as the first and the last element**; put a short drift at each end.

## The space-charge mesh {#simulation-sc-mesh}

### What the mesh does

Particles of the same charge repel each other. Computing the Coulomb force between every pair of $N$ macro-particles takes about $N^2/2$ pairs: for 5260 macro-particles that is 14 million pairs per step, for $10^6$ macro-particles $5\times 10^{11}$ — too many.

The **particle–mesh method** works differently: divide space into a three-dimensional grid of cells (the mesh), share the charge of all macro-particles among the cells to get the charge density, solve for the electric field on the mesh, and finally hand the field back to each particle. It is a bit like estimating the density of a crowd on a square: divide the square into cells and count the people in each, instead of measuring the distance between every two people. Cells too large hide the shape of the crowd; cells too small hold only a person or two each, and the counted density is nothing but chance fluctuation.

With `spacecharge 1` the engine solves the beam's own field (self-field) on the mesh. `numofgrid Nx Ny Nz` is the number of cells in the three directions, `meshrms V1 V2 V3` sets the total mesh length $L_x = 2V_1\sigma_x$ ($\sigma_x$ being the beam's rms size; likewise for $y$, $z$), so each cell is

$$
h_x = \frac{2V_1\sigma_x}{N_x}
$$

long. For example, with $V_1 = 4$ and $N_x = 32$ the mesh covers $\pm 4\sigma_x$ and each cell is $h_x = 8\sigma_x/32 = 0.25\,\sigma_x$ long.

### Three competing requirements

- **Coverage**: the mesh must cover nearly all particles. Too small a $V_1$ cuts off the tails; for a Gaussian, take $V_1$ at least 3–5. How particles outside the mesh are treated is not described in the manual.
- **Resolution**: the beam core needs enough cells, $h_x$ clearly smaller than $\sigma_x$; the larger $V_1$, the coarser the cells for the same $N_x$.
- **Particles per cell**: the average number of macro-particles per cell is about $N/(N_xN_yN_z)$, more in the beam core. For example, $32\times 32\times 32 = 32768$ cells with 5260 macro-particles have only 0.16 per cell on average, and the computed field is all fluctuation; such a mesh needs $10^5$–$10^6$ macro-particles.

The conflict: higher resolution needs more cells, and more cells need more macro-particles. So **the number of macro-particles and the mesh must be chosen together**. After changing the mesh or the method, compare results as in [judging convergence](#simulation-convergence). The method step by step is in "Going deeper: the particle–mesh method step by step".

## What a simulation gives you: records and output planes {#simulation-output}

A multi-particle simulation leaves two kinds of things behind, which can be compared to a "health record" and a "group photo":

- **Health record**: `DataSet.txt`. Each time the engine records a step it writes one line of statistics: mean energy, beam centre, rms and maximum sizes in the three directions, Twiss parameters and emittances, and the number of remaining macro-particles. The envelope, energy and emittance curves along the beam line all come from it. It holds only statistics; single particles cannot be seen.
- **Group photo**: the particle distribution at an output plane, `outData_<position>.dst`. Put an `outputplane` at a position in the lattice, and when the beam passes there the engine saves the coordinates of all macro-particles; afterwards you can plot phase spaces and distributions or use it as the initial distribution of the next simulation. `automaticoutput` places many of them at equal spacing in one go. At the end the engine always takes a photo at the end of the beam line; no extra plane is needed.

Output planes have one common pitfall: a plane **beyond the end of the lattice** is an error, and one **within 5 mm of the end** also fails, while 2 cm from the end works. So usually a short drift is left at the end of the line.

There are also `Phase.txt` with the entrance and exit of every RF cavity, `synParticle.txt` with the synchronous particle along the line, and, on request, `Beamset.plt` with all particle data (the file can be very large). The complete list and the meaning of every column of DataSet.txt are in [In AVAS](#simulation-in-avas).

## Judging convergence {#simulation-convergence}

For a multi-particle result to be trusted, it must at least be shown to be insensitive to the "numerical parameters": when the number of macro-particles, the step and the mesh — parameters that affect only the accuracy of the computation — are made finer, the result stops changing within the statistical error. This is called **convergence**. A suggested procedure:

1. First tune the lattice with the linear preview and few macro-particles ($10^3$–$10^4$); check that energy, phase and envelope are roughly reasonable.
2. **Macro-particles**: at the design parameters, increase `particlenumber` four times or more (e.g. $10^4 \to 10^5$) and compare the exit rms size, the rms emittance growth and the loss rate. The statistical error drops as $1/\sqrt N$; if the emittance growth decreases clearly as $N$ grows, the original result was contaminated by numerical noise.
3. **Step**: double `steppercycle` (e.g. 50 → 100); the result should stay the same within the statistical error.
4. **Mesh**: change `numofgrid` and `meshrms` together while keeping enough macro-particles per cell; if needed, compare different `scmethod`s.
5. **Repeated runs**: repeat the same input two or three times to estimate the random run-to-run difference, the tolerance for judging "changed or not".

The criterion depends on the question: matching and envelope design care about rms sizes (of order 1 %); emittance growth about the relative growth; beam loss and halo need high statistics and look at maximum sizes and distribution tails. When reporting results, give the number of macro-particles, the step and the mesh used.

## Linear preview and engine {#simulation-preview}

The **linear envelope preview** of the visual editor (`avas.sim.linear_optics`) does not track individual particles; it directly advances a few averages that describe the beam's shape (the beam's "second moments", arranged as a 6×6 σ matrix): the reference particle integrates energy and time along the field maps, field maps are sliced at 4 mm to compute linear focusing and bunching, and space charge is computed as the linear push of an "equivalent uniform ellipsoid" (see [space charge](#space-charge)). One evaluation advances a single 6×6 matrix, so it responds almost immediately while you drag parameters, which suits matching and first designs. The price is that it only knows how "wide" the beam is and cannot see changes of the distribution's shape.

| Item | Linear preview | Multi-particle engine |
|---|---|---|
| Energy | reference particle within about 0.01 % (against the engine's synchronous particle); the engine's DataSet is the mean energy of the macro-particles, affected by off-axis fields, about 5 % difference in gain for the example cavity | reference |
| rms size | usually 1–2 % off, up to 13 % with strong multi-particle effects | reference |
| Nonlinearity, emittance growth, halo | not included | included |
| Losses | only warns when 3 × rms exceeds the aperture | decided particle by particle |
| Dipole / steering components of field maps | no deflection (with a warning) | included |

> **Note**: the preview does not include longitudinal nonlinearity in low-energy cavities, chromatic effects or the offsets of `superposeout`. Design conclusions follow the engine; a large difference between preview and engine usually means strong nonlinear or space-charge effects — exactly when a multi-particle simulation is needed.

## Think about it {#simulation-think}

> **Think about it 1**: with 5260 macro-particles, the exit rms size differs by 1 % before and after changing a parameter. Can you conclude that the parameter matters?
>
> Answer: not for sure. If the two runs sample their initial distributions independently, each result has a statistical error of about 0.97 % and their difference a standard deviation of about $\sqrt2 \times 0.97\ \% = 1.4\ \%$, which covers 1 %. If both runs start from the same sample (the same non-zero `randomseed`: the engine then generates exactly the same initial particles, see [In AVAS](#simulation-in-avas)), the sampling error is shared and mostly cancels in the difference: the new kernel has no other random differences, so the 1 % is the effect of the change on this one sample; the original kernel adds run-to-run differences from multithreading where matrix elements such as quadrupoles meet space charge. A conclusion that holds for one sample need not hold for another, so repeat with a few seeds, or use more macro-particles, before concluding.

> **Think about it 2**: none of the 5260 macro-particles was lost. Is the loss rate zero?
>
> Answer: no. One macro-particle is 0.019 %; zero losses only say that the loss rate is below that order.

> **Think about it 3**: the particles are accelerated from 1.5 MeV to 4 MeV with the same `steppercycle`. How does the distance per step change?
>
> Answer: the time per step stays the same (1/50 of an RF period), but the particles are faster and travel farther per step: $\beta$ grows from 0.057 to about 0.094, and the step from 2.1 mm to about 3.5 mm.

> **Think about it 4**: refining the mesh from $16^3$ to $32^3$ with the same number of macro-particles — does the result get more accurate?
>
> Answer: not necessarily. Eight times as many cells mean one eighth of the macro-particles per cell, larger field fluctuations and possibly worse numerical heating. Resolution and particles per cell must be considered together.

## Common misconceptions {#simulation-misconceptions}

- **More macro-particles are better, but only together with the mesh**: adding macro-particles with the same mesh does not improve resolution; adding cells with the same macro-particles increases noise.
- **Differences below the statistical error mean nothing**: with 5260 macro-particles the rms size error is about 1 %; smaller changes are not conclusions.
- **Zero losses are not no losses**: none of 5260 macro-particles lost only says the loss rate is below about $1/5260$.
- **`steppercycle` is steps per RF period**, not a fixed spatial step: after acceleration each step covers more distance.
- **`multithreading 0` does not switch multithreading off**: it makes the engine lose all particles at the start; leaving the line out is not single-threaded either, both kernels default to `multithreading 1` (see below).
- **NaN in single-particle runs**: with `particlenumber 1` the rms columns of DataSet are NaN; that is normal.

## Going deeper: why the statistical error is 1/√N {#simulation-deep-statistics}

The main text said "the relative statistical error of the rms size is $1/\sqrt{2N}$". This section derives this formula step by step, starting from a coin toss: the first four steps use only school algebra, the last paragraph is for readers who know probability.

### Step one: the error of a mean is σ/√N

Toss a coin, counting heads as $+1$ and tails as $-1$. Toss $N$ times and average: the mean should be close to 0, but not exactly 0. How far off? Adding $N$ independent numbers of zero mean, the average of "the square of the sum" is the sum of the averages of the squares (the cross terms are positive and negative and average to zero); here each square is 1, so the mean square of the sum is $N$, the typical size of the sum $\sqrt N$, and the typical size of the mean $\sqrt N/N = 1/\sqrt N$.

In general: if a single sample has the spread (standard deviation) $\sigma$, the mean of $N$ samples spreads by $\sigma/\sqrt N$. After 100 tosses the mean is usually within $\pm 0.1$.

### Step two: the rms size is also a mean

The square of the rms size, $\sigma_x^2$, is the mean of $x^2$ over the particles (taking the beam centre as 0). Estimating $\sigma_x^2$ from $N$ macro-particles therefore means averaging $N$ numbers $x^2$. By step one, its error is "the spread of $x^2$ itself ÷ $\sqrt N$".

How much does $x^2$ itself spread? Its variance is $\langle x^4\rangle - \langle x^2\rangle^2$ ("mean of the square minus square of the mean"). For a Gaussian, $\langle x^4\rangle = 3\sigma^4$ (check it with a normal-distribution table or with the integral under "For readers who know probability" below). So the variance of $x^2$ is $3\sigma^4 - \sigma^4 = 2\sigma^4$ and its standard deviation $\sqrt2\,\sigma^2$. Hence

$$
\frac{\delta(\sigma^2)}{\sigma^2} = \frac{\sqrt2\,\sigma^2/\sqrt N}{\sigma^2} = \sqrt{\frac{2}{N}} .
$$

### Step three: from the error of σ² to the error of σ

$\sigma$ is the square root of $\sigma^2$. The school approximation $\sqrt{1 + \epsilon} \approx 1 + \epsilon/2$ (for small $\epsilon$; for example $\sqrt{1.02} = 1.00995$) tells us: a 2 % difference in the square is only 1 % after the square root. So

$$
\frac{\delta\sigma}{\sigma} \approx \frac{1}{2}\sqrt{\frac{2}{N}} = \frac{1}{\sqrt{2N}} .
$$

With the example: $N = 5260$, $1/\sqrt{10520} = 0.0097$, i.e. 0.97 %. For the 300 particles of the lab in the main text $1/\sqrt{600} = 4.1\ \%$, and the 4 % deviation the lab showed is of just this order (the lab's rms x is computed from KV particles, whose error formula, in the next step, gives about 2.9 %; a single sample 1.4 standard deviations off is quite common).

### Step four: other distributions, emittance and loss rate

The "3" used in step two belongs to the Gaussian. In general $\langle x^4\rangle = \kappa\,\sigma^4$, where $\kappa$, the **kurtosis**, measures how "peaked and long-tailed" a distribution is. The variance of $x^2$ is then $(\kappa - 1)\sigma^4$, and

$$
\frac{\delta\sigma}{\sigma} \approx \sqrt{\frac{\kappa - 1}{4N}} .
$$

A Gaussian has $\kappa = 3$, giving back $1/\sqrt{2N}$. Distributions with steep edges are estimated better: a uniform projection has $\kappa = 1.8$ and an error of $0.45/\sqrt N$ instead of $0.71/\sqrt N$ (the projection of a KV distribution on $x$ has $\kappa = 2$ and an error of $0.5/\sqrt N$). Heavy-tailed distributions are estimated worse.

The rms emittance combines the three means $\langle x^2\rangle$, $\langle x'^2\rangle$ and $\langle xx'\rangle$, and its error is of the same order, about $1/\sqrt N$ for a Gaussian (1.4 % in the example). The loss rate is "number lost ÷ $N$", with the smallest step $1/N$. The error of the difference of two independent runs is $\sqrt2$ times that of one run (variances add).

### For readers who know probability

The variance of the sample variance $s^2$ is $(\mu_4 - \sigma^4)/N$ (for large $N$); the delta method $\delta\sigma \approx \delta(\sigma^2)/(2\sigma)$ then gives $\sqrt{(\kappa-1)/(4N)}$. For a Gaussian $\mu_4 = \int x^4\,e^{-x^2/2\sigma^2}\dd x/\sqrt{2\pi}\sigma = 3\sigma^4$. The estimation error of the two-dimensional rms emittance $\varepsilon^2 = \langle x^2\rangle\langle x'^2\rangle - \langle xx'\rangle^2$ can be expanded the same way; the result depends on the correlation coefficient and is $\delta\varepsilon/\varepsilon \approx 1/\sqrt N$ for an uncorrelated Gaussian. All this assumes independently sampled macro-particles; distributions generated with quasi-random sequences or symmetrization can have smaller errors. How the AVAS engine generates them: see [initial distributions](#distributions) (`inData.dst` is authoritative).

## Going deeper: the particle–mesh method step by step {#simulation-deep-pic}

The main text explained the mesh by "counting people on a square". This section goes step by step through how the particle–mesh method computes the space-charge force, and why too few particles per cell produce noise. What is described here is the general approach of this family of methods; for which deposition and solver the AVAS engine uses, the manual only lists the names of `scmethod`.

### Step one: why not add up all pairs

$N$ particles form $N(N-1)/2$ pairs. 5260 macro-particles make $1.4\times 10^7$ pairs; crossing one HWR cavity takes about 100 steps, i.e. $1.4\times 10^9$ Coulomb-force evaluations. With $10^6$ macro-particles each step needs $5\times 10^{11}$ pairs. The work grows as $N^2$, and slightly larger numbers of macro-particles become impossible.

### Step two: share the charge among the cells

The simplest way is "head counting": each macro-particle gives all its charge to the nearest grid point. A smoother way shares by distance: a particle between two grid points, 30 % of the spacing from the left point, gives 70 % of its charge to the left point and 30 % to the right (in three dimensions it is shared this way among the 8 surrounding grid points). This step takes work proportional to $N$ only.

### Step three: from the charge to the electric field

With the charge in each cell, the electric field must be found. High-school physics uses Gauss's law for the simplest case: an infinitely long, uniformly charged cylinder of radius $a$ with charge $\lambda$ per metre; inside it, at distance $r$ from the axis, the field is

$$
E = \frac{\lambda\,r}{2\pi\varepsilon_0 a^2}\quad (r < a),
$$

proportional to $r$: the farther from the axis, the harder the push, and linearly so. That is where the "uniform ellipsoid" space charge of the linear preview comes from. A real beam's charge is not uniform, so its field is no longer linear; the mesh method solves for the field numerically at every grid point without assuming the shape of the distribution.

### Step four: hand the field back to the particles and step

Each macro-particle takes the field back from the surrounding grid points with the same weights as in step two, adds the elements' external field to get its force, and is advanced by one time step. Then back to step two, and repeat.

### Step five: why too few particles per cell make noise

The number of macro-particles in a cell is like a coin toss too: if a cell should hold $n$ on average, the actual count fluctuates randomly around $n$ by about $\sqrt n$, a relative fluctuation of $1/\sqrt n$.

- 5260 macro-particles on $32^3 = 32768$ cells: on average $n = 0.16$; most cells are empty, a few hold one or two, and the counted "density" is almost pure chance.
- $10^6$ macro-particles on the same mesh: on average $n \approx 30$, a relative fluctuation of about 18 %; cells in the beam core hold more particles and fluctuate less.

Random fluctuations of the density become random fluctuations of the field, and particles get a random push at every step: that is the **numerical heating** of the main text. A real beam has far more particles and these fluctuations are negligible, so the heating is an artefact of the simulation.

### Step six: the size of the cells

The cell length is $h_x = 2V_1\sigma_x/N_x$. With $V_1 = 4$ and $N_x = 32$, $h_x = 0.25\sigma_x$: the beam core ($\pm\sigma_x$) spans 8 cells and the shape of the distribution can be resolved; the mesh covers $\pm 4\sigma_x$, and for a Gaussian only about six in a hundred thousand particles lie beyond $4\sigma$ in one direction. With $V_1 = 8$ and the same $N_x$ the cells are $0.5\sigma_x$ long and the core spans only 4 cells.

### For readers who know electrodynamics

On the mesh one solves Poisson's equation $\nabla^2\phi = -\rho/\varepsilon_0$. A common approach is to compute the electrostatic field in the beam's rest frame and transform back to the laboratory frame: the transverse electric and magnetic forces partly cancel, and the net transverse force is $1/\gamma^2$ of the electric force alone. Convolving the charge density with the Green's function by FFT takes work of about $M\log M$ ($M$ being the number of grid points), far less than the $N^2$ of direct summation. Numerical heating weakens as the number of macro-particles per cell (more precisely, per Debye length scale) increases, so the number of macro-particles and the mesh must converge together. AVAS's `scmethod` can be `FFT`, `PICNIC` or `SPICNIC`; the manual only lists the names.

## In AVAS {#simulation-in-avas}

### Beam and macro-particles

```avas-params
beam:particlenumber beam:randomseed beam:current beam:frequency
```

`randomseed` seeds the random numbers that generate the initial beam. The manual lists it among the `beam.txt` keywords, while the example project writes `randomseed 0` in `input.txt` (the schema accepts it in both files); when both are present the value in `beam.txt` overrides the one in `input.txt` (not documented in the manual; found in the analysis for the kernel rebuild, the same in both kernels).

**The new kernel's default mode** (the default on every platform): a non-zero seed generates exactly the same initial particles every time, with independent random numbers for each plane, so the distribution is right: a fixed seed is reproducible and safe for studying space charge and emittance growth. With `randomseed 0` the seed is taken from the current time, and the seed actually used is written to `OutputFile/inData.seed.txt`, so the run can be reproduced if needed.

**The original kernel and the new kernel's compatibility mode (compat = dll) have a defect.** Measured (2026-11-18, Windows original kernel, example project with `distribution GS GS`, 5260 particles, seed written in `input.txt`, read from `inData.dst`):

- With `randomseed 12345` two runs generate exactly the same initial particles, but the correlation between $x$ and the phase $\varphi$ is $-1.0000$ and between $x'$ and the energy $W$ $0.81$: the transverse and longitudinal planes use the same random numbers, so each particle's longitudinal position is fixed by its transverse one. Each plane's own rms size and emittance are right, but the six-dimensional distribution is unphysical. According to the disassembly analysis done for the engine rebuild, the transverse and longitudinal generators each reseed with the same seed at their start.
- With `randomseed 0` both correlations are about 0 (0.005 and 0.000); by the same analysis 0 means seeding from the current time, so every run draws different particles.

So with the original kernel or the compatibility mode a non-zero seed is fine for comparing results "same beam, different parameters", but conclusions about space charge, coupling between the planes or emittance growth should use `randomseed 0` (with several runs for statistics if needed), or the new kernel's default mode. The research layer (`avas.research`) uses seed 12345 by default and is affected in the same way in these two cases. The Simulation settings page warns next to the random seed, and the scan and response tabs of the Study page show a note too.

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

`scmethod` can be `FFT`, `PICNIC` or `SPICNIC`; the manual only lists the names. The example project's `input.txt` uses `spacecharge 1` and `scmethod FFT` and does not write `numofgrid` or `meshrms`. `spacechargecomp V` (a lattice keyword, space-charge compensation) multiplies the space charge by $(1 - V)$ from that line on; how the kernels differ is described in [Space charge](#space-charge-in-avas).

### Run settings

```avas-params
input:multithreading input:dumpperiodicity input:boundary input:longlimits input:pchistogram input:scanphase
```

- **Multithreading**: write `multithreading 1` or leave the line out; the two are the same (measured with the new kernel and the original kernel on Linux; by the reverse-engineered specification the original kernel on Windows also defaults to 1). The manual says 0 means off, but the engine loses all particles at the start with `multithreading 0` (and 2, 3 …) (verified with the example project: original kernel 2026-09-19, new kernel 2026-11-19). How many threads a run uses is set by “CPU threads” under “Computing kernel” on the Settings page (command line `--threads`, 0 = all cores), for both kernels. The new kernel's results do not depend on it, only the speed does. The original kernel has a multithreading data race where matrix elements such as quadrupoles meet space charge: the space charge comes out too strong and the results differ every run even with a fixed seed (measured on Windows 2026-11-07: end rms x 14–19 % too large); setting CPU threads to 1 avoids it (AVAS passes it to the original kernel as `OMP_NUM_THREADS`; on Linux two runs with 1 thread were identical byte for byte), at a lower speed.
- **Loss boundary**: a particle beyond the element aperture `R` counts as lost; with `boundary 1` the boundary in `boundary.txt` is used instead, the radii in the lattice no longer apply, and lost particles are written to `CollisionData.txt`. `longlimits 1 phase energy` enables longitudinal loss detection, corresponding to the separatrix of [longitudinal dynamics](#longitudinal).
- **Particle data**: `dumpperiodicity V1` writes all particles to `Beamset.plt` every $V_1$ steps (0 = never); the file can become very large. `pchistogram 1 bins` produces `pchistogram.dat`.
- **Phase scan**: the original manual says `scanphase` controls a phase scan of the RF cavities or reads phases from `scanData.txt`, but no kernel reads them; a cavity's phase comes only from `phase` and V3 of its field line (see [longitudinal dynamics](#longitudinal)), and the Simulation settings page removes a leftover `scanphase` line when saved.
- **GPU**: the new kernel computes on NVIDIA GPUs (CUDA) and Mac GPUs (Metal), chosen under Computing kernel on the Simulation settings page. The original kernel's old GPU path on Linux (ini.ini `device = gpu`, the precompiled libPIC.so) is deprecated and will be removed in a later release.

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
- **Results differ slightly every time**: usually the statistical scatter of `randomseed 0`, which samples new particles every run; compare within the statistical error. To reproduce a run byte for byte, write a non-zero seed and use the new kernel. Two runs started within the same second get the same seed with `randomseed 0` (the seed is written to `OutputFile/inData.seed.txt`).

## Further reading {#simulation-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: chapter 9 (multiparticle simulation and space charge).
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: distribution functions and emittance growth.
- R. W. Hockney, J. W. Eastwood, *Computer Simulation Using Particles*, Adam Hilger, 1988: particle–mesh methods and numerical noise.
- C. K. Birdsall, A. B. Langdon, *Plasma Physics via Computer Simulation*: numerical heating and the choice of mesh in particle simulations.
- This primer's [space charge](#space-charge), [errors and correction](#errors) and [tutorial](#tutorial); the complete description of input and output files is in the interface's user manual (parameter and file reference).
