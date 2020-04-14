@echo off
rem  ===== Date Created: 14 April, 2020 ===== 

set ROOT_DIRECTORY=%CD%

IF NOT EXIST build\ md build
cd build

cmake .. -G "Visual Studio 16 2019"
IF %ERRORLEVEL% NEQ 0 GOTO end
cmake --build .
IF %ERRORLEVEL% NEQ 0 GOTO end

echo.
cd %ROOT_DIRECTORY%

if NOT "%~1"=="-norun" call run.bat

:end
cd %ROOT_DIRECTORY%
exit /B %ERRORLEVEL%
