param(
    [Parameter(Mandatory = $true)][string]$QtRoot,
    [Parameter(Mandatory = $true)][string]$MingwRoot,
    [Parameter(Mandatory = $true)][string]$OpenCvRoot,
    [ValidateSet('debug', 'release')][string]$Configuration = 'release',
    [switch]$Deploy
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$qmake = Join-Path $QtRoot 'bin/qmake.exe'
$make = Join-Path $MingwRoot 'bin/mingw32-make.exe'
$deployQt = Join-Path $QtRoot 'bin/windeployqt.exe'
foreach ($path in @($qmake, $make, (Join-Path $OpenCvRoot 'include/opencv2/core.hpp'))) {
    if (!(Test-Path -LiteralPath $path)) { throw "Required dependency not found: $path" }
}

$env:Path = (Join-Path $MingwRoot 'bin') + ';' + (Join-Path $QtRoot 'bin') + ';' + $env:Path
$env:OPENCV_ROOT = $OpenCvRoot.Replace('\', '/')
$buildDirectory = Join-Path $projectRoot "build/terminal-$Configuration"
New-Item -ItemType Directory -Force -Path $buildDirectory | Out-Null
Push-Location $buildDirectory
try {
    & $qmake '../../smart-access-welcome.pro' "CONFIG+=$Configuration"
    if ($LASTEXITCODE -ne 0) { throw 'qmake configure failed' }
    & $make -j4
    if ($LASTEXITCODE -ne 0) { throw 'application build failed' }
    if ($Deploy) {
        & $deployQt --no-translations --compiler-runtime './bin/SmartAccessWelcome.exe'
        if ($LASTEXITCODE -ne 0) { throw 'Qt deployment failed' }
        Copy-Item -LiteralPath (Join-Path $projectRoot 'config/config.example.json') `
                  -Destination './bin/config.example.json' -Force
    }
} finally {
    Pop-Location
}
