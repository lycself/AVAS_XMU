# Beam matching

Send the example beam straight into a FODO channel and its rms $x$ jumps irregularly between 0.71 mm and 2.80 mm; put four well-tuned quadrupoles in front of the channel and the same beam in the same channel only swings regularly between 1.02 mm and 1.86 mm, repeating every period. The channel is the same, the emittance is the same; only the shape and tilt of the phase-space ellipse at the entrance differ. This chapter explains why: what matching means, how mismatch is measured, what mismatch does under space charge, how many knobs a match needs, and how to do it in AVAS.

The chapter uses the notation of [phase space](#phase-space) and [linear optics](#linear-optics): Twiss parameters $(\alpha, \beta, \gamma)$ with $\beta\gamma - \alpha^2 = 1$, rms geometric emittance $\varepsilon$, beam sigma matrix

$$
\Sigma = \varepsilon \begin{pmatrix} \beta & -\alpha \\ -\alpha & \gamma \end{pmatrix},\qquad x_{\mathrm{rms}} = \sqrt{\beta\varepsilon}
$$

The choice of periods and phase advances is in [lattice design](#lattice-design), space charge in [space charge](#space-charge).

## Definition of the matched beam {#matching-definition}

### A rotating ellipse

Think first of all the particles in a periodic channel. In suitable normalized coordinates every particle turns about the origin of phase space by the same angle $\sigma$ in each period. If the beam is a circle in these coordinates, it is the same circle after the rotation and the beam size is the same in every period: the beam is **matched**. If the beam is an elongated ellipse, the rotation makes it tumble like a rugby ball: its width projected onto the $x$ axis (the beam size) grows and shrinks. After half a turn the ellipse shows the same projection again, so the envelope oscillates at twice the particle frequency.

This picture relies only on linear motion; space charge and non-linearities change the details, but the definition "matched = the phase-space ellipse coincides with the eigen-ellipse of the periodic structure" holds in every case.

### Zero-current periodic solution

Let $M$ be the 2×2 transfer matrix of one period, with $\cos\sigma = \tfrac12\Tr M$ and $\lvert\cos\sigma\rvert < 1$. Writing $M$ in Twiss form,

$$
M = \begin{pmatrix} \cos\sigma + \alpha\sin\sigma & \beta\sin\sigma \\ -\gamma\sin\sigma & \cos\sigma - \alpha\sin\sigma \end{pmatrix}
$$

gives the periodic solution directly:

$$
\beta = \frac{M_{12}}{\sin\sigma},\qquad \alpha = \frac{M_{11} - M_{22}}{2\sin\sigma},\qquad \gamma = -\frac{M_{21}}{\sin\sigma}
$$

The sign of $\sin\sigma$ is that of $M_{12}$, so that $\beta > 0$. These $(\alpha, \beta)$ satisfy $M\Sigma M\tp = \Sigma$: a beam with these Twiss parameters has the same ellipse after one period — the **matched beam**. The periodic solution depends on where the period is taken to start; at the centre of a FODO's F quadrupole $\alpha = 0$ and $\beta$ is largest, at the centre of the D quadrupole $\alpha = 0$ and $\beta$ is smallest.

Example: the FODO of [lattice design](#lattice-design-periods) ($G = 5\ \mathrm{T/m}$, $L_q = 0.1\ \mathrm{m}$, $L_d = 0.3\ \mathrm{m}$), with the period starting at the F-quadrupole entrance, has the periodic solution $\alpha_x = -1.762$, $\beta_x = 1.256\ \mathrm{m}$ in x and $\alpha_y = 0.782$, $\beta_y = 0.493\ \mathrm{m}$ in y. The example beam enters with $\alpha_x = -0.658$, $\beta_x = 0.377\ \mathrm{m}$, far away from it.

### With space charge

The space-charge force depends on the beam size itself, so the matched solution must be self-consistent. For a continuous beam (KV model, $a$, $b$ edge radii, $a = 2x_{\mathrm{rms}}$) the envelope equations are

$$
a'' + k_x(s)\,a - \frac{2K}{a+b} - \frac{\varepsilon_x^2}{a^3} = 0,\qquad
b'' + k_y(s)\,b - \frac{2K}{a+b} - \frac{\varepsilon_y^2}{b^3} = 0
$$

with the edge emittance $\varepsilon$ ($4\varepsilon_{\mathrm{rms}}$) and the generalized perveance $K$ (see [space charge](#space-charge)). Each term has a plain meaning: $k_x a$ is the external focusing pulling the envelope in, $2K/(a+b)$ is space charge pushing it out, $\varepsilon^2/a^3$ is the emittance (the transverse "temperature" of the particles) pushing it out. The matched beam is the solution of these equations with period $L_p$; it changes with current, and zero-current matched parameters used for an intense beam produce a mismatch. Bunched beams are similar in three dimensions, and the longitudinal envelope must be matched too.

### With acceleration

Acceleration increases $\beta\gamma$, the geometric emittance shrinks adiabatically as $1/(\beta\gamma)$, and the periodic solution changes slowly with energy. "Matched" then means that the envelope follows the slowly changing periodic solution, not that it repeats exactly period by period. The Twiss parameters in AVAS's `beam.txt` are the values **at the entrance** (β in mm/mrad, emittance normalized rms in π·mm·mrad); the entrance rms size is $\sqrt{\beta\varepsilon_n/(\beta\gamma)}$.

In the phase-space lab send an ellipse through a thin lens and a drift, and see how α and β together decide the downstream size:

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0.5 L=0.5
```

## Mismatch factor {#matching-mismatch-factor}

### Definition and geometric meaning

Comparing a beam $(\alpha, \beta, \gamma)$ with the matched ellipse $(\alpha_0, \beta_0, \gamma_0)$ (same place, same emittance), define

$$
R = \beta\gamma_0 + \beta_0\gamma - 2\alpha\alpha_0,\qquad
M = \left[\frac{1}{2}\left(R + \sqrt{R^2 - 4}\right)\right]^{1/2} - 1
$$

This is the definition of Wangler's book and of common envelope codes. It comes straight from the rotation picture above: in the normalized coordinates of the matched ellipse the matched ellipse is a circle of radius $\sqrt{\varepsilon}$, and the mismatched ellipse has semi-axes $\sqrt{\lambda_\pm\varepsilon}$,

$$
\lambda_\pm = \tfrac12\left(R \pm \sqrt{R^2-4}\right),\qquad \lambda_+\lambda_- = 1
$$

(equal areas). So $1 + M = \sqrt{\lambda_+}$ is the factor by which the mismatched ellipse exceeds the matched one in the worst direction. Properties:

- $R \ge 2$, and $R = 2$, $M = 0$ exactly when the ellipses coincide.
- $R = \Tr(S S_0^{-1})$ with $S = \begin{pmatrix}\beta & -\alpha\\ -\alpha & \gamma\end{pmatrix}$. In linear transport $S \to MSM\tp$ and the trace is invariant, so **in linear transport without space charge the mismatch factor is conserved along the line** and can be computed anywhere.
- To enclose a mismatched beam completely in an ellipse of the matched shape takes an emittance of $(1+M)^2\varepsilon$.
- Without space charge the envelope of a mismatched beam beats between $1/(1+M)$ and $1+M$ times the matched envelope.

### Some numbers

Where $\alpha_0 = \alpha = 0$: $\beta = 1.2\beta_0$ gives $R = 1.2 + 1/1.2$ and $M = \sqrt{1.2} - 1 = 0.095$; $\beta = 1.44\beta_0$ (a beam 20 % too large) gives $M = 0.200$. With $\beta = \beta_0$ but $\alpha$ off by 0.5, $M = 0.281$.

> **Think about it**: the beam size is exactly right ($\beta = \beta_0$), yet the mismatch factor is 0.28, worse than a beam 20 % too large. What does that tell you?
>
> Answer: the size is only the projection of the ellipse onto one direction. With the wrong $\alpha$ the ellipse has the wrong tilt; it is still an elongated ellipse in phase space, and after about $45^\circ$ of betatron phase (a quarter of the envelope oscillation) its long axis has turned towards $x$ and its projection is clearly larger. Matching the beam size alone is incomplete.

The example from the beginning: the example beam, after a 0.1 m drift, entering the FODO above directly has $M_x = 0.58$ and $M_y = 1.23$ with respect to the periodic solution at the F-quadrupole entrance. By the properties above the x envelope can reach at most 1.58 times the largest matched envelope (about 1.80 mm), i.e. about 2.85 mm; the linear preview gives 2.80 mm, in agreement. With space charge switched off the preview gives 2.77 mm: the bound is not quite reached because within 12 periods the phase of the envelope oscillation never falls exactly on the β maximum; the weak space charge of 0.18 mA adds only about 0.03 mm. Here is that mismatched beam line; note the irregular swings of the envelope from period to period:

```avas-beamline
drift 0.1 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
```

In the FODO lab set `mismatch` to 1 (matched), 1.2 and 1.44 and watch the envelope beat about the periodic envelope. `mismatch` is a scale factor $s$ of the initial size: the lab multiplies β at the cell entrance by $s^2$ and keeps α. Scaling like that where $\alpha = 0$ would give $M = s - 1$; but at the cell entrance $\alpha \approx -1.76$, and the same scaling is a larger mismatch: $M \approx 0.44$ for $s = 1.2$ and $M \approx 1.0$ for $s = 1.44$.

```avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1.2
```

### Mismatch from small errors

After a thin-lens error $\Delta(1/f)$ (a quadrupole gradient error, for example) a matched beam keeps $\beta$ while $\alpha \to \alpha + \beta\,\Delta(1/f)$; substituting gives $R = 2 + \beta^2\Delta(1/f)^2$, hence

$$
M \approx \frac{\beta\,\Delta(1/f)}{2}
$$

The mismatch is proportional to the local β: the same error does more harm where the beam is large. As a rule of thumb $M \lesssim 0.05$–$0.1$ counts as a good match, but what is acceptable depends on current and aperture margin (see [errors and correction](#errors)).

## Envelope modes {#matching-envelope-modes}

Linearizing the envelope equations about the matched solution gives the eigenmodes of mismatch oscillations. For uniform focusing $k_0^2$, a round beam $a = b$ and $\varepsilon_x = \varepsilon_y$, the matching condition is $k_0^2 a = K/a + \varepsilon^2/a^3$; with the particle wave number with space charge $k = \varepsilon/a^2$:

- **breathing mode** ($\delta a = \delta b$, both planes in phase): $k_{\mathrm{b}}^2 = 2k_0^2 + 2k^2$;
- **quadrupole mode** ($\delta a = -\delta b$, planes in antiphase): $k_{\mathrm{q}}^2 = k_0^2 + 3k^2$.

Outline of the derivation: set $a = a_0 + \delta a$ and expand every term to first order. The external focusing gives $k_0^2\,\delta a$; the derivative of the emittance term $\varepsilon^2/a^3$ is $-3\varepsilon^2/a^4 = -3k^2$, a restoring force $3k^2\,\delta a$. The space-charge term $2K/(a+b)$ depends only on $a + b$: an antiphase perturbation leaves $a + b$ unchanged to first order, so the quadrupole mode feels only $k_0^2$ and $3k^2$; an in-phase perturbation changes the term by $-K/a^2\,\delta a$, which acts as a restoring force $K/a^2\,\delta a$ in the equation, and eliminating $K$ with the matching condition $K/a^2 = k_0^2 - k^2$ gives the breathing mode. At zero current $k = k_0$ and both modes are $2k_0$; for very strong space charge $k \to 0$, $k_{\mathrm{b}} \to \sqrt2\,k_0$ and $k_{\mathrm{q}} \to k_0$. Axisymmetric bunched beams have three modes in three dimensions: the transverse quadrupole mode and a high- and a low-frequency mode formed by coupling the transverse breathing and the longitudinal oscillation.

Example: the FODO above has a zero-current phase advance of $61.4^\circ$, and both modes advance $122.9^\circ$ per period. At a DC current of 10 mA, $\sigma/\sigma_0 = 0.876$, i.e. $k^2/k_0^2 = 0.767$; the breathing mode becomes $\sqrt{2 + 2\times 0.767} = 1.88$ times $k_0$ ($115.5^\circ$ per period), the quadrupole mode $\sqrt{1 + 3\times 0.767} = 1.82$ times ($111.6^\circ$).

Physical consequences of mismatch:

1. The envelope oscillates at a frequency different from the particles', so the particles are driven periodically around the oscillating core; non-linear space-charge forces filament the phase space and the rms emittance finally grows. Reiser estimates the emittance growth from the "free energy" released when the mismatch energy is converted completely: the larger the mismatch and the stronger the space charge, the larger the growth.
2. In periodic focusing, the envelope instability occurs when an envelope mode approaches $180^\circ$ per period — the origin of the $\sigma_0 < 90^\circ$ rule in [lattice design](#lattice-design-phase-advance).
3. Parametric resonance between particles and the oscillating core drives some particles to large amplitudes and forms a halo (see [rms matching and halo](#matching-halo)).

Raise the DC current `I` of the FODO lab to 10–20 mA: the zero-current periodic Twiss parameters are no longer matched and the envelope starts to beat; the rhythm of the beating corresponds to the breathing and quadrupole modes above.

```avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=15 mismatch=1
```

## Matching sections: knobs and constraints {#matching-knobs}

### Number of constraints

A full 6D match requires at the target

| Plane | Constraints | Number |
|---|---|---|
| x | $\alpha_x$, $\beta_x$ | 2 |
| y | $\alpha_y$, $\beta_y$ | 2 |
| z | $\alpha_z$, $\beta_z$ | 2 |

The emittances are fixed by the beam itself and are not matching variables (linear transport cannot change them). At least as many **independent** knobs as constraints are therefore needed:

- Transversely with quadrupoles: at least 4 (4 gradients) to match $(\alpha_x, \beta_x, \alpha_y, \beta_y)$; 4–6 are common in practice, the extra freedom being used to limit the largest β, the gradients and the sensitivity to errors.
- A solenoid acts on both planes alike: one solenoid changes x and y together, a round beam has only the two constraints $(\alpha, \beta)$, so at least 2 solenoids are needed; they cannot match x and y to different values.
- Longitudinally with bunchers: a buncher at $\varphi_s = -90^\circ$ adds no energy and only gives an energy offset proportional to the phase offset, a longitudinal thin lens; at least 2 bunchers (with their amplitudes as knobs) match $(\alpha_z, \beta_z)$. The first cavities of the following accelerating section can serve as well.

### An example: four quadrupoles

Let us solve the problem of the beginning. In front of the FODO put a matching section: a 0.1 m drift, four 0.1 m quadrupoles 0.2 m apart, and 0.3 m after the last one before the FODO. At the FODO entrance $(\alpha, \beta)$ in x and y must equal the periodic solution: four equations for four unknown gradients. Solving them with thick-lens matrices (from many random starting points, keeping the solution with the smallest largest gradient) gives $G_1 \ldots G_4 = -3.11$, $8.35$, $-7.43$, $3.96\ \mathrm{T/m}$.

```avas-beamline
drift 0.1 0.02 0
quad 0.1 0.02 0 -3.11
drift 0.2 0.02 0
quad 0.1 0.02 0 8.35
drift 0.2 0.02 0
quad 0.1 0.02 0 -7.43
drift 0.2 0.02 0
quad 0.1 0.02 0 3.96
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
quad 0.1 0.02 0 5
drift 0.3 0.02 0
quad 0.1 0.02 0 -5
drift 0.3 0.02 0
```

After the matching section the envelope repeats every period: at zero current the rms $x$ at every F-quadrupole entrance lies between $1.739$ and $1.744\ \mathrm{mm}$, in agreement with the $1.74\ \mathrm{mm}$ of the periodic solution. With the 0.18 mA space charge of the example beam the same positions vary between $1.72$ and $1.80\ \mathrm{mm}$, a residual beat of about $\pm 2.5\ \%$. 0.18 mA is the average current, but the bunch is short: at the entrance the bunch charge is $1.13\ \mathrm{pC}$ and the rms length 1.32 mm, which for a Gaussian longitudinal distribution means a peak current of about 6 mA, so space charge is not entirely negligible. This illustrates the rule below: with space charge, a zero-current match is only a starting point.

> **Think about it**: are these four gradients the only solution?
>
> Answer: no. The equations are non-linear, and different starting points give different solutions, some with larger gradients, some with very large β inside the matching section. The one above has the smallest largest gradient. When choosing a solution, look at the envelope through the whole matching section, not only at the parameters at its end.

### Practical rules

- Keep β moderate inside the matching section: where β is large the beam is large and more sensitive to quadrupole displacements and gradient errors (mismatch $M \approx \beta\,\Delta(1/f)/2$).
- With space charge the matched parameters depend on current; iterate at the actual current, the zero-current result is only a starting value.
- The target is the periodic Twiss parameters of the downstream period at its entrance, not an arbitrary "small spot".
- Non-linear problems can have several solutions; check that the envelope through the whole matching section is reasonable, not only the end parameters.

## Matching between sections {#matching-sections}

The general procedure for matching between sections:

1. Find the periodic matched parameters at the entrance of the downstream section (at zero current with the formulas above; with space charge from a self-consistent envelope or a multi-particle simulation).
2. Determine the Twiss parameters of the upstream beam at the entrance of the matching section (from upstream simulation or measurement).
3. Use the knobs of the matching section to transform 2 into 1 within the gradient and aperture limits.
4. Check with a multi-particle simulation that the envelope is periodic over the next periods and that the emittance does not grow.

The typical case is the MEBT between the RFQ exit and the first accelerating section (see [the MEBT](#journey-mebt)): the beam leaving an RFQ is usually strongly converging or diverging transversely, asymmetric between the planes, and needs bunchers to control its length. If [lattice design](#lattice-design-transitions) keeps the phase advance per metre continuous between sections, the periodic parameters of neighbouring sections are close and the matching section can be short or even unnecessary.

> **Note**: in an accelerating section the "periodic solution" of the first downstream period itself changes with energy, so exact periodic matched parameters do not exist. One usually matches to the local periodic solution of the first period at its entrance energy and checks with a simulation.

## rms matching and halo {#matching-halo}

rms matching only makes the second moments (rms sizes, Twiss parameters) right. The tails of the real distribution are also shaped by:

- **Particle–core resonance**: a mismatched core oscillates at the envelope breathing frequency, and a particle crossing the core feels a periodic non-linear force; particles oscillating at about half the core frequency are driven resonantly to amplitudes far beyond the core. It is the same parametric resonance as pumping a swing by standing up and squatting: the parameter (the length of the pendulum) changes twice per back-and-forth, and the swing goes ever higher. The particle–core model (Gluckstern; Wangler et al.) gives a maximum halo amplitude that grows with the mismatch, roughly logarithmically; as long as there is mismatch, an intense beam forms a halo.
- **Non-linear external fields**: higher-order components of field maps and the fringe fields of solenoids and cavities distort the phase-space ellipse.
- **Initial distribution**: the tails of a Gaussian already reach several rms, a KV distribution has none; the choice of KV / GS / PB / WB in AVAS's `distribution` affects loss estimates (see [initial distributions](#distributions)).

In AVAS a direct halo indicator is the ratio of maximum to rms size (DataSet columns 22/24 against 16/18), together with the emittance growth along the line and the number of surviving particles (column 28). A good rms match with a steadily growing max/rms ratio usually points to a non-linear or resonance problem.

## Common misconceptions {#matching-misconceptions}

- **Right size is not matched**: with $\beta = \beta_0$ and $\alpha$ off by 0.5, $M = 0.28$. A match must constrain both $\alpha$ and $\beta$.
- **Matching to a "small spot"**: the target is the periodic solution of the downstream period, not the smallest beam somewhere.
- **When the mismatch factor is conserved**: only in linear transport without space charge; with space charge or non-linearities mismatch turns into emittance growth and can no longer be described completely by $M$.
- **Zero-current matching for an intense beam**: space charge changes the periodic solution, so zero-current parameters are themselves a mismatch at finite current; the example above already shows a residual beat of about 2.5 % at 0.18 mA.
- **"Roundness" is not mismatch**: the AI assistant's metric `rms_mismatch_out` is $\lvert x_{\mathrm{rms}} - y_{\mathrm{rms}}\rvert$, something different from the Twiss mismatch factor of this chapter.
- **Definitions in different codes**: the mismatch factor is written in different ways; check the definition before comparing codes. This chapter uses Wangler's.

## In AVAS {#matching-in-avas}

### Setting the input beam

The Beam page sets `twissx`, `twissy`, `twissz` (α, β, ε) with the units listed below:

```avas-params
beam:twissx beam:twissy beam:twissz beam:distribution
```

β is in mm/mrad (numerically m/rad), ε is the normalized rms emittance (π·mm·mrad), twissz uses z (mm) and $z' = \Delta p/p$ (mrad). The example beam `twissx -0.65785 0.37743 0.13768` has an entrance $x_{\mathrm{rms}} = \sqrt{0.37743 \times 0.13768/0.05708} = 0.954\ \mathrm{mm}$.

### Interactive: the linear preview

While you drag quadrupole gradients, solenoid `Kb` and other parameters in the visual editor, the linear envelope preview updates at once (tick "Space charge in the preview" to include linear space charge). The visual test of a match: inside a periodic section the envelope repeats every period without beating from period to period. The preview is an approximate model (rms usually within 1–2 %, up to 13 % where multi-particle effects are strong) without losses or non-linearities; confirm with the engine in the end.

### Scans and optimization

- **Parameter scan page** (or the command line `avas scan --input DIR --target element --param parameter --values …`): runs the engine for a list of values of one parameter in copies under `Scans/` and compares the results.
- **The AI assistant's `optimize` tool**: varies parameters of several elements within bounds to minimise or maximise an objective. Metrics available with the preview engine are `rms_x_max`, `rms_y_max`, `rms_x_out`, `rms_y_out`, `energy_out` and `rms_mismatch_out`; the objective may be an expression such as `abs(rms_x_out-1.5)+abs(rms_y_out-1.5)`. Note that `rms_mismatch_out` is the end value of $\lvert x_{\mathrm{rms}} - y_{\mathrm{rms}}\rvert$ (roundness), not the Twiss mismatch factor of this chapter. The simulation engine adds metrics such as transmission and emittance growth. The result is proposed as an edit and is written to the lattice only after the user approves it.

### Size targets with adjust and diag_size

The static error-correction mechanism can also tune parameters to beam-size targets (details in [errors and correction](#errors-in-avas)):

```avas-params
adjust diag_size
```

- `adjust 0 v n min max first_step` goes in front of the element to tune; `v` is the element's v-th parameter (counted from 1 after the keyword: the gradient of `quad` is the 4th, `Kb` of `field` the 8th, `Ke` the 7th), knobs with the same `n` take the same value, `first_step = 1` starts the optimization from the current value.
- `diag_size 0 sx sy 0` gives the target **rms** size (mm) at its position. The current code pairs `adjust` and `diag_*` by their **first parameter** (equal numbers form one correction, done in order of the numbers); the manual calls this parameter "meaningless, write 0", and writing 0 everywhere makes one group.
- The optimizer is SLSQP (each function evaluation is one engine run); the objective is the mean of $(x_{\mathrm{rms}} - s_x)^2 + (y_{\mathrm{rms}} - s_y)^2$ (mm²) over the group's diagnostics, it stops below 0.05, after at most 100 iterations, with a finite-difference step of 0.1 (in the parameter's own unit).
- The mechanism runs only in a **static error study** (`error_type` `stat` or `stat_dyn` in `ini.ini`) and needs `err_step` and at least one error switch turned on (a static one in `stat` mode). The result goes to `Adjust_Datas_<group>_<run>.txt` and is **not written back to the lattice**.

> **Note**: a single `diag_size` constrains only the size, not α, so it cannot fix a match; size targets at two places about $90^\circ$ of phase advance apart constrain $(\alpha, \beta)$ indirectly. Borrowing this mechanism with zero-amplitude static errors for an error-free match is a use inferred from the current code and not systematically verified with the engine; check the results of every run.

### Checking

- Results page: rms envelope, Twiss α/β along the line, emittance; the "Phase advance" plot (which needs `lattice … lattice_end` period markers) gives the phase advance of the actual beam per period as $\int\dd z/\beta$.
- The Run page shows the rms envelope live; "Compare with the run before" overlays the previous run to compare before and after matching.

> **AVAS convention**: the old matching commands `MATCHING`, `SETTWISS`, `CIRCLE_MATCH` described in the manual belong to an old workflow, are not verified with the current interface and should not be used as current features.

## Further reading {#matching-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: mismatch factor, envelope modes, the particle–core model of halo.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.: envelope equations, free energy of mismatch and emittance growth; and M. Reiser (1991) on the free energy of non-stationary beams.
- F. J. Sacherer (1971): the rms envelope equations, which make rms matching valid for all elliptically symmetric distributions.
- J. Struckmeier, M. Reiser (1984): envelope oscillations and instabilities of mismatched beams in periodic channels.
- The complete list is in [references](#references).
