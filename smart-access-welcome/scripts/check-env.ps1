[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$requiredCommands = @("git", "cmake", "ninja")
$missing = @()

Write-Host "Smart Access Welcome - environment check"
foreach ($commandName in $requiredCommands) {
    $command = Get-Command $commandName -ErrorAction SilentlyContinue
    if ($null -eq $command) {
        Write-Warning "$commandName was not found in PATH"
        $missing += $commandName
    } else {
        Write-Host ("[OK] {0}: {1}" -f $commandName, $command.Source)
    }
}

foreach ($variableName in @("CMAKE_PREFIX_PATH", "OpenCV_DIR")) {
    $value = [Environment]::GetEnvironmentVariable($variableName)
    if ([string]::IsNullOrWhiteSpace($value)) {
        Write-Warning "$variableName is not configured"
    } else {
        Write-Host ("[OK] {0}: {1}" -f $variableName, $value)
    }
}

if ($missing.Count -gt 0) {
    Write-Error ("Missing required commands: " + ($missing -join ", "))
}

Write-Host "Environment command check passed. Dependency discovery is verified during CMake configuration."

