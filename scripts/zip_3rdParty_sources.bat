pushd %~dp0..\3rdParty\xcc
echo "Creating a zip of the (possibly patched) sources (and their original downloads) that have been used to build the MissionEditor. This is for reference and ensuring the original sources are still available on demand. Otherwise, this is not for distribution as it contains a lot of temporary (and possibly legally restricted) files""
if EXIST "%~dp0..\dist\MissionEditorExternalSources.zip" (
  del "%~dp0..\dist\MissionEditorExternalSources.zip"
)
tar.exe -a -c --exclude="PowerShell-*.zip"  --exclude="powershell-*" --exclude="x86-windows-dbg" --exclude="x86-windows-rel" --exclude="vcpkg" --exclude="x64-windows" -f "%~dp0..\dist\MissionEditorExternalSources.zip" vcpkg_downloads vcpkg_installed 
popd
