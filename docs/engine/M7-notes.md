# 新内核 M7 实现记录：接入软件

方案与用户的 8 个决定见 [M7-plan.md](M7-plan.md)。M7 分三段：**M7a 选择层、C 接口补充、运行记录与研究指纹**（本节）、
M7b 界面（模拟设置页“计算内核”分组、运行页 / 结果页标签、PICNIC 提示、scanphase、关于窗口、Mac 上核对界面、README 与使用说明）、
M7c CI 与预编译库（`engine.yml`、`avas/engine/new/` 与构建戳检查、打包、Linux 集群说明）。默认仍用原内核，没有选新内核的用户行为不变。

## M7a：选择层、C 接口、运行记录与研究指纹

### 1. 改动的文件

| 文件 | 内容 |
|---|---|
| `engine/src/core/device.{h,cpp}` | 新增 `DevicePlan` 与 `plan_device()`：按选项（device / precision / gpus / compat）和有无空间电荷决定设备、精度、GPU 列表、原因或错误；`make_device_backend()` 先调用它，再构造后端，并记下实际用的设备（`used_device()`，含 auto 回落到 CPU 的情况） |
| `engine/src/cuda/cuda_backend.cu` | `make_cuda_backend` 拆成 `cuda_plan()`（驱动、卡、探针核函数、显存估计、auto 精度）与按计划构造后端；新增 `cuda_devices()`（卡的序号、名称、算力、fp64 档、显存）。日志文字与原来逐字相同 |
| `engine/src/metal/metal_backend.mm`、`core/metal_stub.cpp`、`core/device_stub.cpp` | `metal_probe()`（Metal 3 设备，不编译着色器）；无 CUDA / Metal 构建的空实现 |
| `engine/src/core/engine.{h,cpp}` | `LastRun`：每次 `main_agent(1)` 返回时记下实际设备、精度、GPU、compat、线程数、是否有空间电荷、耗时与返回值（`main_agent(2)` 的停止调用不改它） |
| `engine/src/capi.cpp`、`include/avas_engine.h`、`src/exports.map` | 新导出 `avas_engine_info()`、`avas_engine_devices()`、`avas_engine_last_run()`（JSON，调用线程的缓冲区）；`avas_engine_version()` 返回源码哈希前 16 位；头文件注释列出全部 `avas_set_option` 键与环境变量。Windows 经头文件的 `AVAS_API` 导出，Linux 版本脚本加入三个名字（L40S 上 `nm -D` 核实共 10 个导出） |
| `engine/cmake/source_hash.cmake`（新）、`engine/CMakeLists.txt` | 引擎源码哈希（第 3 节），构建时生成 `generated/avas_source_hash.h`；配置时写 `generated/avas_build_config.h`（CUDA 机器码、编译器、构建类型） |
| `engine/tests/unit_tests.cpp` | `test_capi_queries`：三个查询的 JSON 可解析（测试内置一个小 JSON 读取器）且字段齐全；cuda / metal 在没有对应支持的构建中给出错误计划；compat=dll 时 auto 为 CPU；一次 300 粒子的漂移段运行后 `last_run` 为 cpu / fp64 / threads 2 / 有空间电荷 / 状态 0，一次失败的运行（缺 lattice）记为无设备、非 0 状态 |
| `avas/engines.py`（新） | 选择层：`EngineChoice`、`resolve_choice`（优先级）、`validate`、库的查找、`file_sha1`、`engine_source_hash`、`result_tier`、`input_space_charge`、`describe`（`avas info`）、`query_subprocess`（子进程预判 auto）、`predicted_profile`、`run_record`（avas_run.json 的 `engine`） |
| `avas/core/MultiParticleEngine.py` | 按 `EngineChoice` 加载原内核或新内核；新内核在构造和每次 `main_agent(1)` 前用 `avas_set_option` 设置全部选项，`check_device()` 在运行前拒绝本机没有的 cuda / metal；`NewLibrary` / `load_new_library` 只做查询 |
| `avas/cli/main.py` | `avas run` 的 `--engine --device --precision --gpus --threads --compat`（`--device cpu|gpu` 仍是原内核的旧通路）；运行前检查设备、输出目录在检查之后才建；`avas_run.json` 的 `engine` 字段；`avas info [--input DIR] [--engine-json]` 与 `avas doctor` 列出两个内核、设备与 auto 的解析；`EngineChoiceError` 以 `error: …` 退出（码 2），没有异常栈 |
| `avas/paths.py` | `NEW_ENGINE_DIR = avas/engine/new` |
| `avas/utils/iniconfig.py` | `read_engine_settings` / `write_engine_settings`（只改 `[engine]` 的键，保留其余段和不认识的键，值经 `parse_value` 校验与规范化）；`IniConfig` 读写保留 `[engine]`，`set_param(engine=…)` 可改它；`[engine]` 没有默认值，所以没有该段的项目不会被写入一段 |
| `avas/research/study.py`、`batch.py`、`engine.py` | 研究层：`Study(engine=, device=, precision=, gpus=, compat=, threads=, engine_query=)`；指纹的内核身份改为结果档（第 5 节）；每次运行的 `input/ini.ini` 写入解析后的 `[engine]`，子进程环境去掉 `AVAS_ENGINE*` 选择变量；运行后核对实际档；GPU 并行绑卡（决定 8） |
| `tests/test_engines.py`（新）、`tests/fake_research_engine.py`、`tests/test_research.py` | 第 6 节；假内核读取运行的 `[engine]` 并写 avas_run.json 的 `engine`，fp32 档 rms x 大 0.1 %（另一档的结果），`fakeauto` 关键字模拟 auto 的实际结果 |
| `AGENTS.md`、`engine/README.md` | 目录地图（`engine/`、`avas/engine/new/`、`avas/engines.py`）；第 5.4 节“内核选择”；研究接口一条的指纹说明 |

