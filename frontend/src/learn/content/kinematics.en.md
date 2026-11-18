# Relativistic kinematics

How fast is the proton of the example project? Its kinetic energy is 1.527 MeV; it flies 17 000 kilometres per second and would get from the Earth to the Moon in 22 and a half seconds. Yet that is only 5.7 % of the speed of light.

If we keep giving it energy, can it become faster than light? By the school formula $W = \tfrac12 mv^2$, a proton with 1000 MeV of kinetic energy would fly at 1.46 times the speed of light. Experiments say no: however much energy is added, the speed only gets closer and closer to the speed of light and never reaches it. Where does the energy go?

There is a more practical puzzle too: raise the proton energy tenfold, and the magnet that bends it needs to be only about 3.2 times stronger, while an electrostatic element that bends it has to be almost 10 times stronger. Why?

This chapter starts from the velocity, momentum and kinetic energy of school mechanics and explains where the most basic quantities of an accelerator come from and what each of them governs:

- **the Lorentz factor $\gamma$ and the velocity $\beta$**: how fast the particle is;
- **the momentum $p$**: how much "punch" the particle has;
- **the magnetic rigidity $B\rho$**: how hard the particle is to bend with a magnetic field; it decides how strong every magnet must be;
- **$\beta\lambda$**: how far the particle flies while the RF field oscillates once; it decides the size of the accelerating cavities.

First, try it. The lab below computes all these quantities for the example proton; drag "kinetic energy W" and watch the $\beta(W)$ curve on the right: at low energy it rises quickly, then flattens more and more and bends towards 1 (the speed of light) without ever reaching it.

```avas-lab
kinematics mass=938.272 q=1 W=1.527 f=162.5
```

With the default values the lab gives $\beta = 0.0570$ (5.7 % of the speed of light), a magnetic rigidity $B\rho = 0.179$ T·m and $\beta\lambda = 105$ mm. These numbers are explained one by one below.

## Starting from school mechanics: velocity, momentum, kinetic energy {#kinematics-school}

School mechanics describes a moving body with three quantities:

- **velocity** $v$: how far it goes per second;
- **momentum** $p = mv$: mass times velocity, a measure of how much "punch" the body has, how hard it is to stop when it hits something;
- **kinetic energy** $W = \tfrac12 mv^2$: how much work it takes to speed it up.

Accelerator energies are usually given in **electronvolts** (eV): an electron (or proton) carries the charge $e = 1.602\times10^{-19}$ C, and the energy it gains when it flies through a voltage of 1 V is 1 eV $= 1.602\times10^{-19}$ J. 1 MeV is a million eV. So "a 1.527 MeV proton" means: its kinetic energy equals what it would gain by flying through 1.527 million volts.

Let us work out the example proton with the school formulas. The kinetic energy is $W = 1.527\times10^{6} \times 1.602\times10^{-19}$ J $= 2.447\times10^{-13}$ J, the proton mass $m = 1.673\times10^{-27}$ kg:

$$
v = \sqrt{\frac{2W}{m}} = \sqrt{\frac{2 \times 2.447\times10^{-13}}{1.673\times10^{-27}}}\ \text{m/s} = 1.710\times10^{7}\ \text{m/s},
$$

which is 5.705 % of the speed of light $c = 3.00\times10^{8}$ m/s. The momentum is $p = mv = 2.861\times10^{-20}$ kg·m/s. Crossing the 0.21 m long RF cavity of the example takes about 12.3 ns (billionths of a second).

Are these numbers accurate? Nearly. The exact relativistic formulas give 5.698 %, only 0.12 % less. But the same formulas are completely wrong at higher energies, as the next section shows.

## There is a speed limit: why we need relativity {#kinematics-why-relativity}

### What happens if we keep pushing

By $W = \tfrac12 mv^2$, four times the kinetic energy means twice the speed, and pushing on would make the speed grow without limit. List protons of various energies and compare with the speeds actually measured:

| Proton kinetic energy | Speed from Newton's formula | Actual speed |
|---|---|---|
| 0.035 MeV (ion source exit) | 0.86 % of light | 0.86 % of light |
| 1.527 MeV (example) | 5.71 % | 5.70 % |
| 10 MeV | 14.6 % | 14.5 % |
| 100 MeV | 46.2 % | 42.8 % |
| 1000 MeV | **146 %** | 87.5 % |

