// Toolchain probe for the AVAS engine rebuild (phase 0). Not part of the product.
// Checks: multi-arch SASS selection, static cudart on old/new drivers,
// fp32/fp64 throughput, and that one __host__ __device__ function gives the
// same answer on CPU (OpenMP) and GPU.
#include <cuda_runtime.h>
#include <omp.h>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#define CK(x) do { cudaError_t e_ = (x); if (e_ != cudaSuccess) { \
  std::printf("CUDA error %s at %s:%d\n", cudaGetErrorString(e_), __FILE__, __LINE__); std::exit(1);} } while (0)

// Shared physics-like kernel body: leapfrog in a nonlinear focusing field.
template <class T>
__host__ __device__ inline void push(T& x, T& p, int steps, T dt) {
  for (int s = 0; s < steps; ++s) {
    p -= T(0.5) * dt * (x + T(0.1) * x * x * x);
    x += dt * p;
    p -= T(0.5) * dt * (x + T(0.1) * x * x * x);
  }
}

template <class T>
__global__ void push_kernel(T* x, T* p, int n, int steps, T dt) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < n) push(x[i], p[i], steps, dt);
}

template <class T>
__global__ void fma_kernel(T* out, int iters) {
  T a = T(threadIdx.x) * T(1e-3), b = T(0.999999), c = T(1e-7);
  T a2 = a + T(1), a3 = a + T(2), a4 = a + T(3);
  for (int i = 0; i < iters; ++i) {
    a = a * b + c; a2 = a2 * b + c; a3 = a3 * b + c; a4 = a4 * b + c;
  }
  out[blockIdx.x * blockDim.x + threadIdx.x] = a + a2 + a3 + a4;
}

template <class T>
double fma_gflops(int sms) {
  int blocks = sms * 32, threads = 256, iters = 4096;
  T* d; CK(cudaMalloc(&d, sizeof(T) * blocks * threads));
  fma_kernel<T><<<blocks, threads>>>(d, 16); CK(cudaDeviceSynchronize());
  cudaEvent_t a, b; cudaEventCreate(&a); cudaEventCreate(&b);
  cudaEventRecord(a);
  fma_kernel<T><<<blocks, threads>>>(d, iters);
  cudaEventRecord(b); CK(cudaEventSynchronize(b));
  float ms; cudaEventElapsedTime(&ms, a, b);
  cudaFree(d);
  return 2.0 * 4 * double(blocks) * threads * iters / (ms * 1e-3) / 1e9;
}

template <class T>
double compare_cpu_gpu(int n, int steps) {
  std::vector<T> x(n), p(n), xg(n), pg(n);
  for (int i = 0; i < n; ++i) { x[i] = T(1e-3) * (i % 1000); p[i] = T(0); }
  T *dx, *dp; CK(cudaMalloc(&dx, n * sizeof(T))); CK(cudaMalloc(&dp, n * sizeof(T)));
  CK(cudaMemcpy(dx, x.data(), n * sizeof(T), cudaMemcpyHostToDevice));
  CK(cudaMemcpy(dp, p.data(), n * sizeof(T), cudaMemcpyHostToDevice));
  push_kernel<T><<<(n + 255) / 256, 256>>>(dx, dp, n, steps, T(1e-2));
  CK(cudaGetLastError());
  CK(cudaMemcpy(xg.data(), dx, n * sizeof(T), cudaMemcpyDeviceToHost));
  #pragma omp parallel for
  for (int i = 0; i < n; ++i) push(x[i], p[i], steps, T(1e-2));
  double worst = 0;
  for (int i = 0; i < n; ++i) {
    double d = std::fabs(double(x[i]) - double(xg[i])) / (std::fabs(double(x[i])) + 1e-30);
    if (d > worst) worst = d;
  }
  cudaFree(dx); cudaFree(dp);
  return worst;
}

int main() {
  int rt = 0, drv = 0, n = 0;
  cudaRuntimeGetVersion(&rt); cudaDriverGetVersion(&drv);
  std::printf("runtime %d.%d (static)  driver supports %d.%d  omp threads %d\n",
              rt / 1000, rt % 1000 / 10, drv / 1000, drv % 1000 / 10, omp_get_max_threads());
  cudaError_t e = cudaGetDeviceCount(&n);
  if (e != cudaSuccess) { std::printf("no usable GPU: %s -> CPU fallback path\n", cudaGetErrorString(e)); return 0; }
  for (int d = 0; d < n; ++d) {
    CK(cudaSetDevice(d));
    cudaDeviceProp pr; CK(cudaGetDeviceProperties(&pr, d));
    int ratio = 0; cudaDeviceGetAttribute(&ratio, cudaDevAttrSingleToDoublePrecisionPerfRatio, d);
    cudaFuncAttributes fa; CK(cudaFuncGetAttributes(&fa, fma_kernel<double>));
    std::printf("[dev %d] %s  sm_%d%d  SMs %d  mem %.1f GB  fp32:fp64 ratio %d  kernel binary sm_%d ptx %d\n",
                d, pr.name, pr.major, pr.minor, pr.multiProcessorCount, pr.totalGlobalMem / 1e9,
                ratio, fa.binaryVersion, fa.ptxVersion);
    std::printf("        fp32 %.0f GFLOPS   fp64 %.0f GFLOPS\n",
                fma_gflops<float>(pr.multiProcessorCount), fma_gflops<double>(pr.multiProcessorCount));
    std::printf("        CPU vs GPU same code, max rel diff: fp64 %.2e  fp32 %.2e\n",
                compare_cpu_gpu<double>(1 << 20, 2000), compare_cpu_gpu<float>(1 << 20, 2000));
  }
  return 0;
}
