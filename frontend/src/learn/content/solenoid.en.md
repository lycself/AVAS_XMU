# Solenoid

A solenoid produces a longitudinal magnetic field $B_z$ along the beam. It focuses in both transverse planes **at once**, and its action on a round beam is axially symmetric; the price is that the beam rotates about the axis and the $x$ and $y$ planes are coupled. The focusing strength of a solenoid is proportional to $(B/B\rho)^2$, which makes it especially suited for low-energy, low-rigidity beams: focusing in low-energy beam transport lines (LEBT) and in low-$\beta$ superconducting cavity sections is mostly done with solenoids. AVAS's example project comes with a solenoid field map, `sol`.

This chapter uses the transfer matrices of [linear optics](#linear-optics) and the focal-length concept of the [quadrupole](#quadrupole) chapter.

## Role and placement {#solenoid-role}

- **Low-energy transport and matching**: in the LEBT after the ion source, 1–3 solenoids converge the diverging beam and match it into the downstream structure.
- **Low-energy section of superconducting linacs**: in low-$\beta$ sections of half-wave (HWR), quarter-wave (QWR) or spoke cavities, "cavity–solenoid" periods are common. Superconducting solenoids fit into the same cryostat, which keeps the structure compact; they focus both planes at once, so a period needs only one focusing element.
- **Correction**: superconducting solenoids often carry dipole corrector windings (see [steerer](#steerer)).

## Physics and field {#solenoid-field}

### Field of a finite solenoid

In the bore, close to the axis, an axially symmetric static field can be expanded from the on-axis field $B(z) \equiv B_z(0, z)$ (paraxial expansion, from $\nabla\cdot\boldsymbol{B}=0$ and $\nabla\times\boldsymbol{B}=0$):

$$
B_z(r, z) \approx B(z) - \frac{r^2}{4}\,B''(z),\qquad
B_r(r, z) \approx -\frac{r}{2}\,B'(z).
$$

In the middle of the solenoid $B(z)$ is nearly constant and $B_r \approx 0$; at the ends $B(z)$ falls off and produces a **radial fringe field** proportional to the radius. For an ideal solenoid of radius $a$, length $\ell$, $n$ turns per unit length and current $I$, the on-axis field is

$$
B(z) = \frac{\mu_0 n I}{2}\left[\frac{z + \ell/2}{\sqrt{(z+\ell/2)^2 + a^2}} - \frac{z - \ell/2}{\sqrt{(z-\ell/2)^2 + a^2}}\right];
$$

at the centre it tends to $\mu_0 n I$ for $\ell \gg a$, at the ends it is about half the central value, and the fringe region is of the order of the coil radius.

### Hard-edge model

Replacing $B(z)$ by a rectangular profile of length $L$ and height $B$ (hard edge) turns the radial field into a $\delta$ function at the ends: at the entrance the particle crosses a thin layer with $\int B_r\,\dd z = -\tfrac{r}{2}B$, at the exit the sign is reversed. Inside the hard-edge model the **transverse field is zero everywhere**; the focusing comes entirely from the combination "radial edge field × rotation inside the body" (next section).

> AVAS convention: the cross-section schematic of a matrix-model solenoid shows no transverse field and draws no grid arrows — this is a property of the hard-edge model, not missing data. A field-map solenoid (such as the example `sol`) has a real fringe field: from $B_r = -\tfrac{r}{2}B'$, for $B_z>0$ the transverse field $(B_x, B_y)$ points to the axis everywhere at the entrance end, where the field rises, and outwards at the exit end, where it falls; in the central cross-section the transverse field vanishes (checked with the example map: at $x = +1$ cm, $B_x$ is about $-0.55$ at $z = 0.12$ m and about $+0.35$ at $z = 0.23$ m, in map units, times `Kb` for T).

### Busch's theorem and canonical angular momentum

In an axially symmetric magnetic field the canonical angular momentum

$$
P_\theta = \gamma m r^2 \dot\theta + q\,r A_\theta = \gamma m r^2\dot\theta + \frac{q}{2\pi}\,\Psi(r, z)
$$

is conserved, where $\Psi$ is the magnetic flux inside radius $r$; paraxially $A_\theta = rB/2$ and $\Psi = \pi r^2 B$. If the particle has no angular momentum in the field-free region ($P_\theta = 0$), then inside the field

$$
\dot\theta = -\frac{q B}{2\gamma m},\qquad
\frac{\dd\theta}{\dd s} = -\frac{q B}{2p} = -k,
$$

that is, the particle rotates about the axis at the **Larmor frequency** (half the cyclotron frequency). This is Busch's theorem. The rotation rate does not depend on $r$, so the whole beam turns like a rigid body; after the solenoid the rotation stops ($\dot\theta=0$ where $B=0$) and only a net rotation angle remains.

## Equations of motion and transfer matrix {#solenoid-matrix}

### Equations of motion

From $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$, kept to linear terms:

$$
x'' = \frac{q}{p}\left(B\,y' + \tfrac12 B'\,y\right),\qquad
y'' = -\frac{q}{p}\left(B\,x' + \tfrac12 B'\,x\right).
$$

Define

$$
k(s) = \frac{q B(s)}{2p} = \frac{B(s)}{2B\rho};
$$

with the complex coordinate $u = x + \mathrm{i}\,y$ the equations read $u'' + 2\mathrm{i}\,k\,u' + \mathrm{i}\,k'\,u = 0$. Substituting $u = w\,\ee^{-\mathrm{i}\theta(s)}$ with $\theta' = k$ cancels the first-derivative terms:

$$
w'' + k^2(s)\,w = 0 .
$$

In the frame rotating at the Larmor frequency (the **Larmor frame**) the two planes decouple and each is focused with strength $k^2$ — whatever the sign of $k$, $k^2 > 0$, so a solenoid **always focuses both planes**.

### 4×4 matrix of a hard-edge solenoid

For a hard-edge solenoid of length $L$ and field $B$, let $C = \cos kL$, $S = \sin kL$. The transfer matrix in laboratory coordinates $(x, x', y, y')$ is

$$
M_{\text{sol}} =
\begin{pmatrix}
C^2 & \dfrac{SC}{k} & SC & \dfrac{S^2}{k}\\[4pt]
-kSC & C^2 & -kS^2 & SC\\[4pt]
-SC & -\dfrac{S^2}{k} & C^2 & \dfrac{SC}{k}\\[4pt]
kS^2 & -SC & -kSC & C^2
\end{pmatrix}.
$$

This is identical to `_solenoid4(k, L)` of AVAS's linear preview ($k = B_z/(2B\rho)$, entrance and exit edge effects included). The longitudinal block is that of a drift, $R_{56} = L/\gamma^2$.

It factorizes into "focusing in the Larmor frame" times "rotation":

$$
M_{\text{sol}} = R(kL)\,F(kL),\quad
F = \begin{pmatrix} F_2 & 0 \\ 0 & F_2 \end{pmatrix},\quad
F_2 = \begin{pmatrix} C & S/k \\ -kS & C \end{pmatrix},\quad
R(\theta) = \begin{pmatrix} \cos\theta\, I & \sin\theta\, I \\ -\sin\theta\, I & \cos\theta\, I \end{pmatrix},
$$

with $I$ the $2\times 2$ unit matrix. $R(\theta)$ takes the point $(x, y) = (x_0, 0)$ to $(x_0\cos\theta, -x_0\sin\theta)$: for $qB > 0$, seen from downstream (beam coming towards the viewer), the beam turns **clockwise** by $\theta = kL$.

### Focal length and rotation angle

- Larmor rotation angle: $\theta_L = kL$, in general $\theta_L = \int k\,\dd s = \dfrac{1}{2B\rho}\int B\,\dd s$.
- Thick-lens focal length (in the Larmor frame, the same in both planes):

$$
\frac{1}{f} = k\sin kL .
$$

- Thin-lens approximation ($kL \ll 1$):

$$
\frac{1}{f} \approx k^2 L = \left(\frac{B}{2B\rho}\right)^2 L,\qquad\text{in general}\quad \frac{1}{f} \approx \frac{1}{4(B\rho)^2}\int B^2\,\dd s .
$$

Compare with a quadrupole: there $1/f = G L/B\rho \propto 1/B\rho$, for a solenoid $1/f \propto 1/(B\rho)^2$. The lower the rigidity, the more "economical" the solenoid; at high rigidity the field needed for the same focal length grows quickly — which is why solenoids are used mainly at low energy.

### Round beams and projected emittance

For an axially symmetric (round) beam without initial canonical angular momentum, the rotation $R$ does not change the second moments, and the projected rms emittance at the solenoid exit equals that at the entrance. **Inside** the solenoid, however, the mechanical angular momentum is non-zero, just past the entrance $x'$ contains an extra term $+k\,y$ ($y'$ an extra $-k\,x$), and the projected emittance

$$
\varepsilon_x^2 = \varepsilon_0^2 + k^2 \langle x^2\rangle\langle y^2\rangle
$$

($\varepsilon_0$ the entrance emittance, $x$ and $y$ assumed uncorrelated) grows temporarily and recovers after the exit. With a field-map solenoid, the "bump" of the rms emittance in the middle of the solenoid in DataSet.txt has this origin; it is not a loss of beam quality. For non-round beams, or beams with initial coupling, a solenoid causes real projected emittance growth.

## In AVAS {#solenoid-avas}

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
- `B`: axial field (T); its sign sets the direction of rotation and does not affect the focusing (which is proportional to $B^2$).

> AVAS convention: $k = B/(2B\rho)$, with the hard-edge matrix above (`linear_optics._solenoid4`). `solenoid` is a z-code matrix-model element and, like the others, should not be the first or last element of a multi-particle lattice.

### Field-map solenoid

The fringe field of a real solenoid is wide (comparable to the coil radius) and in compact cavity–solenoid periods often overlaps neighbouring elements, so solenoids are usually described by a static magnetic field map:

```text
field 0.35 0.02 0 3 0 0 0 Kb sol
```

The example project `examples/hwr010` comes with the solenoid map `sol` (`sol.bsx`, `sol.bsy`, `sol.bsz`), 0.35 m long, transverse grid $\pm 0.02$ m. Actual field = map × `Kb` (T). The raw maximum of the on-axis $B_z$ of this map is about 10.1 (map units, T) and the 9-point smoothed peak about 8.3, near $z \approx 0.177$ m, so `Kb = 0.1` corresponds to a peak field of about 0.83 T (raw maximum about 1.0 T).

> Note: the example map itself is noisy; in its "flat top" `sol.bsz` jumps point by point between 5.9 and 10.1 (AGENTS.md §6). The vertical stripes along $z$ in the slice heat map are the data, not a reading error. Estimate rotation and focal length from integrals ($\int B\,\dd z$, $\int B^2\,\dd z$) rather than from the peak.

Integrating the map gives $\int B\,\dd z \approx 0.729\,\mathrm{Kb}$ T·m and $\int B^2\,\dd z \approx 4.04\,\mathrm{Kb}^2$ T²·m (the noise makes the latter slightly too large). For the example beam ($B\rho = 0.17863$ T·m):

| `Kb` | peak field | Larmor angle | thin-lens focal length |
|---|---|---|---|
| 0.1 | ≈ 0.83 T | 11.7° | 3.16 m |
| 0.2 | ≈ 1.7 T | 23.4° | 0.79 m |

Field-map elements are tracked with the t-code and include the fringe field; the linear preview computes the edge effects from the near-axis gradients of the map, radial components included (see the notes in `linear_optics.py`). Map format, storage order and normalisation are covered in [field maps](#field-maps), placing a solenoid on top of a cavity in [superpose](#superpose).

## Parameters and typical values {#solenoid-params}

| Quantity | Typical range | Remark |
|---|---|---|
| field $B$ | room temperature 0.1–0.6 T; superconducting up to several T | short focal lengths at low energy need strong fields |
| length $L$ | 0.1–0.4 m | solenoids inside superconducting cryostats are usually short |
| aperture radius $R$ | 0.01–0.05 m | larger apertures in LEBTs |
| rotation $\theta_L$ | a few to a few tens of degrees | changes only the orientation of the beam, not the rms size of a round beam |

### Worked example

The example beam (protons at 1.527 MeV, $B\rho = 0.17863$ T·m) through a hard-edge solenoid:

| $B$ | $L$ | $k$ | $kL$ | thick-lens $f$ | thin-lens $f$ |
|---|---|---|---|---|---|
| 0.5 T | 0.2 m | 1.400 m⁻¹ | 16.0° | 2.59 m | 2.55 m |
| 1 T | 0.2 m | 2.799 m⁻¹ | 32.1° | 0.673 m | 0.638 m |
| 2 T | 0.1 m | 5.598 m⁻¹ | 32.1° | 0.336 m | 0.319 m |

For $B = 1$ T and $L = 0.2$ m the 4×4 matrix is

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

Doubling the field and halving the length keeps the rotation and halves the focal length — the focal length is more sensitive to the field than to the length.

## Dynamics and common problems {#solenoid-dynamics}

### Coupling and rotation

The solenoid matrix has non-zero off-diagonal blocks between $x$ and $y$. For a round beam this does not matter; for a flat beam (for example after quadrupoles) a solenoid "turns" emittance from one plane into the other, and the projected emittances grow. In lattices mixing solenoids and quadrupoles, either pair the solenoids with opposite polarity to cancel the net rotation or make the beam round before the quadrupole section.

### Chromatic and spherical aberrations

The focusing strength is $\propto 1/p^2$; the energy spread changes the focal length by $\Delta f/f \approx 2\delta$. Far from the axis the higher-order terms of the field (the $r^2 B''$ term of $B_z$, the third-order radial field at the ends) cause **spherical aberration**: large-amplitude particles are over-focused and the phase space takes an "S" shape, a common source of emittance growth in LEBTs. The matrix model contains none of these effects; field-map elements are tracked by the kernel and include the non-linearities present in the map.

### Space charge and neutralisation

Space charge of low-energy, high-current beams is strong; the axially symmetric focusing of a solenoid matches the space-charge force of a round beam and makes a matched round beam easy to obtain. In a real LEBT, ionisation of the residual gas partly neutralises the space charge; AVAS's lattice has a `spacechargecomp` (space-charge compensation) keyword, but the manual does not describe its model, so check it against kernel results before use, and treat the degree of neutralisation as an uncertainty to be assessed separately (see [space charge](#space-charge)).

### Common problems

- **Thinking the sign of `B` selects focusing/defocusing**: a solenoid always focuses; changing the sign of `B` only reverses the rotation.
- **Too large a map `Kb`**: the example `sol` peaks at about 8.3 T × `Kb` (raw maximum about 10 T × `Kb`), so `Kb = 1` means 8–10 T; for 1.5 MeV protons the thin-lens estimate of the focal length is only about 3 cm, far shorter than the solenoid itself, and the beam is over-focused inside the solenoid and diverges quickly.
- **Comparing matrix and field-map results**: make $\int B^2\,\dd z$ equal (same focal length), not the peak or the length; the rotation $\int B\,\dd z$ then generally differs slightly.

## Hands-on {#solenoid-lab}

First the cross-section of a matrix-model solenoid: inside the hard edge the transverse field is zero and only the direction of the longitudinal field is marked (**schematic**).

```avas-field
solenoid B=1 L=0.2 R=0.02
```

Then slices of the example field map `sol` (**data**): in a longitudinal cut look at the flat top of $B_z$ and its fall-off at both ends; in a cross-section at the entrance end $(B_x, B_y)$ points to the axis, at the exit end outwards, and in the central cross-section the transverse field is close to zero. The vertical stripes along $z$ come from the noise of the map itself.

```avas-field
map sol
```

Finally the solenoid lab shows focal length, rotation and the 4×4 matrix, with the example beam and a drift before and after the solenoid:

1. Increase `B` from 0.5 T to 2 T, compare thick- and thin-lens focal lengths and check $1/f \propto B^2$.
2. Keep $BL$ fixed (for example `B=2 L=0.1`): the rotation stays and the focal length halves.
3. Adjust `d2` to find the waist downstream of the solenoid.

```avas-lab
solenoid L=0.2 B=1 W=1.527 mass=938.272 q=1 emit=0.1377 d1=0.2 d2=0.6
```

## Further reading {#solenoid-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed. (chapter 3: axially symmetric magnetic fields, Busch's theorem, solenoid lenses).
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed. (chapter 7: solenoid focusing; focusing structures of low-$\beta$ superconducting linacs).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (solenoid transfer matrix and coupling).
- Related chapters: [quadrupole](#quadrupole), [field maps](#field-maps), [superpose](#superpose), [RF cavity](#rf-cavity), [space charge](#space-charge); building a cavity–solenoid structure step by step in the [tutorial](#tutorial); the full bibliography is in [references](#references).
