#ifndef _TERMINALSTREAM_H
#define _TERMINALSTREAM_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#ifndef __DEBUG
#define __DEBUG 0
#endif

#define DEBUG if(__DEBUG)

/**
 * arr, pointer to 2d char array
 * cmdCount, pointer to a int that will be set to the amount of commands in the array, words, pipes, etc, but not the word that is NULL
*/
int terminalStream(char ****arr, int *cmdCount, char ***wordArr, int *wordCount, int **allocatedWords);

#endif