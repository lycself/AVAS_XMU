# 新内核 M2 实现记录

M2 在 M1（engine-rebuild 45e4f39）之上实现：z-code 主循环与 t-code ↔ z-code 切换、硬边矩阵元件（quad、solenoid、bend、edge）、校正铁 steerer、
superpose / superposeend / superposeout、multipole，并对 RFQ 做了实验。原则与 M1 相同：默认修正缺陷，`compat=dll` 以 Windows AVAS.dll 为逐位基准。

## 1. 代码

| 文件 | M2 内容 |
|---|---|
| `engine/src/core/lattice.{h,cpp}` | 元件种类 `Quad / Solenoid / Bend / Edge / Steerer / Rfq`；superpose 块生成类型 4 的场行（规格 10 §2.6.5）、子场参数解析；multipole 生成场图并转成静磁场行；`ReadRFQ` 文件转换；`spacechargecomp` 记录 `{z, 1, V}`；参数不足的兼容行为 |
| `engine/src/core/physics.h` | 新增 `AVAS_HD` 模板：`quad_row` / `quad_sync_row`、`solenoid_row`、`bend_row` / `bend_sync_row`、`edge_row`、`steerer_kick`、`ztot_row`（z 表示 → t 表示），运算顺序按 DLL 反汇编 |
| `engine/src/core/tracker.cpp` | `MethodMP::Run` 的分段（t 段 / z 段、steerer 标记、−4 事件、事件排序）、`zcodeRun`、`HardEdgeTrans` 调度（mode、tRef）、同步行传输、`zTotcode`；t-code 中 steerer 事件（冲量后递归重启）；硬边元件的 t-code 场；类型 4 叠加场；superposeout 出口记录、全部粒子越过后的重启、`rotMode` 束流坐标旋转与 DataSet 第 35–38 列 |
| `engine/src/core/synchronous.cpp` | 硬边元件与 steerer 对同步粒子场为零；类型 4 叠加场（RF 子场写 `EzAmp`）；静态场步中 superposeout 出口变换（规格 20 §2.6.6） |
| `engine/tests/parity/` | 新增 41 个 M2 算例（`cases.json`），参考输出 `refs/`（只存摘要文本，全部 refs 共 5.5 MB）；`run_engine.py` 支持算例自带文件、场图目录取输入目录、单独超时；`compare.py` / `run_all.py` 支持“已知不可复现”算例（`compat_expected_diff`）与 M2 元件 |

M1 的 `S5f_quad_not_implemented` 改为 `S5f_err_not_implemented`（quad 已实现）。

## 2. 比对结果

`python3 engine/tests/parity/run_all.py` 一条命令重跑全部 56 个算例（两种模式）。

- **Windows MSVC，compat=dll：54 个有参考输出的算例中 51 个全部文件逐字符一致**（M1 的 15 个仍全部一致；M2 的 z-code、steerer、superpose、
  superposeout、multipole、输入错误各组全部一致，含 `.dst` 粒子块 SHA-256）。另 3 个是原 DLL 的崩溃 / 未定义行为，不可能复现（见第 4 节）：
  `RFQ1_minimal`、`SP2_rf_static_single`、`SP3_rf_drift_dst`。
- **macOS（Apple clang，libm）**：同样 51 个算例最大相对差 1.5e-11（`Z5_mixed_single` 的 synParticle 场值列；硬边元件的 sinh/cosh/sincos 与
  UCRT 末位不同后被后续步骤放大），其余在 1e-14 量级；`SP2_rf_static_dst`、`SP3_rf_drift_single` 的 scanPhaseData 中 NaN 的符号与 MSVC 不同（数值同为 NaN）。
  `run_all.py` 的非严格判据相应放宽为 1e-10。

