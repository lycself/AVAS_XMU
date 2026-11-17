# 给 IMP-AVAS 的问题反馈：AVAS 内核 2026-07-22 版

整理日期：2026-11-17。对象是 IMP-AVAS/AVAS 仓库 `dllfile/` 根目录的 AVAS.dll（链接时间 2026-07-22，SHA-1 `93fdc20d8a24…`）和同目录的 libAVAS.so（同一版本的 Linux 构建）。对照版本是 `dllfile/2026-04-02/AVAS.dll`。

依据分两种，每条都注明：

- **实测**：在 Windows 11 上用该 DLL 实际运行，以示例 `hwr010` 的 beam.txt、input.txt 和场图为基础，只改文中列出的几行。
- **代码阅读**：反编译 libAVAS.so 后阅读，有的另外核对了汇编。没有专门构造算例运行。

按影响排序：先列会崩溃或给出错误结果的，再列让旧项目静默改变的，最后是输出和一致性问题。

---

## 一、崩溃或结果错误

### 1. boundary.txt 的 superpose 块在常见写法下使程序崩溃（实测）

现象：boundary.txt 有 superpose 块时，程序在 “Initialising boundary...” 之后抛出 C++ 异常（Windows 异常码 0xe06d7363），进程直接结束。

只有同时满足下面两条时才能运行：

- 块以 `superposeout` 结束（写 `superposeend` 会崩溃）；
- 每个子形状行都写出 `RLP` 和 6 个放置值。

复现（lattice 为三段 `drift 0.1 0.02 0`，input.txt 写 `boundary 1`）：

```
start
1 Copper 0.1 0.02
superpose 0 0 0 0 0 0
1 Copper 0.15 0.0025
superposeend
1 Copper 0.15 0.02
end
```

改成下面的写法就能运行：

```
superpose 0
1 Copper 0.15 0.0025 RLP 0 0 0 0 0 0
superpose 0.05
3 Copper 0.05 0.004 0.0015 RLP 0 0 0 0 0 0
superposeout 0 0 0 0 0 0
```

推测原因：合成的复合段行中，`RLP` 后面只有 superposeout 时才填 6 个放置值，superposeend 时是 `invalid`。子形状的参数一直读到 `RLP` 为止，子形状行没写 `RLP` 时会读到填充的 `invalid`。两种情况都会让 `stod` 抛异常。

建议：superposeend 时填 0；子形状行没有 `RLP` 时，读到行尾就停止。

### 2. superpose 块首的射频腔没有场，不加速也不报错（实测）

现象：射频腔放在 superpose 块的第一个、V3 = 0 时，synData.txt 中 φs 和 φRF 都是 NaN，出口能量等于入口能量，没有任何报错。2026-04-02 版在这种输入上报 “Scan phase error”；04-22、04-28、05-27 版不报错地运行，05-26 版又报错。

复现（读入粒子文件或生成分布均可）：

```
start
drift 0.05 0.02 0
superpose 0 0 0 0 0 0
field 0.21 0.02 0 1 162.5e6 -33 1.36 -1.36 efield
superpose 0 0 0 0 0 0
field 0.35 0.02 0 3 0 0 1 0.3 sol
superposeend
drift 0.05 0.02 0
end
```

原因：腔在块首（z0 = 0）时，合成的 RF 片头多写了一个 `invalid`（21 个词）。`Lattice::SetElement` 和 `Field::SetParameter` 都按每 20 个词一个子块找关键字，于是 RF 片的所有子块都错位，一个子场也没有加载。

预期：腔正常加速。修正后 φs 解出 −33°，同步粒子能量 1.527 → 1.866 MeV（腔单独放置时也是这个结果）。

腔在块尾或块中间（不从块首开始）时正常。

### 3. `lpboundary V1 1` 的周期盒短了 γ 倍（代码阅读）

空间电荷求解在束流静止系中进行，纵向坐标乘了 γ，所以周期盒应当是 γβλ。07-22 版把 `betaGammaLambda` 改成了 βλ（2026-04-02 版是 βγλ），但 `PIC::Weight` 仍按拉伸后的坐标建网格，网格因此短了 γ 倍：

