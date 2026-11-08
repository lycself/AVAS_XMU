# 65 libPIC.so（AVAS 旧 GPU 内核）

> 来源：`avas/engine/libPIC.so`（md5 f0f7b3f6…，2.8 MB）。宿主代码用 Ghidra headless（`engine/tools/ExportDecomp.java`）导出伪代码（222 个函数，
> Ghidra 地址 = ELF 虚拟地址 + 0x100000，下文地址均为 Ghidra 地址；伪代码不进仓库）；核函数只有 sm_80 的 SASS、没有 PTX，用 `cuobjdump -sass` / `-elf`
> 读机器码与常量段；Python 调用方 `avas/gpu/{structures,pic,initializer,run}.py`、`avas/core/MultiParticle.py`；第 9 节是在 A100 上运行原库的实测。
>
> 第 1–7 节主要依据宿主侧伪代码。粒子推进、外场插值、统计等核函数的函数体按宿主侧的**示踪粒子版本**推测（标“推测”）；
> 空间电荷（第 8 节）读了核函数的 SASS，PICNIC 部分已由实测核实（9.2）。新内核实现了其中的 `scmethod PICNIC`（M6b，见 [M6-notes.md](M6-notes.md)）。

源文件（取自 CUDA 内部符号 `_INTERNAL_xxx_<len>_<name>_cu_`）：`cuInitializer.cu`、`cuBeam.cu`、`cuRealPusher.cu`、`cuPhasePusher.cu`、`cuPIC.cu`、
`cuPICNIC.cu`、`cuOutput.cu`（另有纯宿主代码，名字不可见）。依赖：libcudart.so.11.0、libcufft.so.10、Open MPI 4 的 libmpi.so.40、libstdc++（只需 GLIBC_2.2.5）、CUB/Thrust、pthread；导出 83 个 C 函数（含 `main`）。

---

## 1. 入口、调用流程与结构体

### 1.1 Python（`avas/gpu/pic.py` 的 `SimulationRunner`）调用顺序与对应函数

`avas/core/MultiParticle.py` 在 GPU 模式下先生成 `input_gpu.txt`（在 input.txt 后追加 `outputpath <输出目录>`、`fieldpath <场图目录>`、
`numofgrid 24 24 24`、`MeshRms 4 4 4`、`statOutputInterval 1`、`beampath <输入目录>`）和 `beam_gpu.txt`（给 `kneticenergy` 行追加一个 `0`，
因为 GPU 内核要读两个值，见 2.3），然后：

| 步 | Python | libPIC 函数（地址） | 作用 |
|---|---|---|---|
| 1 | `MPIContext` | `SetDeviceForProcessors`（0012a480） | `devIdx = rank % cudaGetDeviceCount()`（参数 −1 时），`cudaSetDevice`，分配 MPI 收发缓冲 |
| 2 | `_read_configs` | `ReadConfigurations`（0012a930） | argv = `["avas","common=<input_gpu.txt>","beam=<beam_gpu.txt>","lattice=<lattice.txt>"]`，返回 7 个链表（见 2.1） |
| 3 | | `LoadRunningOptions(configs[0])`（0012b340） | 解析 common 文件为 `RunningOptions` |
| 4 | | — | Python 自己 `mkdir(outputPath)`、`mkdir(outputPath/output_0)`；`configs[4]`（误差开关行）非空时把 outputPath 改为 `outputPath/output_0` |
| 5 | | `InitializeLatticeComponents(configs[2], ro)`（0013a600） | 建元件表；场图经 `SearchBaseComponent`（00138a00）/`InsertBaseComponent`（0013a4e0）→ `ReadASCIIField`（001397d0）或 `ReadBinaryField`（00138a50） |
| 6 | | `InitializeErrorParameters(lat, configs[4], configs[5])`（0013ad60） | 动态误差 |
| 7 | | `InitializeSuperpose(lat, configs[6])`（0013b430） | 叠加组，重排 z |
| 8 | `Beam` | `InitializeBeam(configs[1], mpi, lat, ro)`（0012ebd0） | 读 beam 文件 → `ReadParticlesFromFile`（0012df40，5 个 pthread 并行 `ReadParticlesFromFileThreading` 0012dde0）→ `ConvertParticlesFromPhase2Real`（0012a0e0）→ `InitializeParticles`（0012e2c0） |
| 9 | `CUB` | `InitializeCUB`（0011cb50） | CUB 临时缓冲 |
| 10 | `PICHandler` | `InitializePIC`（0011baf0） | PIC 网格数组（8.1） |
| 11 | `Statistics` | `InitializeBeamStatistics`（0012baf0） | 只分配 `pcHistogram`（`numPCHistogram×16` 字节，主机 + 设备） |
| 12 | `FFT` | `InitializerFFT`（0011c910） | cuFFT 计划 |
| 13 | | `InitializeOutputObject(configs[3], ro, beam, lat, out)`（0012bb20） | 输出面（panel）与 BeamSet 缓冲池 |
| 14 | `_setup_streams` | `CreateCudaStream`×2（0012a3a0） | 两个 CUDA 流（`[0]` 推进，`[1]` 清零网格电荷） |
| 15 | `run` | `InitializePhase`（001342c0） | 参考粒子走一遍 lattice，给每个 RF 场图扫相位 |
| 16 | | `RunSimulationWithoutSpaceCharge`（0012fbb0）/ `…UsingFFT`（00130770）/ `…UsingPICNIC`（001316f0） | 主循环；`spaceChargeFlag==0` → 无空间电荷；否则 `spaceChargeMethod==0` → FFT，其他 → PICNIC 系列 |
| 17 | | `OutputPhaseParticles(beam, mpi, "<out>/outData_<latticeLength %f>.dst")`（00136a00） | 末端 TraceWin dst |
| 18 | | `Release*`、`cudaDeviceReset` | 释放；Python 不调 `MPI_Finalize`（注释掉了） |

### 1.2 `main`（0010d450）

`main` 是可执行入口（库同时导出它）。与 Python 流程的差别：

1. `MPI_Init`、`MPI_Comm_rank/size` 填进栈上的 `MPIObject`；`CheckWhetherDeviceAssigned(argc, argv)`（0012a3c0）在 argv 中找 `gpu=N`，
   返回 N（**argc ≥ 2 但没有 `gpu=` 时返回未初始化值**，见第 10 节；argc < 2 返回 −1）→ `SetDeviceForProcessors`。
2. `CheckCommandLineArguments`（0012a510）：要求同时有 `common=`、`beam=`、`lattice=`，否则打印
   `error: command line requires "common=" / "beam=" / "lattice=" …` 并退出。
3. `ReadConfigurations`；要求 `configs[0..2]` 均非空，否则静默结束。
4. rank 0 创建 `outputPath` 和 `outputPath/output_0`；`configs[4]` 非空（有 `err_*_dyn_on` 行）时 outputPath 改为 `…/output_0`（与 Python 一致）。
5. 同 1.1 第 5–14 步，打印 `rank %d : lattice components number = %d, lattice length = %f, create lattice cost time = %f ms`、
   `rank %d : holding %d particles of total %d, reading particles cost time = %f ms`。
6. 若有输出面：估算 panel 所需显存 `numPanels × N_rank × 0x38` 字节，超过 `cudaMemGetInfo` 的可用量则打印
   `The required memory size of panels exceeds the available GPU memory size %lu B` 并退出。
7. `InitializePhase`（返回扫相步数），打印 `rank %d : scan phase cost time = %f ms, scan steps = %d`；`OutputScanPhaseResult`（00136040）写
   **当前工作目录**下的 `scanResult.dat`。
8. `dumpPeriodicity ≠ 0` 时估算 BeamSet 大小 `((scanSteps/100)/dumpPeriodicity)·(N_rank·56 + 25) + 34` 字节，≥ 50 GB 打印
   `The size of output plt files exceeds the setting value 50.0 GB.` 并退出（scanSteps/100 近似主循环步数，因为扫相步长是主步长的 1/100，见 4.1）。
9. 运行主循环，打印 `rank %d : pushing cost time = %lf ms, simulate steps = %d`；`OutputPhaseParticles` 写 `outData_<L>.dst`；释放；`MPI_Finalize`。

Python 路径不做第 6、8 步检查，也不写 `scanResult.dat`。

### 1.3 主要结构体（偏移按二进制核对；与 `structures.py` 不一致处已注明）

`structures.py` 的布局在 x86-64 下大部分与二进制吻合（`InitializeLatticeComponents` 的 `malloc(n·0x138)`、`LoadRunningOptions` 的 `malloc(0x360)` 等可证），
**但 `SyncParticle` 不对**：`extMagnField` / `extElecField` 在二进制里是 `float3`（各 12 字节，偏移 0x98 / 0xa4，见 `GetExtFieldForTracerParticle`
0012d020 把 0x98..0xaf 清零、`UpdateTracerParticle` 以 float 读 0x98..0xac），所以 `SyncParticle` = 0xc0（Python 写成 0xd8），`timeStamp` 在 0xb0、
`phaseTimeStamp` 在 0xb8。连带：`Beam` = 0x4a0（`InitializeBeam` 中 `malloc(0x4a0)`；Python 0x4d0），`scanParticle` 在 0x320、`tracerParticle` 在 0x3e0
（Python 0x3f8）；`ScanPhaseThreadArgs` = 0x108（`InitializePhase` 中 `malloc(n·0x108)`，Python 0x120）。Python 只读 `particles_gpu.numParticles`（0x1dc）和
`particles_init.numParticles`（0x2a4），这两个偏移正确，所以目前不出错。

| 结构 | 大小 | 含义与关键字段 |
|---|---|---|
| `Configurations` | 0x110 | 单链表节点：`buffer[256]`（预处理后的一行）、`componentIdx`（0x100）、`lineIdx`（0x104）、`next`（0x108） |
| `RunningOptions` | 0x360 | common 文件的全部开关，见 2.2 |
| `MPIObject` | 0x68 | `rank`、`commSize`、`devIdx`、`avgNumParticles`（每 rank 粒子数）、`phaseFlag`（全体是否进入“相位推进”模式，MPI min）、`numNormalParticles`（全局，MPI sum）、`numReducedInside`（全局在 PIC 网格内的粒子数）、`numNotMissedParticles`（全局未丢失数）、若干 MPI 缓冲 |
| `Lattice` | 0x28 | `numComponents`、`latticeLength`、`latticeComponents_cpu/gpu`（元件数组）、`baseComponents`（场图链表） |
| `LatticeComponents` | 0x138 | 见下表 |
| `BaseComponents` | 0x168 | 一组场图（按名字去重）：`name[256]`、`nx,ny,nz`、`minx,maxx,miny,maxy,maxz`、`dx,dy,dz`、`field_cpu/gpu`（float6 数组）、`next` |
| `float6` | 32（对齐） | `bx,by,bz,ex,ey,ez`（float），每个网格点 32 字节（`ReadASCIIField` 用 `n<<5` 分配） |
| `ErrorParameters` | 0x60 | `sw[7]`（int 开关）、`errx,erry,errz`（m）、`errpx,errpy,errpz`（rad）、`errkg`（相对值）、`errs`（相位，度） |
| `Particles` | 0xc8 | SoA：`status`、`numStatusChanged`/`numStatusMissed`（managed int，本步状态改变 / 丢失计数）、`phaseFlag`（粒子已到达矩阵元件入口、等待相位推进）、`insideFlag`（在 PIC 网格内）、`panelIdx`（下一个输出面序号）、`globalIdx`（原始粒子序号）、`index`、`locatedIdx`（所在元件，初值 −1）、`numNormalParticles`/`numParticles`/`numNotMissedParticles`、`picGridIndex`、`charge,mass,qOverMass,massMev`（宏粒子）、`pos,vel`（实空间，vel = γβ）、`phasePos,phaseVel,phaseTimeStamp`（穿过某 z 面时记录的坐标）、`extMagnField/extElecField/intElecField`（float3）、`timeStamp` |
| `SyncParticle` | 0xc0 | `charge,mass,qOverMass,massMev`、`cavityEz`（0x20，轴上 Ez 幅值累计，扫相用）、`status`（0x28：0 正常，1 到达末端，2 向后丢失）、`locatedIdx`（0x2c）、`phaseFlag`（0x30）、`pos`（0x38）、`vel`（0x50，γβ）、`phasePos`（0x68）、`phaseVel`（0x80）、`extMagnField`（0x98，float3）、`extElecField`（0xa4，float3）、`timeStamp`（0xb0）、`phaseTimeStamp`（0xb8） |
| `Beam` | 0x4a0 | `distributionType[2]`、`numCharge`（0x8）、`particleRestMass`（0x10，MeV）、`frequence`（0x18，Hz）、`current`（0x20，A）、`timeStart`（0x28）、`timeLength`（0x30）、`kneticEnergySum`（0x38）、`twissx/y/z`、`displacePosition`、`displacedPosition`、`kneticEnergy[2]`（0xb8）、`particles_cpu`（0xc8）、`particles_gpu`（0x190）、`particles_init`（0x258，整束，rank 拆分前）、`scanParticle`（0x320，扫相用参考粒子）、`tracerParticle`（0x3e0，示踪参考粒子） |
| `CUBObject` | 0x98 | CUB 临时缓冲与重排用的备份数组 |
| `PIC` | 0xf8 | 核函数参数偏移（相对 0x160）：`nx,ny,nz,numCopies` 0x0–0xc、`dx,dy,dz` 0x10–0x20、`minx..maxz` 0x28–0x50、`gridCharge` 0x60、`gridCoef` 0x70、`distributedGridCharge`（4 份副本）0x78、`gridPotential` 0x88、`cutoffInt[3]` 0xa0、`cutoffIntRadius` 0xac、`field`（float3）0xb8、`noneZeroLine` 0xd8 |
| `FFTSolver` | 0x40 | cuFFT 句柄与频域数组 |
| `BeamStatistics` | 0x1c0 | 见 7.2 |
| `OutputPanels` | 0x38 | 一个输出面：`panelz`、`phasePos/phaseVel/phaseTimeStamp`（cpu/gpu，按 rank 粒子槽位存） |
| `OutputObject` | 0x20 | `numPanels`、`panels_cpu/gpu`、`bufferPool`（`numBufferPool` 个 `BufferPool`，每个 0x40） |
| `BufferPool` | 0x40 | `mutex`、`bufferPoolCnt`（已填条数）、`itr[]`（步数）、`phaseFlag[]`（0 实空间 / 1 相空间）、`timeStamp[]`、`posz[]`、`outputData_cpu[]`（每条 N_rank×0x38 的 pinned 内存）、`outputData_gpu` |
| `OutputData` | 0x38 | `posx,velx,posy,vely,posz,velz`（double）、`status`、`globalIdx`（int） |

