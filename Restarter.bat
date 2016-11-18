@ECHO OFF
CLS
ECHO Ronin Server Started %time:~0,5% %date:~1%
:SERVERLOOP
ronin-world.exe
ECHO Ronin Restarted %time:~0,5% %date:~1%
ECHO.
GOTO SERVERLOOP
:END