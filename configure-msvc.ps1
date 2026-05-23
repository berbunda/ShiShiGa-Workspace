# Configure and build ai_workspace with MSVC + Qt WebEngine.
$ErrorActionPreference = "Stop"

$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build-msvc"
$VcVars = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if (-not (Test-Path $VcVars)) {
    throw "MSVC environment script not found: $VcVars"
}

$cmakeArgs = @(
    "-S", "`"$ProjectRoot`"",
    "-B", "`"$BuildDir`"",
    "-G", "Ninja",
    "-DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/msvc2022_64",
    "-DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/Ninja/ninja.exe"
)

Write-Host "Configuring with MSVC + Qt..."
cmd /c "`"$VcVars`" >nul && cmake $($cmakeArgs -join ' ')"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Building..."
cmd /c "`"$VcVars`" >nul && cmake --build `"$BuildDir`""
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host ""
Write-Host "Build complete: $BuildDir\ai_workspace.exe"
Write-Host "First run: deploy Qt libraries with windeployqt --webenginewidgets"
Write-Host "  C:\Qt\6.11.1\msvc2022_64\bin\windeployqt.exe --webenginewidgets `"$BuildDir\ai_workspace.exe`""
exit 0
