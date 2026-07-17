$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'toolchain.ps1')

$cmake = Join-Path $env:ANDROID_HOME 'cmake\3.22.1\bin\cmake.exe'
$ctest = Join-Path $env:ANDROID_HOME 'cmake\3.22.1\bin\ctest.exe'
$buildDirectory = Join-Path $WorkspaceRoot 'build\host'

$configureArguments = @(
    '-S', $WorkspaceRoot,
    '-B', $buildDirectory,
    '-G', 'Ninja',
    '-DCMAKE_BUILD_TYPE=RelWithDebInfo'
)
$fallbackZlibRoot = 'C:\msys64\mingw64'
if (Test-Path -LiteralPath (Join-Path $fallbackZlibRoot 'lib\libz.a')) {
    $configureArguments += "-DTMNF_ZLIB_LIBRARY=$fallbackZlibRoot\lib\libz.a"
}

& $cmake @configureArguments
if ($LASTEXITCODE -ne 0) {
    throw "Host CMake configuration failed with exit code $LASTEXITCODE"
}
& $cmake --build $buildDirectory
if ($LASTEXITCODE -ne 0) {
    throw "Host tools build failed with exit code $LASTEXITCODE"
}
& $ctest --test-dir $buildDirectory --output-on-failure
if ($LASTEXITCODE -ne 0) {
    throw "Host tests failed with exit code $LASTEXITCODE"
}
