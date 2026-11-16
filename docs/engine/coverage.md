# 新内核对齐覆盖审计（M5b）
对规格 10–80 的 280 个对齐项与函数覆盖表中的函数逐一给出：新内核中的实现位置、覆盖它的比对算例或检验、compat=dll 与默认模式是否不同（指向 M1–M5 记录中的兼容开关清单）。“无算例”的条目写明原因。
代码路径相对 `engine/src/core/`；算例名见 `engine/tests/parity/cases.json`，`tools_check.py` 为 DLL 独有工具的比对，单元测试为 `engine/tests/unit_tests.cpp`，物理检验为 `engine/tests/physics/run_physics.py`。

## 1. 结论

- 对齐项 280 个：有算例或检验 267 个，无算例 13 个。
  - RFQ（原内核不能运行，没有参考结果）：7 个（B-21、C-22、C-23、C-24、C-25、C-26、C-27）。2026-11-16 起新内核默认模式实现，
    用单元测试（数值梯度）与合成 RFQ 的物理检验做自洽核对，见 M9-notes
  - 只写控制台或含时间、不可比较：4 个（D-13、D-14、E-05、H-24）
  - 需要特殊输入（反射粒子、同步粒子偏轴等）：1 个（D-28）
  - 不适用（.so 的 glibc 行为）：1 个（E-01）
- 函数 249 个（.so 214 个与 DLL 独有函数）全部有去处；其中“不需要”的是构造/析构、空壳、无调用者的旧路径与调试函数。
- RFQ 在默认模式下已实现（M9-notes，CPU）：原内核的 RFQ 从未能运行，兼容模式遇到 rfq 报错。
- M5b 审计中补上的遗漏：superpose 组内误差行、二次粒子、boundary.txt、lpboundary、longlimits、dumpperiodicity、pchistogram、track、importSCF、superposeout 结尾的 lattice、三个 DLL 工具，以及 FullParticle.txt 在“粒子全部丢失”时的输出、collisionData.txt 的粒子种类列。

## 10 input-lattice

| ID | 行为 | 实现 | 覆盖 | compat 与默认 |
|---|---|---|---|---|
| A-01 | `path` 给三个目录各追加 `/`，打印顺序 input、fieldMap、output | capi.cpp path / engine.cpp set_paths | 全部算例（console.txt） | 相同 |
| A-02 | `main_agent` 忽略传入值；返回 0/1/2；非 string 异常 abort | engine.cpp run_main_agent | 全部算例（rc.txt）、S5* | M1 清单：main_agent(2) 请求停止；非 string 异常 ErrorLog |
| A-03 | ErrorLog 格式 `YYYY/MM/DD HH:MM:SS     Initialisation stage error: msg\n`，追加模式 | engine.cpp error_out | S5*、E*、LB4（ErrorLog.txt） | 相同 |
| A-04 | CRLF / CR / LF 等价 | text.cpp normalize_line_endings | 单元测试 test_split；LB3（boundary.txt CRLF） | 相同 |
| A-05 | `!` 紧贴数值吞掉该数值 | text.cpp split_words | 单元测试 test_split | 相同 |
| A-06 | 只有空格、制表符是分隔符；逗号是词的一部分 | text.cpp split_words | 单元测试 test_split | 相同 |
| A-07 | 只把关键字转小写；值区分大小写 | params.cpp read_input / read_beam | 全部算例（关键字大小写混写见 example 输入） | 相同 |
| A-08 | lattice 关键字区分大小写，未知行静默忽略 | lattice.cpp dispatch | S5a、ER13 | 默认对未知关键字打印警告（M1） |
| A-09 | `method` 行额外打印 “input.txt Contains unknown keywords, check input.txt” | params.cpp read_input | CK_A09_method_line（console_check.txt） | 相同 |
| A-10 | `dimension` 报错 | params.cpp read_input | EP_A10_dimension（ErrorLog.txt） | 相同 |
| A-11 | `scutoffdistance`、`pchistogram`、input `randomseed` 按整数解析 | params.cpp read_input | D_seed_fraction、LT1（pchistogram 整数） | 相同 |
| A-12 | `steppercycle` 第二个值写 seStep | params.cpp read_input | SB1–SB5（seStep 决定二次粒子步长） | 相同 |
| A-13 | `setnumberofcores` 少于 10 个参数报错 | params.cpp read_input | EP_A13_cores_short（ErrorLog.txt） | 相同 |
| A-14 | `secondarybeam 1` 强制关闭空间电荷 | params.cpp read_input | SB1–SB4 | 相同 |
| A-15 | schema 的 `scanphase`、`sim_type`、`spacechargelong/type` 内核不读 | params.cpp read_input | S3c（scanphase 被忽略） | 相同 |
| A-16 | `twissx/y/z` 必须恰 4 词，文件顺序 α β ε | params.cpp read_beam | D_twiss_*、D_GS_GS 等生成算例 | 相同 |
| A-17 | `beamtype dc` 与 `twissz` 的顺序相关 | params.cpp read_beam | D_GS_dc_spread、D_KV_dc、D_WB_no_twissz | 相同 |
| A-18 | `displacepos/displacedpos` 乘 0.001 | params.cpp read_beam / beam.cpp init_single | S2a、S2b、Z*_single | 相同 |
| A-19 | `distribution` 必须 3 词 | params.cpp read_beam | D_*（distribution 两值） | 相同 |
| A-20 | beam 未知关键字（`initpos`、`use_dst`）只警告 | params.cpp read_beam | 全部算例（example beam.txt 含 use_dst） | 相同 |
| A-21 | `start` 之前的行忽略；`end` 之后的行忽略 | lattice.cpp LatticeReader::run | 全部算例 | 相同 |
| A-22 | 缺 `end` 时不做收尾 | lattice.cpp LatticeReader::run / synchronous.cpp scan_stage | EX_A22_no_end（不打印收尾行；相扫描开始处空指针读：Windows access violation，rc 与 ctypes 报错一致） | 默认收尾并警告；compat 用 compat_crash 模拟崩溃 |
| A-23 | z 按 word[1] 累加，bend 的 word[1]=0 不计长度 | lattice.cpp push_element | Z3、Z4、SB4（bend L = 0） | 相同 |
| A-24 | `quad` 少参数 / G=0 报错 | lattice.cpp add_quad | E1、E2 | 相同 |
| A-25 | drift/solenoid/steerer/bend/edge 参数不足只打印不退出 | lattice.cpp add_matrix / add_drift | S5b、E7 | M2 清单：默认报错 |
| A-26 | field 参数不足 `exit(1)` | lattice.cpp add_field | S5c | M1 清单：默认报错 |
| A-27 | 7 个单词的 steerer 通过首检后 abort | lattice.cpp add_matrix(steerer) | EX_A27_steerer_7_words（Windows：C++ 异常 0xe06d7363 逸出 DLL） | 默认 EngineError |
| A-28 | RF field 频率为 0 报错 | lattice.cpp add_field | EP_A28_rf_freq0（ErrorLog.txt） | 相同 |
| A-29 | Field 类型 0 不报错、类型 ≥5 打印 “Field type error!” 继续 | lattice.cpp add_field | CK_A29_field_type5（“Field type error!” 后继续；类型 0 不可观测） | 相同 |
| A-30 | V3==0 才置 needSynPhase | lattice.cpp add_field | S3a–S3c（V3 0/1/2） | 相同 |
| A-31 | 腔序号按 RF 元件出现顺序（field、rfq、superpose RF 片）编号 | lattice.cpp add_field / superpose_block | S3a、SP2、E5 | 相同 |
| A-32 | `outputplane` 相对当前末端；按出现顺序 | lattice.cpp dispatch(outputplane) | S4d、SB1、LB* | 相同 |
| A-33 | 超出末端或降序的输出面报 `Output plane anomaly, check automaticoutput` | lattice.cpp finish | EP_A33_plane_beyond_end、EP_A33b_plane_descending | 相同 |
| A-34 | `automaticoutput` 覆盖所有 `outputplane`；逐次累加；末端面补齐 | lattice.cpp finish | SP4、SP5（automaticoutput） | 相同 |
| A-35 | superpose 块长度、半径以 `%f` 舍入到 1e-6 | lattice.cpp superpose_block num() | SP1–SP7、SE* | M2 清单：默认 %.17g |
| A-36 | 块结束行被消耗（漏写 superposeend 吞掉下一元件） | lattice.cpp superpose_block terminator | SP6 | 默认保留结束行并警告 |
| A-37 | superpose 参数 3–6 个报错 | lattice.cpp superpose_block | E4 | 相同 |
| A-38 | superposeout 少于 7 词报错；mode 1/2 判定 | lattice.cpp superpose_block | SP4、SP5、E4 | 相同 |
| A-39 | 块中两个 RF 腔、RF + superposeout 报错 | lattice.cpp superpose_block | E5 | 相同 |
| A-40 | 情形 A（腔在块首）的 RF 片丢失全部子场（缺陷） | lattice.cpp superpose_block rfHead | SP2、SP3 | 默认正确的场行（M2） |
| A-41 | superpose 子 drift 被跳过 | lattice.cpp parse_superpose_subs | SP3 | 默认识别 drift 子块 |
| A-42 | multipole 生成 `.bsx/.bsy/.bsz`（6 位有效数字、x 最快）并按 Kb=G 叠乘 | lattice.cpp multipole_to_field / fieldmap.cpp | MP_sextupole_G1、G2 | M2 清单：默认 Kb = 1 |
| A-43 | multipole 极数非 3/4/5 报错 | lattice.cpp multipole_to_field | E6 | 相同 |
| A-44 | 误差关键字写入 ErrorParameter 的编码与补零规则 | lattice.cpp parse_error_line | ER1–ER21、SE1–SE4 | M5a 清单（束流开关补齐规则同 DLL） |
| A-45 | `cpl` 类误差、`adjust`、`diag_*`、`lattice`、`spacechargecomp`（.so）静默忽略 | lattice.cpp is_ignored / dispatch | ER13（cpl）、SC7（spacechargecomp） | 相同 |
| A-46 | boundary：`start` 前忽略、`1`/`2` 形状、未知词报错、`end` 不结束读取、长度不足报错 | lattice.cpp read_boundary | LB1–LB4 | 默认读到 end 停止 |
| A-47 | boundary superpose 块被结束行代替（缺陷） | lattice.cpp read_boundary(superpose) | EX_A47_boundary_sp_out、EX_A47b_boundary_sp_shape_end（任何块都在输出前 stoi 异常结束，Windows 0xe06d7363） | 默认按子几何加放置实现 |
| A-48 | ReadRFQ 读写 inputFilePath 下的 4 个文件及格式 | lattice.cpp read_rfq | RFQ1（expected：DLL 崩溃） | 默认模式只在两个源文件都在时转换（M9-notes） |
| A-49 | `dt = (1/f)/stepPerCycle` | tracker.cpp run（dt_） | 全部算例 | 相同 |
| A-50 | DLL 宽字符路径截断 | capi.cpp narrow | Windows 全部算例（ASCII 路径） | M1 清单：默认 UTF-8 |

