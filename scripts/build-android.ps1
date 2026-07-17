$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'toolchain.ps1')

Push-Location $WorkspaceRoot
try {
    & (Join-Path $WorkspaceRoot 'gradlew.bat') --no-daemon :app:assembleDebug
    if ($LASTEXITCODE -ne 0) {
        throw "Android build failed with exit code $LASTEXITCODE"
    }
}
finally {
    Pop-Location
}

