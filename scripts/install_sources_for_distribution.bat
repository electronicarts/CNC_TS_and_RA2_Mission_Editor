@echo off
echo This script not only installs the vcpkg packages (this is usually done automatically when building), but also makes the downloaded sources and the build sources (with VCPKG patches applied) available in this repository. This may be helpful for distributing, archiving and license compliance and is not required for daily development.

if "%VCINSTALLDIR%" == "" (
  if not exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    echo Please activate a developer command prompt.
    exit /b 1
  )
)

@echo on
pushd "%~dp0..\3rdParty\xcc"
rmdir /Q /S "%~dp0..\3rdParty\xcc\vcpkg_installed"
vcpkg install --binarysource clear "--downloads-root=%~dp0..\3rdParty\xcc\vcpkg_downloads" "--x-install-root=%~dp0..\3rdParty\xcc\vcpkg_installed\x86-windows" "--x-buildtrees-root=%~dp0..\3rdParty\xcc\vcpkg_installed\x86-windows\_buildtrees" --triplet x86-windows
popd