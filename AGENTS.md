# AVAS 开发约定（给 AI 编程助手和开发者）

本文件是本仓库**唯一的**开发规则来源，Codex、Cursor、Copilot、Claude Code 等工具都应先读它（`CLAUDE.md` 只是引用本文件）。
改动设计时，同一次修改里同步更新本文件。从头到尾的构建 / 调试 / 打包步骤见 [CONTRIBUTING.md](CONTRIBUTING.md)（面向人类贡献者，规则仍以本文件为准，两者不要各写一份互相矛盾的步骤）。

## 1. 项目概况

AVAS（Advanced Virtual Accelerator Software）是直线加速器束流动力学模拟程序：

- C++ 计算内核：`avas/engine/AVAS.dll`（Linux 为 `libAVAS.so`），只有二进制，没有源码，不要修改。新内核（`engine/` 源码，预编译库将放在 `avas/engine/new/`）是可选内核，有原内核的平台（Windows、Linux x86-64）默认仍用原内核，没有原内核的平台（macOS、ARM）默认用新内核（`avas/engines.py`，见第 5.4 节“内核选择”）。
- Python 包 `avas/`：模拟流程、前后处理、命令行 `avas run / plot / gui / serve / info / doctor`。
- 界面：React 网页前端（`frontend/`）+ Python 后端（`avas/gui/services/`），二者只通过 HTTP / WebSocket 通信（`avas/gui/server.py`）。
  `avas gui` 把页面放进 pywebview + Edge WebView2 桌面窗口；`avas serve` 不开窗口，在浏览器里用同一个页面（将来部署到服务器的基础，
  目前没有用户管理和工作区隔离，只在本机或可信网络使用）。
- 右侧 AI 助手：OpenAI 兼容接口（含本地模型），修改必须经用户批准。

用户是加速器物理研究人员，界面有中文 / 英文，**和用户交流用中文**。

## 2. 目录地图

```
avas/
  cli/            命令行入口（main.py；scan_cmd.py 是 avas scan）
  gui/            界面后端：server.py HTTP / WebSocket 服务（RPC、事件、blob、下载、令牌），bridge.py RPC 注册与事件队列，
                  app.py 桌面窗口（avas gui），serve.py 浏览器模式（avas serve），devserver.py 旧命令的别名
    services/     每个页面一个模块，函数用 @rpc("page.action") 注册；fs.py 给页面自带的文件选择器列目录；runs.py 运行记录归档；scan.py 旧版参数扫描（Scans/，命令行仍用）；study.py 研究页扫描（Research/gui/）；analysis.py 周期面板、稳定区图谱、响应与可控性、反馈；learn.py 入门页的实验与教程；kernels.py 计算内核（`engine.info / preview / check / effective`、运行前检查 `require_ready`、运行记录标签 `summary`、模拟设置页的 `[engine]` 表单）
    web/          前端编译结果（提交到仓库，用户不需要 Node.js）
  ai/             AI 助手：client.py、agent.py、avas_tools.py（工具）、sandbox.py（副本上运行模拟）、avas_prompt.py
  model/          线性模型（唯一的传输矩阵实现）：beam.py 束流与初始矩，elements.py 元件矩阵，fieldmaps.py 场图剖面缓存，tracker.py 参考粒子、切片矩阵与场图段缓存，
                  spacecharge.py 线性空间电荷，model.py `LinearModel` / `ModelResult`（全分辨率 σ、质心、传输矩阵、Twiss、相移、旋钮与误差）
  dynamics/       基于线性模型的动力学与控制分析（notebook 使用，暂无界面）：period.py 周期 Floquet 分析与带空间电荷的匹配，envelope.py 包络映射雅可比，
                  kv.py 二维 KV 包络方程参考，grid.py 二维参数图，diagnostics.py 监测器与观测，response.py 旋钮与响应矩阵，systems.py 周期的 (A, B, C, D)，
                  control.py 可控性、可观性、模态分类与闭环，feedback.py 脉冲间反馈仿真，study.py 研究配置文件 study.json（监测器、旋钮、误差，不改 lattice）
  engines.py      内核选择层：`EngineChoice`（kernel original / new，device、precision、gpus、threads、compat）、优先级（命令行 > 环境变量 > ini.ini `[engine]` > 全局默认内核 > 内置默认）、
                  新内核库的查找、结果档 `result_tier`、引擎源码哈希 `engine_source_hash`（与 engine/cmake/source_hash.cmake 同一算法）、avas_run.json 的 `engine` 字段
  research/       notebook / 脚本研究接口：variant.py 变体，study.py `Study`（运行、指纹缓存、记录、table），engine.py 内核子进程，runs.py 两种保真度的统一结果，
                  batch.py 批量与并行，scan.py 多维扫描与拉丁超立方，compare.py 线性模型对内核、多种子统计、内核响应矩阵，export.py npz/CSV，plots.py 作图
  learn/          加速器入门的后端：labs.py 交互实验（复用 linear_optics），tutorial.py 引导教程（示例场图搭 lattice、另存为项目）
  data/           schema.py（全部关键字的参数、单位、中英文说明）、lattice_doc.py（lattice 解析与检查）、segment.py（分段运行物理）、
                  lattice_edit.py（按名称 / 行号定位元件并改一个参数，beam/input 关键字改值；AI 工具、扫描、命令行共用）、inputs.py（复制文本输入）
  sim/            模拟流程；linear_optics.py 线性包络预览（`avas.model` 之上的薄包装，并重新导出旧名称）；error.py 误差研究；scan.py 参数扫描（GUI 与命令行共用）
  post/           后处理（analysis/、plot/）
  core/ api/ utils/ engine/ static/ gpu/    api/basic.py 只有运行入口（惰性导入），api/plotting.py 是命令行画图；无人调用的旧代码在仓库外 archive/
                  engine/ 是原内核二进制，engine/new/ 放新内核的预编译库（CI 构建、人工提交，带 build-stamp.json；见第 5.4 节“新内核的预编译库”）；core/MultiParticleEngine.py 按 `EngineChoice` 加载其中之一
                  utils/readfile.py 的 read_lattice_mulp* 只是 LatticeDocument 之上的适配层（tests/test_lattice_parsers.py 有逐字节金标准）；
                  utils/keywordconfig.py 让 BeamConfig / InputConfig 的键表和类型来自 schema.py，不要再手写键列表；constants.py 的关键字集合同样派生自 schema
frontend/src/
  bridge.ts       与后端的通信（fetch + WebSocket、令牌、blob）；host.ts 桌面窗口 / 浏览器的差异（文件对话框、打开文件、退出、缩放）
  components/     ui.tsx（按钮、输入框等基础组件）、overlays.tsx（菜单、对话框、toast）、Plot.tsx（Plotly 封装）、FileDialog.tsx（浏览器模式的文件选择器）
  pages/          九个页面（StudyPage 是研究页，页面 id 仍为 "scan"；LearnPage 是加速器入门）
  study/          研究页的四个标签：ScanTab（一维／二维扫描）、ChartTab（稳定区图谱）、ControlTab（响应与可控性、反馈）
  learn/          入门：content/*.zh.md / *.en.md 章节，chapters.ts 章节表与审核状态，directives.ts 内嵌块，LabView / TutorialView / FieldView / ParamsTable / BeamlineView，MathView（temml）
  lattice/        文本编辑器（Monaco）、结构编辑器、可视化编辑器、2D 布局、3D 视图（Beamline3D.tsx 只是 React 壳，b3dViewer.ts / b3dHud.ts / b3dGeometry.ts 单向依赖）
  results/ files/ assistant/ shell/ store/（zustand）；util.ts（basename、cssColor、niceStep、debounce）；shell/shortcuts.ts（全部快捷键的唯一表）；results/CompareTab.tsx 运行对比
  i18n/zh_CN.json 中文翻译（英文原文 → 中文）
  styles/tokens.css 全部颜色变量（浅色 + 深色）
tests/            pytest；test_gui.py 覆盖页面调用的每个接口；前端 vitest（frontend/src/**/*.test.ts）
tools/            手动运行的开发工具（不属于快速 pytest）：compare_preview_engine.py 用真实内核对比新旧线性预览，compare_centroid_engine.py 用单粒子运行核对线性模型的质心，compare_response_engine.py 用小发射度多粒子束核对轨道响应矩阵（步骤见 CONTRIBUTING.md）
engine/           新计算内核（重建中，原内核源码已丢失；在 engine-rebuild 分支开发，全部功能对齐前不合并 main，说明见 engine/README.md，规格在 docs/engine/）：src/ C++ 源码（CMake 构建），tests/parity/ 与原 AVAS.dll 的比对，docker/ Linux 构建环境（manylinux2014 + CUDA 12.4），tools/ 工具链检查、原内核比对实验与 Ghidra 导出脚本，cmake/source_hash.cmake 引擎源码哈希；逆向导出的伪代码不进仓库。从源码运行时 `avas run --engine new`（或 `AVAS_ENGINE=new`、ini.ini `[engine] kernel = new`）加载 engine/build 下的新库，`--compat dll` 复现原内核缺陷
.github/workflows/ci.yml  CI：ruff + 快速 pytest（Windows，含 smoke）、npm test + build
.github/workflows/engine.yml  新内核三平台（Windows、Linux、macOS）构建与比对，产物供 `engine/tools/fetch_ci_engine.py` 取用；engine/tools/ci_engine.py 是 CI 的检查与构建记录
examples/cluster/ Linux 集群（Slurm）运行新内核的示例脚本
examples/hwr010/  示例项目（跑一次约 10–30 秒）；examples/research/ 研究接口示例脚本（`# %%` 分格，VS Code / Jupyter 可逐格运行）
docs/             使用说明（以 使用说明20260427.docx 为准，121 版已过时）、CHANGELOG
scripts/          用户个人分析脚本，不属于软件本体，不要重构
packaging/        PyInstaller + Inno Setup 打包
```

## 3. 环境与常用命令（Windows）

- Python：只用仓库里的 `.venv`（Python 3.11），命令写 `.venv\Scripts\python.exe`。**不要用 Anaconda base 解释器**；不要新增创建 venv 的安装脚本（用户明确不要，README 写手动步骤）。
- 测试：`.venv\Scripts\python.exe -m pytest`。`tests/test_gui.py`、`tests/test_segment.py`、`tests/test_scan.py` 会真正运行内核，较慢；改哪部分至少跑对应的测试文件。
- 静态检查：`.venv\Scripts\python.exe -m ruff check avas tests packaging tools`（只开缺陷类规则，见 pyproject.toml；旧代码不重排风格）。前端 `cd frontend && npm test`（vitest，纯函数测试）。
- 前端：`cd frontend && npm run build`（先 `tsc --noEmit` 再 `vite build`），输出到 `avas/gui/web/` 并写 `source-hash.json`。**改了前端就要重新构建，并把 `avas/gui/web/` 一起提交。**
- 浏览器调试界面：`.venv\Scripts\python.exe -m avas serve --port 8765 --token dev --settings <临时 json>`，打开它打印的地址
  （`http://127.0.0.1:8765/index.html?host=browser&token=dev`；`python -m avas.gui.devserver` 是同样效果的旧命令）。改了 Python 代码要重启它。
  脚本调用后端：`POST /api/rpc`（JSON `{"method","params"}`，头 `X-AVAS-Token`），事件用 WebSocket `/api/events?token=…`。
  加 `--fake-engine 60 [--fake-lose 0.1]` 时运行不调用内核，而是重放输出目录里已有的 DataSet.txt（`avas/gui/fake_engine.py`），用来检查实时显示。
  **不要在用户的真实项目上用假引擎**（它会改写 OutputFile），先把项目复制到临时目录。
