#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "file_tree_foreach.h"

// Max number of files that can be found in one search
#define MAX_FILES 500
// Max number of characters for the filenames
#define MAX_FILE_NAME_SIZE 100

// Global array that will hold the filenames of all the files found matching the pattern
char *filenames[MAX_FILES];

/* -- Private Functions -- */
static void saveFileName(const char *filename, void *context);
static void sortFileNames(const char *context);
static size_t arraySize();


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <start dir> <pattern>\n", argv[0]);
        return -1;
    }
    // Search through the filesystem for files matching the pattern the user requested starting at a location user-defined as well
    file_tree_foreach(argv[1], saveFileName, argv[2]);
    // Sort the files from the search and display the results to the user
    sortFileNames(argv[2]);
}

/**
* Save a filename
* @param filename Name of the file to save
* @param context The context in which the file was found
*/
static void saveFileName(const char *filename, void *context)
{
    size_t next_index = arraySize();
    // Important to allocate memory for the new string
    char *string = malloc(strlen(filename) + 1);
    // Copy to the new allocated space
    strcpy(string, filename);
    // Save address of the pointer to the string in filenames array
    filenames[next_index] = string;
}

/**
* Sort File Names
* Sort the names of the files found alphabetically
* @param context The context in which the filenames were found
*/
static void sortFileNames(const char *context)
{
    size_t files_number = arraySize();

    if (files_number == 0) {
        printf("| -- SEARCH RESULTS -- |\n\n-Results no: %ld\n-Sorted: alphabetically\n-Pattern: \"%s\"\n\n", files_number, context);
        return;
    }
    
    // Sort the filenames using bubble sort
    for (int i = 0; i < files_number; i++) {
        for (int j = i+1; j < files_number; j++) {
            if (strcmp(filenames[i], filenames[j]) > 0) {
                char* temp = filenames[i]; 
                filenames[i] = filenames[j]; 
                filenames[j] = temp; 
            }
        }
    }

    // Display sorted results
    printf("| -- SEARCH RESULTS -- |\n\n-Results no: %ld\n-Sorted: alphabetically\n-Pattern: \"%s\"\n\n", files_number, context);
    for (int i = 0; i < files_number; i++)
    {
        printf("%d) %s\n", i + 1, filenames[i]);
    }
    return;
}

/**
* Array Size
* Returns the number of element inside the filenames array
*/
static size_t arraySize() {
  size_t items_num = 0;
  for (; items_num < MAX_FILES ;items_num++)
    if (*(filenames + items_num) == 0) return items_num;
  return items_num;
}
