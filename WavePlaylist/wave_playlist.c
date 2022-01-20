#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/select.h>

#include <termios.h>

#include <alsa/asoundlib.h>

#include "wavelib.h"

#include "console.h"

#include "wave_playlist.h"

/* -- ERROR TRY/CATCH SYSTEM BASE ON SETJMP -- */
#include <setjmp.h>

jmp_buf ex_buf__;

#define TRY do {switch (setjmp(ex_buf__)) { case 0:
#define CATCH(err) break; case err:
#define ENDTRY } }while(0)
#define THROW(err) longjmp(ex_buf__, err)

// Error defenitions
#define NO_HEAP_SPACE 1

/* ---------- PROGRAM MAIN FUNCTION ---------- */

int main(int argc, char *argv[])
{
	// Initialize console object and clear the console to start clean 
	console_init();
	console->clear();

	// Build commands structure which will be accessibly through a global object
	build_commands();

	// Do initial wave file search starting at the /home folder
	file_tree_find_wavs("../");
	// Display the previous search results to user
	file_show_search_results();

	// Initialize the playlist
	Playlist *playlist = playlist_init();

	TRY 
	{
		// Commands Cycle
		while (1)
		{
			// Capture command
			char *command = wait_command(">");

			console->clear();

			// Handle Command
			if (command != NULL) {
				char *instruction = strtok((char *)command, " ");
				char *args = strtok(NULL, " ");
				execute_command(instruction, playlist, args);
			}
		}
	}
	CATCH (NO_HEAP_SPACE)
	{
		console->clear();
		console->printString("Out of memory!\n");
		console->printString("Releasing Memory Allocated for the commands history...\n");
		commands_history_free();
		console->printString("Releasing Memory Allocated for the Playlist...\n");
		playlist_destroy(playlist);
		exit(-1);
	}
	ENDTRY;
}

/* ---------------------------------------------- */

/* ------------- COMMANDS STRUCTURE ------------- */

// GLOBAL COMMANDS OBJECT 
Command *commands = NULL;

void insert_command(const char *name, const char *description, void (*execute)(Playlist *playlist, const char *args)) {
	Command *new_command = (Command *)malloc(sizeof (Command));
	if (new_command == NULL)
		THROW(NO_HEAP_SPACE);
	new_command->name = strdup(name);
	new_command->description = strdup(description);
	new_command->execute = execute;
	new_command->next = commands;
	commands = new_command;
}

void build_commands() {
	insert_command("clear", "Clear console", command_clear_console);
	insert_command("play", "Play the files on the playlist by order of insertion. Typing 'p' while playing will pause and typing 'n' will skip to the next file", command_play);
	insert_command("rm", "Ex: rm <playlist_id>. Remove a file(<playlist_id> from list displayed when the command playlist is executed) from the playlist. 'rm *' removes all", command_remove);
	insert_command("add", "Ex: add <file_id>. Add a file(<file_id> from list displayed when the command files is executed) to the playlist", command_add);
	insert_command("list", "Show all the files in the playlist", command_playlist_print);
	insert_command("files", "Show all the files found in the previous scan", command_print_files);
	insert_command("scan", "Ex: <startdir?>. scan Scan the filesystem(starting at <startdir> or '/home' by default) looking for Wave files and show results", command_scan);	
	insert_command("exit", "Safely shutdown the application", command_exit);
	insert_command("help", "Show this helper", command_print_commands);
}

/**
* Execute Command
* Handles the execution for all possible commands
* @param name Name of the command
* @param playlist Pointer to the playlist object
* @param args Arguments to pass into the execute function
*/
void execute_command(const char* name, Playlist *playlist, const char *args) {
	console->clear();
	for (Command *aux = commands; aux != NULL; aux = aux->next) {
		if (strcmp(aux->name, name) == 0) {
			aux->execute(playlist, args);
			return;
		}
	}
	console->printString("Unknown Command. Type 'help' to see all the available commands");
	console->cursorYPos = 4;
}