## 20 synchronous-phase

| ID | 行为 | 实现 | 覆盖 | compat 与默认 |
|---|---|---|---|---|
| B-01 | 同步粒子初始 γβ 由 beam.txt kneticenergy、particlerestmass 计算（分布 unknown） | synchronous.cpp setup | S1、S2a | 相同 |
| B-02 | .dst 分布：SetSynPhase 用 particles[N].gbz，singlePartM 用 dst 平均能量；频率取 dst 头 ×1e6 | synchronous.cpp / beam.cpp | S4a–S4c、D_edst | 相同 |
| B-03 | beam.txt frequency 原样（Hz）作为步长分母 | synchronous.cpp scan_dt | S1–S3 | 相同 |
| B-04 | SynParticle::Update 三种推进（纵向、无横向磁场、Boris）逐位一致 | synchronous.cpp push | S1–S3、SP* | 相同 |
| B-05 | 只有存在 V3=0 RF field 时才扫描并重写 scanPhaseData.txt / Phase.txt | synchronous.cpp run_scan | S3*、SP2 | 相同 |
| B-06 | 扫描步长 scantstep/(f·N)、主阶段 0.01/(f·N)、单粒子 0.1/(f·N) | synchronous.cpp scan_dt / main_factor | S1–S3、S4* | M1 清单 B-3：默认同步长 |
| B-07 | 21 点扫描网格 −180:18:180，每点一行 `φ0 φs W_in W_out` | synchronous.cpp scan_cavity | S3a（scanPhaseData.txt） | 相同 |
| B-08 | φs 用 atan（±90°），末段乘两次比例、扫描用步首时刻 | synchronous.cpp scan_cavity | S3a | 相同 |
| B-09 | 两候选选择与端点 0↔20 修正 | synchronous.cpp scan_cavity | S3a、S5e | 相同 |
| B-10 | T<0 优先加速支；无加速支时取减速支并打印 "Note that the energy…" | synchronous.cpp scan_cavity | AGENTS.md 第 6 节实测（0.7 MeV 减速解）；S3a | 相同 |
| B-11 | 6 次割线迭代，SPD 写 ≤6 行，`s1 = a1 + 180` 的写法 | synchronous.cpp scan_cavity | S3a（synData、scanPhaseData） | 相同 |
| B-12 | 无解报错条件：\|φs−T\|>1 且不贴近 ±90 | synchronous.cpp scan_cavity | S5e | 相同 |
| B-13 | "3333 3333 3333 3333" 分隔行、Phase.txt 每腔两行（k+1、t、z、W，fixed 13 位） | synchronous.cpp write_phase | S3a（Phase.txt） | 相同 |
| B-14 | cout 的 "The k field, phi_0: …" 与 Entry/Export 行 | synchronous.cpp main_cavity | S3a（console.txt） | 相同 |
| B-15 | synData 行格式：首行精度 6、之后 17；非腔 `invalid invalid`；exitPlane 序号 n−1 / n 两种情况 | synchronous.cpp main_phase | 全部 synData.txt | 相同 |
| B-16 | 主阶段 φs 用步末时刻，腔出口 +1e-13，元件入口 +1e-15 | synchronous.cpp main_phase | S3a、S3b | 相同 |
| B-17 | V3=2：扫描阶段 `word6 − 360 f t_in`，主阶段 `word6 + 360 f t_in`（规约到 ±180） | synchronous.cpp register_cavity | S3b | M1 清单 B-1：默认 +360ft |
| B-18 | V3=1：φRF = word6，cavTime = 主阶段到达时刻 | synchronous.cpp register_cavity | S3c | 相同 |
| B-19 | 最终 fm.cavPhase/cavTime 交给束团：三种 V3 的等价相位 | synchronous.cpp finish | S3a–S3c、D_* | 相同 |
| B-20 | 反向运动报错 "Synchronous particle reverse motion, program stopped" | synchronous.cpp push | SI_B20_sync_reverse（hwr010 Ke 16、相位 180°，0.5 MeV）；SI_B20s_static_reverse（静电场反射：两边都挂起，超时） | 默认：静场中同步粒子回退到起点前也报反向错误（B-20s） |
| B-21 | RFQ 扫描：φ0 = (π/2 − 2πf_beam t)·180/π，减速时重扫 −π/2，仍减速报 "RFQ scanning phase abnormality" | lattice.cpp read_rfq（未实现推进） | - 无算例：缺 RFQ 输入样例（RFQ1 为 expected） | 默认模式实现，见 M9-notes；兼容模式报错 |
| B-22 | superposeout 中同步粒子的坐标变换与出口插值 | synchronous.cpp static_step | SP4、SP5、SE* | 相同 |
| B-23 | needSynPhase==0 时只 push RF field 的 word6，不处理 RFQ | synchronous.cpp finish | S1、S2a（无腔时） | 相同 |
| B-24 | singlePartM：先 ReadBeam，gbzIn=−33，0.1 步长，synData 全 17 位 | synchronous.cpp single_particle_mode | S1（singleparticlemode 见 M1） | 相同 |
| B-25 | SetUpSynParticle 打不开 beam.txt / dst / 无扩展名时的输出与退出码 | synchronous.cpp setup | S5d | M1 清单：默认 ErrorLog |
| B-26 | Drift 孔径外报 "Synchronised particles can't reach the exit plane…" 并 exit(1) | synchronous.cpp drift_aperture | SI_B26_sync_drift_aperture（rc、控制台、synData.txt） | 默认 ErrorLog |
| B-27 | TcodeTrans 单步：Boris 推进、相对坐标三种 rotMode、输出面插值 | tracker.cpp trans | 全部多粒子算例 | 相同 |
| B-28 | monotonicSearch 游标跨查询保留、找不到后全部失败 | spacecharge.cpp import_scf | LS1 | 相同 |
| B-29 | findClosest 并列取小下标、空向量抛 "Vector is empty!" | spacecharge.cpp import_scf | LS1 | 默认空向量时报错而不是 abort |
| B-30 | input.txt `scanphase` 被忽略（只打印未知关键字警告） | params.cpp read_input | S3c | 相同 |

## 30 fields-matrices

