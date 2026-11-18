# Phase space and emittance

With a lens you can focus a beam to a very small spot, at the price that it then spreads out at a larger angle; make the beam very parallel and it becomes wide. Can a beam be narrow and parallel at the same time?

A torch poses the same puzzle. Its filament has a certain size: adjust the reflector and the spot on the wall becomes smaller, but the light beam opens wider; make the light more parallel and the spot at the torch's opening becomes larger. "Spot size times opening angle" cannot be pushed down however you adjust it. The corresponding quantity of a beam is called the **emittance**. This chapter explains what it is, why it stays nearly constant almost everywhere along a beam line, and how accelerator physicists describe a whole beam with one ellipse.

First, have a look. Below, a beam flies through a 1.5 m long drift (an empty vacuum pipe). This time keep your eye on **the "phase space" picture at the bottom right**: every dot is a particle, the horizontal axis is how far it is from the axis (position $x$ in mm), the vertical axis is the direction in which it flies (angle $x'$ in mrad). Press play, or drag the position slider:

```avas-motion
drift
```

You will see: the swarm of dots fills roughly an ellipse (the dashed line is its rms ellipse). At first the ellipse leans down to the right; at about 0.19 m it stands upright, and here the beam is narrowest; after that it leans more and more up to the right and becomes longer and flatter. **The shape of the ellipse keeps changing, but its area does not.** This unchanging area is the hero of this chapter.

## Why position and angle are both needed {#phase-space-coordinates}

### Position alone is not enough

Imagine taking a cross-section photo of two proton beams at the same place in a beam line, and the two spots are equally large. One beam is converging, the other diverging; 10 cm further on they are completely different. Position alone does not determine how the beam goes on.

It is the same as projectile motion at school: to predict where a ball lands, knowing where it is now is not enough; you also need to know in which direction and how fast it flies. For a particle moving along a beam line, "the direction in which it flies" is its **angle** (also called slope) $x'$: how far it moves sideways for each distance forward. $x' = 3$ mrad means that it moves 3 mm sideways for every metre forward (1 mrad is a thousandth of a radian; **1 mrad × 1 m = 1 mm**, a conversion used throughout).

The angle is really the ratio of the transverse to the forward momentum: split the particle's momentum into the forward part $p$ and the transverse part $p_x$; for small angles $x' \approx p_x/p$. This will matter when we come to acceleration.

### Phase plane and phase space

So every particle needs a pair of numbers $(x, x')$ in each transverse plane. The plane drawn with $x$ on the horizontal and $x'$ on the vertical axis is called the **phase plane**, and every particle is a point in it; the horizontal, vertical and longitudinal directions together form the six-dimensional **phase space**. The coordinates are all deviations from the reference particle (the particle moving along the design orbit, see [How AVAS simulates a beam](#intro-reference)); $s$ is the distance along the orbit (the longitudinal position $z$ in a straight section):

| Coordinate | Meaning | Unit (AVAS internal / matrices) |
|---|---|---|
| $x$, $y$ | transverse position deviation | m |
| $x' = \dd x/\dd s$, $y' = \dd y/\dd s$ | transverse angle (slope) | rad |
| $z$ | longitudinal position deviation, positive ahead of the reference particle | m |
| $\delta = \Delta p/p$ | relative momentum deviation | 1 |

($\dd x/\dd s$ reads "the rate of change of $x$ with $s$", i.e. the slope.) In the paraxial approximation $x' \approx p_x/p$, where $p$ is the reference momentum. Transfer matrices act on the column vector $(x, x', y, y', z, \delta)\tp$, see [Linear optics](#linear-optics).

One number shows how much the angle matters: at the entrance the example beam has an rms size of 0.954 mm and an rms angle of 3.026 mrad (rms is the root mean square; think of it as the typical size). A particle with an angle of 3 mrad moves 0.3 mm sideways over 0.1 m, a third of the beam size.

### Two kinds of "snapshot"

Two kinds of "snapshot" must also be told apart:

- the distribution **at one time**: like a photo of the whole bunch at one instant. The t-code (field-map elements, drifts) advances in time, and at a given moment the particles are at different longitudinal positions;
- the distribution **at one longitudinal position**: like a photo finish, each particle recorded as it crosses the plane. The z-code (matrix elements) and transfer-matrix theory compare all particles at the same $s$.

For rms quantities the difference is usually very small, but it needs care for long bunches with large energy spread (for example the $z$–$\delta$ correlation). According to the manual, `DataSet.txt` records the bunch parameters in the s coordinate system. Where the two descriptions come from in AVAS is explained in [t-code and z-code](#intro-tracking).

## The phase-space picture: every particle is a dot {#phase-space-picture}

### How to read the picture

Each of the four regions of a phase-space picture has a meaning (horizontal plane, $x$ positive to the right):

- **upper right** ($x > 0$, $x' > 0$): the particle is to the right of the axis and still flying to the right, getting farther from the centre;
- **lower right** ($x > 0$, $x' < 0$): the particle is to the right but flying back towards the centre;
- **lower left** and **upper left** likewise, with left and right swapped.

So the **lean** of a swarm of dots tells you what the beam is doing:

- the ellipse leans **up to the right**: particles on the right mostly fly to the right, those on the left to the left; the beam is **diverging** (growing);
- the ellipse leans **down to the right**: particles on the right mostly fly to the left; the beam is **converging** (shrinking);
- the ellipse **stands upright**: position and angle are uncorrelated, the beam is narrowest right here; this place is called the **waist**.

### Watching it in the animation

Back to the drift animation at the start of this chapter (default parameters):

- **entrance**: the ellipse leans down to the right, the beam converges. rms size 0.95 mm, rms angle 3.58 mrad.
- **at about 0.19 m**: the ellipse is upright; this is the waist, with the smallest rms size, about 0.68 mm.
- **exit at 1.5 m**: the ellipse leans strongly up to the right, long and flat; the rms size is about 4.8 mm.

Throughout, the height of the ellipse (the spread of angles) hardly changes, because there is no force in a drift and the angles stay the same. Watch the blue particle that starts on the positive $x$ side: at the entrance it is at about (1.4 mm, −3.8 mrad), to the right of the axis but flying to the left; it crosses the axis at about 0.38 m and reaches the exit at about −4.1 mm. In the phase-space picture it just moves steadily to the left.

Drag the "converging or diverging" slider in the animation to a negative value (for example the example beam's −0.66): the ellipse at the entrance now leans up to the right, the beam diverges from the start, and there is no waist.

## How linear elements move the points in phase space {#phase-space-shear}

### A drift pushes sideways

There is no force in a drift. A particle flies straight for a length $L$; its angle stays and its position grows by $L x'$:

$$
x_1 = x_0 + L\,x_0', \qquad x_1' = x_0'.
$$

In the phase-space picture every dot stays at the same height and moves sideways by $L x'$: dots with positive angle move right, dots with negative angle move left, by an amount proportional to the angle. It is like pushing a deck of cards: the bottom card stays, each card higher up is pushed farther, and the deck leans over. This is a **horizontal shear**.

### A lens pushes up or down

A **thin lens** (focal length $f$, for example a short quadrupole) leaves the particle no time to change its position and changes only its angle: the farther from the axis, the more it is bent back,

$$
x_1 = x_0, \qquad x_1' = x_0' - \frac{x_0}{f}.
$$

In the phase-space picture every dot stays at the same horizontal position and moves vertically by $-x/f$: dots on the right move down, dots on the left move up. This is a **vertical shear**.

### Shears do not change areas

When you push a deck of cards, its shape changes, but neither the area of each card nor the total area of the deck changes. A shear is the same: however the dots are pushed over, the **area** they cover in the phase plane **stays the same**. Any combination of drifts and lenses is just a series of shears, so the area covered by a swarm of dots does not change under linear transport. Why shears preserve area, and how to check it with numbers, is shown in [Going deeper: shears, determinants and the conservation of area](#phase-space-deep-area).

Example: through a 0.1 m drift the x plane of the example beam goes from an rms size of 0.954 mm to 1.149 mm, while the area of the rms ellipse stays the same.

## Emittance: the area of the ellipse {#phase-space-emittance}

### Emittance measures how "disordered" a beam is

The area a beam covers in the phase plane is called its **emittance**. It measures how "disordered" the beam is: the wider the spread of positions and of angles, the larger the area. Think of a crowd walking: if everyone is at the same place and walks in the same direction, the area is zero, perfectly "orderly"; if positions and directions all differ, the area is large.

### rms emittance

A beam has many thousands of particles and a blurred edge, so "area" needs a precise definition. The most common one is the **rms emittance**, computed from three averages (angle brackets $\langle\ \rangle$ mean the average over all particles; positions and angles are measured from the beam centre):

- $\langle x^2\rangle$: the average squared position; its square root is the rms size;
- $\langle x'^2\rangle$: the average squared angle; its square root is the rms angle;
- $\langle x x'\rangle$: the average of position times angle, a measure of their **correlation**: positive when particles on the right mostly fly to the right (diverging), negative when they mostly fly to the left (converging), zero when there is no correlation (waist).

$$
\varepsilon_{\mathrm{rms}} = \sqrt{\langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2}
$$

It is the area of the "rms ellipse" in phase space divided by $\pi$. The second term, $\langle x x'\rangle^2$, removes the correlation between position and angle: a beam that is merely converging or diverging can have a large $\langle x^2\rangle\langle x'^2\rangle$, but the correlated part can be "straightened out" with a lens and does not represent real disorder. The rms emittance has a definite value for any distribution, is exactly conserved under linear transport (without coupling) and grows under non-linear forces, so rms emittance growth is the usual measure of beam-quality degradation. Why second moments are the natural measure, and how sensitive they are to the tails, is discussed in [Initial distributions](#distributions-rms).

### Why a beam cannot be narrow and parallel

At a waist the ellipse is upright, $\langle x x'\rangle = 0$, and the formula becomes

$$
\sigma_x\,\sigma_{x'} = \varepsilon ,
$$

**rms size times rms angle equals the emittance**. With the emittance fixed, halving the size doubles the angle. This answers the opening question: lenses and drifts can only trade "narrow" for "parallel", the product cannot be pushed down. The geometric rms emittance of the example beam is 2.412 mm·mrad; to squeeze the rms size at a waist to 0.5 mm, the rms angle must be at least 4.82 mrad.

Check it with the drift animation (default emittance 0.138 π·mm·mrad, a geometric emittance of about 2.42 mm·mrad):

- at the waist (about 0.19 m): 0.68 mm × 3.58 mrad ≈ 2.43 mm·mrad, equal to the emittance;
- at the entrance: 0.95 mm × 3.58 mrad ≈ 3.42 mm·mrad, larger than the emittance; the excess is the contribution of the lean (the correlation).

Drag the animation's "emittance" slider to 0.276 (doubled): the shape of the ellipse stays, the area doubles, the rms size at the waist grows from 0.68 mm to about 0.96 mm ($\sqrt2$ times) and at the exit from 4.8 mm to about 6.7 mm.

### Total and percentage emittance

$\varepsilon_{100\%}$ (total or edge emittance) is the area of the smallest ellipse containing all particles divided by $\pi$; $\varepsilon_{90\%}$, $\varepsilon_{99\%}$ and so on contain the corresponding fraction of the particles. Their ratio to the rms emittance depends on the shape of the distribution: 4 for KV, 6 for the four-dimensional waterbag, 8 for the parabolic distribution, and the Gaussian has no total emittance (its tails are infinitely long), see [The four standard distributions](#distributions-types).

### Units

Emittance has the dimension length × angle. It is customarily written as π·mm·mrad, where the "π" is only a label saying that the number is the area divided by π:

$$
1\ \pi\cdot\mathrm{mm\cdot mrad} \;\to\; \varepsilon = 1\ \mathrm{mm\cdot mrad} = 1\ \mu\mathrm{m} = 10^{-6}\ \mathrm{m\cdot rad}
$$

When quoting an emittance, always say three things: rms or total (percentage), geometric or normalised (see the next section), and whether π has already been divided out.

## Liouville's theorem and adiabatic damping {#phase-space-liouville}

### Liouville's theorem: particles in phase space are like an incompressible fluid

The conservation of area above is a special case of a more general theorem. If the motion of the particles can be described by a Hamiltonian (applied electromagnetic fields, and space charge in the mean-field approximation, qualify), then in the six-dimensional phase space of the canonical coordinates $(x, p_x, y, p_y, z, p_z)$ the particle density stays constant along each particle's path, and the phase-space volume occupied by any swarm of particles is conserved. If the three directions are uncoupled, the area of each two-dimensional projection is conserved separately. **Particles in phase space behave like an incompressible fluid**: they can be stretched and twisted, but not compressed. ("Hamiltonian" can be read as the mathematical description of an "energy-conserving mechanical system"; "canonical coordinates" are positions together with their paired momenta.)

Liouville's theorem does not guarantee that the "effective" emittance is conserved. Non-linear forces (field non-linearities, non-linear space charge, the sinusoidal shape of the RF field) make particles at different distances from the centre rotate at different rates, and the ellipse in phase space is twisted into an S or a spiral. The occupied area does not change, but the area of the ellipse needed to enclose it, and the rms emittance, grow; this is called **filamentation**. Think of stirring a spoonful of milk into coffee: the volume of the milk stays the same, but it spreads through the whole cup, and any outline drawn around the milk must be large. The analogy is right about "incompressible"; it fails in that milk really mixes by molecular diffusion, while the filaments of Hamiltonian motion never truly merge.

Particle losses, collisions, stripping and similar processes do not satisfy the assumptions of Liouville's theorem.

### Adiabatic damping: acceleration makes the angles smaller

The angle $x' = p_x/p$ is the ratio of transverse to forward momentum. Ideal acceleration pushes only forward: it increases $p$ and leaves $p_x$ unchanged. With the numerator fixed and the denominator growing, the angle shrinks as $1/p$, and the area in the $(x, x')$ plane shrinks with it. This is called **adiabatic damping**.

Work it out for the example beam: accelerated from 1.527 MeV to 10 MeV, the momentum grows from 53.55 MeV/$c$ to 137.35 MeV/$c$, 2.565 times as much. The transverse momentum is unchanged, so every angle shrinks to 1/2.565, and so does the area in the phase plane: the geometric emittance goes from 2.412 mm·mrad to 0.940 mm·mrad.

The momentum is proportional to $\beta\gamma$ (see [Relativistic kinematics](#kinematics)), so the area multiplied by $\beta\gamma$ stays the same during acceleration. Hence the definition

$$
\varepsilon_n = \beta\gamma\,\varepsilon
$$

$\varepsilon$ is called the **geometric emittance**, $\varepsilon_n$ the **normalised emittance**. In the ideal linear, uncoupled case without filamentation, $\varepsilon_n$ stays constant along a linac, which makes it the right measure for comparing beam quality at different energies. (Strictly, what is conserved is the area of the canonical plane $(x, p_x/mc) = (x, \beta\gamma x')$; $x'$ is not a canonical momentum.)

Example: the example beam has a normalised rms emittance of 0.1377 π·mm·mrad; at 1.527 MeV ($\beta\gamma$ = 0.05708) its geometric emittance is 2.412 mm·mrad. Accelerated to 10 MeV, $\beta\gamma$ grows to 0.14639, and the geometric emittance shrinks to 1/2.565 of its value, 0.940 mm·mrad, while the normalised emittance stays the same.

> Note: the formula uses the $\beta\gamma$ of the reference particle. For beams with a large energy spread, a normalised emittance computed with each particle's own $\beta\gamma$ differs from this; the difference can be significant in low-energy electron beams.

## The shape of the ellipse: Twiss parameters {#phase-space-twiss}

### Three numbers for the shape, one for the size

To describe an ellipse centred on the origin, you need to say how wide and how tall it is and which way it leans. Accelerator physicists use three numbers for the shape, the **Twiss parameters** (also called Courant–Snyder parameters), and the emittance $\varepsilon$ for the size (the area is $\pi\varepsilon$):

- $\beta_x$ governs the **size**: $\sigma_x = \sqrt{\beta_x\varepsilon}$, in mm/mrad (numerically m/rad, i.e. m);
- $\gamma_x$ governs the **angle**: $\sigma_{x'} = \sqrt{\gamma_x\varepsilon}$;
- $\alpha_x$ governs the **lean**: $\langle x x'\rangle = -\alpha_x\varepsilon$.

Only two of the three are independent; they always satisfy $\beta_x\gamma_x - \alpha_x^2 = 1$.

> **Notation**: in this chapter $\beta$ and $\gamma$ without subscript are the relativistic factors, while $\beta_x$, $\alpha_x$, $\gamma_x$ with a plane subscript are Twiss parameters. They just happen to share names.

The sign of $\alpha_x$ tells whether the beam converges or diverges (note the minus sign):

| $\alpha_x$ | $\langle x x'\rangle$ | Ellipse | State |
|---|---|---|---|
| $\alpha_x > 0$ | $< 0$ | leans down to the right | converging (towards a waist) |
| $\alpha_x = 0$ | $= 0$ | upright | waist |
| $\alpha_x < 0$ | $> 0$ | leans up to the right | diverging (away from a waist) |

The "converging or diverging" slider of the drift animation is the $\alpha$ at the entrance: +1 by default (converging), −0.66 for the example beam (diverging).

### The example beam

The x plane of the example beam: $\alpha_x = -0.658$, $\beta_x = 0.377$ mm/mrad, $\gamma_x = (1 + 0.658^2)/0.377 = 3.796$ mrad/mm, geometric emittance $\varepsilon = 2.412$ mm·mrad. So the rms size is $\sqrt{0.377 \times 2.412} = 0.954$ mm and the rms angle $\sqrt{3.796 \times 2.412} = 3.026$ mrad. $\alpha_x < 0$: the beam diverges at the entrance.

In a drift the waist lies at $\alpha_x/\gamma_x$ (positive means downstream), and at the waist $\beta^* = 1/\gamma_x$. For the example beam $\alpha_x/\gamma_x = -0.173$ m: the waist is 0.173 m upstream of the entrance (an invisible "virtual waist"), where $\beta^*$ = 0.263 mm/mrad and the rms size is 0.797 mm. How the width, height, axis intercepts and tilt of the ellipse follow from the Twiss parameters is shown in [Going deeper: from three averages to Twiss parameters and the invariant](#phase-space-deep-twiss).

### Every particle has its own ellipse: the Courant–Snyder invariant

In a drift the particles fly away and never return. What happens in a focusing channel? Below are 6 FODO cells (focusing quadrupole, drift, defocusing quadrupole, drift, repeated six times, 0.8 m per cell, see [Quadrupole](#quadrupole-fodo)). Again watch the phase-space picture at the bottom right, and use the position slider to stop at different places:

```avas-motion
fodo
```

You will see three things:

1. **The ellipse "breathes" within a cell**: in the middle of the focusing quadrupole (for example at $s$ = 0.05 m) the ellipse is upright and widest (rms about 1.77 mm), in the middle of the defocusing quadrupole (0.45 m) it is also upright but narrowest (about 0.90 mm), and in the drifts it leans.
2. **At the same place in every cell the ellipse is identical**: stop at the entrance of each cell ($s$ = 0, 0.8, 1.6, … m) and the shape and orientation of the ellipse are the same, with an rms size of about 1.7 mm each time. Such a beam is called **matched** (see [Matching](#matching)).
3. **The blue particle goes round its own ellipse**: stop at the cell entrances and watch the blue particle that starts on the positive $x$ side. The first time it is at about (2.5 mm, 4.3 mrad); after every cell it lands on **the same ellipse** (the shape of the beam's rms ellipse, 1.5 times as large), moving on along it by about 76°, so that it goes round once in about 4.8 cells.

Point 3 says: in a linear focusing system the Twiss parameters change with $s$ (following the transport rules of [Linear optics](#linear-optics)), but every particle always stays on its own member of a family of ellipses whose shape keeps changing while their area stays the same. The "size" of this ellipse (written $J$) stays constant along the beam line and is called the **Courant–Snyder invariant**; it measures the amplitude of the particle's oscillation. The particle's path can be written $x(s) = \sqrt{J\beta_x(s)}\cos\left(\psi(s) + \psi_0\right)$, where $\psi$ is the betatron phase ("76° per cell" is its increment, called the **phase advance**), see [Linear optics](#linear-optics). So $\sqrt{\beta_x}$ decides how the oscillation amplitude grows and shrinks along the beam line, and $\sqrt{J}$ decides the "size" of this particle.

Try one more thing: set the "mismatch" slider to 1.3, so that the incoming beam is 30 % larger than the size this channel "likes". Stop at the cell entrances: the ellipses are no longer the same, the rms size jumps between about 2.2, 1.7, 2.9 and 0.9 mm, like a tumbling ellipse. But however it tumbles, **its area always stays the same**.

## The σ matrix: a whole beam in one table {#phase-space-sigma}

### Three averages in one table

Three averages are all it takes to describe the rms ellipse of a beam in one plane. Arranged in a symmetric table of 2 rows and 2 columns they are called the **beam matrix** or **σ matrix**:

$$
\Sigma = \begin{pmatrix} \langle x^2\rangle & \langle x x'\rangle \\ \langle x x'\rangle & \langle x'^2\rangle \end{pmatrix}
= \varepsilon \begin{pmatrix} \beta_x & -\alpha_x \\ -\alpha_x & \gamma_x \end{pmatrix},
\qquad \varepsilon = \sqrt{\det\Sigma}
$$

$\det\Sigma$ is the **determinant** of the table: top left times bottom right, minus top right times bottom left. So $\sqrt{\det\Sigma}$ is exactly the rms emittance formula of the previous section.

The x plane of the example beam: $\langle x^2\rangle$ = 0.910 mm², $\langle x x'\rangle$ = 1.587 mm·mrad, $\langle x'^2\rangle$ = 9.157 mrad², so $\sqrt{\det\Sigma} = \sqrt{0.910 \times 9.157 - 1.587^2} = \sqrt{8.333 - 2.519}$ = 2.412 mm·mrad.

### Why put them in a table

The benefit of the σ matrix: after any linear element the table is updated by one fixed, simple rule that uses only these three numbers and the element's transfer matrix, without following individual particles. In matrix form it reads $\Sigma_1 = M\,\Sigma_0\,M\tp$ (derivation and a numerical check in [Going deeper: from three averages to Twiss parameters and the invariant](#phase-space-deep-twiss)). The rule holds for any distribution (not only elliptical ones), because it involves only averages; the conservation of emittance follows directly from it. This is exactly how AVAS's linear preview advances the beam.

### The six-dimensional σ matrix

In six dimensions, $\Sigma_{ij} = \langle u_i u_j\rangle$ with $u = (x, x', y, y', z, \delta)$, a 6 × 6 table. The diagonal blocks give the emittance of each of the three planes (the projected emittances); the off-diagonal blocks describe correlations between the planes:

- dipoles create an $x$–$\delta$ correlation (dispersion: particles of different energy take different paths), see [Dipole](#dipole);
- solenoids couple the $x$ and $y$ planes; the projected emittances $\varepsilon_x$, $\varepsilon_y$ can then change, but the four-dimensional emittance $\sqrt{\det\Sigma_4}$ is conserved under linear symplectic transformations, see [Solenoid](#solenoid);
- RF cavities create a $z$–$\delta$ correlation, see [Longitudinal motion](#longitudinal).

## Hands on: an ellipse through a lens and a drift {#phase-space-lab}

The lab draws the rms ellipse before and after a thin lens (focal length `f` in m; 0 means no lens) and a following drift of length `L`, and gives the Twiss parameters and rms size at the exit. The parameter `beta` is in mm/mrad, `emit` is the normalised rms emittance (π·mm·mrad).

Start at a waist ($\alpha_x = 0$, $\beta_x$ = 1 m), with a 0.5 m focal-length lens followed by a 0.5 m drift. At the entrance the ellipse is upright, the rms size and rms angle are both 1.87 (mm, mrad), and the area (the lab's "ellipse area / π") is 3.504 mm·mrad.

- **The lens** changes every particle's angle by $-x/f$ and shears the ellipse vertically so that it leans down to the right: the beam starts to converge, and after the lens $\alpha_x = \beta_x/f = 2$.
- **After the 0.5 m drift** $\beta_x = 0.25$ m and $\alpha_x = -0.5$; the rms size has gone from 1.87 mm to 0.94 mm.
- The new waist lies 0.4 m after the lens, not at the focal point 0.5 m away, and there $\beta^*$ = 0.2 m. Why not at the focus? Because the particles already carry their own angles, and the lens only makes them point at the focus "on average" (see Think about it below).
- The area stays 3.504 mm·mrad throughout.

```avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
```

Things to try: change `L` and find where the ellipse is upright (the waist); change `emit` and confirm that the shape stays the same and the ellipse only grows as a whole; raise `W` and watch the geometric emittance shrink as $1/(\beta\gamma)$.

The x plane of the example project's initial beam in a 0.1 m drift (a diverging beam: $\alpha_x$ goes from −0.658 to −1.037, $\beta_x$ grows from 0.377 to 0.547 mm/mrad, the size from 0.954 mm to 1.149 mm):

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.1
```

## Longitudinal phase space {#phase-space-longitudinal}

### Ahead or behind, faster or slower

Longitudinally there is also a pair of numbers: how far ahead of the reference particle a particle is ($z$), and how large its momentum deviation is ($\delta$). Think of runners: $z$ says how far ahead of the leader you are, $\delta$ how much faster you run.

In linacs one more often uses phase and energy $(\Delta\varphi, \Delta W)$, because an RF cavity "sees" **when** a particle arrives, not where it is. A particle $z$ ahead of the reference particle arrives $z/(\beta c)$ earlier and sees a smaller RF phase. The conversion between the two pairs is (see [Kinematics](#kinematics-rf)):

$$
\Delta\varphi = -\frac{2\pi}{\beta\lambda}\,z, \qquad
\Delta W = \beta^2\gamma\,mc^2\,\delta
$$

so the two geometric emittances differ by a constant factor

$$
\varepsilon_{\Delta\varphi\Delta W} = \frac{2\pi}{\beta\lambda}\,\beta^2\gamma\,mc^2\;\varepsilon_{z\delta}
$$

(with the phase in degrees, replace $2\pi$ by $360$). $\lambda$ is the wavelength of the bunch frequency.

### The example beam

With the `twissz` parameters of the example `beam.txt`: the longitudinal rms length of 1.316 mm corresponds to an rms phase width of 4.51° at 162.5 MHz; $\delta_{\mathrm{rms}} = 3.532\times10^{-3}$ corresponds to an rms energy spread of 10.78 keV (0.71 %); the longitudinal geometric rms emittance of 4.644 mm·mrad converts to 48.5 deg·keV (statistics taken directly from the particle file `part_rfq.dst` give 1.24 mm, 4.26°, 11.4 keV and 48.7 deg·keV; the two sets are compared in [Initial distributions: In AVAS](#distributions-in-avas)). The conversion is worked out step by step in [Going deeper: converting the longitudinal coordinates](#phase-space-deep-longitudinal). Motion in longitudinal phase space (synchronous phase, phase-stable region, longitudinal focusing) is discussed in [Longitudinal motion](#longitudinal).

## Think about it {#phase-space-think}

> **Think 1**: The opening question: can lenses and drifts make a beam both narrow and parallel?
>
> Answer: no. At a waist the product of rms size and rms angle equals the rms emittance, and the emittance does not change in lenses and drifts. To squeeze the example beam to an rms size of 0.5 mm at a waist, the rms angle must be at least 4.82 mrad.

> **Think 2**: Adiabatic damping makes the geometric emittance smaller. Has the beam been "cooled"?
>
> Answer: no. The particles' transverse momentum $p_x$ has not changed at all; only the longitudinal momentum is larger and the paths are "straighter". The normalised emittance (proportional to the spread of transverse momentum) is unchanged. Real cooling (electron cooling, stochastic cooling) must reduce the spread of $p_x$, which requires a dissipative process not bound by Liouville's theorem.

> **Think 3**: In the lab, why is the waist 0.4 m after the 0.5 m focal-length lens and not at 0.5 m?
>
> Answer: the lens gives every particle an extra angle $-x/f$; if the particles had all been flying parallel, they would meet in one point at 0.5 m. But the incoming beam has an rms angle of 1.87 mrad of its own: every particle adds its own angle to the direction towards the focus. This part makes the beam start growing again before the focus, so the narrowest point moves forward to 0.4 m, and it is not a point: the rms size there is still 0.84 mm.

> **Think 4**: In the FODO animation the mismatched ellipse tumbles and the largest rms size grows. Has the emittance of the beam grown?
>
> Answer: no. The area stays the same; the ellipse just does not fit the channel and is sometimes stretched long and thin, so its projection on the $x$ axis looks wider. But once there are non-linear forces (strong space charge, for example), a mismatched beam filaments and the rms emittance really grows, which is one of the reasons for matching.

## Common misconceptions {#phase-space-misconceptions}

- **"The π in π·mm·mrad has to be multiplied in."** No. The π is only a label saying that the number is already the area divided by π. Publications are not consistent on this; check when exchanging data.
- **"rms and total emittance differ by a fixed factor."** The factor depends on the distribution: 4 for KV, and a Gaussian has no total emittance at all, see [Initial distributions](#distributions).
- **"Emittance is conserved during acceleration."** The normalised emittance is; the geometric emittance shrinks as $1/(\beta\gamma)$. Beams at different energies must be compared by normalised emittance.
- **"Liouville's theorem guarantees a constant rms emittance."** Liouville's theorem guarantees the occupied phase-space volume. Filamentation from non-linear forces makes the rms emittance grow while the volume stays the same.
- **"Positive α means diverging."** The opposite: $\alpha_x > 0$ means $\langle x x'\rangle < 0$, the beam is converging.
- **"The tilt of the ellipse is a physical quantity."** The tilt depends on the units of $x$ and $x'$ (mm with mrad or mm with rad, for example) and changes with them; when changing units, change the Twiss parameters too.
- **"A smaller beam means a smaller emittance."** The size is $\sqrt{\beta_x\varepsilon}$; focusing changes $\beta_x$, not $\varepsilon$.
- **"The solenoid increased $\varepsilon_x$, so the beam quality got worse."** A solenoid couples $x$ and $y$, so the projected emittances can change while the four-dimensional emittance stays the same; the coupling can disappear after the solenoid, see [Solenoid](#solenoid).

## Going deeper: shears, determinants and the conservation of area {#phase-space-deep-area}

The main text used "pushing a deck of cards" to show that shears do not change areas. This section turns that into formulas with school geometry and then extends it to any linear element and to acceleration.

### Step 1: shearing a square

Take a square of side 1 in the phase plane, with corners $(0,0)$, $(1,0)$, $(0,1)$, $(1,1)$ (in mm, mrad). Through a drift of $L = 0.5$ m ($x_1 = x_0 + L x_0'$, $x_1' = x_0'$) the corners become $(0,0)$, $(1,0)$, $(0.5,1)$, $(1.5,1)$. This is a parallelogram: its base is still 1, its height still 1, and its area = base × height = 1. The square has been pushed over; its area has not changed.

### Step 2: a general linear transformation

Every linear element turns $(x, x')$ into

$$
x_1 = a\,x_0 + b\,x_0', \qquad x_1' = c\,x_0 + d\,x_0' .
$$

The two sides $(1, 0)$ and $(0, 1)$ of the unit square become the vectors $(a, c)$ and $(b, d)$, and the square becomes the parallelogram spanned by them. From school: the parallelogram spanned by $(a, c)$ and $(b, d)$ has the area $\lvert ad - bc\rvert$ (put it into an $(a+b)\times(c+d)$ rectangle and subtract the four corner triangles and two small rectangles to check).

The number $ad - bc$ is called the **determinant** of the transformation. Any shape can be built from many small squares, each of which is scaled by $\lvert ad - bc\rvert$, so **every area is multiplied by the determinant**.

### Step 3: drifts and lenses have determinant 1

- Drift: $a = 1$, $b = L$, $c = 0$, $d = 1$, so $ad - bc = 1$.
- Thin lens: $a = 1$, $b = 0$, $c = -1/f$, $d = 1$, so $ad - bc = 1$.

A lens ($f = 0.5$ m) followed by a drift ($L = 0.5$ m): substituting the lens output into the drift gives $x_2 = x_0 + 0.5\,(x_0' - 2x_0) = 0\cdot x_0 + 0.5\,x_0'$ and $x_2' = -2x_0 + x_0'$, i.e. $a = 0$, $b = 0.5$, $c = -2$, $d = 1$, and $ad - bc = 0 - (0.5)(-2) = 1$. Two elements in a row scale the area by $1 \times 1 = 1$. In general the determinant of a chain of elements is the product of their determinants; if each is 1, so is the total.

### Step 4: the area of an ellipse

A circle of radius $r$ has the area $\pi r^2$. Stretching it by a factor $a$ in one direction and $b$ in the other gives an ellipse with semi-axes $ar$ and $br$; the area is multiplied by $ab$ and becomes $\pi\,(ar)(br)$. So an upright ellipse has the area $\pi \times$ the product of its semi-axes. At a waist the rms ellipse has the semi-axes $\sigma_x$ and $\sigma_{x'}$, and its area is $\pi\sigma_x\sigma_{x'} = \pi\varepsilon$: this is "the rms ellipse area divided by $\pi$ is the rms emittance". Shears tilt the ellipse without changing its area, so the rms ellipse has the area $\pi\varepsilon$ everywhere.

### Step 5: acceleration has a determinant below 1

Ideal acceleration changes only the forward momentum: $x_1 = x_0$, $x_1' = (p_0/p_1)\,x_0'$. The determinant is $ad - bc = p_0/p_1$. For the example beam from 1.527 MeV to 10 MeV, $p_0/p_1 = 53.55/137.35 = 0.390 = 1/2.565$, and the geometric emittance is multiplied by 0.390: 2.412 → 0.940 mm·mrad. This is adiabatic damping. With $(x, \beta\gamma x')$ as coordinates the determinant is back to 1, so the normalised emittance is conserved.

### For readers who know linear algebra and analytical mechanics

For a two-dimensional linear map, area preservation is equivalent to $\det M = 1$; in $2n$ dimensions Liouville's theorem requires the symplectic condition $M\tp J M = J$ ($J$ the standard symplectic matrix), which is stronger than $\det M = 1$ and guarantees that the sum of the "symplectic areas" of the conjugate pairs is conserved. Liouville's theorem itself follows from the divergence-free phase-space velocity of a Hamiltonian flow: $\partial\dot q/\partial q + \partial\dot p/\partial p = \partial^2 H/\partial q\partial p - \partial^2 H/\partial p\partial q = 0$. Under acceleration $(x, x')$ are not canonical coordinates, $(x, p_x)$ are; hence the geometric emittance is not conserved and the normalised one is.

## Going deeper: from three averages to Twiss parameters and the invariant {#phase-space-deep-twiss}

### Step 1: the emittance of four particles

Take four particles with coordinates $(x, x')$ = $(2, 1)$, $(-2, -1)$, $(1, -1)$, $(-1, 1)$ (mm, mrad), centred on the origin.

- $\langle x^2\rangle = (4 + 4 + 1 + 1)/4 = 2.5$ mm²;
- $\langle x'^2\rangle = (1 + 1 + 1 + 1)/4 = 1$ mrad²;
- $\langle x x'\rangle = (2 + 2 - 1 - 1)/4 = 0.5$ mm·mrad;
- $\varepsilon = \sqrt{2.5 \times 1 - 0.5^2} = \sqrt{2.25} = 1.5$ mm·mrad.

The correlation is positive: as a whole these particles are diverging.

### Step 2: why the correlation is subtracted

Imagine a "perfectly orderly" beam: every particle satisfies $x' = kx$ (for example, all come from one point). Then $\langle x x'\rangle = k\langle x^2\rangle$ and $\langle x'^2\rangle = k^2\langle x^2\rangle$, and

$$
\langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2 = k^2\langle x^2\rangle^2 - k^2\langle x^2\rangle^2 = 0 .
$$

The emittance is zero. Indeed, a lens with $1/f = k$ placed after it turns every angle into $x' - x/f = 0$, and the beam becomes perfectly parallel. So the part of $\langle x^2\rangle\langle x'^2\rangle$ that a lens can remove is $\langle x x'\rangle^2$, and what remains is the real disorder.

### Step 3: neither lens nor drift changes ε

Write $A = \langle x^2\rangle$, $B = \langle x x'\rangle$, $C = \langle x'^2\rangle$. Through a lens $x' \to x' - x/f$; expanding the squares and averaging gives

$$
A \to A, \qquad B \to B - \frac{A}{f}, \qquad C \to C - \frac{2B}{f} + \frac{A}{f^2}.
$$

The new $AC - B^2 = A\left(C - \tfrac{2B}{f} + \tfrac{A}{f^2}\right) - \left(B - \tfrac{A}{f}\right)^2 = AC - B^2$: every term containing $f$ cancels. The check for a drift is entirely similar (see [Drift](#drift-deep-matrix)).

### Step 4: Twiss parameters

Dividing the three averages by the emittance gives three numbers that describe only the shape:

$$
\beta_x = \frac{\langle x^2\rangle}{\varepsilon}, \qquad
\alpha_x = -\frac{\langle x x'\rangle}{\varepsilon}, \qquad
\gamma_x = \frac{\langle x'^2\rangle}{\varepsilon}.
$$

Dividing $\varepsilon^2 = \langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2$ by $\varepsilon^2$ immediately gives $\beta_x\gamma_x - \alpha_x^2 = 1$. For the four particles of step 1: $\beta = 2.5/1.5 = 1.667$, $\alpha = -0.5/1.5 = -0.333$, $\gamma = 1/1.5 = 0.667$, and $1.667 \times 0.667 - 0.111 = 1.000$. For the example beam: $0.37743 \times 3.796 - 0.658^2 = 1.4327 - 0.4328 = 1.000$.

### Step 5: the ellipse equation and its geometry

The points of the rms ellipse satisfy

$$
\gamma_x x^2 + 2\alpha_x x x' + \beta_x x'^2 = \varepsilon, \qquad \beta_x\gamma_x - \alpha_x^2 = 1
$$

(for $\alpha_x = 0$ it becomes $x^2/(\beta_x\varepsilon) + x'^2/(\gamma_x\varepsilon) = 1$, the school equation of an upright ellipse with semi-axes $\sqrt{\beta_x\varepsilon}$ and $\sqrt{\gamma_x\varepsilon}$). From this equation one reads off:

- the largest position $x_{\max} = \sqrt{\varepsilon\beta_x}$ and the largest angle $x'_{\max} = \sqrt{\varepsilon\gamma_x}$;
- the intercept with the $x$ axis, $x = \sqrt{\varepsilon/\gamma_x}$ (setting $x' = 0$), and with the $x'$ axis, $x' = \sqrt{\varepsilon/\beta_x}$ (setting $x = 0$);
- the tilt $\theta$ of the major axis against the $x$ axis, from $\tan 2\theta = 2\alpha_x/(\gamma_x - \beta_x)$.

The x plane of the example beam ($\alpha_x = -0.658$, $\beta_x = 0.377$ mm/mrad, $\gamma_x = 3.796$ mrad/mm, $\varepsilon = 2.412$ mm·mrad): the rms ellipse has $x_{\max}$ = 0.954 mm and $x'_{\max}$ = 3.026 mrad, and it crosses the two axes at 0.797 mm and 2.528 mrad.

The intercept with the $x$ axis, 0.797 mm, is exactly the rms size at the virtual waist: the drift shear does not move the points with $x' = 0$, and when the beam is traced back to its waist, the ellipse is upright and its width is the position of these points.

### Step 6: the invariant of one particle

For a single particle define

$$
J = \gamma_x(s)\,x^2 + 2\alpha_x(s)\,x x' + \beta_x(s)\,x'^2 .
$$

$J$ is the size of "the ellipse through this particle that has the shape of the beam's rms ellipse". In a linear system the Twiss parameters and the particle's coordinates both change with $s$, but $J$ stays constant (the proof goes as in step 3: substitute the element's transformation, expand, and every term containing the element's parameters cancels). The blue particle of the FODO animation starts at 1.5 rms, so $J = 1.5^2\varepsilon = 2.25\varepsilon$, and it always stays on the rms ellipse scaled by 1.5. The particle's path can be written $x(s) = \sqrt{J\beta_x(s)}\cos\left(\psi(s) + \psi_0\right)$; the increase of $\psi$ per cell is the phase advance, 75.6° by default in the FODO animation.

### Step 7: the update rule of the σ matrix

The expansion of step 3 for a lens can be written as one rule valid for every linear element. Write the element with the coefficients $a, b, c, d$ of step 2 of the previous going-deeper section ($x_1 = a x_0 + b x_0'$, $x_1' = c x_0 + d x_0'$), expand the squares and average:

$$
\begin{aligned}
\langle x_1^2\rangle &= a^2 A + 2ab\,B + b^2 C,\\
\langle x_1 x_1'\rangle &= ac\,A + (ad + bc)\,B + bd\,C,\\
\langle x_1'^2\rangle &= c^2 A + 2cd\,B + d^2 C .
\end{aligned}
$$

Written with matrix multiplication (row times column), this is exactly $\Sigma_1 = M\Sigma_0M\tp$ with $M = \begin{pmatrix} a & b\\ c & d\end{pmatrix}$, where $M\tp$ is $M$ with rows and columns swapped. Check: the example beam through a 0.1 m drift ($a = 1$, $b = 0.1$, $c = 0$, $d = 1$): $\langle x_1^2\rangle = 0.910 + 2 \times 0.1 \times 1.587 + 0.01 \times 9.157 = 1.319$ mm², an rms size of $\sqrt{1.319} = 1.149$ mm, as in the lab.

### For readers who know linear algebra

The rms ellipse is $X\tp\Sigma^{-1}X = 1$ with $X = (x, x')\tp$ and $\Sigma^{-1} = \varepsilon^{-1}\begin{pmatrix}\gamma_x & \alpha_x\\ \alpha_x & \beta_x\end{pmatrix}$. Deriving $\langle X_1 X_1\tp\rangle = M\langle X_0 X_0\tp\rangle M\tp$ takes one line and holds for any distribution; $\det\Sigma_1 = (\det M)^2\det\Sigma_0$ with $\det M = 1$ gives the conservation of the rms emittance at once. $J = X\tp(\Sigma/\varepsilon)^{-1}X$ is obviously invariant under $X \to MX$, $\Sigma \to M\Sigma M\tp$. In a periodic system the one-period matrix can be written $M = I\cos\mu + \begin{pmatrix}\alpha & \beta\\ -\gamma & -\alpha\end{pmatrix}\sin\mu$, and its Twiss parameters are those of the matched ellipse. With coupling one uses the 4×4 (or 6×6) σ matrix; the four-dimensional emittance $\sqrt{\det\Sigma_4}$ is conserved under symplectic maps, and the eigen-emittances are given by the eigenvalues of $J\Sigma$.

## Going deeper: converting the longitudinal coordinates {#phase-space-deep-longitudinal}

### Step 1: position difference to phase difference

A particle $z$ ahead of the reference particle arrives $z/(\beta c)$ earlier. The RF field turns by $360° \times f$ per second, so the phase is smaller by $360° \times f \times z/(\beta c) = 360° \times z/(\beta\lambda)$:

$$
\Delta\varphi = -360° \times \frac{z}{\beta\lambda}.
$$

For the example $\beta\lambda = 105.1$ mm, and 1.316 mm corresponds to $360 \times 1.316/105.1 = 4.51°$.

### Step 2: momentum difference to energy difference

From [Relativistic kinematics](#kinematics-deep-differential), $\Delta W = \beta^2\gamma\,mc^2\,\delta$. For the example $\beta^2\gamma\,mc^2 = 0.0032470 \times 1.001627 \times 938272$ keV = 3051.6 keV; times $\delta_{\mathrm{rms}} = 3.532\times10^{-3}$ this gives 10.78 keV, 0.71 % of the kinetic energy of 1527 keV.

### Step 3: converting the emittance

The area is multiplied by the conversion factors of both coordinates: $360/\beta\lambda$ for $z$ and $\beta^2\gamma\,mc^2$ for $\delta$. The longitudinal geometric emittance is $4.644$ mm·mrad $= 4.644\times10^{-6}$ m (with $z$ in m and $\delta$ dimensionless):

$$
4.644\times10^{-6} \times \frac{360°}{0.10513\ \text{m}} \times 3051.6\ \text{keV} = 48.5\ \text{deg·keV}.
$$

### Step 4: from twissz to the rms bunch length

In `twissz 0.04275 0.37292 0.26506` the last number is the normalised rms emittance; dividing by $\beta\gamma = 0.057076$ gives the geometric emittance 4.644 mm·mrad. $\sigma_z = \sqrt{\beta_z\varepsilon} = \sqrt{0.37292 \times 4.644} = 1.316$ mm; $\gamma_z = (1 + 0.04275^2)/0.37292 = 2.686$, so $\delta_{\mathrm{rms}} = \sqrt{2.686 \times 4.644} = 3.532$ mrad, i.e. $3.532\times10^{-3}$.

### For readers who know analytical mechanics

$(z, \delta)$ and $(\Delta\varphi, \Delta W)$ are both approximately conjugate pairs differing by constant factors, so their emittances differ only by a constant, and linear maps preserve area in both (with the matrix elements converted accordingly; the drift's $L/\gamma^2$, for example, becomes $2\pi L/(\beta^3\gamma^3\lambda)$ with $\Delta W$ in units of $mc^2$). The strictly canonical pairs are $(t, -E)$ or $(\varphi, -W/\omega)$; when $(z, \delta)$ are compared on one plane $s$, the second-order correction from the velocity itself depending on $\delta$ in $z = -\beta c\,\Delta t$ must also be kept in mind.

## In AVAS {#phase-space-in-avas}

### The twiss keywords

```avas-params
beam:twissx beam:twissy beam:twissz beam:distribution beam:particlenumber beam:initpos beam:initmom
```

The three numbers of `twissx α β ε` (likewise `twissy`, `twissz`):

- $\alpha$: dimensionless;
- $\beta$: in mm/mrad, numerically equal to m/rad;
- $\varepsilon$: the **normalised rms emittance** in π·mm·mrad.

> **AVAS convention**: the rms size is $\sigma_x = \sqrt{\beta_x\,\varepsilon_n/(\beta\gamma)}$ ($\sigma_x$ in mm for $\beta_x$ in mm/mrad and $\varepsilon_n$ in mm·mrad). `twissz` uses the same units and normalisation: $z$ in mm, $z' = \delta = \Delta p/p$ in mrad, normalised with the same factor $\beta\gamma$ (the user interface's linear preview converts this way, and this has been compared with the engine). Other programs may give the longitudinal emittance in $(\Delta\varphi, \Delta W)$ or with a different normalisation factor; check item by item when exchanging data.

### The initial beam of the example project

The example `beam.txt` (protons at 1.527 MeV, $\beta\gamma$ = 0.057076):

```text
twissx -0.65785 0.37743 0.13768
twissy -0.6341 0.37292 0.14059
twissz 0.04275 0.37292 0.26506
distribution GS GS
```

Converted with the convention above:

| Plane | Geometric rms emittance (mm·mrad) | rms size | rms angle / momentum deviation |
|---|---|---|---|
| x | 2.412 | 0.954 mm | 3.026 mrad |
| y | 2.463 | 0.958 mm | 3.043 mrad |
| z | 4.644 | 1.316 mm | $\delta_{\mathrm{rms}}$ = 3.532 × 10$^{-3}$ |

Transversely $\alpha < 0$: the beam diverges at the entrance. The longitudinal rms length of 1.316 mm corresponds to an rms phase width of 4.51° at 162.5 MHz; $\delta_{\mathrm{rms}}$ corresponds to an rms energy spread of 10.78 keV (0.71 %); the longitudinal geometric rms emittance converts to 48.5 deg·keV.

### Other keywords

- `distribution`: the transverse and longitudinal distribution types (KV, GS, PB, WB), see [Initial distributions: In AVAS](#distributions-in-avas).
- `particlenumber`: number of macroparticles; 1 means single-particle mode (then `twiss*`, `current` and `distribution` have no effect).
- `initpos`, `initmom`: initial position (mm) and angle (mrad) offset of the whole bunch, to model injection errors.
- `readparticledistribution`: read the particle distribution from a `.dst`/`.edst` file; then all keywords except `numofcharge` have no effect (the user interface switches with `use_dst`).

In the output, columns 16, 18 and 20 of `DataSet.txt` are the rms sizes of $x$, $y$, $z$ (m), corresponding directly to the $\sigma$ above; the reference-particle kinetic energy in column 0 gives $\beta\gamma$ at every position for converting normalised to geometric emittances.

### Parameters of the animations and labs

- Particle animation `drift`: `L` (drift length, m), `alpha` (Twiss α at the entrance), `emit` (normalised rms emittance, π·mm·mrad), `I` (current, mA); the entrance $\beta$ is that of the example beam. `fodo`: quadrupole length `Lq`, gradient `G`, drift `Ld`, number of cells `cells`, current `I`, mismatch factor `mismatch` (the entrance $\beta$ is multiplied by its square, the size by the factor itself). The animations draw about 60 particles from the rms matrix of the example beam; the dashed ellipse in the phase-space picture is computed from these particles.
- Lab `phase-space`: `alpha`, `beta` (mm/mrad), `emit` (normalised, π·mm·mrad), `W` (MeV), `mass` (MeV), `f` (lens focal length, m; 0 for no lens), `L` (m).

## Further reading {#phase-space-reading}

- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, 2008: emittance, Liouville's theorem, KV and other distributions, rms-equivalent beams.
- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: transverse dynamics and Twiss parameters.
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18 (1971) 1101: the introduction of the rms emittance.
- K. Floettmann, "Some basic features of the beam emittance", *Phys. Rev. ST Accel. Beams* 6, 034202 (2003): normalised emittance and its relation to energy spread.
- Continue with: [Initial distributions](#distributions), [Linear optics](#linear-optics), [Drift](#drift).
