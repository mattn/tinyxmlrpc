.SUFFIXES: .cxx .o

all : test

test : tinyxmlrpc.o test.o
	g++ -g -o $@ tinyxmlrpc.o test.o `pkg-config --libs libxml-2.0` -lcurl

rssping : tinyxmlrpc.o rssping.o
	g++ -g -o $@ tinyxmlrpc.o rssping.o `pkg-config --libs libxml-2.0` -lcurl

.cxx.o :
	g++ -g `pkg-config --cflags libxml-2.0` -c $<

clean :
	rm -f *.o test rssping
