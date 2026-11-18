# Acceleration: the superconducting section from low to high β

Leaving the MEBT, our proton still has only 1.527 MeV and moves at 5.7 % of the speed of light; at the end it must have 1 GeV and 87.5 % of the speed of light, more than six hundred times the energy. Almost all of that comes from the superconducting section.

Oddly, the superconducting section is not one long accelerating pipe but several hundred small, independent cavities, grouped into families of completely different shapes, and at low energy nearly every cavity is followed by a solenoid. This chapter answers four questions:

1. Why superconducting?
2. Why can one cavity family serve only a range of speeds?
3. Why do cavities and focusing elements alternate?
4. Why is the synchronous phase chosen near $-30^\circ$ to $-20^\circ$?

First an overall picture. Imagine a row of stepping stones across a river; whoever crosses has to land on a stone with every step. The stones are laid out for a certain stride: stones laid for a child are awkward for an adult, and stones laid for an adult are out of a child's reach. The "stepping stones" of an accelerating cavity are its accelerating gaps, and the proton must arrive at each next gap exactly when the field points the right way. The proton keeps getting faster, its stride keeps growing, so one cavity family suits only a range of speeds and after a while the next family takes over. On the other hand, every cavity that accelerates the proton also spreads the beam a little transversely (just like the buncher of the previous chapter), so after every cavity or two a "lens" has to gather the beam back in.

