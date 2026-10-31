param([string]$Zig = 'zig')
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $PSScriptRoot 'build/zig-global-cache'
$env:ZIG_LOCAL_CACHE_DIR = Join-Path $PSScriptRoot 'build/zig-cache'
New-Item -ItemType Directory -Force -Path build/tests | Out-Null
& $Zig c++ -std=c++17 -O2 -Isrc tests/core.cpp src/ForeLexer.cpp -o build/tests/core.exe
if ($LASTEXITCODE -ne 0) { throw 'Core test compilation failed' }
& ./build/tests/core.exe | Tee-Object -FilePath 'build/core-tests.log'
if ($LASTEXITCODE -ne 0) { throw 'Core tests failed' }
$log = Get-Content -LiteralPath 'build/core-tests.log' -Raw
$counts = [regex]::Match($log, 'CORE: (\d+)/(\d+) checks passed')
$benchmark = [regex]::Match($log, 'BENCHMARK: (\d+) bytes, (\d+) lines, ([\d.]+) ms')
if (!$counts.Success -or $counts.Groups[1].Value -ne $counts.Groups[2].Value) { throw 'Missing core test completion' }
$result = [ordered]@{
    checked_on='2026-10-31'; passed=$true; checks=[int]$counts.Groups[2].Value;
    compiler=('Zig ' + (& $Zig version));
    benchmark=[ordered]@{ bytes=[int]$benchmark.Groups[1].Value; lines=[int]$benchmark.Groups[2].Value; milliseconds=[double]::Parse($benchmark.Groups[3].Value,[Globalization.CultureInfo]::InvariantCulture) }
}
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'qa/core-validation.json'), ($result | ConvertTo-Json -Depth 5).Replace("`r`n", "`n") + "`n", [Text.UTF8Encoding]::new($false))
& $Zig c++ -std=c++17 -O2 -Isrc tests/detection.cpp src/ForeDetect.cpp src/ForeLexer.cpp -o build/tests/detection.exe
if ($LASTEXITCODE -ne 0) { throw 'Detection test compilation failed' }
& ./build/tests/detection.exe | Tee-Object -FilePath 'build/detection-tests.log'
if ($LASTEXITCODE -ne 0) { throw 'Detection tests failed' }
$detectionLog = Get-Content -LiteralPath 'build/detection-tests.log' -Raw
$detectionCounts = [regex]::Match($detectionLog, 'DETECTION: (\d+)/(\d+) checks passed')
if (!$detectionCounts.Success -or $detectionCounts.Groups[1].Value -ne $detectionCounts.Groups[2].Value) { throw 'Missing detection test completion' }
$detectionResult = [ordered]@{passed=$true;checks=[int]$detectionCounts.Groups[2].Value;compiler=('Zig ' + (& $Zig version));line_endings=@('LF','CRLF','CR')}
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'qa/detection-validation.json'), ($detectionResult | ConvertTo-Json).Replace("`r`n", "`n") + "`n", [Text.UTF8Encoding]::new($false))
