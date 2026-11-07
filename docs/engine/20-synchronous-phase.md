# 20 同步粒子与 RF 相位确定（SetSynPhase / singlePartM / SynParticle）

来源：`decomp/libAVAS/functions/` 伪代码，关键常数、字符串、寄存器与分支用 `objdump -d libAVAS.so` 反汇编逐条核对
（Ghidra 在这两个大函数里丢了大部分字符串字面量，`compare` 的对象都是从反汇编的 `lea rsi, [rip+…]` 恢复的）。
Windows 版对照 `decomp/AVAS_dll/functions/180039f20_FUN_180039f20.c`（SetSynPhase）、`180042860_FUN_180042860.c`（singlePartM）、
`18008c6b0_FUN_18008c6b0.c`（SetUpSynParticle）。字段名按 [structures.md](structures.md)。
下文 "word k" 指 lattice 行按空格拆分后的第 k 个词（word0 关键字、word1 L、word2 R、word3 V3、word4 type、word5 f、word6 phase、
word7 Ke、word8 Kb、word9 场图、word14 由 InitLattice 追加的射频腔序号）。

---

## 1. 概述

### 1.1 做什么

内核在跟踪束团之前，用一个**单独的参考粒子**（`SynParticle`，不是 `Beam.particles[N]`）沿整条 lattice 走两遍：

1. **扫描阶段**（只在 lattice 里至少有一个 V3 = 0 的射频场图元件时执行）：对每个 V3 = 0 的腔，在 21 个入口相位上
   跟踪同步粒子、按积分算 φs，夹出目标 φs 后做 6 次割线（regula falsi）迭代，得到入口相位 φ0；
   V3 = 1、2 的腔和 RFQ 也在这一遍里登记入口时刻与相位。输出 `scanPhaseData.txt`、`Phase.txt`。
2. **主阶段**（总是执行）：用更小的步长再走一遍，记录每个元件入口的 z、t、γβ 和射频腔的 φs、φRF，输出 `synData.txt`，
   并把最终的"每腔 RF 相位 `cavPhase[k]`（deg）与参考时刻 `cavTime[k]`（s）"写回 `Lattice.fm`，
   之后束团跟踪的 `Field::GetField(t, Particle&)` 用 `E·cos(2πf·(t − cavTime[k]) + cavPhase[k]·π/180)`。

单粒子模式（input.txt `singleparticlemode 1`）不跟踪束团，只调用 `Initial::singlePartM`，它是 SetSynPhase 的复制版本（差异见 §2.9）。

### 1.2 在运行流程中的位置

```
main_agent
 ├─ Initial::Read / ReadBeam …
 ├─ singleParticleMode == 1:  InitLattice → Initial::singlePartM(lattice) → "Simulation ends."
 └─ 否则: InitLattice → InitParticle(beam,…) → Initial::SetSynPhase(lattice, beam.particles[N].gbz)
           → InitMethodMP → MethodMP::Run → tcodeRun → 每步 TcodeTrans（束团推进，§2.11）
```

SetSynPhase 的第二个参数是 InitParticle 之后 `Beam.particles[N]`（同步粒子槽位）的 γβz。

### 1.3 入口函数与调用顺序（SetSynPhase）

```
if (Initial.needSynPhase == 1):            # InitLattice 发现 RF field 且 V3==0 时置 1
    cout "Start scan phase...."
    SynParticle S;  S.SetUpSynParticle(...)          # §2.3
    清空 scanPhaseData.txt、Phase.txt，再以追加方式打开
    扫描阶段循环（§2.6）→ 把 S.cavPhase/S.cavTime push_back 到 Lattice.fm
else:
    对每个 typeName=="field" 且 word5≠0 的元件，按元件顺序 fm.cavPhase.push_back(word6)   # 不写 cavTime
SynParticle P;  P.SetUpSynParticle(...)               # 第二次初始化，重新读 beam.txt
主阶段循环（§2.7）→ 写 synData.txt；把 P.cavPhase/P.cavTime 按下标覆盖/追加到 Lattice.fm
Lattice.spOutCur = 0；return 0
```

---

## 2. 规格正文

### 2.1 常数（内核实际使用的值）

| 常数 | 值 | 用途 |
|---|---|---|
| c | 299792458.0 | 推进 |
| e | 1.60217653e-19 | 电荷 |
| m_p | 1.67262171e-27 | 质量单位 |
| 质子 mc² 基准 | 938.271875 | A = mc²/938.271875 |
| 默认 mc² | 938.272029 | beam.txt 无 particlerestmass 时 |
| π、2π、π/2 | 3.141592653589793、6.283185307179586、1.5707963267948966 | |
| 扫描步长系数 | `Initial.scanTStep`（input.txt `scantstep`，默认 0.03） | 扫描阶段 dt |
| 主阶段步长系数 | 0.01（SetSynPhase）/ 0.1（singlePartM），写死 | 主阶段 dt |
| 扫描网格 | φ0 = −180 + 18·i，i = 0…20（21 点，两端是同一物理相位） | V3=0 |
| 割线迭代次数 | 6，写死 | V3=0 |
| 进入元件的 z 偏移 | +1e-15 | 插值到元件入口 / 扫描时出口 |
| 离开腔的 z 偏移 | +1e-13 | 主阶段腔出口、迭代与最终跟踪出口 |
| 判失败阈值 | 1.0°（与目标差）、±90° 邻域 1.0° | "Abnormal settings…" |
| -33.0 | gbzIn 哨兵 | SetUpSynParticle |

### 2.2 SynParticle 对象

构造函数：`coord` = 6 个 0；`E`、`B` = 3 个 0；`cavPhase`、`cavTime` 为空；`elemIndex = lost = fullPush = 0`。
其余标量（charge、mass、qOverM、restMassMeV、EzAmp、frequency）不初始化，由 SetUpSynParticle 写入。
析构只释放 5 个 vector。

`fullPush`（+0x80）语义：0 = 只做纵向推进且把 x、γβx、y、γβy 清零（同步粒子绝大部分时间如此）；
SetSynPhase 在"当前元件 `Element.inSuperposeOut != 0`"的静态场步里置 1，越过该 superposeout 块出口后置回 0（§2.6.6）。

### 2.3 SynParticle::SetUpSynParticle

真实签名（由 mangled 名 `SetUpSynParticleESsddSsid` 和调用点寄存器确定）：

```
void SetUpSynParticle(std::string distFile, double Q, double A_unused,
                      std::string inputDir, int secondaryBeam, double gbzIn)
```

调用方传：`distFile = Initial.distFile`（readparticledistribution，默认 "unknown"）、`Q = Initial.charge`（numofcharge）、
`A_unused = Initial.atomicMass`（**函数内完全没用**，xmm1 未保存）、`inputDir = 全局 inputFilePath`、
`secondaryBeam = Initial.secondaryBeam`、`gbzIn`：SetSynPhase 传 `Beam.particles[N].gbz`，singlePartM 传常数 −33.0。

先 `cout << "Start initialising synchronised particles" << endl`。然后三个分支：

**分支 A：`distFile == "unknown"` 或 `secondaryBeam != 0`**

- 打开 `inputDir + "beam.txt"`；打不开：`cout << "File \"beam.txt\" not opened!" << endl; exit(1)`。
- 整个文件读成字符串后做换行规范化（`NormalizeLineEndings`：先把每个 "\r\n" 换成 "\n"，再把剩余 "\r" 换成 "\n"）。
- 逐行 `getline`，用 `StringSplit` 分词，第一个词转小写后比较：
  - `kneticenergy` → W = stod(word1)（MeV）
  - `particlerestmass` → mc² = stod(word1)（MeV，初值 938.272029）
  - `frequency` → `this.frequency = stod(word1)`，**原样保存，不乘 1e6**（beam.txt 的 frequency 是 Hz，示例 162500000.0）
  - 其余行忽略；同一关键字出现多次时最后一次有效；stod 失败抛 `std::invalid_argument("stod")`/`out_of_range("stod")`。
  - 若没有 kneticenergy 行，W 未初始化（未定义行为）。
- γ = W/mc² + 1；γβ = γ·sqrt(1 − 1/γ²)（先算 `1 − 1/(γ·γ)` 再开方）。**此分支忽略 gbzIn。**

