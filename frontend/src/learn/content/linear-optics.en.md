# Linear optics

When the external forces are proportional to the deviations, the equations of motion are linear: any section of beamline can be described by a transfer matrix, and the second moments and Twiss parameters of the beam are transported by simple rules. Linear optics is the backbone of beamline design, matching and stability analysis, and it is the model behind AVAS's linear envelope preview and the labs of this primer. Starting from Hill's equation, this chapter gives the transfer matrices of a drift, a thin lens and a thick quadrupole, explains symplecticity, the transport of Twiss parameters, the betatron phase advance, the stability condition of periodic systems and the matched beam, and finally presents the 6×6 form used by AVAS's linear preview.

Notation as in [Phase space](#phase-space): $\beta$, $\gamma$ are relativistic factors, $\beta_x$, $\alpha_x$, $\gamma_x$ are Twiss parameters.

## Equation of motion and Hill's equation {#linear-optics-hill}

### Paraxial equation of motion

Let the particle move along $s$ with transverse momentum $p_x \approx p\,x'$. From $\dd p_x/\dd t = F_x$ and $\dd/\dd t = v\,\dd/\dd s$,

$$
x'' + \frac{p'}{p}\,x' = \frac{F_x}{p\,v}
$$

where $p'/p = (\beta\gamma)'/(\beta\gamma)$ is non-zero only in accelerating sections; it is the [adiabatic damping](#phase-space-liouville) term. For a magnetic field $F_x = -q v B_y$ and for an electric field $F_x = qE_x$, so the right-hand side is $-B_y/(B\rho)$ and $E_x/(E\rho)$ respectively ([rigidity](#kinematics-rigidity)).

### Hill's equation

If the transverse fields are linear in the deviations, for example in a quadrupole $B_y = G\,x$, $B_x = G\,y$, the equations of motion in a non-accelerating section become

$$
x'' + K_x(s)\,x = 0, \qquad y'' + K_y(s)\,y = 0
$$

This is Hill's equation. The focusing functions of the elements:

| Element | $K_x$ | $K_y$ | See |
|---|---|---|---|
| drift | 0 | 0 | [Drift](#drift) |
| quadrupole (gradient $G$) | $k = G/(B\rho)$ | $-k$ | [Quadrupole](#quadrupole) |
| sector dipole (field index $N$, bending in $x$) | $(1-N)/\rho^2$ | $N/\rho^2$ | [Dipole](#dipole) |
| solenoid (in the Larmor rotating frame) | $k_s^2$, $k_s = B/(2B\rho)$ | $k_s^2$ | [Solenoid](#solenoid) |

For a positive particle a quadrupole with $G > 0$ focuses in $x$ and defocuses in $y$: at $x > 0$, $B_y > 0$ and $\boldsymbol v \times \boldsymbol B$ points towards $-x$. A quadrupole cannot focus in both planes at once (in a source-free region $\nabla\times\boldsymbol B = 0$ requires $\partial B_y/\partial x = \partial B_x/\partial y$, hence $K_y = -K_x$), so quadrupoles of alternating sign are needed, see [Periodic systems](#linear-optics-periodic).

## Transfer matrices {#linear-optics-matrices}

The solution of a linear equation is a linear combination of the initial values, $X(s) = M(s \mid s_0)\,X(s_0)$ with $X = (x, x')\tp$. If $C(s)$ and $S(s)$ are the two fundamental (cosine-like and sine-like) solutions with $C(s_0) = 1, C'(s_0) = 0$ and $S(s_0) = 0, S'(s_0) = 1$, then

$$
M = \begin{pmatrix} C & S \\ C' & S' \end{pmatrix}
$$

The matrix of a beamline section is the product of the element matrices in order, **with the element passed last on the left**: $M = M_n \cdots M_2 M_1$.

### Drift (length $L$)

$$
M_{\mathrm{drift}} = \begin{pmatrix} 1 & L \\ 0 & 1 \end{pmatrix}
$$

### Thin lens (focal length $f$)

$$
M_{\mathrm{thin}} = \begin{pmatrix} 1 & 0 \\ -1/f & 1 \end{pmatrix}
$$

$f > 0$ focuses, $f < 0$ defocuses.

### Thick quadrupole (length $L$, $k > 0$, $\phi = \sqrt{k}\,L$)

The focusing and the defocusing plane are

$$
M_{\mathrm F} = \begin{pmatrix} \cos\phi & \dfrac{\sin\phi}{\sqrt{k}} \\ -\sqrt{k}\sin\phi & \cos\phi \end{pmatrix}, \qquad
M_{\mathrm D} = \begin{pmatrix} \cosh\phi & \dfrac{\sinh\phi}{\sqrt{k}} \\ \sqrt{k}\sinh\phi & \cosh\phi \end{pmatrix}
$$

The equivalent focal length of a thick lens is given by $1/f = -M_{21}$, i.e. $1/f = \sqrt{k}\sin\phi$ in the focusing plane; for $\phi \ll 1$ it reduces to the thin lens $1/f \approx kL$, the thin-lens approximation.

Example: 1.527 MeV protons ($B\rho$ = 0.17863 T·m), a quadrupole with $G$ = 20 T/m and $L$ = 0.05 m, $k$ = 111.96 m$^{-2}$, $\phi$ = 0.529:

$$
M_{\mathrm F} = \begin{pmatrix} 0.86328 & 0.04770\ \mathrm m \\ -5.34063\ \mathrm m^{-1} & 0.86328 \end{pmatrix}, \qquad
M_{\mathrm D} = \begin{pmatrix} 1.14325 & 0.05237\ \mathrm m \\ 5.86300\ \mathrm m^{-1} & 1.14325 \end{pmatrix}
$$

The equivalent focal length is 0.1872 m in the focusing plane and $-0.1706$ m in the defocusing plane; the thin-lens approximation gives $\pm 0.1786$ m. At $\phi \approx 0.5$ the thin-lens error is already about 5 %, which is common in strongly focusing low-energy sections, so AVAS's matrix elements and the linear preview use thick-lens matrices.

## Symplecticity and the determinant {#linear-optics-symplectic}

The linear transfer matrix of a Hamiltonian system is **symplectic**:

$$
M\tp J\,M = J, \qquad J = \begin{pmatrix} 0 & 1 \\ -1 & 0 \end{pmatrix}
$$

(in several dimensions $J$ is block-diagonal with one 2×2 block per plane). For a 2×2 matrix the symplectic condition is equivalent to $\det M = CS' - SC' = 1$. All matrices above satisfy it, which is a handy check of hand calculations or program output. Symplecticity guarantees conservation of phase-space area — [Liouville's theorem](#phase-space-liouville) in the linear case — and gives the inverse matrix a simple form:

$$
M^{-1} = \begin{pmatrix} S' & -S \\ -C' & C \end{pmatrix}
$$

**With acceleration**, $\det M = (\beta\gamma)_0/(\beta\gamma)_1 < 1$ in $(x, x')$ coordinates — this is adiabatic damping; in $(x, \beta\gamma\,x')$ coordinates the matrix is symplectic again.

> Note: the 6×6 matrices of drift, quad, solenoid, bend and edge in the linear preview are all symplectic (checked numerically with the three conjugate pairs $(x, x')$, $(y, y')$, $(z, \delta)$); only the accelerating slices of field maps have a determinant different from 1.

## Transport of the σ-matrix and Twiss parameters {#linear-optics-twiss-transport}

### σ-matrix

$X_1 = MX_0$ immediately gives the transport rule of the second moments ([σ-matrix](#phase-space-sigma)):

$$
\Sigma_1 = M\,\Sigma_0\,M\tp
$$

Since $\det\Sigma_1 = (\det M)^2\det\Sigma_0$, symplectic transport leaves the rms emittance unchanged.

### Twiss parameters

Substituting $\Sigma = \varepsilon\begin{pmatrix} \beta_x & -\alpha_x \\ -\alpha_x & \gamma_x \end{pmatrix}$ gives

$$
\begin{pmatrix} \beta_x \\ \alpha_x \\ \gamma_x \end{pmatrix}_1 =
\begin{pmatrix} C^2 & -2CS & S^2 \\ -CC' & CS' + SC' & -SS' \\ C'^2 & -2C'S' & S'^2 \end{pmatrix}
\begin{pmatrix} \beta_x \\ \alpha_x \\ \gamma_x \end{pmatrix}_0
$$

Two frequently used special cases:

- **Drift**: $\beta_x(s) = \beta_0 - 2\alpha_0 s + \gamma_0 s^2$, $\alpha_x(s) = \alpha_0 - \gamma_0 s$, $\gamma_x$ unchanged. $\beta_x$ is a parabola in $s$ with the waist at $s^* = \alpha_0/\gamma_0$, where $\beta^* = 1/\gamma_0$. A distance $s$ from the waist, $\beta_x = \beta^* + s^2/\beta^*$.
- **Thin lens**: $\beta_x$ unchanged, $\alpha_1 = \alpha_0 + \beta_0/f$. A focusing lens increases $\alpha_x$, i.e. turns the beam towards convergence.

For example, a lens with $f$ = 0.5 m at a waist with $\beta_0$ = 1 m: after the lens $\alpha_x = 2$ and $\gamma_x = 5$ m$^{-1}$; the new waist lies 0.4 m downstream with $\beta^*$ = 0.2 m, which can be checked in the [phase-space lab](#phase-space-lab).

### Differential form

In a system with focusing function $K(s)$ the Twiss parameters satisfy

$$
\beta_x' = -2\alpha_x, \qquad \alpha_x' = K\beta_x - \gamma_x
$$

With $w = \sqrt{\beta_x}$ this becomes $w'' + K w - 1/w^3 = 0$; multiplying by $\sqrt{\varepsilon}$ gives the equation of the rms envelope $a = \sqrt{\varepsilon\beta_x}$, $a'' + Ka - \varepsilon^2/a^3 = 0$. The last term is the "pressure" of the emittance on the envelope; adding space charge yields the envelope equation of the [Space charge](#space-charge) chapter.

## β-function and betatron phase advance {#linear-optics-phase-advance}

The general solution of Hill's equation can be written (Floquet form) as

$$
x(s) = \sqrt{J\beta_x(s)}\,\cos\left(\psi(s) + \psi_0\right), \qquad
\psi(s) = \int_{s_0}^{s} \frac{\dd\sigma}{\beta_x(\sigma)}
$$

$J$ is the [Courant–Snyder invariant](#phase-space-twiss) and $\psi$ the betatron phase. Hence $\beta_x$ has two meanings: $\sqrt{J\beta_x}$ is the envelope of a single particle's oscillation and $\sqrt{\varepsilon\beta_x}$ is the rms size of the beam; and the smaller $\beta_x$, the faster the phase advances.

In terms of the Twiss parameters at both ends and the phase advance $\Delta\psi$, any transfer matrix can be written as

$$
M = \begin{pmatrix}
\sqrt{\dfrac{\beta_1}{\beta_0}}\left(\cos\Delta\psi + \alpha_0\sin\Delta\psi\right) & \sqrt{\beta_0\beta_1}\,\sin\Delta\psi \\
\dfrac{(\alpha_0 - \alpha_1)\cos\Delta\psi - (1 + \alpha_0\alpha_1)\sin\Delta\psi}{\sqrt{\beta_0\beta_1}} & \sqrt{\dfrac{\beta_0}{\beta_1}}\left(\cos\Delta\psi - \alpha_1\sin\Delta\psi\right)
\end{pmatrix}
$$

Conversely, given $M$ and the entrance Twiss parameters, the phase advance follows from

$$
\tan\Delta\psi = \frac{M_{12}}{\beta_0 M_{11} - \alpha_0 M_{12}}
$$

(choosing the quadrant by the sign of $M_{12}$). The phase advance depends only on the entrance Twiss parameters and the matrix, not on the emittance.

## Periodic systems and stability {#linear-optics-periodic}

Linac focusing usually consists of repeated periodic cells (e.g. FODO: focusing quadrupole – drift – defocusing quadrupole – drift, or a "solenoid + cavity" period). Let $M$ be the matrix of one period.

### Stability condition

After $n$ periods the coordinates are $M^n X_0$. The eigenvalues $\lambda$ of $M$ satisfy $\lambda^2 - (\Tr M)\lambda + 1 = 0$ (because $\det M = 1$). For $\lvert \Tr M \rvert < 2$ the two eigenvalues are $\ee^{\pm i\mu}$ and the motion is bounded; for $\lvert \Tr M \rvert > 2$ one eigenvalue has modulus greater than 1 and the amplitude grows exponentially. The stability condition is therefore

$$
\left\lvert \tfrac12 \Tr M \right\rvert < 1, \qquad \cos\mu = \tfrac12 \Tr M = \tfrac12\left(M_{11} + M_{22}\right)
$$

$\mu$ is the phase advance per period (written $\sigma_0$ at zero current).

### Periodic Twiss parameters

In the stable case $M$ can be written as

$$
M = \begin{pmatrix} \cos\mu + \alpha_x\sin\mu & \beta_x\sin\mu \\ -\gamma_x\sin\mu & \cos\mu - \alpha_x\sin\mu \end{pmatrix}
$$

which gives the periodic solution $\beta_x = M_{12}/\sin\mu$, $\alpha_x = (M_{11} - M_{22})/(2\sin\mu)$, $\gamma_x = -M_{21}/\sin\mu$, with the sign of $\sin\mu$ chosen so that $\beta_x > 0$. These are fixed by the periodic structure alone, independent of the beam.

### Thin-lens FODO

For two thin lenses of focal lengths $\pm f$ a distance $L$ apart (period length $2L$),

$$
\sin\frac{\mu}{2} = \frac{L}{2f}, \qquad
\beta_{\max,\min} = \frac{2L\left(1 \pm \sin(\mu/2)\right)}{\sin\mu}
$$

$\beta_{\max}$ is at the centre of the focusing lens and $\beta_{\min}$ at the centre of the defocusing lens. Stability requires $f > L/2$. The average β can be estimated with the smooth approximation $\bar\beta \approx 2L/\mu$.

### Example

1.527 MeV protons, a period QF (0.05 m) – drift 0.10 m – QD (0.05 m) – drift 0.10 m, $G$ = 20 T/m:

- thick-lens matrices: $\tfrac12\Tr M = 0.726$, $\mu$ = 43.45° (the same in both planes), at the QF entrance $\beta_x$ = 0.546 m and $\beta_y$ = 0.303 m;
- thin-lens estimate ($f$ = 0.1786 m, lens spacing 0.15 m): $\sin(\mu/2) = 0.420$, $\mu$ = 49.65°, about 6° too large;
- raising the gradient to about 38.2 T/m gives $\mu$ = 90°; at about 54.1 T/m $\tfrac12\lvert\Tr M\rvert$ reaches 1 and the beam becomes unstable (the thin-lens approximation gives 47.6 T/m).

Design rules for FODO cells (choice of phase advance, placement of cavities, smooth approximation) are discussed in [Lattice design](#lattice-design).

> Note: stability at zero current is only a necessary condition. Space charge depresses the phase advance per period ($\sigma < \sigma_0$), and for $\sigma_0 > 90°$ intense beams are prone to envelope instabilities and parametric resonances, so high-intensity linacs usually choose $\sigma_0 < 90°$, see [Space charge](#space-charge).

## Matched beam and mismatch {#linear-optics-matched}

If the Twiss parameters of the incoming beam equal the periodic solution, the beam ellipse returns to its original shape after every period and the rms size repeats periodically along the beamline: the beam is **matched**. Otherwise the ellipse rotates by $\mu$ and changes shape from period to period, and the envelope shows a "beating": its phase advances by $2\mu$ per period (the envelope oscillates at twice the betatron frequency) and the maximum size grows.

The degree of mismatch is commonly measured by the mismatch factor. For two sets of Twiss parameters (beam and periodic solution) define

$$
R = \beta_1\gamma_2 + \beta_2\gamma_1 - 2\alpha_1\alpha_2, \qquad
M_{\mathrm{mis}} = \sqrt{\frac{R + \sqrt{R^2 - 4}}{2}} - 1
$$

For a perfect match $R = 2$ and $M_{\mathrm{mis}} = 0$. If only $\beta$ differs ($\alpha = 0$, $\beta_1 = \lambda\beta_2$), $M_{\mathrm{mis}} = \sqrt{\lambda} - 1$, i.e. the relative increase of the maximum rms size. In a linear system the rms emittance of a mismatched beam is unchanged and only the envelope oscillates; in a real beamline with non-linear forces the mismatched ellipse filaments, and after complete filamentation the rms emittance has grown by the factor $R/2$. How to match a beam to a periodic structure is the subject of [Matching](#matching).

## The 6×6 form and AVAS's linear preview {#linear-optics-6d}

AVAS's linear preview treats the transverse and longitudinal planes together: 6×6 matrices acting on $(x, x', y, y', z, \delta)$ push the 6×6 σ-matrix, $\Sigma_1 = M\Sigma_0 M\tp$. For uncoupled elements the matrix is block-diagonal:

$$
M = \begin{pmatrix} M_x & 0 & 0 \\ 0 & M_y & 0 \\ 0 & 0 & M_z \end{pmatrix}, \qquad
M_z^{\mathrm{drift}} = \begin{pmatrix} 1 & L/\gamma^2 \\ 0 & 1 \end{pmatrix}
$$

The $L/\gamma^2$ in the longitudinal block comes from $\Delta\beta/\beta = \delta/\gamma^2$ ([kinematics](#kinematics-differential)): a particle with larger momentum is faster and after a length $L$ is $L\delta/\gamma^2$ ahead of the reference particle. The elements are treated as follows:

- **drift, quad**: transverse as above, longitudinal block as for a drift;
- **solenoid**: an $x$–$y$ coupled 4×4 block (hard edge, $C = \cos k_sL$, $S = \sin k_sL$), see [Solenoid](#solenoid);
- **bend, edge**: the sector dipole includes the field index $N$ and has $x$–$\delta$ and $z$–$x$ correlation terms (dispersion); the edge matrix is a thin pole-face rotation, see [Dipole](#dipole);
- **steerer** and diagnostics are identity matrices in the preview (they change the centroid orbit, not the second moments);
- **field maps**: the reference particle's energy and time are integrated in 1 mm steps; the optics uses 4 mm slices, each "a half step in the local $B_z$ – a thin-lens kick from the near-axis field gradients (including RF bunching) – another half step", with the damping from the momentum gain;
- **space charge** (optional): every few steps a linear kick of the rms-equivalent uniform ellipsoid (bunched beam) or elliptical cylinder (DC beam).

> AVAS convention: the linear preview is for quick estimates only. The reference-particle energy is within about 0.01 %, rms sizes usually within 1–2 %, up to 13 % when multi-particle effects are strong; it does not include dipole / steering content of field maps, `superposeout` offsets, chromatic and non-linear effects (such as the growth of the longitudinal emittance in low-energy cavities), particle losses or `scanphase`. The kernel results are the reference.

## Hands-on lab {#linear-optics-lab}

### Thin lens and drift

Use the phase-space lab to verify the example of [Twiss transport](#linear-optics-twiss-transport): change `f` and `L` and watch the sign of $\alpha_x$ change and the waist position $s^* = \alpha_x/\gamma_x$.

```avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
```

### Stability of a FODO period

The lab gives the phase advance per cell (thick-lens matrix and thin-lens estimate), the stability criterion $\tfrac12\lvert\Tr M\rvert$, the periodic $\beta_x$, $\beta_y$ along one cell and the rms envelope over several cells. Suggestions:

1. Start at $G$ = 20 T/m and confirm $\mu \approx 43°$; increase $G$ step by step and see $\mu$ pass 90° and the beam become unstable at about 54 T/m, with a diverging envelope.
2. Compare the thick-lens and thin-lens phase advances to see the error of the thin-lens approximation when $\sqrt{k}L$ is not small.
3. Set `mismatch` to 1.3 (initial size enlarged by 30 %) and watch the envelope beat; back at 1 the envelope is strictly periodic.

```avas-lab
fodo Lq=0.05 G=20 Ld=0.1 cells=10 W=1.527 mass=938.272 q=1 emit=0.14 I=0 mismatch=1
```

## Further reading {#linear-optics-further}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed.: transverse dynamics, FODO and periodic systems.
- S. Y. Lee, *Accelerator Physics*, 4th ed.: Hill's equation, the Floquet transformation and Courant–Snyder parameters.
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1.
- K. L. Brown, "A first- and second-order matrix theory for the design of beam transport systems and charged particle spectrometers", SLAC-75: the TRANSPORT matrix conventions.
- Continue with [Longitudinal motion](#longitudinal), [Quadrupole](#quadrupole), [Lattice design](#lattice-design).