| ID | 行为 | 实现 | 覆盖 | compat 与默认 |
|---|---|---|---|---|
| C-01 | 主束元件定位：z == zEnd 归下一个元件；z<0、越过 spOut 出口或已丢失的粒子外场清零 | tracker.cpp external_field | 全部多粒子算例 | 相同 |
| C-02 | 单粒子定位版只清 Ex、Bx；z == zEnd 归当前元件 | synchronous.cpp get_field | S1–S3 | 相同 |
| C-03 | lost = 2 的设置条件 | tracker.cpp external_field | SP6（越过末端） | 相同 |
| C-04 | Drift 孔径判丢失（R ≤ r，boundary 0，spOut 条件） | tracker.cpp element_field(drift) | LB2（boundary 1 时关闭）、S4a | 相同 |
| C-05 | Drift 同步粒子越孔径时退出并输出英文提示 | synchronous.cpp drift_aperture | SI_B26_sync_drift_aperture | 默认 ErrorLog |
| C-06 | Quadrupole t-code 场 Bx = G·y，By = G·x | tracker.cpp element_field(quad) | Z1、Z5、ER16 | 相同 |
| C-07 | 硬边元件只走 z-code；bend/edge/steerer/solenoid t-code 场为零或只有 Bz | tracker.cpp segment_lattice | Z1–Z5 | 相同 |
| C-08 | 场图三线性插值、节点顺序（x 最快、z 最慢）、严格边界 | tracker.cpp trilinear（physics.h） | S1–S4、SC* | 相同 |
| C-09 | RF 场：×1e6×Ke、×Kb，E·cos φ，B·cos(φ−π/2)，φ 由 cavPhase/cavTime | tracker.cpp element_field(type 1) | S1–S4 | 相同 |
| C-10 | 静电（type 2）、静磁（type 3）场的分量位置与乘法顺序 | tracker.cpp element_field(type 2/3) | S2a（sol）、SP1 | 相同 |
| C-11 | 场图元件孔径丢失后本步外场保持旧值 | tracker.cpp element_field | S4a（孔径丢失） | 相同 |
| C-12 | 叠加组：局部坐标变换（P[1],P[2] 偏移，P[3]→P[5]→P[4] 旋转）、子类型编码 1 磁/2 RF/3 电、旋转回实验室系 | tracker.cpp superpose_field | SP1–SP7 | 相同 |
| C-13 | 叠加组孔径：不看 boundary；按元件 +0x6c 选局部坐标或相对坐标；丢失时保留部分和 | tracker.cpp superpose_field | SP4、SP5 | 相同 |
| C-14 | 叠加组 RF 子场共用组的频率与 cavIndex | tracker.cpp superpose_field | SP2 | 相同 |
| C-15 | 同步粒子 RF 只算 Ez 并写 EzAmp；静场正确清另一场 | synchronous.cpp get_field | S3* | 相同 |
| C-16 | 同步粒子在场图元件超孔径时输出 GBK 文本并 exit(1) | synchronous.cpp get_field | SI_C16_sync_field_aperture（GBK 文本，console_check.txt） | 默认 ErrorLog（英文） |
| C-17 | 文本 3D 场图头部与 norm（stoi 截断）解析 | fieldmap.cpp read_map | 全部场图算例 | 相同 |
| C-18 | 1D 场图回退：提示文字、mark 检查、横向均匀 Ez、无横向场 | fieldmap.cpp read_map | D_* 1D 场图（efield） | 相同 |
| C-19 | 二进制场图：名以 b 结尾、int32/float64 头 + float32 数据 | fieldmap.cpp read_binary | SI_C19_binary_map_dst（sol 转成 solb）；单元测试 | 相同 |
| C-20 | 打不开场图时的异常文本 | fieldmap.cpp | S5d | 相同 |
| C-21 | 场图按名字去重，序号返回 | fieldmap.cpp intern | S3a（同一场图两次） | 相同 |
| C-22 | RFQ getEleNum 边界（z == cellZEnd 属当前 cell）、getRadius 越界返回 1e99 | kernels.h rfq_field_t | engine/tests unit_tests test_rfq_field、physics/rfq_check.py | 默认模式实现，见 M9-notes；兼容模式报错 |
| C-23 | RFQ 8 项势场公式（含省略的 Bessel 因子、E = +∇U） | kernels.h rfq_field_t | engine/tests unit_tests test_rfq_field、physics/rfq_check.py | 默认模式实现，见 M9-notes；兼容模式报错 |
| C-24 | RFQ 轴上（r<1e-16）Ez 与轴外反号 | kernels.h rfq_field_t | engine/tests unit_tests test_rfq_field、physics/rfq_check.py | 默认模式实现，见 M9-notes；兼容模式报错 |
| C-25 | RFQ getEleNum = −1 时原内核越界读 | kernels.h rfq_field_t | engine/tests unit_tests test_rfq_field、physics/rfq_check.py | 默认模式实现，见 M9-notes；兼容模式报错 |
| C-26 | Bessel 级数 20 项 | kernels.h rfq_bessel_i | engine/tests unit_tests test_rfq_field、physics/rfq_check.py | 默认模式实现，见 M9-notes；兼容模式报错 |
| C-27 | RFQ 同步粒子逐 cell 相位：dt = 1e-4/f、边界线性插值 + 1e-15、atan 相位、−90 特例、能量下降中止 | synchronous.cpp rfq_pass | engine/tests unit_tests test_rfq_field、physics/rfq_check.py | 默认模式实现，见 M9-notes；兼容模式报错 |
| C-28 | 硬边调度：mode 0/1/2 的分派、tRef 取最小 t 或平均 t，edge 不改 mode/tRef | tracker.cpp zcode / het | Z1–Z5、SC5 | 相同 |
| C-29 | 误差坐标变换 T_in/T_out（含非互逆） | tracker.cpp het_err_in / het_err_out | ER1、ER9 | M5a：默认 T_out 为 T_in 的精确逆 |
| C-30 | 误差启用判据只看 dyn[idx+1][0] 与 flags[2]+flags[3] | tracker.cpp setup_errors(hetErr_) | ER1、ER8、ER9 | 相同 |
| C-31 | 半步空间电荷冲量：Δt = 2·r[4]，在误差变换之后 | tracker.cpp het（SC 冲量） | SC5、SC6、ER16 | 相同 |
| C-32 | 四极矩阵（聚焦/散焦分支按 qG 符号，cosh/sinh 参数顺序） | physics.h quad_row | Z1、Z5 | 相同 |
| C-33 | 时间推进 Δt = ℓ/vz，vz = sqrt(β²c²/(x'²+y'²+1)) | physics.h zrow_advance | Z1–Z4 | 相同 |
| C-34 | 螺线管矩阵与 K = qB/(2p) | physics.h solenoid_row | Z2 | 相同 |
| C-35 | 弯转：0.85 半步识别、δ 相对同步行 γβz、ky == 0 走漂移、色散符号 | tracker.cpp bend_step / physics.h bend_row | Z3、Z4 | M2 清单：显式半步 |
| C-36 | 边缘薄透镜 k1、k2 及 hv 交换平面 | physics.h edge_row | Z3、Z4 | 相同 |
| C-37 | 同步行推进：quad 用 L/(βc)，bend 清零横向动量，无误差与冲量 | tracker.cpp sync_het | Z1–Z4 | 相同 |
| C-38 | 校正铁冲量方向、Δ 用后接元件长度与第 0 粒子荷质比、不作用于同步行 | tracker.cpp kick | ST1–ST6 | M2 清单：默认各粒子 q/m |
| C-39 | 弯转色散方向：α>0 时高动量粒子出口 x>0 | physics.h bend_row | Z3、Z4（AGENTS.md 第 6 节方向记录） | 相同 |
| C-40 | 硬边元件参数非法（G=0、B=0、n≥1、n<0、α=0）产生 NaN 而不报错 | lattice.cpp add_quad / add_matrix | E1 | 相同 |

## 40 loop-output

