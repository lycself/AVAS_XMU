# 30 元件的场与硬边矩阵传输（规格）

对齐清单前缀 `C-`。依据：`libAVAS.so` 的 Ghidra 伪代码（`decomp/libAVAS/functions/`），关键处用 `objdump -d` 直接核对了反汇编
（文中写"反汇编核对"的地方，Ghidra 伪代码有遗漏或错误）。结构字段名按 [structures.md](structures.md)。
地址一律写 Ghidra 地址（镜像基址 0x100000，`objdump` 地址 = Ghidra 地址 − 0x100000）。

## 1. 概述

本子系统负责"粒子在某个位置受到什么外场、硬边元件怎样一步传过去"：

| 部分 | 作用 | 入口 |
|---|---|---|
| 元件定位 | 按粒子 z 找到所在元件，调用该元件的 `GetField` | `Lattice::GetExternalField(double t, Beam&)`（主束 t-code）、`Lattice::GetExternalField(double t, Particle&)`（单个粒子，二次粒子路径） |
| 元件场（t-code） | 各元件给出 E、B（V/m、T），写入 `Particle.extE/extB` 或 `SynParticle.E/B` | 虚函数槽 +0x10 `GetField(t, Particle&, fm)`、+0x00 `GetField(t, SynParticle&, fm)` |
| 场图库 | 读场图文件进 `FieldMatrix` | `Lattice::SetMatrix(name, type, fm)`（由 `Lattice::SetElement` 调用，type = field 行 word4，name = word9） |
| RFQ | 8 项势展开的场；同步粒子穿过 RFQ 时逐 cell 求同步相位并写 `RFQsynPhaseEG.txt` | `RFQ::GetField`、`RFQ::getEleNum/getRadius`、`Bessi0/Bessi1/Bessin` |
| 硬边传输（z-code） | quad / solenoid / bend / edge 四种硬边元件按传输矩阵整段推进（含误差坐标变换、半步空间电荷冲量） | `MethodMP::HardEdgeTrans`（1 个调度函数 + 14 个 OpenMP 子函数 + 1 个同步粒子版本） |

运行流程中的位置（调用方属于其他子系统，这里只写接口）：

- `MethodMP::Run` 把 lattice 切成 t-code 段与 z-code 段：类型为 `quad`、`solenoid`、`bend`、`edge` 的元件进 z-code，其余（drift、field、RFQ、steerer 等）走 t-code。
  第一个元件不允许是硬边元件（报 "Do not allow the first element to be a hard-edged model element; very short drift can be added at the beginning"）。
- t-code（`MethodMP::tcodeRun`）每个时间步调用 `Lattice::GetExternalField(t, Beam&)` 取外场，再 Boris 推进。
- z-code（`MethodMP::zcodeRun`）对段内每个硬边元件调用 `HardEdgeTrans`（见 2.8.1），最后调用同步粒子版本 `HardEdgeTrans(int, rows, Beam&, Lattice&)`。
- 同步粒子求相位（`Initial::SetSynPhase`、`singlePartM`）调用各元件的 `GetField(t, SynParticle&, fm)`（经 `SynParticle::GetField`）。

**重要结论（与 t-code / z-code 的分工有关）**：解析元件在 t-code 里的场非常简化——bend、edge、steerer 的 `GetField` 恒为零，solenoid 只有 Bz（无边缘径向场），
quad 只有 Bx=G·y、By=G·x；同步粒子（SynParticle）在所有解析元件里场恒为零。真正的物理只在 z-code 的 `HardEdgeTrans` 里。

## 2. 规格正文

### 2.1 约定

- 常数：c = 299792458.0；2c = 599584916.0；c² = 8.987551787368176e16；π = 3.141592653589793；2π = 6.283185307179586；π/2 = 1.5707963267948966；1e6（MV/m→V/m）。
- `sincos(a, &s, &c)`：s = sin a，c = cos a。下文 "(s,c)=sincos(a)"。
- 公式里的乘法顺序按伪代码写成左结合，例如 `w·F·1e6·Ke` 表示 `((w*F)*1e6)*Ke`；**同一个物理量在不同函数里乘法顺序不同（会差最后一位），按各节写的顺序实现**。
- 浮点比较：`a ≤ b` 等写法即原内核的比较方向，边界相等时的归属按此实现。
- z-code 的每个粒子一行 `row`（`vector<double>`，`rows` 是 `vector<vector<double>>`，下标 i 对应 `Beam.particles[i]`，下标 N 是同步粒子）：

| row 下标 | 含义 |
|---|---|
| [0] | x（m） |
| [1] | γβx（传输过程中临时改写成 x' = γβx/γβz，结束前乘回） |
| [2] | y（m） |
| [3] | γβy（同上，临时为 y'） |
| [4] | 最近一次推进所用的时间 Δt = s/v_z（s） |
| [5] | γβz |
| [6] | 到达当前 z 的时间 t（s），每次推进累加 Δt |
| [7] | 存活标志，1.0 = 存活（只有 `row[7] == 1.0` 的粒子被推进） |

  证据：`MethodMP::zTotcode`（0x19df10、0x1abb50）用 [0..3]、[5]、[6]、[7] 还原 t-code 坐标。

### 2.2 元件定位：`Lattice::GetExternalField`

#### 2.2.1 主束版本 `GetExternalField(double t, Beam&)`（0x188180）

对 i = 0 … N（**含同步粒子 N**，共 N+1 个）依次处理粒子 p：

1. 令 z = p.z，`row6 = spOut[spOutCur][6]`（`Lattice.spOut` 第 `spOutCur` 行的出口 z）。
2. 若 `z < 0` 或 `z > row6` 或 `p.lost != 0`：
   - 若再满足 `z > latticeEnd`（`elemZEnd` 最后一个元素）且 `spOutCur == spOut.size()` 且 `p.lost == 0`，置 `p.lost = 2`；
   - 把 `extE[0..2]`、`extB[0..2]` 全部清零；处理下一个粒子。
   - 说明：`InitLattice` 在 `spOut` 末尾追加一行哨兵 `{0,0,0,0,0,0, latticeEnd}`，所以 `z > row6` 在没有 superposeout 时就是"越过 lattice 末端"，此时只清场、等待其余粒子。
     `spOutCur == spOut.size()` 时第 2 步读 `spOut[spOutCur]` 越界（原内核未定义行为），新内核应把越界的 `row6` 视为 +∞（推测原意）。
3. 否则在 `elemZStart`/`elemZEnd` 上从 `k = p.elemIndex` 线性搜索：
   - 若 `z < zStart[k]`：反复 `k--` 并写回 `p.elemIndex = k`，直到 `zStart[k] ≤ z`；若 k 变成负数，置 `p.elemIndex = 0`，输出 `Warning! beam.particle[i].indexElem<0` 并换行，再从 k=0 重新判断；
   - 否则若 `zEnd[k] ≤ z`：反复 `k++` 直到 `zEnd[k] > z`，写回 `p.elemIndex = k`（**没有上界检查**：z 恰好等于 latticeEnd 时会越过最后一个元件；新内核此时应停在最后一个元件）。
4. 调用 `elements[k]->GetField(t, p, fm)`（虚函数槽 +0x10，fm = `Lattice+0x18`）。

#### 2.2.2 单粒子版本 `GetExternalField(double t, Particle&)`（0x1889e0）

1. 若 `z < 0` 或 `z > latticeEnd` 或 `lost != 0`：**只清零 `extE[0]` 和 `extB[0]`（Ex、Bx）**，其余四个分量保持原值，返回（反汇编核对：只有两条 `movq $0x0`）。不设 lost=2。
2. 搜索规则与 2.2.1 略有不同：`z < zStart[k]` 时递减到 `zStart[k] ≤ z`（警告文字为 `Warning! particle.indexElem<0`）；`z == zStart[k]` 时不动；否则若 `zEnd[k] < z`，递增到 `zEnd[k] ≥ z`。
   即**主束版本把 z == zEnd 的粒子归到下一个元件，单粒子版本归到当前元件**。
3. 调用 `GetField`（槽 +0x10）。

### 2.3 解析元件的 t-code 场

下表 p 为 Particle，S 为 SynParticle。"孔径判丢失"条件统一记为

```
APERTURE_LOST(R) :=  R ≤ sqrt(x²+y²)  且  p.boundaryMode == 0
                     且  elem.zStart + 1e-6 < p.spOutZ1  且  p.spOutZ0 < p.z
```

