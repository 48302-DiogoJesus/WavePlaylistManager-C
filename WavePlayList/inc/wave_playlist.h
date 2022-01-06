#ifndef WAVE_PLAYLIST
#define WAVE_PLAYLIST

/* ---------- PLAY WAVE ---------- */

// void play(Wave* wave);

/* ---------- FILE SEARCH UTILS ---------- */

#define MAX_FILES 500
#define MAX_FILE_NAME_SIZE 100

// Global array that will hold the filepaths of all the files found matching the pattern
char *filepaths[MAX_FILES];

void file_tree_find_wavs(const char *dirpath);
void file_show_search_results();
static size_t filesFound();

/* ---------- PLAYLIST ---------- */

// Queue Item (Linked list)
typedef struct queueItem {
    struct queueItem *next, *prev;
    Wave *wave;
} QueueItem;

// PLaylist Structure
typedef struct playlist {
    size_t size;
    // Doubly linked list
    struct queueItem *head;
} Playlist;

Playlist *playlist_init();
Wave *playlist_first(Playlist *playlist);
size_t playlist_size(Playlist *playlist);
int playlist_add(Playlist *playlist, Wave *wave);
int playlist_remove(Playlist *playlist, size_t index);
int playlist_wipe(Playlist *playlist);
void playlist_destroy(Playlist *playlist);
void playlist_print(Playlist *playlist);
int playlist_has_file(Playlist *playlist, const char* filename);
void nice_print(const char* message);

/* ---------- COMMANDS ---------- */

typedef struct command {
    char *instruction;
    char *args;
} Command;

#define MAX_INPUT_SIZE 100
#define MAX_COMMANDS_CACHE 100

#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))
#define clear_line() printf("\33[2K\r")
#define clear_screen() printf("\e[1;1H\e[2J")

int cursorYPos = 0;

int *command_get_char();
char *command_wait_valid_input(const char *pre_message);
Command *command_wait(const char *pre_message);
void command_execute(Playlist *playlist, Command *command);
void command_handle_result(int result);
// Individual command functions
void command_help();
void command_scan(char *args);
void command_add(char *args, Playlist *playlist);
void command_remove(char *args, Playlist *playlist);
void command_play(Playlist *playlist);
void command_exit(Playlist *playlist);
void commands_free_history();
int commands_history_size();

char *commands_history[MAX_COMMANDS_CACHE];

#endif