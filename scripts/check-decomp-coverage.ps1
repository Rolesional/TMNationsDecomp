[CmdletBinding()]
param(
    [switch]$RequireComplete
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'toolchain.ps1')

$inventoryPath = Join-Path $WorkspaceRoot 'reverse\ghidra\exports\functions.csv'
$decompiledDirectory = Join-Path $WorkspaceRoot 'reverse\ghidra\exports\decompiled'
$manifestPath = Join-Path $WorkspaceRoot 'reverse\reconstruction-manifest.csv'
foreach ($path in @($inventoryPath, $decompiledDirectory, $manifestPath)) {
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Missing decompilation coverage input: $path"
    }
}

$inventory = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::OrdinalIgnoreCase)
$orderedAddresses = [Collections.Generic.List[string]]::new()
foreach ($row in @(Import-Csv -LiteralPath $inventoryPath)) {
    $address = $row.address.Trim().ToLowerInvariant()
    if ($address -notmatch '^[0-9a-f]{8}$' -or -not $inventory.Add($address)) {
        throw "Invalid or duplicate inventory address: $($row.address)"
    }
    $orderedAddresses.Add($address)
}

$raw = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::OrdinalIgnoreCase)
foreach ($file in @(Get-ChildItem -LiteralPath $decompiledDirectory -File -Filter '*.c')) {
    if ($file.BaseName -notmatch '^([0-9a-f]{8})_') {
        throw "Decompiler export has no entry address: $($file.Name)"
    }
    $address = $Matches[1]
    if (-not $inventory.Contains($address)) {
        throw "Decompiler export is absent from the function inventory: $($file.Name)"
    }
    [void]$raw.Add($address)
}

$reconstructed = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::OrdinalIgnoreCase)
foreach ($row in @(Import-Csv -LiteralPath $manifestPath)) {
    $address = $row.address.Trim().ToLowerInvariant()
    if (-not $inventory.Contains($address) -or -not $raw.Contains($address) -or
        -not $reconstructed.Add($address)) {
        throw "Invalid, unknown, or duplicate reconstruction address: $($row.address)"
    }
    foreach ($property in @('source', 'evidence')) {
        $relativePath = $row.$property
        if ([string]::IsNullOrWhiteSpace($relativePath) -or
            -not (Test-Path -LiteralPath (Join-Path $WorkspaceRoot $relativePath))) {
            throw "Missing $property for reconstructed function $address`: $relativePath"
        }
    }
}

$total = $inventory.Count
$nextRaw = $orderedAddresses | Where-Object { -not $raw.Contains($_) } | Select-Object -First 1
$nextReconstructed =
    $orderedAddresses | Where-Object { -not $reconstructed.Contains($_) } | Select-Object -First 1
Write-Output "inventory=$total"
Write-Output ("rawDecompiled={0}/{1} ({2:F4}%)" -f
    $raw.Count, $total, (100.0 * $raw.Count / $total))
Write-Output ("verifiedReconstructed={0}/{1} ({2:F4}%)" -f
    $reconstructed.Count, $total, (100.0 * $reconstructed.Count / $total))
Write-Output "nextRaw=$nextRaw"
Write-Output "nextReconstructed=$nextReconstructed"

if ($RequireComplete -and ($raw.Count -ne $total -or $reconstructed.Count -ne $total)) {
    throw '100% executable function coverage has not been reached.'
}
