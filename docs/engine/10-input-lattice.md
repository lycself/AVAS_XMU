# 10 输入解析与 lattice 初始化（子系统 A）

依据：`decomp/libAVAS/functions/*.c`（Ghidra 伪代码）、对同一个 `libAVAS.so` 直接反汇编（`llvm-objdump`，用来找回 Ghidra
丢掉的字符串字面量、比较分支和寄存器去向）、`.rodata` 常量与 `AVAS.dll` 伪代码（核对差异）。下文的 `word[k]` 指一行按
`StringSplit` 拆出的第 k 个单词（`word[0]` 是关键字）。偏移、字段名沿用 [structures.md](structures.md)。
“推测”表示伪代码/反汇编不足以定论；“确定”的地方都至少有一处直接证据（给出函数名或地址）。

---

## 1. 概述

子系统 A 负责把三个文本输入（`input.txt`、`beam.txt`、`lattice.txt`）以及可选的 `boundary.txt`、RFQ 结构文件读成内核对象：

- `Initial`（参数表）：`Initial::Initial` 给默认值，`Initial::Read` 读 input.txt，`Initial::ReadBeam` 读 beam.txt。
- `Lattice`（元件表 + 场图库 + 输出面 + superposeout 变换）、`ErrorParameter`（误差定义的原始数值）、`Boundary`（几何段）：
  全部由 `Initial::InitLattice` 一次读 lattice.txt（和 boundary.txt）建立；元件对象由 `Lattice::SetElement` 累加 z 后调用各元件的
  `SetParameter`。
- 文本预处理：`Initial::NormalizeLineEndings`（整文件读入、统一换行）、`StringSplit`（空白拆词、`!` 注释）。

整个运行的入口和顺序（`main_agent`，libAVAS.so，确定）：

```
Python: MultiParticleEngine.get_path(input, output, field)  -> path(input, output, field)
        MultiParticleEngine.main_agent(1)                     -> main_agent(int*)   （参数被忽略，见 2.1.2）
main_agent:
  print "The programme is running."
  Initial::Initial; Initial::Read                               (input.txt)          ┐
  ErrorParameter / Lattice / Boundary 构造                                          │ 初始化阶段：
  if singleParticleMode==1: InitLattice; singlePartM; "Simulation ends."; return 0 │ std::string 异常
  InitLattice (lattice.txt [+boundary.txt])                                         │ → ErrorLog 记
  GenerateErrorSet                                                                  │ "Initialisation
  if 有误差开关: 误差循环（每组每次重建输出目录 output_%d/ 并 InitParticle…Run）       │  stage error"，返回 1
  else: InitParticle (→ReadBeam, beam.txt); SetSynPhase(γβz_sync)                   ┘
        print "Initialization time: <s> s"
        MethodMP::MethodMP; Initial::InitMethodMP
        secondaryBeam==0 ? MethodMP::Run : MethodMP::SeRun                          运行阶段：返回 2
        print "Simulated total time: <s> s"
  return 0
```

本文只为 A 的 55 个函数写规格；`InitParticle`（E）、`SetSynPhase`/`singlePartM`（B）、`Lattice::SetMatrix`（C，场图读取）、
`GenerateErrorSet`/`SetErrPara`（G）、`MethodMP::*`（D）只描述到“A 给它们交了什么数据”为止。

---

## 2. 规格正文

### 2.1 入口、路径与错误处理

#### 2.1.1 `path(const char* input, const char* output, const char* fieldMap)`（确定）

- 参数顺序是 **(input, output, fieldMap)**；屏幕打印顺序却是 input、fieldMap、output：
  `"The input path has been set to " + input`、`"The fieldMap path has been set to " + fieldMap`、
  `"The output path has been set to " + output`，各接一个换行并 flush。
- 三个全局字符串 `inputFilePath / fieldMapPath / outputFilePath` 分别赋为 `参数 + "/"`（无条件追加一个 `/`，不检查原来是否已以
  `/` 或 `\` 结尾，也不检查目录是否存在）。永远返回 0。
- 全局变量的静态初值（`.init_array` 中的构造，0x20f03 起）：`inputFilePath = "./inputFile/"`、`fieldMapPath = "./inputFile/"`、
  `outputFilePath = "./outputFile/"`。不调用 `path()` 时按进程当前目录解析。
- DLL 版（`180055ee0 path`）参数是宽字符串（Python 用 `c_wchar_p`），转换方式是**逐字节复制所有非零字节，遇到连续两个 0 字节停止**，
  也就是把 UTF-16LE 的高字节 0 丢掉；非 ASCII 字符（例如中文目录名）会被拆成两个任意字节，路径必然错误。DLL 同样追加 `"/"`。

#### 2.1.2 `main_agent`（确定）

- libAVAS.so 的 `main_agent` 一开头就覆盖 `%rdi`，**完全不读调用者传入的 int***；内部用一个局部 int 作“状态标志”：初始化前置 1，
  `SetSynPhase` 之后置 2，再把它的地址传给 `MethodMP::Run(..., int*)`。因此 Python 的 `main_agent(2)`（“停止”）在 .so 上实际是
  **再启动一次完整运行**。
- DLL 版把传入指针当状态标志：同样不读初值，初始化后写 `*param = 2` 并把该指针传给 Run（Run 中可能据此响应“用户终止”，
  rodata 有 `"Programs terminated by users."`，属 D）。Python 每次调用都传一个新的临时 `c_int`，所以从 Python 侧无法借此停止。
- 返回值：正常 0；**catch(std::string)** 的初始化阶段异常 → `errorOut(msg, 1)` 后返回 1；运行阶段 → `errorOut(msg, 2)` 后返回 2
  （反汇编 0x240d4、0x2416d：选择子 ==1 时 `__cxa_get_exception_ptr` + `std::string` 拷贝构造 + `errorOut`，返回值写 44(%rsp)）。
  Ghidra 伪代码里只看到 `return 0`，这是反编译遗漏。
- **其他类型的异常不被捕获**：`std::stoi/std::stod` 的 `std::invalid_argument("stoi"/"stod")`、`std::out_of_range`
  （包括 `vector::_M_range_check`）、`std::runtime_error` 会穿出 `main_agent` → `std::terminate` → 进程 abort，ErrorLog.txt 不会写。
  元件参数个数不足时部分路径直接 `exit(1)`（见 2.6）。新内核若要“一致”，必须区分这三类结局：返回 1/2 + ErrorLog、abort、exit(1)。
- 误差循环（当 `ErrorParameter.flags[0..3]` 之和非 0）：组数 × 每组次数 = `err_step` 的 a×b（`ErrorParameter+0x00` 默认 {1,1}，
  由 main_agent 构造时写入）；第 i 次（i 从 0 起）创建目录 `outputPath + "output_" + "%d"(i)`（`access(...)==-1` 时 `mkdir(path, 0775)`，
  失败打印 `"Error output folder creation failure"` 并 `exit(1)`），并把全局 `outputFilePath` 改为该目录加 `"/"`。细节属 G/D。

#### 2.1.3 `errorOut(std::string msg, int stage)` 与 `Initial::errorOut(std::string)`（确定）

- 先把 `msg` 打到 stdout 并换行。
- 以 `ios::out | ios::app`（openmode 0x11）打开 `outputFilePath + "ErrorLog.txt"`（追加，不截断）。
- 用 `time()` + `localtime()` + `strftime(buf, 64, "%Y/%m/%d %X  ", tm)` 生成时间戳（`%X` 在 C locale 下是 `HH:MM:SS`，后跟两个空格）。
- stage==1 写 `时间戳 + "   Initialisation stage error: " + msg + "\n"`；stage==2 写 `时间戳 + "   simulation stage error: " + msg + "\n"`；
  其他 stage 什么也不写。关闭文件，返回（不退出）。
- Python `check_error_file` 按 5 个空格（时间戳末尾 2 个 + 前缀开头 3 个）切分取消息正文，依赖这一格式。
- `Initial::errorOut` 写法相同但固定写 “Initialisation stage error”，写完 `exit(1)`。全库没有调用者（死代码）。

#### 2.1.4 Python 侧（`avas/core/MultiParticleEngine.py`、`MultiParticle.py`，仓库参考）

- `ctypes.CDLL` 载入；`path.argtypes = [c_char_p×3]`（Linux，UTF-8 字节串）/`[c_wchar_p×3]`（Windows）；`main_agent.argtypes = [POINTER(c_int)]`。
- `MultiParticle.run`：先删 `output/ErrorLog.txt`，`get_path(input_file, output_file, field_path)`（`field_path` 缺省 = `input_file`），
  `main_agent(1)`；返回 1 或 2 时读 ErrorLog。
- `avas run` 把文本输入复制到 `<输出>/inputs/`，并由 `avas/utils/tolattice.py` 只保留元件、`start/end`、superpose/输出类关键字生成
  **固定文件名** `lattice.txt`（内核只认 `inputFilePath + "lattice.txt"`）。

内核读写的文件（A 部分，全部用全局路径前缀直接拼接）：

| 文件 | 路径 | 读/写 | 函数 |
|---|---|---|---|
| input.txt / beam.txt / lattice.txt | inputFilePath + 名 | 读 | Read / ReadBeam / InitLattice |
| boundary.txt | inputFilePath | 读（input.txt `boundary 1` 时） | InitLattice |
| rfqinputAnn.txt、rfqinputV.txt | inputFilePath | 读（lattice 有 `rfq` 行时） | ReadRFQ |
| rfqinput.txt、E0-z.txt | **inputFilePath** | 写（截断） | ReadRFQ |
| `<名>.bsx/.bsy/.bsz` | fieldMapPath + word[9] | 写（`multipole` 行） | MultipoleFieldMapGenerator |
| 场图分量文件 | fieldMapPath + word[9] + 后缀 | 读 | Lattice::SetMatrix（C） |
| ErrorLog.txt | outputFilePath | 追加 | errorOut |

### 2.2 文本预处理（三个文本文件共用）

#### 2.2.1 `Initial::NormalizeLineEndings(std::ifstream&)`（确定）

- 文件未打开返回空串。否则用 `istreambuf_iterator` 把**整个文件**读成一个字符串（字节流，不做编码转换，不跳过 BOM）。
- 先把所有 `"\r\n"` 替换为 `"\n"`（从位置 0 起 `find`，替换后从 `pos+1` 继续），再把所有剩余的 `"\r"` 替换为 `"\n"`。
- 调用方把结果装进 `std::istringstream`，循环 `while (peek() != EOF) { getline(ss, line, '\n'); StringSplit(line, words); if (words 为空) continue; ... }`。
  因此 CRLF、CR、LF 三种换行等价；行尾没有换行的最后一行同样处理。
- 注意：UTF-8 BOM（EF BB BF）会粘在第一行第一个单词前，使该行关键字无法识别（推测会落入“未知关键字”分支；未实验）。

#### 2.2.2 `StringSplit(const std::string& s, std::vector<std::string>& out)`（确定，反汇编 0x538b0 核对）

```
out.clear()
n = s.size() (按 uint 处理); start = 0
for i in 0..n-1:
    c = s[i]
    if c == '!':  break                      # 注意：不推入 start..i 之间未完成的词
    if c == ' ' or c == '\t':
        if start == i: start = i+1           # 连续空白
        else: out.push(s.substr(start, i-start)); start = i+1
    elif i == n-1:
        out.push(s.substr(start, n-start))   # 行末最后一个词
```

- 只有空格和制表符是分隔符；`,`、`;`、全角空格、`\v`、`\f` 都是词的一部分。
- **`!` 紧贴在数值后面会吞掉这个数值**：`particlenumber 1000!注释` 只得到 `["particlenumber"]`，而 `particlenumber 1000 !注释` 正常。
  `!` 在行首或空白之后则正常起注释作用。
- 没有 `#`、`//` 注释；空行和只含空白的行得到空列表，被调用方跳过。

