
OBJ=player.o util.o init.o info.o
GCH=player.h.gch util.h.gch init.h.gch info.h.gch
LIB=-lncursesw -lpthread
CC=gcc
CPP=g++
FLAG=-c -w

all:pcm minimad tplayer
	cp tplayer minimad pcm play ../bin

tplayer:${OBJ}
	${CPP} ${OBJ} ${LIB} -o tplayer

player.o:player.c player.h
	${CPP} ${FLAG} player.c player.h

util.o:util.c util.h
	${CC} ${FLAG} util.c util.h 

init.o:init.c init.h
	${CPP} ${FLAG} init.c init.h

info.o:info.h info.c
	${CC} ${FLAG} info.c info.h

pcm:pcm.c
	${CC} pcm.c -lasound -o pcm

minimad:minimad.c
	${CC} minimad.c -lmad -o minimad

install:
	cp tplayer minimad pcm play /usr/bin

clean:
	rm ${OBJ} tplayer pcm minimad ${GCH}
	rm ../bin/*

uninstall:
	cd /usr/bin && rm tplayer minimad pcm play
