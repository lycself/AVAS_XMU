# Orbit correction

In the previous chapters every magnet sat exactly where it should, and the centre of the beam flew exactly along the centre line of the pipe. A real accelerator cannot do that: a quadrupole placed 0.1 mm off is quite common, the ground settles slowly with the seasons, and the magnet power supplies wobble a little. Each such small error pushes the centre of the beam off the centre line.

How do the operators of an accelerator know how far off the beam is, and how do they bring it back? This is **orbit correction**, the most basic task of accelerator control and one that is done every day. This chapter starts from scratch: first how errors push the beam off, then how to measure and how to correct, and finally why "computing the exact solution" actually does harm.

First, the animation. This is a FODO channel of 6 periods (from the previous chapter) whose 12 quadrupoles are all placed a little off at random (about 0.4 mm on average). The solid line is the beam centre and the orange dots are particles. Grey diamonds are the detectors that measure the beam position (BPMs), teal vertical lines the small magnets used to correct it (steerers).

```avas-motion
orbit
```

"Singular values used" is now 0, so nothing has been corrected yet: the beam centre snakes left and right, almost 2 mm off the centre line at most. Move this slider step by step to 2, 4, 6 and watch the solid line straighten; the sections before "going deeper" explain what the slider means.

## What is an "orbit"? {#orbit-correction-orbit}

A beam has thousands of particles, but it has a **centre**: the average position of all particles. The path the beam centre takes through the accelerator is called the **orbit**. Ideally the orbit is the centre line of the pipe; with errors it wanders off it.

Why is an off-centre orbit bad?

- **Hitting the wall**: the beam itself is several millimetres wide; move its centre a few millimetres more and the particles at its edge hit the pipe. Lost particles make the equipment radioactive and, in bad cases, damage it.
- **Getting "dirty"**: off centre the beam meets the less ideal parts of the magnets' fields (the farther from the centre, the less the field looks like a perfect quadrupole), its emittance grows and the beam becomes more "disordered".
- **Missing the target**: the beam finally has to hit a target or enter the next accelerator; slightly off, and the experiment suffers.

Accelerators therefore usually keep the orbit within a few tenths of a millimetre.

## Where the errors come from: misplaced quadrupoles {#orbit-correction-errors}

Recall the previous chapter: the field of a quadrupole is zero at its centre and grows with the distance, and a particle is turned back by the angle "distance from the centre ÷ focal length".

Now picture a quadrupole placed a distance $d$ too far to the right. For a beam flying exactly along the centre line, the magnet's centre is $d$ to its right, i.e. the beam is $d$ **to the left** of the magnet's centre, and the magnet pushes it towards its own centre by an angle

$$
\theta = \frac{d}{f}.
$$

So a misplaced quadrupole, besides its normal focusing, "kicks" the whole beam. The quadrupoles of the animation have a focal length of about 0.3 m; placed 0.4 mm off, they kick by about 1.3 mrad. One metre later the beam centre is 1.3 mm off. Twelve quadrupoles, each kicking at random, add up to the snaking orbit of the animation.

In the animation, changing the "error set" is like taking another accelerator with differently misplaced magnets; "quadrupole offsets" sets how far they are off. The orbit is proportional to the size of the offsets: double them and the orbit doubles too.

## Measuring: beam position monitors {#orbit-correction-bpm}

The beam is inside a vacuum pipe; nobody can see or touch it. How do we know where it is?

A **beam position monitor** (BPM) is four small electrodes in the pipe wall, top, bottom, left and right. A bunch of charged particles flying past induces an electric signal on the electrodes; the closer the beam is to an electrode, the stronger its signal. Comparing the left and right signals gives the horizontal position of the beam, comparing top and bottom gives the vertical one.

A BPM measures only the position of the **beam centre**, and only where it is installed. In the animation there is a BPM after every quadrupole (grey diamonds), 12 in all, each measuring both $x$ and $y$.

BPM readings are not perfectly exact either: electronic noise, cables and temperature all add errors; good BPMs reach a few to a few tens of micrometres. The animation's "BPM noise" is the size of this error.

## Correcting: steerers {#orbit-correction-steerer}

The correction is done with **steerers**: small magnets with a uniform transverse field. A uniform field pushes a particle by the same fixed angle wherever it is (unlike a quadrupole, whose push depends on the position), so it is just right for "kicking back".

