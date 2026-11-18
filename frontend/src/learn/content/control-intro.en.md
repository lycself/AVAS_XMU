# Why an accelerator needs control

Up to here we have designed accelerators on paper: every magnet sits where it should, every cavity's field is exactly the design value, and the centre of the beam flies along the centre line of the pipe. A real accelerator is not like that. Magnets are placed a little off, fields are a little wrong, day and night have different temperatures, the ground slowly settles, power supplies wobble slightly, and the beam itself disturbs the fields of the accelerating cavities.

Riding a bicycle is a good comparison. You cannot work out the angle of the handlebars before you set off and then ride to the finish with your eyes closed: the bike slowly drifts, the road is bumpy, the wind pushes you. You keep **looking** (am I drifting?), **comparing** (with the route I want), **adjusting** (turn the handlebars a little), every second. An accelerator is the same: to keep a real machine close to its design, we must keep measuring the beam, comparing it with the target and adjusting magnets and cavities. This is **accelerator control**.

This chapter opens the "Accelerator control" part. It answers three questions: where does a real machine differ from the drawings; why can it not be set up once and left alone; and what are the basic methods of control. At the end it lists what the chapters of this part cover.

## Drawings and the real machine {#control-intro-gap}

There are many reasons why the beam departs from the design. By how fast they change, they fall into three groups:

| Group | How fast it changes | Examples |
|---|---|---|
| **Errors** | essentially fixed once installed | a quadrupole 0.1 mm off, a field 0.1 % wrong, cavity amplitudes and phases miscalibrated |
| **Drifts** | slowly, over minutes to months | supports expanding and contracting with temperature, ground settling, ageing power supplies and electronics |
| **Disturbances** | back and forth, over seconds to microseconds | power-supply ripple, vibrations from pumps and the ground, "microphonics" of superconducting cavities, beam loading |

