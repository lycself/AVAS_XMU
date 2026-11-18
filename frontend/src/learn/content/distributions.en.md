# Describing a cloud of particles: initial distributions

Imagine two proton beams: their "typical size" is the same, their "typical angle" is the same, and the linear preview draws exactly the same envelope for both. Yet send them into the same beam line, with a pipe of finite width and particles that push each other, and one loses hardly any particles while the other keeps losing them on the wall and becomes more and more "disordered". What do a few average numbers miss when they describe a beam?

An analogy. Two marksmen fire 1000 shots each, and the "average distance" of their hits from the bull's-eye is exactly the same. All of the first marksman's holes are inside the 8 ring, with a clean edge; the second shoots tighter most of the time, but now and then a shot flies off the target sheet. If the edge of the sheet is the wall of the accelerator's pipe, what decides "hitting the wall" is not the average distance but the few shots that fly farthest. A beam is the same: **average numbers speak for "most particles", but losses happen to "very few particles"**.

This chapter goes from "one particle" to "a cloud of particles": which numbers describe a cloud, why they are enough and when they are not; what the four standard distributions KV, waterbag, parabolic and Gaussian look like and how much their edges and tails differ; how precise the statistics of a finite number of macro-particles are; and finally how AVAS creates the initial beam. Phase space, emittance and Twiss parameters, which are used here, are introduced more fully in [Phase space and emittance](#phase-space); the consequences of the distribution shape under space charge are developed in [Space charge](#space-charge).

## From one particle to a cloud {#distributions-cloud}

At any moment one particle is fixed by six numbers: position and angle in the two transverse directions $(x, x', y, y')$, and, along the motion, its position offset $z$ from the reference particle and its relative momentum deviation $\delta = \Delta p/p$. Taking these six numbers as coordinates, a particle is a point in a six-dimensional space (**phase space**). Usually we draw only two numbers at a time, for example $x$ across and $x'$ up, and every particle is one dot in the picture.

One bunch of the example project holds about $7.07\times10^{6}$ protons: the bunch charge is $Q = I/f$ = 0.1841 mA / 162.5 MHz = 1.133 pC (average current divided by the number of bunches per second), divided by the charge of one proton, $1.602\times10^{-19}$ C. Listing the coordinates of seven million protons one by one is impossible, and useless too: what we care about is not where a particular proton is, but "how the particles are distributed in phase space".

It is like population: a list of the addresses of everyone in a country cannot be read, but a **population density map** (people per square kilometre) is clear at a glance. A beam is described by the **phase-space density** $f(x, x', y, y', z, \delta)$: a small volume $\dd V$ of phase space holds about $N f\,\dd V$ particles, and all together $\int f\,\dd V = 1$. The analogy fails where few people live — a square in the countryside holds a handful of households, and its "density" is a chance count. A beam is the same: in the core the particles are dense and the density is a smooth function; in the tails (the **halo**) particles are rare, a finite number of macro-particles describes them least accurately — and particles are lost precisely in the tails.

The density function still contains too much information; the most economical description is a few **averages** (also called **moments**). The average of any quantity $g$ is

$$
\langle g\rangle = \int g\,f\,\dd V \approx \frac{1}{N}\sum_{i=1}^{N} g(\boldsymbol u_i),
$$

where the right-hand side is the estimate from $N$ particles: add up $g$ for every particle and divide by $N$, the school arithmetic mean. $\boldsymbol u_i$ are the six coordinates of particle $i$.

- **First moments** $\langle x\rangle$, $\langle x'\rangle$: the beam centre, i.e. how far the whole bunch is off and in which direction it flies.
- **Second moments** (with the centre subtracted) $\langle x^2\rangle$, $\langle x x'\rangle$, $\langle x'^2\rangle$: the spread of size and angle and their correlation.
- **Higher moments** describe the shape; for example the **kurtosis** $\kappa = \langle x^4\rangle/\langle x^2\rangle^2$ measures how heavy the tails are: the fourth power weights distant particles more, and the larger $\kappa$, the longer the tails.

The next sections answer in turn: why the second moments matter most, what they miss, and how the missing part is filled in by the "distribution type".

## rms: why second moments in particular {#distributions-rms}

### What rms is

**rms** (root mean square) means "square, then average, then take the square root". Example: five particles are at $-2, -1, 0, 1, 2$ mm; the squares are 4, 1, 0, 1, 4, their average is 2, and the square root gives an rms size of 1.41 mm. It is "the typical distance of a particle from the centre". There is also an rms angle (the rms of $x'$). The square root of $\langle x^2\rangle$ above is the rms size.

### Linear transport only sees second moments

In linear elements (force proportional to the distance from the axis, see [Linear optics](#linear-optics)) the exit coordinates of each particle are the entrance coordinates multiplied by coefficients and added, for a drift $x = x_0 + L\,x_0'$. Square it and average over all particles:

$$
\langle x^2\rangle = \langle x_0^2\rangle + 2L\,\langle x_0x_0'\rangle + L^2\,\langle x_0'^2\rangle .
$$

The exit $\langle x^2\rangle$ uses only the three second moments at the entrance, and does not depend on how the particles are distributed in detail. This is a striking result: **two beams of completely different shape but with the same second moments have exactly the same rms envelope in a linear beam line**.

Four particles are enough to check it by hand (positions in mm, angles in mrad):

- **Diamond beam**: four particles at $(\pm1.414, 0)$ and $(0, \pm1.414)$, the four corners of a diamond in the phase-space plot.
- **Square beam**: four particles at $(1, 1)$, $(-1, -1)$, $(1, -1)$, $(-1, 1)$, the four corners of a square.

The two beams have the same second moments: $\langle x^2\rangle = 1$, $\langle x'^2\rangle = 1$, $\langle x x'\rangle = 0$. After 1 m of drift ($x \to x + x'$) the diamond beam's positions become $1.414, -1.414, 1.414, -1.414$ and the square beam's $2, -2, 0, 0$. Both have $\langle x^2\rangle = 2$, an rms size of 1.41 mm, as the formula says. But the farthest particles differ: 1.41 mm for the diamond, 2 mm for the square. **Exactly the same rms, yet edges 40 % apart**.

This is why the linear preview and [Linear optics](#linear-optics) need only the Twiss parameters, and why the two beams at the start of this chapter have "exactly the same envelope". The **rms emittance** $\varepsilon = \sqrt{\langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2}$ (the "area of the ellipse"), computed from second moments alone, is likewise constant under linear transport for any shape of distribution (both beams above have 1 mm·mrad before and after the 1 m), see [the deeper section](#distributions-deep-moments) and [σ-matrix](#phase-space-sigma).

### Even with space charge, mostly rms counts

More surprising is space charge (the electric repulsion between particles). Lapostolle introduced the concepts of rms emittance and equivalent beams, and Sacherer (1971) proved that for a continuous beam with elliptical symmetry in its cross-section, the space-charge term in the equation of the rms size depends only on the rms size and not on the detailed shape of the density profile. So beams with the same rms size — uniform, parabolic or Gaussian — feel the same "average" space-charge defocusing and can be replaced by an equivalent uniform beam, the **rms-equivalent beam**. Bunched three-dimensional beams satisfy this only approximately. The influence of the shape has not disappeared; it has moved into the change of the rms emittance (see [below](#distributions-space-charge)).

### The price: rms is extremely sensitive to tails

rms squares before averaging, so the farther a particle is from the centre, the more weight it has. An example: a Gaussian beam has rms size $\sigma_0$; suppose 1 % of its particles are kicked out to $10\sigma_0$ and form a halo, the rest unchanged. Then

$$
\langle x^2\rangle = 0.99\,\sigma_0^2 + 0.01\times(10\sigma_0)^2 = 1.99\,\sigma_0^2 ,
$$

and the rms size grows to $1.41\,\sigma_0$, i.e. by 41 %. 1 % of the particles decide half of the rms growth; conversely, a 1 % rms growth can mean that the whole core grew or just that very few particles ran away.

The other blind spot of rms is the edge: for the same rms size the beam can end sharply at twice the rms (the KV distribution) or extend without limit (the Gaussian). The pipe stops the particles at the edge and in the tails, so rms cannot answer the question "will particles be lost?".

## Twiss parameters: the shape of the rms ellipse {#distributions-twiss}

Drawn in the $x$–$x'$ plot, a cloud of particles roughly fills an ellipse, the **rms ellipse**. The three second moments of a plane can be rewritten as one emittance (the size of the ellipse) and three **Twiss parameters** (its shape):

$$
\beta_x = \frac{\langle x^2\rangle}{\varepsilon}, \qquad
\alpha_x = -\frac{\langle x x'\rangle}{\varepsilon}, \qquad
\gamma_x = \frac{\langle x'^2\rangle}{\varepsilon}, \qquad
\beta_x\gamma_x - \alpha_x^2 = 1 .
$$

The last equality follows automatically from $\varepsilon^2 = \langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2$ (substitute and see). Their meaning: $\beta_x$ is "the squared size per unit emittance", $\gamma_x$ the squared angle per unit emittance, and $\alpha_x$ describes the correlation of position and angle, i.e. the tilt of the ellipse. The strength of the correlation is measured by the **correlation coefficient**

$$
r = \frac{\langle x x'\rangle}{\sqrt{\langle x^2\rangle\langle x'^2\rangle}} = -\frac{\alpha_x}{\sqrt{1 + \alpha_x^2}},
$$

which lies between $-1$ and $1$; $r > 0$ means that the farther out a particle is, the more it flies outwards — the beam is diverging.

Take the $x$ plane of the example beam ($\alpha_x = -0.658$, $\beta_x = 0.377$ mm/mrad, normalised rms emittance 0.1377 π·mm·mrad, relativistic $\beta\gamma = 0.05708$): the geometric emittance is $\varepsilon = 0.1377/0.05708 = 2.412$ mm·mrad, the rms size $\sqrt{\beta_x\varepsilon} = 0.954$ mm, the rms angle $\sqrt{\gamma_x\varepsilon} = 3.026$ mrad, and the correlation coefficient $r = 0.550$: the beam is diverging at the entrance. (The difference between normalised and geometric emittance is in [Phase space](#phase-space).)

Any rms ellipse can also be "stretched and straightened" into a circle of radius $\sqrt{\varepsilon}$ (**normalised coordinates**, formulas in [the deeper section](#distributions-deep-shapes)). In normalised coordinates all beams with different Twiss parameters "look the same": a circle. What still tells them apart is only **how the density changes from the centre outwards**: crowded centre and sparse edge, or equally dense everywhere; a clear edge or none. This is the **distribution type**. Ellipse geometry, the sign of $\alpha$ and the Courant–Snyder invariant are in [Twiss parameters](#phase-space-twiss).

## The four standard distributions {#distributions-types}

### Four answers

In normalised coordinates the distance of a particle from the centre stands for the **amplitude** of its swing (how far it swings). Taking both transverse planes together, every particle has a "total transverse amplitude". The four standard distributions are four answers to "how many particles have how large a total amplitude":

- **KV** (Kapchinskij–Vladimirskij): every particle has the same total amplitude; they all sit on a "spherical shell" in four-dimensional space. This is very unnatural, but it has a unique property: its projection onto real space $(x, y)$ is a **uniform** ellipse, like a uniformly charged cylinder. Inside a uniformly charged cylinder the electric field is proportional to the radius, so the space-charge force is exactly linear and the envelope equation holds exactly; KV is therefore the benchmark of space-charge theory. Real beams are never KV, and under strong space charge it has instabilities of its own.
- **WB** (waterbag): equally dense everywhere inside its edge, like a bag filled with water; the edge is sharp.
- **PB** (parabolic): densest at the centre, falling smoothly to zero at the edge; the density along the radius is like an upside-down parabola.
- **GS** (Gaussian): densest at the centre, falling outwards along a bell curve, with **no edge** — however far out, there are still a few particles. The sum of many independent small disturbances tends to a Gaussian (the central limit theorem); it is also the only rotationally symmetric distribution whose coordinates are mutually independent.

From KV to GS, with the same rms, the core becomes more peaked and the tails longer.

### Edges, tails and kurtosis

The table compares the four distributions (same rms). A few notes:

- "Edge / rms": how many times the rms the farthest particle is from the centre.
- "$n$ × rms ellipse": the ellipse with $n$ times the area of the rms ellipse ($\sqrt{n}$ times its size). "Inside the 4 × rms ellipse" is the fraction of particles inside the ellipse twice the size of the rms ellipse.
- $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}}$: the area ratio of the ellipse enclosing all particles (the **total emittance**) to the rms ellipse.

| Distribution | 4D density | $x$ projection | Edge / rms | $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}}$ | Beyond 2 × rms | Kurtosis $\kappa$ | Inside 4 × rms ellipse |
|---|---|---|---|---|---|---|---|
| KV | shell $r^2 = 1$ | $(1 - x^2/a^2)^{1/2}$ | 2 | 4 | 0 | 2.00 | 100 % |
| WB | uniform in $r^2 \le 1$ | $(1 - x^2/a^2)^{3/2}$ | 2.449 | 6 | 2.50 % | 2.25 | 88.9 % |
| PB | $\propto 1 - r^2$ | $(1 - x^2/a^2)^{5/2}$ | 2.828 | 8 | 3.31 % | 2.40 | 87.5 % |
| GS | $\propto \exp(-R^2/2)$ | Gaussian | unbounded | unbounded | 4.55 % | 3 | 86.5 % |

(The symbols and derivation of the "4D density" and "$x$ projection" columns are in [the deeper section](#distributions-deep-shapes); $a$ is the size of the edge, $r$ the total amplitude in units of $a$, $R$ in units of the rms.)

For the example beam ($x_{\mathrm{rms}}$ = 0.954 mm) the edges of KV, WB and PB are at 1.91, 2.34 and 2.70 mm; an element aperture `R` = 20 mm is about 21 times the rms, so in a drift without space charge or nonlinearity none of the four distributions touches the wall.

### See for yourself

In the lab, four point clouds are generated with the same Twiss parameters (by default the $x$ plane of the example beam); the dashed lines are the rms ellipse and the 4 × rms ellipse, and the first plot overlays the $x$ projections of the four distributions, normalised to the rms. First look at the size of the four clouds: their rms is exactly the same; then look at the edges: KV ends sharply, while the GS dots straggle far out.

```avas-lab
distribution alpha=-0.658 beta=0.377 emit=0.138 W=1.527 mass=938.272 N=4000
```

With the default 4000 particles the lab gives "largest $|x|$ / rms" of about 2.02, 2.35, 2.62 and 3.65 and "fraction beyond 2 × rms" of about 0.18 %, 2.45 %, 3.5 % and 4.6 %, agreeing with the theoretical values in the table within statistical fluctuations. A few things worth noticing:

- The KV cloud fills the 4 × rms ellipse exactly, with a steep edge; the other three cross it with 11 %–14 % of their particles.
- KV shows 0.18 % "beyond 2 × rms": the rms of this sample is about 1 % below the nominal value (0.944 mm against 0.955 mm), so twice the sample rms lies inside the true edge.
- The PB sample maximum 2.62 is clearly below the theoretical edge 2.83: its density near the edge, $\propto (1 - u^2)^{5/2}$, is extremely low, and the expected maximum of 4000 particles is about 2.66 times the rms. A finite sample always underestimates the edge of a smooth distribution.
- Raise N to 20000 and the GS maximum rises from 3.65 to 4.27 (the theoretical values at probability $1/N$ are about 3.66 and 4.06), while KV and WB hardly change. The "edge" of a Gaussian is just a function of the sample size.

## Statistics: how much N particles can tell {#distributions-statistics}

A simulation uses a few thousand macro-particles to represent millions of real particles, just as an opinion poll uses a thousand people to represent a country. A poll of a thousand people has an error of a few percent; a simulation of a few thousand particles has an error in its rms too.

The relative error of an rms size estimated from $N$ particles is about

$$
\frac{\Delta x_{\mathrm{rms}}}{x_{\mathrm{rms}}} \approx \frac{1}{2}\sqrt{\frac{\kappa - 1}{N}}
$$

(derivation in [the deeper section](#distributions-deep-statistics)). Two intuitive points: four times more particles halve the error; the heavier the tails (the larger the kurtosis $\kappa$), the larger the error, because the rms is swayed by a few distant particles. For a Gaussian it is $1/\sqrt{2N}$, for KV $1/(2\sqrt N)$. With the example's 5260 macro-particles the statistical error of the rms size is about 0.97 % for a Gaussian beam and about 0.69 % for KV; the statistical error of the rms emittance is of the same order.

The resolution in the tails is worse: one macro-particle is a share of $1/N$, so losing one macro-particle in the example is 0.019 %. The maximum of a Gaussian sample sits roughly where "the probability of exceeding it ≈ $1/N$", and grows only slowly with $N$, roughly like $\sqrt{2\ln N}$. Seeing small losses reliably needs a surprising number of particles (see [Think about it](#distributions-think)); more on macro-particle numbers and statistical errors in [Multi-particle simulation](#simulation).

## A real beam: the RFQ output in the example {#distributions-real-beam}

The example project's `InputFile/part_rfq.dst` is a particle distribution at the exit of an **RFQ** (radio-frequency quadrupole, the section that gathers the low-energy beam from the ion source into bunches and accelerates it; in the example its exit energy is 1.527 MeV). AVAS usually does not simulate the RFQ itself: another program computes the particle coordinates at the RFQ exit, and they are handed to AVAS as a particle file like this one (or as rms parameters computed from it) to start the simulation (the new kernel also has an `rfq` element, not yet checked against other codes; see [RFQ](#journey-rfq-in-avas)). This file has 5260 particles, 1.527 MeV, 0.1841 mA and 162.5 MHz, exactly the values in `beam.txt`; its rms Twiss parameters in the $x$ and $y$ planes agree with `twissx` and `twissy` to five significant figures. In other words, in the transverse planes the example `beam.txt` is the **rms-equivalent beam** of this distribution, regenerated as `distribution GS GS`; the longitudinal `twissz` does not quite agree with the file (see [Example](#distributions-in-avas)).

Statistics of the file (5260 particles):

| Quantity | RFQ beam | KV | WB | PB | GS |
|---|---|---|---|---|---|
| Kurtosis $\kappa$ of the $x$ projection | 2.07 | 2.00 | 2.25 | 2.40 | 3 |
| Kurtosis $\kappa$ of the $x'$ projection | 2.45 | 2.00 | 2.25 | 2.40 | 3 |
| Fraction of $x$ beyond 2 × rms | 0.89 % | 0 | 2.50 % | 3.31 % | 4.55 % |
| $x$–$x'$: inside 4 × rms ellipse | 89.3 % | 100 % | 88.9 % | 87.5 % | 86.5 % |
| $x$–$x'$: inside 6 × rms ellipse | 98.8 % | 100 % | 100 % | 98.4 % | 95.0 % |
| $x$–$x'$: ellipse of the outermost particle / rms | 8.1 | 4 | 6 | 8 | unbounded |

The real beam is none of the four: its position projection is close to KV, its angle projection close to PB, and its phase-plane fractions lie between WB and PB. This is shaped jointly by the nonlinear fields and the space charge in the RFQ. What matters is the difference from GS: GS puts 5.0 % of the particles outside the 6 × rms ellipse, the real beam only 1.2 %. Replacing it by GS leaves the linear optics and the rms envelope exactly the same but makes the tails much heavier — when studying losses and halo, this difference *is* the result.

## Why the shape matters under space charge {#distributions-space-charge}

Without space charge and with linear elements, the shape of the distribution has no influence on the rms evolution at all; it only decides how many particles fall outside the aperture. Space charge changes this.

First consider a long, uniformly charged cylindrical beam. A particle at a distance $r$ from the axis is pushed outwards, and the push comes only from the charge "inside" it (the rings of charge outside cancel each other's effect), in proportion to "the charge inside ÷ $r$". In a uniform beam the charge inside is proportional to the cross-section $r^2$, so the push is proportional to $r^2/r = r$: **proportional to the distance from the axis, a linear force**, like a (defocusing) lens, which changes only the envelope and not the emittance.

A Gaussian beam is different: densest at the centre, thinner and thinner outwards. Near the axis the push is still roughly proportional to $r$, but **twice as strong** as in a uniform beam with the same rms; far from the axis "the charge inside" hardly grows any more and the push falls like $1/r$ (calculation in [the deeper section](#distributions-deep-space-charge)). So a Gaussian beam feels a **nonlinear** push: the core is pushed hard, the tails weakly.

Nonlinear forces make particles of different amplitude swing at different rates; the ellipse in phase space is twisted into an S shape and gradually **filaments** (like stirred cream being drawn into threads), and the rms emittance grows. In terms of energy: for the same rms size a non-uniform distribution has more field energy than a uniform one; as the beam evolves towards uniformity the excess "free energy" turns into transverse kinetic energy, seen as emittance growth. This charge redistribution is fast, taking about a quarter of a plasma period, and the growth is larger for higher current and a less uniform initial distribution (Wangler, Reiser). A matched KV beam is already uniform in real space, its self-field is exactly linear and it has no free energy to release, so this mechanism does not make its emittance grow.

Nonlinearity of the external fields distinguishes distributions too: the field of an RF cavity varies sinusoidally with phase, and the longer the tails of the bunch longitudinally, the stronger the nonlinearity they feel. The choice of longitudinal distribution (the second parameter of `distribution`) is sensitive to this.

For the example beam: the average current is only 0.18 mA, but the charge is concentrated in bunches. A longitudinal rms length of 1.316 mm corresponds to a time width $\sigma_t = 77.0$ ps, and the peak current of a Gaussian bunch is about $Q/(\sqrt{2\pi}\,\sigma_t)$ = 5.9 mA, 32 times the average. Whether space charge matters depends on the peak current, compared with the external focusing; see [Space charge](#space-charge).

## Think about it {#distributions-think}

> **Think about it 1**: the particles of a KV distribution all sit on a four-dimensional "spherical shell". Why does it look like a solid, uniform ellipse in the $x$–$x'$ plane rather than a ring?
>
> Answer: when a four-dimensional sphere is projected onto a two-dimensional plane, the two directions "squashed away" exactly compensate the curvature of the sphere, and the projected density is constant — just as points uniformly distributed on an ordinary sphere project uniformly onto a diameter (Archimedes' result). The general formula of [the deeper section](#distributions-deep-shapes) gives the same: the projected exponent is $-1 + 1 = 0$.

> **Think about it 2**: roughly how many macro-particles are needed to see losses of order $10^{-4}$ reliably?
>
> Answer: $10^{-4}$ times $N$ is the number of lost macro-particles. For a statistical precision of about 10 % (about 100 lost particles, counting error $1/\sqrt{100}$), $N$ must be of order $10^6$; with 5260 macro-particles the expected number is only 0.5, impossible to resolve.

> **Think about it 3**: if the example's `distribution GS GS` is changed to `WB WB`, does the linear preview on the lattice page change?
>
> Answer: no. The linear preview pushes only the second moments and does not read the distribution type. Only the engine's multi-particle simulation — under space charge, the sinusoidal fields of RF cavities and the aperture — can see the difference in shape.

> **Think about it 4**: for the diamond and square beams above, does the rms size stay the same however far they travel through a linear beam line? And the distance of the farthest particle?
>
> Answer: the rms size is the same everywhere, because it is set by the second moments alone, and those were the same from the start. The farthest particles go their own ways: after 1 m of drift 1.41 mm for the diamond and 2 mm for the square.

## Common misconceptions {#distributions-misconceptions}

- **"Same Twiss parameters means the same beam."** Twiss parameters and rms emittance fix only the second moments. In linear optics the two are equivalent; with apertures, losses, space charge and nonlinear fields, the shape of the distribution matters just as much.
- **"The total emittance is 4 times the rms emittance."** Only for KV (and for an ellipse that is uniform in the two-dimensional phase plane). A four-dimensional WB gives 6, PB 8, and a Gaussian has no total emittance. The ratio also depends on the number of dimensions in which the distribution is defined: both called "waterbag", the two-dimensional definition (uniform in the $x$–$x'$ plane) gives 4 and the four-dimensional one 6.
- **"The sample maximum is the edge of the distribution."** The edge density of a smooth distribution is very low and a finite sample underestimates the edge (PB with 4000 particles reaches only 2.62 times the rms, not 2.83); the maximum of a Gaussian sample reflects only the sample size.
- **"The σ of a Gaussian is the rms."** Only without truncation. Truncation makes the rms smaller: a one-dimensional Gaussian truncated at $\pm 3\sigma$ in each coordinate has an rms of $0.987\sigma$; truncated at $\pm 2\sigma$, $0.880\sigma$.
- **"The rms emittance mainly reflects the beam core."** The opposite: it weights by $x^2$ and is dominated by a few distant particles (1 % of the particles at 10 times the rms raise the rms size by 41 %). To study halo, look at percentage emittances or tail fractions.
- **"KV is the most ideal, most realistic distribution."** KV is the most convenient distribution for theory, not a portrait of real beams; under strong space charge it has instabilities of its own.
- **"Normalised and geometric emittance differ only in notation."** `beam.txt` holds the normalised rms emittance; the clouds drawn by the lab in this chapter and the emittance it reports are geometric, and the two differ by $\beta\gamma$ (about a factor 17.5 in the example).

## Going deeper: why linear transport only sees second moments {#distributions-deep-moments}

The main text showed "linear transport only sees second moments" with a drift and four particles. This section extends it to any linear element and proves that the rms emittance is constant. The first four steps use only school algebra; the last paragraph is for readers who know linear algebra.

### Step 1: averaging is "linear"

Averaging follows two simple rules: the average of a sum is the sum of the averages, and constants can be taken out of the average:

$$
\langle a\,u + b\,v\rangle = a\,\langle u\rangle + b\,\langle v\rangle .
$$

Example: two particles with $u = 1, 3$ and $v = 2, 0$, and $a = 2$, $b = 5$: $2u + 5v$ is 12 and 6, average 9; $2\langle u\rangle + 5\langle v\rangle = 2 \times 2 + 5 \times 1 = 9$.

### Step 2: any linear element

Any linear element can be written as $x = C x_0 + S x_0'$, $x' = C' x_0 + S' x_0'$ ($C$, $S$, $C'$, $S'$ are the element's four coefficients, the same for every particle; see [Linear optics](#linear-optics-matrices)). A drift has $C = 1$, $S = L$, $C' = 0$, $S' = 1$.

### Step 3: square and average

Square $x$ with the formula for a perfect square, then use Step 1 to take out the coefficients:

$$
\begin{aligned}
\langle x^2\rangle &= C^2\langle x_0^2\rangle + 2CS\,\langle x_0x_0'\rangle + S^2\langle x_0'^2\rangle,\\
\langle x x'\rangle &= CC'\langle x_0^2\rangle + (CS' + SC')\,\langle x_0x_0'\rangle + SS'\langle x_0'^2\rangle,\\
\langle x'^2\rangle &= C'^2\langle x_0^2\rangle + 2C'S'\,\langle x_0x_0'\rangle + S'^2\langle x_0'^2\rangle .
\end{aligned}
$$

Only the three second moments at the entrance appear on the right. The shape of the distribution (higher moments such as $\langle x_0^4\rangle$) never gets a chance to enter.

### Step 4: the rms emittance is constant

Write $A = \langle x_0^2\rangle$, $B = \langle x_0x_0'\rangle$, $D = \langle x_0'^2\rangle$ at the entrance. Substituting Step 3 into $\varepsilon^2 = \langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2$ and expanding (patiently multiplying out), all cross terms cancel and what remains is

$$
\varepsilon_1^2 = (CS' - SC')^2\,(AD - B^2) = (CS' - SC')^2\,\varepsilon_0^2 .
$$

$CS' - SC'$ is the element's determinant, equal to 1 in linear elements without acceleration (see [Linear optics: symplecticity](#linear-optics-symplectic)), so $\varepsilon_1 = \varepsilon_0$. Check with the diamond beam of the main text: at the entrance $A = D = 1$, $B = 0$, $\varepsilon = 1$; after 1 m of drift $\langle x^2\rangle = 2$, $\langle x'^2\rangle = 1$, $\langle xx'\rangle = (0 + 0 + 2 + 2)/4 = 1$, $\varepsilon^2 = 2 \times 1 - 1 = 1$, still 1. The square beam also gives 1.

### Step 5: rms under space charge

Sacherer's result says: in the equation of the rms size, space charge appears as $\langle x E_x\rangle$ (the average of position times field); for a continuous beam with elliptically symmetric cross-section this average depends only on the rms sizes, not on the density profile. So the rms envelope equation is the same for every such distribution and can be computed with the rms-equivalent uniform beam. This does not guarantee a constant rms emittance: the nonlinear part of the force still makes the emittance grow, see the main text.

### For readers who know linear algebra

Arranging the second moments in the σ-matrix $\Sigma = \langle \boldsymbol u \boldsymbol u\tp\rangle$, linear transport $\boldsymbol u_1 = M\boldsymbol u_0$ gives

$$
\Sigma_1 = \langle \boldsymbol u_1 \boldsymbol u_1\tp\rangle = M\,\langle \boldsymbol u_0 \boldsymbol u_0\tp\rangle\,M\tp = M\,\Sigma_0\,M\tp ;
$$

this step uses only "averaging is linear and $M$ is the same for all particles", independent of the shape of the distribution. And since $\det\Sigma_1 = (\det M)^2\det\Sigma_0$ while dissipation-free linear transport has $\det M = 1$, the rms emittance $\varepsilon = \sqrt{\det\Sigma}$ (two-dimensional projection) is an invariant of linear motion for any shape of distribution, see [σ-matrix](#phase-space-sigma).

## Going deeper: edges and tails of the four distributions {#distributions-deep-shapes}

The table in the main text gave the edges, tail fractions and kurtoses of the four distributions. This section computes the numbers of the table step by step, starting from normalised coordinates. The first five steps use only school algebra and definite integrals of powers; the last paragraph is for readers who know calculus.

### Step 1: turning the ellipse into a circle

The Twiss parameters give a set of **normalised coordinates**:

$$
X = \frac{x}{\sqrt{\beta_x}}, \qquad X' = \frac{\alpha_x x + \beta_x x'}{\sqrt{\beta_x}} .
$$

Square them and add, using $\gamma_x = (1 + \alpha_x^2)/\beta_x$:

$$
X^2 + X'^2 = \frac{x^2 + \alpha_x^2x^2 + 2\alpha_x\beta_x xx' + \beta_x^2x'^2}{\beta_x} = \gamma_x x^2 + 2\alpha_x x x' + \beta_x x'^2 .
$$

The right-hand side is exactly the particle's Courant–Snyder invariant in the $x$ plane (the squared amplitude). In $(X, X')$ coordinates the rms ellipse becomes a circle of radius $\sqrt{\varepsilon}$, and $\langle X^2\rangle = \langle X'^2\rangle = \varepsilon$.

### Step 2: defining distributions by the "total amplitude"

Taking both transverse planes together, define a four-dimensional "radius"

$$
r^2 = \frac{X^2 + X'^2 + Y^2 + Y'^2}{a^2},
$$

with $a$ the edge scale. A distribution that depends only on $r^2$ says "how many particles have how large a total transverse amplitude". KV has all particles on the shell $r^2 = 1$; WB is uniform in $r^2 \le 1$; PB has a density $\propto 1 - r^2$; GS a density $\propto \exp(-R^2/2)$ ($R$ in units of the rms).

### Step 3: how many times the rms the edge is

In four dimensions the volume of a "ball" of radius $\le r$ is proportional to $r^4$ (area of a circle $\propto r^2$, volume of a ball $\propto r^3$, and so on), so the volume of a thin shell near radius $r$ is proportional to $r^3$. From this compute $\langle r^2\rangle$ and share it equally among the four coordinates by symmetry: $\langle X^2\rangle = a^2\langle r^2\rangle/4$.

- **KV**: every particle has $r^2 = 1$, $\langle r^2\rangle = 1$, $\langle X^2\rangle = a^2/4$, edge / rms $= a/(a/2) = 2$.
- **WB**: uniform density, weighted by the thin shells, $\langle r^2\rangle = \dfrac{\int_0^1 r^2\cdot r^3\,\dd r}{\int_0^1 r^3\,\dd r} = \dfrac{1/6}{1/4} = \dfrac23$, $\langle X^2\rangle = a^2/6$, edge / rms $= \sqrt6 = 2.449$.
- **PB**: multiply also by the density $1 - r^2$, $\langle r^2\rangle = \dfrac{\int_0^1 r^2(1 - r^2)\,r^3\,\dd r}{\int_0^1 (1 - r^2)\,r^3\,\dd r} = \dfrac{1/6 - 1/8}{1/4 - 1/6} = \dfrac12$, $\langle X^2\rangle = a^2/8$, edge / rms $= \sqrt8 = 2.828$.

Only the definite integral of a power, $\int_0^1 r^n\,\dd r = 1/(n+1)$, is used. The ellipse enclosing all particles has radius $a$ in normalised coordinates, "area" $a^2$; the rms ellipse has $\langle X^2\rangle$. Their ratio is $\varepsilon_{100\%}/\varepsilon_{\mathrm{rms}} = 4, 6, 8$.

### Step 4: projection — Archimedes' hat box

What we see and measure are one-dimensional profiles and two-dimensional phase plots, so the four-dimensional distribution has to be "squashed" to fewer dimensions. Archimedes found that points uniformly distributed on a sphere project **uniformly** onto a diameter (the area of a spherical zone between two parallel planes depends only on the distance between the planes). The general result: a distribution with density $\propto (1 - r^2)^k$ in $d$ dimensions, with $d - d'$ directions squashed away, has a projected density $\propto (1 - r^2)^{k + (d - d')/2}$: each squashed direction adds $\tfrac12$ to the exponent. The KV shell is the limit $k = -1$, WB is $k = 0$, PB $k = 1$.

- Projected onto one dimension $x$ (3 directions squashed away):

$$
\rho(x) \propto \left(1 - \frac{x^2}{a^2}\right)^{m}, \qquad m = \tfrac12,\ \tfrac32,\ \tfrac52 \quad (\mathrm{KV},\ \mathrm{WB},\ \mathrm{PB}) .
$$

- Projected onto the two-dimensional $x$–$x'$ plane (2 directions squashed away), the exponent is $k + 1$: KV is a uniform ellipse (exponent $-1 + 1 = 0$, exactly [Think about it 1](#distributions-think)), WB $\propto 1 - r^2$, PB $\propto (1 - r^2)^2$.

### Step 5: how many particles fit in an ellipse

In the two-dimensional projection, let $t$ = ellipse area ÷ total emittance ($t = n/n_{\max}$ with $n_{\max}$ = 4, 6, 8); the fraction inside the $n$ × rms ellipse is the density $(1 - s)^{k+1}$ integrated from 0 to $t$ ($s$ is the squared radius in units of $a^2$; in two dimensions $s$ is weighted uniformly):

$$
\frac{\int_0^t (1 - s)^{k+1}\,\dd s}{\int_0^1 (1 - s)^{k+1}\,\dd s} = 1 - \left(1 - \frac{n}{n_{\max}}\right)^{k+2} .
$$

Check $n = 4$: KV $1 - (1 - 1)^1 = 100\ \%$; WB $1 - (1 - 4/6)^2 = 88.9\ \%$; PB $1 - (1 - 4/8)^3 = 87.5\ \%$. At $n = 6$, PB gives $1 - (1/4)^3 = 98.4\ \%$. A Gaussian has no edge, and its fraction is $1 - \ee^{-n/2}$: 86.5 % at $n = 4$, 95.0 % at $n = 6$. The one-dimensional fractions "beyond 2 × rms" (WB 2.50 %, PB 3.31 %, GS 4.55 %) need integrals of $(1 - u^2)^m$; the results are listed directly in the table.

### For readers who know calculus

In general $\int_{-1}^{1} u^2 (1 - u^2)^m\,\dd u \big/ \int_{-1}^{1} (1 - u^2)^m\,\dd u = 1/(2m + 3)$, so

$$
\langle x^2\rangle = \frac{a^2}{2m + 3} = \frac{a^2}{4},\ \frac{a^2}{6},\ \frac{a^2}{8}
\quad\Longrightarrow\quad
\frac{x_{\max}}{x_{\mathrm{rms}}} = 2,\ \sqrt6,\ \sqrt8 ;
$$

likewise $\langle u^4\rangle/\langle u^2\rangle^2 = 3(2m + 3)/(2m + 5)$, giving kurtoses of 2, 2.25 and 2.4, tending to the Gaussian 3 for $m \to \infty$. The rule for the projected exponent comes from the volume integral over a $(d - d')$-dimensional ball, $\int (1 - r^2 - \rho^2)^k\,\dd^{d-d'}\rho \propto (1 - r^2)^{k + (d - d')/2}$. For the Gaussian fraction: in normalised coordinates $J = X^2 + X'^2$ of a two-dimensional Gaussian is exponentially distributed, $P(J \le n\varepsilon) = 1 - \ee^{-n/2}$.

## Going deeper: where the statistical error comes from {#distributions-deep-statistics}

The main text gave the statistical error of the rms as $\tfrac12\sqrt{(\kappa - 1)/N}$. This section derives it from the school statistics of "the error of a mean".

### Step 1: the error of a mean shrinks like √N

Toss a coin, scoring 1 for heads and 0 for tails; the spread (standard deviation) of one toss is 0.5. Toss 100 times and average (the fraction of heads): the spread of the average shrinks to $0.5/\sqrt{100} = 0.05$, so it lies roughly between 45 % and 55 %. In general the mean of $N$ independent samples has a standard deviation $1/\sqrt N$ times that of a single sample (as taught at school: the variance shrinks as $1/N$).

### Step 2: estimating ⟨x²⟩ is averaging x²

Estimating $\langle x^2\rangle$ means adding up every particle's $x^2$ and dividing by $N$. The spread of a single particle's $x^2$ is $\sqrt{\langle x^4\rangle - \langle x^2\rangle^2}$ ("mean of the square minus square of the mean" is the variance). Written with the kurtosis $\kappa = \langle x^4\rangle/\langle x^2\rangle^2$ it equals $\langle x^2\rangle\sqrt{\kappa - 1}$. By Step 1, after averaging over $N$ particles:

$$
\frac{\Delta\langle x^2\rangle}{\langle x^2\rangle} \approx \sqrt{\frac{\kappa - 1}{N}} .
$$

### Step 3: the square root halves the relative error

The rms is the square root of $\langle x^2\rangle$. If a number is 2 % too large, its square root is only about 1 % too large: $\sqrt{1.02} = 1.00995$. In general $\sqrt{1 + \epsilon} \approx 1 + \epsilon/2$, so the relative error of the rms is half of the above — the formula of the main text.

### Step 4: putting in numbers

Gaussian, $\kappa = 3$: $\tfrac12\sqrt{2/N} = 1/\sqrt{2N}$, which for $N = 5260$ is $1/\sqrt{10520} = 0.97\ \%$. KV, $\kappa = 2$: $\tfrac12\sqrt{1/N} = 1/(2\sqrt N) = 0.69\ \%$. So when judging the effect of a parameter on the rms, the difference must be clearly larger than about 1 % to be trusted (differences between random seeds are of this order too).

### Step 5: counting particles in the tails

Lost particles have to be counted one by one. If $Np$ losses are expected ($p$ being the loss fraction), the actual count fluctuates by about $\sqrt{Np}$ (the Poisson fluctuation of counting). So the relative error is $1/\sqrt{Np}$: 10 % precision needs $Np \approx 100$, and for $p = 10^{-4}$, $N \approx 10^6$. The maximum of a Gaussian sample sits roughly at $P(\lvert x\rvert > x_{\max}) \approx 1/N$; this probability falls extremely fast with $x_{\max}$, so $x_{\max}$ grows only slowly with $N$, roughly like $\sqrt{2\ln N}$.

### For readers who know probability theory

The sample estimate $\frac1N\sum x_i^2$ of $\langle x^2\rangle$ has variance $(\mu_4 - \mu_2^2)/N$; by the delta method the variance of $\sqrt{\cdot}$ is multiplied by $(1/(2\sqrt{\mu_2}))^2$, giving the relative standard deviation $\tfrac12\sqrt{(\kappa - 1)/N}$. This assumes independent sampling and a known centre; centring with the sample mean adds only an $O(1/N)$ correction. The rms emittance involves three second moments; its error is of the same order but depends on the $x$–$x'$ correlation.

## Going deeper: the space-charge field of a Gaussian beam {#distributions-deep-space-charge}

The main text said that "the core of a Gaussian beam feels twice the space-charge defocusing of the rms-equivalent uniform beam". This section computes this "twice" step by step, and the peak current of the example bunch.

### Step 1: the field of a long, straight charged beam

Take a very long charged cylinder with charge $\lambda$ per unit length. The radial field at a distance $r$ from the axis is set only by the charge $\lambda_{\text{in}}$ inside radius $r$: imagine the field lines leaving the charge inside evenly in all directions and crossing a circle of circumference $2\pi r$, so

$$
E_r = \frac{\lambda_{\text{in}}}{2\pi\varepsilon_0\,r},
$$

with $\varepsilon_0$ the vacuum permittivity. (The charge outside cancels its own effect on the particles inside, for the same reason that there is no gravity inside a uniform spherical shell; the rigorous argument is Gauss's law.)

### Step 2: the uniform beam

In a uniform beam of radius $a$ the charge inside $r$ is the fraction $r^2/a^2$, so $E_r = \lambda r/(2\pi\varepsilon_0 a^2)$, **proportional to $r$**: a linear force. The rms size of a uniform round beam is $a/2$: over the cross-section $\langle r^2\rangle = \int_0^a r^2\cdot r\,\dd r \big/ \int_0^a r\,\dd r = a^2/2$, shared equally between $x$ and $y$, $\langle x^2\rangle = a^2/4$. So a uniform beam with rms $\sigma$ has $a = 2\sigma$, and inside

$$
E_r = \frac{\lambda\,r}{2\pi\varepsilon_0\cdot 4\sigma^2} .
$$

### Step 3: the Gaussian beam

In a round Gaussian beam with rms $\sigma$ in each direction, the charge inside $r$ is $\lambda\,(1 - \ee^{-r^2/2\sigma^2})$:

$$
E_r = \frac{\lambda}{2\pi\varepsilon_0}\,\frac{1 - \ee^{-r^2/2\sigma^2}}{r}
\;\approx\; \frac{\lambda}{2\pi\varepsilon_0}\,\frac{r}{2\sigma^2}\quad (r \ll \sigma) .
$$

The approximation uses $\ee^{-u} \approx 1 - u$ for small $u$ (for example $\ee^{-0.01} = 0.99005$). Near the centre it is linear; far out $1 - \ee^{-r^2/2\sigma^2} \to 1$ and the field falls like $1/r$.

### Step 4: comparing

Near the centre the Gaussian beam has $E_r/r = \lambda/(2\pi\varepsilon_0\cdot 2\sigma^2)$, the uniform beam $\lambda/(2\pi\varepsilon_0\cdot 4\sigma^2)$: **the space-charge defocusing gradient in the core of a Gaussian beam is twice that of the rms-equivalent uniform beam**, while the tails feel a much weaker force. It is this "strong core, weak tails" nonlinearity that makes the rms emittance grow.

### Step 5: the peak current of the example bunch

The longitudinal rms length of the example beam is $\sigma_z = 1.316$ mm and its speed $v = \beta c = 0.0570 \times 2.998\times10^{8} = 1.708\times10^{7}$ m/s, so in time $\sigma_t = \sigma_z/v = 77.0$ ps. The charge $Q = 1.133$ pC of a Gaussian bunch is concentrated in a time of about $\sqrt{2\pi}\,\sigma_t$, so the peak current is

$$
I_{\text{peak}} \approx \frac{Q}{\sqrt{2\pi}\,\sigma_t} = \frac{1.133\ \mathrm{pC}}{2.507 \times 77.0\ \mathrm{ps}} = 5.9\ \mathrm{mA},
$$

32 times the average current of 0.184 mA.

### For readers who know electromagnetism

Step 1 is the two-dimensional Gauss law $\oint \boldsymbol E\cdot\dd\boldsymbol l_\perp = \lambda_{\text{in}}/\varepsilon_0$. Sacherer's result rests on $\langle x E_x\rangle = \lambda/(4\pi\varepsilon_0)\cdot\sigma_x/(\sigma_x + \sigma_y)$ holding for any elliptically symmetric distribution; for the free-energy argument see Reiser's book in Further reading (the difference in field energy between a non-uniform and a uniform distribution turns into transverse thermal energy within about a quarter of a plasma period).

## In AVAS {#distributions-in-avas}

### Two kinds of initial beam

AVAS's initial beam has two possible sources, chosen with the "use particle file" switch on the beam page (`use_dst`):

1. **Generated from parameters**: `twissx`, `twissy` and `twissz` give the rms parameters of the three planes, `distribution V1 V2` chooses the distribution type, `particlenumber` the number of macro-particles, and the engine samples with the random seed `randomseed`.
2. **Read from a particle file**: `readparticledistribution` names an existing `.dst` or `.edst` file (for example an RFQ output distribution computed by another program). Then all other `beam.txt` keywords except `numofcharge` have no effect; an `.edst` file can describe a mixed beam of several particle species.

```avas-params
beam:distribution beam:twissx beam:twissy beam:twissz beam:particlenumber beam:randomseed beam:readparticledistribution beam:use_dst beam:initpos beam:initmom beam:numofcharge beam:beamtype
```

### Keywords

- `distribution V1 V2`: V1 is the transverse distribution (shared by the $x$ and $y$ planes), V2 the longitudinal one; choose from `KV`, `GS`, `PB`, `WB`.
- `twissx α β ε` (likewise `twissy`, `twissz`): $\alpha$ dimensionless, $\beta$ in mm/mrad, $\varepsilon$ the **normalised rms** emittance (π·mm·mrad); the "angle" of the $z$ plane is $\delta = \Delta p/p$ (in mrad), likewise normalised with $\beta\gamma$. Units and conversions are in [Phase space: In AVAS](#phase-space-in-avas).
- `particlenumber`: number of macro-particles. 1 means single-particle mode, in which `twiss*`, `current` and `distribution` have no effect. Each macro-particle carries the charge $Q/N$ with the same charge-to-mass ratio as a real particle, so its motion in external fields is unchanged, and only space charge is computed from the total charge. The example's 5260 macro-particles each represent about 1344 protons.
- `randomseed`: random seed for generating the initial beam. The original kernel and the new kernel's compatibility mode (compat = dll) have a defect when `randomseed` ≠ 0: the generated distribution has $x$ fully correlated with the phase (correlation −1), which shows in `inData.dst`; the Simulation settings page warns next to the random seed. The new kernel's default mode does not have this defect.
- `initpos x y z` (mm), `initmom x' y' z'` (mrad): add a position and angle offset to the whole bunch (changing the first moments), for studying an initially off-centre beam.
- `numofcharge`: charge number; it still applies when a particle file is read.
- `beamtype`: `dc` generates a DC beam, `notdc` a bunched beam.
- `use_dst`: a switch used by the interface (not in the manual); 1 reads the file named by `readparticledistribution`, 0 generates from parameters.

> **AVAS convention**: the manual does not say in how many dimensions AVAS defines KV, WB and PB when generating them, nor where it truncates the Gaussian; the engine is authoritative. The lab in this chapter samples by the textbook definitions in four-dimensional transverse phase space. When you need to know exactly, take statistics directly from `inData.dst` in the run's output directory — the initial distribution the simulation actually used.

> **Observation from one run (not in the manual)**: the statistics below come from an `inData.dst` (5260 particles) in an older AVAS example directory outside this repository; its `beam.txt` is the same as the example's (`distribution GS GS`, lacking only the `beamtype` line), and the engine version that produced it was not recorded. These numbers describe that one run, not documented behaviour; when needed, take statistics from the `inData.dst` of your own run. In that file the kurtoses of the coordinate projections are 2.88–3.05, the fractions beyond 2 × rms 4.3 %–4.6 %, the outermost particles at 3.5–4.0 times the rms, the distributions of the 2D, 4D and 6D amplitudes agree with an untruncated Gaussian, and there is no visible correlation between the three planes. No truncation is visible at this sample size. The result of that generation was **not** rescaled to the exact input values: the $x$-plane rms emittance is 0.1361 (input 0.13768, 1.1 % smaller), $\beta_x$ 0.3796 (0.6 % larger), $\alpha_x$ −0.672 (2.1 % larger), all within the statistical fluctuations of 5260 particles. Converting the longitudinal $(\varphi, W)$ back to $(z, \delta)$ gives $\alpha_z = 0.053$, $\beta_z = 0.378$, $\varepsilon_z = 0.262$, consistent with the convention of `twissz`.

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

The Twiss parameters of the $x$ and $y$ planes are the rms values of `part_rfq.dst`. The longitudinal plane differs: statistics of the file give $\beta_z$ = 0.3326 mm/mrad and $\varepsilon_z$ = 0.2659 π·mm·mrad, whereas `twissz` has $\beta_z$ = 0.37292, the same as the $\beta$ of `twissy`; the corresponding rms bunch lengths are 1.245 mm and 1.316 mm. This is only an observation about the example files; why `twissz` was filled in this way is not recorded. To start the simulation from the real RFQ distribution, set `use_dst` to 1 and `readparticledistribution` to `part_rfq.dst`. The beam page can compute rms parameters from a particle file and fill in the Twiss fields.

### Points to watch

- The linear preview uses only rms parameters: `twiss*` when the beam is generated from parameters, the rms Twiss parameters of the file when a `.dst` is read; `.edst` files are not read by the preview, which then uses `twiss*` and says so. The distribution type does not affect the preview.
- When comparing results of different programs, make sure the initial distributions agree — not just the Twiss parameters, but also the distribution type, the definition of dimensions and the truncation. The safest way is to exchange particle files.
- With a finite number of macro-particles, the initial beam itself carries a [statistical error](#distributions-statistics); a difference due to some parameter must exceed this error to be trusted.

## Further reading {#distributions-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: KV and other stationary distributions, rms emittance and rms-equivalent beams, free energy and emittance growth.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: charge redistribution and emittance growth in the space-charge chapter.
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1101.
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1105.
- I. M. Kapchinskij, V. V. Vladimirskij, *Proc. Int. Conf. on High Energy Accelerators and Instrumentation*, CERN, 1959: the original paper on the KV distribution.
- AVAS manual `docs/使用说明20260427.docx`: `beam.txt` keywords.
