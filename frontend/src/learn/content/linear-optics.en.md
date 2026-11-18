# Linear optics

A real beam line consists of tens or hundreds of elements: drifts, quadrupoles, solenoids, RF cavities… To know how wide the beam is at the end and whether it hits the pipe wall, do we really have to push thousands of particles one by one through every element?

No. As long as the elements act on the particles under one simple condition — **the farther a particle is from the axis, the more it is bent back, and exactly in proportion** — every element can be written as a "conversion table" with 2 rows and 2 columns, and the whole beam line is a chain of tables multiplied together. How the beam "breathes", wider and narrower; how every particle swings back and forth about the axis; why focusing that is too strong gets out of control — all of it can be read off with school multiplication and addition. This method is called **linear optics**, and the linear envelope preview on AVAS's lattice page is computed with it.

First, have a look. In the animation below the example beam (protons with a kinetic energy of 1.527 MeV) passes through 10 identical cells, each made of "focusing quadrupole – drift – defocusing quadrupole – drift" (such a cell is called FODO, explained below).

```avas-motion
fodo G=4 cells=10
```

Keep an eye on the two blue particles: they do not fly off for good, but swing back and forth about the central axis, now on one side, now on the other, like a ball rolling to and fro in a valley. Each particle needs about 7.5 cells to complete one swing, and swings about 1.3 times in the 10 cells. The width of the whole beam "breathes"; at its widest the rms size is about 2.0 mm, far from the 2 cm pipe wall. This chapter explains why the particles swing like this, how fast they swing, and when they get out of control.

## The lens: the farther from the axis, the stronger the bend {#linear-optics-lens}

A magnifying glass can focus sunlight into a bright spot. Look closely at the rays: a ray entering along the central axis passes straight through without bending; the farther a ray is from the centre, the more strongly it is bent. The rule of an ideal lens is: **the angle by which a ray is bent is proportional to its distance from the centre**,

$$
\text{bending angle} = -\frac{x}{f},
$$

where $x$ is the distance of the ray from the central axis, the minus sign means it is bent back towards the centre, and $f$ is called the **focal length**. Why must it be proportional? A ray entering parallel to the axis at a distance $x$ is bent by $x/f$, and after travelling the distance $f$ it is exactly back on the axis ($x - f\cdot x/f = 0$). Whatever $x$ is, every ray reaches the axis at the same point, and that point is the focus. If the bending angle were not proportional to $x$, rays at different heights would meet at different places and the image would be blurred.

