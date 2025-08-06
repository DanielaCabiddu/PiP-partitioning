# Define the URL and target paths

# Get script directory
$scriptDir = $PSScriptRoot

$exePath    = Join-Path $scriptDir "..\bin\PBF-FR-partitioning.exe"

$ext_boundary = Join-Path $scriptDir "..\data\example\example_boundary.shp"
$int_boundary = Join-Path $scriptDir "..\data\example\example_buildings.shp"

$outputPath = Join-Path $scriptDir "..\data\example\output\example_partitioning.shp"

$args       = "-i `"$int_boundary`" -e `"$ext_boundary`" -o `"$outputPath`" "

# Ensure output directory exists
$outputDir = Split-Path $outputPath
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force
}

# Run the executable
Write-Host "Running executable: $exePath $args"
Start-Process -FilePath $exePath -ArgumentList $args -Wait

Write-Host "Done."