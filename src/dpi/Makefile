CC=gcc
CFLAGS=-I. -O3 -g 
OBJ = basic_rules.o http_rules.o basic_rules_test.o 

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
clean:
	rm -f *.o