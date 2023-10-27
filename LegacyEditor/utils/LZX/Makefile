all: $(patsubst %.c,%.o,$(wildcard *.c))

-include $(wildcard *.d)

%.o: %.c
	$(CC) -c $(CFLAGS) -MMD -o $@ $<
clean:
	-rm *.o *.d