| 组 | 算例 | Windows compat=dll |
|---|---|---|
| quad / solenoid / bend(水平、竖直) + edge / quad 与场图腔混合 | Z1–Z5（单粒子与 hwr010_seed12345.dst） | 全部一致 |
| steerer（By、Bx、Ex；后接 quad、后接场图腔） | ST1–ST6 单粒子，ST1 / ST4 多粒子 | 全部一致 |
| superpose：两静磁叠加、腔 + 静磁、腔 + drift | SP1–SP3 | SP1 一致；SP2_dst、SP3_single 一致（DLL 报 “Scan phase error”）；SP2_single、SP3_dst 为 DLL 未定义行为 |
| superposeout 偏移、转角；漏写 superposeend；%f 舍入 | SP4（含多粒子）、SP5、SP6、SP7 | 全部一致 |
| multipole G = 0.1 / 0.2 | MP_sextupole_G1 / G2 | 全部一致 |
| 输入错误（quad 梯度 0、参数不足、首元件为硬边、superpose 参数个数、两个腔叠加、磁极数、solenoid 参数不足） | E1–E7 | 全部一致（返回码、ErrorLog、输出文件） |
| RFQ | RFQ1_minimal | DLL 崩溃，新内核报 not implemented |

## 3. 用 DLL 反汇编 / 实验确认的事实（对规格的更正与补充）

1. **superposeout 的 mode**（DLL InitLattice）：`word6 == 0 且 word5 ≠ 0` 为 1，其余（含只有偏移、没有转角）为 2；规格 10 §2.6.5 写的
   `(word6 == 0 && word5 == 0) ? 1 : 2` 是 .so 伪代码的误读。mode 即 tcodeRun 的 rotMode：1 在 y–z 平面、2 在 x–z 平面转束流坐标。
2. **SetSynPhase 的 superposeout 变换**用 CRT 的 `sin`、`cos`，TcodeTrans 和 tcodeRun 的 rotMode 用静态 sincos。
3. **superposeout 重启**（tcodeRun 4.2.11）在 DLL 中与 zTotcode 一样只用 γβz 求 γ：`z = zSeg − (β·Δt)·c`（zTotcode 是 `(β·c)·Δt`，两处结合顺序不同，影响末位）。
4. **zTotcode**（DLL）：`G = γβz² + 1`，`x = x − (Δt·γβx)·(c/√G)`，`z = z_plane − (√(1−1/G)·c)·Δt`，即忽略横向动量求 γ（规格 40 §4.3 按 .so 写的是完整 γ）。
5. **同步行的弯转**：vz 用 `√((c² − c²/γ²)/(x′² + y′² + 1))`，与粒子 worker 的 `√(((1−1/γ²)·c²)/(…))` 写法不同。
6. **HardEdgeTrans mode 2 的 tRef 平均**由 OpenMP 原子操作按线程完成顺序求和，末位不确定；但前半步之后总是紧跟 mode 0 的后半步，tRef 被最小值覆盖，
   不影响任何输出（不开空间电荷时）。新内核按粒子顺序求和。
7. **分段按 typeName**：参数不足时元件保持默认名 “drift”，于是被当作 t 型元件（E7 的 `solenoid 0.2 0.02 0`）。
8. **t 段事件**：DLL 不检查 `p > ev.back()`，把全部输出面、steerer 中点与 −4 事件放进表后按位置排序，再补段末。
9. **multipole**：生成的场图数值本身已乘 G，转换后的 field 行又把 Kb 设为 G；DLL 实验 G = 0.1 → 0.2 时出口 Δγβx 为 −2.148e-6 → −8.577e-6（比 3.99），
   **有效场 ∝ G² 成立**。另：场图网格间距 2R/N（示例 3 mm），三线性插值对二次场在 1 mm 处给出约 3 倍的值，这是网格粗而非公式问题。
10. **RFQ**：按规格构造 `rfqinputAnn.txt`（13 列）/ `rfqinputV.txt`（14 列）的最小输入，DLL 正常写出 `rfqinput.txt`、`E0-z.txt`，lattice 读完，
    同步粒子阶段经过 rfq 元件时 RFQ 元件的 typeName 仍为默认 “drift”、L = R = 0（synData 第 2 行），随后访问空指针崩溃（Python 子进程退出码 1，
    无 ErrorLog）。即 DLL 与 .so 一样 **RFQ 不可用**（`RFQ::SetParameter` 从未被调用，cell 表为空）；轴上 / 轴外 Ez 是否反号无法从运行中判断。

## 4. 默认模式与兼容模式的差别（物理解释）

