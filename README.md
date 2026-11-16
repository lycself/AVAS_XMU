# AVAS — Advanced Virtual Accelerator Software

[中文说明](#中文说明) · [English](#english)

## 中文说明

AVAS 是面向直线加速器的束流动力学模拟软件，提供 C++ 计算内核、Python 前后处理、桌面与浏览器图形界面，以及命令行工具。界面支持中文和英文。

[下载与快速开始](#下载与快速开始) · [主要功能](#主要功能) · [源码安装](#源码安装) · [命令行与浏览器](#命令行与浏览器) · [更新与常见问题](#更新与常见问题) · [文档与反馈](#文档与反馈)

### 下载与快速开始

在[官方发布页](https://github.com/lycself/AVAS_XMU/releases)选择适合的文件：

| 使用方式 | 下载文件 | 启动方法 |
| --- | --- | --- |
| Windows 安装版（推荐） | `AVAS-版本-setup.exe` | 双击安装，再从快捷方式启动 |
| Windows 便携版 | `avas-windows.zip` | 完整解压，运行 `AVASGui.exe` |
| 源码运行或开发 | `avas-source.zip`，或克隆仓库 | 按下方“源码安装”配置环境 |

Windows 安装版和便携版无需 Python、Git 或 GitHub 账户。请保留便携包的完整目录，不能只复制 exe。GitHub 自动附带的 **Source code** 是源码；`update.json` 是供程序读取的元数据，无需手动下载。

桌面界面需要 Microsoft Edge WebView2，缺少时会提示安装。AVAS 安装包内置的程序可离线安装；缺少 WebView2 时仍需联网获取该运行时。安装器支持中文和英文，首次启动沿用安装器语言，后续保留个人设置。

首次使用：

1. 打开 AVAS，新建项目或打开已有项目；源码仓库提供 [hwr010 示例](examples/hwr010)。
2. 在“束流”“结构”“设置”页检查输入；需要切换运行文件时，在结构页点击“设为运行结构”。
3. 点击“运行”，在运行页查看进度和实时包络，在结果页分析输出。
4. 需要留下结果时点击“保留本次结果”；下次完整运行会覆盖 `OutputFile/`。

详细步骤可在 **帮助 → 使用说明** 中离线查看，或阅读[操作指南](frontend/src/help/manual.zh.md)。

### 主要功能

侧栏按工作流排列：项目、束流、结构、模拟设置、文件、运行、结果；其下是研究页，加速器入门固定在底部（`Ctrl+1`…`Ctrl+9` 按此顺序切换）。界面支持浅色／深色主题、界面缩放和鼠标／触控板操作。

| 页面 | 用途 |
| --- | --- |
| 项目 Project | 新建、打开项目，查看输入与上次运行摘要 |
| 束流 Beam | 配置粒子、能量、分布、Twiss 参数；读取 `.dst` 粒子文件并显示其粒子分布；二次粒子开关与 SeParticle.txt |
| 结构 Lattice | 文本与参数联动编辑、关键字检查、二维布局与三维束线视图 |
| 模拟设置 Simulation settings | 选择计算内核、设备与精度；配置模拟类型、步长、空间电荷、多线程与误差研究；编辑损失边界 boundary.txt |
| 文件 Files | InputFile 的文件浏览器：原始文本、场图曲线与切片、粒子文件，导入、复制、重命名 |
| 运行 Run | 启动、暂停、停止模拟，查看实时包络、运行记录与回放 |
| 结果 Results | 分析包络、发射度、损失、能量等，对比运行并导出 PNG／PDF／SVG |
| 研究 Study | 参数扫描（线性模型或内核，一维／二维热图）、周期稳定区图谱、响应矩阵与可控性、反馈仿真 |
| 入门 Primer | 加速器原理、各元件的物理与参数、交互实验和引导教程，无需打开项目 |

#### 计算内核
AVAS 有两个计算内核：重建的**新内核**（所有平台的默认内核）可在 CPU、NVIDIA GPU（CUDA）和 Mac GPU（Metal）上计算，并支持 `scmethod PICNIC` 与按 cell 计算的 `rfq` 元件（在 CPU 上计算，原内核不能运行它，所以尚未与参考结果核对）；**原内核**（`AVAS.dll`／`libAVAS.so`，只有 Windows 与 Linux x86-64，将淘汰）仍可选择。在 **模拟设置 → 计算内核** 中按项目选择，保存在 `ini.ini` 的 `[engine]` 段；设备只列出本机检测到的，选“自动”时显示本项目的运行会用什么（有 NVIDIA 卡用 CUDA，粒子不足 1000 个（含单粒子）时用 CPU；Mac 上有空间电荷、粒子数至少 6 万且不少于空间电荷网格单元数的一半时用 Metal，粒子较少时 CPU 更快；否则 CPU），精度可选自动／fp64／fp32。没有指定内核的项目都用新内核；新内核修正了原内核的若干缺陷，结果会与以前略有不同，Windows／Linux 更新后第一次启动时会说明，并可一键改回原内核；新内核的“复现 AVAS.dll”模式可以逐位得到原内核的结果。菜单 **设置 → 默认计算内核** 可以一次改变所有未指定内核的项目（保存在 `gui.json`，命令行 `avas run`、研究页和 AI 助手的试算同样遵守；优先级为命令行 > 环境变量 > 项目 `ini.ini` > 全局默认 > 内置默认），在没有原内核的平台上选原内核时改用内置默认并给出说明；模拟设置页会注明项目是否跟随默认。开始运行前检查所选设备与组合，不可用时直接提示。运行页、状态栏、结果页与运行对比标注每次运行实际使用的内核和设备（如“新内核 · Metal fp32”，自动回落到 CPU 时注明原因），可直接对比同一项目的原内核与新内核结果；不同内核或精度的结果因随机数与算术不同而略有差别，运行页“与前一次运行对比”和结果页“运行对比”在两次运行的内核或模式不同时会提示。

原内核与新内核兼容模式（compat = dll）在 `randomseed` 非 0 时生成的初始分布中 x 与相位完全相关，模拟设置页和研究页（研究运行固定种子 12345）会提示；新内核默认模式没有这个问题。没有内核读取 `scanphase` 与 scanData.txt：模拟设置页不再提供相位扫描，文件里已有的 `scanphase` 行会注明并在保存时删去，scanData.txt 仍可在文件页查看。二次粒子（束流页）：新内核默认模式在 beam.txt 没写 `readparticledistribution` 时读取 SeParticle.txt，原内核与兼容模式只读 beam.txt 写明的文件，页面会提示；保存时检查每行至少 11 列并去掉空行；二次粒子运行没有包络（DataSet.txt 为空），结果在 `outData_*.txt` 与 `SeSynParticle.txt`，分段运行不支持二次粒子。Linux 上 `ini.ini` 的 `[input] device = gpu`（libPIC.so）已弃用，请改用新内核的 NVIDIA GPU（`--engine new --device cuda`），将在以后的版本中删除。

新内核默认模式加入了原作者 2026-07-22 版原内核的几项新功能（兼容模式与随附的原内核仍按 2026-04-02 版）：读入 `.dst` 粒子文件时同步粒子能量取 beam.txt 的 `kneticenergy`（写负值取文件平均能量，相差超过 1 % 时运行前与运行日志都会提示，线性预览同样按此取参考能量）；单粒子偏移可写 `initpos`（mm）与 `initmom`（mrad、1e-3），与旧的 `displacepos`／`displacedpos` 同时写时用新的；单粒子运行另写 SingleParticlePlane.txt（每个输出面一行）；`outputplane d NAME` 写成 `NAME_<z>.dst`，间距小于束团长度的输出面可以同时记录；boundary.txt 支持矩形管（3）、椭圆管（4）与 superpose 复合段，一步越过的薄准直器也会判碰撞，CollisionData.txt 末尾多一列 superposeout 块数；Ke = 0 的射频 field 行按漂移段处理（不读场图、不占腔号，线性预览与分段运行相同）；`lpboundary 0 1` 用纵向周期空间电荷盒（束流静止系一个射频周期 γβλ，连续束与长束团适用，支持 GPU）。

#### 结构编辑与文件历史

文本、结构和可视化编辑共用同一份内容与撤销历史。可视化编辑器默认处于浏览状态，点击“编辑”后可拖放元件、调整参数；完成时可保存、放弃本次编辑或继续编辑。

二维布局支持叠加元件：拖到已有非漂移元件图形上可对齐入口叠加；重叠元件上下分层显示，纵向位置保持真实比例。二维／三维右键菜单提供叠加、复制和删除，选中元件可定位对应文本与列表项。仅根据场文件名识别的类型标注“推测”，识别依据见详情。

结构页的文件选择框用于打开文件，运行中仍可切换查看；“设为运行结构”单独指定模拟使用的文件。文件页与结构页提供保存历史和差异对比，恢复先进入编辑器，可撤销，保存后才写入文件。历史存于项目 `.avas_history/`，支持不超过 2 MB 的 lattice、`beam.txt` 和 `input.txt`；从启用后的保存开始记录，版本不自动清理。

#### 场分布、实时显示与结果

元件视图的“查看场图／查看场分布”打开浮动窗口，可查看横截面、纵切面、热力图与方向箭头。场文件显示实际数据，解析模型显示按参数计算的硬边界示意场。横截面统一从下游往上游看，束流朝屏幕外（⊙）；箭头在坐标轴等比例时才表示真实方向。

运行页支持本次和历史运行的回放，结构页可回放上次运行；均可暂停、继续或结束。束团、粒子云和线性包络预览用于示意与快速评估，最终结果以内核计算为准。

完整运行与误差研究在输出目录的 `inputs/` 快照上运行；分段结果写入 `Segments/`，研究页的内核运行写入 `Research/gui/`（与 Python 研究接口共用，旧扫描结果仍在 `Scans/`）。结构页“周期分析”标签给出周期的相移、匹配束和稳定性；结果页可叠加按该次运行输入计算的线性模型。完整运行、误差研究和分段运行期间（含暂停），输入文件只读。已保留的完整运行存入 `Runs/`，可在结果页与其他运行对比。

#### 加速器入门

侧栏最后一页（或“帮助 → 加速器入门”）系统介绍直线加速器：相对论运动学、相空间与线性光学、纵向动力学、空间电荷，每种 AVAS 元件的原理、场与方程、传输矩阵、可调参数和常见问题，以及如何组合成一台加速器。公式以 MathML 显示，参数表直接取自关键字定义；元件详情中的“了解此元件”跳到对应章节。入门从“跟着一个质子走”的故事线开始，依次讲离子源、RFQ、中能传输段、超导加速段和高能传输段为什么需要、由什么组成；各章先讲物理图像和推导，再在“在 AVAS 中”参考小节给出关键字和约定；“初始分布”一章配有 KV／水袋／抛物线／高斯分布的粒子云实验。每个元件章节、带束线的实验和引导教程都配有与结构页相同的 2D／3D 束线示意（叠加线性预览包络），点击元件可查看其详情。左侧章节目录和右侧“本页内容”都可以收起；主界面导航栏除 `Ctrl+B` 收成图标外，还可用 `Ctrl+Shift+B`（视图 → 隐藏侧边栏）完全隐藏。

章节中的交互实验（运动学、相空间椭圆、FODO、螺线管、二极铁、射频腔相位、空间电荷）拖动参数即时计算，使用与线性包络预览相同的代码，属于近似模型。引导教程用示例项目的 HWR 腔和螺线管场图逐步搭出一段超导加速段，可随时用多粒子内核运行对比；运行在 AVAS 数据目录的沙盒中，不修改任何项目，满意后可另存为新项目。内容由 AI 撰写，每章标注“AI 撰写，待审核”，审核前请核对公式和数值；“问讲解员”把当前章节或实验结果交给 AI 助手讲解。

#### AI 助手

工具栏右侧或 `Ctrl+Shift+A` 打开助手。它支持 OpenAI 兼容接口和本地模型，可读取项目、解释结果、提出参数修改，并在副本中执行扫描与优化。在助手设置中配置服务地址、模型并测试连接。未打开项目时也可以询问加速器物理与入门内容。回答前的思考和工具调用收在可展开的“思考过程”里，修改提案和带结果的运行仍单独显示。

每项文件修改以提案展示，经批准后应用；也可在单个对话中开启自动应用。写入前备份到 `.avas_ai/backups/`，试算保存在 `.avas_ai/runs/`（使用项目选择的计算内核），不改项目的 `InputFile/` 与 `OutputFile/`。运行锁期间拒绝输入修改。API 密钥在 Windows 上保存在凭据管理器中，其他平台保存在用户数据目录的 `ai_secrets.json`。

### 源码安装

使用 **64 位 Python 3.11 或更高版本**；开发和测试使用 3.11，3.12／3.13 支持。原内核在 Windows 为 `avas/engine/AVAS.dll`，Linux 为 `libAVAS.so`；新内核的预编译库在 `avas/engine/new/`。解释器与内核平台必须匹配。

以下 Windows 命令在仓库根目录的 **PowerShell** 中执行。请手动创建独立 `.venv`，避免直接在 Anaconda base 中安装和运行；可将第一行的 `python` 换成指定的 Python 3.11 解释器路径。

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install --upgrade pip
.\.venv\Scripts\python.exe -m pip install -e .
.\.venv\Scripts\python.exe -m avas gui
```

上面的命令无需激活环境。想直接使用 `avas` 命令时，PowerShell 执行 `.\.venv\Scripts\Activate.ps1`，CMD 使用 `.venv\Scripts\activate.bat`；Linux 使用 `source .venv/bin/activate` 后执行相同的 pip 安装步骤。

**macOS（Apple 芯片）：** 只支持源码安装（没有安装包），命令行、`avas serve` 与 `avas gui` 均可使用，只有新内核，在 CPU 或 Metal 上计算。新内核优先使用仓库提交的预编译库 `avas/engine/new/libavas_engine.dylib`；没有时需要 Xcode 命令行工具与 CMake 自行构建，必须用 Apple clang（含 Metal；若环境设置了 `CC`／`CXX`，例如 conda，先取消）：

```bash
python3.11 -m venv .venv
.venv/bin/python -m pip install -e .
# 仅当 avas/engine/new/ 中没有 libavas_engine.dylib 时：
env -u CC -u CXX cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build -j
.venv/bin/python -m avas gui
```

AVAS 依次在环境变量 `AVAS_ENGINE_LIB`、`avas/engine/new/` 与 `engine/build/` 中寻找新内核库；`avas info` 显示找到的库、本机设备和本项目会用什么。

仓库中的 `run_avas.py`／`run_avas.cmd` 默认使用本地 `.venv`，可用 `AVAS_NO_VENV=1` 关闭此行为。前端编译结果已随仓库提供，普通源码运行无需 Node.js。开发依赖、构建、调试和打包见 [CONTRIBUTING.md](CONTRIBUTING.md)。

### 命令行与浏览器

以下命令在激活 `.venv` 后执行。`--input` 可指向输入目录或包含 `InputFile/` 的项目目录，`--output` 可指向任意输出目录。结构文件默认读取 `ini.ini` 的 `[lattice] source`，未指定时使用 `lattice_mulp.txt`；`--lattice` 可临时覆盖。

```powershell
# 运行模拟
avas run --input "C:\proj\InputFile" --output "C:\proj\Results_001"

# 指定结构文件；误差研究可使用 --mode stat、dyn 或 stat_dyn
avas run --input "C:\proj\InputFile" --output "C:\proj\Results_002" --lattice lattice_init.txt

# 用新内核运行：设备 auto|cpu|cuda|metal，精度 auto|fp64|fp32，多卡 --gpus 0,1|all
avas run --input "C:\proj\InputFile" --output "C:\proj\Results_003" --engine new --device cuda --precision fp32

# 绘图；不加 --save 时弹出窗口
avas plot emittance_x --output "C:\proj\Results_001" --save emit_x.png
avas plot phase --dst "C:\proj\Results_001\outData_0.210000.dst" --plane x-x1 --plane phi-w

# 参数扫描；结果写入项目 Scans/ 下的 scan.json、scan.csv 及各次输出目录
avas scan --input "C:\proj\InputFile" --target Q1 --param G --values 10,12,14

# 桌面、浏览器、版本与安装自检
avas gui --lang zh_CN
avas serve --open
avas info
avas doctor
```

`avas run --help`、`avas plot --help` 和 `avas scan --help` 提供完整选项。内核选项的优先级为：命令行（`--engine --device --precision --gpus --threads --compat`）> 环境变量（`AVAS_ENGINE`、`AVAS_ENGINE_DEVICE` 等）> 项目 `ini.ini` 的 `[engine]` > 默认值；`--compat dll` 让新内核逐位复现 AVAS.dll（只能在 CPU 上）。所选设备本机没有时运行前报错，不会悄悄改用 CPU；`avas_run.json` 的 `engine` 记录实际使用的内核、设备和精度。运行输入快照保存在 `<输出>/inputs/`，`avas_run.json` 记录其位置；场图从原目录读取。绘图默认从运行记录查找输入目录，也可用 `--input` 指定。

**Python 研究接口：** 在 notebook 或脚本中可直接调用线性模型（`avas.model`）、周期与控制分析（`avas.dynamics`）和研究接口（`avas.research`）。研究接口把项目的“变体”（lattice 参数、beam.txt / input.txt 关键字、种子、粒子数）交给线性模型或内核运行，内核在子进程中运行，结果按输入指纹缓存在 `<项目>/Research/<研究名>/`，可批量、并行、续跑，并可做多维扫描、拉丁超立方采样、线性模型与内核对比、多种子统计和数据导出；不会改动项目的输入和 OutputFile。示例见 `examples/research/`。

**浏览器模式：** `avas serve --open` 启动服务并打开带访问令牌的地址，页面与桌面版共用功能。文件选择使用页面内对话框，文件导出使用下载。默认只监听本机；使用 `--host 0.0.0.0` 开放局域网时，请仅用于可信网络，目前没有用户管理和工作区隔离。关闭标签页不等于停止后端服务。

### 更新与常见问题

#### 如何更新？

Windows 安装器在欢迎页点击“下一步”时检查最新版，可下载并校验新版安装器，保留语言与安装范围。失败时显示原因，可重试、选择“忽略”安装内置版本，或退出；`/NOCHECKUPDATE` 和静默安装使用内置版本。只有包含检查功能的新安装器支持此流程。

已安装的 AVAS 会在启动后检查官方更新，结果缓存 6 小时；**帮助 → 检查更新** 可立即重查。确认“更新并重启”后下载并准备安装，完成后自动重开 AVAS。“稍后”只收起本次会话提醒，“忽略此版本”保存对该提交的忽略记录，均可从帮助菜单重新查看。

下载与准备阶段可取消，关闭更新面板只收起显示；进入安装阶段后不能取消。网络中断后可重试并复用已下载内容。运行、暂停、扫描及 AI 试算期间不能安装，更新准备期间也不能开始新模拟。安装时请等待自动重启，避免手动重复启动。

更新保留项目、结果、个人设置和 `.venv`。macOS 上只支持 Git 方式的自动更新（源码安装）。Git 自动更新要求官方 origin、main 分支、工作区干净且可快进；源码包遇到文件修改或覆盖冲突时停止。源码更新需从安装目录的 `.venv` 启动。浏览器模式需在后端机器更新并重启服务。

#### 更新失败或旧版本无法升级？

先查看错误原因；切换系统代理后可直接重新检查，无需重启。日志、结果与备份位于 `%LOCALAPPDATA%\AVAS\updates\update-*\`。若旧版报告 `Update metadata is too large`，或更新器本身无法升级，请关闭 AVAS，从官方发布页手动安装新 setup；便携版完整解压到新目录。

请保留 `.avas-install.json`／`.avas-source.json` 版本标记。安装中断时保留备份与 `journal.json`，避免手动混合不同版本文件。源码更新的依赖安装失败时，可在安装目录执行：

```powershell
.\.venv\Scripts\python.exe -m pip install -e .
.\.venv\Scripts\python.exe -m pip check
```

#### 在哪里找设置、日志和版本信息？

个人设置位于 `%LOCALAPPDATA%\AVAS\gui.json`，程序日志位于 `%LOCALAPPDATA%\AVAS\logs`。**帮助 → 关于 AVAS → 复制版本信息** 提供版本、提交与构建信息，便于反馈问题。界面快捷键见帮助菜单；`Ctrl+S` 保存，`F5` 运行，`Shift+F5` 停止。

### 文档与反馈

| 资料 | 内容 |
| --- | --- |
| [操作指南](frontend/src/help/manual.zh.md) | 项目设置、编辑、运行、扫描与分析 |
| [案例教程](frontend/src/help/cases.zh.md) | 示例步骤、原始资料与验证状态 |
| [参数与文件参考](frontend/src/help/reference.zh.md) | 输入文件、关键字与物理参考入口 |
| [原始物理手册](docs/使用说明20260427.docx) | 物理说明原文；旧案例是否可运行以教程验证状态为准 |
| [开发指南](CONTRIBUTING.md) · [开发约定](AGENTS.md) | 环境、调试、构建、测试与设计规则 |
| [发布变更记录](docs/changes/) · [早期更新记录](docs/CHANGELOG.md) | 各次改动说明 |

界面内的使用说明提供“操作指南／案例教程／参数与文件参考”，支持搜索和浮动阅读。维护者：Yuchen Lin，反馈邮箱：[yuchenlin@stu.xmu.edu.cn](mailto:yuchenlin@stu.xmu.edu.cn)。

#### 引用

科研工作使用 AVAS 时，请引用：

> C. Jin, Z.-J. Wang, X. Qi, Y. He, K. Li, et al., *Advanced virtual accelerator software: A linear accelerator simulation code*, Phys. Rev. Accel. Beams **28**, 044602 (2025). [DOI](https://doi.org/10.1103/PhysRevAccelBeams.28.044602)

---

## English

AVAS is a beam dynamics simulation code for linear accelerators, with a C++ engine, Python pre/post-processing, desktop and browser interfaces, and command-line tools. The interface supports English and Chinese.

[Download and quick start](#download-and-quick-start) · [Features](#features) · [Source installation](#source-installation) · [Command line and browser](#command-line-and-browser) · [Updates and troubleshooting](#updates-and-troubleshooting) · [Documentation and feedback](#documentation-and-feedback)

### Download and quick start

Choose a package from the [official releases](https://github.com/lycself/AVAS_XMU/releases):

| Installation | Download | Start |
| --- | --- | --- |
| Windows installer (recommended) | `AVAS-<version>-setup.exe` | Install, then use the shortcut |
| Windows portable | `avas-windows.zip` | Extract the entire archive and run `AVASGui.exe` |
| Source or development | `avas-source.zip`, or clone the repository | Follow Source installation below |

Windows packages need no Python, Git or GitHub account. Keep the portable directory intact; copying only the executable will not work. GitHub's **Source code** attachments contain source files; `update.json` is application metadata and needs no manual download.

The desktop interface requires Microsoft Edge WebView2 and prompts for installation when missing. The bundled AVAS can be installed offline, but a missing WebView2 runtime requires a download. The installer supports English and Chinese; AVAS adopts that language on first launch and preserves saved preferences afterward.

For your first run:

1. Open AVAS and create or open a project. The source repository includes the [hwr010 example](examples/hwr010).
2. Review Beam, Lattice and Settings. To change the simulation's lattice file, use Set as run lattice on the Lattice page.
3. Start a run, follow progress and live envelopes on Run, then analyze output on Results.
4. Use Keep this run to retain results; the next full run overwrites `OutputFile/`.

Open **Help → User manual** for offline instructions, or read the [user guide](frontend/src/help/manual.en.md).

### Features

The sidebar follows the simulation workflow — Project, Beam, Lattice, Simulation settings, Files, Run, Results — followed by Study, with the Primer pinned at the bottom (`Ctrl+1` … `Ctrl+9` follow this order). The interface supports light/dark themes, UI scaling, and mouse/touchpad gestures.

| Page | Purpose |
| --- | --- |
| Project | Create and open projects; inspect inputs and the last run |
| Beam | Configure particles, energy, distributions and Twiss parameters; read a `.dst` file and show its particles; the secondary beam switch and SeParticle.txt |
| Lattice | Edit linked text and parameters; validate keywords; inspect 2D and 3D beamlines |
| Simulation settings | Choose the computing kernel, device and precision; configure simulation type, step size, space charge, threading and error studies; edit the loss boundary (boundary.txt) |
| Files | Browser of InputFile: raw text, field-map profiles and slices, particle files; import, duplicate, rename |
| Run | Start, pause and stop simulations; inspect live envelopes, records and replay |
| Results | Analyze envelopes, emittance, losses and energy; compare runs and export PNG/PDF/SVG |
| Study | Parameter scans (linear model or engine, 1D / 2D heat maps), period stability charts, response matrix and controllability, feedback simulation |
| Primer | Accelerator physics, the physics and parameters of every component, interactive labs and a guided tutorial; no project needed |

#### Computing kernels
AVAS has two computing kernels: the rebuilt **new kernel** (the default on every platform), which computes on the CPU, NVIDIA GPUs (CUDA) and Mac GPUs (Metal) and supports `scmethod PICNIC` and the cell-by-cell `rfq` element (computed on the CPU; the original kernel cannot run it, so it is not yet checked against reference results), and the **original kernel** (`AVAS.dll` / `libAVAS.so`, Windows and Linux x86-64 only, being retired), which can still be chosen. Choose per project in **Simulation settings → Computing kernel**; the choice is stored in the `[engine]` section of `ini.ini`. Only the devices detected on this computer are offered; with Automatic the page shows what this project's runs will use (CUDA when an NVIDIA GPU is present, the CPU for fewer than 1,000 particles including single-particle runs; on a Mac, Metal for runs with space charge, at least 60,000 particles and at least half as many particles as space-charge mesh cells, since the CPU is faster for fewer particles; otherwise the CPU), and precision can be Automatic, fp64 or fp32. Projects that do not choose a kernel run with the new kernel. It corrects several defects of the original kernel, so results differ slightly from earlier runs; on Windows and Linux the first start after the update explains this and offers to return to the original kernel, and the new kernel's mode "Reproduce AVAS.dll" gives the original kernel's results bit for bit. **Settings → Default computing kernel** changes the kernel of every project that does not choose one (stored in `gui.json` and also followed by `avas run` on the command line, the Study page and the AI assistant's trial runs; priority: command line > environment variables > project `ini.ini` > global default > built-in default); choosing the original kernel on a platform without it falls back to the built-in default with a note, and Simulation settings shows whether a project follows the default. The device and combination are checked before a run starts. Run, the status bar, Results and run comparison label each run with the kernel and device it actually used (e.g. "New kernel · Metal fp32", with the reason when Automatic fell back to the CPU), so original-kernel and new-kernel runs of one project can be compared directly; results of different kernels or precisions differ slightly because of random-number streams and arithmetic, and the Run page's "compare with the run before" and the Results page's run comparison point out when two runs used different kernels or modes.

With the original kernel and the new kernel's compatibility mode (compat = dll), a `randomseed` other than 0 gives an initial distribution with x fully correlated with the phase; Simulation settings and the Study page (research runs use the fixed seed 12345) warn about it, and the new kernel's default mode does not have the defect. No kernel reads `scanphase` or scanData.txt: Simulation settings no longer offers a phase scan, an existing `scanphase` line is pointed out and removed on save, and scanData.txt can still be viewed on the Files page. Secondary particles (Beam page): the new kernel's default mode reads SeParticle.txt when beam.txt has no `readparticledistribution`, while the original kernel and the compatibility mode only read the file beam.txt names, which the page points out; saving checks that every line has at least 11 columns and drops blank lines; secondary-particle runs have no envelope (DataSet.txt is empty), their results are in `outData_*.txt` and `SeSynParticle.txt`, and segment runs do not support them. On Linux, `[input] device = gpu` in `ini.ini` (libPIC.so) is deprecated in favour of the new kernel's NVIDIA GPU (`--engine new --device cuda`) and will be removed in a later release.

The new kernel's default mode adds several features of the authors' 2026-07-22 original kernel (the compatibility mode and the bundled original kernel stay at the 2026-04-02 version): with a `.dst` particle file the synchronous particle starts with beam.txt `kneticenergy` (a negative value: the mean energy of the file; more than 1 % away from the mean is pointed out before the run and in the run log, and the linear preview takes the same reference energy); a single particle can be displaced with `initpos` (mm) and `initmom` (mrad, 1e-3), which win over the old `displacepos` / `displacedpos` when both are written; single-particle runs also write SingleParticlePlane.txt (one line per output plane); `outputplane d NAME` is written as `NAME_<z>.dst` and planes closer together than the bunch length are recorded at the same time; boundary.txt accepts rectangular (3) and elliptical (4) pipes and composite superpose segments, a thin collimator crossed within one step is hit as well, and CollisionData.txt ends with one more column (superposeout blocks passed); an RF `field` row with Ke = 0 is a drift (no field map, no cavity number; the same in the linear preview and segment runs); `lpboundary 0 1` gives a periodic longitudinal space-charge box (one RF period γβλ in the beam rest frame, for continuous beams and long bunches, also on GPUs).

#### Lattice editing and file history

Text, structure and visual editing share one document and undo history. The visual editor starts in browse mode; click Edit to insert elements or change parameters. Finishing offers saving, discarding session edits or continuing.

The 2D layout supports superposition: dropping onto a non-drift element's glyph aligns the new element to its entrance. Overlapping elements appear in separate rows while retaining their longitudinal coordinates. The 2D/3D context menus offer superposition, duplication and deletion; selecting an element locates its text and list entry. Types inferred only from field filenames are marked as inferred, with the basis explained in the details.

The Lattice file selector opens a document and remains available during runs; Set as run lattice separately chooses the simulation input. Files and Lattice offer saved history and comparisons. Restoring loads an undoable edit and requires saving to write the file. History lives in the project's `.avas_history/`, supports lattice, `beam.txt` and `input.txt` up to 2 MB, starts with saves after the feature is enabled, and is not automatically pruned.

#### Fields, live views and results

View field map / View field distribution opens a floating window with transverse and longitudinal slices, heatmaps and arrows. Field files supply actual data; analytical models supply schematic hard-edge fields calculated from parameters. Transverse views look upstream from downstream, with the beam coming out of the screen (⊙). Arrow directions represent physical directions only with equal axis scaling.

Run replays current and historical results; Lattice replays the last run. Both support pause, resume and end. Bunches, particle clouds and linear envelope previews are schematic or preliminary views; use engine results for final analysis.

Full runs and error studies use an `inputs/` snapshot in the output directory. Segment results go to `Segments/`; engine runs of the Study page go to `Research/gui/` (shared with the Python research interface; older scans stay in `Scans/`). The Periods tab of the Lattice page shows phase advances, matched beam and stability of a period; Results can overlay the linear model of a run's own inputs. Inputs are read-only during full runs, error studies and segmented runs, including pauses. Retained full runs are stored in `Runs/` and can be compared on Results.

#### Accelerator primer

The last sidebar page (or Help → Accelerator primer) introduces linear accelerators systematically: relativistic kinematics, phase space and linear optics, longitudinal dynamics, space charge; for every AVAS component its principle, fields and equations, transfer matrix, adjustable parameters and pitfalls; and how components combine into an accelerator. Formulas are shown as MathML and parameter tables come straight from the keyword definitions; Learn about this element in the element details opens the matching chapter. The primer starts with the story line "A proton's journey": why an ion source, RFQ, medium-energy transport, superconducting sections and high-energy transport are needed and what each is made of. Every chapter first builds the physical picture and the derivations, then gives keywords and conventions in its "In AVAS" reference section; the initial-distributions chapter has a lab with KV / waterbag / parabolic / Gaussian particle clouds. Every element chapter, the labs with a beam line and the guided tutorial show the same 2D / 3D beam-line view as the Lattice page (with the linear preview envelope); click an element for its details. The chapter list on the left and On this page on the right can be hidden; besides collapsing to icons with `Ctrl+B`, the main navigation can be hidden completely with `Ctrl+Shift+B` (View → Hide sidebar).

The interactive labs (kinematics, phase-space ellipse, FODO, solenoid, dipole, RF cavity phase, space charge) recompute while you drag the parameters, with the same code as the linear envelope preview, so they are approximate models. The guided tutorial builds a superconducting accelerating section step by step from the example project's HWR cavity and solenoid maps and can compare with the multi-particle engine at any time; engine runs use a sandbox in the AVAS data folder, change no project, and can be saved as a new project. The content is written by AI and every chapter is marked "AI-written, awaiting review": check formulas and numbers before relying on them. Ask the tutor hands the current chapter or lab result to the AI assistant for an explanation.

#### AI assistant

Open the assistant from the toolbar or with `Ctrl+Shift+A`. It supports OpenAI-compatible services and local models, reads projects, explains results, proposes parameter changes, and runs scans or optimization in copies. Configure the endpoint and model and test the connection in assistant settings. Without an open project you can still ask about accelerator physics and the primer. The thinking and tool calls before an answer are folded into one expandable Thinking block; change proposals and runs with results stay visible.

Each file change is a proposal requiring approval; automatic application can be enabled for an individual conversation. Changes are backed up under `.avas_ai/backups/`. Trial runs use `.avas_ai/runs/` (with the project's computing kernel) without changing project `InputFile/` or `OutputFile/`. Input changes are refused while the run lock is active. API keys are stored in Windows Credential Manager on Windows and in `ai_secrets.json` in the user data folder elsewhere.

### Source installation

Use **64-bit Python 3.11 or newer**. Development and tests use 3.11; 3.12/3.13 are supported. The original kernel is `avas/engine/AVAS.dll` on Windows and `libAVAS.so` on Linux; the prebuilt libraries of the new kernel are in `avas/engine/new/`. Interpreter and kernel platforms must match.

Run these Windows commands in **PowerShell** at the repository root. Create a dedicated `.venv` manually, avoiding direct installation into Anaconda base. Replace the first `python` with a specific Python 3.11 interpreter path if needed.

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install --upgrade pip
.\.venv\Scripts\python.exe -m pip install -e .
.\.venv\Scripts\python.exe -m avas gui
```

These commands need no activation. To use `avas` directly, activate with `.\.venv\Scripts\Activate.ps1` in PowerShell or `.venv\Scripts\activate.bat` in CMD. On Linux use `source .venv/bin/activate`, then the same pip installation steps.

**macOS (Apple silicon):** source installation only (no installer). The command line, `avas serve` and `avas gui` all work; only the new kernel is available, computing on the CPU or with Metal. The new kernel uses the prebuilt library committed in `avas/engine/new/libavas_engine.dylib`; without it, build the library with the Xcode command-line tools and CMake, using Apple's clang (which includes Metal; unset `CC` / `CXX` if your environment, e.g. conda, sets them):

```bash
python3.11 -m venv .venv
.venv/bin/python -m pip install -e .
# only when avas/engine/new/ has no libavas_engine.dylib:
env -u CC -u CXX cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build -j
.venv/bin/python -m avas gui
```

AVAS looks for the new kernel's library in the environment variable `AVAS_ENGINE_LIB`, then `avas/engine/new/`, then `engine/build/`; `avas info` shows the library found, this computer's devices and what the project's runs will use.

The repository's `run_avas.py` / `run_avas.cmd` use its `.venv` by default; `AVAS_NO_VENV=1` disables this behavior. Built frontend assets are included, so ordinary source installations need no Node.js. See [CONTRIBUTING.md](CONTRIBUTING.md) for development dependencies, builds, debugging and packaging.

### Command line and browser

Activate `.venv` before using these commands. `--input` accepts an input directory or a project containing `InputFile/`; `--output` accepts any output directory. The lattice defaults to `[lattice] source` in `ini.ini`, falling back to `lattice_mulp.txt`. Override it for one run with `--lattice`.

```powershell
# Run a simulation
avas run --input "C:\proj\InputFile" --output "C:\proj\Results_001"

# Select a lattice; error studies accept --mode stat, dyn or stat_dyn
avas run --input "C:\proj\InputFile" --output "C:\proj\Results_002" --lattice lattice_init.txt

# Run with the new kernel: device auto|cpu|cuda|metal, precision auto|fp64|fp32, several GPUs with --gpus 0,1|all
avas run --input "C:\proj\InputFile" --output "C:\proj\Results_003" --engine new --device cuda --precision fp32

# Plot; omit --save to open a window
avas plot emittance_x --output "C:\proj\Results_001" --save emit_x.png
avas plot phase --dst "C:\proj\Results_001\outData_0.210000.dst" --plane x-x1 --plane phi-w

# Scan; writes scan.json, scan.csv and per-run output under project Scans/
avas scan --input "C:\proj\InputFile" --target Q1 --param G --values 10,12,14

# Desktop, browser, version and installation diagnostics
avas gui --lang en
avas serve --open
avas info
avas doctor
```

Use `avas run --help`, `avas plot --help` and `avas scan --help` for full options. Kernel options take precedence in this order: the command line (`--engine --device --precision --gpus --threads --compat`) > environment variables (`AVAS_ENGINE`, `AVAS_ENGINE_DEVICE`, …) > `[engine]` in the project's `ini.ini` > defaults; `--compat dll` makes the new kernel reproduce AVAS.dll bit for bit (CPU only). A device this computer does not have is an error before the run, never a silent switch to the CPU; the `engine` entry of `avas_run.json` records the kernel, device and precision actually used. Input snapshots live in `<output>/inputs/` and are recorded in `avas_run.json`; field maps are read from their original directory. Plotting finds inputs from the run record unless overridden by `--input`.

**Python research interface:** notebooks and scripts can use the linear model (`avas.model`), period and control analysis (`avas.dynamics`) and the research interface (`avas.research`). It runs "variants" of a project (lattice parameters, beam.txt / input.txt keywords, seed, particles) with the linear model or the engine; engine runs are child processes, cached by an input fingerprint under `<project>/Research/<study>/`, and can be batched, run in parallel and resumed, with multi-dimensional scans, Latin-hypercube samples, linear-vs-engine comparisons, seed ensembles and data exports. The project's inputs and OutputFile are never changed. Examples are in `examples/research/`.

**Browser mode:** `avas serve --open` starts the service and opens a token-bearing URL. It shares the desktop pages, with in-page file dialogs and downloads for exports. The default listener is local only. Use `--host 0.0.0.0` only on trusted networks: user management and workspace isolation are not implemented. Closing the tab does not stop the backend service.

### Updates and troubleshooting

#### How do I update?

Windows Setup checks for a newer installer when you click Next on the welcome page. It can download and verify the new installer while retaining language and installation scope. Failures show their cause and offer Retry, Ignore (install the bundled version), or Abort. `/NOCHECKUPDATE` and silent installation use the bundled version. Only installers containing this feature support discovery.

Installed AVAS checks official updates after startup and caches results for six hours. **Help → Check for updates** checks immediately. Confirm Update and restart to download, prepare and install; AVAS reopens automatically. Later hides the notice for the current session; Ignore this version persists for that revision. Both remain accessible through Help.

Download and preparation can be cancelled; closing the panel only hides it. Installation cannot be cancelled. Retry after a network interruption can reuse downloaded content. Installation is unavailable during running or paused simulations, scans and AI trials; preparation also blocks new simulations. Wait for the automatic restart instead of launching another instance.

Updates preserve projects, results, preferences and `.venv`. On macOS only Git-based automatic updates (source installation) are supported. Git updates require the official origin, main branch, a clean working tree and a fast-forward path. Source archives stop on local file changes or overwrite conflicts. Source updates must run from the installation's `.venv`. Browser deployments require updating and restarting the service on the backend machine.

#### An update failed or an older version cannot upgrade

Check the reported cause. After changing a system proxy, check again without restarting. Logs, results and backups live under `%LOCALAPPDATA%\AVAS\updates\update-*\`. If an old version reports `Update metadata is too large`, or its updater cannot upgrade itself, close AVAS and manually install a new official setup. Extract portable updates in full into a new directory.

Keep `.avas-install.json` / `.avas-source.json` version markers. After an interrupted installation, retain backups and `journal.json` and avoid mixing files from different versions. If dependency installation fails during a source update, run in the installation directory:

```powershell
.\.venv\Scripts\python.exe -m pip install -e .
.\.venv\Scripts\python.exe -m pip check
```

#### Where are settings, logs and version details?

Preferences are in `%LOCALAPPDATA%\AVAS\gui.json`, and application logs in `%LOCALAPPDATA%\AVAS\logs`. **Help → About AVAS → Copy version information** provides version, revision and build details for reports. Help lists keyboard shortcuts; `Ctrl+S` saves, `F5` runs and `Shift+F5` stops.

### Documentation and feedback

| Resource | Contents |
| --- | --- |
| [User guide](frontend/src/help/manual.en.md) | Project setup, editing, runs, scans and analysis |
| [Case tutorials](frontend/src/help/cases.en.md) | Examples, original sources and verification status |
| [Parameter and file reference](frontend/src/help/reference.en.md) | Input files, keywords and physics reference links |
| [Original physics manual](docs/使用说明20260427.docx) | Original Chinese reference; consult tutorial verification status for older cases |
| [Contributing](CONTRIBUTING.md) · [Development conventions](AGENTS.md) | Environment, debugging, builds, tests and design rules |
| [Release change records](docs/changes/) · [Earlier changelog](docs/CHANGELOG.md) | Descriptions of changes |

The in-app manual includes Guide, Cases and Reference with search and a floating reading window. Maintainer: Yuchen Lin, [yuchenlin@stu.xmu.edu.cn](mailto:yuchenlin@stu.xmu.edu.cn).

#### Citation

If you use AVAS in research, please cite:

> C. Jin, Z.-J. Wang, X. Qi, Y. He, K. Li, et al., *Advanced virtual accelerator software: A linear accelerator simulation code*, Phys. Rev. Accel. Beams **28**, 044602 (2025). [DOI](https://doi.org/10.1103/PhysRevAccelBeams.28.044602)
