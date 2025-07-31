LIBS=`pkg-config --libs OpenCL`
FLAGS=-Wall -Wextra #-fopt-info

SRCS := $(wildcard *.c)
EXES := $(SOURCES:.c=)

all: main

main: $(SRCS)
	gcc -O3 $(LIBS) $(FLAGS) -o $@ $^

debug: $(SRCS)
	gcc -g $(LIBS) $(FLAGS) -o $@ $^

clean:
	rm -f main debug
