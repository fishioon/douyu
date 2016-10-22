CXXFLAGS=-O2

bin = douyu
objs = danmu.o client.o

$(bin): $(objs)
	g++ -o $(bin) $(objs) -lpthread

.phony: clean

clean:
	rm $(objs) $(bin)