#### 2.2.3 关键字大小写（确定，反汇编 0x75300–0x753af）

`Read` 与 `ReadBeam` 对每一行执行一个 `for (i = 0; i < words.size(); ++i) transform(words[0] → tolower)` 的循环——循环变量没用上，
**只把 `word[0]`（关键字）转小写**（重复 n 次，结果相同）。参数值保持原样：`scmethod fft` 会报错（必须写 `FFT`/`SPICNIC`），
`distribution gs gs` 会报错（必须大写），`kindofparticle`、`readparticledistribution`、`method` 的值区分大小写。
`InitLattice` 和 `boundary.txt` 解析**不做任何大小写转换**：lattice 关键字必须小写（`Drift` 不被识别，按未知行忽略）。

#### 2.2.4 数值解析语义（确定）

- “stoi” = `__gnu_cxx::__stoa<long,int>` 或内联的 `strtol(s, &end, 10)`：允许前导空白和符号，从第一个非数字处停止并忽略后缀
  （`"3.7"`→3，`"1e3"`→1，`"0.0"`→0）；无任何数字时抛 `std::invalid_argument("stoi")`；`errno==ERANGE` 或超出 int 范围抛
  `std::out_of_range("stoi")`。
- “stod” = `strtod(s, &end)`（C locale；接受 `inf`、`nan`、十六进制浮点、尾随垃圾）；无转换抛 `invalid_argument("stod")`；
  `ERANGE`（上溢，以及 glibc 下的下溢）抛 `out_of_range("stod")`。
- 这些异常都不是 `std::string`，按 2.1.2 会使进程 abort。
- 访问不存在的单词：用 `operator[]` 的地方是越界读（未定义行为，通常崩溃）；用 `.at()` 的地方抛 `out_of_range("vector::_M_range_check")`。
  下文对每个关键字注明是哪一种。

### 2.3 `Initial::Initial` 默认值（确定，常量取自 .rodata）

| 偏移 | 字段 | 默认值 |
|---|---|---|
| 0x000 / 0x004 | secondaryBeam / singleParticleMode | 0 / 0 |
| 0x008 | distFile | `"unknown"` |
| 0x010 / 0x018 | charge / atomicMass | −1.0 / −1.0 |
| 0x020 | distribution | `{"GS","GS"}` |
| 0x038 / 0x03c | particleNumber / （未知 int） | −1 / −1 |
| 0x040 | twiss[9] | 全部 −2027.041（“未设置”哨兵，0xc09fac29fbe76c8b） |
| 0x058 / 0x070 | displacePos[3] / displacedPos[3] | 0 |
| 0x088 | cores[10]（int） | {−1, 1,1,1,1,1,1,1,1,1} |
| 0x0a0 | restMass | 938.272029 |
| 0x0a8 / 0x0b0 / 0x0b8 | current / frequency / kineticEnergy | −1 / −1 / −1 |
| 0x0c0 | （未知 double） | 0 |
| 0x0c8 | randomSeed | 0.0 |
| 0x0d0 | longLimits[5] | 0 |
| 0x0e8 | kind | `"proton"` |
| 0x0f0 | beamLength[2] | 0 |
| 0x108 | method | `"MP"` |
| 0x110 | spaceCharge | 1 |
| 0x118 | scPar[20] | {1, −1,−1,−1,−1,−1,−1,−1, 0.09, 1,1,1,1,1,1,1,1,1,1,1} |
| 0x130 | （未知 int） | 1 |
| 0x138 | scanTStep | 0.03 |
| 0x140 | （未知 int） | 3 |
| 0x144–0x154 | boundary, lpBoundary, probe, indWeight, indCharge | 0 |
| 0x158 | pcHistogram[6] | 0 |
| 0x170 | stepPerCycle | 50.0 |
| 0x178 / 0x17c / 0x180 | dumpPeriodicity / （未知） / runPeriod | 0 / −1 / 100000 |
| 0x188 | track | 空 vector<int> |
| 0x1a0 | boundary 用 superposeout 计数（见 2.9） | 0 |
| 0x1a8 | outputPlanes | 空 |
| 0x1c0 / 0x1c4 | needSynPhase / scSolverRecord | 0 / 0 |
| 0x1c8 | numOfGrid[3] | {−1,−1,−1} |
| 0x1e0 | （未知） | 0 |
| 0x1e4 | bunchNumber | **未初始化**（构造函数不写） |
| 0x1e8 / 0x1ec / 0x1f0 | multithreading / maxThreads / ompMaxThreads | 1 / −1 / `omp_get_max_threads()` |
| 0x1f8 | （未知 vector） | 空 |
| 0x210 | seStep | 0.01 |
| 0x218 / 0x21c | recScField / impScField | 0 / 0 |

### 2.4 input.txt：`Initial::Read`（确定）

流程：打开 `inputFilePath + "input.txt"`（`ios::in`），失败抛 std::string
`File "input.txt" cannot be opened, Check the inputFile routing and input.txt.`；按 2.2 逐行处理；`word[0]` 小写后按下表**依次比较**
（第一个相等者生效）。全部行读完后：若 `secondaryBeam == 1` 则 `spaceCharge = 0`；打印 `"Parameters and functions are set"` 换行、
`"  "` 换行。

| 比较顺序 | 关键字 | 解析与存储 | 检查/特殊 |
|---|---|---|---|
| 1 | `method` | `method = word[1]`（原样字符串） | **赋值后不跳回**，继续与后面的关键字比较，最后落到“未知关键字”分支打印警告（见下） |
| 2 | `secondarybeam` | `secondaryBeam = stoi(word[1])` | |
| 2b | `boundary` | `boundary = stoi(word[1])` | 反汇编比较链中紧跟 `secondarybeam`（0x753f4） |
| 3 | `probe` | `probe = stoi(word[1])` | |
| 4 | `dimension` | — | 抛 std::string `input.txt Contains unknown keywords: Dimension` |
| 5 | `scmethod` | `word[1]=="FFT"` → `scPar[0]=0.0`；`=="SPICNIC"` → `scPar[0]=1.0` | 其他（含 `PICNIC`、小写）抛 `SCMethod Contains unknown item, available options include FFT / SPICNIC` |
| 6 | `negligibleratio` | `scPar[8] = stod(word[1])` | |
| 7 | `lpboundary` | `lpBoundary = stoi(word[1])` | |
| 8 | `indweight` | `indWeight = stoi(word[1])` | |
| 9 | `longlimits` | 当单词数 ≥ 2：对 k=1..n−1，`longLimits[k−1] = stod(word[k])` | 不检查上限：多于 5 个值写出堆外（未定义） |
| 10 | `indcharge` | `indCharge = stoi(word[1])` | |
| 11 | `meshrms` | 当 n ≠ 1：k=1..n−1，`scPar[k] = stod(word[k])` | 3 个值写 scPar[1..3]；更多值会覆盖 integralgrid 等 |
| 12 | `integralgrid` | 当 n ≠ 1：k=1..n−1，`scPar[3+k] = stod(word[k])` | 写 scPar[4..] |
| 13 | `scutoffdistance` | `scPar[7] = (double)stoi(word[1])` | **按整数解析**：`1.5` → 1.0 |
| 14 | `scantstep` | `scanTStep = stod(word[1])` | |
| 15 | `dumpperiodicity` | `dumpPeriodicity = (int)stod(word[1])` | 截断取整 |
| 16 | `spacecharge` | `spaceCharge = stoi(word[1])` | 为 0 时打印 `"Simulation ignores space charge effects"` + endl |
| 17 | `singleparticlemode` | `singleParticleMode = stoi(word[1])` | |
| 18 | `scsolverrecord` | `scSolverRecord = stoi(word[1])` | |
| 19 | `pchistogram` | n ≥ 2 时 k=1..n−1：`pcHistogram[k−1] = (double)stoi(word[k])` | 整数解析；最多 6 个 |
| 20 | `numofgrid` | n ≠ 1 时 k=1..n−1：`numOfGrid[k−1] = stoi(word[k])` | 最多 3 个 |
| 21 | `steppercycle` | `stepPerCycle = stod(word[1])`；n ≥ 3 时 `seStep = stod(word[2])` | V1 是 double（schema 写 INT） |
| 22 | `runperiod` | `runPeriod = stoi(word[1])` | |
| 23 | `track` | n ≥ 2 时 k=1..n−1：`track.push_back(stoi(word[k]))` | 多行 track 会累加 |
| 24 | `randomseed` | `randomSeed = (double)stoi(word[1])` | input.txt 里按整数解析 |
| 25 | `multithreading` | `multithreading = stoi(word[1])` | 不检查取值（AGENTS 第 6 节：0/2/3 会在起点丢全部粒子，属 D） |
| 26 | `setnumberofcores` | n < 11 抛 `keyword: setnumberofcores, Insufficient parameters, need 10 parameters`；k=1..n−1：`cores[k−1] = stoi(word[k])`；然后对 i=0..9：若 `cores[i] > omp_get_max_threads()` 则改为后者；`scPar[10+i] = (double)cores[i]` | 多于 10 个值写出堆外 |
| 27 | `maxthreads` | `maxThreads = stoi(word[1])` | |
| — | 其他 | 打印 `"input.txt Contains unknown keywords, check input.txt"` + endl，忽略 | 只是警告 |

单词不足时：所有 `word[1]` 访问都是 `operator[]`（越界读，未定义）。

**与 schema.py（`_INPUT`）对照**

| 类别 | 关键字 |
|---|---|
| 两边都有 | multithreading, steppercycle, spacecharge, scmethod（schema 多列 `PICNIC`，内核会报错）, numofgrid, meshrms, dumpperiodicity, longlimits, boundary, pchistogram, secondarybeam, randomseed |
| schema 有、内核不读（打印“unknown keywords”警告后忽略） | `sim_type`, **`scanphase`**, `spacechargelong`, `spacechargetype` |
| 内核识别、schema 没有 | `method`（且会额外触发警告）, `probe`, `dimension`（总是报错）, `negligibleratio`, `lpboundary`, `indweight`, `indcharge`, `integralgrid`, `scutoffdistance`, `scantstep`, `singleparticlemode`, `scsolverrecord`, `runperiod`, `track`, `setnumberofcores`, `maxthreads` |

`scanphase` 与 `scanData.txt`：.so 与 DLL 的 rodata 中都没有 `"scanphase"`、`"scanData"` 字符串（只有输出文件名 `scanPhaseData.txt`），
相位是否扫描完全由 lattice 中 field 行的 V3 决定（见 2.6.4.7 的 `needSynPhase`）。GUI 写的 `scanphase` 只会让内核打印警告。

### 2.5 beam.txt：`Initial::ReadBeam`（确定）

由 `InitParticle`（多粒子）或 `singlePartM`（单粒子模式）调用，时间上在 `InitLattice` 之后。打开 `inputFilePath + "beam.txt"`，
失败抛 `File "beam.txt" cannot be opened, Check the inputFile routing and beam.txt.`。预处理与小写规则同 2.2。比较顺序与动作：

