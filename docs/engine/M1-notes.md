# M1 实现记录：新内核第一个里程碑

范围：漂移段与场图元件（射频 / 静电 / 静磁，文本 3D、文本 1D、二进制场图）的 t-code 跟踪，同步相位（V3 = 0 / 1 / 2，含 `singleparticlemode 1`），单宏粒子（`particlenumber 1`）与 `.dst` 读入，原内核的全部常规输出文件。代码在 `engine/`，比对算例在 `engine/tests/parity/`。本文记录实现中的决定、对规格的更正、兼容开关和比对结果；规格正文（10–90）未改动。

## 1. 代码结构与构建

| 位置 | 内容 |
|---|---|
| `engine/include/avas_engine.h` | C 接口：`path`、`main_agent`、`avas_set_option`、`avas_engine_version` |
| `engine/src/capi.cpp` | 接口实现（Windows 为 `wchar_t*`，其他平台 UTF-8） |
| `src/core/physics.h` | 逐粒子物理：三线性插值、宏粒子 Boris 推进、同步粒子推进。无分配、无虚函数、数值类型为模板参数，`AVAS_HD` 标注（CPU 下为空），供以后编译 CUDA / Metal |
| `src/core/params.*` `lattice.*` `fieldmap.*` | input.txt / beam.txt / lattice.txt 与场图读取 |
| `src/core/beam.*` | 粒子（SoA）与 InitParticle（单粒子、`.dst`） |
| `src/core/synchronous.*` | SetSynPhase / singlePartM |
| `src/core/tracker.*` | Run / tcodeRun / TcodeTrans 与输出文件 |
| `src/core/engine.*` | main_agent 流程、ErrorLog、错误分级 |
| `src/core/dllmath.*` | 与 AVAS.dll 相同的 sin/cos（见 3.2） |
| `tests/unit_tests.cpp`、`tests/run_main.cpp` | 单元测试；命令行驱动 `avas_engine_run IN OUT FIELD [none|dll]` |

构建：`cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release && cmake --build engine/build`。
- Windows：VS 2022 Build Tools 的 cmake（`-G "NMake Makefiles"`，先 `vcvars64.bat`），产物 `avas_engine.dll`。全部源码 `/fp:precise`，只有 `dllmath.cpp` 用 `/fp:fast`。
- macOS / Linux：`-ffp-contract=off -fno-fast-math`。本机 CommandLineTools 的 libc++ 头文件不全时加 `-DCMAKE_CXX_FLAGS="-nostdinc++ -isystem $(xcrun --show-sdk-path)/usr/include/c++/v1"`。
- Python：从源码运行时 `AVAS_ENGINE=new` 让 `avas/core/MultiParticleEngine.py` 加载 `engine/build` 下的新库（`AVAS_ENGINE_LIB` 可直接指定文件），`avas run` 与 `engine/tools/oracle/runner.py` 不用改。
- 选项：`avas_set_option("compat", "none" | "dll")`、`avas_set_option("threads", n)`；环境变量 `AVAS_ENGINE_COMPAT`、`AVAS_ENGINE_THREADS`。

## 2. 比对算例与结果

一条命令重跑（Mac 上读取仓库里的参考输出）：`python3 engine/tests/parity/run_all.py`。
`run_engine.py` 对任一内核库（原 DLL 或新库）运行 `cases.json` 的全部算例并生成摘要，`compare.py` 与 `refs/`（Windows AVAS.dll 的输出）比较；`.dst` 只保存摘要（字节数、头、粒子块 SHA-256、各列矩），FullParticle.txt 只保存行数与 SHA-256。S4 的输入分布 `data/hwr010_seed12345.dst` 是 AVAS.dll 用 `randomseed 12345` 生成的 5260 个粒子（与示例 `part_rfq.dst` 同样大小）。

| 算例 | 内容 |
|---|---|
| S1 | 单粒子轴上：drift / efield V3=0 φs −33 / drift |
| S2a、S2b | 单粒子离轴（x 1 mm、x′ 3 mrad、y′ 2 mrad；x −1 mm、y 0.5 mm、x′ −2 mrad、y′ 1 mrad）穿过 hwr010 与静磁 sol |
| S3a / S3b / S3c | 两腔：V3=0+V3=0；V3=2+V3=0；V3=1（φ 取自 S3a 参考 synData）+V3=0 |
| S4a–S4d | 读入 5260 粒子 .dst，穿过 S1、S3a、S3b 的 lattice，以及带中间输出面的 hwr010 |
| S5a–S5f | 未知元件、drift 参数不足、field 参数不足、场图缺失、无相位解、quad（新内核未实现） |