**Errors** come from manufacturing and installation. The [Orbit correction](#orbit-correction) chapter showed that in a channel of 6 periods, 12 quadrupoles placed off by about 0.4 mm on average make the beam centre snake around, 1.05 mm off rms and 1.9 mm at most.

**Drifts** often come from temperature. The thermal expansion coefficient of steel is about $1.2\times10^{-5}$ per degree Celsius, so 1 m of steel grows by about 12 µm for each 1 °C. A magnet on a steel support 1 m high, with 2 °C between day and night, moves its centre up and down by about 24 µm, more than the resolution of a BPM.

**Disturbances** change quickly. Two examples: the walls of a superconducting cavity are thin, and vibrations from the ground and pumps deform it slightly, so its resonant frequency wobbles back and forth; this is called **microphonics**. And a beam passing a cavity takes energy out of it, so the cavity field drops as soon as the beam is switched on; this is called **beam loading**.

## Why it cannot be set up once {#control-intro-why}

If errors stay fixed once installed, why not build the machine accurately enough, or measure the errors and compute the compensation once and for all? There are four reasons:

1. **It cannot be built that accurately.** Aligning a quadrupole's magnetic centre to a few tens of micrometres is already hard; beyond that, the cost rises steeply.
2. **Things change.** Drifts and disturbances happen all the time; a machine tuned today is a little off tomorrow.
3. **Models are imperfect.** The model used for the design always differs from the real machine. Even inside a simulation code, the AVAS linear model and the multi-particle engine differ by a few per cent through the example solenoid field map (see [Orbit correction](#orbit-correction-in-avas)); the difference between the engine and the real machine is harder still to know. Settings computed from a model alone are off by as much as the model is.
4. **Some quantities are not known in advance.** The beam from the ion source differs slightly each time, and whether a cavity's real field amplitude is right is only known once beam passes through it.

So an accelerator cannot run "open loop"; the real beam must be measured, and the machine adjusted accordingly. When a new machine first gets beam, operators adjust one knob after another and watch how the beam readings change; this is called **beam tuning** (or commissioning). It is itself a form of control, only the "controller" is a person.

## Open loop and closed loop {#control-intro-loops}

There are two basic ways of controlling.

**Open loop** (also feedforward): compute from a model or from experience where the knobs should be, set them, and do not look at the beam again. For example, set a magnet's current from its calibration curve to get the required field; or, knowing in advance that the beam takes energy out of a cavity when it is switched on, raise the RF power at that very moment. Open loop is fast, simple, adds no measurement noise and cannot oscillate by itself; but it is only as accurate as the model, and errors the model did not foresee are beyond it entirely.

**Closed loop** (also feedback): measure the beam, compare it with the target, correct according to the difference, then measure again and correct again, round and round. It does not need to know where an error comes from; as long as it can be measured, it can be corrected. Even if the model is off, a few more rounds of correction converge. The price is that it needs diagnostic devices, it brings measurement noise in, and if it reacts too strongly with a delay it overshoots and makes things worse with every correction.

Adjusting the water temperature in a shower is a closed loop everyone has run. The water is too cold, so you turn the tap towards hot; but the hot water takes a few seconds to reach the shower head, you lose patience and turn it a lot more, and the water gets scalding, so you turn it back a lot, and it gets cold again… The temperature swings between hot and cold. The problem is not inaccurate measurement but **delay** combined with **turning too hard**. Accelerator feedback has exactly the same problem.

Real accelerators use both: known, predictable parts are compensated in advance by open loop (feedforward), and the rest is left to closed loop (feedback).

The lab below is a pulse-to-pulse orbit feedback on the channel of the [Orbit correction](#orbit-correction) chapter (6 periods, 12 BPMs, 12 steerers). On every pulse the quadrupole offsets drift a little at random, and the BPM readings carry 10 µm of noise. "Without feedback" is open loop: set once, then left alone.

```avas-lab
feedback
```

Look at three things with the default parameters:

- **Without feedback** (dashed): the orbit drifts farther and farther with the pulses; in the second half its rms is about 0.084 mm, and still growing.
- **With feedback** (solid): the orbit stays at about 0.016 mm. Every pulse corrects part of the reading, so a drift is pulled back as soon as it appears.
- Set the gain to 1.2: the lab warns, and the orbit grows with every pulse, to several metres in the linear model (a real beam would have hit the wall long before). This is "turning too hard". Now set the delay to 0, and the same gain of 1.2 is stable again (about 0.015 mm): the longer the delay, the smaller the gain you can use. Why these are exactly the numbers is explained in this chapter's deep section and in the [Feedback](#feedback) chapter.

## Measure – compare – correct {#control-intro-cycle}

Every closed loop has the same parts:

1. **The plant**: the beam and the accelerator itself.
2. **The sensor**: a diagnostic device that measures the beam, for example a BPM (see [Beam diagnostics](#diagnostics)).
3. **The set point**: the target, for example "the beam centre is 0 at the BPM".
4. **The error signal**: measured value minus set point.
5. **The controller**: an algorithm deciding how much to change the knobs according to the error signal.
6. **The actuator**: the device that actually changes the beam, for example a steerer, a magnet power supply or an RF power source.

The actuator changes the beam, the sensor measures a new reading, and the next round begins. Two examples:

| | Orbit correction | Holding a cavity field |
|---|---|---|
| Sensor | BPM | field probe in the cavity |
| Set point | beam centre at 0 | design amplitude and phase |
| Controller | response matrix and singular value decomposition | proportional-integral circuit |
| Actuator | steerer | RF power source |
| One round takes | one pulse to several minutes | of the order of microseconds |

The simplest controller is "correct part of the error each time". If each correction removes $g$ times the error ($g$ is called the **gain**), the error after the correction is $1 - g$ times what it was. Starting from 1 mm:

| Gain $g$ | Multiply by | Errors in turn (mm) | Result |
|---|---|---|---|
| 0.5 | 0.5 | 1, 0.5, 0.25, 0.125, … | shrinks steadily |
| 1 | 0 | 1, 0, 0, … | done in one step |
| 1.5 | −0.5 | 1, −0.5, 0.25, −0.125, … | overshoots every time, but still converges |
| 2.5 | −1.5 | 1, −1.5, 2.25, −3.375, … | grows with every correction: diverges |

Any $g$ between 0 and 2 converges. $g = 1$ looks best, but measurements are noisy, and getting there in one step means "correcting" all of each reading's noise into the machine every time; so in practice gains below 1 are common, taking a few more steps so that the noise partly averages out between them. When the measurement also has a delay, the stable range shrinks by half, which is why the gain of 1.2 diverged in the lab.

## From microseconds to months: layers of control {#control-intro-timescales}

Many control loops run in an accelerator at the same time, their speeds differing by more than ten orders of magnitude:

| Loop | One round takes | What it does |
|---|---|---|
| Machine protection (interlocks) | microseconds to tens of microseconds | cuts the beam at once on a sudden rise of losses or an equipment fault; not regulation but "braking" |
| RF field feedback (LLRF) | of the order of microseconds | holds each cavity's field amplitude and phase near the set point against microphonics and beam loading |
| Pulse-to-pulse feedback | once per beam pulse | corrects the next pulse from the readings of the previous one, for example orbit and energy |
| Slow orbit correction | seconds to minutes | keeps up with temperature and power-supply drifts |
| Beam tuning | hours, at start-up | operators adjust knobs one by one according to measurements |
| Alignment and beam-based alignment | months to years | re-survey, move magnets, find the BPMs' true zeros |

Fast loops deal with fast disturbances, slow loops with slow drifts; they share the work. A loop can only deal with changes much slower than one of its own rounds: a disturbance faster than the loop has already changed by the time it has been measured, computed and acted on.

RF field feedback is done by an electronic system called **low-level RF** (LLRF). AVAS does not simulate the LLRF: it uses each cavity's field as a given value; the [Feedback](#feedback) chapter explains its principle with a textbook model.

## What control cannot do {#control-intro-limits}

Control is not almighty. The following chapters discuss its limits one by one:

- **What cannot be seen cannot be corrected.** A BPM measures only where it sits; an offset between two BPMs is invisible to it, and so it cannot be corrected. This is the question of **observability**.
- **What cannot be reached cannot be corrected either.** Some beam motions cannot be influenced by any combination of the available actuators. This is the question of **controllability**.
- **Noise gets amplified.** Correcting tiny readings with very weak combinations of knobs multiplies the measurement noise (see [Orbit correction](#orbit-correction-svd)).
- **Delay causes oscillation.** It always takes some time from measurement to actuator; with too much gain the loop swings back and forth like the shower temperature.
- **Nonlinearity and chaos.** Beam motion in strongly nonlinear fields can be extremely sensitive to the initial conditions, amplifying small errors again and again. Remarkably, precisely because of that sensitivity, a tiny nudge can sometimes keep it stable.

## Map of this part {#control-intro-overview}

The "Accelerator control" part proceeds in the order "measure, correct, feed back, limits, systems, chaos":

1. [Beam diagnostics](#diagnostics): how to measure the beam's position, current, size, emittance, phase and losses, and how precisely.
2. [Orbit correction](#orbit-correction): pulling the beam centre back to the centre line with BPMs and steerers; the response matrix and singular value decomposition, and why not to chase the noise.
3. [Feedback](#feedback): correcting automatically, round after round; gain, delay and stability; feedback on RF cavity fields (LLRF, not simulated by AVAS).
4. [Controllability and observability](#controllability): how many BPMs and steerers, and where, so that we can see and reach what matters.
5. [Control systems of real machines](#control-systems): how the thousands of devices of a real accelerator are networked, logged and interlocked, and how operators work.
6. [Chaos control](#chaos-control): regular orbits, island chains and chaos in nonlinear motion, and how tiny nudges can tame chaos.

## Think about it {#control-intro-think}

> **Think 1**: If feedback corrects errors the model does not know about, why use open loop (feedforward) at all?
>
> Answer: Feedback must see an error before it can act, so it is always one step late; for known, predictable disturbances (for example beam loading when the beam is switched on), compensating in advance is much better than chasing afterwards. Feedforward also adds no measurement noise and cannot oscillate. The two are used together.

> **Think 2**: A loop corrects the orbit once a second. Can it remove orbit jitter caused by 50 Hz power-supply ripple?
>
> Answer: No. The 50 Hz jitter swings back and forth 50 times a second; the loop looks once a second and sees only the value at one instant, and by the time its correction arrives the jitter has long changed direction. Fast disturbances need fast loops (or a steadier power supply in the first place).

> **Think 3**: A gain $g = 1$ corrects the error to zero in one step. Why are 0.3–0.5 common in practice?
>
> Answer: Measurements are noisy, and $g = 1$ "corrects" all of each reading's noise into the machine; also, with model errors and measurement delays, $g = 1$ is too close to the stability limit. A smaller gain taking a few more steps is safer.

> **Think 4**: If you set the noise to 0 in the feedback lab, does the orbit with feedback become 0?
>
> Answer: No; with the default parameters it only drops from about 0.016 mm to about 0.014 mm. The quadrupoles keep drifting on every pulse, the correction uses the previous pulse's reading, and each correction removes only part of the error, so a little always remains.

## Common misconceptions {#control-intro-misconceptions}

- **"A machine built accurately enough needs no control"**: drifts and disturbances happen all the time, so even the most accurate machine slowly goes off; and some quantities (the beam itself, the real cavity fields) are not known in advance.
- **"The larger the gain, the better the correction"**: too much gain overshoots; with a delay, beyond a certain gain the loop oscillates and diverges.
- **"Feedback removes every error"**: feedback only removes errors that can be measured, can be reached and are slower than the loop; measurement noise and errors of the sensor itself (for example a BPM's zero) stay in the result unchanged.
- **"Control means automation"**: in beam tuning the person is the controller; the "beam off" of machine protection is control too, but its purpose is protecting equipment, not regulation.
- **"A simulation code is a control system"**: a simulation code like AVAS can predict the beam, compute response matrices and try out feedback algorithms, but it is not connected to real equipment and cannot control a real accelerator.

## Going deeper: the simplest feedback loop — gain, delay and integral action {#control-intro-deep-loop}

The main text showed the effect of the gain with a table, and the lab showed a delay making the loop diverge. This section derives these results step by step in the simplest case of one BPM and one steerer. The first five steps use only school linear equations, geometric sequences and quadratic equations (including complex roots); the final paragraph is for readers who know control theory.

### Step 1: write down the plant

The reading $y$ (mm) of a BPM has two parts: the offset $d$ caused by errors, and the change caused by the steerer field $u$ (mT):

$$
y = d + r\,u,
$$

where $r$ is "how many mm per mT", an element of the response matrix of [Orbit correction](#orbit-correction-response). Take $r = -0.302$ mm/mT from there and $d = 1$ mm.

### Step 2: how accurate is open loop?

Open loop computes $u = -d/r = 1/0.302 = 3.31$ mT from the model's $r$, sets it and leaves it. If the model is 10 % off and the real $r$ is $-0.332$ mm/mT, the result is

$$
y = 1 - 0.332 \times 3.31 = -0.10\ \mathrm{mm}.
$$

A model 10 % off leaves 10 % of the error. Open loop is exactly as accurate as the model.

### Step 3: closed loop, step by step

Closed loop reads $y_k$ at every step and uses the model to remove $g$ times it, i.e. $\Delta u = -g\,y_k/r_{\text{model}}$. The real reading changes by $r_{\text{real}}\,\Delta u$, so

$$
y_{k+1} = y_k + r_{\text{real}}\,\Delta u = \left(1 - g\,\frac{r_{\text{real}}}{r_{\text{model}}}\right) y_k .
$$

With the model 10 % off, the bracket is $1 - 1.1g$. With $g = 1$ every step multiplies by $-0.1$: 1, −0.1, 0.01, −0.001 mm, and after three steps only 1 µm is left. **Although the model is 10 % off, feedback still corrects the error to zero**, it just takes a few more steps. This is the greatest advantage of feedback.

### Step 4: the range of gains

Write $q = 1 - g'$ ($g' = g\,r_{\text{real}}/r_{\text{model}}$ is the "gain that actually acts"). The errors $y_0, q\,y_0, q^2 y_0, \dots$ form a geometric sequence with ratio $q$. A geometric sequence goes to zero when $|q| < 1$, i.e.

$$
0 < g' < 2 .
$$

This is the conclusion of the table in the main text: for $g' = 1.5$, $q = -0.5$, alternating in sign but shrinking; for $g' = 2.5$, $q = -1.5$, growing. With the delay set to 0, the lab shows a largest stable gain of exactly 2.

### Step 5: a delay of one pulse

In practice the correction on pulse $k$ often can only use the reading of pulse $k-1$ (measuring, computing and changing the field all take time). Taking $g' = g$ for simplicity,

$$
y_{k+1} = y_k - g\,y_{k-1}.
$$

Starting from $y_0 = y_1 = 1$ (the first correction has not acted yet) and computing term by term:

| $g$ | $y_0, y_1, y_2, \dots$ | Result |
|---|---|---|
| 0.5 | 1, 1, 0.5, 0, −0.25, −0.25, −0.125, 0, 0.0625 | shrinks while oscillating, ×(−0.25) every 4 steps |
| 1 | 1, 1, 0, −1, −1, 0, 1, 1, 0 | repeats every 6 steps, neither growing nor shrinking |
| 1.2 | 1, 1, −0.2, −1.4, −1.16, 0.52, 1.91, 1.29, −1.01 | grows while oscillating |

Without delay $g = 1.2$ is quite stable; with a delay of one pulse it diverges, and the stability limit falls exactly at $g = 1$.

### Step 6: judging stability with a quadratic equation

Guess a solution of the form $y_k = z^k$ (a geometric sequence with ratio $z$ still to be found), substitute it into $y_{k+1} = y_k - g\,y_{k-1}$ and divide both sides by $z^{k-1}$:

$$
z^2 - z + g = 0 .
$$

The general solution is a sum of the geometric sequences of the two roots, so the error goes to zero as long as both roots have modulus less than 1. By Vieta's formulas the product of the roots is $z_1 z_2 = g$. For $g > 1/4$ the discriminant $1 - 4g$ is negative and the roots are a pair of complex conjugates of equal modulus, $|z|^2 = z_1 z_2 = g$, hence

$$
|z| = \sqrt{g} < 1 \iff g < 1 .
$$

For $g \le 1/4$ both roots are real numbers between 0 and 1, also stable. Check: for $g = 0.5$, $|z| = 0.707$ and $0.707^4 = 0.25$, exactly the "×0.25 every 4 steps" of the table; for $g = 1.2$, $|z| = 1.095$. The lab's "largest |z| of the loop" shows exactly these two numbers (0.7071 and 1.0954). For $g = 1$ the roots are $\frac12 \pm \frac{\sqrt3}{2}i$, a turn of exactly 60°, back to the start after 6 turns: the "repeats every 6 steps" of the table. The delay lowers the allowed gain from 2 to 1.

### Step 7: proportional and integral control

The controller above **adds up** the corrections in $u$: $u_{k+1} = u_k + \Delta u$, so $u$ is (a multiple of) the sum of all past errors; this is called **integral control**. The alternative is to make $u$ proportional only to the present error, $u = -K\,y/r$, called **proportional control**. Substituting into $y = d + r u$:

$$
y = d - K y \quad\Rightarrow\quad y = \frac{d}{1 + K}.
$$

With $K = 9$ a tenth of $d$ remains; however large $K$, it never vanishes, and too large a $K$ is unstable too. Integral control keeps changing $u$ as long as the error is not zero, until it is zero, so a constant error leaves no residual. Pulse-to-pulse orbit feedback and LLRF both contain integral action.

### For readers who know control theory

The plant $y = d + r u$ with a measurement delay of $d$ pulses and the integral controller $C(z) = -g/[r(z - 1)]$ give the closed-loop characteristic polynomial $z^{d+1} - z^d + g = 0$: $g < 2$ for $d = 0$ and $g < 1$ for $d = 1$ (`avas.dynamics.control.integral_loop_modulus` gives the modulus of the largest root). With several inputs and outputs, the singular value decomposition diagonalises the response matrix, and each mode kept obeys this equation separately. The continuous-time analogue is the open-loop transfer function $\omega_c e^{-s\tau}/s$: at the crossover frequency $f_c$ the phase is $-90° - 360° f_c\tau$, the phase margin $90° - 360° f_c\tau$, and the loop oscillates when the margin is used up; this is the criterion of the `llrf` lab in the [Feedback](#feedback) chapter. Combining feedforward with feedback, the trade-off between bandwidth and noise, and robustness analysis are standard material of control theory.

## In AVAS {#control-intro-in-avas}

### What AVAS can and cannot do

AVAS is a beam-dynamics simulation code and can also be used as a "virtual accelerator": it predicts the beam, computes the response to steerers and other knobs, and tries out correction and feedback algorithms in simulation. It is **not a control system**: it is not connected to any real equipment, reads no real diagnostic data, and has no real-time loops, interlocks or alarms. AVAS also does not simulate the LLRF, power supplies or the electronics of diagnostic devices; cavity and magnet fields are used as given in the lattice.

### Automatic correction in error studies

An error study (see [Errors and correction](#errors-in-avas)) can correct automatically after each draw of errors: `adjust` makes an element parameter a knob, and `diag_position`, `diag_size` and `diag_energy` mark the targets (see [Beam diagnostics](#diagnostics-in-avas)); every error run optimises the knobs again, running the engine once per evaluation, which amounts to "measure – compare – correct" inside the simulation. The corrections are written to `Adjust_Datas_<group>_<run>.txt`, not back into the lattice file.

```avas-params
adjust
```

### Research page: response, controllability and feedback

The Research page's "Response & controllability" and "Feedback" tabs are about control:

- **Monitors and knobs**: default to the lattice's `diag_position`, `diag_size` and steerers, can be edited, and are saved in `<project>/Research/diagnostics.json`.
- **Response matrix**: "Response of the linear model" computes the derivatives of the readings with respect to the knobs with the linear model, and gives the rank, the condition number, the reading patterns no knob can correct, the knob combinations no monitor sees, and how much monitor noise the correction amplifies onto the knobs; "Response from the engine" runs the engine once per knob (fixed random seed, lattice parameters only).
- **Modes of one period**: writes the centroid motion of one period (marked in the lattice with `lattice n 1` … `lattice_end`) as $x_{n+1} = A x_n + B u_n$, $y_n = C x_n$, and lists whether each mode is controllable and observable.
- **Feedback**: on every pulse, the linear model with the drifting error and noisy readings, corrected by $u \leftarrow u - g R^{+} y$ (readings delayed by $d$ pulses); it plots the rms of the readings with and without feedback.

The linear calculations take no run lock and write no input files; an engine response is a study run, shown with its progress on the Run page, can be paused or stopped, and excludes ordinary runs meanwhile.

### In Python

The dynamics and control analyses are in `avas/dynamics/`, all built on the linear model `avas.model.LinearModel`: `diagnostics.py` (monitors and readings), `response.py` (knobs and response matrices), `systems.py` ($A$, $B$, $C$, $D$ of a period), `control.py` (controllability, observability, mode classes, closed loops) and `feedback.py` (pulse-to-pulse feedback simulation). `examples/research/` has example scripts that can be run cell by cell.

### Agreement with the engine

The orbit response matrix has been checked against the engine: elements passing only drifts and the HWR cavity agree to within 0.06 % of the largest element; those passing through the example `sol` field map differ by 0.1–2.7 %, because of the field map itself (details in [Orbit correction](#orbit-correction-in-avas)). Fix the random seed when comparing engine results.

## Further reading {#control-intro-reading}

- M. G. Minty, F. Zimmermann, *Measurement and Control of Charged Particle Beams*, Springer (2003): the standard reference on beam measurement and control.
- K. J. Åström, R. M. Murray, *Feedback Systems: An Introduction for Scientists and Engineers*, Princeton University Press (2008): an introduction to feedback control for scientists.
- G. F. Franklin, J. D. Powell, A. Emami-Naeini, *Feedback Control of Dynamic Systems*, Pearson: a classic textbook of control theory.
- A. W. Chao et al. (eds.), *Handbook of Accelerator Physics and Engineering*, 2nd ed., World Scientific (2013): sections on beam diagnostics, feedback and control.
- Related chapters of this primer: [Errors and correction](#errors), [Beam diagnostics](#diagnostics), [Orbit correction](#orbit-correction); full bibliography in [References](#references).
