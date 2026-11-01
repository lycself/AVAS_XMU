# Errors and correction

A FODO channel of ten periods has 20 quadrupoles. Suppose each of them was installed 0.1 mm off at random — about the thickness of a hair. How far off is the beam centre at the exit? About 1 mm, more than half the rms beam size there. Why are small errors amplified so much? How large an error can an accelerator tolerate? How many steerers and beam position monitors does it take to pull the beam back? This chapter first answers these questions with linear optics, then explains how errors are studied statistically, and finally describes, one by one, AVAS's error commands, sampling rules, static error correction (`adjust` and `diag_*`), runs and output.

The chapter uses the β function and phase of [linear optics](#linear-optics), the mismatch factor of [matching](#matching) and the synchronous phase of [longitudinal dynamics](#longitudinal); the examples use the FODO of [lattice design](#lattice-design-periods) ($G = 5\ \mathrm{T/m}$, $L_q = 0.1\ \mathrm{m}$, $L_d = 0.3\ \mathrm{m}$, the 1.527 MeV example proton beam). Steerers themselves are covered in [steerers](#steerer).

## Sources and kinds of errors {#errors-types}

The elements of a real accelerator are never exactly at their design positions, and field strengths, RF amplitudes and phases deviate too:

| Kind | Examples | Columns in AVAS |
|---|---|---|
| Element position | transverse offsets $dx$, $dy$, longitudinal offset $dz$ | `dx`, `dy` (mm), `dz` (mm) |
| Element orientation | rotations about the x, y, z axes | `dφx`, `dφy`, `dφz` (°) |
| Static field | calibration errors of quadrupole gradients, solenoid fields | `dG` (%) |
| RF amplitude and phase | setting errors and jitter of cavity voltage and phase | `ke/kb` (%), `φs` (°) |
| Input beam | offsets of entrance position, angle, energy, phase, emittance, current | `dx dy dφ dx' dy' dE dεx dεy dεz … dIb` |

By their behaviour in time there are two kinds, and this decides whether they can be compensated:

- **Static errors** do not change during a run (a period of operation), for example installation offsets and magnet calibration errors. They can be measured with beam diagnostics and compensated with steerers, cavity phase adjustment and the like.
- **Dynamic errors** change with time, for example RF amplitude and phase jitter, mechanical vibration, power-supply ripple. They change faster than a correction loop or cannot be measured, are usually not compensated and must be kept within tolerances.

An **error study** samples the statistical distribution of the errors many times (many "seeds"), tracks the beam each time, and collects the distributions of figures such as losses, emittance growth and centroid offsets.

## What a misaligned quadrupole does {#errors-effects}

The estimates below are based on linear optics without space charge; they give orders of magnitude and a check of simulation results.

### A misalignment is a kick

A quadrupole displaced transversely by $d$ makes particles see a field centred at $d$: $x'' = -k\,(x - d)$. Split it in two: $-kx$ is a perfect quadrupole, $+kd$ is a constant force independent of the particle's position. A misaligned quadrupole is therefore a perfect quadrupole plus a small kick. For a thin lens the kick is

$$
\Delta x' = k L_q\, d = \frac{d}{f}
$$

pulling the beam towards the quadrupole's actual centre. In the example $kL_q = 2.80\ \mathrm{m^{-1}}$, so a 0.1 mm offset gives a kick of $0.28\ \mathrm{mrad}$.

### How one kick travels downstream

The kick sets the beam centre into a betatron oscillation about the axis, like a pendulum that has been pushed. A kick $\theta_i$ at $s_i$ produces a centroid offset at a downstream position $s$ of

$$
x_c(s) = \sqrt{\beta(s)\,\beta_i}\,\sin\!\big(\psi(s) - \psi_i\big)\,\theta_i
$$

$\sqrt{\beta_i}$ says that a kick matters more where the beam is large, $\sqrt{\beta(s)}$ that an offset looks larger where the beam is large, and the sine is the phase of the oscillation.

### Many random kicks: a random walk

When the offsets of the quadrupoles are independent with zero mean, the kicks arrive downstream with random phases and add in quadrature rather than linearly. Averaging over the ensemble, $\langle\sin^2\rangle = 1/2$, gives

$$
\langle x_c^2(s)\rangle^{1/2} = \sqrt{\frac{\beta(s)}{2}\sum_i \beta_i\,\langle\theta_i^2\rangle}
$$

Example: 10 periods with 20 quadrupoles, each displaced in x with a Gaussian of rms $0.1\ \mathrm{mm}$, $\sigma_\theta = 0.28\ \mathrm{mrad}$. With $\beta = 1.35$ and $0.45\ \mathrm{m}$ at the centres of the F and D quadrupoles and $\beta = 1.26\ \mathrm{m}$ at the end (an F-quadrupole entrance), the formula gives an rms centroid offset of about $0.94\ \mathrm{mm}$ at the end; a Monte Carlo of 20000 samples with thick-lens transfer matrices gives $0.96\ \mathrm{mm}$. The rms beam size there is about $1.74\ \mathrm{mm}$ — an installation error of only 0.1 mm moves the centre by more than half the rms size, which is why long lattices need orbit correction. The centroid offset grows as $\sqrt{N}$ and is damped adiabatically as $1/\sqrt{\beta\gamma}$ by acceleration.

> **Think about it**: 20 quadrupoles each kicking by 0.28 mrad — why is the result not 20 times that of a single one? And what decides the ratio of the centroid offset to the beam size?
>
> Answer: kicks with random phases add in quadrature, so 20 of them count only $\sqrt{20} \approx 4.5$ times. The centroid offset and the rms size are both proportional to $\sqrt{\beta(s)}$, so their ratio is the same everywhere along the line, $\sqrt{\sum_i\beta_i\theta_i^2/(2\varepsilon)}$: each kick has to be compared with the local angular spread of the beam, $\sqrt{\varepsilon/\beta_i}$. In the example the ratio is 0.54, exactly $0.94/1.74$.

### Rotation errors: coupling

A quadrupole rotated by $\varphi_z$ about the z axis acquires a skew-quadrupole component (of strength about $2\varphi_z$ times the normal one) that couples the x and y planes and increases the projected emittances. Rotations about the x and y axes displace the ends of the element by about $\pm L\varphi/2$, an effect similar to, but smaller than, an offset. A solenoid couples the planes anyway, and its offsets and rotations also produce kicks.

### Field errors: mismatch and phase-advance change

A relative gradient error $\Delta G/G$ of a quadrupole produces no kick (the beam is on axis) but changes the focusing strength, equivalent to a thin-lens error $\Delta(1/f) = kL_q\,\Delta G/G$. By the section on the [mismatch factor](#matching-mismatch-factor), a matched beam passing it gets

$$
M \approx \frac{\beta\, kL_q}{2}\,\frac{\Delta G}{G}
$$

Example: a 1 % gradient error in one F quadrupole gives $M \approx 1.35 \times 2.80 \times 0.01/2 = 0.019$. The mismatch from many independent small errors adds roughly in quadrature. The phase advance changes too: the change per period is about $\Delta\sigma \approx \tfrac12\sum\beta_i\,\Delta(1/f)_i$ (radians, focusing positive). With all gradients 1 % too high, one period gets $+\tfrac12 \times 1.35 \times 0.028$ from the F quadrupole and $-\tfrac12 \times 0.45 \times 0.028$ from the D quadrupole, together $0.0125\ \mathrm{rad} = 0.71^\circ$.

In the FODO lab change `G` from 5 to 5.05 (a 1 % gradient error of all quadrupoles): the thick-matrix phase advance goes from $61.44^\circ$ to $62.12^\circ$, up by $0.68^\circ$ in agreement with the estimate; the periodic β at the cell entrance goes from $1.256$ to $1.251\ \mathrm{m}$.

```avas-lab
fodo Lq=0.1 G=5.05 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
```

### RF amplitude and phase errors

For one cavity in the thin-gap approximation

$$
\Delta W = qV_0T\cos\varphi_s\,\frac{\Delta V}{V},\qquad
\Delta W \approx -qV_0T\sin\varphi_s\,\Delta\varphi
$$

Example: the example HWR at $\varphi_s = -33^\circ$, with the effective $V_0T = \Delta W/\cos\varphi_s \approx 0.405\ \mathrm{MV}$ at that phase, gives $\Delta W \approx 3.4\ \mathrm{keV}$ for a 1 % amplitude error and $\Delta W \approx 3.8\ \mathrm{keV}$ for a $1^\circ$ phase error, small compared with the energy half-width of the longitudinal acceptance (about $\pm 0.13\ \mathrm{MeV}$, see [lattice design](#lattice-design-longitudinal)).

The linear preview does not quite give these numbers. Raising `Ke` by 1 % adds $3.5\ \mathrm{keV}$, as estimated. For the phase one must first say *which* phase is in error. A real RF phase error shifts the RF phase against the arriving beam, that is, the entrance phase $\varphi_0$: shifting $\varphi_0$ by $1^\circ$ (V3 = 1) changes the gain by $4.6\ \mathrm{keV}$, about 1.2 times the thin-gap estimate. Changing the synchronous-phase *setting* of a V3 = 0 cavity from $-33^\circ$ to $-32^\circ$ adds $6.0\ \mathrm{keV}$, about 1.6 times: the code then solves for a new entrance phase, and near $-33^\circ$ the $\varphi_s$ of eq. (3) in [longitudinal dynamics](#longitudinal-gap) moves by only about $0.76^\circ$ per degree of entrance phase, so one degree of $\varphi_s$ is about $1.3^\circ$ of RF phase. Both differ from the thin-gap value because in a low-β cavity the particle speeds up noticeably inside the cavity and the effective voltage itself depends on the phase: $\Delta W/\cos\varphi_s$ changes by about $2.6\ \mathrm{keV}$ per degree between $-30^\circ$ and $-40^\circ$, which times $\cos\varphi_s$ adds about $2.2\ \mathrm{keV/{}^\circ}$ to the $3.8\ \mathrm{keV/{}^\circ}$; the sum is just about $6.0\ \mathrm{keV}$. This is physics, not a numerical artefact: integrating the same map with the velocity frozen at its entrance value gives $\Delta W = qV_0T\cos\varphi_s$ with a phase-independent $V_0T = 0.389\ \mathrm{MV}$, and both sensitivities fall back to the thin-gap value of about $3.7\ \mathrm{keV/{}^\circ}$. How the engine applies the `φs` column of `err_cav_*` to a V3 = 0 cavity (as a change of the synchronous-phase setting or as a shift of the RF phase) is not described in the manual; the engine is the reference. Phase tolerances at low energy must therefore be estimated with the real field map, and the meaning of the phase error must be stated with them.

With longitudinal focusing, energy and phase errors show up as a longitudinal oscillation of the bunch centre about the synchronous particle; if they are too large, particles leave the separatrix, are lost longitudinally first and then, because of their energy offset, transversely. At low energy the velocity is sensitive to energy, so an energy error shifts the phase at which particles reach the downstream cavities; chains of independently phased cavities are particularly sensitive to this.

In the RF-cavity lab change the synchronous phase from $-33^\circ$ to $-32^\circ$, or `Ke` by 1 %, and compare the change of the exit energy; the lab also shows the entrance phase $\varphi_0$ it solved for, which moves by about $1.3^\circ$ when $\varphi_s$ moves by $1^\circ$:

```avas-lab
rf-cavity phase=-32 Ke=1.36 W=1.527
```

### Input-beam errors

Offsets of entrance position and angle cause transverse centroid oscillations (with the same propagation formula as element offsets), offsets of energy and phase cause longitudinal centroid oscillations, and emittance and current deviations mainly appear as mismatch.

## Static and dynamic errors {#errors-static-dynamic}

The two kinds of errors are treated differently because one can be measured and compensated and the other cannot. A static error does not change during a run: measure the beam centre with the diagnostics, adjust steerers and cavity phases to pull it back, and what remains is the residual after correction. A dynamic error changes too fast — by the time it is measured it has changed — and adds directly to the corrected beam. An error budget therefore usually has two steps: first apply only the static errors and correct them; then add the dynamic errors on top of the corrected state and look at the total effect.

AVAS's error studies are organized in exactly this way: static errors alone, dynamic errors alone, or both together, with the correction "seeing" only the static errors. How this is implemented at the engine level is explained in [In AVAS](#errors-in-avas).

## Statistical error studies {#errors-statistics}

### Why many seeds

Errors are random, and one run is only one sample. Figures such as losses and emittance growth often have skewed, long-tailed distributions, and the mean alone underestimates the risk. The usual procedure:

1. Study each kind of error alone: scan its amplitude from small to large and find the amplitude at which a figure (loss rate, emittance growth, centroid offset …) reaches its allowed value; that is the individual tolerance.
2. Combine all individual tolerances in a global error study, confirm that the total effect stays within budget, and tighten individual tolerances where necessary.
3. Repeat with correction included to obtain "tolerances after correction".

### Sample size

- The statistical error of a mean is about $\sigma/\sqrt{N}$; estimating the tails of a distribution (the 99 % quantile, say) needs far more samples than estimating the mean.
- Loss rate: if none of $N$ runs shows losses, the probability of a run with losses is below about $3/N$ at 95 % confidence (the "rule of three"). Where it comes from: if a single run has losses with probability $p$, all $N$ runs are loss-free with probability $(1-p)^N \approx e^{-pN}$; setting this to 0.05 gives $p = -\ln 0.05/N = 3.0/N$.
- A uniform distribution on $[-A, A]$ has the rms $A/\sqrt3$, a Gaussian the rms σ. Convert to rms when comparing results with different distributions.

### Scanning the amplitude in groups

Dividing the error amplitude into groups from small to large, with several samples per group, gives a "figure versus error amplitude" curve from a single study. AVAS's `err_step a b` does exactly that: a groups of b runs each, with the amplitude of group g being $g/a$ times the given amplitude (except for the fixed-value type, see [In AVAS](#errors-in-avas)); the last group has the full given amplitude.

## Correction: pulling the beam back to the centre {#errors-correction}

### One-to-one orbit correction

Since a misaligned quadrupole acts as a kick, another kick can cancel it. The classic "one-to-one" orbit correction puts a steerer and a beam position monitor (a diagnostic) near every quadrupole and uses the steerer to bring the centre back to zero at the downstream monitor. By the estimate above, installation errors of the order of 0.1 mm already need correction.

The displacement a steerer kick $\theta_c$ produces at the monitor follows from the same propagation formula: $\Delta x = \sqrt{\beta_c\beta_b}\,\sin\Delta\psi\,\theta_c$. Example: a steerer at an F quadrupole ($\beta_c = 1.35\ \mathrm{m}$) and a monitor at the next F quadrupole ($\beta_b = 1.35\ \mathrm{m}$, phase difference $61.4^\circ$) give a response of $1.18\ \mathrm{mm/mrad}$. Pulling back an offset of 1 mm takes a kick of $0.85\ \mathrm{mrad}$, a field integral $\int B\,\dd l = \theta\,B\rho = 1.5\times 10^{-4}\ \mathrm{T\,m}$, for example a 0.1 m steerer at 1.5 mT.

> **Think about it**: why should the monitor be about $90^\circ$ of phase away from the steerer rather than right next to it?
>
> Answer: the response is proportional to $\sin\Delta\psi$. Right next to it, $\Delta\psi \approx 0$ and the steerer can hardly move the beam at the monitor, only change its angle; the same happens near $180^\circ$. Around $90^\circ$ the response is largest and the kick needed smallest.

### What correction cannot fix

- **Errors of the monitors themselves**: a zero offset or reading error of a monitor becomes a residual offset after correction.
- **Oscillations between monitors**: correction only makes the centre zero at the monitors; between two monitors the beam can still be off.
- **Dynamic errors**: they change too fast to be measured or compensated.
- **Mismatch from field errors**: orbit correction acts on the centre, not on the envelope; mismatch is handled by matching sections or by retuning focusing elements.

The commands and algorithm for correction in AVAS are in [In AVAS](#errors-in-avas).

## Common misconceptions {#errors-misconceptions}

- **Amplitude versus rms**: a uniform distribution on $[-A, A]$ has the rms $A/\sqrt3$, not $A$. "A uniform 0.1 mm error" and "a Gaussian error of rms 0.1 mm" differ by $\sqrt3$.
- **Gaussians are not truncated**: AVAS samples Gaussian errors without truncation (see the sampling rules below); single seeds can have large errors, which is the tail of the distribution, not a bug.
- **Looking only at the mean**: losses and emittance growth have long-tailed distributions; set tolerances from unfavourable quantiles.
- **A finished run is not a successful correction**: the correction may not have converged; always check the diagnostics after correction.
- **Two seeds**: the seed of the error sampling and the seed of the initial particle distribution are two settings; fix both when comparing two studies.
- **The thin-gap formula underestimates the phase sensitivity of low-β cavities**: for the example HWR (linear preview) a shift of the RF phase gives about 1.2 times the estimate, a change of the synchronous-phase setting of a V3 = 0 cavity about 1.6 times. Say which phase a tolerance refers to.

## In AVAS {#errors-in-avas}

### How static and dynamic errors are implemented

Each error run of AVAS (one "run" of a "group") makes Python read the lattice again from the input snapshot, draw all errors for this run's group, write the drawn values as engine commands and run the engine.

> **AVAS convention**: the engine knows only the dynamic error keywords (`err_*_ncpl_dyn`, `err_beam_dyn` and the `*_dyn_on` switches). A static error study rewrites the drawn static errors as dynamic error commands with fixed values for the engine. So "static" and "dynamic" differ in AVAS not inside the engine but in the procedure:

- `error_type = stat`: only static errors (`*_stat` commands) are drawn, dynamic error commands are ignored; `adjust` correction can run.
- `error_type = dyn`: only dynamic errors are drawn, static error commands are ignored; no correction runs.
- `error_type = stat_dyn`: both are drawn. The correction (if any) "sees" only the static errors — the lattice written to the engine during optimization contains only static errors; once the correction is settled, the static and dynamic errors of each element are added column by column into one dynamic error command, the switches being the union of both, and the engine runs. This corresponds to the physical picture "first tune the correctors to the static errors, then add the jitter that cannot be compensated".

Whether the error values stay constant during the whole tracking of a run is up to the engine and not described in the manual; by the commands Python writes, each run has one set of fixed values.

### Error commands: groups and input-beam errors

Error commands are written in the lattice file (between `start` and `end`, usually at the beginning) and act on the elements **below** the command.

```avas-params
err_step err_beam_dyn err_beam_stat
```

- `err_step a b`: a groups of b runs each; an error study requires it.
- `err_beam_dyn r dx dy dφ dx' dy' dE dεx dεy dεz mx my mz dIb`: dynamic errors of the input beam, `r` being the distribution type; not all parameters need to be written, columns left out have no error.
- `err_beam_stat 0 …`: the static version; the manual requires 0 as the first parameter.

> **Note**: the manual does not explain `mx`, `my`, `mz`. The current code interprets the first parameter of `err_beam_stat` like the `r` of `err_beam_dyn`; 0 means a fixed-value error, consistent with the manual.

### Error commands: magnetostatic elements and RF cavities

```avas-params
err_quad_ncpl_stat err_quad_cpl_stat err_cav_ncpl_stat err_cav_cpl_stat
```

The dynamic versions `err_quad_ncpl_dyn`, `err_quad_cpl_dyn`, `err_cav_ncpl_dyn`, `err_cav_cpl_dyn` have the same columns:

```text
err_quad_ncpl_stat  N  r  dx  dy  dφx  dφy  dφz  dG  dz  Nb
err_cav_ncpl_stat   N  r  dx  dy  dφx  dφy  ke/kb  φs  dz  Nb
```

- `N`: acts on the N **elements** below the command. All elements count (drifts, field maps, matrix elements, diagnostics), but the errors are applied only to elements of a suitable type; to cover all elements write a large number.
- Magnetostatic errors (`err_quad_*`) apply to `quad` and to magnetostatic field maps (`field` type 3, such as solenoid maps); cavity errors (`err_cav_*`) apply to RF field maps (`field` type 1). The `solenoid` matrix element and electrostatic maps (type 2) take neither.
- `Nb` is not explained in the manual and is not sampled.

> **Note**: the current code does not sample the **last column** of an element error command and keeps it as written. When the command is written out to `Nb`, that column is `Nb`; if it stops at `dz` and leaves out `Nb`, `dz` becomes the last column and is neither sampled from the distribution nor scaled with the group number. When you need `dz` errors, write `Nb` as well (e.g. 0).

### Switches

```avas-params
err_quad_stat_on err_cav_stat_on err_beam_stat_on
```

`err_quad_dyn_on`, `err_cav_dyn_on`, `err_beam_dyn_on` likewise. Each column is 0 or 1, in the order of the error columns of the corresponding error command; columns switched to 0 get no error. An error study requires at least one switch column set to 1 for its mode, otherwise it reports an error (Missing … error on command).

### An example

```text
start
err_step 3 20
err_quad_stat_on 1 1 0 0 0 0 0
err_quad_ncpl_stat 1000 1 0.1 0.1
err_cav_dyn_on 0 0 0 0 1 1 0
err_cav_ncpl_dyn 1000 2 0 0 0 0 1.0 1.0
drift 0.05 0.02 0
...
end
```

Meaning: 3 groups of 20 runs. All quadrupoles / magnetostatic maps get uniformly distributed offsets in x and y, $[-0.1, 0.1]\ \mathrm{mm}$ in group 3 and $[-0.033, 0.033]\ \mathrm{mm}$ in group 1; all RF cavities get Gaussian dynamic amplitude and phase errors with standard deviations of 1 % and $1^\circ$ in group 3. In `stat_dyn` mode both act; `stat` has only the offsets, `dyn` only the RF errors.

### Sampling rules

Distribution type `r`:

| r | Meaning | Column with given value A in group g (of a) |
|---|---|---|
| 0 | fixed value | always A, not scaled with the group |
| 1 | uniform | uniform in $[-A g/a,\ A g/a]$ |
| 2 | Gaussian | mean 0, standard deviation $A g/a$ (not truncated) |
| -1 | equal steps | $A g/a$ (equivalent to TraceWin's type 0) |

```avas-params
err_quad_ncpl_dyn
```

> **Note**: the equal-step type (r = −1) of input-beam errors is computed as $A(g+1)/a$ in the current code, unlike element errors; check the generated error records before using it. Drawn values are kept to 5 decimal places.

Coupled and uncoupled:

- `ncpl` (uncoupled): every affected element is sampled **independently**.
- `cpl` (coupled): each command draws once and applies the same values to all elements it covers, to model common errors such as a shared support or a shared power supply.
- When one element is covered by several quadrupole / cavity error commands of the same kind (static or dynamic), the current code keeps only the one closest to the element (the lowest) and does not add them.

Random seeds:

- `[error] seed` in `ini.ini` ("Seed" under "Error study" on the Settings page, `--seed` on the command line) decides the error sampling (and the random starting values of the correction). The same seed and lattice give the same error sequence.
- `randomseed` (listed under `beam.txt` in the manual, written in `input.txt` in the example project) seeds the generation of the initial particle distribution, a setting independent of the error seed. Fix both when comparing two error studies.

### Static error correction: commands

```avas-params
adjust diag_energy diag_size diag_position
```

- `adjust 0 v n min max first_step`: placed directly in front of the element to tune, it makes the element's v-th parameter a knob (counted from the 1st parameter after the keyword: the phase of `field` is the 6th, `Ke` the 7th, `Kb` the 8th; the gradient of `quad` the 4th; `Bx`/`By` of `steerer` the 4th/5th), limited to `[min, max]`; knobs with the same (non-zero) `n` are constrained to one value; `first_step = 1` starts from the element's current value, 0 from a random value within the range.
- `diag_position 0 x y 0`: target beam centre at this position (mm).
- `diag_size 0 sx sy 0`: target rms size at this position (mm).
- `diag_energy 0 W 0`: target energy at this position (MeV).
- `diag_*` are zero-length elements marking where a target applies; they are not passed to the engine, Python reads the beam parameters at their position from DataSet.

### Static error correction: algorithm

This is how AVAS's static error correction currently works; the manual does not document these details and they may change:

1. A correction runs only when an `adjust` and a `diag_*` have the same first parameter (the manual calls it "meaningless, write 0"; writing 0 everywhere makes one group). Different numbers are corrected one after another in increasing order, each starting from the lattice corrected by the previous one.
2. Each group is optimized with SLSQP: every function evaluation writes the knobs into the lattice, runs the engine to just past the last diagnostic and computes the objective — the mean squared deviation over the group's diagnostics: position $(x_c - x)^2 + (y_c - y)^2$ (mm²), size $(x_{\mathrm{rms}} - s_x)^2 + (y_{\mathrm{rms}} - s_y)^2$ (mm²), energy $(W - W_{\mathrm{target}})^2$ (MeV²).
3. It stops when the objective falls below 0.05; at most 100 iterations, a finite-difference step of 0.1 (in the knob's own unit), convergence tolerance $10^{-4}$.
4. Every error run is corrected anew; the corrected values go to `Adjust_Datas_<group>_<run>.txt` and the beam parameters at the diagnostics to `par_diag_datas_<group>_<run>.txt`. The engine runs of the correction go under `error_adjust/`. The corrected values are not written back to the lattice file.

> **Note**: the finite-difference step is fixed at 0.1, so for knobs whose useful range is much smaller than 0.1 (for example a steerer field of ±0.01 T in T) the gradient estimate is unreliable; the threshold 0.05 of the objective has the dimension of the target (mm² or MeV²) and may be too loose for small beams: 0.05 mm² corresponds to a centroid deviation of about 0.22 mm. The energy-correction example of the case tutorial written as in the manual (target 5 MeV) does not reach its target (the recorded final energy is still about $2.3\ \mathrm{MeV}$) — a finished run is not a successful correction; always check the diagnostic data after correction.

### Writing an orbit correction (sketch)

The following only shows how the commands go together; it has not been verified with the engine:

```text
adjust 0 4 0 -0.005 0.005 1
adjust 0 5 0 -0.005 0.005 1
steerer 0 0.02 0 0 0 0 0.005
field 0.35 0.02 0 3 0 0 1 0.38 sol
drift 0.05 0.02 0
diag_position 0 0 0 0
```

The two `adjust` lines make `Bx` and `By` of the steerer knobs; the steerer acts on the next element (the solenoid map); `diag_position` asks for the downstream centre to return to (0, 0). Mind the field unit T and the finite-difference step problem above; check the convergence with a small study before relying on it.

### Running an error study

The `[error]` section of `ini.ini`:

| Key | Meaning |
|---|---|
| `error_type` | empty = normal run; `stat`, `dyn`, `stat_dyn` = the corresponding error study |
| `seed` | random seed of the error sampling |
| `if_normal` | 1 = run an error-free reference first |

In the interface: choose None / Static / Dynamic / Static + dynamic under "Error study" on the Settings page and fill in "Seed" (runs from the interface always do the error-free reference first); then write `err_step`, error commands and switches in the lattice, save and run. On the command line: `avas run --input … --output … --mode stat|dyn|stat_dyn --seed 7`, with `--no-normal` to skip the reference run; `--mode auto` (the default) reads `ini.ini`. The AI assistant's `set_error_study` tool changes the same two settings (with the user's approval).

Like a full run, an error study runs on the `inputs/` snapshot of the output folder and does not rewrite the project's InputFile; during the run (including pauses) the input files are read-only.

### Output files

| Path (relative to the output folder) | Content |
|---|---|
| `error_output/output_0_0/` | the error-free reference run |
| `error_output/output_<group>_<run>/` | complete output of each error run (DataSet.txt etc., and the lattice.txt used) |
| `error_middle/output_0/` | the run in progress, moved to `error_output` when finished |
| `errors_par_tot.txt` | one line per run: loss rate, emittance growth $\varepsilon_{\mathrm{out}}/\varepsilon_{\mathrm{in}} - 1$ of the three planes, final centre and angle, rms size and divergence, energy difference to the reference run, final Twiss |
| `errors_par.txt` | one line per group: the means over the group's runs of the quantities from loss rate to energy difference (without Twiss), plus the rms of centre, angle, size, divergence and energy difference |
| `Error_Datas_<group>_<run>.txt` | the error values applied to each cavity / quadrupole in this run |
| `Adjust_Datas_<group>_<run>.txt`, `par_diag_datas_<group>_<run>.txt` | corrected values, beam parameters at the diagnostics |
| `density_par_*.dat`, `density_tot_par*.dat` | particle density statistics (need `pchistogram 1 …` in `input.txt`) |

A run whose beam does not reach the end of the lattice (the last DataSet position more than 5 cm short of the total length) gets a loss rate of 1 and zeros in the other columns.

### Viewing the results

- The Run page shows the envelope of the current seed live and overlays the finished seeds as an envelope band ("Finished error seeds"), a direct picture of the spread caused by the errors.
- The "Error study" group of the Results page has two kinds of plots: "Emittance growth and loss" (reading `errors_par*.txt`) and "Output parameters" (with the mean or root-mean-square statistic).
- The detailed result of a single run can be viewed on the Results page by choosing its `error_output/output_<group>_<run>` folder.

A real statistical study needs the engine: copy the example project to a practice folder, add `err_step` and error commands as in the example above, choose the error mode on the Settings page and run.

## Further reading {#errors-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: errors and tolerances, estimates of centroid offsets.
- S. Y. Lee, *Accelerator Physics*; H. Wiedemann, *Particle Accelerator Physics*: the linear theory of orbit distortion and correction.
- J. A. Hanley, A. Lippman-Hand (1983): the "rule of three" for zero events.
- The chapters on error analysis and static error correction of the AVAS manual (`docs/使用说明20260427.docx`) and the error commands in the in-app "Parameters and files" reference.
- The complete list is in [references](#references).
