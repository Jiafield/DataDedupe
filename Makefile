all: *.cpp
	g++ -o deDupe -g -Wall -std=gnu++0x main.cpp TTTD_s.cpp Utilities.cpp -lssl -lcrypto
clean:
	rm -f *.o deDupe *~
