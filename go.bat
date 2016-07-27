@echo off
Release\PCF-Comp-Separated.exe -i:source-sep.txt -o:dis-sep.txt
REM Release\PCF-Comp-Integrated.exe -i:source-int.txt -o:dis-int.txt
REM cd ..
Release\PCForth.exe -i:dis-sep.txt
REM Release\PCForth.exe -i:dis-int.txt
