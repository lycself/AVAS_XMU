# 新内核规格（从原二进制逆向整理）

原内核源码已丢失。本目录是按 `libAVAS.so`（GCC 4.8.5，带 C++ 符号，214 个非库函数）和 `AVAS.dll`（MSVC，无符号，734 个函数）的反编译结果整理的规格，是新内核的实现依据。伪代码本身不进仓库，项目期间放在 L40S 服务器 `/data/public/lycself/avas_engine_work/decomp/`，结束后归档到 `AVAS_NEW/archive/recovered/`。

规格来自读代码；已经用原内核运行核实的结论记在 [90-experiments.md](90-experiments.md)，下文表格的"实测"列标出状态。文中"推测"字样表示证据不足。实现每一部分前要先跑对应的比对实验。

## 文档

| 文件 | 子系统 | 对齐项 | 覆盖的函数 |
|---|---|---|---|
| [structures.md](structures.md)、[recovered_structs.h](recovered_structs.h) | 数据结构布局（偏移 → 字段名），头文件可导入 Ghidra | — | — |
| [10-input-lattice.md](10-input-lattice.md) | 内核入口、input.txt / beam.txt / lattice 解析、场图与 RFQ 文件读取、边界 | A-01～A-50 | 55 |
| [20-synchronous-phase.md](20-synchronous-phase.md) | 同步粒子、V3 = 0/1/2 的 RF 相位确定 | B-01～B-30 | 12 |
| [30-fields-matrices.md](30-fields-matrices.md) | 场图插值、解析元件的场、硬边矩阵传输（z-code） | C-01～C-40 | 43 |
| [40-loop-output.md](40-loop-output.md) | 主循环（t-code / z-code 切换）、Boris 推进、损失、全部输出文件格式、多线程 | D-01～D-37 | 24 |
| [50-distribution-rng.md](50-distribution-rng.md) | 初始分布、随机数、inData.dst | E-01～E-36 | 10 |
| [60-space-charge.md](60-space-charge.md) | 空间电荷（FFT、SPICNIC） | F-01～F-30 | 40 |
| [70-errors-secondary.md](70-errors-secondary.md) | 误差研究、二次粒子 | G-01～G-33 | 30 |
| [80-dll-extras.md](80-dll-extras.md) | DLL 独有的 sampling / meshing / dataprocess，DLL 与 .so 的差异 | H-01～H-24 | DLL 33 |
| [90-experiments.md](90-experiments.md) | 用原内核运行的比对实验与结果 | — | — |
| [M1-notes.md](M1-notes.md) | 新内核 M1 的实现记录、兼容开关、对规格的更正与比对结果 | — | — |

合计 280 个对齐项；`.so` 的 214 个函数全部出现在各文档的函数覆盖表中。每份文档的结构相同：概述、规格正文、对齐清单、函数覆盖表、未解决的问题、对结构表的更正。

## 运行流程总览

`path()` 设定输入输出目录 → `main_agent()` 读 input.txt、beam.txt、lattice（`Initial::Read` / `ReadBeam` / `InitLattice`）→ 确定同步粒子与各 RF 元件的相位（`SetSynPhase`）→ 生成或读入初始分布（`InitParticle`，写 inData.dst）→ 主循环 `MethodMP::Run`：场图元件段按时间推进（`tcodeRun`，Boris 推进，按步长计算空间电荷），矩阵元件段按 z 推进（`zcodeRun`，一阶硬边矩阵 `HardEdgeTrans`，空间电荷以半步冲量施加），沿途写 DataSet.txt、synData、各输出面的 `outData_<z>.dst`。误差研究由 `GenerateErrorSet` 在同一 lattice 上逐组改元件参数；二次粒子走单独的 `SeRun`。

## 已实测核实（2026-11-07，详见 90-experiments.md）

1. **固定种子时初始分布横纵向完全相关**（E）：randomseed 12345 时 inData.dst 中 corr(x, φ) = −1.0000、corr(x′, W) = 0.81；seed 0 时约为 0。原因是横向和纵向各自 `srand(seed)`。仓库里固定种子 12345 的运行（研究层 `DEFAULT_SEED`、`tools/compare_response_engine.py`、`tests/test_error_study.py`）都受影响。
2. **`spacechargecomp V`**（H）：空间电荷乘以 (1 − V)，只在 V < 1 时生效，V ≥ 1 被静默忽略，负值放大；只有 DLL 识别这个关键字。
3. **原内核在固定种子下完全确定**：同一平台重复运行、改 multithreading 都逐字节相同，可以做逐位比对。DLL 与 .so 读入相同粒子时只差 1e-5～1e-4，但不逐位一致；同一种子生成的粒子因 C 库 rand() 不同而完全不同。
4. **DataSet.txt 第 0 列是平均动能**（与 AGENTS.md 一致，D-07 的说法不成立）。

## 读代码发现的原内核缺陷（候选，待实验确认）

新内核对每一项都要决定"兼容复现"还是"修正"（见文末）。

