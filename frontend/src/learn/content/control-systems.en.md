# How a real facility is controlled

The previous chapters covered orbit correction, feedback and controllability: the algorithms themselves are compact, a few lines of formulas. On a real accelerator, however, the sentence "raise steerer 3 by 1 mT" passes through many hands before anything moves. An operator clicks in the control room, the command crosses the network to a small computer in an equipment room a few hundred metres away, and that computer makes the magnet power supply change its current a little. The supply reports "done", the BPMs see the beam move, and the new reading travels all the way back to the screen. This happens thousands of times a second, 24 hours a day, and it must also guarantee that the beam is cut within an instant when something fails and that nobody is ever exposed to radiation.

What organises all this is the accelerator's **control system**. This chapter describes its layers and what each one does, and finally where a simulation code such as AVAS (a "virtual accelerator") fits. Everything here is general practice; real facilities differ a lot in their designs, and nothing below describes a particular machine.

An analogy first. Think of the traffic of a large city. Every junction has traffic lights and induction loops under the road (**hardware**). A small cabinet next to the junction switches the lights by a fixed program and lengthens the green phase by itself when there are many cars (**front-end controller**). All junctions are networked to a traffic centre whose big screens show the state of each junction (**control-system software** and **operator interfaces**). All lights switch by the same clock, which makes a "green wave" possible (**timing system**). Ambulances and fire engines may turn every junction red at once (**protection system**). The centre keeps each junction's history, so that afterwards one can ask "why was there a jam at 3 a.m.?" (**archiving**). Traffic engineers use those data and computer models to redesign the signal plans (**high-level applications** and **models**). An accelerator's control system matches this almost item by item.

## Why a "control system" is needed {#control-systems-why}

A linac has far more to look after than magnets:

