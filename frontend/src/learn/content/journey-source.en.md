# Birth: ion source and low-energy transport

Between being stripped from a hydrogen molecule and entering the RFQ, a proton travels only a metre or two and gains only 35 keV, yet this short stretch already fixes the current and beam quality the whole accelerator can reach: every later section can only preserve or spoil them, never improve them. Why is the current an ion source can deliver tied to the extraction voltage and to the size of the extraction aperture? Why can a 35 keV DC beam not be left alone in an empty pipe for half a metre? Why does the low-energy transport almost always use solenoids rather than quadrupoles? This chapter answers these questions and shows how the low-energy transport hands the beam to the RFQ. For the overall route see the [overview](#journey-overview); the space-charge ideas are in [space charge](#space-charge) and the focusing of solenoids in [solenoid](#solenoid).

> **AVAS does not simulate this section.** The ion source and the low-energy transport come before the RFQ; their output goes into an RFQ design code first, and only the RFQ output becomes AVAS's input (see [In AVAS](#journey-source-in-avas) below). Understanding this section explains where the beam AVAS receives comes from, and why its emittance and current are what they are.

## Where protons come from {#journey-source-birth}

A proton is an ionised hydrogen atom. An ion source has to make a dense enough hydrogen plasma and then pull the positive ions out of its surface.

Ionising a hydrogen atom takes 13.6 eV (15.4 eV for the H₂ molecule), so the key is to make the electrons in the plasma "hot" enough to knock electrons off hydrogen in collisions. Two common ways are:

- **Filament or arc-discharge sources**: a hot filament emits electrons that are accelerated by the discharge voltage and hit the gas. Simple and capable of high current, but the filament evaporates and burns out, so its lifetime is limited.
- **Microwave sources (electron cyclotron resonance, ECR)**: no filament. Electrons circle in a magnetic field at the cyclotron frequency $f_{ce} = eB/(2\pi m_e)$; when the microwave frequency equals the cyclotron frequency, the microwave field pushes each electron once per turn and its energy keeps growing. Commercial 2.45 GHz microwaves resonate at $B = 2\pi f m_e/e = 0.0875$ T.

For CW proton linacs that must run for weeks or months, microwave sources without a consumable filament are a common choice.

The plasma contains not only protons but also the molecular ions H₂⁺ and H₃⁺. A good microwave source can reach a high proton fraction (typically above 80 %); the remaining molecular ions have to be removed in the low-energy transport, and as we shall see, the solenoids themselves separate them.

## Extraction: the current is locked to voltage and gap {#journey-source-extraction}

Ions leave the plasma surface and are accelerated by the extraction electrode, which is at a negative high voltage relative to the plasma. It might seem that the higher the voltage, the larger the current, and that the current could grow without limit if only the plasma were dense enough. It cannot, because of the ions' own charge: the ions in the extraction gap form a space charge whose field cancels part of the applied field. When the current is too large, the field near the plasma surface is pushed down to zero and no more ions can be pulled out. This limit is the Child–Langmuir law.

The derivation uses a one-dimensional planar model and only three relations. Let the ions start at $x = 0$ (with nearly zero velocity) and the extraction electrode be at $x = d$, with potential difference $V$; let $\phi(x)$ be the potential drop the ions have already crossed. Energy conservation gives the velocity, current continuity gives the charge density, and Poisson's equation links charge density and potential:

$$
\frac12 mv^2 = q\phi, \qquad J = \rho v, \qquad \frac{\dd^2\phi}{\dd x^2} = \frac{\rho}{\varepsilon_0} = \frac{J}{\varepsilon_0}\sqrt{\frac{m}{2q\phi}}
$$

At the limit, the field at the starting surface is zero ($\phi'(0) = 0$). The solution is a power law $\phi \propto x^{4/3}$; putting it back and solving for the coefficient gives the limiting current density

$$
J = \frac{4\varepsilon_0}{9}\sqrt{\frac{2q}{m}}\,\frac{V^{3/2}}{d^2}
$$

The current density grows as the 3/2 power of the voltage and falls with the square of the gap; heavier ions are harder to extract ($1/\sqrt{m}$). For protons the coefficient is $\frac{4\varepsilon_0}{9}\sqrt{2q/m} = 5.45\times10^{-8}$ A·V⁻³ᐟ².

With our 35 kV, $Jd^2 = 0.357$ A. For an extraction aperture of radius $a$ the current is $I = J\pi a^2 = 1.12\ \mathrm{A}\times(a/d)^2$. Getting 10 mA requires $a/d \ge 0.094$: with $a = 3$ mm, for example, the gap may be at most 32 mm. The 0.18 mA of the example needs only $a/d \ge 0.013$.

This is what "the current is tied to the extraction gap" means. To extract more current one can raise the voltage (limited by breakdown and by the equipment), shrink the gap (which raises the field in the gap, again limited by breakdown) or open the aperture (the larger the aperture, the harder it is to keep the plasma surface in its ideal shape, and the larger the aberrations and emittance). An ion-source design balances these three, and a real extraction system has to account for the shape of the plasma surface (the meniscus); the one-dimensional formula gives only the order of magnitude and the scaling.

> **Think about it**: with the same extraction system, how does the maximum current change if the protons are replaced by H₂⁺?
> Answer: the Child–Langmuir current is proportional to $1/\sqrt{m}$. H₂⁺ has about twice the proton mass and the same charge, so its limiting current is about $1/\sqrt2 = 71$ % of the proton's. Molecular ions use up extraction capacity too, which is one reason a high proton fraction is wanted.

## The emittance a beam is born with {#journey-source-emittance}

The ion source sets a lower bound on the beam emittance, which afterwards generally only grows (unless a collimator cuts away part of the particles). There are two contributions.

The first is temperature. The ions move randomly in the plasma, and a temperature $kT$ corresponds to a transverse momentum spread $\sigma_{p_x} = \sqrt{mkT}$. For extraction from a uniform round aperture of radius $a$ the rms position is $a/2$, and the product of the two divided by $mc$ is the **normalised rms emittance** (an invariant measuring the "position × angle" phase-space area occupied by the beam with the effect of later acceleration divided out; the $\pi$ in `π·mm·mrad` is simply a label meaning the ellipse area has been divided by $\pi$, and `mrad` is the trajectory angle in thousandths of a radian; see [phase space and emittance](#phase-space)):

$$
\varepsilon_{n,\mathrm{rms}} = \frac{a}{2}\sqrt{\frac{kT}{mc^2}}
$$

With $a = 3$ mm and $kT = 1$ eV this is 0.049 π·mm·mrad; with $kT = 2$ eV, 0.069 π·mm·mrad. Note that this is the normalised emittance: it does not change with the extraction voltage. A higher voltage only makes the beam move forward "colder", reducing the geometric emittance, while the normalised emittance stays the same (see [phase space](#phase-space)).

The second is the magnetic field. In an ECR source the ions are born inside a magnetic field, and when they leave it they acquire an azimuthal velocity proportional to their radius (Busch's theorem, see [solenoid](#solenoid)), which appears as an extra emittance:

$$
\varepsilon_{n,\mathrm{rms}} = \frac{qBa^2}{8mc}
$$

With $B = 0.1$ T at the extraction and $a = 3$ mm this is about 0.036 π·mm·mrad.

These two are ideal lower bounds. The emittances measured at real ion sources are often larger, with contributions from the aberrations of the extraction system, non-uniformity of the plasma surface and non-linear space charge. The example beam has a normalised rms emittance of 0.14 π·mm·mrad at the RFQ exit, which means the whole front end from the ion source to the RFQ exit must preserve the emittance carefully.

## Space charge of a DC beam, and its compensation {#journey-source-compensation}

A 35 keV proton beam is where space charge is fiercest. The [overview](#journey-overview-space-charge) computed it: at 10 mA the generalised perveance is $K = 9.9\times10^{-4}$, 288 times larger than at 1.527 MeV. Integrating the envelope equation $a'' = K/a + \varepsilon^2/a^3$ numerically, a beam with an edge radius of 5 mm, entering parallel and with the emittance above, doubles its radius after only 0.23 m in a field-free drift. Without some other mechanism to help, the low-energy transport would need a strong lens every twenty or thirty centimetres.

The help comes from the residual gas. There is always a little gas in the vacuum chamber; a proton that hits a gas molecule can ionise it, producing an electron and a slow positive ion. The beam itself is positively charged and forms a transverse potential well, with a potential difference between centre and wall of

$$
\Delta\phi = \frac{I}{4\pi\varepsilon_0 v}\left(1 + 2\ln\frac{b}{a}\right)
$$

($a$ is the beam radius, $b$ the wall radius). For 10 mA, 35 keV and $b/a = 5$ this is about 146 V. The new electrons are trapped in this well and stay in the beam, while the slow positive ions are pushed to the wall. Electrons accumulate, the net charge of the beam decreases, until the well is almost filled. This is space-charge compensation (neutralisation); the degree of compensation $\eta$ is the fraction cancelled, and the effective perveance becomes $K(1-\eta)$. In a real LEBT the degree of compensation can be high (typically above 90 %, depending on pressure and gas species).

The effect is large. For the beam above, with $\eta = 95$ % the distance in which the radius doubles grows from 0.23 m to 0.57 m, while without any space charge it would be 0.67 m: the beam behaves almost like a zero-current beam, governed mainly by its emittance.

Compensation takes time. Electrons are produced at the rate $n\sigma v$ ($n$ the gas density, $\sigma$ the ionisation cross section), so the characteristic time to build up compensation is about

$$
\tau \approx \frac{1}{n\sigma v}
$$

For an H₂ pressure of $10^{-3}$ Pa ($n = 2.4\times10^{17}$ m⁻³) and a cross section of the order of $1.5\times10^{-16}$ cm², $\tau$ is about 100 µs for 35 keV protons; ten times lower pressure means ten times longer. The pressure is therefore a trade-off: somewhat higher pressure compensates faster and more completely, but collisions with the gas also cause more beam loss and emittance growth.

> **Think about it**: why are electrostatic lenses and long electrostatic deflector plates avoided in the LEBT where possible?
> Answer: compensation relies on electrons trapped in the beam's potential well. The applied field of an electrostatic element is much stronger than the well (about a hundred volts) and sweeps these electrons away; where the element sits, the beam loses its compensation, space charge returns suddenly and the emittance grows. A solenoid is a magnetic element and does not clear the electrons, so it is compatible with compensation. For the same reason the RFQ entrance usually has an electrode that repels electrons, to stop the strong RFQ fields from draining the electrons out of the LEBT.

## Why the low-energy transport uses solenoids {#journey-source-solenoid}

A solenoid focuses charged particles through its fringe radial field: crossing the entrance fringe, a particle acquires an azimuthal velocity, and inside, the longitudinal field turns that azimuthal velocity into a force towards the axis. The result is focusing in both planes at once, with a thin-lens focal length of (derivation in [solenoid](#solenoid))

$$
\frac1f = \int\left(\frac{B_z}{2B\rho}\right)^2\dd z
$$

The focusing strength is proportional to $(B/B\rho)^2$, the inverse **square** of the magnetic rigidity, whereas the strength $GL/B\rho$ of a quadrupole is inversely proportional only to the first power of $B\rho$. This decides where each one belongs.

A 35 keV proton has $B\rho$ of only 0.0270 T·m. A solenoid 0.2 m long with 0.3 T already gives $1/f = 6.16$ m⁻¹, a focal length of 0.16 m (thin lens), easy for an ordinary room-temperature coil. A quadrupole would need only a 0.83 T/m gradient for the same strength, which looks cheaper; but a quadrupole focuses in one plane and defocuses in the other, so at least two (a doublet) are needed to focus both planes. At 1 GeV ($B\rho = 5.66$ T·m) things are reversed: the same focal length would need a 62.8 T solenoid (impossible), while a quadrupole needs 174 T/m (also strong, but it can be made longer). So solenoids are used at low energy and quadrupoles at high energy.

Solenoids have three further advantages in an LEBT. They are rotationally symmetric, which suits the round beam from the source, and the space-charge force is rotationally symmetric too. They are magnetic and do not destroy space-charge compensation. And they separate ion species: at the same extraction voltage H₂⁺ has $B\rho = \sqrt{2mV/q}$ larger by $\sqrt2$ than the proton, so it is focused only half as strongly, and H₃⁺ only a third. When the protons are focused onto a small aperture downstream, the molecular ions have their focus far away, spread out before the aperture and are mostly stopped.

The lab below is one solenoid (0.3 m of drift before it, 0.8 m after it). By default it is set to 35 keV, 0.3 T and 0.2 m: the beam is focused quickly after the solenoid and passes through a focus. Change $W$ to 1.527 MeV and the same solenoid has almost no effect (focal length 7.1 m), and the beam rotation drops from 64° to 10°.

```avas-lab
solenoid W=0.035 B=0.3 L=0.2
```

## Converging into the RFQ {#journey-source-matching}

The last task of the LEBT is to hand the beam to the RFQ, and the RFQ is demanding. Its electric quadrupole focusing is strong and its period short (one focusing period per RF period, only 15.9 mm at 35 keV), so its matched beam is very thin. A smooth-approximation estimate: the matched β function is about the period length divided by the phase advance per period, $\beta \approx \beta\lambda/\sigma_0$. With a typical $\sigma_0 \approx 42°$ (parameters in [RFQ](#journey-rfq-focusing)), $\beta \approx 0.022$ mm/mrad. At 35 keV, a normalised emittance of 0.14 π·mm·mrad is a geometric emittance of 16.2 π·mm·mrad, so the zero-current matched rms radius is only $\sqrt{\beta\varepsilon} = 0.59$ mm, and the rms divergence at the waist is $\varepsilon/\sigma_x = 27$ mrad. Space charge makes the real size somewhat larger, but this is the order of magnitude.

In the LEBT solenoids the rms beam size is usually a few millimetres, so the last solenoid has to converge the beam strongly into the RFQ entrance, with divergences of tens of milliradians. To soften this requirement the RFQ begins with a radial matching section: over a few cells the electrode gap closes gradually and the focusing grows from weak to strong, letting the beam pass from the "static" LEBT to the time-alternating RFQ focusing.

This transition is also difficult for space-charge compensation: between the last solenoid and the RFQ the compensation is often incomplete, just where the beam is thinnest and space charge strongest. This stretch is therefore usually kept as short as possible.

## Chopper {#journey-source-chopper}

Sometimes the continuous beam has to be cut into pieces: short pulses at low duty factor for commissioning, a machine-protection system that has to stop the beam within a few microseconds, or gaps without beam. This is done by a chopper, usually a pair of deflector plates that, when powered, deflect the beam onto a beam stop.

Whether the chopper sits in the LEBT or the MEBT is a trade-off. In the LEBT the beam energy is low, the deflecting voltage needed is low, and the beam stop takes little power (10 mA × 35 kV = 350 W), so the device is simple. But an LEBT chopper cannot pick individual bunches: a 35 keV proton needs 38.6 ns to cross 0.1 m long plates, 6.3 periods at 162.5 MHz, and there are no bunches yet anyway. Moreover, after each chop the head of the beam needs about $\tau$ (estimated above at the order of 100 µs) to rebuild its compensation, and during that time the beam is mismatched. When bunch-by-bunch selection is needed, the chopper goes after the RFQ, into the [MEBT](#journey-mebt).

## Common misconceptions {#journey-source-misconceptions}

**"A higher extraction voltage reduces the emittance."** A higher voltage reduces the geometric emittance (the beam is more "parallel"); the normalised emittance is set by the ion temperature and the aperture size and does not change with voltage. Beams at different energies must be compared by normalised emittance.

**"At 95 % compensation space charge can be ignored completely."** This holds only approximately where compensation has been established and no electrostatic element disturbs it. At the head of a pulse, near electrostatic elements and just before the RFQ the compensation is incomplete; the degree of compensation itself varies with pressure and beam size, and is one of the largest uncertainties in LEBT simulations.

**"Space charge only makes the beam larger, and stronger lenses can undo it."** For a uniform distribution the space-charge force is linear and can indeed be compensated by lenses; but real distributions are not uniform, and the non-linear part of the space-charge force distorts phase space and grows the rms emittance, which lenses cannot recover. The front end is therefore designed to keep the beam uncompensated as briefly as possible.

**"The more current from the source, the better."** The emittance and current the RFQ can accept are limited; anything beyond is lost in the RFQ and causes activation. The goal of the source and the LEBT is to give the RFQ a well-matched beam with small emittance and just enough current.

## In AVAS {#journey-source-in-avas}

### AVAS does not simulate this section

AVAS does not simulate the ion source or the low-energy transport. The plasma and the extraction process need dedicated codes, and the key physics of the LEBT, space-charge compensation, depends on the pressure, ionisation cross sections and electron dynamics, which are also outside AVAS's models. This primer does not treat the LEBT as something to simulate with AVAS.

`beam.txt` has a `beamtype` keyword (`dc` generates a DC beam) and the lattice has a `spacechargecomp V` (space-charge compensation) keyword: from that line on, the space charge is multiplied by $(1 - V)$, $V$ being the degree of compensation; how the kernels differ is described in [Space charge](#space-charge-in-avas). It is only a constant factor without the physics of electron trapping, and the degree of compensation has to be estimated separately. Do not take these keywords as a basis for LEBT simulations.

```avas-params
beam:beamtype beam:current beam:frequency spacechargecomp
```

### How the beam reaches AVAS

The LEBT output goes to an RFQ design code, and the RFQ output (a particle file or Twiss parameters) goes to AVAS; how, is described in [the RFQ chapter's In AVAS section](#journey-rfq-in-avas). The properties of the source and the LEBT reach AVAS through two quantities: the emittance (the emittance at the RFQ exit is generally not smaller than at its entrance) and the current (`current` in `beam.txt`; the value at the RFQ exit already excludes what was lost in the RFQ).

> **AVAS convention**: the emittance in `twissx` / `twissy` of `beam.txt` is the normalised rms emittance (π·mm·mrad) and β is in mm/mrad. The "edge emittance" (4 × rms for a KV distribution) and the geometric emittance that are common in ion-source and LEBT papers must be converted before comparing.

## Further reading {#journey-source-reading}

- I. G. Brown (ed.), *The Physics and Technology of Ion Sources*, 2nd ed., Wiley-VCH, 2004: principles and extraction of the various ion sources.
- B. Wolf (ed.), *Handbook of Ion Sources*, CRC Press, 1995.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: the Child–Langmuir law, space charge and neutralisation, the envelope equation.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: linac injectors and low-energy beam transport.
- CERN Accelerator School (CAS) lecture notes on ion sources.
