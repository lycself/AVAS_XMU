# Hands-on: building a small superconducting linac

With nothing but the field maps of one solenoid and one accelerating cavity, can we build a real piece of linear accelerator — one that accelerates protons and keeps the beam a thin pencil all the way? This chapter tries it out by hand.

The materials are two field maps from the example project `examples/hwr010`; a "field map" is the electromagnetic field of an element, computed beforehand and stored in files. One is the 162.5 MHz half-wave resonator `hwr010` (0.21 m long, for acceleration), the other the solenoid `sol` (0.35 m long, for focusing). In five steps we build a superconducting proton accelerating section; each step adds one thing and teaches one idea:

1. why a beam grows by itself;
2. how focusing brings it back;
3. why acceleration also spreads the beam transversely;
4. why repeating the same period many times keeps the beam stable for a long distance;
5. why more charge in the beam (a higher current) changes everything.

At every step you can drag the parameters and see the linear envelope preview (a very fast approximate calculation) at once, or press "Run with the engine" to let the AVAS multi-particle engine simulate it and overlay both results. Before you drag a parameter, **try to guess how the curves will change, then look**. Where your guess was wrong is exactly where it pays to go back to an earlier chapter. Engine runs happen in a sandbox, need no open project and change no project file (details in [In AVAS](#tutorial-in-avas)).

This chapter ties together the chapters on [drifts](#drift), [solenoids](#solenoid), [RF cavities](#rf-cavity), [longitudinal dynamics](#longitudinal), [lattice design](#lattice-design), [matching](#matching) and [space charge](#space-charge); where this section sits in a whole accelerator is shown in [the superconducting section](#journey-sc). You can start before reading those chapters: the main text explains what you see in the most intuitive terms, and the formulas are in the "Going deeper" parts at the end.

## The tutorial tool {#tutorial-workbench}

The tool below contains all five steps. When you switch steps, the parameters you **changed yourself** in the previous step (the solenoid factor or the phase, for example) are carried over; the ones you did not change take the new step's own defaults.

```avas-tutorial
build-linac
```

All steps start from the beam of the example project's `beam.txt`: protons with kinetic energy $W_0 = 1.527\ \mathrm{MeV}$, moving at about 5.7 % of the speed of light ($\beta \approx 0.057$), RF frequency 162.5 MHz. At the entrance the beam's rms size (the typical distance of a particle from the centre) is 0.95 mm. The beam is diverging at the entrance, like light just leaving a torch (in Twiss parameters: $\alpha_x = -0.658$, $\beta_x = 0.377\ \mathrm{mm/mrad}$, normalized rms emittance $\varepsilon_{n,x} = 0.138\ \pi\,\mathrm{mm\,mrad}$, similar in $y$; $\alpha_x < 0$ is what "diverging" means, see [Drift](#drift-deep-matrix)). Steps 1–4 use the 0.184 mA current of `beam.txt`; in step 5 the current can be changed (default 5 mA).

> **AVAS convention**: the dashed curves (or the only curve, before any engine run) are the linear envelope preview, a quick approximation (rms usually within 1–2 %, more than ten per cent where multi-particle effects are strong); the solid curves that appear after an engine run are the multi-particle engine's result and are the reference. See [linear preview and engine](#simulation-preview).

## Step 1: a beam in a drift {#tutorial-step1}

**What this step teaches**: why a beam grows even when no force acts on it.

**What to build**: a single drift `drift L 0.02 0`, i.e. an empty pipe of length $L$ and radius 2 cm.

**Why it grows**: every particle flies in a straight line, but their directions differ, like the rays from a torch, which spread farther apart the farther they go. The beam is already diverging at the entrance (particles farther from the centre fly farther outwards), so it grows from the start.

**What to look at**: with the default $L = 0.8\ \mathrm{m}$, the preview shows rms $x$ growing from 0.95 mm to about 3.1 mm. Far from the entrance the size grows almost in proportion to the distance (the angles stay the same, so the offsets grow with the distance).

The same happens along the direction of motion: nothing in a drift "squeezes" the bunch together; particles with slightly more energy are slightly faster and run ahead, those with slightly less fall behind, and the bunch lengthens. The example beam has an rms momentum spread of about 0.35 %, and rms $z$ (the typical bunch length) grows from about 1.3 mm to about 3.1 mm.

**Try it**: first guess how wide the beam is after a 2 m pipe. Drag $L$ to 2 m: the preview shows rms $x$ of about 6.8 mm and warns near 1.97 m that "3 × rms exceeds the aperture". The pipe radius is 20 mm; once 3 times the rms beam size touches the wall, particles at the edge are lost (as a rule of thumb, be careful once the rms reaches about 1/3 of the aperture). Then compare with "Run with the engine": in a pure drift the linear preview and the engine should agree closely.

## Step 2: focusing with a solenoid {#tutorial-step2}

**What this step teaches**: how focusing brings the beam back, and why the focusing of a solenoid is so sensitive to the field strength.

**What to build**: a solenoid field map after the entrance:

```text
field 0.35 0.02 0 3 0 0 0 Kb sol
```

The line means: a field-map element 0.35 m long with an aperture radius of 2 cm; `3` means a static magnetic field, frequency and phase are 0, the electric factor `Ke` is 0; `Kb` is the magnetic factor — the field in the map times `Kb` is the actual field; the final `sol` is the field-map file name. The smoothed on-axis peak of the `sol` map is about 8.3 T (the map is noisy, single points reach 10 T), so the actual peak field is about $8.3\,K_b\ \mathrm{T}$.

**Why it focuses**: the field inside a solenoid points mostly along the direction of motion. Where a particle enters the solenoid, the field fans out slightly, and this outward component sets the particle circling about the axis; a circling particle in the field along the axis then feels a force towards the centre. Each of the two steps uses the field once, so the focusing is proportional to **the square of the field**.

**What to look at**: with the default $K_b = 0.35$ (peak about 2.9 T) the beam is narrowest at $z \approx 0.6\ \mathrm{m}$ (about 0.2 m after the solenoid exit); this narrowest point is called the "waist", and rms $x$ there is about 0.81 mm. $K_b = 0.25$ is too weak: near the solenoid exit the beam merely stops growing (about 1.5 mm) and then keeps growing. $K_b = 0.45$ is too strong: the waist comes earlier and is narrower (about 0.42 mm), the beam then spreads faster and is 3.2 mm at the exit.

**Try it**: first guess: $K_b$ from 0.25 to 0.35 is only 40 % more — how much stronger is the focusing? Almost twice as strong: the focusing goes with the square of the field, $1.4^2 = 1.96$, and the focal length shrinks from 0.51 m to 0.26 m. This square law makes solenoids effective at low energy and also very sensitive to setting errors. The calculation is in "Going deeper: checking each step with formulas".

**Note**: a solenoid also turns the particles by an angle about the axis, mixing the $x$ and $y$ directions (coupling); for a round beam, rms $x$ and rms $y$ nevertheless stay equal. A solenoid focuses both directions at once, one reason why low-energy sections like to use it.

## Step 3: accelerating in a cavity {#tutorial-step3}

**What this step teaches**: how the synchronous phase sets the trade between "how much acceleration" and "how tightly the bunch is squeezed", and why squeezing along the direction of motion spreads the beam transversely.

**What to build**: add an HWR accelerating cavity:

```text
field 0.21 0.02 0 1 162.5e6 phis Ke -Ke hwr010
```

`1` means an RF field, `162.5e6` is the frequency (Hz), `phis` the synchronous phase $\varphi_s$, `Ke` and `-Ke` the electric and magnetic factors (as in the example, $K_b = -K_e$); the third parameter of the line is 0 (V3 = 0), which means the sixth parameter is the synchronous phase and the engine solves for the entrance phase itself. The defaults of this step are $K_b = 0.35$ and $K_e = 1.36$.

**What the phase is**: the field in the cavity swings back and forth 162.5 million times per second, and when a particle crosses the cavity decides how strong a field it meets and whether the field points forwards or backwards. The **synchronous phase** is the timing of the reference particle (the ideal particle at the bunch centre): $0^\circ$ is the crest, where the field is strongest, and negative values are before the crest. In the AVAS convention the electric field varies as $K_e E(\boldsymbol r)\cos(\omega t + \varphi_0)$ and the magnetic field as $K_b B(\boldsymbol r)\sin(\omega t + \varphi_0)$.

**What to look at**: in the preview, with the cavity at $K_e = 1.36$:

| $\varphi_s$ | Exit energy (MeV) | Exit rms $z$ (mm) |
|---|---|---|
| −60° | 1.694 | 2.15 |
| −30° | 1.884 | 1.12 |
| 0° | 2.002 | 2.43 |
| +20° | 1.995 | 5.05 |

Near $\varphi_s = 0$ the acceleration is largest (riding the crest), but the bunch is lengthened; at $\varphi_s < 0$ the acceleration is a little smaller, but the bunch is squeezed. This is **phase stability**: at a negative synchronous phase, a late particle meets a stronger field, gains a little more energy, speeds up and catches up with the reference particle; an early particle gains less and slows down to wait for it. The positive side ($+20^\circ$) is the other way round: there is no such automatic correction and the bunch quickly lengthens.

The price is [RF defocusing](#longitudinal): the laws of the electric field make it so that the more tightly the bunch is squeezed along the direction of motion, the harder it is pushed apart transversely. In the preview the largest transverse rms $x$ is about 1.40 mm at $\varphi_s = 0^\circ$, 1.94 mm at $-30^\circ$ and 2.41 mm at $-60^\circ$.

**Try it**: fine-tune the phase near $-30^\circ$ and watch exit energy and rms $z$ trade against each other; then run the engine and compare the energy (the preview follows the on-axis reference particle, the engine shows the mean of all macro-particles; in the example cavity the mean gain is about 5 % higher, see "Running the engine" below) and rms $z$ (larger differences, because low-energy cavities are noticeably non-linear longitudinally and the linear preview has no longitudinal emittance growth).

## Step 4: a periodic accelerating section {#tutorial-step4}

**What this step teaches**: why repeating the same period confines the beam for a long distance, and why a real design cannot use the same settings in every period.

**What to build**: repeating "solenoid + drift + cavity + drift" $N$ times gives the most common structure of superconducting linacs: one focusing element and one accelerating cavity per period.

```text
! period k
field 0.35 0.02 0 3 0 0 0 Kb sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 phis Ke -Ke hwr010
drift 0.1 0.02 0
```

The period is about 0.76 m long. This step starts from gentler defaults than step 3: $K_b = 0.30$ and $K_e = 0.8$ (step 3 uses $K_b = 0.35$, $K_e = 1.36$). Why change? Repeating the period with the settings of step 3, the bunch "swings" too fast along the direction of motion in each period (the longitudinal phase advance of the first period is about $128^\circ$, above the 90° rule discussed below), and on an 8-period line the engine loses particles from the stable region and rms $z$ grows to more than ten millimetres.

**What to look at** (preview, $K_b = 0.30$, $\varphi_s = -30^\circ$, $K_e = 0.8$): about 1.73 MeV after 1 period, 2.28 MeV after 3, 3.35 MeV after 6 and 4.14 MeV after 8. The gains of the successive periods are about 0.21, 0.25, 0.30, 0.33, 0.36, 0.38, 0.39 and 0.40 MeV, growing: as the particles speed up they come ever closer to the velocity the cavity is made for ($\beta \approx 0.10$), and the cavity's transit-time factor (a factor measuring how much of the field a particle can use while crossing the cavity) grows; after the 8th period $\beta \approx 0.094$ and the gain is still rising slowly. The largest transverse rms is about 1.95 mm, and after the first period rms $z$ goes up and down between about 1.1 and 2.1 mm instead of growing for ever: the beam is confined periodically both transversely and longitudinally.

**Why it is confined**: in each period the solenoid first gathers the beam, which then spreads in the drifts and the cavity, and the next period gathers it again. The particles swing back and forth about the centre instead of flying off. The "angle" a particle swings through per period is called the **phase advance** (a full swing being $360^\circ$). Design follows a few rules of thumb (details in [lattice design](#lattice-design)):

- The transverse and the longitudinal phase advance per period should both stay below 90°, and "resonances" between transverse and longitudinal motion should be avoided (for example the two being close to equal or in a simple integer ratio, see the Hofmann stability chart). With the default settings the first period has about $74^\circ$–$77^\circ$ transversely and about $86^\circ$ longitudinally, within this rule.
- As the energy rises the particles get "stiffer" ($B\rho$ grows, and the same solenoid can bend them less) and the bunch gets harder to squeeze, so the same $K_b$ focuses less, the effect of the cavity changes too, and the phase advances fall from period to period. A real design adjusts $K_b$, $\varphi_s$ and even the cavity type section by section; for simplicity this tutorial uses the same settings in every period.
- The entrance beam is not matched to this periodic structure (the beam's shape does not fit the channel); the envelope ripples of the first one or two periods show the mismatch, see [matching](#matching).

**Try it**: raise the number of periods to 8 and see whether the energy grows in proportion to the number of periods (it does not: the gain grows from period to period); set $K_b$ to 0.2 or 0.45 and watch the envelope run away. The two run away for different reasons: at $K_b = 0.2$ the solenoid is too weak to overcome the RF defocusing of the cavity, there is almost no net focusing, rms $x$ grows to about 8.4 mm and the preview warns that 3 × rms exceeds the aperture; at $K_b = 0.45$ the solenoid is too strong, focuses "too much" and exceeds the limit of periodic stability by itself, rms $x$ grows to about 5.3 mm, and at $K_b = 0.5$ beyond 20 mm. The phase-advance numbers are in "Going deeper: the phase advance per period and the 90° rule".

## Step 5: adding beam current {#tutorial-step5}

**What this step teaches**: how the charge in the beam weakens the focusing, and why a few milliamperes already matter at low energy.

**What to build**: the structure of step 4 (by default 3 periods, $K_b = 0.30$, $K_e = 0.8$), now with the beam current `current` of `beam.txt` adjustable.

**Why**: all protons are positively charged, and like charges repel. The denser the beam, the stronger the outward push, which cancels part of the focusing. This effect is called **space charge**. It is stronger the slower the particles: a slow beam is packed more densely into the same length of pipe, and slow particles are more easily pushed aside. At the example's entrance energy, 5 mA already matter clearly.

**What to look at**: the largest rms $x$ over 3 periods in the preview is about 1.9 mm at 0.18 mA, 3.1 mm at 5 mA, 5.0 mm at 15 mA and 7.1 mm at 30 mA. The largest longitudinal rms $z$ is about 2.3, 2.8, 3.7 and 4.7 mm respectively: space charge weakens the longitudinal "squeeze" as well, and the bunch lengthens.

This step is the best one to check with the engine: the engine solves the beam's own field with macro-particles on a mesh and shows what the linear model cannot — emittance growth, halo and beam loss ("Transmission (engine)" below 100 % means particles hit the pipe wall). You can raise "Macro-particles" above 5000 to reduce the statistical noise, at the cost of a slower run, see [multi-particle simulation essentials](#simulation). The formula for the strength of space charge is step four of "Going deeper: checking each step with formulas".

## Running the engine and saving as a project {#tutorial-engine}

"Run with the engine" writes the lattice of the current step (the text shown under "AVAS lattice of this step" in the tool) and the modified `beam.txt` into the sandbox, runs the engine with the example project's `input.txt` (space charge on, FFT solver), reads the rms sizes, energy and surviving macro-particles from `DataSet.txt` and plots them together with the preview. The run time grows with the number of periods and macro-particles; for reference, the example project (5260 macro-particles, one cavity) takes about 10–30 seconds on an ordinary computer.

When comparing, keep in mind:

- The preview's energy is that of the on-axis reference particle; the engine curve is the mean energy of all macro-particles. Off-axis particles feel a different accelerating field than on axis: in the example HWR field map the gain 1–2 mm off axis is a few per cent higher than on axis, so the gain of the engine's mean energy is usually about 5 % higher than the preview's (the example's single cavity near 1.5 MeV); at lower energy the difference can be larger and even change sign. A difference of tens of per cent most likely means a wrong phase or field factor.
- At low current the rms sizes of a single cavity should agree within 1 %; with more periods the differences accumulate: particles in the engine move in non-linear fields, the transverse emittance grows (about 16 % over the 8 periods of step 4's defaults) and the beam centroid wanders a few tenths of a millimetre off axis, none of which the linear preview can compute, so after 8 periods the rms can differ by about 10 %. At high current the differences are larger; the engine is the reference. With few macro-particles the engine result itself fluctuates statistically by a few per cent.
- If parameters were changed without a new run, the tool says that the engine result belongs to the earlier parameters.

"Save as a new project" turns the current step into an ordinary AVAS project and opens it. You can then edit it further on the Lattice page (add output planes, errors, diagnostics …), run it on the Run page and analyse it on the Results page.

## Next steps {#tutorial-next}

- To design your own focusing period: read [linear optics](#linear-optics) and [lattice design](#lattice-design) first and estimate phase advances with the FODO lab.
- To study tolerances: save the tutorial project, add cavity phase and amplitude errors and solenoid misalignments as in [errors and correction](#errors), and run an error study.
- When something puzzles you, press "Ask the tutor" at the top of the page; the AI assistant explains with the help of this primer and the lab numbers.

## Think about it {#tutorial-think}

> **Think about it 1**: in the drift of step 1, does the emittance change?
>
> Answer: no. The ellipse in phase space is "pushed askew" (positions shift with angles); its area stays the same, only its shape changes. The beam grows not because the emittance grows but because the same phase-space area projects more widely onto $x$.

> **Think about it 2**: in step 3 the longitudinal focusing is strongest at $\varphi_s = -60^\circ$. Why is the exit rms $z$ (2.15 mm) then larger than at $-30^\circ$ (1.12 mm)?
>
> Answer: one cavity is a single "longitudinal lens". At $-60^\circ$ this lens is too strong: the bunch is overcompressed inside the cavity and has lengthened again by the exit — the same thing as the early waist and wider exit beam of step 2 when $K_b$ is too large.

> **Think about it 3**: with the defaults of step 4, does every period satisfy "phase advance below 90°"?
>
> Answer: yes. The first period has about $74^\circ$–$77^\circ$ transversely and about $86^\circ$ longitudinally, falling from period to period afterwards. With the step-3 settings $K_b = 0.35$, $K_e = 1.36$ the first period already has about $90^\circ$–$94^\circ$ and $128^\circ$. See "Going deeper: the phase advance per period and the 90° rule".

> **Think about it 4**: how much weaker is space charge for the same 10 mA beam at 100 MeV?
>
> Answer: the strength of space charge goes as $1/(\beta\gamma)^3$ (see step four of "Going deeper"). A 100 MeV proton has $\beta\gamma \approx 0.47$, about 8.3 times more than at 1.527 MeV, so space charge is about 570 times weaker. That is why space charge is mainly a low-energy problem.

## Common misconceptions {#tutorial-misconceptions}

- **The dashed curve is not the result**: the dashed curve is the linear preview, the solid one the engine result; they agree at low current and can differ a lot at high current or with strong longitudinal nonlinearity.
- **$K_b$ is not the field**: it is a factor of the field map; the on-axis peak of the `sol` map is about $8.3\,K_b\ \mathrm{T}$.
- **The same settings in every period are not a design**: the tutorial fixes them for simplicity; a real superconducting section changes focusing and cavity voltage with energy.
- **Envelope ripples are not necessarily a problem**: the ripples of the first one or two periods come from the entrance mismatch and disappear with a matching section; only ripples that keep growing indicate instability.

## Going deeper: checking each step with formulas {#tutorial-deep-formulas}

The main text only described what happens. This section writes down the formulas behind each step and checks the preview's numbers with the example beam. The tools are, in order, the school formula for the square of a sum, the Lorentz force, energy = charge × voltage, and Coulomb's law; the full derivation of each formula is in the corresponding element chapter.

### Step one: the beam size in a drift

The [Drift](#drift-deep-matrix) chapter derives: with the mean of the squared position $\sigma_{x,0}^2$, the mean of position times angle $\sigma_{xx',0}$ and the mean of the squared angle $\sigma_{x'x',0}$ at the entrance, after a distance $s$

$$
\sigma_x^2(s) = \sigma_{x,0}^2 + 2\sigma_{xx',0}\, s + \sigma_{x'x',0}\, s^2 ,
$$

which is just $x = x_0 + x's$ squared and averaged (the square of a sum). In Twiss parameters, $\sigma_x(s) = \sqrt{\varepsilon\,(\beta_0 - 2\alpha_0 s + \gamma_0 s^2)}$: with $\alpha_0 < 0$ (diverging) the middle term is positive and the beam grows from the start; for large $s$ the term $\gamma_0 s^2$ dominates and $\sigma_x$ is almost proportional to $s$. With the example ($\varepsilon = 2.41\ \mathrm{mm\,mrad}$, $\beta_0 = 0.377$, $\alpha_0 = -0.658$, $\gamma_0 = 3.80$): at $s = 0.8\ \mathrm{m}$, $\sigma_x = \sqrt{2.41 \times (0.377 + 1.053 + 2.432)} = 3.05\ \mathrm{mm}$; the preview gives about 3.1 mm (it also includes the space charge of 0.184 mA).

Longitudinally: a particle with momentum deviation $\delta = \Delta p/p$ moves away from the synchronous particle by $\delta/\gamma^2$ per metre, i.e. $R_{56} = L/\gamma^2$ ("Going deeper: why bunches lengthen" in the Drift chapter). For the example $\gamma \approx 1.0016$, practically "1 % more momentum, 1 % more speed".

### Step two: the focal length of the solenoid

The [Solenoid](#solenoid) chapter gives the focusing strength $k = B/(2B\rho)$, where $B\rho$ is the magnetic rigidity (momentum ÷ charge, $B\rho \approx 0.179\ \mathrm{T\,m}$ for the example protons). The 2 in the denominator corresponds to "each of the two steps uses the field once". The thin-lens focal length is about

$$
\frac{1}{f} \approx \int k^2\, \dd z = \int \left(\frac{B_z}{2B\rho}\right)^2 \dd z ,
$$

where "$\int \dots \dd z$" means adding up the contributions of every short piece along the axis. For the `sol` map ($K_b = 1$), summing along the axis gives $\int B_z\,\dd z = 0.729\ \mathrm{T\,m}$ and $\int B_z^2\,\dd z \approx 4.04\ \mathrm{T^2\,m}$ (the noise of the map makes the latter slightly too large). Multiplying by $K_b^2$:

$$
\frac1f = K_b^2 \times \frac{4.04}{4 \times 0.179^2} = 31.7\,K_b^2\ \mathrm{m^{-1}} .
$$

For $K_b = 0.25$, 0.35 and 0.45 the focal lengths are about 0.51, 0.26 and 0.16 m. The angle by which the beam turns about the axis (the Larmor angle) is $\theta = \int k\,\dd z = K_b \times 0.729/(2 \times 0.179) = 2.04\,K_b$ radians, i.e. about $29^\circ$, $41^\circ$ and $53^\circ$.

### Step three: the energy gain of the cavity and phase stability

The energy gain of the reference particle is approximately (energy = charge × voltage, times the "timing")

$$
\Delta W = q V_0 T \cos\varphi_s ,
$$

with $V_0T$ the effective voltage of the cavity (see [longitudinal dynamics](#longitudinal)). $\cos\varphi_s$ is largest at $\varphi_s = 0$, so the crest accelerates most; on the side $\varphi_s < 0$, $\cos$ grows with the phase, so a late particle (larger phase) gains more energy: that is phase stability. In the table, the gain at $-30^\circ$, 0.357 MeV, divided by that at $0^\circ$, 0.475 MeV, is 0.75, less than $\cos 30^\circ = 0.87$: in a low-energy cavity the particle accelerates noticeably inside, and the effective voltage itself depends on the phase (see [Errors and correction](#errors-deep-rf)).

RF defocusing comes from Laplace's equation for the electric field: in vacuum the "divergence" of the field adds up to zero, so a field component that pushes particles towards the middle along the direction of motion must come with a component that pushes them outwards transversely. The stronger the longitudinal focusing, the stronger the transverse defocusing.

### Step four: the strength of space charge

The strength of space charge is measured by the **generalized perveance**:

$$
K = \frac{q I}{2\pi \varepsilon_0 m c^3 (\beta\gamma)^3}
$$

(the generalized perveance of a continuous beam; for a bunched beam the average charge is used as an approximation). The larger the current $I$ in the numerator, the stronger the push; the $(\beta\gamma)^3$ in the denominator is a product of three factors: for slower particles the same current means more charge per metre of pipe ($1/\beta$); moving charges also produce a magnetic field whose attraction cancels part of the electric repulsion, leaving $1/\gamma^2$; and particles with less momentum are more easily pushed aside transversely ($1/\beta^2\gamma$). The full derivation is in [space charge](#space-charge-perveance).

At the example's entrance: $\beta\gamma = 0.05708$, $(\beta\gamma)^3 = 1.86\times 10^{-4}$, and at 5 mA $K \approx 1.7\times 10^{-6}$. A 100 MeV proton has $\beta\gamma \approx 0.47$, $(\beta\gamma)^3$ is about 570 times larger, and $K$ for the same current about 570 times smaller.

### For readers who know calculus and electrodynamics

Step one is the special case $\Sigma_1 = M\Sigma_0M\tp$ for the drift matrix. The solenoid's $1/f = \int k^2\,\dd z$ holds in the Larmor rotating frame in the thin-lens limit; in the rotating frame $x$ and $y$ decouple. The cavity gain is strictly $\Delta W = q\int E_z(z)\cos(\omega t(z) + \varphi_0)\,\dd z$, with $T$ the transit-time factor; Laplace's equation $\nabla\cdot\boldsymbol E = 0$ gives near the axis $E_r = -\tfrac r2\,\partial E_z/\partial z$, which is RF defocusing. $K$ appears in the KV envelope equation $\sigma'' + k_0^2\sigma - K/(4\sigma) - \varepsilon^2/\sigma^3 = 0$ (here $\sigma$ is the rms size), see the space-charge chapter.

## Going deeper: the phase advance per period and the 90° rule {#tutorial-deep-periods}

### Step one: how the phase advance is computed

Take one period as a whole: it turns the $(x, x')$ at its entrance into the $(x, x')$ at its exit and can be written as a 2×2 "conversion table" (a transfer matrix, see [Drift](#drift-deep-matrix)) $M = \begin{pmatrix} a & b \\ c & d \end{pmatrix}$. The [Quadrupole](#quadrupole-deep-fodo) chapter shows that when the same period is repeated, the particles swing back and forth stably if $|\tfrac12(a + d)| < 1$, and the phase advance $\sigma$ per period satisfies

$$
\cos\sigma = \tfrac12\,(a + d).
$$

A solenoid mixes $x$ and $y$, so transversely the computation is done in a "frame that turns along with the particles" (the Larmor rotating frame), where $x$ and $y$ are independent again. Longitudinally there is likewise a 2×2 table giving the longitudinal phase advance.

### Step two: the numbers for the defaults

Estimated from the period transfer matrices of the linear preview ($K_b = 0.30$, $K_e = 0.8$, $\varphi_s = -30^\circ$): the first period has a transverse phase advance (in the Larmor rotating frame, $x$ and $y$ planes) of about $74^\circ$–$77^\circ$ and a longitudinal one of about $86^\circ$; by the 3rd period they have dropped to about $58^\circ$–$62^\circ$ and $78^\circ$, by the 5th to about $48^\circ$–$51^\circ$ and $67^\circ$, and the transverse and longitudinal values do not come close to each other.

With the step-3 settings $K_b = 0.35$, $K_e = 1.36$ the first period has about $90^\circ$–$94^\circ$ and $128^\circ$, and the 3rd still $95^\circ$ longitudinally. For a weak 0.18 mA beam the linear preview shows no problem (single particles only need a phase advance below $180^\circ$), but the non-linear longitudinal motion already makes the engine lose particles on a long line.

### Step three: why the phase advance falls from period to period

$B\rho$ grows with energy, so the same solenoid bends the particles less; longitudinally, how hard the bunch is to "squeeze" grows as $\beta^3\gamma^3$. The same solenoid and cavity thus become "weaker" period by period. This is exactly why a real design lets $K_b$ and $K_e$ change with energy (the beam line of the [superconducting section](#journey-sc) chapter raises them period by period).

### Step four: Kb too weak and too strong

- **$K_b = 0.2$**: the solenoid alone gives a transverse phase advance of about $57^\circ$. In the "smooth approximation", RF defocusing subtracts half the square of the longitudinal phase advance from the square of the transverse one: $\sigma_t^2 \approx \sigma_{0t}^2 - \sigma_{0l}^2/2$. With $86^\circ$ longitudinally the subtracted amount corresponds to $86^\circ/\sqrt2 \approx 61^\circ$, more than the solenoid can provide. The full period matrix leaves a transverse phase advance of only about $11^\circ$–$25^\circ$, almost no net focusing, so rms $x$ grows to about 8.4 mm.
- **$K_b = 0.45$**: the solenoid is too strong; in the first period $|\cos\sigma| > 1$, beyond the limit of periodic stability (the condition of step one is violated), and rms $x$ grows to about 5.3 mm; at $K_b = 0.5$ beyond 20 mm.

### For readers who know linear algebra

The eigenvalues of the period matrix are $e^{\pm i\sigma}$ and $\operatorname{tr}M = 2\cos\sigma$; for $|\operatorname{tr}M| > 2$ the eigenvalues are real, one of them has modulus above 1, and the amplitude grows geometrically from period to period. An accelerating period is not strictly symplectic and must first be normalized by $\sqrt{\beta\gamma_{\text{out}}/\beta\gamma_{\text{in}}}$ (see the note on symplecticity in [In AVAS](#tutorial-in-avas)). The smooth approximation $\sigma_t^2 = \sigma_{0t}^2 - \sigma_{0l}^2/2$ comes from averaging the RF defocusing over a period into a uniform negative focusing.

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

### Period transfer matrix symplecticity in the linear preview

In step 4 the period starts and ends in field-free drifts, so with the cavity's damping divided out the period transfer matrix should be symplectic, with all eigenvalues of modulus 1. The linear preview gives moduli between 0.9996 and 1.0004 (exactly 1 at $K_e = 0$); the small remainder comes from the cavity, since with acceleration $(x, x')$ are not canonical coordinates. If the solenoid's radial fringe field were taken directly from the `sol` map's own $B_x$, $B_y$, which disagree point by point with $-\tfrac12\,\partial B_z/\partial z$ (the map's transverse field is lower and broader than the slope of its on-axis field suggests, see [solenoid](#solenoid-in-avas)), the eigenvalue moduli would be 0.95 and 1.05, and on a long line one mode would grow by about 4 % per period. In the linear preview the radial fringe comes from on-axis $B_z$ by Maxwell's equations, preserving symplecticity.

## Further reading {#tutorial-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: superconducting linacs, solenoid focusing and RF defocusing.
- This primer's [lattice design](#lattice-design), [matching](#matching) and [multi-particle simulation essentials](#simulation); the complete list is in [references](#references).
