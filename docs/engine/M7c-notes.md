# 新内核 M7c 实现记录：CI 构建与比对、预编译库、打包、集群说明

方案见 [M7-plan.md](M7-plan.md) 第 2.5 节与用户决定 2、3、4；M7a 的选择层、源码哈希与库的查找顺序见 [M7-notes.md](M7-notes.md)。
本段在 M7a（0c91567）之上做：三平台的 CI 构建与比对（`engine.yml`）、CUDA 机器码精简、预编译库的提交方式与构建戳检查、打包与更新、Linux 集群说明。
与 M7b（界面）同时进行，文件分工见第 1 节；本段没有往 `avas/engine/new/` 放任何库（第一次提交要等 CI 跑出产物）。

## 1. 改动的文件

| 文件 | 内容 |
|---|---|
| `.github/workflows/engine.yml`（新） | 三平台构建与检查加汇总作业，见第 2 节 |
| `.github/workflows/ci.yml` | Windows python 作业加一步：`avas/engine/new/` 有 Windows 库时用新内核跑短算例（1000 粒子的示例项目）并检查 `avas_run.json` 的 `engine` 字段，没有库时打印“SKIPPED”说明 |
| `engine/CMakeLists.txt` | 默认 CUDA 机器码改为 `61-real;75-real;80-real;86-real;89-real;90`（去掉 sm_60、sm_70，第 3 节）；`OBJCXX_VISIBILITY_PRESET hidden`：Mac 库原先导出 `metal_backend.mm` 的 3 个 C++ 符号，现在只导出 10 个 C 接口 |
| `engine/src/cuda/cuda_backend.cu` | 探针核函数报 `no kernel image` 时，原因里加上“本库含哪些机器码、缺 sm_XY，用 `-DCMAKE_CUDA_ARCHITECTURES=XY-real` 重新构建” |
| `engine/docker/manylinux2014-cuda12.4.Dockerfile` | 基础镜像固定到 `quay.io/pypa/manylinux2014_x86_64:2026.11.06-1`（构建参数 `MANYLINUX_IMAGE`；与 L40S 现有镜像的基础层同一摘要）；NVIDIA 源的主机改为构建参数 `CUDA_MIRROR`（默认官方 `developer.download.nvidia.com`，L40S 用 `.cn`）；rpm 写全版本（CUDA 12.4 是最后一个有 RHEL 7 源的版本，版本不会再变）并检查下载到 10 个包；`curl -f` |
| `engine/tools/build_cuda_windows.bat` | 用 vswhere 找 Visual Studio 2022（任何版本或 Build Tools，限定 17.x，CUDA 12.4 不接受更新的 MSVC），cmake 取自 vcvars 的 PATH，有 Ninja 时用 Ninja（原来 NMake 串行），可选 `AVAS_CMAKE_ARGS`；CI 直接调用它 |
| `engine/tools/ci_engine.py`（新） | 标准库脚本：`exports`（导出符号与 `src/exports.map` 的 C 接口完全一致；ELF / Mach-O 用 nm，PE 由脚本自己解析导出表）、`glibc`（GLIBC ≤ 2.17、GLIBCXX ≤ 3.4.19、CXXABI ≤ 1.3.7）、`deps`（依赖库，PE 解析导入表）、`metal`（有无可用的 Metal 3 设备，写 `$GITHUB_OUTPUT`）、`build-info`、`stamp`、`smoke`；`verify_stamp()` 供 fetch 脚本与设计规则测试共用 |
| `engine/tools/fetch_ci_engine.py`（新） | 用 `gh run download` 取汇总产物（`--run ID` 或分支最近一次成功运行，`--from-dir` 用已下载的目录），校验后放进 `avas/engine/new/`，第 4 节 |
| `tests/test_design_rules.py` | `test_new_engine_libraries_are_current`：`avas/engine/new/` 有库时构建戳的源码哈希必须等于 `engine_source_hash()`、每个库的 SHA-256 与大小必须匹配、不能有未登记的库；没有库时构建戳必须不存在或为空。`test_new_engine_stamp_check`：用假库检查校验本身（通过、源码哈希不同、文件被改、缺文件、缺平台、空构建戳） |
| `tests/test_packaging.py` | `test_spec_bundles_the_new_kernel_of_the_platform`（按 avas.spec 的收集逻辑：Windows 包只带 `avas_engine.dll` 与构建戳）、`test_update_manifest_lists_the_new_kernel`（更新清单含新库）。此文件不在本段原定分工内，改动只是追加两个测试 |
| `packaging/avas.spec` | `avas/engine/new/` 显式收集：本平台的库与 `build-stamp.json`（第 5 节） |
| `pyproject.toml` | `package-data` 加 `new/*.dll`、`new/*.so`、`new/*.dylib`、`new/build-stamp.json`（非 editable 的 `pip install .` 原本不会带上子目录里的库）。不在原定分工内 |
| `.gitattributes` | 加 `*.dylib binary`（原来只靠 Git 的二进制探测）。不在原定分工内 |
| `examples/cluster/avas_slurm.sh`（新） | Slurm 示例：单节点多卡 `avas run --engine new --device cuda --gpus all`，注释里写 MPI 版的做法 |
| `CONTRIBUTING.md` | 中英文“新内核：构建、CI 与预编译库”一节（发布流程、三平台本地构建、manylinux 容器、MPI 版、检查命令、Linux 集群）；英文目录表加 `engine/`、`avas/engine/new/`、`examples/cluster/` |
| `engine/README.md` | Dockerfile 与工具说明、Windows 构建、机器码列表，新增“CI 与预编译库（M7c）”一节 |

