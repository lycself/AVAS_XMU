# RF cavity

The elements of the previous chapters (drift, quadrupole, solenoid) only change the **direction** in which a particle flies; they never change how fast it goes. The reason is simple: a magnetic force is always perpendicular to the velocity, so it bends the path but does no work. To make a particle faster you need an **electric field** that pushes it along its direction of motion. In a linac this job is done by **RF cavities**.

But there is a puzzle. At school you learned that a charge $q$ crossing a voltage $U$ gains the energy $qU$. So why not put two metal plates at high voltage in the beam pipe? The trouble is that after passing the high-voltage plate the particle must eventually return into the grounded pipe, and on the way back it gives the energy back, exactly. An RF cavity is also a metal box, open at both ends and grounded as a whole, yet a proton gains several hundred keV in each one. Where does the energy come from? Why does the same cavity treat fast and slow particles differently? And why does the timing that holds the bunch together also push the beam apart transversely? This chapter answers these questions.

First, have a look. In the animation below the bunch passes a 5 cm drift, then the example project's cavity hwr010 (0.21 m long), then a 0.2 m drift.

```avas-motion
cavity
```

Switch to 3D first. The see-through vertical cylinder is the cavity tank. A metal post across the beam (the inner conductor) carries a short tube in its middle (the drift tube), which splits the beam path into two **accelerating gaps**. The thin field lines and small cones in the gaps show the direction of the electric field at this moment. Notice that the arrows in the two gaps point opposite ways, and that they keep flipping, growing and shrinking while the bunch flies: this is "radio frequency", a field that oscillates 162.5 million times a second (162.5 MHz).

Then look at three things; the chapter explains each of them:

1. **Acceleration**: the value panel shows an "energy gain" of 0.357 MeV; the entrance energy is 1.527 MeV, the exit energy about 1.884 MeV.
2. **Holding the bunch together**: of the two blue particles, one starts 1.5 rms (about 2 mm) **ahead of** the bunch centre and one **behind** it. By the end the one in front has dropped behind and the one behind has moved in front. In 2D, switch the phase space to z: the ellipse of the bunch turns.
3. **Transverse spreading**: in the 2D top view the particles clearly spread out after the cavity. The rms $x$ at the end is 3.01 mm; drag Ke to 0 (cavity off) and it is only 2.09 mm.

