
@echo off

echo %~dp0

set sleep=ping 127.0.0.1 -n
set second_5=5
set second_2=2

echo 复制头文件...

if not exist %~dp0\third (
	md %~dp0\third
)

if not exist %~dp0\third\communication (
	md %~dp0\third\communication
)

if not exist %~dp0\third\common (
	md %~dp0\third\common
)

if not exist %~dp0\third\utils (
	md %~dp0\third\utils
)

xcopy /H /S /E /Y %~dp0\..\communication\*.h %~dp0\third\communication
xcopy /H /S /E /Y %~dp0\..\common\*.h %~dp0\third\common
xcopy /H /S /E /Y %~dp0\..\utils\*.h %~dp0\third\utils


echo 更新NetServer头文件...
if not exist %~dp0\..\NetServer\third (
	md %~dp0\..\NetServer\third
)

xcopy /H /S /E /Y %~dp0\third\*.h %~dp0\..\NetServer\third

echo 更新testcommdll头文件...
if not exist %~dp0\..\testcommdll\third (
	md %~dp0\..\testcommdll\third
)

xcopy /H /S /E /Y %~dp0\third\*.h %~dp0\..\testcommdll\third

echo 清理中间文件...
del /f /s /q .\*.exp
del /f /s /q .\*.ilk
del /f /s /q .\*.pdb

%sleep% %second_5% >nul


