@echo off
echo This script is a convenience script for building a full FinalSun and FinalAlert 2 YR distribution. It is not required for daily development.

if "%VCINSTALLDIR%" == "" (
  if not exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    echo Visual Studio 2022 Community Edition is not installed in "%ProgramFiles%\Microsoft Visual Studio\2022\Community". If you use another edition of Visual Studio, please activate a developer command prompt.
    exit /b 1
  )
  call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x86 -host_arch=x86
)


@echo on
pushd "%~dp0\.."
rmdir /Q /S "%~dp0..\3rdParty\xcc\vcpkg_installed"
msbuild "-p:VcpkgAdditionalInstallOptions=--binarysource clear """--downloads-root=%~dp0..\3rdParty\xcc\vcpkg_downloads""" """--x-buildtrees-root=%~dp0..\3rdParty\xcc\vcpkg_installed\x86-windows\_buildtrees"""" "-p:Configuration=FinalAlertRelease YR" -p:Platform=Win32 -p:DistributeMissionEditor=true /t:Rebuild MissionEditor.sln
msbuild "-p:VcpkgAdditionalInstallOptions=--binarysource clear """--downloads-root=%~dp0..\3rdParty\xcc\vcpkg_downloads""" """--x-buildtrees-root=%~dp0..\3rdParty\xcc\vcpkg_installed\x86-windows\_buildtrees"""" "-p:Configuration=FinalAlertRelease" -p:Platform=Win32 -p:DistributeMissionEditor=true /t:Rebuild MissionEditor.sln
msbuild "-p:VcpkgAdditionalInstallOptions=--binarysource clear """--downloads-root=%~dp0..\3rdParty\xcc\vcpkg_downloads""" """--x-buildtrees-root=%~dp0..\3rdParty\xcc\vcpkg_installed\x86-windows\_buildtrees"""" "-p:Configuration=FinalSunRelease" -p:Platform=Win32 -p:DistributeMissionEditor=true /t:Rebuild MissionEditor.sln

popd

pushd "%~dp0"
call zip_sources.bat
call zip_3rdParty_sources.bat
popd

