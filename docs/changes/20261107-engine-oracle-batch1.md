• 内部维护：新增原内核比对实验工具 `engine/tools/oracle/` 和第一批实验记录 `docs/engine/90-experiments.md`，软件功能不变。
• 实测确认原内核的几处行为：input.txt 的 `scanphase` 与 scanData.txt 不起作用；`scmethod PICNIC` 会报错，默认方法是 SPICNIC；`secondarybeam 1` 时不读 SeParticle.txt；superpose 块中含 RF 腔时不加速；静态束流误差会使内核崩溃；固定种子时同一平台的运行结果完全可重复。
