@echo off
call ..\..\Core\build.bat

cd demos.vs

SET configuration="Release"
IF "%~1" == "Debug" SET configuration="Debug"
SET target="x64"
IF "%~2" == "x86" SET target="x86"
msbuild demos.sln /p:Configuration=%configuration% /p:Platform=%target%
REM msbuild demo_1.vcxproj /p:Configuration=%configuration% /p:Platform=%target%
REM msbuild demo_2.vcxproj /p:Configuration=%configuration% /p:Platform=%target%

cd ..

if %errorlevel% neq 0 (
    echo Compilation failed with error code %errorlevel%.
    exit /b %errorlevel%
)

echo Build succeeded.