The deflection of a steerer follows from the previous chapter: with field $B$, length $L$ and magnetic rigidity $B\rho$, the angle is $\theta = B L/B\rho$. In the animation each steerer acts over a 0.1 m quadrupole (real machines often wind the steering coils onto the quadrupoles), and 1 mT deflects the example protons by about 0.56 mrad. Each steerer of the animation can give at most 5 mT; beyond that it is capped. Details are in the [Steerer](#steerer) chapter.

## The response matrix: which knob moves which reading {#orbit-correction-response}

We now have 12 BPM readings and 12 adjustable steerers (per direction). How strong should each steerer be?

First an experiment: raise steerer 1 by 1 mT, leave all the others, and note how much each BPM reading changes; set it back and do the same with steerer 2, and so on. After 12 experiments we have a 12 × 12 table. The number in row $i$, column $j$ is "how much BPM $i$ changes per mT of steerer $j$". This table is the **response matrix**.

For the first two BPMs and the first two steerers (horizontal), the table reads:

| | steerer 1, per mT | steerer 2, per mT |
|---|---|---|
| BPM 1 | −0.054 mm | 0 |
| BPM 2 | −0.302 mm | −0.058 mm |

There is a 0: steerer 2 is downstream of BPM 1; the beam passes BPM 1 before it reaches steerer 2, and a magnet further on cannot change an earlier reading. The whole 12 × 12 table is 0 above its diagonal.

The good news is that orbit changes are **linear**: when two steerers change together, the readings change by the sum of the two separate changes. So "bring all BPM readings back to 0" becomes a set of linear equations. If BPM 1 and BPM 2 now read $y_1$ and $y_2$, we look for steerer changes $u_1$ and $u_2$ with

$$
\begin{aligned}
-0.054\,u_1 &= -y_1,\\
-0.302\,u_1 - 0.058\,u_2 &= -y_2.
\end{aligned}
$$

These are school simultaneous equations: solve the first for $u_1$, put it into the second and solve for $u_2$. Twelve BPMs and twelve steerers make twelve equations in twelve unknowns; the idea is exactly the same, and a computer solves them.

## Noise and singular values: don't chase the noise {#orbit-correction-svd}

Solve the twelve equations exactly and the orbit is corrected? Pull the slider straight to 12 in the animation: the steerers all hit their limit (a warning appears), and the corrected orbit is worse than no correction, up to about 3 mm off. What went wrong?

The trouble is **noise** and **invisible orbits**. Some combinations of the 12 steerers hardly change any BPM reading: for example two neighbouring steerers kicking left and right, so that the beam bulges out between two BPMs and is back in place at the next BPM. The BPMs can hardly "see" such an orbit. When the equations are solved exactly, even a few micrometres of noise in the readings make them call for such a combination to "correct" it, and because it changes the readings so little, it needs very large fields. The steerers end up following the noise and create large offsets between the BPMs.

The remedy is to line up all combinations of steerers by "how much they change the BPM readings", from large to small, use only those with a large effect and give up the ones with too small an effect. The size of each combination's effect is called a **singular value**, and the mathematical method of lining them up is the **singular value decomposition** (SVD). The animation's slider means "use the first few combinations". The plot on the right below shows the 12 singular values: the first 6 are fairly large, then they get smaller and smaller, and the last one is almost zero (that is the invisible orbit).

Watch the animation (BPM noise 10 µm, default error set):

| Singular values used | rms orbit $x$ at the BPMs | largest offset $x$ along the line | strongest steerer |
|---|---|---|---|
| 0 (no correction) | 1.05 mm | 1.90 mm | 0 |
| 2 | 0.26 mm | 0.59 mm | 2.3 mT |
| 4 | 0.073 mm | 0.42 mm | 2.8 mT |
| 6 | 0.028 mm | 0.23 mm | 3.2 mT |
| 11 | 0.008 mm | 0.17 mm | 2.9 mT |
| 12 | 1.24 mm | 2.97 mm | 5 mT (capped) |

Raise the noise to 50 µm: with 6 singular values only about 0.03 mm is left at the BPMs, and using more does not help, because what is left is about as large as the noise; correcting further just chases the noise.

One more thing to notice: the offsets at the BPMs can be made very small, but **between** the BPMs about 0.2 mm remains, because a BPM sees the orbit only where it is installed.

## Orbit correction in practice {#orbit-correction-practice}

Orbit correction in a real accelerator is much like the animation, with a few differences:

- **The response matrix is measured.** Models always have errors, so operators really change the steerers one by one and measure the response matrix, or use measurements to correct the model.
- **Iterate when once is not enough.** Each step corrects only part of the orbit (times a factor below 1), then the orbit is measured and corrected again, until it stops improving.
- **The BPMs may be misplaced too.** The "zero" of a BPM and the magnetic centre of the quadrupole need not coincide; dedicated methods (beam-based alignment) find the difference.
- **Correction never stops.** Temperature, ground and power supplies drift slowly, so large accelerators correct the orbit automatically every few seconds or minutes; this is **slow feedback**, treated in the feedback chapter later.

## Think about it {#orbit-correction-think}

> **Think about it 1**: why is the response matrix all zeros above its diagonal?
>
> Answer: a steerer can only affect BPMs downstream of it. The beam passes the upstream BPM before it meets the downstream steerer, and what happens later cannot change an earlier reading.

> **Think about it 2**: double the gradient of a quadrupole. What happens to the "kick" its misplacement causes?
>
> Answer: the focal length halves, so the same offset kicks by twice the angle $d/f$. The stronger a channel focuses, the more sensitive it is to misalignment.

> **Think about it 3**: all BPMs read 0. Does that mean the orbit is on the centre line everywhere?
>
> Answer: no. A BPM measures only where it is installed; between two BPMs the orbit can bulge out unseen. With 11 singular values in the animation only 0.008 mm is left at the BPMs, yet the largest offset along the line is 0.17 mm.

## Common misconceptions {#orbit-correction-misconceptions}

- **"Using all singular values corrects best"**: the smallest singular values belong to orbits the BPMs can hardly see; using them amplifies the noise, the steerers may hit their limits, and the orbit gets worse.
- **"Zero at the BPMs means a perfect orbit"**: there can still be offsets between the BPMs, and the zero of a BPM itself can be off.
- **"The more steerers, the better"**: the number and positions of steerers and BPMs have to match. Where no BPM is watching, nobody notices the offsets a steerer creates.
- **"The correction computed by the linear model is the final answer"**: the response matrix of a real machine has to be measured or calibrated; AVAS's linear model is exact for steerers, but through noisy field maps it differs from the engine by a few per cent (see "In AVAS" below).

## Going deeper: response matrix and singular value decomposition {#orbit-correction-deep-svd}

The main text explained the response matrix with simultaneous equations and singular values with "lining up". This section turns both into exact mathematics step by step: the first five steps use school algebra and the least-squares method from school statistics; the last paragraph is for readers who know linear algebra.

### Step 1: why a misplaced quadrupole only "kicks" the beam centre

Inside a quadrupole a particle's position relative to the magnetic centre is $x - d$ ($d$ the magnet's offset). As a thin lens, the change of angle at the exit is

