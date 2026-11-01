# Putting an accelerator together

The previous chapters dealt with single components. This chapter is about combining drifts, quadrupoles, solenoids and RF cavities into a linac lattice that transports and accelerates the beam stably: choose the focusing period and the phase advance per period, arrange the cavities and the synchronous-phase law, handle the transitions between sections, check the aperture margin, and finally write it all as an AVAS lattice file. The chapter gives design rules and their physical origin; the [hands-on tutorial](#tutorial) is the practical version, building a short superconducting linac step by step from the example HWR cavity and solenoid field maps.

The notation of the [linear optics](#linear-optics), [longitudinal dynamics](#longitudinal) and [space charge](#space-charge) chapters is assumed: transverse coordinates $(x, x', y, y')$, longitudinal $(z, \delta = \Delta p/p)$, period length $L_p$, phase advance per period $\sigma$ (in degrees), zero-current phase advance $\sigma_0$.

## Functional sections of a linac {#lattice-design-overview}

A high-intensity proton or heavy-ion linac usually consists of the following parts (in beam direction):

| Section | Purpose | Typical elements | Simulated by AVAS |
|---|---|---|---|
| Ion source | produces a DC beam | extraction electrodes | no |
| LEBT, low-energy transport | matches the DC beam into the RFQ | solenoids, chopper | no (usually) |
| RFQ | bunches, focuses, accelerates to a few MeV | four-vane / four-rod electrodes | no |
| MEBT, medium-energy transport | transverse and longitudinal matching, chopping, diagnostics | quadrupoles, buncher cavities | yes |
| Accelerating sections | main energy gain | superconducting HWR / spoke / elliptical cavities + solenoids or quadrupoles | yes (cavities as field maps) |
| HEBT, high-energy transport | transport to the target or downstream | quadrupoles, dipoles | yes |

AVAS tracks from the initial beam given in `beam.txt` (or a `.dst` particle file such as the example's `part_rfq.dst`), so it describes the lattice **downstream of a given entrance beam**. Ion sources, RFQs and the accelerating structures of a DTL are not elements AVAS models directly; if a three-dimensional field map of such a structure exists it can in principle be read as a `field` element, but this chapter does not discuss it.

The example project `examples/hwr010` corresponds to the start of a superconducting section: protons, $W = 1.527\ \mathrm{MeV}$, $f = 162.5\ \mathrm{MHz}$, $I = 0.18\ \mathrm{mA}$. For this beam:

$$
\gamma = 1.00163,\quad \beta = 0.05698,\quad \beta\gamma = 0.05708,\quad B\rho = 0.1786\ \mathrm{T\,m},\quad \beta\lambda = 0.1051\ \mathrm{m}
$$

$\beta\lambda$ is the distance a particle travels in one RF period; it sets the scale of cavity gaps and period lengths. $B\rho$ sets how strongly a given quadrupole gradient or solenoid field focuses (see [kinematics](#kinematics)).

## Focusing periods {#lattice-design-periods}

Transverse focusing in a linac is almost always periodic: the same "period" (cell) is repeated many times. The transfer matrix $M$ of one period is stable when

$$
\left\lvert \tfrac{1}{2}\Tr M \right\rvert < 1,\qquad \cos\sigma = \tfrac{1}{2}\Tr M
$$

Then there is a unique periodic set of Twiss parameters $(\alpha, \beta)$ (the matched beam, see [matching](#matching)) and the envelope oscillates in step with the period. Common periods follow.

### FODO

Focusing (F) and defocusing (D) quadrupoles alternate, with drifts or cavities between them. For thin lenses of focal length $\pm f$ spaced $L_p/2$ apart:

$$
\sin\frac{\sigma}{2} = \frac{L_p}{4f},\qquad
\beta_{\max,\min} = \frac{L_p\,(1 \pm \sin\frac{\sigma}{2})}{\sin\sigma}
$$

Stability requires $f > L_p/4$. FODO is symmetric in the two planes and needs few elements per period; on the other hand $\beta_{\max}/\beta_{\min}$ grows with the phase advance, and the beam is largest in the F quadrupoles.

Example: the example beam, quadrupoles $L_q = 0.1\ \mathrm{m}$, $G = 5\ \mathrm{T/m}$, drifts $L_d = 0.3\ \mathrm{m}$ (period $L_p = 0.8\ \mathrm{m}$). $k = G/B\rho = 27.99\ \mathrm{m^{-2}}$ and the thin-lens focal length is $f = 1/(kL_q) = 0.357\ \mathrm{m}$. The thin-lens formula gives $\sigma = 68.1^\circ$; the exact thick-lens matrices give $\sigma = 61.4^\circ$, with periodic $\beta_x = 1.35\ \mathrm{m}$ at the centre of the F quadrupole and $\beta_x = 0.45\ \mathrm{m}$ at the centre of the D quadrupole. When $kL_q^2$ is not small (here $kL_q^2 = 0.28$) the thin-lens approximation overestimates the phase advance; design with the thick matrices.

### Doublets and triplets

Placing the F and D quadrupoles close together (FD + long drift) leaves a long free space for cavities. Doublet periods use the long drift efficiently but the $\beta$ functions differ between the planes; triplets (FDF or DFD) restore the symmetry. Their stability regions differ from FODO and must be checked with the actual $\Tr M$.

### Solenoid periods

Low-energy superconducting sections often use solenoid + cavity periods (S–C, S–C–C, …). A solenoid focuses both planes; in the rotating Larmor frame its thin-lens focal length is

$$
\frac{1}{f} = \int k^2\,\dd s,\qquad k = \frac{B_z}{2B\rho}
$$

(see [solenoid](#solenoid)). A period of one thin solenoid and a drift $L_p$ has $\cos\sigma = 1 - L_p/(2f)$. Solenoid focusing scales as $1/(B\rho)^2$, so at low energy (small $B\rho$) it is "cheaper" than quadrupole focusing, while at higher energy quadrupoles are more effective — one reason why low-energy superconducting sections use solenoids and higher-energy sections switch to quadrupoles.

> Note: a solenoid rotates the beam by $kL$ about the axis. A round beam is unchanged by the rotation, but a non-round beam or a beam with transverse errors becomes x–y coupled; the AVAS 2D/3D views and DataSet show laboratory-frame quantities.

## Choosing the phase advance per period {#lattice-design-phase-advance}

### Transverse zero-current phase advance σ0t < 90°

The most important design rule is: **the zero-current transverse phase advance per focusing period, $\sigma_{0t}$, stays below $90^\circ$**. The reason is a parametric resonance of the envelope. Small envelope oscillations of a round beam in a uniform focusing channel have two eigenmodes (see [matching](#matching-envelope-modes)):

$$
k_{\mathrm{breathing}} = \sqrt{2k_0^2 + 2k^2},\qquad k_{\mathrm{quadrupole}} = \sqrt{k_0^2 + 3k^2}
$$

where $k_0$ and $k$ are the zero-current and space-charge-depressed transverse wave numbers. At zero current both equal $2k_0$: the envelope oscillates with twice the particle phase advance. Periodic focusing modulates the envelope once per $360^\circ$ of the period, and when the envelope oscillation advances $180^\circ$ per period (half-integer parametric resonance) the **envelope instability** occurs, i.e. at $\sigma_0 = 90^\circ$. Space charge lowers the envelope-mode frequencies, so the unstable band lies at $\sigma_0 > 90^\circ$ with high enough current (depressed $\sigma$); for $\sigma_0 < 90^\circ$ the envelope modes advance less than $180^\circ$ per period and do not enter this band. To leave margin for errors and current, high-intensity designs usually take $\sigma_{0t} \approx 50^\circ$–$80^\circ$. In addition, when $\sigma_0 > 90^\circ$ and space charge depresses the phase advance to about $90^\circ$, a fourth-order resonance ($4\sigma \approx 360^\circ$) is excited and causes emittance growth.

### Too small a phase advance is not good either

A small phase advance means weak focusing, large $\beta$ functions and large beams, relatively stronger space charge (smaller tune depression $\sigma/\sigma_0$), and larger apertures. For the example FODO ($\sigma_0 = 61.4^\circ$, $L_p = 0.8\ \mathrm{m}$), estimated with the smooth approximation for a DC beam:

| DC current | depressed σ | σ/σ0 |
|---|---|---|
| 0.18 mA | 61.3° | 0.998 |
| 5 mA | 57.5° | 0.936 |
| 10 mA | 53.8° | 0.876 |
| 20 mA | 47.2° | 0.769 |

(Generalised perveance $K = 2I/(I_0\beta^3\gamma^3)$, $I_0 = 4\pi\varepsilon_0 mc^3/q \approx 31.3\ \mathrm{MA}$, KV-equivalent edge emittance $4\varepsilon_{\mathrm{rms}}$.) The lab of the [space charge](#space-charge) chapter lets you check this by dragging the parameters. The example's 0.18 mA beam has very weak space charge, but above about 10 mA the depression is no longer negligible.

### Longitudinal phase advance and RF defocusing

In an accelerating section a synchronous phase $\varphi_s < 0$ provides longitudinal focusing. In the smooth approximation the zero-current longitudinal wave number is

$$
k_{l0}^2 = \frac{2\pi\, q E_0 T \sin(-\varphi_s)}{m c^2\, \lambda\, \beta^3\gamma^3}
$$

where $E_0T$ is the period-averaged effective accelerating gradient (for one cavity per period, $V_0T/L_p$). By Laplace's equation the RF field defocuses transversely by half the longitudinal focusing:

$$
k_{t0}^2 = k_{\mathrm{foc}}^2 - \frac{k_{l0}^2}{2}
$$

with $k_{\mathrm{foc}}$ the focusing of the quadrupoles or solenoids. The transverse elements must therefore be stronger than without RF to reach the same $\sigma_{0t}$.

Example: with the example HWR at $\varphi_s = -33^\circ$, $K_e = 1.36$ the linear preview gives an energy gain of $0.339\ \mathrm{MeV}$ per cavity, i.e. $V_0T \approx \Delta W/(q\cos\varphi_s) = 0.405\ \mathrm{MV}$. With one cavity per $L_p = 0.7\ \mathrm{m}$ and a mean $\beta = 0.060$ the smooth approximation gives $\sigma_{0l} \approx 92^\circ$; the RF defocusing is equivalent to a "negative" phase advance of $\sigma_{0l}/\sqrt{2} \approx 65^\circ$, so for $\sigma_{0t} = 60^\circ$ the solenoids must provide about $\sqrt{60^2 + 65^2} \approx 89^\circ$. Lowering the cavity to $V_0T = 0.2\ \mathrm{MV}$ gives $\sigma_{0l} \approx 65^\circ$. At the low-energy end ($\beta \approx 0.06$) the cavities therefore usually cannot run at full gradient, or the longitudinal phase advance exceeds $90^\circ$.

> Note: these $\sigma_{0l}$ values use the smooth approximation, which is already inaccurate near $90^\circ$; treat them as order-of-magnitude estimates. In a real periodic structure check with multi-particle simulations or the "Phase advance" plot of the Results page.

### Ratios, equipartitioning and resonances

Besides each being below $90^\circ$, the relation between longitudinal and transverse phase advances matters:

- Avoid $\sigma_l \approx \sigma_t$ (with space charge, use the depressed values): when the two planes oscillate at the same frequency the space-charge coupling is strongest and a non-equipartitioned beam exchanges emittance between the planes.
- Avoid $\sigma_l \approx 2\sigma_t$: the longitudinal (synchrotron) oscillation of a particle modulates the RF defocusing, a parametric drive of the transverse motion that resonates at a frequency ratio of 2.
- The **equipartitioning** condition $\varepsilon_{nl}\,k_l = \varepsilon_{nt}\,k_t$ (rms normalised emittances with a consistent normalisation, depressed wave numbers): the two planes then have the same "temperature" and there is no free energy driving emittance exchange.
- The Hofmann stability charts use the tune ratio $k_z/k_x$ and the transverse tune depression $k_x/k_{x0}$ as coordinates and show, for a given emittance ratio $\varepsilon_z/\varepsilon_x$, the unstable bands of the space-charge coupling resonances. A non-equipartitioned design is equally possible as long as the working point stays in a stable region or crosses narrow resonance bands quickly.

These rules matter most at high current; for the example's 0.18 mA beam space charge is weak and the main constraints are the single-particle $\sigma_0 < 90^\circ$ and the longitudinal acceptance.

## Cavity periods, energy gain and the synchronous-phase law {#lattice-design-longitudinal}

### Periods with cavities and focusing

Each period usually contains one or a few cavities and a set of transverse focusing elements: S–C (solenoid + cavity), S–C–C, FD–C and so on. Longer periods give less energy gain per metre but more space between cavities (for diagnostics, steerers, vacuum valves). Low-β superconducting sections often put only one cavity per period to keep the longitudinal phase advance per period down.

In the thin-gap approximation the energy gain of one cavity is

$$
\Delta W = q V_0 T(\beta) \cos\varphi_s
$$

with $T(\beta)$ the transit-time factor. AVAS does not use this formula for field-map elements: it integrates $q\,K_e E_z(z)\cos(\omega t + \varphi_0)$ along the map; with `V3 = 0` the synchronous phase is defined as $\varphi_s = \operatorname{atan2}(\int E\sin\varphi\,\dd z, \int E\cos\varphi\,\dd z)$ and the kernel solves for the entrance phase (see [RF cavity](#rf-cavity)). At low β, when one cavity adds more than 20 % of the entrance energy, the particle accelerates noticeably inside the cavity and $\Delta W/\cos\varphi_s$ is no longer constant: for the example cavity from $\varphi_s = -20^\circ$ to $-40^\circ$ it falls from $0.437$ to $0.386\ \mathrm{MeV}$ (linear preview).

### The energy-gain profile

For three identical S–C periods (the lattice example below) the linear preview gives cavity exit energies of $1.867$, $2.332$ and $2.895\ \mathrm{MeV}$, i.e. gains of $0.340$, $0.465$ and $0.563\ \mathrm{MeV}$. The gain grows from cavity to cavity because the particle velocity approaches the optimum β of the HWR and the transit-time factor $T(\beta)$ increases. In general:

- one cavity type covers a β range; $T(\beta)$ peaks near the optimum β and the gain drops towards both ends of the range;
- plot $W(z)$ and the gain of each cavity during design to check the range in which each cavity type is used;
- independently phased superconducting cavities can each be phased for the local velocity, which makes them more flexible than multi-gap room-temperature structures.

### The synchronous-phase law

The longitudinal acceptance (the phase space inside the separatrix) grows with $\lvert\varphi_s\rvert$: its phase width is about $3\lvert\varphi_s\rvert$ (exactly $60.3^\circ$ at $-20^\circ$, $100.2^\circ$ at $-33^\circ$, $122.2^\circ$ at $-40^\circ$), and its energy half-width is

$$
\Delta W_{\max} = 2\left[\frac{q\, m c^2 \beta^3\gamma^3 E_0 T \lambda\,(\varphi_s\cos\varphi_s - \sin\varphi_s)}{\pi}\right]^{1/2}
$$

(about $\pm 0.13\ \mathrm{MeV}$ for the example with $E_0T = 0.405\ \mathrm{MV}/0.7\ \mathrm{m}$, $\beta = 0.06$, $\varphi_s = -33^\circ$). But a larger $\lvert\varphi_s\rvert$ means a smaller $\cos\varphi_s$, i.e. less efficient acceleration, and a larger $\sin(-\varphi_s)$, i.e. a larger longitudinal phase advance. The usual practice is:

1. at the low-energy end use a larger $\lvert\varphi_s\rvert$ (for example $-40^\circ$ to $-30^\circ$) for capture and acceptance;
2. reduce $\lvert\varphi_s\rvert$ gradually as the energy rises (for example to about $-20^\circ$) for more efficient acceleration;
3. adjust $E_0T$ and $\varphi_s$ together so that the longitudinal phase advance **per metre**, $k_{l0}$, varies smoothly along the linac and no abrupt change mismatches the beam longitudinally.

Since $k_{l0}^2 \propto E_0T\sin(-\varphi_s)/(\beta^3\gamma^3)$, the longitudinal focusing weakens naturally as the velocity rises, so the gradient can be increased step by step with β.

## Transitions between sections {#lattice-design-transitions}

Sections with different cavity types, period structures or frequencies need transitions. The basic principles:

- **Continuous phase advance per metre**: keep the transverse and longitudinal $k_0 = \sigma_0/L_p$ as continuous as possible across the transition. $\sigma_0$ may change with the period length, but the phase advance per metre should not jump; then the matched Twiss parameters of the two sections are similar and the transition needs only small corrections.
- **Changing period length**: a longer period means larger $\beta$ functions; keeping the beam size requires a larger phase advance per metre, without exceeding $90^\circ$ per period.
- **Frequency jumps**: doubling the frequency doubles the RF phase width of a given bunch length, so the longitudinal acceptance becomes relatively smaller; compress the bunch before the transition or start the new section with a larger $\lvert\varphi_s\rvert$.
- **Matching sections**: when the periodic Twiss parameters of the two sections differ a lot, insert a dedicated matching section (quadrupoles, solenoids or buncher cavities), see [matching](#matching-sections).

Mismatch at transitions is one of the main sources of emittance growth and halo. After a design, plot the rms envelope and the "Phase advance" over the whole line on the Results page; in the new section the envelope should quickly become periodic again.

## Aperture and beam size {#lattice-design-aperture}

The ratio of the aperture radius $R$ to the rms beam size directly measures the loss margin. There is no universal standard; the following values are common rules of thumb in low-loss high-intensity linac design, for orientation only:

- design (error-free) beam: $R/x_{\mathrm{rms}} \gtrsim 6$–$10$;
- worst case with errors: the maximum beam size (including halo) must not touch the aperture;
- high-power machines require extremely low losses per metre and more conservative ratios.

In AVAS the second parameter `R` (m) of every element is the aperture radius the kernel uses to detect particle losses (with `boundary 1` in `input.txt` it uses `boundary.txt` instead). The linear preview warns "expect losses" when $3\times$ the rms size exceeds the aperture.

> Note: the aperture ratio is based on the rms size, whereas losses are decided by the tails of the distribution. A truncated Gaussian, space-charge halo and error-induced centroid offsets all make the real envelope much larger than the rms estimate; confirm with multi-particle simulations (maximum sizes in DataSet columns 22/24, surviving macro-particles in column 28) and error studies (see [errors and correction](#errors)).

## Writing it as an AVAS lattice file {#lattice-design-avas}

### File skeleton

- Only the lines between the first `start` and the first `end` are simulated; `!` starts a comment.
- Use comments as section headings: comments such as `! ---- MEBT ----` or `!;;; SC1 ;;;` appear as group headings in the structure tree.
- `section NAME {` … `}` only folds lines in the GUI and is not simulated.
- Elements can be named: `Q1 : quad 0.1 0.02 0 5`; the name is used by the Lattice page, parameter scans (`avas scan --target Q1`) and the AI assistant to find elements. For a run AVAS writes the kernel's `lattice.txt` in the output's `inputs/` folder with only the commands the kernel knows; the project's input files are not rewritten.
- The lattice that runs is the file named in `[lattice] source` of `ini.ini` (the "Set as run lattice" button of the Lattice page).

### Period markers

`lattice n1 n2` and `lattice_end` enclose a periodic structure; `n1` is the number of elements per basic period and `n2` is written as 1:

```avas-params
lattice lattice_end
```

The number of elements between the two commands (drifts, field maps and matrix elements count, diagnostics do not) must be an integer multiple of `n1`, otherwise an error is reported. The "Phase advance" plot of the Results page uses these periods and the beam Twiss β from DataSet to compute $\int \dd z/\beta$ per period (x, y and z), shown per period or per metre. Because it uses the Twiss parameters of the actual beam, it reflects the phase advance with space charge and RF effects, and it equals the phase advance of the periodic structure itself only for a matched beam.

### A three-period example

Three S–C periods with the example field maps (4 elements per period: drift, solenoid map, drift, HWR map), period length $0.66\ \mathrm{m}$:

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

With the example beam the linear preview gives a final energy of about $2.89\ \mathrm{MeV}$ and a maximum rms size of about $1.6\ \mathrm{mm}$ (aperture 20 mm, ratio about 12). The envelope is very sensitive to the solenoid factor `Kb`: with `Kb = 0.45` the maximum rms is about $2.7\ \mathrm{mm}$, and with `Kb = 0.531` the beam is already overfocused and the preview reports that 3 × rms exceeds the aperture. The entrance beam is not the matched beam of this period, so the envelope is not strictly periodic — exactly the problem the [matching](#matching) chapter solves.

### Further points

- **Superpose**: when a solenoid and a cavity are so close that their fringe fields overlap, put them in one `superpose` block so both fields act in the overlap region; a block may contain at most one RF cavity (see [superpose](#superpose)).
- **Steerers and diagnostics**: reserve places in the period for `steerer` (length 0, acting over the next element) and `diag_position` / `diag_size` / `diag_energy`, used by the error correction (see [errors and correction](#errors)).
- **No matrix elements at the ends**: in multi-particle runs avoid matrix elements such as `quad` or `solenoid` as the first or last element; add a short `drift`.
- **Output planes**: `outputplane V1` dumps the particle distribution V1 metres downstream or upstream of its position; it must not lie past the lattice end, 5 mm before the end fails, 2 cm or more works (see [simulation settings and output](#simulation)).
- **Linear preview first**: the visual editor shows the linear envelope live while you drag parameters (energy to about 0.01 %, rms usually within 1–2 %); once it looks reasonable, run the kernel for the multi-particle simulation.

## Hands-on {#lattice-design-lab}

In the FODO lab change `G` and watch the phase advance per period and the stability. With the thick matrices the phase advance passes $90^\circ$ at `G` ≈ $6.9\ \mathrm{T/m}$; at about $9.8\ \mathrm{T/m}$, $\lvert\Tr M\rvert/2$ reaches 1 (phase advance $180^\circ$), the periodic β functions diverge, and beyond that no periodic solution exists. Compare the thick-matrix phase advance with the thin-lens estimate as well.

```avas-lab
fodo Lq=0.1 G=5 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
```

In the space-charge lab increase the DC current `I` and watch the tune depression $\sigma/\sigma_0$ and the envelope; compare the same current with `G = 5` ($\sigma_0 \approx 61^\circ$) and `G = 6.6` ($\sigma_0 \approx 85^\circ$).

```avas-lab
space-charge I=10 W=1.527 mass=938.272 q=1 emit=0.1377 G=5 Lq=0.1 Ld=0.3 cells=10
```

In the RF cavity lab change the synchronous phase and watch the energy gain and the longitudinal rms size:

```avas-lab
rf-cavity phase=-33 Ke=1.36 W=1.527
```

The complete step-by-step construction is in the [hands-on tutorial](#tutorial).

## Further reading {#lattice-design-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: the chapters on longitudinal and transverse dynamics, space charge and linac design; this chapter follows its notation.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed.: periodic focusing channels, envelope equations, equipartitioning.
- I. Hofmann's papers on the stability of anisotropic beams: ratios and resonance charts.
- The full bibliography is in [references](#references).
