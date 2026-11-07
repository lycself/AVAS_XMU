# 比对实验记录

用原内核实际运行，核实各规格文档里"未解决的问题"。工具在 `engine/tools/oracle/`：`runner.py` 经 `avas run --mode basic` 运行（与用户使用的路径相同），`direct.py` 绕过 Python 的 lattice 处理、直接调用 `path()` 与 `main_agent()`。所有算例都从 `examples/hwr010` 出发，只改动下文写明的部分。

## 第一批（2026-11-07）

环境：Windows 11 / AVAS.dll（XPS，GTX 1650 机器，CPU 运行）；Ubuntu 24.04 / libAVAS.so（L40S 服务器，CPU 运行，OMP 限 32 线程）。示例的场图元件是 `field 0.21 0.02 0 1 162.5e6 -33 1.36 -1.36 efield`（下称 CAV），`D5` 是 `drift 0.05 0.02 0`。除特别说明外 beam.txt 加 `randomseed 12345`。

### 1. 界面功能是否在内核里生效

| 实验 | 结果 | 结论 |
|---|---|---|
| input.txt `scanphase 2`，scanData.txt 改成 `0.0 0.0`；另做 `scanphase 1` | DataSet.txt 与基准逐字节相同；日志多一条 "input.txt Contains unknown keywords" | **内核忽略 scanphase 和 scanData.txt**（A、B 的结论成立）。模拟设置页的"相位扫描 = 从 scanData.txt"对 CPU 运行无效 |
| `scmethod PICNIC` | 初始化报错 "SCMethod Contains unknown item, available options include FFT / SPICNIC" | **PICNIC 不可用**（F） |
| 删除 scmethod 行 | 与 `scmethod SPICNIC` 逐字节相同 | **默认是 SPICNIC**（F） |
| input.txt `secondarybeam 1` + SeParticle.txt（3 个粒子） | 报错 "The initial bunch distribution file cannot be opened, check particleDistributionPath" | **内核不读 SeParticle.txt**，二次粒子文件取自 beam.txt 的 readparticledistribution（G） |
| `main_agent(1)` 后紧接 `main_agent(2)` | 第二次调用完整重跑一次模拟（"Start scan phase…" 到 "Simulation completed"），返回 0 | **main_agent 不看参数**（A）。`avas/core/MultiParticle.py` 的 `stop()` 实际会再启动一次运行；目前没有调用者（界面停止运行靠结束子进程），是死代码 |

### 2. superpose、V3、DataSet 第 0 列（空间电荷关闭）

| 实验 | 结果 | 结论 |
|---|---|---|
| `D5 / CAV / D5`（腔单独） | φs = −32.9305°，末端能量 1.88343 MeV | 基准 |
| `D5 / superpose 0 0 0 0 0 0 / CAV / superpose 0 0 0 0 0 0 / drift 0.21 0.02 0 / superposeend / D5`（腔在块首） | synData 的 φs、φRF 为 NaN，末端 1.52694 MeV（无加速） | 腔在 superpose 块首时场丢失（A 缺陷 1 成立） |
| 同上但腔在块尾 | **同样 NaN、无加速** | A 预计块尾正常，**实测也失效**：原内核的 superpose 只要含 RF 腔就不能加速（至少在"腔 + drift"组合下） |
| `CAV / D5 / CAV`（两腔都 V3 = 0） | 两腔 φs = −32.92967°、−32.92950° | 目标 −33°，**系统偏差约 +0.07°**（B-3 的"扫描与主阶段差一步"） |
| 第一腔改 V3 = 2（φ = 0）：`field 0.21 0.02 2 1 162.5e6 0 1.36 -1.36 efield / D5 / CAV` | 第二腔 φs = −32.93020° | 扫描阶段 V3 = 2 的减号（B-1）**在这个布置下没有明显影响**下游 V3 = 0 腔 |
| DataSet 第 0 列：第一行与 inData.dst 比较 | 第一行 1.52694；inData 各粒子动能平均 1.5269415，由平均动量算的 (γ̄−1)mc² 为 1.5269223 | **第 0 列是平均动能**（与 AGENTS.md 一致）；D-07 的"由平均动量算"不成立 |

