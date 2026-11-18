# AVAS user manual

Keep this manual open while working. Drag the title bar to move it or its edges to resize it. Maximize and restore using the title buttons; press Esc to close. Reopening restores the default position and size. Search filters chapter titles and content; select a contents entry to jump to it. The manual follows the interface language.

## Quick start

1. Open a project or create one from the File menu. For a first run, work on a copy of examples/hwr010.
2. Check the beam energy, particle count and distribution, then the lattice, field maps and simulation settings.
3. Confirm the file marked for simulation in the Lattice page, save changes and start the run.
4. Follow progress, logs and live envelopes in Run; inspect plots and phase space in Results afterwards.
5. Keep the completed run in the run records before starting another calculation if you need its results.

## Projects and input files

ini.ini defines the project inputs and outputs. InputFile holds beam, settings, lattice and field-map files; OutputFile holds the latest full run. The lattice/source setting selects the simulation lattice; its filename is not necessarily lattice_mulp.txt.

Use the File menu or project switcher to open another project. Unsaved changes are checked before switching or closing. Files is the browser of InputFile: raw text, field-map profiles and slices, particle files, and importing, duplicating, renaming and recycling. Everyday edits happen on their own pages: the initial particle distribution and secondary particles on Beam, boundary.txt on Simulation settings. No kernel reads the scanData.txt of older projects; it can only be viewed as raw text on Files.

Inputs are read-only during a run. You can still inspect files and results.

## Beam settings

Use Beam to configure energy, current, particle count and distribution. Saving updates beam.txt while retaining unmanaged keywords and comments. Ctrl+Z and Ctrl+Y undo and redo changes.

The beam reference entries below provide keyword meanings, units and choices. With “Read particles from a .dst file” checked and a file chosen, the page shows that file’s particles (x-x′, y-y′, φ-W; the red curve is the rms ellipse of the particles), and “Fill parameters from file” copies mass, current, energy and Twiss into the form.

Checking “Secondary beam” writes secondarybeam 1 in input.txt and edits SeParticle.txt in the table below it (one particle per row); the synchronous particle still comes from the beam parameters. Saving checks that every row has at least 11 columns (x y z vx vy vz charge mass weight time kind, the first ten numeric) and drops blank rows. The new kernel's default mode reads SeParticle.txt when beam.txt has no `readparticledistribution`; the original kernel and the new kernel's compatibility mode (Reproduce AVAS.dll) read only the file named by `readparticledistribution` in beam.txt, and the page shows a notice in that case (AVAS does not rewrite beam.txt). Secondary-particle runs write no envelope (DataSet.txt is empty); the results are in `outData_*.txt` and `SeSynParticle.txt`, and the run log says so. Segment runs refuse secondarybeam 1.

“History” lists the saved versions of beam.txt; restoring one fills the form as one undoable change, written to the file when you save.

## Lattice editing

The file selector opens a lattice for viewing or editing. Use the separate Set as run lattice button to choose the simulation input.

Text + Structure shows the text and physical parameter panels together. Select an element to inspect its parameters, position and diagnostics. Text, structure and visual edits share one source and undo history.

Visual mode starts in read-only browsing. Enter editing before changing elements or parameters. Finishing with unsaved changes offers save, discard this editing session or continue editing. TraceWin .dat files are currently read-only.

The top-left controls in 2D and 3D zoom and fit the view. Double-click the 2D view to fit all; in 3D, double-click empty space to fit all or an element to focus it. Manual 3D zoom stops roaming and bunch following.

The Periods tab of Text + Structure analyses the periods marked by `lattice n 1` … `lattice_end`: phase advances with and without current, tune depression, matched α / β, mismatch of the present beam, Floquet and envelope stability, and the phase advance of every period along the line. It uses the editor's current text and the project's beam (linear model; accelerating periods are normalised by βγ). Use the matched α, β on the Beam page only fills the Beam page's fields; saving there writes beam.txt.

## Simulation settings

The Simulation settings page manages simulation options in input.txt and the computing kernel in ini.ini (see Computing kernels) with undo and redo. See the input reference entries for individual fields. With “Apply boundary (boundary.txt)” checked, the loss boundary table is edited below it; the table is saved with the page, and an empty table does not create a file. “History” lists the saved versions of input.txt and restores them as on the Beam page.

