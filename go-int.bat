@echo off
Release\PCF-Comp-Integrated.exe -i:source-int.txt -o:dis-int.txt
Release\PCForth.exe -i:dis-int.txt
