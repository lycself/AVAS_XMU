# libAVAS.so 核心数据结构布局（逆向恢复）

来源：Ghidra 12.2 对 `libAVAS.so`（GCC 4.8.5，x86-64，带 C++ 符号、无 DWARF）导出的 214 个函数伪代码，
另外直接读了同一个 `.so` 的动态重定位表（vtable 内容）和 `.rodata` 字符串（关键字比较时 Ghidra 丢掉的字面量）。
同目录的 [`recovered_structs.h`](recovered_structs.h) 是同样内容的 C 定义，全部偏移和 sizeof 已用
`clang -target x86_64-linux-gnu` 的 `_Static_assert(offsetof…)` 核对过（只证明头文件与本表一致，不证明推断本身）。

## 0. 阅读约定

- 偏移为十六进制字节偏移；"把握"一栏：**确定** = 有直接读写证据且含义与物理/文件格式吻合；**推测** = 位置或类型确定但含义靠旁证，或者只见到一处访问。
- 证据列写函数名（`类::函数`），必要时附一句话说明；伪代码文件在 `decomp/libAVAS/functions/<地址>_<名字>.c`。
- C++ 容器：`std::vector<T>` = 24 字节 `{begin, end, cap}`；`std::string`（GCC 4.8 COW）= 8 字节指针，`_Rep` 头在指针前 0x18 字节。
  伪代码里把空串 `_S_empty_rep_storage` 误标成 `&std::ofstream::vtable`，看到这个就是"默认构造的 std::string"。
- **Ghidra 参数错位**：凡是被 Ghidra 当成静态函数的成员函数（例如 `Quadrupole::GetField(double, Particle&, …)`、
  `Lattice::GetExternalField(double, Particle&)`），伪代码里的 `param_2` 实际是 `this`，`param_3` 才是 `Particle&`。
  `Initial::InitLattice` 里 `param_2` 是 Lattice、`param_3` 是 ErrorParameter；`Lattice::SetMatrix` 里 `param_4` 是 FieldMatrix。下文引用时已按实际含义换算。
- 单位：未注明一律 SI。γβ 指 p/(mc)。lattice 行按空格拆成 `words`，`word[k]` 位于 `words.begin + 8k`。

## 1. 结构一览

| 结构 | sizeof | 依据 | 整体把握 |
|---|---|---|---|
| Particle | 0x190 (400) | 粒子数组步长 400（`LeapFrog::Update(double,Beam&)`、`InitParticle` 的 `new[] (N+1)*400+8`） | 确定；0x148–0x16f、0xf8 未见访问 |
| SynParticle | 0xb8 | `Initial::SetSynPhase` 栈布局：对象 @-0x918，+0x78/+0x80/+0xa0/+0xa8/+0xb0 都有同名局部变量，下一个对象在 +0xc0 | 确定（尾部可能还有 8 字节对齐） |
| Beam | 0xe8 | `main_agent` 栈上 Beam(@-0x858) 与 Initial(@-0x768) 相距 0xf0，最后成员 +0xe0 | 成员确定；sizeof 推测（0xe8 或 0xf0） |
| Element（= Drift） | 0x88 | `InitLattice`：`operator new(0x88)` 后内联构造 Drift | 确定 |
| Quadrupole / Solenoid / Steerer / Bend / Edge / Field / RFQ | 0xf8 / 0xc0 / 0xa8 / 0x108 / 0xf8 / 0x1f8 / 0x178 | `InitLattice` 中各自的 `operator new(size)` 紧跟构造函数 | 确定 |
| FieldMatrix | 0x1c0 | 嵌在 Lattice+0x18，Lattice 下一个成员在 +0x1d8 | 确定 |
| Lattice | 0x268（栈上占 0x270） | `main_agent`：`Lattice local_548[624]`；最后成员 0x250 vector | 确定 |
| Geometry | 0x100 | `InitLattice`：`operator new(0x100)` + `Geometry::Geometry` | 确定（成员含义多为推测） |
| Boundary | 0x78 | 构造/析构只碰 0x00–0x78 | 确定 |
| ErrorSample | 0x48 | 析构函数 | 确定 |
| ErrorParameter | ≥ 0xc8 | 析构函数 | 只恢复了容器位置 |
| Mesh | 0x140 | `MethodMP::Run` 栈上 `Mesh local_1ec8[320]` | 确定（8 个三维数组各自含义推测） |
| PIC | ≥ 0x530 | 析构函数最后访问 +0x528 | 只恢复关键成员 |
| Initial | 0x220 | `main_agent` 栈上 Initial(@-0x768) 与 Lattice(@-0x548) 相距 0x220；最后成员 +0x21c | 确定 |
| MethodMP | 0x2a8（推测） | `main_agent`：`long local_2d8[85]` | 只恢复关键成员 |
| LeapFrog | 1（空类） | `MethodMP::Run` 中 `LeapFrog local_2139` 只占 1 字节，`Update` 不访问 `this` | 确定 |

## 2. Particle（宏粒子，0x190）

`Beam.particles` 用 `new Particle[N+1]` 分配（8 字节计数 cookie 在数组前），**下标 N 是同步/参考粒子**：
`LeapFrog::Update(double,Beam&)` 先推 0..N-1（跳过 `lost==1`），再无条件推第 N 个；`UpdateBeamParameters` 用第 N 个算相对坐标。
`Particle` 的默认构造只做两件事：`+0xfc = 0`、`+0x188` 构造空串（`InitParticle` 里 `new[]` 后的内联循环）。

