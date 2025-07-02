LIBS=`pkg-config --libs OpenCL`
FLAGS=-Wall -Wextra #-fopt-info
FSAN=-fsanitize=address,undefined,leak,bounds-strict

SRCS := $(wildcard *.c)
EXES := $(SOURCES:.c=)

all: main

main: $(SRCS)
	gcc -O3 $(LIBS) $(FLAGS) -o $@ $^

debug: $(SRCS)
	gcc -g $(LIBS) $(FLAGS) -o $@ $^

san: $(SRCS)
	gcc -O3 $(LIBS) $(FLAGS) $(FSAN) -o $@ $^

clean: main debug san
	rm -f $^