## 2. 工作流 `engine.yml`

触发：`workflow_dispatch`；`push` 到 `engine-rebuild` / `main` 且改动 `engine/**` 或工作流本身；`pull_request` 同样的路径。只提交 `avas/engine/new/` 的提交不会再触发它。
权限只读；同一分支的新推送不取消正在跑的运行（PR 才取消），以免产物被半途丢弃。全局 `PYTHONUTF8=1`。

| 作业 | 步骤 | 预计耗时 |
|---|---|---|
| `windows`（`windows-2022`） | 检出；Python 3.11；从 NVIDIA redist 下载 `cuda_nvcc 12.4.131`、`cuda_cudart 12.4.127`、`cuda_cccl 12.4.127` 三个 zip（共约 70 MB，按 `redistrib_12.4.1.json` 的 SHA-256 写死在工作流里校验），解到一个目录；`build_cuda_windows.bat`（VS 2022 + Ninja）；单元测试；`exports` 与 `deps`；`run_all.py --strict`；`tools_check.py` 先用仓库里的 AVAS.dll 跑出参考、再用新库 `--compat dll --ref` 逐字节比较；`run_gpu.py --device emu --fp32`；`build-info`；上传 `engine-windows-x86_64` | CUDA 1–2 min，构建 15–25 min（4 核，3 个 .cu × 6 种机器码 × 动态库与测试静态库两份），run_all 5–8 min，emu 25–40 min；合计约 1 小时，超时 150 min |
| `linux`（`ubuntu-24.04`） | 检出；buildx 构建镜像（`cache-from/to: type=gha`，镜像层在运行之间缓存）；容器里（以 runner 的 uid、`HOME=/tmp`）`build_cuda_linux.sh`、单元测试、`exports`、`glibc`、`deps`、`run_all.py`（容差）、`run_gpu.py --device emu --fp32`、`build-info`；上传 `engine-linux-x86_64` | 镜像首次约 2 min（之后命中缓存），构建 6–10 min，run_all 4–6 min，emu 20–35 min；超时 120 min |
| `macos`（`macos-15`） | 检出；Python 3.11；Apple clang、`CMAKE_OSX_ARCHITECTURES=arm64;x86_64`、`CMAKE_OSX_DEPLOYMENT_TARGET=13.0`；检测 Rosetta；单元测试（arm64，x86_64 在 Rosetta 下）；`exports`、`deps`；arm64 `run_all.py`；x86_64 `run_all.py`（`arch -x86_64 /usr/bin/python3`，系统自带的双架构 Python）；emu；`metal` 检测，可用时 `run_gpu.py --device metal`，否则日志写明跳过、不失败；`build-info` 记录 Metal 与 x86_64 是否检查过；上传 `engine-macos-universal2` | 构建 2–3 min，两次 run_all 5–8 min，emu 15–25 min；超时 120 min |
| `stamp`（`ubuntu-24.04`，依赖前三个） | 检出；下载三个产物；`ci_engine.py stamp --require windows-x86_64,linux-x86_64,macos-universal2`：核对每个文件的 SHA-256、三份源码哈希和提交相同且等于检出的 `engine/`，写 `build-stamp.json`；上传 `avas-engine-new-<源码哈希前 16 位>`（保留 90 天，平台产物 14 天） | 1 min |

`build-info.json`：平台、文件名、SHA-256、大小、源码哈希（库自己报告的必须等于检出的 `engine/`，否则失败——防止用了旧构建）、版本、构建选项（CUDA / Metal / MPI、机器码）、
编译器（C++、CMake 识别的、CUDA）、库报告的平台、runner 镜像（`ImageOS` / `ImageVersion`）、构建时间、各项检查的结果、工作流运行（仓库、运行号、序号、重试次数、地址、提交、分支）。
`build-stamp.json`：schema、源码哈希全长与前 16 位、提交、分支、仓库、工作流运行（含地址）、生成时间，以及每个文件的平台、SHA-256、大小、编译器、机器码与检查结果。

产物 = 三个库 + `build-stamp.json`，正是要提交进 `avas/engine/new/` 的内容。

## 3. CUDA 机器码

