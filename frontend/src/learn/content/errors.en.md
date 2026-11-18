# Errors and correction

In the chapters so far every magnet sat exactly where the drawing put it, and every cavity had exactly its set voltage and phase. A real accelerator cannot do this: a quadrupole installed 0.1 mm off (about the thickness of a hair) is quite ordinary, magnet power supplies fluctuate slightly, and the voltage and phase of RF cavities jitter a little. All these small departures from the design are called **errors**.

Are errors this small worth a whole chapter? They are. A FODO channel of ten periods has 20 quadrupoles; if each of them is installed 0.1 mm off at random, the beam centre at the exit is off by about 1 mm, more than half the rms size of the beam itself there. Why are small errors amplified? How large an error can an accelerator tolerate? How do we answer these questions with simulations? This chapter starts from scratch.

First, the animation. This is the 6-period FODO channel of the *Orbit correction* chapter; all 12 quadrupoles are slightly misaligned at random (set here to rms 0.1 mm). The solid line is the beam centre, the orange dots are particles and the grey diamonds are the monitors that measure the beam position (BPMs).

```avas-motion
orbit err=0.1
```

Keep "Singular values used" at 0, i.e. no correction at all, and look only at the errors themselves. Watch three things:

- **The beam centre no longer follows the centre line**; it swings left and right, up and down. With the default error set, the rms of the beam centre at the BPMs is about 0.26 mm in $x$ and 0.48 mm in $y$, and the largest $x$ along the line is 0.48 mm.
- **The deviation is proportional to the error**: change "Quadrupole offsets" from 0.1 to 0.2 mm and every deviation exactly doubles (0.52 mm in $x$ at the BPMs).
- **Another error set gives another result**: change "Error set" from 1 to 2. The offsets are still rms 0.1 mm, but the rms $x$ at the BPMs becomes 0.48 mm, almost twice as much. Computing once is therefore not enough; we have to compute many times and look at the statistics. That is the **error study** discussed below.

