@echo off

set "CONFIGURATION=%1"

set "BUILD_TEMP_DIR=%~dp0..\..\solutions\dependencies_msvc16\%CONFIGURATION%"

@mkdir "%BUILD_TEMP_DIR%"
@pushd "%BUILD_TEMP_DIR%"

call cmake -G "Visual Studio 16 2019" -A Win32 -S "%CD%\..\..\..\cmake\dependencies_win32" -DCMAKE_CONFIGURATION_TYPES:STRING=%CONFIGURATION% -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION%
call cmake --build .\ --config %CONFIGURATION%

@popd
