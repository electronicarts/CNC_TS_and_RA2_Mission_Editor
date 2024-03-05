# FinalSun / FinalAlert (YR) 2 Mission Editor

This repository contains the source code for the FinalSun/FinalAlert (YR) Mission Editor.

The official version for both applications has been increased to v2.0 in order to illustrate they now run properly on modern operating systems. We also used this opporunity to update the application and fix some known issues.

# Fixes and Changes
- Fixed a few code issues to allow the application to run on modern operating systems.
- Updated the application icons with new 256x256 graphics.
- You can now zoom in & out with the middle mouse button or wheel
- You don't need administrator access anymore if the program is installed in the program folder, as all user data and settings are now stored in your AppData folder
- Tunnel tube editing completely reimplemented
  + Tunnel tilesets available
  + You can edit existing tubes by using the tool on the end of an existing tube
  + You can now create curved tubes
  + You can now create unidirectional tubes
- LAT support for Crystal and Swamp terrain in Tiberian Sun temperate maps
- House colors are now read from map or rules.ini
- Units and buildings are now shaded in the house colors correctly
- Voxel units now have shading applied
- Minimap can now be resized
- Undo steps increased to 64
- Map rendering performance improved
- Several fixes for crashes
- Fixed addon turrets display
- SHP turrets and Voxel turrets + barrels have their positioning fixed.
- Maps up to 400x112 (or 112x400) are now allowed.
- Fixed minimap for non-quadratic maps.

# Install build requirements
- Microsoft Visual Studio 2022
  1. Install Visual Studio 2022 if you haven't done so already.
  2. (Re-)Start the Visual Studio Installer
  3. Apply any updates to Visual Studio 2022
  4. Click on Modify.
  5. Under Workloads ensure "Desktop Development with C++" is enabled. Ensure the details are visible on the right panel below "Desktop Development with C++" and enable the following checkboxes:
     - MSVC v143 - VS 2022 C++-x64/x86 build tools
     - C++-ATL for v143 build tools x86 & x64
     - vcpkg package manager: if this item is missing, please check that you have updated Visual Studio 2022!
     - newest Windows 10 SDK
     - newest Windows 11 SDK
     - C++-MFC for v143 build tools x86 & x64
     - Do not untick any other checkboxes!
  6. Apply the changes.
- Git™ for Windows - Ensure you do not run any other git implementation when using vcpkg.

# Building the source code
The source code consists of 2 projects:
- MissionEditor: The main mission editor application code.
- MissionEditorPackLib: Wraps XCC objects with C functions and includes some loading and packing code.

## Compilation
1)	In this example, we are going to build Final Alert 2 (for Red Alert 2). 
1)	Open MissionEditor.sln with Visual Studio 2022 and set the project configuration to "FinalAlertYRRelease". This will cause an automatic switch to FinalAlert2YR.exe.
2)	Build the project by pressing F7 (Build Solution), once completed it will produce FinalAlert2YR.exe in the "dist/FinalAlert2" subdirectory
3)	You can now run and debug the editor by pressing F5

## Updating third-party libraries
In order to update zlib, bzip, boost and lzo to newest version, start a "Developer Command Prompt for VS 2022", move to the "3rdParty\xcc" folder with vcpkg and Git for Windows in PATH:
   
       git --version
       vcpkg x-update-baseline

A version of XCC is directly included in "3rdParty\xcc". It is a slightly modified and heavily stripped down version. The changes are available in `3rdParty\xcc\patch.<COMMIT_HASH>.diff`, where `COMMIT_HASH` is the commit hash of the XCC repository (at the time of this writing `3rdParty\xcc\patch.70358b46858973426c1ecf204485cb2a88716217.diff`). If you update XCC to a newer version by replacing the files, please also apply the (eventually updated) patch afterwards.

# Creating a distribution
After modifying the source code you might want to create a distribution.
Please ensure that you have taken care of everything required by the licenses (e.g. notices of source code changes and your copyright statement) and that the program itself does make clear it has been modified by you.

There is a helper script that can create the distributions in the script folder:

    cd scripts
    build_and_distribute.bat

This rebuilds everything and creates several files in the dist folder:

  - FinalSun.zip
    - Contains a FinalSun distribution
  - FinalAlert2.zip
    - Contains a FinalAlert2 distribution
  - FinalAlert2YR.zip
    - Contains a FinalAlert2 Yuri's Revenge distribution
  - MissionEditorSource.zip
    - The source code of this git repository (no uncommitted changes included)
  - MissionEditorExternalSources.zip
    - A best-effort dump of third party source code and binaries used for the build. This possibly contains material not for distribution! You should archive this for reference and/or in the event that original sources become unavailable.	

When distributing, you yourself are responsible for fulfilling all license requirements both of this repository and of all third party libraries being used.
There is no warranty that the helper script automatically fulfills all requirements.

# Directories
MissionEditor\data\shared - Data copied to both FS/FA2 distribution
MissionEditor\data\FinalAlert2 - Additional data copied to FA2 distribution
MissionEditor\data\FinalSun - Additional data copied to FS distribution

MissionEditor\PropertySheets - Some property sheets to ease managing the many project configurations

dist - Will contain the output binaries in subfolders. Required DLLs of third-party libraries and the data directory will be copied there using MissionEditor\PropertySheets\common.props

# Contributions
This is an archive repository and will not be maintained. We suggest you fork this repository to fix any issues or add new features.

# Source
Most of the source code has been written around 1999-2001 and does not take advantage of modern C++ features (e.g. smart pointers or RAII) or hardware rendering. However, we've enabled C++20 support and refactored some critical parts to be up to date.

# Authors
- Electronic Arts Inc.
- Matthias Wagner
  + Author of the original FinalSun and FinalAlert 2 (YR)
  + Bug fixes, updates and refactorings
  + Updating the build system
- Olaf van der Spek
  + For the XCC Library
- Luke "CCHyper" Feenan
  + Additional programming
  + Preparing the source code for open source release
  + New icons and various other graphics

# Special Thanks
We'd like to thank the team at Electronic Arts for approving and making the release of the source code possible.

# Legal
The video games "Command & Conquer: Tiberian Sun", "Command & Conquer: Red Alert 2", and "Command & Conquer: Yuri's Revenge" are copyright of Westwood Studios. All Rights Reserved. Westwood Studios is a trademark or registered trademark of Electronic Arts in the U.S. and/or other countries. All rights reserved.

Microsoft, DirectX, Visual C++, Visual Studio and Windows are trademarks of the Microsoft group of companies.

Git and the Git logo are either registered trademarks or trademarks of Software Freedom Conservancy, Inc., corporate home of the Git Project, in the United States and/or other countries.

# License
Unless otherwise stated, the source code provided in this repository is licenced under the [GNU General Public License version 3](<https://www.gnu.org/licenses/gpl-3.0.html>). Please see the accompanying LICENSE file and the actual files of interest.

Third party libraries in the 3rdParty folder may be licensed differently. Please see their accompanying LICENSE or COPYING file and the actual files of interest.
