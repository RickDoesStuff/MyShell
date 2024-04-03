#ifndef _MYSH_H
#define _MYSH_H
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>


// free the array
int freeArr(char ***wordArr, int *wordCount);

#endif