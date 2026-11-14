# 测试：AI 试算暂停 / 停止的测试在 Windows 原内核下不再偶尔失败

`test_sandbox_study_shows_on_the_run_page_and_can_be_paused_and_stopped` 要在第一个取值运行中途看到进度，再暂停、继续、停止。原内核跑示例只要约 4 秒，又每 3 秒才报告一次进度，测试常常看不到进度就跑完了（main 上同样会偶尔失败）。现在原内核用 6 万个粒子、新内核用 150 万个粒子，保证有足够的运行时间；Windows 11 上连续 5 次通过。软件行为不变。