### 2. C 接口的设计

- **auto 的解析只有一处**：`plan_device()`。`main_agent` 经 `make_device_backend()` 调用它（带粒子数，做显存估计），`avas_engine_devices()` 调用它两次
  （有 / 无空间电荷，粒子数 0 即不估显存）。`"auto"` 是把 device 临时换成 auto 的结果，`"selected"` 是当前选项本身的结果（cuda / metal 不可用时给 `error`）。
  CUDA 的查询会在所选的卡上建立上下文并运行探针核函数（与运行前的检查相同）。Metal 只查设备是否支持 Metal 3，着色器编译失败只能在运行时发现：
  显式 metal 时运行报错，auto 时与原来一样在日志写明后用 CPU，`avas_engine_last_run()` 记下实际的 CPU。
- 查询函数先调用 `init_options_from_env()`（devices），所以环境变量照常是初始值，`avas_set_option` 之后的值优先。
- 返回的文本在调用线程自己的 `thread_local` 缓冲区里，直到同一线程再次调用同一函数前有效；不要在另一线程运行 `main_agent` 时调用（devices 会临时改 device 选项）。
- 精度的报告：CPU 恒为 fp64；Metal 恒为 fp32（与 `precision` 无关，这与原来的运行行为一致——`run_gpu.py --device metal` 设 `precision=fp64` 也得到 fp32 档）；
  emu 按 `precision`。Python 层把 “metal + fp64”“cpu + fp32”“compat dll + cuda / metal / emu” 当作无效组合在运行前报错，库本身不改行为。
- 日志里的 `Device: …` 行与 M6 逐字相同（文字仍在原处生成）。

### 3. 引擎源码哈希

输入：`engine/CMakeLists.txt`、`engine/include/**`、`engine/src/**`，以及 `engine/third_party/**` 中扩展名为 .h .hpp .c .cc .cpp .cu .cuh .mm .metal .inc 的文件
（“构建实际用到的源文件”按扩展名判断：现在只有 `pocketfft_hdronly.h`，许可证文件不进）；任何一级以 “.” 开头的名字跳过（.DS_Store 等）。
按相对路径（`/` 分隔，按字节）排序，每个文件 CRLF → LF，哈希 = SHA-256(Σ "相对路径\n" + 文件 SHA-256 十六进制 + "\n")，库报告全长（`source_hash`）与前 16 位（`version`）。

CMake 侧（`cmake/source_hash.cmake`）：文件列表在配置时用 `file(GLOB_RECURSE … CONFIGURE_DEPENDS)` 得到（新增文件触发重新扫描），
自定义命令依赖全部被哈希的文件，每次有改动就以 `cmake -P` 重新计算，只在哈希变化时重写头文件（`capi.cpp` 只在那时重编），用一个时间戳文件避免每次构建都运行。
Python 侧 `avas.engines.engine_source_hash(root)` 同一算法；`tests/test_engines.py` 在临时目录里核对规则（CRLF、隐藏文件、third_party 过滤、tests/ 不进），
有 cmake 时与 `cmake -P` 的结果比较，有 `engine/build` 的库时比较库报告的哈希（经子进程 `avas info --engine-json`）。cmake 脚本本身不在哈希里（它定义算法，改它要同步改 Python）。

### 4. 选择层与各运行入口

优先级、默认值和组合检查见 `avas/engines.py` 文件头与 AGENTS.md 第 5.4 节。`avas run` 的 `--device` 同时承担两种含义：新内核时是 auto / cpu / cuda / metal，
原内核时仍是旧的 cpu / gpu（gpu = libPIC.so）；原内核下给新内核才有的值（auto、cuda、metal 或 `--precision` 等）直接报错并提示 `--engine new`，
新内核下给 `--device gpu` 报错并提示 `--device cuda`。环境变量与 ini.ini 中的其余键在原内核下不校验、忽略。

| 入口 | 如何拿到 `[engine]` |
|---|---|
| 完整运行（运行页） | `runner.py` 以项目 InputFile 为 `--input`，`avas run` 读其中 ini.ini |
| 误差研究（`--mode stat / dyn / stat_dyn`，一次进程内多次 `main_agent`） | 同上；`_run` 解析一次，`engines.set_active()` 让每个 `MultiParticle` 新建的 `MultiParticleEngine` 都用它（库只加载一次，选项每次运行前重设）；avas_run.json 记最后一次运行 |
| 分段运行 | `segments.py` 的 `_write_inputs` 复制文本输入（含 ini.ini）到每个阶段的目录 |
| 研究层 `Study` | 在本进程按“参数 > 环境 > 项目 ini”解析，写进每次运行的 `input/ini.ini [engine]`；`research.engine.run` 的子进程去掉 `AVAS_ENGINE*` 选择变量，使 ini 成为唯一来源（`AVAS_ENGINE_LIB` 保留）；GUI 研究页经 `Sandbox.run_in`，同样读这份 ini |
| AI 沙盒、参数扫描（`avas scan`、旧 Scans/） | `Sandbox` 复制项目文本输入（含 ini.ini）；环境照常继承 |
| 入门教程 | 复制示例项目的 ini.ini（没有 `[engine]`）→ 原内核；没有项目也不出错 |

