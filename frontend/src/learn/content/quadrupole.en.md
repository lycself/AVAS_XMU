# Quadrupole

The previous chapter showed that a beam spreads out by itself in a drift, growing by several millimetres per metre, while the vacuum pipe is only a centimetre or two in radius. For the beam to travel tens or hundreds of metres it has to be "gathered in" again every so often. The workhorse for this is the **quadrupole magnet**.

A quadrupole has an odd temper, though: it pushes particles towards the centre horizontally but away from it vertically, and it can never gather both directions at once. How can an element that "helps with one hand and hinders with the other" keep a beam together? That is what this chapter is about.

First, see what one quadrupole does. In the animation the beam passes a short drift, then a 0.1 m long quadrupole (the green band), then a 0.5 m drift.

```avas-motion
quad
```

Look at the upper picture (seen from above, horizontal direction $x$): after the quadrupole the particles turn towards the centre, the beam closes in and becomes very narrow behind it. Now the lower picture (seen from the side, vertical direction $y$): the same particles are pushed outwards, and the beam keeps growing. Drag the position bar so that the particles are inside the quadrupole and look at the cross-section on the right: four green poles, N and S in turn, and red arrows for the force on the particles, towards the centre horizontally and away from it vertically.

## Why a "magnetic lens" is needed {#quadrupole-why}

In a drift the example beam (protons at 1.5 MeV) grows by about 3 mm per metre. Left alone, after 2.5 m its rms size is 8 mm; three times the rms size is already beyond the 2 cm radius of the pipe, and many particles would hit the wall.

In optics, diverging light is brought back together by a lens. A lens has one key property: the farther a ray is from the centre, the more it is bent back, and **exactly in proportion to its distance from the centre**. Only then do all rays meet at the same focus.