### 3. 内核自己的误差处理（direct.py 直接调用，单粒子 x 偏 1 mm，空间电荷关闭）

`avas run --mode basic` 生成的 lattice.txt 会去掉全部 `err_*` 行，所以这一组必须直接调用内核。lattice：`start / err_step a b / err_quad_dyn_on 0 0 0 0 0 1 0 / D5 / err_quad_ncpl_dyn 1 0 0 0 0 0 0 10 0 0 / quad 0.1 0.02 0 10 / D5 / end`（动态、r = 0、dG = 10 %）。比较 SingleParticle.txt 末行的 x 与 px：

| 实验 | 出口 x（m） | 等效梯度 |
|---|---|---|
| 无误差 G = 10 | 0.000465353 | 10 |
| 无误差 G = 11 | 0.000416116 | 11 |
| err_step 1 1 | 0.000416116 | 11（dG 10 % 按乘法施加） |
| err_step 1 3：output_0/1/2 | 0.000448857 / 0.000415030 / 0.000361641 | ≈10.33 / 11.02 / 12.1 |
| err_step 3 1：output_0/1/2 | 0.000416116 / 0.000362815 / 0.000305216 | 11 / 12.1 / 13.31 |

结论：**误差在一次内核调用的多个样本之间累乘**（G §4.1 成立）；err_step 的 b 是幅度分级数（10 %·k/b），a 是重复次数，与 schema 的"分组数 / 每组次数"不同。Python 的误差研究每次都以 `err_step 1 1` 调用内核，所以界面用户不受影响，但新内核要按这个语义实现。

束流误差（`err_step 1 1`，dx = 1 mm）：
- **静态束流误差**（err_beam_stat + err_beam_stat_on）：内核崩溃，"access violation reading 0x0000000000000000"（G §5 成立）。
- **动态束流误差**（err_beam_dyn + err_beam_dyn_on）：inData.dst 的 ⟨x⟩ 从 +0.00145 cm 变为 +0.10145 cm，平移正好 1 mm，结果写在 output_0/。

### 4. 可重复性与平台差异

| 实验 | 结果 |
|---|---|
| 同一输入（FFT）连续运行两次；再与另一次基准运行比较 | DataSet.txt 逐字节相同 |
| `multithreading 1` 运行两次；与不写该行比较 | 逐字节相同 |
| SPICNIC + `multithreading 1` 运行两次；与不写 multithreading 的 SPICNIC 比较 | 逐字节相同（F 推测的 SPICNIC 赋值缺陷在这个算例下没有表现出线程相关性） |
| 同一 beam.txt（seed 12345）在 Windows 与 Linux 生成初始分布 | inData.dst 不同（rms x 0.096162 对 0.095809 cm），两边都是 corr(x, φ) = −1 |
| 两个平台读入同一个 inData.dst（readparticledistribution） | 读入后的粒子逐字节相同；末端 rms x、y 相差约 2e-5，rms z 2.4e-4，能量 6e-5；同步相位一致到 1e-6 |

结论：原内核在固定种子下**完全确定**，同一平台可以做逐位比对；AVAS.dll 与 libAVAS.so 在相同粒子下也只相差 1e-5～1e-4 量级，但**不逐位一致**（E 指出 DLL 的 InitParticle 读回换算公式不同），而同一种子生成的粒子则因 C 库 rand() 不同而完全不同。

## 待做的实验

各文档"未解决的问题"中这一批没有覆盖的部分，尤其是 RFQ（需要 RFQ 输入样例）、boundary.txt 的 superpose 块、多极场图 Kb = G、二次粒子的输出面、奇数网格的 SPICNIC、多线程数不同（改变 OMP 线程数）对 SPICNIC 的影响。