CMake 默认从 `60-real;61-real;70-real;75-real;80-real;86-real;89-real;90` 改为 `61-real;75-real;80-real;86-real;89-real;90`（`90` = sm_90 机器码 + compute_90 PTX）。
构建脚本（`build_cuda_linux.sh`、`build_cuda_windows.bat`）都不自己指定机器码、只用 CMake 的默认值，所以三处一致；需要别的 GPU 时传 `-DCMAKE_CUDA_ARCHITECTURES=…`。

注意：PTX 只能向更新的架构 JIT，compute_90 的 PTX 不能在 sm_60 / sm_70 上运行。所以去掉以后 P100、V100 **完全不能**用预编译库的 CUDA（不是“改走 JIT”）：
探针核函数失败，`device=cuda` 时报错（原因含本库的机器码列表与重新构建的写法），`device=auto` 时写进日志后改用 CPU。M6 的说明“加 sm_60、sm_70 使 P100、V100 不必走 JIT”不准确，原因同此。
sm_62/72/87（Jetson）本来就不在目标内；sm_100 / sm_120（Blackwell）用 compute_90 PTX JIT。

体积（L40S 上同一镜像、同一源码，只改机器码；`ls -l` 字节数）：

| 构建 | 改前（8 种） | 改后（6 种） |
|---|---|---|
| Linux `libavas_engine.so`（CUDA，manylinux2014） | 15 846 232 | 12 872 344（−18.8 %） |
| Linux 同上 gzip -9 | 4 958 212 | 4 054 810（−18.2 %） |
| macOS `libavas_engine.dylib`（Metal，无 CUDA） | arm64 1 351 224 | universal2 2 974 264 |
| Windows `avas_engine.dll`（CUDA） | — | 待 CI（预计与 Linux 相近，约 12–14 MB） |

三个平台合计约 30 MB 进仓库（每次更新内核增加这么多）；Windows 安装包只带 `avas_engine.dll`，约增加 13 MB（压缩后更少），远低于决定 3 的 100 MB 门槛，不必拆包。

## 4. 预编译库的提交方式（决定 2）

- `avas/engine/new/`：`avas_engine.dll`、`libavas_engine.so`、`libavas_engine.dylib`、`build-stamp.json`，全部来自同一次 CI 运行。
- `engine/tools/fetch_ci_engine.py`：不给 `--run` 时取当前分支 `engine.yml` 最近一次成功运行；`gh run download --pattern "avas-engine-new-*"`；
  然后要求 ① 每个文件的 SHA-256 与大小等于构建戳、没有未登记的库；② 构建戳的源码哈希等于本地 `engine/` 的哈希（本地 `engine/` 有未提交的改动就对不上，脚本拒绝）；
  ③ 本平台的库在子进程里能加载、报告同一哈希；④ 三个平台齐全（`--allow-partial` 可放宽，只用于试验）；⑤ 给了 `--run` 时构建戳记的运行号一致。
  任何一项不符就退出、不碰目标目录；通过后先删掉旧构建戳登记的文件和三种库名，再复制，最后对目标目录再校验一遍。
- `tests/test_design_rules.py`：见第 1 节。改了 `engine/`（`CMakeLists.txt`、`include/`、`src/`、`third_party/` 的源文件）而库没重新生成，测试失败并提示跑工作流、运行 fetch 脚本。
- **M7 合并 main 前 `avas/engine/new/` 必须有三平台的库**。测试不强制这一点（现在没有库是合法状态）；合并前的检查清单要包括它。
- 发布流程：推送 `engine/` 的改动 → `engine.yml` 成功 → `fetch_ci_engine.py` → `pytest tests/test_design_rules.py` → 提交 `avas/engine/new/`（加变更记录）。
  只改 `avas/engine/new/` 的提交不触发 `engine.yml`，所以不会循环。
- 演示（本地，临时目录，见第 7 节）：用 Mac 的 universal2 构建走一遍 `build-info` → `stamp` → `fetch --from-dir`（缺平台、改一个字节、源码哈希不同三种情况都被拒绝，`--allow-partial` 时安装成功）
  → `ci_engine.py smoke --lib-dir`（真实运行，`engine` 字段六项检查通过）→ 设计规则测试指向该目录通过、改一个字节后失败。

## 5. 打包与更新

- **PyInstaller**：`collect_data_files("avas", subdir="engine")` 会递归收集子目录，但会排除当前平台的扩展名后缀——在 Linux / macOS 上排除 `.so`（以及 `.dylib` 以外的扩展模块），在 Windows 上只排除 `.pyd`。
  所以原来的写法在 Windows 打包时会把 `libavas_engine.so`、`libavas_engine.dylib`（约 16 MB，Windows 上没用）也带进去（原内核的 `libAVAS.so`、`libPIC.so` 现在就是这样）。
  `avas.spec` 改为：`avas/engine/` 其余文件照旧，`avas/engine/new/` 只收本平台的库和 `build-stamp.json`，目标路径 `avas/engine/new`，即冻结后的 `_internal/avas/engine/new/`，
  正是 `avas.paths.NEW_ENGINE_DIR`（`PACKAGE_DIR` 在冻结程序里是 `_internal/avas`，原内核也这样找到）。在 Mac 上用 PyInstaller 6.22.3 的 `collect_data_files`
  对放了假库的包副本核实了三种平台的收集结果；`tests/test_packaging.py` 把这段逻辑固定下来。原内核的 `.so` 仍随 Windows 包分发（不在本段范围）。