**分支 B：distFile 的扩展名（最后一个 '.' 之后）== "dst"**

- distFile 中没有 '.' → 抛 std::string 异常 `"The initial bunch distribution file cannot be opened, check particleDistributionPath"`。
- 以二进制打开 `inputDir + distFile`；失败或 fail/bad → `cout << "Cannot read initial distribution" << endl; exit(1)`。
- 按 TraceWin dst 读：char[2]、int32 Np、double Ib、double f(MHz)、char[1]、Np×6 double（每行 x, x', y, y', φ, W）、double mc²。
- `frequency = f × 1e6`；W̄ = (ΣW_i)/Np（按文件顺序累加）；γ = W̄/mc² + 1；γβ̄ = γ·sqrt(1 − 1/γ²)。
- **γβz = gbzIn；只有 gbzIn == −33.0 时才用 γβ̄。** 质量、荷质比用 dst 末尾的 mc²。

**分支 C：扩展名 == "edst"**

- 与 A 相同地读 beam.txt（W、mc²、frequency 原样），γβ 由 W 算出，但 **γβz = gbzIn，除非 gbzIn == −33.0**。

**扩展名两者都不是**：什么都不写（coord 仍全 0，q/m 未初始化），之后的跟踪无法前进（推测为死循环或 NaN）。

公共收尾（A/B/C）：
```
coord = [0, 0, 0, 0, 0(z), γβz]
restMassMeV = mc²;  EzAmp = 0;  elemIndex = 0
charge = Q·1.60217653e-19
mass   = (mc²/938.271875)·1.67262171e-27
qOverM = ((Q/(mc²/938.271875))·1.60217653e-19)/1.67262171e-27     # 按此运算顺序
E = {0,0,0};  B = {0,0,0}
```
同步粒子的起点总是 z = 0、t = 0（lattice 入口）。

### 2.4 SynParticle::GetField(double t, Lattice&)（Ghidra 中 param_2 是 this）

```
z = coord[4]
if 0 <= z <= L.spOut[L.spOutCur][6] and z < L.elemZEnd.back():
    i = elemIndex
    if z < elemZStart[i]:  do i-- while z < elemZStart[i]          # 向后找
    elif elemZEnd[i] <= z: do i++ while elemZEnd[i] <= z           # 向前找
    elemIndex = i
    elements[i]->GetField(t, *this, L.fm)        # vtable 槽 0
else:
    E = {0,0,0}; B = {0,0,0}
```
`Lattice.spOut` 至少有一行：InitLattice 在没有 superposeout（或最后一块出口 < lattice 总长）时追加哨兵行 `{0,0,0,0,0,0,L_total}`。

各元件的 `GetField(t, SynParticle&, fm)`（子系统 C）要点：矩阵元件（quad、solenoid、bend、edge、steerer）与 drift 只给零场；
drift 在 sqrt(x²+y²) ≥ R 时打印 `Synchronised particles can't reach the exit plane, problems with the beamline` 并 `exit(1)`
（x、y 只在 fullPush=1 时可能非零）；`Field` 用 `S.cavPhase[k]`、`S.cavTime[k]`（同步粒子自己的列表，不是 fm 的）给出
`E·cos(2πf(t − cavTime[k]) + cavPhase[k]·π/180)`，并把**未乘 cos 因子的轴向场幅值 Σ Ez·Ke·1e6** 写入 `EzAmp`（+0xa8）；
RFQ 走 `RFQ::GetField(t, SynParticle&, fm)`（§2.10）。`lost`（+0x7c）会被 Field 置位，但 SetSynPhase 从不检查它。

### 2.5 SynParticle::Update(double dt)

记 u = (γβx, γβy, γβz)，k = qOverM。三种情况：

**(a) fullPush == 0（纵向推进）**
```
x = γβx = y = γβy = 0
γβz = ((k·Ez)/c)·dt + γβz
γ   = sqrt(γβz² + 1)
z   = ((c/γ)·γβz)·dt + z
```
只用 E[2]，忽略 Ex、Ey 与全部磁场。

**(b) fullPush != 0 且 Bx == 0 且 By == 0**（注意：**不检查 Bz**，Bz 被忽略）
```
u += ((k·E)/c)·dt               # 三个分量逐个
γ = sqrt(γβx² + 1 + γβy² + γβz²)  # 求和顺序：((γβx²+1)+γβy²)+γβz²
x += (γβx·(c/γ))·dt;  y += (γβy·(c/γ))·dt;  z += ((c/γ)·γβz)·dt
```

**(c) 其余：Boris**
```
u⁻ = u + ((k·E)/c)·dt·0.5
γ⁻ = sqrt(u⁻x² + 1 + u⁻y² + u⁻z²)
t  = (B·k·dt)/γ⁻                     # 三分量，注意此处没有 1/2
s  = 1/((tx²+ty²+tz²)·0.25 + 1)
u⁺x = s·[ ((tx²−ty²−tz²)·0.25+1)·u⁻x + (tx·ty·0.5 + tz)·u⁻y + (tx·tz·0.5 − ty)·u⁻z ]
u⁺y = s·[ (tx·ty·0.5 − tz)·u⁻x + ((ty²−tz²−tx²)·0.25+1)·u⁻y + (ty·tz·0.5 + tx)·u⁻z ]
u⁺z = s·[ (tx·tz·0.5 + ty)·u⁻x + (ty·tz·0.5 − tx)·u⁻y + ((tz²−ty²−tx²)·0.25+1)·u⁻z ]
u  = u⁺ + ((k·E)/c)·dt·0.5
γ  = sqrt(γβx² + 1 + γβy² + γβz²);  位置同 (b)
```
即 τ = t/2 的标准 Boris 旋转（Cayley 形式），正电荷 du/dt ∝ u × B。伪代码里每一项都写成"先乘 s 再相加"，
`(…)·u·s` 的结合顺序见 `001d4490_SynParticle_Update.c`，要逐位复现时照抄。

### 2.6 SetSynPhase：扫描阶段（needSynPhase == 1）

#### 2.6.1 准备

- 步长 `dt_s = (scanTStep / S.frequency) / stepPerCycle`（stepPerCycle = input.txt steppercycle V1，默认 50；
  frequency 来自 §2.3，分支 A/C 为 beam.txt 的 Hz 值）。默认 0.03/(f·50)，即每 RF 周期约 1667 步。
- 最大步数 `Nmax = stepPerCycle·runPeriod / scanTStep`（runperiod 默认 100000）；计数器 `cnt` 每处理一个"步或一个腔"加 1。
- `ofstream(outputFilePath+"scanPhaseData.txt", trunc)`、`ofstream(…"Phase.txt", trunc)` 各开一次立即关闭（清空），
  再各用 `fstream(out|app)` 打开，记为 SPD、PH。
- `t = 0`；`prev = [z, γβz, 0, 0, 0, 0]`（6 个 double 缓冲：z、γβz、x、γβx、y、γβy）。
- 若 `Nmax <= 0` 跳过整个循环。

#### 2.6.2 主循环（每轮）

```
loop:
  z = S.coord[4]
  if 0 <= z <= spOut[spOutCur][6] and z < L_total:
      按 §2.4 的规则只更新 S.elemIndex（不插值、不输出）
  e = elements[S.elemIndex]
  if e.typeName ∉ {"field", "rfq"}:                        # 普通元件
      S.GetField(t); prev[0:2] = (z, γβz); S.Update(dt_s); t += dt_s
  else:
      f = stod(word5)                                       # 词数 < 6 时抛 out_of_range
      if f == 0:  静态场步（§2.6.6，含 superposeout 处理）
      elif e.typeName == "field":
          V3 = stod(word3)
          if V3 <= 0.5:  V3=0 扫描（§2.6.3）
          else:          V3=1 / V3=2 登记（§2.6.4）
      else:            RFQ（§2.6.5）
  # 终止判断
  if S.coord[4] > L_total: break
  cnt += 1; if cnt >= Nmax: break
```

#### 2.6.3 V3 = 0 的腔：扫描、选区间、割线迭代

设 k = stoi(word14)（射频腔序号）、f = word5、T = word6（目标 φs，deg）。