| 元件 | `GetField(t, Particle&)` | `GetField(t, SynParticle&)` |
|---|---|---|
| Drift（0x127080 / 0x127340） | 若 `APERTURE_LOST(radius)`：`p.lost = 1`；**然后**无论如何 extE = extB = 0 | 若 `radius ≤ sqrt(x²+y²)`：`S.lost = 1`，输出 `Synchronised particles can't reach the exit plane, problems with the beamline` 换行，`exit(1)`；否则 E = B = {0,0,0} |
| Quadrupole（0x1d02a0 / 0x1d05c0） | Ex=Ey=Ez=0，**Bx = y·G，By = G·x**，Bz = 0（G = `strength` +0x58，含已叠加的误差 dG；无 z 边界、无孔径判断） | E = B = {0,0,0} |
| Solenoid（0x1d3540 / 0x1d3890） | E = 0，Bx = By = 0，**Bz = B**（`strength`）；没有边缘径向场 | E = B = {0,0,0} |
| Steerer（0x1d3d50 / 0x1d3db0） | E = B = 0（校正铁的作用见 2.9） | E = B = {0,0,0} |
| Bend（0x125c80 / 0x125df0） | E = B = 0（弯转只能在 z-code 里传） | E = B = {0,0,0} |
| Edge（0x127600 / 0x127770） | E = B = 0 | E = B = {0,0,0} |
| Element 基类（0x126970 / 0x126950） | 空函数，不写任何值（所有子类都覆盖，实际不会被调用） | 空函数 |

SynParticle 版本用 `vector::assign` 把 E、B 赋为三个 0（静态初始化表 `._97`/`._98` 的内容已从 .rodata 读出确认为 0.0）。
除 Drift 和 Field 外，**t-code 里的解析元件都不判孔径**。

`Element::getRadius(z)`（0x1269b0）：返回 `radius`（+0x28），与 z 无关。RFQ 覆盖为按 cell 返回（2.7.1）。

### 2.4 场图元件：`Field::GetField(double t, Particle&, FieldMatrix const&)`（0x12cb30）

按 `fieldType`（+0x8c）分派：**1 = 射频（6 分量），2 = 静电（3 分量 → E），3 = 静磁（3 分量 → B），4 = 叠加组**；其他值直接返回、不写任何场。

#### 2.4.1 类型 1、2、3 的共同步骤

1. **孔径**：若 `APERTURE_LOST(radius)`，置 `p.lost = 1` 并立即返回，**不修改 extE/extB**（保持上一步的值；下一次 `GetExternalField` 因 lost≠0 清零）。
   （反汇编核对：0x12d441 处 `movl $1, 0x100(%r15)`，Ghidra 伪代码漏掉了这条写入。）
2. **网格范围**（m = `mapIndex`）：若 `x ≤ xmin[m]` 或 `x ≥ xmax[m]` 或 `y ≤ ymin[m]` 或 `y ≥ ymax[m]`，或 `z ≤ zStart` 或 `z ≥ zStart + zlen[m]`：
   把 `zeroField`（{0,0,0}，+0xf8）逐分量拷给 extE 和 extB，返回。注意纵向范围用**场图长度 zlen**，不是元件长度 L；场图入口与元件入口 zStart 对齐。
3. **三线性插值**：
   ```
   fx = (x − xmin[m]) / dx[m];   fy = (y − ymin[m]) / dy[m];   fz = (z − zStart) / dz[m]
   for c = 0..7:
       ix = (int)(ceil(fx) − (c & 1 ? 1 : 0))
       iy = (int)(ceil(fy) − ((c & 3) ≥ 2 ? 1 : 0))
       iz = (int)(ceil(fz) − (c ≥ 4 ? 1 : 0))
       n  = (iy + iz·(ny[m]+1))·(nx[m]+1) + ix          # x 最快、y 次之、z 最慢
       w  = (1 − |fx − ix|)·(1 − |fy − iy|)·(1 − |fz − iz|)
       F  = maps[m][n]
       累加（各分量从 0.0 起，按 c = 0..7 顺序相加）
   ```
   由于第 2 步用严格不等式排除了边界，ceil(fx) ≥ 1，所有角点都在网格内。fx 恰为整数时两个角点权重为 1 和 0。
4. 各类型的累加与输出：

| 类型 | 累加 | 输出 |
|---|---|---|
| 1 RF | `SE[j] += w·F[j]·1e6·Ke`（j=0..2），`SB[j] += w·F[3+j]·Kb` | φ = `frequency·2π·(t − fm.cavTime[cavIndex]) + (fm.cavPhase[cavIndex]/180)·π`；cE = cos φ，cB = cos(φ − 1.5707963267948966)；extE = SE·cE，extB = SB·cB |
| 2 静电 | `S[j] += w·F[j]·1e6·Ke` | extE = S，extB = 0 |
| 3 静磁 | `S[j] += w·F[j]·Kb`（F[0..2] 存的是 Bx、By、Bz，见 2.6） | extE = 0，extB = S |

   Ke、Kb 用 `ke`(+0x90)、`kb`(+0x98)，不是误差版的 `keErr/kbErr`（误差版在 `GetErrField`，不属本文）。
   **RF 相位只用 `FieldMatrix.cavPhase/cavTime`，元件自己的 word6 相位（+0x110）在这里不用**（由同步相位子系统写进 cavPhase）。
   单位：场图电场 MV/m，×Ke×1e6 得 V/m；磁场 T，×Kb；E 按 cos φ、B 按 sin φ（与 AGENTS.md 第 6 节一致）。

#### 2.4.2 类型 4：叠加组（superpose）

先把 extE、extB 六个分量清零，再对每个子场 j = 0 … n−1（n = `subZStart.size()`）：

1. 局部坐标（P = `subPlace[j]`；P[0] 在这里不用）：
   ```
   dz = z − subZStart[j];   X0 = x − P[1];   Y0 = y − P[2]
   (s,c)=sincos(P[3]):  u = c·X0 + s·Y0;        v = Y0·c − X0·s        # 绕 z
   (s,c)=sincos(P[5]):  xl = c·u − dz·s;       w = u·s + dz·c          # 绕 y
   (s,c)=sincos(P[4]):  zl = w·c − v·s;        yl = s·w + v·c          # 绕 x
   ```
2. 孔径：若 `0 < zl < subLength[j]` 且 `zStart + 1e-6 < p.spOutZ1` 且 `p.spOutZ0 < z`：
   - 若元件 +0x6c == 0（束流坐标未旋转），r = sqrt(xl² + yl²)；否则 r = sqrt(rel[0]² + rel[2]²)（相对同步粒子的坐标，Particle +0xc0、+0xd0）；
   - 若 `subRadius[j] ≤ r`：`p.lost = 1`，返回（**此时 extE/extB 里留有前面子场的部分和**）。
   - 注意这里**不检查 `boundaryMode`**（反汇编核对：该分支不读 +0x178）。
3. 按 `subType[j]`（**编码与顶层不同：1 = 静磁，2 = 射频，3 = 静电**；`Field::SetParameter` 把子块 word4 的 1 存成 2、3 存成 1）取 m = `subMapIndex[j]`，
   要求 `xmin<xl<xmax`、`ymin<yl<ymax`、`0<zl<zlen[m]`（严格不等式），否则跳过该子场。插值同 2.4.1，但 `fx=(xl−xmin)/dx`，`fy=(yl−ymin)/dy`，`fz = zl/dz`。
4. 子场累加与转回实验室系（a、b、d、cz 为临时量；(s3,c3)=sincos(P[3])，(s4,c4)=sincos(P[4])，(s5,c5)=sincos(P[5])，按 P[4]、P[5]、P[3] 的顺序调用 sincos）：

   - 静磁（subType 1）：`S[j'] += w·F[j']·subKb[j]`；
     ```
     a = Sy·c4 − s4·Sz;   b = Sy·s4 + c4·Sz;   cz = b·c5;   d = Sx·c5 + s5·b
     Bx += c3·d − a·s3;   By += s3·d + a·c3;   Bz += cz − s5·Sx;     E 三个分量 += 0.0
     ```
   - 静电（subType 3）：`S[j'] += w·F[j']·subKe[j]·1e6`（**注意顺序是 ·Ke·1e6，与顶层静电 ·1e6·Ke 不同**），同样公式加到 E 上，B += 0.0。
   - 射频（subType 2）：`SE += w·F[0..2]·1e6·subKe[j]`，`SB += w·F[3..5]·subKb[j]`；φ、cE、cB 同 2.4.1，**但用叠加组元件自身的 `frequency` 和 `cavIndex`**
     （`SetParameter` 用最后一个 RF 子块的频率、相位覆盖组的 +0xa8/+0x110）。
     ```
     a = cE·Sy·c4 − cE·Sz·s4;     b = s4·cE·Sy + cE·Sz·c4;     cz = b·c5;     d = Sx·cE·c5 + s5·b
     Ex += c3·d − a·s3;   Ey += d·s3 + a·c3;   Ez += cz − s5·Sx·cE
     a' = cB·By·c4 − cB·Bz·s4;    b' = cB·By·s4 + cB·Bz·c4;    cz'= b'·c5;    d' = Bx·cB·c5 + s5·b'
     Bx += c3·d' − a'·s3; By += s3·d' + a'·c3; Bz += cz' − s5·Bx·cB
     ```
     （这里 Sx,Sy,Sz 指 SE 的分量，Bx,By,Bz 在右边指 SB 的分量。）

