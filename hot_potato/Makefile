all: ringmaster player

ringmaster: ringmaster.cpp potato.h 
	g++ -g -o ringmaster ringmaster.cpp

player: player.cpp potato.h
	g++ -g -o player player.cpp

.PHONY:
	clean

clean:
	rm -rf *.o ringmaster player
