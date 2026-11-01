# Shaping: the medium-energy beam transport

The RFQ has just brought our proton to 1.527 MeV and has already cut the continuous beam into neat bunches. The next stop ought to be the superconducting cavities, so why put two or three metres of beam line, or more, in between, without a single accelerating cavity in it? What would go wrong if the first cryomodule were attached directly to the RFQ? This chapter shows that the beam leaving the RFQ "does not fit" the next section, transversely or longitudinally, and that the bunches lengthen by themselves once they leave the RFQ. The medium-energy beam transport (MEBT) is the section that reshapes the beam; on the way it also chops, measures and collimates. The chapter continues from [the RFQ](#journey-rfq) and uses ideas from [matching](#matching) and [longitudinal motion](#longitudinal); the numbers for every stage are collected in [the overview](#journey-overview-thread).

## The beam as it leaves the RFQ {#journey-mebt-rfq-exit}

First, where our proton stands. Kinetic energy $W = 1.527\ \mathrm{MeV}$, $\beta = 0.05698$, $\beta\gamma = 0.05708$, magnetic rigidity $B\rho = 0.1786\ \mathrm{T\,m}$; the bunches advance at 162.5 MHz, 6.15 ns apart, which is $\beta\lambda = 105.1\ \mathrm{mm}$. The example project's Twiss parameters come from the RFQ output file `part_rfq.dst`: transversely $\alpha_x = -0.658$, $\beta_x = 0.377\ \mathrm{mm/mrad}$, normalized rms emittance $\varepsilon_{n,x} = 0.138\ \pi\,\mathrm{mm\,mrad}$, and similar in $y$. In quantities one can picture: rms size $\sqrt{\beta_x\varepsilon_{n,x}/(\beta\gamma)} = 0.954\ \mathrm{mm}$, rms angle 3.03 mrad, and $\alpha_x < 0$ means the beam is opening up. Longitudinally the bunch has an rms length of 1.24 mm (4.3° of phase, 73 ps), an rms momentum spread of 0.375 % and an energy spread of 11.4 keV.

Three things about this beam are wrong for what comes next.

First, it is diverging transversely. The electric quadrupole focusing of the RFQ stops abruptly at its exit; the beam leaves at some phase of the RFQ's last focusing period and spreads with an angle of 3 mrad. Left alone, it grows from 0.95 mm to about 3.7 mm rms within one metre of drift.

Second, it has the wrong shape. The "solenoid + cavity" periods of the superconducting section downstream have their own matched beam (the periodic Twiss parameters, see [matching](#matching)). For the first period used in the [superconducting section](#journey-sc) chapter, the periodic matrix of the linear preview gives matched parameters of about $\alpha \approx -0.78$, $\beta \approx 0.61$–$0.67\ \mathrm{mm/mrad}$ at its entrance (similar in $x$ and $y$). Compared with these, the RFQ beam has a mismatch factor of about 0.3–0.4 in $x$ and $y$ and about 0.2 longitudinally (the transverse values depend at the level of a few hundredths on how the coupled, slightly non-symplectic period matrix of the noisy solenoid map is reduced to Larmor coordinates). With the definition of the [matching](#matching) chapter, a mismatch factor of 0.35 means the envelope beats between $1/1.35$ and $1.35$ times the matched envelope, and with space charge it drives halo.

Third, the bunch lengthens by itself; the next section is about that.

There are engineering needs as well: the entrance of a cryomodule needs vacuum separation and a warm-to-cold transition, and there must be room for a chopper, beam instrumentation and scrapers. All of that takes length between the RFQ and the superconducting section, and for the beam, length means drift, which makes all three problems worse. The MEBT is therefore not an optional connecting pipe but a beam line that must be designed with care.

## Why a bunch lengthens by itself {#journey-mebt-debunching}

Think of runners crossing the start line at the same moment, each at a slightly different speed. Nobody pushes them, yet the group spreads out: the fast ones move ahead, the slow ones fall behind. A bunch does the same. Each proton has a slightly different momentum, relative deviation $\delta = \Delta p/p$, and its relative velocity deviation is

$$
\frac{\Delta v}{v} = \frac{1}{\gamma^2}\,\frac{\Delta p}{p} = \frac{\delta}{\gamma^2}
$$

(differentiate $p = \gamma m v$: $\dd p/p = \gamma^2\,\dd v/v$). Over a length $L$ the longitudinal position $z$ of a proton relative to the synchronous particle changes by

$$
\Delta z = \frac{L}{\gamma^2}\,\delta ,\qquad R_{56} = \frac{L}{\gamma^2}
$$

$R_{56}$ is the element of the drift's transfer matrix that maps $\delta$ to $z$. At low energy $\gamma \approx 1$, so the momentum spread turns into position spread almost one to one: the example's rms momentum spread of 0.375 % adds about 3.7 mm of rms length per metre. At high energy the $1/\gamma^2$ weakens the effect; at 1 GeV only a quarter is left.

With the full Twiss parameters ($\alpha_z$ is small, so the bunch is nearly at a waist) and without space charge, one metre of drift takes the rms length from 1.24 mm to 3.89 mm, the phase width from 4.3° to 13.3°; after 2.7 m (the length of the MEBT below) it is about 10 mm, or about 35°. Note that no force acts inside the bunch and the longitudinal phase-space area does not change (Liouville's theorem): the phase ellipse is only sheared, and the length is paid for by an ever stronger correlation between position and energy.

Why not let it lengthen? With a synchronous phase of $-30^\circ$ in the superconducting section, the longitudinal stable region extends from about $-61^\circ$ to $+30^\circ$, about 91° wide (see [the overview](#journey-overview-bunching)). A bunch with an rms phase width of 35° covers 140° at $\pm 2\sigma$, so many particles necessarily fall outside the stable region; even inside it, particles near the boundary sit on the non-linear part of the RF wave and the longitudinal emittance grows. The bunch must be shortened again before it enters the superconducting section, and shortened to the "matched" length and energy spread that the longitudinal focusing downstream requires.

Space charge makes things worse: within a bunch, protons at the head are pushed forward by the charge behind them and those at the tail backward, so the bunch is also pushed apart longitudinally. The shorter the bunch, the higher its peak current: for a Gaussian bunch at the RFQ exit the peak current is about 34 times the average (see [the overview](#journey-overview-space-charge)), so the example's average 0.18 mA corresponds to about 6 mA in the bunch centre.

> **Think about it**: if the bunch lengthens in a drift while its longitudinal emittance stays the same, can it "shorten back" on its own after the drift?
> Answer: not in a drift. A drift keeps shearing the ellipse in the same direction (the fast ones stay ahead). To shorten the bunch, the protons at the head must be slowed down and those at the tail sped up, reversing the correlation between position and energy. That needs a time-dependent longitudinal force: a buncher.

## The buncher: a longitudinal lens that does not accelerate {#journey-mebt-buncher}

A buncher is an RF cavity with its phase set at a special point. In AVAS's cosine convention the reference particle sees the field as $\cos\varphi$, and the energy gain is $\Delta W = qV_0T\cos\varphi_s$. With $\varphi_s = -90^\circ$ the synchronous particle crosses when the field passes through zero, and its energy does not change. A proton that arrives earlier (at the head of the bunch, $z > 0$) has a smaller phase, sees a negative field and is decelerated; a late one is accelerated.

In formulas: a proton at a distance $z$ from the synchronous particle arrives with a phase offset $\Delta\varphi = -2\pi z/(\beta\lambda)$, so

$$
\Delta W = qV_0T\left[\cos(\varphi_s + \Delta\varphi) - \cos\varphi_s\right] \approx -qV_0T\sin\varphi_s\,\Delta\varphi
$$

At $\varphi_s = -90^\circ$, $\sin\varphi_s = -1$ and $\Delta W = -2\pi qV_0T\,z/(\beta\lambda)$: the energy change is proportional to $z$, with the sign that makes the head slower and the tail faster. With $\dd p/p = \dd W/(\beta^2\gamma\,mc^2)$ this becomes a momentum change

$$
\Delta\delta = -K z ,\qquad K = \frac{2\pi\, qV_0T}{mc^2\,\lambda\,\beta^3\gamma}
$$

which is exactly a thin lens: a change in "angle" proportional to the position and directed towards the centre. Together with the drift's $R_{56} = L/\gamma^2$, a bunch without initial correlation reaches its shortest length a distance

$$
L_f = \frac{\gamma^2}{K}
$$

downstream; $L_f$ is the longitudinal focal length.

With the example's numbers, $mc^2\lambda\beta^3\gamma/(2\pi) = 0.0511\ \mathrm{MeV\,m}$, so every 51 kV of effective voltage $V_0T$ gives $1\ \mathrm{m^{-1}}$ of longitudinal focusing strength. The first buncher in the beam line below borrows the example HWR field map at $K_e = 0.195$; its largest energy gain (on crest) is about 57.6 keV, so $K \approx 1.13\ \mathrm{m^{-1}}$ and the longitudinal focal length is about 0.89 m. A few tens of kilovolts are enough. That is the advantage of low energy: $K$ contains $1/\beta^3$, so at low velocity a small energy difference produces a large velocity difference.

The buncher has a price. By Maxwell's equations longitudinal focusing always comes with transverse defocusing, and the RF defocusing strength is $\propto\sin(-\varphi_s)$ (see [the RF cavity](#rf-cavity)). At $\varphi_s = -90^\circ$, $\sin(-\varphi_s) = 1$ is largest, so for a given voltage the buncher is the most strongly defocusing cavity of all. In the thin-gap approximation (derived in [the RF cavity](#rf-cavity)) the transverse strength is half the longitudinal one, with the opposite sign: $1/f_x = -K/(2\gamma^2) \approx -0.56\ \mathrm{m^{-1}}$, a defocusing focal length of about 1.8 m. The MEBT quadrupoles have to take this into account.

In the lab below, with the phase at $-90^\circ$ and $K_e = 0.2$, the energy gain reads 0, yet the rms $z$ curve no longer rises after the cavity as it would in a drift. The lab's beam line is only 0.46 m long (its beam is the example `beam.txt`, rms $z$ 1.32 mm at the entrance): the exit rms $z$ is 2.06 mm at $K_e = 0$, 1.68 mm at $K_e = 0.2$ and 1.37 mm at $K_e = 0.4$, while the exit rms $x$ grows from 2.09 mm to 2.35 mm and 2.61 mm. That growth is the RF defocusing.

```avas-lab
rf-cavity phase=-90 Ke=0.2
```

> **Think about it**: what happens if the buncher's phase is changed from $-90^\circ$ to $+90^\circ$?
> Answer: the synchronous particle still gains nothing, but now the head is accelerated and the tail decelerated, so the bunch grows longer downstream: a longitudinally defocusing lens, which in turn focuses transversely. The buncher's phase must be on the "slow head, fast tail" side, which in AVAS's convention is $-90^\circ$.

## Matching: why so many knobs {#journey-mebt-matching}

The core task of the MEBT is matching: turning the RFQ's $(\alpha, \beta)$ into the values the superconducting section requires, in all three planes. Each plane has two constraints, $\alpha$ and $\beta$, so four transversely and two longitudinally. The emittance is a property of the beam, not a matching variable. At least four independent transverse knobs and two longitudinal ones are therefore needed (see [matching](#matching)).

Why quadrupoles transversely, rather than solenoids as in the LEBT? At 1.5 MeV a solenoid, whose strength goes as $(B/B\rho)^2$, is still quite effective, but it acts identically on $x$ and $y$ and can only make the two planes the same; the two planes of an RFQ beam are in general not symmetric, and quadrupoles can adjust them separately. A quadrupole's strength is $k = G/B\rho$: with the example's $B\rho = 0.1786\ \mathrm{T\,m}$, a gradient of 7 T/m gives $k = 39\ \mathrm{m^{-2}}$, and a 0.1 m long quadrupole has a focal length of about 0.26 m, so a compact triplet catches the diverging beam within a metre or two.

Longitudinally two bunchers act as two longitudinal lenses; their amplitudes together set the bunch length and the energy–position correlation at the entrance of the superconducting section.

The beam line below is an illustrative MEBT laid out along these lines: a quadrupole triplet catches the diverging beam, followed by a buncher, a quadrupole doublet, a second buncher and a final doublet, 2.69 m in total; behind it come the first two periods of the superconducting section. The two bunchers borrow the example HWR field map ($K_e = 0.195$ and $0.182$, phase $-90^\circ$), only because it is a field map at hand; the bunchers of a real MEBT are usually normal-conducting cavities with a different field distribution. The seven gradients and two buncher amplitudes were found with the transfer matrices of the linear preview at zero current, so that the mismatch factor at the entrance of the superconducting section is about 2 % or less transversely and below 1 % longitudinally (at this level the matched parameters themselves are uncertain, for the reason given above).

```avas-beamline
! ---- MEBT: triplet, buncher, doublet, buncher, doublet ----
drift 0.1 0.02 0
quad 0.1 0.02 0 3.33
drift 0.08 0.02 0
quad 0.1 0.02 0 -8.33
drift 0.08 0.02 0
quad 0.1 0.02 0 6.41
drift 0.25 0.02 0
field 0.21 0.02 0 1 162.5e6 -90 0.195 -0.195 hwr010
drift 0.25 0.02 0
quad 0.1 0.02 0 -7.09
drift 0.08 0.02 0
quad 0.1 0.02 0 7.12
drift 0.25 0.02 0
field 0.21 0.02 0 1 162.5e6 -90 0.182 -0.182 hwr010
drift 0.25 0.02 0
quad 0.1 0.02 0 -5.15
drift 0.08 0.02 0
quad 0.1 0.02 0 3.79
drift 0.15 0.02 0
! ---- first two SC periods: solenoid + HWR ----
field 0.35 0.02 0 3 0 0 0 0.3 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 0.8 -0.8 hwr010
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.32 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 0.8 -0.8 hwr010
drift 0.3 0.02 0
```

Look at the energy curve in the 2D view: through the MEBT the energy stays at 1.527 MeV (the bunchers add nothing); in the superconducting section the two cavities raise it to 1.733 MeV and 1.985 MeV. Then the envelope: in the MEBT the beam alternately widens and narrows in $x$ and $y$ between the quadrupoles, with a largest rms size of about 3.7 mm (aperture 20 mm, so 3 × rms stays inside); at the end of the MEBT the rms $x$ and $y$ are back to about 1.2 mm and the rms bunch length is 1.48 mm (5.1°). With both bunchers switched off ($K_e = 0$) the rms bunch length at the end of the MEBT would be about 10 mm, the 35° of the previous section.

The effect of matching shows best in the superconducting section: the rms $x$ at the entrance of each period is 1.24, 1.32 and 1.15 mm; what variation remains comes from the acceleration itself (the energy changes, so a strictly periodic solution does not exist). For comparison, if the RFQ beam enters the same periods directly without an MEBT (as in the beam line of the [superconducting section](#journey-sc) chapter), the rms $x$ at the period entrances is 1.05, 1.66, 0.99 and 1.42 mm, a beat of about ±25 %: what a mismatch factor of 0.3–0.4 looks like.

> **Think about it**: these settings were computed at zero current. Do they still work at higher current?
> Answer: not as they are. With the same settings and 2 mA, the rms $x$ at the entrances of the first six superconducting periods jumps between 1.0 and 1.8 mm; at 5 mA between 0.9 and 2.7 mm, and the largest rms size in the MEBT grows from 3.7 mm to 6.0 mm. Space charge acts as a defocusing lens that depends on the beam size; it changes the transport of every element, so the matching must be redone at the real current (the linear preview can include linear space charge; the multi-particle engine has the final word). Space charge is strongest at low energy, which makes the MEBT the section most sensitive to the current.

The lab below is a quadrupole FODO channel. `mismatch = 1.3` makes the entrance β deviate from the matched value, so the envelope beats from cell to cell, with a largest rms size of about 3.05 mm; drag it back to 1 and the envelope repeats every cell, with a largest rms size of 1.91 mm. That beat is what a matching section removes.

```avas-lab
fodo mismatch=1.3
```

## Chopping, measuring and collimating {#journey-mebt-chopper}

The MEBT is the lowest-energy part of the machine where the beam is already bunched, and many devices that "handle" the beam sit here.

**Chopper**: fast deflecting electrodes or magnets that steer selected bunches onto an absorber and so leave gaps in the time structure of the beam, for example to reduce the average power during commissioning or to provide the time structure needed by an injection or a target station downstream (the purpose varies between machines). At 162.5 MHz the bunches are 6.15 ns apart while each bunch is only about 73 ps long; to remove single bunches, the deflecting field must rise and fall within a few nanoseconds. It is placed in the low-energy MEBT because the deflecting field needed for a given angle (a magnetic field $\propto B\rho$, an electric field $\propto pv/q$) is smallest here, and so is the power of the removed bunches on the absorber.

**Measurements**: beam position monitors (BPMs) measure the position and also the arrival phase of the bunches; the time of flight between two BPMs gives the energy. At low energy this is very sensitive: at 1.527 MeV a 1 keV change of energy changes the time of flight by about 19 ps per metre, 1.1° at 162.5 MHz. Other devices measure the beam profile, the current and the bunch length. The MEBT measurements are the main basis for tuning the RFQ and MEBT and for confirming the matching.

**Collimation (scraping)**: the RFQ beam has tails and halo, and scrapers remove them here rather than letting them be lost at high energy. The reason is activation: 1.5 MeV protons are far below the Coulomb barrier of common materials such as copper (roughly 7 MeV for copper, from $1.44Z/[1.2(A^{1/3}+1)]\ \mathrm{MeV}$), so they hardly cause nuclear reactions and losing them here leaves almost no radioactivity; the same particles lost above a few tens of MeV would activate the equipment.

## Common misconceptions {#journey-mebt-misconceptions}

**"The MEBT does not accelerate, so it is just a connecting pipe."** The MEBT determines the quality of the beam that enters the superconducting section. A mismatch here turns into emittance growth and halo downstream, and becomes harder to correct the further it travels.

**"A buncher is just a weaker accelerating cavity."** At $-90^\circ$ the synchronous particle gains no energy; the buncher is a longitudinal lens that changes the correlation between energy and position. For the same voltage it is also the most strongly defocusing cavity transversely.

**"Bunches lengthen in a drift because of space charge."** Even at zero current the momentum spread lengthens the bunch through $R_{56} = L/\gamma^2$; space charge only makes it faster.

**"The smaller the beam spot, the better."** The goal of matching is the matched Twiss parameters of the downstream period, not the smallest spot. At the entrance of the superconducting section the matched beam is in fact opening up ($\alpha \approx -0.78$), because a solenoid follows immediately and the matched β reaches its maximum inside it.

**"1.5 mm is already a short bunch."** Whether a bunch fits downstream depends on the fraction of the RF period it occupies: at 162.5 MHz 1.5 mm is 5°, at 650 MHz the same length is 20°. Keep bunch lengths in millimetres and in degrees apart.

**"Matched at zero current means matched."** The matching depends on the current; see the "Think about it" above.

## In AVAS {#journey-mebt-in-avas}

### What AVAS can model

The MEBT is the first section AVAS can model (except the chopper). Quadrupoles are the matrix element `quad` (or magnetostatic field maps), bunchers are RF field-map elements `field` (field type 1), drifts `drift`, correctors `steerer`. A chopper (a time-dependent deflection) is not among AVAS's elements, nor are beam instruments: `diag_position`, `diag_size` and `diag_energy` are "measurement targets" for the error correction, not models of instruments (see [errors and correction](#errors)).

```avas-params
quad field drift steerer
```

A buncher is written like an accelerating cavity, only with a phase of $-90^\circ$:

```text
field  0.21  0.02  0  1  162.5e6  -90  0.195  -0.195  hwr010
```

> **AVAS convention**: with `V3 = 0`, `phase` is the synchronous phase $\varphi_s = \operatorname{atan2}(\int E\sin\varphi\,\dd z, \int E\cos\varphi\,\dd z)$ and the engine solves for the entrance phase; the RF electric field acts as $K_e\boldsymbol{E}\cos(\omega t + \varphi_0)$, the magnetic field as $K_b\boldsymbol{B}\sin(\omega t + \varphi_0)$, with the map in MV/m and T (see [the RF cavity](#rf-cavity)). So $-90^\circ$ is the bunching phase at which the synchronous particle gains nothing and the head is slowed down.

> **AVAS convention**: in `quad L R 0 G`, $G > 0$ focuses a positive charge in $x$ and defocuses in $y$, with $k = G/B\rho$ (see [the quadrupole](#quadrupole)). In multi-particle runs the first and last elements of a lattice should not be matrix elements, which is why the beam line above starts with a drift.

### Taking over the beam from the RFQ

AVAS does not simulate the RFQ; the MEBT's input beam comes from the output of an RFQ code, in one of two ways (see [the RFQ](#journey-rfq)): `readparticledistribution` in `beam.txt` reads a particle file (the example's `part_rfq.dst`; the GUI switches it with `use_dst`; when a file is read, every keyword except `numofcharge` is ignored), or Twiss parameters in all three planes let AVAS generate the distribution.

```avas-params
beam:readparticledistribution beam:use_dst beam:twissx beam:twissz beam:current
```

> **AVAS convention**: for `twissx` and `twissy`, β is in mm/mrad and the emittance is the normalized rms emittance (π·mm·mrad), so rms size $=\sqrt{\beta\varepsilon_n/(\beta\gamma)}$; `twissz` uses the same units with the longitudinal angle $z' = \Delta p/p$.

> **Note**: in the example `beam.txt` the β of `twissz` is 0.37292, exactly the β of `twissy`, while the statistics of `part_rfq.dst` give $\beta_z = 0.3326$ (the $\alpha_z$ and the emittance agree). The two give rms bunch lengths of 1.32 mm and 1.24 mm. This chapter's longitudinal numbers are taken from the particle file; the page's beam-line views and labs use `beam.txt`. The difference looks like a transcription slip; check it before relying on the example.

### Matching in AVAS

- Start with the linear preview of the visual editor: drag the quadrupole gradients and the buncher `Ke` and watch whether the envelope repeats period by period downstream (linear space charge can be switched on). The preview is an approximate model; rms sizes usually agree within 1–2 %.
- For fine tuning use a parameter scan (`avas scan`) or the AI assistant's optimizer, which run copies of the project repeatedly; the `adjust` / `diag_size` mechanism of the error study can also tune parameters to beam sizes, with the conditions and limits described in [matching](#matching).
- Confirm with the multi-particle engine at the real current: the Results page's rms envelope, Twiss parameters and emittance along the line; the envelope in the downstream periods should quickly become periodic.

### Collimation and losses

AVAS has no dedicated scraper element. The engine judges particle loss by each element's aperture `R`, so a drift with a smaller aperture acts as a limiting aperture in the simulation; with `boundary 1` in `input.txt`, the boundaries of `boundary.txt` are used instead and the lattice's `R` no longer applies.

```avas-params
input:boundary
```

## Further reading {#journey-mebt-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: longitudinal motion, bunching, RF defocusing and beam matching.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: beam transport, the envelope equations and matching with space charge.
- CERN Accelerator School (CAS) proceedings on linac beam dynamics and beam instrumentation.
- AVAS manual `docs/使用说明20260427.docx`: the `quad`, `field` and `beam.txt` keywords.
- This primer: [matching](#matching), [the RF cavity](#rf-cavity), [the quadrupole](#quadrupole), [longitudinal motion](#longitudinal), [errors and correction](#errors).
