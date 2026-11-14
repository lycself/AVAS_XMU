// Particle back end of the tracker on an accelerator (CUDA in src/cuda/; Metal later).
//
// The tracker keeps all control flow, the synchronous particle, the space-charge mesh set-up and
// every output file on the host.  The back end holds the macro particles (indices 0 .. n-1) and runs
// the per-particle work of kernels.h on them; sums over particles come back as the per-group
// partial sums of reduce.h, so the host finishes every statistic in the same fixed order as the CPU
// engine.  Host copies of the particle arrays are refreshed explicitly with download() / upload()
// where the tracker reads or changes them (output planes, restarts, diagnostics).
#pragma once
#include <memory>
#include <string>
#include <vector>

#include "core/beam.h"
#include "core/kernels.h"
#include "core/latview.h"
#include "core/secondary.h"

namespace avas {

// Particle arrays for download() / upload().
enum DevMask : unsigned {
    kDevCoord = 1u,     // x, ux, y, uy, z, uz
    kDevField = 2u,     // ex .. bz
    kDevSc = 4u,        // scx, scy, scz
    kDevRel = 8u,       // rel[6]
    kDevLost = 16u,     // lost
    kDevIdx = 32u,      // elemIndex, nextPlane, boundaryMode, geomIndex, spOutCount
    kDevSpz = 64u,      // spOutZ0, spOutZ1
    kDevOld = 128u,     // coordinates before the last step
    kDevRec = 256u,     // plane crossing records
    kDevSpRec = 512u,   // superposeout crossing records
    kDevAll = 1023u
};

// Result of one step for one reduction group (4096 particles).
struct StepGroup {
    double s[kern::kStepSums];
    double sp, rec;  // superposeout crossings, flagged crossing records
    double mx[6];
    double zmin, zmax;
};

struct LatScalars {
    int spOutCur, spOutCount, spOutSize;
    double totalLength, spExit;
};

// One boundary.txt hit (or a missing shape parameter, code < 0).
struct BoundaryHit {
    int i;
    int code;
    int geom;
    double P[6];
    double tHit;
};

// Host data of one space-charge solve on the device.  The Green table of SPICNIC / PICNIC and the
// SPICNIC correction list are computed on the device (sc_kernels.h, the same bits as the CPU engine).
struct ScSolveIn {
    int method;  // 0 FFT, 1 SPICNIC, 2 PICNIC
    int nx, ny, nz, mx, my, mz;
    const double* rho;
    double dx, dy, dz;
    // SPICNIC / PICNIC: cell volume (dy dx dz, as on the host) and the SPICNIC threshold
    double vol;
    double thr;
    // FFT
    const double* lx;
    const double* ly;
    const double* lz;
    double scale, norm;
};

// Secondary particles (secondary.h): host arrays of the segments and the results of the device.
struct SeHost {
    std::vector<kern::SeSegV> segs;
    std::vector<double> ev;
    std::vector<int> evMark, evSlot, el, elMark, elSlot;
    std::vector<kern::SeElemV> elems;
    int nSlots = 0;
};
struct SeOut {
    std::vector<double> planes;  // per particle and output slot: S[0..7] (S[7] = 1 when written)
    std::vector<BoundaryHit> hits;
};

class DeviceBackend {
public:
    virtual ~DeviceBackend() = default;
    // One line for the log: devices, precision and why.
    virtual std::string describe() const = 0;
    virtual bool fp32() const = 0;

    // Allocates the particle arrays (n macro particles), uploads them and the lattice.
    virtual void begin_run(Beam& b, const LatHost& L, bool spRec) = 0;
    // Element table again (element rotMode changes inside superposeout blocks).
    virtual void set_elems(const LatHost& L) = 0;
    virtual void download(Beam& b, double* old, double* rec, double* spRec, unsigned mask) = 0;
    virtual void upload(const Beam& b, const double* old, const double* rec, const double* spRec, unsigned mask) = 0;
    virtual void zero_rec(bool sp) = 0;