### 5. 研究层的结果档

- 结果档：`original`、`new-cpu-fp64`、`new-cuda-fp64`（CUDA fp64，以及在 CPU 上复现设备后端的 emu fp64）、`new-fp32`（CUDA / Metal / emu fp32 逐字节相同，M6c）、
  `new-compat-dll`。指纹的 `engine` 部分：原内核仍是 `research.engine.identity()` 的 SHA-1 字符串（与 M7 前完全相同，原内核的旧缓存继续有效），
  新内核是 `{"tier", "version"}`。ini.ini 参与文件摘要时去掉 `[engine]` 段，所以线程数、绑定的 GPU 不进指纹。
- auto 在运行前解析成具体档：`engines.query_subprocess()` 在子进程里运行 `avas info --engine-json` 加完整的选择参数（环境变量去掉），按（库路径、大小、修改时间、选择）缓存在进程内；
  有无空间电荷按该运行的 input.txt `spacecharge`、`secondarybeam` 与 beam.txt 的流强、粒子数判断（与内核相同的规则）。
- 运行后读 `output/avas_run.json` 的 `engine`，实际档写进 run.json 的 `engine.actual`；与预判不符时记 `warnings` 并写日志（缓存条目仍按预判归档）。
- 设备不能用（例如显式 cuda 而没有驱动）时 `Study.engine` 直接抛 `EngineChoiceError`，不记录也不缓存（换台机器就可能能跑）；GUI 研究页把它显示为该行的错误。
- 决定 8：`run_many(workers > 1)` 先问 `Study.parallel_plan()`：CUDA（含 auto 解析为 CUDA）时每个 worker 从队列里取一张卡写进该运行的 `[engine] gpus`，
  worker 数不超过卡数；一张卡或 Metal 时降为 1 并写日志；CPU 照常并行。`gpus` 不进指纹（多卡与单卡结果相同，M6a）。

### 6. 检查结果（Mac M2，2026-11-11）

构建：Apple clang 17（含 Metal）与 conda clang 14（不含 Metal）两种都无警告；作为对照，从 HEAD（375787c）`git archive` 出的 engine/ 以同样两种编译器各构建一份。

| 检查 | 命令 | 结果 |
|---|---|---|
| 单元测试 | `avas_engine_unit` | 两种构建 OK (0 failures) |
| parity | `run_all.py --lib … --work …`（两种构建与两份对照） | 全部 rc 0；与 AVAS.dll 最大相对差 3.44e-11（与对照相同）。新构建与同编译器对照的全部输出逐字节相同：Apple 10 322 个文件、conda 10 322 个文件，0 个不同（只排除带时间戳的 log / console / synParticle / SimulationTime / ErrorLog / errorData.seed） |
| Metal 全套 | `run_gpu.py --lib … --device metal` | Metal 与 emu fp32 逐字节相同 184/184，ALL REQUIRED CHECKS PASSED；与对照的 20 628 个输出文件 0 个不同 |
| 物理检验 | `avas_engine_physics --metal` | ELLIPSOID CHECK OK，输出与对照相同 |
| CUDA（L40S 5 号卡，manylinux2014 + CUDA 12.4 容器，sm_89） | 构建、`avas_engine_unit`、`run_gpu.py --gpus 0 --fp32 --only`（7 个算例：SC5、SC8、PN1、Z2、SE4、D_GS_GS、ER7） | 构建无警告，单元测试 OK；ALL REQUIRED CHECKS PASSED（fp64 无空间电荷与 CPU 逐位相同，fp32 重跑与 emu fp32 7/7）；与 HEAD 对照的 1 236 个输出文件 0 个不同 |
| L40S 上的查询与运行 | `avas info --engine new`、`avas run --engine new --threads 16` | 见第 7 节 |

pytest、端到端与 `AVAS_ENGINE=new` 下的界面测试见第 8 节。

### 7. 设备查询的实际输出

Mac（Apple clang 构建）：`cuda.devices` 为空（note “this engine was built without CUDA”），`metal.devices` = Apple M2（usable），CPU 8 线程；
auto：有空间电荷 → metal fp32，无空间电荷 → cpu fp64（原因 “auto: no space charge, Metal is used for runs with space charge”）。conda 构建：两者都是 cpu fp64。

L40S（`CUDA_VISIBLE_DEVICES=5`）：`{"index":0,"name":"NVIDIA L40S","compute":"8.9","fp64_full":false,"memory_gb":47.67}`；auto 与 device cuda 都解析为 cuda fp32（GPU 0，
“auto: fp64 is slow on this GPU …”），`--precision fp64` 时 cuda fp64；`gpus 3`（只有 1 块可见）给出错误计划 “gpus: device 3 does not exist (1 visible)”；device metal 给出错误计划。
示例项目 `avas run --engine new --threads 16`：0.96 s，avas_run.json `engine` = cuda / fp32 / gpus [0] / threads 16 / 有空间电荷 / new-fp32；`--device metal` 运行前报错退出。

