@echo off

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

SET PDIR=
IF "%2"=="64" SET PDIR=64
::echo %PDIR%

SET TPATH=%5../bin%PDIR%
::echo %TPATH%

IF NOT EXIST "%TPATH%/libs/%1" ( mkdir "%TPATH%/libs/%1" && echo Created "%TPATH%/libs/%1" )
	::IF ERRORLEVEL != 0 ( echo "Failed to create "%TPATH%/libs/%1" && exit 1 )
	::)

copy "%3%4.dll" "%TPATH%/libs/%1/%4.dll"
copy "%3%4.lib" "%TPATH%/libs/%1/%4.lib"
::copy "%3%4.dll" "%TPATH%/%4.dll"

::echo del "%TPATH%/.env.*"
cd %TPATH%
del .env.*
echo. 2>.env.%1
cd %DPATH%
GOTO done

:printUsage
echo Usage: %0 Toolset PathToSource PathToTarget SolutionDirectory

:done