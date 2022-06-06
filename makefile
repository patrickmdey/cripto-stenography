include makefile.inc

BINARY	= stegobmp
SOURCES = $(wildcard *.c ./utils/*.c)
OBJECTS = $(SOURCES:.c=.o)
rm      = rm -rf

all: compile link

compile:$(SOURCES)
	$(CC) $(CFLAGS) -I./utils/include $(SOURCES)

link:$(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $(BINARY)

%.o: %.c
	$(CC) $(CCFLAGS) -I./utils/include -c $< -o $@

%.out : %.o
	$(CC) $(LFLAGS) $< -o $@

clean:
	@${rm} ${OBJECTS} 
	@${rm} *.o 
	@${rm} ${BINARY}.out

.PHONY: all clean link compile

