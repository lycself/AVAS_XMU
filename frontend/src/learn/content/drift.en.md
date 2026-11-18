# Drift

A drift is the simplest part of an accelerator: a metal pipe pumped down to vacuum, with no electric and no magnetic field inside. A particle that flies in feels no force at all; like a spaceship coasting in space, it moves on in a straight line at constant speed.

The odd thing is that a beam leaving a drift is clearly wider than when it went in. Nothing pushes the particles, so why does the beam spread out? This chapter answers that question. The answer is simple, and it is the key to every element that follows.

First, have a look. In the animation below the orange dots are 60 protons of the example beam (they fly at about 6 % of the speed of light), and the two blue dots are two particles we follow in particular. Press play and watch the particles fly from the left (the entrance) to the right (the exit).

```avas-motion
drift
```

You have probably seen it already: **every particle flies straight, but each in a slightly different direction**, so they drift apart. The two long pictures show the paths seen from above and from the side; the square on the right is the cross-section "looking into the beam", as if you stood at the end of the pipe with a swarm of particles flying towards you.

## What a drift is and why it is there {#drift-what}

A linear accelerator is a long row of "elements" placed end to end: RF cavities that accelerate the particles, magnets that keep the beam together, instruments that measure it. Between the elements there is always some space for flanges, valves, bellows and measuring devices. There is no electromagnetic field in these gaps, so for the particles they are **drifts**.

Drifts are therefore everywhere in an accelerator, often more than half of its length. A drift neither accelerates nor focuses, but the beam changes inside it by itself, and that is what we look at now.

> **A few words**: a **particle** is one proton (or electron, or ion). A **beam** is a large swarm of particles flying in the same direction; one "bunch" of the example beam holds about seven million protons, which a simulation represents by a few thousand "macro-particles". **Transverse** means the two directions perpendicular to the motion, horizontal $x$ and vertical $y$; **longitudinal** means the direction of motion, $z$.

## Why the beam grows: the torch {#drift-spread}

Shine a torch at a distant wall. Every ray from the bulb is straight, yet the farther the wall, the larger the spot. The reason is that the rays point in slightly different **directions**: some a little to the left, some a little to the right, and the farther they go, the more they separate.

A beam is exactly the same. The distance of a particle from the central axis is its **position** $x$; the angle between its direction and the axis is its **angle** $x'$. In a drift there is no force, so the angle stays the same, and the position grows a little with every bit of distance travelled:

$$
x = x_0 + x' \cdot s
$$

Here $x_0$ is the position at the entrance and $s$ the distance travelled. It is the school formula "displacement = speed × time" for uniform motion, with the time replaced by the distance travelled.

Angles in an accelerator are small and usually given in **milliradians** (mrad, a thousandth of a radian). A handy rule: **a particle with an angle of 1 mrad moves 1 mm off course for every metre it travels**. Most particles of the example beam have angles of about ±3 mrad, so the beam grows by about 3 mm per metre.

In the animation, make the drift longer and watch the beam at the exit get wider; watch the two blue particles: they keep flying straight, only in different directions.

## Describing a beam: size, angle and emittance {#drift-size}

A beam has thousands of particles; we cannot list them one by one. Physicists describe it with two "typical values":

- **rms size**: the typical distance of the particles from the centre (precisely the root mean square: average the squares of all positions and take the square root). At the entrance the example beam has an rms size of about 0.95 mm; nearly all particles are within ±3 mm.
- **rms angle** (also rms divergence): the typical size of the angles, about 3 mrad for the example beam.

The picture at the bottom right of the animation draws every particle as a dot, with its position $x$ across and its angle $x'$ up. This picture is called **phase space**. In it, a beam roughly fills an ellipse (the dashed line is its rms ellipse).

In a drift something interesting happens in phase space: every dot keeps its height (the angle does not change) and moves sideways by $x' \cdot s$ (the position changes). Dots with a positive angle move right, dots with a negative angle move left, and the ellipse is "pushed over" like a stack of playing cards. However far it is pushed, **the area of the ellipse stays the same**.