- 打包：`python packaging/build.py`。生成的 exe 不会随源码更新，改代码后要重新打包。
- 模拟结果每次运行略有差别（内核多线程），比较 DataSet.txt 要带容差，不能要求完全相同。

## 4. 修改完成前的检查清单

1. 相关的 pytest 通过；前端有改动时 `npm run build` 成功，且 `tests/test_design_rules.py` 通过（颜色、翻译、构建是否过期）。
2. 新增的界面文字都用 `t("English")`，并在 `frontend/src/i18n/zh_CN.json` 加中文。
3. 新增颜色只加在 `styles/tokens.css`，浅色和深色两套都要给。
4. 新增或修改的后端接口在 `tests/test_gui.py`（或对应测试文件）里有测试。
5. 改动物理计算时，用示例项目跑内核对比，并说明误差。
6. 用户能感觉到的行为变化写进 `README.md`（中英文两部分）；设计规则变化写进本文件。README 中英文采用相同章节结构，依次为下载与快速开始、主要功能、源码安装、命令行与浏览器、更新与常见问题、文档与反馈。新增说明并入对应章节，保持入口简明；详细操作放使用说明，构建与发布步骤放 `CONTRIBUTING.md`，历次修复放新变更记录，不在 README 末尾追加零散段落。
7. 不要自行 `git commit` / `push`，除非用户要求。

## 5. 不能破坏的设计

### 5.1 界面技术与风格

- 侧栏、首页和关于统一显示版本号与 7 位短提交号；侧栏／首页不显示日期，本地修改以星号和提示标识。关于提供完整提交号、提交时间、软件／界面构建时间及复制版本信息。Git 源码以当前 HEAD 为准，不读遗留打包标记；源码 ZIP 和打包版复用更新模块的发布清单／export-subst 标记，无法识别时明确显示未知。提交时间写入源码归档标记与打包标记；界面构建时间写入 `source-hash.json`，不得用文件修改时间替代。旧包缺少记录时显示未知。

- 构建时间以带时区的 ISO 8601 保存（UTC），关于弹窗按客户端本地时区显示并注明 UTC 偏移；旧包无时区的时间不猜测，标注时区未知。关于弹窗单列参考文献与反馈区（维护者 Yuchen Lin，yuchenlin@stu.xmu.edu.cn）。

- 输入设备模式为全局设置 `ui/pointerDevice`（自动／鼠标／触控板），设置页、设置菜单与 3D 快捷入口同步；未设置时沿用旧 `ui/b3dPointer`。2D、3D 与 Plotly 共用手势识别，鼠标滚轮缩放，触控板双指平移、捏合缩放；二维束线图将纵向滑动也映射到束线方向。选择跨重启保存，自动识别按手势更新。

- 使用说明在当前窗口内以非模态浮动页面显示，不遮罩、不阻挡工作区操作；支持拖动、边缘缩放、最大化和还原。每次关闭再打开恢复默认位置和大小，不持久化几何状态。中英文操作指南在 `frontend/src/help/manual.*.md`，随前端构建离线打包；参数参考读取 `schema.all`，不另写参数表。Word 保留为物理原始参考。浮动页分“操作指南／案例教程／参数与文件参考”三栏；搜索跨栏并显示来源，Markdown 用稳定章节 ID 互链。`cases.*.md` 和 `reference.*.md` 保留来源与核对状态，原案例片段在 `help/cases/`，迁移清单与验证条件随源代码保留；未复现的旧包络／匹配流程不得标为当前可运行功能。

- 技术栈固定：React 19 + TypeScript + Vite + zustand 前端，FastAPI + uvicorn 后端服务，桌面窗口用 pywebview；文本编辑用 Monaco，图用 Plotly，3D 用 three.js。不要引入其他 UI 框架或组件库。
- **桌面与浏览器两种宿主**：页面地址里的 `host=webview|browser` 决定（`bridge.ts` 的 `isDesktop()`）。所有依赖宿主的操作
  （文件对话框、打开文件 / 文件夹、外部链接、退出、缩放）只写在 `host.ts` 里，页面调用 `pickFolder` / `pickFile` / `openPath` 等，
  **不要在页面里直接调 `dialog.*` / `shell.*` / `app.quit` / `app.zoom`**。浏览器里用 `components/FileDialog.tsx`（后端 `fs.list`）、下载链接 `/download?path=`。
- 优先复用 `components/ui.tsx`、`components/overlays.tsx`（`openMenu`、`choiceDialog`、`toast`、`reportError` 等）、`components/Plot.tsx`。
- 非模态浮动窗口一律用 `components/FloatingWindow.tsx`（拖动、八向缩放、最大化／还原、Esc 关闭、跟随界面缩放），使用说明和元件场分布窗口都基于它；几何不持久化，每次打开回到 `initialRect`。不要再各写一套拖拽缩放。
- **颜色**只来自 `styles/tokens.css` 的 CSS 变量；canvas 和 three.js 通过 `getComputedStyle` / `cssColor()` 读取变量。确实需要写死颜色时（Plotly 调色板、Monaco 主题）在该行注明 `design:allow-colour 原因`。主题切换必须保持一帧内完成，不要做逐元素重新计算样式的方案。
- **文字**：界面文字写英文原文 `t("...")`，中文放 `zh_CN.json`；带参数用 `t("… {name}", { name })`。关键字、参数的物理说明来自 `avas/data/schema.py` 的中英文对照，前端用 `pick()` 选择，不要在组件里另写说明文字。
- 布局仿 VS Code：菜单栏、左侧导航、页面、下方日志面板、状态栏。左侧导航按 `store/app.ts` 的 `PAGE_GROUPS` 分三组：工作流（项目、束流、结构、模拟设置、文件、运行、结果）、研究页，加速器入门固定在底部、与收起按钮同组；`PAGES` 由它展开，`Ctrl+1…9` 与视图菜单都按这个顺序，调整顺序只改 `PAGE_GROUPS`。设置页在导航中叫“模拟设置”（与菜单栏的“设置”区分），页面 id 仍为 `settings`。左侧导航可收成图标（`Ctrl+B`、侧栏底部的收起按钮或双击分隔条，`ui/sidebarCollapsed`；图标固定在 46 px 列内，收起只是宽度滑动，动效遵守 视图 → 动效，`<html data-motion>`）或完全隐藏（`Ctrl+Shift+B`／视图 → 隐藏侧边栏，`ui/sidebarHidden`）；隐藏时工具栏的侧栏按钮与 `Ctrl+B` 先让它重新显示。页面切换时**页面保持挂载**（`display: none`），写 effect 时要考虑页面隐藏的情况。
- 界面缩放、主题、语言、默认计算内核（`engine/defaultKernel`，菜单“设置”）等偏好保存在 `%LOCALAPPDATA%\AVAS\gui.json`（`persist()`）；只影响单个视图的小状态可以放 `localStorage`，读写要 try/catch。

### 5.2 前后端通信

- 前端只通过 `call(method, params)` 调用后端（`POST /api/rpc`）；后端函数放在 `avas/gui/services/<页面>.py`，用 `@rpc("页面.动作")` 注册。
  桌面窗口和浏览器走同一条 HTTP 通路，pywebview **不再**提供 js_api，也不用 `evaluate_js` 推送事件。
- 每个请求必须带启动时生成的**访问令牌**（头 `X-AVAS-Token`，WebSocket 和下载链接用 `?token=`），页面从自己的地址里读取；
  不要加免令牌的接口，也不要放开 CORS（RPC 能读写本机文件）。服务器默认只监听 127.0.0.1。
- 需要显示给用户的错误抛 `bridge.UserError("…")`；其他异常会作为程序错误记录日志。
- 大的数值数组用 `bridge.blob(array)` 以二进制传输，不要塞进 JSON。
- 后端主动通知用 `bridge.emit(name, payload)`（批量经 WebSocket 发送），前端 `on(name, fn)` 接收。payload 要是发送时刻的快照（深拷贝），不要传之后还会被修改的对象。
  没有页面连接时事件直接丢弃，不排队；页面重连后收到本地事件 `bridge.reconnected`，需要的状态自己重新取（`store/app.ts` 已取运行状态和项目摘要）。
- `project` 事件在项目摘要任何变化时都会发出，不只是切换项目；重置"每个项目"的界面状态前先比较项目路径。
- 后端 `UserError` 的文字是英文，前端在 `zh_CN.json` 有对应条目时自动翻译（`bridge.ts`）；给用户看的固定提示请同时加翻译。
- 运行记录 `avas_run.json` 里的 `lattice_sha1` 是运行所用 lattice 文本的指纹（`avas/gui/textio.py` 的 `text_fingerprint`，前端 `textFingerprint` 必须算出相同结果），用于提示"上次运行后 lattice 已修改"。
- **绝不能在 pywebview 的界面线程事件（如窗口关闭）里调用 `evaluate_js`**，会死锁；需要的状态保存在 Python 端（正常代码路径已不用 `evaluate_js`，只有自动化检查脚本用）。

### 5.3 Lattice 编辑

- 元件图框内右上角提供图标加文字的“查看场图／查看场分布”按钮，分别对应场图文件和解析模型；按钮占独立一行，不覆盖图形，不放在参数区远端。

- 可视化二维拖放命中具体非漂移元件图形时，以该元件入口对齐叠加（优先于纵向区间判断），支持连续建立多元件同起点叠加。二维／三维元件右键复用结构菜单，提供叠加元件、复制、删除等；浏览／运行锁下写操作禁用。

- 元件列表、详情、场分布窗口与 2D／3D 共用类型识别，仅文件名推断的元件类型附加“（推测）”，不附加“场图”；识别依据放入详情说明。拖放到普通非漂移元件内部自动创建 superpose 组，端点仍按顺序插入，漂移段沿用拆分；组内位置相对首元件，首元件固定零，组长度变化会移动下游。组操作按钮必须明确说明作用于整组。

- 恢复历史先备份当前编辑文本，作为独立撤销步骤放入编辑器，成功保存后才形成恢复版本；保留后续已有版本。保存记录携带同文件的来源版本 ID，后台按实际写入内容区分“从版本 N 恢复”和“从版本 N 恢复后修改”。恢复来源随 Monaco 撤销／重做分支变化，撤销恢复后的普通编辑不带恢复标记，成功保存后清除本次来源；相邻相同内容不重复记录，保存失败不记录为成功恢复。

- 文件历史跟随具体文件，不设全局主导航入口。文件页在左侧文件列表下方设置可折叠、可拖动调高的历史区，两区独立滚动，标明当前文件；选择版本在右侧主区域对比，文件树保留可用。结构页在文件选择框旁提供「历史」按钮，弹窗左侧版本列表、右侧差异，下方恢复操作。两处共用版本与恢复逻辑；原编辑器保持挂载，恢复仍可撤销、另行保存且遵守运行锁。切换文件／项目清除旧对比；恢复前校验目标及当前编辑内容。元件栏全部类型按单行横向滚动展示，名称不因窗口变窄隐藏，左右箭头、触控板横滑及 Shift＋滚轮可导航，保留点击／拖拽插入。

