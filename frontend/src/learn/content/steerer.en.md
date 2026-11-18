# Steerer

Every quadrupole, solenoid and cavity in an accelerator is installed a few tenths of a millimetre off. As the beam centre passes these misaligned elements, it is pushed little by little away from the designed centre line (the path the beam centre follows is called the **orbit**). If it drifts too far, the beam scrapes the pipe wall.

To bring the beam back, we need an element that bends the **whole** beam gently by a small angle: the **steerer** (also called a dipole corrector or steering magnet). It is just a weak uniform magnetic field that deflects the beam by a few tenths of a milliradian to a few milliradians. Think of turning the nozzle of a garden hose slightly: the whole jet of water changes direction, and its thickness does not change at all.

This chapter answers three questions: how large a field bends the beam by how much? If it does not change the beam's size, why does it matter so much? Where is the same deflection most effective?

In the animation the beam passes a short drift, then a steerer: vertical field $B_y = 3$ mT acting over the 0.1 m element after it (drawn in 3D as a "window-frame" magnet), then a 1 m drift. This animation has no space charge.

```avas-motion
steerer
```

First look at the 2D view from above (horizontal direction $x$): after the magnet, the whole beam swings towards $-x$ together. The solid line is the beam centre; at the end it is 1.76 mm off. The beam's size is exactly the same as without the steerer (set $B_y$ to 0 to compare). Now the view from the side (vertical direction $y$): nothing changes, because $B_x = 0$. Drag the position bar so that the particles are inside the magnet and look at the cross-section (seen from downstream, the beam flies out of the screen towards you): the red force arrows on the two blue particles are equally long and both point to $-x$. The force does not depend on where the particle is — completely unlike a quadrupole. Switch to 3D: for $B_y > 0$ the N pole is below, the field lines cross the beam from bottom to top, and the pink force arrows labelled F on both particles point to $-x$.

## Why steerers are needed {#steerer-role}

- **Orbit correction**: to cancel the orbit deviations (the "orbit distortion") caused by misaligned elements, field errors and an initial beam offset, reducing the beam loss and effective emittance growth an off-centre beam brings.
- **Beam centring**: to send the beam through the electromagnetic centres of quadrupoles, solenoids and cavities, avoiding the extra deflection (quadrupoles), the coupling of the two directions (solenoids) or the transverse RF "kick" (cavities) of an off-centre passage.
- **Local bumps**: 3–4 steerers create a controlled orbit offset over a short stretch, for example to avoid an obstacle or to scan an aperture.
- **Where they go**: usually right next to quadrupoles, or as correction coils built into quadrupole or solenoid windings (superconducting solenoids often carry built-in dipole steering coils), interleaved with the **beam position monitors** (BPMs) that measure the beam position, at least one per direction per focusing period.

## A small-angle deflection {#steerer-field}

### The direction of the force

The field in a steerer is approximately uniform and transverse (in $x$ and $y$). School physics says that a charged particle in a magnetic field feels the **Lorentz force** $F = qvB$ (velocity perpendicular to the field), whose direction follows Fleming's left-hand rule: let the field lines enter the palm, point the fingers along the motion of the positive charge, and the thumb gives the force. For a particle flying along $+z$:

- a vertical field $B_y$ deflects it horizontally ($x$);
- a horizontal field $B_x$ deflects it vertically ($y$).

Field and deflection are perpendicular to each other. In the AVAS cross-sections (seen from downstream, beam out of the screen, $x$ to the right, $y$ up), $B_y > 0$ deflects a positive particle to the left ($-x$). The two pictures below are cross-section **schematics** computed from the parameters: a uniform transverse field fills the aperture. First $B_y > 0$: the field points up, and the left-hand rule gives a force on a positive charge to the left.

```avas-field
steerer Bx=0 By=0.01 R=0.02
```

Now $B_x > 0$: the field points to the right, and the force points up ($+y$).

```avas-field
steerer Bx=0.01 By=0 R=0.02
```

### How large the deflection is: impulse

Let a particle cross a steerer of length $L$ and field $B$:

- the force on it is $F = q\,v\,B$;
- crossing the magnet takes $t = L/v$;
- the change of transverse momentum (impulse) is $F\,t = q\,B\,L$ — the velocity cancels;
- the transverse momentum divided by the forward momentum $p$ is the deflection angle of the path.

So

