# 新内核 M5 实现记录

M5 分两段：M5a 做误差研究与束流误差（本节），M5b 做二次粒子、boundary / lpboundary / longlimits 等、其余输出文件、DLL 独有工具和对齐清单的覆盖审计。

## M5a：误差研究与束流误差

在 M4（1974ca1）与用户决定（223979f）之上实现规格 70 第 2–7 节：lattice 中 `err_*` 行由内核自己处理时的抽样、施加、误差场与输出，
束流误差，以及经 Python（`avas run --mode stat|dyn|stat_dyn`）的误差研究全流程。

### 1. 代码

| 文件 | 内容 |
|---|---|
| `engine/src/core/lattice.{h,cpp}` | `ErrorParameter` / `ErrorSample`；`err_step`、`err_beam_stat/dyn(_on)`、`err_quad/cav_ncpl_stat/dyn`、`err_quad/cav_stat/dyn_on` 的解析（含 DLL 的开关补齐规则）；`err_*_cpl_*` 静默忽略；元件新增错位、转角、keErr / kbErr / phaseErr 字段 |
| `engine/src/core/errorset.{h,cpp}`（新） | `generate_error_set`（GenerateErrorSet）、`set_errors_in_elements`（SetErrInEle / SetErrPara）、`write_error_sample`（errSampleOut） |
| `engine/src/core/engine.cpp` | main_agent 的误差循环：a·b 个样本，各写 `output_<k>/`；compat 共用一个 lattice（误差累积），默认每个样本复制标称 lattice 与参数 |
| `engine/src/core/beam.cpp` | 束流误差：生成路径（流强、发射度、失配、坐标平移）、`.dst` 路径（流强、平移，inData.dst 写入施加后的值）；默认模式另对单粒子与 `.edst` 施加 |
| `engine/src/core/tracker.cpp` | Run 开头 errSampleOut + SetErrInEle；t-code `GetErrField`（场图类型 1、3 的错位 / 转角坐标变换、孔径、相位误差、场转回实验室系）；z-code（quad / solenoid / bend / edge）入口 T_in 与出口 T_out |
| `engine/tests/unit_tests.cpp` | 默认抽样的统计（均匀、高斯、不截断、列间独立、种子）、r = 0 的等步长 / 固定值、DLL 的 rand 序列 |
| `engine/tests/parity/` | 34 个 `ER*` 算例（含 8 个默认模式专用的手工对照 `ER4m*`）；`run_engine.py` 汇总 `output_<k>/`、新增 `--cases`；`compare.py` 逐个比较 `output_<k>/`；`run_all.py` 新增误差样本检查与线程检查 |

### 2. 原内核的行为（以 AVAS.dll 反汇编与实验为准；与规格 70 不同之处）

规格 70 以 libAVAS.so 为主，下列各项按 AVAS.dll 改正（compat 照此实现）：

- 均匀数是 `(rand() % 10000) * 0.0001`（/fp:fast 把除以 10000 换成乘 0.0001）；静态 / 束流 r = 1 为 `A_j · u`，动态 r = 1 为 `(A_j + A_j)·(u − 0.5)`。
- GenerateGaussian 返回 `(r·cos)·σ`，缓存 `r·sin`，下一次返回 `缓存·σ`；`u1 = rand()·3.0518e-05`，角度 `rand()·0.00019175`。
- SetErrPara 的转角用 `值 × 0.017453292519943295`（不是先乘 π 再除 180）。
- 场图静态 + 动态同时打开时，幅度为 `((S + D)·0.01 + 1)·K`（不是 `D·0.01 + 1 + S·0.01`）；相位误差 `S6 + D6`。
- 束流开关向量的补齐：长度 ≤ 30 补到 30，否则补到 60（长度正好 30 时不再补到 60）。同时写 `err_beam_stat` 与 `err_beam_dyn` 时，
  动态分支在偏移 30 处读 r，越过数值向量末尾；实测 DLL 输出 “指Specify an unknown error distribution…” 后 exit(1)，compat 照此处理（ER19）。
