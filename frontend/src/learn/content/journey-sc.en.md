# Acceleration: the superconducting section from low to high β

Leaving the MEBT, our proton still has only 1.527 MeV and moves at 5.7 % of the speed of light; at the end it must have 1 GeV and 87.5 % of the speed of light, more than six hundred times the energy. Almost all of that comes from the superconducting section. Oddly, this is not one long accelerating pipe but several hundred small, independent cavities, grouped into families of completely different shapes, and at low energy nearly every cavity is followed by a solenoid. Why superconducting? Why can one cavity family serve only part of the way? Why do cavities and focusing elements alternate, and why is the synchronous phase chosen near $-30^\circ$ to $-20^\circ$? This chapter answers these questions one by one and builds the beginning of this section from the two field maps of the AVAS example project. It uses [the RF cavity](#rf-cavity), [longitudinal motion](#longitudinal) and [lattice design](#lattice-design); [the guided tutorial](#tutorial) is its hands-on version.

## Why superconducting cavities {#journey-sc-why-sc}

The RF fields in a cavity induce currents in its walls, and the currents heat the resistive walls. For an accelerating voltage $V$ the wall loss is

$$
P_c = \frac{V^2}{(R/Q)\,Q_0}
$$

$R/Q$ (here in the linac convention $R = V^2/P_c$; the circuit convention is half as large) depends only on the cavity's shape and $Q_0$ is the intrinsic quality factor (defined in [the RF cavity](#rf-cavity)). For the same shape, a room-temperature copper cavity has $Q_0$ of about $10^4$, a superconducting niobium cavity $10^9$ to $10^{10}$: five to six orders of magnitude apart.

A number. The example HWR map at $K_e = 1.36$ gives about 1 MV; take $R/Q = 150\ \Omega$ (low-β cavities typically have tens to hundreds of ohms; this is only for illustration) and continuous-wave (CW) operation. A copper cavity with $Q_0 = 10^4$ dissipates about 670 kW in its walls; a superconducting one with $Q_0 = 5\times10^9$ only 1.3 W. The power this cavity gives to the beam is about 65 W for the example's 0.18 mA, and only a few kilowatts even at 10 mA. Spending 670 kW to accelerate a beam of tens of watts to a few kilowatts is plainly impossible. The superconducting cavity's 1.3 W is released at 2–4 K and has to be pumped to room temperature by a refrigerator: an ideal Carnot machine needs 66 W of electric power per watt removed at 4.5 K, a real plant usually several hundred. Even so, each cavity costs below a kilowatt.

For CW or high-duty-factor machines, superconductivity is therefore almost the only choice. It brings two more advantages:

- **The aperture can be larger.** A room-temperature cavity keeps its beam hole small to raise the shunt impedance and reduce wall losses; a superconducting cavity has small losses anyway, so its aperture can be larger, which lowers the risk of beam loss.
- **Every cavity has its own RF supply and its own phase.** If one cavity fails, the phases and amplitudes of the others can be retuned to compensate (a typical practice). The room-temperature drift-tube structure of the next section cannot do this.

> **Think about it**: if superconductivity is so good, why do many pulsed proton linacs still use room-temperature structures?
> Answer: wall losses count in average power. If a pulsed machine has a duty factor of a few per cent, the average wall loss of a copper cavity drops by the same factor, and the peak power can come from a pulsed RF source; superconductivity, on the other hand, needs a cryogenic plant, which may not pay off at low duty factor. The advantage of superconductivity is greatest for CW and high duty factor.

## The other way: the room-temperature drift-tube linac {#journey-sc-dtl}

> **AVAS does not simulate this section**: a room-temperature drift-tube linac (DTL) has no dedicated model. If its 3D field map is available it could in principle be read as a `field` element; this primer does not cover that. Its output beam is handed to AVAS as a particle file or as Twiss parameters, like any other (see [In AVAS](#journey-sc-in-avas)).

Besides the superconducting section, a common accelerating structure at low and medium energy is the Alvarez DTL: a long cylindrical cavity operating in the TM010 mode (the "0 mode"), with a string of drift tubes suspended along its axis. In the 0 mode all gaps have the field in the same direction at the same moment, so the proton must cross exactly one cell per RF period, and the cell length equals $\beta\lambda$. During the half period in which the field points backwards, the proton is hidden inside a drift tube and feels no decelerating field. At 162.5 MHz this length is 105 mm at 1.527 MeV and 267 mm at 10 MeV; DTLs usually run at a few hundred MHz, where the cells are shorter.

The drift tubes contain quadrupoles that form a FODO-type transverse focusing lattice; at low energy RF defocusing and space charge are both strong and focusing is needed every cell or two, which is exactly what quadrupoles inside the drift tubes provide. The cell lengths of a DTL are machined one by one for a design velocity profile, and once built, the tank fits only that one $\beta(z)$ curve: if part of the field is wrong, it cannot be rephased separately as an independent superconducting cavity can.

## Why one cavity family covers only a range of β {#journey-sc-ttf}

The [overview](#journey-overview-velocity) used the transit-time factor $T(\beta)$ to show that a cavity family fits only a range of velocities: the example HWR map has its largest $T$, about 0.82, at $\beta \approx 0.104$; $T$ stays above 70 % of that for $\beta = 0.066$–$0.21$ (about 2–22 MeV); at the example beam's $\beta = 0.057$ it is only 0.40. Two further questions here: what sets the width of that window, and why small two-gap cavities are used at low energy and multi-cell elliptical cavities at high energy.

First the map itself. Below is the field of hwr010: the on-axis $E_z$ has two peaks of opposite sign at $z \approx 0.074\ \mathrm{m}$ and $0.140\ \mathrm{m}$, $d = 0.066\ \mathrm{m}$ apart. The fields of the two gaps point in opposite directions, so the proton must get from one gap to the other in half an RF period to be accelerated twice: $d = \beta\lambda/2$, i.e. $\beta = 2d/\lambda \approx 0.072$. Yet the maximum of $T$ is at 0.104, because each gap has a finite length of its own; the transit-time factor of a single gap grows with $\beta$, and multiplied together the maximum shifts towards higher β.

```avas-field
map hwr010
```

Leave the single-gap factor aside and consider $N$ identical gaps in a row, adjacent fields reversed, spaced $\beta_g\lambda/2$ ($\beta_g$ is the geometric β). A proton with velocity $\beta$ accumulates a phase difference $\pi\beta_g/\beta$ between neighbouring gaps, plus $\pi$ for the field reversal, and the contributions of the $N$ gaps add up:

$$
\frac{T(\beta)}{T_g} = \frac{1}{N}\left|\sum_{n=0}^{N-1} (-1)^n\,\ee^{\,\mathrm{i}\,n\pi\beta_g/\beta}\right|
$$

At $\beta = \beta_g$ every term equals 1 and the sum is $N$; away from $\beta_g$ the phases of the terms drift apart and cancel. The larger $N$, the faster they cancel: this factor stays above 0.8 for $\beta/\beta_g = 0.71$–$1.69$ with two gaps, $0.87$–$1.17$ with five and $0.93$–$1.09$ with nine. It is exactly the physics of a grating: the more slits, the narrower the diffraction peak.

Next, how much the velocity changes per cavity. At 1.527 MeV one cavity adds 0.36 MeV and $\beta$ goes from 0.0570 to 0.0633, up 11 %; at 500 MeV, even 10 MeV from one cavity changes $\beta$ by only 0.5 % ($\dd\beta/\beta = \dd W/(mc^2\beta^2\gamma^3)$). At low energy the velocity changes fast, so the cavities must have few gaps and a wide window: two-gap HWRs and QWRs or few-gap spoke cavities. At high energy the velocity hardly changes, and elliptical cavities with five to nine cells can give a much larger voltage per cavity.

A common division (typical values, different between machines): HWRs have a geometric β near 0.1; spoke cavities 0.2–0.5, corresponding to about 19–145 MeV; elliptical cavities above 0.5. Our proton passes through these families in turn. The [overview](#journey-overview-frequency) also explains why the frequency can jump from 162.5 MHz to 325 MHz and 650 MHz when the family changes.

## The period: solenoid + cavity {#journey-sc-period}

Why not join a dozen cavities and focus once at the end? Because every cavity defocuses the beam transversely. Longitudinal focusing (a negative synchronous phase) always comes with transverse defocusing, with a strength $\propto \sin(-\varphi_s)/(\beta\gamma)^3$ (see [the RF cavity](#rf-cavity)). $1/(\beta\gamma)^3$ is 5378 at 1.527 MeV and 319 at 10 MeV: at low energy the defocusing is very strong. A rough thin-gap estimate for the cavity of the first period below ($K_e = 0.8$, $\varphi_s = -30^\circ$) gives a defocusing strength of about $1.2\ \mathrm{m^{-1}}$ per cavity, a focal length below 1 m. Space charge is strongest in the same place. So at low energy every cavity must be followed by a focusing element.

Solenoids do the focusing, for three reasons. A solenoid focuses $x$ and $y$ at the same time, unlike a quadrupole that focuses one plane and defocuses the other, so one element is enough; it is short and compact and can sit in the cryomodule with the cavities (as a superconducting solenoid); and it is strong at low energy. Its thin-lens focusing strength is

$$
\frac{1}{f} = \int\left(\frac{B_z}{2B\rho}\right)^2\dd z
$$

For the example map `sol` ($K_b = 0.30$, on-axis peak about 2.5 T) at 1.527 MeV this gives $1/f = 2.85\ \mathrm{m^{-1}}$, a focal length of 0.35 m, while the beam rotates by 35° about the axis (the Larmor angle). Compared with the cavity's defocusing of $1.2\ \mathrm{m^{-1}}$, some forty per cent of the solenoid's focusing goes into cancelling the RF defocusing.

But a solenoid's strength falls as $1/(B\rho)^2$, a quadrupole's only as $1/B\rho$ ($k = G/B\rho$). From 1.527 MeV to 10 MeV, $B\rho$ rises from 0.179 to 0.458 T·m: the same solenoid then focuses 6.6 times more weakly, the same quadrupole only 2.6 times; at 150 MeV the factors are 106 and 10. At higher energy the focusing therefore switches to quadrupole doublets or FODO; by then the RF defocusing is also much weaker, and the cavities can be spaced further apart.

The central rule of period design: the zero-current phase advance per period must stay below 90°, transversely and longitudinally (it comes from the parametric resonance of the envelope; derivation in [lattice design](#lattice-design)). The beam line below consists of six "solenoid + cavity" periods built from the example maps, each 0.76 m long. The first two cavities have $K_e = 0.8$, the middle two 1.0, the last two 1.2; the solenoid $K_b$ rises from 0.30 to 0.40 in steps. From the period matrices of the linear preview, the longitudinal phase advances of the six periods are about 86°, 83°, 89°, 80°, 80° and 70°, the transverse ones (in Larmor coordinates) about 76°, 77°, 74°, 72°, 69° and 67°, all below 90° (accurate to about 1°; the preview's period matrix for the noisy solenoid map is slightly non-symplectic, see the remark in the [tutorial](#tutorial-step4)).

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

The energy curve in the 2D view is a staircase: one step per cavity, the steps 0.21, 0.25, 0.37, 0.43, 0.56 and 0.59 MeV high, reaching 3.94 MeV after six periods. The steps grow partly because $K_e$ rises and partly because the proton approaches the velocity the cavity is made for, so $T(\beta)$ grows. The rms envelope swings between 0.7 and 2.0 mm, with a largest value of about 2.0 mm, comfortably inside the 20 mm aperture.

The envelope does not repeat exactly from period to period: here the RFQ beam enters the first period directly, without an MEBT, with a mismatch factor of about 0.3–0.4, and the rms $x$ at the period entrances jumps between 0.94 mm and 1.64 mm. The beam line of the [MEBT](#journey-mebt) chapter puts an MEBT in front, and the same periods then vary much less.

> **Think about it**: in [the guided tutorial](#tutorial) all six periods use $K_e = 1.36$ and $K_b = 0.35$ and reach 4.99 MeV, much more than 3.94 MeV here. Why are the first cavities turned down?
> Answer: to keep the 90° rule. From the period matrices of the linear preview, with all cavities at full field the first period has a longitudinal phase advance of about 128° and a transverse one of about 94°, both above 90°; only in the third period do they drop to about 95° and 62°. The longitudinal phase advance goes as $\sqrt{E_0T\sin(-\varphi_s)/(\beta\gamma)^3}$, and at $\beta = 0.057$ the cavity is "too strong". Turning the first cavities down costs about 1 MeV and buys a stable working point in every period. Real designs also often keep the cavities below full gradient at the low-energy end.

## The synchronous phase: a compromise between acceleration and stability {#journey-sc-phase}

The synchronous phase $\varphi_s$ is a lever with energy at one end and stability at the other. The energy gain is $\propto\cos\varphi_s$, larger towards 0; the phase width of the longitudinal stable region is about $3|\varphi_s|$, wider for more negative phases; longitudinal focusing (and the transverse RF defocusing that comes with it) is $\propto\sin(-\varphi_s)$. The exact stable widths and the other two ends of the lever, computed from the example maps for the first two periods (all $K_e = 1.36$, $K_b = 0.35$):

| $\varphi_s$ | $\cos\varphi_s$ | Stable phase width | Energy after two periods | Longitudinal phase advance, period 1 |
|---|---|---|---|---|
| $-20^\circ$ | 0.940 | 60° | 2.497 MeV | 107° |
| $-30^\circ$ | 0.866 | 91° | 2.374 MeV | 128° |
| $-40^\circ$ | 0.766 | 122° | 2.226 MeV | 149° |

(The stable widths use the separatrix for constant parameters, see [longitudinal motion](#longitudinal); the phase advances come from the linear preview.)

At $-20^\circ$ there is more energy, but the stable region is only 60° wide, while the bunch entering the superconducting section (about 5° rms after the illustrative MEBT of the [MEBT](#journey-mebt) chapter), its tails and the various errors need a good margin; at $-40^\circ$ the stable region is wide, but $\cos\varphi_s$ is 18 % smaller (in the linear preview the energy gain of the first two periods is actually 28 % smaller, because at low energy $V_0T$ also changes with the phase), and both the longitudinal focusing and the transverse RF defocusing are stronger. That is why about $-30^\circ$ is common at the low-energy end. As the beam accelerates, the bunch's phase width shrinks by adiabatic damping (see [the overview](#journey-overview-frequency): about 2.4° rms at 10 MeV), and $|\varphi_s|$ can be reduced gradually to about $-20^\circ$ for more energy.

The lab below is a single HWR cavity. Drag the phase from $-30^\circ$ to $-60^\circ$ and to $0^\circ$ and watch the energy gain, rms $z$ and rms $x$ trade against each other: the closer to 0, the more energy, but the less the bunch is held together longitudinally.

```avas-lab
rf-cavity phase=-30 Ke=0.8
```

> **Think about it**: why must the synchronous phase be negative? Doesn't $+30^\circ$ give the same energy gain?
> Answer: $\cos$ is even, so the synchronous particle gains the same at $\pm 30^\circ$; for the other particles of the bunch the result is opposite. In AVAS's cosine convention, with $\varphi_s < 0$ a late proton has a larger phase, closer to the crest, gains more energy and catches up; an early one gains less and slows down: the bunch is focused longitudinally. With $\varphi_s > 0$ it is the other way round and the bunch spreads. The price is that longitudinal focusing always comes with transverse defocusing; conversely, at $\varphi_s > 0$ the beam is focused transversely but unstable longitudinally. No phase focuses both directions at once, which is exactly why solenoids are added.

## How much each period gains, and how long the section is {#journey-sc-energy-gain}

Continuing the six periods above, with $K_e = 1.36$ for the later cavities and the solenoid $K_b$ still rising step by step, the linear preview reaches 10 MeV after about 16 periods, 12 m: the typical exit energy of an HWR section (see [the overview](#journey-overview-thread)). There each cavity adds about 0.6 MeV, the longitudinal phase advance has fallen to about 30° and the transverse one stays at 60°–70°. Such a low longitudinal phase advance means that towards higher energy one solenoid can serve two or more cavities, giving more acceleration per metre: a common layout of low-β superconducting sections (for example "solenoid–cavity–cavity"), see [lattice design](#lattice-design).

The average accelerating gradient of this section (cavities, solenoids and drifts averaged together) is about 0.5–0.8 MeV/m, far below the field in a single cavity: the cavity fills less than a third of the period, and at the low-energy end the cavities are turned down and $T(\beta)$ is not at its best. With the assumptions of [the overview](#journey-overview-thread) (an average $E_0T$ of 1 MV/m at $\varphi_s = -25^\circ$ in the spoke section, 3 MV/m at $-20^\circ$ in the elliptical section), going from 10 MeV to 150 MeV takes about 155 m and from 150 MeV to 1 GeV about 300 m. The superconducting section is by far the longest part of the machine.

Along the way our proton's velocity goes from 0.057 to 0.875 and $B\rho$ from 0.18 T·m to 5.66 T·m; the normalized emittance ideally stays the same while the geometric emittance shrinks as $1/(\beta\gamma)$, by a factor of 32; the bunch length in millimetres changes little, but in RF phase it drops from a few degrees to below 1° (see [the overview](#journey-overview-thread)).

## Common misconceptions {#journey-sc-misconceptions}

**"The geometric β of a cavity is the proton's velocity in it."** The geometric β is set by the gap spacing, and the maximum of $T(\beta)$ also depends on the length of a single gap; for the example hwr010 they are about 0.072 and 0.104. A cavity family must in any case work over a wide range on both sides.

**"The closer the synchronous phase is to 0, the better, because the energy gain is largest."** At $\varphi_s = 0$ the longitudinal stable region has zero width and the bunch cannot be held.

**"A cavity's energy gain is $qV_0T\cos\varphi_s$ with a constant $V_0T$."** At low energy, when one cavity adds more than about 20 % of the entrance energy, the proton accelerates noticeably inside the cavity and $V_0T$ depends on the phase and the amplitude (see [the RF cavity](#rf-cavity)). AVAS integrates through the field map and does not use this approximation.

**"A solenoid only focuses and does nothing else."** A solenoid rotates the transverse phase space about the axis (about 35° per period in the example above) and couples $x$ and $y$. A round beam does not show it; a non-round or off-centre beam does. The transverse phase advance must be computed in Larmor coordinates.

**"Phase advance below 90° is a transverse rule."** It applies longitudinally too; at low β it is precisely the longitudinal one that exceeds it first.

**"Weaker focusing at high energy is a problem."** As the focusing weakens, RF defocusing and space charge weaken even faster ($\propto 1/(\beta\gamma)^3$), so high-energy sections can use longer periods and fewer focusing elements.

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

- Tune `Kb`, `Ke` and the phases first with the linear preview of the visual editor: energy agrees to about 0.01 %, rms sizes usually within 1–2 %; longitudinal non-linearity and emittance growth do not show, and the multi-particle engine has the final word. [The guided tutorial](#tutorial) runs the engine in a sandbox for direct comparison.
- `steppercycle` in `input.txt` is the number of integration steps per RF period, 50 in the example; `scanphase` decides whether the cavity phases are scanned or read from `scanData.txt`; the linear preview ignores it.
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