/* -------------------------------------------- */

/* ------------- COMMANDS HISTORY ------------- */

/**
* Comands History Size
* @returns number of commands on the commands history
*/
int commands_history_size()
{
	int items_num = 0;
	for (; items_num < MAX_COMMANDS_CACHE; items_num++)
	{
		if (*(commands_history + items_num) == 0)
			return items_num;
	}
	return items_num;
}

/**
* Free the pointers allocated for storing the previously used commands (under the form of input)
*/
void commands_history_free()
{
	for (int i = 0; i < commands_history_size(); i++)
	{
		free(commands_history[i]);
	}
}

/* ------------------------------------- */

/**
* Waits for valid user input so that it can build a Command object
* @param pre_message Pointer to the playlist object
* @returns a pointer to a Command object with the instruction and the arguments to execute the instruction with
*/
char *wait_command(const char *pre_message)
{
	/* 
	* Alternative to fgets(input, MAX_INPUT_SIZE, stdin);
	*/
	char *input = wait_valid_input(pre_message);

	int new_command_index = commands_history_size();

	if (new_command_index < MAX_COMMANDS_CACHE)
	{
		commands_history[new_command_index] = (char *)malloc(strlen(input) + 1);

		if (commands_history[new_command_index] == NULL)
			THROW(NO_HEAP_SPACE);
		
		strcpy(commands_history[new_command_index], input);
	}

	return input;
}

/* ------------- INDIVIDUAL COMMANDS IMPLEMENTATION ------------- */

/**
* Print all the available commands
* @param playlist Pointer to playlist object
* @param args
*/
void command_print_commands(Playlist *playlist, const char *args) {
	console->printString(
	"|| -- COMMANDS LIST -- |\n\n"
	"? - Opcional parameter\n");
	for (Command *aux = commands; aux != NULL; aux = aux->next)
		printf("%s:\t\t%s\n", aux->name, aux->description);
	console->cursorYPos = 16;
}

/**
* Exit the program safely by freeing memory allocated during the program execution
* @param playlist Pointer to playlist object
* @param args
*/
void command_exit(Playlist *playlist, const char *args)
{
	console->printString("\nReleasing Memory Allocated for the commands history...\n");
	commands_history_free();
	console->printString("Releasing Memory Allocated for the Playlist...\n");
	playlist_destroy(playlist);
	console->printString("Exiting...\n");
	exit(0);
}

/**
* Scan the filesystem for wave files
* @param playlist Pointer to playlist object
* @param args Starting directory for the scan. If not passed it will scan the default directory (/home)
*/
void command_scan(Playlist *playlist, const char *args)
{
	// Reset the array with the filepaths of the wave files found
	memset(filepaths, 0, MAX_FILES * sizeof(filepaths[0]));
	// First one is the home folder to be faster
	const char *startDir = args == NULL ? "/home/" : args;
	file_tree_find_wavs(startDir);
	file_show_search_results();
}

/**
* Display the wave files found during last scan
* @param playlist Pointer to playlist object
* @param args
*/
void command_print_files(Playlist *playlist, const char *args) {
	file_show_search_results();
}

/**
 * Playlist Print
 * Print all the playlist items to the console
 * @param playlist pointer to the playlist object
 * @param args
 */
void command_playlist_print(Playlist *playlist, const char *args)
{
	size_t current_playlist_size = playlist_size(playlist);
	console->printString("| --  PLAYLIST -- |\n");
	if (current_playlist_size == 0)
		console->printString("Playlist is empty!");

	QueueItem *current = playlist->head;
	for (int i = 0; i < current_playlist_size; i++, current = current->next)
	{
		printf("%d) %s\n", i + 1, strrchr(current->wave->filename, '/') + 1);
	}
	console->cursorYPos = (current_playlist_size < 2 ? 0 : current_playlist_size - 1) + 6;
}

