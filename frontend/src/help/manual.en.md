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

Use the File menu or project switcher to open another project. Unsaved changes are checked before switching or closing. Files supports viewing, importing, duplicating, renaming and recycling input files.

Inputs are read-only during a run. You can still inspect files and results.

## Beam settings

Use Beam to configure energy, current, particle count and distribution. Saving updates beam.txt while retaining unmanaged keywords and comments. Ctrl+Z and Ctrl+Y undo and redo changes.

The beam reference entries below provide keyword meanings, units and choices. Prepare any external distribution file before using it and check that the distribution and particle count suit the study.

## Lattice editing

The file selector opens a lattice for viewing or editing. Use the separate Set as run lattice button to choose the simulation input.

Text + Structure shows the text and physical parameter panels together. Select an element to inspect its parameters, position and diagnostics. Text, structure and visual edits share one source and undo history.

Visual mode starts in read-only browsing. Enter editing before changing elements or parameters. Finishing with unsaved changes offers save, discard this editing session or continue editing. TraceWin .dat files are currently read-only.

The top-left controls in 2D and 3D zoom and fit the view. Double-click the 2D view to fit all; in 3D, double-click empty space to fit all or an element to focus it. Manual 3D zoom stops roaming and bunch following.

The Periods tab of Text + Structure analyses the periods marked by `lattice n 1` … `lattice_end`: phase advances with and without current, tune depression, matched α / β, mismatch of the present beam, Floquet and envelope stability, and the phase advance of every period along the line. It uses the editor's current text and the project's beam (linear model; accelerating periods are normalised by βγ). Use the matched α, β on the Beam page only fills the Beam page's fields; saving there writes beam.txt.

## Simulation settings

Settings manages simulation options in input.txt with undo and redo. See the input reference entries for individual fields.

CPU multithreading writes multithreading 1 when enabled and removes the line when disabled. Do not manually write multithreading 0: the engine loses all particles at the start with that value.

Resolve lattice errors and missing field maps before running. The linear envelope preview is a quick estimate; use engine results for final conclusions.

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

Parameter scan: choose one or two axes (a lattice element parameter, or a beam.txt / input.txt keyword) and their values, with the linear model (a whole map in seconds) or the engine (one multi-particle simulation per point, one at a time, pausable and stoppable on Run). One axis gives a curve, two a heat map; click a point to see its envelope along the line, or open an engine run on Results. Engine runs use a fixed random seed and are kept in `Research/gui/`; a point with identical inputs comes from the cache, and `avas.research` in scripts reads the same runs. Scan descriptions live in `Research/gui/scans/`. Scans from the old `Scans/` remain listed.

Stability chart: choose a period and two axes (element parameter or beam current); every point gets the matched beam with space charge, the phase advances and the envelope stability, drawn as heat maps. Empty cells have no matched beam.

Response & controllability: monitors (position or rms size, resolution) and knobs (steerers, element parameters or offsets) default to the lattice's `diag_position` / `diag_size` lines and steerers; edit them and save to `Research/diagnostics.json`. Compute the response matrix of the linear model or of the engine (one run per knob), its singular values, rank, uncorrectable reading patterns and the effect of monitor noise on the knobs, and which modes of a period are stable, controllable and observable.

Feedback: simulate pulse-to-pulse orbit feedback with gain, delay, a drifting element and monitor noise, compare the readings with and without feedback, and see the theoretical stability limit.

## Results analysis

Choose the latest output, a kept run, a segment result, an engine run of the Study page or another output directory in Results, then select plots or phase-space views. Interactive zoom and figure export are available. Envelope, centroid, energy and emittance plots can overlay the linear model of the run's own inputs (dashed, Linear model) to show what the multi-particle simulation adds.

Compare runs overlays the same quantity from multiple runs. Check units, inputs and coordinate ranges before comparing. Multithreaded simulations can differ slightly; output need not be byte-identical.

Replay lives in Run; Results is for analysis. NaN rms columns can be normal for a single-particle calculation; check surviving particles and logs as well.

## AI assistant

Open the assistant with Ctrl+Shift+A and configure an OpenAI-compatible endpoint or local model. API keys are stored in Windows Credential Manager. Use a local model service when data must remain on your machine.

The assistant can read the project, explain results and run trials on copies. Project changes appear as proposals applied after approval, unless automatic application is enabled for the conversation. Changes are backed up and can be undone.

Trials write under .avas_ai/runs without changing InputFile or OutputFile. Modification proposals are refused while project inputs are locked; request them again after the run.

## Troubleshooting

### Missing field maps or lattice errors

Check referenced files, names and paths, then resolve the Lattice diagnostics before running.

### All particles lost at the start

First check for multithreading 0, then inspect beam settings, apertures, field maps and logs. Remove the keyword line to disable multithreading.

### Source changes do not appear in the desktop executable

Packaged executables need rebuilding after source changes. Users of release packages do not need Node.js or Word.


## Further help

[Cases and verification conditions](#cases-setup) · [Parameters and files](#reference-home) · [Acceptance](#cases-acceptance)

## Error studies

Choose static, dynamic or combined errors and a seed in Settings. Configure err_step, amplitudes and enable commands in the lattice, then save and run. Results are stored in error_output.

[Error tutorial](#cases-errors)
