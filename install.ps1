param(
    [string]$NotepadDirectory = 'C:\Program Files\Notepad++',
    [string]$ConfigurationDirectory = '',
    [ValidateSet('Light','Dark')][string]$Palette = 'Light'
)
$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$exe = Join-Path $NotepadDirectory 'notepad++.exe'
if (!(Test-Path -LiteralPath $exe)) { throw "Notepad++ not found: $exe" }
$nppVersion = (Get-Item -LiteralPath $exe).VersionInfo.FileVersion
if ([version]$nppVersion -lt [version]'8.4') { throw 'This plugin requires the ILexer5 interface introduced in Notepad++ 8.4.' }
if (!$ConfigurationDirectory) {
    $ConfigurationDirectory = if (Test-Path -LiteralPath (Join-Path $NotepadDirectory 'doLocalConf.xml')) { $NotepadDirectory } else { Join-Path $env:APPDATA 'Notepad++' }
}
$binary = Join-Path $root 'build/x64/ForeLexer.dll'
if (!(Test-Path -LiteralPath $binary)) { $binary = Join-Path $root 'plugins/ForeLexer/ForeLexer.dll' }
$style = Join-Path $root "styles/ForeLexer-$Palette.xml"
if (!(Test-Path -LiteralPath $style)) { $style = Join-Path $root "presets/ForeLexer-$Palette.xml" }
if (!(Test-Path -LiteralPath $binary) -or !(Test-Path -LiteralPath $style)) { throw 'Build the project or unpack the complete release archive first.' }
$targetBinary = Join-Path $NotepadDirectory 'plugins/ForeLexer/ForeLexer.dll'
$targetStyle = Join-Path $ConfigurationDirectory 'plugins/Config/ForeLexer.xml'
$globalStyle = Join-Path $NotepadDirectory 'plugins/Config/ForeLexer.xml'
if ($globalStyle -ne $targetStyle -and (Test-Path -LiteralPath $globalStyle)) {
    throw "An install-wide XML takes precedence: $globalStyle. Move that existing configuration to a backup first, or use the local configuration directory explicitly."
}
# Check the PE machine before copying a DLL into a different architecture editor.
function Get-PeMachine([string]$Path) {
    $data = [IO.File]::ReadAllBytes($Path)
    $pe = [BitConverter]::ToInt32($data, 0x3c)
    return [BitConverter]::ToUInt16($data, $pe + 4)
}
if ((Get-PeMachine $exe) -ne 0x8664 -or (Get-PeMachine $binary) -ne 0x8664) { throw 'Foresight Platform Fore 1.0.0 requires Notepad++ Windows x64.' }
if ((Test-Path -LiteralPath $targetBinary) -and @(Get-Process -Name 'notepad++' -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $exe }).Count) {
    throw 'Close all instances of this Notepad++ before updating an already installed DLL.'
}
$backup = Join-Path $ConfigurationDirectory ('ForeLexerBackups/' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $targetBinary) | Out-Null
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $targetStyle),$backup | Out-Null
$changes = [Collections.Generic.List[object]]::new()
function Backup-Existing([string]$Path, [string]$Name) {
    if (Test-Path -LiteralPath $Path) {
        $destination = Join-Path $backup $Name
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destination) | Out-Null
        Copy-Item -LiteralPath $Path -Destination $destination
        $changes.Add([pscustomobject]@{ target=$Path; backup=$destination; before_sha256=(Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant() })
    }
}
Backup-Existing $targetBinary 'previous/ForeLexer.dll'
Backup-Existing $targetStyle 'previous/ForeLexer.xml'
Copy-Item -LiteralPath $binary -Destination $targetBinary
Copy-Item -LiteralPath $style -Destination $targetStyle
foreach ($pair in @(@($binary,$targetBinary), @($style,$targetStyle))) {
    if ((Get-FileHash -LiteralPath $pair[0]).Hash -ne (Get-FileHash -LiteralPath $pair[1]).Hash) { throw 'Installed file hash mismatch' }
}
# A UDL extension has precedence over external lexers. Preserve UDL definitions,
# backing up each file before removing only its competing .fore association.
$udlFiles = @()
$legacy = Join-Path $ConfigurationDirectory 'userDefineLang.xml'
if (Test-Path -LiteralPath $legacy) { $udlFiles += Get-Item -LiteralPath $legacy }
$udlDir = Join-Path $ConfigurationDirectory 'userDefineLangs'
if (Test-Path -LiteralPath $udlDir) { $udlFiles += Get-ChildItem -LiteralPath $udlDir -Filter '*.xml' -File }
foreach ($file in $udlFiles) {
    $xml = [Xml.XmlDocument]::new(); $xml.PreserveWhitespace = $true; $xml.XmlResolver = $null; $xml.Load($file.FullName)
    $changed = $false
    foreach ($language in $xml.SelectNodes('/NotepadPlus/UserLang')) {
        $extensions = @($language.GetAttribute('ext') -split '\s+' | Where-Object { $_ })
        if (@($extensions | Where-Object { $_ -ieq 'fore' }).Count) {
            if (!$changed) { Backup-Existing $file.FullName ('udl/' + $file.Name) }
            $language.SetAttribute('ext', (@($extensions | Where-Object { $_ -ine 'fore' }) -join ' '))
            $changed = $true
        }
    }
    if ($changed) { $xml.Save($file.FullName) }
}
$receipt = [ordered]@{
    version='1.0.0'; palette=$Palette; binary=$targetBinary; style=$targetStyle;
    binary_sha256=(Get-FileHash -LiteralPath $targetBinary -Algorithm SHA256).Hash.ToLowerInvariant();
    style_sha256=(Get-FileHash -LiteralPath $targetStyle -Algorithm SHA256).Hash.ToLowerInvariant();
    backups=$changes.ToArray(); restart_required=$true
}
$receipt | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $backup 'installation.json') -Encoding utf8NoBOM
$receipt | ConvertTo-Json -Depth 8