### 8. pytest 与端到端

Mac 的 Anaconda 是 Python 3.9（界面代码要 3.10 以上），worktree 里没有 `.venv`；测试用 uv 在临时目录建的 Python 3.11 venv（`pip install -r requirements.txt pytest httpx`），用完删除。

- **全套 pytest（默认原内核）**：464 passed、16 skipped、14 failed、14 errors。对照：同一 venv 跑 HEAD（375787c）的 `git archive` 为 439 passed、13 failed、14 errors，
  失败与错误的测试完全相同（另一个是 `test_library_reports_the_source_hash_of_this_tree`：当时 `engine/build` 还是改动前构建的库，重新构建后通过——它正是用来发现过期构建的）。
  这些既有失败都与本次无关：Mac 不能加载原内核（`test_gui.py` 的 14 个 setup 错误；`test_smoke.py` 7 项——两次运行与依赖其结果的 5 个画图；`test_scan`、`test_segment` 各 1 项；
  `test_assistant.py` 的沙盒 2 项），`test_run_path::test_stage_inputs…` 要求示例目录里有 `lattice.txt`（worktree 里没有这个生成文件），
  `test_updates::…[frozen]`（打包版自动更新只支持 Windows）。因没有原内核而跳过：`test_error_study` 3 项、`test_research::test_real_engine_run_of_the_example`、
  `test_run_path::test_engine_wrapper…`；`test_plot_registry` 6 项因没有 `test_smoke` 的输出而跳过；其余跳过是 Windows 专有测试。
- **新增 `tests/test_engines.py`**：26 项全部通过（含 3 项需要新内核库的端到端：cpu 运行的 engine 字段、ini `[engine]` 与命令行优先级、cuda 缺失时报错且不建输出目录）。
  `test_research.py`、`test_study_gui.py`、`test_configs.py`、`test_run_path.py`、`test_design_rules.py` 与之同跑：64 passed、2 skipped、1 failed（上面的 `lattice.txt`）。
- **`AVAS_ENGINE=new` 跑真正运行内核的测试**（`test_smoke`、`test_plot_registry`、`test_gui`、`test_segment`、`test_scan`、`test_error_study`、`test_research`、`test_run_path`、
  `test_study_gui`、`test_live`，新内核 auto = Mac 上有空间电荷时 Metal）：84 passed、2 failed。`test_gui.py` 全部通过，`test_error_study` 的 3 种方式在容差内通过，
  `test_research` 的真实内核算例通过。失败一是上面的 `lattice.txt`；二是 `test_research::test_parallel_batch…`：环境里的 `AVAS_ENGINE=new` 让假内核测试也选了新内核，
  auto 预判为 Metal，按决定 8 降为一个 worker，于是不再去掉 `multithreading`——这是设计行为，测试已改为显式 `engine="original"`（假内核测试与环境无关），重跑通过；
  `tests/test_engines.py` 的研究测试同样先清掉 `AVAS_ENGINE*`。
- **ruff**（`uvx ruff@0.6.9 check avas tests packaging tools engine/tests engine/tools`）通过；`git diff --check` 无输出；没有改前端，未跑 `npm test`。

端到端（示例 hwr010 复制到临时目录）：

| 命令 | 结果 |
|---|---|
| `avas run --engine new --device metal` | 2.7 s；`engine` = metal / fp32 / threads 8 / 有空间电荷 / new-fp32，version 与库一致 |
| `avas run --engine new --device cpu` | 3.1 s；cpu / fp64 / new-cpu-fp64 |
| `avas run --engine new --device cuda` | 退出码 2：“error: the new kernel cannot use device 'cuda': this engine was built without CUDA (AVAS_WITH_CUDA=OFF). Choose device auto (the best available) or cpu”，不建输出目录 |
| ini.ini `[engine] kernel = new, device = auto, threads = 4`，`avas run` 不加参数 | metal / fp32 / threads 4，`requested` 记录 ini 的值 |
| `avas run --device metal`（原内核）、`--engine new --device metal --precision fp64`、`--gpus x`、`AVAS_ENGINE_DEVICE=warp` | 各自给出一行清楚的错误（退出码 2，没有异常栈） |
| `avas info`（在项目里） | 两个内核、Metal 设备、auto 的解析、本项目的运行会用什么 |
| `Study`：线性 + 内核运行（2000 粒子） | 线性 rms_x 出口 1.8155 mm；metal 运行 2.3 s 归入 new-fp32；emu fp32 的 Study 直接命中同一缓存（0.11 s，不运行）；单独运行时 Metal 与 emu fp32 的指纹相同、DataSet.txt 逐字节相同；cpu 为 new-cpu-fp64，另一个缓存目录；auto 预判为 new-fp32，命中 Metal 的缓存；原内核的指纹（0d29d051…）与新内核（9a99cc04…）不同。内核 rms_x 出口：Metal 1.8194、CPU 1.8194 mm |

### 9. 与方案不同或方案未写明的地方