- **更新清单**：`update_release.manifest()` 对目录下全部文件求哈希，新库自动进入 `.avas-install.json` 与源码包 / 打包版的清单（`test_update_manifest_lists_the_new_kernel`）。
- **源码 ZIP**：`git archive` 打包已提交的文件，`.gitattributes` 没有 `export-ignore`，`.gitignore` 不忽略 `avas/engine/new/` 下的这些文件（`git check-ignore` 核实），所以提交后自动包含。
- **pip**：`pyproject.toml` 的 `package-data` 补上 `new/` 下的文件。
- **ci.yml 冒烟测试**：`python engine/tools/ci_engine.py smoke`。有库时先校验构建戳与当前源码一致，再把示例项目复制到临时目录、粒子数改为 1000，
  `python -m avas run --engine new --device auto`（环境里去掉 `AVAS_ENGINE*`，用默认查找顺序），检查 `avas_run.json` 的 `engine`：kernel new、库文件就是 `avas/engine/new/` 里的、
  version 等于构建戳、device 为 cpu / cuda / metal 之一、有结果档、DataSet.txt 超过 10 行。CI 的 Windows runner 没有 GPU，auto 应解析为 CPU（CUDA 探针报“没有驱动”）。

## 6. 检查结果

| 检查 | 命令 / 位置 | 结果 |
|---|---|---|
| actionlint 1.7.12 | `actionlint .github/workflows/engine.yml .github/workflows/ci.yml`（`uvx` 被本机的权限规则拦下，改为装进临时 venv 的 `actionlint-py`；本机没有 shellcheck，内嵌脚本未经 shellcheck） | 无问题 |
| ruff 0.6.9 | `uvx ruff@0.6.9 check avas tests packaging tools engine/tests engine/tools` | All checks passed |
| `git diff --check` | 整个工作区 | 无输出 |
| 设计规则与打包测试 | Python 3.11 临时 venv（uv）：`pytest tests/test_design_rules.py tests/test_packaging.py` | 23 passed（含本段新增 4 项；中途一次 `test_web_build_is_current` 失败是 M7b 改了前端尚未重新构建，M7b 构建后通过） |
| Mac 构建（工作流同样参数） | Apple clang 17，`arm64;x86_64`，部署目标 13.0 | 无警告，23 s；`lipo` 两个架构；导出 10 个 C 接口（改 `OBJCXX_VISIBILITY_PRESET` 前多 3 个 C++ 符号） |
| Mac 单元测试 | arm64 与 `arch -x86_64` | 都是 OK (0 failures) |
| Mac run_all | arm64（Anaconda Python）80 s；x86_64（Rosetta，`/usr/bin/python3` 3.9）127 s | 都 rc 0，与 AVAS.dll 最大相对差 3.44e-11（同 M7a）；arm64 切片与非 universal 构建的全部 10 694 个输出逐字节相同；x86_64 切片与 arm64 有 1 563 个文件末位不同（系统数学库），都在容差内 |
| Mac emu fp32 | `run_gpu.py --device emu --fp32` | universal2 构建：fp64 无空间电荷 154/154 与 CPU 逐字节相同，fp32 重跑 184/184、与 emu fp32 184/184，ALL REQUIRED CHECKS PASSED（5 min 34 s，M2 8 核） |
| Mac Metal | `run_gpu.py --device metal` | Apple M2：Metal 与 emu fp32 逐字节相同 184/184，ALL REQUIRED CHECKS PASSED（3 min 56 s）；`ci_engine.py metal` 报告 “Metal 3 device: Apple M2” |
| L40S 镜像 | 改过的 Dockerfile、`--build-arg CUDA_MIRROR=developer.download.nvidia.cn` | 36 s；基础镜像 `2026.11.06-1` 命中已有层（与旧镜像同一摘要 `c387140863a0…`），10 个写全版本的 rpm 下载安装成功；新镜像构建的 `.so` 与旧镜像构建的只差 8 个字节，全在 `.strtab`（局部符号名里 nvcc 临时文件名 `tmpxft_<进程号>` 不同），代码与数据相同——CUDA 构建不是逐字节可重现的，这也是库必须取自 CI、以构建戳登记 SHA-256 的原因（第 9 节待定点 6）；新镜像用完已删 |
| L40S 容器内（工作流 Linux 作业的步骤，不挂 GPU，32 线程） | 构建、单元测试、exports、glibc、deps、run_all、emu、build-info | 构建 73 s 无警告；单元测试 OK（CUDA 构建、无驱动）；导出恰好 10 个 C 接口；GLIBC 2.14、GLIBCXX 3.4.19、CXXABI 1.3.7；run_all 最大差 3.44e-11；emu：fp64 无空间电荷 154/154 与 CPU 逐字节相同，fp32 重跑 184/184、与 emu fp32 184/184，ALL REQUIRED CHECKS PASSED；run_all 139 s、emu 961 s（与他人共用的机器，负载约 50）；`build-info.json` 正常生成 |
| L40S 5 号卡 | `CUDA_VISIBLE_DEVICES=5 run_gpu.py --gpus 0 --fp32 --only 10 个算例` 与设备查询 | 设备查询：L40S（8.9，fp64 非全速，47.7 GB），auto → cuda fp32；fp64 无空间电荷 6/6 与 CPU 逐字节相同，空间电荷 4 个算例重跑相同（SC5、PN1、PN3 与 CPU 也相同）；fp32 重跑 10/10、与 emu fp32 10/10 逐字节相同；ALL REQUIRED CHECKS PASSED（45 s；该卡同时有他人进程占用约 80 %） |
| lyc-pc GTX 1080 Ti（sm_61） | 同上 10 个算例（SC5、SC8、PN1、PN3、Z2、SE4、D_GS_GS、ER7、LB2、SP1），0 号卡 | fp64 无空间电荷 6/6 与 CPU 逐字节相同，空间电荷 4 个算例重跑相同；fp32 重跑 10/10、与 emu fp32 10/10 逐字节相同；ALL REQUIRED CHECKS PASSED；设备查询列出两块 1080 Ti（6.1，fp64 非全速），auto → cuda fp32 |
| 缺机器码时的报错 | 只含 `61-real` 的构建（4.5 MB）在 L40S（sm_89）上查询 | `device=cuda`：error “GPU 0 (NVIDIA L40S) cannot run the engine kernels (no kernel image is available for execution on the device); this library has machine code for 61-real but not for sm_89, build the engine with -DCMAKE_CUDA_ARCHITECTURES=89-real”；auto：cpu fp64，原因同上 |

