full pathname to executable:
ssh linux.student.cs.uwaterloo.ca
/u/cs452/tftp/ARM/<userID>/main.elf

How to run
1. Build:
> cd <path to CS452-Microkernel>/CS452-Microkernel/util/src
> make
> cd <path to CS452-Microkernel>/CS452-Microkernel/experiment/src
> make
.elf and .map files are in the "src" directory
generated .s and .o files are in the "build" directory
.a static library is under "lib" directory
2. Load and run the program in from Redboot terminal:
>load -b 0x218000 -h 10.15.167.5 "/ARM/<user id>/main.elf"
>go
