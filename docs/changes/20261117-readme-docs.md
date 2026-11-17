# 文档：README 改版为中英文两个文件并配截图；开发文档按内核重建完成后的状态更新

• README 重写：顶部有图标、徽章和语言切换，配主截图与四张功能截图；新增“在 CPU 和 GPU 上计算”一节（各平台的安装方式、驱动要求、支持的显卡架构、命令示例）和“GPU 没有被使用？”问答；细节较多的内容收进可展开的折叠块。中文在 README.md，英文移到 README.en.md。
• AGENTS.md、engine/README.md、docs/engine/README.md 不再写“新内核重建中、不合并”，改为已完成并成为默认内核的状态；AGENTS.md 新增“各平台怎样用 CPU／GPU 计算”一节，供 AI 编程助手回答用户问题。CONTRIBUTING.md 补充合并流程和截图工具。
• 新增 tools/readme_screenshots.py：自动建演示项目、运行并截取 README 用的界面截图。
• 软件功能没有变化。