## 7. 推送后要在 CI 上确认的事项（本地无法验证）

Windows：
1. **CUDA 安装**：redist 三个 zip 合并后的目录能否被 CMake 识别为 CUDA 工具包（`CMAKE_CUDA_COMPILER` + `CUDAToolkit_ROOT`，`lib\x64\cudart_static.lib`、`nvvm\bin\cicc.exe` 的相对位置）。
   不行时的退路：在 zip 之外加 `cuda_nvrtc` / `visual_studio_integration` 等组件，或改用 NVIDIA 网络安装器 `-s nvcc_12.4 cudart_12.4 thrust_12.4`。
2. **批处理**：vswhere 的 `-version [17.0^,18.0^)` 转义、`where ninja`、Ninja 生成器下 nvcc 与 cl.exe 的配合；构建耗时。
3. **MSVC 版本**：`windows-2022` 镜像目前是 VS 17.14（MSVC 19.44），CUDA 12.4 的 `host_config.h` 接受到 19.4x；镜像若升级到 19.5x 会编译失败（那时加 `-allow-unsupported-compiler` 或固定工具集版本）。
4. **strict parity 的环境依赖**：AVAS.dll 参考是在 Windows 11 上生成的；`compat=dll` 用动态 UCRT 的数学函数（`dllmath.cpp` 以 `/fp:fast` 编译调用 CRT 的 sincos），
   runner 是 Windows Server 2022，`ucrtbase.dll` 版本不同。若 strict 因少数末位失败，先看是不是只在数学库敏感的算例上，再决定是否用 XPS 复核（M1–M5 在 XPS 上逐字符一致）。
5. **换行（autocrlf）**：Windows runner 的 Git 默认 `core.autocrlf=true`，`* text=auto` 的文本文件检出为 CRLF。逐个核对过：`compare.py` 读文件时 CRLF → LF；
   `run_engine.py` 用 LF 重写 beam.txt、input.txt、lattice.txt 与内联文件；场图、`.dst`、`.dstb`、`.plt` 有 `binary` 属性或是二进制内容，不转换；
   源码哈希（CMake 与 Python）都先把 CRLF 换成 LF，Windows 的 `build-info` 源码哈希与 Linux / Mac 相同。唯一会以 CRLF 进入内核的输入是
   `tests/parity/data/scf_mesh.txt`（LS1 importSCF 算例）——参考在 XPS 上生成时多半同样是 CRLF；若 LS1 在 strict 下不同，先查这一点（可在工作流里 `git config --global core.autocrlf false` 后再检出对照）。
6. **AVAS.dll 在 runner 上加载**（tools_check 的参考）：需要仓库里的 fftw DLL 与 runner 自带的 VC++ 运行库（MSVCP140、VCOMP140）；ci.yml 的 smoke 测试已经在 windows-latest 上加载过它。
7. **编码与路径**：`PYTHONUTF8=1`；工作目录在 `D:\a\AVAS_XMU\engine-work\…`，最长路径约 150 字符，不需要长路径支持。
8. **DLL 依赖**：`deps` 会列出 `avas_engine.dll` 的导入（预计 MSVCP140、VCRUNTIME140(_1)、api-ms-win-crt-*、KERNEL32）。见第 9 节待定点 1。

