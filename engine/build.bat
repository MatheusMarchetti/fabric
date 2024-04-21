REM Build script for core
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all the .cpp files.
SET cFilenames=
FOR /R %%f in (*.cpp) do (
    SET cFilenames=!cFilenames! %%f
)

REM echo "Files:" %cFilenames%

SET assembly=core
SET compilerFlags=-g -shared -Wvarargs -Wall -Werror -std=c++20
SET includeFlags=-Isource
SET linkerFlags= -luser32 -ld3d12 -ldxgi -ldxguid
SET defines=-D_DEBUG -DFBEXPORT -D_CRT_SECURE_NO_WARNINGS

ECHO "Building %assembly%%..."
clang %cFilenames% %compilerFlags% -o ../bin/%assembly%.dll %defines% %includeFlags% %linkerFlags%