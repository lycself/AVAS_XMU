# Beam matching

A periodic structure has its own "eigen" phase ellipse — the periodic Twiss parameters. When the phase ellipse of the incoming beam agrees with it, the beam envelope repeats every period; when it does not, the envelope oscillates around the periodic envelope (mismatch), and nonlinear forces and space charge turn this into emittance growth and halo. This chapter gives the definition of matching, the mismatch factor, the envelope oscillation modes, the number of knobs a match needs, and how to match in AVAS.

The notation follows [phase space](#phase-space) and [linear optics](#linear-optics): Twiss parameters $(\alpha, \beta, \gamma)$ with $\beta\gamma - \alpha^2 = 1$, rms geometric emittance $\varepsilon$, beam sigma matrix

$$
\Sigma = \varepsilon \begin{pmatrix} \beta & -\alpha \\ -\alpha & \gamma \end{pmatrix},\qquad x_{\mathrm{rms}} = \sqrt{\beta\varepsilon}
$$

## Definition of a matched beam {#matching-definition}

### The zero-current periodic solution

Let $M$ be the 2×2 transfer matrix of one period with $\cos\sigma = \tfrac12\Tr M$ and $\lvert\cos\sigma\rvert < 1$. Writing $M$ in Twiss form

$$
M = \begin{pmatrix} \cos\sigma + \alpha\sin\sigma & \beta\sin\sigma \\ -\gamma\sin\sigma & \cos\sigma - \alpha\sin\sigma \end{pmatrix}
$$

gives the periodic solution:

$$
\beta = \frac{M_{12}}{\sin\sigma},\qquad \alpha = \frac{M_{11} - M_{22}}{2\sin\sigma},\qquad \gamma = -\frac{M_{21}}{\sin\sigma}
$$

The sign of $\sin\sigma$ is taken equal to that of $M_{12}$ so that $\beta > 0$. These $(\alpha, \beta)$ satisfy $M\Sigma M\tp = \Sigma$: a beam with these Twiss parameters has the same phase ellipse after one period — the **matched beam**. The periodic solution depends on where the period is taken to start; at the centre of a FODO F quadrupole $\alpha = 0$ and $\beta$ is maximal, at the centre of the D quadrupole $\alpha = 0$ and $\beta$ is minimal.

### With space charge

The space-charge force depends on the beam size itself, so the matched solution must be self-consistent. For a continuous beam (KV model, edge radii $a$, $b$, $a = 2x_{\mathrm{rms}}$) the envelope equations are

$$
a'' + k_x(s)\,a - \frac{2K}{a+b} - \frac{\varepsilon_x^2}{a^3} = 0,\qquad
b'' + k_y(s)\,b - \frac{2K}{a+b} - \frac{\varepsilon_y^2}{b^3} = 0
$$

with $\varepsilon$ the edge emittance ($4\varepsilon_{\mathrm{rms}}$) and $K$ the generalised perveance (see [space charge](#space-charge)). The matched beam is the solution of these equations with period $L_p$; it changes with the current, and zero-current matched parameters applied to an intense beam produce a mismatch. Bunched beams are similar in three dimensions, and the longitudinal envelope must be matched as well.

### With acceleration

Acceleration increases $\beta\gamma$, the geometric emittance shrinks adiabatically as $1/(\beta\gamma)$, and the periodic solution also changes slowly with energy. "Matched" then means that the envelope follows the slowly varying periodic solution rather than repeating exactly from period to period. The Twiss parameters in AVAS's `beam.txt` are the values **at the entrance** (β in mm/mrad, emittance normalised rms in π·mm·mrad); the entrance rms size is $\sqrt{\beta\varepsilon_n/(\beta\gamma)}$.

## The mismatch factor {#matching-mismatch-factor}

To compare a beam $(\alpha, \beta, \gamma)$ with the matched ellipse $(\alpha_0, \beta_0, \gamma_0)$ (same place, same emittance) define

$$
R = \beta\gamma_0 + \beta_0\gamma - 2\alpha\alpha_0,\qquad
M = \left[\frac{1}{2}\left(R + \sqrt{R^2 - 4}\right)\right]^{1/2} - 1
$$

This is the definition used in Wangler's book and common envelope codes. Properties:

- $R \ge 2$, with $R = 2$ and $M = 0$ if and only if the two ellipses coincide.
- $R = \Tr(S S_0^{-1})$ with $S = \begin{pmatrix}\beta & -\alpha\\ -\alpha & \gamma\end{pmatrix}$. In linear transport $S \to MSM\tp$ and the trace is invariant, so **in linear transport without space charge the mismatch factor is conserved along the line** and can be evaluated anywhere.
- Geometric meaning: in the normalised coordinates of the matched ellipse the matched ellipse is a circle of radius $\sqrt{\varepsilon}$ and the mismatched ellipse has semi-axes $\sqrt{\lambda_\pm\varepsilon}$, $\lambda_\pm = \tfrac12(R \pm \sqrt{R^2-4})$, $\lambda_+\lambda_- = 1$ (equal areas). So $1 + M = \sqrt{\lambda_+}$ is the factor by which the mismatched ellipse exceeds the matched one in the worst direction; enclosing the mismatched beam in an ellipse of the matched shape needs an emittance $(1+M)^2\varepsilon$.
- Without space charge the envelope of a mismatched beam beats between $1/(1+M)$ and $1+M$ times the matched envelope.

Examples (at a place with $\alpha_0 = \alpha = 0$): $\beta = 1.2\beta_0$ gives $R = 1.2 + 1/1.2$ and $M = \sqrt{1.2} - 1 = 0.095$; $\beta = 1.44\beta_0$ (beam 20 % too large) gives $M = 0.200$. With $\beta = \beta_0$ but $\alpha$ off by 0.5, $M = 0.281$ — errors in α matter just as much.

A useful small-error result: after a thin-lens error $\Delta(1/f)$ (for example a quadrupole gradient error) a matched beam keeps $\beta$ while $\alpha \to \alpha + \beta\,\Delta(1/f)$; substituting gives $R = 2 + \beta^2\Delta(1/f)^2$ and therefore

$$
M \approx \frac{\beta\,\Delta(1/f)}{2}
$$

Empirically $M \lesssim 0.05$–$0.1$ counts as a good match, but the acceptable value depends on the current and the aperture margin (see [errors and correction](#errors)).

## Envelope oscillation modes {#matching-envelope-modes}

Linearising the envelope equations around the matched solution gives the eigenmodes of the mismatch oscillation. For uniform focusing $k_0^2$ and a round beam $a = b$, $\varepsilon_x = \varepsilon_y$, the matching condition is $k_0^2 a = K/a + \varepsilon^2/a^3$; write $k = \varepsilon/a^2$ for the depressed particle wave number:

- **breathing mode** ($\delta a = \delta b$, planes in phase): $k_{\mathrm{b}}^2 = 2k_0^2 + 2k^2$;
- **quadrupole mode** ($\delta a = -\delta b$, planes in antiphase): $k_{\mathrm{q}}^2 = k_0^2 + 3k^2$.

Outline of the derivation: for an antisymmetric perturbation $2K/(a+b)$ is unchanged to first order, so the quadrupole mode feels only $k_0^2$ and the emittance term $3\varepsilon^2/a^4 = 3k^2$; for a symmetric perturbation the space-charge term contributes $K/a^2 = k_0^2 - k^2$. At zero current $k = k_0$ and both modes are at $2k_0$; in the extreme space-charge limit $k \to 0$, $k_{\mathrm{b}} \to \sqrt2\,k_0$ and $k_{\mathrm{q}} \to k_0$. Axisymmetric bunched beams have three modes in three dimensions: the transverse quadrupole mode, and a high-frequency and a low-frequency mode in which the transverse breathing and the longitudinal oscillation are coupled.

Physical consequences of a mismatch:

1. The envelope frequency differs from the particle frequency, so particles are driven periodically by the oscillating core; nonlinear space-charge forces filament the phase space and the rms emittance eventually grows. Reiser's "free energy" picture estimates the emittance growth when the mismatch energy is fully converted; larger mismatch and stronger space charge give more growth.
2. In periodic focusing, the envelope instability occurs when an envelope mode advances by about $180^\circ$ per period — the origin of the $\sigma_0 < 90^\circ$ rule of [lattice design](#lattice-design-phase-advance).
3. The parametric resonance between particles and the oscillating core drives some particles to large amplitudes and forms a halo (see [rms matching and halo](#matching-halo)).

## Matching sections: knobs and constraints {#matching-knobs}

### Number of constraints

A full 6D match must satisfy at the target point

| Plane | Constraints | Number |
|---|---|---|
| x | $\alpha_x$, $\beta_x$ | 2 |
| y | $\alpha_y$, $\beta_y$ | 2 |
| z | $\alpha_z$, $\beta_z$ | 2 |

The emittances belong to the beam and are not matching variables. At least as many **independent** knobs as constraints are therefore needed:

- Transversely with quadrupoles: at least 4 (four gradients) to match $(\alpha_x, \beta_x, \alpha_y, \beta_y)$; in practice 4–6 are common, the extra freedom being used to limit the maximum β, the gradients and the sensitivity to errors.
- A solenoid acts identically on both planes: one solenoid changes x and y together, a round beam has only the two constraints $(\alpha, \beta)$, so at least 2 solenoids are needed, and they cannot match x and y to different values.
- Longitudinally with bunchers: a buncher at $\varphi_s = -90^\circ$ gives no energy gain, only an energy offset proportional to the phase offset — a longitudinal thin lens; at least 2 bunchers (amplitudes as knobs) match $(\alpha_z, \beta_z)$. The first cavities of the downstream accelerating section can be used too.

### Practical rules

- Avoid large β inside the matching section: the beam is large there and more sensitive to quadrupole displacements and gradient errors (mismatch $M \approx \beta\,\Delta(1/f)/2$).
- With space charge the matched parameters depend on the current; iterate at the actual current, using the zero-current result only as a start.
- The target is the periodic Twiss parameters of the downstream period at its entrance, not an arbitrary "small spot".
- The nonlinear problem can have several solutions; check that the envelope through the whole matching section is reasonable, not only the end parameters.

## Matching between sections {#matching-sections}

The general procedure for matching between sections:

1. Find the periodic matched parameters at the entrance of the downstream section (with the formulas above at zero current; with self-consistent envelopes or multi-particle simulations with space charge).
2. Determine the Twiss parameters of the upstream beam at the entrance of the matching section (from upstream simulations or measurements).
3. Use the knobs of the matching section to transform 2 into 1 within the gradient and aperture limits.
4. Check with a multi-particle simulation that the envelope is periodic over several downstream periods and that the emittance does not grow.

The typical case is the MEBT between the RFQ exit and the first accelerating section: the beam leaving an RFQ usually converges or diverges strongly and differently in the two planes, and bunchers are needed to control the bunch length. If the [lattice design](#lattice-design-transitions) keeps the phase advance per metre continuous, the periodic parameters of the two sections are close and the matching section can be short or even omitted.

> Note: in an accelerating section the "periodic solution" of the first downstream period itself changes with energy, so strictly periodic matched parameters do not exist. One usually matches to the local periodic solution of the first period at its entrance energy and checks with simulations.

## rms matching and halo {#matching-halo}

rms matching only makes the second moments (rms sizes, Twiss parameters) right. The tails of the real distribution are also affected by:

- **Particle–core resonance**: a mismatched core oscillates at the envelope breathing frequency and a particle passing through it feels a periodic nonlinear force; particles whose oscillation frequency is about half the core frequency are resonantly driven to amplitudes well beyond the core. The particle–core model (Gluckstern; Wangler et al.) gives a maximum halo amplitude that grows with the mismatch, approximately logarithmically; any mismatch lets an intense beam form a halo.
- **Nonlinear external fields**: higher-order components in field maps and the fringe fields of solenoids and cavities distort the phase ellipse.
- **Initial distribution**: Gaussian tails already extend several rms sizes out, a KV distribution has no tails; the choice of KV / GS / PB / WB in AVAS's `distribution` affects loss estimates.

In AVAS the direct halo indicators are the ratio of maximum to rms size (DataSet columns 22/24 against 16/18), the emittance growth along the line and the surviving macro-particles (column 28). A good rms match with a steadily growing max/rms ratio usually points to nonlinear or resonance problems.

## Matching in AVAS {#matching-avas}

### Setting the entrance beam

The Beam page sets `twissx`, `twissy`, `twissz` (α, β, ε); units:

```avas-params
beam:twissx beam:twissy beam:twissz beam:distribution
```

β is in mm/mrad (numerically m/rad), ε is the normalised rms emittance (π·mm·mrad), and twissz uses z (mm) and $z' = \Delta p/p$ (mrad). The example beam `twissx -0.65785 0.37743 0.13768` has an entrance $x_{\mathrm{rms}} = \sqrt{0.37743 \times 0.13768/0.05708} = 0.954\ \mathrm{mm}$.

### Interactively: the linear preview

When you drag quadrupole gradients, solenoid `Kb` and other parameters in the visual editor, the linear envelope preview updates live (tick "Space charge in the preview" to include linear space charge). The visual criterion for a match: in a periodic section the envelope repeats every period without a period-to-period beat. The preview is an approximate model (rms usually within 1–2 %, up to 13 % with strong multi-particle effects) without losses or nonlinearities; confirm with the kernel.

### Scans and optimisation

- **The Parameter scan page** (or the command line `avas scan --input DIR --target ELEMENT --param PARAM --values …`): runs the kernel for a list of values of one parameter in copies under `Scans/` and compares the results.
- **The AI assistant's `optimize` tool**: varies parameters of several elements within bounds to minimise or maximise an objective. Metrics available with the preview engine are `rms_x_max`, `rms_y_max`, `rms_x_out`, `rms_y_out`, `energy_out` and `rms_mismatch_out`; the objective may be an expression such as `abs(rms_x_out-1.5)+abs(rms_y_out-1.5)`. Note that `rms_mismatch_out` is the end value of $\lvert x_{\mathrm{rms}} - y_{\mathrm{rms}}\rvert$ (roundness), not the Twiss mismatch factor of this chapter. The simulation engine adds metrics such as transmission and emittance growth. The result is proposed as an edit and is written to the lattice only after the user approves it.

### Size targets with adjust and diag_size

The static-error correction can also be used to tune parameters for beam sizes (details in [errors and correction](#errors-correction)):

```avas-params
adjust diag_size
```

- `adjust 0 v n min max first_step` goes in front of the element to tune; `v` is the v-th parameter of that element (counted from 1 after the keyword: the gradient of `quad` is the 4th, `Kb` of `field` the 8th and `Ke` the 7th), knobs with the same `n` share one value, and `first_step = 1` starts the optimisation from the current value.
- `diag_size 0 sx sy 0` gives the target **rms** sizes (mm) at its position. The current code pairs `adjust` and `diag_*` by their **first parameter** (equal numbers form one correction, done in order of the number); the manual describes this parameter as "meaningless, write 0", and writing 0 everywhere makes one group.
- The optimiser is SLSQP (every function evaluation is one kernel run); the objective is the mean of $(x_{\mathrm{rms}} - s_x)^2 + (y_{\mathrm{rms}} - s_y)^2$ (mm²) over the group's diagnostics; it stops below 0.05, after at most 100 iterations, with a finite-difference step of 0.1 (in the parameter's own unit).
- The mechanism runs only in a **static error study** (`error_type` `stat` or `stat_dyn` in `ini.ini`) and needs `err_step` and at least one error switch turned on (a static one in `stat` mode). The results are written to `Adjust_Datas_<group>_<run>.txt` and are **not written back to the lattice**.

> Note: a single `diag_size` constrains only sizes, not α, and cannot fix a match; size targets at two places about $90^\circ$ of phase advance apart constrain $(\alpha, \beta)$ indirectly. Using zero-amplitude static errors to borrow this mechanism for an error-free match is inferred from the current code and has not been validated systematically with the kernel; check the result of every run.

### Checking

- Results page: rms envelope, Twiss α/β along the line, emittances; the "Phase advance" plot (needs `lattice … lattice_end` period markers) gives the phase advance of the actual beam as $\int\dd z/\beta$ per period.
- The Run page shows the rms envelope live; "compare with the previous run" overlays the previous run to compare before and after matching.

> AVAS convention: the old matching commands described in the manual (`MATCHING`, `SETTWISS`, `CIRCLE_MATCH`, …) belong to a legacy workflow that has not been verified in the current GUI; do not use them as a current feature.

## Hands-on {#matching-lab}

In the FODO lab set `mismatch` to 1 (matched), 1.2 and 1.44 and watch the envelope beat around the periodic envelope. `mismatch` is the initial size scale factor $s$: the lab multiplies β at the cell entrance by $s^2$ and keeps α. At a place with $\alpha = 0$ this would give $M = s - 1$; at the cell entrance $\alpha \approx -1.76$, so the same scaling is a larger mismatch: $M \approx 0.44$ for $s = 1.2$ and $M \approx 1.0$ for $s = 1.44$.

```avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1.2
```

Then raise the DC current `I` to 10–20 mA: the zero-current periodic Twiss parameters are no longer matched and the envelope begins to beat, with periods corresponding to the breathing and quadrupole modes of [envelope oscillation modes](#matching-envelope-modes).

```avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=12 W=1.527 mass=938.272 q=1 emit=0.1377 I=15 mismatch=1
```

In the phase-space lab send an ellipse through a thin lens and a drift to see how α and β together determine the downstream size:

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0.5 L=0.5
```

## Further reading {#matching-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: mismatch factor, envelope modes, the particle–core model of halo.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.: envelope equations, free energy of mismatch and emittance growth.
- F. J. Sacherer (1971): rms envelope equations, which make rms matching valid for any elliptically symmetric distribution.
- J. Struckmeier, M. Reiser (1984): envelope oscillations and instabilities of mismatched beams in periodic channels.
- The full bibliography is in [references](#references).
