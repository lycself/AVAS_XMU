# 50 初始粒子分布与随机数（规格）

对象：`libAVAS.so`（GCC 4.8.5，下称 **.so**）中 `Initial::InitParticle` 及其调用的分布生成器、随机数函数；
同时核对了 `AVAS.dll`（MSVC，下称 **.dll**，Windows 用户实际运行的是它）中的对应函数，差异集中写在 §2.6。
伪代码：`decomp/libAVAS/functions/00162630_Initial_InitParticle.c` 等；关键算术顺序、读写字节数、字符串常量已用
`llvm-objdump` 反汇编 `.so` 逐条核对（Ghidra 丢失的比较字面量、复数运算、`std::istream::read` 的长度都以反汇编为准）。
字段名沿用 [structures.md](structures.md)；`this` 指 `Initial`，`beam` 指 `Beam`，`P[i]` 指 `beam.particles[i]`，`P[N]` 是同步粒子。

---

## 1. 概述

### 1.1 在运行流程中的位置

`main_agent`（多粒子模式）的顺序：

```
Initial::Initial()            // 默认值（§2.3.1）
Initial::Read()               // input.txt，其中 randomseed（整数）写 this.randomSeed
Initial::InitLattice(...)     // lattice
GenerateErrorSet(...)         // 误差样本；内部用 rand() 与 GenerateGaussian，但不调用 srand
Initial::InitParticle(beam, errSample, lattice)   // ← 本文
Initial::SetSynPhase(lattice, P[N].gbz)           // 同步相位（B 子系统），输入是本文给出的同步粒子 γβ
Initial::InitMethodMP / MethodMP::Run 或 SeRun
```

`singleparticlemode 1` 时 `main_agent` 走 `singlePartM`，不调用 `InitParticle`（不属于本文）。
`particlenumber 1`（单宏粒子）仍走 `InitParticle`，见 §2.4.3。

### 1.2 InitParticle 做什么

1. `ReadBeam()` 读 beam.txt（A 子系统；本文只写它给出的值如何被使用，§2.3.1）。
2. 按 `secondarybeam`（`this.secondaryBeam`）与 `readparticledistribution`（`this.distFile`）选择分支（§2.3.2）：
   按 Twiss 参数生成（最常用）、读 `.dst`、读 `.edst`、读二次粒子（`.dst` 或文本）。
3. 填写 `beam`（粒子数组、频率、流强、每宏粒子代表的真实粒子数、束团时间范围等）和每个 `Particle` 的全部初值。
4. 生成路径与 `.dst` 路径都写出 `OutputFile/inData.dst`（TraceWin `.dst` 二进制格式），生成路径还**再读回**它，粒子的最终初值来自读回的数据。
5. 顺带填写空间电荷网格、线程数等默认参数（F 子系统使用，§2.5.2）。
6. 最后给所有粒子写 superposeout 相关的两个字段（§2.5.4）。

### 1.3 分布生成器的调用关系

```
InitParticle ──srand(time(NULL))──┬─ KV2D ─┐
                                  ├─ WB2D ─┼─ PreTreatment(twiss) ; GenerateCoordinates（每粒子 3 次 rand）
                                  ├─ PB2D ─┘
                                  └─ GS2D ── PreTreatment(twiss) ; Gaussu（每次尝试 8 次 rand）
（每个 *2D 函数开头：seed != 0 时 srand(seed)）
GenerateErrorSet ── GenerateGaussian（Box–Muller，静态缓存）
rand_gen、normalRandom：.so 中没有任何调用者（死代码）
```

`PreTreatment` 属 A 子系统的清单，但它是生成器的一部分，§2.2.1 写出它对本文必需的输入输出约定。

---

## 2. 规格正文

### 2.1 随机数发生器

#### 2.1.1 底层发生器：C 库 `rand()`，两种平台不同

全部随机数都来自 C 库 `rand()`/`srand()`，没有 `std::mt19937`、`std::normal_distribution` 或自写 LCG。
源码显然使用了 `RAND_MAX` 宏，所以两个二进制里归一化常数不同：

| | .so（Linux，glibc） | .dll（Windows，UCRT） |
|---|---|---|
| RAND_MAX | 2147483647，代码中写作 `/ 2147483647.0` | 32767，代码中写作 `* 3.051850947599719e-05`（=1/32767） |
| 算法 | glibc `random()` TYPE_3（加性反馈，见下） | LCG：`s = s*214013 + 2531011 (mod 2^32)`，返回 `(s >> 16) & 0x7FFF` |
| 未调用 srand 时 | 等同 `srand(1)` | 状态 1 |
| 时间种子 | `srand((unsigned)time(NULL))` | `srand((unsigned)_time64(NULL))` |
| 线程 | 进程全局，带锁 | UCRT 每线程一份状态 |

glibc `srand(seed)` / `rand()` 的精确算法（重写时必须自己实现，不能用宿主平台的 `rand()`）：

```
r[0] = (int32)seed;  若 seed == 0 则 r[0] = 1
for i in 1..30:      hi = r[i-1] / 127773; lo = r[i-1] % 127773      // C 的截断除法，int32
                     w  = 16807*lo - 2836*hi; if w < 0: w += 2147483647; r[i] = w
for i in 31..33:     r[i] = r[i-31]
for i in 34..343:    r[i] = (r[i-31] + r[i-3]) mod 2^32               // 丢弃（glibc 预热 310 次）
第 k 次 rand()（k=0,1,…）: r[344+k] = (r[344+k-31] + r[344+k-3]) mod 2^32; 返回 (uint32)r[344+k] >> 1
```

由此得到的整数序列再按各函数自己的公式转成实数（下文逐一写出）。**同一份输入在 Linux 与 Windows 上生成的粒子分布不同**，
Windows 的均匀数只有 32768 个取值（例如 KV/WB/PB 的 r² 量化为 1/32767 的整数倍）。

#### 2.1.2 种子：randomseed 的来源、优先级与转换

- input.txt `randomseed`：`Initial::Read` 用 `stoi` 读成整数，再存成 double 到 `this.randomSeed`（+0xc8）。
- beam.txt `randomseed`：`ReadBeam` 用 `stod` 读成 double，写同一个字段。
- `Read` 在 `InitParticle` 之前执行，`ReadBeam` 在 `InitParticle` 开头执行，所以 **beam.txt 有 randomseed 时覆盖 input.txt 的值**；
  两处都没有时为构造函数默认值 0。
- 使用时 `(int)this.randomSeed`（`cvttsd2si`，向零截断；超出 int 范围得 INT_MIN），作为 `*2D(..., seed)` 的第三个参数。
- 生成路径的播种次序（.so 与 .dll 相同）：
  1. `srand((unsigned)time(NULL))`（无条件）；
  2. 调横向生成器 `XX2D(twiss, rows, seed)`：函数开头 `if (seed != 0) srand(seed)`；
  3. 若生成纵向（§2.4.1 第 15 步），再调 `YY2D(twissZ, rows, seed)`：**再次** `if (seed != 0) srand(seed)`。
- seed = 0：两次调用共用一条以当前秒数为种子的序列（横向用完接着纵向），每次运行不同。
- seed ≠ 0：横向与纵向**各自从同一个种子重新开始**（见 §2.1.3）。
- `GenerateErrorSet` 在 `InitParticle` 之前消耗的随机数对生成的分布没有影响（随后必然重新播种）。

#### 2.1.3 重要发现：固定种子时横纵向完全相关

seed ≠ 0 时纵向生成器与横向生成器从同一状态开始、按同样次序取数。只要横纵向分布类型相同，第 i 个粒子在两次调用中得到
完全相同的随机量（KV/WB/PB 的 R²、r²、θ₁；GS 的 g0、g1），于是

- (z, z') 与 (x, x') 是同一组随机量经过各自 Twiss 变换的结果：z_i/√(β_zε_z) ≡ x_i/√(β_xε_x)，z'、x' 同理；
- inData.dst 中 x 与相位 φ = −2πf·z/v_z 的相关系数约为 −1。

各平面自身的 rms 发射度与 Twiss 参数不受影响，但 6 维分布带有完全的 x–z、x'–δ 关联。类型不同时（如 GS/KV）也共用同一随机序列，
关联较弱但不为零。seed = 0 时没有这个问题。实验验证见 §5 Q1。

#### 2.1.4 多线程与进程状态

- `InitParticle` 及其生成器完全串行，在主线程执行；`multithreading`、`setnumberofcores` 不影响随机序列。
- 随机状态是进程级的。若同一进程多次调用内核（例如 Python 侧在同一进程内连续调用 `main_agent`），
  上一次运行留下的 `rand()` 状态和 `GenerateGaussian` 的静态缓存会带入下一次（对生成分布无影响，因为必然重新播种；
  对 `GenerateErrorSet` 有影响，属 G 子系统）。

#### 2.1.5 `rand_gen`、`normalRandom`（.so 中无调用者）

- `rand_gen() = (rand() + 1.0) * 4.656612873077393e-10`（= (rand+1)/2^31，取值 (0, 1]）。
- `normalRandom()`：先 `u1 = rand_gen()`，后 `u2 = rand_gen()`；返回 `cos(u2 * 6.283185307179586) * sqrt(log(u1) * -2.0)`。
- 两者在全部 214 个函数中都没有调用点，新内核可以不实现；若实现，次序如上。

