@echo off

set BOOST_ROOT=E:\dev\3rdparty\boost_1_61_0

cd %~dp0/build
cmake .. -G"Visual Studio 14 2015 Win64" -DBOOST_LIBRARYDIR=E:\dev\3rdparty\boost_1_61_0\stage\lib64
cd %~dp0