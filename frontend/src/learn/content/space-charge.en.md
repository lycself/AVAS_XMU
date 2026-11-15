# Space charge

The Coulomb force between two protons does not depend on their energy. Yet the same 10 mA proton beam is completely dominated by space charge at the ion-source exit (tens of keV), still has to be taken seriously at 1.5 MeV, and is negligible at 1 GeV — the perveance that measures space charge changes by about seven orders of magnitude along the way. Why? What does space charge actually do to a beam? And what does it mean physically when people say "the tune is depressed to 0.5"?

The chapter first computes the self-fields of a uniform beam, identifies the three sources of the factor $1/(\beta^3\gamma^3)$ and introduces the generalized perveance $K$; it then derives the KV and rms envelope equations and uses the smooth approximation for the matched size and the tune depression; finally it discusses emittance growth and halo qualitatively and explains how the AVAS engine and the linear preview treat space charge. You need the envelope equation and phase advance per period of [Linear optics](#linear-optics), the rms emittance of [Phase space and emittance](#phase-space) and the bunches of [Longitudinal dynamics](#longitudinal). In the story line, space charge is the root reason why [the source and low-energy transport](#journey-source) need strong focusing and why, in [From the source to the target](#journey-overview), the low-energy part must "focus hard and bunch gently".

The numerical examples use the beam of the example project: protons, $W = 1.527\ \mathrm{MeV}$ ($\beta = 0.05698$, $\gamma = 1.00163$, $\beta\gamma = 0.05708$, $B\rho = 0.1786\ \mathrm{T\,m}$), average current $I = 0.184\ \mathrm{mA}$, beam frequency $f = 162.5\ \mathrm{MHz}$, transverse normalized rms emittance $0.138\ \pi\,\mathrm{mm\,mrad}$ (geometric rms emittance $\varepsilon = 2.41\ \mathrm{mm\,mrad}$).

## Self-fields: the electric field pushes, the magnetic field pulls {#space-charge-self-fields}

### Physical picture

Every particle of the beam is pushed away by the electric field of the others; this is where space-charge defocusing comes from. But moving charges are also currents, and parallel currents attract, so the beam's own magnetic field pulls the particles back in. The closer the velocity is to the speed of light, the closer the magnetic attraction comes to the electric repulsion; they almost cancel and only the fraction $1 - \beta^2 = 1/\gamma^2$ of the electric force remains. This is the first reason why space charge weakens at high energy, but not the whole story — the next section shows two more.

### Round DC beam

Let charge be distributed uniformly in a cylinder of radius $a$ with line charge density $\lambda_q = I/(\beta c)$. Gauss's and Ampère's laws give inside the beam ($r \le a$)

$$
E_r = \frac{\lambda_q}{2\pi\varepsilon_0}\frac{r}{a^2},\qquad B_\theta = \frac{\beta}{c}E_r .
$$

The magnetic force on the moving charges opposes the electric force:

$$
F_r = q\left(E_r - \beta c B_\theta\right) = q E_r\left(1 - \beta^2\right) = \frac{q E_r}{\gamma^2} .
$$

With $x'' = F_x/(\gamma m\beta^2c^2)$ the equation of motion of a particle inside the beam is

$$
x'' = \frac{K}{a^2}\,x ,
$$

with $K$ the generalized perveance defined in the next section. The force is proportional to $x$ — **the self-field of a uniform distribution is linear**. This is the basis of KV theory: a linear space-charge force is just a "negative spring" that joins [Hill's equation](#linear-optics-hill) directly, and the rms emittance is still conserved.

> **Think about it**: if the self-field of a uniform beam is linear, why does space charge make the emittance grow at all?
>
> Answer: because real beams are not uniform. A Gaussian or peaked distribution has a dense centre and thin edges; its self-field is nearly linear in the centre and weakens outwards — it is non-linear. Non-linear forces twist the phase-space ellipse into an "S" and the rms emittance grows, see [Emittance growth and halo](#space-charge-emittance-growth).

### Elliptical cross-section

Inside a uniform beam of elliptical cross-section with semi-axes $a$, $b$:

$$
E_x = \frac{\lambda_q}{\pi\varepsilon_0}\frac{x}{a(a + b)},\qquad
E_y = \frac{\lambda_q}{\pi\varepsilon_0}\frac{y}{b(a + b)},
\qquad
x'' = \frac{2K}{a(a + b)}\,x,\quad y'' = \frac{2K}{b(a + b)}\,y .
$$

For $a = b$ this is the round-beam result. The narrow direction is defocused more strongly.

### Bunches: the uniform ellipsoid

An RF-accelerated beam is a train of bunches, each with charge $Q = I/f$ ($I$ the average current). Treat a bunch as a uniform ellipsoid with laboratory semi-axes $a$, $b$, $c_z$; in the beam rest frame the longitudinal semi-axis is $\gamma c_z$ and the charge density $\rho' = 3Q/(4\pi a b\,\gamma c_z)$. The field inside a uniform ellipsoid is linear:

$$
E'_x = \frac{\rho'}{\varepsilon_0}M_x\,x,\quad E'_y = \frac{\rho'}{\varepsilon_0}M_y\,y,\quad E'_z = \frac{\rho'}{\varepsilon_0}M_z\,\gamma z,\qquad M_x + M_y + M_z = 1,
$$