| 偏移 | 类型 | 建议字段名 | 含义与单位 | 证据 | 把握 |
|---|---|---|---|---|---|
| 0x000 | double | x | 横向位置 m | `LeapFrog::Update`：`x += γβx·c/γ·dt`；`InitParticle`：dst 的 cm 值 /100 | 确定 |
| 0x008 | double | gbx | γβx | `LeapFrog::Update` 的 Boris 推进；`InitParticle`：`x'·γβz`；`dstOut` 输出 `γβx/γβz` 作为 x' | 确定 |
| 0x010 | double | y | m | 同上 | 确定 |
| 0x018 | double | gby | γβy | 同上 | 确定 |
| 0x020 | double | z | 实验室纵向坐标 m | `Lattice::GetExternalField` 用它和元件起止 z 比较定位元件；`simulationProgress` 打印同步粒子的它（进度行末尾"0.07m"） | 确定 |
| 0x028 | double | gbz | γβz | `InitParticle`：`γ·v_z/c` | 确定 |
| 0x030–0x040 | double[3] | extE | 元件给出的外电场 Ex,Ey,Ez（V/m） | 各 `*::GetField(double,Particle&,…)` 写入；`Quadrupole::GetField` 写 0；越界/丢失时 `Lattice::GetExternalField` 清零 | 确定 |
| 0x048–0x058 | double[3] | extB | 外磁场 Bx,By,Bz（T） | `Quadrupole::GetField`：`Bx=G·y`、`By=G·x`；`Solenoid::GetField`：`Bz=B` | 确定 |
| 0x060–0x070 | double[3] | scE | 空间电荷电场 V/m | `PIC::importSCF`：`k·Δx/r³`，k=8.98755e9（1/4πε₀） | 确定 |
| 0x078–0x088 | double[3] | scB | 空间电荷磁场 T（实现中恒写 0） | `PIC::importSCF` | 确定 |
| 0x090–0x0a0 | double[3] | totE | 推进用总电场 | `LeapFrog::Update` 读取；`MethodMP::SeTRun` 把 extE 拷进来 | 确定 |
| 0x0a8–0x0b8 | double[3] | totB | 推进用总磁场 | 同上 | 确定 |
| 0x0c0–0x0e8 | double[6] | rel | 相对同步粒子的坐标：x−xs, γβx, y−ys, γβy, z−zs, γβz（动量是绝对值）；弯转段按 `Beam.rotAngle` 旋转 | `UpdateBeamParameters`、`TcodeTrans`（`rotMode` 1/2 的两种旋转） | 确定 |
| 0x0f0 | int | elemIndex | 当前所在元件下标（向前/向后线性搜索维护） | `Lattice::GetExternalField`："Warning! particle.indexElem<0" | 确定 |
| 0x0f4 | int | geomIndex | 当前所在 boundary.txt 几何段下标 | `MethodMP::tcodeRun` / `SeTRun`：用它索引 `Boundary.geoms` 再调 `Geometry::Boundary` | 确定 |
| 0x0f8 | int | unk_0f8 | 未见任何访问 | — | 推测（可能是填充） |
| 0x0fc | int | spOutCount | 已穿过的 superposeout 块数；索引 `Boundary.spOutMap` | 构造时置 0；`tcodeRun` 中 `++`，并按它取 `Boundary+0x60` 的行 | 推测 |
| 0x100 | int | lost | 0 存活；1 丢失（孔径、纵向限）；2 越过 lattice 末端 | `Drift::GetField`/`Field::GetField` 置 1；`LongitudinalBunchLoss` 置 1；`Lattice::GetExternalField` 置 2；全部代码只出现 0/1/2 | 确定 |
| 0x104 | int | nextPlane | 下一个输出面序号 | `SeTRun`：穿过 `planes[nextPlane]` 后插值输出并 `++` | 推测（只在二次粒子路径里见到） |
| 0x108 | double | spOutZ0 | 最近穿过的 superposeout 出口面 z | `SeTRun`：坐标变换后写入该块出口 z；孔径判丢失要求 `z > spOutZ0` | 推测 |
| 0x110 | double | spOutZ1 | 下一个 superposeout 出口面 z | `SeTRun`/`tcodeRun`：写入 `Lattice.spOut[i+1][6]`；判丢失要求 `元件 zStart+1e-6 < spOutZ1` | 推测 |
| 0x118 | double | charge | 宏粒子电荷 = N_real·w·Q·e（C）；同步粒子为 Q·e | `InitParticle`：`realPerMacro·weight·Q·1.60217653e-19` | 确定 |
| 0x120 | double | mass | 宏粒子质量 = N_real·w·A·m_p（kg） | `InitParticle`：`…·1.67262171e-27` | 确定 |
| 0x128 | double | qOverM | 荷质比 Q·e/(A·m_p)（C/kg） | `InitParticle`；`LeapFrog::Update` 用 `qOverM/c·dt/2·E` 更新 γβ、`qOverM·dt/γ·B` 作旋转角，正是 Boris 推进的两项 | 确定 |
| 0x130 | double | restMassMeV | 静止能量 mc²（MeV） | `InitParticle`；`dstOut` 用 `(γ−1)·它` 输出 W | 确定 |
| 0x138 | double | t | 粒子自身时间（s） | `InitParticle`（二次粒子）读 SeParticle.txt 的 time 列；`SeTRun` 每步 `t += dt` | 确定 |
| 0x140 | double | dt | 粒子自身时间步（s）= V2/(βc) | `InitParticle`（二次粒子）：`(Beam.seStep/β)/c`；`SeTRun` 用它推进 | 确定 |
| 0x148–0x168 | double[5] | unk_148 | 未见访问 | 全部 214 个函数中无此范围的读写 | 推测 |
| 0x170 | int | id | 粒子编号 | `InitParticle`（二次粒子）写循环下标；`SePlaneOut` 输出的第一列 | 确定 |
| 0x174 | int | spOutIndex | 二次粒子的下一个 superposeout 变换序号 | `SeTRun`：与 `Lattice.spOutCount` 比较，变换后 `++` | 推测 |
| 0x178 | int | boundaryMode | = input.txt `boundary`；0 时用元件孔径判丢失 | `InitParticle` 写 `Initial+0x144`；`Drift::GetField`/`Field::GetField` 只在它为 0 时判孔径 | 确定 |
| 0x180 | double | weight | 权重，默认 1.0；`indweight 1` 时从文件读 | `InitParticle`："Particle weight loaded."；`UpdateBeamParameters` 用作统计权重 | 确定 |
| 0x188 | std::string | kind | 粒子种类名（kindofparticle） | `InitParticle` 对每个粒子 `assign(Initial+0xe8)` | 确定 |

