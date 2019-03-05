all: integration1.o subfile0.o subfile1.o
	clang -o integration1 subfile0.o subfile1.o integration1.o

%.o: %.c
	clang -I ./include -c $< -o $@

clean:
	rm -rf *.o integration1