- `err_beam_stat` 数值行写在 `err_beam_stat_on` 之前时，静态开关块是全零，静态束流误差不起作用（ER21，与规格一致）。
- Field::GetErrField（0x180007050）：射频场 `E = Σ(f·w)·(keErr·1e6)`、`B = Σ(f·w)·kbErr`，相位 `(t − t_cav)·f·2π + (φ_cav + φ_err)·π/180`；
  `0 < z_l < L` 且 `r ≥ R` 时丢失，不看 boundary 与 superposeout；不在网格内时场为零。静电场图在 DLL 中返回零场（从不是误差目标）。
- z-code 的 T_in / T_out（quad 0x180092eb0 等、edge 0x180092330、bend 0x1800961e0 等）：判据看动态行标记，误差向量是元件的
  (dx, dy, dz, θx, θy, θz)。四种元件都做这两个变换。
- 束流误差：生成路径只在动态行标记为 1 时改流强（`dIb·0.001 + I`）与 Twiss（`ε·(1 + e/100)`，`β、α·pow(1 + m/100, 2)`），平移不看标记；
  单粒子（particlenumber 1）走另一条分支，束流误差完全不起作用；`.dst` 路径 `Ib += dIb`，平移同上，inData.dst 写施加后的值。
- r = −1（以及其他未知值）沿用上一次的值，首次为未初始化的栈值：ER20 中 DLL 写出 9.88131e-324，compat 为 0（标为 expected）。

实验复核（规格 90 第一批第 3 节）：`err_step 3 1` 与 `1 3` 的单粒子出口 x，compat 与 DLL 实测一致到打印位数（0.000416116 / 0.000362815 / 0.000305216，
0.000448857 / 0.000415030 / 0.000361641）。

### 3. compat=dll 比对（Windows MSVC，OMP_NUM_THREADS=12）

算例（全部直接调用内核，lattice.txt 保留 err_* 行；z-code + 空间电荷 + 多线程的 ER16 用 `cores[2] = 1`）：
单粒子与 `.dst` 多粒子；quad 动态 r = 0 / 1 / 2（七列全开，含错位与三个转角）；`err_step` 1 1、1 3、3 1、2 2、1 2、2 1；N > 1（N 跨漂移段与第二个四极）；
射频腔静态 r = 0 / 1、动态 r = 1 / 2、静态 + 动态（第二个腔只有动态误差）；静磁场图静态（仅静态、静态 + quad 动态）；硬边螺线管、弯铁与边缘的动态错位；
动态束流误差（生成分布的 13 列全开、`.dst` r = 1 两个样本、单粒子）；静态束流误差（DLL 崩溃，expected）；耦合误差行；r = 3（exit）；
两条束流数值行；数值行写在开关行之前；r = −1（expected）。

结果：**26 个参考算例中 23 个的全部输出逐字符一致**（DataSet、synData、Static / DynamicErrorData、inData / outData 粒子块、SingleParticle、files.txt）；
ER16（SPICNIC + 误差）DataSet 等文本逐字符一致，outData 粒子块末位差 ≤ 1.4e-15（M4 已知的 SPICNIC 求和次序，`compat_tolerance 1e-10`）；
另 2 个为 expected（ER12 静态束流误差 DLL 崩溃、ER20 r = −1 读未初始化值）。`run_all.py --strict` 通过。macOS 与 DLL 的最大相对差 3.4e-11（libm）。

### 4. 默认模式

修正（compat 保持原样）：

