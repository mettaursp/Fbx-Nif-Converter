@echo off

:loop
@echo %1
glslangValidator.exe -V100 ./shaderSource/%1 -o ./shaders/%1
shift
if not "%~1"=="" goto loop


@pause