- 文件历史由 `gui/filehistory.py` 统一记录，存于项目 `.avas_history/`，支持 lattice、beam.txt、input.txt（单文件最多 2 MB），保存前保留旧文本、成功写入后记录新文本，相邻相同内容不重复。文本／结构／可视化编辑、文件页、束流页、设置页、已批准 AI 写入共用；历史按项目内相对文件路径归属，重命名后视为另一文件，不自动清理版本。结构页和文件页提供历史对比，束流页与模拟设置页的「历史」按钮（beam.txt／input.txt）复用同一对话框：对比的是已保存的文件，恢复时经 `beam.parse`／`settings.parse` 把版本文本换成表单值，作为一步可撤销修改，保存时带 `restoredFrom`（只恢复页面管理的关键字，其余行保持当前文件）；`history.prepareRestore` 先检查运行锁并留存当前编辑内容，只返回历史文本，由编辑器以独立撤销步骤应用，保存仍走原接口，不新增绕过 AI 批准或运行锁的输入写入通道。历史从启用后的保存开始；外部修改在下次 AVAS 保存时留存，不监视外部编辑过程。

- 可视化编辑的撤销／重做直接使用当前 Monaco 文本模型的历史，不依赖键盘焦点；无对应历史时按钮禁用。「撤销本次所有修改」恢复到进入本次编辑时的文本，保持编辑状态，恢复本身作为独立一步可撤销，写入文件仍需保存。文本点击按鼠标命中的实际模型行选中元件，重复点击仍定位；键盘移动同步选中，程序定位、编辑和撤销／重做造成的光标移动不得覆盖元件选择。

- 元件定位使用独立的整行高亮与左侧标记，失焦／只读仍可见；定位展开目标折叠并立即居中，重复点击也生效。文本＋结构右侧复用 ComponentView 展示元件及场曲线，修改仍经同一个 Monaco 模型一步撤销。

- 场的图形一律区分**数据**与**示意**。场图文件的切片是数据：读取、缓存与切面集中在 `avas/gui/fieldcache.py`（`files.fieldSlice` 按路径、`lattice.fieldSlice` 按场图名字，两个 service 都调它，彼此不互相 import），整个 cube 留在后端，只把要看的那一个平面按 `bridge.blob` 传出，同一 family（`bs`／`es`／`bd`／`ed`）中网格相同的分量一起切，供模与面内箭头使用；色标用 `Plot.tsx` 的 `fieldScale()`，有符号分量必须配 `zmid: 0`。矩阵模型元件孔径内的场由 `lattice/analyticField.ts` 按元件参数算出，是示意图，必须用 `.cv-schematic` 或视图里的说明标注，不得与场图数据混为一谈；螺线管横向无场，横截面不画网格。
- 两种来源经 `files/slice.ts` 归一成同一个 `Slice`（后端 payload 走 `fromPayload`，解析场走 `analyticSlice`），`files/FieldSlice.tsx` 一套视图渲染两者，文件页和元件场分布窗口共用。解析场按**硬边界**生成：元件外恒为零，纵切面向两端各留 14 % 余量把这条边画出来；零长度元件（校正铁）只有 `xy` 一个平面，请求别的平面时由 `analyticSlice` 改回来，所以渲染一律用 `data.plane` 而不是组件 state。
- 场分布窗口开着时由 `ComponentView` 推送更新（`useFieldWindow`）：换元件换内容，改参数实时重画。窗口内容的 React key 只能用元件标题，不能用 source 全量，否则拖一下滑块就把用户选的切面和分量重置了。箭头位移在数据坐标下正比于场分量（`files/quiver.ts`），因此屏幕方向随坐标轴拉伸，只有等比例时才是真实方向，界面要说明这一点。**所有横截面一律从下游往上游看，束流垂直屏幕向外**（2026-09-21 统一；校正铁原先按射入屏幕画受力，已改号）：这样同一张图里的场箭头和受力箭头可以直接用 F = qv × B 相互核对。圆心的 ⊙ 记号（`BeamOut`、`.cv-beam-ring`）标出这个方向，新的横截面都要带上它，不要再引入别的观察方向。校正磁铁倒三角加竖线符号提供图例，场图类型的文件名识别依据在详情说明中标明。

- 可视化编辑器 2D / 3D 选中元件时，列表展开其全部祖先分组并滚动到高亮行；重复点击同一元件也触发定位。搜索或筛选挡住目标时只清除阻挡目标的条件；手动折叠、输入筛选本身不触发重新展开。

- 结构 / 可视化编辑器共用的元件列表显示具体类型，并支持中英文类型搜索；场图射频 / 静电 / 静磁分类来自场类型，静磁元件复用图形的场文件名推测规则，在详情说明中说明文件名识别依据，在推断的元件类型后加“（推测）”，不加“场图”后缀。无法识别时保留场类别或关键字名称，不把名称推测作为物理数据。

- 二维布局图与结构页小束线图按真实 z 区间将重叠元件上下分层，横向位置和长度不变；分层仅用于示意，不表示横向偏移。各层独立点击选中，悬停列出当前位置全部叠加元件，superpose 组用括线标识；层数不限，多层或小窗口允许滚动，不压缩成无法辨认的图形。显示层号不能改变孔径等物理数据的选择规则。

- 2D / 3D 视图区左上角统一使用 ViewNavigation 的放大、缩小、显示全部按钮，外层不再重复提供入口；2D 为工具栏预留顶部空间。按钮围绕当前观察中心缩放，不依赖指针检测。3D 双击空白显示全部、双击元件聚焦；2D 双击视图显示全部，图例双击仍只显示该曲线。3D 手动缩放停止漫游和束团跟随。

- 可视化编辑器的 2D / 3D 上方视图区随可用高度收缩（最低 160px），下方元件列表和参数面板至少保留 180px；空间不足时编辑器滚动，不能裁掉下方面板。窗口缩小时不覆盖用户保存的视图区高度，恢复窗口后恢复该高度；拖动分隔条从实际显示高度开始计算。
- **输入文件的归属**（2026-11-03 与用户商定）：初始粒子分布的显示（`ParticlesView`，叠加按粒子协方差算的 rms 椭圆）和二次粒子（input.txt 的 `secondarybeam` 开关 + SeParticle.txt 表格）在束流页；boundary.txt 的表格在模拟设置页“启用边界”下方展开（scanData.txt 没有内核读取，M8b 起不再有表格，只在文件页查看）。`beam.save` 在写任何文件前检查 SeParticle.txt（开关打开或内容有改动时）：每行至少 11 列、前十列为数字，否则 `UserError` 指出行号，空行去掉（原内核遇到短行或空行会越界读）。二次粒子运行没有 DataSet 行（两个内核都一样，2026-11-15 用新内核核实），`dataset_diagnostics` 据输入快照（`inputs/input.txt`）说明结果在 outData_*.txt／SeSynParticle.txt，`results.overview` 的 `secondary` 让结果页注明没有包络曲线；分段运行遇到 `secondarybeam 1` 直接拒绝（`segment.secondary_beam`）。这些小文件随页面一起保存（`beam.save` 的 `secondary`、`settings.save` 的 `companions`），只在内容变化时写，空表格不新建文件，同样受运行锁约束。束流页只改 input.txt 的 `secondarybeam` 一行，模拟设置页不管理这个关键字，两页写 input.txt 互不覆盖。文件页是 InputFile 的文件浏览器（原始文本、场图、文件管理），不要再让某项功能只能从文件页进入。元件场分布窗口底部的“场图文件”表（`lattice.fieldInfo`）列出每个分量文件的实际路径、网格、范围、归一化与存储格式。
- 一个 lattice 文件在界面里只有**一个 Monaco 文本模型**，它是唯一的数据源。结构编辑器、可视化编辑器、AI 助手的修改都以"一次修改 = 一步撤销"写回这个文本模型，再重新解析（`lattice.parse`）。不要另存一份可以单独修改的元件数据。
- `LatticeEditor` 用带 key 的子元素切换"文本 + 结构"和"可视化编辑器"两种模式，保证文本面板不被重新挂载（未保存的文字和撤销历史不丢）。
- 修改只改写对应的行，保留注释和页面不认识的关键字；beam.txt、input.txt 保存时同样只改页面管理的关键字。
- 参数含义、单位、取值、检查规则以 `docs/使用说明20260427.docx` 为准，集中写在 `avas/data/schema.py` 和 `avas/data/lattice_doc.py`。schema 是唯一的关键字表：改类型 / 取值时只改 schema，配置类、常量表、界面都会跟着变。
- 运行使用的 lattice 是 `ini.ini` 里 `[lattice] source` 指定的文件，用 `avas.paths.lattice_source_path()` 获取；**不要写死 `lattice_mulp.txt`**。路径一律用 `avas.paths` 里的函数，不要用 `__file__` 层层向上找。
- TraceWin `.dat` lattice 目前只读。
- **可视化编辑器的浏览 / 编辑状态**：打开、切换项目、重新载入、运行开始时都回到浏览状态（`store/latticeUi.ts` 的 `visualEditing`，不持久化）。
  浏览状态下可视化编辑器和旁边的文本栏都是只读，改动入口（元件面板、手柄、参数、结构树拖动、快捷键）一律不可用并给出提示；
  「完成」时有未保存修改必须询问「保存 / 放弃本次编辑 / 继续编辑」。「文本 + 结构」模式不受此限制。
  页面发来的整段替换（还原、已批准的 AI 修改）在浏览状态下也要生效（`TextEditor.setText` 临时解除只读）。

### 5.4 运行

- 结构页回放条同步控制其显示的项目／分段回放，显示来源以及暂停／继续、结束按钮；结束只停止动画，保留历史记录。运行页返回箭头退出历史记录并结束动画；最大化使用四角展开／收拢图标，结束回放使用实心停止图标，互不混用。

- 运行页束流实时／回放面板默认按元件分层高度加曲线空间自动增高（上限 720px，极多层仍可图内滚动），页面整体可滚动。视图底边可拖动调高（240–1200px，`avas.live.height` 持久化），双击或“自动调整视图高度”恢复自动。最大化只占运行页工作区，保留回放控制与统计，还原不丢失普通高度和回放状态；最大化状态不持久化。

