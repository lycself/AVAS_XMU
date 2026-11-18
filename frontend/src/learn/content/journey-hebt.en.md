# Arrival: the high-energy beam transport and the target

Our proton has finally reached 1 GeV, 87.5 % of the speed of light. If the design current is 10 mA, this beam, only a few millimetres across, carries 10 MW: as much as several thousand electric heaters together, concentrated in a beam as thin as a pencil lead.

The acceleration is over, so why not let the linac run straight on into the target? The beam line between the end of the linac and the target is the **high-energy beam transport** (HEBT), and this chapter answers three questions about it:

1. Why does it bend?
2. Why does it deliberately make the beam bigger instead of focusing it as small as possible?
3. Why worry about losing one particle in ten million per metre?

Keep two everyday pictures in mind. One is a magnifying glass: focus sunlight into a small spot and it sets paper alight; spread the same sunlight over the paper and it only gets warm. A high-power beam on a target is the same: the spot has to be spread out. The other is a prism: white light passing through it fans out into a rainbow, because different colours are bent by different amounts. When a magnet bends the beam, protons of different energy are bent by different amounts and also "fan out"; to bring them back together you need two "prisms" set opposite to each other.

Finally, the chapter walks through the whole journey again on one page. It uses [the dipole](#dipole), [the quadrupole](#quadrupole) and [errors and correction](#errors); the numbers for every stage are in [the overview](#journey-overview-thread).

## The proton on arrival {#journey-hebt-arrival}

At 1 GeV: Lorentz factor $\gamma = 2.066$ (the proton's total energy is 2.066 times its rest energy), $\beta = 0.8750$, $\beta\gamma = 1.808$, momentum 1696 MeV/c, magnetic rigidity $B\rho = 5.657\ \mathrm{T\,m}$, 32 times its value at the RFQ exit: the same magnet bends it only one thirty-second as much.

The beam has become thinner. If the normalized rms emittance (the beam quality that does not change with acceleration, see [drift](#drift-size)) stays at 0.14 π·mm·mrad all the way, the actual (geometric) emittance is only $0.14/1.808 = 0.077\ \pi\,\mathrm{mm\,mrad}$; where the β function is 10 m (the β function describes how thick the beam is at a given place; this value is for illustration only), the rms beam size is about 0.9 mm. At 650 MHz the bunch is about 0.8 mm (3 ps) rms long, with an rms energy spread of about 0.26 MeV, i.e. a relative momentum spread $\sigma_\delta \approx 2\times10^{-4}$ (from the estimate with a conserved longitudinal emittance in [the overview](#journey-overview-frequency)).

Almost every problem of the low-energy sections has disappeared. The strength of space charge (the generalized perveance) goes as $1/(\beta\gamma)^3$, so the same 10 mA at 1 GeV gives about one thirty-thousandth of its value at 1.527 MeV; the RF defocusing is just as weak.

A new problem has taken over: **power**. Beam power follows the school formula $P = UI$: the proton carries one unit of charge and 1 GeV is like being accelerated through $10^9$ V, so the example's 0.18 mA at 1 GeV is $10^9\ \mathrm{V} \times 0.184\ \mathrm{mA} = 184\ \mathrm{kW}$. High-power CW proton linacs are typically designed for a few to somewhat over ten milliamperes, and 10 mA is 10 MW. Almost everything in the HEBT design revolves around this: deliver that power safely to where it belongs, and lose almost none of it on the way.

## Why bend: dispersion and achromats {#journey-hebt-bending}

**Why not go straight.** Putting the target straight in front of the linac axis has several drawbacks (typical considerations that differ between machines): neutrons and other radiation from the target would shine back along the beam pipe into the accelerator; commissioning usually needs a beam dump straight ahead, separate from the target; and a bending magnet also lets one measure and limit the beam energy. So the HEBT usually bends the beam with **dipoles** (magnets with a uniform field that bend the beam, see [the dipole](#dipole)).

**The bending radius.** A proton in a uniform magnetic field moves on a circle, the Lorentz force providing the centripetal force: $qvB = mv^2/\rho$, which rearranges to $B\rho = mv/q = p/q$; this is where the name magnetic rigidity comes from. So a dipole's field is $B = B\rho/\rho$. At 1 GeV a 1.5 T room-temperature iron-dominated dipole (iron saturation usually limits such magnets to about 1.5–1.8 T) gives a bending radius $\rho = 5.657/1.5 = 3.77\ \mathrm{m}$. Compared with the 0.12 m radius the same field would give at the RFQ exit, the high-energy beam is much "stiffer".

**Dispersion: the magnet is the beam's prism.** Bending brings a new effect. A proton with higher momentum is "stiffer", takes a larger radius in the same field and drifts to the outside of the bend. For a relative momentum deviation $\delta$, the displacement from the central orbit is

$$
x = D\,\delta ,
$$

where $D$ is the **dispersion function**, in metres: 1 % more momentum moves the position by $0.01\,D$. At the exit of a sector dipole with bending angle $\theta$ and radius $\rho$,

$$
D = \rho\,(1 - \cos\theta),\qquad D' = \sin\theta
$$

($D'$ is the slope at which the dispersion grows along the path; a derivation with school geometry is in [Going deeper: the geometry of dipoles, dispersion and achromats](#journey-hebt-deep-dispersion), see also [the dipole](#dipole)). For $15^\circ$ and $\rho = 3.77\ \mathrm{m}$, $D = 0.128\ \mathrm{m}$ and $D' = 0.259$. After the exit the dispersion grows along the drift like a straight sloping line and reaches $0.128 + 5 \times 0.259 = 1.42\ \mathrm{m}$ after 5 m.

What does that mean? If an upstream cavity's phase were slightly off and the beam energy 1 MeV too low, with a relative momentum deviation $\delta = \Delta W/(\beta^2\gamma\,mc^2) = 6.7\times10^{-4}$ (the conversion is in [the medium-energy transport](#journey-mebt-deep-bunching)), the beam centre on the target would move by $1.42 \times 6.7\times10^{-4} \approx 0.95\ \mathrm{mm}$, as much as the rms beam size. A beam spot that wanders on the target with every energy fluctuation is not acceptable.

In the lab below, with an energy of 1000 MeV, a bending angle of $15^\circ$ and a radius of 3.77 m, you can read the required field of 1.50 T, the exit dispersion of 0.128 m and $D' = 0.259$.

```avas-lab
dipole angle=15 rho=3.77 W=1000 delta=0.1
```

**Achromat: two prisms set opposite.** The cure is an **achromat**: an arrangement of dipoles and quadrupoles that makes $D$ and $D'$ both zero at the exit of the bending section, so that downstream the beam position no longer depends on energy. The simplest one uses mirror symmetry: two identical dipoles with a quadrupole exactly in the middle.

- Before the quadrupole the dispersion grows with slope $D' = \sin\theta$, like a ray slanting outwards;
- the quadrupole acts like a lens and bends this "ray" back. If it turns the slope exactly round to $-\sin\theta$, the second half is the mirror image of the first;
- in the first half the dispersion rises from 0 to its maximum, so in the second half it falls from the maximum back to 0: at the exit of the second dipole $D = D' = 0$.

A thin lens changes the slope by $-D_q/f$ ($D_q$ is the dispersion at the quadrupole and $f$ its focal length); to go from $+\sin\theta$ to $-\sin\theta$ it must change it by $2\sin\theta$:

$$
\frac{D_q}{f} = 2\sin\theta \quad\Rightarrow\quad f = \frac{D_q}{2\sin\theta} .
$$

With 2 m from the dipole exit to the quadrupole centre, $D_q = 0.128 + 2\times0.259 = 0.646\ \mathrm{m}$ and $f = 1.25\ \mathrm{m}$; for a 0.3 m quadrupole the gradient is $G = B\rho/(fL_q) \approx 15.1\ \mathrm{T/m}$ (focal-length formula in [the quadrupole](#quadrupole-focal)). With thick-lens matrices (AVAS's linear preview) the exact value is 15.7 T/m; then $D$ and $D'$ are both zero at the exit, and the largest dispersion inside is about 0.63 m.

## Spreading the beam on the target {#journey-hebt-expansion}

**The magnifying glass in reverse.** A 10 MW beam hitting the target as a Gaussian spot with 1 mm rms has a central power density of $P/(2\pi\sigma_x\sigma_y) \approx 1.6\ \mathrm{MW/mm^2}$: over a million watts per square millimetre. No material can take that. How much a target can take depends on its material and cooling and varies widely between facilities; but whatever the target, the spot has to be enlarged from millimetres to centimetres or even tens of centimetres, its area by a factor of a thousand to ten thousand.

**Enlarging with linear optics.** The rms size is $\sigma = \sqrt{\beta\varepsilon}$; for $\sigma = 30\ \mathrm{mm}$ with a geometric emittance of 0.077 π·mm·mrad one needs $\beta \approx 1.2\times10^4\ \mathrm{m}$. This is not impossible: just as a torch beam gets wider the further it shines (see [drift](#drift-spread)), a beam starting from a very narrow waist becomes very wide after a long enough drift. With the formula of [Going deeper: the beam spot on the target and loss statistics](#journey-hebt-deep-target), starting from a waist with β function $\beta^* = 1\ \mathrm{m}$, a drift of about 108 m suffices (for illustration only; in practice the emittance in front of the target is often larger than the ideal value, and the β needed is smaller).

**But the shape stays the same.** Linear enlargement does not change the shape of the distribution: an enlarged Gaussian spot is still high in the middle and low around the edge, with a central power density about 2.5 times that of a uniform distribution over the same area (taken as the $\pm 2\sigma$ square), and the life of the target is set by its hottest spot. Two common improvements (typical practice):

- **Non-linear magnets**: octupoles and other multipoles act more strongly on particles far from the axis and "fold" the tails of the distribution back, making the distribution on the target nearly uniform.
- **Beam rastering**: fast-changing deflecting magnets sweep a smaller spot back and forth over the target face, like waving a torch quickly to light up a wall; the sweep is much faster than the thermal response of the target, so on average the face is heated evenly.

> **AVAS cannot simulate either approach directly**: AVAS has no matrix model for octupoles or other multipoles, and no time-dependent deflecting magnets; the target itself is outside the simulation altogether. The linear part (quadrupoles, dipoles, drifts) can be simulated, and the particle distribution before the target can be passed to a dedicated target code (see [In AVAS](#journey-hebt-in-avas)).

## Beam loss: why one watt per metre {#journey-hebt-losses}

**Activation.** High-energy protons hitting the beam pipe or a magnet cause nuclear reactions and make the equipment radioactive (**activation**). After a shutdown people have to work near the equipment, so high-power proton linacs widely use an empirical figure: beam loss along the line of no more than about **1 W/m**, which keeps the residual activation low enough for hands-on maintenance (a general design guideline, not a physical limit; it applies to sections whose energy is high enough to cause significant activation).

**How strict is it?** Each 1 GeV proton carries $10^9\ \mathrm{eV} = 10^9 \times 1.602\times10^{-19}\ \mathrm{J}$, so 1 W is $1/(10^9 \times 1.602\times10^{-19}) = 6.2\times10^{9}$ protons per second. A 10 mA beam carries $0.01/(1.602\times10^{-19}) = 6.2\times10^{16}$ protons per second, so the fraction allowed to be lost per metre is $10^{-7}$, one in ten million; for the example's 0.18 mA (184 kW) it is $5.4\times10^{-6}$.

**What it asks of simulations.** This is easy to overlook. The example project uses 5260 macroparticles (each "macroparticle" stands for a large group of real protons), so each represents $1/5260 = 1.9\times10^{-4}$ of the beam, 1.9 kW at 10 MW. In other words, even if the simulation loses not a single particle, that only shows the loss is below the kilowatt level, nowhere near proving 1 W/m. Studying beam loss needs far more particles, attention to halo formation (mismatch and non-linearities are the main sources, see [matching](#matching)), and error studies that show how close the envelope comes to the aperture in the worst case (see [errors and correction](#errors)).

**Collimators.** The HEBT usually also has dedicated **collimators** that remove halo particles so that losses happen at designed, well-shielded places. A common rule: two collimators about 90° of transverse phase advance apart, one stopping particles with large position and the other particles with large angle. The reason is the same as for a pendulum: the bob has zero speed at its furthest point and the largest speed in the middle, so after a quarter period "large position" has become "large speed". In the beam, every 90° of phase advance in the particles' transverse swing turns particles with large angle into particles with large position (see [Going deeper: the beam spot on the target and loss statistics](#journey-hebt-deep-target)). Low-energy scraping is done in the MEBT, where protons cause almost no activation (see [the medium-energy transport](#journey-mebt-chopper)).

## What the proton gained in all {#journey-hebt-total}

Putting the start and the end side by side:

| Quantity | Ion-source exit | RFQ exit | End | End / start |
|---|---|---|---|---|
| Kinetic energy $W$ | 35 keV | 1.527 MeV | 1 GeV | about 28,600× |
| $\beta$ | 0.00864 | 0.05698 | 0.8750 | 101× |
| Momentum $p$ | 8.10 MeV/c | 53.6 MeV/c | 1696 MeV/c | 209× |
| $B\rho$ | 0.0270 T·m | 0.1786 T·m | 5.657 T·m | 209× |
| Geometric rms emittance (normalized 0.14 constant) | 16.2 | 2.45 | 0.077 π·mm·mrad | 1/209 |
| rms bunch duration | DC | 73 ps | about 3 ps | — |

A few things are worth taking in:

- **The speed grew only 101 times, the energy more than twenty thousand times.** In the second half the proton is already close to the speed of light, and the energy given to it goes almost entirely into $\gamma$ rather than speed.
- **Momentum and $B\rho$ grew 209 times**, which means the same magnet has only 1/209 of its bending power at the end.
- **The geometric emittance shrank by the same factor** (adiabatic damping) and the beam got thinner and thinner, but this is not "cooling": ideally the normalized emittance is constant, and in reality it grows a little with every mismatch and every non-linear field.
- **In time**, the beam went from a continuous stream to pulses a few picoseconds long, and that is what allows the frequency to double from 162.5 MHz to 650 MHz.

## Looking back over the whole journey {#journey-hebt-recap}

**Ion source and LEBT** ([birth](#journey-source)): the proton is produced in a plasma and extracted by a few tens of kilovolts as a 35 keV DC beam. The Child–Langmuir law ties the current to the extraction voltage and gap; at low energy space charge is strongest, so the LEBT focuses both planes at once with solenoids, relies on electrons from residual-gas ionization to partly neutralize the space charge, and delivers a strongly converging beam to the RFQ. AVAS does not simulate this section.

**RFQ** ([bunching](#journey-rfq)): a DC beam cannot go straight into cavities; only a quarter of it would be captured by the bucket. The RFQ focuses with an electric quadrupole field (an electric force that does not weaken with speed), uses electrode modulation to create a longitudinal field, slowly cuts the continuous beam into bunches and then accelerates them to 1.527 MeV. AVAS usually does not simulate the RFQ; its output `part_rfq.dst` is the starting point of the AVAS example.

**MEBT** ([shaping](#journey-mebt)): after leaving the RFQ the bunches lengthen on their own because of their momentum spread ($R_{56} = L/\gamma^2$), the beam diverges transversely, and its shape does not suit the downstream section. Quadrupoles and non-accelerating bunchers match the beam to the superconducting section in all three planes, while chopping, diagnostics and low-energy scraping happen on the way. AVAS simulation starts here.

**Superconducting section** ([acceleration](#journey-sc)): in CW operation only superconducting cavities have tolerable wall losses. The velocity keeps changing and one cavity family serves only a range of β, so there are several families, HWRs, spoke and elliptical cavities, and the frequency doubles as the bunches shorten. At low energy "solenoid + cavity" periods alternate to keep RF defocusing in check; the synchronous phase of $-30^\circ$ to $-20^\circ$ is a compromise between energy gain and longitudinal stability; the phase advance per period stays below 90° in both directions.

**HEBT** (this chapter): power has become the main issue. Dipoles bend the beam and an achromat keeps the beam spot on the target from wandering with energy; the spot is enlarged and made uniform to protect the target; losses are kept to about 1 W per metre.

Behind the whole route there are only three facts (see [the overview](#journey-overview)): the proton's speed changes a hundredfold, space charge is strongest at low energy, and RF can only accelerate a bunched beam. The structure of every section is the best answer to these three facts at the speed and current found there; the matching between sections makes sure each section's answer can be taken over by the next.

## Think about it {#journey-hebt-think}

> **Think about it 1**: inside an achromat the dispersion is not zero. Does the beam get wider there?
>
> Answer: yes, but very little. At the quadrupole $D \approx 0.63\ \mathrm{m}$; times the momentum spread $\sigma_\delta \approx 2\times10^{-4}$ that is only about 0.12 mm, which added to the 0.9 mm beam size by Pythagoras increases it by less than 1 %. A high-energy beam has a small energy spread, so this part is easy; the hard part is the coupling of a shift of the whole beam energy (cavity phase or amplitude errors) to the downstream position, which is exactly what the achromat removes. Conversely, a slit placed where the dispersion is large can stop particles whose energy is too far off, or measure the energy spread.

> **Think about it 2**: for the same $15^\circ$ bend with a 1.5 T dipole at the RFQ exit (1.527 MeV), what is the dispersion $D$? Why is dispersion "less scary" at low energy?
>
> Answer: $\rho = 0.1786/1.5 = 0.12\ \mathrm{m}$, $D = 0.12 \times (1 - \cos 15^\circ) = 4\ \mathrm{mm}$, more than thirty times smaller than at 1 GeV. Dispersion is proportional to the bending radius, and the radius to $B\rho$. On the other hand, the relative energy spread of a low-energy beam is much larger (the rms momentum spread at the RFQ exit is 0.375 %), so what really has to be compared is $D\,\sigma_\delta$ against the beam size.

> **Think about it 3**: if the number of simulated particles in the example were raised from 5260 to 100,000, what would "not a single particle lost" show?
>
> Answer: each macroparticle would represent $10^{-5}$ of the beam, 100 W at 10 MW. Losing none, statistically (see [Going deeper: the beam spot on the target and loss statistics](#journey-hebt-deep-target)), only shows that the loss fraction is probably below $3\times10^{-5}$, i.e. a few hundred watts, still far above 1 W per metre. Proving losses at the $10^{-7}$ level by particle count alone would need tens of millions of particles, so in practice it has to be combined with halo analysis and error studies.

## Common misconceptions {#journey-hebt-misconceptions}

**"At high energy, beam dynamics becomes simple."** Space charge and RF defocusing do become weak, but the beam power makes every bit of loss expensive: a loss of $10^{-5}$ at 10 MW is 100 W.

**"An achromat has no dispersion anywhere."** An achromat only guarantees $D = D' = 0$ at the exit of the bending section; inside, the dispersion can be large (about 0.63 m in the example above), and a beam with a large energy spread gets wider there.

**"The smaller the beam on target, the better the aim."** For a high-power target it is the opposite: the spot must be enlarged and made as uniform as possible.

**"1 W/m is a physical limit."** It is an empirical guideline for maintainability; at lower energy the same power causes much less activation and the requirement can be relaxed, according to the facility's radiation protection design.

**"No particles lost in the simulation means no beam loss."** With 5260 macroparticles, losing one is $1.9\times10^{-4}$ of the beam; losing none only shows the loss is below that level.

**"The proton is already close to the speed of light, so further acceleration is useless."** Acceleration changes energy and momentum, which still grow linearly near the end; only the speed hardly changes any more.

## Going deeper: the geometry of dipoles, dispersion and achromats {#journey-hebt-deep-dispersion}

The main text gave $D = \rho(1 - \cos\theta)$, $D' = \sin\theta$ and the achromat condition $f = D_q/(2\sin\theta)$. This section derives them with nothing but school circular motion and triangles.

### Step 1: where the magnetic rigidity comes from

A charged particle moving perpendicular to a uniform magnetic field feels the Lorentz force $qvB$, always perpendicular to its velocity, which provides the centripetal force:

$$
qvB = \frac{mv^2}{\rho}\quad\Rightarrow\quad B\rho = \frac{mv}{q} = \frac{p}{q} .
$$

At high speed $p = \gamma mv$ and the result is unchanged (just replace $m v$ by the true momentum). The usual conversion is $B\rho\,[\mathrm{T\,m}] = p\,[\mathrm{MeV}/c]/299.79$: a 1 GeV proton has $p = 1696\ \mathrm{MeV}/c$ and $B\rho = 5.657\ \mathrm{T\,m}$. In $B = 1.5\ \mathrm{T}$ its radius is $\rho = 3.77\ \mathrm{m}$.

### Step 2: two circles

Draw the field region of the dipole as a sector starting at the entrance and turning by $\theta$ about its centre $O$. The reference proton follows an arc of radius $\rho$, starting at the entrance point $P$ with its velocity along the tangent. A proton with $\delta$ more momentum starts from the same point in the same direction, with radius $\rho(1 + \delta)$. The two circles touch at $P$, so both centres lie on the line through $P$ perpendicular to the velocity: the centre $O'$ of the high-momentum proton is $\rho\delta$ further from $P$ than $O$, i.e. on the far side of $O$, at distance $\rho\delta$ from it.

### Step 3: measure the offset at the exit

The exit of a sector magnet is the radius from $O$ at angle $\theta$ to $OP$. The high-momentum proton crosses this line at a point $X$; let $OX = \rho + x$, so $x$ is its distance from the reference orbit. In triangle $OO'X$ the sides are $OO' = \rho\delta$, $O'X = \rho(1 + \delta)$ and $OX = \rho + x$, with $\angle O'OX = 180^\circ - \theta$. By the law of cosines:

$$
\rho^2(1 + \delta)^2 = (\rho + x)^2 + \rho^2\delta^2 + 2(\rho + x)\rho\delta\cos\theta .
$$

$\delta$ and $x$ are both small; keep only first-order terms (drop $\delta^2$, $x^2$ and $x\delta$): $\rho^2 + 2\rho^2\delta = \rho^2 + 2\rho x + 2\rho^2\delta\cos\theta$, hence

$$
x = \rho\,\delta\,(1 - \cos\theta),\qquad D = \frac{x}{\delta} = \rho\,(1 - \cos\theta) .
$$

With $\rho = 3.77\ \mathrm{m}$ and $\theta = 15^\circ$: $D = 3.77 \times (1 - 0.9659) = 0.128\ \mathrm{m}$, the same as the lab's 0.128 m.

### Step 4: the slope of the dispersion

The growth rate of the dispersion along the path, $D'$, tells us how the dispersion continues after the exit. Turn the sector by a further small angle $\Delta\theta$: $D$ changes from $\rho(1 - \cos\theta)$ to $\rho(1 - \cos(\theta + \Delta\theta))$, an increase of about $\rho\sin\theta\,\Delta\theta$ (angle-sum formula plus small-angle approximation); the arc length of this piece is $\rho\,\Delta\theta$. Dividing:

$$
D' = \sin\theta .
$$

For $15^\circ$, $D' = 0.259$. Outside the magnet the dispersion grows in a straight line like a ray: after a drift $L$, $D = 0.128 + 0.259\,L$, which is 0.646 m for $L = 2\ \mathrm{m}$ and 1.42 m for $L = 5\ \mathrm{m}$.

### Step 5: the mirror-symmetric achromat

Think of the achromat as a "video of the dispersion". First half: from the entrance of the first dipole ($D = 0$, $D' = 0$) to the quadrupole in the middle, the dispersion rises to $D_q$ with slope $+\sin\theta$. The second half is the mirror image of the first. Play the video of the second half backwards and it looks like the first half, except that the sign of the slope is reversed (slanting outwards becomes slanting inwards when played backwards). So provided the slope at the midpoint is exactly $-\sin\theta$, the second half takes the dispersion "back along the same road" as the first half, and at the exit of the second dipole $D = 0$ and $D' = 0$.

A thin lens acts on the dispersion as on a ray: the position stays, the slope changes by $-D_q/f$. Requiring $\sin\theta - D_q/f = -\sin\theta$ gives $f = D_q/(2\sin\theta)$. With $D_q = 0.646\ \mathrm{m}$: $f = 0.646/(2 \times 0.2588) = 1.25\ \mathrm{m}$, gradient $G = B\rho/(fL_q) = 5.657/(1.25 \times 0.3) = 15.1\ \mathrm{T/m}$. The exact thick-lens value of 15.7 T/m comes from the quadrupole's 0.3 m length: inside the quadrupole the dispersion is bent back while it moves on, so the effective position and strength differ slightly from a thin lens. Checked with AVAS's linear preview: at 15.74 T/m the exit $D$ and $D'$ are both below $10^{-3}$, and the largest dispersion in the section is 0.63 m (near the quadrupole); with the thin-lens 15.1 T/m, $D = 0.066\ \mathrm{m}$ remains at the exit.

### For readers who know linear algebra

The 3 × 3 transfer matrix of a sector dipole (zero pole-face angles) in the bending plane, acting on $(x, x', \delta)$, is

$$
\begin{pmatrix}
\cos\theta & \rho\sin\theta & \rho(1 - \cos\theta)\\
-\sin\theta/\rho & \cos\theta & \sin\theta\\
0 & 0 & 1
\end{pmatrix},
$$

whose third column is the $D$ and $D'$ of steps 3 and 4; $-\sin\theta/\rho = -0.0687\ \mathrm{m^{-1}}$ is the dipole's own weak focusing, in agreement with the $R_{21}$ the lab reports. The dispersion function satisfies $D'' + K(s)D = 1/\rho(s)$, and the achromat condition is $R_{16} = R_{26} = 0$ for the whole section; more general designs such as the double-bend achromat (DBA) and triple-bend achromat build on this and control the β functions at the same time.

## Going deeper: the beam spot on the target and loss statistics {#journey-hebt-deep-target}

The main text used three results: a drift from a waist can raise the β function to $10^4$ m; two collimators 90° of phase advance apart stop particles with large position and large angle respectively; and no losses in a simulation cannot prove that the loss is very low. This section works each one out.

### Step 1: β function and beam size

In one direction, the rms size is $\sigma = \sqrt{\beta\varepsilon}$, where $\varepsilon$ is the geometric emittance (carried by the beam, unchanged by a drift) and $\beta$ describes how thick the beam is at this place (see [drift](#drift-deep-matrix)). At 1 GeV with $\varepsilon = 0.077\ \mathrm{mm\,mrad}$, $\sigma = 30\ \mathrm{mm}$ needs $\beta = \sigma^2/\varepsilon = 900/0.077 \approx 1.2\times10^4\ \mathrm{m}$ (the unit $\mathrm{mm^2}/(\mathrm{mm\,mrad})$ is the metre).

### Step 2: a drift from a waist

At a waist the beam is thinnest, with size $\sigma^*$, positions and angles uncorrelated, and rms angle $\sigma'^* = \varepsilon/\sigma^*$ (emittance ≈ size × angle). After a drift $L$, each particle's position increases by "angle × $L$". The original positions and the added offsets are unrelated, so by Pythagoras:

$$
\sigma^2 = \sigma^{*2} + (\sigma'^*L)^2 .
$$

Divide both sides by $\varepsilon$ and use $\sigma^{*2} = \beta^*\varepsilon$, $\sigma'^{*2} = \varepsilon/\beta^*$:

$$
\beta = \beta^* + \frac{L^2}{\beta^*} .
$$

For $\beta^* = 1\ \mathrm{m}$, reaching $\beta = 1.2\times10^4\ \mathrm{m}$ needs $L = \sqrt{(\beta - \beta^*)\beta^*} \approx 108\ \mathrm{m}$. The narrower the waist (the smaller $\beta^*$), the faster the beam grows over the same distance: the principle of the torch.

### Step 3: the hottest spot of a Gaussian

The power density at the centre of a two-dimensional Gaussian spot is $P/(2\pi\sigma_x\sigma_y)$. For $P = 10\ \mathrm{MW}$ and $\sigma_x = \sigma_y = 1\ \mathrm{mm}$ it is $10^7/(2\pi) = 1.6\times10^6\ \mathrm{W/mm^2}$. If the same power were spread uniformly over a square of side $4\sigma$ ($\pm 2\sigma$), the power density would be $P/(16\sigma^2)$. The ratio is $16/(2\pi) = 2.5$: for the same spot size, the centre of a Gaussian is 2.5 times hotter than a uniform distribution.

### Step 4: why collimators are 90° apart

In a periodic focusing channel the transverse motion of a particle is a simple harmonic oscillation: $x = A\cos\phi$, angle $x' \propto -A\sin\phi$, with $\phi$ the phase advance travelled. As with a pendulum, the angle is zero when the position is largest, and the angle is largest when the position is zero. The first collimator removes particles whose $\lvert x\rvert$ is large at that moment; particles that are in the middle at that moment but carry a large angle have, after another $90^\circ$ of phase advance, swapped $\cos$ and $\sin$ and become particles with large position, which the second collimator removes. Together, the two stop particles of large amplitude in phase space, whatever stage of their swing they are in.

### Step 5: what no losses show

Suppose the true loss fraction is $p$ and the simulation uses $N$ macroparticles. Each macroparticle is lost independently with probability $p$, so the probability that none is lost is $(1 - p)^N$. For small $p$, $(1 - p)^N \approx \ee^{-pN}$ (check with a calculator: for $p = 10^{-4}$ and $N = 5260$ both are about 0.591). For the probability of "none lost" to be only 5 %, we need $\ee^{-pN} = 0.05$, i.e. $pN \approx 3$. So:

> If none of $N$ simulated particles is lost, this only shows (with 95 % confidence) that the loss fraction is below about $3/N$.

For $N = 5260$ that is $5.7\times10^{-4}$; proving $10^{-7}$ needs about $3\times10^7$ macroparticles. This is the quantitative meaning of "studying beam loss needs far more particles" in the main text, and the reason why practical work combines it with halo analysis and error studies.

### For readers who know statistics and linear algebra

Step 2 is the (1,1) element of the σ-matrix transformation $\Sigma \to M\Sigma M\tp$ by a drift matrix; at a general position $\beta(s) = \beta^* + (s - s^*)^2/\beta^*$. Step 4 says that phase space normalized as $(x, \alpha x + \beta x')$ rotates rigidly; two collimators cut a polygonal aperture in normalized phase space, closer to a square the closer the phase advance is to 90°. Step 5 is the upper confidence limit for zero counts of a Poisson distribution (the "rule of three"); a real beam-loss study must also keep in mind that the sampled macroparticles only represent the initial distribution, that halo particles are produced by non-linearities and mismatch, and that with too few particles even the shape of the halo cannot be described.

## In AVAS {#journey-hebt-in-avas}

### What can be simulated

The linear part of the HEBT can all be simulated in AVAS: drifts `drift`, quadrupoles `quad`, dipoles `bend` with an `edge` on each side, correctors `steerer`; with field maps, magnetostatic field maps (field type 3) as well. The achromat above as an AVAS lattice (sector dipoles, pole-face angle 0; aperture 5 cm):

```text
start
drift  0.5   0.05  0
edge   0  0.05  0  0  3.77  0  0  0  0
bend   0  0.05  0  15  3.77  0  0
edge   0  0.05  0  0  3.77  0  0  0  0
drift  1.85  0.05  0
quad   0.3   0.05  0  15.74
drift  1.85  0.05  0
edge   0  0.05  0  0  3.77  0  0  0  0
bend   0  0.05  0  15  3.77  0  0
edge   0  0.05  0  0  3.77  0  0  0  0
drift  0.5   0.05  0
end
```

```avas-params
bend edge quad steerer
```

> **AVAS convention**: in `bend 0 R 0 alpha rho N HV` the first parameter is the arc length, which can be written as 0 (the program computes it from α and ρ); `alpha` is the bending angle (°), `rho` the bending radius (m), `N` the field index, `HV` selects the bending plane (0 = horizontal, 1 = vertical); in the `edge 0 R 0 beta rho G K1 K2 HV` on either side, `beta` is the pole-face rotation angle, `G` the magnet gap and `K1`/`K2` the fringe-field factors, and `rho` and `HV` must agree with the `bend` (see [the dipole](#dipole)). In `quad`, $G > 0$ focuses in $x$. In multi-particle runs the first and last elements should not be matrix elements, which is why the lattice above starts and ends with a drift.

> **Note**: when a dipole is described by a field map, the linear preview does not model the map's dipole component and gives a warning; the engine's result is the reference (see [the dipole](#dipole)). The numbers in this chapter all come from the matrix model.

### What is outside AVAS's elements

The target, octupoles and other non-linear multipoles, time-dependent elements such as raster magnets, and choppers are not AVAS elements. There is no dedicated collimator element, but the engine judges particle loss by each element's aperture `R`, so a drift with a smaller aperture can stand in for one in the simulation; with `boundary 1` in `input.txt`, the boundaries of `boundary.txt` are used instead.

### Handing the beam to a target code

Dump the particle distribution before the target with `outputplane` and pass it to a dedicated target or shielding code. An output plane may not lie beyond the end of the lattice and fails when it is too close to the end (5 mm); 2 cm or more is fine; at the end of a run the engine always writes `outData_<end>.dst` for the end of the lattice (see [simulation settings and output](#simulation)).

```avas-params
outputplane beam:particlenumber
```

### Estimating beam loss

- On the Results page, look at the number of surviving macro-particles (DataSet column 28) and at the ratio of maximum to rms size (columns 22/24 over 16/18): a steadily growing ratio means halo is forming.
- The number of macro-particles `particlenumber` sets the smallest loss fraction that can be resolved (about its inverse); for low-loss studies increase it and use error studies (the `err_*` commands) to look at the worst cases.
- The linear preview warns "expect losses" when 3 × the rms size exceeds the aperture; it is based on rms values only and cannot replace a multi-particle simulation.

## Further reading {#journey-hebt-reading}

- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer: bending magnets, the dispersion function and achromatic systems.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific: dispersion and beam transport.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: beam halo, beam loss and high-power proton linacs.
- CERN Accelerator School (CAS) proceedings on beam transport, beam loss and collimation.
- AVAS manual `docs/使用说明20260427.docx`: the `bend`, `edge` and `outputplane` keywords.
- This primer: [the dipole](#dipole), [the quadrupole](#quadrupole), [matching](#matching), [errors and correction](#errors), [simulation settings and output](#simulation).
