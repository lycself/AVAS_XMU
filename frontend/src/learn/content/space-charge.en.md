# Space charge

The charged particles of a beam repel each other. In low-energy, high-intensity beams this **space-charge force** cancels part of the external focusing, enlarges the beam, depresses the phase advance and, through its non-linear part, causes emittance growth and halo. This chapter derives the self fields of uniform beams, introduces the generalised perveance $K$, derives the KV envelope equations and Sacherer's rms envelope equations, gives analytic expressions for the matched size and the tune depression in the smooth approximation, discusses emittance growth and halo qualitatively, and explains how the AVAS kernel and the linear preview handle space charge. Transverse linear optics and Twiss parameters are in [Linear optics](#linear-optics) and [Phase space and emittance](#phase-space), longitudinal motion in [Longitudinal dynamics](#longitudinal).

The numerical examples use the beam of the example project: protons, $W = 1.527\ \mathrm{MeV}$ ($\beta = 0.05698$, $\gamma = 1.00163$, $\beta\gamma = 0.05708$, $B\rho = 0.1786\ \mathrm{T\,m}$), average current $I = 0.184\ \mathrm{mA}$, beam frequency $f = 162.5\ \mathrm{MHz}$, transverse normalised rms emittance $0.138\ \pi\,\mathrm{mm\,mrad}$ (geometric rms emittance $\varepsilon = 2.41\ \mathrm{mm\,mrad}$).

## Self fields of a uniform beam {#space-charge-self-fields}

### Round DC beam

Take charge distributed uniformly in a cylinder of radius $a$ with line density $\lambda_q = I/(\beta c)$. Gauss's and Ampère's laws give inside the beam ($r \le a$)

$$
E_r = \frac{\lambda_q}{2\pi\varepsilon_0}\frac{r}{a^2},\qquad B_\theta = \frac{\beta}{c}E_r .
$$

The magnetic force of the moving charges opposes the electric force:

$$
F_r = q\left(E_r - \beta c B_\theta\right) = q E_r\left(1 - \beta^2\right) = \frac{q E_r}{\gamma^2} .
$$

This $1/\gamma^2$ cancellation makes space charge fade quickly at relativistic energies. With $x'' = F_x/(\gamma m\beta^2c^2)$ the equation of motion of a particle inside the beam is

$$
x'' = \frac{K}{a^2}\,x ,
$$

with $K$ the generalised perveance defined in the next section. The force is proportional to $x$: **the self field of a uniform distribution is linear**, which is the basis of the KV theory.

### Elliptical cross-section

Inside a uniform beam of elliptical cross-section with semi-axes $a$, $b$

$$
E_x = \frac{\lambda_q}{\pi\varepsilon_0}\frac{x}{a(a + b)},\qquad
E_y = \frac{\lambda_q}{\pi\varepsilon_0}\frac{y}{b(a + b)},
\qquad
x'' = \frac{2K}{a(a + b)}\,x,\quad y'' = \frac{2K}{b(a + b)}\,y .
$$

For $a = b$ this reduces to the round beam.

### Bunches: the uniform ellipsoid

An RF-accelerated beam is a train of bunches, each with charge $Q = I/f$ ($I$ the average current). Model a bunch as a uniform ellipsoid with lab-frame semi-axes $a$, $b$, $c_z$; in the beam rest frame the longitudinal semi-axis is $\gamma c_z$ and the charge density $\rho' = 3Q/(4\pi a b\,\gamma c_z)$. The field inside a uniform ellipsoid is linear:

$$
E'_x = \frac{\rho'}{\varepsilon_0}M_x\,x,\quad E'_y = \frac{\rho'}{\varepsilon_0}M_y\,y,\quad E'_z = \frac{\rho'}{\varepsilon_0}M_z\,\gamma z,\qquad M_x + M_y + M_z = 1,
$$

with $\gamma z$ the rest-frame longitudinal coordinate and $M_i$ the depolarisation factors (all equal to $1/3$ for a sphere). For a spheroid with rest-frame aspect ratio $p = \gamma c_z/\sqrt{ab} > 1$,

