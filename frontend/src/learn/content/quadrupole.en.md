# Quadrupole

A quadrupole pushes the beam towards the axis in one transverse plane and away from it in the other — it can never focus both planes at once. Yet almost all transverse focusing in an accelerator is done with quadrupoles, and the beam stays together through tens or hundreds of metres of them. How can an element that focuses in one half and defocuses in the other hold a beam together once you put several of them in a row?

This chapter first shows from Maxwell's equations why a static magnetic field can only focus this way, then works out the focal length of a quadrupole, and finally shows why alternating them gives net focusing in both planes. It uses the transfer matrices of the [drift](#drift) and the Hill equation and phase advance of [linear optics](#linear-optics); the beam's Twiss parameters and emittance are in [phase space](#phase-space), the magnetic rigidity $B\rho$ in [kinematics](#kinematics).

First look at what a single quadrupole ($G = 10$ T/m, 0.1 m long) does to the example beam: in $x$ the rms size, about 1.05 mm at the entrance of the quadrupole, is squeezed to a 0.57 mm waist at $z \approx 0.34$ m, while in $y$ it keeps growing and reaches 6.3 mm at 0.65 m.

```avas-beamline
drift 0.05 0.02 0
quad 0.1 0.02 0 10
drift 0.5 0.02 0
```

## What a magnetic lens has to do {#quadrupole-role}

Every particle in the beam has a small transverse position and angle; let them fly through a drift and the beam keeps growing. The example beam (protons at 1.527 MeV, rms size 0.95 mm, rms divergence 3.0 mrad) would reach an rms size of 8.0 mm after a 2.45 m drift (without space charge; the linear preview, which includes the 0.18 mA of the example beam, shows 8.2 mm), and three times rms is already beyond a 2 cm aperture. To bring it back we need something like an optical lens: the further a particle is from the axis, the larger the angle by which it is turned back, ideally **in proportion** to the distance. Only a linear restoring force images all particles with the same focal length and does not distort the ellipse in phase space.

A quadrupole provides exactly such a linear force, and that is why it does most of the transverse focusing in a linac:

- **Transverse focusing**: in the medium- and high-energy sections, quadrupoles form FODO, doublet or triplet periods that keep the beam inside the aperture; normal-conducting drift-tube linacs (DTL) put them inside the drift tubes.
- **Matching**: where two structures meet (the medium-energy beam transport MEBT after an RFQ, transitions between periods), several independently adjustable quadrupoles bring the beam's Twiss parameters to the values the next period needs (see [matching](#matching) and [the MEBT](#journey-mebt)).
- **Beam shaping**: adjusting the size and shape of the spot on a target or beam dump (see [the HEBT](#journey-hebt)).

Quadrupoles work just as well on low-energy beams, but they act oppositely in $x$ and $y$, must be alternated and take more room; right after the ion source and between superconducting cavities a [solenoid](#solenoid), which focuses both planes at once, is often used instead (see [the source and LEBT](#journey-source) and [the superconducting section](#journey-sc)).

## What fields Maxwell's equations allow {#quadrupole-field}

### Only two kinds of linear field

Imagine a long magnet and look at its middle: the field does not depend on $z$ and has no $B_z$. There is no current or iron in the bore, so $\nabla\cdot\boldsymbol{B} = 0$ and $\nabla\times\boldsymbol{B} = 0$. Expand the transverse field near the axis to first order (the constant term is a uniform dipole field, the business of [dipoles](#dipole) and [steerers](#steerer)):

$$
B_x = a\,x + b\,y,\qquad B_y = c\,x + d\,y .
$$

Each of the two equations removes one freedom: $\nabla\cdot\boldsymbol{B} = 0$ gives $a + d = 0$, and the $z$ component of the curl, $\partial B_y/\partial x - \partial B_x/\partial y = 0$, gives $b = c$. Only two fields are left: the **normal quadrupole** with $b = c \equiv G$ and the **skew quadrupole** with $a = -d$ (the same field turned by 45°). The normal quadrupole is

$$
B_x = G\,y,\qquad B_y = G\,x,\qquad G = \frac{\partial B_y}{\partial x} = \frac{\partial B_x}{\partial y},
$$

where $G$ is the **field gradient** in T/m. The field strength $\lvert\boldsymbol{B}\rvert = G\,r$ grows linearly with the distance from the axis and vanishes on it: the reference particle feels nothing, only particles off the axis are pushed.

### Why both planes cannot be focused at once

A particle moving along $+z$ feels $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$; counting only its longitudinal velocity $v$, $F_x = -q v B_y$ and $F_y = q v B_x$. Add up the gradients of the restoring force in the two directions:

$$
\frac{\partial F_x}{\partial x} + \frac{\partial F_y}{\partial y}
= -q v\left(\frac{\partial B_y}{\partial x} - \frac{\partial B_x}{\partial y}\right)
= -q v\,(\nabla\times\boldsymbol{B})_z = 0 .
$$

Focusing means a negative force gradient. The two gradients always add up to zero, so whatever one plane gains in focusing the other must lose — this is not a flaw of the quadrupole's design but a property of any source-free static magnetic field (one form of Earnshaw's theorem). The argument only uses "the force is longitudinal velocity times transverse field"; a [solenoid](#solenoid) works with the longitudinal field $B_z$ and the particle's own transverse velocity and escapes the restriction, at the price of focusing that is of second order.

> **Think about it**: can an electrostatic quadrupole lens (the electric quadrupole field of an RFQ is of this kind) get around this limit?
> Answer: No. In the bore $\nabla\cdot\boldsymbol{E} = 0$, and with a field that does not change along $z$, $\partial E_x/\partial x + \partial E_y/\partial y = 0$: the two gradients of the force $q\boldsymbol{E}$ again add up to zero. An electric quadrupole, like a magnetic one, focuses one plane and defocuses the other (see [the RFQ](#journey-rfq)).

### The shape of the poles

The normal quadrupole field is the gradient $\boldsymbol{B} = -\nabla\Phi_m$ of the magnetic scalar potential $\Phi_m = -G\,x\,y$, or equivalently comes from the vector potential $A_z = \tfrac{G}{2}(y^2 - x^2)$. The surface of an iron pole is close to an equipotential, so the ideal poles are the hyperbolas $x y = \pm R^2/2$: four poles, alternately N and S, at $\pm 45^\circ$ and $\pm 135^\circ$, with $R$ the radius of the inscribed circle (the aperture). In AVAS's cross-section convention (viewed from downstream, the beam coming out of the screen, $x$ to the right, $y$ up), with $G > 0$ the upper-left and lower-right poles are north poles; the field lines leave them and run to the south poles at upper right and lower left.

The field at the pole tip is $B_{\text{pole}} = G\,R$. Iron starts to saturate around 1–1.5 T, so a normal-conducting iron quadrupole roughly obeys $G R \lesssim 1$ T: with a 2 cm aperture radius the gradient is limited to about 50 T/m, and the smaller the aperture, the higher the gradient. The $G = 6$ T/m used below has a pole-tip field of only 0.12 T at $R = 2$ cm.

Here is the cross-section for $G = 6$ T/m (a **schematic** computed from the parameters, not field-map data). Switch to the $x$–$y$ cross-section: the field grows linearly from the centre outwards and the arrows turn with the azimuth. Read it with the right-hand rule: on the positive $x$ axis $B_y > 0$ (up) and the force on a positive charge, $-qvB_y\,\hat{x}$, points to the centre; on the positive $y$ axis $B_x > 0$ (to the right) and the force $+qvB_x\,\hat{y}$ points away from it.

```avas-field
quadrupole G=6 L=0.1 R=0.02
```

### Fringe field and effective length

The gradient of a real quadrupole does not drop to zero at its ends but falls off smoothly over about one aperture radius (the fringe field). Linear optics usually replaces the real distribution by a "hard-edge" model of central gradient $G_0$ and **effective length**

$$
L_{\text{eff}} = \frac{1}{G_0}\int G(s)\,\dd s .
$$

The hard-edge model keeps the integrated focusing strength $\int G\,\dd s$ but ignores the higher-order (non-linear) effects of the fringe field, such as end effects cubic in position. The pole shape and machining errors also add higher multipoles (12-pole, 20-pole …), which are not in the matrix model either.

## Force and equation of motion {#quadrupole-force}

### Size and direction of the force

In a normal quadrupole

$$
F_x = -q v G\,x,\qquad F_y = +q v G\,y .
$$

With $qG > 0$ the force in $x$ points to the axis (focusing) and the force in $y$ away from it (defocusing); with $qG < 0$ the planes swap. So the "focusing plane" depends on the sign of the gradient and on the sign of the particle's charge: the same quadrupole focuses protons in $x$ and H⁻ ions in $y$.

How large is it? The example proton has $v = 1.71\times 10^7$ m/s; with $G = 6$ T/m, at $x = 1$ mm, $B_y = 6$ mT and the force is $1.6\times 10^{-14}$ N. That looks tiny, but so is the transverse momentum of the proton.

### The Hill equation

Turn the force into a change of the trajectory's slope: over $\dd s$ the particle spends a time $\dd s/v$, its transverse momentum changes by $F_x\,\dd s/v$, and divided by the longitudinal momentum $p$ that is the change of the slope $x' = \dd x/\dd s$. Hence

$$
x'' + k\,x = 0,\qquad y'' - k\,y = 0,\qquad
k = \frac{q\,G}{p} = \frac{G}{B\rho}.
$$

$B\rho = p/q$ is the magnetic rigidity, in practical units $B\rho\,[\mathrm{T\,m}] = p\,[\mathrm{MeV}/c]/(299.792458\,Z)$; $k$ is in m$^{-2}$. This is the Hill equation with constant coefficients (see [linear optics](#linear-optics)): in the plane with $k > 0$ the motion is harmonic, in the plane with $k < 0$ it grows exponentially.

The example beam has $p = 53.55$ MeV/$c$ and $B\rho = 0.17863$ T·m, so $G = 6$ T/m gives $k = 33.6$ m$^{-2}$. A 0.1 m long quadrupole of this kind turns a particle at $x = 1$ mm by $kL\,x = 3.4$ mrad, comparable to the beam's rms divergence of 3.0 mrad — a few T/m is already a strong lens for 1.5 MeV protons. $k$ scales as $1/B\rho$: at 100 MeV ($B\rho = 1.48$ T·m) the same $k$ needs about 50 T/m.

## Thick and thin lens {#quadrupole-matrix}

### Thick-lens matrix

Solving the Hill equation through a quadrupole of length $L$, with $\varphi = \sqrt{\lvert k\rvert}\,L$: for $k>0$, focusing in $x$ and defocusing in $y$,

$$
M_x =
\begin{pmatrix}
\cos\varphi & \dfrac{\sin\varphi}{\sqrt{k}}\\[4pt]
-\sqrt{k}\,\sin\varphi & \cos\varphi
\end{pmatrix},
\qquad
M_y =
\begin{pmatrix}
\cosh\varphi & \dfrac{\sinh\varphi}{\sqrt{k}}\\[4pt]
\sqrt{k}\,\sinh\varphi & \cosh\varphi
\end{pmatrix}.
$$

For $k<0$ the planes swap. $\varphi$ is the phase of the oscillation the particle goes through inside the magnet: in the focusing plane it follows a piece of a cosine, in the defocusing plane a piece of a hyperbolic cosine. The longitudinal part is that of a drift of the same length ($R_{56} = L/\gamma^2$), and the full $6\times 6$ matrix has three diagonal blocks:

$$
M_{\text{quad}} = \operatorname{diag}\!\left(M_x,\; M_y,\; \begin{pmatrix} 1 & L/\gamma^2 \\ 0 & 1 \end{pmatrix}\right).
$$

This is the `quad_matrix(L, k, gamma)` of AVAS's linear envelope preview ($k = G/B\rho$, focusing in $x$ when positive).

### Thin-lens approximation

When $\varphi \ll 1$ the particle's position hardly changes while it crosses the magnet; only its angle does. Treating the quadrupole as a thin lens at its centre between two drifts of $L/2$, the lens matrix is

$$
M_{\text{thin}} =
\begin{pmatrix}
1 & 0\\
-1/f & 1
\end{pmatrix},
\qquad
\frac{1}{f} = k\,L = \frac{G L}{B\rho}.
$$

The focal length is $+f$ in $x$ and $-f$ in $y$. It depends only on the integrated gradient $GL$ over the rigidity, which is why the effective-length model keeps $\int G\,\dd s$. The actual thick-lens focal length ($-1/M_{21}$) differs from the thin-lens one by $\mathcal{O}(\varphi^2)$: in the focusing plane $f_x = 1/(\sqrt{k}\sin\varphi)$ is slightly longer, in the defocusing plane $\lvert f_y \rvert = 1/(\sqrt{k}\sinh\varphi)$ slightly shorter.

For the example beam: with $G = 6$ T/m and $L = 0.1$ m, $\varphi = 0.58$, the thin lens has $f = 0.298$ m, the thick lens $f_x = 0.315$ m and $f_y = -0.282$ m; with $G = 10$ T/m, $\varphi = 0.75$, $f = 0.179$ m against $f_x = 0.196$ m, a 10 % difference. In the first picture of this chapter the $x$ waist lies about 0.24 m from the magnet centre, a little beyond the 0.2 m focal length, because the incoming beam is itself diverging ($\alpha_x = -0.66$). More numbers are in the [numerical examples](#quadrupole-in-avas) below.

## Why alternating gives focusing in both planes {#quadrupole-alternating}

### Two lenses together

Take two thin lenses of focal lengths $+f$ and $-f$ a distance $d$ apart. The combination formula of geometrical optics gives

$$
\frac{1}{F} = \frac{1}{f_1} + \frac{1}{f_2} - \frac{d}{f_1 f_2} = \frac{d}{f^2},
$$

positive in both planes, whichever lens comes first — this is the heart of **alternating-gradient** (strong) focusing. The first-order terms $1/f_1 + 1/f_2$ cancel exactly (that is the $k_x + k_y = 0$ of the previous section), and the net focusing that remains is the second-order quantity $d/f^2$.

The physical reason is that the particle is at different distances from the axis in the two lenses. In $x$, a particle that first passes the focusing lens is bent towards the axis; by the time it reaches the defocusing lens it is closer to the axis, so the outward push (proportional to the distance) is smaller than the inward push it just received. In $y$ it is defocused first, so it arrives at the focusing lens further out and the inward force is the larger one. In both orders the particle is pushed inwards when it is far out and outwards when it is close in, and the net effect is focusing.

There is a physically correct analogy: an inverted pendulum whose pivot is shaken rapidly up and down can stand upright (the Kapitza pendulum). The pivot's acceleration alternately adds to gravity and outweighs it, so the effective gravity alternately topples the pendulum and rights it; the pendulum is further off when it is being righted, and on average there is a restoring force. The analogy breaks at "rapidly": the Kapitza pendulum needs a shaking much faster than its swinging, while a FODO channel has a phase advance of several tens of degrees per period, so focal lengths and stability must be computed with matrices, not with averages.

With $f = 0.298$ m ($G = 6$ T/m) and $d = 0.3$ m the combination has $F = 0.30$ m, about as strong as one lens. But the focal points in the two planes are not in the same place: measured from the second lens, the focus in $x$ (focused first) is almost at the second lens, while in $y$ (defocused first) it lies about 0.59 m behind it (thin-lens estimate). That is why a doublet in general does not deliver a round beam. A symmetric triplet ($-f/2$, $+f$, $-f/2$ or the reverse) can image both planes alike and is often used where a round spot is needed.

### The FODO period

Repeat focusing quadrupole QF, drift, defocusing quadrupole QD, drift, and you have a FODO channel. For thin lenses with centres $L_h$ apart, the phase advance per period $\sigma_0$ obeys

$$
\sin\frac{\sigma_0}{2} = \frac{L_h}{2 f}.
$$

Periodic motion is stable when the one-period matrix satisfies $\lvert \Tr M \rvert/2 < 1$, for thin lenses $f > L_h/2$: with lenses that are too strong, the particle "overshoots" within a period and the beam diverges after all. "The stronger the gradient, the better" is therefore wrong. For thick quadrupoles, multiply the thick-lens matrices to get $M$ and take the phase advance from $\cos\sigma_0 = \Tr M/2$.

For the example beam with $L_q = 0.1$ m, $L_d = 0.2$ m ($L_h = 0.3$ m) and $G = 6$ T/m, the thick lenses give $\sigma_0 = 52.7^\circ$ and the thin-lens estimate $60.5^\circ$. The periodic $\beta_x$ is largest at the centre of QF (1.06 m) and smallest at the centre of QD (0.43 m), corresponding to rms sizes of 1.60 mm and 1.02 mm: the beam really is 57 % larger in the focusing quadrupole than in the defocusing one — that is the "pushed inwards when far out" of the previous paragraph in numbers. At a gradient of about 13.5 T/m (same $L_q$, $L_d$) $\lvert\Tr M\rvert/2$ reaches 1 and the period becomes unstable.

The beam line below is four such FODO periods. The rms size in both planes stays below 3.4 mm over the 2.45 m, whereas a drift of the same length would let it grow to about 8 mm. The envelope oscillates unevenly because the Twiss parameters of the example beam are not the matched solution of this period (see [matching](#matching)):

```avas-beamline
drift 0.05 0.02 0
quad 0.1 0.02 0 6
drift 0.2 0.02 0
quad 0.1 0.02 0 -6
drift 0.2 0.02 0
quad 0.1 0.02 0 6
drift 0.2 0.02 0
quad 0.1 0.02 0 -6
drift 0.2 0.02 0
quad 0.1 0.02 0 6
drift 0.2 0.02 0
quad 0.1 0.02 0 -6
drift 0.2 0.02 0
quad 0.1 0.02 0 6
drift 0.2 0.02 0
quad 0.1 0.02 0 -6
drift 0.2 0.02 0
```

Check these statements yourself in the FODO lab (thick-lens matrices, linear model). First reproduce the 52.7° with $G = 6$ T/m and $L_d = 0.2$ m, then:

1. Increase `G` step by step and watch the phase advance and $\lvert\Tr M\rvert/2$; find the edge of the stable region (where the phase advance approaches $180^\circ$).
2. Increase `Ld` to 0.3 m: with $G = 10$ T/m the period is no longer stable and the envelope blows up.
3. Set `mismatch` to 1.3 and watch the envelope beating of a mismatched beam; then add a current `I` and compare the envelopes with and without space charge.

```avas-lab
fodo Lq=0.1 G=6 Ld=0.2 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
```

> **Think about it**: the net focusing $d/f^2$ of a FODO is of second order — why is a quadrupole channel still more economical than a single lens?
> Answer: because $1/f$ itself can be made large. The quadrupole's force comes from a transverse gradient, $1/f = GL/B\rho$, and a pole-tip field below 1 T already gives tens of T/m; the second-order net focusing $(GL/B\rho)^2 d$ is still strong. A solenoid's focusing $\propto (B/B\rho)^2$, by contrast, needs the longitudinal field itself across the whole bore; see the comparison in the [solenoid](#solenoid) chapter.

## The beam in a quadrupole channel {#quadrupole-dynamics}

### The "breathing" envelope

In a FODO period the $\beta$ in $x$ is largest at QF and smallest at QD, and the other way round in $y$, half a period apart. The beam cross-section therefore flattens alternately in the two planes, the signature of quadrupole focusing; it is round only where the two envelopes cross.

### Chromatic effects

The focusing strength is $k \propto 1/p$, so a particle with momentum deviation $\delta$ feels $k(1-\delta)$ (to first order). In straight, dispersion-free sections the chromatic effect on the rms size is usually small, but where the energy spread is large and the focusing strong it causes effective emittance growth. AVAS's linear preview does not include chromatic effects (see the notes in `linear_optics.py`); the engine result is the reference.

### Misalignment, roll and gradient errors

- A transverse offset $d_x$ of the quadrupole: the beam centre is at $-d_x$ from the magnetic centre and gets a kick $\Delta x' = k L\,d_x$ (thin lens), i.e. a dipole "kick". For the example beam, a $G = 6$ T/m, $L = 0.1$ m quadrupole offset by 0.1 mm kicks by 0.34 mrad, the same as a steerer with an integrated field of $6\times 10^{-5}$ T·m. This is the main source of orbit distortion and is corrected with [steerers](#steerer).
- A roll $\theta$ about the $z$ axis: adds a skew quadrupole component (about $\sin 2\theta$ times the strength), couples $x$ and $y$ and makes the projected emittances grow.
- A gradient error $\Delta G/G$: changes the phase advance and causes mismatch and envelope beating (beta beating).

The full error study is in [errors and correction](#errors).

### Space charge

At high current space charge defocuses in both planes and cancels part of the quadrupole focusing, depressing the phase advance per period from $\sigma_0$ to $\sigma$ ("tune depression"). In a quadrupole channel the beam flattens alternately in the two planes and the space-charge force changes with it; to avoid envelope instabilities and parametric resonances the zero-current phase advance is usually kept at $\sigma_0 < 90^\circ$. The lab in the [space charge](#space-charge) chapter shows the effect.

## Common misconceptions {#quadrupole-misconceptions}

- **"$G > 0$ means focusing"**: the focusing plane is set by the sign of $qG$, and there is always one focusing and one defocusing plane. In AVAS, $G > 0$ focuses positive charges in $x$.
- **"Alternating focusing works because the defocusing quadrupole is weaker"**: in a FODO, QF and QD usually have gradients of the same magnitude. The net focusing comes from the beam having different sizes in the two lenses, not from different strengths.
- **"The larger the gradient, the smaller the beam"**: beyond $180^\circ$ phase advance per period the period is unstable; beyond about $90^\circ$ space charge can also drive envelope instabilities.
- **The thin-lens formulas are good enough**: at $\varphi \approx 0.6$ the thin-lens phase advance is already 8° off, more with stronger focusing; design with thick-lens matrices.
- **The effective length equals the iron length**: the effective length is defined by $\int G\,\dd s$ and is usually longer than the iron, by something of the order of an aperture radius.
- **The rms size is the edge of the beam**: the plots show rms sizes; the edge of the beam is several rms out, and losses have to be judged by comparing the aperture with several rms (see [phase space](#phase-space)).

## In AVAS {#quadrupole-in-avas}

### Matrix-model quadrupole

```text
quad L R 0 G
```

```avas-params
quad
```

- `L`: length (m), the effective length of the hard-edge model.
- `R`: aperture radius (m), used for losses.
- The third parameter is reserved; write 0.
- `G`: field gradient (T/m).

> **AVAS convention**: $k = G/(B\rho)$, and **$G > 0$ focuses a positive particle in $x$ and defocuses it in $y$** (checked with the linear preview's `quad_matrix`: $G = 10$ T/m and $L = 0.1$ m give $M_{21} = -5.09\ \mathrm{m^{-1}}$, i.e. focusing in $x$; the manual itself does not state the sign convention, so before relying on it for the engine, check it once with a single quadrupole and a beam offset in $x$). For negative particles (e.g. H⁻) the same $G$ swaps the planes. In a FODO design, QF has positive $G$ and QD negative $G$.

> **AVAS convention**: `quad` is a **z-code matrix-model element**: it acts as a whole through its transfer matrix and no field distribution enters the particle tracking. The manual asks that the first and last elements of a multi-particle simulation not be matrix-model elements; add a very short drift at both ends (see [drift](#drift)). How space charge is treated inside matrix-model elements is not described in the manual; the engine result is the reference.

Example: one FODO period (numbers below):

```text
quad  0.1 0.02 0  6
drift 0.2 0.02 0
quad  0.1 0.02 0 -6
drift 0.2 0.02 0
```

### Field-map quadrupole

When fringe fields or measured fields matter, use a magnetostatic field-map element (`field`, type 3) instead:

```text
field 0.1 0.02 0 3 0 0 0 1.0 qmap
```

The field-map files are `qmap.bsx`, `qmap.bsy` and `qmap.bsz` (whether the engine accepts a set without `.bsz` is not described in the manual; the linear preview then takes $B_z = 0$), in T, scaled by `Kb` in the engine: actual field = map × `Kb`. **The sign of `Kb` sets the polarity**: flipping it swaps the focusing and defocusing planes. The parameters a static field does not use (frequency, phase, `Ke`) are written as 0, as the manual says. Field-map elements are tracked by the t-code and naturally include the fringe fields and the higher-order content of the map. Storage order and reading of maps are in the [field maps](#field-maps) chapter.

### Errors

Position, roll and gradient errors of quadrupoles are given with the "magnetostatic element error" command (acting on the N elements below it):

```avas-params
err_quad_ncpl_stat
```

`dx`, `dy` are transverse offsets (mm), `dφx`, `dφy`, `dφz` rotations about the three axes (°), `dG` the relative gradient error (%), `dz` the longitudinal offset (mm). The complete error-study workflow is in [errors and correction](#errors).

### Typical values

| Quantity | Typical range (low/medium-energy proton linac) | Note |
|---|---|---|
| Gradient $G$ | a few to tens of T/m | normal-conducting electromagnets; permanent-magnet quadrupoles can go higher |
| Effective length $L$ | 0.05–0.3 m | short and strong at low energy, longer at high energy |
| Aperture radius $R$ | 0.01–0.04 m | set by beam size, errors and loss budget |
| Pole-tip field $G R$ | $\lesssim 1$ T (iron) | beyond it the iron saturates and the field quality degrades |
| Phase advance per period $\sigma_0$ | 40°–90° (zero current) | avoid parametric resonances above 90°, see [space charge](#space-charge) |

### Numerical examples

Example beam: protons, $W = 1.527$ MeV, $p = 53.55$ MeV/$c$, $B\rho = 0.17863$ T·m.

| $G$ | $L$ | $k$ | $\varphi$ | thin-lens $f$ | thick-lens $f_x$ | thick-lens $f_y$ |
|---|---|---|---|---|---|---|
| 4 T/m | 0.1 m | 22.39 m⁻² | 0.473 | 0.447 m | 0.464 m | −0.430 m |
| 6 T/m | 0.1 m | 33.59 m⁻² | 0.580 | 0.298 m | 0.315 m | −0.282 m |
| 10 T/m | 0.1 m | 55.98 m⁻² | 0.748 | 0.179 m | 0.196 m | −0.163 m |
| 20 T/m | 0.05 m | 111.96 m⁻² | 0.529 | 0.179 m | 0.187 m | −0.171 m |

As $\varphi$ approaches 1 the thin-lens error reaches the 10 % level. The last two rows have the same integrated gradient $GL$ and thus the same thin-lens focal length, but different thick-lens focal lengths: the short, strong magnet is closer to a thin lens.

FODO period with $L_q = 0.1$ m, $L_d = 0.2$ m (lens centres $L_h = 0.3$ m apart):

| $G$ | thick-lens phase advance $\sigma_0$ | thin-lens estimate | $\Tr M/2$ |
|---|---|---|---|
| 6 T/m | 52.7° | 60.5° | 0.606 |
| 8 T/m | 72.6° | 84.4° | 0.299 |
| 10 T/m | 95.4° | 114.2° | −0.094 |
| 10 T/m, $L_d = 0.3$ m | unstable | unstable | −1.083 |

At $G = 6$ T/m the periodic $\beta_x$ varies between 0.43 and 1.06 m, i.e. rms sizes of 1.0–1.6 mm for the example emittance ($\varepsilon_{n,x} = 0.1377\ \pi$·mm·mrad).

### Common problems

- A gradient with the wrong sign: the intended FODO becomes "FFOO" and the beam diverges quickly in one plane and is lost.
- Copying TraceWin's mm units: length and aperture of a TraceWin `QUAD` are in mm, in AVAS in m.
- A matrix-model quadrupole as the first or last element of the lattice (see the AVAS convention above).
- Comparing matrix-model and field-map results: the map contains fringe fields and its effective length is not necessarily the map length; make $\int G\,\dd s$ equal before comparing.

## Further reading {#quadrupole-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed. (chapter 7: quadrupole focusing, FODO and periodic solutions).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (multipole expansion, quadrupole design and fringe fields).
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific (2019) (Hill equation, strong focusing and stability).
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1–48 (the classic paper on alternating-gradient focusing).
- Related chapters: [linear optics](#linear-optics), [solenoid](#solenoid), [matching](#matching), [lattice design](#lattice-design), [errors and correction](#errors); the full bibliography is in [references](#references).
