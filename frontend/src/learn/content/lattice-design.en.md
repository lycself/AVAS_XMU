# Designing a lattice: periods, phase advance and aperture

The previous chapters introduced drifts, quadrupoles, solenoids and RF cavities one by one. But an accelerator is not a random pile of elements: what goes where, how far apart, and how strong each one is must all be drawn up in advance. This drawing is called the **lattice** (the word means "grid": elements arranged in a regular pattern).

Suppose you are asked to draw the first part of a superconducting section: how far apart the solenoids go, how strong they are, how hard the cavities are driven, which synchronous phase they run at, how wide the beam pipe is. That looks like many numbers, but they follow from only a few choices: the **period length**, the **phase advance per period** (one transverse, one longitudinal), the **synchronous phase**, and the **beam size and aperture** that result from them. This chapter shows how those choices are made and why, how one section is joined to the next, and finally how the result is written as an AVAS lattice file.

First look at the simplest lattice: one group of "focusing quadrupole, drift, defocusing quadrupole, drift" repeated 6 times (the FODO channel of the [quadrupole](#quadrupole-fodo) chapter), with a gradient of 5 T/m.

```avas-motion
fodo G=5
```

Watch the two blue particles: they swing back and forth about the axis, and in 6 periods they complete about one swing (the readout "oscillations" on the right is 1.01). Now look at the width of the beam: it has the same shape in every period and is widest horizontally in the focusing quadrupoles. The first question of this chapter is how such a quantity, "how far a particle swings per period", decides the fate of the beam.

Why a linac is divided into an ion source, an RFQ, an MEBT, a superconducting section and so on, and which problem each section solves, is told in the first part, [From the ion source to the target](#journey-overview), and in the chapters on the [MEBT](#journey-mebt), the [superconducting section](#journey-sc) and the [HEBT](#journey-hebt); this chapter does not repeat it and discusses only how the lattice **inside one section** is designed. It uses ideas from [linear optics](#linear-optics), [longitudinal dynamics](#longitudinal) and [space charge](#space-charge), each explained again where it first appears. Notation: transverse coordinates $(x, x', y, y')$ (positions and angles), longitudinal $(z, \delta = \Delta p/p)$ (position offset and relative momentum offset), period length $L_p$, phase advance per period $\sigma$ (in degrees), zero-current phase advance $\sigma_0$ (the phase advance without space charge). The [hands-on tutorial](#tutorial) is the practical version of this chapter.

## What designing a lattice has to answer {#lattice-design-overview}

AVAS tracks the beam from the initial beam given in `beam.txt`: described by Twiss parameters (a few numbers giving the size and shape of the beam, see [phase space](#phase-space)), or read from a particle file (for example the example's `part_rfq.dst`, the distribution at the RFQ exit). It therefore describes the lattice **after a given input beam**, which suits the MEBT, the superconducting accelerating sections and the HEBT. AVAS does not simulate the ion source, the RFQ or the accelerating structures of a normal-conducting DTL; the beam they produce is handed to AVAS as `beam.txt` or a particle file. With 3D field maps of these structures they could in principle be read as `field` elements, which this chapter does not discuss.

All examples of this chapter use the beam of the example project `examples/hwr010`, which is exactly the start of a superconducting section: protons, kinetic energy $W = 1.527\ \mathrm{MeV}$, RF frequency $f = 162.5\ \mathrm{MHz}$, current $I = 0.18\ \mathrm{mA}$. For this beam

$$
\gamma = 1.00163,\quad \beta = 0.05698,\quad \beta\gamma = 0.05708,\quad B\rho = 0.1786\ \mathrm{T\,m},\quad \beta\lambda = 0.1051\ \mathrm{m}
$$

$\beta$ is the speed as a fraction of the speed of light (about 6 %), and $\gamma$ is the relativistic factor (close to 1: the proton is still "slow"). $\beta\lambda$ is the distance a particle travels in one RF period and sets the scale of accelerating gaps and cavities; $B\rho$ is the magnetic rigidity, how hard the particle is to deflect with a magnetic field, and sets how much a given field bends or focuses the beam (see [kinematics](#kinematics)).

Designing the lattice of a section really means answering four questions in turn:

1. **Period structure**: what goes into a period (FODO, doublet, solenoid plus cavity …) and how long it is;
2. **Phase advance per period**: how large the transverse $\sigma_{0t}$ and the longitudinal $\sigma_{0l}$ are;
3. **Acceleration**: how the cavity voltage and synchronous phase vary along the line;
4. **Margins**: what aperture goes with the beam size that results from the first three.

On top of that, sections must be joined. The following sections take these questions in this order.

## Why periodic structures {#lattice-design-periods}

### A pendulum pushed back again and again

Think of one particle's transverse motion first. Without any focusing, a particle with an angle drifts away from the axis along a straight line (see [drift](#drift)). Put a lens every so often to push it back, and the particle swings about the axis, like a pendulum that is given a push at regular intervals. Push too gently and the particle wanders too far; push too hard and it overshoots, swinging ever wider. Only when the strength of the push and the interval between pushes fit together does the swing stay bounded for ever.

Repeating the same group of elements over and over is called a **periodic structure**, and the repeated group is one **period** (also called a cell). The advantage of a periodic structure is that once one period is designed and checked, the behaviour of the whole section is known.

### Phase advance: the angle swung through per period

A particle's swing, like a mass on a spring, can be tracked by an "angle" telling where in the swing it is: one full swing back and forth is $360^\circ$. The angle by which the swing advances while the particle passes one period is the **phase advance per period** $\sigma$. The period in the opening animation has a phase advance of $61.4^\circ$, so a particle completes one swing every $360/61.4 \approx 5.9$ periods, about once in 6 periods.

The phase advance is the most important number in lattice design. It merges two things, "how hard the push" and "how far apart", into one quantity: a small phase advance means gentle pushes or close spacing; a large one means hard pushes.

### The condition for stability

How much overshoot makes the swing run away? The answer is surprisingly simple. Write the effect of one period on a particle's position and angle as a 2×2 conversion table $M$ (the **transfer matrix**, see [drift](#drift-deep-matrix)), and look only at the mean of the two numbers on its diagonal, $\tfrac12(M_{11} + M_{22})$, "half the trace" $\tfrac12\Tr M$:

$$
\cos\sigma = \tfrac{1}{2}\Tr M,\qquad \left\lvert \tfrac{1}{2}\Tr M \right\rvert < 1
$$

While this number lies between $-1$ and $1$, the particle swings about the axis for ever, and the number is the cosine of the phase advance; outside this range the particle moves farther from the axis by a fixed factor every period, snowballing away. Why the condition is exactly this is shown step by step in the "going deeper" part with a school quadratic equation.

The FODO example at the start has $\tfrac12\Tr M = 0.478$ and a phase advance $\arccos 0.478 = 61.4^\circ$. Raise the gradient $G$ in the animation to about $9.8\ \mathrm{T/m}$ and the number passes $-1$: the period is no longer stable, the animation warns, and the particles soon hit the pipe (red crosses).

A periodic structure has a second advantage: there is exactly one beam shape (one set of periodic Twiss parameters $(\alpha, \beta)$) for which the beam returns to its shape after every period. A beam entering with these parameters is the **matched beam** (see [matching](#matching)), and its width repeats with the period, as in the animation. Without periodicity there is no such "natural" shape, and every stretch has to be designed on its own.

### How wide the beam is: the smooth approximation

The phase advance also decides directly how wide the beam is. Write the phase advance per metre as

$$
k_0 = \frac{\sigma}{L_p}
$$

($\sigma$ in radians), the **phase advance per metre**. If we do not care about the small ripples of the beam within one period, we can think of the channel as one uniform "spring", and the particle performs a simple harmonic oscillation that turns through $k_0$ radians per metre. From school: the maximum speed of a simple harmonic oscillation is the angular frequency times the amplitude; here "speed" becomes the angle $x'$, so a particle of amplitude $A$ has a maximum angle $k_0 A$. The spread of positions and the spread of angles in the beam are therefore proportional, and their product is the fixed emittance $\varepsilon$ (the area the beam occupies in phase space, unchanged as the beam travels). This gives

$$
\bar\beta \approx \frac{L_p}{\sigma} = \frac{1}{k_0},\qquad x_{\mathrm{rms}} \approx \sqrt{\frac{\varepsilon}{k_0}}
$$

Here $\varepsilon$ is the rms geometric emittance and $\bar\beta$ is the mean of the β function (the function describing the beam size along the line, $x_{\mathrm{rms}} = \sqrt{\beta\varepsilon}$). This is the **smooth approximation**, the most useful estimate in design: the beam size depends only on the emittance and the phase advance per metre, not on the details of the focusing elements. Doubling the phase advance per metre shrinks the beam by $1/\sqrt2$.

Example: the FODO at the start has $\sigma_0 = 61.4^\circ = 1.072\ \mathrm{rad}$ and $L_p = 0.8\ \mathrm{m}$, so $k_0 = 1.34\ \mathrm{m^{-1}}$ and $\bar\beta = 0.746\ \mathrm{m}$. The geometric emittance of the example beam is $\varepsilon = 0.1377/0.05708 = 2.41\ \mathrm{mm\,mrad}$, giving the estimate $x_{\mathrm{rms}} \approx 1.34\ \mathrm{mm}$; the exact periodic solution is $1.04\ \mathrm{mm}$ at the centre of the D quadrupole and $1.80\ \mathrm{mm}$ at the centre of the F quadrupole, and the smooth approximation gives just the size in between.

### FODO

A FODO is a period in which focusing (F) and defocusing (D) quadrupoles alternate, with drifts (O) or cavities in between. Treating the quadrupoles as thin lenses of focal length $\pm f$ spaced $L_p/2$ apart, the matrix multiplication of the "going deeper" part gives

$$
\sin\frac{\sigma}{2} = \frac{L_p}{4f},\qquad
\beta_{\max,\min} = \frac{L_p\,(1 \pm \sin\frac{\sigma}{2})}{\sin\sigma}
$$

The sine on the left cannot exceed 1, so stability requires $f > L_p/4$: the lenses must not be too strong. The advantages of FODO are its symmetry between the planes and few elements per period; the drawback is that $\beta_{\max}/\beta_{\min}$ grows with the phase advance, and the beam is largest in the F quadrupoles.

Example: example beam, quadrupoles of length $L_q = 0.1\ \mathrm{m}$ and gradient $G = 5\ \mathrm{T/m}$, drifts $L_d = 0.3\ \mathrm{m}$ (period $L_p = 0.8\ \mathrm{m}$). The focusing strength is $k = G/B\rho = 27.99\ \mathrm{m^{-2}}$, the thin-lens focal length $f = 1/(kL_q) = 0.357\ \mathrm{m}$. The thin-lens formula gives $\sigma = 68.1^\circ$; the thick-lens matrix (which includes the length of the quadrupole, see [quadrupole](#quadrupole-deep-matrix)) gives the exact $\sigma = 61.4^\circ$, a periodic $\beta_x = 1.35\ \mathrm{m}$ at the centre of the F quadrupole and $\beta_x = 0.45\ \mathrm{m}$ at the centre of the D quadrupole. The thin-lens approximation overestimates the phase advance when $kL_q^2$ is not small (here $kL_q^2 = 0.28$); design with the thick matrix.

> **Think**: each quadrupole of a FODO defocuses in one plane, so why is the net effect focusing in both planes?
>
> Answer: because the beam is large in the F quadrupole and small in the D quadrupole. A lens deflects a particle in proportion to its distance from the axis; in the example the x plane has $\beta = 1.35\ \mathrm{m}$ at F and $0.45\ \mathrm{m}$ at D, beam sizes differing by $\sqrt{3}$, so the focusing push wins over the defocusing one. In the y plane the roles are swapped. Two thin lenses of equal strength and opposite sign a distance $d$ apart have a combined focal length $1/f_{\mathrm{tot}} = d/f^2 > 0$: net focusing is a second-order effect, which is why FODO needs much stronger gradients than a single lens.

In the lab below change `G`: by the thick matrix, the phase advance passes $90^\circ$ at `G` about $6.9\ \mathrm{T/m}$; at about $9.8\ \mathrm{T/m}$ $\lvert\Tr M\rvert/2$ reaches 1 (phase advance $180^\circ$), the periodic β function diverges, and beyond that no periodic solution exists. Compare the thick-matrix phase advance with the thin-lens estimate as you go.

```avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
```

### Doublets and triplets

Putting an F and a D quadrupole close together (FD + long drift) leaves a long space for cavities; this is a doublet period. Its long drift is used efficiently, but the β functions of the two planes are not symmetric; a triplet (FDF or DFD) restores the symmetry. Their stability regions differ from FODO's, so $\Tr M$ must be computed from the actual matrices.

### Solenoid periods

Low-energy superconducting sections often use periods of solenoids and cavities (S–C, S–C–C and so on, S for solenoid, C for cavity). A solenoid focuses both planes with its field along the axis; in the frame rotating at the Larmor angle (a frame that turns about the axis together with the particles) its thin-lens focal length is

$$
\frac{1}{f} = \int k^2\,\dd s,\qquad k = \frac{B_z}{2B\rho}
$$

($\int k^2\,\dd s$ adds up $k^2$ along the solenoid, see [solenoid](#solenoid)). For one thin solenoid plus drift with period $L_p$, $\cos\sigma = 1 - L_p/(2f)$.

A common statement is: "the focusing strength $1/f$ of a solenoid scales as $1/(B\rho)^2$, that of a single quadrupole only as $1/B\rho$, so solenoids pay off at low energy (small $B\rho$) and quadrupoles at high energy." For the focal length of a single lens that is true, but it easily suggests that the phase advance of the two kinds of period scales differently with energy. In fact the net focusing of a FODO is second order (see the "Think" above), and the phase advance of both kinds of period scales roughly as "field $/B\rho$". The real difference is **how strong a field is needed**. Compare at the same period length of 0.8 m: for $\sigma = 61.4^\circ$ the FODO above needs only $5\ \mathrm{T/m}$, a pole-tip field of $0.1\ \mathrm{T}$ at 2 cm aperture, whereas a 0.2 m long solenoid needs $B \approx 0.93\ \mathrm{T}$ (thick-lens matrix), about 9 times more. Both required fields grow linearly with $B\rho$: at low energy a superconducting solenoid of a few tesla is quite sufficient, a single element focuses both planes, gives a round beam and is compact inside a cryostat; at higher energy the field a solenoid would need exceeds what is practical, and quadrupoles become the sensible choice.

> **Note**: a solenoid rotates the beam about the axis by $\int k\,\dd s$. A round beam is unchanged by the rotation, but a non-round beam or one with transverse errors develops x–y coupling (the motions in the two directions mix); the 2D/3D views and DataSet of AVAS give laboratory-frame quantities.

## How large a phase advance per period {#lattice-design-phase-advance}

### First look at the beam size

The larger the phase advance, the stronger the focusing; does the beam keep getting thinner? In the opening animation slowly raise the gradient $G$ from 4 to 9.8 and watch the readouts "phase advance per cell" and "largest rms size":

| Gradient G | Phase advance per period | Largest rms size |
|---|---|---|
| 4 T/m | 48.3° | 1.97 mm |
| 5 T/m | 61.4° | 1.86 mm |
| 6 T/m | 75.6° | 1.82 mm |
| 8 T/m | 109.6° | 1.95 mm |
| 9.5 T/m | 151.7° | 2.87 mm |
| about 9.8 T/m | unstable | particles hit the wall |

The beam first gets thinner, then wider again. With a small phase advance the pushes are too gentle and the particles swing far; with a large one they overshoot, and the beam is flung wide in the F quadrupoles. For a thin-lens FODO the largest β, in the F quadrupole, is smallest at a phase advance of about $76^\circ$ (derived in the "going deeper" part). Judged by beam size alone, phase advances between $60^\circ$ and $90^\circ$ differ little.

### Upper limit: zero-current phase advance below 90°

A single particle only requires $\sigma_0 < 180^\circ$ ($\lvert\Tr M\rvert < 2$). High-intensity design has a stricter rule: **the zero-current transverse phase advance per focusing period, $\sigma_{0t}$, stays below $90^\circ$**. It does not come from single particles but from oscillations of the whole beam **envelope** (the outline of the beam, i.e. how the rms size varies along the line).

Why $90^\circ$ of all values? Picture the beam as an ellipse in phase space. Each particle turns through $\sigma$ on the ellipse every period. If the beam does not have the matched shape, the ellipse is "lopsided", and as it turns it tumbles like a rugby ball, its projection on the $x$ axis now wide, now narrow. Half a turn brings the ellipse back to the same projection, so **the envelope ripples twice as fast as the particles swing**: when the particles turn $90^\circ$ per period, the envelope turns $180^\circ$ per period.

The periodic structure pushes the envelope once per period. When the envelope turns $180^\circ$ per period, every push lands at the same moment of the envelope's oscillation, like a child on a swing who stands up and squats down twice per swing and pumps the swing higher by changing its length periodically (a **parametric resonance**: what is changed is not an outside force but a parameter of the system itself, at twice its natural frequency). At zero current the envelope is merely the result of the individual particles' motion, and this coincidence does no harm; with space charge the size of the beam feeds back into the force on the particles, the envelope becomes a self-driven oscillator, and it really is pumped ever larger: the **envelope instability**.

Quantitatively: small envelope oscillations of a round beam in a uniform focusing channel come in two modes (derivation in [matching](#matching-envelope-modes)):

$$
k_{\mathrm{breathing}} = \sqrt{2k_0^2 + 2k^2},\qquad k_{\mathrm{quadrupole}} = \sqrt{k_0^2 + 3k^2}
$$

where $k_0$ and $k$ are the phase advances per metre of a particle without and with space charge. In the "breathing mode" both directions swell and shrink together; in the "quadrupole mode" one direction swells while the other shrinks. At zero current both equal $2k_0$: the envelope turns $2\sigma_0$ per period, twice the particle phase advance.

Space charge lowers $k$ and with it the mode frequencies, so the unstable band lies on the side $\sigma_0 > 90^\circ$ with sufficiently high current. In the example FODO ($\sigma_0 = 61.4^\circ$) both modes advance $122.9^\circ$ per period at zero current; at 10 mA DC, $\sigma/\sigma_0 = 0.876$, the breathing mode drops to $115.5^\circ$ and the quadrupole mode to $111.6^\circ$, still far from $180^\circ$. For $\sigma_0 < 90^\circ$ the zero-current envelope modes stay below $180^\circ$ per period and space charge only lowers them, so this unstable band is never entered. To leave margin for errors and current, high-intensity designs usually choose $\sigma_{0t} \approx 50^\circ$–$80^\circ$. In addition, when $\sigma_0 > 90^\circ$ and space charge depresses the phase advance to around $90^\circ$, a fourth-order resonance ($4\sigma \approx 360^\circ$) is excited and the emittance grows.

### Lower limit: too small a phase advance is bad too

By the smooth approximation $x_{\mathrm{rms}} \approx \sqrt{\varepsilon L_p/\sigma}$, a small phase advance means a large beam and a large aperture. More important, the relative effect of space charge grows: the repulsion between particles does not change with the focusing, so the weaker the focusing, the larger the fraction it cancels, and the more the phase advance is "depressed". For the example FODO, by the smooth approximation for a DC beam:

| DC current | σ with space charge | σ/σ0 |
|---|---|---|
| 0.18 mA | 61.3° | 0.998 |
| 5 mA | 57.5° | 0.936 |
| 10 mA | 53.8° | 0.876 |
| 20 mA | 47.2° | 0.769 |

(The generalized perveance $K = 2I/(I_0\beta^3\gamma^3)$ measures the strength of space charge, $I_0 = 4\pi\varepsilon_0 mc^3/q \approx 31.3\ \mathrm{MA}$, KV-equivalent edge emittance $4\varepsilon_{\mathrm{rms}}$; the steps are in the "going deeper" part.) Space charge of the example 0.18 mA beam is weak, but above 10 mA the depression is no longer negligible.

In the lab below raise the DC current `I` and watch $\sigma/\sigma_0$ and the envelope; then set `G` to 6.6 ($\sigma_0 \approx 85^\circ$) and compare at the same current: at 10 mA the lab gives $\sigma/\sigma_0$ rising from 0.878 to 0.910 (a few thousandths from the smooth-approximation estimate in the table); a strongly focusing channel suffers less from space charge.

```avas-lab
space-charge I=10 W=1.527 mass=938.272 q=1 emit=0.1377 G=5 Lq=0.1 Ld=0.3 cells=10
```

> **Think**: if a larger phase advance helps against space charge, why not simply take $85^\circ$?
>
> Answer: $85^\circ$ is only $5^\circ$ below $90^\circ$. Gradient errors, the change of phase advance with energy and the modulation by longitudinal motion can all push the local phase advance over $90^\circ$; at large phase advance $\beta_{\max}/\beta_{\min}$ is also larger, so the beam is bigger in the F quadrupoles and more sensitive to errors. $50^\circ$–$80^\circ$ is the compromise between these.

## Longitudinal: cavities, synchronous phase and energy gain {#lattice-design-longitudinal}

### Longitudinal focusing and RF defocusing

The longitudinal direction (the direction of travel) needs a "lens" too, or fast and slow particles drift ever farther apart. The longitudinal lens is the RF field itself. The RF electric field varies as a cosine in time; the design fixes the moment (the RF **phase**) at which an ideal particle crosses the cavity, the **synchronous phase** $\varphi_s$. With $\varphi_s < 0$ a late particle meets a stronger field, gains more energy and catches up with the synchronous particle; an early particle gains less and is caught up. This is phase stability (see [longitudinal dynamics](#longitudinal)). Think of surfing: standing ahead of the crest, the wave pushes you forward; whoever slides back is pushed harder and returns. The farther from the crest you stand (larger $\lvert\varphi_s\rvert$), the stronger the restoring force, but the weaker the push forward (the energy gain). The analogy fails at large amplitude: the real restoring force varies as a sine of the phase, and a particle that strays too far leaves the separatrix.

Treating this back-and-forth chasing as a simple harmonic oscillation too, the zero-current longitudinal phase advance per metre is (derived in two steps, "more energy, then faster", in the "going deeper" part)

$$
k_{l0}^2 = \frac{2\pi\, q E_0 T \sin(-\varphi_s)}{m c^2\, \lambda\, \beta^3\gamma^3}
$$

where $E_0T$ is the period-averaged effective accelerating gradient (for one cavity per period take $V_0T/L_p$, $V_0T$ being the effective cavity voltage), $\lambda$ is the RF wavelength and $mc^2$ the rest energy of the particle. The $\beta^3\gamma^3$ in the denominator says: the slower the beam, the stronger the longitudinal focusing, because the speed of a slow particle is more sensitive to a change of energy.

Longitudinal focusing has a price. In space free of charge, field lines do not appear or vanish out of nothing (written $\nabla\cdot\boldsymbol E = 0$; this rule was explained in the "going deeper" part of the [quadrupole](#quadrupole-deep-field) chapter): if the longitudinal field pushes particles towards the bunch centre, the field lines must "leak out" sideways, so the two transverse directions together must receive a push of equal size and opposite sign, i.e. transverse **defocusing**. (The rigorous argument is made in the rest frame of the bunch, where the RF field is nearly electrostatic and the relation holds directly; in the laboratory frame the force of the RF magnetic field has to be included, with the same conclusion.) For an axisymmetric field each transverse plane receives half of the longitudinal value, so

$$
k_{t0}^2 = k_{\mathrm{foc}}^2 - \frac{k_{l0}^2}{2}
$$

$k_{\mathrm{foc}}$ is the transverse focusing provided by quadrupoles or solenoids. The transverse elements must therefore be stronger than without RF to reach the same $\sigma_{0t}$.

Example: the example HWR at $\varphi_s = -33^\circ$, $K_e = 1.36$ gains $0.339\ \mathrm{MeV}$ per cavity in the linear preview, corresponding to $V_0T \approx \Delta W/(q\cos\varphi_s) = 0.405\ \mathrm{MV}$ (the effective value along the actual trajectory at this phase; assuming the particle's speed does not change inside the cavity gives $0.389\ \mathrm{MV}$, the difference coming from the acceleration inside the cavity, see below). With one cavity every $L_p = 0.7\ \mathrm{m}$ and a mean $\beta = 0.060$, the smooth approximation gives $\sigma_{0l} \approx 92^\circ$; the RF defocusing is equivalent to a "negative phase advance" of $\sigma_{0l}/\sqrt{2} \approx 65^\circ$, so to obtain $\sigma_{0t} = 60^\circ$ the solenoids must provide about $\sqrt{60^2 + 65^2} \approx 89^\circ$. Lowering the cavity voltage to $V_0T = 0.2\ \mathrm{MV}$ gives $\sigma_{0l} \approx 65^\circ$.

> **Think**: what limit of low-energy design does this example show?
>
> Answer: at $\beta \approx 0.06$, cavities at full gradient would push the longitudinal phase advance per period above $90^\circ$, and the RF defocusing forces very strong transverse elements. So cavities at the low-energy end usually cannot run at full gradient; the gradient is raised step by step as the speed increases and $\beta^3\gamma^3$ grows.

> **Note**: the $\sigma_{0l}$ above uses the smooth approximation, which is already inaccurate near $90^\circ$ and gives only the order of magnitude; the exact value in a periodic structure has to be checked with a multi-particle simulation or the "Phase advance" plot of the Results page in AVAS.

### Periods of cavities and focusing

Each period usually contains one or several cavities and a set of transverse focusing elements, for example S–C (solenoid + cavity), S–C–C, FD–C. The longer the period, the lower the energy gain per metre, but the more room between cavities (for diagnostics, steerers, vacuum valves). Superconducting low-β sections often put only one cavity per period to lower the longitudinal phase advance per period.

In the thin-gap approximation the energy gain of one cavity is

$$
\Delta W = q V_0 T(\beta) \cos\varphi_s
$$

$T(\beta)$ is the **transit-time factor**: the particle needs time to cross the gap, during which the field changes, so it gains less than "charge times peak voltage"; $T$ is this discount (between 0 and 1, depending on speed). For field-map elements AVAS does not use this formula but integrates $q\,K_e E_z(z)\cos(\omega t + \varphi_0)$ along the map; with `V3 = 0` the synchronous phase is defined as $\varphi_s = \operatorname{atan2}(\int E\sin\varphi\,\dd z, \int E\cos\varphi\,\dd z)$ and the engine solves for the entrance phase (see [RF cavity](#rf-cavity)). At low β, when one cavity adds more than 20 % of the entrance energy, the particle accelerates noticeably inside the cavity and $\Delta W/\cos\varphi_s$ is no longer constant: for the example cavity from $\varphi_s = -20^\circ$ to $-40^\circ$, $\Delta W/\cos\varphi_s$ falls from $0.437$ to $0.386\ \mathrm{MeV}$ (linear preview); if the particle's speed is artificially held constant inside the cavity, it is about $0.389\ \mathrm{MeV}$ at every phase, so the difference really comes from the acceleration inside the cavity and not from the preview's algorithm.

### The energy-gain curve

In three identical S–C periods (the lattice example in [In AVAS](#lattice-design-in-avas)), the linear preview gives exit energies of $1.867$, $2.332$ and $2.895\ \mathrm{MeV}$ for the three cavities, gains of $0.340$, $0.465$ and $0.563\ \mathrm{MeV}$, on average $0.69\ \mathrm{MeV}$ per metre. The gain grows from cavity to cavity because the particle speed approaches the optimum β of the HWR and the transit-time factor $T(\beta)$ increases. In general:

- one kind of cavity covers a range of β, with $T(\beta)$ largest near the optimum β and the gain dropping at both ends of the range;
- the design should plot $W(z)$ and $\Delta W$ per cavity and check the range in which each kind of cavity is used;
- independently phased superconducting cavities can each be set to their own phase to suit the changing speed, which makes them more flexible than multi-gap normal-conducting structures.

### How the synchronous phase is chosen

The range of particles the longitudinal motion can "hold" is the **longitudinal acceptance** (the region of phase space inside the separatrix). It grows with $\lvert\varphi_s\rvert$: its phase width is about $3\lvert\varphi_s\rvert$ (exact values: $60.3^\circ$ at $-20^\circ$, $100.2^\circ$ at $-33^\circ$, $122.2^\circ$ at $-40^\circ$), and its energy half-width is (derived from energy conservation in the "going deeper" part)

$$
\Delta W_{\max} = \left[\frac{2\,q\, m c^2 \beta^3\gamma^3 E_0 T \lambda\,(\varphi_s\cos\varphi_s - \sin\varphi_s)}{\pi}\right]^{1/2}
$$

For the example, with $E_0T = 0.405\ \mathrm{MV}/0.7\ \mathrm{m}$, $\beta = 0.06$ and $\varphi_s = -33^\circ$, it is about $\pm 0.092\ \mathrm{MeV}$. But the larger $\lvert\varphi_s\rvert$, the smaller $\cos\varphi_s$ and the lower the accelerating efficiency, and the larger $\sin(-\varphi_s)$ makes the longitudinal phase advance larger. Common practice:

1. at the low-energy end use a larger $\lvert\varphi_s\rvert$ (for example $-40^\circ$ to $-30^\circ$) to ensure capture and acceptance;
2. reduce $\lvert\varphi_s\rvert$ gradually as the energy rises (for example to about $-20^\circ$) to raise the accelerating efficiency;
3. adjust $E_0T$ and $\varphi_s$ together so that the longitudinal phase advance **per metre** $k_{l0}$ varies smoothly along the linac, avoiding jumps that cause longitudinal mismatch.

$k_{l0}^2 \propto E_0T\sin(-\varphi_s)/(\beta^3\gamma^3)$: as the speed increases, the longitudinal focusing weakens by itself, so the gradient can be raised step by step as β grows.

In the RF-cavity lab change the synchronous phase and watch how the energy gain and the longitudinal rms size vary with phase:

```avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
```

## Between the planes: resonances and equipartitioning {#lattice-design-coupling}

Keeping the transverse and longitudinal phase advances each below $90^\circ$ is not enough; their relation matters too. The reason is that space charge couples the three directions: an oscillation in one direction changes the shape of the bunch and so the forces in the other two. When two oscillation frequencies are close or in a simple ratio, these mutual pushes add up time after time (just as two tuning forks of the same pitch set each other ringing), and emittance is transferred between the planes.

- Avoid $\sigma_l \approx \sigma_t$ (with space charge, use the depressed phase advances): when the two planes oscillate at the same frequency, space-charge coupling is strongest, and a non-equipartitioned beam exchanges emittance between the planes.
- Avoid $\sigma_l \approx 2\sigma_t$: the particles' longitudinal (synchrotron) oscillation modulates the RF defocusing and so drives the transverse motion parametrically, resonantly when the frequency ratio is 2.
- The **equipartitioning** condition $\varepsilon_{nl}\,k_l = \varepsilon_{nt}\,k_t$ (rms normalized emittances and depressed wave numbers, with emittances normalized consistently): when it holds, the two planes have the same "temperature" and there is no free energy driving emittance exchange, just as two bodies at the same temperature exchange no heat on contact.
- The Hofmann stability chart uses the ratio of wave numbers $k_z/k_x$ and the transverse depression $k_x/k_{x0}$ as coordinates and, for a given emittance ratio $\varepsilon_z/\varepsilon_x$, draws the unstable bands of space-charge coupling resonances. A non-equipartitioned design works just as well if the working point lies in a stable region or crosses narrow resonance bands quickly.

These rules matter most at high current; for the example 0.18 mA beam, space charge is weak and the main constraints are the single-particle $\sigma_0 < 90^\circ$ and the longitudinal acceptance.

## Transitions between sections {#lattice-design-transitions}

By the smooth approximation the mean β function of the matched beam is about $1/k_0$. So as long as two sections have the same **phase advance per metre**, their matched beams have similar sizes and the junction needs only small corrections; if the phase advance per metre jumps, the matched beam size jumps, and a dedicated matching section is needed in between. That is the core of transition design.

- **Continuous phase advance per metre**: keep the transverse and longitudinal $k_0 = \sigma_0/L_p$ as continuous as possible across the transition. $\sigma_0$ may change with the period length, but the phase advance per metre should not jump.
- **Changing period length**: a longer period makes the β function larger. Example: doubling the period of the example FODO from 0.8 m at the same phase advance per period changes $\bar\beta$ from $0.746$ to $1.49\ \mathrm{m}$, and the beam size grows by $\sqrt2$ ($1.34 \to 1.90\ \mathrm{mm}$); keeping the phase advance per metre would require $122.9^\circ$ per period, above $90^\circ$. So the period can only lengthen gradually with energy. Fortunately acceleration helps: the geometric emittance shrinks adiabatically as $1/(\beta\gamma)$ ("adiabatically" meaning slowly enough for the beam to follow); accelerating from 1.527 MeV to about 5 MeV raises $\beta\gamma$ from 0.0571 to 0.103, which at the same $\bar\beta$ shrinks the beam to 0.74 times its size and makes room for longer periods.
- **Frequency jumps**: doubling the frequency doubles the RF phase width of a given bunch length. Example: the example bunch has an rms length of 1.32 mm, which is $4.5^\circ$ at 162.5 MHz but $9.0^\circ$ at 325 MHz; the longitudinal acceptance becomes relatively smaller, so the bunch has to be compressed before the transition, or the new section starts with a larger $\lvert\varphi_s\rvert$.
- **Matching sections**: when the periodic Twiss parameters of two sections differ substantially, insert a dedicated matching section in between (quadrupoles, solenoids or bunchers), see [matching](#matching-sections).

Mismatch at transitions is one of the main sources of emittance growth and halo (sparse particles far from the beam core). After the design, plot the rms envelope and the "Phase advance" plot for the whole line on the Results page; in a new section the envelope should quickly settle into a periodic shape again.

## Aperture and beam size {#lattice-design-aperture}

The ratio of the aperture radius $R$ (the inner radius of the beam pipe) to the rms beam size is a direct measure of the "loss margin". There is no universal standard; the following are common rules of thumb in low-loss high-intensity linac design, for orientation only:

- design (error-free) beam: $R/x_{\mathrm{rms}} \gtrsim 6$–$10$;
- worst case with errors: the largest beam size (including halo) must not reach the aperture;
- high-power machines require very low losses per metre and use more conservative ratios.

Why as much as 6 to 10? If the distribution were truly Gaussian (a normal distribution), the fraction beyond $6\sigma$ in one dimension would be only $2\times 10^{-9}$. But losses are decided by the tails of the distribution, and the tails are precisely not Gaussian: halo from space charge and mismatch, and centroid offsets from errors, push the real edge far beyond the rms estimate. The margin of the aperture ratio is mostly for them.

From $x_{\mathrm{rms}} \approx \sqrt{\varepsilon_n/(\beta\gamma\,k_0)}$ ($\varepsilon_n$ being the normalized emittance), the aperture ratio can be raised by increasing the phase advance per metre (limited by the $90^\circ$ rule) or by waiting until acceleration has made the geometric emittance smaller.

> **Note**: the aperture ratio is based on the rms size only, while losses are decided by the tails. A truncated Gaussian, halo from space charge and centroid offsets from errors all make the real envelope much larger than the rms estimate; in the end this must be confirmed with multi-particle simulations (look at the maximum sizes in DataSet columns 22/24 and the surviving particles in column 28) and error studies (see [errors and correction](#errors)).

## Think {#lattice-design-think}

> **Think 1**: in the opening animation a particle swings about once in 6 periods. Set the gradient to 8 T/m (phase advance $109.6^\circ$): how many swings now?
>
> Answer: $6 \times 109.6/360 \approx 1.8$; the animation's "oscillations" readout is 1.81 (the weak space charge of the example's 0.18 mA slows the swing a little). The phase advance is "the angle swung per period"; times the number of periods, divided by $360^\circ$, it gives the number of swings.

> **Think 2**: double the period length at the same phase advance per period. Does the beam get wider or thinner?
>
> Answer: wider. The phase advance per metre halves, and by the smooth approximation $x_{\mathrm{rms}} \approx \sqrt{\varepsilon/k_0}$ the beam size grows by $\sqrt2$. Compare lattices by phase advance per metre, not per period.

> **Think 3**: a period with a zero-current phase advance of $100^\circ$ is perfectly stable for a single particle. Why do high-intensity designs still avoid it?
>
> Answer: $180^\circ$ is the stability limit for single particles, $90^\circ$ the limit for the envelope. The envelope oscillates twice as fast; at a phase advance of $100^\circ$ the zero-current envelope turns $200^\circ$ per period, and as space charge depresses it, it sweeps through $180^\circ$ into the envelope-instability band.

## Common misconceptions {#lattice-design-misconceptions}

- **"The larger the gradient, the thinner the beam"**: the beam gets thinner as the phase advance grows, wider again above about $76^\circ$, and the period becomes unstable near $180^\circ$.
- **The thin-lens phase advance is too high**: when $kL_q^2$ is not small, the thin-lens formula overestimates the phase advance ($68.1^\circ$ against $61.4^\circ$ in the example); design with the thick matrix.
- **$90^\circ$ versus $180^\circ$**: $180^\circ$ is the stability limit for a single particle; $90^\circ$ is the rule for envelope stability of intense beams. A period with $\sigma_0 = 100^\circ$ at zero current is perfectly stable for single particles but unsuitable for high intensity.
- **Per period versus per metre**: the beam size is set by the phase advance per metre. Comparing two lattices by phase advance per period alone is not enough.
- **Energy scaling of solenoids and quadrupoles**: the $1/f$ of single lenses scales differently, but the phase advance of both kinds of period scales roughly as field $/B\rho$; what limits solenoids is the field they need.
- **The "Phase advance" plot of the Results page**: it is computed from the Twiss β of the actual beam in DataSet and equals the phase advance of the periodic structure only when the beam is matched.
- **Aperture ratio and the 3 × rms warning**: the linear preview warns of possible losses when $3\times$ rms exceeds the aperture; that is only an alarm for obvious problems, the design aperture ratio is much larger.

## Going deeper: is a period stable? The trace criterion and the FODO formula {#lattice-design-deep-stability}

The main text gave two results: stability for $\lvert\tfrac12\Tr M\rvert < 1$, and $\sin(\sigma/2) = L_p/(4f)$ for FODO. This section derives both with nothing but matrix multiplication, quadratic equations and trigonometric identities; the last paragraph is for readers who know linear algebra.

### Step 1: the conversion table of one period

The [drift](#drift-deep-matrix) chapter wrote the state of a particle as a column of two numbers (position $x$, angle $x'$), and each element as a 2×2 conversion table: a drift of length $L$ is $\begin{pmatrix}1 & L\\ 0 & 1\end{pmatrix}$, a thin lens of focal length $f$ is $\begin{pmatrix}1 & 0\\ -1/f & 1\end{pmatrix}$ (see [quadrupole](#quadrupole-deep-matrix)). A thin-lens FODO period consists of a focusing lens $F$ (focal length $+f$), a drift $O$ (length $L = L_p/2$), a defocusing lens $D$ (focal length $-f$) and a drift $O$. The element the particle meets first is written on the right, so the table of the whole period is $M = O\,D\,O\,F$.

### Step 2: multiply the four tables

One table at a time (each step is school matrix multiplication: row times column):

$$
O F = \begin{pmatrix} 1 - \frac{L}{f} & L \\ -\frac{1}{f} & 1 \end{pmatrix},\qquad
D\,O F = \begin{pmatrix} 1 - \frac{L}{f} & L \\ -\frac{L}{f^2} & 1 + \frac{L}{f} \end{pmatrix}
$$

$$
M = O\,D\,O F = \begin{pmatrix} 1 - \frac{L}{f} - \frac{L^2}{f^2} & 2L + \frac{L^2}{f} \\ -\frac{L}{f^2} & 1 + \frac{L}{f} \end{pmatrix}
$$

You can check $M_{11}M_{22} - M_{12}M_{21} = 1$ (every table has determinant 1, so does the product; this is the conservation of phase-space area). The sum of the diagonal, the **trace**, is

$$
\Tr M = M_{11} + M_{22} = 2 - \frac{L^2}{f^2}.
$$

### Step 3: many periods: a recurrence

Let $X_n$ be the particle's state at the entrance of period $n$; then $X_{n+1} = M X_n$. For any 2×2 matrix of determinant 1 there is an identity you can verify by multiplying out:

$$
M^2 = (\Tr M)\,M - I ,
$$

$I$ being the unit matrix. (Check: with $M = \begin{pmatrix}a & b\\ c & d\end{pmatrix}$, the top-left entry of $M^2$ is $a^2 + bc$, that of the right-hand side $(a+d)a - 1$; they are equal precisely because $ad - bc = 1$, and likewise for the other three entries.) Multiplying both sides by $X_{n-1}$ gives $X_{n+1} = (\Tr M)\,X_n - X_{n-1}$. Looking only at the position, with $T = \Tr M$:

$$
x_{n+1} = T\,x_n - x_{n-1}.
$$

The position at each period entrance is determined by the two before it; that is all we need to study.

### Step 4: when does it run away?

Try a geometric sequence $x_n = r^n$. Substituting into the recurrence and dividing by $r^{n-1}$:

$$
r^2 - T\,r + 1 = 0 .
$$

This is a school quadratic equation. The product of its roots is 1 (Vieta's formulas). When the discriminant $T^2 - 4 > 0$, i.e. $\lvert T\rvert > 2$, both roots are real with product 1, so one of them has magnitude above 1: every period the particle's distance from the axis is multiplied by this factor and keeps growing.

Example: at $L_d = 0.3\ \mathrm{m}$, $G = 10\ \mathrm{T/m}$ the thick-lens matrix gives $T/2 = -1.083$ (see the table in [quadrupole](#quadrupole-deep-fodo)); the roots are $-0.667$ and $-1.499$: every period the particle ends up 1.5 times farther from the axis and on the other side of it, and after 6 periods $1.5^6 \approx 11$ times farther.

### Step 5: when does it oscillate?

For $\lvert T\rvert < 2$ we may write $T = 2\cos\sigma$. The trigonometric identity (sum to product)

$$
\cos\big((n+1)\sigma + \phi\big) + \cos\big((n-1)\sigma + \phi\big) = 2\cos\sigma\,\cos(n\sigma + \phi)
$$

shows that $x_n = A\cos(n\sigma + \phi)$ satisfies the recurrence ($A$ and $\phi$ are fixed by the entrance position and angle). It never exceeds $A$, and its angle advances by $\sigma$ every period: this is the oscillation of the main text, $\sigma$ is the phase advance per period, and $\cos\sigma = \tfrac12\Tr M$. The thick-lens matrix of the example FODO has $T/2 = 0.478$, so $\sigma = 61.4^\circ$.

### Step 6: the FODO phase-advance formula

Insert the trace from step 2: $\cos\sigma = 1 - \dfrac{L^2}{2f^2}$. With the double-angle formula $1 - \cos\sigma = 2\sin^2\dfrac{\sigma}{2}$:

$$
\sin\frac{\sigma}{2} = \frac{L}{2f} = \frac{L_p}{4f}.
$$

Numbers: $f = 0.357\ \mathrm{m}$ and $L_p = 0.8\ \mathrm{m}$ give $\sin(\sigma/2) = 0.560$, $\sigma = 68.1^\circ$, higher than the thick-lens $61.4^\circ$. The left side cannot exceed 1, which requires $f > L_p/4 = 0.2\ \mathrm{m}$, i.e. $G < B\rho/(f L_q) = 8.9\ \mathrm{T/m}$; the thick-lens matrix puts the limit at $9.8\ \mathrm{T/m}$, which is what the animation shows.

### Step 7: when is the largest β smallest?

The β function of the periodic solution can be read off $M$ ($\beta = M_{12}/\sin\sigma$, derived in [matching](#matching-deep-periodic)); for a thin-lens FODO at the two kinds of lens it gives the main text's

$$
\frac{\beta_{\max}}{L_p} = \frac{1 + \sin\frac{\sigma}{2}}{\sin\sigma},\qquad \frac{\beta_{\min}}{L_p} = \frac{1 - \sin\frac{\sigma}{2}}{\sin\sigma}.
$$

Tabulate $\beta_{\max}/L_p$: 1.89 at $48.3^\circ$, 1.72 at $61.4^\circ$, 1.67 at $75.6^\circ$, 1.71 at $90^\circ$, 1.93 at $109.6^\circ$, 2.65 at $133.5^\circ$. It is smallest at $\sigma \approx 76.3^\circ$ (about 1.665) and grows on both sides, which is why the largest rms size in the table of the main text first falls and then rises. $\beta_{\max}/\beta_{\min}$, however, grows steadily with the phase advance: about 3 at $61.4^\circ$, about 5.8 at $90^\circ$.

### For readers who know linear algebra

The identity of step 3 is the Cayley–Hamilton theorem for 2×2 matrices. With $\det M = 1$ the eigenvalues satisfy $\lambda_1\lambda_2 = 1$ and $\lambda_1 + \lambda_2 = \Tr M$; for $\lvert\Tr M\rvert < 2$, $\lambda = e^{\pm i\sigma}$ lies on the unit circle and $M^n$ stays bounded; for $\lvert\Tr M\rvert > 2$ one eigenvalue has modulus above 1 and $M^n$ grows exponentially. Writing $M = I\cos\sigma + J\sin\sigma$ ($J^2 = -I$, built from the periodic Twiss parameters) is the form Floquet theory takes for linear periodic systems, see [linear optics](#linear-optics-periodic).

## Going deeper: the smooth approximation and the space-charge depression {#lattice-design-deep-smooth}

The main text estimated the beam size from "the maximum speed of a simple harmonic oscillation is the angular frequency times the amplitude", and gave a table of the phase advance depressed by space charge. This section works out both step by step.

### Step 1: the β function and the phase

In a periodic channel the rms beam size along the line is written $x_{\mathrm{rms}}(s) = \sqrt{\beta(s)\,\varepsilon}$; $\beta(s)$ is the **β function** (in m). The particles' swing is tied to it as well: where β is large, the beam is wide and the particles swing slowly; where β is small, they swing fast. Precisely, the phase advances by $1/\beta$ radians per metre:

$$
\frac{\dd\psi}{\dd s} = \frac{1}{\beta}.
$$

Cut one period into many short pieces $\Delta s$; on each the phase advances by $\Delta s/\beta$, and adding them up gives the phase advance per period $\sigma = \int_0^{L_p} \dd s/\beta$. With uniform focusing β is the same everywhere and $\sigma = L_p/\beta$.

### Step 2: simple harmonic motion in uniform focusing

If the focusing is "smeared out" into a uniform channel with phase advance $k_0$ per metre, a particle performs the simple harmonic motion $x = A\cos(k_0 s + \phi)$ with angle $x' = -k_0 A\sin(k_0 s + \phi)$, so its largest angle is $k_0 A$. The particles of the beam are at different stages of their swing; in the $(x, x')$ plane they form an upright ellipse whose horizontal and vertical half-axes are in the ratio $1 : k_0$. So the rms angle is $x'_{\mathrm{rms}} = k_0\,x_{\mathrm{rms}}$. The rms emittance of an upright ellipse is the product of the two:

$$
\varepsilon = x_{\mathrm{rms}}\,x'_{\mathrm{rms}} = k_0\,x_{\mathrm{rms}}^2
\quad\Longrightarrow\quad
x_{\mathrm{rms}} = \sqrt{\frac{\varepsilon}{k_0}},\qquad \bar\beta = \frac{1}{k_0}.
$$

Numbers: $k_0 = 1.072/0.8 = 1.34\ \mathrm{m^{-1}}$, $\varepsilon = 2.41\ \mathrm{mm\,mrad}$, $x_{\mathrm{rms}} = \sqrt{2.41/1.34}\ \mathrm{mm} = 1.34\ \mathrm{mm}$.

### Step 3: a three-way tug of war

With space charge, picture the beam as a uniform cylinder of radius $a$ (the KV model, $a = 2x_{\mathrm{rms}}$) with edge emittance $\varepsilon_e = 4\varepsilon_{\mathrm{rms}}$. Three effects act on the beam edge: the external focusing pulls in, $k_0^2 a$; space charge pushes out, $K/a$ ($K$ being the generalized perveance of the main text); the emittance pushes out, $\varepsilon_e^2/a^3$ (the particles each have an angle, like the pressure of a gas). For a matched beam they balance:

$$
k_0^2\,a = \frac{K}{a} + \frac{\varepsilon_e^2}{a^3}.
$$

### Step 4: the depressed phase advance from a quadratic equation

With space charge a particle's phase advance per metre becomes $k = \varepsilon_e/a^2$ (the relation of step 2, with part of the focusing cancelled). Substituting $a^2 = \varepsilon_e/k$ into the balance and dividing by $a$:

$$
k_0^2 = \frac{K}{\varepsilon_e}\,k + k^2 ,
$$

a quadratic equation in $k$; its positive root is

$$
\frac{k}{k_0} = \frac{\sigma}{\sigma_0} = \sqrt{1 + u^2} - u,\qquad u = \frac{K}{2\varepsilon_e k_0}.
$$

Numbers: at $I = 10\ \mathrm{mA}$, $K = 2I/(I_0\beta^3\gamma^3) = 0.02/(3.13\times 10^{7} \times 1.86\times 10^{-4}) = 3.44\times 10^{-6}$, $\varepsilon_e = 4 \times 2.41 = 9.65\ \mathrm{mm\,mrad}$, $u = 0.133$ and $\sigma/\sigma_0 = 1.0088 - 0.133 = 0.876$, one row of the table in the main text. At $0.18\ \mathrm{mA}$, $u = 0.0024$ and the ratio is 0.998; at $20\ \mathrm{mA}$, $u = 0.266$ and the ratio is 0.769. $u$ is proportional to $K/k_0$: the stronger the focusing (larger $k_0$), the smaller the relative effect of space charge, which is why the ratio is closer to 1 at $G = 6.6$ in the lab of the main text.

### For readers who know calculus

Step 1 comes from the Floquet solution $x = \sqrt{2J\beta}\cos\psi$, $\psi' = 1/\beta$, of Hill's equation $x'' + k(s)x = 0$, where β satisfies $2\beta\beta'' - \beta'^2 + 4k\beta^2 = 4$. Step 3 is the matched solution $a'' = 0$ of the KV envelope equation $a'' + k_0^2 a - K/a - \varepsilon_e^2/a^3 = 0$ (for a round beam $a = b$, $2K/(a+b) = K/a$).

## Going deeper: longitudinal focusing, RF defocusing and the longitudinal acceptance {#lattice-design-deep-longitudinal}

The main text simply stated the longitudinal phase advance per metre $k_{l0}$, the RF defocusing and the acceptance. This section derives them from "a late particle gains more energy", using a trigonometric identity, simple harmonic motion and energy conservation.

### Step 1: how much more energy a late particle gains

A particle crossing the accelerating gap at phase $\varphi = \varphi_s + \Delta\varphi$ gains $qE_0T\cos\varphi$ per metre. With $\cos(a + b) = \cos a\cos b - \sin a\sin b$, and for small $\Delta\varphi$ $\cos\Delta\varphi \approx 1$, $\sin\Delta\varphi \approx \Delta\varphi$ (radians), it gains per metre more than the synchronous particle by

$$
\Delta W' \approx -qE_0T\sin\varphi_s\;\Delta\varphi = qE_0T\sin(-\varphi_s)\;\Delta\varphi .
$$

With $\varphi_s < 0$, $\sin(-\varphi_s) > 0$: a late particle ($\Delta\varphi > 0$) gains more, an early one less.

### Step 2: more energy, faster, phase recovered

The [drift](#drift-deep-longitudinal) chapter showed that a particle with relative momentum excess $\delta$ is faster by $\Delta v/v = \delta/\gamma^2$; kinetic-energy and momentum offsets are related by $\delta = \Delta W/(mc^2\beta^2\gamma)$ (see [kinematics](#kinematics)), so together $\Delta v/v = \Delta W/(mc^2\beta^2\gamma^3)$. Over 1 m the synchronous particle takes $1/v$, this one $\Delta v/v^2$ less; the RF turns through $2\pi f$ radians per second, so its phase moves ahead per metre by

$$
\Delta\varphi' = -2\pi f\,\frac{\Delta v}{v^2} = -\frac{2\pi}{\beta\lambda}\,\frac{\Delta v}{v} = -\frac{2\pi\,\Delta W}{mc^2\,\lambda\,\beta^3\gamma^3}
$$

($\lambda = c/f$ is the RF wavelength, $v = \beta c$).

### Step 3: together, simple harmonic motion

Step 1 says "phase behind, energy up", step 2 says "energy up, phase catches up". Taking the rate of change of step 2 with $s$ once more and inserting step 1:

$$
\Delta\varphi'' = -\frac{2\pi}{mc^2\lambda\beta^3\gamma^3}\,\Delta W' = -k_{l0}^2\,\Delta\varphi,\qquad
k_{l0}^2 = \frac{2\pi\, q E_0 T \sin(-\varphi_s)}{m c^2\, \lambda\, \beta^3\gamma^3}.
$$

This is the same equation as the spring oscillator $a = -\omega^2 x$, and $k_{l0}$ is the longitudinal phase advance per metre. Numbers: $E_0T = 0.405/0.7 = 0.579\ \mathrm{MV/m}$, $\sin 33^\circ = 0.545$, $\lambda = 1.845\ \mathrm{m}$, and at $\beta = 0.060$ $\beta^3\gamma^3 = 2.17\times 10^{-4}$, giving $k_{l0}^2 = 5.27\ \mathrm{m^{-2}}$, $k_{l0} = 2.30\ \mathrm{m^{-1}}$, a phase advance of $1.61\ \mathrm{rad} = 92^\circ$ per 0.7 m.

### Step 4: why the RF defocusing is one half

In the frame moving with the bunch, the RF field is nearly electrostatic. Longitudinal focusing means that near the bunch centre the longitudinal field is $E_z \approx -g\,z$ ($g > 0$, pushing particles in front backwards and particles behind forwards). Take a small box centred on the bunch: on the front face ($z > 0$) the field points backwards, on the back face ($z < 0$) forwards, so on both faces field lines enter the box, a net inflow of $g$ times the volume of the box. With no extra charge inside, field lines in and out must balance, and the surplus can only leave through the four side faces, so the transverse field on the sides points outward: $\partial E_x/\partial x + \partial E_y/\partial y = g$. With axial symmetry each transverse direction takes half, $E_x = \tfrac{g}{2}x$: defocusing. In the language of phase advances, the square of the transverse phase advance per metre drops by $k_{l0}^2/2$, the main text's $k_{t0}^2 = k_{\mathrm{foc}}^2 - k_{l0}^2/2$. Numbers: $92^\circ/\sqrt2 = 65^\circ$.

### Step 5: at large amplitude like a pendulum: energy conservation gives the acceptance

The approximation of step 1 holds only for small $\Delta\varphi$. Without it, with $w = W - W_s$, the two equations are $w' = qE_0T(\cos\varphi - \cos\varphi_s)$ and $\varphi' = -Aw$, $A = 2\pi/(mc^2\lambda\beta^3\gamma^3)$. By analogy with a pendulum one finds a "total energy" that does not change along the line:

$$
H = \frac{A}{2}\,w^2 + qE_0T\,(\sin\varphi - \varphi\cos\varphi_s).
$$

(Check: its rate of change with $s$ is $Aww'$ from the first term and $qE_0T(\cos\varphi - \cos\varphi_s)\varphi'$ from the second; inserting the two equations, they cancel exactly.) The first term is like kinetic energy, the second like potential energy. The potential is lowest at $\varphi_s$ (the bottom of the valley) and highest at $-\varphi_s$ (the top of the hill). The trajectory of a particle that just makes it over the hilltop is the **separatrix**, and its energy offset is largest at the valley bottom. Setting the potential difference between hilltop and valley bottom equal to the "kinetic energy" at the bottom:

$$
\frac{A}{2}\,w_{\max}^2 = 2qE_0T\,(\varphi_s\cos\varphi_s - \sin\varphi_s)
\quad\Longrightarrow\quad
w_{\max} = \left[\frac{2\,q\,mc^2\beta^3\gamma^3E_0T\lambda\,(\varphi_s\cos\varphi_s - \sin\varphi_s)}{\pi}\right]^{1/2}.
$$

Numbers: at $\varphi_s = -33^\circ = -0.576\ \mathrm{rad}$ the phase factor in brackets is 0.0616, $mc^2\beta^3\gamma^3E_0T\lambda = 0.2175\ \mathrm{MeV^2}$, and $w_{\max} = 0.092\ \mathrm{MeV}$. The other end of the separatrix is where the potential equals that of the hilltop; solving numerically gives $\varphi = -67.2^\circ$, so the phase width is $33^\circ + 67.2^\circ = 100.2^\circ$, as in the main text.

### For readers who know calculus

$H(\varphi, w)$ is the Hamiltonian of the longitudinal motion: $w' = \partial H/\partial\varphi$, $\varphi' = -\partial H/\partial w$ (Hamilton's equations with $w$ as coordinate and $\varphi$ as conjugate momentum). The small-amplitude limit $H \approx \tfrac{A}{2}w^2 + \tfrac12 qE_0T\sin(-\varphi_s)\,\Delta\varphi^2$ gives step 3's $k_{l0}^2 = A\,qE_0T\sin(-\varphi_s)$. Step 4 is $\nabla\cdot\boldsymbol E = 0$ in the bunch rest frame; in the laboratory frame the effective transverse defocusing also contains the contribution of the RF magnetic field, with the same conclusion at low speed.

## In AVAS {#lattice-design-in-avas}

### Skeleton of the file

- Only the lines between the first `start` and the first `end` are simulated; `!` starts a comment.
- Comments serve as section headings: comments such as `! ---- MEBT ----` or `!;;; SC1 ;;;` appear as group headings in the structure tree.
- `section NAME {` … `}` only folds lines in the interface and does not enter the calculation.
- Elements can be named: `Q1 : quad 0.1 0.02 0 5`; the name is used by the Lattice page, parameter scans (`avas scan --target Q1`) and the AI assistant to locate the element. For a run AVAS writes the `lattice.txt` read by the engine into `inputs/` of the output folder, with only the commands the engine knows, and does not rewrite the project's input files.
- The lattice used for runs is the file named by `[lattice] source` in `ini.ini` (the "Use for the run" button of the Lattice page).

### Period markers

The periodic structure lies between `lattice n1 n2` and `lattice_end`; `n1` is the number of elements of one basic period, `n2` is 1:

```avas-params
lattice lattice_end
```

The number of elements between the two commands (drifts, field-map elements and matrix elements count, diagnostics do not) must be a multiple of `n1`, otherwise an error is reported. The "Phase advance" plot of the Results page uses these periods and computes $\int \dd z/\beta$ per period from the beam's Twiss β in DataSet (x, y and z planes), shown per period or per metre. Because it uses the Twiss parameters of the actual beam, it reflects the phase advance with space charge and RF effects, and it equals the phase advance of the periodic structure itself only when the beam is matched.

### A three-period example

Three S–C periods written with the example field maps (4 elements per period: drift, solenoid map, drift, HWR map), period length $0.66\ \mathrm{m}$:

```text
start
! ---- SC section: solenoid + HWR ----
lattice 4 1
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
lattice_end
drift  0.1  0.02  0
outputplane -0.05
end
```

The same beam line in the 2D / 3D view, with the linear preview of the example beam:

```avas-beamline
lattice 4 1
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
drift  0.05  0.02  0
field  0.35  0.02  0  3  0        0    1     0.38   sol
drift  0.05  0.02  0
field  0.21  0.02  0  1  162.5e6  -33  1.36  -1.36  hwr010
lattice_end
drift  0.1  0.02  0
outputplane -0.05
```

Linear preview: final energy about $2.89\ \mathrm{MeV}$, largest rms size about $1.6\ \mathrm{mm}$ (aperture 20 mm, aperture ratio about 13). The solenoid factor `Kb` affects the envelope strongly: at `Kb = 0.45` the largest rms is about $2.8\ \mathrm{mm}$, and at `Kb = 0.531` the line is already overfocused and the preview reports 3 × rms beyond the aperture. The input beam is not the matched beam of this period, so the envelope is not strictly periodic — exactly the problem the [matching](#matching) chapter addresses.

### Other points

- **Superposed fields**: when a solenoid and a cavity are so close that their fringe fields overlap, put them into one superpose block with `superpose` so that both fields act in the overlap; one block holds at most one RF cavity (see [superposed fields](#superpose)).
- **Steerers and diagnostics**: reserve places in the period for `steerer` (zero length, acting on the next element) and `diag_position` / `diag_size` / `diag_energy`, used by error correction (see [errors and correction](#errors)).
- **No matrix elements at the ends**: in multi-particle simulations avoid `quad`, `solenoid` and other matrix elements as the first and last element; add a short `drift`.
- **Output planes**: `outputplane V1` writes the particle distribution V1 metres downstream (or upstream) of its position; it must not lie beyond the end of the lattice, 5 mm from the end fails, 2 cm or more works (see [simulation settings and output](#simulation)).
- **Linear preview first**: while you drag parameters in the visual editor the linear envelope updates at once (reference-particle energy to about 0.01 %, rms usually to 1–2 %; the engine's mean energy feels the off-axis field and can differ by a few per cent of the gain); once it looks reasonable, run the multi-particle engine.
- **Aperture and losses**: the second parameter `R` (m) of every element is its aperture radius, which the engine uses to decide particle losses (with `boundary 1` in `input.txt`, `boundary.txt` is used instead). The linear preview warns "expect losses" where $3\times$ rms exceeds the aperture.

The step-by-step construction is in the [hands-on tutorial](#tutorial).

## Further reading {#lattice-design-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: the chapters on longitudinal and transverse dynamics, space charge and linac design; this chapter follows its notation.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.: periodic focusing channels, the smooth approximation, envelope equations, equipartitioning.
- J. Struckmeier, M. Reiser (1984): envelope oscillations and instabilities of mismatched intense beams in periodic channels, one of the classic analyses behind the $\sigma_0 < 90^\circ$ rule (studies of the KV-beam stability in periodic channels by I. Hofmann and co-workers in the same period reached the same conclusion).
- I. Hofmann (1998) on the stability of anisotropic beams: the ratio of the planes and the resonance chart.
- The complete list is in [references](#references).
