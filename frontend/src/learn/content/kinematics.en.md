# Relativistic kinematics

The protons of the example project have a kinetic energy of 1.527 MeV, only 0.16 % of their rest energy, and Newton's formula gets their speed wrong by just 0.12 % — so why bother with relativity? An electron of the same 1 MeV already moves at 0.94 times the speed of light, while Newton's formula would give 1.98 times. And a more practical puzzle: raise the proton energy tenfold, and the magnet that bends it needs to be only about 3.2 times stronger, while an electrostatic element must be nearly 10 times stronger. Why?

Starting from the definitions of energy and momentum, this chapter shows where the most basic kinematic quantities of an accelerator come from — the Lorentz factor $\gamma$, the velocity $\beta$, the momentum $p$, the magnetic rigidity $B\rho$ and the distance $\beta\lambda$ travelled in one RF period — how they are connected, and which scale of the accelerator each one sets. By the end you will see why quadrupole strengths are normalized by $B\rho$, why bunches lengthen in low-energy drifts, and why an accelerating cavity works only over a certain range of velocity. The whole picture of a linac, in which $\beta$ spans two orders of magnitude, is in [From ion source to target](#journey-overview).

## Energy, velocity and momentum {#kinematics-variables}

### Starting from the total energy

In special relativity a particle of mass $m$ moving at speed $v$ has the total energy $E = \gamma mc^2$ with $\gamma = 1/\sqrt{1 - v^2/c^2}$. At rest $\gamma = 1$ and $E = mc^2$ is the rest energy; the excess is the kinetic energy $W = E - mc^2$. So $\gamma$ has an intuitive meaning: **how many times the rest energy the total energy is**. Hence:

$$
\gamma = 1 + \frac{W}{mc^2}, \qquad
\beta = \frac{v}{c} = \sqrt{1 - \frac{1}{\gamma^2}}, \qquad
\beta\gamma = \sqrt{\gamma^2 - 1}
$$

The momentum is $p = \gamma m v$; multiplied by $c$ it gives $pc = \beta\gamma\,mc^2$. Multiplying $\gamma^2 - \beta^2\gamma^2 = 1$ by $(mc^2)^2$ gives the energy–momentum relation:

$$
E = \gamma mc^2 = W + mc^2, \qquad
pc = \beta\gamma\, mc^2 = \sqrt{W\left(W + 2mc^2\right)}, \qquad
E^2 = (pc)^2 + (mc^2)^2
$$

All of these are exact. For the example beam (protons, $W$ = 1.527 MeV): $\gamma = 1.001627$, $\beta = 0.05698$, $\beta\gamma = 0.05708$, $p$ = 53.55 MeV/$c$. $\gamma$ exceeds 1 by only 0.16 %, but the combination $\beta\gamma$ runs through the whole primer: it links geometric and normalized emittance, and it appears as $(\beta\gamma)^3$ in the space-charge force.

Accelerator physicists use MeV for energy and MeV/$c$ for momentum, so that $p\,[\mathrm{MeV}/c] = \beta\gamma\, mc^2\,[\mathrm{MeV}]$. Common rest energies:

| Particle | $mc^2$ (MeV) |
|---|---|
| electron | 0.51099895 |
| proton | 938.272088 |
| atomic mass unit u | 931.494102 |

> Note: for a heavy ion use the mass of the ion itself, i.e. the atomic mass minus the mass of the stripped electrons (binding energies are usually negligible); it is **not** $A \times 938.272$ MeV.

Drag the kinetic energy $W$ in the lab and watch how $\beta$, $\beta\gamma$, $B\rho$, $E\rho$ and $\beta\lambda$ change, and how the $\beta(W)$ curve bends from the $\sqrt W$ rise of low energies towards 1:

```avas-lab
kinematics mass=938.272 q=1 W=1.527 f=162.5
```

> **Think about it**: double the kinetic energy of a low-energy proton. How much faster does it go?
>
> Answer: non-relativistically $\beta \propto \sqrt W$, so the speed grows only by $\sqrt2 = 1.414$. For the example proton going from 1.527 MeV to 3.054 MeV the exact factor is 1.412. Each increase of energy buys less increase of speed — the root of every "velocity saturation" effect later on.

## Non-relativistic and ultra-relativistic limits {#kinematics-nonrel}

Write $u = W/(mc^2)$. Each limit has its own simple formulas, and knowing where they fail matters just as much.

### Non-relativistic limit ($u \ll 1$)

$$
W \approx \tfrac12 mv^2, \qquad \beta \approx \sqrt{2u}, \qquad pc \approx \sqrt{2\,mc^2\,W}
$$

The exact expansion is $\beta = \sqrt{2u}\,\left(1 - \tfrac34 u + \dots\right)$, so the non-relativistic formula has a relative error of about $3u/4$. For 1.527 MeV protons $u = 1.63\times10^{-3}$; the non-relativistic $\beta = 0.057052$, the exact value $0.056983$, an error of 0.12 %. That answers the opening question: the error is small, but it accumulates — over a 0.21 m cavity a 0.12 % velocity error shifts the arrival time by about 15 ps, 0.9° of RF phase, already comparable to the 1° level one usually cares about in RF phases. For heavy ions and low-energy protons the non-relativistic formulas are fine for estimates, but the AVAS linear preview uses the exact formulas (the manual does not discuss the kernel's relativistic treatment separately; one expects exact formulas there too).

### Ultra-relativistic limit ($\gamma \gg 1$)

$$
\beta \approx 1 - \frac{1}{2\gamma^2}, \qquad pc \approx E \approx W
$$

A 1 MeV electron has $\gamma = 2.96$ and $\beta = 0.941$; the non-relativistic formula would give $\beta \approx 1.98$ and fails completely. Electron beams must therefore always be treated relativistically. Set the lab mass to 0.511 and watch $\beta$ approach 1 quickly above 1 MeV and $\beta\lambda$ approach $\lambda$:

```avas-lab
kinematics mass=0.511 q=1 W=1 f=2856
```

## How small deviations propagate: energy, momentum and velocity spread {#kinematics-differential}

The particles of a beam have slightly different energies. Push a particle a little harder so that its momentum is a little larger — how much faster does it go? Differentiate the relations above:

$$
\dd\gamma = \frac{\dd W}{mc^2}, \qquad
\frac{\dd\beta}{\beta} = \frac{\dd\gamma}{\beta^2\gamma^3} = \frac{1}{\gamma^2}\,\frac{\dd p}{p}, \qquad
\frac{\dd p}{p} = \frac{\dd\gamma}{\beta^2\gamma}
$$

The middle relation carries the key physics: **the velocity deviation is $1/\gamma^2$ times the momentum deviation**. At low energy a bit more momentum means a bit more speed; at high energy the momentum keeps growing while the speed hardly moves away from $c$. With the relative momentum deviation $\delta = \Delta p / p$ (one of AVAS's longitudinal coordinates, see [Phase space](#phase-space-coordinates)):

$$
\Delta W = \beta^2\gamma\, mc^2\,\delta = v\,\Delta p, \qquad
\frac{\Delta W}{W} = \frac{\gamma + 1}{\gamma}\,\delta
$$

Non-relativistically $\Delta W / W \approx 2\delta$; in the relativistic limit $\Delta W / W \to \delta$. For the example beam $1/\gamma^2 = 0.99675$ and $\Delta W/W = 1.998\,\delta$; its $\delta_{\mathrm{rms}} = 3.53\times10^{-3}$ corresponds to an rms energy spread of 10.78 keV (note: from this chapter onwards, the longitudinal examples in Fundamentals and Components use the `twissz` values in `beam.txt`, giving an rms bunch length of 1.316 mm and energy spread of 10.78 keV, whereas Part 1 quoted the direct statistics of `part_rfq.dst`, 1.24 mm and 11.4 keV; the slight difference comes from the $\beta_z$ entry in the example's `beam.txt`, see [Initial distributions: In AVAS](#distributions-in-avas)).

The most direct consequence of the velocity spread is that **bunches lengthen in drifts**: a particle with momentum deviation $\delta$ gains $L\delta/\gamma^2$ over a distance $L$. This is why the $z$–$\delta$ element of the drift matrix in [Linear optics](#linear-optics) is $L/\gamma^2$.

> **Think about it**: how long is the example bunch (rms length 1.316 mm) after a 1 m drift without RF cavities? And for 10 MeV electrons?
>
> Answer: pushing the σ matrix with the linear map $z_1 = z_0 + (L/\gamma^2)\,\delta$ gives an rms length of 3.71 mm after 1 m, almost three times the original — the energy spread alone contributes $L\,\delta_{\mathrm{rms}}/\gamma^2$ = 3.52 mm. That is why low-energy sections need bunching cavities at short intervals to "pull the bunch back" (see [The medium-energy beam transport](#journey-mebt)). For 10 MeV electrons $1/\gamma^2 = 2.4\times10^{-3}$, so the same $\delta$ lengthens the bunch only about 1/420 as much as for the protons. This is why electrons above a few MeV have almost no longitudinal phase slip while low-energy protons slip strongly (see [Longitudinal motion](#longitudinal)).

## Magnetic and electric rigidity {#kinematics-rigidity}

### Magnetic rigidity: how hard it is to bend a particle

A particle in a uniform magnetic field $B$ perpendicular to its velocity moves on a circle, the Lorentz force providing the centripetal force: $qvB = \gamma m v^2/\rho = pv/\rho$. Cancel $v$ on both sides:

$$
B\rho = \frac{p}{q}
$$

$B\rho$ is the **magnetic rigidity**: the field times radius needed to bend the particle on a radius $\rho$. It depends only on the ratio of momentum to charge — more momentum makes it "stiffer", more charge "softer". The velocity drops out because both the magnetic force $qvB$ and the force needed for the curve, $pv/\rho$, are proportional to $v$. In practical units:

$$
\Brho\,[\mathrm{T\cdot m}] = \frac{p\,[\mathrm{MeV}/c]}{299.792458\,\lvert Z \rvert} = 3.3356\,\frac{p\,[\mathrm{GeV}/c]}{\lvert Z \rvert}
$$

The example proton has $B\rho$ = 0.1786 T·m: a 1 T field bends it on a radius of 0.179 m, 0.18 T on 0.992 m. The strengths of all magnetic elements are normalized by $B\rho$:

- dipole bending radius $\rho = B\rho / B$ (see [Dipole](#dipole));
- quadrupole focusing strength $k = G/(B\rho)$ in m$^{-2}$ (see [Quadrupole](#quadrupole));
- solenoid $k = B/(2B\rho)$ in m$^{-1}$ (see [Solenoid](#solenoid)).

The same magnet therefore has a different strength for beams of different energy or charge-to-mass ratio; after changing the beam energy, the fields must be scaled in proportion to $B\rho$ to keep the same optics.

### Electric rigidity

In a transverse electric field $E$ we have $qE = pv/\rho$ — this time $v$ does not drop out, because the electric force does not depend on the velocity. Define the **electric rigidity**

$$
E\rho = \frac{pv}{q} = \beta c\,(B\rho), \qquad
E\rho\,[\mathrm{MV}] = \frac{\beta\,pc\,[\mathrm{MeV}]}{\lvert Z \rvert}
$$

Non-relativistically $pv \approx 2W$, i.e. $E\rho \approx 2W/Z$ ($E\rho$ in MV for $W$ in MeV). The bending radius of an electrostatic element is $\rho = E\rho/E$, and the strength of an electrostatic quadrupole is $k = (\partial E_x/\partial x)/(E\rho)$. The example proton has $E\rho$ = 3.052 MV; a transverse field of 5 MV/m bends it on $\rho$ = 0.610 m.

Now the opening question can be answered: non-relativistically $B\rho \propto p \propto \sqrt W$ while $E\rho \propto 2W$. Raising the energy tenfold (1.527 → 15.27 MeV) increases $B\rho$ by a factor 3.17 and $E\rho$ by 9.93. Put differently: for the same bending, the electric field has to equal $v$ times the magnetic field. For the example proton 1 T of magnetic field acts like 17.1 MV/m of electric field; for a particle with $\beta \to 1$ it acts like 300 MV/m — far beyond any electrostatic field that can be sustained in practice. Electrostatic elements are therefore effective only at low velocity ($E\rho \propto \beta^2$), and at high energy magnetic elements are far more economical; the low-energy RFQ exploits exactly the fact that electric focusing does not weaken with velocity (see [The RFQ](#journey-rfq)). Electrostatic elements are covered in [Static fields](#static-fields).

Set $W$ to 15.27 MeV in the lab and compare the change of $B\rho$ and $E\rho$:

```avas-lab
kinematics mass=938.272 q=1 W=15.27 f=162.5
```

> Note: the sign of the rigidity follows the sign of the charge. The practical formulas above use $\lvert Z \rvert$; for negative particles the same magnetic field pushes the other way, and the focusing planes of a quadrupole are swapped.

## RF wavelength and βλ {#kinematics-rf}

An RF frequency $f$ corresponds to the free-space wavelength and period

$$
\lambda = \frac{c}{f}, \qquad T_{\mathrm{RF}} = \frac{1}{f}
$$

The RF field changes sign every half period. In one RF period a particle travels $\beta\lambda$ — the basic unit of longitudinal length in a linac. Picture a row of accelerating gaps: a particle that catches the accelerating half-period in the first gap must also be accelerated in the next one, so the distance between gaps has to "keep time" with the particle's speed:

- in π-mode structures neighbouring gaps have opposite fields and are $\beta\lambda/2$ apart; in 0-mode structures (DTL type) neighbouring gaps have the same field direction and are $\beta\lambda$ apart;
- converting longitudinal position to phase: a particle $\Delta z$ ahead of the reference particle reaches the gap earlier and sees a smaller RF phase, $\Delta\varphi = -2\pi\,\Delta z/(\beta\lambda)$;
- the transit-time factor varies with $\beta$, so a cavity accelerates efficiently only over a certain range of $\beta$ (see [RF cavity](#rf-cavity)).

Example: at 162.5 MHz $\lambda = 1.8449$ m and $T_{\mathrm{RF}}$ = 6.154 ns. For 1.527 MeV protons $\beta\lambda$ = 0.10513 m, so 1° of RF phase corresponds to 0.292 mm. Crossing the 0.21 m HWR field map at its initial speed takes the reference particle about $0.21/(\beta c)$ = 12.29 ns, about 2.0 RF periods.

> **Think about it**: why can't one type of 162.5 MHz cavity accelerate protons all the way from the 35 keV of the ion source to 1 GeV?
>
> Answer: $\beta\lambda$ grows from 15.9 mm at 35 keV to 1.614 m at 1 GeV, about a factor 100 (drag $W$ to both ends in the lab to see it). A cavity with fixed gap spacing "keeps time" with one design velocity only; away from it, particles reach the later gaps at the wrong phase and the effective acceleration falls with the transit-time factor. That is why a linac is divided into sections, each with cavities designed for a certain $\beta$; see [From ion source to target](#journey-overview).

## Tables: from protons to electrons {#kinematics-examples}

The three tables below are computed with the exact formulas of this chapter, to build a feeling for the numbers. Three things are worth noticing: from 1.527 MeV to 1 GeV the proton $\beta$ grows 15-fold but $B\rho$ 32-fold; above 1 MeV the electron $\beta\lambda$ is already almost $\lambda$; the electric rigidity $E\rho = \beta pc$ is close to $2W$ at low energy and tends to $pc$ at high energy.

### Protons ($mc^2$ = 938.272 MeV, $\beta\lambda$ at 162.5 MHz)

| $W$ (MeV) | $\gamma$ | $\beta$ | $\beta\gamma$ | $p$ (MeV/$c$) | $B\rho$ (T·m) | $E\rho$ (MV) | $\beta\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 1.527 | 1.001627 | 0.056983 | 0.057076 | 53.55 | 0.17863 | 3.052 | 0.10513 |
| 10 | 1.010658 | 0.144844 | 0.146388 | 137.35 | 0.45816 | 19.89 | 0.26722 |
| 100 | 1.106579 | 0.428195 | 0.473832 | 444.58 | 1.48297 | 190.4 | 0.78997 |
| 1000 | 2.065789 | 0.875026 | 1.807618 | 1696.04 | 5.65737 | 1484.1 | 1.61431 |

The first row is the beam of the example project.

### Electrons ($mc^2$ = 0.511 MeV, $\beta\lambda$ at 2856 MHz, $\lambda$ = 0.10497 m)

| $W$ (MeV) | $\gamma$ | $\beta$ | $\beta\gamma$ | $p$ (MeV/$c$) | $B\rho$ (T·m) | $E\rho$ (MV) | $\beta\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 0.1 | 1.195695 | 0.548221 | 0.655505 | 0.3350 | 0.001117 | 0.1836 | 0.05755 |
| 1 | 2.956951 | 0.941079 | 2.782725 | 1.4220 | 0.004743 | 1.338 | 0.09878 |
| 10 | 20.56951 | 0.998818 | 20.54519 | 10.499 | 0.03502 | 10.49 | 0.10485 |
| 100 | 196.6951 | 0.999987 | 196.6926 | 100.51 | 0.33526 | 100.51 | 0.10497 |

### Heavy ions (when the energy is given per nucleon)

Take $^{40}$Ar$^{8+}$ at 0.5 MeV per nucleon: the ion's rest energy is about $39.9624 \times 931.494 - 8\times0.511$ = 37220.6 MeV and the total kinetic energy $W = 40\times0.5$ = 20 MeV, giving $\beta$ = 0.03277, $p$ = 1220.3 MeV/$c$ and $B\rho = p/(299.79\times8)$ = 0.5088 T·m. Compared with the 1.527 MeV proton it is only about 58 % as fast but 2.85 times as rigid: the smaller the charge-to-mass ratio $Z/A$, the larger the rigidity at the same speed, and the harder the focusing.

## Common misconceptions {#kinematics-misconceptions}

- **"Kinetic energy, total energy and energy per nucleon are the same thing."** $W$ is the kinetic energy and $E = W + mc^2$ the total energy; heavy-ion energies are often quoted per nucleon and must be multiplied by the number of nucleons for the whole ion. AVAS's `kneticenergy` is the kinetic energy of the whole particle.
- **"Momentum in MeV/$c$ and energy in MeV are interchangeable."** Only in the ultra-relativistic limit is $pc \approx W$; the example proton has $pc$ = 53.55 MeV but a kinetic energy of only 1.527 MeV.
- **"A heavy ion's mass is $A$ times the proton mass."** Count in atomic mass units u = 931.494 MeV and subtract the stripped electrons; computing $^{40}$Ar as $40\times938.272$ comes out 0.8 % too high.
- **"Non-relativistic formulas are fine everywhere at the MeV scale."** For protons the error is about $3u/4$, fine for estimates; for electrons they fail already at 100 keV.
- **"$\beta\lambda$ is a fixed length."** It changes with energy and tends to $\lambda$ only as $\beta \to 1$.
- **"Rigidity has no sign."** The practical formulas use $\lvert Z \rvert$, but the direction of the force follows the sign of the charge; for negative particles the focusing planes of a quadrupole are swapped.
- **"A given quadrupole focuses equally at any energy."** $k = G/(B\rho)$; change the energy and $k$ changes.

## In AVAS {#kinematics-in-avas}

### Keywords

The kinematics-related keywords of `beam.txt`:

```avas-params
beam:particlerestmass beam:numofcharge beam:kneticenergy beam:frequency beam:current
```

- `particlerestmass`: rest energy $mc^2$ in MeV (938.272088159928 in the example project, i.e. protons).
- `numofcharge`: charge number $Z$ in units of $e$. It enters both the rigidity $B\rho = p/(Ze)$ and the space charge (the charge per particle).
- `kneticenergy`: kinetic energy $W$ of the reference particle in MeV (spelled exactly `kneticenergy`; it is not in the manual's table and is written by the Beam page).
- `frequency`: beam frequency in **Hz** (`162500000.0` in the example), used to convert longitudinal position into phase and to compute the bunch charge $Q = I/f$.
- `current`: beam current in mA.

> **AVAS convention**: the interface's linear preview computes $\gamma = 1 + W/m$; `kneticenergy` and `particlerestmass` are quantities of the **whole particle** (not per nucleon). For heavy ions enter the total mass and total kinetic energy of the ion; the $^{40}$Ar$^{8+}$ above becomes `particlerestmass 37220.6`, `kneticenergy 20`, `numofcharge 8`.

> Note: the `frequency` of `beam.txt` and the frequency parameter `f` of `field` elements in the lattice are both in Hz; the `f` of this chapter's lab is in MHz, only to make dragging convenient. Negatively charged particles (such as electrons) should by definition get a negative charge number; the manual does not say how the kernel handles negative charges, so check the focusing direction of a quadrupole with a simple lattice first.

### Bunch charge and macro-particles

The bunch charge of the example project is $Q = 0.1841\,\mathrm{mA} / 162.5\,\mathrm{MHz}$ = 1.133 pC, about $7.07\times10^6$ protons; with 5260 macro-particles each macro-particle stands for about 1344 protons. What macro-particles mean is explained in [Initial distributions](#distributions).

### Kinematic quantities in the output

Column 0 of `DataSet.txt` is the kinetic energy $W$ of the reference particle (MeV); with this chapter's formulas you can compute $\beta\gamma$ at every position and from it the geometric emittance from the normalized one (see [Phase space](#phase-space-emittance)).

### Lab parameters

The `kinematics` lab takes `mass` (MeV), `q` (charge number), `W` (MeV) and `f` (MHz). Suggested checks: start from the example proton beam and confirm the table values; raise $W$ tenfold (15.27 MeV) and see $B\rho$ grow by about 3.17 and $E\rho$ by about 9.93; keep the speed fixed while changing the charge number and see the rigidity scale like $1/Z$.

## Further reading {#kinematics-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: relativistic relations, $\beta\lambda$ and the length scales of accelerating structures.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific: magnetic rigidity and practical units.
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015.
- Continue with: [Phase space and emittance](#phase-space).
