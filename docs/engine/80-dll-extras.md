# 80 · AVAS.dll 独有功能与 DLL / .so 差异

材料：`decomp/AVAS_dll/functions/*.c`（Ghidra，无符号）、`AVAS.dll` 原始字节（`.rdata` 字符串、常数、导入表、反汇编 `llvm-objdump`）、
`decomp/libAVAS/functions/*.c` 与 `libAVAS.so` 的 `.rodata`。DLL 镜像基址 0x180000000；下文地址都是 DLL 虚拟地址。
DLL 的 PE 时间戳 2026-04-01 09:34:43 UTC，链接器 14.44（VS 2022），PDB 路径 `D:\AVAS_Pack\AVAS\x64\Release\AVAS.pdb`；
.so 为 GCC 4.8.5（Red Hat 4.8.5-44），无时间戳。

---

## 1. 概述

AVAS.dll 导出 5 个函数：`path`、`main_agent`（与 .so 相同的模拟入口），以及 DLL 独有的 `sampling`、`dataprocess`、`meshing`。
三个独有导出**与模拟内核完全无关**：它们不读 input.txt / beam.txt / lattice，不调用任何元件、推进或空间电荷代码，只读写 TraceWin 格式的
`.dst` 粒子分布文件及其权重文件，做的是"宏粒子分布的压缩与机器学习数据集预处理"：

| 导出 | 一句话 | 输入 → 输出 |
|---|---|---|
| `sampling` | 按 6D 归一化半径分层、按"放大倍数"加权抽样，把一个大分布压缩成 p3 个带权宏粒子，并在参数网格上搜索使失配、密度误差、均值偏差、晕参数偏差的秩和最小的参数 | `in.dst` → `out.dst` + `out.dst` 后加 `b` 的权重文件 + `./SPTest.txt` 搜索日志 |
| `meshing` | 6D 云网格（CIC）沉积到 n⁶ 网格，每个非零节点变成一个位于节点中心、权重为沉积量的宏粒子 | `in.dst` → `out.dst` + `out` 加 `b` 的权重文件 |
| `dataprocess` | 多线程批处理：对每个样本编号 i，读取 `OutputFile_i/FiveCube/` 与 `OutputFile_i/original/` 下各输出面的 `.dst`，计算 Twiss / 统计量和二维相空间密度图，写二进制 `.ppj` | 目录树 → `preprocessing_dataset/OutputFile_i.ppj` |

用途（推测，理由见各节）：`sampling` / `meshing` 产生带权的"代理分布"（输出正好是内核 `readparticledistribution` + `indweight 1` 能直接读的 .dst + 权重文件），
用代理分布跑 AVAS 得到 `FiveCube/` 结果，用原始分布跑得到 `original/` 结果；`dataprocess` 把两组结果整理成成对的特征文件，供训练神经网络代理模型或评估
压缩误差。目录名 `FiveCube`、文件后缀 `.ppj`、"preprocessing_dataset" 都只出现在这里。

**Python 侧没有任何调用**：在 `AVAS_XMU`、`AVAS_XMU-engine-rebuild`、`AVAS` 三个仓库的 `avas/`、`scripts/`、`tools/`、`packaging/` 以及全部 `.py/.ipynb` 中
grep `sampling(`、`dataprocess`、`meshing`、`FiveCube`、`preprocessing_dataset`、`.ppj`、`SPTest`、`Magnification` 均无结果。
`avas/core/MultiParticleEngine.py` 只声明 `path`（Windows 用 `c_wchar_p`）和 `main_agent(POINTER(c_int))`。

调用顺序（三者互相独立，可单独调用，不需要先调 `path`）：

```
sampling(in, out, p3..p10)                 meshing(in, out, n, mag)
  读 dst → 坐标换算 → 二阶矩/Twiss/晕参数      读 dst → 均值/rms → 6D 网格 → CIC 沉积 → 节点→粒子 → 写 dst + 权重
  → 6D 半径排序 → 3D 原始密度网格
  → for S in [p4..p5] 降序, for M in [p6..p7] 降序:
       分段/权重/抽样 → for 偏移 o: 计算 4 个指标 → 偏移内排秩 → 记一行 → 追加 SPTest.txt
  → 全局排秩选最优 (S,M,o) → 重新抽样 → 写 dst + 权重 → 追加 SPTest.txt

dataprocess(root, i0, i1, g0, g1, mag, useW)
  建 root/preprocessing_dataset/ → 线程池 (max(2, 硬件线程数)) → [i0,i1] 切成 ≤4 段
  每段 worker: for i in 段: process_index(root, i, {g0,g1}, mag, useW)  (FUN_180051fb0)
```

---

## 2. 共同约定

### 2.1 ctypes 调用约定（x64 Windows）

```python
dll.sampling.argtypes    = [c_wchar_p, c_wchar_p, c_int, c_int, c_int, c_int, c_int, c_int, c_double, c_double]
dll.meshing.argtypes     = [c_wchar_p, c_wchar_p, c_int, c_double]
dll.dataprocess.argtypes = [c_wchar_p, c_int, c_int, c_double, c_double, c_double, c_int]
# 三者 restype = c_int，正常返回 0
```

**宽字符路径的转换**（三个函数与 DLL 版 `path` 相同）：把参数当作字节串，先取第 0 字节到第一个 `\0` 为止的 C 串（对 UTF-16 只得到首字符），
然后从第 1 字节起逐字节扫描：非零字节追加到 std::string；遇到 `\0` 且下一字节也是 `\0` 就结束。等价于"UTF-16LE 去掉所有零字节"。
因此路径只能含 ASCII；非 ASCII 字符的两个字节都会被当成字符追加（例如 U+4E2D 变成 `-N`）。.so 的 `path` 直接收 `char*`，没有这一步。

### 2.2 .dst 二进制格式（三个函数读写方式一致）

| 字段 | 类型 | 说明 |
|---|---|---|
| h0, h1 | 2 × char | 头两个字节（内容不用） |
| N | int32 | 粒子数 |
| Ib | double | 流强 mA |
| f | double | 频率 MHz |
| h2 | char | 1 字节（内容不用） |
| 粒子 | N × 6 double | x(cm), x'(rad), y(cm), y'(rad), φ(rad), W(MeV) |
| mc² | double | 静止能量 MeV |

