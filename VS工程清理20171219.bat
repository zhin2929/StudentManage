cls
title VS项目清理 BY CR28 QHR
color 0A
cls
@echo VS项目清理 BY CR28 QHR
@echo 正在清理......
@echo 请等待清理完成！
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

@echo 清理完成!
echo ^^o^^ 
@echo 按任意键关闭! 
pause