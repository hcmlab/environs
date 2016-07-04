@echo off

SET DBG=1
SET DPATH=%~dp0
::echo %DPATH%

:: Toolset
IF "%1"=="" GOTO printUsage
:: Architecture 32 64
IF "%2"=="" GOTO printUsage
:: Sourcefile
IF "%3"=="" GOTO printUsage
:: Filename
IF "%4"=="" GOTO printUsage
:: SolutionDir
IF "%5"=="" GOTO printUsage
:: binPostfix

SET PDIR=
IF "%2"=="64" SET PDIR=64
::echo %PDIR%

SET BINFIX=_%7
IF "%7"=="" SET BINFIX=

SET TPATH=%5../bin%PDIR%%BINFIX%
::echo %TPATH%

IF NOT EXIST "%TPATH%/libs/%1" ( mkdir "%TPATH%/libs/%1" && echo Created "%TPATH%/libs/%1" )
	::IF ERRORLEVEL != 0 ( echo "Failed to create "%TPATH%/libs/%1" && exit 1 )
	::)

IF "%DBG%"=="1" echo copy "%3%4.dll" "%TPATH%/libs/%1/%4.dll"
copy "%3%4.dll" "%TPATH%/libs/%1/%4.dll"

IF "%DBG%"=="1" echo copy "%3%4.lib" "%TPATH%/libs/%1/%4.lib"
IF NOT "%6"=="1" copy "%3%4.lib" "%TPATH%/libs/%1/%4.lib"

IF "%6"=="1" echo copy "%3%4.dll" "%TPATH%/%4.dll"
IF "%6"=="1" copy "%3%4.dll" "%TPATH%/%4.dll"

::echo del "%TPATH%/.env.*"
cd %TPATH%
del .env.*
echo. 2>.env.%1
cd %DPATH%
GOTO done

:printUsage
echo Usage: %0 Toolset PathToSource PathToTarget SolutionDirectory

:done