| 项目 | compat=dll（AVAS.dll） | 默认（修正） |
|---|---|---|
| 样本之间 | 共用一个 lattice：quad / solenoid / bend / edge 的错位相加、梯度连乘（G 11 → 12.1 → 13.31） | 每个样本从标称 lattice 与参数出发 |
| 静态束流误差 | 只写静态行；仅有静态束流误差时样本缺动态行，访问空指针崩溃；有动态行时静态值也从不施加 | 静态与动态束流误差相加后施加 |
| 抽样的随机数 | 进程全局 rand()，在任何 srand 之前（与 randomseed 无关，后续调用接着序列） | Philox，键 = 种子，计数器 = (样本, 行, 列, 静/动)；种子取 randomseed（beam.txt 优先，0 时取时间并写 `errorData.seed.txt`），所有样本的初始分布也用这个种子 |
| 分布 | 静态 r = 1 为 [0, A) 单边；静态 r = 2 报错退出；束流 r = 2 给 0；未知 r 沿用未初始化值 | r = 0：静态 x、动态等步长 A_j；r = −1：A_j；r = 1：[−A_j, A_j) 对称均匀；r = 2：σ = A_j 的高斯（不截断，静态、动态、束流相同）；其他 r 报错 |
| 两条束流数值行 / 数值行在开关行之前 | 偏移 30 读越界 / 静态开关块全零 | 每种模式取自己那一条数值行与开关行 |
| 场图只有动态误差而静态误差也打开 | t-code 看静态行标记，动态误差被忽略 | 静态或动态任一标记即用误差场 |
| 误差场的孔径 | 不看 boundary 与 superposeout | 与 GetField 相同的条件 |
| 静电场图误差场 | 零场（不可达） | 正确的误差场（同样不可达：静态误差只作用于静磁场图） |
| z-code 出口变换 T_out | 先加回偏移再转回（并以 +dz 作局部 z），元件有转角时不是 T_in 的逆 | 先转回再加偏移（用 T_in 的局部 z），T_out 是 T_in 的精确逆 |
| 单粒子与 .edst 的束流误差 | 不施加 | 施加（单粒子：dx、dy、dx′、dy′，dφ 换成 z 平移，dE 改能量保持方向） |

检验（`run_all.py` 默认模式部分，macOS 与 Windows 都通过）：

- **不累积**：`ER4`（err_step 2 2，单粒子与 .dst）的每个样本与“在标称 lattice 上用 err_step 1 1 手工写同样误差”的单次运行（`ER4m0…3`）
  全部输出文件逐字节相同（8/8）。
- **统计**（单元测试，2×10⁵ 个样本）：均匀分布均值 0、方差 A²/3（偏差 < 2 %）、全部落在 [−A, A)；高斯方差 A²、3σ 外约 0.27 %（不截断）；
  不同列之间无相关；同一种子结果相同，换种子不同。
- **线程无关**：ER3（高斯、3 个样本）、ER16（SPICNIC + 误差）1 线程与多线程的 DataSet / outData 逐字节相同。
- **跨平台**：8 个随机抽样算例的 Static / DynamicErrorData（全部样本）在 macOS 与 Windows 上逐字节相同（`default_errdata_sha256`）。
  跟踪结果与 M1–M4 一样不跨平台逐位（libm 末位）：DataSet 文本相同，粒子坐标最大相对差 2.4e-10。

### 5. 经 Python 的误差研究

`avas run --mode stat|dyn|stat_dyn --seed 7`（`tests/test_error_study.py` 的输入：300 粒子、randomseed 12345、2×2 组，束流 + 腔误差）。
Python 的抽样没有改动。

- Windows：原 DLL 与新内核 compat=dll 的 `error_output/output_<g>_<t>/DataSet.txt` 在三种模式的 15 次运行中**逐字节相同**。
  新内核默认模式与 DLL 的差别来自初始分布生成器（M3 决定）：末端能量相对差 ≤ 3.3e-4，rms x 大 2–3.5 %，都在该测试的容差内。
- `tests/test_error_study.py`：Windows（仓库 .venv，`AVAS_ENGINE=new`）默认模式与 compat=dll 都 3 passed；macOS 同样通过。

### 6. 其他检查