/**
* Add an item to the playlist
* @param playlist Pointer to playlist object
* @param args ID of the item to add
*/
void command_add(Playlist *playlist, const char *args)
{
	if (args == NULL)
	{
		console->printString("You need to specify the file ID. Ex: add 1\nUse the command 'files' to see all the possible IDs");
		console->cursorYPos = 5;
		return;
	}
	size_t index = atoi(args) - 1;
	if (index >= files_found_num())
	{
		console->printString("Invalid ID\nUse the command 'files' to see all the possible IDs");
		console->cursorYPos = 5;
		return;
	}
	const char *filepath = filepaths[index];
	// Allocate space for new Wave file memory representation
	Wave *loadedWave = (Wave *)malloc(sizeof(Wave));
	if (loadedWave == NULL) {
		THROW(NO_HEAP_SPACE);
	}

	loadedWave = wave_load(filepath);

	if (playlist_add(playlist, loadedWave))
		console->printString("Successfuly added to playlist");
	else
		console->printString("Could not add to playlist!");
	console->cursorYPos = 4;
}

/**
* Remove one or all items from the playlist
* @param playlist Pointer to playlist object
* @param args Specify if 1 item(by ID) should be deleted or all of them
*/
void command_remove(Playlist *playlist, const char *args)
{
	if (args == NULL || strlen(args) == 0)
	{
		console->printString("You need to specify the ID(to remove one) or '*'(to remove all).\nUse the command 'playlist' to see add the possible IDs.");
		console->cursorYPos = 5;
		return;
	}
	else if (strcmp(args, "*") == 0)
	{
		// Remove all
		if (playlist_wipe(playlist))
		{
			console->printString("All removed from playlist");
		}
		else
		{
			console->printString("Playlist is empty!");
		}
		console->cursorYPos = 4;
	}
	else
	{
		// Remove individual
		size_t index = atoi(args) - 1;
		if (index < 0 || index > playlist_size(playlist) - 1)
		{
			console->printString("Invalid ID\nUse the command 'files' to see all the possible IDs");
			console->cursorYPos = 5;
			return;
		}
		if (playlist_remove(playlist, index))
			console->printString("Successfuly removed from playlist");
		else
			console->printString("Could not remove from playlist");
		console->cursorYPos = 4;
		return;
	}
}


/**
* Play the full playlist, starting at the first item inserted in the playlist and removing it after it has played
* @param playlist Pointer to playlist object
* @param args
*/
void command_play(Playlist *playlist, const char *args)
{
	console->cursorYPos = 4;

	size_t init_playlist_size = playlist_size(playlist);

	if (init_playlist_size == 0) {
		console->printString("Playlist is empty!");
		return;
	}

	// While there are songs in the playlist
	while (playlist_size(playlist) > 0)
	{
		// Play the first in Queue
		Wave *firstInPlaylist = playlist_first(playlist);

		printf("Currently playing \"%s\"\n", strrchr(firstInPlaylist->filename, '/') + 1);

		int result = play(firstInPlaylist); // UNCOMMENT WHEN ALSA LIB IS WORKING
		if (result == WAVE_PAUSE) {
			console->clear();

			console->printString("Paused");
			console->cursorYPos = 4;
			return;
		}
		if (result == WAVE_NEXT) {
			console->clear();
			console->printString("Skipping...");
			console->cursorYPos = 4;
		}
		// if (result == NEXT) is not needed because it will remove current from playlist and play next one by default
		// Remove from queue after playing
		playlist_remove(playlist, 0);
	}
	// Playlist ended
	console->clear();
	console->printString("No more wave files in queue");
	console->cursorYPos = 4;
}

/**
* Clear console
* @param playlist Pointer to playlist object
* @param args
*/
void command_clear_console(Playlist *playlist, const char *args) {
	console->clear();
	console->cursorYPos = 3;
}

/* ------------------------------------------- */