    // TcodeTrans of all macro particles; out = one entry per reduction group (red_groups(n)),
    // spCount = superposeout crossings, recCount = flagged crossing records after the step.
    virtual void step(const LatScalars& ls, const kern::StepArgs& a, const std::vector<double>& rf,
                      const std::vector<double>& events, std::vector<StepGroup>& out, long long& spCount,
                      long long& recCount) = 0;
    // UpdateBeamParameters: pass 1 sets rel and gives the sums of kern::ubp_pass1 (fixed order),
    // pass 2 the rms sums and the extent in z of all particles.
    virtual void ubp1(const double* s6, double* sums) = 0;
    virtual void ubp2(const double* cen, double igz, double* q3, double& zmin, double& zmax) = 0;

    // z-code rows
    virtual void rows(const kern::RowOp& op) = 0;
    virtual void rows_tref(bool average, double& tRef) = 0;
    virtual void rows_kick(int kind, double wx, double wy, double len) = 0;
    // sync: the synchronous particle after zTotcode {x, ux, y, uy, z, uz} (the fp32 tier keeps the
    // particles relative to it)
    virtual void ztot(double zPlane, double tRef, const double* sync) = 0;
    virtual void rec_tmin(bool sp, double& tmin) = 0;
    virtual void hesc1(double tRef, double* sums) = 0;
    virtual void hesc2(const double* cen, double gc, double* q3) = 0;

    // space charge: deposit (fixed point), solve, interpolation
    virtual void sc_deposit(const kern::MeshV& M, std::vector<long long>& acc) = 0;
    virtual void sc_solve(const ScSolveIn& in) = 0;
    virtual void sc_fields(std::vector<double>& ex, std::vector<double>& ey, std::vector<double>& ez) = 0;
    virtual void sc_interp(const kern::MeshV& M) = 0;

    // boundary.txt, lpboundary, longlimits
    virtual void boundary(const LatScalars& ls, double t, double dt, std::vector<BoundaryHit>& hits) = 0;
    virtual void lp_fold(double zs, double full, double half) = 0;
    virtual void long_loss(double zs, double ws, double dW, double dzMax) = 0;