$$
\Delta x' = -\frac{q B_y L}{p} = -\frac{B_y L}{B\rho},\qquad
\Delta y' = \frac{q B_x L}{p} = \frac{B_x L}{B\rho}.
$$

$x'$, $y'$ are the angles of the particle's path to the centre line, and $\Delta$ means "change of"; the minus sign means that $B_y > 0$ deflects towards $-x$. $B\rho = p/q$ is the **magnetic rigidity** of the [quadrupole](#quadrupole-lorentz) chapter (how hard the particle is to deflect with a magnetic field); for the example proton $B\rho = 0.179$ T·m. The product $BL$ is called the **integrated field**: the deflection angle is the integrated field divided by the rigidity.

With the animation's defaults: $B_y L = 0.003\ \mathrm{T}\times 0.1\ \mathrm{m} = 3\times 10^{-4}$ T·m, a deflection of $3\times 10^{-4}/0.179 = 1.68$ mrad. A deflection of 1 mrad needs $BL = 1.79\times 10^{-4}$ T·m.

Here is the key point: the force depends only on the field, and the field is the same everywhere, so **every particle is deflected by the same angle**, however far it is from the centre. This is the opposite of a quadrupole, whose force is proportional to the distance from the centre: the further out, the harder the push.

It is the same kind of field as in a [dipole](#dipole), only much weaker: inside, the particle follows an arc of radius $\rho = B\rho/B$, which is 60 m with the defaults, while the magnet is only 0.1 m long, so the path turns through only a very small angle.

The deflection angle is proportional to $1/B\rho$: the "stiffer" the particle, the harder it is to deflect. The same $10^{-4}$ T·m deflects a 35 keV proton by 3.7 mrad but a 100 MeV proton by only 0.067 mrad.

### How far off downstream

After the deflection the beam centre leaves the centre line in a straight line through the drift: offset = deflection angle × distance. In the animation it is 1.05 m from the middle of the magnet to the end, so the end offset is $1.68\ \mathrm{mrad}\times 1.05\ \mathrm{m} = 1.76$ mm (why measure from the middle of the magnet: see [Think about it](#steerer-think)).

Drag the parameters to check:

- Double $B_y$ to 6 mT: the deflection becomes 3.36 mrad and the end offset 3.53 mm, exactly double.
- Set $B_y$ to $-3$ mT: the beam swings to $+x$ instead, $+1.76$ mm at the end.
- Set $B_y$ to 0 and $B_x$ to 3 mT: the view from above no longer bends, and in the side view the beam moves 1.76 mm towards $+y$.
- Set the kinetic energy to 10 MeV: the rigidity becomes 0.458 T·m, the deflection only 0.65 mrad, and the end offset 0.69 mm.

## Electric steerers {#steerer-electric}

An electric field can deflect too: a pair of parallel plates produces a transverse field $E_x$ (or $E_y$). The electric force $F = qE_x$ does not depend on the velocity and points along the field (for a positive charge). The same impulse argument: the particle spends a time $L/v$ in the field, its transverse momentum changes by $qE_x L/v$, and so

$$
\Delta x' = \frac{q E_x L}{p v} = \frac{E_x L}{E\rho},\qquad
\Delta y' = \frac{E_y L}{E\rho},\qquad
E\rho = \frac{p v}{q} = B\rho \cdot v .
$$

$E\rho$ is called the **electric rigidity**, in volts. When the particle is not too fast, $pv = mv^2 = 2W$ ($W$ is the kinetic energy); exactly, in MeV, $pv = \beta^2\gamma\,mc^2$. The example beam has $pv = 3.05$ MeV, $E\rho = 3.05$ MV: $E_x = 10^5$ V/m over 0.1 m deflects by 3.28 mrad, and 1 mrad needs $EL = 3.05\times 10^3$ V.

Electric deflection goes as $1/(pv)$, magnetic deflection as $1/p$: their ratio is exactly the velocity $v$. For the example beam ($\beta = 0.057$, $v = 1.71\times 10^7$ m/s), an electric field of $10^5$ V/m is equivalent to a magnetic field of 5.9 mT ($E = vB$); the same integrated voltage of $10^4$ V deflects a 35 keV proton by 143 mrad but a 100 MeV proton by only 0.053 mrad. The slower the particle, the more effective an electric steerer, which is why low-energy beam lines often use electrostatic deflector plates.

## Only the beam centre moves, not the size {#steerer-matrix}

Every particle is deflected by the same angle, so a steerer **shifts** the whole beam: in the "position–angle" picture (phase space, see [drift](#drift-size)), the ellipse that represents the beam moves along the angle direction without changing shape. The rms size, emittance and Twiss parameters of the beam do not change; only the beam centre does. Compare $B_y = 3$ mT with $B_y = 0$ in the animation: the rms size at the end is 4.23 mm in both cases.

If the rms size does not change, why does a steerer matter? Because whether the beam hits the wall depends on how far its **edge** is from the wall. For a beam 1 mm off centre with an rms size of 1.5 mm, the 3-rms edge is at 5.5 mm, not 4.5 mm. In the animation, stretch the drift `Ld` to 2 m so that the beam grows to an rms of about 7.2 mm: with $B_y = 0$, 1 of the 60 particles hits the pipe (red cross); with $B_y = 3$ mT, 3 do, while the rms size has not changed at all. An off-centre passage through a quadrupole adds an extra deflection, and through a cavity a transverse RF "kick"; both amplify the orbit deviation further.

Seen the other way round, a misaligned quadrupole is an "unwanted steerer". If the beam is off the quadrupole's magnetic centre, the whole beam sits on one side of the quadrupole, the quadrupole's average push on the beam is no longer zero, and the whole beam gets a kick. A quadrupole with $G = 6$ T/m and $L = 0.1$ m that is 0.1 mm off kicks the example beam by 0.34 mrad, like a steerer with an integrated field of $6\times 10^{-5}$ T·m. Many such random kicks along the line add up to the orbit distortion that has to be corrected (see [Errors and correction](#errors), [Orbit correction](#orbit-correction)).

In the beam line below, the steerer has zero length and acts over the element after it (here a 0.2 m drift): $B_y = 1$ mT over 0.2 m is an integrated field of $2\times 10^{-4}$ T·m, deflecting the example beam by 1.12 mrad; at the end of the line (0.5 m after the deflection centre) the beam centre is about 0.56 mm off, about one fifth of the rms size there (2.8 mm). The envelope plot in the editor shows only rms sizes, identical to those without the steerer, and does not show this offset.

```avas-beamline
drift 0.1 0.02 0
steerer 0 0.02 0 0 0.001 0 0.002
drift 0.2 0.02 0
drift 0.4 0.02 0
```

## How a deflection propagates downstream {#steerer-orbit}

In a drift the deflected centre leaves the centre line in a straight line, as computed above.

In a focusing channel with quadrupoles things are different. The beam centre moves exactly like a single particle: in the FODO animation of the [quadrupole](#quadrupole-fodo) chapter the particles swing back and forth about the axis, like a ball rolling in a valley. A beam centre kicked by a steerer does the same: it does not keep running away but swings back and forth about the centre line. Three practical conclusions follow (the quantitative formulas are in the "going deeper" part):

- **A steerer is more "powerful" where the beam is wide.** Where the particles swing widely, the beam is wide too; a kick there produces large swings downstream. So in a FODO the horizontal steerer usually sits next to the focusing quadrupole where the beam is widest horizontally, and likewise vertically.
- **The offset follows the "phase" of the swing.** After the kick, the centre first moves out downstream, comes back, moves out to the other side, like the swinging ball. A quarter of a swing later (90° of phase) the offset is largest; half a swing later (180°) it is zero. So to correct both position and angle at one place you need two steerers about 90° apart.
- **Three steerers can change the orbit over a short stretch only**: the first kicks the beam out, the second bends it back, the third removes the remaining angle, and the orbit outside the stretch is not affected. This is a local bump.

With BPMs and steerers all along the line, the whole orbit can be measured and pulled back. How to compute from the BPM readings how much to set each steerer is explained, with an animation, in the [Orbit correction](#orbit-correction) chapter.

## Think about it {#steerer-think}

> **Think about it 1**: the same steerer with $B_y > 0$, but negatively charged H⁻ ions come through. Which way are they deflected?
>
> Answer: the force reverses: towards $+x$. The direction of the deflection depends on the signs of both the field and the charge.

> **Think about it 2**: in the animation the deflection is 1.68 mrad and the drift after the magnet is 1 m long, yet the end offset is 1.76 mm, not 1.68 mm. Where does the extra come from?
>
> Answer: the particles already start bending inside the 0.1 m magnet and leave it about 0.08 mm off. The effect is the same as a single kick at the **middle** of the magnet, so the distance is measured from there: $1.68 \times (0.05 + 1) = 1.76$ mm (derivation in "going deeper").

> **Think about it 3**: with BPMs and steerers all along the line, if every BPM reading is set to zero, does the beam necessarily pass through the magnetic centre of every quadrupole?
>
> Answer: not necessarily. A BPM reads the beam position relative to its own electrical centre, and the BPM itself is offset from the quadrupole's magnetic centre (BPM offset); moreover, the orbit between two BPMs can still be an oscillation that happens to be zero at the BPMs. So a centre that is zero at the BPMs after correction is not necessarily zero at the quadrupole or cavity centres.

## Common misconceptions {#steerer-misconceptions}

- **"A steerer changes the beam size"**: an ideal uniform field only moves the centroid (the beam centre); the rms size and emittance are unchanged. The editor's envelope plot shows no effect for two reasons: the rms size does not change anyway, and the envelope preview does not draw the centre orbit.
- **"Larger deflections are better, and it doesn't matter where a steerer goes"**: the same angle has a larger effect where the beam is wide ($\beta$ large); and since the offset varies as the sine of the phase, a steerer 180° in phase from the target does almost nothing.
- **"BPM readings of zero mean the beam is centred"**: see "Think about it 3" above.
- **"Electric and magnetic steerers are equally effective at all energies"**: their ratio is the velocity; at low energy electric steerers are far more efficient.
- **"A steerer's length is its effective length"**: in AVAS a steerer's length is written as 0 and its effective length is that of the next element (see below).

## Going deeper: from impulse to the matrix of a "kick" {#steerer-deep-kick}

The main text computed the deflection angle from the impulse. This section works out the numbers one by one, shows why a steerer can be treated as a "kick" at the middle of the magnet, and then puts it into the conversion table (matrix) of the [drift](#drift-deep-matrix) chapter. The last paragraph is for readers who know linear algebra.

### Step 1: count the force and the impulse

The example proton has velocity $v = 1.71\times 10^7$ m/s; in $B_y = 3$ mT it feels a force

$$
F = q v B = 1.60\times 10^{-19}\ \mathrm{C} \times 1.71\times 10^{7}\ \mathrm{m/s}\times 0.003\ \mathrm{T} = 8.2\times 10^{-15}\ \mathrm{N}.
$$

Crossing 0.1 m takes $t = 0.1/(1.71\times 10^7) = 5.85$ ns, so the impulse is $F t = 4.8\times 10^{-23}$ kg·m/s. The proton's momentum is $p = \gamma m v = 2.86\times 10^{-20}$ kg·m/s ($\gamma = 1.0016$, so this is almost exactly $mv$); the ratio of the two is $1.68\times 10^{-3}$, the 1.68 mrad of the main text.

### Step 2: inside the magnet the path is an arc

In a uniform field the particle moves on an arc of radius $\rho = p/(qB) = B\rho/B$ (the school formula $R = mv/(qB)$). With the defaults $\rho = 0.179/0.003 = 59.5$ m. After an arc length $L$ the direction of motion has turned by $L/\rho = 0.1/59.5 = 1.68$ mrad, the same as from the impulse.

How far sideways is the particle when it leaves the magnet? Let it travel a distance $s$ along its original direction and be a distance $d$ off that direction. Drop a perpendicular from the particle's present position onto the radius through the entrance point: this gives a right triangle with hypotenuse $\rho$ and legs $s$ and $\rho - d$. Pythagoras gives $(\rho - d)^2 + s^2 = \rho^2$, which expands to $2\rho d = s^2 + d^2$. Since $d$ is much smaller than $s$, drop $d^2$: $d \approx s^2/(2\rho)$. With $s = L = 0.1$ m:

$$
\frac{L^2}{2\rho} = \frac{0.01}{119} = 8.4\times 10^{-5}\ \mathrm{m} = 0.084\ \mathrm{mm}.
$$

And $\theta\cdot L/2 = 1.68\ \mathrm{mrad}\times 0.05\ \mathrm{m} = 0.084$ mm, exactly the same. In other words, position and angle at the exit are the same as for "turning by $\theta$ all at once at the middle of the magnet and going straight before and after" (to first order in $\theta$). So the steerer can be replaced by a thin "kick" at its middle that changes only the angle, not the position.

### Step 3: a kick is not a multiplication but an addition

The conversion table of a drift is a multiplication: $(x, x')$ times a matrix gives the new $(x, x')$. A steerer instead **adds** a constant to the angle:

$$
\begin{pmatrix} x \\ x' \end{pmatrix}_1 =
\begin{pmatrix} x \\ x' \end{pmatrix}_0 +
\begin{pmatrix} 0 \\ \theta \end{pmatrix}.
$$

What is added does not depend on the particle's original $x$, $x'$, so it cannot be written as a 2 × 2 conversion table. There is a small trick: append a number that is always 1 to the state, $(x, x', 1)$, and "add $\theta$" becomes a multiplication:

$$
\begin{pmatrix} x \\ x' \\ 1 \end{pmatrix}_1 =
\begin{pmatrix} 1 & 0 & 0 \\ 0 & 1 & \theta \\ 0 & 0 & 1 \end{pmatrix}
\begin{pmatrix} x \\ x' \\ 1 \end{pmatrix}_0 .
$$

The drift gets an extra row and column too: $\begin{pmatrix} 1 & L & 0 \\ 0 & 1 & 0 \\ 0 & 0 & 1 \end{pmatrix}$. Check: a particle on the axis, $(0, 0, 1)$, is first kicked by $\theta = -1.68$ mrad and then travels 1.05 m; matrix multiplication gives $x = 1.05\times(-1.68) = -1.76$ mm, the animation's result.

### Step 4: several kicks can be computed separately and added

The equations above show that the state of each particle equals "its state without the steerer" plus "the offset caused by the steerer alone". When several steerers act together, the offset downstream is the sum of the offsets each steerer causes alone; double one steerer's field and the offset it causes doubles. This "add up and scale" property is called **linearity**, and it is the basis of the response matrix in [Orbit correction](#orbit-correction).

### For readers who know linear algebra

A steerer is an affine map $\boldsymbol{x} \to \boldsymbol{x} + \boldsymbol{d}$, $\boldsymbol{d} = (0, \Delta x', 0, \Delta y', 0, 0)\tp$: it moves the beam centre and leaves the second central moments (rms size, emittance, Twiss parameters) unchanged. In the $6\times 6$ linear matrix a steerer is the identity; the deflection needs a $7\times 7$ augmented matrix $\begin{pmatrix} M & \boldsymbol{d} \\ 0 & 1 \end{pmatrix}$ or separate tracking of the centre orbit. A whole beam line is a composition of such affine maps: the centre orbit $\bar{\boldsymbol{x}}$ propagates as $\bar{\boldsymbol{x}} \to M\bar{\boldsymbol{x}} + \boldsymbol{d}$, while the second moments about the centre, $\Sigma \to M\Sigma M\tp$, do not depend on $\boldsymbol{d}$.

## Going deeper: how a deflection propagates in a focusing channel {#steerer-deep-orbit}

The main text said that "a kicked beam centre swings back and forth about the centre line". This section turns that into numbers: first with school simple harmonic motion, then for a real FODO channel, and finally solving for the three kicks of a local bump.

### Step 1: the beam centre is a particle

In the linear model every particle is transported by the same conversion table, and so is the beam centre (the average of all particles). So "how does the kicked centre move" is the same as "how does a particle starting on the centre line with angle $\theta$ move".

### Step 2: uniform focusing: simple harmonic motion

First treat the FODO channel roughly as the same "spring" everywhere (the smooth approximation): the particle's motion is simple harmonic,

$$
x'' = -\frac{1}{\bar\beta^2}\,x ,
$$

like $x'' = -kx$ in the [quadrupole](#quadrupole-deep-matrix) chapter, with the spring constant written as $1/\bar\beta^2$. $\bar\beta$ has the unit of length and is called the average **beta function**; it is the "wavelength" of the swing divided by $2\pi$. The solution starting on the centre line $x = 0$ with initial angle $\theta$ is

$$
x(s) = \bar\beta\,\theta\,\sin\frac{s}{\bar\beta}
$$

(check: at $s = 0$, $x = 0$ and the angle is $x' = \theta\cos 0 = \theta$). The amplitude of the swing is $\bar\beta\,\theta$: the larger $\bar\beta$, the larger the offset from the same kick. $s/\bar\beta$ is the phase of the swing.

An example: the $G = 6$ T/m FODO period of the [quadrupole](#quadrupole-deep-fodo) chapter ($L_q = 0.1$ m, $L_d = 0.2$ m, period length 0.6 m) has a phase advance of 52.7° = 0.920 rad per period, so the average $\bar\beta \approx 0.6/0.920 = 0.65$ m. A kick of 0.56 mrad gives a swing amplitude of about $0.65\times 0.56 = 0.37$ mm.

### Step 3: when the beta function varies along the line

In a real FODO $\beta$ is not constant: the beam is wide in the focusing quadrupoles and narrow in the defocusing ones, and $\beta$ follows. An angle $\theta_j$ applied at position $j$ then gives, at a downstream position $i$ (no acceleration), the offset

$$
\Delta x_i = \sqrt{\beta_i\,\beta_j}\,\sin(\psi_i - \psi_j)\,\theta_j ,
$$

where $\beta$ and $\psi$ are the beta function and phase of that direction (see [Linear optics](#linear-optics)). For constant $\beta$ it reduces to $\bar\beta\,\theta\sin(\Delta\psi)$ of step 2. It is the transfer-matrix element $R_{12}$ written in Twiss parameters. With acceleration it is multiplied by the adiabatic damping factor $\sqrt{(\beta\gamma)_j/(\beta\gamma)_i}$ ($\beta\gamma$ is the relativistic velocity factor; the swing shrinks as the energy rises).

In the same FODO period, $\beta_x$ is 1.06 m at QF and 0.43 m at QD. A kick of 0.56 mrad at a QF gives downstream orbit oscillations with amplitude $\sqrt{1.06 \times 1.06}\times 0.56 = 0.59$ mm at the QFs and $\sqrt{0.43\times 1.06}\times 0.56 = 0.38$ mm at the QDs; at the next QF (52.7° of phase later) the actual offset is $0.59\times\sin 52.7^\circ = 0.47$ mm. The smooth approximation's 0.37 mm lies in between.

The formula shows:

- A steerer is more "powerful" where $\beta$ is large: the larger $\sqrt{\beta_j}$, the larger the downstream offset from the same angle. In a FODO, steerers usually sit next to the focusing quadrupole where $\beta$ of their direction is largest.
- The offset varies as the sine of the phase: largest 90° downstream, zero at 180°. One steerer can correct the position only where the phase difference is 90° and the angle where it is 0° or 180°; correcting position and angle together needs two steerers about 90° of phase apart.

### Step 4: a local bump: solving one equation

Three steerers with kicks $\theta_1$, $\theta_2$, $\theta_3$ must return the position and the angle to zero after the third steerer.

The third steerer changes only the angle, not the position, so at its location the **position** caused by the first two kicks must already be zero. With the formula of step 3:

$$
\sqrt{\beta_3\beta_1}\,\sin\psi_{13}\,\theta_1 + \sqrt{\beta_3\beta_2}\,\sin\psi_{23}\,\theta_2 = 0 ,
$$

where $\psi_{ij}$ is the phase advance from steerer $i$ to steerer $j$. This is a linear equation in $\theta_2$; solve it, then let the third steerer remove the remaining angle to get $\theta_3$. Without acceleration the result is

$$
\theta_2 = -\theta_1\sqrt{\frac{\beta_1}{\beta_2}}\,\frac{\sin\psi_{13}}{\sin\psi_{23}},\qquad
\theta_3 = \theta_1\sqrt{\frac{\beta_1}{\beta_3}}\,\frac{\sin\psi_{12}}{\sin\psi_{23}}.
$$

There is no solution only when $\sin\psi_{23} = 0$, and the required kicks diverge as $\psi_{23}$ approaches a multiple of $180^\circ$; in practice the whole bump is kept below about $180^\circ$ of phase, so that the kicks stay moderate and the bump short. With four steerers, the position and the angle in the middle of the bump can be set separately.

### For readers who know linear algebra

The 2 × 2 transfer matrix of any stretch can be written with the Twiss parameters at its two ends (no acceleration):

$$
R_{12} = \sqrt{\beta_i\beta_j}\,\sin\Delta\psi ,\qquad
R_{22} = \sqrt{\beta_j/\beta_i}\,\left(\cos\Delta\psi - \alpha_i\sin\Delta\psi\right),
$$

and since a thin kick changes only the angle, the downstream position offset is $R_{12}\theta_j$ and the angle offset $R_{22}\theta_j$. A local bump is the linear system $\sum_j R^{(3j)}\theta_j = 0$ (the position and angle rows).

## Going deeper: orbit correction and side effects {#steerer-deep-dynamics}

### The response matrix

Write the responses of all BPMs to all steerers as a response matrix $\mathsf{R}$, $R_{ij} = \partial x_i/\partial\theta_j$ (how much the reading of BPM $i$ changes when the kick of steerer $j$ changes a little; in a linac it is non-zero only for $j$ upstream of $i$, and its values come from the formula above or from simulations changing one steerer at a time). After measuring the orbit $\boldsymbol{x}$, the correction is

$$
\boldsymbol{\theta} = -\,\mathsf{R}^{+}\,\boldsymbol{x},
$$

where $\mathsf{R}^{+}$ is the pseudo-inverse (the least-squares solution, usually via singular value decomposition, truncating small singular values when needed to avoid excessive corrections). Common simplifications are "one-to-one" correction (each steerer aims at the nearest BPM downstream) and local bumps. AVAS's `adjust` tunes parameters iteratively by gradient descent until the diagnostic targets are met (see [Errors and correction](#errors)). A step-by-step explanation of least squares, singular values and truncation starting from school mathematics is in [Orbit correction](#orbit-correction-deep-svd).

### Steerers have side effects too

- **Dispersion**: a steerer is essentially a weak dipole and creates a small dispersion after the deflection (particles of different energy are deflected by different angles and end up at different positions downstream), $D \sim \theta\cdot(\text{distance})$; for beams with a large energy spread, corrections with large deflections increase the effective size.
- **Off-centre passage through elements**: a centre that is zero at the BPMs after correction is not necessarily zero at the quadrupole or cavity centres; the offset between BPMs and element magnetic centres (BPM offset) must be considered too.
- **Field quality**: the field of a small steerer is not uniform near the edge of the aperture, so large-amplitude particles are deflected differently; the matrix model does not include this.
- **Saturation and limits**: if `max` is the limit of the correction (see the keyword description below), a correction running near `max` means the errors exceed the correction capability; check the error settings or add steerers.

## In AVAS {#steerer-in-avas}

### Keyword

```text
steerer 0 R 0 Bx By kind max
```

```avas-params
steerer
```

- The first parameter (length) **must be 0**; the third parameter is reserved; write 0.
- `R`: aperture radius (m).
- `Bx`, `By`: magnetic fields (T) for `kind = 0`, electric fields `Ex`, `Ey` (V/m) for `kind = 1`.
- `kind`: 0 = magnetic steerer, 1 = electric steerer.
- `max`: maximum value. The manual says only "maximum value", usually understood as the allowed limit during correction; neither its meaning nor its unit is explained further in the manual, so the engine is the reference.

> **AVAS convention**: a steerer has length 0 but **acts on the next element**: by default its effective length equals the length of the next element and it is located at the middle of the next element (manual wording). The integrated field is therefore $B\,L_{\text{next}}$, and the deflection angle follows from the formulas of the main text with the length of the next element. The lattice check warns about a `steerer` with non-zero length.

Example: a steerer with a vertical field $B_y = 1$ mT superposed on a quadrupole:

```text
steerer 0 0.02 0 0 0.001 0 0.002
quad 0.1 0.02 0 6
```

For the example beam ($B\rho = 0.17863$ T·m), $B_y L = 1\times 10^{-4}$ T·m, a deflection of $\lvert\Delta x'\rvert = 0.56$ mrad. The sign of the deflection follows from $\boldsymbol{F} = q\boldsymbol{v}\times\boldsymbol{B}$ in a right-handed system; the manual does not state the engine's sign convention, and on 2026-11-03 it was checked with single-particle engine runs: a magnetic $B_y > 0$ deflects a positive particle towards $-x$, $B_x > 0$ towards $+y$, and an electric $E_x > 0$ (`kind = 1`, in V/m) towards $+x$; direction, units and effective length (a uniform field over the whole next element) all agree with the AVAS linear model (end offsets of the three cases: engine −1.2312 / +1.2312 / +7.2069 mm, linear model −1.2316 / +1.2316 / +7.2094 mm).

### Envelope preview, linear model and engine

- The **envelope preview** (the envelope plot of the lattice editor and the beam line in this chapter) computes only rms envelopes and does not draw the centre orbit, so the effect of a steerer is not visible there.
- The **linear model** (`avas.model`) itself does move the beam centre: every step is "a matrix multiplication plus a constant", the constants coming from steerers, element misalignments and the on-axis transverse fields of field maps. The animation of this chapter, the animation of [Orbit correction](#orbit-correction) and the response matrices of the Research page use it; for steerers the linear model's response matrix is exact.
- **Engine**: in DataSet.txt, the $x$ centre = column 29 (synchronous particle $x$) + column 1 (relative centroid offset), the $y$ centre = column 31 + column 3, in m.

### Correction commands

AVAS static-error correction specifies the parameter to adjust with `adjust` and the target with a diagnostic command:

```avas-params
adjust diag_position
```

`adjust 0 v n min max first_step` is written before the element to adjust, and `v` is which of the element's parameters. The manual's only example is `adjust 1 7 5 0 3 0` before a `field`, adjusting the 7th parameter `Ke`, i.e. counting from the first number after the keyword; counting the same way, the 4th parameter of `steerer` is `Bx` and the 5th is `By`. The manual gives no steerer example, so this correspondence should be confirmed against the engine: before relying on it, run a short error study to check that the steerer field is actually changed. `diag_position 0 x y 0` gives the target centre position (mm). An illustrative fragment:

```text
adjust 0 5 1 -0.002 0.002 0
steerer 0 0.02 0 0 0 0 0.002
quad 0.1 0.02 0 6
drift 0.3 0.02 0
diag_position 0 0 0 0
```

Correction runs within an error study (which needs commands such as `err_step`); the algorithm, parameter meanings and a complete example are in [Errors and correction](#errors).

### Seeing the orbit in the engine

To see a steerer's effect in the engine, add a `steerer` (followed by a drift or a quadrupole) to a copy of the example project, run the engine with one or a few particles, follow the beam centre (column 29 + column 1) along $z$ in DataSet.txt, and check the downstream offset and its direction against $\Delta x' = -B_y L/B\rho$.

### Typical values

| Quantity | Typical range | Note |
|---|---|---|
| Integrated field $BL$ | $10^{-5}$–$10^{-3}$ T·m | small room-temperature correction coils |
| Electric field $E$ | $10^4$–$10^6$ V/m | low-energy electrostatic plates |
| Deflection angle | 0.1 to a few mrad | set by the orbit error budget |
| Number | one per direction every 1–2 focusing periods | paired with BPMs |

### Numerical examples

Example beam: protons, 1.527 MeV, $B\rho = 0.17863$ T·m, $E\rho = 3.05$ MV ($pv = 3.05$ MeV).

| Steerer | Field | Effective length | Deflection |
|---|---|---|---|
| magnetic | $B_y = 1$ mT | 0.1 m | 0.56 mrad |
| magnetic | $B_y = 3$ mT | 0.1 m | 1.68 mrad |
| magnetic | $B_y = 10$ mT | 0.1 m | 5.6 mrad |
| electric | $E_x = 10^5$ V/m | 0.1 m | 3.28 mrad |

A deflection of 1 mrad needs $BL = 1.79\times 10^{-4}$ T·m or $EL = 3.05\times 10^{3}$ V. At such a low velocity ($\beta = 0.057$), an electric field of $10^5$ V/m is equivalent to a magnetic field of 5.9 mT ($E = vB$, $v = 1.71\times 10^7$ m/s).

### Common problems

- A `steerer` with non-zero length: the manual requires length 0, and the lattice check warns.
- A `steerer` as the last element of the lattice: there is no "next element" for it to act on; and as a matrix-model element it should not be first or last anyway.
- `kind` not matching the units: with `kind = 1` the `Bx`, `By` positions hold V/m, values usually several orders of magnitude larger than magnetic fields in T.
- No visible steerer effect in the envelope preview: the envelope preview does not draw the centre orbit; look at the animation, the Research page or an engine run.

## Further reading {#steerer-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed. (transverse beam dynamics in linacs).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed. (orbit distortion and correction, response matrices).
- CERN Accelerator School lecture notes on beam diagnostics and orbit correction (CAS proceedings).
- Related chapters: [Orbit correction](#orbit-correction), [Errors and correction](#errors), [Quadrupole](#quadrupole), [Dipole](#dipole), [Simulation](#simulation); full bibliography in [References](#references).
