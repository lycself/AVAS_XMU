# Longitudinal dynamics

The previous chapters were all about the **transverse** directions: how to keep the beam from growing fatter. This chapter turns to the **longitudinal** direction, the direction in which the particles travel, and answers two questions. How do we make the particles go faster and faster? And the particles of a bunch arrive one after another: how do we keep them together while they are being accelerated?

Particles are accelerated by electric fields. But the electric field in an accelerator is not steady: it swings back and forth 162.5 million times a second (162.5 MHz), forward one moment and backward the next. How can such a field keep pushing the particles forward? The particles of a bunch arrive at slightly different times and see different fields, so one would expect their energies to spread and the bunch to grow longer and longer; yet in a real linac a bunch passes tens or hundreds of cavities without falling apart. What pulls the early and the late particles back together? Why does the AVAS example set the "synchronous phase" to −33° rather than the crest, where the push is strongest? This chapter answers these questions.

Keep two everyday pictures in mind:

- **Pushing a swing**. Push at the right moment and the swing goes higher and higher; push at the wrong moment and you slow it down. RF acceleration is "pushing at the right moment".
- **Surfing**. A surfer does not stand on the crest of the wave but stays on its front slope. Falling a little behind, the slope is steeper, the push stronger, and he catches up; getting a little ahead, the push is weaker and he drops back. The particles of a bunch stay together for exactly the same reason.

First, have a look. In the lab below the protons of the example beam (kinetic energy 1.527 MeV, about 5.7 % of the speed of light) fly through one RF cavity of the example project (a half-wave resonator, HWR, 0.21 m long):

```avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
```

Look at the plot "Kinetic energy along z": crossing the cavity, the proton's energy rises from 1.527 MeV to about 1.867 MeV, a gain of 0.340 MeV. Now look at "Energy gain versus RF phase at the entrance": it is a wave. A proton entering the cavity at different moments gains completely different energies, at most about 0.48 MeV, and at some moments it is even **decelerated** (negative gain). The marked point is "this setting"; drag the "synchronous phase" slider and watch the point move along the wave. This wave is the hero of the chapter.

## RF acceleration: pushing at the right moment {#longitudinal-rf}

### Why not a steady high voltage

School physics says: a charge $q$ crossing a voltage $U$ has work $qU$ done on it, and its kinetic energy grows by $qU$. The most direct approach is therefore a very high voltage. But that road does not go far: a DC voltage of millions of volts is hard to build (air and insulators break down and spark), and you cannot "chain" many stages — the work done by an electrostatic field depends only on the potentials at the start and the end, so to enter the next stage the particle must first return to ground potential and give back what it gained.

### An oscillating field, plus hiding

The solution is a field that **oscillates**. In an accelerator the particles fly through a string of metal cavities. Each cavity has a narrow slot in the middle, called a **gap**, and only in the gap does the field act on the particles. When the field points forward the particle is timed to cross the gap and gets a push; when the field turns backward the particle is hiding inside a metal tube, which screens the field, and nothing happens to it. The next time the field points forward the particle has reached the next gap.

The frequency of the oscillation is between tens of megahertz and a few gigahertz, the same as radio waves, hence the name **radio frequency** (RF). The metal cavity in which the RF field "lives" is an **RF cavity**: like a bottle whose air vibrates at a fixed pitch when you blow across its mouth, the electromagnetic field in the cavity oscillates at a fixed frequency.

This is exactly like pushing a swing: the rhythm of the pushes must match the rhythm of the swing.

### Phase: saying "when" with an angle

An oscillation repeats itself, and the most convenient way to say "at which moment of the oscillation" is an angle: one full period counts as 360°. This angle is the **phase** $\varphi$. This chapter and AVAS use the conventions:

- $\varphi = 0$ is the moment the forward field is strongest, the **crest**;
- **a larger phase means the particle arrives later**.

At the example frequency $f = 162.5$ MHz one period is $1/f = 6.15$ ns (nanoseconds, billionths of a second), so arriving 1 ns later means a phase 58.5° larger.

