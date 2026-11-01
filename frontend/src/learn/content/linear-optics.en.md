# Linear optics

A quadrupole can focus in one plane only; in the other plane it necessarily defocuses. Yet when focusing and defocusing quadrupoles alternate, the beam is held together in both planes instead of converging in one and flying apart in the other. How can that be? This chapter answers with a single assumption — **the force is proportional to the displacement**. With it, any piece of beam line becomes a matrix, the shape of the beam is passed on by matrix multiplication, and whether a system is stable depends on a single number: the trace of the period matrix. After this chapter you should be able to explain why alternating-gradient focusing focuses on balance, why focusing that is too strong becomes unstable, what a "matched beam" is, and what AVAS's linear preview computes.

You need the $\beta$, $\gamma$ and $B\rho$ of [Relativistic kinematics](#kinematics) and the Twiss parameters $\beta_x$, $\alpha_x$, $\gamma_x$ and rms emittance of [Phase space and emittance](#phase-space). The material appears all along the story [From the source to the target](#journey-overview): the quadrupoles of the medium-energy section and the "solenoid + cavity" periods of the superconducting section are the periodic systems of this chapter. All numbers use the example beam: protons, $W = 1.527\ \mathrm{MeV}$, $\beta\gamma = 0.05708$, $B\rho = 0.1786\ \mathrm{T\,m}$.

## A force like a spring: Hill's equation {#linear-optics-hill}

### Physical picture

Picture a particle flying at a distance $x$ beside the beam axis. If the transverse force on it is proportional to $x$ and points towards the axis, it behaves like a ball on a spring and oscillates about the axis; only the stiffness of this "spring" changes along the beam line — stiff inside a quadrupole, absent in a drift. Linear optics is the study of such a spring whose strength depends on the position $s$.

Why does linearity matter so much? Because solutions of a linear equation superpose: add the initial coordinates of two particles and their trajectories add as well. So two special trajectories determine the motion of every particle, which is where the transfer matrix of the next section comes from.

### Paraxial equation of motion

Let the particle move along $s$ with transverse momentum $p_x \approx p\,x'$ (paraxial approximation, $x' \ll 1$). From $\dd p_x/\dd t = F_x$ and $\dd/\dd t = v\,\dd/\dd s$,

