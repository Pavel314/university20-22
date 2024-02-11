@echo off
setlocal EnableDelayedExpansion

set /a iters=%1%
set /a lb=%2%
set /a ub=%3%

for /L %%a in (1 1 %iters%) do (
        call:rand %lb% %ub%
        echo !rand_num!
)

:rand
set /a rand_num=%random% * (%2 - %1 + 1) / 32768 + %1
goto:EOF
