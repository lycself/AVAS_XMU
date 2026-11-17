"""How to compute on each platform (CPU / GPU, kernels, requirements), for the assistant's ``kernel_info`` tool.

The facts are the ones verified during the kernel rebuild (docs/engine/M6-notes.md section 6, M7c-notes.md
section 3, M8-plan.md sections 12-13) and repeated for users in README.md and the manual (guide-kernels,
guide-linux, guide-gpu).  Keep the three in step when one changes.
"""

MIN_DRIVER = {"Linux": "525.60.13", "Windows": "527.41"}
CUDA_ARCHS = ("sm_61 Pascal (GTX 10, P40, P4)", "sm_75 Turing (T4, RTX 20, GTX 16)", "sm_80 Ampere (A100, A30)",
              "sm_86 Ampere (RTX 30, A40, A10, A6000)", "sm_89 Ada (L40S, L4, RTX 40)", "sm_90 Hopper (H100, H200)")
CUDA_MIN_PARTICLES = 1000          # engine/src/core/device.h kCudaMinParticles
METAL_MIN_PARTICLES = 60000        # kMetalMinParticles (and at least 0.5 particle per mesh cell)

COMMON = [
    "Two kernels: the new kernel (default on every platform) and the original kernel AVAS.dll / libAVAS.so 2026-04-02 "
    "(Windows and Linux x86-64 only, CPU only, being retired). The new kernel's mode compat=dll reproduces AVAS.dll "
    "bit for bit (CPU only).",
    "Choose per project in Simulation settings -> Computing kernel (ini.ini [engine]: kernel, device auto|cpu|cuda|metal, "
    "precision auto|fp64|fp32, gpus, threads, compat), for all projects in Settings -> Default computing kernel, or per "
    "run on the command line: avas run --input DIR --output DIR --engine new --device cuda --precision auto --gpus all "
    "--threads N. Priority: command line > environment (AVAS_ENGINE, AVAS_ENGINE_DEVICE, ...) > ini.ini > default.",
    "A device chosen explicitly that this computer does not have stops the run before it starts (no silent fallback "
    "to the CPU); with device auto the reason for using the CPU is written to the log. avas_run.json 'engine' records "
    "the kernel, device and precision actually used.",
    "precision auto: fp64 on the CPU and on GPUs with full-rate fp64 (A100, H100, ...), fp32 (coordinates relative "
    "to the reference particle) on other GPUs; Metal is fp32 only. Results of different devices or precisions differ "
    "slightly (random numbers, arithmetic).",
    "avas info prints the library found, this computer's devices and what auto would use; avas doctor checks the "
    "installation.",
]

PLATFORMS = {
    "Windows": [
        "Install with AVAS-<version>-setup.exe (or avas-windows.zip); both kernels are included.",
        f"CPU: always available. NVIDIA GPU (CUDA): only the graphics driver is needed (>= {MIN_DRIVER['Windows']}); "
        "the CUDA runtime is built into the kernel library, no CUDA toolkit is required.",
        f"device auto uses CUDA when an NVIDIA GPU is present and the run has at least {CUDA_MIN_PARTICLES} particles, "
        "else the CPU.",
    ],
    "Linux": [
        "Linux x86-64 with glibc >= 2.17 (CentOS 7 clusters included); install from source: python3.11 -m venv .venv, "
        ".venv/bin/python -m pip install -e . (the prebuilt avas/engine/new/libavas_engine.so is in the repository). "
        "There is no Linux installer.",
        f"NVIDIA GPU: only the driver is needed (>= {MIN_DRIVER['Linux']}); the CUDA 12.4 runtime is linked into the "
        "library, so the installed CUDA toolkit version (11.x, 12.x, 13) does not matter and no 'module load cuda' is "
        "needed.",
        "Several GPUs of one node: --gpus all (one process drives them); Slurm example: examples/cluster/avas_slurm.sh. "
        "On a shared server limit the CPU threads with --threads. A browser interface for a server: avas serve "
        "(open it through an SSH tunnel).",
        f"device auto uses CUDA when an NVIDIA GPU is present and the run has at least {CUDA_MIN_PARTICLES} particles, "
        "else the CPU. The original kernel (libAVAS.so) runs on the CPU only; ini.ini [input] device = gpu (the old "
        "libPIC.so) is deprecated.",
    ],
    "Darwin": [
        "macOS on Apple silicon, source install only (no installer); only the new kernel. It uses the prebuilt "
        "avas/engine/new/libavas_engine.dylib, or one built with Apple clang (env -u CC -u CXX cmake -S engine -B "
        "engine/build).",
        f"Devices: CPU, and the Mac GPU through Metal (fp32 only). device auto uses Metal only with space charge, at "
        f"least {METAL_MIN_PARTICLES} particles and at least half a particle per space-charge mesh cell; smaller runs are "
        "faster on the CPU. NVIDIA CUDA is not available on macOS.",
    ],
}

GPU_SUPPORT = [
    "Prebuilt CUDA machine code: " + "; ".join(CUDA_ARCHS) + ". Newer GPUs run through the embedded compute_90 PTX "
    "(compiled by the driver; not tested).",
    "Not supported by the prebuilt library: P100 (sm_60), V100 (sm_70), Maxwell and older; device cuda then stops "
    "with the reason, device auto uses the CPU. Rebuilding the library with CMake and CUDA 12.x adds them "
    "(-DCMAKE_CUDA_ARCHITECTURES=60-real;70-real;...; CUDA 13 has no Pascal).",
    "Linux on ARM (Jetson, Kunpeng) is not supported.",
]


def guide(system):
    """Facts for *system* (``platform.system()``: Windows, Linux, Darwin) plus those for every platform."""
    return {"this_platform": PLATFORMS.get(system, ["This platform has no prebuilt kernel; build engine/ with CMake."]),
            "all_platforms": COMMON, "nvidia_gpus": GPU_SUPPORT,
            "other_platforms": {("macOS" if k == "Darwin" else k): v for k, v in PLATFORMS.items() if k != system}}