with $\gamma z$ the rest-frame longitudinal coordinate and $M_i$ the depolarization factors (all $1/3$ for a sphere). That the $M_i$ add up to 1 means the total "share" of defocusing is fixed: the longer the bunch, the less goes to the longitudinal and the more to the transverse direction. For a spheroid with rest-frame aspect ratio $p = \gamma c_z/\sqrt{ab} > 1$,

$$
M_z = \frac{1 - e^2}{e^3}\left(\operatorname{artanh} e - e\right),\qquad e = \sqrt{1 - 1/p^2},
$$

often approximated by $M_z \approx 1/(3p)$ ($p$ from about 1 to a few), and transversely $M_x \approx (1 - M_z)\,b/(a + b)$. Back in the laboratory frame (the transverse force gets another $1/\gamma$):

$$
x'' = \frac{K_3 M_x}{a b c_z}\,x,\qquad
z'' = \frac{K_3 M_z}{a b c_z}\,z,\qquad
K_3 = \frac{3 q Q}{4\pi\varepsilon_0\, mc^2\,\beta^2\gamma^3},
$$

where $z' = \dd z/\dd s = \delta/\gamma^2$. Space charge also defocuses longitudinally: it pushes the bunch longer and cancels part of the RF bunching.

## Generalized perveance: why 1/(βγ)³ {#space-charge-perveance}

### DC beam

$$
K = \frac{q I}{2\pi\varepsilon_0\, m c^3\beta^3\gamma^3} = \frac{2I}{I_0\,\beta^3\gamma^3},\qquad
I_0 = \frac{4\pi\varepsilon_0\, m c^3}{q} .
$$

$I_0$ is the characteristic current: $I_0 = 3.13\times 10^7\ \mathrm{A}$ for protons and $3.13\times 10^7\,A/Z\ \mathrm{A}$ for ions of mass number $A$ and charge state $Z$. $K$ is dimensionless and measures space charge against the "stiffness" of the beam.

$\beta^3\gamma^3$ has three parts, each with a clear physical origin:

1. **$1/\gamma^2$**: what is left of the electric repulsion after the magnetic attraction (previous section);
2. **$1/\beta$**: for the same current, slower particles mean more charge per unit length ($\lambda_q = I/\beta c$) and a stronger self-field;
3. **$1/(\gamma\beta^2)$**: $x'' = F/(pv)$ — the force has to overcome the momentum $p = \gamma m\beta c$ to change the direction, and the faster the particle, the shorter the time the force acts per unit length ($1/v$).

