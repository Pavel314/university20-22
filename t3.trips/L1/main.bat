@echo off
setlocal enableDelayedExpansion
setlocal enableExtensions
REM Mode con cols=80 lines=25
REM chcp 65001 >NUL

set prev_choice=0

call :reset_state

:draw_menu
cls
echo -----------------------------
echo Welcome to DICEGenerator 1.0!
echo -----------------------------
echo 1 - Random coin
echo 2 - Random dice
echo 3 - User settings (terminal)
echo 4 - User settings (file)
echo 5 - Clear screen
echo 6 - Show state
echo 7 - Reset state
echo 8 - Help
echo 9 - Quit

:start
choice /n /c:123456789 /m "What would you like to do?"
set cur_choice=%errorlevel%
if "%cur_choice%"=="1" call :dice_gen coin & goto start
if "%cur_choice%"=="2" call :dice_gen dice & goto start
if "%cur_choice%"=="3" call :many_rnd term & goto start
if "%cur_choice%"=="4" call :many_rnd file & goto start
if "%cur_choice%"=="5" goto draw_menu
if "%cur_choice%"=="6" (  
  echo Numbers count=%iters%
  echo lower bound=%lb%
  echo upper bound=%ub%
  call :fact_path !write_path! 
  echo Write path=!write_path!
  echo rolls: count=%roll_count% sum=%dice_sum%
  goto start
)
if "%cur_choice%"=="7" (
  call :reset_state
  echo OK. The state has been reset
  goto start
)

if "%cur_choice%"=="8" call help.bat
if "%cur_choice%"=="9" goto :eof

goto start








:reset_state
set "iters=10"
set "lb=1"
set "ub=10"
set "write_path=%cd%\rndfile.txt"

set /a dice_sum=0
set /a roll_count=0
goto :eof



:dice_gen
call dice_gen.bat %~1
if not "%prev_choice%"=="%cur_choice%" (
 set /a dice_sum=0
 set /a roll_count=0
)
set /a "dice_sum=%dice_sum%+%errorlevel%"
set /a "roll_count=%roll_count%+1"

echo rolls: count=%roll_count% sum=%dice_sum%
set prev_choice=%cur_choice%
goto :eof





:many_rnd
set /p iters="Enter the count of numbers:"
call :check_nat %iters%
if %is_nat% NEQ 1 (goto :start)

set /p lb="Enter the lower bound:"
call :check_int %lb%
if %is_int% NEQ 1 (goto :start)

set /p ub="Enter the upper bound:"
call :check_int %ub%
if %is_int% NEQ 1 (goto :start)

if "%~1" == "file" (
  set /p "write_path=Enter the write path:"
  call :fact_path !write_path!
  if not exist !path_value! (
     echo Path !write_path! not exist. Canceled
     goto :eof
  )
  call nums_gen.bat %iters% %lb% %ub% > !write_path!
  echo OK. %iters% numbers were written to !write_path!
) else (
  call nums_gen.bat %iters% %lb% %ub%
)

goto :eof





:check_int
set /a is_int=0
SETLOCAL
set /a test=%~1
if %test% NEQ %~1 (
  echo Could not recognize integer number. Canceled
  goto :eof
)
ENDLOCAL
set /a is_int=1
goto :eof





:check_nat
set /a is_nat=0
SETLOCAL
set /a test=%~1
if %test% NEQ %~1 (
  echo Could not recognize natural number. Canceled  
  goto :eof
)
if %~1 LSS 0 (
  echo Could not recognize natural number. Canceled  
  goto :eof
)
ENDLOCAL
set /a is_nat=1
goto :eof





:fact_path 
set "path_value=%~dp1"
set "%~1=%~dp1%~nx1"
goto :eof