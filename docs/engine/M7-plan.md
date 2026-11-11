# M7 方案：新内核接入 Python、界面、打包与 CI（2026-11-11，待用户确认）

目标：新内核作为**可选**内核进入 main。这是淘汰计划的第一阶段，默认仍用原内核。
- 用户可以在命令行、模拟设置页和研究接口里选择内核、设备和精度。
- 运行记录写清这次用了什么。
- 三个平台的预编译库随源码和安装包发布，CI 负责构建和比对。

不在 M7 里做的事：
- 把默认切换成新内核（第二阶段，等用户在真实项目上用过一段时间）；
- RFQ（等输入样例）；
- 待定点 4 中的 Mac 安装包。

## 1. 现状诊断

### 1.1 运行通路与内核选择

所有内核运行都在子进程里执行 `avas run`，子进程里由 `MultiParticleEngine` 用 ctypes 加载库。用到子进程的地方有：
- 运行页、分段运行、误差研究：`gui/services/runner.py`；
- 研究层：`research/engine.py`；
- AI 沙盒：`ai/sandbox.py`；
- 入门教程。

因此只要在 `avas run` 里决定加载哪个库、设置哪些选项，所有入口就都能用上新内核，不需要改各处的进程管理。暂停（挂起进程树）、停止（结束进程）、进度行、DataSet.txt 实时读取对新内核照常有效：进度行与 DataSet 格式和原内核相同（D-13、D-04 等）。

分支上目前只有 `AVAS_ENGINE=new` 环境变量，它在 `engine/build/` 下找自己构建的库，所以只有从源码、自己编译时能用。设备、精度、GPU、兼容模式、线程数只能用环境变量（`AVAS_ENGINE_DEVICE` 等）或 `avas_set_option` 设置。

原有的设备设置是 ini.ini 的 `[input] device = cpu|gpu`：
- `gpu` 走 `avas/gpu/`（ctypes 调用 libPIC.so，只有 Linux）；
- Windows 打包时排除了 `avas.gpu`；
- 模拟设置页不提供这个选项，只是原样保留。

### 1.2 运行记录与研究层

- `avas_run.json` 不记录内核。
- 研究层指纹里的内核身份固定取 `avas/engine/` 下原内核文件的 SHA-1（`research/engine.py: identity()`）。所以现在用 `AVAS_ENGINE=new` 跑研究，会和原内核的运行**共用同一个缓存指纹**，结果会被错误复用。这是分支上已有的问题，M7 必须修。

### 1.3 C 接口

`avas_engine_version()` 返回固定的 "avas-engine M5"。头文件的注释只写了 compat、threads 两个选项。

缺少三个接口：
- 查询本机可用设备；
- 查询本次运行**实际**选中的设备和精度（auto 时界面和运行记录需要）；
- 查询构建信息（是否含 CUDA / Metal / MPI、源码版本）。

### 1.4 二进制与 CI

- 原内核的二进制提交在 `avas/engine/`，新内核没有任何预编译库。
- CI 只有 Windows 的 ruff + pytest、前端测试与构建、main 上的打包发布，不构建、也不测试 `engine/`。

### 1.5 平台

- README 只写了 Windows 和 Linux。
- 在 macOS 上，Python 部分加新内核可以运行命令行，但界面从未在 Mac 上核对过。要核对的有：
  - pywebview 的 Cocoa 后端；
  - `host.ts` 的文件对话框；
  - 暂停用的进程树挂起；
  - AI 密钥：非 Windows 平台存在文件里（`ai/secrets.py`），没有用钥匙串。

### 1.6 界面与内核不一致（main 上已经存在）

| 项 | 原内核 | 新内核 |
|---|---|---|
| input.txt `scanphase` 与 scanData.txt（模拟设置页"相位扫描 = 从 scanData.txt"） | 不读 | 不读（与原内核一致） |
| `scmethod PICNIC` | 报错 | 支持（M6b） |
| `err_*_cpl_*` | 静默忽略 | 静默忽略；Python 误差研究自己抽样，不受影响 |
| SeParticle.txt | 不读 | 默认模式在 beam.txt 未写 `readparticledistribution` 时读取（M5b） |

### 1.7 合并前还欠的验收（XPS 不通）

