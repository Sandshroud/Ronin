@ECHO OFF
CLS
ECHO Sandshroud Hearthstone Server Started %time:~0,5% %date:~1%
:SERVERLOOP
hearthstone-World.exe
ECHO Sandshroud Hearthstone Restarted %time:~0,5% %date:~1%
ECHO.
GOTO SERVERLOOP
:END