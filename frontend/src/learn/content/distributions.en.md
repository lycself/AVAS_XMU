# Describing a cloud of particles: initial distributions

Imagine two proton beams with the same Twiss parameters and the same rms emittance; the linear preview draws exactly the same envelope for both. Yet send them through the same beamline with a finite aperture and space charge switched on, and one loses almost nothing while the other keeps losing particles on the pipe wall and its emittance grows faster. What do a few rms numbers leave out?

This chapter goes from one particle to a cloud of particles: why second moments (rms quantities) are the natural language for a beam and what they cannot tell; what physical picture each of the four standard distributions — KV, waterbag, parabolic, Gaussian — stands for and how their edges and tails differ; how precise the statistics of a finite number of macro-particles are; and finally how AVAS builds the initial beam. The concepts used here — phase-space coordinates, rms emittance, Twiss parameters — are introduced in [Phase space and emittance](#phase-space); the consequences of the distribution shape under space charge are developed in [Space charge](#space-charge).

## From one particle to a cloud {#distributions-cloud}

At any moment a particle is fixed by six numbers: transverse position and angle $(x, x', y, y')$, longitudinal position deviation and relative momentum deviation $(z, \delta)$ — one point in six-dimensional phase space. A bunch of the example project contains about $7.07\times10^{6}$ protons (bunch charge $Q = I/f$ = 0.1841 mA / 162.5 MHz = 1.133 pC). Listing all their coordinates is neither possible nor useful: what we care about is not where one particular proton is, but how the particles are spread over phase space.

So we use the **phase-space density** $f(x, x', y, y', z, \delta)$: a small phase-space volume $\dd V$ holds about $N f\,\dd V$ particles, with $\int f\,\dd V = 1$. It is like a population-density map: people per square kilometre is more useful than a list of addresses. The analogy fails where hardly anybody lives — in the countryside a cell holds a few houses, and the "density" is just a chance count. A beam is the same: the core is dense and its density is a smooth function; the tail (halo) is sparse, it is where a finite number of macro-particles describes the beam worst — and it is exactly where particles are lost.

The density still holds too much information. The most economical description is its **moments**: the average of any quantity $g$,

$$
\langle g\rangle = \int g\,f\,\dd V \approx \frac{1}{N}\sum_{i=1}^{N} g(\boldsymbol u_i)
$$

(the right-hand side is the estimate from $N$ particles). The first moments $\langle x\rangle$, $\langle x'\rangle$ are the beam centre — how far the bunch is off and where it is heading; the (centred) second moments $\langle x^2\rangle$, $\langle x x'\rangle$, $\langle x'^2\rangle$ give size, divergence and their correlation; higher moments describe the shape, for example the kurtosis $\kappa = \langle x^4\rangle/\langle x^2\rangle^2$ measures how heavy the tails are. The next sections answer in turn: why second moments matter most, what they miss, and how the "distribution type" fills the gap.

## rms: why second moments in particular {#distributions-rms}

### Linear transport turns second moments into second moments

In linear elements the particle coordinates are transformed by a transfer matrix $M$: $\boldsymbol u_1 = M\boldsymbol u_0$. Average $\boldsymbol u_1\boldsymbol u_1\tp$ over all particles:

$$
\Sigma_1 = \langle \boldsymbol u_1 \boldsymbol u_1\tp\rangle = M\,\langle \boldsymbol u_0 \boldsymbol u_0\tp\rangle\,M\tp = M\,\Sigma_0\,M\tp
$$

This step only uses that averaging is linear and that $M$ is the same for every particle; the shape of the distribution never enters. It says: **the second moments at the exit depend only on the second moments at the entrance**. Two beams of completely different shape but with the same second moments have identical rms envelopes in a linear beamline. That is why the linear preview and [linear optics](#linear-optics) need only the Twiss parameters, and why the two beams of the opening have "exactly the same envelope".

And since $\det\Sigma_1 = (\det M)^2\det\Sigma_0$ with $\det M = 1$ for non-dissipative linear transport, the rms emittance $\varepsilon = \sqrt{\det\Sigma}$ (of a 2-D projection) is an invariant of linear motion for a distribution of any shape; see [the σ matrix](#phase-space-sigma).

### Even with space charge, mostly rms counts

More surprising is space charge. Lapostolle introduced the rms emittance and the idea of equivalent beams, and Sacherer (1971) proved: for a continuous beam with elliptical symmetry in the cross-section, the term $\langle x E_x\rangle$ through which space charge enters the rms envelope equation depends only on the rms sizes, not on the shape of the density profile. Beams with the same rms size — uniform, parabolic or Gaussian — therefore feel the same "average" space-charge defocusing and can be replaced by an equivalent uniform beam, the **rms-equivalent beam**. Bunched three-dimensional beams satisfy this only approximately. The influence of the shape does not disappear; it moves into the change of the rms emittance (see [below](#distributions-space-charge)).

### The price: rms is extremely sensitive to tails

rms weights each particle by $x^2$, so the farther from the centre, the heavier the weight.

> **Think about it**: a Gaussian beam has rms size $\sigma_0$. If 1 % of its particles are kicked out to $10\sigma_0$ to form a halo while the rest stays unchanged, what is the new rms size?
>
> Answer: $\langle x^2\rangle = 0.99\,\sigma_0^2 + 0.01\times(10\sigma_0)^2 = 1.99\,\sigma_0^2$, so the rms size grows to $1.41\,\sigma_0$, i.e. by 41 %. One per cent of the particles accounts for half of the rms growth; conversely, a 1 % rms increase may mean that the whole core grew or that a handful of particles drifted far out.

The other blind spot of rms is the edge: with the same rms size the beam may end at twice the rms (KV) or extend without limit (Gaussian). The aperture stops particles at the edge and in the tail, so the question "will particles be lost?" is one rms cannot answer.

## Twiss parameters: the shape of the rms ellipse {#distributions-twiss}

The three second moments of a plane can be rewritten as one emittance and three Twiss parameters:

$$
\beta_x = \frac{\langle x^2\rangle}{\varepsilon}, \qquad
\alpha_x = -\frac{\langle x x'\rangle}{\varepsilon}, \qquad
\gamma_x = \frac{\langle x'^2\rangle}{\varepsilon}, \qquad
\beta_x\gamma_x - \alpha_x^2 = 1
$$

The last identity follows automatically from $\varepsilon^2 = \langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2$. Their meaning: $\beta_x$ is the size squared per unit emittance, $\gamma_x$ the divergence squared per unit emittance, and $\alpha_x$ describes the correlation between position and angle. The correlation coefficient

$$
r = \frac{\langle x x'\rangle}{\sqrt{\langle x^2\rangle\langle x'^2\rangle}} = -\frac{\alpha_x}{\sqrt{1 + \alpha_x^2}}
$$

lies between $-1$ and $1$; $r > 0$ means that particles farther out move outwards faster — the beam is diverging.

Take the x plane of the example beam ($\alpha_x = -0.658$, $\beta_x = 0.377$ mm/mrad, normalized rms emittance 0.1377 π·mm·mrad, $\beta\gamma = 0.05708$): the geometric emittance is $\varepsilon = 0.1377/0.05708 = 2.412$ mm·mrad, the rms size $\sqrt{\beta_x\varepsilon} = 0.954$ mm, the rms angle $\sqrt{\gamma_x\varepsilon} = 3.026$ mrad and the correlation $r = 0.550$: the beam diverges at the entrance.

The Twiss parameters also define **normalized coordinates**:

$$
X = \frac{x}{\sqrt{\beta_x}}, \qquad X' = \frac{\alpha_x x + \beta_x x'}{\sqrt{\beta_x}}, \qquad
X^2 + X'^2 = \gamma_x x^2 + 2\alpha_x x x' + \beta_x x'^2
$$

In these coordinates the rms ellipse becomes a circle of radius $\sqrt{\varepsilon}$, and beams with different Twiss parameters all "look the same". What is left to tell them apart is only how the density changes along the radius — that is the distribution type. The geometry of the ellipse, the sign of $\alpha$ and the Courant–Snyder invariant are covered in [Twiss parameters](#phase-space-twiss).

## The four standard distributions {#distributions-types}

### Defining a distribution by amplitudes

In normalized coordinates $X^2 + X'^2$ is exactly the particle's Courant–Snyder invariant in the x plane (its amplitude squared). Putting both transverse planes together, define a four-dimensional "radius"

$$
r^2 = \frac{X^2 + X'^2 + Y^2 + Y'^2}{a^2}
$$

with $a$ the edge scale. A distribution that depends only on $r^2$ states "how many particles have how large a total transverse amplitude". The four standard distributions are four answers:

- **KV** (Kapchinskij–Vladimirskij): all particles lie on one hyper-ellipsoidal shell $r^2 = 1$, i.e. every particle has the same sum of its two plane amplitudes. That is very unnatural, but it has a unique property: its projection onto real space $(x, y)$ is a **uniform** ellipse. Inside a uniformly charged column the electric field is proportional to the radius (Gauss's law: $E_r$ is the enclosed charge divided by $r$, i.e. $r^2/r$), so the space-charge force is exactly linear and the envelope equations hold exactly — which makes KV the reference of space-charge theory. Real beams are never KV; under strong space charge it also has instabilities of its own.
- **WB** (waterbag): uniform inside $r^2 \le 1$, like a bag full of water with a sharp edge.
- **PB** (parabolic): density $\propto 1 - r^2$, densest at the centre and falling smoothly to zero at the edge.
- **GS** (Gaussian): density $\propto \exp(-R^2/2)$ ($R$ in rms units), with no edge. The sum of many small independent perturbations tends to a Gaussian (central limit theorem); it is also the only rotationally symmetric distribution whose coordinates are mutually independent.

> **Think about it**: the particles of a KV distribution all sit on a "spherical shell". Why does it look like a filled, uniform ellipse in the $x$–$x'$ plane rather than a ring?
>
> Answer: when a four-dimensional sphere is projected onto a plane, the two directions integrated out exactly compensate the curvature of the shell, and the projected density is constant — just as points spread uniformly over an ordinary sphere project uniformly onto a diameter (Archimedes' result). The general formula below gives the same: the projected exponent is $-1 + 1 = 0$.

### Projection formula and edge-to-rms ratios

The definitions above live in four dimensions, while what we see and measure are one-dimensional profiles and two-dimensional phase-space plots. A general result connects them: a distribution with density $\propto (1 - r^2)^k$ in $d$ dimensions, integrated over $d - d'$ of them, has the projected density $\propto (1 - r^2)^{k + (d - d')/2}$. The KV shell corresponds to the limit $k = -1$, WB to $k = 0$, PB to $k = 1$. For $d = 4$ projected onto one coordinate $x$:

$$
\rho(x) \propto \left(1 - \frac{x^2}{a^2}\right)^{m}, \qquad m = \tfrac12,\ \tfrac32,\ \tfrac52 \quad (\mathrm{KV},\ \mathrm{WB},\ \mathrm{PB})
$$

With $\int_{-1}^{1} u^2 (1 - u^2)^m\,\dd u \big/ \int_{-1}^{1} (1 - u^2)^m\,\dd u = 1/(2m + 3)$ this gives

$$
\langle x^2\rangle = \frac{a^2}{2m + 3} = \frac{a^2}{4},\ \frac{a^2}{6},\ \frac{a^2}{8}
\quad\Longrightarrow\quad
\frac{x_{\max}}{x_{\mathrm{rms}}} = 2,\ \sqrt6,\ \sqrt8
$$

Correspondingly, the ellipse containing all particles (the total emittance) has $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}} = 4, 6, 8$ times the area of the rms ellipse. Projected onto the two-dimensional $x$–$x'$ plane the exponent is $k + 1$: KV is a uniform ellipse, WB $\propto 1 - r^2$, PB $\propto (1 - r^2)^2$, so the fraction of particles inside the $n$-times-rms ellipse is $1 - (1 - n/n_{\max})^{k+2}$ ($n_{\max}$ = 4, 6, 8); for the Gaussian it is $1 - \ee^{-n/2}$.

| Distribution | 4-D density | $x$ projection | edge / rms | $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}}$ | beyond 2 × rms | kurtosis $\kappa$ | inside 4 × rms ellipse |
|---|---|---|---|---|---|---|---|
| KV | shell $r^2 = 1$ | $(1 - x^2/a^2)^{1/2}$ | 2 | 4 | 0 | 2.00 | 100 % |
| WB | uniform in $r^2 \le 1$ | $(1 - x^2/a^2)^{3/2}$ | 2.449 | 6 | 2.50 % | 2.25 | 88.9 % |
| PB | $\propto 1 - r^2$ | $(1 - x^2/a^2)^{5/2}$ | 2.828 | 8 | 3.31 % | 2.40 | 87.5 % |
| GS | $\propto \exp(-R^2/2)$ | Gaussian | unbounded | unbounded | 4.55 % | 3 | 86.5 % |

How to read the table: all four have the same rms; from KV to GS the core becomes more peaked and the tail longer. For the example beam ($x_{\mathrm{rms}}$ = 0.954 mm) the KV, WB and PB edges are at 1.91, 2.34 and 2.70 mm; the element aperture `R` = 20 mm is about 21 times the rms, so in a drift without space charge or non-linearities none of the four distributions touches the wall.

See it for yourself in the lab: the four clouds are generated with the same Twiss parameters (by default the x plane of the example beam); the dashed curves are the rms ellipse and the 4 × rms ellipse; the first plot overlays the $x$ projections of the four distributions, normalized to their rms.

```avas-lab
distribution alpha=-0.658 beta=0.377 emit=0.138 W=1.527 mass=938.272 N=4000
```

With the default 4000 particles the lab gives a "largest |x| / rms" of about 2.02, 2.35, 2.62 and 3.65 and a "share beyond 2 rms" of about 0.18 %, 2.45 %, 3.5 % and 4.6 %, matching the theoretical values in the table within statistical fluctuations. A few things are worth noticing:

- The KV cloud exactly fills the 4 × rms ellipse with a sharp edge; 11 % to 14 % of the other three lie outside it.
- KV shows 0.18 % "beyond 2 rms": the rms of this sample is about 1 % below the nominal value (0.944 mm against 0.955 mm), so twice the sample rms falls inside the true edge.
- The largest PB value, 2.62, is clearly below the theoretical edge 2.83: near the edge its density $\propto (1 - u^2)^{5/2}$ is tiny, and the expected maximum of 4000 particles is about 2.66 rms. A finite sample always underestimates the edge of a smooth distribution.
- Push N to 20000 and the Gaussian maximum rises from 3.65 to 4.27 (the theoretical values at probability $1/N$ are about 3.66 and 4.06), while KV and WB hardly change. The "edge" of a Gaussian is only a function of the sample size.

## Statistics: how much N particles can tell {#distributions-statistics}

Estimating $\langle x^2\rangle$ from $N$ particles has a variance of about $(\langle x^4\rangle - \langle x^2\rangle^2)/N$. The rms is the square root of $\langle x^2\rangle$, which halves the relative error, so

$$
\frac{\Delta x_{\mathrm{rms}}}{x_{\mathrm{rms}}} \approx \frac{1}{2}\sqrt{\frac{\kappa - 1}{N}}
$$

The heavier the tail (the larger the kurtosis $\kappa$), the larger the statistical error of the rms: $1/\sqrt{2N}$ for a Gaussian, $1/(2\sqrt N)$ for KV. With the example's 5260 macro-particles the rms size of a Gaussian beam has a statistical error of about 0.97 %, a KV beam about 0.69 %; the rms emittance has errors of the same order.

The resolution of the tail is worse: one macro-particle is a share of $1/N$, so in the example losing one macro-particle means 0.019 %. The largest value of a Gaussian sample sits roughly where $P(|x| > x_{\max}) \approx 1/N$ and grows only like $\sqrt{2\ln N}$.

> **Think about it**: roughly how many macro-particles do you need to see losses of order $10^{-4}$ reliably?
>
> Answer: the number of lost macro-particles is $10^{-4}$ times $N$. For a statistical precision of about 10 % (about 100 lost particles, Poisson error $1/\sqrt{100}$) $N$ must be of order $10^6$; with 5260 macro-particles the expected number is 0.5, which cannot be resolved at all. More on macro-particle numbers and statistical errors in [Multi-particle simulation essentials](#simulation).

## A real beam: the RFQ output in the example {#distributions-real-beam}

The file `InputFile/part_rfq.dst` of the example project is a particle distribution at an RFQ exit: 5260 particles, 1.527 MeV, 0.1841 mA, 162.5 MHz — the very numbers in `beam.txt`; its rms Twiss parameters in x and y agree with `twissx` and `twissy` to five significant digits. In other words, in the transverse planes the example's `beam.txt` is the **rms-equivalent beam** of this distribution, regenerated as `distribution GS GS`; the longitudinal `twissz` does not match the file exactly (see [the example](#distributions-in-avas)).

Statistics of the file (5260 particles):

| Quantity | RFQ beam | KV | WB | PB | GS |
|---|---|---|---|---|---|
| kurtosis $\kappa$ of the $x$ projection | 2.07 | 2.00 | 2.25 | 2.40 | 3 |
| kurtosis $\kappa$ of the $x'$ projection | 2.45 | 2.00 | 2.25 | 2.40 | 3 |
| share of $x$ beyond 2 × rms | 0.89 % | 0 | 2.50 % | 3.31 % | 4.55 % |
| $x$–$x'$: inside the 4 × rms ellipse | 89.3 % | 100 % | 88.9 % | 87.5 % | 86.5 % |
| $x$–$x'$: inside the 6 × rms ellipse | 98.8 % | 100 % | 100 % | 98.4 % | 95.0 % |
| $x$–$x'$: ellipse of the outermost particle / rms | 8.1 | 4 | 6 | 8 | unbounded |

The real beam is none of the four: its position projection is close to KV, its angle projection close to PB, and the phase-plane fractions lie between WB and PB. This is what the non-linear fields and the space charge in the RFQ have made of it. The important difference is the one to GS: GS puts 5.0 % of the particles outside the 6 × rms ellipse, the real beam only 1.2 %. Replacing the real beam by GS leaves linear optics and the rms envelope unchanged but makes the tail much heavier — and when you study losses and halo, that difference is the result.

> **Think about it**: if you change the example's `distribution GS GS` to `WB WB`, does the linear preview on the Lattice page change?
>
> Answer: no. The linear preview pushes only second moments and does not read the distribution type. Only the kernel's multi-particle simulation — with space charge, the sinusoidal fields of the RF cavities and the apertures — can see the difference in shape.

## Why the shape matters under space charge {#distributions-space-charge}

Without space charge and with linear elements, the shape of the distribution has no influence on the rms evolution; it only decides how many particles end up outside the aperture. Space charge changes this, and Gauss's law shows why in one step.

The radial electric field $E_r$ of a long round beam is proportional to the charge enclosed within radius $r$, divided by $r$. Inside a uniform beam the enclosed charge is $\propto r^2$, so $E_r \propto r$: a linear force, which together with linear focusing changes only the envelope, not the emittance. A Gaussian beam (rms $\sigma$) encloses the charge $\lambda\,(1 - \ee^{-r^2/2\sigma^2})$:

$$
E_r = \frac{\lambda}{2\pi\varepsilon_0}\,\frac{1 - \ee^{-r^2/2\sigma^2}}{r}
\;\approx\; \frac{\lambda}{2\pi\varepsilon_0}\,\frac{r}{2\sigma^2}\quad (r \ll \sigma)
$$

It is linear near the centre and falls off like $1/r$ outside. The uniform beam with the same rms has the radius $a = 2\sigma$ and inside it $E_r = \lambda r/(2\pi\varepsilon_0 a^2) = \lambda r/(2\pi\varepsilon_0\cdot 4\sigma^2)$. In other words, **the core of a Gaussian beam feels twice the space-charge defocusing gradient of its rms-equivalent uniform beam**, while the tail feels much less.

A non-linear force makes particles of different amplitude oscillate at different frequencies; the phase-space ellipse is twisted into an S shape and gradually filaments, and the rms emittance grows. In terms of energy: for the same rms size a non-uniform distribution has more field energy than a uniform one; as the beam evolves towards uniformity, the excess "free energy" turns into energy of transverse motion, which shows up as emittance growth. This charge redistribution is fast — it happens in about a quarter of a plasma period — and the growth is larger the higher the current and the less uniform the initial distribution (Wangler, Reiser). A matched KV beam is already uniform in real space and its self-field is exactly linear: it has no free energy to release, so this mechanism gives it no emittance growth.

Non-linear external fields also distinguish distributions: the field of an RF cavity varies sinusoidally with phase, and the longer the longitudinal tail of a bunch, the stronger the non-linearity it feels. The choice of the longitudinal distribution (the second parameter of `distribution`) is sensitive to this.

Take the example beam: the average current is only 0.18 mA, but the charge is concentrated in bunches. The longitudinal rms length 1.316 mm corresponds to $\sigma_t = 77.0$ ps, so the peak current of a Gaussian bunch is about $Q/(\sqrt{2\pi}\,\sigma_t)$ = 5.9 mA, 32 times the average. Whether space charge matters depends on the peak current compared with the external focusing; see [Space charge](#space-charge).

## Common misconceptions {#distributions-misconceptions}

- **"Same Twiss parameters, same beam."** Twiss parameters and rms emittance fix only the second moments. In linear optics the two beams are equivalent; with apertures, losses, space charge and non-linear fields the shape matters just as much.
- **"The total emittance is four times the rms emittance."** Only for KV (and for an ellipse that is uniform in the 2-D phase plane). The 4-D WB gives 6, PB 8, and a Gaussian has no total emittance at all. The ratio also depends on the dimension in which the distribution is defined: a "waterbag" defined in 2-D (uniform in the $x$–$x'$ plane) gives 4, one defined in 4-D gives 6.
- **"The largest value of a sample is the edge of the distribution."** Smooth distributions have a very low density near the edge, so a finite sample underestimates the edge (PB reaches only 2.62 rms with 4000 particles instead of 2.83); the maximum of a Gaussian sample reflects only the sample size.
- **"The σ of a Gaussian is its rms."** Only if it is not truncated. Truncation lowers the rms: a 1-D Gaussian cut at $\pm 3\sigma$ has an rms of $0.987\sigma$, cut at $\pm 2\sigma$ it is $0.880\sigma$.
- **"The rms emittance mainly reflects the beam core."** Quite the opposite: it weights by $x^2$ and is dominated by the few particles far out (1 % of the particles at 10 rms makes the rms size 41 % larger). To study halo, use fractional emittances or tail fractions.
- **"KV is the most ideal and realistic distribution."** KV is the most convenient distribution for theory, not a portrait of real beams; under strong space charge it also has instabilities of its own.
- **"Normalized and geometric emittance differ only in notation."** `beam.txt` holds the normalized rms emittance; the clouds and the emittance shown in this chapter's lab are geometric, a factor $\beta\gamma$ apart (about 17.5 in the example).

## In AVAS {#distributions-in-avas}

### Two kinds of initial beam

The initial beam of AVAS comes from one of two sources, chosen with the "Use particle file" switch (`use_dst`) on the Beam page:

1. **Generated from parameters**: `twissx`, `twissy`, `twissz` give the rms parameters of the three planes, `distribution V1 V2` selects the distribution types, `particlenumber` the number of macro-particles, and the kernel samples with the random seed `randomseed`.
2. **Read from a particle file**: `readparticledistribution` names an existing `.dst` or `.edst` file (for example an RFQ output distribution computed by another code). In this case every keyword of `beam.txt` except `numofcharge` is ignored; an `.edst` file can describe a mixed beam of several particle species.

```avas-params
beam:distribution beam:twissx beam:twissy beam:twissz beam:particlenumber beam:randomseed beam:readparticledistribution beam:use_dst beam:initpos beam:initmom beam:numofcharge beam:beamtype
```

### Keywords

- `distribution V1 V2`: V1 is the transverse distribution (shared by the x and y planes), V2 the longitudinal one; the choices are `KV`, `GS`, `PB`, `WB`.
- `twissx α β ε` (likewise `twissy`, `twissz`): $\alpha$ dimensionless, $\beta$ in mm/mrad, $\varepsilon$ the **normalized rms** emittance (π·mm·mrad); the "angle" of the $z$ plane is $\delta = \Delta p/p$ (in mrad), normalized with the same factor $\beta\gamma$. Units and conversions are in [Phase space: In AVAS](#phase-space-in-avas).
- `particlenumber`: number of macro-particles. 1 selects single-particle mode, in which `twiss*`, `current` and `distribution` have no effect. Each macro-particle carries the charge $Q/N$ with the charge-to-mass ratio of a real particle, so its motion in external fields is unchanged and only the space charge uses the total charge. In the example 5260 macro-particles each stand for about 1344 protons.
- `randomseed`: random seed used to generate the initial beam.
- `initpos x y z` (mm), `initmom x' y' z'` (mrad): position and angle offsets of the whole bunch (they change the first moments), used to study an initial off-centre beam.
- `numofcharge`: charge number; it still applies when a particle file is read.
- `beamtype`: `dc` generates a DC beam, `notdc` a bunched beam.
- `use_dst`: switch used by the interface (not in the manual); 1 reads the file named by `readparticledistribution`, 0 generates the beam from the parameters.

> **AVAS convention**: the manual does not say in how many dimensions AVAS defines KV, WB and PB, or where it truncates the Gaussian; the kernel is the reference. This chapter's lab samples the textbook definitions in 4-D transverse phase space. When you need to know exactly, take the `inData.dst` in a run's output directory — the initial distribution the simulation actually used — and compute its statistics.

> **Observation from one run (not described in the manual)**: the statistics below come from a single `inData.dst` (5260 particles) found in an older AVAS example folder outside this repository; its `beam.txt` is the example's (`distribution GS GS`, without the `beamtype` line), and the engine version that produced it is not recorded. They show what that run did, not documented behaviour; check `inData.dst` of your own run when it matters. In that file the projections on the individual coordinates have kurtosis 2.88 to 3.05, 4.3 % to 4.6 % beyond 2 × rms and their outermost particles at 3.5 to 4.0 rms; the distributions of the 2-D, 4-D and 6-D amplitudes agree with an untruncated Gaussian, and there is no visible correlation between the three planes. No truncation can be seen at this sample size. The generated beam was **not** rescaled to the exact input values: the x-plane rms emittance is 0.1361 (input 0.13768, 1.1 % lower), $\beta_x$ is 0.3796 (0.6 % higher) and $\alpha_x$ is −0.672 (2.1 % larger in magnitude), all within the statistical fluctuations of 5260 particles. Converting the longitudinal coordinates from $(\varphi, W)$ back to $(z, \delta)$ gives $\alpha_z = 0.053$, $\beta_z = 0.378$, $\varepsilon_z = 0.262$, consistent with the `twissz` convention.

### Example: beam.txt and part_rfq.dst

```text
readparticledistribution unknown
use_dst 0
particlenumber 5260
twissx -0.65785 0.37743 0.13768
twissy -0.6341 0.37292 0.14059
twissz 0.04275 0.37292 0.26506
distribution GS GS
```

The Twiss parameters of the x and y planes are the rms values of `part_rfq.dst`. The longitudinal plane differs: the file gives $\beta_z$ = 0.3326 mm/mrad and $\varepsilon_z$ = 0.2659 π·mm·mrad, whereas `twissz` has $\beta_z$ = 0.37292 — the same value as the $\beta$ of `twissy`; the corresponding rms bunch lengths are 1.245 mm and 1.316 mm. This is an observation about the example files; why `twissz` was written this way is not recorded. To start the simulation from the real RFQ distribution, set `use_dst` to 1 and `readparticledistribution` to `part_rfq.dst`. The Beam page can compute the rms parameters of a particle file and fill them into the Twiss fields.

### Points to watch

- The linear preview uses only rms parameters: `twiss*` when the beam is generated, the rms Twiss parameters of the file when a `.dst` is read; an `.edst` file is not read by the preview, which falls back to `twiss*` with a warning. The distribution type does not affect the preview.
- When comparing results of different codes, make sure the initial distributions agree — not only the Twiss parameters but also the distribution type, its dimensional definition and truncation. The safest way is to exchange particle files.
- With a finite number of macro-particles the initial beam itself carries a [statistical error](#distributions-statistics); when judging the effect of a parameter, the difference must be larger than this error to be reliable.

## Further reading {#distributions-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: KV and other stationary distributions, rms emittance and rms-equivalent beams, free energy and emittance growth.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: charge redistribution and emittance growth in the space-charge chapter.
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1101.
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1105.
- I. M. Kapchinskij, V. V. Vladimirskij, *Proc. Int. Conf. on High Energy Accelerators and Instrumentation*, CERN, 1959: the original paper on the KV distribution.
- The AVAS user manual `docs/使用说明20260427.docx`: the `beam.txt` keywords.
