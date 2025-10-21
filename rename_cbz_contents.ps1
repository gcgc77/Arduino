# PowerShell script to aggregate and volume-package comic archives.
#
# Description:
# This script finds all .zip files in the current directory, treating them as
# comic book archives. It performs the following steps:
# 1. Creates a single temporary directory to act as a staging area.
# 2. For each .zip file, it extracts its contents and renames every file by
#    prefixing it with the name of its original .zip archive to prevent name
#    collisions.
# 3. All renamed files are collected into the main staging directory.
# 4. After processing all .zip files, it gets a single, alphabetically sorted
#    list of all the renamed files.
# 5. It then creates multiple .cbz archives, named "VOLUME_X.cbz", where X is a
#    sequential number. Each archive contains a batch of up to 10 files from
#    the sorted collection.
#
# To work around a limitation in PowerShell's Compress-Archive command, the
# script first creates a .zip file for each volume and then renames it to .cbz.
# The script is designed to be robust, ensuring that all temporary files are
# cleaned up at the end, even if errors occur.
#
# Usage:
# 1. Open a PowerShell terminal.
# 2. Navigate to the directory containing your .zip files.
# 3. Run the script by executing: .\rename_cbz_contents.ps1

# Get all .zip files in the current directory
$zipFiles = Get-ChildItem -Path . -Filter *.zip

# Create a unique name for the main temporary directory
$mainTempDirName = [System.IO.Path]::GetRandomFileName()
$mainTempDir = $null

try {
    # Create a single main temporary directory to aggregate all files
    $mainTempDir = New-Item -ItemType Directory -Path (Join-Path $env:TEMP $mainTempDirName) -Force

    # --- Phase 1: Extract and Rename All Files ---
    foreach ($zipFile in $zipFiles) {
        # Create a temporary subdirectory for this specific zip to avoid name conflicts during extraction
        $subTempDir = New-Item -ItemType Directory -Path (Join-Path $mainTempDir.FullName $zipFile.BaseName)

        # Extract the archive
        Expand-Archive -Path $zipFile.FullName -DestinationPath $subTempDir.FullName

        # Rename and move files to the main temp directory
        $extractedFiles = Get-ChildItem -Path $subTempDir.FullName -Recurse -File
        foreach ($file in $extractedFiles) {
            $newName = "$($zipFile.BaseName)-$($file.Name)"
            Move-Item -Path $file.FullName -Destination (Join-Path $mainTempDir.FullName $newName)
        }
        # Clean up the empty subdirectory
        Remove-Item -Path $subTempDir.FullName -Recurse
    }

    # --- Phase 2: Sort All Aggregated Files ---
    $allRenamedFiles = Get-ChildItem -Path $mainTempDir.FullName -File | Sort-Object Name

    # --- Phase 3: Create Batched Volume Archives ---
    if ($allRenamedFiles.Count -gt 0) {
        $volumeCounter = 1
        $filesPerVolume = 10
        for ($i = 0; $i -lt $allRenamedFiles.Count; $i += $filesPerVolume) {
            # Create a temporary subdirectory for this volume's contents
            $volumeContentDir = New-Item -ItemType Directory -Path (Join-Path $mainTempDir.FullName "VOLUME_$volumeCounter")

            # Select the next batch of files to move
            $filesToMove = $allRenamedFiles[$i..([System.Math]::Min($i + $filesPerVolume - 1, $allRenamedFiles.Count - 1))]
            foreach ($fileToMove in $filesToMove) {
                Move-Item -Path $fileToMove.FullName -Destination $volumeContentDir.FullName
            }

            # Create the .zip archive
            $volumeZipPath = Join-Path -Path $zipFiles[0].DirectoryName -ChildPath "VOLUME_$($volumeCounter).zip"
            Compress-Archive -Path "$($volumeContentDir.FullName)\*" -DestinationPath $volumeZipPath

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
