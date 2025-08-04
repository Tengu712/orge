@echo off

rem Windows用のvcpkg関連の初期化処理ユーティリティスクリプト
rem その値が"static"か"shared"であるコマンドライン引数を必ず1個取る

setlocal enabledelayedexpansion

if not defined ERRORLEVEL set ERRORLEVEL=0

if "%~1"=="" (
		echo Usage: %~nx0 ^<static^|shared^>
		exit /b 1
)

set "CLARG=%~1"
if not "%CLARG%"=="static" if not "%CLARG%"=="shared" (
		echo Usage: %~nx0 ^<static^|shared^>
		exit /b 1
)

set "PROJROOT=%~dp0.."
set "VCPKG_EXE=%PROJROOT%\vcpkg\vcpkg.exe"
if not exist "%VCPKG_EXE%" (
		echo [ info ] running %PROJROOT%\vcpkg\bootstrap-vcpkg.bat
		call %PROJROOT%\vcpkg\bootstrap-vcpkg.bat
		if !ERRORLEVEL! neq 0 (
				echo [ error ] failed to call %PROJROOT%\vcpkg\bootstrap-vcpkg.bat
				exit /b !ERRORLEVEL!
		)
)

set "TRIPLET=custom-x64-windows-%CLARG%"
set "INSTALL_DIR=%PROJROOT%\vcpkg_installed_%CLARG%"
set "VCPKG_INSTALL_CMD=%VCPKG_EXE% install --overlay-triplets=%PROJROOT%\triplets --triplet=%TRIPLET% --x-install-root=%INSTALL_DIR%"
echo [ info ] running %VCPKG_INSTALL_CMD%
%VCPKG_INSTALL_CMD%
if !ERRORLEVEL! neq 0 (
		echo [ error ] failed to run %VCPKG_INSTALL_CMD%
		exit /b !ERRORLEVEL!
)
if not exist "%INSTALL_DIR%" (
		echo [ error ] %INSTALL_DIR% not generated.
		exit /b 1
)

exit /b 0