### 2.5 `Field::GetField(double t, SynParticle&, FieldMatrix const&)`（0x1305d0）

同步粒子版，坐标取 `S.coord`（[0]=x、[2]=y、[4]=z）。与 Particle 版的差别：

1. 孔径：类型 1、2、3 若 `radius ≤ sqrt(x²+y²)`，输出 GBK 编码字符串"同步粒子无法抵达出口平面，束线存在问题,模拟结束"（.rodata 0x1d77c0，原样输出字节）并换行，`exit(1)`。
   不看 boundaryMode、spOut。
2. 越界（同 2.4.1 第 2 步，严格不等式）：`E = zeroField`，`B = zeroField`（vector 赋值）。
3. 类型 1 RF：**只累加 Ez**：`EzAmp = Σ_c w·F[2]·1e6·Ke`（从 0.0 起按 c 顺序累加）；
   φ = `frequency·2π·(t − S.cavTime[cavIndex]) + (S.cavPhase[cavIndex]/180)·π`；
   `E = (0, 0, cos φ · EzAmp)`，`S.EzAmp = EzAmp`，`B = {0,0,0}`（反汇编核对：0x133094 起写 E[2]、EzAmp、E[0]=E[1]=0）。
4. 类型 2：`E[j] = Σ w·F[j]·Ke·1e6`（**·Ke·1e6** 顺序），`B = 0`。类型 3：`B[j] = Σ w·F[j]·Kb`，`E = 0`（反汇编核对 0x13315e、0x1331d0）。
5. 类型 4：先 E=B=0；局部坐标同 2.4.2；孔径条件为 `0 < zl < subLength[j]` 且元件 **+0x68（inSuperposeOut）== 0** 时 r = sqrt(xl²+yl²)，`subRadius[j] ≤ r` → 同第 1 步退出程序。
   子场累加和旋转公式与 2.4.2 相同（RF 用 `S.cavTime/S.cavPhase`），RF 子场结束时 `S.EzAmp = SE[2]`（**该子场局部系、未乘 cos、未旋转**的 Ez 幅值，多个 RF 子场时取最后一个）。

### 2.6 场图读取：`Lattice::SetMatrix(string name, int type, FieldMatrix& fm)`（0x188bb0）

1. 若 `name` 已在 `fm.names` 中，返回它的序号（`std::find`），不重读。
2. **name 最后一个字符为 `'b'` 时按二进制读，否则按文本读**（name 已含路径前缀；文本场图名以 b 结尾会被误当成二进制）。
3. 文件名为 `name + 扩展名`：type 1 → `.edx .edy .edz .bdx .bdy .bdz`（节点 6 分量 [Ex,Ey,Ez,Bx,By,Bz]）；type 3 → `.bsx .bsy .bsz`（3 分量，存在 F[0..2]）；type 2 → `.esx .esy .esz`（3 分量）。
   打不开时抛出 `std::string` 异常 `"Element field file cannot be opened : " + 文件名`。type 不是 1/2/3 时不读任何文件，返回值未定义（原内核返回流析构的返回值）。
4. **文本 3D 格式**（每个分量文件，值用 `operator>>` 读）：
   ```
   第 1 行: nz zlen            （int, double；zlen 单位 m）
   第 2 行: nx xmin xmax       （int, double, double；m）
   第 3 行: ny ymin ymax
   第 4 行: norm               （用 getline 读整行，再 stoi —— strtol 十进制，"1.0" 得 1，"2.5" 得 2）
   其后 (nx+1)(ny+1)(nz+1) 个数，循环顺序 iz 外层、iy 中层、ix 内层
   节点 n = ((ny+1)·iz + iy)·(nx+1) + ix，maps[n][分量] = 值 / (double)norm
   ```
   只有第一个分量文件的头部被解析并决定网格；其后的分量文件跳过 4 行头（getline 4 次），网格假定相同。
   dx = (xmax−xmin)/nx，dy = (ymax−ymin)/ny，dz = zlen/nz（double 除以 int 转 double）。
   `examples/hwr010/InputFile/hwr010.edz` 头部为 `105 0.21 / 20 -0.02 0.02 / 20 -0.02 0.02 / 1`，46746 个数，与此一致。
5. **文本 1D（只对 type 1）**：若 `name.edx` 打不开，输出 `The one-dimensional field has been expanded into a three-dimensional field.          `（原样含尾部空格）并换行，改读 `name.edz`：
   ```
   第 1 行: nz zlen
   第 2 行: mark  （stoi；不等于 1 时输出 "mark error : " + 文件名 + "  " + 该行 + 换行，继续）
   其后 nz+1 个 Ez 值（不除以 norm）
   ```
   网格设为 nx = ny = 2，xmin = ymin = −1.0，xmax = ymax = 1.0（dx = dy = 1.0），每个 z 截面 9 个节点的 F[2] 都等于该 Ez，其余 5 个分量为 0。
   即 **1D 场在 |x|,|y|<1 m 内横向均匀，没有 Er、Bθ**。`.edz` 打不开时抛同样的异常。
6. **二进制格式**（每个分量文件，`ifstream(name+ext, in|binary)`，小端、无填充）：
   ```
   int32 nz; float64 zlen; int32 nx; float64 xmin; float64 xmax; int32 ny; float64 ymin; float64 ymax; float64 norm;
   随后 (nx+1)(ny+1)(nz+1) 个 float32，顺序同文本；maps[n][分量] = (double)value / norm
   ```
   每个分量文件都重读头部（覆盖 cur*），但节点数组按第一个文件分配；dx、dy、dz 只在第一个文件后计算。二进制没有 1D 回退。
7. 读完后依次 push：`maps`、`names`（name）、`nMaps++`、`nx, ny, nz, xmin, xmax, ymin, ymax, zlen, dx, dy, dz`，返回 `nMaps − 1`。

### 2.7 RFQ

#### 2.7.1 `RFQ::getEleNum(z)`（0x1d0ad0）与 `RFQ::getRadius(z)`（0x1d0bb0）

- `getEleNum`：若 cell 数（按 `cellZStart` 计数）为 0，或 z 大于最后一个 `cellZEnd`，输出 `Particle in RFQ error: exceed the RFQ length but still in RFQ` 换行，返回 −1；
  否则返回第一个满足 `z ≤ cellZEnd[i]` 的 i（i 从 0 起；不检查 z 是否小于 cellZStart[0]）。
- `getRadius`：`i = getEleNum(z)`；i == −1 时返回 **1e99**（0x547d42aea2879f2e），否则返回 `cellAperture[i]`（m）。

#### 2.7.2 Bessel 函数（0x14a870 / 0x14a960 / 0x14aa70）

都是 20 项级数，k = 0 … 19，按 k 递增顺序累加到 0.0 上，`inv_k! ` 用 `1.0` 依次除以 1、2、…、k 得到：

```
Bessi0(x)   = Σ  pow(x·0.5, 2k)    / tgamma(k+1)   · inv_k!      （k=0 项用 pow(x·0.5, 0.0)）
Bessi1(x)   = Σ  pow(x·0.5, 2k+1)  / tgamma(k+2)   · inv_k!      （k=0 项分母写成 tgamma(2.0)）
Bessin(n,x) = Σ  pow(x·0.5, n+2k)  / tgamma(n+k+1) · inv_k!
```
每项计算为 `(pow(...) / tgamma(...)) * inv_k!`。只有 `Bessin` 被 RFQ 调用（n = 0、1）。

#### 2.7.3 `RFQ::GetField(double t, Particle&, FieldMatrix const&)`（0x1d0be0）