| ID | 行为 | 实现 | 覆盖 | compat 与默认 |
|---|---|---|---|---|
| D-01 | dt = (1/f)/V1；Run 打印的 "Simulation time step is" 为该值 | tracker.cpp run（dt_） | 全部算例（console） | 相同 |
| D-02 | 元件分段与 markers（§3.2），第一元件为 z 型时报错原文 | tracker.cpp segment_lattice | E3、Z* | 相同 |
| D-03 | Run 截断的文件列表；PCHistogram.dat、SingleParticle.txt 不截断 | tracker.cpp run（截断） | 全部算例（files.txt） | 相同 |
| D-04 | BeamSet.plt 头（dumpperiodicity≠0）与 beamOut 记录二进制格式 | tracker.cpp run / beam_out | LT1、LT2（BeamSet.plt） | 相同 |
| D-05 | tcodeRun 每步顺序：SC → 宏粒子推进 → … → 同步粒子推进 | tracker.cpp tcode | 全部算例（synParticle ↔ DataSet） | 相同 |
| D-06 | DataSet.txt 41 列定义、时间对齐（首行第 39 列 −1、第 40 列 tMin+dt） | tracker.cpp tcode（DataSet 行） | 全部算例 | 相同 |
| D-07 | DataSet 第 0 列 = (γ̄−1)mc²（平均动量），不是平均能量 | tracker.cpp tcode | 全部算例（AGENTS 第 6 节：第 0 列为平均动能，已按实测实现） | M1 清单：默认加权完整 γ |
| D-08 | α/β/ε 定义（x'=Δγβx/γβz_i，εn = ε·β·γ，εnz = γ³βεz） | tracker.cpp tcode | 全部多粒子算例 | 相同 |
| D-09 | 第 28 列存活数含 lost==2；整数格式 | tracker.cpp tcode | SP6 | 相同 |
| D-10 | 第 35–38 列弯转/位移标志 | tracker.cpp tcode prev[14..17] | SP4、SP5 | 相同 |
| D-11 | DataSet 数值格式（%g 6 位，单空格，行尾 '\n'） | format.cpp fmt_g | 全部算例 | 相同 |
| D-12 | synParticle.txt 格式（fixed12，分隔符，18 字段） | tracker.cpp syn_out | 全部算例（synParticle.txt 去时间戳） | 相同 |
| D-13 | 进度行格式、3 s 间隔、R 含 0.5 因子 | tracker.cpp progress | - 无算例：只写控制台，含运行时间 | 相同 |
| D-14 | SimulationTime.txt 一行格式（precision 5、c5 重复） | tracker.cpp run（SimulationTime） | - 无算例：含计时，只比较存在性 | 相同 |
| D-15 | outData_<z>.dst TraceWin 格式、文件名 `%f`、φ=2πf(t−t_s)、W 用同步粒子 mc² | tracker.cpp dst_out | S4*、Z*、SC* | 相同 |
| D-16 | .dstb（indweight 1）与 .edst（初始分布来自 .dst）格式 | tracker.cpp dst_out | D_dst_weight、D_edst | 相同 |
| D-17 | 末端总写 outData_<L>.dst；输出面越界报 "Output plane anomaly…" | lattice.cpp finish | 全部算例（outData_<L>） | 相同 |
| D-18 | 面间距 < 束长报 "The output plane interval is less than the bunch length…" | tracker.cpp tcode | EP_D18_plane_interval（ErrorLog.txt、出错前的输出） | 相同 |
| D-19 | 输出面在 z 段元件内部 → 元件中点输出；在出口 → 出口输出 | tracker.cpp zcode | S4d、ER*（中点 dst） | M2 清单：默认参考时间在中点 |
| D-20 | 校正铁冲量方向与大小（磁 −By·L·q/(mc)，电 Ex·L·q/(mc²β)），施加于其后元件中点 | tracker.cpp kick | ST1–ST6 | 相同 |
| D-21 | zTotcode 重建（x−γβx·Δt/γ·c，z=zPlane−vzΔt，同步粒子强制在轴） | tracker.cpp ztot | Z*、ST* | M2 清单：默认完整 γ |
| D-22 | zcodeRun 空间电荷在元件中点、每元件一次；无 SC 且无输出时整段传输 | tracker.cpp zcode | SC5、SC6 | 相同 |
| D-23 | Boris 推进公式（§4.5） | physics.h boris_push | 全部多粒子算例；单元测试 test_physics | 相同 |
| D-24 | 空间电荷每 t 步一次、位置在推进前 | tracker.cpp tcode / spacecharge.cpp solve | SC1–SC14 | 相同 |
| D-25 | LongitudinalBunchLoss 判据（相位按 βs 换成长度，能量差绝对值） | tracker.cpp longitudinal_loss | LL1 | 相同 |
| D-26 | boundary.txt 直管/弯管碰撞与 collisionData.txt 一行格式 | tracker.cpp boundary_check / geometry_hit | LB1、LB2 | 默认几何段号夹取为 [r1, r2] |
| D-27 | lpboundary 周期折返 ±βλ/2（h 用同步粒子 γβz 与平均 γ̄） | tracker.cpp lp_fold / spacecharge.cpp weight | LP1–LP3 | M5b：默认周期 βλ、网格周期 βγλ |
| D-28 | 停滞判丢及 LongitudinalBeamLoss.txt 格式（id 与 x 无分隔） | tracker.cpp tcode（停滞判丢） | - 无算例：判丢要求粒子在 t-code 区 80 % 以上的步数里不越过下一平面；hwr010 强减速场反射的粒子在 z<0 处立即判丢，腔内往复的粒子最多约 2800 步（需要 >4000 步）；superposeout 块内反射粒子不判丢，两个内核都不结束（挂起） | 相同 |
| D-29 | pchistogram → PCHistogram.dat 二进制格式与 else-if 范围更新 | tracker.cpp ex_output | LT1（PCHistogram.dat） | 默认 min/max 分别比较 |
| D-30 | track → ParticleTracking.txt 20 列 | tracker.cpp single_track | LT1、LT2（ParticleTracking.txt） | 默认越界序号报错 |
| D-31 | N==1 → SingleParticle.txt（能量用 938.271875） | tracker.cpp single_out | S1、Z*_single | M1 清单：默认实际 mc² |
| D-32 | multithreading 0/2/3 → "End of simulation, all particles lost."；1 正常 | tracker.cpp tcode | EX_D32_multithreading0（ErrorLog.txt：粒子全部丢失） | 相同 |
| D-33 | cores 表按 N 选取并截到 omp_get_max_threads/maxthreads；cores[0] 被改写为块数 | beam.cpp default_cores / spacecharge.cpp | SC*（SimulationTime 不比较） | M4：默认分区不随机器线程数截断 |
| D-34 | 块内/块间求和顺序固定（同核数下结果可重复） | tracker.cpp tcode 块归约 | 全部算例；run_all thread_check | 相同 |
| D-35 | superposeout 出口重启（t = tmin − dt，未越过者 lost=1） | tracker.cpp tcode（superposeout 重启） | SP4、SP5 | M2 清单 |
| D-36 | "Tcode won't start…" 报错条件与消息（含两个 %f） | tracker.cpp tcode | EP_D36_tcode_wont_start（ErrorLog.txt） | 相同 |
| D-37 | UpdateBeamParameters 的 betaGammaLambda 与 rms 定义（含 x−⟨x−xs⟩ 缺陷） | tracker.cpp update_beam_parameters | SC*（首个网格） | M4 清单：默认相对坐标 |

## 50 distribution-rng

| ID | 行为 | 实现 | 覆盖 | compat 与默认 |
|---|---|---|---|---|
| E-01 | glibc `rand()` 序列 | rng.h | - 无算例：glibc rand 只用于 .so | 不需要（新内核以 DLL 为基准） |
| E-02 | MSVC `rand()` 序列 | rng.h MsvcRand | 单元测试 test_random；D_* | 相同 |
| E-03 | 种子优先级 | params.cpp / beam.cpp default_seed | D_seed_input、D_seed_fraction | 相同 |
| E-04 | 种子截断 | beam.cpp default_seed | D_seed_fraction | 相同 |
| E-05 | seed 0 不可复现 | beam.cpp default_seed | - 无算例：时间种子不可复现（默认写 inData.seed.txt） | M3 清单 |
| E-06 | 横纵重播种导致相关 | distribution.cpp dll_generate | D_GS_GS（compat corr = −1） | M3 清单：默认独立流 |
| E-07 | 不同分布类型也共用序列 | distribution.cpp dll_generate | D_KV_GS、D_GS_WB | 相同 |
| E-08 | KV 超曲面 | distribution.cpp KV | D_KV_KV、run_all distribution_check | 相同 |
| E-09 | WB 径向分布 | distribution.cpp WB | D_WB_WB | 相同 |
| E-10 | PB 径向分布 | distribution.cpp PB | D_PB_PB | 相同 |
| E-11 | GS 不截断 | distribution.cpp GS | D_GS_GS | 相同 |
| E-12 | 横向 Twiss 与发射度换算 | beam.cpp init_generated | D_twiss_* | 相同 |
| E-13 | 纵向换算 | beam.cpp init_generated | D_* 纵向 | M3 清单：默认 δ 作用于总动量 |
| E-14 | φ 符号 | beam.cpp init_generated | D_* | 相同 |
| E-15 | DC 束相位 | beam.cpp init_generated(dc) | D_GS_dc_spread、D_KV_dc | 相同 |
| E-16 | 不写 twissz 视为 DC | params.cpp read_beam | D_WB_no_twissz | 相同 |
| E-17 | inData.dst 格式 | beam.cpp dst_bytes | G0、D_*（inData.dst） | 相同 |
| E-18 | 同步粒子能量 = 粒子平均 W | beam.cpp init_generated | D_* | 相同 |
| E-19 | 束团时间范围 | beam.cpp bunch_time | D_*、S4* | 相同 |
| E-20 | 读回换算 | beam.cpp readback | S4*、D_* | M1 清单 |
| E-21 | rpm 与宏粒子电荷 | beam.cpp init_generated / init_from_dst | S4*、SC* | 相同 |
| E-22 | 单粒子模式 | beam.cpp init_single | S1、Z*_single | 相同 |
| E-23 | probe 网格 | beam.cpp init_generated(probe) | D_probe、D_probe_n100（expected） | M3 清单 |
| E-24 | .dst 复制与误差 | beam.cpp init_from_dst | S4*、ER11 | 相同 |
| E-25 | 权重文件 | beam.cpp init_from_dst(indweight) | D_dst_weight | 相同 |
| E-26 | .edst | beam.cpp init_from_edst | D_edst | M3 清单：atomicMass |
| E-27 | 二次束文本 | beam.cpp init_secondary | SB1、SB4、SB5 | M5b：默认 γ 用总速度 |
| E-28 | 二次束 .dst | beam.cpp init_secondary(.dst) | SB2、SB3 | M5b：默认束流误差 ×0.1 |
| E-29 | Twiss 误差 | beam.cpp init_generated（beam_errors） | ER10 | M5a 清单 |
| E-30 | 空间电荷默认参数 | spacecharge.cpp space_charge_defaults | SC* | 相同 |
| E-31 | 线程数默认 | beam.cpp default_cores | SC* | M4 清单 |
| E-32 | 报错信息 | beam.cpp | S5d、D_* | 相同 |
| E-33 | GenerateCoordinates NaN 保护 | distribution.cpp GenerateCoordinates | D_* | 相同 |
| E-34 | GS2D 互异重抽 | distribution.cpp GS2D | D_GS_GS | 相同 |
| E-35 | GenerateGaussian 成对缓存 | errorset.cpp DllGaussian / distribution.cpp | ER3、ER7、D_GS_GS | 相同 |
| E-36 | Windows 量化 | distribution.cpp | D_*（Windows 逐位） | 相同 |

