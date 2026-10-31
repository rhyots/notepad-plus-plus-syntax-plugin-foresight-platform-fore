param([string]$NotepadDirectory='C:\Program Files\Notepad++',[string]$Zig='zig')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$work=Join-Path $PSScriptRoot ('notepad-theme/'+[guid]::NewGuid().ToString('N'))
$utf8=[Text.UTF8Encoding]::new($false)
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $root 'build/zig-global-cache'
$env:ZIG_LOCAL_CACHE_DIR=Join-Path $root 'build/zig-cache'
New-Item -ItemType Directory -Path $work -Force | Out-Null
$probeDll=Join-Path $work 'ThemeProbe.dll'
& $Zig c++ -target x86_64-windows-gnu -std=c++17 -O2 -shared ('-I'+(Join-Path $root 'vendor/npp-sdk')) ('-I'+(Join-Path $root 'src')) (Join-Path $PSScriptRoot 'ThemeProbe.cpp') -luser32 -fno-sanitize=undefined -o $probeDll
if($LASTEXITCODE -ne 0){throw 'Theme probe build failed'}
$runs=@()
foreach($variant in @('standard','custom')) {
 $dir=Join-Path $work $variant
 $plugin=Join-Path $dir 'plugins/ForeLexer';$probe=Join-Path $dir 'plugins/ThemeProbe';$config=Join-Path $dir 'plugins/Config';$themes=Join-Path $dir 'themes'
 New-Item -ItemType Directory -Force -Path $dir,$plugin,$probe,$config,$themes | Out-Null
 foreach($name in @('notepad++.exe','langs.model.xml','stylers.model.xml')){Copy-Item -LiteralPath (Join-Path $NotepadDirectory $name) -Destination (Join-Path $dir $name)}
 Copy-Item -LiteralPath (Join-Path $NotepadDirectory 'themes/DarkModeDefault.xml') -Destination $themes
 Copy-Item -LiteralPath (Join-Path $root 'build/x64/ForeLexer.dll') -Destination (Join-Path $plugin 'ForeLexer.dll')
 Copy-Item -LiteralPath $probeDll -Destination (Join-Path $probe 'ThemeProbe.dll')
 New-Item -ItemType File -Path (Join-Path $dir 'doLocalConf.xml') | Out-Null
 [IO.File]::WriteAllText((Join-Path $dir 'config.xml'),'<NotepadPlus><GUIConfigs><GUIConfig name="langsExcluded" langMenuCompact="yes" /></GUIConfigs></NotepadPlus>',$utf8)
 [xml]$xml=Get-Content -LiteralPath (Join-Path $root 'styles/ForeLexer-Light.xml') -Raw
 $keyword=$xml.SelectSingleNode('//WordsStyle[@styleID="1"]');$keyword.SetAttribute('fontSize','13');$keyword.SetAttribute('fontStyle','2')
 if($variant -eq 'custom'){$keyword.SetAttribute('fgColor','336699');New-Item -ItemType File -Path (Join-Path $dir 'custom.flag') | Out-Null}
 $style=Join-Path $config 'ForeLexer.xml';$xml.Save($style)
 $before=@{}
 foreach($node in $xml.SelectNodes('//WordsStyle')) {
  foreach($attribute in $node.Attributes){$before[$node.styleID+':'+$attribute.Name]=$attribute.Value}
 }
 [IO.File]::WriteAllText((Join-Path $dir 'sample.fore'),"Sub Demo;`nBegin`n Return;`nEnd Sub Demo;`n",$utf8)
 [IO.File]::WriteAllText((Join-Path $dir 'sample.cpp'),"int main() { return 0; }`n",$utf8)
 foreach($launch in @(1,2)) {
  $result=Join-Path $dir 'theme-results.json';if(Test-Path -LiteralPath $result){Remove-Item -LiteralPath $result}
  $process=Start-Process -FilePath (Join-Path $dir 'notepad++.exe') -ArgumentList '-multiInst','-nosession' -WindowStyle Hidden -PassThru
  if(!$process.WaitForExit(20000)){Stop-Process -Id $process.Id;throw 'Isolated theme test timed out'}
  $report=Get-Content -LiteralPath $result -Raw | ConvertFrom-Json
  if(!$report.passed){$report.cases|Where-Object{!$_.passed}|Format-Table;throw 'Native theme switching failed'}
  if($report.startup_dark -ne ($launch -eq 2)){throw 'Theme did not persist across restart'}
  [xml]$saved=Get-Content -LiteralPath $style -Raw
  foreach($node in $saved.SelectNodes('//WordsStyle')) {
   foreach($attribute in $node.Attributes){if($before[$node.styleID+':'+$attribute.Name] -ne $attribute.Value){throw 'User lexer style attributes changed'}}
  }
  $runs+=@{variant=$variant;launch=$launch;result=$report}
 }
}
$final=[ordered]@{passed=$true;environment='Notepad++ 8.9.8.1 x64';dll_sha256=(Get-FileHash -LiteralPath (Join-Path $root 'build/x64/ForeLexer.dll')).Hash.ToLowerInvariant();method='Real Style Configurator theme combo, both views, saved theme and actual restart';runs=$runs;lexer_style_attributes_preserved=$true}
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'theme-validation.json'),($final|ConvertTo-Json -Depth 12).Replace("`r`n","`n")+"`n",$utf8)
Write-Output ('THEMES: '+$runs.Count+' native runs passed; real theme changes, both views, restart and custom color/font settings verified.')
