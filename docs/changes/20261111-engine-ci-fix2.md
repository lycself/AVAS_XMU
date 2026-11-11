# 新内核 CI：Windows 导出符号检查允许 CUDA 运行库的独显开关

Windows 版新内核静态链接 CUDA 运行库，库里自带一个导出 `NvOptimusEnablementCuda`：在集显加 NVIDIA 独显的笔记本上，它让驱动为 AVAS 启用独显（使用 NVIDIA GPU 时需要）。导出符号检查现在把它列为允许项，其余仍只能是 C 接口。
