# From ion source to target: a linear accelerator as a whole

If all we want is protons at a high energy, the easiest plan seems to be: build one long, uniform tube and accelerate all the way. Real linear accelerators are instead split into several sections: an ion source, a low-energy transport line, an RFQ, a medium-energy transport line, several families of differently shaped superconducting cavities and a high-energy transport line. Each section has its own structure, its own "beat" (RF frequency) and its own kind of focusing. Why go to all this trouble?

This chapter follows "our proton" from its birth to the end of the line and shows that this division is not an engineering habit. It is forced by three physical facts:

1. the proton's velocity changes by a factor of a hundred along the way;
2. the mutual repulsion of the protons (space charge) is strongest at low energy;
3. an oscillating RF electric field can only accelerate a beam that is already "bunched".

With this chapter in mind, the next five chapters each answer why a section is where it is and why it looks the way it does.

> **How to read this primer (three routes)**
> - **New to accelerators**: read Part 1 (the first six chapters, "A proton's journey") in order for the big picture of a whole accelerator. The main text of every chapter needs only high-school physics; the collapsed blocks marked "Going deeper (optional)" can be skipped at first. Drag the interactive sliders in each chapter to see the trends. When you want to understand a concept properly, Part 2 (starting from [How AVAS models a beam](#intro), [Relativistic kinematics](#kinematics) and [Phase space and emittance](#phase-space)) builds every definition and derivation from scratch.
> - **Familiar with physics**: skim "[Numbers along the way](#journey-overview-thread)" below, open the "Going deeper" blocks for the quantitative derivations, then jump to the [Components](#drift) chapters as needed or work through the five-step [guided tutorial](#tutorial).
> - **Accelerator experts / looking up software conventions**: each chapter's final dashed-box section "**In AVAS**" collects exact keywords, units and kernel behaviour; a one-stop summary of coordinate/sign conventions, differences from TraceWin and known kernel pitfalls is in [References: In AVAS](#references-in-avas).

## The route {#journey-overview-route}

First a unit. When the charge of one proton (or electron) passes through a voltage of 1 volt, the energy it gains is 1 **electronvolt** (eV). 1 keV is a thousand electronvolts, 1 MeV a million, 1 GeV a billion. Saying the proton has a kinetic energy of 35 keV means it is as if it had been accelerated through 35,000 volts.

> **A few words**: a **beam** is a large crowd of particles flying in the same direction. A **DC beam** is a continuous beam, like water running from a tap, with no gaps. A **bunch** is a small packet of particles; a bunched beam is like a train made of separate carriages. **RF** (radio frequency) means an electric field that oscillates millions of times per second or more; a **cavity** is a metal hollow that stores such an oscillating field, and a proton crossing it gets a push from the field.

Now let us walk the route once; the following sections come back to explain every turn.

1. **[Ion source](#journey-source)**: our proton is born in the plasma of the ion source and pulled out by a voltage of a few tens of kilovolts. Its kinetic energy is 35 keV, and together with its companions it forms a DC beam.
2. **Low-energy beam transport** (LEBT): a few solenoids (coil magnets wound around the pipe) focus this beam into the entrance of the RFQ.
3. **[RFQ](#journey-rfq)**: it holds the beam tight with electric fields, slowly chops the continuous beam into bunches, and accelerates the protons to 1.527 MeV. From here on the beam moves bunch by bunch at a beat of 162.5 MHz: 162.5 million bunches per second, 6.15 ns (nanoseconds, billionths of a second) apart.
4. **[Medium-energy beam transport](#journey-mebt)** (MEBT): no more acceleration, only "shaping": it adjusts the size and length of the beam to what the next section needs, measures and collimates it, and can remove some bunches if necessary.
5. **[Superconducting section](#journey-sc)**: several families of cavities. 162.5 MHz half-wave resonators (HWR) take the protons from 1.5 MeV to about 10 MeV; 325 MHz spoke cavities continue to about 150 MeV; 650 MHz elliptical cavities take them to about 1 GeV.
6. **[High-energy beam transport](#journey-hebt)** (HEBT): quadrupoles and dipoles deliver the beam to the target or the beam dump.

This is a typical continuous-wave (CW, the beam runs without interruption) proton linac. The boundary energies between sections (35 keV, 10 MeV, 150 MeV, 1 GeV) are common values for such machines, not the parameters of one particular machine. The only fully "real" point is the RFQ exit: the initial beam of the AVAS example project `examples/hwr010` is exactly a 1.527 MeV proton beam at the exit of a 162.5 MHz RFQ (particle file `part_rfq.dst`), and its next stop is the HWR superconducting section. AVAS simulates everything from that point on.

## A hundredfold change in velocity {#journey-overview-velocity}

The first fact is the simplest and also the most decisive.

Think of pushing a swing. To make it go higher you must push while it is moving forward; pushing too early or too late is wasted effort. An RF cavity is the same: the electric field in it reverses 162.5 million times per second (162.5 MHz), and only a proton that crosses an **accelerating gap** (a short stretch of the cavity where the field is concentrated) while the field points forward is accelerated. A cavity often has several gaps, and neighbouring gaps have opposite fields. When the proton flies from one gap to the next, the field must have just turned around, so **the distance between gaps must suit the proton's velocity**. If the velocity changes, the distance must change too.

How much does the velocity actually change? At low speed we can use the school formula for kinetic energy, $W = \tfrac12 mv^2$. Using the speed of light $c$ as a yardstick and writing $\beta = v/c$ (the velocity as a fraction of the speed of light), we get

$$
\beta \approx \sqrt{\frac{2W}{mc^2}} .
$$

Here $mc^2$ is the proton's **rest energy** (Einstein's $E = mc^2$), 938.272 MeV. For 35 keV: $\beta = \sqrt{2 \times 0.035 / 938.272} = 0.00864$, a velocity of about 2600 km/s, less than one per cent of the speed of light. At 1.527 MeV, $\beta = 0.0570$.

At higher energies this formula fails (it would give speeds faster than light) and must be replaced by the relativistic formulas $\gamma = 1 + W/(mc^2)$, $\beta = \sqrt{1 - 1/\gamma^2}$, where the **Lorentz factor** $\gamma$ says how many times the total energy exceeds the rest energy (see [Relativistic kinematics](#kinematics)). At 1 GeV, $\beta = 0.875$. From the ion source to the end the velocity grows by a factor of about 101. Measured in factors, most of the growth happens in the first few tens of MeV (at 50 MeV the velocity is already 36 times the starting value): at low energy $\beta$ grows like $\sqrt{W}$, so the energy has to quadruple for the velocity to double; close to the speed of light the velocity hardly increases any more.

Now the gap distance. One period of the field lasts $1/f$, and in that time the proton flies a distance

$$
\beta\lambda = \frac{\beta c}{f},
$$

where $\lambda = c/f$ is the RF **wavelength**; at 162.5 MHz, $\lambda = 1.845$ m. When neighbouring gaps have opposite fields (the π mode) the field reverses every half period, so the spacing must equal the distance flown in half a period, $\beta\lambda/2$; when the fields point the same way (the 0 mode, as in an Alvarez drift-tube linac) the spacing is $\beta\lambda$. At 162.5 MHz the $\beta\lambda/2$ of a 35 keV proton is only 8.0 mm, at 1.527 MeV it is 52.6 mm, and at 1 GeV it would be 0.81 m. No single geometry can "keep in step" with the proton over such a range.

In the lab below you can drag the kinetic energy and watch $\beta$ and $\beta\lambda$ change; it starts at the 35 keV of the ion source exit. Drag $W$ from 0.035 MeV to 1.527 MeV and then to 1000 MeV, and notice that $\beta\lambda$ grows fast at first and then slowly.

```avas-lab
kinematics W=0.035
```

### One cavity suits only a range of velocities

Even with the gap spacing right, one cavity can only serve a range of velocities. The reason is that the proton needs time to cross the cavity, and the field changes during that time. If the proton is too slow, the field reverses before it has left a gap, and the energy gained at first is taken back afterwards; if it is too fast, it crosses two opposite gaps before the field has had time to reverse, and the two pushes cancel. Only at the right velocity does it collect almost all of the energy.

Written as a formula, the energy gained by a proton crossing one cavity is

$$
\Delta W = qV_0\,T(\beta)\cos\varphi_s .
$$

The letters mean: $q$ is the proton's charge; $V_0$ is the total voltage obtained by adding up the field amplitudes of all gaps, the most the proton could get if the field stood still at its peak; $\varphi_s$ is the **synchronous phase**, which says at which moment of the oscillation the reference proton crosses the centre of the cavity, $0°$ being the crest where the field is strongest and $-30°$ a little before the crest (why one deliberately does not ride the crest is explained in [Longitudinal motion](#longitudinal)); and $T(\beta) \le 1$ is a discount called the **transit-time factor**, which is exactly the "penalty for the wrong velocity" described above. How $T$ follows from the gap spacing is shown in "[Going deeper: the transit-time factor](#journey-overview-deep-ttf)" below.

Computing it for the example project's HWR field map `hwr010` (162.5 MHz, 0.21 m long; on-axis field, proton velocity assumed constant in the cavity): $T$ peaks at $\beta \approx 0.104$ at about 0.82, matching the "010" in its name (presumably a design β of 0.10); $T$ stays above 70 % of its maximum for $\beta = 0.066$–0.21, i.e. proton energies of about 2–22 MeV. At the example beam's $\beta = 0.057$, $T$ is only 0.40; at $\beta = 0.145$ (10 MeV) it is 0.75; at $\beta = 0.43$ (100 MeV) it drops to 0.31. One family of cavities is useful over roughly a factor of 3 in velocity, while the whole machine has to cover a factor of 100, so several families are needed, each optimised for one design velocity (the design $\beta$; for how it relates to the geometric β set by the gap spacing, see [Superconducting section](#journey-sc-ttf)).

The lab below is this HWR cavity (synchronous phase −30°). The default entry energy is 5 MeV, close to its best velocity; set $W$ back to 1.527 MeV, then try 10 MeV and 50 MeV, and compare the "maximum gain" row: about 0.48 MeV at 1.527 MeV, about 0.80 MeV at 5 MeV, about 0.72 MeV at 10 MeV, and only about 0.41 MeV at 50 MeV. (The lab accounts for the change of velocity inside the cavity, so its numbers differ slightly from the constant-velocity $T$ above.)

```avas-lab
rf-cavity W=5
```

## Space charge is strongest at low energy {#journey-overview-space-charge}

The second fact is **space charge**: all the protons in the beam are positively charged, like charges repel, and this force tries to push the beam apart. Why is it strongest at low energy? There are three reasons, all of which everyday experience can explain.

- **Slow means crowded.** Imagine a toll gate that lets the same number of cars through every minute. If the cars on the road are slow, they are packed bumper to bumper; if they are fast, they are far apart. A beam is the same: for a given **beam current** (charge passing per second, in mA), the slower the protons, the more of them per unit length, and the stronger the repulsion.
- **Slow means easy to push.** The same sideways force acting on a slow, light proton acts for longer on each stretch of the way, and the proton's own momentum is small, so it is deflected more easily.
- **Fast gets help from magnetism.** Moving charges are a current, and two wires carrying currents in the same direction attract each other (the school-physics force between currents). The protons in a beam feel this magnetic attraction too, and it cancels part of the electric repulsion. But the magnetic attraction is only a fraction $\beta^2$ of the electric repulsion; at low speed $\beta^2$ is negligible and the repulsion acts almost in full.

Together, the effect of space charge grows steeply as the velocity drops, roughly like $1/(\beta\gamma)^3$ ($\beta\gamma$ is the dimensionless momentum, about equal to $\beta$ at low speed). The dimensionless number measuring its strength is the **generalized perveance** $K$:

$$
K = \frac{2I}{I_0\,(\beta\gamma)^3}, \qquad I_0 \approx 31.3\ \mathrm{MA} .
$$

$I$ is the beam current and $I_0$ a constant set by the proton's charge and mass (where the formula comes from is shown in "[Going deeper: how strong is space charge](#journey-overview-deep-space-charge)"). Counting: a 35 keV, 10 mA DC beam has $K = 9.9\times10^{-4}$; the same current at 1.527 MeV has $K = 3.4\times10^{-6}$, 288 times smaller. The example beam's average current is only 0.184 mA, giving $K = 1.8\times10^{-5}$ (35 keV) and $6.3\times10^{-8}$ (1.527 MeV). But bunching eats part of this advantage: the bunch at the RFQ exit has an rms duration of about 73 ps (rms, "root mean square", is the typical width of a distribution; 1 ps is a trillionth of a second), while bunches are 6.15 ns apart, so the protons are all crowded into the bunches. For a Gaussian bunch the peak current at the bunch centre is $1/(f\sqrt{2\pi}\,\sigma_t) \approx 34$ times the average current ($f$ is the bunch frequency, $\sigma_t$ the rms duration).

What should space charge be compared with? With the **emittance**. The [Drift](#drift) chapter showed that even with no force at all a beam spreads out because its particles fly in slightly different directions; emittance measures this built-in disorder (see [Phase space and emittance](#phase-space)). So two things push the beam outwards: space charge and emittance. Comparing them for 35 keV, 10 mA, a beam edge radius of 5 mm and the example beam's emittance, the envelope equation (derived in the same deep block above) gives an outward "push strength" of 0.198 m⁻¹ for space charge but only 0.034 m⁻¹ for emittance: the former is about 6 times the latter. Such a beam is called **space-charge dominated**: its size is set mainly by its own Coulomb repulsion, not by the disorder of its particles.

The animation below is the example beam (1.527 MeV, slightly converging at the entrance) in a drift, with the current preset to 10 mA: after 1.5 m the rms size is about 7.7 mm. Drag "Beam current" back to 0.184 mA and the size after the same distance is only about 4.8 mm. And remember that at 35 keV the same current gives a space-charge effect another 288 times stronger.

```avas-motion
drift I=10
```

This gives the design rules for the low-energy end: focusing must be strong and the focusing elements closely spaced to hold the repulsion down; bunching must be slow, because bunching raises the peak current; and the energy must be raised quickly to leave this regime. Together these three rules are the design brief of the RFQ.

## RF only accelerates a bunched beam {#journey-overview-bunching}

The third fact: what comes out of the ion source is a DC beam, with the protons spread evenly in time. The field of an RF cavity points forward half of each period and backward the other half, so of evenly spread protons half are accelerated and half decelerated, and on average they gain nothing.

Think of surfing. Many surfers float evenly spread over the sea; when a wave comes, only those who happen to be at the right spot just in front of the crest are carried along, and the rest are left behind. An RF cavity is the same: it can steadily accelerate only a limited range of phases around the synchronous phase, called the **bucket** or stable region (see [Longitudinal motion](#longitudinal)). For a synchronous phase of $-30°$ the stable region extends from about $-61°$ to $+30°$, about 91° wide, only a quarter of one period (360°). So if a DC beam is sent straight into cavities, at most a quarter can be "captured"; the rest are decelerated or fall out of step and eventually hit the wall. For a high-power proton beam such losses are both wasteful and dangerous: energetic protons hitting metal make it radioactive (**activation**), and the equipment can then no longer be serviced.

So before accelerating, the beam must be **bunched**: the protons spread evenly over a period must be gathered into the range of phases the stable region can hold. One rule here cannot be broken, **Liouville's theorem**: on the "time × energy" chart, the area occupied by a group of particles cannot shrink, just as a lump of modelling clay can be squashed and stretched but keeps its volume. If the range of phases is narrowed, the spread of energies must grow. How to narrow the phases without losing particles and without making the beam more "disordered" is the subject of the [RFQ](#journey-rfq) chapter.

Once formed, the bunches do not stay bunched by themselves. As in a race, the faster protons of a bunch run ahead and the slower ones fall behind, so the bunch lengthens in a drift (see the [Drift](#drift-longitudinal) chapter). The example beam has a relative momentum spread of 0.375 %, and a drift of only 1 m (without space charge) lengthens the rms bunch from 1.24 mm to 3.89 mm, and the phase width it occupies in the RF period from 4.3° to 13.3°. So the line between the RFQ and the superconducting section cannot be an empty pipe: bunching cavities are needed to shorten the bunches again, one of the tasks of the [medium-energy transport line](#journey-mebt).

## Between sections: matching {#journey-overview-matching}

In a relay race the next runner does not stand and wait; they start running early so that at the hand-over both runners move at the same speed. The sections of an accelerator need such a "hand-over" too.

Each section has its own focusing structure repeating cell after cell: the LEBT has a few solenoids, the RFQ an electric quadrupole field that alternates sign once per RF period, the low-energy part of the superconducting section "solenoid + cavity" cells, the high-energy part quadrupole doublets. In such a structure the beam swells and shrinks. If the incoming beam has just the right shape, its size repeats the same pattern in every cell; this is the **matched beam**. The beam shape is described by the Twiss parameters: $\beta$ reflects the size (larger means wider), and $\alpha$ whether the beam is converging ($\alpha > 0$) or diverging ($\alpha < 0$) (see [Phase space and emittance](#phase-space-twiss), [Linear optics](#linear-optics) and [Matching](#matching)).

The beam leaving one section is in general not the matched beam of the next. In the example, for instance, the transverse Twiss parameters at the RFQ exit are $\alpha_x = -0.658$, $\beta_x = 0.377$ mm/mrad, set by the RFQ structure, whereas the superconducting section asks for another set. Connected directly without treatment, the beam would "breathe" periodically in the next section, swelling and shrinking; with space charge, this mismatch transfers energy to a few particles and forms a **halo** far from the beam core, whose particles eventually hit the wall. The same holds longitudinally: bunch length and energy spread must suit the longitudinal focusing of the next section.

So there is a "hand-over" between every two sections: the final LEBT solenoid focuses the beam into the strongly converging beam the RFQ requires, the RFQ starts with a radial matching section, the MEBT does transverse and longitudinal matching with quadrupoles and bunchers, and the transitions between cavity families in the superconducting section are matched one by one. A rule of thumb for the design: particles oscillate about the centre line in a focusing channel, and the angle by which the oscillation advances per cell is the **phase advance per cell** (see [Quadrupole](#quadrupole-fodo)); the zero-current phase advance per cell should stay below 90° to avoid resonances driven by space charge, and the phase advance per metre should vary smoothly from section to section, without jumps (see [Lattice design](#lattice-design)).

## Why the frequency can double {#journey-overview-frequency}

In the superconducting section the frequency goes from 162.5 MHz to 325 MHz and then to 650 MHz. Why low frequency at low energy and high frequency at high energy?

**Low energy needs low frequency**, for three reasons:

- **The gaps would be too close**: at 650 MHz the $\beta\lambda/2$ of a 35 keV proton is only 2.0 mm, impossible to build.
- **The stable region would be too short**: the $-30°$ stable region is about 91° wide, 1.55 ns at 162.5 MHz but only 0.39 ns at 650 MHz, too short for a freshly formed, still fairly long bunch.
- **The RF defocusing would be too strong**: while the cavity squeezes the bunch longitudinally, it inevitably pushes the beam outwards transversely (an electric field cannot focus in all directions at once, for the same kind of reason as "cannot focus in both planes" in the [Quadrupole](#quadrupole-deep-field) chapter). The strength of this defocusing is proportional to the frequency and inversely proportional to $(\beta\gamma)^3$. $1/(\beta\gamma)^3$ is 5378 at 1.527 MeV, 319 at 10 MeV, 4.9 at 150 MeV and only 0.17 at 1 GeV: large at low energy, where a higher frequency could no longer be held in check.

**High energy can use high frequency** because the bunches become shorter in time. One way to see it: the closer the protons get to the speed of light, the smaller the velocity difference produced by a given energy difference, so the faster and slower protons in a bunch increasingly "run as one", and the amplitude of their longitudinal oscillation shrinks. Working from "time × energy area stays constant" (derivation in "[Going deeper: longitudinal oscillation and adiabatic phase damping](#journey-overview-deep-frequency)"), the phase width of the bunch shrinks during acceleration like $(\beta\gamma)^{-3/4}$; this is called **adiabatic phase damping**. Using the example RFQ exit and the assumptions listed in "Numbers along the way" below: the rms duration of the bunch shrinks from 73 ps at the RFQ exit to about 41 ps at the end of the HWR section, about 11 ps at the end of the spoke section, and only about 3 ps at 1 GeV. When the frequency doubles, the phase width in degrees doubles instantly (the period is half as long), e.g. from 2.4° to 4.8° at 10 MeV, still far below the 91° stable region, so the frequency can be changed; after longitudinal matching at the new frequency the phase width comes back to about 3.6°.

Why is a higher frequency worth having? Cavity size is proportional to the wavelength: the simplest cylindrical cavity (pillbox, TM010 mode) has a diameter of $2.405\,\lambda/\pi = 0.766\lambda$, 1.41 m at 162.5 MHz but only 0.35 m at 650 MHz. At high energy, smaller multi-cell elliptical cavities are more compact and can provide more accelerating voltage per unit length (typically), so both length and cost go down.

## Numbers along the way {#journey-overview-thread}

The table collects the state of our proton at the exit of each section. The last column is the main factor limiting the beam in that section, which explains why the next section changes structure.

| Section | Exit kinetic energy $W$ | $\beta$ | $\beta\gamma$ | $B\rho$ (T·m) | RF frequency | $\beta\lambda$ | rms bunch length | Main limitation |
|---|---|---|---|---|---|---|---|---|
| Ion source + LEBT | 35 keV | 0.00864 | 0.00864 | 0.0270 | DC | 15.9 mm (at 162.5 MHz) | continuous, unbunched | DC-beam space charge, source emittance |
| RFQ | 1.527 MeV | 0.05698 | 0.05708 | 0.1786 | 162.5 MHz | 15.9 → 105.1 mm | 1.24 mm (4.3°, 73 ps) | capture and bunching, low-energy space charge |
| MEBT | 1.527 MeV | 0.05698 | 0.05708 | 0.1786 | 162.5 MHz | 105.1 mm | held by bunchers (a 1 m drift would give 3.9 mm) | debunching, transverse/longitudinal matching |
| HWR section | ≈ 10 MeV | 0.1448 | 0.1464 | 0.458 | 162.5 MHz | 267 mm | ≈ 1.8 mm (2.4°, 41 ps) | transit-time factor, RF defocusing |
| Spoke section | ≈ 150 MeV | 0.5066 | 0.5876 | 1.839 | 325 MHz | 467 mm | ≈ 1.7 mm (1.3°, 11 ps) | longitudinal matching at the frequency jump, family transitions |
| Elliptical section | ≈ 1 GeV | 0.8750 | 1.8076 | 5.657 | 650 MHz | 404 mm | ≈ 0.8 mm (0.7°, 3 ps) | surface field, beam loss |
| HEBT and target | 1 GeV | 0.8750 | 1.8076 | 5.657 | — | — | — | beam loss (about 1 W/m), power density on target |

Some notes. $\beta$, $\beta\gamma$, the magnetic rigidity $B\rho$ and $\beta\lambda$ are exact kinematics. The **magnetic rigidity** $B\rho = p/q$ measures how "stiff" the beam is, how hard to bend with a magnetic field; numerically it is the magnetic field needed to bend the beam on a circle of 1 m radius (see [Relativistic kinematics](#kinematics-rigidity)). The boundary energies are typical values. The RFQ row comes from the example `part_rfq.dst` (which also has an rms energy spread of 11.4 keV, 0.75 %). The bunch lengths in the following rows are estimates: assuming a longitudinally matched bunch whose longitudinal rms emittance does not grow, using the adiabatic damping formula, with average accelerating gradients (cavities and focusing elements averaged together) $E_0T$ of 0.54 MV/m (HWR section, the value of the example "solenoid + drift + cavity" cell), 1 MV/m (spoke) and 3 MV/m (elliptical), and synchronous phases of −30°, −25° and −20°. The same formula at the RFQ exit gives 4.9°, close to the measured 4.3°, showing that the model captures the main physics. Since the bunch length depends only on the 1/4 power of the gradient, a factor of two in the gradient changes it by only about 19 %: with 6 MV/m in the elliptical section, for example, the result at 1 GeV is 0.7 mm (0.6°). In a real machine the longitudinal emittance always grows somewhat, so bunches are somewhat longer than in the table.

As for current: the example beam carries an average 0.184 mA (1.13 pC per bunch, about $7.1\times10^{6}$ protons), a beam power of 184 kW at 1 GeV; high-power CW proton linacs are usually designed for a few to a dozen or so milliamperes (typical values), and 10 mA corresponds to 10 MW. At 10 MW the common "no more than 1 W lost per metre" rule means losing no more than a fraction $10^{-7}$ per metre.

AVAS takes over at the RFQ exit. Below are the first few metres after the example: the solenoid map `sol` and the HWR map `hwr010` alternating for three cells, the structure built step by step in [Superconducting section](#journey-sc) and the [guided tutorial](#tutorial). Open the 3D view to see the beam squeezed in the solenoids and accelerated in the cavities.

```avas-beamline
drift 0.05 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010
drift 0.2 0.02 0
```

## Think about it {#journey-overview-think}

> **Think about it 1**: the example project uses a $\beta = 0.10$ HWR for protons with $\beta = 0.057$, where the transit-time factor is only half its best value. Is that a design mistake?
>
> Answer: no. A cavity family has to cover a range of velocities, so its first cavities always work below the design velocity and its last ones above. Every additional family means another cavity type and cryomodule to design, which is expensive, so one usually accepts lower efficiency at both ends of a family. Besides, a low transit-time factor only means "less energy per cavity"; it does not prevent stable acceleration.

> **Think about it 2**: if space charge weakens at higher energy, why not accelerate the protons to a few MeV directly in the ion source?
>
> Answer: electrostatic acceleration is limited by high-voltage breakdown, and DC high-voltage equipment above a few hundred kilovolts is large and hard to keep stable; moreover, the extracted current itself is limited by the Child–Langmuir law (see [Ion source](#journey-source-extraction)), so a higher voltage does not give a high-energy, high-current beam for free. In practice one extracts at a few tens of kilovolts and lets RF structures take over.

> **Think about it 3**: in the "Numbers along the way" table the rms bunch length in millimetres stays around 1–2 mm all the way, hardly changing. What then does "the bunches become shorter" mean?
>
> Answer: shorter in time and in RF phase. The protons move faster and faster, so the same 1.5 mm bunch passes a point in less time; what matters to a cavity is what fraction of the RF period the bunch occupies, so the bunch length in degrees or picoseconds is the quantity that decides whether the frequency can be changed.

> **Think about it 4**: from the HWR to the spoke cavities, the velocity grows from $\beta = 0.057$ to about 0.5 and needs two families; yet a single family of elliptical cavities takes the protons from 150 MeV to 1 GeV. Why can one family cover so much at high energy?
>
> Answer: because what matters is velocity, not energy. From 150 MeV to 1 GeV the energy grows almost sevenfold, but the velocity only from 0.507 to 0.875, less than a factor of 1.8, well within the roughly threefold useful range of one family.

## Common misconceptions {#journey-overview-misconceptions}

**"A cavity's design β is the velocity of the protons in it."** The design β only marks where the cavity works best: low-β cavities are usually labelled by the velocity of maximum $T(\beta)$, while the geometric β set by the gap spacing can be lower (about 0.088 for hwr010, see [Superconducting section](#journey-sc-ttf)). A family has to work over a fairly wide range on both sides: the example HWR has its maximum $T$ at $\beta \approx 0.10$, yet it is used for protons with $\beta = 0.057$.

**"Emittance is conserved all the way, so the beam keeps getting thinner."** What is conserved (ideally) is the normalized emittance. The geometric emittance $\varepsilon_n/(\beta\gamma)$ shrinks with acceleration: a normalized emittance of 0.14 π·mm·mrad corresponds to a geometric emittance of 16.2 π·mm·mrad at 35 keV, 2.45 at 1.527 MeV and 0.077 at 1 GeV. The beam getting thinner is this "adiabatic damping", not a smaller emittance. In a real machine, mismatch, nonlinear fields and space charge all make the normalized emittance grow section by section, and the transitions between sections are where things most easily go wrong.

**"The higher the energy, the harder the beam is to control."** For space charge it is the opposite: strongest at low energy, negligible at high energy. At high energy the difficulty becomes the beam power: for a 1 GeV, 10 mA beam, losing one part in a hundred thousand is 100 W.

**"When the frequency doubles, the bunch spacing halves."** No. The bunches were formed in the RFQ at 162.5 MHz and stay 6.15 ns apart; in the 325 MHz cavities there is simply one bunch every other RF period, and the other "bucket" is empty.

**"Getting $\beta\lambda/2$ right in every section is enough."** The gap spacing only keeps the reference proton in step. The other protons of the bunch stay in the stable region through longitudinal focusing (a negative synchronous phase), and longitudinal focusing inevitably comes with transverse RF defocusing, so the accelerating structure and the transverse focusing are designed together.

## Going deeper: the transit-time factor—why one cavity suits only a range of velocities {#journey-overview-deep-ttf}

The main text explained the transit-time factor $T(\beta)$ as "too slow or too fast costs a discount". This section computes it with a small two-gap model, using only the definition of work and the sum-to-product formula of trigonometry.

### Step 1: the energy from one gap

An electric field $E$ pushing a charge $q$ over a distance $g$ does work $qEg$ (school physics: "work = force × distance"). If the field is the same throughout the gap, $Eg$ is the voltage $V$ across the gap and the proton gains energy $qV$.

### Step 2: the field oscillates

An RF field changes in time: $E(t) = E_{\max}\cos(\omega t + \varphi)$ with $\omega = 2\pi f$. If the gap is very narrow, the proton crosses it in a flash and sees the value of the field at that moment, gaining $qV\cos\varphi$, where $\varphi$ is the phase at the crossing. The gain is largest for $\varphi = 0$ (the crest) and zero for $\varphi = \pm 90°$.

### Step 3: two gaps with opposite fields

Now put two very narrow gaps a distance $d$ apart, whose voltages at any instant are equal and opposite, of size $V_1$ (this is the π mode). The proton flies from the first gap to the second with velocity $v$ in a time $d/v$, during which the RF phase advances by

$$
\theta = \omega\,\frac{d}{v} = \frac{2\pi d}{\beta\lambda}.
$$

Let the phase be $\varphi$ when the proton is halfway between the gaps; then it crosses the first gap at phase $\varphi - \theta/2$ and the second at $\varphi + \theta/2$. Adding the two gains (the second gap's voltage is negative):

$$
\Delta W = qV_1\cos\!\left(\varphi - \tfrac{\theta}{2}\right) - qV_1\cos\!\left(\varphi + \tfrac{\theta}{2}\right) = 2qV_1\sin\varphi\,\sin\frac{\theta}{2},
$$

where the last step is the school sum-to-product formula $\cos A - \cos B = -2\sin\frac{A+B}{2}\sin\frac{A-B}{2}$.

### Step 4: read off the transit-time factor

The voltage amplitudes of the two gaps add up to $V_0 = 2V_1$. The expression above is largest for $\sin\varphi = \pm 1$, where it equals $qV_0\,|\sin(\theta/2)|$. Compared with the $qV_0$ obtainable "if the field did not oscillate", the discount is

$$
T(\beta) = \left|\sin\frac{\pi d}{\beta\lambda}\right| .
$$

(The formula contains $\sin\varphi$ rather than $\cos\varphi$ only because the zero of phase was put halfway between the gaps; moving the zero by 90° gives the $\cos\varphi_s$ form of the main text.)

This formula says everything in the main text's picture:

- For $d = \beta\lambda/2$, $T = 1$: flying $d$ takes exactly half a period, the field has just reversed, and both gaps push the proton. This defines the **geometric β**: $\beta_g = 2d/\lambda$.
- If the proton is twice as slow ($\beta = \beta_g/2$), $\theta/2 = \pi$ and $T = 0$: flying $d$ takes a whole period, the field in the second gap now has the "same sign" as in the first, and everything gained is taken back.
- If the proton is twice as fast ($\beta = 2\beta_g$), $T = \sin 45° = 0.71$: the field has not had time to reverse, and the second gap starts to cancel the first.

Check: the two main peaks of the on-axis field of the `hwr010` map are at $z = 0.074$ m and $0.140$ m. The field of a real gap is spread over several centimetres; taking the field-weighted gap centres ($0.065$ m and $0.145$ m) as the positions of the "lines", $d = 0.081$ m, so $\beta_g = 2 \times 0.081 / 1.845 = 0.088$, exactly the geometric β given in the [Superconducting section](#journey-sc-ttf) chapter. (Measuring between the two tips, 0.066 m, would give 0.072; for wide gaps the weighted centres are the right measure.)

### Step 5: real gaps have a width

The model treats each gap as a line. The field of a real gap is spread over a few centimetres, and a slow proton already experiences a large stretch of RF phase **within one gap**, which adds another discount. This extra discount is harsher for slow protons and milder for fast ones; multiplied in, it shifts the peak of $T$ towards higher velocity. That is why the actual $T$ of `hwr010` peaks at $\beta \approx 0.104$ (0.82) rather than at the geometric β of 0.088, and is only 0.40 at the example beam's $\beta = 0.057$. These numbers are obtained by putting the actual field of the map into the integral below.

### For readers who know calculus and Fourier transforms

With an on-axis field $E_z(z)\cos(\omega t + \varphi)$ and a particle crossing at constant velocity $\beta c$ ($t = z/(\beta c)$),

$$
\Delta W = q\int E_z(z)\cos\!\left(\frac{\omega z}{\beta c} + \varphi\right)\dd z = qV_0\,T(\beta)\cos\varphi_s, \qquad
T(\beta) = \frac{\left|\int E_z(z)\,\ee^{\,\mathrm{i}kz}\,\dd z\right|}{\int |E_z(z)|\,\dd z}, \quad k = \frac{2\pi}{\beta\lambda}.
$$

$T$ is the modulus of the Fourier transform of the on-axis field at wavenumber $k$ (normalised by $\int|E_z|\,\dd z$). Two delta-function gaps give the $|\sin(kd/2)|$ of Step 4; a gap of width $g$ multiplies this by a single-gap factor ($\sin(kg/2)/(kg/2)$ for a uniform field), which tends to 1 as $\beta$ grows, hence the peak shift of Step 5. The gain in the lab of the main text also includes the change of velocity inside the cavity, so it differs slightly from the constant-velocity approximation. Full derivations in [Longitudinal motion](#longitudinal-gap) and [RF cavity](#rf-cavity).

## Going deeper: how strong is space charge—from Coulomb's law to the envelope equation {#journey-overview-deep-space-charge}

The main text said that space charge "grows like $1/(\beta\gamma)^3$" and gave the perveance $K$. This section starts from school-level electric and magnetic fields, derives $K$ step by step, and then the envelope equation used to compare it with emittance.

### Step 1: how much charge per unit length

The beam current $I$ is the charge passing per second. The charge that passes in one second is spread along the pipe over the distance the protons fly in one second, $v$, so the charge per unit length (the **line charge density**) is

$$
\lambda_q = \frac{I}{v}.
$$

A 10 mA beam at 35 keV ($v = 2.59\times10^6$ m/s) has $\lambda_q = 3.9\times10^{-9}$ C/m; at 1.527 MeV ($v = 1.71\times10^7$ m/s) only 1/6.6 of that. This is "slow means crowded" in the main text.

### Step 2: the electric field inside a uniform round beam

The field of a point charge, $E = Q/(4\pi\varepsilon_0 r^2)$, can be understood like this: field lines leave the charge and pass evenly through a sphere of radius $r$; the larger the sphere's area $4\pi r^2$, the sparser the lines. For a long charged line the field lines spread out flat in all directions and pass through the side of a cylinder, whose area per unit length is $2\pi r$, so

$$
E_r = \frac{\lambda_{\text{in}}}{2\pi\varepsilon_0 r},
$$

where $\lambda_{\text{in}}$ is the line charge density inside radius $r$. In a uniform round beam of radius $a$, the charge inside radius $r$ is a fraction $r^2/a^2$ of the total, so inside the beam

$$
E_r = \frac{\lambda_q\,r}{2\pi\varepsilon_0 a^2},
$$

a field proportional to the distance $r$ from the axis, like a spring pushing outwards.

### Step 3: magnetic attraction cancels part of it

Moving charges are a current. The current $I_{\text{in}}$ inside radius $r$ produces at radius $r$ a magnetic field $B = \mu_0 I_{\text{in}}/(2\pi r)$ (the school formula for a long straight wire), and a proton moving with velocity $v$ feels an inward Lorentz force $qvB$ (parallel currents attract). Since $I_{\text{in}} = \lambda_{\text{in}} v$, the ratio of magnetic to electric force is

$$
\frac{qvB}{qE_r} = \mu_0\varepsilon_0 v^2 = \frac{v^2}{c^2} = \beta^2,
$$

using $\mu_0\varepsilon_0 = 1/c^2$. So the net outward force is $F = qE_r(1 - \beta^2) = qE_r/\gamma^2$. At 35 keV, $\beta^2 = 7.5\times10^{-5}$ and the magnetic force is negligible; at 1 GeV, $\beta^2 = 0.77$ and the net force is only 23 % of the electric one.

### Step 4: the force bends the trajectory

As in the [Quadrupole](#quadrupole-deep-matrix) chapter: a sideways force $F$ acting for a time $\Delta t$ changes the sideways momentum by $F\Delta t$ and the angle by $F\Delta t/p$; covering a distance $\Delta s$ takes $\Delta t = \Delta s/v$. So the "bending" of the trajectory (the rate of change of the angle with distance) is

$$
r'' = \frac{F}{p\,v} = \frac{F}{\gamma m v^2}.
$$

### Step 5: the particle at the beam edge

Putting the force of Steps 2 and 3 at the beam edge $r = a$ into Step 4:

$$
a'' = \frac{q\lambda_q}{2\pi\varepsilon_0\,a\,\gamma^2}\cdot\frac{1}{\gamma m v^2} = \frac{qI}{2\pi\varepsilon_0\,m\,v^3\gamma^3}\cdot\frac1a = \frac{K}{a},
\qquad
K = \frac{qI}{2\pi\varepsilon_0\, m c^3 \beta^3\gamma^3} = \frac{2I}{I_0\,(\beta\gamma)^3}.
$$

The $(\beta\gamma)^3$ in the denominator is exactly the main text's three reasons: $1/\beta$ from Step 1 (slow means crowded), $1/(\gamma\beta^2)$ from Step 4 (slow means easy to push), $1/\gamma^2$ from Step 3 (magnetic cancellation). The constant $I_0 = 4\pi\varepsilon_0 mc^3/q \approx 31.3$ MA is fixed for protons. Check: at 35 keV, $(\beta\gamma)^3 = 0.008637^3 = 6.44\times10^{-7}$, and 10 mA gives $K = 2 \times 0.01/(3.13\times10^7 \times 6.44\times10^{-7}) = 9.9\times10^{-4}$, as in the main text.

### Step 6: emittance "pushes outwards" too

The [Drift](#drift-waist) chapter has a Pythagoras-like formula: starting from a waist and covering a distance $d$, the rms size is $\sigma^2 = \sigma_w^2 + (\sigma' d)^2$. For small $d$, using the approximation $\sqrt{1+u} \approx 1 + u/2$:

$$
\sigma \approx \sigma_w + \frac{\sigma'^2}{2\sigma_w}\,d^2 .
$$

Comparing with uniformly accelerated motion, $s = \tfrac12 a t^2$, the "acceleration" of the beam size is $\sigma'' = \sigma'^2/\sigma_w$. At the waist $\sigma_w\sigma' = \varepsilon$ (the emittance), so $\sigma'' = \varepsilon^2/\sigma_w^3$. Without any force, the beam envelope still bends outwards with strength $\varepsilon^2/a^3$. Putting both terms together (and adding the external focusing $-k^2 a$) gives the **envelope equation**:

$$
a'' = \frac{K}{a} + \frac{\varepsilon^2}{a^3} - k^2 a .
$$

When $a$ is the beam edge radius, $\varepsilon$ must be the edge emittance (4 times the rms value for a uniform beam). Checking the main text's comparison: 35 keV, 10 mA, $a = 5$ mm; a normalized rms emittance of 0.14 π·mm·mrad corresponds to an edge geometric emittance of $4 \times 0.14 / 0.008637 = 64.8$ mm·mrad (the π is just a unit marker saying the ellipse area has been divided by π). $K/a = 9.9\times10^{-4}/0.005 = 0.198$ m⁻¹ and $\varepsilon^2/a^3 = (64.8\times10^{-6})^2/(0.005)^3 = 0.034$ m⁻¹: the former is about 6 times the latter.

### For readers who know vector calculus

Steps 2 and 3 are Gauss's law and Ampère's law applied to a uniform round beam: $E_r = Ir/(2\pi\varepsilon_0 a^2\beta c)$, $B_\theta = (\beta/c)E_r$, $F_r = q(E_r - \beta cB_\theta) = qE_r/\gamma^2$. The equation of Step 6 is the Kapchinsky–Vladimirsky (KV) envelope equation, exact for a KV distribution; for other distributions, written in rms quantities, it becomes $\sigma'' = K/(4\sigma) + \varepsilon_{\mathrm{rms}}^2/\sigma^3 - k^2\sigma$ (Sacherer's rms envelope equation, whose space-charge term depends only weakly on the shape of the distribution). The full field and envelope-equation derivation is in [Space charge](#space-charge-perveance).

## Going deeper: longitudinal oscillation, RF defocusing and adiabatic phase damping {#journey-overview-deep-frequency}

The main text said that "the phase width of the bunch shrinks like $(\beta\gamma)^{-3/4}$" and that RF defocusing is "proportional to the frequency and inversely proportional to $(\beta\gamma)^3$". This section derives both statements in six steps, using derivatives, simple harmonic motion and "the area stays constant".

### Step 1: an early proton gets less energy

Let $\Delta\varphi$ be how much later in phase a proton arrives than the reference proton ($\Delta\varphi > 0$ means late), and $\Delta W$ how much more energy it has. In a structure with average accelerating gradient $E_0T$, the reference proton gains $qE_0T\cos\varphi_s$ per metre and this proton $qE_0T\cos(\varphi_s + \Delta\varphi)$. Take $\varphi_s = -30°$: a proton 30° early ($\Delta\varphi = -30°$) meets $\cos(-60°) = 0.5$, less than the reference proton's $\cos(-30°) = 0.866$; a late one meets a field closer to the crest and gains more. For small $\Delta\varphi$, using the derivative $(\cos x)' = -\sin x$:

$$
\frac{\dd\,\Delta W}{\dd s} = qE_0T\left[\cos(\varphi_s + \Delta\varphi) - \cos\varphi_s\right] \approx qE_0T\sin(-\varphi_s)\,\Delta\varphi .
$$

For $\varphi_s < 0$, $\sin(-\varphi_s) > 0$: late protons gain more energy, early ones less. This is exactly why the synchronous phase is chosen negative.

### Step 2: the extra energy makes it arrive earlier

More energy means faster, and faster means earlier. The [Drift](#drift-deep-longitudinal) chapter derived the velocity difference $\Delta v/v = \delta/\gamma^2$, with momentum difference $\delta = \Delta p/p = \Delta W/(\beta^2\gamma mc^2)$. Per metre, the faster proton arrives $\Delta v/v^2$ seconds earlier; in phase, multiply by $\omega = 2\pi\beta c/(\beta\lambda)$:

$$
\frac{\dd\,\Delta\varphi}{\dd s} = -\frac{2\pi}{\beta\lambda}\cdot\frac{\Delta v}{v} = -\frac{2\pi\,\Delta W}{mc^2\,\beta^3\gamma^3\,\lambda}.
$$

$\beta^3\gamma^3$ appears again: the closer the proton gets to the speed of light, the more slowly an energy difference turns into a phase difference.

### Step 3: together, simple harmonic motion

Differentiating Step 2 once more with respect to $s$ and inserting Step 1:

$$
\frac{\dd^2\Delta\varphi}{\dd s^2} = -k_l^2\,\Delta\varphi, \qquad k_l^2 = \frac{2\pi qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3}.
$$

This is the same equation as a mass on a spring, $x'' = -\omega^2 x$: the protons of a bunch oscillate about the reference proton, by $k_l$ radians per metre. Check at the RFQ exit ($E_0T = 0.55$ MV/m, $\varphi_s = -30°$, $\beta^3\gamma^3 = 1.86\times10^{-4}$): $k_l = 2.3$ m⁻¹, so one oscillation takes $2\pi/k_l = 2.7$ m. The [RFQ](#journey-rfq-bunching) chapter uses this number.

### Step 4: longitudinal focusing inevitably brings transverse defocusing

In vacuum, field lines cannot appear or vanish out of nothing: for a small region of space, the field lines "flowing in" along $x$, $y$ and $z$ must balance those "flowing out". The longitudinal focusing of Step 1 means the field pushes particles along $z$ towards the reference proton; then transversely the field lines must spread out and push particles outwards (the same kind of reason as "a magnetic field cannot focus in both planes" in the [Quadrupole](#quadrupole-deep-field) chapter). The two transverse planes share this defocusing, each getting half the longitudinal strength:

$$
k_r^2 = \frac{k_l^2}{2} = \frac{\pi qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3}.
$$

It is proportional to $1/\lambda$ (the frequency) and inversely proportional to $(\beta\gamma)^3$: the main text's third reason.

### Step 5: the "shape" of a matched bunch

In the simple harmonic motion $\Delta\varphi = \Phi\cos k_l s$, the phase amplitude $\Phi$ and energy amplitude $W_a$ are linked by Step 2: differentiating $\Delta\varphi$ gives $\Phi k_l = 2\pi W_a/(mc^2\beta^3\gamma^3\lambda)$. Inserting $k_l$ gives the ratio of energy spread to phase width of a matched bunch:

$$
\frac{\sigma_W}{\sigma_\varphi} = \frac{W_a}{\Phi} = \sqrt{\frac{qE_0T\sin(-\varphi_s)\,\lambda\,mc^2\,\beta^3\gamma^3}{2\pi}} .
$$

Check at the RFQ exit: $\sqrt{0.275 \times 1.845 \times 938.272 \times 1.86\times10^{-4}/(2\pi)} = 0.119$ MeV/rad, i.e. 2.07 keV per degree.

### Step 6: constant area gives damping

When acceleration is slow (parameters change only a little per oscillation), the bunch's rms area on the $(\Delta\varphi, \Delta W)$ chart, $\varepsilon_{\varphi W} = \sigma_\varphi\sigma_W$, stays constant. Hence

$$
\sigma_\varphi^2 = \frac{\varepsilon_{\varphi W}}{\sigma_W/\sigma_\varphi} \propto \frac{1}{(\beta\gamma)^{3/2}}, \qquad \sigma_\varphi \propto (\beta\gamma)^{-3/4}.
$$

Check: the longitudinal rms emittance at the example RFQ exit is 0.83 ns·keV, i.e. 48.7°·keV at 162.5 MHz, giving $\sigma_\varphi = \sqrt{48.7/2.07} = 4.9°$, against 4.3° measured; at 10 MeV ($E_0T = 0.54$ MV/m) it is 2.4°. At 325 MHz the period is half as long, so the same duration corresponds to twice as many degrees, 4.8°; after re-matching at the new frequency it is about 3.6°. The bunch length depends only on the 1/4 power of the gradient (Steps 5 and 6), so errors in the assumed gradient matter little for the estimate.

### For readers who know Hamiltonian mechanics

$(\Delta\varphi, \Delta W)$ are a canonically conjugate pair, and the Hamiltonian $H = -\frac{\pi\,\Delta W^2}{mc^2\beta^3\gamma^3\lambda} - qE_0T\left[\sin(\varphi_s + \Delta\varphi) - \Delta\varphi\cos\varphi_s\right]$ gives the full nonlinear equations of Steps 1 and 2; its separatrix is the main text's stable region (phases from $-61°$ to $+30°$ for $\varphi_s = -30°$). For slowly varying parameters, the action $\oint \Delta W\,\dd\Delta\varphi$ is an adiabatic invariant, and Step 6 is its small-amplitude form. $k_r^2 = k_l^2/2$ follows from $\nabla\cdot\boldsymbol E = 0$ in vacuum applied to the time-averaged RF field (a dynamic version of Earnshaw's theorem). See [Longitudinal motion](#longitudinal).

## In AVAS {#journey-overview-in-avas}

### Which sections AVAS covers

AVAS is a multi-particle tracking code that starts from a given input beam (see [Accelerators and AVAS](#intro)). Its elements are drifts, quadrupoles, solenoids, dipoles and pole faces, steerers, RF / electrostatic / magnetostatic field-map elements and superposed fields. Against the route above:

| Section | Simulated by AVAS? | Notes |
|---|---|---|
| Ion source, LEBT | No | AVAS does not simulate this section; see [Ion source](#journey-source-in-avas) |
| RFQ | Usually not | Normally computed with dedicated codes; its exit beam is handed to AVAS as a particle file or Twiss parameters. The new kernel's default mode also has an `rfq` element computed cell by cell, not yet checked against other codes; see [RFQ](#journey-rfq-in-avas) |
| MEBT | Yes | quadrupoles `quad`, solenoids, buncher field maps `field … 1 …`, drifts, steerers |
| Room-temperature DTL | No (no dedicated model) | with a 3D field map it could in principle be read as a `field` element; not discussed in this primer |
| Superconducting section | Yes | cavity field maps `field … 1 …`, focusing with solenoid maps `field … 3 …` or matrix elements `solenoid`, `quad` |
| HEBT | Partly | `quad`, `bend` + `edge`, `steerer`; targets, nonlinear magnets and scanning magnets are not among AVAS's elements |

> **AVAS convention**: in multi-particle runs the first and last elements of the lattice should not be matrix elements (a requirement of the manual), so the beam line above starts and ends with short drifts.

### Handing the beam to AVAS

AVAS's initial beam can be given in two ways, both in `beam.txt`:

- Read a particle file: `readparticledistribution <file>` names a `.dst` / `.edst` file (e.g. `part_rfq.dst` written by the RFQ code); the interface switches with `use_dst 1`. When a file is read, all `beam.txt` keywords except `numofcharge` are ignored.
- Generate from parameters: give the particle species, kinetic energy, current, beam frequency, distribution type and Twiss parameters in the three planes, and AVAS generates the macro-particles. The example project does this: `use_dst 0`, with transverse Twiss parameters equal to the rms values computed from `part_rfq.dst` and `GS` distributions transversely and longitudinally.

```avas-params
beam:readparticledistribution beam:use_dst beam:kneticenergy beam:current beam:frequency beam:twissx beam:twissz beam:distribution
```

> **AVAS convention**: in `twissx` / `twissy`, β is in mm/mrad and the emittance is the **normalized rms** emittance (π·mm·mrad); `twissz` uses the same units with the longitudinal angle coordinate $z' = \Delta p/p$. The example's 0.14 π·mm·mrad is a normalized rms value.

> **AVAS convention**: `frequency` in `beam.txt` is the beam frequency; the charge per bunch is $Q = I/f$. Each cavity's RF frequency is written in its own `field` line. How the kernel handles a beam frequency different from the cavity frequency (e.g. one bunch in every other "bucket" of a 325 MHz cavity) is not stated in the manual; the kernel is authoritative.

### The example project and this chapter's formulas

The `beam.txt` of `examples/hwr010` gives a 1.527 MeV, 0.184 mA, 162.5 MHz proton beam with 5260 macro-particles; the lattice has a single HWR field map. All numbers in this chapter can be checked on the example: read the parameters from `part_rfq.dst` on the Beam page, or extend it to several cells in the [guided tutorial](#tutorial).

All formulas quoted in the main text are in the deep blocks above: the Fourier integral for the transit-time factor in the constant-velocity approximation at the end of "[Going deeper: the transit-time factor](#journey-overview-deep-ttf)"; the space-charge fields $E_r$, $B_\theta$ of a uniform round beam and the perveance in "[Going deeper: how strong is space charge](#journey-overview-deep-space-charge)"; the longitudinal wavenumber $k_l$, the RF defocusing $k_r$ and the matched-bunch $\sigma_W/\sigma_\varphi$ in "[Going deeper: longitudinal oscillation](#journey-overview-deep-frequency)". Full derivations in [Longitudinal motion](#longitudinal-gap) and [Space charge](#space-charge-perveance).

## Further reading {#journey-overview-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: the linac as a whole, transit-time factor, longitudinal motion and adiabatic phase damping, RFQs and superconducting linacs are all treated systematically; this chapter's formulas agree with it.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: space charge, perveance and envelope equations.
- The CERN Accelerator School (CAS) lecture notes on linacs and RF.
- AVAS user manual `docs/使用说明20260427.docx`: `beam.txt` keywords and particle files.
