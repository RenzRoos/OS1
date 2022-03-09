all: os1.o 
	gcc -Wall -Wextra -g -o os1 os1.o 
main.o: ls.c
	gcc -Wall -Wextra -g -c os1.c
clean:
	rm -f os1
	rm -f os1.o