1. **`avas run --device` 两用**：方案写“增加 `--device`”，但 `avas run` 已有 `--device cpu|gpu`（原内核的 libPIC.so 通路，决定 6 要求保持不动）。
   做法见第 4 节：同一个参数，按所选内核解释；原内核下给新内核的值报错。没有另起 `--engine-device` 之类的名字。
2. **原内核的研究指纹保持不变**：原内核的 `engine` 部分仍是 `identity()` 的 SHA-1 字符串（`ENGINE_FILES` 的顺序也不变：Linux 上仍取 AVAS.dll 的 SHA-1，只是身份标记），
   原内核的研究缓存不失效；只有新内核用 `{tier, version}`。已核对：同一个原内核 Study 运行的输入，HEAD（375787c）与本次代码算出的指纹相同（7dfde7a4…）。
3. **emu fp64 归入 `new-cuda-fp64`**：方案的五档没有 emu fp64；emu 是在 CPU 上复现设备后端，M6 中与 GPU fp64 的输出相同，所以与 CUDA fp64 共用一档。
4. **研究层把选择写进运行的 ini.ini**，而不是给 `engine_command` / `runner` 增加参数：这样自定义的 `engine_command`、GUI 的沙盒 runner 都不用改签名；
   为此子进程环境去掉 `AVAS_ENGINE*`（否则环境会再次覆盖 Study 的参数）。
5. **新内核库的版本用源码哈希**（不是库文件的 SHA-1）进指纹：同一份源码的不同构建（Apple clang / conda clang、带不带 Metal）同一档内结果相同，可共用缓存；
   库文件的 SHA-1 记在 run.json 的 `engine.lib_sha1`、avas_run.json 的 `engine.sha1`。
6. **无效组合在 Python 层报错**：metal + fp64、cpu + fp32、compat dll + cuda / metal / emu。库本身对 metal + fp64 仍静默用 fp32（保持 `run_gpu.py` 等现有脚本的行为）。

### 10. 受限 / 未完成

- Windows（MSVC）与 Linux 上没有运行 pytest；CUDA 构建只在 L40S 上编译和抽查（7 个算例），未跑全部 184 个；MSVC 构建未验证（XPS 不通，M7c 的 CI 覆盖）。
- `avas/engine/new/` 现在没有库（M7c）；没有它时从源码树的 `engine/build` 找。打包后的路径已走 `avas.paths`，但 PyInstaller 收集 `avas/engine/new/` 要在 M7c 核对。
- `[engine]` 的界面（M7b）尚未做：GUI 的运行失败信息里 “the new kernel cannot use device …” 等是英文（来自子进程的 `error:` 行），M7b 做设置页时一并给出中文提示。
- auto 在研究层按“有空间电荷”的计划决定并行绑卡；同一批里有无空间电荷混合时，没有空间电荷的运行若在 Mac 上实际走 CPU，也只按一个 worker 运行（不影响结果）。
- 混用不同型号的卡时 precision auto 按所选卡是否全部 fp64 全速决定；绑卡并行时每个 worker 只用一张卡，若各卡档位不同，实际档可能与预判不同（会记警告）。

### 11. 新的待定点（每项附推荐）

1. **新内核在 auto 下没有 GPU 时的提示**：auto 回落到 CPU 是设计行为，但用户可能以为在用 GPU。推荐 M7b 在运行页显示“实际：CPU（auto：…原因）”，取自 avas_run.json，不另加弹窗。
2. **原内核身份的文件顺序**：`identity()` 在 Linux 上取 AVAS.dll 的 SHA-1（M7 前就如此，只作身份标记）。推荐保持（改动会让 Linux 上原内核的研究缓存全部失效）；
   第三阶段移除原内核时再统一。
3. **研究缓存对库文件的敏感度**：现在同一源码哈希、同一档的不同构建共用缓存。推荐保持；若以后发现编译器差异让同档结果在末位不同（目前 Apple / conda 两种构建与 AVAS.dll 的差别相同、彼此逐字节相同于各自对照），再把编译器加入指纹。
4. **`avas info --engine-json` 的稳定性**：它现在是研究层的内部接口（JSON 字段见 `engines.describe`）。推荐 M7b 的关于窗口与设置页也用它（后端在子进程里调用，与研究层共用缓存），
   并把字段视为稳定接口，改动时同步 `tests/test_engines.py`。
5. **`threads` 写进 ini.ini 时的含义**：新内核的线程数只影响速度。推荐设置页把它放进“高级”，默认 0（自动），不随项目在不同机器间强制同一个数。

### 12. 临时数据

Mac：`$TMPDIR/claude-…/scratchpad/m7a/` 下的两种新构建、两份对照构建、HEAD 的源码副本、parity / Metal 工作目录、示例项目副本、Python 3.11 测试 venv，结束时已删除；
测试在 worktree 里生成的 `tests/_output*`（已被 .gitignore 忽略）与 HEAD 副本留下的 `~/.local/share/AVAS/updates/installation-26588f….lock` 两个锁文件也已删除
（建 venv 时下载的包留在 uv 的全局缓存里）。
`engine/build`（开发者自己的构建目录，不提交）已用新代码重新构建。L40S：`/data/public/lycself/m7a_tmp`（源码、两份 CUDA 构建、比对工作目录、示例运行）与当天新建的
`~/.local/share/AVAS/updates/installation-68ca….lock` 两个锁文件已删除；没有新建 docker 镜像。L40S 上的设备查询按要求用 5 号卡；为看多卡下 `--gpus 1` 的解析，有一次查询以 `CUDA_VISIBLE_DEVICES=5,6` 运行，在 6 号卡上建立过一次上下文（只有探针核函数，没有计算）。

