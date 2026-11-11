#!/bin/bash
# Slurm example: one AVAS simulation with the new kernel on the GPUs of one node.
#
#   sbatch examples/cluster/avas_slurm.sh /path/to/project/InputFile /path/to/project/Results_001
#
# Works on CentOS 7 / RHEL 7 clusters (glibc 2.17): the prebuilt avas/engine/new/libavas_engine.so needs
# GLIBC 2.14, GLIBCXX 3.4.19 and no CUDA installation -- the CUDA runtime is linked into it, only the NVIDIA
# driver is needed (Linux 525.60.13 or newer).  So no "module load cuda" for the run.
# Needed on the cluster:
#   * Python 3.11-3.13, e.g. a Miniforge / conda environment in your home directory, with AVAS installed from
#     the source ZIP or a git clone: python -m venv .venv && .venv/bin/pip install -e .   (or pip install -r
#     requirements.txt); compute nodes without internet need the environment prepared on the login node;
#   * nothing else for one node.  MPI (several nodes) needs a self-built library, see the end of this file.
# The prebuilt library has machine code for sm_61 (GTX 10x0), 75, 80 (A100), 86, 89 (L40S) and 90 (H100) plus PTX
# for newer GPUs; P100 (sm_60) and V100 (sm_70) need a self-built library.
# Check first on the login node (no GPU there): python -m avas info --engine new
#
#SBATCH --job-name=avas
#SBATCH --partition=gpu
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --gres=gpu:2
#SBATCH --cpus-per-task=8
#SBATCH --time=02:00:00
#SBATCH --output=avas_%j.log

set -euo pipefail
INPUT=${1:?usage: sbatch avas_slurm.sh INPUT_DIR OUTPUT_DIR}
OUTPUT=${2:?usage: sbatch avas_slurm.sh INPUT_DIR OUTPUT_DIR}
AVAS_DIR=${AVAS_DIR:-$HOME/AVAS_XMU}          # the AVAS source folder
PYTHON=${PYTHON:-$AVAS_DIR/.venv/bin/python}

# Slurm sets CUDA_VISIBLE_DEVICES to the GPUs of this job: "--gpus all" means all of those, never other
# users' GPUs.  One process drives every GPU of the node (the particles are split between them).
echo "node $(hostname), GPUs ${CUDA_VISIBLE_DEVICES:-none}"
"$PYTHON" -m avas run --input "$INPUT" --output "$OUTPUT" \
  --engine new --device cuda --gpus all --threads "${SLURM_CPUS_PER_TASK:-8}"
# --device cuda stops with a clear message when the job got no GPU; --device auto would use the CPU instead.
# --precision auto (default): fp64 on A100 / H100 (full-rate fp64), the fp32 tier on GeForce / L40S-class GPUs;
# add --precision fp64 to force fp64.  OutputFile/avas_run.json records what was actually used.

# ---------------------------------------------------------------------------------------------------------
# Several nodes (MPI).  The prebuilt library has no MPI; build one on the cluster (CONTRIBUTING.md,
# "New kernel": cmake -S engine -B build-mpi -DAVAS_WITH_CUDA=ON -DAVAS_WITH_MPI=ON with the cluster's MPI).
# avas run cannot start MPI ranks: stage the inputs once, then start the engine's own driver, one rank per GPU
# (rank 0 writes the outputs).  Replace the "avas run" lines above with:
#
#   #SBATCH --nodes=2  --ntasks-per-node=2  --gres=gpu:2
#   module load mpi/intelmpi              # the MPI the library was built with (name depends on the cluster)
#   "$PYTHON" -c "import sys; from avas.api.basic import stage_inputs; stage_inputs(sys.argv[1], sys.argv[2])" \
#     "$INPUT" "$OUTPUT"                  # writes $OUTPUT/inputs/ with the generated lattice.txt
#   export AVAS_ENGINE_DEVICE=cuda AVAS_ENGINE_PRECISION=fp64
#   srun --mpi=pmi2 "$AVAS_DIR/build-mpi/avas_engine_run" "$OUTPUT/inputs/" "$OUTPUT/" "$INPUT/"
#
# MPI speeds up the fp64 tier only (in the fp32 tier every rank computes all particles), and the run is not
# recorded in avas_run.json; the results equal a single-process run on the same GPUs.
