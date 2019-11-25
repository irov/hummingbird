@echo off

@echo Starting download dependencies...

@mkdir %~dp0..\..\solutions\downloads
@pushd %~dp0..\..\solutions\downloads
@call CMake.exe "%CD%\..\..\cmake\downloads"
@call CMake.exe --build .
@popd

@echo Done

@pause