# Space charge

So far we have pretended that the particles of a beam ignore each other, each feeling only the magnets and cavities. But protons all carry positive charge, and like charges repel: every proton in the beam is pushed outward by the countless protons around it. This push between the particles is called the **space-charge** effect ("space charge" means charge filling a region of space, as opposed to a single point charge).

How much does it matter? The Coulomb force between two protons has nothing to do with how fast they go; yet the same 10 mA proton beam is almost entirely ruled by space charge at the ion-source exit (some tens of keV), still has to be taken seriously at 1.5 MeV, and is negligible at 1 GeV — the quantity that measures the strength of space charge changes by about seven orders of magnitude along the way. Why? What does space charge actually do to a beam? Accelerator designers often say "the phase advance is depressed to 0.5": what does that mean physically? This chapter answers these questions.

First, have a look. In the animation below the orange dots are 60 protons of the example beam and the two blue dots are particles we follow in particular; they fly through a 1.5 m long vacuum pipe with no elements in it at all (a **drift**). The two long strips are the trajectories seen from above and from the side, and the square on the right is the cross-section "looking into the beam". The beam current has been turned up to 20 mA, more than a hundred times that of the example beam (0.184 mA):

```avas-motion
drift I=20
```

At the entrance the beam is converging: it first gets narrower, then wider. Press play: the beam is narrowest already at about 0.06 m (about 0.87 mm), then swells quickly, reaching an rms size of about 10.4 mm at the exit; 9 of the 60 particles hit the wall of the pipe (red crosses). Drag "Beam current" to 0: the same beam is narrowest only at about 0.19 m (about 0.68 mm), is only about 4.7 mm at the exit, and loses no particles. There is no electric or magnetic field in the pipe; the extra swelling is entirely due to the particles pushing each other. At the example's 0.184 mA the exit size is about 4.8 mm: for the example beam, space charge makes it only about 1 % fatter.

## Like charges repel: Coulomb's law {#space-charge-coulomb}

### Between two protons

**Coulomb's law** from school: the force between two point charges is

$$
F = k\,\frac{q_1 q_2}{r^2},\qquad k = 8.99\times 10^{9}\ \mathrm{N\,m^2/C^2},
$$

repulsive for like charges, attractive for unlike ones. The charge of a proton is $e = 1.602\times 10^{-19}$ C. Two protons 1 mm apart push each other with $8.99\times 10^9\times(1.602\times 10^{-19})^2/(10^{-3})^2 = 2.3\times 10^{-22}$ N. That looks pitifully small, but a proton is also extremely light ($1.67\times 10^{-27}$ kg), and the resulting acceleration is about $1.4\times 10^5\ \mathrm{m/s^2}$, fourteen thousand times the acceleration of gravity.

### How many protons a bunch holds

An RF-accelerated beam travels in packets, each called a **bunch**. The example beam has a mean current $I = 0.184$ mA and $f = 162.5\times 10^6$ bunches arrive per second, so each bunch carries

$$
Q = \frac{I}{f} = \frac{0.184\times 10^{-3}\ \mathrm{A}}{162.5\times 10^{6}\ \mathrm{s^{-1}}} = 1.13\times 10^{-12}\ \mathrm{C},
$$

about 7.07 million protons, packed into a small cloud about 1 mm across and about 1.3 mm long (rms). Every proton is pushed by the other few million at the same time.

### The net force points outward and grows with distance

The net force on a proton is the sum of the pushes of all the others. A proton exactly at the centre of the beam is pushed equally from all sides and feels no net force; further from the centre there are fewer neighbours outside and more inside, so the net force is larger and points outward.

For a cylindrical beam with **uniformly** distributed charge there is a result identical to gravity inside the Earth: at a distance $r$ from the axis only the charge within radius $r$ gives a net push; the shells further out cancel each other exactly. (Gravity inside the Earth behaves the same way: the deeper you go, the weaker it gets, zero at the centre.) The charge within radius $r$ is proportional to $r^2$, while its push weakens as $1/r$ like that of a charged line; multiply the two and **the push is proportional to $r$**.

