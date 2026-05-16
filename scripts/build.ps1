<#
.SYNOPSIS
    Build the active plugin in PluginSkeleton.

.PARAMETER Config
    Build configuration: Release (default) or Debug.

.PARAMETER Generator
    CMake generator. Defaults to "Visual Studio 17 2022".
    Use "Visual Studio 16 2019" for VS2019, or "Ninja" if Ninja is on PATH.

.PARAMETER Clean
    Remove build/ directory before configuring.

.EXAMPLE
    .\scripts\build.ps1
    .\scripts\build.ps1 -Config Debug
    .\scripts\build.ps1 -Clean
#>

param(
    [string] $Config    = "Release",
    [string] $Generator = "Visual Studio 18 2026",
    [switch] $Clean
)

$ErrorActionPreference = "Stop"
$Root     = Split-Path $PSScriptRoot -Parent
$BuildDir = Join-Path $Root "build"

Set-Location $Root

if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Removing build directory..." -ForegroundColor Yellow
    Remove-Item $BuildDir -Recurse -Force
}

Write-Host "Configuring with '$Generator'..." -ForegroundColor Cyan
if ($Generator -eq "Ninja") {
    cmake -B $BuildDir -G "Ninja" -DCMAKE_BUILD_TYPE=$Config
} else {
    cmake -B $BuildDir -G $Generator -A x64
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed." -ForegroundColor Red
    exit 1
}

Write-Host "Building $Config..." -ForegroundColor Cyan
cmake --build $BuildDir --config $Config --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Build succeeded." -ForegroundColor Green
Write-Host ""

# Report all VST3 artefacts
$vst3s = Get-ChildItem -Path $BuildDir -Recurse -Filter "*.vst3" -Directory -ErrorAction SilentlyContinue
if ($vst3s) {
    Write-Host "VST3 output:" -ForegroundColor Cyan
    foreach ($v in $vst3s) {
        Write-Host "  $($v.FullName)"
    }
} else {
    Write-Host "No .vst3 artefact found — check build output." -ForegroundColor Yellow
}
