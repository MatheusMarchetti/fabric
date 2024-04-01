REM Build script for launcher
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all the .cpp files.
SET cFilenames=
FOR /R %%f in (*.cpp) do (
    SET cFilenames=!cFilenames! %%f
)

REM echo "Files:" %cFilenames%

SET assembly=launcher
SET compilerFlags=-g -std=c++20
REM -Wall -Werror
SET includeFlags=-Isource -I../engine/source/
SET linkerFlags=-L../bin/ -lcore.lib
SET defines=-D_DEBUG

ECHO "Building %assembly%%..."
clang %cFilenames% %compilerFlags% -o ../bin/%assembly%.exe %defines% %includeFlags% %linkerFlags%