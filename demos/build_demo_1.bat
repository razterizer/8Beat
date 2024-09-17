call ..\..\Core\build.bat
 
REM Compile the demo using VC++
cl /std:c++20 /EHsc /Fe:bin/demo_1.exe /Fo:bin/demo_1.obj .\demo_1.cpp /I..\.. /I..\..\..\3rdparty\include /LIBPATH:..\..\..\3rdparty\lib

if %errorlevel% neq 0 (
    echo Build failed with error code %errorlevel%.
    exit /b %errorlevel%
)
