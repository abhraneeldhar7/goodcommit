# scripts/install.ps1 - Windows installer
#
# Usage:
#   irm https://raw.githubusercontent.com/abhraneeldhar7/xommit/main/cli-tool/scripts/install.ps1 | iex

$ErrorActionPreference = "Stop"

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Host "Error: git is not installed."
    Write-Host "Install it from: https://git-scm.com/install/"
    exit 1
}

$Repo = "https://github.com/abhraneeldhar7/xommit/releases/latest/download"
$Asset = "xommit.exe"
$InstallDir = Join-Path $env:LOCALAPPDATA "Programs\xommit"
$Target = Join-Path $InstallDir $Asset

New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null

Write-Host "Downloading $Asset..."
Invoke-WebRequest -Uri "$Repo/$Asset" -OutFile $Target -UseBasicParsing

$UserPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($UserPath -notlike "*$InstallDir*") {
    [Environment]::SetEnvironmentVariable("Path", "$UserPath;$InstallDir", "User")
    Write-Host "Added $InstallDir to your PATH (restart your terminal)."
}

$env:Path += ";$InstallDir"

Write-Host ""
Write-Host "xommit installed successfully."
Write-Host ""
Write-Host "Next steps:"
Write-Host "  1. Set your API key:  xommit --connect"
Write-Host "  2. Stage some files:  git add <files>"
Write-Host "  3. Run:               xommit `"your vague message`""