- **内核选择**（M7a，2026-11-11 用户决定，详见 docs/engine/M7-plan.md、M7-notes.md）：`avas/engines.py` 是唯一的选择层。每个键的优先级为命令行
  （`avas run --engine original|new --device auto|cpu|cuda|metal --precision auto|fp64|fp32 --gpus all|0,1 --threads N --compat none|dll`）> 环境变量
  （`AVAS_ENGINE`、`AVAS_ENGINE_DEVICE`、`AVAS_ENGINE_PRECISION`、`AVAS_ENGINE_GPUS`、`AVAS_ENGINE_THREADS`、`AVAS_ENGINE_COMPAT`）> 项目 ini.ini 的 `[engine]`
  （键 `kernel device precision gpus threads compat`，`avas/utils/iniconfig.py` 的 `read_engine_settings` / `write_engine_settings`，保留其余段和键）>（只对 kernel）
  全局默认内核（M8 决定 5，2026-11-15：菜单“设置 → 默认计算内核”，gui.json 的 `engine/defaultKernel` = original / new，未设为内置默认；`engines.read_preference` /
  `default_choice_kernel`，来源标签 `preference`；文件取环境变量 `AVAS_GUI_SETTINGS`，否则用户数据目录的 gui.json——与 `avas.gui.settings.default_path` 同一规则；
  GUI 进程在创建设置对象时把 `AVAS_GUI_SETTINGS` 设为它的路径，`avas run` 子进程、研究层与 AI 沙盒因此读同一个文件，单独用命令行时读用户的 gui.json；
  在没有原内核的平台上偏好为 original 时回落到内置默认并在 `sources["note"]` 说明（CLI 打印 note），不报错；测试由 tests/conftest.py 指向临时文件，不读开发者的 gui.json）> 内置默认（kernel 按平台：
  有原内核的平台 original，没有的平台 new——`engines.original_available()` 只认 Windows / Linux 的 x86-64，`default_kernel()`；新内核 auto / auto / all / 0 / none）。
  这与“默认仍用原内核”不冲突：那是对有原内核的平台（M7b，2026-11-11）。没有原内核的平台上**显式**选原内核（ini、`--engine original`、`AVAS_ENGINE`）在运行前报错
  （`engines.check_platform`，CLI 退出码 2；GUI 的 `kernels.require_ready`），`avas info`、设置页、运行记录都体现平台默认。原内核被选中时忽略其余键；旧的 `[input] device = gpu`（libPIC.so）只对原内核有效，M8 起运行时记弃用警告（`MultiParticle.LEGACY_GPU_DEPRECATION`，改用 `--engine new --device cuda`），第三阶段删除。不写 input.txt（原内核要能读同一项目）。
  新内核库的查找：`AVAS_ENGINE_LIB` > `avas/engine/new/` > `engine/build`（`Release`、`RelWithDebInfo`），路径来自 `avas.paths`。
  所有内核运行都经 `avas run` 子进程，各入口拿到 `[engine]` 的方式：完整运行 / 误差研究直接用项目 InputFile；分段运行、AI 沙盒、参数扫描、入门教程复制的文本输入含 ini.ini；
  研究层 `Study` 把解析好的选择写进每次运行的 `input/ini.ini [engine]`，并在子进程环境里去掉上述环境变量。
  **选了 cuda / metal 而本机或这个构建没有时，运行前报错并提示改 auto 或 cpu，不得悄悄改用 CPU**（`MultiParticleEngine.check_device`，判断用新内核自己的
  `avas_engine_devices()`，与 `main_agent` 同一段代码 `plan_device`）；Mac 上 auto 只在有空间电荷且粒子数 ≥ 网格单元数 × `kMetalParticlesPerCell`（0.5，`device.h`）时选 Metal，否则 CPU（原因代码 `few_particles`；查询不知道运行规模，Metal 计划里带 `cpu_below_particles_per_cell`，界面运行前的预计由 `engines.size_adjusted_plan` 按项目的 particlenumber 与 numofgrid 换算，见 M8-plan 第 12 节）；compat dll 只能配 auto / cpu，Metal 只有 fp32，CPU 只有 fp64，组合不对时在解析阶段报错。
  内核库只在 `avas run` / `avas info` 进程里用 ctypes 加载；其他进程（研究层、GUI）要知道 auto 会选什么时调用子进程 `avas info --engine-json`（`engines.query_subprocess`）。
  **avas_run.json 的 `engine` 字段**：原内核 `{"kernel": "original", "file", "sha1", "tier"}`（libPIC.so 通路另记 `device: gpu`）；新内核另有 `version`（引擎源码哈希前 16 位）、
  实际的 `device`、`precision`、`gpus`、`compat`、`threads`、`space_charge`（取自 `avas_engine_last_run()`）、`tier` 与 `requested`，auto 时还有 `reason` 与 `reason_code`
  （内核给出的原因代码，界面 `engine.ts` 的 `autoReason` 按代码翻译，不匹配英文原因；没有代码的旧记录才按文字归类）；误差研究等多次运行记最后一次；
  旧记录没有该字段，读取时要容忍。**结果档**（`engines.result_tier`）：original、new-cpu-fp64、new-cuda-fp64（CUDA 与 emu 的 fp64）、new-fp32（CUDA / Metal / emu 的 fp32 逐字节相同）、
  new-compat-dll；研究指纹的内核身份是原内核文件 SHA-1（与 M7 前相同，旧缓存仍有效）或新内核的（结果档，版本），线程数和 GPU 编号不进指纹；
  运行后把实际档写进 run.json，与预判不符时记警告。研究层 `workers > 1` 且用 CUDA 时每个 worker 绑定一张卡，worker 数不超过卡数，单卡或 Metal 降为 1 并写日志。
  device auto 的新内核运行在 `engine` 里另记 `reason`（内核计划给出的原因；计划的设备在运行时失败时注明），界面据此显示“实际：CPU（原因）”。
- **新内核的预编译库**（M7c，2026-11-11，详见 docs/engine/M7c-notes.md）：`avas/engine/new/` 的三个库只能来自 `.github/workflows/engine.yml` 的同一次成功运行，用 `python engine/tools/fetch_ci_engine.py [--run ID]` 校验后放入，与 `build-stamp.json` 一起提交；不要提交本地构建的库，不要手改构建戳。`tests/test_design_rules.py` 检查构建戳的源码哈希等于 `engine_source_hash()`、文件 SHA-256 匹配，所以改了 `engine/`（CMakeLists.txt、include/、src/、third_party/ 的源文件）就要重新跑工作流并 fetch。M7 合并 main 前三个平台的库必须齐全。CUDA 机器码只在 `engine/CMakeLists.txt` 定义（默认 sm_61/75/80/86/89/90 + compute_90 PTX，构建脚本不另写；P100 / V100 因此不能用预编译库的 CUDA）；Windows 作业固定 `windows-2022`（发布库的 MSVC 由镜像决定，CUDA 12.4 不接受 MSVC 19.5x）。`ci_engine.py` 借用 `avas.engines.engine_source_hash`，因此 `avas/engines.py`、`avas/paths.py` 只能依赖标准库。`packaging/avas.spec` 只把本平台的新库和构建戳打进安装包；Windows 上加载新库时 `avas/engine/`（自带 MSVCP140.dll）也加入 DLL 搜索路径。
- **界面中的计算内核**（M7b，2026-11-11）：模拟设置页“计算内核”分组（`pages/EngineSettings.tsx`）编辑 `[engine]`，随 `settings.save` 的 `engine` 参数保存，
  `kernels.save_settings` 只在与文件不同时写（表单 kernel 为 "" 表示项目不写 kernel、跟随全局默认内核，页面注明“跟随全局默认（当前为 X）”，“跟随默认”按钮删去项目的 kernel；
  选了内核才显式写；其余键等于默认值时删掉，改其余键不会把 kernel 写死），保存前 `validate_form` 校验组合，受运行锁约束；设备只列本机检测到的
  （`engine.info`，子进程查询并缓存，刷新按钮清缓存；壳层启动 5 秒后预取一次），auto 时用 `engine.preview` 显示本项目（按表单的空间电荷开关）会用什么；
  不合法的精度 / 兼容组合禁用并说明（`engine.ts` 的 `precisionProblem` / `compatProblem`，与后端 `engines.validate` 同一规则）；原内核下其余项禁用；新内核库缺失时不可选并说明。
  **每个运行入口在启动前调用 `kernels.require_ready(input_dir)`**：`run.check` / `run.start`、分段运行、研究页内核扫描与内核响应、AI 沙盒的 `Sandbox.run_in`；
  它检查原内核的平台与 `scmethod PICNIC`（只有新内核支持）、新内核库、设备与组合，用带参数的 `UserError`（模板 + params，前端 `t(template, params)` 翻译）给出可操作的提示。
  新增运行入口时也要调用它。运行状态的 `engine`（runner `Job.engine`、沙盒 activity）先是预计值（`predicted: true`），阶段结束后换成 avas_run.json 的实际值；
  标签文字只由 `engine.ts` 的 `engineLabel` / `engineShort` 生成（`EngineBadge` 组件），用于运行页、实时面板、运行记录、状态栏、结果页来源与运行对比；
  `results.sources`、`run.replay` 带 `engine`（`kernels.summary`，旧记录为 null，不显示标签）。input.txt `scanphase` 与 scanData.txt 没有内核读取（原内核、新内核两种模式、libPIC.so）：
  模拟设置页不提供相位扫描与 scanData 表格（M8b），文件里已有的 `scanphase` 行显示一行提示（`meta.hasScanphase`），保存时删去（`write_to_file_input_ini` 的 None 删行，保留其他行与注释）；
  scanData.txt 不删除，在文件页查看；schema 保留该关键字只为旧文件能解析。依赖实际内核的提示用 `engine.effective`（项目的解析结果，不起子进程）或表单的内核：
  随机种子（原内核或 compat = dll 且种子非 0 时 x 与相位完全相关，`engine.ts` 的 `seedDefect`；模拟设置页、研究页扫描与响应标签）、束流页二次粒子（原内核与 compat = dll
  只读 beam.txt `readparticledistribution` 写明的文件，新内核默认模式没写时读 SeParticle.txt，`secondaryNeedsBeamFile`；不改写 beam.txt）。运行页“与前一次运行对比”
  （实时数据 `previous.engine`）与结果页“运行对比”在两次运行的内核或模式（original / new / new compat=dll）不同时提示“差别可能来自内核”（`kernelsDiffer` / `kernelsDifferAmong`）。关于窗口与“复制版本信息”列出两个内核与本机设备（`engine.ts` 的 `kernelLines`）。
- 同一时间只运行一个任务：`avas/gui/services/runner.py` 的 `Job`（可含多个 `Stage`）。暂停是挂起整个子进程树（`avas/gui/proctree.py`），不是停止。
- 开始运行前前端会先检查并保存所有有修改的页面（`actions.ts` 的 `prepareRun`）。
- 分段运行（`avas/data/segment.py`、`services/segments.py`）结果写在 `<项目>/Segments/<名称>_<时间>/`，不改项目的 OutputFile。
- **运行记录**（`services/runs.py`）：完整运行总是写 OutputFile 并覆盖上一次；`runs.archive` 把 OutputFile（含 `inputs/` 快照）复制到
  `<项目>/Runs/<时间>_<名称>/`，并在 OutputFile/avas_run.json 记 `archived`。前端开始运行前先调 `runs.unsaved`，上次结果完成且未保留时
  询问「保留并运行 / 直接运行 / 取消」（`actions.ts` 的 `resolveUnsavedRun`）。`results.sources` 列出 OutputFile、已保留运行（kind `archived`）和分段运行；
  `runs.delete` 三种都能移到回收站。
- **参数扫描**（`avas/sim/scan.py`、`services/scan.py`、`avas scan`）：在 `<项目>/Scans/<名称>_<时间>/` 下用 `Sandbox(root=...)` 跑副本，
  每个取值一个 `run_NNN/`，结果 `scan.json` / `scan.csv`。扫描是一个 activity（source `scan`），运行页 / 状态栏 / 实时显示按 assistant 同样处理，
  期间不能开始普通运行；`scan.start` 在返回前就占住 runner（`prepare_scan`），不要改成线程里再占。改 lattice 参数用 `avas/data/lattice_edit.py`。