M1–M4 的全部比对：Windows `run_all.py --strict` ALL IDENTICAL（新增 ER 算例在内），macOS 最大相对差 3.4e-11；能量守恒、分段末端漂移、
分布、inData 指纹、线程检查全部通过；`run_physics.py` 在 macOS 与 Windows 通过；单元测试通过。

### 7. 待用户决定

1. **默认模式的误差分布**：r = 1 改为对称均匀 [−A, A)（原静态为 [0, A) 单边）、r = 2 为 σ = A 的不截断高斯（原静态报错、束流给 0）、r = −1 为等步长。
   请确认（另一选择：静态 r = 1 保留单边）。
2. **默认 T_out 改为 T_in 的精确逆**：只在元件同时有偏移与转角时与原内核不同。请确认。
3. **默认模式静态与动态束流误差相加施加**、单粒子与 `.edst` 也施加束流误差。请确认。
4. **误差抽样的种子**：默认取 randomseed（0 时取时间并写 `errorData.seed.txt`），同一次调用的所有样本共用初始分布种子。请确认。
5. superpose 组内的误差行（规格 70 2.4）仍报 not implemented；是否在 M5b 实现。

### 8. 兼容开关累积清单（M1 + M2 + M3 + M4 + M5a）

M1–M3 见 [M3-notes.md](M3-notes.md) 第 5 节，M4 见 [M4-notes.md](M4-notes.md) 第 6 节（不变）。M5a 新增的就是第 4 节的表格各行，另加：

| 项目 | compat=dll（AVAS.dll） | 默认（修正） |
|---|---|---|
| 误差样本的 rand 状态 | 进程全局，跨 main_agent 调用延续 | 不使用 rand() |
| 两条束流数值行 | 动态 r 越界读取，按“未知分布”报错 exit(1) | 正常抽样 |
| 未知 r 的报错 | 打印后 exit(1)（动态束流前缀 0xD6 0xB8） | EngineError（ErrorLog.txt，返回 1） |

### 9. 用户决定（2026-11-08）

第 7 节“待用户决定”各项按推荐处理：
1. 默认模式的误差分布：r = 1 为对称均匀 [−A, A)，r = 2 为 σ = A 的不截断高斯（静态、动态、束流相同），r = −1 为等步长。
2. 默认模式的 z-code 出口变换 T_out 是入口变换 T_in 的精确逆。
3. 默认模式静态与动态束流误差相加施加；单粒子与 `.edst` 也施加束流误差。
4. 误差抽样的种子取 randomseed（0 时取时间并写 `errorData.seed.txt`），同一次调用的所有样本共用初始分布种子。
5. superpose 组内的误差行在 M5b 实现。

## M5b：其余 CPU 功能与对齐覆盖审计

在 M5a（2532dfd）与用户决定（3c5e5b2）之上，实现规格中其余仍报 not implemented 的 CPU 功能，补齐输出文件与 DLL 独有工具，
并完成 280 个对齐项与全部函数的覆盖审计（[coverage.md](coverage.md)）。做完后除 RFQ（缺输入样例）外，原内核的每一项 CPU 功能都已实现。

### 1. 代码

