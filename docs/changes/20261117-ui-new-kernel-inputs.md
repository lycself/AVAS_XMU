# 界面：单粒子偏移、纵向周期边界、单粒子输出面结果，以及只有新内核读取的输入的标注与检查

• 束流页：粒子数可以填 1（单粒子模式，原来会被当作错误拦下），并显示“单粒子初始偏移”：x、y、z（mm），x′、y′（mrad），δ（1e-3）。文件里有 `initpos`／`initmom` 时先读它们；保存时统一写成各内核都能读取的 `displacepos`／`displacedpos`（displacedpos 的数值是 mrad 和 1e-3 的 100 倍），并去掉 `initpos`／`initmom` 行。
• 模拟设置页：空间电荷分组新增“纵向周期边界”：`lpboundary` V1（粒子折叠到一个射频周期）和 V2（周期空间电荷盒，仅新内核默认模式）。
• 结果页：单粒子运行（新内核默认模式写 SingleParticlePlane.txt）新增“单粒子在各输出面”，可画位置、角度或能量沿 z 的变化，下方附数据表，也能导出图片。
• 只有新内核默认模式读取的输入（输出面名称、lpboundary V2、initpos／initmom、nplanes）在模拟设置页、结构页参数栏和使用说明的参数参考中标注“仅新内核”；项目选用原内核或“复现 AVAS.dll”模式时改为黄色的“当前内核会忽略”，输入照常保留。
• 选用原内核或“复现 AVAS.dll”模式时：boundary.txt 中的矩形管（3）、椭圆管（4）在运行前直接提示改用新内核默认模式（原内核会报 “Unknown geometric shapes” 退出）；initpos／initmom、lpboundary V2、带名称的输出面、Ke = 0 的射频场在运行前提醒（命令行 `avas run` 相同）。
• boundary 表格的类型提示补充了 superpose 复合段的写法。
• 修正：lattice 中“射频腔位于叠加块开头”的运行前提醒，行号原来比编辑器显示的少 1。
