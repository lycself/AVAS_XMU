# Designing a lattice: periods, phase advance and aperture

Suppose you are asked to draw the first part of a superconducting section: how far apart the solenoids go, how strong they are, how hard the cavities are driven, which synchronous phase they run at, how wide the beam pipe is. That looks like many numbers, but they follow from only a few choices: the **period length**, the **phase advance per period** (one transverse, one longitudinal), the **synchronous phase**, and the **beam size and aperture** that result from them. This chapter shows how those choices are made and why, how one section is joined to the next, and finally how the result is written as an AVAS lattice file.

Why a linac is divided into an ion source, an RFQ, an MEBT, a superconducting section and so on, and which problem each section solves, is told in the first part, [From the ion source to the target](#journey-overview), and in the chapters on the [MEBT](#journey-mebt), the [superconducting section](#journey-sc) and the [HEBT](#journey-hebt); this chapter does not repeat it and discusses only how the lattice **inside one section** is designed. It uses the ideas of [linear optics](#linear-optics), [longitudinal dynamics](#longitudinal) and [space charge](#space-charge). Notation: transverse coordinates $(x, x', y, y')$, longitudinal $(z, \delta = \Delta p/p)$, period length $L_p$, phase advance per period $\sigma$ (in degrees), zero-current phase advance $\sigma_0$. The [hands-on tutorial](#tutorial) is the practical version of this chapter.

## What designing a lattice has to answer {#lattice-design-overview}

AVAS tracks the beam from the initial beam given in `beam.txt`: described by Twiss parameters, or read from a particle file (for example the example's `part_rfq.dst`, the distribution at the RFQ exit). It therefore describes the lattice **after a given input beam**, which suits the MEBT, the superconducting accelerating sections and the HEBT. The ion source, the RFQ and the accelerating structures of a normal-conducting DTL are not elements AVAS can model directly; with 3D field maps of these structures they could in principle be read as `field` elements, which this chapter does not discuss.

All examples of this chapter use the beam of the example project `examples/hwr010`, which is exactly the start of a superconducting section: protons, $W = 1.527\ \mathrm{MeV}$, $f = 162.5\ \mathrm{MHz}$, $I = 0.18\ \mathrm{mA}$. For this beam

$$
\gamma = 1.00163,\quad \beta = 0.05698,\quad \beta\gamma = 0.05708,\quad B\rho = 0.1786\ \mathrm{T\,m},\quad \beta\lambda = 0.1051\ \mathrm{m}
$$

$\beta\lambda$ is the distance a particle travels in one RF period and sets the scale of accelerating gaps and cavities; $B\rho$ is the magnetic rigidity and sets how much a given field bends or focuses the beam (see [kinematics](#kinematics)).

Designing the lattice of a section really means answering four questions in turn:

1. **Period structure**: what goes into a period (FODO, doublet, solenoid plus cavity …) and how long it is;
2. **Phase advance per period**: how large the transverse $\sigma_{0t}$ and the longitudinal $\sigma_{0l}$ are;
3. **Acceleration**: how the cavity voltage and synchronous phase vary along the line;
4. **Margins**: what aperture goes with the beam size that results from the first three.

On top of that, sections must be joined. The following sections take these questions in this order.

## Why periodic structures {#lattice-design-periods}

### A pendulum pushed back again and again

Think of one particle's transverse motion first. Without any focusing, a particle with an angle drifts away from the axis along a straight line. Put a lens every so often to push it back, and the particle swings about the axis, like a pendulum that is given a push at regular intervals. Push too gently and the particle wanders too far; push too hard and it overshoots, swinging ever wider. Only when the strength of the push and the interval between pushes fit together does the swing stay bounded for ever.

To make this precise: let $M$ be the 2×2 transfer matrix of one period; after $n$ periods a particle's coordinates are $M^n$ times the initial ones. The motion is bounded exactly when both eigenvalues of $M$ lie on the unit circle. Because $\det M = 1$ (phase-space area is conserved, see [phase space](#phase-space)), the eigenvalues satisfy $\lambda_1\lambda_2 = 1$ and $\lambda_1 + \lambda_2 = \Tr M$. Writing $\lambda = e^{\pm i\sigma}$ gives

$$
\cos\sigma = \tfrac{1}{2}\Tr M,\qquad \left\lvert \tfrac{1}{2}\Tr M \right\rvert < 1
$$

$\sigma$ is the angle a particle turns through in phase space per period, the **phase advance per period**. For $\lvert\Tr M\rvert/2 > 1$ the eigenvalues are real, one exceeds 1, and the motion grows geometrically.

A periodic structure has a second advantage: there is exactly one set of periodic Twiss parameters $(\alpha, \beta)$ for which the phase-space ellipse returns to its shape after every period. A beam entering with these parameters is the **matched beam** (see [matching](#matching)), and its envelope repeats with the period. Without periodicity there is no such "natural" ellipse, and every stretch has to be designed on its own.

### The smooth approximation: beam size is set by the phase advance per metre

The β function and the phase are related by $\dd\psi/\dd s = 1/\beta$. Integrated over one period, $\sigma = \int_0^{L_p} \dd s/\beta$. If β does not vary much within the period, replace it by its mean $\bar\beta$:

$$
\bar\beta \approx \frac{L_p}{\sigma} = \frac{1}{k_0},\qquad k_0 = \frac{\sigma}{L_p},\qquad x_{\mathrm{rms}} \approx \sqrt{\frac{\varepsilon}{k_0}}
$$

Here $\sigma$ is in radians, $\varepsilon$ is the rms geometric emittance and $k_0$ is the **phase advance per metre** (the transverse wave number of the smooth approximation). This is the most useful estimate in design: the beam size depends only on the emittance and the phase advance per metre, not on the details of the focusing elements. Doubling the phase advance per metre shrinks the beam by $1/\sqrt2$.

Example: the FODO example below has $\sigma_0 = 61.4^\circ = 1.072\ \mathrm{rad}$ and $L_p = 0.8\ \mathrm{m}$, so $k_0 = 1.34\ \mathrm{m^{-1}}$ and $\bar\beta = 0.746\ \mathrm{m}$. The geometric emittance of the example beam is $\varepsilon = 0.1377/0.05708 = 2.41\ \mathrm{mm\,mrad}$, giving the estimate $x_{\mathrm{rms}} \approx 1.34\ \mathrm{mm}$; the exact periodic solution is $1.04\ \mathrm{mm}$ at the centre of the D quadrupole and $1.80\ \mathrm{mm}$ at the centre of the F quadrupole, and the smooth approximation gives just the size in between.

### FODO

Focusing (F) and defocusing (D) quadrupoles alternate, with drifts or cavities in between. For thin lenses of focal length $\pm f$ spaced $L_p/2$ apart one finds

$$
\sin\frac{\sigma}{2} = \frac{L_p}{4f},\qquad
\beta_{\max,\min} = \frac{L_p\,(1 \pm \sin\frac{\sigma}{2})}{\sin\sigma}
$$

Stability requires $f > L_p/4$. FODO treats both planes alike and needs few elements per period; on the other hand $\beta_{\max}/\beta_{\min}$ grows with the phase advance, and the beam is largest in the F quadrupoles.

Example: the example beam, quadrupoles $L_q = 0.1\ \mathrm{m}$, $G = 5\ \mathrm{T/m}$, drifts $L_d = 0.3\ \mathrm{m}$ (period $L_p = 0.8\ \mathrm{m}$). $k = G/B\rho = 27.99\ \mathrm{m^{-2}}$, thin-lens focal length $f = 1/(kL_q) = 0.357\ \mathrm{m}$. The thin-lens formula gives $\sigma = 68.1^\circ$; the exact value from the thick-lens matrices is $\sigma = 61.4^\circ$, with periodic $\beta_x = 1.35\ \mathrm{m}$ at the centre of the F quadrupole and $\beta_x = 0.45\ \mathrm{m}$ at the centre of the D quadrupole. The thin-lens approximation overestimates the phase advance when $kL_q^2$ is not small (here $kL_q^2 = 0.28$); design with the thick matrices.

> **Think about it**: in a FODO every quadrupole defocuses in one plane. Why is the net effect focusing in both planes?
>
> Answer: because the beam is large in the F quadrupoles and small in the D quadrupoles. A lens deflects a particle in proportion to its distance from the axis; in the example the x plane has $\beta = 1.35\ \mathrm{m}$ at F and $0.45\ \mathrm{m}$ at D, a factor $\sqrt{3}$ in beam size, so the focusing push wins over the defocusing one. In the y plane the roles are exchanged. Two thin lenses of equal strength and opposite sign a distance $d$ apart have the combined focal length $1/f_{\mathrm{tot}} = d/f^2 > 0$: the net focusing is a second-order effect, which is why a FODO needs much stronger gradients than a single lens would.

Change `G` in the lab below: with the thick matrices the phase advance passes $90^\circ$ at about $6.9\ \mathrm{T/m}$; at about $9.8\ \mathrm{T/m}$, $\lvert\Tr M\rvert/2$ reaches 1 (a phase advance of $180^\circ$), the periodic β function diverges, and beyond that there is no periodic solution. Compare the thick-matrix phase advance with the thin-lens estimate as you go.

```avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
```

### Doublets and triplets

Putting the F and D quadrupoles next to each other (FD + long drift) leaves a long space for cavities. The long drifts of a doublet period are used efficiently, but the β functions of the two planes are not symmetric; a triplet (FDF or DFD) restores the symmetry. Their stability regions differ from FODO and $\Tr M$ must be computed from the actual matrices.

### Solenoid periods

Low-energy superconducting sections often use periods of solenoids and cavities (S–C, S–C–C …). A solenoid focuses both planes at once; in the frame rotating with the Larmor angle its thin-lens focal length is

$$
\frac{1}{f} = \int k^2\,\dd s,\qquad k = \frac{B_z}{2B\rho}
$$

(see [solenoid](#solenoid)). For one thin solenoid plus a drift, period $L_p$, $\cos\sigma = 1 - L_p/(2f)$.

One often hears: "the focusing strength $1/f$ of a solenoid scales as $1/(B\rho)^2$, that of a single quadrupole lens only as $1/B\rho$, so solenoids pay off at low energy (small $B\rho$) and quadrupoles at high energy." For the focal length of a single lens that is right, but it easily suggests that the phase advance of the two kinds of period changes differently with energy. In fact the net focusing of a FODO is second order (see the "Think about it" above), and the phase advance of both periods is roughly proportional to "field $/B\rho$". The real difference is **how much field is needed**. Compare them with the same period length of 0.8 m: for $\sigma = 61.4^\circ$ the FODO above needs only $5\ \mathrm{T/m}$, a pole-tip field of $0.1\ \mathrm{T}$ at a 2 cm aperture, whereas a 0.2 m long solenoid needs $B \approx 0.93\ \mathrm{T}$ (from the thick-lens matrix), about nine times more. Both required fields grow linearly with $B\rho$: at low energy a superconducting solenoid of a few tesla is quite enough, and one element focuses both planes, gives a round beam and fits compactly into a cryomodule; at higher energy the solenoid field needed exceeds what is practical, and quadrupoles become the better choice.

> **Note**: a solenoid rotates the beam about the axis by $\int k\,\dd s$. A round beam is unchanged by the rotation, but a non-round beam or a beam with transverse errors shows x–y coupling; the 2D/3D views of AVAS and DataSet give quantities in the laboratory frame.

## How much phase advance per period {#lattice-design-phase-advance}

### The upper limit: zero-current phase advance below 90°

A single particle only needs $\sigma_0 < 180^\circ$ ($\lvert\Tr M\rvert < 2$). High-intensity design has a stricter rule: **the zero-current transverse phase advance per focusing period $\sigma_{0t}$ stays below $90^\circ$**. It does not come from single particles but from the oscillation of the beam envelope as a whole.

The envelope itself can oscillate: a matched envelope repeats with the period, a mismatched one oscillates about the matched values. Small envelope oscillations of a round beam in a uniform focusing channel have two eigenmodes (derived in [matching](#matching-envelope-modes)):

$$
k_{\mathrm{breathing}} = \sqrt{2k_0^2 + 2k^2},\qquad k_{\mathrm{quadrupole}} = \sqrt{k_0^2 + 3k^2}
$$

where $k_0$ and $k$ are the transverse wave numbers without and with space charge. At zero current both equal $2k_0$: the envelope turns through $2\sigma_0$ per period, twice the particle phase advance, because the envelope is an average of $x^2$ and so oscillates at twice the frequency. Periodic focusing drives the envelope with a rhythm of $360^\circ$ per period; when the envelope oscillation approaches $180^\circ$ per period a half-integer parametric resonance forms (as a child on a swing pumps it higher by standing up and squatting twice per back-and-forth: a parameter, the length of the pendulum, changes at twice the natural frequency), the **envelope instability**. At zero current it corresponds to $\sigma_0 = 90^\circ$.

Space charge lowers $k$ and with it the frequency of the envelope modes, so the stop band lies on the side $\sigma_0 > 90^\circ$ at sufficiently high current. In the example FODO ($\sigma_0 = 61.4^\circ$) both modes advance $122.9^\circ$ per period at zero current; at a DC current of 10 mA, $\sigma/\sigma_0 = 0.876$, the breathing mode drops to $115.5^\circ$ and the quadrupole mode to $111.6^\circ$, still far from $180^\circ$. For $\sigma_0 < 90^\circ$ the zero-current envelope modes stay below $180^\circ$ per period and space charge only lowers them further, so the beam never enters this stop band. To leave a margin for errors and current, high-intensity designs usually take $\sigma_{0t} \approx 50^\circ$–$80^\circ$. Moreover, when $\sigma_0 > 90^\circ$ and space charge depresses the phase advance to near $90^\circ$, a fourth-order resonance ($4\sigma \approx 360^\circ$) is excited and the emittance grows.

### The lower limit: too little phase advance is bad as well

By the smooth approximation $x_{\mathrm{rms}} \approx \sqrt{\varepsilon L_p/\sigma}$, too small a phase advance means a large beam and a large aperture. More important, the relative effect of space charge grows: the space-charge force does not change with the focusing, so the weaker the focusing, the larger the fraction of it that space charge cancels. For the example FODO, estimated with the smooth approximation for a DC beam:

| DC current | σ with space charge | σ/σ0 |
|---|---|---|
| 0.18 mA | 61.3° | 0.998 |
| 5 mA | 57.5° | 0.936 |
| 10 mA | 53.8° | 0.876 |
| 20 mA | 47.2° | 0.769 |

(Generalized perveance $K = 2I/(I_0\beta^3\gamma^3)$, $I_0 = 4\pi\varepsilon_0 mc^3/q \approx 31.3\ \mathrm{MA}$, KV-equivalent edge emittance $4\varepsilon_{\mathrm{rms}}$.) The 0.18 mA example beam has very weak space charge, but above 10 mA the depression is no longer negligible.

In the lab below raise the DC current `I` and watch $\sigma/\sigma_0$ and the envelope; then set `G` to 6.6 ($\sigma_0 \approx 85^\circ$) and compare at the same current: at 10 mA, $\sigma/\sigma_0$ rises from 0.876 to 0.908 — the more strongly focused channel suffers less from space charge.

```avas-lab
space-charge I=10 W=1.527 mass=938.272 q=1 emit=0.1377 G=5 Lq=0.1 Ld=0.3 cells=10
```

> **Think about it**: if a larger phase advance helps against space charge, why not simply take $85^\circ$?
>
> Answer: $85^\circ$ is only $5^\circ$ away from $90^\circ$. Gradient errors, the change of phase advance with energy and the modulation by the longitudinal motion can all push the local phase advance beyond $90^\circ$; at large phase advance $\beta_{\max}/\beta_{\min}$ is also larger, so the beam is bigger in the F quadrupoles and more sensitive to errors. $50^\circ$–$80^\circ$ is the compromise.

## Longitudinal: cavities, synchronous phase and energy gain {#lattice-design-longitudinal}

### Longitudinal focusing and RF defocusing

The longitudinal "lens" is the RF field itself. With a synchronous phase $\varphi_s < 0$ a late particle meets a stronger field, gains more energy and catches up with the synchronous particle: phase stability (see [longitudinal dynamics](#longitudinal)). Think of surfing: standing ahead of the crest, the wave pushes you on; whoever slides back is pushed harder and returns. The farther from the crest you stand (the larger $\lvert\varphi_s\rvert$), the stronger the restoring force, but the weaker the push forward (the energy gain). The analogy fails at large amplitude: the real restoring force varies sinusoidally with phase, and particles that stray too far leave the separatrix.

In the smooth approximation the longitudinal zero-current wave number is

$$
k_{l0}^2 = \frac{2\pi\, q E_0 T \sin(-\varphi_s)}{m c^2\, \lambda\, \beta^3\gamma^3}
$$

where $E_0T$ is the effective accelerating gradient averaged over the period (for one cavity per period $V_0T/L_p$). The $\beta^3\gamma^3$ in the denominator says: the slower the beam, the stronger the longitudinal focusing.

Longitudinal focusing has a price. Where there is no space charge the RF field obeys Laplace's equation $\nabla\cdot\boldsymbol E = 0$: if the longitudinal field has a focusing gradient along $z$, the two transverse directions together must have a gradient of equal size and opposite sign. (The rigorous argument is made in the rest frame of the bunch, where the RF field is nearly electrostatic and this relation holds directly; in the laboratory frame the force of the RF magnetic field has to be added, with the same result.) For an axisymmetric field each transverse plane takes half of the longitudinal one, so

$$
k_{t0}^2 = k_{\mathrm{foc}}^2 - \frac{k_{l0}^2}{2}
$$

$k_{\mathrm{foc}}$ is the transverse focusing of the quadrupoles or solenoids. The transverse elements must therefore be stronger than without RF to reach the same $\sigma_{0t}$.

Example: the example HWR at $\varphi_s = -33^\circ$, $K_e = 1.36$ gains $0.339\ \mathrm{MeV}$ per cavity in the linear preview, so $V_0T \approx \Delta W/(q\cos\varphi_s) = 0.405\ \mathrm{MV}$ (the effective value along the real trajectory at this phase; assuming a constant velocity inside the cavity gives $0.389\ \mathrm{MV}$, the difference comes from the acceleration inside the cavity, see below). With one cavity every $L_p = 0.7\ \mathrm{m}$ and a mean $\beta = 0.060$, the smooth approximation gives $\sigma_{0l} \approx 92^\circ$; the RF defocusing acts like a "negative phase advance" of $\sigma_{0l}/\sqrt{2} \approx 65^\circ$, so to reach $\sigma_{0t} = 60^\circ$ the solenoids must supply about $\sqrt{60^2 + 65^2} \approx 89^\circ$. Lowering the cavity voltage to $V_0T = 0.2\ \mathrm{MV}$ gives $\sigma_{0l} \approx 65^\circ$.

> **Think about it**: which limit of low-energy design does this example show?
>
> Answer: at $\beta \approx 0.06$, cavities at full gradient would push the longitudinal phase advance per period beyond $90^\circ$, and the RF defocusing forces very strong transverse elements. So at the low-energy end the cavities usually cannot run at full gradient; the gradient is raised step by step as the velocity, and with it $\beta^3\gamma^3$, increases.

> **Note**: the $\sigma_{0l}$ above uses the smooth approximation, whose error is already large near $90^\circ$; it is an order-of-magnitude estimate only. Check the exact values of a periodic structure with a multi-particle simulation or the "Phase advance" plot of the Results page.

### Periods of cavities and focusing

A period usually holds one or a few cavities and a set of transverse focusing elements, for example S–C (solenoid + cavity), S–C–C, FD–C. Longer periods give less energy per metre but more room between cavities (for diagnostics, steerers, vacuum valves). Low-β superconducting sections often put only one cavity in each period to keep the longitudinal phase advance per period down.

In the thin-gap approximation the energy gain of one cavity is

$$
\Delta W = q V_0 T(\beta) \cos\varphi_s
$$

$T(\beta)$ is the transit-time factor. For field-map elements AVAS does not use this formula; it integrates $q\,K_e E_z(z)\cos(\omega t + \varphi_0)$ along the map. With `V3 = 0` the synchronous phase is defined as $\varphi_s = \operatorname{atan2}(\int E\sin\varphi\,\dd z, \int E\cos\varphi\,\dd z)$ and the engine solves for the entrance phase (see [RF cavity](#rf-cavity)). At low β, when one cavity adds more than 20 % of the entrance energy, the particle speeds up noticeably inside the cavity and $\Delta W/\cos\varphi_s$ is no longer constant: for the example cavity it drops from $0.437$ to $0.386\ \mathrm{MeV}$ between $\varphi_s = -20^\circ$ and $-40^\circ$ (linear preview); if the velocity inside the cavity is artificially held constant it stays at about $0.389\ \mathrm{MeV}$ at every phase, so the difference really comes from the acceleration inside the cavity, not from the preview's algorithm.

### The energy-gain curve

In three identical S–C periods (the lattice example in [In AVAS](#lattice-design-in-avas)), the linear preview gives cavity exit energies of $1.867$, $2.332$ and $2.895\ \mathrm{MeV}$, gains of $0.340$, $0.465$ and $0.563\ \mathrm{MeV}$, on average $0.69\ \mathrm{MeV}$ per metre. The gain grows from cavity to cavity because the particle velocity approaches the optimum β of the HWR and the transit-time factor $T(\beta)$ increases. In general:

- one cavity type covers a range of β; $T(\beta)$ is largest near the optimum β and the gain drops at both ends of the range;
- in design, plot $W(z)$ and $\Delta W$ per cavity and check the range in which each cavity type is used;
- independently phased superconducting cavities can each be set to their own phase to suit the changing velocity, which makes them more flexible than multi-gap normal-conducting structures.

### The synchronous-phase law

The longitudinal acceptance (the phase space inside the separatrix) grows with $\lvert\varphi_s\rvert$: its phase width is about $3\lvert\varphi_s\rvert$ (exactly $60.3^\circ$ at $-20^\circ$, $100.2^\circ$ at $-33^\circ$, $122.2^\circ$ at $-40^\circ$), and its energy half-width is

$$
\Delta W_{\max} = 2\left[\frac{q\, m c^2 \beta^3\gamma^3 E_0 T \lambda\,(\varphi_s\cos\varphi_s - \sin\varphi_s)}{\pi}\right]^{1/2}
$$

For the example, $E_0T = 0.405\ \mathrm{MV}/0.7\ \mathrm{m}$, $\beta = 0.06$ and $\varphi_s = -33^\circ$ give about $\pm 0.13\ \mathrm{MeV}$. But the larger $\lvert\varphi_s\rvert$, the smaller $\cos\varphi_s$ and the acceleration efficiency, and the larger $\sin(-\varphi_s)$ and the longitudinal phase advance. The usual practice is:

1. at the low-energy end use a larger $\lvert\varphi_s\rvert$ (e.g. $-40^\circ$ to $-30^\circ$) for capture and acceptance;
2. reduce $\lvert\varphi_s\rvert$ gradually as the energy rises (e.g. to about $-20^\circ$) for efficiency;
3. adjust $E_0T$ and $\varphi_s$ together so that the longitudinal phase advance **per metre** $k_{l0}$ changes smoothly along the linac, avoiding jumps that mismatch the beam longitudinally.

$k_{l0}^2 \propto E_0T\sin(-\varphi_s)/(\beta^3\gamma^3)$: the longitudinal focusing weakens by itself as the velocity grows, so the gradient can be raised step by step as β increases.

In the RF-cavity lab change the synchronous phase and watch the energy gain and the longitudinal rms size:

```avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
```

## Between the planes: resonances and equipartitioning {#lattice-design-coupling}

Keeping each plane below $90^\circ$ is not enough; the relation between the planes matters too. The reason is that space charge couples the three directions: an oscillation in one direction changes the shape of the bunch and so the force felt in the other two. When two oscillation frequencies are close or in a simple ratio, these mutual pushes add up in phase, and emittance moves from one plane to another.

- Avoid $\sigma_l \approx \sigma_t$ (with space charge, use the depressed phase advances): when the two planes oscillate at the same frequency the space-charge coupling is strongest, and a non-equipartitioned beam exchanges emittance between the planes.
- Avoid $\sigma_l \approx 2\sigma_t$: the longitudinal (synchrotron) oscillation modulates the RF defocusing, a parametric drive of the transverse motion that resonates at a frequency ratio of 2.
- The **equipartitioning** condition $\varepsilon_{nl}\,k_l = \varepsilon_{nt}\,k_t$ (rms normalized emittances and depressed wave numbers, emittances normalized consistently): when it holds the two planes have the same "temperature" and there is no free energy to drive an emittance exchange, just as two bodies at the same temperature exchange no heat.
- The Hofmann stability chart uses the wave-number ratio $k_z/k_x$ and the transverse depression $k_x/k_{x0}$ as coordinates and, for a given emittance ratio $\varepsilon_z/\varepsilon_x$, shows the stop bands of space-charge coupling resonances. A non-equipartitioned design works as well, as long as the working point lies in a stable region or crosses a narrow stop band quickly.

These rules matter most at high current; for the 0.18 mA example beam space charge is weak, and the main constraints are the single-particle $\sigma_0 < 90^\circ$ and the longitudinal acceptance.

## Transitions between sections {#lattice-design-transitions}

By the smooth approximation the mean β of the matched beam is about $1/k_0$. So if two sections have the same **phase advance per metre**, their matched beams have similar sizes and the junction needs only a small correction; if the phase advance per metre jumps, the matched beam size jumps, and a dedicated matching section is needed. That is the core of transition design.

- **Continuous phase advance per metre**: keep the transverse and longitudinal $k_0 = \sigma_0/L_p$ as continuous as possible across the transition. $\sigma_0$ may change with the period length, but the phase advance per metre should not jump.
- **Changing period length**: a longer period means larger β functions. Example: doubling the period of the example FODO from 0.8 m while keeping the phase advance per period changes $\bar\beta$ from $0.746$ to $1.49\ \mathrm{m}$ and the beam size by $\sqrt2$ ($1.34 \to 1.90\ \mathrm{mm}$); to keep the phase advance per metre the phase advance per period would have to become $122.9^\circ$, beyond $90^\circ$. Periods can therefore only lengthen gradually with energy. Acceleration helps: the geometric emittance shrinks adiabatically as $1/(\beta\gamma)$; accelerating from 1.527 MeV to about 5 MeV raises $\beta\gamma$ from 0.0571 to 0.103, and at the same $\bar\beta$ the beam shrinks to 0.74 of its size, room for a longer period.
- **Frequency jumps**: doubling the frequency doubles the RF phase width of a given bunch length. Example: the example bunch, 1.32 mm rms long, spans $4.5^\circ$ at 162.5 MHz and $9.0^\circ$ at 325 MHz; the longitudinal acceptance becomes relatively smaller, so the bunch must be compressed before the transition or the new section must start with a larger $\lvert\varphi_s\rvert$.
- **Matching sections**: when the periodic Twiss parameters of two sections differ a lot, a dedicated matching section (quadrupoles, solenoids or bunchers) goes in between, see [matching](#matching-sections).

Mismatch at transitions is one of the main sources of emittance growth and halo. After designing, plot the rms envelope and the "Phase advance" plot of the whole line on the Results page; in the new section the envelope should quickly become periodic again.

## Aperture and beam size {#lattice-design-aperture}

The ratio of the aperture radius $R$ to the rms beam size is a direct measure of the "loss margin". There is no universal standard; the following are values commonly used in the design of low-loss high-intensity linacs, for orientation only:

- design (error-free) beam: $R/x_{\mathrm{rms}} \gtrsim 6$–$10$;
- worst case with errors: the largest beam size (including halo) must not touch the aperture;
- high-power machines require very low losses per metre and use more conservative ratios.

Why as much as 6 to 10? If the distribution were truly Gaussian, the fraction beyond $6\sigma$ in one dimension would be only $2\times 10^{-9}$. But losses are decided by the tails, and the tails are precisely what is not Gaussian: halo from space charge and mismatch and centroid offsets from errors make the real edge far larger than the rms estimate. The margin of the aperture ratio is mostly kept for them.

From $x_{\mathrm{rms}} \approx \sqrt{\varepsilon_n/(\beta\gamma\,k_0)}$, the aperture ratio can be raised by more phase advance per metre (limited by the $90^\circ$ rule) or by waiting until acceleration has reduced the geometric emittance.

> **Note**: the aperture ratio is based on the rms size, whereas losses are decided by the tails. Gaussian truncation, space-charge halo and centroid offsets from errors make the real envelope much larger than the rms estimate; confirm with a multi-particle simulation (the maximum sizes in DataSet columns 22/24 and the surviving particles in column 28) and an error study (see [errors and correction](#errors)).

## Common misconceptions {#lattice-design-misconceptions}

- **Thin lenses overestimate the phase advance**: when $kL_q^2$ is not small the thin-lens formula gives too much ($68.1^\circ$ against $61.4^\circ$ in the example); design with the thick matrices.
- **$90^\circ$ versus $180^\circ$**: $180^\circ$ is the limit of single-particle stability; $90^\circ$ is the rule for envelope stability of intense beams. A period with $\sigma_0 = 100^\circ$ at zero current is perfectly stable for single particles but unsuitable for high current.
- **Per period versus per metre**: the beam size is set by the phase advance per metre. Comparing the phase advance per period of two lattices is not enough.
- **Energy scaling of solenoids and quadrupoles**: the single-lens $1/f$ scales differently, but the phase advance of both periods is roughly proportional to field $/B\rho$; what limits solenoids is the field required.
- **The "Phase advance" plot of the Results page**: it is computed from the Twiss β of the actual beam in DataSet and equals the phase advance of the periodic structure only when the beam is matched.
- **Aperture ratio and the 3 × rms warning**: the linear preview warns of possible losses when $3\times$ rms exceeds the aperture; that is only an alarm for obvious problems, the design aperture ratio is much larger.

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
- **Linear preview first**: while you drag parameters in the visual editor the linear envelope updates at once (energy to about 0.01 %, rms usually to 1–2 %); once it looks reasonable, run the multi-particle engine.
- **Aperture and losses**: the second parameter `R` (m) of every element is its aperture radius, which the engine uses to decide particle losses (with `boundary 1` in `input.txt`, `boundary.txt` is used instead). The linear preview warns "expect losses" where $3\times$ rms exceeds the aperture.

The step-by-step construction is in the [hands-on tutorial](#tutorial).

## Further reading {#lattice-design-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: the chapters on longitudinal and transverse dynamics, space charge and linac design; this chapter follows its notation.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.: periodic focusing channels, the smooth approximation, envelope equations, equipartitioning.
- J. Struckmeier, M. Reiser (1984): envelope oscillations and instabilities of mismatched intense beams in periodic channels, one of the classic analyses behind the $\sigma_0 < 90^\circ$ rule (studies of the KV-beam stability in periodic channels by I. Hofmann and co-workers in the same period reached the same conclusion).
- I. Hofmann (1998) on the stability of anisotropic beams: the ratio of the planes and the resonance chart.
- The complete list is in [references](#references).
