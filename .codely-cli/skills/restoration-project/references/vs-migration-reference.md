# 老工程迁移参考（VS 2026）

restoration-project skill 的详细参考：设置映射表、工具链细节、验证命令与常见陷阱。

## 目录

1. 老工程设置 → .vcxproj 映射表
2. VS 2026 工具链要点
3. 构建验证命令
4. 运行验证命令模板与陷阱
5. 常见编译问题与工程级修复
6. 清理清单细则

## 1. 老工程设置 → .vcxproj 映射表

### .vcproj（VS2008 XML）属性值

| 旧属性 | 旧值 | vcxproj 对应值 |
|---|---|---|
| ConfigurationType | 1 | `<ConfigurationType>Application</ConfigurationType>` |
| | 2 | DynamicLibrary |
| | 4 | StaticLibrary |
| CharacterSet | 0 | `<CharacterSet>NotSet</CharacterSet>` |
| | 1 | Unicode |
| | 2 | MultiByte |
| RuntimeLibrary（VCCLCompilerTool） | 0 | `<RuntimeLibrary>MultiThreaded</RuntimeLibrary>`（/MT） |
| | 1 | MultiThreadedDebug（/MTd） |
| | 2 | MultiThreadedDLL（/MD） |
| | 3 | MultiThreadedDebugDLL（/MDd） |
| SubSystem（VCLinkerTool） | 1 | `<SubSystem>Console</SubSystem>` |
| | 2 | Windows |
| WarningLevel | 3 | `<WarningLevel>Level3</WarningLevel>` |
| PreprocessorDefinitions | 如 WIN32;_DEBUG;_CONSOLE | 原样保留，末尾加 `;%(PreprocessorDefinitions)` |
| OutputDirectory=".\\Debug" |  | 不需设置：默认输出到 `<sln目录>\<Config>\` |

### .dsp（VC6）编译开关

| VC6 开关 | 处理方式 |
|---|---|
| /Gm | 对应 LinkIncremental，Debug 默认 true，无需设置 |
| /GX | 即 /EHsc，vcxproj 默认已含，无需设置 |
| /ZI /GZ | Debug 默认 /Zi + SDLCheck，无需设置 |
| /O2 | Release 默认已含 |
| /YX（自动 PCH） | vcxproj 不使用 PCH，忽略 |
| /FD /c /nologo | 编译器常规开关，无需设置 |

## 2. VS 2026 工具链要点

- VS 2026 = 版本 18.x；默认 C++ 平台工具集 **v145**（MSVC 14.5x）。
- 安装路径惯例：`C:\Program Files\Microsoft Visual Studio\18\<Edition>`。
- 定位安装：`& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath`
- MSBuild：`<安装路径>\MSBuild\Current\Bin\MSBuild.exe`。
- `<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>` 表示自动选用最新已安装 SDK；**勿硬编码**具体版本号（机器间会不匹配）。
- Win32 (x86) 目标在 VS 2026 默认 C++ 桌面工作负载下仍可构建（x64/x86 工具集组件同时提供两平台编译器）。
- 不要设置 OutDir/IntDir：默认 exe 输出到 `<sln目录>\<Config>\`，中间文件输出到 `<sln目录>\<工程名>\<Config>\`（嵌套一层工程名目录是 v145 的正常行为，勿"修复"它）。
- C++98 风格源码（Buckland 书代码）在 v145 下默认语言标准即可直接编译，无需设置 LanguageStandard。
- 保持旧工程的 GUID 不变并同步 .sln 与 .vcxproj，可让 VS 2026 无缝替换加载。

## 3. 构建验证命令

每个配置（Debug/Release）都必须通过：

```powershell
$vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath
& "$vs\MSBuild\Current\Bin\MSBuild.exe" "<sln路径>" /p:Configuration=Debug   /p:Platform=Win32 /m /v:m /nologo
& "$vs\MSBuild\Current\Bin\MSBuild.exe" "<sln路径>" /p:Configuration=Release /p:Platform=Win32 /m /v:m /nologo
```

成功标志：Exit Code 0，末尾出现 `工程名.vcxproj -> ...\<Config>\工程名.exe`。`/v:m` 精度下无警告输出即零警告；有警告（如 C4996）需记录并向用户报告，但不阻塞。

## 4. 运行验证命令模板与陷阱

```powershell
$exe = "<Debug exe 完整路径>"
$out = "$env:TEMP\smoke_test.txt"
Remove-Item $out -ErrorAction SilentlyContinue
$p = Start-Process -FilePath $exe -RedirectStandardOutput $out -PassThru
Start-Sleep -Seconds 18        # 时长 = 主循环次数 × Sleep毫秒 ÷ 1000 + 2（先读 main() 算准）
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force }   # 完整跑完后卡在"按任意键继续"属预期
Get-Content $out               # 检查输出是否包含全部关键业务行为
Remove-Item $out -ErrorAction SilentlyContinue
```

陷阱（都在实战中踩过，勿凭直觉判断"程序坏了"）：

- **cout 全缓冲**：stdout 重定向到文件/管道时不是行缓冲；程序跑完主循环、执行到结尾的 `std::endl`（典型如 "Press any key to continue"）才整缓冲区落盘。等待时间不够读到空文件 ≠ 程序故障。
- **_kbhit() 挂起**：stdin 被重定向（非控制台）时 `_kbhit()` 恒返回 0，程序永不自行退出；等待足够时长后 `Stop-Process` 是标准做法，不影响验证结论。
- **SetConsoleTextAttribute 静默失败**：重定向后无控制台句柄，设置文本颜色失败但无副作用，忽略即可。
- **等待时长必须先算**：读 main() 获取循环次数与 Sleep 毫秒数（如 20 次 × 800ms = 16 秒 → 等 18 秒）。
- 输出检查要点：行数合理、关键状态/消息齐全、数值符合程序逻辑（如累计值递增）、与该程序的已知行为一致。

## 5. 常见编译问题与工程级修复

全部通过工程配置解决，**绝不改源码**：

| 现象 | 工程级修复 |
|---|---|
| fatal error C1083: 无法打开 `misc/xxx.h` 等外部头 | `<AdditionalIncludeDirectories>` 添加 `..\..\Common`（或按源码 #include 与目录结构推导） |
| 无法打开 windows.h / kernel32.lib | WindowsTargetPlatformVersion 用 `10.0`；确认 VS 已装"使用 C++ 的桌面开发"工作负载 |
| LNK2001/LNK2019 缺少符号或 _main | 检查 ClCompile 清单是否漏了 .cpp |
| C4996（strcpy/sprintf 等） | 默认仅是警告，可保留并报告；如用户要求零警告，经确认后添加 `_CRT_SECURE_NO_WARNINGS` 预处理器定义 |
| LNK4098（/MT 与 /MD 冲突） | 核对所有配置 RuntimeLibrary 与旧工程一致且统一 |
| TCHAR/文本宏相关错误 | 保持 CharacterSet=MultiByte 与旧工程一致，勿改成 Unicode |
| 多字节字符集 MBCS 报错 | 仅 MFC 库才需 MBCS 支持库；纯 Win32/Console 工程无需处理 |

## 6. 清理清单细则

- 删除 `.vs/` 前先 `Get-Process devenv -ErrorAction SilentlyContinue` 确认 VS 已关闭；仍在运行则跳过该项并告知用户。
- 嵌套中间目录 = 工程目录下与工程同名的子目录（如 `WestWorld1\WestWorld1\`，内含 .obj/.tlog）。
- `git status` 终态参考：旧工程文件 `D`、`.sln` 为 `M`、`.vcxproj`/`.filters` 未跟踪（`??`）。
- 仓库若无 .gitignore，建议添加 `Debug/`、`Release/`、`.vs/`、`x64/`（但添加前先征求用户同意，属仓库级变更）。
- 删除是永久性的：git 跟踪文件可从历史恢复，未跟踪的产物本就无保留价值；仍应向用户逐项说明删除原因。