- 多线程路径按 βλ 重新折叠 γ·z，外侧的 (γ − 1) 部分发生混叠，镜像束团离得太近；
- 串行路径按 γβλ 折叠，在 ±βλ/2 之外的粒子落在网格外，既不贡献电荷也不受力。

两条路径的结果也不一致。误差随 γ − 1 增大：1.5 MeV 质子约 1.6e-3，看不出来；100 MeV 约 10 %。粒子按 βλ 折叠本身是对的，问题只在空间电荷网格的长度。

### 4. 新的“一步越过整段短管”检查有两处错误（代码阅读，第 2 处已核对汇编）

07-22 版新增了一项检查：粒子一步越过整段短管（例如薄准直器）时，用入口面上的位置判断是否碰撞。实现有两处错误：

1. 判据用的是全局 x、y（`P[0]`、`P[2]`），不是经过 RLP 平移、旋转后的局部坐标。管子有 dx、dy 或角度时判断错误。
2. 椭圆管（形状 4）写成了 `P[0]²/a² + P[1]²/b² < 1`，其中 `P[1]` 是 γβx，应为 y（`P[2]`）。汇编 0x4ff89 处确认取的是第 2 个分量。

另外，形状 1 的碰撞时间按插值点计算（`t − (1 − f)·dt`），形状 3、4 却直接用 `t`，与插值的坐标不对应。

### 5. 参数不足的射频 field 行使程序越界崩溃（实测）

新增的 “|Ke| < 1e-6 当漂移段” 判断，在把 field 行补齐到 14 个词之前，就用不检查边界的 `operator[]` 读了 `word[4]` 和 `word[7]`。不足 8 个词的 field 行会越界读取，Windows 上访问冲突，进程结束。同样的输入在 2026-04-02 版上不崩溃。

复现：`field 0.21 0.02 0 1 162.5e6`（6 个词）。

建议：先补齐或先检查词数，再读这两个参数。

### 6. `spacechargecomp` 找不到对应行时用 −1 作下标（代码阅读）

事件处理里用 `findRow(…, z_ev, 1.0, 1e-9)` 找 spacechargecomp 行，找不到时返回 −1，随后直接用作下标（未定义行为）。另外，spacechargecomp 放在 z-code 元件（quad、solenoid、bend、edge）处时不产生事件，这一行永远不生效。

### 7. libAVAS.so 遇到 CRLF 换行的 boundary.txt 必然段错误（旧版 libAVAS.so 实测，07-22 版这部分代码未变）

boundary.txt 的读取没有经过 NormalizeLineEndings，`start\r` 不等于 `start`，于是全部行被跳过，之后对空 vector 取 `back()`，程序段错误。Windows 的 AVAS.dll 以文本模式打开文件，`\r` 已被去掉，所以不受影响。在 Windows 上编辑、再到 Linux 上运行的项目会遇到这个问题。

---

## 二、旧项目的结果静默改变

### 8. `displacepos`、`displacedpos` 不再被识别（实测）

单粒子偏移的关键字改成了 `initpos`、`initmom`，同时去掉了 ×0.01（`initmom` 的 x′ 单位是 mrad，δ 单位是 1e-3）。旧名字被当作未知关键字忽略，只有一般的未知关键字提示，粒子留在轴上。

用旧名字的项目（包括按旧手册写的项目和其他程序生成的 beam.txt）结果会改变，用户不容易察觉。例如一个经过 HWR 腔和螺线管的单粒子算例（`displacepos 1 0 0`、`displacedpos 300 200 0`）：2026-04-02 版粒子从 x = 1 mm、x′ = 3 mrad、y′ = 2 mrad 出发，末端能量 1.8835 MeV；07-22 版的 SingleParticle.txt 第一行 x = y = 0，偏移没有生效，粒子沿轴线运动。

建议：继续接受旧名字和旧单位；两套都写时以新名字为准，并给出警告。

### 9. 单粒子运行不再写 DataSet.txt 与 outData，SingleParticle.txt 丢了表头（实测）

- 粒子数为 1 时，DataSet.txt 为空，输出面不写 `outData_*.dst`，只写新的 SingleParticlePlane.txt。依赖 DataSet 或 outData 的后处理和界面都会得到空结果。
- `MethodMP::Run` 在 InitParticle 写完 3 行表头（dx、dpx、Energy）之后，又以截断方式打开 SingleParticle.txt，表头因此丢失。