- **研究页**（`pages/StudyPage.tsx`，页面 id 仍为 `scan`，在导航中单独一组）：扫描、稳定区图谱、响应与可控性、反馈四个标签。内核扫描和内核响应矩阵都是研究层（`avas.research`）的运行，存于 `<项目>/Research/gui/`，notebook 的 `Study(项目, name="gui")` 读到同一批记录和缓存；GUI 通过 `Study(runner=…)` 把每次内核运行交给 AI 沙盒的 `Sandbox.run_in`，所以它们是 source `scan` 的 activity：运行页显示、可暂停／停止、与普通运行互斥，`study.start`／`analysis.responseEngineStart` 在返回前就占住 runner。线性扫描、周期面板、稳定区图谱、响应矩阵和反馈只用线性模型，不加运行锁、不写输入文件。扫描描述在 `Research/gui/scans/*.json`（删除只删描述，运行留在缓存）；旧的 `Scans/` 只读列出（`scan.*` 与 `avas scan` 不变）。监测器与旋钮保存在 `<项目>/Research/diagnostics.json`（study.json 格式），默认取自 lattice 的 `diag_*` 和校正铁；内核没有确定性的错位旋钮，内核响应只接受 lattice 参数。周期面板是结构页“文本 + 结构”的第三个标签（`lattice/PeriodPanel.tsx`），用编辑器当前文本；“把匹配的 α、β 填入束流页”只经窗口事件 `avas:beam-twiss` 改束流页的未保存表单，不新增写文件入口。结果页来源新增 kind `research`（不可在 GUI 删除）；包络／质心／能量／发射度图的“线性模型”开关按该次运行自己的输入（`output/inputs/` 快照或研究运行的 `input/`）计算虚线，不用项目当前输入。
- AI 助手的参数扫描 / 优化在 `<项目>/.avas_ai/runs/` 的副本里运行（`avas/ai/sandbox.py`），开始时复制输入文件，**不能改动项目的 InputFile 和 OutputFile**。
- **运行锁**：完整运行、误差研究、分段运行进行中（包括暂停）时，所有输入文件只读。原因：分段运行每个阶段开始时才复制 InputFile，
  而且 `<输出>/inputs/` 快照与实时显示都对应运行开始时的输入（完整运行和误差研究自 2026-09-20 起都在该快照上运行，不再读 InputFile）。
  实现分三层，新增写输入文件的入口时三层都要照顾到：
  后端写入接口先调用 `avas/gui/locks.py` 的 `require_unlocked()`；前端用 `useInputsLocked()` 让页面只读并显示 `RunLockBanner`；
  AI 修改提案在提出和应用时都检查（`assistant.py` 的 `_refuse_if_locked`），直接拒绝而不是挂起等待。AI 自己的沙盒试算不加锁。
  结构页的文件下拉框只是"打开"（查看 / 编辑），任何时候都可用，运行中只读；把文件设为运行使用（`lattice.setSource`，写 ini.ini）
  是单独的「设为运行结构」按钮，只有它受运行锁约束。不要把"打开哪个文件"和"运行用哪个文件"重新耦合到一个控件上。
- **实时显示**：`avas/data/dataset_stream.py` 增量读取正在写的 DataSet.txt（比运行开始时间更早的文件是上一次运行的，忽略到被重写为止）；
  `avas/gui/services/live.py` 每秒轮询当前运行（一个 job 或一次 AI 研究，每次内核运行一个 episode），用 `run.live` 事件推送新增行，
  `run.liveSnapshot` 一次取全；前端 `store/live.ts` 保存，`lattice/bunchPlayer.ts` 统一给出束团位置（实时跟随 / 回放），
  2D（`LayoutView`）、3D（`Beamline3D`）、运行页（`LiveBeamPanel`）只负责画。束团和粒子云是按 rms 包络画的**示意**，界面上必须标注，
  不能画成像真实分布。可视化编辑器只显示本项目的完整运行、误差研究和分段运行；AI 试算只在运行页显示。
- **回放的分工**（2026-09-20 与用户商定）：运行页是回放的主场——实时面板在运行结束后回放本次运行，「运行记录」列表里每条完成的记录
  有「回放」按钮，`run.replay(outputDir)` 从记录的 DataSet.txt 读包络（完整运行按 `inputs/` 快照里的 lattice 画，分段运行用项目当前 lattice
  加 z 偏移），前端 `store/live.ts` 的 `record` 让 `LiveBeamPanel` 切换到该记录，新运行开始（`begin` 事件）时自动清掉。
  结构页只回放「上次运行」（曲线绑定正在编辑的 lattice），不做历史选择；结果页不放播放器。运行页的「与前一次运行对比」灰线和结构页一样
  默认关闭（`localStorage` `avas.live.compare`）。结构页「上次运行」曲线默认不显示（显示设置存 `avas.visual.show.v2`，旧键会迁移），
  回放条不依赖它，点回放时自动打开。图例每项左侧的 × 隐藏该曲线（×放右侧会和 x/y 字母混淆），隐藏集合存 `localStorage` `avas.layout.hidden`，
  所有布局图共用。
  动画遵守 **视图 → 动效** 设置（用户要求默认「完整」；「自动」才跟随 Windows 动画效果），最多 30 帧 / 秒，不可见时停止。

### 5.5 加速器入门

- 入门页（`pages/LearnPage.tsx`）不需要打开项目。章节是 `frontend/src/learn/content/<id>.zh.md` 与 `<id>.en.md`，中英文同结构；`##` 小节必须带 `{#<章节id>-…}` 稳定 ID，两种语言 ID 相同，链接写 `[文字](#id)`。章节顺序、所属部分和审核状态只写在 `learn/chapters.ts`；内容由 AI 撰写时状态为 `ai-draft`，页面显示“AI 撰写，待审核”，物理人员审核后改为 `reviewed`。章节文字只由入门页懒加载（`learn/content.ts`），壳层和元件视图只引用 `chapters.ts`。
- 写法按费曼学习法（2026-11-01 与用户商定）：每章先提出问题、给出物理图像，逐步推出方程并代入示例束流的数值，在需要验证处放实验或束线，设“想一想”与“容易误解的地方”；AVAS 关键字、参数表与约定集中在最后的参考小节 `## 在 AVAS 中 {#<章节id>-in-avas}`（页面以虚线和“参考”标签与理解部分分开），不能为了易读删去技术事实。第一部分是故事线“跟着一个质子走”（`journey-*` 六章：全貌、离子源与 LEBT、RFQ、MEBT、超导段、HEBT），解释加速器为什么分段、每段由什么组成；讲原理时可以介绍 AVAS 不模拟的段，但必须标明“AVAS 不模拟”并说明束流如何交给 AVAS。元件章节仍只介绍 AVAS 的元件。初始分布单独成章（`distributions`），配 `distribution` 实验：按教科书四维定义抽样 KV／WB／PB／GS，标注内核实际生成方式以 `inData.dst` 为准。
- 公式写 LaTeX（`$…$`、单独成行的 `$$…$$`），由 temml 转成 MathML（`learn/MathView.tsx`，`trust: false`），入门章节和 AI 回答共用；`learn/content.test.ts` 编译全部章节公式并检查 ID、链接、内嵌块和中英文一致，公式错误要让 `npm test` 失败而不是显示成代码。
- 章节中的交互只用五种内嵌块（`learn/directives.ts`）：`avas-lab`（实验）、`avas-params`（参数表直接取自 `schema.all`，不要在章节里重抄参数表）、`avas-field`（解析示意场或示例场图切片，沿用数据／示意的区分）、`avas-tutorial`、`avas-beamline`（块内直接写 lattice 行，可省略 start／end，后端 `learn.beamline` 补齐并用示例束流算线性预览）。实验的参数、范围和全部数值都在后端 `avas/learn/labs.py`，物理计算复用 `avas/sim/linear_optics.py` 的矩阵和预览，不要在前端另写物理公式；实验结果属于线性模型，界面要注明以内核为准。`tests/test_learn.py` 检查章节里的实验预设和关键字确实存在，每个元件章节都有中英文相同、解析无问题、预览可算的 `avas-beamline`。
- 束线示意一律复用结构页的 `LayoutView` 与 `Beamline3D`（`learn/BeamlineView.tsx`，2D／3D 切换、默认 2D，3D 只在接近可视区时挂载以节省 WebGL 上下文），lattice 经 `lattice.parse` 解析（不需要项目），点击元件显示只读 `ComponentView`；实验结果的 `beamline` 字段和教程预览提供包络。不要在入门里另画束线图。入门页左侧章节目录与右侧“本页内容”的开关都在正文上方的固定顶栏（“章节”按钮、当前位置、“本页内容”按钮），收起时面板宽度滑动为零，不留窄条；页面宽度不足时（章节目录 < 720 px、本页内容 < 1060 px，`LearnPage.tsx` 的 `NAV_DOCK_WIDTH`／`TOC_DOCK_WIDTH`）改为浮在正文上的抽屉，选择条目、点正文或 Esc 关闭，抽屉状态不保存；停靠状态保存在 `localStorage` `avas.learn.panels`。
- 引导教程第 4、5 步（多周期）的默认值是 Kb 0.30、Ke 0.8（`PERIOD_DEFAULTS`，第 3 步单腔仍是 Kb 0.35、Ke 1.36）：沿用 0.35／1.36 时第一个周期纵向相移约 128°，内核在 8 个周期上丢失粒子（2026-11-02 Windows 核实，rms z 涨到十几毫米）；0.30／0.8 时每个周期横向、纵向都低于 90°（第一个周期约 75°／86°）。页面换步时只带走读者改过的值，其余用该步自己的默认值。
- 引导教程（`avas/learn/tutorial.py`）只用示例项目 `examples/hwr010` 的 `hwr010`、`sol` 场图和漂移段，首尾不放矩阵元件，lattice 文字保持 ASCII。示例项目由 `avas.paths.example_input_dir()` 定位，打包时 `packaging/avas.spec` 随附它的 beam/input/ini 与这两组场图。内核运行用 `Sandbox`（source `learn`）在 `<用户数据>/learn/runs/` 中进行，只保留最近 3 次，不需要也不触碰任何项目，与其他模拟互斥；结果事件 `learn.tutorial` 不带曲线，页面再用 `learn.tutorial.state` 取 blob。“另存为新项目”只写入新的或空的文件夹。
- 讲解员是 AI 助手本身：`primer_contents / primer_search / primer_read / primer_show` 通过页面（`assistant/front.ts` 的 `learn.*`）读取章节，`primer_lab` 直接调用后端实验；这些工具不需要项目，助手在未打开项目时也可以对话。页面的 `ui.state` 带当前章节，提示词里有讲解员的规则。
- 助手面板把一次回答之前的思考、中间说明和普通工具调用合并成一个默认收起的“思考过程”块（`assistant/steps.ts` 的 `layoutItems`），运行中标题显示当前步骤；修改提案、通知、错误以及带进度、图表或表格结果的工具调用仍单独显示。

### 5.6 AI 助手

- 必须支持 OpenAI 兼容接口和本地模型（数据可能不能离开本机）；客户端只用标准库。API 密钥存在 Windows 凭据管理器，不写进设置文件。
- 对项目文件的每一处修改都是一张提案卡片，用户批准后才写入（或用户在该对话打开了自动应用）；写入前备份到 `<项目>/.avas_ai/backups/`，可撤销。不要增加绕过批准的写文件途径。
- 本机没有真实的大模型服务，测试用 `tests/test_assistant.py` 里的脚本化模拟服务器。

### 5.7 软件更新

- 变更记录被误改后，只允许精确恢复到该路径首次提交的 Git blob，追加说明移入新条目；修复本身仍须新增非空记录。检查错误指出具体路径，不能通过放开普通改写／删除来解除 CI 阻塞。

- Windows setup 在欢迎页继续时检查官方 `setup-latest.ini`；失败必须显示具体原因和重试／安装内置版本／退出选择，不得伪称已是最新或悄悄回退。新版安装包经固定完整提交地址及 SHA-256 校验后启动，继承语言、安装范围和命令行选项；子安装器跳过再次检查，原安装器等待子进程完成且不再安装旧包。静默模式及 `/NOCHECKUPDATE` 安装内置版本。schema 1 的 INI 协议保持兼容，由不可变 Release 元数据生成，在两处固定发布就绪后上传；以完整 Git 历史的提交计数比较先后，浅克隆或无标识构建明确提示无法比较。

