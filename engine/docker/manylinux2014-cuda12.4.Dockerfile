# Build image of the Linux engine library: manylinux2014 (CentOS 7, glibc 2.17, GCC 10 devtoolset)
# plus the parts of the CUDA 12.4 tool kit that nvcc needs (no driver, no libraries besides cudart).
#
#   docker build -f engine/docker/manylinux2014-cuda12.4.Dockerfile -t avas-engine-build:cuda12.4-manylinux2014 engine/docker
#
# Build arguments:
#   MANYLINUX_IMAGE  the base image, pinned to a dated tag so that CI and local builds use the same
#                    compiler and CMake (quay.io/pypa/manylinux2014_x86_64 publishes one tag per rebuild)
#   CUDA_MIRROR      host of NVIDIA's package repository: CI uses the official host; machines that cannot
#                    reach it (e.g. in mainland China) pass --build-arg CUDA_MIRROR=developer.download.nvidia.cn
# CUDA 12.4 is the last release with a RHEL 7 repository, so the package versions below are final.
ARG MANYLINUX_IMAGE=quay.io/pypa/manylinux2014_x86_64:2026.11.06-1
FROM ${MANYLINUX_IMAGE}
ARG CUDA_MIRROR=developer.download.nvidia.com
RUN curl -fsSL https://${CUDA_MIRROR}/compute/cuda/repos/rhel7/x86_64/cuda-rhel7.repo \
      | sed "s#developer.download.nvidia.com#${CUDA_MIRROR}#g" > /etc/yum.repos.d/cuda-rhel7.repo \
 && mkdir /tmp/rpms && cd /tmp/rpms \
 && yumdownloader --disablerepo="*" --enablerepo="cuda*" \
      cuda-nvcc-12-4-12.4.131 cuda-crt-12-4-12.4.131 cuda-nvvm-12-4-12.4.131 cuda-cudart-12-4-12.4.127 \
      cuda-cudart-devel-12-4-12.4.127 cuda-cccl-12-4-12.4.127 cuda-driver-devel-12-4-12.4.127 \
      cuda-toolkit-config-common-12.4.127 cuda-toolkit-12-config-common-12.4.127 cuda-toolkit-12-4-config-common-12.4.127 \
 && ls && test "$(ls *.rpm | wc -l)" = 10 && rpm -ivh --nodeps *.rpm && cd / && rm -rf /tmp/rpms /var/cache/yum
ENV PATH=/usr/local/cuda-12.4/bin:$PATH