补充：`FullParticleOutput` 每行依次输出 `lost, charge, x, γβx, y, γβy, z, γβz`；`dstOut` 写 .dst 时换算为
x(cm)、x'=γβx/γβz、y(cm)、y'、φ=2πf(t−t_sync)(rad)、W=(γ−1)mc²(MeV)。

## 3. SynParticle（同步粒子求相位用，0xb8）

只在 `Initial::SetSynPhase` 和 `Initial::singlePartM` 中作为局部对象使用，与 Beam 里第 N 个 Particle 是两回事。

| 偏移 | 类型 | 建议字段名 | 含义与单位 | 证据 | 把握 |
|---|---|---|---|---|---|
| 0x00 | vector\<double\>(6) | coord | [x, γβx, y, γβy, z, γβz] | 构造函数分配 6 个；`SynParticle::Update`、`SetUpSynParticle` 写 `[5]=γβ` | 确定 |
| 0x18 | vector\<double\>(3) | E | 外电场 | `Update` 用它加速；元件 `GetField(…,SynParticle&,…)` 赋值 | 确定 |
| 0x30 | vector\<double\>(3) | B | 外磁场 | `Update`：B 全为 0 时跳过旋转 | 确定 |
| 0x48 | vector\<double\> | cavPhase | 每个射频腔的 RF 相位（deg） | `Field::GetField(…,SynParticle&,…)`、`RFQ::GetSynField`：`ωt − ω·cavTime[k] + cavPhase[k]·π/180` | 确定 |
| 0x60 | vector\<double\> | cavTime | 每个射频腔的参考时间（s） | 同上 | 确定 |
| 0x78 | int | elemIndex | 当前元件下标 | `SynParticle::GetField(double, Lattice&)` 线性搜索维护 | 确定 |
| 0x7c | int | lost | 超出孔径置 1 | `Field::GetField(…,SynParticle&,…)` | 确定 |
| 0x80 | int | fullPush | 0 = 只做纵向推进（横向清零） | `SynParticle::Update` 首个分支 | 确定 |
| 0x88 | double | charge | Q·e（C） | `SetUpSynParticle` | 确定 |
| 0x90 | double | mass | A·m_p（kg） | 同上 | 确定 |
| 0x98 | double | qOverM | C/kg | 同上；`Update` 使用 | 确定 |
| 0xa0 | double | restMassMeV | MeV | 同上（读 beam.txt / .dst） | 确定 |
| 0xa8 | double | EzAmp | 当前位置轴向电场幅值（未乘 cos 相位因子） | `Field::GetField(…Syn…)` 写插值后的 Ez 和；`RFQ::GetSynField` 写幅值 | 推测 |
| 0xb0 | double | frequency | 束流频率 Hz | `SetUpSynParticle`：beam.txt `frequency`(MHz)×1e6 | 确定 |

## 4. Beam（0xe8）

| 偏移 | 类型 | 建议字段名 | 含义与单位 | 证据 | 把握 |
|---|---|---|---|---|---|
| 0x00 | vector\<double\>(6) | centroid | 权重平均 [x, γβx, y, γβy, z, γβz] | `UpdateBeamParameters`；`~Beam` 释放 | 确定 |
| 0x18 | vector\<double\>(3) | rms | rms x、rms y、rms(z)·γ（m） | `UpdateBeamParameters`：z 偏差除以 sqrt(1−β²) | 确定 |
| 0x30 | vector\<double\>(6) | syncCoord | 同步粒子坐标副本 | `tcodeRun` 把 `particles[N][0..5]` 拷入 | 确定 |
| 0x48 | Particle* | particles | N+1 个宏粒子（new[]），[N] 为同步粒子 | `LeapFrog::Update`、`InitParticle` | 确定 |
| 0x50 | double | frequency | 束流频率 Hz | `InitParticle`：MHz×1e6；`dstOut` 写 `/1e6` | 确定 |
| 0x58 | int | n | 宏粒子数 N | 全部粒子循环的上界 | 确定 |
| 0x60 | double | current | 流强 A（mA/1000） | `InitParticle` | 确定 |
| 0x68 | int | nAlive | 存活宏粒子数 | `UpdateBeamParameters` 计数；`simulationProgress` 打印（进度行倒数第二个数） | 确定 |
| 0x6c | int | scSolverRecord | = input.txt `scsolverrecord` | `InitParticle` | 确定 |
| 0x70 | double | betaGammaLambda | (c/f)·βγ（m） | `UpdateBeamParameters`、`tcodeRun` | 确定 |
| 0x78 | double | gamma | 平均 γ | `UpdateBeamParameters` | 确定 |
| 0x80 | double | bunchTime | 束团时间长度 =(φmax−φmin)/(2πf)，默认 3e-9 s | `InitParticle`；`main_agent` 初值 | 确定 |
| 0x88 | double | tMin | φmin/(2πf)：初始分布最早时间 | `InitParticle`；同步粒子初始 z = βc·tMin | 确定 |
| 0x90 | double | zMin | 存活粒子最小 z，初值 1e10 | `UpdateBeamParameters`、`tcodeRun` | 确定 |
| 0x98 | double | zMax | 最大 z，初值 −1e10 | 同上 | 确定 |
| 0xa0 | double | rotAngle | 弯转段参考坐标系旋转角（rad） | `TcodeTrans`、`PIC::Allocate`：`sincos(rotAngle)` | 确定 |
| 0xa8 | int | rotMode | 0 不旋转；1、2 两种旋转平面（x–z 或 y–z） | `TcodeTrans`：1 旋转 y/z、2 旋转 x/z | 确定（编号与 HV 的对应推测） |
| 0xac | int | unk_ac | `main_agent` 初值 0 | — | 推测 |
| 0xb0 | double | seStep | 二次粒子步长参数 steppercycle V2（m，默认 0.01） | `InitParticle` 写 `Initial+0x210`；二次粒子 `dt = seStep/(βc)` | 确定 |
| 0xb8 | int | unk_b8 | 初值 0 | — | 推测 |
| 0xbc | int | useWeightFile | indweight 读权重文件时为 1；同时决定 `dstOut` 是否写 `outData_*`（权重） | `InitParticle`、`dstOut` | 确定 |
| 0xc0 | int | fromDstFile | 从 .dst 读初始分布时为 1 | `InitParticle`；`dstOut` 分支 | 确定 |
| 0xc8 | double | totalWeight | Σweight（无权重文件时 = N） | `InitParticle`、`UpdateBeamParameters` | 确定 |
| 0xd0 | double | realPerMacro | 每个宏粒子代表的真实粒子数 = I/f/Σw/e/Q | `InitParticle`："The number of macroparticle representative particles is" | 确定 |
| 0xd8 | int | recScField | = beam.txt `recscfield` | `InitParticle` | 确定 |
| 0xdc | int | impScField | = beam.txt `impscfield` | `InitParticle`；`tcodeRun` 为 1 时 `PIC::importSCF` | 确定 |
| 0xe0 | double | time | 当前模拟时间 s | `tcodeRun` 每步写入；`PIC::recordSCF` 读 | 确定 |