Together they give $1/(\beta^3\gamma^3)$. At low energy all three factors grow at once, so space charge is strongest in the low-energy part. A 10 mA proton beam at different energies:

| Kinetic energy | $\beta\gamma$ | $K$ (10 mA) | Relative to 1.527 MeV |
|---|---|---|---|
| 35 keV (ion-source exit scale) | 0.00864 | $9.9\times 10^{-4}$ | 290 times |
| 1.527 MeV | 0.0571 | $3.44\times 10^{-6}$ | 1 |
| 10 MeV | 0.146 | $2.04\times 10^{-7}$ | 0.059 |
| 100 MeV | 0.474 | $6.0\times 10^{-9}$ | 0.0017 |
| 1 GeV | 1.81 | $1.08\times 10^{-10}$ | $3.1\times 10^{-5}$ |

From the ion source to 1 GeV, $K$ falls by about seven orders of magnitude. That is why the design of the low-energy part is governed almost entirely by space charge, whereas the high-energy part is mainly concerned with errors and losses. $K$ at the example energy for several currents:

| DC current | $K$ |
|---|---|
| 0.184 mA | $6.32\times 10^{-8}$ |
| 1 mA | $3.44\times 10^{-7}$ |
| 10 mA | $3.44\times 10^{-6}$ |

### Equivalent perveance of a bunch

Comparing the transverse equation of the uniform ellipsoid, $x'' = K_3 M_x x/(a b c_z)$, with that of a round DC beam, $x'' = K x/a^2$ (for $a = b$, $M_x = (1 - M_z)/2$), the central slice of a bunch acts like a DC beam with the **equivalent current** and perveance

$$
I_{\mathrm{eq}} = \frac{3}{4}\,\frac{Q\,\beta c}{c_z}\,(1 - M_z),\qquad
K_{\mathrm{eq}} = \frac{2 I_{\mathrm{eq}}}{I_0\beta^3\gamma^3} .
$$

