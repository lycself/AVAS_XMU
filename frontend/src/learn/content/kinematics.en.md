# Relativistic kinematics

Every "strength" in a linac — the focusing strength of a quadrupole, the bending radius of a dipole, the phase slip in an RF cavity — is tied to the beam energy through a few kinematic quantities: the Lorentz factor $\gamma$, the velocity $\beta$, the momentum $p$, the magnetic rigidity $B\rho$ and $\beta\lambda$. This chapter gives their definitions, relations and numerical examples, and shows how they correspond to the keywords of `beam.txt`.

## Basic quantities {#kinematics-variables}

Let the particle have rest energy $mc^2$, charge $q = Ze$ ($Z$ is the charge number, possibly negative) and kinetic energy $W$. The following relations of special relativity are exact:

$$
\gamma = 1 + \frac{W}{mc^2}, \qquad
\beta = \frac{v}{c} = \sqrt{1 - \frac{1}{\gamma^2}}, \qquad
\beta\gamma = \sqrt{\gamma^2 - 1}
$$

Total energy and momentum are

$$
E = \gamma mc^2 = W + mc^2, \qquad
pc = \beta\gamma\, mc^2 = \sqrt{W\left(W + 2mc^2\right)}, \qquad
E^2 = (pc)^2 + (mc^2)^2
$$

In accelerators energies are usually given in MeV and momenta in MeV/$c$, so $p\,[\mathrm{MeV}/c] = \beta\gamma\, mc^2\,[\mathrm{MeV}]$. Common rest energies:

| Particle | $mc^2$ (MeV) |
|---|---|
| electron | 0.51099895 |
| proton | 938.272088 |
| atomic mass unit u | 931.494102 |

> Note: for heavy ions use the rest energy of the ion itself, i.e. the atomic mass minus the mass of the stripped electrons (binding energies can usually be neglected); it is **not** $A \times 938.272$ MeV.

## Differential relations and energy spread {#kinematics-differential}

The particles of a beam have slightly different energies, and energy deviations must be converted into velocity and momentum deviations. Differentiating the relations above:

$$
\dd\gamma = \frac{\dd W}{mc^2}, \qquad
\frac{\dd\beta}{\beta} = \frac{\dd\gamma}{\beta^2\gamma^3} = \frac{1}{\gamma^2}\,\frac{\dd p}{p}, \qquad
\frac{\dd p}{p} = \frac{\dd\gamma}{\beta^2\gamma}
$$