Linux：`docker/build-push-action` 的 gha 缓存（首次运行写入约 1.5 GB，是否包含基础镜像层）；以 runner uid 运行容器时检出目录可读写。

macOS：`macos-15` 上 Rosetta 与 `/usr/bin/python3` 是否可用（不可用时日志说明并跳过 x86_64）；Metal 在虚拟机里几乎肯定不可用（日志写“SKIPPED”，`build-info` 记“skipped”）；
runner 的 Apple clang 版本（Xcode 15/16）与本机 17 不同，结果在容差内即可。

汇总：`download-artifact` 的 `pattern` 把三个产物各放一个子目录；`stamp` 的 `name` 输出用于产物名。

## 8. 与方案不同或方案未写明的地方

1. **Windows runner 固定为 `windows-2022`**（方案写 windows-latest）：发布库的编译器由镜像决定，`-latest` 改指向新镜像时会悄悄换 MSVC，甚至超出 CUDA 12.4 支持的范围。Linux 用 `ubuntu-24.04`（编译在容器里，宿主版本无关紧要）。
2. **CUDA 用 NVIDIA 的 redist 压缩包**而不是安装器或第三方 action：只下三个组件（约 70 MB），版本与 SHA-256 写死，无需管理员安装，几十秒完成，不需要缓存。
3. **macOS 做 universal2**（arm64 + x86_64，部署目标 macOS 13）：多 1.6 MB；x86_64 切片在 Intel Mac 上同样带 Metal，运行时探测 Metal 3（AMD 5000/6000、Vega、Intel UHD 630 / Iris Plus 等支持），没有就用 CPU。
   CI 在 Rosetta 下跑 x86_64 的单元测试与 run_all。两个切片的数值末位不同（各自的系统数学库），与 M7a 决定“同一源码哈希、同一档的不同构建共用研究缓存”一致，但要知道 Intel Mac 与 Apple 芯片的 CPU 结果并非逐字节相同。
4. **Windows 包只带 Windows 的新库**：方案写“PyInstaller 本来就收集 `avas/engine/` 下的全部文件”，那样会多带约 16 MB 无用的 Linux / macOS 库。
5. **Windows 作业另做 DLL 工具比对**：仓库里有 AVAS.dll，runner 能加载，`tools_check.py` 先跑出 AVAS.dll 的结果再逐字节比较，不需要预存参考。
6. **ci_engine.py 借用 `avas.engines.engine_source_hash`**（把仓库根目录加入 `sys.path`），没有第三份哈希实现；因此 `avas/engines.py` 与 `avas/paths.py` 必须保持只依赖标准库（CI 的容器与汇总作业里没有装 numpy 等）。

## 9. 新的待定点（附推荐）

1. **Windows 新库的 C++ 运行库**。CMake 默认 `/MD`，`avas_engine.dll` 依赖 MSVCP140.dll / VCRUNTIME140.dll。打包版里 PyInstaller 会带上；源码安装的用户若没装 VC++ 运行库，
   而 `MultiParticleEngine._load` 只把库自己的目录（`avas/engine/new/`）加入 DLL 搜索路径——原内核目录里的 MSVCP140.dll 不在其中，新库可能加载失败。
   不能简单改成 `/MT`：静态 CRT 有自己的 stdout 缓冲，`_unbuffer_stdout` 对 UCRT 共享 stdout 的处理就失效了（GUI 进度会一块一块地到）。
   推荐：先看 CI 的 `deps` 输出；若确有 MSVCP140，**由 M7b / 选择层在加载新库时也把 `avas/engine/` 加入 DLL 搜索路径**（那里已有 MSVCP140.dll，改动最小）；
   备选是“混合 CRT”（静态 vcruntime / msvcp + 动态 UCRT：`/MT` 加 `/NODEFAULTLIB:libucrt.lib ucrt.lib`），库自足且保留共享 stdout，但要在 CI 上重新确认 strict parity。
2. **原内核的 `.so` 进 Windows 包**：`libAVAS.so`、`libPIC.so`、`libfftw3*.so*` 现在都被带进 Windows 安装包（几 MB 到十几 MB）。推荐另开一项，在 spec 里按平台排除；本段不动（不属于新内核）。
3. **CI 时长**：emu fp32 全套是每个作业最慢的一步（CPU 一遍、emu fp64 两遍、emu fp32 三遍）。推荐先保持全套；若作业超过一小时，给 `run_gpu.py` 加一个只跑一遍 emu fp32 的轻量选项，或 PR 上只跑子集、推送时跑全套。
4. **三平台 emu fp32 的跨平台一致性**：fp32 档在同一台机器的各设备上逐字节相同；跨平台（MSVC / GCC / Apple clang）是否也相同尚未比较。推荐：各作业把 emu fp32 输出的摘要写进 `build-info.json`，
   汇总作业只报告不判失败，先积累数据。
