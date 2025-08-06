# Define the URL of the LAS file on OneDrive (direct download link)
$downloadUrl = "https://cnrsc-my.sharepoint.com/:u:/g/personal/daniela_cabiddu_cnr_it/EdinMfuDSExGqYzODGvayugBEzty5darzZK44fTuY6LUbw?e=azc8qC&download=1"

# Get the script's directory
$scriptDir = $PSScriptRoot

# Define paths relative to script location
$downloadPath = Join-Path $scriptDir "..\data\H3D\H3D.las"
$exePath      = Join-Path $scriptDir "..\bin\PBF-FR-partitioning"

$ext_boundary = Join-Path $scriptDir "..\data\H3D\hess_boundary_25832.shp"
$int_boundary = Join-Path $scriptDir "..\data\H3D\hess_25832.shp"
$outputPath   = Join-Path $scriptDir "..\data\H3D\output\H3D_partitioning.shp"

# Build the argument string (careful with quoting)
$args = "-l `"$downloadPath`" -i `"$int_boundary`" -e `"$ext_boundary`" -o `"$outputPath`""

# Ensure output directory exists
$outputDir = Split-Path $outputPath
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

# Download the LAS file only if it's not already present
if (-not (Test-Path $downloadPath)) {
    Write-Host "LAS file not found. Downloading from OneDrive..."
    try {
        Invoke-WebRequest -Uri $downloadUrl -OutFile $downloadPath -UseBasicParsing
        Write-Host "Download complete: $downloadPath"
    } catch {
        Write-Error "Failed to download file from OneDrive: $_"
        exit 1
    }
} else {
    Write-Host "LAS file already exists: $downloadPath (skipping download