**(1) 插值到入口**：若 `S.z != prev[0]`：`r = (zStart[e] − prev[0])/(S.z − prev[0])`；
`S.z = (S.z − prev[0])·r + prev[0] + 1e-15`；`S.γβz = (S.γβz − prev[1])·r + prev[1]`；`t = r·dt_s + (t − dt_s)`。
记 `z_in = S.z`、`gbz_in = S.γβz`、`t_in = t`。

**(2) 登记**：`S.cavTime.push_back(t_in)`；`S.cavPhase.push_back(−180.0)`（占位；之后按下标 k 改写，假定 push 的位置恰好是 k）。

**(3) Phase.txt 入口行**：`PH << (k+1) << "  " << fixed << setprecision(13) << t_in << "  " << z_in << "  " << W_in << endl`，
其中 `W_in = (sqrt(gbz_in² + 1) − 1)·mc²`。

**(4) 21 点扫描**：对 φ0 = −180, −162, …, +180（`φ0 += 18` 共 21 次）：
```
S.coord = [0,0,0,0,z_in,gbz_in];  S.cavPhase[k] = φ0;  τ = t_in
Sc = Ss = 0
loop:
    S.GetField(τ);  zp = S.z; gp = S.γβz
    S.Update(dt_s)
    Ez = S.EzAmp                       # GetField 在旧位置 zp 写入的幅值
    if S.z > zEnd[e]: break
    (sn, cs) = sincos(φ0/180·π + f·2π·(τ − t_in))     # 用推进前的时刻 τ
    τ += dt_s
    Sc = cs·Ez·(S.z − zp) + Sc
    Ss = Ez·sn·(S.z − zp) + Ss
# 出口
r  = (zEnd[e] − zp)/(S.z − zp)
S.z = (S.z − zp)·r + zp + 1e-15;  S.γβz = (S.γβz − gp)·r + gp
(sn, cs) = sincos(φ0/180·π + f·2π·(τ − t_in))         # τ 仍是越界那一步开始的时刻
dz = S.z − zp                                          # 含 1e-15
φs = atan( (Ss + r·dz·(Ez·sn)) / (Sc + (cs·Ez)·dz·r) ) / π · 180
```
注意末段被乘了两次比例（`r·dz` 而 dz 本身已是 `r·Δz`），且相位取的是步首时刻；这是原内核行为，必须照做。
**φs 用 atan（不是 atan2），取值 (−90°, 90°)**；加速与减速（相差 180°）的解给出相同的 φs。
每点记录 `Φ0[i] = φ0`、`Φs[i] = φs`、`G[i] = S.γβz(出口)`，并写
`SPD << fixed << setprecision(10) << φ0 << " " << φs << " " << W_in << " " << W_out << endl`（W_out 由出口 γβz 算）。
扫描内部**没有**反向运动检查，粒子若在腔内折返会一直跑下去（无步数上限）。

**(5) 选两个候选**（在 Φ0 上至少相隔 90° 的两支）：
```
b1 = b2 = 1e10;  i1 = i2 = -1
for i in 0..20:
    d = |Φs[i] − T|
    if d >= b2: continue
    if d < b1:
        if i1 != -1 and |Φ0[i]−Φ0[i1]| >= 90 and ||Φ0[i]−Φ0[i1]| − 360| >= 0.5:
            i2 = i1; b2 = b1
        i1 = i; b1 = d
    else:                                  # b1 <= d < b2
        if |Φ0[i]−Φ0[i1]| >= 90 and ||Φ0[i]−Φ0[i1]| − 360| >= 0.5:
            i2 = i; b2 = d
```
端点修正（让 i±1 存在）：对 i1：若 i1 == 0 且 Φs[0] > T → i1 = 20；若 i1 == 20 且 Φs[20] < T → i1 = 0。i2 同样处理。
（i2 仍为 −1 时后面会越界读，未定义。）

**(6) 选夹逼区间** `(a0, a1) = (Φ0[lo], Φ0[hi])`、`(s0, s1) = (Φs[lo], Φs[hi])`，hi = lo+1：

- **T ≥ 0**：`c = (G[i1] <= gbz_in) ? i2 : i1`（i1 不加速就换 i2，**不检查 i2 是否加速、不打印提示**）；
  若 `T <= Φs[c]` 取 (c−1, c)，否则取 (c, c+1)。
- **T < 0**：
  1. 若 `T < Φs[i1]` 且 `G[i1] − G[i1−1] > 0` → 区间 (i1−1, i1)；否则若 `T > Φs[i1]` 且 `G[i1+1] − G[i1] > 0` → 区间 (i1, i1+1)；
     得到区间后：若 `G[i1] >= gbz_in` 就用它；否则转第 3 步。
  2. 以上都不成立（T == Φs[i1]，或 G 在该方向不递增）：改用 i2：`T < Φs[i2]` 且 `G[i2+1] − G[i2] > 0` → (i2, i2+1)；
     `T > Φs[i2]` 且 `G[i2] − G[i2−1] > 0` → (i2−1, i2)；都不成立 → 抛 `"Scan phase error, unexpected situation, check code"`。
  3. （i1 的区间是减速解）若 `G[i2] > gbz_in`，按第 2 步的两个条件尝试 i2 的区间，成功就用 i2；
     否则（i2 也不加速或条件不满足）保留 i1 的区间并 `cout << "Note that the energy of the particle decreases after it passes through the cavity" << endl`。
- 任何分支得到区间后：若 `a0 == 0.0 且 a1 == 0.0` → 抛 "Scan phase error, unexpected situation, check code"（哨兵检查）。

"G 在 φ0 增大方向递增"选出的是 dW/dφ0 > 0 的一支；结合 atan 的 180° 简并，内核在 T < 0 时优先取加速解，
找不到才接受相差 180° 的减速解并打印上述提示（与 AGENTS.md 第 6 节"0.7 MeV 时取相差 180° 的解"的实测一致）。

**(7) 割线（regula falsi）**：结果 p = (φ0*, φs*)。
```
if s0 == T: p = (a0, s0)            # 不迭代、不写行
elif s1 == T: p = (a1, s1)
else:
    if |s0 − T| > 90:  s0 −= 180
    elif |s1 − T| > 90: s1 = a1 + 180          # 原内核写的是 a1（相位!）+180，不是 s1+180；照抄
    repeat 6 times:
        φ0n = ((T − s0)·(a1 − a0))/(s1 − s0) + a0
        按 (4) 的方法从入口状态跟踪一次，但出口 z 偏移用 +1e-13，得到 φsn、W_out
        p = (φ0n, φsn)
        if |φsn − T| <= 90:
            if φsn > T: (a1, s1) = (φ0n, φsn)
            elif φsn < T: (a0, s0) = (φ0n, φsn)
            else: 写 SPD 行，结束迭代
        else:
            if φsn < T: (a1, s1) = (φ0n, φsn + 180)
            if φsn > T: (a0, s0) = (φ0n, φsn − 180)
        写 SPD 行：φ0n φsn W_in W_out（fixed、精度 10、" " 分隔）
```
没有收敛容差判断，固定 6 次（恰好命中才提前退出）；最终 p 是最后一次的点，不是区间里最好的点。

**(8) 收尾与报错**：
- `SPD << "3333 3333 3333 3333" << endl`（每个 V3=0 腔一段的分隔行）。
- `cout << "The " << (k+1) << " field, phi_0:" << φ0* << "    phi_s:" << T << "  Frequency is " << f << endl`。
- **若 `|φs* − T| > 1` 且 `|φs* + 90| > 1` 且 `|φs* − 90| > 1`**，抛 std::string：
  `"Abnormal settings of the initial energy or the phase parameters of the lattice cause the scan phase to fail to find the corresponding RF phase"`。
  main_agent 捕获后由 `errorOut` 写 ErrorLog.txt，前缀 `"   Initialisation stage error: "`（子系统 A/D）。
  也就是说"无解"判据是：6 次割线后 φs 离目标仍超过 1°，且没有贴近 atan 的饱和值 ±90°。
