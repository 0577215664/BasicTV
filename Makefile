CFLAGS+=-Wall -Wextra
LDFLAGS+=-std=c++11 -lstdc++ -lcurl -lSDL2_net -lSDL2
CC=clang++

all:
	$(CC) $(LDFLAGS) */*.cpp *.cpp $(CFLAGS) 

debug:
	$(CC) $(LDFLAGS) */*.cpp *.cpp $(CFLAGS) -O0 -g