| 顺序 | 关键字 | 解析与存储 | 检查/特殊 |
|---|---|---|---|
| 1 | `readparticledistribution` | `distFile = word[1]` | 值区分大小写 |
| 2 | `numofcharge` | `charge = stod(word[1])` | |
| 3 | `particlerestmass` | `restMass = stod(word[1])` | MeV |
| 4 | `kindofparticle` | `kind = word[1]` | |
| 5 | `distribution` | 单词数必须**恰为 3**，否则抛 `The initial bunch distribution contains two parameters, separately in transvers and longitudinal`；对 k=0,1：先 `distribution.at(k) = word[k+1]`，再检查它 ∈ {`KV`,`WB`,`PB`,`GS`}（按此顺序比较，区分大小写），否则抛 `Distribution Type Error, The Format is: distribution  KV/WB/PB/GS  KV/WB/PB/GS` | |
| 6 | `beamtype` | 若 `word[1] == "dc"`：`twiss[6] = −2027.0415`（直流标记，0xc09fac2a7ef9db23） | 其他值静默忽略 |
| 7 | `twissx` | 单词数必须恰为 4，否则抛 `twissx have 3 parameters, emittance, beta, alpha`；`twiss[2]=stod(word[1])`，`twiss[1]=stod(word[2])`，`twiss[0]=stod(word[3])` | 文件顺序是 α β ε（报错文字写反了）；存储为 [ε, β, α] |
| 8 | `twissy` | 同上，`twiss[5]←word1, twiss[4]←word2, twiss[3]←word3` | 报错 `twissy have 3 parameters, emittance, beta, alpha` |
| 9 | `twissz` | 单词数必须恰为 4（报错 `twissz have 3 parameters, emittance, beta, alpha`）；**若当时 `twiss[6] == −2027.0415`（已遇到 `beamtype dc`）则只把 `twiss[6]` 改回 −2027.041、不读三个值**；否则 `twiss[8]←word1, twiss[7]←word2, twiss[6]←word3` | 与 `beamtype` 的先后有关：dc 在前 → twissz 被忽略且 dc 标记被清除；dc 在后 → εz 被 dc 标记覆盖 |
| 10 | `particlenumber` | `particleNumber = stoi(word[1])` | |
| 11 | `displacepos` | n ≠ 1 时 k=1..n−1：`displacePos[k−1] = stod(word[k]) * 0.001` | **mm→m**（与 AGENTS 第 6 节“按 mm 读取”一致）；多于 3 个值写出堆外 |
| 12 | `displacedpos` | 同上，`displacedPos[k−1] = stod(word[k]) * 0.001` | 同样乘 0.001（schema 写单位 %，内核按“千分之一”换算；含义属 B） |
| 13 | `frequency` | `frequency = stod(word[1])` | 内核按 **MHz**（InitParticle 中 ×1e6，schema 写 Hz） |
| 14 | `kneticenergy` | `kineticEnergy = stod(word[1])` | 拼写就是 kneticenergy |
| 15 | `beamlength` | `beamLength[0] = (double)stoi(word[1])`；`beamLength[1] = stod(word[2])` | 第一个值按整数解析 |
| 16 | `current` | `current = stod(word[1])` | mA（InitParticle 中 /1000） |
| 17 | `bunchnumber` | `bunchNumber = stoi(word[1])` | |
| 18 | `randomseed` | `randomSeed = stod(word[1])` | beam.txt 里按 double 解析；与 input.txt 同一字段，后读者覆盖（ReadBeam 在 Read 之后） |
| 19 | `recscfield` | `recScField = stoi(word[1])` | |
| 20 | `impscfield` | `impScField = stoi(word[1])` | |
| — | 其他 | 打印 `"beam.txt Contains unknown keywords, check input.txt"`（原文就是 input.txt）+ endl | 警告 |

ReadBeam 不检查必需参数是否给出（未给的保持 −1/−2027.041 哨兵，由 InitParticle 处理）。

**与 schema.py（`_BEAM`）对照**：schema 有、内核不读：`use_dst`、`initpos`、`initmom`（打印警告）。内核识别、schema 没有：
`beamlength`、`bunchnumber`、`recscfield`、`impscfield`。schema 的 twiss 参数顺序（α, β, ε）与内核一致；`frequency` 单位 schema 写 Hz、
内核按 MHz；`displacedpos` 单位 schema 写 %、内核乘 0.001。

### 2.6 lattice.txt：`Initial::InitLattice(Lattice&, ErrorParameter&, Boundary&)`（确定，另注）

调用约定：`this = Initial`，`param_2 = Lattice`，`param_3 = ErrorParameter`，第 4 参数（Ghidra 的 `in_RCX`）= `Boundary`。

#### 2.6.1 读取主循环与 start / end

1. 打开 `inputFilePath + "lattice.txt"`，失败抛 `File "lattice.txt" cannot be opened, Check the inputFile routing and lattice.txt.`；
   打印 `"Initialising lattice..."` 换行；整文件经 `NormalizeLineEndings` 后逐行读取。
2. 局部状态：`skip = true`（寄存器 r14d），全局 RF 腔计数 `cav = 0`（local_19f0），`autoRows`（automaticoutput 行，局部）。
3. 每读一行非空行：若 `skip` 为真且 `word[0] != "start"` → 丢弃该行；否则把该行送入关键字分派（`start` 行自身也走一遍分派，
   不匹配任何关键字，按“未识别”处理）。**每处理完一行（无论识别与否）都置 `skip = false`**（LAB_0017764f），所以第一个 `start`
   之前的行全部忽略，之后的行全部处理；再出现的 `start` 只是一行被忽略的未知行。
4. `end` 行触发收尾（2.6.6）并**结束读取**（跳到关闭文件），其后的行全部忽略。
5. 文件读完仍未遇到 `end`：直接关闭文件，**不做收尾**——输出面列表缺末端面、automaticoutput 不展开、不追加 spOut 哨兵、
   不写 `ErrorParameter+0xa8`、不打印 “Lattice initialisation complete…”。后续以 `outputPlanes.back()` 为进度分母，空列表时为未定义行为。
6. 读完 lattice（遇 `end` 或 EOF）后，若 `Initial.boundary == 1`，读 boundary.txt（2.9）。

#### 2.6.2 关键字分派顺序（反汇编比较链，确定）

`drift` → `automaticoutput` → `outputplane` → `superpose`（块，2.6.5）→ `multipole` → `quad` → `solenoid` → `steerer` → `bend`
→ `edge` → `rfq` → `field` → `err_beam_stat` → `err_beam_dyn` → `err_cav_ncpl_stat` → `err_cav_ncpl_dyn` → `err_quad_ncpl_stat`
→ `err_quad_ncpl_dyn` → `err_step` → `err_beam_stat_on` → `err_beam_dyn_on` → `err_quad_stat_on` → `err_quad_dyn_on`
→ `err_cav_stat_on` → `err_cav_dyn_on` → `end` → 其余：**静默忽略**（不打印任何东西）。

- 比较是区分大小写的精确比较。
- **内核（.so）不识别**、因而静默忽略的 schema 关键字：`superposeend`（只在 superpose 块里作为“块结束行”被消耗）、`lattice`、
  `lattice_end`、`spacechargecomp`、`adjust`、`diag_energy`、`diag_size`、`diag_position`、`err_quad_cpl_*`、`err_cav_cpl_*`。
- 内核识别而 schema 没有的：`multipole`、`rfq`。
- DLL 另外识别 `spacechargecomp`（2.13）。

#### 2.6.3 元件入表：`Lattice::SetElement(vector<string> words, Element* e)`（确定）

```
zStart = elemZStart.empty() ? 0.0 : elemZStart.back() + elemLength.back()
elemZStart.push(zStart)
L = stod(words[1]);  elemLength.push(L)
elemZEnd.push(elemZStart.back() + elemLength.back())          # = zStart + L
if words[0] == "field":
    t = stoi(words[4])
    if t == 4:                                                 # 叠加组
        m = words.size() / 20
        for off = 20, 40, ..., 20*(m-1):                       # 只处理 m-1 个子块
            if words[off] == "field":
                idx = SetMatrix(words[off+9], stoi(words[off+4]), fm); lastMapIndex = idx
                words[off+9] = sprintf("%d", idx)
    else:
        idx = SetMatrix(words[9], t, fm); lastMapIndex = idx; words[9] = sprintf("%d", idx)
e->SetParameter(index = (int)round(elements.size()), zStart = elemZStart.back(), words)   # 虚函数槽 +0x28
elements.push(e)
```

- z 只按 `word[1]` 累加，**不看元件类型**：`bend` 的 `word[1]`（schema 说“写 0 即可，程序按 α、ρ 计算”）若写 0，lattice 的 z 坐标
  里该二极铁长度就是 0（弧长 ρ|α| 只存进 Bend 对象，用法属 C/D）。
- `field` 行在交给 SetElement 前已由 InitLattice 改写（2.6.4.7）；`SetMatrix` 在 `Field::SetParameter` 的单词数检查**之前**执行，
  单词不足 10 的 field 行会先越界访问 `words[9]`。
- 元件序号 `index` = 入表前的元件数（从 0 起）。

#### 2.6.4 各元件

##### 2.6.4.1 公共部分 `Element::SetBasicParameter(double zStart, const vector<string>& w)`（确定）

`words = w`；`zStart(+0x48) = zStart`；若 w 非空：`typeName(+0x40) = w[0]`；若 `w.size() ≥ 2`：`length(+0x50) = stod(w[1])`；
然后要求 `w.size() ≥ 3`（否则 `out_of_range("vector::_M_range_check")`）：`radius(+0x28) = radius2(+0x30) = stod(w[2])`，
`apertureType(+0x38) = 0`。单位：L、R 都是 m。

各元件 `SetParameter(int index, double zStart, words)` 先做单词数检查，再写 `index(+0x20)`、调 `SetBasicParameter`，再读专有参数。
“打印并返回”指：打印 `"Error! parameter not enough in element : \n"`，再把所有单词以 `"\t"` 分隔打印并换行，然后 **return**——
元件仍然已入表（Lattice 的 z 已累加），但对象保持构造时的默认值（L=0、zStart=0、typeName="drift"）。

##### 2.6.4.2 `drift`（Drift，0x88）

InitLattice 内联构造（vtable Drift）。`Drift::SetParameter`：单词数 < 4 → 打印并返回；否则只调 `SetBasicParameter`。
schema：L, R, 保留。✓

##### 2.6.4.3 `quad`（Quadrupole，0xf8）

- 构造：`+0x88 = 0`，`+0x90 = −4e-05`，`+0x98 = 4.518219`，`+0xa0..+0xb8 = 0`（两个常数含义不明，未找到读取处）。
- SetParameter：单词数 < 4 → 抛 std::string `Error! parameter not enough in element : quad`（返回 1 + ErrorLog）；单词数 < 5 →
  `out_of_range`（abort）；`G(+0x58) = stod(word[4])`（T/m）；`aperDiam(+0xa0) = R + R`；`G == 0` → 抛
  `The gradient of the quadrupole cannot be set to 0, Drift can be used to replace quadrupole with a field gradient of 0`。
- GetParameter（追加到 vector<double>）：`[+0xc8 dx, +0xd0 dy, +0xd8 dz, +0xe0 rotX, +0xe8 rotY, +0xf0 rotZ, +0xc0 G·(1+dG)]`。

##### 2.6.4.4 `solenoid`（Solenoid，0xc0）

SetParameter：单词数 < 5 → 打印并返回；`B(+0x58) = stod(word[4])`（T）。GetParameter：`[+0x90..+0xb8（dx,dy,dz,rx,ry,rz）, +0x88]`。

##### 2.6.4.5 `steerer`（Steerer，0xa8）

SetParameter：单词数 < 7 → 打印并返回；`kind(+0x88) = (int)stod(word[6])`；`Bx/Ex(+0x90) = stod(word[4])`；
`By/Ey(+0x98) = stod(word[5])`；单词数 < 8 → `out_of_range`（abort）；`max(+0xa0) = stod(word[7])`。
（7 个单词的 steerer 行会通过第一道检查然后 abort。）GetParameter：`[(double)kind, Bx, By, max]`。
注意 kind 用 stod 再截断，`0.9` → 0。

##### 2.6.4.6 `bend`（Bend，0x108）与 `edge`（Edge，0xf8）

- Bend::SetParameter：单词数 < 5 → 打印并返回；单词数 < 8 → `out_of_range`；
  `hv(+0x88) = (int)stod(word[7])`；`α(+0x98) = stod(word[4]) / 180 · π`（先除 180 再乘 π）；`ρ(+0xb0) = stod(word[5])`；
  `n(+0xb8) = stod(word[6])`；然后 `arc(+0x90) = ρ·|α|`；`h(+0xc0) = (α/|α|)/ρ`（α=0 时为 NaN）；
  `kx(+0xa0) = sqrt((1−n)·h·h)`；`ky(+0xa8) = sqrt(n·h·h)`（运算顺序：`(1.0 − n) * h * h`、`n * h * h`；负数开方得 NaN）。
