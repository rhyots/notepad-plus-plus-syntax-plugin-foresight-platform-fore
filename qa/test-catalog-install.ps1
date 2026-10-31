param(
    [string]$NotepadDirectory = 'C:\Program Files\Notepad++',
    [Parameter(Mandatory)][string]$Archive,
    [ValidateSet('x64','x86')][string]$Architecture = 'x64'
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$testRoot = Join-Path $PSScriptRoot ('notepad-catalog/' + [guid]::NewGuid().ToString('N'))
$plugin = Join-Path $testRoot 'plugins/ForeLexer'
$probe = Join-Path $testRoot 'plugins/ForeProbe'
New-Item -ItemType Directory -Force -Path $testRoot,$plugin,$probe | Out-Null
foreach ($name in @('notepad++.exe','langs.model.xml','stylers.model.xml')) {
    Copy-Item -LiteralPath (Join-Path $NotepadDirectory $name) -Destination (Join-Path $testRoot $name)
}
Expand-Archive -LiteralPath $Archive -DestinationPath $plugin
Copy-Item -LiteralPath (Join-Path $PSScriptRoot "notepad-test-$Architecture/plugins/ForeProbe/ForeProbe.dll") -Destination (Join-Path $probe 'ForeProbe.dll')
Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot "notepad-test-$Architecture") -Filter '*.fore' -File | Copy-Item -Destination $testRoot
$minimal = '<NotepadPlus><GUIConfigs><GUIConfig name="langsExcluded" langMenuCompact="yes" /></GUIConfigs></NotepadPlus>'
$utf8 = [Text.UTF8Encoding]::new($false)
New-Item -ItemType File -Path (Join-Path $testRoot 'doLocalConf.xml') | Out-Null
[IO.File]::WriteAllText((Join-Path $testRoot 'config.xml'), $minimal, $utf8)
$config = Join-Path $testRoot 'plugins/Config/ForeLexer.xml'
function Run-Probe([string[]]$AdditionalArguments = @()) {
    foreach ($file in @(Get-ChildItem -LiteralPath $testRoot -Filter '*.json' -File)) { Remove-Item -LiteralPath $file.FullName }
    $arguments = @('-multiInst','-nosession') + $AdditionalArguments
    $process = Start-Process -FilePath (Join-Path $testRoot 'notepad++.exe') -ArgumentList $arguments -WindowStyle Hidden -PassThru
    if (!$process.WaitForExit(20000)) { Stop-Process -Id $process.Id; throw 'Isolated catalog test timed out' }
    $ui = Get-Content -LiteralPath (Join-Path $testRoot 'ui.json') -Raw | ConvertFrom-Json
    if (!$ui.style_dialog_contains_fore -or !$ui.menu_path.EndsWith('/F/Fore')) { throw 'Fore failed to register from catalog payload' }
}
Run-Probe
if ((Get-FileHash -LiteralPath $config).Hash -ne (Get-FileHash -LiteralPath (Join-Path $root 'styles/ForeLexer-Light.xml')).Hash) { throw 'Embedded first-run XML differs from light preset' }
$custom = [IO.File]::ReadAllText((Join-Path $root 'styles/ForeLexer-Dark.xml')).Replace('</NotepadPlus>', '<!-- preserved user configuration --></NotepadPlus>')
[IO.File]::WriteAllText($config, $custom, $utf8)
$customHash = (Get-FileHash -LiteralPath $config).Hash
Run-Probe
if ((Get-FileHash -LiteralPath $config).Hash -ne $customHash) { throw 'Existing user XML was overwritten' }
$report = [ordered]@{
    passed = $true
    notepad = "8.9.8.1 $Architecture"
    archive_sha256 = (Get-FileHash -LiteralPath $Archive -Algorithm SHA256).Hash.ToLowerInvariant()
    dll_sha256 = (Get-FileHash -LiteralPath (Join-Path $plugin 'ForeLexer.dll') -Algorithm SHA256).Hash.ToLowerInvariant()
    fresh_portable_configuration_created = $true
    existing_custom_configuration_preserved = $true
    regular_appdata_installation_tested = $false
    language_menu_and_style_configurator_verified = $true
    actual_plugins_admin_download_install_update_remove_tested = $false
}
$reportText = ($report | ConvertTo-Json).Replace("`r`n", "`n") + "`n"
$reportDirectory = if ($Architecture -eq 'x64') {$PSScriptRoot} else {Join-Path $PSScriptRoot $Architecture}
[IO.File]::WriteAllText((Join-Path $reportDirectory 'catalog-install-validation.json'), $reportText, $utf8)
Write-Output 'CATALOG PAYLOAD: clean portable startup, custom XML preservation and real language/style registration passed. Regular APPDATA deployment and actual Plugins Admin lifecycle remain untested.'
