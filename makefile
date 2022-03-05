all: ls.o 
	gcc -Wall -Wextra -g -o ls ls.o 
main.o: ls.c
	gcc -Wall -Wextra -g -c ls.c
clean:
	rm -f ls
	rm -f ls.o
	