- Bend::GetParameter（输出 vector 非空时有副作用）：若输出 vector 为空，依次追加 `hv, arc, h, kx, ky`；否则先把输出 vector 的第 0 个
  元素保存到 `+0xc8`，把第 0 个元素改为 `hv`，再追加 `arc, h, kx, ky, (+0xc8 保存值)`；最后统一追加 `+0xd8..+0x100`（错位、转角）
  和 `+0xd0`（fieldErr）。
- Edge::SetParameter：单词数 < 5 → 打印并返回；之后逐个要求单词存在（否则 `out_of_range`）：
  `β(+0x90) = stod(word[4]) / 180 · π`，`|ρ|(+0xa8) = |stod(word[5])|`，`G(+0xb0) = stod(word[6])`，`K1(+0x98) = stod(word[7])`，
  `K2(+0xa0) = stod(word[8])`，`hv(+0x88) = stoi(word[9])`；
  `ψ(+0xb8) = ((K1·G)/|ρ|) · ((sin²β + 1)/cos β) · (1 − (K1·K2·G·tan β)/|ρ|)`（乘法顺序 `K1*K2*G*tanβ`）。
- Edge::GetParameter：`[(double)hv, tan(β)/|ρ|, −tan(β−ψ)/|ρ|, +0xc8..+0xf0, +0xc0]`。

##### 2.6.4.7 `field`（Field，0x1f8）与 `multipole`

InitLattice 对独立的 `field` 行（第 12 个比较）先改写单词再建元件：

```
word[9] = fieldMapPath + word[9]
f = stod(word[5])
if f != 0:
    while words.size() < 14: words.push("invalid")
    if stoi(word[3]) == 0: Initial.needSynPhase = 1          # V3 == 0 才需要扫相位
    words.push(sprintf("%d", cav)); cav += 1                  # 原行恰好 ≤14 词时它成为 word[14]
e = new Field; SetElement(words, e)
```

（若原行多于 14 个单词，腔序号被追加到更后面，`word[14]` 是用户写的多余单词 → stoi 失败或腔序号错。）

`Field::Field`：基类默认 + `fieldType(+0x8c) = 1`，`zeroField(+0xf8) = {0,0,0}`，其余 vector 空。

`Field::SetParameter(index, zStart, words)`：

1. 单词数 < 10：打印 `"Error! parameter not enough in element : \n"` + 单词（`\t` 分隔）+ 换行，然后 **`exit(1)`**（不写 ErrorLog）。
2. `SetBasicParameter`；`zEnd(+0xa0) = zStart + L`；
   `fieldType(+0x8c) = stoi(word[4])`；`frequency(+0xa8) = stod(word[5])`（Hz）；`phase(+0x110) = stod(word[6]) / 180 · π`；
   `Ke(+0x90) = stod(word[7])`；`Kb(+0x98) = stod(word[8])`；`mapIndex(+0x88) = stoi(word[9])`（已被 SetElement 改成序号）。
   **V3（word[3]）不存进 Field**。
3. 按类型：
   - 1（RF）：`frequency == 0` → 抛 `RF cavity frequency cannot be set to 0, Check the parameters of the RF cavity elements`；
     `cavIndex(+0x118) = stoi(word[14])`；返回。
   - 0、2、3：直接返回（类型 0 不报错）。
   - 4（叠加组）：`frequency != 0` 时先 `cavIndex = stoi(word[14])`；然后解析子块（2.6.5.3）。
   - 其他：打印 `"Field type error!"` 换行，返回（不报错、不退出）。

`multipole` 行（独立，或作为 superpose 子元件）：先 `MultipoleFieldMapGenerator`（写场图文件，2.7.2），再
`MultipoleFieldMapConverter` 把单词改写为 field 行：

```
word[0] = "field"; word[1], word[2], word[9] 不变; word[8] = word[6];
word[3] = "0"; word[4] = "3"; word[5] = "0"; word[6] = "0"; word[7] = "1"
```

即 `field L R 0 3 0 0 1 G name`：静磁场（类型 3）、Ke=1、**Kb = word[6]（G）**。生成的场图本身已按 G 定标（2.7.2），
所以有效磁场 ∝ G²（推测为缺陷，需实验，见第 5 节）。独立 multipole 行随后与 field 行同样处理（加 fieldMapPath 前缀；f="0"
不加腔号），建 Field 元件。multipole 行至少需要 10 个单词，Generator 不检查（越界读）。

##### 2.6.4.8 `rfq`

`ReadRFQ(inputFilePath, outputFilePath)`（2.8）；单词补 `"invalid"` 到 14 个；追加 `"%d"(cav)`，`cav += 1`；`new RFQ`；SetElement。
RFQ 的虚函数 `SetParameter` 槽是 `Element::SetParameter`（空函数，vtable 0x2ec8d8 重定位核对），**所以 RFQ 对象的 index、zStart、
长度、孔径等一个都不设置**；非虚的 `RFQ::SetParameter(double, vector<string>)` 在 .so 中没有任何调用者（死代码，2.8.2）。
lattice 的 z 照常按 `word[1]` 累加。RFQ 在 .so 中实际不可用（推测，需实验）。schema 也没有 `rfq`。

#### 2.6.5 superpose 块（确定，另注）

##### 2.6.5.1 解析

遇到 `superpose` 行进入块循环（以下“读下一行”均指跳过空行读下一非空行，**不检查 EOF**）：

```
z0List=[], spList=[], subs=[], zEndList=[], errRows=[], Rmin=1.0, sub=0
loop:
    if word[0] != "superpose": mode = 0; break          # 该行被丢弃（不再分派）
    z0List.push(strtod(word[1]))
    if n < 3:   sp = [stod(word[1]), 0,0,0,0,0]
    elif n < 7: 抛 "Abnormal number of parameters for keyword superpose"
    else:       sp = [strtod(word[1..6])]             # 不做角度换算，多余单词忽略
    spList.push(sp)
    读下一行
    若是 err_cav_ncpl_stat / err_cav_ncpl_dyn / err_quad_ncpl_stat / err_quad_ncpl_dyn 之一（只认一行）：
        errRows += [2, (dyn?1:0), (double)elements.size(), strtod(word[1..n-1])...,
                    补 0 到 18 (mod 20), (cav?1.0:0.0), (double)sub]
        读下一行
    若 word[0]=="multipole": Generator + Converter（就地改写为 field 行）
    subs.push(words)                                   # 不检查是什么关键字，任何行都被当作子元件
    R = strtod(word[2]); if R < Rmin: Rmin = stod(word[2])
    zEndList.push(z0List[sub] + strtod(word[1]))
    读下一行
    if word[0]=="superposeout":
        n < 7 → 抛 "Abnormal number of parameters for keyword superposeout"
        out = [stod(word[1..3]), stod(word[4..6])·π/180]     # z0,x0,y0, θz,θx,θy(rad)
        mode = (stod(word[6])==0 && stod(word[5])==0) ? 1 : 2
        break                                          # superposeout 行被消耗
    sub += 1
```

要点：
- 块内“读下一行”的循环是 `do { getline; StringSplit; } while (单词表为空)`，不看流状态：块没写完就到文件末尾时，`getline`
  失败后行串保持为空，循环永不结束（推测：内核挂起；boundary.txt 的 superpose 块同样）。
- **块的结束行总被消耗**：`superposeend` 是约定写法，但任何非 `superpose` 行都会结束块并被丢弃。漏写 `superposeend` 时，紧跟块后的
  那个元件**会消失**（lattice 总长随之变短）。
- 块半径 `Rmin = min(1.0, 各子元件 R)`：子元件 R 都大于 1 m 时块半径为 1.0。
- 子元件的错误行编码：`[0]=2`（超位置子元件）、`[1]` 静态 0/动态 1、`[2]` 当前元件数（即块元件将获得的序号）、`[3..]` 原行数值
  （N、r、误差列…）、补零使长度 ≡ 18 (mod 20) 后追加 `[18]` 腔 1/四极 0、`[19]` 子元件序号。原行数值多于 15 个时不补零，行长度错位。
- 子元件为 field 且 `f = stod(word[5]) != 0` 时计为 RF；块结束后统一给所有 `word[0]=="field"` 的子元件加 `fieldMapPath` 前缀
  （multipole 改写后也是 field），并对 RF 子元件：`stoi(word[3]) == 0` → `needSynPhase = 1`。
- RF 子元件数 > 0 且 `mode != 0` → 抛 `RF cavity is temporarily not allowed in curve transmission`；RF 子元件数 > 1 → 抛
  `Only one RF cavity is allowed to be used in a set of superimposed fields`。

##### 2.6.5.2 合成的 type-4 field 行

块被改写成一个（无 RF）或 2–3 个（有 RF）合成 field 行，每行由 20 个单词的“头”加若干 20 单词的“子块”组成。
数值用 `__to_xstring(vsnprintf, 328, "%f", v)` 生成，即 **printf `%f`：6 位小数**——块长度、半径、子元件位置都被**舍入到 1e-6 m**，
再被 `stod` 读回；下游所有元件的 z 都受这一舍入影响。整数用 `"%d"`。

子块（每个子元件一个）：子元件原单词全部照抄，不足 12 个用 `"invalid"` 补到 12（多于 12 个则全部照抄，之后错位），再加 8 个 `%f` 值：
`[12] z0_i, [13] zEnd_i, [14..19] = sp_i[0..5]`（即 superpose 行的 z0, x0, y0, θz, θx, θy，角度仍为度）。

无 RF 时只有一个元件，头为：

| 位置 | 0 | 1 | 2 | 3 | 4 | 5–9 | 10 | 11 | 12 | 13 | 14–19 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 值 | `field` | `%f`(max zEnd − min z0) | `%f` Rmin | `0` | `4` | `0`×5 | `%d` mode | `invalid` | `0` | `%f` 同 [1] | `invalid`×6 |

有一个 RF 子元件 k 时按其位置拆分（`z0`、`zEnd` 都是块内相对坐标）：

- 情形 A：`z0_k == 0`（腔在块首）
  - 片 1（RF）：头 = `field, word1_k, %f Rmin, 0, 4, word5_k, word6_k, word7_k, word8_k, 0, invalid, invalid, 0, word1_k, %d cav, invalid×6`
    （**21 个单词**，见下面的缺陷）；子块 = 所有 `z0_i < zEnd_k` 的子元件。
  - 片 2（静态）：头 `[1] = %f(max zEnd − zEnd_k)`，`[4]=4`，`[5..9]=0`，`[10],[11]=invalid`，`[12] = %f zEnd_k`，`[13] = %f max zEnd`，
    `[14..19]=invalid`；子块 = 所有 `zEnd_k < zEnd_i` 的子元件。
- 情形 B：`zEnd_k == max zEnd`（腔在块尾）
  - 片 1（静态）：`[1] = %f z0_k`，`[12]=0`，`[13]=%f z0_k`；子块 = `z0_i < z0_k`。
  - 片 2（RF）：头 = `field, word1_k, %f Rmin, 0, 4, word5..8_k, 0, invalid, invalid, %f z0_k, %f zEnd_k, %d cav, invalid×5`（20 个单词）；
    子块 = `z0_k < zEnd_i`。
- 情形 C：其他（腔在中间）：片 1 静态（`z0_i < z0_k`）、片 2 RF（`z0_k < zEnd_i` 且 `z0_i < zEnd_k`，头同情形 B）、
  片 3 静态（`zEnd_k < zEnd_i`，头同情形 A 的片 2）。
- 每建一个 RF 片 `cav += 1`。子元件可能同时出现在多个片中（跨越片边界的元件）。