- `S.cavPhase[k] = φ0*`；从入口状态（x、γβx、y、γβy 置 0，z_in、gbz_in，t = t_in）再跟踪整个腔：
  打印 `"Entry  time " << t << "  Entering energy " << W_in`；循环 {GetField(t)；prev；Update(dt_s)；t += dt_s；
  若 z < zStart 抛 `"Synchronous particle reverse motion, program stopped"`；z == zStart 继续}，直到 z > zEnd；
  插值 `r = (zEnd − zp)/(z − zp)`，`z = (z−zp)·r + zp + 1e-13`，`t = r·dt_s + (t − dt_s)`，γβz 线性插值；
  打印 `"Export time " << t << "    Export energy " << W`；`S.elemIndex += 1`（不钳位）；
  Phase.txt 出口行：`(k+1) "  " t "  " z "  " W`（格式同入口行）。

#### 2.6.4 V3 = 1 / V3 = 2 的腔（扫描阶段）

- 若 word5 == 0 跳过（不会发生，外层已判过）。读 word14（结果不用）。
- 插值到入口（同 2.6.3(1)，+1e-15），`S.cavTime.push_back(t_in)`。
- **V3 > 1.5（V3=2）**：`φ_in = word6 − f·t_in·360`，再规约到 [−180, 180]（小于 −180 加 360、大于 180 减 360，循环），push 进 `S.cavPhase`。
- **否则（V3=1）**：`S.cavPhase.push_back(word6)`。
- 然后 x、γβx、y、γβy 置 0，在腔内一直推进到 z > zEnd（反向检查同 2.6.3(8)），插值到 zEnd + 1e-13、t 修正，
  `S.elemIndex += 1`，若超过 `元件数 − 1` 则钳为 `元件数 − 1`。不写 Phase.txt、不写 SPD。

**符号问题（已用反汇编 0x5ca17–0x5ca38 核实，DLL 相同）**：扫描阶段 V3=2 用的是 `word6 − 360·f·t_in`，
而主阶段（§2.7.3）用 `word6 + 360·f·t_in`。束团跟踪最终用的是主阶段的值（正确地等于"t=0 时的绝对相位"）。
扫描阶段里 V3=2 腔对同步粒子的作用相位因此是 `2πf·t + word6 − 4πf·t_in`，与主阶段不同；
若 V3=2 腔位于某个 V3=0 腔上游，这个 V3=0 腔的入口时刻和能量在扫描阶段与主阶段不一致，synData 里该腔的 φs 会偏离目标。
新内核默认必须复现此行为；对齐清单 B-17 用于确认。

#### 2.6.5 RFQ（扫描阶段，typeName == "rfq" 且 word5 ≠ 0）

```
插值到入口（+1e-15）；保存入口 (z, γβz)
S.cavTime.push_back(t_in)
φ0 = ((π/2 − Initial.frequency·2π·t_in)/π)·180          # 用束流频率（Hz），不是 RFQ 的 word5
S.cavPhase.push_back(φ0)
cout << "RFQ Complete phase scanning, phi_0:" << φ0 << "  frequency is " << word5 << endl
S.GetField(t)          # → RFQ::GetField(Syn)：内部把粒子推过整个 RFQ（§2.10）
if S.coord[1] == 1.0:  # RFQ::GetField 发现逐 cell 能量下降时置的标志
    S.coord[1] = 0; 恢复入口 z、γβz; S.cavTime.back() = t_in
    φ0 = ((−π/2 − Initial.frequency·2π·t_in)/π)·180; S.cavPhase.back() = φ0
    cout << "RFQ Rescan phase,phi_0:" << φ0 << "  frequency is " << word5 << endl
    S.GetField(t)
    if S.coord[1] == 1.0: 抛 "RFQ scanning phase abnormality"
t = S.coord[0]; S.coord[0] = 0          # RFQ::GetField 把出口时刻放在 coord[0]
```

#### 2.6.6 静态场步与 superposeout（word5 == 0 的 field/rfq）

```
S.GetField(t)
if e.inSuperposeOut != 0: S.fullPush = 1
prev = [z, γβz, x, γβx, y, γβy]
S.Update(dt_s)
j = L.spOutCur
if j < L.spOutCount and elements[S.elemIndex].inSuperposeOut != 0:
    S.fullPush = 1
    row = L.spOut[j]          # {z0, x0, y0, θa, θb, θc, z_exit}，θ 为 rad
    对当前点和 prev 点分别做：
        X = x − row[1];  Y = y − row[2];  Zl = (z − row[0]) − e.zStart
        X1 = cos(row[3])·X + sin(row[3])·Y;     Y1 = cos(row[3])·Y − sin(row[3])·X
        X2 = cos(row[5])·X1 − sin(row[5])·Zl;   Z2 = sin(row[5])·X1 + cos(row[5])·Zl
        Z3 = cos(row[4])·Z2 − sin(row[4])·Y1;   Y3 = cos(row[4])·Y1 + sin(row[4])·Z2
        动量 (γβx, γβy, γβz) 做同样的三次旋转（不减偏移）
    if Z3(当前) > 0:                         # 越过块出口面
        r = Z3p/(Z3p − Z3)
        S.x, S.γβx, S.y, S.γβy, S.γβz = 局部坐标在 prev→当前 之间按 r 线性插值
        S.z = row[6]
        t = (r·dt_s + t) − dt_s
        if row[6] == L_total: 结束扫描阶段（不再执行后面的 t += dt_s）
        L.spOutCur = j + 1; S.fullPush = 0
t += dt_s
```
（坐标变换与子系统 G 的 superposeout 规格一致时以 G 为准；此处只保证同步粒子路径照此实现。）

#### 2.6.7 扫描阶段结束

关闭 SPD、PH；`L.spOutCur = 0`；按下标 i = 0…|S.cavPhase|−1 依次 `fm.cavPhase.push_back(S.cavPhase[i])`、
`fm.cavTime.push_back(S.cavTime[i])`（fm 原本为空）。

needSynPhase == 0 时只做"else"分支：遍历元件，`typeName == "field"` 且 word5 ≠ 0 的按顺序 `fm.cavPhase.push_back(word6)`，
**不处理 RFQ、不写 cavTime、不清空也不写 scanPhaseData.txt / Phase.txt**（上次运行留下的这两个文件保持原样）。

### 2.7 SetSynPhase：主阶段（总是执行）

#### 2.7.1 准备

- 新建 SynParticle P，同 §2.3 再初始化一次（会再次打印 "Start initialising synchronised particles"、再次读 beam.txt）。
- `dt_m = (0.01 / P.frequency) / stepPerCycle`（singlePartM 为 0.1）。
- `P.cavPhase = fm.cavPhase`（逐个复制）；`P.cavTime` 为空。
- `synData.txt`：先以 `out|trunc` 打开再关闭，然后 `out|app` 打开，记为 SD。
- t = 0，`prev = [z, γβz, 0,0,0,0]`，写第一行行首（无换行）：
  `SD << 0 << " " << elements[0].typeName << " " << L_0 << " " << z << " " << t << " " << γβz`
  （SetSynPhase 这一行用流的默认精度 6；singlePartM 在长度之前就设了 setprecision(17)）。

#### 2.7.2 主循环

```
while P.z <= L_total:
    z = P.z
    if 0 <= z <= spOut[spOutCur][6] and z < L_total:
        向后找：只改 elemIndex，不输出
        向前找（elemZEnd[i] <= z）：
            i 前进到新元件；若 z != prev[0]：
                r = (zStart[i] − prev[0])/(z − prev[0])
                P.z = (z − prev[0])·r + prev[0] + 1e-15;  P.γβz 线性插值;  t = r·dt_m + (t − dt_m)
            SD << " invalid invalid" 换行（put('\n')+flush）
            SD << i << " " << typeName << " " << setprecision(17) << L_i << " " << P.z << " " << t << " " << P.γβz
    if P.z > L_total: break
    e = elements[P.elemIndex]
    if e.typeName == "field" and word5 == 0:   静态场步（同 §2.6.6，用 dt_m；row[6]==L_total 时跳到收尾 §2.7.4 之后的写回）
    elif e.typeName == "field" (word5 != 0):   射频腔（§2.7.3）
    else:                                      P.GetField(t); prev[0:2]=(z,γβz); P.Update(dt_m); t += dt_m
```
主阶段没有步数上限；反向运动只在射频腔内检查。typeName 为 "rfq" 时按普通元件处理（见 §5 问题 4）。

