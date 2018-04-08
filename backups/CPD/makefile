CFLAGS= -fopenmp -Wall -Werror 

lab06: lab06.o tree.o
	gcc $(CFLAGS) -o lab06 tree.o lab06.o

lab06.o: lab06.c tree.h queue.h
	gcc $(CFLAGS) -c lab06.c

tree.o: tree.c tree.h queue.h
	gcc $(CFLAGS) -c tree.c

clean:
	rm -f *.o *.~ lab06 *.gch