$$
M_z = \frac{1 - e^2}{e^3}\left(\operatorname{artanh} e - e\right),\qquad e = \sqrt{1 - 1/p^2},
$$

with the common approximation $M_z \approx 1/(3p)$ (for $p$ from about 1 to a few) and transversely $M_x \approx (1 - M_z)\,b/(a + b)$. Transforming back to the lab (the transverse force gets another $1/\gamma$) gives

$$
x'' = \frac{K_3 M_x}{a b c_z}\,x,\qquad
z'' = \frac{K_3 M_z}{a b c_z}\,z,\qquad
K_3 = \frac{3 q Q}{4\pi\varepsilon_0\, mc^2\,\beta^2\gamma^3},
$$

where $z' = \dd z/\dd s = \delta/\gamma^2$.

## Generalised perveance {#space-charge-perveance}

### DC beams

$$
K = \frac{q I}{2\pi\varepsilon_0\, m c^3\beta^3\gamma^3} = \frac{2I}{I_0\,\beta^3\gamma^3},\qquad
I_0 = \frac{4\pi\varepsilon_0\, m c^3}{q} .
$$

$I_0$ is the characteristic current: $I_0 = 3.13\times 10^7\ \mathrm{A}$ for protons and $3.13\times 10^7\,A/Z\ \mathrm{A}$ for ions of mass number $A$ and charge state $Z$. $K$ is dimensionless and measures space charge against the beam "stiffness"; the $\beta^3\gamma^3$ makes it large at low energy. At the example energy:

| DC current | $K$ |
|---|---|
| 0.184 mA | $6.32\times 10^{-8}$ |
| 1 mA | $3.44\times 10^{-7}$ |
| 10 mA | $3.44\times 10^{-6}$ |

### Equivalent perveance of a bunch

Comparing the transverse equation of the uniform ellipsoid, $x'' = K_3 M_x x/(a b c_z)$, with $x'' = K x/a^2$ of a round DC beam (taking $a = b$, $M_x = (1 - M_z)/2$), the centre of a bunch acts like a DC beam with the **equivalent current** and perveance

$$
I_{\mathrm{eq}} = \frac{3}{4}\,\frac{Q\,\beta c}{c_z}\,(1 - M_z),\qquad
K_{\mathrm{eq}} = \frac{2 I_{\mathrm{eq}}}{I_0\beta^3\gamma^3} .
$$

