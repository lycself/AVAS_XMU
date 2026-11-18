# Controllability and observability

In the [Orbit correction](#orbit-correction) chapter we corrected the orbit with 12 BPMs and 12 steerers and found two things: there is one kind of orbit (a bulge between BPMs) that the BPMs can hardly see, and correcting it takes enormous steerer strength. This is not a quirk of that particular channel; these are the two questions every control problem should ask before any computing starts:

- Can my **knobs** push the system into **every** state I might want? This is **controllability**.
- Can my **monitors** tell **which** state the system is in? This is **observability**.

Both concepts were introduced around 1960 by the control theorist R. E. Kalman and are the foundation of modern control theory. This chapter first explains them with everyday examples, then experiments with the orbit-correction channel: remove some BPMs or steerers and see what is corrected well, what badly, and what cannot be seen at all; then it introduces the two mathematical tools that judge them, **rank** and **singular values**. The "going deeper" part gives the textbook definitions, using exactly the $(A, B, C)$ matrices of one period that AVAS computes.

## Two questions {#controllability-questions}

Start with a tap. A washbasin has a cold and a hot valve, and you care about two things: how much water (flow) and how hot (temperature). Turning the two valves together, you can set both flow and temperature as you like. Two knobs, two things to control: just right.

Now suppose a plumber made a mistake and connected both valves to the cold pipe. You still have two knobs, but however you turn them you only change the flow; the temperature stays cold. **The number of knobs is not the number of independently adjustable directions**: the two knobs really do the same thing. That is "uncontrollable": some states (hot water) cannot be reached however you turn the knobs.

Now monitoring. Driving a car, you have a speedometer. From the speedometer alone you never know which street you are on: starting at the same speed, you could have started anywhere. That is "unobservable": the speedometer cannot reveal the position. Conversely, with a navigation display that shows only the position, read once a second, you can work out the speed: the difference of two positions divided by one second. The speed is not displayed directly, but it can be inferred from a **series** of readings, so the speed is "observable".

That is the key to observability: a monitor need not measure every quantity directly; it is enough that the readings over a stretch of time (or distance), taken together, determine the state uniquely. Controllability likewise: the knobs need not get there in one go; it is enough that within a few steps they can bring the system into any state.

## The "state" of the beam centre {#controllability-state}

What is the "state" of an orbit? At any place in the accelerator the beam centre is completely described by four numbers: horizontal position $x$, horizontal angle $x'$, vertical position $y$, vertical angle $y'$ (longitudinally there are also energy and arrival time; we leave those aside). Know these four numbers at one place, and what each element downstream does (the transfer matrices of [Linear optics](#linear-optics)), and the whole orbit downstream is fixed.

So "an orbit" is determined by only a handful of numbers: the position and angle at the entrance, plus every "kick" along the way (misplaced quadrupoles, steerers). From here on we look only at the horizontal plane, i.e. the two numbers $x$ and $x'$.

## Can we see it? BPMs and observability {#controllability-observe}

A BPM measures the **position** $x$ of the beam centre where it sits; it cannot measure the angle $x'$. One reading, two unknowns: no solution.

Add a second BPM. Suppose there is a 1 m drift between the two BPMs, the first reads 0.2 mm and the second 0.5 mm. In a drift the beam goes straight, $x_2 = x_1 + L\,x'$, so

$$
x' = \frac{x_2 - x_1}{L} = \frac{0.5 - 0.2}{1\ \mathrm{m}}\ \mathrm{mm} = 0.3\ \mathrm{mrad}.
$$

Two readings, two unknowns: position and angle are both fixed. Observable.

What about noisy readings? With 10 µm of noise on each BPM, the angle carries an error of about $\sqrt 2 \times 10\ \mu\mathrm m / 1\ \mathrm m \approx 14\ \mu\mathrm{rad}$. The closer the two BPMs, the larger the angle error from the same noise; too close, and the angle is visible "in principle" but drowned in noise in practice. Observability is not only "can" or "cannot", but also "how clearly".

There is a worse case. In a focusing channel the beam oscillates back and forth about the centre line (the FODO channel of the [Quadrupole](#quadrupole) chapter); one full oscillation is 360° of **phase advance**. If two BPMs are exactly half an oscillation apart (180° phase advance), the second reading is always minus the first, $x_2 = -x_1$, whatever the angle. The second reading brings no new information, and the angle is completely invisible. So BPMs cannot be placed just anywhere.

## Can we reach it? Steerers and controllability {#controllability-control}

A steerer gives the beam an angular "kick" at its own location; it changes only the angle, not the position at that point. Only after some distance does that angle turn into a position offset.

Suppose we want the beam to have **both** zero position and zero angle at some target point (to send it into the next accelerator section, say). One steerer is one knob; it cannot fix two numbers at once. Two steerers usually can: one looks mostly after the position, the other mostly after the angle, like the hot and cold valves. But if these two steerers are exactly 180° of phase advance apart, they become "both connected to the cold pipe" again: the second one's effect is just minus the first one's, and together they still give only one direction.

You will notice that the story of observability and the story of controllability are almost the same, only in opposite directions: one asks "can the state be inferred from the readings", the other "can the state be produced by the knobs". Control theory calls this **duality**.

## Lab: remove some BPMs or steerers {#controllability-lab}

The lab below uses the same channel as the orbit-correction chapter (6 FODO cells, 12 quadrupoles, a BPM behind each and a steerer on each, quadrupoles misplaced at random by about 0.4 mm), horizontal plane only. You choose how many BPMs and steerers to use (picked evenly along the line); the lab computes the least-squares correction with them (tiny singular values dropped, see [Orbit correction](#orbit-correction-svd)) and reports how much is left at the BPMs used, at all 12 BPM places, and at most along the line.

```avas-lab
controllability
```

With the default 12 BPMs and 12 steerers: before correction the rms orbit at the 12 BPM places is 1.05 mm; after correction it is almost 0 at the BPMs, but the largest offset along the line is still 0.17 mm (the bulges between BPMs seen in the orbit-correction chapter). Now change things:

| BPMs used | Steerers used | Rank | rms at the BPMs used | rms at all 12 BPM places | largest offset along the line | Condition number |
|---|---|---|---|---|---|---|
| 12 | 12 | 11 | 0.000 mm | 0.000 mm | 0.17 mm | 86 |
| 4 | 12 | 4 | 0.000 mm | 0.23 mm | 0.67 mm | 25 |
| 12 | 4 | 4 | 0.36 mm | 0.36 mm | 0.87 mm | 26 |
| 6 | 12 | 6 | 0.000 mm | 0.12 mm | 0.42 mm | 73 |
| 12 | 6 | 6 | 0.074 mm | 0.074 mm | 0.25 mm | 52 |
| 4 | 4 | 4 | 0.000 mm | 19 mm | 58 mm | 2600 |

Three typical cases:

- **Only 4 BPMs (not enough observability)**: 12 steerers have more than enough power to bring these 4 readings to 0, and it "looks corrected"; but at the 8 unused BPM places 0.23 mm remain, and up to 0.67 mm along the line. Part of the orbit is invisible to these 4 BPMs; nobody knows it is there, so of course nobody corrects it.
- **Only 4 steerers (not enough controllability)**: all 12 BPMs see clearly, but 4 knobs can cancel only 4 independent orbit shapes; the rest stays, and 0.36 mm remain at the BPMs. "Visible but out of reach."
- **4 BPMs and 4 steerers**: as many equations as unknowns, and the 4 readings are brought to 0 exactly. Yet the rms at all 12 BPM places is 19 mm, up to 58 mm along the line: more than ten times worse than no correction! The reason lies in the singular values of the next sections: in this choice of positions one combination is almost invisible, and the exact solution asks some steerers for about 0.5 T, a hundred times the maximum of a real steerer (5 mT in the orbit-correction chapter). The lab neither caps the steerers nor lets the beam hit the wall; in a real machine the beam would long be lost.

## Rank: how many truly independent ways there are {#controllability-rank}

The lab's **rank** is "how many truly independent ways of correcting there are". Two valves on the cold pipe: two knobs, but rank 1.

In orbit correction the rank cannot exceed the number of BPMs, nor the number of steerers, and it usually equals the smaller of the two. So:

- What the BPMs have beyond the rank are **reading shapes the steerers cannot produce**: offsets of these shapes nobody can remove (8 = 12 − 4 of them in the "only 4 steerers" case).
- What the steerers have beyond the rank are **steerer combinations the BPMs cannot see**: changing the steerers this way leaves every BPM reading unchanged (8 = 12 − 4 of them in the "only 4 BPMs" case). They may simply be wasted, or quietly create offsets between the BPMs.

Why is the rank of the default 12 × 12 case 11 and not 12? Strictly mathematically, the table has full rank (12); but the 12th singular value is only about one two-millionth of the largest, and the lab treats singular values below one millionth of the largest as 0. That 12th way is the bulge between BPMs: "within reach" mathematically, in practice requiring too much strength to be usable. **What the rank is depends on how small a number counts as 0**, and that depends on the noise and on what the steerers can do.

## Singular values: within reach, but at what effort? {#controllability-singular}

So asking "can it" is not enough; we must also ask "with how much effort". As the [Orbit correction](#orbit-correction-svd) chapter explained, a **singular value** is the "gain" of each independent way of correcting: how much the BPM readings change per tesla of steerer field. With a large singular value a little field has a clear effect; with a small one it takes a large field to have any effect, and the noise in the readings is amplified into large corrections.

The largest singular value divided by the smallest (not counted as 0) is the **condition number**; it says "how many times harder the hardest way is than the easiest". In the lab:

- 12 × 12: condition number 86 (the 12th not counted);
- 4 BPMs, 4 steerers: condition number about 2600, the smallest singular value only 0.0003 mm/mT; that is why the previous section needed 0.5 T.

In practice, "controllable" means: the steerer fields needed to correct the offsets along each way do not exceed what the steerers can give, and the noise is not amplified too much. "Observable" means: every state we need to know can be inferred from the readings, with an error smaller than the offsets we care about.

## Phase advance: where to put BPMs and steerers {#controllability-placement}

As we saw, two BPMs (or two steerers) 180° of phase advance apart count as one. Conversely, 90° apart is best: one BPM sees mainly the position, the other mainly the angle, and they complement each other as fully as possible. A common rule in accelerator design is therefore: **in each plane, one BPM and one steerer every 90° or so of phase advance**, for example one of each next to every quadrupole that focuses in that plane.

The lab channel has a phase advance of about 64° per FODO cell (computed in "going deeper"), with two BPMs and two steerers per cell, even denser than the rule. With only 4 BPMs, three or four quadrupoles lie between neighbouring BPMs used (about 100–130° of phase advance); the phase advance itself is not bad, but every misplaced quadrupole in between kicks the beam again: more happens between the BPMs than 4 readings can determine, so the orbit in between is invisible. The 90° rule is about how two readings are most useful along a stretch without new kicks; having a BPM watching near every place where things can go wrong is a separate requirement.

## Not only the orbit {#controllability-beyond}

The same two questions apply to everything controlled in an accelerator:

- **Beam size (envelope)**: the knobs are quadrupole gradients, the monitors profile monitors that measure the beam's cross-section. Whether a few quadrupoles can set the beam shape right in all three planes, and whether a few profile monitors can reveal the beam's emittance and Twiss parameters, is the controllability and observability of the envelope (see [Matching](#matching)).
- **Energy and phase**: the knobs are cavity amplitudes and phases, the monitors phase probes measuring the arrival time of the bunches.

One case is especially important: if some motion **grows by itself** (is unstable, such as an envelope instability caused by [space charge](#space-charge)) and the knobs cannot reach it, then no control whatsoever can save it; this is an "uncontrollable unstable mode". Conversely, as long as the knobs can reach it, feedback can hold it down; this is called "stabilizable". In a stable focusing channel the orbit only oscillates back and forth and does not grow by itself, so all orbit modes are "stable". The AVAS Research page labels every mode of a period with one of these three classes (see [In AVAS](#controllability-in-avas)).

## Think about it {#controllability-think}

> **Think about it 1**: there are 12 BPMs but only 1 steerer. Can all 12 readings be brought to 0? When?
>
> Answer: in general not: one knob produces only one reading shape (rank 1), and everything cancels only if the error orbit happens to have exactly that shape. In the lab, with 12 BPMs and 1 steerer, 0.84 mm remain at the BPMs.

> **Think about it 2**: two BPMs are 360° of phase advance (one full oscillation) apart. Can they measure the angle?
>
> Answer: no. After 360° of phase advance the state at the second BPM is exactly the same as at the first (in a periodic channel without acceleration); the readings are equal and bring no new information. As bad a position as 180°.

> **Think about it 3**: with only 4 BPMs, the BPMs were corrected to 0. Why call it a "failed correction"?
>
> Answer: because what we really care about is the orbit everywhere along the line, not the BPM readings. Zero at the BPMs only means "the visible part" is 0; the invisible part (up to 0.67 mm along the line in the lab) has not shrunk at all.

> **Think about it 4**: as many equations as unknowns: can we then safely solve exactly?
>
> Answer: not necessarily. With 4 BPMs and 4 steerers the equations can be solved exactly, but the condition number is about 2600, the smallest singular value is tiny, and the steerer fields needed are unrealistically large. Look at the singular values, don't just count.

## Common misconceptions {#controllability-misconceptions}

- **"More knobs, better controllability"**: the number of knobs is not the number of independent directions. Two steerers 180° of phase advance apart count as one direction.
- **"Zero BPM readings mean the orbit is corrected"**: they only mean the part the BPMs can see is 0. With too few or badly placed BPMs the invisible orbit can be larger.
- **"Mathematically controllable is enough"**: full rank only says "possible in principle"; with too small a singular value the effort exceeds what the steerers can give, or the noise is amplified into large errors, which in practice amounts to uncontrollable. The 12th way of the default channel is exactly that.
- **"Controllability and observability are the same thing"**: one is about knobs, the other about monitors. In the lab, only 4 BPMs is a lack of observability, only 4 steerers a lack of controllability, and they look completely different: in the first case the BPMs read 0 and there are offsets elsewhere, in the second the BPMs cannot even be corrected cleanly.
- **"AVAS can connect to an accelerator and control it"**: it cannot. AVAS is a simulation program; controllability and observability are analysed on the linear model, and the response matrix of a real machine has to be measured.

## Going deeper: from two readings to the observability matrix {#controllability-deep-observe}

The main text explained observability as "two readings solve for two unknowns". This section turns that into the textbook form step by step: the first five steps use only simultaneous linear equations, determinants and matrix multiplication (the "going deeper" part of the [Drift](#drift-deep-matrix) chapter showed how two lines of formulas become a matrix); the last paragraph is for readers who know linear algebra.

### Step 1: one period "converts" the state once

Write the state of the beam centre at the entrance of a period as a column $\boldsymbol x_n = (x, x')$, with $n$ the number of the period. Linear optics tells us that after one period

$$
\boldsymbol x_{n+1} = A\,\boldsymbol x_n ,
$$

with $A$ the 2 × 2 transfer matrix of the period. Its determinant is 1 (without acceleration), and half its trace is the cosine of the phase advance $\mu$: $\cos\mu = \tfrac12(A_{11} + A_{22})$ (see [Linear optics](#linear-optics-phase-advance)).

### Step 2: a BPM reading is a "combination" of the state

The position read by a BPM in the period is a linear combination of the entrance position and angle (there is some transport in front of the BPM):

$$
y_n = c_1\,x + c_2\,x' = C\,\boldsymbol x_n ,
$$

with $C = (c_1, c_2)$ a row of two numbers. A BPM right at the entrance has $C = (1, 0)$; one after a drift of length $L$ has $C = (1, L)$.

### Step 3: readings of two periods: simultaneous equations

The reading of one period is one equation for two unknowns: no solution. Add the next period: $y_{n+1} = C\boldsymbol x_{n+1} = CA\,\boldsymbol x_n$. Together they form the system

$$
\begin{pmatrix} y_n \\ y_{n+1} \end{pmatrix} = \begin{pmatrix} C \\ CA \end{pmatrix} \boldsymbol x_n .
$$

The 2 × 2 matrix on the right is the **observability matrix**. School algebra tells us that two linear equations in two unknowns have a unique solution if and only if the determinant of the coefficients is not zero. So: **if the determinant of the observability matrix is not zero, the entrance state can be inferred from the readings of two periods.**

### Step 4: why 180° of phase advance is blind

Any period with 180° of phase advance has the transfer matrix $A = -I$ ($I$ the identity matrix; this follows from the general form of a periodic matrix $A = I\cos\mu + J\sin\mu$ at $\mu = 180°$, where $J$ is made of the Twiss parameters, see [Linear optics](#linear-optics-periodic)). Then $CA = -C$, the two rows of the observability matrix are proportional, and the determinant is 0: the second reading is just minus the first.

And for a general phase advance? In "normalised" coordinates the periodic matrix is a rotation by the angle $\mu$, and a BPM reads only the first coordinate, $C = (1, 0)$:

$$
\det\begin{pmatrix} C \\ CA \end{pmatrix} = \det\begin{pmatrix} 1 & 0 \\ \cos\mu & \sin\mu \end{pmatrix} = \sin\mu .
$$

It is largest at $\mu = 90°$ and zero at 0° and 180°: the origin of "90° best, 180° worst" in the main text.

### Step 5: the numbers of the example channel

One period of the lab channel (starting at the steerer in front of a focusing quadrupole, 0.7 m long; units m and rad):

$$
A = \begin{pmatrix} -1.177 & 0.959\ \mathrm m \\ -3.569\ \mathrm{m^{-1}} & 2.059 \end{pmatrix}, \qquad \det A = 1, \qquad \cos\mu = \frac{-1.177 + 2.059}{2} = 0.441 ,
$$

a phase advance $\mu = 63.8°$. The first BPM of the period has $C = (0.678,\ 0.136\ \mathrm m)$, so $CA = (-1.284,\ 0.931\ \mathrm m)$, and the determinant of the observability matrix is $0.678 \times 0.931 - 0.136 \times (-1.284) = 0.806\ \mathrm m$, not zero: one BPM per period is observable.

### Step 6: how precisely, with noise

With 10 µm of noise on each reading, the $x$ and $x'$ obtained by solving the equations carry errors too. Combining the readings of several periods with [least squares](#orbit-correction-deep-svd) makes the errors smaller. For the example channel (AVAS's `observability` computes the smallest achievable error from the noise):

| Readings used | error of $x$ | error of $x'$ |
|---|---|---|
| 1 BPM per period, 2 periods | 11.7 µm | 18 µrad |
| 1 BPM per period, 6 periods | 6.1 µm | 12 µrad |
| 2 BPMs per period, 2 periods | 9.8 µm | 16 µrad |
| 2 BPMs per period, 6 periods | 4.9 µm | 9.6 µrad |

More readings give better estimates, but "observable" already holds in the first row.

### For readers who know linear algebra

For an $n$-dimensional state the observability matrix is $\mathcal O = [C;\ CA;\ \dots;\ CA^{n-1}]$, and the system is observable if and only if $\operatorname{rank}\mathcal O = n$ (further rows $CA^k$ add nothing by the Cayley–Hamilton theorem). With noise covariance $R_n$, the inverse of the observability Gramian over $N$ periods, $W_o = \sum_{k<N} (A^k)\tp C\tp R_n^{-1} C A^k$, is the Cramér–Rao bound on the covariance of any unbiased estimate; the table of step 6 is $\sqrt{\operatorname{diag} W_o^{-1}}$ (`state_error` of `avas.dynamics.control.observability`). In normalised coordinates $A$ is the rotation $R(\mu)$ and $\det\mathcal O = \sin\mu$; the system is unobservable when $\mu$ is a multiple of 180°.

## Going deeper: the (A, B, C) of a period and the textbook definitions {#controllability-deep-abc}

The previous section had only monitors. This section adds the knobs, writes down the standard form of control-theory textbooks, and explains what AVAS's `avas/dynamics/systems.py` and `control.py` compute.

### Step 1: the state-space form

Put the changes of all steerer fields of a period in a column $\boldsymbol u_n$ and all BPM readings in a column $\boldsymbol y_n$. Since every step is "matrix times state plus the steerers' kicks",

$$
\boldsymbol x_{n+1} = A\,\boldsymbol x_n + B\,\boldsymbol u_n , \qquad \boldsymbol y_n = C\,\boldsymbol x_n + D\,\boldsymbol u_n .
$$

One by one: $\boldsymbol x_n$ is the state at the entrance of period $n$ ($x$ and $x'$ for the horizontal plane alone, four numbers for both planes); $A$ is the transfer matrix of the period; each column of $B$ is "how much this steerer changes the state at the period's exit per unit field"; each row of $C$ is "which combination of the entrance state this BPM reads"; $D$ is the direct effect on a BPM of steerers upstream of it within the same period. AVAS's `orbit_system` returns exactly $A$, $B$, $C$, $D$ in this form: $A$ from the period transfer matrix of the linear model (normalised by $\sqrt{\beta\gamma}$ with acceleration), $B$ and $D$ from central differences of the model with each knob.

The columns of steerers SF2 (at the period entrance) and SD2 in the example period, per mT:

$$
B = \begin{pmatrix} -0.539\ \mathrm{mm} & -0.177\ \mathrm{mm} \\ -1.186\ \mathrm{mrad} & -0.592\ \mathrm{mrad} \end{pmatrix} \text{ (per mT)}.
$$

### Step 2: where we can get in a few periods

Start from $\boldsymbol x_0 = 0$ and use only the first steerer (call the first column of $B$ $\boldsymbol b$; it is set once per period, with changes $u_0, u_1, \dots$):

$$
\boldsymbol x_1 = \boldsymbol b\,u_0, \qquad \boldsymbol x_2 = A\boldsymbol b\,u_0 + \boldsymbol b\,u_1 .
$$

After two periods, the reachable states are all combinations of the two directions $\boldsymbol b$ and $A\boldsymbol b$. As long as these two columns are not parallel, i.e. the determinant $\det(\boldsymbol b,\ A\boldsymbol b) \ne 0$, every $(x, x')$ can be reached. The matrix $(\boldsymbol b,\ A\boldsymbol b)$ is the **controllability matrix**; it is exactly the "transpose" of the previous section's observability matrix: duality.

Example: $A\boldsymbol b = (-0.503\ \mathrm{mm},\ -0.518\ \mathrm{mrad})$, $\det(\boldsymbol b,\ A\boldsymbol b) = (-0.539)(-0.518) - (-0.503)(-1.186) = -0.318\ \mathrm{mm \cdot mrad}$, not zero: one steerer per period is controllable. With both steerers of a period together, $\det B = 0.110\ \mathrm{mm\cdot mrad} \ne 0$, and every state can be reached within one period.

### Step 3: why A²b need not be checked

The third period adds $A^2\boldsymbol b$. Could it bring a new direction? For any 2 × 2 matrix, multiplying out shows

$$
A^2 = (A_{11} + A_{22})\,A - (\det A)\,I ,
$$

so $A^2\boldsymbol b$ is only a combination of $A\boldsymbol b$ and $\boldsymbol b$, with no new direction. In general, $n$ states need checking only up to $A^{n-1}B$. The textbook **controllability criterion** (Kalman's criterion) is:

$$
\operatorname{rank}\,(B,\ AB,\ \dots,\ A^{n-1}B) = n .
$$

The **observability criterion** is its dual: the rank of $(C;\ CA;\ \dots;\ CA^{n-1})$ is $n$. "Rank" is the number of truly independent columns (or rows): the main text's "how many truly independent ways there are".

### Step 4: how much effort: the Gramian and singular values

The criterion only answers "can it". "How much effort" is answered by the **Gramian** $W = \sum_{k} A^k B B\tp (A^k)\tp$: the square roots of its eigenvalues are "how far one unit of knob pushes" along each direction. In the example, with one steerer and two periods, these two numbers are 1.47 and 0.215 (in combinations of mm and mrad per mT), a ratio of 6.8: the hardest direction to reach needs about 7 times more field. AVAS's `controllability` returns the rank, these singular values and the condition number.

The orbit-correction lab of the main text is a different, simpler case: the beam passes only once, there is no "period $n$", and readings and knobs are related simply by the response matrix $\boldsymbol y = R\,\boldsymbol u$. Then the rank, singular values and condition number of $R$ answer controllability and observability directly: if the rank of $R$ is below the number of BPMs, there are reading shapes the steerers cannot produce; if it is below the number of steerers, there are steerer combinations the BPMs cannot see. AVAS's `static` lists both kinds (`uncorrectable`, `invisible`), and the statements of the Research page's "Response & controllability" tab come from it.

### Step 5: modes and three classes

Compute the eigenvalues of $A$: the example period has a pair $\lambda = 0.441 \pm 0.897i$, modulus 1, argument ±63.8°, which is the phase advance. Modulus 1 means this motion neither grows nor decays (a stable oscillation). If an eigenvalue had a modulus above 1, this motion would grow from period to period, and the question would be: can the knobs reach it? The test is whether $(A - \lambda I,\ B)$ has rank $n$ (the PBH test). Each mode thus falls into one of three classes:

- **stable**: modulus not above 1 (AVAS allows a tolerance of $10^{-3}$, because normalised accelerating periods are only approximately symplectic);
- **stabilizable**: unstable, but the knobs can reach it, and feedback can hold it down;
- **uncontrollable**: unstable and out of the knobs' reach; no control can save it.

Both modes of the example period are "stable", and controllable and observable.

### For readers who know linear algebra

Kalman's rank criterion, the Cayley–Hamilton theorem and the PBH test $\operatorname{rank}[A - \lambda I,\ B] = n$ are equivalent characterisations of controllability; observability follows from the dual pair $(A\tp, C\tp)$. The finite-horizon controllability Gramian $W_c = \sum_{k<N} A^k B B\tp (A^k)\tp$ gives the cost $\boldsymbol x\tp W_c^{-1}\boldsymbol x$ of reaching $\boldsymbol x$ with minimum energy $\boldsymbol u\tp\boldsymbol u$. Requiring controllability only of the unstable modes is stabilizability, whose dual is detectability; the Kalman decomposition splits the state space into four blocks (controllable and observable, controllable but unobservable, …), and the transfer function sees only the controllable and observable part. When stabilizable, state feedback $u = K\boldsymbol x$ can place the closed-loop poles of the controllable modes at will (`closed_loop`, `lqr`), and an observer or Kalman filter gives a converging state estimate when detectable. `envelope_system` gives the same $(A, B, C, D)$ form for the second moments (envelope): the state is the upper triangle of Σ around the matched beam, $A$ the Jacobian of the envelope map, the knobs usually quadrupole gradients, the monitors rms sizes.

## In AVAS {#controllability-in-avas}

### Monitors and knobs

AVAS stores monitors and knobs in `<project>/Research/diagnostics.json` (study.json format, the lattice is not changed). "From the lattice" takes the lattice's position diagnostics `diag_position` (BPMs), size diagnostics `diag_size` and all steerers; each monitor can have a resolution (rms noise), and each knob can be one lattice parameter of an element (for example a steerer's By, a quadrupole's gradient) with a difference step, or an element offset (dx, dy, roll; linear model only).

```avas-params
diag_position diag_size steerer
```

### Research page: the "Response & controllability" tab

- **Response of the linear model**: computes the response matrix of every reading to every knob with the linear model, shows it as a heat map and plots the singular values (log scale). Below it lists: the rank (of how many knobs) and the condition number; how many reading patterns no knob can correct; how many knob combinations no monitor can see; and each knob's error caused by monitor noise amplified through the correction.
- **Response from the engine**: one base run plus one engine run per knob (fixed random seed, adjustable number of macro-particles, 200 by default; with the original kernel or the compatibility mode the page notes the fixed-seed defect, which hardly affects centroid responses), lattice parameters only. It is a study run, shown on the Run page and excluding ordinary runs meanwhile.
- **Modes of one period**: the run lattice must mark periods with `lattice n 1` … `lattice_end` (on the Lattice page). Pick a period and "Analyse": AVAS computes the $(A, B, C)$ of the beam centre over that period with `orbit_system` (using only knobs inside the period; it says so if the period has none), gives the controllable rank and observable rank (out of $n$ states), and lists for each mode $|\lambda|$, phase, class (stable / stabilizable / uncontrollable), whether controllable and whether observable, plus the smallest error of the entrance state inferred from the readings, given the monitors' resolutions.

These calculations use only the linear model, take no run lock and write no input files.

### Linear model and engine

The orbit response matrix has been checked against the engine: elements passing only drifts and the HWR cavity agree to within 0.06 % of the largest element, those passing through the example `sol` field map differ by 0.1–2.7 % (because of the map itself); fix the random seed when comparing (details in [Orbit correction](#orbit-correction-in-avas)). For steerers and offsets the linear model's response is exact (the model is linear in them); for other parameters such as gradients it is a central difference around the operating point.

### In Python

```text
from avas.model import LinearModel
from avas.dynamics import (Diagnostics, controllability, modes, observability,
                           orbit_system, response, static, steerers)

model = LinearModel.from_project("my_project")
diag = Diagnostics.from_lattice(model)
r = response(model, steerers(model), diag)
s = static(r.matrix)                       # rank, singular values, condition, uncorrectable, invisible, knob_noise

knobs = [k for k in steerers(model, "x") if k.target in ("SF2", "SD2")]   # knobs inside the period (use your lattice's names)
sysd = orbit_system(model, 2, knobs, diag, planes="x")                   # A, B, C, D of period 2
controllability(sysd["A"], sysd["B"])["rank"]
observability(sysd["A"], sysd["C"], noise=[10e-6] * len(sysd["C"]))["state_error"]
modes(sysd["A"], sysd["B"], sysd["C"])     # per mode: eigenvalue, phase advance, class, controllable, observable
```

The period of `orbit_system` can be the number of a `lattice … lattice_end` group of the lattice or a z range `(z0, z1)`; the envelope version is `envelope_system`. This chapter's lab is `controllability` in `avas/learn/control.py`.

### What AVAS does not do

AVAS is a simulation program and virtual accelerator, not a control system: it does not connect to real BPMs and power supplies, and controllability and observability are analysed on the model. A real machine needs measured response matrices; the model tells you where to put BPMs and steerers and how strong they must be, the measurement tells you how the machine really behaves.

## Further reading {#controllability-reading}

- K. J. Åström, R. M. Murray, *Feedback Systems: An Introduction for Scientists and Engineers*, Princeton University Press (2008): an introduction to reachability, observability and state feedback.
- R. E. Kalman, "On the general theory of control systems", *Proceedings of the First IFAC Congress*, Moscow (1960): the original paper on controllability and observability.
- M. G. Minty, F. Zimmermann, *Measurement and Control of Charged Particle Beams*, Springer (2003): placement of BPMs and steerers, response matrices.
- Related chapters: [Orbit correction](#orbit-correction), [Feedback](#feedback), [Linear optics](#linear-optics); full bibliography in [References](#references).
