# Beam diagnostics

The beam flies inside an evacuated metal pipe. You cannot see it or touch it, and it is astonishingly fast: the protons of the example beam travel at 17 million metres per second (5.7 % of the speed of light), a bunch passes every 6.15 nanoseconds, and each bunch holds about seven million protons with a total charge of only 1.13 picocoulombs (pC, 1.13 millionths of a millionth of a coulomb). We cannot hold a ruler against it, and we cannot stop it to have a look.

Yet the operators of an accelerator must know where the beam is, how much of it there is, how wide it is, how "disordered" it is, when it arrives and whether it is being lost. The devices that answer these questions are called **beam diagnostics**; they are the "eyes" of the accelerator. As the previous chapter said, the first step of control is measurement; without diagnostics there is no control.

This chapter goes through the most common diagnostic devices one by one: what each measures, which principle it uses and how precisely it measures. You will find that nearly all of them use only three tricks: **listen to the beam's electromagnetic field**, **let the beam hit something**, and **look at what lost particles leave behind**.

## How do you measure an invisible beam? {#diagnostics-why}

First the quantities we want to know:

| We want to know | It is called | Common device |
|---|---|---|
| How many particles | current, charge | beam current transformer, Faraday cup |
| Where the centre is | position (centroid) | beam position monitor (BPM) |
| How wide | profile, rms size | wire scanner, screen, residual-gas profile monitor |
| How "disordered" | emittance | slit and wire, quadrupole scan |
| When it arrives | phase, time of flight, energy | phase probe (often a BPM doubles as one) |
| How long the bunch is | bunch length | bunch shape monitor |
| How much is lost, and where | beam loss | beam loss monitor, difference of two transformers |

Then the three tricks:

1. **Listen to the field**: the beam is an electric current with electric and magnetic fields around it. Electrodes or coils on the pipe wall "hear" it pass. These devices do not touch the beam; they are called **non-intercepting** (or non-destructive) and can stay on all the time.
2. **Hit something**: put a wire, a screen or a metal cup into the beam and look at the signal the beam produces when it hits. These devices block or disturb the beam; they are called **intercepting** (destructive) and are normally used only briefly during commissioning and tuning.
3. **Look at the lost particles**: particles hitting the pipe produce radiation, and detectors outside the pipe tell you where beam is being lost.

## The beam's "shadow" on the pipe wall: image charge {#diagnostics-image-charge}

High-school physics covers **electrostatic induction**: bring a positively charged ball close to a grounded piece of metal, and negative charge appears on the metal surface on the side facing the ball. This is called **induced charge**, or **image charge**. When the ball moves, the induced charge moves with it over the metal surface.

A bunch flying through a metal pipe is like a positively charged ball moving along the middle of the pipe. A ring of negative image charge on the wall always runs along with it, equal in amount to the bunch charge and opposite in sign. Image charge moving along the wall is a current flowing along the wall, called the **wall current**, equal in size to the beam current.

This is the basis of every "listen to the field" device:

- The **closer** the bunch is to one side of the wall, the more image charge there is on that side. Cut a few small windows into the wall, fit electrodes insulated from each other, compare the charge each one receives, and you know which way the beam is off (BPM).
- **When** the image charge arrives is when the bunch arrives (phase probe).
- The **total** image charge is the bunch charge; but to measure it, the wall current must be made to bypass the measuring coil, which is why the pipe of a beam current transformer has a ceramic insert (see below).

One more point we will need later: the example protons travel at only 5.7 % of the speed of light, so their electric field is spread out a long way forwards and backwards, and so is the image charge on the wall. In a pipe of radius 20 mm, the signal on an electrode while a bunch passes lasts about 1.8 nanoseconds (estimated in "Going deeper: timing measurements"), more than twenty times longer than the bunch itself (rms about 77 picoseconds).

## Measuring position: the beam position monitor {#diagnostics-bpm}

A **beam position monitor** (BPM) most often looks like four small round electrodes in the pipe wall, called **buttons**, one each on the right, top, left and bottom. When a bunch passes, each button receives part of the image charge and produces an electrical signal.

With the beam in the centre, all four signals are equal. If the beam moves right, the right button is nearer and its signal grows; the left button is farther away and its signal shrinks. So "right minus left" says how far the beam is off; then divide by "right plus left":

$$
\frac{\Delta}{\Sigma} = \frac{R - L}{R + L}.
$$

Why divide by the sum? Because with more beam current all four signals grow together, so "right minus left" grows too, although the beam has not moved. Dividing by "right plus left" cancels the current exactly and leaves only the position information. Near the centre this ratio is proportional to the position, approximately $2x/a$ ($a$ is the pipe radius). So the position can be read as

$$
x_{\text{reading}} = \frac{a}{2}\cdot\frac{\Delta}{\Sigma}.
$$

The top and bottom buttons give the vertical position $y$ in the same way.

The lab below shows the signals of the four buttons when a charged "line" (standing for the beam) is inside a round pipe, and the position read with the formula above.

```avas-lab
bpm
```

By default the beam is at $x = 2$ mm in a pipe of radius 20 mm: the right button's signal is 0.195, the left one's 0.130 (in shares of the image charge), $\Delta/\Sigma = 0.198$, and the reading is 1.980 mm, 0.020 mm or 1 % less than the true position. Now drag $x$ outwards:

| True $x$ | Reading $(a/2)\cdot\Delta/\Sigma$ | Reading minus true value |
|---|---|---|
| 0.1 mm | 0.100 mm | 0 |
| 2 mm | 1.980 mm | −0.020 mm |
| 5 mm | 4.706 mm | −0.294 mm |
| 10 mm | 8.000 mm | −2.000 mm |
| 15 mm | 9.600 mm | −5.400 mm |

Near the centre the reading is accurate; the farther from the centre, the more the reading "lags behind", and at 15 mm it reads only 9.6 mm. The "reading against the true position" plot on the right draws this bending curve. Real BPMs are therefore **calibrated** in advance: the whole curve is measured with known positions (for example with a current-carrying wire placed at different positions), and the curve is then used to turn readings back into true positions. Try changing the radius $a$ to 10 mm: the same 2 mm offset now has a reading error of about 4 % instead of 1 %, because the beam is "farther off" relative to the pipe.