## M7b：界面接入（2026-11-11，Mac M2）

### 1. 改动的文件

| 文件 | 内容 |
|---|---|
| `avas/engines.py` | 平台默认：`original_available()`（只认 Windows / Linux 的 x86-64）、`default_kernel()`（有原内核 original，否则 new）、`platform_label()`、`check_platform()`（没有原内核的平台上显式选原内核时报错，说明来源 ini / 命令行 / 环境变量）；`describe()` 加 `platform` 与 `original.available`；`input_keywords()`、`input_space_charge(…, spacecharge=)`（表单未保存的开关）；`clear_query_cache()`；auto 运行的 `engine.reason`（`_auto_reason`：运行后取内核计划的原因，计划的设备与实际不同时注明“运行时不能用”） |
| `avas/cli/main.py` | `avas run` 在写输出目录前调用 `check_platform`（退出码 2）；原内核分支保留 kernel 的来源；`avas info` 在没有原内核的平台写“not available on macOS arm64 (only the new kernel; it is the default here)”；`--engine` 帮助文字 |
| `avas/gui/services/kernels.py`（新） | `engine.info`（子进程查询，`refresh` 清缓存）、`engine.preview`（表单的选择对本项目 sc / nosc 的计划）、`engine.check` 与 `require_ready(input_dir)`（运行前检查：设置无效、原内核平台、原内核 + PICNIC、新内核库、设备与组合）、`summary` / `read_summary`（avas_run.json 的 `engine` → 标签数据，旧记录 None）、`settings_form` / `validate_form` / `save_settings`（`[engine]` 表单） |
| `avas/gui/bridge.py`、`frontend/src/bridge.ts` | `UserError(template, **params)`：回复带 `template` 与 `params`，前端 `t(template, params)` 翻译，带变量的错误也能翻译；没有参数时与原来完全相同 |
| `avas/gui/services/simsettings.py` | `settings.load` 返回 `engine`；`settings.save(…, engine=)` 写任何文件前先校验，最后只在与文件不同时写 `[engine]` |
| `avas/gui/services/runner.py` | `run.check` 返回预计的内核（并检查），`run.start` 把它放进 `Job.engine`；阶段结束后读该阶段 avas_run.json 换成实际值；运行状态与 `run.finished` 带 `engine` |
| `avas/gui/services/segments.py`、`study.py`、`analysis.py`、`avas/ai/sandbox.py` | 分段运行、研究页内核扫描、内核响应矩阵、AI 沙盒（`Sandbox.run_in`，每次内核运行）在启动前 `require_ready`；沙盒 activity 带 `engine`（预计 → 实际）；`results.sources` 每条带 `engine`（OutputFile、Runs/、Research/、Segments/） |
| `avas/gui/services/live.py` | `run.replay` 带 `engine` |
| `avas/ai/avas_prompt.py`、`avas/gui/services/assistant.py` | 提示词：两个内核、`[engine]`、沙盒沿用项目选择、PICNIC / scanphase、比较不同内核或结果档时的注意事项；项目事实加一行“本项目的内核 + 上次运行用了什么” |
| `frontend/src/engine.ts`（新）、`engine.test.ts` | 选项与组合规则（`deviceChoices`、`precisionProblem`、`compatProblem`、`formProblems`）、ini 行（`iniLines`）、标签（`engineLabel`、`engineShort`、`autoReason`）、关于窗口的文字（`kernelLines`）；17 个 vitest |
| `frontend/src/pages/EngineSettings.tsx`（新）、`SettingsPage.tsx` | 模拟设置页“计算内核”分组；PICNIC 选项与原内核下的提示（运行前 `validate` 也拦下）；scanphase 注明两个内核都不读、scanData 表默认收起 |
| `frontend/src/components/EngineBadge.tsx`（新）、`store/engine.ts`（新） | 标签组件；`engine.info` 的共享缓存（壳层启动 5 秒后预取一次） |
| `frontend/src/pages/RunPage.tsx`、`lattice/LiveBeamPanel.tsx`、`shell/Shell.tsx`、`pages/ResultsPage.tsx`、`results/CompareTab.tsx`、`store/app.ts`、`store/live.ts`、`actions.ts` | 运行页信息区“内核”一行、实时面板标题、运行记录每行、状态栏（运行中为本次运行，空闲时为本项目上次运行）、结果页来源列表与当前来源下方一行、运行对比的勾选列表与图例；关于窗口与“复制版本信息”的“计算内核”段；非 Windows 的关于窗口不再写“WebView2 –”（macOS 写 WKWebView） |
| `frontend/src/i18n/zh_CN.json`、`styles/pages.css` | 新文字的中文（88 条，删掉不再使用的“相位扫描模式 2 需要 scanData.txt”一条）；颜色只用已有变量 |
| `README.md`、`frontend/src/help/manual.{zh,en}.md`、`AGENTS.md` | 第 2 节 |
| `tests/test_gui_engine.py`（新）、`tests/test_engines.py`、`tests/test_study_gui.py`、`tests/test_assistant.py` | 第 4 节 |

