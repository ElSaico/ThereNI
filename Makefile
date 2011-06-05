BIN_NAME = thereni

OBJS = main.o
SRCS = $(OBJS,.o=.cpp)

all: main

main: $(OBJS)
	g++ -O2 -lSDL -lopenal -o $(BIN_NAME) $(OBJS)

.cpp.o:
	g++ -c -O2 -Wall $<

clean:
	rm -f $(BIN_NAME) $(OBJS)