/**
* Custom function to get user input analysing every keystroke to make detecting key combinations possible
* Functionality Advantages:
* - CTRL+C -> Safely end app (Release memory)
* - Arrows UP & DOWN (use previous commands)
* @param pre_message Message to show before accepting user input (Ex: "Command >")
* @returns A pointer to the user input string
*/
char *wait_valid_input(const char *pre_message)
{
	char *input = (char *)calloc(1, MAX_INPUT_SIZE);
	console->cursorXPos = 3;
	int current_commands_history_size = commands_history_size();
	int commandsCacheIndex = current_commands_history_size == 0 ? 0 : current_commands_history_size - 1;
	putchar('\n');
	while (1)
	{
		printf("%s %s", pre_message, input);
		// Put cursor at the bottom
		console->forceCursorTo(console->cursorXPos, console->cursorYPos - 1);
		int *key = console->get_keycode();
		// printf("\nKey:  %d %d %d\n", key[0], key[1], key[2]);
		if (key[1] != 0)
		{
			// Key combo
			// printf("\nCombo:  %d %d %d\n", key[0], key[1], key[2]);
			if (key[0] == 27 && key[1] == 91)
			{
				switch (key[2])
				{
				case 68:
					if (console->cursorXPos > 3)
					{
						console->cursorXPos--;
					}
					break;
				case 67:
					if (console->cursorXPos - 3 < strlen(input))
					{
						console->cursorXPos++;
					}
					break;
				case 65:
					if (current_commands_history_size > 0 && commandsCacheIndex >= 0)
					{
						strcpy(input, commands_history[commandsCacheIndex]);
						if (commandsCacheIndex - 1 >= 0)
							commandsCacheIndex--;
						console->cursorXPos = strlen(input) + 3;
					}
					break;
				case 66:
					// Make sure the input buffer is not polluted by previous commands if user pressed UP arrow
					if (current_commands_history_size > 0 && commandsCacheIndex <= current_commands_history_size)
					{
						input = (char *)calloc(1, MAX_INPUT_SIZE);
						if (commandsCacheIndex + 1 == current_commands_history_size)
						{
							strcpy(input, "");
							console->cursorXPos = 3;
							break;
						}
						strcpy(input, commands_history[++commandsCacheIndex]);
						console->cursorXPos = strlen(input) + 3;
					}
					break;
				}
				continue;
			}
		}
		else
		{
			if (key[0] == 127)
			{
				if (console->cursorXPos - 3 > 0)
				{
					// Backspace
					int prevLen = strlen(input);
					input[--console->cursorXPos - 3] = '\0';
					if (console->cursorXPos - 2 < prevLen)
					{
						// Shift the right side 1 to the left
						memcpy(input + console->cursorXPos - 3, input + console->cursorXPos - 2, strlen(input + console->cursorXPos - 2));
						input[strlen(input) - 1] = '\0';
					}
				}
			}
			else if (key[0] == 126)
			{
				// Delete
				int prevLen = strlen(input);
				input[--console->cursorXPos - 2] = '\0';
				if (console->cursorXPos - 1 < prevLen)
				{
					// Shift the right side 1 to the left
					memmove(input + console->cursorXPos - 2, input + console->cursorXPos - 1, strlen(input + console->cursorXPos - 1));
					input[strlen(input) - 1] = '\0';
				}
				console->cursorXPos++;
			}
			else if (key[0] == 32)
			{
				// Space
				memmove(input + console->cursorXPos - 2, input + console->cursorXPos - 3, strlen(input + console->cursorXPos - 3));
				input[console->cursorXPos - 3] = 32;
				console->cursorXPos++;
			}
			else if (key[0] == 13)
			{
				// Enter
				if (strlen(input) == 0)
					continue;
				/*
				* If user types a command in which the first or last chars are Spaces 
				* reset the input field  
				*/
				if (input[0] == 32 || input[(console->cursorXPos - 4) == -1 ? 0 : (console->cursorXPos - 4)] == 32)
				{
					// Reset current input
					input = (char *)calloc(1, MAX_INPUT_SIZE);
					console->cursorXPos = 3;
					continue;
				}
				return input;
			}
			else if (key[0] == 3)
			{
				strcpy(input, "exit");
				return input;
			}
			else
			{
				// Non-special key (at the end, beggining or middle)
				// If cursor is not at the end
				if (strlen(input) > console->cursorXPos - 3)
				{
					// When replacing in the beggining doesnt work properly
					memmove(input + console->cursorXPos - 2, input + console->cursorXPos - 3, strlen(input + console->cursorXPos - 3));
				}
				// Append Character
				input[console->cursorXPos++ - 3] = key[0];
			}
		}
	}
}