Neither the original nor the new kernel reads `scanphase` in input.txt or scanData.txt; the RF phases come only from the phase parameter of the field lines (V3 = 0: synchronous phase, for which the kernel finds the entrance phase by its own phase scan; V3 = 2: absolute phase at t = 0), so the page no longer offers a phase-scan option. When input.txt contains a `scanphase` line, a one-line notice says that the kernels do not read it, and the line is removed when the page is saved. scanData.txt is not deleted and can still be viewed as raw text on Files.

With the original kernel or the new kernel's compatibility mode (Reproduce AVAS.dll) and `randomseed` ≠ 0, the generated initial distribution has x fully correlated with the phase (correlation −1), a defect of the original kernel; the page warns next to the random seed. The new kernel's default mode does not have this defect.

How many CPU threads a run uses is set by CPU threads under Computing kernel, for both kernels. The `multithreading` keyword of input.txt cannot switch multithreading off: both kernels treat a missing line as `multithreading 1`, and 0, 2 or 3 lose every particle at the start. The page no longer has a multithreading switch; when input.txt has such a value the page warns and saving removes the line, while a `multithreading 1` line is kept as it is.

Resolve lattice errors and missing field maps before running. The linear envelope preview is a quick estimate; use engine results for final conclusions.

## Computing kernels {#guide-kernels}

