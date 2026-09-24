---
name: restoration-project
description: 将老版本 Visual Studio 工程（VC6 的 .dsw/.dsp、VS2008 的 .vcproj）修复为可在 Visual Studio 2026 (18.x，工具集 v145) 中正常启动、构建、运行的干净工程。当用户要求"修复工程"、"把工程升级/转换到 VS 2026"、"老工程打不开"、"清理旧版本工程内容"、"整理干净的 VS 工程"时使用。四项原则：源代码零改动、设置忠实映射、构建+试运行双重验证、清理全部旧版本遗留。
---

# 老工程修复（restoration-project）

目标：交付一个满足以下四项要求的干净 VS 2026 工程：

1. 工程可在 Visual Studio 2026 (18.x) 正常启动、构建、运行
2. 源代码逻辑功能完全不变（源码零改动）
3. 清理 VS 2026 不需要的所有旧版本内容
4. 交付功能不变的干净工程

## 核心原则

- **源代码零改动**：禁止修改 .cpp/.h 的任何逻辑。遇到编译错误优先用工程级手段修复：AdditionalIncludeDirectories、预处理器定义、CharacterSet、LanguageStandard、警告设置。穷尽后仍无法编译时，向用户说明并确认，不得擅自改源码。
- **设置忠实映射**：新 .vcxproj 的 ProjectGUID、配置名、运行库、子系统、字符集从旧工程原样映射（映射表见 [references/vs-migration-reference.md](references/vs-migration-reference.md)），不擅自"现代化"（如 /MT 不得改 /MD）。
- **构建+运行双验证后才算完成**：MSBuild 零错误 + 试运行行为与原程序一致，两者缺一不可。
- **删除前确认可恢复**：git 仓库中被跟踪的旧工程文件删除后可从历史恢复；删除 .vs 前必须确认 devenv.exe 未运行。

## 工作流程

### 阶段 1：调研

1. 列出工程目录，识别旧工程文件（.dsw/.dsp = VC6；.vcproj = VS2008，由 .sln 引用；UpgradeLog.htm、Backup/ = 旧迁移残留）。
2. 读旧工程文件，提取并记录：ProjectGUID、配置列表（通常 Win32 Debug/Release）、RuntimeLibrary/SubSystem/CharacterSet/输出目录、源文件与头文件清单、已有包含路径。
3. 通读全部源文件的 #include，找出引用工程外部的头（如 `#include "misc/ConsoleUtils.h"`），推导需要的包含目录（本仓库惯例：`..\..\Common`）。注意：老工程常常**从未配置过包含路径**，这是旧工程编译失败的最常见原因。
4. 用 git status / git ls-files 区分"被跟踪的旧工程文件（删除可恢复）"与"未跟踪的构建产物"。

### 阶段 2：创建 .vcxproj 与 .vcxproj.filters

1. 把 [assets/template.vcxproj](assets/template.vcxproj) 与 [assets/template.vcxproj.filters](assets/template.vcxproj.filters) 的内容复制到工程目录，文件名为 `<工程名>.vcxproj` 与 `<工程名>.vcxproj.filters`。
2. 按模板内注释替换占位符：ProjectGUID（与 .sln 保持一致）、根命名空间、源/头文件清单、包含路径。
3. 特殊设置查映射表：RuntimeLibrary 1→MultiThreadedDebug、SubSystem 1→Console、CharacterSet 2→MultiByte、ConfigurationType 1→Application。

### 阶段 3：更新 .sln

只改 Project 行的文件名，GUID 与其余内容一律不动：

```
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "工程名", "工程名.vcxproj", "{原ProjectGUID}"
```

### 阶段 4：构建验证

两个配置都要编译，命令与环境细节见 [references/vs-migration-reference.md](references/vs-migration-reference.md)：

```powershell
$vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath
& "$vs\MSBuild\Current\Bin\MSBuild.exe" "<sln路径>" /p:Configuration=Debug /p:Platform=Win32 /m /v:m /nologo
```

要求：Exit Code 0、零错误；出现警告需记录并向用户报告。

### 阶段 5：运行验证（证明逻辑功能不变）

启动 Debug 构建的 exe，重定向 stdout 到临时文件，检查输出与程序应有关键业务行为一致：

- **先读 main() 算准等待时长**（主循环次数 × Sleep 毫秒 ÷ 1000 + 2 秒）。std::cout 重定向到文件时是全缓冲，只有程序跑完主循环遇到 std::endl 才落盘，等太短会读到"空文件"（不代表程序坏了）。
- 程序完整跑完后卡在按键等待属预期（_kbhit 对重定向 stdin 恒返回 0），Stop-Process 结束即可。
- 完整命令模板与陷阱见 references 文档。

### 阶段 6：清理

逐项删除并向用户说明原因：

| 删除对象 | 原因 |
|---|---|
| *.dsw、*.dsp | VC6 工程文件 |
| *.vcproj | VS2008 工程文件 |
| UpgradeLog.htm | 旧迁移日志 |
| Backup/ | 旧升级备份 |
| .vs/ | IDE 缓存（确认 devenv 未运行才可删；重开 VS 自动重建） |
| Debug/、Release/、嵌套中间目录 | 构建产物与中间文件 |

### 阶段 7：终态验证与交付

1. 清理后重新构建 Debug/Release 并再运行一次，证明干净工程依然完整可编译、功能不变。
2. 删除本次验证生成的构建产物（Debug/、Release/、嵌套中间目录）。
3. 展示最终目录清单（应只含：源文件 + .sln + .vcxproj + .filters）与 git status（旧文件 D、.sln M、新工程文件未跟踪）。
4. 告知用户：VS 2026 构建时会自动重新生成 Debug/Release 与中间目录（正常行为）；建议仓库添加 .gitignore（忽略 Debug/、Release/、.vs/）。

## 资源索引

- `assets/template.vcxproj`、`assets/template.vcxproj.filters`：VS 2026 工程模板（v145、Win32 Debug/Release、Console 子系统，含占位注释）。
- `references/vs-migration-reference.md`：老工程设置→vcxproj 映射表、VS 2026 工具链细节、构建/试运行命令详解、常见编译问题与工程级修复手段。