## 60 space-charge

| ID | 行为 | 实现 | 覆盖 | compat 与默认 |
|---|---|---|---|---|
| F-01 | scmethod 只接受 FFT / SPICNIC，PICNIC 等抛错 | params.cpp read_input | EP_F01_scmethod_picnic（ErrorLog.txt） | 相同 |
| F-02 | 不写 scmethod 时默认 SPICNIC | params.cpp read_input | SC1（未写 scmethod） | 相同 |
| F-03 | FFT 默认 meshrms 6.5、网格 64³ | spacecharge.cpp space_charge_defaults | SC2、SC8 | 相同 |
| F-04 | SPICNIC 默认 meshrms、网格按粒子数分档 | spacecharge.cpp space_charge_defaults | SC1、SC9 | 相同 |
| F-05 | 默认 cores 按粒子数取表并截断到 omp 最大线程数 | beam.cpp default_cores | SC* | M4 清单 |
| F-06 | 流强 0 或单粒子关闭空间电荷 | beam.cpp init_generated | D_*（单粒子、流强 0） | 相同 |
| F-07 | 网格以束流中心为中心、节点覆盖 ±meshrms·rms，静止系 z 拉伸 γ | spacecharge.cpp weight | SC* | 相同 |
| F-08 | CIC 权重与分配顺序 | spacecharge.cpp weight | SC*；物理检验 ellipsoid | M4 清单：默认单遍累加 |
| F-09 | 网格外粒子用点电荷场，Particle+0xf8 标志 | spacecharge.cpp allocate | SC* | 相同 |
| F-10 | FFT 求解：DST-I 横向 + 周期 FFT 纵向，归一化 1/(2(Nx+1)·2(Ny+1)·Nz) | spacecharge.cpp fft | SC2、SC8、SC11、SC13、SC14；物理检验 | pocketfft 与 FFTW 末位差（1e-10 容差，用户已定） |
| F-11 | 中心差分求 E，横向 φ 边界外为 0、纵向周期 | spacecharge.cpp fft | SC2 | 相同 |
| F-12 | 静止系 → 实验室：E⊥/γ、Ez 不变、B = 0 | spacecharge.cpp allocate | SC* | 相同 |
| F-13 | rotMode 1/2 时旋转 SC 场 | spacecharge.cpp allocate | SP4、SP5（rotMode） | 相同 |
| F-14 | SPICNIC 格林函数制表（近场 10³ 中点积分 / V、远场 1/r、截断 0.75Nx） | spacecharge.cpp tabulate | SC1、SC3–SC7 | M4 清单：默认解析积分、全网格核 |
| F-15 | SPICNIC 对称化：只用上卦限电荷，结果镜像 | spacecharge.cpp spicnic | SC1 | 相同 |
| F-16 | SPICNIC 非对称修正"最后一个赢"，结果依赖 c7 | spacecharge.cpp spicnic | X_c7_1、X_c7_12（实验） | M4 清单：默认全部累加 |
| F-17 | negligibleratio 阈值（中心格点电荷 × 比例，严格大于） | spacecharge.cpp spicnic | SC4 | 相同 |
| F-18 | 奇数网格 SPICNIC 死循环 | spacecharge.cpp 构造函数 | X_odd_grid（实验） | 两种模式报错（DLL 挂起） |
| F-19 | Ny ≠ Nz 时 SPICNIC 的 j 起点用 Nz/2 | spacecharge.cpp spicnic | X_ny_nz_a、X_ny_nz_b（实验） | M4 清单 |
| F-20 | autoPIC 只比较 rms_z 的相对变化（< 1e-8 跳过） | spacecharge.cpp solve(autoPIC) | SC1、SC12 | 相同 |
| F-21 | 跳过求解时粒子保留旧 scE | spacecharge.cpp solve | SC1 | 相同 |
| F-22 | lpboundary 1：dz = βγλ/(Nz−1)、网格以同步粒子为原点、并**不**强制 FFT | spacecharge.cpp weight（lpBoundary） | LP2、LP3 | M5b：默认周期 βγλ、FFT |
| F-23 | recordSCF 文件格式：二进制追加、k-j-i 顺序、静止系场；文本行 12 列 6 位有效数字 | spacecharge.cpp record | SC12–SC14（SpaceChargeMesh.txt、SpaceChargeSet.plt） | 相同 |
| F-24 | importSCF 时间折叠（改写 Beam.time）、按最近中心 z 选记录、点电荷用拉伸 z | spacecharge.cpp import_scf | LS1 | 相同 |
| F-25 | t < times[0] 时 SC 场为 0 | spacecharge.cpp import_scf | LS1（前段） | 相同 |
| F-26 | z-code 的 HESCFieldPre：同一时刻换算、不加权中心与 rms、不更新 γ | spacecharge.cpp hesc_field_pre | SC5、SC6、SC11 | M4 清单（DLL 数据竞争） |
| F-27 | 多线程 Weight 与串行的差异 | spacecharge.cpp weight | SC*（cores[4]） | M4 清单 |
| F-28 | spacechargecomp（仅 DLL）把电荷乘 (1 − V1)，V1 ≥ 0.999999999 不缩放 | spacecharge.cpp weight / lattice.cpp | SC7 | M3 清单 |
| F-29 | spacechargelong / spacechargetype 内核不认识 | params.cpp read_input | CK_F29_spacechargelong（console_check.txt） | 相同 |
| F-30 | FFT 归一化分母按 int 相乘可能溢出 | spacecharge.cpp fft_norm | 单元测试 test_fft_norm（1024³ 网格 compat 按 32 位 int 溢出） | 默认 64 位不溢出 |

## 70 errors-secondary

| ID | 行为 | 实现 | 覆盖 | compat 与默认 |
|---|---|---|---|---|
| G-01 | 内核忽略 `err_*_cpl_*` | lattice.cpp is_ignored | ER13 | 相同 |
| G-02 | 动态 r=0、`err_step 1 1` 时施加值 = 写入值 | errorset.cpp generate_error_set | ER1、ER4、Python 误差研究 | 相同 |
| G-03 | 输出目录 `output_<k>`，k=j·a+i | engine.cpp run_error_samples | ER* | 相同 |
| G-04 | b 为幅度级数 | errorset.cpp generate_error_set | ER2、ER4 | 相同 |
| G-05 | 误差序列与 randomseed 无关 | errorset.cpp generate_error_set | ER2、单元测试 test_error_sampling | M5a：默认 Philox 按种子 |
| G-06 | 静态 r=1 为 [0,A)，动态 r=1 为 [−A,A) | errorset.cpp generate_error_set | ER2、ER5 | M5a：默认对称均匀 |
| G-07 | 动态 r=2 为 σ=A 的不截断高斯 | errorset.cpp DllGaussian | ER3、ER7 | M5a：默认 σ = A 高斯（静态同） |
| G-08 | 静态 r=2/3、动态 r=3 报错退出 | errorset.cpp generate_error_set | ER15 | 默认 EngineError |
| G-09 | 束流 r=2 → 0；r 非 0/1/2 → 报错（动态带 0xD6B8 前缀） | errorset.cpp generate_error_set | ER11、ER19 | 相同 |
| G-10 | 静态元件误差只作用于场图类型 3/1 | errorset.cpp generate_error_set | ER5、ER8、ER17 | 相同 |
| G-11 | 动态 quad 误差作用于 field3/quad/solenoid/bend/edge，N 计入所有元件 | errorset.cpp generate_error_set | ER1、ER8、ER9 | 相同 |
| G-12 | 每种开关只用最后一条 | lattice.cpp parse_error_line | ER*（多条开关见 ER18） | 相同 |
| G-13 | 跨样本累积（四极/螺线管） | errorset.cpp set_errors_in_elements / engine.cpp | ER4、ER4m* | M5a：默认每样本从标称出发 |
| G-14 | Field 不累积；RF 的 ke/kb 与相位误差 | errorset.cpp set_errors_in_elements | ER5–ER7、ER18 | 相同 |
| G-15 | 场图错位/转角变换与孔径丢失 | tracker.cpp element_err_field | ER5、ER8、ER17 | 默认孔径条件同 GetField |
| G-16 | 静、动同时开时 t-code 用静态行标记 | tracker.cpp setup_errors | ER18 | M5a：默认任一标记 |
| G-17 | 同步粒子主束不受元件误差影响 | tracker.cpp external_field | ER5–ER7 | 相同 |
| G-18 | 束流动态误差作用 | beam.cpp beam_errors | ER10、ER11 | 相同 |
| G-19 | 静态束流误差不生效 / 可能崩溃 | beam.cpp beam_errors | ER12（expected：DLL 崩溃） | M5a：默认施加 |
| G-20 | 束流开关顺序缺陷 | lattice.cpp parse_error_line / errorset.cpp | ER19、ER21 | M5a：默认按模式取行 |
| G-21 | errSampleOut 文件格式 | errorset.cpp write_error_sample | ER*（Static/DynamicErrorData.txt） | 相同 |
| G-22 | 二次粒子文件名来自 readparticledistribution | beam.cpp init_secondary | SB1–SB4 | M5b：默认无文件名时读 SeParticle.txt（SB5） |
| G-23 | 二次粒子初始化 | beam.cpp init_secondary | SB1、SB2 | M5b 清单 |
| G-24 | 二次粒子 t-code 推进（漂移） | tracker.cpp se_trun | SB1–SB4 | M5b 清单（比较寄存器） |
| G-25 | 只在段末面输出 | tracker.cpp se_trun | SB1（中间面不输出）、SB5（默认输出） | M5b：默认每个输出面都写 |
| G-26 | outData_<z>.txt 格式 | tracker.cpp se_plane_out | SB1–SB4（outData_*.txt） | 相同 |
| G-27 | SeSynParticle.txt 格式 | tracker.cpp se_syn_out | SB1（首行 DLL 读未初始化内存，忽略） | 相同 |
| G-28 | 步数上限 = 同步粒子单段最大步数 ×10 | tracker.cpp se_trun（maxSec_） | SB2（.dst 粒子） | 相同 |
| G-29 | z-code 段四极/螺线管/弯铁/边缘的传输 | tracker.cpp se_quad / se_solenoid / se_bend / edge_row | SB2、SB4 | 相同 |
| G-30 | z-code 出口孔径判断 | tracker.cpp se_zrun | SB4 | 相同 |
| G-31 | 校正铁对二次粒子无效 | tracker.cpp run_secondary | SB6_secondary_steerer 与 SB6n_secondary_no_steerer（run_all：DLL、compat、默认三方都相同） | 相同 |
| G-32 | 二次模式下 DataSet.txt 为空、BeamSet.plt 只有头 | tracker.cpp run_secondary | SB1–SB4（files.txt、DataSet.txt 为空） | 相同 |
| G-33 | 第一段为硬边元件时报错 | tracker.cpp segment_lattice | E3 | 相同 |