Example bunch: $Q = I/f = 1.133\ \mathrm{pC}$ ($7.07\times 10^6$ protons), rms sizes $\sigma_x = 0.954$, $\sigma_y = 0.958$, $\sigma_z = 1.316\ \mathrm{mm}$. The semi-axes of the rms-equivalent uniform ellipsoid are $\sqrt5\,\sigma$ (see [next section](#space-charge-envelope)), the rest-frame aspect ratio is $p = 1.379$ and $M_z = 0.252$, so $I_{\mathrm{eq}} = 3.69\ \mathrm{mA}$ and $K_{\mathrm{eq}} = 1.27\times 10^{-6}$.

> Note: the space-charge strength of a bunch is set by the **peak line density inside the bunch**, not by the average current. In the example the equivalent current is 20 times the average current, and $I_{\mathrm{eq}}$ grows further when the bunch is compressed longitudinally. Estimating $K$ from the average current badly underestimates space charge.

## KV and rms envelope equations {#space-charge-envelope}

### KV envelope equations

The KV (Kapchinsky–Vladimirsky) distribution is uniform on a hyper-ellipsoidal shell in four-dimensional phase space; its projection on the $x$–$y$ plane is a uniform ellipse, its self field is exactly linear, and it stays KV in linear external fields. The beam-edge semi-axes $a(s)$, $b(s)$ obey

$$
a'' + k_x(s)\,a - \frac{2K}{a + b} - \frac{\varepsilon_x^2}{a^3} = 0,\qquad
b'' + k_y(s)\,b - \frac{2K}{a + b} - \frac{\varepsilon_y^2}{b^3} = 0 ,
$$

with $k_{x,y}(s)$ the external focusing (quadrupoles $k = \pm G/B\rho$, see [Quadrupoles](#quadrupole)) and $\varepsilon$ the **edge** (total) geometric emittance. The three terms are external focusing, space-charge defocusing and the emittance ("thermal pressure") term.

### Sacherer's rms envelope equations

Sacherer (1971) showed that for any distribution with elliptical symmetry the rms size $\sigma = \sqrt{\langle x^2\rangle}$ obeys an equation of the same form, for continuous (2D) beams the space-charge term does not depend on the profile at all, and for 3D ellipsoidally symmetric bunches it depends only weakly on it:

$$
\sigma_x'' + k_x\,\sigma_x - \frac{K}{2(\sigma_x + \sigma_y)} - \frac{\varepsilon_{x}^2}{\sigma_x^3} = 0 ,
$$

with $\varepsilon_x$ the rms geometric emittance. Substituting $a = 2\sigma_x$, $\varepsilon_{\mathrm{edge}} = 4\varepsilon_{\mathrm{rms}}$ in the KV equation gives exactly this. Any distribution can therefore be described by its "rms-equivalent KV beam": **edge size = 2 × rms size, edge emittance = 4 × rms emittance**.

### Three-dimensional rms envelope equations of a bunch

For a uniform ellipsoid $\langle x^2\rangle = a^2/5$, i.e. the rms-equivalent semi-axis is $\sqrt5\,\sigma$. With the linear forces of the previous section:

$$
\sigma_x'' + k_x\sigma_x - \frac{\varepsilon_x^2}{\sigma_x^3} - \frac{K_3 M_x}{5\sqrt5\,\sigma_y\sigma_z} = 0,\qquad
\sigma_z'' + k_z\sigma_z - \frac{\varepsilon_z^2}{\sigma_z^3} - \frac{K_3 M_z}{5\sqrt5\,\sigma_x\sigma_y} = 0 ,
$$

$y$ being analogous to $x$, $k_z$ the longitudinal RF focusing ($k_{l0}^2$ of [Longitudinal dynamics](#longitudinal)) and $\varepsilon_z$ the geometric rms emittance in the $(z, \dd z/\dd s)$ plane. The three planes are coupled through the $M_i$ and the sizes.

> AVAS convention: the longitudinal angle of `twissz` is $\delta = \Delta p/p$, while the equation above uses $\dd z/\dd s = \delta/\gamma^2$; they differ by $\gamma^2$. At the example energy $\gamma^2 = 1.003$ and the difference is negligible, but convert carefully at higher energy.

### Emittance-dominated or space-charge-dominated

Compare the two defocusing terms $K/(4\sigma)$ (round beam) and $\varepsilon^2/\sigma^3$: for $K\sigma^2/(4\varepsilon^2) \ll 1$ the beam is **emittance dominated**, for $\gg 1$ **space-charge dominated**. With the example emittance, a 10 mA DC beam at $\sigma = 1.33\ \mathrm{mm}$ has a ratio of about 0.26: still emittance dominated, but space charge already changes the matched size noticeably (next section).

## Tune depression and the smooth approximation {#space-charge-tune-depression}

### Smooth approximation

If the zero-current phase advance per focusing period $L_p$, $\sigma_0$, is not too large (roughly $\sigma_0 \lesssim 60°$–$80°$), the periodic focusing can be averaged into continuous focusing $k_0 = \sigma_0/L_p$. The rms envelope equation of a round DC beam becomes

$$
\sigma'' + k_0^2\,\sigma - \frac{K}{4\sigma} - \frac{\varepsilon^2}{\sigma^3} = 0 .
$$

### Matched size

For a matched beam $\sigma'' = 0$:

$$
k_0^2\sigma^4 - \frac{K}{4}\sigma^2 - \varepsilon^2 = 0
\quad\Longrightarrow\quad
\sigma^2 = \frac{K/4 + \sqrt{K^2/16 + 4k_0^2\varepsilon^2}}{2k_0^2} .
$$

At zero current $\sigma_{0,\mathrm{rms}} = \sqrt{\varepsilon/k_0}$.

### Depressed phase advance

A particle inside the beam feels the external focusing and the linear space-charge defocusing, so its wave number is $k^2 = k_0^2 - K/(4\sigma^2)$. The matching condition gives $k = \varepsilon/\sigma^2$; inserting and rearranging:

$$
\frac{\sigma}{\sigma_0} = \frac{k}{k_0} = \sqrt{1 + u^2} - u,\qquad
u = \frac{K}{8 k_0\,\varepsilon_{\mathrm{rms}}} = \frac{K}{2 k_0\,\varepsilon_{\mathrm{edge}}} ,
$$

where all emittances are **geometric** (unnormalised) and $\varepsilon_{\mathrm{edge}} = 4\varepsilon_{\mathrm{rms}}$. Here $\sigma$, $\sigma_0$ denote phase advances per period (not rms sizes); the ratio $\sigma/\sigma_0$ is the **tune depression**: 1 means no space charge, values approaching 0 mean extreme space-charge domination. Designs commonly keep $\sigma/\sigma_0 \gtrsim 0.4$–0.5.

### Numerical example

FODO period: quadrupoles $L_q = 0.1\ \mathrm{m}$, $G = 6\ \mathrm{T/m}$, drifts $L_d = 0.2\ \mathrm{m}$, period $L_p = 0.6\ \mathrm{m}$. AVAS's thick-lens matrices give $\sigma_0 = 52.7°$, $k_0 = 1.534\ \mathrm{m}^{-1}$ and a zero-current matched rms size of 1.254 mm.

| Beam | $K$ | $u$ | $\sigma/\sigma_0$ | $\sigma$ | Matched rms size |
|---|---|---|---|---|---|
| Example bunch ($I_{\mathrm{eq}} = 3.69\ \mathrm{mA}$) | $1.27\times 10^{-6}$ | 0.043 | 0.958 | 50.5° | 1.281 mm |
| 10 mA DC | $3.44\times 10^{-6}$ | 0.116 | 0.891 | 47.0° | 1.329 mm |
| 30 mA DC | $1.03\times 10^{-5}$ | 0.348 | 0.711 | 37.5° | 1.488 mm |

For the same period at $G = 4\ \mathrm{T/m}$ ($\sigma_0 = 34.5°$) the 30 mA DC beam is depressed to 0.60: the weaker the focusing, the larger the relative effect of space charge.

```avas-lab
space-charge I=10 W=1.527 mass=938.272 q=1 emit=0.13768 G=6 Lq=0.1 Ld=0.2 cells=10
```

> Note: the smooth approximation ignores the envelope flutter within a period, and its error grows with $\sigma_0$; exact matching requires solving the periodic envelope equations (see [Matching](#matching)). The equivalent $K$ of a bunch also changes with the bunch length; the first row of the table is only an estimate at the entry.

## Emittance growth and halo {#space-charge-emittance-growth}

In linear theory the rms emittance is conserved. Real beams are not KV, their self fields have non-linear parts, and the emittance grows. The main mechanisms (qualitatively):

- **Charge redistribution**: an rms-matched but non-uniform beam (Gaussian, or with a peaked core) relaxes towards a more uniform distribution within about a quarter of a plasma period, and the released "free energy" becomes emittance growth. For a round uniform beam the plasma wave number is $k_p^2 = 2K/a^2 = K/(2\sigma^2)$; in the 10 mA example $k_p = 0.99\ \mathrm{m}^{-1}$, a plasma wavelength of about 6.4 m. The stronger the space charge, the faster this happens and the larger the growth.
- **Mismatch and halo**: the envelope oscillations of a mismatched beam (for a round beam in the smooth approximation, breathing mode $k_{\mathrm{b}} = \sqrt{2k_0^2 + 2k^2}$, quadrupole mode $k_{\mathrm{q}} = \sqrt{k_0^2 + 3k^2}$) drive single particles through a 2:1 parametric resonance onto orbits far from the core, forming a **halo**. Halo particles are few and barely change the rms size, but they decide the losses.
- **Envelope instabilities and resonances**: with a zero-current phase advance $\sigma_0 > 90°$, space charge can drive envelope instabilities (hence the common design rule $\sigma_0 < 90°$); other parametric resonances of the periodic structure also cause growth.
- **Transverse–longitudinal coupling**: when the transverse and longitudinal "temperatures" (emittance over focusing) differ, space charge exchanges energy between the planes (the equipartition problem); Hofmann's stability charts show the dangerous parameter regions.

In AVAS output, compare the rms sizes in DataSet.txt (columns 16/18/20) with the maximum sizes (columns 22/24/26): a steadily growing ratio of maximum to rms usually signals developing halo. Emittance growth and halo require the multi-particle kernel; the linear preview contains neither.

## In AVAS {#space-charge-avas}

### Keywords

Space charge is controlled in `input.txt`, the beam charge comes from `beam.txt`:

```avas-params
input:spacecharge input:scmethod input:numofgrid input:meshrms input:steppercycle beam:current beam:frequency beam:beamtype beam:particlenumber spacechargecomp
```

- `spacecharge 1` turns space charge on, `0` off.
- `current` (mA, average current) and `frequency` (Hz, beam frequency) give the bunch charge $Q = I/f$; each macro-particle carries $Q$ divided by the number of macro-particles `particlenumber`. `beamtype dc` generates a DC beam.
- `scmethod` selects the solver: `FFT`, `PICNIC` or `SPICNIC`. The manual only lists the names and does not describe the algorithms; the example project uses `FFT`. When switching to another solver, compare with `FFT` on the same problem.
- `numofgrid Nx Ny Nz` sets the number of mesh points per direction; `meshrms V1 V2 V3` sets the mesh length $L_x = 2 V_1\sigma_x$ (likewise $y$, $z$), i.e. the mesh covers $\pm V_1$ rms sizes. The mesh must cover the whole bunch (tails included) while each cell still holds enough macro-particles; see [Multi-particle simulation in practice](#simulation).
- Space charge is computed every time step; the step is set by `steppercycle` (steps per RF period).
- The manual also states that with a longitudinal periodic boundary the space-charge solver switches to FFT automatically and the longitudinal mesh length is set to the bunch period, so more longitudinal mesh points are needed.
- The lattice keyword `spacechargecomp` (space-charge compensation) is not described in the manual; neither its parameters nor its effect are documented, so do not rely on it.
- `spacechargelong` and `spacechargetype` act only in the envelope model (`sim_type env`).

> Note: `boundary` in `input.txt` refers to the beam **loss** boundary (`boundary.txt`), not to a boundary condition of the space-charge solver. The boundary conditions, coordinate frame and interpolation of the kernel's space-charge solvers are not documented in the manual; the kernel is the reference.

### Space-charge model of the linear preview

The linear preview of the visual editor adds linear space charge when the beam current is positive:

- Bunched beams: the rms-equivalent uniform ellipsoid (semi-axes $\sqrt5\,\sigma_{x,y,z}$), $M_z$ from the spheroid formula with the rest-frame aspect ratio $p = \gamma c_z/\sqrt{ab}$, transversely $M_x = (1 - M_z)\,b/(a + b)$, i.e. the linear $K_3$ force above. It needs `frequency` and `twissz`; without them space charge is left out with a warning.
- DC beams (`beamtype dc`): the rms-equivalent uniform elliptical cross-section (semi-axes $2\sigma$), i.e. the KV force $2K/(a(a+b))$.
- A thin kick with the strength integrated over 4 steps is applied in the middle of each group of steps (second-order splitting), with the force updated from the current rms sizes.

It describes only the linear part of the rms envelope, without emittance growth, halo or losses. With strong space charge its rms sizes can differ from the kernel's by up to 13 %.

## Further reading {#space-charge-further}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: self fields, KV distribution, envelope equations, emittance growth.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: multiparticle dynamics with space charge and halo.
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18, 1105 (1971).
- I. M. Kapchinsky, V. V. Vladimirsky, Proc. Int. Conf. on High Energy Accelerators, CERN, 1959 (KV distribution).
- I. Hofmann, "Stability of anisotropic beams with space charge", *Phys. Rev. E* 57, 4713 (1998).
- In this primer: [Linear optics](#linear-optics), [Matching](#matching), [Multi-particle simulation in practice](#simulation).
