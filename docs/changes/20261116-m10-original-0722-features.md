# 新内核默认模式加入原内核 2026-07-22 版的新功能

• 新内核默认模式读入 .dst 粒子文件时，同步粒子能量取 beam.txt 的 kneticenergy（写负值仍取文件平均能量）；两者相差超过 1 % 时运行前、运行日志和线性预览都会提示。兼容模式与原内核不变。
• 单粒子偏移可写 initpos（mm）与 initmom（mrad、Δp/p 单位 1e-3），与旧的 displacepos／displacedpos 同时写时以新的为准；单粒子运行另写 SingleParticlePlane.txt，每个输出面一行。
• 输出面可以命名：outputplane d NAME 写成 NAME_<z>.dst；间距小于束团长度的输出面可以同时记录，不再报 “The output plane interval is less than the bunch length”。
• boundary.txt 支持矩形管（3）、椭圆管（4）和多个子形状的 superpose 复合段；一步越过的薄准直器也会判碰撞；CollisionData.txt 每行末尾多一列 superposeout 块数。模拟设置页的边界表格提示了新形状。
• Ke = 0 的射频 field 行按漂移段处理（不读场图、不占腔号），线性预览和分段运行同样处理。
• input.txt 的 lpboundary 0 1 使用纵向周期空间电荷盒（束流静止系一个射频周期），适合连续束和长束团，CPU 与 GPU 都支持。