#### 2.7.3 主阶段射频腔

```
k = stoi(word14)；f = word5
若 P.z != prev[0]：再插值一次到 zStart（+1e-15，t、γβz 同样修正；刚在 §2.7.2 插值过时这次的 r ≈ 1 − 1e-15/Δz，照做）
V3 = stod(word3)
if V3 > 1.5: φ = word6 + f·t_in·360，规约到 [−180,180]（同 2.6.4 的循环）；P.cavPhase[k] = φ
if |P.cavTime| < |P.cavPhase|: P.cavTime.push_back(t_in)       # 不按 k 放置，按遇到的顺序追加
x = γβx = y = γβy = 0
Sc = Ss = 0
loop:
    P.GetField(t); zp = P.z; gp = P.γβz
    P.Update(dt_m)
    Ez = P.EzAmp
    t = t + dt_m                                  # 先推进时间
    if P.z <= zStart[e]:
        if P.z < zStart[e]: 抛 "Synchronous particle reverse motion, program stopped"
        continue                                  # z == zStart：不累加
    if P.z > zEnd[e]: break
    (sn, cs) = sincos(P.cavPhase[k]/180·π + f·2π·(t − P.cavTime[k]))   # 用推进后的时刻
    Sc = cs·Ez·(P.z − zp) + Sc
    Ss = Ez·sn·(P.z − zp) + Ss
r = (zEnd − zp)/(P.z − zp)
P.z = (P.z − zp)·r + zp + 1e-13;  P.γβz = (P.γβz − gp)·r + gp
(sn, cs) = sincos(P.cavPhase[k]/180·π + f·2π·(t − P.cavTime[k]))       # t = 越界那一步的步末时刻（尚未修正）
dz = P.z − zp
P.elemIndex += 1
t = dt_m·r + (t − dt_m)
φs = atan( (sn·Ez·dz·r + Ss) / (cs·Ez·dz·r + Sc) ) / π · 180
```
注意：主阶段积分用"推进后的时刻"、扫描阶段用"推进前的时刻"，两者相差一步的相位（≈ 2πf·dt）。
输出：
- 若 `P.elemIndex <= 元件数 − 1`：`SD << " " << setprecision(17) << φs << " " << P.cavPhase[k]` 换行，再写下一元件的行首（同 §2.7.2）。
- 否则（腔是最后一个元件）：`SD << " " << φs << " " << P.cavPhase[k] << endl`；再用旧的 prev（腔之前最后一步保存的值）
  插值到 L_total（无 +ε，r 与 t 修正同上）；写 `P.elemIndex(=元件数) << " exitPlane " << setprecision(17) << L_{n−1} << " " << z << " " << t << " " << γβz << " invalid invalid" << endl`；直接到写回（§2.7.4 后半）。

#### 2.7.4 收尾

正常结束（`P.z > L_total` 跳出）：若 `P.z != prev[0]`，`r = (L_total − prev[0])/(P.z − prev[0])`，z、γβz 插值（无 +ε），`t = r·dt_m + (t − dt_m)`；
`SD << " invalid invalid" << endl`；写 `elemIndex(=n−1) << " exitPlane " << L_{n−1} << " " << z << " " << t << " " << γβz << " invalid invalid" << endl`。
注意两种结束方式 exitPlane 行的序号不同（n−1 与 n）。

然后：关闭 SD；`L.spOutCur = 0`；`cout << "Exit bunch energy: " << (sqrt(γβz²+1) − 1)·mc² << "                       " << endl`（23 个空格）；
对 i = 0…|P.cavPhase|−1：`fm.cavPhase[i] = P.cavPhase[i]`（越界则 push_back），`fm.cavTime[i] = P.cavTime[i]`（同上；
P.cavTime 比 P.cavPhase 短时读越界，未定义）。

#### 2.7.5 各 V3 最终交给束团跟踪的相位

束团跟踪用 `cos(2πf·(t − cavTime[k]) + cavPhase[k]·π/180)`，t 以同步粒子在 z = 0 时为 0：

| V3 | cavTime[k] | cavPhase[k] | 物理含义 |
|---|---|---|---|
| 0 | 主阶段到达腔入口的时刻 | 扫描阶段求得的 φ0*（deg，未规约） | 同步粒子到达时 RF 相位为 φ0*，使 atan 积分 φs ≈ word6 |
| 1 | 主阶段到达时刻 | word6 | 到达时 RF 相位 = word6 |
| 2 | 主阶段到达时刻 | word6 + 360·f·t_in，规约到 [−180,180] | 等价于 `cos(2πf·t + word6)`：t = 0 时的绝对相位（与 AGENTS.md 第 6 节一致） |

### 2.8 输出文件格式

| 文件 | 何时写 | 每行内容 | 格式 |
|---|---|---|---|
| `synData.txt` | 每次 SetSynPhase / singlePartM 都重写 | `i 关键字 L z_in t_in γβz_in φs φRF`；非射频元件后两列为 `invalid invalid`；最后一行 `i exitPlane L_last z t γβz invalid invalid` | 单空格分隔；第一行在 SetSynPhase 中精度 6（`%g`），此后 `setprecision(17)` 非 fixed（≈`%.17g`）；singlePartM 全部 17 位 |
| `scanPhaseData.txt` | needSynPhase==1 时清空重写 | 每个 V3=0 腔：21 行扫描 + ≤6 行迭代：`φ0 φs W_in W_out`；然后 `3333 3333 3333 3333` | fixed、10 位小数、单空格 |
| `Phase.txt` | needSynPhase==1 时清空重写 | 每个 V3=0 腔两行（入口、出口）：`k+1  t  z  W` | 整数后两个空格；fixed、13 位小数、两空格分隔 |
| `synParticle.txt` | 由 `MethodMP::synPout`（子系统 D）在 tcodeRun 中追加；MethodMP::Run 开始时清空 | `YYYY/MM/DD HH:MM:SS  z  W  x  y  γβx  γβy  γβz  rotMode  rotAngle Ex  Ey  Ez  Bx  By  Bz  lost`（同步粒子 particles[N]，E/B 为 extE/extB） | 时间戳 `strftime("%Y/%m/%d %X  ")`；fixed、精度 12；多数两空格分隔，rotAngle 后是一个空格 |

synData 列含义：φs 为 §2.7.3 的 atan 积分（deg，(−90, 90)）；φRF 为腔入口时刻的 RF 相位 `P.cavPhase[k]`（deg）。
同步粒子取相差 180° 的减速解时 φs 列仍写成接近目标的值（AGENTS.md 实测 −29.92° 而 atan2 为 150°），这正是 atan 的结果。
`t` 单位 s，`z`、`L` 单位 m，第 6 列是 γβ（不是能量）。

终端输出（cout，默认精度 6）：`Start scan phase....`、`Start initialising synchronised particles`（每次 SetUpSynParticle）、
`The k field, phi_0:… phi_s:… Frequency is …`、`Entry  time …  Entering energy …`、`Export time …    Export energy …`、
`RFQ …`、`Note that the energy…`、`Exit bunch energy: …`。

### 2.9 Initial::singlePartM 与 SetSynPhase 的差异（逐行 diff 核实）

1. 开头先调用 `Initial::ReadBeam(this)`（单粒子模式不经过 InitParticle）。
2. 两次 SetUpSynParticle 的 gbzIn 都是 −33.0：.dst 分布用文件平均能量，.edst 与 beam.txt 用 kneticenergy。
3. 主阶段 `dt_m = 0.1/f/stepPerCycle`（SetSynPhase 为 0.01）。
4. synData.txt 第一行就设 precision 17。
5. 其余（扫描、选区间、迭代、报错、文件）完全相同。DLL 版 singlePartM（FUN_180042860）同样传 −33.0、用 0.1。

### 2.10 RFQ::GetSynField(double t, SynParticle&)

