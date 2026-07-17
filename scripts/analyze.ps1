param(
    [switch]$Reimport
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'toolchain.ps1')

$ghidra = Join-Path $WorkspaceRoot '.tools\ghidra_12.1.2_PUBLIC'
$headless = Join-Path $ghidra 'support\analyzeHeadless.bat'
$inputBinary = Join-Path $WorkspaceRoot 'TmNationsForever\TmForever.exe'
$projectDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\projects'
$scriptDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\scripts'
$exportDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\exports'

New-Item -ItemType Directory -Force -Path $projectDirectory, $exportDirectory | Out-Null

$arguments = @(
    $projectDirectory,
    'TmForever-2.11.11',
    '-scriptPath', $scriptDirectory,
    '-analysisTimeoutPerFile', '3600'
)

if ($Reimport -or -not (Test-Path -LiteralPath (Join-Path $projectDirectory 'TmForever-2.11.11.gpr'))) {
    $arguments += @('-import', $inputBinary, '-overwrite')
}
else {
    $arguments += @('-process', 'TmForever.exe', '-noanalysis')
}

$arguments += @(
    '-postScript', 'ExportProgramInventory.java', $exportDirectory,
    '-postScript', 'ExportClassRegistry.java', $exportDirectory
)

& $headless @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Ghidra analysis failed with exit code $LASTEXITCODE"
}