The example bunch: $Q = I/f = 1.133\ \mathrm{pC}$ ($7.07\times 10^6$ protons), rms sizes $\sigma_x = 0.954$, $\sigma_y = 0.958$, $\sigma_z = 1.316\ \mathrm{mm}$. The rms-equivalent uniform ellipsoid has semi-axes $\sqrt5\,\sigma$ (see the [next section](#space-charge-envelope)), the rest-frame aspect ratio is $p = 1.379$, $M_z = 0.252$, hence $I_{\mathrm{eq}} = 3.69\ \mathrm{mA}$ and $K_{\mathrm{eq}} = 1.27\times 10^{-6}$.

> **Think about it**: the example beam has an average current of only 0.184 mA. Why is the equivalent current 20 times larger?
>
> Answer: the particles are not spread evenly along the line but packed into bunches. At 162.5 MHz the bunches are $\beta\lambda = 105\ \mathrm{mm}$ apart while their rms length is only 1.3 mm, so the line density inside a bunch is far above the average. The same charge in a Gaussian bunch would have a peak current $Q\beta c/(\sqrt{2\pi}\sigma_z) = 5.9\ \mathrm{mA}$, of the same order as the 3.69 mA of the uniform ellipsoid.

> Note: the space-charge strength of a bunch is set by the **peak line density inside the bunch**, not by the average current. In the example the equivalent current is 20 times the average; compressing the bunch longitudinally increases $I_{\mathrm{eq}}$ further. Estimating $K$ from the average current seriously underestimates space charge.

## KV and rms envelope equations {#space-charge-envelope}

### A balance of three forces

The size of the beam is set by a balance of three actions: the external focusing pushes the beam in; the emittance (the disordered transverse motion of the particles) and space charge (their mutual repulsion) push it out. The envelope equation of [Linear optics](#linear-optics-twiss-transport) already has the first two; now we add the third.

### KV envelope equations

The KV (Kapchinsky–Vladimirsky) distribution is uniform on the surface of a hyper-ellipsoid in 4D phase space (the distributions are compared in [Beam distributions](#distributions)). Its projection on the $x$–$y$ plane is a uniform ellipse, its self-field is exactly linear, and it remains KV in linear external fields. The semi-axes $a(s)$, $b(s)$ of the beam boundary satisfy

$$
a'' + k_x(s)\,a - \frac{2K}{a + b} - \frac{\varepsilon_x^2}{a^3} = 0,\qquad
b'' + k_y(s)\,b - \frac{2K}{a + b} - \frac{\varepsilon_y^2}{b^3} = 0 ,
$$

with $k_{x,y}(s)$ the external focusing strength (quadrupole $k = \pm G/B\rho$, see [Quadrupole](#quadrupole)) and $\varepsilon$ the **edge** (total) geometric emittance. The three terms are external focusing, space-charge defocusing and the emittance ("thermal pressure") term.

### Sacherer's rms envelope equations

Sacherer (1971) showed that for any distribution with elliptical symmetry the rms size $\sigma = \sqrt{\langle x^2\rangle}$ obeys an equation of the same form; for 2D continuous beams the space-charge term does not depend on the shape of the distribution, and for 3D ellipsoidally symmetric bunches only weakly:

$$
\sigma_x'' + k_x\,\sigma_x - \frac{K}{2(\sigma_x + \sigma_y)} - \frac{\varepsilon_{x}^2}{\sigma_x^3} = 0 ,
$$

with $\varepsilon_x$ the rms geometric emittance. Inserting $a = 2\sigma_x$ and $\varepsilon_{\mathrm{edge}} = 4\varepsilon_{\mathrm{rms}}$ into the KV equation gives exactly this. Any distribution can therefore be described by its "rms-equivalent KV beam": **edge size = 2 × rms size, edge emittance = 4 × rms emittance**. This is very useful: the rms envelope depends only on rms quantities, and the details of the distribution do not enter the equation (they do enter how the emittance grows).

### 3D rms envelope equations of a bunch

For a uniform ellipsoid $\langle x^2\rangle = a^2/5$, i.e. the rms-equivalent semi-axis is $\sqrt5\,\sigma$. With the linear forces above:

$$
\sigma_x'' + k_x\sigma_x - \frac{\varepsilon_x^2}{\sigma_x^3} - \frac{K_3 M_x}{5\sqrt5\,\sigma_y\sigma_z} = 0,\qquad
\sigma_z'' + k_z\sigma_z - \frac{\varepsilon_z^2}{\sigma_z^3} - \frac{K_3 M_z}{5\sqrt5\,\sigma_x\sigma_y} = 0 ,
$$

with $y$ analogous to $x$, $k_z$ the longitudinal RF focusing (the $k_{l0}^2$ of [Longitudinal dynamics](#longitudinal)) and $\varepsilon_z$ the geometric rms emittance in $(z, \dd z/\dd s)$. The three directions are coupled through the $M_i$ and the sizes: squeezing the bunch transversely also strengthens its longitudinal space charge.

### Emittance-dominated or space-charge-dominated

Compare the two defocusing terms $K/(4\sigma)$ (round beam) and $\varepsilon^2/\sigma^3$: for $K\sigma^2/(4\varepsilon^2) \ll 1$ the beam is **emittance dominated**, for $\gg 1$ **space-charge dominated**. The two terms scale differently with size: the emittance term as $1/\sigma^3$, the space-charge term as $1/\sigma$. The thinner the beam is squeezed, the more the emittance term wins; the wider the beam, the more space charge wins. With the example emittance, a 10 mA DC beam at $\sigma = 1.33\ \mathrm{mm}$ gives a ratio of about 0.26 — still emittance dominated, but space charge already changes the matched size noticeably (next section).

## Tune depression: the particles oscillate more slowly {#space-charge-tune-depression}

### Physical picture

In a uniform beam the space-charge force is linear, a "negative spring". Every particle inside the beam feels the positive spring of the external focusing and the negative spring of space charge together; the net spring is softer, the particle oscillates more slowly, and the phase it turns through per period is smaller. This is **tune depression**: the zero-current phase advance per period $\sigma_0$ drops to $\sigma < \sigma_0$ with current.

Two things must be kept apart:

- **single particles** oscillate more slowly ($\sigma < \sigma_0$);
- **the beam envelope** can still be matched exactly and repeat with the period of the structure; only the matched size is larger — space charge pushes the beam out until external focusing balances "emittance + space charge" again.

$\sigma/\sigma_0 = 1$ means space charge is negligible; as $\sigma/\sigma_0 \to 0$ the external focusing is almost entirely cancelled by space charge, the particles cross an almost uniform "cold plasma" on nearly straight lines, and the beam size is set by the balance between focusing and space charge, almost independently of the emittance.

### Smooth approximation

When the zero-current phase advance $\sigma_0$ per focusing period $L_p$ is not too large (roughly $\sigma_0 \lesssim 60°$–$80°$), the periodic focusing can be averaged into continuous focusing $k_0 = \sigma_0/L_p$. The rms envelope equation of a round DC beam becomes

$$
\sigma'' + k_0^2\,\sigma - \frac{K}{4\sigma} - \frac{\varepsilon^2}{\sigma^3} = 0 .
$$

### Matched size

For the matched beam, $\sigma'' = 0$:

$$
k_0^2\sigma^4 - \frac{K}{4}\sigma^2 - \varepsilon^2 = 0
\quad\Longrightarrow\quad
\sigma^2 = \frac{K/4 + \sqrt{K^2/16 + 4k_0^2\varepsilon^2}}{2k_0^2} .
$$

At zero current $\sigma_{0,\mathrm{rms}} = \sqrt{\varepsilon/k_0}$.

### Depressed phase advance

A particle in the beam feels the external focusing and the linear space-charge defocusing, so its wave number is $k^2 = k_0^2 - K/(4\sigma^2)$. The matching condition gives $k = \varepsilon/\sigma^2$, and after some algebra

$$
\frac{\sigma}{\sigma_0} = \frac{k}{k_0} = \sqrt{1 + u^2} - u,\qquad
u = \frac{K}{8 k_0\,\varepsilon_{\mathrm{rms}}} = \frac{K}{2 k_0\,\varepsilon_{\mathrm{edge}}} ,
$$

where all $\varepsilon$ are **geometric** (unnormalized) emittances and $\varepsilon_{\mathrm{edge}} = 4\varepsilon_{\mathrm{rms}}$. Here $\sigma$ and $\sigma_0$ are phase advances per period (not rms sizes); $\sigma/\sigma_0$ is the **tune depression**. Designs commonly keep $\sigma/\sigma_0 \gtrsim 0.4$–0.5. From $k = \varepsilon/\sigma_{\mathrm{rms}}^2$ follows an intuitive relation: the matched rms size grows as $\sigma_{\mathrm{rms}}/\sigma_{0,\mathrm{rms}} = (\sigma/\sigma_0)^{-1/2}$; a tune depression of 0.5 makes the beam 41 % wider.

The ratio $K/\varepsilon$ in $u$ tells us that the relative strength of space charge depends not only on the current but also on the emittance — a beam with small emittance (a "cold" beam) is more easily space-charge dominated.

### Numerical example

FODO period: quadrupoles $L_q = 0.1\ \mathrm{m}$, $G = 6\ \mathrm{T/m}$, drifts $L_d = 0.2\ \mathrm{m}$, period $L_p = 0.6\ \mathrm{m}$. AVAS's thick-lens matrices give $\sigma_0 = 52.7°$, $k_0 = 1.534\ \mathrm{m}^{-1}$ and a zero-current matched rms size of 1.254 mm.

| Beam | $K$ | $u$ | $\sigma/\sigma_0$ | $\sigma$ | Matched rms size |
|---|---|---|---|---|---|
| Example bunch ($I_{\mathrm{eq}} = 3.69\ \mathrm{mA}$) | $1.27\times 10^{-6}$ | 0.043 | 0.958 | 50.5° | 1.281 mm |
| 10 mA DC | $3.44\times 10^{-6}$ | 0.116 | 0.891 | 47.0° | 1.329 mm |
| 30 mA DC | $1.03\times 10^{-5}$ | 0.348 | 0.711 | 37.5° | 1.488 mm |

With the same period at $G = 4\ \mathrm{T/m}$ ($\sigma_0 = 34.5°$), the 30 mA DC beam is depressed to 0.60: the weaker the focusing, the larger the relative effect of space charge.

Drag the current `I` in the lab below: $\sigma/\sigma_0$ falls with the current, and the rms envelope with space charge (solid) is wider than at zero current (dashed). Then lower `G` from 6 to 4: at the same current the tune depression becomes stronger. The lab's envelope comes from the uniform-beam kick of the linear preview and the entrance beam is not rematched, so with space charge the envelope shows a little mismatch beating.

```avas-lab
space-charge I=10 W=1.527 mass=938.272 q=1 emit=0.13768 G=6 Lq=0.1 Ld=0.2 cells=10
```

> **Think about it**: a depressed tune means the particles feel weaker focusing. Can stronger external focusing "cure" the tune depression?
>
> Answer: it helps, at a price. $u \propto K/k_0$: with stronger external focusing ($k_0$ larger) the beam becomes thinner and the space-charge force larger, but the external focusing grows more, and $\sigma/\sigma_0$ moves closer to 1 — raising $G$ from 4 to 6 above improves the 30 mA beam from 0.60 to 0.71. The price is a larger $\sigma_0$, and $\sigma_0$ above 90° leads to envelope instabilities ([Linear optics](#linear-optics-periodic)); at low energy quadrupole gradients and apertures also set limits. Design is a balance between these constraints, see [Lattice design](#lattice-design).

> Note: the smooth approximation ignores the envelope ripple within a period and becomes less accurate for larger $\sigma_0$; exact matching requires solving the periodic envelope equations (see [Matching](#matching)). The equivalent $K$ of a bunch also changes with the bunch length; the first row of the table is only an estimate at the entrance.

## Emittance growth and halo {#space-charge-emittance-growth}

In linear theory the rms emittance is conserved. Real distributions are not KV, the self-field contains non-linear parts, and the emittance grows. The main mechanisms (qualitatively):

- **Charge redistribution**: among beams with the same rms size, the uniform one has the lowest self-field energy. An rms-matched but non-uniform beam (Gaussian, peaked) relaxes towards a more uniform distribution within about a quarter of a plasma period, and the surplus field energy ("free energy") is converted into disordered transverse motion of the particles — emittance growth. For a round uniform beam the plasma wave number is $k_p^2 = 2K/a^2 = K/(2\sigma^2)$; in the 10 mA example $k_p = 0.99\ \mathrm{m}^{-1}$, a plasma wavelength of about 6.4 m. The stronger the space charge, the faster and larger the growth.
- **Mismatch and halo**: the envelope of a mismatched beam breathes periodically (in the smooth approximation for a round beam the breathing mode has $k_{\mathrm{b}} = \sqrt{2k_0^2 + 2k^2}$ and the quadrupole mode $k_{\mathrm{q}} = \sqrt{k_0^2 + 3k^2}$). A particle that oscillates in and out of the core feels a space-charge force that breathes with the envelope; when its own oscillation frequency is about half the envelope frequency, it is pushed further out again and again, like a child on a swing who stands up and squats twice per swing — a 2:1 parametric resonance, and the particles pushed out form the **halo**. Where the analogy ends: for small swings a swing's frequency hardly depends on its amplitude, whereas the further a halo particle is from the core, the weaker the space-charge force and the more its frequency changes, so the resonance detunes itself and the halo has a limited extent (which can still be far beyond the rms size). Halo particles are few and hardly change the rms size, but they determine the losses.
- **Envelope instabilities and resonances**: for zero-current phase advances $\sigma_0 > 90°$ space charge can drive envelope instabilities (hence the common design rule $\sigma_0 < 90°$); other parametric resonances of the periodic structure also cause growth.
- **Transverse–longitudinal coupling**: when the transverse and longitudinal "temperatures" (emittance over focusing) differ, space charge exchanges energy between them (the equipartitioning problem); Hofmann's stability charts show the dangerous parameter regions.

In AVAS output, compare the rms sizes (columns 16/18/20) with the maximum sizes (columns 22/24/26) of DataSet.txt: a steadily growing ratio of maximum to rms usually signals developing halo. Emittance growth and halo require the multi-particle engine; the linear preview contains neither.

## Common misconceptions {#space-charge-misconceptions}

- **Looking only at the average current**: the space charge of a bunch is set by the peak line density inside it. The example's equivalent current is 20 times the average, and more after longitudinal compression.
- **Space charge weakens only as $1/\gamma^2$**: $1/\gamma^2$ is only the electromagnetic cancellation; the trajectory equation contains $K \propto 1/(\beta^3\gamma^3)$, and for non-relativistic protons ($\gamma \approx 1$) the weakening with energy comes mostly from $\beta^3$.
- **Mixing rms and edge quantities**: the KV equation uses edge sizes and edge emittance, the rms equation rms quantities; sizes differ by 2, emittances by 4. The two forms of $u$ use $\varepsilon_{\mathrm{rms}}$ and $\varepsilon_{\mathrm{edge}}$ respectively; mixing them up costs a factor 4.
- **Mixing normalized and geometric emittance**: the envelope equations and $u$ contain the geometric emittance $\varepsilon = \varepsilon_n/(\beta\gamma)$; the example's normalized 0.138 is a geometric 2.41 mm·mrad, a factor 17.5.
- **Two meanings of $\sigma$**: in the tune-depression formula $\sigma$ and $\sigma_0$ are phase advances per period, not rms sizes; elsewhere in this chapter $\sigma_x$ is an rms size.
- **Tune depression means focusing has failed**: what is depressed is the single-particle oscillation frequency; the matched envelope still repeats with the period of the structure, only wider.
- **Space charge always makes the emittance grow**: linear (uniform) space charge does not change the rms emittance; growth comes from the non-linear part, mismatch and resonances.
- **Space-charge compensation works for every beam**: in low-energy DC beams, electrons from residual-gas ionization are trapped in the beam potential and partly neutralize the space charge (see [Ion source and low-energy transport](#journey-source)); the potential of RF bunches changes rapidly in time, and this compensation is usually weak there. AVAS's `spacechargecomp` only multiplies the space charge by a constant factor $(1 - V)$; you have to supply the degree of compensation $V$ yourself (see below).
- **`boundary` is the boundary condition of space charge**: it is not; it is the beam-loss boundary, see below.

## In AVAS {#space-charge-in-avas}

### Keywords

Space charge is controlled by `input.txt`, the beam charge comes from `beam.txt`:

```avas-params
input:spacecharge input:scmethod input:numofgrid input:meshrms input:steppercycle beam:current beam:frequency beam:beamtype beam:particlenumber spacechargecomp
```

- `spacecharge 1` switches space charge on, `0` off.
- `current` (mA, average current) and `frequency` (Hz, beam frequency) set the bunch charge $Q = I/f$; each macro-particle carries $Q$ divided by the number of macro-particles `particlenumber`. `beamtype dc` generates a DC beam.
- `scmethod` selects the algorithm: `FFT`, `PICNIC` or `SPICNIC`. The manual only lists the names without describing the implementations; the example project uses `FFT`. When using another algorithm, compare with `FFT` on the same problem.
- `numofgrid Nx Ny Nz` are the numbers of grid cells in the three directions; `meshrms V1 V2 V3` set the grid size $L_x = 2 V_1\sigma_x$ (likewise $y$, $z$), i.e. the grid covers $\pm V_1$ rms sizes. The grid must cover the whole bunch (tails included) while each cell still holds enough macro-particles, see [Multi-particle simulation](#simulation).
- The time step is set by `steppercycle` (steps per RF period); how often the engine solves the space-charge field within these steps is not stated in the manual (engine is the reference).
- The manual also states that with longitudinally periodic boundaries the space-charge algorithm switches to FFT automatically and the longitudinal grid length is set to the bunch period, so more longitudinal grid points are needed.
- The lattice keyword `spacechargecomp V` (space-charge compensation): from the line's position in the lattice onward, the charge on the grid (i.e. the space charge) is multiplied by $(1 - V)$; $V$ is the degree of compensation (0 none, 0.5 half). Of the original kernels only the Windows AVAS.dll knows the keyword; the Linux libAVAS.so ignores it. AVAS.dll ignores the line for $V \ge 1$ (no scaling, so writing $V = 1$ for full compensation does nothing) and amplifies the space charge for $V < 0$. The new kernel's default mode limits $1 - V$ to $[0, 1]$ ($V \ge 1$ removes the space charge, $V < 0$ acts as $V = 0$); the compatibility mode (compat = dll) reproduces AVAS.dll (verified from the AVAS.dll disassembly and by comparing the new kernel's compatibility mode with AVAS.dll character for character). It is a crude model: a constant factor without the physics of electron trapping, and the degree of compensation has to be estimated separately.
- `spacechargelong` and `spacechargetype` act only in the envelope model (`sim_type env`).

> Note: `boundary` in `input.txt` refers to the beam **loss** boundary (`boundary.txt`), not to a boundary condition of the space-charge solver. The boundary conditions, coordinates and interpolation of the engine's space-charge solver are not described in the manual; the engine is the reference.

> **AVAS convention**: the longitudinal angle of `twissz` is $\delta = \Delta p/p$, whereas the 3D envelope equations above use $\dd z/\dd s = \delta/\gamma^2$; they differ by $\gamma^2$. At the example energy $\gamma^2 = 1.003$ and the difference is negligible; at high energy convert carefully.

### Space-charge model of the linear preview

The linear preview of the visual editor adds linear space charge when the beam current is positive:

- bunches: the rms-equivalent uniform ellipsoid (semi-axes $\sqrt5\,\sigma_{x,y,z}$), with $M_z$ from the spheroid formula and the rest-frame aspect ratio $p = \gamma c_z/\sqrt{ab}$, and transversely $M_x = (1 - M_z)\,b/(a + b)$, i.e. the $K_3$ linear force above; it needs `frequency` and `twissz` and otherwise omits space charge with a warning.
- DC beams (`beamtype dc`): the rms-equivalent uniform elliptical cross-section (semi-axes $2\sigma$), i.e. the KV force $2K/(a(a+b))$.
- Every 4 steps a thin-lens kick with the integrated strength is applied (in the middle of the group of steps, second-order splitting), with the force updated from the current rms sizes.

It describes only the linear part of the rms envelope, without emittance growth, halo or losses. With strong space charge its rms sizes may differ from the engine's by up to 13 %. The space-charge lab of this chapter uses the same model (DC beam), with the smooth approximation for the tune depression and a KV edge emittance of $4\varepsilon$.

## Further reading {#space-charge-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: self-fields, KV distribution, envelope equations, emittance growth.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: space-charge multiparticle dynamics and halo.
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18, 1105 (1971).
- I. M. Kapchinsky, V. V. Vladimirsky, Proc. Int. Conf. on High Energy Accelerators, CERN, 1959 (KV distribution).
- I. Hofmann, "Stability of anisotropic beams with space charge", *Phys. Rev. E* 57, 4713 (1998).
- In this primer: [Linear optics](#linear-optics), [Matching](#matching), [Multi-particle simulation](#simulation).
