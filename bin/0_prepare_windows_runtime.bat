@echo off

SET DBG=1
SET DPATH=%~dp0
echo %DPATH%

cd %DPATH%
del .env.*


:checkv140
IF NOT EXIST "C:/Windows/System32/msvcp140.dll" GOTO checkv120
echo "Preparing v140 Environment ..."

copy "%DPATH%libs\v140\Environs.Net.dll" "%DPATH%Environs.Net.dll"
copy "%DPATH%libs\v140\"Environs.PixelSense.dll "%DPATH%Environs.PixelSense.dll
echo "" >>./.env.v140
GOTO done

:checkv120
IF NOT EXIST "C:/Windows/System32/msvcr120.dll" GOTO checkv100
echo "Preparing v120 Environment ..."

copy "%DPATH%libs\v120\Environs.Net.dll" "%DPATH%Environs.Net.dll"
copy "%DPATH%libs\v120\"Environs.PixelSense.dll "%DPATH%Environs.PixelSense.dll
echo "" >>./.env.v120
GOTO done

:checkv100
IF NOT EXIST "C:/Windows/System32/msvcr100.dll" GOTO checkErr
echo "Preparing v100 Environment ..."

copy "%DPATH%libs\v100\Environs.Net.dll" "%DPATH%Environs.Net.dll"
copy "%DPATH%libs\v100\"Environs.PixelSense.dll "%DPATH%Environs.PixelSense.dll
echo "" >>./.env.v100
GOTO done

:checkErr
echo "Error: No supported windows runtime detected!"
GOTO done


:done

pause
