CC = gcc

CFLAGS = -Wall

LIBS = ./lib/

SRC = ./src/

BUILD = ./build/

INC = ./inc/

####### STATIC LINKING SINGLE COMMAND #######
# 1 - Create wave_dump.o and wavelib_static.o | 2 - Create Library | 3 - Link library with wave_dump.o
static_linking_complete:
	make wave_dump.o && make wavelib_static.o && make lib_wavelib_static.a && make static_linking && make clean_objects && make

####### DYNAMIC LINKING SINGLE COMMAND #######
# 1 - Create wave_dump.o and wavelib_static_dynamic.o | 2 - Create Library | 3 - Link library with wave_dump.o | 4 - Add dynamic library to global libraries folder
dynamic_linking_complete:
	make wave_dump.o && make wavelib_dynamic.o && make lib_wavelib_dynamic.so && make dynamic_linking && make clean_objects && cp $(LIBS)lib_wavelib_dynamic.so /lib/


####### CREATE OBJECTS #######
# CREATE OBJECT FROM "WAVE_DUMP.c"
wave_dump.o: wave_dump.c
	$(CC) $(FLAGS) $< -c -o $(BUILD)$@ -I $(INC)
# CREATE OBJECT FROM "WAVELIB.c" (STATIC)
wavelib_static.o: $(SRC)wavelib.c
	$(CC) $(CFLAGS) -c $< -o $(BUILD)$@ -I $(INC)

# CREATE OBJECT FROM "WAVE_LIB.c" (DYNAMIC)
wavelib_dynamic.o: $(SRC)wavelib.c
	$(CC) $(CFLAGS) -c -fpic $< -o $(BUILD)$@ -I $(INC)


####### CREATE LIBRARIES #######
# CREATE STATIC LIBRARY #
lib_wavelib_static.a: $(BUILD)wavelib_static.o
	$(CC) $(CFLAGS) -c $(SRC)wavelib.c -o $< -I $(INC) && ar cr $(LIBS)$@ $<

# CREATE DYNAMIC LIBRARY #
lib_wavelib_dynamic.so: $(BUILD)wavelib_dynamic.o
	$(CC) $(CFLAGS) -c -fpic $(SRC)wavelib.c -o $< -I $(INC) && $(CC) $(CFLAGS) -shared -o $(LIBS)$@ $<


####### LINK "wave_dump.o" TO LIBRARIES #######
# LINK TO STATIC LIBRARY #
static_linking: $(BUILD)wave_dump.o $(LIBS)lib_wavelib_static.a
	$(CC) $(CFLAGS) -static $< -o wave_dump -L. $(LIBS)lib_wavelib_static.a

# LINK TO DYNAMIC LIBRARY #
dynamic_linking: $(BUILD)wave_dump.o $(LIBS)lib_wavelib_dynamic.so
	$(CC) $(CFLAGS) $< -o wave_dump -L. $(LIBS)lib_wavelib_dynamic.so


####### CLEAN COMMANDS #######
clean: 
	rm -f $(BUILD)* && rm -f $(LIBS)*