#### 2.1.6 `GenerateGaussian(mean, sigma)`（GenerateErrorSet 使用）

Box–Muller，成对生成、隔次返回，状态是三个函数内静态变量 `generate`（bool，初值 false）、`z0`、`z1`：

```
generate ^= 1
if generate == 0: return z1 * sigma + mean          // 偶数次调用：返回上次缓存（未缩放）的 z1
do { u1 = rand() * 4.656612875245797e-10;            // = rand()/2147483647
     k2 = rand();                                     // 每轮循环两个数都重新抽
} while (u1 <= 2.2250738585072014e-308)               // 拒绝 u1 = 0
r  = sqrt(log(u1) * -2.0)
(s, c) = sincos(k2 * 4.656612875245797e-10 * 6.283185307179586)   // 先乘 1/RAND_MAX，再乘 2π
z0 = r * c ; z1 = r * s                                // z1 用第二次 log/sqrt 计算，数值相同
return sigma * r * c + mean                            // 即 (sigma*r)*c + mean
```

缓存的 z1 未缩放，下一次调用用**那一次**的 mean、sigma 缩放。.dll 版（`FUN_18001e1a0`）：`u1 = rand()/32767`，
角度 `= k2 * 0.00019175345033660654`（=2π/32767，一次乘法），且只用 sigma、不加 mean（所有调用点 mean 都是 0.0，编译器特化，推测）。

### 2.2 四维分布生成器

#### 2.2.1 输入向量与 PreTreatment 约定

`XX2D(const vector<double>& tw, vector<vector<double>>& rows, int seed)`：

- `tw` 至少 6 个元素：`[ε₁, β₁, α₁, ε₂, β₂, α₂, …]`，ε 为**几何 rms 发射度**（m·rad），β 单位 m/rad（= mm/mrad），α 无量纲。
  横向调用传 `this.twiss`（9 个，已换算，§2.4.1 第 10 步）；纵向调用传 `[ε_z, β_z, α_z, ε_z, β_z, α_z, 0, 0, 0]`。
- `rows` 预先分配，每行 6 个 0.0；生成器对每行写 `[q₁, q₁', q₂, q₂', 0, 0]`（GenerateCoordinates 与 GS2D 都把 [4]、[5] 写 0）。

`PreTreatment(tw)` 返回 6 个 double 的向量 `T`（数学上）：

```
T[0] = √β₁         T[1] = −α₁/√β₁
T[2] = √β₂         T[3] = −α₂/√β₂
T[4] = ε₁/ε₂       T[5] = 0     （KV/WB/PB 随后把 T[5] 改写为本粒子的 R²_max）
```

但它不是按上式直接算的：先构造 σ = ε[[β, −α], [−α, (1+α²)/β]]，检查 `σ11σ22 > σ12²`（否则打印
`ERROR!!!   @SigmaToAxial2D!` 并 `exit(1)`），求 σ⁻¹ 的特征值（Eigen `SelfAdjointEigenSolver`，2×2 动态矩阵）、
半轴 a、b 与倾角 θ = ½·atan(2σ12/(σ11−σ22))（σ11 == σ22 时 θ = 0），再由 a、b、θ 回算 √β = √((a/b)cos²θ + (b/a)sin²θ) 等，
`T[4] = (4ab)₁/(4ab)₂`。逐位复现需要复制这一算法（属 A 子系统文档）。本文只依赖上面的数学约定，并指出一个缺陷：
**σ11 == σ22 而 σ12 ≠ 0（β² = 1+α² 且 α ≠ 0）时 θ 应为 ±45° 却取 0，T[0]、T[1] 错误**。

#### 2.2.2 `GenerateCoordinates(T, row)`（KV、WB、PB 共用）

要求 `T.size() ≥ 5`、`row.size() ≥ 6`（否则 `std::out_of_range("vector::_M_range_check")`）。设 U(k) = k / RAND_MAX：

```
k0 = rand();  r2 = U(k0) * T[5];  r = sqrt(r2)
ry2 = (T[5] - r*r) / T[4];        ry = sqrt(ry2)          // 用 r*r，不是 r2
k1 = rand();  k2 = rand()
th1 = (U(k1) + U(k1)) * 3.141592653589793                 // .so；两个角度都在任何 sincos 之前算出
th2 = (U(k2) + U(k2)) * 3.141592653589793
(s1, c1) = sincos(th1)
row[0] = (T[0] * r) * c1
row[1] = (c1 * T[1] - s1 / T[0]) * r
(s2, c2) = sincos(th2)
row[2] = (T[2] * ry) * c2
row[3] = (c2 * T[3] - s2 / T[2]) * ry
row[4] = 0;  row[5] = 0
if !(row[2] < 100000.0 && row[3] > -100000.0):  row[2] = row[0]; row[3] = row[1]
```

最后一行是 NaN 保护：k0 = RAND_MAX 时 r·r 可能比 T[5] 大 1 ulp，ry2 为负、ry = NaN，此时**第二平面直接复制第一平面**
（.so 中概率 ~5e-10/粒子；.dll 中 rand()==32767 的概率 1/32768，十万粒子约 3 个）。

