# Arrival: the high-energy beam transport and the target

Our proton has finally reached 1 GeV, 87.5 % of the speed of light. If the design current is 10 mA, this beam, only a few millimetres across, carries 10 MW. Since the acceleration is over, why not let the linac run straight on into the target? Why does the high-energy beam transport (HEBT) bend, why does it deliberately make the beam bigger, and why worry about losing one particle in ten million per metre? This chapter answers these three questions and ends with one page that walks through the whole journey again. It uses [the dipole](#dipole), [the quadrupole](#quadrupole) and [errors and correction](#errors); the numbers for every stage are in [the overview](#journey-overview-thread).

## The proton on arrival {#journey-hebt-arrival}

At 1 GeV: $\gamma = 2.066$, $\beta = 0.8750$, $\beta\gamma = 1.808$, momentum 1696 MeV/c, magnetic rigidity $B\rho = 5.657\ \mathrm{T\,m}$, 32 times its value at the RFQ exit. If the normalized rms emittance stays at 0.14 π·mm·mrad all the way, the geometric emittance is only $0.14/1.808 = 0.077\ \pi\,\mathrm{mm\,mrad}$; where the β function is 10 m (for illustration only), the rms beam size is about 0.9 mm. At 650 MHz the bunch is about 0.8 mm (3 ps) rms long, with an rms energy spread of about 0.26 MeV, i.e. a relative momentum spread $\sigma_\delta \approx 2\times10^{-4}$ (from the estimate with a conserved longitudinal emittance in [the overview](#journey-overview-frequency)).

Almost every problem of the low-energy sections has disappeared. The generalized perveance of space charge goes as $1/(\beta\gamma)^3$, so the same 10 mA at 1 GeV gives about one thirty-thousandth of its value at 1.527 MeV; the RF defocusing is just as weak. A new problem has taken over: power. The example's 0.18 mA is 184 kW at 1 GeV; high-power CW proton linacs are typically designed for a few to somewhat over ten milliamperes, and 10 mA is 10 MW. Almost everything in the HEBT design revolves around this: deliver that power safely to where it belongs, and lose almost none of it on the way.

## Why bend: dispersion and achromats {#journey-hebt-bending}

Putting the target straight in front of the linac axis has several drawbacks (typical considerations that differ between machines): neutrons and other radiation from the target would shine back along the beam pipe into the accelerator; commissioning usually needs a beam dump straight ahead, separate from the target; and a bending magnet also lets one measure and limit the beam energy. So the HEBT usually bends the beam with dipoles.

A dipole's field is $B = B\rho/\rho$. At 1 GeV a 1.5 T room-temperature iron-dominated dipole (iron saturation usually limits such magnets to about 1.5–1.8 T) gives a bending radius $\rho = 5.657/1.5 = 3.77\ \mathrm{m}$. Compared with the 0.12 m radius the same field would give at the RFQ exit, the high-energy beam is much "stiffer".

Bending brings a new effect: **dispersion**. A proton with higher momentum takes a larger radius in the field and drifts outwards. For a relative momentum deviation $\delta$, the displacement from the central orbit is $x = D\,\delta$, where $D$ is the dispersion function. At the exit of a sector dipole with bending angle $\theta$ and radius $\rho$,

$$
D = \rho\,(1 - \cos\theta),\qquad D' = \sin\theta
$$

(derived in [the dipole](#dipole)). For $15^\circ$ and $\rho = 3.77\ \mathrm{m}$, $D = 0.128\ \mathrm{m}$ and $D' = 0.259$. After the exit the dispersion grows linearly in the drift and reaches 1.42 m after 5 m. If an upstream cavity's phase were slightly off and the beam energy 1 MeV too low ($\delta = \Delta W/(\beta^2\gamma\,mc^2) = 6.7\times10^{-4}$), the beam centre on the target would move by about 0.95 mm, as much as the rms beam size. A beam spot that wanders on the target with every energy fluctuation is not acceptable.

In the lab below, with an energy of 1000 MeV, a bending angle of $15^\circ$ and a radius of 3.77 m, you can read the required field of 1.50 T, the exit dispersion of 0.128 m and $D' = 0.259$.

```avas-lab
dipole angle=15 rho=3.77 W=1000 delta=0.1
```

The cure is an **achromat**: arrange dipoles and quadrupoles so that $D$ and $D'$ are both zero at the exit of the bending section, after which the beam position no longer depends on the energy. The simplest one uses symmetry: two identical dipoles with a quadrupole exactly in the middle. Before the quadrupole the dispersion grows with slope $D' = \sin\theta$; if the quadrupole reverses that slope to exactly $-\sin\theta$, the second half is the mirror image of the first and the dispersion returns to zero at the exit of the second dipole. For a thin lens this requires

$$
\frac{D_q}{f} = 2\sin\theta \quad\Rightarrow\quad f = \frac{D_q}{2\sin\theta}
$$

where $D_q$ is the dispersion at the quadrupole. With 2 m from the dipole exit to the quadrupole centre, $D_q = 0.128 + 2\times0.259 = 0.646\ \mathrm{m}$ and $f = 1.25\ \mathrm{m}$; for a 0.3 m quadrupole the gradient is $G = B\rho/(fL_q) \approx 15.1\ \mathrm{T/m}$. With thick-lens matrices (AVAS's linear preview) the exact value is 15.7 T/m; then $D$ and $D'$ are both zero at the exit, and the largest dispersion inside the section is about 0.63 m.

> **Think about it**: inside the achromat the dispersion is not zero. Does the beam get wider there?
> Answer: yes, but only slightly. At the quadrupole $D \approx 0.63\ \mathrm{m}$; multiplied by the momentum spread $\sigma_\delta \approx 2\times10^{-4}$ it gives only about 0.12 mm, which added in quadrature to the 0.9 mm beam size increases it by less than 1 %. A high-energy beam has a small energy spread, so this is easy; what matters is the coupling of an overall energy offset (phase or amplitude errors of the cavities) to the downstream position, and that is what the achromat removes. Conversely, a slit placed where the dispersion is large can stop particles whose energy deviates too much, or measure the energy spread.

## Spreading the beam on the target {#journey-hebt-expansion}

A 10 MW beam on the target as a Gaussian spot with 1 mm rms has a central power density of $P/(2\pi\sigma_x\sigma_y) \approx 1.6\ \mathrm{MW/mm^2}$. No material can take that. What a target can take depends on its material and cooling and differs widely between facilities; but for any target the spot must be enlarged from millimetres to centimetres or even ten centimetres, an area a thousand to ten thousand times larger.

The most direct way is linear optics: the rms size is $\sigma = \sqrt{\beta\varepsilon}$, so $\sigma = 30\ \mathrm{mm}$ at a geometric emittance of 0.077 π·mm·mrad needs $\beta \approx 1.2\times10^4\ \mathrm{m}$. That is not impossible: drifting a length $L$ from a waist with β function $\beta^*$ gives $\beta = \beta^* + L^2/\beta^*$, for example $\beta^* = 1\ \mathrm{m}$ and about 108 m of drift (for illustration only; in practice the emittance before the target is usually larger than the ideal value, and the required β is smaller).

But linear enlargement does not change the shape of the distribution: an enlarged Gaussian spot is still high in the middle and low at the edges, its central power density about 2.5 times that of a uniform distribution over the same area (taken as a $\pm 2\sigma$ square), and the target's life is set by its hottest point. Two common improvements (typical practice):

- **Non-linear magnets**: octupoles and other multipoles act more strongly on particles far from the axis and "fold" the tails of the distribution back, making the distribution on the target close to uniform.
- **Beam rastering**: fast deflecting magnets sweep a smaller spot back and forth over the target, much faster than the target's thermal response, so that on time average the target is heated uniformly.

> **AVAS cannot simulate either approach directly**: AVAS has no matrix model for octupoles or other multipoles, and no time-dependent deflecting magnets; the target itself is outside the simulation altogether. The linear part (quadrupoles, dipoles, drifts) can be simulated, and the particle distribution before the target can be passed to a dedicated target code (see [In AVAS](#journey-hebt-in-avas)).

## Beam loss: why one watt per metre {#journey-hebt-losses}

High-energy protons that hit the beam pipe or a magnet cause nuclear reactions and leave radioactivity behind. After shutdown people must get close to the equipment for maintenance, so high-power proton linacs widely use a rule of thumb: the beam loss along the line should not exceed about **1 W/m**, so that the residual activation still allows hands-on maintenance (it is a general design guideline, not a physical limit; it applies to sections where the energy is high enough to cause significant activation).

How strict is it? At 1 GeV, 1 W corresponds to $1/(10^9 \times 1.602\times10^{-19}) = 6.2\times10^{9}$ protons per second. A 10 mA beam carries $6.2\times10^{16}$ protons per second, so the allowed loss is a fraction of $10^{-7}$ per metre; for the example's 0.18 mA (184 kW) it is $5.4\times10^{-6}$.

This sets a requirement on simulations that is easily overlooked. The example project uses 5260 macro-particles, each representing $1.9\times10^{-4}$ of the beam, or 1.9 kW at 10 MW. In other words, even if a simulation loses not a single particle, it only shows that the loss is below the kilowatt level, far from proving 1 W/m. Studying beam loss needs many more particles, attention to how halo forms (mismatch and non-linearity are the main sources, see [matching](#matching)), and error studies that show how close the envelope gets to the aperture in the worst cases (see [errors and correction](#errors)).

The HEBT usually also has dedicated **collimators** that stop halo particles, so that the losses are concentrated at places designed and shielded for them. A common principle: two collimators about 90° of transverse phase advance apart, one stopping particles with large positions and the other those with large angles (in phase space, angle turns into position after 90°). Low-energy scraping is done in the MEBT, where the protons hardly cause activation (see [the MEBT](#journey-mebt-chopper)).

## What the proton has gained {#journey-hebt-total}

Start and end side by side:

| Quantity | Ion-source exit | RFQ exit | End | End / start |
|---|---|---|---|---|
| Kinetic energy $W$ | 35 keV | 1.527 MeV | 1 GeV | about 28600 times |
| $\beta$ | 0.00864 | 0.05698 | 0.8750 | 101 times |
| Momentum $p$ | 8.10 MeV/c | 53.6 MeV/c | 1696 MeV/c | 209 times |
| $B\rho$ | 0.0270 T·m | 0.1786 T·m | 5.657 T·m | 209 times |
| Geometric rms emittance (normalized 0.14 kept) | 16.2 | 2.45 | 0.077 π·mm·mrad | 1/209 |
| rms bunch duration | DC | 73 ps | about 3 ps | — |

A few things are worth pausing on. The velocity has grown only 101 times, the energy more than twenty thousand times: in the second half of the journey the proton is close to the speed of light, and the energy given to it goes almost entirely into the growth of $\gamma$, not into velocity. Momentum and $B\rho$ have grown 209 times, so the same magnet bends the proton at the end only 1/209 as much as at the start. The geometric emittance shrinks by the same factor (adiabatic damping) and the beam gets ever thinner, but this is not "cooling": ideally the normalized emittance stays constant, and in reality it grows a little at every mismatch and in every non-linear field. In time, the beam has turned from a continuous stream into pulses a few picoseconds long, and that is what allowed the frequency to double from 162.5 MHz up to 650 MHz.

## Looking back over the whole journey {#journey-hebt-recap}

**Ion source and LEBT** ([birth](#journey-source)): the proton is created in a plasma and extracted by a few tens of kilovolts into a 35 keV DC beam. The Child–Langmuir law ties the current to the extraction voltage and gap; at low energy space charge is strongest, and the LEBT uses solenoids that focus both planes, relies on electrons from residual-gas ionisation to partly neutralise the space charge, and delivers a strongly converging beam into the RFQ. AVAS does not simulate this section.

**RFQ** ([bunching](#journey-rfq)): a DC beam cannot go straight into cavities, since only a quarter of it would be caught by the stable region. The RFQ focuses with an electric quadrupole field (an electric force does not weaken at low velocity), creates a longitudinal field by modulating its electrodes, slowly cuts the continuous beam into bunches and then accelerates them to 1.527 MeV. AVAS does not simulate the RFQ; its output `part_rfq.dst` is the starting point of the AVAS example.

**MEBT** ([shaping](#journey-mebt)): after the RFQ the bunches lengthen by themselves through the momentum spread ($R_{56} = L/\gamma^2$), the beam diverges transversely and has the wrong shape for what follows. Quadrupoles and non-accelerating bunchers match the beam to the superconducting section in all three planes, while the section also chops, measures and scrapes at low energy. AVAS simulation starts here.

**Superconducting section** ([acceleration](#journey-sc)): in CW operation only the wall losses of superconducting cavities are affordable. The velocity keeps changing and one cavity family covers only a range of β, hence the HWR, spoke and elliptical families, with the frequency doubling as the bunches get shorter. At low energy "solenoid + cavity" periods hold the RF defocusing in check; the synchronous phase of $-30^\circ$ to $-20^\circ$ balances energy gain against longitudinal stability; the phase advance per period stays below 90° in both directions.

**HEBT** (this chapter): power has become the main problem. Dipoles bend the beam and an achromat keeps the spot position on the target independent of the energy; the spot is enlarged and made as uniform as possible to protect the target; losses are kept to about 1 W per metre.

Behind the whole route there are only three facts (see [the overview](#journey-overview)): the proton's velocity changes a hundredfold, space charge is strongest at low energy, and RF can only accelerate a bunched beam. Each section's structure is the best answer to these three facts at the velocity and current of that stage; the matching between sections makes sure that one section's answer can be caught by the next.

## Common misconceptions {#journey-hebt-misconceptions}

**"At high energy beam dynamics becomes simple."** Space charge and RF defocusing do become weak, but the beam power makes every bit of loss expensive: a loss of $10^{-5}$ at 10 MW is 100 W.

**"An achromat has no dispersion anywhere."** An achromat only guarantees $D = D' = 0$ at the exit of the bending section; inside, the dispersion can be large (about 0.63 m in the example above), and a beam with a large energy spread becomes wider there.

**"The smaller the beam, the better it hits the target."** For a high-power target it is exactly the opposite: the spot must be enlarged and made as uniform as possible.

**"1 W/m is a physical limit."** It is a rule of thumb for maintainability; at lower energy the same power causes much less activation and the limit can be relaxed; the actual value is set by the facility's radiation-protection design.

**"No particle lost in the simulation means no beam loss."** With 5260 macro-particles, one lost particle is $1.9\times10^{-4}$ of the beam; losing none only shows that the loss is below that level.

**"The proton is already close to the speed of light; further acceleration is pointless."** Acceleration changes the energy and momentum, which still grow linearly near the end; only the velocity hardly changes any more.

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