This chapter uses [the RF cavity](#rf-cavity), [longitudinal motion](#longitudinal) and [lattice design](#lattice-design), and builds the beginning of this section from the two field maps of the AVAS example project; [the guided tutorial](#tutorial) is its hands-on version.

## Why superconducting cavities {#journey-sc-why-sc}

**The intuitive picture.** An RF cavity is like a "bucket" holding an oscillating electromagnetic field. Its walls are metal; the field induces currents in them, the currents heat the resistive walls, and the bucket "leaks": the lost energy must be topped up from outside all the time. An ordinary copper cavity leaks badly; a superconducting niobium cavity hardly leaks at all.

**A number.** The wall loss of a cavity looks just like the heating of a resistor in school electricity, $P = U^2/R$:

$$
P_c = \frac{V^2}{R_{\text{sh}}} ,\qquad R_{\text{sh}} = (R/Q)\,Q_0 .
$$

$V$ is the accelerating voltage of the cavity and $R_{\text{sh}}$ the **shunt impedance**, a kind of "equivalent resistance" of the cavity: the larger, the less power is wasted. It splits into two factors: $R/Q$ depends only on the shape of the cavity; $Q_0$, the **intrinsic quality factor**, says how slowly the walls "leak" (its meaning is explained in [Going deeper: wall losses, quality factor and refrigeration](#journey-sc-deep-power)). For the same shape, a room-temperature copper cavity has $Q_0$ of about $10^4$, a superconducting niobium cavity $10^9$ to $10^{10}$, five to six orders of magnitude more.

The example HWR field map with $K_e = 1.36$ gives a voltage of about 1 MV; take $R/Q = 150\ \Omega$ (low-β cavities typically have tens to hundreds of ohms; this value is only illustrative) and continuous-wave operation (CW, RF always on):

- copper, $Q_0 = 10^4$: $R_{\text{sh}} = 1.5\times10^6\ \Omega$, wall loss $(10^6)^2/(1.5\times10^6) \approx 670\ \mathrm{kW}$;
- superconducting, $Q_0 = 5\times10^9$: a wall loss of only about 1.3 W.

The power this cavity delivers to the beam, from $P = UI$, is about $0.36\ \mathrm{MV}\times 0.18\ \mathrm{mA} \approx 65\ \mathrm{W}$ for the example's 0.18 mA, and only a few kilowatts even at 10 mA. Spending 670 kW to accelerate a beam of tens of watts to a few kilowatts is clearly not workable.

The 1.3 W of the superconducting cavity are released at 2–4 K and have to be pumped up to room temperature by a refrigerator. Moving heat from cold to warm is hard work: an ideal refrigerator needs 66 W of electrical power for every watt removed at 4.5 K; large real plants reach only about 35 % of the ideal efficiency, roughly 200 W per watt at 4.5 K and more at 2 K. Even so, each cavity costs of order a kilowatt or less.

So for CW or high-duty-factor machines (where the RF is on a large fraction of the time), superconductivity is practically the only choice. It brings two further advantages:

- **Larger apertures**. A room-temperature cavity keeps its beam hole small to raise the shunt impedance and cut the wall loss; a superconducting cavity has small losses anyway and can afford a larger aperture, which lowers the risk of beam loss.
- **Each cavity is powered and phased independently**. When one cavity fails, the phases and amplitudes of the others can be readjusted to compensate (a typical practice). This is impossible in the room-temperature drift-tube structure of the next section.

## The other way: the room-temperature drift-tube linac {#journey-sc-dtl}

> **AVAS does not simulate this section**: a room-temperature drift-tube linac (DTL) has no dedicated model. If its 3D field map is available it could in principle be read as a `field` element; this primer does not cover that. Its output beam is handed to AVAS as a particle file or as Twiss parameters, like any other (see [In AVAS](#journey-sc-in-avas)).

Besides the superconducting section, a common accelerating structure at low and medium energy is the Alvarez DTL: a long cylindrical cavity with a string of metal tubes (**drift tubes**) suspended along its axis. It runs in a mode called TM010 (the "0 mode"): all gaps have the field in the same direction at the same moment, and half a period later they all reverse together.

How can the proton only be accelerated and never decelerated? By "hiding": during the half period when the field points the wrong way, the proton is inside a drift tube (there is no field inside a metal tube); when the field points the right way, it is crossing a gap. So the proton must cross exactly one cell per RF period, and the cell length equals $\beta\lambda$ (the distance the proton travels in one RF period). At 162.5 MHz this length is 105 mm at 1.527 MeV and 267 mm at 10 MeV; DTLs usually run at a few hundred MHz, where the cells are shorter.

The drift tubes contain quadrupoles that form a FODO-type transverse focusing lattice (see [quadrupole](#quadrupole-fodo)). At low energy RF defocusing and space charge are both strong and focusing is needed every cell or two, which is exactly what quadrupoles inside the drift tubes provide. The cell lengths of a DTL are machined one by one for a design velocity profile, and once built, the tank fits only that one velocity curve $\beta(z)$: if part of the field is wrong, it cannot be rephased separately as an independent superconducting cavity can.

## Why one cavity family covers only a range of β {#journey-sc-ttf}

**The stride of the stepping stones.** First look at the field of the example HWR cavity. The on-axis field $E_z$ has two peaks of opposite sign, at $z \approx 0.074\ \mathrm{m}$ and $0.140\ \mathrm{m}$: this cavity has two accelerating gaps. Each peak is spread over several centimetres and trails further towards the outside of the cavity, so the field-weighted gap centres are at $0.065\ \mathrm{m}$ and $0.145\ \mathrm{m}$, $d = 0.081\ \mathrm{m}$ apart (measuring between the two tips gives 0.066 m).

```avas-field
map hwr010
```

The fields of the two gaps point in opposite directions, so the proton has to get from one gap to the other in **half an RF period**: while it is accelerated in the first gap, the field in the second gap points the other way; by the time it reaches the second gap, the field has turned over and accelerates again. In half a period the proton travels $\beta\lambda/2$, so

$$
d = \frac{\beta\lambda}{2}\quad\Rightarrow\quad \beta = \frac{2d}{\lambda} = \frac{2\times 0.081}{1.845} \approx 0.088 .
$$

This speed is the cavity's **geometric β**. At a different speed the proton reaches the second gap at the wrong moment, when the field has not fully turned over or is already weakening, and it gains less. The number measuring "the fraction of the ideal acceleration actually obtained" is the **transit-time factor** $T(\beta)$.

The [overview](#journey-overview-velocity) chapter computed $T(\beta)$ for the example HWR map: it peaks at $\beta \approx 0.104$ with about 0.82; it stays above 70 % of the peak for $\beta = 0.066$–$0.21$ (about 2–22 MeV); at the example beam's $\beta = 0.057$ it is only 0.40. The peak is not at the geometric β of 0.088 but at 0.104, because each gap has a length of its own and the transit-time factor of a single gap grows with $\beta$; multiplied together, the peak moves towards higher β.

**More gaps, narrower window.** Set the single-gap factor aside and consider only $N$ identical gaps in a row, neighbouring gaps with opposite fields, spaced by half the wavelength belonging to the geometric β. At exactly the geometric β every gap is crossed at the best moment; a little off, the timing slips a little at every gap, and the slips accumulate one after another. The more gaps, the faster they accumulate, and the last gaps may even decelerate. Computed as in [Going deeper: the transit-time factor](#journey-sc-deep-ttf), the speed range (in units of the geometric β) where this factor stays at least 0.8 is 0.71–1.69 for two gaps, 0.87–1.17 for five and 0.93–1.09 for nine. It is exactly like a diffraction grating: the more slits, the narrower the bright fringes.

**At low energy the speed changes fast.** At 1.527 MeV one cavity adds 0.36 MeV and $\beta$ goes from 0.0570 to 0.0633, up 11 % (at low energy the kinetic energy is $W = \tfrac12 mv^2$, so the relative change in speed is about half the relative change in energy: $0.36/(2\times1.527) \approx 12\ \%$); at 500 MeV, even a 10 MeV cavity changes $\beta$ by only 0.5 %. At low energy the cavities must have few gaps and wide windows, hence the two-gap HWR (half-wave resonator), QWR (quarter-wave resonator) or spoke cavities with few gaps; at high energy the speed hardly changes and five- to nine-cell elliptical cavities can be used, each giving a much higher voltage.

A common division (typical values; facilities differ): HWRs have geometric β near 0.1; spoke cavities 0.2–0.5, about 19–145 MeV; elliptical cavities above 0.5. Our proton passes through these families in turn. The [overview](#journey-overview-frequency) chapter also explains why the frequency can jump from 162.5 MHz to 325 MHz and 650 MHz when the family changes.

## The period: solenoid + cavity {#journey-sc-period}

**Why every cavity needs a lens after it.** Why not string a dozen cavities together and focus once at the end? Because every cavity defocuses the beam transversely. The previous chapter showed that an RF field pushing the bunch towards its centre longitudinally (with a negative synchronous phase) necessarily pushes particles outwards transversely (see [the medium-energy transport](#journey-mebt-buncher)). This RF defocusing is especially strong at low energy: its strength is proportional to $\sin(-\varphi_s)/(\beta\gamma)^3$ (see [the RF cavity](#rf-cavity)), and $1/(\beta\gamma)^3$ is 5378 at 1.527 MeV but only 319 at 10 MeV. A thin-gap estimate for the cavities of the first period below ($K_e = 0.8$, $\varphi_s = -30^\circ$) gives a defocusing strength of about $1.2\ \mathrm{m^{-1}}$ each, a focal length under 1 m. Space charge is strongest in the same place. So at low energy every cavity must be followed by a focusing element.

**Why solenoids.** A solenoid is a current-carrying coil whose field inside points along the beam ($B_z$). It focuses differently from a quadrupole: as a particle enters the solenoid, the fringe field sets it rotating about the axis, and the rotating particle then interacts with $B_z$ and feels a force towards the axis (details in [solenoid](#solenoid)). There are three reasons to choose it:

- it focuses $x$ and $y$ at the same time, unlike a quadrupole that focuses one direction and defocuses the other, so one element suffices;
- it is short and compact and can sit in the cryomodule with the cavities (a superconducting solenoid);
- at low energy it is strong.

The thin-lens focusing strength of a solenoid is

$$
\frac{1}{f} = \int\left(\frac{B_z}{2B\rho}\right)^2\dd z
$$

(the integral just adds up $(B_z/2B\rho)^2 \times$ length over every short piece along the line). For the example map `sol` ($K_b = 0.30$, on-axis peak about 2.5 T) at 1.527 MeV this gives $1/f = 2.85\ \mathrm{m^{-1}}$, a focal length of 0.35 m, while the beam turns $35^\circ$ about the axis (the Larmor angle). Compared with the cavity's defocusing of $1.2\ \mathrm{m^{-1}}$, four tenths of the solenoid's focusing go into cancelling the RF defocusing.

But the formula contains $(1/B\rho)^2$: a solenoid's strength falls with the square of the rigidity, a quadrupole's only with the first power ($k = G/B\rho$). From 1.527 MeV to 10 MeV, $B\rho$ grows from 0.179 to 0.458 T·m; the same solenoid's focusing drops to 1/6.6, the same quadrupole's only to 1/2.6; at 150 MeV the factors are 1/106 and 1/10. So at higher energy the focusing switches to quadrupole doublets or FODO, and by then the RF defocusing is much weaker too and cavities can be spaced further apart.

**Phase advance and the 90° rule.** In a periodic focusing channel the particles swing back and forth about the axis, like a ball rolling to and fro in a valley. The "angle" a particle advances in its swing per period is the **phase advance** of the period. The animation below is a quadrupole FODO channel (the focusing used at high energy, but the idea of phase advance is exactly the same):

```avas-motion
fodo
```

Watch the two blue particles: they swing about the centre, with a phase advance of about $76^\circ$ per period at the default settings, completing about 1.25 swings in 6 periods. Now set "mismatch" to 1.3: the entrance beam is thicker than the shape the channel "prefers", and the size of the whole beam starts to beat up and down, the largest rms growing from 1.82 mm to 3.07 mm. This beating (an oscillation of the beam **envelope**) is twice as fast as the swing of a single particle: the beam is widest both when particles are at their leftmost and at their rightmost.

The central rule of period design is: **the zero-current phase advance per period must stay below 90°, transversely and longitudinally**. The intuitive reason is a swing. On a swing you squat and stand up twice in every back-and-forth, and the swing goes higher and higher; this is **parametric resonance**: the pumping rhythm is exactly twice the swinging frequency. The envelope oscillates at twice the frequency of the particle swing; at a phase advance of $90^\circ$ the envelope oscillates once every two periods, that is, half an oscillation per period, while the periodic structure "pushes" the beam once per period, so the pushes come at exactly twice the envelope frequency: the swing's rhythm. At zero current this resonance does not make the linear motion unstable; with space charge, the beam size in turn changes the space-charge force, and the resonance makes the envelope oscillate more and more. The derivation is in [lattice design](#lattice-design).

**Six periods from the example field maps.** The beam line below has six "solenoid + cavity" periods, each 0.76 m long. The first two cavities have $K_e = 0.8$, the middle two 1.0 and the last two 1.2; the solenoid $K_b$ rises step by step from 0.30 to 0.40. From the linear preview's period matrices, the longitudinal phase advances of the six periods are about 86°, 83°, 89°, 80°, 80° and 70°, the transverse ones (in Larmor coordinates) about 76°, 77°, 74°, 72°, 69° and 67°, all below 90° (accurate to about 1°; the noisy solenoid map makes the preview's period matrices slightly non-symplectic, see the note in [the tutorial](#tutorial-step4)). How much the cavity defocusing matters: switching off the cavity of the first period raises the transverse phase advance from 76° to about 91°, see [Going deeper: RF defocusing, solenoids and the phase advance](#journey-sc-deep-period).

```avas-beamline
drift 0.05 0.02 0
field 0.35 0.02 0 3 0 0 0 0.3 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 0.8 -0.8 hwr010
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.32 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 0.8 -0.8 hwr010
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.34 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.0 -1.0 hwr010
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.36 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.0 -1.0 hwr010
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.38 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.2 -1.2 hwr010
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.4 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.2 -1.2 hwr010
drift 0.3 0.02 0
```

Look at two things in the 2D plot:

- **The energy curve is a staircase**: every cavity adds a step, the steps being 0.21, 0.25, 0.37, 0.43, 0.56 and 0.59 MeV high, reaching 3.94 MeV after six periods. The steps grow partly because $K_e$ increases and partly because the proton gets closer to the speed the cavity suits, so $T(\beta)$ increases.
- **The rms envelope** varies between 0.7 and 2.0 mm, with a maximum of about 2.0 mm, comfortably inside the 20 mm aperture.

The envelope does not repeat exactly from period to period: here the beam from the RFQ enters the first period directly, without an MEBT, with a mismatch factor of about 0.3–0.4, and the rms $x$ at the period entrances jumps between 0.94 mm and 1.64 mm. The beam line of the [medium-energy transport](#journey-mebt) chapter puts an MEBT in front, and the variation in the same periods is much smaller.

> **Think about it**: if all six periods used $K_e = 1.36$ and $K_b = 0.35$ of the single cavity in step 3 of the [tutorial](#tutorial), the exit energy would be 4.99 MeV, much more than the 3.94 MeV here. Why are the first cavities turned down?
>
> Answer: to keep to the 90° rule. From the linear preview's period matrices, at full field the first period's longitudinal phase advance is about 128° and the transverse about 94°, both above 90°; only by the third period do they drop to about 95° and 62°. The longitudinal phase advance scales as $\sqrt{E_0T\sin(-\varphi_s)/(\beta\gamma)^3}$, and at $\beta = 0.057$ the cavity is too "strong". Turning down the first cavities costs about 1 MeV but puts every period at a stable working point. Real designs, too, often do not run the low-energy cavities at full gradient.

## The synchronous phase: a compromise between acceleration and stability {#journey-sc-phase}

**Where the surfer stands.** The RF field is like a wave and the proton like a surfer. On the crest the push is largest but the footing is unstable: protons arriving earlier or later than the synchronous particle both gain less energy, and the bunch spreads. Standing on the rising slope in front of the crest (a negative synchronous phase in AVAS's convention), the push is a bit smaller but there is a benefit: a late proton has a larger phase, closer to the crest, gains more energy and catches up; an early one gains less and slows down. The bunch is "held" longitudinally (see [longitudinal motion](#longitudinal)).

So the synchronous phase $\varphi_s$ is a lever with energy at one end and stability at the other:

- the energy gain is $\propto\cos\varphi_s$, larger the closer to 0;
- the phase range that can hold particles longitudinally (the bucket) is about $3|\varphi_s|$ wide, wider the more negative (why 3 times: see [Going deeper: the bucket](#journey-sc-deep-bucket));
- the longitudinal focusing and the accompanying transverse RF defocusing are $\propto\sin(-\varphi_s)$, stronger the more negative.

The exact bucket width and the other two ends of the lever, computed with the example maps for the first two periods (all $K_e = 1.36$, $K_b = 0.35$):

| $\varphi_s$ | $\cos\varphi_s$ | bucket phase width | energy after two periods | longitudinal phase advance, first period |
|---|---|---|---|---|
| $-20^\circ$ | 0.940 | 60° | 2.497 MeV | 107° |
| $-30^\circ$ | 0.866 | 91° | 2.374 MeV | 128° |
| $-40^\circ$ | 0.766 | 122° | 2.226 MeV | 149° |

(The bucket width uses the separatrix for constant parameters, see [longitudinal motion](#longitudinal); the phase advances come from the linear preview.)

$-20^\circ$ gives more energy, but the bucket is only 60° wide, and the bunch entering the superconducting section (rms about 5° after the schematic MEBT of the [medium-energy transport](#journey-mebt) chapter), together with its tails and all kinds of errors, needs enough margin; $-40^\circ$ has a wide bucket, but $\cos\varphi_s$ is 18 % smaller (in the linear preview the energy gain of the first two periods is actually 28 % smaller, because at low energy $V_0T$ also depends on phase), and both the longitudinal focusing and the transverse RF defocusing are stronger. That is why the low-energy end often uses about $-30^\circ$. As the beam is accelerated, the phase width of the bunch shrinks by adiabatic damping (see the [overview](#journey-overview-frequency): about 2.4° rms at 10 MeV), and $|\varphi_s|$ can gradually be reduced to about $-20^\circ$ to gain more energy.

**Check it yourself.** The lab below is a single HWR cavity ($K_e = 0.8$). Drag the phase from the default $-30^\circ$ to $-60^\circ$ and to $0^\circ$ and watch three numbers trade off:

| phase | energy gain | exit rms $z$ | exit rms $x$ |
|---|---|---|---|
| $0^\circ$ | 0.259 MeV | 1.79 mm | 2.11 mm |
| $-30^\circ$ | 0.206 MeV | 1.27 mm | 2.61 mm |
| $-60^\circ$ | 0.106 MeV | 1.10 mm | 2.99 mm |

The closer to 0, the more energy, but the less the bunch is held longitudinally (larger rms $z$); the more negative, the tighter the longitudinal squeeze and the more the beam spreads transversely.

```avas-lab
rf-cavity phase=-30 Ke=0.8
```

## How much each period gains, and how long the section is {#journey-sc-energy-gain}

Continuing the six periods above with $K_e = 1.36$ for the later cavities and the solenoid $K_b$ increasing step by step, the linear preview reaches 10 MeV after about 16 periods and 12 m, the typical exit energy of an HWR section (see the [overview](#journey-overview-thread)). By then each cavity adds about 0.6 MeV, the longitudinal phase advance has dropped to about 30° and the transverse stays at 60°–70°. Such a low longitudinal phase advance shows that at the high-energy end one solenoid can serve two or more cavities, giving more acceleration per metre: a common layout of low-β superconducting sections (such as "solenoid–cavity–cavity"), see [lattice design](#lattice-design).

The average accelerating gradient of this section (cavities, solenoids and drifts together) is about 0.5–0.8 MeV/m, much lower than the field of a single cavity: the cavities take up less than a third of each period, the low-energy cavities are turned down, and $T(\beta)$ is not at its best.

How long is the whole superconducting section? Estimate it as "length = energy to add ÷ energy per metre". With the assumptions of the [overview](#journey-overview-thread) (spoke section average $E_0T = 1\ \mathrm{MV/m}$ at $\varphi_s = -25^\circ$, elliptical section 3 MV/m at $-20^\circ$): from 10 MeV to 150 MeV, 140 MeV are added at $1 \times \cos 25^\circ = 0.91\ \mathrm{MeV}$ per metre, about 155 m; from 150 MeV to 1 GeV, 850 MeV at $3 \times \cos 20^\circ = 2.8\ \mathrm{MeV}$ per metre, about 300 m. The superconducting section is the longest part of the machine.

Along the way, our proton's speed rises from 0.057 to 0.875 and $B\rho$ from 0.18 T·m to 5.66 T·m; the normalized emittance ideally stays constant while the geometric emittance shrinks as $1/(\beta\gamma)$, by a factor of 32; the bunch length in millimetres changes little, but in RF phase it drops from a few degrees to less than 1° (see the [overview](#journey-overview-thread)).

## Think about it {#journey-sc-think}

> **Think about it 1**: if superconductivity is so good, why do many pulsed proton linacs still use room-temperature structures?
>
> Answer: wall losses count as average power. If a pulsed machine has a duty factor of only a few per cent, the average wall loss of a copper cavity drops by the same factor, and the peak power can come from pulsed RF sources; superconductivity, meanwhile, needs a cryogenic plant, which may not pay off for a low duty factor. The advantage of superconductivity is clearest for CW and high duty factors.

> **Think about it 2**: why must the synchronous phase be negative? Doesn't $+30^\circ$ give the same energy gain?
>
> Answer: the cosine is even, so the synchronous particle gains the same at $\pm 30^\circ$; but for the other particles of the bunch the result is opposite. In AVAS's cosine convention, with $\varphi_s < 0$ a late proton has a larger phase, closer to the crest, gains more energy and catches up; an early one gains less and slows down: the bunch is focused longitudinally. With $\varphi_s > 0$ it is the other way round and the bunch spreads. The price is that longitudinal focusing always comes with transverse defocusing; conversely, with $\varphi_s > 0$ the transverse directions focus while the longitudinal motion is unstable. No phase focuses both at once, which is exactly why solenoids are added.

> **Think about it 3**: the two gaps of the example HWR cavity are 6.6 cm apart. For a two-gap cavity suited to protons at $\beta = 0.2$, at the same frequency, how far apart should the gaps be?
>
> Answer: $d = \beta\lambda/2 = 0.2 \times 1.845/2 \approx 0.18\ \mathrm{m}$, almost three times the example cavity. Cavity size grows with the speed it is built for, which is one reason the high-β sections switch to higher frequencies: doubling the frequency halves $\lambda$, so the cavities need not become too large (see the [overview](#journey-overview-frequency)).

## Common misconceptions {#journey-sc-misconceptions}

**"The geometric β of a cavity is the velocity of the proton in it."** The geometric β is set by the gap spacing, and the maximum of $T(\beta)$ is also affected by the length of each gap: about 0.088 and 0.104 respectively for the example hwr010. And a cavity family actually works over a wide range on both sides.

**"The closer the synchronous phase is to 0 the better, because it gives the most energy."** At $\varphi_s = 0$ the longitudinal bucket has zero width and the bunch cannot be held.

**"A cavity's energy gain is $qV_0T\cos\varphi_s$ with constant $V_0T$."** At low energy, when one cavity adds more than 20 % of the entrance energy, the proton speeds up noticeably inside the cavity and $V_0T$ depends on phase and amplitude (see [the RF cavity](#rf-cavity)). AVAS integrates along the field map and does not use this approximation.

**"A solenoid only focuses and does nothing else."** A solenoid rotates the transverse phase space about the axis (about 35° per period in the example above), coupling $x$ and $y$. With a round beam this is invisible; with a non-round beam or a transverse offset it shows. The transverse phase advance has to be computed in Larmor coordinates.

**"Phase advance below 90° is only a transverse rule."** It applies longitudinally too; at low β it is the longitudinal plane that exceeds it first.

**"Weaker focusing at high energy is a bad thing."** While the focusing gets weaker, RF defocusing and space charge weaken even faster ($\propto 1/(\beta\gamma)^3$), so the high-energy end can use longer periods and fewer focusing elements.

## Going deeper: wall losses, quality factor and refrigeration {#journey-sc-deep-power}

The main text compared copper and superconducting cavities using $P_c = V^2/R_{\text{sh}}$. This section explains what $Q_0$ and $R/Q$ mean, why superconductivity reduces the losses so much, and why one watt at low temperature takes hundreds of watts of electricity to remove.

### Step 1: the quality factor is "how long it rings"

Strike a bell and it rings for a while before falling silent. A good bell rings long because it loses very little energy per oscillation. A cavity is the same: it stores an oscillating electromagnetic energy $U$, and the wall losses consume it at the power $P_c$. The quality factor is defined as

$$
Q_0 = \frac{\omega U}{P_c} ,
$$

with $\omega = 2\pi f$ the angular frequency. It means: if the power is switched off, the stored energy falls to $1/\ee \approx 37\ \%$ of its value in $Q_0/(2\pi)$ oscillation periods. At 162.5 MHz: a copper cavity with $Q_0 = 10^4$, counting only the wall loss, loses its energy in about 10 μs; a superconducting cavity with $Q_0 = 5\times10^9$ takes about 5 s, close to a billion oscillations. (A real superconducting cavity also has a coupler feeding power in and out, which makes the ringing much shorter; here we compare only the walls themselves.)

### Step 2: R/Q depends only on the shape

The relation between the accelerating voltage $V$ and the stored energy $U$ depends only on the shape of the cavity: for a given shape, doubling the whole field doubles $V$ and quadruples $U$. So $V^2/(\omega U)$ is a number fixed by the shape alone, and that is $R/Q$:

$$
\frac{R}{Q} = \frac{V^2}{\omega U} .
$$

Multiplying step 1 and this step: $(R/Q)\,Q_0 = V^2/P_c = R_{\text{sh}}$, the shunt impedance of the main text. This uses the linac convention $R_{\text{sh}} = V^2/P_c$; the circuit convention gives half of it.

### Step 3: the resistance of the walls

A high-frequency current flows only in a very thin surface layer of the metal (the **skin effect**), thinner at higher frequency, so the walls resist a high-frequency current more than a direct current. For copper at 162.5 MHz the layer is about 5 μm thick and the equivalent **surface resistance** about 3 mΩ. $Q_0$ is inversely proportional to the surface resistance: for the same shape, a surface resistance smaller by some factor gives a $Q_0$ larger by that factor. From the ratio of $Q_0$ in the main text, the equivalent resistance of a superconducting wall is about one five-hundred-thousandth of copper's, a few nano-ohms.

A superconductor has no resistance to direct current, so why a tiny bit at high frequency? In a superconductor most electrons are bound in "pairs" that flow without resistance; but at any finite temperature a few electrons remain unpaired, and the alternating field drives them, causing a small loss. The lower the temperature, the fewer unpaired electrons and the smaller the loss, which is why many superconducting cavities accept the extra refrigeration cost and run at 2 K rather than 4.5 K.

### Step 4: why one watt at low temperature is expensive

Moving heat from cold to warm is like pumping water uphill: it takes work. The second law of thermodynamics gives the minimum cost for an ideal (Carnot) refrigerator: removing 1 W at temperature $T_c$ and rejecting it to surroundings at $T_h$ takes at least

$$
\frac{T_h - T_c}{T_c}\ \mathrm{W}
$$

of power. For $T_c = 4.5\ \mathrm{K}$ and $T_h = 300\ \mathrm{K}$ this is $295.5/4.5 = 66\ \mathrm{W}$; at 2 K it is 149 W. Large real 4 K plants reach about 35 % of the ideal efficiency, i.e. about 190 W per watt at 4.5 K and about 430 W at 2 K. So the 1.3 W wall loss of a superconducting cavity costs about 0.25–0.55 kW of electrical power at room temperature, still far less than the 670 kW of copper.

### For readers who know electrodynamics

Surface resistance $R_s = \sqrt{\pi f\mu_0\rho}$ (normal conductor, $\rho$ the resistivity), $Q_0 = G/R_s$, where the geometry factor $G = \omega\mu_0\int\lvert H\rvert^2\dd V/\oint\lvert H\rvert^2\dd S$ depends only on the shape. A superconductor's BCS surface resistance is approximately $R_{\text{BCS}} \propto (f^2/T)\,\ee^{-\Delta/k_BT}$ ($\Delta$ the energy gap), plus a temperature-independent residual resistance; low frequency and low temperature help. When $V$ in $R/Q$ includes the transit-time factor, the result is an "effective shunt impedance" of the $ZT^2$ kind; mind the convention when comparing cavities.

## Going deeper: the transit-time factor — adding the pushes of the gaps as arrows {#journey-sc-deep-ttf}

The main text said that the speed window of a two-gap cavity is much wider than that of a nine-gap cavity. This section computes it with school vector addition.

### Step 1: the energy from one gap depends on the arrival time

The field in a gap oscillates as $\cos(\omega t + \varphi)$. A proton arriving at phase $\varphi$ gains energy proportional to $\cos\varphi$. Draw it as an arrow of length 1 at angle $\varphi$ to the horizontal: the horizontal projection $\cos\varphi$ of the arrow is the energy gained.

### Step 2: several gaps are several arrows head to tail

The total energy from $N$ gaps is the sum of the horizontal projections of $N$ arrows, i.e. the horizontal projection of the total arrow when they are placed head to tail. As long as the synchronous phase is chosen suitably (the total arrow pointing the way we want), the largest energy actually obtained is set by the **length of the total arrow**. Ideally (all arrows pointing the same way) the total length is $N$. The actual total length divided by $N$ is the transit-time factor due to the arrangement of the gaps.

### Step 3: the angle between neighbouring arrows

Neighbouring gaps are $\beta_g\lambda/2$ apart ($\beta_g$ the geometric β). A proton with speed $\beta$ needs $\beta_g\lambda/(2\beta c)$ to cover this, during which the RF phase advances by

$$
2\pi f\cdot\frac{\beta_g\lambda}{2\beta c} = \pi\,\frac{\beta_g}{\beta} .
$$

Since neighbouring gaps also have opposite fields (another turn by $\pi$), neighbouring arrows make an angle $\theta = \pi\beta_g/\beta + \pi$; dropping a full turn, this is equivalent to $\theta = \pi(\beta_g/\beta - 1)$. At $\beta = \beta_g$, $\theta = 0$: all arrows point the same way and the total length is $N$.

### Step 4: two gaps

Two arrows of length 1 at angle $\theta$ add to a total of length $2\cos(\theta/2)$ (the diagonal of a parallelogram, check it with the law of cosines). So the two-gap factor is $\lvert\cos(\theta/2)\rvert$. For it to be at least 0.8: $\theta/2 \le 36.9^\circ$, $\theta \le 73.7^\circ = 0.409\pi$, i.e. $\lvert\beta_g/\beta - 1\rvert \le 0.409$, so $\beta/\beta_g$ lies between $1/1.409 = 0.71$ and $1/0.591 = 1.69$. This is the 0.71–1.69 of the main text.

### Step 5: more gaps

With $N$ arrows, each turned by $\theta$ more than the previous one, placed head to tail they form a broken line on an arc of a circle. Geometry shows that the total length is $\lvert\sin(N\theta/2)/\sin(\theta/2)\rvert$ (for $N = 2$ this becomes $2\lvert\cos(\theta/2)\rvert$, in agreement with step 4). The larger $N$, the more the same $\theta$ curls the line up and the faster the total length falls: the range where the factor stays at least 0.8 is $\beta/\beta_g = 0.87$–$1.17$ for five gaps and $0.93$–$1.09$ for nine. This is exactly multi-slit interference: the light from each slit is an arrow, and the more slits, the narrower the bright fringes.

### Step 6: how fast the speed changes

At low energy $W = \tfrac12 mv^2$ and $\Delta v/v \approx \Delta W/(2W)$. One cavity adding 0.36 MeV at 1.527 MeV gives $\Delta v/v \approx 12\ \%$ (exactly 11 %: $\beta$ from 0.0570 to 0.0633), nearly leaving the window of a nine-gap cavity. The exact relativistic formula is $\Delta\beta/\beta = \Delta W/(mc^2\beta^2\gamma^3)$; at 500 MeV ($\beta = 0.758$, $\gamma = 1.533$) adding 10 MeV gives $\Delta\beta/\beta = 10/(938.3 \times 0.575 \times 3.60) = 0.5\ \%$.

### For readers who know complex numbers

The arrows of step 2 are the complex numbers $\ee^{\mathrm{i}\varphi}$, the total energy is $\mathrm{Re}\sum$, and the factor is

$$
\frac{T(\beta)}{T_g} = \frac{1}{N}\left|\sum_{n=0}^{N-1} (-1)^n\,\ee^{\,\mathrm{i}\,n\pi\beta_g/\beta}\right| = \frac{1}{N}\left|\frac{\sin(N\theta/2)}{\sin(\theta/2)}\right| ,
$$

obtained by summing the geometric series. The full $T(\beta)$ is the single-gap factor (the Fourier transform of the on-axis field, $\int E_z\cos(2\pi z/\beta\lambda)\,\dd z/\int E_z\,\dd z$) times this array factor.

## Going deeper: RF defocusing, solenoids and the phase advance {#journey-sc-deep-period}

The main text said that four tenths of the solenoid's focusing go into cancelling the cavity's defocusing and that the phase advance per period must stay below 90°. This section ties these numbers together with thin lenses and matrix multiplication.

### Step 1: how strong the cavity's defocusing is

The [medium-energy transport](#journey-mebt-deep-defocus) chapter showed that an RF cavity's longitudinal focusing comes with transverse defocusing, half as strong and of opposite sign. For a general synchronous phase the thin-gap transverse defocusing strength is

$$
\frac{1}{f_{\text{rf}}} = -\frac{\pi\,qV_0T\sin(-\varphi_s)}{mc^2\lambda(\beta\gamma)^3} .
$$

For the cavity of the first period: $\Delta W = qV_0T\cos\varphi_s = 0.206\ \mathrm{MeV}$, so $qV_0T = 0.238\ \mathrm{MeV}$; $\sin 30^\circ = 0.5$; the denominator is $938.27 \times 1.845 \times 1.860\times10^{-4} = 0.322\ \mathrm{MeV\,m}$. Hence $1/f_{\text{rf}} \approx -\pi \times 0.238 \times 0.5/0.322 = -1.16\ \mathrm{m^{-1}}$, the "about $1.2\ \mathrm{m^{-1}}$" of the main text.

### Step 2: the solenoid's focusing

The solenoid's thin-lens strength is $1/f = \int(B_z/2B\rho)^2\dd z$. Where it comes from (see [solenoid](#solenoid)): the radial fringe field gives the particle a rotation about the axis, and the rotation interacts with $B_z$ to produce a force towards the axis; each of the two steps contributes a factor $B_z/B\rho$, hence the square. For the example map `sol` with $K_b = 0.30$ the linear preview gives $1/f = 2.85\ \mathrm{m^{-1}}$ and a rotation of the beam by $35^\circ$.

### Step 3: the matrix of one period

Approximate one period as "one thin lens + a drift of length $L$" and multiply the 2 × 2 tables of lens and drift (see [drift](#drift-deep-matrix)):

$$
\begin{pmatrix} 1 & L \\ 0 & 1 \end{pmatrix}
\begin{pmatrix} 1 & 0 \\ -1/f & 1 \end{pmatrix}
= \begin{pmatrix} 1 - L/f & L \\ -1/f & 1 \end{pmatrix} .
$$

The [quadrupole](#quadrupole-deep-fodo) chapter showed that when particles swing stably, the phase advance per period $\sigma$ satisfies $\cos\sigma = $ half the sum of the diagonal elements of the table, here

$$
\cos\sigma = 1 - \frac{L}{2f} .
$$

### Step 4: insert the numbers

The period is $L = 0.76\ \mathrm{m}$ long. With the solenoid alone, $1/f = 2.85\ \mathrm{m^{-1}}$, $\cos\sigma = 1 - 0.76 \times 2.85/2 = -0.083$, $\sigma \approx 95^\circ$; simply subtracting the cavity's defocusing, $1/f = 2.85 - 1.16 = 1.69\ \mathrm{m^{-1}}$, $\cos\sigma = 0.358$, $\sigma \approx 69^\circ$. The linear preview, computing with the real thick elements, gives about $91^\circ$ with the cavity off and $76^\circ$ with it on. The rough estimate catches the trend: the cavity's defocusing lowers the phase advance by 15° to 25°, pulling a period that would be above 90° back below it. The estimate is off because it treats the 0.35 m solenoid as a thin lens and simply adds two lenses that are about 0.38 m apart.

### Step 5: where 90° comes from

$\cos\sigma = 1 - L/(2f)$ from step 3 requires $-1 < \cos\sigma < 1$, i.e. $0 < L/f < 4$: however strong the lens, it becomes unstable only at $180^\circ$; this is the stability condition for a single particle. The 90° rule is stricter and comes from the beam envelope. The envelope (beam size) oscillates with twice the phase of the particles, advancing $2\sigma$ per period; at $\sigma = 90^\circ$ the envelope advances $180^\circ$ per period, which together with a focusing structure that repeats once per period produces the swing-like parametric resonance of the main text. With space charge this resonance makes the envelope unstable ([lattice design](#lattice-design) has the derivation; AVAS's dynamics analysis has checked it: a FODO channel with a zero-current phase advance of about 95° has an unstable envelope when the current depresses the phase advance to about 83°). So designs keep the zero-current phase advance below 90°, with some margin for space charge.

### For graduate readers

A period with a solenoid is a 4 × 4 coupled problem. In coordinates rotating with the Larmor angle, a round solenoid splits into two identical 2 × 2 blocks for $x$ and $y$, and in laboratory coordinates the period matrix has the two eigenphases $\sigma_L \pm \theta_L$. From the linear preview's period matrix: with the cavity on the two eigenphases are about $41.6^\circ$ and $112.2^\circ$, their mean $76.9^\circ$ being the phase advance in Larmor coordinates and half their difference, $35.3^\circ$, the Larmor rotation; with the cavity off they are $56.2^\circ$ and $126.5^\circ$, mean $91.4^\circ$. Accelerating periods are not exactly periodic; the period matrix is multiplied by $\sqrt{(\beta\gamma)_{\text{out}}/(\beta\gamma)_{\text{in}}}$ before the Floquet analysis.

## Going deeper: why the bucket is about three times the synchronous phase wide {#journey-sc-deep-bucket}

The table of the main text says that at $\varphi_s = -30^\circ$ the bucket is $91^\circ$ wide, from $-61^\circ$ to $+30^\circ$. This section computes these numbers from energy conservation.

### Step 1: the phase deviation behaves like the position of a ball

Consider a proton with phase $\varphi$ (the synchronous particle has $\varphi_s$). At every cavity it gains more energy than the synchronous particle, in proportion to $\cos\varphi - \cos\varphi_s$. More energy means faster, so it arrives earlier at the next cavity and its phase becomes smaller. So:

- the rate of change of the energy deviation is proportional to $\cos\varphi - \cos\varphi_s$;
- the rate of change of the phase is proportional to the energy deviation, with opposite sign.

Together, the "acceleration" of the phase is proportional to $-(\cos\varphi - \cos\varphi_s)$. This is the same law as a ball rolling on an undulating track: think of $\varphi$ as the ball's position, and the track height $U(\varphi)$ obeys "force on the ball = rate at which the height falls".

### Step 2: the shape of the track

The track height satisfying this is

$$
U(\varphi) = \sin\varphi - \varphi\cos\varphi_s .
$$

(Check: increasing $\varphi$ by a small step $h$ raises $U$ by about $(\cos\varphi - \cos\varphi_s)\,h$, so the rate of fall is $-(\cos\varphi - \cos\varphi_s)$, the force of step 1.) $\sin\varphi$ is a wave and $-\varphi\cos\varphi_s$ tilts it as a whole: a **tilted washboard**. The bottom of the valley is at $\varphi = \varphi_s$, and the nearest hilltop to the right of it at $\varphi = -\varphi_s$.

### Step 3: when the ball is caught

The ball rolls to and fro in the valley with "kinetic energy + height" conserved. If its total energy is below the hilltop, it can never leave the valley: it is a particle held in the bucket. The path that starts exactly at the hilltop is the boundary of the bucket (the **separatrix**), and its turning point $\varphi_2$ on the other side of the valley is as high as the hilltop:

$$
U(\varphi_2) = U(-\varphi_s) .
$$

### Step 4: insert −30°

$\varphi_s = -30^\circ = -0.5236\ \mathrm{rad}$, $\cos\varphi_s = 0.8660$. The hilltop height is $U(30^\circ) = 0.5 - 0.5236 \times 0.8660 = 0.0466$. Look for a point of the same height on the left side of the valley, trying with a calculator:

- $\varphi = -60^\circ$: $\sin = -0.8660$, $-\varphi\cos\varphi_s = 1.0472 \times 0.8660 = 0.9069$, $U = 0.0409$, slightly low;
- $\varphi = -61^\circ$: $U = -0.8746 + 1.0647 \times 0.8660 = 0.0474$, slightly high.

So $\varphi_2 \approx -61^\circ$ and the bucket runs from $-61^\circ$ to $+30^\circ$, $91^\circ$ wide. The same procedure gives $\varphi_2 \approx -40^\circ$ and a width of $60^\circ$ for $-20^\circ$, and $\varphi_2 \approx -82^\circ$ and $122^\circ$ for $-40^\circ$, in agreement with the table of the main text.

### Step 5: why about 3 times

For a synchronous phase that is not too large, use the approximations $\sin x \approx x - x^3/6$ ($x$ in radians; check with a calculator: $\sin 0.5 = 0.4794$, $0.5 - 0.0208 = 0.4792$) and $\cos x \approx 1 - x^2/2$. Inserted into step 2, $U(\varphi) \approx \tfrac12\varphi_s^2\,\varphi - \tfrac16\varphi^3$. The hilltop is $U(-\varphi_s) = -\tfrac13\varphi_s^3$. The equation $U(\varphi_2) = U(-\varphi_s)$ multiplied by 6 reads

$$
3\varphi_s^2\,\varphi_2 - \varphi_2^3 + 2\varphi_s^3 = 0 ,
$$

which factorises as $-(\varphi_2 + \varphi_s)^2(\varphi_2 - 2\varphi_s) = 0$ (multiply out to check). Apart from the double root at the hilltop itself, the other root is $\varphi_2 = 2\varphi_s$. So the bucket runs from $2\varphi_s$ to $-\varphi_s$, $3\lvert\varphi_s\rvert$ wide. At $-30^\circ$ the approximation gives $90^\circ$; the exact value is $91^\circ$.

> **Think about it**: how wide is the bucket at $\varphi_s = 0$?
>
> Answer: $3 \times 0 = 0$. At $\varphi_s = 0$ the washboard is not tilted and has no valley: hilltop and valley bottom coincide, and any deviation slides away; the bunch cannot be held. This is where "the closer the synchronous phase is to 0 the better" goes wrong.

### For readers who know calculus

The longitudinal equations $\dd\Delta W/\dd s = qE_0T(\cos\varphi - \cos\varphi_s)$ and $\dd\varphi/\dd s = -2\pi\Delta W/(mc^2\beta^3\gamma^3\lambda)$ have the conserved quantity $H = \tfrac{\pi}{mc^2\beta^3\gamma^3\lambda}\Delta W^2 + qE_0T(\sin\varphi - \varphi\cos\varphi_s)$. The separatrix passes through the unstable fixed point $\varphi = -\varphi_s$, the largest energy deviation is at $\varphi = \varphi_s$, and the bucket height is $\Delta W_{\max} \propto \sqrt{\varphi_s\cos\varphi_s - \sin\varphi_s}$ ($\varphi_s < 0$ in AVAS's cosine convention). When the parameters change slowly during acceleration the bucket deforms slowly, and the bunch phase width is adiabatically damped by a law of the $(\beta\gamma)^{-3/4}$ kind (see [longitudinal motion](#longitudinal)).

## In AVAS {#journey-sc-in-avas}

### Cavities and solenoids

The superconducting section is AVAS's home ground, and it is exactly the example project `examples/hwr010`: its lattice has a single HWR cavity (field-map name `efield`, whose files are identical to those of `hwr010`, at a synchronous phase of $-33^\circ$), and its beam is the 1.527 MeV proton beam from the RFQ. Cavities are RF field maps (field type 1); solenoids can be magnetostatic field maps (field type 3) or the matrix element `solenoid`; quadrupoles at higher energy are `quad` or magnetostatic field maps.

```text
field  0.35  0.02  0  3  0        0    0     0.30   sol      ! solenoid map, Bz peak about 8.3*Kb T
field  0.21  0.02  0  1  162.5e6  -30  0.8   -0.8   hwr010   ! HWR, V3 = 0: phase is phi_s
```

```avas-params
field solenoid quad
```

> **AVAS convention (verified with the engine)**: field-map electric fields are in MV/m and multiplied by Ke, magnetic fields in T and multiplied by Kb. The RF electric field acts as $\cos(\omega t + \varphi_0)$, the RF magnetic field as $\sin(\omega t + \varphi_0)$. With `V3 = 0`, `phase` is the synchronous phase $\varphi_s = \operatorname{atan2}(\int E\sin\varphi\,\dd z, \int E\cos\varphi\,\dd z)$; `V3 = 1` is the entrance phase; `V3 = 2` is the absolute phase at $t = 0$ (see [the RF cavity](#rf-cavity)). The example cavity uses $K_b = -K_e$; the smoothed on-axis peak of the example solenoid map `sol` is about $8.3\,K_b$ T.

> **AVAS convention**: the example field maps are noisy (clear point-to-point fluctuations along $z$); the vertical stripes in slice plots are data, not a reading error (see [field maps](#field-maps)).

### Period markers and the phase-advance plot

Put the periodic structure between `lattice n1 n2` and `lattice_end` (`n1` is the number of elements per period, `n2` is 1), and the Results page's "phase advance" plot shows the phase advance per period (computed from the Twiss β of the actual beam in DataSet, so it equals the structure's own phase advance only when the beam is matched). Each period above has 4 elements (solenoid, drift, cavity, drift). See [lattice design](#lattice-design).

```avas-params
lattice lattice_end
```

### Superposed fields

When a solenoid and a cavity are so close that their fringe fields overlap, `superpose` makes both fields act together in the overlap; one superpose group may contain at most one RF cavity (see [superposed fields](#superpose)).

### Where the beam comes from

The input beam of the superconducting section comes from a simulation of the MEBT, or directly from the RFQ code's particle file (`readparticledistribution`, the example's `part_rfq.dst`), or from Twiss parameters. The output beam of a room-temperature DTL is handed to AVAS in the same two ways. To continue from an upstream section simulated with AVAS, dump the particle distribution at its end with `outputplane`.

```avas-params
beam:readparticledistribution outputplane
```

### Running and checking

- Tune `Kb`, `Ke` and the phases first with the linear preview of the visual editor: reference-particle energy agrees to about 0.01 % (the engine's mean energy feels the off-axis field and can differ by a few per cent of the gain), rms sizes usually within 1–2 %; longitudinal non-linearity and emittance growth do not show, and the multi-particle engine has the final word. [The guided tutorial](#tutorial) runs the engine in a sandbox for direct comparison.
- `steppercycle` in `input.txt` is the number of integration steps per RF period, 50 in the example; the cavity phases come only from `phase` and V3 of the field lines; no kernel reads `scanphase` or `scanData.txt`.
- `multithreading` may only be 1 or absent: 0 makes the engine lose all particles at the start (verified with the example).
- In a segmented run, phases with V3 = 2 must be converted with $\varphi_{RF} - 360^\circ\cdot f\cdot(t_{in} - T_{entry})$; AVAS's segmented runs do this automatically.

```avas-params
input:steppercycle input:scanphase input:multithreading
```

## Further reading {#journey-sc-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: transit-time factor, DTL, superconducting linacs, longitudinal and transverse RF dynamics.
- H. Padamsee, J. Knobloch, T. Hays, *RF Superconductivity for Accelerators*, Wiley: $Q_0$, $R/Q$ and cavity types of superconducting cavities.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: periodic focusing channels and envelope stability.
- CERN Accelerator School (CAS) proceedings on RF superconductivity and linacs.
- AVAS manual `docs/使用说明20260427.docx`: the `field` element and the `input.txt` keywords.
- This primer: [the RF cavity](#rf-cavity), [longitudinal motion](#longitudinal), [the solenoid](#solenoid), [lattice design](#lattice-design), [the guided tutorial](#tutorial).
