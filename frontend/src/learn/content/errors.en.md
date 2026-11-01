# Errors and correction

The elements of a real accelerator are never exactly at their design positions, and field strengths and RF amplitudes and phases are off as well. A lattice design must answer two questions: how far do these errors push the beam off, and how much beam do they lose? How many correctors and diagnostics are needed, and how far can they reduce the effect? This chapter first gives linear estimates of the effects of each kind of error, then the method of statistical error studies, and finally goes through AVAS's error commands, the sampling rules, static error correction (`adjust` and `diag_*`), and running and output.

## Sources and kinds of errors {#errors-types}

| Kind | Examples | AVAS columns |
|---|---|---|
| Element position | transverse offsets $dx$, $dy$, longitudinal offset $dz$ | `dx`, `dy` (mm), `dz` (mm) |
| Element attitude | rotations about the x, y, z axes | `dφx`, `dφy`, `dφz` (°) |
| Static field strength | calibration errors of quadrupole gradients and solenoid fields | `dG` (%) |
| RF amplitude and phase | set-point errors and jitter of cavity voltage and phase | `ke/kb` (%), `φs` (°) |
| Initial beam | entrance position, angle, energy, phase, emittance, current offsets | `dx dy dφ dx' dy' dE dεx dεy dεz … dIb` |

By their time behaviour there are two classes:

- **Static errors** stay constant during a run (over a period of time), for example installation offsets and magnet calibration errors. They can be measured with diagnostics and compensated with steerers, cavity re-phasing and similar means.
- **Dynamic errors** vary in time, for example RF amplitude and phase jitter, mechanical vibration and power-supply ripple. They change faster than the correction loops or cannot be measured, are usually not correctable, and can only be controlled by tolerances.

An error study samples the statistical distribution of the errors many times (many "seeds"), tracks the beam each time, and collects the distribution of figures of merit such as losses, emittance growth and centroid offsets.

## Linear effects of errors {#errors-effects}

The estimates below use linear optics and neglect space charge; they give orders of magnitude and a check of simulation results.

### Transverse quadrupole offsets: centroid offsets

With a quadrupole offset by $d$ the particles feel the field centred on $d$: $x'' = -k\,(x - d)$. For a thin lens this is an ideal quadrupole plus a deflection

$$
\Delta x' = k L_q\, d = \frac{d}{f}
$$

i.e. in its focusing plane the beam is pulled towards the actual quadrupole centre. Deflections $\theta_i$ produce at a downstream position $s$ the centroid offset

$$
x_c(s) = \sum_i \sqrt{\beta(s)\,\beta_i}\,\sin\!\big(\psi(s) - \psi_i\big)\,\theta_i
$$

For independent zero-mean offsets the ensemble average gives $\langle\sin^2\rangle = 1/2$ and

$$
\langle x_c^2(s)\rangle^{1/2} = \sqrt{\frac{\beta(s)}{2}\sum_i \beta_i\,\langle\theta_i^2\rangle}
$$

