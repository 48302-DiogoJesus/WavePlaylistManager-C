#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "file_tree_foreach.h"

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
* Tree File Search
* Recursively searches through the file system starting at [dirpath] searching for files
* which match a certain [context]. If they do, the function [doit] is called passing in
* the file name
* @param dirpath Path where the depth search will begin 
* @param doit Function to be called everytime a filename matches [context]
* @param context Pattern to search for while searching through the files system
*/
void file_tree_foreach(const char *dirpath, void (*doit)(const char *, void *), void *context)
{
    DIR *dir;
    dir = opendir(dirpath);
    if (dir == NULL)
    {
        fprintf(stderr, "opendir(%s): %s\n", dirpath, strerror(errno));
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

            char *filename = strrchr(filepath, '/') + 1;
            if (string_match(context, filename))
            {
                doit(filename, context);
            }

            int result = stat(filepath, &statbuf);
            if (result == -1)
            {
                fprintf(stderr, "stat(%s,...): %s\n", filepath, strerror(errno));
                continue;
            }
            if (S_ISDIR(statbuf.st_mode) &&
                strcmp(entry->d_name, ".") != 0 &&
                strcmp(entry->d_name, "..") != 0)
            {
                file_tree_foreach(filepath, doit, context);
            }
        }
        closedir(dir);
    }
}