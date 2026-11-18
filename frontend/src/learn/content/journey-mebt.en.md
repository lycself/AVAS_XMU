# Shaping: the medium-energy beam transport

The RFQ has just brought our proton to 1.527 MeV and cut the continuous beam into neat little packets (**bunches**). The next stop ought to be the superconducting cavities, yet between the two sits a beam line two or three metres long, sometimes longer, without a single accelerating cavity. Why not attach the first superconducting cavity straight to the end of the RFQ?

An analogy. A group of runners has just burst over the start line and is handed over to the next stage of the race. As they run they drift apart sideways, and the faster ones pull ahead while the slower ones fall behind, so the group gets longer and longer. The next stage, however, has a track of fixed width and wants the group to arrive in a set formation and with a set length. Between the two stages there must be a "hand-over zone" where someone gathers the spreading group back in, squeezes the stretched group short again and arranges the formation the next stage wants.

In an accelerator this hand-over zone is the **medium-energy beam transport** (MEBT). It does not accelerate. It does three things: transversely it catches the diverging beam, longitudinally it squeezes the bunches that have lengthened on their own, and in all three directions it shapes the beam the way the superconducting section wants it (this is called **matching**); on the side it also chops, measures and scrapes away the outermost particles. This chapter continues from the [RFQ](#journey-rfq), uses the ideas of [matching](#matching) and [longitudinal motion](#longitudinal), and the data of all stages are in the [overview](#journey-overview-thread).

## The beam as it leaves the RFQ {#journey-mebt-rfq-exit}

First look at our proton's state at this moment. Kinetic energy $W = 1.527\ \mathrm{MeV}$, speed 5.7 % of the speed of light ($\beta = v/c = 0.05698$). The bunches follow one another at the 162.5 MHz beat, 6.15 ns apart; in that time the proton travels $\beta\lambda = 105.1\ \mathrm{mm}$, which is the distance between neighbouring bunches ($\lambda = c/f = 1.845\ \mathrm{m}$ is the RF wavelength). How hard it is for a magnet to bend the proton is expressed by the **magnetic rigidity** $B\rho = 0.1786\ \mathrm{T\,m}$ (see [quadrupole](#quadrupole-lorentz)).

The thickness of the beam is described by its **rms size**, which you can think of as the "average" distance of the particles from the centre (precise definition in [drift](#drift-size)). The example beam comes from the RFQ exit particle file `part_rfq.dst`: the transverse rms size is about 0.95 mm, the rms angle of the particles about 3 mrad (they spread by about 3 mm per metre), and the beam as a whole is **opening up**. Longitudinally, the rms bunch length is 1.24 mm; the momenta of the particles also differ a little, with an rms relative deviation of 0.375 %, corresponding to an energy difference of about 11.4 keV.

This beam is unsuitable in three ways.

First, **it diverges transversely**. Inside the RFQ the electric field keeps pushing the beam towards the centre, but at the exit this push stops abruptly and the beam spreads out with its 3 mrad angles. Left alone, the rms size grows from 0.95 mm to about 3.7 mm after 1 m of drift.

Second, **its shape is wrong**. The superconducting section is built from many identical "solenoid + cavity" units (**periods**), and such a periodic structure "prefers" a beam of one particular shape: a beam entering with that shape leaves every period with the same shape, and its size repeats period after period. This beam is called the period's **matched beam** (see [matching](#matching)). The beam from the RFQ is different. The number measuring "how different" is the **mismatch factor**: 0 means a perfect fit; for the example it is about 0.3–0.4 transversely and about 0.2 longitudinally. A mismatch factor of 0.35 means that the beam size will "beat" back and forth between $1/1.35 \approx 0.74$ and $1.35$ times the matched size, and with space charge it will also fling some particles outwards to form a **halo**. How these numbers are computed is shown in [Going deeper: the mismatch factor and the number of knobs](#journey-mebt-deep-matching).

Third, **the bunches lengthen on their own**; the next section is about that.

On top of this there are engineering needs: the superconducting cavities sit in cryostats (cryomodules), whose entrance needs vacuum isolation and a transition; and there must be room for a chopper, beam diagnostics and scrapers. All of this calls for some length between the RFQ and the superconducting section, and for the beam a length is a drift, which makes all three problems above worse. So the MEBT is not a "connecting pipe you could do without" but a beam line that has to be designed with care.

## Why bunches lengthen on their own {#journey-mebt-debunching}

Back to the runners: a group crosses the start line at the same moment with slightly different speeds. Nobody pushes them, yet the group gets longer and longer, the fast ones ahead, the slow ones behind.

The protons in a bunch do the same. Each proton's momentum is slightly different; the relative deviation is written $\delta = \Delta p/p$. At low energy the proton is much slower than light and its momentum is $p = mv$, so 1 % more momentum means 1 % more speed. Let $z$ be the coordinate along the bunch (relative to the "synchronous particle" at the bunch centre, positive forwards). A proton faster by $\delta$ travels $\delta \cdot L$ further while the synchronous particle covers $L$:

$$
\Delta z = L\,\delta .
$$

With the example numbers: the rms momentum deviation is 0.375 %, so the particle positions spread by about 3.75 mm per metre. The bunch, originally only 1.24 mm long, adds this by Pythagoras (the original length and the new spread are unrelated, like two perpendicular sides): $\sqrt{1.24^2 + 3.75^2} \approx 3.9\ \mathrm{mm}$. The exact calculation gives 3.89 mm after 1 m of drift, and about 10 mm after 2.7 m (the length of the MEBT below).

Note that no force acts here. The bunch is not "pushed apart" by anything; the fast and slow particles simply separate: after a drift the fast particles gather at the front and the slow ones at the back, and position and energy become more and more strongly tied together. (Near the speed of light the same momentum difference produces a smaller speed difference and the formula gains a factor $1/\gamma^2$; see [Going deeper: bunch lengthening and bunching](#journey-mebt-deep-bunching).)

**Why not let it lengthen?** An RF cavity does not "see" millimetres but phase. The 105.1 mm between neighbouring bunches correspond to one whole RF period, $360^\circ$, so 1 mm corresponds to $3.43^\circ$: the 1.24 mm bunch at the RFQ exit occupies only $4.3^\circ$, after 1 m of drift $13.3^\circ$, after 2.7 m about $35^\circ$. With a synchronous phase of $-30^\circ$ in the superconducting section, the phase range that can hold particles stably (the **bucket**) extends from about $-61^\circ$ to $+30^\circ$, about $91^\circ$ wide (see the [overview](#journey-overview-bunching) and [Going deeper: the bucket](#journey-sc-deep-bucket)). A bunch with an rms phase of $35^\circ$ spans $140^\circ$ at plus and minus two rms, so many particles inevitably fall outside the bucket and are lost; even inside it, particles near the edge feel the curved part of the RF field and the beam quality suffers. So the bunch has to be squeezed short again before it enters the superconducting section, and to the length the downstream section needs.

**Space charge makes things worse.** The protons are all positively charged and repel each other. Within a bunch, the protons at the head are pushed forwards by the charge behind them and those at the tail backwards, so the bunch is also pushed apart longitudinally. The shorter the bunch, the more densely packed the charge: for a Gaussian bunch, the peak current at the RFQ exit is about 34 times the average current (see the [overview](#journey-overview-space-charge)), so the example's average 0.18 mA corresponds to about 6 mA at the bunch centre.

> **Think about it**: since the bunch lengthens during a drift, can it "shorten back by itself" after the drift?
>
> Answer: a drift cannot do that. A drift only keeps the fast ones moving ahead and the slow ones falling back. To shorten the bunch again, the protons at the head must be slowed down and those at the tail sped up, turning "fast front, slow back" into "slow front, fast back". That needs a longitudinal force that changes in time: a buncher.

## The buncher: a longitudinal lens that does not accelerate {#journey-mebt-buncher}

**The intuitive picture.** Imagine a referee standing in front of the runners whose push changes from moment to moment: he pushes back those who arrive early, pushes forward those who arrive late, and his push is exactly zero at the moment the centre of the group arrives. After passing him, the front runners are slower and the back runners faster; a little further on, the back runners catch up with the front ones and the group is shorter.

A **buncher** is such a referee. It is an RF cavity whose electric field oscillates like a cosine. An ordinary accelerating cavity lets the bunch through when the field is near its maximum; a buncher instead times it so that the synchronous particle at the bunch centre crosses **exactly when the field passes through zero**: it gains no energy and loses none.

In AVAS's convention the synchronous particle feels a field proportional to $\cos\varphi$, and its energy gain is $\Delta W = qV_0T\cos\varphi_s$, where $\varphi_s$ is the synchronous phase and $qV_0T$ the largest energy gain, on the crest (see [RF cavity](#rf-cavity)). A buncher uses $\varphi_s = -90^\circ$: $\cos(-90^\circ) = 0$, so the synchronous particle's energy does not change. A proton arriving earlier (at the front of the bunch, $z > 0$) has a smaller phase, the cosine is negative, and it is decelerated; one arriving later has a larger phase, the cosine is positive, and it is accelerated. Exactly "slow front, fast back".

**How strong is the push?** Near the zero crossing the field changes almost linearly in time (just as $\sin x \approx x$ for small $x$). So a proton arriving $\Delta\varphi$ (in radians) earlier than the synchronous particle gets an energy change of about the crest gain times $\Delta\varphi$, decelerating:

$$
\Delta W \approx -qV_0T \cdot \Delta\varphi .
$$

With the example numbers. The first buncher in the beam line below borrows the example HWR field map with $K_e = 0.195$; its largest energy gain (on crest) is about 57.6 keV. A proton 1 mm ahead of the bunch centre arrives $3.43^\circ = 0.0598\ \mathrm{rad}$ early, so it is slowed by $57.6 \times 0.0598 \approx 3.4\ \mathrm{keV}$. In terms of momentum: at low energy the kinetic energy is $W = p^2/(2m)$, so the relative change of energy is twice the relative change of momentum, and $\delta$ changes by $-3.4/(2 \times 1527) \approx -0.11\ \%$. In other words, for every millimetre away from the centre the momentum is changed by 0.11 %, towards the centre:

$$
\Delta\delta = -K\,z ,\qquad K \approx 1.13\ \mathrm{m^{-1}} .
$$

**This is a lens.** An optical lens bends a ray at distance $x$ from the axis by an angle proportional to $x$, towards the axis; a buncher gives a proton at distance $z$ from the centre a momentum deviation proportional to $z$, towards the centre. The following drift turns momentum deviations into position changes ($\Delta z = L\,\delta$): the proton 1 mm ahead falls back $1.13\ \mathrm{mm}$ per metre, so after $1/K \approx 0.89\ \mathrm{m}$ it is exactly back at the centre. For a bunch without an initial "fast front, slow back" relation, 0.89 m is where it is squeezed shortest; this is the **longitudinal focal length**.

A few tens of kilovolts suffice, which is the advantage of low energy: when the speed is low, a small energy difference produces a large speed difference. The full formula $K = 2\pi qV_0T/(mc^2\lambda\beta^3\gamma)$ contains $1/\beta^3$; the derivation is in [Going deeper: bunch lengthening and bunching](#journey-mebt-deep-bunching).

**The price: transverse defocusing.** While a buncher pushes particles towards the centre longitudinally, it pushes them outwards transversely. This is not a design flaw but a property of the electric field itself: where there is no charge, an electric field cannot push particles back towards the centre in all three directions at once (the same reason as "no focusing in both directions at once" in the [quadrupole](#quadrupole-field) chapter). However much it pushes back longitudinally, it must push out by the same amount in the two transverse directions together, half in each. So the buncher's transverse focal length is about twice the longitudinal one, with opposite sign: $1/f_x \approx -0.56\ \mathrm{m^{-1}}$, a defocusing focal length of about 1.8 m. At $\varphi_s = -90^\circ$ this defocusing is the strongest for a given voltage (see [RF cavity](#rf-cavity)), and the MEBT quadrupoles have to take it into account. Why exactly half: see [Going deeper: why a buncher defocuses transversely](#journey-mebt-deep-defocus).

**Check it yourself.** The lab below sets the phase to $-90^\circ$ and $K_e = 0.2$. Its beam line is only 0.46 m long (the entrance beam is the example `beam.txt`, rms $z$ 1.32 mm). Watch three things: the energy gain reads 0; the dashed rms $z$ curve in the envelope plot no longer rises after the cavity as it would in a drift; the solid rms $x$ grows faster after the cavity. Moving $K_e$ through 0, 0.2 and 0.4 gives exit rms $z$ of 2.06, 1.68 and 1.37 mm (the stronger, the shorter) and exit rms $x$ of 2.09, 2.35 and 2.61 mm (the stronger, the more it spreads transversely).

```avas-lab
rf-cavity phase=-90 Ke=0.2
```

## Matching: why so many knobs {#journey-mebt-matching}

**How many numbers to set.** To shape the beam the way the downstream section wants, each direction needs at least two things set: how **thick** the beam is at this moment, and whether it is **getting thicker or thinner, and how fast**. Setting only the thickness but not the trend, the beam keeps opening or closing once it enters the periods and is mismatched all the same. It is like throwing a ball into a basket: you have to get both the release point and the direction right. The two transverse directions give four requirements, the longitudinal direction two. You learned at school that satisfying $n$ equations at once generally needs $n$ adjustable unknowns. So the MEBT needs at least four independent transverse "knobs" and two longitudinal ones. The quality of the beam itself (its emittance, see [drift](#drift-size)) belongs to the beam and cannot be changed by matching.

**Transversely: quadrupoles.** First look at what one quadrupole does to an opening beam like the one from the RFQ. The animation below shows the example beam passing a quadrupole 0.1 m long with a gradient of 10 T/m:

```avas-motion
quad
```

In the upper plot (horizontal direction $x$) the beam is caught, with an exit rms of only 1.44 mm; in the lower plot (vertical direction $y$) the same particles are pushed further apart, to an exit rms of 6.32 mm. Drag the gradient $G$ to 0 and both directions are about 2.65 mm (a pure drift); drag it to $-10$ and the two directions swap. A single quadrupole always focuses in one direction and defocuses in the other (see [quadrupole](#quadrupole)), so several quadrupoles of alternating sign are needed to catch both directions and give each the required shape.

Why not use solenoids as in the LEBT? At 1.5 MeV a solenoid is still strong enough (its strength scales as $(B/B\rho)^2$), but it acts identically on $x$ and $y$ and can only make the two directions the same; the two directions at the RFQ exit are generally not symmetric, and quadrupoles can adjust them separately. A quadrupole's strength is $k = G/B\rho$: a gradient of 7 T/m gives $k = 39\ \mathrm{m^{-2}}$, so a 0.1 m quadrupole has a focal length of about 0.26 m and a compact triplet (a group of three) catches the diverging beam within a metre or two.

**Longitudinally: two bunchers**, acting as two longitudinal lenses; adjusting their amplitudes controls both the bunch length at the entrance of the superconducting section and the strength of its "slow front, fast back" relation.

**A schematic MEBT.** The beam line below follows this recipe: a quadrupole triplet catches the diverging beam, followed by a buncher, a quadrupole doublet, a second buncher and a final doublet, 2.69 m in all; after it come the first two periods of the superconducting section. Both bunchers borrow the example HWR field map ($K_e = 0.195$ and $0.182$, phase $-90^\circ$) only to demonstrate with an existing map; the bunchers of a real MEBT are usually room-temperature cavities with a different field shape. The gradients of the seven quadrupoles and the amplitudes of the two bunchers were found with the linear preview's transfer matrices at zero current, so that the mismatch factor at the entrance of the superconducting section is about 2 % or less transversely and below 1 % longitudinally (to this precision the matched parameters themselves carry some uncertainty, see [Going deeper: the mismatch factor and the number of knobs](#journey-mebt-deep-matching)).

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

Look at three things in the 2D plot:

- **Energy curve**: in the MEBT the energy stays at 1.527 MeV (the bunchers do not accelerate); in the superconducting section the two cavities raise it to 1.733 MeV and 1.985 MeV.
- **Transverse envelope**: in the MEBT the beam alternately widens and narrows in $x$ and $y$ between the quadrupoles, with a largest rms size of about 3.7 mm (aperture 20 mm, so 3 rms is still inside); at the end of the MEBT the rms $x$ and $y$ are back to about 1.2 mm.
- **Longitudinal**: the rms bunch length at the end of the MEBT is 1.48 mm ($5.1^\circ$). With both bunchers switched off ($K_e = 0$) it is about 10 mm here, the $35^\circ$ of the previous section.

The effect of matching is clearest in the superconducting section: the rms $x$ at the entrance of each period is 1.24, 1.30 and 1.15 mm, and the remaining variation comes from acceleration itself (the energy changes, so an exactly repeating solution does not exist). For comparison, without an MEBT, letting the beam from the RFQ enter the same periods directly (as the beam line of the [superconducting section](#journey-sc) chapter does), the rms $x$ at the period entrances is 1.05, 1.64, 0.99 and 1.41 mm, a beating of about ±25 %, exactly what a mismatch factor of 0.3–0.4 produces.

**What mismatch looks like.** The lab below is a quadrupole FODO channel (see [quadrupole](#quadrupole-fodo)). `mismatch = 1.3` means that the β of the entrance beam differs from the matched value (β is the parameter describing the beam size, see [Going deeper: the mismatch factor and the number of knobs](#journey-mebt-deep-matching)); the envelope beats up and down, with a largest rms of about 3.05 mm. Drag it back to 1 and the envelope repeats period after period, with the largest rms down to 1.91 mm. This is what a matching section has to remove.

```avas-lab
fodo mismatch=1.3
```

## Chopping, measuring and collimating {#journey-mebt-chopper}

The MEBT is the lowest-energy part of the machine where the beam is already bunched, so many devices that "handle" the beam are placed here.

**Chopper**: deflecting electrodes or magnets that can be switched quickly and kick selected bunches onto an absorber, leaving gaps in the beam's time structure. Uses include lowering the average power during commissioning, or providing the time structure needed for injection or a target downstream (this depends on the facility). At 162.5 MHz the bunches are 6.15 ns apart while a bunch is only about 73 ps long, so to remove individual bunches the deflecting field must rise and fall within a few nanoseconds. The chopper sits in the low-energy MEBT because the deflecting field needed for a given angle is smallest there (a magnetic field scales with $B\rho$, an electric field with $pv/q$), and the power of the removed bunches hitting the absorber is lowest too.

**Beam diagnostics**: **beam position monitors** (BPMs, see [orbit correction](#orbit-correction-bpm)) measure the position of the beam centre and also the arrival time (phase) of the bunches; the time of flight between two BPMs gives the energy. At low energy this measurement is very sensitive: at 1.527 MeV a 1 keV change in energy changes the time of flight by about 19 ps per metre, $1.1^\circ$ at 162.5 MHz. (How: the time of flight per metre is $1/v = 58.5\ \mathrm{ns}$; at low energy the relative change in speed is half the relative change in energy, $1/(2 \times 1527) = 3.3\times10^{-4}$, and the product is about 19 ps.) There are also devices for beam profile, current and bunch length. The measurements in the MEBT are the main basis for tuning the RFQ and MEBT and for confirming the match.

**Collimation (scraping)**: the beam from the RFQ has tails and a sparse outer halo, and scrapers here stop them, which is better than losing them at high energy. The reason is **activation**: high-energy protons hitting metal cause nuclear reactions and make the equipment radioactive. A 1.5 MeV proton, however, is far below the Coulomb barrier of copper and other common materials (to approach a nucleus the proton has to overcome the repulsion of its positive charge; for copper roughly 7 MeV, from $1.44Z/[1.2(A^{1/3}+1)]\ \mathrm{MeV}$ with $Z$ the atomic number and $A$ the mass number), so it causes almost no nuclear reactions and losing particles here leaves almost no radioactivity; the same particles lost above a few tens of MeV would activate the equipment.

## Think about it {#journey-mebt-think}

> **Think about it 1**: what happens if the buncher phase is changed from $-90^\circ$ to $+90^\circ$?
>
> Answer: the synchronous particle still gains no energy, but now the head is accelerated and the tail decelerated, the bunch stretches further downstream, and the buncher becomes a longitudinally defocusing lens, while transversely it now focuses. A buncher's phase must be on the "slow head, fast tail" side, which in AVAS's convention is $-90^\circ$. Drag `phase` to $+90^\circ$ in the buncher lab above to see it.

> **Think about it 2**: the MEBT settings above were found at zero current. Do they still work at higher current?
>
> Answer: not as they are. With the same settings at 2 mA, the rms $x$ at the entrances of the first six superconducting periods jumps between 1.0 and 1.8 mm; at 5 mA between 0.95 and 2.7 mm, and the largest rms size in the MEBT grows from 3.7 mm to 6.0 mm. Space charge acts like a defocusing lens that depends on the beam size; it changes the transport of every section, and the matching has to be redone at the actual current (the linear preview can include linear space charge; the multi-particle engine has the final word). Space charge is strongest at low energy, so the MEBT is the section most sensitive to current.

> **Think about it 3**: the buncher's longitudinal focal length is 0.89 m. If its voltage is doubled, what does the focal length become? If the proton energy is raised fourfold (speed about doubled), what does it become at the same voltage?
>
> Answer: doubling the voltage doubles $K$ and halves the focal length, to about 0.45 m. Doubling the speed makes $K \propto 1/\beta^3$ eight times smaller, so the focal length becomes about 7 m: the same buncher is much weaker at higher energy. This is one reason why bunching is best done at low energy.

## Common misconceptions {#journey-mebt-misconceptions}

**"The MEBT does not accelerate, so it is just a connecting pipe."** The MEBT decides the quality of the beam entering the superconducting section. Mismatch here turns into emittance growth and halo downstream, and becomes harder to correct the further on it gets.

**"A buncher is just a weaker accelerating cavity."** A buncher at $-90^\circ$ gives the synchronous particle zero energy gain; it is a longitudinal lens that changes the relation between energy and position. For a given voltage it is also the cavity with the strongest transverse defocusing.

**"Bunches lengthen in a drift because of space charge."** Even at zero current the momentum spread lengthens the bunch through $R_{56} = L/\gamma^2$; space charge only makes it faster.

**"The smaller the beam spot, the better."** The goal of matching is the matched beam of the downstream periods, not the smallest spot. At the entrance of the superconducting section the matched beam is actually opening up ($\alpha \approx -0.78$), because a solenoid follows immediately and the matched beam reaches its largest size inside the solenoid.

**"A 1.5 mm bunch is already very short."** Whether the downstream section can capture a bunch depends on the fraction of the RF period it occupies: at 162.5 MHz 1.5 mm is $5^\circ$, at 650 MHz the same length is $20^\circ$. Keep bunch lengths in millimetres and in degrees apart.

**"Matched at zero current means matched."** The matched settings depend on the current; see "Think about it 2" above.

## Going deeper: bunch lengthening and bunching — R₅₆ and the longitudinal focal length {#journey-mebt-deep-bunching}

The main text explained bunch lengthening with "1 % more momentum, 1 % more speed" and the buncher with "the field is linear near the zero crossing". This section makes both precise and finally combines them into a "conversion table" identical to that of an optical lens.

### Step 1: how much speed difference for a momentum difference

At low energy $p = mv$, so $\Delta v/v = \Delta p/p = \delta$. But nothing can go faster than light: in relativity $p = \gamma m v$, where $\gamma = 1/\sqrt{1 - v^2/c^2}$ is the **Lorentz factor**, which grows as the speed approaches $c$. When the momentum increases, part of the increase goes into speed and part into $\gamma$. Working out the split (the detailed derivation is in [drift](#drift-deep-longitudinal)) gives

$$
\frac{\Delta v}{v} = \frac{1}{\gamma^2}\,\frac{\Delta p}{p} = \frac{\delta}{\gamma^2} .
$$

For the example beam $\gamma = 1.0016$ and $1/\gamma^2 = 0.997$, almost the same as the low-energy formula; at 1 GeV $\gamma = 2.066$ and $1/\gamma^2 = 0.23$, so the same momentum difference produces only about a quarter of the speed difference.

### Step 2: speed difference becomes position difference: R₅₆

A proton faster by $\Delta v$ travels $\Delta v \cdot L/v$ further in the time $L/v$ the synchronous particle needs for $L$:

$$
\Delta z = \frac{L}{\gamma^2}\,\delta ,\qquad R_{56} = \frac{L}{\gamma^2} .
$$

The name $R_{56}$ comes from the drift's 6 × 6 transfer matrix: it is the element that converts the sixth coordinate $\delta$ into the fifth coordinate $z$ (see [drift](#drift-deep-matrix)). At low energy $\gamma \approx 1$ and momentum spread turns into position spread almost one to one: the example's rms momentum spread of 0.375 % adds about 3.7 mm to the rms length per metre of drift.

Check the numbers of the main text. At the RFQ exit the bunch is almost at a "waist" (the longitudinal Twiss parameter $\alpha_z$ is small, meaning there is almost no relation between position and momentum), so the original length and the new spread are unrelated and add by Pythagoras: after 1 m, $\sqrt{1.24^2 + (1000 \times 0.997 \times 0.00375)^2} = \sqrt{1.54 + 13.97} = 3.94\ \mathrm{mm}$; the exact Twiss calculation (including that small $\alpha_z$) gives 3.89 mm. After 2.7 m it is about 10 mm.

**The longitudinal phase-space area does not change.** Plot all particles with $z$ on the horizontal axis and $\delta$ on the vertical axis and you get an ellipse (longitudinal phase space). A drift moves each point horizontally by $R_{56}\,\delta$: points with larger $\delta$ move further right, so the ellipse is "sheared" into a long tilted ellipse, but its area stays the same (sliding horizontal strips sideways cannot change the area). This is what **Liouville's theorem** looks like here: without forces the phase-space area is conserved. The longer bunch is paid for by an ever stronger correlation between position and energy.

### Step 3: millimetres into degrees

An RF cavity sees phase. The synchronous particle arrives at phase $\varphi_s$; a particle ahead of it arrives earlier by $z/v$, which corresponds to a phase smaller by $2\pi f \cdot z/v = 2\pi z/(\beta\lambda)$ (in radians):

$$
\Delta\varphi = -\frac{2\pi z}{\beta\lambda} .
$$

For the example $\beta\lambda = 105.1\ \mathrm{mm}$, so 1 mm corresponds to $360^\circ/105.1 = 3.43^\circ$. The $4.3^\circ$, $13.3^\circ$ and $35^\circ$ of the main text are 1.24, 3.89 and about 10 mm times 3.43.

### Step 4: the buncher near the zero crossing

A proton $\Delta\varphi$ away from the synchronous particle gains $qV_0T\cos(\varphi_s + \Delta\varphi)$, which exceeds the synchronous particle's gain by

$$
\Delta W = qV_0T\left[\cos(\varphi_s + \Delta\varphi) - \cos\varphi_s\right] \approx -qV_0T\sin\varphi_s\,\Delta\varphi .
$$

The approximation uses the school angle-sum formula $\cos(a + b) = \cos a\cos b - \sin a\sin b$ and the small-angle approximations $\cos b \approx 1$, $\sin b \approx b$. At $\varphi_s = -90^\circ$, $\sin\varphi_s = -1$; inserting $\Delta\varphi$ from step 3:

$$
\Delta W = -\frac{2\pi\,qV_0T}{\beta\lambda}\,z .
$$

The energy change is proportional to $z$, with the sign that makes the bunch "slow at the head, fast at the tail".

### Step 5: energy difference into momentum difference

The relativistic relation between kinetic energy and momentum gives $\Delta p/p = \Delta W/(\beta^2\gamma\,mc^2)$ (at low energy $\beta^2\gamma\,mc^2 \approx mv^2 = 2W$, which is the "relative energy change is twice the relative momentum change" of the main text). For the example $\beta^2\gamma\,mc^2 = 3.052\ \mathrm{MeV}$, almost the same as $2W = 3.054\ \mathrm{MeV}$. Hence

$$
\Delta\delta = -K z ,\qquad K = \frac{2\pi\, qV_0T}{mc^2\,\lambda\,\beta^3\gamma} .
$$

One $\beta$ from $\beta\lambda$ joins $\beta^2\gamma$ to give $\beta^3\gamma$ in the denominator. With the example numbers: $mc^2\lambda\beta^3\gamma/(2\pi) = 938.27 \times 1.845 \times 1.853\times10^{-4}/(2\pi) = 0.0511\ \mathrm{MeV\,m}$, so every 51 kV of effective voltage $V_0T$ provides $1\ \mathrm{m^{-1}}$ of longitudinal focusing strength. With $qV_0T = 57.6\ \mathrm{keV}$, $K = 0.0576/0.0511 = 1.13\ \mathrm{m^{-1}}$, in agreement with the step-by-step estimate of the main text. The $1/\beta^3$ shows that bunching is much easier when the speed is low.

### Step 6: lens plus drift, find the focus

Write a proton's state as two numbers $(z, \delta)$. The buncher (a thin lens) and the drift are each a 2 × 2 conversion table (for the rules of matrix multiplication see [drift](#drift-deep-matrix)):

$$
M_{\text{buncher}} = \begin{pmatrix} 1 & 0 \\ -K & 1 \end{pmatrix},\qquad
M_{\text{drift}} = \begin{pmatrix} 1 & L/\gamma^2 \\ 0 & 1 \end{pmatrix} .
$$

Buncher first, then a drift $L$: the total table is the product (the later element on the left):

$$
M_{\text{drift}}\,M_{\text{buncher}} = \begin{pmatrix} 1 - KL/\gamma^2 & L/\gamma^2 \\ -K & 1 \end{pmatrix} .
$$

For a bunch without initial correlation ($\delta$ unrelated to $z$), the exit $z$ is the top-left element $1 - KL/\gamma^2$ times the entrance $z$, plus a contribution from $\delta$. Where the top-left element is zero, the entrance position differences are wiped out completely and the bunch is shortest:

$$
L_f = \frac{\gamma^2}{K} .
$$

For the example $L_f = 1.0033/1.13 = 0.89\ \mathrm{m}$. These two tables have exactly the form of an optical thin lens and free propagation: the buncher is a longitudinal lens and the drift is longitudinal "free propagation".

### For readers who know calculus

Taking the logarithmic derivative of $p = \gamma m v = m v/\sqrt{1 - v^2/c^2}$ gives $\dd p/p = \gamma^2\,\dd v/v$; from $W = (\gamma - 1)mc^2$ and $p^2c^2 = W^2 + 2Wmc^2$ follows $\dd p/p = \dd W/(\beta^2\gamma\,mc^2)$. In a drift the rms bunch length satisfies $\sigma_z^2(L) = \varepsilon_z(\beta_z - 2\alpha_z s + \gamma_z s^2)$ with $s = L/\gamma^2$; the buncher is the phase-space shear $\delta \to \delta - Kz$, and like the drift's shear $z \to z + s\,\delta$ it is a linear map with determinant 1, so the longitudinal emittance is conserved. A more accurate treatment keeps the higher-order terms of the cosine: $\Delta W \propto \sin\Delta\varphi$ departs visibly from a straight line once $\lvert\Delta\varphi\rvert$ reaches a few tens of degrees, and this is where the longitudinal emittance growth of a long bunch passing a buncher comes from.

## Going deeper: why a buncher defocuses transversely {#journey-mebt-deep-defocus}

The main text said: pushing particles back towards the centre longitudinally necessarily pushes them out transversely, half in each direction. This section shows step by step where that comes from.

### Step 1: an observer travelling with the bunch

Imagine an observer riding on the synchronous particle and moving with the bunch. To him the buncher sweeps past from the front; we only care about the field in a small region around the bunch, and during the short time the protons spend crossing the cavity the shape of the field in this region changes slowly, so it can be treated approximately as a static field ("quasi-static"). Near the bunch centre there are no other charges (the beam's own charge is space charge and is treated separately), so this field has to obey the rules of an electrostatic field in vacuum.

### Step 2: the rule of electrostatics: what goes in comes out

You learned at school that electric field lines start on positive charges and end on negative charges. Where there is no charge, field lines do not appear or vanish: take a small box, and as many field lines go in as come out. It is the same rule as "what goes in comes out" for magnetic field lines in the [quadrupole](#quadrupole-deep-field) chapter.

### Step 3: write the force as three springs

Near the bunch centre the electric force on a particle can be written as "springs": longitudinally $F_z = -K_z z$, transversely $F_x = -K_x x$ and $F_y = -K_y y$. $K > 0$ means pushing back to the centre (focusing), $K < 0$ pushing out (defocusing). The field is $E = F/q$, so $E_z = -K_z z/q$: for each unit length along $z$, $E_z$ decreases by $K_z/q$; likewise for $E_x$ and $E_y$.

### Step 4: count field lines on a small box

Put a small cube of side $w$ at the centre. The difference between the field lines leaving through the front and back faces is proportional to the difference of $E_z$ on the two faces, $-K_z w/q$; for the left and right faces it is $-K_x w/q$, for top and bottom $-K_y w/q$. What goes in must come out, so the three add to zero:

$$
K_x + K_y + K_z = 0 .
$$

The buncher focuses longitudinally ($K_z > 0$), so the two transverse spring constants must add to $-K_z$. The cavity is symmetric about its axis, $x$ and $y$ are alike, so in each direction $K_x = K_y = -K_z/2$: transverse defocusing, half as strong as the longitudinal focusing.

### Step 5: the example numbers

The longitudinal strength $K = 1.13\ \mathrm{m^{-1}}$ of the main text acts on $\delta$; as a focal length for position it has to be divided by $\gamma^2$: $1/f_z = K/\gamma^2 = 1.126\ \mathrm{m^{-1}}$. Transversely it is half of that with opposite sign:

$$
\frac{1}{f_x} = -\frac{K}{2\gamma^2} \approx -0.56\ \mathrm{m^{-1}},\qquad f_x \approx -1.8\ \mathrm{m} .
$$

When the synchronous phase is not $-90^\circ$, the longitudinal focusing is proportional to $\sin(-\varphi_s)$, and so is the transverse defocusing; at $\varphi_s = -90^\circ$, $\sin(-\varphi_s) = 1$ is the largest. So for a given voltage the buncher is the cavity with the strongest transverse defocusing. In the lab of the main text, raising $K_e$ from 0 to 0.4 increases the exit rms $x$ from 2.09 mm to 2.61 mm: that is this defocusing.

> **Think about it**: since the three spring constants add to zero, could the phase be put on the side where the transverse directions focus?
>
> Answer: yes, for $\varphi_s > 0$ the transverse directions focus, but then the longitudinal direction defocuses and the bunch flies apart. No phase focuses all three directions at once (see the [superconducting section](#journey-sc-phase)), so transverse focusing has to be left to quadrupoles or solenoids.

### For readers who know vector calculus

Step 4 is $\nabla\cdot\boldsymbol{E} = 0$ in the bunch rest frame (Earnshaw's theorem). In the laboratory the field changes in time, and the rigorous derivation uses the Lorentz transformation (the longitudinal length in the bunch rest frame is $\gamma$ times that in the laboratory, the transverse ones are unchanged), so the focal-length relation in the laboratory acquires a $\gamma^2$; the result is transversely $k_t^2 = -\tfrac12 k_l^2$ with $k_l^2 = 2\pi qE_0T\sin(-\varphi_s)/(mc^2\beta^3\gamma^3\lambda)$ (Wangler, chapter 7). For a thin gap $\Delta x' = -\pi qE_0TL\sin\varphi_s\,x/(mc^2\beta^3\gamma^3\lambda)$, which for $\varphi_s = -90^\circ$ and $E_0TL = V_0T$ is exactly this section's $1/f_x = -K/(2\gamma^2)$.

## Going deeper: the mismatch factor and the number of knobs {#journey-mebt-deep-matching}

The main text said the beam from the RFQ has a mismatch factor of about 0.3–0.4 and that matching needs at least six knobs. This section computes these numbers. The tool needed is the Twiss parameters introduced in the [drift](#drift-deep-matrix) chapter; here is a short recap.

### Step 1: three numbers describe one direction of a beam

In one direction (say $x$), plot all particles with position $x$ on the horizontal axis and angle $x'$ on the vertical axis; they lie roughly inside an ellipse. The area of the ellipse (divided by $\pi$) is the **emittance** $\varepsilon$, a quality the beam carries with it; the shape of the ellipse is described by two numbers:

- $\beta$: how "tall" the ellipse is (how thick the beam is), rms size $\sigma = \sqrt{\beta\varepsilon}$;
- $\alpha$: which way the ellipse leans. $\alpha < 0$ means that particles further out have angles pointing further out, so the beam is **opening up**; $\alpha > 0$ means it is **closing in**; $\alpha = 0$ is a waist.

A helper quantity is $\gamma_T = (1 + \alpha^2)/\beta$, and the rms angle is $\sigma' = \sqrt{\gamma_T\varepsilon}$ (the subscript T distinguishes it from the Lorentz factor $\gamma$).

### Step 2: the numbers at the RFQ exit

The example's Twiss parameters come from `part_rfq.dst`: $\alpha_x = -0.658$, $\beta_x = 0.377\ \mathrm{mm/mrad}$ (that is, 0.377 m), normalised rms emittance $\varepsilon_{n,x} = 0.138\ \pi\,\mathrm{mm\,mrad}$, with $y$ similar. Dividing the normalised emittance by $\beta\gamma = 0.05708$ gives the actual (geometric) emittance $\varepsilon = 2.42\ \mathrm{mm\,mrad}$. So

$$
\sigma = \sqrt{0.377 \times 2.42} = 0.954\ \mathrm{mm},\qquad
\sigma' = \sqrt{\frac{1 + 0.658^2}{0.377} \times 2.42} = 3.03\ \mathrm{mrad} .
$$

$\alpha_x < 0$: the beam is opening up. After a drift $L$ the rms size satisfies $\sigma^2 = \varepsilon(\beta - 2\alpha L + \gamma_T L^2)$; for $L = 1\ \mathrm{m}$, $\sigma^2 = 2.42\times10^{-6} \times (0.377 + 1.316 + 3.801)\ \mathrm{m^2}$ and $\sigma = 3.6\ \mathrm{mm}$, the "about 3.7 mm" of the main text. Longitudinally, the same particle file gives an rms length of 1.24 mm (phase $4.3^\circ$, time 73 ps), an rms momentum spread of 0.375 % and an energy spread of 11.4 keV.

### Step 3: where the matched beam is

The first "solenoid + cavity" period of the superconducting section has its own matched beam. From the linear preview's period matrix, at its entrance it is about $\alpha \approx -0.78$, $\beta \approx 0.61$–$0.67\ \mathrm{mm/mrad}$ ($x$ and $y$ similar). The values are uncertain by a few per cent, for two reasons: the solenoid mixes $x$ and $y$ (coupling), so one has to go to the "Larmor coordinates" that rotate with the beam before the two can be treated separately; and the solenoid field map is noisy, which makes the period matrix not exactly area-preserving (not symplectic), see the [tutorial](#tutorial-step4).

### Step 4: the mismatch factor

When two ellipses have the same area but different shapes, the standard measure of how far apart they are is to first compute

$$
R = \beta\,\gamma_{T0} + \beta_0\,\gamma_T - 2\alpha\alpha_0 ,
$$

(subscript 0 for the matched beam), and then the mismatch factor

$$
M = \sqrt{\tfrac12\left(R + \sqrt{R^2 - 4}\right)} - 1 .
$$

For identical ellipses $R = 2$ and $M = 0$. With the RFQ exit ($\alpha = -0.658$, $\beta = 0.377$, $\gamma_T = 3.80$) and the matched beam (taking $\alpha_0 = -0.78$, $\beta_0 = 0.64$, $\gamma_{T0} = 2.51$): $R = 0.377 \times 2.51 + 0.64 \times 3.80 - 2 \times 0.658 \times 0.78 = 2.35$, $M = 0.34$. This is the "about 0.3–0.4" of the main text. The same calculation longitudinally gives about 0.2.

What $M$ means: in a matched periodic channel, a mismatched beam's size beats between $1/(1 + M)$ and $(1 + M)$ times the matched envelope (see [matching](#matching)). $M = 0.35$ means 0.74 to 1.35 times.

### Step 5: why at least six knobs

Matching requires the entrance $(\alpha, \beta)$ to equal the matched values in all three directions: four equations transversely, two longitudinally. The emittance belongs to the beam and cannot, and need not, be adjusted. Each quadrupole gradient and each buncher amplitude is one adjustable number. By "$n$ equations need $n$ unknowns", at least four transverse and two longitudinal knobs are needed. These equations are not linear (the gradients appear inside $\cos$ and $\sin$), and with exactly as many unknowns as equations a solution may not exist, or may need very large gradients or blow the beam up inside the section; a real MEBT usually has a few more quadrupoles and uses the extra freedom to keep the envelope small inside the section. The schematic MEBT of the main text uses seven quadrupoles and two bunchers.

### Step 6: check with the schematic MEBT

Results of the beam line in the main text: at the end of the MEBT the rms $x$ and $y$ are about 1.2 mm and the rms bunch length is 1.48 mm ($5.1^\circ$); at the entrances of the superconducting periods the rms $x$ is 1.24, 1.30 and 1.15 mm. Without the MEBT it is 1.05, 1.64, 0.99 and 1.41 mm: the ratio of largest to smallest is $1.64/0.99 = 1.66$, while for $M = 0.3$ the beating gives a largest-to-smallest ratio of $1.3^2 = 1.69$; the two agree.

### For readers who know linear algebra

One direction of a beam is described by the 2 × 2 σ matrix $\Sigma = \varepsilon\begin{pmatrix}\beta & -\alpha\\ -\alpha & \gamma_T\end{pmatrix}$, which a transfer matrix $M$ turns into $M\Sigma M\tp$. For equal emittances $R = \Tr(\Sigma\,\Sigma_0^{-1})$; this trace is unchanged when the same transport acts on both σ matrices, so the mismatch factor can be computed at any position. Matching means finding knobs $\boldsymbol{u}$ with $\Sigma_{\text{out}}(\boldsymbol{u}) = \Sigma_0$, usually solved by Newton's method or least squares; with space charge $M$ depends on $\Sigma$ itself and the equations become a non-linear fixed-point problem. Solenoid coupling is handled with 4 × 4 matrices; after rotating to Larmor coordinates the transverse motion splits into two 2 × 2 blocks.

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

AVAS usually does not simulate the RFQ; the MEBT's input beam comes from the output of an RFQ code, in one of two ways (see [the RFQ](#journey-rfq)): `readparticledistribution` in `beam.txt` reads a particle file (the example's `part_rfq.dst`; the GUI switches it with `use_dst`; when a file is read, every keyword except `numofcharge` is ignored), or Twiss parameters in all three planes let AVAS generate the distribution.

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
