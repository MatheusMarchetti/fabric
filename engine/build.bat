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
REM -Wall -Werror
SET includeFlags=-Isource
REM SET linkerFlags= -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32 -llibcpmt -lkernel32
SET linkerFlags= -luser32
SET defines=-D_DEBUG -DFBEXPORT -D_CRT_SECURE_NO_WARNINGS

ECHO "Building %assembly%%..."
clang %cFilenames% %compilerFlags% -o ../bin/%assembly%.dll %defines% %includeFlags% %linkerFlags%