| 文件 | 内容 |
|---|---|
| `lattice.{h,cpp}` | boundary.txt（`read_boundary`、`set_geometry`、`add_geometry`，含 superpose 块）；superpose 组内误差行（记录 `{2, mode, E, …, 类别, 子场号}`）；`Element.subErr` |
| `errorset.cpp` | superpose 记录的抽样（动态；默认也抽静态）与 SetErrPara 类型 4 |
| `tracker.cpp` | boundary.txt 几何损失与 collisionData.txt；lpboundary 折返；longlimits；track（ParticleTracking.txt）；pchistogram（PCHistogram.dat）；dumpperiodicity（BeamSet.plt 头、t 段与 z 段记录）；superpose 误差场；importSCF 调度；“粒子全部丢失”前写 FullParticle.txt；二次粒子 SeRun（`run_secondary`、`se_trun`、`se_zrun`、SeHardEdgeTrans 的四个核、SePlaneOut、SeSynPout） |
| `beam.{h,cpp}` | 二次粒子读入（文本与 .dst）；每粒子自身时间、步长、编号、种类、几何段号、superposeout 计数 |
| `spacecharge.{h,cpp}` | importSCF（读 SpaceChargeMesh.txt 索引、按 RF 周期折叠时间、CIC 读 SpaceChargeSet.plt）；lpboundary 的电荷分配与网格外点电荷 |
| `synchronous.cpp` | superposeout 出口正好是 lattice 末端时结束相位扫描 |
| `tools.{h,cpp}`（新）、`capi.cpp`、`avas_engine.h` | 导出 `sampling`、`meshing`、`dataprocess`（与 DLL 相同的参数；Windows 为宽字符路径） |
| `engine.cpp` | 删除剩余的 not implemented 检查（boundary、lpboundary、dumpperiodicity、pchistogram、longlimits、track） |
| `tests/parity/` | 22 个新算例（LB 边界、LT 输出、LL、LP、LS importSCF、SE superpose 误差、SB 二次粒子）；`tools_check.py`；run_engine 汇总 outData_*.txt、SeSynParticle、ParticleTracking、collisionData 与二进制文件指纹；compare 支持 `compat_ignore_lines`；run_all 新增默认模式检查 |

### 2. 原内核行为（DLL 实测或反汇编，与规格不同或规格未写的）

- boundary.txt：Windows 文本流去掉 CR，CRLF 的 `end` 正常结束；superpose 块不论以 superposeout 还是形状行结束，都在任何输出之前因 stoi 异常结束进程（规格 10 2.9.2 说“以形状行结束时被该行代替”，实测不对，见第 8 节）。
  Geometry::Boundary 的碰撞行最后一列是粒子种类字符串（beam.txt `kindofparticle`；单粒子模式为空），不是整数。
- 任何“粒子全部丢失”退出之前都先写 FullParticle.txt。
- pchistogram：bin = floor((v − v_min)·n/(v_max − v_min))；均值为和乘以 1/计数。
- lpboundary：折返周期 λ·γβz_s/γ̄，半周期判断，先减后加；空间电荷分配只在多线程分区（cores[4] > 1）时以同步粒子为中心拉伸，串行时以质心为中心。
- importSCF：与规格 2.9.3 一致；网格外点电荷的纵向距离用拉伸后的值。
- superpose 误差：InitLattice 每个子块之前只认一条误差行；GenerateErrorSet 写子场槽的同时把元件行第 0 列置 1，所以只有第 1 个以后子场有误差时 t-code 仍用误差场；
  GetErrField 类型 4 不用子场在块内的放置，只用误差变换，转回时有一处调用 `sin()` 而不是静态 sincos。
- 二次粒子：文本读入时 γ 只用 v_z（dt 也是）；.dst 读入时粒子从 z = 0 出发、t = φ/(2πf) − ⟨t⟩、权重为每个宏粒子代表的粒子数；
  .dst 的束流误差平移为 `dx·0.001`（cm，应为 0.1，已确认）；SeTRun 的“z ≥ 0”比较寄存器只在第一步为 0，之后是粒子自己的新 z；
  中间输出面只插值不输出；粒子在场图内不判孔径（superposeout 面未设置）；同步粒子第一行 SeSynParticle 的场是未初始化内存。
  SeHardEdgeTrans 的求和次序与主束 HardEdgeTrans 不同（γ² = (u_y² + u_z²) + (u_x² + 1)，x′ 用乘倒数），新内核单独实现；元件长度取 SeZRun 传入的 (z_入 + L) − z_入（第 8 节）。
- DLL 工具：SPTest.txt 中偏移按 12 位小数写，末行 `offset:` 与 `terminalCoefficient:` 之间没有空格。