`LatticeComponents`（0x138）：

| 偏移 | 字段 | 写入者 / 含义 |
|---|---|---|
| 0x00 | `phiScanned`（bool） | RF 元件解析时置 0，扫相后置 1；其他类型不初始化 |
| 0x08 / 0x10 / 0x18 | `zstart / zend / len` | 按 word1 累加；叠加组由 `InitializeSuperpose` 改写 |
| 0x20 | `rad` | word2（m）；宿主侧代码未见使用（孔径损失若有，只在核函数内，推测） |
| 0x28 | `freq` | field 行 word5（Hz） |
| 0x30 | `phiS` | field 行 word6（度），扫相目标 |
| 0x38 / 0x40 | `ke / kb` | word7 / word8 |
| 0x48 | `phi0` | 扫相结果（度） |
| 0x50 | `time0` | 参考粒子到达元件入口的时刻（s），RF 相位参考 |
| 0x58 | `superposeEnd` | 叠加组终点 z |
| 0x60 | `gradient` | solenoid / quad 的 word4（T、T/m） |
| 0x68 | `errorAnalysis` | 1 = 有误差 |
| 0x6c | `type` | 0 drift，1 solenoid，2 quad，3 bend，4 静磁场图，5 静电场图，6 RF 场图（1–3 称“矩阵元件”） |
| 0x70–0xd0 | `nx..field_gpu` | 从 `BaseComponents` 复制的场图网格 |
| 0x7c / 0x80 / 0x84 | `superpose / superposeStartIdx / superposeEndIdx` | 叠加组 |
| 0xd8 | `errorParameters` | |

---

## 2. 输入文件与关键字

### 2.1 `ReadConfigurations`（0012a930）与文本预处理

命令行参数逐个 `strtok(arg, "=")`：`common=` → 读入 `configs[0]`；`beam=` → `configs[1]`；`lattice=` → 按下述分派。返回 `calloc(0x38)` 的 7 个链表头：

| 下标 | 内容 |
|---|---|
| 0 | common（input）文件全部非空行 |
| 1 | beam 文件全部非空行 |
| 2 | lattice 元件行：`drift`、`field`、`quad`、`bend`、`solenoid`（每遇一行，元件计数 +1） |
| 3 | `outputplane` 行 |
| 4 | `err_quad_dyn_on`、`err_cav_dyn_on` 行 |
| 5 | `err_quad_ncpl_dyn`、`err_cav_ncpl_dyn` 行，`componentIdx` = 该行之前的元件数（即作用于其后的元件） |
| 6 | `superpose`、`superposeend` 行，`componentIdx` = 之前的元件数，`lineIdx` = 文件行号 |

- 每行 `fgets(buf, 256)`：**超过 255 字符的行被截成多行**。`PreprocessString`（0012a630）去掉行首空白，把连续的空格 / Tab 压成一个空格，删去 `\r`、`\n`、`\0`。
- 打不开文件打印 `Reading common/beam/lattice configuration file "%s" error.` 并返回（链表不完整）。
- lattice：第一个关键字小写后等于 `start` 之前的行全部丢弃（`start` 行本身也不入表）；其后关键字（**小写后**比较，CPU 内核区分大小写）为 `end` 时停止读取；
  不认识的关键字**静默忽略**，包括 `lattice`、`lattice_end`、`steerer`、`edge`、`multipole`、`rfq`、`automaticoutput`、`superposeout`、`err_*_stat*`、
  `err_beam_*`、`err_step`、`diag_*`、`adjust` 等。没有 `start` 行时 `configs[2]` 为空，`main` 静默结束。
- common / beam 文件没有注释语法，注释行只是匹配不到关键字。

### 2.2 common 文件（input.txt）：`LoadRunningOptions`（0012b340）

每行第一个词小写后与下表**依次**比较（不是 else-if，匹配后继续比较，但值词不会再命中），值用 `strtok/strtod/strtol` 读取，**缺值时 `strtod(NULL)` 段错误**；不认识的关键字静默忽略。

| 关键字 | 字段（偏移） | 默认值（`malloc` 后写入） | 说明 |
|---|---|---|---|
| `spacecharge V` | `spaceChargeFlag`（0x0） | 1 | `V≠0` 为真 |
| `scmethod S` | `spaceChargeMethod`（0x1c） | 0 | 值小写后：`fft`→0（FFT），`picnic`→1（PICNIC），`s2picnic`→2（S2PICNIC），`s4picnic` 或 `spicnic`→3（S4PICNIC，日志写作 “SPICNIC”）；其他值保持原值 |
| `steppercycle V` | `stepPerCycle`（0x38） | **未初始化** | 主步长 dt = 1/(f·V)；只读第一个值（CPU 还读 seStep） |
| `numofgrid a b c` | `numGridofPIC[3]`（0x4） | **未初始化** | 必须恰好 3 个值；`InitializePIC` 三个方向都向上取到 8 的倍数（8.1） |
| `meshrms a b c` | `rmsSize[3]`（0x48） | **未初始化** | 必须 3 个值；网格半宽 = a·σ（见 2.6） |
| `scananglestep V` | `scanAngleStep`（0x40） | 18.0 | 扫相初值间隔（度），初值个数 = ⌊360/V⌋ |
| `numsamplingcoef V` | `numSamplingCoef`（0x30，float） | 1.0 | DataSet 统计的抽样比例，见 7.3 |
| `usingaggregation V` | `usingAggregation`（0x1） | 1 | 电荷分配用 warp 聚合，并按 PIC 网格序号重排粒子 |
| `arrangesequenceinterval V` | `arrangeSequenceInterval`（0x10） | 15 | 每隔几步按网格序号重排一次（仅 usingAggregation） |
| `blockgroupsize V` | `blockGroupSize`（0x14） | 64 | 电荷分配核函数用（求解器部分） |
| `fieldpath P` | `fieldPath`（0x60） | `"field"`（相对当前目录） | 场图目录 |
| `outputpath P` | `outputPath`（0x160） | `"outputFile"`（相对当前目录） | 输出目录 |
| `beampath P` | `beamPath`（0x260） | `"field"`（显然是复制粘贴错误） | 粒子文件目录 |
| `dumpperiodicity V` | `dumpPeriodicity`（0x20） | 0 | 每 V 步向 BeamSet 缓冲池存一次全体粒子；0 = 不存 |
| `statoutputinterval V` | `statOutputInterval`（0x24） | 0 | 每 V 步写一行 DataSet.txt；**0 = 不写 DataSet**（Python 固定追加 1） |
| `numbufferpool V` | `numBufferPool`（0x28） | 4 | 缓冲池 / 写盘线程数 |
| `bufferpoolsize V` | `bufferoolSize`（0x2c） | 100 | 每个池的条数 |
| `pchistogram F N` | `histogramFlag`（0x2）、`numPCHistogram`（0x18） | 0、300 | **必须两个值**：开关和 bin 数 |

与 AVAS.dll（`Initial::Read`，10-input-lattice.md §2.4）比较：

| 类别 | 关键字 |
|---|---|
| 两边都认 | `spacecharge`、`scmethod`（取值不同：CPU 只认大写 `FFT`/`SPICNIC`，GPU 小写比较，多 `picnic`、`s2picnic`、`s4picnic`）、`steppercycle`（GPU 无默认值、不读 seStep）、`numofgrid`（GPU 必须 3 个）、`meshrms`（CPU 存 scPar[1..3]）、`dumpperiodicity`（GPU strtol、CPU stod 截断）、`pchistogram`（CPU：最多 6 个值，`[1]` 为 bin 数；GPU：恰 2 个值，开关 + bin 数，默认 300） |
| 只有 GPU | `scananglestep`、`numsamplingcoef`、`usingaggregation`、`arrangesequenceinterval`、`blockgroupsize`、`fieldpath`、`outputpath`、`beampath`、`statoutputinterval`、`numbufferpool`、`bufferpoolsize`；命令行 `gpu=N` |
| 只有 CPU（GPU 静默忽略） | `method`、`secondarybeam`、`boundary`、`probe`、`dimension`、`negligibleratio`、`lpboundary`、`indweight`、`longlimits`、`indcharge`、`integralgrid`、`scutoffdistance`、`scantstep`、`singleparticlemode`、`scsolverrecord`、`runperiod`、`track`、`randomseed`、`multithreading`、`setnumberofcores`、`maxthreads` |
| 两边都不认 | `scanphase`（GUI 写的；GPU 也没有任何 `scanData` 字符串，见第 4 节）、`sim_type`、`spacechargelong`、`spacechargetype` |

### 2.3 beam 文件：`InitializeBeam`（0012ebd0）

| 关键字 | 字段 | 说明 |
|---|---|---|
| `readparticledistribution F` | 文件 `beamPath/F` | **唯一的束流来源**；没有这一行时函数直接返回未填粒子的 `Beam`（`malloc` 未清零），之后 `InitializeCUB` 等使用未初始化内存（崩溃，推测） |
| `numofcharge V` | `numCharge`（0x8） | 电荷数 |
| `particlerestmass V` | `particleRestMass`（0x10） | **被粒子文件末尾的 mc² 覆盖** |
| `particlenumber V` | `particles_init.numParticles` | **被粒子文件头的 N 覆盖** |
| `distribution A B` | `distributionType[2]` | `GS`→0，`KV`→1，`WB`→2，`PB`→3（区分大小写，其他值保留原值）；之后无人使用 |
| `twissx/twissy/twissz a b c` | 0x40/0x58/0x70 | 各 3 个 double，原样存储；之后无人使用 |
| `displacepos a b c` | 0x88 | 不换算单位；之后无人使用 |
| `displacedpos a b c` | 0xa0 | 同上 |
| `frequency V` | `frequence`（0x18） | **被粒子文件头覆盖**（文件里是 MHz，×1e6） |
| `current V` | `current`（0x20） | **被粒子文件头覆盖**（文件里是 mA，÷1000） |
| `kneticenergy a b` | `kneticEnergy[2]`（0xb8） | **必须两个值**（Python 因此补 0）；只写进 BeamSet 文件头，不参与计算 |

