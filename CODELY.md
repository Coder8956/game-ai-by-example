

## Codely Structured Memories

### User

### Feedback

### Project
- [2026-09-24 18:08:46] 仓库为《Programming Game AI by Example》配套源码，全部是 VC6/VS2008 旧工程（.dsw/.dsp/.vcproj）。用户环境：VS 2026 Community 18.9.x（C:\Program Files\Microsoft Visual Studio\18\Community，工具集 v145）。2026-09-24 已按此模式完成 WestWorld1 修复与清理：最终目录仅含 9 个源文件 + .sln + .vcxproj + .filters；旧工程文件（.dsw/.dsp/.vcproj/UpgradeLog.htm/Backup/）已按用户要求删除（git 中标记为 D）。转换要点：新建 .vcxproj/.filters（保持原 ProjectGuid、Win32 Debug/Release、Console 子系统、MultiByte、/MTd 与 /MT），AdditionalIncludeDirectories 必须=..\..\Common（源码 #include "misc/ConsoleUtils.h" 依赖此路径，旧工程从未设置过），.sln 改引 .vcxproj，源码零改动。注意：v145 构建时中间文件默认生成在嵌套的 WestWorld1\WestWorld1\<Config>\（obj/tlog），exe 生成在 WestWorld1\<Config>\，属正常行为。仓库根目录没有 .gitignore，若继续转换其他章节工程建议先添加（忽略 Debug/、Release/、.vs/）。其余章节工程（WestWorldWithMessaging、WestWorldWithWoman、Chapter3+ 等）仍是旧格式，可按同一模式转换清理。

### Reference

