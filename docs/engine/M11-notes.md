# M11：硬边元件前的 spacechargecomp、superposeout 块内反射粒子、原内核的运行前防护与补测（2026-11-17）

本批改动修正两处新内核默认模式的问题（兼容模式保持与 AVAS.dll 2026-04-02 逐字节相同），在 Python 侧为随附的原内核加三项防护，并给覆盖审计中“无算例”的条目补上检验。

## 1. 内核改动

### 1.1 写在硬边元件前的 spacechargecomp（默认模式修正）

- 现象：lattice.cpp 把 `spacechargecomp V` 记成 `{z, 1, V}`（z 为当前末端），tracker 只在 t 段（`segment_lattice` 的场图／漂移段事件）里生成 −4 事件。紧接在 quad、solenoid、bend、edge（z 段元件）前面的一行因此在两种模式里都不生效（AVAS.dll 与 07-22 版相同），直到下一个 t 段元件才会遇到别的记录。
- 修正（tracker.cpp `zcode`）：默认模式在每个 z 段元件开始时查找 `z0 ≤ z < z0 + L`（零长度元件取 `z = z0`）的记录，置 `scComp = 1 − V`，同一 z 有多条时最后一条生效（与 t 段相同）。z 段的空间电荷经 `sc_solve` 使用 `b.scComp`，CPU、CUDA、Metal、emu 走同一条路径。兼容模式不变（忽略）。
- 边界情形的核对：
  - 记录在 z 段与 t 段的交界（quad 之后、漂移段之前）：属于 t 段漂移段 `[zS, zE)`，两种模式原本就在漂移段入口生效（兼容模式按 AVAS.dll 在该事件处重启时间步循环），不变。
  - 记录在最后一个元件之后（z = 总长）：没有元件包含它，两种模式都不起作用；之后也没有空间电荷要算，无需处理。
  - 记录在第一个元件之前（z = 0）：t 段事件位于 0，而读入 .dst 的束团头部正好在 0 处，原来新内核两种模式都报 “Tcode won't start, there are particles located behind the output plane”。
    - AVAS.dll 2026-04-02（Windows 参考，2026-11-17）**不报错**：InitParticle 只给单粒子设置 zMin、zMax（反编译 `FUN_180027eb0`），读入 .dst 时第一次进入 tcodeRun 的检查 `events[0] <= zMax` 读到未初始化的值；随后在束团全部越过 z = 0 时按事件处理，像 steerer 一样退回 z = 0 重启时间步循环，此后补偿生效（DataSet 第 6 行回到起点）。
    - 兼容模式改为同样：读入 .dst 时初始 zMax 取 −∞（beam.cpp `init_particles`），这个检查不触发，之后的事件与重启沿用原有实现；Windows 上与参考逐字节相同。生成分布的束流在 AVAS.dll 中同样没有设置 zMax，但没有参考，兼容模式保持 0。
    - 默认模式在该段开始时直接生效、不作为事件、不重启（`tcode` 开头）。
- 决定：只在默认模式修正；z 段元件内的记录按“从该元件起”生效，不在元件中间切换（元件内只算一次空间电荷）。

### 1.2 superposeout 块内反射粒子导致运行不结束

- 复现：示例 hwr010 的射频场图当作静电场（Ke −1.2）放进 superposeout 块，0.5 MeV、能散约 10 % 的 500 个粒子中约 40 个被反射。AVAS.dll 规格（40 §8）只在“所有 superposeout 块都已通过”之后判 `γβz < 0 且 z < 0` 丢失；块内被反射的粒子退回 z < 0 后外场为零，一直向后漂移，而块出口的重启条件（4.2.11）要等每个活粒子都越过出口面，于是时间步循环永不结束（停滞判丢也不触发：块内没有被记录的事件面穿越，`nc = 0`）。新内核两种模式原来都挂起（30 s 超时）；同一 lattice 不放进块时两边都正常结束。
- 默认模式修正（kernels.h `trans_particle`，f32_kernels.h 同一判据）：粒子向后运动并退到所在参考系的入口之前（第一个块之前为 z = 0，块出口重启后为上一个块的出口 `spOutZ0`）即判丢失（`lost = 1`），与原来 z < 0 的规则一致，不写 LongitudinalBeamLoss.txt 或 collisionData.txt（原规则也不写）。没有 superposeout 块时 `spOutZ0 = 0`，判据与原来逐位相同；fp32 层 `spz0 − zs` 在没有块时等于原来的 `zNeg`。修正后该算例 460 个粒子到达末端，与不放进块的同一 lattice 相同。
- 兼容模式（AVAS.dll 不结束，没有参考输出）：tcode 在有未通过的 superposeout 块时，若有活粒子在 z < 0 处向后运动且尚未越过出口面，连续 10000 步后抛 EngineError（“A particle turned back and left the lattice entrance (z < 0) before the exit of a superposeout block …”，返回 2，写 ErrorLog）。只读粒子状态，不改变任何已有兼容模式输出。
- 决定：判丢不另写记录文件；兼容模式用“连续 10000 步”的上限而非立即报错，因为 `longlimits` 仍可能在有限步内把该粒子判丢、让 AVAS.dll 正常结束。

