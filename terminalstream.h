#ifndef _TERMINALSTREAM_H
#define _TERMINALSTREAM_H

#include "executeCmd.h"

#ifndef __DEBUG
#define __DEBUG 0
#endif

#define DEBUG if(__DEBUG)





/**
 * arr, pointer to 2d char array
 * cmdCount, pointer to a int that will be set to the amount of commands in the array, words, pipes, etc, but not the word that is NULL
*/
int terminalStream(char ***wordArr, int *wordCount);

/**
 * take in an array of strings and the amount of words in that array
 * and writes to them
 * return 1 on success
 * return 0 on something really weird happening
 * return -1 on error
*/
int readWordsIntoArray(char ***arr, int *wordCount);

// free the words in the current command
int freeWords(command *cmd);

// setup the command for the first time
int initCommand(command *cmd);

// reset the command
int resetCommand(command *cmd);

// malloc the words
int mallocWords(command *cmd);


#endif