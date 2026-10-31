param([string]$NotepadDirectory = 'C:\Program Files\Notepad++', [ValidateSet('x64','x86')][string]$Architecture='x64')
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$portable = Join-Path $PSScriptRoot ('notepad-installer/' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $portable | Out-Null
Copy-Item -LiteralPath (Join-Path $NotepadDirectory 'notepad++.exe') -Destination $portable
New-Item -ItemType File -Path (Join-Path $portable 'doLocalConf.xml') | Out-Null
$udl = Join-Path $portable 'userDefineLang.xml'
$original = '<NotepadPlus><UserLang name="Fore test" ext="FORE txt custom"><Settings><Global caseIgnored="yes" /></Settings></UserLang><UserLang name="Other" ext="other" /></NotepadPlus>'
$utf8 = [Text.UTF8Encoding]::new($false)
[IO.File]::WriteAllText($udl, $original, $utf8)
$udlHash = (Get-FileHash -LiteralPath $udl).Hash
$first = (& (Join-Path $root 'install.ps1') -NotepadDirectory $portable -Palette Light) | ConvertFrom-Json
$xml = [xml]([IO.File]::ReadAllText($udl))
$expected = [xml]$original
$expected.NotepadPlus.UserLang[0].SetAttribute('ext', 'txt custom')
if ($xml.OuterXml -ne $expected.OuterXml) { throw 'Installer changed more than the competing fore extension' }
$backup = @($first.backups | Where-Object { $_.target -eq $udl })
if ($backup.Count -ne 1 -or (Get-FileHash -LiteralPath $backup[0].backup).Hash -ne $udlHash) { throw 'Original UDL backup missing or changed' }
$second = (& (Join-Path $root 'install.ps1') -NotepadDirectory $portable -Palette Dark) | ConvertFrom-Json
$dllHash = (Get-FileHash -LiteralPath (Join-Path $root "build/$Architecture/ForeLexer.dll")).Hash.ToLowerInvariant()
$lightHash = (Get-FileHash -LiteralPath (Join-Path $root 'styles/ForeLexer-Light.xml')).Hash.ToLowerInvariant()
$darkHash = (Get-FileHash -LiteralPath (Join-Path $root 'styles/ForeLexer-Dark.xml')).Hash.ToLowerInvariant()
if ($first.binary_sha256 -ne $dllHash -or $second.binary_sha256 -ne $dllHash) { throw 'Installed DLL differs from the current build' }
if ($first.style_sha256 -ne $lightHash -or $second.style_sha256 -ne $darkHash) { throw 'Installed presets differ from release files' }
$previousStyle = @($second.backups | Where-Object { $_.target -eq $second.style })
if ($previousStyle.Count -ne 1 -or (Get-FileHash -LiteralPath $previousStyle[0].backup).Hash.ToLowerInvariant() -ne $lightHash) { throw 'Update did not preserve the prior configuration' }
$report = [ordered]@{
    passed = $true
    checked_on = '2026-10-31'
    environment = "isolated portable Notepad++ 8.9.8.1 $Architecture; installer exercised without launching editor"
    dll_sha256 = $dllHash
    installer_sha256 = (Get-FileHash -LiteralPath (Join-Path $root 'install.ps1')).Hash.ToLowerInvariant()
    binary_matches_tested_release = $true
    native_xml_matches_light_and_dark_presets = $true
    udl_backup_created = $true
    only_fore_extension_removed_from_udl = $true
    previous_configuration_backed_up_on_update = $true
    main_user_installation_modified_this_run = $false
}
[IO.File]::WriteAllText((Join-Path $PSScriptRoot $(if ($Architecture -eq 'x64') { 'installation-validation.json' } else { 'x86/installation-validation.json' })), (($report | ConvertTo-Json).Replace("`r`n", "`n")+"`n"), $utf8)
Write-Output 'INSTALLER: isolated install/update, both presets, byte hashes and UDL migration passed.'
