param(
    [string]$NotepadDirectory = 'C:\Program Files\Notepad++',
    [string]$Zig = 'zig',
    [string]$Python = 'python',
    [string]$ExtraExamplesDirectory = ''
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$qaRoot = Join-Path $PSScriptRoot 'notepad-test'
$out = Join-Path $PSScriptRoot 'results'
New-Item -ItemType Directory -Force -Path $qaRoot,$out,(Join-Path $qaRoot 'plugins/ForeLexer'),(Join-Path $qaRoot 'plugins/ForeProbe'),(Join-Path $qaRoot 'plugins/Config') | Out-Null
foreach ($name in @('notepad++.exe','langs.model.xml','stylers.model.xml')) { Copy-Item -LiteralPath (Join-Path $NotepadDirectory $name) -Destination (Join-Path $qaRoot $name) }
New-Item -ItemType File -Force -Path (Join-Path $qaRoot 'doLocalConf.xml') | Out-Null
[IO.File]::WriteAllText((Join-Path $qaRoot 'config.xml'), '<NotepadPlus><GUIConfigs><GUIConfig name="langsExcluded" langMenuCompact="yes" /></GUIConfigs></NotepadPlus>', [Text.UTF8Encoding]::new($false))
Copy-Item -LiteralPath (Join-Path $root 'build/x64/ForeLexer.dll') -Destination (Join-Path $qaRoot 'plugins/ForeLexer/ForeLexer.dll')
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $root 'build/zig-global-cache'
$env:ZIG_LOCAL_CACHE_DIR = Join-Path $root 'build/zig-cache'
& $Zig c++ -target x86_64-windows-gnu -std=c++17 -O2 -shared ('-I' + (Join-Path $root 'vendor/npp-sdk')) (Join-Path $PSScriptRoot 'Probe.cpp') -luser32 -fno-sanitize=undefined -o (Join-Path $qaRoot 'plugins/ForeProbe/ForeProbe.dll')
if ($LASTEXITCODE -ne 0) { throw 'Integration probe build failed' }
foreach ($fixture in @(@('demo','Fore-demo.fore'), @('edges','Folding-edge-cases.fore'))) {
    $text = [IO.File]::ReadAllText((Join-Path $root ('examples/' + $fixture[1]))).Replace("`r`n","`n")
    [IO.File]::WriteAllText((Join-Path $qaRoot ($fixture[0] + '-lf.fore')), $text, [Text.UTF8Encoding]::new($false))
    [IO.File]::WriteAllText((Join-Path $qaRoot ($fixture[0] + '-crlf.fore')), $text.Replace("`n","`r`n"), [Text.UTF8Encoding]::new($false))
}
if ($ExtraExamplesDirectory) {
    foreach ($example in @(@('01-minesweeper.fore','minesweeper.fore'), @('02-russian-declension.fore','declension.fore'), @('03-db-session-handler.fore','db-handler.fore'))) {
        Copy-Item -LiteralPath (Join-Path $ExtraExamplesDirectory $example[0]) -Destination (Join-Path $qaRoot $example[1])
    }
}
foreach ($theme in @('Light','Dark')) {
    [xml]$styles = [IO.File]::ReadAllText((Join-Path $qaRoot 'stylers.model.xml'))
    $default = $styles.SelectSingleNode('//GlobalStyles/WidgetStyle[@name="Default Style"]')
    $default.SetAttribute('bgColor', $(if ($theme -eq 'Dark') { '1E1E1E' } else { 'FFFFFF' }))
    $default.SetAttribute('fgColor', $(if ($theme -eq 'Dark') { 'D4D4D4' } else { '24292F' }))
    $styles.Save((Join-Path $qaRoot 'stylers.xml'))
    Copy-Item -LiteralPath (Join-Path $root "styles/ForeLexer-$theme.xml") -Destination (Join-Path $qaRoot 'plugins/Config/ForeLexer.xml')
    foreach ($json in Get-ChildItem -LiteralPath $qaRoot -Filter '*.json' -File) { Remove-Item -LiteralPath $json.FullName }
    $process = Start-Process -FilePath (Join-Path $qaRoot 'notepad++.exe') -ArgumentList '-multiInst','-nosession' -WindowStyle Hidden -PassThru
    if (!$process.WaitForExit(20000)) { Stop-Process -Id $process.Id; throw 'Isolated Notepad++ integration test timed out' }
    $dest = Join-Path $out $theme.ToLowerInvariant()
    New-Item -ItemType Directory -Force -Path $dest | Out-Null
    if (!(Test-Path -LiteralPath (Join-Path $qaRoot 'ui.json'))) { throw 'Integration probe did not complete' }
    Get-ChildItem -LiteralPath $qaRoot -Filter '*.json' -File | Copy-Item -Destination $dest
}
& $Python -X utf8 (Join-Path $root 'tools/verify.py') --native-root $qaRoot
if ($LASTEXITCODE -ne 0) { throw 'Native integration validation failed' }
