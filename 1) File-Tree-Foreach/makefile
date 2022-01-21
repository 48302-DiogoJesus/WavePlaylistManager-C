CC = gcc

CFLAGS = -Wall

LIBS = ./lib/

SRC = ./src/

BUILD = ./build/

INC = ./inc/

################## STATIC AND DYNAMIC LINKING SINGLE COMMAND ##################

both:
	make static_linking_complete && make dynamic_linking_complete

####### STATIC LINKING SINGLE COMMAND #######
# 1 - Create prog_teste.o and file_tree_foreach_static.o | 2 - Create Library | 3 - Link library with prog_teste.o
static_linking_complete:
	make prog_teste.o && make file_tree_foreach_static.o && make lib_file_tree_foreach_static.a && make static_linking

####### DYNAMIC LINKING SINGLE COMMAND #######
# 1 - Create prog_teste.o and file_tree_foreach_dynamic.o | 2 - Create Library | 3 - Link library with prog_teste.o | 4 - Add dynamic library to global libraries folder
dynamic_linking_complete:
	make prog_teste.o && make file_tree_foreach_dynamic.o && make lib_file_tree_foreach_dynamic.so && make dynamic_linking && cp $(LIBS)lib_file_tree_foreach_dynamic.so /lib/

###############################################################################

####### CREATE OBJECTS #######

# CREATE OBJECT FROM "PROG_TESTE"
prog_teste.o: prog_teste.c
	$(CC) $(CFLAGS) -c $< -o $(BUILD)$@ -I $(INC)

# CREATE OBJECT FROM "FILE_TREE_FOREACH.c" (STATIC)
file_tree_foreach_static.o: $(SRC)file_tree_foreach.c
	$(CC) $(CFLAGS) -c $< -o $(BUILD)$@ -I $(INC)

# CREATE OBJECT FROM "FILE_TREE_FOREACH.c" (DYNAMIC)
file_tree_foreach_dynamic.o: $(SRC)file_tree_foreach.c
	$(CC) $(CFLAGS) -c -fpic $< -o $(BUILD)$@ -I $(INC)


####### CREATE LIBRARIES #######

# CREATE STATIC LIBRARY #
lib_file_tree_foreach_static.a: $(BUILD)file_tree_foreach_static.o
	ar cr $(LIBS)$@ $<

# CREATE DYNAMIC LIBRARY #
lib_file_tree_foreach_dynamic.so: $(BUILD)file_tree_foreach_dynamic.o
	$(CC) $(CFLAGS) -shared -o $(LIBS)$@ $<


####### LINK "prog_teste.o" TO LIBRARIES #######

# LINK TO STATIC LIBRARY #
static_linking: $(BUILD)prog_teste.o $(LIBS)lib_file_tree_foreach_static.a
	$(CC) $(CFLAGS) -static $< -o prog_teste_s -L. $(LIBS)lib_file_tree_foreach_static.a

# LINK TO DYNAMIC LIBRARY #
dynamic_linking: prog_teste.c $(LIBS)lib_file_tree_foreach_dynamic.so
	$(CC) $(CFLAGS) $< -o prog_teste_d -L. $(LIBS)lib_file_tree_foreach_dynamic.so -I $(INC)


####### CLEAN BUILD FOLDER #######
clean: 
	rm -f $(BUILD)*