## 80 dll-extras

| ID | 行为 | 实现 | 覆盖 | compat 与默认 |
|---|---|---|---|---|
| H-01 | 三个导出的 ctypes 签名与返回值 0 | capi.cpp sampling / meshing / dataprocess | tools_check.py | 默认错误返回码而不是 exit |
| H-02 | 宽字符路径去零字节转换；非 ASCII 路径被破坏 | capi.cpp narrow | tools_check.py（Windows） | M1 清单：默认 UTF-8 |
| H-03 | .dst 读写格式与写出头部字节顺序 `h2,h0,N,Ib,f,h2` | tools.cpp read_dst / write_dst | tools_check.py（mesh.dst、samp.dst 逐字节） | 相同 |
| H-04 | 权重文件名为输出全名加 `b`，格式 int32 + double[] | tools.cpp write_dst | tools_check.py（.dstb 逐字节） | 相同 |
| H-05 | meshing：均值、总体 rms、步长 2σ·mag/(n−1)、节点中心公式 | tools.cpp tool_meshing | tools_check.py | 相同 |
| H-06 | meshing：任一维越界即丢弃；64 角 CIC，权重和 = 网格内粒子数 | tools.cpp tool_meshing | tools_check.py | 相同 |
| H-07 | meshing：非零节点字典序输出（第 5 维最快） | tools.cpp tool_meshing | tools_check.py | 相同 |
| H-08 | sampling：坐标换算（z、δ、漂移回 φ_min） | tools.cpp tool_sampling | tools_check.py（samp.dst 逐字节） | 相同 |
| H-09 | sampling：6D 半径排序与 I[0]、I[N−1] 两个附加粒子 | tools.cpp tool_sampling | tools_check.py | 相同 |
| H-10 | sampling：段权重、段大小、D、抽样位置公式 | tools.cpp draw / seg_weights | tools_check.py | 默认最终抽样也做 idx < N 检查 |
| H-11 | sampling：偏移范围 A 与偏移内四指标秩和选优 | tools.cpp tool_sampling | tools_check.py（SPTest.txt 逐字节） | 相同 |
| H-12 | sampling：Mismatch / MSE / DeltaMean / DeltaHalo 定义 | tools.cpp tool_sampling | tools_check.py | 相同 |
| H-13 | sampling：全局秩和选优与最终输出用原始坐标 | tools.cpp tool_sampling | tools_check.py | 相同 |
| H-14 | sampling："Magnification is too large S M k" 后 exit(0) | tools.cpp tool_sampling | tools_check.py sampling_too_large（rc 与控制台） | 默认返回错误码 |
| H-15 | SPTest.txt 每次调用先清空、之后追加；格式与分隔空格 | tools.cpp tool_sampling | tools_check.py | 相同 |
| H-16 | dataprocess：区间切分（≤4 段）与 "NumofThreads" 输出 | tools.cpp tool_dataprocess | tools_check.py（console） | 相同 |
| H-17 | dataprocess：文件发现、z 解析（`_` 后 8 字符）、按 z 排序 | tools.cpp process_index | tools_check.py（两个 z） | 相同 |
| H-18 | dataprocess：FiveCube 第 0 个 `.dstb` 必须存在，s_F = N_O0/Σw_F0 | tools.cpp process_index | tools_check.py | 相同 |
| H-19 | dataprocess：36 个统计量（εn、α、β、均值、σ、W̄、N） | tools.cpp process_index | tools_check.py（.ppj 逐字节相同） | 相同 |
| H-20 | dataprocess：网格尺度取 FiveCube 的 σ，节点中心取当前数据集均值，双线性 CIC | tools.cpp process_index | tools_check.py（.ppj 逐字节相同） | 相同 |
| H-21 | dataprocess：.ppj 头与记录布局 | tools.cpp process_index | tools_check.py（长度与布局相同） | 相同 |
| H-22 | `spacechargecomp V1` 在元件入口把 Beam 补偿系数设为 V1（DLL）；.so 忽略 | lattice.cpp / spacecharge.cpp | SC7 | M3 清单 |
| H-23 | 相位扫描与读初始分布在 DLL 与 .so 中行为相同 | synchronous.cpp / beam.cpp | S3*、S4* | 相同 |
| H-24 | 未引用字符串不产生任何输出文件 | - | - 无算例：不产生输出 | 不需要 |

## 函数去处

