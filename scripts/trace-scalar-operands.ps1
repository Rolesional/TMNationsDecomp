param(
    [string]$Output = 'reverse\ghidra\exports\cgameinterface-field-offsets.csv',
    [string[]]$Offset = @('0x14c', '0x150', '0x154', '0x158', '0x160')
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'toolchain.ps1')

$ghidra = Join-Path $WorkspaceRoot '.tools\ghidra_12.1.2_PUBLIC'
$headless = Join-Path $ghidra 'support\analyzeHeadless.bat'
$projectDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\projects'
$scriptDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\scripts'
$outputPath = Join-Path $WorkspaceRoot $Output

$arguments = @(
    $projectDirectory,
    'TmForever-2.11.11',
    '-process', 'TmForever.exe',
    '-noanalysis',
    '-scriptPath', $scriptDirectory,
    '-postScript', 'TraceScalarOperands.java', $outputPath
) + $Offset

& $headless @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Ghidra scalar-operand trace failed with exit code $LASTEXITCODE"
}
