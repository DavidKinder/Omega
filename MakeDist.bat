@echo off
"\Program Files\Zip\zip" -j \Temp\omega.zip Release\Omega.exe Omega.txt
"\Program Files\Zip\zip" \Temp\omega.zip Docs\* OmegaLib\* 32*
"\Program Files\Zip\zip" \Temp\omegasrc.zip Omega\* *.h *.bat Win* Omega*

