# PowerShell script to rename files inside a .zip archive and save as .cbz.
#
# Description:
# This script finds all .zip files in the current directory. For each .zip file,
# it extracts the contents, renames each file (including files in subdirectories)
# by adding the .zip filename as a prefix followed by a dash, and then creates a
# new .cbz archive with the renamed files. The new archive will have the suffix
# "-renamed". The script is designed to be robust, ensuring that temporary files
# are cleaned up even if errors occur.
#
# Usage:
# 1. Open a PowerShell terminal.
# 2. Navigate to the directory containing your .zip files.
# 3. Run the script by executing: .\rename_cbz_contents.ps1

# Get all .zip files in the current directory
$zipFiles = Get-ChildItem -Path . -Filter *.zip

foreach ($zipFile in $zipFiles) {
    $tempDir = $null
    try {
        # Create a temporary directory, forcing it to overwrite if it exists
        $tempDir = New-Item -ItemType Directory -Path (Join-Path $env:TEMP ($zipFile.BaseName)) -Force

        # Extract the .zip file
        Expand-Archive -Path $zipFile.FullName -DestinationPath $tempDir.FullName

        # Get the base name of the .zip file
        $prefix = $zipFile.BaseName

        # Get all extracted files recursively
        $extractedFiles = Get-ChildItem -Path $tempDir.FullName -Recurse -File

        foreach ($file in $extractedFiles) {
            # Rename the file
            $newName = "$prefix-$($file.Name)"
            Rename-Item -Path $file.FullName -NewName $newName
        }

        # Create a new .cbz archive with the renamed files
        $newCbzPath = Join-Path -Path $zipFile.DirectoryName -ChildPath "$($zipFile.BaseName)-renamed.cbz"
        Compress-Archive -Path "$($tempDir.FullName)\*" -DestinationPath $newCbzPath
    }
    finally {
        # Clean up the temporary directory if it was created
        if ($tempDir -and (Test-Path $tempDir.FullName)) {
            Remove-Item -Path $tempDir.FullName -Recurse -Force
        }
    }
}