结果（compat=dll）：
- **Windows MSVC 构建：全部算例全部文件逐字符一致**（synData、scanPhaseData、Phase、SingleParticle、DataSet、synParticle 去时间戳、ErrorLog 去时间戳、文件列表、返回码；outData / inData `.dst` 粒子块 SHA-256 相同，即逐位一致）。单粒子算例的 DataSet 只比较第 28–40 列，见 3.4。
- **macOS（arm64、clang、系统 libm）**：DataSet、SingleParticle、scanPhaseData、Phase、文件列表、返回码逐字符一致；synData 最后几位不同（最大相对差 1.5e-15），synParticle 的场分量最大相对差 5.2e-14，`.dst` 各列矩最大相对差 4.4e-15。全部来自 libm（cos/sin/atan、sincos）的末位差异。要求 `OMP_NUM_THREADS=12`（参考机器的线程数，`run_all.py` 自动设置），见 3.5。

默认模式（缺陷修正）：全部 V3 = 0 腔 synData 的 φs 与目标相差 ≤ 4.5e-12°（AVAS.dll 为 +0.07°）；φs 相对 DLL 移动 −0.070°（V3 = 2 腔为 −0.072° = 360·f·dt_m）；同步粒子出口能量相对差 6.5e-6～9.8e-6；S4 DataSet 各列与 DLL 的最大差为该列最大值的 0.4 %（来自 `.dst` 读回公式和 φs 修正：读回时 |p| 不再多出横向分量，粒子速度相对差约 x′²/2；φs 改变 0.07° 对应 Δz ≈ 0.07/360·βλ ≈ 2e-5 m，两者都与 max|Δz|、max|δ| 列的变化量级一致）。

## 3. 对规格的更正与实现中的发现

### 3.1 Ghidra 伪代码会改写浮点运算的结合顺序（重要）

AVAS.dll 用 `/fp:fast` 编译（`x/c` 被换成 `x*3.3356409519815204e-09` 等），而 Ghidra 输出的 `a*b*c*d` 并不保证就是二进制里的结合顺序。逐位复现必须以反汇编（`llvm-objdump -d`）为准。M1 中查到的与伪代码不同之处：

| 位置 | 伪代码 | 实际 |
|---|---|---|
| Field::GetField（同步粒子与宏粒子）电场项 | `w·F·Ke·1e6` | `(w·F)·(Ke·1e6)` |
| 三线性权重（展开式）最后一项 | `|c·a·b|` 按 `(c·a)·b` | `|c·(a·b)|` |
| SynParticle::Update 动量增量 | `((E·q)·dt)·(1/c)` | `(E·q)·(dt·(1/c))`，Boris 半步同理 `(E·q)·(dt·0.5/c)` |
| SynParticle::Update / TcodeTrans 求 γ | `((uz²+uy²)+ux²)+1` | `(uz²+uy²)+(ux²+1)`（TcodeTrans 第二次求 γ 本来就是这样） |
| SynParticle::Update Boris 旋转参数 | `((ig·dt)·q)·Bx` 等 | `(ig·dt)·(q·Bx)`，`(q·By)·(ig·dt)`，`(q·Bz)·(ig·dt)` |
| SetSynPhase 主阶段腔内 sincos 参数 | `((t−T)·f)·2π + φ·π/180` | `(t−T)·(f·2π) + φ·π/180`；出腔时 cos 用 `(t−T)·(f·2π)`、sin 用 `((t−T)·f)·2π` |

已逐条核对过反汇编的函数：Field::GetField（同步粒子 0x18000c200、宏粒子 0x180004640 的 RF 分支）、SynParticle::Update（0x18008dba0）、TcodeTrans 的 Boris（0x180068500）、SetSynPhase 的扫描点、割线迭代与主阶段腔内积分（0x180039f20）。其余公式（统计量、`.dst` 读回、输出）按伪代码实现，比对已逐位一致。

### 3.2 AVAS.dll 的 sin / cos 有两套