头 `[12]` 是该片在块内的起点，`Field::SetParameter` 用 `subZStart = 片 zStart + (z0_i − stod(word[12]))` 定位子元件。

错误行随片复制：对每个片中的子元件 i，把 `errRows` 中 `[19] == i` 的 20 元行复制到该片的错误表，`[19]` 改为片内序号；
后续片中 `[2]`（元件序号）加 1。非空则 `ErrorParameter.elemErr(+0x60).push(该片的整个错误向量)`（一个片一行、行内是 20×m 个数）。

superposeout（`mode` 1/2，只可能无 RF）：`out.push(elemZEnd.back())` 后 `Lattice.spOut(+0x210).push(out)`（7 个数：
z0, x0, y0, θz, θx, θy, 出口 z），并置 `elements.back()->inSuperposeOut(+0x68) = mode`。

**已确认的缺陷（.so 与 DLL 相同）**
1. 情形 A 的 RF 片头多了一个 `invalid`（21 个单词）。`Lattice::SetElement` 和 `Field::SetParameter` 都按 `words.size()/20` 和固定
   偏移 20+20j 找子块关键字，于是 **RF 片的所有子块都对不上位置**：不加载任何子场图，也不解析任何子元件。按伪代码，“腔放在
   superpose 块第一个”时该片没有场（推测其后果是腔不加速、扫相位失败或报错；必须用原内核实验，见第 5 节）。
2. 子块关键字为 `drift` 的检测用了错误的步长（`Field::SetParameter` 中比较 `words[15+15j]`），实际上 drift 子元件几乎不会被识别，
   被静默跳过（子元件 drift 的孔径不生效）。

##### 2.6.5.3 `Field::SetParameter` 的 type-4 子块解析（确定）

设 `m = words.size() / 20`（整除）；`m < 2` 直接返回；对 j = 0..m−2，`w = 20 + 20j`：

- 若 `words[w] == "field"`：`t = stoi(words[w+4])`
  - t==1（RF）：`subType.push(2)`，`subKe.push(stod(w+7))`，`subKb.push(stod(w+8))`，**`frequency = stod(w+5)`、`phase = stod(w+6)/180·π`
    （覆盖 Field 自身的值）**，`subMapIndex.push(stoi(w+9))`；
  - t==3（静磁）：`subType.push(1)`，`subKe.push(0)`，`subKb.push(stod(w+8))`，`subMapIndex.push(stoi(w+9))`；
  - t==2（静电）：`subType.push(3)`，`subKe.push(stod(w+7))`，`subKb.push(0)`，`subMapIndex.push(stoi(w+9))`；
  - 其他：打印 GBK 编码的 `叠加场中出现无法叠加的场`（24 字节）+ 换行，跳过该子块。
  - 以上三种都继续：`subZStart.push(zStart + (stod(w+12) − stod(word[12])))`，`subRadius.push(stod(w+2))`，`subLength.push(stod(w+1))`，
    `subPlace.push([stod(w+14), stod(w+15), stod(w+16), stod(w+17)·π/180, stod(w+18)·π/180, stod(w+19)·π/180])`。
- 否则若 `words[15+15j] == "drift"`（步长错误，见缺陷 2）：`subType.push(0)`，Ke、Kb、map 置 0，z/R/L 同上（place 不填）。
- 注意内部子类型编码：RF→2、静磁→1、静电→3、drift→0，与 lattice 的 1/2/3 不同。

#### 2.6.6 输出面与 `end` 收尾（确定）

- `outputplane V1`：`z = elemZEnd.empty() ? stod(word[1]) : elemZEnd.back() + stod(word[1])`，`Initial.outputPlanes.push(z)`。
  即相对“到此为止最后一个元件的出口”，负值表示上游。
- `automaticoutput V1 V2 V3`：`first = (同上，相对当前末端) + stod(word[1])`，`spacing = stod(word[2])`，`span = stod(word[3])`，
  存入局部 `autoRows`（此时不展开）。

`end` 时依次：
1. `ErrorParameter.nElements(+0xa8) = elements.size()`。
2. `outputPlanes.push(L_tot)`，`L_tot = elemZEnd.back()`（无元件时未定义）。
3. 若有 automaticoutput 行：**`outputPlanes` 清空**（所有 `outputplane` 和刚加的末端面都丢掉），对每行按出现顺序：
   `z = first; while (z <= span + first) { push(z); z += spacing; }`（逐次累加，不是 first + k·spacing；比较用 `<=`；
   spacing ≤ 0 会死循环）；最后若 `outputPlanes.back() != L_tot` 则 `push(L_tot)`。
4. 检查单调：存在相邻 `planes[i+1] < planes[i]` → 抛 `Output plane anomaly, check automaticoutput`（相等允许）。
   因为末端面总在最后，**任何超出 lattice 末端的输出面都会触发这个错误**（无论来自 outputplane 还是 automaticoutput；
   这解释了 AGENTS 第 6 节“超出 lattice 末端会报错”）；按非升序书写的 outputplane 也会报错。
5. spOut 哨兵：若 `spOut` 为空 → `push([0,0,0,0,0,0,L_tot])`（`spOutCount(+0x228)` 保持 0）；否则 `spOutCount = spOut.size()`，
   且仅当 `L_tot > spOut.back()[6]` 时 `push([0,0,0,0,0,0,L_tot])`。
6. 若 `ErrorParameter.beamErr(+0x48)` 为空：`flags[0] = flags[1] = 0`（没有 err_beam 行时束流误差开关无效）。
7. 打印：`"Lattice initialisation complete, total length " << L_tot << " m"` endl；`"A total of " << outputPlanes.size() << " output planes"` endl；
   `"  "` endl（`<<double` 为默认 6 位有效数字）。
8. 结束读取。

#### 2.6.7 误差关键字（解析部分；含义属 G）（确定）

编码：类型 腔=1、静磁元件=0、superpose 子元件=2；模式 静态=0、动态=1。“值”= `strtod(word[1..n−1])`，“开关值”= `stoi(word[1..n−1])`。

| 关键字 | 写入 |
|---|---|
| `err_step a b` | `ErrorParameter.step(+0x00)[0] = stoi(word[1])`，`[1] = stoi(word[2])` |
| `err_beam_stat` / `err_beam_dyn` | `beamErr(+0x48)` 依次 push `0.0`/`1.0` 及所有值（不补齐，多行首尾相接）；然后把 `beamSw(+0x78)` 补零：长度 ≤ 30 补到 30，否则 < 60 补到 60 |
| `err_cav_ncpl_stat/dyn`、`err_quad_ncpl_stat/dyn` | 行 `[类型, 模式, (double)elements.size(), 值...]` 补零到 20 个 → `elemErr(+0x60).push` |
| `err_beam_stat_on` / `err_beam_dyn_on` | `beamSw(+0x78)` push `0`/`1` 及所有开关值；同样补零到 30/60；任一开关值 ≠ 0 → `flags(+0xb0)[0]` / `[1]` = 1 |
| `err_quad_stat_on`、`err_cav_stat_on` | `vector<int>{类型, 0, 开关值...}` 补零到 20 → `elemSw(+0x90).push`；任一 ≠ 0 → `flags[2] = 1` |
| `err_quad_dyn_on`、`err_cav_dyn_on` | 同上（模式 1）；任一 ≠ 0 → `flags[3] = 1`（err_quad_dyn_on 的置位走共用尾部，推测） |

`elements.size()` 是“下一个元件的序号”，即误差作用于其后的元件。开关值每个调用两次 stoi（一次判非零、一次存储），结果相同。
多行 `err_beam_*` / `*_on` 的补零规则与顺序相关（先写 `err_beam_stat` 会先补到 30，再写 `err_beam_stat_on` 时长度已超过 30 → 补到 60），
具体位置约定由 G 的 `GenerateErrorSet` 决定。

#### 2.6.8 与 schema.py 的 38 个 lattice 关键字对照

| schema 关键字 | 内核（.so） |
|---|---|
| drift, field, quad, solenoid, bend, steerer, edge | 识别；参数位置与 schema 一致（见 2.6.4；quad/solenoid 的 word[3] 保留不读） |
| diag_energy, diag_size, diag_position | **不识别**，静默忽略 |
| superpose | 识别（块） |
| superposeend | 不识别；只作为块结束行被消耗（任何非 superpose 行都有同样效果） |
| superposeout | 识别（块结束 + 坐标变换） |
| start, end | 识别（2.6.1） |
| lattice, lattice_end | 不识别，静默忽略 |
| outputplane, automaticoutput | 识别（2.6.6） |
| spacechargecomp | .so 不识别（DLL 识别，2.13） |
| err_step, err_beam_dyn, err_beam_stat | 识别 |
| err_quad_ncpl_dyn/stat, err_cav_ncpl_dyn/stat | 识别 |
| err_quad_cpl_dyn/stat, err_cav_cpl_dyn/stat | **不识别**，静默忽略 |
| err_beam/quad/cav_dyn_on、_stat_on（6 个） | 识别 |
| adjust | 不识别，静默忽略 |
| （schema 没有） | multipole、rfq |

### 2.7 场图文件

#### 2.7.1 InitLattice 交给场图读取的内容

- 文件名基：`fieldMapPath + word[9]`（独立 field、multipole、superpose 子元件都一样）；分量类型 = `stoi(word[4])`（子块为 `word[w+4]`）。
  读取由 `Lattice::SetMatrix(name, type, FieldMatrix&)`（C）完成：按名字查重，未载入时读文件并返回序号，SetElement 再把 `word[9]`
  改成序号字符串。
- rodata 中与场图有关的后缀和消息（读取逻辑见 C 的文档）：`.edx .edy .edz`、`.bdx .bdy .bdz`（RF 电/磁）、`.esx .esy .esz`（静电）、
  `.bsx .bsy .bsz`（静磁）、`"Element field file cannot be opened :"`、`"The one-dimensional field has been expanded into a three-dimensional field."`。
  按类型选择后缀的对应关系属 C，这里不作定论（推测 1→ed*/bd*、2→es*、3→bs*）。
- 存储顺序判断、归一化、网格在 SetMatrix 中，A 不涉及。

#### 2.7.2 `Initial::MultipoleFieldMapGenerator(vector<string>& w)`（确定）

输入为 multipole 行：`m = stoi(w[4])`（极数：3 六极、4 八极、5 十极），`L = stod(w[1])`，`N = stoi(w[5])`（每个横向方向的网格段数），
`G = stod(w[6])`，`R = stod(w[2])`。

```
k = G/(R*R);   k = k + k                 # m == 3
k = (G/(R*R*R)) * 6.0                    # m == 4
k = (G/(R*R*R*R)) * 24.0                 # m == 5
其他 m：抛 "Abnormal setting of the number of magnetic poles"
d = (R + R) / N
节点 (i, j, s)，i, j = 0..N，s = 0..2（3 个 z 平面，内容相同）；内存与写出顺序 i 最快、其次 j、最后 s
x = i*d − R,  y = j*d − R
m=3:  Bx = 2·(x*y*k)（先算 x*y*k 再自加）;            By = (x*x − y*y)*k
m=4:  Bx = (x*3.0*x*y − y*y*y)*k;                      By = (x*x*x − x*3.0*y*y)*k
m=5:  x3 = x*x*x; Bx = (x3*y − x*y*y*y)*k*4.0;          By = (y*y*y*y + (x3*x − x*6.0*x*y*y))*k
Bz = 0
```

写出三个文件 `fieldMapPath + w[9] + ".bsx" / ".bsy" / ".bsz"`（`ios::out`，覆盖同名文件），每个文件内容：

```
"  2  " <L>            ← "  " << 2 << "  " << L，然后换行
"  N  " <-R> "  " <R>   ← x 方向：段数、最小、最大
"  N  " <-R> "  " <R>   ← y 方向
"  1"                   ← 归一化系数
然后 3×(N+1)×(N+1) 行，每行 "  " << 值（对应分量）
```