- 更新发布时间复用关于窗口的本地时间格式（注明 UTC 偏移，无时区不猜测）。主面板交接前同步进入安装状态；Windows 独立窗口接收经校验的可见布局绘图快照（纯文字、坐标、字距，不执行 HTML），保留版本和可见说明的排布。快照仅用于该次交接，旧调用继续用默认布局。独立窗口缓存静态绘制与进度条画刷，等待动画只更新进度条区域，按 60 帧目标扣除绘制耗时调度；高精度定时只在可见且需要动画时启用，退出、最小化和关闭动效时释放。

- 更新元数据请求遇到暂时网络故障最多尝试三次，间隔 1／2 秒；证书、非临时 HTTP 错误及无效元数据不重试。磁盘日志记录请求阶段、尝试次数及耗时；正常首次成功的诊断不进入界面日志和状态栏，重试、失败及重试后恢复仍显示。检测使用独立串行锁，联网不得持有更新状态锁。

- 更新网络请求每次独立创建连接器并重新读取系统／环境代理，不复用首次请求的全局 urllib opener。独立窗口交接已解析主题与动效，旧调用缺失时从当前设置兜底；Windows 图形超采样抗锯齿、文字按原生 DPI 绘制，最终双缓冲显示；阶段标签保留捕获的纵向位置但使用完整阶段列宽，避免高 DPI 原生字体取整造成换行缺字。等待条复用主程序的 30 % 宽、1.2 秒 ease-in-out 从左向右循环，不能往返运动或伪造百分比；关闭动效时静止、最小化时停止重绘。各安装类型都将 AVAS 图标复制至更新工作目录，更新窗口设置大小图标与独立任务栏标识，打包更新器也嵌入同一图标。

- 更新确认、下载与取消结果集中在基于 FloatingWindow 的非模态更新面板，使用浅／深色更新配色与束线式阶段图；版本、说明和进度均来自实际更新状态，百分比仅表示当前下载，未知大小不伪造总进度。关闭面板只收起显示，准备期间的「取消更新」才中止任务，正在等待版本确认时关闭视为稍后。取消后保留结果与再次更新入口，重新更新仍需重新检测与确认。独立安装窗口沿用更新面板配色、版本信息和下载／校验／准备／安装／重启阶段图；备份、依赖及启动检查显示为安装阶段的详细状态，百分比只表示当前可测量操作，不伪造整体进度。

- ZIP 更新下载遇到暂时网络错误、不完整响应或校验失败最多尝试四次，间隔 1／2／4 秒；目标附件缓存按含完整版本的 URL 与 SHA-256 隔离，用独占文件锁防止同时写入，取消或重启后可复用。完整缓存重新校验后使用，残缺缓存用 Range 续传，严格校验 Content-Range，不支持续传或返回 416 时重新下载，完整校验失败从零下载。未校验的旧源码基线不续传。下载、解压、准备完成前及实际替换前检查相关磁盘空间，备份和目标同盘时合并预算，保留 64 MiB 余量；空间预检不能替代写入错误处理，也不保证 pip／Git 的未知空间需求。
- 更新准备阶段支持协作取消，下载读取、校验、解压与文件预检检查取消信号，Git fetch 可终止自身子进程树；网络连接／读取等待可能延迟到本次返回或超时。取消请求不能提前释放 maintenance 预约，任务实际退出才释放；准备完成发布安装计划前再次检查。取消是正常结果，不弹失败提示，保留目标包缓存；进入独立安装后前后端均拒绝取消，不中断文件替换。

- 安装包、便携包、源码 ZIP 与 Git 版更新使用同一启动保护协议（`update_guard.py`）：独立更新器在主程序退出前持有同安装目录的更新锁，贯穿安装及自动重启；重复启动只通知更新窗口并立即退出，不在占用程序文件的第二个进程中弹出阻塞对话框。源码 CLI／GUI／serve 在加载运行依赖前获取启动许可，打包入口通过 `runtime_update_guard.py` 提前检查。自动重启用一次性票据通过更新锁，主窗口显示后确认，才关闭更新窗口和释放保护；异常退出由操作系统释放锁，遗留元数据不阻止正常启动。
- 主窗口退出前说明“更新完成自动重启，请勿手动打开”。安装期间 `update_status.py` 在独立更新器中持续显示等待、备份、安装、依赖、检查、恢复及重启状态；更新完成后自动关闭，安装时不能通过关闭按钮终止。该独立状态窗口是主界面技术栈的必要例外：Windows 只用标准库 ctypes + GDI 双缓冲绘制，其他源码桌面用标准库 Tk Canvas，共用 `update_view.py` 的布局。主窗口交接当前语言、经校验的主题颜色及更新面板屏幕位置／大小；Windows 按 WebView 客户区、界面缩放和显示器 DPI 换算并限制在显示器工作区内，旧调用方使用同风格默认值。窗口可拖动、最小化；首次绘制成功后才允许主程序退出。不加载将被替换的 WebView／React 或第三方 Python GUI 依赖，创建失败时不得通知主程序退出。源码更新需同时复制 worker、guard、status、view 四个独立模块。

- 下载进度通过 `updates.progress` 和 `updates.status` 同一份快照提供，包含阶段、已下载字节、可选总大小和平均速度；总大小未知时不伪造百分比，校验／解压阶段不沿用下载数值。准备、更新器启动、安装都有日志，主日志不可写时用系统临时目录兜底，错误详情独立保存到结果；日志与结果在重启前刷新落盘。主程序等独立更新器写入 `ready.json` 后才退出；Windows 使用已校验目标包里的独立更新器。替换前检查文件可写性，短暂占用限时重试，不强制关闭其他 AVAS；先复制到同目录临时文件再替换，清单最后写入，未变化文件不重写，失败仍遵守备份恢复规则。

- Windows 打包版 `AVAS.exe` 无参数时启动同目录 `AVASGui.exe`，有参数时保持命令行功能；源码入口行为不变。安装器将所选语言写入安装目录 `avas-install.ini`，打包版仅在用户设置没有 `ui/language` 时读取并持久化；升级、重装不得覆盖已保存的语言偏好。该安装配置不属于发布包文件清单，自动更新保留它。

- 安装包简体中文翻译随仓库保存在 `packaging/languages/`，保留上游固定提交、校验值和许可证；不依赖 Inno Setup 安装目录自带中文文件，也不在构建时下载翻译。调用编译器的构建在 PyInstaller 开始前检查该文件。
- 官方源固定为 `lycself/AVAS_XMU`。main 通过 CI 后生成源码包与 Windows 程序包并验证。正式发布使用 `avas-<7 位 SHA>`，缩短 GitHub 自动源码归档名称；发布前验证短标签实际指向完整 SHA，冲突时拒绝覆盖。完整 SHA 的兼容发布保留相同附件，新建时标为预发布，供旧更新器的严格下载地址校验和源码基线查询使用；短标签附件从已发布兼容包复制，重试不覆盖已发布文件。两处就绪后才发布 `avas-latest/update.json`，其下载说明链接短标签正式版本，元数据仍使用完整 SHA 地址。发布时检查祖先关系，较旧流水线不得覆盖较新的指针。
- 后端 `services/updates.py` 统一检测与准备，`avas/updates.py` 下载校验，标准库独立程序 `avas/update_worker.py` 在主进程退出后安装；Windows 打包成独立的 `AVASUpdate.exe`，不能依赖将被替换的 `_internal`。
- 自动检测缓存 6 小时，帮助菜单可强制检查；用户确认前重新检查，目标变化要重新确认。确认后所有获取与安装锁定 SHA，禁止裸 `git pull` 或下载可变分支压缩包。忽略版本按 SHA 保存，弹窗明确提醒仍可从帮助菜单获取更新。
- 每次提交前，AI / 开发者必须主动新增 `docs/changes/*.md` 简短变更条目，用用户能理解的语言说明变化；内部维护也须明确说明范围。尚未提交的本次条目可编辑，已提交条目不可修改或删除，纠正时另增条目。CI 检查提交范围内新增非空条目。发布脚本按 `avas-latest/update.json` 的上次成功提交汇总新条目，生成 `dist/updates/docs/update-notes.md`（源码包内为 `docs/update-notes.md`），不回写提交；同一份文字固定写入该版本的 `update.json` 和 GitHub Release。弹窗按纯文本展示，不执行 HTML，也不从 Git 提交标题生成说明；新版本重新确认时同时展示新版本说明。
- 正式发布同时包含 Windows Inno Setup 安装包 `AVAS-版本-setup.exe`、`avas-windows.zip`、`avas-source.zip` 和 `update.json`。首次安装推荐 setup.exe，自动更新继续使用 ZIP；CI 缺少安装编译器必须失败，不能静默跳过。Release 正文统一解释所有附件（含 GitHub 自动源码归档），自动更新入口注明供程序使用并链接正式版本。
- Git 更新仅支持官方 origin 的干净 main，快进到确认的提交，不 stash/reset/clean；源码与打包版按 `.avas-install.json` 哈希验证本地改动，备份后替换并处理移除的程序文件，不覆盖用户新增文件。GitHub 源码 ZIP 用 `.avas-source.json` 的 export-subst 标记读取基准版本；无安装清单时始终取该完整 SHA 的官方源码归档作为基准（包括 Release 自动 Source code ZIP 和 Code → Download ZIP），不得用同提交的 avas-source.zip 替代，两者换行符与生成文件可能不同；不能以现有文件冒充基准或放宽字节哈希校验。源码/打包版用 GitHub 提交比较防止降级；请求 `per_page=1&page=2`，只用全局 status，避开第一页文件补丁，保留 JSON 响应大小限制。
- 更新准备通过 `gui/maintenance.py` 与 runner、sandbox 启动共享锁；模拟、暂停、扫描或 AI 试算活跃时拒绝，准备期间不允许新模拟。安装退出由 `host.ts` 触发，保存/放弃编辑仍须询问。浏览器模式仅检测与下载入口，服务器需本机维护。
- `.venv`、项目与结果、个人设置保留。替换失败尝试恢复备份；打包启动检查失败恢复旧程序。源码依赖失败保留日志与修复入口，不能声称环境已完整回退。回归测试只用临时安装目录和本地临时 Git 仓库。
- `installation_lock.py` 为当前用户同一安装目录的 CLI、桌面与服务进程持有共享文件锁；独立更新器取得独占锁后才替换，其他进程仍运行时拒绝。更新准备状态可在页面重连后恢复，重启后读取持久化结果，错误日志与备份不自动删除。

## 6. 物理与内核约定（已用内核实际运行核实，手册里没写清楚的以这里为准）

本节多数条目是用原内核（AVAS.dll）核实的。2026-11-15 用新内核重跑了核实工具（`tools/compare_centroid_engine.py`、`compare_response_engine.py`、`compare_preview_engine.py`，设 `AVAS_ENGINE=new`）：新内核的默认模式和兼容模式（`compat = dll`）都复现了这里记下的质心约定、响应矩阵、能量增益、V3 = 0 的解与拒绝运行等结论。兼容模式下的数值与原内核相同（例如 S2 的 Bx 对 x 交叉项 1.205e-5 m/T、同步相位 −29.93°），只在原内核特有的现象上与默认模式不同，相关条目已注明适用哪个内核。用 `tools/compare_kernels.py` 比较两个内核在项目上的结果（M8，`docs/engine/M8-plan.md`）。


