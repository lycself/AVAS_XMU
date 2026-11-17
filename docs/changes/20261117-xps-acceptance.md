# 更正原内核遇到矩形管、椭圆管时的提示；Windows 真机验收

• 选用原内核或“复现 AVAS.dll”模式、boundary.txt 写了矩形管（3）或椭圆管（4）时，运行前的提示原来说原内核会报 “Unknown geometric shapes”。在 Windows 上核对 AVAS.dll 的实际输出，它报的是 “Undefined shape exists, check boundary.txt”，提示文字已改正（拒绝运行的行为不变）。2026-11-17 的界面变更记录沿用了旧说法，以本条为准。
• 内部：在 Windows 11（GTX 1650）上用正式发布的源码包和便携包验收了单粒子偏移、纵向周期边界、单粒子输出面结果、“仅新内核”标签与运行前检查、AI 助手的内核查询；束流页写出的单粒子偏移在原内核、新内核 CPU 与 CUDA 上得到相同的轨迹。
