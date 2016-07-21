@echo off
cd Release
PCF-Comp-Separated.exe -i:source-sep.txt -o:dis-sep.txt
REM PCF-Comp-Integrated.exe -i:source-int.txt -o:dis-int.txt
cd ..
Release\PCForth.exe -i:dis-sep.txt
REM Release\PCForth.exe -i:dis-int.txt