5. **Mac 部署目标 13.0**：AVAS 的其余部分没有更高要求（Python 3.11 官方安装包支持更旧的 macOS），13.0 只因为 Metal 3。推荐保持 13.0（Metal 后端需要它）；需要支持更旧 macOS 时再给 Metal 加运行时可用性检查并降到 11.0。
6. **CUDA 构建不可逐字节重现**：同一镜像、同一源码两次构建的 `.so` 差 8 个字节（`.strtab` 里的 nvcc 临时文件名）。推荐暂不处理——库取自 CI 并登记 SHA-256 已经足够；
   若以后想让任何人都能复现出同一个文件，可在发布构建里去掉静态符号表（链接时 `-s`，Linux 库还会小约 0.5 MB）。

## 10. 给 AGENTS.md 的规则（由协调者合并）

- 第 2 节目录地图：`engine/tools/ci_engine.py`（CI 检查与构建记录）、`engine/tools/fetch_ci_engine.py`（取 CI 产物）、`examples/cluster/`（Slurm 示例）、`.github/workflows/engine.yml`（新内核三平台构建与比对）；`avas/engine/new/` 一行注明“CI 构建、人工提交，带 build-stamp.json”。
- 第 5.4 节（或新内核一节）加一条：**新内核的预编译库**：`avas/engine/new/` 的三个库只能来自 `.github/workflows/engine.yml` 的同一次成功运行，用 `python engine/tools/fetch_ci_engine.py [--run ID]` 校验后放入，
  与 `build-stamp.json` 一起提交；不要提交本地构建的库，不要手改构建戳。`tests/test_design_rules.py` 检查构建戳的源码哈希等于 `engine_source_hash()`、文件 SHA-256 匹配，
  所以改了 `engine/`（CMakeLists.txt、include/、src/、third_party/ 的源文件）就要重新跑工作流并 fetch。M7 合并 main 前三个平台的库必须齐全。
  CUDA 机器码只在 `engine/CMakeLists.txt` 定义（默认 sm_61/75/80/86/89/90 + compute_90 PTX，构建脚本不另写）；Windows 作业固定 `windows-2022`（发布库的 MSVC 由镜像决定，CUDA 12.4 不接受 MSVC 19.5x）。
  `ci_engine.py` 借用 `avas.engines.engine_source_hash`，因此 `avas/engines.py`、`avas/paths.py` 只能依赖标准库。`packaging/avas.spec` 只把本平台的新库和构建戳打进安装包。

## 11. 临时数据

- Mac：`$TMPDIR/claude-…/scratchpad/m7c/`（四份构建：arm64、universal2，两次 run_all、两次 emu、Metal 的工作目录，构建戳 / fetch / smoke 演示目录，PyInstaller 收集检查用的包副本，
  CUDA redist 清单，Python 3.11 venv）结束时已删除；`ci_engine.py smoke` 的临时项目在系统临时目录、运行后自动删除。uv 的全局缓存里留有建 venv 下载的包。
  worktree 的 `engine/build`（开发者自己的构建）按要求未动：它是改 `CMakeLists.txt` / `cuda_backend.cu` 之前构建的，`tests/test_engines.py::test_library_reports_the_source_hash_of_this_tree` 要重新构建后才会通过。
- L40S：`/data/public/lycself/m7c_tmp`（两份源码、改前 / 改后 / 新镜像 / 只含 sm_61 四份构建、容器内检查与 5 号卡检查的工作目录）、测试镜像 `avas-engine-build:m7c-test` 及其构建缓存记录已删除；
  旧镜像 `avas-engine-build:cuda12.4-manylinux2014` 保留。GPU 只用 5 号卡，CPU 用 `--cpuset-cpus 0-31` / `taskset -c 0-31`。
- lyc-pc：`~/m7c_tmp`（库、测试脚本、示例输入、工作目录）已删除，只用 0 号卡。

## 12. 推送后的 CI 结果（2026-11-11）

第一次推送后逐项修正：
- **源码哈希脚本**：`cmake -P` 的脚本模式下策略全未设置，CMake 3.31（Windows 镜像）把 `IN_LIST` 当普通单词，构建第一步失败；本机 CMake 4 没有旧行为所以没暴露。脚本里 `cmake_policy(SET CMP0057 NEW)`，CMake 3.31 下哈希与 Python 一致。
- **macOS 镜像**：GitHub 已不提供 `macos-14`，改 `macos-15`。
- **导出符号**：Windows 的 CUDA 静态运行库自带导出 `NvOptimusEnablementCuda`（双显卡笔记本上让驱动为本进程启用独显，device=cuda 需要），`ci_engine.py exports` 对 `.dll` 允许它。
- **产物路径**：`upload-artifact` 不接受含 `..` 的路径，Windows / macOS 的 `BUILD`、`WORK`、`CUDA` 改为 `${{ github.workspace }}-engine-build` 这类同级绝对路径。
- **strict parity**：Windows Server 2022 上 compat=dll 与参考有 8 个算例的 `.dst`（少数粒子末位，矩完全相同）和 LT1 的 BeamSet.plt / PCHistogram.dat 不同，DataSet 等文本全部逐字符一致。参考是 Windows 11 上 AVAS.dll 写的，compat=dll 与 AVAS.dll 一样调用动态 UCRT 的数学函数（第 7 节第 4 条预计的情况）。`run_all.py` 新增 `--rerun-original AVAS.dll`：严格比对有差别时，在同一台机器上用原 AVAS.dll 重跑这些算例；新内核与本机 AVAS.dll 逐字节相同即通过并逐项写明“environment”，本机 AVAS.dll 仍能复现参考而新内核不能则失败。Mac 上用新内核 compat=dll 充当“原内核”自测：97 个算例 269 处差别全部判为环境差别、通过。失败时上传各算例摘要（产物 `parity-windows-summaries`，保留 7 天）。