**写出**时头部写成 `h2, h0, N, Ib, f, h2`（第 1 个字节写的是读入时的 h2，第 2 个是 h0，原 h1 丢失），总长仍为 23 字节头。
权重文件（文件名 = 输出 .dst 全名后加字符 `b`，例如 `x.dst` → `x.dstb`）：`int32 N` + `N × double`。这与内核 InitParticle 读 `.dstb` 的格式相同（见 E 组文档）。

### 2.3 致命错误会结束宿主进程

所有错误都 `exit()`：`exit(1)`（文件打不开、建目录失败）、`exit(0)`（sampling 的 "Magnification is too large"）。被 Python 以 ctypes 直接调用时会把整个
Python 进程（包括 GUI）一起结束。重建时应改为返回错误码，但对齐测试要知道原内核行为。

### 2.4 MSVC 浮点与 STL 细节

- `floor` 一律写成"截断，若为负且非整数再减 1"（`cvttsd2si` + `movmskpd` 符号位）；`round` 调 CRT `round`（半数远离零）。
- 排序用 MSVC `std::sort`（introsort：长度 ≤ 32 用插入排序，否则三数取中快排，深度用尽改堆排序），**不稳定**；比较键相等时顺序由 MSVC 实现决定。
  索引数组本身存成 double（`(int)` 后作下标）。
- 反三角/平方根照常；`pow(a, n)` 调 CRT `pow`。

---

## 3. sampling：分层加权抽样与参数搜索

入口 `sampling` @0x180058620（34 866 字节）。

### 3.1 参数

| 参数 | 类型 | 含义（依据：打印标签与用法） |
|---|---|---|
| p1 | wchar* | 输入 .dst 路径（"The inFile Name has been set to "） |
| p2 | wchar* | 输出 .dst 路径（"The outFile Name has been set to "）；权重写到 p2+"b" |
| p3 | int | 输出宏粒子总数；其中 T = p3 − 2 个由分层抽样产生，另 2 个是半径最小与最大的粒子 |
| p4, p5 | int | 段数 S 的搜索范围：S 从 p4 递减到 p5（p4 < p5 时一次都不搜索，随后对空结果取下标 0 —— 未定义行为） |
| p6, p7 | int | 放大倍数 M（"Magnification"）的搜索范围：从 p6 递减到 p7 |
| p8 | int | 三维密度网格每维节点数 n |
| p9 | double | 密度网格半宽，以 rms 为单位：网格跨度 ±p9·σ |
| p10 | double | 末端系数 C_t（"terminalCoefficient"），最外段权重再除以它 |

### 3.2 读入与坐标换算

1. 打开 `./SPTest.txt`（模式 out|trunc）随即关闭 —— 每次调用先清空该文件（相对当前工作目录）。
2. 读 p1（打不开："The initial bunch distribution file cannot be opened, check particleDistributionPath"，`exit(1)`）。
   保存一份原始数组 R[i][0..5]（之后输出用），另一份 P[i][0..5] 做换算。打印 `N " sorting......"`。
