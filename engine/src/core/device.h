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

// Host data of one space-charge solve on the device.
struct ScSolveIn {
    int method;  // 0 FFT, 1 SPICNIC
    int nx, ny, nz, mx, my, mz;
    const double* rho;
    double dx, dy, dz;
    // SPICNIC
    const double* G;
    double thr;
    const int* corr;  // I, J, K per entry
    const double* dval;
    long long ncorr;
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
    virtual void ztot(double zPlane, double tRef) = 0;
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

// Device chosen for a run (options device / precision / gpus, environment AVAS_ENGINE_DEVICE /
// AVAS_ENGINE_PRECISION / AVAS_ENGINE_GPUS).  Returns null for the CPU; throws EngineError when a
// GPU was required and cannot be used.  nParticles is used for the memory estimate.
std::unique_ptr<DeviceBackend> make_device_backend(long long nParticles, bool spaceCharge);

// Only the CUDA build: the back end itself (null and a reason when no usable GPU).
std::unique_ptr<DeviceBackend> make_cuda_backend(long long nParticles, bool spaceCharge, bool required,
                                                 std::string& why);
// Test device "emu" (device_emu.cpp): the device back end on the CPU.
std::unique_ptr<DeviceBackend> make_emu_backend();
// Compiled with CUDA support?
bool cuda_compiled();

}  // namespace avas
