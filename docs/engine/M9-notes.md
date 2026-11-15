# M9：RFQ 元件（新内核默认模式）与原始仓库的发现（2026-11-16）

## 1. 为什么以前没有 RFQ

- 原内核的 RFQ **从来不能用**，所有版本都一样：
  - AVAS.dll 的 `InitLattice` 遇到 `rfq` 行只调用 `ReadRFQ` 和基类的空 `SetParameter`（10-input-lattice 2.6.4.8），
    RFQ 对象的起点、长度、cell 表都不设置，随后访问空数组崩溃；
  - 真正读 cell 表的 `RFQ::SetParameter(double, vector<string>)` 在 libAVAS.so 中是没有调用者的死代码，在 DLL 中已经不存在。
- 原作者的仓库 IMP-AVAS/AVAS（2026-11 查看）中，2024-11 到 2026-05 各版本 DLL 的 RFQ 字符串完全相同。
  他们自己的脚本把 RFQ 当作 6 分量射频场图处理（`otherdemand/change_field/read_field_txt.py` 的 `RFQ.edx … .bdz`），
  这条路新内核一直支持。
- 所以 RFQ 元件**没有参考结果**：
  - 新内核按逆向得到的规格加修正实现，只在默认模式下可用；
  - 兼容模式遇到 `rfq` 报错，因为原内核也算不了。

## 2. 输入

- **lattice 行**：`rfq L R V3 type f phase file`。
  - `word5` 是频率（Hz），`word7` 是结构文件名（输入文件夹中）。`V3`、`type`、`phase` 不使用，相位由扫描确定。
  - 和 field 一样补到 14 个词，再追加腔号。腔号与 field 共用一个计数。
  - 依据：反汇编 `RFQ::SetParameter` 0xd19ad 处 `leaq 0x38(%rdx)`，即 word[7]，直接接在 inputFilePath 后，不加扩展名；
    `word[5]` 存频率，`word[14]` 存腔号。
- **结构文件**：每个 cell 一行，至少 14 列（libAVAS.so `RFQ::SetParameter`）。

  | 列 | 含义 | 单位换算 |
  |---|---|---|
  | 0 | 序号 | 不使用 |
  | 1 | cell 长度 | cm → m；cell 起点、终点从元件起点累加 |
  | 2 | 极间电压 V | × 1000（kV → V） |
  | 3 | 孔径 a | cm → m |
  | 4 | 未用量 | 原样保存 |
  | 5 | — | 不使用 |
  | 6–13 | 8 项势系数 A0…A7 | 原样 |

- **rfqinput.txt 的生成**：`ReadRFQ` 把 `rfqinputAnn.txt`、`rfqinputV.txt` 转成 `rfqinput.txt`（10-input-lattice 2.8.1）。
  - 新内核只在两个源文件都存在时转换。原内核总是重写，会把手写的 rfqinput.txt 清空。
  - 新内核跳过空行，原内核遇到空行抛 out_of_range。
- **检查**：
  - 少于 14 列、长度或孔径非正、没有 cell，都报错；
  - cell 总长超过元件长度 L 时报错；
  - 不足 L 时，剩余部分无场，用元件孔径 R。

## 3. 场（kernels.h `rfq_field_t`）

- **cell 的确定**：按 `RFQ::getEleNum` 取第一个满足 z ≤ cell 终点的 cell。超出最后一个 cell 时无场。
  原内核在 i = −1 时越界读。
- **势函数**，k = π / cell 长度，ζ 从 cell 起点算，σ = (−1)ⁱ：

  U = V/2 · [A0 (r/a)² cos2θ + A2 (r/a)⁶ cos6θ + σ (A1 I0(kr) sin kζ + A6 I0(3kr) sin 3kζ + A3 I4(kr) cos4θ sin kζ + A7 I4(3kr) cos4θ sin 3kζ + A4 I2(2kr) cos2θ sin 2kζ + A5 I6(2kr) cos6θ sin 2kζ)]

  这就是标准 8 项势，系数位置与 AVAS.dll 的 `RFQ::GetField` 相同。
- **场**：E = −∇U 乘以射频因子 cos(2πf(t − t_cav) + φ_cav)，B = 0。修正了原内核的两处缺陷（30-fields-matrices 2.7.3）：
  - 轴外用 +∇U，轴上用 −∂U/∂z，两者反号。新内核一律用 −∇U，与同步粒子的轴上场一致。
  - A3…A7 项省略了 Bessel 因子，例如 I4(kr) 按 1 处理，使近轴本应消失的项变成常数。新内核用完整的 Iₙ。
- **Bessel 函数**：20 项级数，同 `Bessin`。导数用 Iₙ′ = (Iₙ₋₁ + Iₙ₊₁)/2。
- **孔径**：cell 的 a（`RFQ::getRadius`），越界条件与其他元件相同。
- **核对**：单元测试 `test_rfq_field` 在轴外三点用数值梯度核对，三个分量都在 1e-6 内。