- **beam.txt twiss**：β 单位 mm/mrad；发射度是**归一化 rms**，π·mm·mrad（rms_x0 = sqrt(β·ε/(βγ))）；twissz 同样单位，z' = Δp/p。
- **场图**：电场 MV/m × Ke，磁场 T × Kb；射频电场按 E·cos(ωt+φ0)，射频磁场用 sin；V3=0 的同步相位 = atan2(∫E sinφ, ∫E cosφ)。场图存储顺序要按**一组分量文件**判断（`avas/data/fieldmap.py: group_order()`），不能按单个文件判断。
- **示例项目的场图本身有噪声**（2026-09-21 核实）：`examples/hwr010` 的 sol / bfield / hwr010 三组场图沿 z 的二阶差分平均为量程的 6～9 %，`sol.bsz` 在峰值附近逐点在 5.9～10.1 之间跳（`sol` 没有平顶：平滑后峰值约 8.3、位于 z ≈ 0.177 m，半高宽约 7.4 cm；2026-11-02 核实）。`sol` 的横向分量与 Bz 逐点并不自洽（z = 0.1435 m、x = 1 cm 处 Bx 为 −0.99，−(r/2)·dBz/dz 为 −1.44；积分到峰值则只差约 4 %）。线性预览早先直接用 bsx/bsy 算螺线管边缘场，含 `sol` 的周期传输矩阵因此不辛（单个 `sol` 特征值模 0.983／1.017，`sol`+`hwr010` 周期 0.95／1.05），长束线上一个模式逐周期增长约 4 %；2026-11-02 起静磁场的径向部分 ∂Bx/∂x + ∂By/∂y 一律由叠加后的 Bz 求出（−∂Bz/∂z，作为每个 4 mm 光学切片两端的薄边缘），场图的 Bx、By 只提供无散部分（四极、斜四极），单个 `sol` 的周期矩阵严格辛，含腔时模在 0.9995～1.0005（剩余来自加速），相移变化小于 1°。只对称化 ∂Bx/∂x 与 ∂By/∂y 没有用（模仍为 0.983／1.017）。内核直接用场图里这些不自洽的 Bx、By 跟踪（2026-11-03 核实，见“线性模型的质心约定”一条）。切片热图会把它显示成沿 z 的竖条纹——**那是数据，不是读取或切片的错误**。判断存储顺序的 `_cube_roughness` 在这种噪声下区分度很弱（sol 组两种顺序的粗糙度只差约 1 %），只能靠 `group_order()` 按整组投票，并用 Bz 峰值是否落在元件中心这类物理判据复核。横向场的正确性可以这样核实：按 B_r ≈ −(r/2)·dB_z/dz，B_z > 0 时螺线管**入口端**（场强上升）横截面上 (Bx, By) 处处指向轴心，**出口端**（场强下降）处处背离轴心，中心截面上横向场为零（2026-11-01 用 `sol` 核实：z ≈ 0.143 m 与 0.199 m 两个截面 4–18 mm 环带内分别 100 % 指向、100 % 背离轴心，z = 0.175 m 横向场为零；2026-09-21 原写"两端处处指向轴心"有误）。
- **displacepos** 内核按 **mm** 读取（手册写 m，已在 schema.py 注明）。
- **spacechargecomp V**（lattice，2026-11-15 由 AVAS.dll 反汇编与新内核兼容模式逐字符比对核实，docs/engine/60-space-charge.md F-28、M4-notes）：从这一行所在位置起空间电荷乘以 (1 − V)。原内核只有 Windows 的 AVAS.dll 认识（Linux libAVAS.so 忽略），V ≥ 1 时忽略这一行、V < 0 时放大空间电荷；新内核默认模式把 1 − V 限制在 [0, 1]，兼容模式与 AVAS.dll 相同。说明写在 schema。
- **`main_agent` 不读参数**：每次调用都完整运行一次模拟，旧的 `MultiParticle.stop()`（`main_agent(2)`）会让原内核把模拟重跑一遍，已删除；停止运行一律结束 `avas run` 子进程。
- **randomseed**：原内核与新内核兼容模式在种子非 0 时生成的初始分布 x 与相位完全相关（相关系数 −1，C 库 rand() 横向、纵向各自用同一种子 srand）；新内核默认模式没有这个缺陷。beam.txt 的 randomseed 覆盖 input.txt。研究层默认种子 12345，用原内核时发射度、空间电荷类结果受影响，界面在研究页与模拟设置页提示。
- **RF 相位 V3**：V3=2 是 t=0 时刻的绝对相位；synData.txt 的 φRF = phase_t0 + 360·f·t_in。分段运行把中段单独拿出来时，要用 φRF − 360·f·(t_in − T_entry) 重新换算（`segment.rephase`），不能直接沿用或改成 V3=0。
- **输出面**：超出 lattice 末端会报错；离末端太近（5 mm）也会失败，2 cm 可以，所以分段 lattice 会去掉离末端 5 cm 以内的输出面。内核结束时总会写 `outData_<末端>.dst`。
- **单粒子运行**（particlenumber 1 或关闭空间电荷）同步粒子数据相同但 rms 列为 NaN，属正常（原内核与新内核兼容模式；新内核默认模式单粒子的 rms 为 0、质心为有限值）。
- **DataSet.txt**：每行 41 列；z = 第 5 列 + 第 33 列（有二极铁时按 `dataset_envelope` 的规则累加弧长）；rms x/y/z 为第 16/18/20 列（m），最大值第 22/24/26 列，存活宏粒子数第 28 列，能量第 0 列（MeV）。内核运行开始时重建该文件，之后**逐行追加**，读取正在写的文件要丢弃不完整的最后一行（`read_dataset_array` 已处理）。
- **进度行**形如 `Simulate progress 32.83%. Estimated remaining time 3s. Run time 3s.  5260  0.07m`，末尾两个数是存活宏粒子数和当前位置。
  原内核（以及新内核 compat=dll）每 3 s 打印一次；新内核默认模式每 1 s 或每 5 % 打印一次（先到者），格式不变。