```
if S.z > rfq.zStart + rfq.L:  E = B = 0; return            # EzAmp 不变
n = getEleNum(S.z)                                         # cell 序号；−1 时把 E、B 清零但继续用 n=−1 计算（越界）
dz = S.z − cellStart[n];  κ = π / cellLen[n]
A = coef[n][1];  B3 = coef[n][6];  V = cellV[n]（RFQ+0x100，读入值×1000）
Ez0 = −|V|·0.5·( I0(0)·A·cos(κ·dz)·κ + cos(3κ·dz)·B3·3·κ ) · (−1)^n     # I0(0) = Bessin(0, κ·0.0) = 1
S.E = {0, 0, cos(f_rfq·2π·(t − S.cavTime[c]) + S.cavPhase[c]/180·π) · Ez0}
S.EzAmp = Ez0;  S.B = {0,0,0}
```
c = RFQ.cavIndex（word14），f_rfq = word5。系数含义（双项势展开 A10、A30）属推测，详见子系统 C。

调用者 `RFQ::GetField(t, SynParticle&, fm)`（不在本子系统清单，要点供理解 §2.6.5）：步长 `0.05/f_rfq/500`，逐 cell 推进并累加
φs（atan，cell 平均），每过一个 cell 比较 γβz，若比上一 cell 小则置 `S.coord[1] = 1.0` 返回；全部走完时打印、把出口时刻写入
`S.coord[0]`，并把逐 cell 的 φs 与 W 写入 `RFQsynPhaseEG.txt`（fixed、精度 10）。

### 2.11 TcodeTrans（束团 t-code 一步推进的线程工作函数）

被 `MethodMP::tcodeRun` 调用：先单独对同步粒子 `[N, N+1)` 调一次（thread = 0、param_21 = 0），再把宏粒子分块交给线程池，每块一次。
参数（按签名顺序）：`Lattice& L, Beam& B, double& zField, ErrorSample& err, vector<double>& planes, vv& planeHit, vv& spHit,
vv& prev, double t_end, double dt, vector<double>& sync, vector<int>& spCnt, vv& cnt, vv& maxAbs, vv& sum2, vv& cross,
vector<double>& zMin, vector<double>& zMax, vector<double>& sumZ2, vv& sum1, double g, int th, int i0, int i1`
（vv = vector<vector<double>>，下标 th 是线程号；名字按用法推测）。对每个粒子 i ∈ [i0, i1)，p = B.particles[i]：

1. **步前统计**（`p.lost != 1` 且 `i != B.n`），用上一步的 `p.rel` 与 `B.centroid`（c）：
   `d = [rel0−c0, (rel1−c1)/rel5, rel2−c2, (rel3−c3)/rel5, rel4−c4, (rel5−c5)/rel5]`，w = p.weight；
   `zMin[th] = min(zMin[th], p.z)`、`zMax[th] = max(…)`；`sumZ2[th] += g·d4·g·d4·w`；
   对 j = 0…5：`maxAbs[th][j] = max(…, |d_j|)`，`sum2[th][j] += d_j·d_j·w`；
   `cross[th] += {d1·d0·w, d3·d2·w, w·d4·d5}`。
2. **合成推进场**：若 `zField < B.zMin` 或 `zField < p.z`：`totE = extE + scE`、`totB = extB + scB`；否则 totE = totB = 0。
3. `prev[i] = p 的 6 个坐标`（推进前）。
4. 若 `p.lost != 1`：Boris 推进（与 §2.5(c) 同一算法，但系数写法为 `h = (qOverM/c)·dt·0.5`，`u += totE·h`；
   旋转参数 `t = totB·(dt·qOverM)/γ⁻`；第二个半踢 `+ totE·h` 直接并入旋转结果；位置 `x += γβx·(dt·(c/γ))`）；
   B = 0 时也走这条路径（旋转为单位阵），**不忽略 Bz**——这一点与 SynParticle::Update(b) 不同。
5. **相对坐标**（按 `B.rotMode`，s = sync，θ = B.rotAngle）：
   - 0：`rel = [x−s0, γβx, y−s2, γβy, z−s4, γβz]`
   - 2：`dx=x−s0, dz=z−s4`；`rel0 = cosθ·dx − sinθ·dz`，`rel4 = sinθ·dx + cosθ·dz`，`rel1 = γβx·cosθ − γβz·sinθ`，`rel5 = γβz·cosθ + γβx·sinθ`，y 不旋转
   - 1：`dy=y−s2`；`rel2 = cosθ·dy + sinθ·dz`，`rel4 = dz·cosθ − dy·sinθ`，`rel3 = γβy·cosθ + γβz·sinθ`，`rel5 = γβz·cosθ − γβy·sinθ`，x 不旋转
6. **取下一步外场**：若 `0 <= z <= L.spOut[L.spOutCur][6]` 且 `p.lost == 0`：按 §2.4 的线性搜索更新 `p.elemIndex`，
   无元件误差（`err.flags[2] + err.flags[3] == 0`）时调 `GetField(t_end, p, fm)`（vtable +0x10）；
   有误差时取 `rows = (err.flags[2]==1) ? err.stat : err.dyn`，`rows[elemIndex+1][0] == 1.0` 调 `GetErrField`（+0x08），否则 GetField。
   否则：若 `z > L_total` 且 `L.spOutCur == |L.spOut|` 且 `p.lost == 0` 则 `p.lost = 2`；把 extE、extB 清零。
7. **向后出口**：若 `rel5 < 0` 且 `L.spOutCount <= L.spOutCur` 且 `z < 0`：`p.lost = 1`。
8. **输出面**：`m = p.nextPlane`（+0x104）；若 `m <= |planes|−1` 且 `planes[m] <= z` 且 `planes[m] <= spOut[spOutCur][6]`：
   `r = (planes[m] − prev4)/(z − prev4)`，`planeHit[i] = [6 个坐标在 prev→当前 的插值, (r − 1)·dt + t_end, 1.0]`，`p.nextPlane++`。
9. **superposeout 穿越记录**：若 `L.spOutCur < L.spOutCount`、`elements[p.elemIndex].inSuperposeOut != 0`、`spHit[i][7] == 0`、
   `z > p.spOutZ0`（+0x108）：用与 §2.6.6 相同的三次旋转求局部 Z3；`Z3 > 0` 时 `r = Z3p/(Z3p − Z3)`，
   `spHit[i] = [x2, γβx, y3, γβy, 局部 z, γβz（均为插值）, (r−1)·dt + t_end, 1.0]`，`spCnt[th]++`。粒子本身不在此处变换（由 tcodeRun 处理）。
10. **步后累计**（`p.lost != 1` 且 `i != B.n`）：`cnt[th] += {1, w}`；`sum1[th][j] += p.rel[j]·w`。

（这一函数的统计量最终如何合成 DataSet.txt 属子系统 D；本节只保证单步行为。）

### 2.12 monotonicSearch 与 findClosest（PIC::importSCF 的辅助函数）

两者只被 `PIC::importSCF`（impscfield = 1 时导入记录的空间电荷场，子系统 F）调用。

`monotonicSearch(const vector<double>& xs, const vector<double>& qs, vector<SearchResult>& out)`：
`SearchResult = {int lo; int hi; double xlo; double xhi;}`（24 字节）。清空 out；`xs` 或 `qs` 为空直接返回。
游标 `j` 从 0 开始，**跨查询保留**：对每个 q（按 qs 顺序），从当前 j 向后找第一个满足 `xs[j] < q <= xs[j+1]` 的 j，
找到则输出 `{j, j+1, xs[j], xs[j+1]}`（j 不变）；找不到则输出 `{−1, −1, 0, 0}`，且 j 已走到 |xs|−1，之后所有查询都找不到。
隐含要求 xs、qs 都升序。

`findClosest(const vector<double>& v, double x)` → int：v 为空抛 `std::runtime_error("Vector is empty!")`；|v| = 1 返回 0；
否则 `it = lower_bound(v, x)`（第一个 ≥ x）；`it == begin` 返回 0；`it == end` 返回 |v|−1；
否则 k = it − begin，`|v[k−1] − x| <= |v[k] − x|` 时返回 k−1（并列取小下标），否则 k。

### 2.13 AVAS.dll 版本的差异

- SetSynPhase（FUN_180039f20）、singlePartM（FUN_180042860）、SetUpSynParticle（FUN_18008c6b0）的算法与 .so 一致：
  同样的 atan（DLL 只导入 atan/sin/cos，无 atan2）、0.01/0.1 步长系数、21 点 18° 网格、两候选选择、`ae0 = a1 + 180` 的写法、
  扫描阶段 V3=2 的减号与主阶段的加号、相同的报错字符串。输出精度用 `setprecision`/`setiosflags`，值相同。Initial 成员偏移不同（MSVC string 32 字节）。