All numbers in this chapter refer to the example project `examples/hwr010`: protons with injection kinetic energy $W = 1.527\ \mathrm{MeV}$ (5.7 % of the speed of light, $\beta = 0.05698$, $\beta\gamma = 0.05708$), RF frequency $f = 162.5\ \mathrm{MHz}$, RF wavelength $\lambda = c/f = 1.845\ \mathrm{m}$, distance a particle flies in one RF period $\beta\lambda = 0.1051\ \mathrm{m}$, current 0.184 mA. Unless stated otherwise the numbers come from AVAS's own linear model; the engine's results are the reference. The kinematics you need (what β, γ and βλ mean) is in [Kinematics](#kinematics).

## Why acceleration needs an alternating field {#rf-cavity-role}

### Why a static field does not work

The work a static electric field does on a charge moved from one point to another depends only on the **potential difference** between the points, not on the path, just as the work done by gravity depends only on the difference in height. The entrance and the exit of an RF cavity are both connected to the grounded beam pipe and are at the same potential. So if the cavity held only a static field, a particle would give back at the exit whatever it gained at the entrance, and the net work would be zero, like climbing a hill and coming back down to the same height.

Electrostatic accelerators do exist (a particle "rolls down" from a high-voltage terminal, for example), but a couple of tens of MV is the limit before sparks fly, and the voltage can be used only once. To bring protons to tens or hundreds of MeV the same field has to push them again and again.

### Let the field flip

Electromagnetic induction from school tells you that a changing magnetic field produces an **induced electric field** whose field lines are closed loops, and the work it does around a closed path is **not zero** (that is how a transformer works). So as soon as the field in the cavity changes in time, the argument "what you gain going in you give back coming out" no longer holds.

But change alone is not enough. A particle that stays in the cavity for many periods is pushed and pulled in turn and gains nothing on average. RF acceleration is about **timing**:

- when the particle crosses an accelerating gap, the field points forward and gives it a push;
- when the field points backwards, the particle has either not arrived yet, already left, or is hiding inside a drift tube. There is no electric field inside a metal conductor (electrostatic shielding from school), so the drift tube is the particle's shelter.

It is like pushing a swing: push each time the swing moves forward and, with little effort, it goes higher and higher; push at the wrong moment and you stop it.

### How far apart the gaps must be

A 162.5 MHz field changes direction every $1/(2f) = 3.08\ \mathrm{ns}$. A proton at 1.527 MeV moves at $1.71\times 10^7\ \mathrm{m/s}$ and covers 5.3 cm in 3.08 ns. So if the first gap gave it a push, after the field has flipped it should just be inside the next gap, whose field points the opposite way: the two gaps should be half a $\beta\lambda$ apart, about 5 cm. **The size of a cavity is set by the particle's speed and the frequency.** This is the most important sentence of the chapter, and it will come back again and again.

### Three things an RF cavity does to the beam

- **Acceleration**: the synchronous particle (the "standard" particle at the bunch centre, see below) gains $\Delta W = qV_0T\cos\varphi_s$ in each cavity. $V_0$ is the cavity voltage, $T$ measures how well the timing fits, and $\varphi_s$ tells where on the RF wave the particle arrives ([transit time](#rf-cavity-energy-gain)).
- **Longitudinal focusing**: put the bunch centre on the side where the field is **rising** in time; particles that arrive early gain less, late ones gain more, and the bunch does not fall apart ([phase](#rf-cavity-phase)).
- **Transverse defocusing**: longitudinal focusing necessarily comes with transverse defocusing. This is a property of the electric field itself, not a flaw of the cavity design ([transverse defocusing](#rf-cavity-transverse)).

At low energy (below about 20 % of the speed of light) superconducting proton linacs are usually built from periods of "one or a few cavities + one solenoid"; the solenoid compensates exactly the third point. [The tutorial](#tutorial) builds such a structure. Why a whole linac needs several kinds of cavities is explained in [From source to target](#journey-overview) and [The superconducting section](#journey-sc).

## Resonant cavities: why the field oscillates {#rf-cavity-modes}

### The picture of an LC circuit

Take the LC circuit from school physics. When the capacitor is fully charged all the energy sits in the electric field and the current is zero. The capacitor discharges, the current flows through the inductor and the energy moves into the magnetic field. A quarter period later the electric field is zero and the current is maximal; then the inductor charges the capacitor the other way round… The energy sloshes back and forth between electric and magnetic field at the frequency $f = 1/(2\pi\sqrt{LC})$.

A resonant cavity is the same thing "spread out": the metal faces on either side of the accelerating gap act as the capacitor, and the electric field is concentrated there; high-frequency currents flow on the surrounding metal walls, which act as the inductor, and the magnetic field circles around those currents. An important conclusion follows at once: **when the electric field is maximal the magnetic field is zero; the two are a quarter period ($90^\circ$) apart in time.** This is where AVAS's cos for the RF electric field and sin for the RF magnetic field come from (see [In AVAS](#rf-cavity-in-avas)).

A cavity is also like a bell: strike it once and it rings for a long time at its own frequency. The RF power source outside keeps "pushing" it at that frequency (resonance), and a small push builds up a strong field inside.

Where does this picture fail? In a real cavity the capacitance and inductance are spread over all of space, and a cavity has infinitely many ways to oscillate (**modes**, each with its own field shape and frequency); an LC circuit describes only one of them. The actual field shape requires solving Maxwell's equations, see [Going deeper: the fields in a resonator](#rf-cavity-deep-modes).

### Why not one big cylinder

The simplest accelerating cavity is a cylinder closed at both ends (a "pillbox"): the electric field points along the axis, strongest at the centre and zero at the wall. Its frequency is set by the radius alone; 162.5 MHz needs a radius of about 0.71 m, a cavity 1.4 m across. For a low-energy proton beam that needs a bore of a few centimetres, this is bulky and wasteful: most of the stored energy sits far away from the beam.

### Coaxial resonators: QWR and HWR

Low-frequency, low-velocity sections therefore use **coaxial resonators**: an outer metal tank with a metal post inside (the inner conductor). Electromagnetic waves run back and forth along the inner conductor and form a standing wave, like the standing wave on a string fixed at both ends that you met at school.

- **Half-wave resonator (HWR)**: both ends of the inner conductor are connected to the tank (shorted), like a string fixed at both ends, and its length is about half a wavelength ($\lambda/2 \approx 0.92\ \mathrm{m}$ at 162.5 MHz). The middle of the string has the largest amplitude: in the middle of the inner conductor the voltage is maximal and the current minimal. The beam crosses there, so near the beam the electric field is strong and the magnetic field weak. An HWR is symmetric top to bottom about the beam axis and has none of the vertical steering of the QWR below, but its field in the cross-section is not fully axially symmetric and contains a quadrupole component.
- **Quarter-wave resonator (QWR)**: one end of the inner conductor is connected to the tank and the other is free (open), like a string fixed at one end only, about $\lambda/4$ long. The beam crosses the drift tube near the free end, forming two accelerating gaps. The structure is not symmetric top to bottom; its field contains a dipole component that steers the beam vertically by an amount that depends on the phase.
- **Multi-gap structures**: spoke cavities, elliptical cavities and others have three or more gaps. For AVAS they are all just "a field map"; the physical differences are entirely in the field distribution.

In a two-gap cavity the fields in the two gaps point opposite ways at any moment (in the animation the two sets of arrows are always opposite). If the particle takes exactly half an RF period to go from the centre of the first gap to the centre of the second, the field has just turned over and both gaps accelerate it. This requires a gap distance $d = \beta_g\lambda/2$. $\beta_g$ is called the cavity's **geometric beta**: the cavity is "tailored" for that speed.

### The example cavity hwr010

The on-axis field $E_z$ of the example map has a peak at $z \approx 0.074\ \mathrm{m}$ and one at $z \approx 0.140\ \mathrm{m}$, of opposite sign, crossing zero at $z \approx 0.104\ \mathrm{m}$ (cavity centre 0.105 m): a typical two-gap HWR. The field-weighted centres of the two peaks are $d = 0.081\ \mathrm{m}$ apart, and $d = \beta_g\lambda/2$ gives $\beta_g \approx 0.088$, protons of about 3.7 MeV.

The example beam has $\beta = 0.057$, much slower than 0.088. It needs 4.74 ns to go from the first gap to the second, but the field flips after 3.08 ns: by the time it reaches the second gap the field has turned about $97^\circ$ further than the ideal timing. So this cavity gives the example beam a large discount on the energy; the next section works it out.

In the viewer below, look at the longitudinal slice of `edz` to see the two gaps of opposite sign directly; then look at `bdx` and `bdy`: near the beam hole the RF magnetic field is only of order millitesla (the largest value in the whole map is about 3.4 mT, while the electric field reaches about 17 MV/m, 9.6 MV/m on axis). This is "maximal voltage, minimal current in the middle of an HWR" in numbers. Note also that `bdx` is clearly stronger than `bdy` (maxima of about 3.4 and 0.65 mT): the RF magnetic field of an HWR is far from axially symmetric; the [Field maps](#field-maps-interpolation) chapter compares them quantitatively.

```avas-field
map hwr010
```

Below is the same cavity in a beam line (synchronous phase $-30^\circ$, Ke = 1.36). Turn on the energy curve in the 2D view: the kinetic energy rises from 1.527 MeV to 1.884 MeV. Then look at rms $x$: drifting to the end after the cavity it reaches 3.01 mm, whereas with the cavity replaced by a drift of the same length it is only 2.09 mm.

```avas-beamline
drift 0.05 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010
drift 0.2 0.02 0
```

## Transit time: why a cavity picks its velocity {#rf-cavity-energy-gain}

### The ideal case: the voltage V₀

Start with an ideal case: if the electric field were frozen at its maximum and the particle crossed instantly, the energy it gains would be "field × length" added up along the axis, times the charge, just like $qU$ at school. Adding up the contributions of all gaps as positive numbers gives a voltage called $V_0$. For the example cavity at Ke = 1.36, $V_0 \approx 0.978\ \mathrm{MV}$: a proton can gain at most about 0.98 MeV.

### The real case: a discount for timing, T

A real particle needs time to cross the cavity (12.3 ns in the example, two RF periods), and the field changes all the while. The actual energy gain is therefore written

$$
\Delta W = qV_0\,T\cos\varphi_s .
$$

The two new quantities mean:

- The **transit-time factor** $T$ (between 0 and 1) is the discount for timing. It has two sources. First, during the few nanoseconds the particle needs to cross **one** gap the field changes, so the average is less than the peak; the shorter the gap and the faster the particle, the smaller this discount. Second, the particle does not necessarily arrive at the **second** gap exactly half a period later; at the geometric beta there is no discount from this part.
- The **synchronous phase** $\varphi_s$ says where on the RF wave, before or after the crest, the particle arrives. $\varphi_s = 0$ is exactly on the crest (gaining $qV_0T$); $\varphi_s = -30^\circ$ gains only $\cos 30^\circ = 0.87$ times as much. The next section explains why one deliberately stays off the crest.

For a two-gap cavity $T$ can be written as the product of two discounts:

$$
T(\beta) = T_g(\beta)\cdot\left\lvert\sin\frac{\pi d}{\beta\lambda}\right\rvert ,
$$

$T_g$ is the discount of a single gap, and the second factor says how well the two gaps keep step: for $d = \beta\lambda/2$ the sine is $\sin 90^\circ = 1$, which is what geometric beta means. For the example beam $\pi d/(\beta\lambda) = 139^\circ$ and $\sin 139^\circ = 0.66$: the step between the two gaps alone costs a third. Where these formulas come from is shown in [Going deeper: the transit-time factor](#rf-cavity-deep-transit), using nothing but adding arrows and the geometry of a circle.

### One cavity type serves only a range of velocities

$T(\beta)$ computed from the real on-axis field of the example map:

| β | 0.05 | 0.057 (injection) | 0.07 | 0.08 | 0.10 | 0.1035 (max.) | 0.12 | 0.15 | 0.20 |
|---|---|---|---|---|---|---|---|---|---|
| $T$ | 0.21 | 0.40 | 0.65 | 0.75 | 0.82 | 0.82 | 0.80 | 0.73 | 0.61 |

$T$ falls quickly on the slow side and more slowly on the fast side: $T$ stays above 90 % of its maximum for $0.079 \le \beta \le 0.147$ (proton kinetic energies of about 2.9–10.4 MeV). **One cavity type can serve only a range of velocities efficiently**, which is why a whole linac changes cavity types several times ([From source to target](#journey-overview)). For the example beam $T = 0.40$, so $qV_0T \approx 0.39\ \mathrm{MeV}$, only four tenths of $V_0$.

### At low energy the particle speeds up noticeably inside the cavity

The $T(\beta)$ above assumes that the particle's speed stays constant in the cavity. But the example beam goes from 1.527 MeV to about 1.88 MeV inside the cavity, a 10 % change in speed: the first gap gives it a push, so at the second gap its speed is closer to the geometric beta and the timing there actually fits better. Calculated step by step along the real trajectory, the largest gain is 0.482 MeV, a quarter more than the 0.39 MeV estimated from $T(0.057)$. This also explains why AVAS does not replace a cavity by a "$qV_0T\cos\varphi$" formula but pushes the particles step by step through the three-dimensional field map (see [Field maps](#field-maps)): at low energy gain, phase and focusing are all entangled with the trajectory itself. The detailed tables are in [Going deeper: the transit-time factor](#rf-cavity-deep-transit).

The other way round, the formula is accurate when the speed is close to the geometric beta. The lab below sets the injection energy to 4.727 MeV ($\beta = 0.100$): the phase scan is nearly symmetric and the "$\Delta W/\Delta W_{max}$" value is 0.866, exactly $\cos 30^\circ$. Drag W back to 1.527 MeV: the ratio drops to 0.74 and the curve becomes lopsided.

```avas-lab
rf-cavity phase=-30 Ke=1.36 W=4.727
```

## Phase: acceleration against bunching {#rf-cavity-phase}

### Why the synchronous phase is negative

The particles of a bunch do not arrive together: some are ahead, some behind. We pick the particle at the bunch centre as the "standard", call it the **synchronous particle**, and compare all others with it.

Imagine the electric field rising and falling in time along a cosine curve. In AVAS $\varphi = 0$ is the crest; $\varphi < 0$ is before the crest, while the field is still **rising**; $\varphi > 0$ is after the crest, while it falls. Now put the synchronous particle on the rising side, say $\varphi_s = -30^\circ$:

- a particle arriving **earlier** than the synchronous one catches an earlier, weaker field and gains **less** energy;
- a particle arriving **later** catches a stronger field and gains **more**.

At low energy, more energy means faster. So the early ones (in front) slow down and are caught up; the late ones (behind) speed up and catch up. The particles oscillate back and forth about the synchronous particle and the bunch does not fall apart. This is the **principle of phase stability**, like an invisible spring pulling all particles towards the bunch centre. Details are in [Longitudinal motion](#longitudinal).

If the synchronous particle sat on the crest ($\varphi_s = 0$), the field nearby would be almost equally strong, early and late particles would gain about the same, and there would be no "spring"; on the falling side ($\varphi_s > 0$) early particles gain more, those in front run further ahead, and the bunch is stretched.

### Going through it with numbers

Take the blue particle in front in the animation and use school physics only:

1. It is 1.97 mm ahead of the bunch centre and flies at $1.71\times 10^7\ \mathrm{m/s}$, so it arrives $1.97\ \mathrm{mm} \div 1.71\times 10^7\ \mathrm{m/s} = 0.116\ \mathrm{ns}$ early.
2. The RF turns $360^\circ$ in one 6.15 ns period, so 0.116 ns is $6.8^\circ$: it sees the phase $-36.8^\circ$ instead of $-30^\circ$.
3. How much less energy? Near $-30^\circ$ the cosine curve changes by about $\sin 30^\circ \times \pi/180$ per degree; times $6.8^\circ$ and times this cavity's $qV_0T \approx 0.41\ \mathrm{MeV}$, that is about 0.024 MeV less.
4. As a relative change in momentum: at low speed $W = p^2/(2m)$, so the relative change in energy is twice the relative change in momentum, and $\Delta p/p = 0.024/(2\times 1.527) \approx -7.9\,‰$. The animation's model gives $-8.0\,‰$.
5. Its speed is also lower by about 8 ‰. In the 0.2 m drift after the cavity it falls back by $0.008 \times 0.2\ \mathrm{m} = 1.6\ \mathrm{mm}$ relative to the bunch centre. At the cavity exit it has already fallen back to 0.65 mm ahead of the centre; another 1.6 mm takes it to about 0.95 mm **behind** the centre. At the end of the animation it is at $-0.94\ \mathrm{mm}$.

The blue particle behind does the opposite: it gains about 0.024 MeV more and moves from behind to the front. That is the "swap" you saw at the start.

### Changing the phase in the animation

The animation below sets the synchronous phase to $+30^\circ$, after the crest:

```avas-motion
cavity phase=30
```

Watch the two blue particles: the one in front gains extra energy ($+6.0\,‰$), runs further and further ahead and ends 3.4 mm ahead of the centre; the one behind falls further behind. The bunch length (rms $z$) grows from 1.32 mm to 3.07 mm. Now look at the top view: this time the beam is held tight transversely, rms $x$ at the end is only 1.36 mm.

Then drag `phase` to 0, −30, −60 and −90 in turn and read the energy gain and the exit rms $z$ in the value panel. The panel has no rms $x$; the table gives the numbers of the same model, and turning on the rms envelope in the top view lets you compare the beam size:

| $\varphi_s$ | energy gain / MeV | rms $z$ at the end / mm | rms $x$ at the end / mm |
|---|---|---|---|
| $-90^\circ$ | 0.000 | 1.21 | 3.83 |
| $-60^\circ$ | 0.167 | 1.33 | 3.64 |
| $-30^\circ$ | 0.357 | 1.17 | 3.01 |
| $0^\circ$ | 0.475 | 1.56 | 2.15 |
| $+30^\circ$ | 0.437 | 3.07 | 1.36 |
| cavity off (Ke = 0) | 0 | 2.06 | 2.09 |

The pattern is clear:

- **The more negative $\varphi_s$, the tighter the bunch longitudinally and the wider the beam transversely**; for $\varphi_s > 0$ the beam is held together transversely and spreads longitudinally. The next section explains why longitudinal and transverse always go opposite ways.
- **At $-60^\circ$ the bunch at the end is longer than at $-30^\circ$**: it is squeezed so hard that it reaches its shortest length (0.62 mm) already at $z \approx 0.29\ \mathrm{m}$; after that front and back swap and it spreads again. At $-30^\circ$ the shortest point is at $z \approx 0.30\ \mathrm{m}$, 0.75 mm.
- **$-90^\circ$ does not accelerate at all, yet it bunches**: a cavity used only to compress bunches is called a **buncher** and works at this phase (see [The medium-energy transport](#journey-mebt-buncher)).
- **$0^\circ$ gives the most energy but still bunches a little**: at this low speed the real "crest" of this cavity is at $\varphi_s \approx +8^\circ$, so $0^\circ$ is still on the rising side (the reason is in [Going deeper: the transit-time factor](#rf-cavity-deep-transit)).

The price is less acceleration: the synchronous particle gains only $\cos\varphi_s$ times the peak energy, while the range of phases the "spring" can hold (the stable region, about $3\lvert\varphi_s\rvert$ wide) requires $\lvert\varphi_s\rvert$ not to be too small. At low energy the compromise is usually between $-40^\circ$ and $-20^\circ$; the example project uses $-33^\circ$, the animation $-30^\circ$ by default. Space charge matters little at the example current of 0.184 mA: setting the current to 0 changes rms $z$ at the end only from 1.174 mm to 1.176 mm.

### Where is the crest: phase scans

In a real machine nobody knows which reading of the signal source corresponds to "$\varphi_s = -30^\circ$"; it is found by a **phase scan**: the cavity phase is stepped, the exit energy is measured from the time of flight between downstream beam position monitors, giving a curve of exit energy against phase; the phase of maximum energy (the "crest") is found and the design $\varphi_s$ is set as an offset from it.

The lab below is such a phase scan. The horizontal axis is the RF phase $\varphi_0$ when the particle reaches the cavity entrance, and the marker is the current setting. At $W = 1.527\ \mathrm{MeV}$ and $\varphi_s = -30^\circ$ the lab gives $\Delta W = 0.357\ \mathrm{MeV}$, only 74 % of the scan maximum of 0.482 MeV, while $\cos 30^\circ = 0.866$; the curve is also asymmetric (minimum near $\varphi_0 \approx -106^\circ$ with $\Delta W \approx -0.31\ \mathrm{MeV}$). Drag `phase` from $-90^\circ$ to $0^\circ$ and watch exit energy, rms $z$ and rms $x/y$ trade off; set W to 4.727 MeV and the ratio returns to 0.866; then lower Ke from 1.36 to 0.7 and see where the synchronous point moves on the curve.

```avas-lab
rf-cavity phase=-30 Ke=1.36 W=1.527
```

### When the particle is too slow the phase may not exist

Take care when lowering W in the animation (or the lab). Far below the geometric beta the two gaps almost cancel each other, the synchronous phase as AVAS defines it can reach only part of the angles, and some $\varphi_s$ have no solution at all. Drag W in the animation to 0.7 MeV: a warning appears, the "synchronous phase" becomes $150^\circ$ and the energy gain $-0.113\ \mathrm{MeV}$: the cavity decelerates, and the engine does the same. Drag it to 0.9 MeV: neither $-30^\circ$ nor $150^\circ$ can be reached, and the engine would refuse to run. Which energies behave like this, and why, is in [Going deeper: phase stability and the synchronous-phase solution](#rf-cavity-deep-phase) and [In AVAS](#rf-cavity-in-avas).

## Transverse defocusing: the price of bunching {#rf-cavity-transverse}

### The field lines in a gap

In an accelerating gap the field lines leave one metal face and end on the opposite one. Because there is a hole in the middle for the beam, the field lines bulge outwards near the hole: on the **entrance** side of the gap they converge towards the axis, on the **exit** side they spread out. In [the hwr010 viewer above](#rf-cavity-modes), look at the $x$–$y$ cross-sections of `edx` and `edy` and move the section from the gap entrance to its exit: the transverse field reverses.

So an off-axis particle crossing a gap is pushed towards the axis in the first half (focusing) and away from it in the second half (defocusing). If the field did not change in time and the speed stayed constant, the two halves would cancel exactly: this is the transverse version of "a static field gives no net effect" from the beginning.

But for $\varphi_s < 0$ the field **grows** while the particle crosses the gap: the defocusing second half is stronger than the focusing first half and the net effect is **defocusing**. For $\varphi_s > 0$ the field weakens and the net effect is focusing. This agrees with the table of the previous section.

### Why it cannot be avoided

Could one design a cavity that bunches longitudinally and also focuses transversely? No. The reason is the same as "no magnet can focus both planes at once" in the [Quadrupole](#quadrupole-field) chapter.

Fly along with the bunch and look at the field around it. The RF wavelength of 1.85 m is much longer than the bunch (rms length about 1 mm), so in this frame the field changes very slowly and is almost electrostatic, and there are no other charges near the bunch. Field lines do not appear or vanish out of nothing, so the three "spring constants" must add up to zero:

$$
K_x + K_y + K_z = 0 .
$$

Longitudinal bunching needs $K_z > 0$, so the two transverse directions together must be negative (defocusing). For an axially symmetric gap each transverse direction gets half the longitudinal strength with the opposite sign. **A phase that is longitudinally stable is necessarily transversely defocusing**, and vice versa. This holds whatever the shape of the cavity; space charge and magnetic fields are not part of the argument. The step-by-step proof is in [Going deeper: RF defocusing in numbers](#rf-cavity-deep-transverse).

### How strong

At $\varphi_s = -33^\circ$ the example cavity acts like a defocusing lens with a focal length of about 0.38 m, comparable to a weak solenoid, so it must be included in the transverse design. At this low speed most of the defocusing comes from the **first** gap: it sits far from its own crest (the field is rising steeply), barely accelerates, and defocuses strongly; the second gap is close to its crest and actually focuses a little. The numbers are in [Going deeper: RF defocusing in numbers](#rf-cavity-deep-transverse).

The strength of RF defocusing is proportional to $\sin(-\varphi_s)/(\beta\gamma)^3$: the more negative the synchronous phase and the slower the particle, the stronger the defocusing. $1/(\beta\gamma)^3$ is 5378 at 1.527 MeV but only 319 at 10 MeV. That is why at low energy every cavity must be followed closely by a focusing element ([The superconducting section](#journey-sc), [the tutorial](#tutorial)).

Acceleration also has a transverse benefit: the particle's transverse momentum stays the same while its longitudinal momentum grows, so the angle $x'$ shrinks. This is called **adiabatic damping**. The example cavity shrinks the angles by about 10 %.

## Power, quality factor and superconductivity {#rf-cavity-figures}

Back to the LC circuit and the bell: a real metal wall has resistance, a little energy turns into heat in the wall every period, and the ringing slowly fades. These quantities decide "how much power a cavity needs, how much voltage it can give, where the field limit lies"; they belong to cavity design. AVAS's beam-dynamics calculation does not use them directly, but you should keep them in mind when choosing Ke.

The **quality factor** $Q_0$ says how long the bell rings: the energy $U$ stored in the cavity divided by the energy lost in the walls per radian of oscillation ($1/2\pi$ of a period). Room-temperature copper cavities have about $10^4$, superconducting niobium cavities $10^9$–$10^{10}$: "strike" a superconducting cavity and it rings millions of times longer.

| Quantity | Definition | Meaning and typical size |
|---|---|---|
| Intrinsic quality factor $Q_0$ | $Q_0 = \omega U / P_c$ | Stored energy $U$ over wall loss per radian ($P_c$ is the wall-loss power, $\omega = 2\pi f$); about $10^4$ for room-temperature copper, $10^9$–$10^{10}$ for superconducting niobium |
| Shunt impedance $R_{sh}$ | $R_{sh} = V^2 / P_c$ | Voltage obtained per unit wall-loss power; the usual linac convention, the circuit convention differs by a factor 2 |
| $R/Q$ | $R/Q = V^2/(\omega U)$ | Depends only on the geometry, not on the material; tens to hundreds of Ω for low-β superconducting cavities |
| Accelerating gradient $E_{acc}$ | $E_{acc} = V_{acc}/L_{eff}$ | Voltage per metre; superconducting cavities often use the effective length $L_{eff} = N_{gap}\,\beta_g\lambda/2$ ($N_{gap}$ is the number of gaps) |
| $E_{pk}/E_{acc}$, $B_{pk}/E_{acc}$ | Peak surface fields over gradient | Typically about 4–6 and 5–10 mT/(MV/m) for low-β cavities; too strong a surface electric field emits electrons (field emission), too strong a surface magnetic field destroys superconductivity (quench) |

Here $V$ usually means the effective voltage including the transit-time factor, $V_0T$. From the definitions the wall loss is $P_c = V^2/[(R/Q)\,Q_0]$: $R/Q$ is fixed by the shape, so the loss depends only on $Q_0$. An order of magnitude (assuming $R/Q = 100\ \Omega$, typical for low-β cavities, not the design value of hwr010): at $V = 0.4\ \mathrm{MV}$ a superconducting cavity ($Q_0 = 10^9$) dissipates about 1.6 W, a copper cavity of the same shape ($Q_0 \sim 10^4$) about 160 kW. The superconducting cavity's 1.6 W must be removed at liquid-helium temperature, and each watt of heat load costs hundreds of watts of refrigerator power (typical value), but that is still far below the copper cavity. This is the main reason continuous-wave (CW) proton linacs choose superconductivity.

The power the beam takes from the cavity is the school formula $P = UI$: $P_b = I\,V_0T\cos\varphi_s$. With $I = 0.184\ \mathrm{mA}$ and $\Delta W = 0.34\ \mathrm{MeV}$ at $\varphi_s = -33^\circ$, $P_b \approx 63\ \mathrm{W}$.

## Think about it {#rf-cavity-think}

> **Think about it 1**: acting on a proton with $\beta = 0.057$, which matters more: an RF magnetic field of 1 mT in the map or a transverse electric field of 2 MV/m?
>
> Answer: the ratio of magnetic to electric force is about $\beta c B/E = 0.057\times 3\times 10^8\times 10^{-3}/(2\times 10^6) \approx 1\,\%$, and the two are $90^\circ$ apart in time. A slow proton feels almost only the electric field; switching Kb between 0 and $\pm 1.36$ in the linear model changes the exit rms sizes by less than 0.4 %. The faster the particle, the more the magnetic term matters.

> **Think about it 2**: the gap distance gives a geometric beta of 0.088. Why is the maximum of $T(\beta)$ at $\beta \approx 0.104$?
>
> Answer: $T = T_g\cdot\lvert\sin(\pi d/\beta\lambda)\rvert$ is a product of two factors. The second is largest at $\beta = 0.088$, but the single-gap factor $T_g$ rises steadily with β (the faster the particle, the less the field changes while it crosses a gap) and pushes the maximum of the product to higher β. So "geometric beta" and "the beta of maximum $T$" are different numbers; the beta in a cavity type's name usually means the latter or a design value, so say which one you mean.

> **Think about it 3**: an operator says "this cavity is set $-30^\circ$ from crest". Is that the same as writing V3 = 0, `phase` = −30 in AVAS?
>
> Answer: only for $\beta \approx \beta_g$. At low energy AVAS's $\varphi_s = 0$ is not the phase of maximum energy: in the example, $\varphi_s = 0$ corresponds to an entrance phase of $95.4^\circ$, about $10^\circ$ earlier than the entrance phase of maximum energy. "$-30^\circ$ from crest" in the example corresponds to an entrance phase of $75.5^\circ$, not to AVAS's $\varphi_s = -30^\circ$ (entrance phase $56.6^\circ$). To reproduce a machine setting, writing the entrance phase with V3 = 1 or V3 = 2 is more direct.

> **Think about it 4**: since $\varphi_s > 0$ focuses transversely, could some cavities run at positive phase and replace the solenoids?
>
> Answer: a cavity with $\varphi_s > 0$ debunches longitudinally (in the animation at $+30^\circ$ the bunch grows from 1.32 mm to 3.07 mm). "The spring constants add up to zero" only allows focusing to be "moved" to another direction, never all three directions focused at once. Alternating positive and negative phases can indeed give net three-dimensional stability, as FODO does (alternating-phase focusing, APF), but it costs acceleration and acceptance and has only been used in a few compact low-energy structures; proton linacs usually still use magnets for transverse focusing.

> **Think about it 5**: at $\varphi_s = -90^\circ$ the animation shows zero energy gain. Why is the bunch still compressed?
>
> Answer: the synchronous particle meets the field exactly at its zero crossing and gains nothing itself; but early particles meet a negative field (decelerated) and late ones a positive field (accelerated), so the "spring" is strongest here. This is how a buncher works.

## Common misconceptions {#rf-cavity-misconceptions}

- **"$\varphi_s = -30^\circ$ means $30^\circ$ from crest"**: true only for $\beta \approx \beta_g$. At the example's injection $\beta = 0.057$, AVAS's $\varphi_s = -30^\circ$ gains only 74 % of the maximum and the crest is at $\varphi_s \approx +8^\circ$ ([phase scan](#rf-cavity-phase)).
- **"$V_0T$ is a constant of the cavity"**: $T$ depends on β, and at low energy $V_0T$ also depends on phase (0.405–0.475 MV, [transit time](#rf-cavity-energy-gain)). A cavity type suits only a range of β.
- **"Geometric beta is the beta of maximum $T$"**: in the example the gap distance gives 0.088, the maximum of $T$ is at 0.104.
- **Mixing phase conventions**: AVAS uses $\cos(\omega t+\varphi)$, so bunching phases are negative; some books and codes use a sine convention or the zero crossing as reference, with different signs and values for the stable phase. When importing phases from another code (such as TraceWin), check its phase definition first.
- **"A cavity only acts longitudinally"**: the RF defocusing of the example cavity has a focal length of about 0.4 m, comparable to a weak solenoid, and must be included in the transverse design.
- **"Acceleration reduces the emittance"**: adiabatic damping reduces the geometric emittance (about 10 % for the example cavity); the normalised emittance stays the same.
- **"The energy gain is strictly proportional to Ke"**: at low energy the trajectory changes with the field level; $\Delta W/K_e$ is 0.245, 0.248, 0.250 and 0.251 MeV for Ke = 0.5, 1.0, 1.36 and 2.0, nearly but not strictly proportional.
- **"The magnetic field is small, so its setting does not matter"**: at low β the effect of Kb is indeed small, but the electric and magnetic fields are two parts of the same mode of oscillation, so Kb should scale with Ke; at high β an inconsistent Kb produces spurious transverse effects.
- **"The energy in the results is the synchronous particle's energy"**: column 0 of DataSet.txt is the **mean** energy of all macro-particles; off-axis particles gain a different energy in the three-dimensional map, and the mean can be a few per cent above the synchronous particle (see [In AVAS](#rf-cavity-in-avas)).
- **"Any synchronous phase can be set"**: at low energy some $\varphi_s$ have no solution, and the engine then takes the decelerating solution or refuses to run ([Going deeper: phase stability and the synchronous-phase solution](#rf-cavity-deep-phase)).

## Going deeper: the fields in a resonator {#rf-cavity-deep-modes}

The main text described the field in a cavity with an LC circuit and a standing wave on a string. This section makes that more precise step by step: first the frequency, then what the electric field in a pillbox looks like, and finally why the magnetic field "lags" a quarter period behind the electric field. The first four steps use only school physics; the last paragraph is for readers who know vector calculus.

### Step 1: the frequency of an LC circuit

The larger the capacitance $C$, the more charge is needed to fill it; the larger the inductance $L$, the harder it is to change the current. Both slow the oscillation down, and the frequency is

$$
f = \frac{1}{2\pi\sqrt{LC}} .
$$

To lower the frequency you need more capacitance or inductance, i.e. a bigger cavity. This is the basic reason why low-frequency cavities are large.

### Step 2: standing waves on a coaxial line

In a coaxial resonator electromagnetic waves run back and forth along the inner conductor at the speed of light, reflect at the ends and superpose into a standing wave. As on a string:

- at a **shorted end** (inner conductor connected to the tank) the voltage must be zero, a **node** of the voltage, like the fixed end of a string;
- at an **open end** (inner conductor free) the current must be zero and the voltage is maximal, an **antinode** of the voltage, like the free end of a string.

With both ends shorted the lowest standing wave is half a wavelength: the inner conductor of an HWR is about $\lambda/2 = 1.845/2 \approx 0.92\ \mathrm{m}$ long, with the voltage antinode in the middle, where the beam crosses. With one end shorted and one open it is a quarter wavelength: a QWR is about $\lambda/4 \approx 0.46\ \mathrm{m}$ long, with the voltage antinode at the free end, where the beam crosses. Real cavities are somewhat shorter because of the capacitance near the drift tube.

### Step 3: the pillbox cavity

The lowest accelerating mode of a closed cylinder (radius $R$, length $d$) is called TM010. Its electric field has only a component $E_z$ along the axis, strongest on the axis and decreasing outwards to zero at the wall (there can be no electric field parallel to a metal surface), like a vibrating drum skin: largest amplitude at the centre, fixed at the rim. The function describing this shape is the Bessel function of order zero, $J_0$, whose first zero is at 2.405, so

$$
E_z(r,t) = E_0\,J_0\!\left(\frac{2.405\,r}{R}\right)\cos\omega t .
$$

The resonant frequency

$$
f_{010} = \frac{2.405\,c}{2\pi R}
$$

depends only on the radius, not on the length: the frequency is set by the transverse size. For $f = 162.5\ \mathrm{MHz}$ this gives $R \approx 0.706\ \mathrm{m}$, the cavity about 1.4 m across of the main text.

### Step 4: why the magnetic field lags a quarter period

Faraday's law of induction from school says: when the magnetic flux through a loop changes, an induced electric field appears in the loop. Maxwell added the symmetric other half: **when the flux of the electric field changes, a magnetic field appears around it**. In a pillbox take a circle of radius $r$ centred on the axis. The electric flux through it varies as $\cos\omega t$, and the magnetic field $B_\theta$ around it is proportional to **how fast** that flux changes.

$\cos\omega t$ changes slowest at its maximum (rate of change zero) and fastest at its zero crossing. So when the electric field is maximal the magnetic field is zero, when the electric field is zero the magnetic field is maximal, and the magnetic field varies as $\sin\omega t$: the $90^\circ$ of the LC picture. **This is the physical origin of AVAS's cos for the RF electric field and sin for the RF magnetic field.** The example map stores its magnetic field with the same minus sign as the pillbox $B_\theta$ here, which is why the example uses $K_b = -K_e$ (see [In AVAS](#rf-cavity-in-avas)).

### For readers who know vector calculus

Solving the wave equation in cylindrical coordinates with zero tangential electric field at the conducting wall, the TM010 mode has only the components $E_z$ and $B_\theta$:

$$
E_z(r,t) = E_0\,J_0\!\left(\frac{2.405\,r}{R}\right)\cos\omega t,\qquad
B_\theta(r,t) = -\frac{E_0}{c}\,J_1\!\left(\frac{2.405\,r}{R}\right)\sin\omega t .
$$

The magnetic field follows directly from Faraday's law $\nabla\times\boldsymbol{E} = -\partial\boldsymbol{B}/\partial t$: $(\nabla\times\boldsymbol{E})_\theta = -\partial E_z/\partial r = E_0 k J_1(kr)\cos\omega t$ (with $k = 2.405/R$ and $J_0' = -J_1$); integrating over time gives $B_\theta$ varying as $\sin\omega t$. The frequency follows from $k = \omega/c$. In a closed cavity $\oint\boldsymbol{E}\cdot\dd\boldsymbol{l} = -\dd\Phi_B/\dd t \neq 0$, which is the rigorous form of "the electrostatic argument no longer holds" in the main text.

## Going deeper: the transit-time factor {#rf-cavity-deep-transit}

The main text gave $\Delta W = qV_0T\cos\varphi_s$ and the $T$ of a two-gap cavity. This section derives them step by step by "adding arrows" and shows exactly how AVAS's synchronous phase is defined. The first five steps use only school trigonometry and the geometry of a circle; the last paragraph is for readers who know Fourier transforms.

### Step 1: slice the cavity

Cut the cavity along the axis into many thin slices of thickness $\Delta z$. In slice $i$ the particle feels the force $qK_eE_z(z_i)\cos\varphi_i$, and the work done is force times distance, so

$$
\Delta W = q\sum_i K_eE_z(z_i)\,\Delta z\,\cos\varphi_i .
$$

$\varphi_i$ is the RF phase when the particle reaches slice $i$. If the particle reaches the cavity entrance at $t_{in}$ and position $z$ at time $t(z)$, then

$$
\varphi(z) = \omega\,\bigl(t(z) - t_{in}\bigr) + \varphi_0 ,
$$

where $\varphi_0$ is the RF phase when the particle reaches the entrance and $\omega = 2\pi f$. This sum (an integral when the slices become infinitely thin) is exact for the reference particle on axis, but it mixes "field" and "timing". Next we separate them.

### Step 2: draw each slice as an arrow

From school: an arrow of length $a$ at angle $\varphi$ to the horizontal has the horizontal projection $a\cos\varphi$. So the contribution of slice $i$ can be drawn as an arrow of length $K_eE_z(z_i)\Delta z$ and direction $\varphi_i$; its contribution to the energy is its horizontal projection.

Put all arrows head to tail and draw a **resultant arrow** from the start of the first to the end of the last. The horizontal projections of the arrows add up to the horizontal projection of the resultant, so

$$
\Delta W = q\,(\text{length of resultant})\cos(\text{direction of resultant}) .
$$

Call the horizontal component of the resultant $C = \int E_z\cos\varphi\,\dd z$ and its vertical component $S = \int E_z\sin\varphi\,\dd z$; then

$$
\varphi_s = \operatorname{atan2}(S,\,C),\qquad V_0T = K_e\sqrt{C^2+S^2},\qquad \Delta W = qV_0T\cos\varphi_s .
$$

$\operatorname{atan2}(S, C)$ is simply "the direction of the arrow with vertical component $S$ and horizontal component $C$" (taking the quadrant into account). **This is exactly the synchronous phase of V3 = 0 in AVAS**: by how many degrees the resultant deviates from the horizontal. The length of the resultant, $V_0T$, is the voltage the particle can gain with the best timing.

### Step 3: one uniform gap, arrows along an arc

Take a gap of length $g$ with a uniform field, and a particle of constant speed. The RF turns once for every $\beta\lambda$ the particle travels, so the phase grows uniformly: $\varphi(z) = 2\pi z/(\beta\lambda) + \varphi_0$.

Then all arrows have the same length, and each is turned by the same small angle relative to the one before: head to tail they form an **arc of a circle**.

- arc length = sum of the arrow lengths = $V_0$ (the voltage for a frozen field and instant crossing);
- the angle the arc subtends at the centre = the angle the RF turns while the particle crosses the gap, $\Theta = 2\pi g/(\beta\lambda)$;
- the radius of the circle is $\rho = V_0/\Theta$ (arc length = radius × angle);
- the resultant is the **chord** of the arc, of length $2\rho\sin(\Theta/2)$.

So

$$
T_g = \frac{\text{chord}}{\text{arc}} = \frac{\sin(\Theta/2)}{\Theta/2} = \frac{\sin(\pi g/\beta\lambda)}{\pi g/\beta\lambda} .
$$

The more the arc curves (longer gap, slower particle), the shorter the chord compared with the arc. Numbers: treat each gap of the example cavity roughly as a uniform field 40 mm long (the full width at half maximum of one on-axis peak); for $\beta = 0.057$, $\Theta/2 = 68.5^\circ$ and $T_g = 0.778$.

### Step 4: two gaps of opposite sign

The second gap is like the first except that the field points the other way: its chord is first turned by $180^\circ$ (opposite sign) and then by the angle the RF turns while the particle goes from the centre of the first gap to the centre of the second, $2\pi d/(\beta\lambda)$. The angle between the two equal chords is $2\pi d/(\beta\lambda) - 180^\circ$.

Two equal arrows at an angle $\alpha$ add up to $2\cos(\alpha/2)$ times one of them (the diagonal of a rhombus). Compared with "two arrows in the same direction" the ratio is $\lvert\cos(\alpha/2)\rvert = \lvert\sin(\pi d/\beta\lambda)\rvert$, so

$$
T(\beta) = T_g(\beta)\,\left\lvert\sin\frac{\pi d}{\beta\lambda}\right\rvert .
$$

For $d = \beta\lambda/2$ the angle is zero and the two chords point the same way: that is geometric beta. Numbers: for the example beam $2\pi d/(\beta\lambda) = 277^\circ$, the angle is $97^\circ$ (the "about $97^\circ$ further" of the main text) and $\cos 48.5^\circ = 0.66$; the rough model gives $T \approx 0.778\times 0.66 = 0.51$. For $\beta = 0.10$ it gives 0.91. Integrating the real field distribution as in step 2 gives 0.40 and 0.82: the rough model gets the trend, but the numbers need the real field.

### Step 5: the table of T(β) at constant speed

Computing with constant speed and the real on-axis field gives the table of $T(\beta)$ in the main text: 0.40 at $\beta = 0.057$, a maximum of 0.82 at $\beta \approx 0.1035$, the 90 % range $0.079 \le \beta \le 0.147$. With the origin at the cavity centre it takes the classical form

$$
\Delta W = qV_0T(\beta)\cos\varphi_s,\qquad
V_0 = \int\lvert E_z\rvert\,\dd z,\qquad
T(\beta) = \frac{\left\lvert\int E_z(z)\,\ee^{\,i\,2\pi z/(\beta\lambda)}\,\dd z\right\rvert}{\int\lvert E_z\rvert\,\dd z} ,
$$

(here $V_0$ and $E_z$ already include $K_e$). For the example map with $K_e = 1.36$, $V_0 \approx 0.978\ \mathrm{MV}$. Why the maximum is not at the geometric beta of 0.088 is explained in [Think about it 2](#rf-cavity-think).

### Step 6: at low energy the arrows no longer form a neat arc

The example beam goes from 1.527 MeV to about 1.87 MeV inside the cavity, and $\beta$ from 0.057 to 0.063, a 10 % change. As the particle speeds up the phase no longer grows uniformly and the arrows do not form a neat arc; moreover, for a different entrance phase the particle gains a different energy in the first gap and enters the second gap with a different speed at a different time. The constant-speed approximation gives a largest gain of $V_0T(0.057) = 0.978\times 0.40 \approx 0.39\ \mathrm{MeV}$, while integrating along the real trajectory (scanning the entrance phase point by point with V3 = 1) gives 0.482 MeV: the particle speeds up in the first gap, its β at the second gap is closer to $\beta_g$, and the timing of the second gap actually fits better. Results integrated along the real trajectory:

| Setting | Entrance phase $\varphi_0$ | $\Delta W$ / MeV | Resulting $V_0T$ / MV |
|---|---|---|---|
| V3 = 0, $\varphi_s = 0^\circ$ | $95.4^\circ$ | 0.475 | 0.475 |
| V3 = 0, $\varphi_s = -30^\circ$ | $56.6^\circ$ | 0.357 | 0.412 |
| V3 = 0, $\varphi_s = -33^\circ$ | $52.7^\circ$ | 0.340 | 0.405 |
| maximum of the V3 = 1 scan | $\approx 105.5^\circ$ | 0.482 | — |

So $V_0T$ itself depends on the phase. The largest energy gain is not at $\varphi_s = 0$ but at an entrance phase about $10^\circ$ later (where $\varphi_s \approx +8^\circ$). At $\varphi_s = -33^\circ$ the particle is first decelerated in the first half of the first gap (kinetic energy about 1.44 MeV at $z = 0.05\ \mathrm{m}$, minimum about 1.40 MeV) and only then accelerated, typical for $\beta \ll \beta_g$.

Conversely, when the beam's β is close to $\beta_g$ the approximation is good: with the injection energy changed to 4.727 MeV ($\beta = 0.100$) the largest gain of the phase scan, 0.803 MeV, is almost the same as $V_0T(0.10) = 0.978\times 0.82 = 0.80\ \mathrm{MeV}$.

### For readers who know Fourier transforms

$T(\beta)$ is the Fourier component of the on-axis field at the spatial frequency $k = 2\pi/(\beta\lambda)$ (normalised to $\int\lvert E_z\rvert\,\dd z$): the shorter the gap, the broader the field's spectrum, and both high β (low $k$) and low β (high $k$) can reach a large $T$. The $T_g$ of a uniform gap is the transform of a rectangle, a $\operatorname{sinc}$; two gaps of opposite sign are the convolution of the rectangle with $\delta(z - d/2) - \delta(z + d/2)$, whose transform adds the factor $2i\sin(kd/2)$. When the speed changes, $\varphi(z)$ is no longer linear in $z$, the Fourier form fails, and only integration along the trajectory remains, which is exactly why AVAS uses its t-code.

## Going deeper: phase stability and the synchronous-phase solution {#rf-cavity-deep-phase}

The main text used the numbers of one blue particle to show "early particles gain less, late ones more". This section writes it as general formulas, shows the longitudinal oscillation, and then uses the arrow picture of the previous section to explain why at low energy some synchronous phases have no solution. The first five steps use only school algebra and trigonometry; the last paragraph is for readers who know calculus.

### Step 1: a distance becomes a phase difference

Let $z$ be the particle's distance ahead of the synchronous particle ($z > 0$ means ahead, arriving first). It arrives $z/v$ early, the RF has turned $\omega z/v = 2\pi z/(\beta\lambda)$ less, so the phase it sees is

$$
\varphi = \varphi_s - \frac{2\pi z}{\beta\lambda} .
$$

Example: $z = 1.97\ \mathrm{mm}$, $\beta\lambda = 105.1\ \mathrm{mm}$, a phase difference of $360^\circ\times 1.97/105.1 = 6.8^\circ$.

### Step 2: a phase difference becomes an energy difference

It gains $qV_0T\cos\varphi$. With $\cos(\varphi_s - a) = \cos\varphi_s\cos a + \sin\varphi_s\sin a$ and, for small angles, $\cos a \approx 1$ and $\sin a \approx a$ (in radians), it gains more than the synchronous particle by

$$
\Delta W - \Delta W_s \approx qV_0T\,\sin\varphi_s\cdot\frac{2\pi z}{\beta\lambda} .
$$

For $\varphi_s < 0$, $\sin\varphi_s < 0$: particles in front ($z > 0$) gain less, those behind gain more. Example: $0.412\times(-0.5)\times 0.118 = -0.024\ \mathrm{MeV}$.

### Step 3: an energy difference becomes a momentum difference

Relativistically kinetic energy $W$ and momentum $p$ are related by $\dd W = v\,\dd p$, so

$$
\delta = \frac{\Delta p}{p} = \frac{\Delta W}{\beta^2\gamma\,mc^2} ,
$$

and at low speed $\beta^2\gamma\,mc^2 \approx 2W$, the $\Delta W/(2W)$ of the main text. Example: $-0.024/(0.003247\times 1.0016\times 938.27) = -7.9\,‰$; the animation's model gives $-8.0\,‰$. Inserting step 2, the longitudinal "kick" of one cavity is

$$
\Delta\delta = \frac{2\pi\,qV_0T\,\sin\varphi_s}{mc^2\,\lambda\,\beta^3\gamma}\;z ,
$$

proportional to $z$ with the opposite sign (for $\varphi_s < 0$): a spring, of the same form as a quadrupole's $\Delta x' = -x/f$.

### Step 4: a momentum difference becomes a change in position

A particle with $\delta$ more momentum is faster by $\delta/\gamma^2$ (just $\delta$ at low speed), and after a distance $L$ it has moved ahead of the synchronous particle by

$$
\Delta z = \frac{\delta}{\gamma^2}\,L ,
$$

see [Drift](#drift). Example: $-0.008\times 0.2\ \mathrm{m} = -1.6\ \mathrm{mm}$; the blue particle moves from 0.65 mm ahead of the centre to about 0.95 mm behind it.

### Step 5: together they make an oscillation

Step 3 says "the larger $z$, the more $\delta$ is reduced"; step 4 says "the more negative $\delta$, the faster $z$ decreases". Like a ball in a valley, a mass on a spring or a particle in a FODO channel, this pair of relations makes the particle oscillate about the synchronous particle; this is called the **synchrotron oscillation**, and the angle it turns through per cavity is the longitudinal phase advance. Squeezing too hard (too large $\lvert\varphi_s\rvert$ or Ke, too long a drift after the cavity) "overshoots": in the animation at $-60^\circ$ the bunch reaches its shortest length soon after the cavity, and by the end front and back have swapped and it is longer again. The oscillation frequency and the boundary of the stable region (the separatrix) are in [Longitudinal motion](#longitudinal).

### Step 6: why some synchronous phases have no solution at low energy

Back to the arrow picture of the previous section. If the particle's speed stayed constant, changing the entrance phase $\varphi_0$ would turn all arrows together by the same angle; the resultant would turn with them, and its direction $\varphi_s$ would take every value in $360^\circ$: every synchronous phase would have a solution.

At low energy this is not so. For a different entrance phase the particle gains a different energy in the first gap, so the arrows of the second gap are turned by a different angle relative to the first: changing $\varphi_0$ not only turns the chain of arrows but also changes its shape. When the two gaps nearly cancel, the resultant is short and swings back and forth near the origin without going all the way round it, and $\varphi_s$ can take only a range of angles.

The numbers for the example cavity (Ke = 1.36; linear model, checked with the engine):

- Scanning all entrance phases, $\varphi_s$ covers only about $65^\circ$–$107^\circ$ at 0.3 MeV, about $58^\circ$–$136^\circ$ at 0.8 MeV and about $14^\circ$–$144^\circ$ at 1.0 MeV.
- The accelerating solution $\varphi_s = -30^\circ$ exists only for 0.4–0.65 MeV and above 1.1 MeV.
- Without an accelerating solution, the engine accepts the solution $180^\circ$ away ($\varphi_s = 150^\circ$) and the cavity decelerates. According to the linear model this happens at 0.7–0.75 MeV and 1.05 MeV. Checked on Windows on 2026-11-02: at 0.7 MeV the entrance phase is $-107.7^\circ$, the synchronous particle loses 0.1129 MeV, and synData.txt still records $\varphi_s = -29.92^\circ$, although atan2 actually gives $150^\circ$; the linear model gives $-107.79^\circ$ and $-0.11286\ \mathrm{MeV}$. At 1.05 MeV the entrance phase is $-165.8^\circ$ and the synchronous particle loses 0.095 MeV; the linear model differs by $0.02^\circ$ and 0.005 %. When there are several roots, the engine takes the one on the rising branch of $\varphi_s(\varphi_0)$.
- When neither solution exists (0.15–0.35 MeV, 0.8–1.0 MeV) the engine refuses to run with “Initialisation stage error: … the scan phase to fail to find the corresponding RF phase” (checked at 0.3, 0.8 and 1.0 MeV).
- The linear model first iterates to a fixed point (which converges whenever there is a solution); if that fails it scans $\varphi_0$ and bisects for a root, looking first for the accelerating solution, then for the one $180^\circ$ away (with a "decelerating" warning); if neither exists it gives a warning with the reachable range and continues with the closest $\varphi_s$. The "synchronous phase" shown in the animation and the lab is always the value actually reached: at W = 0.9 MeV the reachable range is about $46^\circ$–$120^\circ$ and $45.9^\circ$ is used.

### For readers who know calculus

Averaging the kick of each cavity per unit length, steps 3 and 4 combine into

$$
\frac{\dd^2 z}{\dd s^2} + k_{l0}^2\,z = 0,\qquad
k_{l0}^2 = \frac{2\pi\,qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3} ,
$$

where $E_0T$ is the average effective field per unit length. This is the small-amplitude synchrotron oscillation; at large amplitude the nonlinearity of $\cos\varphi$ must be kept, giving the pendulum equation and the separatrix, see [Longitudinal motion](#longitudinal). The "no solution" of step 6 means that the map $\varphi_0 \mapsto \varphi_s$ is not surjective: when the winding number of $\varphi_s(\varphi_0)$ is zero it covers only an interval.

## Going deeper: RF defocusing in numbers {#rf-cavity-deep-transverse}

The main text said "longitudinal bunching necessarily means transverse defocusing" and gave a focal length of about 0.38 m. This section proves it step by step, derives the thin-gap formula, and explains why in the example it is mainly the first gap that defocuses. The first six steps use only field lines, electromagnetic induction and algebra from school; the last paragraph is for readers who know vector calculus.

### Step 1: in the bunch frame, the three spring constants add up to zero

Flying with the synchronous particle, the field around the bunch is almost electrostatic, and there are no charges nearby. Put a small cubic box of side $w$ at the bunch centre. Field lines do not appear or vanish out of nothing, so as many field lines enter the box as leave it.

Let the field vary linearly near the centre: $E_x = -a_x x$, $E_y = -a_y y$, $E_z = -a_z z$ ($a > 0$ means the field points towards the centre, i.e. focusing). The difference of the field lines leaving through the left and right faces is proportional to the difference of $E_x$ on the two faces, $-a_x w$; for top and bottom it is $-a_y w$, for front and back $-a_z w$. The total is zero:

$$
a_x + a_y + a_z = 0 .
$$

The force is $F = qE$, so the three "spring constants" add up to zero. Longitudinal bunching needs $a_z > 0$, so transversely $a_x + a_y < 0$; with axial symmetry $a_x = a_y = -a_z/2$. This is the three-dimensional version of the quadrupole's $K_x + K_y = 0$, often called the RF form of Earnshaw's theorem.

### Step 2: back in the laboratory, the transverse field near the axis

In the gap take a small cylinder centred on the axis, of radius $r$ and thickness $\Delta z$. The difference of the field lines through its front and back faces is $\pi r^2\,\Delta E_z$; it must leave through the side, of area $2\pi r\,\Delta z$:

$$
E_r\cdot 2\pi r\,\Delta z = -\pi r^2\,\Delta E_z\quad\Rightarrow\quad E_r = -\frac{r}{2}\,\frac{\Delta E_z}{\Delta z} .
$$

At the gap entrance $E_z$ grows along $z$, $E_r < 0$ points towards the axis (focusing); at the exit $E_z$ decreases, $E_r > 0$ points away from the axis (defocusing). This is the "field lines converge at the entrance and spread at the exit" of the main text.

### Step 3: a changing electric field produces a magnetic field

Step 4 of [Going deeper: the fields in a resonator](#rf-cavity-deep-modes) said: when the flux of the electric field changes, a magnetic field appears around it. For a circle of radius $r$ around the axis the electric flux is $\pi r^2E_z$, and the result is

$$
B_\theta = \frac{r}{2c^2}\,\frac{\Delta E_z}{\Delta t} .
$$

The radial force on the particle is $F_r = q(E_r - vB_\theta)$. The second term is roughly $\beta^2$ times smaller than the first and negligible at low speed, but as shown below it cancels most of the electric force at high speed.

### Step 4: a small trick, only the time-varying part remains

The $E_z$ the particle sees in the gap changes for two reasons: it moves to places where the field is different (the change along $z$), and the field itself changes in time (the change in $t$). Rewrite the "change along $z$" of step 2 as "the total change the particle sees on its way" minus "the change in time", and insert it into $F_r$:

- The "total change on the way" term, added from the gap entrance to its exit, is the field at the exit minus the field at the entrance; the field is zero at both ends, so the sum is zero. This is "a static field gives no net effect".
- What remains is the "change in time" term, where electric and magnetic force together leave only $1/\gamma^2$ of it.

The rate of change in time of $E_z = E(z)\cos\varphi$ is $-\omega E(z)\sin\varphi$. Adding it up over the gap ($\dd t = \dd z/v$) and using the definition of the previous section, $\int E\sin\varphi\,\dd z \to E_0TL\sin\varphi$, the change of transverse momentum is

$$
\Delta p_x = -\frac{\pi\,qE_0TL\,\sin\varphi}{c\,\lambda\,\beta^2\gamma^2}\;x .
$$

Dividing by $mc$ gives the thin-gap formula (Wangler, chapter 7)

$$
\Delta(\beta\gamma\,x') = -\frac{\pi\,qE_0TL\,\sin\varphi}{mc^2\,\lambda\,\beta^2\gamma^2}\;x .
$$

The same holds for $y$. Here $qE_0TL$ is the energy-gain amplitude of the gap (in MeV, $q$ in units of $e$), and $\beta\gamma$ is taken at the gap centre. For $\varphi < 0$ the right-hand side has the same sign as $x$: transverse defocusing. Its strength grows as $1/(\beta\gamma)^2$ (as $1/(\beta\gamma)^3$ after dividing by the momentum), so it is worst at low energy.

### Step 5: transverse against longitudinal

Write the longitudinal kick of step 3 of the previous section in the same form for one gap:

$$
\Delta\delta = \frac{2\pi\,qE_0TL\,\sin\varphi}{mc^2\,\lambda\,\beta^3\gamma}\;z .
$$

For $\varphi < 0$ an early particle has its $\delta$ reduced: longitudinal focusing. Compared with step 4, the transverse strength is exactly half the longitudinal one with the opposite sign, which is $a_x = a_y = -a_z/2$ of step 1. Including the increase of momentum from the energy gain, the thin-gap matrix in the $(x, x')$ plane is

$$
M_x = \begin{pmatrix} 1 & 0 \\[2pt] -\dfrac{\pi\,qE_0TL\sin\varphi}{mc^2\lambda\,\bar\beta^2\bar\gamma^2\,(\beta\gamma)_{out}} & \dfrac{(\beta\gamma)_{in}}{(\beta\gamma)_{out}} \end{pmatrix} .
$$

The ratio at the bottom right is **adiabatic damping**: the transverse momentum is unchanged while the longitudinal momentum grows, so $x'$ shrinks. For the example cavity $(\beta\gamma)_{in}/(\beta\gamma)_{out} = 0.05708/0.06311 \approx 0.90$: the geometric emittance shrinks by about 10 %, the normalised emittance is unchanged.

### Step 6: the example numbers

**Rough estimate**: take $\varphi_s = -33^\circ$, give each gap half of the 0.405 MeV, both at $-33^\circ$, with the injection $\beta\gamma$; the thin-gap formula gives $1/f \approx 1.08\ \mathrm{m^{-1}}$ per gap and about $2.15\ \mathrm{m^{-1}}$ for the cavity. The length-averaged form (Wangler's notation)

$$
k_{l0}^2 = \frac{2\pi\,qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3},\qquad
k_{t,\mathrm{rf}}^2 = -\frac{\pi\,qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3} = -\frac{k_{l0}^2}{2}
$$

gives the same result: with the cavity's average field $E_0T \approx 0.405\ \mathrm{MV}/0.21\ \mathrm{m} = 1.93\ \mathrm{MV/m}$, $k_{t,\mathrm{rf}}^2 \approx -10.3\ \mathrm{m^{-2}}$, times 0.21 m again $2.15\ \mathrm{m^{-1}}$. Both are proportional to $\sin(-\varphi_s)$ and $1/(\beta\gamma)^3$: at low energy, the larger $\lvert\varphi_s\rvert$, the stronger the bunching and the stronger the transverse focusing needed. This is one reason why low-energy sections use lower frequencies and phases around $-30^\circ$ as a compromise; see [Longitudinal motion](#longitudinal) and [Lattice design](#lattice-design). A thin defocusing lens of this strength at the cavity centre, plus adiabatic damping, gives an exit rms $x$ of about 1.67 mm for the example beam, while the linear model gives 1.81 mm.

**The two gaps separately**: the difference is telling. Far below $\beta_g$ the two gaps see different phases. Integrating each gap separately along the real trajectory for $\varphi_s = -33^\circ$: the first gap sits about $-81^\circ$ from its own crest (amplitude $qE_0TL \approx 0.27\ \mathrm{MeV}$, actual gain only $+0.04\ \mathrm{MeV}$), the second at about $+9^\circ$ (amplitude 0.30 MeV, gain $+0.30\ \mathrm{MeV}$). Defocusing follows $\sin\varphi$, not the gain: the first gap, which hardly accelerates, defocuses strongly ($1/f \approx +3.0\ \mathrm{m^{-1}}$), while the second actually focuses slightly ($1/f \approx -0.37\ \mathrm{m^{-1}}$), together about $2.6\ \mathrm{m^{-1}}$, a focal length of about 0.38 m. These two thin lenses at the two gap centres reproduce the linear model's 1.81 mm (1.41 mm for a drift of the same length). A cavity's $\varphi_s$ is just one number; it hides how the phase is shared between the gaps, and at low β the transverse effect depends precisely on that sharing.

**The cavity exit at different phases**: with the cavity directly at the start of the line (no 5 cm drift before it, no space charge, linear model), the results at the cavity exit are:

| $\varphi_s$ | $W_{out}$ / MeV | exit rms $x$ / mm | exit rms $z$ / mm |
|---|---|---|---|
| $-60^\circ$ | 1.694 | 1.96 | 0.62 |
| $-33^\circ$ | 1.867 | 1.81 | 0.70 |
| $0^\circ$ | 2.002 | 1.50 | 1.10 |
| $+33^\circ$ | 1.951 | 1.20 | 1.74 |
| drift of the same length | 1.527 | 1.41 | 1.48 |

The entrance rms sizes are $x \approx 0.95\ \mathrm{mm}$ and $z \approx 1.32\ \mathrm{mm}$, and the initial beam is diverging. The pattern is the same as in the animation table of the main text: the more negative $\varphi_s$, the tighter longitudinally and the wider transversely. At the example current of 0.184 mA, space charge changes the rms sizes at the cavity exit by less than 0.5 % in the linear model.

### For readers who know vector calculus

In the bunch rest frame the quasi-static potential satisfies Laplace's equation

$$
\frac{\partial^2\Phi}{\partial x^2} + \frac{\partial^2\Phi}{\partial y^2} + \frac{\partial^2\Phi}{\partial z^2} = 0 ,
$$

and the coefficients of the linear restoring forces are proportional to the second derivatives of $\Phi$, which add up to zero (step 1). In the laboratory frame, from $\nabla\cdot\boldsymbol{E} = 0$ and $\nabla\times\boldsymbol{B} = c^{-2}\,\partial\boldsymbol{E}/\partial t$, an axially symmetric gap has

$$
E_r \approx -\frac{r}{2}\,\frac{\partial E_z}{\partial z},\qquad
B_\theta \approx \frac{r}{2c^2}\,\frac{\partial E_z}{\partial t} .
$$

The trick of step 4 is $\partial_z E_z = v^{-1}(\dd E_z/\dd t - \partial_t E_z)$ with $\dd/\dd t$ along the trajectory; then $F_r = -\tfrac{qr}{2}\bigl[v^{-1}\,\dd E_z/\dd t - (v\gamma^2)^{-1}\partial_t E_z\bigr]$, the first term integrates to zero and the second gives the thin-gap formula. A speed change inside the gap, fields that are not axially symmetric (the quadrupole component of an HWR) and nonlinear terms are not part of this derivation; the engine integrates the Lorentz force directly in the three-dimensional map and includes them naturally.

## In AVAS {#rf-cavity-in-avas}

### Syntax and parameters

An RF cavity is described by the field-map element `field` with field type 1:

```text
field  L  R  V3  1  f  phase  Ke  Kb  file
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
```

```avas-params
field
```

- `L` (m): element length; it should match the length recorded in the field-map header (0.21 m in the example). `R` (m): aperture radius; particles beyond it are counted as lost.
- `V3`: sets the meaning of `phase` (see below).
- `f` (Hz): RF frequency of this cavity, written `162.5e6` in the example. The beam (bunch repetition) frequency is given separately as `frequency` in beam.txt; the two are set independently.
- `phase` (degrees): its meaning depends on V3.
- `Ke`, `Kb`: scale factors of the electric and magnetic fields; `file`: field-map file name (without extension); an RF cavity reads `.edx/.edy/.edz` and `.bdx/.bdy/.bdz`, format in [Field maps](#field-maps).

> **AVAS convention (verified with the engine)**: electric fields in the map are in MV/m and multiplied by Ke; magnetic fields are in T and multiplied by Kb. The RF electric field acts as $K_e\,\boldsymbol{E}(\boldsymbol{r})\cos(\omega t+\varphi_0)$, the RF magnetic field as $K_b\,\boldsymbol{B}(\boldsymbol{r})\sin(\omega t+\varphi_0)$, consistent with the $90^\circ$ between E and B in [resonant cavities](#rf-cavity-modes).

> **AVAS convention**: the engine does not use the thin-gap matrix; it integrates the Lorentz force directly in the field map's $\boldsymbol{E}(x,y,z)$ and $\boldsymbol{B}(x,y,z)$ (t-code), so nonlinearity, transverse–longitudinal coupling and the non-axisymmetric parts of the HWR are all included. The linear model (editor preview, primer animations and labs) tracks the reference particle in 1 mm steps and builds the transverse optics from 4 mm slices, using the field gradients near the axis and a linearised RF bunching approximation.

### V3: phase reference

| V3 | Meaning of `phase` | Use |
|---|---|---|
| 0 | Synchronous phase $\varphi_s = \operatorname{atan2}(\int E\sin\varphi\,\dd z,\ \int E\cos\varphi\,\dd z)$; the engine solves for the entrance phase | Design: specify $\varphi_s$ directly; it is kept when upstream elements change |
| 1 | RF phase $\varphi_0$ when the reference particle reaches the element entrance | Phase scans, reproducing a given entrance phase |
| 2 | Absolute RF phase at $t = 0$; entrance phase $\varphi_0 = \varphi + 360^\circ\cdot f\cdot t_{in}$ | Modelling a real machine: phases locked to the master clock, so upstream energy changes alter the arrival time and the actual $\varphi_s$ |

The engine records in `synData.txt` the time and energy of the reference particle at each element entrance together with $\varphi_s$ and $\varphi_{RF}$; setting one computes the other (`φRF = phase_t0 + 360·f·t_in`). When a segment run takes a middle section on its own, V3 = 2 phases must be converted as $\varphi_{RF} - 360^\circ\cdot f\cdot(t_{in} - T_{entry})$; they cannot be reused unchanged or switched to V3 = 0. AVAS's segment runs do this automatically (`segment.rephase`).

### When V3 = 0 has no solution

At low energy the synchronous phase may have no solution (the reason is in [Going deeper: phase stability and the synchronous-phase solution](#rf-cavity-deep-phase)). Example cavity, Ke = 1.36, $\varphi_s = -30^\circ$:

| Injection energy | Engine | Linear model |
|---|---|---|
| 0.4–0.65 MeV, above 1.1 MeV | normal, accelerating solution | same |
| 0.7–0.75 MeV, 1.05 MeV | takes the solution $180^\circ$ away, the cavity decelerates; synData.txt still records $-30^\circ$ | takes the same solution, with a "decelerating" warning |
| 0.15–0.35 MeV, 0.8–1.0 MeV | refuses to run: “Initialisation stage error: … the scan phase to fail to find the corresponding RF phase” | warns, gives the reachable range and continues with the closest $\varphi_s$ |

The energy ranges are computed with the linear model; the engine's behaviour was checked at 0.3, 0.7, 0.8, 1.0 and 1.05 MeV (2026-11-02, Windows). When there are several roots the engine takes the one on the rising branch of $\varphi_s(\varphi_0)$. In such a case, give the entrance phase directly with V3 = 1, or change the upstream energy or the cavity's field level.

### Ke and Kb

- **Ke is the field-level knob.** In the example at $\varphi_s = -33^\circ$, Ke = 0.5, 1.0, 1.36, 2.0 give $\Delta W$ = 0.122, 0.248, 0.340, 0.502 MeV. At Ke = 1.36, $V_0 \approx 0.978\ \mathrm{MV}$.
- **Kb should scale with Ke.** The electric and magnetic fields of a cavity are two parts of the same mode of oscillation; changing Ke without Kb is physically inconsistent. The example uses $K_b = -K_e$. The manual does not explain the minus sign, but the field map answers it: the circulation of the map's $\boldsymbol{B}$ around the axis equals $+(\omega/c^2)$ times the flux of the map's $E_z$ to within 2 % (see [Field maps](#field-maps-interpolation)), i.e. the map stores $\boldsymbol{B}$ as the amplitude of a field varying as $-\sin$, like the $B_\theta$ of the pillbox mode. With AVAS's $+\sin$ convention only $K_b = -K_e$ makes the electric and magnetic fields one Maxwell solution. For field maps from other sources determine the sign of $K_b/K_e$ the same way (or from the export convention) and keep the ratio fixed.
- For low-β protons the RF magnetic field matters far less than the electric field (see [Think about it 1](#rf-cavity-think)): switching Kb between 0 and $\pm 1.36$ in the example changes the exit rms sizes of the linear model by less than 0.4 %.

> **AVAS convention**: in AVAS the cavity field level is given directly by Ke (and Kb) and is fixed. The engine does not model the power source, beam loading, detuning or low-level RF control; field and phase jitter are introduced with the error commands `err_cav_*`, see [Errors and correction](#errors).

> **Note**: `scanphase` in input.txt and `scanData.txt` do **not** affect the phases: neither the original nor the new kernel reads them (verified by reverse engineering and runs); the RF phases come only from `phase` and V3 of the field line. With V3 = 0 the kernel scans the entrance phase itself to find the one giving the requested $\varphi_s$; this has nothing to do with `scanphase`. To reproduce a given entrance phase, write it directly in the field line with V3 = 1 or V3 = 2. The Simulation settings page no longer offers a phase-scan option and removes a leftover `scanphase` line from input.txt when saved; the `scanData.txt` of older projects stays in the project and can be viewed on the Files page.

```avas-params
input:scanphase input:steppercycle input:longlimits
```

### Energy: synchronous particle against mean

When comparing energies, keep two numbers apart:

- **The synchronous particle's energy**: `synData.txt` records the reference particle on axis. The linear model tracks it too; the two differ by about 0.01 %.
- **The mean energy**: column 0 of DataSet.txt is the **mean** energy of all macro-particles. Off-axis particles feel a different $E_z$ in the three-dimensional map: 2 mm off axis in the example `hwr010` the gain is 4–6 % higher than on axis, and averaged over a Gaussian beam with σ of 1–1.8 mm it is 2.5–5 % higher. For the example cavity (1.527 MeV, $\varphi_s = -33^\circ$) the engine's mean gain is 4.5 % above the linear model (0.3556 against 0.3395 MeV, checked on Windows on 2026-11-02). At 0.5 MeV the two gaps nearly cancel and the engine's mean gain is about 20 % below the linear model instead.

`tools/compare_preview_engine.py` reports both numbers. The linear model's rms sizes usually differ from the engine by 1–2 %, and by up to 13 % when multi-particle effects are strong; the engine's results are the reference.

### Typical values

| Parameter | Role | Example / typical range |
|---|---|---|
| `phase` (V3 = 0) | Synchronous phase: a compromise between acceleration, longitudinal acceptance and transverse defocusing | Example $-33^\circ$; usually $-40^\circ$ to $-20^\circ$ at low energy, closer to the crest at high energy |
| `Ke` | Electric field level | Example 1.36 ($V_0 \approx 0.98\ \mathrm{MV}$); upper limit set by $E_{pk}$, $B_{pk}$ |
| `Kb` | Magnetic field level | Proportional to Ke, example $-1.36$ |
| `f` | Cavity frequency | Example 162.5 MHz; 162.5 and 325 MHz are common in low-β superconducting sections |
| `R` | Aperture for the loss criterion | Example 0.02 m; should match the actual beam pipe / drift tube bore |
| input.txt `steppercycle` | Integration steps per RF period | Example 50: $\beta\lambda/50 \approx 2.1\ \mathrm{mm}$, comparable to the 2 mm grid of the map |

### Choosing phases in a design

1. Choose each cavity's $\varphi_s$ from the analysis in [Longitudinal motion](#longitudinal): the longitudinal acceptance (phase width about $3\lvert\varphi_s\rvert$) must hold the bunch, and the longitudinal and transverse phase advances must avoid parametric resonances; see [Lattice design](#lattice-design).
2. During design write $\varphi_s$ directly with V3 = 0. When upstream elements change, the engine solves for the entrance phases again and each cavity keeps its synchronous phase.
3. Once the design is fixed, read each cavity's entrance RF phase from `synData.txt` and switch to V3 = 1 or V3 = 2 if needed, to study phase errors or reproduce machine settings.

### Phase scans in AVAS

Set the cavity to V3 = 1, then use a parameter scan on the research page, or scan the entrance phase point by point from the command line:

```text
avas scan --input InputFile --target 2 --param phase --values -180:170:36
```

`--target 2` means line 2 of the lattice file (where the cavity is in the example); an element name works too; `start:stop:count` gives 36 points $10^\circ$ apart; the result metric `energy_out` is the exit energy. With the curve in hand:

1. Find the maximum. In the example it is near $\varphi_0 \approx 105.5^\circ$ with $\Delta W_{max} \approx 0.482\ \mathrm{MeV}$; the minimum is near $\varphi_0 \approx -106^\circ$ with $\Delta W \approx -0.31\ \mathrm{MeV}$; the curve is not symmetric.
2. Note that "$-33^\circ$ from crest" and AVAS's $\varphi_s = -33^\circ$ differ at low energy (see [Think about it 3](#rf-cavity-think)).
3. When the beam's β approaches the geometric beta the two coincide: at 4.727 MeV injection ($\beta = 0.10$) the scan is nearly symmetric, the largest $\Delta W \approx 0.803\ \mathrm{MeV}$ is at $\varphi_s \approx 0$, and $\varphi_s = -33^\circ$ gives $\Delta W \approx 0.673\ \mathrm{MeV}$, exactly $0.803\cos 33^\circ$.

### Pitfalls

- **The field map does not include the cavity surface**: field maps usually cover only the region near the beam bore (the example only $\lvert x\rvert, \lvert y\rvert \le 2\ \mathrm{cm}$), so $E_{pk}$ cannot be read from an AVAS map; use the $E_{pk}/E_{acc}$ of the cavity design report to check that the Ke you use is within the operating range.
- **β mismatch**: when the beam's β is clearly below the cavity's geometric beta, $T$ is small and the first gap may decelerate first. With the example's injection $\beta = 0.057$, the cavity at $\varphi_s = -33^\circ$ delivers only about 35 % of $V_0$.
- **Longitudinal nonlinearity**: when the bunch's phase width is comparable to $\lvert\varphi_s\rvert$, particles fall outside the separatrix and the longitudinal emittance grows or particles are lost longitudinally; the linear model cannot show this, use the multi-particle engine. `longlimits` in input.txt sets the longitudinal loss boundary.
- **Field-map noise**: the example map is itself noisy (the on-axis $E_z$ jitters visibly from point to point). This is a property of the data, not a reading error; see [field-map noise](#field-maps-interpolation).
- **Superposed fields**: when a cavity overlaps the fringe field of a solenoid use `superpose`; one superposition group can contain at most one RF cavity, see [Superposed fields](#superpose).
- **First and last elements**: in multi-particle runs the first and last elements should not be matrix-model elements; an RF cavity is a field-map element and has no such restriction.

## Further reading {#rf-cavity-reading}

- T. P. Wangler, *RF Linear Accelerators* (2nd ed.), chapters 2–5 (resonators, transit-time factor, shunt impedance) and chapters 6–7 (longitudinal and transverse dynamics, thin-gap formulas).
- H. Padamsee, J. Knobloch, T. Hays, *RF Superconductivity for Accelerators*: $Q_0$, $R/Q$, geometry factor and peak fields of superconducting cavities.
- The CAS – CERN Accelerator School lectures on RF cavities and linacs.
- AVAS user manual `docs/使用说明20260427.docx`: the `field` element and its phase parameters.
- This primer: [Longitudinal motion](#longitudinal), [Field maps](#field-maps), [Superposed fields](#superpose), [Errors and correction](#errors), [the tutorial](#tutorial), [References](#references).
