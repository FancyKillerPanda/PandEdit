@echo off
rem  ===== Date Created: 14 April, 2020 ===== 

set ROOT_DIRECTORY=%CD%

set CONFIG_DIR=Debug
IF "%~1"=="-relinfo" set CONFIG_DIR=RelWithDebInfo
IF "%~1"=="-release" set CONFIG_DIR=Release

IF EXIST bin\%CONFIG_DIR%\ (
	cd bin\%CONFIG_DIR%
	PandEdit.exe
)

:end
cd %ROOT_DIRECTORY%
