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
