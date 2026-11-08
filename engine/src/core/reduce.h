// Fixed-order summation of per-particle contributions (corrected engine, CPU and GPU).
//
// Every statistic that is a sum over particles is evaluated in one fixed order that depends only on
// the particle index, never on the thread count, the scheduling, the device or the number of GPUs:
//   1. particles in chunks of kRedChunk = 256 consecutive indices: a pairwise tree inside the chunk,
//      for s = 128, 64, ..., 1: a[t] = a[t] + a[t + s] (missing particles count as +0.0);
//   2. chunks in groups of kRedGroupChunks = 16 (4096 particles): the same tree over the 16 chunk sums;
//   3. groups summed one after the other in index order, starting from 0.0.
// A GPU block of 256 threads computes level 1 in shared memory with exactly the same pattern; level 2
// is a second small kernel; level 3 runs on the host.  Several GPUs (or MPI ranks) hold whole groups,
// so the result is also independent of how the particles are distributed.
// Order-independent reductions (minimum, maximum, integer counts) need no fixed order.
#pragma once
#include "core/hd.h"

namespace avas {

constexpr int kRedChunk = 256;
constexpr int kRedGroupChunks = 16;
constexpr int kRedGroup = kRedChunk * kRedGroupChunks;  // 4096

// In-place pairwise tree of n (a power of two) values; the sum ends up in a[0].
AVAS_HD inline void tree_sum(double* a, int n) {
    for (int s = n / 2; s >= 1; s >>= 1)
        for (int t = 0; t < s; ++t) a[t] = a[t] + a[t + s];
}

// Level 1 and 2 for the group that starts at particle g0 (a multiple of kRedGroup) on the host.
// fill(i, c) writes the K contributions of particle i (i < n) to c.
template <int K, class Fill>
inline void group_sum(int g0, int n, Fill&& fill, double out[K]) {
    double chunks[K][kRedGroupChunks];
    double buf[K][kRedChunk];
    double c[K];
    for (int ch = 0; ch < kRedGroupChunks; ++ch) {
        const int base = g0 + ch * kRedChunk;
        if (base >= n) {
            for (int k = 0; k < K; ++k) chunks[k][ch] = 0.0;
            continue;
        }
        for (int t = 0; t < kRedChunk; ++t) {
            const int i = base + t;
            if (i < n) {
                fill(i, c);
                for (int k = 0; k < K; ++k) buf[k][t] = c[k];
            } else {
                for (int k = 0; k < K; ++k) buf[k][t] = 0.0;
            }
        }
        for (int k = 0; k < K; ++k) {
            tree_sum(buf[k], kRedChunk);
            chunks[k][ch] = buf[k][0];
        }
    }
    for (int k = 0; k < K; ++k) {
        tree_sum(chunks[k], kRedGroupChunks);
        out[k] = chunks[k][0];
    }
}

// Level 3: group sums added in index order.
template <int K>
inline void sum_groups(const double* groups, int nGroups, double out[K]) {
    for (int k = 0; k < K; ++k) out[k] = 0.0;
    for (int g = 0; g < nGroups; ++g)
        for (int k = 0; k < K; ++k) out[k] = out[k] + groups[static_cast<long long>(g) * K + k];
}

inline int red_groups(int n) { return (n + kRedGroup - 1) / kRedGroup; }

// Fixed-order sum over particles [0, n) on the host, groups computed by run(nGroups, fn) (in any
// order, possibly in parallel).
template <int K, class Run, class Fill>
inline void fixed_sum(int n, Run&& run, Fill&& fill, double out[K]) {
    const int ng = red_groups(n);
    double* groups = new double[static_cast<size_t>(ng > 0 ? ng : 1) * K];
    run(ng, [&](int g) { group_sum<K>(g * kRedGroup, n, fill, groups + static_cast<long long>(g) * K); });
    sum_groups<K>(groups, ng, out);
    delete[] groups;
}

// Rounds to the nearest integer, ties to even (|v| < 2^62); the same result on every device.
AVAS_HD inline long long round_to_ll(double v) {
    long long q = static_cast<long long>(v);
    const double f = v - static_cast<double>(q);
    if (f > 0.5 || (f == 0.5 && (q & 1))) q += 1;
    else if (f < -0.5 || (f == -0.5 && (q & 1))) q -= 1;
    return q;
}

}  // namespace avas