Example: the FODO of [focusing periods](#lattice-design-periods) ($G = 5\ \mathrm{T/m}$, $kL_q = 2.80\ \mathrm{m^{-1}}$), 10 periods with 20 quadrupoles, each with a Gaussian x offset of rms $0.1\ \mathrm{mm}$. With $\sigma_\theta = 0.28\ \mathrm{mrad}$, $\beta = 1.35$ and $0.45\ \mathrm{m}$ at the F and D centres and $\beta = 1.26\ \mathrm{m}$ at the end (F entrance), the formula gives an rms centroid offset of about $0.94\ \mathrm{mm}$ at the end; a Monte Carlo of 4000 samples with transfer matrices gives $0.97\ \mathrm{mm}$. The rms beam size there is about $1.74\ \mathrm{mm}$ — an installation error of only 0.1 mm moves the centroid by more than half the rms size, which is why long lattices need orbit correction. The centroid offset grows as $\sqrt{N}$ and is damped adiabatically as $1/\sqrt{\beta\gamma}$ by acceleration.

### Rotation errors: coupling

A quadrupole rotated by $\varphi_z$ about z acquires a skew-quadrupole component (about $2\varphi_z$ times the normal component) that couples x and y and increases the projected emittances. Rotations about x or y displace the element ends transversely by about $\pm L\varphi/2$, an effect similar to but smaller than offsets. A solenoid couples the planes anyway; its offsets and rotations also deflect the beam.

### Field errors: mismatch

A relative gradient error $\Delta G/G$ is a thin-lens error $\Delta(1/f) = kL_q\,\Delta G/G$. From the section on the [mismatch factor](#matching-mismatch-factor), a matched beam passing it gets

$$
M \approx \frac{\beta\, kL_q}{2}\,\frac{\Delta G}{G}
$$

Example: a 1 % gradient error in one F quadrupole of the FODO above gives $M \approx 1.35 \times 2.80 \times 0.01/2 = 0.019$. The mismatch from many independent small errors adds roughly in quadrature. The phase advance changes as well, by about $\Delta\sigma \approx \tfrac12\sum\beta_i\,\Delta(1/f)_i$ (radians) per period.

### RF amplitude and phase errors

For one cavity, in the thin-gap approximation,

$$
\Delta W = qV_0T\cos\varphi_s\,\frac{\Delta V}{V},\qquad
\Delta W \approx -qV_0T\sin\varphi_s\,\Delta\varphi
$$

Example: for the example HWR at $\varphi_s = -33^\circ$ with $V_0T \approx 0.405\ \mathrm{MV}$, a 1 % amplitude error gives $\Delta W \approx 3.4\ \mathrm{keV}$ and a $1^\circ$ phase error $\Delta W \approx 3.8\ \mathrm{keV}$, small compared with the energy half-width of the longitudinal acceptance (about $\pm 0.13\ \mathrm{MeV}$, see [lattice design](#lattice-design-longitudinal)). With longitudinal focusing, energy and phase errors show up as an oscillation of the bunch centre around the synchronous particle; if they are too large, particles leave the separatrix, are lost longitudinally first and then transversely because of their energy offset. At low energy the velocity is sensitive to the energy, so an energy error shifts the arrival phase at the downstream cavities; chains of independently phased cavities are particularly sensitive to this.

### Initial beam errors

Entrance position and angle offsets cause transverse centroid oscillations (with the same propagation formula as element offsets), energy and phase offsets cause longitudinal centroid oscillations, and emittance and current errors show up mainly as mismatch.

## Static and dynamic errors in AVAS {#errors-static-dynamic}

For every error run (one "run" of one "group") AVAS's Python code re-reads the lattice from the input snapshot, draws all errors for the group, writes the drawn values as kernel commands and runs the kernel.

> AVAS convention: the kernel only knows the dynamic error keywords (`err_*_ncpl_dyn`, `err_beam_dyn` and the `*_dyn_on` switches). A static error study rewrites the drawn static errors as dynamic error commands with fixed values. The difference between "static" and "dynamic" in AVAS is therefore not in the kernel but in the workflow:

- `error_type = stat`: only static errors (`*_stat` commands) are drawn, dynamic error commands are ignored; `adjust` correction can run.
- `error_type = dyn`: only dynamic errors are drawn, static error commands are ignored; no correction.
- `error_type = stat_dyn`: both are drawn. A correction (if any) "sees" only the static errors — the lattice written for the kernel during the optimisation contains only static errors; once the correction is fixed, the static and dynamic errors of each element are added column by column into one dynamic error command, the switches are the union of both, and the kernel runs. This corresponds to the physical picture "tune the correctors on the static errors, then add the uncorrectable jitter".

Whether the error values stay constant throughout the tracking of one run is up to the kernel and not described in the manual; from the commands Python writes, each run has one fixed set of values.

## Statistical error studies {#errors-statistics}

### Why many seeds

Errors are random and a single run is one sample. Figures of merit such as losses and emittance growth often have skewed, long-tailed distributions, and looking only at the mean underestimates the risk. The usual procedure:

1. Study each kind of error alone: scan its amplitude from small to large and find the amplitude at which the figure of merit (loss rate, emittance growth, centroid offset) reaches the acceptable value — the individual tolerance.
2. Combine all individual tolerances in a global error study, confirm that the total effect stays within budget, and tighten individual tolerances where needed.
3. Repeat with correction to obtain "corrected tolerances".

### Sample size

- The statistical error of a mean is about $\sigma/\sqrt{N}$; estimating the tail of a distribution (for example the 99 % quantile) needs far more samples than estimating the mean.
- Loss rate: if none of $N$ runs loses particles, the probability that a run has losses is below about $3/N$ at 95 % confidence (the "rule of three").
- A uniform distribution on $[-A, A]$ has rms $A/\sqrt3$, a Gaussian has rms σ. Compare results of different distributions at equal rms.

### Scanning the amplitude with groups

AVAS's `err_step a b` divides the study into a groups of b runs each. Group g uses $g/a$ times the given amplitudes (except the fixed-value type, see the next section), so one study yields a "figure of merit versus error amplitude" curve: the last group has the full given amplitude, and the b runs of each group give the statistics at that amplitude.

## AVAS error commands {#errors-commands}

Error commands are written in the lattice file (between `start` and `end`, usually at the beginning) and act on the elements **below** the command.

### Groups and initial beam errors

```avas-params
err_step err_beam_dyn err_beam_stat
```

- `err_step a b`: a groups of b runs; required for an error study.
- `err_beam_dyn r dx dy dφ dx' dy' dE dεx dεy dεz mx my mz dIb`: dynamic errors of the initial beam, `r` is the distribution type; the parameters need not all be written, trailing empty columns carry no error.
- `err_beam_stat 0 …`: the static version; the manual says to write 0 as the first parameter.

> Note: the manual does not describe `mx`, `my`, `mz`. The current code interprets the first parameter of `err_beam_stat` like `r` of `err_beam_dyn`; writing 0 gives fixed-value errors, consistent with the manual.

### Static-field element and RF cavity errors

```avas-params
err_quad_ncpl_stat err_quad_cpl_stat err_cav_ncpl_stat err_cav_cpl_stat
```

The dynamic versions `err_quad_ncpl_dyn`, `err_quad_cpl_dyn`, `err_cav_ncpl_dyn`, `err_cav_cpl_dyn` have the same columns:

```text
err_quad_ncpl_stat  N  r  dx  dy  dφx  dφy  dφz  dG  dz  Nb
err_cav_ncpl_stat   N  r  dx  dy  dφx  dφy  ke/kb  φs  dz  Nb
```

- `N`: acts on the N **elements** below the command. All elements are counted (drifts, field maps, matrix elements, diagnostics), but the errors go only to elements of the right type; write a large value to cover all elements.
- Static-element errors (`err_quad_*`) go to `quad` elements and static magnetic field maps (`field` type 3, e.g. solenoid maps); RF cavity errors (`err_cav_*`) go to RF field maps (`field` type 1). `solenoid` matrix elements and static electric maps (type 2) take neither.
- `Nb` is not described in the manual and is not sampled.

> Note: the current code leaves the **last column** of an element error command unsampled, as written. With the command written up to `Nb` that column is `Nb`; if you stop at `dz` and omit `Nb`, `dz` becomes the last column and is neither sampled nor scaled with the group. When you need `dz` errors, write `Nb` too (for example 0).

### Switches

```avas-params
err_quad_stat_on err_cav_stat_on err_beam_stat_on
```

`err_quad_dyn_on`, `err_cav_dyn_on` and `err_beam_dyn_on` work the same way. Each column is 0 or 1, in the order of the error columns of the corresponding command; columns switched off carry no error. An error study needs at least one switch column set to 1 for its mode, otherwise it fails (Missing … error on command).

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

Meaning: 3 groups of 20 runs. All quadrupoles / static magnetic maps get uniformly distributed x and y offsets, $[-0.1, 0.1]\ \mathrm{mm}$ in group 3 and $[-0.033, 0.033]\ \mathrm{mm}$ in group 1; all RF cavities get Gaussian dynamic amplitude and phase errors with standard deviations 1 % and $1^\circ$ in group 3. Run in `stat_dyn` mode both act; `stat` gives only the offsets, `dyn` only the RF errors.

## Sampling rules {#errors-sampling}

### Distribution type r

| r | Meaning | Column with given value A in group g (of a) |
|---|---|---|
| 0 | fixed value | always A, not scaled with the group |
| 1 | uniform | uniform in $[-A g/a,\ A g/a]$ |
| 2 | Gaussian | mean 0, standard deviation $A g/a$ (not truncated) |
| -1 | equal steps | $A g/a$ (TraceWin's type 0) |

```avas-params
err_quad_ncpl_dyn
```

> Note: for initial beam errors the current code computes equal steps (r = −1) as $A(g+1)/a$, unlike element errors; check the generated error records before relying on it. Drawn values are rounded to 5 decimals.

### Coupled and uncoupled

- `ncpl` (uncoupled): every affected element is sampled **independently**.
- `cpl` (coupled): each command is sampled once and the same values go to all elements it covers, modelling common supports, common power supplies and other collective errors.
- When an element is covered by several commands of the same family (static or dynamic, quadrupole or cavity), the current code keeps only the one closest to the element (the lowest); they do not add.

### Random seeds

- `[error] seed` in `ini.ini` ("Seed" under "Error study" on the Settings page, `--seed` on the command line) drives the error sampling (and the random start of the correction). The same seed and lattice give the same error sequence.
- `randomseed` (listed under `beam.txt` in the manual, written in `input.txt` in the example project) is the random seed used to generate the initial particle distribution, a setting independent of the error seed. Fix both when comparing two error studies.

## Static error correction {#errors-correction}

### Commands

```avas-params
adjust diag_energy diag_size diag_position
```

- `adjust 0 v n min max first_step`: placed directly in front of the element to tune; makes its v-th parameter a knob (counted from the first parameter after the keyword: the phase of `field` is the 6th, `Ke` the 7th, `Kb` the 8th; the gradient of `quad` the 4th; `Bx`/`By` of `steerer` the 4th/5th), limited to `[min, max]`; knobs with the same non-zero `n` are constrained to one value; `first_step = 1` starts from the element's current value, 0 from a random value inside the range.
- `diag_position 0 x y 0`: target beam centre at its position (mm).
- `diag_size 0 sx sy 0`: target rms sizes at its position (mm).
- `diag_energy 0 W 0`: target energy at its position (MeV).
- `diag_*` are zero-length elements marking where the targets are; they are not passed to the kernel, Python reads the beam parameters at their positions from DataSet.

### Algorithm

The following describes the current Python code (`avas/sim/err_adjust.py`); the manual does not document these details and they may change:

1. A correction runs only when an `adjust` and a `diag_*` have the same first parameter (described in the manual as "meaningless, write 0"; writing 0 everywhere makes one group). Different numbers are corrected one after another in increasing order, each starting from the lattice corrected by the previous one.
2. Each group is optimised with SLSQP: every evaluation writes the knobs into the lattice, runs the kernel up to just after the last diagnostic and computes the objective — the mean squared deviation over the group's diagnostics: position $(x_c - x)^2 + (y_c - y)^2$ (mm²), size $(x_{\mathrm{rms}} - s_x)^2 + (y_{\mathrm{rms}} - s_y)^2$ (mm²), energy $(W - W_{\mathrm{target}})^2$ (MeV²).
3. It stops as soon as the objective is below 0.05; at most 100 iterations, finite-difference step 0.1 (in the knob's own unit), tolerance $10^{-4}$.
4. The correction is repeated for every error run; the knob values go to `Adjust_Datas_<group>_<run>.txt` and the beam parameters at the diagnostics to `par_diag_datas_<group>_<run>.txt`. The kernel runs of the optimisation are kept under `error_adjust/`. The values are not written back to the lattice file.

> Note: the finite-difference step is fixed at 0.1, so for knobs whose useful range is much smaller than 0.1 (for example steerer fields in T, ±0.01 T) the gradient estimate is unreliable; the threshold 0.05 has the units of the target type (mm² or MeV²) and may be loose for small beams. The energy-correction example of the manual reproduced in the case tutorials (target 5 MeV) does not reach its target (the recorded final energy stays at about $2.3\ \mathrm{MeV}$) — a finished run is not a successful correction; always check the diagnostic data after correction.

### Orbit correction

The classic "one-to-one" orbit correction puts a steerer and a position diagnostic near each quadrupole and uses the steerer to bring the centroid at the downstream diagnostic back to zero. By the estimate above, offsets of order 0.1 mm already call for correction. In AVAS it looks like this (a sketch not validated with the kernel, showing only how the commands go together):

```text
adjust 0 4 0 -0.005 0.005 1
adjust 0 5 0 -0.005 0.005 1
steerer 0 0.02 0 0 0 0 0.005
field 0.35 0.02 0 3 0 0 1 0.38 sol
drift 0.05 0.02 0
diag_position 0 0 0 0
```

The two `adjust` lines make the steerer's `Bx` and `By` knobs; the steerer acts over the next element (the solenoid map), and `diag_position` asks for the downstream centre at (0, 0). Mind the field unit T and the finite-difference step above; check the convergence on a small study first.

## Running error studies and their output {#errors-running}

### Choosing the mode

The `[error]` section of `ini.ini`:

| Key | Meaning |
|---|---|
| `error_type` | empty = normal run; `stat`, `dyn`, `stat_dyn` = the corresponding error study |
| `seed` | random seed of the error sampling |
| `if_normal` | 1 = run an error-free reference first |

In the GUI: under "Error study" on the Settings page choose None / Static / Dynamic / Static + dynamic and fill in "Seed" (GUI runs always do the error-free reference first); then write `err_step`, the error commands and the switches in the lattice, save and run. On the command line use `avas run --input … --output … --mode stat|dyn|stat_dyn --seed 7`, with `--no-normal` to skip the reference run; `--mode auto` (the default) reads `ini.ini`. The AI assistant's `set_error_study` tool changes the same two settings (with the user's approval).

Like a full run, an error study runs on the `inputs/` snapshot in the output folder and does not rewrite the project's InputFile; the input files are read-only while it runs (including pauses).

### Output files

| Path (relative to the output folder) | Contents |
|---|---|
| `error_output/output_0_0/` | the error-free reference run |
| `error_output/output_<group>_<run>/` | complete output of each error run (DataSet.txt etc., and the lattice.txt used) |
| `error_middle/output_0/` | the run in progress, moved to `error_output` when finished |
| `errors_par_tot.txt` | one row per run: loss rate, emittance growth $\varepsilon_{\mathrm{out}}/\varepsilon_{\mathrm{in}} - 1$ in three planes, final centroid and angle, rms sizes and divergences, energy difference to the reference run, final Twiss parameters |
| `errors_par.txt` | one row per group: means over the group's runs of the quantities from loss ratio to energy difference (not the Twiss parameters), and root-mean-square spreads of centre, angle, size, divergence and energy difference |
| `Error_Datas_<group>_<run>.txt` | the actual error values applied to each cavity / quadrupole in that run |
| `Adjust_Datas_<group>_<run>.txt`, `par_diag_datas_<group>_<run>.txt` | correction values, beam parameters at the diagnostics |
| `density_par_*.dat`, `density_tot_par*.dat` | particle density statistics (need `pchistogram 1 …` in `input.txt`) |

A run whose beam does not reach the end of the lattice (last DataSet position more than 5 cm short of the total length) is recorded with loss rate 1 and zeros in the other columns.

### Viewing in the GUI

- The Run page shows the envelope of the current seed live and overlays the finished seeds as an envelope band ("Finished error seeds"), which shows the spread caused by the errors at a glance.
- The "Error study" group of the Results page has "Emittance growth and loss" (from `errors_par*.txt`) and "Output parameters" (with the mean or root-mean-square statistic) plots.
- Detailed results of single runs can be viewed by choosing the corresponding `error_output/output_<group>_<run>` folder on the Results page.

## Hands-on {#errors-lab}

The linear effects of errors can be estimated with the earlier labs. In the FODO lab change `G` from 5 to 5.05 (a 1 % gradient error on all quadrupoles) and compare the phase advance per period and the periodic β:

```avas-lab
fodo Lq=0.1 G=5.05 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
```

In the RF cavity lab change the synchronous phase from $-33^\circ$ to $-32^\circ$, or `Ke` by 1 %, compare the exit energies and check them against the $\Delta W$ estimates of this chapter:

```avas-lab
rf-cavity phase=-32 Ke=1.36 W=1.527
```

Real statistical studies need the kernel: copy the example project to a practice folder, add `err_step` and error commands as in the example above, choose the error mode on the Settings page and run.

## Further reading {#errors-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: errors and tolerances, estimates of beam centroid offsets.
- S. Y. Lee, *Accelerator Physics*; H. Wiedemann, *Particle Accelerator Physics*: the linear theory of orbit distortion and correction.
- The chapters on error analysis and static error correction of the AVAS user manual (`docs/使用说明20260427.docx`), and the error commands in the in-app "Parameter and file reference".
- The full bibliography is in [references](#references).
