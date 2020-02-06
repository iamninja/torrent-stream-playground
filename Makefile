CC = g++
CFLAGS = -lpthread -lstdc++ -ltorrent-rasterbar -lboost_filesystem

simple: simple.cpp
	$(CC) -o ./out/$@ $< $(CFLAGS)

clean:
	rm -f ./out/*
