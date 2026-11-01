# From ion source to target: a linear accelerator as a whole

If all we want is protons at 1 GeV, why not build one long, uniform accelerating tube and accelerate all the way? Real linacs are instead split into an ion source, a low-energy transport line, an RFQ, a medium-energy transport line, several families of differently shaped superconducting cavities and a high-energy transport line, each with its own structure, RF frequency and kind of focusing. This chapter follows "our proton" from 35 keV to 1 GeV and shows that this division is not an engineering habit: it is forced by three physical facts. The proton's velocity changes by a factor of a hundred; space charge is strongest at low energy; and RF can only accelerate a beam that is already bunched. With this chapter in mind, the next five chapters each answer why a section is where it is and why it looks the way it does.

> **How to read this primer (three routes)**
> - **New to accelerators**: follow Part 1 (the first six chapters, "A proton's journey") for the big picture and drag the interactive sliders in each chapter to see the trends; if a symbol is unfamiliar, do not get stuck—Part 2 (starting from [How AVAS models a beam](#intro), [Relativistic kinematics](#kinematics) and [Phase space and emittance](#phase-space)) builds every definition and derivation from scratch.
> - **Familiar with accelerator physics**: skim "[Numbers along the way](#journey-overview-thread)" below, then jump straight to the [Components](#drift) chapters or the five-step [guided tutorial](#tutorial).
> - **Accelerator experts / looking up software conventions**: each chapter's final dashed-box section "**In AVAS**" collects exact keywords, units and kernel behaviour; a one-stop summary of coordinate/sign conventions, differences from TraceWin and known kernel pitfalls is in [References: In AVAS](#references-in-avas).

## The route {#journey-overview-route}

Let us walk the route once; the following sections come back to explain every turn.

Our proton is born in the plasma of the [ion source](#journey-source) and pulled out by a few tens of kilovolts. Its kinetic energy is then 35 keV, and together with its companions it forms a continuous (DC) beam. The low-energy beam transport (LEBT) uses solenoids to converge this beam into the entrance of the RFQ. The [RFQ](#journey-rfq) holds the beam tightly with electric quadrupole fields, slowly cuts the continuous beam into bunches and accelerates the protons to 1.527 MeV; from here on the beam travels bunch by bunch at the 162.5 MHz rhythm. The [medium-energy beam transport](#journey-mebt) (MEBT) does not accelerate. It shapes the beam: it matches the RFQ output transversely and longitudinally to the superconducting section, measures and collimates the beam, and removes bunches when needed. The [superconducting section](#journey-sc) consists of several cavity families: 162.5 MHz half-wave resonators (HWR) take the proton from 1.5 MeV to about 10 MeV, 325 MHz spoke cavities continue to about 150 MeV, and 650 MHz elliptical cavities bring it to about 1 GeV. Finally the [high-energy beam transport](#journey-hebt) (HEBT) delivers the beam to a target or a beam dump with quadrupoles and dipoles.

The running case is a typical continuous-wave (CW) proton linac. The section boundaries (35 keV, 10 MeV, 150 MeV, 1 GeV) are common choices for this kind of machine, not the parameters of one particular facility. The one fully "real" point is the RFQ exit: the initial beam of the AVAS example project `examples/hwr010` is the 1.527 MeV proton beam at the exit of a 162.5 MHz RFQ (particle file `part_rfq.dst`), and its next step is the HWR superconducting section. AVAS simulates exactly the part from here on.

## The velocity changes a hundredfold {#journey-overview-velocity}

The first fact is the simplest and the most decisive. Kinetic energy $W$, relative velocity $\beta = v/c$ (velocity as a fraction of the speed of light) and the Lorentz factor $\gamma$ (how many times the rest energy $mc^2$ the total energy is; see [relativistic kinematics](#kinematics)) are related by

$$
\gamma = 1 + \frac{W}{mc^2}, \qquad \beta = \sqrt{1 - \frac{1}{\gamma^2}} \approx \sqrt{\frac{2W}{mc^2}} \quad (W \ll mc^2)
$$

For the proton $mc^2 = 938.272$ MeV. At 35 keV, $\beta = 0.00864$: about 2600 km/s, less than one per cent of the speed of light. At 1.527 MeV, $\beta = 0.0570$; at 1 GeV, $\beta = 0.875$. From the source to the end the velocity grows by a factor of about 101; counted as a ratio, most of that growth happens in the first few tens of MeV (at 50 MeV the velocity is already 36 times its starting value): at low energy $\beta \propto \sqrt{W}$, so the energy has to quadruple for the velocity to double, while close to the speed of light the velocity hardly changes any more.

Why does velocity matter? The electric field in an RF cavity oscillates at frequency $f$, and the proton must cross an accelerating gap during the half period in which the field points forward. In one RF period $1/f$ the proton travels

$$
\beta\lambda = \frac{\beta c}{f}
$$

so the distance between neighbouring gaps has to follow the velocity: it is $\beta\lambda/2$ when neighbouring gaps have opposite fields (π mode) and $\beta\lambda$ when they have the same field (0 mode, as in an Alvarez drift-tube linac). At 162.5 MHz ($\lambda = 1.845$ m), $\beta\lambda/2$ is only 8.0 mm for a 35 keV proton, 52.6 mm at 1.527 MeV and 0.81 m at 1 GeV. No single geometry can stay in step with the proton over such a range.

In the lab below you can drag the kinetic energy and watch $\beta$ and $\beta\lambda$; it starts at the 35 keV of the source exit. Drag $W$ from 0.035 MeV to 1.527 MeV and then to 1000 MeV, and notice that $\beta\lambda$ grows fast at first and then slowly.

```avas-lab
kinematics W=0.035
```

### Transit-time factor: one cavity suits one range of velocity

More quantitatively, the field changes while the proton crosses the cavity. Even if we add up the field amplitude across the gaps into a total voltage $V_0 = \int |E_z|\,\dd z$ (the maximum voltage one would get "if the field stayed at its crest without oscillating"), the proton takes a finite time to cross the cavity, so the actual energy gain is reduced by a factor:

$$
\Delta W = qV_0\,T(\beta)\cos\varphi_s
$$

where $\varphi_s$ is the synchronous phase (the effective RF phase when the reference particle passes the cavity centre), and the discount factor $T(\beta) \le 1$ is the **transit-time factor** (its exact Fourier integral definition is in [In AVAS](#journey-overview-in-avas) at the end of this chapter, and the full derivation in [longitudinal motion](#longitudinal-gap) and [RF cavity](#rf-cavity)). Its physical meaning is direct: if the proton is too slow, the field reverses before it finishes crossing a gap, taking back part of the energy gained; if it is too fast, it crosses two neighbouring gaps of opposite field before the RF has had time to flip sign, and their pushes cancel. $T(\beta)$ is close to its peak only when the half-period distance $\beta\lambda/2$ matches the gap spacing.

Computed for the HWR field map `hwr010` of the example project (162.5 MHz, 0.21 m long; on-axis $E_z$, constant-velocity approximation): $T$ peaks at $\beta \approx 0.104$ with about 0.82, consistent with the "010" in its name (presumably a design β of 0.10). $T$ stays above 70 % of the maximum for $\beta = 0.066$–0.21, i.e. for protons of about 2–22 MeV. At the example beam's $\beta = 0.057$, $T$ is only 0.40; at $\beta = 0.145$ (10 MeV) it is 0.75, and at $\beta = 0.43$ (100 MeV) it has fallen to 0.31. One cavity family is useful over roughly a factor of 3 in velocity, while the whole linac has to cover a factor of 100, so several families are needed, each optimised for one design velocity (its design $\beta$; how this relates to the geometric β set by the gap spacing is discussed in the [superconducting section](#journey-sc-ttf)).

The lab below is this HWR cavity (synchronous phase −30°). Its default entrance energy is 5 MeV, close to the optimum velocity. Change $W$ back to 1.527 MeV, then try 50 MeV, and compare the "largest gain" value (the lab includes the change of velocity inside the cavity, so its numbers differ slightly from the constant-velocity $T$): about 0.48 MeV at 1.527 MeV, 0.80 MeV at 5 MeV and 0.72 MeV at 10 MeV.

```avas-lab
rf-cavity W=5
```

> **Think about it**: the example project uses a $\beta = 0.10$ HWR for protons at $\beta = 0.057$, where the transit-time factor is only half its best value. Is that a design mistake?
> Answer: no. A cavity family has to cover a range of velocity, so its first cavities always work below the design velocity and its last ones above it. Each extra family means another cavity design and another kind of cryomodule, which is expensive, so designers usually accept lower efficiency at both ends of a family. A low transit-time factor only means that each cavity gives less energy; it does not stop the beam from being accelerated stably.

## Space charge is strongest at low energy {#journey-overview-space-charge}

The second fact is space charge. The protons in the beam repel each other, and this force tries to blow the beam apart. Picture two protons flying side by side at the same velocity. In the laboratory there is the electric repulsion between them and also the magnetic attraction between two parallel currents; the latter is $\beta^2$ times the former, so the net transverse force is reduced to $1 - \beta^2 = 1/\gamma^2$ of the electric one. At low energy $\gamma \approx 1$ and this cancellation is negligible: the repulsion acts almost in full.

Expressed as the defocusing curvature of a particle trajectory at the beam edge (the full electromagnetic field and envelope-equation derivation is in [space charge](#space-charge-perveance)), its strength is measured by the dimensionless **generalised perveance**:

$$
K = \frac{qI}{2\pi\varepsilon_0\, m c^3 \beta^3\gamma^3} = \frac{2I}{I_0\,(\beta\gamma)^3}, \qquad I_0 = \frac{4\pi\varepsilon_0 mc^3}{q} \approx 31.3\ \mathrm{MA}
$$

Each factor in $(\beta\gamma)^{-3}$ (where $\beta\gamma = p/(mc)$ is the dimensionless momentum) has a clear physical origin: at the same current $I$, a slower beam packs more protons per unit length and has a larger line charge density ($1/\beta$); the same transverse force deflects a slower, lower-momentum particle more ($1/(\gamma\beta^2)$); and the magnetic attraction between parallel currents cancels part of the electric repulsion ($1/\gamma^2$). Multiplied together, space-charge defocusing blows up as $(\beta\gamma)^{-3}$ at low velocity.

Some numbers: a 35 keV, 10 mA DC beam has $K = 9.9\times10^{-4}$; at the same current and 1.527 MeV, $K = 3.4\times10^{-6}$, smaller by a factor of 288. The example beam's average current is only 0.18 mA, giving $K = 1.8\times10^{-5}$ (35 keV) and $6.2\times10^{-8}$ (1.527 MeV). Bunching eats part of this advantage, however: at the RFQ exit the bunch has an rms (root-mean-square, i.e. standard-deviation) duration of about 73 ps while bunches are 6.15 ns apart, so for a Gaussian bunch the peak current at the bunch centre is $1/(f\sqrt{2\pi}\,\sigma_t) \approx 34$ times the average current.

What should space charge be compared with? With the emittance (the measure of how much "position × angle" phase-space area the beam's own random thermal motion occupies; see [phase space and emittance](#phase-space)). In the beam envelope equation (see [space charge](#space-charge)) the outward space-charge term $K/a$ stands next to the outward emittance term $\varepsilon^2/a^3$. Take 35 keV, 10 mA, an edge radius $a = 5$ mm and a normalised rms emittance of 0.14 π·mm·mrad (here $\pi$ is simply a unit label indicating that the ellipse area has been divided by $\pi$, corresponding to an edge geometric emittance $4\varepsilon_{n,\mathrm{rms}}/(\beta\gamma) = 64.8$ mm·mrad): $K/a = 0.198$ m⁻¹ and $\varepsilon^2/a^3 = 0.034$ m⁻¹, so space charge is about 6 times the emittance term. Such a beam is called **space-charge dominated**: its size is governed mainly by its own Coulomb repulsion rather than by its thermal spread (emittance).

This gives the design rules for low energy: focusing must be strong and the focusing period short to hold the repulsion; bunching must be slow, because bunching raises the peak current; and the energy should be raised quickly to leave this regime. Together, these three rules are the design brief of the RFQ.

> **Think about it**: if space charge weakens with energy, why not accelerate the protons to a few MeV directly in the ion source?
> Answer: electrostatic acceleration is limited by high-voltage breakdown, and DC high-voltage installations above a few hundred kilovolts are large and hard to keep stable. Moreover, the extracted current is itself limited by the Child–Langmuir law (see [ion source](#journey-source)); raising the voltage does not give a high-energy, high-current beam for free. In practice the beam is extracted at a few tens of kilovolts and handed over to RF structures.

## RF can only accelerate a bunched beam {#journey-overview-bunching}

The third fact: the source delivers a DC beam, with the protons uniformly spread in time. The field of an RF cavity is positive for half of each period and negative for the other half, so uniformly spread protons gain on average $qV_0T\langle\cos\varphi\rangle = 0$: half are accelerated and half decelerated.

RF accelerates stably only a limited phase interval around the synchronous phase, the "stable region" of longitudinal phase space enclosed by the separatrix (see [longitudinal motion](#longitudinal)). For a synchronous phase $\varphi_s = -30°$ the stable region extends from about −61° to +30° in phase, about 91° wide, one quarter of a period. A DC beam sent straight into cavities would therefore be captured at most to one quarter; the rest is decelerated or slips out of synchronism and ends up on the vacuum chamber. For a high-power proton beam such losses are wasteful and cause severe activation.

So the beam has to be bunched before it is accelerated: the protons spread uniformly over a period must be gathered into the phase range the stable region can hold. Liouville's theorem sets the price: the area of longitudinal phase space cannot shrink, so if the phase range is squeezed, the energy spread must grow. How to squeeze the phase without losing particles and without growing the emittance is the subject of the [RFQ](#journey-rfq) chapter.

Once formed, a bunch does not stay short by itself. Within the bunch, protons with more energy run ahead and those with less fall behind, so the bunch lengthens in a drift. The example beam has a relative momentum spread of 0.375 %; after only 1 m of drift (without space charge) its rms length grows from 1.24 mm to 3.89 mm and its phase width from 4.3° to 13.3°. The transition from the RFQ to the superconducting section therefore cannot be an empty pipe: re-bunching cavities have to compress the bunch again, one of the tasks of the [medium-energy beam transport](#journey-mebt).

## Between sections: matching {#journey-overview-matching}

Each section has its own periodic focusing structure: in the LEBT a few solenoids; in the RFQ an electric quadrupole field that changes sign once per RF period; in the low-β part of the superconducting section a "solenoid + cavity" period; at higher β quadrupole doublets. A periodic structure has its own "matched beam": the Twiss parameters $\alpha$, $\beta$ describing the tilt and aspect ratio of the phase-space ellipse ($\alpha$ tells whether the beam is converging or diverging, $\beta$ sets the ratio of size to divergence; see [phase space and emittance](#phase-space-twiss)) for which the beam envelope repeats from period to period (see [linear optics](#linear-optics) and [matching](#matching)).

The beam leaving one section is generally not the matched beam of the next. For example, the transverse Twiss parameters at the RFQ exit of the example are $\alpha_x = -0.658$, $\beta_x = 0.377$ mm/mrad; they are set by the RFQ structure, whereas the superconducting section needs a different pair. Connected without care, the beam envelope "breathes" periodically in the next section; with space charge this mismatch passes energy to a few particles and forms a halo far from the beam core, and halo particles end up on the wall. The same holds longitudinally: bunch length and energy spread must be matched to the longitudinal focusing of the next section.

Every pair of sections therefore needs a "translation" stage: the last LEBT solenoids converge the beam to the strongly convergent beam the RFQ requires, the RFQ starts with a radial matching section, the MEBT uses quadrupoles and re-bunching cavities for transverse and longitudinal matching, and the transitions between cavity families in the superconducting section are matched one by one. Two rules of thumb from design practice: keep the zero-current phase advance per focusing period below 90° to avoid parametric resonances driven by space charge, and let the phase advance per metre vary smoothly from section to section rather than jump (see [lattice design](#lattice-design)).

## Why the frequency can double {#journey-overview-frequency}

The frequency of the superconducting section changes from 162.5 MHz to 325 MHz and then to 650 MHz. Why low frequency at low energy and high frequency at high energy?

Low energy needs low frequency, for three reasons. First, the gap spacing $\beta\lambda/2$: at 650 MHz it would be only 2.0 mm for a 35 keV proton, too small to build. Second, longitudinal acceptance: the stable region at −30° is about 91° wide, which is 1.55 ns at 162.5 MHz but only 0.39 ns at 650 MHz, too short for a bunch that has just been formed and is still long. Third, RF defocusing: a cavity that focuses longitudinally necessarily defocuses transversely, with strength

$$
k_r^2 = \frac{\pi q E_0T\sin(-\varphi_s)}{mc^2\,\lambda\,\beta^3\gamma^3}
$$

proportional to $1/\lambda$ and inversely proportional to $(\beta\gamma)^3$. $1/(\beta\gamma)^3$ is 5378 at 1.527 MeV, 319 at 10 MeV, 4.9 at 150 MeV and only 0.17 at 1 GeV: at low energy it is large, and a higher frequency would make it impossible to hold.

High energy can use high frequency because the bunch has become shorter in time. In small-amplitude longitudinal motion the area in the $(\Delta t, \Delta W)$ plane is conserved (time and energy are a canonically conjugate pair), and the ratio of energy spread to phase width of a matched bunch is set by the longitudinal focusing (derivation in [longitudinal motion](#longitudinal)):

$$
\frac{\sigma_W}{\sigma_\varphi} = \sqrt{\frac{qE_0T\sin(-\varphi_s)\,\lambda\,mc^2\,\beta^3\gamma^3}{2\pi}}
$$

Together, the rms phase width of a matched bunch, $\sigma_\varphi = \sqrt{\varepsilon_{\varphi W}/(\sigma_W/\sigma_\varphi)}$, decreases as $(\beta\gamma)^{-3/4}$ during acceleration; this is adiabatic phase damping. Inserting the longitudinal rms emittance of the example RFQ exit (0.83 ns·keV, i.e. 48.7°·keV at 162.5 MHz) and the assumptions listed in the section "Numbers along the way" below, the rms duration of the bunch shrinks from 73 ps at the RFQ exit to about 41 ps at the end of the HWR section, about 11 ps at the end of the spoke section and only about 3 ps at 1 GeV. When the frequency doubles, the phase width in degrees doubles at once, at 10 MeV for example from 2.4° to 4.8°, still far smaller than the stable region, so the frequency can be changed; after longitudinal matching at the new frequency the phase width settles at about 3.6°.

Why is a higher frequency worth it? Cavity size scales with wavelength: the simplest TM010 cylindrical (pillbox) cavity has a diameter of $2.405\,\lambda/\pi = 0.766\lambda$, 1.41 m at 162.5 MHz and only 0.35 m at 650 MHz. At high β the smaller multi-cell elliptical cavities are more compact and typically provide more accelerating voltage per unit length, so length and cost go down.

> **Think about it**: in the estimate above the rms bunch length in millimetres stays around 1–2 mm all the way. What, then, does "the bunch becomes shorter" mean?
> Answer: shorter in time and in RF phase. The proton moves faster and faster, so the same 1.5 mm bunch passes a point in less time. What matters to a cavity is the fraction of the RF period the bunch occupies, so the bunch length in degrees or picoseconds is the quantity that decides whether the frequency can be changed.

## Numbers along the way {#journey-overview-thread}

The table collects the state of our proton at the exit of each section. The last column is the main factor limiting the beam in that section; it explains why the next section uses a different structure.

| Section | Exit energy $W$ | $\beta$ | $\beta\gamma$ | $B\rho$ (T·m) | RF frequency | $\beta\lambda$ | rms bunch length | Main limitation |
|---|---|---|---|---|---|---|---|---|
| Ion source + LEBT | 35 keV | 0.00864 | 0.00864 | 0.0270 | DC | 15.9 mm (at 162.5 MHz) | continuous, not bunched | space charge of the DC beam, source emittance |
| RFQ | 1.527 MeV | 0.05698 | 0.05708 | 0.1786 | 162.5 MHz | 15.9 → 105.1 mm | 1.24 mm (4.3°, 73 ps) | capture and bunching, low-energy space charge |
| MEBT | 1.527 MeV | 0.05698 | 0.05708 | 0.1786 | 162.5 MHz | 105.1 mm | held by re-bunchers (1 m of drift makes it 3.9 mm) | debunching, transverse and longitudinal matching |
| HWR section | ≈ 10 MeV | 0.1448 | 0.1464 | 0.458 | 162.5 MHz | 267 mm | ≈ 1.8 mm (2.4°, 41 ps) | transit-time factor, RF defocusing |
| Spoke section | ≈ 150 MeV | 0.5066 | 0.5876 | 1.839 | 325 MHz | 467 mm | ≈ 1.7 mm (1.3°, 11 ps) | longitudinal matching at the frequency jump, transitions between families |
| Elliptical section | ≈ 1 GeV | 0.8750 | 1.8076 | 5.657 | 650 MHz | 404 mm | ≈ 0.8 mm (0.7°, 3 ps) | cavity surface fields, beam loss |
| HEBT and target | 1 GeV | 0.8750 | 1.8076 | 5.657 | — | — | — | beam loss (about 1 W/m), power density on the target |

Some notes. $\beta$, $\beta\gamma$, the magnetic rigidity $B\rho = p/q$ (a measure of how "stiff" the beam is against magnetic bending, numerically the field in tesla needed for a 1 m bending radius; see [relativistic kinematics](#kinematics-rigidity)) and $\beta\lambda$ are exact kinematics; the boundary energies are typical choices. The RFQ row comes from the example's `part_rfq.dst` (which also has an rms energy spread of 11.4 keV, i.e. 0.75 %). The bunch lengths in the later rows are estimates: the bunch is assumed longitudinally matched with no growth of the longitudinal rms emittance, and the adiabatic-damping formula of the previous section is used with an average accelerating gradient $E_0T$ (cavities and focusing elements averaged together) of 0.54 MV/m (HWR section, the value of the example period "solenoid + drift + cavity"), 1 MV/m (spoke) and 3 MV/m (elliptical), and synchronous phases −30°, −25° and −20°. Applied at the RFQ exit, the same formula gives 4.9°, quite close to the measured 4.3°, which shows that the model captures the main physics. Because the bunch length depends only on the 1/4 power of the gradient, a factor of two in the gradient changes it by only about 19 %: with 6 MV/m in the elliptical section, for example, the bunch at 1 GeV is 0.7 mm (0.6°). In a real machine the longitudinal emittance always grows somewhat, and bunches are a little longer than in the table.

As for current: the example beam's average current is 0.18 mA (1.13 pC per bunch, about $7.1\times10^{6}$ protons), which makes a beam power of 184 kW at 1 GeV; high-power CW proton linacs are typically designed for a few to over ten milliamperes, and 10 mA means 10 MW. At 10 MW the widely used guideline of "no more than 1 W lost per metre" means losing no more than a fraction $10^{-7}$ of the beam per metre.

AVAS takes over at the RFQ exit. Below are the first few metres after the example project: the solenoid field map `sol` and the HWR field map `hwr010` alternate in three periods, the structure that the [superconducting section](#journey-sc) chapter and the [guided tutorial](#tutorial) build step by step. Open the 3D view to see the beam squeezed in the solenoids and accelerated in the cavities.

```avas-beamline
drift 0.05 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010
drift 0.2 0.02 0
```

## Common misconceptions {#journey-overview-misconceptions}

**"The design β of a cavity is the velocity of the proton in it."** The design β only marks where the cavity works best: for low-β cavities it is usually quoted as the velocity of the largest $T(\beta)$, while the geometric β set by the gap spacing can be lower (about 0.072 for hwr010, see the [superconducting section](#journey-sc-ttf)). A cavity family works over a fairly wide range on both sides: the example HWR has its largest $T$ at $\beta \approx 0.10$ but is used for protons at $\beta = 0.057$.

**"The emittance is conserved all the way, so the beam gets thinner."** What is (ideally) conserved is the normalised emittance. The geometric emittance $\varepsilon_n/(\beta\gamma)$ decreases with acceleration: a normalised emittance of 0.14 π·mm·mrad corresponds to a geometric emittance of 16.2 π·mm·mrad at 35 keV, 2.45 at 1.527 MeV and 0.077 at 1 GeV. The beam gets thinner because of this "adiabatic damping", not because the emittance shrinks. In a real machine mismatch, non-linear fields and space charge make the normalised emittance grow from section to section, and the transitions between sections are where things most easily go wrong.

**"The higher the energy, the harder the beam is to control."** For space charge it is the other way round: it is strongest at low energy and negligible at high energy. At high energy the difficulty becomes beam power: for a 1 GeV, 10 mA beam, a loss of one part in a hundred thousand is 100 W.

**"When the frequency doubles, the bunch spacing halves."** It does not. The bunches are formed in the RFQ at 162.5 MHz and stay 6.15 ns apart; in the 325 MHz cavities there is simply one bunch every second RF period, and the other "bucket" is empty.

**"Getting $\beta\lambda/2$ right in every section is enough."** The gap spacing only keeps the synchronous particle in step. The other particles of the bunch stay in the stable region through longitudinal focusing (a negative synchronous phase), and longitudinal focusing necessarily brings transverse RF defocusing, so accelerating structure and transverse focusing are designed together.

## In AVAS {#journey-overview-in-avas}

### Which sections AVAS covers

AVAS is a multi-particle tracking code that starts from a given initial beam (see [accelerators and AVAS](#intro)). Its elements are drifts, quadrupoles, solenoids, dipoles with pole faces, steerers, RF / electrostatic / magnetostatic field-map elements, and superposed fields. Against the route above:

| Section | Simulated by AVAS | Notes |
|---|---|---|
| Ion source, LEBT | no | AVAS does not simulate this section, see [ion source](#journey-source-in-avas) |
| RFQ | no | AVAS does not simulate this section; its output beam is handed to AVAS as a particle file or as Twiss parameters, see [RFQ](#journey-rfq-in-avas) |
| MEBT | yes | quadrupoles `quad`, solenoids, buncher field maps `field … 1 …`, drifts, steerers |
| Normal-conducting DTL | no (no dedicated model) | with a 3D field map it could in principle be read as a `field` element; not discussed in this primer |
| Superconducting section | yes | cavity field maps `field … 1 …`, focusing with solenoid field maps `field … 3 …` or matrix elements `solenoid`, `quad` |
| HEBT | partly | `quad`, `bend` + `edge`, `steerer`; targets, non-linear magnets and raster magnets are not among AVAS's elements |

> **AVAS convention**: in multi-particle runs the first and last element of the lattice must not be matrix elements (a requirement of the manual); the beam line above therefore starts and ends with short drifts.

### Handing the beam to AVAS

There are two ways to give AVAS its initial beam, both in `beam.txt`:

- Read a particle file: `readparticledistribution <file>` names a `.dst` / `.edst` file (for example the `part_rfq.dst` written by an RFQ code); the GUI switches this with `use_dst 1`. When a file is read, every keyword in `beam.txt` except `numofcharge` is ignored.
- Generate from parameters: give the particle species, kinetic energy, current, beam frequency, distribution type and the Twiss parameters of the three planes, and AVAS generates the macro-particles. The example project does this: `use_dst 0`, the transverse Twiss parameters agree with the rms values computed from `part_rfq.dst`, and both the transverse and the longitudinal distributions are `GS`.

```avas-params
beam:readparticledistribution beam:use_dst beam:kneticenergy beam:current beam:frequency beam:twissx beam:twissz beam:distribution
```

> **AVAS convention**: in `twissx` / `twissy` the β unit is mm/mrad and the emittance is the **normalised rms** emittance (π·mm·mrad); `twissz` uses the same units with the longitudinal angle coordinate $z' = \Delta p/p$. The 0.14 π·mm·mrad of the example is a normalised rms value.

> **AVAS convention**: `frequency` in `beam.txt` is the beam frequency; the charge per bunch is $Q = I/f$. The RF frequency of each cavity is written in its own `field` line. How the engine handles a beam frequency different from the cavity frequency (for example one bunch in every second "bucket" of a 325 MHz cavity) is not described in the manual; the engine is the reference.

### The example project and supplementary formulas

The `beam.txt` of `examples/hwr010` gives a 1.527 MeV, 0.184 mA, 162.5 MHz proton beam with 5260 macro-particles; the lattice is a single HWR field map. All numbers of this chapter can be checked on the example: read the parameters from `part_rfq.dst` on the Beam page, or extend it to several periods in the [guided tutorial](#tutorial).

For reference, here are the two integral and field expressions cited in the main text (full derivations in [longitudinal motion](#longitudinal-gap) and [space charge](#space-charge-perveance)):

- **Transit-time factor in the constant-velocity approximation**: for an on-axis field $E_z(z)\cos(\omega t + \varphi)$ and a particle moving at constant velocity $\beta c$ ($t = z/(\beta c)$),

$$
\Delta W = q\int E_z(z)\cos\!\left(\frac{\omega z}{\beta c} + \varphi\right)\dd z = qV_0\,T(\beta)\cos\varphi_s, \qquad
T(\beta) = \frac{\left|\int E_z(z)\,\ee^{\,\mathrm{i}kz}\,\dd z\right|}{\int |E_z(z)|\,\dd z}, \quad k = \frac{2\pi}{\beta\lambda}
$$

- **Space-charge fields inside a uniform round beam**: inside a uniform cylinder of radius $a$ and current $I$, the radial electric and azimuthal magnetic fields are

$$
E_r = \frac{I\,r}{2\pi\varepsilon_0 a^2 \beta c}, \qquad B_\theta = \frac{\beta}{c}E_r, \qquad F_r = q(E_r - \beta c B_\theta) = \frac{qE_r}{\gamma^2}
$$

Dividing by $\gamma m v^2$ for $r'' = F_r/(\gamma m v^2)$ yields the edge defocusing term $K/a$.

## Further reading {#journey-overview-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: the linac as a whole, transit-time factor, longitudinal motion and adiabatic phase damping, the RFQ and superconducting linacs; the formulas of this chapter agree with it.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: space charge, perveance and the envelope equation.
- CERN Accelerator School (CAS) lecture notes on linear accelerators and RF.
- The AVAS manual `docs/使用说明20260427.docx`: the `beam.txt` keywords and particle files.
