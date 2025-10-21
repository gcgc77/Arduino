# PowerShell script to rename files inside a .zip archive.
#
# Description:
# This script finds all .zip files in the current directory. For each .zip file,
# it extracts the contents, renames each file by adding the .zip filename as a
# prefix followed by a dash, and then creates a new .zip archive with the
# renamed files. The new archive will have the suffix "-renamed".
#
# Usage:
# 1. Open a PowerShell terminal.
# 2. Navigate to the directory containing your .zip files.
# 3. Run the script by executing: .\rename_cbz_contents.ps1

# Get all .zip files in the current directory
$zipFiles = Get-ChildItem -Path . -Filter *.zip

foreach ($zipFile in $zipFiles) {
    # Create a temporary directory
    $tempDir = New-Item -ItemType Directory -Path (Join-Path $env:TEMP ($zipFile.BaseName))

    # Extract the .zip file
    Expand-Archive -Path $zipFile.FullName -DestinationPath $tempDir.FullName

    # Get the base name of the .zip file
    $prefix = $zipFile.BaseName

    # Get all extracted files
    $extractedFiles = Get-ChildItem -Path $tempDir.FullName

    foreach ($file in $extractedFiles) {
        # Rename the file
        $newName = "$prefix-$($file.Name)"
        Rename-Item -Path $file.FullName -NewName $newName
    }

    # Create a new .zip archive with the renamed files
    $newZipPath = Join-Path -Path $zipFile.DirectoryName -ChildPath "$($zipFile.BaseName)-renamed.zip"
    Compress-Archive -Path "$($tempDir.FullName)\*" -DestinationPath $newZipPath

    # Clean up the temporary directory
    Remove-Item -Path $tempDir.FullName -Recurse -Force
}
