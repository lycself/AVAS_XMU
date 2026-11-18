# Beam matching

Send the example beam straight into a FODO channel and its rms $x$ jumps irregularly between 0.71 mm and 2.80 mm; put four well-tuned quadrupoles in front of the channel and the same beam in the same channel only swings regularly between 1.02 mm and 1.86 mm, repeating every period. The channel is the same, the emittance is the same; the only difference is the "shape" of the beam as it enters the channel. This chapter answers why the shape of the beam matters so much, what **matching** means, how mismatch is measured, what mismatch does under space charge, how many knobs a match needs, and how to do it in AVAS.

First the animation. This is the 6-period FODO channel (gradient 6 T/m) met in the [quadrupole](#quadrupole-fodo) chapter, except that the entering beam has been enlarged by a factor of 1.5 (the "mismatch" slider is at 1.5).

```avas-motion
fodo mismatch=1.5
```

First pull the "mismatch" slider back to 1: the width of the beam traces the same pattern in every period, the horizontal rms size rises and falls regularly between 0.88 mm and 1.80 mm, and the readout "largest rms size" on the right is 1.82 mm. Now set it to 1.5: the beam no longer breathes regularly but jumps irregularly, often thick in one period and thin in the next; horizontally it reaches 4.08 mm at its widest and only 0.39 mm at its narrowest. No particle hits the wall yet, but three times the rms size has grown from about 5.5 mm to about 12 mm, much closer to the 20 mm pipe. The beam was enlarged by only 1.5, yet its widest point is 2.2 times the matched one; this chapter explains why.

The chapter uses ideas from [phase space](#phase-space) and [linear optics](#linear-optics), each explained again where it first appears; the choice of periods and phase advances is in [lattice design](#lattice-design), space charge in [space charge](#space-charge).

## What matching is: a tumbling ellipse {#matching-definition}

### The ellipse in phase space

Two numbers describe a particle's transverse motion: its distance from the axis $x$ and the angle $x'$ between its direction and the axis. Draw every particle as a point in a plane (horizontal axis $x$, vertical axis $x'$); this plane is **phase space**. A beam is a cloud of points whose boundary is roughly an ellipse.

The ellipse has three features:

- **Area**: proportional to the **emittance** $\varepsilon$, how "disordered" the beam is. Linear focusing elements can change the shape of the ellipse but not its area.
- **Width**: the projection of the ellipse on the $x$ axis is the beam size. It is described by **β** (in m): $x_{\mathrm{rms}} = \sqrt{\beta\varepsilon}$.
- **Tilt**: which way the ellipse leans. It is described by **α**: for $\alpha < 0$ particles farther from the axis fly farther outward and the beam is opening up; for $\alpha > 0$ the beam is converging; for $\alpha = 0$ the ellipse is upright and the beam is at its narrowest or widest (a waist).

$\alpha$ and $\beta$ are the **Twiss parameters**, together with a third, $\gamma = (1 + \alpha^2)/\beta$, fixed by them (it shares its name with the relativistic factor $\gamma$ but is a different quantity). These three numbers are the shape of the beam.

### A rotating ellipse

In a periodic channel the particles swing about the axis (see [lattice design](#lattice-design-periods)). Seen in phase space, the swing is a point circling the origin: every period each particle turns through the same angle $\sigma$ (the phase advance per period).

If the coordinates are stretched suitably so that the particles move on circles, a round beam turns into the same circle, and the beam size is the same in every period: that is **matching**. If the beam is an elongated ellipse, the turning makes it tumble like a rugby ball: its projection on the $x$ axis (the beam size) is now large, now small. Half a turn brings the ellipse back to the same projection, so the envelope ripples at twice the frequency of the particle oscillation. In the animation the phase advance is about $76^\circ$, the ripple of the envelope advances about $151^\circ$ per period (twice the particle phase advance), close to half a ripple ($180^\circ$), which is why one period is often thick and the next thin.

The picture relies only on linear motion; space charge and non-linearities change the details, but the definition "matched = the phase-space ellipse agrees with the eigen-ellipse of the periodic structure" holds in every case.

### The matched beam: the ellipse the channel "likes"

So every periodic channel has an ellipse it "likes": enter with this shape, and after one period you leave with the same shape. To find it, let an ellipse travel through one period and require its outgoing $\alpha$ and $\beta$ to equal the incoming ones: two equations, two unknowns. The "going deeper" part gives the solution: once the transfer matrix of one period is known, this $(\alpha, \beta)$, the **periodic solution**, can be read off directly.

The periodic solution changes with position: at the centre of the focusing quadrupole (F) of a FODO $\alpha = 0$ and $\beta$ is largest, at the centre of the defocusing quadrupole (D) $\alpha = 0$ and $\beta$ is smallest. Matching means agreeing with the periodic solution at the entrance of the channel; after that the beam follows the periodic solution by itself.

Example: the FODO of [lattice design](#lattice-design-periods) ($G = 5\ \mathrm{T/m}$, quadrupole length $L_q = 0.1\ \mathrm{m}$, drift $L_d = 0.3\ \mathrm{m}$) with the period starting at the F quadrupole entrance has the periodic solution $\alpha_x = -1.762$, $\beta_x = 1.256\ \mathrm{m}$ in x and $\alpha_y = 0.782$, $\beta_y = 0.493\ \mathrm{m}$ in y. The example beam at its entrance has $\alpha_x = -0.658$, $\beta_x = 0.377\ \mathrm{m}$: far off.

In the phase-space lab send an ellipse through a thin lens and a drift and see how α and β together decide the size downstream:

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0.5 L=0.5
```

## How large is the mismatch: the mismatch factor {#matching-mismatch-factor}

### Definition

To compare a beam $(\alpha, \beta, \gamma)$ with the matched ellipse $(\alpha_0, \beta_0, \gamma_0)$ (same position, same emittance), first compute an intermediate quantity $R$, then the **mismatch factor** $M$:

$$
R = \beta\gamma_0 + \beta_0\gamma - 2\alpha\alpha_0,\qquad
M = \left[\frac{1}{2}\left(R + \sqrt{R^2 - 4}\right)\right]^{1/2} - 1
$$

This is the definition of Wangler's book and of common envelope codes. Its meaning is intuitive: $1 + M$ is the factor by which the mismatched ellipse exceeds the matched one in the least favourable direction (the "going deeper" part explains where it comes from). Several properties follow:

- $R \ge 2$, with $R = 2$ and $M = 0$ exactly when the two ellipses coincide.
- **In linear transport without space charge the mismatch factor is conserved along the line** and can be computed at any position: the ellipse just turns with the channel, and the difference in shape stays the same.
- Enclosing the mismatched beam in an ellipse of the matched shape takes an emittance of $(1+M)^2\varepsilon$.
- Without space charge the envelope of a mismatched beam beats between $1/(1+M)$ and $1+M$ times the matched envelope ("beating" meaning the envelope swings above and below the matched envelope).

### Some numbers

Where $\alpha_0 = \alpha = 0$: $\beta = 1.2\beta_0$ gives $R = 1.2 + 1/1.2$ and $M = \sqrt{1.2} - 1 = 0.095$; $\beta = 1.44\beta_0$ (size 20 % too large) gives $M = 0.200$. With $\beta = \beta_0$ but $\alpha$ off by 0.5, $M = 0.281$.

> **Think**: the beam size is exactly right ($\beta = \beta_0$), yet the mismatch factor is 0.28, worse than a size 20 % too large. What does that tell us?
>
> Answer: the size is only the projection of the ellipse in one direction. With $\alpha$ wrong the tilt is wrong, the ellipse is still elongated in phase space, and after about $45^\circ$ more of betatron phase (the phase of the transverse swing; a quarter of the envelope oscillation) its long axis turns into the $x$ direction and the projection grows noticeably. Matching by beam size alone is incomplete.

### The opening example

The example beam after a 0.1 m drift, sent straight into the FODO above, compared with the periodic solution at the F entrance: $M_x = 0.58$, $M_y = 1.23$. By the properties above the x envelope can reach 1.58 times the largest matched envelope (about 1.80 mm), about 2.85 mm; the linear preview gives 2.80 mm, in agreement. Without space charge the preview gives 2.77 mm: the bound is not quite reached because within the 6 periods the phase of the envelope oscillation does not fall exactly on the β maximum; the weak space charge of 0.18 mA adds only about 0.03 mm. The y mismatch is larger: the envelope reaches about 3.78 mm (bound $2.23 \times 1.80 \approx 4.0\ \mathrm{mm}$). Here is the mismatched line; note the irregular rise and fall of the envelope from period to period:

```avas-beamline
drift 0.1 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
```

### Mismatch in the animation and the lab

Now the puzzle of the opening animation can be solved. The animation's "mismatch" is a scale factor $s$ of the initial size: it multiplies β at the cell entrance by $s^2$ and keeps α. Scaling like that where $\alpha = 0$ would give $M = s - 1$; but at the F entrance the matched beam is itself opening up fast ($\alpha_x = -2.02$ at 6 T/m), and enlarging the size without changing the tilt makes the shape differ even more from what the channel wants: $s = 1.5$ gives $M_x = 1.31$ and $M_y = 0.65$. So the x envelope reaches at most $2.31$ times the matched value and at least $1/2.31$ of it: matched 0.88–1.80 mm, so mismatched at most about 4.17 mm and at least about 0.38 mm, and the animation's 4.08 mm and 0.39 mm lie just inside. In y the bound is $1.65 \times 1.82 \approx 3.0\ \mathrm{mm}$, and the animation gives 2.89 mm.

In the FODO lab below (gradient 5 T/m) set `mismatch` to 1 (matched), 1.2 and 1.44 and watch the envelope beat about the periodic envelope. At the cell entrance $\alpha \approx -1.76$, so the same scaling gives a mismatch larger than $s - 1$: $M \approx 0.44$ for $s = 1.2$ and $M \approx 1.0$ for $s = 1.44$.

```avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1.2
```

### Mismatch from small errors

When a matched beam passes a small focusing error $\Delta(1/f)$ (a quadrupole gradient error, for example, equivalent to an extra thin lens of focal length $f$), the beam size has no time to change ($\beta$ stays), only the tilt changes: $\alpha \to \alpha + \beta\,\Delta(1/f)$. Substituting into the definition gives $R = 2 + \beta^2\Delta(1/f)^2$, hence

$$
M \approx \frac{\beta\,\Delta(1/f)}{2}
$$

The mismatch is proportional to the local β: the same error does more harm where the beam is large. As a rule of thumb $M \lesssim 0.05$–$0.1$ counts as a good match, but what is acceptable depends on current and aperture margin (see [errors and correction](#errors)).

## With space charge and acceleration {#matching-space-charge}

### The matched beam changes with current

The space-charge force (the repulsion between particles) depends on the beam size itself: the thinner the beam, the closer the particles and the harder they push. So the matched solution has to be "self-consistent": the force computed from this size must keep exactly this size. For a continuous beam (the KV model, a uniform elliptical cylinder with edge radii $a$, $b$ in the two directions, $a = 2x_{\mathrm{rms}}$) the envelope equations are

$$
a'' + k_x(s)\,a - \frac{2K}{a+b} - \frac{\varepsilon_x^2}{a^3} = 0,\qquad
b'' + k_y(s)\,b - \frac{2K}{a+b} - \frac{\varepsilon_y^2}{b^3} = 0
$$

$a''$ says how fast the envelope bends along the line, and the other three terms are a three-way tug of war: $k_x a$ is the external focusing pulling the envelope in, $2K/(a+b)$ is space charge pushing out, and $\varepsilon^2/a^3$ is the emittance (the particles each have an angle, like the "temperature" of a gas) pushing out. Here $\varepsilon$ is the edge emittance ($4\varepsilon_{\mathrm{rms}}$) and $K$ the generalized perveance (a measure of the strength of space charge, see [space charge](#space-charge)). The matched beam is the solution of these equations with period $L_p$; it changes with current, and zero-current matched parameters applied to an intense beam produce a mismatch. Bunched beams in 3D are similar, and the longitudinal envelope must be matched too.

### With acceleration

Acceleration increases $\beta\gamma$, the geometric emittance shrinks adiabatically as $1/(\beta\gamma)$ ("adiabatically" meaning slowly enough for the beam to follow), and the periodic solution also changes slowly with energy. "Matched" then means that the envelope follows the slowly varying periodic solution rather than repeating exactly from period to period. The Twiss parameters in AVAS's `beam.txt` are the values **at the entrance** (β in mm/mrad, emittance normalized rms in π·mm·mrad), and the entrance rms size is $\sqrt{\beta\varepsilon_n/(\beta\gamma)}$.

## What mismatch does: envelope oscillations {#matching-envelope-modes}

The envelope of a mismatched beam oscillates about the matched envelope. When the envelopes of the two transverse directions oscillate together, there are two basic ways:

- the **breathing mode**: both directions swell together and shrink together, as if breathing ($\delta a = \delta b$, planes in phase);
- the **quadrupole mode**: one direction swells while the other shrinks, like squeezing a rubber ball ($\delta a = -\delta b$, planes in antiphase).

Take uniform focusing and a round beam, with a particle phase advance per metre $k_0$ at zero current, lowered to $k$ by space charge. The phase per metre of the two modes is (derived in the "going deeper" part with a school approximation formula)

$$
k_{\mathrm{b}}^2 = 2k_0^2 + 2k^2,\qquad k_{\mathrm{q}}^2 = k_0^2 + 3k^2 .
$$

At zero current $k = k_0$ and both modes are $2k_0$, exactly "the envelope twice as fast as the particles"; for very strong space charge $k \to 0$, $k_{\mathrm{b}} \to \sqrt2\,k_0$ and $k_{\mathrm{q}} \to k_0$. Axisymmetric bunched beams in 3D have three modes: the transverse quadrupole mode, and a high- and a low-frequency mode formed by coupling the transverse breathing with the longitudinal oscillation.

Example: the FODO above has a zero-current phase advance of $61.4^\circ$, both modes $122.9^\circ$ per period. At 10 mA DC, $\sigma/\sigma_0 = 0.876$, i.e. $k^2/k_0^2 = 0.767$, the breathing mode becomes $\sqrt{2 + 2\times 0.767} = 1.88$ times $k_0$ ($115.5^\circ$ per period) and the quadrupole mode $\sqrt{1 + 3\times 0.767} = 1.82$ times ($111.6^\circ$).

Physical consequences of mismatch:

1. The envelope oscillates at a frequency different from the particles', so the particles are periodically driven around the oscillating core; non-linear space-charge forces filament the phase space (the ellipse is drawn into thin threads that wind up), and the rms emittance finally grows. Reiser estimates the emittance growth from the "free energy" released when the mismatch energy is fully converted; the larger the mismatch and the stronger the space charge, the larger the growth.
2. In periodic focusing, an envelope instability occurs when an envelope mode approaches $180^\circ$ per period — the origin of the $\sigma_0 < 90^\circ$ rule in [lattice design](#lattice-design-phase-advance).
3. Parametric resonance between particles and the oscillating core drives some particles to large amplitudes and forms halo (see [rms matching and halo](#matching-halo)).

Raise the DC current `I` in the FODO lab to 10–20 mA: the zero-current periodic Twiss parameters are no longer matched and the envelope starts to beat, at the rhythm of the breathing and quadrupole modes above.

```avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=15 mismatch=1
```

## Matching sections: how many knobs {#matching-knobs}

### Number of constraints

Matching means making the beam's $(\alpha, \beta)$ equal the periodic solution at the target position. A full 6D match requires

| Plane | Constraints | Count |
|---|---|---|
| x | $\alpha_x$, $\beta_x$ | 2 |
| y | $\alpha_y$, $\beta_y$ | 2 |
| z | $\alpha_z$, $\beta_z$ | 2 |

The emittance is set by the beam itself and is not a matching variable (linear transport cannot change it). As with a system of equations, you need at least as many unknowns as equations, so at least as many **independent** knobs as constraints:

- Transversely, quadrupoles: at least 4 (4 gradients) to match $(\alpha_x, \beta_x, \alpha_y, \beta_y)$; in practice 4–6 are common, the extra freedom being used to limit the largest β, the gradients and the sensitivity to errors.
- A solenoid acts on both planes alike: one solenoid changes x and y together, so for a round beam there are only the two constraints $(\alpha, \beta)$, needing at least 2 solenoids; it cannot match x and y to different values.
- Longitudinally, bunchers: a buncher at $\varphi_s = -90^\circ$ adds no energy and only gives an energy offset proportional to the phase offset, a longitudinal thin lens; at least 2 bunchers (amplitudes as knobs) match $(\alpha_z, \beta_z)$. The first cavities of the downstream accelerating section can serve as well.

### An example: four quadrupoles

Let us solve the opening problem. Put a matching section in front of the FODO: a 0.1 m drift, four 0.1 m quadrupoles 0.2 m apart, and 0.3 m after the last one into the FODO. Require x and y $(\alpha, \beta)$ at the FODO entrance to equal the periodic solution: four equations, four unknown gradients. Solving with thick-lens matrices (from many random starting points, keeping the solution with the smallest largest gradient) gives $G_1 \ldots G_4 = -3.11$, $8.35$, $-7.43$, $3.96\ \mathrm{T/m}$.

```avas-beamline
drift 0.1 0.02 0
quad 0.1 0.02 0 -3.11
drift 0.2 0.02 0
quad 0.1 0.02 0 8.35
drift 0.2 0.02 0
quad 0.1 0.02 0 -7.43
drift 0.2 0.02 0
quad 0.1 0.02 0 3.96
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
```

After the matching section the envelope repeats every period: at zero current the rms $x$ at every F entrance lies between $1.739$ and $1.744\ \mathrm{mm}$, in agreement with the $1.74\ \mathrm{mm}$ of the periodic solution. With the space charge of the example beam's 0.18 mA, the same positions vary between $1.72$ and $1.80\ \mathrm{mm}$, a residual beating of about $\pm 2.5\ \%$. 0.18 mA is the average current, but the bunches are short: at the entrance a bunch carries $1.13\ \mathrm{pC}$ with an rms length of 1.32 mm, which for a Gaussian longitudinal profile is a peak current of about 6 mA, so space charge is not entirely negligible. This illustrates the rule below: with space charge, a zero-current match is only a starting point.

> **Think**: are these four gradients the only solution?
>
> Answer: no. The equations are non-linear, and different starting points lead to different solutions, some with larger gradients, some with a very large β inside the matching section. The one above has the smallest largest gradient. Choose a solution by looking at the envelope through the whole matching section, not only at the end parameters.

### Practical rules

- Keep β moderate inside the matching section: where β is large the beam is large and more sensitive to quadrupole offsets and gradient errors (mismatch $M \approx \beta\,\Delta(1/f)/2$).
- With space charge the matched parameters depend on the current; iterate with the real current, the zero-current result is only a starting point.
- The target of matching is the periodic Twiss parameters of the downstream period at its entrance, not some arbitrary "small spot".
- Non-linear problems may have several solutions; check that the envelope through the whole matching section is reasonable, not only the end parameters.

## Matching between sections {#matching-sections}

The general procedure for matching between sections:

1. Find the periodic matched parameters at the entrance of the downstream section (at zero current with the formulas of the "going deeper" part; with space charge from a self-consistent envelope or a multi-particle simulation).
2. Establish the Twiss parameters of the upstream beam at the entrance of the matching section (from an upstream simulation or a measurement).
3. Use the knobs of the matching section to transform 2 into 1 while respecting gradient and aperture limits.
4. Check with a multi-particle simulation that the envelope is periodic over several downstream periods and that the emittance does not grow.

A typical case is the MEBT between the RFQ exit and the first accelerating section (see [MEBT](#journey-mebt)): the beam leaving the RFQ is usually strongly converging or diverging transversely and asymmetric between the planes, and longitudinally bunchers are needed to control the bunch length. AVAS does not simulate the RFQ itself; its exit beam is handed to AVAS as Twiss parameters in `beam.txt` or as a particle file. If the two sections were given a continuous phase advance per metre in the [lattice design](#lattice-design-transitions), their periodic parameters are close and the matching section can be short or even omitted.

> **Note**: in an accelerating section the "periodic solution" of the first downstream period itself changes with energy, so strictly periodic matched parameters do not exist. Usually the first period is matched to the local periodic solution at its entrance energy and then checked by simulation.

## rms matching and halo {#matching-halo}

rms matching only makes the second moments (rms size, Twiss parameters) right. The tails of the real distribution are also affected by:

- **Particle–core resonance**: a mismatched core oscillates at the envelope breathing frequency, and a single particle crossing the core feels a periodic non-linear force; particles oscillating at about half the core frequency are resonantly driven to amplitudes far beyond the core. This is the same parametric resonance as pumping a swing by standing and squatting: the parameter (the length of the pendulum) changes twice per swing and the swing goes ever higher. The particle–core model (Gluckstern; Wangler and others) gives a maximum halo amplitude that grows with the mismatch, roughly logarithmically; as long as there is mismatch, an intense beam forms halo.
- **Non-linear external fields**: higher-order components in field maps and the fringe fields of solenoids and cavities distort the phase-space ellipse.
- **Initial distribution**: a Gaussian distribution has tails reaching several rms by itself, a KV distribution has none; the choice of KV / GS / PB / WB in AVAS's `distribution` affects loss estimates (see [initial distributions](#distributions)).

In AVAS the direct indicator of halo is the ratio of maximum to rms size (DataSet columns 22/24 against 16/18), together with the emittance growth along the line and the surviving particles (column 28). A good rms match with a steadily growing ratio of maximum size usually points to non-linearities or resonances.

## Think {#matching-think}

> **Think 1**: the animation enlarges the entering beam by only 1.5, yet its widest point becomes 2.2 times the matched one. Why not 1.5 times?
>
> Answer: at the F entrance the matched beam is opening up fast ($\alpha_x = -2.02$). Enlarging the size without changing the tilt moves the shape of the ellipse further off than the size alone; the mismatch factor is $M_x = 1.31$, and the envelope can reach $1 + M = 2.31$ times the matched value.

> **Think 2**: the matched beam has different sizes at the F and the D quadrupole centres. Is it still "matched" at D?
>
> Answer: yes. The periodic solution itself varies along the line; matched means the beam equals the local periodic solution at every point, not that it is equally wide everywhere. Judge a match by whether the envelope repeats every period.

> **Think 3**: the beam size measured at one place is exactly the design value. Does that show the beam is matched?
>
> Answer: no. One size constrains only β, not α. Sizes measured at two places about $90^\circ$ of phase advance apart (or a measurement of the whole ellipse) are needed to fix $(\alpha, \beta)$.

## Common misconceptions {#matching-misconceptions}

- **The right size is not a match**: with $\beta = \beta_0$ and $\alpha$ off by 0.5, $M = 0.28$. Matching must constrain both $\alpha$ and $\beta$.
- **"Enlarge by 1.5, and the envelope is 1.5 times larger"**: the extremes of the envelope are set by the mismatch factor and can far exceed the scale factor when the tilt is wrong (the animation's enlargement by 1.5 gives $M_x = 1.31$).
- **Matching to a "small spot"**: the target of matching is the periodic solution of the downstream period, not the smallest beam somewhere.
- **When the mismatch factor is conserved**: only in linear transport without space charge; with space charge or non-linearities mismatch turns into emittance growth and can no longer be fully described by $M$.
- **Zero-current matching for an intense beam**: space charge changes the periodic solution, so zero-current parameters are themselves a mismatch at finite current; the example above already shows about 2.5 % residual beating at 0.18 mA.
- **"Roundness" is not mismatch**: the AI assistant's metric `rms_mismatch_out` is $\lvert x_{\mathrm{rms}} - y_{\mathrm{rms}}\rvert$, a different thing from the Twiss mismatch factor of this chapter.
- **Definitions differ between codes**: the mismatch factor is written in different ways; check the definition before comparing results of different codes. This chapter uses Wangler's.

## Going deeper: how the matched beam is computed — the periodic solution {#matching-deep-periodic}

The main text said "once the transfer matrix of one period is known, the matched $(\alpha, \beta)$ can be read off". This section shows step by step how; the first five steps use only matrix multiplication and a trigonometric identity.

### Step 1: a small matrix records the ellipse

Average $x^2$, $x x'$ and $x'^2$ over all particles of the beam and arrange them in a 2×2 table, the beam's **sigma matrix**. Written with the Twiss parameters it is

$$
\Sigma = \begin{pmatrix} \langle x^2\rangle & \langle x x'\rangle \\ \langle x x'\rangle & \langle x'^2\rangle \end{pmatrix}
= \varepsilon \begin{pmatrix} \beta & -\alpha \\ -\alpha & \gamma \end{pmatrix},\qquad x_{\mathrm{rms}} = \sqrt{\beta\varepsilon}
$$

$\langle\ \rangle$ denotes the average. Since $\gamma = (1+\alpha^2)/\beta$, the determinant of this table is $\varepsilon^2(\beta\gamma - \alpha^2) = \varepsilon^2$: the area (emittance) is hidden in the determinant. The top-left entry gives the size, the sign of the off-diagonal entry the tilt: for $\alpha < 0$, $\langle x x'\rangle > 0$, particles far from the axis fly outward and the beam is opening up.

### Step 2: how one period changes this matrix

Each particle transforms as $X \to MX$ ($M$ being the transfer matrix of one period). The [drift](#drift-deep-matrix) chapter showed that the whole sigma matrix then transforms as

$$
\Sigma \to M\,\Sigma\,M\tp
$$

($M\tp$ is the transpose of $M$, rows and columns swapped).

### Step 3: the matching condition

Matched means the sigma matrix is unchanged after one period:

$$
M\,\Sigma\,M\tp = \Sigma .
$$

$\Sigma$ contains the two unknowns $\alpha$ and $\beta$ ($\gamma$ follows from them, $\varepsilon$ is fixed), and this matrix equation yields exactly two independent conditions.

### Step 4: rewrite M as "phase advance + Twiss"

Every 2×2 matrix of determinant 1 with $\lvert\tfrac12\Tr M\rvert < 1$ can be written as

$$
M = \begin{pmatrix} \cos\sigma + \alpha\sin\sigma & \beta\sin\sigma \\ -\gamma\sin\sigma & \cos\sigma - \alpha\sin\sigma \end{pmatrix}
$$

(check the determinant: $\cos^2\sigma - \alpha^2\sin^2\sigma + \beta\gamma\sin^2\sigma = \cos^2\sigma + \sin^2\sigma = 1$). The sum of the diagonal is $2\cos\sigma$, so $\sigma$ is the phase advance per period. Conversely, the periodic solution can be read off the four numbers of $M$:

$$
\beta = \frac{M_{12}}{\sin\sigma},\qquad \alpha = \frac{M_{11} - M_{22}}{2\sin\sigma},\qquad \gamma = -\frac{M_{21}}{\sin\sigma}
$$

The sign of $\sin\sigma$ is taken equal to that of $M_{12}$ so that $\beta > 0$. Putting this $(\alpha, \beta)$ into the $\Sigma$ of step 1 and multiplying out $M\Sigma M\tp$ entry by entry confirms that it equals $\Sigma$: a beam with these Twiss parameters has the same phase-space ellipse after one period, which is the **matched beam**.

### Step 5: check with numbers

The x-plane thick-lens matrix of the example FODO ($G = 5\ \mathrm{T/m}$) from one F entrance to the next is

$$
M_x = \begin{pmatrix} -1.0697 & 1.1033 \\ -2.8705 & 2.0258 \end{pmatrix}.
$$

$\tfrac12\Tr M_x = 0.478$, $\sigma = 61.4^\circ$, $\sin\sigma = 0.878$. Hence $\beta_x = 1.1033/0.878 = 1.256\ \mathrm{m}$, $\alpha_x = (-1.0697 - 2.0258)/(2 \times 0.878) = -1.762$, $\gamma_x = 2.8705/0.878 = 3.268\ \mathrm{m^{-1}}$; checking, $(1 + \alpha_x^2)/\beta_x = 4.105/1.256 = 3.268$. The matched rms size at the F entrance is $\sqrt{1.256 \times 2.41}\ \mathrm{mm} = 1.74\ \mathrm{mm}$, exactly the $1.739$–$1.744\ \mathrm{mm}$ at every F entrance after the matching section in the main text.

### Step 6: a different starting point, a different periodic solution

With another starting point (the F centre, say) the matrix of one period changes, and so does the $(\alpha, \beta)$ read off it: at the F centre of a FODO $\alpha = 0$ and $\beta$ is largest, at the D centre $\alpha = 0$ and $\beta$ is smallest. But the periodic solutions from different starting points are just the values of one and the same periodic β function at different positions, while the phase advance $\sigma$ does not depend on the starting point.

### For readers who know linear algebra

Let $S = \begin{pmatrix}\beta & -\alpha\\ -\alpha & \gamma\end{pmatrix}$, $\Omega = \begin{pmatrix}0 & 1\\ -1 & 0\end{pmatrix}$ and $J = S\Omega = \begin{pmatrix}\alpha & \beta\\ -\gamma & -\alpha\end{pmatrix}$; then $J^2 = -I$ and $M = I\cos\sigma + J\sin\sigma$. A 2×2 matrix with $\det M = 1$ satisfies $M\tp\Omega M = \Omega$, so $MSM\tp = MJ\Omega^{-1}M\tp = MJM^{-1}\Omega^{-1}$; $M$ commutes with $J$, so $MSM\tp = J\Omega^{-1} = S$, i.e. $\Sigma = \varepsilon S$ is the matched beam. Equivalently, if $v$ is the eigenvector of $M$ for $e^{i\sigma}$, the matched beam is proportional to $\mathrm{Re}(v v^\dagger)$; AVAS's `avas/dynamics/period.py` uses this construction for x–y coupled periods, scaled by the eigen-emittances. With space charge $M$ itself depends on $\Sigma$, and matching becomes a fixed-point problem.

## Going deeper: where the mismatch factor comes from {#matching-deep-mismatch}

The main text gave the formulas for $R$ and $M$ and said that $1 + M$ is "the excess in the least favourable direction". This section turns the matched ellipse into a circle, explains the formula with a quadratic equation, and then derives the range of the envelope beating and the small-error formula.

### Step 1: turn the matched ellipse into a circle

The boundary of the matched ellipse is $\gamma_0 x^2 + 2\alpha_0 x x' + \beta_0 x'^2 = \varepsilon$. Change to the coordinates

$$
X = \frac{x}{\sqrt{\beta_0}},\qquad X' = \frac{\alpha_0 x + \beta_0 x'}{\sqrt{\beta_0}};
$$

expanding $X^2 + X'^2$ gives $\gamma_0 x^2 + 2\alpha_0 x x' + \beta_0 x'^2$ (using $\gamma_0\beta_0 = 1 + \alpha_0^2$), so in the new coordinates the matched ellipse is a circle of radius $\sqrt{\varepsilon}$. The determinant of this conversion is 1, so areas are unchanged: it merely "squeezes the ellipse round", like stretching and shearing a rubber sheet with an ellipse printed on it until the ellipse is a circle.

### Step 2: what the mismatched ellipse becomes

In the new coordinates the mismatched beam is still an ellipse, again of area $\pi\varepsilon$ (same emittance). Let its half-axes be $\sqrt{\lambda_+\varepsilon}$ and $\sqrt{\lambda_-\varepsilon}$; equal areas require

$$
\lambda_+\lambda_- = 1 .
$$

$\lambda_+ \ge 1$ is the square of the factor by which the long axis is stretched.

### Step 3: R is the sum of the two λ

Convert the sigma matrix of the mismatched beam to the new coordinates as well (insert the conversion of step 1 into the $\Sigma$ of the first section and expand entry by entry); the sum of its diagonal divided by $\varepsilon$ is exactly

$$
\lambda_+ + \lambda_- = \beta\gamma_0 + \beta_0\gamma - 2\alpha\alpha_0 = R .
$$

(For an upright ellipse the diagonal entries are the squares of the half-axes, and the sum of the diagonal does not change when the coordinates are rotated.)

### Step 4: solve a quadratic equation

Two numbers with sum $R$ and product 1 are, by Vieta's formulas, the roots of $\lambda^2 - R\lambda + 1 = 0$:

$$
\lambda_\pm = \tfrac12\left(R \pm \sqrt{R^2-4}\right).
$$

The long half-axis is $\sqrt{\lambda_+}$ times the radius of the matched circle, which is the main text's $1 + M = \sqrt{\lambda_+}$. Both roots being positive real numbers requires $R \ge 2$; at $R = 2$ both roots are 1 and the ellipses coincide. Where $\alpha = \alpha_0 = 0$ and $\beta = s^2\beta_0$, $R = s^2 + 1/s^2$, $\lambda_+ = s^2$ and $M = s - 1$: for an upright ellipse that is only enlarged, the mismatch factor is just the enlargement.

### Step 5: why it is conserved, and why the envelope stays between 1/(1+M) and 1+M

In the new coordinates the matched circle returns to itself every period, so the effect of one period is a **rotation** about the origin by the angle $\sigma$ (a linear map with determinant 1 that takes a circle centred on the origin onto itself can only be a rotation). The mismatched ellipse therefore turns as a whole, keeping its shape, so $\lambda_\pm$ do not change and $M$ is conserved along the line. Its projection on the $X$ axis (the beam size) varies with the rotation between $\sqrt{\lambda_-\varepsilon}$ and $\sqrt{\lambda_+\varepsilon}$, while the projection of the matched circle is $\sqrt{\varepsilon}$, so the mismatched envelope stays between $1/(1+M)$ and $1+M$ times the matched envelope.

Numbers (the opening animation, $G = 6\ \mathrm{T/m}$): at the F entrance $\alpha_0 = -2.023$ and $\beta_0 = 1.193\ \mathrm{m}$; after enlarging by $s = 1.5$, $R = (1 + \alpha_0^2)(s^2 + 1/s^2) - 2\alpha_0^2 = 5.09 \times 2.694 - 8.18 = 5.54$, $\lambda_+ = 5.35$ and $1 + M = 2.31$.

### Step 6: the small-error formula

A matched beam passes a thin-lens error $\Delta(1/f)$: every particle's angle changes by $-x\,\Delta(1/f)$ while its position stays, so $\beta = \beta_0$ and $\alpha = \alpha_0 + \beta_0\Delta(1/f)$. Insert into $R$ (with $\gamma = (1 + \alpha^2)/\beta_0$):

$$
R = (1 + \alpha_0^2) + (1 + \alpha^2) - 2\alpha\alpha_0 = 2 + (\alpha - \alpha_0)^2 = 2 + \beta_0^2\Delta(1/f)^2 .
$$

Write $R = 2 + \epsilon$; for small $\epsilon$, $\sqrt{R^2 - 4} = \sqrt{4\epsilon + \epsilon^2} \approx 2\sqrt\epsilon$, $\lambda_+ \approx 1 + \sqrt\epsilon$ and $\sqrt{\lambda_+} \approx 1 + \sqrt\epsilon/2$, so $M \approx \beta_0\Delta(1/f)/2$.

### For readers who know linear algebra

With $S = \Sigma/\varepsilon$, $R = \Tr(S S_0^{-1})$, and $\lambda_\pm$ are the eigenvalues of $S S_0^{-1}$. In linear transport $S \to MSM\tp$ and $S_0 \to MS_0M\tp$, and $\Tr(MSM\tp (MS_0M\tp)^{-1}) = \Tr(M S S_0^{-1} M^{-1}) = \Tr(S S_0^{-1})$, so $R$ is invariant. The smallest ellipse of the matched shape enclosing the mismatched beam has area $\lambda_+\varepsilon = (1+M)^2\varepsilon$.

## Going deeper: the envelope equation and the two oscillation modes {#matching-deep-modes}

The main text gave the formulas for the breathing and the quadrupole mode. This section derives them from the envelope equation with nothing but a first-order approximation.

### Step 1: the three terms of the envelope equation

The KV envelope equation of the main text writes $a''$ (how fast the envelope bends) as the sum of three effects. In a uniformly focusing channel (focusing strength $k_x = k_y = k_0^2$):

$$
a'' = -k_0^2\,a + \frac{2K}{a+b} + \frac{\varepsilon^2}{a^3},\qquad
b'' = -k_0^2\,b + \frac{2K}{a+b} + \frac{\varepsilon^2}{b^3}.
$$

Here $k_0$ is a particle's phase advance per metre at zero current (in the smooth approximation of [lattice design](#lattice-design-deep-smooth), $k_0 = \sigma_0/L_p$).

### Step 2: the matched round beam

Both directions alike ($a = b = a_0$, $\varepsilon_x = \varepsilon_y = \varepsilon$) and the envelope not bending ($a'' = 0$):

$$
k_0^2\,a_0 = \frac{K}{a_0} + \frac{\varepsilon^2}{a_0^3}.
$$

With the particle phase advance per metre under space charge $k = \varepsilon/a_0^2$, dividing by $a_0$ gives $K/a_0^2 = k_0^2 - k^2$: space charge cancels exactly the part $k_0^2 - k^2$ of the focusing.

### Step 3: a small departure: first-order approximation

Let $a = a_0 + \delta a$ and $b = a_0 + \delta b$ with small $\delta a$, $\delta b$. With the school approximation $(1 + u)^n \approx 1 + nu$ (small $u$):

$$
\frac{\varepsilon^2}{a^3} \approx \frac{\varepsilon^2}{a_0^3}\left(1 - \frac{3\,\delta a}{a_0}\right),\qquad
\frac{2K}{a+b} \approx \frac{K}{a_0}\left(1 - \frac{\delta a + \delta b}{2a_0}\right).
$$

Insert into step 1; the matched parts cancel by step 2, leaving

$$
\delta a'' = -k_0^2\,\delta a - 3k^2\,\delta a - (k_0^2 - k^2)\,\frac{\delta a + \delta b}{2},
$$

and the equation for $\delta b$ with $a$ and $b$ swapped.

### Step 4: the quadrupole mode

For $\delta b = -\delta a$, $a + b$ does not change and neither does the space-charge term:

$$
\delta a'' = -(k_0^2 + 3k^2)\,\delta a .
$$

This is simple harmonic motion turning through $k_{\mathrm{q}} = \sqrt{k_0^2 + 3k^2}$ per metre.

### Step 5: the breathing mode

For $\delta b = \delta a$ the last term is $-(k_0^2 - k^2)\,\delta a$:

$$
\delta a'' = -(k_0^2 + 3k^2 + k_0^2 - k^2)\,\delta a = -(2k_0^2 + 2k^2)\,\delta a ,
$$

turning through $k_{\mathrm{b}} = \sqrt{2k_0^2 + 2k^2}$ per metre. At zero current $k = k_0$ and both are $2k_0$.

### Step 6: numbers and instability

The example FODO: $\sigma_0 = 61.4^\circ$, at 10 mA $k = 0.876\,k_0$, breathing mode $61.4^\circ \times 1.88 = 115.5^\circ$ per period, quadrupole mode $61.4^\circ \times 1.82 = 111.6^\circ$. In a real periodic channel the focusing changes once per period, and an envelope mode near $180^\circ$ per period is amplified by parametric resonance: the envelope instability of [lattice design](#lattice-design-phase-advance). For an envelope mode to reach $180^\circ$, the zero-current phase advance must exceed $90^\circ$: that is the $\sigma_0 < 90^\circ$ rule. The dynamics analysis of AVAS (`avas/dynamics/envelope.py`, the Jacobian of the envelope map about the matched beam) has checked this: a FODO with $\sigma_0 \approx 95^\circ$ is envelope-unstable when the current depresses σ to about $83^\circ$ (largest growth 1.061 per period), and stable for $\sigma_0 = 50^\circ$ or at very low or very high current.

### For readers who know calculus

Step 1 is the special case for KV beams of Sacherer's rms envelope equations, which hold in the rms sense for any elliptically symmetric distribution. Steps 3, 4 and 5 linearize about the matched solution, giving $\delta\boldsymbol a'' = -C\,\delta\boldsymbol a$ with a 2×2 coefficient matrix $C$ whose two eigenvectors are the in-phase and the antiphase motion. In a periodic channel the coefficients of the linearized envelope equation vary periodically with $s$, stability is decided by the eigenvalues of the linearized map over one period (the Floquet matrix), and unstable bands of half-integer parametric resonance appear where a mode's phase advance per period approaches $180^\circ$.

## In AVAS {#matching-in-avas}

### Setting the input beam

The Beam page sets `twissx`, `twissy`, `twissz` (α, β, ε) with the units listed below:

```avas-params
beam:twissx beam:twissy beam:twissz beam:distribution
```

β is in mm/mrad (numerically m/rad), ε is the normalized rms emittance (π·mm·mrad), twissz uses z (mm) and $z' = \Delta p/p$ (mrad). The example beam `twissx -0.65785 0.37743 0.13768` has an entrance $x_{\mathrm{rms}} = \sqrt{0.37743 \times 0.13768/0.05708} = 0.954\ \mathrm{mm}$.

### Interactive: the linear preview

While you drag quadrupole gradients, solenoid `Kb` and other parameters in the visual editor, the linear envelope preview updates at once (tick "Space charge in the preview" to include linear space charge). The visual test of a match: inside a periodic section the envelope repeats every period without beating from period to period. The preview is an approximate model (rms usually within 1–2 %, up to 13 % where multi-particle effects are strong) without losses or non-linearities; confirm with the engine in the end.

### Scans and optimization

- **Parameter scan page** (or the command line `avas scan --input DIR --target element --param parameter --values …`): runs the engine for a list of values of one parameter in copies under `Scans/` and compares the results.
- **The AI assistant's `optimize` tool**: varies parameters of several elements within bounds to minimise or maximise an objective. Metrics available with the preview engine are `rms_x_max`, `rms_y_max`, `rms_x_out`, `rms_y_out`, `energy_out` and `rms_mismatch_out`; the objective may be an expression such as `abs(rms_x_out-1.5)+abs(rms_y_out-1.5)`. Note that `rms_mismatch_out` is the end value of $\lvert x_{\mathrm{rms}} - y_{\mathrm{rms}}\rvert$ (roundness), not the Twiss mismatch factor of this chapter. The simulation engine adds metrics such as transmission and emittance growth. The result is proposed as an edit and is written to the lattice only after the user approves it.

### Size targets with adjust and diag_size

The static error-correction mechanism can also tune parameters to beam-size targets (details in [errors and correction](#errors-in-avas)):

```avas-params
adjust diag_size
```

- `adjust 0 v n min max first_step` goes in front of the element to tune; `v` is the element's v-th parameter (counted from 1 after the keyword: the gradient of `quad` is the 4th, `Kb` of `field` the 8th, `Ke` the 7th), knobs with the same `n` take the same value, `first_step = 1` starts the optimization from the current value.
- `diag_size 0 sx sy 0` gives the target **rms** size (mm) at its position. The current code pairs `adjust` and `diag_*` by their **first parameter** (equal numbers form one correction, done in order of the numbers); the manual calls this parameter "meaningless, write 0", and writing 0 everywhere makes one group.
- The optimizer is SLSQP (each function evaluation is one engine run); the objective is the mean of $(x_{\mathrm{rms}} - s_x)^2 + (y_{\mathrm{rms}} - s_y)^2$ (mm²) over the group's diagnostics, it stops below 0.05, after at most 100 iterations, with a finite-difference step of 0.1 (in the parameter's own unit).
- The mechanism runs only in a **static error study** (`error_type` `stat` or `stat_dyn` in `ini.ini`) and needs `err_step` and at least one error switch turned on (a static one in `stat` mode). The result goes to `Adjust_Datas_<group>_<run>.txt` and is **not written back to the lattice**.

> **Note**: a single `diag_size` constrains only the size, not α, so it cannot fix a match; size targets at two places about $90^\circ$ of phase advance apart constrain $(\alpha, \beta)$ indirectly. Borrowing this mechanism with zero-amplitude static errors for an error-free match is a use inferred from the current code and not systematically verified with the engine; check the results of every run.

### Checking

- Results page: rms envelope, Twiss α/β along the line, emittance; the "Phase advance" plot (which needs `lattice … lattice_end` period markers) gives the phase advance of the actual beam per period as $\int\dd z/\beta$.
- The Run page shows the rms envelope live; "Compare with the run before" overlays the previous run to compare before and after matching.

> **AVAS convention**: the old matching commands `MATCHING`, `SETTWISS`, `CIRCLE_MATCH` described in the manual belong to an old workflow, are not verified with the current interface and should not be used as current features.

## Further reading {#matching-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: mismatch factor, envelope modes, the particle–core model of halo.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.: envelope equations, free energy of mismatch and emittance growth; and M. Reiser (1991) on the free energy of non-stationary beams.
- F. J. Sacherer (1971): the rms envelope equations, which make rms matching valid for all elliptically symmetric distributions.
- J. Struckmeier, M. Reiser (1984): envelope oscillations and instabilities of mismatched beams in periodic channels.
- The complete list is in [references](#references).
