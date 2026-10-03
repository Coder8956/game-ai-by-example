

## Codely Structured Memories

### User

### Feedback
- [2026-10-03 17:22:24] 用户为《Programming Game AI by Example》源码添加注释的规范已固化为项目级 skill「add-code-comments」（2026-09-24 已安装到 .codely-cli/skills/，用户要求注释任何工程代码时激活并遵循它）。规范：①零基础中文注释、逐词详解（#include、enum、virtual=0、单例等都要解释）、每个文件头部说明自身作用+与其他文件的包含/调用关系（含 ASCII 关系图）；②严禁改动源码，只准新增独立注释行，用 git diff --numstat 校验"0 删除"确保纯增量；③写完用 skill 自带 scripts/normalize-verify.ps1 归一化编码与行尾并按 git HEAD blob 对齐末尾换行数（行尾由脚本逐文件按 git 检测，并非固定 CRLF——2026-10-03 WestWorldWithWoman 全部 15 个文件实测均为 UTF-8 BOM + LF）（write_file 产出是 UTF-8 无 BOM 的 LF；.ps1 脚本本身也必须带 UTF-8 BOM，否则 PowerShell 5.1 按 GBK 误读中文报语法错误）；MSBuild Debug+Release 编译需 0 错误 0 警告。Common\ 目录是全书共享文件，未经要求不改动，仅在章节文件注释中解释其作用。风格基准：WestWorld1 的 10 个已注释文件。
- [2026-10-03 17:22:28] 为旧工程源码添加注释的实操陷阱清单（2026-10-03 WestWorldWithWoman 15 文件注释实测，后续章节注释任务必看）：①动笔前先用 git cat-file -p "HEAD:<路径>" 输出配合正则（行尾空格：-match ' $'；纯空格行：-match '^\s+$'）扫描原版全部隐形空白行并记录行号，逐行精确复刻（实例：Miner.h 初始化列表行有 41 个尾随空格、MinersWife.h 构造函数后有 70 空格行、多处 "  "/"    " 行——read_file 的显示无法可靠计数，靠此法可免多轮返工）；②原作者英文文件头说明块必须原样保留、中文翻译块附在其后——本次曾在 4 个文件中误把原文块替换成翻译块导致 VERIFY FAIL，是最高频失误；③原文空注释框（//------\n//\n空行\n//------）中间有一个空行，容易漏；④原文分隔注释（//------methods for...）后原文空行紧跟分隔线，中文注释应放在该空行之后（WestWorld1 范式：分隔线+空行+中文注释+空行+下一段）；⑤replace 工具会把 new_string 中行尾空格规范化剥掉，含行尾空格/纯空格的行无法用它写入——须用 PowerShell 按字节修补，最优做法是直接把 git HEAD blob 的整行内容拷进目标行；⑥每个文件写完立即跑 normalize-verify.ps1 单文件校验，FAIL 时用 git diff -U0 | rg "^-" 定位删除行再修。

### Project
- [2026-10-03 17:22:30] 仓库为《Programming Game AI by Example》配套源码，全部是 VC6/VS2008 旧工程（.dsw/.dsp/.vcproj）。用户环境：VS 2026 Community 18.9.x（C:\Program Files\Microsoft Visual Studio\18\Community，工具集 v145）。2026-09-24 已按同一模式完成 WestWorld1 与 WestWorldWithMessaging 修复清理：最终目录仅含源文件 + .sln + .vcxproj + .filters；旧工程文件（.dsw/.dsp/.vcproj/.plg）删除（git 标记 D）。转换要点：新建 .vcxproj/.filters（保持原 ProjectGuid、Win32 Debug/Release、Console、MultiByte、/MTd 与 /MT），AdditionalIncludeDirectories 必须=..\..\Common（旧工程从未设置过；Common\FSM\StateMachine.h 的 "Messaging/Telegram.h" 也依赖此路径），源码零改动。关键：VS2026 MSBuild 无法解析 "Format Version 9.00" 旧 .sln（报 MSB4025 缺少根元素），必须把 .sln 头升级为 Format Version 12.00 + VisualStudioVersion = 18.9.12120.119 stable + MinimumVisualStudioVersion 行（Project GUID 与配置映射不动，可新增 SolutionGuid）。旧工程引用的 ..\..\Common\2D\SVector2D.h、..\..\Common\ConsoleUtils.h、..\..\Common\utils.h 是磁盘上不存在的幽灵文件，新工程清单只列实际存在的。v145 中间目录：工程名短时为 <工程目录>\<工程名>\<Config>\，长时用截断名+GUID（如 WestWorl.0E2C3421）；exe 在 <工程目录>\<Config>\，均正常。仓库根目录已添加 .gitignore（忽略构建产物）。2026-10-03 已按同一模式完成第三个工程 WestWorldWithWoman（工程名 WestWorldWithWomen，ProjectGUID {B91191CB-4504-493E-8BF6-2688EBB2015B}；该工程自带 State.h/StateMachine.h、无 Messaging 依赖，外部头仅 Common\misc\ConsoleUtils.h 与 Common\misc\Utils.h——源码 include 为 "misc/Utils.h" 大写 U；双配置 0 警告，运行 78 行日志验证通过）。同日已按 add-code-comments 规范完成该工程全部 15 个文件的零基础中文注释（+1645 行纯增量 0 删除、UTF-8 BOM + LF 归一化、双配置 0 警告、运行输出与注释前逐项一致；实操陷阱详见同日新增的 feedback 条目）。其余章节（Chapter3+ 等）仍待转换。


- [2026-10-02 20:21:51] [2026-10-02] 用户要求为控制台英文输出加中文翻译时的既定规范（已按此完成 WestWorld1 全部 18 处）：格式为 english(中文)，半角括号包中文、中文内用全角标点；范围仅限各章节工程自身文件，Common\ 共享文件（如 misc\ConsoleUtils.h 的 "Press any key to continue"）未经要求不改动。技术要点：本仓库工程为 MultiByte 字符集 + UTF-8 BOM 源码，MSVC 编译时自动将中文字面量按系统 ACP(GBK) 编码进 exe，中文控制台(936)显示正常——切勿添加 /utf-8 或 /execution-charset（会使输出与 GBK 控制台不匹配而乱码）；验证手段：MSBuild 双配置 0 警告 + 在 exe 内按 GBK(936) 字节检索中文串且确认无 UTF-8 残留。其他章节若提出同样需求可复用此模式。
- [2026-10-03 15:38:24] EagleStudy\ 学习笔记 md 文件的编码约定：UTF-8 无 BOM + CRLF（2026-10-03 由 00-ReadCodeGuide 等既有文件实测确认），与工程源码的"UTF-8 BOM + CRLF"规范不同——write_file 产出为无 BOM 的 LF，写完笔记需用 PowerShell 归一化为无 BOM 的 CRLF；笔记风格：中文、H1 分节（一、二…）、代码摘录用 ```text 围栏。Mermaid 渲染兼容性（2026-10-03 实测）：用户查看环境对 stateDiagram-v2 的单行 note（note right of X : text）等语法渲染报错，已改用 flowchart TD 修复；写笔记时流程类图优先用 flowchart TD（引号标签 + <br/> + |边标签| 均验证可渲染），stateDiagram 仅用于状态转换图且禁用单行 note；ASCII 双冒号在 flowchart/sequence 的引号标签与消息文本中实测无碍，但在 stateDiagram 转移标签中存在风险。


### Reference

