set djgpp=c:\djgpp\djgpp.env
mkdir o
c:\djgpp\bin\gcc -c callbacks.c -o o\callbacks.o -Iinclude
c:\djgpp\bin\gcc -c cardinfo.c -o o\cardinfo.o -Iinclude
c:\djgpp\bin\gcc -c getopt.c -o o\getopt.o -Iinclude
c:\djgpp\bin\gcc -c iopl.c -o o\iopl.o -Iinclude
c:\djgpp\bin\gcc -c mysnprtf.c -o o\mysnprtf.o -Iinclude
c:\djgpp\bin\gcc -c pnp-access.c -o o\pnp-access.o -Iinclude
c:\djgpp\bin\gcc -c pnp-select.c -o o\pnp-select.o -Iinclude
c:\djgpp\bin\gcc -c realtime.c -o o\realtime.o -Iinclude
c:\djgpp\bin\gcc -c release.c -o o\release.o -Iinclude
c:\djgpp\bin\gcc -c res-access.c -o o\res-access.o -Iinclude
c:\djgpp\bin\gcc -c resource.c -o o\resource.o -Iinclude
c:\djgpp\bin\gcc -c pnpdump.c -o pnpdump.o -Iinclude
c:\djgpp\bin\gcc -c pnpdump_main.c -o pnpdump_main.o -Iinclude
c:\djgpp\bin\gcc o\*.o pnpdump.o pnpdump_main.o -o pnpdump.exe