所有 double 用 ostream 默认格式（6 位有效数字，`%g` 风格），**场值只保留 6 位有效数字**。随后 Converter 把行改为
`field L R 0 3 0 0 1 G name`（2.6.4.7），读回由 SetMatrix 完成。

#### 2.7.3 `BinaryFileReader`（确定）

`BinaryFileReader::BinaryFileReader(const std::string& path)`：构造 `std::ifstream`，保存路径（+0x208），以 `ios::in | ios::binary`（0xc）打开；
失败抛 `std::runtime_error("Unable to open file: " + path)`（会使进程 abort）。析构：已打开则关闭，释放路径与流。
它只被 `PIC::importSCF`（F，空间电荷场导入）使用，**与场图读取无关**；读取方法在 F 的函数里。

### 2.8 RFQ

#### 2.8.1 `ReadRFQ(const string& inPath, const string& outPath)`（确定）

`outPath` 未使用，读写都用 `inPath`（即 inputFilePath；AVAS 运行时是 `<输出>/inputs/` 副本）。数值都以 `float`（单精度）保存。

1. `inPath + "rfqinputAnn.txt"`：打不开只打印 `"RFQ Structure path Ann ERROR!"` 换行并继续（此后读循环立即遇到 EOF）。
   逐行 `getline`（**不经 NormalizeLineEndings**）+ StringSplit，用 `.at()` 取第 0..12 个词 `strtod` 转 float（不足 13 个 → `out_of_range`，
   空行同样）；保存第 5..12 列到 8 个数组 A5..A12。
2. `inPath + "rfqinputV.txt"`：打不开打印 `"RFQ Structure path V ERROR!"`；每行取第 0..13 列，保存第 0,1,4,6,7,8,12,13 列到 V0,V1,V4,V6,V7,V8,V12,V13。
3. 写 `inPath + "rfqinput.txt"`（`ios::out|ios::trunc`，失败打印 `"output error"`）：对 i = 0..len(V1)−1 写一行
   `V0 V12 V1 V7 V8 V6 A6 A5 A8 A7 A10 A12 A9 A11`，单空格分隔，前 6 个用 ostream 默认格式（float→double，6 位有效数字），
   后 8 个用 `"%f"`（缓冲 58 字节）；行尾 `'\n'` 并 flush。（Ann 行数少于 V 行数时越界读。）
4. 写 `inPath + "E0-z.txt"`：对每个 i：`V13[i] = V13[i] / 100.0f`（就地，单精度），写 `V13[i] " " V4[i]` 换行。

#### 2.8.2 `RFQ::RFQ`、`RFQ::SetParameter`（确定；后者在 .so 中是死代码）

- `RFQ::RFQ`：基类默认，`+0x88..+0x140` 置 0，`+0x158 = vector{0,0,0}`，`+0x170 = 1e-16`。
- `RFQ::SetParameter(double zStart, const vector<string>& w)`（无调用者；DLL 中整个函数已不存在）：`SetBasicParameter`；`frequency(+0x150) = stod(w[5])`；
  `cavIndex(+0x148) = stoi(w[14])`；打开 `inputFilePath + w[?]`（要求 ≥ 8 个单词，推测是 w[7]），失败打印 `"RFQ Structure path ERROR!"` 并 `exit(1)`；
  逐行读取：第一行初始化起点，cell 长度 `stod(word[1]) / 100`（cm→m）推入 +0xa0，起点、终点累加到 +0xb8/+0xd0，其余列推入各 cell 数组；
  结束打印 `"RFQ cell number : "` 与 `"RFQ length      : "`。由于不被调用，具体列含义不再展开。

### 2.9 boundary.txt（`InitLattice` 后半、`Boundary::SetElement`、`Geometry::SetParameter`）

仅当 `Initial.boundary(+0x144) == 1`。

#### 2.9.1 读取（确定）

- 打开 `inputFilePath + "boundary.txt"`，失败抛 `File "boundary.txt" cannot be opened, Check the inputFile routing and boundary.txt.`；
  打印 `"Initialising boundary..."` 换行；**不经 NormalizeLineEndings**，直接对 ifstream 逐行 `getline` + StringSplit（CRLF 行最后一个
  单词带 `\r`：数值不受影响，但 `end\r` ≠ `end`）。
- 局部：`offsets = [0.0]`，`first = [0]`，`last = []`；`skip = true`。
- 每行（非空）：`skip` 且不是 `start` → 丢弃；是 `start` → `skip = false`；否则分派：
  - `word[0]` 为 `"1"` 或 `"2"`：`new Geometry`，`Boundary::SetElement(words, g)`。
  - `superpose`：块（2.9.2）。
  - `end`：打印 `"Boundary loaded, total length: " << Bnd.zEnd.back() << " m,  Lattice length: " << L_tot << " m."` endl；
    `last.push(geoms.size() − 1)`；对每个 i：`Boundary.spOutMap(+0x60).push([offsets[i], (double)first[i], (double)last[i]])`；
    **然后继续读下一行**（`end` 不结束读取；第二个 `end` 会再追加一遍行；`end` 之后的形状行仍被加入）。
  - 其他：抛 `Undefined shape exists, check boundary.txt`（包括形状 `0`、大写、`start` 之后的 `!` 行以外的任何未知词）。
- EOF 后：若 `Bnd.zEnd.back() < Lattice.elemZEnd.back()` → 抛
  `Boundary has been enabled, but the total length of the boundary is less than the Lattice. Restart the simulation after completing the boundary file.`
  （无任何几何段时 `back()` 未定义）。

#### 2.9.2 boundary superpose 块（确定，含缺陷）

结构与 lattice 版相同：`superpose` 行（单词数 < 3 → `[V1,0,0,0,0,0]`；3–6 → 抛 `Abnormal number of parameters for keyword superpose in boundary.txt`；
≥ 7 → 前 6 个值），下一非空行是子几何行（`Rmin = min(1.0, word[2])`，`zEnd = z0 + stod(word[1])`），再下一行若为 `superposeout` → `iVar8 = 1` 结束，
若不是 `superpose` → `iVar8 = 0` 结束（**superposeout 的参数在这里不解析**）。随后构造一个合成单词表
`["0", "Copper", %f(max zEnd − min z0), %f Rmin, %d iVar8, "RLP", …（iVar8==1 时取 Lattice.spOut[Initial+0x1a0] 的 7 个值两遍）, %d (Initial+0x1a0), invalid×6, 子块…]`，
**但调用 `Boundary::SetElement` 时传的是当前行（结束块的那一行）的单词，合成表被丢弃**（反汇编 0x83536：`rsi = r12 = rsp+5008`，
即当前行单词向量；合成表在 `*(40(%rsp))`）。后果：
- 块以形状行（`1`/`2`）结束 → 整个 superpose 块被这一行代替（该行也不会再单独处理）；
- 块以 `superposeout …` 或 `end` 结束 → `Geometry::SetParameter` 对 `word[0]` 做 stoi → `invalid_argument` → 进程 abort。
iVar8 == 1 时随后还会：`k = Initial+0x1a0`，`Initial+0x1a0 += 1`；`offsets.push(Lattice.spOut[k][6] − Bnd.zEnd.back())`；
`first.push(geoms.size())`；`last.push(geoms.size() − 1)`。

#### 2.9.3 `Boundary::SetElement(vector<string> w, Geometry* g)`（确定）

```
zStart = segStart.empty() ? 0 : segStart.back() + segLen.back();  segStart(+0x30).push(zStart)
segLen(+0x48).push(stod(w[2]))                                    # 长度是 word[2]
segEnd(+0x18).push(segStart.back() + segLen.back())
g->SetParameter(index = (int)round(geoms.size()), zStart = segStart.back(), w)
geoms(+0x00).push(g)
```

#### 2.9.4 `Geometry::SetParameter(int index, double zStart, vector<string> w)`（确定，成员含义部分推测）

- `words(+0x00) = w`；`index(+0x38)`；`shape(+0x58) = stoi(w[0])`（非整数 → abort）；若 ≥ 2 个单词：`name(+0x60) = w[1]`，`zStart(+0x30)`。
- shape 1、2：从 k=2 起，遇到单词 `"RLP"` 前的每个单词 `stod` 推入 `shapePar(+0x40)`（所以 shapePar[0] 是长度 word[2]）；没有 `RLP` 则到行尾返回；
  有 `RLP` 则其后的所有单词依次写入 `place(+0x18)[0..]`（vector 只有 6 个元素，多于 6 个写出堆外），再把 `place[3..5]` 由度转弧度。
  推测行格式：`<1|2> <名称/材料> <长度> <形状参数…> RLP <dx> <dy> <dz> <θx> <θy> <θz>`。
- shape 0（复合，只应来自 2.9.2 的合成表，实际上不可达）：`mode(+0x68) = stoi(w[4])`，`spIndex(+0x6c) = stoi(w[13])`，`w[6..12]` 推入 `+0xe8`；
  子块从 20 起步长 20：子块首词为 `1` 或 `2` 时 `subShape(+0xb8).push((int)stod)`，`subLen(+0xd0).push(stod(w+2))`，解析到 `RLP` 的形状参数（随后被丢弃，推测）、
  RLP 后的 place，再读子块 `[14..16]` 原值与 `[17..19]` 度→弧度，各加上 place 后推入 `+0x88`。
- 其他 shape：打印 `"Unknown geometric shapes have been set"` endl 并 `exit(1)`（InitLattice 只把 `1`/`2` 行交进来，正常不可达）。

### 2.10 `Initial::InitMethodMP(MethodMP&)`（确定）

在 InitParticle 之后调用（此时 `Initial.frequency` 已是 Hz）。

| MethodMP | 来源 |
|---|---|
| +0x198 dt | `(1.0 / frequency) / stepPerCycle`（先求周期再除） |
| +0x1a0 | spaceCharge |
| +0x1a8 | track（vector<int> 复制） |
| +0x208 / +0x238 / +0x220 / +0x250 | cores / scPar / outputPlanes / numOfGrid |
| +0x1c0 / +0x1c4 / +0x1cc / +0x1d0 | multithreading / dumpPeriodicity / boundary / lpBoundary |
| +0x1d8 / +0x1f0 | pcHistogram / longLimits |

### 2.11 `PreTreatment(vector<double>)`（确定；不是文本处理）

名字容易误解：它是分布生成（E）中 `KV2D/WB2D/PB2D/GS2D` 调用的数值函数（rodata 报错 `ERROR!!!   @SigmaToAxial2D!`）。输入 6 个数
`[ε1, β1, α1, ε2, β2, α2]`，构造两个 2×2 σ 矩阵（σ11 = β·ε，σ12 = −α·ε，σ22 = (1+α²)/β·ε）；任一 `σ11·σ22 ≤ σ12²` 时打印该报错并
`exit(1)`；否则用 Eigen `SelfAdjointEigenSolver` 求特征值，得到椭圆半轴、倾角（`atan(...)·0.5·180/π`，度）等，返回 6 个数的 vector。
精确公式和输出含义由 E 的文档给出，这里不重复。

### 2.12 构造与析构（确定）

- 元件构造函数都先内联基类：`words` 置空，`typeName = "drift"`，`strength(+0x58) = unk_60 = 0`，`+0x68 = +0x6c = 0`，`+0x70..+0x80 = 0`，再设子类 vtable：
  Bend 另清 `+0xd0..+0x100`；Edge 另清 `+0xc0..+0xf0`；Quadrupole 见 2.6.4.3；Solenoid 无额外；Steerer 清 `+0x88..+0xa0`；Field、RFQ 见上。
- `Geometry::Geometry`：全部清零，`place(+0x18) = vector<double>(6, 0)`，`name = ""`。`Boundary::Boundary`、`Lattice::Lattice`：全部清零
  （`Lattice+0x78`、`+0x208`、`+0x228` 等 int 置 0）。
