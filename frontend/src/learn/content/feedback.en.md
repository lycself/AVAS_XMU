# Feedback

The previous chapter corrected the orbit caused by misplaced quadrupoles once. But the errors of an accelerator do not stay put: a few degrees between day and night make magnets and supports expand and shrink; the ground settles slowly; power-supply currents wobble a little; water pumps and cryogenic plants make the pipes vibrate gently; and the beam itself, passing through an RF cavity, "drains" part of the field. Correct once, and a while later the beam is off again.

There is only one remedy: **measure all the time and correct all the time**, and let a machine do it automatically. This is **feedback**. From RF cavity fields adjusted millions of times a second, to the orbit adjusted once per beam pulse, to the energy adjusted every few minutes, accelerators are full of feedback loops.

This chapter starts from the thermostat at home to make the three key words of feedback clear: **gain** (how much to correct each time), **delay** (how late the correction comes) and **stability** (whether correcting makes things worse). Two labs then check them: pulse-to-pulse orbit feedback, and the **low-level RF** (LLRF) feedback that holds the amplitude and phase of an RF cavity.

## What feedback is: starting from a thermostat {#feedback-loop}

A room thermostat in winter works like this:

1. A **sensor** (thermometer) measures the present temperature, say 18 °C;
2. it is compared with the **set point** (the 20 °C you want); the difference of 2 °C is the **error**;
3. a **controller** decides what to do from the error: a positive error means heat;
4. an **actuator** (the heater) really heats, and the room warms up;
5. back to step 1, measure again.

The result of the measurement comes back to decide the next action; the information runs round in a circle, hence the name **feedback loop** (or closed loop). If the heater were switched by a timer without looking at the temperature, that would be **open loop**: when the weather changes, the room gets too cold or too warm and nobody notices.

The direction of feedback matters. The thermostat heats when the temperature is **low** and stops when it is **high**; its action always **reduces** the error. This is **negative feedback**, the basis of all control. An action that increases the error is **positive feedback**: with a microphone too close to a loudspeaker, the sound is amplified, picked up again, amplified again, and within seconds becomes a piercing howl. We will see that a loop meant as negative feedback also turns into positive feedback when its action comes **too late**.

Feedback in an accelerator corresponds point by point to the thermostat:

| | Thermostat | Orbit feedback | RF cavity (LLRF) feedback |
|---|---|---|---|
| Sensor | thermometer | BPM (beam position monitor) | pick-up probe in the cavity |
| Set point | 20 °C | orbit on the centre line | the wanted field amplitude and phase |
| Actuator | heater | steerers | drive signal of the power amplifier |
| How often | every few minutes | every beam pulse | several times per microsecond |

## Gain: how much to correct each time {#feedback-gain}

Suppose the orbit is 1 mm off and we already know "how much to change the steerers to bring it back" (the response matrix of the previous chapter). Should each step change them by exactly that much?

Not necessarily. The fraction of "the computed change" that is actually applied each time is called the **gain** $g$. After each correction, the remaining error is $(1-g)$ times what it was:

| Gain $g$ | Successive errors (mm) | Result |
|---|---|---|
| 0.5 | 1 → 0.5 → 0.25 → 0.125 → … | halves every time, shrinks smoothly |
| 1 | 1 → 0 | done in one step |
| 1.5 | 1 → −0.5 → 0.25 → −0.125 → … | overshoots, jumps between signs, but shrinks |
| 2 | 1 → −1 → 1 → −1 → … | jumps back and forth forever, never converges |
| 2.5 | 1 → −1.5 → 2.25 → … | gets worse and worse |

So without delay the gain must lie between 0 and 2. $g = 1$ looks best; why do real systems often use a smaller gain?

- **The model is not exact.** The real response matrix differs from the model by a few to more than ten per cent (the previous chapter said it has to be measured); with $g = 1$ the actual effect may be 1.1, or 0.9. A smaller gain leaves a margin.
- **Measurements are noisy.** BPM readings carry a few to a few tens of micrometres of noise. $g = 1$ means "correcting" every bit of noise in full as if it were a real error, and the steerers jump with the noise. With $g = 0.2$ each step trusts only 20 %, which amounts to averaging several readings, and the noise matters much less.

A small gain reacts slowly but steadily; a large gain reacts fast but is easily led astray by noise and model errors. Every feedback has to make this trade.

## Delay: when the correction comes too late {#feedback-delay}

Showering in a hotel with a long hot-water pipe: open the hot tap, and it takes several seconds for hot water to reach the shower head. The water is cold, you open a bit more; two seconds later it is still cold, you open more; suddenly the hot water arrives, scalding! You turn back quickly; two seconds later it is ice cold… You keep swinging between hot and cold. You are not turning the tap the wrong way; you are always seeing the result of **a few seconds ago**.