1. `i = getEleNum(p.z)`。若 i == −1，先把 `zeroField` 拷给 extE、extB，**但不返回**，继续用 i = −1 越界读 cell 数据并覆盖 extE（反汇编核对：0x1d1740 处理完跳回 0x1d0c16）。新内核应在此返回零场（推测原意），并作为已知差异记录。
2. 记号：θ = atan2(y, x)，r = sqrt(x²+y²)；Lc = `cellLength[i]`，ζ = z − `cellZStart[i]`，k = π / Lc，A[0..7] = `cellCoef[i]`（8 个系数），V = `cellVoltage[i]`，a = `cellAperture[i]`，σ = pow(−1.0, (double)i)。
3. 若 r ≥ eps（RFQ +0x170 = 1e-16）：Vh = |V|·0.5，各 sin/cos 记 s2=sin2θ、c2=cos2θ、s4、c4、s6、c6、s1=sin kζ、c1=cos kζ、s3z=sin 3kζ、c3z、s2z=sin 2kζ、c2z。
   ```
   Er = ( ((A0+A0)·r)/(a·a)·c2  +  ((r^5·A2·6)/a^6)·c6  +  I1(k·r)·A1·k·s1·σ  +  A6·3·k·s3z·σ  +  A3·k·s1·c4·σ
          +  A7·3·k·s3z·c4·σ  +  (A4+A4)·k·s2z·c2·σ  +  (A5+A5)·k·s2z·c6·σ ) · Vh
   ξ = r/a;  T0 = ξ·ξ·A0·s2;  T4 = A4·s2z·s2
   Eθ = ( −|V|·0.5·( T0+T0 + A2·pow(ξ,6)·s6·6 + s1·A3·s4·4·σ + s3z·A7·s4·4·σ + (T4+T4)·σ + s2z·A5·s6·6·σ ) ) / r
   Ez = ( I0(k·r)·A1·c1·k + A6·c3z·3·k + c1·A3·k·c4 + c3z·A7·3·k·c4 + (A4·c2z + A4·c2z)·k·c2 + (c2z·A5 + c2z·A5)·k·c6 ) · Vh · σ
   ```
   （I0 = Bessin(0,·)，I1 = Bessin(1,·)；r^5、a^6 用 `pow(r,5.0)`、`pow(a,6.0)`。）
   这相当于对势 U = Vh·[A0 ξ² cos2θ + A2 ξ⁶ cos6θ + σ(A1 I0(kr) sin kζ + A6 sin3kζ + A3 cos4θ sin kζ + A7 cos4θ sin3kζ + A4 cos2θ sin2kζ + A5 cos6θ sin2kζ)]
   取 **E = +∇U**（Er=∂U/∂r，Eθ=(1/r)∂U/∂θ，Ez=∂U/∂z）；**除 A0、A1、A2 项外，高次项都省略了 Bessel 因子**（I0(3kr)、I4(kr)、I2(2kr) 等按 1 处理）。照原样实现。
4. 若 r < eps：Er = Eθ = 0，`Ez = −|V|·0.5·(I0(r·k)·A1·c1·k + c3z·A6·3·k)·σ`。**注意与第 3 步的 Ez 符号相反**（原内核在轴上和轴外 Ez 反号；同步粒子用的 `RFQ::GetSynField` 与轴上公式一致）。
5. (sθ,cθ)=sincos(θ)；cφ = cos(`frequency`·2π·(t − fm.cavTime[cavIndex]) + (fm.cavPhase[cavIndex]/180)·π)；
   `Ez_out = Ez·cφ`，`Ex = (cθ·Er − sθ·Eθ)·cφ`，`Ey = (Er·sθ + Eθ·cθ)·cφ`，B = `zeroField`。
   RFQ 的 t-code 场里没有孔径判断（丢失由别处用 `getRadius` 判断，调用方不在本子系统）。

#### 2.7.4 `RFQ::GetField(double t, SynParticle&, FieldMatrix const&)`（0x1d29a0）：同步粒子逐 cell 求相位

这是同步相位子系统穿过 RFQ 时调用的专用积分器（虚函数槽 +0x00）：

