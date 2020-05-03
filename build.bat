@echo off
rem  ===== Date Created: 14 April, 2020 ===== 

set ROOT_DIRECTORY=%CD%

set NORUN=false
set CONFIG=Debug

FOR %%A IN (%*) DO (
	IF [%%A]==[-norun] (
		set NORUN=true
	) ELSE IF [%%A]==[-relinfo] (
		set CONFIG=RelWithDebInfo
	) ELSE IF [%%A]==[-release] (
		set CONFIG=Release
	)
)

IF NOT EXIST build\ md build
cd build

cmake .. -G "Visual Studio 16 2019"
IF %ERRORLEVEL% NEQ 0 GOTO end
cmake --build . --config %CONFIG%
IF %ERRORLEVEL% NEQ 0 GOTO end

echo.
cd %ROOT_DIRECTORY%

IF NOT %NORUN%==true (
	IF %CONFIG%==Debug (
		call run.bat
	) ELSE IF %CONFIG%==RelWithDebInfo (
		call run.bat -relinfo
	) ELSE (
		call run.bat -release
	)
)

:end
cd %ROOT_DIRECTORY%
exit /B %ERRORLEVEL%
