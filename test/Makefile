CFLAGS = -std=c11 -Wall -Werror -Wextra -pedantic -D_XOPEN_SOURCE=600

all: bin/testopt
	
bin/testopt: testopt.c
	mkdir -p bin/
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	$(RM) *.o

distclean:
	$(RM) -rf *.o bin/
