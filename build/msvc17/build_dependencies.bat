@echo off

set "CONFIGURATION=%1"

set "VERSION=17"
set "YEAR=2022"

set "BUILD_TEMP_DIR=%~dp0..\..\solutions\dependencies_msvc%VERSION%\%CONFIGURATION%"

@mkdir "%BUILD_TEMP_DIR%"
@pushd "%BUILD_TEMP_DIR%"

call cmake -G "Visual Studio %VERSION% %YEAR%" -A Win32 -S "%CD%\..\..\..\cmake\dependencies_win32" -DCMAKE_CONFIGURATION_TYPES:STRING=%CONFIGURATION% -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION%
call cmake --build .\ --config %CONFIGURATION%

@popd

@pause