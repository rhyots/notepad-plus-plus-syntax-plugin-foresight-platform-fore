param(
    [string]$NotepadDirectory = 'C:\Program Files\Notepad++',
    [string]$Zig = 'zig'
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$qaRoot = Join-Path $PSScriptRoot 'notepad-autodetect'
$plugin = Join-Path $qaRoot 'plugins/ForeLexer'
$probe = Join-Path $qaRoot 'plugins/AutoProbe'
$config = Join-Path $qaRoot 'plugins/Config'
New-Item -ItemType Directory -Force -Path $qaRoot,$plugin,$probe,$config | Out-Null
foreach ($name in @('notepad++.exe','langs.model.xml','stylers.model.xml')) { Copy-Item -LiteralPath (Join-Path $NotepadDirectory $name) -Destination (Join-Path $qaRoot $name) }
Copy-Item -LiteralPath (Join-Path $root 'build/x64/ForeLexer.dll') -Destination (Join-Path $plugin 'ForeLexer.dll')
Copy-Item -LiteralPath (Join-Path $root 'styles/ForeLexer-Light.xml') -Destination (Join-Path $config 'ForeLexer.xml')
New-Item -ItemType File -Force -Path (Join-Path $qaRoot 'doLocalConf.xml') | Out-Null
$utf8 = [Text.UTF8Encoding]::new($false)
[IO.File]::WriteAllText((Join-Path $qaRoot 'config.xml'), '<NotepadPlus><GUIConfigs><GUIConfig name="langsExcluded" langMenuCompact="yes" /></GUIConfigs></NotepadPlus>', $utf8)
$code = "Sub Demo;`nBegin`n x := 1;`nEnd Sub Demo;`n"
foreach ($name in @('auto.txt','extensionless','upper.TXT','blocked.log','blocked.py','revert.txt','native.fore')) { [IO.File]::WriteAllText((Join-Path $qaRoot $name),$code,$utf8) }
foreach ($name in @('paste.txt','manual.txt','manual-ui.txt','chosen.txt')) { [IO.File]::WriteAllText((Join-Path $qaRoot $name),'',$utf8) }
[IO.File]::WriteAllText((Join-Path $qaRoot 'plain.txt'),'class begin end function namespace',$utf8)
[IO.File]::WriteAllText((Join-Path $qaRoot 'basic.txt'),"Sub Demo()`nDim x As Integer`nEnd Sub`n",$utf8)
[IO.File]::WriteAllText((Join-Path $qaRoot 'comments.txt'),"{`n"+$code+"}`n",$utf8)
[IO.File]::WriteAllText((Join-Path $qaRoot 'strings.txt'),"`"Sub Demo;`"`n`"Begin`"`n`"End Sub Demo;`"`n",$utf8)
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $root 'build/zig-global-cache'
$env:ZIG_LOCAL_CACHE_DIR = Join-Path $root 'build/zig-cache'
& $Zig c++ -target x86_64-windows-gnu -std=c++17 -O2 -shared ('-I'+(Join-Path $root 'vendor/npp-sdk')) (Join-Path $PSScriptRoot 'AutoProbe.cpp') -luser32 -fno-sanitize=undefined -o (Join-Path $probe 'AutoProbe.dll')
if ($LASTEXITCODE -ne 0) { throw 'Auto-detection probe build failed' }
$results = Join-Path $qaRoot 'auto-results.json'
if (Test-Path -LiteralPath $results) { Remove-Item -LiteralPath $results }
$process = Start-Process -FilePath (Join-Path $qaRoot 'notepad++.exe') -ArgumentList '-multiInst','-nosession' -WindowStyle Hidden -PassThru
if (!$process.WaitForExit(35000)) { Stop-Process -Id $process.Id; throw 'Isolated auto-detection test timed out' }
$report = Get-Content -LiteralPath $results -Raw | ConvertFrom-Json
$report | Add-Member -NotePropertyName dll_sha256 -NotePropertyValue (Get-FileHash -LiteralPath (Join-Path $plugin 'ForeLexer.dll')).Hash.ToLowerInvariant()
$report | Add-Member -NotePropertyName environment -NotePropertyValue 'Notepad++ 8.9.8.1 x64, isolated portable profile'
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'autodetect-validation.json'),($report|ConvertTo-Json -Depth 8).Replace("`r`n","`n")+"`n",$utf8)
$failed = @($report.cases | Where-Object { !$_.passed })
if (!$report.passed -or $failed.Count) { $failed | Format-Table -AutoSize; throw 'Auto-detection integration failed' }
Write-Output ('AUTO NATIVE: '+$report.cases.Count+'/'+$report.cases.Count+' scenarios passed')