1. dt = (0.05 / f) / 500.0（f = RFQ `frequency`，Hz）。
2. 输出到 stdout：.rodata 0x1dcbf9 起 0x17 字节（损坏的中文"进入RFQ时间："，原样输出这些字节）、t（`ostream<<double`，默认精度 6）、0x1dcc11 起 0x1a 字节（损坏的"入口能量"）、`(sqrt(γβz²+1) − 1)·restMassMeV`，换行。
3. 设 i = 0（cell），C = S = 0，τ = t。循环：
   - `GetSynField(τ)`（取轴上 Ez，见 2.7.5）；记下 z_prev、γβz_prev；`SynParticle::Update(dt)`。
   - 若新 z ≤ `cellZEnd[i]`：φ = (cavPhase[cav]/180)·π + f·2π·(τ − cavTime[cav])（SynParticle 的 cav 向量）；τ += dt；
     `C += cos φ · EzAmp · (z − z_prev)`；`S += EzAmp · sin φ · (z − z_prev)`；回到循环开头（重新 GetSynField）。
   - 否则（越过 cell 末端）：λ = (zEnd − z_prev)/(z − z_prev)；`z = (z − z_prev)·λ + z_prev + 1e-15`；`γβz = (γβz − γβz_prev)·λ + γβz_prev`；
     用 τ（本步开始时刻）算 φ，Δ = z − z_prev，`C' = cos φ·EzAmp·Δ + C`；
     相位 φs_i = C' == 0 ? −90.0 : `atan((sin φ·EzAmp·Δ + S) / C')/π·180`（**atan 不是 atan2**）存入相位表；γβz 存入能量表。
     若能量表最后一个值 < 倒数第二个值（γβz 减小），置 `coord[1] = 1.0` 作为失败标志，结束（不写文件）。
     （第一个 cell 时能量表只有一个元素，原内核读越界；新内核第一个 cell 不做该比较。）
     i++，τ += λ·dt；若 i ≥ cell 数：输出 0x1dcc2c 起 0x11 字节（"离开RFQ时间："）、τ、0x1dcb98 起 0x20 字节、出口能量，换行；
     若 `coord[1] != 1.0`：`coord[0] = τ`（**把 x 改写成离开时间**，供调用方读取），写 `outputFilePath + "RFQsynPhaseEG.txt"`：
     每个 cell 一行 `φs_i(deg)` + 三个空格 + `(sqrt(γβz_i²+1)−1)·restMassMeV`，`std::fixed`、精度 10，行尾 `'\n'` 并 flush；结束。
     否则 C = S = 0，`GetSynField(τ)`，继续循环。

#### 2.7.5 相关但不属本文的 `RFQ::GetSynField`

供参考：z 超过 RFQ 末端时 E=B=0 返回；否则轴上 `Ez = −|V|·0.5·(I0(0)·A1·cos kζ·k + cos3kζ·A6·3·k)·σ`，`E=(0,0,cos φ·Ez)`，`EzAmp = Ez`（含负号），B=0。

### 2.8 硬边传输 `MethodMP::HardEdgeTrans`

#### 2.8.1 调用框架（调用方 `zcodeRun`，属 D 子系统，这里只写约定）

对 z-code 段内每个硬边元件 e（下标 idx，入口 z0 = e.zStart，长度 L = e.length），`attach` 为 Run 生成的附属标志
（≥0：紧挨在前面的 steerer 的元件下标；−1：有输出面落在 (z0, z0+L) 内；−2/−3：其他）：

- 若 `(spaceCharge != 1 且 attach ≤ −1.5)` 或 e 是 `edge`：一次调用 `HardEdgeTrans(idx, z0, z0+L, rows, beam, lat, mode=0, tRef, err)`。
- 否则分两半：`mode=2` 调用 [z0, z0+L/2]；若空间电荷开启，在中点计算空间电荷场（`HESCFieldPre` + PIC，写 `Particle.scE`）并置 `mode=1`；
  attach == −1 时中点输出 dst；attach ≥ 0 时中点施加校正铁冲量（2.9）；再调用 [z0+L/2, z0+L]（mode 为 0 或 1）。
- 最后调用同步粒子版本 `HardEdgeTrans(idx, rows, beam, lat)`（2.8.7），用**整段**长度推进第 N 行。

`mode`（int&）与 `tRef`（double&，后续 `zTotcode` 的参考时间）的约定见下节；**edge 既不改 mode 也不改 tRef**。

#### 2.8.2 调度函数 `HardEdgeTrans(int idx, double zFrom, double zTo, rows&, Beam&, Lattice&, int& mode, double& tRef, ErrorSample& err)`（0x1aa5c0）

按 `elements[idx]->typeName` 比较字符串（字面量从 .rodata 恢复："quad" 0x1d7bf3、"solenoid" 0x1d7bf8、"bend" 0x1d7c01、"edge" 0x1d7c06）：

| 类型 | mode == 1 | mode == 2 | 其他 |
|---|---|---|---|
| quad | tRef=1e10，mode=0，并行 fn.1，tRef=min | tRef=0，mode=0，并行 fn.2，tRef=ΣT/ΣCnt | tRef=1e10，并行 fn.3，tRef=min |
| solenoid | 同上，fn.4 | fn.5 | fn.6 |
| bend（hv==0.0 / 其他） | fn.7 / fn.8 | fn.9 / fn.10 | fn.11 / fn.12 |
| edge（hv==0.0 / 其他） | 不看 mode：fn.13 / fn.14 | | |
| 其他类型 | 什么都不做 | | |

- "tRef=min"：并行结束后串行扫描 i = 0 … N−1，`row[7] == 1.0` 且 `row[6] < tRef` 时 tRef = row[6]。
- "ΣT/ΣCnt"：fn.2/5/9/10 每个线程按自己负责的下标顺序累加存活粒子的 row[6]（传输后）与计数 1.0，结束时在 `GOMP_atomic` 区里加到共享和上；tRef = ΣT/ΣCnt（无存活粒子时为 NaN）。
  **线程完成顺序不同，求和顺序就不同，结果最后几位不确定**。
- quad mode==1 分支还算了同步行的 β = sqrt(1−1/γ²) 但结果丢弃，无副作用。
- 并行：`omp_set_num_threads(MethodMP.cores[1])`；粒子 i ∈ [0, N) 按 GOMP 静态划分（chunk = N/T，余数 rem = N%T，线程 t<rem 处理 chunk+1 个，从 t·(chunk+1) 开始，否则从 rem + t·chunk 开始）。各粒子相互独立，划分方式不影响结果。
- 线程闭包内容：quad/solenoid `{rows, beam, lat, err, strength(+0x58), Lstep = zTo − zFrom, idx}`；
  bend `{rows, beam, err, s, h, kx, kx², ky, cos(s·kx), sin(s·kx), cos(s·ky), sin(s·ky), &errvec, idx}`；edge `{rows, beam, err, k1, k2, &errvec, idx}`。
- bend 的准备：`par = bend.GetParameter()` = [hv, arcLength, h, kx, ky, unk_c8, dx, dy, dz, θx, θy, θz, fieldErr]；
  **s = arcLength；若 (zTo − zFrom) < arcLength·0.85 则 s = arcLength·0.5**（用这个比例识别半步）；(sx,cx)=sincos(s·kx)，(sy,cy)=sincos(s·ky)。
  误差启用时 errvec = par[6..12]（7 个），否则为空向量。
- edge 的准备：`par = edge.GetParameter()` = [hv, k1 = tanβ/ρ, k2 = −tan(β−ψ)/ρ, dx, dy, dz, θx, θy, θz, fieldErr]（ρ = |ρ|）；误差启用时 errvec = par[3..9]。
- quad/solenoid 的误差参数在线程里每个粒子单独调用 `GetParameter` 取得：quad = [dx,dy,dz,θx,θy,θz,Gerr]，solenoid = [dx,dy,dz,θx,θy,θz,Berr]。

#### 2.8.3 每个粒子的通用步骤（fn.1 … fn.12）

对 i ∈ [0, N)，r = rows[i]，p = particles[i]，若 `r[7] != 1.0` 跳过：

1. **误差启用判据**：`err.flags[2] + err.flags[3] != 0` 且 `err.dyn[idx+1][0] == 1.0`（只看 dyn 表的第 0 列，静态误差也按 dyn 表判断）。启用时做入口变换 T_in（d = 误差向量）：
   ```
   X0 = x − dx;  Y0 = y − dy
   (sz,cz)=sincos(θz):  u = cz·X0 + sz·Y0;   v = Y0·cz − sz·X0;   px1 = cz·px + sz·py;   py1 = cz·py − px·sz
   (sy,cy)=sincos(θy):  x  ← cy·u − (−dz)·sy;                     pz1 = px1·sy + cy·pz;  px ← cy·px1 − pz·sy
   (sx,cx)=sincos(θx):  y  ← v·cx + (sy·u + cy·(−dz))·sx;          py ← cx·py1 + sx·pz1;  pz ← pz1·cx − py1·sx
   ```
   （px,py,pz 指 r[1],r[3],r[5]；sincos 调用顺序 θz、θy、θx；纵向位置分量被丢弃，r[4]、r[6] 不变。）
2. **空间电荷冲量**（只在 mode==1 的 fn.1、fn.4、fn.7、fn.8，且在第 1 步之后）：
   `a = (p.qOverM / c)·r[4]; a = a + a;  r[1] = p.scE[0]·a + r[1];  r[3] = p.scE[1]·a + r[3];  r[5] = a·p.scE[2] + r[5]`。
   r[4] 是前半步的 Δt，所以冲量时间 = 两个半步之和；scE 是实验室系的场，直接加到（可能已旋转的）元件系动量上。
3. γ = sqrt(r1·r1 + 1.0 + r3·r3 + r5·r5)，β = sqrt(1.0 − (1.0/γ)/γ)；
   x' = r1/r5，y' = r3/r5；`vz = sqrt((β·c·β·c) / (x'·x' + y'·y' + 1.0))`。
4. 时间：`r[4] = ℓ/vz`，`r[6] = r[6] + ℓ/vz`，ℓ 为本次推进长度（quad/solenoid 用 Lstep，bend 用 s）。edge 不做这一步。
5. 元件矩阵（2.8.4–2.8.6），结果先写进 `particles[i]` 的 x、gbx、y、gby（bend 还写 z、gbz）作为暂存，再 `r[0]=X; r[1]=X'·r[5]; r[2]=Y; r[3]=r[5]·Y'`。
   **γβz（r[5]）在矩阵传输中不变**，所以传输后 |p| 与 γ 会略有变化。Particle 的这些字段之后由 `zTotcode` 覆盖，暂存值没有其他读者（推测，`HESCFieldPre` 只写 rel[]）。
6. mode==2 的函数在这里累加 r[6]（第 4 步之后）与计数。
7. 误差启用时做出口变换 T_out：
   ```
   Y1 = y + dy
   (sx,cx)=sincos(θx):  w = cx·Y1 − dz·sx
   (sy,cy)=sincos(θy):  q = (x + dx)·cy + (Y1·sx + dz·cx)·sy
   (sz,cz)=sincos(θz):  pz1 = sx·py + cx·pz;  py1 = cx·py − sx·pz;  px1 = cy·px + sy·pz1
                        x ← cz·q − w·sz;  y ← q·sz + w·cz;  pz ← pz1·cy − sy·px（旧 px）;  px ← cz·px1 − sz·py1;  py ← px1·sz + py1·cz
   ```
   **T_out 不是 T_in 的精确逆**：偏移在旋转之前加回（T_out = R_z(−θz)R_y(−θy)R_x(−θx)(r + d)，T_in = R_x R_y R_z (r − d)，纵向分量取 −dz / +dz）。只有转角为 0 时互逆。

#### 2.8.4 四极（fn.1 / fn.2 / fn.3）

```
k² = | G / ((γ·c·β) / p.qOverM) |,   k = sqrt(k²)        （G = quad.strength，L = Lstep）
若 p.qOverM·G ≤ 0（x 散焦）:
    x0 = r0;  x0' = r1/r5 (写回 r1)
    X  = x0·cosh(k·L) + (sinh(L·k)·x0')/k
    X' = x0·k·sinh(L·k) + cosh(k·L)·x0'
    y0' = r3/r5 (写回 r3);  (s,c)=sincos(k·L)
    Y  = c·y0 + (y0'·s)/k;    Y' = −y0·k·s + c·y0'
否则（x 聚焦）:
    x0' = r1/r5 (写回);  (s,c)=sincos(k·L)
    X  = c·x0 + (x0'·s)/k;    X' = −x0·k·s + c·x0'
    y0' = r3/r5 (写回)
    Y  = y0·cosh(k·L) + (sinh(L·k)·y0')/k;   Y' = y0·k·sinh(L·k) + cosh(L·k)·y0'
```
纯一阶、无色散修正（k 用每个粒子自己的 γβ，因此自带能量依赖），无边缘场。G = 0 时 k = 0，0/0 得 NaN（`Quadrupole::SetParameter` 拒绝 G=0，与 AGENTS.md 一致）。
聚焦方向与 t-code 场 Bx=G·y、By=G·x 一致：qG > 0 时 x 聚焦、y 散焦。

#### 2.8.5 螺线管（fn.4 / fn.5 / fn.6）

```
K = (p.qOverM·B) / (β·599584916.0·γ)        （= qB/(2p)，B = solenoid.strength，L = Lstep）
(S,C) = sincos(K·L);  S2 = S·S;  SC = S·C;  C2 = C·C;  x' = r1/r5, y' = r3/r5（都写回）
X  = x·C2 + (x'·SC)/K + y·SC + (y'·S2)/K
X' = (−x)·K·SC + x'·C2 + (−y)·K·S2 + y'·SC
Y  = ((−x')·S2)/K − x·SC + y·C2 + (y'·SC)/K
Y' = (x·K·S2 − x'·SC) + (−y)·K·SC + C2·y'
```
即标准硬边螺线管矩阵（含入口、出口边缘的横向冲量）。B = 0 时 K = 0，0/0 得 NaN，原内核不拒绝 B = 0（见第 5 节）。
**t-code 的 Solenoid::GetField 只有 Bz，没有边缘场，因此 t-code 与 z-code 对螺线管的结果不同**。

#### 2.8.6 弯转（fn.7–fn.12）与边缘（fn.13、fn.14）

**弯转**（`h = sign(α)/ρ`，`kx = sqrt((1−n)h²)`，`ky = sqrt(n·h²)`，都来自 `Bend::SetParameter`）：

```
δ = (r5 − rows[N][5]) / rows[N][5]            （相对同步行 γβz 的纵向动量偏差）
x' = r1/r5, y' = r3/r5（写回）
弯转平面（hv==0 为 x，hv≠0 为 y）设为 (q, q')，另一平面 (o, o')：
    Q  = q·cx + (q'·sx)/kx + (δ·h·(1 − cx))/kx²
    Q' = −q·kx·sx + cx·q' + (δ·h·sx)/kx
    若 ky == 0.0:  O = o'·s + o;   O' = o'
    否则:          O = o·cy + (o'·sy)/ky;   O' = −o·ky·sy + cy·o'
路程项（只写到 particles[i].z 暂存，未用于 r[6]）:
    (−q·h·sx)/kx + (−q'·h·(1−cx))/kx² + ((−h·h·(kx·s·β·β − sx))/kx²/kx + ((1 − h²/kx²)·s)/γ/γ)·δ
```
其中 δ·h 先算成一个量（`δ·h` 再乘其他因子）。弯转是纯几何映射，与粒子电荷、实际磁场无关；n ≥ 1（kx = 0 或 NaN）或 n < 0（ky NaN）都会产生 NaN，原内核不检查。
时间增量用 s/vz（s 为弧长或半弧长）。**弯转内部没有极面边缘聚焦**，极面角由单独的 edge 元件给出。

**边缘**（fn.13 hv==0、fn.14 hv≠0；无时间推进、无空间电荷冲量）：
```
x' = r1/r5;  y' = r3/r5                  （若启用误差，在 T_in 之后用新的 r5 计算）
hv==0:  X' = x·k1 + x';   Y' = y·k2 + y'
hv≠0:  X' = x·k2 + x';   Y' = y·k1 + y'
x、y 不变；r1 = X'·r5，r3 = Y'·r5；然后 T_out
```
k1 = tanβ/ρ（弯转平面，β 为极面角，rad），k2 = −tan(β−ψ)/ρ，ψ 见 structures.md（TRANSPORT 边缘场修正角，`Edge::SetParameter` 计算）。

#### 2.8.7 同步粒子版本 `HardEdgeTrans(int idx, rows&, Beam&, Lattice&)`（0x1a0ad0）

只推进第 N 行，**不做误差变换、不加空间电荷冲量**，用整段长度：

- quad：L = e.length；γ、β 同上；k² = |G/((γ·c·β)/particles[N].qOverM)|；**Δt = L/(β·c)**（用总速度，不用 vz）；r[4] = Δt，r[6] += Δt；矩阵同 2.8.4（sincos、cosh、sinh 的参数都是 L·k）。
- solenoid：L = e.length；K、矩阵同 2.8.5；Δt = L/vz。
- bend：s = arcLength（不做 0.85 判断）；**先把 r[1]、r[3] 置 0**（同步行横向动量被清零），δ 项乘 0.0；Δt = s/vz（vz 用清零前的 x'、y' 算出）；矩阵同 2.8.6。
- edge：薄透镜同 2.8.6（无误差）。

### 2.9 校正铁冲量与两个方向问题的答案

`Steerer::GetField` 恒为零，校正铁的作用写在 `zcodeRun`（D 子系统）里：steerer 紧接着的硬边元件 e 在中点（前半步之后）给所有存活粒子（i < N，**不含同步行**）一次冲量，
参数 `par = steerer.GetParameter()` = [kind, word4, word5, maxValue]，`Δ = e.length · particles[0].qOverM`（**用 e 的长度和第 0 个粒子的荷质比；maxValue 不用**）：

- 磁（kind ≠ 1）：`r[1] −= word5·Δ/c`（By），`r[3] += word4·Δ/c`（Bx），即 Δ(γβx) = −(q/m)·By·L/c，Δ(γβy) = +(q/m)·Bx·L/c。
- 电（kind == 1）：`r[1] += (word4·Δ/c²)/β`，`r[3] += (word5·Δ/c²)/β`（β 为该粒子的 sqrt(1−1/γ²)），即 Δ(γβx) = (q/m)·Ex·L/(βc²)。

结论：**By > 0 使正粒子向 −x 偏，Bx > 0 向 +y，Ex > 0 向 +x**，与 AGENTS.md 第 6 节"已用内核核实"的约定一致。
（steerer 后面接的是 t-code 元件时，由 `Run` 记录元件中点、在 `tcodeRun` 中施加，不属本文；见第 5 节。）

**弯转方向**：z-code 矩阵里 R16 = h(1−cos(kx·s))/kx²、R26 = h·sin(kx·s)/kx，h = sign(α)/ρ。α > 0（ρ > 0）时动量偏大的粒子出口 x > 0，
即 **x 指向弯转外侧、参考轨道向 −x 弯（hv=1 时向 −y）**，与线性预览 `bend_matrix` 相同，与 3D 视图的画法相反。
该方向与粒子电荷符号、磁场无关（纯几何）。实验室系里束流坐标系的旋转由 `TcodeTrans`（`Beam.rotAngle/rotMode`）决定，不在本文范围。

## 3. 对齐清单

建议的通用做法：用原内核与新内核跑同一组小 lattice（几十个宏粒子或单粒子，固定 `randomseed`，关闭多线程以外的随机源），比较 `DataSet.txt`、`outData_*.dst` 及专门加的调试输出。
"逐位"指 IEEE 双精度逐位相同；涉及 OpenMP 归约或 libm 差异时放宽到相对 1e-14。

| ID | 行为 | 涉及关键字/文件 | 建议的比对测试 |
|---|---|---|---|
| C-01 | 主束元件定位：z == zEnd 归下一个元件；z<0、越过 spOut 出口或已丢失的粒子外场清零 | lattice 元件边界 | 构造粒子恰好落在元件边界的单粒子 t-code 运行，比较每步 extE/extB（调试钩子）；逐位 |
| C-02 | 单粒子定位版只清 Ex、Bx；z == zEnd 归当前元件 | secondarybeam | 二次粒子穿过 lattice 末端，检查越界后 Ey/Ez/By/Bz 保留旧值；逐位 |
| C-03 | lost = 2 的设置条件 | lattice 末端 | 束流全部越过末端，比较丢失类型统计；完全一致 |
| C-04 | Drift 孔径判丢失（R ≤ r，boundary 0，spOut 条件） | drift R、input `boundary` | 单粒子 x = R−ε / R / R+ε 三种初值；比较存活数；完全一致 |
| C-05 | Drift 同步粒子越孔径时退出并输出英文提示 | drift R | 同步粒子初始偏离使其超出孔径；比较 stdout 文本与退出码 |
| C-06 | Quadrupole t-code 场 Bx = G·y，By = G·x | quad G | 若 quad 出现在 t-code（不会，见 C-07）则比较场；否则用单元测试直接调用 |
| C-07 | 硬边元件只走 z-code；bend/edge/steerer/solenoid t-code 场为零或只有 Bz | lattice | 单元测试调用 GetField；运行层面比较含 quad、solenoid、bend 的 lattice 的 DataSet，相对 1e-12 |
| C-08 | 场图三线性插值、节点顺序（x 最快、z 最慢）、严格边界 | field 行、场图文件 | 用示例 hwr010、sol 场图，单粒子在网格节点、格内、边界上，比较 extE/extB；逐位 |
| C-09 | RF 场：×1e6×Ke、×Kb，E·cos φ，B·cos(φ−π/2)，φ 由 cavPhase/cavTime | field type 1、V3 | 单粒子穿过 hwr010，比较 DataSet 能量列；相对 1e-12 |
| C-10 | 静电（type 2）、静磁（type 3）场的分量位置与乘法顺序 | `.esx/.bsx` 文件 | sol（type 3）单粒子离轴 1 mm，比较轨迹；逐位 |
| C-11 | 场图元件孔径丢失后本步外场保持旧值 | field R | 单粒子越过 field 孔径的那一步，检查推进结果；逐位 |
| C-12 | 叠加组：局部坐标变换（P[1],P[2] 偏移，P[3]→P[5]→P[4] 旋转）、子类型编码 1 磁/2 RF/3 电、旋转回实验室系 | superpose 块 | 两个场图叠加并给偏移、转角，单粒子比较场与轨迹；逐位 |
| C-13 | 叠加组孔径：不看 boundary；按元件 +0x6c 选局部坐标或相对坐标；丢失时保留部分和 | superpose、bend 区 | boundary 1 时仍应丢失；比较存活数 |
| C-14 | 叠加组 RF 子场共用组的频率与 cavIndex | superpose 两个不同频率 RF | 比较能量增益；相对 1e-12 |
| C-15 | 同步粒子 RF 只算 Ez 并写 EzAmp；静场正确清另一场 | V3=0 腔 | 比较 synData.txt 的入口相位与能量；逐位 |
| C-16 | 同步粒子在场图元件超孔径时输出 GBK 文本并 exit(1) | field R 很小 | 比较 stdout 字节与退出码 |
| C-17 | 文本 3D 场图头部与 norm（stoi 截断）解析 | 场图第 4 行 | norm 写 "2.5"，比较场幅（应按 2 除）；完全一致 |
| C-18 | 1D 场图回退：提示文字、mark 检查、横向均匀 Ez、无横向场 | 只有 `.edz` | 1D 场图单粒子离轴运行；比较 stdout 与轨迹（横向不受力）；逐位 |
| C-19 | 二进制场图：名以 b 结尾、int32/float64 头 + float32 数据 | `xxxb.edx` 等 | 把示例场图转成二进制，比较与文本版结果（数据经 float32 舍入）；逐位 |
| C-20 | 打不开场图时的异常文本 | 缺文件 | 比较错误输出 |
| C-21 | 场图按名字去重，序号返回 | 两个 field 用同一场图 | 比较运行结果与内存（序号）；完全一致 |
| C-22 | RFQ getEleNum 边界（z == cellZEnd 属当前 cell）、getRadius 越界返回 1e99 | RFQ | 单元测试 |
| C-23 | RFQ 8 项势场公式（含省略的 Bessel 因子、E = +∇U） | RFQ 文件 | 单粒子在 RFQ 内若干点，比较 extE；逐位 |
| C-24 | RFQ 轴上（r<1e-16）Ez 与轴外反号 | RFQ | 单粒子 x=y=0 与 x=1e-12 两种；比较 Ez 符号 |
| C-25 | RFQ getEleNum = −1 时原内核越界读 | RFQ 末端 | 记录原内核行为（可能不可重复），新内核返回零场；标为已知差异 |
| C-26 | Bessel 级数 20 项 | — | 单元测试与原函数逐位比较（同一 libm） |
| C-27 | RFQ 同步粒子逐 cell 相位：dt = 1e-4/f、边界线性插值 + 1e-15、atan 相位、−90 特例、能量下降中止 | RFQ、`RFQsynPhaseEG.txt` | 比较 RFQsynPhaseEG.txt 全文（fixed 10 位）与 stdout；逐字节 |
| C-28 | 硬边调度：mode 0/1/2 的分派、tRef 取最小 t 或平均 t，edge 不改 mode/tRef | z-code | 调试输出 tRef；mode 2 相对 1e-14（归约顺序） |
| C-29 | 误差坐标变换 T_in/T_out（含非互逆） | err_quad 等误差文件 | 只给一个四极 dx、dy、θz 误差的单粒子运行，比较出口坐标；逐位 |
| C-30 | 误差启用判据只看 dyn[idx+1][0] 与 flags[2]+flags[3] | StaticErrorData/DynamicErrorData | 静态误差时检查是否施加；完全一致 |
| C-31 | 半步空间电荷冲量：Δt = 2·r[4]，在误差变换之后 | spacecharge 1 | 含 quad 的 lattice 开空间电荷，比较 DataSet rms；相对 1e-12（单线程） |
| C-32 | 四极矩阵（聚焦/散焦分支按 qG 符号，cosh/sinh 参数顺序） | quad G 正负、负电荷粒子 | 单粒子 +x/+y 初值，正负 G、正负电荷；逐位 |
| C-33 | 时间推进 Δt = ℓ/vz，vz = sqrt(β²c²/(x'²+y'²+1)) | — | 比较出口 dst 相位列；逐位 |
| C-34 | 螺线管矩阵与 K = qB/(2p) | solenoid B | 单粒子 x 初值，比较 x、y 耦合；逐位 |
| C-35 | 弯转：0.85 半步识别、δ 相对同步行 γβz、ky == 0 走漂移、色散符号 | bend α、ρ、n、hv | 单粒子 δ≠0，比较出口 x；n = 0 与 0<n<1；hv 0/1；逐位 |
| C-36 | 边缘薄透镜 k1、k2 及 hv 交换平面 | edge β、K1、K2、G、hv | 单粒子 x、y 初值；逐位 |
| C-37 | 同步行推进：quad 用 L/(βc)，bend 清零横向动量，无误差与冲量 | z-code | 比较 synData 或调试输出同步粒子时间；逐位 |
| C-38 | 校正铁冲量方向、Δ 用后接元件长度与第 0 粒子荷质比、不作用于同步行 | steerer + quad | 磁 By、Bx 与电 Ex 各一次，单粒子比较末端偏移（AGENTS 已有核实值）；逐位 |
| C-39 | 弯转色散方向：α>0 时高动量粒子出口 x>0 | bend | 单粒子 δ>0 比较出口 x 符号 |
| C-40 | 硬边元件参数非法（G=0、B=0、n≥1、n<0、α=0）产生 NaN 而不报错 | quad/solenoid/bend | 比较原内核输出是否为 NaN / 全部丢失；行为一致即可 |

## 4. 函数覆盖表

| 函数 | 作用一句话 | 正文 |
|---|---|---|
| `Bend::GetField(double, Particle&, FieldMatrix const&)` 0x125c80 | t-code 弯转外场恒为 0 | 2.3 |
| `Bend::GetField(double, SynParticle&, FieldMatrix const&)` 0x125df0 | 同步粒子 E、B 赋 {0,0,0} | 2.3 |
| `Element::GetField(double, SynParticle&, …)` 0x126950 | 基类空实现 | 2.3 |
| `Element::GetField(double, Particle&, …)` 0x126970 | 基类空实现 | 2.3 |
| `Element::getRadius(double)` 0x1269b0 | 返回 radius | 2.3 |
| `Drift::GetField(double, Particle&, …)` 0x127080 | 孔径判丢失 + 零场 | 2.3 |
| `Drift::GetField(double, SynParticle&, …)` 0x127340 | 同步粒子超孔径退出，否则零场 | 2.3 |
| `Edge::GetField(double, Particle&, …)` 0x127600 | 零场 | 2.3 |
| `Edge::GetField(double, SynParticle&, …)` 0x127770 | 零场 | 2.3 |
| `Field::GetField(double, Particle&, …)` 0x12cb30 | 场图元件插值（RF/静电/静磁/叠加组）与孔径 | 2.4 |
| `Field::GetField(double, SynParticle&, …)` 0x1305d0 | 同步粒子版场图插值（RF 只取 Ez） | 2.5 |
| `Bessi0` 0x14a870 | I0 级数（RFQ 未调用） | 2.7.2 |
| `Bessi1` 0x14a960 | I1 级数（RFQ 未调用） | 2.7.2 |
| `Bessin` 0x14aa70 | In 级数，RFQ 用 n=0、1 | 2.7.2 |
| `Lattice::GetExternalField(double, Beam&)` 0x188180 | 主束逐粒子定位元件并取场、lost=2 | 2.2.1 |
| `Lattice::GetExternalField(double, Particle&)` 0x1889e0 | 单粒子定位并取场 | 2.2.2 |
| `Lattice::SetMatrix(string, int, FieldMatrix&)` 0x188bb0 | 读场图（文本 3D / 1D / 二进制）入库 | 2.6 |
| `MethodMP::HardEdgeTrans … _omp_fn.1` 0x1940f0 | quad，mode 1（冲量 + 传输） | 2.8.3, 2.8.4 |
| `… _omp_fn.2` 0x194e10 | quad，mode 2（传输 + 平均时间归约） | 2.8.3, 2.8.4 |
| `… _omp_fn.3` 0x195b40 | quad，mode 0 | 2.8.4 |
| `… _omp_fn.4` 0x1967f0 | solenoid，mode 1 | 2.8.5 |
| `… _omp_fn.5` 0x1972d0 | solenoid，mode 2 | 2.8.5 |
| `… _omp_fn.6` 0x197e00 | solenoid，mode 0 | 2.8.5 |
| `… _omp_fn.7` 0x1988a0 | bend 水平，mode 1 | 2.8.6 |
| `… _omp_fn.8` 0x1993a0 | bend 竖直，mode 1 | 2.8.6 |
| `… _omp_fn.9` 0x199ee0 | bend 水平，mode 2 | 2.8.6 |
| `… _omp_fn.10` 0x19aa20 | bend 竖直，mode 2 | 2.8.6 |
| `… _omp_fn.11` 0x19b580 | bend 水平，mode 0 | 2.8.6 |
| `… _omp_fn.12` 0x19c030 | bend 竖直，mode 0 | 2.8.6 |
| `… _omp_fn.13` 0x19cad0 | edge 水平薄透镜 | 2.8.6 |
| `… _omp_fn.14` 0x19d160 | edge 竖直薄透镜 | 2.8.6 |
| `MethodMP::HardEdgeTrans(int, rows&, Beam&, Lattice&)` 0x1a0ad0 | 同步行整段推进 | 2.8.7 |
| `MethodMP::HardEdgeTrans(int, double, double, rows&, Beam&, Lattice&, int&, double&, ErrorSample&)` 0x1aa5c0 | 调度：按类型/mode/hv 选并行子函数，算 tRef | 2.8.2 |
| `Quadrupole::GetField(double, Particle&, …)` 0x1d02a0 | Bx=G·y、By=G·x | 2.3 |
| `Quadrupole::GetField(double, SynParticle&, …)` 0x1d05c0 | 零场 | 2.3 |
| `RFQ::getEleNum(double)` 0x1d0ad0 | z 所在 cell，超出返回 −1 并警告 | 2.7.1 |
| `RFQ::getRadius(double)` 0x1d0bb0 | cell 孔径，越界 1e99 | 2.7.1 |
| `RFQ::GetField(double, Particle&, …)` 0x1d0be0 | 8 项势展开场 | 2.7.3 |
| `RFQ::GetField(double, SynParticle&, …)` 0x1d29a0 | 同步粒子逐 cell 相位积分，写 RFQsynPhaseEG.txt | 2.7.4 |
| `Solenoid::GetField(double, Particle&, …)` 0x1d3540 | 只有 Bz | 2.3 |
| `Solenoid::GetField(double, SynParticle&, …)` 0x1d3890 | 零场 | 2.3 |
| `Steerer::GetField(double, Particle&, …)` 0x1d3d50 | 零场 | 2.3, 2.9 |
| `Steerer::GetField(double, SynParticle&, …)` 0x1d3db0 | 零场 | 2.3 |

共 43 个函数（分配说明写的"17 个 HardEdgeTrans"实际是 16 个：1 个调度 + 14 个 OpenMP 子函数 + 1 个同步行版本）。

## 5. 未解决的问题

1. **RFQ 轴上/轴外 Ez 反号**（2.7.3 第 4 步）：伪代码与反汇编都确认轴外 Ez 为 +Vh·σ·(…)、轴上为 −Vh·σ·(…)。
   实验：RFQ 中放 r=0 与 r=1e-12 的两个单粒子，比较一步后的 γβz 变化符号。若确认，新内核需选择"兼容"还是"修正"，并在比对测试中区分。
2. **RFQ 越界**：getEleNum 返回 −1 时 `RFQ::GetField(Particle)` 继续用 i=−1 读数据；同步粒子版第一个 cell 读 `energy[-1]`。实验：粒子刚好在 RFQ 末端之后仍被分到 RFQ 时观察原内核输出是否随机。
3. **lost = 2 与 spOut 越界**：`spOutCur == spOut.size()` 时读 `spOut[spOutCur]` 越界。需要结合 `tcodeRun` 对 spOutCur 的递增规则（D 子系统）确认是否真的会到这个状态；实验：无 superposeout 的 lattice，统计末端 lost 类型。
4. **校正铁后接 t-code 元件**：`Run` 为这种情况记录"元件中点 z + steerer 下标"，冲量在 `tcodeRun` 里施加，方向与长度规则需由 D 子系统确认是否与 2.9 一致。
   AGENTS.md 的核实算例里 steerer 后接的元件类型未注明；实验：steerer 后分别接 drift 和 quad，比较末端偏移。
5. **mode 2 平均时间的归约顺序**造成的最后几位差异：需确认对 DataSet 输出的影响量级（多线程下原内核自身可重复性）。实验：同输入、cores[1] 取 1 与 8 各跑两次比较。
6. **1D 场图**：确认原内核确实不做横向展开（横向场为零、Ez 横向均匀）。实验：只提供 `.edz` 的腔，离轴粒子不受横向 RF 力。
7. **solenoid B = 0、bend n ≥ 1 或 n < 0** 的 NaN 是否被上游（SetParameter/InitLattice）拦截：本文只看到 quad 拒绝 G=0。实验：各写一个，看原内核报错还是输出 NaN。
8. **叠加组元件 +0x6c 分支**（bend 区内的叠加组用相对同步粒子坐标判孔径）只在 `tcodeRun` 设置 rotMode 时触发，未构造过算例。
9. **Field::GetField(Particle) 的 t 与 RF 相位**：cavPhase/cavTime 的写入与 V3=0/2 的关系属 B 子系统；本文只规定使用方式。
10. **AVAS.dll**：只核对了字符串集合（`.edx`… `.esz`、1D 提示、`RFQsynPhaseEG.txt`、indexElem 警告都在），函数体未逐个比对，DLL 是否有额外场类型未知。

## 6. 对结构表的更正

| 位置 | structures.md 现状 | 更正 / 补充 | 证据 |
|---|---|---|---|
| Field +0x08c fieldType | 1 RF、4 叠加确定；2/3 推测 | **2 = 静电（.esx/.esy/.esz，×Ke×1e6 → E），3 = 静磁（.bsx/.bsy/.bsz，×Kb → B）**，确定 | `Field::GetField` 两个重载；`Lattice::SetMatrix` 的扩展名分支 |
| Field +0x138 subType | "子场类型" | **编码与 fieldType 不同：1 = 静磁，2 = 射频，3 = 静电**（SetParameter 把子块 word4=1 存为 2、word4=3 存为 1） | `Field::SetParameter` 0x1352cf 起；`Field::GetField` 0x12ce5a 处 cmp |
| Field +0x198 subPlace | {3 个位移, 3 个角度}，顺序推测 | GetField 只用 [1]、[2]（x、y 偏移）与 [3]、[4]、[5]（rad）：先绕 z 转 [3]，再绕 y 转 [5]，最后绕 x 转 [4]；[0] 不用（纵向位置已并入 subZStart） | `Field::GetField` 两个重载 |
| Element +0x06c | "运行中被清 0 的标志" | 束流坐标旋转模式：`tcodeRun` 把 `Beam.rotMode`（0/1/2）写入当前元件；叠加组孔径判断据此选择局部坐标或 rel[] | `tcodeRun` 伪代码第 2297 行（清 0）、2616 行（写 rotMode）；`Field::GetField(Particle)` |
| Element +0x068 | inSuperposeOut（推测） | 同步粒子版叠加组孔径只在它为 0 时判断；`InitLattice` 对 superposeout 块写 1 或 2 | `Field::GetField(Syn)`、`InitLattice` |
| FieldMatrix +0x000 maps | 每节点 {Ex,Ey,Ez,Bx,By,Bz} | 只有 RF 图每节点 6 个分量；静电/静磁图每节点 **3 个**分量，存在 [0..2]（静磁的 Bx,By,Bz 也在 [0..2]） | `SetMatrix` 的 `vector<double>(3)` 与 `(6)` |
| SynParticle +0xa8 EzAmp | 推测 | 确定：RF 场图写 Σ 插值 Ez·1e6·Ke（未乘 cos）；叠加组写最后一个 RF 子场的局部 Ez 幅值；RFQ 写轴上 Ez（含负号） | `Field::GetField(Syn)`、`RFQ::GetSynField` |
| Bend +0x0c8 unk_c8 | SetParameter 不写 | `Bend::GetParameter` 在传入非空向量时把 v[0] 存进 +0xc8 并把 v[0] 改为 hv（getter 带副作用）；HardEdgeTrans 传空向量 | `Bend::GetParameter` 0x1264a0 |
| Element +0x058 strength（Bend/Edge） | 赋值处未找到 | 硬边传输不使用 Bend/Edge 的 +0x58；Bend 输出参数里也没有它 | `HardEdgeTrans` 调度函数 |
| RFQ +0x088 | 每 cell 系数（推测） | 每 cell 8 个系数 A0…A7，含义见 2.7.3；+0x100 为 cell 电压 V（取绝对值使用）；+0x170 为轴上判据 eps | `RFQ::GetField(Particle)` |
| Lattice +0x210 spOut | 推测 | `InitLattice` 末尾追加哨兵行 {0,0,0,0,0,0, latticeEnd}，`spOutCount` 为真实块数（不含哨兵） | `InitLattice` 0x185a00 附近 |
| MethodMP +0x208 cores | vector\<int\> | `HardEdgeTrans` 用 cores[1] 作 OpenMP 线程数，`zTotcode` 用 cores[3] | 调度函数、`zTotcode` |
| Particle x/gbx/y/gby/z/gbz | — | z-code 期间被 `HardEdgeTrans` 当暂存区改写（bend 还改 z、gbz），之后由 `zTotcode` 覆盖 | 各 omp 子函数 |
