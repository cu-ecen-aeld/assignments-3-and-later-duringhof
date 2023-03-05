CC=gcc
CROSS_COMPILE=aarch64-none-linux-gnu-

all: writer

clean:
		-rm -f *.o writer

%.o:%.c
	$(CROSS_COMPILE)$(CC) -c -o $@ $< -Wall -Werror
	
writer:writer.o
	$(CROSS_COMPILE)$(CC) -o $@ $^