`~Beam` 只释放 0x00/0x18/0x30 三个 vector，**不释放 `particles`**（在别处 `delete[]`，未追踪）。

## 5. Element 体系

### 5.1 vtable（7 个槽，无虚析构）

从 `.so` 的 `R_X86_64_64` 重定位直接读出（`_ZTV7Element` @0x2ebe00，0x48 字节 = offset-to-top + typeinfo + 7 槽）。
调用点验证：`Lattice::GetExternalField` 调 `vptr+0x10`（GetField(Particle)）、`SeTRun` 有误差时调 `vptr+0x08`、
`SynParticle::GetField` 调 `vptr+0x00`、`Lattice::SetElement` 调 `vptr+0x28`。

| 槽偏移 | 函数 | 覆盖情况 |
|---|---|---|
| +0x00 | `GetField(double t, SynParticle&, FieldMatrix const&)` | 全部子类覆盖 |
| +0x08 | `GetErrField(double t, Particle&, FieldMatrix const&)` | RFQ 用基类空实现 |
| +0x10 | `GetField(double t, Particle&, FieldMatrix const&)` | 全部子类覆盖 |
| +0x18 | `GetParameter(std::vector<double>&)` | RFQ 用基类 |
| +0x20 | `SetErrPara(ErrorSample const&)` | RFQ、Steerer 用基类（不支持误差） |
| +0x28 | `SetParameter(int index, double zStart, std::vector<std::string> const&)` | RFQ 用基类（RFQ 另有非虚 `SetParameter(double, vector<string>)`） |
| +0x30 | `getRadius(double z)` | 只有 RFQ 覆盖（按 cell 返回孔径） |

vtable 地址（Ghidra 镜像基址 0x100000）：Element 0x3ebe10、Bend 0x3ebe70、Drift 0x3ebef0、Edge 0x3ebf70、Field 0x3ebff0、
Quadrupole 0x3ec830、RFQ 0x3ec8b0、Solenoid 0x3ec930、Steerer 0x3ec9b0。

### 5.2 Element 基类（0x88，Drift 不加成员）

各子类构造函数都内联了同一段基类构造：`+0x08..+0x18` 清零、`+0x40 = "drift"`、`+0x58/+0x60/+0x68/+0x6c/+0x70..+0x80` 清零。
`SetParameter` 的公共部分是 `Element::SetBasicParameter(zStart, words)`。

| 偏移 | 类型 | 建议字段名 | 含义与单位 | 证据 | 把握 |
|---|---|---|---|---|---|
| 0x00 | ElementVtbl* | vptr | | 构造函数 | 确定 |
| 0x08 | vector\<string\> | words | lattice 行原始单词 | `SetBasicParameter`：`words = param` | 确定 |
| 0x20 | int | index | 元件序号（= 当时 `elements.size()`），也是 ErrorSample 行号−1 | 各 `SetParameter`；`SetErrPara` 取 `err[index+1]` | 确定 |
| 0x28 | double | radius | 孔径半径 R（word2） | `SetBasicParameter`；`getRadius` 返回；`Drift/Field::GetField` 判丢失 | 确定 |
| 0x30 | double | radius2 | 同时写为 R | `SetBasicParameter` | 推测（可能是 Ry） |
| 0x38 | int | apertureType | 写 0 | `SetBasicParameter` | 推测 |
| 0x40 | std::string | typeName | 关键字（word0） | `SetBasicParameter`：`assign(word0)`；`SetSynPhase` 按它比较 | 确定 |
| 0x48 | double | zStart | 元件入口 z（m） | `SetBasicParameter(param_1)`，`Lattice::SetElement` 传入累加 z | 确定 |
| 0x50 | double | length | L（word1，m） | `SetBasicParameter` | 确定 |
| 0x58 | double | strength | 主场强：Quadrupole G（word4，T/m）、Solenoid B（word4，T）；Bend/Edge 的 SetErrPara 也按 dG 缩放它 | `Quadrupole::SetParameter`（G=0 报错）、`Solenoid::SetParameter`、各 `SetErrPara` | 确定（Bend/Edge 的赋值处未找到） |
| 0x60 | double | unk_60 | 构造清零 | — | 推测 |
| 0x68 | int | inSuperposeOut | 非 0 表示元件位于 superposeout 偏移坐标系中 | `SeTRun`：为 0 时才按 z 换元件；非 0 时走坐标变换分支 | 推测 |
| 0x6c | int | unk_6c | 运行中被清 0 的标志 | `tcodeRun` | 推测 |
| 0x70 | vector\<?\> | unk_70 | `~Element` 释放 | — | 推测 |

### 5.3 子类成员

lattice 单词序号与 `avas/data/schema.py` 一致：word0 关键字、word1 L、word2 R、word3 保留/V3。

**Quadrupole（0xf8）**

