all: *.cpp
	g++ -o deDupe -g -Wall main.cpp TTTD_s.cpp Bloom_filter.cpp Utilities.cpp -lssl -lcrypto -std=gnu++0x
clean:
	rm -f *.o deDupe *~
