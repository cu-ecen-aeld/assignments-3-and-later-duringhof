all:
	CROSS_COMPILE=aarch64-none-linux-gnu
	writer

writer: writer.o
	$(CROSS_COMPILE)gcc writer.o -o writer

writer.o: writer.c
	$(CROSS_COMPILE)gcc -c writer.c

clean:
	rm -f *.o writer
	
