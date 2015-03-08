CC=g++
LD=g++
CFLAGS =-std=c++11 -Iinclude 
CCFLAGS=-c -g
LDFLAGS=-Wl,--no-as-needed -lpthread -lrt

# TcpConnection.o

LIBOBJS=test.o InetAddr.o InetSock.o LogFile.o TimeUtil.o \
	Logger.o FileUtil.o Channel.o MultiplexLooper.o \
	TcpAcceptor.o TcpConnector.o TcpServer.o MessageLooper.o
TARGET=main

$(TARGET):$(LIBOBJS)
	$(LD) -o $(TARGET) $(LIBOBJS) $(LDFLAGS)

%.o : %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $<

%.o : %.cpp
	$(CC) $(CCFLAGS) $(CFLAGS) $<

%.d : %.cpp
	set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.c
	set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(LIBOBJS:.o=.d)

.PHONY:clean
clean:
	rm -f $(TARGET) $(LIBOBJS) $(LIBOBJS:.o=.d) core *.log
