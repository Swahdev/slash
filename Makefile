CC = gcc -Wall *.c -o slash -lreadline
all:
	$(CC) -o slash
clean :
	rm -f *.o slash
