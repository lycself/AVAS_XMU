# 贡献 AVAS / Contributing to AVAS

[中文](#中文) · [English](#english)

面向想要修改代码、调试界面或打包发布的开发者。软件规则与设计约束的唯一来源是 [AGENTS.md](AGENTS.md)（AI 编程助手和人类开发者共用），这里是从头到尾的操作步骤。
For developers changing code, debugging the interface or packaging a release. The single source of design rules and conventions is [AGENTS.md](AGENTS.md) (shared by AI coding assistants and human developers); this page is the step-by-step version.

---

## 中文

### 环境

先按 [README「源码安装」](README.md#源码安装)建好 `.venv`，开发再装一组额外依赖：

```bash
pip install -e .[dev]
```

### 前端开发

界面源码在 `frontend/`（React + TypeScript + Vite），编译结果在 `avas/gui/web/`（已提交到仓库，运行 AVAS 不需要 Node.js）。
修改界面需要 Node.js：

```bash
cd frontend && npm install && npm run build
```

前后端只通过 HTTP 通信（`avas/gui/server.py`：`POST /api/rpc` 调用、WebSocket `/api/events` 事件、`/blob/` 二进制数组），
桌面窗口和浏览器用同一条通路，pywebview 只负责窗口和原生文件对话框。调试界面：

```bash
avas serve --settings <临时 json>                                  # 在浏览器里打开界面
avas serve --settings <临时 json> --fake-engine 60 --fake-lose 0.1  # 「运行」不调内核，重放已有 DataSet.txt，用于检查实时显示
```

`python -m avas.gui.devserver` 是等价的旧命令（固定 `--token dev`）。后端接口在 `avas/gui/services/`，每个页面调用的函数在
`tests/test_gui.py` 中有测试，HTTP / WebSocket 通信层在 `tests/test_server.py`；`tests/test_design_rules.py` 检查写死的颜色、
缺少的中文翻译，以及 `avas/gui/web/` 是否由当前前端源码构建。

### 打包独立程序与安装包

```bash
python packaging/fetch_webview2.py
```

```bash
python packaging/build.py
```

第一条命令（只需一次）从微软官方地址下载 WebView2 安装程序（约 1.7 MB）。第二条命令写入构建信息（时间、git 提交，显示在
**帮助 → 关于** 和 `AVAS.exe info` 中），用 PyInstaller 生成 `dist/AVAS/`（`AVAS.exe` 命令行、`AVASGui.exe` 界面、
WebView2 安装程序），如果安装了 [Inno Setup 6](https://jrsoftware.org/isinfo.php)（`winget install JRSoftware.InnoSetup`），
再生成安装包 `dist/installer/AVAS-2.0.0-setup.exe`：默认为当前用户安装（无需管理员），开始菜单和桌面快捷方式，
可选把 `AVAS.exe` 加入 PATH，缺少 WebView2 时自动安装。界面源码有改动时加 `--frontend` 先重新编译前端。
**exe 不会随源码自动更新**，修改代码后需要重新运行 `build.py`。安装后可运行 `AVAS.exe doctor` 自检。

安装包中文翻译固定保存在 `packaging/languages/`（来源、版本与许可证见该目录），不依赖编译器附带中文文件。打包脚本在 PyInstaller 开始前检查翻译文件；更新翻译后须验证中英文安装脚本编译。

### 目录结构

完整的目录地图（含每个模块的作用）见 [AGENTS.md 第 2 节](AGENTS.md#2-目录地图)，两处不重复维护。

### 翻译维护

界面文字以英文写在前端代码中（`t("...")`），中文翻译在 `frontend/src/i18n/zh_CN.json`（英文原文 → 中文）。
修改后重新编译前端（`npm run build`）。手册中的关键字、参数说明是 `avas/data/schema.py` 中的中英文对照。

### 测试

```bash
pytest
```

`tests/test_gui.py`、`tests/test_segment.py` 会真正运行内核，较慢，只改动对应部分时建议单独跑那个文件。
更多约定（环境、常用命令、修改前的检查清单）见 [AGENTS.md](AGENTS.md)。

### 线性预览与内核对比

`tools/compare_preview_engine.py` 用真实内核检查线性包络预览：在临时文件夹里复制示例项目、引导教程的多周期束线（`sol` + HWR，第 3 步参数 `periods` 与第 4 步默认值 `periods-new` 各一条）和若干低能 V3 = 0 腔，运行内核，并与**旧**预览（修复前的 `avas/sim/linear_optics.py`，默认取提交 `63085a9~1`，作为独立模块载入，不用切换分支）和**新**预览（当前检出的代码）比较末端与最大 rms x / y / z、DataSet 的平均能量，以及 synData.txt 中同步粒子的能量、第一个腔的入口相位和同步相位（相位比较的是度数差）。示例项目和用户项目都不会被改动。需要能运行内核的 Windows 机器（Mac 只能用 `--no-engine` 比较两个预览）。

在 Windows 的仓库目录里（PowerShell 或 cmd）：

```bash
git fetch origin
```

```bash
git switch claude/preview-followups
```

```bash
.venv\Scripts\python.exe tools\compare_preview_engine.py --repeat 3
```

（分支合并后改用 `git switch main` 和 `git pull`。）`--old-ref <提交>` 指定别的旧版本（2026-11-03 起 linear_optics.py 只是 `avas/model/` 的薄包装，脚本会把同一版本的 `avas/model/` 一起取出，以 `avas_model_old` 的名字载入）。`--repeat N` 每个算例运行内核 N 次并给出差值范围（内核多线程，结果略有差别）；`--cases example,periods-new` 只跑部分算例，`--periods 8`、`--energies 0.3,0.4,0.6,0.7,0.8,1.0,1.05,1.1`（默认值，覆盖加速解、相差 180° 的减速解和无解三种情况及其边界）改算例参数，`--particles N` 统一设定每次内核运行的宏粒子数（教程算例默认只有 2000 个，统计噪声较大），`--work 文件夹` 指定输出位置。表格另外列出内核的发射度增长（x / y / z）和最大质心偏移，线性预览没有这些量。表格打印在终端，同时写成工作文件夹里的 `comparison.md` 和 `comparison.csv`（每次内核运行的日志在 `OutputFile_<n>.log`）。内核无法载入时会说明原因并只比较两个预览；某次内核运行失败（例如 V3 = 0 无解时的 scan phase 错误）时照样列出两个预览的结果和预览的警告；算例本身出错不影响其他算例，退出码为 1。这个脚本不在 `tests/` 里，快速 pytest 不会运行它。

### 线性模型质心与内核对比

`tools/compare_centroid_engine.py` 用单粒子运行（`particlenumber 1`、`spacecharge 0`）检查 `avas/model/` 的质心约定：磁校正铁 By、Bx 和电校正铁 Ex 的偏转方向与大小（`steerer-by`、`steerer-bx`、`steerer-ex`），粒子离轴 1 mm / 0.5 mm（`displacepos`，内核按 mm 读取）穿过教程的一个 `sol` + HWR 周期（`offset`，检查螺线管转向和射频场中的一阶传输），以及同一周期轴上粒子被场图自身横向场偏转的量（`map-dipoles`，模型分别给出计入和不计入场图二极分量的结果）；`offset-quad`、`offset-sol`、`offset-hwr` 把离轴算例拆成单个四极、单个螺线管和单个 HWR，`offset-sol-neg` 从 −1 mm 出发（示例 `sol` 场图在轴附近不对称，两侧响应不同）。内核单粒子运行有时整列质心都是 NaN（同一算例这次正常、下次不正常），脚本遇到时自动重跑，最多 `--attempts` 次（默认 3），表里列出尝试次数和末端存活粒子数。每个算例先做编辑器的 lattice 检查（校正铁要写 7 个参数，不能紧跟 start，否则内核会崩溃），有错误就跳过并记录。每个算例在临时文件夹里复制示例项目，不改动示例和用户项目。表格列出内核与模型末端和最大的质心 x / y（mm），末端能量，以及沿 z 的差值 rms 相对内核最大偏移的比例，写成工作文件夹里的 `centroid.md` / `centroid.csv`（含内核运行失败的原因和日志位置），沿 z 的曲线写成 `curves.csv`。在 Windows 的仓库目录里：

```bash
.venv\Scripts\python.exe tools\compare_centroid_engine.py
```

`--cases steerer-by,offset` 只跑部分算例，`--work 文件夹` 指定输出位置，`--no-engine` 只打印模型结果。2026-11-03 已全部核实（结论见 AGENTS.md 第 6 节“线性模型的质心约定”），`avas/model/model.py` 的 `centroid_checked` 为 True；改动质心约定后要重新运行并更新那一条。

### 轨道响应矩阵与内核对比

`tools/compare_response_engine.py` 用内核检查 `avas/dynamics` 的轨道响应矩阵：在临时文件夹里复制示例项目，束线为“漂移 · 校正铁 S1 · `sol` · 校正铁 S2 · HWR · 漂移”，四个位置监测器，内核先不加冲量运行一次，再对 S1、S2 的 By、Bx 各加 1 mT 运行一次（质心对冲量是线性的，单边差分即可）。束流用 200 个宏粒子、发射度缩小 1000 倍、不计空间电荷，取平均位置作为质心，避开单粒子运行的不稳定；每次运行都写 `randomseed 12345`，粒子相同，基准运行做两次，两次之差作为噪声底列在报告末尾。表格列出两个矩阵的每个元素（mm/mT）和差值相对最大元素的比例，写成工作文件夹里的 `response.md` / `response.csv`。在 Windows 的仓库目录里：

```bash
.venv\Scripts\python.exe tools\compare_response_engine.py
```

`--particles N` 改宏粒子数，`--work 文件夹` 指定输出位置，`--no-engine` 只计算模型。核对后把结论写进 AGENTS.md 第 6 节“动力学与控制分析”一条。


---

## English

### Environment

Build `.venv` as in [README's Source installation section](README.md#source-installation) first, then add the development extras:

```bash
pip install -e .[dev]
```

### Front-end development

The front end lives in `frontend/` (React + TypeScript + Vite); its build output `avas/gui/web/` is committed, so running AVAS needs no Node.js. Rebuild with:

```bash
cd frontend && npm install && npm run build
```

Front end and back end talk only over HTTP (`avas/gui/server.py`, FastAPI + uvicorn: `POST /api/rpc` for calls, a WebSocket
`/api/events` for events, `/blob/` for binary arrays); the desktop window and a browser use the same transport, pywebview only
provides the window and the native dialogs. To debug the interface:

```bash
avas serve --settings <scratch json>                                  # opens the GUI in a browser
avas serve --settings <scratch json> --fake-engine 60 --fake-lose 0.1  # a run replays an existing DataSet.txt instead of the engine
```

`python -m avas.gui.devserver` is an equivalent alias (fixes the token to `dev`). Back-end calls are in `avas/gui/services/` and
tested in `tests/test_gui.py`, the HTTP / WebSocket transport in `tests/test_server.py`; `tests/test_design_rules.py` checks for
hard-coded colours, missing Chinese translations and a stale `avas/gui/web/` build.

### Stand-alone build

```bash
python packaging/fetch_webview2.py
```

```bash
python packaging/build.py
```

`build.py` stamps the build (time, git commit; shown in Help > About and `AVAS.exe info`), runs PyInstaller (`dist/AVAS/`:
`AVAS.exe` command line, `AVASGui.exe` GUI, WebView2 bootstrapper) and, when [Inno Setup 6](https://jrsoftware.org/isinfo.php) is
installed, compiles `dist/installer/AVAS-<version>-setup.exe` (per-user install without admin rights, shortcuts, optional PATH
entry, WebView2 installed when missing). Add `--frontend` to rebuild the web page first. The exe does not follow source changes:
rebuild after editing. `AVAS.exe doctor` checks an installation.

The pinned Chinese installer translation is in `packaging/languages/`, with its provenance and license. It does not depend on a compiler-supplied Chinese file. The build checks it before PyInstaller starts; compile the bilingual installer after updating the translation.

### Directory layout

```
avas/            Python package
  cli/           command-line entry points (avas run / plot / gui / serve / info / doctor)
  gui/           GUI back end: server.py HTTP/WebSocket service, app.py desktop window, serve.py browser mode,
                 services/ per-page RPC handlers (fs.py for the browser's own file chooser), web/ the built front end
  ai/            AI assistant: OpenAI-compatible client, tool-call loop, AVAS tools, sandbox simulation, manual search, secrets
  api/           basic.py: unified entry point for simulation and plotting; qt/: interface used by the GUI
  core/          ctypes wrapper around the C++ engine
  model/         the linear model (transfer matrices, space charge, centroid): LinearModel / ModelResult
  dynamics/      dynamics and control analysis on the linear model: periods, envelope stability, responses, control
  sim/           simulation flow: multi-particle, envelope, error studies, matching, acceptance; linear_optics.py preview
  post/          post-processing: analysis/ data analysis, plot/ plotting
  data/          output file parsing (DataSet, BeamSet, dst, ...)
  utils/         I/O and configuration utilities
  engine/        AVAS.dll / libAVAS.so and its dependencies
  static/        atomic mass table, field tables
  gpu/ hpc/      GPU engine and HPC job scripts
frontend/        GUI front-end source (React + TypeScript)
examples/        example project (hwr010)
tests/           pytest test suite
tools/           developer tools run by hand (compare_preview_engine.py: linear preview vs engine;
                 compare_centroid_engine.py: linear-model centroid vs engine, single particle;
                 compare_response_engine.py: orbit response matrix vs engine)
scripts/         personal analysis scripts (not part of the software itself)
packaging/       packaging: build.py, PyInstaller config, Inno Setup installer script, icon
docs/            user manual and changelog
```

This mirrors [AGENTS.md's map](AGENTS.md) (Chinese); keep the two in sync when either changes.

### Translation maintenance

UI text is written in English in the front-end code (`t("...")`), with the Chinese translation in
`frontend/src/i18n/zh_CN.json` (English source → Chinese). Rebuild the front end after changes (`npm run build`). Keyword and
parameter descriptions in the manual come from the English/Chinese pairs in `avas/data/schema.py`.

### Automatic update publishing

安装器更新入口 `avas-latest/setup-latest.ini` 使用稳定的 ASCII schema 1（Schema、Commit、Revision、Name、SHA256）。Revision 为完整历史 `git rev-list --count`，构建传入同一计数与完整提交；官方 CI 必须完整检出。发布从固定版本元数据生成入口，在完整／短标签附件就绪后上传，随后更新应用内 `update.json`。两个入口短暂不同步只会选择各自完整发布的版本。无需更改旧安装器即可继续读取该协议。`/NOCHECKUPDATE` 和静默安装固定使用内置内容。

`.venv\Scripts\python.exe packaging/test_setup_installer.py --iscc <ISCC.exe>` 编译原始中英文安装脚本，并在临时安装器中以脚本化下载、对话框和子进程执行边界验证 Pascal 流程；不安装 AVAS、不访问发布服务。发布 CI 在正式打包前运行它。交接时若指定 `/LOG=文件`，子安装器写入 `文件.latest.log`，避免占用父安装器日志。

AI / 开发者在每次提交前新增 `docs/changes/*.md`，内容要求和汇总规则见 [变更记录](docs/changes/README.md)。不要修改已提交的条目，不手动维护生成的 `docs/update-notes.md`。CI 检查新增非空条目；发布从上次成功发布提交汇总，因此失败构建的说明不会丢失。

Before committing, add a reviewed change fragment under `docs/changes/`. Keep committed fragments immutable. CI checks for new nonempty entries; publishing collects all entries since the last successful release without an AI service or a generated Git commit.

The `publish-update` job in `.github/workflows/ci.yml` runs only for pushes to `lycself/AVAS_XMU:main`, after both CI jobs pass. It builds `AVASUpdate.exe` as an independent one-file executable, builds the Windows bundle and requires Inno Setup to produce the installer, runs startup/helper checks, then runs `packaging/update_release.py --publish`. GitHub Actions must be enabled and the job's `contents: write` permission permitted. No personal access token is needed; the job uses `github.token`.

Each `avas-<full SHA>` release holds `AVAS-<version>-setup.exe`, `avas-source.zip`, `avas-windows.zip` and `update.json`. Only after these are ready is the `avas-latest` metadata asset replaced. Fixed-version assets are not overwritten. The publisher checks ancestry to keep a slower old workflow from replacing a newer update pointer. During replacement a transient metadata download failure is treated as a failed check, never as “up to date”.

Run `.venv\Scripts\python.exe -m pytest tests/test_updates.py` for disposable-installation regressions. After `packaging/build.py --no-installer`, run `.venv\Scripts\python.exe packaging/test_update_helper.py` to verify that the frozen helper runs outside the application folder, preserves diagnostics and restores files after a failed packaged startup check. Do not run a real update against a working developer checkout. The release builder uses `git archive HEAD`, so uncommitted work is intentionally not a publishable source package.

### Tests

```bash
pytest
```

`tests/test_gui.py` and `tests/test_segment.py` actually run the engine and are slow; run just that file when your change only
touches it. More conventions (environment, common commands, the pre-change checklist) are in [AGENTS.md](AGENTS.md).

### Linear preview against the engine

`tools/compare_preview_engine.py` checks the linear envelope preview with the real engine: in a temporary folder it copies the example project, the primer tutorial's multi-period line (`sol` + HWR, once with the step-3 settings as `periods` and once with the step-4 defaults as `periods-new`) and several low-energy V3 = 0 cavities, runs the engine and compares the final and maximum rms x / y / z, the DataSet mean energy, and the synchronous particle's energy, entry phase and synchronous phase of the first cavity from synData.txt (phases as differences in degrees) with the **old** preview (`avas/sim/linear_optics.py` before the fixes, by default commit `63085a9~1`, loaded as a separate module, no branch switch needed) and the **new** one (the checked-out code). Neither the example project nor any user project is touched. It needs a Windows machine where the engine runs (elsewhere `--no-engine` compares the two previews only).

In the repository folder on Windows (PowerShell or cmd):

```bash
git fetch origin
```

```bash
git switch claude/preview-followups
```

```bash
.venv\Scripts\python.exe tools\compare_preview_engine.py --repeat 3
```

(Once the branch is merged, use `git switch main` and `git pull` instead.) `--old-ref <revision>` picks another old version (since 2026-11-03 linear_optics.py is a thin layer over `avas/model/`; the script takes the package from the same revision and loads it as `avas_model_old`). `--repeat N` runs the engine N times per case and shows the spread (the engine is multi-threaded and varies a little); `--cases example,periods-new` runs a subset, `--periods 8` and `--energies 0.3,0.4,0.6,0.7,0.8,1.0,1.05,1.1` (the default: accelerating, decelerating (180° away) and unsolvable cases and the edges between them) change the cases, `--particles N` sets the macro-particles of every engine run (the tutorial cases use only 2000, which is noisy), `--work DIR` sets the output folder. The table also lists the engine's emittance growth (x / y / z) and largest centroid offsets, which the linear preview does not have. The table is printed and also written as `comparison.md` and `comparison.csv` in the work folder (each engine run's log is `OutputFile_<n>.log`). If the engine cannot be loaded the script says why and compares the previews only; when an engine run fails (for example the scan-phase error of an unsolvable V3 = 0 phase) the two previews and the preview's warnings are still listed; a case that breaks does not stop the others and makes the exit code 1. The script is not under `tests/`, so the fast pytest run does not collect it.

### Linear-model centroid against the engine

`tools/compare_centroid_engine.py` checks the centroid conventions of `avas/model/` with single-particle runs (`particlenumber 1`, `spacecharge 0`): direction and size of the kick of a magnetic steerer (By, Bx) and an electric one (Ex) (`steerer-by`, `steerer-bx`, `steerer-ex`), a particle started 1 mm / 0.5 mm off axis (`displacepos`, read in mm by the engine) through one `sol` + HWR period of the tutorial (`offset`: solenoid rotation sense and first-order transport through the RF map), and the deflection of an on-axis particle by the maps' own transverse fields in that period (`map-dipoles`, the model with and without the map dipoles); `offset-quad`, `offset-sol` and `offset-hwr` split the off-axis case into one quadrupole, the solenoid alone and the HWR alone, `offset-sol-neg` starts at −1 mm (the example `sol` map is not antisymmetric near the axis, so the two sides respond differently). Single-particle engine runs sometimes give an all-NaN centroid (the same case fine on one run and not on the next); the script then repeats the run up to `--attempts` times (default 3) and lists the attempts and the surviving macro-particles. Every lattice first passes the editor's checks (a steerer needs all seven parameters and must not follow start directly, or the engine crashes); a case with errors is skipped and noted. Every case copies the example project into a temporary folder; neither the example nor any user project is touched. The table gives the engine's and the model's final and largest centroid x / y (mm) the final energy and the rms of their difference along z relative to the largest engine offset, written as `centroid.md` / `centroid.csv` (with the reasons and log locations of failed engine runs) and the curves as `curves.csv` in the work folder. In the repository folder on Windows:

```bash
.venv\Scripts\python.exe tools\compare_centroid_engine.py
```

`--cases steerer-by,offset` runs a subset, `--work DIR` sets the output folder, `--no-engine` prints the model only. Everything was checked on 2026-11-03 (results in the item on the linear model's centroid conventions in AGENTS.md section 6) and `centroid_checked` in `avas/model/model.py` is True; after changing a centroid convention, run it again and update that item.

### Orbit response matrix against the engine

`tools/compare_response_engine.py` checks the orbit response matrix of `avas/dynamics` with the engine: it copies the example project into a temporary folder, builds the line "drift · steerer S1 · `sol` · steerer S2 · HWR · drift" with four position monitors, runs the engine once without kicks and once with 1 mT on each of S1 By, S1 Bx, S2 By, S2 Bx (the centroid is linear in the kicks, so one-sided steps suffice). The beam has 200 macro-particles, emittances 1000 times smaller than the example's and no space charge; its mean position is the centroid, without the instability of single-particle runs. Every run uses `randomseed 12345`, so the particles are the same; the base case runs twice and the difference is reported as the noise floor. The table lists every element of both matrices (mm/mT) and their difference relative to the largest element, written as `response.md` / `response.csv` in the work folder. In the repository folder on Windows:

```bash
.venv\Scripts\python.exe tools\compare_response_engine.py
```

`--particles N` changes the macro-particles, `--work DIR` the output folder, `--no-engine` computes the model only. Record the result in the "dynamics and control analysis" item of AGENTS.md section 6.