第 7 节确认点：
1. CUDA redist 目录：CMake 识别正常（`Found CUDAToolkit … 12.4.131`），构建约 5 分钟。
2. 批处理、vswhere、Ninja：正常。
3. MSVC：镜像为 19.44.35229，CUDA 12.4 接受。
4. strict parity：见上，用本机 AVAS.dll 复核。
6. DLL 依赖：`avas_engine.dll` 导入 MSVCP140、VCRUNTIME140、VCRUNTIME140_1、api-ms-win-crt-*、KERNEL32。VCRUNTIME140(_1) 随 python.org 的 Python 3.11 与 PyInstaller 打包提供（加载内核的总是 Python 进程），MSVCP140 由 `avas/engine/` 自带并已加入搜索路径，第 9 节待定点 1 按现状关闭。
- Linux：全部通过（单元测试、只导出 C 接口、glibc ≤ 2.17、与参考 1e-10 以内、emu fp64 / fp32 184 个算例逐字节可重复），库 12.9 MB；镜像层缓存第二次命中。
- macOS：构建、Rosetta 下 x86_64 单元测试与比对、arm64 比对、emu 全部通过；Metal 在虚拟机里不可用（“Apple Paravirtual device does not support Metal 3”），按设计跳过并记入 build-info；库 2.97 MB。
- 告警：`actions/checkout@v4` 等仍以 Node.js 20 为目标，GitHub 已强制改用 Node.js 24 运行，目前只是告警。
- 第四次运行（42973557438，提交 f7423ab）全部通过。Windows strict：8 个算例 18 处与参考不同（`.dst` 少数粒子末位、LT1 的 BeamSet.plt / PCHistogram.dat），在 runner 上重跑 AVAS.dll 后全部与新内核逐字节相同；其余输出直接与参考逐字符一致；DLL 工具逐字节一致。Windows 库 11.3 MB。
- `fetch_ci_engine.py --run 42973557438` 校验后装入 `avas/engine/new/`（dll 6cacb6bc…、so 8cc96be7…、dylib 3382681f…）；macOS 库与第二次运行的构建逐字节相同。Mac 上默认改用这个库，`test_design_rules.py` 等通过。

## 13. 第 9 节待定点的决定与实施（2026-11-14，用户“全部按推荐”）

1. VC 运行库：保持现状（`avas/engine/` 加入搜索路径，第 12 节已由 CI 的 `deps` 确认）。
2. Windows 安装包不再带原内核的 Linux 文件：`packaging/avas.spec` 在 win32 上排除 `*.so`、`*.so.*`（libAVAS.so、libPIC.so、libfftw3.so*）；Linux 包仍带 AVAS.dll（原内核身份）。
   `tests/test_packaging.py` 覆盖。自动更新按清单移除旧包里的这些文件。
3. CI 时长：保持全套。
4. 三平台 emu fp32：各平台作业 `ci_engine.py build-info --fp32-dir <emu>/emu_fp32` 把每个算例 fp32 输出的摘要（去掉计时、种子与时间戳，文本文件 CRLF → LF）写进 build-info.json；
   汇总作业在 build-stamp.json 写 `fp32_cross_platform`（平台、算例数、逐字节相同数、不同的算例），日志打印，不判失败。
5. macOS 部署目标保持 13.0。
6. CUDA 构建不可逐字节重现：暂不处理。

第一次记录（CI 运行 43613162331，源码哈希 bb045561）：184 个算例中 58 个三平台逐字节相同；两两比较 Linux = Windows 90 个、Linux = macOS 64 个、macOS = Windows 58 个。
不同的主要是初始分布算例（`D_*` 25 个）和带 RF 场的算例：fp32 档的初始化与场计算调用各平台数学库的 `sinf` / `expf` 等，末位不同后经跟踪放大。fp32 档在同一平台各设备间逐字节相同的保证不变，
跨平台差别属于正常的单精度舍入，按决定只记录。预编译库取自该运行（dll 89ee1eeb…、so b13a6e4d…、dylib e2c030a8…）。