With the relative momentum deviation $\delta = \Delta p / p$ (one of AVAS's longitudinal coordinates, see [Phase space](#phase-space-coordinates)):

$$
\Delta W = \beta^2\gamma\, mc^2\,\delta = v\,\Delta p, \qquad
\frac{\Delta W}{W} = \frac{\gamma + 1}{\gamma}\,\delta
$$

Some consequences:

- Non-relativistically $\Delta W / W \approx 2\delta$; in the relativistic limit $\Delta W / W \to \delta$.
- The velocity deviation is $\Delta\beta/\beta = \delta/\gamma^2$: at low energy the velocity is very sensitive to momentum deviations, at high energy it hardly changes. This is why the $z$–$\delta$ element of a drift in [linear optics](#linear-optics-6d) is $L/\gamma^2$, and why electrons above a few MeV show almost no longitudinal phase slip while low-energy protons slip strongly (see [Longitudinal motion](#longitudinal)).
- For the example beam (protons, $W$ = 1.527 MeV): $1/\gamma^2 = 0.9968$ and $\Delta W/W = 1.998\,\delta$.

## Non-relativistic and ultra-relativistic limits {#kinematics-nonrel}

Write $u = W/(mc^2)$.

### Non-relativistic limit ($u \ll 1$)

$$
W \approx \tfrac12 mv^2, \qquad \beta \approx \sqrt{2u}, \qquad pc \approx \sqrt{2\,mc^2\,W}
$$

The exact expansion is $\beta = \sqrt{2u}\,\left(1 - \tfrac34 u + \dots\right)$, so the relative error of the non-relativistic formula is about $3u/4$. For 1.527 MeV protons $u = 1.63\times10^{-3}$: the non-relativistic $\beta = 0.057052$, the exact value $0.056983$, an error of 0.12 %. Heavy ions and low-energy proton sections can be estimated with the non-relativistic formulas, but the AVAS kernel and the linear preview always use the exact ones.

### Ultra-relativistic limit ($\gamma \gg 1$)

$$
\beta \approx 1 - \frac{1}{2\gamma^2}, \qquad pc \approx E \approx W
$$

At 1 MeV an electron has $\gamma = 2.96$ and $\beta = 0.941$; the non-relativistic formula would give $\beta \approx 1.98$ and fails completely. Electron beams must always be treated relativistically.

## Magnetic and electric rigidity {#kinematics-rigidity}

### Magnetic rigidity

A particle in a uniform magnetic field $B$ perpendicular to its velocity moves on a circle; the Lorentz force provides the centripetal force, $qvB = \gamma m v^2/\rho = pv/\rho$, hence

$$
B\rho = \frac{p}{q}
$$

$B\rho$ is the **magnetic rigidity**; it depends only on the ratio of momentum to charge. In practical units:

$$
\Brho\,[\mathrm{T\cdot m}] = \frac{p\,[\mathrm{MeV}/c]}{299.792458\,\lvert Z \rvert} = 3.3356\,\frac{p\,[\mathrm{GeV}/c]}{\lvert Z \rvert}
$$

The strengths of all magnetic elements are normalised by $B\rho$:

- dipole bending radius $\rho = B\rho / B$ (see [Dipole](#dipole));
- quadrupole focusing strength $k = G/(B\rho)$ in m$^{-2}$ (see [Quadrupole](#quadrupole));
- solenoid $k = B/(2B\rho)$ in m$^{-1}$ (see [Solenoid](#solenoid)).

The same magnet therefore acts differently on beams of different energy or charge-to-mass ratio; after changing the beam energy the fields must be scaled in proportion to $B\rho$ to keep the same optics.

### Electric rigidity

In a transverse electric field $E$, $qE = pv/\rho$, which defines the **electric rigidity**

$$
E\rho = \frac{pv}{q} = \beta c\,(B\rho), \qquad
E\rho\,[\mathrm{MV}] = \frac{\beta\,pc\,[\mathrm{MeV}]}{\lvert Z \rvert}
$$

Non-relativistically $pv \approx 2W$, i.e. $E\rho \approx 2W/Z$ ($E\rho$ in MV for $W$ in MeV). An electrostatic element bends with radius $\rho = E\rho/E$, and an electrostatic quadrupole has strength $k = (\partial E_x/\partial x)/(E\rho)$. Electrostatic elements are thus effective for slow particles ($E\rho \propto \beta^2$), while at high energy magnetic elements are far more economical. Electrostatic field elements are covered in [Static fields](#static-fields).

> Note: the sign of the rigidity follows the sign of the charge. The practical formulas above use $\lvert Z \rvert$; for a negative particle the same magnetic field produces the opposite force, and the focusing planes of a quadrupole are swapped.

## RF wavelength and βλ {#kinematics-rf}

An RF frequency $f$ corresponds to the free-space wavelength and period

$$
\lambda = \frac{c}{f}, \qquad T_{\mathrm{RF}} = \frac{1}{f}
$$

In one RF period the particle travels a distance $\beta\lambda$. This is the basic longitudinal length scale of a linac:

- gap spacing: in a π-mode structure neighbouring gaps are $\beta\lambda/2$ apart, in a 0-mode structure (DTL type) $\beta\lambda$;
- conversion between longitudinal position and phase: a particle a distance $\Delta z$ ahead of the reference particle reaches an accelerating gap earlier and sees a smaller RF phase, $\Delta\varphi = -2\pi\,\Delta z/(\beta\lambda)$;
- the transit-time factor depends on $\beta$, so a cavity accelerates efficiently only within a certain range of $\beta$ (see [RF cavity](#rf-cavity)).

Example: at 162.5 MHz, $\lambda = 1.8449$ m and $T_{\mathrm{RF}}$ = 6.154 ns. For 1.527 MeV protons $\beta\lambda$ = 0.10513 m, i.e. 1° of RF phase corresponds to 0.292 mm. Crossing the 0.21 m HWR field map at the initial velocity takes the reference particle about $0.21/(\beta c)$ = 12.29 ns, about 2.0 RF periods.

## Numerical examples {#kinematics-examples}

### Protons ($mc^2$ = 938.272 MeV, $\beta\lambda$ at 162.5 MHz)

| $W$ (MeV) | $\gamma$ | $\beta$ | $\beta\gamma$ | $p$ (MeV/$c$) | $B\rho$ (T·m) | $E\rho$ (MV) | $\beta\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 1.527 | 1.001627 | 0.056983 | 0.057076 | 53.55 | 0.17863 | 3.052 | 0.10513 |
| 10 | 1.010658 | 0.144844 | 0.146388 | 137.35 | 0.45816 | 19.89 | 0.26722 |
| 100 | 1.106579 | 0.428195 | 0.473832 | 444.58 | 1.48297 | 190.4 | 0.78997 |
| 1000 | 2.065789 | 0.875026 | 1.807618 | 1696.04 | 5.65737 | 1484.1 | 1.61431 |

The first row is the beam of the example project: a 0.18 T dipole field bends it with $\rho$ = 0.992 m; a 5 MV/m transverse electric field corresponds to $\rho$ = 0.610 m.

### Electrons ($mc^2$ = 0.511 MeV, $\beta\lambda$ at 2856 MHz, $\lambda$ = 0.10497 m)

| $W$ (MeV) | $\gamma$ | $\beta$ | $\beta\gamma$ | $p$ (MeV/$c$) | $B\rho$ (T·m) | $E\rho$ (MV) | $\beta\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 0.1 | 1.195695 | 0.548221 | 0.655505 | 0.3350 | 0.001117 | 0.1836 | 0.05755 |
| 1 | 2.956951 | 0.941079 | 2.782725 | 1.4220 | 0.004743 | 1.338 | 0.09878 |
| 10 | 20.56951 | 0.998818 | 20.54519 | 10.499 | 0.03502 | 10.49 | 0.10485 |
| 100 | 196.6951 | 0.999987 | 196.6926 | 100.51 | 0.33526 | 100.51 | 0.10497 |

### Heavy ions (energy given per nucleon)

Take $^{40}$Ar$^{8+}$ at 0.5 MeV per nucleon: the ion rest energy is about $39.9624 \times 931.494 - 8\times0.511$ = 37220.6 MeV and the total kinetic energy $W = 40\times0.5$ = 20 MeV, so $\beta$ = 0.03277, $p$ = 1220.3 MeV/$c$ and $B\rho = p/(299.79\times8)$ = 0.5088 T·m. The smaller the charge-to-mass ratio $Z/A$, the larger the rigidity at a given velocity and the harder the beam is to focus.

## In AVAS {#kinematics-in-avas}

The kinematic keywords of `beam.txt`:

```avas-params
beam:particlerestmass beam:numofcharge beam:kneticenergy beam:frequency beam:current
```

- `particlerestmass`: rest energy $mc^2$ in MeV (938.272088159928 in the example project, i.e. a proton).
- `numofcharge`: charge number $Z$ in units of $e$. It enters both the rigidity $B\rho = p/(Ze)$ and space charge (the charge of each particle).
- `kneticenergy`: kinetic energy $W$ of the reference particle in MeV (spelled exactly `kneticenergy`; not in the manual's table, written by the Beam page).
- `frequency`: beam frequency in **Hz** (`162500000.0` in the example), used to convert longitudinal position into phase and to fix the bunch charge $Q = I/f$.
- `current`: beam current in mA.

> AVAS convention: the linear preview of the interface computes $\gamma = 1 + W/m$, so `kneticenergy` and `particlerestmass` refer to the **whole particle** (not per nucleon). For heavy ions write the total mass and total kinetic energy of the ion, e.g. for the $^{40}$Ar$^{8+}$ above `particlerestmass 37220.6`, `kneticenergy 20`, `numofcharge 8`.

> Note: both `frequency` in `beam.txt` and the frequency parameter `f` of the `field` element in the lattice are in Hz; the `f` of this chapter's lab is in MHz only for convenient dragging. A negatively charged particle (e.g. an electron) should by definition be given a negative charge number; the manual does not describe how the kernel handles negative charges, so check the focusing direction of a quadrupole with a simple lattice before relying on it.

The bunch charge of the example project is $Q = 0.1841\,\mathrm{mA} / 162.5\,\mathrm{MHz}$ = 1.133 pC, about $7.07\times10^6$ protons; with 5260 macro-particles each macro-particle stands for about 1340 protons.

Kinematics in the output: column 0 of `DataSet.txt` is the kinetic energy $W$ of the reference particle (MeV); with this chapter's formulas you can compute $\beta\gamma$ at every position and from it the geometric emittance from the normalised one (see [Phase space](#phase-space-emittance)).

## Hands-on lab {#kinematics-lab}

Drag the kinetic energy $W$ and watch $\beta$, $\beta\gamma$, $B\rho$, $E\rho$, $\beta\lambda$ and the $\beta(W)$ curve. Suggestions:

1. Start from the example proton beam and confirm the numbers in the table; raise $W$ to 10 times its value (15.27 MeV): $B\rho$ grows only to about 3.17 times (close to $\sqrt{10}$), while $E\rho$ grows to about 9.93 times (close to 10).
2. Change the mass to 0.511 (electrons) and watch $\beta$ approach 1 quickly above 1 MeV and $\beta\lambda$ approach $\lambda$.
3. Keep the velocity fixed and change the charge number: the rigidity scales as $1/Z$.

```avas-lab
kinematics mass=938.272 q=1 W=1.527 f=162.5
```

Electron example (2856 MHz):

```avas-lab
kinematics mass=0.511 q=1 W=1 f=2856
```

## Further reading {#kinematics-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: relativistic relations, $\beta\lambda$ and the length scales of accelerating structures.
- S. Y. Lee, *Accelerator Physics*, 4th ed.: magnetic rigidity and practical units.
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015.
- Continue with [Phase space and emittance](#phase-space).
