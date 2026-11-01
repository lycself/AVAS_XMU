# Hands-on: build a small superconducting linac

With nothing but the field maps of one solenoid and one accelerating cavity, can you build a stretch of linac that really accelerates the beam and keeps it confined? This chapter tries it out: with two field maps of the example project `examples/hwr010` — the 162.5 MHz half-wave resonator `hwr010` (0.21 m) and the solenoid `sol` (0.35 m) — you build a proton superconducting accelerating section in five steps. Each step adds one thing and teaches one idea: why a beam grows, how focusing brings it back, why acceleration defocuses at the same time, why a periodic structure keeps the beam stable over a long distance, and how beam current changes all of this.

At every step you can drag the parameters and see the linear envelope preview at once, or press "Run with the engine" to let the AVAS multi-particle engine simulate it and overlay both results. Before you drag a parameter, try to guess how the curves will change, then look — where your guess was wrong is exactly where it pays to go back to an earlier chapter. Engine runs happen in a sandbox, need no open project and change no project file (details in [In AVAS](#tutorial-in-avas)).

This chapter ties together [drift](#drift), [solenoid](#solenoid), [RF cavity](#rf-cavity), [longitudinal dynamics](#longitudinal), [lattice design](#lattice-design), [matching](#matching) and [space charge](#space-charge); where this section sits in a whole accelerator is told in [the superconducting section](#journey-sc).

## The tutorial workbench {#tutorial-workbench}

The tool below contains all five steps. When you move on to the next step, the parameters you chose (solenoid factor, phase …) are carried over.

```avas-tutorial
build-linac
```

Every step starts from the beam of the example project's `beam.txt`: protons of kinetic energy $W_0 = 1.527\ \mathrm{MeV}$ ($\beta \approx 0.057$), frequency 162.5 MHz, Twiss parameters $\alpha_x = -0.658$, $\beta_x = 0.377\ \mathrm{mm/mrad}$, normalized rms emittance $\varepsilon_{n,x} = 0.138\ \pi\,\mathrm{mm\,mrad}$ (similar in $y$), entrance rms $x = 0.95\ \mathrm{mm}$. Steps 1–4 use the 0.184 mA of `beam.txt`; in step 5 the current can be changed (default 5 mA). $\alpha_x < 0$ means that the beam is diverging at the entrance.

> **AVAS convention**: the dashed curves (or the only curve, before any engine run) are the linear envelope preview, a quick approximation (rms usually within 1–2 %, more than ten per cent where multi-particle effects are strong); the solid curves that appear after an engine run are the multi-particle engine's result and are the reference. See [linear preview and engine](#simulation-preview).

## Step 1: a beam in a drift {#tutorial-step1}

**What this step teaches**: why a beam grows even when no force acts on it.

There is a single drift `drift L 0.02 0`. Every particle moves on a straight line, but their angles differ: a non-zero emittance means there is always an angular spread in the beam. Particles with different angles move apart along straight lines and the beam grows by itself, like the light from a torch. $\alpha < 0$ at the entrance means that angle and position are positively correlated (particles farther from the axis head farther out), so the beam diverges from the start. Following the [drift](#drift) chapter:

$$
\sigma_x^2(s) = \sigma_{x,0}^2 - 2\sigma_{xx',0}\, s + \sigma_{x'x',0}\, s^2 ,
$$

i.e. $\sigma_x(s) = \sqrt{\varepsilon\,(\beta_0 - 2\alpha_0 s + \gamma_0 s^2)}$. With $L = 0.8\ \mathrm{m}$ the preview shows rms $x$ growing from 0.95 mm to about 3.1 mm. At large $s$ the $\gamma_0 s^2$ term dominates and the growth is nearly linear.

Longitudinally it is the same: a drift has no focusing, and the bunch lengthens because of its momentum spread. A particle with momentum offset $\delta$ has a different velocity and moves $\delta/\gamma^2$ away from the synchronous particle per metre: $R_{56} = L/\gamma^2$. The example beam has an rms momentum spread of about 0.35 %, and rms $z$ grows from about 1.3 mm to about 3.1 mm.

> **Think about it**: does the emittance change in the drift?
>
> Answer: no. The ellipse in phase space is "sheared" — positions shift in proportion to angles — and its area stays the same, only its shape changes. The beam grows not because the emittance grows but because the same phase-space area projects more widely onto $x$.

**Try it**: raise $L$ to 2 m and see how fast the envelope approaches the aperture (be careful once it reaches about 1/3 of the 20 mm): the preview shows rms $x$ of about 6.8 mm at 2 m and warns near 1.97 m that 3 × rms exceeds the aperture. Then compare with "Run with the engine" — in a pure drift the linear preview and the engine should agree closely.

## Step 2: focusing with a solenoid {#tutorial-step2}

**What this step teaches**: how focusing brings the beam back, and why the focusing of a solenoid is so sensitive to its field.

Put a solenoid field map after the entrance:

```text
field 0.35 0.02 0 3 0 0 0 Kb sol
```

`type = 3` is a magnetostatic field, frequency and phase are 0, `Ke` is 0; the smoothed on-axis peak of the map `sol` is about 8.3 T (the map is noisy, single points reach 10 T), so $B_{z,\max} \approx 8.3\,K_b\ \mathrm{T}$.

A solenoid focuses in two stages: entering the fringe field, the radial field component gives the particle an azimuthal velocity and it starts to rotate about the axis; the rotating particle then feels a force towards the axis in the longitudinal field. Each stage contributes one factor $B$, so the focusing goes as $B^2$. Following the [solenoid](#solenoid) chapter, $k = B/(2B\rho)$ and the thin-lens focal length is about

$$
\frac{1}{f} \approx \int k^2\, \dd z = \int \left(\frac{B_z}{2B\rho}\right)^2 \dd z .
$$

The example protons have $B\rho \approx 0.179\ \mathrm{T\,m}$. For the `sol` map ($K_b = 1$) the integrals along the axis are $\int B_z\,\dd z = 0.729\ \mathrm{T\,m}$ and $\int B_z^2\,\dd z \approx 4.04\ \mathrm{T^2\,m}$ (the map noise makes the latter slightly too large). So $K_b = 0.25$, 0.35 and 0.45 give focal lengths of about 0.51, 0.26 and 0.16 m, and Larmor rotation angles $\theta = \int k\,\dd z$ of about $29^\circ$, $41^\circ$ and $53^\circ$.

In the preview: at the default $K_b = 0.35$ (peak about 2.9 T) the beam forms a waist at $z \approx 0.6\ \mathrm{m}$ (about 0.2 m after the solenoid exit) with a minimum rms $x$ of about 0.81 mm; $K_b = 0.25$ is too weak and rms $x$ only comes down to about 1.5 mm; $K_b = 0.45$ is too strong: the waist comes earlier and is narrower (about 0.42 mm), after which the beam diverges faster and reaches 3.2 mm at the exit.

> **Think about it**: why does changing $K_b$ only from 0.25 to 0.35 make such a difference?
>
> Answer: $1/f \propto B^2 \propto K_b^2$; a 40 % increase of $K_b$ almost doubles the focusing strength ($1.4^2 = 1.96$), and the focal length drops from 0.51 m to 0.26 m. This square law makes solenoids effective at low energy (small $B\rho$), and also sensitive to setting errors.

**Note**: a solenoid also rotates the transverse phase space about the axis, coupling $x$ and $y$, but for a round beam rms $x$ and rms $y$ remain equal. This is one reason solenoids are common at low energy: they focus both directions at once.

## Step 3: accelerating in a cavity {#tutorial-step3}

**What this step teaches**: how the synchronous phase trades energy gain against longitudinal focusing, and why longitudinal focusing necessarily comes with transverse defocusing.

Add an HWR:

```text
field 0.21 0.02 0 1 162.5e6 phis Ke -Ke hwr010
```

`type = 1` is an RF field; `V3 = 0` makes the sixth parameter the synchronous phase $\varphi_s$ (the engine finds the entrance phase itself); the electric field acts as $K_e E(\boldsymbol r)\cos(\omega t + \varphi_0)$ and the magnetic field as $K_b B(\boldsymbol r)\sin(\omega t + \varphi_0)$, here with $K_b = -K_e$ as in the example. The energy gain of the reference particle is approximately

$$
\Delta W = q V_0 T \cos\varphi_s ,
$$

see [longitudinal dynamics](#longitudinal). The preview of the cavity with $K_e = 1.36$:

| $\varphi_s$ | Exit energy (MeV) | Exit rms $z$ (mm) |
|---|---|---|
| −60° | 1.694 | 2.15 |
| −30° | 1.884 | 1.12 |
| 0° | 2.002 | 2.43 |
| +20° | 1.995 | 5.05 |

Near $\varphi_s = 0$ the gain is largest (on crest) but the bunch lengthens; with $\varphi_s < 0$ the gain is slightly smaller but the bunch is compressed — **phase stability**: with AVAS's $\cos(\omega t+\varphi)$ convention a negative synchronous phase gives late particles more energy, so they catch up with the reference particle. On the positive side ($+20^\circ$) there is no longitudinal focusing and the bunch lengthens quickly.

The price is [RF defocusing](#longitudinal): by Laplace's equation, the stronger the longitudinal focusing, the stronger the transverse defocusing. In the preview the largest rms $x$ is about 1.40 mm at $\varphi_s = 0^\circ$, 1.94 mm at $-30^\circ$ and 2.41 mm at $-60^\circ$.

> **Think about it**: the longitudinal focusing is strongest at $\varphi_s = -60^\circ$, so why is the exit rms $z$ (2.15 mm) larger than at $-30^\circ$ (1.12 mm)?
>
> Answer: one cavity is a single "longitudinal lens". At $-60^\circ$ this lens is too strong: the bunch is overcompressed inside the cavity and has lengthened again by the exit — the same thing as the early waist and larger exit size of step 2 when $K_b$ is too large.

**Try it**: fine-tune the phase near $-30^\circ$ and watch the trade between exit energy and rms $z$; then run the engine and compare the energy (the preview follows the on-axis reference particle, the engine shows the mean of all macro-particles; in the example cavity the mean gain is about 5 % higher, see "Running the engine" below) and rms $z$ (larger differences, because low-energy cavities are noticeably non-linear longitudinally and the linear preview has no longitudinal emittance growth).

## Step 4: a periodic accelerating section {#tutorial-step4}

**What this step teaches**: why repeating the same period keeps the beam confined over a long distance, and why a real design cannot use the same parameters in every period.

Repeating "solenoid + drift + cavity + drift" $N$ times gives the most common structure of superconducting linacs: one focusing element and one accelerating cavity per period.

```text
! period k
field 0.35 0.02 0 3 0 0 0 Kb sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 phis Ke -Ke hwr010
drift 0.1 0.02 0
```

The period is about 0.76 m long. This step starts from gentler defaults than step 3: $K_b = 0.30$ and $K_e = 0.8$ (step 3 uses $K_b = 0.35$, $K_e = 1.36$). The "Think about it" below explains why: repeating the period with the settings of step 3 gives a longitudinal phase advance of about $128^\circ$ in the first period, above the 90° rule, and on an 8-period line the engine loses particles from the bucket and rms $z$ grows to more than ten millimetres. Preview ($K_b = 0.30$, $\varphi_s = -30^\circ$, $K_e = 0.8$): about 1.73 MeV after 1 period, 2.28 MeV after 3, 3.35 MeV after 6 and 4.14 MeV after 8. The gains of the successive periods are about 0.21, 0.25, 0.30, 0.33, 0.36, 0.38, 0.39 and 0.40 MeV: as the particles speed up they approach the velocity the cavity is made for ($\beta \approx 0.10$) and the transit-time factor grows; after the 8th period $\beta \approx 0.094$ and the gain is still rising slowly. The largest transverse rms is about 1.95 mm and rms $z$ oscillates between about 1.1 and 2.1 mm instead of growing for ever — the beam is confined periodically in both the transverse and the longitudinal plane.

Some points (details in [lattice design](#lattice-design)):

- The transverse phase advance $\sigma_{0t}$ and the longitudinal phase advance $\sigma_{0l}$ per period should both stay below 90° and avoid low-order resonances between the transverse and longitudinal motion (for example when they are nearly equal or in a simple integer ratio; see the Hofmann stability chart).
- As the energy rises, $\beta\lambda$ gets longer and $B\rho$ larger, so the same $K_b$ focuses less, and the transit-time factor of the same cavity changes too. A real design adjusts $K_b$, $\varphi_s$ and even the cavity type section by section; this tutorial keeps the same parameters in every period for simplicity.
- The input beam is not matched to this periodic structure; the envelope swings of the first one or two periods are the sign of the mismatch, see [matching](#matching).

> **Think about it**: with the default parameters, does the phase advance per period stay below 90°?
>
> Answer: yes. Estimated from the period transfer matrices of the linear preview, the first period has a transverse phase advance (in the Larmor rotating frame, $x$ and $y$ planes) of about $74^\circ$–$77^\circ$ and a longitudinal one of about $86^\circ$; by the 3rd period they have dropped to about $58^\circ$–$62^\circ$ and $78^\circ$, by the 5th to about $48^\circ$–$51^\circ$ and $67^\circ$, and the transverse and longitudinal values do not come close to each other. With the step-3 settings $K_b = 0.35$, $K_e = 1.36$ the first period has about $90^\circ$–$94^\circ$ and $128^\circ$, and the 3rd still $95^\circ$ longitudinally; for a weak 0.18 mA beam the linear preview shows no problem (single particles only need less than $180^\circ$), but the non-linear longitudinal motion already makes the engine lose particles on a long line. The phase advances fall from period to period because $B\rho$ and $\beta^3\gamma^3$ grow with energy and the same solenoid and cavity become "weaker"; this is exactly why a real design lets $K_b$ and $K_e$ change with energy (the beam line of the [superconducting section](#journey-sc) chapter raises them period by period).
>
> A note about these numbers: the period starts and ends in field-free drifts, so with the cavity's damping divided out the period transfer matrix should be symplectic, with all eigenvalues of modulus 1. The linear preview gives moduli between 0.9996 and 1.0004 (exactly 1 at $K_e = 0$); the small remainder comes from the cavity, since with acceleration $(x, x')$ are not canonical coordinates. Earlier versions of the preview took the solenoid's radial fringe field from the `sol` map's own $B_x$, $B_y$, which disagree point by point with $-\tfrac12\,\partial B_z/\partial z$ (the map's transverse field is lower and broader than the slope of its on-axis field suggests, see [solenoid](#solenoid-field)); the moduli were then 0.95 and 1.05, and on a long line one mode grew by about 4 % per period. The radial fringe now comes from $B_z$ by Maxwell's equations.

**Try it**: raise the number of periods to 8 and see whether the energy grows linearly (it does not: the gain grows from period to period as the particles approach the velocity the cavity is made for); set $K_b$ to 0.2 or 0.45 and see the envelope run away. The two fail for different reasons: at $K_b = 0.2$ the solenoid is too weak to overcome the RF defocusing of the cavity (the solenoid alone gives about $57^\circ$; in the smooth approximation the RF defocusing subtracts $\sigma_{0l}^2/2$ from $\sigma_{0t}^2$, and $86^\circ/\sqrt2 \approx 61^\circ$ is more than the solenoid provides; the full period matrix leaves a transverse phase advance of only about $11^\circ$–$25^\circ$), there is hardly any net focusing, rms $x$ grows to about 8.4 mm and the preview warns that 3 × rms exceeds the aperture; at $K_b = 0.45$ the solenoid is too strong and alone exceeds the stability limit of the period ($\lvert\cos\sigma\rvert > 1$ in the first period), and rms $x$ grows to about 5.3 mm, at $K_b = 0.5$ beyond 20 mm.

## Step 5: adding beam current {#tutorial-step5}

**What this step teaches**: how space charge weakens the focusing, and why a few milliamperes already matter at low energy.

The last step lets you change `current` in `beam.txt`. Like charges repel, and the denser the beam, the stronger the outward push; it is as if the focusing of every period were weakened. Its strength is measured by the generalized perveance:

$$
K = \frac{q I}{2\pi \varepsilon_0 m c^3 (\beta\gamma)^3}
$$

(the generalized perveance of a continuous beam; for a bunched beam the average charge is used as an approximation.) The $(\beta\gamma)^3$ in the denominator has three sources: at low velocity the same current means a higher line charge density ($1/\beta$); the magnetic attraction between moving charges cancels part of the electric repulsion, leaving $1/\gamma^2$, and at low velocity hardly anything is cancelled; and the smaller the particle's momentum and velocity, the more a given transverse force deflects it ($1/\beta^2\gamma$). At the example's entrance $(\beta\gamma)^3 = 1.86\times 10^{-4}$, and 5 mA gives $K \approx 1.7\times 10^{-6}$.

In the preview, the largest rms $x$ over 3 periods is about 1.9 mm at 0.18 mA, 3.1 mm at 5 mA, 5.0 mm at 15 mA and 7.1 mm at 30 mA. The largest rms $z$ is about 2.3, 2.8, 3.7 and 4.7 mm: space charge weakens the longitudinal focusing too, and the bunch gets longer.

> **Think about it**: how much weaker is the space charge of the same 10 mA beam at 100 MeV?
>
> Answer: looking only at $K \propto 1/(\beta\gamma)^3$: 100 MeV protons have $\beta\gamma \approx 0.47$, about 8.3 times more than at 1.527 MeV, so $K$ is about 570 times smaller. That is why space charge is mainly a problem of the low-energy sections.

This step is the best one to check with the engine: the engine solves the self-field with macro-particles on a mesh and shows what the linear model cannot — emittance growth, halo and beam loss ("Transmission (engine)" below 100 % means particles hit the aperture). You can raise "Macro-particles" above 5000 to reduce the statistical noise, at the cost of a slower run, see [multi-particle simulation essentials](#simulation).

## Running the engine and saving as a project {#tutorial-engine}

"Run with the engine" writes the lattice of the current step (the text shown under "AVAS lattice of this step" in the tool) and the modified `beam.txt` into the sandbox, runs the engine with the example project's `input.txt` (space charge on, FFT solver), reads the rms sizes, energy and surviving macro-particles from `DataSet.txt` and plots them together with the preview. The run time grows with the number of periods and macro-particles; for reference, the example project (5260 macro-particles, one cavity) takes about 10–30 seconds on an ordinary computer.

When comparing, note:

- The preview's energy is that of the on-axis reference particle; the engine curve is the mean energy of all macro-particles. Off axis the particles see a different $E_z$: the example HWR map gives a few per cent more gain 1–2 mm off axis than on the axis, so the engine's mean energy gain is usually about 5 % higher than the preview's (the example's one cavity, around 1.5 MeV); at low energy the difference can be larger and of either sign. A difference of tens of per cent usually means a wrong phase or field factor.
- At low current the rms sizes of one cavity should agree within 1 %; over many periods the differences add up: in the engine the particles move in non-linear fields, the transverse emittance grows (about 16 % over the 8 periods of the step-4 defaults) and the beam centroid moves a few tenths of a millimetre off the axis, none of which the linear preview can compute, so after 8 periods rms sizes can differ by about 10 %. At high current the difference is larger, and the engine is the reference. With few macro-particles the engine's own results also scatter by a few per cent.
- If you changed parameters without running again, the tool tells you that the engine result belongs to the earlier parameters.

"Save as a new project" turns the current step into an ordinary AVAS project and opens it. You can then edit it further on the Lattice page (add output planes, errors, diagnostics …), run it on the Run page and analyse it on the Results page.

## Next steps {#tutorial-next}

- To design your own focusing period: read [linear optics](#linear-optics) and [lattice design](#lattice-design) first and estimate the phase advance with the FODO lab.
- To study tolerances: save the tutorial as a project, add cavity phase and amplitude errors and solenoid misalignments following [errors and correction](#errors), and run an error study.
- When something puzzles you, press "Ask the tutor" at the top of the page; the AI assistant explains with the help of this primer and the lab numbers.

## Common misconceptions {#tutorial-misconceptions}

- **The dashed line is not the result**: the dashed line is the linear preview, the solid line the engine result; they agree at low current and can differ a lot at high current or with strong longitudinal non-linearity.
- **$K_b$ is not the field**: it is the factor of the field map; the on-axis peak of the `sol` map is about $8.3\,K_b\ \mathrm{T}$.
- **The same parameters in every period are not a design**: the tutorial fixes the parameters for simplicity; a real superconducting section lets focusing and cavity voltage change with energy.
- **Envelope swings are not always a problem**: the swings of the first one or two periods come from the mismatch at the entrance and disappear with a matching section; swings that keep growing indicate instability.

## In AVAS {#tutorial-in-avas}

### Sandbox and runs

Engine runs happen in a sandbox copy in the AVAS data folder (`%LOCALAPPDATA%\AVAS\learn\runs\`); only the last 3 runs are kept, no project file is changed and no project needs to be open. While it runs, the Run page, tool bar and status bar show it and can pause or stop it; no other simulation can start meanwhile.

### The lattice of each step

The tutorial uses only the example project's `hwr010` and `sol` maps and drifts, with no matrix element at either end; the lattice text is all ASCII:

- Step 1: one `drift L 0.02 0` ($L$ 0.1–2 m, default 0.8 m).
- Step 2: `drift 0.05`, the solenoid, a drift after the solenoid (0.05–1.5 m, default 0.6 m).
- Steps 3–5: `drift 0.05`, then $N$ periods (solenoid, `drift 0.1`, HWR, `drift 0.1`; $N = 1$ in step 3, 1–8 in steps 4 and 5, default 3), and finally `drift 0.2`.

Parameter ranges: $K_b$ 0–0.8 (default 0.35, 0.30 in steps 4 and 5), $\varphi_s$ −90° to +30° (default −30°), $K_e$ 0–2.5 (default 1.36, 0.8 in steps 4 and 5), current 0–30 mA (default 5 mA), macro-particles of an engine run 200–20000 (default 2000).

### Beam and input files

The beam is the example project's `beam.txt` with two changes only: `current` in step 5, and the `particlenumber` used for the engine run. `input.txt` and `ini.ini` are the example project's files.

### Saving as a new project

"Save as a new project" writes only into a new or empty folder; it creates `InputFile/` (`beam.txt`, `input.txt`, `ini.ini`, `lattice_mulp.txt` and the field maps used) and `OutputFile/`, and opens the project.

## Further reading {#tutorial-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: superconducting linacs, solenoid focusing and RF defocusing.
- This primer's [lattice design](#lattice-design), [matching](#matching) and [multi-particle simulation essentials](#simulation); the complete list is in [references](#references).