How BPMs and steerers pull the beam back has been covered in detail in the [Orbit correction](#orbit-correction) chapter (that is the slider in the animation). This chapter is about the errors themselves: where they come from, what they do, how to estimate them and how to study them statistically with AVAS.

The chapter uses the β function and phase advance of [linear optics](#linear-optics), the mismatch of [matching](#matching) and the synchronous phase of [longitudinal dynamics](#longitudinal). Don't worry if these are unfamiliar: the main text first explains everything in the most intuitive terms. The estimates use the FODO of [lattice design](#lattice-design-periods) (gradient $G = 5\ \mathrm{T/m}$, quadrupole length $L_q = 0.1\ \mathrm{m}$, drift length $L_d = 0.3\ \mathrm{m}$, the 1.527 MeV example proton beam). Steerers themselves are covered in [steerers](#steerer).

## Sources and kinds of errors {#errors-types}

The errors of a real accelerator fall roughly into five kinds:

| Kind | In plain words | Examples | Columns in AVAS |
|---|---|---|---|
| Element position | installed off-centre | transverse offsets $dx$, $dy$, longitudinal offset $dz$ | `dx`, `dy` (mm), `dz` (mm) |
| Element orientation | installed crooked | rotations about the x, y, z axes | `dφx`, `dφy`, `dφz` (°) |
| Static field | a magnet slightly stronger or weaker than set | calibration errors of quadrupole gradients, solenoid fields | `dG` (%) |
| RF amplitude and phase | the cavity's voltage and "beat" are off | setting errors and jitter of cavity voltage and phase | `ke/kb` (%), `φs` (°) |
| Input beam | the incoming beam is already off | offsets of entrance position, angle, energy, phase, emittance, current | `dx dy dφ dx' dy' dE dεx dεy dεz … dIb` |

The "columns in AVAS" are the names used in error commands; they are explained in "In AVAS" at the end of the chapter.

By **how they change in time**, errors also fall into two kinds. The distinction matters a great deal because it decides whether an error can be compensated:

- **Static errors** stay constant during a run (a period of time). Think of a picture hanging crooked on the wall: once crooked, it stays crooked. Installation offsets and magnet calibration errors are of this kind. They can be measured with beam diagnostics and compensated with steerers, by adjusting cavity phases and so on — like straightening the picture once.
- **Dynamic errors** change in time. Think of a hand holding a glass of water and trembling slightly. Jitter of RF amplitude and phase, mechanical vibration and power-supply ripple are of this kind. They change faster than the correction loop, or cannot be measured at all, and usually cannot be compensated; they are controlled only by making the tolerances (the allowed error ranges) tight enough.

An **error study** draws many random sets of errors according to their statistical distribution (each set is called a "seed" or a sample), tracks the beam once for each set, and collects the distributions of figures such as beam loss, emittance growth and centroid offset.

## What a misaligned quadrupole does {#errors-effects}

This section estimates the effects of the various errors with school physics, to judge their size. The estimates are based on linear optics and ignore space charge; they serve to check whether simulation results are reasonable.

### A misalignment is a "kick"

Recall the [Quadrupole](#quadrupole) chapter: a quadrupole acts like a lens. There is no field at its centre, and the farther a particle is from the centre, the larger the angle by which it is pushed back: angle = distance from the centre ÷ focal length $f$.

Now the whole quadrupole is installed $d$ too far to the right. A beam flying along the centre line of the pipe is $d$ to the **left** of the magnet's centre, so it is pushed towards the magnet's centre (to the right) by the angle

$$
\Delta x' = \frac{d}{f}.
$$

So **a misaligned quadrupole = a well-aligned quadrupole + a small "kick"**. The example quadrupole has $f = 0.357\ \mathrm{m}$; a 0.1 mm offset gives a kick of $0.1/0.357 \approx 0.28\ \mathrm{mrad}$ (milliradians, thousandths of a radian).

### How one kick travels downstream

The kicked beam centre behaves like a swing that was pushed: it does not run off to one side for ever, because the quadrupoles downstream push it back, so the beam centre starts swinging about the centre line. This swinging is called a **betatron oscillation** (transverse oscillation).

How large is the swing? In the simplest case only a drift follows the kick: offset = angle × distance. A kick of 0.28 mrad moves the beam centre by 0.28 mm after 1 m (remember "1 mrad over 1 m is 1 mm"). With quadrupoles, the offset also depends on two things: how wide the beam is where the kick happens (the same kick matters more where the beam is wide), and which stage of the swing has been reached (at the far end of the swing, or just crossing the centre line). The exact formula is in "Going deeper: how errors propagate and add up" below.

### Many random kicks: a random walk

When 20 quadrupoles each give a kick, is the result 20 times that of one? No, only about $\sqrt{20} \approx 4.5$ times.

Think of tossing a coin: heads, you step forward; tails, you step back. After 20 tosses you are usually not 20 steps away, because steps forward and back partly cancel; on average you end up about $\sqrt{20}$ steps from the start. The quadrupole offsets have random directions (some left, some right), and the stages of the swing are random too, so their kicks also partly cancel. This is a **random walk**.

Applied to the example FODO: 20 quadrupoles, each offset being a random number of rms 0.1 mm, give an rms beam-centre offset at the exit of about $0.94\ \mathrm{mm}$; letting a computer actually draw 20000 sets of errors and compute each gives $0.96\ \mathrm{mm}$ — a very good estimate. The rms size of the beam itself at the same place is about $1.74\ \mathrm{mm}$: 0.1 mm of installation error moves the beam centre by more than half the beam size. That is why long beam lines need orbit correction. The centroid offset grows with the square root $\sqrt N$ of the number of quadrupoles; when the beam is accelerated the offset slowly shrinks (as $1/\sqrt{\beta\gamma}$, and $\beta\gamma$ grows with energy).

### Installed crooked: the two planes get mixed

When a quadrupole is rotated by a small angle $\varphi_z$ about the direction of motion (the z axis), $x$ and $y$ no longer mind their own business: the magnet mixes the horizontal and vertical motion, so a horizontal deviation turns into a vertical one and vice versa. This is called **coupling**. Coupling increases the emittance seen in $x$ or $y$ alone. Rotations about the x and y axes tilt the magnet so that one end is higher than the other; each end is a little off, with an effect similar to but smaller than an offset. A solenoid couples the two planes anyway, and its offsets and rotations also deflect the beam.

### Field strength off: focusing stronger or weaker

A quadrupole gradient 1 % above its setting does not push the beam centre aside (the beam is still at the magnet's centre, where there is no field), but the lens becomes a little stronger. There are two consequences:

- **The beam shape no longer fits the channel** (mismatch): the beam size gets extra ripples. A 1 % gradient error in one F quadrupole gives a mismatch factor of about 0.019 (for what the mismatch factor means see [matching](#matching-mismatch-factor)), which is small; but the errors of many magnets add up.
- **The oscillation of each period gets a little faster** (larger phase advance): with all quadrupoles 1 % too strong, the phase advance per period grows by about 0.7°.

Check it in the FODO lab below: the gradient `G` is set to 5.05, 1 % above 5 (as if all quadrupoles had a 1 % gradient error). The lab gives a phase advance per period of $62.12^\circ$; set `G` back to 5 and it is $61.44^\circ$, only $0.68^\circ$ less, in line with the estimate. β at the period entrance changes from $1.256$ to $1.251\ \mathrm{m}$, also very little.

```avas-lab
fodo Lq=0.1 G=5.05 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
```

### RF cavity off: a little more or less energy

The energy a cavity gives a particle is the charge times the "voltage the particle feels". With the cavity voltage 1 % too high the energy gain is also about 1 % higher; with the cavity phase a little off, the particle meets a somewhat stronger or weaker field. Estimating with the thin-gap formula of [Longitudinal dynamics](#longitudinal) for the example HWR (synchronous phase $-33^\circ$, energy gain 0.339 MeV): a 1 % amplitude error changes the gain by about $3.4\ \mathrm{keV}$, a $1^\circ$ phase error by about $3.8\ \mathrm{keV}$. Both are small compared with the energy range the longitudinal motion can accept (about $\pm 0.09\ \mathrm{MeV}$, see [lattice design](#lattice-design-longitudinal)).

Low-energy cavities have a catch, though: computed directly with the AVAS linear preview, a phase error has 1.2 to 1.6 times the effect of this estimate, and how much more also depends on "which phase" is wrong. The reasons and numbers are in "Going deeper: phase errors of low-energy cavities" below.

With longitudinal focusing, energy and phase errors make the bunch centre swing back and forth about the synchronous particle (a longitudinal oscillation). When the errors are too large, particles leave the longitudinally stable region, are lost longitudinally first and then hit the wall transversely because their energy is wrong. At low energy the speed depends strongly on the energy: a slightly wrong energy means a slightly wrong arrival time at the next cavity, which is a phase offset. Chains of independently phased cavities are especially sensitive to this.

Try it in the RF cavity lab: the synchronous phase is set to $-32^\circ$; change it back to $-33^\circ$, or change `Ke` by 1 % (1.36 → 1.3736), and compare the change of the exit energy. The lab also shows the entrance phase $\varphi_0$ it solved for: when $\varphi_s$ changes by $1^\circ$, it changes by about $1.3^\circ$.

```avas-lab
rf-cavity phase=-32 Ke=1.36 W=1.527
```

### The incoming beam is already off

Offsets of position and angle at the entrance cause transverse swings of the beam centre, just as quadrupole offsets do; offsets of energy and phase cause longitudinal swings; offsets of emittance and current show up mainly as mismatch (the beam shape not fitting the channel).

## Static and dynamic errors {#errors-static-dynamic}

The two kinds of error are handled differently, and the key is whether they can be measured and compensated.

A static error stays constant during a run: first measure the beam centre with BPMs and other beam diagnostics, then adjust steerers and cavity phases to pull it back; what remains is the residual after correction. A dynamic error changes too fast: by the time it is measured it has changed, so it simply adds to the corrected beam.

An error budget is therefore usually made in two steps: first apply only static errors and correct them, which gives the "corrected" state; then add dynamic errors on top of that state and look at the total effect.

AVAS's error studies are organized exactly this way: you can study only static errors, only dynamic errors, or both, and the correction "sees" only the static errors. How this is implemented is described in [In AVAS](#errors-in-avas).

## Statistical error studies {#errors-statistics}

### Why many seeds

Errors are random; one computation is one sample, like throwing a die once. The animation already showed that the same size of offsets with another sample gives twice the centroid deviation. Figures such as beam loss and emittance growth often have skewed distributions: most samples are fine, a few are especially bad (a "long tail"). Looking only at the mean underestimates the risk.

The usual procedure has three steps:

1. **Study each kind of error alone**: increase the error step by step and find the size at which a figure (loss rate, emittance growth, centroid offset) just reaches its allowed value; that is the tolerance for this item.
2. **Combined study**: apply all tolerances together, confirm that the total effect stays within the budget, and tighten individual tolerances if needed.
3. **Repeat with correction**: this gives the "tolerances after correction".

### How many runs

- The statistical error of a mean is about $\sigma/\sqrt N$ ($\sigma$ being the spread of single results, $N$ the number of samples): four times as many samples halve the error. Estimating a tail such as "the worst 1 %" takes far more samples than estimating the mean.
- **No loss does not mean no losses**: the "rule of three" says that when none of $N$ runs has losses, all we can say is that the probability of a single run having losses is probably below $3/N$ (with 95 % confidence). With 100 loss-free runs, the probability is only known to be below about 3 %. Where this comes from is in "Going deeper: how many runs".
- **The "size" of an error can be stated in different ways**: an error uniformly distributed in $[-A, A]$ has the rms $A/\sqrt3 \approx 0.58A$, not $A$; a Gaussian has the rms of its standard deviation σ. Convert to rms when comparing results of different distributions.

### Scanning the error size in groups

Divide the error size into several groups from small to large and draw several samples per group; one study then gives the curve "figure versus error size". AVAS's `err_step a b` does exactly this: a groups of b runs each, the error size of group g being $g/a$ times the given value (except for the fixed-value type, see [In AVAS](#errors-in-avas)); the last group is the given full amplitude.

## Correction: pulling the beam back to the centre {#errors-correction}

Since a misaligned quadrupole amounts to a "kick", another kick in the opposite direction can cancel it. That is orbit correction: place steerers (small magnets with a uniform field, see [steerers](#steerer)) and BPMs near the quadrupoles and adjust the steerers to bring the beam centre at the downstream BPMs back to zero. The classic "one-to-one" correction puts one steerer and one BPM next to every quadrupole. By the estimate of the previous section, installation errors of the order of 0.1 mm already call for correction.

How strong must a steerer be? Estimating with the example FODO: with a steerer at one F quadrupole and a BPM at the next F quadrupole, each 1 mrad of steerer kick moves the beam centre at the BPM by about $1.18\ \mathrm{mm}$. Pulling back an offset of 1 mm needs a kick of $0.85\ \mathrm{mrad}$, which is a steerer 0.1 m long with 1.5 mT (field integral $1.5\times 10^{-4}\ \mathrm{T\,m}$). The calculation is step three of "Going deeper: how errors propagate and add up".

How to compute many BPMs and many steerers together, and why one must not "solve exactly", is explained in the [Orbit correction](#orbit-correction) chapter (response matrix and singular value decomposition).

Some things correction cannot do:

- **Errors of the BPMs themselves**: a wrong BPM zero or noisy readings turn directly into a residual offset after correction.
- **Swings between BPMs**: correction only makes the beam centre zero at the BPMs; between two BPMs the beam may still be off.
- **Dynamic errors**: too fast to be measured or compensated.
- **Mismatch from field errors**: orbit correction only deals with the beam centre, not the beam size; mismatch is handled with matching sections or by adjusting focusing elements.

The commands and algorithm for correction in AVAS are in [In AVAS](#errors-in-avas).

## Think about it {#errors-think}

> **Think about it 1**: 20 quadrupoles each kick by 0.28 mrad; why is the result not 20 times that of one?
>
> Answer: the kicks have random directions and the swing is at random stages, so they partly cancel, like the coin-tossing walk. They add "in squares": 20 quadrupoles amount to only $\sqrt{20} \approx 4.5$ times one.

> **Think about it 2**: is a beam-centre offset of 0.94 mm large or small?
>
> Answer: compare it with the size of the beam itself. The rms beam size at the same place is about 1.74 mm, a ratio of 0.54. Interestingly, along the channel the centroid offset and the beam size grow together where the beam is wide and shrink together where it is narrow, so this ratio is the same everywhere (see step five of "Going deeper: how errors propagate and add up").

> **Think about it 3**: why is a BPM best placed "a quarter of a swing" away from its steerer (a phase difference near $90^\circ$) rather than right next to it?
>
> Answer: the steerer kicks the angle, and an angle needs some distance to become a position. Right next to it, the steerer can hardly move the beam at the BPM, only change its direction; after half a swing (a phase difference of $180^\circ$) the beam centre is back where it was, equally invisible. A quarter of a swing gives the largest change of position and needs the smallest kick.

> **Think about it 4**: in the animation, another "error set" changed the result by a factor of two. How many samples are needed to say reliably that "the beam is off by 0.3 mm on average"?
>
> Answer: the error of a mean shrinks as $1/\sqrt N$. If single results scatter by ±50 %, bringing the error of the mean down to 5 % takes about $(50/5)^2 = 100$ samples.

## Common misconceptions {#errors-misconceptions}

- **Error size versus rms**: a uniform distribution on $[-A, A]$ has the rms $A/\sqrt3$, not $A$. "A uniform 0.1 mm error" and "a Gaussian error of rms 0.1 mm" differ by $\sqrt3$.
- **Gaussians are not truncated**: AVAS samples Gaussian errors without truncation (see the sampling rules below); single seeds can have large errors, which is the tail of the distribution, not a bug.
- **Looking only at the mean**: losses and emittance growth have long-tailed distributions; set tolerances from the bad part (for example the worst 10 % or 1 %).
- **A finished run is not a successful correction**: the correction may not have converged; always check the diagnostics after correction.
- **Two seeds**: the seed of the error sampling and the seed of the initial particle distribution are two settings; fix both when comparing two studies.
- **The thin-gap formula underestimates the phase sensitivity of low-β cavities**: for the example HWR (linear preview) a shift of the RF phase gives about 1.2 times the estimate, a change of the synchronous-phase setting of a V3 = 0 cavity about 1.6 times. Say which phase a tolerance refers to.

## Going deeper: how errors propagate and add up {#errors-deep-propagation}

The main text described the propagation of errors with "kicks", "swings" and "coin tosses". This section turns them step by step into exact formulas: the first five steps use only school physics and algebra, the last paragraph is for readers who know linear algebra and probability.

### Step one: why a misalignment is just a "kick"

In a quadrupole, the push on a particle is proportional to its distance from the **magnet's centre**. With the magnet off by $d$, that distance is $x - d$. For a thin lens, the angle changes on the way through by

$$
\Delta x' = -\frac{x - d}{f} = -\frac{x}{f} + \frac{d}{f}.
$$

The first term $-x/f$ is the normal focusing, computed for each particle from its own $x$; the second term $d/f$ is the same for all particles, a constant "kick". So a misalignment only pushes the whole beam aside together and does not change its shape.

Focal length and gradient are related by $1/f = kL_q$, where $k = G/(B\rho)$ is the focusing strength and $B\rho$ the magnetic rigidity (for the example protons $B\rho = 0.179\ \mathrm{T\,m}$, see [Quadrupole](#quadrupole)). In the example: $k = 5/0.1786 = 28.0\ \mathrm{m^{-2}}$, $kL_q = 2.80\ \mathrm{m^{-1}}$, so

$$
\Delta x' = kL_q\,d = 2.80\ \mathrm{m^{-1}} \times 0.1\ \mathrm{mm} = 0.28\ \mathrm{mrad}.
$$

### Step two: two quantities that describe the swing: β function and phase advance

To say exactly "how the beam centre swings after a kick" we need two quantities. They are introduced in detail in [Linear optics](#linear-optics); here we only use what they mean intuitively:

- **The β function** $\beta(s)$: a measure of how wide the beam is at each point. The rms beam size is $\sigma = \sqrt{\beta\varepsilon}$, with $\varepsilon$ the geometric emittance (the area of the phase-space ellipse, see [Drift](#drift)). In the example FODO, $\beta = 1.35\ \mathrm{m}$ at the centre of the F quadrupole (wide) and $0.45\ \mathrm{m}$ at the centre of the D quadrupole (narrow).
- **The phase advance** $\psi(s)$: "how much of a turn" the swing has made from the start to $s$, counted as an angle, a full swing being $360^\circ$. The example FODO advances $61.4^\circ$ per period, one full swing in about 5.9 periods.

### Step three: how large an offset one kick produces downstream

A kick of angle $\theta_i$ at $s_i$: how far is the beam centre off at $s$ downstream? The answer is

$$
x_c(s) = \sqrt{\beta(s)\,\beta_i}\;\sin\!\big(\psi(s) - \psi_i\big)\;\theta_i .
$$

Each factor means something: $\sqrt{\beta_i}$ says that a kick where the beam is wide has more effect; $\sqrt{\beta(s)}$ says that the offset also looks larger where the beam is wide; the $\sin$ factor is the stage of the swing: right after the kick (phase difference 0) there is no offset yet, after a quarter swing ($90^\circ$) the offset is largest, after half a swing ($180^\circ$) the beam is back on the centre line.

A check: with only a drift, the formula should reduce to "angle × distance". Let the waist be at $s_i$ with $\beta_i = \beta_0$; the [Drift](#drift) chapter gives $\beta(s) = \beta_0 + L^2/\beta_0$ ($L$ being the distance from the waist), and the phase advance satisfies $\tan\Delta\psi = L/\beta_0$. Then $\sqrt{\beta\beta_i} = \sqrt{\beta_0^2 + L^2}$ and $\sin\Delta\psi = L/\sqrt{\beta_0^2 + L^2}$, whose product is exactly $L$: it indeed reduces to $x_c = \theta\times L$.

The steerer response of the main text, computed with the example: steerer at an F quadrupole ($\beta_c = 1.35\ \mathrm{m}$), BPM at the next F quadrupole ($\beta_b = 1.35\ \mathrm{m}$, phase difference $61.4^\circ$):

$$
\frac{\Delta x}{\theta_c} = \sqrt{1.35 \times 1.35}\ \mathrm{m} \times \sin 61.4^\circ = 1.35 \times 0.878 = 1.18\ \mathrm{mm/mrad}.
$$

Pulling back 1 mm needs $\theta = 1/1.18 = 0.85\ \mathrm{mrad}$. A steerer's deflection is $\theta = \int B\,\dd l\,/\,B\rho$ (field integral divided by rigidity), so $\int B\,\dd l = 0.85\times 10^{-3} \times 0.179 = 1.5\times 10^{-4}\ \mathrm{T\,m}$, for example 0.1 m long with 1.5 mT.

### Step four: many random kicks: variances add

First the coin. Each step is $+1$ or $-1$ with equal probability. Two steps have four equally likely outcomes: $+2$, $0$, $0$, $-2$. The mean of the squared distance from the start is $(4 + 0 + 0 + 4)/4 = 2$; its square root is $\sqrt2$. After $N$ steps the mean squared distance is $N$ and the rms distance $\sqrt N$.

The reason: when you add $N$ independent random numbers of zero mean and expand the square, the cross terms (such as "step 1 × step 2") are positive half the time and negative half the time and average to zero; only each term's own square remains. So **the "mean square" of a sum of independent random quantities is the sum of their "mean squares"** — variances add.

Apply this to the formula of step three: each quadrupole contributes a term $\sqrt{\beta(s)\beta_i}\sin(\Delta\psi_i)\,\theta_i$. The offsets $\theta_i$ are random and independent, so

$$
\langle x_c^2(s)\rangle = \beta(s)\sum_i \beta_i\,\langle\sin^2\Delta\psi_i\rangle\,\langle\theta_i^2\rangle .
$$

The angle brackets $\langle\ \rangle$ mean averaging over many error sets. The average of $\sin^2$ over a turn is $1/2$ ($\sin^2 + \cos^2 = 1$, and both have the same average, half each), so

$$
\langle x_c^2(s)\rangle^{1/2} = \sqrt{\frac{\beta(s)}{2}\sum_i \beta_i\,\langle\theta_i^2\rangle} .
$$

With the example: 10 periods with 20 quadrupoles, each with an rms $x$ offset of 0.1 mm, $\sigma_\theta = 0.28\ \mathrm{mrad}$; $\beta = 1.35$ at F and $0.45\ \mathrm{m}$ at D, so $\sum\beta_i = 10\times(1.35 + 0.45) = 18\ \mathrm{m}$; at the exit (entrance of an F quadrupole) $\beta = 1.26\ \mathrm{m}$:

$$
\sqrt{\frac{1.26}{2} \times 18 \times (0.28\times 10^{-3})^2}\ \mathrm{m} = \sqrt{8.9\times 10^{-7}}\ \mathrm{m} \approx 0.94\ \mathrm{mm}.
$$

20000 random samples with thick-lens transfer matrices give $0.96\ \mathrm{mm}$. As the number of quadrupoles $N$ grows, the number of terms in the sum grows proportionally to $N$, so the offset grows as $\sqrt N$.

### Step five: comparing with the beam size

The rms beam size at the same place is $\sigma = \sqrt{\beta(s)\,\varepsilon}$. In the example $\varepsilon = 0.1377/0.05708 = 2.41\ \mathrm{mm\,mrad}$ (normalized emittance divided by $\beta\gamma$), so $\sigma = \sqrt{1.256 \times 2.41} = 1.74\ \mathrm{mm}$.

Note that the centroid offset and $\sigma$ are both proportional to $\sqrt{\beta(s)}$, so their ratio does not depend on the position:

$$
\frac{\langle x_c^2\rangle^{1/2}}{\sigma} = \sqrt{\frac{\sum_i\beta_i\,\theta_i^2}{2\varepsilon}} = \sqrt{\frac{18 \times 7.84\times 10^{-8}}{2 \times 2.41\times 10^{-6}}} = 0.54,
$$

which is just $0.94/1.74$. Put differently: each kick angle $\theta_i$ is to be compared with the beam's own rms angle $\sqrt{\varepsilon/\beta_i}$ there.

During acceleration the geometric emittance shrinks as $1/(\beta\gamma)$ (the normalized emittance stays constant); the beam and its centroid offset both shrink as $1/\sqrt{\beta\gamma}$. This is called **adiabatic damping**.

### For readers who know linear algebra and probability

The effect of a kick downstream is the transfer-matrix element $M_{12}(s_i \to s)\,\theta_i$; written with Twiss parameters, $M_{12} = \sqrt{\beta\beta_i}\sin\Delta\psi$, which is step three. Each step of the linear model is an affine map $\boldsymbol x \to M\boldsymbol x + \boldsymbol d$ whose constant term $\boldsymbol d$ comes from steerers and misalignments, so the centroid is exactly linear in the errors and the σ matrix (second moments about the centroid) is not affected by misalignments. Step four is the additivity of variances of independent random variables; the ensemble average $\langle\sin^2\rangle = 1/2$ assumes the phases are spread evenly enough over $[0, 2\pi)$; with few periods, or phase advances near integer multiples of $180^\circ$, sum over the actual phases. Adiabatic damping follows from the conservation of $\varepsilon_n = \beta\gamma\,\varepsilon$.

## Going deeper: gradient errors, rotations and mismatch {#errors-deep-gradient}

### Step one: a gradient error = an extra thin lens

A relative gradient error $\Delta G/G$ changes the inverse focal length (the "strength" of the lens) by

$$
\Delta\!\left(\frac1f\right) = kL_q\,\frac{\Delta G}{G}.
$$

A 1 % error: $\Delta(1/f) = 2.80 \times 0.01 = 0.028\ \mathrm{m^{-1}}$, as if a weak lens with a focal length of 36 m were stacked onto the quadrupole.

### Step two: how large the mismatch is

After a matched beam passes this extra weak lens, the shape of its phase-space ellipse no longer fits the channel. By the section on the [mismatch factor](#matching-mismatch-factor), the mismatch factor is about

$$
M \approx \frac{\beta\, kL_q}{2}\,\frac{\Delta G}{G}.
$$

$\beta$ is the β function at the lens: the wider the beam at the lens, the more the same lens error disturbs it. A 1 % error in one F quadrupole: $M \approx 1.35 \times 2.80 \times 0.01/2 = 0.019$. The mismatch from many independent small errors adds up roughly as a sum of squares (the same reasoning as the random walk of the previous section).

### Step three: how much the phase advance changes

A stronger lens makes the swing faster. The phase advance per period changes by about

$$
\Delta\sigma \approx \frac12\sum_i \beta_i\,\Delta\!\left(\frac1f\right)_i
$$

(radians, positive for focusing, negative for defocusing). With all quadrupoles 1 % too strong, within one period the F quadrupole contributes $+\tfrac12 \times 1.35 \times 0.028$; the D quadrupole is a defocusing lens and defocuses more when stronger, contributing $-\tfrac12 \times 0.45 \times 0.028$; together about $0.0125\ \mathrm{rad} \approx 0.71^\circ$. The FODO lab of the main text uses thick-lens matrices and gives $61.44^\circ \to 62.12^\circ$, an increase of $0.68^\circ$, in line with the estimate.

### Step four: rotation errors

A quadrupole rotated by $\varphi_z$ about the z axis can be split into a well-aligned quadrupole plus a "skew quadrupole" (a quadrupole with its poles turned by 45°) whose strength is about $2\varphi_z$ times the normal one. A skew quadrupole turns an $x$ deviation into a force in $y$, coupling the two planes and increasing the projected emittances. Rotations $\varphi$ about the x and y axes offset the two ends of an element of length $L$ transversely by about $\pm L\varphi/2$; on average the centre is not offset, so the effect is similar to but smaller than an offset error.

### For readers who know vector analysis

A quadrupole field rotated by $\varphi_z$ about the z axis equals, in the original coordinates, a normal quadrupole field of strength $G\cos 2\varphi_z$ plus a skew quadrupole field of strength $G\sin 2\varphi_z$; for small angles the skew component is about $2\varphi_z G$. The derivation of the mismatch-factor formula is in the matching chapter; the phase-advance change $\Delta\sigma = \tfrac12\oint\beta\,\Delta k\,\dd s$ is the first-order perturbation result, corresponding to the linearization of $\cos\sigma = \tfrac12\operatorname{tr}M$.

## Going deeper: phase errors of low-energy cavities {#errors-deep-rf}

The main text said that phase errors of low-energy cavities have more effect than the thin-gap formula estimates. This section works out why, step by step.

### Step one: the thin-gap formula

Think of the cavity as a very thin accelerating gap, crossed so quickly that the field does not change meanwhile. The energy gain is

$$
\Delta W = qV_0T\cos\varphi_s ,
$$

with $V_0T$ the effective voltage and $\varphi_s$ the synchronous phase. An amplitude change $\Delta V/V$ and a phase change $\Delta\varphi$ change the energy by (the second formula uses the school approximation $\cos(\varphi + \Delta\varphi) \approx \cos\varphi - \sin\varphi\,\Delta\varphi$, with $\Delta\varphi$ in radians):

$$
\Delta W = qV_0T\cos\varphi_s\,\frac{\Delta V}{V},\qquad
\Delta W \approx -qV_0T\sin\varphi_s\,\Delta\varphi .
$$

### Step two: the example cavity

The example HWR gains 0.339 MeV at $\varphi_s = -33^\circ$; take the effective $V_0T = \Delta W/\cos\varphi_s \approx 0.405\ \mathrm{MV}$ at this phase. A 1 % amplitude error: $\Delta W \approx 0.339\ \mathrm{MeV} \times 0.01 = 3.4\ \mathrm{keV}$. A phase error of $1^\circ = 0.01745\ \mathrm{rad}$: $\Delta W \approx 0.405 \times \sin 33^\circ \times 0.01745\ \mathrm{MeV} = 3.8\ \mathrm{keV}$.

### Step three: the numbers of the linear preview

Changing the parameters and recomputing with the linear preview: increasing `Ke` by 1 % adds $3.5\ \mathrm{keV}$ of gain, in line with step two. For the phase, first say **which** phase is in error:

- **A shift of the RF phase**: a real RF phase error is a shift of the RF relative to the arriving beam, i.e. a change of the entrance phase $\varphi_0$. Shifting $\varphi_0$ by $1^\circ$ (V3 = 1) changes the gain by $4.6\ \mathrm{keV}$, about 1.2 times the thin-gap estimate.
- **The synchronous-phase setting**: changing the synchronous-phase **setting** of a V3 = 0 cavity from $-33^\circ$ to $-32^\circ$ adds $6.0\ \mathrm{keV}$, about 1.6 times. Here the program solves for the entrance phase again. Near $-33^\circ$, each degree of entrance phase moves $\varphi_s$ as defined with atan2 in [longitudinal dynamics](#longitudinal-deep-gap) by only about $0.76^\circ$; conversely, changing $\varphi_s$ by 1° amounts to changing the entrance (RF) phase by about $1/0.76 \approx 1.3^\circ$ (in the lab of the main text the entrance phase goes from $52.66^\circ$ to $53.97^\circ$).

### Step four: why the thin-gap formula is not enough

In a low-β cavity the particle accelerates noticeably inside the cavity; as its speed changes, so do the times at which it crosses the gaps, and the effective voltage itself depends on the phase. $\Delta W/\cos\varphi_s$ changes by about $2.6\ \mathrm{keV}$ per degree between $-30^\circ$ and $-40^\circ$; multiplied by $\cos\varphi_s$ this contributes about $2.2\ \mathrm{keV/{}^\circ}$ on top of the $3.8\ \mathrm{keV/{}^\circ}$, adding up to about $6.0\ \mathrm{keV}$ — the number of step three.

This is physics, not a numerical error: integrating the same field map with the velocity frozen at its entrance value gives $\Delta W = qV_0T\cos\varphi_s$ with a phase-independent $V_0T = 0.389\ \mathrm{MV}$, and both sensitivities return to the thin-gap value of about $3.7\ \mathrm{keV/{}^\circ}$.

### Step five: what it means for tolerances

Phase tolerances in the low-energy section must be estimated with the actual field map, stating at the same time what the phase error refers to (a shift of the RF phase or the synchronous-phase setting). How the engine applies the `φs` column of `err_cav_*` to a V3 = 0 cavity (as a change of the synchronous-phase setting or as a shift of the RF phase) is not described in the manual; the engine is authoritative.

### For readers who know calculus

Write $\Delta W(\varphi_s) = qV(\varphi_s)\cos\varphi_s$ with $V(\varphi_s) = \Delta W/\cos\varphi_s$ the phase-dependent effective voltage; then $\dd\Delta W/\dd\varphi_s = -qV\sin\varphi_s + q\cos\varphi_s\,\dd V/\dd\varphi_s$. The thin-gap formula keeps only the first term; in a low-β cavity the second is of the same order. The map between entrance phase and $\varphi_s = \operatorname{atan2}(\int E\sin\varphi, \int E\cos\varphi)$, with $\dd\varphi_s/\dd\varphi_0 \approx 0.76$, is not 1 either, which is why the "setting" and "shift" sensitivities differ.

## Going deeper: how many runs {#errors-deep-statistics}

### Step one: the error of a mean

If single results have the spread (standard deviation) $\sigma$, the mean of $N$ results has the spread $\sigma/\sqrt N$. The reason is the same as for the coin: the variance of the sum of $N$ independent results is $N\sigma^2$; dividing by $N$ for the mean makes the variance $N\sigma^2/N^2 = \sigma^2/N$, whose square root is $\sigma/\sqrt N$.

### Step two: the rule of three

Let $p$ be the probability that a single run has losses. Then one run is loss-free with probability $1 - p$, and all $N$ runs with probability $(1 - p)^N$.

If $p$ is large, "all $N$ runs loss-free" hardly ever happens. We ask: how large can $p$ be while "all $N$ loss-free" still has a 5 % chance? Set $(1 - p)^N = 0.05$. For small $p$, $(1 - p)^N \approx e^{-pN}$ (for example $0.99^{100} = 0.366$ and $e^{-1} = 0.368$), so $e^{-pN} = 0.05$ and

$$
p = \frac{-\ln 0.05}{N} = \frac{3.0}{N}.
$$

That is the "rule of three": when all $N$ runs are loss-free, we can say with 95 % confidence that $p < 3/N$.

### Step three: the rms of a uniform distribution

What is the mean square of numbers uniformly distributed in $[-A, A]$? Cutting the interval into many small pieces, it is the average height of $x^2$. With the school sum formula $1^2 + 2^2 + \dots + n^2 = n(n+1)(2n+1)/6 \approx n^3/3$: take $x = A\cdot k/n$ ($k = 1 \dots n$); the mean is $\approx A^2 \cdot (n^3/3)/n^3 = A^2/3$. So the rms is $A/\sqrt3 \approx 0.577A$.

### For readers who know probability

$\langle x^2\rangle = \frac{1}{2A}\int_{-A}^{A}x^2\,\dd x = A^2/3$. The rule of three approximates the one-sided 95 % upper confidence limit of a binomial proportion with zero events (Hanley and Lippman-Hand, 1983). The estimation error of a quantile depends on the density of the distribution at that quantile; the tail has little density and hence a large error, which is why estimating "the worst 1 %" takes far more samples than estimating the mean.

## In AVAS {#errors-in-avas}

### How static and dynamic errors are implemented

Each error run of AVAS (one "run" of a "group") makes Python read the lattice again from the input snapshot, draw all errors for this run's group, write the drawn values as engine commands and run the engine.

> **AVAS convention**: the engine knows only the dynamic error keywords (`err_*_ncpl_dyn`, `err_beam_dyn` and the `*_dyn_on` switches). A static error study rewrites the drawn static errors as dynamic error commands with fixed values for the engine. So "static" and "dynamic" differ in AVAS not inside the engine but in the procedure:

- `error_type = stat`: only static errors (`*_stat` commands) are drawn, dynamic error commands are ignored; `adjust` correction can run.
- `error_type = dyn`: only dynamic errors are drawn, static error commands are ignored; no correction runs.
- `error_type = stat_dyn`: both are drawn. The correction (if any) "sees" only the static errors — the lattice written to the engine during optimization contains only static errors; once the correction is settled, the static and dynamic errors of each element are added column by column into one dynamic error command, the switches being the union of both, and the engine runs. This corresponds to the physical picture "first tune the correctors to the static errors, then add the jitter that cannot be compensated".

Whether the error values stay constant during the whole tracking of a run is up to the engine and not described in the manual; by the commands Python writes, each run has one set of fixed values.

### Error commands: groups and input-beam errors

Error commands are written in the lattice file (between `start` and `end`, usually at the beginning) and act on the elements **below** the command.

```avas-params
err_step err_beam_dyn err_beam_stat
```

- `err_step a b`: a groups of b runs each; an error study requires it.
- `err_beam_dyn r dx dy dφ dx' dy' dE dεx dεy dεz mx my mz dIb`: dynamic errors of the input beam, `r` being the distribution type; not all parameters need to be written, columns left out have no error.
- `err_beam_stat 0 …`: the static version; the manual requires 0 as the first parameter.

> **Note**: the manual does not explain `mx`, `my`, `mz`. The current code interprets the first parameter of `err_beam_stat` like the `r` of `err_beam_dyn`; 0 means a fixed-value error, consistent with the manual.

### Error commands: magnetostatic elements and RF cavities

```avas-params
err_quad_ncpl_stat err_quad_cpl_stat err_cav_ncpl_stat err_cav_cpl_stat
```

The dynamic versions `err_quad_ncpl_dyn`, `err_quad_cpl_dyn`, `err_cav_ncpl_dyn`, `err_cav_cpl_dyn` have the same columns:

```text
err_quad_ncpl_stat  N  r  dx  dy  dφx  dφy  dφz  dG  dz  Nb
err_cav_ncpl_stat   N  r  dx  dy  dφx  dφy  ke/kb  φs  dz  Nb
```

- `N`: acts on the N **elements** below the command. All elements count (drifts, field maps, matrix elements, diagnostics), but the errors are applied only to elements of a suitable type; to cover all elements write a large number.
- Magnetostatic errors (`err_quad_*`) apply to `quad` and to magnetostatic field maps (`field` type 3, such as solenoid maps); cavity errors (`err_cav_*`) apply to RF field maps (`field` type 1). The `solenoid` matrix element and electrostatic maps (type 2) take neither.
- `Nb` is not explained in the manual and is not sampled.

> **Note**: the current code does not sample the **last column** of an element error command and keeps it as written. When the command is written out to `Nb`, that column is `Nb`; if it stops at `dz` and leaves out `Nb`, `dz` becomes the last column and is neither sampled from the distribution nor scaled with the group number. When you need `dz` errors, write `Nb` as well (e.g. 0).

### Switches

```avas-params
err_quad_stat_on err_cav_stat_on err_beam_stat_on
```

`err_quad_dyn_on`, `err_cav_dyn_on`, `err_beam_dyn_on` likewise. Each column is 0 or 1, in the order of the error columns of the corresponding error command; columns switched to 0 get no error. An error study requires at least one switch column set to 1 for its mode, otherwise it reports an error (Missing … error on command).

### An example

```text
start
err_step 3 20
err_quad_stat_on 1 1 0 0 0 0 0
err_quad_ncpl_stat 1000 1 0.1 0.1
err_cav_dyn_on 0 0 0 0 1 1 0
err_cav_ncpl_dyn 1000 2 0 0 0 0 1.0 1.0
drift 0.05 0.02 0
...
end
```

Meaning: 3 groups of 20 runs. All quadrupoles / magnetostatic maps get uniformly distributed offsets in x and y, $[-0.1, 0.1]\ \mathrm{mm}$ in group 3 and $[-0.033, 0.033]\ \mathrm{mm}$ in group 1; all RF cavities get Gaussian dynamic amplitude and phase errors with standard deviations of 1 % and $1^\circ$ in group 3. In `stat_dyn` mode both act; `stat` has only the offsets, `dyn` only the RF errors.

### Sampling rules

Distribution type `r`:

| r | Meaning | Column with given value A in group g (of a) |
|---|---|---|
| 0 | fixed value | always A, not scaled with the group |
| 1 | uniform | uniform in $[-A g/a,\ A g/a]$ |
| 2 | Gaussian | mean 0, standard deviation $A g/a$ (not truncated) |
| -1 | equal steps | $A g/a$ (equivalent to TraceWin's type 0) |

```avas-params
err_quad_ncpl_dyn
```

> **Note**: the equal-step type (r = −1) of input-beam errors is computed as $A(g+1)/a$ in the current code, unlike element errors; check the generated error records before using it. Drawn values are kept to 5 decimal places.

Coupled and uncoupled:

- `ncpl` (uncoupled): every affected element is sampled **independently**.
- `cpl` (coupled): each command draws once and applies the same values to all elements it covers, to model common errors such as a shared support or a shared power supply.
- When one element is covered by several quadrupole / cavity error commands of the same kind (static or dynamic), the current code keeps only the one closest to the element (the lowest) and does not add them.

Random seeds:

- `[error] seed` in `ini.ini` ("Seed" under "Error study" on the Simulation settings page, `--seed` on the command line) decides the error sampling (and the random starting values of the correction). The same seed and lattice give the same error sequence.
- `randomseed` (listed under `beam.txt` in the manual, written in `input.txt` in the example project) seeds the generation of the initial particle distribution, a setting independent of the error seed. Fix both when comparing two error studies.

### Static error correction: commands

```avas-params
adjust diag_energy diag_size diag_position
```

- `adjust 0 v n min max first_step`: placed directly in front of the element to tune, it makes the element's v-th parameter a knob (counted from the 1st parameter after the keyword: the phase of `field` is the 6th, `Ke` the 7th, `Kb` the 8th; the gradient of `quad` the 4th; `Bx`/`By` of `steerer` the 4th/5th), limited to `[min, max]`; knobs with the same (non-zero) `n` are constrained to one value; `first_step = 1` starts from the element's current value, 0 from a random value within the range.
- `diag_position 0 x y 0`: target beam centre at this position (mm).
- `diag_size 0 sx sy 0`: target rms size at this position (mm).
- `diag_energy 0 W 0`: target energy at this position (MeV).
- `diag_*` are zero-length elements marking where a target applies; they are not passed to the engine, Python reads the beam parameters at their position from DataSet.

### Static error correction: algorithm

This is how AVAS's static error correction currently works; the manual does not document these details and they may change:

1. A correction runs only when an `adjust` and a `diag_*` have the same first parameter (the manual calls it "meaningless, write 0"; writing 0 everywhere makes one group). Different numbers are corrected one after another in increasing order, each starting from the lattice corrected by the previous one.
2. Each group is optimized with SLSQP: every function evaluation writes the knobs into the lattice, runs the engine to just past the last diagnostic and computes the objective — the mean squared deviation over the group's diagnostics: position $(x_c - x)^2 + (y_c - y)^2$ (mm²), size $(x_{\mathrm{rms}} - s_x)^2 + (y_{\mathrm{rms}} - s_y)^2$ (mm²), energy $(W - W_{\mathrm{target}})^2$ (MeV²).
3. It stops when the objective falls below 0.05; at most 100 iterations, a finite-difference step of 0.1 (in the knob's own unit), convergence tolerance $10^{-4}$.
4. Every error run is corrected anew; the corrected values go to `Adjust_Datas_<group>_<run>.txt` and the beam parameters at the diagnostics to `par_diag_datas_<group>_<run>.txt`. The engine runs of the correction go under `error_adjust/`. The corrected values are not written back to the lattice file.

> **Note**: the finite-difference step is fixed at 0.1, so for knobs whose useful range is much smaller than 0.1 (for example a steerer field of ±0.01 T in T) the gradient estimate is unreliable; the threshold 0.05 of the objective has the dimension of the target (mm² or MeV²) and may be too loose for small beams: 0.05 mm² corresponds to a centroid deviation of about 0.22 mm. The energy-correction example of the case tutorial written as in the manual (target 5 MeV) does not reach its target (the recorded final energy is still about $2.3\ \mathrm{MeV}$) — a finished run is not a successful correction; always check the diagnostic data after correction.

### Writing an orbit correction (sketch)

The following only shows how the commands go together; it has not been verified with the engine:

```text
adjust 0 4 0 -0.005 0.005 1
adjust 0 5 0 -0.005 0.005 1
steerer 0 0.02 0 0 0 0 0.005
field 0.35 0.02 0 3 0 0 1 0.38 sol
drift 0.05 0.02 0
diag_position 0 0 0 0
```

The two `adjust` lines make `Bx` and `By` of the steerer knobs; the steerer acts on the next element (the solenoid map); `diag_position` asks for the downstream centre to return to (0, 0). Mind the field unit T and the finite-difference step problem above; check the convergence with a small study before relying on it.

### Running an error study

The `[error]` section of `ini.ini`:

| Key | Meaning |
|---|---|
| `error_type` | empty = normal run; `stat`, `dyn`, `stat_dyn` = the corresponding error study |
| `seed` | random seed of the error sampling |
| `if_normal` | 1 = run an error-free reference first |

In the interface: choose None / Static / Dynamic / Static + dynamic under "Error study" on the Simulation settings page and fill in "Seed" (runs from the interface always do the error-free reference first); then write `err_step`, error commands and switches in the lattice, save and run. On the command line: `avas run --input … --output … --mode stat|dyn|stat_dyn --seed 7`, with `--no-normal` to skip the reference run; `--mode auto` (the default) reads `ini.ini`. The AI assistant's `set_error_study` tool changes the same two settings (with the user's approval).

Like a full run, an error study runs on the `inputs/` snapshot of the output folder and does not rewrite the project's InputFile; during the run (including pauses) the input files are read-only.

### Output files

| Path (relative to the output folder) | Content |
|---|---|
| `error_output/output_0_0/` | the error-free reference run |
| `error_output/output_<group>_<run>/` | complete output of each error run (DataSet.txt etc., and the lattice.txt used) |
| `error_middle/output_0/` | the run in progress, moved to `error_output` when finished |
| `errors_par_tot.txt` | one line per run: loss rate, emittance growth $\varepsilon_{\mathrm{out}}/\varepsilon_{\mathrm{in}} - 1$ of the three planes, final centre and angle, rms size and divergence, energy difference to the reference run, final Twiss |
| `errors_par.txt` | one line per group: the means over the group's runs of the quantities from loss rate to energy difference (without Twiss), plus the rms of centre, angle, size, divergence and energy difference |
| `Error_Datas_<group>_<run>.txt` | the error values applied to each cavity / quadrupole in this run |
| `Adjust_Datas_<group>_<run>.txt`, `par_diag_datas_<group>_<run>.txt` | corrected values, beam parameters at the diagnostics |
| `density_par_*.dat`, `density_tot_par*.dat` | particle density statistics (need `pchistogram 1 …` in `input.txt`) |

A run whose beam does not reach the end of the lattice (the last DataSet position more than 5 cm short of the total length) gets a loss rate of 1 and zeros in the other columns.

### Viewing the results

- The Run page shows the envelope of the current seed live and overlays the finished seeds as an envelope band ("Finished error seeds"), a direct picture of the spread caused by the errors.
- The "Error study" group of the Results page has two kinds of plots: "Emittance growth and loss" (reading `errors_par*.txt`) and "Output parameters" (with the mean or root-mean-square statistic).
- The detailed result of a single run can be viewed on the Results page by choosing its `error_output/output_<group>_<run>` folder.

A real statistical study needs the engine: copy the example project to a practice folder, add `err_step` and error commands as in the example above, choose the error mode on the Simulation settings page and run.

## Further reading {#errors-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: errors and tolerances, estimates of centroid offsets.
- S. Y. Lee, *Accelerator Physics*; H. Wiedemann, *Particle Accelerator Physics*: the linear theory of orbit distortion and correction.
- J. A. Hanley, A. Lippman-Hand (1983): the "rule of three" for zero events.
- The chapters on error analysis and static error correction of the AVAS manual (`docs/使用说明20260427.docx`) and the error commands in the in-app "Parameters and files" reference.
- The complete list is in [references](#references).