## 4. 同步粒子（synchronous.cpp）

- **逐 cell 推进**：同 `RFQ::GetField(Syn)`（30-fields-matrices 2.7.4）。
  - 步长 0.05/f/500；用轴上场。
  - 过 cell 终点时线性插值（+1e-15），用 atan 求每个 cell 的 φs，写入 RFQsynPhaseEG.txt。
- **扫描阶段**：
  - 原内核：φ0 = ((±π/2 − 2π f_束流 t_入) / π)·180，射频相位却从 t_入 起算，所以入口相位随到达时间变化（缺陷）。
  - 新内核：入口射频相位直接取 ±90°。第一个 cell 的同步相位因此是 −90°，与 RFQ 设计惯例（从 −90° 聚束开始）一致。
  - 原内核在相邻 cell 能量下降时判失败，接近 −90° 的聚束段会因舍入误判。新内核用两个相位各走完一遍，取出口能量高的；
    两个都没有加速（出口能量低于入口、反向或停滞）时报 “RFQ scanning phase abnormality”。
- **主阶段**：
  - 原内核把 rfq 当普通元件：不记录渡越时间，不登记入口时刻，下游腔的时刻下标错位（20-synchronous-phase 第 5 节问题 4）。
  - 新内核登记入口时刻，按扫描得到的相位逐 cell 推进，渡越时间计入 t，synData 记最后一个 cell 的 φs。
- **计算设备**：只在 CPU 上计算。auto 选 CPU，原因代码 `rfq_cpu_only`；指定 GPU 或模拟设备时报错。
  界面运行前的检查（`engine.check`）与内核一致。
- **不支持误差**：同原内核，RFQ 用基类空实现。
- **线性预览**：把 rfq 当漂移段，并给出提示。

## 5. 验证（没有参考，只能做自洽检查）

`engine/tests/physics/rfq_check.py` 用一台合成质子 RFQ：
- 162.5 MHz，35 keV 起，80 个 cell，长度为设计能量下的 βλ/2；
- V = 70 kV，a = 6 mm，A0 = 0.95，A1 = 0.1；
- φs 在前 60 % 的 cell 内从 −90° 升到 −40°。

| 检查 | 结果 |
|---|---|
| 同步粒子出口能量 | 0.27092 MeV，设计估算 (π/4)·A1·V·cosφs 逐 cell 累加为 0.27568 MeV，差 1.7 % |
| 最后一个 cell 的 φs | −42.3°（设计 −40°） |
| 2000 个粒子 | 1429 个到达终点，731 个被俘获并加速（出口能量高于同步能量的 90 %） |
| 被俘获粒子平均能量 | 0.27236 MeV，与同步粒子差 0.5 % |

- 其余粒子没有被俘获。这台合成 RFQ 没有缓变的聚束段，相位变化过快。
- 孔径 3 mm 时聚焦强度 B ≈ 27，远超通常的 3–7，粒子很快丢失，这是设计问题。
- 空间电荷开启（5 mA）也能正常完成。

**已知局限**：
- 系数约定是推测：A0…A7 的顺序取自 `ReadRFQ` 的列映射与 `GetField` 的位置；
- 没有原内核结果可比；
- GPU 不支持；
- 有真实 RFQ 设计（及其他程序的结果，如 TraceWin、Toutatis）后应再核对。

## 6. IMP-AVAS/AVAS 仓库的其他发现

- **原内核的新版本**：仓库 `dllfile/` 保存了 2024-11 至 2026-05 的各版 AVAS.dll／libAVAS.so，最新是 2026-05-27 与根目录版本（1.30 MB）。
  本项目的参考是 2026-04-02 版 AVAS.dll（0.87 MB）和 2025-12-31 版 libAVAS.so。新版本的变化：
  - 改为静态链接 C 运行时，这是体积增加的主要原因；
  - 新增导出函数 `GenBunch`（只生成束团）；
  - 新增 superpose 边界检查的报错（“There are types of superpose areas that cannot be used for superpose”、
    “Undefined type appears in the superposition boundary”）、“Insufficient permissions to write dst file”，以及输出 `ConvertPlane.txt`；
  - RFQ 部分不变。

  物理算法是否改变，字符串看不出来。要确认，可以用新版 DLL 在 XPS 上跑一遍对齐算例（尚未做）。
- **avasx**：原作者自己的多 GPU 内核，CUDA + MPI + NCCL、cuFFT，`sim_gpu/avasx_*` 和 `libavasx.so`。
  元件与新内核覆盖的范围相同，不含 RFQ。
- **LongAccelerator2/3.dll**：配合 `staticfile/ifr.txt` 等表的另一个程序，与 RFQ 无关。