    // secondary particles: every particle alone through the lattice (SeRun)
    virtual void secondary(const LatScalars& ls, const SeHost& se, const kern::SeV& v, SeOut& out) = 0;
    virtual void download_times(Beam& b) = 0;  // own time t (secondary particles)
};

// MPI (AVAS_WITH_MPI, GPU runs only): every rank runs the whole host program; rank 0 writes the
// outputs, the others work in a private folder.  {0, 1} without MPI.
struct ProcessRank {
    int rank = 0;
    int size = 1;
};
ProcessRank process_rank();

// The device a run gets from the options (device / precision / gpus / compat): plan_device() is the
// only place that decides it, for main_agent (make_device_backend) and for the C API query
// avas_engine_devices() (resolution of "auto" without running a simulation).
struct DevicePlan {
    std::string device = "cpu";      // "cpu", "cuda", "metal" or "emu"
    std::string precision = "fp64";  // "fp64" or "fp32" (Metal: always fp32)
    std::vector<int> gpus;           // CUDA devices of the run
    std::string reason;              // why ("requested", "auto: ...")
    // the same as a fixed code for programs (avas_engine_devices / avas_engine_last_run "reason_code"):
    // requested; auto_space_charge (Metal), auto_fp64_full_rate, auto_fp32 (CUDA); CPU fallbacks of auto:
    // compat_dll, no_space_charge, no_metal, no_cuda_build, no_driver, driver_too_old, no_gpu, bad_gpus,
    // gpu_too_old, no_kernel_image, gpu_unusable, gpu_memory, run_time_failure.  Empty with an error.
    std::string code;
    std::string error;               // not empty: the run fails with this message (device required, unusable)
    std::string desc;                // CUDA: the selected GPUs for the log line; Metal: the GPU name
    bool spaceCharge = false;        // make_device_backend: the run it was set up for has space charge
};
// nParticles < 1: no memory estimate (query without a run).  The CUDA probe creates a context on
// every selected GPU.
DevicePlan plan_device(long long nParticles, bool spaceCharge);

// Device chosen for a run (options device / precision / gpus, environment AVAS_ENGINE_DEVICE /
// AVAS_ENGINE_PRECISION / AVAS_ENGINE_GPUS).  Returns null for the CPU; throws EngineError when a
// GPU was required and cannot be used.  nParticles is used for the memory estimate.  The device
// actually used (after a fallback of "auto" to the CPU) is kept for avas_engine_last_run().
std::unique_ptr<DeviceBackend> make_device_backend(long long nParticles, bool spaceCharge);
// The last device make_device_backend() set up (plan.device empty: none since reset_used_device()).
const DevicePlan& used_device();
void reset_used_device();

// Only the CUDA build.  cuda_plan: driver, GPUs of the option gpus, their kernels (probe), the
// memory estimate (nParticles > 0) and the precision; false and a reason when no usable GPU.
// On failure plan.code holds the reason code (see DevicePlan::code).
bool cuda_plan(long long nParticles, bool spaceCharge, DevicePlan& plan, std::string& why);
// The back end of a plan from cuda_plan (null and a reason when it cannot be set up).
std::unique_ptr<DeviceBackend> make_cuda_backend(const DevicePlan& plan, std::string& why);
// CUDA devices of this machine for the C API query (empty without CUDA support or driver).
struct CudaDeviceInfo {
    int index = 0;
    std::string name;
    int major = 0, minor = 0;
    bool fp64Full = false;  // HPC part with fp64 at half the fp32 rate (precision auto: fp64)
    double memoryGB = 0.0;  // total memory, GB = 1e9 B
};
std::vector<CudaDeviceInfo> cuda_devices(std::string& why);
// Space-charge mesh work on an accelerator without fp64 (Metal, M6c phase 1): the particles stay in
// the device back end of the CPU (emu, fp64) and only the deposit, the PICNIC / SPICNIC sums and the
// interpolation run on the accelerator in fp32.  x, y, z are the rel coordinates of the particles.
class ScAccel {
public:
    virtual ~ScAccel() = default;
    virtual std::string describe() const = 0;
    // Adds the fixed-point deposit of n particles to acc (64-bit integer sums, order independent).
    virtual void deposit(const kern::MeshV& M, int n, const double* x, const double* y, const double* z,
                         const double* q, const int* lost, std::vector<long long>& acc) = 0;
    // The mesh solve on the accelerator from the charge in.rho (PICNIC / SPICNIC: Green table, correction
    // list, sums and gradient there); the field stays there (interp_state with null arrays, fields() on the
    // host).  false: not handled, the caller solves on the CPU.
    virtual bool solve_mesh(const ScSolveIn& in) = 0;
    virtual void fields(std::vector<double>& ex, std::vector<double>& ey, std::vector<double>& ez) = 0;
    // Field at the particles from the mesh field ex, ey, ez (sc = 0 for lost particles).
    virtual void interp(const kern::MeshV& M, const double* ex, const double* ey, const double* ez, int n,
                        const double* x, const double* y, const double* z, const int* lost, double* scx, double* scy,
                        double* scz) = 0;

    // ---- particle push on the accelerator (Metal phase 2, fp32 relative to the synchronous
    // particle).  After a step the accelerator's float state is the current one ("authoritative")
    // until to_host() writes it back into the fp64 arrays P of the device back end. ----
    virtual void begin_run(const LatHost& L, int n) { (void)L; (void)n; }
    // The t-code step on the accelerator; false: not supported for this lattice (the caller steps
    // on the CPU).  P: the fp64 arrays (old / rec are written there, in double, by the host).
    virtual bool step(const LatScalars& ls, const kern::StepArgs& a, const std::vector<double>& rf,
                      const std::vector<double>& events, const kern::PartV& P, int n, std::vector<StepGroup>& out,
                      long long& spCount, long long& recCount) {
        (void)ls; (void)a; (void)rf; (void)events; (void)P; (void)n; (void)out; (void)spCount; (void)recCount;
        return false;
    }
    virtual bool authoritative() const { return false; }
    // Writes the float state into P; the accelerator stays authoritative (the host only reads P)
    // until invalidate() says that the host changed P.
    virtual void to_host(const kern::PartV& P, int n) { (void)P; (void)n; }
    virtual void invalidate() {}
    // rec / spRec flags cleared by the tracker (zero_rec)
    virtual void rec_cleared(bool sp) { (void)sp; }
    // boundary.txt on the accelerator (authoritative only); false: the caller does it on the CPU.
    virtual bool boundary(const LatScalars& ls, double t, double dt, std::vector<BoundaryHit>& hits) {
        (void)ls; (void)t; (void)dt; (void)hits;
        return false;
    }
    // element table changed (superposeout rotMode)
    virtual void set_elems(const LatHost& L) { (void)L; }

