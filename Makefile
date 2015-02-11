CC=g++
LD=g++
CFLAGS=-c -O3 -std=c++11 -Iinclude
LDFLAGS=-Wl,--no-as-needed -lpthread

OBJS=test.o InetAddr.o LogFile.o TimeUtil.o Logger.o
TARGET=main

$(TARGET):$(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) $<

%.o : %.cpp
	$(CC) $(CFLAGS) $<

test.o : include/Logger.hpp

.PHONY:clean
clean:
	rm -f $(TARGET) $(OBJS) core