This is a very important result: a push proportional to the distance from the axis is exactly the hallmark of a "lens" from the [quadrupole](#quadrupole) chapter, only pointing outward. So the space charge of a uniform beam is a **defocusing lens**, a "negative spring" pushing outward, and it cancels part of the applied focusing. The actual field strength is worked out in the next section.

## Self-fields: the electric field pushes, the magnetic field pulls {#space-charge-self-fields}

### How strong the electric field is

Approximate the beam as a long cylinder of radius $a$ with uniformly distributed charge, carrying a charge $\lambda_q$ per metre (the **line charge density**). The current is the charge passing per second, and the particles travel $v$ metres per second, so

$$
\lambda_q = \frac{I}{v} = \frac{I}{\beta c}.
$$

Outside the beam the field is that of a charged line, $E = 2k\lambda_q/r$ (the same form as the school formula for the magnetic field of a straight wire, $B \propto I/r$); inside, by the argument of the previous section, only the charge $\lambda_q r^2/a^2$ within radius $r$ counts:

$$
E_r = \frac{2k\lambda_q}{a^2}\,r = \frac{\lambda_q}{2\pi\varepsilon_0}\frac{r}{a^2}\qquad (r \le a),
$$

where $\varepsilon_0 = 1/(4\pi k)$ is the vacuum permittivity.

Let us estimate it for the example beam. The charge of a bunch is packed together; in terms of the "[equivalent current](#space-charge-perveance)" below, the centre of the bunch acts like a continuous beam of 3.69 mA. The example proton has $v = \beta c = 1.71\times 10^7$ m/s, so $\lambda_q = 3.69\times 10^{-3}/1.71\times 10^7 = 2.16\times 10^{-10}$ C/m. The edge of the uniform beam is at about twice the rms size, $a = 2\times 0.954 = 1.9$ mm, where the field is about 2.0 kV/m (two thousand volts per metre). If the beam stayed this narrow, a proton at the edge would gain about 0.66 mrad of outward angle per metre, while the example beam's own rms angle is about 3 mrad — space charge is not large, but no longer completely negligible.

### Moving charges are a current too: the magnetic field pulls them back

The protons in the beam all fly forward, and moving charges are a current. School physics says: **two parallel wires carrying currents in the same direction attract each other**. So the beam's own magnetic field pulls the particles inward, cancelling part of the electric push.

The ratio of magnetic to electric force is $\beta^2 = (v/c)^2$ (derivation in [Going deeper](#space-charge-deep-fields)), so the net push is only

$$
1 - \beta^2 = \frac{1}{\gamma^2}
$$

of the electric force. For the example proton, $\beta = 0.057$ and $1 - \beta^2 = 0.997$: the magnetic field hardly matters. For a 1 GeV proton, $\beta = 0.875$ and the net push is only 23 %; close to the speed of light the magnetic attraction cancels the electric repulsion almost completely. This is the first reason why space charge weakens at high energy, but not the whole story; the next section shows two more.

### Equation of motion inside the beam

Dividing the net push by the particle's "resistance to deflection" gives how much the trajectory bends (worked out step by step in [Going deeper](#space-charge-deep-fields)):

$$
x'' = \frac{K}{a^2}\,x ,
$$

where $x''$ is the rate of change of the trajectory's slope along the path, as acceleration is to position (the [Linear optics](#linear-optics) chapter uses the same quantity); the plus sign means an outward push. $K$ is the **generalized perveance** defined in the next section, which measures how strong space charge is. The force is proportional to $x$ — **the self-field of a uniform distribution is linear**. This is the basis of the KV theory: a linear space-charge force is just a "negative spring" that can be added straight into [Hill's equation](#linear-optics-hill), and the rms emittance is still conserved.

> **Think about it**: if the self-field of a uniform beam is linear, why can space charge make the emittance grow at all?
>
> Answer: because real beams are not uniform. A Gaussian or peaked distribution is dense in the centre and thin at the edge; its self-field is nearly linear in the centre and weakens outward, which is non-linear. Non-linear forces twist the phase-space ellipse into an "S" and the rms emittance grows; see [Emittance growth and halo](#space-charge-emittance-growth).

When the cross-section is elliptical, or the beam comes in bunches, the self-field of a uniform distribution is still linear, only with different strengths in different directions: the narrow direction is pushed harder, and the longer the bunch, the larger the share of the push that goes transversely and the smaller the longitudinal share. Longitudinally it is defocusing too: space charge stretches the bunch and cancels part of the RF bunching ([Longitudinal dynamics](#longitudinal)). The formulas are in [Going deeper: from Coulomb's law to the self-fields of a beam](#space-charge-deep-fields).

## Generalized perveance: why 1/(βγ)³ {#space-charge-perveance}

### Three reasons

The strength of space charge is measured by a dimensionless number, the **generalized perveance**:

$$
K = \frac{q I}{2\pi\varepsilon_0\, m c^3\beta^3\gamma^3} = \frac{2I}{I_0\,\beta^3\gamma^3},\qquad
I_0 = \frac{4\pi\varepsilon_0\, m c^3}{q} .
$$

$I_0$ is the characteristic current: $I_0 = 3.13\times 10^7$ A for protons, $3.13\times 10^7\,A/Z$ A for ions of mass number $A$ and charge state $Z$. $K$ measures the strength of space charge relative to the beam's "stiffness".

The $\beta^3\gamma^3$ in the denominator has three parts, each understandable with school physics:

1. **$1/\gamma^2$: magnetic cancellation**. What is left of the electric repulsion after the magnetic attraction (previous section).
2. **$1/\beta$: slower means more crowded**. For the same current, the slower the particles, the more charge per metre ($\lambda_q = I/\beta c$). Like traffic on a motorway: with the same number of cars passing per minute, the slower they drive, the denser the road.
3. **$1/(\gamma\beta^2)$: faster means harder to push**. To change the particle's direction the force has to overcome its momentum $p = \gamma m\beta c$; and the faster the particle, the less time it takes to fly a metre and the shorter it is pushed ($1/v$). Deflection angle = impulse ÷ momentum = (force × time) ÷ momentum; the two factors together give $1/(\gamma\beta^2)$.

Multiplied together they give $1/(\beta^3\gamma^3)$. At low energy all three factors grow at once, so space charge is strongest in the low-energy section. A 10 mA proton beam at various energies:

| Kinetic energy | $\beta\gamma$ | $K$ (10 mA) | relative to 1.527 MeV |
|---|---|---|---|
| 35 keV (ion-source exit) | 0.00864 | $9.9\times 10^{-4}$ | 290 times |
| 1.527 MeV | 0.0571 | $3.44\times 10^{-6}$ | 1 |
| 10 MeV | 0.146 | $2.04\times 10^{-7}$ | 0.059 |
| 100 MeV | 0.474 | $6.0\times 10^{-9}$ | 0.0017 |
| 1 GeV | 1.81 | $1.08\times 10^{-10}$ | $3.1\times 10^{-5}$ |

From the ion source to 1 GeV, $K$ falls by about seven orders of magnitude. That is why low-energy design is dominated by space charge while at high energy the main concerns are errors and losses. $K$ at the example energy for various currents:

| DC current | $K$ |
|---|---|
| 0.184 mA | $6.32\times 10^{-8}$ |
| 1 mA | $3.44\times 10^{-7}$ |
| 10 mA | $3.44\times 10^{-6}$ |

### Bunches: look at the peak, not the average

The $K$ above is for an uninterrupted "DC beam". An RF-accelerated beam comes in bunches, and the particles are not spread evenly along the line: at 162.5 MHz the bunch spacing is $\beta\lambda = 105$ mm, while the rms bunch length is only 1.3 mm, so the charge per metre inside a bunch is much higher than the average. The space charge of a bunch must therefore be computed from the line charge density at the bunch centre, which amounts to a DC beam with a larger **equivalent current**.

The example bunch ($Q = 1.133$ pC, 7.07 million protons, rms sizes $\sigma_x = 0.954$, $\sigma_y = 0.958$, $\sigma_z = 1.316$ mm) has an equivalent current $I_{\mathrm{eq}} = 3.69$ mA and $K_{\mathrm{eq}} = 1.27\times 10^{-6}$, 20 times what the mean current alone would give. How to compute it: [Going deeper: the equivalent current of a bunch](#space-charge-deep-bunch).

> **Think about it**: the example beam's mean current is only 0.184 mA; why is the equivalent current 20 times larger?
>
> Answer: the particles are packed into bunches. The bunches are 105 mm apart but only 1.3 mm long (rms), so the line charge density inside a bunch is much higher than the average. With the same charge in a Gaussian distribution, the peak current $Q\beta c/(\sqrt{2\pi}\sigma_z) = 5.9$ mA, of the same order as the 3.69 mA of the uniform ellipsoid.

> Note: the space-charge strength of a bunch is set by the **peak line density inside the bunch**, not by the mean current. In the example the equivalent current is 20 times the mean current; when the bunch is compressed longitudinally $I_{\mathrm{eq}}$ grows further. Estimating $K$ from the mean current seriously underestimates space charge.

## A balance of three forces: how fat is the beam {#space-charge-envelope}

### What squeezes and what pushes out

The size of the beam is set by a balance of three effects:

- **external focusing** (quadrupoles, solenoids) squeezes the beam inward;
- **emittance** pushes outward: the random transverse motion of the particles (each with its own angle) makes the beam grow by itself in a drift, like the "thermal pressure" of a gas;
- **space charge** pushes outward: the repulsion between the particles.

In [Linear optics](#linear-optics-twiss-transport) the rms beam size $\sigma$ obeys an **envelope equation** containing only the first two. Now we add the third. Averaging a string of focusing elements into continuous focusing (the smooth approximation, focusing strength $k_0$ in 1/m) and taking a round beam, the rms envelope equation is

$$
\sigma'' + k_0^2\,\sigma - \frac{K}{4\sigma} - \frac{\varepsilon^2}{\sigma^3} = 0 .
$$

$\sigma''$ is the rate of change of the envelope's slope; $k_0^2\sigma$ is focusing, inward; $K/(4\sigma)$ is space charge, outward; $\varepsilon^2/\sigma^3$ is emittance (geometric rms emittance $\varepsilon$), outward. Where the equation comes from: [Going deeper: the envelope equation](#space-charge-deep-envelope).

### Matching: the beam size from a quadratic equation

If the beam size stays constant along the line (called **matched**), $\sigma'' = 0$. Multiplying by $\sigma^3$ gives a quadratic equation in $\sigma^2$:

$$
k_0^2\,(\sigma^2)^2 - \frac{K}{4}\,\sigma^2 - \varepsilon^2 = 0 ,
$$

and the quadratic formula, taking the positive root, gives

$$
\sigma^2 = \frac{K/4 + \sqrt{K^2/16 + 4k_0^2\varepsilon^2}}{2k_0^2} .
$$

At zero current $\sigma_0 = \sqrt{\varepsilon/k_0}$. Take a FODO cell (quadrupoles 0.1 m, 6 T/m, 0.2 m apart, cell 0.6 m, mean focusing $k_0 = 1.534\ \mathrm{m^{-1}}$) and the example emittance $\varepsilon = 2.41$ mm·mrad: the matched rms size is 1.254 mm at zero current, 1.329 mm for a 10 mA DC beam and 1.488 mm for a 30 mA DC beam. Space charge swells the beam until external focusing and "emittance + space charge" balance again.

### Emittance-dominated or space-charge-dominated

Compare the two outward terms $K/(4\sigma)$ and $\varepsilon^2/\sigma^3$: when the ratio $K\sigma^2/(4\varepsilon^2) \ll 1$ the beam is **emittance-dominated**, when $\gg 1$ it is **space-charge-dominated**. The two terms scale differently with size: the emittance term $\propto 1/\sigma^3$, the space-charge term $\propto 1/\sigma$. The more the beam is squeezed, the more the emittance term wins; the fatter the beam, the more space charge wins. With the example emittance, a 10 mA DC beam at $\sigma = 1.33$ mm has a ratio of about 0.26: still emittance-dominated, but space charge already changes the matched size noticeably.

## Tune depression: the particles oscillate more slowly {#space-charge-tune-depression}

### Two springs

In a focusing channel every particle swings back and forth around the axis (see the FODO animation in [Quadrupole](#quadrupole-fodo)). The angle a particle turns through in its swing during one focusing period is the **phase advance per cell**.

In a uniform beam the space-charge force is linear, a "negative spring". Every particle in the beam feels the positive spring of external focusing and the negative spring of space charge at once; the net spring is softer, the particle swings more slowly, and the angle turned per cell is smaller. This is **tune depression**: the phase advance per cell is $\sigma_0$ at zero current and drops to $\sigma < \sigma_0$ with current. (Here $\sigma$ and $\sigma_0$ are phase advances, not rms sizes — a historical notation.)

Keep two things apart:

- the oscillation of **a single particle** gets slower ($\sigma < \sigma_0$);
- the **beam envelope** can still be exactly matched and repeat with the period of the structure, only the matched size is larger.

$\sigma/\sigma_0 = 1$ means space charge is negligible; as $\sigma/\sigma_0 \to 0$ the external focusing is almost entirely cancelled by space charge, the particles cross the beam almost in straight lines as through a uniform "cold plasma", and the beam size is set by the balance of focusing and space charge, almost independent of the emittance.

### How much depression

From the matching condition one finds ([Going deeper: the tune-depression formula](#space-charge-deep-tune) uses only school algebra):

$$
\frac{\sigma}{\sigma_0} = \sqrt{1 + u^2} - u,\qquad
u = \frac{K}{8 k_0\,\varepsilon_{\mathrm{rms}}} = \frac{K}{2 k_0\,\varepsilon_{\mathrm{edge}}} ,
$$

where all $\varepsilon$ are **geometric** (unnormalized) emittances and $\varepsilon_{\mathrm{edge}} = 4\varepsilon_{\mathrm{rms}}$ is the edge emittance of a KV beam. Designs commonly have $\sigma/\sigma_0 \gtrsim 0.4$–0.5. The matched rms size grows as $(\sigma/\sigma_0)^{-1/2}$: depress the phase advance to 0.5 and the beam is 41 % fatter. The $K/\varepsilon$ in $u$ tells us that the relative strength of space charge depends not only on the current but also on the emittance — a beam with small emittance (a "cold" beam) is more easily space-charge-dominated.

### Numerical example

The FODO cell above: quadrupoles $L_q = 0.1$ m, $G = 6$ T/m, drifts $L_d = 0.2$ m, period $L_p = 0.6$ m. AVAS's thick-lens matrices give $\sigma_0 = 52.7°$, $k_0 = 1.534\ \mathrm{m^{-1}}$ and a zero-current matched rms size of 1.254 mm.

| Beam | $K$ | $u$ | $\sigma/\sigma_0$ | $\sigma$ | matched rms size |
|---|---|---|---|---|---|
| example bunch ($I_{\mathrm{eq}} = 3.69\ \mathrm{mA}$) | $1.27\times 10^{-6}$ | 0.043 | 0.958 | 50.5° | 1.281 mm |
| 10 mA DC | $3.44\times 10^{-6}$ | 0.116 | 0.891 | 47.0° | 1.329 mm |
| 30 mA DC | $1.03\times 10^{-5}$ | 0.348 | 0.711 | 37.5° | 1.488 mm |

The same cell at $G = 4$ T/m ($\sigma_0 = 34.5°$) depresses the phase advance of a 30 mA DC beam to 0.60: the weaker the focusing, the larger the relative effect of space charge.

In the lab below drag the current `I`: watch $\sigma/\sigma_0$ fall with current (0.891 at 10 mA, depressed phase advance 47.0°) and the rms envelope with space charge (solid, largest about 1.79 mm) grow fatter than at zero current (dashed, largest about 1.60 mm). Then lower `G` from 6 to 4: at the same 10 mA, $\sigma/\sigma_0$ drops to 0.838. The lab's envelopes come from the uniform-beam kicks of the linear preview without rematching the incoming beam, so with space charge the envelope shows a slight mismatch beat.

```avas-lab
space-charge I=10 W=1.527 mass=938.272 q=1 emit=0.13768 G=6 Lq=0.1 Ld=0.2 cells=10
```

You can also watch the particles slow down directly. Below is the particle animation of 6 FODO cells with the current set to 10 mA. Look at the values: "Phase advance per cell μx" is the cell's zero-current value (about 75.6°), while "Oscillations of a particle in x along the channel" includes space charge: about 1.04. Drag the current back to the example's 0.184 mA and it returns to about 1.25: over the same 6 cells the particles swing about 0.2 of an oscillation less. The incoming beam is matched for zero current, so at 10 mA the beam also "breathes" (largest rms size about 4.0 mm, against about 1.8 mm at 0.184 mA).

```avas-motion
fodo I=10
```

> **Think about it**: a depressed phase advance means the particles feel weaker focusing. Can stronger external focusing "cure" tune depression?
>
> Answer: it helps, at a price. $u \propto K/k_0$: with stronger external focusing ($k_0$ larger) the beam gets narrower and the space-charge force larger, but the external focusing grows more, so $\sigma/\sigma_0$ moves closer to 1 — raising $G$ from 4 to 6 above improves the depression of the 30 mA beam from 0.60 to 0.71. The price is a larger $\sigma_0$, and $\sigma_0$ above 90° causes envelope instability ([Linear optics](#linear-optics-periodic)); at low energy quadrupole gradients and apertures set limits too. Design is a balance between these constraints; see [Lattice design](#lattice-design).

> Note: the smooth approximation ignores the envelope's ripple within a cell, and its error grows with $\sigma_0$ (it is usable for roughly $\sigma_0 \lesssim 60°$–$80°$); exact matching requires solving the periodic envelope equations (see [Matching](#matching)). The equivalent $K$ of a bunch also changes with bunch length; the first row of the table is only an estimate at the entrance.

## Emittance growth and halo {#space-charge-emittance-growth}

In linear theory the rms emittance is conserved. Real beam distributions are not uniform, their self-fields contain non-linear parts, and the emittance grows. The main mechanisms (qualitatively; the frequencies involved are in [Going deeper: plasma oscillations and halo](#space-charge-deep-halo)):

- **Charge redistribution**: of all beams with the same rms size, the uniform one has the lowest self-field energy. An rms-matched but non-uniform beam (Gaussian, peaked) relaxes towards a more uniform distribution within about a quarter of a plasma period, and the surplus field energy ("free energy") turns into random transverse motion of the particles, i.e. emittance growth. Like hot and cold water poured into a cup, which mix by themselves quickly and irreversibly. The stronger the space charge, the faster this happens and the larger the growth.
- **Mismatch and halo**: the envelope of a mismatched beam swells and shrinks periodically ("breathes"). A particle moving in and out of the core feels a space-charge force that swells and shrinks with the envelope; when its oscillation frequency is about half that of the envelope, like squatting and standing twice per swing on a swing, it gets pushed further out again and again — this is the 2:1 parametric resonance, and the particles pushed out form the **halo**. Where the analogy ends: on a swing the frequency hardly changes with amplitude at small amplitudes, but the further a halo particle is from the core, the weaker the space-charge force and the more its frequency changes, so the resonance detunes itself and the halo has a finite extent (which can still be much larger than the rms size). Halo particles are few and hardly change the rms size, but they decide the beam losses.
- **Envelope instability and resonances**: with a zero-current phase advance $\sigma_0 > 90°$, space charge can drive envelope instabilities (hence the common design rule $\sigma_0 < 90°$); other parametric resonances of the periodic structure also cause growth.
- **Transverse–longitudinal coupling**: when the transverse and longitudinal "temperatures" (emittance over focusing) differ, space charge exchanges energy between them (the equipartitioning problem); Hofmann's stability charts show the dangerous parameter regions.

In AVAS output, compare the rms sizes in DataSet.txt (columns 16/18/20) with the maximum sizes (columns 22/24/26): a steadily growing ratio of maximum to rms usually signals developing halo. Emittance growth and halo require the multi-particle engine; neither the linear preview nor the primer's animations contain these effects.

## Think about it {#space-charge-think}

> **Think about it 1**: accelerate a 10 mA proton beam from 1.527 MeV to 10 MeV. How much weaker is space charge?
>
> Answer: $K \propto 1/(\beta\gamma)^3$; $\beta\gamma$ grows from 0.0571 to 0.146, so $K$ becomes 0.059 of its value, about 17 times weaker. The magnetic cancellation ($1/\gamma^2$) contributes only 1.8 % of this; almost all of it comes from the velocity, $\beta^3$.

> **Think about it 2**: in the opening animation, at 20 mA the waist moved from 0.19 m forward to 0.06 m and got fatter. Why does space charge move the waist forward?
>
> Answer: at the entrance the beam is converging, and space charge pushes outward, cancelling part of the converging angles, so the beam stops "closing" earlier; the narrower it gets, the stronger the push (the push goes as $1/\sigma$), so the waist cannot become as narrow as at zero current.

> **Think about it 3**: at the same 10 mA, which has stronger space charge: a DC beam or a 162.5 MHz bunched beam with an rms bunch length of 1.3 mm?
>
> Answer: the bunched beam. At equal mean current the peak line density inside a bunch is much higher than the average (20 times in the example), and space charge goes by the peak.

> **Think about it 4**: why can electron linacs often ignore space charge?
>
> Answer: electrons are light and approach the speed of light at a few MeV; $\beta\gamma$ is large and $K \propto 1/(\beta\gamma)^3$ very small, and the magnetic attraction almost completely cancels the electric repulsion ($1/\gamma^2$). Only in the low-energy region near the electron gun does space charge matter.

## Common misconceptions {#space-charge-misconceptions}

- **Only the mean current counts**: the space charge of a bunch is set by the peak line density inside the bunch. In the example the equivalent current is 20 times the mean current, and more after longitudinal compression.
- **Space charge weakens only as $1/\gamma^2$**: $1/\gamma^2$ is only the electromagnetic cancellation; what enters the trajectory equation is $K \propto 1/(\beta^3\gamma^3)$, and for non-relativistic protons with $\gamma \approx 1$ the weakening with energy comes mostly from $\beta^3$.
- **Mixing rms and edge quantities**: the KV equations use edge sizes and edge emittances, the rms equations rms quantities; the conversion is ×2 for sizes and ×4 for emittances. The two forms of $u$ use $\varepsilon_{\mathrm{rms}}$ and $\varepsilon_{\mathrm{edge}}$ respectively; mixing them up is a factor 4.
- **Mixing normalized and geometric emittances**: the envelope equation and $u$ use the geometric emittance $\varepsilon = \varepsilon_n/(\beta\gamma)$; the example's normalized 0.138 corresponds to a geometric 2.41 mm·mrad, a factor 17.5.
- **Two meanings of $\sigma$**: in the tune-depression formula $\sigma$ and $\sigma_0$ are phase advances per cell, not rms sizes; elsewhere in this chapter $\sigma_x$ is an rms size.
- **Tune depression means focusing fails**: what is depressed is the oscillation frequency of single particles; the envelope of a matched beam still repeats with the period of the structure, only fatter.
- **Space charge always makes the emittance grow**: linear (uniform-distribution) space charge does not change the rms emittance; growth comes from the non-linear part, mismatch and resonances.
- **Space-charge compensation works for every beam**: in low-energy DC beams, electrons produced by ionizing the residual gas are trapped in the beam potential and partly neutralize the space charge (see [Ion source and LEBT](#journey-source)); the potential of RF bunches changes violently in time and such compensation is usually weak. AVAS's `spacechargecomp` only multiplies the space charge by a constant factor $(1 - V)$; you have to supply the degree of compensation $V$ yourself (see [In AVAS](#space-charge-in-avas)).
- **`boundary` is the space-charge boundary condition**: no, it is the beam-loss boundary; see below.

## Going deeper: from Coulomb's law to the self-fields of a beam {#space-charge-deep-fields}

The main text gave the field of a uniform beam via the analogy with gravity inside the Earth and said that the magnetic field reduces the net push to $1/\gamma^2$. This section derives both step by step and then the equation of motion $x'' = Kx/a^2$. The first steps use only field lines, magnetic fields and impulse from school; the last paragraph is for readers who know vector calculus.

### Step 1: counting field lines

At school the electric field is drawn with field lines: they start on positive charges, and the denser they are, the stronger the field. A point charge $q$ sends out a number of field lines proportional to $q$, and they cross a sphere of radius $r$ (area $4\pi r^2$) evenly, so the field strength goes as $q/r^2$ — Coulomb's law. Stated exactly: **the number of field lines crossing any closed surface (field strength × area) = enclosed charge ÷ $\varepsilon_0$**. This is **Gauss's law**, and it says the same as Coulomb's law.

### Step 2: the field of a cylindrical beam

For a long, uniformly charged cylindrical beam take a coaxial cylindrical surface of radius $r$ and length $\ell$. By symmetry the field points radially outward and crosses only the side (area $2\pi r\ell$).

- Surface outside the beam ($r > a$): enclosed charge $\lambda_q\ell$, $E_r\cdot 2\pi r\ell = \lambda_q\ell/\varepsilon_0$, $E_r = \lambda_q/(2\pi\varepsilon_0 r) = 2k\lambda_q/r$.
- Surface inside the beam ($r \le a$): only the charge $\lambda_q\ell\,r^2/a^2$ is enclosed,

$$
E_r = \frac{\lambda_q}{2\pi\varepsilon_0}\frac{r}{a^2}.
$$

This is the exact form of "only the charge within radius $r$ counts": the field lines of the charge outside all cross the surface on both sides and contribute nothing net.

### Step 3: the magnetic field

The current $I$ flows along the axis. The school field of a straight wire is $B = \mu_0 I/(2\pi r)$ outside; inside the beam only the current $Ir^2/a^2$ within radius $r$ counts:

$$
B_\theta = \frac{\mu_0 I}{2\pi}\frac{r}{a^2} = \frac{\beta}{c}\,E_r .
$$

(using $I = \lambda_q\beta c$ and $\mu_0\varepsilon_0 = 1/c^2$.) The field circles the axis.

### Step 4: the net force

A positive charge moving forward in this magnetic field feels a Lorentz force $qvB_\theta$ pointing to the axis (parallel currents attract), opposite to the electric force:

$$
F_r = q\left(E_r - \beta c B_\theta\right) = q E_r\left(1 - \beta^2\right) = \frac{q E_r}{\gamma^2} .
$$

### Step 5: from force to trajectory

A particle with speed $v$ covers $\Delta s$ in a time $\Delta s/v$ and gains transverse momentum $F_x\,\Delta s/v$. The angle $x'$ = transverse momentum ÷ longitudinal momentum $p = \gamma m v$, so the change of angle per metre is

$$
x'' = \frac{F_x}{p\,v} = \frac{F_x}{\gamma m\beta^2 c^2}.
$$

Inserting step 4 ($F_x = F_r\,x/r$ in the $x$ direction):

$$
x'' = \frac{q}{\gamma m\beta^2c^2}\cdot\frac{1}{\gamma^2}\cdot\frac{I}{2\pi\varepsilon_0\beta c}\cdot\frac{x}{a^2}
= \frac{qI}{2\pi\varepsilon_0\,mc^3\beta^3\gamma^3}\cdot\frac{x}{a^2} = \frac{K}{a^2}\,x .
$$

The three factors each find their place: $1/\gamma^2$ from step 4, $1/\beta$ from $\lambda_q = I/\beta c$, $1/(\gamma\beta^2)$ from the $pv$ of this step. Example, 10 mA: $K = 3.44\times 10^{-6}$.

### Step 6: an elliptical cross-section

Inside a uniform beam of elliptical cross-section with half-axes $a$ and $b$ (the result can be checked along the same lines and reduces to the round beam for $a = b$):

$$
E_x = \frac{\lambda_q}{\pi\varepsilon_0}\frac{x}{a(a + b)},\qquad
E_y = \frac{\lambda_q}{\pi\varepsilon_0}\frac{y}{b(a + b)},
\qquad
x'' = \frac{2K}{a(a + b)}\,x,\quad y'' = \frac{2K}{b(a + b)}\,y .
$$

The narrow direction has the smaller denominator and is defocused more strongly.

### Step 7: bunches — the uniform ellipsoid

An RF beam is a train of bunches, each with charge $Q = I/f$ ($I$ the mean current). Treat a bunch as a uniform ellipsoid with half-axes $a$, $b$, $c_z$ in the laboratory. In the frame moving with the bunch (the rest frame) relativity stretches it longitudinally by $\gamma$, the longitudinal half-axis is $\gamma c_z$ and the charge density $\rho' = 3Q/(4\pi a b\,\gamma c_z)$. Inside a uniform ellipsoid the field is linear:

$$
E'_x = \frac{\rho'}{\varepsilon_0}M_x\,x,\quad E'_y = \frac{\rho'}{\varepsilon_0}M_y\,y,\quad E'_z = \frac{\rho'}{\varepsilon_0}M_z\,\gamma z,\qquad M_x + M_y + M_z = 1,
$$

where $\gamma z$ is the longitudinal coordinate in the rest frame and $M_i$ are the **depolarization factors** (all $1/3$ for a sphere). That the $M_i$ add up to 1 means the total "share" of defocusing is fixed: the longer the bunch, the smaller the longitudinal share and the larger the transverse one. For a spheroid with rest-frame aspect ratio $p = \gamma c_z/\sqrt{ab}$, for $p > 1$

$$
M_z = \frac{1 - e^2}{e^3}\left(\operatorname{artanh} e - e\right),\qquad e = \sqrt{1 - 1/p^2},
$$

with the common approximation $M_z \approx 1/(3p)$ ($p$ from about 1 to a few) and transversely $M_x \approx (1 - M_z)\,b/(a + b)$. Transforming back to the laboratory (the transverse force picks up another $1/\gamma$, the same thing as the magnetic cancellation of step 4) gives

$$
x'' = \frac{K_3 M_x}{a b c_z}\,x,\qquad
z'' = \frac{K_3 M_z}{a b c_z}\,z,\qquad
K_3 = \frac{3 q Q}{4\pi\varepsilon_0\, mc^2\,\beta^2\gamma^3},
$$

with $z' = \dd z/\dd s = \delta/\gamma^2$. Space charge is defocusing longitudinally as well.

### For readers who know vector calculus

Step 1 is the integral form of $\nabla\cdot\boldsymbol E = \rho/\varepsilon_0$, step 3 is Ampère's law $\oint\boldsymbol B\cdot\dd\boldsymbol l = \mu_0 I_{\mathrm{enc}}$. Alternatively compute only the electrostatic field $\boldsymbol E'$ in the beam rest frame and Lorentz-transform back: $E_\perp = \gamma E'_\perp$, $\boldsymbol B = \boldsymbol\beta\times\boldsymbol E/c$, transverse force $q(E_\perp - \beta cB) = qE'_\perp/\gamma$, which is where the $1/\gamma^2$ (relative to the laboratory $E_\perp$) comes from. That the potential inside a uniform ellipsoid is quadratic is the classical ellipsoid result (Newton–Ivory theorem); the depolarization factors are elliptic integrals.

## Going deeper: the equivalent current of a bunch {#space-charge-deep-bunch}

The main text says the example bunch acts like a 3.69 mA DC beam. This section works it out step by step.

### Step 1: rms sizes and the half-axes of the uniform ellipsoid

AVAS and experiments give rms sizes $\sigma$, while the formulas of the previous section use the half-axes of a uniform ellipsoid. In a uniform solid sphere of radius $a$, $\langle x^2\rangle = a^2/5$ (the classical result of integrating over a ball; for comparison, a uniform disc gives $a^2/4$). So a bunch with rms size $\sigma$ has an equivalent uniform ellipsoid with half-axes $\sqrt5\,\sigma$. Example: $\sqrt5\times(0.954, 0.958, 1.316)$ mm = (2.13, 2.14, 2.94) mm.

### Step 2: the depolarization factor

Rest-frame aspect ratio $p = \gamma c_z/\sqrt{ab} = 1.0016\times 2.94/\sqrt{2.13\times 2.14} = 1.379$; the formula of the previous section gives $M_z = 0.252$ (the approximation $1/(3p) = 0.242$).

### Step 3: comparison with a DC beam

Comparing the transverse equation of the uniform ellipsoid, $x'' = K_3 M_x x/(a b c_z)$, with that of a round DC beam, $x'' = K x/a^2$ (taking $a = b$, $M_x = (1 - M_z)/2$), the central slice of the bunch acts like a DC beam with **equivalent current** and perveance

$$
I_{\mathrm{eq}} = \frac{3}{4}\,\frac{Q\,\beta c}{c_z}\,(1 - M_z),\qquad
K_{\mathrm{eq}} = \frac{2 I_{\mathrm{eq}}}{I_0\beta^3\gamma^3} .
$$

$\tfrac34 Q/c_z$ is the line charge density of the central slice of the uniform ellipsoid (total charge divided by an "equivalent length" $\tfrac43 c_z$); multiplying by $\beta c$ turns it into a current, and $(1 - M_z)$ is the transverse share of the push.

### Step 4: the numbers

$Q = 1.133$ pC, $\beta c = 1.708\times 10^7$ m/s, $c_z = 2.94$ mm: $\tfrac34\times 1.133\times 10^{-12}\times 1.708\times 10^7/2.94\times 10^{-3}\times(1 - 0.252) = 3.69$ mA, $K_{\mathrm{eq}} = 2\times 3.69\times 10^{-3}/(3.13\times 10^7\times 1.860\times 10^{-4}) = 1.27\times 10^{-6}$. That is 20 times the mean current of 0.184 mA.

### Step 5: comparison with a Gaussian bunch

If the bunch is Gaussian longitudinally (rms length $\sigma_z$), the line charge density at its centre is $Q/(\sqrt{2\pi}\sigma_z)$ and the peak current $Q\beta c/(\sqrt{2\pi}\sigma_z) = 1.133\times 10^{-12}\times 1.708\times 10^7/(2.507\times 1.316\times 10^{-3}) = 5.9$ mA. Of the same order as the 3.69 mA of the uniform ellipsoid (which also deducts the longitudinal share $M_z$).

### For readers who know integration

$\langle x^2\rangle = \frac{3}{4\pi a^3}\int_{r<a} x^2\,\dd V = \frac{1}{3}\cdot\frac{3}{4\pi a^3}\int_0^a r^2\cdot 4\pi r^2\,\dd r = a^2/5$. The line density of the central slice of a uniform ellipsoid is $\lambda(0) = \frac{3Q}{4\pi abc_z}\cdot\pi ab = \frac{3Q}{4c_z}$. Sacherer showed that for ellipsoidally symmetric bunches the space-charge term of the rms envelope equation depends only weakly on the shape of the distribution, which justifies computing with the rms-equivalent uniform ellipsoid.

## Going deeper: the envelope equation {#space-charge-deep-envelope}

The main text simply wrote down the envelope equation $\sigma'' + k_0^2\sigma - K/(4\sigma) - \varepsilon^2/\sigma^3 = 0$. This section shows where each term comes from, then gives the KV and rms equations of the textbooks. Tools: the waist formula of the [Drift](#drift-waist) chapter, lenses, derivatives from school.

### Step 1: the emittance term — the "bend" of a drift at the waist

The [Drift](#drift-waist) chapter found that a distance $d$ from the waist $\sigma(d)^2 = \sigma_w^2 + (\varepsilon/\sigma_w)^2 d^2$ (rms angle at the waist $\sigma' = \varepsilon/\sigma_w$). There is no force at all, yet the envelope is a curved line (a hyperbola). Take its second derivative at the waist: with $\sigma = \sqrt{\sigma_w^2 + \varepsilon^2d^2/\sigma_w^2}$, differentiate twice with respect to $d$ and set $d = 0$:

$$
\sigma''\big|_{\text{waist}} = \frac{\varepsilon^2}{\sigma_w^3}.
$$

This is the $\varepsilon^2/\sigma^3$ of the envelope equation: the spread of particle angles makes the envelope bend outward by itself, the more so the narrower the beam. Think of it as the "thermal pressure" of the emittance.

### Step 2: the focusing term

External focusing acts on every particle as $x'' = -k_0^2x$. The envelope, the "typical distance" of many particles, is bent inward the same way: $-k_0^2\sigma$.

### Step 3: the space-charge term

A uniform round beam has $x'' = Kx/a^2$, and the edge of a KV beam is at $a = 2\sigma$ (a two-dimensional uniform disc has $\langle x^2\rangle = a^2/4$); for the rms envelope it is therefore a defocusing lens of strength $K/(4\sigma^2)$, which acting on $\sigma$ gives $+K/(4\sigma)$.

Together the three terms make the envelope equation of the main text.

### Step 4: the KV envelope equations

The KV (Kapchinsky–Vladimirsky) distribution is uniform on a hyper-ellipsoid surface in four-dimensional phase space (the various distributions are in [Beam distributions](#distributions)); its projection on the $x$–$y$ plane is a uniform ellipse, its self-field is exactly linear, and it stays a KV distribution in linear external fields. The beam's edge half-axes $a(s)$, $b(s)$ obey

$$
a'' + k_x(s)\,a - \frac{2K}{a + b} - \frac{\varepsilon_x^2}{a^3} = 0,\qquad
b'' + k_y(s)\,b - \frac{2K}{a + b} - \frac{\varepsilon_y^2}{b^3} = 0 ,
$$

where $k_{x,y}(s)$ is the external focusing strength (for a quadrupole $k = \pm G/B\rho$, see [Quadrupole](#quadrupole)) and $\varepsilon$ the **edge** (total) geometric emittance. The three terms stand for external focusing, space-charge defocusing and emittance (the elliptical-cross-section force $2K/(a(a+b))$ of step 6 of [Going deeper: from Coulomb's law to the self-fields of a beam](#space-charge-deep-fields), multiplied by $a$, is $2K/(a+b)$).

### Step 5: Sacherer's rms envelope equations

Sacherer (1971) proved that for any distribution with elliptical symmetry the rms size $\sigma = \sqrt{\langle x^2\rangle}$ obeys an equation of the same form; for two-dimensional continuous beams the space-charge term does not depend on the shape of the distribution, and for three-dimensional ellipsoidally symmetric bunches only weakly:

$$
\sigma_x'' + k_x\,\sigma_x - \frac{K}{2(\sigma_x + \sigma_y)} - \frac{\varepsilon_{x}^2}{\sigma_x^3} = 0 ,
$$

with $\varepsilon_x$ the rms geometric emittance. Inserting $a = 2\sigma_x$ and $\varepsilon_{\mathrm{edge}} = 4\varepsilon_{\mathrm{rms}}$ into the KV equation gives exactly this (check: $a'' = 2\sigma''$, $2K/(a+b) = K/(\sigma_x + \sigma_y)$, $\varepsilon_{\mathrm{edge}}^2/a^3 = 16\varepsilon^2/(8\sigma^3) = 2\varepsilon^2/\sigma^3$; divide the whole equation by 2). Any distribution can therefore be described by its "rms-equivalent KV beam": **edge size = 2 × rms size, edge emittance = 4 × rms emittance**. The rms envelope depends only on rms quantities; the details of the distribution do not enter the equation (but they do enter how the emittance grows). For a round beam, $\sigma_x = \sigma_y$, the space-charge term is $K/(4\sigma)$.

### Step 6: the 3D rms envelope equations of a bunch

For a uniform ellipsoid the rms-equivalent half-axis is $\sqrt5\,\sigma$ ([previous section](#space-charge-deep-bunch)). Inserting the linear forces of the uniform ellipsoid:

$$
\sigma_x'' + k_x\sigma_x - \frac{\varepsilon_x^2}{\sigma_x^3} - \frac{K_3 M_x}{5\sqrt5\,\sigma_y\sigma_z} = 0,\qquad
\sigma_z'' + k_z\sigma_z - \frac{\varepsilon_z^2}{\sigma_z^3} - \frac{K_3 M_z}{5\sqrt5\,\sigma_x\sigma_y} = 0 ,
$$

$y$ like $x$; $k_z$ is the longitudinal RF focusing (the $k_{l0}^2$ of [Longitudinal dynamics](#longitudinal-synchrotron)) and $\varepsilon_z$ the geometric rms emittance in the $(z, \dd z/\dd s)$ plane. The three directions are coupled through the $M_i$ and the sizes: squeeze the bunch transversely and its longitudinal space charge grows too.

### For readers who know calculus

For any distribution, from $x'' = -k x + F_x/(pv)$ take derivatives of the second moments: $\dd\langle x^2\rangle/\dd s = 2\langle xx'\rangle$, $\dd\langle xx'\rangle/\dd s = \langle x'^2\rangle + \langle xx''\rangle$. With $\sigma = \sqrt{\langle x^2\rangle}$ and eliminating $\langle xx'\rangle$, $\sigma'' = \varepsilon^2/\sigma^3 + \langle xx''\rangle/\sigma$, where $\varepsilon^2 = \langle x^2\rangle\langle x'^2\rangle - \langle xx'\rangle^2$. For elliptically symmetric distributions the space-charge term $\langle xF_x\rangle$ depends only on the total charge and the rms sizes (Sacherer); this is the rms equation. Whether $\varepsilon$ itself is conserved depends on whether the force is linear.

## Going deeper: the tune-depression formula {#space-charge-deep-tune}

The main text gave $\sigma/\sigma_0 = \sqrt{1 + u^2} - u$. This section derives it with school algebra. To avoid confusion, the rms size is written $R$ in this section; the phase advance remains $\sigma$.

### Step 1: the smooth approximation

When the zero-current phase advance $\sigma_0$ per focusing period $L_p$ is not too large (roughly $\sigma_0 \lesssim 60°$–$80°$), the periodic focusing can be averaged into continuous focusing with wave number $k_0 = \sigma_0/L_p$ (angle per metre). The rms envelope equation of a round DC beam is

$$
R'' + k_0^2\,R - \frac{K}{4R} - \frac{\varepsilon^2}{R^3} = 0 .
$$

### Step 2: the wave number of a single particle in the beam

Inside a matched beam every particle feels external focusing $-k_0^2x$ and linear space charge $+Kx/(4R^2)$ (the defocusing lens of step 3 of the previous section), a net spring

$$
k^2 = k_0^2 - \frac{K}{4R^2}.
$$

The particle oscillates with wave number $k$ and phase advance per cell $\sigma = kL_p$, so $\sigma/\sigma_0 = k/k_0$.

### Step 3: a neat relation

Multiply the matching condition $R'' = 0$ by $R^3$: $k_0^2R^4 - KR^2/4 = \varepsilon^2$. Factor $R^4$ out of the left side: $R^4\left(k_0^2 - K/(4R^2)\right) = \varepsilon^2$, i.e. $k^2R^4 = \varepsilon^2$:

$$
k = \frac{\varepsilon}{R^2}.
$$

Likewise at zero current $k_0 = \varepsilon/R_0^2$. So $R/R_0 = (k_0/k)^{1/2} = (\sigma/\sigma_0)^{-1/2}$: depress the phase advance to 0.5 and the size grows by $\sqrt2 = 1.41$.

### Step 4: solving for σ/σ₀

From the quadratic formula of the main text, $R^2 = \left(K/4 + \sqrt{K^2/16 + 4k_0^2\varepsilon^2}\right)/(2k_0^2)$:

$$
\frac{k}{k_0} = \frac{\varepsilon}{k_0R^2} = \frac{2k_0\varepsilon}{K/4 + \sqrt{K^2/16 + 4k_0^2\varepsilon^2}} .
$$

Divide numerator and denominator by $2k_0\varepsilon$ and write $u = K/(8k_0\varepsilon)$:

$$
\frac{\sigma}{\sigma_0} = \frac{1}{u + \sqrt{u^2 + 1}} = \sqrt{1 + u^2} - u
$$

(for the last step multiply numerator and denominator by $\sqrt{u^2 + 1} - u$; the denominator becomes $(u^2 + 1) - u^2 = 1$).

### Step 5: check

10 mA DC beam, the FODO cell of the main text: $u = 3.437\times 10^{-6}/(8\times 1.534\times 2.412\times 10^{-6}) = 0.116$, $\sqrt{1 + 0.116^2} - 0.116 = 0.891$, $\sigma = 0.891\times 52.7° = 47.0°$; $R = 1.254\times 0.891^{-1/2} = 1.329$ mm. As in the table of the main text and the lab.

### For graduate students

In the smooth approximation $u$ is also often written $u = K/(2k_0\varepsilon_{\mathrm{edge}})$. In the exact periodic case one solves the KV envelope equations numerically for the periodic solution and obtains $\sigma$ from the depressed single-particle transfer matrix; near $\sigma_0 = 90°$ the KV envelope has a parametric instability band (see [Linear optics](#linear-optics-periodic) and the envelope-map analysis of AVAS's dynamics module).

## Going deeper: plasma oscillations and halo {#space-charge-deep-halo}

The main text described emittance growth and halo qualitatively. This section gives the frequencies that appear there and where they come from.

### Step 1: plasma oscillations

Imagine a "cold" uniform beam with neither external focusing nor emittance. If the particles somewhere move outward a little as a whole, the space-charge field inside a uniform beam is proportional to the distance from the axis, and the displaced layer of charge feels a restoring force and oscillates back and forth like a spring. For a uniform round beam this "plasma wave number" is

$$
k_p^2 = \frac{2K}{a^2} = \frac{K}{2\sigma^2}.
$$

In the 10 mA example ($\sigma = 1.33$ mm), $k_p = 0.99\ \mathrm{m^{-1}}$ and the plasma wavelength is about 6.4 m. A non-uniform beam relaxes towards a uniform distribution in about a quarter of a plasma period: the stronger the space charge (the larger $K$), the faster the relaxation.

### Step 2: breathing and quadrupole modes of the envelope

The envelope of a mismatched beam oscillates about the matched size. For a round beam in the smooth approximation, the **breathing mode**, in which both planes swell and shrink in phase, has wave number $k_{\mathrm b} = \sqrt{2k_0^2 + 2k^2}$, and the **quadrupole mode**, in which they move in antiphase (one swells while the other shrinks), $k_{\mathrm q} = \sqrt{k_0^2 + 3k^2}$, where $k$ is the depressed wave number of the previous section. At zero current $k = k_0$ and both are $2k_0$: the envelope oscillates at twice the frequency of a single particle (because the size only cares how far a particle is from the axis, not on which side).

### Step 3: the 2:1 parametric resonance

A halo particle moving in and out of the core oscillates at a frequency close to the single-particle $k$; the envelope swells and shrinks at about $2k$. For every swing of the particle the space-charge force it feels swells and shrinks twice — exactly the rhythm of "squatting and standing twice per swing" on a swing: each time it gains a bit of energy at the right moment, and its amplitude keeps growing. Further from the core the space-charge force is weaker and the particle's frequency changes, the resonance detunes, and so the halo has a finite outer edge.

### Step 4: equipartitioning

When the ratio of transverse to longitudinal "temperatures", $T_z/T_x = \varepsilon_z k_z/(\varepsilon_x k_x)$, is not 1, non-linear space-charge forces move energy between the two. Hofmann's stability charts, drawn in the depressed tune ratio and the temperature ratio, mark the resonance regions that drive the exchange; designs keep the working point out of them or keep the beam close to equipartition.

### For graduate students

The linear stability analysis of the KV beam (Gluckstern modes) gives the instability bands of the envelope and of higher-order modes; the particle–core model treats a halo particle as a single particle moving in the field of a pulsating uniform core and gives the maximum amplitude of the 2:1 resonance quantitatively. AVAS's linear preview contains only the linear part of the rms envelope; all these effects have to be studied with the multi-particle engine.

## In AVAS {#space-charge-in-avas}

### Keywords

Space charge is controlled in `input.txt`, the beam charge given in `beam.txt`:

```avas-params
input:spacecharge input:scmethod input:numofgrid input:meshrms input:steppercycle beam:current beam:frequency beam:beamtype beam:particlenumber spacechargecomp
```

- `spacecharge 1` switches space charge on, `0` off.
- `current` (mA, mean current) and `frequency` (Hz, beam frequency) set the bunch charge $Q = I/f$; each macro-particle carries $Q$ divided by the number of macro-particles `particlenumber`. `beamtype dc` makes a DC beam.
- `scmethod` selects the algorithm: `FFT`, `PICNIC` or `SPICNIC`. The manual only lists the names without describing the implementations; the example project uses `FFT`. When switching to another algorithm, compare with `FFT` on the same problem.
- `numofgrid Nx Ny Nz` is the number of grid cells in the three directions; `meshrms V1 V2 V3` sets the grid length $L_x = 2 V_1\sigma_x$ (likewise $y$, $z$), i.e. the grid covers $\pm V_1$ rms sizes. The grid must cover the whole bunch (including its tails) while each cell holds enough macro-particles; see [Multi-particle simulation](#simulation).
- The time step is set by `steppercycle` (steps per RF period); how often the engine solves the space-charge field within these steps is not stated in the manual; the engine is the reference.
- The manual also says: with longitudinal periodic boundaries the space-charge algorithm switches to FFT automatically and the longitudinal grid length is set to the bunch period, so more longitudinal grid points are needed.
- The lattice keyword `spacechargecomp V` (space-charge compensation): from the line's position in the lattice onward, the charge on the grid (i.e. the space charge) is multiplied by $(1 - V)$; $V$ is the degree of compensation (0 none, 0.5 half). Of the original kernels only the Windows AVAS.dll knows the keyword; the Linux libAVAS.so ignores it. AVAS.dll ignores the line for $V \ge 1$ (no scaling, so writing $V = 1$ for full compensation does nothing), amplifies the space charge for $V < 0$, and applies the line only at the entrance of a field-map or drift element (not before a `quad`, `solenoid`, `bend` or `edge`). The new kernel's default mode limits $1 - V$ to $[0, 1]$ ($V \ge 1$ removes the space charge, $V < 0$ acts as $V = 0$) and applies the line wherever it stands, also before a hard-edge element or before the first element; the compatibility mode (compat = dll) reproduces AVAS.dll (verified from the AVAS.dll disassembly and by comparing the new kernel's compatibility mode with AVAS.dll character for character). It is a crude model: a constant factor without the physics of electron trapping, and the degree of compensation has to be estimated separately.
- `spacechargelong` and `spacechargetype` only affect the envelope model (`sim_type env`).

> Note: `boundary` in `input.txt` is the beam **loss** boundary (`boundary.txt`), not a boundary condition of the space-charge solver. The boundary conditions, coordinate system and interpolation of the engine's space-charge solver are not stated in the manual; the engine is the reference.

> **AVAS convention**: the longitudinal angle of `twissz` is $\delta = \Delta p/p$, while the 3D envelope equations above use $\dd z/\dd s = \delta/\gamma^2$; the two differ by $\gamma^2$. At the example energy $\gamma^2 = 1.003$ and the difference is negligible; at high energy mind the conversion.

### Space-charge model of the linear preview

The visual editor's linear preview adds linear space charge when the beam current is above zero (the primer's particle animations use it too):

- Bunches: an rms-equivalent uniform ellipsoid (half-axes $\sqrt5\,\sigma_{x,y,z}$), with $M_z$ from the spheroid formula and the rest-frame aspect ratio $p = \gamma c_z/\sqrt{ab}$, and transversely $M_x = (1 - M_z)\,b/(a + b)$, i.e. the linear $K_3$ force above; it needs `frequency` and `twissz`, and without them leaves out space charge with a notice.
- DC beams (`beamtype dc`): an rms-equivalent uniform elliptical cross-section (half-axes $2\sigma$), i.e. the KV force $2K/(a(a+b))$.
- Every 4 steps a thin-lens kick of the integrated strength is applied (in the middle of the group, second-order splitting), with the force updated from the current rms sizes.
- Linear space charge is centred on the beam centre and does not move the centroid.

It describes only the linear part of the rms envelope, without emittance growth, halo or losses. With strong space charge its rms sizes can differ from the engine's by up to 13 %. The space-charge lab of this chapter uses the same model (DC beam), with the smooth approximation for tune depression and a KV edge emittance of $4\varepsilon$.

## Further reading {#space-charge-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: self-fields, KV distribution, envelope equations, emittance growth.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: multi-particle dynamics with space charge, and halo.
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18, 1105 (1971).
- I. M. Kapchinsky, V. V. Vladimirsky, Proc. Int. Conf. on High Energy Accelerators, CERN, 1959 (KV distribution).
- I. Hofmann, "Stability of anisotropic beams with space charge", *Phys. Rev. E* 57, 4713 (1998).
- In this primer: [Linear optics](#linear-optics), [Drift](#drift-space-charge), [Matching](#matching), [Multi-particle simulation](#simulation).