| 项目 | compat=dll | 默认 | 物理解释 / 影响（相对 DLL） |
|---|---|---|---|
| 含 RF 腔的 superpose | 腔在块首时多写一个 `invalid`，子场整体错位，组内没有任何场；V3 被写成 0（腔不在块首）；φs = NaN，不加速；候选下标 −1 越界（未定义行为） | 按规格拼出正确的场行，场求和，相位正常扫描 | SP2/SP3 默认 φs = −33.000°，出口 γβz 0.05708 → 0.06311（DLL 不加速） |
| 漏写 superposeend | 结束块的那一行被丢掉（SP6 中下一个 drift 消失，总长 0.45 m） | 打印警告并照常处理该行（总长 0.55 m） | 原内核静默改变 lattice |
| 叠加组长度、位置 | `%f`，舍入到 1e-6 m（SP7：0.4234567 → 0.423457） | `%.17g` 原值 | 原内核把下游所有元件移动最多 0.5 µm |
| superpose 中的 drift 子块 | 用错误下标 `15+15j` 判断，几乎从不识别 | 识别，参与孔径判断 | 组内 drift 的孔径生效 |
| bend 半步判断 | `(zTo − zFrom) < 0.85·弧长` 即走半个弧长：写成 L = 0 的 bend 不分割时只走一半 | 由调用方明确告知是否分割 | Z3（L = 0 的 bend）默认走完整个 20° 弧，DataSet 横向列差到 O(1)；Z4（L = 弧长）两种模式相同 |
| zTotcode / superposeout 重启的 γ | 只用 γβz | 完整 γ、vz = βc/√(1+x′²+y′²) | 只影响 1e-9 m 量级（横向动量很小），Z1_dst 中 ⟨z−zs⟩ 变化 8e-10 m |
| z 段中点输出的参考时间 | 同步行还停在元件入口 | 同步行先推进半个元件 | `.dst` 相位不再偏移半个元件的渡越时间（当前算例未覆盖中点输出） |
| steerer 冲量的荷质比 | 一律取粒子 0 的 q/m | 每个粒子自己的 q/m | 多种粒子时才有差别 |
| multipole 的 Kb | Kb = G（场 ∝ G²） | Kb = 1（场 ∝ G） | MP：G 0.1 → 0.2 时默认出口 Δγβx 比 1.99，DLL 3.99 |
| 参数不足的 quad 以外硬边元件 / steerer | 打印后以默认值入表（名为 drift，L = R = 0） | ErrorLog 报错，返回 1 | — |
| 硬边矩阵与 steerer 冲量后的动量 | γβz 不变，只改 γβx、γβy = γβz·x′，于是 |γβ| 随斜率变化（磁场元件中能量不守恒） | 保持每个粒子的 |γβ|，按新斜率分配：γβz = |γβ|/√(1+x′²+y′²) | 修正前 Z1_dst DataSet 第 0 列在第一块 quad 出口从 1.52694 跳到 1.52708、末端 1.52701（相对 +9e-5 / +5e-5），Z2_dst 在螺线管出口降到 1.52693（斜率减小，同一原因）；修正后全程 1.52694，outData 平均 W 与入口相差 3e-15 |
| 步内插值（输出面、superposeout 出口记录与同步粒子出口） | 动量矢量线性插值，磁场内转动时 |p| 偏小约 (转角)²/8 | 方向线性插值、大小单独插值 | SP4_dst 出口平均 W 的相对偏差从 6.8e-11 降到 3e-15 |
| 弧长 ≠ L 的 bend（如 L = 0）的同步粒子时间 | 相位确定阶段按 L 漂移（L = 0 时不花时间），跟踪却加上弧长（实际半弧）的渡越时间，两者不一致：Z3 跟踪中同步粒子被放回 z = −0.076 m | 相位确定阶段在 bend 入口补上 (弧长 − L)/v | Z3 默认 synData 出口时间 2.6287e-8 s，与 L = 弧长的 Z4 相同，跟踪末行同步粒子位于 z = 0.098 m；下游腔的相位与跟踪一致（DLL 在 162.5 MHz 下会差约 238°） |

