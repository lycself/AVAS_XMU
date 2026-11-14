# 新内核：更新三个平台的预编译库（兼容较旧 VC++ 运行库的构建）

`avas/engine/new/` 换成包含 Windows 运行库兼容改动的新构建（CI 构建并校验）。Windows 版在兼容模式下仍与原 AVAS.dll 逐字节一致，Linux、macOS 版的计算与上一版相同。
