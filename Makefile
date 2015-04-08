CC=g++
AR=ar
CFLAGS =-Wall -c -g -Iinclude
CCFLAGS=-Wall -c -g -std=c++11 -Iinclude 
#LDFLAGS=-Wl,--no-as-needed -lpthread -lrt
ARFLAGS=crs

LIBOBJS=InetAddr.o InetSock.o LogFile.o TimeUtil.o Daemon.o \
	Logger.o FileUtil.o Channel.o MultiplexLooper.o TcpConnection.o \
	TcpAcceptor.o TcpConnector.o TcpServer.o MessageLooper.o BitmapTree.o HashedWheelTimer.o

TARGET=libnetio.a

$(TARGET):$(LIBOBJS)
	$(AR) $(ARFLAGS) $(TARGET) $(LIBOBJS) 

%.o : %.c
	$(CC) $(CFLAGS) $<

%.o : %.cpp
	$(CC) $(CCFLAGS) $<

%.d : %.cpp
	set -e; rm -f $@; \
	$(CC) -MM $(CCFLAGS) $< > $@.$$$$; \
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
