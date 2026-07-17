param(
    [Parameter(Mandatory = $true)]
    [string[]]$Address,
    [string]$Output = 'reverse\ghidra\exports\address-references.csv'
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
    '-postScript', 'TraceAddressReferences.java', $outputPath
) + $Address

& $headless @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Ghidra address-reference trace failed with exit code $LASTEXITCODE"
}