### 3. compat=dll 比对（Windows MSVC，`run_all.py --strict`）

- M5b 新算例 19 个（参考由 DLL 生成）全部一致；其中 LP2、LP3、LS1（空间电荷）按已有 1e-10 容差，二进制 BeamSet.plt 在容差算例中只比较长度；
  SB2（二次粒子 .dst 经过四极）在补充中查明原因后逐字符一致（第 8 节）；
  SB1 的 SeSynParticle 第一行（未初始化内存）不比较。
- DLL 工具：meshing（.dst、.dstb）、sampling（.dst、.dstb、SPTest.txt）、dataprocess（.ppj）逐字节一致（.ppj 在补充中改正运算次序后一致，第 8 节）。
- M1–M5a 全部算例继续逐字符一致；macOS 最大相对差 3.4e-11。

### 4. 默认模式

| 项目 | compat=dll（AVAS.dll） | 默认（修正） |
|---|---|---|
| boundary.txt 读到 end | 继续读，第二个 end 再追加一遍段表 | 在第一个 end 停止 |
| boundary superpose 块 | stoi 异常结束进程 | 子几何加 superpose 放置（偏移相加、角度相加） |
| 几何段号夹取 | `r2 < gi < r1` 才夹到 r2（笔误，几乎不成立） | 夹到 [r1, r2] |
| pchistogram 范围 | else-if（刷新最小值后不再比较最大值） | 最小、最大分别比较 |
| track 越界序号 | 越界读 | 报错 |
| lpboundary 周期 | 折返 λ·γβz_s/γ̄，网格 βγλ·Nz/(Nz−1)，不强制 FFT | 折返 β_sλ，网格周期 βγλ，使用 FFT；拉伸总以同步粒子为中心 |
| importSCF 时间等于第一条记录 / 晚于最后一条 | 下标 −1 越界 / abort | 报错 |
| superpose 误差 | 只认动态；不用子场放置；子场 0 以外的误差靠元件行第 0 列 | 静态与动态都认；误差加到子场放置上，幅度、相位同样作用 |
| 二次粒子文件 | 只读 beam.txt `readparticledistribution` | 同上；beam.txt 没写时读 SeParticle.txt（界面写的文件） |
| 二次粒子 γ、dt | 只用 v_z | 用总速度 |
| 二次粒子 .dst 束流误差 | 平移 dx·0.001 cm | dx·0.1 cm |
| 二次粒子孔径与输出面 | 场图内不判孔径；只在段末输出面写 | superposeout 面按 lattice 设置（场图孔径生效）；每个输出面都写 |
| SeTRun 的 z 比较 | 第一步后与自身 z 比较 | 与 0 比较 |
| 工具出错 | exit(1) / exit(0) 结束宿主进程 | 返回错误码 |
| 静场（场图类型 2、3）中同步粒子反向 | 只在射频腔内检查反向，同步粒子退回起点之前后无限循环 | 报 “Synchronous particle reverse motion” |

检验（run_all 默认部分，macOS 与 Windows）：LB1 碰撞点半径等于管半径（1e-6 内）；LP1–LP3 全部粒子在半个 RF 周期内；LL1 全部粒子在相位窗口内；
SB5 读 SeParticle.txt 并在两个输出面各写 3 个粒子；SE4 默认模式“子场误差 dx、dy”与“同样偏移写在 superpose 行”的 outData 横向坐标逐位相同；
LB2、LP2 一线程与多线程逐字节相同；M5a 的全部默认检查继续通过。

### 5. Python 侧导出符号

| 符号 | 调用者 | 新库 |
|---|---|---|
| `path`、`main_agent` | `avas/core/MultiParticleEngine.py`（研究层经 `avas run` 子进程） | 提供（M1） |
| `sampling`、`meshing`、`dataprocess` | 仓库中没有调用者 | 提供（M5b），与 DLL 同签名 |
| `avas_set_option`、`avas_engine_version` | 新内核扩展（parity 工具、`AVAS_ENGINE_COMPAT`） | 提供 |

