# =============================================================================
#  build_all.ps1  —  main_*.cpp を MSVC(cl) で一括ビルドする補助スクリプト
# -----------------------------------------------------------------------------
#  各 .cpp は独立した main() を持つため、1 本ずつ別の .exe にビルドします。
#  Visual Studio の環境(vcvars64.bat)を自動で読み込むので、通常の PowerShell から
#  実行できます（Developer PowerShell でなくても可）。
#
#  使い方（このフォルダで）：
#    powershell -ExecutionPolicy Bypass -File .\build_all.ps1          # 全部ビルド
#    powershell -ExecutionPolicy Bypass -File .\build_all.ps1 -Run     # ビルド後に実行
#    powershell -ExecutionPolicy Bypass -File .\build_all.ps1 -O2 -Run # 最適化ありで実行
#    powershell -ExecutionPolicy Bypass -File .\build_all.ps1 -Only main_move_semantics.cpp
#
#  生成物は build\ サブフォルダに出力されます（*.obj / *.exe）。
# =============================================================================
[CmdletBinding()]
param(
    [switch]$Run,                 # ビルド後に各 exe を実行する
    [switch]$O2,                  # 最適化あり(/O2)でビルドする
    [string]$Std = 'c++latest',   # /std: に渡す規格（C++23 相当は c++latest）
    [string]$Only                 # 特定の 1 ファイルだけを対象にする（例: main_forward.cpp）
)

$ErrorActionPreference = 'Stop'
$here  = Split-Path -Parent $MyInvocation.MyCommand.Path
$build = Join-Path $here 'build'
New-Item -ItemType Directory -Force $build | Out-Null

# --- vcvars64.bat を探す --------------------------------------------------
function Find-VcVars {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $p = & $vswhere -prerelease -latest -products * `
                 -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
                 -property installationPath 2>$null | Select-Object -First 1
        if ($p) {
            $v = Join-Path $p 'VC\Auxiliary\Build\vcvars64.bat'
            if (Test-Path $v) { return $v }
        }
    }
    foreach ($base in @(
        'C:\Program Files\Microsoft Visual Studio\18\Community',
        'C:\Program Files\Microsoft Visual Studio\18\Professional',
        'C:\Program Files\Microsoft Visual Studio\18\Enterprise',
        'C:\Program Files\Microsoft Visual Studio\2022\Community',
        'C:\Program Files\Microsoft Visual Studio\2022\Professional',
        'C:\Program Files\Microsoft Visual Studio\2022\Enterprise'
    )) {
        $v = Join-Path $base 'VC\Auxiliary\Build\vcvars64.bat'
        if (Test-Path $v) { return $v }
    }
    throw 'vcvars64.bat が見つかりません。VS の「Developer PowerShell」から実行してください。'
}

# --- vcvars の環境変数を、この PowerShell セッションに取り込む（1 回だけ）---
function Import-VcVars([string]$vcvars) {
    cmd /c "call `"$vcvars`" >nul 2>&1 && set" | ForEach-Object {
        if ($_ -match '^(.*?)=(.*)$') { Set-Item -Path "env:$($matches[1])" -Value $matches[2] }
    }
}

$vcvars = Find-VcVars
Write-Host "using vcvars : $vcvars" -ForegroundColor Cyan
Import-VcVars $vcvars

$flags = @("/std:$Std", '/utf-8', '/EHsc', '/nologo')
if ($O2) { $flags += '/O2' }
Write-Host ("cl flags     : {0}" -f ($flags -join ' ')) -ForegroundColor Cyan

$files = if ($Only) { Get-ChildItem -Path $here -Filter $Only }
         else       { Get-ChildItem -Path $here -Filter 'main_*.cpp' | Sort-Object Name }

$ok = 0; $ng = 0
foreach ($f in $files) {
    $exe = Join-Path $build ($f.BaseName + '.exe')
    Write-Host ("`n[build] {0}" -f $f.Name) -ForegroundColor Yellow
    $out = & cl @flags /Fo"$build\" /Fe"$exe" $f.FullName 2>&1
    if ($LASTEXITCODE -eq 0) {
        $ok++
        if ($Run) {
            Write-Host ("[run  ] {0}" -f $f.Name) -ForegroundColor Green
            & $exe
        }
    } else {
        $ng++
        Write-Host ("[FAIL ] {0}" -f $f.Name) -ForegroundColor Red
        $out | ForEach-Object { Write-Host "        $_" }
    }
}
Write-Host ("`n==== done: {0} ok / {1} ng ====" -f $ok, $ng) `
           -ForegroundColor $(if ($ng) { 'Red' } else { 'Cyan' })