`/fp:fast` 下同一参数同时要 sin 和 cos 时，MSVC 调用静态链接在 DLL 里的 SSE2 sincos（`__libm_sse2_sincos_`，DLL 中 0x180099180），其余地方调用 UCRT 的 `sin`/`cos`。在测试机上两者对随机参数有约 60 % 末位不同。直接调用 DLL 内部 0x180099180 与本机 MSVC 14.42 `/fp:fast` 生成的 sincos 比较 200 万个参数，结果全部相同；新内核把 sincos 放在单独用 `/fp:fast` 编译的 `dllmath.cpp` 里，其余用 `std::sin/cos`（`/fp:precise` 下就是 UCRT）。M1 中用 sincos 的只有 SetSynPhase 主阶段腔内积分（TcodeTrans 的 rotMode 旋转以后也会用到）。

### 3.3 输出格式

- AVAS.dll 的文本输出全部是 Windows 文本模式（`\r\n`），新内核同样用文本模式写，Linux / macOS 上为 `\n`；`compare.py` 比较前统一换行。
- NaN / 无穷按 MSVC iostream 的写法（`-nan(ind)`、`nan`、`inf`）。
- `std::istream >> double`（MSVCP140）读场图与 `strtod` 结果相同（三个示例场图共 21.9 万个数逐位相同），新内核用 `strtod`。
- 文本场图数值为 `值·(1/norm)`（DLL），不是 `值/norm`；norm 为整数时只在 norm 不是 2 的幂时有差别。

### 3.4 DataSet.txt 第 0 列与单粒子统计（更正 D-07 与 40 §4.2.7）

- AVAS.dll 的 TcodeTrans 多一个累加量 `Σ w·sqrt(1+γβz²)`，tcodeRun 用它除以总权重作为 `beam.gamma`；第 0 列 = `(上一步 beam.gamma − 1)·mc²`。所以第 0 列是忽略横向动量的平均动能（与 90-experiments 的实测一致），γp（发射度归一化用）也是它。UpdateBeamParameters 中同一量不加权（权重均为 1 时相同）。
- `particlenumber 1` 时 AVAS.dll 不初始化粒子 0 的权重（50 §2.4.3 已指出），实测 DataSet 第 0–27 列随运行而异（`inf`、`-4.2e158`、`-nan(ind)`、`1.06e-12` 等）。比对只比较第 28–40 列；兼容模式取权重 0，默认模式取 1。

### 3.5 线程分块决定求和顺序

宏粒子按 `cores[0]` 分块（`cores[0]` = 按粒子数查表后截到 `omp_get_max_threads()`），块内累加、块间按序号累加，因此统计量的最后几位取决于机器的逻辑处理器数。新内核按同样规则分块（读取 `OMP_NUM_THREADS`，否则用逻辑处理器数），块在自己的线程池里并行，结果与调度无关。不同核数的机器之间 DataSet 在舍入级别不同；比对时设 `OMP_NUM_THREADS=12`。

### 3.6 输入错误的实际结局（S5 实测）

- `foo 0.1 0.02 0`（未知关键字）：静默忽略，总长不变。
- `drift 0.05 0.02`（少一个参数）：打印 “Error! parameter not enough in element : ” 与单词表后继续，元件保持默认值（半径 0），随后同步粒子在该处报 “Synchronised particles can't reach the exit plane…” 并 `exit(1)`。
- `field 0.21 0.02 0 1 162.5e6`（6 个词，f ≠ 0）：**不是** 10 §2.6.4.7 所说的 `exit(1)`。InitLattice 给第 9 个词加路径前缀时该词不存在，随后补 `invalid` 到 14 个词，于是场图名为不带路径的 `invalid`，报 “Element field file cannot be opened : invalid.edz”，返回 1。f = 0 且不足 10 词时为越界访问后 `exit(1)`（未测，兼容模式按 `exit(1)` 处理）。
- 场图文件缺失：先找 `.edx`，失败时打印一维展开提示再找 `.edz`，报错里的路径就是 `fieldMapPath + "/" + 名字`（例如 `...\InputFile\/nosuchmap.edz`）。

### 3.7 其他