    // ---- z-code rows on the accelerator (M6c phase 4; authoritative only, false: the caller does it on
    // the CPU).  The rows are the fp64 records P.rec in float; after row work they are newer than
    // P.rec until rec_to_host(). ----
    virtual bool rows(const kern::RowOp& op, const kern::PartV& P, int n) { (void)op; (void)P; (void)n; return false; }
    virtual bool rows_tref(bool average, const kern::PartV& P, int n, double& tRef) {
        (void)average; (void)P; (void)n; (void)tRef;
        return false;
    }
    virtual bool rows_kick(int kind, double wx, double wy, double len, const kern::PartV& P, int n) {
        (void)kind; (void)wx; (void)wy; (void)len; (void)P; (void)n;
        return false;
    }
    virtual bool hesc1(double tRef, const kern::PartV& P, int n, double* sums) { (void)tRef; (void)P; (void)n; (void)sums; return false; }
    virtual bool hesc2(const double* cen, double gc, double* q3) { (void)cen; (void)gc; (void)q3; return false; }
    virtual bool ztot(double zPlane, double tRef, const double* sync, const kern::PartV& P, int n) {
        (void)zPlane; (void)tRef; (void)sync; (void)P; (void)n;
        return false;
    }
    virtual bool ubp1(const double* s6, double* sums) { (void)s6; (void)sums; return false; }
    virtual bool ubp2(const double* cen, double igz, double* q3, double& zmin, double& zmax) {
        (void)cen; (void)igz; (void)q3; (void)zmin; (void)zmax;
        return false;
    }
    virtual void rec_to_host(const kern::PartV& P, int n) { (void)P; (void)n; }
    // lpboundary / longlimits on the accelerator (authoritative only; false: the caller does it)
    virtual bool lp_fold(double zs, double full, double half) { (void)zs; (void)full; (void)half; return false; }
    virtual bool long_loss(double zs, double ws, double dW, double dzMax) {
        (void)zs; (void)ws; (void)dW; (void)dzMax;
        return false;
    }
    // the host changed P.rec (upload): the float rows are rebuilt before their next use
    virtual void invalidate_rows() {}
    // Deposit / interpolation from the float state (authoritative only).
    virtual void deposit_state(const kern::MeshV& M, int n, std::vector<long long>& acc) { (void)M; (void)n; (void)acc; }
    // (ex null: the field of the last solve_mesh)
    virtual void interp_state(const kern::MeshV& M, const double* ex, const double* ey, const double* ez, int n) {
        (void)M; (void)ex; (void)ey; (void)ez; (void)n;
    }
};

// Test device "emu" (device_emu.cpp): the device back end on the CPU; with an accelerator for the
// space-charge mesh work (Metal).
std::unique_ptr<DeviceBackend> make_emu_backend(std::unique_ptr<ScAccel> accel = nullptr);
// Metal (src/metal/, AVAS_WITH_METAL): the accelerator, or null and a reason.
std::unique_ptr<ScAccel> make_metal_accel(std::string& why);
// The fp32 tier of the emu device (precision fp32): the host logic of the fp32 GPU back ends
// (core/f32_host.cpp) with their kernels run on the CPU (core/f32_cpu.cpp); every build.
std::unique_ptr<ScAccel> make_f32_cpu_accel(std::string& why);
bool metal_compiled();
// The Metal device (Metal 3 needed) without compiling the kernels: false and a reason when there is none.
bool metal_probe(std::string& name, std::string& why);
// The CUDA fp32 tier (src/cuda/cuda_f32_device.cu): the accelerator on the given GPUs.
std::unique_ptr<ScAccel> make_cuda_f32_accel(const std::vector<int>& devs, const std::string& desc, std::string& why);
// Compiled with CUDA support?
bool cuda_compiled();

}  // namespace avas
