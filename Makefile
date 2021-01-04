CC = gcc
CFLAGS = -g

shell: shell.o
	$(CC) -I/usr/include/readline -lreadline -o shell shell.o
.PHONY : clean
clean:
	-rm *.o shell