- 同步粒子相位扫描中，同一个腔的 21 个扫描点和割线迭代共用一个 SynParticle，`EzAmp` 在场图范围外不更新。腔正好从场图起点开始时，每次跟踪的第一步用的是上一次跟踪留下的 `EzAmp`，而主阶段第一步用 0，两者 φs 相差约 2e-4°（AVAS.dll 行为，兼容模式保留；默认模式在场图外置 0）。
- 宏粒子在 z 恰好等于 lattice 末端时元件搜索会越界（30 §2.2.1），两种模式都钳到最后一个元件。
- `automaticoutput` 间距 ≤ 0 时原内核死循环，两种模式都报 “Output plane anomaly, check automaticoutput”。
- 缺少 `end` 行时原内核不收尾（之后为未定义行为）；两种模式都按读到的元件收尾，默认模式打印警告。
- 原内核用 GBK 字节打印的 “同步粒子无法抵达出口平面…” 没有复现，统一打印英文句子。

## 4. 兼容开关（`compat`）

| 项目 | compat=dll（AVAS.dll） | 默认（修正） |
|---|---|---|
| B-3 V3 = 0 腔 φs 偏差 | 扫描步长 `scantstep/f/N`、相位取步首；主阶段相位取步末 → φs 偏 +0.07° | 扫描与主阶段都用主阶段步长 `0.01/f/N`（singlePartM 为 0.1）且相位都取步首 → φs 等于目标；此时 `scantstep` 不起作用 |
| B-1 扫描阶段 V3 = 2 | `φ − 360·f·t` | `φ + 360·f·t`（与主阶段一致） |
| 扫描中场图外的 EzAmp | 保留上次值 | 置 0 |
| A main_agent(2) | 不看参数，再运行一次 | 运行中时请求停止（每个时间步检查，报 “Programs terminated by users.”），否则直接返回 0 |
| A Windows 宽字符路径 | 逐字节去 0（非 ASCII 路径损坏） | UTF-16 → UTF-8，打开文件用宽字符接口 |
| stoi / stod 失败、`vector::at` 越界 | 进程 abort（不写 ErrorLog） | ErrorLog + 返回 1/2 |
| field 不足 10 词 | 见 3.6（`invalid` 场图或 `exit(1)`） | ErrorLog “Error! parameter not enough in element : field” |
| drift 不足 4 词 | 打印后以默认值入表 | ErrorLog “Error! parameter not enough in element : drift” |
| 同步粒子超出孔径、读不到 beam.txt / 分布文件 | `exit(1)` | ErrorLog + 返回 1 |
| `.dst` 读回 | DLL 公式：vz = βc，pz = βγ（|p| 偏大），`x·0.01` | libAVAS.so 公式：vz = βc/√(1+x′²+y′²)，pz = γvz/c，|p| = βγ |
| 单宏粒子粒子 0 的权重、spOutZ0/Z1 | 0（DLL 未初始化）、不设 | 1、按 lattice 设 |
| DataSet 第 0 列与 γp | `Σw·√(1+γβz²)/W`（UpdateBeamParameters 不加权） | `Σw·√(1+|γβ|²)/W`，即真正的平均动能 |
| SingleParticle.txt 能量 | `(√(1+γβz²)−1)·938.271875` | `(√(1+|γβ|²)−1)·mc²` |
| outData_*.dst 能量 | `(√(1+γβz²)−1)·mc²` | `(√(1+|γβ|²)−1)·mc²` |
| lattice 未知关键字 | 静默 | 控制台警告 |

## 5. 未实现（明确报错 “… is not implemented in the new engine yet”）

空间电荷（`spacecharge 1`，注意 input.txt 默认是 1）；superpose / multipole / quad / solenoid / steerer / bend / edge / rfq；全部 `err_*` 误差关键字；随机分布生成（`readparticledistribution unknown` 且 `particlenumber` ≠ 1）、`.edst`；`secondarybeam 1`、`boundary 1`、`lpboundary`、`dumpperiodicity`、`pchistogram 1`、`longlimits`、`track`、`probe 1`；field 类型 4。superposeout 的同步粒子坐标变换与 rotMode 旋转未写（M1 不会出现）。

## 6. 遗留问题

- 未覆盖的路径：静电场图（类型 2）、二进制场图、一维场图、`singleparticlemode 1`、停滞判丢（LongitudinalBeamLoss.txt）都已实现但没有比对算例。
- 规格 40 §3.4 的 SimulationTime.txt 只按格式写出（计时值本来就每次不同），PIC 计时恒为 0。
- 原内核 `particlenumber 1` 的 DataSet 统计列不可复现（3.4），兼容模式也不追求一致。