- Windows MSVC 构建与 AVAS.dll 的 strict parity；
- `test_error_study` 的三种方式；
- MSVC CUDA 构建与 GTX 1650 上的比对和性能。

## 2. 设计

### 2.1 库的位置与 C 接口

新库放在 `avas/engine/new/`：`avas_engine.dll`、`libavas_engine.so`、`libavas_engine.dylib`。和原内核文件分开，第三阶段移除原内核时不会碰到它们。

新增三个导出函数，都返回 JSON 文本。原有的 7 个导出不变，输入输出文件格式不变。

| 函数 | 内容 |
|---|---|
| `avas_engine_info()` | 引擎源码哈希、构建选项（CUDA / Metal / MPI、机器码架构）、编译器 |
| `avas_engine_devices()` | CUDA 卡的名称、算力、fp64 档、显存；Metal 设备名称；CPU 线程数 |
| `avas_engine_last_run()` | 上一次 `main_agent` 实际使用的设备、精度、GPU 列表、compat、线程数、耗时 |

`avas_engine_version()` 改为返回引擎源码哈希，和 2.5 节的构建戳一致。

### 2.2 选择方式与优先级

优先级从高到低：

1. 命令行：`avas run --engine original|new --device auto|cpu|cuda|metal --precision auto|fp64|fp32 --gpus 0,1|all --compat none|dll --threads N`；
2. 环境变量：`AVAS_ENGINE`、`AVAS_ENGINE_*`，保留现有名字；
3. ini.ini 的 `[engine]` 段：`kernel`、`device`、`precision`、`gpus`、`threads`、`compat`；
4. 默认值：`kernel = original`；选新内核时 `device = auto`、`precision = auto`、`compat = none`。

其他规则：
- 不写进 input.txt，原内核仍能读同一个项目。这是 2026-11-07 的决定。
- 原内核被选中时，忽略 `[engine]` 里其余的键。
- 旧的 `[input] device = gpu` 只对原内核有意义，保持原样（见待定点 6）。
- 新内核在 `device = auto` 时的规则：
  - 有 NVIDIA 卡用 CUDA，精度 auto 时按卡分 fp64 / fp32；
  - Mac 上有空间电荷用 Metal；
  - 都没有就用 CPU。
- 选了 cuda / metal 但本机没有时：报 UserError，说明原因并提示改成 auto 或 cpu。不悄悄改用 CPU。

### 2.3 运行记录、结果页与研究层

`avas_run.json` 新增 `engine` 字段：
- 原内核：`{"kernel": "original", "sha1": …}`；
- 新内核：再加 `version`、实际的 `device`、`precision`、`gpus`、`compat`、`threads`，取自 `avas_engine_last_run()`。

运行页的运行记录、结果页的来源列表、运行对比都显示内核标签，例如"新内核 · CUDA fp32"。运行对比本来就能比较两次运行，所以可以直接拿同一项目的原内核运行和新内核运行对比，这也是用户验证新内核最直接的办法。

研究层的指纹把内核身份换成**结果档**加库的版本，然后在指纹相同时复用缓存。结果档分五种：

| 结果档 | 说明 |
|---|---|
| 原内核 | |
| 新内核 CPU fp64 | |
| 新内核 CUDA fp64 | 无空间电荷时与 CPU 逐位相同，有空间电荷时不同，所以单列一档 |
| 新内核 fp32 | CUDA、Metal、emu 逐字节相同，共用一档 |
| 新内核 compat=dll | |

新内核默认模式的结果与线程数无关（M4 决定），所以线程数不进指纹。

`Study(…, engine="new", device=…, precision=…)` 透传这些选项。`workers > 1` 且用 GPU 时的处理见待定点 8。

### 2.4 界面

**模拟设置页**新增"计算内核"分组，写 ini.ini 的 `[engine]`，受运行锁约束：

| 选项 | 内容 |
|---|---|
| 内核 | 原内核 / 新内核（预览） |
| 设备 | 自动 / CPU / NVIDIA GPU / Mac GPU。只列本机检测到的设备，选自动时显示将会选中什么 |
| 精度 | 自动 / fp64 / fp32 |
| GPU | 多卡时出现 |
| 线程数 | |
| 兼容模式 | 修正原内核缺陷（默认）/ 复现 AVAS.dll，作为高级选项收起 |