多粒子算例默认模式 DataSet 的最大偏差在接近 0 的均值列（按该列最大值归一后显得大）：第 5 列 ⟨z − zs⟩（Z1_dst 0.71、Z2_dst 0.14、ST1_dst 0.14）、
竖直弯转 Z4_dst 的第 4 列 ⟨γβy⟩（0.42）。来源：一是 M1 已有的 `.dst` 读回修正（|p| = βγ，vz = βc/√(1+x′²+y′²)）改变了纵向动量分布；
二是 |γβ| 守恒——带角度离开 quad 的粒子 γβz 略减（Z1_dst 末端 ⟨γβz⟩ 0.0570737 → 0.0570719），在下游漂移中落后，⟨z − zs⟩ 末端 17.4 µm → 3.2 µm
（DLL 中这些粒子反而获得能量）。两者都经过渡越时间 L/vz 与弯转色散 δ·h 放大；rms 与发射度列的差别 < 0.8 %。Z3（L = 0 的 bend）另有半弧 / 全弧的差别，横向列差到 O(1)。

`run_all.py` 另检查默认模式的能量守恒：对只含磁场元件、漂移（与薄 steerer）的 21 个算例（cases.json 中 `conserves_energy`），
DataSet 第 0 列按打印精度必须全程不变，outData_*.dst 的平均 W 必须等于入口值（多粒子相对 1e-12，单粒子按 SingleParticle.txt 的 12 位小数取 1e-11），否则判失败。

## 5. 待用户决定

1. **spacechargecomp V ≥ 1**：DLL 实测 V < 1 时空间电荷乘 (1 − V)，V ≥ 1 静默忽略。目前两种模式都按兼容行为（系数已解析保存，空间电荷在 M4 实现）。
   默认模式是否改成“V ≥ 1 报错”或“V ≥ 1 即完全补偿（乘 0）”？
2. **RFQ**：两个原内核都不能运行 RFQ，`rfqinput*.txt` 的 cell 格式没有文档，物理上正确的 RFQ 模型（两项势、cell 参数含义、相位约定）需要另行确定。
   M2 只实现了 ReadRFQ 的文件转换，`rfq` 元件在两种模式下都报 “not implemented”——这偏离了说明中“实现物理正确版本”的要求，原因是没有可依据的定义，不想猜测。
   是否提供 RFQ 输入格式和期望模型（例如 PARMTEQ 风格的 cell 表），再单独做？
3. **multipole 的 G**：默认按“场图已含 G、Kb = 1”修正。G 的物理含义（生成公式 3 极 `2G/R²·(x²−y²)`）是否需要改成按极尖场或梯度定义，需要物理确认。
4. **L ≠ 弧长的 bend（Z3：`bend 0 …` 写成 L = 0）**：原内核不分割时只走半个弧（长度 < 0.85 弧长即判为半步），默认模式走完整弧长，
   DataSet 横向列与 DLL 相差 O(1)；同时默认模式在相位确定阶段补上 (弧长 − L)/v，使下游 RF 相位与跟踪一致（原内核两处不一致）。
   lattice 的 z 坐标仍按 L 计（不含弧长）。这是物理行为的改变，请确认：是否接受“走全弧 + 补时间”，还是要求 bend 的 L 必须等于弧长（否则报错）？
5. 硬边元件在 t-code 里的简化场（quad 只有 Bx = yG、By = xG，solenoid 只有 Bz，bend / edge 为零）只影响已越过段末、仍在 t-code 中推进的粒子的统计；
   是否在默认模式改成“段末之后不推进 / 不统计”？目前两种模式相同。

## 6. 兼容开关累积清单（M1 + M2）

