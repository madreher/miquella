@echo off

:: Below line allows setting variables inside loops and conditionals and reading current values with !!
:: https://devblogs.microsoft.com/oldnewthing/20060823-00/?p=29993
setlocal enabledelayedexpansion

:: Get directory where this script resides
:: https://stackoverflow.com/questions/17063947/get-current-batchfile-directory
set DIR=%~dp0

:: To have the script exit early on error, pipe each command into exit
::ping -invalid-arg || echo ERROR && exit /b

set DISTRO_CODENAME=windows

:: Build configuration
set BUILD_CONFIG=Release


::===============================
:: Get GIT information
pushd %DIR%
:: To set the output of a command to a variable, see
:: https://stackoverflow.com/questions/16203629/assign-command-output-to-variable-in-batch-file

git submodule update --init

for /f "delims=" %%i in ('git rev-parse --verify HEAD') do set GIT_COMMIT_HASH=%%i

popd



::===============================
:: Create a new conan profile from detected properties
:: --detect => autodetect settings and fill [settings] section
:: --force => overwrite existing profile with given name, if exists
:: autodetect => name of created profile
conan profile new ^
--detect ^
--force ^
autodetect

:: Install missing Conan packages
:: --settings => apply an overwriting setting
::   compiler.cppstd => set the C++ standard to C++ 17 with GNU extensions
:: --build missing => build packages from source whose binary package is not found
:: --generator => generator to use
:: --profile autodetect => Use the 'autodetect' profile as the host and build profile
conan install %DIR%\conanfile_win.txt ^
--build missing ^
--settings build_type=%BUILD_CONFIG% ^
--settings compiler.cppstd=17 ^
--profile autodetect ^
--generator cmake_find_package ^
--generator cmake_paths


::===============================
:: Replace \ with / for cmake
set "DIR=%DIR:\=/%"

::  -D CMAKE_MODULE_PATH    =>  Semicolon-separated list of directories, using forward slashes,
::                              specifying a search path for CMake modules to be loaded by the
::                              include() or find_package() commands before checking the default
::                              modules that come with CMake
::  -D CMAKE_PREFIX_PATH    =>  Semicolon-separated list of directories specifying installation
::                              prefixes to be searched by the find_package(), find_program(),
::                              find_file(), and find_path() commands. Each command will add
::                              appropriate subdirectories (like bin, lib, or include) as
::                              specified in its own documentation
::  -S                      =>  Source directory
::  -B                      =>  Build directory
cmake ^
-D CMAKE_MODULE_PATH="%DIR%build" ^
-D CMAKE_PREFIX_PATH="%DIR%build" ^
-D CMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
-S "%DIR%" ^
-B "%DIR%build


::===============================
echo "=================="
echo "Build Information:"
echo "  GIT_COMMIT_HASH : %GIT_COMMIT_HASH%"
echo "  DISTRO          : %DISTRO_CODENAME%"