也就是说 GPU 内核**不会按 twiss 生成分布**（库里没有任何 KV/WB/PB/GS 生成函数），只能从 TraceWin `.dst` 读粒子；beam.txt 里真正起作用的只有
`readparticledistribution` 和 `numofcharge`。AVAS.dll 的 `beamtype`、`randomseed`、`beamlength`、`bunchnumber`、`recscfield`、`impscfield`、
`kindofparticle` GPU 都不认。

粒子文件（`ReadParticlesFromFile` 0012df40，二进制小端，TraceWin dst 格式）：

```
char[2]（跳过）; int32 N → particles_init.numParticles; double I(mA) → current = I/1000;
double f(MHz) → frequence = f·1e6; char（跳过）;
N × {x, x', y, y', φ, W}（6 double，读入 pos.x, vel.x, pos.y, vel.y, pos.z, vel.z 的位置）;
double mc²(MeV) → particleRestMass
```

5 个线程各读一段（前 4 段各 ⌊N/5⌋ 个，推测）。同时求 φ 的最小 / 最大值和 ΣW：`kneticEnergySum = ΣW`，`timeStart = φmin/(2πf)`，`timeLength = (φmax−φmin)/(2πf)`。

`ConvertParticlesFromPhase2Real`（0012a0e0）启动 `ConvertParticlesFromPhase2Real_Device(timeStart, mc², f, …)` 把 (x, x', y, y', φ, W) 转成实空间
(pos[m], vel=γβ, timeStamp, globalIdx)。核函数不可见；推测 x 从 cm 换成 m（输出 dst 时乘 100，见 5.4），t_i = φ_i/(2πf)，并把粒子放在
`z_i = (timeStart − t_i)·v_zi ≤ 0`（最早的粒子在 z = 0，其余在入口上游）。依据：示踪粒子初始化为 `z = timeStart·βc`、`t = timeStart`（见下），
即 φ = 0 的参考粒子在 t = timeStart 时的位置。

`InitializeParticles`（0012e2c0）：

- 质量比 `m/mp = mc²/938.27187`。宏粒子权重 `w = |I/f/N/e/Q|`（`I ≠ 0`），宏粒子电荷 `q = Q·e·w`，质量 `w·(m/mp)·1.67262171e-27`，
  `q/m = (Q/(m/mp))·e/1.67262171e-27`；`I == 0` 时 w = 1（e = 1.60217653e-19）。
- MPI 拆分：每 rank `N/commSize` 个（推测），最后一个 rank 取余数；`particles_cpu/gpu` 只放本 rank 的粒子。`locatedIdx` 置 −1，`status`、`phaseFlag`、`panelIdx` 置 0。
- 扫相粒子 `scanParticle`（0x320）：电荷 `Q·e`、质量 `(m/mp)·mp`，位置 (0,0,0)，`locatedIdx = 0`，`t = 0`，`γβz` 由**全部粒子平均动能** `ΣW/N` 求出。
- 示踪粒子 `tracerParticle`（0x3e0）：同样的电荷质量，`status = 0`，`locatedIdx = −1`，`t = timeStart`，`z = timeStart·β·c`，`γβz` 同上，x、y、横向动量为 0。

### 2.4 lattice：`InitializeLatticeComponents`（0013a600）

每个元件行先 `tolower` 关键字，然后（`z` 按 word1 累加，`zstart = 前一元件的 zend`）：

| 关键字 | type | 读取的词 | 备注 |
|---|---|---|---|
| `drift L R` | 0 | L、R | |
| `solenoid L R x B` | 1 | L、R、跳过 word3、`gradient = word4`（T） | 硬边 |
| `quad L R x G` | 2 | L、R、跳过 word3、`gradient = word4`（T/m） | 硬边；G = 0 不报错（CPU 抛异常），相位推进中除以 0 → NaN（推测） |
| `bend L R …` | 3 | **只读 L、R** | α、ρ、n、hv 全部不读；GPU 没有任何二极铁物理（见 3.4、第 8 节） |
| `field L R V3 t f φ Ke Kb name` | 见右 | **V3（word3）被跳过**；`t = word4`：3 → type 4（静磁），1 → type 6（RF，`phiScanned = 0`），2 → type 5（静电）；`freq = word5`、`phiS = word6`（度）、`ke = word7`、`kb = word8`、`word9` = 场图名 | `t` 为 0 或 4（CPU 的超位置组）时 **type 不写，保留 malloc 垃圾值** |

场图：先 `SearchBaseComponent(name)` 按名字查已载入的组，未载入则 `InsertBaseComponent(name, type, fieldPath)`：名字**最后一个字符是 `b` 或 `B`**
（CPU 只认 `b`）时读二进制，否则读文本。读失败 `InitializeLatticeComponents` 返回 NULL（Python 退出 “Failed to initialize lattice components”）。

### 2.5 场图文件：`ReadASCIIField`（001397d0）/ `ReadBinaryField`（00138a50）

- 先 `opendir(fieldPath)`，对每个普通文件取第一个 `.` 之前的部分与 `name` 做 `strcasecmp`，命中则改用文件系统里的大小写（**大小写不敏感查找**）。
  文件名 `"%s/%s.%s"`（fieldPath/name.ext）。
- type 6（RF）：依次读 `bdx bdy bdz edx edy edz` 六个文件 → float6 的 `bx,by,bz`（原值）和 `ex,ey,ez`（×1e6，MV/m → V/m）。
- type 4（静磁）：读 `bsx bsy bsz` → `bx,by,bz`。
- **type 5（静电）及其他：直接返回 1（成功）而不读任何文件**，`field_cpu/gpu`、`nx…` 都是未初始化值（`esx/esy/esz` 字符串在库里根本不存在）。
- 文本头（每个分量文件都重读一遍，网格以最后一个文件为准，数组按第一个文件分配）：`"%d %lf"` → nz、zmax（dz = zmax/nz）；`"%d %lf %lf"` → nx、xmin、xmax；
  `"%d %lf %lf"` → ny、ymin、ymax；`"%lf"` → **归一化系数，读入后丢弃**（CPU 用 stoi 截断后除）。随后 (nx+1)(ny+1)(nz+1) 个 `%f`，
  循环顺序 z 外层、y 中层、x 内层，节点 `((ny+1)·iz + iy)·(nx+1) + ix`——与 CPU 相同，但**不判断存储顺序**（CPU 也只按文件顺序读；AVAS 的
  `group_order()` 是 Python 侧的事）。
- 二进制头：`int32 nz; double zmax; int32 nx; double xmin, xmax; int32 ny; double ymin, ymax; double norm（丢弃）`，随后逐个 `fread` float32（每个值一次 fread，慢）。
- 没有 CPU 的“一维场图展开成三维”（`.edx` 打不开时读 `.edz`）回退；打不开直接打印 `Reading ascii/binary field file "%s" error.` 并失败。
- 网格原样拷到 GPU（`cudaMemcpy`），float 精度。

### 2.6 动态误差：`InitializeErrorParameters`（0013ad60）

1. 先扫 `configs[4]`：`err_quad_dyn_on s1…s7` → 7 个 int 开关 A；`err_cav_dyn_on s1…s7` → 开关 B。多行时**最后一行有效**；少于 7 个值时 `strtol(NULL)` 崩溃。
2. 再扫 `configs[5]`：`err_quad_ncpl_dyn N r v1…v7` / `err_cav_ncpl_dyn N r v1…v7`（`r` 被跳过：**不抽样，值原样施加**）。从该行的 `componentIdx`
   起向后找，**只数类型匹配的元件**：quad 行只作用于 **type 4（静磁场图）**，cav 行只作用于 **type 6（RF 场图）**，跳过其他元件直到找满 N 个。
   硬边 quad / solenoid / bend 永远不受误差影响。
3. 每个目标元件：`errorAnalysis = 1`，`sw[0..6]` = 对应开关行，
   - quad：`errx = v1·1e-3`、`erry = v2·1e-3`、`errpx = v3·π/180`、`errpy = v4·π/180`、`errpz = v5·π/180`、`errkg = v6·0.01`、`errz = v7·1e-3`；
   - cav：`errx = v1·1e-3`、`erry = v2·1e-3`、`errpx = v3°`、`errpy = v4°`、`errkg = v5·0.01`、`errs = v6`（度，不换算）、`errz = v7·1e-3`（`errpz` 不写）。
4. 列含义与 CPU（70-errors-secondary.md §4）一致：quad 列 dx、dy、θx、θy、θz、dG(%)、dz；cav 列 dx、dy、θx、θy、dK(%)、φ(°)、dz。
   开关下标 `sw[k]` 对应第 k+1 列，在 `GetExternalField` 中作为**乘数**使用（`offset = sw[k]·err`），见 3.2。

与 CPU 的差别：GPU 只有动态（`*_dyn`）元件误差，没有 `*_stat`、`err_beam_*`、`err_step`、随机抽样（r 无效）、DynamicErrorData.txt；N 只数匹配类型；
quad 误差只作用于静磁场图（CPU 作用于 field3/quad/solenoid/bend/edge）。`configs[4]` 非空会把输出改到 `output_0/`（与 CPU 误差研究的目录习惯一致）。

### 2.7 叠加：`InitializeSuperpose`（0013b430）

lattice 写法：每个子元件前一行 `superpose z x y θx θy θz`（**必须 6 个数**，少了 `strtod(NULL)` 崩溃），组末一行 `superposeend`。

- 6 个数全为 0 且当前没有打开的组：以下一个元件为组首（`superpose=1`、`superposeStartIdx` = 自己），组终点 = 它的 zend。
- 否则（任一数非 0，或组已打开）：要求组已打开（否则 `Superpose error line %d : the superpose command without a superpose start command.`）、
  z 不小于上一行的 z（否则 `… is less than that of previous.`），目标元件 `zstart = 组首 zstart + z`、`zend = zstart + len`，组终点取最大 zend。
- **只用 z**；x、y 和三个角度只用来判断“是否全 0”，不平移也不旋转场（CPU 的 superpose 带 6 个偏移量，另有 `superposeout`）。
- 矩阵元件（type 1–3）不能叠加：`… cannot be used on matrix components.`；一组最多一个 RF：`… cannot set more than 1 electromagnetic field.`。
- `superposeend`：子元件数必须等于 superpose 行数（`… the number of superpose components %d is not equal to the number of set components %d.`）；
  组内全部元件写 `superposeEnd` 与 `superposeEndIdx`；**组后所有元件的 z 从组终点重新串接**；`latticeLength` 更新。缺 `superposeend` 时报
  `… missed a superpose end command.`。任何错误使函数返回 false（Python 退出 “Failed to initialize superpose”）。

---

## 3. 元件、外场与推进

### 3.1 两种坐标表示

- **real（实空间）**：所有粒子在同一时刻 t，坐标 (x, y, z, γβx, γβy, γβz)，用时间步推进。场图元件（type 0、4、5、6 和叠加组）在 real 表示下推进。
- **phase（相空间 / 穿越面记录）**：粒子穿过某个 z 面时记录的 (x, y, γβ, t_i)，存于 `phasePos/phaseVel/phaseTimeStamp`。用于：
  ① 矩阵元件（type 1–3）的整段传输；② 输出面记录；③ 末端 `outData` 输出。
- `ConvertNotMissedParticlesFromPhase2RealWithOffset(zend, t_ref)` 把 phase 记录换回 real：按示踪版 `ConvertTracerParticleFromPhase2Real`（00138670），
  `Δt = t_phase − t_ref`，`x = x_ph − c·γβx·Δt/γ`，`y` 同理，`z = zend − v_z·Δt`，`v_z = βc/sqrt(1 + x'² + y'²)`，动量 = 记录值，`phaseFlag = 0`。
  这与 CPU 的 `zTotcode`（40-loop-output.md §4.3）相同。`…WithoutOffset` 版只带一个参数（`timeStart`），用于矩阵元件中点求空间电荷（推测：不加 zend 偏移，在相位坐标里直接换算到平均时刻）。

### 3.2 外场：`GetExternalField`（0012bfe0，宿主版；核函数 `GetExtFieldForNotMissedParticles_Device` 推测相同）

签名 `GetExternalField(float3* B, float3* E, double3 pos, double t, LatticeComponents*, int idx, int type)`，结果**累加**到 B、E（叠加组逐个子元件调用）。
只处理 type > 3（场图）；type 0–3 在 real 表示下没有外场。