/* ----------------- PLAYLIST ----------------- */

/**
* Playlist Init
* Allocate space for a Playlist Object, initialize it's size to zero and return a pointer to it
* @returns pointer to the playlist object in memory
*/
Playlist *playlist_init()
{
	// Allocate space for new playlist
	Playlist *newPlaylist = (Playlist *)malloc(sizeof(Playlist));
	if (newPlaylist == NULL) {
		THROW(NO_HEAP_SPACE);
	}
	newPlaylist->size = 0;
	return newPlaylist;
}

/**
 * Playlist Size
 * @param playlist pointer to the playlist object
 * @return number of items inside the playlist
 */
size_t playlist_size(Playlist *playlist)
{
	return playlist->size;
}

/**
 * Add item to Playlist
 * @param playlist pointer to the playlist object
 * @param wave pointer to the wave object to add to [playlist]
 * @returns 1 if the item was added or 0 if not
 */
int playlist_add(Playlist *playlist, Wave *wave)
{
	QueueItem *newQueueItem = (QueueItem *)malloc(sizeof(QueueItem));
	if (newQueueItem == NULL) {
		THROW(NO_HEAP_SPACE);
	}
	newQueueItem->wave = wave;

	// Special case for the first in the playlist
	if (playlist_size(playlist) == 0)
	{
		playlist->head = newQueueItem;
		newQueueItem->next = newQueueItem;
		newQueueItem->prev = newQueueItem;
		playlist->size++;
	}
	else
	{
		/* 
		Example List: <- 1 <-> 2 <-> 3 -> 
		Objective: insert 4
		1: (3 <- 4)
		2: (4 -> 1)
		3: (3 -> 4) & destroy (3 -> 1)
		4: (4 <- 1) & destroy (3 <- 1)
		Result: <- 1 <-> 2 <-> 3 <-> 4 ->
		*/
		newQueueItem->prev = playlist->head->prev;
		newQueueItem->next = playlist->head;
		playlist->head->prev->next = newQueueItem;
		playlist->head->prev = newQueueItem;
		playlist->size++;
	}
	// Confirm if it was added
	if (playlist_has_file(playlist, strrchr(wave->filename, '/') + 1) != -1)
		return 1;
	else
		return 0;
}

/**
 * Remove item from Playlist
 * @param playlist pointer to the playlist object
 * @param index position of the item to remove
 * @returns 1 if the item was removed or 0 if not
 */
int playlist_remove(Playlist *playlist, size_t index)
{
	// Index out of bounds
	if (index == -1 || index + 1 > playlist_size(playlist))
		return 0;

	QueueItem *current = playlist->head;
	for (int searchIndex = 0; searchIndex < index; searchIndex++, current = current->next)
	{
	}
	// Connect prev with next
	current->next->prev = current->prev;
	current->prev->next = current->next;

	// Delete head means head->next will be the new head
	if (index == 0)
	{
		playlist->head = current->next;
	}

	playlist->size--;
	free(current);
	return 1;
}

/**
 * Get the first wave from the playlist
 * @param playlist pointer to the playlist object
 * @returns the first wave object in the playlist
 */
Wave *playlist_first(Playlist *playlist)
{
	return playlist->head->wave;
}

/**
 * Check if playlist has wave file
 * @param playlist pointer to the playlist object
 * @param filename Name/Path of the file to check
 * @returns -1 if the file is not inside the playlist or it's index if it is
 */