### 2. 设计要点

- **平台默认**：`[engine] kernel` 未写时，有原内核的平台用原内核（M7 的“默认仍用原内核”针对的就是这些平台），没有原内核的平台（macOS、ARM Linux）用新内核。
  `avas info`、设置页（“本平台只有新内核”）、运行记录都按这个默认。显式选原内核在运行前报错（CLI `check_platform`，GUI `require_ready`，设置页的 `validate` 也拦下）。
- **设置页**：设备只列本机检测到的（`engine.info`，子进程 `avas info --engine-json`，按库文件与选择缓存）；auto 时用 `engine.preview` 按表单的空间电荷开关显示本项目会用什么，
  同时给出另一种情况（有 / 无空间电荷）。不合法的组合（Metal + fp64、CPU + fp32、复现 AVAS.dll + GPU）禁用并说明，规则与后端 `engines.validate` 相同；
  新内核库缺失时“新内核”不可选并说明如何获得；本机没有 ini 里写的设备时保留显示并警告。GPU 多选只在多张 NVIDIA 卡时出现；“高级”里是线程数（默认 0）与兼容模式，
  并显示将写入的 `[engine]` 行。环境变量 `AVAS_ENGINE*` 覆盖 ini 时提示。保存规则：kernel 显式写，其余键等于默认值时删掉，表单与文件相同时不写。
- **运行前检查**：所有 GUI 运行入口调 `require_ready`。它与子进程解析选择的方式相同（命令行没有参数，环境变量 > ini > 默认），失败时是带参数的 `UserError`，
  模板有中文翻译（细节保留内核给出的英文原因）。AI 沙盒的检查在每次 `run_in` 前（查询有缓存，只有第一次需要约 1–3 s）。
- **标签**：数据只来自 avas_run.json 的 `engine`；运行中显示 `run.check` 的预计值（斜体，提示“按设置预计”），阶段结束换成实际值。auto 落到 CPU 时标签写
  “实际：CPU（自动：…）”，原因按内核的英文原因归为几类翻译（无空间电荷、兼容模式、构建不含 GPU、运行时失败、本机没有可用 GPU），完整原因在悬停提示里。
- **scanphase / scanData.txt**：只改显示（注明不读、表格默认收起），不删文件、不改保存逻辑；去掉了“相位扫描 = 2 且表格为空时不能运行”的检查——两个内核都不读，这个检查只会挡住运行。

### 3. 与方案不同或方案未写明的地方

1. **macOS 默认新内核**：方案只写“默认 original”，本次按任务说明在没有原内核的平台默认 new（理由见上）。`tests/test_engines.py` 里依赖“默认 original”的两处改为显式设定平台或 ini。
2. **带参数的 UserError**：为翻译带变量的提示新增（模板 + params）；只是多两个字段，旧调用不变。
3. **关于窗口按需查询**：方案写“列出两个内核与本机设备”；查询是子进程（Mac 上约 1–3 s），所以壳层启动 5 秒后在后台预取一次，关于窗口直接用缓存。
4. **研究页内核运行的 GUI 检查**：研究层 `Study` 本来就在设备不可用时抛 `EngineChoiceError`；GUI 另在 `study.start` / `analysis.responseEngineStart` 开始前检查一次，失败时整个扫描不开始，而不是每行报错。

### 4. 检查结果（Mac M2）

新内核库：Apple clang 17 构建到临时目录（含 Metal），用 `AVAS_ENGINE_LIB` 指向；Mac 上没有写 `[engine]` 的项目默认就用新内核（auto：有空间电荷 → Metal fp32），
所以下面的全套 pytest 就是“新内核下跑真正运行内核的测试”。

| 检查 | 结果 |
|---|---|
| 全套 pytest（`AVAS_ENGINE_LIB=… python -m pytest`） | 524 passed、5 skipped、3 failed；失败只有既有项（`test_run_path::test_stage_inputs…` 要求示例目录有生成的 `lattice.txt`；`test_updates::…[frozen]` 打包更新只支持 Windows）与 `test_engines::test_library_reports_the_source_hash_of_this_tree`（M7c 同时在改 `engine/` 源码，临时库按改动前的源码构建，属预期） |
| `tests/test_gui_engine.py`（新，15 项） | 全部通过：UserError 模板、`engine.info` 形状 / 失败不抛异常 / 真实 `query_subprocess` 的缓存与 refresh、平台默认、只在变化时写 `[engine]`、保留 ini 其余段、无效组合在写任何文件前拒绝、无效项与环境变量提示、运行锁、`engine.preview`、`engine.check` / `run.check` 的各种失败、AI 沙盒带上 `[engine]` 并在设备不可用时拒绝、`summary`、`results.sources` 的 `engine`（含旧记录为 None）、runner 状态的预计 → 实际 |
| `tests/test_engines.py` | 新增平台默认与 `check_platform`、表单空间电荷、Mac 上显式原内核的 CLI 报错（只在没有原内核的平台运行） |
| `tests/test_assistant.py::test_sandbox_study_shows_on_the_run_page…` | 原来在 Mac 新内核下失败：Metal 跑完示例只要约 1 s，而进度行每 3 s 才打印一次（与原内核相同），测试等不到进度。新内核时把这次扫描的粒子数临时调到 150 万，通过；原内核行为不变 |
| vitest `npm test` | 23 个文件 146 项通过 |
| `npm run build` | 成功，`avas/gui/web/` 已更新 |
| ruff 0.6.9 | All checks passed |
| `git diff --check` | 无输出 |