### 1.3 停滞判丢重复记录（默认模式，补测时发现）

M10 D4 的 planeStore_ 保留相邻事件面的记录时，被停滞判丢的粒子在下一步仍满足条件，LongitudinalBeamLoss.txt 会把同一粒子再写一次。默认模式跳过已丢失的粒子（`lost == 1`），兼容模式不变。只影响停滞判丢真正触发的运行（已有算例没有）。

## 2. Python 侧对原内核的防护（原内核二进制不能修改）

- **CRLF 的 boundary.txt（libAVAS.so）**：.so 读 boundary.txt 不规范换行，`start\r` ≠ `start` 后段错误。所有内核运行都经 `avas run` 的输入快照（`avas/api/basic.py` 的 `stage_inputs`、`_error_study`；分段运行、研究层、AI 沙盒、扫描、入门教程复制输入后同样经过它）。原内核在非 Windows 上运行时，快照里的 boundary.txt 改为 LF（`avas/data/inputs.py` 的 `prepare_for_original_kernel`），用户的 InputFile 不动；新内核与 Windows 上的 AVAS.dll 不改。
- **boundary.txt 的 superpose 块**：AVAS.dll 2026-04-02 与兼容模式读到块时在任何输出前 stoi 异常结束（spec 10 §2.9.2，EX_A47、LB5）。`input.txt boundary 1` 且 boundary.txt 在 `start` 之后有 `superpose` 行时，运行前检查（`kernels.require_ready`，run.check、engine.check、分段运行、研究页内核运行、AI 沙盒共用）对原内核和兼容模式给出 `UserError`（中英文），提示改用新内核默认模式；`avas run` 同样在写任何文件前以退出码 2 拒绝（`engines.boundary_superpose`）。
- **superpose 块首的射频腔（情形 A，spec 10 §2.6.5.2 缺陷 1）**：原内核与兼容模式里该块没有场（扫相位失败或 φs 为 NaN，不加速）。运行前检查的 `warnings`（`kernels.input_warnings`）与 `avas run` 的控制台给出不阻止运行的提示，列出 lattice 行号（`engines.rf_at_superpose_start`，用 `lattice_doc.LatticeDocument`：块内射频场图元件前的 superpose 行 z0 = 0）。

## 3. 补测（覆盖审计“无算例”条目）

| 条目 | 检验 |
|---|---|
| D-28 长期停滞判丢 | `M11_D28_stall_long_dst`（`data/stall_long.dst`：三个 1.527 MeV 粒子与一个 10 eV 粒子，20 mm 处输出面，其后 5 m 漂移）：第 2000 步后慢粒子被判丢，LongitudinalBeamLoss.txt 一行 `3-0.000500000000  …`（编号紧接 x），两个输出面各 3 个粒子；两种模式相同（run_all `m11_check`）。`M11_D28_stall_overlap_dst`（仅新内核）检查 1.3 的单行记录 |
| B-21 RFQ 扫相位 | `rfq_check.py` 第 3 项（单粒子）：正常扫描保留 +90°；A1 取负（轴向场反向）时保留重扫的 −90°；为 1 keV 设计的 RFQ 两个相位都不加速，报 “RFQ scanning phase abnormality”（返回 1，无 rfq 行）。默认模式两相位都试、取出口能量高者（M9-notes） |
| D-13 进度行 | run_all `log_format_check`：默认模式全部算例的进度行按正则核对格式，同一取整运行时间内相邻两行百分比至少差 5 %；兼容模式另跑一次 10 万粒子带空间电荷的运行，进度行格式相同、间隔不少于 3 s |
| D-14 SimulationTime.txt | 同上，两种模式所有完成的运行：一行、精度 5 的数值、c5 写两次 |
| E-05 seed 0 | run_all `seed_check`：默认模式 seed 0 写 inData.seed.txt（取当前时间），用该种子重跑 inData.dst、DataSet.txt、outData 逐字节相同 |
| H-24 未引用字符串 | 已有覆盖：每个算例 OutputFile 的文件名集合（files.txt）与 AVAS.dll 参考比较（compare.py）；工作目录由 run_all `stray_file_check` 检查两种模式不写任何其他文件 |
| E-01 glibc rand | 改记“不适用”：逐位基准是 AVAS.dll（MSVC rand），新内核不复现 .so 的 glibc 序列 |

