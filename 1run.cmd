@echo off


@REM mingw32-make




@REM del program.exe
@REM g++ -o program -DSIZE_DATA="1000" src/main.cpp src/generate_packet/generate_packet.c
@REM program res/data.txt res/gold_sequence.txt res/gold_seq_end.txt res/data_bin.txt 

mingw32-make -j16
@echo:
@echo:
rem Program res/data.txt res/gold_sequence.txt res/gold_seq_end.txt res/data_bin.txt 
Program.exe "ip:192.168.1.3" data/data_test.txt

exit

@REM del Program.exe
@REM del /q build\*
@REM mingw32-make SHELL=CMD.exe -j10 -f makefile_build.mk
@REM @echo:
@REM @echo:

@REM Program.exe res/data.txt res/gold_sequence.txt res/gold_seq_end.txt res/data_bin.txt 

@REM rem mingw32-make