$$
\Delta x' = -\frac{x - d}{f} = -\frac{x}{f} + \frac{d}{f}.
$$

The first term is the normal focusing, acting on each particle according to its own $x$; the second, $d/f$, is the same for all particles: a constant "kick". A misplacement therefore pushes the whole beam off together (it moves the beam centre) and leaves the beam's shape and size alone. In AVAS's linear model every step is $\boldsymbol x \to M\boldsymbol x + \boldsymbol d$: the matrix $M$ does the focusing, and the constant $\boldsymbol d$ comes exactly from steerers and element offsets.

### Step 2: how far one kick moves the beam at a downstream BPM

Give the beam an angle $\theta$ at $s_j$. How far off is it at a BPM at $s_i$ downstream? With only drifts in between, the answer is "angle × distance": $\theta\,(s_i - s_j)$. With quadrupoles in between, the "distance" becomes the top-right number $M_{12}$ of the transfer matrix from $s_j$ to $s_i$ (for a drift $M_{12}$ is exactly its length $L$); it is the "conversion factor from angle to position":

$$
\Delta y_i = M_{12}(s_j \to s_i)\,\theta_j .
$$

A steerer field $u_j$ gives $\theta_j = -u_j L_j/B\rho$ (for the direction convention of a magnetic steerer's By see [Steerer](#steerer)). One element of the response matrix is therefore

$$
R_{ij} = \frac{\Delta y_i}{\Delta u_j} \approx M_{12}(s_j \to s_i)\,\frac{\theta_j}{u_j}\quad (s_j < s_i),
$$

with "≈" because the steerer's field is spread over the whole next element rather than concentrated at a point. For a BPM upstream of the steerer ($s_i < s_j$) $R_{ij} = 0$: the zeros above the diagonal of the main text.

### Step 3: all BPMs together: a set of linear equations

Put the $m$ BPM readings in a column $\boldsymbol y$ and the fields of the $n$ steerers in a column $\boldsymbol u$. Since every step is "matrix times position plus a constant", readings and steerers are related linearly:

$$
\boldsymbol y = R\,\boldsymbol u + \boldsymbol y_0 ,
$$

with $\boldsymbol y_0$ the orbit caused by the errors. Correcting means finding $\Delta\boldsymbol u$ with $R\,\Delta\boldsymbol u = -\boldsymbol y$. For steerers and offsets $R$ does not depend on the operating point (the model is linear), so it can be measured once and reused; for other knobs such as gradients $R$ is only the rate of change near the operating point.

### Step 4: when the equations cannot be solved exactly: least squares

The numbers of BPMs and steerers often differ, and the readings carry noise, so the equations generally have no exact solution. The remedy is the same as for the regression line of school statistics: instead of demanding that every equation hold exactly, minimise "the sum of the squares of what is left at each BPM",

$$
\text{minimise } \sum_i \left(y_i + \sum_j R_{ij}\,\Delta u_j\right)^2 .
$$

This is the **least-squares method**. A regression line fits many points with two unknowns (slope and intercept); here twelve unknowns (the steerers) cancel twelve readings; the idea is the same.

### Step 5: singular values intuitively: two knobs doing almost the same thing

Why does solving exactly amplify noise? Take a small example with two steerers and two BPMs and a response matrix (in mm/mT)

$$
R = \begin{pmatrix} 1.0 & 0.9 \\ 0.9 & 1.0 \end{pmatrix}.
$$

The two steerers act very similarly. Raise **both** by 1 mT (the "same direction" combination) and each BPM changes by 1.9 mm, a large effect; raise one by +1 and the other by −1 (the "opposite" combination) and the BPMs change by only ±0.1 mm, hardly noticeable. The "gains" 1.9 and 0.1 of these two combinations are the **singular values** of the matrix.

Now suppose the readings carry 0.01 mm of noise that happens to look like "one +, one −". An exact solution concludes that the "opposite" combination must correct it, and since its gain is only 0.1, it asks for changes of about 0.1 mT on each steerer: the noise is amplified ten times. In the channel with 12 steerers the smallest singular value is tens of thousands of times smaller than the largest, so the noise is amplified tens of thousands of times; that is why the steerers hit their limit in the main text when all singular values were used.

The **singular value decomposition** splits any response matrix into such a set of "non-interfering combinations", each with its own gain (singular value), sorted from large to small. Correcting with only the combinations of large gain keeps the noise from being amplified, at the price of leaving the offsets that only the small-gain combinations could fix.

### Step 6: the truncation formula

Keep $k$ singular values $\sigma_1 \ge \sigma_2 \ge \dots \ge \sigma_k$; let the $i$-th combination be a set of steerer changes $\boldsymbol v_i$ and the reading pattern it produces at the BPMs $\boldsymbol u_i$ (both taken as vectors of length 1). The correction is "project the readings onto each pattern, divide by the gain, turn it into the steerer combination":

$$
\Delta\boldsymbol u = -\sum_{i=1}^{k} \frac{\boldsymbol v_i\,(\boldsymbol u_i\tp \boldsymbol y)}{\sigma_i},
$$

where $\boldsymbol u_i\tp \boldsymbol y = \sum_m (\boldsymbol u_i)_m\,y_m$ is "the component of the readings along pattern $i$". With noise of rms $\sigma_n$ on each BPM, its component along each pattern is also of size $\sigma_n$, and it changes the steerers by $\sigma_n/\sigma_i$: the smaller the singular value, the more the noise is amplified.

For the channel of the animation $\sigma_1/\sigma_{11} \approx 86$, while $\sigma_{12}$ is more than four orders of magnitude below $\sigma_{11}$ (the ratio of the largest to the smallest singular value, the "condition number", is about $2\times 10^6$). Pattern 12 is the "bulge between BPMs" of the main text. The number $k$ trades the residual orbit against the noise amplification.

### Step 7: correcting again and again: feedback

Written as a repeated step $\boldsymbol u_{k+1} = \boldsymbol u_k - g\,R^{+}\boldsymbol y_k$ ($R^{+}$ the truncated solution of step 6, $0 < g \le 1$ the gain), each repetition multiplies the offset along every kept pattern by $(1 - g)$. $g = 1$ corrects in one go, $g = 0.5$ removes half each time and is safer. This is already a **feedback** system; with a measurement delay, too large a gain makes it oscillate and diverge; see the [Feedback](#feedback) chapter. AVAS's `avas/dynamics/feedback.py` simulates this kind of pulse-to-pulse feedback.

### For readers who know linear algebra

$R = U\Sigma V\tp$ with $U$, $V$ orthogonal and $\Sigma = \operatorname{diag}(\sigma_1 \ge \sigma_2 \ge \dots)$; step 6 is the minimum-norm least-squares solution of the truncated pseudo-inverse $R^{+}_k = V_k\Sigma_k^{-1}U_k\tp$. Tikhonov regularisation replaces $1/\sigma_i$ by $\sigma_i/(\sigma_i^2 + \lambda^2)$, a smooth version of the truncation. Causality makes $R$ lower triangular, so $\det R$ is the product of its diagonal; in the animation's channel the diagonal elements are all about 0.05 m/T, and twelve of them multiply to about $10^{-15}$, which is why the smallest singular value is so small. On each pattern the iteration of step 7 is $e_{k+1} = (1-g)\,e_k$; with a measurement delay of $d$ pulses the characteristic equation is $z^{d+1} - z^d + g = 0$, which needs $g < 2$ without delay and $g < 1$ with one pulse of delay.

## In AVAS {#orbit-correction-in-avas}

### Steerers and BPMs in the lattice

```avas-params
steerer diag_position
```

- A `steerer` must have length 0; it acts over the whole length of the **next element**. Parameters 4 and 5 are Bx and By (in T for a magnetic steerer), parameter 7 is the maximum. A magnetic By > 0 deflects positive particles towards −x, Bx > 0 towards +y (checked with the engine, see [Steerer](#steerer)).
- `diag_position` marks a position diagnostic (BPM) used by the correction of an error study.

### Research page: response and controllability

The "Response & controllability" tab of the Research page computes the response matrix, singular values and controllability with the linear model, and the "Feedback" tab simulates pulse-to-pulse feedback. Monitors and knobs are stored in `<project>/Research/diagnostics.json` and default to the lattice's `diag_*` lines and steerers. The linear calculations take no run lock and write no input files; the response matrix can also be computed with the engine (as a study run, which excludes ordinary runs meanwhile).

### Linear model and engine

The orbit response matrix has been checked against the engine (`tools/compare_response_engine.py`, 200 macro-particles, emittance reduced 1000 times, fixed `randomseed`): elements passing only drifts and the HWR cavity agree with the engine to within 0.06 % of the largest element; elements passing through the example `sol` field map differ by 0.1–2.7 %, because of the inconsistent transverse fields of the map itself. Fix the random seed when comparing engine results; otherwise every run draws new particles, and the mean position of 200 particles fluctuates by about 3.5 µm, which shows up as a spurious "response" even at BPMs upstream of the kick. With the original kernel or the new kernel's compatibility mode (compat = dll), a non-zero seed makes $x$ fully correlated with the phase in the initial distribution (see [Simulation](#simulation-in-avas)); this hardly affects centroid responses, and the new kernel's default mode does not have the defect.

### In Python

```text
from avas.model import LinearModel
from avas.dynamics.diagnostics import Diagnostics
from avas.dynamics.response import response, steerers

model = LinearModel.from_project("my_project")
diag = Diagnostics.from_lattice(model)          # the lattice's diag_position / diag_size
r = response(model, steerers(model), diag)      # r.matrix: derivatives of the readings
u, s, vt = r.svd()
```

The channel, offsets and SVD correction of this chapter's animation are in `orbit` of `avas/learn/motion.py`, a starting point for scripts.

## Further reading {#orbit-correction-reading}

- M. G. Minty, F. Zimmermann, *Measurement and Control of Charged Particle Beams*, Springer (2003): the chapters on orbit measurement and correction.
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer (2015): orbit distortion and correction.
- A. W. Chao et al. (eds.), *Handbook of Accelerator Physics and Engineering*, 2nd ed., World Scientific (2013): the sections on orbit correction and beam diagnostics.
- Related chapters: [Quadrupole](#quadrupole), [Steerer](#steerer), [Errors and correction](#errors); full bibliography in [References](#references).
