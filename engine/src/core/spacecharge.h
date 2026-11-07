// Space charge (spec 60): mesh in the beam rest frame, CIC deposit, FFT (pocketfft) or SPICNIC
// solver, interpolation back to the particles (scE = (Ex/gamma, Ey/gamma, Ez), B = 0).
#pragma once
#include <array>
#include <vector>

#include "core/beam.h"
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

private:
    bool compat_;
    int method_;  // 0 FFT, 1 SPICNIC
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
    double prevRms_[3] = {0, 0, 0};
    long long calls_ = 0, solves_ = 0;

    size_t at(int i, int j, int k) const { return (static_cast<size_t>(i) * ny_ + j) * nz_ + k; }
    size_t gat(int a, int b, int c) const {
        return (static_cast<size_t>(a) * (2 * ny_ + 1) + b) * (2 * nz_ + 1) + c;
    }
    void weight(Beam& b, double factor);
    void fft();
    void spicnic();
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