A beam needs exactly the same thing: an element that pushes a particle back the harder the farther it is from the axis, with a force proportional to that distance, like a spring. Such an element is a **magnetic lens**, and the quadrupole is the most common one in accelerators: in the medium- and high-energy sections long rows of them keep the beam inside the pipe; where two structures meet, a few individually adjustable quadrupoles "reshape" the beam (called [matching](#matching)); before the end of the line they set the size of the spot on the target.

## How a magnetic field pushes a charged particle {#quadrupole-lorentz}

From school physics: a charged particle moving in a magnetic field feels the **Lorentz force**

$$
F = q\,v\,B
$$

(when the velocity is perpendicular to the field). $q$ is the charge, $v$ the speed and $B$ the magnetic flux density (in tesla, T). The force is perpendicular both to the velocity and to the field; its direction follows the right-hand rule: point the fingers of your right hand along the velocity of a positive charge, curl them towards the field, and the thumb gives the force (mathematically $\boldsymbol F = q\,\boldsymbol v \times \boldsymbol B$).

In an accelerator the particles fly almost along the central axis (we take it as $+z$, "coming out of the screen"), while the magnet's field lies in the cross-section ($x$ and $y$). So the force also lies in the cross-section, just right for pushing particles towards the centre or away from it.

How strongly a field acts on a particle depends on how "heavy" and how fast the particle is. A particle with momentum $p$ in a field $B$ moves on a circle of radius $\rho$, with $B\rho = p/q$. $B\rho$ is called the **magnetic rigidity**: "how hard it is to bend this particle with a magnetic field". The larger the momentum, the larger the rigidity and the harder the particle is to deflect. The example protons have $B\rho = 0.179$ T·m; accelerated to 10 MeV they have 0.458 T·m, and the same magnet does less than half as much to them.

## The field of a quadrupole {#quadrupole-field}

A quadrupole has four poles, north and south in turn, on the four diagonals of the cross-section. Here is the cross-section of one (a **schematic** computed from the parameters, not measured data). Switch to the $x$–$y$ cross-section and look at the arrows:

```avas-field
quadrupole G=6 L=0.1 R=0.02
```

Its field has two properties:

1. **The field is zero at the centre.** A particle flying exactly along the axis feels no force; a quadrupole only "deals with" particles away from the centre.
2. **The farther from the centre, the stronger the field, exactly in proportion**: $B = G \cdot r$, with $r$ the distance from the centre. The proportionality constant $G$ is the **gradient**, in T/m. With $G = 10$ T/m, for example, the field 1 mm from the centre is 0.01 T.

A field proportional to the distance means a force proportional to the distance: the "spring" a lens needs. Let us find the direction of the force (cross-sections are always **seen from downstream looking upstream**, with the beam coming out of the screen towards you, $x$ to the right and $y$ up). For $G > 0$:

- On the positive $x$ axis (right) the field points up; by the right-hand rule a positive charge is pushed **to the left**, towards the centre. On the negative half everything is reversed and the force points right, again towards the centre. So $x$ is **focusing**.
- On the positive $y$ axis (top) the field points right; a positive charge is pushed **up**, away from the centre. So $y$ is **defocusing**.

For $G < 0$ the two directions swap. Every quadrupole therefore "focuses one direction and defocuses the other", and which direction it focuses depends on the sign of the gradient and on the sign of the particle's charge.

**Could a magnet be designed that focuses both directions?** No. This is not a lack of ingenuity but a property of magnetic fields: field lines have no beginning and no end (there are no separate "magnetic charges"), and in a vacuum without currents no transverse magnetic field can push particles towards the centre everywhere. Wherever there is an inward push, there must be an outward push in the other direction, and just as strong. The exact proof is in the "going deeper" part.

## Focal length: how strong the lens is {#quadrupole-focal}

The impulse theorem from school gives the focal length of a quadrupole. Let a particle pass through a quadrupole of length $L$ at a distance $x$ from the centre:

- the force on it is $F = q\,v\,(G x)$;
- it takes the time $t = L/v$ to pass the magnet;
- the change of its transverse momentum (the impulse) is $F \cdot t = q\,G\,x\,L$;
- the transverse momentum divided by the forward momentum $p$ is the angle by which its path turns: $\theta = q\,G\,L\,x/p$.

The angle is proportional to $x$, which is exactly a lens. By the optics definition the angle is $x/f$, so the focal length is

$$
f = \frac{p}{q\,G\,L} = \frac{B\rho}{G\,L}.
$$

With the animation's defaults, $B\rho = 0.179$ T·m, $G = 10$ T/m and $L = 0.1$ m, we get $f = 0.18$ m. A particle 1 mm from the centre is turned back by 5.6 mrad, more than the typical angle of the example beam (3 mrad); for 1.5 MeV protons this is already a strong lens.

The formula tells three things, all of which you can try in the animation:

- **The larger the gradient, the shorter the focal length.** Set $G$ to 0 and the quadrupole becomes an ordinary pipe: at the exit both rms sizes are about 2.6 mm; at the default 10 T/m the horizontal size shrinks to 1.4 mm while the vertical one grows to 6.3 mm.
- **Reversing the gradient swaps the directions.** Set $G$ to −10: horizontally the beam spreads, vertically it closes in.
- **The "stiffer" the particle, the weaker the lens.** Set the kinetic energy to 10 MeV: the rigidity becomes 0.458 T·m and the focal length 0.46 m, much weaker focusing; to get the old focal length back the gradient must go up to about 26 T/m. This is why high-energy sections need stronger quadrupoles.

## Alternating: why push-and-pull gathers the beam {#quadrupole-alternating}

One quadrupole always defocuses one direction, so add a second one of opposite sign. But one focusing and one defocusing, equally strong: don't they simply cancel?

The answer hides in "the force is proportional to the distance from the centre". Look at the horizontal direction: the particle first enters the focusing quadrupole while it is fairly far from the centre and is pushed back hard; it heads towards the centre, and by the time it reaches the defocusing quadrupole it is closer, so the outward push it gets is smaller than the inward push before. Now the vertical direction: the particle is first pushed outwards by the defocusing quadrupole and arrives at the focusing one farther from the centre, so the inward push is even larger.

Both cases are "**pushed in when far out, pushed out when close in**", and together the inward push always wins. This is **alternating-gradient focusing**, also called **strong focusing**, one of the most important inventions in accelerator history (1950s).

The optics formula for two lenses gives the net effect: two lenses of focal length $+f$ and $-f$ at a distance $d$ combine to a focal length $F$ with

$$
\frac{1}{F} = \frac{d}{f^2},
$$

positive in both directions whichever lens comes first: both directions are focused.

A physically correct analogy: an upside-down pendulum falls over at the slightest touch, but if its pivot is shaken quickly up and down, the pendulum can stand upright (Kapitza's pendulum). The shaking alternately "hinders" and "rights" it, and the pendulum is righted most when it leans most, so on average there is a restoring force.

## The FODO channel: particles swinging about the centre {#quadrupole-fodo}

Repeat "focusing quadrupole, drift, defocusing quadrupole, drift" many times and you have the most common focusing structure of accelerators, the **FODO channel** (F focusing, D defocusing, O drift). The animation shows 6 FODO periods:

```avas-motion
fodo
```

This time the particles do not fly off but swing back and forth about the axis, like a ball rolling to and fro in a valley. Watch the two blue particles: they cross the centre again and again in one direction, and the whole beam stays in the middle of the pipe. The beam's width "breathes": horizontally it is widest in the focusing quadrupoles and narrowest in the defocusing ones, vertically the other way round.

How far a particle gets through its swing during one period is called the **phase advance** of the period. The animation's default is about 76°: a particle completes one full swing every 4.8 periods or so, about 1.25 swings in the 6 periods.

**Is a larger gradient always better?** No. In the animation increase the gradient $G$ slowly from 6 T/m: the phase advance grows, and at about 9.8 T/m the particles are "pushed past the centre" so much in each period that their swing no longer stays bounded; it grows from period to period and they soon hit the wall (red crosses). The period has become **unstable**. Like pushing a swing: with the right timing it stays steady, pushing too hard makes it go out of control. Designs usually keep the phase advance below 90°, with extra margin when space charge is strong.

Try two more things:

- Set "mismatch" to 1.3, so that the beam at the entrance is 30 % larger than the size this channel "likes": the width no longer breathes regularly but swells and shrinks, and the largest size grows from about 1.8 mm to about 3.1 mm. Making the beam fit the channel is what [matching](#matching) does.
- Raise the beam current to 10 mA: space charge pushes the particles apart and cancels part of the focusing; the beam is wider (up to about 4 mm) and the particles swing more slowly.

Here are 4 FODO periods as the lattice editor shows them (each quadrupole 0.1 m long at 6 T/m, 0.2 m apart). Click "3D" to look at the row of magnets from any angle and click an element for its details. The example beam's rms size stays below 3.4 mm over the 2.45 m, whereas a drift of the same length would let it grow to about 8 mm:

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

## Think about it {#quadrupole-think}

> **Think about it 1**: a quadrupole with $G > 0$ focuses protons horizontally. What if negatively charged ions (H⁻, say) come through?
>
> Answer: the force reverses: horizontal defocusing, vertical focusing. Which direction is focused depends on the signs of both the gradient and the charge.

> **Think about it 2**: the focusing and defocusing quadrupoles of a FODO channel have gradients of the same size. Someone says "alternating focusing works because the defocusing one is actually weaker". Right?
>
> Answer: no. Both are equally strong; the net focusing comes from the different distances of the particles from the centre in the two magnets, not from different strengths.

> **Think about it 3**: a particle flying exactly along the axis feels no force in a quadrupole. Why does the rms size still change?
>
> Answer: the rms size describes the whole beam, and almost every particle of the beam is a little off centre. The quadrupole pushes those off-centre particles.

## Common misconceptions {#quadrupole-misconceptions}

- **"$G > 0$ means focusing"**: a quadrupole always focuses one direction and defocuses the other. In AVAS $G > 0$ focuses positive charges in $x$ (horizontally).
- **"A larger gradient makes the beam smaller"**: beyond 180° of phase advance per period the period is unstable; beyond about 90° space charge can also drive envelope instabilities.
- **"The rms size is the edge of the beam"**: the plots show rms sizes; the beam edge is several rms out, so judge losses by comparing the aperture with several rms (see [Phase space](#phase-space)).
- **The thin-lens formula is enough**: the focal-length formula above assumes a "thin" quadrupole (the particle's position hardly changes inside). For strong focusing it is off by 10 % or more; designs should use the thick-lens matrices of the "going deeper" part.
- **The effective length equals the iron length**: a real quadrupole's field fades out gradually at both ends; the effective length is usually longer than the iron, by about one aperture radius.

## Going deeper: why magnetic fields can only focus this way {#quadrupole-deep-field}

The main text said that "no magnet can focus both directions at once". This section proves it step by step and explains why the poles of a quadrupole have their shape. The first four steps use school geometry and algebra only; the last paragraph is for readers who know vector analysis.

### Step 1: two rules a magnetic field obeys in vacuum

In the vacuum in the middle of the magnet, where the beam flies, there is no iron and no current. There the magnetic field must obey two rules:

1. **Field lines do not appear or vanish.** For any small box, as many field lines go in as come out (school physics: field lines are closed curves; there is no separate north or south pole).
2. **Where there is no current, the field does not "swirl".** Walk around any small closed loop and add up the component of the field along the path: the total is zero. (It is non-zero only when a current passes through the loop, which is why a wire carrying a current has a circular field around it.)

### Step 2: what the field looks like near the centre

Look at the middle of a long magnet, where the field lies in the cross-section ($B_x$, $B_y$), and only close to the centre. Close to the centre any smoothly varying quantity can be approximated by "a constant + a part proportional to $x$ + a part proportional to $y$". The constant part is a uniform field, which belongs to [dipoles](#dipole) and [steerers](#steerer); leave it out:

$$
B_x = a\,x + b\,y,\qquad B_y = c\,x + d\,y ,
$$

with four unknown numbers $a$, $b$, $c$, $d$ (in T/m).

### Step 3: each rule removes one number

**Rule 1**: put a small square of side $w$ near the centre (think of it as a very flat box). The difference between the field lines through its right and left sides is proportional to the difference of $B_x$ there, $a\,w$; between top and bottom it is $d\,w$. In and out must balance, so $a + d = 0$.

**Rule 2**: walk anticlockwise around the same square. Going up the right side adds $B_y$; going down the left side subtracts $B_y$; the difference is $c\,w$ times the side length. Going left along the top and right along the bottom gives $-b\,w$ times the side length. The total is zero, so $c - b = 0$, i.e. $b = c$.

Only two of the four numbers are free. The case $b = c \equiv G$, $a = d = 0$ is the **normal quadrupole field**:

$$
B_x = G\,y,\qquad B_y = G\,x ,
$$

where $G$ is the gradient of the main text and the field strength is $\sqrt{B_x^2 + B_y^2} = G\,r$. The other case, $a = -d$ with $b = c = 0$, is the **skew quadrupole field**: the same field turned by 45°, which mixes the $x$ and $y$ motions (coupling).

### Step 4: why both directions cannot be focused

A particle flying along $+z$ feels $F_x = -qvB_y = -qvG\,x$ and $F_y = +qvB_x = +qvG\,y$. Writing the forces like springs, $F_x = -K_x\,x$ and $F_y = -K_y\,y$, gives $K_x = qvG$ and $K_y = -qvG$, so

$$
K_x + K_y = 0 .
$$

Focusing needs $K > 0$ (a force towards the centre). The two "spring constants" always add up to zero, so if one is positive the other is negative: one direction is defocused exactly as much as the other is focused. The step used only $b = c$ from step 3, i.e. "no current, no swirl". So this is not a flaw of quadrupole design but a property of every static magnetic field (a form of Earnshaw's theorem). A [solenoid](#solenoid) escapes it because it uses the longitudinal field $B_z$ together with the particle's own transverse velocity, at the price of much weaker focusing.

> **Think about it**: can an electrostatic quadrupole lens (the electric quadrupole field of an RFQ is of this kind) escape the limit?
> Answer: no. In vacuum without charges the electric field also obeys "in equals out", so the two electric spring constants also add up to zero. Electric quadrupoles, like magnetic ones, focus one direction and defocus the other (see [RFQ](#journey-rfq)).

### Step 5: why the poles are hyperbolas

The surface of iron is approximately a magnetic "equipotential", just as the surface of a conductor is an equipotential in electrostatics. The magnetic scalar potential of the normal quadrupole field is $\Phi_m = -G\,x\,y$, and the equipotentials $x y = $ constant are hyperbolas. So the ideal pole surfaces are the hyperbolas $x y = \pm R^2/2$, with the four poles, alternately north and south, at $\pm 45^\circ$ and $\pm 135^\circ$; $R$ is the aperture radius (radius of the inscribed circle). In AVAS's cross-section convention (seen from downstream looking upstream, beam coming out of the screen, $x$ to the right, $y$ up), for $G > 0$ the upper-left and lower-right poles are north poles; the field lines leave them and flow to the south poles at the upper right and lower left.

The field at the pole tip is $B_{\text{pole}} = G\,R$. Iron starts to saturate around 1–1.5 T, so a normal-conducting iron quadrupole roughly obeys $G R \lesssim 1$ T: with a 2 cm aperture radius the gradient is limited to about 50 T/m, and the smaller the aperture, the higher the gradient. The $G = 6$ T/m of the main text has a pole-tip field of only 0.12 T at $R = 2$ cm.

The schematic cross-section of the main text shows this field for $G = 6$ T/m: on the positive $x$ axis $B_y > 0$ (up) and the force on a positive charge, $-qvB_y\,\hat{x}$, points to the centre; on the positive $y$ axis $B_x > 0$ (right) and the force $+qvB_x\,\hat{y}$ points away from it.

### Step 6: the ends of a real magnet: fringe field and effective length

The gradient of a real quadrupole does not drop to zero abruptly at its ends but fades out over about one aperture radius (the fringe field). Linear optics replaces it by a "hard-edge" model: the central gradient $G_0$ is kept and the length is the **effective length**

$$
L_{\text{eff}} = \frac{1}{G_0}\int G(s)\,\dd s ,
$$

where the integral $\int G\,\dd s$ is the area under the curve of the gradient along $s$. Replacing the real curve by "a rectangle of the same area" keeps the total focusing strength (the focal length of the main text depends only on $G L$), but drops the higher-order (non-linear) effects of the fringe field, such as end effects cubic in the position. The pole shape and manufacturing errors add higher multipoles too (12-pole, 20-pole, …), which are not in the matrix model either.

### For readers who know vector analysis

The two rules are $\nabla\cdot\boldsymbol{B} = 0$ and $\nabla\times\boldsymbol{B} = 0$ in the aperture; step 3 gives $a + d = 0$ and $b = c$. The general form of step 4 is

$$
\frac{\partial F_x}{\partial x} + \frac{\partial F_y}{\partial y}
= -q v\left(\frac{\partial B_y}{\partial x} - \frac{\partial B_x}{\partial y}\right)
= -q v\,(\nabla\times\boldsymbol{B})_z = 0 .
$$

The normal quadrupole field can also be written with the vector potential $A_z = \tfrac{G}{2}(y^2 - x^2)$. Writing the transverse field as the complex function $B_y + iB_x = \sum_n C_n (x + iy)^{n-1}$ gives the multipole expansion; $n = 2$ is the quadrupole term.

## Going deeper: from the force to the transfer matrix {#quadrupole-deep-matrix}

The main text derived the focal length $f = B\rho/(GL)$ from the impulse theorem, assuming a "thin" quadrupole in which the particle's position has no time to change. This section drops that assumption and arrives step by step at the "thick-lens" matrix that accelerator design really uses.

### Step 1: how large the force is

In a normal quadrupole field

$$
F_x = -q v G\,x,\qquad F_y = +q v G\,y .
$$

For $qG > 0$ the force in $x$ points to the axis (focusing) and in $y$ away from it (defocusing); for $qG < 0$ the planes swap. The same quadrupole that focuses protons in $x$ focuses H⁻ ions in $y$.

As a number: the example protons have $v = 1.71\times 10^7$ m/s; with $G = 6$ T/m, at $x = 1$ mm the field is $B_y = 6$ mT and the force $1.6\times 10^{-14}$ N. That looks tiny, but the proton's transverse momentum is tiny too.

### Step 2: the force bends the path

A particle crossing a short distance $\Delta s$ takes the time $\Delta s/v$, so its transverse momentum changes by $F_x\,\Delta s/v$; divided by the forward momentum $p$, this is the change of the angle $x'$. The "rate of change of the angle along the path" (written $x''$, "x double-prime", i.e. how fast the path bends) is therefore

$$
x'' = -k\,x,\qquad y'' = +k\,y,\qquad
k = \frac{q\,G}{p} = \frac{G}{B\rho}.
$$

$B\rho = p/q$ is the magnetic rigidity of the main text; in practical units $B\rho\,[\mathrm{T\,m}] = p\,[\mathrm{MeV}/c]/(299.792458\,Z)$ with $Z$ the charge number; $k$ is in m$^{-2}$. The example beam has $p = 53.55$ MeV/$c$ and $B\rho = 0.17863$ T·m; $G = 6$ T/m gives $k = 33.6$ m$^{-2}$. A 0.1 m long quadrupole of this kind turns a particle at $x = 1$ mm by $kL\,x = 3.4$ mrad, comparable to the example beam's rms angle of 3.0 mrad. $k$ is proportional to $1/B\rho$: at 100 MeV ($B\rho = 1.48$ T·m) the same $k$ needs a gradient of about 50 T/m.

### Step 3: this is simple harmonic motion

$x'' = -k\,x$ is the same equation as the school spring oscillator $a = -(K/m)\,x$, with "time" replaced by "distance $s$". The oscillator's solution is $x = A\cos(\omega t + \phi)$, and here

$$
x(s) = x_0\cos\!\left(\sqrt{k}\,s\right) + \frac{x_0'}{\sqrt{k}}\sin\!\left(\sqrt{k}\,s\right),
$$

with the two coefficients fixed by the entrance position $x_0$ and angle $x_0'$ (check by putting in $s = 0$). In the focusing direction the particle follows a short piece of a cosine inside the magnet. In the defocusing direction $y'' = +k\,y$ has the opposite sign: the solution no longer oscillates but grows faster and faster, described by the "hyperbolic functions" $\cosh u = (e^u + e^{-u})/2$ and $\sinh u = (e^u - e^{-u})/2$, which play the role of $\cos$ and $\sin$.

### Step 4: the thick-lens matrix

Let $\varphi = \sqrt{\lvert k\rvert}\,L$, the oscillation angle the particle "turns through" inside the magnet. Writing the solution of step 3 at $s = L$ as a conversion table like the drift's (the angle is the rate of change along $s$), for $k > 0$:

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

For $k < 0$ the planes swap. For $k \to 0$ both tables become the drift's $\begin{pmatrix} 1 & L \\ 0 & 1\end{pmatrix}$, as $\cos\varphi \approx 1$ and $\sin\varphi \approx \varphi$ confirm.

### Step 5: back to the thin lens

For small $\varphi$ the quadrupole can be seen as a thin lens at its centre with drifts of $L/2$ on both sides; the lens table is

$$
M_{\text{thin}} =
\begin{pmatrix}
1 & 0\\
-1/f & 1
\end{pmatrix},
\qquad
\frac{1}{f} = k\,L = \frac{G L}{B\rho},
$$

exactly the focal length the main text got from the impulse theorem: $+f$ in $x$, $-f$ in $y$. It depends only on the integrated gradient $GL$, which is why the effective-length model keeps $\int G\,\dd s$. The actual thick-lens focal length ($-1/M_{21}$) differs from the thin-lens one at order $\varphi^2$: in the focusing plane $f_x = 1/(\sqrt{k}\sin\varphi)$ is slightly longer, in the defocusing plane $\lvert f_y \rvert = 1/(\sqrt{k}\sinh\varphi)$ slightly shorter.

For the example beam: with $G = 6$ T/m and $L = 0.1$ m, $\varphi = 0.58$, the thin lens has $f = 0.298$ m, the thick lens $f_x = 0.315$ m and $f_y = -0.282$ m; with $G = 10$ T/m, $\varphi = 0.75$, $f = 0.179$ m against $f_x = 0.196$ m, a 10 % difference. In the animation at the start of this chapter the $x$ waist lies about 0.24 m from the magnet centre, a little beyond the 0.2 m focal length, because the incoming beam is itself diverging ($\alpha_x = -0.66$). More numbers are in the [numerical examples](#quadrupole-in-avas) below.

### For graduate readers

$x'' + k(s)\,x = 0$ is Hill's equation (see [Linear optics](#linear-optics)), with constant $k$ inside a quadrupole. The longitudinal part is that of a drift of the same length ($R_{56} = L/\gamma^2$), so the full 6 × 6 matrix has three diagonal blocks:

$$
M_{\text{quad}} = \operatorname{diag}\!\left(M_x,\; M_y,\; \begin{pmatrix} 1 & L/\gamma^2 \\ 0 & 1 \end{pmatrix}\right),
$$

the thick-lens matrix of AVAS's linear model ($k = G/B\rho$; positive values focus positive charges in $x$). $\det M_x = \det M_y = 1$: a quadrupole conserves the emittance too.

## Going deeper: alternating focusing in numbers {#quadrupole-deep-fodo}

The main text explained alternating focusing with "pushed in when far out". This section turns it into numbers by ray tracing from school geometrical optics and then derives the stability condition of a FODO channel.

### Step 1: ray tracing through two lenses

Take a ray parallel to the axis at height $h$, through a lens of focal length $+f$, then a distance $d$, then a lens of focal length $-f$:

- after the first lens its angle is $-h/f$ (bent towards the axis);
- after the distance $d$ its height is $h - d\,h/f = h\,(1 - d/f)$;
- the second lens bends it outwards by "height ÷ focal length", $+h(1 - d/f)/f$, so the total angle is $-h/f + h(1 - d/f)/f = -h\,d/f^2$.

A parallel ray comes out converging towards the axis at the angle $-h\,d/f^2$, so the combined focal length $F$ satisfies

$$
\frac{1}{F} = \frac{d}{f^2}.
$$

Swap the order (first $-f$, then $+f$) and the same calculation gives $1/F = d/f^2$. Both directions are positive: this is the heart of **alternating-gradient** (strong) focusing. The "first-order" effects of the two lenses, $1/f - 1/f$, cancel exactly (the "two spring constants add up to zero" from before); the net focusing comes from the change of height between the two lenses and is a "second-order" quantity like $d/f^2$.

### Step 2: doublets and triplets

With $f = 0.298$ m ($G = 6$ T/m) and $d = 0.3$ m the combined focal length is $F = f^2/d = 0.30$ m, about as strong as one lens. But the two directions do not focus at the same place: by the ray tracing of step 1, in $x$ (focusing first) the ray reaches the second lens at a height of only $h(1 - d/f) \approx 0$, so the focus is almost at the second lens; in $y$ (defocusing first) the focus is about 0.59 m behind the second lens. A beam leaving a doublet is therefore generally not round. A symmetric triplet ($-f/2$, $+f$, $-f/2$ or the reverse) gives similar imaging in both directions and is used where a round spot is needed.

A physically correct analogy: an upside-down pendulum whose pivot is shaken quickly up and down (Kapitza's pendulum) can stand upright. The pivot's acceleration alternately adds to gravity and overcomes it, so the effective gravity alternately topples and rights the pendulum; it is righted when it leans most, and on average there is a restoring force. The analogy fails on "quickly": Kapitza's pendulum needs the shaking to be much faster than the swinging, while a FODO channel advances by tens of degrees per period, so focal lengths and stability must be computed with matrices, not averages.

### Step 3: when is a FODO period stable?

Multiplying the four tables of F, O, D, O gives the table $M$ of one period. Each period transforms the particle once more by this table. One can show (see the end of this section) that the particles swing about the centre without spreading out when

$$
\left\lvert \frac{M_{11} + M_{22}}{2} \right\rvert < 1 ,
$$

and that then the **phase advance** per period $\sigma_0$ satisfies $\cos\sigma_0 = (M_{11} + M_{22})/2$. For thin lenses with centres $L_h$ apart this simplifies to

$$
\sin\frac{\sigma_0}{2} = \frac{L_h}{2 f}.
$$

The left side cannot exceed 1, so stability needs $f > L_h/2$: with lenses that are too strong the particle is "pushed past the centre" in each period and swings wider and wider. "The stronger the gradient, the better" is wrong.

Numbers: for the example beam with $L_q = 0.1$ m, $L_d = 0.2$ m ($L_h = 0.3$ m) and $G = 6$ T/m the thick-lens matrices give $\sigma_0 = 52.7^\circ$ and the thin-lens estimate $60.5^\circ$; at about 13.5 T/m (same $L_q$, $L_d$) $\lvert M_{11} + M_{22}\rvert/2$ reaches 1 and the period becomes unstable. The animation of the main text uses $L_d = 0.3$ m ($L_h = 0.4$ m): the thin-lens estimate is unstable from 8.9 T/m, the thick-lens matrices from 9.8 T/m, and the latter is what you see in the animation.

### Step 4: the beam is wider in the focusing quadrupoles

A stable period has a set of "matched" Twiss parameters: a beam entering the period with this shape leaves it with the same shape (see [matching](#matching)). With $L_d = 0.2$ m and $G = 6$ T/m the periodic $\beta_x$ is largest at the centre of QF (1.06 m) and smallest at the centre of QD (0.43 m), corresponding to rms sizes of 1.60 mm and 1.02 mm: the beam really is 57 % wider in the focusing quadrupole than in the defocusing one, the "pushed in when far out" of the main text in numbers. The envelope of the 4 FODO periods ($L_d = 0.2$ m) in the main text oscillates unevenly because the Twiss parameters of the example beam are not the matched solution of this period.

Check these results in the FODO lab (thick-lens matrices, linear model). First reproduce 52.7° with $G = 6$ T/m and $L_d = 0.2$ m, then:

1. Increase `G` step by step and watch the phase advance and $\lvert\Tr M\rvert/2$; find the edge of the stable region (phase advance near $180^\circ$).
2. Increase `Ld` to 0.3 m: at $G = 10$ T/m the period is no longer stable and the envelope diverges.
3. Set `mismatch` to 1.3 and watch the envelope beating of a mismatched beam; then add a current `I` and compare the envelopes with and without space charge.

```avas-lab
fodo Lq=0.1 G=6 Ld=0.2 cells=10 W=1.527 mass=938.272 q=1 emit=0.1377 I=0 mismatch=1
```

> **Think about it**: the net focusing $d/f^2$ of a FODO cell is of second order. Why is a quadrupole channel still more "economical" than a single lens?
> Answer: because $1/f$ itself can be made large. The quadrupole force comes from a transverse gradient, $1/f = GL/B\rho$, and a pole-tip field below 1 T already gives tens of T/m; the second-order net focusing $(GL/B\rho)^2 d$ is still strong enough. The focusing of a solenoid, $\propto (B/B\rho)^2$, needs the longitudinal field itself throughout the aperture; see the comparison in the [Solenoid](#solenoid) chapter.

### For graduate readers: where the stability condition comes from

The 2 × 2 matrix $M$ of one period has $\det M = 1$, so its eigenvalues satisfy $\lambda_1\lambda_2 = 1$ and $\lambda_1 + \lambda_2 = \Tr M$. For $\lvert\Tr M\rvert < 2$ they are $\lambda = e^{\pm i\sigma_0}$ on the unit circle, $M^n$ stays bounded and the motion is stable; for $\lvert\Tr M\rvert > 2$ one eigenvalue has modulus above 1 and $M^n$ grows exponentially. Writing $M = I\cos\sigma_0 + J_T\sin\sigma_0$ ($J_T$ built from the periodic Twiss parameters) gives the periodic $\beta$ and the phase advance: Floquet theory for linear periodic systems, see [Linear optics](#linear-optics-periodic).

## Going deeper: errors, chromatic effects and space charge {#quadrupole-deep-dynamics}

So far the quadrupoles were perfect. This section looks at four things that affect the beam in a real quadrupole channel, each first as a phenomenon, then with a number.

### The "breathing" envelope

In a FODO period $\beta$ in $x$ is largest at QF and smallest at QD, in $y$ the other way round, half a period apart. The beam cross-section therefore flattens alternately in the two directions, the signature of quadrupole focusing; a round cross-section appears only where the two envelopes cross.

### Chromatic effects: different energy, different focal length

The focusing strength $k = G/B\rho$ is inversely proportional to the momentum; a particle with $\delta$ more momentum feels about $k(1 - \delta)$, just as a glass lens has different focal lengths for different colours (chromatic aberration). In a straight section without bends the effect on the rms size is usually small, but where the energy spread is large and the focusing strong it makes the effective emittance grow. AVAS's linear preview computes the matrices at the reference momentum without chromatic effects; the multi-particle engine is the reference.

### Offsets, rolls and gradient errors

- **Transverse offset** $d_x$: the beam is $-d_x$ off the field centre and receives a deflection $\Delta x' = k L\,d_x$ (thin lens), a "kick" to the whole beam. For the example beam, a quadrupole with $G = 6$ T/m and $L = 0.1$ m that is 0.1 mm off kicks by 0.34 mrad, like a steerer with an integrated field of $6\times 10^{-5}$ T·m. This is the main source of orbit errors and is corrected with [steerers](#steerer), see [Orbit correction](#orbit-correction).
- **Roll** $\theta$ about $z$: produces a skew quadrupole component (about $\sin 2\theta$ times the strength), which mixes the $x$ and $y$ motions and makes the projected emittances grow.
- **Gradient error** $\Delta G/G$: changes the phase advance, causing mismatch and envelope beating (beta beating).

The full error study is in [Errors and correction](#errors).

### Space charge

At high current the repulsion between the particles defocuses in both directions, cancels part of the quadrupole focusing and lowers the phase advance per period from $\sigma_0$ to $\sigma$ ("tune depression"). The beam in a quadrupole channel flattens alternately in the two directions, and the space-charge force changes with it; to avoid envelope instabilities and parametric resonances the zero-current phase advance is usually kept below $90^\circ$. The lab of the [Space charge](#space-charge) chapter shows the effect.

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

> **AVAS convention**: $k = G/(B\rho)$, and **$G > 0$ focuses a positive particle in $x$ and defocuses it in $y$** (the linear envelope preview uses this convention: $G = 10$ T/m and $L = 0.1$ m give $M_{21} = -5.09\ \mathrm{m^{-1}}$, i.e. focusing in $x$; the manual itself does not state the sign convention, so before relying on it for the engine, check it once with a single quadrupole and a beam offset in $x$). For negative particles (e.g. H⁻) the same $G$ swaps the planes. In a FODO design, QF has positive $G$ and QD negative $G$.

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