The "lenses" of an accelerator are magnets. The most common one is the **quadrupole**: its field is zero at the centre, grows with distance from the centre, and grows exactly in proportion, so its push on a particle is also proportional to the distance from the axis (how a magnet does this is in [Quadrupole](#quadrupole)). An example: a quadrupole 0.1 m long with a gradient of 4 T/m acts on the protons of the example beam like a lens with a focal length of 0.447 m: a particle 1 mm off axis is bent back by 2.24 mrad, one 2 mm off axis by 4.48 mrad.

"Proportional to the distance" is called **linear** in mathematics. Linearity has a wonderful advantage: **motions can simply be added and scaled**. If one particle starts parallel at 1 mm from the axis and another starts parallel at 2 mm, the path of the second is everywhere exactly twice that of the first. So once we understand a few "basic paths", we know the motion of every particle. This is the starting point of every method in this chapter.

> **A few words**: the **position** $x$ of a particle is its distance from the central axis (horizontal $x$, vertical $y$); its **angle** $x'$ (read "$x$ prime") is the angle between its direction of flight and the axis. Angles in accelerators are all small and are given in **milliradians** (mrad, a thousandth of a radian); remember one conversion: a particle with an angle of 1 mrad moves 1 mm off course for every metre it travels. The distance travelled along the beam line is $s$. A **beam** is a large swarm of particles flying in the same direction; the rms size, emittance and related ideas used to describe it are in [Phase space and emittance](#phase-space).

## A force like a spring: Hill's equation {#linear-optics-hill}

A force that points towards the axis and is proportional to the distance from it — that is exactly the force of a spring. A ball on a spring oscillates back and forth, and a particle in a chain of lenses also swings back and forth about the axis; that is what the opening animation shows. The difference is that this "spring" comes and goes and changes its stiffness: very stiff inside a quadrupole, absent in a drift, and in a defocusing quadrupole it even pushes outwards.

### How a force changes the angle

The school impulse theorem is enough to write down the equation of motion. Let the particle have speed $v$ and forward momentum $p$, and feel a transverse force $F$:

- it needs the time $\Delta s/v$ to travel a short distance $\Delta s$;
- in that time its transverse momentum grows by $F\,\Delta s/v$ (impulse = force × time);
- transverse momentum divided by forward momentum is the angle, so the angle grows by $\Delta x' = F\,\Delta s/(p\,v)$.

The "rate of change of the angle along the path" is written $x''$ (read "$x$ double prime"; it is how quickly the path bends), so

$$
x'' = \frac{F}{p\,v}.
$$

The denominator $p\,v$ is worth a moment's thought: the larger the momentum, the harder it is for the same force to change the direction; the higher the speed, the less time the particle spends feeling the force on each metre of its path.

The force of a magnetic field is the Lorentz force $F = -q\,v\,B_y$ ($q$ is the charge, $B_y$ the vertical field, the minus sign comes from the left-hand rule); substituting it, $v$ cancels:

$$
x'' = -\frac{B_y}{B\rho}, \qquad B\rho = \frac{p}{q}.
$$

$B\rho$ is the **magnetic rigidity**, how hard the particle is to bend with a magnetic field; for the example proton $B\rho = 0.1786\ \mathrm{T\,m}$ (see [Relativistic kinematics](#kinematics-rigidity)). In a quadrupole $B_y = G\,x$, with $G$ the gradient (in T/m), so $x'' = -(G/B\rho)\,x$: the bending of the path is proportional to the distance from the axis and points towards it — a spring.

### Hill's equation

Call the "spring stiffness" $K(s)$; it changes with the position $s$:

$$
x'' + K_x(s)\,x = 0, \qquad y'' + K_y(s)\,y = 0 .
$$

This is **Hill's equation**; $K$ has the unit m$^{-2}$. Where $K > 0$ the force points to the axis (focusing), where $K < 0$ it points away (defocusing), and $K = 0$ is a drift. The $K$ of the elements:

| Element | $K_x$ | $K_y$ | See |
|---|---|---|---|
| Drift | 0 | 0 | [Drift](#drift) |
| Quadrupole (gradient $G$) | $k = G/(B\rho)$ | $-k$ | [Quadrupole](#quadrupole) |
| Sector dipole (field index $N$, bending in $x$) | $(1-N)/\rho^2$ | $N/\rho^2$ | [Dipole](#dipole) |
| Solenoid (in the rotating Larmor frame) | $k_s^2$, $k_s = B/(2B\rho)$ | $k_s^2$ | [Solenoid](#solenoid) |

Numbers: a quadrupole with $G = 4\ \mathrm{T/m}$ has $k = 4/0.1786 = 22.4\ \mathrm{m^{-2}}$ for the example proton. A particle 1 mm off axis that crosses 0.1 m of such a quadrupole changes its angle by about $k L x = 2.24\ \mathrm{mrad}$, comparable to the rms angle of 3.0 mrad of the example beam — exactly the order of "moderate" focusing.

For a positive particle a quadrupole with $G > 0$ focuses in $x$ and defocuses in $y$: at $x > 0$, $B_y > 0$ and the Lorentz force points to $-x$. That the table gives a quadrupole $K_y = -K_x$ is no coincidence: **no quadrupole can focus in both directions at once**. This is a property of the magnetic field itself (where there is no current in vacuum, the field does not "swirl"), not a lack of clever design; the proof is in [the deeper section of the quadrupole chapter](#quadrupole-deep-field). Focusing and defocusing quadrupoles must therefore alternate, and we shall see below why that works.

Hill's equation rests on a few approximations worth remembering: the particle flies nearly along the axis (angle $x' \ll 1$, the **paraxial approximation**); the field is linear in the distance from the axis; the momentum differences between particles are small enough (particles with different momenta have different $K$, called **chromatic aberration**, neglected here); there is no acceleration in the section (with acceleration one more term appears, see [the deeper section](#linear-optics-deep-hill)). Whenever one of these fails, linear optics is only an approximation.

## Transfer matrices: two numbers in, two numbers out {#linear-optics-matrices}

In one transverse plane the "state" of a particle at a given place needs only two numbers: position $x$ and angle $x'$. What every element does is turn the two numbers at its entrance into two numbers at its exit.

**Drift** (length $L$): no force, the angle stays, the position grows with the angle:

$$
x = x_0 + L\,x_0', \qquad x' = x_0' .
$$

**Thin lens** (focal length $f$; "thin" means the position has no time to change while the particle crosses it): the position stays, the angle changes by the lens rule:

$$
x = x_0, \qquad x' = x_0' - \frac{x_0}{f} .
$$

$f > 0$ is a focusing lens, $f < 0$ a defocusing one.

Both pairs of lines say "each exit number = the two entrance numbers, each multiplied by a coefficient, added up". Arranging the coefficients in a table with 2 rows and 2 columns gives a **matrix**:

$$
\begin{pmatrix} x \\ x' \end{pmatrix}
= \begin{pmatrix} a & b \\ c & d \end{pmatrix}
\begin{pmatrix} x_0 \\ x_0' \end{pmatrix}
\quad\text{means}\quad
x = a\,x_0 + b\,x_0', \quad x' = c\,x_0 + d\,x_0' .
$$

The table of a drift is $\begin{pmatrix} 1 & L \\ 0 & 1 \end{pmatrix}$, that of a thin lens $\begin{pmatrix} 1 & 0 \\ -1/f & 1 \end{pmatrix}$. This table is called the **transfer matrix**.

### One particle through two elements

Example: a particle 1 mm off axis and parallel to it ($x_0 = 1$ mm, $x_0' = 0$) crosses a lens of focal length 0.447 m and then a 0.3 m drift (combining mm, mrad and m like this needs no conversion):

- through the lens: $x = 1$ mm, $x' = 0 - 1/0.447 = -2.24$ mrad;
- through the drift: $x = 1 + 0.3 \times (-2.24) = 0.328$ mm, $x' = -2.24$ mrad.

The two tables can also be combined into one first. The rule for multiplying tables is "row of the left table" times "column of the right table":

$$
\begin{pmatrix} a_2 & b_2 \\ c_2 & d_2 \end{pmatrix}
\begin{pmatrix} a_1 & b_1 \\ c_1 & d_1 \end{pmatrix}
=
\begin{pmatrix} a_2a_1 + b_2c_1 & a_2b_1 + b_2d_1 \\ c_2a_1 + d_2c_1 & c_2b_1 + d_2d_1 \end{pmatrix}.
$$

A lens followed by a drift combine to

$$
\begin{pmatrix} 1 & L \\ 0 & 1 \end{pmatrix}
\begin{pmatrix} 1 & 0 \\ -1/f & 1 \end{pmatrix}
=
\begin{pmatrix} 1 - L/f & L \\ -1/f & 1 \end{pmatrix};
$$

with $L = 0.3$ m and $f = 0.447$ m the upper left entry is $1 - 0.3/0.447 = 0.328$, the same as element by element. However many elements a beam line has, the tables can be multiplied one by one into a single table from entrance to exit:

$$
M = M_n \cdots M_2\,M_1 ,
$$

**with the element passed later written on the left** (why: see [Think about it](#linear-optics-think)).

### Two basic paths

The four numbers of the total table have an intuitive meaning. Let a particle start parallel at a distance 1 from the axis ($x_0 = 1$, $x_0' = 0$): its $(x, x')$ at the exit is exactly the first column of the table; this path is called the **cosine-like path**, written $(C, C')$. Let a particle start on the axis with angle 1 ($x_0 = 0$, $x_0' = 1$): its exit state is the second column, the **sine-like path** $(S, S')$. Every particle is these two paths scaled by $x_0$ and $x_0'$ and added, so

$$
M = \begin{pmatrix} C & S \\ C' & S' \end{pmatrix}.
$$

This is what the opening meant by "understand a few basic paths and you know every particle".

### Lenses with a length

A real quadrupole has a length: inside it the particle moves while it is being pushed, and the four numbers of the table become cosine- and sine-like functions (the **thick-lens matrix**, derived in [the deeper section](#linear-optics-deep-matrix)). The shorter and stronger a quadrupole, the closer it is to a thin lens; for quadrupoles typical of low-energy sections the thin-lens estimate is off by about 5 %. AVAS's linear preview uses the thick-lens matrix for `quad`.

## Alternating gradient: why "focus + defocus" focuses {#linear-optics-alternating}

A quadrupole always focuses in one direction and defocuses in the other. So add a second one of opposite sign. But one focusing and one defocusing, equally strong — do they not simply cancel? Following a ray with the method of the previous section tells us.

Take a focusing lens of focal length $f = 0.447$ m and a defocusing lens of focal length $-0.447$ m, a distance $d = 0.4$ m apart (exactly the distance between the centres of two quadrupoles in the opening animation). A particle enters 1 mm off axis and parallel:

- **First focusing, then defocusing**: through the focusing lens its angle becomes $-2.24$ mrad; after 0.4 m it is only $1 - 0.4 \times 2.24 = 0.104$ mm off axis; the defocusing lens pushes it outwards by "distance ÷ focal length" $= 0.104/0.447 = 0.23$ mrad. The final angle is $-2.24 + 0.23 = -2.01$ mrad: **net bending towards the axis**.
- **First defocusing, then focusing** (the order the other direction sees): through the defocusing lens its angle becomes $+2.24$ mrad; after 0.4 m it is $1 + 0.896 = 1.896$ mm off axis; the focusing lens bends it back by $1.896/0.447 = 4.24$ mrad. The final angle is $2.24 - 4.24 = -2.01$ mrad: **again towards the axis, and by the same amount**.

A parallel ray comes in and leaves converging towards the axis at 2.01 mrad in both cases, like a focusing lens with a focal length of $1/2.01 \approx 0.50$ m, the same in both directions.

The key is that the force is proportional to the distance from the axis: **the particle is always farther from the axis in the focusing lens and closer to it in the defocusing lens**. It is pushed inwards when it is far out and pushed outwards when it is close in, and altogether it is always pushed inwards more. This is **alternating-gradient focusing**.

Multiplying tables gives the general formula. For two thin lenses with focal lengths $f_1$ and $f_2$ a distance $d$ apart, multiply the three tables (lens, drift, lens); the lower left entry of the result with its sign flipped ($-M_{21}$) is the combined "focusing power" $1/F$:

$$
\frac{1}{F} = \frac{1}{f_1} + \frac{1}{f_2} - \frac{d}{f_1 f_2} .
$$

With $f_1 = f$ and $f_2 = -f$ the first two terms cancel and what remains is

$$
\frac{1}{F} = \frac{d}{f^2} > 0 ;
$$

the other way round, $f_1 = -f$ and $f_2 = f$, gives exactly the same, so **both directions get net focusing**. With numbers $F = 0.447^2/0.4 = 0.50$ m, matching the step-by-step result above. The net focusing $d/f^2$ is of "second order" in the lens strength $1/f$: the stronger the single lenses, the faster the net focusing of the alternating arrangement grows. The alternating-gradient synchrotron used magnets with very large gradients (compared with the very small field index of the earlier "weak-focusing" machines), so the scheme is also called "**strong focusing**" (proposed by Christofilos in 1950 but not formally published, and independently proposed and published by Courant, Livingston and Snyder in 1952). It is one of the most important inventions in the history of accelerators.

A physically correct analogy is **Kapitza's pendulum**: an inverted pendulum whose pivot vibrates rapidly up and down can stand stably upright. The vibration gives the pendulum angle a small, fast jitter, and this jitter is correlated with the vibrating force — when the force pushes the pendulum outwards the angle is a little smaller, when it pulls back the angle is a little larger — so on average there is a restoring torque towards the vertical. This is the same mechanism as "the particle is farther from the axis in the focusing lens". The analogy has limits: Kapitza's pendulum needs the vibration to be much faster than the natural swing, whereas the phase advance per FODO cell can be tens of degrees, so only the stability criterion below decides exactly; also, a convex lens in optics focuses in both directions at once, whereas with quadrupoles one direction sees "focus – defocus" and the other "defocus – focus".

## Symplecticity: why the phase-space area stays {#linear-optics-symplectic}

Draw every particle as a dot with position $x$ across and angle $x'$ up; this picture is the **phase-space plot**. A beam occupies a region of it, roughly an ellipse, and its area is the **emittance** (see [Phase space and emittance](#phase-space)).

See how the two elements move the dots:

- **Drift**: every dot keeps its angle and its position grows by $L\,x'$. Dots with positive angle move right, those with negative angle move left; the whole region is pushed sideways like a deck of cards sheared over.
- **Thin lens**: every dot keeps its position and its angle drops by $x/f$. The whole region is sheared vertically.

A sheared deck of cards keeps its total area. So neither a drift nor a thin lens changes the phase-space area, and neither does a chain of them.

In general a table $\begin{pmatrix} a & b \\ c & d \end{pmatrix}$ multiplies areas in phase space by $ad - bc$; this number is the **determinant** $\det M$ of the table. For a drift $1 \times 1 - L \times 0 = 1$, for a thin lens $1 \times 1 - 0 \times (-1/f) = 1$. When tables are multiplied their determinants multiply too, so the whole beam line has $\det M = 1$. Check it on the combined "focus – drift – defocus" table of the previous section ($0.104$, $0.4$, $-2.006$, $1.896$): $0.104 \times 1.896 - 0.4 \times (-2.006) = 1.00$.

This is what [Liouville's theorem](#phase-space-liouville) looks like in linear optics: motion without friction and without losses does not change the phase-space area. Matrices that obey this are called **symplectic**; "symplecticity" means "area preserving" (the full condition for several directions is in [the deeper section](#linear-optics-deep-matrix)). So the emittance is conserved in linear elements, and $\det M = 1$ is a check on hand calculations or programs.

**With acceleration** the forward momentum grows, the same transverse momentum corresponds to a smaller angle, the ellipse is squeezed in $x'$ and its area shrinks: $\det M = (\beta\gamma)_0/(\beta\gamma)_1 < 1$ ($\beta\gamma$ is the relativistic quantity telling how fast the particle is; the subscripts 0 and 1 mean entrance and exit). This is **adiabatic damping**, and it does not violate Liouville's theorem: plot the transverse momentum (multiply the angle by $\beta\gamma$) instead of the angle and the area is conserved again — which is why the **normalised emittance** stays constant during acceleration.

## Transport of the σ-matrix and Twiss parameters {#linear-optics-twiss-transport}

So far we have followed single particles. A beam has thousands of macro-particles, and what we care about is the overall shape: how large, how flat and how tilted the ellipse occupied by the particle cloud in phase space is.

The ellipse is described by three averages (angle brackets mean averaging over all particles): $\langle x^2\rangle$ (its square root is the rms size), $\langle x'^2\rangle$ (its square root is the rms angle) and $\langle x x'\rangle$ (the correlation of position and angle: positive when the beam diverges, negative when it converges). Arranged in a symmetric table the three numbers form the **σ-matrix** (see [Phase space: σ-matrix](#phase-space-sigma)). Divided by the emittance $\varepsilon$ they become the three **Twiss parameters**, which describe only the shape:

$$
\beta = \frac{\langle x^2\rangle}{\varepsilon}, \qquad
\alpha = -\frac{\langle x x'\rangle}{\varepsilon}, \qquad
\gamma = \frac{\langle x'^2\rangle}{\varepsilon},
$$

$\beta$ governs the size (rms size $= \sqrt{\beta\varepsilon}$), $\alpha$ the tilt ($\alpha > 0$ converging, $\alpha < 0$ diverging), $\gamma$ the angles. (These $\beta$ and $\gamma$ merely share the names of the relativistic $\beta$ and $\gamma$; with the subscript $x$ they mean the Twiss parameters.)

Here linear optics saves the most work: **the ellipse is transported by the same table as a single particle**. Square $x = C x_0 + S x_0'$ and average: the three averages at the exit depend only on the three averages at the entrance, with no need to follow each particle (step-by-step derivation in [the deeper section](#linear-optics-deep-twiss)). The two most common results:

- **Drift**: $\beta(s) = \beta_0 - 2\alpha_0 s + \gamma_0 s^2$, a quadratic function of $s$ whose minimum is the **waist** (where the beam is narrowest), at $s^* = \alpha_0/\gamma_0$, with $\beta^* = 1/\gamma_0$ there.
- **Thin lens**: $\beta$ stays (the positions did not change), $\alpha$ grows by $\beta_0/f$: a focusing lens turns the beam towards convergence.

Example: at a waist ($\beta_0 = 1$ m, $\alpha_0 = 0$) put a lens with $f = 0.5$ m. After the lens $\alpha = 0 + 1/0.5 = 2$ and $\gamma = 5\ \mathrm{m^{-1}}$; the new waist is $2/5 = 0.4$ m behind the lens, with $\beta^* = 1/5 = 0.2$ m. **The new waist is not at the focus (0.5 m)**: the beam has an emittance, its particles were already flying in all directions rather than forming a parallel beam of light, so the narrowest point is closer to the lens than the geometric focus.

The lab below is this very example (example protons, normalised emittance 0.2 π·mm·mrad). With the default $L = 0.5$ m the readings put the waist 0.40 m behind the lens, give $\alpha = -0.50$ and $\beta = 0.25$ mm/mrad after the drift and an rms size shrinking from 1.87 mm at the entrance to 0.94 mm, while the ellipse area stays 3.50 mm·mrad throughout. Drag `f` and `L` and watch $\alpha$ change from positive to negative at the waist while the area stays the same:

```avas-lab
phase-space alpha=0 beta=1 emit=0.2 W=1.527 mass=938.272 f=0.5 L=0.5
```

A beam with an emittance cannot be squeezed into a point: to make it narrower its angles must become larger, and it grows faster after the waist. Think of the emittance as a pressure from the transverse "thermal motion" of the particles in the beam, which rises the faster the smaller the beam. The balance between the external focusing and this pressure sets the beam size; with beam current, the repulsion between particles adds more outward pressure, see [Space charge](#space-charge).

## β-function and phase advance {#linear-optics-phase-advance}

The blue particles of the opening animation swing back and forth. How far do they swing? How fast?

Start with the simplest case: the spring stiffness is the same everywhere, $K$ is a constant (imagine a long, uniformly focusing channel). $x'' = -K x$ is the same equation as the school spring oscillator $a = -(k/m)\,x$, with time replaced by distance, so the particle follows a cosine:

$$
x(s) = A\cos\!\left(\sqrt{K}\,s + \phi\right),
$$

$A$ is the amplitude, $\phi$ is set by the starting state. The distance of one swing (the wavelength) is $2\pi/\sqrt{K}$. Then $\beta = 1/\sqrt{K}$, exactly "wavelength ÷ $2\pi$", and the rms beam size $\sqrt{\varepsilon\beta}$ is constant. The stronger the focusing, the shorter the wavelength and the narrower the beam.

In a real beam line $K$ is on in some places and off in others; the particle's path still looks like a cosine, only its amplitude and "wavelength" change along the way:

$$
x(s) = \sqrt{J\,\beta(s)}\,\cos\bigl(\psi(s) + \psi_0\bigr),
$$

$J$ is a constant of each particle (the "size of its amplitude", the [Courant–Snyder invariant](#phase-space-twiss)), and $\psi$ is the **betatron phase** (betatron oscillation is the transverse swinging of a particle about the axis). $\beta(s)$ plays two roles here:

- **Amplitude**: the envelope of a particle's swing is $\sqrt{J\beta}$, the rms beam size is $\sqrt{\varepsilon\beta}$. Where $\beta$ is large the particles swing far out and the beam is wide.
- **Local wavelength**: for every short step $\Delta s$ the phase advances by $\Delta s/\beta$. The smaller $\beta$, the faster the phase advances.

The amount by which the phase advances over a section of beam line is the **phase advance** of that section, given as an angle: 360° is one full swing. In the opening animation the phase advance per cell is 48.3°, so one swing takes $360°/48.3° \approx 7.5$ cells; a cell is 0.8 m long, the wavelength is about 6.0 m, and divided by $2\pi$ that gives an average $\beta \approx 0.95$ m, consistent with the constant-$K$ picture "$\beta$ = wavelength ÷ $2\pi$".

The phase advance depends only on the beam line and the entrance Twiss parameters, not on the emittance: it describes how far a single particle has turned, not how big the beam is. The formula for the phase advance from the Twiss parameters at both ends and the matrix is in [the deeper section](#linear-optics-deep-twiss).

## Periodic systems and stability {#linear-optics-periodic}

The focusing of a linac usually consists of many identical cells placed end to end, for example **FODO**: F is a focusing quadrupole, D a defocusing quadrupole, O a drift, and one cell is "F, O, D, O"; the "solenoid + cavity" cells of a superconducting section are periodic in the same way. Let the total table of one cell be $M$; passing $n$ cells means multiplying by $M$ $n$ times.

### What happens when we keep multiplying

Try it with the cell of the opening animation (quadrupoles 0.1 m long, gradient ±4 T/m, drifts 0.3 m). Starting at the centre of the focusing quadrupole, 1 mm off axis and parallel to it, record the position after every cell (in mm):

$$
1,\ 0.666,\ -0.113,\ -0.817,\ -0.974,\ -0.481,\ 0.334,\ 0.926,\ 0.899,\ 0.271,\ \ldots
$$

This is exactly $\cos(n \times 48.3^\circ)$: the particle advances 48.3° in its swing per cell and never exceeds 1 mm.

Raise the gradient to 10 T/m and compute again:

$$
1,\ -1.083,\ 1.348,\ -1.837,\ 2.632,\ -3.866,\ 5.746,\ -8.583,\ \ldots
$$

The particle crosses to the other side of the axis every cell and gets farther out each time, about 1.5 times per cell, and soon hits the wall. The focusing became stronger and the beam got out of control.

### One number decides

How can the two cases be told apart in advance? Just look at the average of the two numbers on the diagonal of the cell table:

$$
c = \frac{M_{11} + M_{22}}{2} .
$$

- $\lvert c\rvert < 1$: **stable**. The particle swings about the axis, and the phase advance per cell $\mu$ satisfies $\cos\mu = c$.
- $\lvert c\rvert > 1$: **unstable**. The distance from the axis is multiplied by a factor larger than 1 every cell and grows exponentially.

(Why it is this number can be proved with school sequences and trigonometric identities, see [the deeper section](#linear-optics-deep-periodic).) A 2 × 2 table has four numbers, yet stability depends on a single combination; that simplification comes from $\det M = 1$ (the phase-space area is preserved). $c$ is half the trace of $M$ (the sum of the diagonal, written $\Tr M$), so the stability condition is often written $\lvert\tfrac12\Tr M\rvert < 1$. The phase advance per cell at zero current is often written $\sigma_0$.

With AVAS's thick-lens matrices for the opening cell:

- gradient 4 T/m: $c = 0.666$, $\mu = 48.3°$, the same in both directions;
- gradient 6.92 T/m: $\mu = 90°$;
- gradient about 9.8 T/m: $\lvert c\rvert$ reaches 1; any stronger and the cell is unstable.

The beam in this channel also has a shape that repeats with the structure: $\beta_x$ is largest at the centre of the focusing quadrupole (1.485 m) and smallest at the centre of the defocusing quadrupole (0.645 m), which for the emittance of the lab means rms sizes of 1.9 mm and 1.3 mm. **The beam is widest in the focusing quadrupole** — precisely the "particle is always farther from the axis in the focusing lens" of the previous section. In $y$ it is the other way round.

Check it yourself in the lab below (10 cells, thick-lens matrices):

1. start at $G = 4$, confirm $\mu \approx 48°$ and that the periodic $\beta_x$, $\beta_y$ are largest at the focusing quadrupole;
2. raise $G$ step by step and watch $\mu$ pass 90° and the cell become unstable at about 9.8 T/m, the envelope blowing up;
3. compare the thick-lens $\mu$ in the readings with the thin-lens estimate (48.3° against 53.2°) to see the error of the thin-lens approximation for quadrupoles with a length.

```avas-lab
fodo Lq=0.1 G=4 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.14 I=0 mismatch=1
```

You can also go back to the opening animation and set the gradient to 10 T/m: there is no stable shape any more, and almost all particles hit the wall (red crosses).

The design of FODO cells (how to choose the phase advance, how to combine them with cavities) is covered in [Lattice design](#lattice-design).

> Note: stability at zero current is only a necessary condition. Space charge depresses the phase advance per cell ($\sigma < \sigma_0$), and for $\sigma_0 > 90°$ intense beams are prone to envelope instabilities and parametric resonances, so high-intensity linacs usually choose $\sigma_0 < 90°$, see [Space charge](#space-charge).

## Matched beam and mismatch {#linear-optics-matched}

A stable periodic channel has an ellipse "it likes": if the beam at the entrance has exactly this shape, the ellipse returns to itself after every cell, and the rms size repeats cell by cell along the beam line. This is the **matched beam**. Think of this ellipse as a "contour line" in phase space: every particle circles along it, and however far they turn, the overall shape does not change.

If the entrance ellipse has the wrong shape, each particle still circles along its own contour line, but the whole ellipse turns by $\mu$ every cell and changes its shape, and the envelope grows and shrinks — it "beats". After half a turn the ellipse coincides with itself again, so the envelope oscillates twice as fast as the particles swing (advancing $2\mu$ per cell), and the largest size exceeds the matched one.

In the lab, set `mismatch` to 1.3: the entrance $\beta$ is multiplied by $1.3^2 = 1.69$ while $\alpha$ keeps its matched value. The largest envelope grows from 1.91 mm (matched) to 3.05 mm, 60 % larger. Set it back to 1 and the envelope is exactly periodic. The "mismatch factor" that measures the degree of mismatch, and how to compute it, are in [the deeper section](#linear-optics-deep-periodic).

```avas-lab
fodo Lq=0.1 G=4 Ld=0.3 cells=10 W=1.527 mass=938.272 q=1 emit=0.14 I=0 mismatch=1.3
```

In a purely linear system a mismatched beam keeps its emittance; only the envelope beats. A real beam line always has some nonlinearity, the particles circle at slightly different speeds, and the mismatched ellipse slowly "filaments" (like stirred cream being drawn into threads) into a larger ellipse, with a larger emittance. That is why the shape of the beam must be adjusted to what the channel likes — this is [Matching](#matching).

## From 2×2 to 6×6 {#linear-optics-6d}

A real beam has three directions: horizontal $(x, x')$, vertical $(y, y')$ and the direction of motion. Along the motion we use the position $z$ of a particle relative to the reference particle and the relative momentum deviation $\delta = \Delta p/p$. Putting the six numbers together, the transfer table has 6 rows and 6 columns, the σ-matrix is 6 × 6 as well, and it is transported by the same rule; this is also what AVAS's linear preview does.

In most elements the three directions do not affect each other, and the big table is made of three 2 × 2 tables along the diagonal with zeros everywhere else:

$$
M = \begin{pmatrix} M_x & 0 & 0 \\ 0 & M_y & 0 \\ 0 & 0 & M_z \end{pmatrix}, \qquad
M_z^{\mathrm{drift}} = \begin{pmatrix} 1 & L/\gamma^2 \\ 0 & 1 \end{pmatrix} .
$$

The $L/\gamma^2$ in the longitudinal table comes from speed differences: a particle with $\delta$ more momentum is faster by $\delta/\gamma^2$ ($\gamma$ is the relativistic factor, about 1 at low speed; see [Kinematics](#kinematics-differential)) and after $L$ is $L\delta/\gamma^2$ ahead of the reference particle. It is exactly like the transverse drift's $L$: the longitudinal "angle" is the momentum deviation, the "position" the longitudinal offset. Near the speed of light $1/\gamma^2 \to 0$ and the longitudinal motion is almost frozen, see [Drift](#drift-longitudinal).

Non-zero entries outside the small tables mean that two directions are "mixed" (**coupling**):

- a **solenoid** couples $x$ and $y$ (the particles rotate about the axis); only in the co-rotating Larmor frame do they separate into two independent directions, see [Solenoid](#solenoid);
- a **dipole** correlates $x$ with $\delta$ (**dispersion**: different momenta are bent differently) and, through path-length differences, $z$ with $x$, see [Dipole](#dipole);
- the bunching of an **RF cavity** is a "lens" in the longitudinal table ($\delta$ changes with $z$), with transverse defocusing at the same time, see [Longitudinal dynamics](#longitudinal).

How AVAS's linear preview treats each element is in [In AVAS](#linear-optics-in-avas).

## Think about it {#linear-optics-think}

> **Think about it 1**: why is the element passed later written on the left?
>
> Answer: a table acts on the column of numbers to its right. The particle first passes $M_1$ and becomes $M_1X_0$, then passes $M_2$ and becomes $M_2(M_1X_0)$. The table that acts first is closest to the column, so it stands rightmost.

> **Think about it 2**: what happens if every quadrupole of a FODO channel is given the same polarity?
>
> Answer: $x$ is focused everywhere, no problem; $y$ is defocused everywhere, the paths grow exponentially and the beam soon hits the wall in $y$. The alternating arrangement is not there to be "better" — it is the only way to keep both directions stable at once (a solenoid focuses both directions at once, but its strength scales as $(B/B\rho)^2$ and is an advantage only at low energy, see [Solenoid](#solenoid)).

> **Think about it 3**: focusing is good, so why does focusing that is too strong become unstable?
>
> Answer: every lens pulls the particle back, and pulling too hard "overcorrects": the particle crosses the axis to the other side and ends up farther from it. At $\mu = 180°$ the particle flips to the other side exactly once per cell; a little stronger and the amplitude grows at every flip, $\lvert\Tr M\rvert > 2$. In thin-lens terms, the focal length is shorter than half the lens spacing, and the beam has already crossed the axis and spread out before it reaches the next lens.

> **Think about it 4**: the focusing strength contains $G/(B\rho)$, not the gradient $G$ itself. How differently does the same quadrupole act on a 1.5 MeV proton and a 1 GeV proton?
>
> Answer: $B\rho$ is proportional to the momentum. A 1 GeV proton has $B\rho = 5.66\ \mathrm{T\,m}$, 32 times that at 1.527 MeV, so the same quadrupole's $k$ drops to 1/32. That is why high-energy quadrupoles are longer or have higher gradients.

> **Think about it 5**: the phase advance per cell of the example cell is 48.3°. If the emittance of the beam is doubled, does the phase advance change?
>
> Answer: no. The phase advance is set by the cell's table alone and describes how fast a single particle swings; doubling the emittance only makes the beam $\sqrt2$ times wider (without space charge).

## Common misconceptions {#linear-optics-misconceptions}

- **Two βs**: the Twiss parameter $\beta_x$ is a length (m, i.e. mm/mrad), the relativistic factor $\beta = v/c$ has no unit. Where both appear in this chapter, the one with a subscript is the Twiss parameter.
- **The beam is narrowest in the focusing quadrupole?** The opposite. In a FODO $\beta$ is largest at the focusing quadrupole and smallest at the defocusing one; the focusing lens makes the beam narrower *after* it.
- **The waist is at the focus?** Only for a "parallel beam of light" with zero emittance. For a beam with emittance the waist at $s^* = \alpha/\gamma_x$ is closer to the lens than the geometric focus (example above: focal length 0.5 m, waist at 0.4 m).
- **The thin-lens approximation is always good enough?** When the quadrupole's $\sqrt{k}L$ reaches about 0.5, the phase advance is already off by 5°–6° and the estimated instability gradient is about 10 % too low.
- **The mismatch factor depends only on $\beta$?** No. A wrong $\alpha$ is a mismatch too; in the lab above $\beta$ is only scaled by 1.69, but because $\alpha \ne 0$ the mismatch factor reaches 0.61 (see [the deeper section](#linear-optics-deep-periodic)).
- **The matched beam is the smallest beam?** Matching means "the same shape in every cell", not the smallest size. The periodic β is set by the structure; for a narrower beam change the structure (stronger focusing, shorter cells).
- **$\det M = 1$ always holds?** Only in canonical coordinates. In $(x, x')$ with acceleration $\det M < 1$ (adiabatic damping): the geometric emittance shrinks, the normalised emittance stays.
- **Stability at zero current is enough?** It is only necessary. Space charge depresses the phase advance and excites resonances; for intense beams design with $\sigma_0 < 90°$.
- **The phase advance does not depend on the current?** At zero current it is set by the structure alone, independent of the emittance; with space charge the single-particle phase advance drops with current, see [Space charge](#space-charge-tune-depression).

## Going deeper: from impulse to Hill's equation {#linear-optics-deep-hill}

The main text wrote $x'' = F/(pv)$ with the impulse theorem. This section turns it into numbers, adds the extra term that appears with acceleration, and shows how electric and magnetic fields are written. The first four steps use only school physics; the last paragraph is for readers who know vector analysis.

### Step 1: a real impulse

The example proton has a kinetic energy of 1.527 MeV, a speed $v = 1.708\times10^{7}$ m/s (5.7 % of the speed of light) and a momentum $p = 2.862\times10^{-20}$ kg·m/s. It crosses a quadrupole 0.1 m long with a gradient of 4 T/m at 1 mm from the axis:

- the field there is $B_y = G\,x = 4 \times 0.001 = 0.004$ T;
- the Lorentz force is $F = q\,v\,B_y = 1.602\times10^{-19} \times 1.708\times10^{7} \times 0.004 = 1.095\times10^{-14}$ N;
- crossing the magnet takes $t = 0.1/v = 5.85$ ns;
- the impulse is $F\,t = 6.41\times10^{-23}$ kg·m/s; divided by $p$ the angle changes by $2.24\times10^{-3}$, i.e. 2.24 mrad.

Exactly what the main text got from $k L x$. The force is only $10^{-14}$ N, but the proton's momentum is small too, so the deflection is not small.

### Step 2: magnetic and electric rigidity

Substituting $F = -q v B_y$ into $x'' = F/(pv)$, $v$ cancels and only $q/p$ remains, hence the magnetic rigidity $B\rho = p/q$. A handy formula is

$$
B\rho\ [\mathrm{T\,m}] = \frac{p\ [\mathrm{MeV}/c]}{299.792458\,Z},
$$

with $Z$ the charge number. For the example proton $p = 53.55$ MeV/$c$ and $B\rho = 53.55/299.79 = 0.1786$ T·m. For an electric field $F_x = qE_x$, $v$ does not cancel and $x'' = E_x/(E\rho)$ with the **electric rigidity** $E\rho = pv/q$ (see [Relativistic kinematics](#kinematics-rigidity)).

### Step 3: Hill's equation

When the field is linear in the distance from the axis, for instance a quadrupole with $B_y = G\,x$, $B_x = G\,y$, Step 2 gives $x'' = -k\,x$, $y'' = +k\,y$ with $k = G/B\rho$. Writing every element as $x'' + K(s)\,x = 0$ gives Hill's equation of the main text, where the table of $K(s)$ also is.

### Step 4: the extra term with acceleration

The angle is transverse momentum divided by forward momentum: $x' = p_x/p$. If over some distance the forward momentum grows from $p$ to $p + \Delta p$ while the transverse momentum stays, the angle shrinks to $p/(p + \Delta p)$ of its value, i.e. $\Delta x' \approx -(\Delta p/p)\,x'$. Adding the contribution of the force, the equation of motion becomes

$$
x'' + \frac{p'}{p}\,x' = \frac{F_x}{p\,v},
$$

where $p'/p$ is the "relative growth rate of the momentum along the path", equal to $(\beta\gamma)'/(\beta\gamma)$ and non-zero only where there is acceleration. This term makes the angles shrink with acceleration: [adiabatic damping](#phase-space-liouville). Example: the example HWR cavity raises the energy from 1.527 MeV to about 1.87 MeV, $\beta\gamma$ grows from 0.0571 to 0.0631, and the same transverse momentum corresponds to an angle about 10 % smaller.

### For readers who know vector analysis

The exact paraxial equation is $\dd(p\,x')/\dd s = F_x/v$, which expands to the equation above. The quadrupole's $K_y = -K_x$ comes from $\nabla\times\boldsymbol B = 0$ in the aperture: $(\nabla\times\boldsymbol B)_z = \partial B_y/\partial x - \partial B_x/\partial y = 0$, so $\partial B_y/\partial x = \partial B_x/\partial y$, while $K_x = (\partial B_y/\partial x)/B\rho$ and $K_y = -(\partial B_x/\partial y)/B\rho$. This is a limit set by Maxwell's equations, not a lack of clever design. The solenoid's $k_s^2$ comes from the longitudinal field $B_z$ and the particle's own transverse velocity; only in a frame rotating at the Larmor frequency are the two directions equal and decoupled. In the dipole's $(1-N)/\rho^2$, the $1/\rho^2$ is the geometric focusing of the curved orbit itself.

## Going deeper: thick lenses, symplecticity and the inverse matrix {#linear-optics-deep-matrix}

The main text treated the quadrupole as a thin lens. This section derives step by step the table of a quadrupole with a length, and shows why all these tables "preserve area".

### Step 1: inside the magnet it is simple harmonic motion

Inside a quadrupole $K = k$ is constant, and $x'' = -k\,x$ is the equation of a spring oscillator (with time replaced by distance). Its solution is

$$
x(s) = x_0\cos\!\left(\sqrt{k}\,s\right) + \frac{x_0'}{\sqrt{k}}\sin\!\left(\sqrt{k}\,s\right);
$$

putting $s = 0$ gives $x = x_0$ and an angle (rate of change of $x$ with $s$) of $x_0'$, so the two coefficients are fixed by the entrance state. In the defocusing direction $y'' = +k\,y$ the solution no longer oscillates but grows faster and faster; it uses the **hyperbolic functions** $\cosh u = (\ee^u + \ee^{-u})/2$ and $\sinh u = (\ee^u - \ee^{-u})/2$, which play the same role as $\cos$ and $\sin$.

### Step 2: the thick-lens matrix

Let $\phi = \sqrt{k}\,L$, the "oscillation phase" the particle goes through in the magnet. Writing the solution of Step 1 at $s = L$ as a table (first column the cosine-like path, second column the sine-like path):

$$
M_{\mathrm F} = \begin{pmatrix} \cos\phi & \dfrac{\sin\phi}{\sqrt{k}} \\ -\sqrt{k}\sin\phi & \cos\phi \end{pmatrix}, \qquad
M_{\mathrm D} = \begin{pmatrix} \cosh\phi & \dfrac{\sinh\phi}{\sqrt{k}} \\ \sqrt{k}\sinh\phi & \cosh\phi \end{pmatrix} .
$$

$M_{\mathrm F}$ is the focusing direction, $M_{\mathrm D}$ the defocusing one.

### Step 3: equivalent focal length and the thin-lens approximation

Comparing with the thin-lens table, the equivalent focal length of the thick lens is given by $1/f = -M_{21}$, i.e. $1/f = \sqrt{k}\sin\phi$ in the focusing direction. For small $\phi$, $\sin\phi \approx \phi$ and $\cos\phi \approx 1$, so $1/f \approx kL$ and the table reduces to a thin lens: the **thin-lens approximation**. A small $\phi$ means the particle has hardly any time to change its position in the lens, and the whole lens can be compressed into one kick.

### Step 4: a numerical example

A 1.527 MeV proton ($B\rho$ = 0.17863 T·m) and a quadrupole with $G$ = 20 T/m, $L$ = 0.05 m: $k$ = 111.96 m$^{-2}$, $\phi$ = 0.529,

$$
M_{\mathrm F} = \begin{pmatrix} 0.86328 & 0.04770\ \mathrm m \\ -5.34063\ \mathrm m^{-1} & 0.86328 \end{pmatrix}, \qquad
M_{\mathrm D} = \begin{pmatrix} 1.14325 & 0.05237\ \mathrm m \\ 5.86300\ \mathrm m^{-1} & 1.14325 \end{pmatrix} .
$$

The equivalent focal length is 0.1872 m in the focusing direction and $-0.1706$ m in the defocusing one, while the thin-lens approximation gives $\pm 0.1786$ m. The thick lens is weaker than the thin one in the focusing direction and stronger in the defocusing one: in a focusing lens the particle approaches the axis as it goes and feels less force in the second half; in a defocusing lens it moves away from the axis and feels more. At $\phi \approx 0.5$ the thin-lens approximation is already off by about 5 %, common in strongly focusing low-energy sections, so AVAS's linear preview uses the thick-lens matrix for `quad` (the manual does not state the internal form of the engine's matrix elements; the engine's results are authoritative).

### Step 5: why the area stays

The determinant $\det M = CS' - SC'$ of a table is the factor by which it multiplies phase-space areas. Thick lens: $\cos^2\phi + \sin^2\phi = 1$; defocusing direction: $\cosh^2\phi - \sinh^2\phi = 1$ (substitute the definitions and expand to check). With the numbers above: $0.86328^2 + 0.04770 \times 5.34063 = 0.74525 + 0.25475 = 1.00000$.

A determinant of 1 also gives a convenient **inverse matrix** (working back from the exit to the entrance):

$$
M^{-1} = \begin{pmatrix} S' & -S \\ -C' & C \end{pmatrix};
$$

multiply it with $M$ by the table rule: the upper left entry is $CS' - SC' = 1$, the upper right $SS' - SS' = 0$, which is exactly the "do-nothing" unit table.

### Step 6: with acceleration

In an accelerating section the angles shrink by $p_0/p_1 = (\beta\gamma)_0/(\beta\gamma)_1$, so in $(x, x')$ coordinates $\det M = (\beta\gamma)_0/(\beta\gamma)_1 < 1$. In $(x,\ \beta\gamma\,x')$ coordinates (the second number is proportional to the transverse momentum) the matrix is again symplectic with determinant 1, and the normalised emittance is conserved. $(x, x')$ are not "canonical coordinates" in the sense of mechanics, so their shrinking area does not violate Liouville's theorem.

### For readers who know linear algebra

In the linear case [Liouville's theorem](#phase-space-liouville) says that the transfer matrix is **symplectic**,

$$
M\tp J\,M = J, \qquad J = \begin{pmatrix} 0 & 1 \\ -1 & 0 \end{pmatrix} .
$$

In several dimensions $J$ is block diagonal with one 2 × 2 block per plane. For a 2 × 2 matrix the symplectic condition is equivalent to $\det M = 1$; in several dimensions it is stronger than $\det M = 1$ (it preserves the sum of the projected areas on every pair of conjugate coordinates, not only the total volume). The inverse of a symplectic matrix is $M^{-1} = -J M\tp J$, which for 2 × 2 is the formula above.

## Going deeper: transporting the ellipse, the envelope equation and the phase {#linear-optics-deep-twiss}

The main text stated how a drift and a thin lens change the Twiss parameters, and that a particle's path looks like a cosine with varying amplitude and wavelength. This section derives these results step by step, starting from "square and average".

### Step 1: square and average

A particle is transported by $x = C x_0 + S x_0'$, $x' = C' x_0 + S' x_0'$. Square the first line with the formula for a perfect square and average over all particles (averaging is linear and $C$, $S$ are the same for every particle, so they come out of the average):

$$
\langle x^2\rangle = C^2\langle x_0^2\rangle + 2CS\,\langle x_0x_0'\rangle + S^2\langle x_0'^2\rangle .
$$

Do the same for $\langle x x'\rangle$ and $\langle x'^2\rangle$, divide by the emittance $\varepsilon$ (which does not change), and use $\langle x^2\rangle = \beta\varepsilon$, $\langle xx'\rangle = -\alpha\varepsilon$, $\langle x'^2\rangle = \gamma\varepsilon$ to switch to Twiss parameters:

$$
\begin{pmatrix} \beta \\ \alpha \\ \gamma \end{pmatrix}_1 =
\begin{pmatrix} C^2 & -2CS & S^2 \\ -CC' & CS' + SC' & -SS' \\ C'^2 & -2C'S' & S'^2 \end{pmatrix}
\begin{pmatrix} \beta \\ \alpha \\ \gamma \end{pmatrix}_0 .
$$

The shape at the exit depends only on the shape at the entrance, not on how the particles are distributed in detail (a point that matters in the chapter on [initial distributions](#distributions-rms)).

### Step 2: two special cases

**Drift** ($C = 1$, $S = s$, $C' = 0$, $S' = 1$): $\beta(s) = \beta_0 - 2\alpha_0 s + \gamma_0 s^2$, $\alpha(s) = \alpha_0 - \gamma_0 s$, $\gamma$ unchanged. $\beta$ is a quadratic function of $s$; completing the square gives its minimum: the waist is at $s^* = \alpha_0/\gamma_0$, where $\alpha = 0$ and $\beta^* = 1/\gamma_0$ (using $\beta\gamma - \alpha^2 = 1$), and at a distance $s$ from the waist $\beta = \beta^* + s^2/\beta^*$.

**Thin lens** ($C = 1$, $S = 0$, $C' = -1/f$, $S' = 1$): $\beta_1 = \beta_0$, $\alpha_1 = \alpha_0 + \beta_0/f$, $\gamma_1 = \gamma_0 + 2\alpha_0/f + \beta_0/f^2$.

Check the example of the main text: $\beta_0 = 1$ m, $\alpha_0 = 0$, $\gamma_0 = 1\ \mathrm{m^{-1}}$, $f = 0.5$ m: $\alpha_1 = 2$, $\gamma_1 = 1 + 0 + 4 = 5\ \mathrm{m^{-1}}$, waist at $2/5 = 0.4$ m, $\beta^* = 0.2$ m. In the lab, after another 0.5 m: $\beta = 1 - 2 \times 2 \times 0.5 + 5 \times 0.25 = 0.25$ m and $\alpha = 2 - 5 \times 0.5 = -0.5$, exactly the lab's readings.

### Step 3: step by little step: the differential form

Cut any beam line into very short pieces $\Delta s$ and treat each as "a drift + a thin lens with $1/f = K\Delta s$". From Step 2:

- the drift changes $\beta$ by $-2\alpha\,\Delta s$ (the $\Delta s^2$ term is too small and is dropped) and $\alpha$ by $-\gamma\,\Delta s$;
- the thin lens changes $\alpha$ by $\beta/f = K\beta\,\Delta s$ and leaves $\beta$ alone.

Dividing by $\Delta s$ gives the rates of change of the Twiss parameters along the beam line:

$$
\beta' = -2\alpha, \qquad \alpha' = K\beta - \gamma .
$$

### Step 4: the envelope equation

With $w = \sqrt{\beta}$, Step 3 leads to $w'' + K w - 1/w^3 = 0$ (the derivation needs differentiation, see the end of this section). Multiplying by $\sqrt{\varepsilon}$ gives the equation of the rms envelope $a = \sqrt{\varepsilon\beta}$:

$$
a'' + K a - \frac{\varepsilon^2}{a^3} = 0 .
$$

It differs from the single-particle Hill equation only by the last term. That term always pushes outwards, like the pressure of a gas: the emittance reflects the disordered transverse motion of the particles, and however strong the external focusing, a beam with emittance cannot be squeezed into a point — the smaller it is, the faster this term grows ($\propto 1/a^3$). Check one case with school algebra: for constant $K$ and a beam whose size does not change ($a'' = 0$), $K a = \varepsilon^2/a^3$, i.e. $a^2 = \varepsilon/\sqrt{K}$ — exactly the $\beta = 1/\sqrt{K}$ of the main text. With space charge the equation gains one more outward term, and becomes the envelope equation of the [Space charge](#space-charge) chapter.

### Step 5: how the phase advances

The main text said that the phase advances by $\Delta s/\beta$ for every $\Delta s$, and that the path is $x = \sqrt{J\beta}\cos(\psi + \psi_0)$ with $\psi = \int \dd s/\beta$ ($\int$ means adding up the pieces $\Delta s/\beta$). Test it on a drift that starts at a waist. From the waist $\beta = \beta^* + s^2/\beta^*$, and adding up $\Delta s/\beta$ by the rule above (this sum needs an integral; here we check the result another way) gives the phase advance

$$
\Delta\psi = \arctan\frac{s}{\beta^*} .
$$

Check: the upper right entry of the table written with the phase should be $\sqrt{\beta_0\beta_1}\,\sin\Delta\psi$ (see Step 6); here $\beta_0 = \beta^*$, $\beta_1 = (\beta^{*2} + s^2)/\beta^*$, $\sin\Delta\psi = s/\sqrt{\beta^{*2} + s^2}$, and the product is exactly $s$, in agreement with the drift table $\begin{pmatrix} 1 & s \\ 0 & 1 \end{pmatrix}$. Numbers: $\beta^* = 0.2$ m and 0.4 m of drift give a phase advance of $\arctan 2 = 63.4°$. The phase advance of a drift never exceeds 90° (counted from the waist), and from infinitely far upstream to infinitely far downstream it adds up to only 180°, so drifts alone cannot make a particle circle the axis.

### Step 6: any matrix in terms of Twiss parameters and phase advance

Given the Twiss parameters at both ends and the phase advance $\Delta\psi$, any transfer matrix can be written as

$$
M = \begin{pmatrix}
\sqrt{\dfrac{\beta_1}{\beta_0}}\left(\cos\Delta\psi + \alpha_0\sin\Delta\psi\right) & \sqrt{\beta_0\beta_1}\,\sin\Delta\psi \\
\dfrac{(\alpha_0 - \alpha_1)\cos\Delta\psi - (1 + \alpha_0\alpha_1)\sin\Delta\psi}{\sqrt{\beta_0\beta_1}} & \sqrt{\dfrac{\beta_0}{\beta_1}}\left(\cos\Delta\psi - \alpha_1\sin\Delta\psi\right)
\end{pmatrix} .
$$

Conversely, given $M$ and the entrance Twiss parameters, the phase advance follows from

$$
\tan\Delta\psi = \frac{M_{12}}{\beta_0 M_{11} - \alpha_0 M_{12}}
$$

(choosing the quadrant by the sign of $M_{12}$). The emittance does not appear: the phase advance describes how far a single particle has turned, not how big the beam is.

### For readers who know calculus

All of the above is $\Sigma_1 = M\,\Sigma_0\,M\tp$ (with $\Sigma = \varepsilon\begin{pmatrix} \beta & -\alpha \\ -\alpha & \gamma \end{pmatrix}$); since $\det\Sigma_1 = (\det M)^2\det\Sigma_0$, the rms emittance is constant under symplectic transport. Writing the general solution of Hill's equation in Floquet form $x = \sqrt{J}\,w(s)\cos(\psi + \psi_0)$, substituting into $x'' + Kx = 0$ and setting the coefficients of $\cos$ and $\sin$ to zero: the $\sin$ term gives $(w^2\psi')' = 0$, normalised to $\psi' = 1/w^2 = 1/\beta$; the $\cos$ term gives $w'' + Kw - 1/w^3 = 0$. $J = \gamma x^2 + 2\alpha x x' + \beta x'^2$ is the [Courant–Snyder invariant](#phase-space-twiss).

## Going deeper: periods, the stability criterion and mismatch {#linear-optics-deep-periodic}

The main text gave the stability criterion $\lvert\tfrac12\Tr M\rvert < 1$ and the sequences obtained by repeated multiplication. This section proves it with school sequences and trigonometric identities, and then derives the thin-lens FODO formulas and the mismatch factor.

### Step 1: the square of a table

Let the table of one cell be $M = \begin{pmatrix} a & b \\ c & d \end{pmatrix}$ with determinant $ad - bc = 1$ and trace $T = a + d$. Computing $M^2$ by the table rule, its upper left entry is $a^2 + bc$; $T\,M$ minus the unit table has upper left entry $a^2 + ad - 1$. Since $ad - 1 = bc$ the two are equal; the other three entries check the same way. Hence

$$
M^2 = T\,M - I ,
$$

where $I = \begin{pmatrix} 1 & 0 \\ 0 & 1 \end{pmatrix}$ is the "do-nothing" unit table.

### Step 2: a sequence

Applying this to the state of the particle after $n - 1$ cells gives: the position $x_{n+1}$ after $n + 1$ cells, $x_n$ after $n$ cells and $x_{n-1}$ after $n - 1$ cells satisfy

$$
x_{n+1} = T\,x_n - x_{n-1} .
$$

This is a recursive sequence of the kind taught at school. Check the numbers of the main text ($G = 4$ T/m, $T = 2 \times 0.666 = 1.332$): $x_2 = 1.332 \times 0.666 - 1 = -0.113$, $x_3 = 1.332 \times (-0.113) - 0.666 = -0.817$, as in the main text.

**The case $\lvert T\rvert < 2$**: write $T = 2\cos\mu$. The sum-to-product identity $\cos(\theta + \mu) + \cos(\theta - \mu) = 2\cos\mu\cos\theta$ shows that $x_n = A\cos(n\mu + \varphi)$ satisfies the sequence, so the particle advances $\mu$ in its swing per cell and the amplitude $A$ never changes: **stable**. At $G = 4$ T/m, $\cos\mu = 0.666$ and $\mu = 48.3°$; starting parallel at the centre of the focusing quadrupole, $x_n = \cos(n \times 48.3°)$.

**The case $\lvert T\rvert > 2$**: try $x_n = \lambda^n$; substituting gives $\lambda^2 - T\lambda + 1 = 0$. This quadratic equation has discriminant $T^2 - 4 > 0$ and thus two real roots whose product is 1 (Vieta's formulas), so one root has absolute value larger than 1 and $x_n$ grows with its $n$-th power: **unstable**. At $G = 10$ T/m, $T/2 = -1.083$ and the root $\lambda = -1.083 - \sqrt{1.083^2 - 1} = -1.50$: a sign flip and a factor 1.5 every cell, exactly the sequence of the main text.

The boundary between the two cases, $T = \pm 2$, corresponds to $\mu = 0°$ and $180°$. So the stability condition is $\lvert\tfrac12\Tr M\rvert < 1$, and the phase advance is given by $\cos\mu = \tfrac12\Tr M$. The trace does not depend on the point of the cell where we start counting, so $\mu$ is a property of the whole channel.

### Step 3: periodic Twiss parameters

When the cell is stable its table can be written as

$$
M = \begin{pmatrix} \cos\mu + \alpha\sin\mu & \beta\sin\mu \\ -\gamma\sin\mu & \cos\mu - \alpha\sin\mu \end{pmatrix},
$$

which is the general formula of Step 6 of the previous section when the Twiss parameters at both ends are the same. Reading off the periodic solution: $\beta = M_{12}/\sin\mu$, $\alpha = (M_{11} - M_{22})/(2\sin\mu)$, $\gamma = -M_{21}/\sin\mu$, with the sign of $\sin\mu$ chosen so that $\beta > 0$. They depend only on the periodic structure, not on the beam, and the ellipse they define returns to itself after one cell: the matched beam.

Example: counted from the centre of the defocusing quadrupole of the example cell, the thick-lens table is $\begin{pmatrix} 0.6659 & 0.4809\ \mathrm m \\ -1.1575\ \mathrm{m^{-1}} & 0.6659 \end{pmatrix}$. $\sin 48.25° = 0.7462$, so $\beta = 0.4809/0.7462 = 0.645$ m, $\alpha = 0$ (a symmetry point), $\gamma = 1.1575/0.7462 = 1.551\ \mathrm{m^{-1}} = 1/\beta$. Counted from the centre of the focusing quadrupole, $\beta = 1.485$ m.

### Step 4: the thin-lens FODO

Two thin lenses with focal lengths $\pm f$ a distance $L$ apart (cell length $2L$). First multiply the three tables "focusing lens – drift $L$ – defocusing lens" (the numbers in the main text's alternating-gradient section are one example of it), giving $\begin{pmatrix} 1 - L/f & L \\ -L/f^2 & 1 + L/f \end{pmatrix}$; then multiply by one more drift $\begin{pmatrix} 1 & L \\ 0 & 1 \end{pmatrix}$. The sum of the diagonal is

$$
\Tr M = \left(1 - \frac{L}{f} - \frac{L^2}{f^2}\right) + \left(1 + \frac{L}{f}\right) = 2 - \frac{L^2}{f^2} .
$$

So $\cos\mu = 1 - L^2/(2f^2)$. With the half-angle formula $1 - \cos\mu = 2\sin^2(\mu/2)$:

$$
\sin\frac{\mu}{2} = \frac{L}{2f}, \qquad
\beta_{\max,\min} = \frac{2L\left(1 \pm \sin(\mu/2)\right)}{\sin\mu} .
$$

The left side cannot exceed 1, so stability requires $f > L/2$. $\beta_{\max}$ is at the centre of the focusing lens, $\beta_{\min}$ at the centre of the defocusing one. The average β can be estimated with the smooth approximation $\bar\beta \approx 2L/\mu$.

### Step 5: comparing numbers

The example cell ($L_q = 0.1$ m, $G = \pm 4$ T/m, $L_d = 0.3$ m, period 0.8 m):

- thick-lens matrices: $\tfrac12\Tr M = 0.666$, $\mu = 48.3°$; the periodic $\beta_x$ varies between 1.485 m and 0.645 m; smooth approximation $\bar\beta \approx 0.8/0.842 = 0.95$ m.
- Thin-lens estimate ($f = 0.447$ m, lens spacing 0.4 m): $\sin(\mu/2) = 0.448$, $\mu = 53.2°$, $\beta$ between 1.446 m and 0.552 m. The quadrupole's $\phi = \sqrt{k}L_q = 0.47$; the thin-lens approximation overestimates the phase advance by about 5°.
- At a gradient of 6.92 T/m, $\mu = 90°$; at 9.80 T/m, $\tfrac12\lvert\Tr M\rvert$ reaches 1 and the beam becomes unstable (the thin-lens approximation gives 8.93 T/m).

Another example uses short, strong quadrupoles: QF (0.05 m) – drift 0.10 m – QD (0.05 m) – drift 0.10 m, $G$ = 20 T/m. Thick-lens matrices give $\tfrac12\Tr M = 0.726$, $\mu$ = 43.45°, and at the QF entrance $\beta_x$ = 0.546 m, $\beta_y$ = 0.303 m; the thin-lens estimate $\sin(\mu/2) = 0.420$, $\mu$ = 49.65° is about 6° too high; at about 38.2 T/m $\mu$ = 90°, and at about 54.1 T/m the cell becomes unstable (the thin-lens approximation gives 47.6 T/m).

### Step 6: the mismatch factor

To measure how far the beam (Twiss parameters with subscript 1) is from the periodic solution (subscript 2), define

$$
R = \beta_1\gamma_2 + \beta_2\gamma_1 - 2\alpha_1\alpha_2, \qquad
M_{\mathrm{mis}} = \sqrt{\frac{R + \sqrt{R^2 - 4}}{2}} - 1 .
$$

For a perfect match $R = 2$ and $M_{\mathrm{mis}} = 0$. If only $\beta$ differs ($\alpha = 0$, $\beta_1 = \lambda\beta_2$), $R = \lambda + 1/\lambda$ and $M_{\mathrm{mis}} = \sqrt{\lambda} - 1$, the fraction by which the largest rms size grows.

Check the lab of the main text: with `mismatch` = 1.3, $\beta_1 = 1.69\,\beta_2$ while both $\alpha$ have the matched value $-1.57$; $R = 1.69 \times 3.459 + 3.459/1.69 - 2 \times 2.459 = 2.97$, and the mismatch factor is 0.61 rather than 0.3 (because $\alpha \ne 0$). The largest envelope grows from 1.91 mm to 3.05 mm, 60 % larger, close to the upper limit $1 + M_{\mathrm{mis}} = 1.61$ (the limit is reached only when the phase of the envelope beat coincides with a β maximum, which in 10 cells it just misses). If nonlinear forces filament the mismatched ellipse completely, the rms emittance grows to $R/2$ times its value, here 1.49 times.

### For readers who know linear algebra

With $\det M = 1$ the eigenvalues satisfy $\lambda^2 - (\Tr M)\lambda + 1 = 0$; $M^2 = TM - I$ of Step 1 is the Cayley–Hamilton theorem. For $\lvert\Tr M\rvert < 2$, $\lambda = \ee^{\pm i\mu}$ lie on the unit circle and $M^n$ is bounded; for $\lvert\Tr M\rvert > 2$ one eigenvalue has modulus larger than 1 and $M^n$ grows exponentially. When stable, $M = I\cos\mu + J_T\sin\mu$ with $J_T = \begin{pmatrix} \alpha & \beta \\ -\gamma & -\alpha \end{pmatrix}$ and $J_T^2 = -I$, the form Floquet theory takes for linear periodic systems; the matched σ-matrix satisfies $M\Sigma M\tp = \Sigma$.

## In AVAS {#linear-optics-in-avas}

### Matrix elements and beam parameters

The elements of a lattice that act as a whole transfer matrix (z-code matrix-model elements) are `quad`, `solenoid`, `bend`, `edge` and `steerer`; `drift` is a t-code field-model element (see [Drift](#drift-in-avas)). Their parameters and sign conventions are described in the element chapters:

```avas-params
drift quad solenoid bend edge steerer
```

The entrance Twiss parameters of the beam are in `beam.txt`:

```avas-params
beam:twissx beam:twissy
```

> **AVAS convention**: `twissx` and `twissy` give $\alpha$, $\beta$ (mm/mrad) and the **normalised** rms emittance (π·mm·mrad), in that order; the rms size is $\sqrt{\beta\varepsilon_n/(\beta\gamma)}$. The example beam `twissx -0.65785 0.37743 0.13768` has an rms $x$ of 0.954 mm.

### Linear preview

The linear preview of the visual editor follows [From 2×2 to 6×6](#linear-optics-6d): it pushes the 6 × 6 σ-matrix with 6 × 6 matrices acting on $(x, x', y, y', z, \delta)$, $\Sigma_1 = M\Sigma_0 M\tp$, and gives the rms envelope almost instantly while you drag parameters. The elements are treated as follows:

- **drift, quad**: transversely as in the main text (`quad` with the thick-lens matrix), longitudinally like a drift;
- **solenoid**: an $x$–$y$ coupled 4 × 4 block (hard edge, $C = \cos k_sL$, $S = \sin k_sL$);
- **bend, edge**: the sector dipole includes the field index $N$, with $x$–$\delta$ and $z$–$x$ correlation terms (dispersion); the edge matrix is a thin pole-face rotation;
- **steerer** and diagnostics are unit matrices in the preview (they change only the central orbit, not the second moments);
- **field maps**: the reference particle integrates energy and time in 1 mm steps; the optics uses 4 mm slices, each "half a step in the local $B_z$ – a thin-lens kick from the paraxial field gradients (including RF bunching) – another half step", including the damping from the momentum gain;
- **space charge** (optional): every few steps a linear kick of the rms-equivalent uniform ellipsoid (bunched beam) or elliptical cylinder (DC beam) is applied.

> **AVAS convention**: the linear preview is only for quick estimates. The reference-particle energy is accurate to about 0.01 % (compared with the engine's synchronous particle; the energy in the engine's DataSet is the average over all macro-particles, and particles off axis see a different $E_z$ — in the example HWR the average gain is about 5 % above that of the reference particle), rms sizes typically differ by 1–2 %, and up to 13 % when multi-particle effects are strong; it does not include dipole / steering components in field maps, `superposeout` offsets, chromatic and nonlinear effects (such as longitudinal emittance growth in low-energy cavities), or particle losses. The engine's results are authoritative.

> Note: the 6 × 6 matrices of drift, quad, solenoid, bend and edge in the linear preview are all symplectic (checked numerically in the three conjugate pairs $(x, x')$, $(y, y')$, $(z, \delta)$); only the accelerating slices of field maps have a determinant different from 1.

The labs of this primer (phase space, FODO, solenoid, dipole, RF cavity, space charge) and the particle animations call the matrices and code of the linear model directly, so their results agree with the preview in the visual editor and, likewise, the engine's results are authoritative.

## Further reading {#linear-optics-reading}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, 2008: transverse dynamics, FODO and periodic systems.
- S. Y. Lee, *Accelerator Physics*, 4th ed., World Scientific: Hill's equation, the Floquet transformation and Courant–Snyder parameters.
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015: transfer matrices and the evolution of Twiss parameters.
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Annals of Physics* 3 (1958) 1.
- K. L. Brown, "A first- and second-order matrix theory for the design of beam transport systems and charged particle spectrometers", SLAC-75: TRANSPORT matrix conventions.
- Read on: [Longitudinal dynamics](#longitudinal), [Space charge](#space-charge), [Quadrupole](#quadrupole), [Lattice design](#lattice-design), [Matching](#matching).