浏览器面板（`avas serve --token dev`，示例副本）：设置页计算内核分组（深色 / 浅色、中文）、本平台只有新内核的说明、ini 写原内核时的警告与禁用、Metal 下 fp64 禁用、
auto 的解析（“自动，本项目（有空间电荷）：Metal fp32；无空间电荷时：CPU fp64”）、PICNIC 与 scanData 提示、原内核 + PICNIC 的“无法运行”对话框、
Metal 运行后运行页 / 运行记录 / 实时面板 / 状态栏的标签、关闭空间电荷后 auto 落到 CPU 的标签（“实际：CPU（自动：无空间电荷，GPU 只用于有空间电荷的运行）”）、
结果页来源与运行对比（图例带“[new kernel Metal fp32]”）、关于窗口。保存后的 ini：`[engine] kernel = new, device = metal`，其余段不变。

Mac 真窗口（`avas gui`，pywebview Cocoa，脚本给 `webview.start` 打补丁驱动、`screencapture -l` 按窗口截图）：页面正常加载（中文），设置页显示正确；
原生文件夹对话框经 `dialog.openFolder` 打开、取消后返回 None；150 万粒子的 Metal 运行：暂停后进度停在 85.36 %（5 s 内不变，内核报告的墙钟时间含暂停的 5 s），
继续后完成，标签为“新内核（预览）· Metal fp32”；第二次运行在 90 % 时停止，记录为 stopped，运行页显示预计的标签、状态栏显示“请求：自动”。日志中没有错误。

### 5. Mac 上发现的问题

- 已改：关于窗口在 macOS 写“WebView2 –”，现在写 WKWebView（其他非 Windows 平台写“系统网页视图”）。
- 未改（不属于 M7b 文件范围或较大）：
  1. （M7c 已在 `pyproject.toml` 的 `package-data` 加入 `avas/engine/new/` 的库与构建戳，`pip install .` 也会带上新内核库。）
  2. 新内核短于 3 s 的运行没有进度行（与原内核相同的 3 s 间隔），运行页在这期间显示 0 %；不影响结果，Metal 上的小算例常见。
  3. 原生文件对话框是独立窗口（不是附着在主窗口上的 sheet），按窗口截图看不到它；功能正常。
  4. `examples/hwr010` 的 `beam.txt` 让两个内核都打印 “beam.txt Contains unknown keywords”，与本次无关。

### 6. 新的待定点（每项附推荐）

1. **进度行间隔**：新内核沿用 3 s；GPU 上多数算例几秒内结束，运行页长时间显示 0 %。推荐新内核在非 compat 模式下改为“1 s 或 5 % 先到者”（日志格式不变，只影响频率），M7c 之后再定。
2. **auto 原因的翻译**：现在按关键字把内核的英文原因归为五类。推荐 C 接口在 `avas_engine_last_run()` 里直接报告原因代码（如 `no_space_charge`、`no_gpu`、`compat_dll`），界面按代码翻译，不再匹配英文。
3. **状态栏空闲时显示什么**：现在显示本项目上次运行的内核（点击打开模拟设置）。推荐保持；若用户觉得与当前设置混淆，可改为“当前设置 + 上次运行”两段。

### 7. 临时数据

`scratchpad/m7b/` 下的新内核构建、Python 3.11 venv、两个示例项目副本、`avas serve` 与真窗口的临时设置、测试输出，结束时删除（截图除外，留给报告）；
测试在 worktree 生成的 `tests/_output*` 与本次 `avas serve` / 真窗口写进 `~/.local/share/AVAS/logs/20261111.log` 的日志已删除（没有新建安装锁文件）。`frontend/node_modules` 是本次 `npm ci` 安装的（被 .gitignore 忽略），留给后续构建。`engine/build` 未动。

### 8. 待定点的决定与实施（2026-11-14，用户“全部按推荐”）

1. 进度行：新内核默认模式每 1 s 或每 5 % 打印一次（先到者，`tracker.cpp` 的 `progress_due`），compat=dll 仍为 3 s，格式不变。`test_assistant` 的扫描算例仍需要 150 万个粒子
   （要在第一个取值运行中途暂停、继续、停止，与进度间隔无关），只改了注释。
2. 原因代码：`DevicePlan::code` 在每个决定处设置，`avas_engine_devices()` 的计划与 `avas_engine_last_run()` 都带 `reason_code`（last_run 另带 `reason`，即运行实际所用设备的原因，
   运行时回落为 `run_time_failure`）。`engines.run_record` 有 `reason_code` 时直接记入 avas_run.json（旧库仍按设备计划推断原因），`kernels.summary` / `engine.check` 传 `reasonCode`，
   前端 `autoReason(reason, code)` 按代码给出 13 种中文说明，没有代码的旧记录才按文字归类；模拟设置页“自动”预览在回落到 CPU 时也写出原因。代码表见 `engine/README.md`。
3. 状态栏：保持现状。
