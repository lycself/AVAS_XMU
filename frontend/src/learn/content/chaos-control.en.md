# Chaos and controlling chaos

Take a system whose rules are completely fixed, with no randomness at all: knowing the present, you can compute the future step by step. Can its future be predicted? Intuition says "of course". Yet from the 1960s and 70s on, people found countless counterexamples: fixed rules whose outcome cannot, in practice, be predicted for long. This is called **chaos**.

Accelerator physicists care about chaos because particles in a circular accelerator go round billions of times, passing some nonlinear magnets every turn. Particles near the centre oscillate in an orderly way; a little farther out their motion may become chaotic, wander around phase space and finally hit the wall. How large the region is in which particles survive for a long time (the **dynamic aperture**) is one of the most important questions in designing a circular accelerator.

This chapter also presents a surprising idea: chaos can not only be avoided but **controlled**, and with very small nudges.

To be clear from the start: the two labs in this chapter are **textbook models that AVAS does not simulate**. AVAS is a linac code, in which the beam passes once; the Hénon map is a simplified model of "one turn" of a circular accelerator, and the logistic map is the simplest mathematical example of chaos. The phenomenon related to chaos in linacs (beam halo) is discussed separately below.

## Determined, yet unpredictable {#chaos-control-what}

First some everyday examples.

- **The double pendulum**: a pendulum with a second pendulum hanging from it. Release it twice from almost the same position: for the first few seconds the two motions are nearly the same, a little later they are completely different.
- **A post on a billiard table**: stand a round post in the middle of a billiard table. Each time the ball hits the post, a tiny difference in the angle of approach is magnified; after a few hits, where the ball goes depends entirely on that initial tiny difference.
- **The weather**: in the 1960s the meteorologist Lorenz computed with a simplified set of atmospheric equations. He typed in an intermediate result rounded to three decimals and restarted, and after a while the result was completely different from the original run. This is where the expression "butterfly effect" comes from.

What these examples share:

1. **Determined**: the same start always gives the same future. No dice are thrown.
2. **Sensitive to initial conditions**: tiny differences in the start grow larger and larger, roughly doubling every fixed stretch of time.
3. **Bounded**: the motion does not run off to infinity but keeps tossing about within a limited range. Differences are stretched, folded back and stretched again.

Point 2 is the key. The start can never be known perfectly (measurements have errors, computers have a finite number of digits), and the error grows by factors, so predictions are valid only for a limited time: making the start 10 times more precise buys only a fixed extra stretch of prediction, not 10 times as long.

## The simplest chaos: the logistic map {#chaos-control-logistic}

Chaos needs no complicated equations. Ecology has a simple model of a population: let $x$ be this year's population (as a fraction of the most the environment can hold, between 0 and 1); next year's population is

$$
x_{\text{next year}} = r\,x\,(1-x).
$$

$r\,x$ stands for "breeding": the more there are, the larger the next generation; $(1-x)$ stands for "crowding": when the habitat is nearly full, food runs short and growth is held back. $r$ is the breeding rate. A rule like this, "compute next year from this year", is called a **map**, and repeating it again and again is called **iteration**.

Work it out with a calculator for $r = 3.7$, starting at $x = 0.3$:

$$
0.3 \to 0.777 \to 0.6411 \to 0.8513 \to 0.4683 \to 0.9213 \to 0.2683 \to \dots
$$

No pattern; up and down. Now start again at $0.300001$, a difference of $10^{-6}$: at step 10 the two differ by about $5\times10^{-5}$, at step 20 by about $0.002$, at step 25 by about $0.01$, and at step 30 by 0.12; from then on the two sequences have nothing to do with each other.

The lab below draws this map. "Largest nudge allowed" is set to 0 for now, so **no control at all** is applied (control comes later); the lab also adds a tiny noise (0.0005) every step to imitate a real system.

```avas-lab
chaos-control dmax=0
```

Look at the bottom plot first (the **bifurcation diagram**). Its horizontal axis is $r$; for every $r$ the map is iterated a few hundred steps to "settle down", and the next 60 values are drawn as dots. Drag $r$, watch the time series at the top and compare with the bifurcation diagram:

- $r = 2.8$: it ends on one value, 0.643; the diagram shows a single line.
- $r = 3.2$: it jumps back and forth between two values, 0.513 and 0.800 (period 2); the line has split in two.
- $r = 3.5$: it cycles through four values (period 4). $r = 3.55$: eight values.
- Above about $r = 3.57$: it never repeats; the dots spread into a band. This is chaos. The default $r = 3.7$ is here.
- The chaotic region contains "windows": at $r = 3.83$ it again cycles through only three values (0.156, 0.505, 0.957).

The period doubles again and again, faster and faster, and finally turns into chaos; this route is called **period doubling**. It is not peculiar to this formula: experiments on fluids, circuits and lasers show the same pattern, and a "going deeper" section works out that the intervals between doublings shrink by the same ratio.

## Balanced on a needle: the unstable fixed point {#chaos-control-fixed-point}

Is there an $x$ that iteration leaves unchanged? Such a point is called a **fixed point**. Set $x = r\,x\,(1-x)$ and divide by $x$ ($x \ne 0$): $1 = r(1-x)$, so

$$
x^{*} = 1 - \frac{1}{r}.
$$

