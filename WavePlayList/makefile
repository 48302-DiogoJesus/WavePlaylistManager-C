CC = gcc

CFLAGS = -Wall

LIBS = ./lib/

BUILD = ./build/

INC = ./inc/

####### LINK "wave_playlist.o" TO "wave_lib" library #######
####### STATIC LINKING #######
static_linking_complete:
	make wave_playlist.o && make console.o && $(CC) $(CFLAGS) -static $(BUILD)console.o $(BUILD)wave_playlist.o -o wave_playlist -L. $(LIBS)lib_wavelib_static.a -I $(INC)

####### DYNAMIC LINKING #######
dynamic_linking_complete:
	$(CC) $(CFLAGS) wave_playlist.c -o wave_playlist -L. $(LIBS)lib_wavelib_dynamic.so -I $(INC)

####### CREATE OBJECTS #######

# CREATE OBJECT FROM "wave_playlist"
wave_playlist.o: wave_playlist.c 
	$(CC) $(CFLAGS) $< -c -o $(BUILD)$@ -I $(INC)

console.o: console.c
	$(CC) $(CFLAGS) $< -c -o $(BUILD)$@

####### CLEAN COMMANDS #######
clean: 
	rm -f $(BUILD)* && rm -f $(LIBS)*