| 偏移 | 类型 | 字段 | 含义 | 证据 | 把握 |
|---|---|---|---|---|---|
| 0x58 | double | (base.strength) | 梯度 G T/m（word4） | `SetParameter` | 确定 |
| 0x88 | double | unk_88 | 构造为 0 | 构造函数 | 推测 |
| 0x90 | double | c90 | 构造为 −4e-5 | 构造函数；未找到读取处 | 推测（含义不明） |
| 0x98 | double | c98 | 构造为 4.518219 | 同上 | 推测（含义不明） |
| 0xa0 | double | aperDiam | 2R | `SetParameter` | 确定 |
| 0xa8–0xb8 | double[3] | unk_a8 | 构造为 0 | — | 推测 |
| 0xc0 | double | gradErr | G·(1+dG%) | `SetErrPara`；`GetParameter` 输出 | 确定 |
| 0xc8/0xd0/0xd8 | double | dx, dy, dz | 错位 m（mm×1e-3） | `SetErrPara`（err 列 1、2、7） | 确定 |
| 0xe0/0xe8/0xf0 | double | rotX, rotY, rotZ | 转角 rad（err 列 3、4、5，deg） | 同上 | 确定 |

`GetField(Particle)`：Bx = G·y，By = G·x，其余 0（硬边、无错位）；错位只在 `GetErrField` / z-code 矩阵里用。

**Solenoid（0xc0）**：0x58 B（word4）；0x88 B·(1+dG%)；0x90/0x98/0xa0 dx/dy/dz；0xa8/0xb0/0xb8 rotX/Y/Z。
证据 `Solenoid::SetParameter`、`SetErrPara`、`GetParameter`（输出顺序 dx,dy,dz,rx,ry,rz,Berr）。`GetField(Particle)` 只给 Bz = B。全部确定。

**Steerer（0xa8）**：0x88 int kind（word6，0 磁 / 1 电）；0x90 Bx/Ex（word4）；0x98 By/Ey（word5）；0xa0 最大值（word7）。
证据 `Steerer::SetParameter`。`GetField(Particle)` 恒为 0：校正铁只在 z-code（`HardEdgeTrans`）里作用。确定。

**Bend（0x108）**

| 偏移 | 类型 | 字段 | 含义 | 证据 | 把握 |
|---|---|---|---|---|---|
| 0x88 | int | hv | 0 水平 / 1 竖直（word7） | `SetParameter` | 确定 |
| 0x90 | double | arcLength | ρ·|α| | `SetParameter` | 确定 |
| 0x98 | double | alpha | 偏转角 rad（word4，deg） | 同上 | 确定 |
| 0xa0 | double | kx | sqrt((1−n)·h²) | 同上 | 确定 |
| 0xa8 | double | ky | sqrt(n·h²) | 同上 | 确定 |
| 0xb0 | double | rho | ρ（word5） | 同上 | 确定 |
| 0xb8 | double | n | 场梯度指数（word6） | 同上 | 确定 |
| 0xc0 | double | h | sign(α)/ρ | 同上 | 确定 |
| 0xc8 | double | unk_c8 | `GetParameter` 会输出，`SetParameter` 不写 | `Bend::GetParameter` | 推测 |
| 0xd0 | double | fieldErr | strength·(1+dG%) | `SetErrPara` | 确定 |
| 0xd8–0x100 | double×6 | dx,dy,dz,rotX,rotY,rotZ | 同 Quadrupole | `SetErrPara` | 确定 |

**Edge（0xf8）**：0x88 int hv（word9）；0x90 β 极面角 rad（word4）；0x98 K1（word7）；0xa0 K2（word8）；0xa8 |ρ|（word5）；0xb0 间隙 G（word6）；
0xb8 ψ = K1·G/ρ·(1+sin²β)/cosβ·(1 − K1·K2·G·tanβ/ρ)（TRANSPORT 的边缘场修正角）；0xc0 fieldErr；0xc8–0xf0 dx,dy,dz,rotX,rotY,rotZ。
证据 `Edge::SetParameter`、`SetErrPara`。全部确定。

**Field（场图元件，0x1f8）**

`InitLattice` 会先改写 field 行再交给 `SetParameter`：word9（场图名）换成 `fieldMapPath+名字`，`Lattice::SetElement` 调 `SetMatrix` 载入后
再把 word9 换成场图序号；频率非 0 时补齐到 14 个单词并追加 word14 = 射频腔序号；V3（word3）为 0 时置 `Initial.needSynPhase = 1`，**V3 本身不存进 Field**。

| 偏移 | 类型 | 字段 | 含义 | 证据 | 把握 |
|---|---|---|---|---|---|
| 0x088 | int | mapIndex | FieldMatrix 中的场图序号（word9） | `SetParameter`；`GetField` 用它查网格范围 | 确定 |
| 0x08c | int | fieldType | word4：1 RF，4 叠加组；2/3 应为静电/静磁 | `SetParameter`（=1 时频率不得为 0；=4 解析子场） | 1、4 确定；2、3 推测 |
| 0x090 | double | ke | word7 | `SetParameter`；`GetField` 电场 ×Ke×1e6 | 确定 |
| 0x098 | double | kb | word8 | 同上 | 确定 |
| 0x0a0 | double | zEnd | zStart+L | `SetParameter` | 确定 |
| 0x0a8 | double | frequency | Hz（word5） | `SetParameter`；`GetField` 中 2π·f | 确定 |
| 0x0b0–0x0d8 | double×6 | errDx,errDy,errDz,errRotX,errRotY,errRotZ | 错位（m）与转角（rad）；腔误差没有 rotZ | `Field::SetErrPara` | 确定 |
| 0x0e0 | double | keErr | Ke·(1+dK%) | 同上 | 确定 |
| 0x0e8 | double | kbErr | Kb·(1+dK%) | 同上 | 确定 |
| 0x0f0 | double | phaseErr | 腔相位误差（err 列 6，直接相加） | 同上 | 推测（单位按输入，应为 deg） |
| 0x0f8 | vector\<double\>(3) | zeroField | {0,0,0}，粒子在网格外时拷给 E、B | 构造函数；`GetField` | 确定 |
| 0x110 | double | phase | 相位 rad（word6，deg） | `SetParameter` | 确定 |
| 0x118 | int | cavIndex | 射频腔序号（word14），索引 cavPhase/cavTime | `SetParameter`；`GetField` | 确定 |
| 0x120 | vector\<double\> | subZStart | 叠加子场入口 z | `SetParameter`（type 4） | 确定 |
| 0x138 | vector\<int\> | subType | 子场类型 | 同上 | 确定 |
| 0x150 | vector\<int\> | subMapIndex | 子场场图序号 | 同上 | 确定 |
| 0x168 / 0x180 | vector\<double\> | subKe / subKb | | 同上 | 确定 |
| 0x198 | vector\<vector\<double\>\> | subPlace | 每个子场 {3 个位移, 3 个角度(rad)}；`GetField(Syn)` 用 [3]、[4]、[5] 作 sincos | 同上 | 推测（各分量顺序） |
| 0x1b0 / 0x1c8 | vector\<double\> | subRadius / subLength | 子场的 R、L（取自子块的 word2、word1） | 同上 | 推测 |
| 0x1e0 | vector\<vector\<double\>\> | subErr | 子场误差参数 | `SetErrPara`、`GetErrField` | 推测 |