For $r = 3.7$, $x^{*} = 0.7297$ ("fixed point x*" in the lab's results). Yet the time series never stays there. Why?

Write $x$ as "the fixed point plus a small deviation": $x = x^{*} + e$. Substitute into the map, expand, and drop the tiny $e^2$ term (this takes only the expansion of a square; a "going deeper" section writes out every step). The result is

$$
e_{\text{next}} \approx (2 - r)\,e .
$$

For $r = 3.7$, $2 - r = -1.7$: every step the deviation becomes $-1.7$ times what it was, that is, it moves to the other side and grows by a factor of 1.7 ("slope there" in the lab's results). Starting from $e = 0.001$: $-0.0017$, $0.0029$, $-0.0049$, $0.0083$, … after 8 steps it is already 0.065.

It is like standing a pencil on its tip: it is an equilibrium, but the slightest tilt makes it fall. When $|2 - r| < 1$ deviations shrink and the fixed point is stable, which requires $1 < r < 3$; once $r$ exceeds 3 the fixed point becomes "a pencil on its tip" and the orbit has to go elsewhere: first it jumps between two values, further on it is chaotic.

The chaotic region holds infinitely many such unstable points (and unstable orbits of period 2, period 3, …). The chaotic orbit weaves between them, now and then coming close to one, lingering a few steps and being flung away again. Controlling chaos, described later, exploits exactly that.

## Chaos in accelerators: circular machines and sextupoles {#chaos-control-rings}

In a circular accelerator a particle flies round the ring while oscillating transversely about the design orbit (**betatron oscillation**, see [Linear optics](#linear-optics)). The number of oscillations per turn is the **tune**, written $\nu$; only its fractional part matters for resonances.

If the ring contains only linear magnets (dipoles, quadrupoles), the particle's point in phase space (position and angle, see [Phase space](#phase-space)) moves round an ellipse by a fixed angle $2\pi\nu$ every turn. In suitable coordinates (normalised coordinates) the ellipse becomes a circle, a turn is just "a rotation by an angle", and the particle's point stays on the same circle forever.

But rings also contain **sextupoles**. Quadrupoles focus particles of different energy differently (higher-energy particles are "stiffer" and are deflected less), just as a glass lens has different focal lengths for different colours of light; this effect is called **chromaticity**. A sextupole's field grows with position as $x^2$; placed where particles of different energy are separated (where there is dispersion, see [Dipole](#dipole)), it can correct the chromaticity. The price: a sextupole deflects a particle in proportion to $x^2$, which is **nonlinear**.

Written as the simplest possible model, each turn has two steps:

1. The linear part: a rotation by $2\pi\nu$ in normalised phase space;
2. A very thin sextupole: the angle coordinate $p$ increases by $x^2$ (with a suitable unit of length the coefficient becomes 1).

This is the **Hénon map** (studied by the French astronomer Hénon in 1969). The lab below takes 14 starting points on the $x$ axis (from 0.04 to "largest starting amplitude"), iterates each for a number of turns, and draws the position after every turn as a dot.

```avas-lab
henon
```

Default tune $\nu = 0.2114$, 600 turns. Look at:

- **Regular rings**: starting points near the centre draw smooth closed curves, more and more deformed farther out, somewhat like triangles. Although the particles are pushed nonlinearly, their motion is still regular and can be predicted for a long time.
- **Island chains**: the outermost 5 starting points (0.50 to 0.70) no longer draw large rings but 5 small loops arranged in a ring. The particle jumps from one "island" to the next every turn and returns near its original island after 5 turns. We counted the oscillations separately with the same map: the start at 0.04 oscillates 0.2113 times per turn, fewer at larger amplitudes, about 0.205 at 0.45, and from 0.50 on it is locked at exactly $1/5 = 0.2$. An island chain is particles captured by the resonance "repeat every 5 turns".
- **A chaotic sea**: drag "largest starting amplitude" to 0.82, and the outermost starting point no longer draws a curve but scatters into an irregular cloud of dots. Set the turns to 3000 and that starting point is lost ("starting points lost" becomes 1).

## Resonances, island chains and the chaotic sea {#chaos-control-islands}

Why do island chains appear? The key is **resonance**.

When pushing a swing, if you push at the same moment every time, the pushes add up and the swing goes higher and higher; if you push at random, sometimes pushing, sometimes pulling, the effects mostly cancel. A particle pushed by the sextupole once a turn is the same: if the tune makes the particle return to the same set of places in phase space every $m$ turns ($m\nu$ close to an integer), the sextupole's pushes repeat every $m$ turns and their effects accumulate; otherwise they cancel.

The resonance a sextupole drives most directly is $m = 3$ (the **third-order resonance**, $\nu$ close to $1/3$). Try it in the lab:

| Tune $\nu$ | Dynamic aperture | Lost of 14 starts |
|---|---|---|
| 0.2114 (default) | 0.82 | 0 |
| 0.31 | 0.59 | 2 |
| 0.32 | 0.24 | 8 |
| 0.333 | 0 | 14 |

The closer the tune to 1/3, the smaller the region that survives; right at 1/3 not a single start is kept. That is why circular accelerators are designed with tunes away from low-order resonances. Other resonances (such as $1/5$) come from combinations of several pushes and are weaker; they usually do not throw particles out directly but "prop open" a chain of islands in phase space.

Near the boundary between the islands, motion is particularly sensitive: a particle is just a hair from sliding from inside an island to outside. A very thin chaotic layer appears there. The stronger the nonlinearity, or the farther from the centre, the larger the islands of each resonance; when the islands of neighbouring resonances grow until they touch, the regular rings are torn apart and join into one large chaotic region: the **chaotic sea**. A particle in the chaotic sea is not lost at once; it can wander inside for a long time and then at some moment reach the outer edge of the sea and never come back.

## Dynamic aperture {#chaos-control-aperture}

The largest amplitude at which particles survive for a long time is called the **dynamic aperture**. It is not the same thing as the physical aperture of the vacuum pipe: even if the pipe is wide enough, particles with amplitudes beyond the dynamic aperture drift farther and farther because of their nonlinear motion and finally hit the wall.

The lab's "dynamic aperture" tries starting points on the $x$ axis from small to large and reports the last one that survives the given number of turns. With the default 600 turns it is 0.82; with 3000 turns it becomes 0.80. The more turns, the smaller the aperture: particles at the edge of the chaotic sea can survive hundreds of turns and escape only after one or two thousand.

How many turns does a real ring make? In a ring of 300 m circumference, particles flying at nearly the speed of light go round

$$
\frac{3\times10^{8}\ \mathrm{m/s}}{300\ \mathrm m} = 10^{6}\ \text{times per second},
$$

which is billions of turns per hour. No computer can track every particle for that many turns, so dynamic-aperture calculations usually track thousands of turns or more and are combined with theory and measurement.

## In a linac: the beam passes only once {#chaos-control-linac}

AVAS simulates linacs: the beam goes from one end to the other once, with no "billions of turns", and the main line of a proton linac usually has no sextupoles. Does chaos still matter here?

It does, in a different form. Focusing in a linac is mostly **periodic** (one FODO period after another, see [Lattice design](#lattice-design)), and each period a particle passes is like "one turn". A linac has tens to hundreds of periods, and the main nonlinear forces acting on the particles are:

- **Space charge**: the self-field of a non-uniform beam is nonlinear (see [Space charge](#space-charge)).
- **RF fields**: longitudinal motion is itself like a pendulum, with a force varying as the sine of the phase, which is nonlinear (see [Longitudinal motion](#longitudinal)); particles too far from the synchronous particle leave the "RF bucket".
- **Higher-order components in field maps**, and the fringe fields of solenoids and cavities.

**Beam halo** is one example. The envelope of a mismatched beam swells and shrinks ("breathes"); a particle passing through the core whose oscillation frequency is about half the breathing frequency gets pushed farther out again and again (a 2:1 parametric resonance, see [Matching](#matching) and [Space charge](#space-charge)). The **particle–core model**, widely used to study halo, records a particle's position once per breathing period, and the resulting picture looks much like the Hénon lab: regular rings in the middle, islands of the 2:1 resonance outside, and a chaotic layer near the boundary of the islands. The few particles carried outward are the halo; they hardly change the rms size but decide the beam losses.

So in a linac, "avoid resonances, match well" is the same idea as "choose a good tune" in a circular accelerator.

## Controlling chaos: the OGY method {#chaos-control-ogy}

In 1990 the physicists Ott, Grebogi and Yorke proposed that chaos need not be bad: it actually makes control **cheap**. Their method is named after their initials: the **OGY method**.

The idea has three steps:

1. A chaotic orbit hides infinitely many unstable fixed points and periodic orbits, such as $x^{*} = 0.7297$ above.
2. The chaotic orbit wanders everywhere and sooner or later comes near $x^{*}$. **Wait** for it to come close.
3. When it is close, change the parameter $r$ by a tiny amount so that the next step lands exactly on $x^{*}$. Do the same every step after that, and the orbit stays there.

It is like balancing a broom upright on your palm: when the broom leans a lot nobody can save it, but as long as it is nearly upright a small movement of the hand keeps it there. The difference is that a chaotic system comes "by itself" to nearly upright; all we have to do is wait.

Here is the same lab with its default settings: control switched on at step 100, and each nudge limited to $|\delta r| \le 0.1$.

```avas-lab
chaos-control
```

Look at:

- After the control is switched on, the orbit jumps around a few more steps, comes within 0.008 of $x^{*}$ at step 104 and is "caught" ("step at which the control catches the orbit"). That step uses the largest nudge, $\delta r = 0.069$.
- From then on the orbit stays near $x^{*}$, at an rms distance of only 0.0005, as large as the noise of each step. In the middle plot, the nudges $\delta r$ after the capture are never more than 0.016, typically about 0.004, roughly a thousandth of $r$. The dashed curve without control keeps jumping about.

Try a few more experiments:

- Reduce "largest nudge allowed" to 0.02: the control dares to act only when the orbit comes closer, so it waits longer and catches it only at step 377; at 0.01 it does not get a chance within 400 steps.
- Set $r$ to 3.5 (period 4, not chaotic): the orbit cycles through four values forever and never comes near $x^{*}$, so the control is never used. **It is precisely chaos's wandering that gives small nudges their chance.**
- Raise the noise to 0.005: now and then the noise kicks the orbit out of the controllable range, control is lost, and it has to wait for the next close approach (the rms distance from $x^{*}$ after capture becomes 0.09).

Soon after it was proposed, the OGY method was realised in experiments, for example on a vibrating magnetoelastic metal ribbon, lasers and electronic circuits. Its significance: no powerful actuator is needed to fight the system; it is enough to know the system's linear behaviour near the fixed point and nudge it gently at the right moment.

## No need to know where the target is: delayed feedback {#chaos-control-pyragas}

The OGY method needs to know where the fixed point $x^{*}$ is and what the slope near it is. In 1992 Pyragas proposed a simpler way: **compare now with one period ago**.

If the target is a fixed point (period 1), add every step a correction proportional to $(x_{\text{now}} - x_{\text{previous step}})$; if the target is an orbit of period $T$, compare now with $T$ ago. When the orbit really is on the target, now and one period ago are identical and the correction becomes zero by itself: the control does not change the target orbit, it only makes it stable. So the target need not be computed in advance; the system finds it by itself.

The method suits continuously running systems especially well (circuits, lasers): delay the measured signal by one period, subtract it from the present signal, multiply by a gain and feed it back. The price is that the gain must lie in a suitable range, and not every unstable orbit can be stabilised this way. This chapter's lab does not implement delayed feedback; a "going deeper" section shows with a quadratic equation that for the logistic map at $r = 3.7$ it stabilises the fixed point for gains between 0.35 and 1.

## How accelerators actually deal with chaos {#chaos-control-practice}

In everyday accelerator operation, chaos is dealt with mainly by **avoiding it in the design**, not by controlling it during operation:

- **Choosing the working point**: avoiding low-order resonances (circular accelerators), choosing a suitable phase advance per period and matching well (linacs, see [Lattice design](#lattice-design)).
- **Optimising the nonlinear elements**: circular accelerators divide the sextupoles into families and tune their strengths to correct the chromaticity while keeping the dynamic aperture as large as possible; this usually takes a lot of tracking and optimisation.
- **Measuring**: kicking the beam to different amplitudes with fast kicker magnets and seeing how long it survives, or analysing turn-by-turn BPM data, to check the actual dynamic aperture and resonances.
- **Feedback**: suppressing collective instabilities of the beam (for example bunch-by-bunch feedback in circular accelerators). That is a different problem from the chaos control of this chapter, but the same "measure and correct" idea (see [Feedback](#feedback)).

Controlling chaotic beam motion or halo with methods such as OGY or delayed feedback has been studied in the literature, mostly on simplified models; it is not a routine tool of accelerator operation. Its value lies in the way of thinking: a nonlinear system does not always need to be suppressed by force; sometimes cleverly using its own motion is cheaper.

## Think about it {#chaos-control-think}

> **Think about it 1**: If chaos is deterministic, why not just use a better computer with more decimals and keep predicting?
>
> Answer: that buys only a little. At $r = 3.7$ errors grow on average by about 1.42 per step, so each extra decimal (10 times more precision) buys only about 6.5 more steps. A start known to $10^{-6}$ gives about 30 steps of prediction; 60 steps would need about $10^{-12}$, and the start of a real system can never be measured that precisely.

> **Think about it 2**: Why is the OGY control never used at $r = 3.5$ but succeeds quickly at $r = 3.7$?
>
> Answer: OGY only "catches" the orbit with a small nudge when it comes near the fixed point. At $r = 3.5$ the orbit cycles through four fixed values and never approaches $x^{*} = 0.714$; when the motion is chaotic, the orbit wanders everywhere and sooner or later passes near the fixed point.

> **Think about it 3**: In the Hénon lab, raising the turns from 600 to 3000 changes the dynamic aperture from 0.82 to 0.80. A real ring makes billions of turns. What does this tell you?
>
> Answer: particles at the edge of the chaotic sea can survive a long time before escaping, so the more turns are tracked, the smaller the aperture appears. Tracking only a few hundred turns overestimates the dynamic aperture, which is why real calculations track as many turns as possible and keep a margin.

> **Think about it 4**: At $r = 3.83$ the logistic map cycles through only three values. Is that chaos?
>
> Answer: no, it is a **periodic window** inside the chaotic region. Change $r$ a little and it goes back to chaos through period doubling. The vertical blank stripes in the bifurcation diagram are such windows.

## Common misconceptions {#chaos-control-misconceptions}

- **"Chaos is randomness"**: a chaotic system has no random element; the same start always gives the same result. The unpredictability comes from sensitivity to initial conditions, not from throwing dice. The small noise in the lab is there only to make it more like a real system; without it (noise set to 0) the map is just as chaotic.
- **"Chaos means nothing can be predicted"**: the short term can be predicted, and the long-term statistics (such as the distribution of values) are determined too. Chaos also often coexists with regular motion: the rings near the centre in the Hénon lab are completely regular.
- **"The linear model shows no chaos, so there is none"**: a linear system cannot be chaotic at all; its small deviations either stay small or keep growing by the same factor until they run away, never being stretched and folded again and again within a limited range. AVAS's linear preview and linear model therefore cannot judge chaos; that needs tracking with nonlinearities (the multi-particle engine).
- **"AVAS gives slightly different results each run, so the beam is chaotic"**: these differences come from the random sampling of macro-particles (`randomseed 0` takes the seed from the clock; with a non-zero seed the new kernel is identical byte for byte), plus a multithreading data race in the original kernel (see [Multi-particle simulation](#simulation)); they say nothing about whether the beam's motion is chaotic.
- **"The dynamic aperture is the aperture of the vacuum pipe"**: the dynamic aperture is set by nonlinear motion and is often smaller than the physical aperture; even with a wide enough pipe, particles beyond it drift farther and farther and are finally lost.
- **"Controlling chaos takes a lot of force"**: the point of the OGY method is the opposite: let the chaotic orbit come near the target by itself and nudge it gently at that moment; the nudges in the lab are about a thousandth of $r$.

## Going deeper: period doubling and the Lyapunov exponent {#chaos-control-deep-logistic}

The main text showed in the lab period 1 → 2 → 4 → chaos and the exponential growth of errors. This section works them out step by step, using only the expansion of a square, quadratic equations, Vieta's formulas and logarithms.

### Step 1: the fixed point

Set $x = r\,x(1-x)$. $x = 0$ is one solution; for $x \ne 0$ divide both sides by $x$ to get $x^{*} = 1 - 1/r$. For $r = 3.7$: $x^{*} = 0.72973$.

### Step 2: deviations near the fixed point

Let $x = x^{*} + e$ and substitute into $f(x) = r\,x(1-x)$:

$$
f(x^{*} + e) = r\,(x^{*} + e)(1 - x^{*} - e) = r\,x^{*}(1-x^{*}) + r\,(1 - 2x^{*})\,e - r\,e^2 .
$$

The first term is just $x^{*}$ (the definition of the fixed point). When $e$ is small, $e^2$ is smaller still and can be dropped. Then substitute $x^{*} = 1 - 1/r$ into $1 - 2x^{*}$: $r(1 - 2x^{*}) = r - 2r + 2 = 2 - r$. So

$$
e_{\text{next}} \approx (2 - r)\,e .
$$

$2 - r$ is called the **slope** (multiplier) of the map at the fixed point.

### Step 3: when it is stable

Every step multiplies the deviation by $(2 - r)$. For $|2 - r| < 1$, i.e. $1 < r < 3$, deviations shrink and the fixed point is stable; for $r > 3$, $2 - r < -1$ and deviations grow with alternating sign. At $r = 3.7$ the multiplier is $-1.7$.

### Step 4: period 2

Just above $r = 3$ the orbit jumps between two values $a$ and $b$: $f(a) = b$ and $f(b) = a$, so both $a$ and $b$ satisfy $f(f(x)) = x$. This is an equation of fourth degree whose roots already include the two fixed points $0$ and $x^{*}$; dividing out those two factors leaves the quadratic equation

$$
r^2 x^2 - r(r+1)\,x + (r+1) = 0
$$

whose two roots are $a$ and $b$:

$$
x = \frac{(r+1) \pm \sqrt{(r+1)(r-3)}}{2r}.
$$

The $r - 3$ under the square root says period 2 exists only for $r > 3$. Check $r = 3.2$: $x = (4.2 \pm \sqrt{4.2\times0.2})/6.4 = (4.2 \pm 0.9165)/6.4$, i.e. 0.7995 and 0.5130, exactly the two values seen in the lab.

### Step 5: when period 2 becomes unstable

Over one round trip ($a \to b \to a$) a deviation is multiplied first by the slope at $a$, $r(1-2a)$, then by the slope at $b$, $r(1-2b)$. By Vieta's formulas $a + b = (r+1)/r$ and $ab = (r+1)/r^2$, so

$$
r^2(1-2a)(1-2b) = r^2\left[1 - 2(a+b) + 4ab\right] = -r^2 + 2r + 4 .
$$

Period 2 is stable while its absolute value is below 1. Setting it to $-1$: $r^2 - 2r - 5 = 0$, $r = 1 + \sqrt 6 \approx 3.449$. Beyond that value period 2, too, becomes a pencil on its tip and turns into period 4.

### Step 6: doubling faster and faster: Feigenbaum's constant

The same method (with a computer's help) finds where each doubling happens:

| Doubling | $r$ |
|---|---|
| 1 → 2 | 3 |
| 2 → 4 | 3.4495 |
| 4 → 8 | 3.5441 |
| 8 → 16 | 3.5644 |
| infinitely many doublings (chaos begins) | 3.5699 |

The ratio of successive intervals: $(3.4495 - 3)/(3.5441 - 3.4495) \approx 4.75$, $(3.5441 - 3.4495)/(3.5644 - 3.5441) \approx 4.66$, approaching $4.669\ldots$ further on. This number is **Feigenbaum's constant**. Remarkably, many entirely different systems (as long as the "map" looks like a parabola near its top) give the same number; this is the "same pattern" of the main text.

### Step 7: how much errors grow per step on average: the Lyapunov exponent

On a chaotic orbit each step multiplies a deviation by the local slope $r(1 - 2x)$; the slope varies with $x$, sometimes magnifying, sometimes shrinking. After $n$ steps the deviation is multiplied by the product of $n$ slopes. Taking logarithms turns the product into a sum; the average per step

$$
\lambda = \frac{1}{n}\sum_{k} \ln\left|r(1 - 2x_k)\right|
$$

is called the **Lyapunov exponent**. Computed over a hundred thousand steps of an orbit at $r = 3.7$ it is $\lambda \approx 0.353$, i.e. a deviation is multiplied on average by $\ee^{0.353} \approx 1.42$ per step. $\lambda > 0$ is the quantitative statement of "sensitive to initial conditions".

Check the numbers of the main text: growing from $10^{-6}$ to 0.1 is a factor $10^5$, which takes about $\ln(10^5)/0.353 \approx 33$ steps, in line with the observed "about 30 steps". Each extra decimal (a factor 10) buys only $\ln 10/0.353 \approx 6.5$ steps.

### For readers who know calculus

$\lambda = \lim_{n\to\infty}\frac1n\sum_k \ln|f'(x_k)|$. For $r = 4$ the substitution $x = \sin^2(\pi\theta)$ turns the map into $\theta \to 2\theta \pmod 1$, giving $\lambda = \ln 2$. Feigenbaum's constant is universal for unimodal maps with a quadratic maximum and follows from a renormalisation-group argument. Li and Yorke (1975) proved that an interval map with period 3 has every period ("period three implies chaos"), a special case of Sharkovskii's theorem; $r = 1 + \sqrt 8 \approx 3.828$ is where the period-3 window begins.

## Going deeper: the Hénon map, area preservation and the third-order resonance {#chaos-control-deep-henon}

The main text showed in the lab regular rings, island chains and a chaotic sea. This section explains step by step where the Hénon map comes from, why it differs from the logistic map, and why sextupoles particularly fear $\nu = 1/3$.

### Step 1: a linear turn is a rotation

In the [Phase space](#phase-space) and [Linear optics](#linear-optics) chapters, a particle's point in phase space moves through a stretch of linear elements according to the transfer matrix; in a periodic structure, in suitable coordinates (normalised coordinates $x$, $p$), the matrix of one turn is a rotation

$$
\begin{pmatrix} x \\ p \end{pmatrix} \to
\begin{pmatrix} \cos 2\pi\nu & \sin 2\pi\nu \\ -\sin 2\pi\nu & \cos 2\pi\nu \end{pmatrix}
\begin{pmatrix} x \\ p \end{pmatrix}.
$$

Writing $x = a\cos\varphi$ and $p = -a\sin\varphi$, the rotation simply increases the angle $\varphi$ by $2\pi\nu$ and leaves the radius $a$ (the amplitude) unchanged.

### Step 2: a thin sextupole

A sextupole's field $B_y$ in the horizontal plane is proportional to $x^2$, and a thin sextupole changes the particle's angle by an amount proportional to $x^2$. In normalised units, with a suitable unit of length, that is

$$
p \to p + x^2 .
$$

(The sign of the coefficient only decides which way the picture faces; replacing $x$ by $-x$ swaps it.) One push per turn followed by a rotation is the lab's map.

### Step 3: area is preserved, so there is no "attractor"

A push is a "shear": every point keeps its $x$, and $p$ is shifted by an amount that depends only on $x$. Pushing a deck of cards askew makes each card slide sideways, and the volume of the deck does not change; in the same way a shear leaves the area of any region in phase space unchanged. A rotation of course preserves area too. The Hénon map therefore **preserves area**, which is exactly Liouville's theorem from the [Phase space](#phase-space) chapter.

The logistic map, by contrast, squeezes and folds the interval, and orbits are "pulled" onto a point or a periodic orbit (such an end state is called an attractor). An area-preserving map cannot squeeze a region into a point, so it has no attractors: regular orbits stay on their rings forever, chaotic orbits wander in the chaotic sea, and particles near the centre are neither "pulled away" nor do they "return to the centre" by themselves.

### Step 4: how much one push changes the amplitude

Before the push the square of the amplitude is $a^2 = x^2 + p^2$. After the push $p$ becomes $p + x^2$:

$$
a'^2 = x^2 + (p + x^2)^2 = a^2 + 2p\,x^2 + x^4 .
$$

For small $x$, $x^4$ is much smaller than $2p\,x^2$; ignore it for now. Substituting $x = a\cos\varphi$ and $p = -a\sin\varphi$:

$$
\Delta(a^2) \approx -2a^3 \sin\varphi\cos^2\varphi .
$$

With the product-to-sum formulas of trigonometry, $\sin\varphi\cos^2\varphi = \frac14\left(\sin\varphi + \sin 3\varphi\right)$ (check it by substituting $\cos^2\varphi = (1 + \cos 2\varphi)/2$), so

$$
\Delta(a^2) \approx -\frac{a^3}{2}\left(\sin\varphi + \sin 3\varphi\right).
$$

### Step 5: when the pushes of many turns add up

Every turn $\varphi$ increases by $2\pi\nu$. Adding the pushes of many turns, the $\sin\varphi$ term gives $\sin\varphi_0 + \sin(\varphi_0 + 2\pi\nu) + \sin(\varphi_0 + 4\pi\nu) + \dots$; these values are positive and negative and their sum stays small, unless $\nu$ is an integer (the push comes at the same angle every turn). The $\sin 3\varphi$ term advances by $3\times 2\pi\nu$ per turn and likewise cancels unless $3\nu$ is an integer.

So a single sextupole directly drives the **integer resonance** and the **third-order resonance** $\nu = 1/3$, $2/3$: at these tunes the pushes add up turn after turn like pushing a swing in time, and the amplitude grows and grows. The lab confirms it: at $\nu = 0.31$, $0.32$ and $0.333$ the dynamic aperture is 0.59, 0.24 and 0.

### Step 6: larger amplitudes oscillate more slowly, so island chains appear

Including the higher-order terms dropped in step 4 and combinations of several pushes brings in terms such as $\sin 5\varphi$, belonging to resonances with $5\nu$ an integer, and so on. They are weaker, but there is one more effect: the number of oscillations depends on the amplitude. We counted it with the lab's default parameters: the start at 0.04 oscillates 0.2113 times per turn, the start at 0.45 about 0.205. At some larger amplitude the number of oscillations equals exactly $1/5$, the particles there are locked by the fifth-order resonance, and five islands form: this is what happens to the starts from 0.50 to 0.70. Elsewhere the resonance only twists the rings slightly; at the amplitude where "the oscillations are exactly in step" it props open a chain of islands.

### Step 7: islands join into a sea

The stronger the nonlinearity (or the larger the amplitude), the wider each chain of islands. When two neighbouring chains become wide enough to touch, no regular ring is left between them to hold the particles back; the chaotic layers join, and a particle can drift all the way from inside to outside. In the lab, the starts near 0.80–0.82 lie in such a layer: still there after 600 turns, lost within 3000.

### For readers who know Hamiltonian mechanics

Area-preserving maps correspond to periodically driven Hamiltonian systems with one degree of freedom. The KAM theorem guarantees that for small enough perturbations most invariant tori (the regular rings) survive; rational tori break up according to the Poincaré–Birkhoff theorem into alternating chains of elliptic points (island centres) and hyperbolic points, whose stable and unstable manifolds intersect transversally and create chaotic layers; Chirikov's resonance-overlap criterion estimates the onset of global chaos. When the two transverse planes are coupled (four-dimensional maps), tori no longer divide phase space and Arnold diffusion becomes possible. In accelerators, normal forms and resonance driving terms are used to analyse the effect of sextupoles, and frequency map analysis (Laskar) to identify resonances in tracking or measured data.

## Going deeper: the formulas of OGY and delayed feedback {#chaos-control-deep-control}

The main text said the OGY method "changes $r$ a little so that the next step lands on the fixed point". This section works out by how much, why one has to wait and what noise does, and finally analyses delayed feedback with a quadratic equation.

### Step 1: how much the next step changes when the parameter changes a little

Changing the parameter from $r$ to $r + \delta r$ turns the map into $(r + \delta r)\,x(1-x) = r\,x(1-x) + \delta r\cdot x(1-x)$. With $x$ near $x^{*}$ the second term is about $\delta r\cdot x^{*}(1-x^{*})$. Using step 2 of the first deeper section,

$$
e_{\text{next}} \approx (2 - r)\,e + x^{*}(1 - x^{*})\,\delta r .
$$

Of the two terms on the right, one comes from the deviation and one from our nudge.

### Step 2: how hard to nudge

To land the next step exactly on $x^{*}$, set the right-hand side to 0:

$$
\delta r = \frac{(r - 2)\,e}{x^{*}(1 - x^{*})} .
$$

For $r = 3.7$: $x^{*}(1 - x^{*}) = 0.7297\times0.2703 = 0.1972$, so $\delta r = 1.7e/0.1972 = 8.62\,e$. The largest nudge allowed is 0.1, so the control acts only when $|e| < 0.1/8.62 = 0.0116$. In the lab at step 104, $e = 0.0080$ and $\delta r = 8.62\times0.0080 = 0.069$, exactly "largest nudge used".

### Step 3: how long to wait

Without control, about 4.75 % of the steps of the chaotic orbit fall within the window $|e| < 0.0116$ (counted from the lab's time series), so on average there is a chance about every 21 steps. In the lab it came 4 steps after switching on, a stroke of luck. With the allowed nudge reduced to 0.02 the window shrinks to $|e| < 0.0023$, chances are much rarer, and in the lab it waits until step 377. The smaller the window, the longer the average wait: that is the price of "nudging gently".

### Step 4: noise

After capture each step should land exactly on $x^{*}$, but the noise kicks it away by a little $\eta$, so the next step needs a nudge $\delta r \approx 8.62\,\eta$. With noise 0.0005, $\delta r$ is about 0.0043, in line with the typical nudge of 0.004 after capture in the lab, and the rms distance of the orbit from $x^{*}$ is just the noise, 0.0005. With noise 0.005 the needed nudge is about 0.043, and once in a while a large noise kick asks for more than the limit of 0.1; control is then lost until the next close approach.

### Step 5: why delayed feedback works

Delayed feedback changes the map to $x_{n+1} = f(x_n) + K(x_n - x_{n-1})$. Near the fixed point, written as in step 2 ($\lambda = 2 - r$ is the slope),

$$
e_{n+1} = \lambda\,e_n + K\,(e_n - e_{n-1}) .
$$

As with the loop delay of the [Control systems](#control-systems) chapter, try $e_n = z^n$, which gives the quadratic equation

$$
z^2 - (\lambda + K)\,z + K = 0 .
$$

For the deviation to decay, both roots must be smaller than 1 in size. For $r = 3.7$, $\lambda = -1.7$:

- The product of the roots is $K$ (Vieta's formulas), so at least $K < 1$;
- When $K$ is too small, one root moves beyond $-1$. On the boundary $z = -1$ is a root: $1 + (\lambda + K) + K = 0$, i.e. $1 - 1.7 + 2K = 0$, $K = 0.35$.

So the fixed point is stabilised for $0.35 < K < 1$. For example $K = 0.6$: the two roots are a complex-conjugate pair of size $\sqrt{0.6} = 0.775$, and the deviation shrinks to about three quarters each step. We checked it on a computer: for $K = 0.4$ to $0.95$ the orbit settles on $x^{*}$, for $K = 0.2$ it does not.

This also explains "not every orbit can be stabilised": if the slope $\lambda$ is a positive number larger than 1, substituting $z = 1$ into the left-hand side gives $1 - \lambda - K + K = 1 - \lambda < 0$, while for large $z$ the left-hand side is positive, so there is always a real root larger than 1 and no $K$ works. The fixed point of the logistic map has a negative slope, which is what gives it this chance.

### For readers who know dynamical systems

The general form of OGY: linearise on a Poincaré section of the unstable periodic orbit, $\xi_{n+1} \approx J\xi_n + \boldsymbol g\,\delta p$, and choose $\delta p$ to put $\xi_{n+1}$ on the stable manifold (zeroing the component along the unstable eigendirection), a pole placement valid only in a small neighbourhood. Pyragas's continuous form is $u(t) = K[y(t-\tau) - y(t)]$ with $\tau$ the period of the target orbit, and the characteristic equation contains $\ee^{-s\tau}$; the obstacle for $\lambda > 1$ above corresponds to the limitation "an odd number of real Floquet multipliers larger than 1" for non-autonomous systems (which does not hold for autonomous ones), and extended delayed feedback (Socolar, Sukow, Gauthier, 1994) widens the range of orbits that can be stabilised.

## In AVAS {#chaos-control-in-avas}

### The labs of this chapter are textbook models

The two labs `henon` and `chaos-control` are in `avas/learn/control.py`:

- `henon`: the Hénon map (a rotation by $2\pi\nu$ per turn plus one thin sextupole kick $p \to p + x^2$, normalised units), with 14 starting points on the $x$ axis; the dynamic aperture is the largest of 120 starting points between 0.01 and 1.2, tried one by one, that survives the given number of turns; a coordinate beyond 10 counts as lost.
- `chaos-control`: the logistic map with OGY control. The two time series (with and without control) use the same noise; the bifurcation diagram has no noise.

Neither is an AVAS lattice, and neither calls the engine or the linear model. **AVAS does not simulate circular accelerators**: there is no turn-by-turn tracking, and the lattice has no sextupole element.

### Where the nonlinearity is in AVAS

- The **multi-particle engine** includes nonlinear forces: space charge computed by the particle-in-cell method (nonlinear for a non-uniform beam) and three-dimensional field maps (the actual distribution of RF and static magnetic fields, including their higher-order content). Its results therefore show emittance growth, halo and losses. Indicators: a steadily growing ratio of the maximum size (columns 22/24/26 of DataSet.txt) to the rms size (columns 16/18/20) usually means halo is developing; the number of surviving macro-particles is column 28.
- The **linear preview, the linear model and `avas/dynamics/`** are linear; they cannot show chaos or give a dynamic aperture. The research page's "stability chart" gives the linear phase advances and envelope stability of one period (for example the 90° stop band), not a dynamic aperture.
- To explore roughly with the engine "at which parameters particles start to be lost", run an engine scan on the research page (element parameters or beam parameters) and watch the number of surviving particles and the max/rms size ratio.
- The slight differences between two AVAS runs come from the sampling of macro-particles (plus a multithreading data race in the original kernel), not from chaotic beam motion; compare with tolerances, and note that the research layer fixes the random seed by default.

## Further reading {#chaos-control-reading}

- S. H. Strogatz, *Nonlinear Dynamics and Chaos*, 2nd ed., Westview Press (2015; reissued by CRC Press): an introductory textbook on the logistic map, period doubling and Lyapunov exponents.
- R. M. May, "Simple mathematical models with very complicated dynamics", *Nature* 261, 459 (1976).
- M. Hénon, "Numerical study of quadratic area-preserving mappings", *Q. Appl. Math.* 27, 291 (1969).
- E. Ott, C. Grebogi, J. A. Yorke, "Controlling chaos", *Phys. Rev. Lett.* 64, 1196 (1990).
- K. Pyragas, "Continuous control of chaos by self-controlling feedback", *Phys. Lett. A* 170, 421 (1992).
- B. V. Chirikov, "A universal instability of many-dimensional oscillator systems", *Phys. Rep.* 52, 263 (1979): the resonance-overlap criterion.
- S. Y. Lee, *Accelerator Physics*, World Scientific: the chapters on nonlinear resonances and dynamic aperture.
- A. Wolski, *Beam Dynamics in High Energy Particle Accelerators*, Imperial College Press (2014): nonlinear dynamics and dynamic aperture.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH (2008): beam halo and the particle–core model; R. L. Gluckstern, *Phys. Rev. Lett.* 73, 1247 (1994).
- Related chapters of this primer: [Phase space](#phase-space), [Linear optics](#linear-optics), [Space charge](#space-charge), [Matching](#matching), [Feedback](#feedback), [Control systems](#control-systems); the full bibliography is in [References](#references).
