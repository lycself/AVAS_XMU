# Bunching: the RFQ turns a continuous beam into bunches

When our proton leaves the low-energy transport line it is still part of a continuous 35 keV beam, with the protons spread evenly in time. A few metres later it sits in a bunch about 1.2 mm long (rms), moving with its companions at a beat of 162.5 MHz, with a kinetic energy of 1.527 MeV. The machine that makes this change is the **radio-frequency quadrupole** (RFQ): four long electrodes running along the beam surround it, with an RF voltage between them.

The RFQ has to do three things at once that seem to contradict each other:

- hold the beam tightly together transversely, right where space charge is strongest;
- chop the continuous beam into bunches while losing almost no particles;
- and accelerate.

Why can a buncher cavity plus ordinary accelerating cavities not do the job? Why does the RFQ focus with electric rather than magnetic fields? Why are its electrode surfaces wavy? This chapter answers each in turn. For the background see [Ion source and low-energy transport](#journey-source) and [The whole linac](#journey-overview-bunching); the basic concepts of longitudinal motion are in [Longitudinal motion](#longitudinal).

> **AVAS usually does not simulate this section.** The RFQ is normally computed with dedicated design codes; its exit particle distribution (`part_rfq.dst` in the example), or the Twiss parameters computed from it, is AVAS's initial beam. The new kernel also has an `rfq` element computed cell by cell, not yet checked against other codes. Both ways are described in [In AVAS](#journey-rfq-in-avas) in this chapter.

## A continuous beam meets an RF cavity {#journey-rfq-dc-problem}

First a thought experiment: what happens if the 35 keV continuous beam is sent straight into a string of ordinary 162.5 MHz accelerating cavities?

Imagine a crowd walking at random towards a revolving door that never stops turning: only those who happen to arrive at a gap get through, and the rest bump into the door. Protons are the same. At which moment of its oscillation (its **phase**) the RF field is when a proton reaches the first gap is completely random, spread evenly over 360°. With the energy gain $\Delta W = qV_0T\cos\varphi$ (the letters are explained in [The whole linac](#journey-overview-velocity)), half of the protons are accelerated and half decelerated, and the average gain is zero.

In the next few cavities the protons with a suitable phase keep being accelerated; those with an unsuitable phase are decelerated or even reversed, or drift away gradually because their velocity differs from the reference proton's. In the end only those inside the longitudinal **stable region** follow the reference proton in stable acceleration. For a synchronous phase of −30° the stable region is about 91° wide in phase (see [The whole linac](#journey-overview-bunching)), so at most a quarter of the protons can be captured, and that is before considering the height of the stable region in energy.

The other three quarters do not vanish at once; they travel some distance through the accelerator with the wrong energy and then hit the wall. For a 10 mA proton accelerator this means several milliamperes of beam lost at a few MeV, an unacceptable level of activation and heat load. So the continuous beam must be bunched first, and the bunching itself must lose almost nothing.

## Bunching slowly {#journey-rfq-bunching}

The most direct way to bunch is a **buncher cavity** followed by a drift. The idea is like making runners cross the finish line together: slow down those in front and speed up those behind, and after a while they are all packed together. A buncher does exactly this: it decelerates early protons and accelerates late ones, and after a drift they converge at the same moment.

Unfortunately the buncher voltage is sinusoidal, so only near the centre is the correction proportional to "how early"; protons far from the centre are under-corrected and do not converge. An ideal estimate (kinematics only, no space charge or energy spread): at the best buncher voltage about 74 % of the protons end up within ±45°, and about a quarter remain outside. Moreover the converged bunch forms a curved S shape on the "phase × energy" chart, with an rms emittance far larger than that of the original DC beam. At 35 keV, in the space-charge-dominated regime, it is even worse: as the bunch is compressed its peak current multiplies (to about 34 times the average current at the RFQ exit, see [The whole linac](#journey-overview-space-charge)), and space charge pushes the converging bunch apart again.

The RFQ's answer is **adiabatic bunching** ("adiabatic" here means "changing slowly enough"). Imagine a train of small "dips" moving along with the protons: at first the dips are shallow and wide, so wide that together they cover the whole RF period and every proton sits in some dip; then the dips are slowly deepened and narrowed along the accelerator. As long as the change is slow enough, each proton follows its own dip, and as the dip narrows the amplitude of its back-and-forth motion shrinks with it, until the whole crowd has been gathered into narrow bunches. It is like a sheepdog patiently driving a scattered flock into a pen that gets narrower and narrower, without losing a single sheep.

The analogy must be clear about where it fails. A ball rolls to the bottom of a bowl thanks to friction, which removes energy; a beam has no friction, and the area on the "phase × energy" chart is strictly conserved (Liouville's theorem). How then can the range of phases shrink? Think of a mass on a spring: if the spring is slowly replaced by stiffer and stiffer ones, the amplitude of the oscillation gets smaller, but the speed through the middle gets larger. A deepening dip is like a stiffening spring: the range of phases over which the protons swing back and forth shrinks, and the price is a larger spread of energies. The phases are squeezed from 360° to a few degrees, the energy spread grows accordingly, and the area stays the same (the quantitative version is in "[Going deeper: adiabatic bunching](#journey-rfq-deep-bunching)").

"Slowly enough" has a concrete scale: during one back-and-forth swing of a proton, the shape of the dip may change only a little. An estimate near the RFQ exit (average accelerating gradient 0.55 MV/m, synchronous phase −30°): one back-and-forth swing takes 2.7 m, about 51 cells (a cell is half a period of the electrode wave, see "Vane modulation" below). Adiabatic bunching has to be spread over many such swings, one reason why RFQs are often several metres long.

Following the classic design approach developed at Los Alamos, an RFQ is divided lengthwise into four sections:

- **Radial matching section**: over a few cells the transverse focusing goes from weak to strong, carrying the converging beam from the LEBT into the alternating RFQ focusing; there is no longitudinal field yet.
- **Shaper**: the longitudinal field rises slowly from zero, with the synchronous phase starting at −90°. At −90° there is no acceleration and the stable region covers the whole period; the protons begin to gather in phase.
- **Gentle buncher**: the synchronous phase moves gradually towards about −30° and the longitudinal field keeps growing, designed to keep the small-amplitude longitudinal oscillation frequency and the spatial bunch length roughly constant, so that space charge does not suddenly become strong anywhere.
- **Accelerator section**: the synchronous phase and electrode parameters are essentially fixed, and the bunches are accelerated to the exit energy in a nearly constant way.

An RFQ designed this way usually transmits more than 95 % of the beam (typical value), far better than a single buncher.

## Electric quadrupole focusing {#journey-rfq-focusing}

Now the transverse plane. At 35 keV space charge is strongest and strong focusing is needed. Why does the RFQ use electric rather than magnetic fields?

Compare the two forces on a proton: the electric force $qE$ does not depend on velocity, it is the same however fast the proton moves; the magnetic force $qvB$ is proportional to velocity, and the slower the proton, the less grip it has. So for slow protons the electric field is far more effective. To put a number on it: an electric quadrupole lens with field gradient $E'$ (how fast the field grows with distance from the axis, in V/m²) acts on a proton like a magnetic quadrupole lens of gradient

$$
G_{\mathrm{eq}} = \frac{E'}{v}
$$

(the gradient $G$ of the [Quadrupole](#quadrupole-field) chapter, in T/m).

The four electrodes of the RFQ (vanes, shaped like four blades) run along the beam: opposite ones have the same polarity, neighbouring ones opposite polarity, and there is an RF voltage $V$ between neighbouring vanes. With the vane tips at a distance $a$ from the axis, the field gradient is about $E' = V/a^2$. Take a typical order of magnitude, $V = 65$ kV and $a = 6$ mm (not the parameters of a particular RFQ): $E' = 65000/0.006^2 = 1.8\times10^{9}$ V/m². A 35 keV proton moves at $2.59\times10^{6}$ m/s, so the equivalent magnetic gradient is 697 T/m; at 1.527 MeV ($1.71\times10^{7}$ m/s) it is 106 T/m. A magnetic quadrupole with a 6 mm aperture and a gradient of several hundred T/m would need a pole field of several tesla, and could only be a few millimetres long: the focusing period here is only $\beta\lambda = 15.9$ mm, so each lens would be 8 mm long, shorter than its aperture, and the fringe fields at its ends could not be separated at all.

This leads to the second clever feature of RFQ focusing: **the alternation comes from time**. The [Quadrupole](#quadrupole-alternating) chapter explained that a quadrupole field focuses in one plane and defocuses in the other, so focusing and defocusing lenses must alternate to contain the beam in both planes. The RFQ does not need to line up separate lenses: the voltage on the vanes changes sign every half RF period, so the quadrupole field at a given place switches between focusing and defocusing. In one RF period the proton moves exactly $\beta\lambda$, so from the proton's point of view it passes through an alternating "focus–defocus" structure with period $\beta\lambda$, while the vanes themselves are continuous along the beam.

How strong is the focusing? A quantitative analysis (see "[Going deeper: the RFQ's electric quadrupole focusing](#journey-rfq-deep-focusing)") gives a dimensionless **focusing parameter** $B$ that depends only on voltage, aperture and RF wavelength and **not on the proton velocity**: the quantitative statement that "electric focusing does not weaken at low velocity". The parameters above give $B = 6.55$ and a zero-current phase advance per focusing period of $\sigma_0 \approx 42°$ (the **phase advance** is the angle by which a particle's oscillation about the centre line advances per period, see [Quadrupole](#quadrupole-fodo)); with $a = 5$ mm, $B = 9.43$ and $\sigma_0 \approx 61°$. These values are for unmodulated vanes (modulation follows in the next section). Once the vanes are made wavy, the transverse focusing weakens by a factor $X$ (the **focusing efficiency**): without modulation (radial matching section and the start of the shaper) $X = 1$, but at 1.527 MeV with a modulation $m = 2$ ($a = 6$ mm) $X$ is only 0.41. So a real design adjusts $a$ and $V$ along the RFQ to keep $\sigma_0$ roughly constant.

Keeping $\sigma_0 \approx 42°$, the β function of the matched beam is about $\beta\lambda/\sigma_0$: 0.022 mm/mrad at 35 keV and 0.14 mm/mrad at 1.527 MeV. Such a small β function means a very thin beam, which is why the [LEBT](#journey-source-matching) has to converge the beam strongly into the RFQ.

How high can the voltage be? If the field on the vane surface is too strong, it sparks (**breakdown**). The empirical yardstick for RF breakdown is the Kilpatrick criterion $f = 1.64E_K^2\,\ee^{-8.5/E_K}$ ($f$ in MHz, $E_K$ in MV/m); at 162.5 MHz, $E_K = 13.6$ MV/m. The peak surface field on the vanes is somewhat higher than $V/a$ (it depends on the curvature of the vane tip), and RFQ designs usually set the peak surface field at 1.2–1.8 times the Kilpatrick field (typical values), with CW RFQs at the conservative end. Above, $V/a = 10.8$ MV/m is only 0.8 times the Kilpatrick field; with a typical tip enhancement factor of 1.3–1.4, the peak surface field is about 1.0–1.1 times the Kilpatrick field, at or slightly below the low end of that range: a conservative choice for CW operation.

## Vane modulation: a longitudinal field grown out of a quadrupole field {#journey-rfq-modulation}

A pure quadrupole field has only transverse components; it can neither accelerate nor bunch. The RFQ's trick is to make the vane surfaces undulate along the beam, wavy: where the horizontal vanes come close to the axis, the vertical ones are far away, and half a period later the other way round. The period of the undulation is $\beta\lambda$, and each half period ($\beta\lambda/2$) is called a **cell**. So the cell length of an RFQ grows with the proton velocity: 8.0 mm at the 35 keV entrance and 52.6 mm at the 1.527 MeV exit.

Why does the undulation produce a longitudinal field? Let the closest distance of a vane from the axis be $a$ and the farthest $ma$ ($m > 1$ is the **modulation**). In one cell the horizontal vanes are close to the axis (distance $a$) and the vertical ones far (distance $ma$), so the potential on the axis leans towards that of the horizontal vanes; in the next cell it is the other way round, and the axis potential leans towards the vertical vanes. Between neighbouring cells there is thus a potential difference proportional to the inter-vane voltage $V$, and with it an accelerating field along the axis.

The same inter-vane voltage $V$ is thereby split into two competing shares, described by two dimensionless coefficients:

- **Acceleration efficiency $A$** ($0 \le A < 1$): how large a share goes into the field along the axis. Without modulation ($m = 1$), $A = 0$; the deeper the modulation (the larger $m$), the larger $A$.
- **Focusing efficiency $X$** ($0 < X \le 1$): since the vanes retreat to the larger distance $ma$ over half their length, the transverse quadrupole focusing is weakened to $X$ times its unmodulated value (the $X$ of the previous section). The stronger the longitudinal acceleration (larger $A$), the less transverse focusing is left (smaller $X$).

The energy gained by a proton per cell is (derivation in "[Going deeper: modulated vanes](#journey-rfq-deep-modulation)")

$$
\Delta W_{\mathrm{cell}} = \frac{\pi}{4}\,qAV\cos\varphi_s .
$$

Two things about this result are worth noting.

First, the energy gain of one cell does not depend on the cell length, only on $A$ and $V$. But since the cells get longer and longer, the average accelerating gradient per unit length, $E_0T = \pi AV/(2\beta\lambda)$, falls like $1/\beta$ as the velocity grows.

Second, the acceleration efficiency $A$ depends on the ratio of aperture to cell length (the dimensionless ratio $ka = 2\pi a/(\beta\lambda)$). At low velocity the cells are short, and the "wavelength" of the vane undulation is not much larger than the aperture. Look at a corrugated iron roof from a distance: the finer the corrugation, the sooner it looks flat as you step back. The vane undulation is the same: when it is fine, the longitudinal field it produces stays "screened" close to the vane surface and does not reach the axis, so $A$ is small.

With $m = 2$, $a = 6$ mm, $V = 65$ kV and $\varphi_s = -30°$: at 35 keV, $ka = 2.37$ and $A = 0.090$, only 4.0 keV per cell; at 0.3 MeV, $A = 0.47$ and 20.6 keV; at 1.527 MeV, $ka = 0.36$ and $A = 0.57$, 25.2 keV, with an average gradient $E_0T = 0.55$ MV/m. This explains the division of labour in an RFQ: in the low-energy part $A$ is naturally small, just right for gentle shaping and bunching; in the high-energy part $A$ is close to saturation while the average gradient falls like $1/\beta$, so accelerating further with an RFQ pays off less and less, and the beam is handed to the next section at around 1.5–3 MeV (typical values).

## The beam at the exit {#journey-rfq-output}

The example project's `part_rfq.dst` is exactly the particle distribution at the exit of a 162.5 MHz RFQ. Statistics over its 5260 macro-particles (each macro-particle stands for many real protons):

- mean kinetic energy 1.527 MeV ($\beta = 0.05698$), rms energy spread 11.4 keV (0.75 %), relative momentum spread 0.375 %;
- current 0.184 mA, 1.13 pC per bunch, about $7.1\times10^{6}$ protons;
- rms phase width 4.26°, i.e. rms bunch length 1.24 mm, 73 ps; all particles together span about 20° in phase, far less than the 91° of the −30° stable region;
- transverse rms size about 0.95 mm, normalized rms emittances $\varepsilon_x = 0.138$, $\varepsilon_y = 0.141$ π·mm·mrad, Twiss parameters $\alpha_x = -0.658$, $\beta_x = 0.377$ mm/mrad;
- longitudinal normalized rms emittance (in AVAS's $z$–$\Delta p/p$ convention) 0.266 π·mm·mrad, equivalent to 48.7°·keV at 162.5 MHz.

The height of the longitudinal stable region gives a check: at the exit ($E_0T = 0.55$ MV/m, $\varphi_s = -30°$) the stable region's half-height in energy is about 73 keV, more than 6 times the rms energy spread, so the bunch sits safely inside the stable region.

The lab below uses exactly these transverse Twiss parameters and compares four distributions: KV, waterbag (WB), parabolic (PB) and Gaussian (GS). All have an rms size of 0.94 mm, but their shapes and edges are very different. The farthest particle, for example, is about 2.0 times the rms from the centre for KV but about 3.6 times for the Gaussian; beyond 2 rms there are only 0.2 % of the particles for KV but 4.6 % for the Gaussian. The example `beam.txt` uses GS.

```avas-lab
distribution alpha=-0.658 beta=0.377 emit=0.138
```

The beam leaving the RFQ "spreads" in both directions. Transversely, $\alpha_x < 0$ means it is diverging; longitudinally, the higher-energy protons run ahead and the bunch lengthens in a drift. The animation below is exactly this RFQ-exit beam travelling through a 1 m drift: the transverse rms size grows from 0.95 mm to about 3.7 mm (linear model, example current). Longitudinally, as computed in [The whole linac](#journey-overview-bunching), the rms bunch length goes from 1.24 mm to 3.89 mm over the same metre. So right after the RFQ come quadrupoles and bunchers: the job of the [medium-energy transport line](#journey-mebt).

```avas-motion
drift alpha=-0.658 L=1
```

The same 1 m drift as it appears in the lattice editor (switch between 2D and 3D, click an element for details):

```avas-beamline
drift 1.0 0.02 0
```

## Think about it {#journey-rfq-think}

> **Think about it 1**: if adiabatic bunching is so good, why not do another round of "slow bunching" after 1.5 MeV with ordinary superconducting cavities?
>
> Answer: it is neither needed nor possible. At the RFQ exit the bunches are already formed, and afterwards they only need to be maintained (which is what the MEBT bunchers do). And superconducting cavities are discrete: each has only a few gaps with a fixed field shape, neighbouring cavities are separated by drifts and focusing elements, and the longitudinal focusing comes as a "kick" every few tens of centimetres; the RFQ, by contrast, modulates its field continuously in every cell ($\beta\lambda/2$). A string of such cavities cannot start from an extremely weak field covering the full 360° of phase and deepen the "dips" smoothly. This ability of the RFQ comes from its unique electrode structure.

> **Think about it 2**: if electric quadrupole focusing is so good, why not keep using it after the RFQ?
>
> Answer: the equivalent magnetic gradient $E'/v$ of an electric lens falls with velocity: from 35 keV to 1.527 MeV it already drops from 697 T/m to 106 T/m and keeps falling, whereas a magnetic lens does not weaken with velocity. Besides, the RFQ's acceleration efficiency also falls with velocity (the average gradient falls like $1/\beta$), so at a few MeV a structure of "magnetic focusing + high-gradient cavities" is more effective.

> **Think about it 3**: the RFQ cell length is $\beta\lambda/2$, but the focusing period is $\beta\lambda$. Why the factor of two?
>
> Answer: the focusing comes from the alternation of the voltage in time, positive and negative once each per RF period, which forms one "focus–defocus" period, the $\beta\lambda$ a proton travels in one period. The acceleration comes from the vane undulation: neighbouring cells have opposite longitudinal fields, and each time the proton travels half a period, $\beta\lambda/2$, the RF field also reverses, so every cell accelerates. This is the same reason why the gap spacing of a π-mode accelerating structure is $\beta\lambda/2$.

> **Think about it 4**: in the distribution lab, the four distributions have exactly the same rms size. Since AVAS's Twiss parameters only describe the rms, why care which distribution is chosen?
>
> Answer: because beam losses are decided by the edges, not by the rms. Apertures are usually only a few times the rms size; the particles of a KV distribution reach at most 2 times the rms, but a Gaussian has particles at 3.6 times or further. When estimating losses or looking at halo, the tails of the distribution matter much more than the rms, which is also why, with strong space charge, it is better to read the RFQ particle file directly (see "In AVAS" in this chapter).

## Common misconceptions {#journey-rfq-misconceptions}

**"The RFQ's quadrupole is a magnetic quadrupole."** No. Between the four vanes of an RFQ there is an RF electric field; the focusing is an electric force, and the alternation comes from time rather than from a spatial arrangement. This is exactly why its focusing parameter $B$ does not depend on velocity.

**"The transmission is the fraction that gets accelerated."** Not necessarily. Uncaptured protons may also get through the RFQ, only with an energy far below the design value, and they are lost soon downstream. RFQ design codes usually distinguish total transmission from the transmission of accelerated particles; check which one is meant when comparing.

**"The RFQ cell length is $\beta\lambda$."** A cell (between two neighbouring modulation extremes) is $\beta\lambda/2$ long; $\beta\lambda$ is the period of the vane undulation and of the transverse focusing.

**"Bunching compresses the phase-space area."** What is compressed is the range of phases; the energy spread grows accordingly, and ideally the area is conserved. In practice the rms longitudinal emittance does grow during bunching, because nonlinear bunching distorts the phase-space distribution and makes it filament, so the rms ellipse enclosing it grows, even though the exact phase-space volume is still conserved.

**"The bunches at the RFQ exit are already matched to the next section."** The Twiss parameters at the RFQ exit are set by the structure at the end of the RFQ. In general the MEBT has to re-match them transversely and longitudinally before they can enter the superconducting section.

## Going deeper: adiabatic bunching—why slowly, and how slowly {#journey-rfq-deep-bunching}

The main text explained adiabatic bunching with a sheepdog and a stiffening spring. This section makes it precise in five steps: first why a single buncher gathers only 74 %, then how the protons in a bunch oscillate back and forth, and finally the scale of "slowly".

### Step 1: why a buncher plus drift cannot gather everything

Let the phase at which a proton reaches the buncher be $\varphi$ (relative to the moment the buncher field crosses zero, $\varphi > 0$ meaning late, spread evenly from $-180°$ to $180°$). The buncher accelerates late protons and decelerates early ones, with an energy correction proportional to $\sin\varphi$. After a drift the energy difference becomes a difference in arrival time: protons that gained energy are faster and arrive earlier (the [Drift](#drift-deep-longitudinal) chapter), so the phase after the drift is

$$
\varphi_{\text{after}} = \varphi - \alpha\sin\varphi ,
$$

where $\alpha$ is set by the buncher voltage and the drift length. For small $\varphi$, $\sin\varphi \approx \varphi$, and $\alpha = 1$ makes $\varphi_{\text{after}} \approx 0$: protons near the centre all converge to one point. But far from the centre $\sin\varphi$ is much smaller than $\varphi$ ($\sin 90° = 1$, whereas $90° = 1.57$ radians), and the correction falls short. Scanning $\alpha$ from 0.3 to 2.5 and counting the fraction that ends up within $\pm 45°$ after the drift, the best is about 74 % (at $\alpha \approx 2.1$). By then the protons near the centre have already "overshot", and the whole distribution is twisted into an S shape on the "phase × energy" chart.

### Step 2: the protons in a bunch oscillate back and forth

The [overview's deep block](#journey-overview-deep-frequency) showed that in an accelerating structure with synchronous phase $\varphi_s < 0$, late protons gain more energy and early ones less; those that gained more run faster and then arrive earlier. Together this is simple harmonic motion,

$$
\frac{\dd^2\Delta\varphi}{\dd s^2} = -k_l^2\,\Delta\varphi, \qquad k_l^2 = \frac{2\pi qE_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3},
$$

where $\Delta\varphi$ is the proton's phase relative to the reference proton and $k_l$ the number of radians of oscillation per metre. The "dip" of the main text is this restoring force: the deeper the dip, the larger $k_l$.

### Step 3: a slowly stiffening spring

A mass on a spring has energy $E = \tfrac12 kA^2$ ($k$ the spring constant, $A$ the amplitude) and angular frequency $\omega = \sqrt{k/m}$. Physics has an important result: if the spring constant changes slowly enough (only a little per oscillation period), $E/\omega$ stays constant. Inserting:

$$
\frac{E}{\omega} = \frac{\tfrac12 kA^2}{\sqrt{k/m}} \propto \sqrt{k}\,A^2 = \text{const} \quad\Longrightarrow\quad A \propto k^{-1/4} .
$$

Make the spring 16 times stiffer and the amplitude halves, while the maximum speed through the middle, $\omega A \propto k^{1/4}$, doubles. For a bunch, position corresponds to phase and speed to energy difference: as the dip deepens, the range of phases shrinks and the energy spread grows, and their product (the area) stays the same. That is adiabatic bunching.

Why must it be "slow"? If the spring constant suddenly jumped, the oscillator would have no time to adjust; those at the edge at that moment would carry on oscillating with large energy, the amplitudes would not shrink evenly, and the distribution would be scrambled, like the S shape of Step 1.

### Step 4: the scale of "slowly"

"Only a little per oscillation period" means in an RFQ: over the distance $2\pi/k_l$ of one back-and-forth swing of a proton, the depth of the dip may change only a little. With the numbers near the RFQ exit ($E_0T = 0.55$ MV/m, $\varphi_s = -30°$, $\beta^3\gamma^3 = 1.86\times10^{-4}$, $\lambda = 1.845$ m):

$$
k_l^2 = \frac{2\pi \times 0.55 \times 0.5}{938.272 \times 1.845 \times 1.86\times10^{-4}} = 5.37\ \mathrm{m^{-2}}, \qquad k_l = 2.3\ \mathrm{m^{-1}}, \qquad \frac{2\pi}{k_l} = 2.7\ \mathrm{m} .
$$

A cell is $\beta\lambda/2 = 52.6$ mm long, so one swing takes about 51 cells. The shape of the dips must change over several such swings, which is why an RFQ is several metres long, often with several hundred cells.

### Step 5: why start at −90°

At a synchronous phase of $-90°$ the reference proton meets the field just as it crosses zero and is not accelerated; early and late protons are decelerated and accelerated by symmetric amounts. The stable region is then symmetric and covers the full 360° (a "stationary bucket"), so at the start every proton of the continuous beam sits in some dip. The shaper starts from here, first slowly deepening the dips, and the gentle buncher then moves the synchronous phase towards $-30°$, tightening the bunches and starting to accelerate at the same time.

### For readers who know analytical mechanics

The $E/\omega$ of Step 3 is the action $J = \oint p\,\dd q/(2\pi)$ of the harmonic oscillator, an adiabatic invariant when parameters change slowly. The Hamiltonian of longitudinal motion is at the end of the [overview's deep block](#journey-overview-deep-frequency); adiabatic bunching requires the bucket area to grow more slowly than the small-amplitude synchrotron oscillation frequency, while space charge lowers the effective $k_l$; the Los Alamos gentle buncher chooses $\varphi_s(z)$ and $A(z)$ so as to keep $k_l$ and the spatial bunch length $\Delta\varphi\,\beta\lambda$ constant. Filamentation from the non-adiabatic part makes the rms longitudinal emittance grow, the main reason why the longitudinal emittance at the RFQ exit exceeds the "DC-beam equivalent".

## Going deeper: the RFQ's electric quadrupole focusing—focusing parameter and smooth approximation {#journey-rfq-deep-focusing}

The main text gave the focusing parameter $B = 6.55$ and the phase advance per period $\sigma_0 \approx 42°$. This section derives them in five steps, using "the field is the slope of the potential", the trajectory-bending formula, simple harmonic motion and "the average of $\sin^2$ is 1/2".

### Step 1: the potential and field between the vanes

Of the four RFQ vanes, the horizontal pair is at potential $+V/2$ and the vertical pair at $-V/2$ (and then the signs flip in time). Near the axis the simplest potential with this symmetry is

$$
U = \frac{V}{2}\,\frac{x^2 - y^2}{a^2}\,\sin(\omega t + \varphi) .
$$

At the horizontal vane tip ($x = a$, $y = 0$) it is exactly $V/2$, at the vertical vane tip ($x = 0$, $y = a$) exactly $-V/2$. The field is the slope of the potential with position (with a minus sign): $E_x = -\dfrac{\partial U}{\partial x} = -\dfrac{V}{a^2}\,x\,\sin(\omega t + \varphi)$. The field is proportional to $x$, and the coefficient $V/a^2$ is the field gradient $E'$ of the main text.

### Step 2: the field bends the trajectory

As in Step 4 of the [overview's deep block](#journey-overview-deep-space-charge), a sideways force $F$ bends the trajectory as $x'' = F/(mv^2)$ (low speed, $\gamma \approx 1$). The proton moves with velocity $v$, $t = s/v$, so $\omega t = 2\pi s/(\beta\lambda)$:

$$
x'' = \frac{qE_x}{mv^2} = -\frac{qV}{mc^2\beta^2a^2}\,\sin\!\left(\frac{2\pi s}{\beta\lambda} + \varphi\right)x .
$$

Compared with a magnetic quadrupole, $x'' = -\frac{qvG}{mv^2}x$, the two are equal for $G = E'/v$: the equivalent magnetic gradient of the main text.

### Step 3: counting in periods, the velocity disappears

Replace distance by "how many periods have been travelled": $\tau = s/(\beta\lambda)$. Each time $s$ grows by $\beta\lambda$, $\tau$ grows by 1, so the second derivative with respect to $\tau$ is the second derivative with respect to $s$ times $(\beta\lambda)^2$:

$$
\frac{\dd^2x}{\dd\tau^2} = -B\sin(2\pi\tau + \varphi)\,x, \qquad B = \frac{qV\lambda^2}{mc^2a^2} .
$$

The $\beta^2$ in the denominator and the $\beta^2$ in $(\beta\lambda)^2$ cancel exactly: $B$ does not depend on velocity. Check: $qV = 0.065$ MeV, $\lambda^2 = 3.404$ m², $mc^2a^2 = 938.272\ \mathrm{MeV} \times 3.6\times10^{-5}$ m², so $B = 0.065 \times 3.404/0.03378 = 6.55$.

### Step 4: why push-and-pull focuses on balance

The force $-B\sin(2\pi\tau)\,x$ points inwards part of the time and outwards the rest, averaging to zero, so why is there net focusing? Write the particle position as "a slowly varying main part $X$ + a fast small wiggle $\xi$". The wiggle is driven by the fast force: $\xi'' = -B\sin(2\pi\tau)X$; integrating twice, as for simple harmonic motion, $\xi = \dfrac{B X}{4\pi^2}\sin(2\pi\tau)$. Wiggle and force are in step: when the force points inwards ($\sin(2\pi\tau) > 0$), $\xi > 0$ and the particle is just farther from the axis, so it is pushed back more; when the force points outwards, $\xi < 0$ and the particle is closer to the axis, so it is pushed out less. Putting $\xi$ back into the force, the part due to the wiggle is

$$
-B\sin(2\pi\tau)\,\xi = -\frac{B^2X}{4\pi^2}\sin^2(2\pi\tau) ,
$$

and since the average of $\sin^2$ over a period is 1/2, on average this is a force towards the axis, $-\dfrac{B^2}{8\pi^2}X$. It is the same reason as "the particle is farther from the axis in the focusing lens and closer in the defocusing lens" in the [Quadrupole](#quadrupole-alternating) chapter. So the main part performs simple harmonic motion:

$$
\frac{\dd^2X}{\dd\tau^2} = -\frac{B^2}{8\pi^2}\,X ,
$$

and the angle by which the oscillation advances per period ($\tau$ growing by 1) is

$$
\sigma_0 \approx \frac{B}{2\sqrt2\,\pi} .
$$

### Step 5: the numbers

$B = 6.55$: $\sigma_0 = 6.55/(2\sqrt2\pi) = 0.737$ radians $= 42°$. With $a = 5$ mm, $B = 6.55 \times 36/25 = 9.43$ and $\sigma_0 = 61°$. With modulated vanes, $B$ must be multiplied by the focusing efficiency $X$: $B = qXV\lambda^2/(mc^2a^2)$. The β function of the matched beam is about "period length ÷ phase advance per period", $\beta\lambda/\sigma_0$: at 35 keV, $15.9\ \mathrm{mm}/0.737 = 0.022$ m (= 0.022 mm/mrad); at 1.527 MeV, $105.1\ \mathrm{mm}/0.737 = 0.14$ mm/mrad. RF defocusing (Step 4 of the [overview's deep block](#journey-overview-deep-frequency)) reduces $\sigma_0$ somewhat; it is neglected here.

### For readers who know mathematical physics

Including RF defocusing, Step 3 becomes a Mathieu-type equation $\dd^2x/\dd\tau^2 + [\Delta_{rf} + B\sin(2\pi\tau)]x = 0$, where the RF defocusing term (Wangler's notation) $\Delta_{rf} = \frac{\pi^2 qAV\sin\varphi_s}{2mc^2\beta^2}$ is negative for $\varphi_s < 0$. Stability is decided by the trace of the one-period transfer matrix, $|\mathrm{Tr}\,M| < 2$; the smooth approximation gives $\sigma_0^2 \approx \frac{B^2}{8\pi^2} + \Delta_{rf}$, accurate enough for $\sigma_0 \lesssim 60°$. The "fast–slow separation" of Step 4 is the idea of the Kapitza pendulum (effective potential $\langle F^2\rangle/(2m\Omega^2)$).

## Going deeper: modulated vanes—acceleration efficiency and energy gain per cell {#journey-rfq-deep-modulation}

The main text gave the gain per cell, $\Delta W_{\mathrm{cell}} = \frac{\pi}{4}qAV\cos\varphi_s$, and the values of $A$ at different energies. This section derives the gain formula in five steps and then explains why $A$ is small at low velocity.

### Step 1: the potential on the axis flips cell by cell

In one cell of the vane undulation, the axis potential leans towards the potential of the closer vane pair; in the next cell towards the other pair. Write this alternation as a cosine:

$$
U_{\text{axis}}(z, t) = \frac{AV}{2}\cos kz\,\sin(\omega t + \varphi), \qquad k = \frac{2\pi}{\beta\lambda} .
$$

$\cos kz$ goes from $+1$ to $-1$ over one cell ($\beta\lambda/2$). If the modulation were so deep that the axis potential equalled that of the closer vanes, the amplitude would be $V/2$; in reality it reaches only $A$ times that, and $A$ is the acceleration efficiency.

### Step 2: the slope of the potential is the longitudinal field

$E_z = -\dfrac{\partial U_{\text{axis}}}{\partial z}$. The derivative of $\cos kz$ with respect to $z$ is $-k\sin kz$, so

$$
E_z = \frac{kAV}{2}\,\sin kz\,\sin(\omega t + \varphi) .
$$

The average of $|E_z|$ over a cell is $\frac{kAV}{2}\cdot\frac{2}{\pi} = \frac{2AV}{\beta\lambda}$ (the average of $|\sin|$ over half a period is $2/\pi$); call it the average axial field $E_0 = 2AV/(\beta\lambda)$.

### Step 3: the field the proton sees along the way

At time $t$ the proton is at $z = vt$, so $\omega t = \omega z/v = kz$ (this is what "the cell length follows $\beta\lambda$" means: each time the proton crosses a cell, the RF advances exactly half a period). The field it sees is

$$
E_z = \frac{kAV}{2}\,\sin kz\,\sin(kz + \varphi) .
$$

Using the product-to-sum formula $\sin u\,\sin(u + \varphi) = \tfrac12[\cos\varphi - \cos(2u + \varphi)]$: the second term cancels over a cell, averaging to 0; the first is the constant $\tfrac12\cos\varphi$.

### Step 4: the energy gain of one cell

Energy gain = average field × charge × cell length:

$$
\Delta W_{\mathrm{cell}} = q\cdot\frac{kAV}{2}\cdot\frac{\cos\varphi}{2}\cdot\frac{\beta\lambda}{2} = q\cdot\frac{kAV}{2}\cdot\frac{\cos\varphi}{2}\cdot\frac{\pi}{k} = \frac{\pi}{4}\,qAV\cos\varphi .
$$

Writing it in the form $qE_0T\cos\varphi\cdot\frac{\beta\lambda}{2}$ gives the transit-time factor $T = \pi/4$ and the average gradient $E_0T = \pi AV/(2\beta\lambda)$. Check at 1.527 MeV ($A = 0.57$, $V = 65$ kV, $\varphi_s = -30°$): $\frac{\pi}{4} \times 0.57 \times 65\ \mathrm{keV} \times 0.866 = 25.2$ keV; $E_0T = \pi \times 0.57 \times 0.065\ \mathrm{MV}/(2 \times 0.1051\ \mathrm{m}) = 0.55$ MV/m.

### Step 5: why $A$ is small at low velocity

The undulation of the vane surface is a potential pattern along $z$ with "wavelength" $\beta\lambda$. In charge-free space such a pattern fades quickly away from the surface: when the pattern is fine, every distance of about $\beta\lambda/(2\pi) = 1/k$ reduces it to roughly a third. The axis is a distance $a$ from the vane surface, so what matters is $ka$: when $ka$ is large (short cells, large aperture), the axis hardly feels the undulation. The exact result is ($I_0$ is the modified Bessel function of order zero, $I_0(0) = 1$, growing rapidly with its argument)

$$
A = \frac{m^2 - 1}{m^2 I_0(ka) + I_0(mka)}, \qquad X = 1 - A\,I_0(ka) .
$$

With $m = 2$ and $a = 6$ mm: at 35 keV, $ka = 2.37$, $A = 0.090$ and $X = 0.73$, 4.0 keV per cell; at 0.3 MeV, $ka = 0.81$, $A = 0.47$, 20.6 keV; at 1.527 MeV, $ka = 0.36$, $A = 0.57$ and $X = 0.41$, 25.2 keV.

### For readers who know mathematical physics

The lowest-order near-axis RFQ potential (the two-term Kapchinsky–Teplyakov expansion) is a solution of Laplace's equation in cylindrical coordinates $(r, \theta, z)$:

$$
U(r,\theta,z,t) = \frac{V}{2}\left[X\left(\frac{r}{a}\right)^2\cos2\theta + A\,I_0(kr)\cos kz\right]\sin(\omega t + \varphi) .
$$

$I_0(kr)$ comes from the radial equation of $\nabla^2[f(r)\cos kz] = 0$ (the modified Bessel equation) and is the exact form of Step 5's "the pattern fades away from the surface": seen the other way round, the amplitude on the axis is $I_0(ka)$ times smaller than at the vane surface. Requiring the potential to be $V/2$ at $(r, \theta, z) = (a, 0, 0)$ and $-V/2$ at $(ma, \pi/2, 0)$ gives the $A$ and $X$ above. Higher multipoles (dodecapole etc.) are set by the vane-tip shape; design codes use multi-term expansions or 3D field computations.

## In AVAS {#journey-rfq-in-avas}

### Usually starting at the RFQ exit

The vane modulation of an RFQ changes continuously from cell to cell, and it is usually computed with dedicated RFQ design and simulation codes; AVAS starts from the beam at the RFQ exit. Two other ways:

- **Field map**: write the RFQ's 3-D RF field as a field map (`.edx` … `.bdz`) and use it as an ordinary `field` element.
- **`rfq` element** (new kernel, default mode only, computed on the CPU): `rfq L R 0 1 f 0 rfqinput.txt`.
  - The structure file has one row per cell: number, length (cm), inter-vane voltage (kV), aperture $a$ (cm), two unused columns, the 8-term potential coefficients $A_0 \ldots A_7$.
  - The field is the negative gradient of the Kapchinsky–Teplyakov 8-term potential (the main text and the deep sections use only its two main terms: the quadrupole and the accelerating term).
  - The synchronous phase comes from the scan: −90° in the first cell, the same design habit as [the shaper starting at −90°](#journey-rfq-bunching).
  - The original kernel's rfq element never ran, so there are no original-kernel results to compare with; check it against other codes before using it for real designs. The linear preview treats it as a drift.

### Particle file: `readparticledistribution`

The most complete hand-over is to read the particle distribution written by the RFQ code directly. In `beam.txt` write

```text
readparticledistribution part_rfq.dst
```

The file goes in `InputFile/`; the Beam page can import a particle file (copying it to `InputFile/`), and `use_dst` switches between "generate from parameters" and "read a particle file". When a particle file is read, all `beam.txt` keywords except `numofcharge` are ignored: particle number, current, frequency, energy and distribution all come from the file. An `.edst` file can describe a mixed beam of several species.

`.dst` is a TraceWin binary format. AVAS reads it as: a header with the number of particles, the current (mA) and the frequency (MHz), then 6 doubles per particle, $x$ [cm], $x'$ [rad], $y$ [cm], $y'$ [rad], $\varphi$ [rad], $W$ [MeV], and finally the rest energy $mc^2$ [MeV]. The example file has 5260 particles, 0.1841 mA, 162.5 MHz, 938.272 MeV.

```avas-params
beam:readparticledistribution beam:use_dst beam:numofcharge
```

### Twiss parameters: generating from parameters

The other way is to reduce the particle distribution to rms Twiss parameters and let AVAS generate a distribution with the same rms values. The Beam page can compute the parameters from a particle file and fill them in. The example project is set up this way: `use_dst 0`, with `twissx -0.65785 0.37743 0.13768` and `twissy -0.6341 0.37292 0.14059` matching the statistics of `part_rfq.dst`, and distributions `GS GS`.

```avas-params
beam:kneticenergy beam:current beam:frequency beam:particlenumber beam:twissx beam:twissy beam:twissz beam:distribution
```

> **AVAS convention**: β is in mm/mrad and the emittance is the normalized rms emittance (π·mm·mrad); `twissz` uses the longitudinal coordinates $z$ and $z' = \Delta p/p$, in the same units. Longitudinal units common in RFQ codes (deg·MeV, keV·ns, …) must be converted first: the example's 48.7°·keV at 162.5 MHz corresponds to 0.266 π·mm·mrad.

The two ways differ in the details of the distribution. Twiss parameters keep only the rms ellipse; the shape, the tails and the transverse–longitudinal coupling of the real RFQ exit distribution are lost. With strong space charge, or when halo and losses matter, read the particle file. For linear estimates, parameter scans or comparing designs, Twiss parameters are more convenient.

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

When generating from parameters, the interface writes `readparticledistribution unknown` and `use_dst 0`. Current, particle number, frequency and energy match the header of `part_rfq.dst`. To read the particle file directly instead, select `part_rfq.dst` on the Beam page (the interface writes `use_dst 1` and puts the file name into `readparticledistribution`).

### This chapter's RFQ formulas

The RFQ formulas used in the main text are in the deep blocks above: the focusing parameter $B$ and the smooth-approximation $\sigma_0$ in "[Going deeper: the RFQ's electric quadrupole focusing](#journey-rfq-deep-focusing)"; the two-term Kapchinsky–Teplyakov potential expansion and the formulas giving the acceleration efficiency $A$ and focusing efficiency $X$ from the vane geometry at the end of "[Going deeper: modulated vanes](#journey-rfq-deep-modulation)".

## Further reading {#journey-rfq-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: the RFQ potential expansion, focusing parameter, adiabatic bunching and the Los Alamos four-section design method; this chapter's formulas agree with it.
- I. M. Kapchinskii, V. A. Teplyakov, "Linear ion accelerator with spatially homogeneous strong focusing", *Prib. Tekh. Eksp.* No. 2 (1970): the original paper proposing the RFQ principle.
- K. R. Crandall, R. H. Stokes, T. P. Wangler, "RF quadrupole beam dynamics design studies", 1979 Linear Accelerator Conference.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: focusing and bunching of space-charge-dominated beams.
- AVAS user manual `docs/使用说明20260427.docx`: `readparticledistribution` and particle files.