**RFQ（0x178）**：0x88 vector\<vector\<double\>\> 每 cell 的场展开系数（推测）；0xa0 cell 长度（cm/100）；0xb8 cell 起点 z；0xd0 cell 终点 z（`getEleNum` 按它查 cell）；
0xe8 cell 孔径（cm/100，`getRadius` 返回）；0x100 读入值 ×1000（推测 kV→V）；0x118、0x130 两个用途不明的 vector；0x148 int 射频腔序号（word14）；
0x150 频率 Hz（word5）；0x158 vector{0,0,0}；0x170 = 1e-16。证据 `RFQ::RFQ`、`RFQ::SetParameter`、`getEleNum`、`getRadius`、`GetSynField`。

## 6. FieldMatrix（场图库，0x1c0，嵌在 Lattice+0x18）

`Lattice::SetMatrix(name, type, FieldMatrix&)` 按名字查重，未载入则读文件、push 进各数组并返回序号。

| 偏移 | 类型 | 字段 | 含义 | 证据 | 把握 |
|---|---|---|---|---|---|
| 0x000 | vector\<vector\<vector\<double\>\>\> | maps | [图][节点] → {Ex,Ey,Ez,Bx,By,Bz}，节点数 (nx+1)(ny+1)(nz+1) | `SetMatrix`；`Field::GetField(Syn)` 按 [0..5] 取并乘 Ke·1e6 / Kb | 确定 |
| 0x018 | vector\<string\> | names | 已载入的场图名 | `SetMatrix` 开头 `std::find` | 确定 |
| 0x030 | vector\<double\> | cavPhase | 每个射频腔 RF 相位（deg） | `Field::GetField(Particle)`：`ω(t−cavTime[k]) + cavPhase[k]·π/180` | 确定 |
| 0x048 | vector\<double\> | cavTime | 每个射频腔参考时间（s） | 同上 | 确定 |
| 0x060 | int | nMaps | 已载入数 | `SetMatrix` `++` | 确定 |
| 0x068/0x080/0x098 | vector\<int\> | nx, ny, nz | 网格数 | `SetMatrix` | 确定 |
| 0x0b0/0x0c8/0x0e0/0x0f8 | vector\<double\> | xmin, xmax, ymin, ymax | 横向范围 m | `SetMatrix`；`Field::GetField` 越界判断 | 确定 |
| 0x110 | vector\<double\> | zlen | 纵向长度 | 同上 | 确定 |
| 0x128/0x140/0x158 | vector\<double\> | dx, dy, dz | 步长 | 同上；插值 | 确定 |
| 0x170–0x1b8 | int×3 + double×8 | cur* | 读单个文件时的临时值（nx,ny,nz,xmin,xmax,ymin,ymax,zlen,dx,dy,dz），随后 push 进上面的数组 | `SetMatrix`；一维场展开时设 x,y∈[−1,1]、nx=ny=2 | 确定 |

## 7. Lattice（0x268）

| 偏移 | 类型 | 字段 | 含义 | 证据 | 把握 |
|---|---|---|---|---|---|
| 0x000 | vector\<Element*\> | elements | 元件指针（`~Lattice` 只释放数组，不删元件） | `SetElement`、`GetExternalField` | 确定 |
| 0x018 | FieldMatrix | fm | 场图库 | `SetElement` 传 `this+0x18`；`GetExternalField` 把 `this+0x18` 传给 GetField | 确定 |
| 0x1d8 | vector\<double\> | elemZEnd | 元件出口 z；`end[-1]` = lattice 总长 | `SetElement`；`GetExternalField`（z ≥ zEnd 时 index++） | 确定 |
| 0x1f0 | vector\<double\> | elemZStart | 元件入口 z | 同上（z < zStart 时 index−−） | 确定 |
| 0x208 | int | spOutCur | 主束当前 superposeout 变换序号 | `GetExternalField`：越过 `spOut[cur][6]` 的粒子场清零（等其余粒子到齐）；`tcodeRun` `++` | 推测 |
| 0x210 | vector\<vector\<double\>\> | spOut | 每个 superposeout 块：{z0, x0, y0, θz, θx, θy, 出口 z}（[6] 为出口 z） | `SeTRun` 坐标变换用 [0..5]，出口判断用 [6] | 推测（[0..5] 顺序按 schema 的 superpose 参数） |
| 0x228 | int | spOutCount | superposeout 块数 | `SeTRun`：与 `Particle.spOutIndex` 比较 | 推测 |
| 0x22c | int | unk_22c | 构造清零 | — | 推测 |
| 0x230 | vector\<double\> | elemLength | 元件长度（word1） | `SetElement` | 确定 |
| 0x248 | int | lastMapIndex | 最近一次 `SetMatrix` 返回值 | `SetElement` | 确定 |
| 0x250 | vector\<?\> | unk_250 | `~Lattice` 释放 | — | 推测 |

## 8. Geometry / Boundary（boundary.txt）

**Boundary（0x78）**：0x00 vector\<Geometry*\>；0x18 vector\<double\> 段终点 z；0x30 段起点 z（前一段起点+长度）；0x48 段长度（word2）；
0x60 vector\<vector\<double\>\>，按 `Particle.spOutCount` 取行 {z 偏移, 首几何段, 末几何段}（推测）。证据 `Boundary::SetElement`、`~Boundary`、`tcodeRun`。

