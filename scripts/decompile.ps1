[CmdletBinding(DefaultParameterSetName = 'Address')]
param(
    [Parameter(Mandatory = $true, ParameterSetName = 'Address')]
    [ValidatePattern('^(?:0x)?[0-9A-Fa-f]{8}$')]
    [string[]]$Address,

    [Parameter(Mandatory = $true, ParameterSetName = 'Missing')]
    [switch]$Missing,

    [Parameter(ParameterSetName = 'Missing')]
    [ValidateRange(1, 500)]
    [int]$Limit = 500
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'toolchain.ps1')

$ghidra = Join-Path $WorkspaceRoot '.tools\ghidra_12.1.2_PUBLIC'
$headless = Join-Path $ghidra 'support\analyzeHeadless.bat'
$projectDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\projects'
$scriptDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\scripts'
$outputDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\exports\decompiled'

if ($Missing) {
    $inventoryPath = Join-Path $WorkspaceRoot 'reverse\ghidra\exports\functions.csv'
    if (-not (Test-Path -LiteralPath $inventoryPath)) {
        throw "Missing Ghidra function inventory: $inventoryPath"
    }

    $existing = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::OrdinalIgnoreCase)
    Get-ChildItem -LiteralPath $outputDirectory -File -Filter '*.c' -ErrorAction SilentlyContinue |
        ForEach-Object {
            if ($_.BaseName -match '^([0-9a-f]{8})_') {
                [void]$existing.Add($Matches[1])
            }
        }
    $Address = @(
        Import-Csv -LiteralPath $inventoryPath |
            Where-Object { -not $existing.Contains($_.address) } |
            Select-Object -First $Limit -ExpandProperty address
    )
    if ($Address.Count -eq 0) {
        Write-Output 'All inventoried functions already have raw decompiler exports.'
        return
    }
}

Write-Output "Decompiling $($Address.Count) function(s)."

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