只选原内核时，后面几项变灰，并说明它们只对新内核有效。

其他界面改动：
- 运行页和状态栏显示本次实际使用的内核与设备。
- 关于窗口与 `avas info` / `avas doctor` 列出两个内核的版本和本机设备。
- 原内核下选择 `scmethod PICNIC` 时，提示"只有新内核支持"，否则运行时原内核会报错。
- scanphase / scanData.txt 的处理见待定点 5。
- AI 助手和入门教程的运行沿用项目设置。助手提示词加一句内核说明，比较两个内核的结果时提醒注意结果档。

### 2.5 预编译库的构建与分发

**CI**：新增工作流 `engine.yml`，在 `engine/` 改动时自动运行，也可以手动触发。

| 平台 | 构建 | 检查 |
|---|---|---|
| windows-latest | MSVC 加 CUDA 12.4（CI 里安装工具包） | 单元测试；parity `run_all` 在 compat=dll 下与 AVAS.dll 参考逐位比较；emu fp32 |
| ubuntu-latest | 仓库里的 manylinux2014 + CUDA 12.4 Dockerfile | 单元测试；`run_all` 带容差比较 |
| macos-14 | Apple clang + Metal（arm64） | 单元测试；`run_all` 带容差比较 |

- CI 没有 GPU，GPU 通路由 emu 设备覆盖，真卡比对仍在 A100 / L40S / 1080Ti / 1650 上手动做。
- Windows 上的逐位比较就是欠下的 strict parity。CI 能做它，就不再依赖 XPS，但 1650 真卡仍需要 XPS。
- 三个平台的产物上传为 artifact，附 SHA-256。

**提交方式**（见待定点 2）：做法同 `avas/gui/web/`。
- 预编译库提交到 `avas/engine/new/`，用户不需要编译器。
- `avas/engine/new/build-stamp.json` 记录引擎源码哈希、构建工作流的运行号和各文件的 SHA-256。
- `tests/test_design_rules.py` 增加检查：`engine/` 的源码变了而库没有重新生成时失败。
- 库从 CI 产物下载后人工提交，CI 不自己推送。

**打包与更新**：
- PyInstaller 本来就收集 `avas/engine/` 下的全部文件，新库会进入 Windows 安装包和 ZIP。
- 更新清单按文件哈希自动包含新库，源码包也包含。

**体积**：带 CUDA 的库每个平台大约十几到几十 MB，要实测（见待定点 3）。可以去掉没有目标设备的 sm_60、sm_70 来缩小，只保留 61/75/80/86/89/90 加 PTX。

**平台说明**：
- Linux：源码安装，补集群说明：CentOS 7 / glibc 2.17 可用；Slurm 示例脚本；单进程多卡用 `--gpus all`。MPI 版不随包发布，CONTRIBUTING 里写构建方法。
- macOS：见待定点 4。

### 2.6 测试

**pytest**：
- 覆盖内核选择优先级、`[engine]` 读写、设置页接口、`avas_run.json` 的 engine 字段、研究指纹区分结果档、`avas info` 输出、选了不存在的设备时的报错。
- CI 的 Windows 作业里用新内核跑一个短算例的冒烟测试。
- `test_error_study` 增加"新内核 compat=dll"参数，在 Windows 上要求与原内核逐字节相同；默认模式改用随机数新发生器，只检查能跑通、文件齐全。

**前端 vitest**：设置页选项与 ini 键的映射。

**真机**：
- Windows（XPS）：
  - 真实窗口里选新内核运行一次，运行、分段运行、误差研究、研究页扫描各一次；
  - 打包后的 exe 选新内核运行；
  - GTX 1650 上的 fp32 / fp64；
- Mac：`avas gui` 与 `avas serve` 上用 Metal 运行。
- Linux：L40S 上 `avas serve` 用 CUDA 运行；A100 上用命令行。

用完的临时数据照例删除。

## 3. 分阶段

