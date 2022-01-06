all: wave_dump wave_play wave_capture

wave_dump: wave_dump.o
	gcc wave_dump.o  -lwave -L./wavelib -lasound -o wave_dump

wave_play: wave_play.o
	gcc wave_play.o -lwave -L./wavelib -lasound -o wave_play

wave_capture: wave_capture.o
	gcc wave_capture.o -lwave -L./wavelib -lasound -o wave_capture

wave_dump.o: wave_dump.c
	gcc -c wave_dump.c -Iwavelib -g

wave_play.o: wave_play.c
	gcc -c wave_play.c -Iwavelib -g

wave_capture.o: wave_capture.c
	gcc -c wave_capture.c -Iwavelib -g

clean:
	rm -rf *.o *.i
