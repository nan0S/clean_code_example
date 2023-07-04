@echo off

IF NOT EXIST build mkdir build
pushd build

cl ../cleancode.cpp -Fecleancode.exe /O2

popd