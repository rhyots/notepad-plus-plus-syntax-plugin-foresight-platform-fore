param([string]$Zig = 'zig', [ValidateSet('x64','x86','arm64')][string]$Architecture = 'x64')
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $PSScriptRoot 'build/zig-global-cache'
$env:ZIG_LOCAL_CACHE_DIR = Join-Path $PSScriptRoot 'build/zig-cache'
$target = @{x64='x86_64-windows-gnu'; x86='x86-windows-gnu'; arm64='aarch64-windows-gnu'}[$Architecture]
$out = Join-Path $PSScriptRoot "build/$Architecture"
New-Item -ItemType Directory -Force -Path $out | Out-Null
& $Zig rc /fo "$out/ForeLexer.res" src/ForeLexer.rc
if ($LASTEXITCODE -ne 0) { throw 'Windows resource compilation failed' }
& $Zig c++ -target $target -std=c++17 -O2 -shared -Isrc -Ivendor/npp-sdk src/Plugin.cpp src/ForeLexer.cpp src/ForeDetect.cpp src/AutoDetect.cpp src/ThemePalette.cpp src/ForeLexer.def "$out/ForeLexer.res" -luser32 -lcomctl32 -fno-sanitize=undefined -o "$out/ForeLexer.dll"
if ($LASTEXITCODE -ne 0) { throw 'Foresight Platform Fore build failed' }
Copy-Item -LiteralPath 'styles/ForeLexer-Light.xml' -Destination "$out/ForeLexer.xml"
Get-FileHash -LiteralPath "$out/ForeLexer.dll" -Algorithm SHA256 | Select-Object Path,Hash
