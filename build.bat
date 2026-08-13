@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if not exist build mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
cd ..

