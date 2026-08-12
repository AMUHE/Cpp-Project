param(
    [Parameter(Mandatory = $true)][string]$QtRoot,
    [Parameter(Mandatory = $true)][string]$MingwRoot
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$qmake = Join-Path $QtRoot 'bin/qmake.exe'
$make = Join-Path $MingwRoot 'bin/mingw32-make.exe'
if (!(Test-Path -LiteralPath $qmake) -or !(Test-Path -LiteralPath $make)) {
    throw 'Qt qmake or MinGW make was not found.'
}

$env:Path = (Join-Path $MingwRoot 'bin') + ';' + (Join-Path $QtRoot 'bin') + ';' + $env:Path
$tests = @('access_policy_test', 'access_server_test', 'infrastructure_test')
foreach ($test in $tests) {
    $buildDirectory = Join-Path $projectRoot "build-tests/$test"
    New-Item -ItemType Directory -Force -Path $buildDirectory | Out-Null
    Push-Location $buildDirectory
    try {
        & $qmake "../../tests/qmake/$test.pro" CONFIG+=debug
        if ($LASTEXITCODE -ne 0) { throw "qmake failed for $test" }
        & $make -j4
        if ($LASTEXITCODE -ne 0) { throw "build failed for $test" }
        & "./bin/$test.exe" -txt
        if ($LASTEXITCODE -ne 0) { throw "test failed: $test" }
    } finally {
        Pop-Location
    }
}
