param (
    [string]$exePath
    [string]$outputLASPath
)

# Get script directory
$scriptDir = $PSScriptRoot

# Define the URL and target paths
$downloadUrl = "https://cnrsc-my.sharepoint.com/:u:/g/personal/daniela_cabiddu_cnr_it/EdinMfuDSExGqYzODGvayugBEzty5darzZK44fTuY6LUbw?e=azc8qC&download=1" 

if (-not $exePath) {
    # Default path if not passed in
    $exePath = Join-Path $scriptDir "..\bin\PBF-FR-partitioning.exe"
}

if (-not $outputLASPath)
{
    # Default path if not passed in
    $outputLASPath = Join-Path $scriptDir "..\data\H3D\output_LAS\"
}

$downloadPath = Join-Path $scriptDir "..\data\H3D\H3D.las"

$ext_boundary = Join-Path $scriptDir "..\data\H3D\hess_boundary_25832.shp"
$int_boundary = Join-Path $scriptDir "..\data\H3D\hess_25832.shp"

$outputLASPath = Join-Path $scriptDir "..\data\H3D\output_LAS\"
$outputSHPFile = Join-Path $scriptDir "..\data\H3D\output_SHP\H3D_partitioning.shp"

$args       = "-l `"$downloadPath`" -i `"$int_boundary`" -e `"$ext_boundary`" -L `"$outputLASPath`" -O `"$outputSHPFile`" "

# Ensure output directory exists
$outputDir = Split-Path $outputSHPFile
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force
}

# Download only if file doesn't already exist
if (-not (Test-Path $downloadPath)) {
    Write-Host "File not found. Downloading from OneDrive..."
    #Invoke-WebRequest -Uri $downloadUrl -OutFile $downloadPath
    Start-BitsTransfer -Source $downloadUrl -Destination $downloadPath
    Write-Host "Download complete: $downloadPath"
} else {
    Write-Host "File already exists: $downloadPath (skipping download)"
}

# Run the executable
Write-Host "Running executable: $exePath $args"
Start-Process -FilePath $exePath -ArgumentList $args -Wait

Write-Host "Done."