- **误差研究**每组都会重新读取项目里的 lattice 文件（`avas/sim/error.py`），当前一次的输出在 `OutputFile/error_middle/output_0/`，完成的结果复制到 `OutputFile/error_output/output_<组>_<次>/`。
- **线性模型**（`avas/model/`，2026-11-03 由 linear_optics.py 拆出）：编辑器预览、入门实验、AI 工具和将来的分析层都用它，不要另写传输矩阵或空间电荷。`LinearModel(text, beam, field_dirs)` 构建一次，`evaluate(space_charge, knobs, errors, centroid0, beam)` 返回 `ModelResult`，一律国际单位（m、rad、MeV，δ = Δp/p）；`linear_preview` 是 `ModelResult.preview()`，输出与拆分前逐位相同（不跟踪质心、不用段缓存）。每一步是仿射映射 x → M·x + d：常数项 d 只来自校正铁、元件错位（`errors` 的 dx、dy、roll，参考轨道不变）和场图轴上横向场（`map_dipoles`，可关闭；示例场图有噪声，`sol` + HWR 一个周期约偏 0.04 mm），只推动质心；σ 是关于质心的二阶矩，线性空间电荷以束流中心为中心，不推动质心。改元件参数（`knobs`，经 `lattice_edit` 定位）属于设计改变，重算参考粒子；场图段按"语句参数 + 入口 z、能量（V3 = 2 时加入口时间）"缓存，场图文件改动后要新建模型。校正铁按 schema 在下一个元件全长上施加均匀场（写 7 个参数，最后是最大值）：磁 By > 0 使正粒子向 −x 偏，Bx > 0 向 +y，电 Ex（kind 1，单位 V/m）向 +x。superpose 块内场图先求和再切片，块内单个场图的错位不建模（报错）。`avas.paths.field_dirs()` 给出项目的场图目录（GUI 的 `Project.field_dirs` 也调用它）。
- **线性模型的质心约定已用内核核实**（2026-11-03，`tools/compare_centroid_engine.py` 单粒子运行，`info["centroid_checked"]` 为 True；改动约定后要重新核对）：磁校正铁 By、Bx 与电校正铁 Ex 的方向、单位和作用长度（末端偏移内核 −1.2312／+1.2312／+7.2069 mm，模型 −1.2316／+1.2316／+7.2094 mm）；四极一阶传输与 `displacepos` 按 mm 读取（沿 z 相对差 1.5e-6）；HWR 离轴传输（0.9 %）；场图轴上横向场的偏转（教程一个 `sol` + HWR 周期，轴上粒子 y 末端 −0.0405 对 −0.0408 mm，0.5 %；不计时差 41 %）；螺线管转向。**离轴穿过示例 `sol` 场图的差别来自场图本身**：内核直接用场图里与 Bz 不自洽的 Bx、By 跟踪（按场图三维插值做 RK4 跟踪可复现内核：+1 mm 起点 x −1.294 对内核 −1.272，y 0.084 对 0.084），而该场图在 1.7 mm 横向网格尺度上不对称，+1 mm 与 −1 mm 起点的响应差约 30 %（内核 −1.272 与 +0.920 mm），且随偏移大小变化；线性模型用轴上中心差分，给出两侧的平均（±1.128 mm），任何线性模型都不能同时符合两侧。改用场图自身的径向梯度既不能消除差别（x −1.114），又使周期矩阵不辛（模 0.926／1.014），不采用；4 mm 切片平均对 ∫Bz² 只有 0.8 % 的影响。原内核单粒子运行不稳定：同样的输入约三成整列质心为 NaN，脚本自动重跑；一次重跑后的静磁算例末端能量出现 1.584 MeV（应为 1.527），这类结果不可信。新内核兼容模式在 Mac 上每次都是整列 NaN（与原内核同类）；新内核默认模式单粒子运行一次即成、质心总是有限值，数值与原内核相同到第 4 位（2026-11-15：校正铁 −1.2312／+1.2312／+7.2068 mm，离轴 `sol` x −1.2718、y 0.0844 mm，`sol` + HWR 周期轴上 y −0.0405 mm）。原内核不接受梯度为 0 的四极。
- **动力学与控制分析**（`avas/dynamics/`，2026-11-03）：一律建立在 `LinearModel` 之上，不另写传输矩阵；单位同模型（流强 mA）。周期取自 `lattice … lattice_end` 组或给定 z 范围。加速周期不严格周期性：一个周期的矩阵乘 √(βγ_出/βγ_入) 归一化，“匹配”指 Σ_出·βγ_出/βγ_入 = Σ_入；归一化后示例 `sol` + HWR 周期的特征值模为 0.9996～1.0004，所以稳定判据容差取 1e-3（`STABLE_TOL`，`control.modes` 的 `stable_tol` 同样 1e-3），带空间电荷的匹配以残差小于 max(1e-6, 4ε) 为收敛、包络映射以最大模不超过 1 + 1e-4 + 2ε 为稳定（ε = 归一化周期矩阵最大模 − 1；示例 sol + HWR 周期 ε ≈ 4.2e-4，0.18 mA 时残差约 8e-4、包络最大模 1.00085；2 mA 时 1.015 才是真实的包络不稳定）。零流强下不稳定的平面（无 RF 时的 z）不参与匹配，沿用入口束流的块；x、y 有耦合时一起匹配；耦合周期的匹配束按本征向量 Re(v·v†) 构造并按本征发射度缩放。带空间电荷的匹配用最小二乘同时满足 F(Σ) = Σ 与本征发射度不变（发射度守恒使 I − DF 有零空间，单纯 Newton 法会奇异）。包络雅可比在匹配束附近对匹配平面块的上三角做中心差分，零流强时本征值为 λiλj，三个 1 对应发射度。已核对：直流束 FODO 匹配包络与 `kv.py` 的二维 KV 方程相差 2e-5；σ₀ ≈ 95° 的 FODO 在流强把 σ 压到约 83° 时包络不稳定（最大模 1.061），σ₀ = 50° 或流强很小、很大时稳定（90° 失稳带）。响应矩阵对校正铁和错位是精确的（模型线性），对其他参数为中心差分。脉冲间积分反馈 u ← u − g·R⁺·y_{k−d} 的每个校正模态满足 z^{d+1} − z^d + g = 0（无延迟 g < 2、一个脉冲延迟 g < 1 才稳定）。**轨道响应矩阵已用内核核实**（2026-11-03，`tools/compare_response_engine.py`：S1、S2 两个校正铁的 By、Bx 对四个监测器 x、y，200 个宏粒子、发射度缩小 1000 倍、`randomseed 12345`）：固定种子后两次基准运行完全相同（噪声底 0，内核多线程也可重复）；只经过漂移段和 HWR 的 S2 元素与内核相差不超过最大元素的 0.06 %，连 S2 的 Bx 对 x 的微小交叉项也一致（1.121e-5 对 1.205e-5 m/T）；穿过示例 `sol` 场图的 S1 元素相差 0.1～2.7 %，内核聚焦略强（M4 的 x 对 S1 By：−0.4146 对 −0.4260 m/T），与离轴质心的结论一致，来自场图本身。不固定种子时每次运行重新抽样粒子，200 个粒子的平均位置起伏约 3.5 µm（每 1 mT 步长 0.0035 mm/mT），会在冲量上游的监测器上也显示出“响应”，比较内核结果时要固定种子。新内核重跑（2026-11-15）：默认模式与兼容模式噪声底同样为 0，最大差别为最大元素的 2.83 %／2.74 %（M4 的 x 对 S1 By −0.4263／−0.4260 m/T），兼容模式的 S2 交叉项 1.205e-5 与原内核相同，默认模式为 0.915e-5（这个元素只有最大元素的 3e-5，受初始分布影响）。
- **研究接口**（`avas/research/`，2026-11-03）：内核**必须在子进程里运行**（内核抛 C++ 异常会结束加载它的进程，notebook 不能跟着崩），命令同 `avas run --mode basic --field --lattice`，测试可替换为假内核（`tests/fake_research_engine.py`）。运行记录在 `<项目>/Research/<研究名>/runs/<指纹前 16 位>/`（`input/` 实际输入、`output/` 只保留 DataSet / avas_run.json / synData / Phase、`engine.log`、`run.json`），也可用 `root=` 放到项目外；绝不写项目的 InputFile / OutputFile。指纹包括全部输入文件（文本按换行规范化）、lattice 用到的场图（文件名、大小、修改时间；`strict=True` 用内容）和内核身份（原内核库的 SHA-1，或新内核的结果档与版本，见第 5.4 节“内核选择”；ini.ini 的 `[engine]` 段不进文件摘要），不含 AVAS 提交号（只记录在 run.json）；指纹已存在且状态为 done / failed 时直接复用（失败也缓存，`force=True` 重跑；stopped 不缓存）。默认 `randomseed 12345`，固定种子时内核可重复，缓存才有意义；统计起伏用 `ensemble` 显式换种子。注意原内核（及新内核兼容模式）`randomseed` 非 0 时生成的初始分布 x 与相位完全相关（相关系数 −1，原内核缺陷），新内核默认模式没有这个问题；比较两个内核的物理结果时原内核要用 `randomseed 0`（`compare_kernels.py` 即如此）。`workers > 1` 时去掉 input.txt 的 `multithreading`（每个内核单线程），指纹因此与多线程运行不同。线性保真度不落盘：lattice 参数走 `LinearModel` 的段缓存，beam / input 改动经临时复制后 `read_beam`，input.txt `spacecharge 0` 关闭模型的空间电荷。内核结果只有 rms 尺寸、rms 散角、x / y 质心和归一化发射度（其余为 NaN），`Diagnostics.read` 对内核结果在相邻行之间按 z 插值（行间距约一个步长）。内核侧只支持 lattice 参数和 beam / input 关键字的变体，确定性的元件错位只能用线性模型（`superposeout` 偏移尚未核实）。研究运行与 GUI 互不知晓：不显示在运行页，也不与 GUI 运行互斥。
- **线性包络预览**（`avas/sim/linear_optics.py`，模型见上一条）只作快速评估：轴上参考粒子的能量与内核同步粒子（synData.txt）相差约 0.01 %，rms 通常差 1–2 %，多粒子效应强时可到 13 %；最终以内核结果为准。**DataSet.txt 第 0 列是全部宏粒子的平均能量**，粒子离轴时在三维场图里感受到不同的 Ez：示例 `hwr010` 离轴 2 mm 处的增益比轴上高 4–6 %，按 σ 1–1.8 mm 的高斯束平均高 2.5–5 %，内核的平均增益比预览高 4.5 %（1.527 MeV，2026-11-02 Windows 核实：0.3556 对 0.3395 MeV）；0.5 MeV 时两个间隙几乎抵消，内核平均增益反而低约 20 %。比较能量要分清同步粒子与平均值；`tools/compare_preview_engine.py` 两者都给出。**多周期 rms 的已知差别**（2026-11-02，20000 个宏粒子、3 次运行核实）：单腔时预览与内核的 rms x、y 相差 < 1 %（2000 个宏粒子时内核自身起伏可达 6 %，不要据此下结论）；教程新默认的 8 周期线上内核横向发射度增长约 16 %（x、y 相近，纵向 56 %），质心偏离轴线最多约 0.44 mm（x）/ 0.22 mm（y），末端 rms y 预览比内核小约 10 %（x 小 1 %）。线性模型里去掉 `sol` 的斜四极项、把 HWR 的 x/y 梯度对称化、或改用按束流尺寸加权的梯度，都只让结果变化 1–3 %，所以这是线性预览的已知局限（无发射度增长、不跟踪质心），不是可以在预览里修正的错误。新内核重跑（2026-11-15，每例一次）：默认模式示例平均增益 0.3555 MeV（原内核 0.3556），V3 = 0 在 0.3、0.8、1.0 MeV 同样拒绝运行，0.7 与 1.05 MeV 同样取相差 180° 的减速解（入口相位 −107.7°、−165.8°）；8 周期线发射度增长 x 11 %、y 19 %，末端 rms y 预览比内核小 9.6 %（兼容模式 17 %、15 %、10.1 %，与原内核相同）。兼容模式的同步相位与原内核一样偏 +0.07°（−29.93°），默认模式为 −30.00°。
- **V3 = 0 的同步相位可能无解**（2026-11-02 用示例 hwr010 核实，Ke = 1.36、φs = −30°）：φs 定义为 atan2(∫E sinφ, ∫E cosφ)，在两个间隙几乎抵消的速度下，扫遍入口相位 φ0 得到的 φs 只覆盖一段角度（0.3 MeV 约 65°～107°，0.8 MeV 约 58°～136°，1.0 MeV 约 14°～144°）。按线性预览，加速的 −30° 解在 0.4–0.65 MeV 与 1.1 MeV 以上存在。**没有加速解时内核接受相差 180° 的解**（Windows 核实：0.7 MeV 入口相位 −107.7°、同步粒子减速 0.1129 MeV，synData.txt 仍记 φs = −29.92°，atan2 实为 150°；预览 −107.79°、−0.11286 MeV），按线性预览这发生在 0.7–0.75 MeV 与 1.05 MeV（1.05 MeV 也已核实：入口相位 −165.8°，同步粒子减速 0.095 MeV，预览相差 0.02°、0.005 %）；有多个根时内核取 φs(φ0) 上升分支上的那个。两种解都没有时（0.15–0.35、0.8–1.0 MeV）**内核拒绝运行**，报 “Initialisation stage error: … the scan phase to fail to find the corresponding RF phase”（核实：0.3、0.8、1.0 MeV）。线性预览先做不动点迭代（有解时都收敛），不收敛时扫描 φ0 并二分求根，依次找加速解、相差 180° 的解（给出“减速”警告），都没有时给出警告和可达范围、取最接近的 φs。
- **弯转方向**：线性预览的 `bend_matrix` 对 α > 0 给出 R16 > 0（x 指向弯转外侧），即正粒子向 −x（竖直弯转向 −y）偏转；元件场示意图（`analyticField.ts` 的 `bendField`）按同一约定定轴上场符号，场梯度指数项 ∂By/∂x = −n·|B0|/ρ 与弯转方向无关并画出 Maxwell 伴随项。**内核方向相同，已核实**（2026-11-15，新内核兼容模式 = AVAS.dll 与默认模式，4000 个粒子、无空间电荷，弯转后 d⟨x⟩/dW：α = +20° 为正、−20° 为负，竖直弯转同样在 y 上；兼容模式下 L = 0 的 bend 只走半个弧长，数值减半；固定种子时 x 与相位完全相关，要用 randomseed 0）。3D 视图（`beamline3dModel.ts` 的 `Orbit`）按此把 α > 0 画成向 −x（竖直向 −y）偏转（此前画反了）；edge 极面按 TRANSPORT 约定（β > 0 外侧路程变短）由相邻 bend 的方向和入口／出口决定倾斜方向（`edgeTilt`）。
- **GPU**：`avas/gpu/` 只封装了 Linux 预编译的 `libPIC.so`，没有源码；不要提议做 Windows GPU 版本。多线程（input.txt `multithreading 1`）是 CPU 上的加速手段。
- **`multithreading` 只能写 1 或不写**：手册写 0 = 关闭，但内核遇到 `multithreading 0`（2、3 也一样）会在起点丢失全部粒子，报 `End of simulation, all particles lost`（2026-09-19 用示例项目核实，与 particlenumber 无关）。关闭多线程就删掉这一行；设置页（`services/simsettings.py`）已按此处理，`avas/utils/inputconfig.py` 对值为 None 的关键字不写行。新内核两种模式保留了这一行为（M6-notes：为保持 CPU 结果不变未改），所以这条规则对两个内核都适用。
- **运行不写 InputFile**：`basic_mulp` 和误差研究（`api/basic._error_study`）都把文本输入复制到 `<输出>/inputs/`，`lattice.txt`（误差研究还有每个种子的 lattice）生成在那里，内核以副本为输入、原目录为场图目录。`tests/test_error_study.py` 用固定 randomseed 的示例做回归（误差参数文件必须逐字节相同）。

## 7. 协作习惯

- **Git 提交不添加 AI 署名**：Claude、Codex 等 AI 助手不得把自己或模型名称写入提交的作者、提交者或共同作者，不得自动添加 `Co-Authored-By`、`Signed-off-by` 或 `Generated with` 等 AI 署名。沿用用户现有的 Git 身份，不修改 Git 身份配置；提交前检查最终提交说明，删除自动生成的 AI 署名，保留真实人类贡献者的署名。此规则同样适用于提交模板和工具自动追加的文字。
- 用户希望先看诊断和方案、讨论确定后再一次性完整实施；方案里的决策点要明确列出。
- 命令行保持简短好用：`avas run --input DIR --output DIR`、`avas plot 类型 --output DIR`、`avas scan --input DIR --target 元件 --param 参数 --values 1,2,3`。
- 旧代码、历史 DLL 和日志已移到仓库外的 `AVAS_NEW/archive/`，不要搬回仓库。
- 示例项目的输出（`examples/*/OutputFile/`）不提交。