建议：保留 DataSet 行和 outData；在 InitParticle 之前截断 SingleParticle.txt，或者截断后重新写表头。

---

## 三、输出问题

### 10. collisionData.txt 重复记录同一个粒子（实测）

| 算例（boundary.txt） | collisionData 行数 | 实际损失的粒子 |
|---|---|---|
| 圆管 + 矩形管 + 椭圆管 | 5000 | 3041 |
| 1 mm 厚的薄准直器 | 3599 | 2342 |
| superpose 复合段（问题 1 中能运行的写法） | 2574 | 2030 |

“实际损失的粒子”按速度三元组去重计数，与 DataSet 的存活数变化一致；重复的行内容完全相同。按行数统计损失会多算 20–50 %。

推测原因：

- 一步中走过的每一段都会调用 `Geometry::Boundary`；
- 复合段对各子形状的循环在碰撞后不停止。

同一个粒子因此被记录多次。

此外，粒子在孔径外从入口面进入时，07-22 版把碰撞点记在进入后的第一步（例如 z = 0.100143 m），而不是入口面（z = 0.1 m）。

### 11. z-code 段里的命名输出面丢失名字（代码阅读）

`outputplane d NAME` 在 t-code 段里写成 `NAME_<z>.dst`。但 `zcodeRun` 调用 `dstOut` 时一律传 `"yuzu"`，所以落在 quad、solenoid、bend、edge 处的命名输出面仍写成 `outData_<z>.dst`。另外，名字恰好写成 `yuzu` 的输出面也会写成 outData。

---

## 四、不一致之处

### 12. 二次粒子：文本输入与 .dst 输入对速度的处理相反（代码阅读）

- 文本文件：γ 和步长都只用 vz：`γ = 1/√(1 − (vz/c)²)`，`dt = seStep/vz`。有横向速度的粒子 γ 偏小（2026-04-02 版用 |v|）。
- .dst 文件：步长改为 `seStep/(β_tot·c)`（2026-04-02 版是 `seStep/vz`）。

两者正好相反，看起来是重构时把两处写反了。

### 13. 平均 γ 的求和不加权，却除以总权重（代码阅读）

07-22 版的平均 γ 改为 `⟨√(1 + (γβz)²)⟩_w`，只计纵向动量。但 `UpdateBeamParameters`（tcodeRun 入口处）求和时没有乘权重，却除以总权重。粒子权重不全为 1 时（`indweight 1` 或二次粒子），这个值是错的。

### 14. GenBunch 与正常运行生成的分布约定不同（代码阅读）

新导出的 `GenBunch` 有自己的一套解析和默认值：

- 质量 938.271875 MeV（正常运行用 beam.txt 或 938.272088…）；
- 能量 W 和相位 φ 按旧的只计纵向动量的约定计算；
- 解析了 `displacepos`、`displacedpos`，但没有使用。

因此它写出的 .dst 与同样输入下正常运行的 inData.dst 不一致。

### 15. superpose 误差场去掉了块外提前返回（代码阅读，请确认是否有意）

`Field::GetErrField` 类型 4 原来在粒子位于块外时直接返回零场（`|x| ≥ R || |y| ≥ R || z ≤ zStart || z ≥ zEnd`）。07-22 版去掉了这个判断，块外的粒子也会求子场，并可能按子场半径被判丢失。不加误差时的 `GetField` 本来就没有这个判断，所以这个改动可能是有意统一的，但它会改变误差研究的损失。

---

## 附：观察到但不确定是否算问题

- V3 = 0 扫相位得到的 φs 与设定值相差约 0.07°（设定 −33°，synData 记 −32.93°）。2026-04-02 版相同，应当是扫描步长的限制。如果需要精确的同步相位，可以在扫描后用二分或牛顿迭代细化。
- 2026-04-02 版 AVAS.dll 本身就要求停滞判丢在 2000 步之后（`step > 2000`），2026-04-02 之前的 libAVAS.so 没有这个条件，07-22 版 .so 已补上。两个平台此前在这一点上不一致。
