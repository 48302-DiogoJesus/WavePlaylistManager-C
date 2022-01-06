CC = gcc

CFLAGS = -Wall

LIBS = ./lib/

BUILD = ./build/

INC = ./inc/

####### CREATE OBJECTS #######

# CREATE OBJECT FROM "wave_playlist"
wave_playlist.o: wave_playlist.c 
	$(CC) $(CFLAGS) $< -c -o $(BUILD)$@ -I $(INC)


####### LINK "wave_playlist.o" TO "wave_lib" library #######

####### STATIC LINKING #######
static_linking_complete:
	make wave_playlist.o && $(CC) $(CFLAGS) -static $(BUILD)wave_playlist.o -o wave_playlist -L. $(LIBS)lib_wavelib_static.a -I $(INC)

####### DYNAMIC LINKING #######
dynamic_linking_complete:
	make wave_playlist.o && $(CC) $(CFLAGS) wave_playlist.c -o wave_playlist -L. $(LIBS)lib_wavelib_dynamic.so -I $(INC)


####### CLEAN COMMANDS #######
clean: 
	rm -f $(BUILD)* && rm -f $(LIBS)*