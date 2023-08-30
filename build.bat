@echo off

IF NOT EXIST build mkdir build
pushd build

cl /O2 /FC ../cleancode.cpp -Fecleancode.exe

popd