- 字符串 `"invalid？Synchronized particle phase abnormality, exit scanning phase  "`（UTF-8 夹杂乱码全角标点，
  文件偏移 0xa3040 与 0xa3780，分别位于 FUN_180039f20 与 FUN_180042860 的字面量池中，紧挨 "field" 与
  "Synchronous particle reverse motion…"）**在 DLL 中没有任何引用**：objdump 注释、对整个 .text 的 rel32 暴力搜索、
  8 字节指针与 4 字节 RVA 搜索都找不到。推测是源码里存在、但被编译器删掉的分支留下的字面量，这一版 DLL 永远不会打印它。
  新内核不需要实现；若要兼容旧日志可忽略。
- 两个二进制都**不识别 input.txt `scanphase`**，也没有 `scanData.txt` 的字符串：`Initial::Read` 对未知关键字只打印
  `input.txt Contains unknown keywords, check input.txt` 后继续。schema.py 里的 scanphase（0/1/2、从 scanData.txt 读相位）
  在现有内核中无效果；相位扫描是否执行只由"有无 V3=0 的 RF field"决定（needSynPhase）。

### 2.14 φs 定义的核实结论

内核的同步相位是
`φs = atan( ∫ Ez_amp(z) · sin φ(t(z)) dz / ∫ Ez_amp(z) · cos φ(t(z)) dz )`，`φ(t) = φ_in + 2πf(t − t_in)`（rad），
积分按步离散（§2.6.3(4)、§2.7.3），Ez_amp 是插值场图 Ez·Ke·1e6 的轴上值（未乘 cos），t(z) 是同步粒子实际轨迹。
**是 atan，不是 atan2**：结果限于 (−90°, 90°)。AGENTS.md 第 6 节写的 atan2 是我们线性模型的约定，
它与内核在加速解上一致，在相差 180° 的减速解上差 180°（内核写 −29.92°，atan2 为 150°）。

---

## 3. 对齐清单

| ID | 行为 | 涉及关键字/文件 | 建议的比对测试 |
|---|---|---|---|
| B-01 | 同步粒子初始 γβ 由 beam.txt kneticenergy、particlerestmass 计算（分布 unknown） | beam.txt | 单 field V3=1 lattice，比较 synData 第 0 行第 6 列与 `γ·sqrt(1−1/γ²)`，相对 1e-15 |
| B-02 | .dst 分布：SetSynPhase 用 particles[N].gbz，singlePartM 用 dst 平均能量；频率取 dst 头 ×1e6 | readparticledistribution、*.dst | part_rfq.dst 两种模式各跑一次，比较 synData 首行 γβ 与 Exit bunch energy，1e-12 |
| B-03 | beam.txt frequency 原样（Hz）作为步长分母 | frequency | 把 frequency 改为 2 倍，主阶段步数（由 synData t 推算）减半；z、t 与原内核比对 1e-9 |
| B-04 | SynParticle::Update 三种推进（纵向、无横向磁场、Boris）逐位一致 | — | 单元测试：给定 E、B、dt 与初值，比较 6 个坐标，0 ULP（同编译选项）或 1e-15 |
| B-05 | 只有存在 V3=0 RF field 时才扫描并重写 scanPhaseData.txt / Phase.txt | lattice V3 | V3=1 lattice 运行前放入旧 Phase.txt，运行后文件不变；V3=0 时被重写 |
| B-06 | 扫描步长 scantstep/(f·N)、主阶段 0.01/(f·N)、单粒子 0.1/(f·N) | scantstep、steppercycle | 改 scantstep 只影响 scanPhaseData；改 steppercycle 两者都变；比较 synData t 列 1e-12 |
| B-07 | 21 点扫描网格 −180:18:180，每点一行 `φ0 φs W_in W_out` | scanPhaseData.txt | hwr010 示例（V3=0，φs −33），逐行比较前 21 行，φs 1e-8 deg，W 1e-9 MeV |
| B-08 | φs 用 atan（±90°），末段乘两次比例、扫描用步首时刻 | — | 同 B-07；再把扫描改为 atan2 实现应在减速点出现 180° 差，确认原内核为 atan |
| B-09 | 两候选选择与端点 0↔20 修正 | — | 构造 φs 曲线跨越 ±90 的入口能量（0.7 MeV、1.05 MeV），比较选中的迭代起点（SPD 第 22 行 φ0） |
| B-10 | T<0 优先加速支；无加速支时取减速支并打印 "Note that the energy…" | stdout | 入口能量 0.7 MeV、φs −30：stdout 含该句，最终 φ0 ≈ −107.7°（AGENTS 实测），差 < 1e-6 deg |
| B-11 | 6 次割线迭代，SPD 写 ≤6 行，`s1 = a1 + 180` 的写法 | scanPhaseData.txt | 逐行比较迭代行；构造 T 接近 ±90 使 `|s1−T|>90` 分支触发 |
| B-12 | 无解报错条件：|φs−T|>1 且不贴近 ±90 | ErrorLog.txt | 0.3、0.8、1.0 MeV 入口（AGENTS 已核实拒绝运行），比较报错文字与 φs |
| B-13 | "3333 3333 3333 3333" 分隔行、Phase.txt 每腔两行（k+1、t、z、W，fixed 13 位） | Phase.txt | 两个 V3=0 腔的 lattice，逐字节比较 Phase.txt（容差 1e-12） |
| B-14 | cout 的 "The k field, phi_0: …" 与 Entry/Export 行 | stdout | 文本比较（数值 6 位有效数字） |
| B-15 | synData 行格式：首行精度 6、之后 17；非腔 `invalid invalid`；exitPlane 序号 n−1 / n 两种情况 | synData.txt | 末元件为 drift 与末元件为 field 的两条 lattice，逐字段比较 |
| B-16 | 主阶段 φs 用步末时刻，腔出口 +1e-13，元件入口 +1e-15 | synData.txt | hwr010：synData 第 6 列与原内核差 < 1e-9 deg，z_in 差 < 1e-14 m |
| B-17 | V3=2：扫描阶段 `word6 − 360 f t_in`，主阶段 `word6 + 360 f t_in`（规约到 ±180） | lattice V3=2 | (a) 只有 V3=2 的 lattice：synData φRF = word6+360ft 规约；(b) V3=2 腔在 V3=0 腔上游：synData 中 V3=0 腔 φs 偏离目标的量与原内核一致 |
| B-18 | V3=1：φRF = word6，cavTime = 主阶段到达时刻 | — | V3=1 单腔，比较 synData φRF 与 Exit bunch energy |
| B-19 | 最终 fm.cavPhase/cavTime 交给束团：三种 V3 的等价相位 | DataSet.txt | V3=0 跑一次，取 synData 的 φRF 改写成 V3=1、再按 +360ft 改成 V3=2，三次 DataSet 能量列一致（固定种子，1e-10） |
| B-20 | 反向运动报错 "Synchronous particle reverse motion, program stopped" | — | 低能 + 强减速相位（V3=1，phase 180）使粒子折返 |
| B-21 | RFQ 扫描：φ0 = (π/2 − 2πf_beam t)·180/π，减速时重扫 −π/2，仍减速报 "RFQ scanning phase abnormality" | rfq | 需要 RFQ 输入样例（见 §5） |
| B-22 | superposeout 中同步粒子的坐标变换与出口插值 | superposeout | 含 superposeout 块的 lattice，比较 synData 中块后元件的 z_in、t_in |
| B-23 | needSynPhase==0 时只 push RF field 的 word6，不处理 RFQ | — | 只含 V3=1/2 的 lattice，与原内核 DataSet 一致 |
| B-24 | singlePartM：先 ReadBeam，gbzIn=−33，0.1 步长，synData 全 17 位 | singleparticlemode 1 | 单粒子模式跑 hwr010，逐字段比较 synData |
| B-25 | SetUpSynParticle 打不开 beam.txt / dst / 无扩展名时的输出与退出码 | — | 删除 beam.txt：stdout 文本与退出码 1 |
| B-26 | Drift 孔径外报 "Synchronised particles can't reach the exit plane…" 并 exit(1) | — | superposeout 偏移大于孔径时触发（fullPush=1） |
| B-27 | TcodeTrans 单步：Boris 推进、相对坐标三种 rotMode、输出面插值 | DataSet.txt、outData_*.dst | 单元测试对单个粒子；整体用固定种子比较 DataSet（子系统 D） |
| B-28 | monotonicSearch 游标跨查询保留、找不到后全部失败 | — | 单元测试：xs=[0,1,2,3]，qs=[0.5, 5, 1.5] → 第三个返回 {−1,−1,0,0} |
| B-29 | findClosest 并列取小下标、空向量抛 "Vector is empty!" | — | 单元测试 |
| B-30 | input.txt `scanphase` 被忽略（只打印未知关键字警告） | scanphase | 加 `scanphase 2` 与 scanData.txt，结果与不加完全相同 |