## 4. 新算例与需要的 AVAS.dll 参考

cases.json 新增 6 个算例：

| 算例 | 内容 | 兼容模式参考 |
|---|---|---|
| `M11_scc_before_quad_dst` | SC5 的 lattice，第一个 quad 前 `spacechargecomp 0.5`；兼容模式输出与 SC5_spicnic_Z1 逐字节相同，默认模式只从 quad 起变化，出口 rms x、y 的空间电荷效应为不补偿时的 0.56、0.68 | 已由 Windows AVAS.dll 2026-04-02 生成，兼容模式逐字节相同 |
| `M11_scc_at_start_dst` | 第一个元件前 `spacechargecomp 0.5`；兼容模式与 AVAS.dll 一样不报错，束团越过 z = 0 后重启并生效；默认模式从起点生效 | 已生成 |
| `M11_D28_stall_long_dst` | 长期停滞判丢；AVAS.dll 在判丢行的粒子编号和 synParticle.txt 第一行写入未初始化的内存，这两行不比较（`compat_ignore_lines`） | 已生成 |
| `M11_sp_reflect_static` | superposeout 块内反射；`compat_expected_diff`：AVAS.dll 不结束（参考生成时以 60 s 超时记 timeout），兼容模式报错 | 不需要（可照常生成，比较时标 EXPECTED） |
| `M11_sp_reflect_noblock` | 同上不放进块（仅新内核） | 不需要 |
| `M11_D28_stall_overlap_dst` | 停滞判丢与相邻事件面（仅新内核） | 不需要 |

## 5. 检验结果（Mac M2，2026-11-17）

- 单元测试通过；`run_all.py` 退出码 0，兼容模式对 AVAS.dll 参考的最大相对差仍为 3.44e-11，新增的默认模式检查全部通过。
- 改动前后（同一构建目录，改动前的库另存）全部 204 个已有算例的输出：兼容模式逐字节相同；默认模式只有 SB3 的 errorData.seed.txt（取时间的种子）不同，其余逐字节相同。
- `run_gpu.py --device metal --fp32`：Metal 与 emu fp32 逐字节相同 210/210、重跑相同；`--device emu`：无空间电荷算例 emu fp64 与 CPU 逐字节相同 177/177，空间电荷算例中 9 个 FFT／周期盒算例与 CPU 只差舍入（≤ 2.4e-13，与改动前的库逐项相同），其余逐字节相同。
- `run_physics.py`（含 rfq_check 新增的扫相位三项）通过；pytest 除 macOS 上已知的两项（test_run_path 的 stage_inputs 测试、test_updates[frozen]）外通过；ruff 通过；前端 `npm test`、`npm run build` 通过（zh_CN.json 新增一条）。

## 6. 留待处理

- 上表三个算例的 Windows AVAS.dll 参考（`run_engine.py --reference` 在 XPS 上生成后放入 `refs/`）。
- `M11_scc_at_start_dst` 的兼容模式按新内核对 AVAS.dll 时间步循环的复现报错，参考生成后核实。

## Windows 复核（XPS，MSVC + CUDA 12.4，2026-11-17）

- 三个新算例的兼容模式参考由 Windows 上的 AVAS.dll 2026-04-02 生成（`M11_scc_before_quad_dst`、`M11_scc_at_start_dst`、`M11_D28_stall_long_dst`）。第一次比对发现两处：AVAS.dll 在 lattice 起点的 spacechargecomp 处不报错（据此改了兼容模式的初始 zMax，见 1.1），长期停滞算例的判丢行编号与 synParticle.txt 第一行是 AVAS.dll 的未初始化内存（标 `compat_ignore_lines`）。
- 改正后 `run_all.py --strict` 与 AVAS.dll 参考全部逐字节相同；GTX 1650 上 fp64 无空间电荷算例与 CPU 逐字节相同 177/177，fp32 与 CPU 上的 fp32 参考逐字节相同 210/210、重跑相同；rfq_check（含 B-21 三个分支）通过。

