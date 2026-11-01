# Hands-on: build a small superconducting linac

This chapter ties the previous ones together. Using only two field maps of the example project `examples/hwr010` — the 162.5 MHz half-wave resonator `hwr010` (0.21 m) and the solenoid `sol` (0.35 m) — you build a proton superconducting accelerating section step by step. At every step you can drag the parameters and see the linear envelope preview at once, or press "Run with the engine" to let the AVAS multi-particle engine simulate it and overlay both results.

> **Note**: engine runs happen in a sandbox copy in the AVAS data folder (`%LOCALAPPDATA%\AVAS\learn\runs\`); only the last few runs are kept, no project file is changed and no project needs to be open. While it runs, the Run page, tool bar and status bar show it and can pause or stop it; no other simulation can start meanwhile. When you are happy with a step, "Save as a new project" turns it into an ordinary AVAS project.

## The tutorial workbench {#tutorial-workbench}

The tool below contains all five steps. When you move on to the next step, the parameters you chose (solenoid factor, phase …) are carried over.

```avas-tutorial
build-linac
```

Every step starts from the example project's `beam.txt`: protons, kinetic energy $W_0 = 1.527\ \mathrm{MeV}$ ($\beta \approx 0.057$), frequency 162.5 MHz, Twiss parameters $\alpha_x = -0.658$, $\beta_x = 0.377\ \mathrm{mm/mrad}$, normalized rms emittance $\varepsilon_{n,x} = 0.138\ \pi\,\mathrm{mm\,mrad}$ (similar in $y$), current 0.184 mA (adjustable in step 5). $\alpha_x < 0$ means the beam is diverging at the entrance.

> **AVAS convention**: dashed curves (or the only curve) are the linear envelope preview, a quick approximation (rms usually within 1–2 %, up to some ten percent with strong multi-particle effects); the solid curves that appear after an engine run are the multi-particle engine, which is the reference. See [linear preview and engine](#simulation).

## Step 1: a beam in a drift {#tutorial-step1}

A single drift `drift L 0.02 0`. The beam enters diverging ($\alpha<0$) and nothing focuses it, so the rms size grows as in the [drift](#drift) chapter:

$$
\sigma_x^2(s) = \sigma_{x,0}^2 - 2\sigma_{xx',0}\, s + \sigma_{x'x',0}\, s^2 ,
$$

i.e. $\sigma_x(s) = \sqrt{\varepsilon\,(\beta_0 - 2\alpha_0 s + \gamma_0 s^2)}$. For $L = 0.8\ \mathrm{m}$ the preview gives rms $x$ growing from 0.95 mm to about 3.1 mm. The longitudinal plane behaves alike: with no focusing force the bunch lengthens through its momentum spread ($R_{56} = L/\gamma^2$), rms $z$ from about 1.3 mm to about 3.1 mm.

**Try**: increase $L$ to 2 m and see how quickly the envelope approaches the aperture (be careful beyond about a third of the 20 mm bore); then compare with "Run with the engine" — in a plain drift the preview and the engine should agree closely.

## Step 2: focusing with a solenoid {#tutorial-step2}

A field-map solenoid follows the entrance:

```text
field 0.35 0.02 0 3 0 0 0 Kb sol
```

`type = 3` is a static magnetic field, frequency and phase are 0 and so is `Ke`; the smoothed on-axis peak of the map `sol` is about 8.3 T (the map is noisy; single points reach 10 T), so $B_{z,\max} \approx 8.3\,K_b\ \mathrm{T}$. As in the [solenoid](#solenoid) chapter the focusing strength is $k = B/(2B\rho)$ and the thin-lens focal length is about

$$
\frac{1}{f} \approx \int k^2\, \dd z = \int \left(\frac{B_z}{2B\rho}\right)^2 \dd z .
$$

The example protons have $B\rho \approx 0.179\ \mathrm{T\,m}$. With the default $K_b = 0.35$ (peak about 2.9 T) the beam forms a waist at $z \approx 0.6\ \mathrm{m}$ (about 0.2 m after the solenoid exit), smallest rms $x$ about 0.82 mm; $K_b = 0.25$ is too weak (rms $x$ only comes down to about 1.5 mm); $K_b = 0.45$ is too strong: the waist comes earlier and is smaller (about 0.43 mm), after which the beam diverges faster and reaches 3.1 mm at the exit. The focal length scales with $B^2$, so a 30 % change of $K_b$ shows clearly.

**Note**: the solenoid also rotates the transverse phase space about the axis (Larmor angle $\theta = \int k\,\dd z$) and couples $x$ and $y$, but for a round beam rms $x$ and rms $y$ stay equal. This is why low-energy sections like solenoids: they focus both planes at once.

## Step 3: accelerating in a cavity {#tutorial-step3}

An HWR cavity is added:

```text
field 0.21 0.02 0 1 162.5e6 phis Ke -Ke hwr010
```

`type = 1` is an RF field; `V3 = 0` makes the sixth parameter the synchronous phase $\varphi_s$ (the engine solves for the entry phase). The electric field acts as $K_e E(\boldsymbol r)\cos(\omega t + \varphi_0)$, the magnetic field as $K_b B(\boldsymbol r)\sin(\omega t + \varphi_0)$; as in the example, $K_b = -K_e$. The reference particle gains approximately

$$
\Delta W = q V_0 T \cos\varphi_s ,
$$

see [longitudinal dynamics](#longitudinal). The preview for $K_e = 1.36$:

| $\varphi_s$ | exit energy (MeV) | exit rms $z$ (mm) |
|---|---|---|
| −60° | 1.694 | 2.15 |
| −30° | 1.884 | 1.12 |
| 0° | 2.002 | 2.43 |
| +20° | 1.995 | 5.05 |

The gain is largest near $\varphi_s = 0$ (on crest), but the bunch lengthens; for $\varphi_s < 0$ the gain is slightly lower and the bunch is compressed — **phase stability**: in AVAS's $\cos(\omega t+\varphi)$ convention a negative synchronous phase gives late particles more energy, so they catch up with the reference particle. On the positive side ($+20^\circ$) there is no longitudinal focusing and the bunch lengthens quickly. The price is [RF defocusing](#longitudinal): the stronger the longitudinal focusing, the stronger the transverse defocusing, which is why the transverse envelope is largest at $\varphi_s = -60^\circ$.

**Try**: fine-tune the phase around $-30^\circ$ and watch the trade-off between exit energy and rms $z$; then run the engine and compare the energy (preview and engine usually differ by about 0.01 %) and rms $z$ (larger differences: longitudinal non-linearity is strong in a low-energy cavity and the linear preview does not model longitudinal emittance growth).

## Step 4: a periodic accelerating section {#tutorial-step4}

Repeating "solenoid + drift + cavity + drift" $N$ times gives the most common structure of a superconducting linac: one focusing element and one cavity per period.

```text
! period k
field 0.35 0.02 0 3 0 0 0 Kb sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 phis Ke -Ke hwr010
drift 0.1 0.02 0
```

A period is about 0.76 m long. The preview ($K_b = 0.35$, $\varphi_s = -30^\circ$, $K_e = 1.36$) gives about 1.88 MeV after 1 period, 2.96 MeV after 3 and 4.99 MeV after 6. The first period gains about 0.36 MeV, later ones gradually more (on average about 0.58 MeV over 6 periods): as the particles speed up they come closer to the velocity this cavity suits and its transit-time factor grows. The largest transverse rms is about 1.8 mm and rms $z$ oscillates below about 3 mm instead of growing without bound — both planes are held by the periodic focusing.

Points to keep in mind (see [building an accelerator](#lattice-design)):

- The transverse and longitudinal phase advances per period, $\sigma_{0t}$ and $\sigma_{0l}$, should both stay below 90°, and low-order resonances between the two planes (for example both nearly equal, or in a simple integer ratio; see the Hofmann stability chart) should be avoided.
- As the energy rises, $\beta\lambda$ grows and $B\rho$ increases, so the same $K_b$ focuses less and the cavity's transit-time factor changes. Real designs adjust $K_b$, $\varphi_s$ and even the cavity type section by section; this tutorial keeps one set of values for simplicity.
- The entrance beam is not matched to this periodic structure: the envelope wobble over the first periods is the sign of the mismatch, see [matching](#matching).

**Try**: increase the number of periods to 8 and check whether the energy keeps growing; set $K_b$ to 0.25 or 0.45 and watch the envelope get out of hand.

## Step 5: adding beam current {#tutorial-step5}

The last step lets you change `current` in `beam.txt`. The higher the current, the stronger the defocusing by [space charge](#space-charge), which effectively weakens the focusing of every period:

$$
K = \frac{q I}{2\pi \varepsilon_0 m c^3 (\beta\gamma)^3}
$$

(generalized perveance of a continuous beam; for a bunched beam use the average charge as an approximation). The preview's largest rms $x$ over 3 periods: about 1.8 mm at 0.18 mA, 2.8 mm at 5 mA, 4.6 mm at 15 mA and 6.7 mm at 30 mA; rms $z$ grows too. At low energy ($\beta\gamma \approx 0.057$) $(\beta\gamma)^3$ is small, so a few milliamperes already matter.

This step is the one to check with the engine: it solves the self-fields with macro-particles on a mesh and shows what the linear model cannot — emittance growth, halo and losses ("Transmission (engine)" below 100 % means particles hit the aperture). Raise "Macro-particles" above 5000 to reduce the statistical noise, at the cost of a slower run, see [multi-particle simulation](#simulation).

## Engine runs and saving as a project {#tutorial-engine}

"Run with the engine" writes the step's lattice (the text shown under "AVAS lattice of this step") and the modified `beam.txt` into the sandbox, runs the engine with the example project's `input.txt` (space charge on, FFT solver), reads rms sizes, energy and surviving macro-particles from `DataSet.txt` and draws them over the preview. The run time grows with the number of periods and of macro-particles; for reference, the example project (5260 macro-particles, one cavity) takes about 10–30 s on an ordinary computer.

When comparing:

- The energy curves should almost coincide; a clear difference usually means a wrong phase or field factor.
- At low current the rms sizes should be close (within a few percent); at high current the difference grows and the engine is the reference.
- If you changed parameters after the run, the tool tells you that the engine result belongs to the earlier parameters.

"Save as a new project" creates `InputFile/` (`beam.txt`, `input.txt`, `ini.ini`, `lattice_mulp.txt` and the field maps used) and `OutputFile/` in a new folder of your choice and opens it. From there you can keep editing on the Lattice page (output planes, errors, diagnostics …), run it from the Run page and analyse it on the Results page.

## Where to go next {#tutorial-next}

- To design your own focusing period: read [linear optics](#linear-optics) and [building an accelerator](#lattice-design), and estimate the phase advance with the FODO lab.
- To study tolerances: save the tutorial as a project and add cavity phase and amplitude errors and solenoid misalignments as in [errors and correction](#errors), then run an error study.
- If something puzzles you, press "Ask the tutor" at the top of the page: the AI assistant explains it with this primer and the lab numbers.