`avas/gpu/` 经 ctypes 调用的是另一个库 libPIC.so（M6），不属于 AVAS.dll / libAVAS.so。

### 6. 待用户决定

1. **二次粒子文件**：推荐默认模式在 beam.txt 没写 `readparticledistribution` 时读 SeParticle.txt（界面束流页写的就是它），写了时与原内核一样读该文件。
2. **二次粒子输出面**：默认每个输出面都写（原内核只写段末的面）。请确认。
3. **lpboundary 周期**：默认折返周期取同步粒子的 βλ、空间电荷网格周期 βγλ。请确认。
4. **superpose 组内误差**：默认把子场误差加到它在块内的放置上（角度直接相加，小角度近似）。请确认。
5. **DLL 工具**：三个工具在仓库中没有调用者，是否保留在新内核中（.ppj 已逐字节一致）。

### 7. 兼容开关累积清单（M1 + M2 + M3 + M4 + M5a + M5b）

M1–M3 见 [M3-notes.md](M3-notes.md) 第 5 节，M4 见 [M4-notes.md](M4-notes.md) 第 6 节，M5a 见本文件 M5a 第 4、8 节；M5b 新增的就是上面第 4 节的表格各行。

### 8. 补充：覆盖审计中“无算例”条目的算例与两处末位差

审计时 35 个无算例的条目补上 22 个（23 个新算例，参考由 AVAS.dll 在 Windows 上生成），剩下 13 个：按协调意见保留的 RFQ 7 个与 E-01、D-13、D-14、E-05、H-24，
以及 D-28（见下）。覆盖表见 [coverage.md](coverage.md)（267 / 280）。

| 类别 | 算例 | 比较内容 |
|---|---|---|
| 报错路径 | EP_A10_dimension、EP_A13_cores_short、EP_A28_rf_freq0、EP_A33_plane_beyond_end、EP_A33b_plane_descending、EP_D18_plane_interval、EP_D36_tcode_wont_start、EP_F01_scmethod_picnic | rc、ErrorLog.txt（时间戳屏蔽）、出错前写出的全部文件 |
| 退出 / 崩溃 | EX_A22_no_end、EX_A27_steerer_7_words、EX_A47_boundary_sp_out、EX_A47b_boundary_sp_shape_end、EX_D32_multithreading0 | rc、控制台中的消息行（`console_check.txt`）、退出前的文件 |
| 控制台消息 | CK_A09_method_line、CK_F29_spacechargelong、CK_A29_field_type5 | `console_check.txt`（按关键字取出的控制台行） |
| 特殊输入 | SI_B20_sync_reverse、SI_B20s_static_reverse、SI_B26_sync_drift_aperture、SI_C16_sync_field_aperture、SI_C19_binary_map_dst | 同上；C-19 把 sol 场图转成二进制 `solb`（`binary_maps`） |
| G-31 | SB6_secondary_steerer、SB6n_secondary_no_steerer | 与 DLL 逐字符一致；另在 run_all 中检查 DLL、compat、默认三方 SB6 与 SB6n 的二次粒子输出相同 |
| H-14 | tools_check.py `sampling_too_large` | rc（exit(0)）与控制台行；`tools_check.py` 现在比较每次调用的 rc 与控制台 |
| F-30 | 单元测试 `test_fft_norm` | compat 下 1024³ 网格的归一化分母按 32 位 int 溢出（1/8392704），默认 64 位 |

实测得到、并已在 compat 中照做的原内核行为：

- **缺 `end`**（A-22）：不打印“Lattice initialisation complete”与输出面数，相扫描一开始（Phase.txt、scanPhaseData.txt 已截断）读空指针，Windows 上是 access violation。
  compat 用 `compat_crash` 在同一位置结束：Windows 上 `RaiseException(EXCEPTION_ACCESS_VIOLATION)`，ctypes 给出与 DLL 相同的报错与返回码；其他平台 SIGSEGV。
