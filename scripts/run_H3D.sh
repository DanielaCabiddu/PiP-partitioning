# Define the URL and target paths
$downloadUrl = "https://cnrsc-my.sharepoint.com/:u:/g/personal/daniela_cabiddu_cnr_it/EdinMfuDSExGqYzODGvayugBEzty5darzZK44fTuY6LUbw?e=azc8qC&download=1" 

# Get script directory
$scriptDir = $PSScriptRoot

$downloadPath = Join-Path $scriptDir "..\data\H3D\H3D.las"
$exePath    = Join-Path $scriptDir "..\bin\PBF-FR-partitioning"

$ext_boundary = Join-Path $scriptDir "..\data\H3D\hess_boundary_25832.shp"
$int_boundary = Join-Path $scriptDir "..\data\H3D\hess_25832.shp"

$outputPath = Join-Path $scriptDir "..\data\H3D\output\H3D_partitioning.shp"

$args       = "-l `"$outputPath`" -i `"$int_boundary`" -e `"$ext_boundary`" -o `"$outputPath`" "

# Ensure output directory exists
$outputDir = Split-Path $outputPath
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force
}

# Download only if file doesn't already exist
if (-not (Test-Path $downloadPath)) {
    Write-Host "File not found. Downloading from OneDrive..."
    Invoke-WebRequest -Uri $downloadUrl -OutFile $downloadPath
    Write-Host "Download complete: $downloadPath"
} else {
    Write-Host "File already exists: $downloadPath (skipping download)"
}

# Run the executable
Write-Host "Running executable: $exePath $args"
Start-Process -FilePath $exePath -ArgumentList $args -Wait

Write-Host "Done."