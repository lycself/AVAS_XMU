# Solenoid

The field inside a solenoid points almost entirely along the beam, and a magnetic field parallel to the velocity exerts no force at all. So how does it focus? Stranger still, whichever way the current flows in the coil, it always focuses both planes at once and never defocuses.

This chapter shows that a longitudinal field necessarily carries a radial fringe field at its ends; the fringe field first sets the particles rotating about the axis, and the longitudinal field then pulls the rotating particles towards the axis. Focusing is the product of these two steps, which is why it does not depend on the sign of the field and scales as $(B/B\rho)^2$ — and why solenoids are mainly used at low energy. The chapter uses the transfer matrices of [linear optics](#linear-optics) and the focal lengths of the [quadrupole](#quadrupole) chapter; the magnetic rigidity $B\rho$ is in [kinematics](#kinematics).

A 0.2 m, 2 T hard-edge solenoid: it focuses $x$ and $y$ together, and the beam forms an rms waist of 0.50 mm at $z \approx 0.46$ m; the rms $x$ and rms $y$ curves almost coincide (the example beam's Twiss parameters differ slightly between the planes). Compare it with the opening picture of the [quadrupole](#quadrupole) chapter, where one plane shrinks and the other grows.

```avas-beamline
drift 0.1 0.02 0
solenoid 0.2 0.02 0 2
drift 0.5 0.02 0
```

## Role and placement {#solenoid-role}

The strength of a solenoid is that a single element gives the same, axially symmetric focusing in both planes: a round beam stays round. In low-energy sections, where space charge is strong, this symmetric focusing makes it easy to keep a matched round cross-section, and no alternating pairs as with quadrupoles are needed. The price is that the beam rotates about the axis, $x$ and $y$ are coupled, and the field required quickly becomes impractical as the energy rises (see below). Solenoids are therefore found mainly in:

- **Low-energy transport and matching**: in the low-energy beam transport (LEBT) after the ion source, 1–3 solenoids converge the diverging beam and match it into the RFQ (see [the source and LEBT](#journey-source)).
- **The low-energy part of superconducting linacs**: in low-$\beta$ sections of half-wave (HWR), quarter-wave (QWR) or spoke cavities, "cavity–solenoid" periods are common. A superconducting solenoid fits into the same cryostat as the cavities, which keeps the structure compact; it focuses both planes, so one focusing element per period is enough (see [the superconducting section](#journey-sc); AVAS's example project is the start of such a period and comes with the solenoid field map `sol`).
- **Correction**: superconducting solenoids often carry dipole correction coils inside (see [steerer](#steerer)).

## A longitudinal field comes with a radial field {#solenoid-field}

### Field lines converge at the entrance

Think of the solenoid as a "tube" of field lines: far away the field is weak and the lines are sparse; inside the coil the field is strong and the lines must be packed more densely. Field lines can neither appear nor end ($\nabla\cdot\boldsymbol{B} = 0$), so the only way to pack them is to draw them towards the axis — at the entrance, where the field rises, the field has a radial component pointing to the axis; at the exit, where it falls, the lines spread out again and the radial component points outwards.

In formulas: for an axially symmetric field, in cylindrical coordinates $\nabla\cdot\boldsymbol{B} = \frac{1}{r}\frac{\partial (r B_r)}{\partial r} + \frac{\partial B_z}{\partial z} = 0$. Close to the axis $B_z \approx B(z)$ (the on-axis field), and one integration over $r$ gives

$$
B_r(r, z) \approx -\frac{r}{2}\,B'(z),\qquad
B_z(r, z) \approx B(z) - \frac{r^2}{4}\,B''(z),
$$

the second from $\nabla\times\boldsymbol{B} = 0$ (paraxial expansion). The radial field is proportional to the distance from the axis and to the slope of the on-axis field: in the middle of the solenoid $B(z)$ is nearly constant and $B_r \approx 0$; only at the ends is $B_r$ non-zero.

An ideal solenoid of radius $a$, length $\ell$, $n$ turns per unit length and current $I$ has the on-axis field

$$
B(z) = \frac{\mu_0 n I}{2}\left[\frac{z + \ell/2}{\sqrt{(z+\ell/2)^2 + a^2}} - \frac{z - \ell/2}{\sqrt{(z-\ell/2)^2 + a^2}}\right],
$$

which tends to $\mu_0 n I$ at the centre for $\ell \gg a$, is about half of that at the ends, and falls off over a distance of the order of the coil radius.

### Hard-edge model

Replace $B(z)$ by a rectangle of length $L$ and height $B$ (hard edge): $B'$ becomes a $\delta$ function at the ends. At the entrance the particle crosses a thin layer with $\int B_r\,\dd z = -\tfrac{r}{2}B$, at the exit one of opposite sign. Inside the hard-edge model **the transverse field is zero everywhere**; the focusing comes entirely from the combination "radial field at the ends × longitudinal field inside" (next section).

First the cross-section of a hard-edge solenoid (a **schematic** computed from the parameters): the $x$–$y$ cross-section holds only a uniform $B_z$ and no arrows; a longitudinal cut shows the field stopping dead at both ends.

```avas-field
solenoid B=1 L=0.2 R=0.02
```

> **AVAS convention**: the cross-section schematic of a matrix-model solenoid has no transverse field and draws no arrows — that is a property of the hard-edge model, not missing data. A field-map solenoid (such as the example `sol`) has real fringe fields: from $B_r = -\tfrac{r}{2}B'$, with $B_z>0$, $(B_x, B_y)$ points to the axis everywhere at the entrance, where the field rises, and outwards at the exit, where it falls, and the transverse field nearly vanishes in the central cross-section (checked on the example map: at $x = +1$ cm, $B_x$ is about $-0.55$ at $z = 0.12$ m and about $+0.35$ at $z = 0.23$ m, in map units, times `Kb` for T). The direction always follows the formula; the size only roughly, see the next paragraph.

Now a slice of the example map `sol` (**data**). Its on-axis field is not flat-topped: the smoothed peak of about 8.3 (map units) is at $z \approx 0.177$ m with a full width at half maximum of only about 7.4 cm, and most of the 0.35 m map length is slowly decaying fringe. The field rises most steeply at $z \approx 0.144$ m and falls most steeply at $z \approx 0.200$ m. How well does the map obey $B_r = -\tfrac{r}{2}B'$? At $z = 0.12$ m the slope of the smoothed on-axis field gives $-\tfrac{r}{2}B' \approx -0.59$ at $r = 1$ cm, close to the $B_x = -0.55$ read directly from the map. At the steepest point, $z = 0.1435$ m, the formula gives $-1.44$ but the map has only $B_x \approx -0.99$ (and at $z = 0.23$ m, $+0.65$ against $+0.35$): the transverse field in the map is lower and broader than the slope of the noisy on-axis field suggests, so point by point the two agree only in sign and order of magnitude. What the focusing actually depends on agrees well: integrated from the start of the map to the peak, the near-axis gradients of the map give $\int \partial B_x/\partial x\,\dd z \approx -3.98$ and $\int \partial B_y/\partial y\,\dd z \approx -4.15$, against $-\tfrac12 B_{\text{peak}} = -4.14$ that $\nabla\cdot\boldsymbol{B} = 0$ requires for the smoothed peak (the raw on-axis maximum of 10.1 has no counterpart in the transverse field: it is noise). The $x$ and $y$ gradients of the map also differ by 10–25 % point by point ($B_y = -0.42$ at $y = 1$ cm, $z = 0.12$ m): the map is not exactly axially symmetric. In cross-sections on the entrance side $(B_x, B_y)$ points to the axis, on the exit side outwards, and in the central cross-section the transverse field is close to zero. The vertical stripes along $z$ are the noise of the map itself.

```avas-field
map sol
```

## Focusing in two steps: set spinning, then pulled back {#solenoid-busch}

In AVAS's cross-section convention we look from downstream, the beam comes out of the screen ($+z$ towards the viewer), $x$ points right and $y$ up; the azimuth $\theta$ is counted counter-clockwise from the $x$ axis. Take a positive particle entering parallel to the axis at radius $r$, with $B > 0$.

### Step one: the radial field at the entrance sets the particle spinning

At the entrance $B_r < 0$ (towards the axis). The azimuthal component of the Lorentz force is $F_\theta = q(\boldsymbol{v}\times\boldsymbol{B})_\theta = q(v_z B_r - v_r B_z)$; the particle has no radial velocity yet, so $F_\theta = q v_z B_r$. Angular momentum is the easiest way to write it: $\dd(\gamma m r^2\dot\theta)/\dd t = r F_\theta$, and with $\dd t = \dd z/v_z$ and $B_r = -\tfrac{r}{2}B'$,

$$
\frac{\dd}{\dd z}\left(\gamma m r^2 \dot\theta\right) = -\frac{q r^2}{2}\,\frac{\dd B}{\dd z}.
$$

$r$ hardly changes while the particle crosses the entrance; integrating gives $\gamma m r^2\dot\theta = -\tfrac{q}{2} r^2 B$, i.e.

$$
\dot\theta = -\frac{q B}{2\gamma m},\qquad
\frac{\dd\theta}{\dd s} = -\frac{q B}{2p} = -k,\qquad k = \frac{B}{2B\rho}.
$$

The particle rotates about the axis at the **Larmor frequency** (half the cyclotron frequency). $\dot\theta$ does not depend on $r$, so the whole beam turns like a rigid body; for $qB > 0$, $\dot\theta < 0$, which is **clockwise** seen from downstream. In 1 T the example beam has $k = 2.80$ rad/m: a particle at $r = 1$ mm acquires an azimuthal angle of 2.8 mrad (comparable to the beam's rms divergence of 3.0 mrad) and turns by 32° in total through a 0.2 m solenoid.

### Step two: the longitudinal field pulls the spinning particle in

Inside the solenoid the particle carries the azimuthal velocity $v_\theta = r\dot\theta = -q r B/(2\gamma m)$. Now the longitudinal field acts: the radial force is $F_r = q(v_\theta B_z - v_z B_\theta) = q v_\theta B$. With $v_\theta$ inserted,

$$
F_r = -\frac{q^2 B^2}{2\gamma m}\,r .
$$

This force **points to the axis** and is proportional to $r$ — a linear lens. It does not depend on the signs of $q$ or $B$: reverse $B$ and the particle turns the other way, leaving the product $v_\theta B$ unchanged. Half of it keeps the particle on its circular motion (the centrifugal term $\gamma m r\dot\theta^2 = q^2B^2 r/(4\gamma m)$) and the other half pulls the particle towards the axis. Changing $\dd t$ into $\dd s$,

$$
r'' = -k^2\,r,\qquad k^2 = \left(\frac{B}{2B\rho}\right)^2 .
$$

That is the whole origin of solenoid focusing: the fringe field gives an azimuthal velocity proportional to $B$, the longitudinal field turns it into a radial force proportional to $B$ again, and the product is $B^2$.

### Exit: the rotation stops and the particle heads for the axis

At the exit $B_r > 0$ (outwards), the azimuthal force is reversed and exactly removes the azimuthal velocity. This is exact: in an axially symmetric magnetic field the **canonical angular momentum**

$$
P_\theta = \gamma m r^2 \dot\theta + q\,r A_\theta = \gamma m r^2\dot\theta + \frac{q}{2\pi}\,\Psi(r, z)
$$

is conserved, where $\Psi$ is the magnetic flux inside radius $r$; paraxially $A_\theta = rB/2$ and $\Psi = \pi r^2 B$ (Busch's theorem). A particle entering from a field-free region has no angular momentum ($P_\theta = 0$), and after it leaves $B = 0$, so $\dot\theta = 0$ — however far it was pulled in on the way.

So the particle leaves without rotation and keeps only the radial velocity towards the axis that it gained inside: its trajectory lies in a plane containing the axis and heads straight for the axis. Every particle that entered parallel does the same, each in its own plane turned by the same angle — a lens, plus a rigid rotation.

> **Think about it**: what happens to particles "born" inside a magnetic field (for example ionised in the magnetic field of an ECR ion source) once they leave it?
> Answer: then $P_\theta = \tfrac{q}{2\pi}\Psi \neq 0$, and after the field $\dot\theta = P_\theta/(\gamma m r^2) \neq 0$: the beam keeps a rotation. No axially symmetric element can remove it, because $P_\theta$ is conserved in all of them (only non-symmetric optics such as skew-quadrupole channels can convert it, at the price of a flat beam), and in the $x$–$x'$ and $y$–$y'$ planes it appears as a larger projected emittance. This is one of the contributions to the emittance of the beam extracted from such a source (see [the source and LEBT](#journey-source)).

## The Larmor frame and the transfer matrix {#solenoid-matrix}

### Equations of motion

In Cartesian coordinates, keeping linear terms, from $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$ with $B_x = -\tfrac{x}{2}B'$, $B_y = -\tfrac{y}{2}B'$:

$$
x'' = \frac{q}{p}\left(B\,y' + \tfrac12 B'\,y\right),\qquad
y'' = -\frac{q}{p}\left(B\,x' + \tfrac12 B'\,x\right).
$$

The equation for $x$ contains $y$ and $y'$: the two planes are coupled. Define

$$
k(s) = \frac{q B(s)}{2p} = \frac{B(s)}{2B\rho},
$$

and combine the two with the complex coordinate $u = x + \mathrm{i}\,y$: $u'' + 2\mathrm{i}\,k\,u' + \mathrm{i}\,k'\,u = 0$. The previous section said the particle rotates with $\theta' = -k$, so rotate with it: substituting $u = w\,\ee^{-\mathrm{i}\theta(s)}$ with $\theta' = k$ cancels the first-derivative terms,

$$
w'' + k^2(s)\,w = 0 .
$$

In the frame rotating at the Larmor frequency (the **Larmor frame**) the two planes decouple and each is focused with strength $k^2$ — positive whatever the sign of $k$, so a solenoid **always focuses both planes**.

### The 4×4 matrix of a hard-edge solenoid

For a hard-edge solenoid of length $L$ and field $B$, let $C = \cos kL$, $S = \sin kL$. In laboratory coordinates $(x, x', y, y')$ the transfer matrix is

$$
M_{\text{sol}} =
\begin{pmatrix}
C^2 & \dfrac{SC}{k} & SC & \dfrac{S^2}{k}\\[4pt]
-kSC & C^2 & -kS^2 & SC\\[4pt]
-SC & -\dfrac{S^2}{k} & C^2 & \dfrac{SC}{k}\\[4pt]
kS^2 & -SC & -kSC & C^2
\end{pmatrix}.
$$

This is exactly `_solenoid4(k, L)` of AVAS's linear preview ($k = B_z/(2B\rho)$, entrance and exit fringe kicks included). The longitudinal block is that of a drift, $R_{56} = L/\gamma^2$.

It factorises into "focusing in the Larmor frame" times "rotation":

$$
M_{\text{sol}} = R(kL)\,F(kL),\quad
F = \begin{pmatrix} F_2 & 0 \\ 0 & F_2 \end{pmatrix},\quad
F_2 = \begin{pmatrix} C & S/k \\ -kS & C \end{pmatrix},\quad
R(\theta) = \begin{pmatrix} \cos\theta\, I & \sin\theta\, I \\ -\sin\theta\, I & \cos\theta\, I \end{pmatrix},
$$

with $I$ the $2\times 2$ unit matrix. $F$ is the same in both planes, so $R$ and $F$ commute and the rotation can be put before or after the focusing. $R(\theta)$ takes the point $(x, y) = (x_0, 0)$ to $(x_0\cos\theta, -x_0\sin\theta)$: for $qB > 0$, seen from downstream (beam towards the viewer), the beam turns **clockwise** by $\theta = kL$, as found in the previous section.

### Focal length and rotation angle

- Larmor rotation angle: $\theta_L = kL$, in general $\theta_L = \int k\,\dd s = \dfrac{1}{2B\rho}\int B\,\dd s$.
- Thick-lens focal length (in the Larmor frame, the same in both planes):

$$
\frac{1}{f} = k\sin kL .
$$

- Thin-lens approximation ($kL \ll 1$):

$$
\frac{1}{f} \approx k^2 L = \left(\frac{B}{2B\rho}\right)^2 L,\qquad \frac{1}{f} \approx \frac{1}{4(B\rho)^2}\int B^2\,\dd s\ \ (\text{general } B(s)).
$$

The rotation angle is proportional to $\int B\,\dd s$, the focal length is set by $\int B^2\,\dd s$: these two integrals describe a thin solenoid completely. For the example beam with $B = 1$ T and $L = 0.2$ m, $k = 2.80$ m$^{-1}$, $kL = 32.1^\circ$, the thick-lens $f = 0.673$ m and the thin-lens $f = 0.638$ m. Double the field and halve the length: the rotation angle stays, the focal length halves (0.336 m) — the focal length is more sensitive to the field than to the length.

Check this in the solenoid lab (hard-edge matrix model; the initial beam is a round waist with $\beta = 0.4$ mm/mrad). The lab also draws the trajectory of a particle starting parallel at $x = 1$ mm and its projection on the $(x, y)$ plane (seen from downstream):

1. Raise `B` from 0.5 T to 2 T and compare the thick- and thin-lens focal lengths to verify $1/f \propto B^2$; reverse the sign of `B`: the focal length stays, the projected trajectory turns the other way.
2. Keep $BL$ fixed (e.g. `B=2 L=0.1`): the rotation angle stays the same, the focal length halves.
3. Look at the projection: inside the solenoid the particle moves on an arc; after it, on a straight line towards the origin. Adjust `d2` to find the waist downstream of the solenoid.

```avas-lab
solenoid L=0.2 B=1 W=1.527 mass=938.272 q=1 emit=0.1377 d1=0.2 d2=0.6
```

## Why solenoids are mostly used at low energy {#solenoid-energy}

A single quadrupole has $1/f = GL/B\rho$, a solenoid $1/f = (B/2B\rho)^2 L$: halve the rigidity and the quadrupole gets twice as strong, the solenoid four times. For a fixed, technically achievable field, the lower the energy, the better the solenoid does.

The same fact from another angle: for a given focal length, the field needed is proportional to $B\rho$, for both. The table gives the solenoid field and the quadrupole gradient needed for protons with $f = 0.3$ m and $L = 0.2$ m (thin-lens estimates; the quadrupole focuses one plane only):

| Proton kinetic energy | $B\rho$ | solenoid $B$ | quadrupole $G$ | quadrupole pole tip ($R = 2$ cm) |
|---|---|---|---|---|
| 35 keV (LEBT) | 0.0270 T·m | 0.22 T | 0.45 T/m | 0.009 T |
| 1.527 MeV (example) | 0.1786 T·m | 1.46 T | 2.98 T/m | 0.06 T |
| 10 MeV | 0.458 T·m | 3.7 T | 7.6 T/m | 0.15 T |
| 100 MeV | 1.483 T·m | 12.1 T | 24.7 T/m | 0.49 T |

In every row the solenoid needs about 25 times the quadrupole's pole-tip field. Divide the two formulas and $B\rho$ drops out: $B_{\text{sol}}/(GR) = 2\sqrt{fL}/R$, here $2\sqrt{0.3 \times 0.2}/0.02 = 24.5$. The ratio does not depend on energy, but it does depend on the geometry: it grows with the focal length and shrinks with a larger bore. The reason is that a solenoid's field acts on the particle only through the weak, second-order "spin, then pull" mechanism, while a quadrupole pushes directly with a transverse field that grows linearly from the axis to the pole tip. The comparison with a single quadrupole also flatters the quadrupole, which focuses only one plane; against a doublet (see the "Think about it" below) the factor in that example is 17, and for the 0.8 m periods compared in [lattice design](#lattice-design) it is about 9. From tens of keV to a few MeV the 0.2–2 T required can be built — normal-conducting at the low end, superconducting at the high end — and buys symmetric focusing in both planes; at 100 MeV a dozen tesla is no longer practical while the quadrupole's pole tip is still below 0.5 T.

> **Think about it**: quadrupoles need a FODO to focus both planes, and the net focusing of a FODO is also of second order ($1/F = d/f^2$). Is the comparison fair?
> Answer: it is, and the conclusion stands. Two thin lenses $\pm f$ a distance $d$ apart have $1/F = (G L_q/B\rho)^2 d$, which scales as $1/(B\rho)^2$ like the solenoid; the two are equal when $B = 2G\sqrt{L_q d}$. (taking the solenoid as long as one quadrupole, $L_q$). A doublet with $G = 6$ T/m, $L_q = 0.1$ m, $d = 0.3$ m (pole tip 0.12 T at $R = 2$ cm) is equivalent to a 0.1 m solenoid of 2.1 T, 17 times the pole-tip field. The quadrupole buys an enormous "lever" with its transverse gradient; the price is that the envelopes of the two planes oscillate alternately and the beam is not round.

A real solenoid has wide fringe fields. The two beam lines below compare the example map `sol` (`Kb = 0.35`, smoothed peak about 2.9 T, the default of the [tutorial](#tutorial)) with a hard-edge solenoid with **the same integrals**: both $\int B\,\dd z$ and $\int B^2\,\dd z$ equal to those of the map ($L = 0.13$ m, $B = 1.95$ T), centred on the peak of the map. Both give a waist at $z \approx 0.6$ m with an rms size of about 0.8 mm (map: $x$ waist 0.81 mm, $y$ waist 0.75 mm; hard edge: 0.79 mm in both planes): once the two integrals agree, the hard-edge model represents this solenoid well. The Larmor rotation is about 41° in both. The $x$–$y$ difference of the map is its own: with identical Twiss parameters in both planes the map still gives 0.81 / 0.75 mm, because its $x$ and $y$ gradients differ (see [above](#solenoid-field); the linear preview takes only their difference, a skew-quadrupole term, from the map and derives the radial part from $B_z$). The hard-edge values use the raw integrals of the map; with the smoothed $\int B^2\,\dd z$ (3.91 instead of 4.04, see [In AVAS](#solenoid-in-avas)) the equivalent becomes $B = 1.88$ T, $L = 0.136$ m and its waist 0.83 mm — the noise of the map makes the "equivalent" solenoid uncertain by a few per cent.

```avas-beamline
drift 0.05 0.02 0
field 0.35 0.02 0 3 0 0 0 0.35 sol
drift 0.6 0.02 0
```

```avas-beamline
drift 0.162 0.02 0
solenoid 0.13 0.02 0 1.95
drift 0.708 0.02 0
```

## Rotation, coupling and projected emittance {#solenoid-dynamics}

### Round beams and projected emittance

For an axially symmetric (round) beam without initial canonical angular momentum, the rotation $R$ does not change the second moments, and the projected rms emittance at the solenoid exit equals that at the entrance. **Inside** the solenoid, however, the mechanical angular momentum is non-zero: just past the entrance $x'$ gains a term $+k\,y$ (and $y'$ a term $-k\,x$), and the projected emittance

$$
\varepsilon_x^2 = \varepsilon_0^2 + k^2 \langle x^2\rangle\langle y^2\rangle
$$

($\varepsilon_0$ the entrance emittance, $x$ and $y$ assumed uncorrelated) grows temporarily and comes back after the field. For the example beam at the entrance of a 1 T solenoid (rms size 0.95 mm), the projected emittance temporarily becomes about 1.46 times the entrance value. With field-map solenoids, the "bump" of the rms emittance in the middle of a solenoid in DataSet.txt has this origin; it is not a degradation of the beam.

### Flat beams and coupling

The solenoid matrix has non-zero off-diagonal $x$–$y$ blocks. For a round beam this does not matter; for a flat beam (for example after quadrupoles) the solenoid "turns" emittance from one plane into the other, a real growth of the projected emittances. In lattices mixing solenoids and quadrupoles, either pair the solenoids with opposite polarity so that the total rotation cancels, or make the beam round before the quadrupole section.

### Chromatic and spherical aberration

The focusing strength scales as $1/p^2$, so an energy spread changes the focal length by $\Delta f/f \approx 2\delta$. Far from the axis the higher-order terms of the field (the $r^2 B''$ term of $B_z$, the third-order radial field at the ends) cause **spherical aberration**: large-amplitude particles are over-focused and phase space gets an "S"-shaped distortion, a common cause of emittance growth in LEBTs. The matrix model has none of these effects; field-map elements are tracked by the engine and include the non-linearities of the map.

### Space charge and neutralisation

Space charge is strong for low-energy, high-current beams; the axially symmetric focusing of a solenoid matches the space-charge force of a round beam and makes it easy to form a matched round beam. In a real LEBT, ionisation of the residual gas partly neutralises the space charge; AVAS's lattice has a `spacechargecomp` (space-charge compensation) keyword, but the manual does not describe its model, so check it against engine results before use, and treat the degree of neutralisation as a separate uncertainty (see [space charge](#space-charge)).

## Common misconceptions {#solenoid-misconceptions}

- **"The sign of `B` decides between focusing and defocusing"**: a solenoid always focuses; reversing `B` only reverses the rotation.
- **"The longitudinal field exerts no force, so the focusing comes from the fringe field itself"**: the fringe field gives only an azimuthal kick and does not push the particle towards the axis; the real radial force is $v_\theta B_z$ inside. In the hard-edge model the transverse field inside is zero and the focusing is still there.
- **"The exit fringe field cancels the focusing"**: the exit cancels the rotation (the azimuthal velocity), not the radial velocity gained inside.
- **"The emittance bump inside a solenoid means the beam got worse"**: for a round beam it is a projection effect of the rotation and disappears after the field.
- **"Rotation and focal length are set by the same quantity"**: the rotation follows $\int B\,\dd s$, the focal length $\int B^2\,\dd s$.
- **"A solenoid needs less field at low energy"**: for the same focal length and bore a solenoid needs several to a few tens of times the quadrupole's pole-tip field (the factor $2\sqrt{fL}/R$ depends on the geometry, not on the energy); it is used at low energy because the required field can then be built, and it buys symmetric focusing in both planes.

## In AVAS {#solenoid-in-avas}

### Matrix-model solenoid

```text
solenoid L R 0 B
```

```avas-params
solenoid
```

- `L`: length (m), the hard-edge length.
- `R`: aperture radius (m).
- The third parameter is reserved; write 0.
- `B`: axial field (T); its sign sets the direction of rotation and does not affect the focusing (which goes as $B^2$).

> **AVAS convention**: $k = B/(2B\rho)$ and the matrix is the hard-edge form above (`linear_optics._solenoid4`). `solenoid` is a z-code matrix-model element and should also not be the first or last element of a multi-particle lattice.

### Field-map solenoid

The fringe field of a real solenoid is wide (comparable to the coil radius) and in compact cavity–solenoid periods it often overlaps neighbouring elements, so solenoids are usually described by a magnetostatic map:

```text
field 0.35 0.02 0 3 0 0 0 Kb sol
```

The example project `examples/hwr010` contains the solenoid map `sol` (`sol.bsx`, `sol.bsy`, `sol.bsz`), 0.35 m long with a transverse grid of $\pm 0.02$ m. Actual field = map × `Kb` (T). The raw maximum of the on-axis $B_z$ of this map is about 10.1 (map units, T); the 9-point smoothed peak is about 8.3, at $z \approx 0.177$ m. So `Kb = 0.1` corresponds to a peak field of about 0.83 T (raw maximum about 1.0 T).

> Note: the example map itself is noisy; `sol.bsz` jumps point by point between 5.9 and 10.1 in its "flat-top" region (AGENTS.md §6). The vertical stripes along $z$ in the slice heat map are the data, not a reading error. Integrals ($\int B\,\dd z$, $\int B^2\,\dd z$) give more reliable estimates of rotation and focal length than the peak.

Integrating the map gives $\int B\,\dd z \approx 0.729\,\mathrm{Kb}$ T·m and $\int B^2\,\dd z \approx 4.04\,\mathrm{Kb}^2$ T²·m (the noise makes the latter slightly too large; integrating the 9-point smoothed on-axis field gives about 3.91). For the example beam ($B\rho = 0.17863$ T·m):

| `Kb` | peak field | Larmor rotation | thin-lens focal length |
|---|---|---|---|
| 0.1 | ≈ 0.83 T | 11.7° | 3.16 m |
| 0.2 | ≈ 1.7 T | 23.4° | 0.79 m |
| 0.35 | ≈ 2.9 T | 40.9° | 0.26 m |

Field-map elements are tracked by the t-code, fringe fields included; the linear preview derives the radial fringe field from the on-axis $B_z$ by $\nabla\cdot\boldsymbol{B} = 0$, and the map's $B_x$, $B_y$ only supply the divergence-free normal and skew quadrupole content (see the notes in `linear_optics.py`): as shown above, the transverse components of the example map do not agree point by point with $-\tfrac{r}{2}B_z'$, and using them directly would make the period transfer matrix non-symplectic. Map format, storage order and normalisation are in [field maps](#field-maps); how to overlap a solenoid with an RF cavity is in [superpose](#superpose).

### Typical values

| Quantity | Typical range | Note |
|---|---|---|
| Field $B$ | normal-conducting 0.1–0.6 T; superconducting up to several T | short focal lengths at low energy need strong fields |
| Length $L$ | 0.1–0.4 m | solenoids inside superconducting cryostats are usually short |
| Aperture radius $R$ | 0.01–0.05 m | large in LEBTs |
| Rotation $\theta_L$ | a few to tens of degrees | only changes the orientation of the beam, not the rms size of a round beam |

### Numerical examples

The example beam (1.527 MeV protons, $B\rho = 0.17863$ T·m) through a hard-edge solenoid:

| $B$ | $L$ | $k$ | $kL$ | thick-lens $f$ | thin-lens $f$ |
|---|---|---|---|---|---|
| 0.5 T | 0.2 m | 1.400 m⁻¹ | 16.0° | 2.59 m | 2.55 m |
| 1 T | 0.2 m | 2.799 m⁻¹ | 32.1° | 0.673 m | 0.638 m |
| 2 T | 0.1 m | 5.598 m⁻¹ | 32.1° | 0.336 m | 0.319 m |
| 2 T | 0.2 m | 5.598 m⁻¹ | 64.2° | 0.198 m | 0.160 m |

The 4×4 matrix for $B = 1$ T, $L = 0.2$ m is

$$
M =
\begin{pmatrix}
0.718 & 0.161 & 0.450 & 0.101\\
-1.260 & 0.718 & -0.789 & 0.450\\
-0.450 & -0.101 & 0.718 & 0.161\\
0.789 & -0.450 & -1.260 & 0.718
\end{pmatrix},
\qquad \det M = 1 .
$$

For comparison, a typical LEBT solenoid: 35 keV protons ($B\rho = 0.0270$ T·m) through a 0.3 T, 0.2 m solenoid have $kL = 64^\circ$ and a thick-lens focal length of 0.20 m (the thin-lens estimate, 0.16 m, is already clearly off).

### Common problems

- **`Kb` of the field map too large**: the example `sol` peaks at about 8.3 T × `Kb` (raw maximum about 10 T × `Kb`), so `Kb = 1` means 8–10 T; for 1.5 MeV protons the thin-lens focal length is then only about 3 cm, far shorter than the solenoid itself, and the beam is over-focused inside the solenoid and diverges quickly.
- **Comparing matrix and field-map results**: make $\int B^2\,\dd z$ equal (same focal length), not the peak or the length; the rotation $\int B\,\dd z$ then usually differs slightly. The two beam lines above match both integrals.
- **Taking a reversed `B` for defocusing**: see the common misconceptions.

## Further reading {#solenoid-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed. (axially symmetric magnetic fields, Busch's theorem, solenoid lenses).
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed. (transverse focusing in linacs, including solenoid focusing).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (solenoid transfer matrix and coupling).
- Related chapters: [quadrupole](#quadrupole), [field maps](#field-maps), [superpose](#superpose), [RF cavity](#rf-cavity), [space charge](#space-charge); building a cavity–solenoid structure by hand is in the [tutorial](#tutorial); the full bibliography is in [references](#references).