- **Magnet power supplies**: every quadrupole, steerer and dipole has a supply whose current is set, read back and watched for faults.
- **RF systems**: every cavity has a power source (amplifier), low-level RF control (holding the amplitude and phase of the cavity field, see [Feedback](#feedback)) and a tuner; superconducting cavities also have cryogenics.
- **Vacuum**: pumps, gauges, valves. When the vacuum degrades, valves close automatically to protect what lies up- and downstream.
- **Beam diagnostics**: BPMs, beam current, beam loss monitors, profile measurements (see [Beam diagnostics](#diagnostics)).
- **Utilities**: cooling water, electricity, air conditioning, cryogenic plants.

Each device has several quantities to read and write; together these are thousands, and in a large facility hundreds of thousands or more. While the accelerator runs there is radiation in the tunnel and nobody may enter, so all these quantities can only be read and written remotely. Someone must also know at all times where something is wrong, some equipment must react by itself within milliseconds or even microseconds, and everything must be recorded somewhere. That is the problem a control system solves.

## From the magnet to the screen: layer by layer {#control-systems-layers}

Almost every accelerator control system is organised in layers. From the bottom up:

| Layer | What it is | Typical reaction time |
|---|---|---|
| Devices | The magnet power supplies, RF amplifiers, BPM electrodes, vacuum gauges, valves … themselves | — |
| Front-end (input/output) controllers | Embedded computers, programmable logic controllers (PLCs) and FPGA boards in racks next to the devices; they turn electrical signals into numbers and run fast local logic and loops | microseconds to milliseconds |
| Network | Connects the front ends to the layers above; plus a dedicated timing network and protection signal lines | — |
| Control-system software (middleware) | Gives every quantity a name so that any program can read, write and subscribe to it by name (e.g. EPICS, Tango) | milliseconds |
| Services | Archiving, alarms, snapshots of the settings, electronic logbook | seconds |
| Operator interfaces and high-level applications | The panels on the control-room screens; physics programs such as orbit correction and phase scans; the online model | seconds to minutes |

Two principles run through all layers:

- **The faster the task, the lower it lives.** Whatever must be finished within microseconds (holding a cavity field, cutting the beam after a loss) must happen in electronics next to the device; it cannot wait for a command to travel round the network. What is done once every few seconds (correcting the orbit of the whole line) is fine on an ordinary computer at the top. The section "Fast loops low, slow loops high" below shows why with a lab.
- **Lower layers do not depend on higher ones.** If the control-room computer crashes, the front ends keep working with the last settings and the protection system can still cut the beam. The top only "gives commands and looks at results"; it is not what keeps the devices alive.

## A name for every quantity: EPICS and Tango {#control-systems-names}

The central idea of control-system software is simple: **give every quantity a unique name**. For example (illustrative; every facility has its own naming convention)

```text
LINAC:QUAD03:CURRENT_SET     current set point of the power supply of quadrupole 3
LINAC:QUAD03:CURRENT_READ    its current read-back
LINAC:BPM05:X                horizontal position reading of BPM 5
```

Any program, whether a control-room panel, the orbit-correction program or a Python script, can do three things knowing only the name: **read** (what is it now?), **write** (set it to this) and **subscribe** (tell me whenever it changes). Subscribing matters: if a thousand screens asked for every quantity every second, the network would drown in questions; with subscriptions a message goes out only when a value changes.

Two open-source systems are widely used in the accelerator world:

- **EPICS** (Experimental Physics and Industrial Control System): started in 1989 as a collaboration between Los Alamos (building on its Ground Test Accelerator control system) and Argonne (for the Advanced Photon Source) national laboratories in the USA, now maintained by an international community. Each quantity is a **process variable** (PV); the program running in a front end is an **IOC** (Input/Output Controller), which keeps a database of "records", each handling the reading and writing, conversion and alarm checks of one quantity; clients access them by name with the Channel Access protocol or the newer PV Access. Every value carries a time stamp and an alarm state.
- **Tango**: originating at the European Synchrotron Radiation Facility (ESRF), object-oriented. Each device is served by a **device server**; a device has **attributes** (quantities to read or write, such as a current), **commands** (such as "on" or "reset") and configuration properties. Many European synchrotron light sources use it.

Some facilities use commercial SCADA systems or frameworks of their own. Whichever it is, the idea is the same: the details of the hardware are hidden in the front ends, and the upper layers see only named quantities with units and time stamps.

## The timing system: keeping thousands of devices in step {#control-systems-timing}

Many linacs run in **pulses**: the ion source sends a beam pulse every so often, the RF power must be switched on a little earlier so that the cavity fields can build up, the BPMs and other diagnostics must sample exactly when the beam arrives, and certain fast deflecting magnets must open in the gap between two bunches. These actions are spread over a tunnel hundreds of metres long and must be aligned to one clock with nanosecond precision or better.

That is the job of the **timing system**. A common design: a master timing generator broadcasts "events" (such as "a beam pulse is about to start") over a fibre network, and receivers everywhere produce trigger signals after their own preset delays. The same event stream can also number every pulse, so that the data different devices take on the same pulse can be matched by that number: pulse-to-pulse feedback needs to know "which pulse this reading belongs to". There are also schemes that distribute precise time over Ethernet, for example White Rabbit, started at CERN.

How precise? Take the example beam's RF: one period of 162.5 MHz is

$$
T = \frac{1}{162.5\ \mathrm{MHz}} = 6.15\ \mathrm{ns},
$$

so 1° of cavity phase is $6.15\ \mathrm{ns}/360 \approx 17\ \mathrm{ps}$ (picoseconds, $10^{-12}$ s). In 17 ps light travels only about 5 mm. Distributing the RF phase reference (the RF signal that all cavities "set their watches" by) is therefore far more demanding than ordinary triggers and is usually done by a dedicated reference distribution system. A "going deeper" section works out how troublesome temperature changes of long cables are.

## Fast loops low, slow loops high {#control-systems-loops}

The [Feedback](#feedback) chapter explained that feedback keeps going round "measure — compute — correct". The time one round takes (the **delay**) decides how strong the feedback can be. The lab below is pulse-to-pulse orbit feedback on the channel of the [Orbit correction](#orbit-correction) chapter: every pulse the BPMs are read and a fraction $g$ (the gain) of the measured error is corrected; "delay" is how many pulses late the reading is used.

```avas-lab
feedback
```

With the defaults, gain 0.5 and a delay of 1 pulse, the rms orbit at the BPMs drops from 0.084 mm without feedback to 0.016 mm. Look at "largest stable gain for this delay": about 2 with no delay, about 1 with 1 pulse, about 0.62 with 2 and about 0.45 with 3. Set the delay to 3 and keep the gain at 0.5, and the loop becomes unstable: every correction overshoots and the orbit grows (rms 0.60 mm in the second half, worse than without feedback).

That is why "the faster the task, the lower it lives": the longer the delay, the smaller the gain must be, and the slower and weaker the feedback. A command that leaves a control-room program, passes the network, the middleware and the front end, and whose reading comes back again, takes a few to a few tens of milliseconds per round. For a slow orbit feedback that corrects once a second this is no problem at all; for low-level RF feedback, which must hold a cavity field within microseconds, it is far too slow. That loop therefore always lives in dedicated electronics next to the cavity (often an FPGA), and the control system only gives it set points ("this amplitude, this phase") and displays its state.

## Machine protection and personnel safety {#control-systems-protection}

Two special systems in the control system aim not at "a better beam" but at "nothing going wrong".

The **personnel safety system** (often called the personnel protection system or safety interlock) makes sure that no beam can be produced while anyone is in a radiation area. Tunnel doors are interlocked, so opening one cuts the beam; before start-up somebody "searches" the area along a prescribed route to confirm nobody is left inside; radiation monitors above their limits also cut the beam. This system is usually **separate** from the ordinary control system, built with hard wiring or certified safety controllers, designed and reviewed to functional-safety standards, and the software of the ordinary control system has no means to bypass it.

The **machine protection system** keeps the equipment from being damaged by the beam. A beam loss monitor seeing a sudden rise in losses, a magnet power supply tripping, a vacuum valve closing, a cavity losing its field … any one of these signals must cut the beam within a very short time (for instance by stopping the extraction from the ion source, or by having a deflecting element in the low-energy section stop the beam). It has two features:

- **Fast**: high-power facilities usually require the beam to be cut within microseconds (a few to a few tens of microseconds). A "going deeper" section works out why.
- **Fail-safe**: every device continuously sends a signal "all well, beam permitted"; when that signal disappears (whether because of a real fault or because a cable was pulled) it counts as a fault.

How much power does the example beam carry? Each proton has 1.527 MeV and the current is 0.184 mA. Exactly as "power = voltage × current" in electricity:

$$
P = 1.527\ \mathrm{MV} \times 0.184\ \mathrm{mA} = 281\ \mathrm{W}.
$$

(This is the average power from the average current.) 281 W is already like an electric heater, enough to burn a small piece of metal when concentrated on it; high-power proton linacs reach beam powers of the order of megawatts, thousands of times more.

## Archiving, alarms, snapshots and logbooks {#control-systems-archive}

The control system offers several "services" the operators use every day:

- **Archiving**: important quantities are recorded over time and kept for months or years. Afterwards one can answer "why was the beam lost at 3:12 last night?" or "how far has this magnet's current drifted in a month?".
- **Alarms**: each quantity can have upper and lower limits; when it leaves them, it appears on the alarm panel by severity (such as "minor" or "major") and the operator acknowledges each entry. One fault often sets off a cascade of alarms (an "alarm flood"), so alarm systems group and rank them to keep the real cause from being buried.
- **Snapshots (save and restore)**: all set points are saved as a "snapshot" that can later be restored with one click. Changing operating modes and restarting after maintenance rely on it.
- **Post-mortem data**: when the protection system cuts the beam, fast acquisition devices everywhere freeze a short stretch of data before and after the cut, so that one can find out what failed first.
- **Electronic logbook**: operators record what they did and saw, which complements the archived data.

## High-level physics applications {#control-systems-hla}

At the top are **high-level applications**: programs that use physics to organise many readings and set points. The algorithms of the previous chapters exist on real machines in this form:

- **Orbit correction**: read all BPMs, compute the steerer changes from the response matrix, write them to the power supplies (see [Orbit correction](#orbit-correction)). The response matrix is often measured directly on the machine: change one steerer at a time and record how the BPMs move.
- **RF phase scans**: scan a cavity's phase, convert the bunch arrival time measured by downstream BPMs (time of flight) into energy, compare with the model and find the phase and amplitude the cavity should have.
- **Emittance measurements**, **beam-based alignment** (finding the offset between a BPM's zero and a quadrupole's magnetic centre), **matching**, **optimisation**.

These programs have one thing in common: they need a **model** that knows "if I turn this knob, the beam does that". Online, the model must agree with the machine: read the actual current of every magnet, convert it to a gradient with the magnet's **excitation curve** (the measured current–field relation), and give that to the model. Such a model is called an **online model**. High-level applications can be written in many languages; many facilities use Python scripts, and there are dedicated frameworks, for example Open XAL, an open-source Java framework that originated at the Spallation Neutron Source in the USA; many synchrotron light sources use a MATLAB-based middle layer. In recent years some facilities have also tried machine learning and Bayesian optimisation for tuning the beam.

## Models and "virtual accelerators": where AVAS fits {#control-systems-virtual}

A **virtual accelerator** usually means something that looks to the upper layers just like the real machine (quantities with the same names that can be read and written) but has a simulation code behind it instead of hardware. Connecting high-level applications to a virtual accelerator lets one debug programs, train operators and rehearse new operating schemes without beam. The term "digital twin" is also used for a model kept continuously in step with the real machine.

AVAS is the "Advanced Virtual Accelerator Software", but it is **not a control system**: it connects to no hardware, has no EPICS or Tango interface, neither reads nor writes machine settings, and does no real-time control. Its place in this picture is the **model** layer:

- In design: checking with the multi-particle engine and the linear model how the beam travels through a lattice and how large errors may be (see [Errors and correction](#errors)).
- In control studies: the "response and controllability" and "feedback" tabs of the research page do on the model the calculations a high-level application does: the response matrix and its singular values, which orbits can be corrected and seen (see [Controllability and observability](#controllability)), and simulated pulse-to-pulse feedback. The linear model's orbit response matrix has been compared with the engine: through drifts and HWR cavities only, the two differ by no more than 0.06 %.
- To use AVAS as the online model of a facility or as the computing core of a virtual accelerator, more software has to be written: read the set points from the control system, convert them into lattice parameters, call AVAS, and hand the results to the high-level applications. AVAS itself does not contain that part.

A model never agrees perfectly with the machine: excitation curves have errors, field maps are noisy (see [Field maps](#field-maps)), elements are not exactly where they should be. On real machines the model is therefore often used as a "good starting point" and corrected with measurements, for instance by replacing the model's response matrix with a measured one (see [Orbit correction](#orbit-correction-practice)).

## Think about it {#control-systems-think}

> **Think about it 1**: The control-room computer suddenly crashes. What happens to the running beam?
>
> Answer: usually nothing right away. The front ends keep working with the last settings, the fast feedback loops in the lower layers carry on, and the protection system does not depend on the top either. One just cannot change settings or see the screens for a while. That is the point of the layered design.

> **Think about it 2**: Why can't low-level RF feedback be a Python program in the control room?
>
> Answer: the delay is too long. It must suppress disturbances of hundreds to thousands of hertz, which needs a delay of the order of microseconds; a command going round the network and the software takes milliseconds. The longer the delay, the smaller the largest stable gain, as the feedback lab in this chapter shows.

> **Think about it 3**: Why does the machine protection system treat "the signal is gone" as a fault?
>
> Answer: if it cut the beam only on receiving "fault", then a broken cable or a receiver without power would keep a real fault from getting through and the beam would keep going. Requiring devices to keep proving "all well" instead makes every accident stop the system on the safe side.

> **Think about it 4**: The example beam's energy doubles at the same current. What is its power?
>
> Answer: power = energy (in MeV, i.e. a voltage in MV) × current, so it doubles to about 562 W.

## Common misconceptions {#control-systems-misconceptions}

- **"The control system is the software in the control room"**: the software on the screens is only the top layer. What keeps the devices working, the beam stable and the machine protected during faults lives mostly in the lower layers: the front ends, FPGAs and hard wiring.
- **"Protection is one function of the control system"**: the personnel safety system is usually independent of the ordinary control system, with its own hardware and review process; the machine protection system also has its own fast paths that do not go through the ordinary software.
- **"With an exact model, no measurement is needed"**: a model is only a starting point. The real machine's response has to be measured or calibrated against measurements, and the environment keeps changing, which is why feedback is needed.
- **"AVAS is called a virtual accelerator, so it can control an accelerator"**: AVAS is a simulation code with no interface to hardware or control systems. It can be the computing core of an online model, but the connection has to be developed separately.
- **"The more precise the timing the better; every signal needs picoseconds"**: requirements differ enormously. A command to a vacuum valve can be milliseconds late; BPM sampling triggers need nanoseconds; only the RF phase reference needs picoseconds. Grading the requirements is what makes the system buildable and maintainable.

## Going deeper: time, phase and cables {#control-systems-deep-timing}

The main text said the RF phase reference is demanding. This section works out step by step where the difficulty lies, using only "distance = speed × time".

### Step 1: phase into time

A signal of frequency $f$ has a period $T = 1/f$, and one full turn is 360°. A phase difference $\Delta\phi$ (in degrees) is therefore a time difference

$$
\Delta t = \frac{\Delta\phi}{360°}\,T = \frac{\Delta\phi}{360°\,f}.
$$

For the example beam, $f = 162.5\ \mathrm{MHz}$: $T = 6.15\ \mathrm{ns}$, and $\Delta\phi = 1°$ is $\Delta t = 17.1\ \mathrm{ps}$.

### Step 2: how fast signals travel in a cable

The speed of light in a fibre is the vacuum speed divided by the refractive index $n$. For ordinary fibre $n \approx 1.47$, so

$$
v = \frac{3.0\times 10^{8}\ \mathrm{m/s}}{1.47} \approx 2.0\times 10^{8}\ \mathrm{m/s},
$$

and each metre of fibre delays a signal by about $1/v \approx 4.9\ \mathrm{ns}$. A 1 km fibre delays it by about $4.9\ \mathrm{\mu s}$.

The delay itself does not matter: as long as it stays constant it can be subtracted at the receiving end (which is exactly what the "preset delay" of the timing system does). What matters is how much it **changes**.

### Step 3: how much change is allowed

To keep the phase at the end of 1 km of fibre within 1°, the delay must change by less than 17 ps, that is by a fraction

$$
\frac{17\ \mathrm{ps}}{4.9\ \mathrm{\mu s}} \approx 3.5\times 10^{-6}
$$

of the total, 3.5 parts per million. But the delay of ordinary fibre changes with temperature by about 7–8 parts per million per degree (about 40 ps per kilometre per degree for standard single-mode fibre). In other words, a one-degree temperature change in the tunnel can make the phase at the end drift by about two degrees.

### Step 4: what to do

The remedy is the same as feedback: measure how the delay changes and compensate. A common way is to send the signal to the far end and reflect it back, measure the change of the round-trip time, take half of it as the change of the one-way delay, and adjust the sending end the opposite way. Two kinds of error must be told apart here: **drift** is slow change (temperature, ageing) that can be measured and compensated; **jitter** is fast random fluctuation that can only be reduced by better components and filtering.

### For readers who know signal processing

Jitter is usually described by the phase-noise spectrum $\mathcal L(f)$; the integrated jitter $\sigma_t = \frac{1}{2\pi f_0}\sqrt{2\int \mathcal L(f)\,\dd f}$ depends on the integration band, and the band that matters for an accelerator is set by the bandwidths of the beam and of the feedback loops. An actively compensated fibre link is a phase-locked loop of finite bandwidth and removes only drifts slower than its loop bandwidth.

## Going deeper: loop delay and the largest gain {#control-systems-deep-loop}

The lab in the main text showed that the longer the delay, the smaller the largest stable gain. This section explains it with a school quadratic equation and then translates it into the language of continuously running RF feedback.

### Step 1: one correction per pulse

Let the error of one orbit mode be $e_k$ at pulse $k$, and let each correction remove $g$ times the measured error. Using the reading of the same pulse (no delay),

$$
e_{k+1} = e_k - g\,e_k = (1-g)\,e_k .
$$

Every pulse multiplies the error by $(1-g)$. It shrinks only if $|1-g| < 1$, so $0 < g < 2$. $g = 1$ removes it in one step; $g = 1.5$ overshoots by half each time, alternating in sign but shrinking; $g = 2.5$ overshoots more each time and grows.

### Step 2: the reading is one pulse late

With a delay of one pulse the correction uses the previous pulse's reading:

$$
e_{k+1} = e_k - g\,e_{k-1}.
$$

Try a solution that "multiplies by the same number $z$ every step", $e_k = z^k$; substitute and divide both sides by $z^{k-1}$:

$$
z^2 - z + g = 0 .
$$

This is a quadratic equation. For $g > 1/4$ the discriminant $1 - 4g$ is negative, and the two roots are a complex-conjugate pair of equal size; by Vieta's formulas their product is $g$, so each root has size $|z| = \sqrt g$. For the error to decay we need $|z| < 1$, i.e. $g < 1$. In the lab, "largest stable gain" for a delay of 1 shows 1, exactly this result.

### Step 3: longer delays

With a delay of $d$ pulses the equation becomes $z^{d+1} - z^{d} + g = 0$. For $d = 2$ the limit is $g = (\sqrt 5 - 1)/2 \approx 0.62$, for $d = 3$ about 0.45, in agreement with the lab's 0.618 and 0.445. The longer the delay, the smaller the allowed gain, and the more pulses the loop needs to remove a disturbance.

### Step 4: continuously running loops: the phase margin

Low-level RF feedback does not act "once per pulse" but measures and corrects continuously. Then a different description helps: think of the disturbance as sine waves of many frequencies. A delay $\tau$ makes a wave of frequency $f$ arrive $f\tau$ periods late, i.e. its phase lags by $360°\cdot f\tau$. If the correction lags by half a period (180°), it no longer cancels the disturbance but helps it.

The llrf lab of the [Feedback](#feedback) chapter uses a simplified model: the cavity itself responds to disturbances above its half bandwidth $f_{1/2}$ about 90° late (like a bucket that fills slowly), and the room the feedback has left at the **crossover frequency** $f_c = \text{gain}\times f_{1/2}$ (the **phase margin**) is

$$
90° - 360°\cdot f_c\,\tau .
$$

With the defaults $f_{1/2} = 100\ \mathrm{Hz}$ and gain 50, $f_c = 5\ \mathrm{kHz}$:

- $\tau = 2\ \mathrm{\mu s}$: $90° - 360°\times 5000\times 2\times10^{-6} = 86.4°$, very stable;
- $\tau = 30\ \mathrm{\mu s}$: $36°$, still working but near the edge;
- $\tau = 50\ \mathrm{\mu s}$: $0°$, at the stability limit.

```avas-lab
llrf delay=30
```

A few tens of microseconds of delay eat up the whole margin, while a command going round the network and the software takes milliseconds. That is the quantitative reason why cavity-field feedback must be built into hardware next to the cavity.

### For readers who know control theory

The discrete characteristic equation $z^{d+1} - z^d + g = 0$ is the closed loop of the plant $1/(z-1)$ (an integrator) with the delay $z^{-d}$; its stability limit follows from the Jury criterion or from setting $z = \ee^{\mathrm i\theta}$: $g_{\max} = 2\sin\dfrac{\pi}{2(2d+1)}$, which is 2, 1, 0.618 and 0.445 for $d = 0, 1, 2, 3$. In the continuous case the cavity is close to an integrator for $f \gg f_{1/2}$, the open loop is $L(s) = K\omega_{1/2}\ee^{-s\tau}/s$, the crossover frequency is $K f_{1/2}$ and the phase margin $90° - 360° f_c\tau$. Real low-level RF systems also include feedforward (e.g. predicted compensation of beam loading) and tuning loops against cavity detuning.

## Going deeper: how fast protection must be {#control-systems-deep-protection}

The main text said the machine protection system must cut the beam within microseconds. This section makes a rough estimate of why, with "heat = mass × specific heat × temperature rise". The numbers below are order-of-magnitude estimates only.

### Step 1: beam power

A proton has energy $W$ (in eV), and $I/e$ protons pass per second ($I$ the current, $e$ the elementary charge), so the power is

$$
P = \frac{I}{e}\cdot W\,e = I\cdot\frac{W}{e} .
$$

The number $W/e$ is "the energy in volts". For the example beam: $P = 0.184\times10^{-3}\ \mathrm{A}\times 1.527\times 10^{6}\ \mathrm{V} = 281\ \mathrm{W}$.

### Step 2: the example beam on a small piece of copper

Suppose all 281 W are deposited in 1 g of copper, ignoring heat conduction. The specific heat of copper is about $0.385\ \mathrm{J/(g\cdot K)}$, so the temperature rises per millisecond by

$$
\Delta T = \frac{P\,\Delta t}{m\,c} = \frac{281\ \mathrm{W}\times 10^{-3}\ \mathrm{s}}{1\ \mathrm g\times 0.385\ \mathrm{J/(g\cdot K)}} \approx 0.73\ \mathrm K .
$$

In one second that is over seven hundred degrees. In reality low-energy protons deposit their energy in a very thin surface layer of the metal, so locally it heats up even faster.

### Step 3: a megawatt beam

Suppose a facility has a beam power of 1 MW (the order of high-power proton linacs). On the same 1 g of copper:

- 10 µs: 10 J deposited, a rise of about 26 K;
- 100 µs: about 260 K;
- 1 ms: about 2600 K, far above the melting point of copper (about 1085 °C).

So the beam has to be cut within a few tens of microseconds to stop before damage happens. The estimate also shows that the protection system's reaction time must follow from the beam power and the most fragile component; there is no single answer.

### For readers who know heat transfer

A real estimate uses the energy deposition profile of the beam (low-energy protons penetrate only a few tens of micrometres of the surface, high-energy protons produce showers), the spot size and the thermal diffusion time of the material; for pulses short compared with the diffusion time the temperature rise can be taken as adiabatic, and thermal stress matters, not only melting. Personnel safety systems are usually given a required safety integrity level following functional-safety standards (such as the IEC 61508 series).

## In AVAS {#control-systems-in-avas}

### What AVAS does and does not do

AVAS is a beam-dynamics simulation code: a multi-particle engine plus a linear model. It has **no** hardware interface, no control-system interface (EPICS, Tango, …), no real-time or protection functions, and reads or writes the settings of no real facility. Its interface (the `avas gui` desktop window and the `avas serve` browser mode) connects the page and the back end over HTTP and WebSocket, is meant only for the local machine or a trusted network, and is not a control system either.

### Monitors and knobs

The "response and controllability" and "feedback" tabs of the research page share one list of monitors (BPMs, profile monitors) and knobs (steerers, element parameters, element offsets), saved in `<project>/Research/diagnostics.json`; by default it is taken from the lattice's `diag_position`, `diag_size` and steerers. Each monitor can have a resolution (µm), which the feedback simulation uses for reading noise.

```avas-params
diag_position diag_size
```

- "Response and controllability": the linear model's response matrix, singular values and condition number, the reading patterns that cannot be corrected and the knob combinations no monitor sees; with a period, whether the modes of one period are controllable and observable. The response matrix can also be measured with the engine (one run per knob, fixed random seed, lattice parameters only); this is a research run and excludes normal runs.
- "Feedback": every pulse the linear model with drifting errors, noisy readings and integral feedback $u \leftarrow u - g\,R^{+}y$ with a delay of $d$ pulses.
- Linear calculations take no run lock and write no input files. The orbit response matrix has been checked against the engine: through drifts and HWR cavities only, the elements differ by at most 0.06 % of the largest element; through the example `sol` field map by 0.1–2.7 % (caused by the field map itself).

### Correction in error studies

After each set of random errors, an error study can tune the knobs given by `adjust` to bring the beam at the `diag_*` places to their targets, which rehearses on the model "can a machine with errors be corrected?". The commands and the algorithm are in [Errors and correction](#errors-in-avas).

### In Python

`avas/dynamics/` offers the same calculations as the research page: `diagnostics.py` (monitors), `response.py` (response matrices), `systems.py` (state-space matrices of a period), `control.py` (controllability, observability, mode classes) and `feedback.py` (pulse-to-pulse feedback simulation); `Study` in `avas.research` runs the engine in a subprocess and caches the results. Connecting AVAS to the control system of a real facility needs a program of your own (for instance reading set points with an EPICS or Tango Python client library and converting them into lattice parameters through the excitation curves); it is not part of AVAS.

### The labs of this chapter

This chapter's two labs, `feedback` and `llrf`, are in `avas/learn/control.py`. `feedback` uses the linear model of the orbit-correction channel; `llrf` is a textbook baseband model: **AVAS does not simulate low-level RF**; the amplitude and phase of a cavity field in the engine are given by the lattice (error studies can add random errors), with no feedback loop.

## Further reading {#control-systems-reading}

- The EPICS website epics-controls.org and the Tango website tango-controls.org: architecture descriptions and getting-started documents.
- L. R. Dalesio et al., "The experimental physics and industrial control system architecture: past, present, and future", *Nucl. Instrum. Methods A* 352, 179 (1994).
- Proceedings of ICALEPCS (International Conference on Accelerator and Large Experimental Physics Control Systems), freely available on the JACoW website: how facilities actually build their control systems.
- Joint International Accelerator School, *Beam Loss and Accelerator Protection*, ed. R. Schmidt, CERN-2016-002 (2016): machine protection.
- M. G. Minty, F. Zimmermann, *Measurement and Control of Charged Particle Beams*, Springer (2003): the physics of beam measurement, feedback and high-level applications.
- Related chapters of this primer: [Accelerator control overview](#control-intro), [Beam diagnostics](#diagnostics), [Orbit correction](#orbit-correction), [Feedback](#feedback), [Controllability and observability](#controllability); the full bibliography is in [References](#references).
