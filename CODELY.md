

## Codely Structured Memories

### User

### Feedback

### Project
- [2026-09-24 18:34:05] 仓库为《Programming Game AI by Example》配套源码，全部是 VC6/VS2008 旧工程（.dsw/.dsp/.vcproj）。用户环境：VS 2026 Community 18.9.x（C:\Program Files\Microsoft Visual Studio\18\Community，工具集 v145）。2026-09-24 已按同一模式完成 WestWorld1 与 WestWorldWithMessaging 修复清理：最终目录仅含源文件 + .sln + .vcxproj + .filters；旧工程文件（.dsw/.dsp/.vcproj/.plg）删除（git 标记 D）。转换要点：新建 .vcxproj/.filters（保持原 ProjectGuid、Win32 Debug/Release、Console、MultiByte、/MTd 与 /MT），AdditionalIncludeDirectories 必须=..\..\Common（旧工程从未设置过；Common\FSM\StateMachine.h 的 "Messaging/Telegram.h" 也依赖此路径），源码零改动。关键：VS2026 MSBuild 无法解析 "Format Version 9.00" 旧 .sln（报 MSB4025 缺少根元素），必须把 .sln 头升级为 Format Version 12.00 + VisualStudioVersion = 18.9.12120.119 stable + MinimumVisualStudioVersion 行（Project GUID 与配置映射不动，可新增 SolutionGuid）。旧工程引用的 ..\..\Common\2D\SVector2D.h、..\..\Common\ConsoleUtils.h、..\..\Common\utils.h 是磁盘上不存在的幽灵文件，新工程清单只列实际存在的。v145 中间目录：工程名短时为 <工程目录>\<工程名>\<Config>\，长时用截断名+GUID（如 WestWorl.0E2C3421）；exe 在 <工程目录>\<Config>\，均正常。仓库根目录没有 .gitignore，建议添加（忽略 Debug/、Release/、.vs/）。其余章节（WestWorldWithWoman、Chapter3+ 等）仍待转换。


### Reference

