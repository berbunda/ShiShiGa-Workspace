# Configure, build, and deploy ShiShiGa-Workspace with MSVC + Qt WebEngine.
$ErrorActionPreference = "Stop"

function Move-DeployArtifactsToEngine {
    param(
        [Parameter(Mandatory)][string]$DistDir,
        [Parameter(Mandatory)][string]$ExecutableName
    )

    $engineDir = Join-Path $DistDir "engine"
    New-Item -ItemType Directory -Path $engineDir -Force | Out-Null

    foreach ($dirName in @("resources", "qml", "translations")) {
        $source = Join-Path $DistDir $dirName
        if (-not (Test-Path $source)) {
            continue
        }

        $dest = Join-Path $engineDir $dirName
        if (Test-Path $dest) {
            Copy-Item -Path (Join-Path $source "*") -Destination $dest -Recurse -Force
            Remove-Item -Path $source -Recurse -Force
        } else {
            Move-Item -Path $source -Destination $dest
        }
    }

    Get-ChildItem -Path $DistDir -File | Where-Object {
        $_.Name -ne $ExecutableName -and
        $_.Name -ne "qt.conf" -and
        ($_.Extension -in ".dll", ".exe", ".pak", ".dat", ".bin")
    } | ForEach-Object {
        Move-Item -Path $_.FullName -Destination $engineDir -Force
    }
}

function New-EngineLocalJunction {
    param(
        [Parameter(Mandatory)][string]$DistDir,
        [Parameter(Mandatory)][string]$ExecutableName
    )

    $engineDir = Join-Path $DistDir "engine"
    $localDir = Join-Path $DistDir "$ExecutableName.local"

    if (Test-Path $localDir) {
        Remove-Item $localDir -Force -Recurse
    }

    cmd /c "mklink /J `"$localDir`" `"$engineDir`""
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to create engine junction: $localDir -> $engineDir"
    }
}

function Write-QtConf {
    param(
        [Parameter(Mandatory)][string]$DistDir,
        [Parameter(Mandatory)][string]$ProjectRoot
    )

    $sourceQtConf = Join-Path $ProjectRoot "resources/qt.conf"
    if (-not (Test-Path $sourceQtConf)) {
        throw "qt.conf template not found: $sourceQtConf"
    }

    Copy-Item -Path $sourceQtConf -Destination (Join-Path $DistDir "qt.conf") -Force
}

function Deploy-QtRuntime {
    param(
        [Parameter(Mandatory)][string]$DistDir,
        [Parameter(Mandatory)][string]$ExecutableName,
        [Parameter(Mandatory)][string]$QtRoot,
        [Parameter(Mandatory)][string]$VcVars,
        [Parameter(Mandatory)][string]$ProjectRoot
    )

    $windeployqt = Join-Path $QtRoot "bin/windeployqt.exe"
    if (-not (Test-Path $windeployqt)) {
        throw "windeployqt not found: $windeployqt"
    }

    $executablePath = Join-Path $DistDir $ExecutableName
    if (-not (Test-Path $executablePath)) {
        throw "Executable not found: $executablePath"
    }

    Write-Host "Deploying Qt runtime to $DistDir..."

    $distDirNative = (Resolve-Path $DistDir).Path
    $deployArgs = @(
        "--webenginewidgets",
        "--release",
        "--compiler-runtime",
        "--libdir", "engine",
        "--plugindir", "engine/plugins",
        "--translationdir", "engine/translations",
        "--force",
        $ExecutableName
    ) -join " "

    $deployCmd = "cd /d `"$distDirNative`" && `"$windeployqt`" $deployArgs"
    cmd /c "`"$VcVars`" >nul && $deployCmd"
    if ($LASTEXITCODE -ne 0) {
        throw "windeployqt failed with exit code $LASTEXITCODE"
    }

    Move-DeployArtifactsToEngine -DistDir $DistDir -ExecutableName $ExecutableName
    Write-QtConf -DistDir $DistDir -ProjectRoot $ProjectRoot
    New-EngineLocalJunction -DistDir $DistDir -ExecutableName $ExecutableName
}

$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build-msvc"
$AppName = "ShiShiGa-Workspace"
$DistDir = Join-Path $BuildDir "dist/$AppName"
$QtRoot = "C:/Qt/6.11.1/msvc2022_64"
$VcVars = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if (-not (Test-Path $VcVars)) {
    throw "MSVC environment script not found: $VcVars"
}

Write-Host "Cleaning portable release directory..."
if (Test-Path $DistDir) {
    Remove-Item $DistDir -Recurse -Force
}
New-Item -ItemType Directory -Path $DistDir -Force | Out-Null

$cmakeArgs = @(
    "-S", "`"$ProjectRoot`"",
    "-B", "`"$BuildDir`"",
    "-G", "Ninja",
    "-DCMAKE_BUILD_TYPE=Release",
    "-DCMAKE_PREFIX_PATH=$QtRoot",
    "-DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/Ninja/ninja.exe"
)

Write-Host "Configuring with MSVC + Qt..."
cmd /c "`"$VcVars`" >nul && cmake $($cmakeArgs -join ' ')"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Building..."
cmd /c "`"$VcVars`" >nul && cmake --build `"$BuildDir`""
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$BuiltExe = Join-Path $BuildDir "$AppName.exe"
if (-not (Test-Path $BuiltExe)) {
    throw "Build output not found: $BuiltExe"
}

Write-Host "Copying $AppName.exe to portable release..."
Copy-Item -Path $BuiltExe -Destination (Join-Path $DistDir "$AppName.exe") -Force

Deploy-QtRuntime -DistDir $DistDir -ExecutableName "$AppName.exe" -QtRoot $QtRoot -VcVars $VcVars -ProjectRoot $ProjectRoot

Write-Host ""
Write-Host "Portable release ready: $DistDir"
Write-Host "Run: $DistDir\$AppName.exe"
exit 0