At low energy the two agree almost exactly; at 100 MeV they differ by 8 %; at 1000 MeV Newton's formula gives a speed above the speed of light, which is impossible. Electrons are lighter and the trouble comes earlier: by Newton's formula a 1 MeV electron would fly at 1.98 times the speed of light; in reality it flies at 0.94 times. In 1964 W. Bertozzi accelerated electrons to 0.5–15 MeV and timed their flight over 8.4 m: the energy grew 30-fold, but the measured speed rose only from about 0.87 c to c itself (within the roughly 10 % measurement error), as relativity predicts (86 % to 99.95 % of c), and never exceeded it (by Newton's formula a 15 MeV electron would fly at 7.7 times the speed of light).

### Where the energy goes

Special relativity says that **the speed of light $c$ is the upper limit for the speed of any body**. The closer a body gets to the speed of light, the harder it becomes to make it faster: the pushing force still does work and the energy still grows, but the speed hardly grows any more.

An analogy that helps: it is like pushing a cart that gets "heavier" the faster it goes. The analogy is right about "harder and harder to speed up"; but physicists do not say that the mass changes, they say that the **relation of energy and momentum to velocity** changes: energy and momentum can grow without limit, but the speed has an upper limit.

### Rest energy and the Lorentz factor γ

You know Einstein's mass–energy relation $E = mc^2$ from school: mass itself is a form of energy, called the **rest energy**. The rest energy of a proton is 938.27 MeV, that of an electron 0.511 MeV. The example proton's kinetic energy of 1.527 MeV is only 0.16 % of its rest energy, so it is "still slow" and Newton's formulas almost suffice; a 1 MeV electron already has twice its rest energy as kinetic energy, and Newton's formulas are useless.

The **total energy** of a particle is its rest energy plus its kinetic energy: $E = mc^2 + W$. How many times the rest energy the total energy is, is called the **Lorentz factor** $\gamma$:

$$
\gamma = \frac{E}{mc^2} = 1 + \frac{W}{mc^2}.
$$

For the example proton $\gamma = 1 + 1.527/938.27 = 1.0016$. At rest $\gamma = 1$; the larger $\gamma$, the more "relativistic" the particle and the closer it is to the speed of light. When $\gamma$ is only slightly above 1, Newtonian mechanics is a very good approximation.

## Energy, velocity and momentum {#kinematics-variables}

### Three exact formulas

With $\gamma$ the velocity and the momentum can be computed exactly (the derivation is in [Going deeper: from γ to velocity and momentum](#kinematics-deep-relations)). The velocity is usually given as its ratio to the speed of light, $\beta = v/c$:

$$
\beta = \frac{v}{c} = \sqrt{1 - \frac{1}{\gamma^2}}, \qquad
p = \gamma m v, \qquad
pc = \beta\gamma\, mc^2 = \sqrt{W\left(W + 2mc^2\right)}.
$$

The momentum has changed from the school $mv$ to $\gamma m v$: the speed stops growing, but the momentum keeps growing with $\gamma$. Total energy, momentum and rest energy are related like the sides of a right triangle:

$$
E^2 = (pc)^2 + (mc^2)^2 .
$$

All of these are exact relations. For the example proton ($W$ = 1.527 MeV): $\gamma = 1.001627$, $\beta = 0.05698$, $\beta\gamma = 0.05708$, $p$ = 53.55 MeV/$c$. $\gamma$ is only 0.16 % above 1, but the combination $\beta\gamma$ runs through the whole primer: it links the geometric and the normalised emittance (see [Phase space and emittance](#phase-space)) and appears as $(\beta\gamma)^3$ in the space-charge force.

### The momentum unit MeV/c

Accelerator physicists use MeV as the unit of energy and **MeV/$c$** as the unit of momentum: momentum times the speed of light $c$ has the unit of energy, so "53.55 MeV/$c$" means $pc = 53.55$ MeV. Then $p\,[\mathrm{MeV}/c] = \beta\gamma\, mc^2\,[\mathrm{MeV}]$ and no conversion to kg·m/s is needed (if it is: 1 MeV/$c$ = $5.344\times10^{-22}$ kg·m/s, see [Going deeper: practical units and heavy ions](#kinematics-deep-units)). Note that the example proton's $pc$ is 53.55 MeV while its kinetic energy is only 1.527 MeV; the two are not the same thing.

Common rest energies:

| Particle | $mc^2$ (MeV) |
|---|---|
| electron | 0.51099895 |
| proton | 938.272088 |
| atomic mass unit u | 931.494102 |

> Note: the rest energy of a heavy ion is the mass of the ion itself, i.e. the atomic mass minus the stripped electrons (binding energies can usually be neglected); it is **not** $A \times 938.272$ MeV.

Drag the kinetic energy $W$ in the lab at the start of this chapter and watch how $\beta$, $\beta\gamma$, $B\rho$, $E\rho$ and $\beta\lambda$ change, and how the $\beta(W)$ curve bends from the $\sqrt W$ growth at low energy towards 1.

## When Newton's formulas are good enough {#kinematics-nonrel}

Write $u = W/(mc^2)$, the kinetic energy as a fraction of the rest energy. Each of the two extremes has its own set of simple formulas, and knowing where they fail matters just as much.

### Low speed: Newton's formulas (small u)

$$
W \approx \tfrac12 mv^2, \qquad \beta \approx \sqrt{2u}, \qquad pc \approx \sqrt{2\,mc^2\,W}
$$

These are the school formulas. The speed they give is too high, by a relative error of about $\tfrac34 u$ (derived in [Going deeper: from γ to velocity and momentum](#kinematics-deep-relations)). For a 1.527 MeV proton $u = 1.63\times10^{-3}$; Newton's formula gives $\beta = 0.057052$, the exact value is $0.056983$, an error of 0.12 %.

This is what "nearly" meant above: the error is small, but it accumulates. In a 0.21 m cavity a 0.12 % speed error shifts the arrival time by about 15 ps, or 0.9° of RF phase, already comparable with the 1° scale one usually cares about in RF phase. Heavy-ion and low-energy proton sections can be estimated with Newton's formulas first, but AVAS's linear preview uses the exact formulas (the manual does not describe the engine's relativistic treatment separately; presumably it uses the exact formulas too).

### High speed: the ultra-relativistic limit (large γ)

$$
\beta \approx 1 - \frac{1}{2\gamma^2}, \qquad pc \approx E \approx W
$$

The speed is almost the speed of light, and the momentum (times $c$) is almost equal to the energy. An electron at 1 MeV has $\gamma = 2.96$ and $\beta = 0.941$; Newton's formula would give $\beta \approx 1.98$, a complete failure. Electron beams must therefore always be treated relativistically. In the lab below (mass set to 0.511 MeV for an electron, frequency 2856 MHz), watch $\beta$ approach 1 quickly above 1 MeV and $\beta\lambda$ approach $\lambda$:

```avas-lab
kinematics mass=0.511 q=1 W=1 f=2856
```

## A little more energy, how much more speed {#kinematics-differential}

### Runners

In a beam every particle has a slightly different energy. Give a particle an extra push so that its momentum is a little higher: how much faster does it get?

Think about low and high speed. At low speed $p = mv$: 1 % more momentum is 1 % more speed. Close to the speed of light the speed can no longer grow: 1 % more momentum leaves the speed almost unchanged. The exact result is:

$$
\frac{\Delta v}{v} = \frac{1}{\gamma^2}\,\frac{\Delta p}{p}.
$$

**The velocity deviation is $1/\gamma^2$ times the momentum deviation.** For the example proton $1/\gamma^2 = 0.99675$, practically the low-speed "1 % for 1 %"; for a 1 GeV proton $1/\gamma^2 = 0.234$; for a 10 MeV electron only 0.0024. The derivation is in [Going deeper: why the velocity deviation is 1/γ² of the momentum deviation](#kinematics-deep-differential).

### Momentum deviation and energy spread

Accelerator physicists usually write the relative momentum deviation as $\delta = \Delta p / p$ (one of AVAS's longitudinal coordinates, see [Phase space](#phase-space-coordinates)). It is related to the energy deviation by

$$
\Delta W = \beta^2\gamma\, mc^2\,\delta = v\,\Delta p, \qquad
\frac{\Delta W}{W} = \frac{\gamma + 1}{\gamma}\,\delta .
$$

At low speed $\Delta W / W \approx 2\delta$ (because $W \propto v^2$, 1 % more speed is 2 % more kinetic energy); in the ultra-relativistic limit $\Delta W / W \to \delta$. For the example beam $\Delta W/W = 1.998\,\delta$; its $\delta_{\mathrm{rms}} = 3.53\times10^{-3}$ corresponds to an rms energy spread of 10.78 keV (note: from this chapter on, the longitudinal examples in the Fundamentals and Components parts use the `twissz` values of `beam.txt`, with an rms bunch length of 1.316 mm and an energy spread of 10.78 keV; Part 1 uses statistics taken directly from the particle file `part_rfq.dst`, 1.24 mm and 11.4 keV; the small difference comes from the $\beta_z$ entered in `twissz` in the example `beam.txt`, see [Initial distributions: In AVAS](#distributions-in-avas)).

### The consequence: bunches lengthen in drifts

The most direct consequence of different speeds is that **a bunch gets longer as it flies**: a particle with momentum deviation $\delta$ gains $L\delta/\gamma^2$ on every length $L$, just as runners who start together with slightly different speeds spread out. This is exactly why the $z$–$\delta$ matrix element of a drift in [Linear optics](#linear-optics) is $L/\gamma^2$.

After a 1 m drift without RF cavities the example bunch (rms length 1.316 mm) has an rms length of 3.71 mm, almost three times as long; the energy spread alone contributes $L\,\delta_{\mathrm{rms}}/\gamma^2$ = 3.52 mm. That is why low-energy sections need buncher cavities at short intervals to "pull the bunch back together" (see [Medium-energy transport](#journey-mebt)). For a 10 MeV electron the same $\delta$ lengthens the bunch only about 1/420 as much as for the proton; this is why electrons above a few MeV show almost no longitudinal phase slippage while low-energy protons slip strongly (see [Longitudinal motion](#longitudinal)).

## Magnetic and electric rigidity {#kinematics-rigidity}

### Magnetic rigidity: how hard it is to bend a particle

From school: a charged particle flying perpendicularly into a uniform magnetic field moves on a circle, with the Lorentz force $qvB$ supplying the centripetal force,

$$
qvB = \frac{mv^2}{\rho} \quad\Longrightarrow\quad \rho = \frac{mv}{qB}.
$$

$\rho$ is the radius of the circle. Replacing $mv$ by the relativistic momentum $p = \gamma mv$ (the derivation is exactly the same, only the momentum is different) and rearranging gives

$$
B\rho = \frac{p}{q}.
$$

$B\rho$ is the **magnetic rigidity**: the field times the radius needed to make the particle turn on a circle of radius $\rho$. It depends only on the ratio of momentum to charge: more momentum makes the particle "stiffer", more charge makes it "softer". The velocity $v$ cancels because both the magnetic force $qvB$ and the centripetal force needed for the turn, $pv/\rho$, are proportional to $v$.

For the example proton: $p = 2.862\times10^{-20}$ kg·m/s, $q = 1.602\times10^{-19}$ C, $B\rho$ = 0.1786 T·m. That is, a 1 T field bends it onto a circle of radius 0.179 m, 0.18 T onto 0.992 m. The usual practical formula is

$$
\Brho\,[\mathrm{T\cdot m}] = \frac{p\,[\mathrm{MeV}/c]}{299.792458\,\lvert Z \rvert} = 3.3356\,\frac{p\,[\mathrm{GeV}/c]}{\lvert Z \rvert}
$$

($Z$ is the charge number, 1 for a proton; where the 299.79 comes from is shown in [Going deeper: practical units and heavy ions](#kinematics-deep-units)).

The strength of every magnetic element is measured against $B\rho$:

- dipole bending radius $\rho = B\rho / B$ (see [Dipole](#dipole));
- quadrupole focusing strength $k = G/(B\rho)$ in m$^{-2}$ (see [Quadrupole](#quadrupole));
- solenoid $k = B/(2B\rho)$ in m$^{-1}$ (see [Solenoid](#solenoid)).

The same magnet therefore has a different strength for beams of different energy or charge-to-mass ratio; after changing the beam energy, the fields must be scaled in proportion to $B\rho$ to keep the same optics.

### Electric rigidity

When a transverse electric field $E$ does the bending, the electric force $qE$ supplies the centripetal force: $qE = mv^2/\rho$, relativistically $qE = pv/\rho$. This time $v$ does not cancel, because the electric force does not depend on the velocity. Define the **electric rigidity**

$$
E\rho = \frac{pv}{q} = \beta c\,(B\rho), \qquad
E\rho\,[\mathrm{MV}] = \frac{\beta\,pc\,[\mathrm{MeV}]}{\lvert Z \rvert}
$$

At low speed $pv \approx mv^2 = 2W$, i.e. $E\rho \approx 2W/Z$ ($E\rho$ in MV for $W$ in MeV). The bending radius of an electrostatic element is $\rho = E\rho/E$, the strength of an electrostatic quadrupole $k = (\partial E_x/\partial x)/(E\rho)$. The example proton has $E\rho$ = 3.052 MV; a transverse field of 5 MV/m bends it onto $\rho$ = 0.610 m.

### The answer to the opening puzzle

At low speed $B\rho \propto p \propto \sqrt W$, while $E\rho \approx 2W$. Raising the energy tenfold (1.527 → 15.27 MeV) makes $B\rho$ only 3.17 times larger, but $E\rho$ 9.93 times larger.

Put another way: for the same bending, the electric field has to equal $v$ times the magnetic field. For the example proton a 1 T magnetic field acts like an electric field of 17.1 MV/m; for a particle close to the speed of light it acts like 300 MV/m, far beyond any electrostatic field that can be sustained in practice. Electrostatic elements are therefore effective only at low speed ($E\rho \propto \beta^2$), and at high energy magnetic elements are far more economical; the low-energy RFQ uses exactly the fact that electric focusing does not weaken with speed (see [RFQ](#journey-rfq)). Electrostatic elements are described in [Static field elements](#static-fields).

Set $W$ to 15.27 MeV in the lab and compare $B\rho$ (0.567 T·m) and $E\rho$ (30.3 MV) with the example values:

```avas-lab
kinematics mass=938.272 q=1 W=15.27 f=162.5
```

> Note: the sign of the rigidity follows the sign of the charge. The practical formulas above use $\lvert Z \rvert$; for negative particles the same field gives a force in the opposite direction, and the focusing plane of a quadrupole is swapped.

## RF wavelength and βλ {#kinematics-rf}

### Crossing a bridge in step

The electric field in an RF cavity swings back and forth, reversing every half period. A particle is accelerated only if it crosses an accelerating gap during the half period in which the field points forward. A row of accelerating gaps is like a row of doors that open and close on a timer: a particle that met the accelerating moment at the first gap must still be accelerated at the next one, so the distance between the gaps has to "keep the beat" with the particle's speed.

The RF frequency $f$ corresponds to the period and the free-space wavelength

$$
T_{\mathrm{RF}} = \frac{1}{f}, \qquad \lambda = \frac{c}{f}.
$$

In one RF period the particle covers $vT_{\mathrm{RF}} = \beta\lambda$. **$\beta\lambda$ is the basic unit of the longitudinal scale of a linac**:

- in π-mode structures the fields in neighbouring gaps point in opposite directions and the gaps are $\beta\lambda/2$ apart; in 0-mode structures (such as the DTL type) neighbouring gaps point the same way and are $\beta\lambda$ apart;
- converting longitudinal position to phase: a particle $\Delta z$ ahead of the reference particle reaches the accelerating gap earlier and sees a smaller RF phase, $\Delta\varphi = -2\pi\,\Delta z/(\beta\lambda)$;
- the transit-time factor changes with $\beta$, so a cavity accelerates effectively only within a certain range of $\beta$ (see [RF cavity](#rf-cavity)).

Example: at 162.5 MHz $\lambda = 1.8449$ m and $T_{\mathrm{RF}}$ = 6.154 ns. A 1.527 MeV proton has $\beta\lambda$ = 0.10513 m, i.e. 1° of RF phase corresponds to 0.292 mm. At its initial speed the reference particle needs about $0.21/(\beta c)$ = 12.29 ns to cross the 0.21 m HWR field map, about 2.0 RF periods.

When the speed changes, $\beta\lambda$ changes with it: from 35 keV at the ion source ($\beta\lambda$ = 15.9 mm) to 1 GeV (1.614 m) it grows about 100-fold. Drag $W$ to both ends in the lab at the start of the chapter to see it. That is why a linac is divided into several sections, each with cavities designed for one speed, see [From the ion source to the target](#journey-overview).

## Tables: from protons to electrons {#kinematics-examples}

The three tables below are computed with the exact formulas of this chapter, to build a feeling for the numbers. Three things are worth noticing: from 1.527 MeV to 1 GeV a proton's $\beta$ grows 15-fold but its $B\rho$ 32-fold; above 1 MeV an electron's $\beta\lambda$ is already almost $\lambda$; the electric rigidity $E\rho = \beta pc$ is close to $2W$ at low energy and approaches $pc$ at high energy.

### Protons (mc² = 938.272 MeV, βλ at 162.5 MHz)

| $W$ (MeV) | $\gamma$ | $\beta$ | $\beta\gamma$ | $p$ (MeV/$c$) | $B\rho$ (T·m) | $E\rho$ (MV) | $\beta\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 1.527 | 1.001627 | 0.056983 | 0.057076 | 53.55 | 0.17863 | 3.052 | 0.10513 |
| 10 | 1.010658 | 0.144844 | 0.146388 | 137.35 | 0.45816 | 19.89 | 0.26722 |
| 100 | 1.106579 | 0.428195 | 0.473832 | 444.58 | 1.48297 | 190.4 | 0.78997 |
| 1000 | 2.065789 | 0.875026 | 1.807618 | 1696.04 | 5.65737 | 1484.1 | 1.61431 |

The first line is the beam of the example project.

### Electrons (mc² = 0.511 MeV, βλ at 2856 MHz, λ = 0.10497 m)

| $W$ (MeV) | $\gamma$ | $\beta$ | $\beta\gamma$ | $p$ (MeV/$c$) | $B\rho$ (T·m) | $E\rho$ (MV) | $\beta\lambda$ (m) |
|---|---|---|---|---|---|---|---|
| 0.1 | 1.195695 | 0.548221 | 0.655505 | 0.3350 | 0.001117 | 0.1836 | 0.05755 |
| 1 | 2.956951 | 0.941079 | 2.782725 | 1.4220 | 0.004743 | 1.338 | 0.09878 |
| 10 | 20.56951 | 0.998818 | 20.54519 | 10.499 | 0.03502 | 10.49 | 0.10485 |
| 100 | 196.6951 | 0.999987 | 196.6926 | 100.51 | 0.33526 | 100.51 | 0.10497 |

### Heavy ions (when the energy is given per nucleon)

Take $^{40}$Ar$^{8+}$ at 0.5 MeV per nucleon: the rest energy of the ion is about $39.9624 \times 931.494 - 8\times0.511$ = 37220.6 MeV, the total kinetic energy $W = 40\times0.5$ = 20 MeV, so $\beta$ = 0.03277, $p$ = 1220.3 MeV/$c$ and $B\rho = p/(299.79\times8)$ = 0.5088 T·m. Compared with a 1.527 MeV proton it is only about 58 % as fast but 2.85 times as rigid: the smaller the charge-to-mass ratio $Z/A$, the larger the rigidity at the same speed and the harder the focusing.

## Think about it {#kinematics-think}

> **Think 1**: The kinetic energy of a low-energy proton doubles. How much faster does it get?
>
> Answer: with Newton's formulas $\beta \propto \sqrt W$, so the speed grows only to $\sqrt2 = 1.414$ times. For the example proton going from 1.527 MeV to 3.054 MeV the exact value is 1.412 times. Each increase in energy buys less and less increase in speed; this is the root of every "speed saturation" effect later on.

> **Think 2**: How long is the example bunch (rms length 1.316 mm) after a 1 m drift without RF cavities? And for a 10 MeV electron?
>
> Answer: propagated with the linear transfer $z_1 = z_0 + (L/\gamma^2)\,\delta$, the rms length after 1 m is 3.71 mm, almost three times as long; the energy spread alone contributes 3.52 mm. A 10 MeV electron has $1/\gamma^2 = 2.4\times10^{-3}$, so the same $\delta$ lengthens the bunch only about 1/420 as much as for the proton.

> **Think 3**: Why can't one type of 162.5 MHz cavity accelerate protons all the way from the ion source's 35 keV to 1 GeV?
>
> Answer: $\beta\lambda$ grows from 15.9 mm at 35 keV to 1.614 m at 1 GeV, about 100-fold. A cavity with fixed gap spacing "keeps the beat" only for one design speed; at other speeds the particles reach the following gaps at the wrong phase, and the effective acceleration drops with the transit-time factor. That is why a linac is divided into sections, each with cavities designed for one $\beta$, see [From the ion source to the target](#journey-overview).

> **Think 4**: An electron and a proton both have 1.527 MeV. Which is harder to bend with a magnetic field?
>
> Answer: the proton. The electron's $pc$ is only 1.97 MeV and its $B\rho$ = 0.0066 T·m; the proton's $pc$ = 53.55 MeV and its $B\rho$ = 0.179 T·m, about 27 times the electron's. The electron is much faster (0.968 times the speed of light), but it is so light that its momentum is smaller. Magnetic rigidity depends on momentum, not on speed and not on kinetic energy.

## Common misconceptions {#kinematics-misconceptions}

- **"Kinetic energy, total energy and energy per nucleon are the same thing."** $W$ is the kinetic energy, $E = W + mc^2$ the total energy; heavy-ion energies are often quoted as kinetic energy per nucleon and must be multiplied by the number of nucleons for the whole ion. AVAS's `kneticenergy` is the kinetic energy of the whole particle.
- **"The momentum unit MeV/$c$ and the energy unit MeV are interchangeable."** Only in the ultra-relativistic limit is $pc \approx W$; the example proton has $pc$ = 53.55 MeV but a kinetic energy of only 1.527 MeV.
- **"The faster a particle, the larger its mass."** This is an old way of speaking. Today one says that the (rest) mass stays the same and that what changes is the relation of energy and momentum to speed: $E = \gamma mc^2$, $p = \gamma mv$.
- **"The mass of a heavy ion is $A$ times the proton mass."** Use the atomic mass unit u = 931.494 MeV and subtract the stripped electrons; $40\times938.272$ for $^{40}$Ar is 0.8 % too large.
- **"Non-relativistic formulas work everywhere at the MeV scale."** For protons the error is about $3u/4$, fine for estimates; for electrons they already fail at 100 keV.
- **"$\beta\lambda$ is a fixed length."** It changes with energy and approaches $\lambda$ only for $\beta \to 1$.
- **"Rigidity has no sign."** The practical formulas use $\lvert Z \rvert$, but the direction of the force follows the sign of the charge; for negative particles the focusing plane of a quadrupole is swapped.
- **"A given quadrupole focuses equally well at any energy."** $k = G/(B\rho)$; when the energy changes, $k$ changes.

## Going deeper: from γ to velocity and momentum {#kinematics-deep-relations}

The main text gave the relations of $\beta$ and $p$ to $\gamma$ directly. This section starts from two results of special relativity and derives them step by step with school algebra, checking every step with the example proton.

### Step 1: two starting points

Special relativity says that a particle moving with velocity $v$ has the total energy and momentum

$$
E = \gamma mc^2, \qquad p = \gamma m v, \qquad \gamma = \frac{1}{\sqrt{1 - v^2/c^2}} = \frac{1}{\sqrt{1-\beta^2}} .
$$

This $\gamma$ is the same number as "how many times the rest energy the total energy is" in the main text. Check: the example proton has $\beta = 0.056982$, $\beta^2 = 0.0032470$, $\gamma = 1/\sqrt{0.9967530} = 1.001627$; and $1 + W/mc^2 = 1 + 1.527/938.272 = 1.001627$. The two agree.

### Step 2: the velocity from γ

Square $\gamma = 1/\sqrt{1-\beta^2}$ and take the reciprocal: $1 - \beta^2 = 1/\gamma^2$, so

$$
\beta = \sqrt{1 - \frac{1}{\gamma^2}} .
$$

Example: $1/\gamma^2 = 1/1.003257 = 0.996753$, $\beta = \sqrt{0.003247} = 0.05698$.

### Step 3: βγ and the momentum

The square of $\beta\gamma$ is $\gamma^2\beta^2 = \gamma^2(1 - 1/\gamma^2) = \gamma^2 - 1$, so $\beta\gamma = \sqrt{\gamma^2 - 1}$. Example: $\sqrt{1.003257 - 1} = 0.05708$. The momentum times $c$:

$$
pc = \gamma m v c = \beta\gamma\, mc^2 = 0.05708 \times 938.272\ \text{MeV} = 53.55\ \text{MeV}.
$$

### Step 4: the "Pythagoras" of energy and momentum

Take $\gamma^2 - \beta^2\gamma^2 = 1$ from step 3 and multiply both sides by $(mc^2)^2$:

$$
(\gamma mc^2)^2 - (\beta\gamma mc^2)^2 = (mc^2)^2 \quad\Longrightarrow\quad E^2 = (pc)^2 + (mc^2)^2 .
$$

Example: $E = 939.799$ MeV, $E^2 - (pc)^2 = 883222 - 2868 = 880354$, and $938.272^2 = 880354$. Substituting $E = W + mc^2$ and expanding, $(pc)^2 = (W + mc^2)^2 - (mc^2)^2 = W^2 + 2Wmc^2$, which is the main text's $pc = \sqrt{W(W + 2mc^2)} = \sqrt{1.527 \times 1878.07} = 53.55$ MeV.

### Step 5: back to Newton at low speed

A common school approximation: for small $x$, $(1+x)^n \approx 1 + nx$. For example $\sqrt{1.02} = 1.00995 \approx 1.01$. With it, $\gamma = (1 - \beta^2)^{-1/2} \approx 1 + \tfrac12\beta^2$, so

$$
W = (\gamma - 1)\,mc^2 \approx \tfrac12 \beta^2 mc^2 = \tfrac12 m v^2 .
$$

Newton's kinetic energy is the low-speed approximation of relativity. Check: for the example proton $\tfrac12\beta^2 = 0.0016235$ while $\gamma - 1 = 0.0016275$, a difference of only 0.25 %.

How wrong is the speed from Newton's formula? From step 2, $\beta^2 = 1 - 1/(1+u)^2 = (2u + u^2)/(1+u)^2$ (with $u = W/mc^2$), so

$$
\beta = \sqrt{2u}\cdot\frac{\sqrt{1 + u/2}}{1 + u} \approx \sqrt{2u}\left(1 + \tfrac14 u\right)\left(1 - u\right) \approx \sqrt{2u}\left(1 - \tfrac34 u\right).
$$

Newton's $\sqrt{2u}$ is too large by about $\tfrac34 u$. For the example $u = 1.627\times10^{-3}$, $\tfrac34 u = 0.12\,\%$, the number in the main text.

### Step 6: hugging the speed of light at high speed

When $\gamma$ is large, $1/\gamma^2$ is small, and $(1+x)^n \approx 1 + nx$ again gives $\beta = (1 - 1/\gamma^2)^{1/2} \approx 1 - 1/(2\gamma^2)$. A 10 MeV electron has $\gamma = 20.57$: $1 - 1/(2\times423.1) = 0.998818$, exactly the precise value 0.998818. For a 1 MeV electron with $\gamma = 2.957$ the approximation gives 0.943 against the exact 0.941: when $\gamma$ is not large enough, the approximation is rougher. At the same time the rest energy in $E^2 = (pc)^2 + (mc^2)^2$ can be neglected, so $pc \approx E \approx W$.

### For readers who know calculus and special relativity

$(E/c, \boldsymbol p)$ form the four-momentum, and $E^2 - (pc)^2 = (mc^2)^2$ is its Lorentz invariant. The Taylor series of $\beta(u)$ is $\beta = \sqrt{2u}\,(1 - \tfrac34 u + \tfrac{23}{32}u^2 - \dots)$. The rapidity $\eta$ is also common: $\gamma = \cosh\eta$, $\beta\gamma = \sinh\eta$, $\beta = \tanh\eta$, and the identity of step 3 is $\cosh^2\eta - \sinh^2\eta = 1$.

## Going deeper: why the velocity deviation is 1/γ² of the momentum deviation {#kinematics-deep-differential}

### Step 1: a numerical experiment first

Raise the example proton's momentum by 1 %: $pc$ goes from 53.552 MeV to 54.088 MeV. The velocity is $\beta = pc/E$ with $E = \sqrt{(pc)^2 + (mc^2)^2}$:

- before: $\beta = 53.552/939.799 = 0.056982$;
- after: $E = \sqrt{54.088^2 + 938.272^2} = 939.830$ MeV, $\beta = 54.088/939.830 = 0.057550$.

The velocity grew by 0.997 %, exactly $1/\gamma^2 = 0.99675$ times 1 %. The same experiment for a 1 GeV proton: the velocity grows by only 0.23 %, and $1/\gamma^2 = 0.234$.

### Step 2: deriving it with "small changes"

The velocity can be written as $\beta = pc/E$ (dividing $p = \gamma mv$ by $E = \gamma mc^2$). Let the momentum become $p(1 + \delta)$:

- energy: in $E^2 = (pc)^2 + (mc^2)^2$ the term $(pc)^2$ becomes $(pc)^2(1 + 2\delta)$ (neglecting $\delta^2$), so $E^2$ grows by $2\delta\,(pc)^2$, a relative growth of $2\delta\,(pc/E)^2 = 2\beta^2\delta$, and $E$ itself grows by half as much, $\beta^2\delta$ relative;
- velocity: $\beta = pc/E$ becomes $\beta\,(1+\delta)/(1+\beta^2\delta) \approx \beta\,(1 + \delta - \beta^2\delta)$ (using $1/(1+x) \approx 1 - x$).

The relative change of the velocity is therefore $(1 - \beta^2)\,\delta = \delta/\gamma^2$. At low speed $\beta^2 \approx 0$ and velocity and momentum change in the same proportion; at high speed $\beta^2 \to 1$, the energy grows almost together with the momentum, and their ratio (the velocity) hardly changes.

### Step 3: the energy deviation

From step 2 the absolute change of the energy is $\Delta E = \beta^2 E\,\delta = \beta^2\gamma\,mc^2\,\delta$. Kinetic and total energy differ only by the constant $mc^2$, so $\Delta W = \Delta E$. Also $\beta^2\gamma\,mc^2 = \beta c \cdot \beta\gamma mc = v\,p$, so $\Delta W = v\,\Delta p$.

Dividing by $W = (\gamma - 1)\,mc^2$ and using $\beta^2\gamma^2 = \gamma^2 - 1 = (\gamma-1)(\gamma+1)$:

$$
\frac{\Delta W}{W} = \frac{\beta^2\gamma}{\gamma - 1}\,\delta = \frac{\gamma + 1}{\gamma}\,\delta .
$$

Example: $(\gamma+1)/\gamma = 2.001627/1.001627 = 1.998$; $\Delta W_{\mathrm{rms}} = 0.0032470 \times 1.001627 \times 938.272\ \text{MeV} \times 3.532\times10^{-3} = 10.78$ keV.

### Step 4: why bunches lengthen

The reference particle needs the time $L/v$ to cover $L$; a particle faster by $\Delta v$ covers an extra $\Delta v \cdot L/v = L\,\delta/\gamma^2$ in that time. For the example $\delta_{\mathrm{rms}} = 3.532\times10^{-3}$, a 1 m drift spreads the particles by 3.52 mm, which adds in squares to the initial length of 1.316 mm (at the entrance the two are almost uncorrelated): $\sqrt{1.316^2 + 3.52^2} \approx 3.76$ mm; including the weak correlation between $z$ and $\delta$ of the example beam at the entrance, the exact value is 3.71 mm. The full drift derivation is in [Drift](#drift-deep-longitudinal).

### For readers who know calculus

Differentiating $\gamma = 1 + W/mc^2$, $\beta = \sqrt{1 - \gamma^{-2}}$ and $p = \beta\gamma mc$:

$$
\dd\gamma = \frac{\dd W}{mc^2}, \qquad
\frac{\dd\beta}{\beta} = \frac{\dd\gamma}{\beta^2\gamma^3} = \frac{1}{\gamma^2}\,\frac{\dd p}{p}, \qquad
\frac{\dd p}{p} = \frac{\dd\gamma}{\beta^2\gamma}.
$$

In ring accelerators the factor $1/\gamma^2$ combines with the momentum compaction factor into the slip factor $\eta = \alpha_c - 1/\gamma^2$; in a linac $\alpha_c = 0$ and only the velocity term remains.

## Going deeper: practical units and heavy ions {#kinematics-deep-units}

### Step 1: MeV/c in SI units

1 MeV $= 10^{6} \times 1.602\times10^{-19}$ J; dividing by $c = 2.998\times10^{8}$ m/s:

$$
1\ \mathrm{MeV}/c = \frac{1.602\times10^{-13}\ \text{J}}{2.998\times10^{8}\ \text{m/s}} = 5.344\times10^{-22}\ \text{kg·m/s}.
$$

The example proton's $p = 53.552\ \mathrm{MeV}/c = 2.862\times10^{-20}$ kg·m/s, only 0.04 % more than the school value $mv = 2.861\times10^{-20}$ ($\gamma$ is only 0.16 % above 1, and in $\gamma mv$ the velocity is 0.12 % below Newton's value; the two largely cancel).

### Step 2: where 299.79 comes from

$B\rho = p/(Ze)$. With $p$ in MeV/$c$:

$$
B\rho = \frac{p\,[\mathrm{MeV}/c] \times 5.344\times10^{-22}}{Z \times 1.602\times10^{-19}}\ \text{T·m} = \frac{p\,[\mathrm{MeV}/c]}{299.79\,Z}\ \text{T·m}.
$$

$299.79$ is just the speed of light in units of $10^{6}$ m/s (the ratio of $e$ to $10^{6}e/c$). Example: $53.552/299.79 = 0.17863$ T·m.

### Step 3: practical units of the electric rigidity

$E\rho = pv/(Ze) = \beta\,(pc)/(Ze)$. With $pc$ in MeV, dividing by the charge number $Z$ gives the electric rigidity in MV: for the example $0.056982 \times 53.552 = 3.0515$ MV. At low speed $pv \approx mv^2 = 2W$, so $E\rho \approx 2 \times 1.527 = 3.054$ MV, only 0.1 % off the exact value.

### Step 4: heavy ions

The rest energy of a heavy ion is computed with the atomic mass unit u = 931.494 MeV, minus the stripped electrons: $^{40}$Ar has an atomic mass of 39.9624 u, and with 8 electrons stripped its rest energy is $39.9624 \times 931.494 - 8 \times 0.511 = 37220.6$ MeV. $40 \times 938.272$ would be 0.8 % too large.

Heavy-ion energies are often given **per nucleon**: $^{40}$Ar at 0.5 MeV per nucleon has a total kinetic energy $W = 40 \times 0.5 = 20$ MeV. Then $u = W/mc^2 = 5.37\times10^{-4}$, $\beta = 0.03277$, $pc = \beta\gamma\,mc^2 = 1220.3$ MeV and $B\rho = 1220.3/(299.79 \times 8) = 0.5088$ T·m.

Why are heavy ions hard to focus? At the same speed the momentum is proportional to the mass and the rigidity to $A/Z$: Ar⁸⁺ has $A/Z = 5$, so it is only 58 % as fast as the example proton but 2.85 times as rigid.

### For readers who know dimensional analysis

In natural units ($c = 1$) energy, momentum and mass share one dimension, and the 3.3356 in $B\rho\,[\mathrm{T\cdot m}] = 3.3356\,p\,[\mathrm{GeV}/c]/Z$ is $10^{9}/c$ (in SI numbers). When the kinetic energy per nucleon $T_u$ is given, $\gamma = 1 + T_u/(m_u c^2)$ holds only approximately ($m_u$ being the average rest energy per nucleon, including binding energy and electron corrections); an exact calculation first computes $mc^2$ of the whole ion.

## In AVAS {#kinematics-in-avas}

### Keywords

The kinematics keywords of `beam.txt`:

```avas-params
beam:particlerestmass beam:numofcharge beam:kneticenergy beam:frequency beam:current
```

- `particlerestmass`: rest energy $mc^2$ of the particle in MeV (938.272088159928 in the example project, a proton).
- `numofcharge`: charge number $Z$ in units of $e$. It enters both the rigidity $B\rho = p/(Ze)$ and space charge (the charge of each particle).
- `kneticenergy`: kinetic energy $W$ of the reference particle in MeV (spelled exactly `kneticenergy`; not listed in the manual's table, written by the Beam page).
- `frequency`: bunch frequency in **Hz** (`162500000.0` in the example), used to convert longitudinal position to phase and to get the bunch charge $Q = I/f$.
- `current`: beam current in mA.

> **AVAS convention**: the user interface's linear preview computes $\gamma = 1 + W/m$; `kneticenergy` and `particlerestmass` are quantities of the **whole particle** (not per nucleon). For heavy ions write the total mass and total kinetic energy of the ion; for the $^{40}$Ar$^{8+}$ above, `particlerestmass 37220.6`, `kneticenergy 20`, `numofcharge 8`.

> Note: the `frequency` of `beam.txt` and the frequency parameter `f` of lattice `field` elements are both in Hz; the `f` of this chapter's labs is in MHz, only to make it easier to drag. Negatively charged particles (such as electrons) should by definition get a negative charge number; the manual says nothing specific about how the engine handles negative charges, so check the focusing direction of a quadrupole with a simple lattice before relying on it.

### Bunch charge and macroparticles

The bunch charge of the example project is $Q = 0.1841\,\mathrm{mA} / 162.5\,\mathrm{MHz}$ = 1.133 pC, about $7.07\times10^6$ protons; with 5260 macroparticles each macroparticle stands for about 1344 protons. The meaning of macroparticles is explained in [How AVAS simulates a beam](#intro-macroparticles) and [Initial distributions](#distributions).

### Kinematic quantities in the output

Column 0 of `DataSet.txt` is the kinetic energy $W$ of the reference particle (MeV); with this chapter's formulas it gives $\beta\gamma$ at every position, and from it the geometric emittance from the normalised one (see [Phase space](#phase-space-emittance)).

### Lab parameters

Parameters of the `kinematics` lab: `mass` (MeV), `q` (charge number), `W` (MeV), `f` (MHz). Suggested checks: start from the example proton beam and confirm the numbers in the table; raise $W$ tenfold (15.27 MeV), and $B\rho$ grows about 3.17 times, $E\rho$ about 9.93 times; keep the speed fixed and change the charge number, and the rigidity scales as $1/Z$.

## Further reading {#kinematics-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: relativistic relations, $\beta\lambda$ and the scale of accelerating structures.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific: magnetic rigidity and practical units.
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015.
- W. Bertozzi, "Speed and kinetic energy of relativistic electrons", *Am. J. Phys.* 32 (1964) 551: a direct measurement showing that electrons do not exceed the speed of light.
- Continue with: [Phase space and emittance](#phase-space).
