# 新内核：随源码提供三个平台的预编译库

`avas/engine/new/` 现在包含 CI 构建并校验过的新内核库：Windows（`avas_engine.dll`，CPU + NVIDIA GPU）、Linux（`libavas_engine.so`，CPU + NVIDIA GPU，CentOS 7 起可用）、macOS（`libavas_engine.dylib`，Apple 芯片与 Intel，CPU + Mac GPU）。源码安装不必再自己编译新内核，`build-stamp.json` 记录对应的引擎源码、校验值和 CI 运行。

这批库所在的 CI 运行里：Windows 版在兼容模式下与原 AVAS.dll 逐字节一致（少数输出与参考文件末位不同，在同一台机器上重跑 AVAS.dll 后与新内核完全相同，差别来自系统数学库）；Linux、macOS 与参考结果的差别在 1e-10 以内。原内核仍是 Windows 和 Linux 上的默认选择。
