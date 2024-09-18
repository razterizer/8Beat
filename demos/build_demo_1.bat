@echo off
call ..\..\Core\build.bat

REM Compile the demo using VC++
cl /std:c++20 /EHsc /D_CRT_FUNCTIONS_REQUIRED /D_CRT_SECURE_NO_WARNINGS /Fo:bin/demo_1.obj /Fe:bin/demo_1.obj .\demo_1.cpp ^
/I..\.. ^
/I..\..\Termin8or ^
/I..\..\AudioLibSwitcher_OpenAL ^
/I..\..\..\3rdparty\include

if %errorlevel% neq 0 (
    echo Compilation failed with error code %errorlevel%.
    exit /b %errorlevel%
)

REM Link the compiled object to create the executable
link /OUT:bin/demo_1.exe bin/demo_1.obj /LIBPATH:..\..\..\3rdparty\lib OpenAL32.lib

if %errorlevel% neq 0 (
    echo Linking failed with error code %errorlevel%.
    exit /b %errorlevel%
)

echo Build succeeded.
