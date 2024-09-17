call ..\..\Core\build.bat
 
REM Compile the demo using VC++
cl /std:c++20 /EHsc /Fe:bin/demo_1.obj /Fo:bin/demo_1.obj .\demo_1.cpp /I../.. /I../../../3rdparty\include

REM Link the compiled object to create the executable
link /OUT:bin/demo_1.exe bin/demo_1.obj /LIBPATH:..\..\..\3rdparty\lib OpenAL32.lib

if %errorlevel% neq 0 (
    echo Build failed with error code %errorlevel%.
    exit /b %errorlevel%
)
