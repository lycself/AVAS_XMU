# Bunching: the RFQ turns a continuous beam into bunches

When it leaves the low-energy transport, our proton is still part of a continuous 35 keV beam, with the protons ahead of and behind it spread uniformly in time. A few metres later it sits in a bunch with an rms length of about 1.2 mm, travelling with its companions at the 162.5 MHz rhythm, with a kinetic energy of 1.527 MeV. The radio-frequency quadrupole (RFQ) makes this change. It has to do three things at once that seem to contradict each other: hold the beam tightly where space charge is strongest; cut the continuous beam into bunches while losing almost no particles; and accelerate. Why can a buncher cavity followed by ordinary accelerating cavities not do the job? Why does the RFQ focus with electric rather than magnetic fields? Why are its electrode surfaces rippled? This chapter answers these questions one by one. The background is in [ion source and low-energy transport](#journey-source) and the [overview](#journey-overview-bunching); the basics of longitudinal motion are in [longitudinal motion](#longitudinal).

> **AVAS does not simulate this section.** The RFQ is computed with dedicated design codes; its output particle distribution (`part_rfq.dst` in the example) or the Twiss parameters computed from it are AVAS's initial beam, as described in [In AVAS](#journey-rfq-in-avas) below.

## A continuous beam meets an RF cavity {#journey-rfq-dc-problem}

Start with a thought experiment: send the continuous 35 keV beam straight into a string of ordinary 162.5 MHz accelerating cavities. What happens?

Each proton reaches the first gap at a random RF phase, uniformly spread over 360°. With the energy gain $\Delta W = qV_0T\cos\varphi$, half of the protons are accelerated and half decelerated, and the average gain is zero. In the next cavities, protons with a suitable phase keep being accelerated, while the others are decelerated or even reversed, or slowly slip away because their velocity differs from that of the synchronous particle. Only the part that falls inside the longitudinal stable region can follow the synchronous particle and be accelerated stably. With a synchronous phase of −30° the stable region is about 91° wide in phase (see the [overview](#journey-overview-bunching)), so at most a quarter of the protons can be captured, and that is before the limited height of the stable region in energy is taken into account.

The other three quarters do not vanish at once: they travel some way with the wrong energy and then hit the wall. For a 10 mA proton accelerator this means several milliamperes lost at a few MeV, an unacceptable source of activation and heat load. So the continuous beam has to be bunched first, and the bunching itself must lose almost nothing.

## Bunching slowly {#journey-rfq-bunching}

The most direct way to bunch is a buncher cavity followed by a drift: the buncher gives protons of different phase different velocity corrections, slowing down the early ones and speeding up the late ones, so that after a drift they converge at the same moment. Unfortunately the buncher voltage is sinusoidal, and the correction is close to linear only near the centre; protons far from the centre are under-corrected and do not converge. An ideal estimate (kinematics only, no space charge and no energy spread): with the best buncher voltage about 74 % of the protons end up within ±45°, so about a quarter remains outside. Moreover, the converged bunch is a bent S shape in longitudinal phase space, with an rms emittance far larger than that of the original DC beam. At 35 keV, with space charge dominating, things are worse still: as soon as the bunch is compressed, the peak current rises many times (about 34 times the average current at the RFQ exit, see the [overview](#journey-overview-space-charge)), and space charge pushes the converging bunch apart again.

The RFQ's answer is **adiabatic bunching**. Picture a train of "potential wells" moving along with the protons. At first the wells are shallow and wide, so wide that together they cover the whole RF period and every proton sits in some well. Then the wells are made slowly deeper and narrower along the accelerator. If the change is slow enough, each proton follows its own well; as the well narrows, the amplitude of its phase oscillation shrinks too, and finally the whole group of protons is gathered into a narrow bunch.

We must say where this analogy fails. A ball settles at the bottom of a bowl through friction, which removes its energy; a beam has no friction, and its phase-space area is strictly conserved (Liouville's theorem). The phase range of the protons shrinks because the frequency of the longitudinal oscillation rises as the well deepens, and what is conserved under slow change is the action (oscillation energy divided by frequency), so the amplitude decreases; the price is a larger energy spread. The phase is squeezed from 360° to a few degrees, the energy spread grows accordingly, and the phase-space area stays the same.

"Slow enough" has a definite scale: during one longitudinal oscillation the shape of the well may change only a little. With the wave number of small-amplitude longitudinal oscillations (see [longitudinal motion](#longitudinal))

$$
k_l^2 = \frac{2\pi qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3}
$$

we can estimate the situation near the RFQ exit ($E_0T = 0.55$ MV/m and $\varphi_s = -30°$, derived further below): $k_l = 2.3$ m⁻¹, so one longitudinal oscillation takes 2.7 m, about 51 cells. Adiabatic bunching has to be spread over many oscillation periods, which is one reason RFQs are often several metres long.

Following the classic design approach developed at Los Alamos, an RFQ is divided longitudinally into four sections:

- **Radial matching section**: over a few cells the transverse focusing rises from weak to strong, taking the convergent beam from the LEBT into the alternating RFQ focusing; there is no longitudinal field yet.
- **Shaper**: the longitudinal field rises slowly from zero, with the synchronous phase starting at −90°. The stable region at −90° covers the whole period without accelerating, and the protons begin to gather in phase.
- **Gentle buncher**: the synchronous phase moves gradually towards about −30° and the longitudinal field keeps growing; it is designed to keep the small-amplitude longitudinal oscillation frequency and the spatial bunch length roughly constant, so that space charge does not become suddenly stronger anywhere.
- **Accelerator**: the synchronous phase and the electrode parameters are nearly fixed, and the bunch is accelerated to the exit energy in a nearly uniform way.

RFQs designed this way typically reach transmissions above 95 %, much higher than a single buncher cavity.

> **Think about it**: if adiabatic bunching is so good, why not do another "slow bunching" after 1.5 MeV with ordinary superconducting cavities?
> Answer: it is neither needed nor possible. At the RFQ exit the bunches already exist; afterwards they only have to be kept (which is what the MEBT bunchers do). And superconducting cavities are discrete: each has a fixed field profile of a few gaps, and neighbouring cavities are separated by drifts and focusing elements, so the longitudinal focusing arrives in kicks spaced by tens of centimetres, whereas the RFQ modulates its field continuously, cell after cell ($\beta\lambda/2$). A string of such cavities cannot start from a vanishing field covering the full 360° of phase and deepen the wells smoothly. The RFQ owes this ability to its unique electrode structure, which the next two sections describe.

## Electric quadrupole focusing {#journey-rfq-focusing}

First the transverse plane. At 35 keV space charge is strongest and very strong focusing is needed. Why does the RFQ use electric rather than magnetic fields?

Compare the two forces on a proton: the electric force $qE$ does not depend on velocity, while the magnetic force $qvB$ is proportional to it. An electric quadrupole lens with gradient $E'$ (in V/m²) is therefore equivalent to a magnetic quadrupole with gradient

$$
G_{\mathrm{eq}} = \frac{E'}{v}
$$

The slower the proton, the better the deal offered by electric lenses. The four electrodes (vanes) of an RFQ run along the beam; opposite vanes have the same polarity and neighbouring ones opposite polarity, with the RF voltage $V$ between neighbouring vanes. Near the axis the potential is approximately $U = \frac{V}{2}\frac{x^2 - y^2}{a^2}\sin(\omega t + \varphi)$ ($a$ is the distance from the vane to the axis), giving a field-gradient amplitude $E' = V/a^2$.

Take a typical order of magnitude, $V = 65$ kV and $a = 6$ mm (not the parameters of a particular RFQ): $E' = 1.8\times10^{9}$ V/m². A 35 keV proton moves at $2.59\times10^{6}$ m/s, so the equivalent magnetic gradient is 697 T/m; at 1.527 MeV ($1.71\times10^{7}$ m/s) it is 106 T/m. A magnetic quadrupole with a 6 mm aperture and a gradient of several hundred T/m would need a pole field of several tesla, and it could only be a few millimetres long: the focusing period here is just $\beta\lambda = 15.9$ mm, so each lens would be 8 mm long, shorter than its aperture, and its fringe fields could not be separated at all.

This leads to the second clever point of RFQ focusing: the alternation comes from time. The vane voltage changes sign every half RF period, so the quadrupole field at a given place switches between focusing and defocusing; and in one RF period the proton travels exactly $\beta\lambda$. To the proton, therefore, it passes through an FD alternating-gradient structure of period $\beta\lambda$, while the electrodes themselves are continuous in $z$, with no need to line up separate lenses.

How strong is the focusing? In the non-relativistic approximation the transverse equation of motion is

$$
\frac{\dd^2x}{\dd s^2} = \frac{qE_x}{mv^2} = -\frac{qV}{mc^2\beta^2a^2}\sin\!\left(\frac{2\pi s}{\beta\lambda} + \varphi\right)x
$$

Changing the independent variable to $\tau = s/(\beta\lambda)$, measured in periods, cancels the $\beta^2$ in the coefficient:

$$
\frac{\dd^2x}{\dd\tau^2} = -B\sin(2\pi\tau + \varphi)\,x, \qquad B = \frac{qV\lambda^2}{mc^2a^2}
$$

$B$ is the focusing parameter of the RFQ, and it does not depend on the proton velocity: this is the quantitative form of "electric focusing does not weaken at low velocity". Treat the alternating term in the smooth approximation: the rapidly oscillating force $B\sin(2\pi\tau)$ makes the particle position jitter slightly, and the product of jitter and force averages to a net restoring force whose strength is the mean square of $\int B\sin$, i.e. $B^2/(8\pi^2)$. The zero-current phase advance per period is then

$$
\sigma_0 \approx \frac{B}{2\sqrt2\,\pi}
$$

(RF defocusing reduces it somewhat; it is neglected here). The parameters above give $B = 6.55$ and $\sigma_0 \approx 42°$; with $a = 5$ mm, $B = 9.43$ and $\sigma_0 \approx 61°$. These values hold for unmodulated vanes. Strictly, $B$ carries the focusing efficiency $X$ of the potential in the next section, $B = qXV\lambda^2/(mc^2a^2)$: $X = 1$ without modulation (the radial matching section and the start of the shaper), but with a modulation $m = 2$ at 1.527 MeV ($a = 6$ mm) $X$ is only 0.41. Real designs therefore adjust $a$ and $V$ along the RFQ to keep $\sigma_0$ roughly constant. If $\sigma_0 \approx 42°$ is kept, the smooth-approximation matched β function, about $\beta\lambda/\sigma_0$, is 0.022 mm/mrad at 35 keV and 0.14 mm/mrad at 1.527 MeV. This is why the [LEBT](#journey-source-matching) has to converge the beam strongly into the RFQ.

How high can the voltage be? The field on the vane surface must stay below breakdown. The empirical yardstick for RF breakdown is the Kilpatrick criterion $f = 1.64E_K^2\,\ee^{-8.5/E_K}$ ($f$ in MHz, $E_K$ in MV/m), which gives $E_K = 13.6$ MV/m at 162.5 MHz. The peak surface field is somewhat higher than $V/a$ (depending on the curvature of the vane tip), and RFQ designs typically use 1.2–1.8 times the Kilpatrick field for the peak surface field, with CW RFQs on the conservative side. Above, $V/a = 10.8$ MV/m is only 0.8 times the Kilpatrick field; with a typical tip enhancement of 1.3–1.4 the peak surface field is about 1.0–1.1 times the Kilpatrick field, at or slightly below the lower end of that range, a conservative CW-style choice.

> **Think about it**: if electric quadrupole focusing is so good, why not keep using electric quadrupoles after the RFQ?
> Answer: the equivalent magnetic gradient of an electric lens, $E'/v$, falls with velocity: from 35 keV to 1.527 MeV it already drops from 697 T/m to 106 T/m and keeps falling afterwards, while magnetic lenses do not weaken with velocity. The RFQ's accelerating efficiency also falls with velocity (next section), so at a few MeV a structure of "magnetic focusing + high-gradient cavities" becomes more effective.

## Vane modulation: a longitudinal field grown from a quadrupole {#journey-rfq-modulation}

A pure quadrupole field has only transverse components; it can neither accelerate nor bunch. The RFQ's trick is to make the vane surfaces undulate along $z$: where the horizontal pair comes close to the axis, the vertical pair moves away, and half a period later the other way round. The period of the undulation is $\beta\lambda$, and each half period ($\beta\lambda/2$) is called a cell, so the RFQ cell length grows with the proton velocity: 8.0 mm at the 35 keV entrance and 52.6 mm at the 1.527 MeV exit.

Why does the undulation create a longitudinal field? In one cell the horizontal vanes are closer to the axis, so the potential on axis leans towards that of the horizontal vanes; in the next cell the vertical vanes are closer, and the potential on axis leans towards the other pole. Neighbouring cells therefore have an axial potential difference, and hence an axial electric field. The lowest-order potential is a sum of two terms (the Kapchinsky–Teplyakov form):

$$
U = \frac{V}{2}\left[X\left(\frac{r}{a}\right)^2\cos2\theta + A\,I_0(kr)\cos kz\right]\sin(\omega t + \varphi), \qquad k = \frac{2\pi}{\beta\lambda}
$$

The first term is the quadrupole focusing term, the second the accelerating term, and $I_0$ is the modified Bessel function of order zero. $A$ and $X$ are set by the minimum vane aperture $a$ and the modulation factor $m$ (maximum aperture $ma$):

$$
A = \frac{m^2 - 1}{m^2 I_0(ka) + I_0(mka)}, \qquad X = 1 - A\,I_0(ka)
$$

Differentiating with respect to $z$ gives the longitudinal field on axis, $E_z = \frac{kAV}{2}\sin kz\,\sin(\omega t + \varphi)$. Its average amplitude over a cell is $E_0 = 2AV/(\beta\lambda)$ and its transit-time factor $T = \pi/4$, so the energy gain per cell is

$$
\Delta W_{\mathrm{cell}} = qE_0T\cos\varphi_s\cdot\frac{\beta\lambda}{2} = \frac{\pi}{4}\,qAV\cos\varphi_s
$$

Two points about this result. First, the energy gain per cell does not depend on the cell length, only on $A$ and $V$; but the average accelerating gradient $E_0T = \pi AV/(2\beta\lambda)$ falls as $1/\beta$ with increasing velocity. Second, $A$ depends on $ka = 2\pi a/(\beta\lambda)$: at low velocity the cells are short, not much longer than the aperture, $ka$ is large, $I_0(ka)$ is large and $A$ is small; the longitudinal field is "hidden" near the vanes and hardly reaches the axis.

With $m = 2$, $a = 6$ mm, $V = 65$ kV and $\varphi_s = -30°$: at 35 keV, $ka = 2.37$, $A = 0.090$ and the gain per cell is only 4.0 keV; at 0.3 MeV, $A = 0.47$ and 20.6 keV; at 1.527 MeV, $ka = 0.36$, $A = 0.57$, 25.2 keV, and an average gradient $E_0T = 0.55$ MV/m. This explains the division of labour in the RFQ: at low energy $A$ is naturally small, just right for gentle shaping and bunching; at high energy $A$ is close to saturation while the average gradient falls as $1/\beta$, so accelerating further in the RFQ pays off less and less, and the beam is handed to the next section at around 1.5–3 MeV (typical values).

> **Think about it**: the RFQ cell length is $\beta\lambda/2$ while the focusing period is $\beta\lambda$. Why the factor of two?
> Answer: focusing comes from the time alternation of the voltage, positive and negative once each per RF period, which makes one FD period of length $\beta\lambda$, the distance the proton travels in one period. Acceleration comes from the vane undulation: neighbouring cells have opposite longitudinal fields, and every time the proton advances half a period, $\beta\lambda/2$, the RF field also changes sign, so every cell accelerates. This is the same reason why the gap spacing of a π-mode accelerating structure is $\beta\lambda/2$.

## The beam at the exit {#journey-rfq-output}

The example project's `part_rfq.dst` is the particle distribution at the exit of a 162.5 MHz RFQ. Statistics of its 5260 macro-particles:

- mean kinetic energy 1.527 MeV ($\beta = 0.05698$), rms energy spread 11.4 keV (0.75 %), relative momentum spread 0.375 %;
- current 0.184 mA, 1.13 pC per bunch, about $7.1\times10^{6}$ protons;
- rms phase width 4.26°, i.e. an rms bunch length of 1.24 mm or 73 ps; all particles together span about 20° in phase, far less than the 91° of the stable region at −30°;
- transverse rms size about 0.95 mm, normalised rms emittances $\varepsilon_x = 0.138$ and $\varepsilon_y = 0.141$ π·mm·mrad, Twiss parameters $\alpha_x = -0.658$, $\beta_x = 0.377$ mm/mrad;
- longitudinal normalised rms emittance (in AVAS's $z$–$\Delta p/p$ convention) 0.266 π·mm·mrad, equivalent to 48.7°·keV at 162.5 MHz.

The height of the stable region gives a cross-check: at the exit ($E_0T = 0.55$ MV/m, $\varphi_s = -30°$) the half-height of the stable region is about 73 keV, more than 6 times the rms energy spread, so the bunch sits safely inside the stable region.

The lab below uses exactly these transverse Twiss parameters and lets you compare KV, waterbag (WB), parabolic (PB) and Gaussian (GS) distributions: their rms values are the same, but their shapes and edges are very different. The example's `beam.txt` uses GS.

```avas-lab
distribution alpha=-0.658 beta=0.377 emit=0.138
```

The beam leaving the RFQ is "spreading" in both directions. Transversely, $\alpha_x < 0$ means it is diverging; longitudinally, the higher-energy protons run ahead and the bunch lengthens in a drift. The beam line below is just a 1 m drift: the transverse rms size grows from 0.95 mm to about 3.7 mm (linear preview); longitudinally, as computed in the [overview](#journey-overview-bunching), the rms bunch length grows from 1.24 mm to 3.89 mm over the same metre. The RFQ therefore has to be followed immediately by quadrupoles and bunchers, which is the job of the [medium-energy beam transport](#journey-mebt).

```avas-beamline
drift 1.0 0.02 0
```

## Common misconceptions {#journey-rfq-misconceptions}

**"The quadrupoles of an RFQ are magnetic."** They are not. Between the four vanes of an RFQ there is an RF electric field; the focusing is an electric force, and the alternation comes from time, not from a spatial sequence. This is exactly why its focusing parameter $B$ does not depend on velocity.

**"Transmission is the accelerated fraction."** Not necessarily. Protons that were not captured may also pass through the RFQ, only with an energy far below the design value, and they are lost soon downstream. RFQ design codes usually distinguish the total transmission from the transmission of accelerated particles; when comparing, check which one is meant.

**"The RFQ cell length is $\beta\lambda$."** A cell (between two neighbouring modulation extrema) is $\beta\lambda/2$ long; the period of the vane undulation and of the transverse focusing is $\beta\lambda$.

**"Bunching compresses the phase-space area."** What is compressed is the phase range; the energy spread grows accordingly, and ideally the area is conserved. The real rms longitudinal emittance does grow during bunching, because non-linear bunching distorts the phase-space distribution and makes it filament, so the rms ellipse enclosing it becomes larger, even though the exact phase-space volume is still conserved.

**"The bunch leaving the RFQ is already matched to the next section."** The Twiss parameters at the RFQ exit are set by the structure at the end of the RFQ. In general the MEBT has to re-match the beam transversely and longitudinally before it can enter the superconducting section.

## In AVAS {#journey-rfq-in-avas}

### AVAS does not simulate the RFQ

AVAS has no RFQ element and does not simulate the RFQ. The vane modulation of an RFQ changes continuously from cell to cell and needs dedicated RFQ design and simulation codes. AVAS starts from the beam at the RFQ exit.

### Particle file: `readparticledistribution`

The most complete hand-over is to read the particle distribution written by the RFQ code directly. In `beam.txt`:

```text
readparticledistribution part_rfq.dst
```

The file goes into `InputFile/`. The Beam page of the GUI can import a particle file (copying it into `InputFile/`) and switch between "generate from parameters" and "read particle file" with `use_dst`. When a particle file is read, every keyword in `beam.txt` except `numofcharge` is ignored: number of particles, current, frequency, energy and distribution all come from the file. An `.edst` file can describe a mixed beam of several particle species.

`.dst` is the binary TraceWin format. AVAS reads it (`avas/data/particles.py`) as follows: the header holds the number of particles, the current (mA) and the frequency (MHz); then each particle has six double-precision numbers $x$ [cm], $x'$ [rad], $y$ [cm], $y'$ [rad], $\varphi$ [rad], $W$ [MeV]; the file ends with the rest energy $mc^2$ [MeV]. The example file gives 5260 particles, 0.1841 mA, 162.5 MHz and 938.272 MeV.

```avas-params
beam:readparticledistribution beam:use_dst beam:numofcharge
```

### Twiss parameters: generating from parameters

The other way is to reduce the particle distribution to rms Twiss parameters and let AVAS generate a distribution with the same rms values. The Beam page can compute the parameters from a particle file and fill them in. The example project is set up like this: `use_dst 0`, and `twissx -0.65785 0.37743 0.13768` and `twissy -0.6341 0.37292 0.14059` agree with the statistics of `part_rfq.dst`; the distribution is `GS GS`.

```avas-params
beam:kneticenergy beam:current beam:frequency beam:particlenumber beam:twissx beam:twissy beam:twissz beam:distribution
```

> **AVAS convention**: β is in mm/mrad and the emittance is the normalised rms emittance (π·mm·mrad); `twissz` uses the longitudinal coordinates $z$ and $z' = \Delta p/p$ in the same units. Longitudinal units common in RFQ codes (deg·MeV, keV·ns …) must be converted first: the example's 48.7°·keV at 162.5 MHz, for instance, corresponds to 0.266 π·mm·mrad.

The two ways differ in the details of the distribution. Twiss parameters keep only the rms ellipse; the shape, the tails and the transverse–longitudinal coupling of the real RFQ output are lost. When space charge is strong, or when halo and losses matter, read the particle file. For linear estimates, parameter scans or comparisons between designs, Twiss parameters are more convenient.

### Example

The `beam.txt` of `examples/hwr010`:

```text
readparticledistribution unknown
numofcharge 1
particlerestmass 938.272088159928
current 0.18409999999999999
particlenumber 5260
frequency 162500000.0
kneticenergy 1.5270289036445652
use_dst 0
beamtype notdc
twissx -0.65785 0.37743 0.13768
twissy -0.6341 0.37292 0.14059
twissz 0.04275 0.37292 0.26506
distribution GS GS
```

When generating from parameters, the GUI writes `readparticledistribution unknown` and `use_dst 0`. Current, number of particles, frequency and energy agree with the header of `part_rfq.dst`. To read the particle file directly instead, select `part_rfq.dst` on the Beam page (the GUI writes `use_dst 1` and puts the file name into `readparticledistribution`).

## Further reading {#journey-rfq-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: the potential expansion of the RFQ, the focusing parameter, adiabatic bunching and the Los Alamos four-section design method; the formulas of this chapter agree with it.
- I. M. Kapchinskii, V. A. Teplyakov, "Linear ion accelerator with spatially homogeneous strong focusing", *Prib. Tekh. Eksp.* No. 2 (1970): the original paper proposing the RFQ principle.
- K. R. Crandall, R. H. Stokes, T. P. Wangler, "RF quadrupole beam dynamics design studies", 1979 Linear Accelerator Conference.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: focusing and bunching of space-charge-dominated beams.
- The AVAS manual `docs/使用说明20260427.docx`: `readparticledistribution` and particle files.