That area is the **emittance**. It measures how "disordered" a beam is: the more spread out the positions and the angles, the larger the area. Without forces the emittance stays constant, and that remains true in almost every element that follows. The "emittance" slider of the animation changes this area: double it, and the beam is much wider after the same drift.

## The waist: first narrower, then wider {#drift-waist}

Does a beam always get wider in a drift? Not necessarily. If the particles come in heading towards the axis (like the light converging behind a lens), the beam first gets narrower, becomes narrowest at one point and then widens again. The narrowest point is called the **waist**.

The animation shows this case by default (the "converging or diverging" slider is positive): the beam closes in, is narrowest about 0.19 m after the entrance with an rms size of about 0.68 mm, and then grows all the way to about 4.8 mm at 1.5 m. Move the slider to a negative value: the beam diverges from the start and there is no waist to see.

Near a waist there is a very simple rule. Let the rms size at the waist be $\sigma_w$, the rms angle $\sigma'$ and the distance from the waist $d$; then

$$
\sigma(d)^2 = \sigma_w^2 + (\sigma' \cdot d)^2
$$

Like Pythagoras: the width of the waist itself plus the offset that the angles build up over the distance $d$. Check it with the animation's defaults: $\sigma_w = 0.68$ mm, $\sigma' = 3.58$ mrad and $d = 1.5 - 0.19 = 1.31$ m give $\sigma \approx 4.74$ mm, the same as the 4.74 mm computed without space charge.

An even more important result: **the size at the waist times the angle is the emittance** ($\sigma_w \cdot \sigma' \approx 2.4$ mm·mrad, exactly the geometric emittance of the example beam). For a fixed emittance, a narrower waist needs larger angles, so the beam grows faster away from it. Pull the slider to 3: the waist shrinks to about 0.30 mm, but 1.5 m later the beam is 11 mm wide. A beam that is both very narrow and hardly diverging is impossible; that is what emittance means.

## Longitudinally: runners pull apart {#drift-longitudinal}

The beam spreads sideways; what about the direction of motion? The beam of an RF accelerator travels in clumps, each called a **bunch**. The particles of a bunch do not all have the same speed either: some are a little faster, some a little slower.

Picture a group of runners starting together at slightly different speeds. The farther they run, the more the fast ones pull away from the slow ones. A bunch in a drift does the same: the fast particles move to the front, the slow ones fall behind, and the bunch gets longer. This is called **debunching**.

How fast a bunch debunches depends on how fast the particles go. Well below the speed of light (the example protons fly at 6 % of it), a little more energy means a little more speed, and the bunch lengthens about as fast as the beam spreads sideways: after 1 m the example bunch is about three times as long. But close to the speed of light more energy hardly makes a particle faster (nothing can exceed the speed of light), so the bunch hardly lengthens: 10 MeV electrons already fly at 99.9 % of the speed of light, and over the same metre their bunch length barely changes.

This is why the cavities of a low-energy proton linac cannot be far apart: a bunch that has grown too long reaches the next cavity with its particles meeting the field at different phases, some gaining more energy, some less, some getting lost. The derivation is in the "going deeper" part below.

## Space charge: the particles push each other {#drift-space-charge}

So far we assumed that the particles do not affect each other. In fact protons are positively charged, like charges repel, and every proton in the beam is pushed outwards by all the others. This is the **space-charge** effect. So even in a drift without applied fields the particles are not entirely free of forces.

The more "crowded" the beam (higher current, smaller size, lower speed), the stronger the repulsion. In the animation, raise the "beam current" from 0.184 mA (the example beam) to 10 mA and watch the beam spread faster: after 1.5 m the rms size grows from about 4.8 mm to about 7.7 mm. With the actual example beam over 1 m of drift the rms size is:

| Average current | rms $x$ after 1 m |
|---|---|
| 0 | 3.64 mm |
| 0.184 mA (example) | 3.69 mm |
| 1 mA | 3.93 mm |
| 10 mA | 6.08 mm |

At the example current space charge widens the beam by only 1.5 %; the same beam at 10 mA would be 67 % wider. Low-energy, high-current accelerators therefore pack their elements tightly. Space charge is treated fully in the [Space charge](#space-charge) chapter.

## Think about it {#drift-think}

> **Think about it 1**: in a drift no particle changes its angle. Why does the ellipse in phase space tilt?
>
> Answer: it is not one particle that changes but how the particles are arranged. After some distance the particles with a positive angle have all moved to the positive side, those with a negative angle to the negative side; position and angle now "share their sign", and the ellipse tilts.

> **Think about it 2**: in the animation, set "converging or diverging" to 1 and then to −1. Is the rms size at the entrance the same? At the exit?
>
> Answer: the same at the entrance (the animation fixes the entrance size), different at the exit: about 4.8 mm after 1.5 m when converging (+1), about 6.2 mm when diverging (−1). Both waists are equally narrow, but the "waist" of the diverging beam is 0.19 m upstream of the entrance (an invisible virtual waist), 0.38 m farther from the exit. The Pythagoras rule with $d$ = 1.31 m and 1.69 m gives the numbers (4.74 mm and 6.08 mm without space charge).

> **Think about it 3**: why is it wrong to say that a drift "does nothing"?
>
> Answer: it changes no particle's momentum, but it turns the existing differences in angle and speed into differences in position, and with current there is space charge. In a multi-particle simulation of AVAS a drift computes space charge and checks whether particles hit the wall as usual.

## Common misconceptions {#drift-misconceptions}

- **Does the beam always grow in a drift?** Not necessarily. A converging beam first shrinks to a waist and then grows.
- **Do the angles never change in a drift?** Only strictly without space charge. With current, the repulsion between the particles slowly increases the angles.
- **Is the longitudinal motion "frozen" in a drift?** Only approximately, for particles close to the speed of light (for example high-energy electrons). A bunch of low-energy protons lengthens quickly in a drift.
- **The emittance changed in a drift?** Without space charge it must not. If a multi-particle simulation shows a change, first check whether particles were lost on the wall (lost particles change the statistics), then look at space charge.

## Going deeper: from one particle to a beam — transfer matrices and Twiss parameters {#drift-deep-matrix}

The main text described one particle with $x = x_0 + x' s$. Accelerator physicists write the same thing as "transfer matrices" and "Twiss parameters"; textbooks and AVAS's input files use this language. This section builds it in five steps, each only a little beyond the previous one: the first four use school algebra only, the last paragraph is for readers who know linear algebra.

### Step 1: the "state" of a particle is two numbers

In one transverse plane (say horizontal) a particle is described by two numbers: its position $x$ and its angle $x'$. In a drift the angle stays and the position grows with it; a drift of length $L$ turns the entrance values $(x_0, x_0')$ into

$$
x = x_0 + L\,x_0', \qquad x' = x_0'.
$$

Example: $x_0 = 1$ mm and $x_0' = 2$ mrad at the entrance, $L = 0.5$ m: at the exit $x = 1 + 0.5 \times 2 = 2$ mm, and the angle is still 2 mrad (mm, mrad and m combine without conversion, because 1 mrad × 1 m = 1 mm).

### Step 2: writing the two lines as a "conversion table"

In the two lines above, each exit number is "coefficients times the two entrance numbers, added up". Arranging the coefficients in a table of 2 rows and 2 columns gives a **matrix**:

$$
\begin{pmatrix} x \\ x' \end{pmatrix}
=
\begin{pmatrix} 1 & L \\ 0 & 1 \end{pmatrix}
\begin{pmatrix} x_0 \\ x_0' \end{pmatrix}.
$$

How to read it: multiply the first row $(1, L)$ with $(x_0, x_0')$ entry by entry and add, giving $x = 1\cdot x_0 + L\cdot x_0'$; the second row $(0, 1)$ gives $x' = x_0'$. In general

$$
\begin{pmatrix} a & b \\ c & d \end{pmatrix}
\begin{pmatrix} u \\ v \end{pmatrix}
=
\begin{pmatrix} a u + b v \\ c u + d v \end{pmatrix}.
$$

Why bother? Because **a row of elements is a row of matrices**. First $L_1$, then $L_2$: put the result of the first drift into the second,

$$
x = (x_0 + L_1 x_0') + L_2 x_0' = x_0 + (L_1 + L_2)\,x_0',
$$

which is one drift of $L_1 + L_2$, as expected. In matrix language this step is multiplying two tables (the element passed later is written on the left). However many elements a beam line has, the tables can be multiplied one after another into one table from entrance to exit. The quadrupole of the next chapter has its own table, so drifts and quadrupoles can be combined freely.

### Step 3: adding the vertical and the forward direction

The vertical direction is exactly like the horizontal one: $y = y_0 + L\,y_0'$. In the forward (longitudinal) direction the position $z$ of a particle relative to the reference particle changes too, because the speeds differ; the next section derives $z = z_0 + (L/\gamma^2)\,\delta$, with $\delta = \Delta p/p$ the relative momentum deviation and $\gamma$ the relativistic factor (about 1 at low speed). With the six numbers $(x, x', y, y', z, \delta)$ together, the table of a drift has 6 rows and 6 columns:

$$
M_{\text{drift}}(L) =
\begin{pmatrix}
1 & L & 0 & 0 & 0 & 0\\
0 & 1 & 0 & 0 & 0 & 0\\
0 & 0 & 1 & L & 0 & 0\\
0 & 0 & 0 & 1 & 0 & 0\\
0 & 0 & 0 & 0 & 1 & L/\gamma^2\\
0 & 0 & 0 & 0 & 0 & 1
\end{pmatrix}.
$$

It is mostly zeros: the three directions do not affect each other, and each is a 2 × 2 table like the one of step 2. This is exactly the matrix AVAS's linear model uses for a drift.

### Step 4: from one particle to a beam

A beam has thousands of particles, and what we care about are three averages (the brackets $\langle\ \rangle$ mean the average over all particles; the beam centre is taken at 0 here):

- $\langle x^2 \rangle$: the average of the squared positions; its square root is the rms size $\sigma$;
- $\langle x'^2 \rangle$: the average of the squared angles; its square root is the rms angle $\sigma'$;
- $\langle x x' \rangle$: the average of position times angle, which measures their **correlation**: positive means "particles on the right mostly fly to the right" (the beam diverges), negative "particles on the right mostly fly to the left" (it converges).

Square $x = x_0 + L x_0'$ and average, with the school formula for a square:

$$
\begin{aligned}
\langle x^2 \rangle &= \langle x_0^2 \rangle + 2L\,\langle x_0 x_0' \rangle + L^2\,\langle x_0'^2 \rangle,\\
\langle x x' \rangle &= \langle x_0 x_0' \rangle + L\,\langle x_0'^2 \rangle,\\
\langle x'^2 \rangle &= \langle x_0'^2 \rangle .
\end{aligned}
$$

These three lines hold every result of the main text:

- **The angles do not change**: the third line.
- **The waist**: the beam is narrowest where $\langle x x' \rangle = 0$ (the first line is an upward parabola in $L$, and at its vertex $\langle x x' \rangle = 0$). The second line gives the waist at $L_w = -\langle x_0 x_0'\rangle/\langle x_0'^2\rangle$ from the entrance: for a beam converging at the entrance ($\langle x_0 x_0' \rangle < 0$) $L_w > 0$, the waist is downstream.
- **The Pythagoras rule**: starting at the waist (where $\langle x x' \rangle = 0$), the first line becomes $\sigma^2 = \sigma_w^2 + \sigma'^2 d^2$.
- **Emittance conservation**: define $\varepsilon^2 = \langle x^2\rangle\langle x'^2\rangle - \langle x x'\rangle^2$. With $A = \langle x_0^2\rangle$, $B = \langle x_0x_0'\rangle$, $C = \langle x_0'^2\rangle$ the three lines give

$$
(A + 2LB + L^2C)\,C - (B + LC)^2 = AC - B^2,
$$

all terms with $L$ cancel, so $\varepsilon$ does not depend on the drift length. $\varepsilon$ is the "area of the ellipse in phase space" of the main text (the rms ellipse has area $\pi\varepsilon$); at the waist $\varepsilon = \sigma_w\sigma'$.

### Step 5: Twiss parameters — the same information in another notation

Divide the three averages by the emittance and you get three numbers that describe only the "shape of the ellipse", not its size: the **Twiss parameters**

$$
\beta = \frac{\langle x^2\rangle}{\varepsilon}, \qquad
\alpha = -\frac{\langle x x'\rangle}{\varepsilon}, \qquad
\gamma = \frac{\langle x'^2\rangle}{\varepsilon},
$$

which always satisfy $\beta\gamma - \alpha^2 = 1$ (straight from the definition of $\varepsilon$). $\beta$ sets the size ($\sigma = \sqrt{\beta\varepsilon}$), $\gamma$ the angles ($\sigma' = \sqrt{\gamma\varepsilon}$), $\alpha$ the tilt: $\alpha > 0$ converging, $\alpha < 0$ diverging. This $\gamma$ has nothing to do with the relativistic factor $\gamma$; they merely share a name.

Dividing the three lines of step 4 by $\varepsilon$ gives the textbook Twiss formulas of a drift:

$$
\begin{aligned}
\beta(s) &= \beta_0 - 2\alpha_0 s + \gamma_0 s^2,\\
\alpha(s) &= \alpha_0 - \gamma_0 s,\\
\gamma(s) &= \gamma_0 ,
\end{aligned}
$$

with the waist at $s_w = \alpha_0/\gamma_0$, $\beta_w = 1/\gamma_0$ there, and $\beta(s) = \beta_w + (s - s_w)^2/\beta_w$.

Check with the example beam: beam.txt has $\alpha_x = -0.658$ and $\beta_x = 0.377$ mm/mrad, so $\gamma_x = (1 + 0.658^2)/0.377 = 3.80$ mrad/mm. The geometric emittance is $\varepsilon = 2.41$ mm·mrad (how it follows from the numbers in beam.txt is in "Going deeper: numbers for the example beam" at the end of this chapter), so $\sigma = \sqrt{0.377 \times 2.41} = 0.954$ mm and $\sigma' = \sqrt{3.80 \times 2.41} = 3.03$ mrad. $\alpha < 0$: the beam diverges, with its waist at $s_w = -0.658/3.80 = -0.173$ m, 0.17 m upstream of the entrance (an invisible virtual waist), where $\sigma_w = \sqrt{2.41/3.80} = 0.80$ mm.

> **Think about it**: the rms angle stays, so why does $\alpha$ change? Because $\alpha$ describes the correlation of position and angle, and the second line of step 4 shows that a drift keeps increasing $\langle x x'\rangle$.

### For readers who know linear algebra

With the second moments arranged in the symmetric matrix $\Sigma = \langle \boldsymbol{x}\boldsymbol{x}\tp\rangle$, step 4 is $\Sigma_1 = M\Sigma_0 M\tp$, valid for every linear element. Each 2 × 2 block has $\det\Sigma = \varepsilon^2$, and $\det(M\Sigma M\tp) = (\det M)^2\det\Sigma$, so the emittance is conserved whenever $\det M = 1$. The drift matrix satisfies the stronger symplectic condition $M\tp J M = J$ ($J$ the standard symplectic matrix), the linear form of Liouville's theorem; a drift does not accelerate, $\beta\gamma$ stays, and so does the normalized emittance. In a multi-particle simulation **without space charge** the rms emittances in DataSet.txt should stay constant through a drift (within statistical noise); a clear change usually comes from the non-linear part of space charge or from particle losses (particles removed by the aperture change the statistics).

## Going deeper: why bunches lengthen — speed differences, R₅₆ and the RF phase {#drift-deep-longitudinal}

The main text said "a little more energy means a little more speed, so the bunch lengthens; close to the speed of light it hardly lengthens any more". This section turns that sentence into numbers, starting from uniform motion at school level and ending with relativity.

### Step 1: the speed limit

Special relativity says no particle can move faster than light, $c$. Two dimensionless numbers describe how fast a particle is:

$$
\gamma = 1 + \frac{W}{mc^2}, \qquad \beta = \frac{v}{c} = \sqrt{1 - \frac{1}{\gamma^2}} .
$$

$W$ is the kinetic energy, $mc^2$ the rest energy (938.27 MeV for a proton, 0.511 MeV for an electron). The example protons have $W = 1.527$ MeV, $\gamma = 1.0016$ and $\beta = 0.0570$: only 5.7 % of the speed of light, almost "Newtonian" particles. A 1 GeV proton has $\beta = 0.875$; a 10 MeV electron $\beta = 0.9988$, right at the speed of light. See [Kinematics](#kinematics).

### Step 2: how much speed difference for a momentum difference

At low speed the momentum is $p = mv$: 1 % more momentum, 1 % more speed. At high speed $p = \gamma m v$, and most of the extra momentum goes into a larger $\gamma$ while the speed hardly grows. Exactly:

$$
\frac{\Delta v}{v} = \frac{1}{\gamma^2}\,\frac{\Delta p}{p} = \frac{\delta}{\gamma^2},
$$

with $\delta = \Delta p/p$. The example protons have $1/\gamma^2 = 0.997$, almost the low-speed "1 % for 1 %"; 10 MeV electrons have $1/\gamma^2 = 0.0024$: 1 % more momentum gives only 0.0024 % more speed. (The derivation is at the end of this section.)

### Step 3: speed differences become position differences: R₅₆

The reference particle crosses a length $L$ at speed $v$ in the time $t = L/v$. In the same time a particle faster by $\Delta v$ travels $\Delta v \cdot t$ farther: school "distance = speed × time". With step 2:

$$
z = z_0 + \Delta v \cdot \frac{L}{v} = z_0 + \frac{L}{\gamma^2}\,\delta, \qquad R_{56} = \frac{L}{\gamma^2}.
$$

$z$ is the particle's longitudinal position relative to the reference particle ($z > 0$ ahead). $R_{56}$ is the number in row 5, column 6 of the 6 × 6 transfer matrix (the bottom right of the matrix in the previous section), the counterpart of the transverse $L$: the longitudinal "angle" is $\delta/\gamma^2$. $R_{56}$ per metre of drift:

| Particle and kinetic energy | $\gamma$ | $\beta$ | $R_{56}/L = 1/\gamma^2$ |
|---|---|---|---|
| proton 1.527 MeV (example) | 1.0016 | 0.0570 | 0.997 |
| proton 10 MeV | 1.0107 | 0.145 | 0.979 |
| proton 100 MeV | 1.107 | 0.428 | 0.817 |
| proton 1 GeV | 2.066 | 0.875 | 0.234 |
| electron 1 MeV | 2.957 | 0.941 | 0.114 |
| electron 10 MeV | 20.57 | 0.9988 | 0.0024 |

The $R_{56}$ of 10 MeV electrons is only 1/420 of the example protons': over one metre an electron with $\delta = 10^{-3}$ moves only 2.4 μm out of place.

### Step 4: an RF cavity sees phases, not millimetres

The electric field of an RF cavity oscillates with frequency $f$ (162.5 MHz in the example), and a particle is accelerated only if it crosses while the field points forward. During one oscillation of the field a particle flies $\beta\lambda = \beta c/f$, 105.1 mm for the example beam. A particle $z$ ahead of the reference particle therefore arrives early by the phase

$$
\Delta\varphi = -360^\circ \times \frac{z}{\beta\lambda},
$$

3.42° per mm for the example beam. The example's rms momentum deviation $\delta = 3.53\ \mathrm{mrad}$ (rms energy spread 10.8 keV) widens the rms phase width from 4.5° to 12.7° in one metre of drift. The longer the bunch, the more spread the phases its particles meet in the next cavity and the more different the energies they gain; near the width of the longitudinal acceptance (about 100° for the example cavity at $-33°$, see [Longitudinal dynamics](#longitudinal-separatrix)) particles are lost.

Between two cavities the phase at which the reference particle reaches the next cavity is set by $360^\circ \times L/(\beta\lambda)$; the example beam advances 342.4° per 0.1 m. When a drift between cavities is changed, the meaning of the downstream cavity phases depends on the V3 setting of `field`: with V3 = 0 the engine solves for the synchronous phase again, with V3 = 2 the absolute phase stays and the entrance phase changes (see [RF cavity](#rf-cavity) and [Longitudinal dynamics](#longitudinal-in-avas)).

### Step 5: using the lengthening the other way round: bunching

If the bunch is first given **less energy at its head and more at its tail**, a following drift lets the tail catch up with the head, and the bunch gets shorter. An RF cavity at the synchronous phase $\varphi_s = -90°$ does not accelerate but produces exactly this "slow head, fast tail" energy difference, so it is a **buncher**; the drift turns the energy differences into position differences. With the example cavity (linear preview): hwr010 at $\varphi_s = -90°$ leaves the energy unchanged; with 5 cm of drift in front, the rms $z$ is 0.77 mm at the cavity exit, reaches a minimum of about 0.64 mm some 6 cm downstream, and grows again because of the energy spread the bunch now carries. Watch the dashed rms $z$ curve in the envelope plot of the lab below; move `phase` away from −90° and the cavity starts to accelerate as well as bunch. The bunchers of the medium-energy transport work this way, see [Medium-energy transport](#journey-mebt).

```avas-lab
rf-cavity phase=-90 Ke=1.36 W=1.527
```

### For readers who know calculus

From $E^2 = p^2c^2 + m^2c^4$ we get $\dd E/E = \beta^2\,\dd p/p$; the velocity is $v = pc^2/E$, so

$$
\frac{\Delta v}{v} = \frac{\Delta p}{p} - \frac{\Delta E}{E} = \left(1 - \beta^2\right)\frac{\Delta p}{p} = \frac{\delta}{\gamma^2}.
$$

In $(\Delta\varphi, \Delta W)$ coordinates the drift coefficient becomes the $2\pi/(\beta^3\gamma^3\lambda)$ of [Longitudinal dynamics](#longitudinal-stability) (with $\Delta W$ in units of $mc^2$): $z \to \Delta\varphi$ contributes $2\pi/(\beta\lambda)$, $\delta \to \Delta W$ contributes $1/(\beta^2\gamma)$, times $1/\gamma^2$; do not mix the two. The path-length difference $\tfrac12 L\,(x'^2 + y'^2)$ from transverse motion is neglected; it is of second order and does not appear in the linear matrix.

## Going deeper: numbers for the example beam {#drift-deep-example}

This section recomputes the formulas of the two previous sections with the real parameters of the example project `examples/hwr010`; you can follow with a calculator.

### From beam.txt to the rms size

beam.txt says $\alpha_x = -0.65785$, $\beta_x = 0.37743$ mm/mrad, $\varepsilon_{n,x} = 0.13768\ \pi$·mm·mrad. The last number is the **normalized** emittance: when particles are accelerated their angles shrink (the forward momentum grows), and the geometric emittance shrinks with them; multiplied by $\beta\gamma$ it becomes the "normalized emittance", which stays constant during acceleration, so input files use it. The example protons have $\beta\gamma = 0.0570 \times 1.0016 = 0.05708$, so the geometric emittance is

$$
\varepsilon = \frac{\varepsilon_n}{\beta\gamma} = \frac{0.13768}{0.05708} = 2.41\ \text{mm·mrad},
\qquad
\sigma_x = \sqrt{\beta_x\varepsilon} = 0.954\ \text{mm}.
$$

### Numbers along a drift

With the formulas of the first section (zero current):

| Drift length | rms $x$ | rms $y$ | rms $z$ | rms phase width (162.5 MHz) |
|---|---|---|---|---|
| 0 | 0.954 mm | 0.958 mm | 1.316 mm | 4.5° |
| 0.3 m | 1.64 mm | 1.64 mm | 1.65 mm | 5.7° |
| 0.5 m | 2.19 mm | 2.19 mm | 2.15 mm | 7.4° |
| 1.0 m | 3.64 mm | 3.65 mm | 3.71 mm | 12.7° |

The phase width is $360^\circ \times z/(\beta\lambda)$ with $\beta\lambda = 105.1$ mm. Incidentally the beam grows about equally fast in both directions: the rms angle of 3.03 mrad is close to the longitudinal "angle" rms $\delta/\gamma^2$ of 3.52 mrad.

### Try it yourself

The lab below puts the example beam's $x$-plane parameters into a drift (`f=0` means no lens). Drag `L`: the ellipse keeps its area, its tilt increases with distance, $\alpha$ becomes more negative, and the rms size grows while the rms angle stays. Set `alpha` to a positive value (e.g. 1.5) to check the waist position $s_w = \alpha_0/\gamma_0$.

```avas-lab
phase-space alpha=-0.65785 beta=0.37743 emit=0.13768 W=1.527 mass=938.272 f=0 L=0.5
```

Here is the same 1 m drift as the lattice editor shows it (switch between 2D and 3D, click the element for its details):

```avas-beamline
drift 1.0 0.02 0
```

## In AVAS {#drift-in-avas}

### Syntax

A drift is written in the lattice as

```text
drift L R 0
```

```avas-params
drift
```

- `L`: length in **m** (TraceWin lattices use mm; convert when importing or comparing).
- `R`: aperture radius in m, used for loss detection: a particle whose transverse position exceeds this radius counts as lost.
- The third parameter is reserved; write 0.

> **AVAS convention**: in the schema `drift` is a **field-model element**, advanced by the t-code (time as independent variable, integrated with the step set by `steppercycle` in input.txt). Space charge is computed in drifts as usual. By contrast, quadrupoles, solenoids, dipoles, steerers and edges are z-code matrix-model elements (see [Quadrupole](#quadrupole)).

### Very short drifts at both ends

The AVAS manual advises against matrix-model elements as the first or the last element of a multi-particle model and suggests a very short drift to avoid this. For example:

```text
start
drift 0.000001 0.02 0
quad 0.1 0.02 0 6
drift 0.2 0.02 0
quad 0.1 0.02 0 -6
drift 0.2 0.02 0
end
```

Both ends are then t-code elements. The final drift also leaves a margin for output planes (an output plane too close to the end of the lattice fails, see [Multi-particle simulation](#simulation)).

### Aperture and independent boundary

The drift's `R` is a circular aperture for loss detection. With `boundary 1` in input.txt the engine uses the independent boundary in Boundary.txt instead, and the radii of all lattice elements no longer apply:

```avas-params
input:boundary
```

### Typical values

| Parameter | Meaning | Typical values (low-energy proton linac) |
|---|---|---|
| `L` | drift length | 0.05–0.3 m between elements; 0.1–0.5 m between cavities; up to several metres in transfer lines |
| `R` | aperture radius | 0.01–0.03 m, often the same as neighbouring elements |

Choosing a drift length is a trade-off:

- **Transverse**: the longer the distance between two focusing elements, the more the beam grows in between and the stronger the focusing must be; in a periodic structure the length directly sets the phase advance per period (see the FODO lab in [Linear optics](#linear-optics-periodic)).
- **Longitudinal**: low-energy beams debunch quickly. The example beam has $1/\gamma^2 = 0.99675$, $L/\gamma^2 \approx L$; the longer the drift between cavities, the longer the bunch that reaches the next one.
- **Aperture**: as a rule of thumb the aperture should be at least 5–10 times the rms size (depending on halo and error budget), otherwise the losses become very sensitive.

### Common problems

- **Units**: AVAS lengths and apertures are in m; mm values copied from TraceWin make a drift 1000 times longer, and typically the beam is lost almost immediately.
- **Aperture 0 or too small**: particles are counted as lost at the first step.
- **Using a drift in place of a missing field map**: this removes all focusing and acceleration of the element; use the result only for comparison.
- **Emittance "changes"**: without space charge the emittance should be conserved in a drift; if it changes, check losses first (surviving macro-particles, column 28 of DataSet.txt).

### Watching debunching in the editor

To see longitudinal debunching, build a lattice with only `drift` in the visual editor and look at rms $z$ in the linear envelope preview; then run the same structure with the engine and compare with and without space charge (`spacecharge 0/1` in input.txt).

## Further reading {#drift-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH (2008): chapter 7 on transverse dynamics (matrix methods), chapter 6 on longitudinal motion.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH (2008): chapter 3 (beam optics and focusing systems without space charge).
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer (2015): transfer matrices and the evolution of Twiss parameters.
- Related chapters: [Phase space](#phase-space), [Linear optics](#linear-optics), [Longitudinal dynamics](#longitudinal), [Space charge](#space-charge); full bibliography in [References](#references).