3. φ_min = min_i φ_i，初值 9999999.0（0x416312CFE0000000）。
4. 对每个粒子（c = 299792458.0）：
   - γ = W/mc² + 1；v_z = sqrt( (c·sqrt(1 − 1/γ²))² / (x'² + y'² + 1) )（`pow(.,2)` 后开方，x'、y' 为 rad）
   - Δt = (1e-6 / (f · 6.283185307179586)) · (φ − φ_min)
   - z = v_z · Δt · (−1000)          [mm，越晚到达越靠后]
   - P5 = v_z · 3.3356409519815204e-09 · γ   （= γβ_z，常数是 1/c）
   - x = (x_cm · 0.01 − Δt · x' · v_z) · 1000   [mm，漂移回 φ_min 时刻]；y 同理
   - x' ← x' · 1000，y' ← y' · 1000   [mrad]
   - 同时累加六个换算后坐标之和、以及原始 W 之和。
5. 均值 ⟨P_k⟩ = Σ/N（k = 0..5），W̄₀ = ΣW/N。
6. 对每个粒子：P5 ← (P5 − ⟨P5⟩)/⟨P5⟩ · 1000（δ，单位 1e-3）；P0..P4 减去各自均值。

### 3.3 原始分布的矩、Twiss 与晕参数

13 个二阶矩（**不再减均值**，直接 Σ/N）：
`m0=⟨x²⟩ m1=⟨x'²⟩ m2=⟨xx'⟩ m3=⟨y²⟩ m4=⟨y'²⟩ m5=⟨yy'⟩ m6=⟨z²⟩ m7=⟨δ²⟩ m8=⟨zδ⟩ m9=⟨xy⟩ m10=⟨xy'⟩ m11=⟨x'y⟩ m12=⟨x'y'⟩`。
7 组 Twiss，每组 6 个数 `{ε, α, β, γ_T, σ1, σ2}`，ε = sqrt(a·b − c²)，α = −c/ε，γ_T = (α² + 1)/β：

| 组 | a, b, c | β 取 | σ1, σ2 |
|---|---|---|---|
| 0 x | m0, m1, m2 | m0/ε | √m0, √m1 |
| 1 y | m3, m4, m5 | m3/ε | √m3, √m4 |
| 2 z | m6, m7, m8 | m6/ε | √m6, √m7 |
| 3 x–y | m3·m0 − m9² | m0/ε | √m0, √m3 |
| 4 x–y' | m4·m0 − m10² | m0/ε | √m0, √m4 |
| 5 x'–y | m3·m1 − m11² | **m3/ε** | √m3, √m1 |
| 6 x'–y' | m4·m1 − m12² | **m1/ε** | √m1, √m4 |

晕参数（Allen–Wangler 二维定义），对 (x,x')、(y,y')、(z,δ) 三个平面，原点矩（不减均值）：
H = sqrt(3⟨u⁴⟩⟨u'⁴⟩ + 9⟨u²u'²⟩² − 12⟨uu'³⟩⟨u³u'⟩) / (2⟨u²⟩⟨u'²⟩ − 2⟨uu'⟩²) − 2，矩都是 Σ/N。记 H₀[3]。

### 3.4 6D 归一化半径排序

r_i = sqrt((x/σx)² + (x'/σx')² + (y/σy)² + (y'/σy')² + (z/σz)² + (δ/σδ)²)，σ 取上表 0、1、2 组的 σ1、σ2。
索引表 I = [0..N−1] 按 r 升序 `std::sort`（键相等时不稳定）。之后"第 k 个粒子"都指 I[k]。

### 3.5 原始三维密度网格（打印 "Meshing......"）

每维 n = p8 个节点，h_x = 2σx·p9·(1/(n−1))（σx = √m0；y 用 √m3，z 用 √m6）；节点坐标 c[i] = ((i − n·0.5) + 0.5)·h（中心在 0）。
每个粒子：ix = floor((x − c_x[0]) · (1/h_x))，iy、iz 同理（乘倒数，不是除法）。
若 0 ≤ ix, iy, iz ≤ n − 2，计算 8 个 CIC 权重 g_k = |x − c_x[ix + (1 − (k&1))]| · |y − c_y[iy + ((k&3) < 2)]| · |z − c_z[iz + (k < 4)]| · (1/(h_y·h_x·h_z))
（乘法顺序 ((|Δx|·|Δy|)·|Δz|)·inv），否则 8 个权重为 0。沉积到 G₀：k = 0..7 依次对应节点
(ix,iy,iz)、(ix+1,iy,iz)、(ix,iy+1,iz)、(ix+1,iy+1,iz)、(ix,iy,iz+1)、(ix+1,iy,iz+1)、(ix,iy+1,iz+1)、(ix+1,iy+1,iz+1)。
每个粒子的 (ix,iy,iz) 与 8 个权重保留下来，供抽样子集复用。G₀ 不归一化（Σ = 网格内粒子数）。

### 3.6 参数搜索（打印 "Searching......"）

对 S 从 p4 递减到 p5、M 从 p6 递减到 p7 的每一对：

**(a) 段权重**：w₀ = M；w_{k+1} = M^(−1/(S−2)) · w_k（k = 0..S−3，于是 w_{S−2} ≈ 1）；w_{S−1} = M^(−1/(S−2)) · w_{S−2} · (1/C_t)。
`pow` 每次重新调用。S = 2 时除以零（未检查）。

**(b) 段大小**：n_k = floor(N / S)（整数除法），最后一段 n_{S−1} += N − Σn_k。段按排序后的顺序连续划分（第 0 段最靠近中心）。

**(c) 归一化**：D = Σ_k n_k / (T · w_k)，T = p3 − 2。（S > 3 时编译器用 2 路 SIMD + 4 展开求和，顺序与逐项相加不同，结果在末位可能不同。）

**(d) 抽样位置**：start₀ = 0；第 k 段：K_k = round(n_k / (D·w_k))；j = 0..K_k−1：
idx = round((j + 0.5)·w_k·D + start_k)；若 idx ≥ N 结束本段；记录 idx 与权重 w_k，计数 cnt_k++。
start_{k+1} = (int)(start_k + n_k)。于是第 k 段每 D·w_k 个粒子取一个，权重 w_k：中心段稀疏取、权重大，外层密集取、权重小 —— 用较少宏粒子保留晕。

**(e) 放大倍数过大**：任一 cnt_k = 0 → 打印 `Error! The Magnification is too large <S> <M> <k>` 并 `exit(0)`。

**(f) 偏移范围**：A = min(0.8·idx_first − 3, 0.8·(N − idx_last) − 3)（idx_first/last 为全部抽样位置的第一个/最后一个）。
A ≥ 1 时偏移 o 取 −round(A), …, +round(A)；否则只取 o = 0。偏移把所有抽样位置整体平移，用来避开"每次都取同一相位的粒子"。

**(g) 每个偏移的样本**：抽样粒子 I[idx + o]（换算后坐标 P，权重 w_k），再加上 I[0] 与 I[N−1]（权重都是 w_{S−1}）。Σw 记 W_s。
四个指标：

1. **Mismatch**：样本加权、减样本均值的 13 个矩（/W_s），按 3.3 同样的 7 组公式得样本 Twiss。每组
   R = (α_s − α₀)² − (β_s − β₀)(γ_s − γ₀)，M_k = sqrt((R + sqrt(R(R + 4)))·0.5 + 1) − 1
   （代数上等于标准失配因子 sqrt(½(R* + sqrt(R*² − 4))) − 1，R* = β₀γ_s + β_sγ₀ − 2α₀α_s）。
   Mismatch = (M₀ + M₁ + M₂ + M₃ + M₄ + M₅ + M₆)/9 + M₂·2/9（z 平面共占 3/9）。
2. **MSE**：样本网格 G_s = Σ_样本 w·(N/W_s)·(该粒子的 8 个 CIC 权重)，用 3.5 保存的格点与权重，同样只取网格内粒子。
   MSE = sqrt( Σ_{ix,iy,iz} (G_s − G₀)² / n³ )（求和顺序：ix 外、iy、iz 内层，内层 4 展开）。
3. **DeltaMean** = |W̄_s − W̄₀| + Σ_{k=0..5} |⟨P_k⟩_s|，其中 W̄_s = Σ w·W_原始 / W_s，⟨P_k⟩_s = Σ w·P_k / W_s（单位混用：mm、mrad、1e-3、MeV）。
4. **DeltaHalo** = Σ_{x,y,z} |H_s − H₀| / H₀，H_s 用样本加权原点矩（/W_s，不减样本均值）。

**(h) 偏移内选优**：对四个指标分别 argsort（升序，`std::sort`），每个偏移的秩 = 它在该排序中的位置（0 起）。
四秩之和最小者为该 (S,M) 的最优偏移（严格小于，初值 9999999.0，并列时取先出现的偏移）。
记录行 `[S, M, o*, Mismatch, MSE, DeltaMean, DeltaHalo]`，并追加一行到 `./SPTest.txt`（out|app，`std::fixed`，`setprecision(12)`）：

```
<S>   <M> <C_t>   <o*>   <Mismatch>   <MSE>  <DeltaMean> <DeltaHalo> <A>\n
```
S、M 是整数（无小数）；其余 double 以 12 位小数输出；分隔符依次为 3 空格、1 空格、3、3、3、2、1、1 个空格；最后一列是偏移半宽 A（可为负或小数）。

### 3.7 全局选优与输出

对全部 (S,M) 行的四个指标再分别 argsort 求秩，四秩和最小（严格小于，初值 999999.0）的行为最优。控制台打印
`segments: <S> Magnification: <M> offset: <o> rankMismatch: <r> rankMse: <r> rankMean: <r> rankeHalo: <r> Mismatch: <v> MSE: <v> DeltaMean: <v> DeltaHalo: <v>`
（S、M、o 是 double，默认精度 6），并以 fixed/12 位追加到 SPTest.txt，格式相同但在 offset 后多 `terminalCoefficient: <C_t>`。

用最优 S、M（取 round）、o 重新生成：权重 w₀ = M，w_{k+1} = w_k · (1/M^(1/(S−2)))，w_{S−1} = w_{S−2}/(C_t · M^(1/(S−2)))（写法与搜索时不同，末位可能差一个 ulp）；
段大小、D、抽样位置公式同上，但**没有 idx ≥ N 的检查**。输出粒子取**原始未换算**坐标 R[I[idx + o]]，权重 w_k；最后追加 R[I[0]] 与 R[I[N−1]]，权重 w_{S−1}。
写 p2（.dst，2.2 节格式，mc² 原样）与 p2+"b"（权重，未归一化，Σw ≠ N —— 内核以 I/f/Σw 计算每个宏粒子代表的粒子数，绝对尺度无关）。
"output error"：输出文件打不开时只打印，不退出。最后打印 `Search completed, mismatch is: <最优行的 Mismatch>`。

---

## 4. meshing：6D 网格化压缩

入口 `meshing` @0x180061460（8 809 字节）。参数 `(wchar* in, wchar* out, int n, double mag)`。

1. 读 in（打不开："The initial bunch distribution file cannot be opened, check particleDistributionPath"，`exit(1)`）。打印 `N " Meshing......"`。
2. 六列（原始单位 cm、rad、cm、rad、rad、MeV）的均值 μ_k = Σ/N，rms σ_k = sqrt(Σ(x − μ)²/N)（总体 rms，无权重）。
3. 步长 h_k = (σ_k + σ_k)·mag·(1/(n − 1))；节点 c_k[i] = ((i − n·0.5) + 0.5)·h_k + μ_k（n 个节点覆盖 μ ± mag·σ）。
4. 网格 G：n⁶ 个 double（6 层嵌套 vector，内存 8·n⁶ 字节；n = 20 时 512 MB，未检查）。
5. 每个粒子：i_k = floor((x_k − c_k[0]) / h_k)（**除法**）。只要有一维 i_k ≥ n − 1 或 < 0 就跳过。
   否则对 64 个角 o ∈ {0,1}⁶（二进制计数，第 5 维为最低位）：
   w_o = ((((((1·|x₀ − c₀[i₀+o₀]|)·|x₁ − c₁[i₁+o₁]|)·…)·|x₅ − c₅[i₅+o₅]|) · (1/(h₅·h₄·h₃·h₂·h₀·h₁))，
   把 w_o 加到节点 i + (1 − o)（即"对角"节点 —— 标准 CIC）。每个网格内粒子的 64 个权重之和为 1。
6. 按 (i₀, i₁, …, i₅) 字典序（i₅ 最快）扫描 G，值 ≠ 0.0 的节点生成一个粒子：坐标 (c₀[i₀], …, c₅[i₅])，权重 = G 值。
7. 写 out（.dst，2.2 节格式，粒子数 = 非零节点数 N'，Ib、f、mc² 原样）和 out+"b"（权重）。打开失败只打印 "output error"。
   打印 `Mesh completed. Number of extracted particles is: <N'>`。

Σ权重 = 网格内的原始粒子数（落在网格外的粒子丢弃）。

---

## 5. dataprocess：成对模拟结果的批处理

入口 `dataprocess` @0x180060e60；worker 包装 FUN_180055e20；每个样本的处理 FUN_180051fb0（15 960 字节）。

### 5.1 参数与线程

`(wchar* root, int i0, int i1, double g0, double g1, double mag, int useW)`：
g0、g1 分别是 FiveCube 与 original 数据的二维网格节点数（用 round 取整）；mag 是网格半宽（rms 单位）；useW == 1 时 FiveCube 数据读权重。

1. 打印 `The inFile Name has been set to <root>`；建 `root/preprocessing_dataset`（不存在时 `_mkdir`，失败 "Error output folder creation failure"，`exit(1)`）。
2. 线程池（progschj::ThreadPool，线程数 max(2, `std::thread::hardware_concurrency()`)）。
3. 切分：step = ceil((i1 − i0 + 1)·0.25)；边界 b = [i0, i0+step, i0+2·step, …（< i1+1）, i1+1]；打印 `NumofThreads: <边界数−1>`；
   每段 [b_j, b_{j+1}) 一个任务，任务内按 i 递增顺序串行处理。等待全部完成，打印 `PostProcess complete. index: [<i0>, <i1>]`。
   各任务的控制台输出交错，顺序不确定；每个 i 完成打印 `Inedx <i> complete.`（原文拼写）。

### 5.2 单个样本 i 的输入

- 目录 `D_F = root/OutputFile_<i>/FiveCube/`，`D_O = root/OutputFile_<i>/original/`。
- 文件表：用 `_findfirst("D_O\*.dst")` 列出 original 中全部 `.dst`（跳过 `.`、`..`），**FiveCube 中假定有同名文件**。
- 每个文件名的 z：取第一个 `_` 之后最多 8 个字符做 `strtod`（`outData_1.234567.dst` → 1.234567；超过 8 字符被截断）。按 z 升序选择排序。
- 输出 `root/preprocessing_dataset/OutputFile_<i>.ppj`：先以 out|trunc 打开一次（第二个流），再以 out|binary 打开写入；失败 "<路径> open failed, Check permissions"，`exit(1)`。
- 先读 `D_O/file0` 头得 N_O0；再读 `D_F/file0` + "b"（**无论 useW 是否为 1 都必须存在**，否则 "The initial bunch distribution b file cannot be opened, check particleDistributionPath: <路径>"，`exit(1)`）求 Σw_F0。
  缩放 s_F = N_O0 / Σw_F0（只用第 0 个文件），s_O = 1。

### 5.3 每个文件 j、每个数据集 d ∈ {FiveCube, original} 的计算

读 `.dst`（失败 "The initial bunch distribution file cannot be opened, check particleDistributionPath: <路径>"，`exit(1)`）。
权重 w：d = FiveCube 且 useW == 1 时读 `file_j` + "b"（其中的 N 覆盖 .dst 的 N，二者不一致会越界），否则全 1。Σw 记 W。

1. 逐粒子（x' 与 y' 仍为 rad 时）：γ = W/mc² + 1，v_z = (299792458.0 / sqrt(x'² + y'² + 1))·sqrt(1 − 1/γ²)；
   累加 Σwφ、ΣwW、Σw·v_z；然后 x ← 10x [mm]，x' ← 1000x' [mrad]，y、y' 同理。
2. W̄ = ΣwW·(1/W)，γ̄ = W̄/mc² + 1，β̄ = sqrt(1 − 1/γ̄²)，φ̄ = (1/W)·Σwφ，K = γ̄·Σw v_z·(1/W)。
3. z_i = (0.001/(f·6.283185307179586))·(φ̄ − φ_i)·v_z,i [mm]；δ_i = (v_z,i·γ_i − K)·(1/K)·100 [%]。
4. 均值 ū_k = Σw u_k / W；中心二阶矩（/W）：x、y、z 三个平面的 ⟨u²⟩、⟨u'²⟩、⟨uu'⟩ 和 ⟨xy⟩。
5. 写 4 组 × 9 个 double：x、y、z 组为 `[ε·β̄γ̄, α, β, ū, ū', σu, σu', W̄, N]`（ε = sqrt(⟨u²⟩⟨u'²⟩ − ⟨uu'⟩²)，α = −⟨uu'⟩/ε，β = ⟨u²⟩/ε）；
   x–y 组 `[ε_xy·β̄γ̄, −⟨xy⟩/ε_xy, ⟨x²⟩/ε_xy, x̄, ȳ, σx, σy, W̄, N]`。N 是文件粒子数（不是 Σw）。
   d = FiveCube 时保存 σx、σx'、σy、σy'、σz、σδ 作为本文件两个数据集共用的网格尺度。
6. 写 `double z_j`、`double Ib`（本数据集文件头的流强）。
7. 三个相平面 (x,x')、(y,y')、(z,δ) 与实空间 (x,y)，各一张 n_d × n_d 网格（n_F = round(g0)，n_O = round(g1)）：
   步长 h = (2σ_FiveCube)·mag·(1/(n_d − 1))，节点 u[i] = (i − (n_d·0.5 − 0.5))·h + ū_当前数据集。
   ix = floor((u − u[0])·(1/h))，需 0 ≤ ix, iy ≤ n_d − 2；双线性 CIC：
   节点 (ix,iy) += |(v − v[iy+1])(u − u[ix+1])|·(1/(h_v h_u))·w·s_d，(ix+1,iy) 用 v[iy+1]、u[ix]，(ix,iy+1) 用 v[iy]、u[ix+1]，(ix+1,iy+1) 用 v[iy]、u[ix]。
   写网格 n_d² 个 double（行 = ix，列 = iy）；再以网格中心为原点算 Σg·Δu²、Σg·Δv²、Σg·ΔuΔv，除以 s_d·W，
   写 5 个 double `[ε·β̄γ̄, α, β, σu, σv]`。

### 5.4 .ppj 文件布局

```
int32 nFiles; int32 n_F; int32 n_O; double mag
for j in 文件（z 升序）:
  for d in [FiveCube, original]:
    36 double   统计量（x, y, z, xy 各 9 个）
    double z_j; double Ib_d
    4 × ( n_d² double 网格 + 5 double 网格 Twiss )   顺序 (x,x'), (y,y'), (z,δ), (x,y)
```
每个 (j,d) 共 38 + 4(n_d² + 5) 个 double。单位：mm、mrad、mm、%、MeV。

---

## 6. DLL 核心与 .so 的差异

### 6.1 结论：DLL 是同一源码树稍后的构建（核心几乎相同）

| 证据 | 说明 |
|---|---|
| 字符串超集 | 把两个二进制的用户字符串（去掉编译器、运行库、符号表名）求差：.so 独有的用户字符串为 **0**；DLL 独有的只有三组导出工具的字符串、`spacechargecomp`、一个死字符串（6.3）以及 PIC 调试输出文件名（6.4）。内核全部报错、提示、进度行格式、输出文件名（DataSet、synData、BeamSet.plt、outData_*.dst/.dstb/.edst、PCHistogram.dat 等）两边逐字相同。 |
| 新的 lattice 关键字 | DLL 的 InitLattice（FUN_180030ba0）识别 `spacechargecomp`，.so 没有这个字符串（6.2）。 |
| 新的 Beam 成员 | DLL Beam 构造（FUN_180058440）在 +0xe8 写 1.0；.so `main_agent` 内联的 Beam 初始化最后只写到 +0xd0，不写 +0xe8。 |
| 相位扫描、读初始分布 | 两边的 SetSynPhase / singlePartM / InitParticle 字符串集合相同（"Start scan phase...."、"Scan phase error, unexpected situation, check code"、"Synchronous particle reverse motion, program stopped"、"Note that the energy …"、"RFQ Rescan phase,phi_0:"、"zCode only"、"edst"、"Particle weight loaded."、"The initial bunch distribution file cannot be opened, check particleDistributionPath"、"The initial bunch Weight file …" 等）。**.so 同样有 `readparticledistribution` 从 .dst / .edst 读初始分布与 `.dstb` 权重的功能**；DLL 里该提示出现 16 次只是因为 MSVC 没有合并相同字面量，加上三个导出工具各自带 ": " 后缀的版本。 |
| DataSet 写法 | 两边相同：Run 先截断 DataSet.txt，tcodeRun 每次输出以追加模式重新打开（.so `filebuf::open(…, 0x11)` = out|app；DLL `open(…, 10, 0x40)` = out|app），写完关闭。 |
| main_agent | 两边字符串、流程相同（"The programme is running."、"Single particle mode startup ... ..."、"Initialization time: "、"Simulated total time: "）。 |
| 随机数 | 两边都导入 C `rand`/`srand`（.so 另有 `rand_gen()`）；Windows 与 glibc 的 `rand` 序列不同，同一 randomseed 在两个平台产生的初始分布不同（推测，需实验）。 |

### 6.2 `spacechargecomp`（DLL 独有的空间电荷补偿）

- InitLattice：行 `spacechargecomp V1` → 生成向量 `{z, 1.0, V1}`，z = 当前累计元件末端 z（还没有元件时为 0），push 到 Initial+0x258（DLL 布局）。
- InitMethodMP（FUN_180039dd0）把该表拷到 MethodMP+0x2c0。
- Run（FUN_18006b990）：对每个元件，若某条记录的 z ∈ [元件 zStart, zStart + L)，在事件表里加入该元件的 zStart，事件类型 −4。
- tcodeRun（FUN_180070b90）：处理类型 −4 事件时在表中找 |z_记录 − z_事件| < 1e-9 且 [1] 与 1.0 相差 < 1e-9 的记录，把 [2]（V1）写到 Beam+0xe8；找不到时下标为 −1（越界读）。
- Beam+0xe8 默认 1.0。**读取它的代码没有找到**（全部 `0xe8` 偏移的 double 读取都属于其他结构）；按名称推测它是空间电荷力的乘子（1 = 不补偿，0 = 完全补偿）。
- schema.py 已列出 `spacechargecomp` 但标"手册未说明"。.so 的 InitLattice 没有该分支，推测把这一行当未知关键字忽略。

### 6.3 "Synchronized particle phase abnormality, exit scanning phase"

DLL 中有两份，地址 0x1800a4240 与 0x1800a4980，字节为 `invalid` + `EF BC 3F`（全角逗号 U+FF0C 的 UTF-8 `EF BC 8C` 第三个字节被换成 `?`）+ 英文句子 + 两个空格，
分别位于 SetSynPhase（FUN_180039f20）与 singlePartM（FUN_180042860）的字面量区中间。**代码中没有任何引用**：反汇编里没有 RIP 相对地址指向它，
暴力搜索全部 32 位位移也只得到落在指令中间的偶然命中，`.data` 里也没有指向它的指针。.so 中不存在此字符串。
推测：DLL 的源码在两处相位扫描里新增了一段"同步粒子相位异常，退出扫描"的输出，但该分支被编译器证明不可达而删除（例如 `/fp:fast` 下 `x != x` 之类的 NaN 判断被折叠为假），字面量残留。
**行为上 DLL 与 .so 的相位扫描没有因此产生差别**；重建时不需要实现这条信息，但可作为"DLL 源码比 .so 新"的旁证。

### 6.4 DLL 中其他未被引用的字符串

0x1800a8150–0x1800a83a0 一段：`./OutputFile/sdsData.txt`、`./rho.txt`、`RMS_R`、`./weight.txt`、`./OutputFile/rho.txt`、`./phi.txt`、`./E.txt`、
`./OutputFile/spaceEfieldAndPhi.txt`、`numofgridx: / numofgridy: / numofgridz:`、`stepx: / stepy: / stepz:`、`./OutputFile/EOnParticledata.txt`、
`Phi  and  E output`、`E On Particle  output  done`、`./EOnMesh.txt`，以及 `hello world`（0x1800a6400）、`sssssssss`。
它们夹在 PIC 相关字符串（SpaceChargeMesh.txt、SpaceChargeSet.plt）中间，全部无代码引用 —— 是空间电荷求解器的调试输出函数被链接器丢弃后残留的字面量。
**不属于任何可观测行为**，新内核不需要实现；.so 中都没有。

### 6.5 编译器 / ABI 造成的布局差异（读 DLL 伪代码时要换算）

| 项 | .so | DLL | 原因 |
|---|---|---|---|
| std::string | 8 字节（GCC COW） | 32 字节（MSVC SSO） | ABI |
| Particle 步长 | 0x190 | **0x1a8** | 末尾 `kind` string 多 24 字节；其余字段偏移相同（例：FUN_180078570 中 `index*0x1a8 + 0x20` 为 z、`+0x104` 为 nextPlane） |
| Beam | 粒子数组 +0x48、n +0x58 | 同 | 无 string 成员 |
| Beam +0xe8 | 无（未初始化） | SC 补偿系数，默认 1.0 | 新功能 |
| Element zStart / length | +0x48 / +0x50 | +0x60 / +0x68 | typeName string 在 +0x40 |
| Initial 输出面列表 | +0x1a8 | +0x1f0 | 前面有 string 成员 |
| Initial `spacechargecomp` 表 | 无 | +0x258 | 新功能 |
| MethodMP 输出面 / SC 补偿表 | +0x220 / 无 | +0x290 / +0x2c0 | 同上 |
| 并行 | GOMP + progschj::ThreadPool | VCOMP（`_vcomp_fork`）+ 同一 ThreadPool | 编译器 |
| `path` 参数 | `char*` | `wchar_t*`（2.1 节转换） | 平台 |

---

## 7. 对齐清单

| ID | 行为 | 涉及关键字/文件 | 建议的比对测试 |
|---|---|---|---|
| H-01 | 三个导出的 ctypes 签名与返回值 0 | AVAS.dll | Windows 上按 2.1 节声明调用，检查返回 0、无崩溃 |
| H-02 | 宽字符路径去零字节转换；非 ASCII 路径被破坏 | sampling/meshing/dataprocess/path | 用 ASCII 路径与含中文路径各调一次，比较控制台 "The inFile Name has been set to" 后的文本 |
| H-03 | .dst 读写格式与写出头部字节顺序 `h2,h0,N,Ib,f,h2` | .dst | 构造头字节已知的 .dst，经 meshing 输出后逐字节比较头 23 字节 |
| H-04 | 权重文件名为输出全名加 `b`，格式 int32 + double[] | `*.dstb` | 检查 meshing/sampling 输出目录文件名与长度 4 + 8N |
| H-05 | meshing：均值、总体 rms、步长 2σ·mag/(n−1)、节点中心公式 | meshing | 2000 粒子高斯分布，n = 6、mag = 3：比较输出粒子坐标集合（应全落在节点上），容差 1e-12 相对 |
| H-06 | meshing：任一维越界即丢弃；64 角 CIC，权重和 = 网格内粒子数 | meshing | 比较 Σ权重与独立计算的网格内粒子数（完全相等或 1e-9） |
| H-07 | meshing：非零节点字典序输出（第 5 维最快） | meshing | 比较输出粒子顺序与权重逐项，容差 1e-12 |
| H-08 | sampling：坐标换算（z、δ、漂移回 φ_min） | sampling | 无法直接观测中间量；用 SPTest.txt 的 Mismatch、DeltaMean 与重算结果比较，容差 1e-9 相对 |
| H-09 | sampling：6D 半径排序与 I[0]、I[N−1] 两个附加粒子 | sampling | 输出 .dst 最后两个粒子应为原始分布 r 最小/最大者，逐字节相等 |
| H-10 | sampling：段权重、段大小、D、抽样位置公式 | p3–p7、p10 | 固定 S、M 范围各取单值（p4 = p5，p6 = p7），比较输出粒子数（≈ p3）与权重向量，容差 1e-12 |
| H-11 | sampling：偏移范围 A 与偏移内四指标秩和选优 | SPTest.txt | 比较每行的 o* 与 A（逐字符，fixed 12 位） |
| H-12 | sampling：Mismatch / MSE / DeltaMean / DeltaHalo 定义 | SPTest.txt | 同一输入比较四列，容差 1e-9 相对（求和顺序差异） |
| H-13 | sampling：全局秩和选优与最终输出用原始坐标 | out.dst、SPTest.txt 末行 | 多个 (S,M) 搜索，比较最终行的 S、M、offset 与输出 .dst 字节 |
| H-14 | sampling："Magnification is too large S M k" 后 exit(0) | 控制台 | 设极大 M（例如 p6 = 1000、N = 1000），检查信息与进程退出码 0 |
| H-15 | SPTest.txt 每次调用先清空、之后追加；格式与分隔空格 | ./SPTest.txt | 连续调用两次，文件只含第二次的内容；逐字符比较格式 |
| H-16 | dataprocess：区间切分（≤4 段）与 "NumofThreads" 输出 | dataprocess | i0 = 0、i1 = 9 时应打印 4；i0 = i1 时打印 1 |
| H-17 | dataprocess：文件发现、z 解析（`_` 后 8 字符）、按 z 排序 | original/*.dst | 放入 3 个乱序 z 文件，检查 .ppj 中 z_j 顺序 |
| H-18 | dataprocess：FiveCube 第 0 个 `.dstb` 必须存在，s_F = N_O0/Σw_F0 | FiveCube/*.dstb | 删除该文件应 exit(1)；改变其权重和，网格值按比例变化 |
| H-19 | dataprocess：36 个统计量（εn、α、β、均值、σ、W̄、N） | .ppj | 用已知分布生成 .dst，与 Python 重算比较，容差 1e-12 相对 |
| H-20 | dataprocess：网格尺度取 FiveCube 的 σ，节点中心取当前数据集均值，双线性 CIC | .ppj | 比较 4 张网格与 5 个网格 Twiss，容差 1e-12 |
| H-21 | dataprocess：.ppj 头与记录布局 | .ppj | 文件长度 = 20 + nFiles·Σ_d 8·(38 + 4(n_d² + 5)) 字节 |
| H-22 | `spacechargecomp V1` 在元件入口把 Beam 补偿系数设为 V1（DLL）；.so 忽略 | lattice | 同一项目加/不加 `spacechargecomp 0`，在 DLL 与 .so 上比较 DataSet.txt rms（DLL 应接近零流强结果；需实验确认含义） |
| H-23 | 相位扫描与读初始分布在 DLL 与 .so 中行为相同 | V3 = 0、readparticledistribution | 同一输入在 Windows / Linux 跑，比较 synData.txt 的 φ0、φs（容差 1e-9）与 inData.dst |
| H-24 | 未引用字符串不产生任何输出文件 | rho.txt、E.txt 等 | 跑带空间电荷的示例，确认工作目录与 OutputFile 中不出现这些文件 |

---

## 8. 函数覆盖表

只从三个独有导出可达、核心（`main_agent`/`path` 调用树）不可达的函数共 33 个，全部列出；其后列出它们使用的、与核心共用的辅助函数。

| 函数 | 作用一句话 | 在正文哪节 |
|---|---|---|
| sampling @0x180058620 | 分层加权抽样与参数搜索主体 | 3 |
| dataprocess @0x180060e60 | 建目录、线程池、切分区间、等待 | 5.1 |
| meshing @0x180061460 | 6D CIC 网格化压缩 | 4 |
| FUN_180055e20 | dataprocess 的任务体：对 [lo,hi) 逐个调 FUN_180051fb0 | 5.1 |
| FUN_180051fb0 | 单个样本 i 的完整处理，写 .ppj | 5.2–5.4 |
| FUN_180051a60 | `_findfirst/_findnext` 列目录下指定扩展名的文件 | 5.2 |
| FUN_180051d40 | 按 z 值选择排序文件名表 | 5.2 |
| FUN_180063900 | ThreadPool::enqueue 的实例化（绑定 string,int,int,vector<double>,double,int） | 5.1 |
| FUN_1800636f0 | std::string + char | 2.1 |
| FUN_180064450 | std::string 移动赋值包装 | 2.1 |
| FUN_18001dc70 | ofstream::open(name, mode, prot)（SPTest.txt 追加） | 3.6 |
| FUN_1800637d0 | 以 double 键对"double 形式的下标数组"排序（调 FUN_180064530） | 3.4、3.7 |
| FUN_180064530 | MSVC introsort 主体（double 下标数组版） | 2.4 |
| FUN_1800652e0 | introsort 分区（三数取中） | 2.4 |
| FUN_180065ca0 | introsort 堆调整 | 2.4 |
| FUN_180066150 | 三数取中比较交换 | 2.4 |
| FUN_180063880 | 以 double 键对 int 下标数组排序（偏移内求秩） | 3.6(h) |
| FUN_1800638a0 | std::find（int 数组），求秩位置 | 3.6(h) |
| FUN_1800638d0 | 在 double 形式下标数组中查找 int 值，求全局秩位置 | 3.7 |
| FUN_180063800 | int → double 再 floor（段大小 N/S） | 3.6(b) |
| FUN_180063850 | round((double)int) | 3.6 |
| FUN_180063860 | (double)n 的立方（MSE 分母） | 3.6(g) |
| FUN_1800637f0 | pow((double)int, double)（最终权重） | 3.7 |
| FUN_18004b4e0 | 迭代器拷贝（返回指针） | — STL |
| FUN_1800642f0 | vector::end() | — STL |
| FUN_1800643a0 | vector<int>(n, v) 构造 | 3.6(h) |
| FUN_18000f4c0 | vector<int>::push_back（扩容路径） | 3.6(f) |
| FUN_180064230 | vector<vector<vector<double>>>(n, …) 构造（3D 网格） | 3.5 |
| FUN_180064ba0 | vector<vector<int>>(N, v)（每粒子格点） | 3.5 |
| FUN_180064c40 | vector<vector<vector<double>>> 填充构造（样本网格） | 3.6(g) |
| FUN_180064ce0 | vector<int>(n, v) 构造 | 3.5 |
| FUN_180064a30 | 6 层嵌套 vector 构造（n⁶ 网格） | 4 |
| FUN_180064d80 | 6 层嵌套 vector 析构 | 4 |
| FUN_180064e50 | 5 层嵌套 vector 析构 | 4 |
| FUN_1800663c0 | 5 层嵌套 vector 拷贝构造 | 4 |

与核心共用、在本子系统中用到的辅助函数（作用即标准库语义，不单独写规格）：FUN_180064780 / FUN_1800655c0 / FUN_180065db0（int 下标 introsort，核心 FUN_180068040 也用）、
FUN_180064f20、FUN_1800664d0（4/3 层 vector 析构与拷贝）、FUN_1800255a0、FUN_1800251d0、FUN_180024c70（vector 拷贝 / emplace）、FUN_18000f7f0（vector<vector<double>> 扩容）、
FUN_18004a890、FUN_180023ef0（ifstream 构造/打开）、FUN_180023f70、FUN_180024220、FUN_180024180（ofstream 构造/打开/关闭）、FUN_1800247f0、FUN_18004b710（ofstream / ifstream 以 (name, mode) 构造）、
FUN_1800033f0（pow(x, n)）、FUN_1800010a0（fabs）、FUN_180001c40 / FUN_18000ec60 / FUN_18000f190（vector::operator[]）、FUN_180001d50 / FUN_1800020c0 / FUN_18000f1a0（push_back）、
FUN_180001980 / FUN_180001ba0 / FUN_180001ef0（ostream << 字符串）、FUN_18001dba0 / FUN_18004b690 / FUN_18001c930 / FUN_180049f80 / FUN_180049eb0（字符串拼接）、FUN_180049d30 / FUN_1800256c0（to_string(int)）、
FUN_180050e00 / FUN_180051000（ThreadPool 构造/析构）。

为 6 节比较而查看的核心函数（规格属其他子系统）：FUN_180030ba0（InitLattice）、FUN_180039dd0（InitMethodMP）、FUN_18006b990（Run）、FUN_180070b90（tcodeRun）、
FUN_180039f20 / FUN_180042860（SetSynPhase / singlePartM）、FUN_180027eb0（InitParticle）、FUN_180058440（Beam 构造）、FUN_180078570、FUN_180055ee0（path）、FUN_180056390（main_agent）。

---

## 9. 未解决的问题

1. **Beam+0xe8（spacechargecomp 系数）在哪里被读取**：伪代码与反汇编中都没找到。实验：Windows 上用示例项目开空间电荷，在第一个元件前加 `spacechargecomp 0`、`0.5`、`1`，
   比较 DataSet.txt 的 rms 包络；若 0 时与 `current 0` 一致，则为空间电荷力乘子。再在 Linux 用 .so 跑同一 lattice，确认 .so 忽略该行（或报错）。
2. **sampling / dataprocess 的排序在键相等时的顺序**：取决于 MSVC `std::sort` 实现。实验：构造有大量相同 r（例如粒子坐标量化到网格）的输入，看输出是否与 Python 稳定排序结果一致；
   重建时如需逐位对齐，要照搬 MSVC introsort（阈值 32、三数取中、`(n>>1)+(n>>2)` 深度预算）。
3. **sampling 的 SIMD 求和顺序**（D 与 MSE 的 4 展开）造成的末位差异：建议对齐测试用相对容差 1e-9，不要求逐位。
4. **FiveCube 的命名与 dataprocess 的真实用途**：推测为"压缩分布的模拟结果"，需开发者确认；也可能 FiveCube 指 5σ 截断的立方体网格。
5. **outData_*.dst 是否只含存活粒子**、文件名中 z 的格式（是否总能用 `_` 后 8 字符解析）：属于内核输出格式（D 组），需对照实际文件。
6. **Windows 与 Linux 的 `rand` 差异**是否导致同一 randomseed 初始分布不同：用同一 beam.txt 在两个平台各写一次 inData.dst 比较。
7. **6.3 节死字符串对应的源码分支**：无法从二进制恢复；如能找到 DLL 源码的更晚版本可核对。

---

## 10. 对结构表的更正 / 补充

`structures.md` 描述的是 .so；对 DLL 需要以下换算（证据见 6.5）：

- Particle 在 DLL 中步长 **0x1a8**，原因是 MSVC `std::string` 占 32 字节（.so 的 0x188 `kind` 之后多 24 字节），其余字段偏移不变。
  证据：FUN_180078570 与 FUN_180070b90 中 `index * 0x1a8 + {0x00, 0x20, 0x28, 0x104}`。
- Beam：DLL 在 **+0xe8** 有一个 double，构造为 1.0，由 `spacechargecomp` 改写（FUN_180058440、FUN_180070b90 第 2454 行）。.so 的 Beam 初始化不写该位置，
  因此 structures.md 第 1 节"Beam sizeof 0xe8 或 0xf0"中，.so 的 0xe8–0xef 若存在也未被使用。
- DLL 的 Element、Initial、MethodMP 因 string 尺寸不同整体后移（例：Element zStart 0x60、Initial 输出面 0x1f0、MethodMP 输出面 0x290），
  另有新成员 Initial+0x258、MethodMP+0x2c0（`vector<vector<double>>`，spacechargecomp 表，每条 `{z, 1.0, 系数}`）。

## 实测核实：spacechargecomp（2026-11-07，Windows AVAS.dll）

示例 `examples/hwr010`（单个 `field` 元件），beam.txt 改为 `current 20`（mA）、加 `randomseed 12345`，在 lattice 第二行插入 `spacechargecomp V`，比较末端 DataSet.txt 的 rms x：

| V | rms x（mm） | 等效空间电荷系数 |
|---|---|---|
| 关闭空间电荷（spacecharge 0） | 1.84327 | 0 |
| 不写该行 | 2.26262 | 1 |
| −0.5 | 2.44295 | 1.5 |
| 0 | 2.26262 | 1 |
| 0.001 | 2.26224 | 0.999 |
| 0.25 | 2.16631 | 0.75 |
| 0.5 | 2.06499 | 0.5 |
| 0.75 | 1.95773 | 0.25 |
| 0.999 | 1.84374 | ≈0 |
| 1、1.5、2 | 2.26262 | 1（被忽略） |

结论：**空间电荷力乘以 (1 − V)，只在 V < 1 时生效；V ≥ 1 时这一行被静默忽略**（所以写 `spacechargecomp 1` 想表示"完全补偿"，实际得到的是完整的空间电荷）；负值会放大空间电荷。写了这一行后 DataSet.txt 多出 6 行（101 → 107），原因待查。.so 是否忽略该关键字尚未在 Linux 上实测。