A feedback loop is the same. In pulse-to-pulse feedback, the BPM readings of one pulse must be processed by electronics, computed, and sent as commands to the steerer power supplies; by the time the steerer currents have really changed, it is often the **next** pulse or the one after. Let us see what one pulse of delay does.

Let $e_k$ be the error of pulse $k$. Without delay $e_{k+1} = e_k - g\,e_k$. With one pulse of delay, the correction uses the readings of the **previous** pulse:

$$
e_{k+1} = e_k - g\,e_{k-1}.
$$

Start from $e_0 = e_1 = 1$ (the first correction has not arrived yet) and compute by hand:

| Pulse $k$ | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|---|---|
| $g = 0.5$ | 1 | 1 | 0.5 | 0 | −0.25 | −0.25 | −0.125 | 0 | 0.0625 |
| $g = 1$ | 1 | 1 | 0 | −1 | −1 | 0 | 1 | 1 | 0 |

With $g = 0.5$ the error **overshoots** (turns negative) and then shrinks while swinging, to 1/4 every 4 pulses. With $g = 1$, which without delay corrects in one step, the pattern now repeats every 6 pulses and never converges: the error has already been corrected when the late correction pushes it to the other side. A little more gain and the swings grow.

Conclusion: **with one pulse of delay the gain must be below 1**; with two pulses the limit drops to about 0.62, with three to about 0.45. The longer the delay, the smaller the usable gain and the slower the loop. This is the hardest limit in feedback design.

## Pulse-to-pulse orbit feedback {#feedback-orbit}

