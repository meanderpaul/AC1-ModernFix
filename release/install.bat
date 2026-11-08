@echo off
rem AC1 Modern Fix installer - by Meanderpaul_gaming
rem Extract this zip into your Assassin's Creed folder, then double-click this file.
setlocal
cd /d "%~dp0"
echo.
echo  AC1 Modern Fix installer
echo  ------------------------
if not exist "AssassinsCreed_Dx9.exe" (
  echo  [X] AssassinsCreed_Dx9.exe not found.
  echo      Extract this zip into your Assassin's Creed game folder and run install.bat from there.
  goto end
)
if not exist "scripts" mkdir "scripts"
copy /y "AC1AffinityFix.asi" "scripts\" >nul && copy /y "AC1AffinityFix.ini" "scripts\" >nul
if errorlevel 1 ( echo  [X] Could not copy AC1AffinityFix files. & goto end )
del "AC1AffinityFix.asi" "AC1AffinityFix.ini" >nul 2>&1
echo  [OK] AC1AffinityFix installed to scripts\
if exist "dinput8.dll" ( echo  [OK] Ultimate ASI Loader found ) else ( echo  [!] dinput8.dll missing - get Ultimate ASI Loader v4.68, see README & set MISSING=1 )
if exist "scripts\EaglePatchAC1.asi" ( echo  [OK] EaglePatch found ) else ( echo  [!] EaglePatchAC1.asi missing - get it from github.com/Sergeanur/EaglePatch/releases & set MISSING=1 )
echo.
if defined MISSING ( echo  Install the missing pieces above, then launch the game. ) else ( echo  All set. Plug in your controller and press Play. Choose DirectX 9 if asked. )
echo  Tip: if xinput1_3.dll is reported missing, run DirectX\DXSETUP.exe once.
:end
echo.
pause
