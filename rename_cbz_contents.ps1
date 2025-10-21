# PowerShell script to rename files inside a .cbz archive.
#
# Description:
# This script finds all .cbz files in the current directory. For each .cbz file,
# it extracts the contents, renames each file by adding the .cbz filename as a
# prefix followed by a dash, and then creates a new .cbz archive with the
# renamed files. The new archive will have the suffix "-renamed".
#
# Usage:
# 1. Open a PowerShell terminal.
# 2. Navigate to the directory containing your .cbz files.
# 3. Run the script by executing: .\rename_cbz_contents.ps1

# Get all .cbz files in the current directory
$cbzFiles = Get-ChildItem -Path . -Filter *.cbz

foreach ($cbzFile in $cbzFiles) {
    # Create a temporary directory
    $tempDir = New-Item -ItemType Directory -Path (Join-Path $env:TEMP ($cbzFile.BaseName))

    # Extract the .cbz file
    Expand-Archive -Path $cbzFile.FullName -DestinationPath $tempDir.FullName

    # Get the base name of the .cbz file
    $prefix = $cbzFile.BaseName

    # Get all extracted files
    $extractedFiles = Get-ChildItem -Path $tempDir.FullName

    foreach ($file in $extractedFiles) {
        # Rename the file
        $newName = "$prefix-$($file.Name)"
        Rename-Item -Path $file.FullName -NewName $newName
    }

    # Create a new .cbz archive with the renamed files
    $newCbzPath = Join-Path -Path $cbzFile.DirectoryName -ChildPath "$($cbzFile.BaseName)-renamed.cbz"
    Compress-Archive -Path "$($tempDir.FullName)\*" -DestinationPath $newCbzPath

    # Clean up the temporary directory
    Remove-Item -Path $tempDir.FullName -Recurse -Force
}