**Geometry（0x100）**：0x00 vector\<string\> 原始单词；0x18 vector\<double\>(6) 3 个位移 + 3 个角度（后三个 deg→rad，推测）；0x30 起点 z；0x38 int 序号；
0x40 vector\<double\> 形状参数；0x58 int 形状类型（word0，只接受 0、1、2，否则 "Unknown geometric shapes"）；0x60 string（word1，推测为材料/名称，
相关字符串有 "RLP"、"Copper"）；0x68、0x6c int（word4、word13）；0x88 vector\<vector\<double\>\>、0xb8 vector\<int\>、0xd0/0xe8 vector\<double\> 等。
除前几项外含义均为推测。证据 `Geometry::Geometry`、`Geometry::SetParameter`。

## 9. ErrorSample / ErrorParameter

**ErrorSample（0x48）**

| 偏移 | 类型 | 字段 | 含义 | 证据 | 把握 |
|---|---|---|---|---|---|
| 0x00 | vector\<int\> | flags | [0] 束流静态、[1] 束流动态、[2] 元件静态、[3] 元件动态 是否启用 | `errSampleOut`：[0]或[2] 写 StaticErrorData.txt、[1]或[3] 写 DynamicErrorData.txt；`Quadrupole::SetErrPara` 只看 [3]；`tcodeRun` 用 [2]+[3]==0 判无元件误差 | 确定 |
| 0x18 | vector\<vector\<double\>\> | dyn | 行 0 = 束流误差，行 i+1 = 元件 i | `errSampleOut`（Dynamic）；`*::SetErrPara` 取 `[index+1]`；`InitParticle` 取行 0 | 确定 |
| 0x30 | vector\<vector\<double\>\> | stat | 同上，静态 | `errSampleOut`（Static）；`SeTRun` 按 flags 选择 0x18 或 0x30 | 确定 |

元件误差行的列：静磁元件 [1]dx(mm) [2]dy(mm) [3]θx(°) [4]θy(°) [5]θz(°) [6]dG(%) [7]dz(mm)；射频腔 [1]dx [2]dy [3]θx [4]θy [5]dK(%) [6]φ [7]dz；
束流行 [1]dx(mm) [2]dy(mm) [3]dφ(°) [4]dx'(mrad) [5]dy'(mrad) [6]dE(MeV) … [13]dIb(mA)。[0] 列含义未确认（推测为误差分布类型或标记）。与 schema.py 的 QUAD/CAV/BEAM_ERRORS 一致。

**ErrorParameter（≥0xc8）**：只从析构函数和 `InitLattice` 恢复了容器：0x00 vector（POD）、0x18 与 0x30 两个 vector\<vector\<vector\<double\>\>\>、
0x48 vector\<double\>（某 err_* 关键字的数值，首元素补 0）、0x60 vector\<vector\<double\>\>（每条误差定义）、0x78 vector\<int\>（补足到 30 或 60 个，推测为误差开关）、
0x90 vector\<vector\<double\>\>、0xa8 int、0xb0 vector。具体对应哪个关键字需要再读 `GenerateErrorSet`（263 KB），本次未做，全部标推测。

## 10. Mesh / PIC（只列关键成员）

**Mesh（0x140）**：0x00 vector\<int\> {Nx,Ny,Nz}；0x18 int 粒子数；0x20 vector\<vector\<int\>\> 每个粒子所在格点（3 个 int）；0x38/0x50/0x68 vector\<double\> x/y/z 网格坐标；
0x80、0x98、0xb0、0xc8、0xe0、0xf8、0x110、0x128 共 8 个 vector\<vector\<vector\<double\>\>\> 三维数组（电荷密度、势、场分量等，逐个对应关系未确认）。证据 `Mesh::Initial`、`~Mesh`。

**PIC（≥0x530）**：0x000–0x167 内嵌 `progschj::ThreadPool`（`~PIC` 调 `ThreadPool::~ThreadPool`）；0x170 double 初值 1.0；0x1b8/0x1bc/0x1c0 Nx,Ny,Nz；0x1c4–0x1cc N−1；
0x1d0–0x1d8 N/2；0x1e4 int = SC 参数[9]；0x240–0x250 meshrms 三个系数；0x258 double scmethod（0 = FFT，非 0 = SPICNIC；`tcodeRun` 据此选 `Weight+FFT`）；
0x2b0 vector\<int\>(3)；0x4d0–0x508 八个 `fftw_malloc` 缓冲；0x510–0x528 四个 `fftw_plan`。证据 `PIC::PIC`、`PIC::Initial`、`~PIC`。

## 11. Initial（参数读取，0x220）

`Initial::Read` 读 input.txt、`Initial::ReadBeam` 读 beam.txt；关键字字面量从 `.rodata` 恢复。

