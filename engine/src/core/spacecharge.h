// Space charge (spec 60, PICNIC spec 65): mesh in the beam rest frame, CIC deposit, FFT (pocketfft),
// SPICNIC or PICNIC solver, interpolation back to the particles (scE = (Ex/gamma, Ey/gamma, Ez), B = 0).
#pragma once
#include <array>
#include <vector>

#include "core/beam.h"
#include "core/kernels.h"
#include "core/params.h"

namespace avas {

double fft_norm(int nx, int ny, int nz, bool compat);

class SpaceCharge {
public:
    SpaceCharge(const Params& p, int nParticles);
    // One solve with the current beam (rel, centroid, rms, gamma); writes b.scx/scy/scz.
    // factor = space-charge factor of spacechargecomp.  Returns false when SPICNIC skipped the
    // step (autoPIC), in which case the particles keep their previous field.
    bool solve(Beam& b, double factor, const double* sync = nullptr);
    int nx() const { return nx_; }
    // impscfield 1: fields from a recorded run (importSCF) instead of a solve
    bool importing() const { return imp_; }
    void import_scf(Beam& b);

    // ---- pieces of solve() for a device back end (corrected engine only) ----
    // autoPIC test and counters; false: skip this solve (the particles keep their field).
    bool begin(const Beam& b, const double* sync);
    // Mesh of this solve (cell sizes, node coordinates, centre) from the beam rms and centroid.
    void mesh(const Beam& b);
    // View of the mesh for the particle kernels (Q = total charge, set after the deposit).
    kern::MeshV mesh_view(const Beam& b) const;
    // Fixed-point exponent of the corrected deposit (charge * 2^e is an exact 64-bit integer).
    long long fix_exp() const { return fixExp_; }
    void prepare_fix(const Beam& b) { init_fix(b); }
    bool recording() const { return rec_ == 1; }
    // rho from the summed fixed-point deposit, times the space-charge factor.
    void set_rho(const std::vector<long long>& acc, double factor);
    int method() const { return method_; }
    // SPICNIC: Green table, threshold and the asymmetric correction list (host part of spicnic()).
    void spicnic_prepare(std::vector<int>& corr, std::vector<double>& dval, double& thr);
    // SPICNIC threshold negligibleratio * rho(centre) and the cell volume of this solve (device solve).
    double spicnic_threshold() const { return negligible_ * rho_[at(nx_ / 2, ny_ / 2, nz_ / 2)]; }
    double cell_volume() const { return vol_; }
    // FFT: eigenvalue tables lx, ly, lz and the normalisation (host part of fft()).
    void fft_tables(std::vector<double>& lx, std::vector<double>& ly, std::vector<double>& lz, double& scale,
                    double& norm) const;
    double total_charge() const;
    // recscfield 1 after a device solve (ex/ey/ez filled by the caller).
    void record_after(const Beam& b, double Q) { if (rec_ == 1) record(b, Q); }
    std::vector<double>& rho() { return rho_; }
    std::vector<double>& ex() { return ex_; }
    std::vector<double>& ey() { return ey_; }
    std::vector<double>& ez() { return ez_; }
    const std::vector<double>& green() const { return G_; }
    const std::vector<double>& xs() const { return xs_; }
    const std::vector<double>& ys() const { return ys_; }
    const std::vector<double>& zs() const { return zs_; }
    int ny() const { return ny_; }
    int nz() const { return nz_; }
    int mx() const { return mx_; }
    int my() const { return my_; }
    int mz() const { return mz_; }

private:
    bool compat_;
    int method_;  // 0 FFT, 1 SPICNIC, 2 PICNIC (libPIC.so)
    int nx_, ny_, nz_;
    double mr_[3];
    double negligible_;
    int lp_;
    int rec_;  // recscfield 1: append the mesh field to SpaceChargeSet.plt / SpaceChargeMesh.txt
    bool pool_;  // multithreading 1: AVAS.dll task partitions
    std::array<int, 10> cores_{};
    int mx_, my_, mz_, r2_;
    std::vector<int> wBounds_, aBounds_;  // particle partitions (Weight, Allocate)
    std::vector<std::array<int, 4>> sTasks_;  // SPICNIC target segments {is, js, ie, je}
    // mesh
    double dx_ = 0, dy_ = 0, dz_ = 0, vol_ = 0, gamma_ = 1, igamma_ = 1;
    std::vector<double> xs_, ys_, zs_;
    std::vector<double> rho_, phi_, ex_, ey_, ez_;
    std::vector<double> G_;  // potential kernel (2Nx+1)(2Ny+1)(2Nz+1)
    std::vector<std::array<int, 3>> cell_;
    std::vector<std::array<double, 8>> w_;
    std::vector<int> out_;
    std::vector<long long> fix_;  // corrected deposit (fixed point)
    long long fixExp_ = 0;
    bool fixInit_ = false;
    double prevRms_[3] = {0, 0, 0};
    long long calls_ = 0, solves_ = 0;

    size_t at(int i, int j, int k) const { return (static_cast<size_t>(i) * ny_ + j) * nz_ + k; }
    size_t gat(int a, int b, int c) const {
        return (static_cast<size_t>(a) * (2 * ny_ + 1) + b) * (2 * nz_ + 1) + c;
    }
    void weight(Beam& b, double factor);
    void init_fix(const Beam& b);
    void fft();
    void spicnic();
    void picnic();
    void tabulate();
    void allocate(Beam& b);
    void record(const Beam& b, double Q);
    double sync_[6] = {0, 0, 0, 0, 0, 0};
    bool imp_ = false;
    int inx_ = 0, iny_ = 0, inz_ = 0;
    std::vector<double> itime_, izc_, iQ_, igam_;
    std::vector<std::array<double, 2>> ixy_;
    std::vector<std::array<double, 3>> id_;
    std::string iset_;  // SpaceChargeSet.plt of the input folder
};

// z-code: rows {x, ux, y, uy, dt, uz, t, flag} at one z -> rel coordinates at time tRef, centroid
// and rms of the live particles (MethodMP::HESCFieldPre).
void hesc_field_pre(Beam& b, const std::vector<std::array<double, 8>>& rows, double tRef, int threads);

// Default numerical parameters of the space charge solver (spec 60 2.1.3).
void space_charge_defaults(Params& p, long long n);

}  // namespace avas
