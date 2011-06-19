BIN_NAME = thereni

OBJS = thereni.o libfreenect_cv.o
SRCS = $(OBJS,.o=.cpp)

all: main

main: $(OBJS)
	gcc -O2 `pkg-config --libs opencv` -L/usr/local/lib -lfreenect -lfreenect_sync -llo -o $(BIN_NAME) $(OBJS)

.c.o:
	gcc -c -O2 -Wall $< -I/usr/local/include/libfreenect

clean:
	rm -f $(BIN_NAME) $(OBJS)
