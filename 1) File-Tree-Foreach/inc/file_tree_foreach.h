#ifndef TREE_SEARCH
#define TREE_SEARCH

void file_tree_foreach(const char *dirpath, void (*doit)(const char *, void *), void *context);

#endif