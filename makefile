FLAGS = -g -c

run: ruterdrift.o
	gcc -o $@ $^

ruterdrift.o: ruterdrift.c
	gcc $(FLAGS) $^

clean: 
	rm run *.o new-topology.dat


