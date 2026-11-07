• 内部维护：开始重建计算内核（原内核源码已丢失）。新增 `engine/` 目录，包含 Linux 构建环境（manylinux2014 + CUDA 12.4）、工具链检查程序和 Ghidra 伪代码导出脚本；暂不影响软件的任何功能，仍使用原内核运行。
• 已验证同一套编译方式生成的程序可以在 Pascal（GTX 1080 Ti）、Turing（GTX 1650，Windows）、Ampere（A100，CentOS 7 集群）和 Ada（L40S）显卡上运行，且不需要另外安装 CUDA。
