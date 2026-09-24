# normalize-verify.ps1 —— 注释完成后归一化编码并校验"纯新增"
#
# 用法（在仓库根目录执行；files 为本批全部注释文件，路径相对仓库根）：
#   powershell -NoProfile -ExecutionPolicy Bypass -File <skill路径>\scripts\normalize-verify.ps1 <file1> <file2> ...
#
# 对每个文件：
#   1) 按该文件的 git 属性检测目标行尾（i/lf w/crlf → CRLF；w/lf → LF；默认 CRLF）；
#   2) 已提交文件：末尾换行符个数与 HEAD 原版逐字节对齐（部分原文件结尾没有换行符）；
#   3) 统一转 UTF-8 BOM（MSVC/VS 正确显示中文，避免 C4819 警告）；
#   4) 幂等：对已归一化的文件重复执行不产生任何变化。
# 最后打印每个文件的 git diff 增删统计：任一文件删除数 > 0 即整体 FAIL（退出码 1）。
param(
  [Parameter(Mandatory = $true, Position = 0, ValueFromRemainingArguments = $true)][string[]]$Files
)

$ErrorActionPreference = 'Stop'

$repoRoot = (git rev-parse --show-toplevel | Select-Object -First 1)
if (-not $repoRoot) { Write-Output 'FAIL: 当前目录不在 git 仓库内'; exit 1 }
Set-Location $repoRoot

$utf8bom = New-Object System.Text.UTF8Encoding $true
$strict  = New-Object System.Text.UTF8Encoding($false, $true)
$gbk     = [System.Text.Encoding]::GetEncoding(936)
$lf  = [string][char]10
$cr  = [string][char]13
$crlf = $cr + $lf

foreach ($f in $Files) {
  $rel = ($f -replace '\\', '/')
  $abs = Join-Path $repoRoot ($f -replace '/', '\')
  if (-not (Test-Path -LiteralPath $abs)) { Write-Output "SKIP (文件不存在): $f"; continue }

  # 检测该文件的目标行尾（工作区属性 w/crlf 或 w/lf）
  $eolInfo = git ls-files --eol -- $rel
  $targetEol = 'crlf'
  if ($eolInfo -match 'w/lf') { $targetEol = 'lf' }

  # 读取 HEAD 原版 blob，统计末尾换行符个数（未跟踪/未提交文件跳过对齐）
  $oTrail = -1
  $tmp = Join-Path $env:TEMP "blobcheck.bin"
  cmd /c "git cat-file -p ""HEAD:$rel"" > ""$tmp""" | Out-Null
  if ($LASTEXITCODE -eq 0) {
    $ob = [System.IO.File]::ReadAllBytes($tmp)
    $oTrail = 0
    for ($i = $ob.Length - 1; $i -ge 0; $i--) { if ($ob[$i] -eq 0x0A) { $oTrail++ } else { break } }
  }
  Remove-Item $tmp -ErrorAction SilentlyContinue

  # 读当前文件字节并解码（优先 UTF-8，含 BOM 时先剥离；失败回退 GBK）
  $cb = [System.IO.File]::ReadAllBytes($abs)
  if ($cb.Length -ge 3 -and $cb[0] -eq 0xEF -and $cb[1] -eq 0xBB -and $cb[2] -eq 0xBF) {
    $text = [System.Text.Encoding]::UTF8.GetString($cb, 3, $cb.Length - 3)
  } else {
    try { $text = $strict.GetString($cb) }
    catch { $text = $gbk.GetString($cb) }
  }

  # 统一为 LF -> 对齐末尾换行符 -> 转目标行尾
  $text = $text.Replace($crlf, $lf).Replace($cr, $lf)
  if ($oTrail -ge 0) {
    $cTrail = 0
    for ($i = $text.Length - 1; $i -ge 0; $i--) { if ($text[$i] -eq 10) { $cTrail++ } else { break } }
    if ($cTrail -lt $oTrail) { $text += $lf * ($oTrail - $cTrail) }
    elseif ($cTrail -gt $oTrail) { $text = $text.Substring(0, $text.Length - ($cTrail - $oTrail)) }
  }
  if ($targetEol -eq 'crlf') { $text = $text.Replace($lf, $crlf) }

  [System.IO.File]::WriteAllText($abs, $text, $utf8bom)
  $trailNote = if ($oTrail -ge 0) { "末尾换行=$oTrail" } else { "未跟踪,未对齐末尾" }
  Write-Output ("OK: {0} -> UTF-8 BOM + {1} ({2})" -f $f, $targetEol.ToUpper(), $trailNote)
}

# 纯增量校验：每个文件删除数必须为 0
$anyFail = $false
foreach ($f in $Files) {
  $rel = ($f -replace '\\', '/')
  $line = git diff --numstat -- $rel
  if ($line) {
    $parts = $line -split "`t"
    $add = [int]$parts[0]; $del = [int]$parts[1]
    if ($del -gt 0) { Write-Output ("VERIFY FAIL: {0} 删除了 {1} 行原代码!" -f $f, $del); $anyFail = $true }
    else { Write-Output ("VERIFY OK: {0} (+{1} / -0)" -f $f, $add) }
  } else {
    Write-Output ("VERIFY OK: {0} (无改动)" -f $f)
  }
}
if ($anyFail) { Write-Output '== 存在 VERIFY FAIL，必须修复后重跑 =='; exit 1 }
Write-Output '== ALL PASS =='; exit 0
