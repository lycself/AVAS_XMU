FROM quay.io/pypa/manylinux2014_x86_64
RUN curl -sL https://developer.download.nvidia.cn/compute/cuda/repos/rhel7/x86_64/cuda-rhel7.repo \
      | sed 's#developer.download.nvidia.com#developer.download.nvidia.cn#g' > /etc/yum.repos.d/cuda-rhel7.repo \
 && mkdir /tmp/rpms && cd /tmp/rpms \
 && yumdownloader --disablerepo="*" --enablerepo="cuda*" \
      cuda-nvcc-12-4 cuda-crt-12-4 cuda-nvvm-12-4 cuda-cudart-12-4 cuda-cudart-devel-12-4 cuda-cccl-12-4 \
      cuda-driver-devel-12-4 cuda-toolkit-config-common cuda-toolkit-12-config-common cuda-toolkit-12-4-config-common \
 && ls && rpm -ivh --nodeps *.rpm && cd / && rm -rf /tmp/rpms /var/cache/yum
ENV PATH=/usr/local/cuda-12.4/bin:$PATH
