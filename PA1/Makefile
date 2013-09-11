CC=gcc
LD=gcc

CC_FLAGS= -g 
LDFLAGS=

OBJECTS=mydisk.o caching.o test.o

all:$(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o mydisk 

test:
	./mydisk

$(OBJECTS):%.o:%.c
	$(CC) $(CC_FLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm -f mydisk diskfile

