# Birth: the ion source and the low-energy transport line

Between being stripped out of a puff of hydrogen and entering the RFQ, our proton travels only a metre or two and its kinetic energy is only 35 keV. Yet this short stretch already decides the current and beam quality the whole accelerator can reach: every later section can only preserve or spoil them, never improve them.

This chapter answers three questions:

- Why is the current an ion source can deliver tied to the extraction voltage and the size of the extraction aperture?
- Why can a 35 keV DC beam not be left alone in an empty pipe for even half a metre?
- Why does the low-energy transport line almost always use solenoids rather than quadrupoles?

Finally it explains how the low-energy transport line hands the beam to the RFQ. The overall route is in [The whole linac](#journey-overview), the space-charge concepts used here in [Space charge](#space-charge), and the focusing principle of solenoids in [Solenoid](#solenoid).

> **AVAS does not simulate this section.** The ion source and the low-energy transport line come before the RFQ; their output first goes into an RFQ design code, and the RFQ's output becomes the input of AVAS (see [In AVAS](#journey-source-in-avas) in this chapter). This section is worth understanding because it explains where the beam AVAS receives comes from, and why its emittance and current are what they are.

## Where protons come from {#journey-source-birth}

A proton is a hydrogen atom without its only electron. An ion source has two jobs: first make a sufficiently dense **plasma** (a gas heated until the electrons leave the atoms, so that positive ions and electrons are mixed together; what glows in lightning and neon lights is plasma), and then pull the positive ions out of its surface.

Knocking an electron off a hydrogen atom takes 13.6 eV (15.4 eV for the H₂ molecule). So the key is to make the free electrons in the plasma "hot" enough to knock electrons off the hydrogen atoms they hit; the released electrons hit further atoms, and like an avalanche this keeps the plasma going. Two common ways of heating are:

- **Filament or arc-discharge sources**: a filament heated by a current emits electrons (like the filament of an old light bulb), which are accelerated by the discharge voltage and hit the gas. Simple, with high current, but the filament slowly evaporates and burns out, so its lifetime is limited.
- **Microwave sources (electron cyclotron resonance, ECR)**: no filament. Electrons circle in a magnetic field, and the number of turns per second (the **cyclotron frequency**) is set by the magnetic field alone. Push them with microwaves, and when the microwave frequency equals the cyclotron frequency, every electron gets a push on every turn, like pushing a swing each time it reaches the top, and its energy keeps growing.

Why is the cyclotron frequency set by the magnetic field alone? School physics answers it. An electron moving with velocity $v$ on a circle in a magnetic field $B$ needs a centripetal force, provided by the Lorentz force:

$$
evB = \frac{m_e v^2}{r} \quad\Longrightarrow\quad r = \frac{m_e v}{eB}, \qquad T = \frac{2\pi r}{v} = \frac{2\pi m_e}{eB}.
$$

A faster electron moves on a larger circle, but the time $T$ for one turn does not depend on the velocity. So the cyclotron frequency is $f_{ce} = 1/T = eB/(2\pi m_e)$. Microwaves at 2.45 GHz, the frequency of a kitchen microwave oven, resonate at $B = 2\pi f m_e/e = 0.0875$ T.

For a CW proton linac that must run continuously for weeks or months, a microwave source with no filament to wear out is a common choice.

The plasma contains not only protons but also the molecular ions H₂⁺ (two protons, one electron) and H₃⁺. A good microwave source can reach a high proton fraction (typically above 80 %); the remaining molecular ions have to be removed in the low-energy transport line, and we shall see that the solenoids themselves can separate them.

## Extraction: the current is locked to the voltage and the gap {#journey-source-extraction}

Ions leave the plasma surface and are pulled across a gap by a negative high voltage (relative to the plasma) on the extraction electrode. It might seem that with a dense enough plasma and a high enough voltage the current could be as large as we like. It cannot.

Imagine a stadium emptying through one door. The more slowly the people outside the door move away, the more crowded the doorway, and the harder it is for those behind to get out. Ions are similar: the ions being accelerated in the gap carry positive charge, and their field opposes the applied field, pushing the following ions back. The more ions there are, the more the field near the plasma surface is cancelled; above a certain current the field at the surface is pushed down to zero, and no more ions can be pulled out. This upper limit is the **Child–Langmuir limit**.

How large is the limit? A school-physics capacitor gives an estimate. The extraction gap is like a parallel-plate capacitor with plate distance $d$, area $A$ and voltage $V$; the most charge it can "hold" is about $Q = CV = \varepsilon_0 AV/d$ (with more ions, the field would be cancelled completely). This charge needs a time of about $d/v$ to cross the gap, and the ions' final velocity follows from energy conservation, $\tfrac12 mv^2 = qV$. Current is "charge ÷ time":

$$
I \approx \frac{Q}{d/v} = \frac{\varepsilon_0 A V}{d}\cdot\frac{1}{d}\sqrt{\frac{2qV}{m}} \;\propto\; \frac{A\,V^{3/2}}{d^2}.
$$

The exact calculation (see "[Going deeper: the Child–Langmuir law](#journey-source-deep-child)") only adds a factor 4/9; the current density $J = I/A$ is

$$
J = \frac{4\varepsilon_0}{9}\sqrt{\frac{2q}{m}}\,\frac{V^{3/2}}{d^2}.
$$

It says: the current density grows with the 3/2 power of the voltage and falls with the square of the gap; heavier ions are harder to extract ($1/\sqrt{m}$). For protons the coefficient is $\frac{4\varepsilon_0}{9}\sqrt{2q/m} = 5.45\times10^{-8}$ A·V⁻³ᐟ².

For our 35 kV: $Jd^2 = 5.45\times10^{-8} \times 35000^{1.5} = 0.357$ A. With an extraction aperture of radius $a$, the extracted current is $I = J\pi a^2 = 1.12\ \mathrm{A}\times(a/d)^2$. 10 mA requires $a/d \ge 0.094$; with $a = 3$ mm, for example, the gap may not exceed 32 mm. The example's 0.184 mA needs only $a/d \ge 0.013$.

This is what "the current is locked to the extraction gap" means. There are three ways to extract more current, each with a price:

- **Raise the voltage**: limited by high-voltage breakdown and the equipment.
- **Shrink the gap**: the field in the gap rises, again limited by breakdown.
- **Open the aperture**: the larger the aperture, the harder it is to keep the plasma surface in its ideal shape, and the larger the aberrations and the emittance of the beam.

Real ion-source design finds a balance between these three, and a real extraction system must also take into account the shape of the plasma surface (the meniscus); the one-dimensional formula gives only orders of magnitude and scaling laws.

## The emittance it is born with {#journey-source-emittance}

The [Drift](#drift-size) chapter introduced **emittance**: it measures the area a beam occupies on the "position × angle" chart, in other words how "disordered" the beam is. The ion source already sets a lower limit to the beam's emittance, which afterwards in general can only grow (unless part of the particles is cut away by collimators). The lower limit has two sources.

**The first is temperature.** The ions in the plasma keep bumping into each other, like a jostling crowd leaving a stadium. When an ion is pulled out, besides its forward velocity it carries a small random sideways velocity. The higher the temperature, the larger the sideways velocity; the larger the aperture, the wider the spread of positions. Their product is the emittance. Expressed as the **normalized rms emittance** ("normalized" means multiplied by $\beta\gamma$, which removes the effect of later acceleration; in the unit π·mm·mrad the π just marks that the ellipse area has been divided by π, and mrad is a thousandth of a radian; see [Phase space and emittance](#phase-space)), the result is

$$
\varepsilon_{n,\mathrm{rms}} = \frac{a}{2}\sqrt{\frac{kT}{mc^2}} ,
$$

where $a$ is the aperture radius and $kT$ the ion temperature (in energy units, $k$ being Boltzmann's constant; 1 eV corresponds to about 11,600 K). With $a = 3$ mm and $kT = 1$ eV this gives 0.049 π·mm·mrad; with $kT = 2$ eV, 0.069 π·mm·mrad. Note that this is the normalized emittance: it does not change with the extraction voltage. A higher voltage only makes the beam move forward "colder" (smaller angles), so the geometric emittance shrinks but the normalized emittance does not (see [Phase space](#phase-space)).

**The second is the magnetic field.** The ions of an ECR source are born in a magnetic field. On leaving the field, they get "twisted" by its fringe and acquire a rotation about the axis, faster the farther they are from the axis (Busch's theorem, see [Solenoid](#solenoid)). This rotation also shows up as extra emittance:

$$
\varepsilon_{n,\mathrm{rms}} = \frac{qBa^2}{8mc} .
$$

With $B = 0.1$ T at the extraction and $a = 3$ mm it is about 0.036 π·mm·mrad. Where the two formulas come from is shown in "[Going deeper: the two lower limits of source emittance](#journey-source-deep-emittance)".

These two terms are ideal lower limits. Measured source emittances are often larger, with contributions from extraction aberrations, an uneven plasma surface and nonlinear space charge. The example beam's normalized rms emittance at the RFQ exit is 0.14 π·mm·mrad, only two to three times these limits, which means that the whole front end from the ion source to the RFQ exit has to preserve emittance carefully.

## Space charge of a DC beam and its compensation {#journey-source-compensation}

A 35 keV proton beam is where space charge is fiercest. The [overview](#journey-overview-space-charge) chapter computed a generalized perveance of $K = 9.9\times10^{-4}$ at 10 mA, 288 times that at 1.527 MeV. Working it out with the envelope equation (see the [overview's deep block](#journey-overview-deep-space-charge)): a beam with a 5 mm edge radius, entering parallel, with the emittance above, doubles its radius after only 0.23 m in a field-free pipe. Without some other mechanism to help, the low-energy transport line would need a strong lens every twenty or thirty centimetres.

The help comes from the little gas left in the pipe. However well the vacuum chamber is pumped, a few gas molecules always remain. A proton hitting a gas molecule ionizes it, producing an electron and a sluggish positive ion.

What happens next can be pictured as rain filling a pothole. The beam is positively charged and forms a "potential well" across its cross-section: the potential is high at the beam centre and low at the wall, so for a negatively charged electron the beam centre is the bottom of a hole. For a 10 mA, 35 keV beam the hole is about 146 V deep. The new electrons fall into the hole and cannot get out, so they stay in the beam; the slow positive ions are pushed to the wall. More and more electrons accumulate, cancelling part of the beam's positive charge, and the hole gets shallower until it is almost filled. This is **space-charge compensation** (or neutralization). The cancelled fraction is the degree of compensation $\eta$, and the effective strength of space charge becomes $(1-\eta)$ times the original. The degree of compensation in a real LEBT can be high (typically above 90 %, depending on pressure and gas species).

The effect of compensation is striking. For the beam above, with $\eta = 95$ % the distance for the radius to double goes from 0.23 m to 0.57 m, compared with 0.67 m without any space charge. In other words, the compensated beam behaves almost like a zero-current beam, governed mainly by its emittance.

Compensation takes time: the hole is filled one newly produced electron at a time. At an H₂ pressure of $10^{-3}$ Pa, a 35 keV proton beam needs about 100 µs to become compensated; at ten times lower pressure, ten times longer (estimate in "[Going deeper: the beam's potential well and the compensation time](#journey-source-deep-compensation)"). So pressure is a trade-off: somewhat higher pressure compensates faster and more completely, but collisions of protons with the gas also cause more losses and emittance growth.

## Why the LEBT uses solenoids {#journey-source-solenoid}

A **solenoid** is a set of coils wound around the beam pipe, with the magnetic field inside the pipe pointing along the beam. How does it focus? In two steps (details in [Solenoid](#solenoid)):

1. as a particle enters the solenoid, it crosses the ring of inward-converging field lines at the edge and gets "twisted", starting to rotate about the axis, faster the farther it is from the axis;
2. inside the solenoid, the rotating particle feels a Lorentz force in the axial field that points straight towards the axis.

Each step contributes a factor "field ÷ magnetic rigidity", so the focusing strength is proportional to $(B/B\rho)^2$. The **magnetic rigidity** $B\rho$ measures how hard a particle is to bend with a magnetic field (the faster and heavier, the harder; see [Relativistic kinematics](#kinematics-rigidity)). The thin-lens focal length is

$$
\frac1f = \int\left(\frac{B_z}{2B\rho}\right)^2\dd z ,
$$

where $B_z$ is the axial field in the solenoid and the integral adds up $(B_z/2B\rho)^2$ along the solenoid (for a uniform field it equals $(B/2B\rho)^2 L$). A quadrupole "twists" only once, and its focusing strength $GL/B\rho$ is inversely proportional to only the first power of $B\rho$ ($G$ is the gradient, see [Quadrupole](#quadrupole-focal)). Square versus first power: that decides the territory of each.

A 35 keV proton has a $B\rho$ of only 0.0270 T·m: very "soft". A solenoid 0.2 m long at 0.3 T already gives $1/f = (0.3/0.054)^2 \times 0.2 = 6.16$ m⁻¹, a focal length of 0.16 m (thin lens), no trouble at all for an ordinary room-temperature coil. The same strength from a quadrupole needs only a gradient of 0.83 T/m, which looks cheaper; but a quadrupole focuses in one plane and defocuses in the other, so at least two (a doublet) are needed to focus in both planes. At 1 GeV ($B\rho = 5.66$ T·m) the situation reverses: the same focal length would need a 62.8 T solenoid (impossible), but a quadrupole of 174 T/m (strong too, but it can be made longer). So solenoids are used at low energy and quadrupoles at high energy.

Solenoids have three more advantages in the LEBT:

- **Rotational symmetry**: they focus both planes equally, which suits the round beam from the ion source, and the space-charge force is rotationally symmetric too.
- **They do not destroy compensation**: being magnetic elements, they do not pull the electrons out of the beam's potential well.
- **They separate ion species**: at the same extraction voltage, the magnetic rigidity $B\rho = \sqrt{2mV/q}$ of H₂⁺ is $\sqrt2$ times that of a proton, so its focusing strength is only half; for H₃⁺ only a third. When the protons are focused onto a small aperture downstream, the molecular ions have their focus farther away and spread out before the aperture, and most of them are stopped.

The lab below is one solenoid (0.3 m of drift before it and 0.8 m after). It starts at 35 keV, 0.3 T, 0.2 m: the beam is focused quickly after the solenoid and passes through a focus (the lab warns that the beam becomes wide downstream and may hit the wall). Change $W$ to 1.527 MeV and the same solenoid has almost no effect (focal length 7.1 m), and the angle by which the particles turn about the axis drops from 64° to 10°.

```avas-lab
solenoid W=0.035 B=0.3 L=0.2
```

Now see what "a quadrupole focuses in one plane and defocuses in the other" looks like. The animation below is the example beam (1.527 MeV) through one 10 T/m quadrupole: at the exit the rms size is about 1.4 mm in $x$ but has opened to about 6.3 mm in $y$. Drag "Kinetic energy W" down to its minimum of 0.1 MeV: the magnetic rigidity drops to 0.046 T·m, the focal length of the same quadrupole shrinks from 0.18 m to 0.046 m, the beam opens violently in $y$, and most particles hit the wall. A low-energy beam is "too soft" for a quadrupole to look after both planes at once.

```avas-motion
quad
```

## Focusing into the RFQ {#journey-source-matching}

The last task of the LEBT is to hand the beam to the RFQ, and the RFQ is demanding. Its electric quadrupole focusing is strong with a very short period (one focusing period per RF period, only 15.9 mm at 35 keV), and the matched beam it asks for is very thin.

Let us estimate how thin. The β function of the matched beam in a periodic structure (the Twiss parameter $\beta$ of the [Drift](#drift-deep-matrix) chapter: rms size $= \sqrt{\beta\varepsilon}$) is roughly "period length ÷ phase advance per period (in radians)", $\beta \approx \beta\lambda/\sigma_0$ (the first $\beta$ is the β function, the $\beta$ in $\beta\lambda$ is the velocity; same name, different meaning). With a typical $\sigma_0 \approx 42°$ (parameters in [RFQ](#journey-rfq-focusing)), $\beta \approx 15.9\ \mathrm{mm}/0.73 = 0.022$ mm/mrad. At 35 keV a normalized emittance of 0.14 π·mm·mrad corresponds to a geometric emittance of $0.14/0.00864 = 16.2$ π·mm·mrad, so the matched rms radius at zero current is only $\sqrt{\beta\varepsilon} = 0.59$ mm, and the rms divergence at the waist is $\varepsilon/\sigma_x = 27$ mrad. Space charge makes the real size somewhat larger, but this is the order of magnitude.

In the LEBT solenoids the beam usually has an rms size of a few millimetres, so the last solenoid has to converge it strongly into the RFQ entrance, with divergences of tens of milliradians, like focusing sunlight into a small spot with a magnifying glass. To ease this requirement the RFQ begins with a **radial matching section**: over a few cells the electrode spacing closes in gradually and the focusing goes from weak to strong, so the beam passes smoothly from the "static" LEBT to the time-alternating RFQ focusing.

This transition is also difficult for space-charge compensation: between the last solenoid and the RFQ the compensation is often incomplete, while here the beam is thinnest and space charge strongest. So this stretch is usually kept as short as possible.

## Chopper {#journey-source-chopper}

Sometimes the continuous beam has to be cut into pieces: short pulses with a low duty factor (the beam is present only a small fraction of the time) for commissioning, a machine-protection system that must stop the beam within microseconds, or gaps without beam. This is the job of a **chopper**, usually a pair of deflecting plates that steer the beam onto a dump when a voltage is applied.

Putting the chopper in the LEBT or in the MEBT has pros and cons. In the LEBT the beam energy is low, so the deflecting voltage is low and the dump receives little power (10 mA × 35 kV = 350 W): a simple structure. But a LEBT chopper cannot pick individual bunches: a 35 keV proton needs 38.6 ns to cross 0.1 m long plates, 6.3 periods at 162.5 MHz, and there are no bunches yet anyway. Moreover, after every chop the head of the beam must wait for electrons to fill the potential well again (of order 100 µs) before compensation is re-established, and during that time the beam is mismatched. When individual bunches must be selected, the chopper goes after the RFQ, in the [MEBT](#journey-mebt).

## Think about it {#journey-source-think}

> **Think about it 1**: with the same extraction system, how does the maximum current change if protons are replaced by H₂⁺?
>
> Answer: the Child–Langmuir current is proportional to $1/\sqrt{m}$; H₂⁺ has about twice the proton mass and the same charge, so the limiting current is about $1/\sqrt2 = 71$ % of the proton value. Molecular ions take up extraction capacity too, one reason for wanting a high proton fraction.

> **Think about it 2**: why should electrostatic lenses and long electrostatic deflectors be avoided in the LEBT?
>
> Answer: compensation relies on electrons trapped in the beam's potential well. The applied field of an electrostatic element is much stronger than the beam's potential well (about a hundred volts) and sweeps these electrons away; at the element the beam loses its compensation, space charge suddenly returns, and the emittance grows. A solenoid is a magnetic element and does not clear the electrons, so it is compatible with compensation. For the same reason the RFQ entrance usually has an electron-repelling electrode, which stops the strong RFQ field from sucking electrons out of the LEBT.

> **Think about it 3**: in an ECR source, fast electrons move on large circles and slow ones on small circles. How can the microwaves push all electrons in step?
>
> Answer: because the time for one turn, $2\pi m_e/(eB)$, does not depend on the velocity (see the derivation in the first section of this chapter): a fast electron's circle is larger, but it also moves faster and completes a turn in the same time. So microwaves of one frequency are in step with all electrons.

## Common misconceptions {#journey-source-misconceptions}

**"A higher extraction voltage reduces the emittance."** A higher voltage reduces the geometric emittance (the beam becomes more "parallel"); the normalized emittance is set by the ion temperature and the aperture size and does not change with voltage. Beams of different energies must be compared by their normalized emittances.

**"With 95 % compensation space charge can be ignored completely."** Only approximately, where compensation has been established and no electrostatic element disturbs it. At the head of a pulse, near electrostatic elements and before the RFQ entrance, compensation is incomplete; the degree of compensation itself varies with pressure and beam size and is one of the largest uncertainties in LEBT simulations.

**"Space charge only makes the beam wider; a stronger lens can cancel it."** For a uniform distribution the space-charge force is proportional to the distance from the axis and can indeed be compensated by a lens; but real distributions are not uniform, and nonlinear space-charge forces distort phase space and make the rms emittance grow, which no lens can undo. So the front-end design aims to keep the beam uncompensated as briefly as possible.

**"The more current from the ion source, the better."** The emittance and current the RFQ can accept are limited; anything beyond is lost in the RFQ and causes activation. The goal of the ion source and LEBT is to give the RFQ a beam with small emittance, good matching and just enough current.

## Going deeper: the Child–Langmuir law—the upper limit of the extracted current {#journey-source-deep-child}

The main text estimated $J \propto V^{3/2}/d^2$ with a capacitor. This section computes the exact factor 4/9 in five steps, using energy conservation, current continuity, the parallel-plate capacitor and "guess a power law".

### Step 1: three quantities and their relations

Let the ions start at $x = 0$ (the plasma surface) with roughly zero velocity; the extraction electrode is at $x = d$, with a potential difference $V$. Write $\phi(x)$ for the potential drop an ion has already passed through when it reaches $x$ ($\phi(0) = 0$, $\phi(d) = V$). At every point of the gap there are three quantities: the ion velocity $v$, the ion charge density $\rho$ (charge per cubic metre) and the potential drop $\phi$.

### Step 2: energy conservation gives the velocity

Passing through a potential drop $\phi$, the field does work $q\phi$ on the ion, all of which becomes kinetic energy:

$$
\frac12 mv^2 = q\phi \quad\Longrightarrow\quad v = \sqrt{\frac{2q\phi}{m}} .
$$

### Step 3: current continuity gives the charge density

In a steady state the same charge passes every cross-section each second (otherwise charge would pile up somewhere), just as the flow is the same everywhere in a water pipe. The current density $J = \rho v$ is the same everywhere, so

$$
\rho = \frac{J}{v} = J\sqrt{\frac{m}{2q\phi}} .
$$

Where the ions have just started they are slowest and the charge is densest: the quantitative version of "the doorway is most crowded" in the main text.

### Step 4: charge changes the field

In a parallel-plate capacitor whose plates carry a surface charge density $\sigma$, the field between the plates is $E = \sigma/\varepsilon_0$. Slice the gap into many thin layers: a layer of thickness $\Delta x$ and charge density $\rho$ acts like a "plate" with surface charge density $\rho\Delta x$, making the fields on its two sides differ by $\rho\Delta x/\varepsilon_0$. So the rate of change of the field with position is $\rho/\varepsilon_0$. The field in turn is the rate of change of the potential drop with position ($E = \phi'$), so

$$
\phi'' = \frac{\rho}{\varepsilon_0} = \frac{J}{\varepsilon_0}\sqrt{\frac{m}{2q}}\;\phi^{-1/2} .
$$

Writing the constant on the right as $k = (J/\varepsilon_0)\sqrt{m/(2q)}$, the equation is $\phi'' = k\,\phi^{-1/2}$. The condition "the current has reached its limit" is that the field at the starting surface has been pushed to zero, $\phi'(0) = 0$.

### Step 5: guess a power law

Try $\phi = Cx^n$. The left side is $\phi'' = n(n-1)Cx^{n-2}$, the right side $kC^{-1/2}x^{-n/2}$. For the two sides to be equal for all $x$, the powers of $x$ must agree: $n - 2 = -n/2$, so $n = 4/3$. The coefficients must agree too: $\frac43\cdot\frac13\,C = kC^{-1/2}$, i.e. $C^{3/2} = \frac94 k$. The field of this solution, $\phi' \propto x^{1/3}$, is exactly zero at $x = 0$, as the limit condition requires.

Finally use $\phi(d) = V$: $V = Cd^{4/3}$; raising both sides to the 3/2 power gives $V^{3/2} = C^{3/2}d^2 = \frac94 k\,d^2$. Putting $k$ back:

$$
J = \frac{4\varepsilon_0}{9}\sqrt{\frac{2q}{m}}\,\frac{V^{3/2}}{d^2} .
$$

Checking the coefficient: $\frac{4 \times 8.854\times10^{-12}}{9}\sqrt{\frac{2 \times 1.602\times10^{-19}}{1.673\times10^{-27}}} = 3.935\times10^{-12} \times 1.384\times10^{4} = 5.45\times10^{-8}$ A·V⁻³ᐟ², as in the main text. Compared with the capacitor estimate of the main text, the exact result is smaller by 4/9: the ions in the gap do not all fly at the final velocity, and the slow ions near the starting surface are packed more densely and cancel more of the field.

### For readers who know calculus

Multiplying $\phi'' = k\phi^{-1/2}$ by $\phi'$ and integrating, with $\phi(0) = \phi'(0) = 0$, gives $\phi'^2 = 4k\phi^{1/2}$; separating variables and integrating again gives $\phi = (9k/4)^{2/3}x^{4/3}$, as in Step 5. The assumptions are a one-dimensional planar geometry, zero initial ion velocity, no magnetic field and non-relativistic motion; real extraction uses Pierce electrodes and corrections for the meniscus shape, an extraction system is often characterised by its perveance $P = I/V^{3/2}$, and spherical or cylindrical geometries have different coefficients.

## Going deeper: the two lower limits of source emittance {#journey-source-deep-emittance}

The main text gave the thermal emittance $\frac{a}{2}\sqrt{kT/mc^2}$ and the magnetic emittance $qBa^2/(8mc)$. This section derives them in five steps, using averages, the mean kinetic energy of molecules and the Lorentz force.

### Step 1: the rms position from a uniform round aperture

Particles leave a round aperture of radius $a$ uniformly. For each particle $x^2 + y^2 = r^2$; averaging and using the symmetry of the two planes, $\langle x^2\rangle = \langle r^2\rangle/2$. The area of a thin ring at radius $r$ is proportional to $r$, so outer rings hold more particles; weighting $r^2$ by area gives $\langle r^2\rangle = a^2/2$ (one can check this by dividing the disc into many rings of equal width and adding them up). So $\langle x^2\rangle = a^2/4$ and the rms position is $\sigma_x = a/2$.

### Step 2: the sideways momentum from temperature

Kinetic theory says that particles at temperature $T$ have an average kinetic energy of $\tfrac12 kT$ in each direction ($k$ is Boltzmann's constant): $\tfrac12 m\langle v_x^2\rangle = \tfrac12 kT$. So the rms sideways momentum is $\sigma_{p_x} = m\sqrt{\langle v_x^2\rangle} = \sqrt{mkT}$.

### Step 3: the normalized emittance

Just after the aperture the position and the sideways velocity of the particles are uncorrelated ($\langle xx'\rangle = 0$), and then the rms emittance is simply the rms position times the rms angle. The angle is the sideways momentum divided by the forward momentum, $x' = p_x/p$; the normalized emittance multiplies by $\beta\gamma = p/(mc)$, which cancels $p$:

$$
\varepsilon_{n,\mathrm{rms}} = \beta\gamma\,\sigma_x\sigma_{x'} = \frac{\sigma_x\,\sigma_{p_x}}{mc} = \frac{a}{2}\cdot\frac{\sqrt{mkT}}{mc} = \frac{a}{2}\sqrt{\frac{kT}{mc^2}} .
$$

The forward momentum (that is, the extraction voltage) does not appear in the result: this is "a higher voltage does not change the normalized emittance". Check: $a = 3$ mm, $kT = 1$ eV, $\sqrt{10^{-6}/938.272} = 3.26\times10^{-5}$, $1.5\ \mathrm{mm} \times 3.26\times10^{-5} = 4.9\times10^{-5}$ mm·rad = 0.049 mm·mrad.

### Step 4: a "twist" on leaving the field

An ion born at rest in a magnetic field $B$ (along $z$) must, on leaving, cross the fringe where the field falls from $B$ to 0. There the field lines open outwards and have a radial component $B_r \approx -\frac{r}{2}\frac{\dd B_z}{\dd z}$ (larger farther from the axis). The ion crosses with velocity $v_z$, and the Lorentz force $qv_zB_r$ points in the direction of rotation about the axis. Adding up this force over the time of crossing the fringe (force × time = change of momentum, $\dd t = \dd z/v_z$):

$$
\Delta p_\theta = q\int v_z B_r\,\dd t = q\int B_r\,\dd z = \frac{qBr}{2} .
$$

($\int \frac{\dd B_z}{\dd z}\dd z$ is the total change of the field, $-B$.) So the ion leaves with a rotational momentum $qBr/2$ proportional to its radius: Busch's theorem as it applies here.

### Step 5: rotation is a kind of emittance too

The $x$ component of the rotational momentum $p_\theta = qBr/2$ is $p_x = -\frac{qB}{2}y$: it depends on the particle's $y$, not on its $x$. So on the $x$–$x'$ chart it widens the ellipse just like a random sideways momentum. Its rms value is $\sigma_{p_x} = \frac{qB}{2}\sigma_y = \frac{qB}{2}\cdot\frac a2$; inserted into Step 3:

$$
\varepsilon_{n,\mathrm{rms}} = \frac{\sigma_x\,\sigma_{p_x}}{mc} = \frac{a}{2}\cdot\frac{qBa}{4}\cdot\frac{1}{mc} = \frac{qBa^2}{8mc} .
$$

Check: $B = 0.1$ T, $a = 3$ mm, $qBa^2c/(8mc^2) = 0.1 \times 9\times10^{-6} \times 3\times10^8/(8 \times 938.272\times10^6)$ m·rad $= 3.6\times10^{-8}$ m·rad = 0.036 mm·mrad.

### For readers who know analytical mechanics

Step 4 is a special case of the conservation of canonical angular momentum $P_\theta = \gamma mr^2\dot\theta + \frac{q}{2}B_z r^2$: at birth $\dot\theta = 0$, after the field $B_z = 0$, so $\gamma mr^2\dot\theta = \frac q2 Br^2$. This part of the emittance comes from $x$–$y$ coupling and is the emittance projected onto the $x$–$x'$ plane; because canonical angular momentum is conserved, downstream solenoids cannot remove it, and in principle it can only be redistributed by a "flat-beam transformation" made of skew quadrupoles. When the two lower limits are uncorrelated they add in quadrature, $\varepsilon^2 = \varepsilon_{\text{th}}^2 + \varepsilon_{\text{mag}}^2$.

## Going deeper: the beam's potential well and the compensation time {#journey-source-deep-compensation}

The main text used two numbers: a potential well 146 V deep and a compensation time of about 100 µs. This section computes them.

### Step 1: the potential drop inside the beam

The [overview's deep block](#journey-overview-deep-space-charge) showed that inside a uniform round beam of radius $a$ the field $E_r = \lambda_q r/(2\pi\varepsilon_0 a^2)$ grows linearly from 0 at the centre to $\lambda_q/(2\pi\varepsilon_0 a)$ at the edge, where $\lambda_q = I/v$ is the line charge density. The potential drop is "field × distance"; for a linearly changing field use the average, half the edge value, times $a$:

$$
\Delta\phi_{\text{in}} = \frac12\cdot\frac{\lambda_q}{2\pi\varepsilon_0 a}\cdot a = \frac{\lambda_q}{4\pi\varepsilon_0} .
$$

It does not depend on the beam radius.

### Step 2: the potential drop from the beam to the wall

Outside the beam the field $E_r = \lambda_q/(2\pi\varepsilon_0 r)$ decreases with $r$, and the potential drop from the beam edge $a$ to the wall $b$ is (the same logarithm appears in the capacitance of a coaxial cable)

$$
\Delta\phi_{\text{out}} = \frac{\lambda_q}{2\pi\varepsilon_0}\ln\frac ba .
$$

Adding the two:

$$
\Delta\phi = \frac{I}{4\pi\varepsilon_0 v}\left(1 + 2\ln\frac{b}{a}\right) .
$$

Check: 10 mA, 35 keV ($v = 2.59\times10^6$ m/s): $\lambda_q = 3.86\times10^{-9}$ C/m, $\lambda_q/(4\pi\varepsilon_0) = 3.86\times10^{-9} \times 8.99\times10^9 = 34.7$ V; for $b/a = 5$ the bracket is $1 + 2\ln5 = 4.22$, so $\Delta\phi = 146$ V.

### Step 3: how long compensation takes

Per metre travelled, each proton ionizes on average $n\sigma$ gas molecules ($n$ is the number of molecules per cubic metre, $\sigma$ the ionization cross-section, which can be thought of as the "target area" each molecule presents). A proton covers $v$ metres per second, so each proton produces $n\sigma v$ electrons per second. To produce as many electrons as there are protons in the beam takes a time of about

$$
\tau \approx \frac{1}{n\sigma v} .
$$

The number of gas molecules follows from the ideal-gas law $p = nkT$: at an H₂ pressure of $10^{-3}$ Pa and room temperature, $n = 10^{-3}/(1.38\times10^{-23} \times 300) = 2.4\times10^{17}$ m⁻³. With a cross-section of order $1.5\times10^{-16}$ cm² (i.e. $1.5\times10^{-20}$ m²), for 35 keV protons: $n\sigma v = 2.4\times10^{17} \times 1.5\times10^{-20} \times 2.59\times10^6 = 9.4\times10^3$ s⁻¹, so $\tau \approx 1/(9.4\times10^3\ \mathrm{s^{-1}}) \approx 100$ µs. At ten times lower pressure, ten times longer.

### Step 4: the envelope after compensation

With a degree of compensation $\eta$, the space-charge term of the envelope equation becomes $K(1-\eta)/a$. Integrating $a'' = K(1-\eta)/a + \varepsilon^2/a^3$ numerically ($a$ starting at 5 mm with $a' = 0$, $K = 9.9\times10^{-4}$, edge emittance 64.8 mm·mrad): for $\eta = 0$ the radius doubles at 0.23 m, for $\eta = 95$ % at 0.57 m, and for $K = 0$ at 0.67 m.

### For readers who know plasma physics

The steady-state degree of compensation is set by a balance between the production of electrons, their confinement in the well and their escape (heating over the potential barrier, collisions with the gas); the residual well is usually only a few to several tens of volts (about 60 V was measured for a 62 mA, 92 keV proton beam). The compensating electrons are not uniformly distributed and introduce nonlinear forces into the beam; LEBT simulations often use PIC codes that track the beam, the electrons and the slow ions together. The compensation time $\tau$ depends on how the ionization cross-section varies with energy, so the cross-section data must be taken anew for another energy or gas.

## In AVAS {#journey-source-in-avas}

### AVAS does not simulate this section

AVAS does not simulate the ion source or the low-energy transport line. The plasma and extraction processes of an ion source need dedicated codes; the key physics of a LEBT is space-charge compensation, which depends on pressure, ionization cross-sections and electron dynamics and is likewise outside AVAS's models. This primer does not treat the LEBT as something AVAS simulates.

`beam.txt` has a `beamtype` keyword (`dc` generates a DC beam) and the lattice has a `spacechargecomp V` (space-charge compensation) keyword: from that line on, the space charge is multiplied by $(1 - V)$, $V$ being the degree of compensation; how the kernels differ is described in [Space charge](#space-charge-in-avas). It is only a constant factor without the physics of electron trapping, and the degree of compensation has to be estimated separately. Do not treat these keywords as a basis for LEBT simulation.

```avas-params
beam:beamtype beam:current beam:frequency spacechargecomp
```

### How the beam reaches AVAS

The LEBT output goes to an RFQ design code, and the RFQ output (a particle distribution file or Twiss parameters) goes to AVAS; see [In AVAS in the RFQ chapter](#journey-rfq-in-avas). The properties of the ion source and LEBT reach AVAS through two quantities: the emittance (the RFQ exit emittance is generally no smaller than at its entrance) and the current (`current` in `beam.txt`; the value at the RFQ exit already excludes the losses in the RFQ).

> **AVAS convention**: the emittance in `twissx` / `twissy` of `beam.txt` is the normalized rms emittance (π·mm·mrad) and β is in mm/mrad. The "edge emittance" (4 × rms, for a KV distribution) and geometric emittances common in ion-source and LEBT literature must be converted before comparing.

## Further reading {#journey-source-reading}

- I. G. Brown (ed.), *The Physics and Technology of Ion Sources*, 2nd ed., Wiley-VCH, 2004: principles and extraction of the various ion sources.
- B. Wolf (ed.), *Handbook of Ion Sources*, CRC Press, 1995.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: the Child–Langmuir law, space charge and neutralization, envelope equations.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: linac injectors and low-energy beam transport.
- The CERN Accelerator School (CAS) lecture notes on ion sources.