In one RF period light travels $\lambda = c/f = 1.845$ m, the **RF wavelength**; the example proton moves at only $\beta = 0.05698$ times the speed of light and covers just $\beta\lambda = 0.1051$ m, i.e. 105 mm, in one period. This length sets how big the low-energy cavities are and how far apart they sit: the slower the particle, the shorter $\beta\lambda$, and the more compact everything must be. (Where $\beta$ and $\gamma$ come from: [Relativistic kinematics](#kinematics).)

## How much energy one gap gives {#longitudinal-gap}

### A short gap: the gain follows a cosine

The field across a cavity gap oscillates in time like a cosine. If the gap is very short, the particle crosses it in the blink of an eye and sees the voltage of that instant. Let the peak voltage be $V_0$ and the phase at arrival $\varphi$; the voltage at that instant is $V_0\cos\varphi$, and the particle gains

$$
\Delta W = q\,V_0\cos\varphi .
$$

At $\varphi = 0$ (the crest) the gain is largest; at $\varphi = \pm 90°$ the field is exactly zero and there is neither acceleration nor deceleration; for $\lvert\varphi\rvert > 90°$ the field points backward and the particle is decelerated. The wave in the opening lab is this cosine.

### The transit-time factor: the gap has a length

A real gap has a certain length; the particle needs time to cross it, and during that time the field keeps changing. In the first part of the gap, the middle and the last part the particle sees different phases, and its energy gain is the average of the $\cos$ of all the parts, always less than "staying on the crest all the time". Call this discount $T$ ($T \le 1$), the **transit-time factor**:

$$
\Delta W = q\,V_0\,T\cos\varphi .
$$

This is **Panofsky's formula**. Here $\varphi$ is the phase at which the particle reaches the "electrical centre" of the gap (for a symmetric gap, the geometric centre).

How large $T$ is depends on what fraction of a period the crossing takes. The particle covers one $\beta\lambda$ in exactly one period, so what matters is the ratio of the gap length $g$ to $\beta\lambda$. For a gap with a uniform field,

$$
T = \frac{\sin x}{x}, \qquad x = \frac{\pi g}{\beta\lambda}\ (\text{radians}).
$$

Checking with the example beam ($\beta\lambda = 105$ mm):

- Gap length $\beta\lambda/4 = 26$ mm: $x = \pi/4$, $T = 0.707/0.785 = 0.90$.
- Gap length $\beta\lambda/2 = 53$ mm: crossing the gap takes half a period; with the centre on the crest the particle sees phases from −90° to +90°, and the average of $\cos$ is only $2/\pi = 0.64$.

The shorter the gap compared with $\beta\lambda$, the closer $T$ is to 1. In a real cavity the field reaches beyond the gap and $T$ has to be computed from the field distribution; where the formula comes from is in [Going deeper: the energy gain of one gap](#longitudinal-deep-gap).

## Cavities with two gaps: the rhythm must match {#longitudinal-multigap}

### Two gaps with opposite fields

The half-wave resonator (HWR) of the example and the common quarter-wave resonator (QWR) both have **two gaps**, and the fields in the two gaps always point in opposite directions: when the first points forward, the second points backward. For both gaps to accelerate, the particle must fly from the first gap to the second in exactly **half a period**, arriving just as the field has turned around.

Picture two people standing some distance apart on a road pushing a cart, with opposite rhythms: when the one behind pushes forward, the one in front is pulling back. Only if the cart takes exactly half a beat to get from the first person to the second does the second person just be turning to push forward.

If the gap centres are a distance $d$ apart, covering $d$ in half a period means $d = \beta\lambda/2$. Every cavity therefore has a "design speed" $\beta_g = 2d/\lambda$: at that speed both gaps accelerate in step, and the further the speed is from it, the more "out of step" the second gap is. Cavities with more gaps are pickier about speed; low-$\beta$ superconducting linacs therefore use few-gap cavities, in several families with different design speeds (see [the superconducting section](#journey-sc)).

### The example cavity hwr010

Integrating the on-axis field $E_z$ of the `hwr010` map (multiplied by `Ke = 1.36`):

| Quantity | Value |
|---|---|
| on-axis field peaks | $+13.03\ \mathrm{MV/m}$ and $-12.74\ \mathrm{MV/m}$ |
| where the field changes sign | $z \approx 0.104\ \mathrm{m}$ (two gaps) |
| centroids of the two gap fields | $0.064\ \mathrm{m}$, $0.145\ \mathrm{m}$, spacing $d = 0.081\ \mathrm{m}$ |
| $\beta_g = 2d/\lambda$ from the spacing | 0.088 |
| $V_0 = \int\lvert E_z\rvert\,\dd z$ (sum of the two gap voltages) | 0.978 MV |
| maximum of $T(\beta)$ | 0.82, at $\beta \approx 0.104$ |
| $T$ at the entrance $\beta = 0.0570$ | 0.40, $V_0 T = 0.39\ \mathrm{MV}$ |

The $\beta \approx 0.10$ of the maximum of $T$ agrees with the "010" in the name (usually meaning design $\beta = 0.10$); it is slightly above the 0.088 estimated from the spacing because the transit-time factor of a single gap also grows with $\beta$. The example proton is too slow: it needs about 0.77 of a period to fly from one gap centre to the other, not the ideal half period, so $T$ is only 0.40. As the proton is accelerated inside the cavity, $T$ gradually increases. Below is a slice of this field map; you can see that the fields of the two gaps point in opposite directions:

```avas-field
map hwr010
```

### When too slow: the two gaps can cancel

What if the particle is slower still and takes **a whole period** from one gap to the other? When it reaches the second gap the field is back in the same state as at the first gap — but the second gap's field points the other way to begin with. Whatever the first gap pushes, the second pulls back, and the cavity hardly accelerates. For the example cavity a constant-speed estimate puts this near 0.9 MeV (two periods: near 0.23 MeV). Near these energies, no setting of the RF phase gives the desired synchronous phase and the AVAS engine refuses to run; see [Going deeper: two oddities of low-energy cavities](#longitudinal-deep-lowbeta) and [In AVAS](#longitudinal-in-avas).

## Synchronous particle and synchronous phase {#longitudinal-synchronous}

A bunch holds countless particles; to say "where the bunch is and at what phase" we need a reference. The **synchronous particle** (also called the reference particle) is the imaginary particle that by design flies along the axis and sits at the design phase in every cavity; the longitudinal positions and energies of all other particles are given relative to it.

The phase the synchronous particle "effectively" sees in a cavity is that cavity's **synchronous phase** $\varphi_s$. For a very short gap it is just the phase at the gap centre; for a real cavity AVAS defines it as an average along the particle's actual path, weighted by the field strength (the exact expression is in [Going deeper](#longitudinal-deep-gap)).

The phase at which the synchronous particle reaches the next cavity depends on both the cavity's RF phase and the particle's flight time. A drift between cavities advances the phase by $360°\times L/(\beta\lambda)$: flying 0.1 m, the example proton's phase advances by 342.4°. At low energy $\beta\lambda$ is only some ten centimetres, so a few millimetres of length error or a small energy offset changes the phase of the downstream cavities noticeably; this is the root of the sensitivity of phase settings in the low-energy section.

AVAS's linear preview for the example cavity (lattice `field 0.21 0.02 0 1 162.5e6 φs 1.36 -1.36 hwr010`):

| $\varphi_s$ (V3 = 0) | entrance phase | exit energy / MeV | $\Delta W$ / MeV |
|---|---|---|---|
| −90° | −14.0° | 1.5270 | 0.000 |
| −60° | 18.6° | 1.6943 | 0.167 |
| −33° | 52.7° | 1.8665 | 0.340 |
| −20° | 69.7° | 1.9374 | 0.410 |
| 0° | 95.4° | 2.0023 | 0.475 |
| +20° | 119.7° | 1.9954 | 0.468 |

At $\varphi_s = -90°$ the gain is zero, as the cosine says. Note that the "synchronous phase" and the "entrance phase" (the RF phase when the particle reaches the cavity entrance) are different things; in the example they differ by about 86°. Two details do not fit a simple cosine: the largest gain found by scanning the entrance phase (0.482 MeV, exit 2.0087 MeV) occurs at $\varphi_s \approx +8°$ rather than 0°, and the gains at $\pm 33°$ are not symmetric. The reason is that the proton is accelerated too much in this cavity (its energy grows by about 30 %); see [Going deeper: two oddities of low-energy cavities](#longitudinal-deep-lowbeta).

In AVAS the parameter V3 of `field` decides whether the phase written in the lattice is the "synchronous phase", the "entrance phase" or the "absolute phase"; the three forms and an example are in [In AVAS](#longitudinal-in-avas).

## Phase stability: the surfer stays on the front slope {#longitudinal-stability}

### Thought experiment: three particles

Imagine three particles entering a string of cavities one behind the other: the synchronous particle at phase $\varphi_s$, one arriving a little early (slightly smaller phase) and one a little late (slightly larger phase). The energy gain goes as $\cos\varphi$, with the crest at $\varphi = 0$.

- If $\varphi_s$ is **before the crest** ($-90° < \varphi_s < 0$, the field is still rising): the late particle is closer to the crest and gains **more** energy; the early one is further from it and gains less. For slow particles more energy means more speed, so on the way that follows the late particle catches up and the early one drops back. The deviation is corrected — this is a **restoring force**.
- If $\varphi_s$ is **after the crest** ($0 < \varphi_s < 90°$): the late particle gains less, is slower and falls further behind; the early one runs further ahead. The deviation grows — **unstable**.
- If $\varphi_s$ is exactly **on the crest**: both the early and the late particle gain a tiny bit less than the synchronous one, a difference small enough to ignore (the cosine is flat at the crest); there is no restoring force and the bunch just keeps lengthening.

This is surfing: stay on the front slope (before the crest), and falling behind gets you a stronger push while getting ahead gets you a weaker one, so you are always held in place. The principle is called **phase stability**, found independently by Veksler and McMillan in 1944–1945; without it there would be no linacs or synchrotrons as we know them.

### Working out the restoring force

The restoring force is a chain of four links. With the example cavity ($V_0 T = 0.39$ MV, $\varphi_s = -33°$), follow a particle that arrives 1° late, link by link:

1. **Late → more energy**. Near $\varphi_s$ the cosine curve can be treated as a straight line with slope $-\sin\varphi_s$ (per radian). Arriving $\Delta\varphi$ late, the particle gains extra energy $\Delta W - \Delta W_s \approx -q V_0 T\sin\varphi_s\cdot\Delta\varphi$. 1° is 0.01745 radians and $\sin(-33°) = -0.545$, so the extra gain is $0.39\ \mathrm{MeV}\times 0.545\times 0.01745 = 3.7\ \mathrm{keV}$.
2. **More energy → faster**. At low speed the kinetic energy is $W = \tfrac12 m v^2$, and the relative change of energy is twice the relative change of speed: $\Delta v/v = \tfrac12\,\Delta W/W = \tfrac12\times 3.7/1527 = 0.12\ \%$.
3. **Faster → earlier**. The reference particle needs $L/v$ to fly $L$; a particle 0.12 % faster arrives $0.12\ \% \times L/v$ earlier. The example proton needs 58.5 ns per metre, so it gains 0.071 ns per metre.
4. **Earlier → smaller phase**. 1 ns corresponds to 58.5°, so 0.071 ns is 4.1°.

So the particle that was 1° late recovers about 4.1° for every metre it flies: after about 0.24 m it has made up that 1°. This "make-up distance" is exactly the **longitudinal focal length** of one cavity, which we will meet again below.

Combining links 2, 3 and 4 into a general expression (valid also near the speed of light): a particle with energy offset $w = (W - W_s)/(mc^2)$ changes its phase per metre by

$$
\frac{\Delta(\Delta\varphi)}{\Delta s} = -A\,w,\qquad A = \frac{2\pi}{\beta^3\gamma^3\lambda} .
$$

$mc^2$ is the rest energy (938.27 MeV for a proton) and $\gamma$ the relativistic factor. At the example energy $A = 1.83\times 10^4\ \mathrm{m^{-1}}$: 1 keV more energy advances the phase by about 1.1° per metre. The $\beta^3\gamma^3$ in the denominator says: the faster the particle, the harder it is for an energy difference to turn into a phase difference. The derivation is in [Going deeper: the equations of phase oscillation](#longitudinal-deep-oscillation).

### Stability condition

Link 1 shows that only when $\sin\varphi_s < 0$ does the late particle gain more energy and the chain act as a restoring force; acceleration also requires $\cos\varphi_s > 0$. Together, the range of stable acceleration is

$$
-90° < \varphi_s < 0° .
$$

This is the conclusion of the thought experiment: a negative synchronous phase means the synchronous particle arrives before the crest. The chain has two parts: the cavity makes the energy depend on phase, like a "lens"; the drift turns velocity differences into position differences (the $R_{56} = L/\gamma^2$ of [Drift](#drift-longitudinal)). This is exactly the structure of transverse "lens + drift". In a straight section $R_{56}$ is always positive, so the stable phase is always on the rising side of the field.

> Note: phase sign conventions vary between texts. AVAS and Wangler's *RF Linear Accelerators* use the cosine convention with larger phase meaning later; stable synchronous phases are negative and the example uses $-33°$. With a sine convention, or with the phase defined as $-\omega t$, the stable region moves to positive phases. Always check the convention when porting phases from another code or text.

### Checking with AVAS's linear preview

The example beam (initial rms $z$ = 1.316 mm, rms $x$ = 0.954 mm) through one hwr010 cavity, rms sizes at the exit (linear preview, no space charge):

| Case | rms $z$ / mm | rms $x$ / mm |
|---|---|---|
| 0.21 m drift (no cavity) | 1.482 | 1.407 |
| $\varphi_s = -33°$ | 0.702 | 1.811 |
| $\varphi_s = +33°$ | 1.738 | 1.196 |

A negative synchronous phase compresses the bunch longitudinally (**bunching**), a positive one stretches it (**debunching**), in agreement with the stability condition; transversely it is just the other way round, which is the [RF defocusing](#longitudinal-rf-defocusing) discussed below. Note also: at $+33°$ the energy gain is 0.424 MeV, more than the 0.340 MeV at $-33°$; the reason is in [Going deeper](#longitudinal-deep-lowbeta).

The lab below is preset to $+33°$ (5 cm of drift before the cavity, 20 cm after). Look at the envelope plot: at the end rms $z$ (dashed) is about 3.25 mm and rms $x$ about 1.29 mm. Drag the "synchronous phase" to −33°: rms $z$ is first squeezed to 0.72 mm about 4 cm behind the cavity and is about 1.19 mm at the end; rms $x$, however, rises to about 3.09 mm.

```avas-lab
rf-cavity phase=33 Ke=1.36 W=1.527
```

## Synchrotron oscillations: particles swinging inside the bunch {#longitudinal-synchrotron}

### An invisible spring

The restoring force of phase stability is proportional to the deviation: the later a particle arrives, the harder it is pushed back. In school physics, motion under "a restoring force proportional to the displacement" is **simple harmonic motion**, like a mass on a spring or a pendulum at small angles. So in a string of cavities every particle swings back and forth around the synchronous particle: now ahead with less energy, now behind with more. This is the **synchrotron oscillation**.

Averaging the push of each cavity over unit length (mean accelerating gradient $E_0 T$, in MV/m), the strength of this "spring" is described by the **longitudinal wave number** $k_{l0}$ (the oscillation angle per metre, in radians); one oscillation takes a distance $\lambda_l = 2\pi/k_{l0}$:

$$
k_{l0}^2 = A\cdot\frac{qE_0T}{mc^2}\cdot\sin(-\varphi_s) = \frac{2\pi\,qE_0T\sin(-\varphi_s)}{mc^2\,\beta^3\gamma^3\lambda} .
$$

It is the product of the three factors of the chain above: $A$ (how quickly an energy difference becomes a phase difference), $qE_0T/mc^2$ (how hard the cavities push) and $\sin(-\varphi_s)$ (how steep the cosine is at $\varphi_s$).

Numerical example: if a string of cavities had a mean gradient equal to that of the example cavity at its entrance, $V_0T/L = 0.39\ \mathrm{MV}/0.21\ \mathrm{m} = 1.86\ \mathrm{MV/m}$, with $\varphi_s = -33°$, then $k_{l0} = 4.44\ \mathrm{m^{-1}}$ and $\lambda_l = 1.41$ m: a particle swings back and forth once every 1.41 m. In a real structure there are solenoids and drifts between the cavities, the mean gradient is lower and $k_{l0}$ smaller accordingly (proportional to $\sqrt{E_0T}$). $\beta^3\gamma^3$ sits in the denominator: synchrotron oscillations are fastest in the low-energy section and slow down as the energy rises.

### One cavity is a longitudinal lens

A cavity can also be seen as a thin longitudinal lens. The previous section found that a particle 1° late has caught up after about 0.24 m; that is the longitudinal focal length:

$$
\frac{1}{f_z} = \frac{2\pi\,qV_0T\sin(-\varphi_s)}{mc^2\,\beta^3\gamma^3\lambda} \approx 4.1\ \mathrm{m^{-1}},\qquad f_z \approx 0.24\ \mathrm{m}.
$$

That is stronger than the quadrupoles of the FODO lab in [Linear optics](#linear-optics) ($f = 0.45$ m). It is a rough estimate from the averaged formula ($\beta$ changes by about 14 % inside the cavity), but it explains why in the table above rms $z$ drops from 1.48 mm to 0.70 mm in a single cavity.

### In AVAS's longitudinal coordinates

AVAS describes the longitudinal plane not with phase and energy but with the distance $z$ (mm) of the particle ahead of the reference particle and the relative momentum deviation $\delta = \Delta p/p$:

$$
z = -\frac{\beta\lambda}{2\pi}\,\Delta\varphi,\qquad
\delta = \frac{\Delta W}{\beta^2\gamma\,mc^2} .
$$

A particle with $z > 0$ is ahead and has a smaller phase. The example beam's `twissz 0.04275 0.37292 0.26506` corresponds to an initial rms $z = 1.316$ mm (rms phase width 4.5°) and rms $\delta = 3.53$ mrad (rms energy spread 10.8 keV). The meaning of the Twiss parameters is in [Phase space and emittance](#phase-space), how to write `twissz` in [In AVAS](#longitudinal-in-avas).

### Adiabatic damping: the bunch gets shorter as it is accelerated

As the particles are accelerated they become "heavier" and faster, the same energy difference produces less and less velocity difference, and the longitudinal spring slowly changes. It is like a pendulum whose string is slowly shortened: the amplitude changes, but the area enclosed by "amplitude × velocity amplitude" stays the same. For synchrotron oscillations the product of phase width and energy spread stays the same, and with constant gradient, phase and frequency

$$
\Delta\varphi \propto (\beta\gamma)^{-3/4},\qquad \Delta W \propto (\beta\gamma)^{3/4}.
$$

For example, accelerating from 1.527 MeV to 10 MeV shrinks the phase width to 0.49 times and grows the absolute energy spread to 2.0 times, while the relative energy spread $\Delta W/W$ falls to about 0.31 times. The bunch gets shorter and shorter in phase, which is also why the RF frequency can be doubled downstream (see [From the source to the target](#journey-overview)). Said in $(z, \delta)$, this is the **conservation of the normalized longitudinal emittance**, just as transversely (the geometric emittance goes as $1/\beta\gamma$); see [Phase space and emittance](#phase-space). The derivation is in [Going deeper: the equations of phase oscillation](#longitudinal-deep-oscillation).

> Note: this conservation holds only for linear, uncoupled motion without space charge. The longitudinal non-linearity in low-energy cavities ($\cos\varphi$ is no longer straight once the phase width reaches tens of degrees) makes the longitudinal rms emittance grow; the linear preview does not contain this effect, so use the multi-particle engine.

## Longitudinal acceptance: how deep is the well {#longitudinal-separatrix}

### A ball on a tilted washboard

At small amplitude the longitudinal motion is like a spring; what about large amplitude? There is an exact picture: think of the particle as a ball on a **tilted washboard**.

- The ripples of the board come from the cosine of the RF field: the bottom of each well is the synchronous phase $\varphi_s$, and the ball rolling back and forth in a well is the synchrotron oscillation;
- the whole board is tilted, because the synchronous particle must receive a fixed amount of acceleration in every cavity; this "price that must be paid" tips the board to one side;
- on the downhill side of each well there is a "ridge", with its top at phase $-\varphi_s$. A ball with too large an energy offset rolls over the ridge and down the tilted board — its phase keeps slipping away and never comes back: this is **longitudinal loss**.

Joining up the motions that "just fail to get over the ridge" draws a closed boundary in the phase–energy plane, the **separatrix**. Particles inside it perform bounded synchrotron oscillations; this fish-shaped (or golf-club-head-shaped) region is the longitudinal **acceptance**, often called the "bucket". Particles outside are lost longitudinally.

### How wide and how high is the stable region

- Phase width: from the left end $\varphi_1$ to the ridge top $-\varphi_s$. For not too large $\lvert\varphi_s\rvert$ the total width is about $3\lvert\varphi_s\rvert$.
- Energy height: largest at $\varphi_s$. The depth of the well is proportional to $\varphi_s\cos\varphi_s - \sin\varphi_s$: zero at $\varphi_s = 0$ (no well at all), deeper as $\lvert\varphi_s\rvert$ grows.

With the mean gradient 1.86 MV/m of the previous section and the example beam energy (formulas in [Going deeper: the equations of the separatrix](#longitudinal-deep-separatrix)):

| $\varphi_s$ | $\varphi_1$ | phase width | $\Delta W_{\max}$ / MeV | $\cos\varphi_s$ |
|---|---|---|---|---|
| −20° | −40.3° | 60° | ±0.073 | 0.940 |
| −30° | −60.9° | 91° | ±0.133 | 0.866 |
| −33° | −67.2° | 100° | ±0.153 | 0.839 |

This answers the opening question: the larger $\lvert\varphi_s\rvert$, the larger the acceptance, but the acceleration per cavity, proportional to $\cos\varphi_s$, gets smaller. The example's $-33°$ gives up about 16 % of the energy gain in exchange for a stable region 100° wide and ±0.15 MeV high, while the bunch itself has an rms phase width of only 4.5° and an rms energy spread of 10.8 keV: plenty of margin. Low-energy sections usually take a larger $\lvert\varphi_s\rvert$ (around −30° to −40°) to secure the acceptance; in the high-energy section, where the bunch has become shorter in phase (adiabatic damping), $\lvert\varphi_s\rvert$ is gradually reduced to gain a bit more energy. The longitudinal size of the beam should fill only part of the separatrix area, so that the non-linearity near its edge does not cause emittance growth and loss.

> **Think about it**: why not put the synchronous phase on the crest, $\varphi_s = 0$, and bunch the beam by other means?
>
> Answer: at $\varphi_s = 0$ the washboard has no wells and the separatrix encloses zero area: no phase deviation has a restoring force. Low-energy protons have a large $A$, a small energy difference quickly becomes a large phase difference, and the bunch falls apart within a few cavities. Either add bunching cavities between the cavities, or move the accelerating cavities themselves off the crest — the latter is the choice $\varphi_s < 0$.

> Note: the separatrix is derived for a periodic structure in which the string of cavities is "averaged into a continuous push". In a line with only one or two cavities, or in a low-energy section with a large energy gain per cavity, it is only a qualitative estimate; the true longitudinal acceptance has to be found by multi-particle simulation (scanning the entrance phase and energy).

## RF defocusing: the price of longitudinal focusing {#longitudinal-rf-defocusing}

The table above has an oddity: the $-33°$ that makes the bunch shorter also makes it fatter transversely. Why does a cavity that gathers the bunch longitudinally spread it transversely? There are two views that confirm each other.

**The view riding with the bunch**. In a frame flying along with the bunch the relative motion of the particles is slow and the cavity field is nearly electrostatic. Electrostatics has an iron rule (**Earnshaw's theorem**): in a charge-free space there cannot be an "electrostatic bowl" pushing particles towards the centre from every side. For longitudinal stability the potential along the direction of motion must be a well; then at least one transverse direction must be a slope, and for an axially symmetric cavity both are. Longitudinal focusing and transverse defocusing come from the same potential; you cannot have one without the other. It is the same kind of reason as "a [quadrupole](#quadrupole) cannot focus in both planes".

**The view of one gap**. At the gap entrance the field lines converge towards the axis and push the particle inward (focusing); at the exit they spread out and push it outward (defocusing). If the field did not change in time, the two halves would roughly cancel. But at a stable phase $\varphi_s < 0$ the field is **rising** while the particle crosses the gap (it has not reached the crest yet), so the defocusing of the exit half is stronger than the focusing of the entrance half, and the net effect is defocusing.

The transverse defocusing focal length of one cavity (derivation in [Going deeper: where RF defocusing comes from](#longitudinal-deep-defocusing))

$$
\frac{1}{f} = \frac{\pi\,qV_0T\sin(-\varphi_s)}{mc^2\,\beta^3\gamma^3\lambda} .
$$

Compared with the longitudinal one, it is exactly half. For the example cavity at $\varphi_s = -33°$, $1/f \approx 2.07\ \mathrm{m^{-1}}$, $f \approx 0.48$ m: one cavity defocuses transversely about as strongly as one quadrupole of the FODO lab focuses ($f = 0.45$ m). In the table above, the exit rms $x$ growing from 1.41 mm (drift) to 1.81 mm at $-33°$ is this effect. The linear preview integrates the field-map gradients along the actual trajectory; its transfer matrix gives $-1/M_{21} \approx 0.37$ m in $x$, 0.45 m in $y$ and 0.24 m longitudinally: the averaged formula gets the order of magnitude and the factor 2 between them right, but not the details. An HWR is not axially symmetric and $x$ and $y$ need not agree (whether the engine shows the same $x$/$y$ difference has not been checked).

Averaging over a string of cavities, the transverse "spring" becomes negative: $k_r^2 = -k_{l0}^2/2$. In the numerical example of the previous section $k_r^2 = -9.9\ \mathrm{m^{-2}}$, which solenoids or quadrupoles must overcome with stronger transverse focusing (see [Solenoid](#solenoid), [Quadrupole](#quadrupole)); this is one of the main reasons why low-energy superconducting sections alternate solenoids and cavities.

The magnetic field in the cavity also pushes the particles, but at low energy far less than the electric field (the ratio is about $\beta^2$): changing `Kb` from −1.36 to 0 in the example changes the exit rms $x$ of the linear preview by only about 0.01 % and rms $y$ by about 0.2 %. The $B$ components of a field map should still be given consistently with $E$ for the same mode.

## Think about it {#longitudinal-think}

> **Think about it 1**: a 10 MeV electron has $\gamma \approx 20.6$. Does an electron linac still need phase stability?
>
> Answer: hardly. $A \propto 1/(\beta^3\gamma^3)$, and for a 10 MeV electron $A$ is only $2\times 10^{-8}$ of the example proton's: energy differences hardly change the speed, and the particle's phase is "frozen". Electron linacs can therefore put the bunch near the crest ($\varphi_s \approx 0$) for the largest energy gain without the bunch spreading longitudinally. Low-energy protons are the opposite and must be held together by phase stability.

> **Think about it 2**: a cavity set to $\varphi_s = -90°$ gives the synchronous particle no energy at all. What is such a cavity good for?
>
> Answer: the synchronous particle passes exactly when the field is zero and rising fastest: it is not accelerated, early particles are decelerated and late ones accelerated, "head slow, tail fast", and after a drift the bunch is shorter. This is a **buncher**; the medium-energy transport uses bunchers to deliver the bunch into the accelerating section downstream (see [the medium-energy transport](#journey-mebt) and [Drift](#drift-deep-longitudinal)).

> **Think about it 3**: the example cavity is written with V3 = 2 (absolute phase) after a drift. Make that drift 5 mm longer: how much does the cavity's entrance phase change?
>
> Answer: flying 5 mm further, the proton arrives $360°\times 5/105.1 = 17.1°$ later; the entrance phase shifts by 17.1° and the synchronous phase changes with it. Written with V3 = 0, the engine solves for the entrance phase again and the synchronous phase stays the same.

> **Think about it 4**: why can the example cavity give no accelerating synchronous phase at all near 0.9 MeV?
>
> Answer: there the proton takes about one whole period from one gap to the other, and the two gaps with opposite fields cancel. Details in [Going deeper: two oddities of low-energy cavities](#longitudinal-deep-lowbeta).

## Common misconceptions {#longitudinal-misconceptions}

- **Sign of the synchronous phase**: AVAS uses the cosine convention with larger phase meaning later; the stable region is $-90° < \varphi_s < 0$. With a sine convention or a $-\omega t$ convention it is at positive phases. Port a phase with the wrong sign and the bunch spreads longitudinally while being focused transversely.
- **The synchronous phase is not the entrance phase**: for the example cavity $\varphi_s = -33°$ corresponds to an entrance phase of 52.7°, a difference of about 86° that depends on the zero of the field map and the field distribution; there is no general conversion.
- **Largest gain at $\varphi_s = 0$?** Only when the particle's speed hardly changes. In the example's low-$\beta$ cavity the largest gain is at $\varphi_s \approx +8°$, and the gains at $\pm 33°$ are asymmetric. This is not a numerical error: freeze the velocity and the asymmetry disappears; it comes from the acceleration inside the cavity.
- **Panofsky's formula is exact?** It assumes a constant velocity in the cavity; when $\Delta W/W$ reaches tens of per cent it is only an estimate.
- **V3 = 0 always finds a synchronous phase?** No. Near energies where the two gaps almost cancel there may be only a decelerating solution, or none at all; see [In AVAS](#longitudinal-in-avas).
- **A growing energy spread means the beam gets worse?** No. Adiabatic damping makes the absolute energy spread grow as $(\beta\gamma)^{3/4}$, but the phase width shrinks at the same time; the normalized longitudinal emittance is unchanged and the relative energy spread even decreases.
- **The bunch can fill the separatrix?** Near the edge of the separatrix the motion is strongly non-linear; particles circulate slowly there and filament easily, and emittance growth and losses start there. The bunch should occupy only a small part of the separatrix.
- **RF defocusing is a flaw of the cavity design?** No. It follows from the laws of electrostatics (Laplace's equation): every longitudinally stable accelerating phase comes with transverse defocusing, which can only be compensated by external focusing.
- **The cavity frequency is the beam frequency?** Not necessarily. `f` of `field` is the cavity frequency, `frequency` in `beam.txt` is the bunch repetition frequency; in frequency-doubled sections they differ.
- **Electrons behave like protons?** The phase of high-energy electrons is practically frozen ($A \propto 1/\beta^3\gamma^3$); their need for phase stability is entirely different from that of low-energy protons.

## Going deeper: the energy gain of one gap {#longitudinal-deep-gap}

The main text simply stated Panofsky's formula $\Delta W = qV_0T\cos\varphi$ and $T = \sin x/x$ for a uniform gap. This section derives them step by step, using only "work = force × distance" and trigonometric identities from school; the last paragraph is for readers who know calculus.

### Step 1: the phase the particle sees along its way

Write the on-axis field in the cavity as

$$
E_z(z, t) = E(z)\cos\left(\omega t + \varphi_0\right),
$$

where $E(z)$ is the field distribution along the axis (given by the field map), $\omega = 2\pi f$ the angle turned per second (radians) and $\varphi_0$ the cavity's RF phase. The particle reaches position $z$ at time $t(z)$ and sees the phase $\varphi(z) = \omega t(z) + \varphi_0$ there.

Assume the particle's speed hardly changes in the gap (energy gain much smaller than its energy); then its phase turns by 360° every $\beta\lambda$. Choose a reference point $z_c$ in the gap and call the phase on arrival there $\varphi$:

$$
\varphi(z) = \varphi + k\,(z - z_c),\qquad k = \frac{2\pi}{\beta\lambda}.
$$

For the example proton $k = 2\pi/0.1051 = 59.8$ radians per metre, i.e. 3.42° per millimetre.

### Step 2: cut the gap into slices and add up the work

Cut the gap into many slices of length $\Delta z$. In slice $i$ the field is $E_i\cos\varphi_i$ and the electric force does work $qE_i\cos\varphi_i\,\Delta z$ on the particle. The total energy gain is the sum over the slices:

$$
\Delta W = q\sum_i E_i\cos\varphi_i\,\Delta z .
$$

### Step 3: split it with the angle-addition formula

Write $\theta_i = k(z_i - z_c)$, so that $\varphi_i = \varphi + \theta_i$. With the school formula $\cos(\varphi + \theta) = \cos\varphi\cos\theta - \sin\varphi\sin\theta$:

$$
\Delta W = q\cos\varphi\sum_i E_i\cos\theta_i\,\Delta z \;-\; q\sin\varphi\sum_i E_i\sin\theta_i\,\Delta z .
$$

The two sums depend only on the field distribution and the particle's speed, not on $\varphi$.

### Step 4: choose the reference point well and the second term vanishes

$z_c$ can always be placed so that the second sum is zero; this point is the **electrical centre** of the gap. For a symmetric gap it is the geometric centre: $\sin\theta$ has opposite signs on the two sides and cancels. Then

$$
\Delta W = qV_0T\cos\varphi,\qquad
V_0 = \sum_i E_i\,\Delta z,\qquad
T = \frac{\sum_i E_i\cos\theta_i\,\Delta z}{\sum_i E_i\,\Delta z}.
$$

$V_0$ is the "voltage" of the gap (the field added up along the axis) and $T$ the average of $\cos\theta$ weighted by the field strength, so $T \le 1$. This is Panofsky's formula; $E_0 = V_0/L$ is called the mean on-axis field over the gap length $L$, and $V_0T = E_0TL$.

### Step 5: a uniform gap

If the field is the same everywhere in a gap of length $g$, $T$ is the plain average of $\cos\theta$, with $\theta$ running evenly from $-x$ to $+x$, $x = kg/2 = \pi g/(\beta\lambda)$. The average of $\cos\theta$ between $-x$ and $x$ is $\sin x/x$ (proved with calculus in the last paragraph; you can also check it with a calculator by averaging $\cos$ over many evenly spaced points in the interval). For $g = \beta\lambda/2$, $x = \pi/2$: one complete "arch" of $\cos$, with mean height $2/\pi = 0.64$, which is the result of the main text.

### Step 6: two gaps with opposite fields

Two identical gaps with centres a distance $d$ apart and opposite fields. Put the first gap at $z_c - d/2$ and the second at $z_c + d/2$, each with its own voltage $V_g$ and transit-time factor $T_g$. By step 4 the two gaps give

$$
\Delta W_1 = qV_gT_g\cos\!\left(\varphi - \tfrac{kd}{2}\right),\qquad
\Delta W_2 = -qV_gT_g\cos\!\left(\varphi + \tfrac{kd}{2}\right)
$$

(the second with a minus sign because its field is reversed). With the sum-to-product formula $\cos(a - b) - \cos(a + b) = 2\sin a\sin b$:

$$
\Delta W = 2qV_gT_g\,\sin\frac{kd}{2}\,\sin\varphi .
$$

With the reference point midway between the gaps the largest gain is at $\varphi = 90°$; shifting the zero of phase by 90° brings back the $\cos$ form. With $V_0 = 2V_g$ (the sum of the absolute gap voltages; for an alternating field $\sum E\,\Delta z \approx 0$, so $\int\lvert E\rvert\,\dd z$ is used instead, as in the table of the main text) we get

$$
T(\beta) = T_g(\beta)\left\lvert\sin\frac{kd}{2}\right\rvert = T_g(\beta)\left\lvert\sin\frac{\pi\beta_g}{2\beta}\right\rvert,\qquad d = \frac{\beta_g\lambda}{2}.
$$

Three conclusions: at $\beta = \beta_g$, $\sin(\pi/2) = 1$ and the two gaps are perfectly in step; away from it $T$ drops; at $kd/2 = \pi$, $2\pi$, … the sine is zero and the two gaps cancel completely — then $d = \beta\lambda$, $2\beta\lambda$, …, i.e. the particle flies a whole number of periods between the gaps. For the example cavity $d = 0.081$ m; $d = \beta\lambda$ corresponds to $\beta = 0.0439$ (about 0.90 MeV), $d = 2\beta\lambda$ to about 0.23 MeV. For a cavity with $N$ gaps $T(\beta)$ is sharper.

### Step 7: the exact definition when the speed changes noticeably

When the energy gain is comparable with the energy (low-energy protons, heavy ions), the constant-speed assumption fails and the $\varphi(z)$ of step 1 is no longer a straight line. The split of step 3 still works, but the sums must follow the reference particle's **actual** arrival time $t(z)$:

$$
C = \sum_i E_i\cos\varphi_i\,\Delta z,\qquad S = \sum_i E_i\sin\varphi_i\,\Delta z,\qquad
\Delta W = q\sqrt{C^2 + S^2}\,\cos\varphi_s,\qquad \varphi_s = \operatorname{atan2}(S, C).
$$

Think of $(C, S)$ as an arrow in a plane: $\sqrt{C^2 + S^2}$ is its length (the "effective voltage") and $\operatorname{atan2}(S, C)$ its angle with the horizontal axis (an arctangent that keeps track of the quadrant). The first sum is itself the energy gain, $\Delta W = qC$, and the expression with $\varphi_s$ says the same thing in another form, so it is exact. This $\varphi_s$ is what AVAS calls the synchronous phase. The price: the effective voltage itself depends on the trajectory and therefore on $\varphi_s$, so the energy gain no longer follows $\cos\varphi_s$ exactly.

### For readers who know calculus

Replace the sums by integrals: $\Delta W = q\int E(z)\cos\varphi(z)\,\dd z$ and $T = \int E(z)\cos k(z - z_c)\,\dd z\big/\int E\,\dd z$, i.e. the Fourier transform of the field distribution at the spatial frequency $k = 2\pi/(\beta\lambda)$ (real part about the electrical centre). For a uniform gap $\frac{1}{2x}\int_{-x}^{x}\cos\theta\,\dd\theta = \sin x/x$. For the two-gap field $E(z) = e(z - d/2) - e(z + d/2)$ the shift theorem gives $\tilde E(k) = -2i\sin(kd/2)\,\tilde e(k)$, which is the factor $\lvert\sin(kd/2)\rvert$ above. In the exact definition $C + iS = \int E(z)\,\ee^{i\varphi(z)}\,\dd z$.

## Going deeper: two oddities of low-energy cavities {#longitudinal-deep-lowbeta}

The example cavity is too "short" for the example proton: the proton's energy grows by about 30 % inside it, and $\beta$ rises from 0.057 to about 0.063–0.065. This produces two effects that do not fit the simple cosine. This section takes them apart.

### Step 1: the largest gain is not at 0°, and the signs are not symmetric

In the table of the main text $\Delta W/\cos\varphi_s$ is 0.405 MeV at −33° and 0.475 MeV at 0°, a difference of about 15 %; the largest gain found by scanning the entrance phase (exit 2.0087 MeV) is at $\varphi_s \approx +8°$; the gain at $+33°$, 0.424 MeV, exceeds the 0.340 MeV at $-33°$.

### Step 2: freeze the velocity and the oddities disappear

Integrate the same field map once more with the speed held at its entrance value: at all phases $\Delta W = qV_0T\cos\varphi_s$ then holds within 1 %, with $V_0T = 0.389$ MV, 0.325 MeV at $-33°$, 0.328 MeV at $+33°$, and the maximum right at $\varphi_s \approx 0$. The asymmetry comes entirely from the noticeable acceleration inside the cavity.

### Step 3: look at the two gaps separately

The example proton needs about 0.77 of a period from one gap centre to the other, not the ideal half period, so the two gaps cannot both be near the crest.

- $\varphi_s = -33°$: the first gap is at about −81° and gives only 0.043 MeV; the second is near the crest and gives 0.297 MeV. $\varphi_s$ is a weighted average of the two.
- $\varphi_s = +33°$: the other way round. The first gap is near the crest and gives 0.292 MeV first. The particle then becomes noticeably faster and reaches the second gap earlier, where the phase moves from about 79° after the crest to about 66°, and the gain there grows from 0.054 MeV to 0.132 MeV.

Where in the cavity the energy is gained decides how much the timing of the second half shifts — this is the physical meaning of "the effective voltage depends on the phase". For such low-energy, high-gain cavities the constant-speed Panofsky formula is only an estimate, and design must rely on integration along the true trajectory.

### Step 4: the synchronous phase may have no solution

Think of the $(C, S)$ of step 7 in the previous section as the sum of two arrows: one for the first gap and one for the second (pointing the opposite way). At constant speed, changing the entrance phase $\varphi_0$ turns both arrows together, the resultant arrow turns a full circle too, and every synchronous phase can be reached.

When the speed changes it is different: how much energy the first gap gives depends on $\varphi_0$, which decides when the particle reaches the second gap, so the second arrow no longer turns in step with the first. Near speeds where the two gaps almost cancel (step 6 of the previous section: flying about a whole number of periods between the gaps), the resultant is the difference of two large arrows and is short; a small wobble of the second arrow is enough to swing it to and fro instead of round a full circle. Then, scanning all $\varphi_0$, $\varphi_s = \operatorname{atan2}(S, C)$ covers only a range of angles. By the linear preview (example cavity, $K_e = 1.36$):

| Entrance energy | reachable range of $\varphi_s$ |
|---|---|
| 0.3 MeV | about 65°–107° |
| 0.8 MeV | about 58°–136° |
| 1.0 MeV | about 14°–144° |

Asking for $\varphi_s = -30°$, by the linear preview:

- 0.4–0.65 MeV and above 1.1 MeV: an accelerating solution exists;
- 0.7–0.75 MeV and 1.05 MeV: no accelerating solution, but one 180° away ($\varphi_s = 150°$, the cavity **decelerates** the beam), which the engine accepts;
- 0.15–0.35 MeV and 0.8–1.0 MeV: neither exists, and the engine refuses to run.

Compare with the rough estimate of step 6: complete cancellation of the two gaps at about 0.23 MeV and 0.90 MeV falls right inside the two "no solution" ranges.

The lab below sets the entrance energy to 0.7 MeV and the synchronous phase to −30°: look at the warning, the negative energy gain (about −0.113 MeV), the synchronous phase shown as 150° and the entrance phase of about −107.8°. Drag the energy to 0.5 MeV and the accelerating solution is back (gain about +0.072 MeV); drag it to 0.8 MeV and neither solution exists — the preview takes the closest reachable value and says that the engine will report an error.

```avas-lab
rf-cavity phase=-30 Ke=1.36 W=0.7
```

What the AVAS engine does in these cases is described in [In AVAS](#longitudinal-in-avas).

### For readers who know calculus

$C + iS = \int E(z)\,\ee^{i\varphi(z;\varphi_0)}\,\dd z$ with $\varphi(z;\varphi_0) = \varphi_0 + \omega t(z;\varphi_0)$. At constant speed $t$ does not depend on $\varphi_0$, $C + iS = \ee^{i\varphi_0}\times$ const, and the map $\varphi_0 \mapsto \varphi_s$ is a shift with winding number 1. When $t$ depends on $\varphi_0$ the curve $\varphi_0 \mapsto C + iS$ need not wind around the origin; when it does not enclose the origin the range of $\varphi_s(\varphi_0)$ is an arc and some $\varphi_s$ have no solution; with several roots the engine takes the one on the rising branch of $\varphi_s(\varphi_0)$.

## Going deeper: the equations of phase oscillation {#longitudinal-deep-oscillation}

The main text worked out the restoring force for one particle arriving 1° late. This section writes it as equations and derives synchrotron oscillations and adiabatic damping. Tools: the relation between speed and energy, simple harmonic motion, the area of an ellipse.

### Step 1: two symbols

$$
\Delta\varphi = \varphi - \varphi_s,\qquad w = \frac{W - W_s}{mc^2}.
$$

$\Delta\varphi$ is how much later (in phase) the particle arrives than the synchronous particle, $w$ its extra energy (in units of the rest energy $mc^2$, dimensionless).

### Step 2: an energy difference becomes a phase difference

The particle covers $\Delta s$ in a time $\Delta s/v$, less than the synchronous particle by $\Delta s/v_s - \Delta s/v \approx \Delta s\cdot\Delta v/v^2$. Its phase advances less by $\omega$ times this time. Velocity difference and energy difference are related by (derivation in [Drift](#drift-deep-longitudinal) and [Relativistic kinematics](#kinematics))

$$
\frac{\Delta v}{v} = \frac{1}{\gamma^2}\frac{\Delta p}{p} = \frac{w}{\beta^2\gamma^3},
$$

which at low speed is just $\tfrac12\Delta W/W$. Together, the change of phase per metre is

$$
\frac{\Delta(\Delta\varphi)}{\Delta s} = -\frac{\omega}{\beta c}\cdot\frac{w}{\beta^2\gamma^3} = -\frac{2\pi}{\beta^3\gamma^3\lambda}\,w \equiv -A\,w .
$$

(using $\omega/c = 2\pi/\lambda$). Example: $\beta^3\gamma^3 = 1.860\times 10^{-4}$, $A = 2\pi/(1.860\times 10^{-4}\times 1.8449) = 1.83\times 10^4\ \mathrm{m^{-1}}$.

### Step 3: a phase difference becomes an energy difference

Average the action of a string of cavities over unit length, mean accelerating gradient $E_0T$. Per metre the synchronous particle gains $qE_0T\cos\varphi_s$ and a particle late by $\Delta\varphi$ gains $qE_0T\cos\varphi$; the difference is

$$
\frac{\Delta w}{\Delta s} = \frac{qE_0T}{mc^2}\left(\cos\varphi - \cos\varphi_s\right) \equiv B\left(\cos\varphi - \cos\varphi_s\right).
$$

### Step 4: small deviations give simple harmonic motion

For small deviations $\cos\varphi - \cos\varphi_s \approx -\sin\varphi_s\,\Delta\varphi$ (the tangent of the cosine at $\varphi_s$; with derivatives, $(\cos\varphi)' = -\sin\varphi$). Step 2 says the rate of change of the phase is proportional to $-w$; step 3 says the rate of change of $w$ is proportional to $-\sin\varphi_s\,\Delta\varphi$. Write "the rate of change of the rate of change" as $\Delta\varphi''$ (second derivative with respect to $s$, as acceleration is to position):

$$
\Delta\varphi'' = -A\,w' = AB\sin\varphi_s\,\Delta\varphi .
$$

Compare with a mass on a spring, $x'' = -(k/m)\,x$: for $\sin\varphi_s < 0$ the right-hand side is a restoring force and the motion is simple harmonic,

$$
\Delta\varphi = a\cos(k_{l0}s),\qquad k_{l0}^2 = -AB\sin\varphi_s = \frac{2\pi\,qE_0T\sin(-\varphi_s)}{mc^2\,\beta^3\gamma^3\lambda}.
$$

Example numbers: $B = 1.86/938.27 = 1.98\times 10^{-3}\ \mathrm{m^{-1}}$, $k_{l0}^2 = 1.83\times 10^4\times 1.98\times 10^{-3}\times 0.545 = 19.8\ \mathrm{m^{-2}}$, $k_{l0} = 4.44\ \mathrm{m^{-1}}$, as in the main text. For $\sin\varphi_s > 0$ the right-hand side amplifies the deviation and the particle runs away exponentially. The longitudinal phase advance per focusing period $L_p$ is $\sigma_{0l} = k_{l0}L_p$.

### Step 5: phase amplitude and energy amplitude

From step 2, $w = -\Delta\varphi'/A$. For $\Delta\varphi = a\cos(k_{l0}s)$, $\Delta\varphi' = -ak_{l0}\sin(k_{l0}s)$, so the amplitude of $w$ is $ak_{l0}/A$:

$$
\frac{w_{\max}}{\Delta\varphi_{\max}} = \frac{k_{l0}}{A}.
$$

Concentrate one cavity into a thin lens: after the cavity a particle late by $\Delta\varphi$ has gained $w = (qV_0T/mc^2)\sin(-\varphi_s)\Delta\varphi$ extra; after a further distance $s$ its phase has changed by $-Aws$; the $s$ at which this change equals $-\Delta\varphi$ is the focal length: $1/f_z = A\,qV_0T\sin(-\varphi_s)/(mc^2)$, in the example $1.83\times 10^4\times(0.39/938.27)\times 0.545 = 4.1\ \mathrm{m^{-1}}$.

### Step 6: converting to AVAS's (z, δ)

In one period the particle flies $\beta\lambda$, so a phase difference of 360° corresponds to a distance $\beta\lambda$, and the late (larger phase) particle is behind (smaller $z$): $z = -\beta\lambda\,\Delta\varphi/(2\pi)$. The relation between momentum and energy, $\Delta p/p = \Delta W/(\beta^2\gamma\,mc^2)$, gives $\delta = w/(\beta^2\gamma)$. $z$ and $\Delta\varphi$ differ only by a constant factor and obey the same oscillation equation. The example's rms $z = 1.316$ mm corresponds to $360°\times 1.316/105.1 = 4.5°$; rms $\delta = 3.53$ mrad corresponds to $3.53\times 10^{-3}\times\beta^2\gamma\,mc^2 = 3.53\times 10^{-3}\times 3.05\ \mathrm{MeV} = 10.8$ keV.

### Step 7: adiabatic damping

A simple harmonic oscillation traces an ellipse in the "phase–energy" plane with half-axes $\Delta\varphi_{\max}$ and $w_{\max}$ and area $\pi\,\Delta\varphi_{\max}w_{\max}$. When $\beta\gamma$ and $E_0T$ change only a little during one oscillation ("adiabatically"), this area stays constant — like a pendulum whose string is slowly shortened. Combining the two relations:

$$
\Delta\varphi_{\max}\,w_{\max} = \text{const},\qquad \frac{w_{\max}}{\Delta\varphi_{\max}} = \frac{k_{l0}}{A} = \sqrt{\frac{qE_0T\sin(-\varphi_s)\,\lambda\,\beta^3\gamma^3}{2\pi\,mc^2}} .
$$

(The second follows by inserting the $k_{l0}$ of step 4 and the $A$ of step 2.) Write $X = E_0T\sin(-\varphi_s)\,\lambda\,\beta^3\gamma^3$: multiplying the two gives $w_{\max}^2 \propto X^{1/2}$, dividing gives $\Delta\varphi_{\max}^2 \propto X^{-1/2}$, and taking square roots:

$$
\Delta\varphi_{\max} \propto \left[E_0T\sin(-\varphi_s)\,\lambda\,\beta^3\gamma^3\right]^{-1/4},\qquad
\Delta W_{\max} \propto \left[E_0T\sin(-\varphi_s)\,\lambda\,\beta^3\gamma^3\right]^{1/4}.
$$

With constant gradient, phase and frequency, $\Delta\varphi \propto (\beta\gamma)^{-3/4}$ and $\Delta W \propto (\beta\gamma)^{3/4}$. Checking the example of the main text: a 10 MeV proton has $\beta\gamma = 0.1464$, a ratio of 2.565 to 0.05708; $2.565^{-3/4} = 0.49$, $2.565^{3/4} = 2.03$, and the relative energy spread $2.03\times 1.527/10 = 0.31$. In $(z, \delta)$, $\beta\gamma\cdot z\,\delta \propto \lambda\,\Delta\varphi\,\Delta W$ stays constant: the normalized longitudinal emittance is conserved.

### For readers who know calculus

Steps 2 and 3 are $\dd\Delta\varphi/\dd s = \omega(1/\beta c - 1/\beta_s c) \approx -Aw$ and $\dd w/\dd s = B(\cos\varphi - \cos\varphi_s)$, using $\Delta\beta = \Delta\gamma/(\beta\gamma^3)$. $(\Delta\varphi, w)$ is a canonical pair, and the action $J = \oint w\,\dd\Delta\varphi$ is an adiabatic invariant when the parameters vary slowly (the Hamiltonian is given in the next section). In the linearized second-order equation $\Delta\varphi'' + (A'/A)\Delta\varphi' + k_{l0}^2\Delta\varphi = 0$ the damping term comes from $A$ varying with $s$; the WKB solution gives the same $-1/4$ power law.

## Going deeper: the equations of the separatrix {#longitudinal-deep-separatrix}

The main text described large-amplitude longitudinal motion as "a tilted washboard". This section shows that the picture is mathematically exact and derives the separatrix and the size of the stable region. Tools: conservation of mechanical energy, and derivatives as learnt in school.

### Step 1: a conserved quantity

For a ball rolling on a slope, kinetic + potential energy is conserved: $\tfrac12 m v^2 + U(x) = $ const. Longitudinal motion has such a conserved quantity too:

$$
\frac{A}{2}w^2 + U(\varphi) = \text{const},\qquad U(\varphi) = B\left(\sin\varphi - \varphi\cos\varphi_s\right).
$$

$\tfrac{A}{2}w^2$ plays the "kinetic energy" and $U$ the "potential energy". Check: differentiate with respect to $s$, using $(\sin\varphi)' = \cos\varphi$ and the two equations of the previous section, $\varphi' = -Aw$ and $w' = B(\cos\varphi - \cos\varphi_s)$:

$$
A\,w\,w' + B(\cos\varphi - \cos\varphi_s)\,\varphi' = A w B(\cos\varphi - \cos\varphi_s) - B(\cos\varphi - \cos\varphi_s)Aw = 0 .
$$

$U$ has two parts: $B\sin\varphi$ is the sinusoidal ripple (the RF field) and $-B\varphi\cos\varphi_s$ the linear tilt (the acceleration the synchronous particle must receive). That is the tilted washboard.

### Step 2: bottom of the well and top of the ridge

Where the ball is at rest ($w = 0$) and feels no force, the slope of the "potential" is zero: $U'(\varphi) = B(\cos\varphi - \cos\varphi_s) = 0$, i.e. $\cos\varphi = \cos\varphi_s$. Within one period there are two solutions:

- $\varphi = \varphi_s$: the bottom of the well, stable (a ball resting there, nudged, rolls back and forth nearby);
- $\varphi_2 = -\varphi_s$: the top of the ridge, unstable (like a ball balanced on a hilltop, which rolls off at the slightest touch).

### Step 3: the separatrix

The trajectory passing through the ridge top is the separatrix: its "total energy" equals that of a ball resting on the ridge top, $U(-\varphi_s)$:

$$
\frac{A}{2}w^2 + B\left(\sin\varphi - \varphi\cos\varphi_s\right) = -B\left(\sin\varphi_s - \varphi_s\cos\varphi_s\right).
$$

Particles with a smaller total energy are trapped in the well and oscillate in a bounded way; those with more go over the ridge and their phase slips away for good.

### Step 4: the height of the stable region

The energy offset is largest at the bottom of the well, $\varphi = \varphi_s$ (lowest potential, largest "kinetic energy"). There the separatrix has

$$
\frac{A}{2}w_{\max}^2 = U(-\varphi_s) - U(\varphi_s) = 2B\left(\varphi_s\cos\varphi_s - \sin\varphi_s\right).
$$

Converting back to energy, $\Delta W = mc^2 w$, with $A = 2\pi/(\beta^3\gamma^3\lambda)$ and $B = qE_0T/(mc^2)$:

$$
\Delta W_{\max} = 2\left[\frac{qE_0T\,\beta^3\gamma^3\,\lambda\, mc^2\left(\varphi_s\cos\varphi_s - \sin\varphi_s\right)}{2\pi}\right]^{1/2}.
$$

Check for $\varphi_s = -33°$ ($-0.576$ radians): $\varphi_s\cos\varphi_s - \sin\varphi_s = -0.483 + 0.545 = 0.0615$; $qE_0T\beta^3\gamma^3\lambda\,mc^2 = 1.86\times 1.860\times 10^{-4}\times 1.8449\times 938.27 = 0.599\ \mathrm{MeV^2}$; $\Delta W_{\max} = 2\sqrt{0.599\times 0.0615/(2\pi)} = 0.153$ MeV, as in the table of the main text.

### Step 5: the width of the stable region

The left end $\varphi_1$ of the separatrix also lies on $w = 0$, so $U(\varphi_1) = U(-\varphi_s)$:

$$
\sin\varphi_1 - \varphi_1\cos\varphi_s = -\sin\varphi_s + \varphi_s\cos\varphi_s .
$$

This equation has no simple solution, but trying a few values on a calculator, or bisection, solves it: for $\varphi_s = -33°$, $\varphi_1 = -67.2°$, total width $33° + 67.2° = 100°$. In all three rows of the table the width is about 3 times $\lvert\varphi_s\rvert$: that is "a width of about $3\lvert\varphi_s\rvert$ for small $\lvert\varphi_s\rvert$".

### Step 6: the limits of the picture

The analogy is mathematically exact, with two limits: $A$ and $B$ change slowly with energy (the well slowly deforms; during acceleration it gets deeper and the particle's amplitude relative to the bottom shrinks — adiabatic damping), and real cavities are discrete, so "averaging into a continuous push" only holds when the longitudinal phase advance per period is not large.

### For readers who know analytical mechanics

With $s$ as the independent variable and $\varphi$, $w$ conjugate, the Hamiltonian is

$$
H(\varphi, w) = -\frac{A w^2}{2} - B\left(\sin\varphi - \varphi\cos\varphi_s\right),
$$

with $\varphi' = \partial H/\partial w = -Aw$ and $w' = -\partial H/\partial\varphi = B(\cos\varphi - \cos\varphi_s)$. The separatrix is the level curve through the hyperbolic fixed point $(-\varphi_s, 0)$, and the area it encloses is the longitudinal acceptance; as $\varphi_s \to 0$ the area goes to zero, at $\varphi_s \to -90°$ it becomes a stationary bucket (no acceleration).

## Going deeper: where RF defocusing comes from {#longitudinal-deep-defocusing}

The main text explained RF defocusing with "Earnshaw's theorem" and "focusing at the entrance, defocusing at the exit". This section derives the focal length. Tools: "field lines in equal field lines out", a changing electric field produces a magnetic field, the Lorentz force.

### Step 1: a changing longitudinal field implies a transverse field

Near the axis take a small cylinder of radius $r$ and length $\Delta z$ (there is no charge in the gap). The field lines entering it must equal those leaving it (the same as "rule 1" of the [quadrupole](#quadrupole-deep-field) deeper section, but for the electric field). The difference between the two end faces is $\pi r^2\left[E_z(z + \Delta z) - E_z(z)\right]$, the side contributes $2\pi r\,\Delta z\,E_r$. The two add up to zero:

$$
E_r = -\frac{r}{2}\,\frac{E_z(z + \Delta z) - E_z(z)}{\Delta z} = -\frac{r}{2}\frac{\partial E_z}{\partial z}.
$$

$\partial E_z/\partial z$ is the rate of change of $E_z$ along the axis (at fixed time). At the entrance $E_z$ grows along the axis, $E_r < 0$ points to the axis (focusing); at the exit $E_z$ decreases and $E_r > 0$ (defocusing). The transverse field is proportional to the distance $r$ from the axis, so it is a linear lens.

### Step 2: a changing electric field produces a magnetic field

Maxwell discovered that a changing electric field produces a magnetic field just as a current does (mentioned in school with electromagnetic waves). On a circle of radius $r$ the magnetic field circles the axis:

$$
B_\theta = \frac{r}{2c^2}\frac{\partial E_z}{\partial t}.
$$

### Step 3: the radial force

The radial force on the particle is $F_r = q(E_r - vB_\theta)$. Along the particle's path, at position $z$ and time $t$, $E_z$ changes both because the position changes and because time passes. Writing the two together as $\dd E_z/\dd z = \partial E_z/\partial z + \partial E_z/\partial t\cdot\frac{1}{\beta c}$ and rearranging,

$$
F_r = -\frac{qr}{2}\left[\frac{\dd E_z}{\dd z} - \frac{1}{\beta c\,\gamma^2}\frac{\partial E_z}{\partial t}\right].
$$

Added up over the whole gap, the first term is "the total change of the $E_z$ seen by the particle", from zero (before the gap) back to zero (after it), so it sums to zero (assuming $r$ and $\beta$ are nearly constant in the gap). Only the second term remains: the change of the field in time. At the particle's arrival it is proportional to $-\omega\sin\varphi$; for $\varphi < 0$ the field is rising.

### Step 4: the focal length

Adding up the second term over the gap (the weighted average again gives the transit-time factor $T$) yields the change of transverse momentum and of angle:

$$
\Delta p_r = -\frac{\pi qE_0TL\,r\sin\varphi}{c\,\beta^2\gamma^2\lambda},\qquad
\Delta r' = \frac{\Delta p_r}{p} = -\frac{\pi qE_0TL\sin\varphi}{mc^2\,\beta^3\gamma^3\lambda}\,r .
$$

For $\sin\varphi < 0$ (longitudinally stable) $\Delta r'$ has the same sign as $r$: the gap is a defocusing lens, $1/f = \pi qE_0TL\sin(-\varphi)/(mc^2\beta^3\gamma^3\lambda)$. For the example, $V_0T = E_0TL = 0.39$ MV and $\varphi_s = -33°$: $1/f = \tfrac12\times 4.1 = 2.07\ \mathrm{m^{-1}}$. Longitudinal focusing and transverse defocusing come from the same $\sin\varphi$; you cannot have one without the other at the same phase.

### Step 5: the three directions add up to zero

Compare the defocusing per metre with the longitudinal $k_{l0}^2$ of the previous section:

$$
k_r^2 = -\frac{\pi qE_0T\sin(-\varphi_s)}{mc^2\,\beta^3\gamma^3\lambda} = -\frac{k_{l0}^2}{2}.
$$

$k_x^2 + k_y^2 + k_z^2 = 0$: however much the beam is focused longitudinally, each transverse direction is defocused by half as much. This is the quantitative form of Earnshaw's theorem. The magnetic term $vB_\theta$ is about $\beta^2$ times the electric one, small at low energy; the $1/\gamma^2$ of step 3 is exactly the result of subtracting the magnetic part from the electric part.

### For readers who know vector calculus

Step 1 is $\nabla\cdot\boldsymbol E = 0$ expanded near the axis, step 2 is $\nabla\times\boldsymbol B = c^{-2}\partial_t\boldsymbol E$. In the beam rest frame the field is nearly electrostatic and $\nabla^2\Phi = 0$ makes the three focusing constants add up to zero. The more general Panofsky–Wenzel theorem, $\Delta\boldsymbol p_\perp \propto \nabla_\perp\Delta p_z$ (integrated along a straight trajectory), links the transverse kick to the transverse gradient of the longitudinal kick, and likewise shows that an energy gain varying with transverse position necessarily comes with a transverse deflection.

## In AVAS {#longitudinal-in-avas}

### Keywords

An RF cavity is a `field` element with `type = 1`:

```avas-params
field input:scanphase input:longlimits beam:twissz beam:frequency
```

```text
start
! L     R     V3 type f        phase Ke   Kb    file
field 0.21  0.02  0  1    162.5e6  -33   1.36 -1.36 hwr010
end
```

- `f` is the cavity frequency; `frequency` in `beam.txt` is the beam frequency (bunch repetition frequency, which sets the bunch charge $Q = I/f$); the two can differ (e.g. in frequency-doubled sections).
- `Ke` scales the electric field (map units MV/m), `Kb` the magnetic field; to change the accelerating gradient change both in proportion. The example HWR uses $K_b = -K_e$ (so does the RF cavity lab of this primer).
- A cavity's phase comes only from `phase` and V3 of its field line. No kernel reads `scanphase` in `input.txt` or `scanData.txt` (the original manual says they control a phase scan); the Simulation settings page removes a leftover `scanphase` line when saved.
- `longlimits` sets the phase and energy limits for longitudinal loss, the counterpart of the separatrix above: particles beyond the limits are counted as longitudinal losses.
- The output file `Phase.txt` gives, in pairs of lines, the time, position and synchronous-particle energy at the entrance and exit of each RF cavity, useful to check each cavity's energy gain.
- `twissz` in `beam.txt` holds the longitudinal Twiss parameters: $\alpha_z$, $\beta_z$ (mm/mrad, with $z$ in mm and $\delta = \Delta p/p$ in mrad) and the normalized rms longitudinal emittance (π·mm·mrad), consistent with the $(z, \delta)$ conversion above.

### Convention for RF fields and phases

> **AVAS convention**: the RF electric field acts as $K_e\,E(\boldsymbol r)\cos(\omega t + \varphi_0)$ and the RF magnetic field as $K_b\,B(\boldsymbol r)\sin(\omega t + \varphi_0)$ ($E$ in MV/m, $B$ in T in the maps). This matches the cosine convention of Wangler's *RF Linear Accelerators*. The longitudinal coordinate relates to the phase by $\Delta\varphi = -2\pi z/(\beta\lambda)$: a particle with $z > 0$ is ahead of the reference particle and has a smaller phase.

### Three meanings of the phase (V3)

In `field L R V3 type f phase Ke Kb file` the third parameter V3 sets the meaning of the sixth parameter `phase`:

| V3 | Meaning of `phase` | How the engine sets the entrance phase $\varphi_0$ |
|---|---|---|
| 0 | synchronous phase $\varphi_s$ | solves for $\varphi_0$ so that $\operatorname{atan2}(S, C) = \varphi_s$ along the reference trajectory, see step 7 of [Going deeper](#longitudinal-deep-gap) |
| 1 | RF phase when the reference particle reaches the element entrance | $\varphi_0$ = `phase` |
| 2 | RF phase at $t = 0$ (absolute phase) | $\varphi_0 = $ `phase` $+ 360°\cdot f\cdot t_{\mathrm{in}}$, with $t_{\mathrm{in}}$ the arrival time of the reference particle at the entrance |

V3 = 0 is the most convenient for design: the synchronous phase is what matters physically, and the code finds the entrance phase. V3 = 1 suits phase scans (the entrance phase is tied directly to the cavity geometry). V3 = 2 corresponds to a real machine, where each cavity's RF is phased against a common reference clock; changes upstream then change the synchronous phase. In the engine's output `synData.txt`, $\varphi_{\mathrm{RF}}$ = `phase_t0` $+ 360°\cdot f\cdot t_{\mathrm{in}}$.

Example: put a 0.1 m drift before the example cavity. The reference particle reaches the cavity entrance at $t_{\mathrm{in}} = 0.1/(\beta c) = 5.854\ \mathrm{ns}$, $360°\,f\,t_{\mathrm{in}} = 342.45°$. With V3 = 2 and `phase = 0` the entrance phase is $342.45° \equiv -17.55°$, a synchronous phase of about −93.7°: the cavity hardly accelerates. With V3 = 0 and `phase = -33`, the entrance phase is 52.7° and the synchronous phase stays −33° whatever the length of the drift.

> **AVAS convention**: in the example the synchronous phase $-33°$ corresponds to an entrance phase of $52.7°$; the difference (about 86°) depends on the zero of the field map and the field distribution, and there is no general conversion. For V3 = 0 cavities the linear preview shows the entrance phase it found together with the synchronous phase. When a segment run takes a piece of the lattice on its own, AVAS takes each cavity's RF phase $\varphi_{\mathrm{RF}}$ from the synchronous-particle data of the reference run and converts it to a V3 = 2 phase with the segment entrance as time zero, $\varphi_{\mathrm{RF}} - 360°\cdot f\cdot(t_{\mathrm{in}} - T_{\mathrm{entry}})$, instead of keeping the original phase or switching to V3 = 0.

### The V3 = 0 synchronous phase may have no solution

With V3 = 0 the synchronous phase is defined as $\varphi_s = \operatorname{atan2}(\int E\sin\varphi, \int E\cos\varphi)$. For the example hwr010 ($K_e = 1.36$), at speeds where the two gaps almost cancel, scanning all entrance phases gives $\varphi_s$ only within a range of angles (0.3 MeV about 65°–107°, 0.8 MeV about 58°–136°, 1.0 MeV about 14°–144°). Taking $\varphi_s = -30°$ as an example (checked with the Windows engine on 2026-11-02):

- By the linear preview, an accelerating −30° solution exists at 0.4–0.65 MeV and above 1.1 MeV.
- **Without an accelerating solution the engine accepts the solution 180° away**: at 0.7 MeV the entrance phase is −107.7° and the synchronous particle is decelerated by 0.1129 MeV, while `synData.txt` still records $\varphi_s = -29.92°$ (atan2 is actually 150°); the linear preview gives −107.79° and −0.11286 MeV. By the linear preview this happens at 0.7–0.75 MeV and 1.05 MeV (1.05 MeV also checked: entrance phase −165.8°, synchronous particle decelerated by 0.095 MeV, preview within 0.02° and 0.005 %). With several roots the engine takes the one on the rising branch of $\varphi_s(\varphi_0)$.
- **When neither exists (0.15–0.35, 0.8–1.0 MeV) the engine refuses to run** with "Initialisation stage error: … the scan phase to fail to find the corresponding RF phase" (checked at 0.3, 0.8 and 1.0 MeV).

The linear preview first runs a fixed-point iteration (which converges whenever a solution exists); if it does not converge it scans $\varphi_0$ and bisects for a root, looking first for an accelerating solution, then for one 180° away (with a "decelerating" warning); when neither exists it warns, gives the reachable range and takes the closest $\varphi_s$. When you see such a warning, change the entrance energy or `Ke`, or use V3 = 1 to set the entrance phase directly.

### Energy: synchronous particle versus the mean of all particles

The linear preview's reference particle and the engine's synchronous particle (`synData.txt`) agree in energy to about 0.01 %. But **column 0 of DataSet.txt is the mean energy of all macro-particles**: off axis, particles see a different $E_z$ in the 3D field map; in the example hwr010 the gain 2 mm off axis is 4–6 % higher than on axis, and the engine's mean gain is therefore about 4.5 % above the preview (1.527 MeV entrance: 0.3556 versus 0.3395 MeV); at 0.5 MeV, where the two gaps almost cancel, the engine's mean gain is instead about 20 % lower. When comparing energies, distinguish the synchronous particle from the mean.

### Longitudinal model of the linear preview

The linear preview integrates the reference particle's energy and time along the axis in 1 mm steps, finds the entrance phase of V3 = 0 cavities as described in the previous subsection, and pushes the second moments with the linear bunching in $(z, \delta)$ and the damping from the momentum gain. It contains no longitudinal non-linearity and so cannot show the growth of the longitudinal emittance in low-energy cavities; rms sizes typically agree to 1–2 %. See [Multi-particle simulation](#simulation).

### Common problems

- **Phase sign**: a phase ported from another code with the opposite sign makes the bunch spread while being focused transversely; compare with the ±33° table above.
- **Cavity β not matched to the beam**: when $T(\beta)$ is small the cavity hardly accelerates and the phase is very sensitive to the energy; check first that the energy gain is reasonable.
- **V3 = 2 cavities dephased after upstream changes**: changing a drift length or the upstream energy changes $t_{\mathrm{in}}$ and with it the synchronous phase; prefer V3 = 0 during design.
- **The engine reports a "scan phase" error**: the V3 = 0 synchronous phase has no solution at that entrance energy; see [above](#longitudinal-in-avas).
- **Large phase width**: when the bunch's phase width approaches the separatrix width, longitudinal tails and losses appear; shorten the incoming bunch or increase $\lvert\varphi_s\rvert$.

## Further reading {#longitudinal-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: transit-time factor, longitudinal dynamics (separatrix, adiabatic damping) and RF defocusing.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: longitudinal motion in linacs.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific: synchrotron oscillations and phase stability.
- CERN Accelerator School linac course notes ([CAS](https://cas.web.cern.ch/)).
- In this primer: [RF cavity](#rf-cavity), [Field maps](#field-maps), [Drift](#drift), [Space charge](#space-charge) and [Tutorial](#tutorial).