int playlist_has_file(Playlist *playlist, const char *filename)
{
	QueueItem *current = playlist->head;
	for (int i = 0; i < playlist_size(playlist); i++, current = current->next)
	{
		if (strcmp(strrchr(current->wave->filename, '/') + 1, filename) == 0)
		{
			// Found it (return index)
			return i;
		}
	}
	return -1;
}

/**
 * Wipe Playlist
 * @param playlist pointer to the playlist object
 * @returns 1 if operation was successful and 0 if not
 */
int playlist_wipe(Playlist *playlist)
{
	if (playlist_size(playlist) == 0)
		return 0;
	QueueItem *next;
	for (QueueItem *p = playlist->head; next != playlist->head; p = next)
	{
		next = p->next;
		// Free the wave first since we had to allocate space for it before adding it to playlist
		free(p->wave);
		free(p);
	}
	// If head is not set to NULL data could still be accessible after the free since the nested structures are not erased
	playlist->head = NULL;
	playlist->size = 0;

	return 1;
}

/**
 * Free Playlist
 * Free the space allocated for the Playlist object and its inner items
 * @param playlist pointer to the playlist object
 */
void playlist_destroy(Playlist *playlist)
{
	playlist_wipe(playlist);
	free(playlist);
}

/* ------------- WAV FILE SEARCH ------------- */

/**
* String Match
* @param pattern Pattern to match
* @param candidate String where the [pattern] will be verified
* @returns '1' if [candidate] string matches a certain [pattern] or '0' if not
*/
static int string_match(const char *pattern, const char *candidate)
{
	if (*pattern == '\0')
	{
		return *candidate == '\0';
	}
	else if (*pattern == '*')
	{
		for (const char *c = candidate; *c != '\0'; c++)
		{
			if (string_match(pattern + 1, c))
				return 1;
		}
		return string_match(pattern + 1, candidate);
	}
	else if (*pattern != '?' && *pattern != *candidate)
	{
		return 0;
	}
	else
	{
		return string_match(pattern + 1, candidate + 1);
	}
}

/**
* Files Found
* @returns the number of files found in the last search
*/
static size_t files_found_num()
{
	size_t items_num = 0;
	for (; items_num < MAX_FILES; items_num++)
		if (*(filepaths + items_num) == 0)
			return items_num;
	return items_num;
}

/**
* Tree File Search
* Recursively searches through the file system starting at [dirpath] searching for files
* which match the pattern "*.wav". If they do, their paths are saved to an array called filepaths
* @param dirpath Path where the depth search will begin
*/
void file_tree_find_wavs(const char *dirpath)
{
	DIR *dir;
	dir = opendir(dirpath);
	if (dir == NULL)
	{
		return;
	}
	else
	{
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL)
		{
			struct stat statbuf;
			char filepath[strlen(dirpath) + 1 + strlen(entry->d_name) + 1];
			strcpy(filepath, dirpath);
			strcat(filepath, "/");
			strcat(filepath, entry->d_name);

			// Check if filename matches pattern
			if (string_match("*.wav", strrchr(filepath, '/') + 1))
			{
				size_t next_index = files_found_num();
				char *string = malloc(strlen(filepath) + 1);
				if (string == NULL) {
					THROW(NO_HEAP_SPACE);
				}

				strcpy(string, filepath);
				filepaths[next_index] = string;
			}

			int result = stat(filepath, &statbuf);
			if (result == -1)
			{
				continue;
			}
			if (S_ISDIR(statbuf.st_mode) &&
				strcmp(entry->d_name, ".") != 0 &&
				strcmp(entry->d_name, "..") != 0)
			{
				file_tree_find_wavs(filepath);
			}
		}
		closedir(dir);
	}
}

/*
* Sort the files from the filepaths array alphabetically by filename
*/
void sort_file_search_results()
{
	size_t files_number = files_found_num();

	// Sort the filepaths using bubble sort
	for (int i = 0; i < files_number; i++)
	{
		for (int j = i + 1; j < files_number; j++)
		{
			// char *filename = strrchr(filepath, '/') + 1;
			if (strcmp(strrchr(filepaths[i], '/') + 1, strrchr(filepaths[j], '/') + 1) > 0)
			{
				char *temp = filepaths[i];
				filepaths[i] = filepaths[j];
				filepaths[j] = temp;
			}
		}
	}
}