AVAS has two computing kernels. The **new kernel** is the rebuilt kernel and the default on every platform; it computes on the CPU, NVIDIA GPUs (CUDA) and Mac GPUs (Metal) and supports the PICNIC space-charge solver and the cell-by-cell `rfq` element (computed on the CPU in the default mode only; the original kernel's rfq never ran, so there are no reference results: check it against other codes before relying on it). It corrects several defects of the original kernel (for example x and the phase of the initial bunch fully correlated with a fixed random seed, space charge in matrix elements too strong and different from run to run with multithreading, SPICNIC cutting off the field of long bunches), so its results differ slightly from the original kernel's. The **original kernel** (AVAS.dll / libAVAS.so) exists for Windows and Linux x86-64 only and will be removed in a later release; it can still be chosen, and the new kernel's mode "Reproduce AVAS.dll" gives its results bit for bit. On Windows and Linux the first start after the update that made the new kernel the default explains this and offers "Use the original kernel by default". A project can choose its kernel, stored in the `[engine]` section of ini.ini; the original kernel still reads the same project (input.txt does not change). Projects without their own choice use the default computing kernel.

**Settings → Default computing kernel** offers Built-in default, Original kernel and New kernel, stored in the interface preferences (gui.json). The built-in default is the new kernel on every platform; choosing Original kernel on a platform without it (macOS, ARM) falls back to the built-in default with a note. The kernel of a run is decided in this order: command line > environment variables `AVAS_ENGINE*` > `kernel` in the project's ini.ini `[engine]` > the default computing kernel > the built-in default. The default computing kernel also applies to `avas run` on the command line (read from the same gui.json, or from the file named by the environment variable `AVAS_GUI_SETTINGS`), research runs and the assistant's sandbox runs.

Choose it in **Simulation settings → Computing kernel**:

- **Kernel**: the new kernel or the original kernel (being retired). When the project's `[engine]` has no `kernel`, the page says that it follows the default (and which kernel that currently is); the Follow the default button removes the project's own choice. On platforms without the original kernel, such as macOS, the page says that this platform only has the new kernel. When the new kernel's library is missing, the option cannot be chosen and the page explains how to get it.
- **Device**: Automatic, CPU, and the NVIDIA GPU or Mac GPU detected on this computer; the refresh button at the top right detects them again. With Automatic, the line below shows what this project's runs will use: CUDA when an NVIDIA GPU is present (the CPU for fewer than 1,000 particles, single-particle tracking included, where the GPU is slower); on a Mac, Metal for runs with space charge, at least 60,000 particles and at least half as many particles as space-charge mesh cells (the product of the three numofgrid values), and the CPU for fewer particles or without space charge (with few particles the fixed cost and the mesh work of every step dominate and the Mac GPU is slower than the CPU); otherwise the CPU.
- **Precision**: Automatic, fp64 or fp32. Metal computes in fp32 only and the CPU in fp64 only; invalid combinations cannot be chosen. Automatic means fp64 on the CPU and on GPUs with full-rate fp64, fp32 on other GPUs and on Metal.
- **GPUs**: shown when the computer has several NVIDIA GPUs; with no GPU ticked, all are used.
- **Advanced**: CPU threads (0 = automatic; for the new kernel only the speed changes, its results do not depend on the thread count; the original kernel receives it as `OMP_NUM_THREADS`, and 1 avoids its data race in the space charge of matrix elements, so the results repeat exactly, at a lower speed); the compatibility mode “Reproduce AVAS.dll” reproduces the original kernel bit for bit (its defects included, CPU only) for comparisons; the default fixes the original kernel's defects.

With the original kernel, device, precision and the compatibility mode are disabled: they apply only to the new kernel; CPU threads apply to both kernels. The settings are saved with the page, `[engine]` is written only when it changed, and the run lock applies. `avas run --engine … --device …` and the `AVAS_ENGINE*` environment variables take precedence over ini.ini; the page warns when such variables are set.

The original kernel's old GPU path on Linux (ini.ini `[input] device = gpu`, using libPIC.so) is deprecated: the run log warns, and it will be removed in a later release. Use the new kernel's NVIDIA GPU instead (`avas run --engine new --device cuda`, or Simulation settings → Computing kernel).

Before a run starts (the Run page, segment runs, error studies, engine scans and response matrices of the Study page, the assistant's trial runs), AVAS checks that the chosen device exists and the combination is valid and says so at once, instead of switching to the CPU silently. With the original kernel, choosing PICNIC shows “Only the new kernel supports PICNIC” next to the solver, and the run is stopped before it starts.

With the original kernel or the compatibility mode (Reproduce AVAS.dll), a project with the boundary enabled and a superpose block in boundary.txt is stopped before the run (AVAS.dll stops with an exception on such a block; use the new kernel's default mode), and an RF cavity at the start of a superpose block (superpose offset 0) is pointed out before the run: it gets no field and does not accelerate in the original kernel. On Linux the run's input copy of boundary.txt gets LF line endings for the original kernel (libAVAS.so crashes on Windows line endings); the project's file is not changed.

The kernel each run actually used is recorded in the `engine` entry of avas_run.json. The Run page (run details, live panel, run records), the status bar, the sources of the Results page and run comparison show it as a label such as “New kernel · Metal fp32” or “Original kernel”; when Automatic fell back to the CPU the label reads “actual: CPU (reason)”. During a run the expected device is shown, replaced by the actual one when it finishes; older records without this information show no label. Run comparison compares original-kernel and new-kernel runs of the same project directly; when the two runs used different kernels or compatibility modes, the Run page's comparison with the previous run and the Compare tab of Results note that differences may come from the kernel. Different kernels and result tiers (original, new CPU fp64, CUDA fp64, fp32, reproduce AVAS.dll) use different random-number streams and arithmetic, so their results differ within statistical noise; only runs of the same tier and seed compare value by value.

**Help → About AVAS** lists both kernels' versions, the new kernel's build options and this computer's devices; Copy version information includes them.

## Differences between the new and the original kernel {#guide-kernel-differences}

The original kernel's source code (AVAS.dll / libAVAS.so) was lost; the new kernel was rewritten from a specification reverse-engineered from the original binaries. Of the 280 items of the parity list, 279 have a test case or check, and the new kernel's mode “Reproduce AVAS.dll” gives byte-identical output to AVAS.dll 2026-04-02 on Windows, so the calculations the two share are the same. The new kernel's default mode also corrects defects of the original kernel, which is why default-mode results differ slightly from before. The differences fall into four groups:

**1. Programming errors of the original kernel (corrected; certain)**

- With a fixed random seed (`randomseed` not 0) the original kernel's initial distribution has x fully correlated with the phase (correlation −1), although the planes should be independent. Old results with the original kernel and a fixed seed may have emittances and losses affected by this.
- With multithreading, space charge in matrix elements such as quadrupoles is too strong and differs from run to run (a data race between threads).
- The asymmetry correction of SPICNIC depends on the number of CPU cores; an odd number of mesh cells makes the program hang.
- A superpose block in boundary.txt crashes the program; a boundary.txt with Windows line endings crashes libAVAS.so on Linux; particles reflected inside a superposeout block keep the run from ever ending; an RF cavity at the start of a superpose block gets no field and no error; some outputs contain uninitialised memory.

**2. More accurate space charge (checked against analytic solutions)**

- **SPICNIC**: the original Green's-function table is badly undersampled for elongated mesh cells (long bunches, continuous beams) and truncates the kernel at 0.75 of the mesh, so the space-charge growth of a DC KV beam was only 38–81 % of the analytic envelope and changed strongly with the mesh. The new kernel integrates 1/r analytically over each cell and covers the whole mesh: the field of a uniformly charged ellipsoid is within 0.34 % of the analytic one, and the growth of a DC KV beam within 1 % of the analytic envelope. For nearly cubic cells the old approximation is adequate and the difference small; for long bunches emittances and sizes may differ by 1–2 %.
- **PICNIC** (formerly libPIC.so on Linux): the new kernel converges to the analytic ellipsoid field (0.19 % at 32³); the original library's field is about 3.3 % weaker, traced to three defects in its coefficient table, truncation and boundary nodes.
- **Synchronous phase**: the original kernel finds the phase by scanning, about 0.07° off (−33° set, −32.93° obtained); the new kernel solves for the set value exactly.

**3. Different conventions (neither is wrong)**

- With a `.dst` particle file the new kernel takes beam.txt `kneticenergy` as the synchronous particle's energy (as the authors' 2026-07-22 version; a negative value takes the mean energy of the file), the original kernel the mean energy of the file. A difference of more than 1 % is pointed out before the run.
- `spacechargecomp` written before a quadrupole, solenoid, dipole or edge (or before the first element) takes effect from there in the new kernel and has no effect in the original kernel.
- Inputs that only the new kernel reads (output plane names, `lpboundary` V2, `initpos` / `initmom`, …) are tagged “New kernel only” in the interface.

**4. Not yet verified**

- Neither kernel has been compared systematically with an independent code such as TraceWin or with measured beam data; “more accurate” means consistent with the physical definitions and converging to analytic solutions.
- Both kernels share the same approximations: PIC mesh resolution, hard-edge models, the conducting walls and periodic images of FFT.
- The cell-by-cell `rfq` element runs only in the new kernel and has no reference results; check it against another code before relying on it.

**Comparing on your own project**: choose the original and the new kernel (or the new kernel's mode “Reproduce AVAS.dll”) in turn in **Simulation settings → Computing kernel**, run once with each and Keep this run after each, then overlay envelopes, emittances and energy with Compare runs on the Results page. The new kernel's random numbers differ from the original kernel's, so multi-particle results differ by statistical fluctuations: use enough particles and several seeds. Runs that read a particle file and single-particle runs have no statistical fluctuation, so their differences come from the kernel only. In a source installation `python tools/compare_kernels.py --input <project>` runs and compares several runs automatically (on copies, without touching the project).

**Going back to the original kernel**: on Windows and Linux choose the original kernel for one project in **Simulation settings → Computing kernel**, or for all projects in **Settings → Default computing kernel**; on the command line use `--engine original`. The original kernel will be removed from the release packages later; the mode “Reproduce AVAS.dll” will still give its results bit for bit.

## CPU and GPU on each platform {#guide-gpu}

| Platform | Installation | CPU | GPU |
| --- | --- | --- | --- |
| Windows 10 / 11 x64 | Installer `AVAS-version-setup.exe` or `avas-windows.zip`; both kernels included | new and original kernel | NVIDIA (CUDA), only the graphics driver >= 527.41 |
| Linux x86-64 (glibc >= 2.17, CentOS 7 included) | From source, no installer (see “Linux servers and clusters”) | new and original kernel | NVIDIA (CUDA), only the driver >= 525.60.13 |
| macOS (Apple silicon) | From source, new kernel only | new kernel | Mac GPU (Metal, fp32 only) |

**No CUDA installation is needed.** The CUDA 12.4 runtime is linked into the new kernel's library, so the installed CUDA toolkit version (11.x, 12.x, 13) does not matter and Linux clusters need no `module load cuda`; only the NVIDIA driver version in the table is required.

**Supported NVIDIA GPUs** (the prebuilt library contains machine code for these architectures):

| Architecture | Typical GPUs |
| --- | --- |
| Pascal sm_61 | GTX 10 series, Tesla P40, P4 |
| Turing sm_75 | T4, RTX 20 series, GTX 16 series |
| Ampere sm_80 / 86 | A100, A30; RTX 30 series, A40, A10, RTX A6000 |
| Ada sm_89 | L40S, L4, RTX 40 series |
| Hopper sm_90 | H100, H200 |

Newer architectures run the embedded intermediate code (compute_90 PTX), compiled by the driver; not tested yet. **P100 (sm_60), V100 (sm_70), Maxwell and older GPUs cannot use the prebuilt library's GPU code**: choosing the GPU stops the run before it starts with the reason, Auto uses the CPU and says so in the log. If needed, rebuild the new kernel with CMake and CUDA 12.x including these architectures (CUDA 13 no longer supports Pascal). Linux on ARM (Jetson, Kunpeng, ...) is not supported.

**Choosing the device**: in the interface with **Simulation settings → Computing kernel** (see “Computing kernels”); on the command line with

```bash
avas run --input proj/InputFile --output proj/Results_001 --engine new --device cuda --precision auto --gpus all
```

`--device` is `auto`, `cpu`, `cuda` or `metal`, `--precision` `auto`, `fp64` or `fp32`, `--gpus` `all` or `0,1`, and `--threads` sets the CPU threads (0 = all). The same keys can be written to the `[engine]` section of the project's ini.ini so that they need not be given every time; priority: command line > environment > ini.ini > default computing kernel. `avas info` shows the kernel library found, this computer's GPUs and driver, and what Auto would use; `avas doctor` checks the installation. The kernel, device and precision a run actually used are recorded in avas_run.json in its output folder.

Auto chooses CUDA when an NVIDIA GPU is present and the run has at least 1000 particles; Metal on a Mac with space charge, at least 60 000 particles and at least half a particle per space-charge mesh cell; otherwise the CPU. Precision Auto is fp64 on the CPU and on GPUs with full-rate fp64 (A100, H100, ...), fp32 on other GPUs.

## Linux servers and clusters {#guide-linux}

On Linux, install from source (clone the repository or download `avas-source.zip` of a release); the new kernel's prebuilt library `avas/engine/new/libavas_engine.so` is part of the repository:

```bash
python3.11 -m venv .venv
.venv/bin/python -m pip install -e .
source .venv/bin/activate
avas info
avas run --input /path/proj/InputFile --output /path/proj/Results_001
```

- **CPU**: `--engine new --device cpu --threads 32`. Limit the threads on a shared server.
- **GPU**: `--engine new --device cuda --gpus 0`; several GPUs of one node with `--gpus all`, driven by one process. A Slurm job example is `examples/cluster/avas_slurm.sh` in the repository.
- **Original kernel**: `--engine original` uses libAVAS.so on the CPU only; ini.ini `[input] device = gpu` (the old libPIC.so) is deprecated.
- **Clusters without internet access**: Python 3.11 can be installed in the home folder with Miniforge, the Python packages downloaded elsewhere and installed offline; no Docker, administrator rights or CUDA module are needed.
- **Graphical interface**: run `avas serve` on the server and open the address it prints (with its access token) in a local browser through an SSH tunnel. The server listens on 127.0.0.1 only and has no user management yet; use it on trusted networks only.

## Running and keeping results

Only one task can run at a time. Starting checks and saves modified pages. If the previous completed result has not been kept, choose Keep and run, Run directly or Cancel.

Pause suspends the process; resume continues it and stop ends it. Inputs remain read-only while paused. Full runs use the inputs snapshot in the output directory without rewriting original inputs.

A full run overwrites OutputFile. Keeping a run copies its results and input snapshot into a separate Runs directory. Completed records can be replayed; unwanted records can be recycled.

Segment runs write under Segments without replacing full-run results. Use the segment workflow for the required RF phase conversion.

## Live envelopes and replay

Run shows the current live envelope and can replay it after completion. Replay on a run record selects a historical result. A new run switches back to the current calculation.

Lattice replays the last run against the edited lattice, so results may be outdated after edits. Last-run curves start hidden and appear when replay begins. The × to the left of a legend item hides its curve.

The bunch and particle cloud are illustrations based on rms envelopes, not actual particle distributions. Comparison with the preceding run is off by default. Animation follows View > Motion settings.

## Parameter scans

The Study page (formerly Scan) has four tabs; everything runs on copies of the inputs and leaves the project's inputs and full-run results alone.

Parameter scan: choose one or two axes (a lattice element parameter, or a beam.txt / input.txt keyword) and their values, with the linear model (a whole map in seconds) or the engine (one multi-particle simulation per point, one at a time, pausable and stoppable on Run). One axis gives a curve, two a heat map; click a point to see its envelope along the line, or open an engine run on Results. Engine runs use a fixed random seed and are kept in `Research/gui/`; a point with identical inputs comes from the cache, and `avas.research` in scripts reads the same runs. Scan descriptions live in `Research/gui/scans/`. Scans from the old `Scans/` remain listed. Research runs use the fixed random seed 12345; with the original kernel or the compatibility mode (Reproduce AVAS.dll), the scan and response tabs warn that the initial distribution generated this way has x fully correlated with the phase (see Simulation settings).

Stability chart: choose a period and two axes (element parameter or beam current); every point gets the matched beam with space charge, the phase advances and the envelope stability, drawn as heat maps. Empty cells have no matched beam.

Response & controllability: monitors (position or rms size, resolution) and knobs (steerers, element parameters or offsets) default to the lattice's `diag_position` / `diag_size` lines and steerers; edit them and save to `Research/diagnostics.json`. Compute the response matrix of the linear model or of the engine (one run per knob), its singular values, rank, uncorrectable reading patterns and the effect of monitor noise on the knobs, and which modes of a period are stable, controllable and observable.

Feedback: simulate pulse-to-pulse orbit feedback with gain, delay, a drifting element and monitor noise, compare the readings with and without feedback, and see the theoretical stability limit.

## Results analysis

Choose the latest output, a kept run, a segment result, an engine run of the Study page or another output directory in Results, then select plots or phase-space views. Interactive zoom and figure export are available. Envelope, centroid, energy and emittance plots can overlay the linear model of the run's own inputs (dashed, Linear model) to show what the multi-particle simulation adds.

Compare runs overlays the same quantity from multiple runs. Check units, inputs and coordinate ranges before comparing. Runs with `randomseed 0` (new particles every run) and the original kernel (a multithreading data race) differ slightly, so their output need not be byte-identical; with a non-zero seed two runs of the new kernel are identical byte for byte, whatever the thread count. When the runs used different kernels or compatibility modes, the comparison notes that differences may come from the kernel.

Replay lives in Run; Results is for analysis. NaN rms columns can be normal for a single-particle calculation; check surviving particles and logs as well. Secondary-particle runs have no envelope (DataSet.txt is empty); Results notes that there are no envelope curves, and the data are in `outData_*.txt` and `SeSynParticle.txt`.

## AI assistant

Open the assistant with Ctrl+Shift+A and configure an OpenAI-compatible endpoint or local model. API keys are stored in Windows Credential Manager. Use a local model service when data must remain on your machine.

The assistant can read the project, explain results and run trials on copies. Project changes appear as proposals applied after approval, unless automatic application is enabled for the conversation. Changes are backed up and can be undone.

Trials write under .avas_ai/runs without changing InputFile or OutputFile. Modification proposals are refused while project inputs are locked; request them again after the run.

## Troubleshooting

### Missing field maps or lattice errors

Check referenced files, names and paths, then resolve the Lattice diagnostics before running.

### All particles lost at the start

First check for multithreading 0, then inspect beam settings, apertures, field maps and logs. Leaving the line out is the same as writing 1.

### Source changes do not appear in the desktop executable

Packaged executables need rebuilding after source changes. Users of release packages do not need Node.js or Word.


## Further help

[Cases and verification conditions](#cases-setup) · [Parameters and files](#reference-home) · [Acceptance](#cases-acceptance)

## Error studies

Choose static, dynamic or combined errors and a seed on the Simulation settings page. Configure err_step, amplitudes and enable commands in the lattice, then save and run. Results are stored in error_output.

[Error tutorial](#cases-errors)
