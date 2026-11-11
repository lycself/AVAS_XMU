# 新内核 CI：修正 Windows 构建与 macOS 作业

- 源码哈希脚本在 CMake 3.x 的脚本模式下不认识 `IN_LIST`，Windows 作业因此在构建第一步失败；脚本里显式启用该语法（CMake 3.31 下哈希与 Python 计算一致）。
- GitHub 已不提供 `macos-14` 镜像，macOS 作业改用 `macos-15`。
- 第一次 CI 中 Linux 作业全部通过：单元测试、只导出 C 接口、glibc ≤ 2.17、与 AVAS.dll 参考在 1e-10 以内、emu fp64 / fp32 共 184 个算例。
