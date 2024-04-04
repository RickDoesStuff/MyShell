#ifndef _TERMINALSTREAM_H
#define _TERMINALSTREAM_H

#include "executeCmd.h"

#ifndef __DEBUG
#define __DEBUG 0
#endif

#define DEBUG if(__DEBUG)





/**
 * arr, pointer to a string array, will be written to, all words typed into a terminal for one command
 * wordCount, pointer to an int, will be written to with the # of words in the arr
 * command struct, where we write the commands to
 * return 1 on success
 * return 0 on user request to exit
 * return -1 on error
*/
int terminalStream(char ***wordArr, int *wordCount, command *cmd);

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