Many linacs are **pulsed**: the beam comes in bursts, from a few to a few tens of pulses per second. Orbit feedback does "measure, compute, change the steerers" between two pulses, and the next pulse uses the new settings. The lab below uses the channel of the [Orbit correction](#orbit-correction) chapter: 6 FODO cells, 12 BPMs, 12 steerers. This time the quadrupoles **move a tiny random step every pulse** ("quadrupole drift per pulse", rms 2 µm by default), imitating slow drifts of the ground and temperature; the BPM readings carry 10 µm of noise. Every pulse, the correction is computed from the response matrix of the previous chapter, multiplied by the gain $g$, and applied to the steerers using the delayed readings.

```avas-lab
feedback
```

With the defaults (gain 0.5, delay 1 pulse), look at the first plot: the dashed line is the orbit without feedback; the quadrupoles drift farther and farther, the orbit grows, more than 0.1 mm rms at the BPMs after 150 pulses and 0.084 mm on average over the second half. The solid line is the orbit with feedback, held at about 0.01–0.02 mm, 0.016 mm on average over the second half. The second plot is the field of the first steerer: it wanders slowly following the quadrupole drift and jitters a little with the noise (about 0.1 mT). The third plot shows "the largest $|z|$ of the loop" against the gain: below 1 the loop is stable, 1 is the stability limit (what $|z|$ means is explained in the "going deeper" section; for now, read it as "roughly the factor the error is multiplied by each pulse").

Change gain and delay (other parameters unchanged):

| Gain $g$ | Delay (pulses) | rms with feedback (mm) | largest $\lvert z\rvert$ | Remark |
|---|---|---|---|---|
| 0.5 | 1 | 0.016 | 0.71 | default |
| 0.2 | 1 | 0.018 | 0.72 | a bit slower, still very good |
| 0.9 | 1 | 0.033 | 0.95 | close to the limit, the steerers swing |
| 1.0 | 1 | 0.10 | 1.00 | right on the limit, a warning appears |
| 1.1 | 1 | 15 (diverges) | 1.05 | worse and worse |
| 1.0 | 0 | 0.013 | 0 | without delay: done in one step |
| 1.9 | 0 | 0.044 | 0.90 | without delay unstable only at 2 |
| 0.5 | 2 | 0.025 | 0.94 | two pulses of delay, close to the limit |
| 0.5 | 3 | 0.60 (diverges) | 1.02 | same gain, three pulses of delay: unstable |

The lab's "largest stable gain for this delay" gives the limit: about 2 without delay, 1 with one pulse, about 0.62 with two and about 0.45 with three pulses, as in the previous section.

## Noise: the gain should not be as large as possible {#feedback-noise}

Now the noise. Keep the delay at 1 pulse and change "BPM noise" and the gain:

| BPM noise | $g = 0.1$ | $g = 0.2$ | $g = 0.5$ |
|---|---|---|---|
| 0 | 0.023 mm | 0.017 mm | 0.014 mm |
| 10 µm (default) | 0.023 mm | 0.018 mm | 0.016 mm |
| 50 µm | 0.026 mm | 0.025 mm | 0.040 mm |

With 10 µm of noise, $g$ around 0.4–0.5 is best: too small cannot keep up with the drift, too large amplifies the noise ($g = 0.9$ gives 0.033 mm). With 50 µm of noise the best gain moves to about 0.2: the less trustworthy the readings, the less one should trust them.

An even more extreme case: set "quadrupole drift per pulse" to 0. Without feedback the orbit stays 0; with feedback it becomes 0.008 mm, all of it noise that the feedback "corrected" into the orbit. **When there is no error to correct, feedback only does harm.**

## Why RF cavities need feedback {#feedback-llrf-why}

Now to a completely different place: the RF accelerating cavity. Its electric field oscillates at 162.5 MHz (the frequency of the example beam), and the energy a particle gains on each passage depends on the field's **amplitude** and on the field's **phase** when the particle arrives (see [RF cavity](#rf-cavity) and [Longitudinal motion](#longitudinal)). In the example the particles cross the cavity at a synchronous phase of −30°:

- 1 % of amplitude error gives 1 % of energy-gain error;
- 1° of phase error also gives about 1 %: the energy gain is proportional to cos φ, and cos(−29°) is 1.0 % larger than cos(−30°).

If one cavity is a little off, the following cavities no longer match its phase; the bunch lengthens, the energy spreads, and in bad cases particles are lost. Accelerators therefore usually require each cavity's field to be stable to about 0.1 % and 0.1° (depending on the machine, from about 1 % and 1° down to 0.01 % and 0.01°). And the cavity field refuses to behave, mainly because of two troublemakers:

**Beam loading.** The beam is accelerated in the cavity, and the energy it gains is taken from the cavity field. The example beam (0.184 mA) gains about 0.34 MeV in one HWR cavity, i.e. it takes about 0.34 MV × 0.184 mA ≈ 63 W out of the cavity. When the beam arrives, the field drops; when it leaves, the field shoots up. Accelerators with higher currents take kilowatts.

**Microphonics.** A cavity is a metal "hollow", and its resonant frequency is set by its shape. Vibrations of water pumps, vacuum pumps and cryogenic plants deform the walls by tens of nanometres, and the resonant frequency moves by a few to a few tens of hertz. The RF frequency fed in stays the same, so the cavity is now "out of tune": this is **detuning**. In a detuned cavity the field amplitude drops and the phase shifts. Superconducting cavities are especially sensitive: their losses are tiny and their resonance extremely narrow; the **half bandwidth** (the frequency range of half the resonance width) is only tens to hundreds of hertz, so tens of hertz of detuning is already a large part of it.

> **AVAS does not simulate LLRF.** AVAS takes each cavity's field amplitude (Ke, Kb) and phase as given and does not simulate the power source, beam loading, detuning or the feedback loop below. This section and the lab below are a textbook single-cavity model to understand what LLRF does; the residual jitter LLRF leaves can be studied in AVAS's error studies with dynamic RF cavity errors (see [In AVAS](#feedback-in-avas) in this chapter).

## Low-level RF (LLRF) feedback {#feedback-llrf}

The system that holds the cavity field is called **low-level RF** (LLRF). "Low-level" is relative to the high power of the amplifier: LLRF handles small signals of milliwatts. Its loop works like this: a small probe antenna in the cavity picks up a little of the field's signal; LLRF compares it with the set amplitude and phase and obtains an amplitude error and a phase error; from them it adjusts the drive signal sent to the power amplifier, which amplifies it and sends it back into the cavity. Modern LLRF does the comparison and computation in fast digital electronics, millions of times per second.

Unlike orbit feedback, the actuator here can change continuously, and the controller usually has two parts:

- a **proportional** part: the change of drive is proportional to the **present** error; the factor is the "feedback gain". It reacts fast but always leaves a little error: once the error is small, the drive stops growing.
- an **integral** part: it keeps **adding up** the error; as long as any error remains, the sum keeps changing and the drive keeps adjusting, until the error is gone completely.

The lab below simulates a superconducting cavity with a half bandwidth of 100 Hz. At 1 ms the beam is switched on and takes away 30 % of the field ("beam loading"); the cavity also suffers microphonics at 230 Hz with an amplitude of 20 Hz.

```avas-lab
llrf
```

With the defaults (feedback gain 50, loop delay 2 µs):

- **Without feedback** (dashed): after the beam is switched on the field amplitude keeps falling, about 25 % down after 3 ms and still heading for −30 %; the rms amplitude error after the beam is on is about 20 %, and the phase swings with the microphonics between about −4° and +6°, 4.3° rms.
- **With feedback** (solid): the amplitude error is only about 0.5 %, the phase error 0.16° rms.

Why 0.5 % is left: proportional feedback shrinks the error to about 1/(1 + gain): 30 % ÷ 51 ≈ 0.6 %, exactly the depth of the dip right after the beam is switched on (−0.58 % in the lab). Then the integral part pulls it back slowly, to −0.49 % at 4 ms; the integral is deliberately slow in this lab and has not finished within 4 ms. With gains 10, 100 and 200 the amplitude error is about 2.5 %, 0.27 % and 0.13 %, all close to 30 %/(1 + gain).

Is a larger gain always better? Here we meet **delay** again. The lab also gives two numbers: the **crossover frequency** (gain × half bandwidth, 5 kHz by default) and the **phase margin** (86.4° by default). Roughly speaking, the loop can only correct disturbances slower than the crossover frequency; and for changes near the crossover frequency, the delay $\tau$ makes the correction arrive late by a phase of 360° × frequency × $\tau$; when that eats up the margin, negative feedback turns into positive feedback (just as in the shower). Try:

- Gain 50, delay raised to 40 µs: phase margin 18°, still stable; delay 50 µs: margin 0°, an instability warning appears.
- Gain 200 (crossover 20 kHz), delay 10 µs: margin 18°; 12 µs: only 3.6° left; 15 µs: margin −18°, and the field diverges within milliseconds.

The higher the gain, the smaller the error, but the shorter the delay it tolerates (the largest delay is about 1/(4 × crossover frequency)). That is why LLRF keeps the delay of the whole loop at the microsecond level: cables as short as possible, digital electronics as fast as possible.

## Feedback in practice: fast and slow loops, nested {#feedback-practice}

A real accelerator runs many feedback loops at once, with speeds many orders of magnitude apart:

- **The fastest** is LLRF, adjusting several times per microsecond inside one beam pulse to hold each cavity's amplitude and phase. Disturbances known in advance, such as beam loading, are also countered by adding drive ahead of time (**feedforward**), so the feedback only handles what is left.
- **Pulse-to-pulse** orbit and energy feedback compute once between two pulses.
- **Slow feedback** corrects the orbit every few seconds to minutes (mentioned at the end of the previous chapter), against drifts of temperature and ground.

A slow loop is usually made much slower than the faster loops inside it, so that they do not "fight". How these loops are implemented in a control system, and how operators watch them, is in the [Control systems](#control-systems) chapter.

## Think about it {#feedback-think}

> **Think about it 1**: without delay, a feedback with $g = 1.5$ makes the error jump between signs but still converge. Why does hardly anybody use it?
>
> Answer: it converges, but overshoots at every step and is very sensitive to noise and model errors: a real gain just one third larger than 1.5 reaches 2 and the loop becomes unstable. And every step amplifies the noise, leaving more residual than a smaller $g$.

> **Think about it 2**: the delay of a loop grows from 1 to 2 pulses and the gain is left at $g = 0.5$. What happens? What can be done to keep a stability margin?
>
> Answer: the largest $|z|$ rises from 0.71 to 0.94, close to the limit; the error decays slowly and swings. Either lower the gain (for example to 0.3, largest $|z|$ about 0.81) or find a way to shorten the delay.

> **Think about it 3**: set the microphonics to 0 in the LLRF lab. What happens to the phase error, and why?
>
> Answer: it becomes 0. In this model beam loading changes only the size of the field, not its phase; the phase error comes entirely from detuning.

> **Think about it 4**: why should the slow orbit feedback be much slower than the fast loops?
>
> Answer: if two loops were equally fast and acted on the same quantity, each would treat the other's correction as a disturbance to be corrected; they could push each other back and forth, even unstably. When one is much slower, the fast loop sees the slow one as nearly constant, and the slow loop always finds the fast one already settled.

## Common misconceptions {#feedback-misconceptions}

- **"The larger the gain, the better the feedback"**: a large gain reacts fast but amplifies noise, and with delay it goes unstable. In the lab, with one pulse of delay, $g = 1.1$ already makes the orbit diverge.
- **"Negative feedback is always stable"**: negative feedback only guarantees the right direction, not the right timing. With a long enough delay the situation has reversed by the time the correction arrives, and the negative feedback has effectively become positive.
- **"Feedback on is always better than off"**: with no error to correct, feedback only feeds the measurement noise into the actuators (in the lab, with zero drift, feedback moves the orbit from 0 to 0.008 mm).
- **"LLRF holds the cavity field, so the beam energy must be right"**: LLRF holds the field the probe sees. The probe calibration, phase drifts of cables with temperature and the phase reference between cavities need separate calibration; in the end the beam itself (for example its measured energy or time of flight) is the check.
- **"AVAS can simulate LLRF"**: it cannot. AVAS's cavity fields are given values; the LLRF lab of this chapter is a separate textbook model.

## Going deeper: gain, delay and stability {#feedback-deep-stability}

The hand-computed tables of the main text showed two rules: "$g < 2$" and "$g < 1$ with one pulse of delay". This section derives them step by step using only geometric sequences, quadratic equations and school complex numbers; the last paragraph is for readers who know the z-transform.

### Step 1: no delay: a geometric sequence

Each pulse multiplies the error by $(1-g)$: $e_{k+1} = (1-g)\,e_k$, so $e_k = (1-g)^k\,e_0$, a geometric sequence with ratio $q = 1-g$. A geometric sequence goes to 0 when $|q| < 1$, i.e.

$$
-1 < 1 - g < 1 \quad\Longleftrightarrow\quad 0 < g < 2 .
$$

For $1 < g < 2$ the ratio is negative and the sequence alternates in sign: that is the "overshoot". In the lab, with delay 0 and $g = 1.9$, the largest $|z|$ is 0.9, exactly $|1 - 1.9|$.

### Step 2: one pulse of delay: guess a geometric sequence

Now $e_{k+1} = e_k - g\,e_{k-1}$: each term depends on the previous **two**, so it is no longer a simple geometric sequence. The trick is to guess: perhaps it is still a geometric sequence $e_k = z^k$? Insert it and divide both sides by $z^{k-1}$:

$$
z^2 = z - g \quad\Longrightarrow\quad z^2 - z + g = 0 .
$$

This is a quadratic equation with roots $z = \tfrac{1}{2}\left(1 \pm \sqrt{1 - 4g}\right)$. The error sequence is a combination of these two geometric sequences, so it dies out only when **both roots have absolute value below 1**. $|z|$ is the lab's "largest $|z|$": roughly the factor the error is multiplied by each pulse.

- For $g < \tfrac14$ both roots are real; the larger is $\tfrac12(1 + \sqrt{1-4g})$. It is 0.887 for $g = 0.1$ and 0.724 for $g = 0.2$, as in the lab.
- For $g > \tfrac14$ the number under the square root is negative and the two roots are a pair of complex conjugates. By Vieta's formulas their product is $z_1 z_2 = g$, and the product of complex conjugates is the square of their modulus, so $|z|^2 = g$, $|z| = \sqrt g$. $|z| < 1$ requires $g < 1$.

For $g = 0.5$, $|z| = \sqrt{0.5} = 0.707$, and every 4 pulses the error shrinks to $0.707^4 = 0.25$: the pattern $1 \to -0.25 \to 0.0625$ of the hand-computed table. Complex roots mean oscillation: the argument $\theta$ of $z$ satisfies $\cos\theta = \tfrac12/\sqrt g$; for $g = 0.5$, $\theta = 45°$, one oscillation every $360°/45° = 8$ pulses, and in the table $e_3 = e_7 = 0$ are half a period, 4 pulses, apart. For $g = \tfrac14$ both roots are 0.5 and the error dies out fastest.

### Step 3: d pulses of delay: where is the limit?

With a delay of $d$ pulses, $e_{k+1} = e_k - g\,e_{k-d}$, and inserting $e_k = z^k$ in the same way gives

$$
z^{d+1} - z^{d} + g = 0 .
$$

For small $g$ all roots lie inside the unit circle (the circle $|z| = 1$); as the gain grows, at some value one root just touches the unit circle: that is the stability limit. Let $z = \cos\theta + i\sin\theta$ there. Write the equation as $z^d\,(z - 1) = -g$ and compare the moduli and the arguments of both sides:

- **Modulus**: $|z^d| = 1$, and $|z - 1|$ is the chord of the unit circle from the point 1 to the point $z$, which by the isosceles triangle equals $2\sin(\theta/2)$. Hence $g = 2\sin(\theta/2)$.
- **Argument**: the argument of $z^d$ is $d\theta$; the chord from the point 1 to the point $z$ makes an angle $90° + \theta/2$ with the real axis (the same isosceles triangle). The right side $-g$ is a negative real number with argument $180°$. So $d\theta + 90° + \theta/2 = 180°$ and $\theta = 180°/(2d+1)$.

Together, the largest stable gain is

$$
g_{\max} = 2\sin\frac{90°}{2d+1} .
$$

$d = 0$: $2\sin 90° = 2$; $d = 1$: $2\sin 30° = 1$; $d = 2$: $2\sin 18° = 0.618$; $d = 3$: $2\sin 12.9° = 0.445$. These are the four limits of the main text and the lab. On the limit the error oscillates with a period of $360°/\theta = 2(2d + 1)$ pulses: for $d = 1$ it changes sign every 3 pulses and repeats every 6, exactly the $g = 1$ row of the main text.

### Step 4: why 12 steerers need only one equation

The lab has 12 BPMs and 12 steerers (12 in each plane). Why does the stability depend only on $g$ and $d$? Because the singular value decomposition of the previous chapter splits the BPM readings into non-interfering patterns, each with its own gain $\sigma_i$. In the feedback $\boldsymbol u_{k+1} = \boldsymbol u_k - g\,R^{+}\boldsymbol y_{k-d}$, $R^{+}$ divides exactly by $\sigma_i$ on each pattern, so the error $c_i$ along pattern $i$ obeys

$$
c_{i,\,k+1} = c_{i,\,k} - g\,c_{i,\,k-d} ,
$$

exactly the equation of step 3, independent of $\sigma_i$. Patterns of the tiny singular values that were cut take no part in the feedback and stay as they are. This assumes an exact response matrix; if the real matrix is 20 % larger than the model along some pattern, the actual gain of that pattern is $1.2g$, so the gain has to leave room for model errors.

### Step 5: the trade between noise and drift

Why should the gain be small when the noise is large? Take the case without delay and add two random quantities: each pulse the error itself drifts by a random step $w_k$ (rms $s$), and the measurement carries noise $n_k$ (rms $\sigma$). The correction uses the noisy reading:

$$
e_{k+1} = (1 - g)\,e_k - g\,n_k + w_k .
$$

Use two properties from school probability: multiplying a random quantity by a constant $a$ multiplies its variance by $a^2$; adding independent random quantities adds their variances. After the feedback has run for a long time, the variance $V$ of the error no longer changes, so $V = (1-g)^2 V + g^2\sigma^2 + s^2$, which gives

$$
V = \frac{g^2\sigma^2 + s^2}{g\,(2 - g)} .
$$

The drift $s^2$ in the numerator does not depend on $g$, and the denominator $g(2-g)$ is small for small $g$, so too small a $g$ cannot keep up with the drift; the noise term $g^2\sigma^2/[g(2-g)] = g\sigma^2/(2-g)$ grows with $g$, and at $g = 1$ the noise enters the error unchanged. The larger the noise $\sigma$, the smaller the best $g$; that is why the best gain moves to about 0.2 with 50 µm of noise in the main text's table (the lab has one pulse of delay, so the numbers differ, but the trend is the same). Without drift ($s = 0$), $V = g\sigma^2/(2-g) > 0$: the feedback only does harm.

### For readers who know the z-transform

For a static plant, the controller of each pattern is an integrator $C(z) = g/(z-1)$ in series with a pure delay $z^{-d}$; the closed-loop characteristic polynomial is $z^{d+1} - z^d + g$, whose roots are the closed-loop poles; `avas.dynamics.control.integral_loop_modulus` computes their largest modulus, and the Jury criterion gives the same limit $g_{\max} = 2\sin\frac{\pi}{2(2d+1)}$. Step 5 is the scalar form of the discrete Lyapunov equation; for a general state-space model the optimal gains come from the Kalman filter (noise side) and the LQR (control side), the latter provided by `avas.dynamics.control.lqr`. When the plant itself has dynamics ($A \ne I$) the closed loop is $A + BK$; with delay see `delayed_loop`.

## Going deeper: the feedback loop of one cavity {#feedback-deep-llrf}

This section builds the model of the LLRF lab step by step. The first four steps use only the school idea of "the change in a small time interval"; step 5 uses the derivative of the sine function; the last paragraph is for readers who know the Laplace transform. For simplicity the field is measured in units of the set point, so a drive signal $u = 1$ gives exactly the set field.

### Step 1: a cavity is like a leaking bucket

Pour water into a bucket (the drive); there is a hole in the bottom, and the higher the level, the faster it leaks (wall losses and the power coupled out are proportional to the field). The level settles where "inflow = outflow". The beam is a second hole that takes an extra $I_b$. In each small time interval $\Delta t$ the field changes by

$$
\Delta V = \omega_{1/2}\,\bigl(u - I_b - V\bigr)\,\Delta t ,
$$

with $\omega_{1/2} = 2\pi f_{1/2}$ and $f_{1/2}$ the half bandwidth. The lab computes exactly like this, one small step after another. The field settles at $V = u - I_b$, and how fast it approaches is set by the **time constant** $T = 1/\omega_{1/2}$: every $T$ the remaining distance shrinks to $1/\mathrm e \approx 0.37$. For a half bandwidth of 100 Hz, $T = 1/(2\pi \times 100) = 1.6$ ms.

Without feedback and with 30 % beam loading, the field approaches 0.7; after 3 ms it has dropped by $30\% \times (1 - \mathrm e^{-3/1.6}) \approx 25\%$, as the dashed line of the lab shows.

### Step 2: proportional feedback: the error shrinks to 1/(1+K)

Change the drive to $u = 1 + K\,(1 - V)$, with $K$ the feedback gain. Insert it into step 1 and wait until the change stops ($\Delta V = 0$):

$$
0 = 1 + K(1 - V) - I_b - V \quad\Longrightarrow\quad 1 - V = \frac{I_b}{1 + K} .
$$

For $K = 50$ and $I_b = 0.3$ the error is $0.3/51 = 0.59\%$, in agreement with the lab's $-0.58\%$ just after the beam is switched on. And the speed: $\Delta V = \omega_{1/2}(1+K)\,(V_\infty - V)\,\Delta t$, so the time constant shrinks to $T/(1+K) = 1.6\ \mathrm{ms}/51 \approx 31\ \mu\mathrm s$. The feedback not only makes the error smaller, it makes the cavity "react" 51 times faster.

### Step 3: integral: removing the last bit of error

Proportional feedback always leaves $I_b/(1+K)$: to keep up the extra drive $K(1-V)$ there must be an error. The integral part adds the error up: $u = 1 + K e + K_i \sum e\,\Delta t$. As long as $e \ne 0$ the sum keeps changing, so the field can only settle at $e = 0$. In the lab $K_i = K\omega_{1/2}/10$, about ten times slower than the proportional part, so within 4 ms the error only comes back from $-0.58\%$ to $-0.49\%$.

### Step 4: detuning: the field's arrow turns by an angle

An RF field is an oscillation and can be drawn as a rotating arrow (a phasor): the arrow's length is the amplitude, its direction the phase. With the cavity detuned by $\Delta f$, the settled field equals the drive divided by $1 - i\,\Delta f/f_{1/2}$ (the fixed point of the lab's model $\Delta V = \omega_{1/2}\bigl[u - I_b - (1 - i\,\Delta f/f_{1/2})V\bigr]\Delta t$). By complex division:

$$
\text{amplitude} = \frac{1}{\sqrt{1 + (\Delta f/f_{1/2})^2}}, \qquad \text{phase} = \arctan\frac{\Delta f}{f_{1/2}} .
$$

20 Hz of detuning with a half bandwidth of 100 Hz: the phase moves by $\arctan 0.2 = 11.3°$, while the amplitude drops by only 1.9 %. So microphonics mainly cause phase errors. In the lab the vibration is at 230 Hz, faster than the cavity's half bandwidth; the cavity cannot follow completely, and the phase moves by only about 6.4° at most.

### Step 5: why delay is dangerous: crossover frequency and phase margin

Let the drive vary as a small sine of frequency $f$, $u = \cos(2\pi f t)$, with $f$ much larger than $f_{1/2}$. Then the $V$ term of step 1 can be neglected, $\Delta V/\Delta t \approx \omega_{1/2}\,u$: the field's rate of change is proportional to the drive. Which function has the derivative $\omega_{1/2}\cos(2\pi f t)$? It is

$$
V = \frac{f_{1/2}}{f}\,\sin(2\pi f t) .
$$

Two conclusions: the cavity responds with $f_{1/2}/f$ times the drive, less at higher frequency; and $\sin$ is a quarter period behind $\cos$, i.e. **lags by 90°**. Times the feedback gain, the amplification once round the loop is $K f_{1/2}/f$, which equals 1 at

$$
f_c = K\,f_{1/2} ,
$$

the **crossover frequency**. By default $f_c = 50 \times 100\ \mathrm{Hz} = 5\ \mathrm{kHz}$. Disturbances slower than $f_c$ are suppressed by about $f_c/f$: at the 230 Hz of the microphonics about $5000/230 \approx 22$ times; in the lab the phase error goes from 4.29° to 0.16° (about 26 times).

A delay $\tau$ makes the signal later by another $360° \times f \times \tau$. Negative feedback "pushes back" (180° apart); if at the frequency $f_c$, where the amplification is still 1, the cavity's 90° plus the delay's phase add up to another 180°, the correction points the same way as the disturbance and becomes positive feedback. The room left is the **phase margin**:

$$
\varphi_m = 180° - 90° - 360°\,f_c\,\tau = 90° - 360°\,f_c\,\tau .
$$

Default $\tau = 2\ \mu\mathrm s$: $\varphi_m = 90° - 360° \times 5000 \times 2\times10^{-6} = 86.4°$. The margin is 0 at $\tau = 1/(4 f_c)$: 50 µs for $f_c = 5$ kHz, 12.5 µs for $f_c = 20$ kHz (gain 200); in the lab, gain 200 is stable at 12 µs and diverges at 15 µs.

### Step 6: where the half bandwidth comes from

A cavity's **loaded quality factor** $Q_L$ describes for how many periods it stores its energy; the full width of the resonance is $f_0/Q_L$ and the half bandwidth $f_{1/2} = f_0/(2Q_L)$. 162.5 MHz with a half bandwidth of 100 Hz corresponds to $Q_L = 162.5\times10^6/200 \approx 8\times10^5$. Superconducting cavities have tiny wall losses; their $Q_L$ is set mainly by the power coupler, often $10^5$–$10^7$. Room-temperature cavities have $Q_L$ of only about $10^3$–$10^4$ and half bandwidths of kilohertz to tens of kilohertz; they do not mind tens of hertz of microphonics, but need far more power.

### For readers who know the Laplace transform

The lab's model is the first-order equation of the field's complex envelope $\dot V = -\omega_{1/2}(1 - i\Delta\omega/\omega_{1/2})V + \omega_{1/2}(u - I_b)$, with transfer function $G(s) = \omega_{1/2}/(s + \omega_{1/2} - i\Delta\omega)$. The controller is PI: $C(s) = K(1 + \omega_i/s)$ with $\omega_i = K\omega_{1/2}/10$; the loop is $L(s) = C(s)G(s)\mathrm e^{-s\tau}$. For $K \gg 1$, $|L(i\omega_c)| = 1$ at $\omega_c \approx K\omega_{1/2}$, and the phase margin is $\approx \pi/2 - \omega_c\tau$; the lab's formula neglects the extra lag of the PI zero at $\omega_c$, about $\arctan(1/10) \approx 5.7°$, so the real limit comes slightly before $\varphi_m = 0$. Real LLRF closes the loop on the I and Q components and controls the vector sum when several cavities share one amplifier; pulsed superconducting cavities also suffer Lorentz-force detuning (the field's own radiation pressure deforms the cavity), compensated by feedforward with piezo tuners.

## In AVAS {#feedback-in-avas}

### The "Feedback" tab of the Research page

The "Feedback" tab of the Research page simulates pulse-to-pulse orbit feedback on the linear model of the project's lattice: every pulse it evaluates the model with the present knob values and that pulse's disturbance, adds noise of each monitor's resolution to the readings, and updates the knobs with $\boldsymbol u \leftarrow \boldsymbol u - g\,R^{+}\boldsymbol y$ (using the readings of $d$ pulses before). One sets the gain $g$, the delay $d$ (pulses) and the number of pulses, picks a "drifting element" with its drifting quantity (dx, dy or roll) and rms step per pulse, and switches monitor noise on or off. The result plots the rms of the readings with and without feedback and gives the largest $|z|$ of the equation of step 3 with "stable / unstable". Monitors and knobs are shared with the "Response & controllability" tab and stored in `<project>/Research/diagnostics.json`, defaulting to the lattice's `diag_*` lines and steerers (see [Controllability and observability](#controllability-in-avas)). These calculations use only the linear model, take no run lock and write no input files; the engine takes no part in the feedback simulation.

### In Python

```text
from avas.model import LinearModel
from avas.dynamics import Diagnostics, Drift, simulate, steerers

model = LinearModel.from_project("my_project")
diag = Diagnostics.from_lattice(model)              # diag_position / diag_size, default resolution 20 µm
knobs = steerers(model)                             # By and Bx of every steerer
drift = Drift(errors={"QF1": {"dx": 2e-6}}, seed=3) # QF1 random-walks 2 µm per pulse (use your lattice's names)
out = simulate(model, knobs, diag, pulses=150, gain=0.5, delay=1, disturbance=drift)
out["rms"], out["open_rms"], out["predicted_modulus"]
```

Every pulse `simulate` computes the true readings with the model, adds noise and updates the knobs with the integral law (knobs capped at the steerers' maxima), and returns the rms with and without feedback and the predicted largest $|z|$. The channel and disturbance of this chapter's orbit-feedback lab are in `feedback` of `avas/learn/control.py`.

### RF cavities: AVAS does not simulate LLRF

In AVAS each cavity's field amplitude is given directly by Ke (and Kb) and its phase by the lattice, both fixed; the engine does not simulate the power source, beam loading, detuning or LLRF. To study how the amplitude and phase jitter left by LLRF affects the beam, add dynamic RF cavity errors to an error study: the `ke/kb` column of `err_cav_ncpl_dyn` is the field amplitude error (%), the `φs` column the phase error (°), and `err_cav_dyn_on` switches the columns on. Syntax and sampling rules are in [Errors and correction](#errors-in-avas).

```avas-params
err_cav_ncpl_dyn err_cav_dyn_on
```

## Further reading {#feedback-reading}

- K. J. Åström, R. M. Murray, *Feedback Systems: An Introduction for Scientists and Engineers*, Princeton University Press (2008): feedback, stability, PI control and delay; an introductory text for scientists and engineers.
- M. G. Minty, F. Zimmermann, *Measurement and Control of Charged Particle Beams*, Springer (2003): the chapters on orbit feedback and beam control.
- S. Simrock, Z. Geng, *Low-Level Radio Frequency Systems*, Springer (2022): principles of LLRF, cavity models, beam loading and microphonics.
- H. Padamsee, J. Knobloch, T. Hays, *RF Superconductivity for Accelerators*, Wiley: $Q$, detuning and microphonics of superconducting cavities.
- Related chapters: [Orbit correction](#orbit-correction), [RF cavity](#rf-cavity), [Errors and correction](#errors); full bibliography in [References](#references).