| 现象 | 文档 | 实测 |
|---|---|---|
| superpose 块首是 RF 腔时合成行多 1 个词，子块错位，该段场可能完全缺失；漏写 superposeend 时块后第一个元件消失；块长度按 %f 舍入到 1e-6 m | A §2.6.5 | 实测：腔在块首和块尾**都**不加速，φs 为 NaN |
| `main_agent` 不读参数，Python `stop()` 调 `main_agent(2)` 实际会再启动一次运行；stoi/stod 失败直接 abort，不写 ErrorLog；`!` 紧贴数值时数值被吞掉 | A | 实测：main_agent(2) 完整重跑一次；stop() 目前无调用者 |
| 场图归一化系数用 stoi 读，小数被截断 | C | 未测 |
| 扫描阶段 V3 = 2 用 φ − 360·f·t，主阶段用 φ + 360·f·t | B | 实测：示例布置下对下游 V3 = 0 腔无明显影响；所有 V3 = 0 腔有 +0.07° 系统偏差 |
| RFQ 轴上与轴外 Ez 符号相反；`getEleNum` 返回 −1 后越界读；.so 中 RFQ::SetParameter 无调用者，RFQ 可能不可用 | C、A | 未测（缺 RFQ 输入） |
| SPICNIC 非对称修正写成赋值而非累加，结果随线程分区变化；奇数网格的镜像循环步长为 0（推测死循环） | F | 实测：重复运行与 multithreading 0/1 结果逐字节相同，未见线程相关性；奇数网格未测 |
| 四极、螺线管误差在原值上累加，同一运行内后面的误差组叠加前面的；静态束流误差从不作用到束流 | G | 实测：样本间累乘成立；静态束流误差使内核崩溃 |
| `multithreading` 写 0/2/3 时线程数为 −1，推进块数为 0，粒子全部"丢失"（解释了 AGENTS.md 的实测现象） | D | 已由此前实测（AGENTS.md）印证 |
| 同步粒子 rotMode 1 的旋转使横向动量约翻倍而非归零（推测） | D | 未测 |
| 误差抽样在任何 srand 之前调用 rand()，与 randomseed 无关 | G | 未测 |

## 界面 / schema 与内核不一致（影响 main 上的现有功能）

| 项 | 内核实际行为 | 文档 | 实测 |
|---|---|---|---|
| input.txt `scanphase`、scanData.txt（模拟设置页、分段运行都在用） | 两个 CPU 内核都不识别（只打印未知关键字警告），相位扫描只由 field 行的 V3 决定；GPU 内核 libPIC.so 可能支持 | A、B | 已确认无效 |
| `scmethod PICNIC` | 只认 `FFT` 和 `SPICNIC`（默认 SPICNIC），PICNIC 报 "SCMethod Contains unknown item…" | F | 已确认报错；默认 SPICNIC 已确认 |
| `err_*_cpl_*` 耦合误差 | 内核没有这些关键字，静默忽略 | G、A | 未测（Python 误差研究自己抽耦合误差） |
| `err_step a b` | 幅度级数是 b，schema 与 Python 的理解相反 | G | 已确认（内核里 b 是幅度分级数）；Python 总以 1 1 调用 |
| SeParticle.txt（束流页的二次粒子表） | 内核不读这个文件名，二次粒子文件取自 beam.txt 的 `readparticledistribution` | G | 已确认不读 |
| `lattice`、`lattice_end`、`adjust`、`diag_*` | 内核静默忽略（界面和线性模型自己使用，没有问题） | A | 未测 |
| DataSet.txt 第 0 列 | 规格认为由平均动量算出（D-07） | D | **不成立**：实测是平均动能，AGENTS.md 原写法正确 |
| 弯转方向 | α > 0 时向 −x 弯（与线性预览一致），3D 视图画反了 | C | 未测 |
| 校正铁 | 冲量作用在紧随其后的元件中点，长度取该元件长度，第 7 个参数（最大值）不用 | C | 未测 |

"未测"的项改 main 之前要先实测。

## 下一步：比对实验

第一批实验（界面功能、superpose、V3、DataSet 第 0 列、内核误差处理、可重复性与平台差异）已完成，见 [90-experiments.md](90-experiments.md)。各文档"未解决的问题"里剩下的项按需在实现对应部分之前补做，其中 RFQ 需要一个 RFQ 输入样例。工具在 `engine/tools/oracle/`。

## 已定的兼容性原则（2026-11-07 与用户商定）

- **原内核缺陷默认修正**：上面"原内核缺陷"表中的每一项，新内核默认按物理上正确的方式实现，同时保留"原版兼容"开关，打开后逐项复现原内核行为，用于和原内核比对。
- **随机数换新**：默认使用跨平台一致、横纵向独立的发生器，同一种子在 Windows、Linux、macOS、CPU、GPU 上生成相同的粒子；"原版"模式在对应平台逐粒子复现原内核的 C 库 rand()（Windows 为 MSVC 的序列）。
- **逐位比对以 Windows AVAS.dll 为基准**；libAVAS.so 只作对照（两者读入相同粒子时相差 1e-5～1e-4）。
