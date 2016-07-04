@echo off

SET DPATH=%~dp0
::ECHO %DPATH%

SET DEFHEAD=#define BUILD_COMMIT

SET COMMITH=%DPATH%\..\..\Common\Environs.Commit.h
::ECHO %COMMITH%
SET ECOMMIT=%DPATH%\..\..\Common\ORIG_HEAD
SET COMMIT=%DPATH%\..\..\.git\ORIG_HEAD

IF NOT EXIST "%COMMIT%" GOTO nogit

IF NOT EXIST "%ECOMMIT%" GOTO update

ECHO n|C:\Windows\System32\COMP "%COMMIT%" "%ECOMMIT%" >null 2>nul

IF "%ERRORLEVEL%" == "0" ( ECHO "Commits are identical." && GOTO done )

:update
ECHO Updating commit ...
copy "%COMMIT%" "%ECOMMIT%"

ECHO | set /p=%DEFHEAD%  >%COMMITH%
TYPE "%COMMIT%" >>%COMMITH%
GOTO done


:nogit
ECHO Updating with unknown commit ...
ECHO | set /p=%DEFHEAD%  >%COMMITH%
ECHO UNKNOWN >>%COMMITH%

:done