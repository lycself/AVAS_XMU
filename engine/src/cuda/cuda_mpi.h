// Optional MPI layer of the CUDA back end (CMake AVAS_WITH_MPI).  Every rank runs the whole host
// program with the same inputs; the particles are divided into whole reduction groups (reduce.h)
// over the ranks and their GPUs.  The ranks exchange the per-group partial sums (all-gather, then
// every rank adds them in group order), the fixed-point space-charge mesh (integer sum) and, where
// the host needs them, the particle arrays -- so the results do not depend on the number of ranks.
// Without AVAS_WITH_MPI every function here is a no-op for a single process.
#pragma once
#include <cstdlib>
#include <vector>

#if defined(AVAS_WITH_MPI)
#include <mpi.h>
#endif

namespace avas {
namespace cu {

struct MpiWorld {
    int rank = 0, size = 1, localRank = 0;
};

#if defined(AVAS_WITH_MPI)
inline void mpi_finalize_at_exit() {
    int fin = 0;
    MPI_Finalized(&fin);
    if (!fin) MPI_Finalize();
}

inline const MpiWorld& mpi_world() {
    static MpiWorld w;
    static bool done = false;
    if (done) return w;
    done = true;
    int inited = 0;
    MPI_Initialized(&inited);
    if (!inited) {
        int prov = 0;
        MPI_Init_thread(nullptr, nullptr, MPI_THREAD_FUNNELED, &prov);
        std::atexit(mpi_finalize_at_exit);
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &w.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &w.size);
    MPI_Comm local;
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, w.rank, MPI_INFO_NULL, &local);
    MPI_Comm_rank(local, &w.localRank);
    MPI_Comm_free(&local);
    return w;
}

// In-place all-gather: rank r owns elements [off[r], off[r] + cnt[r]) of data.
template <class T>
inline void mpi_allgatherv(T* data, const std::vector<long long>& off, const std::vector<long long>& cnt) {
    const MpiWorld& w = mpi_world();
    if (w.size <= 1) return;
    std::vector<int> c(cnt.size()), d(off.size());
    for (size_t r = 0; r < cnt.size(); ++r) {
        c[r] = static_cast<int>(cnt[r] * static_cast<long long>(sizeof(T)));
        d[r] = static_cast<int>(off[r] * static_cast<long long>(sizeof(T)));
    }
    MPI_Allgatherv(MPI_IN_PLACE, 0, MPI_BYTE, data, c.data(), d.data(), MPI_BYTE, MPI_COMM_WORLD);
}

inline void mpi_allreduce_sum(long long* v, long long n) {
    if (mpi_world().size <= 1) return;
    MPI_Allreduce(MPI_IN_PLACE, v, static_cast<int>(n), MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
}

// Variable-length lists of every rank, concatenated in rank order.
template <class T>
inline void mpi_allgather_list(std::vector<T>& v) {
    const MpiWorld& w = mpi_world();
    if (w.size <= 1) return;
    int mine = static_cast<int>(v.size() * sizeof(T));
    std::vector<int> c(static_cast<size_t>(w.size)), d(static_cast<size_t>(w.size));
    MPI_Allgather(&mine, 1, MPI_INT, c.data(), 1, MPI_INT, MPI_COMM_WORLD);
    long long tot = 0;
    for (int r = 0; r < w.size; ++r) {
        d[static_cast<size_t>(r)] = static_cast<int>(tot);
        tot += c[static_cast<size_t>(r)];
    }
    std::vector<T> all(static_cast<size_t>(tot / static_cast<long long>(sizeof(T))));
    MPI_Allgatherv(v.data(), mine, MPI_BYTE, all.data(), c.data(), d.data(), MPI_BYTE, MPI_COMM_WORLD);
    v.swap(all);
}
#else
inline const MpiWorld& mpi_world() {
    static MpiWorld w;
    return w;
}
template <class T>
inline void mpi_allgatherv(T*, const std::vector<long long>&, const std::vector<long long>&) {}
inline void mpi_allreduce_sum(long long*, long long) {}
template <class T>
inline void mpi_allgather_list(std::vector<T>&) {}
#endif

}  // namespace cu
}  // namespace avas