- 析构 `~Element`、`~Field`、`~Lattice`、`~Boundary`、`~Beam`、`~Initial` 只释放各自的 vector/string，**`~Lattice` 不 delete 元件，`~Boundary`
  不 delete Geometry，`~Beam` 不释放粒子数组**（进程结束前泄漏，无可观察影响）。
- 基类 `Element::GetParameter`、`Element::SetParameter`、`Drift::GetParameter`、`Field::GetParameter` 都是空函数。

### 2.13 AVAS.dll 与 libAVAS.so 在本子系统的差异

| 项 | .so | DLL |
|---|---|---|
| `path` 参数 | `char*`（UTF-8） | `wchar_t*`，逐字节去 0 转窄串，非 ASCII 路径损坏 |
| `main_agent` 状态标志 | 局部变量，忽略参数 | 用调用者的 int*，初始化后写 2 并传给 Run |
| `spacechargecomp` | 忽略 | 识别：`Initial+0x258.push([z, 1.0, stod(word[1])])`，z = 当前末端（无元件时 0） |
| superpose 情形 A 的 RF 片头 `[3]` | `"0"` | 腔自己的 `word[3]`（V3） |
| 情形 A 的 21 词缺陷 | 有 | 有（6 个 invalid） |
| `RFQ::SetParameter` | 存在但不被调用 | 不存在 |
| input/beam 关键字集合 | 见 2.4/2.5 | 相同（字符串表逐项一致） |

---

## 3. 对齐清单

建议的通用方法：用 `examples/hwr010` 复制出的临时项目，按每项改一两行输入，分别用原内核（Linux .so / Windows DLL）和新内核运行，
比较 stdout（含 “Lattice initialisation complete, total length …”、“A total of N output planes” 等打印）、`ErrorLog.txt`、进程退出方式
（返回码 0/1/2、abort、exit(1)）、DataSet.txt 的 z 列与输出面个数。只影响内部状态的项可用新内核的单元测试对照本规格。

| ID | 行为 | 涉及关键字/文件 | 建议的比对测试 |
|---|---|---|---|
| A-01 | `path` 给三个目录各追加 `/`，打印顺序 input、fieldMap、output | path() | 捕获 stdout 三行逐字比较 |
| A-02 | `main_agent` 忽略传入值；返回 0/1/2；非 string 异常 abort | main_agent | 传 1 和 2 各跑一次都完整运行；构造 `particlenumber abc` 观察 abort、无 ErrorLog |
| A-03 | ErrorLog 格式 `YYYY/MM/DD HH:MM:SS     Initialisation stage error: msg\n`，追加模式 | errorOut | 制造 `scmethod XYZ`，比较 ErrorLog 去掉时间后的正文；连续两次不删文件，确认两行 |
| A-04 | CRLF / CR / LF 等价 | 三个文本文件 | 同一 input/beam/lattice 分别存三种换行，DataSet 完全一致（固定 randomseed） |
| A-05 | `!` 紧贴数值吞掉该数值 | StringSplit | `particlenumber 2000!x` 与 `particlenumber 2000 !x` 对比：前者走默认 −1 路径（观察 InitParticle 行为/报错） |
| A-06 | 只有空格、制表符是分隔符；逗号是词的一部分 | StringSplit | `twissx 0,1 ...` 应被 strtod 读成 0（尾随垃圾忽略）；比较初始 rms |
| A-07 | 只把关键字转小写；值区分大小写 | Read/ReadBeam | `SCMETHOD FFT` 正常；`scmethod fft` 报 SCMethod 错；`distribution gs gs` 报 Distribution Type Error |
| A-08 | lattice 关键字区分大小写，未知行静默忽略 | InitLattice | `Drift 0.1 0.02 0` 不入表：总长比小写版短 0.1 m |
| A-09 | `method` 行额外打印 “input.txt Contains unknown keywords, check input.txt” | input.txt | stdout 计数该警告行 |
| A-10 | `dimension` 报错 | input.txt | ErrorLog 正文 `input.txt Contains unknown keywords: Dimension`，返回 1 |
| A-11 | `scutoffdistance`、`pchistogram`、input `randomseed` 按整数解析 | input.txt | `randomseed 12345.9` 与 `12345` 结果逐位相同；`scutoffdistance 1.7` 与 `1` 相同 |
| A-12 | `steppercycle` 第二个值写 seStep | input.txt | 二次粒子算例改 V2，SeSynParticle 步长变化（属 G/D，可只验证 dt） |
| A-13 | `setnumberofcores` 少于 10 个参数报错 | input.txt | ErrorLog 正文逐字比较 |
| A-14 | `secondarybeam 1` 强制关闭空间电荷 | input.txt | 与 `spacecharge 1` 同时写，观察不打印/不计算 SC（D 的输出） |
| A-15 | schema 的 `scanphase`、`sim_type`、`spacechargelong/type` 内核不读 | input.txt | 加/删这些行 DataSet 完全一致，stdout 多出警告 |
| A-16 | `twissx/y/z` 必须恰 4 词，文件顺序 α β ε | beam.txt | 5 词报 `twissx have 3 parameters…`；交换值观察 rms 变化 |
| A-17 | `beamtype dc` 与 `twissz` 的顺序相关 | beam.txt | dc 在前/在后两种写法比较纵向分布（属 E 的后续处理） |
| A-18 | `displacepos/displacedpos` 乘 0.001 | beam.txt | 单粒子模式 `displacepos 1 0 0` 的初始 x = 1 mm（SingleParticle.txt） |
| A-19 | `distribution` 必须 3 词 | beam.txt | 2 词报 `The initial bunch distribution contains two parameters…` |
| A-20 | beam 未知关键字（`initpos`、`use_dst`）只警告 | beam.txt | stdout 出现 `beam.txt Contains unknown keywords, check input.txt` |
| A-21 | `start` 之前的行忽略；`end` 之后的行忽略 | lattice | 在 start 前与 end 后各放一个 drift，总长不变 |
| A-22 | 缺 `end` 时不做收尾 | lattice | 删 end：stdout 无 “Lattice initialisation complete”；观察是否崩溃（记录原内核结局） |
| A-23 | z 按 word[1] 累加，bend 的 word[1]=0 不计长度 | lattice | 含 bend 的 lattice 打印总长与 Σword[1] 比较 |
| A-24 | `quad` 少参数 / G=0 报错 | lattice | ErrorLog 正文 `Error! parameter not enough in element : quad`、`The gradient of the quadrupole cannot be set to 0, …` |
| A-25 | drift/solenoid/steerer/bend/edge 参数不足只打印不退出 | lattice | `solenoid 0.1 0.02 0`：stdout 有 “Error! parameter not enough…” 加单词表，运行继续 |
| A-26 | field 参数不足 `exit(1)` | lattice | `field 0.1 0.02 0 1`：进程退出码 1，无 ErrorLog（Windows 上 Python 子进程退出码） |
| A-27 | 7 个单词的 steerer 通过首检后 abort | lattice | 记录原内核结局 |
| A-28 | RF field 频率为 0 报错 | lattice | ErrorLog `RF cavity frequency cannot be set to 0, …` |
| A-29 | Field 类型 0 不报错、类型 ≥5 打印 “Field type error!” 继续 | lattice | stdout 比较 |
| A-30 | V3==0 才置 needSynPhase | lattice | 全部 V3=2 时不出现 “Start scan phase....”（B 的打印）；有一个 V3=0 时出现 |
| A-31 | 腔序号按 RF 元件出现顺序（field、rfq、superpose RF 片）编号 | lattice | 两个腔交换次序，synData.txt 每腔相位顺序随之交换 |
| A-32 | `outputplane` 相对当前末端；按出现顺序 | lattice | 输出 outData_*.dst 文件名中的 z 值与计算值一致 |
| A-33 | 超出末端或降序的输出面报 `Output plane anomaly, check automaticoutput` | lattice | 构造两种情况比 ErrorLog |
| A-34 | `automaticoutput` 覆盖所有 `outputplane`；逐次累加；末端面补齐 | lattice | 同时写两者，统计 “A total of N output planes” 与 outData 文件列表；用 spacing=0.1、span=0.3 检查是否 4 个面（浮点累加边界） |
| A-35 | superpose 块长度、半径以 `%f` 舍入到 1e-6 | lattice | 子元件 L=0.1234567：打印的总长尾数为 0.123457 而非 0.1234567 |
| A-36 | 块结束行被消耗（漏写 superposeend 吞掉下一元件） | lattice | 块后直接跟 drift 0.2：总长比写 superposeend 时短 0.2 |
| A-37 | superpose 参数 3–6 个报错 | lattice | ErrorLog `Abnormal number of parameters for keyword superpose` |
| A-38 | superposeout 少于 7 词报错；mode 1/2 判定 | lattice | ErrorLog 比较；θx=θy=0 与 θx≠0 两种算例输出坐标变换差异（D） |
| A-39 | 块中两个 RF 腔、RF + superposeout 报错 | lattice | 两条 ErrorLog 正文 |
| A-40 | 情形 A（腔在块首）的 RF 片丢失全部子场（缺陷） | lattice | 见第 5 节实验 1；新内核是否“修复”需决策，默认按原内核行为 |
| A-41 | superpose 子 drift 被跳过 | lattice | 块内放 R 很小的 drift，原内核不在该处丢粒子 |
| A-42 | multipole 生成 `.bsx/.bsy/.bsz`（6 位有效数字、x 最快）并按 Kb=G 叠乘 | lattice / 场图 | 比较生成文件逐字节；G=1 与 G=2 单粒子偏转比为 4（见实验 3） |
| A-43 | multipole 极数非 3/4/5 报错 | lattice | ErrorLog `Abnormal setting of the number of magnetic poles` |
| A-44 | 误差关键字写入 ErrorParameter 的编码与补零规则 | err_* | 新内核单元测试对照 2.6.7；原内核侧对比 StaticErrorData.txt / DynamicErrorData.txt（G） |
| A-45 | `cpl` 类误差、`adjust`、`diag_*`、`lattice`、`spacechargecomp`（.so）静默忽略 | lattice | 加/删这些行 DataSet 完全一致 |
| A-46 | boundary：`start` 前忽略、`1`/`2` 形状、未知词报错、`end` 不结束读取、长度不足报错 | boundary.txt | 逐项构造：ErrorLog `Undefined shape exists, check boundary.txt`、`Boundary has been enabled, but …`；两个 end 时 “Boundary loaded” 打印两次 |
| A-47 | boundary superpose 块被结束行代替（缺陷） | boundary.txt | 实验 2 |
| A-48 | ReadRFQ 读写 inputFilePath 下的 4 个文件及格式 | rfqinput*.txt | 准备两份小输入，比较生成的 rfqinput.txt、E0-z.txt 逐字节 |
| A-49 | `dt = (1/f)/stepPerCycle` | InitMethodMP | stdout “Simulation time step is”（D 打印）逐位比较 |
| A-50 | DLL 宽字符路径截断 | DLL path | 用含中文的目录在 Windows 运行，记录失败形式 |

---

## 4. 函数覆盖表

