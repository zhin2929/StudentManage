cls
title VS��Ŀ���� BY CR28 QHR
color 0A
cls
@echo VS��Ŀ���� BY CR28 QHR
@echo ��������......
@echo ��ȴ�������ɣ�
@echo off
rd ".vs" /s /q 
rd "ipch" /s /q 
rd "Debug" /s /q 
rd "Release" /s /q 
for /f "delims=" %%i  in ('dir /b/a:d/s .\')  do (
rd "%%i\.vs" /s /q 
rd "%%i\ipch" /s /q 
rd "%%i\Debug" /s /q 
rd "%%i\Release" /s /q 
)
::del *.exe  *.dll /s 
attrib -s -h -r *.opensdf /s 
del *.VC.db *.opensdf *.opendb /s 
del *.sdf *.log *.user *.ipch *.aps /s 
del *.suo /s /a h 
del *.ilk *.exp *.pdb *.ipdb *.iobj *.tlog *.res *.lastbuildstate /s 
del *.plg *.opt /s 
del *.obj *.pch /s 

del ReadMe.txt /s 
del UpgradeLog.htm /s 

@echo �������!
echo ^^o^^ 
@echo ��������ر�! 
pause