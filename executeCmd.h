#ifndef _EXECUTECMD_H
#define _EXECUTECMD_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>


typedef struct command{
    int type; // type of command, 0 - normal, 1 - pipe
    int size; // how many words allocated 
    int length; // how many words in command
    int indexStart; // what index in the wordArr does this command array start on
    char **words; // an array of words
} command;

/**
 * take a pointer to a command struct
 * execute the given command
*/
int execute_command(command *cmd);

#endif