物理含义：令 J₁ = (q₁² + (α₁q₁+β₁q₁')²)/β₁ 等，则 r² = J₁、ry² = J₂·(ε₁/ε₂)，粒子落在 4 维超椭球面
**J₁/ε₁ + J₂/ε₂ = T[5]/ε₁** 上，两平面相位 θ₁、θ₂ 独立均匀。

#### 2.2.3 四种分布

每个函数：`if (seed != 0) srand(seed); T = PreTreatment(tw); ε = tw[0];` 然后对 `rows` 逐行（按下标递增）：

| 函数 | 本粒子 T[5]（= R²_max）| 每粒子 rand 次数 | 4 维径向分布 |
|---|---|---|---|
| KV2D | `ε * 4.0` | 3 | 超椭球面（K-V），J₁/ε₁+J₂/ε₂ = 4 |
| WB2D | `(sqrt(U(k)) * ε) * 6.0`，k 先于 GenerateCoordinates 抽取 | 4 | 体内均匀（waterbag），R² 的 CDF = s²（s = R²/6ε） |
| PB2D | `s * (8.0 * ε)`，s 见下 | 4 | 抛物线密度 ∝ (1 − R²/8ε)，CDF(s) = 3s² − 2s³ |
| GS2D | 不用 GenerateCoordinates | 8×尝试次数 | 不截断的 4 维高斯 |

四种分布的投影都满足 ⟨q²⟩ = βε、⟨qq'⟩ = −αε（各平面 rms 发射度等于输入的 ε）。

**PB2D 的 s**（反汇编核对的运算次序；`std::sqrt/pow` 为 libstdc++ 复数版本）：

```
U  = rand() / 2147483647.0
q  = csqrt(U*U - U + 0.0i)                       // 实部 U*U-U ≤ 0 → q = (0, √(U-U²))
w  = ((1.0 - (U+U)) + (q.re + q.re))  +  i·(q.im + q.im)      // |w| = 1
c  = (w.im == 0 && w.re > 0) ? (pow(w.re, 1/3.), 0)
                             : polar(exp(clog(w).re * (1/3.)), clog(w).im * (1/3.))   // sincos 求 cos、sin
d  = __divdc3(1.0, -1.7320508075688772, 4.0*c.re, 4.0*c.im)   // (1 − i√3)/(4c)
m  = __muldc3(c.re, c.im, 1.0, 1.7320508075688772)            // c·(1 + i√3)
s  = ((0.5 + d.re) + m.re * 0.25)                              // 只取实部
```

数学上 s = ½ + cos((φ+π)/3)，φ = arg w ∈ [0, π]，即 CDF 3s²−2s³ 的反函数取 1−U（U=0 → s=1，U=1 → s=0）。
复数除法用 libgcc `__divdc3`（带缩放），逐位复现需要照搬其算法。

**GS2D**（Marsaglia 极坐标法的变体）：

```
sq = sqrt(T[4])                      // √(ε₁/ε₂)
s  = sqrt(ε * 4.0) * 0.5             // 等于 √ε（逐位相同）
u[0..7] 是一个跨粒子复用的 8 元缓冲区
对每一行：
  repeat:
    for k in 0..7:                    // 抽 8 个“互不相同”的均匀数
      u[k] = rand()/RAND_MAX，若与 u[0..k-1] 中任何一个相等则重抽 u[k]（只重抽这一个）
    Gaussu：for j in 0..3:
      a = (u[2j]+u[2j]) - 1;  b = (u[2j+1]+u[2j+1]) - 1;  t = a*a + b*b
      if 1.0 < t: 本次尝试作废（已算出的 g[0..j-1] 保留在缓冲区）
      g[j] = a * sqrt((log(t) * -2.0) / t)          // 每对只用一个输出
    g[0..3] *= s（原地）
    row[0] = (g0·s 之前的 g0 * s) * T[0]            // 即 g0s * T[0]
    row[1] = g1s / T[0] + g0s * T[1]
    row[2] = (T[2] * g2s) * (1.0/sq)
    row[3] = (g3s / T[2] + g2s * T[3]) * (1.0/sq)
    row[4] = row[5] = 0
  until 本次尝试未作废                               // 作废时 8 个数全部重抽
```

作废的尝试也会写 row（随后被覆盖），不影响结果。t = 1.0 不作废。互异检查在 .so 中的碰撞概率约 1e-8，在 .dll 中每次尝试约 0.09 %，
必须实现才能逐位复现 .dll。

#### 2.2.4 纵向使用的含义

纵向调用时第二平面参数与第一平面相同，生成后只取 row[0]（z）、row[1]（z' = Δp/p），row[2]、row[3] 丢弃。
因此纵向分布是**4 维分布在一个平面上的投影**：KV → 均匀填充椭圆，WB → 抛物线型（∝ 1−r²），PB → ∝ (1−r²)²，GS → 2 维高斯；
不是二维 KV/WB 的教科书定义。`learn` 入门页的说明应以此为准。

### 2.3 InitParticle 的输入

#### 2.3.1 beam.txt 关键字给出的 Initial 字段（.so 的 ReadBeam 与构造函数）

| 关键字 | 字段 | 读入处理 | 默认（`Initial::Initial`） |
|---|---|---|---|
| readparticledistribution | distFile (+0x08) | 原样字符串 | `"unknown"`（= 生成） |
| numofcharge | charge Q (+0x10) | strtod | — |
| particlerestmass | restMass mc² (+0xa0, MeV) | stod | 938.272029 |
| kindofparticle | kind (+0xe8) | 字符串 | `"proton"` |
| distribution V1 V2 | distribution[0]、[1] (+0x20) | 必须 3 个词，否则抛 `"The initial bunch distribution contains two parameters, separately in transvers and longitudinal"`；每个必须是 KV/WB/PB/GS，否则抛 `"Distribution Type Error, The Format is: distribution  KV/WB/PB/GS  KV/WB/PB/GS"` | `GS GS` |
| beamtype dc | twiss[6] = −2027.0415（哨兵） | 第二个词等于 `dc` 才生效 | — |
| twissx α β ε | twiss[2]=α, [1]=β, [0]=ε | 必须 4 个词，否则抛 `"twissx have 3 parameters, emittance, beta, alpha"`（提示文字顺序与实际相反） | 9 个分量都是 −2027.041 |
| twissy α β ε | twiss[5], [4], [3] | 同上 `twissy …` | |
| twissz α β ε | twiss[8], [7], [6] | 同上；若此时 twiss[6] == −2027.0415（已出现 beamtype dc）则只把 twiss[6] 改为 −2027.041，忽略数值 | |
| particlenumber | particleNumber (+0x38) | stoi | −1 |
| displacepos dx dy dz | displacePos[k] (+0x58) | 每个值 ×0.001（**按 mm 读**，与 AGENTS.md 一致） | 0,0,0 |
| displacedpos | displacedPos[k] (+0x70) | 每个值 ×0.001 | 0,0,0 |
| frequency | frequency (+0xb0) | stod，**单位 Hz**（与 schema 一致；生成路径不再换算） | −1 |
| kneticenergy | kineticEnergy W₀ (+0xb8, MeV) | stod | −1 |
| current | current (+0xa8, mA) | stod | −1 |
| randomseed | randomSeed (+0xc8) | stod | 0 |
| recscfield / impscfield | +0x218 / +0x21c | stoi，InitParticle 开头拷入 beam+0xd8 / +0xdc | 0 |
| beamlength、bunchnumber | +0xf0 / +0x1e4 | — | 本文不用 |

注：
- `use_dst`、`initpos`、`initmom`、`beamtype notdc` 等 schema 中的关键字 .so 不认识：前三者打印
  `beam.txt Contains unknown keywords, check input.txt` 后继续（不抛错）；`beamtype` 后接非 `dc` 的词时什么也不做。
- 不写 twissz 时 twiss[6] 保持 −2027.041，**等同 DC 束**（§2.4.4）。不写 twissx/twissy 时 ε、β 都是 −2027.041，
  换算后 ε 为负，PreTreatment 的正定检查仍可能通过，结果无意义（推测）。
- 不写 particlenumber（−1）时 `new` 申请长度溢出，抛 `std::bad_alloc`。

input.txt 给出的、本文用到的字段：`secondaryBeam`(+0x00)、`spaceCharge`(+0x110)、`boundary`(+0x144)、`probe`(+0x14c)、`indWeight`(+0x150)、
`scSolverRecord`(+0x1c4)、`multithreading`(+0x1e8)、`maxThreads`(+0x1ec)、`ompMaxThreads`(+0x1f0)、`seStep`(+0x210)、`scPar`、`numOfGrid`、`cores`。
另有两个只有默认值的常量：`+0x130 = 1`（“入口分布换算”开关，见 §2.4.6）、`+0x140 = 3`（维数）。

#### 2.3.2 分支选择

```
if this.secondaryBeam == 0:
    if distFile == "unknown":                  → 生成路径 §2.4.1
    ext = distFile 中最后一个 '.' 之后的部分（没有 '.' → 抛 "The initial bunch distribution file cannot be opened, check particleDistributionPath"）
    if ext == "dst":                           → .dst 路径 §2.4.7
    elif ext == "edst":                        → .edst 路径 §2.4.8
    else:                                      抛同一条信息
else:
    （没有 '.' → 抛同一条信息）
    if ext == "dst":                           → 二次束 .dst 路径 §2.4.9
    else:                                      → 二次束文本路径 §2.4.10（任何其他扩展名）
所有读文件分支的文件路径 = 全局 inputFilePath + distFile（字符串直接拼接）。打不开 → 抛同一条信息。
```

比较区分大小写（`std::string::compare`）。抛出的都是 `std::string` 异常。

### 2.4 各分支

#### 2.4.1 生成路径（readparticledistribution unknown）

常数：e = 1.60217653e-19，m_p = 1.67262171e-27，c = 299792458.0，π = 3.141592653589793，2π = 6.283185307179586，
质子单位 938.271875 MeV。下文的除以 1000、1e6 都是真正的除法（`divsd`）。

1. `beam.frequency = this.frequency`（Hz）；`mc2 = this.restMass`；打印 `Starting to generate initial bunch`。
2. γ₀ = W₀/mc2 + 1；β₀ = sqrt(1 − (1/γ₀)/γ₀)；**βγ₀ = β₀·γ₀**。
3. `this.atomicMass A = mc2 / 938.271875`；`this.current = current/1000`（A）；`beam.scSolverRecord = this.scSolverRecord`；`beam.current = this.current`。
4. 束流误差修正 Twiss：若 `flags[0]+flags[1] != 0` 且 `dyn[0][0] == 1.0`（`dyn` = ErrorSample+0x18 的第 0 行）：
   `I += e[13]/1000`；`ε_x *= (e[7]/100+1)`，`ε_y *= (e[8]/100+1)`，`ε_z *= (e[9]/100+1)`；
   `β_x、α_x *= (e[10]/100+1)²`，`β_y、α_y *= (e[11]/100+1)²`，`β_z、α_z *= (e[12]/100+1)²`
   （平方按 `t = e/100+1; v = t*t*v` 计算；对 DC 哨兵 twiss[6] 同样相乘，会破坏哨兵，推测是缺陷）。
5. 若 `I == 0` 或 `particleNumber == 1`：`this.current = beam.current = 0`，`this.spaceCharge = 0`，打印 `0 intensity, stop SCfield calculations`。
6. 分配粒子数组：`probe == 1` 时 `beam.n = 125`、`new Particle[126]`、`this.spaceCharge = 0`；否则 `beam.n = N = particleNumber`、`new Particle[N+1]`。
   （`Particle` 默认构造只置 `spOutCount=0` 与空串 `kind`，其余字段是未初始化内存。）
7. 每宏粒子真实粒子数 `rpm`：`beam.current == 0` → rpm = 1.0，打印 `The intensity is set to 0`（不写 beam.realPerMacro）；
   否则 `rpm = |(((I/f)/N)/e)/Q|`（N = particleNumber，**不是 beam.n**），写 `beam.realPerMacro`，打印
   `The number of particles represented by a single macro particle: ` + rpm。
8. 空间电荷与线程默认值（§2.5.2）。
9. `particleNumber == 1` → 单粒子分支 §2.4.3，**直接 return 0**（不写 inData.dst，不做 §2.5.4）。
10. Twiss 换算为几何量（SI）：
    `twiss[0] = (twiss[0]/1e6)/βγ₀`，`twiss[3] = (twiss[3]/1e6)/βγ₀`；
    twiss[6]：== −2027.0415 → 改为 −2027.041；== −2027.041 → 不变；否则 `twiss[6] = (twiss[6]/1e6)/(βγ₀·γ₀·γ₀)`（即 ε_z,n/(βγ³)）。
    β、α 不换算（mm/mrad = m/rad）。
11. `rows`：N 行，每行 6 个 0.0；`probe == 1` 时另建同样的 `rows2`。`beam.totalWeight = N`。
12. 播种并生成横向（§2.1.2）：按 distribution[0] 依次比较 `"KV"`、`"WB"`、`"GS"`、`"PB"`，调用对应 `XX2D(this.twiss, rows, (int)seed)`。
13. 对 i = 0..beam.n−1（probe 时 beam.n = 125，若 particleNumber < 125 会越界读 rows，未定义行为）：
    `P[i].x,gbx,y,gby = rows[i][0..3]`（此时 gbx、gby 暂存 x'、y'，单位 rad；P[i].z、P[i].gbz 不写）；
    `elemIndex = 0`，`lost = 0`，`nextPlane = 0`，`geomIndex = 0`；
    `charge = (rpm*Q)*e`，`mass = (rpm*A)*m_p`，`qOverM = ((Q/A)*e)/m_p`；`restMassMeV = mc2`；`kind = this.kind`；
    `boundaryMode = this.boundary`；`weight = 1.0`。
14. probe：`rows2[i][0..3] = rows[i][0..3]`；并计算 σx、σy（§2.4.5）。
    同步粒子 P[N]（N = beam.n）：`elemIndex=0, lost=0, nextPlane=0, geomIndex=0`，`charge = Q*e`，`mass = A*m_p`，`qOverM` 同上，
    `restMassMeV = mc2`，`kind`，`boundaryMode`，`weight = 1.0`。
15. 纵向：若 `this+0x140 == 3`（恒成立）且 twiss[6] ≠ −2027.041：`twZ = [ε_z, β_z, α_z, ε_z, β_z, α_z, 0, 0, 0]`，
    按 distribution[1] 同样分派 `YY2D(twZ, rows, (int)seed)`（**rows 被整体覆盖**），然后 `P[i].z = rows[i][0]`，`P[i].gbz = rows[i][1]`（暂存 δ）。
    probe 时 `rows2[i][4..5] = rows[i][0..1]`。
16. 换算为 .dst 单位（逐粒子，i 递增）：
    - DC（twiss[6] == −2027.041）：见 §2.4.4。
    - 否则：
      ```
      pz  = P.gbz * βγ₀ + βγ₀                    // γβ_z = βγ₀(1+δ)
      gx  = P.gbx * pz ;  gy = P.gby * pz         // 只用于算 γ，不写回
      γ   = sqrt(((gx*gx + 1.0) + gy*gy) + pz*pz)
      β   = sqrt(1.0 - (1.0/γ)/γ)
      vz  = sqrt((((β*c)*β)*c) / ((x'*x' + y'*y') + 1.0))     // x'=P.gbx, y'=P.gby
      P.x *= 100.0 ; P.y *= 100.0                              // m → cm
      P.gbz = (γ - 1.0) * mc2                                  // W，MeV
      a = P.z / vz ;  P.z = ((-a) + (-a)) * π * f              // φ，rad；z>0（在前）对应 φ<0
      ```
      P.gbx、P.gby 仍是 x'、y'。注意 W 中包含横向动量。
17. 束流误差平移（§2.5.1），仅当 N > 0。
18. 写 `outputFilePath + "inData.dst"`（§2.4.6 的格式），Ib = `this.current*1000`（mA，含误差与第 5 步清零），f = `beam.frequency/1e6`（MHz），
    粒子 = P[0..beam.n−1] 的前 6 个 double，末尾 = mc2。打不开 → 抛 `"Insufficient permissions to write file: inData.dst"`。
19. 重新打开同一文件读回（打不开 → 抛 `"The initial bunch distribution file cannot be opened, check particleDistributionPath"`），
    `this.particleNumber` 被文件中的 N 覆盖；按 §2.4.6 的“读回换算”得到 P[i] 的最终坐标、`beam.bunchTime`、`beam.tMin`。
20. 同步粒子坐标（用**读回粒子的平均能量**，不是 kneticenergy）：
    `W̄ = (ΣW_i)/N`（按 i 递增累加，不加权）；γ̄ = W̄/mc2 + 1；β̄ = sqrt(1 − (1/γ̄)/γ̄)；
    `P[N].x = gbx = y = gby = 0`；`P[N].gbz = γ̄*β̄`；`P[N].z = (β̄*c)*beam.tMin`。
21. probe：网格粒子覆盖并重写 inData.dst（§2.4.5）。
22. `P[N]` 的 scE、scB、rel[0..2]（+0x60…+0x88、+0xc0…+0xd0）置 0（extE、extB、totE、totB 未置 0）。
    打印 `Synchron particle addition completed`、`Bunch initialisation complete`、`  `。
23. §2.5.4 收尾。

`this.kineticEnergy` 在生成路径中保持 beam.txt 的值；同步粒子能量 W̄ 一般与它略有差别（δ 的样本均值不为 0，且 W 含横向动量）。

#### 2.4.2 生成路径中各 beam 字段

`frequency`(Hz)、`n`、`current`(A)、`scSolverRecord`、`realPerMacro`（I≠0 时）、`particles`、`totalWeight = N`、`bunchTime`、`tMin`、`recScField`、`impScField`。
不写：`useWeightFile`、`fromDstFile`、`seStep`（保留 main_agent 初值 0.01）。

#### 2.4.3 单宏粒子（particlenumber 1）

在 §2.4.1 第 1–8 步之后（第 5 步已把流强、空间电荷清零，第 8 步的 cores 改为 10 个 1）：

- 对 P[0] 与 P[1]（=同步粒子）都执行：`x=y=gbx=gby=z=0`，`gbz = βγ₀`，`elemIndex=lost=nextPlane=0`，`charge=Q*e`，`mass=A*m_p`，`qOverM`，
  `restMassMeV = mc2`，各种场数组（+0x30…+0xd0）置 0。`kind`、`boundaryMode`、`weight=1.0`、`geomIndex=0` 只写到 P[1]
  （伪代码按 `beam.n` 下标写，**P[0] 的 weight、kind 未初始化**）。
- P[0]：`x = displacePos[0]`，`y = displacePos[1]`，`z = displacePos[2]`（m，来自 mm×0.001）；
  `gbx = displacedPos[0]*0.01*βγ₀`，`gby = displacedPos[1]*0.01*βγ₀`，`gbz = (displacedPos[2]*0.01 + 1.0)*βγ₀`。
  displacedPos 已在 ReadBeam ×0.001，所以**输入值 v 对应 x' = v×1e-5 rad、δ = v×1e-5**（schema 写 %，应为 0.001 %；需实验确认，§5 Q6）。
- `beam.tMin = beam.bunchTime = 0`；`beam.zMin = beam.zMax = displacePos[2]`；`beam.totalWeight = 1`；P[1] 的 scE、scB 置 0。
- 写 `outputFilePath + "SingleParticle.txt"`（fstream，ios::out，默认 6 位有效数字 `%g` 风格）：
  ```
  dx:<x*1000>mm  dy:<y*1000>mm  dz:<z*1000>mm
  dpx:<(gbx/gbz)*1000>mrad  dpy:<(gby/gbz)*1000>mrad  dpz:--
  Energy:<W₀>MeV
  ```
- 打印 `Synchron particle addition completed`、`Bunch initialisation complete`、`  `，`return 0`。不写 inData.dst，不应用束流误差，不写 spOutZ0/Z1。

> **默认模式（M10 D2）**：`initpos`／`initmom` 写了时优先（x′ = initmom[0]·1e-3，γβz = (1 + initmom[2]·1e-3)·γβ0；displacedpos 的有效系数是 1e-5），否则用 displacepos／displacedpos。

#### 2.4.4 DC 束（beamtype dc，或不写 twissz）

第 15 步不生成纵向；第 16 步改为（i = 0..N−1，N = beam.n）：

```
P.gbz = W₀ ;  P.x *= 100 ;  P.y *= 100
P.z   = π − (double)i * (2π / (double)N)        // φ 从 π 等间隔递减，不随机
```

所有粒子能量相同（.so）。之后同样写 inData.dst、读回。读回时 φ ∈ (−π, π]，tMin = φ_min/(2πf)（φ_min 与 0 取小）。

#### 2.4.5 probe 模式（input.txt `probe 1`）

- σx、σy（第 14 步，用横向 rows，单位 m，换算前）：`m = (Σ rows[i][0] / n) / n`（**除了两次 n**，且 y 也用这个 m，原样复制），
  `σx = sqrt(Σ(rows[i][0]−m)² / n)`，`σy = sqrt(Σ(rows[i][2]−m)² / n)`，n = rows 行数（= particleNumber）。
- σz（第 21 步，用纵向生成后的 rows[i][0]，DC 时是横向 x）：`m = Σ/n`，`σz = sqrt(Σ(rows[i][0]−m)²/n)`。
- 网格：对 a = −2..2（外层）、b = −2..2（中层）、c ∈ {−2, −1, 0, 1, 2}（内层），按顺序写 P[25(a+2)+5(b+2)+(c+2)]：
  `x = a*σx`，`y = b*σy`，`z = c*σz`（c=0 时为 `σz*0.0`），`gbx = gby = 0`，`gbz = βγ₀`，`charge = 0`（其余字段沿用第 13、16、19 步的值）。
- 另算三个 6 元数组 `−2.5σ + kσ`（k=0..5），随即释放，无作用。
- 重写 inData.dst：表头 Ib、f 用第 19 步读回的值，末尾 mc2 用读回值；每个网格粒子写
  `[x*100, 0, y*100, 0, ((−(z/vz)) + (−(z/vz)))*π*f, (γ−1)*mc2]`，其中 γ = sqrt(gbz²+1)、β = sqrt(1−(1/γ)/γ)、vz = sqrt((β c β c)/(0+0+1))。
- 同步粒子仍按第 20 步（用第 19 步读回的普通束团）。

#### 2.4.6 inData.dst 格式与“读回换算”

**文件格式**（小端，无填充；与 TraceWin `.dst` 相同）：

| 偏移 | 长度 | 内容 |
|---|---|---|
| 0 | 2 字节 | 生成路径写 `0x7D 0x64`；读文件路径原样复制输入文件的这两个字节 |
| 2 | int32 | 粒子数 N |
| 6 | double | 流强 Ib（mA） |
| 14 | double | 频率 f（MHz） |
| 22 | 1 字节 | 生成路径写 `0x7D`；读文件路径原样复制 |
| 23 | N×6 double | 每粒子 x(cm)、x'(rad)、y(cm)、y'(rad)、φ(rad)、W(MeV) |
| 23+48N | double | 静止能量 mc²（MeV） |

**读回换算**（生成路径第 19 步与 .dst 路径共用；.edst 路径用同一公式但 mc² 取每粒子的值）：

```
φ_max = max(0, φ_i)，φ_min = min(0, φ_i)            // 初值都是 0.0，按 i 递增比较
beam.bunchTime = (φ_max − φ_min) / (f*2π)            // f 为 Hz，(f*2π) 先算
beam.tMin      =  φ_min / (f*2π)                     // ≤ 0
对每个粒子：
  γ  = W/mc² + 1.0 ;  β = sqrt(1.0 − (1.0/γ)/γ)
  vz = sqrt((((β*c)*β)*c) / ((x'*x' + y'*y') + 1.0))
  t  = φ/(f*2π) − beam.tMin                           // ≥ 0
  z  = (−t) * vz                                      // ≤ 0
  pz = (vz/c) * γ
  gbx = x' * pz ;  gby = pz * y'
  x  = x_cm/100.0 − (x'*t)*vz ;  y = y_cm/100.0 − (t*y')*vz     // 漂移到同一时刻
  extE、extB、scE、scB、totE、totB、rel[0..2]（+0x30…+0xd0）置 0
```

含义：文件给出的是各粒子穿过 z = 0 平面时的状态（不同时刻），内核把它们换成同一时刻（最早到达者的时刻 tMin）的快照，
并让 |p| = βγ（横向分量从总动量中分出）。注意生成路径 `x*100` 写出、`/100.0` 读回，不保证逐位还原原值。

#### 2.4.7 `.dst` 路径

1. 打印 `Start reading initial beam distribution...`；读表头（打印 `total number of particles `、`current intensity …(mA)`、`beam frequency …(MHz)`）。
2. §2.5.2 默认参数（N 取文件值）。
3. 读 N×6 double 与 mc²（打印 `Particle rest mass: … (MeV)`）。
4. `beam.n = N`，`beam.totalWeight = N`，`new Particle[N+1]`。
5. 权重：`indWeight == 1` 时 `beam.useWeightFile = 1`，读 `inputFilePath + distFile + "b"`（如 `xxx.dstb`；打不开抛
   `"The initial bunch Weight file cannot be opened, check particleDistributionPath"`）：int32 粒子数（写入 this.particleNumber，
   ≠ beam.n 抛 `"The initial bunch Weight file particle number error"`），再 N 个 double 依次写 `P[i].weight`，`beam.totalWeight = Σw`
   （从 0 开始按 i 累加），打印 `Particle weight loaded.`。否则 `P[i].weight = 1.0`。
6. 束流误差（`flags[0]+flags[1] != 0`，`e = dyn[0]`）：`Ib += e[13]`；每粒子 `x += e[1]*0.1`，`x' += e[4]*0.001`，`y += e[2]*0.1`，
   `y' += e[5]*0.001`，`φ += (e[3]/180)*π`，`W += e[6]`（此时仍是 dst 单位）。
7. 写 inData.dst：原 2 字节头、N、Ib（含误差）、f、原 1 字节、修改后的粒子、mc²。打不开抛 `"Insufficient permissions to write file: inData.dst"`。
8. `this.atomicMass = mc²/938.271875`；打印 `atomic mass <A> electric charge of particles <Q>`。
9. `W̄ = Σ(W_i * w_i) / beam.totalWeight`（**按权重**）→ `this.kineticEnergy`，打印 `Average beam energy: … (MeV)`；γ̄、β̄ 由 W̄ 与文件 mc² 算。
10. `beam.frequency = this.frequency = f_MHz*1e6`；`this.current = beam.current = Ib/1000`；`beam.scSolverRecord`；Ib == 0 → `this.spaceCharge = 0`，打印 `0 current, ignoring space charge effects`。
11. `rpm`：Ib == 0 → 1.0；否则 `|(((I/f)/Σw)/e)/Q|`，写 `beam.realPerMacro`，打印 `The number of macroparticle representative particles is `。
12. 每粒子：坐标 = 文件值（含误差），`elemIndex=lost=nextPlane=geomIndex=0`，`charge = ((rpm*w)*Q)*e`，`mass = ((rpm*w)*A)*m_p`，
    `qOverM = ((Q/A)*e)/m_p`，`restMassMeV = 文件 mc²`，`kind`，`boundaryMode`。同步粒子：`charge=Q*e`，`mass=A*m_p`，`weight=1.0`，其余同上。
13. bunchTime、tMin 与读回换算（`this+0x130 == 1`，恒成立）；同步粒子 `x=y=gbx=gby=0`，`gbz = γ̄*β̄`，`z = (β̄*c)*tMin`，scE、scB、rel[0..2] 置 0。
    打印 `Synchronised particles have been added`、`Beam initialisation complete`。（`this+0x130 ≠ 1` 时不换算，打印 `zCode only` 或
    `Inlet particle distribution is not appropriate`；不可达。）
14. 不设 `beam.fromDstFile`（structures.md 的说法需更正，§6）。

> **默认模式（M10 D1，2026-11-16 与用户商定）**：第 9 步仍打印平均能量 W̄，但 beam.txt `kneticenergy` > 0 时同步粒子能量取 kneticenergy（打印 “Synchronous particle energy (beam.txt kneticenergy)”），与 W̄ 相差超过 1 % 时警告；≤ 0 或未写（默认 −1）时取 W̄（07-22 版此时取按权重平均的纵向动量，不跟）。单粒子模式（singleparticlemode 1）的 .dst 分支同样。`.edst` 仍取同步记录。兼容模式总取 W̄。

#### 2.4.8 `.edst` 路径（扩展格式，带每粒子电荷、质量、权重）

文件格式：2 字节、int32 N、double Ib(mA)、double f(MHz)、1 字节，随后 **N+1 条**记录，每条 9 个 double：
`x(cm), x'(rad), y(cm), y'(rad), φ(rad), W(MeV), Q, mc²(MeV), w`，最后一条是同步粒子；末尾 double mc²。

- `beam.fromDstFile = 1`，`beam.useWeightFile = 1`，`beam.n = N`，`new Particle[N+1]`。
- `P[i].weight = w_i`（i = 0..N），`beam.totalWeight = Σ_{i=0..N} w_i − w_N`。
- 不应用束流误差，不写 inData.dst，不设 `beam.realPerMacro`。
- `this.kineticEnergy = W_N`（同步记录的 W），打印 `Average beam energy: `；β 用末尾 mc²。
- 每粒子（i = 0..N）：坐标 = 记录前 6 个，`charge = (Q_i*w_i)*e`，`mass = ((w_i*mc²_i)/938.271875)*m_p`，`qOverM = charge/mass`，`restMassMeV = mc²_i`，`kind`，`boundaryMode`。
- φ 的 min/max 包括同步记录（i = 0..N），初值 0。
- 读回换算（γ 用每粒子 `restMassMeV`），i = 0..N−1。同步粒子：`x=y=gbx=gby=0`，`z = (β*c)*tMin`，`gbz = γβ`（由 W_N 与末尾 mc²），
  scE、scB、rel 清 0；`this.charge = Q_N`，`this.atomicMass = mc²_N`（**写的是 MeV 值而不是 mc²/938.271875**，疑似缺陷）。

#### 2.4.9 二次束（secondarybeam 1）读 `.dst`

与 §2.4.7 相同的文件格式，差异：

- 束流误差的 x、y 平移用 `e[1]*0.001`、`e[2]*0.001`（cm 单位下按 μm？与普通 .dst 的 0.1 不一致，疑似缺陷），其余同 §2.4.7。
- inData.dst 打不开时只打印 `Initial Particle Distribution Not Open`，继续执行。
- `numOfGrid = 3,3,3`，cores 按 §2.5.2；`beam.seStep = this.seStep`（steppercycle V2）。
- 打印 `Synchronous particle energy <this.kineticEnergy>(MeV)`。
- `rpm = |(((I/f)/N)/e)/Q|`（N，不是 Σw；只打印，不写 beam.realPerMacro）。
- 每粒子：`charge = (Q*rpm)*e`，`mass = (A*rpm)*m_p`，`qOverM = charge/mass`，**`weight = rpm`**，`spOutIndex = 0`，`restMassMeV = 文件 mc²`。同步粒子同样。
- `beam.bunchTime = (φ_max − φ_min)/(2πf)`（min/max 初值 0），**`beam.tMin = (Σφ_i/N)/(2πf)`（平均值，不是最小值）**。
- 每粒子换算：γ、β、vz 同读回换算；`t = φ/(2πf) − beam.tMin` 写入 `P.t`；`P.z = 0`；`P.dt = seStep/vz`；`P.id = i`；
  `pz = (vz/c)*γ`；`x = x_cm/100`、`y = y_cm/100`（**不做漂移修正**）；`gbx = x'*pz`；`gby = pz*y'`。
- 同步粒子：γβ 由 `this.kineticEnergy`（beam.txt）与文件 mc² 计算，`x..z = 0`，`t = 0`，`dt = seStep/(β*c)`，`id = N`。

#### 2.4.10 二次束文本文件（secondarybeam 1，非 .dst）

- 打印 `Start reading secondary particle distribution...`；以文本方式打开 `inputFilePath + distFile`（`is_open` 失败抛同一条“cannot be opened”）。
- 逐行 `getline`（直到 `peek() == EOF`），`StringSplit` 拆词；前 10 个词 `stod`（不是数字抛 `std::invalid_argument("stod")`，溢出抛
  `std::out_of_range("stod")`），第 11 个词作为字符串保存。每行至少 11 个词，空行会越界（未定义行为）。
- 列：`x(m) y(m) z(m) vx vy vz(m/s) Q mc²(MeV) w t(s) kind`。粒子数 = 行数，写 `this.particleNumber`、`beam.n`。
- `numOfGrid = 3,3,3`；cores 默认（§2.5.2）；`beam.frequency = 0`，`beam.current = 0`，`beam.seStep = this.seStep`，`beam.scSolverRecord`；
  打印 `Synchronous particle energy <W₀>(MeV)`。
- 每粒子：`|v| = sqrt((vy² + vx²) + vz²)`；`β = |v|/c`；`γ = 1/sqrt(1 − β*β)`；
  `x,y,z = 列 0,1,2`；`gbx = (vx*γ)/c`，`gby = (vy*γ)/c`，`gbz = (γ*vz)/c`；`t = 列 9`；`id = i`；`dt = (seStep/β)/c`；
  `weight = w`；`charge = (w*Q)*e`；`mass = ((w*mc²)*m_p)/938.271875`；`qOverM = charge/mass`；`restMassMeV = mc²`；
  `kind`（伪代码中赋值来源丢失，推测为第 11 列）；`boundaryMode`；`elemIndex=lost=nextPlane=spOutIndex=geomIndex=0`；场数组置 0。
- 同步粒子：W₀、mc2 取 beam.txt；`gbz = γ₀β₀`，`x..z = 0`，`t = 0`，`dt = seStep/(β₀*c)`，`id = N`，`charge = Q*e`，
  `mass = (mc2*m_p)/938.271875`，`qOverM = charge/mass`，`weight = 1.0`，`restMassMeV = mc2`。
- `beam.tMin = beam.bunchTime = 0`。

### 2.5 共用部分

#### 2.5.1 束流误差（ErrorSample 第 0 行）

只在 `flags[0] + flags[1] != 0`（束流静态或动态误差开启）时生效，且**总是取 `dyn`（ErrorSample+0x18）的第 0 行**，不看 `stat`。
列含义（与 schema 的 BEAM_ERRORS 对应）：[1] dx(mm)、[2] dy(mm)、[3] dφ(°)、[4] dx'(mrad)、[5] dy'(mrad)、[6] dW(MeV)、[7–9] ε_x,y,z 相对变化(%)、
[10–12] 失配（β、α 乘 (1+e/100)²）、[13] dIb(mA)；[0] 在生成路径中作为“Twiss 误差开关”（必须恰好等于 1.0）。

| 路径 | Twiss/流强（§2.4.1 第 4 步） | 坐标平移 |
|---|---|---|
| 生成 | 仅 e[0]==1.0 时 | 换算到 dst 单位后（第 17 步）：x += e1*0.1，x' += e4*0.001，y += e2*0.1，y' += e5*0.001，φ += (e3/180)π，W += e6；DC 束同样 |
| .dst | Ib += e13（mA） | 同上（读入后、写 inData.dst 前） |
| 二次束 .dst | Ib += e13 | x、y 用 e*0.001（见 §2.4.9） |
| .edst、文本、单粒子 | 不应用 | 不应用 |

#### 2.5.2 空间电荷与线程默认值（在 InitParticle 中填写，F 子系统使用）

N 为当时的粒子数（生成路径 = particleNumber；文件路径 = 文件中的 N）。各读文件路径与生成路径的写法相同，二次束两条路径只执行第 3、4 项且 numOfGrid 固定 3,3,3。

1. `spaceCharge == 1` 时：
   - `scPar[0] == 0`（FFT）：`scPar[1] == −1` → `scPar[1..3] = 6.5`；`numOfGrid[0] == −1` → `numOfGrid = 64,64,64`。
   - `scPar[0] == 1`（SPICNIC）：
     - `scPar[1] == −1` → `scPar[1..3]` = 3.5（N ≤ 8e4）、4.0（≤ 8e5）、5.0（≤ 8e6）、6.0（≤ 8e7）、7.0（> 8e7）；
     - `scPar[4] == −1` → `scPar[4..6] = 8.0`；
     - `numOfGrid[0] == −1` → 三个分量都取 14（N ≤ 3e4）、18（≤ 8e4）、20（≤ 3e5）、26（≤ 8e5）、30（≤ 3e6）、40（≤ 8e6）、44（≤ 3e7）、56（≤ 8e7）、64（> 8e7）；
     - `scPar[7] == −1` → `scPar[7] = ceil(numOfGrid[0]/3.0)`。
   - `scPar[9] = (double)multithreading`。
2. `spaceCharge != 1`：`numOfGrid = 3,3,3`。
3. `multithreading == 1` 且 `cores[0] == −1`：cores 取 10 个整数的表（N 的分档与上面相同的 30000/80000/3e5/8e5/3e6/8e6/3e7/8e7/3e8）：
   ```
   N ≤ 3e4  : 64 54 20 24  1 48 96 12 48 12
   N ≤ 8e4  : 72 68 24 56 22 48 96 20 48 24
   N ≤ 3e5  : 96 96 32 96 26 48 96 24 48 36
   N ≤ 8e5  : 96 96 52 96 74 48 96 48 48 96
   N ≤ 3e6  : 96 96 62 96 86 48 96 48 48 96
   N ≤ 8e6  : 96 96 82 96 92 48 96 48 48 96
   N ≤ 3e7  : 96 96 92 96 96 48 96 48 48 96
   N > 3e7  : 96 96 96 96 96 48 96 48 48 96
   ```
   生成路径 particleNumber == 1 时随后改为 10 个 1。
4. 对 cores 的每一项 k：先截到 `ompMaxThreads`，再若 `maxThreads > 0` 截到 `maxThreads`；`scPar[10+k] = cores[k]`。

#### 2.5.3 控制台输出

所有提示都写 `std::cout` 并 `endl`，数值用 `ostream<<double` 默认格式（6 位有效数字）。文字原文见各节（`Starting to generate initial bunch` 等）。
.so 的 `Particle weight loaded.` 输出流异常（伪代码把字符串当作 ostream 传给 `operator<<`，推测实际仍输出到 cout）。

#### 2.5.4 收尾（所有路径，单粒子除外）

`lattice.spOut[lattice.spOutCur]` 这一行的第 [6] 个元素（下一个 superposeout 出口 z）记为 Z；对 i = 0..N（含同步粒子）：
`P[i].spOutZ0 = 0`，`P[i].spOutZ1 = Z`。依赖 `lattice.spOut` 非空（没有 superposeout 时由 InitLattice 保证有一行，推测，属 A/D）。
函数返回 0。

#### 2.5.5 Particle 字段初值汇总（生成路径，读回之后）

| 字段 | 普通粒子 | 同步粒子 P[N] |
|---|---|---|
| x, y | m，漂移到 tMin 时刻 | 0 |
| gbx, gby | x'·pz、y'·pz | 0 |
| z | −t·vz ≤ 0 | β̄·c·tMin |
| gbz | (vz/c)·γ | γ̄·β̄（W̄ = 粒子平均 W） |
| extE…rel[0..2] | 0 | 只有 scE、scB、rel[0..2] 置 0 |
| elemIndex, lost, nextPlane, geomIndex | 0 | 0 |
| charge / mass / qOverM | rpm·Q·e / rpm·A·m_p / (Q/A)e/m_p | Q·e / A·m_p / 同左 |
| restMassMeV, kind, boundaryMode | mc2, kind, boundary | 同左 |
| weight | 1.0 | 1.0 |
| spOutZ0 / spOutZ1 | 0 / Z | 0 / Z |
| t, dt, id, spOutIndex | 未写 | 未写 |

### 2.6 AVAS.dll 的差异（Windows 用户实际运行的版本）

.dll 的 `InitParticle` 是 `FUN_180027eb0`，生成器 KV2D/PB2D/WB2D/GS2D = `FUN_1800106e0`/`FUN_1800107a0`/`FUN_180010c10`/`FUN_180010cf0`，
GenerateCoordinates = `FUN_180011070`，PreTreatment = `FUN_18000fee0`，GenerateGaussian = `FUN_18001e1a0`，ReadBeam = `FUN_180027060`。
整体流程、分支、提示文字与 .so 相同，已确认的差异：

1. **随机数**：MSVC `rand()`（§2.1.1），所有 `/2147483647.0` 换成 `*3.051850947599719e-05`；GenerateCoordinates 的角度是 `rand()*0.00019175345033660654`（一次乘法），
   用 `cos`、`sin` 分别计算；PB2D 中 `U*U` 写成 `k*k*9.313794206365304e-10`（=k²/32767²）。时间种子用 `_time64`。
2. **beam.txt `kneticenergy W₀ [ΔW]`**：.dll 接受可选的第二个数 ΔW（Initial +0xd8）。**DC 束的能量 = (rand()/32767 − 0.5)·ΔW + W₀**
   （每粒子在相位赋值之后抽一个数）；.so 没有 ΔW，DC 束能量恒为 W₀。
3. **生成后换算（第 16 步）**：.dll 为
   `pz = (δ + 1.0)·βγ₀`；`γ = sqrt(pz² + 1)`（**不含横向动量**）；`W = (γ − 1)·mc2`；`φ = ((−2.0958450219516816e-08 / sqrt(1 − (1/γ)²))·z)·f`
   （= −2πf·z/(βc)，**用总速度而不是 vz**）。
4. **读回换算**：.dll 用 `vz = β·c`（**不除以 √(1+x'²+y'²)**），`pz = β·γ`，`gbx = pz·x'`，因此 |p| 略大于 βγ；
   `t = (0.15915494309189535/f)·φ − tMin`，`tMin = φ_min·(0.159…/f)`；`1/γ` 先求倒数再平方。
5. **常数折叠**：`A = mc2·0.0010657891669192364`，`I·0.001`，`ε·1e-06`，`qOverM = (Q/A)·95788337.57933226`，`rpm = |((I/(N·f))·6.241509479607718e+18)/Q|`、
   ε_z 换算 `(ε_z·1e-6)/(β·γ·γ·γ)` 等，舍入与 .so 不同。
6. **PB2D**：MSVC 的复数 `sqrt/pow`（`atan2`、`pow`、Smith 法除法，只算实部），加法次序为 `((c.re − c.im·√3)·0.25 + d.re) + 0.5`，再乘 `8ε`（`8ε` 预先算好，与 .so 相同）。
   w 的虚部恰为 −0 时取共轭分支（.so 无此分支）；U≈1 时两版本可能给出不同的 s（§5 Q5）。
7. **结构布局**：Particle 步长 0x1a8（MSVC `std::string` 32 字节，`kind` 位于 0x188），其余粒子字段偏移与 .so 相同；
   Initial 的偏移不同（本文用到的：distFile +0x08，distribution +0x38，particleNumber +0x50，twiss +0x58，displacePos +0x70，displacedPos +0x88，
   charge +0x28，atomicMass +0x30，restMass +0xb8，current +0xc0，frequency +0xc8，W₀ +0xd0，ΔW +0xd8，randomSeed +0xe0，kind +0x100，
   spaceCharge +0x158，scPar +0x160，cores +0xa0，numOfGrid +0x210，入口换算开关 +0x178，维数 +0x188，boundary +0x18c，probe +0x194，
   scSolverRecord +0x20c，maxThreads +0x234，ompMaxThreads +0x238；由用法推定）。
8. GS2D、KV2D、WB2D 的算法与 .so 相同（只差 RAND_MAX 与 libm）；横纵重播种（§2.1.3）同样存在。
9. 未逐行核对：.dll 的 `.edst`、二次束两条路径、probe 网格写文件的细节（结构与字符串一致）。

新内核应提供“.so 兼容”与“.dll 兼容”两种模式，或至少明确以哪一个为基准；两者不可能同时逐位一致。

---

## 3. 对齐清单

| ID | 行为 | 涉及关键字/文件 | 建议的比对测试（输入、比较量、容差） |
|---|---|---|---|
| E-01 | glibc `rand()` 序列 | — | 自实现 TYPE_3 与 Linux `rand()` 对比 seed 1、12345、0、2^31+5 的前 10⁶ 个输出，逐位相同 |
| E-02 | MSVC `rand()` 序列 | — | 自实现 LCG 与 Windows UCRT 对比同上，逐位相同 |
| E-03 | 种子优先级 | beam.txt / input.txt randomseed | 两文件写不同种子（KV GS），比较 inData.dst 与只写 beam.txt 种子时逐字节相同；只写 input.txt 时与该种子相同 |
| E-04 | 种子截断 | randomseed 12345.9 | 与 12345 生成的 inData.dst 逐字节相同 |
| E-05 | seed 0 不可复现 | randomseed 0 | 隔 >1 s 运行两次，inData.dst 不同；同一秒内两次相同 |
| E-06 | 横纵重播种导致相关 | distribution KV KV / GS GS，seed 12345 | inData.dst 中 corr(x, φ) 与 −1 相差 < 1e-3；x/√β_xε_x 与 z 归一化值逐粒子相等（z 由 φ 反算，相对 1e-6） |
| E-07 | 不同分布类型也共用序列 | distribution GS KV，seed 固定 | 新内核按 §2.1.2 复现 inData.dst，逐位（.so）|
| E-08 | KV 超曲面 | KV KV，N=10⁴ | 由 inData.dst 反算 J_x/ε_x + J_y/ε_y = 4，相对 1e-9（x 取 cm/100，ε 用几何值） |
| E-09 | WB 径向分布 | WB | R²/(6ε) 的经验 CDF 与 s² 的 KS 距离 < 0.02（N=10⁴）；max ≤ 1 |
| E-10 | PB 径向分布 | PB | R²/(8ε) 的 CDF 与 3s²−2s³，KS < 0.02；max ≤ 1 |
| E-11 | GS 不截断 | GS，N=10⁵ | max|x|/σ_x > 4；σ_x/√(βε) 相对 1 % 内 |
| E-12 | 横向 Twiss 与发射度换算 | twissx α β ε，W₀ | 由生成坐标（读回前，或 inData.dst 的 x、x'）算 rms 几何发射度 ≈ ε·1e-6/βγ₀（1 %），α、β 同 |
| E-13 | 纵向换算 | twissz | inData.dst 中 φ、W 的 rms 与 ε_z,n/(βγ³)、β_z 的关系（1 %）；逐粒子 W = (γ−1)mc²，γ 含横向动量（.so 逐位） |
| E-14 | φ 符号 | — | z>0 的粒子 φ<0：对 inData.dst 检查 φ 与重算的 z 反号 |
| E-15 | DC 束相位 | beamtype dc | inData.dst 第 i 个 φ = π − i·2π/N（逐位）；.so W 全等于 W₀；.dll 在 W₀±ΔW/2 内均匀 |
| E-16 | 不写 twissz 视为 DC | 删去 twissz | 结果同 E-15 |
| E-17 | inData.dst 格式 | — | 头 2 字节 7D 64、第 22 字节 7D、Ib(mA)=current（I=0 或 N=1 时为 0）、f(MHz)、末尾 mc²；文件长度 31+48N |
| E-18 | 同步粒子能量 = 粒子平均 W | 生成路径 | synData.txt / DataSet 同步粒子起始能量与 mean(W in inData.dst) 相对 1e-12；与 kneticenergy 不同 |
| E-19 | 束团时间范围 | — | 由 inData.dst 算 φ_min=min(0,φ)、φ_max=max(0,φ)，与内核使用的 tMin、bunchTime（经同步粒子初始 z = β̄c·tMin 间接）比较 |
| E-20 | 读回换算 | .dst 输入 | 用已知 .dst（含 x'、φ 大的粒子）运行 1 步，比较第一行 DataSet 的质心、rms 与按 §2.4.6 自算的值（1e-6） |
| E-21 | rpm 与宏粒子电荷 | current、frequency、particlenumber、numofcharge | stdout 的 `The number of particles represented…` 与 |I/f/N/e/Q|（6 位有效数字） |
| E-22 | 单粒子模式 | particlenumber 1，displacepos/displacedpos | SingleParticle.txt 三行文字；dx 等于输入值（mm）；dpx = v·1e-5/(1+v_z·1e-5)·1000 mrad |
| E-23 | probe 网格 | input.txt probe 1 | inData.dst N=125，x/σx、y/σy、φ 对应 z/σz 取 −2..2 的组合，次序 x 外、y 中、z 内；σx 用双重除 n 的均值 |
| E-24 | .dst 复制与误差 | readparticledistribution x.dst + 束流误差 | inData.dst = 输入加 §2.5.1 平移（逐位），Ib += e13 |
| E-25 | 权重文件 | indweight 1，x.dstb | 平均能量按权重（stdout），粒子数不符时报错原文 |
| E-26 | .edst | x.edst | 运行不报错；stdout 平均能量 = 同步记录 W；不生成 inData.dst（或保留旧文件） |
| E-27 | 二次束文本 | secondarybeam 1，文本 11 列 | 粒子初始 γβ = γv/c；dt = seStep/(βc)（通过 SePlaneOut 首行或调试输出） |
| E-28 | 二次束 .dst | secondarybeam 1，x.dst | tMin 为平均相位时间；粒子 z=0，权重 = rpm |
| E-29 | Twiss 误差 | 误差开关，dyn[0][0]=1 | 与手工把 ε、β、α 按 §2.5.1 修改后无误差运行的 inData.dst 逐位相同 |
| E-30 | 空间电荷默认参数 | scmethod、meshrms、numofgrid 缺省，N 跨越分档 | .dll 日志中的网格数（`numofgridy:` 等）或运行时间；与 §2.5.2 表比较 |
| E-31 | 线程数默认 | setnumberofcores 缺省、maxthreads | 同上，或在新内核内部断言 |
| E-32 | 报错信息 | 错误扩展名、文件缺失、distribution 写错、twissx 少参数 | 异常文字逐字相同 |
| E-33 | GenerateCoordinates NaN 保护 | 构造 rand()==RAND_MAX 的种子 | 该粒子 y=x、y'=x'（.dll 中容易找到） |
| E-34 | GS2D 互异重抽 | .dll，GS | 新内核实现/不实现互异检查，比较 inData.dst，必须实现才逐位一致 |
| E-35 | GenerateGaussian 成对缓存 | 误差研究 | 与 G 子系统联合：误差样本序列逐位 |
| E-36 | Windows 量化 | .dll，KV | inData.dst 反算 r²/(4ε_x) 只取 k/32767 的值（相对 1e-9） |

---

## 4. 函数覆盖表

| 函数 | 作用一句话 | 在正文哪节 |
|---|---|---|
| `rand_gen` | (rand()+1)/2³¹ 均匀数；.so 中无调用者 | §2.1.5 |
| `normalRandom` | 用 rand_gen 的 Box–Muller（只取 cos 支）；无调用者 | §2.1.5 |
| `GenerateGaussian` | 成对 Box–Muller、静态缓存的高斯数，供 GenerateErrorSet | §2.1.6 |
| `Gaussu` | 由 8 个均匀数按极坐标法给出 4 个高斯数，任一对落在单位圆外则整体作废 | §2.2.3（GS2D） |
| `GenerateCoordinates` | 给定 R²_max，在 4 维超椭球面上抽一个点（3 次 rand） | §2.2.2 |
| `KV2D` | K-V 分布（R² = 4ε） | §2.2.3 |
| `WB2D` | 水袋分布（R² = 6ε√U） | §2.2.3 |
| `PB2D` | 4 维抛物线分布（R² = 8ε·s，三次方程用复数求根） | §2.2.3 |
| `GS2D` | 不截断 4 维高斯 | §2.2.3 |
| `Initial::InitParticle` | 读 beam.txt、生成或读入初始宏粒子，写 inData.dst，填 Beam/Particle 与空间电荷默认参数 | §2.3–§2.5 |

（相关但不在本清单：`PreTreatment` → A；`Initial::ReadBeam` → A；`MethodMP::dstOut` → D；`GenerateErrorSet` → G。）

---

## 5. 未解决的问题与建议实验

- **Q1 横纵相关**（§2.1.3）：在 Linux 或 Windows 用示例项目 hwr010，beam.txt 加 `randomseed 12345`、`distribution GS GS`，运行后读
  `OutputFile/inData.dst`，算 x 与 φ 的相关系数；预期 ≈ −1。再用 `randomseed 0` 对照（预期 ≈ 0）。若确认，研究层默认种子 12345 下的全部内核结果都带此关联，
  需要在 AGENTS.md 第 6 节记录并决定新内核是否“修复”（修复会破坏兼容）。
- **Q2 libm 逐位**：`log`、`sincos`、`csqrt`、`clog`、`exp`、`pow`、`sqrt` 以及 libgcc `__divdc3`/`__muldc3` 的结果与运行时 glibc 版本有关（`sqrt` 正确舍入无问题）。
  需记录原内核实际运行的 glibc 版本，或在新内核中内置这些函数的实现；.dll 同理（UCRT 版本）。建议：用 E-08/E-10 在两平台比对 inData.dst，统计不一致的粒子比例。
- **Q3 .dll 的其余路径**：`.edst`、二次束、probe 在 .dll 中未逐行比对；建议补读 `FUN_180027eb0` 第 500–1460、2860–4385 行。
- **Q4 PreTreatment 的 .dll 版本**（A 负责）是否同样走 Eigen 特征分解、舍入是否相同；σ11 == σ22 缺陷是否两版都有：用 `twissx 1 1.4142135623730951 1`（β²=1+α²）
  生成，检查 inData.dst 的 rms α、β 是否偏离输入。
- **Q5 PB2D 在 U≈1 时**：.dll 中 rand()==32767 时 `U*U − U` 的舍入符号可能使 w 的虚部为 ±0 或给出不同分支；建议找出这样的种子（自实现 MSVC rand 搜索），
  比较该粒子 R² 是否 ≤ 8ε。
- **Q6 displacedpos 单位**：伪代码给出 x' = v×1e-5；用 particlenumber 1、`displacedpos 1000 0 0` 运行，检查 SingleParticle.txt 的 dpx（预期 10 mrad）。
- **Q7 未初始化字段**：单粒子模式 P[0].weight/kind 未写、同步粒子 extE/extB 未清零、生成路径不写 P[i].t/dt。新内核建议清零；需确认原内核对结果无影响（单粒子模式权重若为 0 会影响统计）。
- **Q8 probe 越界**：particlenumber < 125 时 probe 模式读越界；用 particlenumber 100 运行看是否崩溃。
- **Q9 .edst 的 this.atomicMass = mc²（MeV）**：确认之后是否有代码使用 Initial+0x18（SetSynPhase、InitMethodMP、dstOut），若有则 .edst 运行的同步相位/质量错误。
- **Q10 二次束 .dst 的误差平移 0.001**：确认是否有意（与普通 .dst 的 0.1 不一致）。
- **Q11 原文丢失的赋值来源**：二次束文本路径 `kind` 的赋值来源（推测第 11 列）、`Particle weight loaded.` 的输出流，需用原内核或反汇编确认。
- **Q12 Twiss 误差对 DC 哨兵**：DC 束开启 ε_z 误差时哨兵被乘坏，是否变成按 Twiss 生成纵向；用误差研究 + beamtype dc 检查。
- **Q13 同进程多次运行**：确认 Python 侧（`avas/api/basic.py`、误差研究）是否在同一进程中多次调用 `main_agent`；若是，GenerateErrorSet 的随机序列依赖上一轮的种子与消耗（G 子系统）。

---

## 6. 对结构表的更正

1. **Initial+0x0b0 frequency**：beam.txt 的 `frequency` 单位是 **Hz**（schema 与示例 `frequency 162500000.0` 一致），ReadBeam 原样存，生成路径直接拷入 `Beam.frequency`，
   不做 MHz→Hz 换算；只有 .dst/.edst/二次束 .dst 路径把文件中的 MHz 值 ×1e6 后写回 +0xb0。结构表“MHz；InitParticle 后改为 Hz”不准确。
2. **Beam+0xc0 fromDstFile**：只在 **.edst** 路径置 1；普通 .dst 路径不置。**Beam+0xbc useWeightFile**：.edst 路径恒置 1，.dst 路径仅 `indweight 1` 时置 1。
3. **Initial+0x130**：构造函数置 1，全部代码中没有其他写入；InitParticle 用它决定是否把 dst 坐标换算为内部坐标（1 = 换算，否则打印 `zCode only` / `Inlet particle distribution is not appropriate`）。
4. **Initial+0x140**：构造函数置 3，InitParticle 要求 == 3 才生成纵向分布（维数）。
5. **Initial+0x008 distFile 默认值**为 `"unknown"`（构造函数 assign），结构表写法正确；distribution 默认 `GS GS` 也已核对。
6. **Initial+0x018 atomicMass**：.edst 路径中被写成同步记录的 mc²（MeV），不是 mc²/938.271875。
7. **Initial+0x040 twiss**：InitParticle 原地换算为几何发射度（m·rad）：ε_x,y ← ε/1e6/(βγ)，ε_z ← ε/1e6/(βγ³)；DC 哨兵为 −2027.041（beamtype dc 写入的 −2027.0415 在 InitParticle 中改为 −2027.041），
   所有 9 个分量的“未设置”默认值也是 −2027.041。
8. **Initial+0x038 particleNumber**：.dst/生成路径会被文件中的 N 覆盖（生成路径读回 inData.dst 时）；`indweight` 时被 .dstb 中的计数覆盖；默认 −1。
9. **Particle 在 .dll 中的 sizeof 为 0x1a8**（`kind` 为 32 字节的 MSVC `std::string`），0x000–0x180 的字段偏移与 .so 相同。
10. **.dll 的 Initial +0xd8**：kneticenergy 的第二个数 ΔW（DC 束能量展宽），.so 中没有对应字段（.so 的 +0xc0 在 InitParticle/ReadBeam 中均未使用）。
11. **Particle+0x180 weight**：二次束 .dst 路径写 realPerMacro（不是 1.0 或文件权重）。

## 实测核实

- **固定种子下横纵向完全相关（已核实，2026-11-07）**：示例 `examples/hwr010`（GS GS，5260 个粒子），在 beam.txt 末尾加 `randomseed`，用 Windows 原内核（AVAS.dll）运行，读 `OutputFile/inData.dst`：
  - seed 12345：corr(x, φ) = −1.0000，corr(x′, W) = 0.8143，corr(y, φ) = 0.0077；
  - seed 0（按时间播种）：corr(x, φ) = 0.0064，corr(x′, W) = 0.0026。

  这是原内核的缺陷。仓库里固定种子 12345 的内核运行（研究层 `DEFAULT_SEED`、`tools/compare_response_engine.py`、`tests/test_error_study.py`）都受影响。新内核是复现它还是修正它，待用户决定。
