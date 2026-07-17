$ErrorActionPreference = 'Stop'

$script:WorkspaceRoot = Split-Path -Parent $PSScriptRoot
$localJdk = Join-Path $script:WorkspaceRoot '.tools\jdk-21.0.11+10'
if (Test-Path -LiteralPath $localJdk) {
    $env:JAVA_HOME = $localJdk
    $env:Path = (Join-Path $localJdk 'bin') + [IO.Path]::PathSeparator + $env:Path
}

if (-not $env:ANDROID_HOME) {
    $env:ANDROID_HOME = Join-Path $env:LOCALAPPDATA 'Android\Sdk'
}
$env:ANDROID_SDK_ROOT = $env:ANDROID_HOME

