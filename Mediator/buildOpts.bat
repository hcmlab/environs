@echo off

SET DPATH=%~dp0
::echo %DPATH%

SET VLDINC="%DPATH%\..\Common\Environs.Build.Opts.VLD.h"
IF EXIST "%VLDINC%" GOTO done

SET /a tmp=(%RANDOM%*500/32768)+1 

SET VLDTMP="%DPATH%\..\Common\Environs.Build.Opts.VLD.h.%tmp%"

IF EXIST "c:\Program Files (x86)\Visual Leak Detector\include\vld.h" GOTO dontChangeVLD

:disableVLD
echo "Disabling VLD ..."

echo #ifdef _USE_VLD > "%VLDTMP%"
echo #	undef _USE_VLD >> "%VLDTMP%"
echo #endif >> "%VLDTMP%"
move "%VLDTMP%" "%VLDINC%"
	
GOTO done

:dontChangeVLD
echo // No change in vld usage > "%VLDTMP%"
move "%VLDTMP%" "%VLDINC%"

copy "c:\Program Files (x86)\Visual Leak Detector\bin\Win32\*" "%DPATH%\..\bin\."
copy "c:\Program Files (x86)\Visual Leak Detector\bin\Win64\*" "%DPATH%\..\bin64\."


:done