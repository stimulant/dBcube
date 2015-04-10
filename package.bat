REM DIE IF NOT ON BUILD SERVER
IF %COMPUTERNAME% NEQ JENKINS GOTO NOTBUILD

REM SET UP ENVIRONMENT
SET DIRSWITCHES=/v /np /njs /njh /bytes /fft /ndl /mir
SET FILESWITCHES=/v /np /njs /njh /bytes /fft /ndl 

SET TARGET=..\builds\%BUILD_ID%\archive\
ECHO %BUILD_NUMBER% > %TARGET%\BUILD_%BUILD_NUMBER%

ROBOCOPY Client\vc2012\bin %TARGET%\Client %DIRSWITCHES%
ROBOCOPY Server %TARGET%\Server %DIRSWITCHES%
cd %TARGET%\Server
npm install

EXIT /B 0

:NOTBUILD
ECHO This should only be run on the build server.
PAUSE