| 函数 | 作用一句话 | 在正文哪节 |
|---|---|---|
| FUN_0011f420 | PLT 区的跳转桩（调用地址 0），编译/链接器生成，无语义 | — |
| errorOut | 打印错误并按阶段追加写 ErrorLog.txt（不退出） | 2.1.3 |
| path | 设置 input/fieldMap/output 三个全局路径（各加 `/`） | 2.1.1 |
| main_agent | 内核总入口：读输入、建 lattice、初始化粒子、运行；捕获 string 异常返回 1/2 | 1、2.1.2 |
| Initial::~Initial | 释放 Initial 的 vector/string | 2.12 |
| Beam::~Beam | 释放 Beam 的三个 vector（不释放粒子数组） | 2.12 |
| Bend::SetParameter | 读 α、ρ、n、HV 并算弧长、h、kx、ky | 2.6.4.6 |
| Bend::Bend | 二极铁构造（清零误差字段） | 2.12 |
| Bend::GetParameter | 输出 hv、arc、h、kx、ky、误差；有保存/改写 vec[0] 的副作用 | 2.6.4.6 |
| Element::GetParameter | 基类空实现 | 2.12 |
| Element::SetParameter | 基类空实现（RFQ 的虚函数槽用它） | 2.6.4.8、2.12 |
| Boundary::Boundary | 几何段表构造（清零） | 2.12 |
| Boundary::~Boundary | 释放 vector（不删 Geometry） | 2.12 |
| Boundary::SetElement | 累加几何段 z（长度取 word[2]）并调 Geometry::SetParameter | 2.9.3 |
| Drift::SetParameter | 检查 ≥4 词后只设基本参数 | 2.6.4.2 |
| Drift::GetParameter | 空实现 | 2.12 |
| Edge::SetParameter | 读 β、ρ、G、K1、K2、HV，算边缘修正角 ψ | 2.6.4.6 |
| Edge::Edge | 边缘元件构造 | 2.12 |
| Edge::GetParameter | 输出 hv、tanβ/ρ、−tan(β−ψ)/ρ、误差 | 2.6.4.6 |
| Element::SetBasicParameter | 公共字段：单词、zStart、关键字、L、R | 2.6.4.1 |
| Field::~Field | 释放 Field 的 vector | 2.12 |
| Field::Field | 场图元件构造（类型默认 1、零场向量） | 2.6.4.7 |
| Field::SetParameter | 解析场图元件参数与 type-4 叠加子块 | 2.6.4.7、2.6.5.3 |
| Field::GetParameter | 空实现 | 2.12 |
| Element::~Element | 释放基类 vector/string | 2.12 |
| PreTreatment | 两组 2D Twiss → σ 矩阵 → 特征值/椭圆参数（分布生成用，非文本处理） | 2.11 |
| Geometry::SetParameter | 解析 boundary 几何行（形状、参数、RLP 定位） | 2.9.4 |
| StringSplit | 按空格/制表符拆词，`!` 起注释（吞掉紧贴的词） | 2.2.2 |
| ReadRFQ | 读 rfqinputAnn/V.txt，写 rfqinput.txt 与 E0-z.txt（都在输入目录） | 2.8.1 |
| Initial::errorOut | 写 ErrorLog 后 exit(1)；无调用者 | 2.1.3 |
| Initial::MultipoleFieldMapConverter | 把 multipole 行改写成静磁 field 行（Kb=G） | 2.6.4.7 |
| Initial::MultipoleFieldMapGenerator | 按极数生成多极场的 .bsx/.bsy/.bsz 场图文件 | 2.7.2 |
| Initial::InitMethodMP | 把 Initial 参数交给 MethodMP，算 dt | 2.10 |
| Initial::Initial | 全部默认值 | 2.3 |
| Initial::NormalizeLineEndings | 整文件读入并统一换行 | 2.2.1 |
| Initial::ReadBeam | 解析 beam.txt | 2.5 |
| Initial::Read | 解析 input.txt | 2.4 |
| Initial::InitLattice | 解析 lattice.txt（元件、superpose、输出面、误差）与 boundary.txt | 2.6、2.9 |
| Geometry::Geometry | 几何段构造（place 6 个 0） | 2.12 |
| RFQ::RFQ | RFQ 元件构造（+0x170=1e-16） | 2.8.2 |
| Lattice::Lattice | Lattice 构造（清零） | 2.12 |
| Lattice::~Lattice | 释放 Lattice 的 vector（不删元件） | 2.12 |
| Lattice::SetElement | 累加元件 z、加载场图、调用元件 SetParameter 并入表 | 2.6.3 |
| BinaryFileReader::BinaryFileReader（构造） | 以二进制打开文件，失败抛 runtime_error（供 PIC::importSCF） | 2.7.3 |
| BinaryFileReader::~BinaryFileReader | 关闭并释放 | 2.7.3 |
| Quadrupole::SetParameter | 读梯度 G、孔径直径；G=0 或参数不足报错 | 2.6.4.3 |
| Quadrupole::Quadrupole | 四极铁构造（两个含义不明的常数） | 2.6.4.3 |
| Quadrupole::GetParameter | 输出错位、转角、带误差梯度 | 2.6.4.3 |
| RFQ::SetParameter | 读 RFQ cell 文件；.so 中无调用者 | 2.8.2 |
| Solenoid::SetParameter | 读 B | 2.6.4.4 |
| Solenoid::Solenoid | 螺线管构造 | 2.12 |
| Solenoid::GetParameter | 输出错位、转角、带误差场 | 2.6.4.4 |
| Steerer::SetParameter | 读 Bx/By（Ex/Ey）、类型、最大值 | 2.6.4.5 |
| Steerer::Steerer | 校正铁构造 | 2.12 |
| Steerer::GetParameter | 输出类型、Bx、By、最大值 | 2.6.4.5 |

（共 55 个，与 `assign_A_input_lattice.txt` 一一对应。）

---

## 5. 未解决的问题（需要原内核实验）

1. **superpose 块首为 RF 腔时的场缺失**（2.6.5.2 缺陷 1）。实验：示例 `hwr010` 腔前后各放 drift，(a) 腔单独作为 field；
   (b) `superpose 0 / field <hwr010 行> / superpose 0 / drift <同长> / superposeend`（腔在块首）；(c) 把腔放到块尾（情形 B）。
   比较 synData.txt 的能量增益和 stdout 的扫相位信息。预期（按伪代码）(b) 无增益或扫相位失败、(c) 与 (a) 一致。
2. **boundary.txt 的 superpose 块被结束行代替**（2.9.2）。实验：boundary 中写一个 superpose 块后跟 `superposeout …`（预期 abort），
   以及块后跟一个形状行（预期该形状行代替整块，几何段数少一个），用 LongitudinalBeamLoss/损失位置判断。
3. **multipole 的 Kb = G 叠乘**（2.6.4.7）。实验：六极 `multipole 0.1 0.03 0 3 20 G 0 0 0 m3`，单粒子 x 偏 1 mm，G=1 与 G=2 比较出口 x′；
   预期比值 4（若为 2 则说明 Converter 理解有误）。同时核对生成的 `m3.bsx` 文件内容与 2.7.2 一致。
4. **缺少 `end` 时的结局**：输出面列表为空，进度计算 `outputPlanes.back()` 未定义；记录原内核是崩溃还是用随机值运行。
5. **`!` 紧贴数值**（2.2.2）是否真的丢值：`particlenumber 2000!c`，观察 InitParticle 是否按 −1 处理（报错文本或粒子数）。
6. **RFQ 在 .so 中是否可用**：RFQ::SetParameter 无调用者，RFQ 对象参数全为默认；准备 rfqinputAnn/V.txt 跑一个 `rfq` 行，记录行为。
7. **superpose 子 drift 被跳过**：块内放 R=1 mm 的 drift，大束流下原内核是否在该段丢粒子。
8. **BOM**：UTF-8 带 BOM 的 input.txt/lattice.txt 第一行是否失效（推测失效）。
9. **automaticoutput 的浮点边界**：spacing=0.1、span=0.3 时末尾是否多/少一个面（0.1 累加 3 次 = 0.30000000000000004 > 0.3）。
10. **Windows DLL 的中文路径**：确认 2.1.1 的截断转换（例如目录 `测试`）导致 “File "input.txt" cannot be opened”。
11. `Initial+0x03c / 0x0c0 / 0x130 / 0x140 / 0x17c / 0x1e0 / 0x1f8` 的含义：A 中只见默认值，需看 E/D 的读取处。
12. `Geometry::SetParameter` shape 0 分支中子块形状参数是否真的被丢弃（Ghidra 可能漏掉一次 push_back）；由于该分支在 .so 中不可达，优先级低。

---

## 6. 对结构表的更正与补充

证据都在本文对应小节。

1. **ErrorParameter**（structures.md 第 9 节只恢复了容器位置）：
   - `+0x00 vector<int>(2) step` = err_step {组数, 每组次数}，默认 {1,1}（main_agent 构造时 `new int[2]{1,1}`）。
   - `+0x48 vector<double> beamErr`：每条 err_beam 行依次追加 `[0(stat)/1(dyn), 值…]`，不是“首元素补 0”。
   - `+0x60 vector<vector<double>> elemErr`：普通行 20 个数 `[类型(0 静磁/1 腔), 模式(0/1), 起始元件序号, N, r, 误差列…]`；
     superpose 片的行是 20×m 个数，每 20 个为 `[2, 模式, 元件序号, …, (18)腔?1:0, (19)片内子元件序号]`。
   - `+0x78 vector<int> beamSw`：束流误差开关，补零到 30 或 60。
   - `+0x90 vector<vector<int>> elemSw`：`{类型, 模式, 开关…}` 补零到 20。
   - `+0xa8 int nElements`：`end` 时的元件总数。
   - `+0xb0 vector<int>(4) flags`：[0] 束流静态开、[1] 束流动态开、[2] 元件静态开、[3] 元件动态开（main_agent 构造为 4 个 0）。
2. **Initial+0x1a0**：boundary.txt 中 superposeout 块计数，用作 `Lattice.spOut` 的下标（structures 写“只见到默认值”）。
3. **Initial+0x1e4 bunchNumber 构造时不初始化**（structures 未注明）。
4. **Lattice.spOut 行的分量顺序已核对**：`{z0, x0, y0, θz, θx, θy（弧度）, 出口 z}`，来自 superposeout 的 word[1..6]（后三个 ×π/180）。
   `spOutCount(+0x228)` 只在至少有一个 superposeout 块时于 `end` 写入（= 块数，不含哨兵）。
5. **Element+0x68 inSuperposeOut** 取值 1（只有 z/x/y 平移与 θz）或 2（θx 或 θy 非零），写在块元件上。
6. **Field 叠加子类型编码**：`subType` 中 RF=2、静磁=1、静电=3、drift=0（与 lattice 的 1/2/3 不同）；`subPlace = {z0, x0, y0, θz, θx, θy(rad)}`
   取自子元件所在 superpose 行；`subZStart = 片 zStart + (z0_i − word[12])`；`subRadius/subLength` 取子元件的 word[2]/word[1]（确定）。
   Field 的 frequency/phase 会被 RF 子元件覆盖。`fieldType` 构造默认 1；类型 0 被静默接受。
7. **Bend+0xc8** 由 `Bend::GetParameter` 写入（保存调用者输出 vector 的第 0 个元素），不是 SetParameter。
8. **RFQ 的虚 SetParameter 槽是基类空函数**，非虚 `RFQ::SetParameter` 在 .so 中无调用者——RFQ 的 0x88–0x150 成员在 .so 运行中都不会被赋值。
9. **Geometry**：`+0x40 shapePar` 从 word[2]（长度）开始到 `RLP` 前；`+0x18 place` 是 `RLP` 之后的 6 个值（后三个度→弧度）；`+0x68 = stoi(word[4])`、
   `+0x6c = stoi(word[13])` 只在 shape 0 时写；`+0xb8/+0xd0/+0x88/+0xe8` 是 shape 0 的子块形状、长度、定位和头部 7 个数。
10. **Boundary+0x60** 每行 `{z 偏移, 首几何段, 末几何段}` 已确认（来自 `end` 时的构造），且每遇到一次 `end` 追加一遍。
11. Quadrupole 构造常数确认为 `+0x90 = −4e-05`、`+0x98 = 4.518219`（仍未找到读取处）；RFQ `+0x170 = 1e-16`。
12. AVAS.dll 中粒子步长是 0x1a8（main_agent 中 `local_b20 * 0x1a8 + 0x28`），与 .so 的 0x190 不同，提醒 DLL 结构需单独核对。