$$
x'' + \frac{p'}{p}\,x' = \frac{F_x}{p\,v}
$$

The denominator $p\,v$ on the right is worth a moment: the larger the momentum, the harder it is for a force to change the direction; the larger the velocity, the shorter the time the force acts per unit length. On the left, $p'/p = (\beta\gamma)'/(\beta\gamma)$ is non-zero only where the beam is accelerated; it is the [adiabatic damping](#phase-space-liouville) term. For a magnetic field $F_x = -q v B_y$, for an electric field $F_x = qE_x$, so the right-hand side becomes $-B_y/(B\rho)$ and $E_x/(E\rho)$ respectively ([rigidity](#kinematics-rigidity)).

### Hill's equation

If the transverse field is linear in the displacement, for example in a quadrupole $B_y = G\,x$, $B_x = G\,y$, the equations of motion in a non-accelerating section become

$$
x'' + K_x(s)\,x = 0, \qquad y'' + K_y(s)\,y = 0
$$

This is Hill's equation; $K(s)$ is the "spring stiffness" in m$^{-2}$. Where $K > 0$ the element focuses, where $K < 0$ it defocuses. The focusing functions of the elements:

| Element | $K_x$ | $K_y$ | See |
|---|---|---|---|
| Drift | 0 | 0 | [Drift](#drift) |
| Quadrupole (gradient $G$) | $k = G/(B\rho)$ | $-k$ | [Quadrupole](#quadrupole) |
| Sector dipole (field index $N$, bending in $x$) | $(1-N)/\rho^2$ | $N/\rho^2$ | [Dipole](#dipole) |
| Solenoid (in the rotating Larmor frame) | $k_s^2$, $k_s = B/(2B\rho)$ | $k_s^2$ | [Solenoid](#solenoid) |

A number: a quadrupole with $G = 4\ \mathrm{T/m}$ gives the example protons $k = 4/0.1786 = 22.4\ \mathrm{m^{-2}}$. A particle 1 mm off axis crossing 0.1 m of it changes its angle by about $kLx = 2.24\ \mathrm{mrad}$, comparable to the 3.0 mrad rms divergence of the example beam — exactly the scale of "moderate" focusing.

For positive particles a quadrupole with $G > 0$ focuses in $x$ and defocuses in $y$: at $x > 0$, $B_y > 0$ and $\boldsymbol v \times \boldsymbol B$ points to $-x$. A quadrupole cannot focus in both planes: $\nabla\times\boldsymbol B = 0$ in a source-free region requires $\partial B_y/\partial x = \partial B_x/\partial y$, hence $K_y = -K_x$. This is a limit set by Maxwell's equations, not a lack of ingenuity, so the quadrupoles have to alternate, see [Alternating gradient](#linear-optics-alternating).

> **Think about it**: the focusing strength contains $G/(B\rho)$, not the gradient $G$ itself. How differently does the same quadrupole act on 1.5 MeV and on 1 GeV protons?
>
> Answer: $B\rho$ is proportional to the momentum. At 1 GeV a proton has $B\rho = 5.66\ \mathrm{T\,m}$, 32 times the value at 1.527 MeV, so the same quadrupole gives only 1/32 of the $k$. This is why quadrupoles at high energy are longer or have higher gradients.

Keep the approximations of Hill's equation in mind: paraxial motion ($x' \ll 1$), fields linear in the displacement, a small momentum deviation $\delta$ (particles of different momentum have different $K$ — chromatic effects, neglected here), no acceleration inside the section. If any of them fails, linear optics is only an approximation.

## Transfer matrices: two trajectories decide everything {#linear-optics-matrices}

The solution of a linear equation is a linear combination of the initial values, $X(s) = M(s \mid s_0)\,X(s_0)$ with $X = (x, x')\tp$. Let $C(s)$ and $S(s)$ be the two principal solutions with $C(s_0) = 1, C'(s_0) = 0$ and $S(s_0) = 0, S'(s_0) = 1$: $C$ is the trajectory that "starts off axis and parallel" (cosine-like), $S$ the one that "starts on axis at an angle" (sine-like). Any particle is the superposition of these two weighted with $x_0$ and $x_0'$, hence

$$
M = \begin{pmatrix} C & S \\ C' & S' \end{pmatrix}
$$

The matrix of a beam line is the product of the element matrices in order, **with the later element on the left**: $M = M_n \cdots M_2 M_1$.

> **Think about it**: why does the later element stand on the left?
>
> Answer: the matrix acts on the column vector to its right. The particle first passes $M_1$ and becomes $M_1X_0$, then $M_2$ and becomes $M_2(M_1X_0)$. The matrix that acts first stands next to the vector, at the far right.

### Drift (length $L$)

$$
M_{\mathrm{drift}} = \begin{pmatrix} 1 & L \\ 0 & 1 \end{pmatrix}
$$

The angle stays, the position grows linearly with it; see [Drift](#drift).

### Thin lens (focal length $f$)

$$
M_{\mathrm{thin}} = \begin{pmatrix} 1 & 0 \\ -1/f & 1 \end{pmatrix}
$$

The position stays, the angle changes by $-x/f$; $f > 0$ focuses, $f < 0$ defocuses.

### Thick quadrupole (length $L$, $k > 0$, $\phi = \sqrt{k}\,L$)

Inside the quadrupole $K$ is constant; the solutions are cosine and sine in the focusing plane and hyperbolic functions in the defocusing plane:

$$
M_{\mathrm F} = \begin{pmatrix} \cos\phi & \dfrac{\sin\phi}{\sqrt{k}} \\ -\sqrt{k}\sin\phi & \cos\phi \end{pmatrix}, \qquad
M_{\mathrm D} = \begin{pmatrix} \cosh\phi & \dfrac{\sinh\phi}{\sqrt{k}} \\ \sqrt{k}\sinh\phi & \cosh\phi \end{pmatrix}
$$

The equivalent focal length of a thick lens is $1/f = -M_{21}$, i.e. $1/f = \sqrt{k}\sin\phi$ in the focusing plane; for $\phi \ll 1$ it reduces to the thin lens $1/f \approx kL$, the thin-lens approximation. $\phi$ is the "oscillation phase" a particle covers inside the quadrupole: a small $\phi$ means the particle has hardly any time to change its position inside the lens, and the whole lens can be squeezed into one kick.

Example: 1.527 MeV protons ($B\rho$ = 0.17863 T·m) in a quadrupole with $G$ = 20 T/m and $L$ = 0.05 m, $k$ = 111.96 m$^{-2}$, $\phi$ = 0.529:

$$
M_{\mathrm F} = \begin{pmatrix} 0.86328 & 0.04770\ \mathrm m \\ -5.34063\ \mathrm m^{-1} & 0.86328 \end{pmatrix}, \qquad
M_{\mathrm D} = \begin{pmatrix} 1.14325 & 0.05237\ \mathrm m \\ 5.86300\ \mathrm m^{-1} & 1.14325 \end{pmatrix}
$$

The equivalent focal length is 0.1872 m in the focusing plane and $-0.1706$ m in the defocusing plane; the thin-lens approximation gives $\pm 0.1786$ m. The thick lens is weaker than the thin one in the focusing plane and stronger in the defocusing plane: inside a focusing lens the particle moves towards the axis and feels less force in the second half; inside a defocusing lens it moves away and feels more. At $\phi \approx 0.5$ the thin-lens error is already about 5 %, which is common in strongly focused low-energy sections, so AVAS's linear preview uses thick-lens matrices for `quad` (the internal form of the engine's matrix elements is not described in the manual; the engine is the reference).

## Symplecticity: why the phase-space area stays {#linear-optics-symplectic}

Send a small parallelogram of the $(x, x')$ plane through a matrix $M$: the area that comes out is $\lvert\det M\rvert$ times the original. A drift "shears" the patch along $x$ (points with large angles travel further), a thin lens shears it along $x'$, and neither shear changes the area. [Liouville's theorem](#phase-space-liouville) says that the phase-space volume of a Hamiltonian system is conserved; in the linear case this reads: the transfer matrix is **symplectic**,

$$
M\tp J\,M = J, \qquad J = \begin{pmatrix} 0 & 1 \\ -1 & 0 \end{pmatrix}
$$

(in several dimensions $J$ is block diagonal with one 2×2 block per plane). For a 2×2 matrix the symplectic condition is equivalent to $\det M = CS' - SC' = 1$. All the matrices above satisfy it, which is a handy check of hand calculations and programs. Symplecticity also gives a simple inverse,

$$
M^{-1} = \begin{pmatrix} S' & -S \\ -C' & C \end{pmatrix}
$$

**With acceleration**, $\det M = (\beta\gamma)_0/(\beta\gamma)_1 < 1$ in $(x, x')$: the longitudinal momentum grows, the same transverse momentum corresponds to a smaller angle, and the ellipse shrinks — adiabatic damping. This does not contradict Liouville's theorem because $(x, x')$ are not canonical coordinates; in $(x, \beta\gamma\,x')$ the matrix is symplectic again and the normalized emittance is conserved.

## Transport of the σ-matrix and Twiss parameters {#linear-optics-twiss-transport}

### σ-matrix

A single particle is transported by $X_1 = MX_0$, so the second moments of the beam (the [σ-matrix](#phase-space-sigma)) are transported by

$$
\Sigma_1 = M\,\Sigma_0\,M\tp
$$

The σ-matrix describes the rms ellipse, the "shape of the beam"; this rule says that in a linear system the shape of the beam is transported by the same matrix as a single particle, with no need to track every particle. Since $\det\Sigma_1 = (\det M)^2\det\Sigma_0$, the rms emittance is invariant under symplectic transport.

### Twiss parameters

Inserting $\Sigma = \varepsilon\begin{pmatrix} \beta_x & -\alpha_x \\ -\alpha_x & \gamma_x \end{pmatrix}$ gives

$$
\begin{pmatrix} \beta_x \\ \alpha_x \\ \gamma_x \end{pmatrix}_1 =
\begin{pmatrix} C^2 & -2CS & S^2 \\ -CC' & CS' + SC' & -SS' \\ C'^2 & -2C'S' & S'^2 \end{pmatrix}
\begin{pmatrix} \beta_x \\ \alpha_x \\ \gamma_x \end{pmatrix}_0
$$

Two frequent special cases:

- **Drift**: $\beta_x(s) = \beta_0 - 2\alpha_0 s + \gamma_0 s^2$, $\alpha_x(s) = \alpha_0 - \gamma_0 s$, $\gamma_x$ constant. $\beta_x$ is a parabola in $s$ with the waist at $s^* = \alpha_0/\gamma_0$ and $\beta^* = 1/\gamma_0$ there. At a distance $s$ from the waist, $\beta_x = \beta^* + s^2/\beta^*$.
- **Thin lens**: $\beta_x$ unchanged, $\alpha_1 = \alpha_0 + \beta_0/f$. A focusing lens increases $\alpha_x$, turning the beam towards convergence.

For example, a lens with $f$ = 0.5 m at a waist with $\beta_0$ = 1 m: after the lens $\alpha_x = 2$, $\gamma_x = 5$ m$^{-1}$, and the new waist lies 0.4 m downstream with $\beta^*$ = 0.2 m. Note that the new waist is not at the focal point (0.5 m): a beam with emittance is not a bundle of parallel rays, and its waist lies closer to the lens than the geometric focus. The lab below is this example: drag `f` and `L`, watch $\alpha_x$ change sign at the waist while the ellipse area stays the same.

```avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
```

### Differential form and the envelope equation

In a system with focusing function $K(s)$ the Twiss parameters satisfy

$$
\beta_x' = -2\alpha_x, \qquad \alpha_x' = K\beta_x - \gamma_x
$$

With $w = \sqrt{\beta_x}$ this becomes $w'' + K w - 1/w^3 = 0$; multiplied by $\sqrt{\varepsilon}$ it is the equation of the rms envelope $a = \sqrt{\varepsilon\beta_x}$,

$$
a'' + K a - \frac{\varepsilon^2}{a^3} = 0
$$

It differs from the single-particle Hill equation only by the last term. That term always pushes outwards, like the pressure of a gas: the emittance measures the disordered transverse motion of the particles (the "thermal motion" of the beam), and however strong the external focusing, a beam with emittance cannot be squeezed into a point — the smaller the size, the faster this term grows ($\propto 1/a^3$). With space charge the equation gains one more outward term and becomes the envelope equation of the [Space charge](#space-charge) chapter.

## β-function and phase advance {#linear-optics-phase-advance}

The general solution of Hill's equation can be written in Floquet form,

$$
x(s) = \sqrt{J\beta_x(s)}\,\cos\left(\psi(s) + \psi_0\right), \qquad
\psi(s) = \int_{s_0}^{s} \frac{\dd\sigma}{\beta_x(\sigma)}
$$

$J$ is the [Courant–Snyder invariant](#phase-space-twiss) and $\psi$ the betatron phase. The solution looks like a cosine whose amplitude and frequency both vary, and $\beta_x$ plays two roles at once:

- **amplitude**: $\sqrt{J\beta_x}$ is the envelope of one particle's oscillation, $\sqrt{\varepsilon\beta_x}$ the rms size of the beam;
- **local wavelength**: $\dd\psi/\dd s = 1/\beta_x$; the smaller $\beta_x$, the faster the phase advances.

The easiest case is constant focusing $K$: then $\beta_x = 1/\sqrt{K}$ and $\psi = \sqrt{K}\,s$, so $\beta_x$ is just the oscillation wavelength divided by $2\pi$. Stronger focusing gives a shorter wavelength and a thinner beam ($\sqrt{\varepsilon/\sqrt K}$).

With the Twiss parameters at both ends and the phase advance $\Delta\psi$, any transfer matrix can be written

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

(choose the quadrant by the sign of $M_{12}$). The phase advance depends only on the entrance Twiss parameters and the matrix, not on the emittance: it says how far a single particle has turned, not how large the beam is.

## Alternating gradient: why "focus + defocus" focuses {#linear-optics-alternating}

Back to the opening question. Take two thin lenses with focal lengths $f_1$ and $f_2$, a distance $d$ apart. The quantity $-M_{21}$ of $M = M_{\mathrm{thin}}(f_2)\,M_{\mathrm{drift}}(d)\,M_{\mathrm{thin}}(f_1)$ is the power of the pair:

$$
\frac{1}{F} = \frac{1}{f_1} + \frac{1}{f_2} - \frac{d}{f_1 f_2}
$$

With $f_1 = f$ and $f_2 = -f$ (focusing first) the first two terms cancel and what remains is

$$
\frac{1}{F} = \frac{d}{f^2} > 0
$$

The opposite order, $f_1 = -f$ and $f_2 = f$ (defocusing first, the order the other plane sees), gives exactly the same. **Both planes are focused on balance.**

Why? Follow a particle entering parallel at $x_0$ from the axis. If it meets the focusing lens first, it is bent towards the axis and after $d$ is only $x_0(1 - d/f)$ away, so the outward kick of the defocusing lens is smaller than the inward kick it just received. If it meets the defocusing lens first, it is pushed out and reaches the focusing lens at $x_0(1 + d/f)$, so the inward kick is larger than the outward one. The key is that the force is proportional to $x$: **a particle is always further from the axis in the focusing lens than in the defocusing lens**, so focusing wins. The net focusing $1/F = d/f^2$ is second order in the lens strength $1/f$: the stronger each lens, the faster the net focusing of the alternating pair grows. Alternating-gradient synchrotrons use magnets with very large gradients (compared with the small field index of the earlier weak-focusing machines), which is why the scheme is called "strong focusing" (proposed by Christofilos in 1950 without formal publication, and independently by Courant, Livingston and Snyder, published in 1952).

A number: for the example beam a quadrupole with $G = 4\ \mathrm{T/m}$ and $L_q = 0.1\ \mathrm m$ has the thin-lens focal length $f = 1/(kL_q) = 0.447\ \mathrm m$; with the centres of two such quadrupoles $d = 0.4\ \mathrm m$ apart, the pair has $F = f^2/d = 0.50\ \mathrm m$ in both planes.

A physically correct analogy is Kapitza's pendulum: an inverted pendulum whose pivot vibrates quickly up and down stands upright. The vibration gives the angle a small fast jitter that is correlated with the vibrating force — the angle is a little smaller when the force pushes outwards and a little larger when it pulls back — so on average there is a torque towards the vertical. It is the same mechanism as "the particle is further from the axis in the focusing lens". The analogy has limits: Kapitza's pendulum needs a vibration much faster than its natural frequency, whereas a FODO cell may advance the phase by tens of degrees, so only the matrix criterion of the next section decides stability exactly; and an optical convex lens focuses in both directions at once, whereas with quadrupoles one plane sees "focus–defocus" and the other "defocus–focus".

> **Think about it**: what happens if all quadrupoles of a FODO channel have the same polarity?
>
> Answer: $x$ is focused everywhere, which is fine; $y$ is defocused everywhere, the trajectories grow like $\cosh$, and the beam soon hits the pipe in $y$. Alternation is not "better", it is the only way to make both planes stable with quadrupoles (a solenoid focuses both planes, but its strength goes as $(B/B\rho)^2$ and wins only at low energy, see [Solenoid](#solenoid)).

## Periodic systems and stability {#linear-optics-periodic}

Linac focusing usually consists of repeated periods (e.g. FODO: focusing quadrupole – drift – defocusing quadrupole – drift, or "solenoid + cavity"). Let $M$ be the matrix of one period.

### Stability condition

After $n$ periods a particle has the coordinates $M^n X_0$. The eigenvalues $\lambda$ of $M$ satisfy $\lambda^2 - (\Tr M)\lambda + 1 = 0$ (since $\det M = 1$). For $\lvert \Tr M \rvert < 2$ the two eigenvalues are $\ee^{\pm i\mu}$, $M^n$ just keeps "rotating", and the motion is bounded; for $\lvert \Tr M \rvert > 2$ one eigenvalue has modulus larger than 1 and the amplitude grows exponentially. The stability condition is therefore

$$
\left\lvert \tfrac12 \Tr M \right\rvert < 1, \qquad \cos\mu = \tfrac12 \Tr M = \tfrac12\left(M_{11} + M_{22}\right)
$$

$\mu$ is the phase advance per period ($\sigma_0$ at zero current): in each period a particle goes $\mu/360°$ of a turn round its phase-space ellipse. A 2×2 matrix has four numbers, but stability depends on only one combination, the trace — a simplification that comes from symplecticity ($\det M = 1$).

### Periodic Twiss parameters

When the motion is stable, $M$ can be written

$$
M = \begin{pmatrix} \cos\mu + \alpha_x\sin\mu & \beta_x\sin\mu \\ -\gamma_x\sin\mu & \cos\mu - \alpha_x\sin\mu \end{pmatrix}
$$

which gives the periodic solution $\beta_x = M_{12}/\sin\mu$, $\alpha_x = (M_{11} - M_{22})/(2\sin\mu)$, $\gamma_x = -M_{21}/\sin\mu$, with the sign of $\sin\mu$ chosen so that $\beta_x > 0$. They depend only on the period, not on the beam; the ellipse they define returns to itself after one period and is the basis of the matched beam of the next section.

### Thin-lens FODO

For two thin lenses of focal length $\pm f$ a distance $L$ apart (period length $2L$),

$$
\sin\frac{\mu}{2} = \frac{L}{2f}, \qquad
\beta_{\max,\min} = \frac{2L\left(1 \pm \sin(\mu/2)\right)}{\sin\mu}
$$

$\beta_{\max}$ is at the centre of the focusing lens and $\beta_{\min}$ at the centre of the defocusing lens — consistent with the explanation of the previous section: the beam is thickest in the focusing quadrupole and thinnest in the defocusing one. Stability requires $f > L/2$. The smooth approximation estimates the average β as $\bar\beta \approx 2L/\mu$.

### Example and lab

The default cell of the lab is: quadrupoles $L_q = 0.1\ \mathrm m$, $G = \pm 4\ \mathrm{T/m}$, drifts $L_d = 0.3\ \mathrm m$, period 0.8 m, example protons. With AVAS's thick-lens matrices:

- $\tfrac12\Tr M = 0.666$, $\mu = 48.3°$ (the same in both planes); the periodic $\beta_x$ varies between 1.485 m and 0.645 m; the smooth approximation gives $\bar\beta \approx 0.8/0.842 = 0.95\ \mathrm m$.
- Thin-lens estimate ($f = 0.447\ \mathrm m$, lens spacing 0.4 m): $\sin(\mu/2) = 0.448$, $\mu = 53.2°$, β between 1.446 m and 0.552 m. The quadrupoles have $\phi = \sqrt k L_q = 0.47$, and the thin-lens approximation overestimates the phase advance by about 5°.
- Raising the gradient to 6.92 T/m gives $\mu = 90°$; at 9.80 T/m $\tfrac12\lvert\Tr M\rvert$ reaches 1 and the beam becomes unstable (thin lens: 8.93 T/m).

Another example uses short, strong quadrupoles: QF (0.05 m) – drift 0.10 m – QD (0.05 m) – drift 0.10 m, $G$ = 20 T/m. The thick-lens matrices give $\tfrac12\Tr M = 0.726$, $\mu$ = 43.45°, $\beta_x$ = 0.546 m and $\beta_y$ = 0.303 m at the QF entrance; the thin-lens estimate $\sin(\mu/2) = 0.420$, $\mu$ = 49.65°, is about 6° too large; $\mu$ = 90° at about 38.2 T/m and instability at about 54.1 T/m (thin lens: 47.6 T/m).

Check it yourself in the lab below:

1. Start at $G = 4$, confirm $\mu \approx 48°$ and that the periodic $\beta_x$, $\beta_y$ peak in the focusing quadrupoles;
2. raise $G$ step by step and watch $\mu$ pass 90° and the beam become unstable at about 9.8 T/m, the envelope blowing up;
3. compare the thick- and thin-lens phase advances to see the thin-lens error when $\sqrt{k}L$ is not small.

```avas-lab
fodo Lq=0.1 G=4 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.14 I=0 mismatch=1
```

> **Think about it**: focusing is good — why does too much of it make the beam unstable?
>
> Answer: each lens pulls the particle back, and pulling too hard overshoots: the particle crosses the axis and ends up on the other side, further out than before. At $\mu = 180°$ the particle flips to the other side once per period; a little stronger and each flip amplifies the amplitude, $\lvert\Tr M\rvert > 2$. In thin-lens language, the focal length is shorter than half the lens spacing ($f < L/2$), and the beam has crossed the axis and spread out before it reaches the next lens.

The design of FODO cells (choice of phase advance, placement of cavities, smooth approximation) is treated in [Lattice design](#lattice-design).

> Note: stability at zero current is only a necessary condition. Space charge depresses the phase advance per period ($\sigma < \sigma_0$), and for $\sigma_0 > 90°$ intense beams are prone to envelope instabilities and parametric resonances, so high-intensity linacs usually choose $\sigma_0 < 90°$, see [Space charge](#space-charge).

## Matched beam and mismatch {#linear-optics-matched}

If the Twiss parameters of the incoming beam equal the periodic solution, the beam ellipse returns to its shape after every period and the rms size repeats periodically along the line: a **matched beam**. Think of the periodic ellipse as a "contour line" of phase space: each particle moves along it, and however far they move, the overall shape does not change.

If the incoming ellipse has the wrong shape, each particle still moves along its own contour, but the beam ellipse as a whole turns by $\mu$ per period and changes its shape, and the envelope "beats". After half a turn the ellipse coincides with itself again, so the phase of the envelope oscillation advances by $2\mu$ per period, twice the betatron frequency, and the maximum size exceeds the matched one.

The mismatch is usually quantified by the mismatch factor. For two sets of Twiss parameters (beam and periodic solution) define

$$
R = \beta_1\gamma_2 + \beta_2\gamma_1 - 2\alpha_1\alpha_2, \qquad
M_{\mathrm{mis}} = \sqrt{\frac{R + \sqrt{R^2 - 4}}{2}} - 1
$$

A perfect match has $R = 2$ and $M_{\mathrm{mis}} = 0$. If only $\beta$ differs ($\alpha = 0$, $\beta_1 = \lambda\beta_2$), $M_{\mathrm{mis}} = \sqrt{\lambda} - 1$, the relative increase of the maximum rms size. In a linear system a mismatched beam keeps its rms emittance and merely oscillates; in a real line with non-linear forces the particles turn at slightly different rates, the mismatched ellipse slowly "filaments" into a larger one, and after complete filamentation the rms emittance has grown by the factor $R/2$. How to match a beam to a periodic structure is the subject of [Matching](#matching).

Set `mismatch` to 1.3 in the lab: it multiplies the entrance $\beta$ by $1.3^2 = 1.69$ and keeps $\alpha$ at its matched value $-1.57$. Because $\alpha \ne 0$, the mismatch factor is not 0.3 but 0.61 ($R = 2.97$); the envelope maximum grows from 1.91 mm (matched) to 3.05 mm, 60 % more, close to the bound $1 + M_{\mathrm{mis}} = 1.61$ (the bound is reached only where the phase of the envelope oscillation lines up with the β maximum, which the 10 cells do not quite hit); and complete filamentation would raise the emittance by a factor 1.49. With 1 the envelope is strictly periodic.

```avas-lab
fodo Lq=0.1 G=4 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.14 I=0 mismatch=1.3
```

## From 2×2 to 6×6 {#linear-optics-6d}

A real beam has three planes. Putting the transverse and longitudinal planes together, a 6×6 matrix acting on $(x, x', y, y', z, \delta)$ pushes the 6×6 σ-matrix, $\Sigma_1 = M\Sigma_0 M\tp$; this is also what AVAS's linear preview does. For uncoupled elements the matrix is block diagonal:

$$
M = \begin{pmatrix} M_x & 0 & 0 \\ 0 & M_y & 0 \\ 0 & 0 & M_z \end{pmatrix}, \qquad
M_z^{\mathrm{drift}} = \begin{pmatrix} 1 & L/\gamma^2 \\ 0 & 1 \end{pmatrix}
$$

The $L/\gamma^2$ of the longitudinal block comes from $\Delta\beta/\beta = \delta/\gamma^2$ ([kinematics](#kinematics-differential)): a particle with higher momentum is faster and after $L$ is $L\delta/\gamma^2$ ahead of the reference particle. It is the exact counterpart of the transverse drift's $L$ — the longitudinal "angle" is the momentum deviation and the "position" the longitudinal offset; at high energy $1/\gamma^2 \to 0$ and the longitudinal motion almost freezes, see [Drift](#drift-longitudinal).

The off-diagonal blocks describe coupling between the planes:

- a **solenoid** couples $x$ and $y$ (the particles rotate about the axis); only in the rotating Larmor frame does it split into two independent planes, see [Solenoid](#solenoid);
- a **dipole** correlates $x$ with $\delta$ (dispersion: different momenta are bent differently) and, through the path-length difference, $z$ with $x$, see [Dipole](#dipole);
- the bunching of an **RF cavity** is a "lens" in the longitudinal block ($\delta$ changes with $z$), accompanied by transverse defocusing, see [Longitudinal dynamics](#longitudinal).

How AVAS's linear preview treats the elements:

- **drift, quad**: transverse as above, longitudinal block as for a drift;
- **solenoid**: an $x$–$y$ coupled 4×4 block (hard edge, $C = \cos k_sL$, $S = \sin k_sL$);
- **bend, edge**: sector dipole with field index $N$, including the $x$–$\delta$ and $z$–$x$ terms (dispersion); the edge matrix is a thin pole-face rotation;
- **steerer** and diagnostics are identities in the preview (they move the centroid, not the second moments);
- **field maps**: the reference particle's energy and time are integrated in 1 mm steps; the optics is cut into 4 mm slices, each "half a step in the local $B_z$ – a thin kick from the near-axis field gradients (including RF bunching) – another half step", with the damping from the momentum gain;
- **space charge** (optional): every few steps a linear kick of the rms-equivalent uniform ellipsoid (bunched beam) or elliptical cylinder (DC beam).

The accuracy of the preview and the effects it leaves out are given in [In AVAS](#linear-optics-in-avas).

## Common misconceptions {#linear-optics-misconceptions}

- **Two βs**: the Twiss parameter $\beta_x$ is a length (m, i.e. mm/mrad); the relativistic $\beta = v/c$ is dimensionless. Where both appear in this chapter, the one with a subscript is the Twiss parameter.
- **Is the beam thinnest in the focusing quadrupole?** The opposite. In a FODO channel $\beta_{\max}$ is in the focusing and $\beta_{\min}$ in the defocusing quadrupole; a focusing lens makes the beam thinner *after* it.
- **Is the waist at the focus?** Only for a "parallel bundle" with zero emittance. With emittance the waist $s^* = \alpha/\gamma_x$ lies closer to the lens than the geometric focus (example above: focal length 0.5 m, waist at 0.4 m).
- **Is the thin-lens approximation always good enough?** At $\phi = \sqrt{k}L \approx 0.5$ the phase-advance error is already 5°–6° and the gradient at which the cell becomes unstable is underestimated by about 10 %.
- **Does the mismatch factor depend only on $\beta$?** No. A wrong $\alpha$ is just as much a mismatch: in the lab above $\beta$ is only 1.69 times too large, yet because $\alpha \ne 0$ the mismatch factor reaches 0.61.
- **Is the matched beam the smallest beam?** Matching means "the same shape every period", not the smallest size. The periodic β is set by the structure; a thinner beam needs a different structure (stronger focusing, shorter period).
- **Is $\det M = 1$ always true?** Only in canonical coordinates. With acceleration $\det M < 1$ in $(x, x')$ (adiabatic damping): the geometric emittance shrinks, the normalized emittance stays.
- **Is stability at zero current enough?** It is only necessary. Space charge depresses the phase advance and drives resonances; intense beams call for $\sigma_0 < 90°$.
- **Is the phase advance independent of the current?** At zero current it depends only on the structure, not on the emittance; with space charge the single-particle phase advance falls with the current, see [Space charge](#space-charge-tune-depression).

## In AVAS {#linear-optics-in-avas}

### Matrix elements and beam parameters

The lattice elements that act as a whole through their transfer matrix (z-code matrix-model elements) are `quad`, `solenoid`, `bend`, `edge` and `steerer`; `drift` is a t-code field-model element (see [Drift](#drift-in-avas)). Their parameters and sign conventions are described in the element chapters:

```avas-params
drift quad solenoid bend edge steerer
```

The entrance Twiss parameters of the beam are given in `beam.txt`:

```avas-params
beam:twissx beam:twissy
```

> **AVAS convention**: `twissx` and `twissy` are $\alpha$, $\beta$ (mm/mrad) and the **normalized** rms emittance (π·mm·mrad), in this order; the rms size is $\sqrt{\beta\varepsilon_n/(\beta\gamma)}$. The example beam's `twissx -0.65785 0.37743 0.13768` corresponds to rms $x$ = 0.954 mm.

### Linear preview

The linear preview of the visual editor pushes the σ-matrix as described in [From 2×2 to 6×6](#linear-optics-6d) and gives the rms envelope almost instantly while parameters are dragged.

> **AVAS convention**: the linear preview is for quick estimates only. The reference energy is accurate to about 0.01 % (compared with the engine's synchronous particle; the energy in the engine's DataSet is the mean of all macro-particles, which see a different $E_z$ off axis: in the example HWR the mean gain is about 5 % above the reference particle's), rms sizes typically to 1–2 %, and up to 13 % when multi-particle effects are strong; it leaves out the dipole / steering content of field maps, `superposeout` offsets, chromatic and non-linear effects (e.g. the growth of the longitudinal emittance in low-energy cavities), particle losses and `scanphase`. The engine's result is the reference.

> Note: the 6×6 matrices of drift, quad, solenoid, bend and edge in the linear preview are all symplectic (checked numerically in the conjugate pairs $(x, x')$, $(y, y')$, $(z, \delta)$); the only matrices with determinant ≠ 1 are the accelerating slices of field maps.

The labs of this primer (phase space, FODO, solenoid, dipole, RF cavity, space charge) call the matrices and code of the linear preview directly, so their results agree with the preview of the visual editor, and the engine is again the reference.

## Further reading {#linear-optics-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: transverse dynamics, FODO and periodic systems.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific: Hill's equation, Floquet transformation and Courant–Snyder parameters.
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015: transfer matrices and the evolution of the Twiss parameters.
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1.
- K. L. Brown, "A first- and second-order matrix theory for the design of beam transport systems and charged particle spectrometers", SLAC-75: TRANSPORT matrix conventions.
- Read on: [Longitudinal dynamics](#longitudinal), [Space charge](#space-charge), [Quadrupole](#quadrupole), [Lattice design](#lattice-design), [Matching](#matching).