| 阶段 | 内容 | 依赖 |
|---|---|---|
| M7a | C 接口的三个新函数与版本号；`avas run` 的选择层与优先级；`[engine]` 读写；`avas_run.json` 的 engine 字段；研究指纹改为结果档；`avas info` / `avas doctor`；pytest | 无 |
| M7b | 模拟设置页"计算内核"分组、运行页和结果页的内核标签、PICNIC 提示、scanphase 处理、关于窗口；Mac 上核对界面；README（中英文）、使用说明、AGENTS.md | M7a |
| M7c | `engine.yml`（三平台构建与比对）、预编译库提交与构建戳检查、打包、Linux 集群说明、CONTRIBUTING | M7a，可与 M7b 并行 |
| 验收 | Windows 真机（XPS）、Mac、L40S、A100 | M7b、M7c |

每段我在 Mac 上重新构建、复核后在 `engine-rebuild` 本地提交。Windows 部分以 CI 结果为主，XPS 能连上后补真机。

## 4. 合并到 main 的条件

- Windows MSVC strict parity 全部通过（CI 或 XPS），`test_error_study` 的三种方式通过，GTX 1650 比对通过；
- 默认仍用原内核：没选新内核的用户行为不变，全套 pytest 与打包冒烟测试通过；
- README、使用说明、变更记录写好；
- 合并前问用户。main 会自动发布，推送也要先问。

## 5. 待定点（附推荐）

1. **内核选择放在哪里**。推荐 ini.ini 的 `[engine] kernel`，按项目保存，随项目走，运行记录可追溯；命令行和环境变量可以覆盖。不做全局偏好。备选：gui.json 里放一个全局默认、项目可以覆盖，便于一次切换所有项目，但同一个项目在不同机器上可能用不同内核。
2. **预编译库怎么分发**。推荐 CI 构建，人工下载后提交到 `avas/engine/new/`，加构建戳检查（与前端 web 同一做法）。用户不需要编译器，Git、源码 ZIP、安装包三种方式都直接可用。缺点是仓库每次更新内核都要增加几十 MB。备选：作为 Release 附件按需下载，仓库小，但 Git 用户首次运行要联网，更新机制要增加一类文件。
3. **Windows 安装包是否带 CUDA**。推荐带上：CUDA 运行库是静态链接的，用户只需要显卡驱动。M7c 实测体积，若安装包增加超过约 100 MB，再考虑拆成"CPU 包 + 可选 GPU 组件"。
4. **macOS 的范围**。推荐 M7 只支持源码安装：命令行、`avas serve`、`avas gui` 都能用，支持 CPU 和 Metal。不做 .app、签名和公证。Mac 上自动更新只支持 Git 方式。AI 密钥暂时仍存文件，不改钥匙串。
5. **scanphase / scanData.txt**。两个内核都不读，模拟设置页现在的"相位扫描 = 从 scanData.txt"实际不起作用。推荐在界面上标明"当前内核不读取，只保留文件"，并收起表格。不为新内核臆造这个功能的含义；以后需要时请提供原来的语义（文档或原 GPU 版的用法）再实现。
6. **原来的 GPU 通路**（`avas/gpu/` + libPIC.so，`[input] device = gpu`）。推荐 M7 保持不动：只在 Linux 命令行可用，界面不提供，界面里的 GPU 选择只对新内核有效。第二阶段新内核成为默认时，随原内核一起淘汰。
7. **界面里的名称**。推荐默认切换之前叫"新内核（预览）"，运行记录和结果页的标签都带内核名，避免混淆两个内核的结果。
8. **研究层的 GPU 并行**。`workers > 1` 且用 GPU 时，推荐每个 worker 轮流绑定一张卡，worker 数不超过卡数；单卡时 worker 数自动降为 1，并在日志里说明。fp32 档与 CPU 混合并行不做。

## 6. 用户决定（2026-11-11）
第 5 节 8 个待定点全部按推荐：内核选择放 ini.ini `[engine]`（命令行、环境变量可覆盖），预编译库由 CI 构建后人工提交到 `avas/engine/new/` 并检查构建戳，
Windows 安装包带 CUDA（实测体积后再议拆分），macOS 只支持源码安装，scanphase / scanData.txt 在界面标明不被读取并收起表格，原 libPIC.so 通路保持不动，
界面名称“新内核（预览）”，研究层 GPU 并行每个 worker 绑定一张卡。
