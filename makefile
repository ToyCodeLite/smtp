CC_FLAG= -g
GCC=gcc
#GCC=x86_64-w64-mingw32-gcc 

smtp.exe:main.o smtp.o base64.o
	${GCC} ${CC_FLAG} -o smtp main.o smtp.o base64.o

main.o:main.c
	${GCC} ${CC_FLAG} -c main.c -o main.o

smtp.o:smtp.h smtp.c
	${GCC} ${CC_FLAG} -c smtp.c -o smtp.o

base64.o:lib/base64.h lib/base64.c
	${GCC} ${CC_FLAG} -c lib/base64.c -o base64.o
.PHONY:clean
clean:
	rm -f *.o smtp