1. 局部坐标：`z_l = z − zstart`。若 `errorAnalysis`：`x −= sw0·errx`，`y −= sw1·erry`，`z_l −= sw6·errz`；type 4 先绕 z 转 `sw4·errpz`，type 6 不转 z；
   再绕 y 转 `sw3·errpy`、绕 x 转 `sw2·errpx`（具体旋转矩阵见伪代码，顺序 z→y→x）。
2. 越界（`x<minx`、`x>maxx`、`y<miny`、`y>maxy`、`z_l<0`、`z_l>maxz`）→ 不加任何场（**不判丢失**；横向范围是场图网格范围，不是孔径）。
3. 三线性插值（权重 `1 − |i − r|`，8 个角点），float 累加。
4. RF（type 6）：`φ = 2π·f·(t − time0) + (phi0 [+ sw5·errs])·π/180`；`B = Σw·F[0..2]·Kb·(1 + sw4·errkg)·sin φ`（代码写作 `cos(φ − π/2)`），
   `E = Σw·F[3..5]·Ke·(1 + sw4·errkg)·cos φ`（E 已 ×1e6）。与 AGENTS 第 6 节“射频电场 cos、磁场 sin”一致。
5. 非 RF（type 4，以及 type 5 若能读到）：`B = Σw·F[0..2]·Kb·(1 + sw5·errkg)`，**不计算 E**——即使静电场图被读入，也会被当成磁场乘 Kb（缺陷）。
6. 有误差时把场矢量按相反顺序旋回实验室系。

`GetExtFieldForTracerParticle`（0012d020）：示踪粒子 `locatedIdx` 有效时，非叠加元件 type > 3 调一次；叠加元件对 `superposeStartIdx..EndIdx` 中每个 type > 3 的子元件累加。

### 3.3 real 推进：Boris（`UpdateTracerParticle` 0012d430，推测 `UpdateNotMissedParticles_Device` 相同）

u = γβ（无量纲），E V/m，B T，q/m C/kg，k = (q/m)·dt/c：

- B 全为 0：`u += k·E`，`x += c·u·dt/γ`。
- 否则：`u⁻ = u + (k/2)·E`；`b = (q/m)·B·dt/γ⁻`（γ⁻ = sqrt(1+|u⁻|²)）；用半角矢量 b/2 的精确 Boris 旋转
  （`(1 − |b|²/4)…/(1 + |b|²/4)` 形式）；`u = u_rot + (k/2)·E`；`x += c·u·dt/γ`。
- `t += dt`。dt = 1/(f·stepPerCycle)（`RunSimulation*` 开头：`(1.0/beam.frequence)/stepPerCycle`）。

与 CPU `LeapFrog::Update`（40-loop-output.md §4.5）同一算法（推测逐式相同，未逐项核对乘法顺序）。

元件定位（示踪版，推测核函数同理）：

- z 后退：重新向上游查找所在元件；z < 0 → `status = 2`、`locatedIdx = −1`（向后丢失）。
- z ≥ lattice 末端（最后元件 zend）→ `status = 1`、`locatedIdx = n`，线性插值记录末端穿越面的 phase 坐标与时刻。
- 前进进入**非叠加的矩阵元件**（type 1–3）→ `phaseFlag = 1`，插值记录元件入口 zstart 处的 phase 坐标与时刻（粒子在入口“等待”）。
- 叠加组整体跳到 `superposeEndIdx`。

### 3.4 矩阵元件：相位推进（`UpdatePhaseTracerParticle` 00137b10，推测 `UpdateNotMissedPhaseParticles*_Device` 相同）

当示踪粒子已进入矩阵元件、且本 rank 全部未丢失粒子都 `phaseFlag==1`（`ReduceSumNotMissedParticlesPhaseFlag` == `numNotMissedParticles`），
各 rank 取 MPI min 得到 `mpi.phaseFlag = 1`，下一步整段传输该元件（以入口 phase 坐标为初值，x' = γβx/γβz）：

- solenoid（type 1）：`K = q·B/(2p)`（`B·(q/m)/c/(γ·2β)`），转角 KL，标准螺线管矩阵（x、x'、y、y' 耦合）；`t += L/v_z`。
- quad（type 2）：`k = |G·q/(m γ β c)|`，`G·q > 0` 时 x 聚焦（cos/sin）、y 散焦（cosh/sinh），否则相反；`t += L/v_z`；**G = 0 时除以 sqrt(0)**。
- **bend（type 3）：函数直接返回，坐标与时刻都不变**——随后在出口 zend 处按入口时刻换回 real，相当于粒子在零时间内跨过整段弯铁、不偏转（推测核函数相同）。
- 推进后：取全体 phase 时刻的最小值（MPI min）作为新的全局时间 `beam.timeStart`，`ConvertNotMissedParticlesFromPhase2RealWithOffset(zend)` 换回 real，
  `mpi.phaseFlag = 0`。整个元件只算**一步**迭代计数。

与 CPU 的差别：CPU 把连续的矩阵元件合成一个 z 段、元件间不回到 t 表示，并支持 edge、steerer、bend（HardEdgeTrans）；GPU 每个矩阵元件单独一次
“等全部粒子到齐 → 整段传输 → 换回 real”，没有 edge、steerer 与 bend 物理。

### 3.5 空间电荷下的矩阵元件

FFT / PICNIC 循环在 phase 模式下把元件分成两半（`len·0.5`）：

1. `UpdateNotMissedPhaseParticlesWithoutSpaceCharge(L/2)`；
2. 取全体 phase 时刻的平均（`ReduceAvgNotMissedPhaseParticlesTimeStamp`，MPI sum 后 ÷ commSize——是各 rank 平均值的平均）→ `beam.timeStart`；
   `ConvertNotMissedParticlesFromPhase2RealWithoutOffset`；
3. `CalculateBeamPICParameters` → `SetParametersForPIC` →（PICNIC：`CalculateCoefficientTableForPICGrid_Host`）→ 电荷分配 → 多 GPU 归约 → 求解 → 网格内 / 网格外插值；
4. `UpdateNotMissedPhaseParticlesWithSpaceCharge(L/2)`（推测：在中点施加一次空间电荷冲量后传输后半段）；
5. 示踪粒子整段 `UpdatePhaseTracerParticle`；取 phase 时刻最小值 → 换回 real。

与 CPU `zcodeRun` 的“中点分割 + HESCFieldPre + PIC”同一思路。

---

## 4. 相位扫描：`InitializePhase`（001342c0）

### 4.1 流程

- 输出 `outputPath/Phase.txt` 与 `outputPath/scanPhase.txt`（rank 0，先删除旧文件）。
- 扫相步长 `dt_s = (0.01/f)/stepPerCycle`（**主步长的 1/100**）。参考粒子 = `scanParticle`（平均能量，z = 0，t = 0，轴上）。
- 沿元件逐个推进（每次 `GetFieldForScanParticle`（00132d40）+ `UpdateScanParticle`（00132eb0，Boris，同 3.3）+ `t += dt_s`），每进入一个元件
  把 `time0` 记为到达入口的时刻（越过边界时线性插值）。`GetFieldForScanParticle` 只对 type > 3 求场（`GetExternalFieldOnce` 001326c0），
  所以矩阵元件对参考粒子是漂移。
- type 0、1、2、4 正常步进；type 6 见下；**非叠加的 type 3（bend）、type 5（静电）或未初始化的 type：`goto LAB_001343b1` 不推进也不前移元件序号，
  外层 `while (idx < n)` 以同一元件重复——死循环**（推测：lattice 里有 bend 或静电场图时 GPU 内核挂在扫相阶段）。
- RF（type 6）且 `phiScanned == 0`：
  1. 线程数 n = ⌊360/scanAngleStep⌋（默认 20），初值 `φ0_i = i·step − 180`（度）。
  2. `ScanPhaseThreadingStep1`（001336e0）：每个线程复制参考粒子，`CalculatePhiS(φ0_i)` 得 φs_i，并记下出口 γβz（`phiE[i]`）。
  3. `ScanPhaseThreadingStep2`（00133a90）：线程 i 检查目标 `phiS`（元件 0x30）是否落在 [φs_i, φs_{(i+1)%n}] 之间（处理 ±180° 折返：比较时给 φs 加 180），
     是则用线性插值（regula falsi）迭代 `CalculatePhiS`，直到 `|φs − 目标| ≤ 1e-6°`（**没有迭代次数上限**）；随后 `PhaseDetermination`（001337f0）
     在 φ0 ± δ（δ 从 1° 起减半至 1e-6°）比较出口 γβz，判断是否为可接受的根（推测：确认处于加速 / φs(φ0) 上升分支），结果写 `success[i]`。
  4. 在所有 `success` 的线程中取**出口 γβz 最大**的 φ0 写入元件 `phi0`，`phiScanned = 1`；scanPhase.txt 写 `"%0.15f %0.15f\n"`（phi0[度]，time0[s]）。
  5. 然后参考粒子以选定相位逐步穿过该腔（这次用 phi0），到 zend 时写 Phase.txt 一行。
- 叠加组含 RF：先把参考粒子推到 RF 子元件入口记 time0，再同样扫相；之后按组整体推进到 `superposeEnd`。
- 结束时 `cudaMemcpy` 把元件表（含 phi0、time0）拷到 GPU；返回参考粒子步数。

### 4.2 `CalculatePhiS`（00133420）

从入口开始以 dt_s 推进参考粒子穿过该 RF 元件，`φ = 2πf(t − time0) + φ0·π/180`，累加
`S = Σ cavityEz·sin φ·Δz`、`C = Σ cavityEz·cos φ·Δz`（`cavityEz` 是 `GetExternalFieldOnce` 算出的轴上 Ez 幅值 = Σw·F_ez·Ke，RF 只算 Ez，不算横向场和 B），
到 zend 时插值，返回 `atan((sin φ·Ez·z + S)/(z·Ez·cos φ + C))·180/π`。

- 用 `atan` 而非 `atan2`：结果只在 (−90°, 90°)，减速解与加速解（相差 180°）在 φs 上无法区分，由 Step2 的 +180 处理与“出口 γβz 最大”的选择间接区分。
  CPU 用 atan2，并在无加速解时接受 180° 解（AGENTS 第 6 节）；GPU 的行为在这些情形下与 CPU 不同（推测，需实验）。
- 最后一段的权重乘的是当前 z（`dVar8`）而不是剩余步长 Δz（疑似缺陷，可能是反编译把变量合并造成的误读，需反汇编核对）。

### 4.3 是否支持 scanphase / scanData.txt

**不支持**。库的字符串表中没有 `scanphase`、`scanData`；`LoadRunningOptions` 不认 `scanphase`。V3（word3）在解析时被跳过，**所有 RF 场图一律把 word6 当作同步相位
φs 去扫描**（等价于 CPU 的 V3 = 0）；CPU 的 V3 = 1（φ0 直接给定）、V3 = 2（t = 0 绝对相位）在 GPU 上都会被错误地当成 φs 扫描。

---

## 5. 输出文件

所有路径前缀 `outputPath/` 的由 rank 0（或各 rank 依次）写；标 “CWD” 的写在进程当前目录（缺陷）。

### 5.1 DataSet.txt（`OutputParticlesStatistics_Host` 00125270，rank 0，每 `statOutputInterval` 步一行，`"%0.15f"` 定点 15 位小数，空格分隔，41 列）

| 列 | 内容 |
|---|---|
| 0 | 平均动能（`CalculateNotMissedParticlesEz_Device` 的逐粒子量求和 / N × mc²；推测逐粒子量为 γ−1，即**能量的平均**，CPU 为由平均动量算的动能）；跨 rank 为各 rank 均值之平均 |
| 1, 3, 5 | ⟨x⟩−x_t、⟨y⟩−y_t、⟨z⟩−z_t（t = 示踪粒子） |
| 2, 4, 6 | ⟨γβx⟩、⟨γβy⟩、⟨γβz⟩ |
| 7–9 | α x、y、z（`emitAlpha`） |
| 10–12 | β x、y、z（`emitBeta`） |
| 13–15 | ε x、y、z（`emit`；归一化方式在宿主代码中被优化掉，见 `pow(1/γ, 3)` 的使用，推测与 CPU 相同：横向 ×βγ，纵向含 γ³） |
| 16, 18, 20 | rms x、y、z（`stdPos`） |
| 17, 19, 21 | rms x'、y'、z'（`stdVelSlope`，斜率定义在核函数里，推测 x' = γβx/γβz、z' = δ） |
| 22–27 | max x、max x'、max y、max y'、max z、max z'（`maxPos`、`maxVelSlope`；|偏离均值| 的最大值，MPI max） |
| 28 | 全局未丢失宏粒子数（int） |
| 29–34 | 示踪粒子 x、γβx、y、γβy、z、γβz |
| 35 | 0（int） |
| 36–38 | 0、0、0 |
| 39 | 当前迭代序号（int；矩阵元件整段只算一次） |
| 40 | 示踪粒子时刻 t（s） |

