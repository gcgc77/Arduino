# PowerShell script to merge the contents of .zip files into batched volumes.
#
# Description:
# This script processes a collection of .zip files in the current directory.
# The logic is as follows:
# 1. All .zip files in the directory are sorted alphabetically.
# 2. The sorted list of .zip files is processed in chunks of 10.
# 3. For each chunk of 10 .zip files, their contents are extracted into a
#    single temporary "volume" directory.
# 4. To prevent file name conflicts, each extracted file is renamed to include
#    the name of its original .zip archive as a prefix.
# 5. The entire collection of renamed files for that chunk is then archived into
#    a single file named "VOLUME_X.cbz", where X is a sequential number.
#
# Example: 35 .zip files will result in 4 output files:
# - VOLUME_1.cbz (contains contents of zip files 1-10)
# - VOLUME_2.cbz (contains contents of zip files 11-20)
# - VOLUME_3.cbz (contains contents of zip files 21-30)
# - VOLUME_4.cbz (contains contents of zip files 31-35)
#
# The script uses a main temporary directory for all operations and ensures
# it is cleaned up at the end, even if errors occur.

# --- Phase 1: Get and Sort All Input .zip Files ---
$allZipFiles = Get-ChildItem -Path . -Filter *.zip | Sort-Object Name

# Create a unique name for the main temporary directory
$mainTempDirName = [System.IO.Path]::GetRandomFileName()
$mainTempDir = $null

try {
    # Create the main temporary directory
    $mainTempDir = New-Item -ItemType Directory -Path (Join-Path $env:TEMP $mainTempDirName) -Force

    # --- Phase 2: Process .zip Files in Chunks ---
    if ($allZipFiles.Count -gt 0) {
        $volumeCounter = 1
        $filesPerVolume = 10
        for ($i = 0; $i -lt $allZipFiles.Count; $i += $filesPerVolume) {
            # Select the next chunk of zip files
            $zipFileChunk = $allZipFiles[$i..([System.Math]::Min($i + $filesPerVolume - 1, $allZipFiles.Count - 1))]

            # --- Phase 3: For Each Chunk, Create a Volume ---
            # Create a temporary staging directory for this volume's contents
            $volumeStagingDir = New-Item -ItemType Directory -Path (Join-Path $mainTempDir.FullName "VOLUME_$volumeCounter")

            # Loop through each zip in the chunk and process its files
            foreach ($zipFile in $zipFileChunk) {
                # Use a temporary directory for the initial extraction to handle potential duplicate filenames inside a single zip
                $singleZipTempDir = New-Item -ItemType Directory -Path (Join-Path $mainTempDir.FullName $zipFile.BaseName)
                Expand-Archive -Path $zipFile.FullName -DestinationPath $singleZipTempDir.FullName

                # Rename each file with the zip's name as a prefix and move it to the volume staging area
                $extractedFiles = Get-ChildItem -Path $singleZipTempDir.FullName -Recurse -File
                foreach ($file in $extractedFiles) {
                    $newName = "$($zipFile.BaseName)-$($file.Name)"
                    Move-Item -Path $file.FullName -Destination (Join-Path $volumeStagingDir.FullName $newName)
                }
                Remove-Item -Path $singleZipTempDir.FullName -Recurse # Clean up the single zip's temp dir
            }

            # Create the .zip archive for the volume
            $volumeZipPath = Join-Path -Path $allZipFiles[0].DirectoryName -ChildPath "VOLUME_$($volumeCounter).zip"
            Compress-Archive -Path "$($volumeStagingDir.FullName)\*" -DestinationPath $volumeZipPath

            # Rename it to .cbz
            $newCbzName = "VOLUME_$($volumeCounter).cbz"
            Rename-Item -Path $volumeZipPath -NewName $newCbzName

            $volumeCounter++
        }
    }
}
finally {
    # Clean up the main temporary directory if it was created
    if ($mainTempDir -and (Test-Path $mainTempDir.FullName)) {
        Remove-Item -Path $mainTempDir.FullName -Recurse -Force
    }
}
