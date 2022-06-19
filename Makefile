

all: fs mylib

fs: fs.c fs.h
	gcc -c fs.c

mylib: mylib.c mylib.h
	gcc -c mylib.c

clean:
	rm -f *.o