列布局与 CPU（40-loop-output.md §4.2.7）一致，所以 Python 的 DataSet 读取可以通用；差别：格式（定点 15 位 vs `%g`）、35–38 列恒为 0（CPU 是弯转标志、rotMode、同步粒子位移）、
第 0 列定义、统计用抽样（7.3）、各 rank 平均不按粒子数加权、行与推进的时间对齐（GPU 在推进之后统计并写同一步的行；CPU 第 k 行描述第 k−1 次推进后的状态）。
`statOutputInterval = 0` 时不写 DataSet。每次运行开始时删除旧文件。

### 5.2 PCHistogram.dat（同一函数，`histogramFlag` 非 0 时，rank 0，二进制追加）

`char 0; int32 iteration; int32 numPCHistogram; int32 × (4·numPCHistogram)（Histogram_Device 的 int4 数组，推测每个 bin 存 x、y、z、r 四个计数，按 bin 交错）;`
随后 12 个 double：`min,max,avg` 依次为 x（[0]）、y（[1]）、r（[3]）、z（[2]）。CPU 版首字节 0x7D、四个直方图顺序存放、rmin 固定为 0；GPU 首字节 0、r 的最小值实际计算。

### 5.3 BeamSet_p<rank>_t<k>.plt（`OutputBeamSetThreading` 00124550，`dumpPeriodicity ≠ 0`）

每个 rank 开 `numBufferPool` 个写盘线程，各写一个文件（运行开始时删除旧文件）。主循环每 `dumpPeriodicity` 步把本 rank 全部粒子（`Output{Real,Phase}ParticlesData2BufferPool`，
0x38 字节/粒子）异步拷进当前池的下一槽；池满时解锁、切到下一个池（`usleep(100)` 轮询），写盘线程用 `trylock` 取池、写完清计数。

```
文件头：char[2]（未初始化的栈字节）; int32 dumpPeriodicity; int32 N_rank; double current(A); double frequency(Hz); double kneticEnergy[0]
每条记录：char（未初始化）; int32 flag（0 = real，1 = phase）; int32 iteration; double t（示踪粒子 timeStamp 或 phaseTimeStamp）;
          double z（示踪 pos.z 或 phasePos.z）; N_rank × {posx, velx, posy, vely, posz, velz（double）, int32 status, int32 globalIdx}
```

real 记录（flag 0）是绝对坐标；phase 记录（flag 1）是矩阵元件出口的穿越面坐标。同一 rank 的记录按池轮转分散在多个文件中，需要按 iteration 合并。
CPU 的 BeamSet.plt 记录为 `char 0x7D, int mode, int step, double t, double zs` + 相对同步粒子的坐标；格式不兼容。

### 5.4 outData_<L>.dst（`OutputPhaseParticles` 00136a00）与 outputPlane_<z>.dst（`OutputPanelPhaseParticles_Host` 00126820）

TraceWin dst，各 rank 依次 `ab+` 追加（MPI_Barrier 轮流）：

```
char 0x7D; char 0x64; int32 N（全局）; double I(mA) = current·1000; double f(MHz) = frequence/1e6; char 0x7D;
N × {X = x·100 (cm), X' = γβx/γβz, Y = y·100, Y' = γβy/γβz, φ = 2πf·(t_i − t_ref) (rad), W = (γ−1)·mc² (MeV)}; double mc²（最后一个 rank 写）
```

- outData：坐标取 `phasePos/phaseVel/phaseTimeStamp`（末端穿越面记录）中的前 `numNotMissedParticles` 个，`t_ref` = 示踪粒子末端穿越时刻（`phaseTimeStamp`）——与 CPU dstOut 一致。
  文件名 `outData_%f.dst`，`%f` = latticeLength。
- outputPlane_<panelz %f>.dst：每个输出面一个文件（运行结束时统一写），`t_ref` = **该面全部有效粒子穿越时刻的平均值**（CPU 用同步粒子穿越时刻），
  有效粒子数由 `MarkingValidParticles_Device` 统计、写前 N 个槽位（依赖 `ModifyPanelParticlesSequence` 的重排保证有效记录在前，推测）。
  打印 `outputPlane %d has %d particles.`。**某个面 0 个粒子时函数直接 return，后面的面都不写**（缺陷）。
- 输出面来自 `outputplane z` 行（只读第二个词）；z 落在 solenoid / quad（type 1、2）内部时移到该元件出口（bend 不处理）。CPU 的输出面文件也叫 `outData_<z>.dst`，GPU 改名为 `outputPlane_<z>.dst`。

### 5.5 Phase.txt、scanPhase.txt、scanResult.dat

- `Phase.txt`（rank 0）：参考粒子每离开一个 RF 腔（或叠加组内的 RF）写一行 `"%d %0.15f %f %0.15f\n"`：腔序号、时刻（s）、z（m）、第 4 个 `%0.15f`
  **没有对应实参**（打印寄存器残值，缺陷）。各分支传入的时刻 / z 是出口还是入口不完全一致（伪代码参数顺序被优化，需反汇编核对）。CPU 有同名文件，格式不同。
- `scanPhase.txt`（rank 0）：每个扫描的腔一行 `phi0(度) time0(s)`。CPU 写的是 `scanPhaseData.txt`。
- `scanResult.dat`（**CWD**，只在 `main` 中调用）：各 rank 依次追加 `rank = %d:`，再对每个 RF 元件一行 `phi0 time0`。

### 5.6 死代码 / 调试输出（无调用者，均在 CWD 或调用方给的路径）

`OutputRealParticles`（001361a0，Python 注释掉的 `real.dat`：首行 N，表头 `x y z vx vy vz timeStamp`，每粒子 7 个 `%0.15e` + status + phaseFlag）、
`OutputInitializeRealParticles`（`initialize_particles.dat`）、`OutputSpaceChargeFieldUsingPICNIC`（`intElecFieldUsingPICNIC.dat`）、
`OutputSpaceChargeFieldUsingFFT`（`intElecFieldUsingFFT.dat`、`fft.dat`；fprintf 的格式与实参类型错位，打印的 %d 是浮点寄存器之外的垃圾）、
`ReducePICGridChargeAmongGPUsWithRingAllreduce`、`TransformTracerParticlePosition`。

GPU 没有 CPU 的 synData.txt、synParticle.txt、SimulationTime.txt、LongitudinalBeamLoss.txt、FullParticle.txt、ParticleTracking.txt、ErrorLog.txt。
标准输出：每 1000 步 rank 0 打印 `iteration steps = %d, particles average z-position = %e, completed ratio = %f%%`（z 取 `CalculateBeamPICParameters` 的平均 z，
比例 = ⟨z⟩/latticeLength）。

---

## 6. 示踪粒子与 panel 粒子

- **tracer（示踪粒子）**：`Beam.tracerParticle`，即 φ = 0、平均能量、轴上的参考粒子（2.3），在主循环里与束团同步推进（宿主 CPU 上单独算：
  `GetExtFieldForTracerParticle` + `UpdateTracerParticle`；矩阵元件用 `UpdatePhaseTracerParticle` + `ConvertTracerParticleFromPhase2Real`）。作用：
  ① 判断何时进入矩阵元件（`phaseFlag`）；② 主循环结束条件之一（`status == 1` 到达末端）；③ DataSet 的参考坐标（列 1–6 的差值、29–34、40）；
  ④ BeamSet 记录的 t、z；⑤ outData 的相位参考时刻。它相当于 CPU 的同步粒子 `particles[N]`，但 CPU 同步粒子用 beam.txt 的能量，GPU 用 dst 的平均能量。
- **scan particle**：`Beam.scanParticle`，只在 `InitializePhase` 中用（第 4 节）。
- **panel 粒子**：每个输出面 `OutputPanels` 为本 rank 的每个粒子槽位存一份穿越该面的 phase 坐标。核函数 `UpdateNotMissedParticles_Device(lattice, particles, panels,
  dt, numComponents, numPanels)` 在粒子越过 `panels[panelIdx].panelz` 时插值写入并把 `panelIdx` 加一（推测，函数签名带 `OutputPanels*`）。
  粒子数组因丢失压缩（`ModifyParticlesSequence_UpdateParticlesNumber_Host`）或按网格序号重排（`ArrangeNotMissedParticlesWithPICIndex_Host`）而改变顺序时，
  对“正在被填写”的那些面（`ReduceMaxMinNotMissedParticlesPanelIndex_Host` 求出的 [minPanelIdx, maxPanelIdx)）调 `ModifyPanelParticlesSequence_Host`
  （0011e540）按同一置换重排 panel 数组（核函数重排到 CUB 备份数组后交换指针）。运行结束时 `OutputPanelPhaseParticles_Host` 写出（5.4）。

---

## 7. 损失、统计与抽样

### 7.1 粒子状态与损失

- 计数：`numStatusChanged`（本步离开“正常”状态的数目：到达末端或丢失）从 `numNormalParticles` 中减去；`numStatusMissed`（本步丢失数）从
  `numNotMissedParticles` 中减去；`ModifyParticlesSequence_Device` 用 CUB 把仍有效的粒子压到数组前部（`ModifyParticlesSequence_UpdateParticlesNumber_Host` 0011eb80）。
  推测：到达末端的粒子仍计入 “NotMissed”，停在末端不再推进，因而后期的统计混有已到达末端的粒子（需实验）。
- 丢失判据**只在核函数里**，宿主侧看不到：示踪版只有“z 后退且 < 0 → status 2”；`GetExternalField` 越出场图网格只是不加场，不判丢失；
  元件 `rad` 在宿主代码中从未被读取。核函数是否按 `rad` 判孔径损失、是否有纵向损失（CPU 的 longlimits）都无法从伪代码确认（需实验或反汇编 SASS）。
- 主循环结束条件：全局 `numNormalParticles < 1` 且示踪粒子 `status == 1`（`RunSimulation*` 循环头）。若示踪粒子永远到不了末端（例如被 bend 的零时间处理或场图错误卡住），可能不结束（推测）。

### 7.2 BeamStatistics（0x1c0，偏移 / 含义）

`pcHistogram_cpu/gpu`（0x0/0x8）、`avgPos`（0x10）、`avgVel`（0x28，γβ）、`avgVelSlope`（0x40）、`varPos`（0x58）、`varVelSlope`（0x70）、`stdPos`（0x88）、
`stdVelSlope`（0xa0）、`varCross`（0xb8）、`avgEz`（0xd0，平均动能 MeV）、`emit`（0xd8）、`emitAlpha`（0xf0）、`emitBeta`（0x108）、`maxVelSlope`（0x120）、
`maxPos`（0x138）、`minHis/maxHis/avgHis`（0x150/0x170/0x190，各 4 个：x、y、z、r）、`beta`（0x1b0，β = ⟨γβz⟩/sqrt(1 + |⟨γβ⟩|²)）、`gamma`（0x1b8，γ = 1/sqrt(1 − β²)，由平均动量求得，见 8.1）。

- `OutputParticlesStatistics_Host`：平均 → 最大偏离（MPI max）→ 斜率平均 → 斜率绝对最大（`absMax` 归约，MPI max）→ 方差与交叉项
  （`SamplingNotMissedParticlesVariance_Device`，以均值为中心）→ 开方得 rms，求 α、β、ε（公式在被优化的代码里，推测同 CPU：ε = sqrt(⟨x²⟩⟨x'²⟩−⟨xx'⟩²)，α = −⟨xx'⟩/ε，β = ⟨x²⟩/ε）。
  所有跨 rank 的均值都是“各 rank 均值之和 ÷ commSize”，**不按各 rank 的粒子数加权**。
- `CalculateBeamPICParameters`（0012f230）：每步（除了上一步刚写过 DataSet 的那步，那时沿用其统计）求 ⟨pos⟩、⟨vel⟩、varPos、stdPos，供 PIC 网格与进度打印用；
  方差核函数传入 γ，纵向按 γ(z − ⟨z⟩) 求静止系方差（8.1）。