/**
* Display file search results to console
*/
void file_show_search_results()
{
	// Sort the filepaths
	sort_file_search_results();
	size_t files_number = files_found_num();
	printf("| -- SEARCH RESULTS -- |\n\n-Results no: %ld\n-Sorted: alphabetically\n-Pattern: \"%s\"\n\n", files_number, "*.wav");
	for (int i = 0; i < files_number; i++)
	{
		printf("%d) %s\n", i + 1, strrchr(filepaths[i], '/') + 1);
	}
	console->cursorYPos = files_number + 9;
}

/* ------------- WAV PLAY MANIPULATIONS ------------- */

#define	SOUND_DEVICE "default"

/**
* @returns 0 -> File reached end; -1 -> UNKNOWN COMMAND; 1 -> WAVE_PAUSE; 2 -> WAVE.NEXT; 
*/
int play(Wave *wave) {

	snd_pcm_t *handle = NULL;
	int result = snd_pcm_open(&handle, SOUND_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
	if (result < 0) {
		printf("snd_pcm_open(&handle, %s, SND_PCM_STREAM_PLAYBACK, 0): %s\n",
				SOUND_DEVICE, snd_strerror(result));
		exit(EXIT_FAILURE);
	}

	snd_config_update_free_global();

	result = snd_pcm_set_params(handle,
					  SND_PCM_FORMAT_S16_LE,
					  SND_PCM_ACCESS_RW_INTERLEAVED,
					  wave_get_number_of_channels(wave),
					  wave_get_sample_rate(wave),
					  1,
					  500000);
	if (result < 0) {
		fprintf(stderr, "Playback open error: %s\n", snd_strerror(result));
		exit(EXIT_FAILURE);
	}

	const snd_pcm_sframes_t period_size = 64;
	int frame_size = snd_pcm_frames_to_bytes(handle, 1);

	uint8_t buffer[period_size * frame_size];
	size_t frame_index = 0;

	size_t read_frames = wave_get_samples(wave, frame_index, buffer, period_size);

	// Setup in order to allow async keyboard interruptions
	set_conio_terminal_mode();

	while (read_frames > 0) {
		// If user presses key while playing...
		if (kbhit()) {
			int key_pressed = termius_get_char();

			int return_value = -1;
			switch(key_pressed) {
				case 'p':
					// Save the current frame in order to play it from there next time
					last_frame_index = frame_index;
					return_value = WAVE_PAUSE;
					break;
				case 'n':
					return_value = WAVE_NEXT;
					break;
			}
			// Reset terminal
			reset_terminal_mode();
			return return_value;
		}
		if (last_frame_index == 0) {
			snd_pcm_sframes_t wrote_frames = snd_pcm_writei(handle, buffer, read_frames);
			if (wrote_frames < 0)
				wrote_frames = snd_pcm_recover(handle, wrote_frames, 0);
			if (wrote_frames < 0) {
				printf("snd_pcm_writei failed: %s\n", snd_strerror(wrote_frames));
				break;
			}

			if (wrote_frames < read_frames)
				fprintf(stderr, "Short write (expected %li, wrote %li)\n",
						read_frames, wrote_frames);
		} else {
			frame_index = last_frame_index;
			last_frame_index = 0;
		}

		frame_index += period_size;
		read_frames = wave_get_samples(wave, frame_index, buffer, period_size);
	}

	/* pass the remaining samples, otherwise they're dropped in close */
	result = snd_pcm_drain(handle);
	if (result < 0)
		printf("snd_pcm_drain failed: %s\n", snd_strerror(result));

	snd_pcm_close(handle);
	snd_config_update_free_global();
	
	// Reset terminal
	reset_terminal_mode();

	last_frame_index = 0;

	return 0;
}
