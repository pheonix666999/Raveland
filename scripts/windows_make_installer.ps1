param(
  [Parameter(Mandatory = $true)]
  [string]$ArtefactsDir,

  [Parameter(Mandatory = $true)]
  [string]$OutDir,

  [string]$Version = "0.1.0",

  [switch]$RequireInstaller
)

$ErrorActionPreference = "Stop"

function Resolve-AbsPath([string]$PathValue) {
  return (Resolve-Path -LiteralPath $PathValue).Path
}

function Find-MakeNsis {
  $cmd = Get-Command makensis -ErrorAction SilentlyContinue
  if ($cmd) { return $cmd.Source }

  $candidates = @(
    "$env:ProgramFiles\NSIS\makensis.exe",
    "$env:ProgramFiles(x86)\NSIS\makensis.exe"
  )
  foreach ($c in $candidates) {
    if (Test-Path -LiteralPath $c) { return $c }
  }
  return $null
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$artefactsAbs = Resolve-AbsPath $ArtefactsDir
$outAbs = Resolve-AbsPath $OutDir

$standaloneExe = Join-Path $artefactsAbs "Standalone\RaveLand.exe"
$vst3Dir = Join-Path $artefactsAbs "VST3\RaveLand.vst3"
$vst3Binary = Join-Path $vst3Dir "Contents\x86_64-win\RaveLand.vst3"

if (!(Test-Path -LiteralPath $standaloneExe)) { throw "Missing: $standaloneExe" }
if (!(Test-Path -LiteralPath $vst3Dir)) { throw "Missing: $vst3Dir" }
if (!(Test-Path -LiteralPath $vst3Binary)) { throw "Missing: $vst3Binary" }

# Always produce a zip for customers who prefer drag/drop install.
$zipPath = Join-Path $outAbs "RaveLand-windows.zip"
if (Test-Path -LiteralPath $zipPath) { Remove-Item -Force -LiteralPath $zipPath }

$zipStage = Join-Path $env:TEMP ("raveland_zip_" + [Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $zipStage | Out-Null
try {
  Copy-Item -Force -LiteralPath $standaloneExe -Destination (Join-Path $zipStage "RaveLand.exe")
  Copy-Item -Recurse -Force -LiteralPath $vst3Dir -Destination (Join-Path $zipStage "RaveLand.vst3")
  Compress-Archive -Path (Join-Path $zipStage "*") -DestinationPath $zipPath -Force
  Write-Host "Wrote: $zipPath"
} finally {
  Remove-Item -Recurse -Force -LiteralPath $zipStage -ErrorAction SilentlyContinue
}

$makensis = Find-MakeNsis
if (!$makensis) {
  $msg = "NSIS (makensis) not found. Produced zip only: $zipPath"
  if ($RequireInstaller) { throw $msg }
  Write-Warning $msg
  exit 0
}

$installerOut = Join-Path $outAbs "RaveLand-Installer-Windows.exe"
if (Test-Path -LiteralPath $installerOut) { Remove-Item -Force -LiteralPath $installerOut }

$nsiPath = Join-Path $env:TEMP ("raveland_installer_" + [Guid]::NewGuid().ToString("N") + ".nsi")

$safeName = "RaveLand"
$company = "NS Audio"
$product = "RaveLand"
$uninstKey = "Software\Microsoft\Windows\CurrentVersion\Uninstall\${safeName}"

$nsi = @"
!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"

Name "${product}"
OutFile "${installerOut}"
InstallDir "`$PROGRAMFILES64\\${safeName}"
RequestExecutionLevel admin

VIProductVersion "${Version}.0"
VIAddVersionKey "ProductName" "${product}"
VIAddVersionKey "CompanyName" "${company}"
VIAddVersionKey "FileDescription" "${product} Installer"
VIAddVersionKey "FileVersion" "${Version}"

!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

Section "Install"
  SetOutPath "`$INSTDIR"
  File /oname=RaveLand.exe "${standaloneExe}"

  ; Install VST3 to the standard system-wide location.
  StrCpy `$0 "`$COMMONFILES64\\VST3\\RaveLand.vst3"
  RMDir /r "`$0"
  CreateDirectory "`$0"
  SetOutPath "`$0"
  File /r "${vst3Dir}\*.*"

  ; Add uninstall entry
  WriteUninstaller "`$INSTDIR\\Uninstall.exe"
  WriteRegStr HKLM "${uninstKey}" "DisplayName" "${product}"
  WriteRegStr HKLM "${uninstKey}" "Publisher" "${company}"
  WriteRegStr HKLM "${uninstKey}" "DisplayVersion" "${Version}"
  WriteRegStr HKLM "${uninstKey}" "InstallLocation" "`$INSTDIR"
  WriteRegStr HKLM "${uninstKey}" "UninstallString" "`$INSTDIR\\Uninstall.exe"
SectionEnd

Section "Uninstall"
  Delete "`$INSTDIR\\RaveLand.exe"
  Delete "`$INSTDIR\\Uninstall.exe"
  RMDir /r "`$INSTDIR"
  RMDir /r "`$COMMONFILES64\\VST3\\RaveLand.vst3"
  DeleteRegKey HKLM "${uninstKey}"
SectionEnd
"@

Set-Content -LiteralPath $nsiPath -Value $nsi -Encoding UTF8
try {
  & $makensis /V2 $nsiPath | Write-Host
} finally {
  Remove-Item -Force -LiteralPath $nsiPath -ErrorAction SilentlyContinue
}

if (!(Test-Path -LiteralPath $installerOut)) {
  throw "Installer build failed: $installerOut not created"
}

Write-Host "Wrote: $installerOut"