### 7.3 抽样（`numSamplingCoef` 与 `Sampling*` 核函数）

- DataSet / 直方图统计：抽样数 `M = (int)(numSamplingCoef · numNotMissed)`，步长 `N/M`，各 `Sampling*_Device` 把抽中的粒子的量写进缓冲再用 CUB 归约
  （`SamplingNotMissedParticlesPosAndVel/MaxDistance/VelSlope/Variance/Pos`）。默认 1.0 即全体。
- PIC 网格参数（`CalculateNotMissedParticlesAveragePosAndVelWithSampling_Host` 00114310、`…PosVarianceWithSampling_Host` 001144a0）与输出面序号范围
  （`ReduceMaxMinNotMissedParticlesPanelIndex_Host` 0011e960）：**固定最多 100000 个**（`numNotMissed ≤ 100000` 时全体，否则等间隔抽 100000 个，常数 0x186a0）。

### 7.4 空间电荷循环（求解器见第 8 节）

- `SetParametersForPIC`（00112490）：每个方向 `宽 = 2·σ·meshrms`，`d = 宽/(n−1)`，`min = ⟨pos⟩ − 宽/2`，`max = ⟨pos⟩ + 宽/2`（n 为 `InitializePIC` 取整后的网格数）。
  即网格覆盖均值 ± meshrms·σ，随束流每步自适应。网格外的粒子由 `GetIntFieldForNotMissedParticlesWithoutPICGrid_Device(particles, BeamStatistics, charge)`
  单独处理（全局在网格内的数 < 未丢失数时才调用）。
- `RunSimulationWithSpaceChargeUsingFFT`（00130770）每个 real 步：清零网格（流 1）→ `CalculateBeamPICParameters`（同 7.2 的条件）→ 每 1000 步打印 →
  `SetParametersForPIC` → 每 `arrangeSequenceInterval` 步且 `usingAggregation` 时 `ArrangeNotMissedParticlesWithPICIndex_Host` 并重排 panel →
  `AccumulateNotMissedParticlesCharge2PICGrid_Host` → `GetExtFieldForNotMissedParticles_Host`（流 1）与示踪外场 → commSize > 1 时
  `ReducePICGridChargeAmongGPUsWithButterflyAllreduce` → `SolveSpaceChargeWithFFTSolver_Host` → `GetIntFieldForNotMissedParticelsWithinPICGrid_Host`
  （返回网格内粒子数，MPI sum）→ 必要时 `…WithoutPICGrid_Host` → `cudaDeviceSynchronize` → `UpdateNotMissedParticles_Host` + `UpdateTracerParticle` →
  panel 序号范围 → 丢失压缩与 panel 重排 → phase 模式判定（MPI min）→ MPI sum 正常数、未丢失数 → DataSet / 直方图 → BeamSet。phase 模式见 3.5。
- `RunSimulationWithSpaceChargeUsingPICNIC`（001316f0）结构相同，只是电荷分配前多一步 `CalculateCoefficientTableForPICGrid_Host`，求解换成
  `SolveSpaceChargeWithPICNICSolver_Host(pic, cub, ro, mpi, streams)`（内部区分 PICNIC / S2 / S4，见 8.4、8.5）；`spaceChargeMethod == 1` 打印
  `running simulation with space charge using PICNIC[1]`，否则 `SPICNIC[%d]`（参数缺失，打印垃圾）。
- **空间电荷每个 real 步都求一次**，没有“每 N 步求一次”的选项；矩阵元件内在中点求一次。
- `RunSimulationWithoutSpaceCharge`（0012fbb0）：同上去掉网格与求解步骤，phase 模式下整段一次 `UpdateNotMissedPhaseParticlesWithoutSpaceCharge`。

---

## 8. 空间电荷

依据：宿主侧伪代码（`SolveSpaceChargeWithPICNICSolver_Host` 00122f90、`InitializePIC` 0011baf0、`SetParametersForPIC` 00112490、
`CalculateBeamPICParameters` 0012f230、`CalculateNotMissedParticlesPosVarianceWithSampling_Host` 001144a0、`AccumulateNotMissedParticlesCharge2PICGrid_Host` 00113d00、
`InitializerFFT` 0011c910、`SolveSpaceChargeWithFFTSolver_Host` 0011d2a0），核函数的 SASS（`cuobjdump -sass`，常量取自 `.nv.constant2.*` 段），
以及第 9 节的实测：**按本节写的模型重算的 PICNIC 场与原库实测的场相差 1e-4（rms，float 存储的量级）**，所以本节对 PICNIC 的描述已经核实。
S2 / S4 的标记细节、FFT 的系数只读了结构，标“推测”。

### 8.1 网格（PICNIC 与 FFT 共用）

- `InitializePIC`：`numofgrid a b c` **向上取到 8 的倍数**作为网格点数 nx、ny、nz（核函数按 8 的倍数分块）；`numCopies = 4`（电荷副本数，见 8.2）；
  `cutoffInt[i] = round(0.75·N_i) + 1`、`cutoffIntRadius = ceil((max_i(N_i/2) + 1)²)`（N_i 为**取整前**的值）；系数表数组 `cudaMemset` 为 0。
- 每步 `CalculateBeamPICParameters`：⟨pos⟩、⟨γβ⟩ 与方差，最多抽 100000 个粒子（7.3），多 rank 时为各 rank 结果的**不加权平均**。
  `CalculateNotMissedParticlesPosVarianceWithSampling_Host` 先由 ⟨γβ⟩ 求 β = ⟨γβz⟩/sqrt(1 + |⟨γβ⟩|²)、**γ = 1/sqrt(1 − β²)**，写进
  `BeamStatistics.beta`（0x1b0）与 `BeamStatistics.gamma`（0x1b8）——存的是 γ 本身；纵向方差按 γ(z − ⟨z⟩) 计算，即**束流静止系**的 rms。
- `SetParametersForPIC`：每个方向 `宽 = 2·σ·meshrms`，`d = 宽/(n − 1)`，`min = ⟨pos⟩ − 宽/2`，`max = ⟨pos⟩ + 宽/2`，节点 `min + i·d`。
  这与 AVAS.dll 的 SPICNIC 网格（60-space-charge.md §2.3，节点 `(i − n/2 + 1/2)·d + 中心`）是同一个网格；差别只在 n 取到 8 的倍数、统计来自抽样。
- 粒子的静止系纵坐标 `z' = ⟨z⟩ + γ(z − ⟨z⟩)`（电荷分配与场插值核函数都由 `DADD z, −⟨z⟩; DFMA ·γ + ⟨z⟩` 计算，⟨z⟩ 与 γ 是核函数参数）。

### 8.2 电荷分配

`AccumulateNotMissedParticlesCharge2PICGrid_Device(PIC, Particles, ⟨z⟩, γ, blocksPerGroup, groupSize)`（`usingaggregation 1` 时为 `…WithWarpAggregation`）：

- 任一坐标 `< min` 或 `> max` 的粒子**不分配**（由 8.6 的“网格外”核函数单独处理）；
- CIC：`f = (x − min)/d`，`i = floor(f)`，八个角点权重 `(1 − f̃)` / `f̃` 的乘积（无量纲，未除体积）；
- 用**双精度原子加**（`RED.E.ADD.F64`）写进 `numCopies = 4` 份网格副本之一，再由 `AccumulatePICGridChargeCopiesWithMarkingNone_Device(PIC, q)` 把 4 份相加、乘宏粒子电荷 q（C）。
  原子加的次序不确定，所以**同一输入重复运行结果不逐位相同**；
- `usingaggregation 1`（默认）时每 `arrangesequenceinterval`（默认 15）步按网格序号重排粒子（CUB 排序），warp 内同一单元的粒子先合并再原子加；
- 多 rank：`ReducePICGridChargeAmongGPUsWithButterflyAllreduce` 把各 rank 的网格电荷（double）蝶式全归约，求和次序随 rank 数变化。

### 8.3 系数表（格林函数）

`CalculateCoefficientTableForPICGrid_Device<<<(c0 − 1, c1 − 1, c2 − 1), 8×8×8>>>`（c = cutoffInt，每步重算，因为 d 每步变化）：一个块算一个表项 (bx, by, bz)，表存于
`gridCoef[(bz·ny + by)·nx + bx]`：

- (0, 0, 0)：块直接退出，**自身单元为 0**；
- `bx² + by² + bz² > cutoffIntRadius`：点值 `1/sqrt((bx·dx)² + (by·dy)² + (bz·dz)²)`；
- 否则：单元内 8×8×8 个中点上 1/r 的平均（每个线程一个中点，warp shuffle 再块内求和）；
- 偏移 ≥ c − 1（≈ 0.75 N）的表项保持 0，即**核函数在每个方向截断在约 0.75 N 个单元**。

与 AVAS.dll SPICNIC 的表（60 §2.7.2）相比：同样的半径、同样的截断、同样的自身单元 0，只是中点为 8³（DLL 为 10³）。

### 8.4 PICNIC 求解（scmethod picnic）

`SolveSpaceChargeWithPICNICSolver_Host`，`spaceChargeMethod`（RunningOptions+0x1c）为 1 时：

1. `DeviceSegmentedReduce` 统计每一行（固定 y、z 的一行 x）的非零电荷数（`noneZeroLine`）；
2. `PICNIC_Device<<<(1, ny, nz/commSize), nx, nx·40 字节共享内存>>>(PIC, rank·nz/commSize)`：线程 = 目标点（x = threadIdx，y = blockIdx.y，z = blockIdx.z + 偏移）；
   对 `|Δz| < c2`、`|Δy| < c1` 的每一行源（跳过空行），把该行电荷和对应的系数行读进共享内存，累加 `Σ_x coef[|Δx|]·ρ`；结果乘 1/(4πε0) = 8.98755179e9
   （`.nv.constant2._Z13PICNIC_Device3PICi`）写进 `gridPotential`。网格按 z 平面分给各 MPI rank（nz 须能被 rank 数整除，推测）；
3. 多 rank 时 `ReducePICNICAmongGPUsWithButterflyAllreduce` 合并电势；
4. `InterpolateElectricField_Device<<<ceil(n/8)³, 8³>>>(PIC, 1/(2dx), 1/(2dy), 1/(2dz))`：内部节点中心差分 `E_x = (φ[i−1] − φ[i+1])/(2dx)`，
   **边界节点不计算（保持 0）**，结果存为 **float3**。

求和是直接卷积，计算量 ∝（非零源）×（目标点），没有用对称性。

### 8.5 S2PICNIC / S4PICNIC（scmethod s2picnic / s4picnic，`spicnic` 是 s4picnic 的别名）

- method 2：`Q2MarkingAsymmetricGridCharge_Device<<<(1, ny/2, nz), nx>>>` 标出关于 y 中心面不对称的电荷，`DeviceReduce` 计数；
  method 3：`Q4MarkingAsymmetricGridCharge_Device<<<(1, ny/2, nz), nx/2>>>`，关于 x、y 两个中心面。
- 不对称单元数 ≥ nx·ny·nz/50 时**退回完整 PICNIC**；否则只对半个（S2）或四分之一（S4）截面求和（`S2PICNIC_Device`，共享内存 nx·56 字节；
  S4 在 nx/2 ≤ 32 时用 `WarpShuffleS4PICNIC_Device`，否则 `S4PICNIC_Device`），再镜像；
- 然后 `SelectAsymmetricGridCharge_Host`（多 rank 时蝶式归约）挑出不对称电荷，`CorrectAsymmetricGridCharge_Device<<<…, 8×8×8>>>` 把它们的贡献加到电势上。
- 标记的判据与修正的细节没有完全读出（推测与 AVAS.dll SPICNIC 的非对称修正同一思路）。**实测（9.2）：S2 / S4 的场与 PICNIC 相差 0.5–0.7 %（rms），出口统计与 PICNIC 在种子起伏以内。**
- 日志：method 1 打印 `running simulation with space charge using PICNIC[1].`，其他打印 `SPICNIC[%d]`（参数缺失，打印垃圾值）。

### 8.6 场插值与推进