| 规格 | 函数 | 新内核 |
|---|---|---|
| 10 | FUN_0011f420 | 不需要：库初始化 |
| 10 | errorOut | engine.cpp error_out |
| 10 | path | capi.cpp path |
| 10 | main_agent | engine.cpp run_main_agent |
| 10 | Initial::~Initial | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Beam::~Beam | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Bend::SetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Bend::Bend | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Bend::GetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Element::GetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Element::SetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Boundary::Boundary | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Boundary::~Boundary | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Boundary::SetElement | lattice.cpp add_geometry |
| 10 | Drift::SetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Drift::GetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Edge::SetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Edge::Edge | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Edge::GetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Element::SetBasicParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Field::~Field | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Field::Field | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Field::SetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Field::GetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Element::~Element | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | PreTreatment | distribution.cpp pre_treatment |
| 10 | Geometry::SetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | StringSplit | text.cpp split_words |
| 10 | ReadRFQ | lattice.cpp read_rfq（默认模式只在两个源文件都存在时转换，M9-notes） |
| 10 | Initial::errorOut | engine.cpp error_out |
| 10 | Initial::MultipoleFieldMapConverter | lattice.cpp multipole_to_field / fieldmap.cpp |
| 10 | Initial::MultipoleFieldMapGenerator | lattice.cpp multipole_to_field / fieldmap.cpp |
| 10 | Initial::InitMethodMP | tracker.cpp Tracker 构造（直接读 Params） |
| 10 | Initial::Initial | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Initial::NormalizeLineEndings | text.cpp normalize_line_endings |
| 10 | Initial::ReadBeam | params.cpp read_beam |
| 10 | Initial::Read | params.cpp read_input |
| 10 | Initial::InitLattice | lattice.cpp LatticeReader / read_boundary |
| 10 | Geometry::Geometry | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | RFQ::RFQ | lattice.cpp add_rfq（默认模式；兼容模式报错，M9-notes） |
| 10 | Lattice::Lattice | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Lattice::~Lattice | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Lattice::SetElement | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | BinaryFileReader::BinaryFileReader（构造） | fieldmap.cpp read_binary / spacecharge.cpp import_scf |
| 10 | BinaryFileReader::~BinaryFileReader | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Quadrupole::SetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Quadrupole::Quadrupole | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Quadrupole::GetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | RFQ::SetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Solenoid::SetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Solenoid::Solenoid | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Solenoid::GetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Steerer::SetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 10 | Steerer::Steerer | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 10 | Steerer::GetParameter | lattice.cpp（各元件 add_*、set_basic；参数直接存于 Element） |
| 20 | Initial::SetSynPhase | synchronous.cpp set_synchronous_phase |
| 20 | Initial::singlePartM | synchronous.cpp（single-particle mode） |
| 20 | SynParticle::SynParticle | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 20 | SynParticle::~SynParticle | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 20 | SynParticle::SetUpSynParticle | synchronous.cpp setup |
| 20 | SynParticle::NormalizeLineEndings | text.cpp normalize_line_endings |
| 20 | SynParticle::GetField | synchronous.cpp get_field |
| 20 | SynParticle::Update | synchronous.cpp push |
| 20 | RFQ::GetSynField | synchronous.cpp rfq_syn_field / rfq_pass / scan_rfq（默认模式，M9-notes） |
| 20 | TcodeTrans | tracker.cpp trans |
| 20 | monotonicSearch | spacecharge.cpp import_scf |
| 20 | findClosest | spacecharge.cpp import_scf |
| 30 | `Bend::GetField(double, Particle&, FieldMatrix const&)` 0x125c80 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Bend::GetField(double, SynParticle&, FieldMatrix const&)` 0x125df0 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Element::GetField(double, SynParticle&, …)` 0x126950 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Element::GetField(double, Particle&, …)` 0x126970 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Element::getRadius(double)` 0x1269b0 | tracker.cpp aperture_lost |
| 30 | `Drift::GetField(double, Particle&, …)` 0x127080 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Drift::GetField(double, SynParticle&, …)` 0x127340 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Edge::GetField(double, Particle&, …)` 0x127600 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Edge::GetField(double, SynParticle&, …)` 0x127770 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Field::GetField(double, Particle&, …)` 0x12cb30 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Field::GetField(double, SynParticle&, …)` 0x1305d0 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Bessi0` 0x14a870 | kernels.h rfq_bessel_i（n = 0） |
| 30 | `Bessi1` 0x14a960 | kernels.h rfq_bessel_i（n = 1） |
| 30 | `Bessin` 0x14aa70 | kernels.h rfq_bessel_i（20 项级数） |
| 30 | `Lattice::GetExternalField(double, Beam&)` 0x188180 | tracker.cpp external_field / synchronous.cpp |
| 30 | `Lattice::GetExternalField(double, Particle&)` 0x1889e0 | tracker.cpp external_field / synchronous.cpp |
| 30 | `Lattice::SetMatrix(string, int, FieldMatrix&)` 0x188bb0 | fieldmap.cpp |
| 30 | `MethodMP::HardEdgeTrans … _omp_fn.1` 0x1940f0 | tracker.cpp het / sync_het（physics.h 各 *_row） |
| 30 | `… _omp_fn.2` 0x194e10 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.3` 0x195b40 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.4` 0x1967f0 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.5` 0x1972d0 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.6` 0x197e00 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.7` 0x1988a0 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.8` 0x1993a0 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.9` 0x199ee0 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.10` 0x19aa20 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.11` 0x19b580 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.12` 0x19c030 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.13` 0x19cad0 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `… _omp_fn.14` 0x19d160 | tracker.cpp het（quad / solenoid / bend / edge 的逐粒子 *_row，physics.h） |
| 30 | `MethodMP::HardEdgeTrans(int, rows&, Beam&, Lattice&)` 0x1a0ad0 | tracker.cpp het / sync_het（physics.h 各 *_row） |
| 30 | `MethodMP::HardEdgeTrans(int, double, double, rows&, Beam&, Lattice&, int&, double&, ErrorSample&)` 0x1aa5c0 | tracker.cpp het / sync_het（physics.h 各 *_row） |
| 30 | `Quadrupole::GetField(double, Particle&, …)` 0x1d02a0 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Quadrupole::GetField(double, SynParticle&, …)` 0x1d05c0 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `RFQ::getEleNum(double)` 0x1d0ad0 | kernels.h rfq_field_t（cell 定位）、synchronous.cpp rfq_cell |
| 30 | `RFQ::getRadius(double)` 0x1d0bb0 | kernels.h rfq_field_t（cell 孔径，超出 cell 用元件孔径） |
| 30 | `RFQ::GetField(double, Particle&, …)` 0x1d0be0 | kernels.h rfq_field_t（E = −∇U、完整 Bessel 因子，修正见 M9-notes） |
| 30 | `RFQ::GetField(double, SynParticle&, …)` 0x1d29a0 | synchronous.cpp rfq_pass（逐 cell 推进，入口相位 ±90°） |
| 30 | `Solenoid::GetField(double, Particle&, …)` 0x1d3540 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Solenoid::GetField(double, SynParticle&, …)` 0x1d3890 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Steerer::GetField(double, Particle&, …)` 0x1d3d50 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 30 | `Steerer::GetField(double, SynParticle&, …)` 0x1d3db0 | tracker.cpp element_field / superpose_field；synchronous.cpp get_field（同步粒子版） |
| 40 | Geometry::Boundary (0x147e20, SynParticle 重载) | tracker.cpp geometry_hit |
| 40 | Geometry::Boundary (0x147e50, Particle 重载) | tracker.cpp geometry_hit |
| 40 | LeapFrog::Update(double, Beam&) | physics.h boris_push（tracker.cpp trans 调用） |
| 40 | LeapFrog::Update(double, Particle&) | physics.h boris_push（tracker.cpp trans 调用） |
| 40 | MethodMP::tcodeRun (0x19d7f0, `._omp_fn.0`) | tracker.cpp tcode |
| 40 | MethodMP::zTotcode (0x19df10, `._omp_fn.17`) | tracker.cpp ztot / physics.h ztot_row |
| 40 | MethodMP::UpdateBeamParameters | tracker.cpp update_beam_parameters |
| 40 | MethodMP::synPout | tracker.cpp syn_out |
| 40 | MethodMP::testOut | 不需要：无调用者（Run 仍截断 test.txt、testP.txt） |
| 40 | MethodMP::simulationProgress | tracker.cpp progress |
| 40 | MethodMP::beamOut | tracker.cpp beam_out |
| 40 | MethodMP::errorOut | engine.cpp error_out |
| 40 | MethodMP::exOutput | tracker.cpp ex_output |
| 40 | MethodMP::LongitudinalBunchLoss | tracker.cpp longitudinal_loss |
| 40 | MethodMP::singlePOut | tracker.cpp single_out |
| 40 | MethodMP::FullParticleOutput | tracker.cpp full_particle_out |
| 40 | MethodMP::SingleTrack | tracker.cpp single_track |
| 40 | MethodMP::dstOut | tracker.cpp dst_out |
| 40 | MethodMP::zTotcode (0x1abb50) | tracker.cpp ztot / physics.h ztot_row |
| 40 | MethodMP::zcodeRun | tracker.cpp zcode |
| 40 | MethodMP::~MethodMP | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 40 | MethodMP::MethodMP | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 40 | MethodMP::tcodeRun (0x1afb20) | tracker.cpp tcode |
| 40 | MethodMP::Run | tracker.cpp run |
| 50 | `rand_gen` | distribution.cpp / rng.h（MsvcRand、DllGaussian、Philox） |
| 50 | `normalRandom` | distribution.cpp / rng.h（MsvcRand、DllGaussian、Philox） |
| 50 | `GenerateGaussian` | distribution.cpp / rng.h（MsvcRand、DllGaussian、Philox） |
| 50 | `Gaussu` | distribution.cpp / rng.h（MsvcRand、DllGaussian、Philox） |
| 50 | `GenerateCoordinates` | distribution.cpp dll_generate / default_generate |
| 50 | `KV2D` | distribution.cpp dll_generate / default_generate |
| 50 | `WB2D` | distribution.cpp dll_generate / default_generate |
| 50 | `PB2D` | distribution.cpp dll_generate / default_generate |
| 50 | `GS2D` | distribution.cpp dll_generate / default_generate |
| 50 | `Initial::InitParticle` | beam.cpp init_particles |
| 60 | Mesh::Mesh | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 60 | Mesh::~Mesh | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 60 | Mesh::MeshGenerator | spacecharge.cpp（网格合并在 SpaceCharge；Lattices2Node* / TextVol 为无调用者的旧 PICNIC 路径，不需要） |
| 60 | Mesh::Initial | spacecharge.cpp（网格合并在 SpaceCharge；Lattices2Node* / TextVol 为无调用者的旧 PICNIC 路径，不需要） |
| 60 | MethodMP::HESCFieldPre（00193a50，omp_fn.16） | spacecharge.cpp hesc_field_pre |
| 60 | MethodMP::HESCFieldPre（00193b80，omp_fn.15） | spacecharge.cpp hesc_field_pre |
| 60 | MethodMP::HESCFieldPre（001a20a0） | spacecharge.cpp hesc_field_pre |
| 60 | Calculate_S_PICNIC | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | Calculate_Weight | spacecharge.cpp weight |
| 60 | Calculate_Allocate | spacecharge.cpp allocate |
| 60 | PIC::FFT（001be410，omp_fn.8） | spacecharge.cpp fft（pocketfft） |
| 60 | PIC::Weight（001be8d0，omp_fn.0） | spacecharge.cpp weight |
| 60 | PIC::PICNIC（001bea90，omp_fn.7） | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | PIC::PICNIC（001bef60，omp_fn.6） | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | PIC::autoPIC | spacecharge.cpp solve |
| 60 | PIC::Lattices2NodeNew | spacecharge.cpp（网格合并在 SpaceCharge；Lattices2Node* / TextVol 为无调用者的旧 PICNIC 路径，不需要） |
| 60 | PIC::Lattices2NodeNewPhi | spacecharge.cpp（网格合并在 SpaceCharge；Lattices2Node* / TextVol 为无调用者的旧 PICNIC 路径，不需要） |
| 60 | PIC::Lattices2Node | spacecharge.cpp（网格合并在 SpaceCharge；Lattices2Node* / TextVol 为无调用者的旧 PICNIC 路径，不需要） |
| 60 | PIC::PICNIC（001c0710，omp_fn.5） | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | PIC::Lattices2NodePhi | spacecharge.cpp（网格合并在 SpaceCharge；Lattices2Node* / TextVol 为无调用者的旧 PICNIC 路径，不需要） |
| 60 | PIC::CLattices2Node | spacecharge.cpp（网格合并在 SpaceCharge；Lattices2Node* / TextVol 为无调用者的旧 PICNIC 路径，不需要） |
| 60 | PIC::CalculateVar | spacecharge.cpp（网格合并在 SpaceCharge；Lattices2Node* / TextVol 为无调用者的旧 PICNIC 路径，不需要） |
| 60 | PIC::TextVol | spacecharge.cpp（网格合并在 SpaceCharge；Lattices2Node* / TextVol 为无调用者的旧 PICNIC 路径，不需要） |
| 60 | Integrate_3D_Phi（isra.156） | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | Calculate_Tabulation | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | PIC::FFT（001c1c00） | spacecharge.cpp fft（pocketfft） |
| 60 | Integrate_3D_Vec（isra.227） | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | PIC::Tabulation（001c2d00，omp_fn.2） | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | PIC::Tabulation（001c3190） | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | PIC::TabulationPhi（001c31f0，omp_fn.3） | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | PIC::TabulationPhi（001c3720） | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | PIC::recordSCF | spacecharge.cpp record |
| 60 | PIC::importSCF | spacecharge.cpp import_scf |
| 60 | PIC::~PIC | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 60 | PIC::PIC()（001c5a70） | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 60 | PIC::Initial | spacecharge.cpp SpaceCharge 构造 |
| 60 | PIC::PIC(Mesh&,Beam&,vector&,int)（001c7980） | spacecharge.cpp SpaceCharge 构造 |
| 60 | PIC::Weight（001c8020） | spacecharge.cpp weight |
| 60 | PIC::PICNIC（001c9750） | spacecharge.cpp spicnic / tabulate（旧路径函数无调用者） |
| 60 | PIC::Allocate | spacecharge.cpp allocate |
| 70 | ErrorParameter::~ErrorParameter | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 70 | ErrorSample::~ErrorSample | 构造 / 析构：由 C++ 值语义（vector、默认成员）代替 |
| 70 | Bend::GetErrField | tracker.cpp element_err_field / superpose_err_field_dll（其余元件同 GetField） |
| 70 | Bend::SetErrPara | errorset.cpp set_errors_in_elements |
| 70 | Element::GetErrField | tracker.cpp element_err_field / superpose_err_field_dll（其余元件同 GetField） |
| 70 | Element::SetErrPara | errorset.cpp set_errors_in_elements |
| 70 | Drift::GetErrField | tracker.cpp element_err_field / superpose_err_field_dll（其余元件同 GetField） |
| 70 | Drift::SetErrPara | errorset.cpp set_errors_in_elements |
| 70 | Edge::GetErrField | tracker.cpp element_err_field / superpose_err_field_dll（其余元件同 GetField） |
| 70 | Edge::SetErrPara | errorset.cpp set_errors_in_elements |
| 70 | Field::GetErrField | tracker.cpp element_err_field / superpose_err_field_dll（其余元件同 GetField） |
| 70 | Field::SetErrPara | errorset.cpp set_errors_in_elements |
| 70 | GenerateErrorSet | errorset.cpp generate_error_set |
| 70 | Lattice::GetErrExternalField | tracker.cpp external_field + setup_errors |
| 70 | Lattice::SetErrInEle | errorset.cpp set_errors_in_elements |
| 70 | SeTrans | 不需要：空壳函数 |
| 70 | SeTcode | 不需要：空壳函数 |
| 70 | SeZcode | 不需要：空壳函数 |
| 70 | MethodMP::errSampleOut | errorset.cpp write_error_sample |
| 70 | MethodMP::SePlaneOut | tracker.cpp se_plane_out |
| 70 | MethodMP::SeSynPout | tracker.cpp se_syn_out |
| 70 | MethodMP::SeTRun | tracker.cpp se_trun |
| 70 | MethodMP::SeHardEdgeTrans | tracker.cpp het / sync_het（physics.h 各 *_row） |
| 70 | MethodMP::SeZRun | tracker.cpp se_zrun |
| 70 | MethodMP::SeRun | tracker.cpp run_secondary |
| 70 | Quadrupole::GetErrField | tracker.cpp element_err_field / superpose_err_field_dll（其余元件同 GetField） |
| 70 | Quadrupole::SetErrPara | errorset.cpp set_errors_in_elements |
| 70 | Solenoid::GetErrField | tracker.cpp element_err_field / superpose_err_field_dll（其余元件同 GetField） |
| 70 | Solenoid::SetErrPara | errorset.cpp set_errors_in_elements |
| 70 | Steerer::GetErrField | tracker.cpp element_err_field / superpose_err_field_dll（其余元件同 GetField） |
| 80 | sampling @0x180058620 | tools.cpp tool_sampling |
| 80 | dataprocess @0x180060e60 | tools.cpp tool_dataprocess |
| 80 | meshing @0x180061460 | tools.cpp tool_meshing |
| 80 | FUN_180055e20 | tools.cpp tool_dataprocess（线程任务） |
| 80 | FUN_180051fb0 | tools.cpp process_index |
| 80 | FUN_180051a60 | tools.cpp（内联：`_findfirst/_findnext` 列目录下指定扩） |
| 80 | FUN_180051d40 | tools.cpp（内联：按 z 值选择排序文件名表） |
| 80 | FUN_180063900 | tools.cpp（内联：ThreadPool::enqueue 的实例化（绑定 st） |
| 80 | FUN_1800636f0 | tools.cpp（内联：std::string + char） |
| 80 | FUN_180064450 | tools.cpp（内联：std::string 移动赋值包装） |
| 80 | FUN_18001dc70 | tools.cpp（内联：ofstream::open(name, mode, pro） |
| 80 | FUN_1800637d0 | tools.cpp（内联：以 double 键对"double 形式的下标数组"排序（） |
| 80 | FUN_180064530 | tools.cpp（内联：MSVC introsort 主体（double 下标数组版） |
| 80 | FUN_1800652e0 | tools.cpp（内联：introsort 分区（三数取中）） |
| 80 | FUN_180065ca0 | tools.cpp（内联：introsort 堆调整） |
| 80 | FUN_180066150 | tools.cpp（内联：三数取中比较交换） |
| 80 | FUN_180063880 | tools.cpp（内联：以 double 键对 int 下标数组排序（偏移内求秩）） |
| 80 | FUN_1800638a0 | tools.cpp（内联：std::find（int 数组），求秩位置） |
| 80 | FUN_1800638d0 | tools.cpp（内联：在 double 形式下标数组中查找 int 值，求全局秩位） |
| 80 | FUN_180063800 | tools.cpp（内联：int → double 再 floor（段大小 N/S）） |
| 80 | FUN_180063850 | tools.cpp（内联：round((double)int)） |
| 80 | FUN_180063860 | tools.cpp（内联：(double)n 的立方（MSE 分母）） |
| 80 | FUN_1800637f0 | tools.cpp（内联：pow((double)int, double)（最终权重）） |
| 80 | FUN_18004b4e0 | tools.cpp（内联：迭代器拷贝（返回指针）） |
| 80 | FUN_1800642f0 | tools.cpp（内联：vector::end()） |
| 80 | FUN_1800643a0 | tools.cpp（内联：vector<int>(n, v) 构造） |
| 80 | FUN_18000f4c0 | tools.cpp（内联：vector<int>::push_back（扩容路径）） |
| 80 | FUN_180064230 | tools.cpp（内联：vector<vector<vector<double>>>） |
| 80 | FUN_180064ba0 | tools.cpp（内联：vector<vector<int>>(N, v)（每粒子格） |
| 80 | FUN_180064c40 | tools.cpp（内联：vector<vector<vector<double>>>） |
| 80 | FUN_180064ce0 | tools.cpp（内联：vector<int>(n, v) 构造） |
| 80 | FUN_180064a30 | tools.cpp（内联：6 层嵌套 vector 构造（n⁶ 网格）） |
| 80 | FUN_180064d80 | tools.cpp（内联：6 层嵌套 vector 析构） |
| 80 | FUN_180064e50 | tools.cpp（内联：5 层嵌套 vector 析构） |
| 80 | FUN_1800663c0 | tools.cpp（内联：5 层嵌套 vector 拷贝构造） |
