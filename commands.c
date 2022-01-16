/* -- NOT BEING USED -- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct command {
    char *description;
    void (*execute)(char*, char*);
} Command;

typedef struct commands {
    Command *exit;
    Command *help;
    Command *scan;
    Command *add;
    Command *remove;
    Command *play;
    Command *files;
    Command *playlist;
} Commands;


void command_help(char *args, char *args1);
void command_scan(char *args, char *args1);
void command_add(char *args, char *args1);
void command_remove(char *args, char *args1);
void command_play(char *args, char *args1);


void command_exit() {
    printf("Closing... ");
    exit(0);
}

Command *buildCommand(const char *description, void (*execute)(char*, char*)) {
    Command *cmd = (Command *)malloc(sizeof(Command));
    cmd->description = (char *)malloc(strlen(description) + 1);
    strcpy(cmd->description, description);
    cmd->execute = execute;
    return cmd;
}

Commands *buildCommands() {
    Commands *commands = (Commands *)malloc(sizeof(Commands));

    commands->exit = (Command *)buildCommand("Safely shutdown the application", command_exit);
    commands->help = (Command *)buildCommand("Show this helper", command_help);
    commands->scan = (Command *)buildCommand("Scan the filesystem(starting at <startdir> or '/home' by default) looking for Wave files and show results", command_scan);
    commands->add = (Command *)buildCommand("Add a file(<file_id> from list displayed when the command files is executed) to the playlist", command_add);
    commands->remove = (Command *)buildCommand("Remove a file(<playlist_id> from list displayed when the command playlist is executed) from the playlist. 'rm *' removes all", command_remove);

    // commands->play = (Command *)buildCommand("Start playing the files on the playlist by order of insertion", command_play);
    // commands->files = (Command *)buildCommand("Show all the files found in the previous scan", file_show_search_results);
    // commands->playlist = (Command *)buildCommand("Show all the files in the playlist", playlist_print);
    
    return commands;
}

void freeCommands() {

}

int main() {
    buildCommands();

}
