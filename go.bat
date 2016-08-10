@echo off
Release\PCF-Comp-Separated.exe -i:sep-src.txt -o:sep-dis.txt -b:sep-img.bin
Release\PCForth.exe -b:sep-img.bin

REM Release\PCF-Comp-Integrated.exe -i:source-int.txt -o:dis-int.txt
REM Release\PCForth.exe -i:dis-int.txt
