param(
    [Parameter(Mandatory = $true)]
    [string[]]$Address
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'toolchain.ps1')

$ghidra = Join-Path $WorkspaceRoot '.tools\ghidra_12.1.2_PUBLIC'
$headless = Join-Path $ghidra 'support\analyzeHeadless.bat'
$projectDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\projects'
$scriptDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\scripts'
$outputDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\exports\decompiled'

$arguments = @(
    $projectDirectory,
    'TmForever-2.11.11',
    '-process', 'TmForever.exe',
    '-noanalysis',
    '-scriptPath', $scriptDirectory,
    '-postScript', 'DecompileAt.java', $outputDirectory
) + $Address

& $headless @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Ghidra decompilation failed with exit code $LASTEXITCODE"
}

