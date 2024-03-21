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
 * arrSize, pointer to amt of pointers in the array
 * arr, pointer to 2d char array
 * wordCount, pointer to a int that will be set to the amount of index's in the array, words, pipes, etc, but not the word that is NULL
*/
int terminalStream(int *arrSize, char ***arr, int *wordCount);

#endif