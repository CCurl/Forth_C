@echo off
Release\PCF-Comp-Separated.exe -i:source-sep.txt -o:dis-sep.txt
Release\PCForth.exe -i:dis-sep.txt
