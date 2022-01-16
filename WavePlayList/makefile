CC = gcc

CFLAGS = -Wall

LIBS = ./lib/

BUILD = ./build/

INC = ./inc/

####### LINK "wave_playlist.o" TO "wave_lib" library #######
####### STATIC LINKING #######
static_linking_complete:
	make console.o && make wave_playlist.o && $(CC) $(CFLAGS) $(BUILD)console.o $(BUILD)wave_playlist.o -o wave_playlist -lasound -L. $(LIBS)lib_wavelib_static.a -I $(INC)

####### DYNAMIC LINKING #######
dynamic_linking_complete:
	make console.o && $(CC) $(CFLAGS) $(BUILD)console.o wave_playlist.c -o wave_playlist -lasound -L. $(LIBS)lib_wavelib_dynamic.so -I $(INC)

####### CREATE OBJECTS #######

# CREATE OBJECT FROM "wave_playlist"
wave_playlist.o: wave_playlist.c
	$(CC) $(CFLAGS) $< -c -o $(BUILD)$@ -lasound -I $(INC)

console.o: console.c
	$(CC) $(CFLAGS) $< -c -o $(BUILD)$@

####### CLEAN COMMANDS #######
clean: 
	rm -f $(BUILD)*