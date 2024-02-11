@echo off
echo ==========================
echo DiceGenerator is a general-purpose program for generating pseudorandom numbers with an uniform distribution.
echo The first function is entertaining, you can roll a coin or a dice, after which the results will be displayed along with the status bar. 
echo The status bar includes two variables: the number of rolls and the sum of all dropped values. 
echo At the same time, these variables accumulate when the same object is roll again (i.e. an eagle or a dice).
echo[
echo The second function allows you to generate pseudo-random numbers in an arbitrary number and user range. 
echo If the values are omitted, the values from the previous call will be used.
echo 10 numbers in the range from 1 to 10 are used as the initial state
echo Finally, it is possible to write numbers to a file. 
echo To do this, the path is set, if the path is omitted, the path from the previous call will be use.
echo Otherwise, the output file will be created in the current directory with the name 'rngnum.txt'
echo ===========================
echo Auther: PavelPI. 2021.12.30