---

## 4. 函数覆盖表

| 函数 | 作用一句话 | 在正文哪节 |
|---|---|---|
| Initial::SetSynPhase | 扫描 V3=0 腔的入口相位、登记各腔相位与时刻、主阶段写 synData.txt 并写回 fm | §1.3、§2.6、§2.7、§2.8 |
| Initial::singlePartM | 单粒子模式下的 SetSynPhase 副本（先 ReadBeam、gbzIn=−33、主阶段 0.1 步长） | §2.9 |
| SynParticle::SynParticle | 构造：coord 6 个 0、E/B 3 个 0、标志清零 | §2.2 |
| SynParticle::~SynParticle | 析构：释放 5 个 vector | §2.2 |
| SynParticle::SetUpSynParticle | 由 beam.txt / .dst / .edst 确定同步粒子初始 γβ、质量、荷质比、频率 | §2.3 |
| SynParticle::NormalizeLineEndings | 读整个文件为字符串并把 \r\n、\r 统一为 \n | §2.3 |
| SynParticle::GetField | 线性搜索当前元件并调用元件的同步粒子取场（越界给零场） | §2.4 |
| SynParticle::Update | 同步粒子一步推进：纵向 / 无横向磁场 / Boris 三种 | §2.5 |
| RFQ::GetSynField | RFQ 轴上 Ez 双项展开乘 RF 余弦，写 E 与 EzAmp | §2.10 |
| TcodeTrans | 束团 t-code 一步推进的线程工作函数（统计、Boris、相对坐标、取场、输出面与 superposeout 记录） | §2.11 |
| monotonicSearch | 有序数组的单调游标区间查找（importSCF 用） | §2.12 |
| findClosest | 有序数组中最接近 x 的下标（importSCF 用） | §2.12 |

---

## 5. 未解决的问题（需要原内核实验）

1. **扫描阶段 V3=2 的减号是否真的影响结果**：伪代码与反汇编（.so 0x5ca38 `subsd`，DLL 第 1811 行）都确认减号。
   实验：lattice = [V3=2 腔, drift, V3=0 腔]，比较 synData 中第二个腔的 φs 与目标；再把第一个腔改成等价的 V3=1（φRF 取自 synData），
   若后者 φs 准确而前者偏离，就证实这是原内核行为（新内核需复现）。
2. **`s1 = a1 + 180` 写法的影响**：目标 φs 接近 ±90° 时才触发。实验：φs = 85°、−85° 扫描，比较 scanPhaseData 迭代行。
3. **扫描阶段与主阶段的积分时刻差一步**：会使 synData 的 φs 与目标有约 360·f·dt_m 量级以外的系统差。实验：hwr010（φs −33），
   看 synData φs 与 −33 的差，并在新内核中复现。
4. **RFQ 在主阶段的处理**：主阶段把 rfq 当普通元件，RFQ::GetField(Syn) 会一次推过整个 RFQ，但主阶段不读 `coord[0]` 的出口时刻，
   t 不含 RFQ 渡越时间；且 P.cavTime 不为 RFQ 追加，导致后续腔的 cavTime 下标错位、RFQ 自己读 cavTime 越界。
   这看起来是缺陷，需要一个含 RFQ 的输入（仓库没有）运行原内核确认实际输出，或确认 RFQ+下游腔的组合从未被使用。
5. **cavPhase 的 push 位置假定等于 word14**：若 lattice 中 RF 元件的 word14 不是按出现顺序 0,1,2…（例如有 RFQ 或 InitLattice 的编号规则不同），
   扫描阶段的 push_back 与按下标写入会错位。需要读 InitLattice 的 word14 赋值规则（子系统 A）并做含 RFQ / 多腔的实验。
6. **TcodeTrans 的 lost = 2 条件 `spOutCur == |spOut|`**：spOut 含哨兵行，正常情况下 spOutCur 最大为 |spOut|−1，此条件似乎达不到；
   lattice 末端的 lost=2 实际由谁设置（structures.md 说是 Lattice::GetExternalField）需与子系统 D 对照，并用越过末端的粒子实验。
7. **扩展名非 dst/edst 的分布文件**（如 .dstb、.txt）时 SetUpSynParticle 不初始化同步粒子：实验看原内核是死循环、崩溃还是报错。
8. **扫描内部无反向检查**：入口能量极低、相位使粒子在腔内折返时，扫描循环是否不终止（原内核挂起）。用超时运行验证。
9. **param_21（TcodeTrans 的 g）**：推测为 γ（DataSet 的 rms z 乘 γ），需对照 tcodeRun 中 local_9b0 的来源（子系统 D）。
10. **RFQ::GetSynField 系数含义**（coef[1]、coef[6]、cellV×1000 的单位）与 `getEleNum` 返回 −1 时的越界计算：需 RFQ 输入样例与子系统 C 对照。

---

## 6. 对结构表的更正

1. **SynParticle +0xb0 frequency**：structures.md 写"beam.txt frequency(MHz)×1e6"。实际上 beam.txt 分支（A、C）**原样保存，不乘 1e6**；
   只有 .dst 分支把 dst 头里的 MHz 乘 1e6。beam.txt 的 frequency 本身就是 Hz（schema.py 单位 Hz，示例 `frequency 162500000.0`）。
   证据：`SetUpSynParticle` 0xd6221 `movsd [rax+0xb0], xmm0` 前无乘法；0xd50a1–0xd50d0 只在 dst 分支乘 1e6。
   同理 structures.md 对 `Initial.frequency`"MHz；InitParticle 后改为 Hz"和 `Beam.frequency`"MHz×1e6"的说法值得子系统 A/E 复核（本文件未核）。
2. **SynParticle::Update 的磁场判断**：structures.md 写"B 全为 0 时跳过旋转"。实际只检查 Bx、By；Bx = By = 0 时 Bz 被完全忽略（`001d4490` 首个分支）。
3. **SynParticle +0x80 fullPush**：补充置位规则——SetSynPhase 在当前元件 `Element.inSuperposeOut(+0x68) != 0` 的静态场步置 1，越过该 superposeout 出口后置 0；构造时为 0。
4. **SynParticle +0xa8 EzAmp**：可改为"确定"：SetSynPhase 用它作 φs 积分中的轴向场幅值；SetUpSynParticle 初始化为 0。
5. **SetUpSynParticle 签名**：`(std::string distFile, double Q, double A, std::string inputDir, int secondaryBeam, double gbzIn)`，第三个参数 A 在函数内不使用；
   `gbzIn == −33.0` 是"由文件能量计算"的哨兵。
6. **Particle +0x104 nextPlane**：不只出现在二次粒子路径；主束的 `TcodeTrans` 同样用它做输出面插值并 `++`（§2.11 第 8 步），可改为"确定"。
7. **Lattice.spOut 行的使用顺序**：同步粒子与 TcodeTrans 都按 `[0]=z0, [1]=x0, [2]=y0`，先用 [3] 旋转 x–y、再用 [5] 旋转 x–z、最后用 [4] 旋转 y–z，
   局部 z 还要再减去当前元件的 zStart（`Element+0x48`）；[6] 为出口 z。哨兵行 `{0,0,0,0,0,0,L_total}` 由 InitLattice 追加（`001763e0` 第 5747–5792 行），`spOutCount` 不含哨兵。
