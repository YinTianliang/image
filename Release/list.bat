@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=list test.txt
set image=text ��� 0 600
pause