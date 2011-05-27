all:
	g++ -O2 -Wall -g -c BasicExample.cpp -I../
	g++ -O2 -Wall -g -c CommandsExample.cpp -I../
	g++ -O2 -Wall -g -c KeymapExample.cpp -I../
	g++ -g -o BasicExample BasicExample.o -L/usr/local/lib -lreadline -lncurses
	g++ -g -o CommandsExample CommandsExample.o -L/usr/local/lib -lreadline -lncurses
	g++ -g -o KeymapExample KeymapExample.o -L/usr/local/lib -lreadline -lncurses

clean:
	rm -rf *.o KeymapExample BasicExample CommandsExample