How precise is a BPM? A good one resolves a few micrometres to a few tens of micrometres. That is demanding for the electronics: with $a = 20$ mm, resolving 10 µm means measuring $\Delta/\Sigma$ to 0.001, that is, comparing the left and right signals to one part in a thousand. If the gains of the two electronic channels differ by just 1 %, a beam exactly in the centre reads as about 50 µm off (derived in "Going deeper: the BPM reading formula"). This "zero offset", together with where the BPM is mounted and where the magnetic centre of the quadrupole is, decides how well orbit correction can do in the end (see [Orbit correction](#orbit-correction-practice)).

A BPM also has two important limitations: it measures only the **centre of the beam**, not how wide the beam is; and it measures only **where it sits**, so it cannot see what happens between two BPMs.

## Measuring current: beam current transformers and Faraday cups {#diagnostics-current}

A **beam current transformer** is a ring of magnetic material around the outside of the pipe, with a coil wound on the ring.

It works by two laws from school physics. First, a straight current-carrying wire is surrounded by a circular magnetic field of strength $B = \mu_0 I / (2\pi r)$ at distance $r$ (Ampère's law); the beam is such a "current-carrying wire". Second, when the magnetic flux through a coil changes, a voltage is induced in the coil (Faraday's law of induction). When the beam changes, the field in the ring changes and the coil gives a signal proportional to the beam. You can think of it as a transformer: the beam is a primary winding of a single turn, and the coil on the ring is the secondary.

How weak is the beam's magnetic field? The average current of the example beam is 0.184 mA; at $r = 5$ cm

$$
B = \frac{4\pi\times10^{-7} \times 1.84\times10^{-4}}{2\pi \times 0.05}\ \mathrm{T} = 7.4\times10^{-10}\ \mathrm{T},
$$

less than 1 nanotesla, seventy thousand times weaker than the Earth's field (about 50 µT). The ring is therefore made of a material of very high permeability that "gathers" the field, and it is wrapped in magnetic shielding.

There is one more detail. As we saw, the wall carries a wall current equal and opposite to the beam current. If it also passed through the ring, the two fields would cancel and the transformer would measure nothing. So the section of pipe at the transformer has a ceramic insert that breaks the metal wall and makes the wall current go around the outside of the ring.

Induction responds only to a **changing** current, so an ordinary transformer measures pulsed or bunched beams; measuring a steady direct current needs more elaborate circuitry (a DC current transformer, DCCT). A good beam current transformer resolves currents of the order of microamperes. With one at each of two places, comparing the readings tells how much beam is lost in between; this is called the **transmission**.

A **Faraday cup** is a metal cup that "catches" the whole beam; all the charge flows to ground through an ammeter, so the reading is the current directly, with no calibration needed: the most reliable absolute measurement. It has two costs: the beam is blocked completely, and the full beam power lands in the cup. The example beam carries $1.527\ \mathrm{MV} \times 0.184\ \mathrm{mA} = 281\ \mathrm{W}$, which a Faraday cup can still take; for a megawatt high-energy beam it is impossible. Also, protons hitting the metal knock out electrons, and electrons escaping make the reading too high, so the mouth of the cup has a ring at negative voltage that pushes the electrons back.

## Measuring width: profile monitors and wire scanners {#diagnostics-profile}

To know how wide the beam is, we measure its **profile**: how the particles are distributed along $x$ across the beam.

A **wire scanner** moves a very thin wire (carbon or tungsten, a few to a few tens of micrometres thick) slowly through the beam. At each position of the wire, only the small fraction of particles that hit the wire interacts with it: protons hitting the wire knock electrons out of its surface (so the wire carries a current signal), or produce scattering and radiation (picked up by a detector downstream). The stronger the signal, the more particles at that position. Recording the signal at each wire position gives a profile curve, from which the rms size can be computed (method in "Going deeper: from profiles to emittance").

Note that the wire measures a **projection**: a vertical wire at $x$ collects all particles with the same $x$ and any $y$, so it gives the distribution in $x$ with $y$ "squashed flat".

A wire scanner is mildly intercepting: a 30 µm wire in the centre of the example beam (rms 0.95 mm) blocks only about 1.3 % of the particles. But low-energy protons do not get far: in a wire a few tens of micrometres thick they lose most or even all of their energy. At the full power of the example beam those 1.3 % are about 3.5 W, all heating a wire thinner than a hair, and the wire burns through. So wire scanners in low-energy accelerators are normally used only with short-pulse, low-repetition tuning beams.

Other profile devices:

- **Harp (wire grid)**: many wires side by side measure the whole profile at once, without moving.
- **Screen**: a plate that glows where it is hit; a camera records the spot, giving a two-dimensional image directly. It blocks the beam completely and is used only for tuning.
- **Residual-gas profile monitor**: there is always a little residual gas in the pipe; the beam ionises gas molecules, and collecting the ions or electrons produced gives the profile. It does not touch the beam at all, but the signal is weak.
- **Laser wire**: only for beams of negative hydrogen ions (H⁻, a proton with two electrons). A laser knocks off the extra electron, and counting the knocked-off electrons gives the profile, again without touching the beam.

The wire position can be controlled to the micrometre, and rms sizes can usually be measured to a few tens of micrometres. But the rms is very sensitive to the tails of the distribution and to the background, and the largest error of a measurement often comes from subtracting the background; a deep section below shows an example.

## Measuring emittance: both position and angle are needed {#diagnostics-emittance}

The [Phase space](#phase-space-emittance) chapter explained that emittance is the area of the ellipse the particles occupy in the "position–angle" plot, a measure of how "disordered" the beam is. A profile only tells us how the positions are distributed, not the angles. Two beams can be equally wide, one flying almost parallel, the other spreading in all directions; their emittances are very different. So measuring emittance needs information on both position and angle. There are three common methods:

**Slit and wire.** A plate with a narrow slit is put into the beam and lets through only particles with $x$ in a narrow strip. These particles have almost the same position but different angles; after flying a distance $L$, particles with different angles have separated, and a wire downstream measures the width of this beamlet, which tells the distribution of angles within the strip. Moving the slit from left to right gives the whole phase-space plot. For example, the rms angle of the example beam is about 3 mrad, so after 0.5 m the beamlet has widened to about 1.5 mm, easy to measure.

**Quadrupole scan.** No slit, only a quadrupole and a profile monitor downstream. When the strength of the quadrupole changes, the beam size downstream changes: with weak focusing the beam is still spreading; with too strong focusing it first converges to a point and then spreads again; in between there is a narrowest size. Measure a few pairs of "quadrupole strength – beam size", plot them, and you get a parabola whose shape gives the emittance. For the example beam, the narrowest rms size 1 m behind the quadrupole is 2.53 mm, and this minimum equals "distance × emittance ÷ entrance size" (derivation and full worked example in "Going deeper: from profiles to emittance").

**Several profile monitors.** Instead of changing the quadrupole, measure the size once at several different places. Between them the beam changes according to known transfer matrices, and three or more positions are enough to compute the emittance; the principle is the same as for the quadrupole scan.

Emittance is much harder to measure than position or current; the error is typically about 10 %. At high current, space charge makes the beam grow extra in the measuring section (see [Space charge](#space-charge)), which the calculation must take into account.

## Measuring time: phase, energy and bunch length {#diagnostics-timing}

In an RF accelerator the bunches must arrive at each cavity at the right moment (see [Longitudinal motion](#longitudinal)). So "when it arrives" matters as much as "where it is".

**Phase.** Add the signals of a BPM's four buttons (the sum signal); its arrival time is the arrival time of the bunch. Compared with the accelerator's RF reference signal, it gives the bunch **phase**. At 162.5 MHz one RF period is 6.15 ns, corresponding to 360°, so 1° is 17.1 ps. In practice BPMs often double as phase probes.

**Energy: time of flight.** Two phase probes 1 m apart measure how long a bunch takes to fly that metre, which gives its speed and hence its energy. At low energy this is very sensitive: the example protons take 58.5 ns per metre, and an energy difference of 1 keV changes the flight time by 19.1 ps, or 1.12° (derived in "Going deeper: timing measurements"). Note that a phase measurement only gives "the position within one period": 58.5 ns is 9.51 RF periods, and the integer part 9 must be determined separately (for example by knowing the energy roughly beforehand, or with probes at several different spacings).

Energy can also be measured with a **dipole**: particles of different momentum bend differently in a dipole and leave it at different positions (this is called **dispersion**, see [Dipole](#dipole-dispersion)), so measuring the position at the exit gives the momentum.

**Bunch length.** The example bunch has an rms length of about 1.3 mm, about 77 ps. As we saw, the signal of a slow beam on an electrode lasts about 1.8 ns, much longer than the bunch, so a button "cannot see" the shape of the bunch. Low-energy proton accelerators often use a **bunch shape monitor**: a thin wire at high voltage is put into the beam; the electrons the beam knocks out of it are accelerated by the high voltage and then pass through a deflecting field synchronised with the accelerator's RF. Electrons knocked out at different moments are deflected to different positions, so "time" is turned into "position", and measuring the position gives the time distribution of the bunch. Its resolution can reach the order of 1° of RF phase. For electron beams close to the speed of light, the electric field is squeezed very thin, and fast electrodes, RF deflecting cavities or streak cameras can measure the bunch directly.

## Measuring losses: beam loss monitors {#diagnostics-losses}

A particle that hits the pipe wall or a magnet is lost. High-energy protons hitting metal cause nuclear reactions that produce gamma rays and neutrons, and this radiation gets out through the pipe wall. **Beam loss monitors** placed along the outside of the line tell where beam is being lost and how much. Common types are:

- **Ionisation chamber**: a small gas-filled can. Radiation passing through ionises the gas; collecting the charge produced gives a current that reflects the strength of the radiation.
- **Scintillation detector**: radiation hitting a scintillating material produces faint light, amplified into an electrical signal by a photomultiplier tube. Fast and very sensitive.

Beam loss monitors have two uses. One is **machine protection**: as soon as the loss somewhere suddenly grows (say a magnet power supply trips and the beam swerves towards the wall), the beam is switched off within a few to a few tens of microseconds, before the equipment is damaged. The other is **limiting activation**: high-power proton accelerators require losses along the line to stay below about 1 watt per metre; otherwise the equipment is too radioactive after shutdown for people to maintain it (see [High-energy transport](#journey-hebt-losses)).

There is a special case at low energy: 1.5 MeV protons are far below the Coulomb barrier of common metals and cause hardly any nuclear reactions (see [Medium-energy transport](#journey-mebt-chopper)), so losing them produces almost no radiation, and loss monitors cannot see it. Losses in low-energy sections are therefore mainly measured by comparing the readings of two beam current transformers.

## Diagnostic devices at a glance {#diagnostics-summary}

The "typical resolution" in the table is only an order of magnitude; actual values depend on the machine, the beam and the electronics.

| Device | Measures | Principle | Blocks the beam? | Typical resolution |
|---|---|---|---|---|
| BPM (buttons) | centre position; the sum signal gives the phase | image charge, Δ/Σ | no | position a few µm to tens of µm; phase a few tenths of a degree |
| Beam current transformer | current, transmission | Ampère's law + induction | no | order of µA |
| Faraday cup | current (absolute) | catches all the charge | yes | very accurate, but low power only |
| Wire scanner, harp | profile, rms size | secondary electrons or radiation from the wire | mildly | tens of µm |
| Screen | two-dimensional image | glows when hit, recorded by a camera | yes | limited by camera and screen |
| Residual-gas profile monitor, laser wire | profile | ionised gas, laser-detached electrons | no | weak signal, needs accumulation |
| Slit and wire, quadrupole scan | emittance | position + angle (or size against focusing) | yes / depends on the profile device | about 10 % |
| Phase probe (time of flight) | phase, energy | arrival time of the signal | no | below 1 keV at low energy |
| Bunch shape monitor | bunch length and shape | secondary electrons + RF deflection | mildly | about 1° of RF phase |
| Beam loss monitor | where and how much is lost | ionisation chamber, scintillator collecting radiation | no | far below 1 W/m in high-energy sections |

## The limits of measurement {#diagnostics-limits}

The numbers diagnostics give are not the beam's "truth". Keep a few things in mind when reading them:

- **Noise**: every reading has a random error. Averaging $N$ measurements of the same quantity shrinks the random error to $1/\sqrt{N}$ (for the same reason as the statistical error of macro-particles in a simulation, see [Simulation](#simulation-deep-statistics)), but slowly drifting errors do not average away.
- **Zero and calibration**: the zero of a BPM is not necessarily at the magnetic centre of the quadrupole; the sensitivities of transformers and wire scanners must be calibrated. These systematic errors go straight into the residual error after correction.
- **Measured only where there is a device**: the beam between BPMs, or between profile monitors, is invisible. The orbit-correction chapter showed an example where the orbit at the BPMs is already very good while it is still 0.2 mm off between them (see [Orbit correction](#orbit-correction-svd)).
- **Measuring changes the beam**: wires, slits and screens block or scatter part of the beam, so they are used only for tuning; in operation we rely mainly on devices that do not touch the beam.
- **Only statistics are measured**: a BPM gives the mean position and a wire the rms of a projection, neither is the full distribution; two beams with the same rms can have very different tails.

These limits decide what control can achieve: what cannot be seen cannot be corrected, and what is measured imprecisely is corrected imprecisely. This is the subject of the [Controllability and observability](#controllability) chapter later on.

## Think about it {#diagnostics-think}

> **Think 1**: If the beam current doubles, does the BPM reading change?
>
> Answer: No. All four button signals double, so numerator and denominator of $\Delta/\Sigma$ both double and the ratio stays the same. That is exactly why we divide by the sum.

> **Think 2**: Why must the section of pipe at a beam current transformer be ceramic, not entirely metal?
>
> Answer: The metal wall carries a wall current equal and opposite to the beam current. If it also went through the magnetic ring, the fields of the two currents would cancel and the transformer would see no signal. The ceramic breaks the wall and makes the wall current go around the outside of the ring.

> **Think 3**: In a low-energy proton accelerator the loss monitors show "almost no loss" all day, but two transformers differ by 3 %. Which do you believe?
>
> Answer: The transformers. Protons of around 1.5 MeV cause hardly any nuclear reactions and produce almost no radiation when lost, so the loss monitors cannot see them; the difference between the transformers directly shows how much charge is missing.

> **Think 4**: In a quadrupole scan the narrowest beam downstream is 2.53 mm. If the emittance doubled, how narrow could it get?
>
> Answer: The minimum size equals "distance × emittance ÷ entrance size"; with the same entrance size, the minimum also doubles, to about 5.06 mm. The larger the emittance, the less the beam can be focused down.

## Common misconceptions {#diagnostics-misconceptions}

- **"The BPM reading is the beam position"**: only approximately, near the centre. Far from the centre the reading is too small (10 mm reads as 8 mm in the lab), and there is a zero offset; BPMs must be calibrated.
- **"A BPM measures how wide the beam is"**: an ordinary four-button BPM measures only the centre position. For the width use profile devices such as wire scanners or screens.
- **"A transformer can measure any beam"**: an ordinary transformer responds only to changing currents; DC beams need a DC current transformer. And the wall current must bypass it.
- **"No alarm from the loss monitors means no loss"**: low-energy protons produce almost no radiation when lost, and a monitor is only sensitive near itself.
- **"One profile measurement gives the emittance"**: a profile contains only position information; emittance also needs the angles, which takes at least a slit, a change of focusing or measurements at several places.
- **"Intercepting devices can be used freely"**: low-energy protons leave all their energy in a wire or screen and burn it at full power; interception also changes the beam downstream.

## Going deeper: the BPM reading formula {#diagnostics-deep-bpm}

The main text gave the reading $(a/2)\cdot\Delta/\Sigma$, and the lab showed it falling short the farther the beam is from the centre. This section derives the exact reading formula step by step to see where the shortfall comes from. The first five steps use only fractions; the final paragraph is for readers who know complex analysis.

### Step 1: how the image charge is distributed on the wall

Treat the beam as a thin charged line parallel to the pipe axis at $(x, y)$, at distance $r = \sqrt{x^2 + y^2}$ from the axis and at azimuth $\varphi$. Electrostatics shows (this step needs mathematics beyond high school, so we just state the result) that on a grounded round pipe of radius $a$, the share of the image charge per radian at azimuth $\theta$ is

$$
s(\theta) = \frac{1}{2\pi}\cdot\frac{a^2 - r^2}{a^2 + r^2 - 2ar\cos(\theta - \varphi)}.
$$

First check two easy properties. With the beam in the centre ($r = 0$), $s = 1/(2\pi)$, independent of $\theta$: the image charge is spread evenly, and all four buttons get $1/(2\pi) = 0.159$. With the beam off towards $\theta = \varphi$, the denominator is smallest at $\theta = \varphi$, where the image charge is largest: the "shadow" is densest on the wall closest to the beam. The lab treats each button as a point, so its signal is $s(\theta)$ at the button's angle.

### Step 2: the left and right signals for a beam on the x axis

Let the beam be at $(x, 0)$, so $r = x$ and $\varphi = 0$. The right button is at $\theta = 0$, $\cos 0 = 1$, the denominator is $a^2 + x^2 - 2ax = (a - x)^2$, and the numerator is $a^2 - x^2 = (a - x)(a + x)$. Cancelling,

$$
R = \frac{1}{2\pi}\cdot\frac{a + x}{a - x}.
$$

The left button is at $\theta = \pi$, $\cos\pi = -1$, and likewise

$$
L = \frac{1}{2\pi}\cdot\frac{a - x}{a + x}.
$$

With the lab's defaults $a = 20$ mm, $x = 2$ mm: $R = (22/18)/(2\pi) = 0.1945$ and $L = (18/22)/(2\pi) = 0.1302$, as the lab shows.

### Step 3: difference over sum

$$
\frac{\Delta}{\Sigma} = \frac{R - L}{R + L}
= \frac{\dfrac{a + x}{a - x} - \dfrac{a - x}{a + x}}{\dfrac{a + x}{a - x} + \dfrac{a - x}{a + x}}.
$$

Multiply numerator and denominator by $(a - x)(a + x)$:

$$
\frac{\Delta}{\Sigma} = \frac{(a + x)^2 - (a - x)^2}{(a + x)^2 + (a - x)^2} = \frac{4ax}{2a^2 + 2x^2} = \frac{2ax}{a^2 + x^2}.
$$

The common factor $1/(2\pi)$ has cancelled too. The beam current cancels in the same way: multiplying all signals by the same number does not change the ratio. With $x = 2$, $a = 20$: $80/404 = 0.198$, as in the lab.

### Step 4: why the reading falls short

The reading is $(a/2)\cdot\Delta/\Sigma$:

$$
x_{\text{reading}} = \frac{a}{2}\cdot\frac{2ax}{a^2 + x^2} = \frac{x}{1 + x^2/a^2}.
$$

The denominator is larger than 1, so the reading is always smaller than the true value, and the larger $x$, the larger the denominator. Check: for $x = 10$ mm, $10/(1 + 0.25) = 8.0$ mm; for $x = 15$ mm, $15/(1 + 0.5625) = 9.6$ mm, exactly the numbers in the table of the main text. For small $x$, $1/(1 + \epsilon) \approx 1 - \epsilon$, so the reading error is about $-x^3/a^2$: for $x = 2$ mm, $-8/400 = -0.02$ mm. This is where "linear near the centre, saturating far away" comes from. The reading reaches its largest value $a/2$ at $x = a$, so however far the beam is off, the reading never exceeds half the radius.

### Step 5: sensitivity and zero offset

Near the centre $\Delta/\Sigma \approx 2x/a$: a 10 µm change of position changes $\Delta/\Sigma$ by only $2 \times 0.01/20 = 0.001$. That is where "comparing the left and right signals to one part in a thousand" in the main text comes from.

Now the consequence of unbalanced electronics. Let the beam be in the centre and the right channel's gain be 1 % higher than the left's, so we measure $R = 1.01\,L$:

$$
\frac{\Delta}{\Sigma} = \frac{1.01 - 1}{1.01 + 1} = 0.004975,\qquad x_{\text{reading}} = 10\ \mathrm{mm} \times 0.004975 \approx 0.050\ \mathrm{mm}.
$$

The beam is clearly in the centre, yet the BPM says it is 50 µm off. So BPMs are calibrated channel by channel, and their true zero is found with the beam itself (beam-based alignment, see [Orbit correction](#orbit-correction-practice)).

### Step 6: off in both directions at once

With the beam at $(2, 2)$ mm, $r^2 = 8$; for the right button at $\theta = 0$, $\cos(0 - 45°) = 0.707$ and $2ar\cos = 2 \times 20 \times 2.828 \times 0.707 = 80$, so

$$
R = \frac{1}{2\pi}\cdot\frac{400 - 8}{400 + 8 - 80} = \frac{1}{2\pi}\cdot\frac{392}{328},\qquad
L = \frac{1}{2\pi}\cdot\frac{392}{488},
$$

$\Delta/\Sigma = (1.1951 - 0.8033)/(1.1951 + 0.8033) = 0.1961$, a reading of 1.961 mm, less than the 1.980 mm for $y = 0$. In other words, a vertical offset makes the horizontal reading slightly smaller: the two directions are not entirely independent. Set $y$ to 2 mm in the lab to check; the vertical reading is also 1.961 mm.

### For readers who know complex analysis

The $s(\theta)$ of step 1 is the Poisson kernel of the unit disc, with the expansion $s = \frac{1}{2\pi}\left[1 + 2\sum_{n\ge1}(r/a)^n\cos n(\theta - \varphi)\right]$. Combining four buttons keeps only some of the multipole terms: the $\cos\theta$ term of $\Delta/\Sigma$ gives the linear sensitivity $2/a$, the third-order terms give the $-x^3/a^2$ correction and the cross-coupling of $x$ and $y$. Real buttons subtend a finite angle, which amounts to integrating the kernel over that angle, and the sensitivity is slightly below $2/a$; calibration usually fits a two-dimensional polynomial to the inverse of the map $(x, y) \mapsto (\Delta/\Sigma)_{x,y}$. A button delivers (through its capacitance and the load resistance) the time derivative of the image current; for beams with β ≪ 1, the spreading of the image charge along $z$ makes the sensitivity depend on frequency and velocity, so low-energy BPMs are calibrated for the beam velocity.

## Going deeper: from profiles to emittance — the quadrupole scan {#diagnostics-deep-emittance}

The main text said that the "size against strength" curve of a quadrupole scan is a parabola from which the emittance follows. This section starts from "how to compute the rms from a profile" and works through a complete quadrupole scan step by step. It uses only weighted averages, completing the square and three linear equations in three unknowns; the final paragraph is for readers who know linear algebra.

### Step 1: the rms from a profile

A wire measures these signals (arbitrary units) at five positions:

| Wire position $x_i$ | −2 mm | −1 mm | 0 | 1 mm | 2 mm |
|---|---|---|---|---|---|
| Signal $s_i$ | 1 | 4 | 6 | 4 | 1 |

The signal is proportional to the number of particles there, so we average with the signals as weights. The centre is

$$
\bar x = \frac{\sum_i s_i x_i}{\sum_i s_i} = \frac{-2 - 4 + 0 + 4 + 2}{16} = 0,
$$

and the rms size is the square root of the weighted average of "distance from the centre, squared":

$$
\sigma = \sqrt{\frac{\sum_i s_i (x_i - \bar x)^2}{\sum_i s_i}} = \sqrt{\frac{4 + 4 + 0 + 4 + 4}{16}} = 1.0\ \mathrm{mm}.
$$

Now suppose the electronics add a constant background of 0.5 to every reading, and the wire scans wider, from −4 mm to 4 mm (the true signal is zero at ±3 and ±4 mm). The readings become 0.5, 0.5, 1.5, 4.5, 6.5, 4.5, 1.5, 0.5, 0.5, with sum 20.5 and squared weighted sum

$$
2 \times (0.5 \times 16 + 0.5 \times 9 + 1.5 \times 4 + 4.5 \times 1) = 46,
$$

so $\sigma = \sqrt{46/20.5} = 1.50$ mm. The background is only 8 % of the peak, yet the rms comes out 50 % too large: points far from the centre are weighted by their distance squared, so a little background is magnified a lot. Before computing an rms, subtract the background and sum only over the range that has signal.

### Step 2: why one profile is not enough

The [Phase space](#phase-space-sigma) chapter explained that the shape of a beam in one plane takes three averages: $\langle x^2\rangle$ (the size squared), $\langle x'^2\rangle$ (the angle squared) and $\langle xx'\rangle$ (whether position and angle have the same sign, i.e. whether the beam is spreading or converging). The emittance is

$$
\varepsilon = \sqrt{\langle x^2\rangle\langle x'^2\rangle - \langle xx'\rangle^2}.
$$

One profile gives only one number, $\langle x^2\rangle$. To get three numbers, the size must be measured under at least three different conditions. A quadrupole scan uses three (or more) different focusing strengths.

### Step 3: a particle from the quadrupole to the wire

Treat the quadrupole as a thin lens of strength $k = 1/f$ (the inverse focal length, in 1/m), with the wire scanner a distance $L$ behind it. A particle with position $x_0$ and angle $x_0'$ at the lens has angle $x_0' - k x_0$ after it, and after flying $L$:

$$
x_1 = x_0 + L\,(x_0' - k x_0) = (1 - kL)\,x_0 + L\,x_0'.
$$

(With mm, mrad and m together no unit conversion is needed, see [Drift](#drift-deep-matrix).)

### Step 4: square and average over all particles

Write $u = 1 - kL$. Squaring $x_1 = u x_0 + L x_0'$,

$$
x_1^2 = u^2 x_0^2 + 2uL\,x_0 x_0' + L^2 x_0'^2,
$$

and averaging over all particles ($u$ and $L$ are the same for every particle and come out of the average):

$$
\sigma_1^2 = u^2\langle x_0^2\rangle + 2uL\,\langle x_0x_0'\rangle + L^2\langle x_0'^2\rangle.
$$

This is a quadratic function of $u$, and hence of $k$: the size squared traces a parabola against the quadrupole strength.

### Step 5: completing the square to find the narrowest beam

Complete the square in $u$:

$$
\sigma_1^2 = \langle x_0^2\rangle\left(u + \frac{L\langle x_0x_0'\rangle}{\langle x_0^2\rangle}\right)^2 + L^2\,\frac{\langle x_0^2\rangle\langle x_0'^2\rangle - \langle x_0x_0'\rangle^2}{\langle x_0^2\rangle}.
$$

The first term is a square, at least zero; the numerator of the second is exactly $\varepsilon^2$. Hence

$$
\sigma_{1,\min} = \frac{L\,\varepsilon}{\sigma_0},
$$

where $\sigma_0 = \sqrt{\langle x_0^2\rangle}$ is the size at the lens. This is the "distance × emittance ÷ entrance size" of the main text.

With the example beam ($x$ plane, see [Phase space](#phase-space-twiss)): $\langle x_0^2\rangle = 0.910\ \mathrm{mm^2}$, $\langle x_0x_0'\rangle = 1.587\ \mathrm{mm\cdot mrad}$, $\langle x_0'^2\rangle = 9.157\ \mathrm{mrad^2}$, $\varepsilon = 2.412\ \mathrm{mm\cdot mrad}$, $\sigma_0 = 0.954$ mm. Take $L = 1$ m: $\sigma_{1,\min} = 2.412/0.954 = 2.53$ mm. The minimum is at $u = -1.587/0.910 = -1.743$, i.e. $k = 2.74\ \mathrm{m^{-1}}$ (focal length 0.36 m); for a quadrupole 0.1 m long that is a gradient of about 4.9 T/m.

### Step 6: three averages back from three measurements

Now the other way round: we do not know the beam's three averages, we have only measured three times. Rearrange step 4 as a polynomial in $k$:

$$
\sigma_1^2 = A k^2 + B k + C,\quad
A = L^2\langle x_0^2\rangle,\quad
B = -2L\left(\langle x_0^2\rangle + L\langle x_0x_0'\rangle\right),\quad
C = \langle x_0^2\rangle + 2L\langle x_0x_0'\rangle + L^2\langle x_0'^2\rangle.
$$

Suppose we measured at $L = 1$ m (these are exactly the values the example beam should give):

| Quadrupole strength $k$ | 0 | 2 m⁻¹ | 4 m⁻¹ |
|---|---|---|---|
| Measured rms size | 3.639 mm | 2.626 mm | 2.798 mm |
| Squared $\sigma_1^2$ | 13.241 | 6.894 | 7.830 |

$k = 0$ gives $C = 13.241$ directly. The other two equations are $4A + 2B = 6.894 - 13.241 = -6.347$ and $16A + 4B = 7.830 - 13.241 = -5.411$. Subtract twice the first from the second: $8A = 7.283$, $A = 0.910$; back-substituting gives $B = -4.994$. Then

$$
\langle x_0^2\rangle = \frac{A}{L^2} = 0.910,\qquad
\langle x_0x_0'\rangle = \frac{1}{L}\left(-\frac{B}{2L} - \langle x_0^2\rangle\right) = 2.497 - 0.910 = 1.587,
$$

$$
\langle x_0'^2\rangle = \frac{C - \langle x_0^2\rangle - 2L\langle x_0x_0'\rangle}{L^2} = 13.241 - 0.910 - 3.174 = 9.157,
$$

and $\varepsilon = \sqrt{0.910 \times 9.157 - 1.587^2} = \sqrt{5.818} = 2.412\ \mathrm{mm\cdot mrad}$. Multiplied by $\beta\gamma = 0.0571$ that is a normalised emittance of 0.1377 π·mm·mrad, exactly the third number of `twissx` in the example `beam.txt`.

### Step 7: errors in a real measurement

Three points fix a parabola exactly, but every point has a measurement error, so the result can be quite wrong, even the impossible $\langle x^2\rangle\langle x'^2\rangle - \langle xx'\rangle^2 < 0$. In practice one measures a dozen or more points and fits the parabola by **least squares**, as for the regression line in school mathematics. The points must cover both sides of the minimum; measured on one side only, the curvature of the parabola is poorly determined. In addition, the thin-lens approximation, calibration errors of the quadrupole strength and space charge (at the example current a 1 m drift makes the beam about 1.5 % wider, see [Drift](#drift-space-charge)) all enter the result, which is why emittance measurements are typically only accurate to about 10 %.

### For readers who know linear algebra

In σ-matrix form $\Sigma_1 = M\Sigma_0M\tp$; expanding the $(1,1)$ element, each measurement gives one linear equation in the three independent elements of $\Sigma_0$, $\sigma_{1,j}^2 = M_{11,j}^2\Sigma_{11} + 2M_{11,j}M_{12,j}\Sigma_{12} + M_{12,j}^2\Sigma_{22}$. $n$ measurements form an overdetermined $n \times 3$ system solved by weighted least squares, whose covariance matrix gives the errors of $\Sigma_0$, propagated through $\varepsilon = \sqrt{\det\Sigma_0}$. The multi-monitor method simply replaces the different $M_j$ by the transfer matrices from the entrance to each monitor; the system becomes ill-conditioned when the phase advances of the $M_j$ differ too little. To get the full phase-space distribution rather than just the second moments, more projections can be combined in a tomographic reconstruction (the same mathematics as CT).

## Going deeper: timing measurements — time of flight and signal length {#diagnostics-deep-timing}

This section derives two numbers from the main text: 1 keV of energy difference gives 19.1 ps of flight-time difference per metre, and the signal of a slow beam on an electrode is about 1.8 ns long. It uses only the school formula for kinetic energy, approximations and Coulomb's law; the final paragraph is for readers who know special relativity and electrodynamics.

### Step 1: time of flight

The example protons travel at $v = 0.0570\,c = 1.708\times10^{7}$ m/s and take $1/v = 58.5$ ns per metre. The RF period at 162.5 MHz is $1/(162.5\times10^{6}) = 6.15$ ns, so the flight time for 1 m is $58.5/6.15 = 9.51$ periods. A phase probe only tells us the "0.51 of a period" part (184°); the integer 9 must be known otherwise.

### Step 2: a little more energy, how much more speed?

The example protons are much slower than light, so the school formula $W = \frac12 mv^2$ applies and $v \propto \sqrt{W}$. When the energy changes by a small fraction $\epsilon$, the speed changes by about half that fraction:

$$
\sqrt{1 + \epsilon} \approx 1 + \frac{\epsilon}{2}.
$$

(Check: $\sqrt{1.01} = 1.00499$, indeed about $1 + 0.005$.) For 1 keV of energy difference $\epsilon = 1/1527 = 6.55\times10^{-4}$, and the relative change of speed is $3.27\times10^{-4}$.

### Step 3: flight-time and phase difference

The flight time is $t = D/v$; if the speed is larger by a small fraction $\delta$, the time is smaller by the same fraction ($1/(1 + \delta) \approx 1 - \delta$). The time difference per metre is

$$
\Delta t = 58.5\ \mathrm{ns} \times 3.27\times10^{-4} = 19.1\ \mathrm{ps},
$$

or in phase $19.1\ \mathrm{ps}/6.15\ \mathrm{ns} \times 360° = 1.12°$. If the phase can be measured to 0.1°, two probes 1 m apart resolve about 0.09 keV, i.e. $6\times10^{-5}$ of the energy; a longer spacing is more sensitive, but the integer number of periods becomes harder to determine.

### Step 4: the electric field at the wall as a charge passes

How long the image charge is spread out on the wall depends on how long the bunch's field takes to sweep past it. First ignore the pipe and estimate with Coulomb's law: a charge $q$ flies along the axis, and a point on the wall is at distance $a$ from the axis. At $t = 0$ the charge is right opposite the point; at time $t$ it has moved on by $vt$ along the axis, and the distance between them is $\rho = \sqrt{a^2 + (vt)^2}$. The Coulomb field has magnitude $\propto q/\rho^2$, and its component perpendicular to the wall carries a further factor $a/\rho$ (similar triangles), so

$$
E_\perp \propto \frac{a}{\rho^3} = \frac{1}{a^2}\cdot\frac{1}{\left[1 + (vt/a)^2\right]^{3/2}}.
$$

The image charge density is proportional to $E_\perp$.

### Step 5: the width of the signal

$E_\perp$ drops to half its peak when $\left[1 + (vt/a)^2\right]^{3/2} = 2$, i.e. $(vt/a)^2 = 2^{2/3} - 1 = 0.587$ and $vt = 0.766\,a$. Adding the two halves, the full width at half maximum is

$$
\Delta t_{\text{FWHM}} = \frac{1.53\,a}{v} = \frac{1.53 \times 0.020\ \mathrm{m}}{1.708\times10^{7}\ \mathrm{m/s}} = 1.8\ \mathrm{ns}.
$$

The metal pipe screens the distant field, so the real signal is somewhat shorter, but of the same order: about 1–2 ns, far longer than the 77 ps bunch and shorter than the 6.15 ns bunch spacing. So a button can separate the bunches and measure their arrival times, but cannot see the shape of a bunch.

### Step 6: close to the speed of light

Close to the speed of light, the field of a charge is compressed along the direction of motion by a factor $\gamma$ (the relativistic factor, see [Kinematics](#kinematics-variables)), and the signal width becomes about $1.53\,a/(\gamma v)$. For the example protons $\gamma = 1.0016$, hardly any compression; for 1 GeV electrons $\gamma \approx 2000$, and in the same pipe the signal lasts only about 0.05 ps, so then the bunch length itself decides the shape of the signal. This is why electron accelerators can see the bunch shape directly with electrodes or RF deflecting cavities, while low-energy proton accelerators need bunch shape monitors.

### For readers who know special relativity and electrodynamics

The exact velocity relation is $\mathrm d\beta/\beta = \mathrm dW/W \cdot 1/[\gamma(\gamma + 1)]$, which returns to the 1/2 of step 2 for $\gamma \to 1$; computed exactly, the example's time difference is also 19.1 ps. The transverse field of a moving point charge in free space is $E_\perp = q\gamma a/\{4\pi\varepsilon_0[a^2 + (\gamma vt)^2]^{3/2}\}$, i.e. step 4 compressed by $\gamma$. In a grounded round pipe the electrostatic potential decays along $z$ as $\exp(-j_{01}|z|/a)$ ($j_{01} = 2.405$ is the first zero of $J_0$); solving in the beam's rest frame and transforming back to the laboratory gives a decay length $a/(j_{01}\gamma)$, so the width of the wall-current distribution is $\propto a/(\gamma\beta c)$. A button's output is roughly the time derivative of the wall current (a high-pass), and its spectrum rolls off above $f \gtrsim \gamma\beta c/(2\pi a)$; this roll-off changes with β and is the origin of the velocity dependence of low-energy BPM sensitivity.

## In AVAS {#diagnostics-in-avas}

AVAS is a simulation code; it simulates the beam itself. It does not simulate the signals of diagnostic devices (button voltages, transformer electronics, wire heating and secondary emission, loss-monitor response), nor the interception of the beam by wires, slits or screens. This chapter's `bpm` lab is the textbook model of a line charge in a round pipe with point-like buttons, not part of the AVAS engine.

### Simulation results are "ideal diagnostics"

The output of AVAS gives exactly the quantities diagnostic devices try to measure, with no noise and everywhere (see [Simulation](#simulation-output)):

- `DataSet.txt`, one line per step: the beam centre (an ideal BPM); rms sizes x, y, z (columns 16, 18, 20, in m: an ideal profile monitor); maxima (columns 22, 24, 26); emittances; the **mean** energy of all macro-particles (column 0, MeV; note that this is not the synchronous particle's energy); the number of surviving macro-particles (column 28; compared between two places, it is the transmission an ideal transformer would give).
- `synData.txt`: time, $\gamma\beta$, synchronous phase and RF phase of the synchronous particle at the entrance of each element (an ideal phase probe and time-of-flight measurement).
- `outputplane` saves the coordinates of all macro-particles at a given position in `outData_<position>.dst`, an ideal emittance meter: phase-space plots and moments of any order can be computed directly.

The simulated "readings" have a statistical error of their own: the rms and mean position computed from a few thousand macro-particles fluctuate (see [Simulation](#simulation-deep-statistics)). Fix the random seed `randomseed` when comparing two engine runs (with the original kernel or the compatibility mode a non-zero seed makes $x$ fully correlated with the phase in the initial distribution, which matters for rms readings, see [Simulation](#simulation-in-avas); the new kernel's default mode does not have this problem).

### Diagnostic positions in the lattice

```avas-params
diag_position diag_size diag_energy outputplane
```

- `diag_position 0 x y 0`, `diag_size 0 sx sy 0` and `diag_energy 0 W 0` are zero-length elements marking a diagnostic position and its target value (mm or MeV). They are not passed to the engine; in the correction of an error study, Python reads the beam parameters at that position from DataSet, compares them with the target and corrects with the knobs given by `adjust` (detailed rules in [Errors and correction](#errors-in-avas)).

### Monitors on the Research page

"Monitors and knobs", at the top of the Research page's "Response & controllability" tab, lists the monitors used: each has a name, a position z, what it reads ("position", i.e. a BPM, or "rms size", i.e. a profile monitor), its planes (x, y or xy) and a resolution (µm). They default to the lattice's `diag_position` (resolution 20 µm) and `diag_size` (50 µm) lines (`diag_energy` is not used as a monitor), can be added, removed and edited, and are saved in `<project>/Research/diagnostics.json`. The response matrix, the controllability analysis and the "Feedback" tab all use these monitors; the resolution sets the noise of the simulated readings and how much of it the correction amplifies onto the knobs. The monitors of the research layer measure only position and rms size; there are no phase, energy or current monitors.

### In Python

```text
import numpy as np
from avas.model import LinearModel
from avas.dynamics.diagnostics import Diagnostics, Monitor

model = LinearModel.from_project("my_project")
diag = Diagnostics.from_lattice(model)            # diag_position: 20 µm, diag_size: 50 µm
# or define your own: Diagnostics([Monitor("BPM1", 0.1, resolution=20e-6),
#                                  Monitor("WS1", 0.1, kind="size", resolution=50e-6)])
result = model.evaluate()
print(diag.labels(), diag.read(result))           # noiseless readings (m)
obs = diag.measure(result, rng=np.random.default_rng(1))   # with Gaussian noise of the resolution
```

`Diagnostics.measure` returns `Observation`s (name, position, quantity, plane, value, error); `save_observations` / `load_observations` read and write them. Simulation and measurement share the same `Observation`, so by design measured data only has to be turned into `Observation`s of monitors with the same names to go through the same analyses. For results of engine runs, `Diagnostics.read` interpolates in z between neighbouring rows (about one step apart).

## Further reading {#diagnostics-reading}

- P. Strehl, *Beam Instrumentation and Diagnostics*, Springer (2006): a systematic introduction to all kinds of diagnostic devices.
- P. Forck, *Lecture Notes on Beam Instrumentation and Diagnostics*, Joint University Accelerator School (JUAS): lecture notes for beginners, richly illustrated.
- D. Brandt (ed.), *CAS – CERN Accelerator School: Beam Diagnostics*, CERN-2009-005 (2009).
- M. G. Minty, F. Zimmermann, *Measurement and Control of Charged Particle Beams*, Springer (2003): methods for measuring emittance, energy and bunch length.
- A. V. Feschenko, "Methods and instrumentation for bunch shape measurements", Proc. PAC 2001: bunch shape monitors.
- Related chapters of this primer: [Why control](#control-intro), [Orbit correction](#orbit-correction), [Phase space](#phase-space), [Errors and correction](#errors); full bibliography in [References](#references).