- `GetIntFieldForNotMissedParticlesWithinPICGrid_Device(PIC, Particles, ⟨z⟩, γ)`：在 (x, y, z') 处三线性插值（双精度权重 × float 场），
  **E_x、E_y 除以 γ，E_z 不变**（静止系场换到实验室并计入磁力），存进粒子的 float3 `intElecField`；返回网格内粒子数（MPI sum）。
- 网格内粒子数小于未丢失数时 `GetIntFieldForNotMissedParticlesWithoutPICGrid_Device(Particles, BeamStatistics, q)`：**点电荷场**，
  以束流中心为原点、总电荷为网格上分配到的电荷，静止系计算后横向除以 γ（与 AVAS.dll 的网格外处理相同；实测相差 2e-3）。
- 推进（3.3）：`u += (q/m)·dt/c·(E_ext + E_int)`（无磁场时）或 Boris。**每个实空间时间步都求一次**；矩阵元件在中点求一次（3.5）。

### 8.7 FFT 求解器（scmethod fft）与 AVAS.dll 的差别

- `InitializerFFT`：`cufftPlanMany(3 维, n = {nz, ny, nx}, CUFFT_C2C)`——**单精度复数**（`cufftComplex`，每元素 8 字节），网格不补零；
  另分配三个长度 nx、ny、nz 的 double 系数数组（`CalculatePoissonCoef_Device`）。
- 每步：`PrepareGridChargeForFFTSolver_Device` 把电荷写成复数 → 正变换 → `SolvePoissonWithFFTSolver_Device` 乘以系数（含 1/ε0 = 1.1294e11）→ 逆变换 →
  `InterpolateElectricField_Device(FFTSolver, PIC, …)` 中心差分 → 8.6 的插值。
- 因此 libPIC 的 FFT 在 **x、y、z 三个方向都是周期边界**，单精度。AVAS.dll 的 FFT 是 x、y 方向 DST-I（接地导体壁）+ z 方向周期，双精度（FFTW）。
  两者都不是开放边界；在 meshrms 4 的网格上，libPIC FFT 的纵向场只有 PICNIC 的约 0.57 倍（9.2），出口 rms x 小 5 %、rms z 大 10 %。

### 8.8 与新内核的关系

新内核的 `scmethod PICNIC`（M6b，见 M6-notes）采用同一个网格（8.1，静止系、均值 ± meshrms·σ）、同一个 CIC 分配和点电荷外场，
修正了以下几处（第 9 节逐项量出了它们的影响）：

| libPIC.so | 新内核 PICNIC（默认模式） |
|---|---|
| 系数表：8³ 中点平均、自身单元 0、半径外点值 | 每个表项是单元上 1/r 的**精确平均**（解析原函数，与修正后的 SPICNIC 同一张表） |
| 核函数截断在约 0.75 N 个单元 | 覆盖整个网格 |
| 边界节点的场为 0 | 边界节点用单侧差分（与 SPICNIC 相同） |
| 场存为 float | 双精度 |
| 双精度原子加，结果不可重复；统计抽样 100000 个；rank 间不加权平均 | 定点整数分配、固定次序求和，与线程数、GPU 数、MPI 进程数无关；统计用全部粒子 |
| numofgrid 取到 8 的倍数；没有默认网格 | 网格点数按输入；不写时用 SPICNIC 的默认网格（60 §2.1.3） |

---

## 9. 实测（A100，2026-11-08）

### 9.1 方法

- 机器：A100-PCIE-40GB（sm_80），`module load cuda/11.4`（cudart 11、cufft 10），用户态 OpenMPI 4.1.6（`~/avas_engine_work/toolchains/openmpi4`，见 M6-notes）。
  libPIC.so 只有 sm_80 的 SASS、没有 PTX，只能在 sm_8x 上运行（cuobjdump 确认）。
- 驱动：ctypes 调库导出的 `main(argc, argv)`（参数 `common= beam= lattice= gpu=0`），单进程 singleton MPI。srun 作业里要先清掉 `SLURM_*`、`PMI*` 环境变量，
  否则 OpenMPI 以为是 srun 直接启动而退出；OpenMPI 要设 `OMPI_MCA_plm=isolated`、`OMPI_MCA_btl=self,vader`。
- 束流：同一份 TraceWin dst，质子 3 MeV、162.5 MHz、30 mA、1e5 个宏粒子，六维高斯（σx = σy = 1.5 mm、σx′ = σy′ = 1.5 mrad、σφ = 15°、σW = 5 keV），
  5 个种子。lattice 只有漂移段；`steppercycle 100`、`numofgrid 32 32 32`、`meshrms 4 4 4`（两个内核写法相同）。
- **网格场**：`dumpperiodicity 1` 让 libPIC 每步写出全部粒子（BeamSet_p0_t0.plt）。漂移段里没有外场，推进是 `u += (q/m)·dt/c·E_int`，
  所以相邻两条记录的 γβ 之差直接给出每个粒子受的空间电荷场。对同一批粒子（按 globalIdx 对齐）再用三种方法算场：按第 8 节写的 libPIC 模型、
  新内核的 PICNIC / SPICNIC / FFT（同样的 ⟨pos⟩、σ、γ）。
- **统计**：1 m 漂移，比较两个内核的 DataSet 末行（5 个种子）与束团完全进入 lattice 后（第 100–716 行）rms x′、z′ 的增长。

### 9.2 网格场（第 5 步，99963 个粒子在网格内）

libPIC PICNIC 实测场的 rms：E_x 2.52e4、E_y 2.52e4、E_z 1.67e4 V/m。表中为相对 rms 差与最小二乘斜率（A/B）。

| A 对 B | x | y | z | 斜率 x / y / z |
|---|---|---|---|---|
| 第 8 节模型 对 libPIC picnic | 1.0e-4 | 1.0e-4 | 2.5e-4 | 0.9999 / 0.9999 / 0.9999 |
| libPIC s2picnic 对 模型（PICNIC） | 5.3e-3 | 4.6e-3 | 5.7e-3 | — |
| libPIC s4picnic 对 模型（PICNIC） | 6.3e-3 | 6.1e-3 | 7.2e-3 | — |
| 新内核 PICNIC 对 libPIC picnic | 3.7e-2 | 3.7e-2 | 1.3e-2 | **1.033 / 1.033 / 1.012** |
| 新内核 SPICNIC 对 libPIC picnic | 5.0e-2 | 5.7e-2 | 4.0e-2 | 1.027 / 1.036 / 1.016 |
| 新内核 FFT（接地壁，meshrms 4）对 libPIC picnic | 6.9e-2 | 6.8e-2 | 0.24 | 1.065 / 1.064 / 0.777 |
| libPIC fft 对 libPIC picnic | 0.12 | 0.12 | 0.44 | 1.09 / 1.09 / 0.565 |
| 网格外 16 个粒子：libPIC 对 点电荷模型 | 2.0e-3 | | | |

新内核 PICNIC 与 libPIC 的差别逐项分解（在模型里一次只改一处）：

| 改动 | 与 libPIC 的相对 rms 差 x / z | 最大差 / 最大场 x |
|---|---|---|
| 系数表换成精确的单元平均（含自身单元） | 3.6e-2 / 1.3e-2 | 5.1e-2 |
| 去掉 0.75 N 截断 | 2.4e-3 / 3.3e-4 | 5.0e-2 |
| 边界节点改用单侧差分 | 8.9e-3 / 2.8e-4 | 0.51（只在网格边界附近） |
| 三处都改 | 3.7e-2 / 1.3e-2；与新内核 PICNIC 相差 3e-8（float 存储） | |

即：**新内核 PICNIC 与原库的差别完全来自已查明的三处修正**，主要是系数表（libPIC 的自身单元为 0、近场用 8³ 中点，使横向场弱约 3.3 %、纵向弱约 1.2 %）。

### 9.3 统计（5 个种子，1 m 漂移出口，均值 ± 种子间标准差）

| | rms x（mm） | rms z（mm） | ε x（π mm mrad，归一化 rms） | ε z | 传输 |
|---|---|---|---|---|---|
| libPIC picnic | 3.6444 ± 0.0042 | 7.2982 ± 0.0092 | 0.21907 ± 0.00066 | 0.5660 ± 0.0032 | 100 % |
| libPIC s4picnic | 3.6443 ± 0.0043 | 7.2988 ± 0.0094 | 0.21907 ± 0.00067 | 0.5662 ± 0.0032 | 100 % |
| libPIC fft | 3.4634 ± 0.0036 | 8.0409 ± 0.012 | 0.21526 ± 0.00064 | 0.6680 ± 0.0047 | 100 % |
| 新内核 PICNIC | 3.6092 ± 0.0035 | 7.2490 ± 0.0081 | 0.22025 ± 0.00067 | 0.5643 ± 0.0032 | 100 % |
| 新内核 SPICNIC | 3.5962 ± 0.0047 | 7.2433 ± 0.011 | 0.22074 ± 0.00069 | 0.5705 ± 0.0053 | 100 % |
| 新内核 FFT | 3.6611 ± 0.0036 | 7.0007 ± 0.0079 | 0.22241 ± 0.00067 | 0.5392 ± 0.0028 | 100 % |

同种子差（新内核 PICNIC − libPIC picnic）：rms x −0.97 % ± 0.14 %、rms z −0.68 % ± 0.06 %、ε x +0.54 % ± 0.01 %，超出种子起伏。原因有两个，都已查明：

1. **束团进入 lattice 时的约定不同**：libPIC 把 dst 粒子放在入口上游（z ≤ 0），从第一步起就对整个束团计空间电荷；新内核沿用 AVAS.dll，
   束团进入 lattice 的约 20 步里 rms x′ 不变（新内核的 SPICNIC、FFT 也一样）。这一段使 libPIC 的 x′ 先多长了约 0.03 mrad，并一直带到出口。
2. **场强**：束团完全进入后（第 100–716 行），rms x′ 的增长 libPIC 为 1.7401 ± 0.0022 mrad，新内核 PICNIC 为 1.7969 ± 0.0015 mrad（**+3.3 %**），
   rms z′ 为 1.2451 ± 0.0019 对 1.2534 ± 0.0022（+0.7 %），与 9.2 的场强比 1.033 / 1.012 一致。

libPIC 的 s4picnic 与 picnic 在种子起伏以内一致；libPIC 的 fft 因三维周期边界与 PICNIC 相差 5–18 %（8.7）。

### 9.4 其他观察

- 到达末端的粒子 status = 1，之后从数组前部压出（BeamSet 记录的粒子顺序每步变化，需要按 globalIdx 对齐）。
- `field_s2picnic` 一次运行在 `InitializePhase` 里 `fclose(NULL)` 段错误，重跑正常（偶发，推测是扫相线程与文件句柄的竞争）。
- 性能：1e5 粒子、32³ 网格，libPIC 的 PICNIC 约 1.2 ms/步、FFT 约 0.5 ms/步（`pushing cost time`）；新内核 PICNIC 在同一 A100 上约 4 ms/步（含全部输出），见 M6-notes。

### 9.5 未做的实验

含 RF 场图、硬边 quad / solenoid、bend、静电场图、输出面、多 rank 的 libPIC 运行都没有做（本段只需要空间电荷的参考）。第 3–7 节中标“推测”的推进与损失细节仍未实测；
建议的实验：drift + RF 场图比较 DataSet；bend 是否让扫相死循环；硬边 quad 的相位推进与 G = 0；孔径损失；V3 = 1 的腔；没有粒子到达的输出面。

---

## 10. 缺陷与可疑处

| # | 位置 | 问题 | 依据 / 状态 |
|---|---|---|---|
| 1 | `LoadRunningOptions` | `stepPerCycle`、`numGridofPIC`、`rmsSize` 没有默认值（`malloc` 垃圾）；不写 `steppercycle` 时 dt 是垃圾 | 确定 |
| 2 | 同上 | `beamPath` 默认 `"field"`；`fieldPath`/`outputPath` 默认相对当前目录 | 确定 |
| 3 | 全部关键字解析 | 缺值时 `strtod/strtol(NULL)` 段错误（superpose 少于 6 个数、`kneticenergy` 只有 1 个值、`numofgrid` 少于 3 个、误差开关少于 7 个…） | 确定 |
| 4 | `ReadConfigurations` | 行长 > 255 字符被截成多行 | 确定 |
| 5 | `InitializeLatticeComponents` | field 行 `t` 为 0 或 4 时 `type` 未初始化；V3 被忽略 | 确定 |
| 6 | `ReadASCIIField/ReadBinaryField` | 静电场图（type 5）返回成功但不读文件，网格与数组指针未初始化；`GetExternalField` 对非 RF 只算 B（乘 Kb），即便读入也会把电场当磁场 | 确定 |
| 7 | 场图读取 | 归一化系数读入后丢弃（CPU 除以 norm）；没有一维场图回退 | 确定 |
| 8 | bend | 只读 L、R；real 表示无场，phase 推进不处理 type 3、时间不增加（零时间跨越、不偏转）；`InitializePhase` 遇非叠加 bend 死循环 | 确定（宿主示踪版 + 扫相），核函数行为推测 |
| 9 | `InitializePhase` | 非叠加的 type 3 / 5 / 垃圾类型 → 不前进的死循环 | 确定（控制流） |
| 10 | quad | G = 0 时相位推进除以 0（NaN）；CPU 报错 | 示踪版确定 |
| 11 | `CalculatePhiS` | `atan` 代替 `atan2`；最后一段权重用 z 而非 Δz（疑似） | 前者确定，后者需反汇编 |
| 12 | `ScanPhaseThreadingStep2` | regula falsi 无迭代上限，可能不收敛时死循环 | 推测 |
| 13 | `CheckWhetherDeviceAssigned` | argc ≥ 2 且无 `gpu=` 时返回未初始化寄存器值（main 路径） | 确定 |
| 14 | `OutputPanelPhaseParticles_Host` | 某个输出面 0 粒子时 `return`，后续面全部不写 | 确定 |
| 15 | `ReduceMaxMinNotMissedParticlesPanelIndex_Host` | N > 100000 时只抽 100000 个求 panel 序号范围，可能漏掉需要重排的面，panel 记录与粒子错位 | 推测 |
| 16 | 统计 | 跨 rank 平均不按粒子数加权（DataSet 与 PIC 网格参数都是）；DataSet 第 0 列是能量平均（推测）而非 CPU 的平均动量动能 | 确定（宿主部分） |
| 17 | `Phase.txt` | 4 个格式符 3 个实参 | 确定 |
| 18 | `OutputBeamSetThreading` | 文件头 2 字节与每条记录首字节写未初始化的栈变量 | 确定 |
| 19 | `InitializeBeam` | 没有 `readparticledistribution` 时返回空束流，随后使用未初始化内存；beam.txt 的 current/frequency/rest mass/particlenumber 被 dst 覆盖、twiss/distribution/displace 无效 | 确定 |
| 20 | `InitializeErrorParameters` | 误差 N 只数匹配类型；quad 误差只作用静磁场图；r 被忽略、不抽样 | 确定 |
| 21 | `InitializeSuperpose` | 横向偏移和转角被忽略；组的第一行必须 6 个 0 | 确定 |
| 22 | PICNIC 日志 | `SPICNIC[%d]` 缺参数 | 确定 |
| 23 | `OutputSpaceChargeField*` | fprintf 格式与实参类型错位（死代码） | 确定 |
| 24 | `scanResult.dat`、调试文件 | 写到当前工作目录而非 outputPath | 确定 |
| 25 | `structures.py` | `SyncParticle` 的外场字段应为 float3（结构 0xc0），`Beam` 0x4a0，`ScanPhaseThreadArgs` 0x108；Python 目前不访问受影响字段 | 确定 |
| 26 | `pic.py` | `SetDeviceForProcessors.argtypes` 只声明 1 个参数却传 2 个（ctypes 允许，按默认转换传 int）；`MPIContext` 把 try/except 写在类体里，except 分支引用 `self` 不可用 | 确定（Python） |
| 27 | 主循环 | 进入矩阵元件要等本 rank **全部**未丢失粒子 `phaseFlag==1`；若有粒子因故永远到不了入口（例如倒退），会一直停在 real 模式（推测） | 推测 |
| 28 | `InitializeOutputObject` | 输出面不排序；落在 bend 内的面不移动 | 确定（排序需求为推测） |
| 29 | PICNIC 系数表 | 自身单元为 0、近场 8³ 中点、半径外点值：横向场弱约 3.3 %、纵向弱约 1.2 %（9.2） | 实测 |
| 30 | PICNIC 求和 | 核函数在每个方向截断在约 0.75 N 个单元，长束团的远处电荷被丢掉 | 实测（本例影响 0.2 %） |
| 31 | 场插值 | 网格边界节点的场为 0（不做单侧差分），网格边缘附近的粒子场偏小（最大差 50 %） | 实测 |
| 32 | 场的精度 | 网格场与粒子场都存为 float | 确定（SASS） |
| 33 | 电荷分配 | 双精度原子加，同一输入重复运行不逐位相同；多 rank 的蝶式归约次序随 rank 数变化 | 确定（SASS） |
| 34 | FFT 求解器 | cuFFT 三维 C2C 单精度、三个方向都是周期边界（不补零），与开放边界相差很大（9.2） | 确定（宿主）+ 实测 |
| 35 | 网格 | `numofgrid` 被向上取到 8 的倍数（用户写 30 得到 32），截断与半径却按取整前的值 | 确定 |
| 36 | `InitializePhase` | 偶发 `fclose(NULL)` 段错误（s2picnic 的一次运行，重跑正常） | 实测一次 |

---

## 11. 功能对照

### 11.1 libPIC.so 有、AVAS.dll 没有

| 功能 | 说明 |
|---|---|
| GPU（CUDA sm_80）+ MPI 多卡 | 粒子按 rank 均分，网格电荷 butterfly allreduce（ring 版存在但未用），统计 MPI 归约；命令行 `gpu=N` 指定设备 |
| 空间电荷求解器选项 | `scmethod fft / picnic / s2picnic / s4picnic(spicnic)`（CPU 只有 FFT / SPICNIC） |
| 电荷分配优化 | `usingaggregation`（warp 聚合 + 按网格序号排序）、`arrangesequenceinterval`、`blockgroupsize` |
| 统计抽样 | `numsamplingcoef`；PIC 参数固定最多抽 100000 个 |
| 异步 BeamSet 输出 | `numbufferpool`、`bufferpoolsize`，每 rank 多个 `BeamSet_p<r>_t<k>.plt`，含 real / phase 两种记录 |
| `statoutputinterval` | DataSet 每 N 步写一行 |
| 路径关键字 | `fieldpath`、`outputpath`、`beampath`（写在 input 文件里） |
| 扫相参数 | `scananglestep`（初值间隔），多线程扫相，收敛到 1e-6°；`scanPhase.txt`、`scanResult.dat` |
| 输出面单独成文件 | `outputPlane_<z>.dst`（相位参考为平均穿越时刻） |
| 资源检查 | panel 显存检查、BeamSet 预计 > 50 GB 时拒绝运行（main 路径） |
| 场图文件 | 大小写不敏感的文件名查找；`B` 结尾也按二进制 |
| 直方图 bin 数默认 300 | `pchistogram F N` |

### 11.2 AVAS.dll 有、libPIC.so 没有

| 功能 | 说明 |
|---|---|
| 按 twiss 生成初始分布 | KV / WB / PB / GS、`randomseed`、`beamtype dc`、`displacepos/displacedpos`、`beamlength`、`bunchnumber`；GPU 只能读 dst |
| beam.txt 的 current / frequency / 能量 | GPU 一律取自 dst 文件 |
| 元件 | `steerer`、`edge`、`multipole`、`rfq`、**bend 物理**、**静电场图**、一维场图、场图归一化系数、`automaticoutput` |
| RF 相位模式 | V3 = 1（直接给 φ0）、V3 = 2（绝对相位）；无加速解时的 180° 解与拒绝运行的判据（GPU 用 atan，行为不同） |
| 叠加 | 横向偏移 / 转角、`superposeout`、superpose 块内的误差行编码 |
| 误差 | 静态误差、束流误差、`err_step`、随机抽样（r）、`*_stat_on`、硬边元件误差、DynamicErrorData.txt |
| 其他物理 / 模式 | boundary.txt 几何边界、`lpboundary`、二次粒子（secondarybeam / SeParticle.txt）、单粒子模式、`longlimits` 纵向损失、`probe`、`track`、空间电荷场导入（recScField / impScField）、`integralgrid`、`negligibleratio`、`scutoffdistance`、OpenMP 多线程（multithreading / setnumberofcores） |
| 输出 | synData.txt、synParticle.txt、scanPhaseData.txt、SimulationTime.txt、LongitudinalBeamLoss.txt、FullParticle.txt、ParticleTracking.txt、SingleParticle.txt、ErrorLog.txt、`.dstb` / `.edst`、DataSet 35–38 列 |
| 健壮性 | 未知关键字警告、参数个数检查、quad G = 0 报错、首元件不得为硬边的检查 |

---

### 11.3 libPIC.so 有、而新内核没有的功能（建议）

新内核的功能集是 AVAS.dll 的全部功能（含修正）加 M6a 的 GPU / MPI。下表逐项列出 libPIC.so 独有的功能。除 PICNIC 外本段都没有实现，是否实现由用户决定。

| # | 功能 | 建议 | 理由 |
|---|---|---|---|
| 1 | `scmethod picnic`（直接求和的空间电荷） | **已实现**（M6b） | 用户要求；修正了系数表、截断、边界与精度（8.8） |
| 2 | `scmethod s2picnic` / `s4picnic`（`spicnic` 在 libPIC 里是 s4picnic 的别名） | 不实现，写这两个值时报错并提示改用 PICNIC | 只是 PICNIC 的加速近似（场差 0.5–0.7 %）；新内核 PICNIC 在 A100 上 1e7 粒子每步只比 SPICNIC 多约 2 ms。`spicnic` 在新内核里继续表示 AVAS.dll 的 SPICNIC |
| 3 | libPIC 的 FFT（三维周期、单精度） | 不实现 | 是缺陷而不是功能（8.7）；新内核保留 AVAS.dll 的 FFT |
| 4 | GPU + MPI 多卡 | 已有（M6a） | — |
| 5 | `usingaggregation`、`arrangesequenceinterval`、`blockgroupsize` | 不实现；读到时警告并忽略 | 是原子加电荷分配的调优参数；新内核的定点分配不需要 |
| 6 | `numsamplingcoef`（DataSet 统计抽样）与 PIC 参数固定抽 100000 个 | 不实现 | 抽样使统计和网格依赖抽样方式；新内核在 GPU 上用全部粒子的固定次序归约，代价很小 |
| 7 | 异步 BeamSet 缓冲池（`numbufferpool`、`bufferpoolsize`，每 rank 多个 `BeamSet_p<r>_t<k>.plt`，含 phase 记录） | 不实现 | 新内核写 AVAS.dll 格式的 BeamSet.plt，GUI 与后处理都按它读 |
| 8 | `statoutputinterval N`（每 N 步写一行 DataSet） | **待定，推荐实现** | 长 lattice、大步数时减小输出；实现简单，默认 1 保持现状 |
| 9 | `fieldpath` / `outputpath` / `beampath` 写在 input 文件里 | 不实现 | 新内核由 `path()` 接口给出路径，GUI 已管理 |
| 10 | `scananglestep` 与多线程扫相（收敛到 1e-6°） | 不实现 | 新内核按 20-synchronous-phase.md 定相（atan2、180° 解与拒绝运行的判据），比 libPIC 的 atan 正确 |
| 11 | 输出面写成 `outputPlane_<z>.dst`、相位参考取平均穿越时刻 | 不实现 | 新内核沿用 AVAS.dll 的 `outData_<z>.dst`（参考为同步粒子） |
| 12 | panel 显存检查、BeamSet 预计超过 50 GB 时拒绝运行 | 待定，推荐只实现 BeamSet 大小的预估与警告 | 新内核已按粒子数检查显存（M6a）；输出过大是常见误操作 |
| 13 | 场图文件名大小写不敏感查找；名字以 `B` 结尾也按二进制读 | 待定，推荐不实现 | 会改变 AVAS.dll 在 Linux 上的行为；Windows 文件系统本来就不区分大小写 |
| 14 | `scmethod` 的值不区分大小写 | 待定，推荐默认模式下 FFT / SPICNIC 也不区分 | 无害；PICNIC 已按 libPIC 不区分 |
| 15 | 命令行 `gpu=N` | 已有等价功能 | 选项 `gpus` / 环境变量 `AVAS_ENGINE_GPUS`（M6a） |
| 16 | `pchistogram F N` 的 bin 数默认 300 | 不需要 | AVAS.dll 的 pchistogram 已有 bin 数参数 |

libPIC.so 缺少的功能（bend 物理、静电场图、V3 = 1 / 2、twiss 生成分布、静态误差、二次粒子、boundary.txt 等，见 11.2）新内核都有，不受影响。