| 偏移 | 类型 | 字段 | 关键字 / 含义 | 把握 |
|---|---|---|---|---|
| 0x000 | int | secondaryBeam | secondarybeam | 确定 |
| 0x004 | int | singleParticleMode | singleparticlemode | 确定 |
| 0x008 | string | distFile | readparticledistribution（默认 "unknown"） | 确定 |
| 0x010 | double | charge | numofcharge（e） | 确定 |
| 0x018 | double | atomicMass | mc²/938.271875（InitParticle 计算） | 确定 |
| 0x020 | vector\<string\> | distribution | distribution 横向、纵向（默认 GS GS） | 确定 |
| 0x038 | int | particleNumber | particlenumber | 确定 |
| 0x040 | vector\<double\>(9) | twiss | [εx,βx,αx, εy,βy,αy, εz,βz,αz]（行内顺序 α β ε 反存）；beamtype dc 时 [6] 置哨兵 −2027.0415 | 确定 |
| 0x058 / 0x070 | vector\<double\>(3) | displacePos / displacedPos | displacepos / displacedpos | 确定 |
| 0x088 | vector\<int\> | cores | setnumberofcores（10 个，超过 OpenMP 线程数则截断，并拷到 scPar[10..]） | 确定 |
| 0x0a0 | double | restMass | particlerestmass（默认 938.272029 MeV） | 确定 |
| 0x0a8 | double | current | current（mA；InitParticle 后改为 A） | 确定 |
| 0x0b0 | double | frequency | frequency（MHz；InitParticle 后改为 Hz） | 确定 |
| 0x0b8 | double | kineticEnergy | kneticenergy（MeV；InitParticle 后为平均能量） | 确定 |
| 0x0c8 | double | randomSeed | randomseed（两个文件都可写，存为 double） | 确定 |
| 0x0d0 | vector\<double\>(5) | longLimits | longlimits | 确定 |
| 0x0e8 | string | kind | kindofparticle（默认 "proton"） | 确定 |
| 0x0f0 | vector\<double\>(2) | beamLength | beamlength | 确定 |
| 0x108 | string | method | method（默认 "MP"） | 确定 |
| 0x110 | int | spaceCharge | spacecharge（默认 1；流强 0 或二次束时清 0） | 确定 |
| 0x118 | vector\<double\>(20) | scPar | [0] scmethod（FFT=0 / SPICNIC=1）、[1..3] meshrms、[4..6] integralgrid、[7] scutoffdistance、[8] negligibleratio、[10..] cores | 确定 |
| 0x138 | double | scanTStep | scantstep（默认 0.03） | 确定 |
| 0x144 | int | boundary | boundary | 确定 |
| 0x148 / 0x14c / 0x150 / 0x154 | int | lpBoundary / probe / indWeight / indCharge | 同名关键字 | 确定 |
| 0x158 | vector\<double\>(6) | pcHistogram | pchistogram | 确定 |
| 0x170 | double | stepPerCycle | steppercycle V1（默认 50） | 确定 |
| 0x178 | int | dumpPeriodicity | dumpperiodicity | 确定 |
| 0x180 | int | runPeriod | runperiod（默认 100000） | 确定 |
| 0x188 | vector\<int\> | track | track | 确定 |
| 0x1a8 | vector\<double\> | outputPlanes | 输出面 z 列表（拷到 MethodMP+0x220，进度分母用最后一个） | 推测 |
| 0x1c0 | int | needSynPhase | 存在 V3=0 的射频腔时置 1 | 确定 |
| 0x1c4 | int | scSolverRecord | scsolverrecord | 确定 |
| 0x1c8 | vector\<int\>(3) | numOfGrid | numofgrid | 确定 |
| 0x1e4 | int | bunchNumber | bunchnumber | 确定 |
| 0x1e8 / 0x1ec / 0x1f0 | int | multithreading / maxThreads / ompMaxThreads | multithreading（默认 1）/ maxthreads / `omp_get_max_threads()` | 确定 |
| 0x210 | double | seStep | steppercycle V2（默认 0.01） | 确定 |
| 0x218 / 0x21c | int | recScField / impScField | recscfield / impscfield | 确定 |

其余（0x03c、0x0c0、0x130、0x140、0x17c、0x1a0、0x1e0、0x1f8）只见到默认值，含义推测，见头文件。
注意 input.txt 里的 `dimension` 会直接抛错（"input.txt Contains unknown keywords: Dimension"）。

## 12. MethodMP（关键成员）

0x000–0x167 是内嵌的 `progschj::ThreadPool`（workers vector、任务 deque、最大队列 100000、mutex、三个 condition_variable、进行中任务计数 0x160）。

| 偏移 | 类型 | 字段 | 含义 | 证据 | 把握 |
|---|---|---|---|---|---|
| 0x168 | int64 | startClock | `system_clock::now()` | `simulationProgress` 算运行时间 | 确定 |
| 0x198 | double | dt | 1/(f·steppercycle) s | `InitMethodMP` | 确定 |
| 0x1a0 | int | spaceCharge | | `InitMethodMP`；`dstOut` 为 1 时写流强 | 确定 |
| 0x1a8 | vector\<int\> | track | | `InitMethodMP` | 确定 |
| 0x1c0 / 0x1c4 / 0x1cc / 0x1d0 | int | multithreading / dumpPeriodicity / boundary / lpBoundary | | `InitMethodMP`；`SeTRun` 用 0x1cc 选几何边界 | 确定 |
| 0x1d8 / 0x1f0 | vector\<double\> | pcHistogram / longLimits | | `InitMethodMP` | 确定 |
| 0x208 | vector\<int\> | cores | | `InitMethodMP` | 确定 |
| 0x220 | vector\<double\> | outputPlanes | 最后一个元素作进度分母 | `simulationProgress` | 推测 |
| 0x238 / 0x250 | vector | scPar / numOfGrid | | `InitMethodMP`；`PIC::PIC` 收到 `this+0x238` | 确定 |
| 0x268 / 0x270 / 0x278 / 0x280 | double | progressPercent / runTime / remainingTime / zStartRun | "Simulate progress …" 一行的各量 | `simulationProgress` | 确定 |
| 0x298 | int | maxSecondaryCount | `SeTRun` 记录的最大值 | `SeTRun` | 推测 |

## 13. 仍未弄清的关键点

1. Particle 0x148–0x16f（5 个 double）和 0xf8 在全部伪代码中都没有访问，可能是废弃成员。
2. superposeout 相关的一组字段（Particle 0xfc/0x108/0x110/0x174、Lattice 0x208/0x210/0x228、Boundary 0x60、Element 0x68）按用法串起来是一致的，但
   `spOut` 行内 [0..5] 的顺序没有逐项核对。
3. Quadrupole 构造时写入的常数 −4e-5、4.518219（0x90、0x98）以及 0x88/0xa8–0xb8 没找到读取处；Bend 0xc8、Element 0x58 对 Bend/Edge 的赋值处未找到。
4. Field 的 fieldType 2/3 分别是静电/静磁（按 schema 推测）；叠加组子场的 subPlace 分量顺序、subErr 的列含义未核对。
5. Beam 0xa8 rotMode 的 1/2 与 bend 的 HV 如何对应未核对；Beam 的 sizeof 是 0xe8 还是 0xf0 无法从现有证据区分。
6. ErrorParameter 各容器对应的关键字、Mesh 的 8 个三维数组各是什么、Geometry 大部分成员，需要分别细读 `GenerateErrorSet`、`PIC::*`、`Geometry::Boundary`。
7. `Beam.particles` 由谁 `delete[]` 未追踪（`~Beam` 不释放）。
