CC_FLAG = -g
GCC = gcc
SRC = os/linux.c
ifeq ($(OS),Windows_NT)
    GCC = x86_64-w64-mingw32-gcc
    CC_FLAG += -D_WIN32_WINNT=0x0600
    LDFLAGS += -lws2_32 -liphlpapi
	SRC = os/windows.c
endif

smtp.exe: main.o smtp.o base64.o 
	${GCC} ${CC_FLAG} -o smtp main.o smtp.o base64.o ${LDFLAGS}

main.o: main.c  ${SRC}
	${GCC} ${CC_FLAG} -c main.c -o main.o

smtp.o: smtp.h smtp.c
	${GCC} ${CC_FLAG} -c smtp.c -o smtp.o

base64.o: lib/base64.h lib/base64.c
	${GCC} ${CC_FLAG} -c lib/base64.c -o base64.o

clean:
	rm -f *.o smtp smtp.exe