- **未捕获的 C++ 异常**（A-27、A-47）：异常穿出 DLL，Windows 上是 0xE06D7363。compat_abort 在 Windows 上改为抛出同一个 SEH 异常（原先是 abort()，返回码不同）；
  其他平台保持 libstdc++ 的 terminate 文字与 abort()。ER12（只有静态束流误差，空指针读）也改用 compat_crash，原先标 expected，现在逐字符一致。
  这些算例在 run_all 的跨平台比较中跳过 rc.txt 与 console_check.txt（`windows_process_end`）。
- **boundary superpose 块**：以 superposeout 结束和以形状行结束的块都在任何输出之前异常结束（EX_A47、EX_A47b），compat 对任何块都照此处理。
- **同步粒子越过场图元件孔径**（C-16）：打印 GBK 文字“同步粒子无法抵达出口平面，束线存在问题,模拟结束”后 exit(1)；漂移段仍是英文（B-26、C-05）。
- **synData.txt**：每行以 endl 结束，exit(1)、异常与挂起之前已写的行都在磁盘上（未写完的行在 exit 与异常时也写出）。compat 在主阶段逐行写出，退出前写出未完成的行。
- **静场反射同步粒子**（B-20s）：hwr010 的射频场图当作静电场（Ke = −1、0.3 MeV）时同步粒子被反射。原内核只在射频腔里检查反向，同步粒子退回 z < 0 后一直漂移，
  DLL 与 compat 都不结束（算例 20 s 超时，rc 都是 timeout，synData.txt 都只有第一行）。默认模式在同步粒子退回 lattice 起点之前时报反向错误。
- B-20 本身用 0.5 MeV、hwr010 Ke = 16、V3 = 1、相位 180°（射频腔中减速反向）。

两处末位差的原因（都已逐字节一致）：

- **SB2**：SeZRun 调用 SeHardEdgeTrans 时传入元件的入口 z 与 z_入 + L，SeHardEdgeTrans 用两者之差作长度（四极、螺线管、弯铁都是），与元件长度 L 差一个末位；
  原先用 L，5260 个粒子中 8 个在第 15 位不同。compat 改用 (z_入 + L) − z_入 后 SB1–SB5 逐字符一致，SB2 去掉 1e-10 容差。
- **dataprocess .ppj**：按反汇编改正运算次序：二阶矩 `d·d·w`、`d′·d·w`；ε 写成 `β·ε·γ`、α 写成 `(−1/ε)·σ_xx′`；网格分配 `|…|·(1/(h_u h_v))·w·s`；
  网格矩以 `(i − (n/2 − 0.5))·h` 为偏移（不含均值）、乘 `1/(sW)`；纵向 `K = γ̄·(Σw v_z · (1/W))`（Ghidra 把它显示成 `γ̄·Σ·(1/W)`，只有看反汇编才看出先乘 1/W）。
  改完 .ppj 27476 字节逐字节一致。

仍无算例：**D-28**（停滞判丢与 LongitudinalBeamLoss.txt）。判丢要求有粒子在 t-code 区里 80 % 以上的累计步数不越过下一平面。在 hwr010 强减速场中试了
0.3 / 0.5 MeV、Ke 11.5–16、相位 0–330°、前后漂移段 0.01–2 m、加零场腔延长 t-code 区、2000–20000 个粒子：被反射的粒子退到 z < 0 立即判丢，
在腔内往复的粒子最长约 2800 步（需要 4000 步以上）；把场放进 superposeout 块（反射粒子在 z < 0 不判丢）时射频腔不允许，静电场下两个内核都不结束。
没有找到能让 DLL 正常结束又触发停滞判丢的输入。

检查（Windows MSVC）：`run_all.py --strict`（含新算例）、单元测试、`run_physics.py`、`tests/test_error_study.py`（默认与 compat）；macOS `run_all.py`；ruff。
