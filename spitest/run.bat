@echo off

set PATH=%PATH%;win-rxtx
set JEDITS_CLASSPATH=win-rxtx;win-rxtx\RXTXcomm.jar;.

java -cp %JEDITS_CLASSPATH% test %1 %2 %3 %4 %5 %6 %7 %8 %9