| 项目 | compat=dll（AVAS.dll） | 默认（修正） | 里程碑 |
|---|---|---|---|
| B-3 V3 = 0 腔 φs 偏差 | 扫描步长 `scantstep/f/N`、相位取步首；主阶段取步末 → φs 偏 +0.07° | 扫描与主阶段同步长、都取步首 | M1 |
| B-1 扫描阶段 V3 = 2 | `φ − 360·f·t` | `φ + 360·f·t` | M1 |
| 扫描中场图外的 EzAmp | 保留上次值 | 置 0 | M1 |
| main_agent(2) | 再运行一次 | 请求停止 | M1 |
| Windows 宽字符路径 | 逐字节去 0 | UTF-16 → UTF-8 | M1 |
| stoi / stod 失败、`vector::at` 越界 | 进程 abort | ErrorLog + 返回 1/2 | M1 |
| field 不足 10 词 | `invalid` 场图或 `exit(1)` | ErrorLog 报错 | M1 |
| drift 不足 4 词 | 打印后以默认值入表 | ErrorLog 报错 | M1 |
| 同步粒子超出孔径、读不到 beam.txt / 分布文件 | `exit(1)` | ErrorLog + 返回 1 | M1 |
| `.dst` 读回 | vz = βc，pz = βγ，`x·0.01` | vz = βc/√(1+x′²+y′²)，pz = γvz/c | M1 |
| 单宏粒子权重、spOutZ0/Z1 | 0、不设 | 1、按 lattice 设 | M1 |
| DataSet 第 0 列与 γp | 不加权、只用 γβz | 加权、完整 |γβ| | M1 |
| SingleParticle.txt / outData 能量 | 只用 γβz（SingleParticle 还写死质子质量） | 完整 |γβ|、实际 mc² | M1 |
| lattice 未知关键字 | 静默 | 控制台警告 | M1 |
| solenoid / steerer / bend / edge 参数不足 | 打印后以默认值（名 drift）入表 | ErrorLog 报错 | M2 |
| superpose 含 RF 腔 | 场行错位（块首多一个 `invalid`）、V3 写成 0（腔不在块首） | 正确的场行 | M2 |
| superpose 中 drift 子块 | 错误下标，几乎不识别 | 识别（孔径） | M2 |
| 漏写 superposeend | 丢掉结束块的那一行 | 警告并处理该行 | M2 |
| 叠加组数字格式 | `%f`（1e-6 m） | `%.17g` | M2 |
| multipole Kb | G（场 ∝ G²） | 1 | M2 |
| bend 半步判断 | 长度 < 0.85 弧长即半弧 | 显式分割标志 | M2 |
| zTotcode、superposeout 重启 | γ 只用 γβz | 完整 γ、vz | M2 |
| z 段中点 `.dst` 参考时间 | 同步行在元件入口 | 同步行推进半个元件 | M2 |
| steerer 冲量 q/m | 粒子 0 | 各粒子 | M2 |
| 硬边矩阵、steerer 冲量后的 |γβ| | γβz 不变（|γβ| 随斜率变） | |γβ| 守恒 | M2 |
| 步内插值的动量大小 | 矢量线性插值 | 大小单独插值 | M2 |
| 弧长 ≠ L 的 bend 在相位确定阶段的时间 | 按 L 漂移 | 补 (弧长 − L)/v | M2 |

## 7. 未实现 / 遗留

- 未实现（明确报错）：空间电荷（含 z 段中点的 HESCFieldPre）、全部 `err_*` 误差、`rfq` 元件、以 superposeout 结束整个 lattice 时同步粒子阶段的收尾，
  以及 M1 列出的随机分布、二次粒子、boundary 等。
- 硬边元件在 z-code 中不判孔径（与原内核相同）；superpose 含 RF 腔时不允许 superposeout（两种模式都报原内核的错误）。
- 没有比对算例的路径：静电子场（superpose 中 type 2）、带转角 θz/θy 的 superposeout、多个 superposeout、steerer 后接 bend、两个相邻 steerer、
  z 段中点输出面、`spacechargecomp` 事件。

## 用户决定（2026-11-07）

"待用户决定"各项按推荐处理：
1. **L = 0 的 bend**：默认模式走完整弧长，并在相位确定阶段计入这段弧长的渡越时间（与写成 L = 弧长的 bend 一致）；compat=dll 复现半弧。
2. **multipole**：默认模式场强与 G 成正比（G 即手册意义上的多极强度）；compat=dll 复现 ∝ G²。
3. **RFQ**：两种模式都报 not implemented，待用户提供真实的 RFQ 输入与期望的物理模型后再实现。
4. **spacechargecomp V**：默认模式空间电荷系数取 1 − V 并截断到 [0, 1]（V = 1 即完全补偿），V 超出 [0, 1] 时打印警告；compat=dll 复现原 DLL（V ≥ 1 静默忽略、负值放大）。M4 实现空间电荷时落实。
5. **已越过段末的粒子**：默认模式按无场漂移推进（不再用简化的 quad / solenoid 场），仍计入统计；compat=dll 复现原行为。M4 之前实现。

M3 已实现上面的第 4、5 项（spacechargecomp 截断与警告；越过段末的粒子无场漂移），见 [M3-notes.md](M3-notes.md)。
